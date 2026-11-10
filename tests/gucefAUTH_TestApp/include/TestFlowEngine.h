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

#ifndef GUCEF_AUTH_TESTAPP_TESTFLOWENGINE_H
#define GUCEF_AUTH_TESTAPP_TESTFLOWENGINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** An engine of the test: ends every login at once with a result for a fixed account, and counts what it was asked */
class CTestFlowEngine : public GUCEF::AUTH::IAuthFlowEngine
{
    public:

    bool supported;
    GUCEF::AUTH::TAuthFailureReason failWith;
    bool fail;
    GUCEF::UInt32 interactiveCount;
    GUCEF::UInt32 refreshCount;
    GUCEF::UInt32 revokeCount;
    GUCEF::UInt32 verifyCount;
    GUCEF::CORE::CString lastScopeSeen;
    GUCEF::CORE::CString lastClientIdSeen;
    GUCEF::CORE::CString lastRefreshToken;

    CTestFlowEngine( void )
        : GUCEF::AUTH::IAuthFlowEngine(), supported( true ), failWith( GUCEF::AUTH::AUTHFAIL_DENIED ), fail( false )
        , interactiveCount( 0 ), refreshCount( 0 ), revokeCount( 0 ), verifyCount( 0 ), lastScopeSeen(), lastClientIdSeen(), lastRefreshToken() {}

    virtual bool IsSupported( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor, const GUCEF::AUTH::CAuthProviderSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return supported;
    }

    virtual bool StartInteractive( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor ,
                                   const GUCEF::AUTH::CAuthProviderSettings& settings     ,
                                   const GUCEF::AUTH::CAuthOptions& options               ,
                                   GUCEF::AUTH::IAuthFlowSink& sink                       ,
                                   GUCEF::CORE::CFutureResultSource& result               ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++interactiveCount;
        lastClientIdSeen = settings.clientId;
        lastScopeSeen = options.scopes.empty() ? GUCEF::CORE::CString::Empty : options.scopes[ 0 ];
        GUCEF::AUTH::CAuthActionRequired action;
        action.kind = GUCEF::AUTH::AUTHACTION_SHOW_DEVICE_CODE;
        action.url = "https://example.invalid/device";
        action.userCode = "ABCD-1234";
        sink.OnActionRequired( action );
        if ( fail )
        {
            sink.OnLoginFailed( result, failWith, "the engine says no" );
            return true;
        }
        GUCEF::AUTH::CAuthResult authResult;
        GUCEF::CORE::CString::StringMap claims;
        claims[ "sub" ] = "112233";
        claims[ "tid" ] = "tenant-1";
        claims[ "oid" ] = "object-9";
        GUCEF::CORE::CString::StringMap parameters;
        descriptor.MakeAccountRef( claims, parameters, authResult.accountRef );
        authResult.displayName = "Engine Player";
        authResult.refreshToken.Set( "engine-refresh-token" );
        sink.OnLoginCompleted( result, authResult );
        return true;
    }

    virtual bool StartRefresh( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor ,
                               const GUCEF::AUTH::CAuthProviderSettings& settings     ,
                               const GUCEF::AUTH::CAccountRef& account                ,
                               const GUCEF::AUTH::CSecretString& refreshToken         ,
                               GUCEF::AUTH::IAuthFlowSink& sink                       ,
                               GUCEF::CORE::CFutureResultSource& result               ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++refreshCount;
        lastRefreshToken = refreshToken.Reveal();
        GUCEF::AUTH::CAuthResult authResult;
        authResult.accountRef = account;
        sink.OnLoginCompleted( result, authResult );
        return true;
    }

    virtual bool Revoke( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor ,
                         const GUCEF::AUTH::CAuthProviderSettings& settings     ,
                         const GUCEF::AUTH::CAccountRef& account                ,
                         const GUCEF::AUTH::CSecretString& refreshToken         ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++revokeCount;
        return true;
    }

    virtual bool Verify( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor ,
                         const GUCEF::AUTH::CAuthProviderSettings& settings     ,
                         const GUCEF::AUTH::CAuthProof& proof                   ,
                         GUCEF::AUTH::CAuthResult& result                       ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++verifyCount;
        result.accountRef = GUCEF::AUTH::CAccountRef( descriptor.accountProviderId, "verified-subject" );
        return true;
    }
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTFLOWENGINE_H ? */
