/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTASKINFO_H
#define GUCEF_CORE_CTASKINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CTHREADPOOLINFO_H
#include "gucefCORE_CThreadPoollnfo.h"
#define GUCEF_CORE_CTHREADPOOLINFO_H
#endif /* GUCEF_CORE_CTHREADPOOLINFO_H ? */

#ifndef GUCEF_CORE_TASKSTATUS_H
#include "gucefCORE_TaskStatus.h"
#define GUCEF_CORE_TASKSTATUS_H
#endif /* GUCEF_CORE_TASKSTATUS_H ? */

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

class CTaskDelegator;

/*-------------------------------------------------------------------------*/

/**
 *  Class for representing a snapshot of information obtained from the task
 *  manager class 
 */
class GUCEF_CORE_PUBLIC_CPP CTaskInfo : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    CTaskInfo( void );

    CTaskInfo( const CTaskInfo& src );

    virtual ~CTaskInfo();

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     */
    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successfull.
     */
    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    void SetTaskId( UInt32 taskId );
    
    UInt32 GetTaskId( void ) const;

    void SetThreadId( TSystemThreadId threadId );
    
    TSystemThreadId GetThreadId( void ) const;

    void SetTaskStatus( TTaskStatus taskStatus );

    TTaskStatus GetTaskStatus( void ) const;

    void SetHasTaskData( bool hasTaskData );
    
    bool GetHasTaskData( void ) const;

    void SetCustomTaskDataIsSerializable( bool isSerializable );
    
    bool GetCustomTaskDataIsSerializable( void ) const;

    void SetTaskTypeName( const CString& taskTypeName );

    const CString& GetTaskTypeName( void ) const;

    const CDataNode& GetCustomTaskData( void ) const;

    CDataNode& GetCustomTaskData( void );

    /** The pool of the task */
    void SetThreadPoolName( const CString& threadPoolName );
    const CString& GetThreadPoolName( void ) const;

    /** Monotonic time (CSystemMonotonicClock) at which the task was queued; 0 = unknown */
    void SetQueuedAtMs( UInt64 queuedAtMs );
    UInt64 GetQueuedAtMs( void ) const;

    /** Monotonic time at which a thread started the task; 0 = not started */
    void SetStartedAtMs( UInt64 startedAtMs );
    UInt64 GetStartedAtMs( void ) const;

    /** When the task is a run of a scheduled job: the host name of its job scheduler */
    void SetSchedulerName( const CString& schedulerName );
    const CString& GetSchedulerName( void ) const;

    /** When the task is a run of a scheduled job: the job id */
    void SetScheduledJobId( const CString& scheduledJobId );
    const CString& GetScheduledJobId( void ) const;

    /** When the task is a run of a scheduled job: the run id */
    void SetScheduledRunId( UInt64 scheduledRunId );
    UInt64 GetScheduledRunId( void ) const;

    void Clear( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    UInt32 m_taskId;
    TSystemThreadId m_threadId;
    TTaskStatus m_taskStatus;
    bool m_hasTaskData;
    bool m_customTaskDataIsSerializable;
    CDataNode m_taskData;
    CString m_taskTypeName;
    CString m_threadPoolName;
    UInt64 m_queuedAtMs;
    UInt64 m_startedAtMs;
    CString m_schedulerName;
    CString m_scheduledJobId;
    UInt64 m_scheduledRunId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTASKINFO_H ? */
