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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CJobDefinition.h"

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

const CString CJobDefinition::ClassTypeName = "GUCEF::CORE::CJobDefinition";

/*-------------------------------------------------------------------------*/

CJobDefinition::CJobDefinition( void )
    : CIDataNodeSerializable()
    , m_id()
    , m_title()
    , m_description()
    , m_taskTypeName()
    , m_payload()
    , m_trigger()
    , m_laneName()
    , m_executionTarget( JOBEXECUTIONTARGET_POOL )
    , m_threadPoolName()
    , m_priority( 1 )
    , m_concurrency( JOBCONCURRENCY_SKIP_IF_RUNNING )
    , m_maxParallelRuns( 1 )
    , m_missedRunPolicy( JOBMISSEDRUNPOLICY_RUN_ONCE )
    , m_maxCatchUpRuns( 3 )
    , m_maxLatenessMs( 0 )
    , m_timeoutMs( 0 )
    , m_retryPolicy()
    , m_flags( JOBFLAG_NONE )
    , m_remoteActions()
    , m_remoteCustomActions()
    , m_origin( JOBORIGIN_CODE )
    , m_submittedBy()
    , m_submittedAtUtcMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobDefinition::~CJobDefinition()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

static const char*
ConcurrencyToString( TJobConcurrency concurrency )
{GUCEF_TRACE;

    switch ( concurrency )
    {
        case JOBCONCURRENCY_QUEUE_ONE:  return "QUEUE_ONE";
        case JOBCONCURRENCY_PARALLEL:   return "PARALLEL";
        default:                        return "SKIP_IF_RUNNING";
    }
}

/*-------------------------------------------------------------------------*/

static TJobConcurrency
ConcurrencyFromString( const CString& text, TJobConcurrency defaultValue )
{GUCEF_TRACE;

    if ( text.Equals( "SKIP_IF_RUNNING", false ) )  return JOBCONCURRENCY_SKIP_IF_RUNNING;
    if ( text.Equals( "QUEUE_ONE", false ) )        return JOBCONCURRENCY_QUEUE_ONE;
    if ( text.Equals( "PARALLEL", false ) )         return JOBCONCURRENCY_PARALLEL;
    return defaultValue;
}

/*-------------------------------------------------------------------------*/

static const char*
MissedRunPolicyToString( TJobMissedRunPolicy policy )
{GUCEF_TRACE;

    switch ( policy )
    {
        case JOBMISSEDRUNPOLICY_SKIP:       return "SKIP";
        case JOBMISSEDRUNPOLICY_RUN_EACH:   return "RUN_EACH";
        default:                            return "RUN_ONCE";
    }
}

/*-------------------------------------------------------------------------*/

static TJobMissedRunPolicy
MissedRunPolicyFromString( const CString& text, TJobMissedRunPolicy defaultValue )
{GUCEF_TRACE;

    if ( text.Equals( "SKIP", false ) )     return JOBMISSEDRUNPOLICY_SKIP;
    if ( text.Equals( "RUN_ONCE", false ) ) return JOBMISSEDRUNPOLICY_RUN_ONCE;
    if ( text.Equals( "RUN_EACH", false ) ) return JOBMISSEDRUNPOLICY_RUN_EACH;
    return defaultValue;
}

/*-------------------------------------------------------------------------*/

static TJobControlAction
ControlActionFromString( const CString& text )
{GUCEF_TRACE;

    for ( Int32 i=3; i<=10; ++i )
        if ( text.Equals( JobControlActionToString( (TJobControlAction) i ), false ) )
            return (TJobControlAction) i;
    return JOBCONTROLACTION_UNSPECIFIED;
}

/*-------------------------------------------------------------------------*/

