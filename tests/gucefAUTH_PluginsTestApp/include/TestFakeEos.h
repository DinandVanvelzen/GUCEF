/*
 *  gucefAUTH_PluginsTestApp: Application for testing the gucefAUTH plugins
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

#ifndef GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEEOS_H
#define GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEEOS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H
#include "authpluginEOS_EosShim.h"
#define GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H
#endif /* GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An Epic Online Services SDK of the test, behind the function table of the provider. The answers of the SDK come as
 *  events while the provider ticks it, after the number of ticks the test sets, like callbacks do in the SDK.
 */
class CFakeEos
{
    public:

    bool sdkAvailable;
    bool platformOk;
    GUCEF::CORE::CString platformError;
    bool platformReady;

    // what the SDK answers
    GUCEF::AUTHPLUGIN::EOS::EEosOutcome authOutcome;
    GUCEF::Int32 authSdkCode;
    GUCEF::CORE::CString epicAccountId;
    bool pinGrantFirst;
    bool authNeverAnswers;
    GUCEF::UInt32 ticksUntilAnswer;
    bool idTokenAvailable;
    GUCEF::CORE::CString idToken;
    bool connectStarts;
    GUCEF::AUTHPLUGIN::EOS::EEosOutcome connectOutcome;
    GUCEF::CORE::CString productUserId;
    bool createUserStarts;
    GUCEF::AUTHPLUGIN::EOS::EEosOutcome createOutcome;
    GUCEF::CORE::CString createdProductUserId;
    bool loginStarts;
    bool linkStarts;
    GUCEF::AUTHPLUGIN::EOS::EEosOutcome linkOutcome;

    // what was asked
    GUCEF::UInt32 createPlatformCalls;
    GUCEF::UInt32 releaseCalls;
    GUCEF::UInt32 tickCalls;
    GUCEF::CORE::CString configProductId;
    GUCEF::CORE::CString configSandboxId;
    GUCEF::CORE::CString configDeploymentId;
    GUCEF::CORE::CString configClientId;
    GUCEF::CORE::CString configClientSecret;
    GUCEF::CORE::CString configEncryptionKey;
    GUCEF::CORE::CString configProductName;
    GUCEF::vector< GUCEF::AUTHPLUGIN::EOS::EEosLoginType > loginTypes;
    GUCEF::vector< GUCEF::CORE::CString > connectTokens;
    GUCEF::UInt32 createUserCalls;
    GUCEF::vector< GUCEF::CORE::CString > linkTargets;
    GUCEF::vector< GUCEF::CORE::CString > logoutAccounts;
    GUCEF::vector< bool > logoutDeletes;

    CFakeEos( void )
        : sdkAvailable( true ), platformOk( true ), platformError(), platformReady( false )
        , authOutcome( GUCEF::AUTHPLUGIN::EOS::EOSOUTCOME_SUCCESS ), authSdkCode( 0 ), epicAccountId( "0123456789abcdef0123456789abcdef" )
        , pinGrantFirst( true ), authNeverAnswers( false ), ticksUntilAnswer( 1 ), idTokenAvailable( true )
        , idToken( "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJ4In0.EOS-ID-TOKEN-SIGNATURE-DO-NOT-LOG" ), connectStarts( true )
        , connectOutcome( GUCEF::AUTHPLUGIN::EOS::EOSOUTCOME_SUCCESS ), productUserId( "00aa11bb22cc33dd44ee55ff66778899" ), createUserStarts( true )
        , createOutcome( GUCEF::AUTHPLUGIN::EOS::EOSOUTCOME_SUCCESS ), createdProductUserId( "99887766ffee55dd44cc33bb22aa1100" ), loginStarts( true )
        , linkStarts( true ), linkOutcome( GUCEF::AUTHPLUGIN::EOS::EOSOUTCOME_SUCCESS )
        , createPlatformCalls( 0 ), releaseCalls( 0 ), tickCalls( 0 ), configProductId(), configSandboxId(), configDeploymentId(), configClientId()
        , configClientSecret(), configEncryptionKey(), configProductName(), loginTypes(), connectTokens(), createUserCalls( 0 ), linkTargets(), logoutAccounts(), logoutDeletes()
        , m_pending(), m_ready()
    {}

