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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_TASKSTATUS_H
#include "gucefCORE_TaskStatus.h"
#define GUCEF_CORE_TASKSTATUS_H
#endif /* GUCEF_CORE_TASKSTATUS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "gucefCORE_CTaskInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CTaskInfo::ClassTypeName = "GUCEF::CORE::CTaskInfo";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTaskInfo::CTaskInfo( void )
    : CIDataNodeSerializable()
    , m_taskId( 0 )
    , m_threadId( 0 )
    , m_taskStatus( CORE::TASKSTATUS_UNDEFINED )
    , m_hasTaskData( false )
    , m_customTaskDataIsSerializable( false )
    , m_taskData( GUCEF_DATATYPE_NIL )
    , m_taskTypeName()
    , m_threadPoolName()
    , m_queuedAtMs( 0 )
    , m_startedAtMs( 0 )
    , m_schedulerName()
    , m_scheduledJobId()
    , m_scheduledRunId( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTaskInfo::CTaskInfo( const CTaskInfo& src )
    : CIDataNodeSerializable( src )
    , m_taskId( src.m_taskId )
    , m_threadId( src.m_threadId )
    , m_taskStatus( src.m_taskStatus )
    , m_hasTaskData( src.m_hasTaskData )
    , m_customTaskDataIsSerializable( src.m_customTaskDataIsSerializable )
    , m_taskData( src.m_taskData )
    , m_taskTypeName( src.m_taskTypeName )
    , m_threadPoolName( src.m_threadPoolName )
    , m_queuedAtMs( src.m_queuedAtMs )
    , m_startedAtMs( src.m_startedAtMs )
    , m_schedulerName( src.m_schedulerName )
    , m_scheduledJobId( src.m_scheduledJobId )
    , m_scheduledRunId( src.m_scheduledRunId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTaskInfo::~CTaskInfo()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void 
CTaskInfo::Clear( void )
{GUCEF_TRACE;

    m_taskId = 0;
    m_threadId = 0;
    m_taskStatus = CORE::TASKSTATUS_UNDEFINED;
    m_hasTaskData = false;
    m_customTaskDataIsSerializable = false;
    m_taskData.Clear();
    m_taskTypeName.Clear();
    m_threadPoolName = CString();
    m_queuedAtMs = 0;
    m_startedAtMs = 0;
    m_schedulerName = CString();
    m_scheduledJobId = CString();
    m_scheduledRunId = 0;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskInfo::Serialize( CDataNode& domRootNode                        ,
                      const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    
    totalSuccess = domRootNode.SetAttribute( "taskId", m_taskId ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "threadId", m_threadId ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "taskStatusId", (Int32) m_taskStatus ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "hasTaskData", m_hasTaskData ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "customTaskDataIsSerializable", m_customTaskDataIsSerializable ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "taskTypeName", m_taskTypeName ) && totalSuccess;
    if ( !m_threadPoolName.IsNULLOrEmpty() )
        totalSuccess = domRootNode.SetAttribute( "threadPoolName", m_threadPoolName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "queuedAtMs", m_queuedAtMs ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "startedAtMs", m_startedAtMs ) && totalSuccess;
    if ( !m_schedulerName.IsNULLOrEmpty() )
    {
        totalSuccess = domRootNode.SetAttribute( "schedulerName", m_schedulerName ) && totalSuccess;
        totalSuccess = domRootNode.SetAttribute( "scheduledJobId", m_scheduledJobId ) && totalSuccess;
        totalSuccess = domRootNode.SetAttribute( "scheduledRunId", m_scheduledRunId ) && totalSuccess;
    }

    if ( CDataNodeSerializableSettings::DataNodeSerializableLod_MinimumDetails < settings.levelOfDetail )
    {
        totalSuccess = domRootNode.SetAttribute( "taskStatus", TaskStatusToTaskStatusString( m_taskStatus ) ) && totalSuccess;
    }
    
    if ( m_hasTaskData && m_customTaskDataIsSerializable )
    {
        if ( CDataNodeSerializableSettings::DataNodeSerializableLod_MinimumDetails < settings.levelOfDetail )
        {
            // task data could potentially be substantial while important, only add at LOD above minimum
            CDataNode* taskDataNode = domRootNode.AddChild( "customTaskData", GUCEF_DATATYPE_OBJECT );
            if ( GUCEF_NULL != taskDataNode )
            {                
                // Check to see if task data was ever set on this info object.
                // if not we just fetch it on-demand best-effort
                if ( GUCEF_DATATYPE_NIL != m_taskData.GetNodeType() )
                {
                    totalSuccess = GUCEF_NULL != taskDataNode->AddChild( m_taskData ) && totalSuccess;            
                }
                else
                {
                    CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
                    totalSuccess = taskManager.GetSerializedTaskDataCopy( m_taskId, *taskDataNode, settings ) && totalSuccess;                    
                }
            }
            else
                totalSuccess = false;
        }    
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CTaskInfo::Deserialize( const CDataNode& domRootNode                  ,
                        const CDataNodeSerializableSettings& settings ) 
{GUCEF_TRACE;

    m_taskId = domRootNode.GetAttributeValueOrChildValueByName( "taskId" ).AsUInt32( m_taskId, true );
    m_threadId = (TSystemThreadId) domRootNode.GetAttributeValueOrChildValueByName( "threadId" ).AsUInt64( m_threadId, true );
    m_taskStatus = (TTaskStatus) domRootNode.GetAttributeValueOrChildValueByName( "taskStatusId", m_taskStatus ).AsUInt8( m_taskStatus, true );
    m_hasTaskData = domRootNode.GetAttributeValueOrChildValueByName( "hasTaskData" ).AsBool( m_hasTaskData, true );
    m_customTaskDataIsSerializable = domRootNode.GetAttributeValueOrChildValueByName( "customTaskDataIsSerializable" ).AsBool( m_customTaskDataIsSerializable, true );
    m_taskTypeName = domRootNode.GetAttributeValueOrChildValueByName( "taskTypeName" ).AsString( m_taskTypeName, true );
    m_threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( m_threadPoolName, true );
    m_queuedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "queuedAtMs" ).AsUInt64( m_queuedAtMs, true );
    m_startedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "startedAtMs" ).AsUInt64( m_startedAtMs, true );
    m_schedulerName = domRootNode.GetAttributeValueOrChildValueByName( "schedulerName" ).AsString( m_schedulerName, true );
    m_scheduledJobId = domRootNode.GetAttributeValueOrChildValueByName( "scheduledJobId" ).AsString( m_scheduledJobId, true );
    m_scheduledRunId = domRootNode.GetAttributeValueOrChildValueByName( "scheduledRunId" ).AsUInt64( m_scheduledRunId, true );

    CDataNode* taskDataNode = domRootNode.FindChild( "customTaskData" );
    if ( GUCEF_NULL != taskDataNode )
    {
        CDataNode* actualDataNode = taskDataNode->GetFirstChild();
        if ( GUCEF_NULL != actualDataNode )
            m_taskData = *actualDataNode;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetTaskId( UInt32 taskId )
{GUCEF_TRACE;

    m_taskId = taskId;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskInfo::GetTaskId( void ) const
{GUCEF_TRACE;

    return m_taskId;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetThreadId( TSystemThreadId threadId )
{GUCEF_TRACE;

    m_threadId = threadId;
}

/*-------------------------------------------------------------------------*/

TSystemThreadId
CTaskInfo::GetThreadId( void ) const
{GUCEF_TRACE;

    return m_threadId;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetTaskStatus( TTaskStatus taskStatus )
{GUCEF_TRACE;

    m_taskStatus = taskStatus;
}

/*-------------------------------------------------------------------------*/

TTaskStatus
CTaskInfo::GetTaskStatus( void ) const
{GUCEF_TRACE;

    return m_taskStatus;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetHasTaskData( bool hasTaskData )
{GUCEF_TRACE;

    m_hasTaskData = hasTaskData;
}

/*-------------------------------------------------------------------------*/

bool
CTaskInfo::GetHasTaskData( void ) const
{GUCEF_TRACE;

    return m_hasTaskData;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetCustomTaskDataIsSerializable( bool isSerializable )
{GUCEF_TRACE;

    m_customTaskDataIsSerializable = isSerializable;
}

/*-------------------------------------------------------------------------*/

bool
CTaskInfo::GetCustomTaskDataIsSerializable( void ) const
{GUCEF_TRACE;

    return m_customTaskDataIsSerializable;
}

/*-------------------------------------------------------------------------*/

void 
CTaskInfo::SetTaskTypeName( const CString& taskTypeName )
{GUCEF_TRACE;

    m_taskTypeName = taskTypeName;
}

/*-------------------------------------------------------------------------*/

const CString& 
CTaskInfo::GetTaskTypeName( void ) const
{GUCEF_TRACE;

    return m_taskTypeName;
}


/*-------------------------------------------------------------------------*/

const CString& 
CTaskInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

CICloneable* 
CTaskInfo::Clone( void ) const
{GUCEF_TRACE;

    return new CTaskInfo( *this );
}

/*-------------------------------------------------------------------------*/

const CDataNode&
CTaskInfo::GetCustomTaskData( void ) const
{GUCEF_TRACE;

    return m_taskData;
}

/*-------------------------------------------------------------------------*/

CDataNode& 
CTaskInfo::GetCustomTaskData( void )
{GUCEF_TRACE;

    return m_taskData;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetThreadPoolName( const CString& threadPoolName )
{GUCEF_TRACE;

    m_threadPoolName = threadPoolName;
}

/*-------------------------------------------------------------------------*/

const CString&
CTaskInfo::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetQueuedAtMs( UInt64 queuedAtMs )
{GUCEF_TRACE;

    m_queuedAtMs = queuedAtMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CTaskInfo::GetQueuedAtMs( void ) const
{GUCEF_TRACE;

    return m_queuedAtMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetStartedAtMs( UInt64 startedAtMs )
{GUCEF_TRACE;

    m_startedAtMs = startedAtMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CTaskInfo::GetStartedAtMs( void ) const
{GUCEF_TRACE;

    return m_startedAtMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetSchedulerName( const CString& schedulerName )
{GUCEF_TRACE;

    m_schedulerName = schedulerName;
}

/*-------------------------------------------------------------------------*/

const CString&
CTaskInfo::GetSchedulerName( void ) const
{GUCEF_TRACE;

    return m_schedulerName;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetScheduledJobId( const CString& scheduledJobId )
{GUCEF_TRACE;

    m_scheduledJobId = scheduledJobId;
}

/*-------------------------------------------------------------------------*/

const CString&
CTaskInfo::GetScheduledJobId( void ) const
{GUCEF_TRACE;

    return m_scheduledJobId;
}

/*-------------------------------------------------------------------------*/

void
CTaskInfo::SetScheduledRunId( UInt64 scheduledRunId )
{GUCEF_TRACE;

    m_scheduledRunId = scheduledRunId;
}

/*-------------------------------------------------------------------------*/

UInt64
CTaskInfo::GetScheduledRunId( void ) const
{GUCEF_TRACE;

    return m_scheduledRunId;
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
