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

#ifndef GUCEF_COM_CMCPREQUESTCONTEXT_H
#define GUCEF_COM_CMCPREQUESTCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_COM_CMCPPRINCIPAL_H
#include "gucefCOM_CMcpPrincipal.h"
#define GUCEF_COM_CMCPPRINCIPAL_H
#endif /* GUCEF_COM_CMCPPRINCIPAL_H ? */

#ifndef GUCEF_COM_CMCPREQUESTMETA_H
#include "gucefCOM_CMcpRequestMeta.h"
#define GUCEF_COM_CMCPREQUESTMETA_H
#endif /* GUCEF_COM_CMCPREQUESTMETA_H ? */

#ifndef GUCEF_COM_CMCPTOOLRESULT_H
#include "gucefCOM_CMcpToolResult.h"
#define GUCEF_COM_CMCPTOOLRESULT_H
#endif /* GUCEF_COM_CMCPTOOLRESULT_H ? */

#ifndef GUCEF_COM_CMCPRESOURCECONTENTS_H
#include "gucefCOM_CMcpResourceContents.h"
#define GUCEF_COM_CMCPRESOURCECONTENTS_H
#endif /* GUCEF_COM_CMCPRESOURCECONTENTS_H ? */

#ifndef GUCEF_COM_CMCPPROMPTRESULT_H
#include "gucefCOM_CMcpPromptResult.h"
#define GUCEF_COM_CMCPPROMPTRESULT_H
#endif /* GUCEF_COM_CMCPPROMPTRESULT_H ? */

#ifndef GUCEF_COM_CMCPINPUTREQUIRED_H
#include "gucefCOM_CMcpInputRequired.h"
#define GUCEF_COM_CMCPINPUTREQUIRED_H
#endif /* GUCEF_COM_CMCPINPUTREQUIRED_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMcpRequestContext;
typedef CORE::CTSharedPtr< CMcpRequestContext, MT::CMutex > CMcpRequestContextPtr;

/**
 *  Who drives a request context: told when the context has news (a completion, progress). It is called with the
 *  lock of the context held, from any thread, so it must do no more than queue the news for its own thread.
 */
class GUCEF_COM_PUBLIC_CPP CIMcpRequestContextListener
{
    public:

    CIMcpRequestContextListener( void ) {}

    virtual ~CIMcpRequestContextListener() {}

