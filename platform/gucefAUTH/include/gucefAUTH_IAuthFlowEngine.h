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

#ifndef GUCEF_AUTH_IAUTHFLOWENGINE_H
#define GUCEF_AUTH_IAUTHFLOWENGINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#include "gucefAUTH_CAuthProviderDescriptor.h"
#define GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H ? */

#ifndef GUCEF_AUTH_CAUTHSETTINGS_H
#include "gucefAUTH_CAuthSettings.h"
#define GUCEF_AUTH_CAUTHSETTINGS_H
#endif /* GUCEF_AUTH_CAUTHSETTINGS_H ? */

#ifndef GUCEF_AUTH_CAUTHRESULT_H
#include "gucefAUTH_CAuthResult.h"
#define GUCEF_AUTH_CAUTHRESULT_H
#endif /* GUCEF_AUTH_CAUTHRESULT_H ? */

#ifndef GUCEF_AUTH_CAUTHEVENTDATA_H
#include "gucefAUTH_CAuthEventData.h"
#define GUCEF_AUTH_CAUTHEVENTDATA_H
#endif /* GUCEF_AUTH_CAUTHEVENTDATA_H ? */

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

class IAuthFlowSink;

/**
 *  What became of an address handed to IAuthFlowEngine::SubmitRedirectUrl(), so that the application can say which check failed
 *  without repeating the address (it holds the code)
 */
enum ERedirectCheck
{
    REDIRECTCHECK_ACCEPTED = 0,         /**< a login took it (the login may still fail at the token endpoint) */
    REDIRECTCHECK_NO_LOGIN_WAITING,     /**< no login of the redirect mode custom-scheme or manual waits for an address */
    REDIRECTCHECK_MALFORMED,            /**< empty, too long, a query that cannot be read or has a parameter twice */
    REDIRECTCHECK_ADDRESS_MISMATCH,     /**< the address up to the query is not the redirect uri of a waiting login */
    REDIRECTCHECK_STATE_MISSING,        /**< the address has no state; the provider did not give it back */
    REDIRECTCHECK_STATE_MISMATCH,       /**< the state is not the one of the login */
    REDIRECTCHECK_NO_CODE,              /**< neither a code nor an error */
    REDIRECTCHECK_REFUSED               /**< the engine did not take it and does not say why */
};

typedef enum ERedirectCheck TRedirectCheck;

/** A stable lower case name of the outcome, for logs and an application that shows which check failed */
GUCEF_AUTH_EXPORT_CPP const char*
RedirectCheckToString( TRedirectCheck check );

/*-------------------------------------------------------------------------*/

/** Told when a sink goes away, so that an engine does not report to a provider that is gone */
class GUCEF_AUTH_EXPORT_CPP IAuthFlowSinkWatcher
{
    public:

    IAuthFlowSinkWatcher( void );

    virtual ~IAuthFlowSinkWatcher();

    /** The sink is being destroyed: it must not be used any more. The sink is only good for comparing the address */
    virtual void OnFlowSinkDestroyed( IAuthFlowSink* sink ) = 0;
};

/*-------------------------------------------------------------------------*/

/** What an engine reports to the provider it works for: the provider ends the login and raises the events (see CIAuthProvider) */
class GUCEF_AUTH_EXPORT_CPP IAuthFlowSink
{
    public:

    IAuthFlowSink( void );

    virtual ~IAuthFlowSink();

    /** The watcher is told when this sink is destroyed. Both are used on the thread that pumps the pulse generator */
    void AddWatcher( IAuthFlowSinkWatcher* watcher );

    void RemoveWatcher( IAuthFlowSinkWatcher* watcher );

    virtual bool OnLoginCompleted( CORE::CFutureResultSource& result, const CAuthResult& authResult ) = 0;

    virtual bool OnLoginFailed( CORE::CFutureResultSource& result, TAuthFailureReason reason, const CORE::CString& message ) = 0;

    virtual void OnActionRequired( const CAuthActionRequired& action ) = 0;

    private:

    IAuthFlowSink( const IAuthFlowSink& src );              /**< not implemented */
    IAuthFlowSink& operator=( const IAuthFlowSink& src );   /**< not implemented */

    private:

    GUCEF::vector< IAuthFlowSinkWatcher* > m_watchers;
};

