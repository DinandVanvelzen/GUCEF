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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */
#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */
#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#include "gucefAUTH_CFileCredentialStore.h"
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */
#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_H
#include "gucefASSETSTORE.h"
#define GUCEF_ASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_H ? */
#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "TestAssetStoreSupport.h"
#include "TestFakeLoginSite.h"
#include "TestFakeSignInBrowser.h"
#include "TestFakeStoreProvider.h"
#include "TestWebSignIn.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETSTORE;
using namespace ASSETSTORETEST;
using namespace AUTHTEST;


namespace {

const char* TestEmail = "owner@example.test";
const char* TestPassword = "Pa55-w0rd!\"q'Zx";
const char* TestEmailCode = "731905";
const char* StoreId = "fakeweb";

/** A login script for the fake login site */
WEB::CWebLoginScript
MakeSiteScript( const CFakeLoginSite& site )
{
    WEB::CWebLoginScript script;
    script.name = "Fake login site";
    script.startUrl = site.GetBaseUrl() + "/login";
    script.cookieDomains.push_back( "127.0.0.1" );
    script.credentialSlots.push_back( "username" );
    script.credentialSlots.push_back( "password" );
    script.signedIn.selector = "#account";
    script.signedIn.cookie = "sid";
    script.sessionEndedStatusCodes.push_back( 401 );
    script.sessionEndedRedirectPattern = site.GetBaseUrl() + "/login*";
    script.stepTimeoutInMs = 3000;

    WEB::CWebLoginScriptStep step;
    step.action = WEB::CWebLoginScriptStep::ACTION_FILL;
    step.selector = "input[name=email]";
    step.credential = "username";
    script.steps.push_back( step );
    step.selector = "input[name=\"password\"]";
    step.credential = "password";
    script.steps.push_back( step );

    step = WEB::CWebLoginScriptStep();
    step.action = WEB::CWebLoginScriptStep::ACTION_SUBMIT;
    step.selector = "form#login";
    script.steps.push_back( step );

    step = WEB::CWebLoginScriptStep();
    step.action = WEB::CWebLoginScriptStep::ACTION_FAIL_CHECK;
    step.condition.selector = ".login-error";
    step.reason = "The site refused the e-mail address or password";
    script.steps.push_back( step );

    step = WEB::CWebLoginScriptStep();
    step.action = WEB::CWebLoginScriptStep::ACTION_PERSON_CHECK;
    step.condition.selector = "#captcha";
    step.kind = "captcha";
    step.prompt = "Solve the puzzle in the browser window";
    script.steps.push_back( step );

    step = WEB::CWebLoginScriptStep();
    step.action = WEB::CWebLoginScriptStep::ACTION_PERSON_CHECK;
    step.condition.selector = "input[name=code]";
    step.kind = "code";
    step.prompt = "Type the code from your e-mail in the browser window";
    script.steps.push_back( step );

    step = WEB::CWebLoginScriptStep();
    step.action = WEB::CWebLoginScriptStep::ACTION_WAIT_FOR_SIGNED_IN;
    step.timeoutInMs = 5000;
    script.steps.push_back( step );
    return script;
}

AUTH::CWebSessionSignInSettings
MakeSettings( const CFakeLoginSite& site )
{
    AUTH::CWebSessionSignInSettings settings;
    settings.storeId = StoreId;
    settings.script = MakeSiteScript( site );
    settings.signInTimeoutInMs = 30000;
    settings.handOverTimeoutInMs = 10000;
    settings.pollIntervalInMs = 20;
    settings.browser.profileDirectory = TestDirectory( "websignin-profile" );
    return settings;
}

AUTH::CWebSignInCredentials
MakeCredentials( const char* password )
{
    AUTH::CWebSignInCredentials credentials;
    credentials.Set( "username", AUTH::CSecretString( TestEmail ) );
    credentials.Set( "password", AUTH::CSecretString( password ) );
    return credentials;
}

/** GETs the library of the site with the jar; the answer for CheckResponse() */
int
GetLibrary( const CFakeLoginSite& site, WEB::CHttpCookieJarPtr jar, WEB::CHttpResponseData& response )
{
    WEB::CHttpRequestData request;
    request.SetUrl( site.GetBaseUrl() + "/api/library" );
    request.requestType = "GET";
    WEB::CHttpClientRequestOptions options;
    options.cookieJar = jar;
    options.followRedirects = false;
    options.totalTimeoutInMs = 10000;
    WEB::CHTTPClient client;
    response = WEB::CHttpResponseData();
    if ( !client.Execute( request, options, response ) )
        return 0;
    return response.statusCode;
}

/** Whether the bytes of the file hold the text */
bool
FileHolds( const CORE::CString& path, const CORE::CString& text )
{
    CORE::CDynamicBuffer content;
    if ( text.IsNULLOrEmpty() || !content.LoadContentFromFile( path ) || 0 == content.GetDataSize() )
        return false;
    const UInt32 textSize = text.ByteSize() - 1;
    const char* bytes = content.AsConstTypePtr< char >();
    for ( UInt32 i=0; i+textSize<=content.GetDataSize(); ++i )
        if ( 0 == memcmp( bytes + i, text.C_String(), textSize ) )
            return true;
    return false;
}

/** Whether a file of the folder (not its sub folders) holds the text */
bool
FolderHolds( const CORE::CString& folder, const CORE::CString& text )
{
    const char* names[] = { "credentials.bin", "credentials.key", "credentials.lock" };
    for ( UInt32 i=0; i<3; ++i )
    {
        CORE::CString path = folder;
        CORE::AppendToPath( path, names[ i ] );
        if ( CORE::FileExists( path ) && FileHolds( path, text ) )
            return true;
    }
    return false;
}

/** A store of the fake site on the provider base class: its library needs the web session */
class CTestWebStoreProvider : public CWebSessionAssetStoreProvider
{
    public:

