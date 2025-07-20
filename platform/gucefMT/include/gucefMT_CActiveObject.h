/*
 *  gucefMT: GUCEF module providing multithreading solutions
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#define GUCEF_MT_CACTIVEOBJECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"             /* Simple types used */
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H */

#ifndef GUCEF_MT_GUCEFMT_MACROS_H
#include "gucefMT_macros.h"             /* often used gucef macros */
#define GUCEF_MT_GUCEFMT_MACROS_H
#endif /* GUCEF_MT_GUCEFMT_MACROS_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_MT_THREADSTATUS_H
#include "gucefMT_ThreadStatus.h"
#define GUCEF_MT_THREADSTATUS_H
#endif /* GUCEF_MT_THREADSTATUS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SThreadData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_MT_PUBLIC_CPP CActiveObject : public virtual MT::CILockable
{
    public:

    CActiveObject( void );

    CActiveObject( const CActiveObject& src );

    virtual ~CActiveObject();

    /**
     *  Flag that indicates whether the thread is active
     */
    bool IsActive( void ) const;

    /**
     *  Flag that indicates whether the thread has been asked to gracefully deactivate
     */
    bool IsDeactivationRequested( void ) const;

    /**
     *  Flag that indicates whether the thread has been asked to pause on the next available thread cycle.
     */
    bool IsPauseRequested( void ) const;

    /**
     *  Activates the object (if it is not active already).
     *  This operation will spawn a thread.
     */
    bool Activate( void* threadData = GUCEF_NULL           ,
                   const UInt32 cycleDelay = 25            ,
                   const UInt32 minimalCycleDeltaInMs = 10 );

    /**
     *  Deactivates the active object
     *  If 'force' is false then the thread will deactivate on the next available thread cycle.
     *  If 'force' is true then the thread will be killed immediatly regardless of whether it is busy processing a cycle.
     *
     *  Use caution when killing a thread because this is likely to cause memory leaks etc. and should not be used for normal operation.
     *  If a thread is killed in this manner OnTaskEnd() will still be called from the calling thread so you can place cleanup code there.
     */
    void Deactivate( bool force, bool callerShouldWait );

    /**
     *  Pauses the active object.
     *  If 'force' is false then the thread will pause on the next available thread cycle.
     *  If 'force' is true then the thread will be paused immediatly regardless of whether it is busy processing a cycle
     */
    void Pause( bool force );

    void Resume( void );

    UInt32 GetThreadID( void ) const;

    /**
     *  Sets the CPU affinity mask for the thread
     */
    bool SetCpuAffinityMask( UInt32 affinityMaskSize ,
                             void* affinityMask      );

    /**
     *  Gets the CPU affinity mask for the thread
     *  affinityMaskSize will contain the actual size of the mask in the given buffer
     */
    bool GetCpuAffinityMask( UInt32 affinityMaskBufferSize ,
                             void* affinityMask            ,
                             UInt32& affinityMaskSize      ) const;

    /**
     *  Convenience member function for setting a single CPU affinity
     *  Uses SetCpuAffinityMask()
     */
    bool SetCpuAffinityByCpuId( UInt32 cpuId );

    /**
     *  Convenience member function for getting a single CPU affinity
     *  Will return the last CPU the thread executed on.
     *  Whether the thread will stay on said CPU depends on the affinity mask at large
     */
    bool GetCpuAffinityByCpuId( UInt32& cpuId ) const;

    /**
     *  Blocks the calling thread until the thread has finished
     *  @return result codes from ThreadWait()
     */
    UInt32 WaitForThreadToFinish( Int32 timeoutInMs ) const;

    protected:

    virtual TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Startup routine for the task. You should return true if startup succeeded and the task can commence
     *  cycling.
     */
    virtual bool OnThreadStart( void* taskdata ) = 0;

    /**
     *  Called after a successfull call to OnThreadStart
     */
    virtual void OnThreadStarted( void* taskdata );

    /**
     *  Perorm all your main task work in this function.
     *  It will be called repeatedly until true is returned indicating that the task has been completed.
     *  Thus for ongoing tasks you can write this function to take care of a single interation of the task.
     */
    virtual bool OnThreadCycle( void* taskdata ) = 0;

    virtual void OnThreadPausedForcibly( void* taskdata );

    virtual void OnThreadResumed( void* taskdata );

    /**
     *  Last chance notification to decended classes of impending end of the tread
     *  If the 'willBeForced' flag is true the thread will be killed next
     *  Since this would be used in cases where the thread is misbehaving one should
     *  keep that in mind in the implementation of this callback
     */
    virtual void OnThreadEnding( void* taskdata    ,
                                 bool willBeForced );

    /**
     *  This is where all the cleanup should be done for task data
     *  Note that this member function will be called from within the spawned thread when ending gracefully
     *  but in the case of a forcefull termination of the spawned thread this member function will be called
     *  from the thread that triggered the forcefull termination.
     */
    virtual void OnThreadEnded( void* taskdata ,
                                bool forced    );

    void* GetThreadData( void ) const;

    TThreadStatus GetThreadStatus( void ) const;

    protected:

    UInt32 m_delayInMilliSecs;
    UInt32 m_minimalCycleDeltaInMilliSecs;

    private:

    bool CallOnThreadStartWithTimeout( Int32 maxRetries, void* taskData );
    bool CallOnThreadStartedWithTimeout( Int32 maxRetries, void* taskData );
    bool CallOnThreadEndingWithTimeout( Int32 maxRetries, void* taskData, bool willBeForced );

    private:
    CActiveObject& operator=( const CActiveObject& src );
    static Int32 GUCEF_CALLSPEC_PREFIX OnActivate( void* thisobject ) GUCEF_CALLSPEC_SUFFIX;

    struct SThreadData* _td;
    void* m_threadData;
    bool m_isDeactivationRequested;
    bool _suspend;
    bool _active;
    CMutex m_datalock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 07-02-2005 :
       - Initial implementation.

---------------------------------------------------------------------------*/
