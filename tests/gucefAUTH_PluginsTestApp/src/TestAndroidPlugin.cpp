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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H
#include "authpluginANDROID_CGoogleAuthProvider.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H
#include "authpluginANDROID_CAmazonAuthProvider.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_JNIGLUE_H
#include "authpluginANDROID_JniGlue.h"
#define GUCEF_AUTHPLUGIN_ANDROID_JNIGLUE_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_JNIGLUE_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_H
#include "authpluginANDROID.h"
#define GUCEF_AUTHPLUGIN_ANDROID_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_H ? */

#include "TestPluginsCommon.h"
#include "TestFakeAndroid.h"
#include "TestFakeEngine.h"
#include "TestFakeHttp.h"
#include "TestAndroidPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::AUTHPLUGIN::ANDROIDAUTH;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const TestWebClientId = "web-client-id.apps.example.invalid";
static const char* const TestWebClientSecret = "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";

static CORE::CDataNode
MakeGoogleNode( bool credentialManager = true, const char* proofKind = "auth-code" )
{GUCEF_TRACE;

    CORE::CDataNode node( "google", GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "clientId", TestWebClientId );
    node.SetAttribute( "clientSecret", TestWebClientSecret );
    CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    parameters->SetAttribute( "useCredentialManager", credentialManager );
    parameters->SetAttribute( "proofKind", proofKind );
    parameters->SetAttribute( "httpTimeoutInMs", (UInt32) 3000 );
    return node;
}

/*-------------------------------------------------------------------------*/

static bool
ConfigureGoogle( CGoogleAuthProvider& provider )
{GUCEF_TRACE;

    return provider.LoadConfig( MakeGoogleNode() );
}

/*-------------------------------------------------------------------------*/

/** Runs a silent login to its end */
template< class TProvider >
static bool
RunSilent( TProvider& provider, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return provider.TrySilentLogin( future ) && PumpUntilReady( future, 3000 );
}

/*-------------------------------------------------------------------------*/

/** The text repeated: for values that are longer than an account reference can be */
static CORE::CString
Repeat( const char* text, UInt32 count )
{GUCEF_TRACE;

    CORE::CString result;
    for ( UInt32 i=0; i<count; ++i )
        result += text;
    return result;
}

/*-------------------------------------------------------------------------*/

/** A launcher of the test that records what it was asked to open */
class CRecordingLauncher : public CORE::CIBrowserLauncherHandler
{
    public:

    GUCEF::vector< CORE::CString > urls;

    CRecordingLauncher( void ) : CORE::CIBrowserLauncherHandler(), urls() {}

