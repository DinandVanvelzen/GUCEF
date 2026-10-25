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

#ifndef GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEENGINE_H
#define GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEENGINE_H

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

/**
 *  An OIDC engine of the test: it does not talk to anyone. It verifies a proof when its data is the token it was told to
 *  accept and then gives the account it was told to give, and it records what it was asked, so that a test can see what a
 *  provider handed to the engine of gucefAUTH.
 */
class CFakeOidcEngine : public GUCEF::AUTH::IAuthFlowEngine
{
    public:

    bool supported;
    GUCEF::CORE::CString acceptedToken;
    GUCEF::AUTH::CAccountRef accountToGive;
    GUCEF::UInt32 verifyCount;
    GUCEF::UInt32 supportedCount;
    GUCEF::CORE::CString lastDescriptorName;
    GUCEF::CORE::CString lastClientId;
    GUCEF::CORE::CString lastClientSecret;
    GUCEF::CORE::CString lastProofKind;
    GUCEF::CORE::CString lastProofData;
    bool acceptRedirects;
    GUCEF::CORE::CString lastRedirectUrl;
    GUCEF::UInt32 redirectCount;

    CFakeOidcEngine( void )
        : GUCEF::AUTH::IAuthFlowEngine()
        , supported( true ), acceptedToken(), accountToGive(), verifyCount( 0 ), supportedCount( 0 ), lastDescriptorName(), lastClientId()
        , lastClientSecret(), lastProofKind(), lastProofData(), acceptRedirects( true ), lastRedirectUrl(), redirectCount( 0 )
    {}

    virtual bool IsSupported( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor ,
                              const GUCEF::AUTH::CAuthProviderSettings& settings     ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const_cast< CFakeOidcEngine* >( this )->supportedCount++;
        return supported;
    }

    virtual bool StartInteractive( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor ,
                                   const GUCEF::AUTH::CAuthProviderSettings& settings     ,
                                   const GUCEF::AUTH::CAuthOptions& options               ,
                                   GUCEF::AUTH::IAuthFlowSink& sink                       ,
                                   GUCEF::CORE::CFutureResultSource& result               ) GUCEF_VIRTUAL_OVERRIDE
    {
        return false;
    }

    virtual bool StartRefresh( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor ,
                               const GUCEF::AUTH::CAuthProviderSettings& settings     ,
                               const GUCEF::AUTH::CAccountRef& account                ,
                               const GUCEF::AUTH::CSecretString& refreshToken         ,
                               GUCEF::AUTH::IAuthFlowSink& sink                       ,
                               GUCEF::CORE::CFutureResultSource& result               ) GUCEF_VIRTUAL_OVERRIDE
    {
        return false;
    }

    virtual bool Revoke( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor ,
                         const GUCEF::AUTH::CAuthProviderSettings& settings     ,
                         const GUCEF::AUTH::CAccountRef& account                ,
                         const GUCEF::AUTH::CSecretString& refreshToken         ) GUCEF_VIRTUAL_OVERRIDE
    {
        return false;
    }

    virtual bool Verify( const GUCEF::AUTH::CAuthProviderDescriptor& descriptor ,
                         const GUCEF::AUTH::CAuthProviderSettings& settings     ,
                         const GUCEF::AUTH::CAuthProof& proof                   ,
                         GUCEF::AUTH::CAuthResult& result                       ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++verifyCount;
        lastDescriptorName = descriptor.name;
        lastClientId = settings.clientId;
        lastClientSecret = settings.clientSecret.Reveal();
        lastProofKind = proof.kind;
        lastProofData = proof.data.Reveal();
        if ( proof.kind != GUCEF::AUTH::CAuthProof::KindOidcIdToken || proof.data.Reveal() != acceptedToken )
            return false;
        result = GUCEF::AUTH::CAuthResult();
        result.accountRef = accountToGive;
        return true;
    }

    virtual bool SubmitRedirectUrl( const GUCEF::CORE::CString& redirectUrl ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++redirectCount;
        lastRedirectUrl = redirectUrl;
        return acceptRedirects;
    }
};

/*-------------------------------------------------------------------------*/

/** Makes a fake engine the engine of CAuthGlobal for the time it exists and puts the one that was there back */
class CScopedEngine
{
    public:

    explicit CScopedEngine( CFakeOidcEngine* engine )
        : m_previous( GUCEF::AUTH::CAuthGlobal::Instance()->GetFlowEngine() )
    {
        GUCEF::AUTH::CAuthGlobal::Instance()->SetFlowEngine( engine );
    }

    ~CScopedEngine()
    {
        GUCEF::AUTH::CAuthGlobal::Instance()->SetFlowEngine( m_previous );
    }

    private:

    CScopedEngine( const CScopedEngine& src );
    CScopedEngine& operator=( const CScopedEngine& src );

    GUCEF::AUTH::IAuthFlowEngine* m_previous;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEENGINE_H ? */
