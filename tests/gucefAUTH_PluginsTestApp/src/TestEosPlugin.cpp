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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H
#include "authpluginEOS_CEosAuthProvider.h"
#define GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_EOS_H
#include "authpluginEOS.h"
#define GUCEF_AUTHPLUGIN_EOS_H
#endif /* GUCEF_AUTHPLUGIN_EOS_H ? */

#include "TestPluginsCommon.h"
#include "TestFakeEos.h"
#include "TestFakeEngine.h"
#include "TestEosPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::AUTHPLUGIN::EOS;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const TestClientSecret = "EOS-CLIENT-SECRET-5d8e0c47-DO-NOT-LOG";
static const char* const TestEncryptionKey = "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff";

/** The node CAuthManager hands a provider: clientId and clientSecret as attributes, the rest in Parameters */
static CORE::CDataNode
MakeEosNode( UInt32 tickIntervalInMs = 5 )
{GUCEF_TRACE;

    CORE::CDataNode node( "epic", GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "clientId", "xyz-client-id" );
    node.SetAttribute( "clientSecret", TestClientSecret );
    CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    parameters->SetAttribute( "productId", "product-1" );
    parameters->SetAttribute( "sandboxId", "sandbox-1" );
    parameters->SetAttribute( "deploymentId", "deployment-1" );
    parameters->SetAttribute( "encryptionKey", TestEncryptionKey );
    parameters->SetAttribute( "tickIntervalInMs", tickIntervalInMs );
    return node;
}

/*-------------------------------------------------------------------------*/

static bool
Configure( CEosAuthProvider& provider )
{GUCEF_TRACE;

    return provider.LoadConfig( MakeEosNode() );
}

/*-------------------------------------------------------------------------*/