/*-------------------------------------------------------------------------*/

/**
 *  The OAuth 2.0 / OpenID Connect / OpenID 2.0 machinery that CDescriptorAuthProvider hands the work to: discovery,
 *  authorization code with PKCE and a loopback redirect, device code, refresh, token validation. It works from a
 *  descriptor and the settings of the application and reports through the sink. It runs its work the way the module's
 *  threading rule says (pool tasks, sockets on the application's pulse generator) and ends every login it started
 *  through the sink exactly once. One engine serves all descriptor driven providers; CAuthGlobal::SetFlowEngine()
 *  makes it known.
 */
class GUCEF_AUTH_EXPORT_CPP IAuthFlowEngine
{
    public:

    IAuthFlowEngine( void );

    virtual ~IAuthFlowEngine();

    /** Whether the engine can run the provider as configured: the protocol and flows are supported and the settings have what the descriptor requires */
    virtual bool IsSupported( const CAuthProviderDescriptor& descriptor ,
                              const CAuthProviderSettings& settings     ) const = 0;

    /** Starts the interactive login the options ask for. False when it cannot start */
    virtual bool StartInteractive( const CAuthProviderDescriptor& descriptor ,
                                   const CAuthProviderSettings& settings     ,
                                   const CAuthOptions& options               ,
                                   IAuthFlowSink& sink                       ,
                                   CORE::CFutureResultSource& result         ) = 0;

    /** Starts the renewal of a login from a refresh token. False when it cannot start */
    virtual bool StartRefresh( const CAuthProviderDescriptor& descriptor ,
                               const CAuthProviderSettings& settings     ,
                               const CAccountRef& account                ,
                               const CSecretString& refreshToken         ,
                               IAuthFlowSink& sink                       ,
                               CORE::CFutureResultSource& result         ) = 0;

    /** Revokes the token at the provider as far as the provider can; does not wait for the answer */
    virtual bool Revoke( const CAuthProviderDescriptor& descriptor ,
                         const CAuthProviderSettings& settings     ,
                         const CAccountRef& account                ,
                         const CSecretString& refreshToken         ) = 0;

    /** The server role: checks the proof and fills the result */
    virtual bool Verify( const CAuthProviderDescriptor& descriptor ,
                         const CAuthProviderSettings& settings     ,
                         const CAuthProof& proof                   ,
                         CAuthResult& result                       ) = 0;

    /**
     *  Hands the engine the address the browser, or the operating system, was sent to at the end of a login with a redirect
     *  mode of "custom-scheme" or "manual": the redirect uri with its query. The engine finds the login by its state.
     *  Default: false, the engine has no such mode. True when a login took the address (it may still fail); false when no login
     *  waits for it, the state does not belong to a login or the address is not the redirect uri of it.
     *
     *  The address up to the query is compared with the redirect uri of the login after the syntax based normalization of RFC 3986
     *  section 6.2.2.1 and 6.2.3 for http and https: the scheme and the host in any case, a default port (80, 443) the same as none,
     *  an empty path the same as "/" (a browser shows https://127.0.0.1 as https://127.0.0.1/). Everything else stays exact: the path
     *  byte for byte, no user information, another port or host refused. Other schemes are compared byte for byte.
     */
    virtual bool SubmitRedirectUrl( const CORE::CString& redirectUrl );

    /** The same, and why an address was not taken. Default: the form without the reason, REDIRECTCHECK_REFUSED when it is false */
    virtual bool SubmitRedirectUrl( const CORE::CString& redirectUrl, TRedirectCheck& check );

    /**
     *  Whether the engine would run the provider but for one thing: what the engine asks of the provider goes over https and this build
     *  has no HTTP client for https right now (a plugin such as httpclientpluginCURL is not loaded). IsSupported() says no then and the
     *  provider is shown as not available, with that reason. Default: false.
     */
    virtual bool IsSecureTransportMissing( const CAuthProviderDescriptor& descriptor ,
                                           const CAuthProviderSettings& settings     ) const;

    private:

    IAuthFlowEngine( const IAuthFlowEngine& src );
    IAuthFlowEngine& operator=( const IAuthFlowEngine& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_IAUTHFLOWENGINE_H ? */
