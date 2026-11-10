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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_AUTH_CEMAILOAUTHCREDENTIALPROVIDER_H
#include "gucefAUTH_CEMailOAuthCredentialProvider.h"
#define GUCEF_AUTH_CEMAILOAUTHCREDENTIALPROVIDER_H
#endif /* GUCEF_AUTH_CEMAILOAUTHCREDENTIALPROVIDER_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#include "gucefAUTH_CLocalIdentityLinkStore.h"
#define GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H ? */

#ifndef GUCEF_COM_CIMAPCLIENT_H
#include "gucefCOM_CImapClient.h"
#define GUCEF_COM_CIMAPCLIENT_H
#endif /* GUCEF_COM_CIMAPCLIENT_H ? */

#ifndef GUCEF_COM_CSMTPCLIENT_H
#include "gucefCOM_CSmtpClient.h"
#define GUCEF_COM_CSMTPCLIENT_H
#endif /* GUCEF_COM_CSMTPCLIENT_H ? */

#ifndef GUCEF_COM_CEMAILPROVIDERPRESETS_H
#include "gucefCOM_CEMailProviderPresets.h"
#define GUCEF_COM_CEMAILPROVIDERPRESETS_H
#endif /* GUCEF_COM_CEMAILPROVIDERPRESETS_H ? */

#ifndef GUCEF_TEST_IMAP_SERVER_H
#include "gucef_test_imap_server.h"
#define GUCEF_TEST_IMAP_SERVER_H
#endif /* GUCEF_TEST_IMAP_SERVER_H ? */

#ifndef GUCEF_TEST_SMTP_SERVER_H
#include "gucef_test_smtp_server.h"
#define GUCEF_TEST_SMTP_SERVER_H
#endif /* GUCEF_TEST_SMTP_SERVER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestFiles.h"
#include "TestLogCapture.h"
#include "TestFakeProvider.h"
#include "TestMailOAuth.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CORE::CTFactory< AUTH::CIAuthProvider, CFakeAuthProvider, MT::CMutex > TFakeFactory;

/** Counts the CredentialReadyEvents of a provider */
class CReadyCounter : public CORE::CObserver
{
    public:

    UInt32 count;
    GUCEF::vector< UInt32 > requestIds;

    CReadyCounter( void ) : CORE::CObserver(), count( 0 ), requestIds() {}
    virtual ~CReadyCounter() { UnsubscribeAllFromObserver(); }

    void Attach( COM::CIEMailCredentialProvider& provider )
    {
        typedef CORE::CTEventHandlerFunctor< CReadyCounter > TCallback;
        TCallback callback( this, &CReadyCounter::OnReady );
        SubscribeTo( &provider, COM::CIEMailCredentialProvider::CredentialReadyEvent, callback );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CReadyCounter";
        return classTypeName;
    }

    private:

    void OnReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++count;
        const COM::CIEMailCredentialProvider::TCredentialReadyEventData* data = static_cast< const COM::CIEMailCredentialProvider::TCredentialReadyEventData* >( eventData );
        if ( GUCEF_NULL != data )
            requestIds.push_back( data->GetData() );
    }
};

/** Watches the connection of a mail client */
class CConnectWatch : public CORE::CObserver
{
    public:

    bool ready;
    bool failed;
    bool settled;

    CConnectWatch( void ) : CORE::CObserver(), ready( false ), failed( false ), settled( false ) {}
    virtual ~CConnectWatch() { UnsubscribeAllFromObserver(); }

    void Attach( COM::CEMailClientBase& client )
    {
        typedef CORE::CTEventHandlerFunctor< CConnectWatch > TCallback;
        TCallback readyCallback( this, &CConnectWatch::OnReady );
        SubscribeTo( &client, COM::CEMailClientBase::ReadyEvent, readyCallback );
        TCallback errorCallback( this, &CConnectWatch::OnError );
        SubscribeTo( &client, COM::CEMailClientBase::ErrorEvent, errorCallback );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CConnectWatch";
        return classTypeName;
    }

