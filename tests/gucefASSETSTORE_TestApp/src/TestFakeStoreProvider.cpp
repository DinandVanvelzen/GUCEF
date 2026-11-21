/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */
#ifndef GUCEF_AUTH_CWEBSESSION_H
#include "gucefAUTH_CWebSession.h"
#define GUCEF_AUTH_CWEBSESSION_H
#endif /* GUCEF_AUTH_CWEBSESSION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */
#include "TestFakeStoreProvider.h"

using namespace GUCEF;

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The server of the fake providers: test state, set by the tests before the providers are made */
CFakeStoreServer* g_fakeStoreServer = GUCEF_NULL;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CFakeStoreProvider::SetServer( CFakeStoreServer* server )
{
    g_fakeStoreServer = server;
}

/*-------------------------------------------------------------------------*/

CFakeStoreProvider::CFakeStoreProvider( void )
    : ASSETSTORE::CIAssetStoreProvider()
    , m_providerId( "fakestore" )
    , m_token( CORE::CString( "stale-token" ) )
    , m_planCount( 0 )
    , m_renewCount( 0 )
    , m_lock()
{
}

/*-------------------------------------------------------------------------*/

CFakeStoreProvider::~CFakeStoreProvider()
{
    WaitForAsyncOperations();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreProvider::SetProviderId( const CORE::CString& providerId )
{
    MT::CScopeMutex lock( m_lock );
    m_providerId = providerId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFakeStoreProvider::GetProviderId( void ) const
{
    return m_providerId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeStoreProvider::GetDisplayName( void ) const
{
    return "Fake store " + m_providerId;
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreProvider::IsSignedIn( void ) const
{
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeStoreProvider::GetPlanCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_planCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeStoreProvider::GetRenewCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_renewCount;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreProvider::SetToken( const CORE::CString& token )
{
    MT::CScopeMutex lock( m_lock );
    m_token.Set( token );
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreProvider::RaiseSessionChanged( void )
{
    NotifySessionStateChanged();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeStoreProvider::Get( const CORE::CString& path, CORE::CDataNode& json ) const
{
    if ( GUCEF_NULL == g_fakeStoreServer )
        return ASSETSTORE::ASSETSTORE_STATUS_NOT_SIGNED_IN;
    WEB::CHTTPClient client;
    WEB::CHttpRequestData request;
    request.SetUrl( g_fakeStoreServer->GetBaseUrl() + path );
    request.requestType = "GET";
    WEB::CHttpResponseData response;
    if ( !client.Execute( request, response ) )
        return ASSETSTORE::ASSETSTORE_STATUS_NETWORK_ERROR;
    if ( 404 == response.statusCode )
        return ASSETSTORE::ASSETSTORE_STATUS_NOT_FOUND;
    if ( 200 != response.statusCode )
        return ASSETSTORE::ASSETSTORE_STATUS_HTTP_ERROR;
    CORE::CString text;
    text.Set( response.content.AsConstTypePtr< char >(), response.content.GetDataSize() );
    return ASSETSTORE::CAssetStoreJson::FromText( text, json ) ? ASSETSTORE::ASSETSTORE_STATUS_OK : ASSETSTORE::ASSETSTORE_STATUS_FAILED;
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreProvider::RenewSession( void )
{
    {
        MT::CScopeMutex lock( m_lock );
        ++m_renewCount;
    }
    if ( GUCEF_NULL == g_fakeStoreServer )
        return false;
    WEB::CHTTPClient client;
    WEB::CHttpRequestData request;
    request.SetUrl( g_fakeStoreServer->GetBaseUrl() + "/token" );
    request.requestType = "POST";
    WEB::CHttpResponseData response;
    if ( !client.Execute( request, response ) || 200 != response.statusCode )
        return false;
    CORE::CString token;
    token.Set( response.content.AsConstTypePtr< char >(), response.content.GetDataSize() );
    SetToken( token );
    return true;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeStoreProvider::QueryLibrary( const ASSETSTORE::CAssetStoreQuery& query, ASSETSTORE::CAssetStoreLibraryPage& page )
{
    CORE::CDataNode json;
    const ASSETSTORE::TAssetStoreStatus status = Get( "/library?cursor=" + query.cursor + "&pageSize=" + CORE::ToString( query.pageSize ) +
                                                      "&text=" + query.text + "&type=" + query.listingType, json );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
        return status;
    CORE::CDataNodeSerializableSettings settings;
    return page.Deserialize( json, settings ) ? ASSETSTORE::ASSETSTORE_STATUS_OK : ASSETSTORE::ASSETSTORE_STATUS_FAILED;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeStoreProvider::GetItem( const CORE::CString& itemId, ASSETSTORE::CAssetStoreItem& item )
{
    CORE::CDataNode json;
    const ASSETSTORE::TAssetStoreStatus status = Get( "/items/" + itemId, json );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
        return status;
    CORE::CDataNodeSerializableSettings settings;
    if ( !item.Deserialize( json, settings ) )
        return ASSETSTORE::ASSETSTORE_STATUS_FAILED;
    item.providerId = m_providerId;
    return ASSETSTORE::ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeStoreProvider::PlanDownload( const CORE::CString& itemId, const CORE::CString& formatId,
                                  const CORE::CString::StringVector& fileIds, ASSETSTORE::CAssetStoreDownloadPlan& plan )
{
    {
        MT::CScopeMutex lock( m_lock );
        ++m_planCount;
    }
    ASSETSTORE::CAssetStoreItem item;
    const ASSETSTORE::TAssetStoreStatus status = GetItem( itemId, item );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
        return status;
    const ASSETSTORE::CAssetStoreFormat* format = item.FindFormat( formatId );
    if ( GUCEF_NULL == format )
        return ASSETSTORE::ASSETSTORE_STATUS_NOT_FOUND;
    plan = ASSETSTORE::CAssetStoreDownloadPlan();
    plan.providerId = m_providerId;
    plan.itemId = itemId;
    plan.formatId = formatId;
    plan.fileIds = fileIds;
    for ( size_t f=0; f<format->files.size(); ++f )
    {
        const ASSETSTORE::CAssetStoreFile& file = format->files[ f ];
        if ( !fileIds.empty() && fileIds.end() == std::find( fileIds.begin(), fileIds.end(), file.fileId ) )
            continue;
        ASSETSTORE::CAssetStoreTransfer transfer;
        transfer.fileId = file.fileId;
        transfer.url = g_fakeStoreServer->GetBaseUrl() + "/files/" + file.fileId;
        transfer.size = file.size;
        transfer.digest = file.digest;
        transfer.digestAlgorithm = file.digestAlgorithm;
        transfer.targetName = file.name;
        plan.transfers.push_back( transfer );
    }
    return ASSETSTORE::ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreProvider::AuthorizeTransferRequest( const ASSETSTORE::CAssetStoreTransfer& transfer, WEB::CHttpRequestData& request )
{
    MT::CScopeMutex lock( m_lock );
    request.otherHeaders[ "Authorization" ] = "Bearer " + m_token.Reveal();
    return true;
}

/*-------------------------------------------------------------------------*/

CFakeStoreProviderFactory::CFakeStoreProviderFactory( const CORE::CString& providerId )
    : TBase()
    , m_providerId( providerId )
{
}

/*-------------------------------------------------------------------------*/

CFakeStoreProviderFactory::~CFakeStoreProviderFactory()
{
}

/*-------------------------------------------------------------------------*/

CFakeStoreProviderFactory::TProductPtr
CFakeStoreProviderFactory::Create( void )
{
    TProductPtr product = TBase::Create();
    CFakeStoreProvider* provider = static_cast< CFakeStoreProvider* >( product.GetPointerAlways() );
    if ( GUCEF_NULL != provider )
        provider->SetProviderId( m_providerId );
    return product;
}

/*-------------------------------------------------------------------------*/

CFakeInteractiveSignIn::CFakeInteractiveSignIn( CFakeSignInStoreProvider& provider )
    : ASSETSTORE::CIAssetStoreInteractiveSignIn()
    , m_provider( provider )
    , m_running( false )
    , m_exchanging( false )
    , m_polls( 0 )
    , m_received()
    , m_beginCount( 0 )
    , m_cancelCount( 0 )
{
}

/*-------------------------------------------------------------------------*/

CFakeInteractiveSignIn::~CFakeInteractiveSignIn()
{
}

/*-------------------------------------------------------------------------*/

const char*
CFakeInteractiveSignIn::RightCode( void )
{
    return "RIGHTCODE1234";
}

/*-------------------------------------------------------------------------*/

const char*
CFakeInteractiveSignIn::WrongCode( void )
{
    return "WRONGCODE9999";
}

/*-------------------------------------------------------------------------*/

const char*
CFakeInteractiveSignIn::LoginUrl( void )
{
    return "https://login.fake.invalid/authorize?client=fake-public-client";
}

/*-------------------------------------------------------------------------*/

bool
CFakeInteractiveSignIn::Begin( ASSETSTORE::TAssetSignInPromptVector& prompts, CORE::CString& reason )
{
    if ( !m_provider.IsProvisioned() )
    {
        reason = "The client credentials of the store are not provisioned on the server";
        return false;
    }
    ++m_beginCount;
    m_running = true;
    m_exchanging = false;
    m_polls = 0;
    m_received.Clear();
    ASSETSTORE::CAssetSignInPrompt open;
    open.kind = "openUrl";
    open.text = "Sign in to the store in your own browser, then copy the code the page shows";
    open.url = LoginUrl();
    prompts.push_back( open );
    ASSETSTORE::CAssetSignInPrompt code;
    code.kind = "enterCode";
    code.text = "Paste the code the page shows";
    code.sensitiveAnswer = true;
    prompts.push_back( code );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeInteractiveSignIn::Answer( const CORE::CString& promptKind, const AUTH::CSecretString& answer, CORE::CString& reason )
{
    if ( !m_running || m_exchanging || "enterCode" != promptKind )
    {
        reason = "No sign in waits for a code";
        return false;
    }
    const CORE::CString code = answer.Reveal();
    bool shaped = code.Length() >= 8 && code.Length() <= 32;
    for ( UInt32 i=0; shaped && i<code.Length(); ++i )
    {
        const char c = code[ i ];
        shaped = ( c >= '0' && c <= '9' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' );
    }
    if ( !shaped )
    {
        reason = "That is not a code: paste the code the page shows";
        return false;
    }
    m_received = code;
    m_exchanging = true;
    m_polls = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeInteractiveSignIn::Cancel( void )
{
    if ( m_running )
        ++m_cancelCount;
    m_running = false;
    m_exchanging = false;
    m_received.Clear();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreSignInState
CFakeInteractiveSignIn::Update( CORE::CString& message )
{
    if ( !m_running )
        return ASSETSTORE::ASSETSTORE_SIGNINSTATE_SIGNED_OUT;
    if ( !m_exchanging )
        return ASSETSTORE::ASSETSTORE_SIGNINSTATE_IN_PROGRESS;
    if ( ++m_polls < 2 )
        return ASSETSTORE::ASSETSTORE_SIGNINSTATE_IN_PROGRESS;
    m_running = false;
    m_exchanging = false;
    const bool good = m_received == RightCode();
    m_received.Clear();
    if ( !good )
    {
        message = "The store refused the code: it may be wrong or expired";
        return ASSETSTORE::ASSETSTORE_SIGNINSTATE_FAILED;
    }
    m_provider.SetSignedIn( true );
    return ASSETSTORE::ASSETSTORE_SIGNINSTATE_SIGNED_IN;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeInteractiveSignIn::GetBeginCount( void ) const
{
    return m_beginCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeInteractiveSignIn::GetCancelCount( void ) const
{
    return m_cancelCount;
}

/*-------------------------------------------------------------------------*/

CFakeSignInStoreProvider::CFakeSignInStoreProvider( void )
    : ASSETSTORE::CIAssetStoreProvider()
    , m_providerId( "fakesignin" )
    , m_webSession( false )
    , m_signedIn( false )
    , m_refused( false )
    , m_configured( false )
    , m_provisioned( false )
    , m_clientIdFile()
    , m_importedCookies( 0 )
    , m_importedUserAgent()
    , m_signIn( *this )
    , m_lock()
{
}

/*-------------------------------------------------------------------------*/

CFakeSignInStoreProvider::~CFakeSignInStoreProvider()
{
    WaitForAsyncOperations();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInStoreProvider::SetProviderId( const CORE::CString& providerId )
{
    MT::CScopeMutex lock( m_lock );
    m_providerId = providerId;
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInStoreProvider::SetWebSession( bool webSession )
{
    MT::CScopeMutex lock( m_lock );
    m_webSession = webSession;
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInStoreProvider::SetSignedIn( bool signedIn )
{
    {
        MT::CScopeMutex lock( m_lock );
        m_signedIn = signedIn;
        m_refused = false;
    }
    NotifySessionStateChanged();
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInStoreProvider::ExpireSession( void )
{
    SetSignedIn( false );
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInStoreProvider::RefuseSession( void )
{
    MT::CScopeMutex lock( m_lock );
    m_refused = true;
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInStoreProvider::SessionChanged( void )
{
    NotifySessionStateChanged();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeSignInStoreProvider::ImportWebSession( const AUTH::CWebSession& session, CORE::CString& message )
{
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_webSession )
        {
            message = "not a web session store";
            return ASSETSTORE::ASSETSTORE_STATUS_UNSUPPORTED;
        }
        UInt32 kept = 0;
        for ( size_t i=0; i<session.cookies.size(); ++i )
        {
            const CORE::CString domain = session.cookies[ i ].domain.Lowercase();
            if ( !session.cookies[ i ].value.IsEmpty() && ( domain == "webstore.test" || domain.EndsWith( ".webstore.test" ) ) )
                ++kept;
        }
        if ( 0 == kept )
        {
            message = "no cookie of the store";
            return ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT;
        }
        m_importedCookies = kept;
        m_importedUserAgent = session.userAgent;
        m_signedIn = true;
        m_refused = false;
        message = CORE::ToString( kept ) + " cookies of the session were taken";
    }
    NotifySessionStateChanged();
    return ASSETSTORE::ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeSignInStoreProvider::GetImportedCookieCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_importedCookies;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeSignInStoreProvider::GetImportedUserAgent( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_importedUserAgent;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInStoreProvider::WasConfigured( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_configured;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeSignInStoreProvider::GetClientIdFile( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_clientIdFile;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInStoreProvider::IsProvisioned( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_provisioned;
}

/*-------------------------------------------------------------------------*/

CFakeInteractiveSignIn&
CFakeSignInStoreProvider::GetFakeSignIn( void )
{
    return m_signIn;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFakeSignInStoreProvider::GetProviderId( void ) const
{
    return m_providerId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeSignInStoreProvider::GetDisplayName( void ) const
{
    return "Fake sign in store " + m_providerId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeSignInStoreProvider::GetAuthProviderName( void ) const
{
    return m_webSession ? CORE::CString::Empty : CORE::CString( "fake-oauth" );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreSignInKind
CFakeSignInStoreProvider::GetSignInKind( void ) const
{
    return m_webSession ? ASSETSTORE::ASSETSTORE_SIGNIN_WEB_SESSION : ASSETSTORE::ASSETSTORE_SIGNIN_AUTH_PROVIDER;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::CIAssetStoreInteractiveSignIn*
CFakeSignInStoreProvider::GetInteractiveSignIn( void )
{
    return m_webSession ? GUCEF_NULL : &m_signIn;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInStoreProvider::ConfigureSignIn( const ASSETSTORE::CAssetStoreSignInContext& context, CORE::CString& problem )
{
    MT::CScopeMutex lock( m_lock );
    m_configured = true;
    if ( m_webSession )
        return true;
    const CORE::CDataNode* settings = context.GetStoreSettings( m_providerId );
    if ( GUCEF_NULL == settings )
    {
        problem = "The client credentials of the store are not provisioned on the server";
        return true;
    }
    m_clientIdFile = settings->GetAttributeValueOrChildValueByName( "clientIdFile" ).AsString();
    m_provisioned = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInStoreProvider::IsSignedIn( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_signedIn;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInStoreProvider::SignOut( void )
{
    SetSignedIn( false );
    return true;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeSignInStoreProvider::QueryLibrary( const ASSETSTORE::CAssetStoreQuery& query, ASSETSTORE::CAssetStoreLibraryPage& page )
{
    MT::CScopeMutex lock( m_lock );
    if ( m_refused )
        return ASSETSTORE::ASSETSTORE_STATUS_SESSION_EXPIRED;
    if ( !m_signedIn )
        return ASSETSTORE::ASSETSTORE_STATUS_NOT_SIGNED_IN;
    page.Clear();
    return ASSETSTORE::ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeSignInStoreProvider::GetItem( const CORE::CString& itemId, ASSETSTORE::CAssetStoreItem& item )
{
    return ASSETSTORE::ASSETSTORE_STATUS_NOT_FOUND;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeSignInStoreProvider::PlanDownload( const CORE::CString& itemId, const CORE::CString& formatId,
                                        const CORE::CString::StringVector& fileIds, ASSETSTORE::CAssetStoreDownloadPlan& plan )
{
    return ASSETSTORE::ASSETSTORE_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeSignInStoreProvider::GetPlanProblem( const CORE::CString& itemId, const CORE::CString& formatId ) const
{
    return "the fake sign in store has no downloads (" + itemId + " " + formatId + ")";
}

/*-------------------------------------------------------------------------*/

CFakeSignInStoreProviderFactory::CFakeSignInStoreProviderFactory( const CORE::CString& providerId, bool webSession )
    : TBase()
    , m_providerId( providerId )
    , m_webSession( webSession )
{
}

/*-------------------------------------------------------------------------*/

CFakeSignInStoreProviderFactory::~CFakeSignInStoreProviderFactory()
{
}

/*-------------------------------------------------------------------------*/

CFakeSignInStoreProviderFactory::TProductPtr
CFakeSignInStoreProviderFactory::Create( void )
{
    TProductPtr product = TBase::Create();
    CFakeSignInStoreProvider* provider = static_cast< CFakeSignInStoreProvider* >( product.GetPointerAlways() );
    if ( GUCEF_NULL != provider )
    {
        provider->SetProviderId( m_providerId );
        provider->SetWebSession( m_webSession );
    }
    return product;
}

/*-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