bool
CJobDefinition::IsValidJobId( const CString& jobId )
{GUCEF_TRACE;

    if ( jobId.IsNULLOrEmpty() || jobId.ByteSize() > MaxJobIdLength+1 )
        return false;
    const char* id = jobId.C_String();
    if ( id[ 0 ] < 'a' || id[ 0 ] > 'z' )
        return false;
    for ( UInt32 i=1; '\0' != id[ i ]; ++i )
    {
        char c = id[ i ];
        bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '_' == c || '-' == c;
        if ( !ok )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobDefinition::IsValid( CString& error ) const
{GUCEF_TRACE;

    error.Clear();
    if ( !IsValidJobId( m_id ) )
    {
        error = "the job id \"" + m_id + "\" is not of the form [a-z][a-z0-9._-]* with at most " + ToString( MaxJobIdLength ) + " bytes";
        return false;
    }
    if ( m_taskTypeName.IsNULLOrEmpty() )
    {
        error = "the job " + m_id + " has no task type";
        return false;
    }
    CString triggerError;
    if ( !m_trigger.IsValid( triggerError ) )
    {
        error = "the trigger of job " + m_id + " is not valid: " + triggerError;
        return false;
    }
    if ( JOBEXECUTIONTARGET_POOL != m_executionTarget && JOBEXECUTIONTARGET_MAIN_THREAD != m_executionTarget )
    {
        error = "the job " + m_id + " runs on a pool or on the main thread";
        return false;
    }
    if ( m_priority > 3 )
    {
        error = "the priority of job " + m_id + " is 0 (highest) .. 3";
        return false;
    }
    if ( JOBCONCURRENCY_PARALLEL == m_concurrency && 0 == m_maxParallelRuns )
    {
        error = "the job " + m_id + " runs in parallel: it needs a maximum of parallel runs above 0";
        return false;
    }
    if ( 0 == m_retryPolicy.GetMaxAttempts() )
    {
        error = "the job " + m_id + " needs at least one attempt";
        return false;
    }
    if ( m_retryPolicy.GetJitterPercent() > 50 )
    {
        error = "the retry jitter of job " + m_id + " is 0..50 percent";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobDefinition::Serialize( CDataNode& domRootNode                        ,
                           const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    domRootNode.SetNodeType( GUCEF_DATATYPE_OBJECT );
    totalSuccess = domRootNode.SetAttribute( "id", m_id ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "title", m_title ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "description", m_description ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "taskTypeName", m_taskTypeName ) && totalSuccess;
    if ( m_payload.HasChildren() || !m_payload.GetValue().IsNULLOrEmpty() || m_payload.AttributeBegin() != m_payload.AttributeEnd() )
    {
        CDataNode payload( m_payload );
        payload.SetName( "payload" );
        totalSuccess = GUCEF_NULL != domRootNode.AddChild( payload ) && totalSuccess;
    }
    CDataNode* triggerNode = domRootNode.AddChild( "trigger", GUCEF_DATATYPE_OBJECT );
    totalSuccess = GUCEF_NULL != triggerNode && m_trigger.Serialize( *triggerNode, settings ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "laneName", m_laneName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "executionTarget", JobExecutionTargetToString( m_executionTarget ) ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "threadPoolName", m_threadPoolName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "priority", m_priority ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "concurrency", ConcurrencyToString( m_concurrency ) ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "maxParallelRuns", m_maxParallelRuns ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "missedRunPolicy", MissedRunPolicyToString( m_missedRunPolicy ) ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "maxCatchUpRuns", m_maxCatchUpRuns ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "maxLatenessMs", m_maxLatenessMs ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "timeoutMs", m_timeoutMs ) && totalSuccess;
    CDataNode* retryNode = domRootNode.AddChild( "retry", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != retryNode )
    {
        totalSuccess = retryNode->SetAttribute( "maxAttempts", m_retryPolicy.GetMaxAttempts() ) && totalSuccess;
        totalSuccess = retryNode->SetAttribute( "backoffInitialMs", m_retryPolicy.GetBackoffInitialMs() ) && totalSuccess;
        totalSuccess = retryNode->SetAttribute( "backoffMaxMs", m_retryPolicy.GetBackoffMaxMs() ) && totalSuccess;
        totalSuccess = retryNode->SetAttribute( "jitterPercent", m_retryPolicy.GetJitterPercent() ) && totalSuccess;
    }
    totalSuccess = domRootNode.SetAttribute( "flags", m_flags ) && totalSuccess;
    if ( !m_remoteActions.empty() )
    {
        CString::StringSet actions;
        for ( TJobControlActionSet::const_iterator i = m_remoteActions.begin(); i != m_remoteActions.end(); ++i )
            actions.insert( JobControlActionToString( (TJobControlAction) (*i) ) );
        totalSuccess = domRootNode.AddAllValuesAsChildrenOfChild( "remoteActions", actions, GUCEF_DATATYPE_ARRAY ) && totalSuccess;
    }
    if ( !m_remoteCustomActions.empty() )
        totalSuccess = domRootNode.AddAllValuesAsChildrenOfChild( "remoteCustomActions", m_remoteCustomActions, GUCEF_DATATYPE_ARRAY ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "origin", JOBORIGIN_SUBMITTED == m_origin ? "SUBMITTED" : "CODE" ) && totalSuccess;
    if ( JOBORIGIN_SUBMITTED == m_origin )
    {
        totalSuccess = domRootNode.SetAttribute( "submittedBy", m_submittedBy ) && totalSuccess;
        totalSuccess = domRootNode.SetAttribute( "submittedAtUtcMs", m_submittedAtUtcMs ) && totalSuccess;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CJobDefinition::Deserialize( const CDataNode& domRootNode                  ,
                             const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    // fields that are missing keep their defaults; a definition without an id, a task type or a valid trigger fails
    CJobDefinition job;
    job.m_id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString( CString::Empty, true );
    job.m_title = domRootNode.GetAttributeValueOrChildValueByName( "title" ).AsString( CString::Empty, true );
    job.m_description = domRootNode.GetAttributeValueOrChildValueByName( "description" ).AsString( CString::Empty, true );
    job.m_taskTypeName = domRootNode.GetAttributeValueOrChildValueByName( "taskTypeName" ).AsString( CString::Empty, true );
    const CDataNode* payloadNode = domRootNode.FindChild( "payload" );
    if ( GUCEF_NULL != payloadNode )
        job.m_payload = *payloadNode;
    const CDataNode* triggerNode = domRootNode.FindChild( "trigger" );
    if ( GUCEF_NULL == triggerNode || !job.m_trigger.Deserialize( *triggerNode, settings ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CJobDefinition:Deserialize: job \"" << job.m_id << "\" has no valid trigger" );
        return false;
    }
    job.m_laneName = domRootNode.GetAttributeValueOrChildValueByName( "laneName" ).AsString( CString::Empty, true );
    CString targetText = domRootNode.GetAttributeValueOrChildValueByName( "executionTarget" ).AsString( "POOL", true );
    if ( targetText.Equals( "MAIN_THREAD", false ) )
        job.m_executionTarget = JOBEXECUTIONTARGET_MAIN_THREAD;
    else
    if ( targetText.Equals( "POOL", false ) )
        job.m_executionTarget = JOBEXECUTIONTARGET_POOL;
    else
        job.m_executionTarget = JOBEXECUTIONTARGET_UNSPECIFIED;
    job.m_threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( CString::Empty, true );
    job.m_priority = domRootNode.GetAttributeValueOrChildValueByName( "priority" ).AsUInt32( job.m_priority, true );
    job.m_concurrency = ConcurrencyFromString( domRootNode.GetAttributeValueOrChildValueByName( "concurrency" ).AsString( CString::Empty, true ), job.m_concurrency );
    job.m_maxParallelRuns = domRootNode.GetAttributeValueOrChildValueByName( "maxParallelRuns" ).AsUInt32( job.m_maxParallelRuns, true );
    job.m_missedRunPolicy = MissedRunPolicyFromString( domRootNode.GetAttributeValueOrChildValueByName( "missedRunPolicy" ).AsString( CString::Empty, true ), job.m_missedRunPolicy );
    job.m_maxCatchUpRuns = domRootNode.GetAttributeValueOrChildValueByName( "maxCatchUpRuns" ).AsUInt32( job.m_maxCatchUpRuns, true );
    job.m_maxLatenessMs = domRootNode.GetAttributeValueOrChildValueByName( "maxLatenessMs" ).AsUInt64( job.m_maxLatenessMs, true );
    job.m_timeoutMs = domRootNode.GetAttributeValueOrChildValueByName( "timeoutMs" ).AsUInt64( job.m_timeoutMs, true );
    const CDataNode* retryNode = domRootNode.FindChild( "retry" );
    if ( GUCEF_NULL != retryNode )
    {
        job.m_retryPolicy.SetMaxAttempts( retryNode->GetAttributeValueOrChildValueByName( "maxAttempts" ).AsUInt32( job.m_retryPolicy.GetMaxAttempts(), true ) );
        job.m_retryPolicy.SetBackoffInitialMs( retryNode->GetAttributeValueOrChildValueByName( "backoffInitialMs" ).AsUInt64( job.m_retryPolicy.GetBackoffInitialMs(), true ) );
        job.m_retryPolicy.SetBackoffMaxMs( retryNode->GetAttributeValueOrChildValueByName( "backoffMaxMs" ).AsUInt64( job.m_retryPolicy.GetBackoffMaxMs(), true ) );
        job.m_retryPolicy.SetJitterPercent( retryNode->GetAttributeValueOrChildValueByName( "jitterPercent" ).AsUInt32( job.m_retryPolicy.GetJitterPercent(), true ) );
    }
    job.m_flags = domRootNode.GetAttributeValueOrChildValueByName( "flags" ).AsUInt32( job.m_flags, true );
    CString::StringSet actions;
    if ( domRootNode.GetValuesOfChildByName( "remoteActions", actions ) )
    {
        for ( CString::StringSet::iterator i = actions.begin(); i != actions.end(); ++i )
        {
            TJobControlAction action = ControlActionFromString( (*i) );
            if ( JOBCONTROLACTION_UNSPECIFIED != action )
                job.m_remoteActions.insert( (Int32) action );
        }
    }
    domRootNode.GetValuesOfChildByName( "remoteCustomActions", job.m_remoteCustomActions );
    CString originText = domRootNode.GetAttributeValueOrChildValueByName( "origin" ).AsString( "CODE", true );
    job.m_origin = originText.Equals( "SUBMITTED", false ) ? JOBORIGIN_SUBMITTED : JOBORIGIN_CODE;
    job.m_submittedBy = domRootNode.GetAttributeValueOrChildValueByName( "submittedBy" ).AsString( CString::Empty, true );
    job.m_submittedAtUtcMs = domRootNode.GetAttributeValueOrChildValueByName( "submittedAtUtcMs" ).AsInt64( 0, true );

    CString error;
    if ( !job.IsValid( error ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CJobDefinition:Deserialize: " << error );
        return false;
    }
    *this = job;
    return true;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CJobDefinition::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CJobDefinition( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CJobDefinition::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetId( const CString& id )
{GUCEF_TRACE;

    m_id = id;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobDefinition::GetId( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetTitle( const CString& title )
{GUCEF_TRACE;

    m_title = title;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobDefinition::GetTitle( void ) const
{GUCEF_TRACE;

    return m_title;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetDescription( const CString& description )
{GUCEF_TRACE;

    m_description = description;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobDefinition::GetDescription( void ) const
{GUCEF_TRACE;

    return m_description;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetTaskTypeName( const CString& taskTypeName )
{GUCEF_TRACE;

    m_taskTypeName = taskTypeName;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobDefinition::GetTaskTypeName( void ) const
{GUCEF_TRACE;

    return m_taskTypeName;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetPayload( const CDataNode& payload )
{GUCEF_TRACE;

    m_payload = payload;
}

/*-------------------------------------------------------------------------*/

const CDataNode&
CJobDefinition::GetPayload( void ) const
{GUCEF_TRACE;

    return m_payload;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetTrigger( const CJobTrigger& trigger )
{GUCEF_TRACE;

    m_trigger = trigger;
}

/*-------------------------------------------------------------------------*/

const CJobTrigger&
CJobDefinition::GetTrigger( void ) const
{GUCEF_TRACE;

    return m_trigger;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetLaneName( const CString& laneName )
{GUCEF_TRACE;

    m_laneName = laneName;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobDefinition::GetLaneName( void ) const
{GUCEF_TRACE;

    return m_laneName;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetExecutionTarget( TJobExecutionTarget executionTarget )
{GUCEF_TRACE;

    m_executionTarget = executionTarget;
}

/*-------------------------------------------------------------------------*/

TJobExecutionTarget
CJobDefinition::GetExecutionTarget( void ) const
{GUCEF_TRACE;

    return m_executionTarget;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetThreadPoolName( const CString& threadPoolName )
{GUCEF_TRACE;

    m_threadPoolName = threadPoolName;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobDefinition::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetPriority( UInt32 priority )
{GUCEF_TRACE;

    m_priority = priority;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobDefinition::GetPriority( void ) const
{GUCEF_TRACE;

    return m_priority;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetConcurrency( TJobConcurrency concurrency )
{GUCEF_TRACE;

    m_concurrency = concurrency;
}

/*-------------------------------------------------------------------------*/

TJobConcurrency
CJobDefinition::GetConcurrency( void ) const
{GUCEF_TRACE;

    return m_concurrency;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetMaxParallelRuns( UInt32 maxParallelRuns )
{GUCEF_TRACE;

    m_maxParallelRuns = maxParallelRuns;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobDefinition::GetMaxParallelRuns( void ) const
{GUCEF_TRACE;

    return m_maxParallelRuns;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetMissedRunPolicy( TJobMissedRunPolicy missedRunPolicy )
{GUCEF_TRACE;

    m_missedRunPolicy = missedRunPolicy;
}

/*-------------------------------------------------------------------------*/

TJobMissedRunPolicy
CJobDefinition::GetMissedRunPolicy( void ) const
{GUCEF_TRACE;

    return m_missedRunPolicy;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetMaxCatchUpRuns( UInt32 maxCatchUpRuns )
{GUCEF_TRACE;

    m_maxCatchUpRuns = maxCatchUpRuns;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobDefinition::GetMaxCatchUpRuns( void ) const
{GUCEF_TRACE;

    return m_maxCatchUpRuns;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetMaxLatenessMs( UInt64 maxLatenessMs )
{GUCEF_TRACE;

    m_maxLatenessMs = maxLatenessMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobDefinition::GetMaxLatenessMs( void ) const
{GUCEF_TRACE;

    return m_maxLatenessMs;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetTimeoutMs( UInt64 timeoutMs )
{GUCEF_TRACE;

    m_timeoutMs = timeoutMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobDefinition::GetTimeoutMs( void ) const
{GUCEF_TRACE;

    return m_timeoutMs;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetRetryPolicy( const CJobRetryPolicy& retryPolicy )
{GUCEF_TRACE;

    m_retryPolicy = retryPolicy;
}

/*-------------------------------------------------------------------------*/

const CJobRetryPolicy&
CJobDefinition::GetRetryPolicy( void ) const
{GUCEF_TRACE;

    return m_retryPolicy;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetFlags( UInt32 flags )
{GUCEF_TRACE;

    m_flags = flags;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobDefinition::GetFlags( void ) const
{GUCEF_TRACE;

    return m_flags;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetRemoteActions( const TJobControlActionSet& remoteActions )
{GUCEF_TRACE;

    m_remoteActions = remoteActions;
}

/*-------------------------------------------------------------------------*/

const TJobControlActionSet&
CJobDefinition::GetRemoteActions( void ) const
{GUCEF_TRACE;

    return m_remoteActions;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetRemoteCustomActions( const CString::StringSet& remoteCustomActions )
{GUCEF_TRACE;

    m_remoteCustomActions = remoteCustomActions;
}

/*-------------------------------------------------------------------------*/

const CString::StringSet&
CJobDefinition::GetRemoteCustomActions( void ) const
{GUCEF_TRACE;

    return m_remoteCustomActions;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetOrigin( TJobOrigin origin )
{GUCEF_TRACE;

    m_origin = origin;
}

/*-------------------------------------------------------------------------*/

TJobOrigin
CJobDefinition::GetOrigin( void ) const
{GUCEF_TRACE;

    return m_origin;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetSubmittedBy( const CString& submittedBy )
{GUCEF_TRACE;

    m_submittedBy = submittedBy;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobDefinition::GetSubmittedBy( void ) const
{GUCEF_TRACE;

    return m_submittedBy;
}

/*-------------------------------------------------------------------------*/

void
CJobDefinition::SetSubmittedAtUtcMs( Int64 submittedAtUtcMs )
{GUCEF_TRACE;

    m_submittedAtUtcMs = submittedAtUtcMs;
}

/*-------------------------------------------------------------------------*/

Int64
CJobDefinition::GetSubmittedAtUtcMs( void ) const
{GUCEF_TRACE;

    return m_submittedAtUtcMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
