//
// Copyright (C) 2006-2013 SIPez LLC. All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsTask_h_
#define _OsTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsgQ.h>
#include <os/OsMutex.h>
#include <os/OsStatus.h>
#include <os/OsSysLog.h>
#include <os/OsAtomics.h>
#include <os/OsTaskId.h>

// DEFINES
#define OSTASK_STACK_SIZE_1M 1024*1024
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsTime;

//:Task abstraction
// A task represents a thread of execution. All tasks run within the same
// address space but have their own stack and program counter. Tasks may be
// created and deleted dynamically.
//
// <p>Users create tasks by:
// <ol><li>Deriving a new class based on OsTask or one of its descendants,
//    and overriding the run() method in the derived class.</li>
// <li>Calling the constructor for the derived class.</li>
// <li>Invoking the start() method for the derived class.  This creates
//    the corresponding low-level OS task and associates it with the class.</li>
// </ol>
// <p>Note: Many of the methods in this class are only applicable once the
// start() method for the object has been called and the corresponding
// low-level task has been created.  Accordingly, before a successful call
// to start(), most of the methods in this class return the
// OS_TASK_NOT_STARTED status.
//
// <p>Note: carefully read documentation for ackShutdown() and waitUntilShutDown()
//           methods, or you may get deadlocks.

class OsTaskBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_OPTIONS;        // default task execution options
   static const int DEF_PRIO;           // default task priority
   static const int DEF_STACKSIZE;      // default task stack size
   static const UtlString TASK_PREFIX;   // prefix for OsTask names stored in
                                        //  the name database
   static const UtlString TASKID_PREFIX; // prefix for taskID stored in the
                                        //  name database.

   static OsAtomicInt taskCount;

   enum TaskState
   {
      UNINITIALIZED,   // no low-level task, no name DB entries
      STARTED,         // low-level task and name DB entries exist
      SHUTTING_DOWN,   // requested low-level task shutdown
      SHUT_DOWN        // no low-level task, name DB entries still exist
   };

     //!enumcode: UNINITIALIZED - no low-level task, no name DB entries
     //!enumcode: STARTED  - low-level task and name DB entries exist
     //!enumcode: SHUTTING_DOWN - requested low-level task shutdown
     //!enumcode: SHUT_DOWN - no low-level task, name DB entries still exist

/* ============================ CREATORS ================================== */

   virtual OsStatus deleteForce(void) = 0;
     //:Delete the task even if the task is protected from deletion
     // After calling this method, the user will still need to delete the
     // corresponding OsTask object to reclaim its storage.