    virtual bool OpenUrl( const CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE
    {
        urls.push_back( url );
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** A receiver of redirects of the test */
class CRecordingReceiver : public CAndroidRedirectReceiver
{
    public:

    GUCEF::vector< CORE::CString > urls;
    bool take;

    CRecordingReceiver( void ) : CAndroidRedirectReceiver(), urls(), take( true ) {}

    virtual bool OnAuthRedirect( const CORE::CString& redirectUrl ) GUCEF_VIRTUAL_OVERRIDE
    {
        urls.push_back( redirectUrl );
        return take;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAndroidPluginTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Android plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "AndroidPlugin" );

    GUCEF_TESTFW_TESTCASE( "Test: The providers are google and amazon, silent and verifiable, with the account namespaces of the built in descriptors" )
        try
        {
            CGoogleAuthProvider google;
            ASSERT_TRUE( google.GetProviderName() == "google" );
            ASSERT_TRUE( ( AUTH::AUTHCAP_SILENT | AUTH::AUTHCAP_SERVER_VERIFIABLE | AUTH::AUTHCAP_REQUIRES_CLIENT_ID ) == google.GetCapabilities() );
            ASSERT_TRUE( google.GetClassTypeName() == "GUCEF::AUTHPLUGIN::ANDROIDAUTH::CGoogleAuthProvider" );
            CAmazonAuthProvider amazon;
            ASSERT_TRUE( amazon.GetProviderName() == "amazon" );
            ASSERT_TRUE( ( AUTH::AUTHCAP_SILENT | AUTH::AUTHCAP_SERVER_VERIFIABLE ) == amazon.GetCapabilities() );

            // google:<sub> and amazon:<user_id> are what the built in descriptors make of the same claims
            AUTH::CAuthProviderDescriptor descriptor;
            CORE::CString::StringMap claims;
            AUTH::CAccountRef builtIn;
            ASSERT_TRUE( AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( AUTH::CAuthProviderRegistry::GoogleName, descriptor ) );
            claims[ "sub" ] = "109876543210987654321";
            ASSERT_TRUE( descriptor.MakeAccountRef( claims, CORE::CString::StringMap(), builtIn ) );
            ASSERT_TRUE( builtIn.ToString() == "google:109876543210987654321" );
            ASSERT_TRUE( AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( AUTH::CAuthProviderRegistry::AmazonName, descriptor ) );
            claims.clear();
            claims[ "user_id" ] = "amzn1.account.AFAKEUSERID123";
            ASSERT_TRUE( descriptor.MakeAccountRef( claims, CORE::CString::StringMap(), builtIn ) );
            ASSERT_TRUE( builtIn.ToString() == "amazon:amzn1.account.AFAKEUSERID123" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without the Java side the providers are not available and no login starts" )
        try
        {
            ASSERT_FALSE( CAndroidAuthBridge::Instance()->HasShim() );
            ASSERT_FALSE( IsJavaSideInitialized() );
            CGoogleAuthProvider google;
            ASSERT_TRUE( ConfigureGoogle( google ) );
            ASSERT_FALSE( google.IsAvailable() );
            CORE::CFutureResult silent( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( google.TrySilentLogin( silent ) );
            ASSERT_FALSE( silent.HasAFuture() );

            CProviderRecorder recorder;
            recorder.SubscribeTo( &google );
            CORE::CFutureResult interactive( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( google.BeginLogin( AUTH::CAuthOptions(), interactive ) );
            ASSERT_TRUE( interactive.IsReady() );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );

            CAmazonAuthProvider amazon;
            ASSERT_FALSE( amazon.IsAvailable() );
            CORE::CFutureResult amazonFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( amazon.TrySilentLogin( amazonFuture ) );
            ASSERT_TRUE( 0 == CAndroidAuthBridge::Instance()->GetRequestCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Google is available with Play services, the Java side and the web client id" )
        try
        {
            CFakeAndroid fake;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            ASSERT_FALSE( google.IsAvailable() );           // no client id yet
            ASSERT_TRUE( ConfigureGoogle( google ) );
            ASSERT_TRUE( google.IsAvailable() );
            fake.googlePlay = false;
            ASSERT_FALSE( google.IsAvailable() );
            fake.googlePlay = true;

            CAmazonAuthProvider amazon;
            ASSERT_TRUE( amazon.IsAvailable() );
            fake.amazon = false;
            ASSERT_FALSE( amazon.IsAvailable() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A silent Google login answered inside the call ends with the account of the id token, the server auth code as proof and the persona" )
        try
        {
            CFakeAndroid fake;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            ASSERT_TRUE( ConfigureGoogle( google ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &google );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( google.TrySilentLogin( future ) );
            ASSERT_TRUE( future.IsReady() );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "google:109876543210987654321" );
            ASSERT_TRUE( result.providerName == "google" );
            ASSERT_TRUE( result.displayName == "Gina Google" );
            ASSERT_TRUE( result.avatarUrl == "https://lh3.example.invalid/photo.jpg" );
            ASSERT_TRUE( result.proof.kind == AUTH::CAuthProof::KindPlatformTicket );
            ASSERT_TRUE( result.proof.data.Reveal() == fake.authCode );
            ASSERT_FALSE( result.idToken.IsEmpty() );

            // What was asked of the Java side
            ASSERT_TRUE( 1 == fake.googleCalls.size() );
            ASSERT_TRUE( ANDROIDSIGNIN_SILENT == fake.googleCalls[ 0 ].mode );
            ASSERT_TRUE( fake.googleCalls[ 0 ].clientId == TestWebClientId && fake.googleCalls[ 0 ].useCredentialManager );
            ASSERT_TRUE( 22 == fake.googleCalls[ 0 ].nonce.Length() );
            ASSERT_TRUE( 0 == CAndroidAuthBridge::Instance()->GetRequestCount() && 0 == google.GetPendingRequestCount() );

            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_SUCCEEDED, 1, 1000 ) );
            ASSERT_TRUE( recorder.records[ 0 ].threadId == MT::GetCurrentTaskID() );
            // The nonces are not the same twice
            ASSERT_TRUE( google.TrySilentLogin( future ) );
            ASSERT_TRUE( fake.googleCalls[ 0 ].nonce != fake.googleCalls[ 1 ].nonce );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An answer from a thread of the Java side completes the login, its events come on the pulse thread, the proof can be the id token" )
        try
        {
            CFakeAndroid fake;
            fake.delivery = CFakeAndroid::DELIVER_THREAD;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            ASSERT_TRUE( google.LoadConfig( MakeGoogleNode( true, "id-token" ) ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &google );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( google.TrySilentLogin( future ) );
            // returned before the answer
            ASSERT_FALSE( future.IsReady() );
            ASSERT_TRUE( 1 == google.GetPendingRequestCount() );
            ASSERT_TRUE( PumpUntilReady( future, 3000 ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.proof.kind == AUTH::CAuthProof::KindOidcIdToken );
            ASSERT_TRUE( result.proof.data.Reveal() == result.idToken.Reveal() );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_SUCCEEDED, 1, 1000 ) );
            ASSERT_TRUE( recorder.records[ 0 ].threadId == MT::GetCurrentTaskID() );
            ASSERT_TRUE( 0 == google.GetPendingRequestCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without Credential Manager there is no id token and with it no account: the login says why" )
        try
        {
            CFakeAndroid fake;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            CProviderRecorder recorder;
            recorder.SubscribeTo( &google );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            ASSERT_TRUE( google.LoadConfig( MakeGoogleNode( false ) ) );
            ASSERT_TRUE( google.TrySilentLogin( future ) );
            ASSERT_TRUE( future.IsReady() );
            ASSERT_TRUE( 1 == fake.googleCalls.size() && !fake.googleCalls[ 0 ].useCredentialManager && fake.googleCalls[ 0 ].nonce.IsNULLOrEmpty() );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( recorder.Last( CProviderRecorder::KIND_FAILED )->failure.message.HasSubstr( "id token" ) >= 0 );

            // Credential Manager is on but gave nothing
            ASSERT_TRUE( ConfigureGoogle( google ) );
            fake.withIdToken = false;
            ASSERT_TRUE( google.TrySilentLogin( future ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 2, 1000 ) );
            ASSERT_TRUE( 0 == recorder.Count( CProviderRecorder::KIND_SUCCEEDED ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: What the Java side reports as the end of a request is told as a reason" )
        try
        {
            CFakeAndroid fake;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            ASSERT_TRUE( ConfigureGoogle( google ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &google );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            struct SCase { EAndroidAuthStatus status; AUTH::TAuthFailureReason reason; };
            const SCase cases[] =
            {
                { ANDROIDAUTH_CANCELLED, AUTH::AUTHFAIL_CANCELLED },
                { ANDROIDAUTH_NOT_SIGNED_IN, AUTH::AUTHFAIL_NOT_AVAILABLE },
                { ANDROIDAUTH_NO_CREDENTIAL, AUTH::AUTHFAIL_NOT_AVAILABLE },
                { ANDROIDAUTH_DENIED, AUTH::AUTHFAIL_DENIED },
                { ANDROIDAUTH_NETWORK, AUTH::AUTHFAIL_NETWORK },
                { ANDROIDAUTH_ERROR, AUTH::AUTHFAIL_INTERNAL }
            };
            for ( UInt32 i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                fake.googleStatus = cases[ i ].status;
                ASSERT_TRUE( google.TrySilentLogin( future ) );
                ASSERT_TRUE( future.IsReady() );
                ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, i + 1, 1000 ) );
                ASSERT_TRUE( cases[ i ].reason == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            }
            ASSERT_TRUE( 0 == recorder.Count( CProviderRecorder::KIND_SUCCEEDED ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An id token for another app, another issuer, another login, an expired one or one that cannot be read does not make a login" )
        try
        {
            CFakeAndroid fake;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            ASSERT_TRUE( ConfigureGoogle( google ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &google );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            UInt32 failures = 0;

            #define EXPECT_REFUSED( setup ) \
                { setup; ASSERT_TRUE( google.TrySilentLogin( future ) ); ASSERT_TRUE( future.IsReady() ); ++failures; \
                  ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, failures, 1000 ) ); \
                  ASSERT_TRUE( AUTH::AUTHFAIL_INVALID_RESPONSE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason ); }

            EXPECT_REFUSED( fake.aud = "another-app.apps.example.invalid" )
            fake.aud = TestWebClientId;
            EXPECT_REFUSED( fake.iss = "https://evil.example.invalid" )
            fake.iss = "https://accounts.google.com";
            EXPECT_REFUSED( fake.nonceBehaviour = CFakeAndroid::NONCE_WRONG )
            EXPECT_REFUSED( fake.nonceBehaviour = CFakeAndroid::NONCE_OMIT )
            fake.nonceBehaviour = CFakeAndroid::NONCE_ECHO;
            EXPECT_REFUSED( fake.expiresInSeconds = -60 )
            fake.expiresInSeconds = 3600;
            EXPECT_REFUSED( fake.sub = "" )
            // a subject longer than an account reference can be
            EXPECT_REFUSED( fake.sub = Repeat( "1", 600 ) )
            fake.sub = "109876543210987654321";
            EXPECT_REFUSED( fake.rawIdToken = "this-is-no-jwt" )
            EXPECT_REFUSED( fake.rawIdToken = "aGVhZGVy.bm90IGpzb24.c2ln" )
            EXPECT_REFUSED( fake.rawIdToken = "aGVhZGVy..c2ln" )
            #undef EXPECT_REFUSED

            ASSERT_TRUE( 0 == recorder.Count( CProviderRecorder::KIND_SUCCEEDED ) );

            // the other issuer spelling of Google is fine
            fake.rawIdToken = CORE::CString();
            fake.iss = "accounts.google.com";
            ASSERT_TRUE( google.TrySilentLogin( future ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "google:109876543210987654321" );

            // a subject with characters that an account reference does not have is written the way the descriptors write it
            fake.sub = "has a space";
            ASSERT_TRUE( google.TrySilentLogin( future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "google:has%20a%20space" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The interactive login asks the Java side to ask the person, only one login at a time, and a Java side that does not start fails it" )
        try
        {
            CFakeAndroid fake;
            fake.delivery = CFakeAndroid::DELIVER_NEVER;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            ASSERT_TRUE( ConfigureGoogle( google ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &google );

            CORE::CFutureResult first( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( google.BeginLogin( AUTH::CAuthOptions(), first ) );
            ASSERT_TRUE( 1 == fake.googleCalls.size() && ANDROIDSIGNIN_INTERACTIVE == fake.googleCalls[ 0 ].mode );
            ASSERT_TRUE( first.HasAFuture() && !first.IsReady() );

            CORE::CFutureResult second( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( google.TrySilentLogin( second ) );
            CORE::CFutureResult third( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( google.BeginLogin( AUTH::CAuthOptions(), third ) );
            ASSERT_TRUE( third.IsReady() );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_IN_PROGRESS == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( 1 == fake.googleCalls.size() );

            // cancelling asks the Java side to stop
            ASSERT_TRUE( first.RequestCancellation() );
            ASSERT_TRUE( PumpUntilReady( first, 2000 ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 2, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_CANCELLED == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( 1 == fake.cancelled.size() && fake.cancelled[ 0 ] == fake.googleCalls[ 0 ].requestId );
            ASSERT_TRUE( 0 == CAndroidAuthBridge::Instance()->GetRequestCount() );

            // an answer that comes after that is nobody's
            SAndroidAuthResult late;
            late.requestId = fake.googleCalls[ 0 ].requestId;
            late.status = ANDROIDAUTH_OK;
            ASSERT_FALSE( CAndroidAuthBridge::Instance()->DeliverResult( late ) );

            // a Java side that does not start
            fake.googleStarts = false;
            CORE::CFutureResult refused( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( google.BeginLogin( AUTH::CAuthOptions(), refused ) );
            ASSERT_TRUE( refused.IsReady() );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 3, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( 0 == google.GetPendingRequestCount() && 0 == CAndroidAuthBridge::Instance()->GetRequestCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A login that the Java side does not answer times out, and a provider that goes with a request under way leaves nothing behind" )
        try
        {
            CFakeAndroid fake;
            fake.delivery = CFakeAndroid::DELIVER_NEVER;
            CScopedAndroid scope( fake );
            {
                CGoogleAuthProvider google;
                CORE::CDataNode node = MakeGoogleNode();
                node.FindChild( "Parameters" )->SetAttribute( "silentTimeoutInMs", (UInt32) 120 );
                ASSERT_TRUE( google.LoadConfig( node ) );
                CProviderRecorder recorder;
                recorder.SubscribeTo( &google );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( google.TrySilentLogin( future ) );
                ASSERT_TRUE( PumpUntilReady( future, 3000 ) );
                ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
                ASSERT_TRUE( AUTH::AUTHFAIL_TIMEOUT == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
                ASSERT_TRUE( 1 == fake.cancelled.size() );
                ASSERT_TRUE( 0 == CAndroidAuthBridge::Instance()->GetRequestCount() );
            }

            CORE::CFutureResult orphan( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            UInt32 orphanId = 0;
            {
                CGoogleAuthProvider google;
                ASSERT_TRUE( ConfigureGoogle( google ) );
                ASSERT_TRUE( google.TrySilentLogin( orphan ) );
                orphanId = fake.googleCalls.back().requestId;
                ASSERT_TRUE( 1 == CAndroidAuthBridge::Instance()->GetRequestCount() );
            }
            ASSERT_TRUE( orphan.IsReady() );
            ASSERT_TRUE( 0 == CAndroidAuthBridge::Instance()->GetRequestCount() );
            SAndroidAuthResult late;
            late.requestId = orphanId;
            ASSERT_FALSE( CAndroidAuthBridge::Instance()->DeliverResult( late ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An answer that is delivered while the provider is released is handled or dropped, never lost in a crash" )
        try
        {
            CFakeAndroid fake;
            fake.delivery = CFakeAndroid::DELIVER_THREAD;
            fake.deliveryDelayInMs = 5;
            CScopedAndroid scope( fake );
            for ( UInt32 round=0; round<20; ++round )
            {
                CGoogleAuthProvider google;
                ASSERT_TRUE( ConfigureGoogle( google ) );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( google.TrySilentLogin( future ) );
                // the answer comes about when the provider goes
                MT::ThreadDelay( round % 8 );
            }
            ASSERT_TRUE( 0 == CAndroidAuthBridge::Instance()->GetRequestCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Logout asks the Java side to forget the Google credential state" )
        try
        {
            CFakeAndroid fake;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            ASSERT_TRUE( ConfigureGoogle( google ) );
            ASSERT_TRUE( google.Logout( AUTH::CAccountRef( AUTH::CAuthProviderIds::Google, "1" ), AUTH::CSecretString() ) );
            ASSERT_TRUE( 1 == fake.googleSignOutCalls );
            CAmazonAuthProvider amazon;
            ASSERT_TRUE( amazon.Logout( AUTH::CAccountRef( AUTH::CAuthProviderIds::Amazon, "1" ), AUTH::CSecretString() ) );
            ASSERT_TRUE( 1 == fake.amazonSignOutCalls && 1 == fake.googleSignOutCalls );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The Google server role exchanges the code with the web client, and verifies the id token with the OIDC engine" )
        try
        {
            ASSERT_TRUE( JsonCodecAvailable() );
            CFakeHttpServer server;
            ASSERT_TRUE( server.Start() );
            CFakeAndroid fake;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            CORE::CDataNode node = MakeGoogleNode();
            CORE::CDataNode* parameters = node.FindChild( "Parameters" );
            parameters->SetAttribute( "tokenEndpoint", server.GetBaseUrl() + "/token" );
            parameters->SetAttribute( "allowInsecureHttp", true );
            ASSERT_TRUE( google.LoadConfig( node ) );

            CFakeOidcEngine engine;
            engine.acceptedToken = "id.token.from-google";
            engine.accountToGive = AUTH::CAccountRef( AUTH::CAuthProviderIds::Google, "109876543210987654321" );
            CScopedEngine scopedEngine( &engine );
            AUTH::CAuthProof codeProof( AUTH::CAuthProof::KindPlatformTicket, "4/0AbCdEf-one-time-code" );
            AUTH::CAuthResult result;

            server.SetResponse( 200, "{\"access_token\":\"ya29.A0\",\"expires_in\":3599,\"scope\":\"openid\",\"token_type\":\"Bearer\",\"id_token\":\"id.token.from-google\"}" );
            ASSERT_TRUE( google.Verify( codeProof, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "google:109876543210987654321" && result.providerName == "google" );

            // The request: a POST of a form with the code and the web client, once
            ASSERT_TRUE( 1 == server.GetRequestCount() );
            ASSERT_TRUE( server.GetLastRequestLine().StartsWith( "POST /token " ) );
            ASSERT_TRUE( server.GetLastRequestText().Lowercase().HasSubstr( "content-type: application/x-www-form-urlencoded" ) > 0 );
            CORE::CString body = server.GetLastRequestBody();
            ASSERT_TRUE( body.HasSubstr( "code=4%2F0AbCdEf-one-time-code" ) >= 0 );
            ASSERT_TRUE( body.HasSubstr( "client_id=" + CORE::CString( TestWebClientId ) ) >= 0 );
            ASSERT_TRUE( body.HasSubstr( "client_secret=" + CORE::CString( TestWebClientSecret ) ) >= 0 );
            ASSERT_TRUE( body.HasSubstr( "grant_type=authorization_code" ) >= 0 );
            // the secret is in the body only, not in the url
            ASSERT_TRUE( server.GetLastRequestLine().HasSubstr( "SECRET" ) < 0 && server.GetLastRequestLine().HasSubstr( "code" ) < 0 );
            // the engine got the id token of the exchange as an oidc id token for the google descriptor and the web client
            ASSERT_TRUE( 1 == engine.verifyCount && engine.lastDescriptorName == "google" );
            ASSERT_TRUE( engine.lastProofKind == AUTH::CAuthProof::KindOidcIdToken && engine.lastProofData == "id.token.from-google" );
            ASSERT_TRUE( engine.lastClientId == TestWebClientId );

            // the id token as the proof goes to the engine without an exchange
            AUTH::CAuthProof idProof( AUTH::CAuthProof::KindOidcIdToken, "id.token.from-google" );
            ASSERT_TRUE( google.Verify( idProof, result ) );
            ASSERT_TRUE( 1 == server.GetRequestCount() );
            ASSERT_FALSE( google.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "forged.id.token" ), result ) );

            // the exchange fails: the code is refused, no id token in the answer, not json, a status that is not 200
            server.SetResponse( 400, "{\"error\":\"invalid_grant\",\"error_description\":\"Bad Request\"}" );
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            server.SetResponse( 200, "{\"access_token\":\"ya29.A0\"}" );
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            server.SetResponse( 200, "<html>nope</html>", "text/html" );
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            server.SetResponse( 500, "{\"id_token\":\"id.token.from-google\"}" );
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            server.SetResponse( 200, "" );
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            server.SetLargeResponse( 70000 );
            ASSERT_FALSE( google.Verify( codeProof, result ) );

            // proofs that are neither
            UInt32 requests = server.GetRequestCount();
            ASSERT_FALSE( google.Verify( AUTH::CAuthProof(), result ) );
            ASSERT_FALSE( google.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOpenId2Assertion, "x" ), result ) );
            ASSERT_TRUE( requests == server.GetRequestCount() );

            // the engine verifies a token of another provider's account: not a Google account
            server.SetResponse( 200, "{\"id_token\":\"id.token.from-google\"}" );
            engine.accountToGive = AUTH::CAccountRef( AUTH::CAuthProviderIds::Epic, "abc" );
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            engine.accountToGive = AUTH::CAccountRef( AUTH::CAuthProviderIds::Google, "109876543210987654321" );

            // an engine that does not support it
            engine.supported = false;
            ASSERT_FALSE( google.IsVerifySupported() );
            ASSERT_FALSE( google.Verify( idProof, result ) );
            engine.supported = true;
            ASSERT_TRUE( google.IsVerifySupported() );

            // no engine at all: unsupported, and the exchange is not even tried first
            {
                CScopedEngine none( GUCEF_NULL );
                UInt32 before = server.GetRequestCount();
                ASSERT_FALSE( google.IsVerifySupported() );
                ASSERT_FALSE( google.Verify( idProof, result ) );
                ASSERT_TRUE( before == server.GetRequestCount() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The code is not sent without the web client secret, nor over a connection that is not allowed" )
        try
        {
            ASSERT_TRUE( JsonCodecAvailable() );
            CFakeHttpServer server;
            ASSERT_TRUE( server.Start() );
            server.SetResponse( 200, "{\"id_token\":\"id.token.from-google\"}" );
            CFakeOidcEngine engine;
            engine.acceptedToken = "id.token.from-google";
            engine.accountToGive = AUTH::CAccountRef( AUTH::CAuthProviderIds::Google, "1" );
            CScopedEngine scopedEngine( &engine );
            AUTH::CAuthProof codeProof( AUTH::CAuthProof::KindPlatformTicket, "code" );
            AUTH::CAuthResult result;

            CGoogleAuthProvider google;
            CORE::CDataNode node( "google", GUCEF_DATATYPE_OBJECT );
            node.SetAttribute( "clientId", TestWebClientId );
            CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            parameters->SetAttribute( "tokenEndpoint", server.GetBaseUrl() + "/token" );
            parameters->SetAttribute( "allowInsecureHttp", true );
            ASSERT_TRUE( google.LoadConfig( node ) );
            // no secret
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            ASSERT_TRUE( 0 == server.GetRequestCount() );

            // a secret, and http without permission
            node.SetAttribute( "clientSecret", TestWebClientSecret );
            parameters->SetAttribute( "allowInsecureHttp", false );
            ASSERT_TRUE( google.LoadConfig( node ) );
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            ASSERT_TRUE( 0 == server.GetRequestCount() );

            // user information in the url, another scheme
            parameters->SetAttribute( "allowInsecureHttp", true );
            parameters->SetAttribute( "tokenEndpoint", "http://user@127.0.0.1:" + CORE::UInt32ToString( server.GetPort() ) + "/token" );
            ASSERT_TRUE( google.LoadConfig( node ) );
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            parameters->SetAttribute( "tokenEndpoint", "file:///etc/passwd" );
            ASSERT_TRUE( google.LoadConfig( node ) );
            ASSERT_FALSE( google.Verify( codeProof, result ) );
            ASSERT_TRUE( 0 == server.GetRequestCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A silent Amazon login ends with the account of the Amazon user and the access token as proof" )
        try
        {
            CFakeAndroid fake;
            fake.delivery = CFakeAndroid::DELIVER_THREAD;
            CScopedAndroid scope( fake );
            CAmazonAuthProvider amazon;
            CProviderRecorder recorder;
            recorder.SubscribeTo( &amazon );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            ASSERT_TRUE( RunSilent( amazon, future ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "amazon:amzn1.account.AFAKEUSERID123" );
            ASSERT_TRUE( result.providerName == "amazon" && result.displayName == "Amy Amazon" );
            ASSERT_TRUE( result.proof.kind == AUTH::CAuthProof::KindPlatformTicket && result.proof.data.Reveal() == fake.amazonToken );
            ASSERT_TRUE( result.accessToken.Reveal() == fake.amazonToken );
            ASSERT_TRUE( 1 == fake.amazonCalls.size() && ANDROIDSIGNIN_SILENT == fake.amazonCalls[ 0 ].mode && fake.amazonCalls[ 0 ].scopes == "profile" );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_SUCCEEDED, 1, 1000 ) );

            // interactive, with scopes from the settings
            CORE::CDataNode node( "amazon", GUCEF_DATATYPE_OBJECT );
            node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT )->SetAttribute( "amazonScopes", "profile profile:user_id" );
            ASSERT_TRUE( amazon.LoadConfig( node ) );
            ASSERT_TRUE( amazon.BeginLogin( AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( PumpUntilReady( future, 3000 ) );
            ASSERT_TRUE( 2 == fake.amazonCalls.size() && ANDROIDSIGNIN_INTERACTIVE == fake.amazonCalls[ 1 ].mode && fake.amazonCalls[ 1 ].scopes == "profile profile:user_id" );

            // no user id, no token, a user id that is not usable
            fake.amazonUserId = "";
            ASSERT_TRUE( RunSilent( amazon, future ) );
            fake.amazonUserId = "amzn1.account.AFAKEUSERID123";
            fake.amazonToken = "";
            ASSERT_TRUE( RunSilent( amazon, future ) );
            fake.amazonToken = "Atza|token";
            fake.amazonUserId = Repeat( "a", 600 );
            ASSERT_TRUE( RunSilent( amazon, future ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 3, 1000 ) );
            for ( size_t i=0; i<recorder.records.size(); ++i )
                if ( CProviderRecorder::KIND_FAILED == recorder.records[ i ].kind )
                    ASSERT_TRUE( AUTH::AUTHFAIL_INVALID_RESPONSE == recorder.records[ i ].failure.reason );

            // statuses
            fake.amazonStatus = ANDROIDAUTH_NOT_SIGNED_IN;
            ASSERT_TRUE( RunSilent( amazon, future ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 4, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            fake.amazonStatus = ANDROIDAUTH_CANCELLED;
            ASSERT_TRUE( RunSilent( amazon, future ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 5, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_CANCELLED == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The Amazon server role asks the token info endpoint and requires the client id as audience" )
        try
        {
            ASSERT_TRUE( JsonCodecAvailable() );
            CFakeHttpServer server;
            ASSERT_TRUE( server.Start() );
            CAmazonAuthProvider amazon;
            CORE::CDataNode node( "amazon", GUCEF_DATATYPE_OBJECT );
            node.SetAttribute( "clientId", "amzn1.application-oa2-client.fake" );
            CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            parameters->SetAttribute( "tokenInfoEndpoint", server.GetBaseUrl() + "/auth/o2/tokeninfo" );
            parameters->SetAttribute( "allowInsecureHttp", true );
            parameters->SetAttribute( "httpTimeoutInMs", (UInt32) 3000 );
            ASSERT_TRUE( amazon.LoadConfig( node ) );

            AUTH::CAuthProof proof( AUTH::CAuthProof::KindPlatformTicket, "Atza|abc/def+ghi=" );
            AUTH::CAuthResult result;

            server.SetResponse( 200, "{\"iss\":\"https://www.amazon.com\",\"user_id\":\"amzn1.account.AVERIFIED\",\"aud\":\"amzn1.application-oa2-client.fake\",\"app_id\":\"amzn1.application.x\",\"exp\":3597,\"iat\":1311280970}" );
            ASSERT_TRUE( amazon.Verify( proof, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "amazon:amzn1.account.AVERIFIED" && result.providerName == "amazon" );
            ASSERT_TRUE( server.GetLastRequestLine().StartsWith( "GET /auth/o2/tokeninfo?access_token=Atza%7Cabc%2Fdef%2Bghi%3D " ) );

            // a token that was issued to another app of Amazon
            server.SetResponse( 200, "{\"user_id\":\"amzn1.account.AVERIFIED\",\"aud\":\"amzn1.application-oa2-client.OTHER\",\"exp\":3597}" );
            ASSERT_FALSE( amazon.Verify( proof, result ) );
            // no audience at all
            server.SetResponse( 200, "{\"user_id\":\"amzn1.account.AVERIFIED\",\"exp\":3597}" );
            ASSERT_FALSE( amazon.Verify( proof, result ) );
            // run out
            server.SetResponse( 200, "{\"user_id\":\"amzn1.account.AVERIFIED\",\"aud\":\"amzn1.application-oa2-client.fake\",\"exp\":0}" );
            ASSERT_FALSE( amazon.Verify( proof, result ) );
            // no user id, one that is not usable
            server.SetResponse( 200, "{\"aud\":\"amzn1.application-oa2-client.fake\",\"exp\":100}" );
            ASSERT_FALSE( amazon.Verify( proof, result ) );
            server.SetResponse( 200, "{\"user_id\":\"" + Repeat( "a", 600 ) + "\",\"aud\":\"amzn1.application-oa2-client.fake\",\"exp\":100}" );
            ASSERT_FALSE( amazon.Verify( proof, result ) );
            // characters that an account reference does not have are written the way the descriptors write them
            server.SetResponse( 200, "{\"user_id\":\"a b\",\"aud\":\"amzn1.application-oa2-client.fake\",\"exp\":100}" );
            ASSERT_TRUE( amazon.Verify( proof, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "amazon:a%20b" );
            // Amazon refuses the token
            server.SetResponse( 400, "{\"error\":\"invalid_token\"}" );
            ASSERT_FALSE( amazon.Verify( proof, result ) );
            server.SetResponse( 200, "not json", "text/plain" );
            ASSERT_FALSE( amazon.Verify( proof, result ) );

            // proofs that are not access tokens do not reach the network
            UInt32 requests = server.GetRequestCount();
            ASSERT_FALSE( amazon.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "x.y.z" ), result ) );
            ASSERT_FALSE( amazon.Verify( AUTH::CAuthProof(), result ) );
            ASSERT_TRUE( requests == server.GetRequestCount() );

            // without the client id there is no audience to compare with, and http is not allowed unless the settings say so
            CAmazonAuthProvider noClient;
            CORE::CDataNode node2( "amazon", GUCEF_DATATYPE_OBJECT );
            node2.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT )->SetAttribute( "tokenInfoEndpoint", server.GetBaseUrl() + "/x" );
            ASSERT_TRUE( noClient.LoadConfig( node2 ) );
            ASSERT_FALSE( noClient.Verify( proof, result ) );
            node2.SetAttribute( "clientId", "amzn1.application-oa2-client.fake" );
            ASSERT_TRUE( noClient.LoadConfig( node2 ) );
            ASSERT_FALSE( noClient.Verify( proof, result ) );
            ASSERT_TRUE( requests == server.GetRequestCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Settings: defaults, round trip with the secret, the form CAuthManager hands over and what is not well formed" )
        try
        {
            CAndroidAuthSettings defaults;
            ASSERT_TRUE( defaults.useCredentialManager && defaults.proofKind == "auth-code" && !defaults.allowInsecureHttp );
            ASSERT_TRUE( defaults.tokenEndpoint == "https://oauth2.googleapis.com/token" );
            ASSERT_TRUE( defaults.tokenInfoEndpoint == "https://api.amazon.com/auth/o2/tokeninfo" );
            ASSERT_TRUE( defaults.amazonScopes == "profile" && 10000 == defaults.httpTimeoutInMs && 15000 == defaults.silentTimeoutInMs );
            ASSERT_TRUE( defaults.IsWellFormed() );

            CAndroidAuthSettings filled;
            filled.clientId = "web";
            filled.clientSecret.Set( TestWebClientSecret );
            filled.useCredentialManager = false;
            filled.proofKind = "id-token";
            filled.tokenEndpoint = "https://token.example.invalid/t";
            filled.tokenInfoEndpoint = "https://info.example.invalid/i";
            filled.amazonScopes = "profile:user_id";
            filled.allowInsecureHttp = true;
            filled.httpTimeoutInMs = 1111;
            filled.silentTimeoutInMs = 2222;
            CORE::CDataNode saved( "google", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( filled.SaveConfig( saved ) );
            CAndroidAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( saved ) );
            ASSERT_TRUE( loaded == filled && loaded.clientSecret.Reveal() == TestWebClientSecret );

            ASSERT_TRUE( JsonCodecAvailable() );
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess output( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &saved, &output ) );
            CORE::CDynamicBufferAccess input( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &input ) );
            CAndroidAuthSettings fromJson;
            ASSERT_TRUE( fromJson.LoadConfig( parsed ) );
            ASSERT_TRUE( fromJson == filled );

            // the manager: client id and secret from CAuthProviderSettings, the plugin's keys in Parameters
            AUTH::CAuthSettings authSettings;
            AUTH::CAuthProviderSettings providerSettings;
            providerSettings.clientId = "from-manager";
            providerSettings.clientSecret.Set( TestWebClientSecret );
            providerSettings.parameters[ "proofKind" ] = "id-token";
            authSettings.SetProviderSettings( "google", providerSettings );
            CFakeAndroid fake;
            CScopedAndroid scope( fake );
            TGoogleAuthProviderFactory factory;
            AUTH::CAuthManager manager;
            manager.SetProviderFactory( GUCEF_NULL );
            AUTH::CIAuthProviderPtr providerPtr = factory.Create();
            ASSERT_TRUE( manager.AddProvider( providerPtr ) );
            ASSERT_FALSE( manager.IsProviderOffered( "google" ) );      // no client id yet
            manager.SetSettings( authSettings );
            CGoogleAuthProvider* managed = static_cast< CGoogleAuthProvider* >( manager.GetProvider( "google" ) );
            ASSERT_TRUE( GUCEF_NULL != managed );
            ASSERT_TRUE( managed->GetSettings().clientId == "from-manager" && managed->GetSettings().proofKind == "id-token" );
            ASSERT_TRUE( managed->GetSettings().clientSecret.Reveal() == TestWebClientSecret );
            ASSERT_TRUE( manager.IsProviderOffered( "google" ) );
            manager.RemoveProvider( "google" );

            // not well formed
            CGoogleAuthProvider google;
            ASSERT_TRUE( ConfigureGoogle( google ) );
            ASSERT_FALSE( google.LoadConfig( MakeGoogleNode( true, "pigeon" ) ) );
            CORE::CDataNode noScopes = MakeGoogleNode();
            noScopes.FindChild( "Parameters" )->SetAttribute( "amazonScopes", "x" );
            ASSERT_TRUE( google.LoadConfig( noScopes ) );
            ASSERT_TRUE( google.GetSettings().proofKind == "auth-code" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The auth code, the id token, the access token and the web client secret are never logged" )
        try
        {
            ASSERT_TRUE( JsonCodecAvailable() );
            CFakeHttpServer server;
            ASSERT_TRUE( server.Start() );
            server.SetResponse( 200, "{\"id_token\":\"id.token.from-google\"}" );
            CLogCapture capture;

            CFakeAndroid fake;
            CScopedAndroid scope( fake );
            CGoogleAuthProvider google;
            CORE::CDataNode node = MakeGoogleNode();
            CORE::CDataNode* parameters = node.FindChild( "Parameters" );
            parameters->SetAttribute( "tokenEndpoint", server.GetBaseUrl() + "/token" );
            parameters->SetAttribute( "allowInsecureHttp", true );
            ASSERT_TRUE( google.LoadConfig( node ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &google );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( RunSilent( google, future ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );

            CFakeOidcEngine engine;
            engine.acceptedToken = "id.token.from-google";
            engine.accountToGive = result.accountRef;
            CScopedEngine scopedEngine( &engine );
            AUTH::CAuthResult verified;
            ASSERT_TRUE( google.Verify( result.proof, verified ) );
            server.SetResponse( 400, "{\"error\":\"invalid_grant\"}" );
            ASSERT_FALSE( google.Verify( result.proof, verified ) );
            fake.googleStatus = ANDROIDAUTH_ERROR;
            ASSERT_TRUE( RunSilent( google, future ) );
            fake.googleStatus = ANDROIDAUTH_OK;
            fake.aud = "another";
            ASSERT_TRUE( RunSilent( google, future ) );
            fake.aud = TestWebClientId;

            CAmazonAuthProvider amazon;
            ASSERT_TRUE( RunSilent( amazon, future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            amazon.Logout( result.accountRef, AUTH::CSecretString() );
            PumpFor( 20 );

            CORE::CString logged = capture.Text();
            ASSERT_TRUE( logged.HasSubstr( "AuthProvider" ) >= 0 );
            ASSERT_TRUE( logged.HasSubstr( "SERVER-AUTH-CODE" ) < 0 && logged.HasSubstr( "4/0AbCdEf" ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "AMAZON-ACCESS-TOKEN" ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "WEB-CLIENT-SECRET" ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "id.token.from-google" ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "c2lnbmF0dXJlLURPLU5PVC1MT0c" ) < 0 );
            for ( size_t i=0; i<recorder.records.size(); ++i )
            {
                ASSERT_TRUE( recorder.records[ i ].failure.message.HasSubstr( "SERVER-AUTH-CODE" ) < 0 );
                ASSERT_TRUE( recorder.records[ i ].failure.message.HasSubstr( "c2lnbmF0dXJlLURPLU5PVC1MT0c" ) < 0 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The bridge keeps the requests, the table of the Java side and the receiver of redirects, and is safe against answers it does not know" )
        try
        {
            CAndroidAuthBridge* bridge = CAndroidAuthBridge::Instance();
            ASSERT_FALSE( bridge->HasShim() );

            // answers for nobody
            SAndroidAuthResult stray;
            stray.requestId = 424242;
            ASSERT_FALSE( bridge->DeliverResult( stray ) );
            ASSERT_FALSE( bridge->DeliverResult( SAndroidAuthResult() ) );

            // redirects for nobody
            ASSERT_FALSE( bridge->DeliverRedirect( "com.example.game:/cb?code=1&state=2" ) );

            CRecordingReceiver receiver;
            ASSERT_TRUE( GUCEF_NULL == bridge->SetRedirectReceiver( &receiver ) );
            ASSERT_TRUE( bridge->DeliverRedirect( "com.example.game:/cb?code=1&state=2" ) );
            ASSERT_TRUE( 1 == receiver.urls.size() && receiver.urls[ 0 ] == "com.example.game:/cb?code=1&state=2" );
            receiver.take = false;
            ASSERT_FALSE( bridge->DeliverRedirect( "x:/y" ) );
            CRecordingReceiver second;
            ASSERT_TRUE( &receiver == bridge->SetRedirectReceiver( &second ) );
            ASSERT_TRUE( &second == bridge->SetRedirectReceiver( GUCEF_NULL ) );
            ASSERT_FALSE( bridge->DeliverRedirect( "x:/y" ) );

            // the receiver that is wired to the engine
            CEngineRedirectReceiver toEngine;
            CFakeOidcEngine engine;
            ASSERT_FALSE( toEngine.OnAuthRedirect( "com.example.game:/cb?code=1" ) == ( GUCEF_NULL != AUTH::CAuthGlobal::Instance()->GetFlowEngine() && false ) && false );
            {
                CScopedEngine none( GUCEF_NULL );
                ASSERT_FALSE( toEngine.OnAuthRedirect( "com.example.game:/cb?code=1" ) );
            }
            {
                CScopedEngine scoped( &engine );
                ASSERT_TRUE( toEngine.OnAuthRedirect( "com.example.game:/cb?code=1&state=2" ) );
                ASSERT_TRUE( 1 == engine.redirectCount && engine.lastRedirectUrl == "com.example.game:/cb?code=1&state=2" );
                engine.acceptRedirects = false;
                ASSERT_FALSE( toEngine.OnAuthRedirect( "com.example.game:/cb?code=1&state=other" ) );
            }

            // the table
            CFakeAndroid fake;
            {
                CScopedAndroid scope( fake );
                ASSERT_TRUE( bridge->HasShim() );
                ASSERT_TRUE( GUCEF_NULL != bridge->GetShim().googleSignIn );
            }
            ASSERT_FALSE( bridge->HasShim() );

            // ids are never 0 and not used twice at a time
            CRecordingReceiver unused;
            class CSink : public IAndroidAuthResultSink
            {
                public:
                GUCEF::UInt32 count;
                CSink( void ) : IAndroidAuthResultSink(), count( 0 ) {}
                virtual void OnAndroidAuthResult( const SAndroidAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE { ++count; }
            };
            CSink sink;
            UInt32 a = bridge->BeginRequest( &sink );
            UInt32 b = bridge->BeginRequest( &sink );
            ASSERT_TRUE( 0 != a && 0 != b && a != b && 2 == bridge->GetRequestCount() );
            SAndroidAuthResult answer;
            answer.requestId = a;
            ASSERT_TRUE( bridge->DeliverResult( answer ) );
            ASSERT_FALSE( bridge->DeliverResult( answer ) );
            ASSERT_TRUE( 1 == sink.count && 1 == bridge->GetRequestCount() );
            bridge->ReleaseSink( &sink );
            ASSERT_TRUE( 0 == bridge->GetRequestCount() );
            answer.requestId = b;
            ASSERT_FALSE( bridge->DeliverResult( answer ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The browser launcher opens the url of the engine in a Custom Tab, and only urls that the core lets through" )
        try
        {
            CAndroidAuthBridge* bridge = CAndroidAuthBridge::Instance();
            CRecordingLauncher before;
            CORE::CIBrowserLauncherHandler* original = CORE::SetBrowserLauncherHandler( &before );

            CFakeAndroid fake;
            {
                CScopedAndroid scope( fake );
                ASSERT_TRUE( bridge->InstallBrowserLauncher() );
                ASSERT_FALSE( bridge->InstallBrowserLauncher() );
                ASSERT_TRUE( CORE::GetBrowserLauncherHandler() != &before );

                ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://accounts.example.invalid/authorize?state=abc&nonce=def" ) );
                ASSERT_TRUE( 1 == fake.openedUrls.size() && fake.openedUrls[ 0 ] == "https://accounts.example.invalid/authorize?state=abc&nonce=def" );
                ASSERT_TRUE( 0 == before.urls.size() );

                // the core refuses these before they get to the launcher
                ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "javascript:alert(1)" ) );
                ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://user@host.example.invalid/" ) );
                ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://host.example.invalid/a b" ) );
                ASSERT_TRUE( 1 == fake.openedUrls.size() );

                // the Java side cannot open it
                fake.openTabResult = false;
                ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://accounts.example.invalid/x" ) );

                bridge->RemoveBrowserLauncher();
                ASSERT_TRUE( CORE::GetBrowserLauncherHandler() == &before );

                // someone else's launcher is not taken away
                ASSERT_TRUE( bridge->InstallBrowserLauncher() );
                CRecordingLauncher later;
                CORE::SetBrowserLauncherHandler( &later );
                bridge->RemoveBrowserLauncher();
                ASSERT_TRUE( CORE::GetBrowserLauncherHandler() == &later );
                CORE::SetBrowserLauncherHandler( &before );
            }

            // with no table the launcher says no
            ASSERT_TRUE( bridge->InstallBrowserLauncher() );
            ASSERT_FALSE( CORE::OpenUrlInSystemBrowser( "https://accounts.example.invalid/x" ) );
            bridge->RemoveBrowserLauncher();
            CORE::SetBrowserLauncherHandler( original );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Registration: native providers take the place of the built in ones only where their platform is, and give it back" )
        try
        {
            // a factory of its own: both register
            AUTH::CAuthProviderFactory own;
            ASSERT_TRUE( ( ANDROIDPROVIDER_GOOGLE | ANDROIDPROVIDER_AMAZON ) == RegisterAndroidAuthProviders( own, 0 ) );
            ASSERT_TRUE( own.IsConstructible( "google" ) && own.IsConstructible( "amazon" ) );
            UnregisterAndroidAuthProviders( own, ANDROIDPROVIDER_GOOGLE | ANDROIDPROVIDER_AMAZON, 0 );
            ASSERT_FALSE( own.IsConstructible( "google" ) || own.IsConstructible( "amazon" ) );

            // the factory of the process has the built in ones: they stay unless the platform is there
            AUTH::CAuthProviderFactory& global = AUTH::CAuthGlobal::Instance()->GetProviderFactory();
            ASSERT_TRUE( global.IsConstructible( "google" ) && global.IsConstructible( "amazon" ) );
            ASSERT_TRUE( ANDROIDPROVIDER_NONE == RegisterAndroidAuthProviders( global, 0 ) );
            AUTH::CIAuthProviderPtr builtIn = global.Create( "google" );
            ASSERT_TRUE( builtIn->GetClassTypeName() == "GUCEF::AUTH::CDescriptorAuthProvider" );
            builtIn.Unlink();

            // only google replaces
            ASSERT_TRUE( ANDROIDPROVIDER_GOOGLE == RegisterAndroidAuthProviders( global, ANDROIDPROVIDER_GOOGLE ) );
            AUTH::CIAuthProviderPtr google = global.Create( "google" );
            AUTH::CIAuthProviderPtr amazon = global.Create( "amazon" );
            ASSERT_TRUE( google->GetClassTypeName() == "GUCEF::AUTHPLUGIN::ANDROIDAUTH::CGoogleAuthProvider" );
            ASSERT_TRUE( amazon->GetClassTypeName() == "GUCEF::AUTH::CDescriptorAuthProvider" );
            google.Unlink();
            amazon.Unlink();
            UnregisterAndroidAuthProviders( global, ANDROIDPROVIDER_GOOGLE, ANDROIDPROVIDER_GOOGLE );
            AUTH::CIAuthProviderPtr restored = global.Create( "google" );
            ASSERT_FALSE( restored.IsNULL() );
            ASSERT_TRUE( restored->GetClassTypeName() == "GUCEF::AUTH::CDescriptorAuthProvider" );
            restored.Unlink();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
