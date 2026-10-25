/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_COAUTH2FLOW_H
#define GUCEF_AUTH_COAUTH2FLOW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_AUTH_IAUTHFLOWENGINE_H
#include "gucefAUTH_IAuthFlowEngine.h"
#define GUCEF_AUTH_IAUTHFLOWENGINE_H
#endif /* GUCEF_AUTH_IAUTHFLOWENGINE_H ? */

#ifndef GUCEF_AUTH_COAUTH2PROTOCOL_H
#include "gucefAUTH_COAuth2Protocol.h"
#define GUCEF_AUTH_COAUTH2PROTOCOL_H
#endif /* GUCEF_AUTH_COAUTH2PROTOCOL_H ? */

#ifndef GUCEF_AUTH_COAUTH2HTTP_H
#include "gucefAUTH_COAuth2Http.h"
#define GUCEF_AUTH_COAUTH2HTTP_H
#endif /* GUCEF_AUTH_COAUTH2HTTP_H ? */

#ifndef GUCEF_AUTH_CLOOPBACKREDIRECTLISTENER_H
#include "gucefAUTH_CLoopbackRedirectListener.h"
#define GUCEF_AUTH_CLOOPBACKREDIRECTLISTENER_H
#endif /* GUCEF_AUTH_CLOOPBACKREDIRECTLISTENER_H ? */

#ifndef GUCEF_AUTH_COPENID2_H
#include "gucefAUTH_COpenId2.h"
#define GUCEF_AUTH_COPENID2_H
#endif /* GUCEF_AUTH_COPENID2_H ? */

#ifndef GUCEF_AUTH_CJWT_H
#include "gucefAUTH_CJwt.h"
#define GUCEF_AUTH_CJWT_H
#endif /* GUCEF_AUTH_CJWT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class COAuth2Engine;

/** What a flow is for */
enum EOAuth2FlowKind
{
    OAUTH2FLOW_AUTHORIZATION_CODE = 0,      /**< authorization code with PKCE and a redirect */
    OAUTH2FLOW_DEVICE_CODE,                 /**< device authorization grant */
    OAUTH2FLOW_OPENID2,                     /**< OpenID 2.0 in the browser */
    OAUTH2FLOW_REFRESH                      /**< renewal with a refresh token */
};
typedef enum EOAuth2FlowKind TOAuth2FlowKind;

/**
 *  One login of the flow engine: a state machine that lives on the thread that pumps the pulse generator. It waits for
 *  the user, for the browser to come back to the loopback listener, for the poll time of a device flow and for HTTP
 *  requests that run as tasks of the thread pool, which it looks at on a timer. It ends the login exactly once through
 *  the sink, and ends it as cancelled when the requester asks for that, as timed out at the deadline, and quietly when
 *  the provider it works for goes away.
 */