/* ============================ MANIPULATORS ============================== */

   virtual void requestShutdown(void);
     //:Request a task shutdown
     // The run() method of the derived class is expected to call the
     // isShuttingDown() method to detect when a task shutdown has been
     // requested.  After a task has been shut down, it may be restarted
     // by calling the start() method.

   virtual UtlBoolean restart(void) = 0;
     //:Restart the task
     // The task is first terminated, and then reinitialized with the same
     // name, priority, options, stack size, original entry point, and
     // parameters it had when it was terminated.
     // Return TRUE if the restart of the task is successful.

   virtual OsStatus resume(void) = 0;
     //:Resume the task
     // This routine resumes the task. The task suspension is cleared, and
     // the task operates in the remaining state.

   virtual UtlBoolean start(void) = 0;
     //:Spawn a new task and invoke its run() method.
     // Return TRUE if the spawning of the new task is successful.
     // Return FALSE if the task spawn fails or if the task has already
     // been started.

   virtual OsStatus suspend(void) = 0;
     //:Suspend the task
     // This routine suspends the task. Suspension is additive: thus, tasks
     // can be delayed and suspended, or pended and suspended. Suspended,
     // delayed tasks whose delays expire remain suspended. Likewise,
     // suspended, pended tasks that unblock remain suspended only.

   virtual OsStatus setErrno(int errno) = 0;
     //:Set the errno status for the task

   virtual OsStatus setOptions(int options) = 0;
     //:Set the execution options for the task
     // The only option that can be changed after a task has been created
     // is whether to allow breakpoint debugging.

   virtual OsStatus setPriority(int priority) = 0;
     //:Set the priority of the task
     // Priorities range from 0, the highest priority, to 255, the lowest
     // priority.

   virtual void setUserData(int data);
     //:Set the userData for the task.
     // The class does not use this information itself, but merely stores
     // it on behalf of the caller.

   virtual OsStatus varAdd(int* pVar) = 0;
     //:Add a task variable to the task
     // This routine adds a specified variable pVar (4-byte memory
     // location) to its task's context. After calling this routine, the
     // variable is private to the task. The task can access and modify
     // the variable, but the modifications are not visible to other tasks,
     // and other tasks' modifications to that variable do not affect the
     // value seen by the task. This is accomplished by saving and restoring
     // the variable's value each time a task switch occurs to or from the
     // calling task.

   virtual OsStatus varDelete(int* pVar) = 0;
     //:Remove a task variable from the task
     // This routine removes a specified task variable, pVar, from its
     // task's context. The private value of that variable is lost.

   virtual OsStatus varSet(int* pVar, int value) = 0;
     //:Set the value of a private task variable
     // This routine sets the private value of the task variable for a
     // specified task. The specified task is usually not the calling task,
     // which can set its private value by directly modifying the variable.
     // This routine is provided primarily for debugging purposes.

   virtual OsStatus syslog(const OsSysLogFacility facility,
                           const OsSysLogPriority priority,
                           const char*            format,
                                                  ...)
#ifdef __GNUC__
       // with the -Wformat switch, this enables format string checking
       __attribute__ ((format(printf, 4, 5)))
#endif
       ;

     //:Adds a syslog entry to the system logger.
     //
     //!param: facility - Defines the facility responsible for adding the
     //        event.  See the OsSysLogFacility for more information.
     //!param: priority - Defines the priority of the event.  See
     //        OsSysLogPriority for more information.

   static OsStatus delay(const int milliSecs);
     //:Delay a task from executing for the specified number of milliseconds
     // This routine causes the calling task to relinquish the CPU for the
     // duration specified. This is commonly referred to as manual
     // rescheduling, but it is also useful when waiting for some external
     // condition that does not have an interrupt associated with it.

   static OsStatus safe(void);
     //:Make the calling task safe from deletion
     // This routine protects the calling task from deletion. Tasks that
     // attempt to delete a protected task will block until the task is
     // made unsafe, using unsafe(). When a task becomes unsafe, the
     // deleter will be unblocked and allowed to delete the task.
     // The safe() primitive utilizes a count to keep track of
     // nested calls for task protection. When nesting occurs,
     // the task becomes unsafe only after the outermost unsafe()
     // is executed.

   static OsStatus unsafe(void);
     //:Make the calling task unsafe from deletion
     // This routine removes the calling task's protection from deletion.
     // Tasks that attempt to delete a protected task will block until the
     // task is unsafe. When a task becomes unsafe, the deleter will be
     // unblocked and allowed to delete the task.
     // The unsafe() primitive utilizes a count to keep track of nested
     // calls for task protection. When nesting occurs, the task becomes
     // unsafe only after the outermost unsafe() is executed.

   static void yield(void);
     //:Yield the CPU if a task of equal or higher priority is ready to run

