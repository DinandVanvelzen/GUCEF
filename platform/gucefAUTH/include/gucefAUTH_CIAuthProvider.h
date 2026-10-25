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

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#define GUCEF_AUTH_CIAUTHPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_AUTH_CAUTHRESULT_H
#include "gucefAUTH_CAuthResult.h"
#define GUCEF_AUTH_CAUTHRESULT_H
#endif /* GUCEF_AUTH_CAUTHRESULT_H ? */

#ifndef GUCEF_AUTH_CAUTHEVENTDATA_H
#include "gucefAUTH_CAuthEventData.h"
#define GUCEF_AUTH_CAUTHEVENTDATA_H
#endif /* GUCEF_AUTH_CAUTHEVENTDATA_H ? */

#ifndef GUCEF_AUTH_CAUTHUNAVAILABLEPROVIDER_H
#include "gucefAUTH_CAuthUnavailableProvider.h"
#define GUCEF_AUTH_CAUTHUNAVAILABLEPROVIDER_H
#endif /* GUCEF_AUTH_CAUTHUNAVAILABLEPROVIDER_H ? */

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

/**
 *  One way to log in with an external identity: "google", "aws-cognito", a platform session of an SDK plugin.
 *  A provider is registered in CAuthGlobal::GetProviderFactory() under its name, built in or by a plugin.
 *
 *  Threading. The operations that start a login return at once and give a CFutureResult; the work runs wherever the
 *  provider decides (a pool task, a socket on the application's pulse generator). An implementation ends a login only
 *  through CompleteLogin() and FailLogin(), which end the future and raise the event; the events are delivered on the
 *  thread that pumps the pulse generator of the provider, never inside the call that started the login, so observers
 *  do not have to be thread safe. Verify() runs on the calling thread. A requester that wants to cancel calls
 *  RequestCancellation() on the future; the provider looks at CFutureResultSource::IsCancellationRequested() and ends
 *  the login with FailLogin( AUTHFAIL_CANCELLED ).
 *
 *  A provider does not hold on to tokens after a login ended: the application stores them through an ICredentialStore.
 */
class GUCEF_AUTH_EXPORT_CPP CIAuthProvider : public CORE::CTSGNotifier ,
                                             public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent ActionRequiredEvent;      /**< CAuthActionRequired: the user has to open a url or enter a code */
    static const CORE::CEvent LoginSucceededEvent;      /**< CAuthResult */
    static const CORE::CEvent LoginFailedEvent;         /**< CAuthFailure */

    static void RegisterEvents( void );

    CIAuthProvider( void );

    virtual ~CIAuthProvider();

    /** The name the provider is registered with in the factory and called by in the settings, for example "google" */
    virtual const CORE::CString& GetProviderName( void ) const = 0;

    /** EAuthProviderCaps flags */
    virtual UInt32 GetCapabilities( void ) const = 0;

    /** Whether a login with the provider can be tried here and now: it is configured, the platform session or codec it needs is there */
    virtual bool IsAvailable( void ) const = 0;

    /**
     *  Logs in without the user doing anything, with a platform session. False when the provider has no such session;
     *  the future then has no future. Otherwise the outcome comes through the future and the events.
     */
    virtual bool TrySilentLogin( CORE::CFutureResult& future ) = 0;

    /**
     *  Starts an interactive login. ActionRequiredEvent tells the application what the user has to see. False when the
     *  login cannot start, the future has no future then.
     */
    virtual bool BeginLogin( const CAuthOptions& options, CORE::CFutureResult& future ) = 0;

    /** Renews a login from a refresh token. False when the provider cannot refresh or cannot start. Default: not supported */
    virtual bool Refresh( const CAccountRef& account, const CSecretString& refreshToken, CORE::CFutureResult& future );

    /** Ends the login of the account at the provider as far as it can (revokes the token); the local session is gone when this returns. Default: nothing to do */
    virtual bool Logout( const CAccountRef& account, const CSecretString& refreshToken );

    /**
     *  The server role: checks a proof a client presented (signature, issuer, audience, expiry) and fills the account of
     *  the result. Runs on the calling thread and can wait for the network on a cache miss of the provider's keys, so
     *  servers call it from a worker. Default: false
     */
    virtual bool Verify( const CAuthProof& proof, CAuthResult& result );

    /**
     *  Why the provider cannot be offered now: fills 'unavailable' (reason, a line for the person and the key of the
     *  configuration that fills the gap) and returns true when the provider can say. False when it is available, and
     *  for a provider that does not explain itself: the manager lists a provider that is not offered only when it explains
     *  why (CAuthManager::GetUnavailableProviders()). Default: false
     */
    virtual bool GetUnavailability( CAuthUnavailableProvider& unavailable ) const;

    /**
     *  A hint of who this machine says the person is at the provider, for an application to make the offer to log in
     *  concrete ("<provider>: <name> detected"). True when something was detected: displayName then has the name of the person
     *  at the provider when it is known (empty when only the provider's client was found) and isRunning tells whether the
     *  client of the provider is running. A hint is read from the machine and not verified: it is never an account, is never
     *  linked or stored, and never leaves the process. Logging in is what proves who the person is. Default: nothing detected
     */
    virtual bool GetDetectedAccountHint( CORE::CString& displayName, bool& isRunning ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  Ends the login with the result: sets the provider name, ends the future and raises LoginSucceededEvent.
     *  An invalid result (no account) ends as AUTHFAIL_INVALID_RESPONSE. False when the future had ended already.
     */
    bool CompleteLogin( CORE::CFutureResultSource& source, const CAuthResult& result );

    /** Ends the login as failed (a cancellation ends the future as cancelled) and raises LoginFailedEvent. False when the future had ended already */
    bool FailLogin( CORE::CFutureResultSource& source, TAuthFailureReason reason, const CORE::CString& message );

    /** Raises ActionRequiredEvent with the provider name filled in */
    void RaiseActionRequired( const CAuthActionRequired& action );

    private:

    CIAuthProvider( const CIAuthProvider& src );
    CIAuthProvider& operator=( const CIAuthProvider& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIAuthProvider, MT::CMutex > CAuthProviderFactory;
typedef CAuthProviderFactory::TProductPtr                                    CIAuthProviderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */
