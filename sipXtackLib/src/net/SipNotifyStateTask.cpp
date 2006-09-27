//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "net/SipNotifyStateTask.h"
#include "net/SipUserAgent.h"
#include "net/HttpBody.h"
#include "net/NameValueTokenizer.h"
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"
#include "os/OsEventMsg.h"
#include "os/OsSysLog.h"
#include "os/OsDefs.h"

#ifdef _VXWORKS
#include "resparse/vxw/hd_string.h"
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define RUN_SCRIPT_BUSY_DELAY    60  // When the phone is busy, this is the
                                     // amount of time the phone will wait
                                     // before retrying.

// STATIC VARIABLE INITIALIZATIONS
// STRUCTURES
struct tagRunScriptInfo
{
   UtlString*              pContent ;
} ;   // Used to package data through the OsEvent/queue mechanism


/* //////////////////////////// PUBLIC //////////////////////////////////// */
void SipNotifyStateTask::defaultReboot()
{
    osPrintf("SipNotifyStateTask::defaultReboot request to reboot\n");
}

void SipNotifyStateTask::defaultBinaryMessageWaiting(const char* toUrl,
                                  UtlBoolean newMessages)
{
    osPrintf("SipNotifyStateTask::defaultBinaryMessageWaiting Message status for: %s\n%snew messages available\n",
        toUrl, newMessages ? "" : "NO ");
}

void SipNotifyStateTask::defaultDetailMessageWaiting(const char* toUrl,
                                  const char* messageMediaType,
                                  UtlBoolean absoluteValues,
                                  int totalNewMessages,
                                  int totalOldMessages,
                                  int totalUntouchedMessages,
                                  int urgentUntouchedMessages,
                                  int totalSkippedMessages,
                                  int urgentSkippedMessages,
                                  int totalFlaggedMessages,
                                  int urgentFlaggedMessages,
                                  int totalReadMessages,
                                  int urgentReadMessages,
                                  int totalAnsweredMessages,
                                  int urgentAnsweredMessages,
                                  int totalDeletedMessages,
                                  int urgentDeletedMessages)
{
    osPrintf("SipNotifyStateTask::defaultDetailMessageWaiting\n\
Messages status for URL: %s\n\
Message media type: %s\n\
%s\n\
%d new messages\n\
%d old messages\n\
Status\t\tTotal\tUrgent\n\
Untouched\t%d\t%d\n\
Skipped\t\t%d\t%d\n\
Flagged\t\t%d\t%d\n\
Read\t\t%d\t%d\n\
Answered\t%d\t%d\n\
Deleted\t\t%d\t%d\n",
               toUrl,
               messageMediaType,
               absoluteValues ? "Absolute counts:" : "Message deltas:",
               totalNewMessages,
               totalOldMessages,
               totalUntouchedMessages,
               urgentUntouchedMessages,
               totalSkippedMessages,
               urgentSkippedMessages,
               totalFlaggedMessages,
               urgentFlaggedMessages,
               totalReadMessages,
               urgentReadMessages,
               totalAnsweredMessages,
               urgentAnsweredMessages,
               totalDeletedMessages,
               urgentDeletedMessages);
}

/* ============================ CREATORS ================================== */

// Constructor
SipNotifyStateTask::SipNotifyStateTask(const UtlString& checkSyncPolicy,
                                       SipUserAgent* pSipUserAgent) :
   OsServerTask("SipNotifyStateTask-%d"),
   mCheckSyncPolicy(checkSyncPolicy)
{
   mpSipUserAgent = pSipUserAgent;
   mpRebootFunction = NULL;
   mpBinaryMessageWaitingFunction = NULL;
   mpDetailedMessageWaitingFunction = NULL;
   mpRunScriptTimer = NULL ;
   mpRunScriptEvent = NULL ;
}

// Copy constructor
SipNotifyStateTask::SipNotifyStateTask(const SipNotifyStateTask& rSipNotifyStateTask)
{
}

