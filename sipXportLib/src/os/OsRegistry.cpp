//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2009 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"
#include "os/OsRegistry.h"

#ifdef WIN32
#include <windows.h>
#endif

// APPLICATION INCLUDES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */



/* ============================ CREATORS ================================== */
// Constructor
OsRegistry::OsRegistry()
{
}

// Destructor
OsRegistry::~OsRegistry()
{
}


/* ============================ MANIPULATORS ============================== */
bool OsRegistry::writeInteger(UtlString keyPath, UtlString key, int value)
{
   bool bRet = false;

#ifdef WIN32   
   HKEY hKey;

   DWORD err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,   // handle to open key
              keyPath,              // subkey name
              0,                    // reserved
              KEY_WRITE,            // security access mask
              &hKey                 // handle to open key
              );

    DWORD dwDisposition = NULL;
    if (err != ERROR_SUCCESS)
    {
        err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
            keyPath, 
            0,
            NULL, 
            REG_OPTION_NON_VOLATILE, 
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition);
    }
    DWORD dwValue = value;
    if (ERROR_SUCCESS == err)
    {
        err = RegSetValueEx(hKey,
            key,
            0,
            REG_DWORD,
            (const BYTE*)&dwValue,
            sizeof(&dwValue));
        if (ERROR_SUCCESS == err)
        {
            bRet = true;
        }
    }
   RegCloseKey(hKey);
#endif
   return bRet;
}

/* ============================ ACCESSORS ================================= */
bool OsRegistry::readInteger(UtlString keyPath, UtlString key, int& value)
{
   bool bRet = false;
    
#ifdef _WIN32
   HKEY hKey;
   DWORD    cbData;
   DWORD    dataType;
   DWORD    dwValue;
      
   DWORD err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,   // handle to open key
              keyPath,              // subkey name
              0,                    // reserved
              KEY_READ,             // security access mask
              &hKey                 // handle to open key
              );

   if (err == ERROR_SUCCESS)
   {
      cbData = sizeof(DWORD);
      dataType = REG_DWORD;
      
      err = RegQueryValueEx(
                  hKey,                      // handle to key
                  key,                       // value name
                  0,                         // reserved
                  &dataType,                 // type buffer
                  (LPBYTE)&dwValue,          // data buffer
                  &cbData);                  // size of data buffer

      if (err == ERROR_SUCCESS)
      {
         value = dwValue;
         bRet = true;
      }

      RegCloseKey(hKey);
   }
#endif
   
   return bRet;
}