    private:

    void OnReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { ready = true; settled = true; }
    void OnError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { failed = true; settled = true; }
};

/** A manager with a fake provider "google", a store in memory and a mail credential provider on them */
class CEMailOAuthFixture
{
    public:

    static const char* const MailUser;

    AUTH::CMemoryCredentialStore store;
    AUTH::CLocalIdentityLinkStore linkStore;
    AUTH::CAuthManager manager;
    AUTH::CEMailOAuthCredentialProvider provider;
    CFakeAuthProvider* fake;
    AUTH::CAccountRef account;

    CEMailOAuthFixture( void ) : store(), linkStore(), manager(), provider(), fake( GUCEF_NULL ), account()
    {
        manager.SetCredentialStore( &store );
        manager.SetLinkStore( &linkStore );
        static TFakeFactory factory;        // outlives the providers it made, as the providers of the manager tests do
        TFakeFactory::TProductPtr product = factory.Create();
        fake = static_cast< CFakeAuthProvider* >( product.GetPointerAlways() );
        fake->name = "google";
        fake->accountNamespace = "google";
        fake->subject = "mail-user-1";
        manager.AddProvider( product );
        provider.SetManager( &manager );
        provider.SetCredentialStore( &store );
    }

    ~CEMailOAuthFixture()
    {
        provider.SetManager( GUCEF_NULL );
    }

    /** Signs in with the fake provider and points the credential provider at the account */
    bool SignIn( void )
    {
        AUTH::CAuthOptions options;
        options.linkToProfile = false;
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        if ( !manager.Login( "google", options, future ) )
            return false;
        for ( UInt32 waited=0; waited<5000 && !future.IsReady(); ++waited )
            PumpFor( 1 );
        AUTH::CAuthResult result;
        if ( !AUTH::CAuthResult::TryGetFromFuture( future, result ) )
            return false;
        account = result.accountRef;
        provider.SetAccount( "google", account, MailUser );
        return true;
    }

    COM::CIEMailCredentialProvider::EResult Ask( COM::CEMailCredential& credential, bool previousFailed = false, UInt32 requestId = 1 )
    {
        COM::CEMailCredentialRequest request;
        request.protocol = COM::MAILPROTOCOL_IMAP;
        request.host = "127.0.0.1";
        request.userName = MailUser;
        request.previousAttemptFailed = previousFailed;
        request.requestId = requestId;
        return provider.GetCredential( request, credential );
    }

    private:

    CEMailOAuthFixture( const CEMailOAuthFixture& src );
    CEMailOAuthFixture& operator=( const CEMailOAuthFixture& src );
};

const char* const CEMailOAuthFixture::MailUser = "mail-user-oauth";

/** Reads json text into a data tree with the json codec */
bool
ParseJsonText( const CORE::CString& text, CORE::CDataNode& tree )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) )
        return false;
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( text.ByteSize() - 1, text.C_String() );
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &tree, &access );
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailOAuthTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING mail oauth TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "MailOAuth" );

    RegisterFakeLoginTask();

    GUCEF_TESTFW_TESTCASE( "Test: The mail credential provider gives the token of the session as a bearer token, without a renewal" )
        try
        {
            CEMailOAuthFixture f;
            COM::CEMailCredential credential;
            // nothing set: no credential
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_FAILED == f.Ask( credential ) );
            ASSERT_FALSE( f.provider.HasLogin() );

            ASSERT_TRUE( f.SignIn() );
            ASSERT_TRUE( f.provider.HasLogin() );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_READY == f.Ask( credential ) );
            ASSERT_TRUE( COM::MAILCRED_BEARER_TOKEN == credential.type && credential.secret == CFakeAuthProvider::SecretAccessToken && credential.userName == CEMailOAuthFixture::MailUser );
            ASSERT_TRUE( 0 == f.fake->refreshCount && 0 == f.provider.GetRefreshCount() );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_READY == f.Ask( credential ) && 0 == f.fake->refreshCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A token that expires soon is renewed with the stored refresh token, the client waits for the event, and the rotated refresh token is kept" )
        try
        {
            CEMailOAuthFixture f;
            CReadyCounter ready;
            ready.Attach( f.provider );
            ASSERT_TRUE( f.SignIn() );
            AUTH::CSecretString before;
            ASSERT_TRUE( f.store.Load( "google", f.account.ToString(), before ) && !before.IsEmpty() );

            f.provider.SetSkewMs( 2 * 3600 * 1000 );        // the token of the fake lives an hour: it counts as expired
            COM::CEMailCredential credential;
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_PENDING == f.Ask( credential, false, 41 ) );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_PENDING == f.Ask( credential, false, 42 ) );      // one renewal serves both
            ASSERT_TRUE( PumpUntilAtLeast( ready.count, 1, 5000 ) );
            ASSERT_TRUE( 1 == f.fake->refreshCount && 1 == f.provider.GetRefreshCount() );
            ASSERT_TRUE( f.fake->lastRefreshTokenSeen == before.Reveal() );
            bool got41 = false, got42 = false;
            for ( size_t i=0; i<ready.requestIds.size(); ++i )
            {
                got41 = got41 || 41 == ready.requestIds[ i ];
                got42 = got42 || 42 == ready.requestIds[ i ];
            }
            ASSERT_TRUE( got41 && got42 );
            f.provider.SetSkewMs( 1000 );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_READY == f.Ask( credential ) && credential.secret == CFakeAuthProvider::SecretAccessToken );
            AUTH::CSecretString after;
            ASSERT_TRUE( f.store.Load( "google", f.account.ToString(), after ) && !after.IsEmpty() && after.Reveal() != before.Reveal() );
            ASSERT_FALSE( f.provider.HasRefreshFailed() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A token the server refused is renewed, and refusals are counted" )
        try
        {
            CEMailOAuthFixture f;
            CReadyCounter ready;
            ready.Attach( f.provider );
            ASSERT_TRUE( f.SignIn() );
            COM::CEMailCredential credential;
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_READY == f.Ask( credential ) );
            COM::CEMailCredentialRequest request;
            f.provider.ReportRejected( request );
            ASSERT_TRUE( 1 == f.provider.GetRejectionCount() );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_PENDING == f.Ask( credential, true, 7 ) );
            ASSERT_TRUE( PumpUntilAtLeast( ready.count, 1, 5000 ) );
            ASSERT_TRUE( 1 == f.fake->refreshCount );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_READY == f.Ask( credential, false ) && 1 == f.fake->refreshCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A renewal that fails ends the wait with no credential, is not repeated within the retry delay, and works again later" )
        try
        {
            CEMailOAuthFixture f;
            CReadyCounter ready;
            ready.Attach( f.provider );
            ASSERT_TRUE( f.SignIn() );
            f.provider.SetSkewMs( 2 * 3600 * 1000 );
            f.fake->refreshBehaviour = FAKE_FAIL;
            f.fake->failReason = AUTH::AUTHFAIL_INVALID_GRANT;
            COM::CEMailCredential credential;
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_PENDING == f.Ask( credential ) );
            ASSERT_TRUE( PumpUntilAtLeast( ready.count, 1, 5000 ) );
            ASSERT_TRUE( f.provider.HasRefreshFailed() && 1 == f.fake->refreshCount );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_FAILED == f.Ask( credential ) );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_FAILED == f.Ask( credential, true ) );
            ASSERT_TRUE( 1 == f.fake->refreshCount );

            f.provider.SetRetryDelayMs( 0 );
            f.fake->refreshBehaviour = FAKE_COMPLETE;
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_PENDING == f.Ask( credential ) );
            ASSERT_TRUE( PumpUntilAtLeast( ready.count, 2, 5000 ) );
            f.provider.SetSkewMs( 1000 );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_READY == f.Ask( credential ) && !f.provider.HasRefreshFailed() && 2 == f.fake->refreshCount );

            // an account whose refresh token is gone cannot be renewed
            f.provider.SetSkewMs( 2 * 3600 * 1000 );
            ASSERT_TRUE( f.store.Remove( "google", f.account.ToString() ) );
            f.provider.Reset();
            f.provider.SetAccount( "google", f.account, CEMailOAuthFixture::MailUser );
            ASSERT_TRUE( COM::CIEMailCredentialProvider::CRED_FAILED == f.Ask( credential ) && 2 == f.fake->refreshCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The IMAP and SMTP clients log in with XOAUTH2 through the provider, also after the token was renewed, and no token reaches a log" )
        try
        {
            CLogCapture logs;
            CEMailOAuthFixture f;
            ASSERT_TRUE( f.SignIn() );

            TESTFW::CTestImapServer imap;
            imap.allowLogin = false;
            imap.authMechanisms = "XOAUTH2";
            imap.user = CEMailOAuthFixture::MailUser;
            imap.token = CFakeAuthProvider::SecretAccessToken;
            ASSERT_TRUE( imap.Start() );
            TESTFW::CTestSmtpServer smtp;
            smtp.authMechanisms = "XOAUTH2";
            smtp.user = CEMailOAuthFixture::MailUser;
            smtp.token = CFakeAuthProvider::SecretAccessToken;
            ASSERT_TRUE( smtp.Start() );

            for ( int round=0; round<2; ++round )
            {
                // the second round starts with a token that counts as expired: the renewal runs while the client waits
                if ( 1 == round )
                    f.provider.SetSkewMs( 2 * 3600 * 1000 );
                COM::CImapClientSettings imapSettings;
                imapSettings.host = "127.0.0.1";
                imapSettings.port = imap.GetPort();
                imapSettings.tlsMode = COM::MAILTLS_NONE;
                imapSettings.allowInsecureAuth = true;
                imapSettings.userName = CEMailOAuthFixture::MailUser;
                COM::CImapClient imapClient;
                imapClient.SetCredentialProvider( &f.provider );
                CConnectWatch imapWatch;
                imapWatch.Attach( imapClient );
                ASSERT_TRUE( imapClient.Connect( imapSettings ) );
                ASSERT_TRUE( PumpUntilTrue( imapWatch.settled, 8000 ) && imapWatch.ready && !imapWatch.failed );
                imapClient.Disconnect();

                COM::CSmtpClientSettings smtpSettings;
                smtpSettings.host = "127.0.0.1";
                smtpSettings.port = smtp.GetPort();
                smtpSettings.tlsMode = COM::MAILTLS_NONE;
                smtpSettings.allowInsecureAuth = true;
                smtpSettings.userName = CEMailOAuthFixture::MailUser;
                smtpSettings.heloName = "redux.test";
                COM::CSmtpClient smtpClient;
                smtpClient.SetCredentialProvider( &f.provider );
                CConnectWatch smtpWatch;
                smtpWatch.Attach( smtpClient );
                ASSERT_TRUE( smtpClient.Connect( smtpSettings ) );
                ASSERT_TRUE( PumpUntilTrue( smtpWatch.settled, 8000 ) && smtpWatch.ready && !smtpWatch.failed );
                smtpClient.Disconnect();
            }
            ASSERT_TRUE( f.fake->refreshCount >= 1 );

            // a client that is given a token the server does not know fails with an authentication error
            imap.token = "another-token";
            f.provider.SetSkewMs( 1000 );
            COM::CImapClientSettings imapSettings;
            imapSettings.host = "127.0.0.1";
            imapSettings.port = imap.GetPort();
            imapSettings.tlsMode = COM::MAILTLS_NONE;
            imapSettings.allowInsecureAuth = true;
            imapSettings.userName = CEMailOAuthFixture::MailUser;
            COM::CImapClient refused;
            refused.SetCredentialProvider( &f.provider );
            CConnectWatch refusedWatch;
            refusedWatch.Attach( refused );
            ASSERT_TRUE( refused.Connect( imapSettings ) );
            ASSERT_TRUE( PumpUntilTrue( refusedWatch.settled, 15000 ) && refusedWatch.failed && !refusedWatch.ready );
            refused.Disconnect();
            ASSERT_TRUE( f.provider.GetRejectionCount() >= 1 );

            ASSERT_FALSE( logs.Contains( CFakeAuthProvider::SecretAccessToken ) || logs.Contains( CFakeAuthProvider::SecretRefreshTokenPrefix ) || logs.Contains( CFakeAuthProvider::SecretIdToken ) );
            ASSERT_FALSE( logs.Contains( CEMailOAuthFixture::MailUser ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The table of mail providers loads from a json file: an override, a new provider, a removal, and an entry that is not usable is skipped" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseJsonText(
                "{ \"gmail\": { \"imapHost\": \"imap.json-override.example\", \"imapPort\": 1993, \"domains\": [ \"gmail.com\", \"googlemail.com\", \"json-mail.example\" ] },"
                "  \"corpmail\": { \"displayName\": \"Corp mail\", \"domains\": \"corp.example\", \"imapHost\": \"imap.corp.example\", \"imapSecurity\": \"starttls\", \"smtpHost\": \"smtp.corp.example\", \"smtpPort\": 2525, \"smtpSecurity\": \"none\" },"
                "  \"yahoo\": { \"remove\": true },"
                "  \"broken\": { \"imapHost\": \"imap.bad.example\\r\\nX\", \"smtpHost\": \"smtp.bad.example\" } }", tree ) );
            COM::CEMailProviderPresets presets;
            const UInt32 before = presets.GetCount();
            ASSERT_TRUE( presets.LoadConfig( tree ) );
            ASSERT_TRUE( 1 == presets.GetSkippedCount() );
            ASSERT_TRUE( presets.GetCount() == before );          // + corpmail - yahoo
            const COM::CEMailProviderPreset* preset = GUCEF_NULL;
            ASSERT_TRUE( COM::CEMailProviderPresets::RECOGNIZE_KNOWN == presets.Recognize( "me@json-mail.example", preset ) && preset->id == "gmail" && preset->imap.host == "imap.json-override.example" && 1993 == preset->imap.port );
            ASSERT_TRUE( preset->smtp.host == "smtp.gmail.com" );
            ASSERT_TRUE( COM::CEMailProviderPresets::RECOGNIZE_KNOWN == presets.Recognize( "me@corp.example", preset ) && preset->id == "corpmail" && COM::MAILTLS_STARTTLS == preset->imap.security && 2525 == preset->smtp.port && COM::MAILTLS_NONE == preset->smtp.security );
            ASSERT_TRUE( COM::CEMailProviderPresets::RECOGNIZE_UNKNOWN == presets.Recognize( "me@yahoo.com", preset ) );

            // saved as json text and read back it is the same table
            CORE::CDataNode saved( "MailProviders" );
            ASSERT_TRUE( presets.SaveConfig( saved ) );
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess access( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &saved, &access ) && buffer.GetDataSize() > 0 );
            CORE::CDataNode again;
            access.Setpos( 0 );
            ASSERT_TRUE( codec->BuildDataTree( &again, &access ) );
            COM::CEMailProviderPresets reread;
            reread.Clear();
            ASSERT_TRUE( reread.LoadConfig( again ) && reread.GetCount() == presets.GetCount() && 0 == reread.GetSkippedCount() );
            ASSERT_TRUE( COM::CEMailProviderPresets::RECOGNIZE_KNOWN == reread.Recognize( "me@corp.example", preset ) && preset->id == "corpmail" && 2525 == preset->smtp.port );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