// Destructor
SipNotifyStateTask::~SipNotifyStateTask()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipNotifyStateTask&
SipNotifyStateTask::operator=(const SipNotifyStateTask& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipNotifyStateTask::handleMessage(OsMsg& eventMessage)
{
        int msgType = eventMessage.getMsgType();
        int msgSubType = eventMessage.getMsgSubType();

    // SIP message
    if(msgType == OsMsg::PHONE_APP &&
       msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();

        // If this is a NOTIFY request
        UtlString method;
        if(sipMessage) sipMessage->getRequestMethod(&method);
        method.toUpper();
        if(sipMessage &&
            method.compareTo(SIP_NOTIFY_METHOD) == 0 &&
            !sipMessage->isResponse())
        {
            osPrintf("SipNotifyStateTask::handleMessage got NOTIFY message\n");
            // Loop through the event fields
            int eventIndex = 0;
            const char* eventFieldCharValue = NULL;
            UtlString eventField;
            if ((eventFieldCharValue = sipMessage->getHeaderValue(eventIndex, SIP_EVENT_FIELD)))
            {
                eventField = eventFieldCharValue;
                eventField.toLower();
                                // Need to get the body to if it is text base.
                // We are only going to support text format for now.
                UtlString contentType;
                sipMessage->getContentType(&contentType);
                contentType.toLower();
                const HttpBody* httpBody;


                // If this is a message waiting indication
                // Looking for header: "Event: message-summary"
                // This is for the draft-mahy-sip-message-waiting-00.txt version
                // of the draft.
                if(eventField.index(SIP_EVENT_MESSAGE_SUMMARY, 0, UtlString::ignoreCase) == 0 &&
                                        (contentType.index(CONTENT_TYPE_TEXT_PLAIN, 0, UtlString::ignoreCase) == 0 &&
                        (httpBody = sipMessage->getBody()) != NULL))
                {
                    osPrintf("SipNotifyStateTask::handleMessage found message-summary event ContentType plain/text\n");
                                        UtlDList bodyHeaderNameValues;
                    const char* bodyBytes;
                    int bodyLength;
                    httpBody->getBytes(&bodyBytes, &bodyLength);
                    HttpMessage::parseHeaders(bodyBytes, bodyLength,
                          bodyHeaderNameValues);

                    UtlString toField;
                    sipMessage->getToField(&toField);

                    // Loop through the body header nameValue pairs
                    UtlDListIterator iterator(bodyHeaderNameValues);
                        NameValuePair* nv;
                        while ((nv = (NameValuePair*) iterator()))
                        {
                        HttpMessage::cannonizeToken(*nv);

                        // if binary message waiting status
                        osPrintf("SipNotifyStateTask::handleMessage body field name: %s value: %s\n",
                            nv->data(), nv->getValue());
                        // Check for either a "Messages-Waiting" or a "Message-Waiting" field.
                        // "Messages-Waiting" is what's specified in the Internet draft;
                        // "Message-Waiting" is needed for backward compatibility.
                        if((strcmp(nv->data(), "Messages-Waiting") == 0) ||
                           (strcmp(nv->data(), "Message-Waiting") == 0))
                        {
                            if(mpBinaryMessageWaitingFunction)
                            {
                                UtlString binaryStatus = nv->getValue();
                                binaryStatus.toLower();
                                UtlBoolean newMessages = FALSE;
                                if(binaryStatus.compareTo("yes") == 0) newMessages = TRUE;

                                mpBinaryMessageWaitingFunction(toField.data(),
                                    newMessages);
                                                                                        binaryStatus.remove(0);

                            }
                        }

                        // if detailed, media specific message status
                        else if(mpDetailedMessageWaitingFunction)
                        {
                            UtlString status = nv->getValue();

                            // The name of the header is the message media type
                            // parse the message counts out
                            // The format is generically:
                            // <mediaType>: <newCount/oldCount> [<flag>: <totalCount>[/<urgentCount>]] ...
                            // Where there may be 0 or more flags

                            int newMessages = -1;
                            int oldMessages = -1;
                            int totalUntouched = -1;
                            int urgentUntouched = -1;
                            int totalSkipped = -1;
                            int urgentSkipped = -1;
                            int totalFlagged = -1;
                            int urgentFlagged = -1;
                            int totalRead = -1;
                            int urgentRead = -1;
                            int totalAnswered = -1;
                            int urgentAnswered = -1;
                            int totalDeleted = -1;
                            int urgentDeleted = -1;

                            // Get the number of new messages
                            UtlString numberString;
                            UtlBoolean absoluteValues = TRUE;
                            NameValueTokenizer::getSubField(status.data(), 0,
                                                       " \t/;", &numberString);

                            // If there is a + or - the numbers are in delta values
                            if(numberString.data()[0] == '+' ||
                                numberString.data()[0] == '-')
                            {
                                absoluteValues = FALSE;
                                oldMessages = 0;
                                totalUntouched = 0;
                                urgentUntouched = 0;
                                totalSkipped = 0;
                                urgentSkipped = 0;
                                totalFlagged = 0;
                                urgentFlagged = 0;
                                totalRead = 0;
                                urgentRead = 0;
                                totalAnswered = 0;
                                urgentAnswered = 0;
                                totalDeleted = 0;
                                urgentDeleted = 0;
                            }

                            if(!numberString.isNull())
                            {
                               newMessages = atoi(numberString.data());
                            }

                            // Get the number of old messages
                            NameValueTokenizer::getSubField(status.data(), 1,
                                                       " \t/;", &numberString);
                            if(!numberString.isNull())
                                oldMessages = atoi(numberString.data());

                            int parameterIndex = 2;
                            UtlString flag;
                            NameValueTokenizer::getSubField(status.data(), parameterIndex,
                                                       " \t/:;", &flag);
                            flag.toLower();

                            // Loop through the flags
                            do
                            {
                                osPrintf("SipNotifyStateTask::handleMessage flag=\'%s\'\n",
                                    flag.data());
                                switch(flag.data()[0])
                                {
                                case 'u':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalUntouched, urgentUntouched);
                                    break;

                                case 's':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalSkipped, urgentSkipped);
                                    break;

                                case 'f':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalFlagged, urgentFlagged);
                                    break;

                                case 'r':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalRead, urgentRead);
                                    break;

                                case 'a':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalAnswered, urgentAnswered);
                                    break;

                                case 'd':
                                    parameterIndex += getStatusTotalUrgent(status,
                                        absoluteValues,
                                        parameterIndex + 1,
                                        totalDeleted, urgentDeleted);
                                    break;

                                default:
                                    break;
                                }

                                parameterIndex+=2;
                                NameValueTokenizer::getSubField(status.data(), parameterIndex,
                                                       " \t/:;", &flag);
                            }
                            while(!flag.isNull());

                            // Make the MWI call back for the detailed status
                            mpDetailedMessageWaitingFunction(toField.data(),
                                nv->data(), // message media type
                                absoluteValues,
                                newMessages,
                                oldMessages,
                                totalUntouched,
                                urgentUntouched,
                                totalSkipped,
                                urgentSkipped,
                                totalFlagged,
                                urgentFlagged,
                                totalRead,
                                urgentRead,
                                totalAnswered,
                                urgentAnswered,
                                totalDeleted,
                                urgentDeleted);

                                                                status.remove(0);
                                                                numberString.remove(0);
                                                                flag.remove(0);
                                }
                                delete nv;
                                        }
                                        // Send a 200 OK response
                                        if(mpSipUserAgent)
                                        {
                                                SipMessage notifyOkResponse;
                                                notifyOkResponse.setOkResponseData(sipMessage);
                                                mpSipUserAgent->send(notifyOkResponse);
                                        }
                }

                // If this is a message waiting indication
                // Looking for header: "Event: simple-message-summary"
                // This is for the draft-mahy-sip-message-waiting-01.txt version of the draft.
                else if(((eventField.index(SIP_EVENT_SIMPLE_MESSAGE_SUMMARY, 0, UtlString::ignoreCase) == 0) ||
                                        (eventField.index(SIP_EVENT_MESSAGE_SUMMARY, 0, UtlString::ignoreCase) == 0)) &&
                                        (contentType.index(CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY, 0, UtlString::ignoreCase) == 0 &&
                        (httpBody = sipMessage->getBody()) != NULL))
                {
                    osPrintf("SipNotifyStateTask::handleMessage found simple-message-summary or message-summary event\n");
                    osPrintf("SipNotifyStateTask::handleMessage got application/simple-message-summary body\n");
                    UtlDList bodyHeaderNameValues;
                    const char* bodyBytes;
                    int bodyLength;
                    httpBody->getBytes(&bodyBytes, &bodyLength);
                    HttpMessage::parseHeaders(bodyBytes, bodyLength,
                          bodyHeaderNameValues);

                    UtlBoolean bBinaryNotification = true ;
                    UtlBoolean bNewMessages = false ;
                    UtlString  strMediaType ;
                    int iNewMessages = 0 ;
                    int iOldMessages = 0 ;

                    UtlString toField;
                    sipMessage->getToField(&toField);

                    // Loop through the body header nameValue pairs
                    UtlDListIterator iterator(bodyHeaderNameValues);
                        NameValuePair* nv;
                    while ((nv = (NameValuePair*) iterator()))
                        {
                        HttpMessage::cannonizeToken(*nv);

                        // if binary message waiting status
                        osPrintf("SipNotifyStateTask::handleMessage body field name: %s value: %s\n",
                            nv->data(), nv->getValue());
                        // Check for either a "Messages-Waiting" or a "Message-Waiting" field.
                        // "Messages-Waiting" is what's specified in the Internet draft;
                        // "Message-Waiting" is needed for backward compatibility.
                        if((strcmp(nv->data(), "Messages-Waiting") == 0) ||
                           (strcmp(nv->data(), "Message-Waiting") == 0))
                        {
                            UtlString binaryStatus = nv->getValue();
                            binaryStatus.toLower();
                            if (binaryStatus.compareTo("yes") == 0)
                                bNewMessages = TRUE ;
                            continue ;
                        }

                        if(strcmp(nv->data(), "Voice-Message") == 0)
                        {
                            UtlString numberString ;

                            UtlString status = nv->getValue() ;
                            status.toLower() ;

                            // Parse number of new messages
                            NameValueTokenizer::getSubField(status.data(), 0,
                                                       " \t/;()", &numberString);
                            iNewMessages = atoi(numberString.data());

                            // Parse number of old messages
                            NameValueTokenizer::getSubField(status.data(), 1,
                                                       " \t/;()", &numberString);
                            iOldMessages = atoi(numberString.data());

                            strMediaType = nv->data() ;
                            bBinaryNotification = false ;
                            continue ;
                        }
                    }

                    // If binary, notifiy the binary listener, otherwise
                    // notify either binary or detailed
                    if (bBinaryNotification) {
                        // Notify Binary Listener
                        if (mpBinaryMessageWaitingFunction != NULL) {
                            mpBinaryMessageWaitingFunction(toField.data(),
                                    bNewMessages) ;
                        }
                    } else {
                        if (mpDetailedMessageWaitingFunction != NULL) {
                            mpDetailedMessageWaitingFunction(toField.data(),
                                strMediaType.data(),
                                true,
                                iNewMessages,
                                iOldMessages,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0);
                        }
                    }

                    // Send a 200 OK response
                    if(mpSipUserAgent)
                    {
                        SipMessage notifyOkResponse;
                        notifyOkResponse.setOkResponseData(sipMessage);
                        mpSipUserAgent->send(notifyOkResponse);
                    }
                }

                // If this is a resync notification
                // Looking for header: "Event: check-sync"
                else if (eventField.index(SIP_EVENT_CHECK_SYNC) == 0)
                {
                   handleCheckSyncEvent(sipMessage) ;
                }
                else
                {
                   osPrintf("Unhandled NOTIFY event type: %s\n", eventField.data()) ;
                   syslog(FAC_SIP, PRI_WARNING,
                        "Unhandled NOTIFY event type: %s", eventField.data()) ;
                }

                eventIndex++;
                                    eventField.remove(0);
             }
        }
        method.remove(0);
    }
    else if (eventMessage.getMsgType() == OsMsg::OS_EVENT)
    {
        OsEventMsg* pEventMsg = (OsEventMsg*) &eventMessage;
        int userData ;
        int eventData ;
        pEventMsg->getEventData(eventData);
        pEventMsg->getUserData(userData);

        if (eventData == ((int) mpRunScriptTimer))
        {
            // Restart Event: handle the event and clean up the temp data
            struct tagRunScriptInfo* pInfo =
                    (struct tagRunScriptInfo*) userData ;
            if (pInfo != NULL)
            {
                // Ideally remove this dependency on phone library
                assert("OPENDEV PORT: Unexpected code path for softphone");
                delete pInfo ;
            }
        }
    }

    return(TRUE);
}