    GUCEF::AUTHPLUGIN::EOS::TEosShim MakeShim( void )
    {
        using namespace GUCEF::AUTHPLUGIN::EOS;
        TEosShim shim;
        shim.context = this;
        shim.isSdkAvailable = &CFakeEos::IsSdkAvailable;
        shim.createPlatform = &CFakeEos::CreatePlatform;
        shim.releasePlatform = &CFakeEos::ReleasePlatform;
        shim.isPlatformReady = &CFakeEos::IsPlatformReady;
        shim.tick = &CFakeEos::Tick;
        shim.pollEvent = &CFakeEos::PollEvent;
        shim.authLogin = &CFakeEos::AuthLogin;
        shim.authCopyIdToken = &CFakeEos::AuthCopyIdToken;
        shim.connectLogin = &CFakeEos::ConnectLogin;
        shim.connectCreateUser = &CFakeEos::ConnectCreateUser;
        shim.connectLinkAccount = &CFakeEos::ConnectLinkAccount;
        shim.authLogout = &CFakeEos::AuthLogout;
        return shim;
    }

    private:

    struct SPending
    {
        GUCEF::UInt32 requestId;
        GUCEF::AUTHPLUGIN::EOS::EEosEventKind kind;
        GUCEF::AUTHPLUGIN::EOS::EEosLoginType loginType;
        GUCEF::UInt32 ticksLeft;
    };

    static CFakeEos* Self( void* context ) { return static_cast< CFakeEos* >( context ); }

    static bool IsSdkAvailable( void* context ) { return Self( context )->sdkAvailable; }

    static bool CreatePlatform( void* context, const GUCEF::AUTHPLUGIN::EOS::SEosPlatformConfig* config, char* errorText, GUCEF::UInt32 errorTextSize )
    {
        CFakeEos* self = Self( context );
        ++self->createPlatformCalls;
        if ( !self->platformOk )
        {
            strncpy( errorText, self->platformError.IsNULLOrEmpty() ? "fake: platform failed" : self->platformError.C_String(), errorTextSize - 1 );
            errorText[ errorTextSize - 1 ] = '\0';
            return false;
        }
        self->configProductId = config->productId;
        self->configSandboxId = config->sandboxId;
        self->configDeploymentId = config->deploymentId;
        self->configClientId = config->clientId;
        self->configClientSecret = config->clientSecret;
        self->configEncryptionKey = config->encryptionKey;
        self->configProductName = config->productName;
        self->platformReady = true;
        return true;
    }

    static void ReleasePlatform( void* context ) { CFakeEos* self = Self( context ); ++self->releaseCalls; self->platformReady = false; }
    static bool IsPlatformReady( void* context ) { return Self( context )->platformReady; }

    static void Tick( void* context )
    {
        CFakeEos* self = Self( context );
        ++self->tickCalls;
        for ( size_t i=0; i<self->m_pending.size(); )
        {
            if ( self->m_pending[ i ].ticksLeft > 0 )
                --self->m_pending[ i ].ticksLeft;
            if ( self->m_pending[ i ].ticksLeft > 0 )
            {
                ++i;
                continue;
            }
            self->Answer( self->m_pending[ i ] );
            self->m_pending.erase( self->m_pending.begin() + i );
        }
    }

    void Answer( const SPending& pending )
    {
        using namespace GUCEF::AUTHPLUGIN::EOS;
        SEosEvent event;
        event.requestId = pending.requestId;
        event.kind = pending.kind;
        if ( EOSEVENT_AUTH_LOGIN == pending.kind )
        {
            if ( pinGrantFirst && EOSLOGIN_DEVICE_CODE == pending.loginType )
            {
                SEosEvent pin;
                pin.requestId = pending.requestId;
                pin.kind = EOSEVENT_PIN_GRANT;
                pin.outcome = EOSOUTCOME_SUCCESS;
                strcpy( pin.userCode, "ABCD-1234" );
                strcpy( pin.verificationUri, "https://epicgames.example.invalid/activate" );
                strcpy( pin.verificationUriComplete, "https://epicgames.example.invalid/activate?userCode=ABCD-1234" );
                pin.expiresInSeconds = 600;
                m_ready.push_back( pin );
            }
            event.outcome = authOutcome;
            event.sdkResultCode = authSdkCode;
            if ( EOSOUTCOME_SUCCESS == authOutcome )
                strncpy( event.epicAccountId, epicAccountId.C_String(), sizeof( event.epicAccountId ) - 1 );
        }
        else
        if ( EOSEVENT_CONNECT_LOGIN == pending.kind )
        {
            event.outcome = connectOutcome;
            event.sdkResultCode = EOSOUTCOME_SUCCESS == connectOutcome ? 0 : 3;
            if ( EOSOUTCOME_SUCCESS == connectOutcome )
                strncpy( event.productUserId, productUserId.C_String(), sizeof( event.productUserId ) - 1 );
        }
        else
        if ( EOSEVENT_CONNECT_CREATE_USER == pending.kind )
        {
            event.outcome = createOutcome;
            event.sdkResultCode = EOSOUTCOME_SUCCESS == createOutcome ? 0 : 35;
            if ( EOSOUTCOME_SUCCESS == createOutcome )
                strncpy( event.productUserId, createdProductUserId.C_String(), sizeof( event.productUserId ) - 1 );
        }
        else
        if ( EOSEVENT_CONNECT_LINK_ACCOUNT == pending.kind )
        {
            event.outcome = linkOutcome;
            event.sdkResultCode = EOSOUTCOME_SUCCESS == linkOutcome ? 0 : 35;
            if ( EOSOUTCOME_SUCCESS == linkOutcome )
                strncpy( event.productUserId, linkTargets.back().C_String(), sizeof( event.productUserId ) - 1 );
        }
        m_ready.push_back( event );
    }

