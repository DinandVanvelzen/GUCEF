/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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
#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */
#ifndef GUCEF_WEB_CWEBLOGINSCRIPT_H
#include "gucefWEB_CWebLoginScript.h"
#define GUCEF_WEB_CWEBLOGINSCRIPT_H
#endif /* GUCEF_WEB_CWEBLOGINSCRIPT_H ? */
#ifndef GUCEF_WEB_CWEBSIGNINEVENTDATA_H
#include "gucefWEB_CWebSignInEventData.h"
#define GUCEF_WEB_CWEBSIGNINEVENTDATA_H
#endif /* GUCEF_WEB_CWEBSIGNINEVENTDATA_H ? */
#ifndef GUCEF_WEB_CWEBSIGNINRESULT_H
#include "gucefWEB_CWebSignInResult.h"
#define GUCEF_WEB_CWEBSIGNINRESULT_H
#endif /* GUCEF_WEB_CWEBSIGNINRESULT_H ? */
#ifndef GUCEF_WEB_CCHROMEBROWSERLOCATOR_H
#include "gucefWEB_CChromeBrowserLocator.h"
#define GUCEF_WEB_CCHROMEBROWSERLOCATOR_H
#endif /* GUCEF_WEB_CCHROMEBROWSERLOCATOR_H ? */
#ifndef GUCEF_TEST_FRAMEWORK_H
#ifndef GUCEF_CORE_CBINARYWIREREADER_H
#include "gucefCORE_CBinaryWireReader.h"
#define GUCEF_CORE_CBINARYWIREREADER_H
#endif /* GUCEF_CORE_CBINARYWIREREADER_H ? */

#ifndef GUCEF_WEB_CWEBJSON_H
#include "gucefWEB_CWebJson.h"
#define GUCEF_WEB_CWEBJSON_H
#endif /* GUCEF_WEB_CWEBJSON_H ? */

#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */
#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#include "TestPump.h"
#include "TestFakeLoginSite.h"
#include "TestFakeSignInBrowser.h"
#include "TestWebSessionSignIn.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace AUTHTEST;

namespace {

const char* TestEmail = "owner@example.test";
const char* TestPassword = "Pa55-w0rd!\"q'Zx";
const char* TestEmailCode = "731905";
const char* StoreId = "fakeweb";

/** A folder for the files of a test, below the working directory */
CORE::CString
TestDirectory( const CORE::CString& name )
{
    return CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "authtemp" ), name );
}

/** The outcome of a sign in from its future (CWebSessionSignIn::SignInAsync()): a failed one says "status: message" */
WEB::TWebSignInStatus
SignInStatusOfFuture( const CORE::CFutureResult& future )
{
    if ( !future.IsReady() )
        return WEB::WEBSIGNIN_STATUS_FAILED;
    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() )
        return WEB::WEBSIGNIN_STATUS_FAILED;
    const CORE::TTaskStatus taskStatus = task->GetTaskStatus();
    if ( CORE::TASKSTATUS_FINISHED == taskStatus )
        return WEB::WEBSIGNIN_STATUS_OK;
    if ( CORE::TASKSTATUS_CANCELLED == taskStatus )
        return WEB::WEBSIGNIN_STATUS_CANCELLED;
    return WEB::WebSignInStatusFromString( task->GetTaskStatusExtraInfo().SubstrToChar( ':', true ) );
}

/** Pulses until the future is ready; false when the time ran out */
bool
WaitForFuture( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=2 )
    {
        if ( future.IsReady() || future.HasNoFuture() )
            return future.IsReady();
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 2 );
    }
    return future.IsReady();
}

/** Waits for the future and takes its result data; false when it did not complete with a T */
template < class T >
bool
GetFutureResult( const CORE::CFutureResult& future, T& result, UInt32 timeoutInMs = 30000 )
{
    if ( !WaitForFuture( future, timeoutInMs ) )
        return false;
    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() || CORE::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;
    const T* data = CORE::CICloneable::CloneableAs< T >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;
    result = *data;
    return true;
}

/** Whether the problems hold a text */
bool
ProblemsMention( const CORE::CString::StringVector& problems, const char* text )
{
    for ( size_t i=0; i<problems.size(); ++i )
        if ( problems[ i ].HasSubstr( text, true ) >= 0 )
            return true;
    return false;
}

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
        , lastPrompt(), lastKind(), lastStatus( WEB::WEBSIGNIN_STATUS_OK ), lastHandedOver( false ), startingHeadless( false )
        , m_action( ACT_NONE ), m_browser( GUCEF_NULL ), m_signIn( GUCEF_NULL ), m_site( GUCEF_NULL ), m_fills(), m_clicks()
    {}

    void ActAsPerson( CFakeSignInBrowser* browser )                 { m_action = ACT_PERSON; m_browser = browser; }
    void Fill( const CORE::CString& selector, const CORE::CString& value ) { m_fills.push_back( selector ); m_fills.push_back( value ); }
    void ClickOn( const CORE::CString& selector )                   { m_clicks.push_back( selector ); }
    void CancelOnOwnerAction( AUTH::CWebSessionSignIn* signIn )           { m_action = ACT_CANCEL; m_signIn = signIn; }
    void CloseWindowOnOwnerAction( CFakeSignInBrowser* browser )    { m_action = ACT_CLOSE_WINDOW; m_browser = browser; }
    void ApproveOnOwnerAction( CFakeLoginSite* site )               { m_action = ACT_APPROVE; m_site = site; }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventId == AUTH::CWebSessionSignIn::SignInStartingEvent )
        {
            ++starting;
            startingHeadless = static_cast< WEB::CWebSignInEventData* >( eventData )->headless;
        }
        else
        if ( eventId == AUTH::CWebSessionSignIn::ProgressEvent )
            ++progress;
        else
        if ( eventId == AUTH::CWebSessionSignIn::OwnerActionNeededEvent )
        {
            ++ownerActions;
            const WEB::CWebSignInEventData* data = static_cast< WEB::CWebSignInEventData* >( eventData );
            lastPrompt = data->message;
            lastKind = data->personCheckKind;
            Act();
        }
        else
        if ( eventId == AUTH::CWebSessionSignIn::SignedInEvent || eventId == AUTH::CWebSessionSignIn::SignInFailedEvent )
        {
            const WEB::CWebSignInResult* result = static_cast< WEB::CWebSignInResult* >( eventData );
            lastStatus = result->status;
            lastHandedOver = result->handedOver;
            if ( eventId == AUTH::CWebSessionSignIn::SignedInEvent )
                ++signedIn;
            else
                ++failed;
        }
        else
        if ( eventId == AUTH::CWebSessionSignIn::SessionEndedEvent )
            ++ended;
    }

    UInt32 starting, progress, ownerActions, signedIn, failed, ended;
    CORE::CString lastPrompt;
    CORE::CString lastKind;
    WEB::TWebSignInStatus lastStatus;
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
    AUTH::CWebSessionSignIn* m_signIn;
    CFakeLoginSite* m_site;
    CORE::CString::StringVector m_fills;
    CORE::CString::StringVector m_clicks;
};

/** Subscribes the recorder to every event of the sign in */
void
Record( CSignInRecorder& recorder, AUTH::CWebSessionSignIn& signIn )
{
    recorder.SubscribeTo( &signIn, AUTH::CWebSessionSignIn::SignInStartingEvent );
    recorder.SubscribeTo( &signIn, AUTH::CWebSessionSignIn::ProgressEvent );
    recorder.SubscribeTo( &signIn, AUTH::CWebSessionSignIn::OwnerActionNeededEvent );
    recorder.SubscribeTo( &signIn, AUTH::CWebSessionSignIn::SignedInEvent );
    recorder.SubscribeTo( &signIn, AUTH::CWebSessionSignIn::SignInFailedEvent );
    recorder.SubscribeTo( &signIn, AUTH::CWebSessionSignIn::SessionEndedEvent );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformSessionTests( AUTH::ICredentialStore& store, const CORE::CString& storeFolder, bool storeIsSealedFile )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.Session" );

    GUCEF_TESTFW_TESTCASE( "Test: A session is one secret, goes into a cookie jar and never shows a cookie value" )
        try
        {
            AUTH::CWebSession session;
            session.storeId = StoreId;
            session.capturedAtUtcInMs = AUTH::CWebSession::NowUtcInMs();
            AUTH::CWebSessionCookie cookie;
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
            cookie.expires = (Float64) ( AUTH::CWebSession::NowUtcInMs() / 1000 ) + 3600.0;
            session.cookies.push_back( cookie );
            cookie.name = "old";
            cookie.expires = 1000.0;
            session.cookies.push_back( cookie );

            AUTH::CSecretString secret;
            ASSERT_TRUE( session.ToSecret( secret ) );
            AUTH::CWebSession again;
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
            AUTH::CWebSession loaded;
            ASSERT_TRUE( loaded.LoadFrom( store, StoreId ) );
            ASSERT_TRUE( 2 == loaded.GetCookieCount() && loaded.HasCookie( "sid" ) );
            ASSERT_FALSE( loaded.LoadFrom( store, "otherstore" ) );
            if ( storeIsSealedFile )
                ASSERT_FALSE( FolderHolds( storeFolder, "cookieSecretValue42" ) );
            ASSERT_TRUE( AUTH::CWebSession::RemoveFrom( store, StoreId ) );
            ASSERT_FALSE( loaded.LoadFrom( store, StoreId ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The secret of a session is exactly its JSON text: the expiry times of Chrome (seconds with a fraction) leave no null or stray bytes behind the text, so it is valid UTF-8 as the RPC requires" )
        try
        {
            // the json codec itself: a tree with numbers whose text has a fraction
            CORE::CDataNode tree( GUCEF_DATATYPE_OBJECT );
            tree.SetAttribute( "name", CORE::CString( "fractions" ) );
            for ( UInt32 i=0; i<22; ++i )
                tree.SetAttribute( "expires" + CORE::ToString( i ), CORE::CVariant( (Float64) 1797094392.774613 + 0.25 * i ) );
            CORE::CString text;
            ASSERT_TRUE( WEB::CWebJson::ToText( tree, text ) );
            UInt32 textSize = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSignIn: json text of 22 numbers with a fraction: " << textSize << " bytes, " << (UInt32) strlen( text.C_String() ) << " up to the first null" );
            ASSERT_TRUE( textSize > 0 && textSize == (UInt32) strlen( text.C_String() ) && '}' == text.C_String()[ textSize - 1 ] );
            ASSERT_TRUE( CORE::CBinaryWireReader::IsValidUtf8( text.C_String(), textSize ) );

            // a session as Chrome gives it: persistent cookies with an expiry of seconds and microseconds, session cookies
            AUTH::CWebSession session;
            session.storeId = StoreId;
            session.capturedAtUtcInMs = AUTH::CWebSession::NowUtcInMs();
            session.userAgent = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36";
            const Float64 nowInSeconds = (Float64) ( AUTH::CWebSession::NowUtcInMs() / 1000 );
            for ( UInt32 i=0; i<22; ++i )
            {
                AUTH::CWebSessionCookie cookie;
                cookie.name = "chromeCookie" + CORE::ToString( i );
                cookie.value.Set( "v" + CORE::ToString( i ) + "-eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiIxMjM0NTY3ODkwIn0.c2lnbmF0dXJl%3D%3D" );
                cookie.domain = 0 == i % 2 ? ".store.example.test" : "www.store.example.test";
                cookie.path = "/";
                cookie.httpOnly = 0 == i % 3;
                cookie.secure = true;
                cookie.session = 0 == i % 4;
                cookie.expires = cookie.session ? -1.0 : nowInSeconds + 86400.0 + 0.774613 + 0.001 * i;
                session.cookies.push_back( cookie );
            }
            AUTH::CSecretString secret;
            ASSERT_TRUE( session.ToSecret( secret ) );
            const CORE::CString& json = secret.Reveal();
            const UInt32 jsonSize = json.ByteSize() > 0 ? json.ByteSize() - 1 : 0;
            // lengths only: never a value
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSignIn: the secret of a session of 22 cookies: " << jsonSize << " bytes, " << (UInt32) strlen( json.C_String() ) << " up to the first null" );
            ASSERT_TRUE( jsonSize > 0 && jsonSize == (UInt32) strlen( json.C_String() ) && '}' == json.C_String()[ jsonSize - 1 ] );
            ASSERT_TRUE( CORE::CBinaryWireReader::IsValidUtf8( json.C_String(), jsonSize ) );
            AUTH::CWebSession again;
            ASSERT_TRUE( again.FromSecret( secret ) && 22 == again.GetCookieCount() && again.userAgent == session.userAgent );
            bool same = true;
            for ( UInt32 i=0; i<22; ++i )
                same = same && again.cookies[ i ].name == session.cookies[ i ].name && again.cookies[ i ].value.Reveal() == session.cookies[ i ].value.Reveal() &&
                       again.cookies[ i ].expires == session.cookies[ i ].expires;
            ASSERT_TRUE( same );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Credentials are kept per slot in the credential store, sealed" )
        try
        {
            AUTH::CWebSignInCredentials credentials = MakeCredentials( TestPassword );
            ASSERT_TRUE( 2 == credentials.GetCount() && credentials.Has( "password" ) && !credentials.Has( "otp" ) );
            ASSERT_TRUE( credentials.SaveTo( store, StoreId ) );

            CORE::CString::StringVector slots;
            slots.push_back( "username" );
            slots.push_back( "password" );
            ASSERT_TRUE( AUTH::CWebSignInCredentials::HasSaved( store, StoreId, slots ) );
            AUTH::CWebSignInCredentials loaded;
            ASSERT_TRUE( loaded.LoadFrom( store, StoreId, slots ) );
            AUTH::CSecretString value;
            ASSERT_TRUE( loaded.Get( "password", value ) && value.Reveal() == TestPassword );
            ASSERT_TRUE( loaded.HasAll( slots ) );
            if ( storeIsSealedFile )
            {
                ASSERT_FALSE( FolderHolds( storeFolder, TestPassword ) );
                ASSERT_FALSE( FolderHolds( storeFolder, TestEmail ) );
            }
            ASSERT_TRUE( AUTH::CWebSignInCredentials::RemoveFrom( store, StoreId, slots ) );
            ASSERT_FALSE( AUTH::CWebSignInCredentials::HasSaved( store, StoreId, slots ) );
            loaded.Clear();
            ASSERT_TRUE( 0 == loaded.GetCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformHandOverTests( CFakeLoginSite& site, AUTH::ICredentialStore& store, const CORE::CString& storeFolder, bool storeIsSealedFile )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.HandOver" );

    const AUTH::CWebSessionSignInSettings settings = MakeSettings( site );

    GUCEF_TESTFW_TESTCASE( "Test: The user agent of the browser travels with the session, and only when it is known" )
        try
        {
            AUTH::CWebSession session;
            session.storeId = StoreId;
            AUTH::CWebSessionCookie cookie;
            cookie.name = "sid";
            cookie.value.Set( "handOverValue77" );
            cookie.domain = "127.0.0.1";
            cookie.path = "/";
            session.cookies.push_back( cookie );
            AUTH::CSecretString secret;
            ASSERT_TRUE( session.ToSecret( secret ) && secret.Reveal().HasSubstr( "userAgent" ) < 0 );
            session.userAgent = "Mozilla/5.0 (X11; Linux x86_64) TestChrome/140.0";
            ASSERT_TRUE( session.ToSecret( secret ) );
            AUTH::CWebSession again;
            ASSERT_TRUE( again.FromSecret( secret ) && again.userAgent == session.userAgent && 1 == again.GetCookieCount() );
            again.Clear();
            ASSERT_TRUE( again.userAgent.IsNULLOrEmpty() );
            secret.Clear();
            session.Clear();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A session handed over from another browser is taken: only the cookies of the store's domains that are valid, saved sealed, SignedInEvent" )
        try
        {
            AUTH::CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetSessionStore( &store );
            signIn.ForgetSession();

            AUTH::CWebSession given;
            given.storeId = "anotherName";      // the store id of the sign in counts, not the one given
            given.userAgent = "Mozilla/5.0 TestChrome/140.0";
            AUTH::CWebSessionCookie cookie;
            cookie.name = "sid";
            cookie.value.Set( "handOverValue77" );
            cookie.domain = "127.0.0.1";
            cookie.path = "/";
            given.cookies.push_back( cookie );
            cookie.name = "tracker";
            cookie.value.Set( "foreignValue88" );
            cookie.domain = ".other.example";
            given.cookies.push_back( cookie );
            cookie.name = "stale";
            cookie.value.Set( "staleValue99" );
            cookie.domain = "127.0.0.1";
            cookie.session = false;
            cookie.expires = 1000.0;
            given.cookies.push_back( cookie );

            UInt32 kept = 0;
            CORE::CString problem;
            ASSERT_TRUE( signIn.AdoptSession( given, kept, problem ) );
            ASSERT_TRUE( 1 == kept && problem.IsNULLOrEmpty() && signIn.HasSession() );
            PumpFor( 50 );
            ASSERT_TRUE( 1 == recorder.signedIn && recorder.lastHandedOver );
            AUTH::CWebSession held;
            ASSERT_TRUE( signIn.GetSession( held ) && held.storeId == StoreId && held.HasCookie( "sid" ) && !held.HasCookie( "tracker" ) && !held.HasCookie( "stale" ) );
            ASSERT_TRUE( held.userAgent == given.userAgent );
            held.Clear();

            // saved: a new sign in object of the store reads it again
            AUTH::CWebSessionSignIn later( settings );
            later.SetSessionStore( &store );
            ASSERT_TRUE( later.LoadSavedSession() );
            if ( storeIsSealedFile )
                ASSERT_FALSE( FolderHolds( storeFolder, "handOverValue77" ) );

            // nothing of the store: refused with the reason, the session that is there stays
            AUTH::CWebSession foreign;
            cookie.name = "tracker";
            cookie.value.Set( "foreignValue88" );
            cookie.domain = ".other.example";
            cookie.session = true;
            cookie.expires = -1.0;
            foreign.cookies.push_back( cookie );
            ASSERT_FALSE( signIn.AdoptSession( foreign, kept, problem ) );
            ASSERT_TRUE( 0 == kept && problem.HasSubstr( "None of the 1 cookies" ) >= 0 && signIn.HasSession() );
            ASSERT_TRUE( problem.HasSubstr( "foreignValue88" ) < 0 );
            given.Clear();
            foreign.Clear();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Cookies the store renews in its answers are kept with the session; other domains and unchanged cookies are not; EndSession forgets it" )
        try
        {
            AUTH::CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetSessionStore( &store );
            ASSERT_TRUE( signIn.LoadSavedSession() );
            WEB::CHttpCookieJar jar;
            signIn.ApplySessionTo( jar );
            ASSERT_FALSE( signIn.UpdateSessionCookies( jar ) );            // nothing changed

            // the store renews the session cookie and sets a csrf cookie; a cookie of another site is in the jar too
            WEB::CHttpCookie renewed;
            renewed.name = "sid";
            renewed.value = "renewedValue55";
            renewed.domain = "127.0.0.1";
            renewed.hostOnly = true;
            renewed.path = "/";
            jar.SetCookie( renewed );
            renewed.name = "csrf";
            renewed.value = "csrfValue66";
            jar.SetCookie( renewed );
            renewed.name = "foreign";
            renewed.value = "foreignValue88";
            renewed.domain = "other.example";
            jar.SetCookie( renewed );
            ASSERT_TRUE( signIn.UpdateSessionCookies( jar ) );
            ASSERT_FALSE( signIn.UpdateSessionCookies( jar ) );            // taken already

            AUTH::CWebSessionSignIn later( settings );
            later.SetSessionStore( &store );
            ASSERT_TRUE( later.LoadSavedSession() );
            AUTH::CWebSession saved;
            ASSERT_TRUE( later.GetSession( saved ) && saved.HasCookie( "csrf" ) && !saved.HasCookie( "foreign" ) && 2 == saved.GetCookieCount() );
            WEB::CHttpCookieJar check;
            saved.ApplyTo( check );
            ASSERT_TRUE( check.GetCookieHeaderValue( "127.0.0.1", "/", false ).HasSubstr( "sid=renewedValue55" ) >= 0 );
            saved.Clear();
            if ( storeIsSealedFile )
                ASSERT_FALSE( FolderHolds( storeFolder, "renewedValue55" ) );

            signIn.EndSession( "the test ended it" );
            PumpFor( 50 );
            ASSERT_TRUE( !signIn.HasSession() && 1 == recorder.ended );
            ASSERT_FALSE( later.LoadSavedSession() );
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

    const AUTH::CWebSessionSignInSettings settings = MakeSettings( site );

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

            AUTH::CWebSessionSignIn signIn;
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
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OK == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message ) );
            ASSERT_TRUE( signIn.HasSession() );
            AUTH::CWebSession session;
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
            AUTH::CWebSessionSignIn signIn( settings );
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
            AUTH::CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );
            signIn.SetCredentialStore( &store );
            signIn.SetSessionStore( &store );
            ASSERT_TRUE( signIn.LoadSavedSession() );
            AUTH::CWebSession oldSession;
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
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OK == signIn.RenewSession( &message ) );
            ASSERT_TRUE( browser.GetLastSettings().headless );     // unattended: no window
            ASSERT_TRUE( 1 == browser.GetClearCookiesCount() );   // the ended session is not taken from the profile again
            ASSERT_TRUE( site.GetStats().sessionsIssued == issuedBefore + 1 );
            AUTH::CWebSession newSession;
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
            AUTH::CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );
            signIn.SetCredentials( MakeCredentials( TestPassword ) );
            recorder.ActAsPerson( &browser );
            recorder.Fill( "#answer", "person" );
            recorder.ClickOn( "#solve" );

            CORE::CString message;
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OK == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message ) );
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
            AUTH::CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );
            signIn.SetCredentials( MakeCredentials( TestPassword ) );
            recorder.ActAsPerson( &browser );
            recorder.Fill( "#code", TestEmailCode );
            recorder.ClickOn( "#verify" );

            CORE::CFutureResult future = signIn.SignInAsync( WEB::WEBSIGNIN_MODE_INTERACTIVE );
            ASSERT_TRUE( 1 == recorder.starting );                  // the notice comes before the browser starts
            CORE::CFutureResult second = signIn.SignInAsync( WEB::WEBSIGNIN_MODE_INTERACTIVE );
            ASSERT_TRUE( WaitForFuture( second, 5000 ) );
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_ALREADY_EXISTS == SignInStatusOfFuture( second ) );

            WEB::CWebSignInResult result;
            ASSERT_TRUE( GetFutureResult( future, result, 30000 ) );
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OK == result.status && result.handedOver && result.storeId == StoreId );
            ASSERT_TRUE( 2 == result.cookieCount );                 // the session and the cookie of the pending sign in
            AUTH::CWebSession session;
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
            AUTH::CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );

            CORE::CString message;
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_NOT_SIGNED_IN == signIn.SignIn( WEB::WEBSIGNIN_MODE_UNATTENDED, &message ) );
            ASSERT_TRUE( 0 == browser.GetStartCount() );
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_NOT_SIGNED_IN == signIn.RenewSession( &message ) );
            ASSERT_TRUE( message.HasSubstr( "sign in again" ) >= 0 );

            signIn.SetCredentials( MakeCredentials( TestPassword ) );
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OWNER_ACTION_NEEDED == signIn.SignIn( WEB::WEBSIGNIN_MODE_UNATTENDED, &message ) );
            ASSERT_TRUE( message.HasSubstr( "Solve the puzzle" ) >= 0 );
            ASSERT_TRUE( 0 == recorder.ownerActions && 1 == browser.GetStopCount() );
            ASSERT_TRUE( browser.GetLastSettings().headless );
            ASSERT_FALSE( signIn.HasSession() );
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OWNER_ACTION_NEEDED == recorder.lastStatus );
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
            AUTH::CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetBrowser( &browser );
            recorder.ActAsPerson( &browser );
            recorder.Fill( "#email", TestEmail );
            recorder.Fill( "#password", TestPassword );
            recorder.ClickOn( "#sign-in" );

            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OK == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE ) );
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
            AUTH::CWebSessionSignIn signIn( settings );
            signIn.SetBrowser( &browser );
            signIn.SetCredentials( MakeCredentials( "not the password" ) );
            CORE::CString message;
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_NOT_SIGNED_IN == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message ) );
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
                AUTH::CWebSessionSignIn signIn( settings );
                CSignInRecorder recorder;
                Record( recorder, signIn );
                signIn.SetBrowser( &browser );
                recorder.CancelOnOwnerAction( &signIn );
                CORE::CFutureResult future = signIn.SignInAsync( WEB::WEBSIGNIN_MODE_INTERACTIVE );
                ASSERT_TRUE( WaitForFuture( future, 30000 ) );
                ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_CANCELLED == SignInStatusOfFuture( future ) );
                PumpFor( 50 );
                ASSERT_TRUE( 1 == recorder.ownerActions && 1 == recorder.failed && WEB::WEBSIGNIN_STATUS_CANCELLED == recorder.lastStatus );
                ASSERT_TRUE( 1 == browser.GetStopCount() );
                ASSERT_FALSE( signIn.HasSession() );
            }
            {
                CFakeSignInBrowser browser;
                AUTH::CWebSessionSignIn signIn( settings );
                CSignInRecorder recorder;
                Record( recorder, signIn );
                signIn.SetBrowser( &browser );
                recorder.CloseWindowOnOwnerAction( &browser );
                ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_CANCELLED == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message ) );
                ASSERT_TRUE( message == "The browser window was closed" );
            }
            {
                AUTH::CWebSessionSignInSettings shortSettings = settings;
                shortSettings.handOverTimeoutInMs = 300;
                CFakeSignInBrowser browser;
                AUTH::CWebSessionSignIn signIn( shortSettings );
                signIn.SetBrowser( &browser );
                ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_TIMED_OUT == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message ) );
                ASSERT_TRUE( message.HasSubstr( "in time" ) >= 0 );
                ASSERT_TRUE( 1 == browser.GetStopCount() );
            }
            {
                AUTH::CWebSessionSignInSettings shortSettings = settings;
                shortSettings.signInTimeoutInMs = 300;
                CFakeSignInBrowser browser;
                AUTH::CWebSessionSignIn signIn( shortSettings );
                signIn.SetBrowser( &browser );
                ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_TIMED_OUT == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message ) );
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
                AUTH::CWebSessionSignIn signIn( settings );
                signIn.SetBrowser( &browser );
                ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_BROWSER_UNAVAILABLE == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message ) );
            }
            {
                // the real browser with a path that is not there: nothing is started
                AUTH::CWebSessionSignInSettings missing = settings;
                missing.browser.executablePath = TestDirectory( "no-such-browser" );
                AUTH::CWebSessionSignIn signIn( missing );
                ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_BROWSER_UNAVAILABLE == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message ) );
                ASSERT_TRUE( message.HasSubstr( "does not exist" ) >= 0 );
            }
            {
                // the start page sends the browser on to "localhost": not a domain of the store, so nothing is typed there
                AUTH::CWebSessionSignInSettings elsewhere = settings;
                elsewhere.script.startUrl = site.GetBaseUrl() + "/elsewhere";
                CFakeSignInBrowser browser;
                AUTH::CWebSessionSignIn signIn( elsewhere );
                signIn.SetBrowser( &browser );
                signIn.SetCredentials( MakeCredentials( TestPassword ) );
                ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_FAILED == signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message ) );
                ASSERT_TRUE( message.HasSubstr( "localhost" ) >= 0 );
                ASSERT_TRUE( 0 == browser.GetFillCount() );
            }
            {
                // settings that are not valid are refused before anything starts
                AUTH::CWebSessionSignInSettings invalid = settings;
                invalid.storeId = "../escape";
                AUTH::CWebSessionSignIn signIn;
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
}

/*-------------------------------------------------------------------------*/

static void
PerformRealBrowserTests( CFakeLoginSite& site )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.RealBrowser" );

    // the web socket of the DevTools client needs the SHA-1 codec of codecspluginSTBRUMMEHASH
    if ( GUCEF_TESTFW_GETENV( "GUCEF_TEST_BROWSER" ) == "1" && !WEB::CWebSocketHandshake::IsAvailable() )
        LoadTestPlugin( "codecspluginSTBRUMMEHASH" );

    GUCEF_TESTFW_TESTCASE( "Test: A real browser, headless and on the loopback only, signs in to the fake site and hands over a challenge" )
        CORE::CString executable;
        GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_BROWSER" ) != "1", "set GUCEF_TEST_BROWSER=1 to sign in with a real browser" );
        GUCEF_TESTFW_SKIP_IF( !WEB::CChromeBrowserLocator::FindBrowser( CORE::CString::Empty, executable ), "no Chrome, Chromium, Edge or Brave on this computer" );
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSignIn test: signing in with " << executable << " without a window" );
            site.SetAccount( TestEmail, TestPassword );
            site.SetChallenge( false );
            site.SetEmailCode( CORE::CString::Empty );

            const CORE::CString profile = TestDirectory( "websignin-chrome-profile" );
            if ( CORE::DirExists( profile ) )
                CORE::DeleteDir( profile, true );

            AUTH::CWebSessionSignInSettings settings = MakeSettings( site );
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
            AUTH::CWebSessionSignIn signIn( settings );
            CSignInRecorder recorder;
            Record( recorder, signIn );
            signIn.SetSessionStore( &store );
            signIn.SetCredentials( MakeCredentials( TestPassword ) );

            CORE::CString message;
            const WEB::TWebSignInStatus status = signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message );
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OK == status );
            if ( WEB::WEBSIGNIN_STATUS_OK != status )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSignIn test: the real browser sign in ended with " << WEB::WebSignInStatusToString( status ) << ": " << message );
            ASSERT_TRUE( recorder.startingHeadless );
            AUTH::CWebSession session;
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
            const WEB::TWebSignInStatus second = signIn.SignIn( WEB::WEBSIGNIN_MODE_INTERACTIVE, &message );
            ASSERT_TRUE( WEB::WEBSIGNIN_STATUS_OK == second );
            if ( WEB::WEBSIGNIN_STATUS_OK != second )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSignIn test: the second real browser sign in ended with " << WEB::WebSignInStatusToString( second ) << ": " << message );
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
PerformWebSessionSignInTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING web session sign in TESTS" );

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
    PerformHandOverTests( site, store, storeFolder, sealed );
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
                secrets.push_back( "handOverValue77" );
                secrets.push_back( "foreignValue88" );
                secrets.push_back( "renewedValue55" );
                secrets.push_back( "csrfValue66" );
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