    explicit CTestWebStoreProvider( const CFakeLoginSite& site ) : CWebSessionAssetStoreProvider(), m_site( site ) {}

    virtual ~CTestWebStoreProvider()
    {
        WaitForAsyncOperations();
        SignalUpcomingDestruction();
    }

    virtual const CORE::CString& GetProviderId( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString id = StoreId;
        return id;
    }

    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Fake web store"; }

    virtual TAssetStoreStatus QueryLibrary( const CAssetStoreQuery& query, CAssetStoreLibraryPage& page ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !IsSignedIn() )
            return ASSETSTORE_STATUS_NOT_SIGNED_IN;
        WEB::CHttpResponseData response;
        GetLibrary( m_site, GetCookieJar(), response );
        if ( CheckResponse( response ) )
            return ASSETSTORE_STATUS_SESSION_EXPIRED;
        if ( 200 != response.statusCode )
            return ASSETSTORE_STATUS_HTTP_ERROR;
        CAssetStoreItem item;
        item.itemId = "pack-1";
        item.title = "Pack one";
        page.items.push_back( item );
        return ASSETSTORE_STATUS_OK;
    }

    virtual TAssetStoreStatus GetItem( const CORE::CString& itemId, CAssetStoreItem& item ) GUCEF_VIRTUAL_OVERRIDE { return ASSETSTORE_STATUS_NOT_FOUND; }

    virtual TAssetStoreStatus PlanDownload( const CORE::CString& itemId, const CORE::CString& formatId, const CORE::CString::StringVector& fileIds,
                                            CAssetStoreDownloadPlan& plan ) GUCEF_VIRTUAL_OVERRIDE { return ASSETSTORE_STATUS_UNSUPPORTED; }

    private:

    const CFakeLoginSite& m_site;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformProviderTests( CFakeLoginSite& site, AUTH::ICredentialStore& store )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.FakeSite" );

    const AUTH::CWebSessionSignInSettings settings = MakeSettings( site );

