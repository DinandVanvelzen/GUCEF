/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOM_CMcpRequestContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The codes a handler may answer with: -32602, -32603 and anything outside the range JSON-RPC and MCP reserve */
Int32
AllowedHandlerErrorCode( Int32 code )
{GUCEF_TRACE;

    if ( MCP_ERROR_INVALID_PARAMS == code || MCP_ERROR_INTERNAL == code )
        return code;
    if ( code >= -32768 && code <= -32000 )
        return MCP_ERROR_INTERNAL;
    return code;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpRequestContext::SProgress::SProgress( void )
    : progress( 0.0 )
    , total( 0.0 )
    , hasTotal( false )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpRequestContext::CMcpRequestContext( const CORE::CString& method         ,
                                        const CORE::CString& targetName     ,
                                        const CORE::CVariant& requestId     ,
                                        EMcpProtocolEra era                 ,
                                        const CMcpPrincipal& principal      ,
                                        const CMcpRequestMeta& meta         ,
                                        UInt64 channelId                    )
    : CORE::CTSharedObjCreator< CMcpRequestContext, MT::CMutex >( this )
    , m_method( method )
    , m_targetName( targetName )
    , m_requestId( requestId )
    , m_era( era )
    , m_principal( principal )
    , m_meta( meta )
    , m_channelId( channelId )
    , m_startTimeInMs( CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() )
    , m_hasInputResponses( false )
    , m_inputResponses( GUCEF_DATATYPE_OBJECT )
    , m_hasRequestState( false )
    , m_requestState()
    , m_stateNonce()
    , m_lock()
    , m_cancelled( 0 )
    , m_completed( 0 )
    , m_pulseGenerator()
    , m_listener( GUCEF_NULL )
    , m_ended( false )
    , m_newsPending( false )
    , m_completionKind( COMPLETION_NONE )
    , m_toolResult()
    , m_resourceContents()
    , m_promptResult()
    , m_inputRequired()
    , m_hasRecordedError( false )
    , m_errorCode( 0 )
    , m_errorMessage()
    , m_hasErrorData( false )
    , m_errorData()
    , m_hasAcceptedProgress( false )
    , m_lastAcceptedProgress( 0.0 )
    , m_progressPending( false )
    , m_pendingProgress()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpRequestContextPtr
CMcpRequestContext::Create( const CORE::CString& method         ,
                            const CORE::CString& targetName     ,
                            const CORE::CVariant& requestId     ,
                            EMcpProtocolEra era                 ,
                            const CMcpPrincipal& principal      ,
                            const CMcpRequestMeta& meta         ,
                            UInt64 channelId                    )
{GUCEF_TRACE;

    CMcpRequestContext* context = GUCEF_NEW CMcpRequestContext( method, targetName, requestId, era, principal, meta, channelId );
    return context->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

CMcpRequestContextPtr
CMcpRequestContext::GetSharedPtr( void ) const
{GUCEF_TRACE;

    return CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

CMcpRequestContext::~CMcpRequestContext()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestContext::GetMethod( void ) const
{GUCEF_TRACE;

    return m_method;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestContext::GetTargetName( void ) const
{GUCEF_TRACE;

    return m_targetName;
}

/*-------------------------------------------------------------------------*/

const CORE::CVariant&
CMcpRequestContext::GetRequestId( void ) const
{GUCEF_TRACE;

    return m_requestId;
}

/*-------------------------------------------------------------------------*/

EMcpProtocolEra
CMcpRequestContext::GetEra( void ) const
{GUCEF_TRACE;

    return m_era;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestContext::GetProtocolVersion( void ) const
{GUCEF_TRACE;

    return m_meta.GetProtocolVersion();
}

/*-------------------------------------------------------------------------*/

const CMcpPrincipal&
CMcpRequestContext::GetPrincipal( void ) const
{GUCEF_TRACE;

    return m_principal;
}

/*-------------------------------------------------------------------------*/

const CMcpRequestMeta&
CMcpRequestContext::GetMeta( void ) const
{GUCEF_TRACE;

    return m_meta;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::HasClientCapability( const CORE::CString& capabilityPath ) const
{GUCEF_TRACE;

    return m_meta.HasCapability( capabilityPath );
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::HasInputResponses( void ) const
{GUCEF_TRACE;

    return m_hasInputResponses;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CMcpRequestContext::GetInputResponses( void ) const
{GUCEF_TRACE;

    return m_inputResponses;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::HasRequestState( void ) const
{GUCEF_TRACE;

    return m_hasRequestState;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CMcpRequestContext::GetRequestState( void ) const
{GUCEF_TRACE;

    return m_requestState;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestContext::GetStateNonce( void ) const
{GUCEF_TRACE;

    return m_stateNonce;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::SetInputResponses( const CORE::CDataNode& inputResponses )
{GUCEF_TRACE;

    m_inputResponses = inputResponses;
    m_inputResponses.SetName( CORE::CString::Empty );
    m_hasInputResponses = true;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::SetRequestState( const CORE::CDataNode& state, const CORE::CString& stateNonce )
{GUCEF_TRACE;

    m_requestState = state;
    m_requestState.SetName( CORE::CString::Empty );
    m_stateNonce = stateNonce;
    m_hasRequestState = true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::IsCancellationRequested( void ) const
{GUCEF_TRACE;

    return 0 != m_cancelled.Load();
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::RequestCancellation( void )
{GUCEF_TRACE;

    m_cancelled.Store( 1 );
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CMcpRequestContext::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::SetListener( CIMcpRequestContextListener* listener )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_listener = listener;
    m_newsPending = false;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::TellListener( void )
{GUCEF_TRACE;

    // called with the lock held
    if ( GUCEF_NULL != m_listener && !m_newsPending )
    {
        m_newsPending = true;
        m_listener->OnRequestContextNews( *this );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::AcknowledgeNews( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_newsPending = false;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::MarkEnded( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_ended = true;
    m_progressPending = false;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::IsEnded( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_ended;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::ReportProgress( Float64 progress, Float64 total, const CORE::CString& message )
{GUCEF_TRACE;

    if ( !m_meta.HasProgressToken() || progress != progress )
        return false;

    MT::CScopeMutex lock( m_lock );
    if ( m_ended || 0 != m_completed.Load() || 0 != m_cancelled.Load() )
        return false;
    if ( m_hasAcceptedProgress && progress <= m_lastAcceptedProgress )
        return false;       // the protocol requires the value to increase with every notification

    m_hasAcceptedProgress = true;
    m_lastAcceptedProgress = progress;
    m_pendingProgress.progress = progress;
    m_pendingProgress.hasTotal = total >= 0.0 && total == total;
    m_pendingProgress.total = m_pendingProgress.hasTotal ? total : 0.0;
    m_pendingProgress.message = message;
    m_progressPending = true;
    TellListener();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::TakePendingProgress( SProgress& progress )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_progressPending )
        return false;
    progress = m_pendingProgress;
    m_progressPending = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::HasPendingProgress( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_progressPending;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::BeginCompletion( ECompletionKind kind )
{GUCEF_TRACE;

    // called with the lock held
    if ( 0 != m_completed.Load() || m_ended )
        return false;
    if ( COMPLETION_PROTOCOL_ERROR != kind && COMPLETION_INPUT_REQUIRED != kind )
    {
        // a result of the kind the method gives, nothing else
        ECompletionKind expected = COMPLETION_NONE;
        if ( m_method == CMcpMethods::ToolsCall )
            expected = COMPLETION_TOOL_RESULT;
        else
        if ( m_method == CMcpMethods::ResourcesRead )
            expected = COMPLETION_RESOURCE_CONTENTS;
        else
        if ( m_method == CMcpMethods::PromptsGet )
            expected = COMPLETION_PROMPT_RESULT;
        if ( kind != expected )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpRequestContext: A completion of the wrong kind for a " << m_method << " request was refused" );
            return false;
        }
    }
    m_completionKind = kind;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::EndCompletion( void )
{GUCEF_TRACE;

    // called with the lock held, after the outcome was stored
    m_completed.Store( 1 );
    m_progressPending = false;
    TellListener();
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::CompleteWithToolResult( const CMcpToolResult& result )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !BeginCompletion( COMPLETION_TOOL_RESULT ) )
        return false;
    m_toolResult = result;
    EndCompletion();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::CompleteWithResourceContents( const CMcpResourceContents& contents )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !BeginCompletion( COMPLETION_RESOURCE_CONTENTS ) )
        return false;
    m_resourceContents = contents;
    EndCompletion();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::CompleteWithPromptResult( const CMcpPromptResult& result )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !BeginCompletion( COMPLETION_PROMPT_RESULT ) )
        return false;
    m_promptResult = result;
    EndCompletion();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::CompleteWithInputRequired( const CMcpInputRequired& inputRequired )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !BeginCompletion( COMPLETION_INPUT_REQUIRED ) )
        return false;
    m_inputRequired = inputRequired;
    EndCompletion();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestContext::SetProtocolError( Int32 code, const CORE::CString& message, const CORE::CDataNode* data )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( 0 != m_completed.Load() )
        return;
    Int32 allowed = AllowedHandlerErrorCode( code );
    if ( allowed != code )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpRequestContext: A handler of " << m_method << " used the reserved error code " << code << ", it is sent as " << allowed );
    m_hasRecordedError = true;
    m_errorCode = allowed;
    m_errorMessage = message.IsNULLOrEmpty() ? CORE::CString( "Internal error" ) : message;
    m_hasErrorData = GUCEF_NULL != data;
    if ( m_hasErrorData )
    {
        m_errorData = *data;
        m_errorData.SetName( CORE::CString::Empty );
    }
    else
    {
        m_errorData = CORE::CDataNode();
    }
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::CompleteWithProtocolError( Int32 code, const CORE::CString& message, const CORE::CDataNode* data )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( 0 != m_completed.Load() || m_ended )
        return false;
    SetProtocolError( code, message, data );
    if ( !BeginCompletion( COMPLETION_PROTOCOL_ERROR ) )
        return false;
    EndCompletion();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::CompleteWithRecordedProtocolError( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( 0 != m_completed.Load() || m_ended )
        return false;
    if ( !m_hasRecordedError )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpRequestContext: A handler of " << m_method << " answered with a protocol error without setting one" );
        SetProtocolError( MCP_ERROR_INTERNAL, "Internal error" );
    }
    if ( !BeginCompletion( COMPLETION_PROTOCOL_ERROR ) )
        return false;
    EndCompletion();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::IsCompleted( void ) const
{GUCEF_TRACE;

    return 0 != m_completed.Load();
}

/*-------------------------------------------------------------------------*/

CMcpRequestContext::ECompletionKind
CMcpRequestContext::GetCompletionKind( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_completionKind;
}

/*-------------------------------------------------------------------------*/

const CMcpToolResult&
CMcpRequestContext::GetToolResult( void ) const
{GUCEF_TRACE;

    return m_toolResult;
}

/*-------------------------------------------------------------------------*/

const CMcpResourceContents&
CMcpRequestContext::GetResourceContents( void ) const
{GUCEF_TRACE;

    return m_resourceContents;
}

/*-------------------------------------------------------------------------*/

const CMcpPromptResult&
CMcpRequestContext::GetPromptResult( void ) const
{GUCEF_TRACE;

    return m_promptResult;
}

/*-------------------------------------------------------------------------*/

const CMcpInputRequired&
CMcpRequestContext::GetInputRequired( void ) const
{GUCEF_TRACE;

    return m_inputRequired;
}

/*-------------------------------------------------------------------------*/

Int32
CMcpRequestContext::GetProtocolErrorCode( void ) const
{GUCEF_TRACE;

    return m_errorCode;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestContext::GetProtocolErrorMessage( void ) const
{GUCEF_TRACE;

    return m_errorMessage;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestContext::HasProtocolErrorData( void ) const
{GUCEF_TRACE;

    return m_hasErrorData;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CMcpRequestContext::GetProtocolErrorData( void ) const
{GUCEF_TRACE;

    return m_errorData;
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpRequestContext::GetChannelId( void ) const
{GUCEF_TRACE;

    return m_channelId;
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpRequestContext::GetStartTimeInMs( void ) const
{GUCEF_TRACE;

    return m_startTimeInMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
