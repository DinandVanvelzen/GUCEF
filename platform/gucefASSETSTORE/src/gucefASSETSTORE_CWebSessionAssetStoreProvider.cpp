/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H
#include "gucefASSETSTORE_CAssetStoreTransfer.h"
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H
#include "gucefASSETSTORE_CAssetStoreSignInContext.h"
#define GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H ? */

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CWebSessionAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONASSETSTOREPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWebSessionAssetStoreProvider::CSignInObserver::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( eventId == AUTH::CWebSessionSignIn::SignedInEvent || eventId == AUTH::CWebSessionSignIn::SessionEndedEvent )
    {
        m_provider.RefreshCookieJar();
        m_provider.NotifySessionStateChanged();
    }
}

/*-------------------------------------------------------------------------*/

CWebSessionAssetStoreProvider::CWebSessionAssetStoreProvider( void )
    : CIAssetStoreProvider()
    , m_signIn()
    , m_cookieJar( GUCEF_NEW WEB::CHttpCookieJar() )
    , m_observer( *this )
{GUCEF_TRACE;

    m_observer.SubscribeTo( &m_signIn, AUTH::CWebSessionSignIn::SignedInEvent );
    m_observer.SubscribeTo( &m_signIn, AUTH::CWebSessionSignIn::SessionEndedEvent );
}

/*-------------------------------------------------------------------------*/

CWebSessionAssetStoreProvider::~CWebSessionAssetStoreProvider()
{GUCEF_TRACE;

    m_signIn.Cancel();
    m_signIn.WaitForAsyncSignIn( 60000 );
    m_observer.UnsubscribeFrom( m_signIn );
    WaitForAsyncOperations();
    SignalUpcomingDestruction();
    if ( !m_cookieJar.IsNULL() )
        m_cookieJar->Clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWebSessionAssetStoreProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CWebSessionAssetStoreProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionAssetStoreProvider::ConfigureWebSignIn( const AUTH::CWebSessionSignInSettings& settings   ,
                                                   AUTH::ICredentialStore* sessionStore              ,
                                                   AUTH::ICredentialStore* credentialStore           ,
                                                   CORE::CString::StringVector* problems             )
{GUCEF_TRACE;

    AUTH::CWebSessionSignInSettings ownSettings = settings;
    if ( ownSettings.storeId.IsNULLOrEmpty() )
        ownSettings.storeId = GetProviderId();
    if ( !m_signIn.SetSettings( ownSettings, problems ) )
        return false;
    CAssetStoreGlobal* global = CAssetStoreGlobal::Instance();
    m_signIn.SetSessionStore( GUCEF_NULL != sessionStore || GUCEF_NULL == global ? sessionStore : global->GetWebSessionStore() );
    m_signIn.SetCredentialStore( GUCEF_NULL != credentialStore || GUCEF_NULL == global ? credentialStore : global->GetWebCredentialStore() );
    if ( m_signIn.LoadSavedSession() )
    {
        RefreshCookieJar();
        NotifySessionStateChanged();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionAssetStoreProvider::ConfigureSignIn( const CAssetStoreSignInContext& context, CORE::CString& problem )
{GUCEF_TRACE;

    if ( GUCEF_NULL != context.sessionStore )
        m_signIn.SetSessionStore( context.sessionStore );
    if ( GUCEF_NULL != context.credentialStore )
        m_signIn.SetCredentialStore( context.credentialStore );
    if ( GUCEF_NULL != context.sessionStore && m_signIn.LoadSavedSession() )
    {
        RefreshCookieJar();
        NotifySessionStateChanged();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

AUTH::CWebSessionSignIn&
CWebSessionAssetStoreProvider::GetSignIn( void )
{GUCEF_TRACE;

    return m_signIn;
}

/*-------------------------------------------------------------------------*/

TAssetStoreSignInKind
CWebSessionAssetStoreProvider::GetSignInKind( void ) const
{GUCEF_TRACE;

    return ASSETSTORE_SIGNIN_WEB_SESSION;
}

/*-------------------------------------------------------------------------*/

AUTH::CWebSessionSignIn*
CWebSessionAssetStoreProvider::GetWebSessionSignIn( void )
{GUCEF_TRACE;

    return &m_signIn;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionAssetStoreProvider::IsSignedIn( void ) const
{GUCEF_TRACE;

    return m_signIn.HasSession();
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionAssetStoreProvider::SignOut( void )
{GUCEF_TRACE;

    const bool success = m_signIn.ForgetSession();
    m_cookieJar->Clear();
    NotifySessionStateChanged();
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionAssetStoreProvider::RenewSession( void )
{GUCEF_TRACE;

    const bool renewed = WEB::WEBSIGNIN_STATUS_OK == m_signIn.RenewSession();

    // the caller retries right after this, before an event could reach the jar
    RefreshCookieJar();
    NotifySessionStateChanged();
    return renewed;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionAssetStoreProvider::RefreshCookieJar( void )
{GUCEF_TRACE;

    m_cookieJar->Clear();
    m_signIn.ApplySessionTo( *m_cookieJar );
}

/*-------------------------------------------------------------------------*/

WEB::CHttpCookieJarPtr
CWebSessionAssetStoreProvider::GetCookieJar( void ) const
{GUCEF_TRACE;

    return m_cookieJar;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionAssetStoreProvider::AddSessionCookies( WEB::CHttpRequestData& request ) const
{GUCEF_TRACE;

    const CORE::CString host = request.GetHostName().Lowercase();
    const CORE::CString path = request.requestUri.IsNULLOrEmpty() ? CORE::CString( "/" ) : request.requestUri;
    const CORE::CString cookieHeader = m_cookieJar->GetCookieHeaderValue( host, path, request.requestProtocol.Lowercase() == "https" );
    if ( cookieHeader.IsNULLOrEmpty() )
        return false;
    request.otherHeaders[ "Cookie" ] = cookieHeader;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionAssetStoreProvider::AuthorizeTransferRequest( const CAssetStoreTransfer& transfer, WEB::CHttpRequestData& request )
{GUCEF_TRACE;

    AddSessionCookies( request );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionAssetStoreProvider::CheckResponse( const WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    if ( !m_signIn.CheckResponse( response ) )
    {
        // the store may renew cookies of the session in its answers: they are kept with the session
        m_signIn.UpdateSessionCookies( *m_cookieJar );
        return false;
    }
    m_cookieJar->Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionAssetStoreProvider::EndSession( const CORE::CString& reason )
{GUCEF_TRACE;

    m_signIn.EndSession( reason );
    m_cookieJar->Clear();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CWebSessionAssetStoreProvider::ImportWebSession( const AUTH::CWebSession& session, CORE::CString& message )
{GUCEF_TRACE;

    UInt32 kept = 0;
    CORE::CString problem;
    if ( !m_signIn.AdoptSession( session, kept, problem ) )
    {
        message = problem;
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    // the caller goes on with requests of the session right after this, before an event could reach the jar
    RefreshCookieJar();
    NotifySessionStateChanged();
    message = CORE::ToString( kept ) + " cookies of the session were taken";
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebSessionAssetStoreProvider::GetSessionUserAgent( void ) const
{GUCEF_TRACE;

    AUTH::CWebSession session;
    if ( !m_signIn.GetSession( session ) )
        return CORE::CString::Empty;
    const CORE::CString userAgent = session.userAgent;
    session.Clear();
    return userAgent;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
