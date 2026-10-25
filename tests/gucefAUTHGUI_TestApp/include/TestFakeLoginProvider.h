/*
 *  gucefAUTHGUI_TestApp: Application for testing gucefAUTHGUI
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

#ifndef GUCEF_AUTHGUI_TESTAPP_TESTFAKELOGINPROVIDER_H
#define GUCEF_AUTHGUI_TESTAPP_TESTFAKELOGINPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A login provider the test steers: what happens when a login starts is the mode, and when it ends is up to the test
 *  (Succeed(), FailWith(), HonourCancel()). No network, no browser. The tokens it gives are recognisable, to check
 *  that none of them ever shows in the user interface.
 */
class CFakeLoginProvider : public GUCEF::AUTH::CIAuthProvider
{
    public:

    enum EMode
    {
        MODE_INSTANT        ,   /**< the login ends well at once */
        MODE_BROWSER        ,   /**< asks the application to open a url and waits */
        MODE_DEVICE_CODE    ,   /**< asks the application to show a code and waits */
        MODE_FAIL           ,   /**< ends with a failure at once */
        MODE_REFUSE             /**< cannot start */
    };

    static const GUCEF::CORE::CString SecretAccessToken;
    static const GUCEF::CORE::CString SecretRefreshToken;
    static const GUCEF::CORE::CString SecretProof;

    GUCEF::CORE::CString name;
    GUCEF::CORE::CString accountNamespace;
    GUCEF::CORE::CString subject;
    GUCEF::CORE::CString displayName;
    GUCEF::CORE::CString loginHint;         /**< the name of the account at the provider that a login gives, for the next login to prefill */
    GUCEF::UInt32 capabilities;
    EMode mode;
    GUCEF::UInt32 beginCount;
    GUCEF::AUTH::CAuthOptions lastOptions;

    explicit CFakeLoginProvider( const GUCEF::CORE::CString& providerName = "fake" )
        : GUCEF::AUTH::CIAuthProvider()
        , name( providerName )
        , accountNamespace( "fake" )
        , subject( "player-1" )
        , displayName( "Fake Player" )
        , loginHint()
        , capabilities( GUCEF::AUTH::AUTHCAP_BROWSER | GUCEF::AUTH::AUTHCAP_DEVICE_CODE )
        , mode( MODE_INSTANT )
        , beginCount( 0 )
        , lastOptions()
        , m_source()
        , m_pending( false )
    {
    }

    virtual ~CFakeLoginProvider() {}

    virtual const GUCEF::CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE { return name; }

    virtual GUCEF::UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return capabilities; }

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool TrySilentLogin( GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        future = GUCEF::CORE::CFutureResult( GUCEF::CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        return false;
    }

    virtual bool BeginLogin( const GUCEF::AUTH::CAuthOptions& options, GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++beginCount;
        lastOptions = options;
        if ( MODE_REFUSE == mode )
        {
            future = GUCEF::CORE::CFutureResult( GUCEF::CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            return false;
        }
        m_source = GUCEF::CORE::CFutureResultSource();
        future = m_source.GetFuture();
        m_pending = true;
        if ( MODE_BROWSER == mode )
        {
            GUCEF::AUTH::CAuthActionRequired action;
            action.kind = GUCEF::AUTH::AUTHACTION_OPEN_URL;
            action.url = "https://accounts.example.com/authorize?state=abc";
            action.expiresAtMs = GUCEF::AUTH::GetEpochTimeInMs() + 600000;
            RaiseActionRequired( action );
        }
        else
        if ( MODE_DEVICE_CODE == mode )
        {
            GUCEF::AUTH::CAuthActionRequired action;
            action.kind = GUCEF::AUTH::AUTHACTION_SHOW_DEVICE_CODE;
            action.url = "https://example.com/activate";
            action.urlWithCode = "https://example.com/activate?user_code=WDJB-MJHT";
            action.userCode = "WDJB-MJHT";
            action.expiresAtMs = GUCEF::AUTH::GetEpochTimeInMs() + 900000;
            RaiseActionRequired( action );
        }
        else
        if ( MODE_INSTANT == mode )
        {
            Succeed();
        }
        else
        if ( MODE_FAIL == mode )
        {
            FailWith( GUCEF::AUTH::AUTHFAIL_DENIED, "access denied" );
        }
        return true;
    }

    /** Ends the login that waits with a good result */
    bool Succeed( void )
    {
        if ( !m_pending )
            return false;
        m_pending = false;
        GUCEF::AUTH::CAuthResult result;
        result.accountRef = GUCEF::AUTH::CAccountRef( accountNamespace, subject );
        result.displayName = displayName;
        result.loginHint = loginHint;
        result.accessToken.Set( SecretAccessToken );
        result.refreshToken.Set( SecretRefreshToken );
        result.expiresAtMs = GUCEF::AUTH::GetEpochTimeInMs() + 3600000;
        result.proof = GUCEF::AUTH::CAuthProof( GUCEF::AUTH::CAuthProof::KindOidcIdToken, SecretProof );
        return CompleteLogin( m_source, result );
    }

    bool FailWith( GUCEF::AUTH::TAuthFailureReason reason, const GUCEF::CORE::CString& message )
    {
        if ( !m_pending )
            return false;
        m_pending = false;
        return FailLogin( m_source, reason, message );
    }

    /** What a provider does when it is asked to stop: ends the login that waits as cancelled */
    bool HonourCancel( void )
    {
        if ( !m_pending || !m_source.IsCancellationRequested() )
            return false;
        m_pending = false;
        return FailLogin( m_source, GUCEF::AUTH::AUTHFAIL_CANCELLED, "cancelled" );
    }

    bool IsPending( void ) const { return m_pending; }

    virtual bool SaveConfig( GUCEF::CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CFakeLoginProvider";
        return classTypeName;
    }

    private:

    GUCEF::CORE::CFutureResultSource m_source;
    bool m_pending;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::CORE::CTFactory< GUCEF::AUTH::CIAuthProvider, CFakeLoginProvider, GUCEF::MT::CMutex > TFakeLoginProviderFactory;

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_TESTAPP_TESTFAKELOGINPROVIDER_H ? */
