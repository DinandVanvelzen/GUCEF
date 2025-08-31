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
    , m_taskStatus( TTaskStatus::TASKSTATUS_UNDEFINED )
    , m_hasTaskData( false )
    , m_customTaskDataIsSerializable( false )
    , m_taskData( GUCEF_DATATYPE_NIL )
    , m_taskTypeName()
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
    m_taskStatus = ETaskStatus::TASKSTATUS_UNDEFINED;
    m_hasTaskData = false;
    m_customTaskDataIsSerializable = false;
    m_taskData.Clear();
    m_taskTypeName.Clear();
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
    m_threadId = domRootNode.GetAttributeValueOrChildValueByName( "threadId" ).AsInt32( m_threadId, true );
    m_taskStatus = (TTaskStatus) domRootNode.GetAttributeValueOrChildValueByName( "taskStatusId", m_taskStatus ).AsUInt8( m_taskStatus, true );
    m_hasTaskData = domRootNode.GetAttributeValueOrChildValueByName( "hasTaskData" ).AsBool( m_hasTaskData, true );
    m_customTaskDataIsSerializable = domRootNode.GetAttributeValueOrChildValueByName( "customTaskDataIsSerializable" ).AsBool( m_customTaskDataIsSerializable, true );
    m_taskTypeName = domRootNode.GetAttributeValueOrChildValueByName( "taskTypeName" ).AsString( m_taskTypeName, true );

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
CTaskInfo::SetThreadId( UInt32 threadId )
{GUCEF_TRACE;

    m_threadId = threadId;
}

/*-------------------------------------------------------------------------*/

UInt32
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