/** Starts a silent login and runs it to its end; the future is given back */
static bool
LoginSilent( CEosAuthProvider& provider, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return provider.TrySilentLogin( future ) && PumpUntilReady( future, 2000 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEosPluginTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING EOS plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "EosPlugin" );

    GUCEF_TESTFW_TESTCASE( "Test: The provider is epic with the capabilities of the SDK and the account namespace of the built in epic descriptor" )
        try
        {
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( provider.GetProviderName() == "epic" );
            UInt32 expected = AUTH::AUTHCAP_SILENT | AUTH::AUTHCAP_BROWSER | AUTH::AUTHCAP_DEVICE_CODE | AUTH::AUTHCAP_SERVER_VERIFIABLE | AUTH::AUTHCAP_REQUIRES_CLIENT_ID;
            ASSERT_TRUE( expected == provider.GetCapabilities() );
            ASSERT_TRUE( provider.GetClassTypeName() == "GUCEF::AUTHPLUGIN::EOS::CEosAuthProvider" );

            AUTH::CAuthProviderDescriptor descriptor;
            ASSERT_TRUE( AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( AUTH::CAuthProviderRegistry::EpicName, descriptor ) );
            CORE::CString::StringMap claims;
            claims[ "sub" ] = "0123456789abcdef0123456789abcdef";
            AUTH::CAccountRef builtIn;
            ASSERT_TRUE( descriptor.MakeAccountRef( claims, CORE::CString::StringMap(), builtIn ) );
            ASSERT_TRUE( builtIn.ToString() == "epic:0123456789abcdef0123456789abcdef" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Available when the SDK is there and the product and client are configured" )
        try
        {
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_FALSE( provider.IsAvailable() );
            ASSERT_TRUE( Configure( provider ) );
            ASSERT_TRUE( provider.IsAvailable() );
            // IsAvailable does not create the platform
            ASSERT_TRUE( 0 == eos.createPlatformCalls );

            eos.sdkAvailable = false;
            ASSERT_FALSE( provider.IsAvailable() );
            eos.sdkAvailable = true;

            // every id and both client credentials are needed
            const char* missing[] = { "productId", "sandboxId", "deploymentId" };
            for ( size_t i=0; i<3; ++i )
            {
                CEosAuthProvider partial( eos.MakeShim() );
                CORE::CDataNode node = MakeEosNode();
                node.FindChild( "Parameters" )->DelAttribute( missing[ i ] );
                ASSERT_TRUE( partial.LoadConfig( node ) );
                ASSERT_FALSE( partial.IsAvailable() );
            }
            CEosAuthProvider noSecret( eos.MakeShim() );
            CORE::CDataNode node = MakeEosNode();
            node.DelAttribute( "clientSecret" );
            ASSERT_TRUE( noSecret.LoadConfig( node ) );
            ASSERT_FALSE( noSecret.IsAvailable() );
            CEosAuthProvider noClient( eos.MakeShim() );
            CORE::CDataNode node2 = MakeEosNode();
            node2.DelAttribute( "clientId" );
            ASSERT_TRUE( noClient.LoadConfig( node2 ) );
            ASSERT_FALSE( noClient.IsAvailable() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A silent login creates the platform, logs in with the persistent auth, and ends with the Epic account, the id token as proof and the product user id" )
        try
        {
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            ASSERT_TRUE( future.HasAFuture() );
            // The call did not wait for the SDK
            ASSERT_FALSE( future.IsReady() );
            ASSERT_TRUE( 1 == provider.GetPendingLoginCount() );
            ASSERT_TRUE( 1 == eos.createPlatformCalls );
            ASSERT_TRUE( 1 == eos.loginTypes.size() && EOSLOGIN_PERSISTENT == eos.loginTypes[ 0 ] );

            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "epic:0123456789abcdef0123456789abcdef" );
            ASSERT_TRUE( result.providerName == "epic" );
            ASSERT_TRUE( result.proof.kind == AUTH::CAuthProof::KindOidcIdToken );
            ASSERT_TRUE( result.proof.data.Reveal() == eos.idToken );
            ASSERT_TRUE( result.idToken.Reveal() == eos.idToken );

            // The product user id is per product, it is not the subject of the account
            ASSERT_TRUE( provider.GetProductUserId( result.accountRef ) == "00aa11bb22cc33dd44ee55ff66778899" );
            ASSERT_TRUE( result.accountRef.GetSubject().HasSubstr( "00aa11" ) < 0 );
            ASSERT_TRUE( 1 == eos.connectTokens.size() && eos.connectTokens[ 0 ] == eos.idToken );

            // What the SDK was given
            ASSERT_TRUE( eos.configProductId == "product-1" && eos.configSandboxId == "sandbox-1" && eos.configDeploymentId == "deployment-1" );
            ASSERT_TRUE( eos.configClientId == "xyz-client-id" && eos.configClientSecret == TestClientSecret && eos.configEncryptionKey == TestEncryptionKey );
            ASSERT_TRUE( eos.configProductName == "GUCEF" );

            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_SUCCEEDED, 1, 1000 ) );
            ASSERT_TRUE( recorder.records[ 0 ].threadId == MT::GetCurrentTaskID() );
            ASSERT_TRUE( 0 == provider.GetPendingLoginCount() );

            // The timer stopped with the login: the SDK is not ticked for nothing
            UInt32 ticks = eos.tickCalls;
            PumpFor( 60 );
            ASSERT_TRUE( ticks == eos.tickCalls );

            // The platform is made once
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( 1 == eos.createPlatformCalls );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The product user: made when the account has none, optional, and never a reason to fail the login of the account" )
        try
        {
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            AUTH::CAccountRef account( AUTH::CAuthProviderIds::Epic, "0123456789abcdef0123456789abcdef" );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            // The account has no product user yet: it is made
            eos.connectOutcome = EOSOUTCOME_NEEDS_USER_CREATION;
            ASSERT_TRUE( LoginSilent( provider, future ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( 1 == eos.createUserCalls );
            ASSERT_TRUE( provider.GetProductUserId( account ) == "99887766ffee55dd44cc33bb22aa1100" );

            // the application has a product user already (a login with another provider through EOS Connect): the account is linked to it
            provider.Logout( account, AUTH::CSecretString() );
            eos.createUserCalls = 0;
            provider.SetLinkTargetProductUserId( "1111aaaa2222bbbb3333cccc4444dddd" );
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( 0 == eos.createUserCalls && 1 == eos.linkTargets.size() && eos.linkTargets[ 0 ] == "1111aaaa2222bbbb3333cccc4444dddd" );
            ASSERT_TRUE( provider.GetProductUserId( account ) == "1111aaaa2222bbbb3333cccc4444dddd" );
            // for that login only: the next one creates
            provider.Logout( account, AUTH::CSecretString() );
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( 1 == eos.linkTargets.size() && 1 == eos.createUserCalls );
            provider.Logout( account, AUTH::CSecretString() );
            eos.createUserCalls = 0;

            // linking fails, or does not start: the login stands, a link that does not start falls back to making the user
            eos.linkOutcome = EOSOUTCOME_FAILED;
            provider.SetLinkTargetProductUserId( "1111aaaa2222bbbb3333cccc4444dddd" );
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( provider.GetProductUserId( account ).IsNULLOrEmpty() && 0 == eos.createUserCalls );
            eos.linkOutcome = EOSOUTCOME_SUCCESS;
            eos.linkStarts = false;
            provider.SetLinkTargetProductUserId( "1111aaaa2222bbbb3333cccc4444dddd" );
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( 1 == eos.createUserCalls && provider.GetProductUserId( account ) == "99887766ffee55dd44cc33bb22aa1100" );
            eos.linkStarts = true;
            provider.SetLinkTargetProductUserId( CORE::CString::Empty );
            provider.Logout( account, AUTH::CSecretString() );
            eos.createUserCalls = 0;

            // making it fails: the login stands, without the id
            provider.Logout( account, AUTH::CSecretString() );
            ASSERT_TRUE( provider.GetProductUserId( account ).IsNULLOrEmpty() );
            eos.createOutcome = EOSOUTCOME_FAILED;
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( provider.GetProductUserId( account ).IsNULLOrEmpty() );

            // the settings say it is not made
            CORE::CDataNode node = MakeEosNode();
            node.FindChild( "Parameters" )->SetAttribute( "createConnectUser", false );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            eos.createUserCalls = 0;
            eos.createOutcome = EOSOUTCOME_SUCCESS;
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( 0 == eos.createUserCalls && provider.GetProductUserId( account ).IsNULLOrEmpty() );

            // connect fails for other reasons, or does not start
            eos.connectOutcome = EOSOUTCOME_NO_CONNECTION;
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            eos.connectOutcome = EOSOUTCOME_SUCCESS;
            eos.connectStarts = false;
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.accountRef == account );

            // no connect at all
            eos.connectStarts = true;
            eos.connectTokens.clear();
            node.FindChild( "Parameters" )->SetAttribute( "useConnect", false );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( 0 == eos.connectTokens.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: What the SDK answers to a login is told as a reason: no stored login, refused, offline, cancelled, no time" )
        try
        {
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            struct SCase { EEosOutcome outcome; AUTH::TAuthFailureReason reason; };
            const SCase cases[] =
            {
                { EOSOUTCOME_INVALID_CREDENTIALS, AUTH::AUTHFAIL_INVALID_GRANT },
                { EOSOUTCOME_DENIED, AUTH::AUTHFAIL_DENIED },
                { EOSOUTCOME_NO_CONNECTION, AUTH::AUTHFAIL_NETWORK },
                { EOSOUTCOME_CANCELLED, AUTH::AUTHFAIL_CANCELLED },
                { EOSOUTCOME_TIMEOUT, AUTH::AUTHFAIL_TIMEOUT },
                { EOSOUTCOME_FAILED, AUTH::AUTHFAIL_INTERNAL }
            };
            for ( UInt32 i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                eos.authOutcome = cases[ i ].outcome;
                eos.authSdkCode = 17 + (Int32) i;
                ASSERT_TRUE( LoginSilent( provider, future ) );
                AUTH::CAuthResult none;
                ASSERT_FALSE( AUTH::CAuthResult::TryGetFromFuture( future, none ) );
                ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, i + 1, 1000 ) );
                ASSERT_TRUE( cases[ i ].reason == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
                // The code of the SDK is in the message, nothing else of the exchange
                ASSERT_TRUE( recorder.Last( CProviderRecorder::KIND_FAILED )->failure.message.HasSubstr( CORE::Int32ToString( eos.authSdkCode ) ) >= 0 );
            }
            ASSERT_TRUE( 0 == recorder.Count( CProviderRecorder::KIND_SUCCEEDED ) );
            ASSERT_TRUE( 0 == provider.GetPendingLoginCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An account id or an id token that is not usable ends the login as an invalid answer" )
        try
        {
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            eos.idTokenAvailable = false;
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_INVALID_RESPONSE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );

            eos.idTokenAvailable = true;
            eos.epicAccountId = "not a valid id!";
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 2, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_INVALID_RESPONSE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( 0 == eos.connectTokens.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The interactive logins: the account portal opens in the browser of the SDK, the device code is shown to the person" )
        try
        {
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            AUTH::CAuthOptions portal;
            portal.preferBrowser = true;
            ASSERT_TRUE( provider.BeginLogin( portal, future ) );
            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( 1 == eos.loginTypes.size() && EOSLOGIN_ACCOUNT_PORTAL == eos.loginTypes[ 0 ] );
            // The SDK opens the browser itself: nothing for the application to do
            ASSERT_TRUE( 0 == recorder.Count( CProviderRecorder::KIND_ACTION ) );

            AUTH::CAuthOptions device;
            device.preferDeviceCode = true;
            ASSERT_TRUE( provider.BeginLogin( device, future ) );
            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( 2 == eos.loginTypes.size() && EOSLOGIN_DEVICE_CODE == eos.loginTypes[ 1 ] );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_ACTION, 1, 1000 ) );
            const CProviderRecorder::SRecord* action = recorder.Last( CProviderRecorder::KIND_ACTION );
            ASSERT_TRUE( AUTH::AUTHACTION_SHOW_DEVICE_CODE == action->action.kind );
            ASSERT_TRUE( action->action.providerName == "epic" );
            ASSERT_TRUE( action->action.userCode == "ABCD-1234" );
            ASSERT_TRUE( action->action.url == "https://epicgames.example.invalid/activate" );
            ASSERT_TRUE( action->action.urlWithCode == "https://epicgames.example.invalid/activate?userCode=ABCD-1234" );
            ASSERT_TRUE( action->action.expiresAtMs > AUTH::GetEpochTimeInMs() + 500000 && action->action.expiresAtMs < AUTH::GetEpochTimeInMs() + 700000 );
            // The action came before the end of the login
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_SUCCEEDED, 2, 1000 ) );
            ASSERT_TRUE( recorder.records.size() >= 3 );
            size_t actionIndex = 0, endIndex = 0;
            for ( size_t i=0; i<recorder.records.size(); ++i )
            {
                if ( CProviderRecorder::KIND_ACTION == recorder.records[ i ].kind )
                    actionIndex = i;
                if ( CProviderRecorder::KIND_SUCCEEDED == recorder.records[ i ].kind )
                    endIndex = i;
            }
            ASSERT_TRUE( actionIndex < endIndex );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: One login at a time, a login that does not end times out, and a cancelled one ends as cancelled" )
        try
        {
            CFakeEos eos;
            eos.authNeverAnswers = true;
            CEosAuthProvider provider( eos.MakeShim() );
            CORE::CDataNode node = MakeEosNode();
            node.FindChild( "Parameters" )->SetAttribute( "silentTimeoutInMs", (UInt32) 80 );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );

            CORE::CFutureResult first( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( first ) );
            // a second silent one does not start, an interactive one is told why
            CORE::CFutureResult second( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( provider.TrySilentLogin( second ) );
            ASSERT_FALSE( second.HasAFuture() );
            CORE::CFutureResult third( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.BeginLogin( AUTH::CAuthOptions(), third ) );
            ASSERT_TRUE( third.IsReady() );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_IN_PROGRESS == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );

            // the first runs out of time
            ASSERT_TRUE( PumpUntilReady( first, 2000 ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 2, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_TIMEOUT == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( 0 == provider.GetPendingLoginCount() );

            // cancel
            CORE::CFutureResult fourth( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( fourth ) );
            ASSERT_TRUE( fourth.RequestCancellation() );
            ASSERT_TRUE( PumpUntilReady( fourth, 2000 ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 3, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_CANCELLED == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );

            // The SDK finishes a login nobody waits for any more: dropped
            eos.authNeverAnswers = false;
            eos.ticksUntilAnswer = 1;
            CORE::CFutureResult fifth( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( fifth ) );
            ASSERT_TRUE( fifth.RequestCancellation() );
            PumpFor( 60 );
            ASSERT_TRUE( 0 == recorder.Count( CProviderRecorder::KIND_SUCCEEDED ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The platform that cannot be made, a login that the SDK does not start, a provider that goes with a login under way" )
        try
        {
            CFakeEos eos;
            eos.platformOk = false;
            eos.platformError = "fake: wrong sandbox";
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );

            CORE::CFutureResult silent( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( provider.TrySilentLogin( silent ) );
            CORE::CFutureResult interactive( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.BeginLogin( AUTH::CAuthOptions(), interactive ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( recorder.Last( CProviderRecorder::KIND_FAILED )->failure.message.HasSubstr( "fake: wrong sandbox" ) >= 0 );
            ASSERT_TRUE( 2 == eos.createPlatformCalls );

            // fixed in the meantime
            eos.platformOk = true;
            eos.loginStarts = false;
            CORE::CFutureResult refused( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.BeginLogin( AUTH::CAuthOptions(), refused ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 2, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_INTERNAL == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( 0 == provider.GetPendingLoginCount() );

            // a provider that goes while a login is under way: the future ends, the platform it made is released
            eos.loginStarts = true;
            eos.authNeverAnswers = true;
            CFakeEos other;
            other.authNeverAnswers = true;
            CORE::CFutureResult orphan( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            {
                CEosAuthProvider shortLived( other.MakeShim() );
                ASSERT_TRUE( Configure( shortLived ) );
                ASSERT_TRUE( shortLived.TrySilentLogin( orphan ) );
                ASSERT_FALSE( orphan.IsReady() );
                ASSERT_TRUE( 0 == other.releaseCalls );
            }
            ASSERT_TRUE( orphan.IsReady() );
            ASSERT_TRUE( 1 == other.releaseCalls );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Logout asks the SDK to log out and to forget the persistent login, and forgets the product user id" )
        try
        {
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( LoginSilent( provider, future ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_FALSE( provider.GetProductUserId( result.accountRef ).IsNULLOrEmpty() );

            ASSERT_TRUE( provider.Logout( result.accountRef, AUTH::CSecretString() ) );
            ASSERT_TRUE( 1 == eos.logoutAccounts.size() && eos.logoutAccounts[ 0 ] == "0123456789abcdef0123456789abcdef" );
            ASSERT_TRUE( eos.logoutDeletes[ 0 ] );
            ASSERT_TRUE( provider.GetProductUserId( result.accountRef ).IsNULLOrEmpty() );

            // the SDK is ticked for a while after it, so that the callbacks of the logout run, and then not any more
            UInt32 ticks = eos.tickCalls;
            PumpFor( 40 );
            ASSERT_TRUE( eos.tickCalls > ticks );
            PumpFor( 400 );
            ticks = eos.tickCalls;
            PumpFor( 60 );
            ASSERT_TRUE( ticks == eos.tickCalls );

            // the settings can keep the persistent login
            CORE::CDataNode node = MakeEosNode();
            node.FindChild( "Parameters" )->SetAttribute( "deletePersistentAuthOnLogout", false );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            ASSERT_TRUE( provider.Logout( result.accountRef, AUTH::CSecretString() ) );
            ASSERT_TRUE( 2 == eos.logoutDeletes.size() && !eos.logoutDeletes[ 1 ] );

            // an account of another provider is not logged out of Epic
            ASSERT_TRUE( provider.Logout( AUTH::CAccountRef( AUTH::CAuthProviderIds::Google, "1" ), AUTH::CSecretString() ) );
            ASSERT_TRUE( 2 == eos.logoutAccounts.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Verify hands the id token to the OIDC engine with the epic descriptor, and says no without an engine" )
        try
        {
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            AUTH::CAuthProof proof( AUTH::CAuthProof::KindOidcIdToken, "header.payload.signature" );
            AUTH::CAuthResult result;

            // no engine: unsupported. The process may have an engine of its own, which is put aside for this
            {
                CScopedEngine noEngine( GUCEF_NULL );
                ASSERT_TRUE( GUCEF_NULL == AUTH::CAuthGlobal::Instance()->GetFlowEngine() );
                ASSERT_FALSE( provider.IsVerifySupported() );
                ASSERT_FALSE( provider.Verify( proof, result ) );
            }

            CFakeOidcEngine engine;
            engine.acceptedToken = "header.payload.signature";
            engine.accountToGive = AUTH::CAccountRef( AUTH::CAuthProviderIds::Epic, "fedcba9876543210fedcba9876543210" );
            {
                CScopedEngine scoped( &engine );
                ASSERT_TRUE( provider.IsVerifySupported() );
                ASSERT_TRUE( provider.Verify( proof, result ) );
                ASSERT_TRUE( result.accountRef.ToString() == "epic:fedcba9876543210fedcba9876543210" );
                ASSERT_TRUE( result.providerName == "epic" );
                ASSERT_TRUE( 1 == engine.verifyCount && engine.lastDescriptorName == "epic" );
                ASSERT_TRUE( engine.lastClientId == "xyz-client-id" && engine.lastClientSecret == TestClientSecret );
                ASSERT_TRUE( engine.lastProofKind == AUTH::CAuthProof::KindOidcIdToken && engine.lastProofData == "header.payload.signature" );

                // the engine says no
                ASSERT_FALSE( provider.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "forged" ), result ) );
                // not an id token: the engine is not asked
                UInt32 asked = engine.verifyCount;
                ASSERT_FALSE( provider.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindPlatformTicket, "header.payload.signature" ), result ) );
                ASSERT_FALSE( provider.Verify( AUTH::CAuthProof(), result ) );
                ASSERT_TRUE( asked == engine.verifyCount );

                // an engine that does not support it with these settings
                engine.supported = false;
                ASSERT_FALSE( provider.IsVerifySupported() );
                ASSERT_FALSE( provider.Verify( proof, result ) );
                engine.supported = true;

                // a token of another provider's account is not an Epic account
                engine.accountToGive = AUTH::CAccountRef( AUTH::CAuthProviderIds::Google, "123" );
                ASSERT_FALSE( provider.Verify( proof, result ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Settings: defaults, round trip with the secrets, the form CAuthManager hands over, offered only with a client id, and what is not well formed is refused" )
        try
        {
            CEosAuthSettings defaults;
            ASSERT_TRUE( defaults.useConnect && defaults.createConnectUser && defaults.deletePersistentAuthOnLogout );
            ASSERT_TRUE( defaults.productName == "GUCEF" && defaults.productVersion == "1.0" );
            ASSERT_TRUE( 50 == defaults.tickIntervalInMs && 15000 == defaults.silentTimeoutInMs );
            ASSERT_FALSE( defaults.IsConfigured() );
            ASSERT_TRUE( defaults.IsWellFormed() );

            CEosAuthSettings filled;
            filled.clientId = "client";
            filled.clientSecret.Set( TestClientSecret );
            filled.productId = "p";
            filled.sandboxId = "s";
            filled.deploymentId = "d";
            filled.encryptionKey.Set( TestEncryptionKey );
            filled.productName = "Redux";
            filled.productVersion = "2.5";
            filled.cacheDirectory = "/var/cache/eos";
            filled.useConnect = false;
            filled.createConnectUser = false;
            filled.deletePersistentAuthOnLogout = false;
            filled.tickIntervalInMs = 11;
            filled.silentTimeoutInMs = 2222;
            ASSERT_TRUE( filled.IsConfigured() );

            CORE::CDataNode saved( "epic", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( filled.SaveConfig( saved ) );
            CEosAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( saved ) );
            ASSERT_TRUE( loaded == filled );
            ASSERT_TRUE( loaded.clientSecret.Reveal() == TestClientSecret && loaded.encryptionKey.Reveal() == TestEncryptionKey );

            // json
            ASSERT_TRUE( JsonCodecAvailable() );
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess output( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &saved, &output ) );
            CORE::CDynamicBufferAccess input( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &input ) );
            CEosAuthSettings fromJson;
            ASSERT_TRUE( fromJson.LoadConfig( parsed ) );
            ASSERT_TRUE( fromJson == filled );

            // The manager: CAuthProviderSettings has the client id and secret, the rest is in Parameters
            AUTH::CAuthSettings authSettings;
            AUTH::CAuthProviderSettings providerSettings;
            providerSettings.clientId = "from-manager";
            providerSettings.clientSecret.Set( TestClientSecret );
            providerSettings.jwksUri = "https://keys.example.invalid/jwks";
            providerSettings.parameters[ "productId" ] = "p2";
            providerSettings.parameters[ "sandboxId" ] = "s2";
            providerSettings.parameters[ "deploymentId" ] = "d2";
            authSettings.SetProviderSettings( "epic", providerSettings );

            CFakeEos eos;
            TEosAuthProviderFactory factory;
            AUTH::CAuthManager manager;
            manager.SetProviderFactory( GUCEF_NULL );
            AUTH::CIAuthProviderPtr providerPtr = factory.Create();
            static_cast< CEosAuthProvider* >( providerPtr.GetPointerAlways() )->SetShim( eos.MakeShim() );
            ASSERT_TRUE( manager.AddProvider( providerPtr ) );
            // without settings: not offered, there is no client id
            ASSERT_FALSE( manager.IsProviderOffered( "epic" ) );
            manager.SetSettings( authSettings );
            CEosAuthProvider* managed = static_cast< CEosAuthProvider* >( manager.GetProvider( "epic" ) );
            ASSERT_TRUE( GUCEF_NULL != managed );
            CEosAuthSettings managedSettings = managed->GetSettings();
            ASSERT_TRUE( managedSettings.clientId == "from-manager" && managedSettings.productId == "p2" && managedSettings.deploymentId == "d2" );
            ASSERT_TRUE( managedSettings.clientSecret.Reveal() == TestClientSecret );
            ASSERT_TRUE( manager.IsProviderOffered( "epic" ) );
            // the endpoint overrides are kept for the engine
            ASSERT_TRUE( managedSettings.GetEngineSettings().jwksUri == "https://keys.example.invalid/jwks" );
            ASSERT_TRUE( managedSettings.GetEngineSettings().clientId == "from-manager" );
            manager.RemoveProvider( "epic" );

            // not well formed: the encryption key has to be 64 hex digits, the tick has to be more than 0
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CORE::CDataNode shortKey = MakeEosNode();
            shortKey.FindChild( "Parameters" )->SetAttribute( "encryptionKey", "0011" );
            ASSERT_FALSE( provider.LoadConfig( shortKey ) );
            CORE::CDataNode notHex = MakeEosNode();
            notHex.FindChild( "Parameters" )->SetAttribute( "encryptionKey", "zz112233445566778899aabbccddeeff00112233445566778899aabbccddeeff" );
            ASSERT_FALSE( provider.LoadConfig( notHex ) );
            CORE::CDataNode noTick = MakeEosNode( 0 );
            ASSERT_FALSE( provider.LoadConfig( noTick ) );
            ASSERT_TRUE( provider.GetSettings().productId == "product-1" && provider.GetSettings().tickIntervalInMs == 5 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The client secret, the encryption key and the id token are never logged" )
        try
        {
            CLogCapture capture;
            CFakeEos eos;
            CEosAuthProvider provider( eos.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            ASSERT_TRUE( LoginSilent( provider, future ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            CFakeOidcEngine engine;
            engine.acceptedToken = eos.idToken;
            engine.accountToGive = result.accountRef;
            {
                CScopedEngine scoped( &engine );
                AUTH::CAuthResult verified;
                ASSERT_TRUE( provider.Verify( result.proof, verified ) );
                ASSERT_FALSE( provider.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "forged" ), verified ) );
            }
            eos.authOutcome = EOSOUTCOME_DENIED;
            ASSERT_TRUE( LoginSilent( provider, future ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            provider.Logout( result.accountRef, AUTH::CSecretString() );
            PumpFor( 20 );

            CORE::CString logged = capture.Text();
            ASSERT_TRUE( logged.HasSubstr( "EosAuthProvider" ) >= 0 );
            ASSERT_TRUE( logged.HasSubstr( TestClientSecret ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "5d8e0c47" ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( TestEncryptionKey ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "EOS-ID-TOKEN-SIGNATURE" ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "eyJhbGciOiJSUzI1NiJ9" ) < 0 );
            for ( size_t i=0; i<recorder.records.size(); ++i )
                ASSERT_TRUE( recorder.records[ i ].failure.message.HasSubstr( "EOS-ID-TOKEN" ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Built without the EOS SDK the plugin registers and reports that it is not available" )
        try
        {
            ASSERT_FALSE( CEosSdk::IsCompiledIn() );
            CEosSdk sdk;
            TEosShim empty;
            ASSERT_FALSE( sdk.FillShim( empty ) );
            ASSERT_TRUE( GUCEF_NULL == empty.isSdkAvailable && GUCEF_NULL == empty.authLogin );
            ASSERT_FALSE( sdk.AdoptPlatform( &sdk ) );

            // The provider as an application gets it: configured, and still not available
            CEosAuthProvider provider;
            ASSERT_TRUE( Configure( provider ) );
            ASSERT_FALSE( provider.IsAvailable() );
            CORE::CFutureResult silent( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( provider.TrySilentLogin( silent ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );
            CORE::CFutureResult interactive( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.BeginLogin( AUTH::CAuthOptions(), interactive ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( recorder.Last( CProviderRecorder::KIND_FAILED )->failure.message.HasSubstr( "not part of this build" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Registration: in a factory of its own, next to the built in epic provider, and in its place when the SDK is there" )
        try
        {
            // A factory with no epic provider: it registers
            AUTH::CAuthProviderFactory own;
            ASSERT_TRUE( RegisterEosAuthProviders( own, false ) );
            ASSERT_TRUE( own.IsConstructible( "epic" ) );
            ASSERT_FALSE( RegisterEosAuthProviders( own, false ) == false && !own.IsConstructible( "epic" ) );
            AUTH::CIAuthProviderPtr made = own.Create( "epic" );
            ASSERT_FALSE( made.IsNULL() );
            ASSERT_TRUE( made->GetClassTypeName() == "GUCEF::AUTHPLUGIN::EOS::CEosAuthProvider" );
            made.Unlink();
            UnregisterEosAuthProviders( own, false );
            ASSERT_FALSE( own.IsConstructible( "epic" ) );

            // The factory of the process has the built in provider: without the SDK the plugin leaves it
            AUTH::CAuthProviderFactory& global = AUTH::CAuthGlobal::Instance()->GetProviderFactory();
            ASSERT_TRUE( global.IsConstructible( "epic" ) );
            ASSERT_FALSE( RegisterEosAuthProviders( global, false ) );
            AUTH::CIAuthProviderPtr builtIn = global.Create( "epic" );
            ASSERT_FALSE( builtIn.IsNULL() );
            ASSERT_TRUE( builtIn->GetClassTypeName() == "GUCEF::AUTH::CDescriptorAuthProvider" );
            builtIn.Unlink();

            // With the SDK it takes the place, and gives it back
            ASSERT_TRUE( RegisterEosAuthProviders( global, true ) );
            AUTH::CIAuthProviderPtr replaced = global.Create( "epic" );
            ASSERT_TRUE( replaced->GetClassTypeName() == "GUCEF::AUTHPLUGIN::EOS::CEosAuthProvider" );
            replaced.Unlink();
            UnregisterEosAuthProviders( global, true );
            ASSERT_TRUE( global.IsConstructible( "epic" ) );
            AUTH::CIAuthProviderPtr restored = global.Create( "epic" );
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
