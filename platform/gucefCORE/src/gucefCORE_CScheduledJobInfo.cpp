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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCORE_CScheduledJobInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CScheduledJobInfo::ClassTypeName = "GUCEF::CORE::CScheduledJobInfo";

/*-------------------------------------------------------------------------*/

CScheduledJobInfo::CScheduledJobInfo( void )
    : CIDataNodeSerializable()
    , m_schedulerName()
    , m_jobId()
    , m_title()
    , m_taskTypeName()
    , m_laneName()
    , m_state( JOBSTATE_UNSPECIFIED )
    , m_executionTarget( JOBEXECUTIONTARGET_POOL )
    , m_threadPoolName()
    , m_origin( JOBORIGIN_CODE )
    , m_nextDueUtcMs( 0 )
    , m_activeRunCount( 0 )
    , m_lastRunId( 0 )
    , m_lastRunState( JOBRUNSTATE_UNSPECIFIED )
    , m_lastRunEndedUtcMs( 0 )
    , m_flags( JOBFLAG_NONE )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CScheduledJobInfo::~CScheduledJobInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CScheduledJobInfo::Serialize( CDataNode& domRootNode                        ,
                              const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    domRootNode.SetNodeType( GUCEF_DATATYPE_OBJECT );
    totalSuccess = domRootNode.SetAttribute( "schedulerName", m_schedulerName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "jobId", m_jobId ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "title", m_title ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "taskTypeName", m_taskTypeName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "laneName", m_laneName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "state", JobStateToString( m_state ) ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "executionTarget", JobExecutionTargetToString( m_executionTarget ) ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "threadPoolName", m_threadPoolName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "origin", JOBORIGIN_SUBMITTED == m_origin ? "SUBMITTED" : "CODE" ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "nextDueUtcMs", m_nextDueUtcMs ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "activeRunCount", m_activeRunCount ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "lastRunId", m_lastRunId ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "lastRunState", JobRunStateToString( m_lastRunState ) ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "lastRunEndedUtcMs", m_lastRunEndedUtcMs ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "flags", m_flags ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CScheduledJobInfo::Deserialize( const CDataNode& domRootNode                  ,
                                const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    m_schedulerName = domRootNode.GetAttributeValueOrChildValueByName( "schedulerName" ).AsString( m_schedulerName, true );
    m_jobId = domRootNode.GetAttributeValueOrChildValueByName( "jobId" ).AsString( m_jobId, true );
    m_title = domRootNode.GetAttributeValueOrChildValueByName( "title" ).AsString( m_title, true );
    m_taskTypeName = domRootNode.GetAttributeValueOrChildValueByName( "taskTypeName" ).AsString( m_taskTypeName, true );
    m_laneName = domRootNode.GetAttributeValueOrChildValueByName( "laneName" ).AsString( m_laneName, true );
    CString stateText = domRootNode.GetAttributeValueOrChildValueByName( "state" ).AsString( CString::Empty, true );
    static const TJobState states[] = { JOBSTATE_SCHEDULED, JOBSTATE_QUEUED, JOBSTATE_WAITING, JOBSTATE_RUNNING, JOBSTATE_DISABLED, JOBSTATE_UNAVAILABLE, JOBSTATE_ENDED };
    for ( UInt32 i=0; i<sizeof(states)/sizeof(states[0]); ++i )
        if ( stateText == JobStateToString( states[ i ] ) )
            m_state = states[ i ];
    CString targetText = domRootNode.GetAttributeValueOrChildValueByName( "executionTarget" ).AsString( CString::Empty, true );
    if ( targetText == JobExecutionTargetToString( JOBEXECUTIONTARGET_MAIN_THREAD ) )
        m_executionTarget = JOBEXECUTIONTARGET_MAIN_THREAD;
    else
    if ( targetText == JobExecutionTargetToString( JOBEXECUTIONTARGET_POOL ) )
        m_executionTarget = JOBEXECUTIONTARGET_POOL;
    m_threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( m_threadPoolName, true );
    CString originText = domRootNode.GetAttributeValueOrChildValueByName( "origin" ).AsString( CString::Empty, true );
    if ( !originText.IsNULLOrEmpty() )
        m_origin = originText == "SUBMITTED" ? JOBORIGIN_SUBMITTED : JOBORIGIN_CODE;
    m_nextDueUtcMs = domRootNode.GetAttributeValueOrChildValueByName( "nextDueUtcMs" ).AsInt64( m_nextDueUtcMs, true );
    m_activeRunCount = domRootNode.GetAttributeValueOrChildValueByName( "activeRunCount" ).AsUInt32( m_activeRunCount, true );
    m_lastRunId = domRootNode.GetAttributeValueOrChildValueByName( "lastRunId" ).AsUInt64( m_lastRunId, true );
    CString runStateText = domRootNode.GetAttributeValueOrChildValueByName( "lastRunState" ).AsString( CString::Empty, true );
    for ( Int32 i=0; i<=8; ++i )
        if ( runStateText == JobRunStateToString( (TJobRunState) i ) )
            m_lastRunState = (TJobRunState) i;
    m_lastRunEndedUtcMs = domRootNode.GetAttributeValueOrChildValueByName( "lastRunEndedUtcMs" ).AsInt64( m_lastRunEndedUtcMs, true );
    m_flags = domRootNode.GetAttributeValueOrChildValueByName( "flags" ).AsUInt32( m_flags, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CScheduledJobInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CScheduledJobInfo( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CScheduledJobInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetSchedulerName( const CString& schedulerName )
{GUCEF_TRACE;

    m_schedulerName = schedulerName;
}

/*-------------------------------------------------------------------------*/

const CString&
CScheduledJobInfo::GetSchedulerName( void ) const
{GUCEF_TRACE;

    return m_schedulerName;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetJobId( const CString& jobId )
{GUCEF_TRACE;

    m_jobId = jobId;
}

/*-------------------------------------------------------------------------*/

const CString&
CScheduledJobInfo::GetJobId( void ) const
{GUCEF_TRACE;

    return m_jobId;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetTitle( const CString& title )
{GUCEF_TRACE;

    m_title = title;
}

/*-------------------------------------------------------------------------*/

const CString&
CScheduledJobInfo::GetTitle( void ) const
{GUCEF_TRACE;

    return m_title;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetTaskTypeName( const CString& taskTypeName )
{GUCEF_TRACE;

    m_taskTypeName = taskTypeName;
}

/*-------------------------------------------------------------------------*/

const CString&
CScheduledJobInfo::GetTaskTypeName( void ) const
{GUCEF_TRACE;

    return m_taskTypeName;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetLaneName( const CString& laneName )
{GUCEF_TRACE;

    m_laneName = laneName;
}

/*-------------------------------------------------------------------------*/

const CString&
CScheduledJobInfo::GetLaneName( void ) const
{GUCEF_TRACE;

    return m_laneName;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetState( TJobState state )
{GUCEF_TRACE;

    m_state = state;
}

/*-------------------------------------------------------------------------*/

TJobState
CScheduledJobInfo::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetExecutionTarget( TJobExecutionTarget executionTarget )
{GUCEF_TRACE;

    m_executionTarget = executionTarget;
}

/*-------------------------------------------------------------------------*/

TJobExecutionTarget
CScheduledJobInfo::GetExecutionTarget( void ) const
{GUCEF_TRACE;

    return m_executionTarget;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetThreadPoolName( const CString& threadPoolName )
{GUCEF_TRACE;

    m_threadPoolName = threadPoolName;
}

/*-------------------------------------------------------------------------*/

const CString&
CScheduledJobInfo::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetOrigin( TJobOrigin origin )
{GUCEF_TRACE;

    m_origin = origin;
}

/*-------------------------------------------------------------------------*/

TJobOrigin
CScheduledJobInfo::GetOrigin( void ) const
{GUCEF_TRACE;

    return m_origin;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetNextDueUtcMs( Int64 nextDueUtcMs )
{GUCEF_TRACE;

    m_nextDueUtcMs = nextDueUtcMs;
}

/*-------------------------------------------------------------------------*/

Int64
CScheduledJobInfo::GetNextDueUtcMs( void ) const
{GUCEF_TRACE;

    return m_nextDueUtcMs;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetActiveRunCount( UInt32 activeRunCount )
{GUCEF_TRACE;

    m_activeRunCount = activeRunCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CScheduledJobInfo::GetActiveRunCount( void ) const
{GUCEF_TRACE;

    return m_activeRunCount;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetLastRunId( UInt64 lastRunId )
{GUCEF_TRACE;

    m_lastRunId = lastRunId;
}

/*-------------------------------------------------------------------------*/

UInt64
CScheduledJobInfo::GetLastRunId( void ) const
{GUCEF_TRACE;

    return m_lastRunId;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetLastRunState( TJobRunState lastRunState )
{GUCEF_TRACE;

    m_lastRunState = lastRunState;
}

/*-------------------------------------------------------------------------*/

TJobRunState
CScheduledJobInfo::GetLastRunState( void ) const
{GUCEF_TRACE;

    return m_lastRunState;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetLastRunEndedUtcMs( Int64 lastRunEndedUtcMs )
{GUCEF_TRACE;

    m_lastRunEndedUtcMs = lastRunEndedUtcMs;
}

/*-------------------------------------------------------------------------*/

Int64
CScheduledJobInfo::GetLastRunEndedUtcMs( void ) const
{GUCEF_TRACE;

    return m_lastRunEndedUtcMs;
}

/*-------------------------------------------------------------------------*/

void
CScheduledJobInfo::SetFlags( UInt32 flags )
{GUCEF_TRACE;

    m_flags = flags;
}

/*-------------------------------------------------------------------------*/

UInt32
CScheduledJobInfo::GetFlags( void ) const
{GUCEF_TRACE;

    return m_flags;
}

/*-------------------------------------------------------------------------*/

bool
CScheduledJobInfo::IsHidden( void ) const
{GUCEF_TRACE;

    return 0 != ( m_flags & JOBFLAG_HIDDEN );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