    GUCEF_TESTFW_TESTCASE( "Test: The provider base class: sign in kind, cookies on requests, renewal and sign out" )
        try
        {
            CFakeStoreProvider plainProvider;
            ASSERT_TRUE( ASSETSTORE_SIGNIN_NONE == plainProvider.GetSignInKind() && GUCEF_NULL == plainProvider.GetWebSessionSignIn() );

            // the account of the site and the saved credentials of the store the sign in uses
            site.SetAccount( TestEmail, TestPassword );
            site.SetChallenge( false );
            site.SetEmailCode( CORE::CString::Empty );
            ASSERT_TRUE( MakeCredentials( TestPassword ).SaveTo( store, StoreId ) );

            CTestWebStoreProvider provider( site );
            AUTH::CWebSessionSignInSettings providerSettings = settings;
            providerSettings.storeId.Clear();
            CAssetStoreGlobal::Instance()->SetWebSignInStores( &store, &store );     // the stores the application set
            ASSERT_TRUE( provider.ConfigureWebSignIn( providerSettings, GUCEF_NULL, GUCEF_NULL ) );
            CAssetStoreGlobal::Instance()->SetWebSignInStores( GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ASSETSTORE_SIGNIN_WEB_SESSION == provider.GetSignInKind() );
            ASSERT_TRUE( GUCEF_NULL != provider.GetWebSessionSignIn() && provider.GetWebSessionSignIn()->GetStoreId() == StoreId );
            ASSERT_TRUE( CORE::CString( "webSession" ) == AssetStoreSignInKindToString( provider.GetSignInKind() ) );

            CFakeSignInBrowser browser;
            provider.GetWebSessionSignIn()->SetBrowser( &browser );
            if ( !provider.IsSignedIn() )
                ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OK == provider.GetWebSessionSignIn()->SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE ) );
            PumpFor( 20 );
            provider.RefreshCookieJar();
            ASSERT_TRUE( provider.IsSignedIn() );

            CAssetStoreLibraryPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == provider.QueryLibrary( CAssetStoreQuery(), page ) );
            ASSERT_TRUE( 1 == page.items.size() );

            WEB::CHttpRequestData request;
            request.SetUrl( site.GetBaseUrl() + "/files/pack-1.zip" );
            CAssetStoreTransfer transfer;
            ASSERT_TRUE( provider.AuthorizeTransferRequest( transfer, request ) );
            ASSERT_TRUE( request.otherHeaders[ "Cookie" ].HasSubstr( "sid=" ) == 0 );

            // the session ends, the download manager asks for a renewal
            site.ExpireSessions();
            page = CAssetStoreLibraryPage();
            ASSERT_TRUE( ASSETSTORE_STATUS_SESSION_EXPIRED == provider.QueryLibrary( CAssetStoreQuery(), page ) );
            ASSERT_FALSE( provider.IsSignedIn() );
            ASSERT_TRUE( provider.RenewSession() );
            ASSERT_TRUE( provider.IsSignedIn() );
            page = CAssetStoreLibraryPage();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == provider.QueryLibrary( CAssetStoreQuery(), page ) );

            ASSERT_TRUE( provider.SignOut() );
            ASSERT_FALSE( provider.IsSignedIn() );
            WEB::CHttpRequestData afterSignOut;
            afterSignOut.SetUrl( site.GetBaseUrl() + "/files/pack-1.zip" );
            ASSERT_FALSE( provider.AddSessionCookies( afterSignOut ) );
            provider.GetWebSessionSignIn()->ForgetCredentials();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformWebSignInTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING web session sign in provider TESTS" );

    // every log line of these tests is kept, to check that nothing secret is in one
    TESTFW::CLogTextCapture capture;

    // sessions and credentials in a sealed file when the AEAD codec is there, else in memory
    const CORE::CString storeFolder = TestDirectory( "websignin-credentials" );
    if ( CORE::DirExists( storeFolder ) )
        CORE::DeleteDir( storeFolder, true );
    CORE::CreateDirs( storeFolder );
    const bool sealed = AUTH::CFileCredentialStore::IsAvailable();
    if ( !sealed )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSignIn tests: the AEAD codec (codecspluginMONOCYPHER) is not loaded, the credential store is in memory and the sealing is not checked" );
    AUTH::CFileCredentialStore fileStore( storeFolder );
    AUTH::CMemoryCredentialStore memoryStore;
    AUTH::ICredentialStore& store = sealed ? static_cast< AUTH::ICredentialStore& >( fileStore ) : static_cast< AUTH::ICredentialStore& >( memoryStore );

    CFakeLoginSite site;
    {
        GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.FakeSite" );
        GUCEF_TESTFW_TESTCASE( "Test: The fake login site starts" )
            try
            {
                ASSERT_TRUE( site.Start() );
            }
            catch( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }
    PerformProviderTests( site, store );

    {
        GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.Secrets" );
        GUCEF_TESTFW_TESTCASE( "Test: No password, e-mail code, address or session cookie reached a log or the credential store files" )
            try
            {
                CORE::CString::StringVector secrets = site.GetIssuedSessionValues();
                ASSERT_FALSE( secrets.empty() );
                secrets.push_back( TestPassword );
                secrets.push_back( TestEmailCode );
                secrets.push_back( TestEmail );
                secrets.push_back( "cookieSecretValue42" );
                for ( size_t i=0; i<secrets.size(); ++i )
                {
                    ASSERT_FALSE( capture.Contains( secrets[ i ] ) );
                    if ( sealed )
                        ASSERT_FALSE( FolderHolds( storeFolder, secrets[ i ] ) );
                }
                ASSERT_TRUE( capture.Contains( "WebSessionSignIn(fakeweb)" ) );     // the sign ins did log
            }
            catch( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }

    site.Stop();
    fileStore.Wipe();
    CORE::DeleteDir( storeFolder, true );
    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL web session sign in provider TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