class GUCEF_AUTH_EXPORT_CPP COAuth2Flow : public CORE::CObserver       ,
                                          public IAuthFlowSinkWatcher
{
    public:

    COAuth2Flow( COAuth2Engine& engine                      ,
                 TOAuth2FlowKind kind                       ,
                 const CAuthProviderDescriptor& descriptor  ,
                 const CAuthProviderSettings& settings      ,
                 const CAuthOptions& options                ,
                 IAuthFlowSink& sink                        ,
                 const CORE::CFutureResultSource& result    );

    virtual ~COAuth2Flow() GUCEF_VIRTUAL_OVERRIDE;

    /** Starts the login. False when it cannot start; nothing was raised or reported then */
    bool Start( void );

    /** For a refresh: who and with what */
    void SetRefreshInput( const CAccountRef& account, const CSecretString& refreshToken );

    bool IsFinished( void ) const;

    /** Takes the address the login was redirected to; true when this login took it */
    bool SubmitRedirectUrl( const CORE::CString& redirectUrl );

    /** Ends the login as cancelled */
    void Cancel( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnFlowSinkDestroyed( IAuthFlowSink* sink ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< COAuth2Flow > TEventCallback;

    void OnTimerUpdate( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );

    enum EStage
    {
        STAGE_NEW = 0,
        STAGE_DISCOVERY,            /**< the discovery document is on its way */
        STAGE_AUTHORIZATION,        /**< the browser has been sent, the callback is awaited */
        STAGE_TOKEN,                /**< the token endpoint is asked (code, refresh or device poll) */
        STAGE_KEYS,                 /**< the key set of the provider is on its way */
        STAGE_USERINFO,
        STAGE_DEVICE_REQUEST,
        STAGE_DEVICE_WAIT,          /**< the code is shown, the next poll is not due */
        STAGE_OPENID_CHECK,         /**< check_authentication is on its way */
        STAGE_DONE
    };

    COAuth2Flow( const COAuth2Flow& src );              /**< not implemented */
    COAuth2Flow& operator=( const COAuth2Flow& src );   /**< not implemented */

    void OnTimer( void );
    void CheckPending( void );
    void OnHttpDone( bool ok, CAuthHttpResponse& response );

    bool BeginResolved( void );
    bool BeginAuthorizationCode( void );
    bool BeginOpenId2( void );
    bool BeginDevice( void );
    bool BeginRefresh( void );
    bool PrepareRedirect( CORE::CString& redirectUri, CORE::CString& originForRealm, const CORE::CString& stateParameter = "state" );
    void AnnounceUrl( const CORE::CString& url );

    bool StartRequest( const CAuthHttpRequest& request, EStage stage );
    bool StartDiscovery( void );
    bool StartTokenRequest( const CORE::CString::StringMap& parameters );

    void OnDiscoveryDone( bool ok, CAuthHttpResponse& response );
    void OnCallback( const CORE::CString::StringMap& parameters );
    void OnTokenDone( bool ok, CAuthHttpResponse& response );
    void OnKeysDone( bool ok, CAuthHttpResponse& response );
    void OnUserInfoDone( bool ok, CAuthHttpResponse& response );
    void OnDeviceRequestDone( bool ok, CAuthHttpResponse& response );
    void OnOpenIdCheckDone( bool ok, CAuthHttpResponse& response );

    void ProcessTokens( void );
    void ValidateIdToken( void );
    void AfterIdentity( void );
    void StartUserInfo( void );
    void Complete( void );
    void SendDevicePoll( void );

    void Fail( TAuthFailureReason reason, const char* message );

    /** Ends the login because a request got no usable answer: the reason and the sentence follow from what went wrong (see CAuthHttpResponse::DescribeFailure()) */
    void FailHttp( const CAuthHttpResponse& response, const char* step );
    void Finish( void );
    UInt64 Now( void ) const;

    private:

    COAuth2Engine& m_engine;
    TOAuth2FlowKind m_kind;
    CAuthProviderDescriptor m_descriptor;
    CAuthProviderSettings m_settings;
    CAuthOptions m_options;
    IAuthFlowSink* m_sink;
    CORE::CFutureResultSource m_result;
    CORE::CTimer m_timer;

    EStage m_stage;
    COAuth2Resolved m_resolved;
    COidcMetadata m_metadata;
    bool m_hasMetadata;
    UInt64 m_deadlineMs;

    CORE::CFutureResult m_pending;
    bool m_hasPending;
    EStage m_pendingStage;

    // authorization code and OpenID 2.0
    CLoopbackRedirectListener* m_listener;
    CORE::CString m_state;
    CORE::CString m_nonce;
    CORE::CString m_verifier;
    CORE::CString m_redirectUri;
    CORE::CString m_returnTo;
    CSecretString m_code;
    COpenId2Assertion m_assertion;
    CORE::CString m_assertionNonce;
    bool m_nonceReserved;

    // device
    CSecretString m_deviceCode;
    UInt64 m_deviceExpiresAtMs;
    UInt32 m_intervalMs;
    UInt64 m_nextPollAtMs;
    UInt64 m_lastPollAtMs;
    UInt32 m_pollFailures;
    bool m_pollInFlight;

    // refresh
    CAccountRef m_account;
    CSecretString m_refreshToken;

    // the outcome so far
    COAuth2TokenResponse m_tokens;
    CORE::CString::StringMap m_claims;
    CORE::CString m_tenantId;
    bool m_hasIdentity;
    bool m_keysFetched;
    bool m_checkNonce;
    UInt64 m_idTokenExpiresAtMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_COAUTH2FLOW_H ? */
