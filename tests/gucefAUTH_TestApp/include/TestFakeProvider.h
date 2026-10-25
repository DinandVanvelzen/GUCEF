/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_AUTH_TESTAPP_TESTFAKEPROVIDER_H
#define GUCEF_AUTH_TESTAPP_TESTFAKEPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#include "gucefAUTH_CIAuthProvider.h"
#define GUCEF_AUTH_CIAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a fake provider does when it is asked to log in, to refresh or to log in silently */
enum EFakeBehaviour
{
    FAKE_COMPLETE = 0,          /**< ends well at once, on the calling thread */
    FAKE_FAIL,                  /**< fails at once with failReason */
    FAKE_SLOW,                  /**< ends well after delayMs, on a thread of the pool */
    FAKE_SLOW_FAIL,             /**< fails with failReason after delayMs, on a thread of the pool */
    FAKE_HANG,                  /**< never ends, except when the requester cancels */
    FAKE_IGNORE_CANCEL,         /**< never ends, a cancellation is not looked at */
    FAKE_NO_EVENT,              /**< ends the future well and raises no event */
    FAKE_REFUSE                 /**< cannot start: the operation returns false */
};

/*-------------------------------------------------------------------------*/

/**
 *  A provider that lives in the test: no network, no browser. What it does is set per operation, and it counts
 *  what it was asked. Provider "fake" gives accounts of the namespace "fake".
 */
class CFakeAuthProvider : public GUCEF::AUTH::CIAuthProvider
{
    public:

    static const GUCEF::CORE::CString SecretAccessToken;
    static const GUCEF::CORE::CString SecretIdToken;
    static const GUCEF::CORE::CString SecretRefreshTokenPrefix;
    static const GUCEF::CORE::CString SecretProof;

    GUCEF::CORE::CString name;
    GUCEF::CORE::CString accountNamespace;
    GUCEF::CORE::CString subject;                   /**< the account every login gives */
    GUCEF::CORE::CString displayName;
    GUCEF::CORE::CString resultLoginHint;           /**< the name of the account at the provider that a login gives, empty for none */
    GUCEF::UInt32 capabilities;
    bool available;
    bool announceAction;                            /**< a login raises ActionRequiredEvent before it goes on */
    bool giveRefreshToken;
    EFakeBehaviour loginBehaviour;
    EFakeBehaviour silentBehaviour;
    EFakeBehaviour refreshBehaviour;
    GUCEF::AUTH::TAuthFailureReason failReason;
    GUCEF::UInt32 delayMs;
    bool explainsUnavailability;                    /**< GetUnavailability() says why the provider is not available, when it is not */
    GUCEF::AUTH::CAuthUnavailableProvider unavailability;

    GUCEF::UInt32 beginCount;
    GUCEF::UInt32 silentCount;
    GUCEF::UInt32 refreshCount;
    GUCEF::UInt32 logoutCount;
    GUCEF::UInt32 verifyCount;
    GUCEF::CORE::CString lastRefreshTokenSeen;
    GUCEF::CORE::CString lastLogoutToken;
    GUCEF::AUTH::CAccountRef lastLogoutAccount;
    GUCEF::AUTH::CAuthOptions lastOptions;

    explicit CFakeAuthProvider( const GUCEF::CORE::CString& providerName = "fake" );

    virtual ~CFakeAuthProvider();

    virtual const GUCEF::CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool TrySilentLogin( GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool BeginLogin( const GUCEF::AUTH::CAuthOptions& options, GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Refresh( const GUCEF::AUTH::CAccountRef& account, const GUCEF::AUTH::CSecretString& refreshToken, GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Logout( const GUCEF::AUTH::CAccountRef& account, const GUCEF::AUTH::CSecretString& refreshToken ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Verify( const GUCEF::AUTH::CAuthProof& proof, GUCEF::AUTH::CAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetUnavailability( GUCEF::AUTH::CAuthUnavailableProvider& unavailable ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( GUCEF::CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** What a login of this provider gives */
    GUCEF::AUTH::CAuthResult MakeResult( void ) const;

    /** Ends the login the way CompleteLogin() does; for the pool task, which can not reach the protected member */
    bool Finish( GUCEF::CORE::CFutureResultSource& source, bool fail );

    bool Cancelled( GUCEF::CORE::CFutureResultSource& source );

    /** The config the manager gave the provider */
    GUCEF::CORE::CString configuredClientId;

    /** When set, every operation that is asked of the provider adds a line like "silent:name" */
    GUCEF::vector< GUCEF::CORE::CString >* callLog;

    private:

    bool Run( EFakeBehaviour behaviour, bool interactive, GUCEF::CORE::CFutureResult& future );

    private:

    GUCEF::vector< GUCEF::CORE::CFutureResultSource > m_taskSources;
};

/*-------------------------------------------------------------------------*/

/** A task for the thread pool that ends a login of a fake provider after a delay */
class CFakeLoginTaskConsumer : public GUCEF::CORE::CTaskConsumer
{
    public:

    static const GUCEF::CORE::CString TaskType;

    CFakeLoginTaskConsumer( void );
    virtual ~CFakeLoginTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( GUCEF::CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTaskEnded( GUCEF::CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** Makes the task type known to the task manager of the process; true also when it is known already */
bool
RegisterFakeLoginTask( void );

void
UnregisterFakeLoginTask( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTFAKEPROVIDER_H ? */
