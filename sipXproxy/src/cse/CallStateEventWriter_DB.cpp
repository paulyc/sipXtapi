//
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "CallStateEventWriter_DB.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
static const char* ModuleName = 
   "CallStateEventWriter_DB";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/// Instantiate an event builder and set the observer name for its events
CallStateEventWriter_DB::CallStateEventWriter_DB(const char* logName, 
                                                 const char* logLocation,
                                                 const char* logUserName,
                                                 const char* logDriver,
                                                 const char* logPassword)
                  : CallStateEventWriter(CallStateEventWriter::CseLogDatabase, logName),
                    mLogLocation(logLocation),
                    mLogUserName(logUserName),
                    mLogDriver(logDriver),
                    mLogPassword(logPassword),
                    mHandle(NULL)
{
   OsSysLog::add(FAC_CDR, PRI_DEBUG,
                 "%s::constructor Log type database", SIPX_SAFENULL(ModuleName));      
}

/// Destructor
CallStateEventWriter_DB::~CallStateEventWriter_DB()
{
   closeLog();
}

bool CallStateEventWriter_DB::openLog()
{
   bool bRet = false;
   
   if (!mbWriteable)
   {
      if ((mHandle=odbcConnect(mLogName, mLogLocation, mLogUserName, mLogDriver, mLogPassword)) != NULL)
      {
         OsSysLog::add(FAC_CDR, PRI_DEBUG,
                       "%s::openLog connected to database %s", SIPX_SAFENULL(ModuleName), SIPX_SAFENULL(mLogName.data())); 
         mbWriteable = true;
         bRet = true;
      }
      else
      {
         OsSysLog::add(FAC_CDR, PRI_ERR,
                       "%s::openLog connection to database %s failed", 
                       SIPX_SAFENULL(ModuleName), SIPX_SAFENULL(mLogName.data()));             
      }
   }
   else
   {
     OsSysLog::add(FAC_CDR, PRI_ERR,
                   "%s::openLog log %s already open", SIPX_SAFENULL(ModuleName), SIPX_SAFENULL(mLogName.data()));
   }
   return bRet;
}

bool CallStateEventWriter_DB::closeLog()
{
   bool bRet = false;
   
   if (mHandle)
   {
      odbcDisconnect(mHandle);
      mHandle = NULL;
   }
   mbWriteable = false;
   bRet = true;
      
   OsSysLog::add(FAC_CDR, PRI_DEBUG,
                 "%s::closeLog", SIPX_SAFENULL(ModuleName));      
   return bRet;
}

bool CallStateEventWriter_DB::writeLog(const char* event)
{
   bool bRet = false;

   if (mbWriteable)
   {   
      if (mHandle)
      {
         odbcExecute(mHandle, event);
         bRet = true;
      }
      OsSysLog::add(FAC_CDR, PRI_DEBUG,
                    "%s::writeLog", ModuleName);      
   }
   else
   {
      OsSysLog::add(FAC_CDR, PRI_ERR,
                    "%s::writeLog log %s not writeable", ModuleName, mLogName.data());            
   }
   return bRet;
}
