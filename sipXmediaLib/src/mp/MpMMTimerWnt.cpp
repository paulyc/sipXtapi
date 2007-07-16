//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

// SYSTEM INCLUDES
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <MMSystem.h>
#include <os/OsSysLog.h>

// APPLICATION INCLUDES
#include "mp/MpMMTimerWnt.h"

// APPLICATION INCLUDES

void CALLBACK 
MpMMTimerWnt::timeProcCallback(UINT uID, UINT uMsg, DWORD dwUser, 
                               DWORD dw1, DWORD dw2)
{
   MpMMTimerWnt* srcObj = (MpMMTimerWnt*)dwUser;
   if(srcObj == NULL) return;

   if(srcObj->getTimerType() != On_Fire)
   {
      // Don't do anything
      return;
   }
   else // if(srcObj->mTimerType == On_Fire)
   {
      BOOL firstFire = (srcObj->mbTimerFired == TRUE) ? FALSE : TRUE;
      srcObj->onFire(firstFire);
      srcObj->mbTimerFired = TRUE;
   }
}



MpMMTimerWnt::MpMMTimerWnt(MpMMTimer::MMTimerType type)
   : MpMMTimer(type)
   , mAlgorithm(Multimedia)  // This will get overwritten
   , mbTimerStarted(FALSE)
   , mPeriodMSec(0)
   , mbTimerFired(FALSE)
   , mEventHandle(0)
{
   // We only support linear timer, so only set initialized
   // true if it's constructed as linear.
   mbInitialized = (type == Linear) ? TRUE : FALSE;
   unsigned usResolution;
   if(getResolution(usResolution) != OS_SUCCESS)
   {
      mResolution = 0;
      mbInitialized = FALSE;
   }
   else
   {
      // Resolution we want is in ms, not us, so convert it..
      mResolution = usResolution/1000;
   }
}

MpMMTimerWnt::~MpMMTimerWnt()
{
   // if a timer is started, stop it.
   if(mbTimerStarted)
   {
      stop();
   }
}

OsStatus MpMMTimerWnt::run(unsigned usecPeriodic, 
                           unsigned uAlgorithm)
{
   if(mbInitialized == FALSE)
   {
      return OS_FAILED;
   }

   OsStatus stat = OS_FAILED;

   if((MMTimerWntAlgorithms)uAlgorithm == Multimedia)
   {
      mAlgorithm = Multimedia;
      stat = runMultimedia(usecPeriodic);
   }
   else
   {
      // We don't support other algorithms currently - only Multimedia timers.
      // Queue timers would be good to support though! they have less overhead,
      // and I believe similar high resolution.
      // see http://www.codeproject.com/system/timers_intro.asp
      stat = OS_INVALID_ARGUMENT;
   }

   return stat;
}

OsStatus MpMMTimerWnt::runMultimedia(unsigned usecPeriodic)
{
   OsStatus status = OS_SUCCESS;

   if(mbTimerStarted)
   {
      return OS_INVALID_STATE;
   }

   if(mTimerType != Linear)
   {
      // We only support linear.. if this was constructed with something
      // else then fail.
      return OS_INVALID_STATE;
   }


   unsigned minPeriodUsec = 0;
   unsigned maxPeriodUsec = 0;
   if(getPeriodRange(&minPeriodUsec, &maxPeriodUsec) != OS_SUCCESS)
   {
      // Couldn't get the timer resolution.
      return OS_FAILED;
   }

   // Windows Multimedia timers are only offer millisecond resolution...
   // If someone provides a usecPeriodic value requesting microsecond resolution,
   // then bitch.  Also bitch if the requested resolution is outside our 
   // resolution range.
   if((usecPeriodic % 1000 > 0) ||
      (usecPeriodic < minPeriodUsec || usecPeriodic > maxPeriodUsec))
   {
      return OS_INVALID_ARGUMENT;
   }

   unsigned msecPeriodic = usecPeriodic / 1000;

   // Set the timer resolution to the minimum possible,
   // not the requested period we'll run for, to increase accuracy.
   // (this used to be set to msecPeriodic)
   if(timeBeginPeriod(mResolution) != TIMERR_NOERROR)
   {
      status = OS_LIMIT_REACHED;
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "Couldn't set minimum MMTIMER begin period to %s ms", 
         mResolution);
   }
   else
   {
      mPeriodMSec = msecPeriodic;
      status = OS_SUCCESS;
      mbTimerStarted = TRUE;
   }

   if(mTimerType == Linear)
   {
      // Create the event.
      mEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
   }

   return status;
}

OsStatus MpMMTimerWnt::stop()
{
   if(mbInitialized == FALSE)
   {
      return OS_FAILED;
   }
   if(mbTimerStarted == FALSE)
   {
      // If the timer wasn't started, it's invalid to stop it.
      return OS_INVALID_STATE;
   }

   OsStatus stat = OS_FAILED;

   // See below else clause for why we assert here.
   assert(mAlgorithm == Multimedia);
   if(mAlgorithm == Multimedia)
   {
      stat = stopMultimedia();
   }
   else
   {
      // we don't support other algorithms, and anyway - we should never
      // reach here as the timer had to be started (and thus, have an implementation
      // for that algorithm) in order to get here.
      stat = OS_FAILED;
   }

   // Reset the timer fired state.
   mbTimerFired = FALSE;

   return stat;
}

OsStatus MpMMTimerWnt::stopMultimedia()
{
   // for multimedia timer, there must be a matching timeEndPeriod call to the
   // timeBeginPeriod call that was originally made.
   if(timeEndPeriod(mResolution) != TIMERR_NOERROR)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "Couldn't set minimum MMTIMER end period to %d ms", 
         mResolution);
      return OS_FAILED;
   }

   // Reset vars to stopped state.
   mPeriodMSec = 0;
   mbTimerStarted = FALSE;

   if(mTimerType == Linear)
   {
      // Close and reset the event handle.
      CloseHandle(mEventHandle);
      mEventHandle = 0;
   }

   return OS_SUCCESS;
}

void MpMMTimerWnt::onFire(UtlBoolean bFirstFire) 
{
}

OsStatus MpMMTimerWnt::waitForNextTick()
{
   if(mTimerType != Linear)
   {
      return OS_INVALID_STATE;
   }

   if(mbTimerStarted == FALSE || mbInitialized == FALSE)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "MpMMTimerWnt::waitForNextTick "
                    "- Timer not started or timer not initialized!");
      return OS_FAILED;
   }

   MMRESULT timerID = 
      timeSetEvent(mPeriodMSec, mResolution, (LPTIMECALLBACK)mEventHandle, 
                   NULL, TIME_CALLBACK_EVENT_PULSE);
   if(WaitForSingleObject(mEventHandle, mPeriodMSec) == WAIT_FAILED)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "MpMMTimerWnt - timer WaitForSingleObject failed with %s", 
                    GetLastError());
      // It failed, but it will just have waited till the period ended,
      // which is exactly what we want... so continue on..
   }
   if(timeKillEvent(timerID) == MMSYSERR_INVALPARAM)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "MpMMTimerWnt - No timer to kill!");
      return OS_FAILED;
   }
   return OS_SUCCESS;
}

int MpMMTimerWnt::getUSecSinceLastFire() const
{
   return -1;
}


int MpMMTimerWnt::getUSecDeltaExpectedFire() const
{
   return -1;
}

OsTime MpMMTimerWnt::getAbsFireTime() const
{
   return OsTime::OS_INFINITY;
}