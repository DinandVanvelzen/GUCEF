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

#ifndef GUCEF_AUTH_CMAILOAUTHCREDENTIALPROVIDER_H
#define GUCEF_AUTH_CMAILOAUTHCREDENTIALPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMAILCREDENTIALPROVIDER_H
#include "gucefCOM_CIMailCredentialProvider.h"
#define GUCEF_COM_CIMAILCREDENTIALPROVIDER_H
#endif /* GUCEF_COM_CIMAILCREDENTIALPROVIDER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

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
 *  Gives the mail clients (COM::CImapClient, CSmtpClient, CPop3Client) the OAuth 2.0 access token of a mailbox that was signed in to
 *  with a provider of the CAuthManager (Google, Microsoft), so that the clients log in with XOAUTH2 and the token is renewed when it
 *  has expired, without the mail module knowing about this module.
 *
 *  The access token is used from the session of the manager (the login that just happened) or from an earlier renewal; when it has
 *  expired, or the server refused it, the refresh token in the credential store is exchanged at the provider (CIAuthProvider::Refresh())
 *  and the client waits with CRED_PENDING for CredentialReadyEvent. A rotated refresh token is kept in the store. The refresh token
 *  never leaves the store and the module except in that request; no token is logged. A renewal that failed is not repeated before
 *  'retry delay' passed (the client gets CRED_FAILED meanwhile), so a revoked login does not make the clients hammer the provider.
 *
 *  The manager, the store and the provider are not owned. Everything runs on the thread that pumps the pulse generator.
 */
class GUCEF_AUTH_EXPORT_CPP CMailOAuthCredentialProvider : public COM::CIMailCredentialProvider
{
    public:

    static const UInt32 DefaultSkewMs = 60000;              /**< a token that expires within this time is renewed */
    static const UInt32 DefaultRetryDelayMs = 30000;
    static const UInt32 PollIntervalMs = 25;                /**< how often a renewal in flight is looked at */

    CMailOAuthCredentialProvider( void );

    virtual ~CMailOAuthCredentialProvider() GUCEF_VIRTUAL_OVERRIDE;

    /** The manager that has the provider and the session of the account */
    void SetManager( CAuthManager* manager );

    /** Where the refresh token of the account is (the store of the manager) and where a rotated one is put */
    void SetCredentialStore( ICredentialStore* store );

    /** The account the mail belongs to: the login mechanism ("google"), the account that logged in, and the user name the mail servers expect (the mail address). Forgets the cached token */
    void SetAccount( const CORE::CString& providerName, const CAccountRef& account, const CORE::CString& mailUserName );

    /** Forgets the token and the failure: the account signed in again, or another one is meant */
    void Reset( void );

    /** Whether an account is set that can get a token: its session is known to the manager or its refresh token is in the store */
    bool HasLogin( void ) const;

    void SetSkewMs( UInt32 skewMs );
    void SetRetryDelayMs( UInt32 delayMs );

    virtual EResult GetCredential( const COM::CMailCredentialRequest& request, COM::CMailCredential& credential ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReportRejected( const COM::CMailCredentialRequest& request ) GUCEF_VIRTUAL_OVERRIDE;

    /** How many renewals were started, how many a server refused the token for, and whether the last renewal failed */
    UInt32 GetRefreshCount( void ) const;
    UInt32 GetRejectionCount( void ) const;
    bool HasRefreshFailed( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CPollObserver;
    friend class CPollObserver;

    CMailOAuthCredentialProvider( const CMailOAuthCredentialProvider& src );
    CMailOAuthCredentialProvider& operator=( const CMailOAuthCredentialProvider& src );

    bool HasUsableCachedToken( UInt64 nowMs ) const;
    bool TakeSessionToken( UInt64 nowMs );
    bool StartRefresh( void );
    void FinishRefresh( void );
    void FillCredential( COM::CMailCredential& credential ) const;

    CAuthManager* m_manager;
    ICredentialStore* m_store;
    CORE::CString m_providerName;
    CAccountRef m_account;
    CORE::CString m_mailUserName;
    CSecretString m_accessToken;
    UInt64 m_expiresAtMs;
    CORE::CFutureResult m_refreshFuture;
    bool m_refreshing;
    bool m_refreshFailed;
    UInt64 m_failedAtMs;
    bool m_sessionSuperseded;           /**< a renewal happened: the token of the session of the manager is older than the one in hand */
    bool m_justRenewed;                 /**< the token in hand came from a renewal that no request has taken yet: it is handed out whatever its remaining life */
    GUCEF::vector< UInt32 > m_waitingRequests;
    CORE::CTimer m_pollTimer;
    CPollObserver* m_poller;
    UInt32 m_skewMs;
    UInt32 m_retryDelayMs;
    UInt32 m_refreshCount;
    UInt32 m_rejectionCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CMAILOAUTHCREDENTIALPROVIDER_H ? */
