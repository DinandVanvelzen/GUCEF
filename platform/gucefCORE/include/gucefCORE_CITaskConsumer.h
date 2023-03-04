/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#define GUCEF_CORE_CTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTNUMERICIDGENERATOR_H
#include "CTNumericIDGenerator.h"
#define GUCEF_CORE_CTNUMERICIDGENERATOR_H
#endif /* GUCEF_CORE_CTNUMERICIDGENERATOR_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDataNode;
class CDataNodeSerializableSettings;
class CThreadPool;
class CTaskDelegator;
class CPulseGenerator;

/*-------------------------------------------------------------------------*/

/**
 *  Base class for classes that carry out tasks within a threaded environment
 *
 *  The method used to invoke the functionatility of decending task classes
 *  is not defined by this class but it is assumed to by one means or another
 *  the task is performed within a thread that is exclusve to the task for the
 *  duration of the task.                      
 */
class GUCEF_CORE_PUBLIC_CPP CTaskConsumer : public CTSGNotifier
{
    public:

    static const CEvent TaskKilledEvent;
    static const CEvent TaskStartedEvent;
    static const CEvent TaskStartupFailedEvent;
    static const CEvent TaskPausedEvent;
    static const CEvent TaskResumedEvent;
    static const CEvent TaskFinishedEvent;

    static void RegisterEvents( void );

    public:

    typedef CTBasicSharedPtr< CTaskDelegator, MT::CMutex >  TTaskDelegatorBasicPtr;
    typedef CTNumericIDGenerator< UInt32 >                  TTaskIdGenerator;
    typedef TTaskIdGenerator::TNumericID                    TTaskId;

    CTaskConsumer( void );

    virtual ~CTaskConsumer();

    virtual CString GetType( void ) const = 0;
    
    void SetTaskDelegator( const TTaskDelegatorBasicPtr& delegator );
    
    TTaskDelegatorBasicPtr GetTaskDelegator( void );

    UInt32 GetDelegatorThreadId( void ) const;

    /**
     *  Sets the CPU affinity mask for the thread
     */
    bool SetCpuAffinityMask( UInt32 affinityMaskSize ,
                             void* affinityMask      );

    /**
     *  Convenience member function for setting a single CPU affinity
     *  Uses SetCpuAffinityMask()
     */
    bool SetCpuAffinityByCpuId( UInt32 cpuId );

    UInt32 GetTaskId( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool IsOwnedByThreadPool( void ) const;

    /**
     *  Flag that indicates whether the task has been asked to gracefully deactivate
     */
    bool IsDeactivationRequested( void ) const;

    /**
     *  Flag that indicates whether the task is currently active
     */
    bool IsActive( void ) const;

    /**
     *  Startup routine for the task. You should return true if startup succeeded and the task can commence
     *  cycling.
     */
    virtual bool OnTaskStart( CICloneable* taskdata ) = 0;

    /**
     *  Called after a successfull call to OnTaskStart
     */
    virtual void OnTaskStarted( CICloneable* taskdata );

    /**
     *  Called after a failed call to OnTaskStart
     */
    virtual void OnTaskStartupFailed( CICloneable* taskdata );

    /**
     *  Perorm all your main task work in this function.
     *  It will be called repeatedly until true is returned indicating that the task has been completed.
     *  Thus for ongoing tasks you can write this function to take care of a single interation of the task.
     */
    virtual bool OnTaskCycle( CICloneable* taskdata ) = 0;

    /**
     *  Last chance notification to decended classes of impending end of the task
     *  If the 'willBeForced' flag is true the thread hosting the task will be killed next
     *  Since this would be used in cases where the thread and thus this task is misbehaving
     *  one should keep that in mind in the implementation of this callback
     */
    virtual void OnTaskEnding( CICloneable* taskdata ,
                               bool willBeForced     );

    virtual void OnTaskPaused( CICloneable* taskdata ,
                               bool wasForced        );

    virtual void OnTaskResumed( CICloneable* taskdata );

    /**
     *  This is where all the cleanup should be done for task data
     *  Note that this member function will be called from within the spawned thread when ending gracefully
     *  but in the case of a forcefull termination of the spawned thread this member function will be called
     *  from the thread that triggered the forcefull termination.
     */
    virtual void OnTaskEnded( CICloneable* taskdata ,
                              bool forced           ) = 0;

    /**
     *  If an active delegator is associated with the task consumer (and thus thread) this asks said delegator to stop
     *
     *  This should never be called from the implementation based on this class but rather intended as a convenience 
     *  interface for associated code, especially in cases where dedicated externally owned TaskConsumer objects are used.
     */
    bool RequestTaskToStop( bool waitOnStop );

    /**
     *  Blocks the calling thread up to "timeoutInMs" milliseconds waiting for the task to be finished
     *
     *  This should never be called from the implementation based on this class but rather intended as a convenience 
     *  interface for associated code, especially in cases where dedicated externally owned TaskConsumer objects are used.
     */
    virtual bool WaitForTaskToFinish( Int32 timeoutInMs );

    /**
     *  Requests that prior to the next task cycle a minimum delay of 'requestedDelayInMs' is observed
     *  Note that requests to expedite cycles, contradicting this request, take priority
     *  This functionality can be used to implement throttling a task and thus the owning thread
     */
    void RequestTaskCycleDelayInMs( UInt32 requestedDelayInMs );

    /**
     *  Whether this consumer was provided with any data or was expected to run without a specific handoff of 
     *  'work' data to the consumer.
     */
    bool HasTaskData( void ) const;

    /**
     *  If this consumer was provided with any 'work' data and said data is serializable this can be used to obtain a copy of said data
     */
    bool GetSerializedTaskDataCopy( CDataNode& domNode                                ,
                                    CDataNodeSerializableSettings& serializerSettings ) const;

    private:
    friend class CThreadPool;
    friend class CTaskDelegator;

    typedef CTBasicSharedPtr< CThreadPool, MT::CMutex >     TThreadPoolBasicPtr;

    void SetIsOwnedByThreadPool( bool ownedByThreadPool );

    void SetIsInPhasedSetup( bool inPhasedSetup );

    bool GetIsInPhasedSetup( void ) const;

    void SetThreadPool( const TThreadPoolBasicPtr& threadPool );
        
    private:

    CTaskConsumer( const CTaskConsumer& src  );
    CTaskConsumer& operator=( const CTaskConsumer& src );

    private:

    TTaskId m_taskId;
    TThreadPoolBasicPtr m_threadPool;
    TTaskDelegatorBasicPtr m_delegator;
    bool m_ownedByThreadPool;
    bool m_inPhasedSetup;
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CTaskConsumer, MT::CMutex >         CTaskConsumerPtr;
typedef CTBasicSharedPtr< CTaskConsumer, MT::CMutex >    CTaskConsumerBasicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