    virtual void OnRequestContextNews( CMcpRequestContext& context ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  One request in flight, made by the server and shared (CMcpRequestContextPtr): pool tasks and deferred handlers
 *  may hold it after Execute() returned. What describes the request is fixed before the handler runs.
 *  Cancellation, progress and completion are thread safe: they go into a small locked mailbox which the server
 *  empties on its protocol thread, so a handler on any thread never touches a channel.
 */
class GUCEF_COM_PUBLIC_CPP CMcpRequestContext : public CORE::CTSharedObjCreator< CMcpRequestContext, MT::CMutex >
{
    public:

    enum ECompletionKind
    {
        COMPLETION_NONE             ,
        COMPLETION_TOOL_RESULT      ,
        COMPLETION_RESOURCE_CONTENTS,
        COMPLETION_PROMPT_RESULT    ,
        COMPLETION_INPUT_REQUIRED   ,
        COMPLETION_PROTOCOL_ERROR
    };

    /** A progress value waiting to be sent */
    struct GUCEF_COM_PUBLIC_CPP SProgress
    {
        Float64 progress;
        Float64 total;
        bool hasTotal;
        CORE::CString message;

        SProgress( void );
    };

    /** Makes a context held by a shared pointer, which GetSharedPtr() gives to whoever needs to keep it */
    static CMcpRequestContextPtr Create( const CORE::CString& method         ,
                                         const CORE::CString& targetName     ,
                                         const CORE::CVariant& requestId     ,
                                         EMcpProtocolEra era                 ,
                                         const CMcpPrincipal& principal      ,
                                         const CMcpRequestMeta& meta         ,
                                         UInt64 channelId                    );

    /**
     *  A shared pointer to this context: a DEFERRED handler keeps it to complete the context later, after Execute()
     *  returned. The server holds the context while the request is in flight. The shared data is part of the object
     *  (CTSharedObjCreator), so every shared pointer made from the context counts towards the same object.
     */
    CMcpRequestContextPtr GetSharedPtr( void ) const;

    virtual ~CMcpRequestContext() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetMethod( void ) const;

    /** The tool or prompt name, or the resource URI */
    const CORE::CString& GetTargetName( void ) const;

    /** A string or an integer; empty for an in-process call */
    const CORE::CVariant& GetRequestId( void ) const;

    EMcpProtocolEra GetEra( void ) const;

    const CORE::CString& GetProtocolVersion( void ) const;

    const CMcpPrincipal& GetPrincipal( void ) const;

    /** Capabilities, client info and trace context of this request */
    const CMcpRequestMeta& GetMeta( void ) const;

    /** "elicitation", "elicitation/form", "sampling", "roots": a handler can choose another path before asking for input */
    bool HasClientCapability( const CORE::CString& capabilityPath ) const;

    bool HasInputResponses( void ) const;

    /** The inputResponses of a retry, as the client sent them: an object node; unknown keys are to be ignored */
    const CORE::CDataNode& GetInputResponses( void ) const;

    bool HasRequestState( void ) const;

    /** The state the handler gave with its input_required, verified and unsealed by the server */
    const CORE::CDataNode& GetRequestState( void ) const;

    /** The nonce CMcpInputRequired::SetStateNonce() put into the sealed state, as hexadecimal text; empty when none */
    const CORE::CString& GetStateNonce( void ) const;

    /** Thread safe. True once the client cancelled, the channel closed or the request timed out */
    bool IsCancellationRequested( void ) const;

    /**
     *  Thread safe. Reports progress to a client that asked for it with a progress token. The value must increase
     *  with every report. The server rate limits what it sends and keeps the latest value.
     *  @param total    below 0: unknown, left out
     *  @return false without a progress token, after the end of the request, or when the value does not increase
     */
    bool ReportProgress( Float64 progress, Float64 total = -1.0, const CORE::CString& message = CORE::CString::Empty );

    /** The pulse generator event driven work of this request runs on: see CMcpServer for which one that is */
    CORE::PulseGeneratorPtr GetPulseGenerator( void ) const;

    /** Thread safe; for DEFERRED handlers. The first completion wins, later ones and ones of the wrong kind return false */
    bool CompleteWithToolResult( const CMcpToolResult& result );
    bool CompleteWithResourceContents( const CMcpResourceContents& contents );
    bool CompleteWithPromptResult( const CMcpPromptResult& result );
    bool CompleteWithInputRequired( const CMcpInputRequired& inputRequired );
    bool CompleteWithProtocolError( Int32 code, const CORE::CString& message, const CORE::CDataNode* data = GUCEF_NULL );

    /**
     *  Records the JSON-RPC error a handler answers with when it returns MCP_EXECUTE_PROTOCOL_ERROR. Codes in the
     *  range reserved for JSON-RPC and MCP other than -32602 and -32603 are sent as -32603, since a handler may not
     *  use them: -32602 is for "does not exist" and invalid params, application codes lie outside -32768 to -32000.
     */
    void SetProtocolError( Int32 code, const CORE::CString& message, const CORE::CDataNode* data = GUCEF_NULL );

    /** The id of the channel, for logs and audit only: never a key for state, the protocol is stateless */
    UInt64 GetChannelId( void ) const;

    /** The monotonic time the request started at */
    UInt64 GetStartTimeInMs( void ) const;

    // What the server uses. Handlers do not call these.

    void SetInputResponses( const CORE::CDataNode& inputResponses );
    void SetRequestState( const CORE::CDataNode& state, const CORE::CString& stateNonce );
    void SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    /** GUCEF_NULL detaches: news is not told to anyone from then on */
    void SetListener( CIMcpRequestContextListener* listener );

    void RequestCancellation( void );

    /** The request was answered or abandoned: progress is refused from now on */
    void MarkEnded( void );
    bool IsEnded( void ) const;

    bool IsCompleted( void ) const;
    ECompletionKind GetCompletionKind( void ) const;

    /** The handler returned MCP_EXECUTE_PROTOCOL_ERROR: completes with what SetProtocolError() recorded, else -32603 */
    bool CompleteWithRecordedProtocolError( void );

    /** The outcomes, valid once IsCompleted() returned true: they do not change after that */
    const CMcpToolResult& GetToolResult( void ) const;
    const CMcpResourceContents& GetResourceContents( void ) const;
    const CMcpPromptResult& GetPromptResult( void ) const;
    const CMcpInputRequired& GetInputRequired( void ) const;
    Int32 GetProtocolErrorCode( void ) const;
    const CORE::CString& GetProtocolErrorMessage( void ) const;
    bool HasProtocolErrorData( void ) const;
    const CORE::CDataNode& GetProtocolErrorData( void ) const;

    /** The listener is told about news again after this; call it before looking at the news */
    void AcknowledgeNews( void );

    /** Takes the progress waiting to be sent, if any */
    bool TakePendingProgress( SProgress& progress );

    bool HasPendingProgress( void ) const;

    private:

    CMcpRequestContext( const CORE::CString& method         ,
                        const CORE::CString& targetName     ,
                        const CORE::CVariant& requestId     ,
                        EMcpProtocolEra era                 ,
                        const CMcpPrincipal& principal      ,
                        const CMcpRequestMeta& meta         ,
                        UInt64 channelId                    );

    CMcpRequestContext( const CMcpRequestContext& src );
    CMcpRequestContext& operator=( const CMcpRequestContext& src );

    bool BeginCompletion( ECompletionKind kind );
    void EndCompletion( void );
    void TellListener( void );

    CORE::CString m_method;
    CORE::CString m_targetName;
    CORE::CVariant m_requestId;
    EMcpProtocolEra m_era;
    CMcpPrincipal m_principal;
    CMcpRequestMeta m_meta;
    UInt64 m_channelId;
    UInt64 m_startTimeInMs;
    bool m_hasInputResponses;
    CORE::CDataNode m_inputResponses;
    bool m_hasRequestState;
    CORE::CDataNode m_requestState;
    CORE::CString m_stateNonce;

    mutable MT::CMutex m_lock;
    MT::CTAtomic< UInt32 > m_cancelled;
    MT::CTAtomic< UInt32 > m_completed;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CIMcpRequestContextListener* m_listener;
    bool m_ended;
    bool m_newsPending;
    ECompletionKind m_completionKind;
    CMcpToolResult m_toolResult;
    CMcpResourceContents m_resourceContents;
    CMcpPromptResult m_promptResult;
    CMcpInputRequired m_inputRequired;
    bool m_hasRecordedError;
    Int32 m_errorCode;
    CORE::CString m_errorMessage;
    bool m_hasErrorData;
    CORE::CDataNode m_errorData;
    bool m_hasAcceptedProgress;
    Float64 m_lastAcceptedProgress;
    bool m_progressPending;
    SProgress m_pendingProgress;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPREQUESTCONTEXT_H ? */
