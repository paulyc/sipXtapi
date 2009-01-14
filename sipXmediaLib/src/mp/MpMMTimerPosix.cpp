//  
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <sergey DOT kostanbaev AT SIPez DOT com>

// SYSTEM INCLUDES
#include <os/OsIntTypes.h>
#include <os/OsSysLog.h>

// APPLICATION INCLUDES
#include "mp/MpMMTimerPosix.h"

#define TIMER_SIGNAL    SIGRTMIN

const char * const MpMMTimerPosix::TYPE = "POSIX Timer";

static MpMMTimerPosix::PosixSignalReg sPosixTimerReg(TIMER_SIGNAL, MpMMTimerPosix::signalHandler);

MpMMTimerPosix::MpMMTimerPosix(MpMMTimer::MMTimerType type)
: MpMMTimer(type)
, mbTimerStarted(FALSE)
{
   if (mTimerType == Linear)
   {
      int res = sem_init(&mSyncSemaphore, 0, 0);
      assert( res == 0);
   }

}

MpMMTimerPosix::~MpMMTimerPosix()
{
   if (mbTimerStarted)
      stop();

   if (mTimerType == Linear)
   {
      sem_destroy(&mSyncSemaphore);
   }
}

OsStatus MpMMTimerPosix::run(unsigned usecPeriodic)
{
  OsStatus status = OS_SUCCESS;

   if(mbTimerStarted)
   {
      return OS_INVALID_STATE;
   }

   struct sigevent evnt;

   evnt.sigev_notify = SIGEV_SIGNAL;
   evnt.sigev_value.sival_ptr = this;
   evnt.sigev_signo = TIMER_SIGNAL;

   int res = timer_create(CLOCK_REALTIME, &evnt, &mTimer);
   if (res != 0)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "Couldn't create POSIX timer");

      return OS_INVALID_ARGUMENT;
   }

   struct itimerspec ts;
   ts.it_value.tv_sec = usecPeriodic / 1000000;
   ts.it_value.tv_nsec = (usecPeriodic % 1000000) * 1000;
   ts.it_interval.tv_sec = usecPeriodic / 1000000;
   ts.it_interval.tv_nsec = (usecPeriodic % 1000000) * 1000;

//CLOCK_REALTIME //TIMER_ABSTIME
   res = timer_settime(mTimer, CLOCK_REALTIME, &ts, NULL);
   if (res != 0)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "Couldn't set POSIX timer with %d us period", 
         usecPeriodic);

      return OS_INVALID_ARGUMENT;
   }

   mbTimerStarted = TRUE;
   return status;
}

OsStatus MpMMTimerPosix::stop()
{
   if (mbTimerStarted)
   {
      struct itimerspec ts;
      ts.it_value.tv_sec = 0;
      ts.it_value.tv_nsec = 0;
      ts.it_interval.tv_sec = 0;
      ts.it_interval.tv_nsec = 0;
      timer_settime(mTimer, CLOCK_REALTIME, &ts, NULL);

      int res = timer_delete(mTimer);
      assert (res == 0);

      mbTimerStarted = FALSE;

      return OS_SUCCESS;
   }
   else
      return OS_INVALID_STATE;
}


OsStatus MpMMTimerPosix::getPeriodRange(unsigned* pMinUSecs, 
                                        unsigned* pMaxUSecs)
{
   OsStatus status = OS_FAILED;

   if (pMaxUSecs)
      *pMaxUSecs = INT_MAX;

   if (pMinUSecs)
      return getResolution(*pMinUSecs);

   return OS_SUCCESS;
}

OsStatus MpMMTimerPosix::getResolution(unsigned& resolution)
{
   struct timespec ts;
   int res = clock_getres(CLOCK_REALTIME, &ts);
   if (res != 0)
     return OS_FAILED;

   resolution = ts.tv_nsec / 1000;
   if (resolution == 0)
      resolution++;

   return OS_SUCCESS;
}

OsStatus MpMMTimerPosix::waitForNextTick()
{
   if(mTimerType != Linear)
   {
      return OS_INVALID_STATE;
   }

   if(mbTimerStarted == FALSE)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "MpMMTimerPosix::waitForNextTick "
                    "- Timer not started or timer not initialized!");
      return OS_FAILED;
   }


   int res = sem_wait(&mSyncSemaphore);
   if (res == 0)
      return OS_SUCCESS;

   return OS_FAILED;
}


OsTime MpMMTimerPosix::getAbsFireTime() const
{
   return OsTime::OS_INFINITY;
}

OsStatus MpMMTimerPosix::setNotification(OsNotification* notification)
{ 
   mpNotification = notification;
   return OS_SUCCESS;
}

void MpMMTimerPosix::callback()
{
   if (mTimerType == Notification)
   {
      if(mpNotification != NULL)
      {
         // Then signal it to indicate a tick.
         mpNotification->signal((intptr_t)this);
      }
   }
   else
   {
      sem_post(&mSyncSemaphore);
   }
}

void MpMMTimerPosix::signalHandler(int signum, siginfo_t *siginfo, void *context)
{
   assert(siginfo->si_signo == TIMER_SIGNAL);
   //assert(siginfo->si_code == SI_TIMER);
   
   MpMMTimerPosix* object = (MpMMTimerPosix*)siginfo->si_ptr;
   object->callback();
}

MpMMTimerPosix::PosixSignalReg::PosixSignalReg(int sigNum, void (*sa)(int, siginfo_t *, void *))
: mSigNum(sigNum)
{
   sigset_t mask;
   sigemptyset(&mask);
   sigaddset(&mask, mSigNum);

   struct sigaction act;
   act.sa_sigaction = sa;
   act.sa_flags = SA_SIGINFO | SA_RESTART;
   act.sa_mask = mask;

   int res = sigaction(mSigNum, &act, &mOldAction);
   assert (res == 0);
}

MpMMTimerPosix::PosixSignalReg::~PosixSignalReg()
{
   int res = sigaction(mSigNum, &mOldAction, NULL);
   assert (res == 0);
}