/* ============================ ACCESSORS ================================= */

   static OsTaskBase* getCurrentTask(void);
     //:Return a pointer to the OsTask object for the currently executing task
     // Return NULL if none exists.

   static OsStatus getCurrentTaskId(int &rid);
     //:Return an Id of the currently executing task
     // This Id is unique within the current process, but not necessarily
     // over the entire host.
     // Any two simultaneous executions that share their memory space
     // will have different values from getCurrentTaskId().

   static OsTaskBase* getTaskByName(const UtlString& taskName);
     //:Return a pointer to the OsTask object corresponding to the named task
     // Return NULL if there is no task object with that name.

   static OsTaskBase* getTaskById(const int taskId);
     //:Return a pointer to the OsTask object corresponding to taskId
     // Return NULL is there is no task object with that id.

   virtual void* getArg(void);
     //:Get the void* value passed as an argument to the task

   virtual OsStatus getErrno(int& rErrno) = 0;
     //:Get the errno status for the task

   virtual const UtlString& getName(void);
     //:Get the name associated with the task

   virtual int getOptions(void) = 0;
     //:Return the execution options for the task

   virtual OsStatus getPriority(int& rPriority) = 0;
     //:Return the priority of the task

   virtual int getUserData(void);
     //:Return the userData for the task.

   virtual OsStatus varGet(void) = 0;
     //:Get the value of a task variable
     // This routine returns the private value of a task variable for its
     // task. The task is usually not the calling task, which can get its
     // private value by directly accessing the variable. This routine is
     // provided primarily for debugging purposes.

/* ============================ INQUIRY =================================== */

   virtual OsStatus id(OsTaskId_t &rId) = 0;
     //:Get the task ID for this task

   virtual UtlBoolean isReady(void);
     //:Check if the task is running
     // Return TRUE is the task is started and not suspended, otherwise FALSE.

   virtual UtlBoolean isShutDown(void);
     //:Return TRUE if a task shutdown has been requested and acknowledged

   virtual UtlBoolean isShuttingDown(void);
     //:Return TRUE if a task shutdown has been requested but not acknowledged

   virtual UtlBoolean isStarted(void);
     //:Return TRUE if the task has been started (and has not been shut down)

   virtual UtlBoolean isSuspended(void) = 0;
     //:Check if the task is suspended
     // Return TRUE is the task is suspended, otherwise FALSE.

   virtual UtlBoolean isUnInitialized(void);
     //:Return TRUE if a task is un-initialized


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsMutex   mDataGuard;  // Mutex guard to protect the OsTask internal data
   UtlString  mName;       // global name associated with the task

   volatile TaskState mState;      // Task object state

   OsTaskBase(const UtlString& name,
              void* pArg,
              const int priority,
              const int options,
              const int stackSize);
     //:Constructor

   virtual
   ~OsTaskBase();
     //:Destructor

   virtual int run(void* pArg) = 0;
     //:The entry point for the task
     // Derive new tasks as subclasses of OsTask, overriding this method.

   virtual UtlBoolean waitUntilShutDown(int milliSecToWait = 20000);
    //: Wait until the task is shut down and the run method has exited.
    // Most subclasses of OsTask should call this method in
    // the destructor before deleting any members which are
    // accessed by the run method.

   virtual void ackShutdown(void);
     //:Acknowledge a shutdown request
     // This method should only be called by OS specific derived, concrete thread
     // classes in the threadEntry() static method that invoked the run method.
     // The point of this method is to signal that no member variables are accessed
     // by the run() method and that the run method has exited such that it is now
     // safe to delete this class.  For this reason this method must be called
     // AFTER run exits (not within).  Related to this handshake/signaling is
     // the waitUntilShutDown() method.  The waitUntilShutDown() MUST be called by
     // the destructors of all classes derived from OsTask.  waitUntilShutDown()
     // should be the first thing invoked in the destructor before any members
     // are destructed.


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   void*     mpArg;       // argument passed to the task
   int       mUserData;   // data stored on behalf of the user.  This data
                          //  is read/written via getUserData()/setUserData()

   OsTaskBase(const OsTaskBase& rOsTask);
     //:Copy constructor (not implemented for this class)

   OsTaskBase& operator=(const OsTaskBase& rhs);
     //:Assignment operator (not implemented for this class)


};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsTask)
// with the OS-dependent realization of that type (e.g., OsTaskWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsTaskWnt.h"
   typedef class OsTaskWnt OsTask;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsTaskVxw.h"
   typedef class OsTaskVxw OsTask;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsTaskLinux.h"
   typedef class OsTaskLinux OsTask;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsTask_h_