    static bool PollEvent( void* context, GUCEF::AUTHPLUGIN::EOS::SEosEvent* event )
    {
        CFakeEos* self = Self( context );
        if ( self->m_ready.empty() )
            return false;
        *event = self->m_ready.front();
        self->m_ready.erase( self->m_ready.begin() );
        return true;
    }

    void Schedule( GUCEF::UInt32 requestId, GUCEF::AUTHPLUGIN::EOS::EEosEventKind kind, GUCEF::AUTHPLUGIN::EOS::EEosLoginType loginType )
    {
        SPending pending;
        pending.requestId = requestId;
        pending.kind = kind;
        pending.loginType = loginType;
        pending.ticksLeft = ticksUntilAnswer;
        m_pending.push_back( pending );
    }

    static bool AuthLogin( void* context, GUCEF::UInt32 requestId, GUCEF::AUTHPLUGIN::EOS::EEosLoginType type )
    {
        CFakeEos* self = Self( context );
        self->loginTypes.push_back( type );
        if ( !self->loginStarts )
            return false;
        if ( !self->authNeverAnswers )
            self->Schedule( requestId, GUCEF::AUTHPLUGIN::EOS::EOSEVENT_AUTH_LOGIN, type );
        return true;
    }

    static bool AuthCopyIdToken( void* context, const char* account, char* buffer, GUCEF::UInt32 bufferSize )
    {
        CFakeEos* self = Self( context );
        if ( !self->idTokenAvailable || self->idToken.ByteSize() > bufferSize )
            return false;
        strcpy( buffer, self->idToken.C_String() );
        return true;
    }

    static bool ConnectLogin( void* context, GUCEF::UInt32 requestId, const char* token )
    {
        CFakeEos* self = Self( context );
        self->connectTokens.push_back( token );
        if ( !self->connectStarts )
            return false;
        self->Schedule( requestId, GUCEF::AUTHPLUGIN::EOS::EOSEVENT_CONNECT_LOGIN, GUCEF::AUTHPLUGIN::EOS::EOSLOGIN_PERSISTENT );
        return true;
    }

    static bool ConnectCreateUser( void* context, GUCEF::UInt32 requestId )
    {
        CFakeEos* self = Self( context );
        ++self->createUserCalls;
        if ( !self->createUserStarts )
            return false;
        self->Schedule( requestId, GUCEF::AUTHPLUGIN::EOS::EOSEVENT_CONNECT_CREATE_USER, GUCEF::AUTHPLUGIN::EOS::EOSLOGIN_PERSISTENT );
        return true;
    }

    static bool ConnectLinkAccount( void* context, GUCEF::UInt32 requestId, const char* productUserId )
    {
        CFakeEos* self = Self( context );
        self->linkTargets.push_back( productUserId );
        if ( !self->linkStarts )
            return false;
        self->Schedule( requestId, GUCEF::AUTHPLUGIN::EOS::EOSEVENT_CONNECT_LINK_ACCOUNT, GUCEF::AUTHPLUGIN::EOS::EOSLOGIN_PERSISTENT );
        return true;
    }

    static bool AuthLogout( void* context, GUCEF::UInt32 requestId, const char* account, bool deletePersistentAuth )
    {
        CFakeEos* self = Self( context );
        self->logoutAccounts.push_back( account );
        self->logoutDeletes.push_back( deletePersistentAuth );
        return true;
    }

    private:

    GUCEF::vector< SPending > m_pending;
    GUCEF::vector< GUCEF::AUTHPLUGIN::EOS::SEosEvent > m_ready;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEEOS_H ? */
