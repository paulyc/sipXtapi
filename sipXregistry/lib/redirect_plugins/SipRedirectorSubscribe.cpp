// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "sipdb/ResultSet.h"
#include "SipRedirectorSubscribe.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorSubscribe(instanceName);
}

// Constructor
SipRedirectorSubscribe::SipRedirectorSubscribe(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
}

// Destructor
SipRedirectorSubscribe::~SipRedirectorSubscribe()
{
}

// Initializer
OsStatus
SipRedirectorSubscribe::initialize(const UtlHashMap& configParameters,
                                   OsConfigDb& configDb,
                                   SipUserAgent* pSipUserAgent,
                                   int redirectorNo)
{
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorSubscribe::finalize()
{
}

RedirectPlugin::LookUpStatus
SipRedirectorSubscribe::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   SipMessage& response,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage)
{
   // Return immediately if the method is not SUBSCRIBE.
   if (method.compareTo(SIP_SUBSCRIBE_METHOD, UtlString::ignoreCase) != 0)
   {
      return RedirectPlugin::LOOKUP_SUCCESS;
   }

   UtlString thisContact;
   for (int contactNum = 0;
        response.getContactField(contactNum, thisContact);
        contactNum++)
   {
      Url contactUri(thisContact);
      UtlString qValue;

      // Check if the contact has a q value.
      if (contactUri.getFieldParameter(SIP_Q_FIELD, qValue))
      {
         // If so, remove it.
         contactUri.removeFieldParameter(SIP_Q_FIELD);
         UtlString contactUriString(contactUri.toString());
         response.setContactField(contactUriString, contactNum);

         OsSysLog::add(FAC_SIP, PRI_NOTICE,
                       "SipRedirectorSubscribe::lookUp Remove q value "
                       "'%s' from '%s'",
                       qValue.data(), contactUriString.data());
      }
   } // for all contacts

   return RedirectPlugin::LOOKUP_SUCCESS;
}