/* ============================ ACCESSORS ================================= */

void SipNotifyStateTask::setRebootFunction(void (*rebootNotifyFunction)())
{
    mpRebootFunction = rebootNotifyFunction;
}

void SipNotifyStateTask::setBinaryMessageWaitingFunction(void (*binaryMessageWaitingFunc)(const char* toUrl,
                                        UtlBoolean newMessages))
{
    mpBinaryMessageWaitingFunction = binaryMessageWaitingFunc;
}

void SipNotifyStateTask::setDetailMessageWaitingFunction(void (*detailMessageWaitingFunction)(
                                      const char* toUrl,
                                      const char* messageMediaType,
                                      UtlBoolean absoluteValues,
                                      int totalNewMessages,
                                      int totalOldMessages,
                                      int totalUntouchedMessages,
                                      int urgentUntouchedMessages,
                                      int totalSkippedMessages,
                                      int urgentSkippedMessages,
                                      int totalFlaggedMessages,
                                      int urgentFlaggedMessages,
                                      int totalReadMessages,
                                      int urgentReadMessages,
                                      int totalAnsweredMessages,
                                      int urgentAnsweredMessages,
                                      int totalDeletedMessages,
                                      int urgentDeletedMessages))
{
    mpDetailedMessageWaitingFunction = detailMessageWaitingFunction;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus SipNotifyStateTask::handleCheckSyncEvent(const SipMessage* source)
{
   OsStatus status = OS_SUCCESS ;   // Regardless of our success, we processed the event
   UtlString *pContent = NULL ;

   // First and foremost; send an ack back to the requestor
   if(mpSipUserAgent)
   {
      SipMessage notifyOkResponse;
      notifyOkResponse.setOkResponseData(source);
      mpSipUserAgent->send(notifyOkResponse);
   }

   // Figure out if a valid script file was included
   const HttpBody* body = source->getBody() ;
   if (body != NULL)
   {
      if (strcasecmp(body->getContentType(), CONTENT_TYPE_XPRESSA_SCRIPT) == 0)
      {
         pContent = new UtlString() ;

         int length = 0 ;
         body->getBytes(pContent, &length) ;
         if (pContent->isNull())
         {
            delete pContent ;
            pContent = NULL ;
         }
      }
   }


   // Execute according to policy
   if ((mCheckSyncPolicy.compareTo("SCRIPT", UtlString::ignoreCase) == 0) &&
         (pContent != NULL))
   {
#ifdef _VXWORKS


      pContent = NULL ; // null out pContent so that we don't delete it now.
#endif
   }
   else if (  (mCheckSyncPolicy.compareTo("ENABLE", UtlString::ignoreCase) == 0) ||
         (mCheckSyncPolicy.compareTo("REBOOT", UtlString::ignoreCase) == 0) ||
         (mCheckSyncPolicy.compareTo("SCRIPT", UtlString::ignoreCase) == 0))
   {
      //
      // Perform a simple reboot
      //

      syslog(FAC_UPGRADE, PRI_NOTICE,
            "Rebooting in response to a check-sync event") ;

      if (mpRebootFunction != NULL)
      {
         // Call the reboot handler
         mpRebootFunction() ;
      }
   }
   else
   {
      syslog(FAC_UPGRADE, PRI_NOTICE,
            "Ignoring check-sync; Setting not enabled") ;
      //
      // Do nothing; check-sync is disabled.
      //
   }


   if (pContent != NULL)
   {
      delete pContent ;
      pContent = NULL ;
   }

   return status ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
UtlBoolean SipNotifyStateTask::getStatusTotalUrgent(const char* status,
                                                   UtlBoolean absoluteValues,
                                                 int parameterIndex,
                                                 int& total,
                                                 int& urgent)
{
    UtlString numberString;
    UtlBoolean urgentFound = FALSE;

    // Get the total for this type of status
    NameValueTokenizer::getSubField(status, parameterIndex,
                   " \t/;:", &numberString);
    if(!numberString.isNull())
    {
        total = atoi(numberString.data());

        // Get the total for this type of status
        NameValueTokenizer::getSubField(status, parameterIndex + 1,
                       " \t/;:", &numberString);
        if(!numberString.isNull() &&
            (isdigit(numberString.data()[0]) ||
                numberString.data()[0] == '+' ||
                numberString.data()[0] == '-'))
        {
            urgent = atoi(numberString.data());
            urgentFound = TRUE;
        }
        // It is not a digit so it must be a flag
        else if(absoluteValues)
            urgent = -1;
        else
            urgent = 0;
    }
    else if(absoluteValues)
    {
        total = -1;
        urgent = -1;
    }
    else
    {
        total = 0;
        urgent = 0;
    }

        numberString.remove(0);
    // true/false the urgent count was set
    return(urgentFound);
}


/* ============================ FUNCTIONS ================================= */
