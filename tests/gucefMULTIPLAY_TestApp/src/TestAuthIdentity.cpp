/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#include "gucefAUTH_CFileCredentialStore.h"
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CLocalIdentityProvider.h"
#define GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CAuthIdentityProvider.h"
#define GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H
#include "gucefMULTIPLAY_CPlayerIdentitySigner.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestFakeSsoProvider.h"
#include "TestAuthIdentity.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static TFakeSsoProviderFactory g_fakeSsoFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Records what an identity provider reports */
class CSsoRecorder : public CORE::CObserver
{
    public:

    UInt32 changes;
    UInt32 settingsChanges;
    MULTIPLAY::CPlayerIdentity last;

    CSsoRecorder( void ) : CORE::CObserver(), changes( 0 ), settingsChanges( 0 ), last() {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CSsoRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CIIdentityProvider::IdentityChangedEvent == eventId && GUCEF_NULL != eventData )
        {
            ++changes;
            last = *static_cast< const MULTIPLAY::CPlayerIdentity* >( eventData );
        }
        else
        if ( MULTIPLAY::CAuthIdentityProvider::AuthSettingsChangedEvent == eventId )
        {
            ++settingsChanges;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** A credential store that fails in the way a test asks for */
class CFlakyCredentialStore : public AUTH::ICredentialStore
{
    public:

    enum EMode { MODE_WORKS, MODE_REFUSES_STORE, MODE_WRONG_READBACK };

    EMode mode;
    AUTH::CMemoryCredentialStore inner;

    explicit CFlakyCredentialStore( EMode storeMode ) : AUTH::ICredentialStore(), mode( storeMode ), inner() {}

    virtual bool Store( const CORE::CString& providerName, const CORE::CString& accountKey, const AUTH::CSecretString& secret ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MODE_REFUSES_STORE == mode )
            return false;
        return inner.Store( providerName, accountKey, secret );
    }

    virtual bool Load( const CORE::CString& providerName, const CORE::CString& accountKey, AUTH::CSecretString& secret ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !inner.Load( providerName, accountKey, secret ) )
            return false;
        if ( MODE_WRONG_READBACK == mode )
            secret.Set( "00" + secret.Reveal().SubstrToIndex( 2, false ) );
        return true;
    }

    virtual bool Remove( const CORE::CString& providerName, const CORE::CString& accountKey ) GUCEF_VIRTUAL_OVERRIDE
    {
        return inner.Remove( providerName, accountKey );
    }

    virtual bool ListAccounts( const CORE::CString& providerName, CORE::CString::StringVector& accountKeys ) GUCEF_VIRTUAL_OVERRIDE
    {
        return inner.ListAccounts( providerName, accountKeys );
    }

    virtual bool RemoveProvider( const CORE::CString& providerName ) GUCEF_VIRTUAL_OVERRIDE
    {
        return inner.RemoveProvider( providerName );
    }

    virtual bool Wipe( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        return inner.Wipe();
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A directory of the work directory of the tests, made empty */
static CORE::CString
FreshDirectory( const CORE::CString& name )
{
    CORE::CString path = CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" ), name );
    if ( CORE::DirExists( path ) )
        CORE::DeleteDir( path, true );
    CORE::CreateDirs( path );
    return path;
}

/*-------------------------------------------------------------------------*/

/** The config of an sso provider that keeps everything below the directory */
static void
MakeSsoConfig( const CORE::CString& dir, CORE::CDataNode& config, const char* keyOption = GUCEF_NULL )
{
    config = CORE::CDataNode( "Identity" );
    config.SetAttribute( "profilePath", CORE::CombinePath( dir, "PlayerProfile.json" ) );
    config.SetAttribute( "appName", CORE::CString( "TestGame" ) );
    config.SetAttribute( "defaultDisplayName", CORE::CString( "Newcomer" ) );
    if ( GUCEF_NULL != keyOption )
        config.SetAttribute( "keyInCredentialStore", CORE::CString( keyOption ) );
    CORE::CDataNode* auth = config.AddChild( "Auth", GUCEF_DATATYPE_OBJECT );
    auth->SetAttribute( "appName", CORE::CString( "TestGame" ) );
    auth->SetAttribute( "credentialDirectory", CORE::CombinePath( dir, "cred" ) );
    auth->SetAttribute( "linkStorePath", CORE::CombinePath( dir, "links.json" ) );
}

/*-------------------------------------------------------------------------*/

static MULTIPLAY::CAuthIdentityProvider*
AsSso( MULTIPLAY::CIIdentityProviderPtr& provider )
{
    return static_cast< MULTIPLAY::CAuthIdentityProvider* >( provider.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

static CFakeSsoProvider*
AddFake( AUTH::CAuthManager& manager, const CORE::CString& name, const CORE::CString& ns, const CORE::CString& subject, const CORE::CString& displayName )
{
    AUTH::CIAuthProviderPtr product = g_fakeSsoFactory.Create();
    CFakeSsoProvider* fake = static_cast< CFakeSsoProvider* >( product.GetPointerAlways() );
    fake->name = name;
    fake->accountNamespace = ns;
    fake->subject = subject;
    fake->displayName = displayName;
    if ( !manager.AddProvider( product ) )
        return GUCEF_NULL;
    return fake;
}

/*-------------------------------------------------------------------------*/

static bool
AwaitFuture( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
        PumpFor( 1 );
    return future.IsReady();
}

/*-------------------------------------------------------------------------*/

static bool
LoginWith( MULTIPLAY::CAuthIdentityProvider& provider, const CORE::CString& providerName )
{
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( !provider.Login( providerName, future ) || !AwaitFuture( future, 5000 ) )
        return false;
    AUTH::CAuthResult result;
    return AUTH::CAuthResult::TryGetFromFuture( future, result ) && result.IsValid();
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ReadText( const CORE::CString& path )
{
    CORE::CString content;
    CORE::LoadTextFileAsString( path, content );
    return content;
}

/*-------------------------------------------------------------------------*/

/** Whether the file has the bytes of the text anywhere */
static bool
FileHasBytes( const CORE::CString& path, const CORE::CString& text )
{
    CORE::CDynamicBuffer buffer;
    if ( !buffer.LoadContentFromFile( path ) || text.IsNULLOrEmpty() )
        return false;
    const char* data = static_cast< const char* >( buffer.GetConstBufferPtr() );
    UInt32 size = buffer.GetDataSize();
    UInt32 needle = text.ByteSize() - 1;
    for ( UInt32 i=0; i+needle<=size; ++i )
    {
        if ( 0 == memcmp( data + i, text.C_String(), needle ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Makes the profile of a local provider that keeps its key in the profile file, as profiles did before */
static bool
MakePlainProfile( const CORE::CString& dir, MULTIPLAY::CPlayerIdentity& identity )
{
    CORE::CDataNode config;
    MakeSsoConfig( dir, config );
    MULTIPLAY::CLocalIdentityProvider local;
    return local.LoadConfig( config ) && local.GetIdentity( identity ) && identity.HasKeyPair();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAuthIdentityTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING auth identity TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "AuthIdentity" );

    GUCEF_TESTFW_TESTCASE( "Test: The key of a player identity signs for gucefAUTH: the user id is the key id and a signature holds for the public key" )
        try
        {
            REQUIRE_TRUE( MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() );
            MULTIPLAY::CPlayerIdentity alice;
            REQUIRE_TRUE( alice.GenerateKeyPair() );
            MULTIPLAY::CPlayerIdentitySigner signer( alice );
            ASSERT_TRUE( alice.userId == signer.GetKeyId() && alice.publicKey == signer.GetPublicKeyHex() );

            CORE::CDynamicBuffer signature;
            ASSERT_TRUE( signer.Sign( "link", 4, signature ) && 64 == signature.GetDataSize() );
            CORE::CString signatureHex = AUTH::CAuthCrypto::ToHex( signature.GetConstBufferPtr(), signature.GetDataSize() );
            ASSERT_TRUE( AUTH::CAuthCrypto::VerifySignature( signer.GetPublicKeyHex(), "link", 4, signatureHex ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::VerifySignature( signer.GetPublicKeyHex(), "lonk", 4, signatureHex ) );

            // Another identity is another key; one without a key signs nothing
            MULTIPLAY::CPlayerIdentity bob;
            REQUIRE_TRUE( bob.GenerateKeyPair() );
            signer.SetIdentity( bob );
            ASSERT_TRUE( bob.userId == signer.GetKeyId() && alice.userId != signer.GetKeyId() );
            ASSERT_FALSE( AUTH::CAuthCrypto::VerifySignature( signer.GetPublicKeyHex(), "link", 4, signatureHex ) );
            MULTIPLAY::CPlayerIdentity keyless;
            keyless.userId = MULTIPLAY::CPlayerIdentity::MakeGuid();
            signer.SetIdentity( keyless );
            ASSERT_TRUE( signer.GetPublicKeyHex().IsNULLOrEmpty() );
            ASSERT_FALSE( signer.Sign( "link", 4, signature ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The sso provider is registered, and without a login it is a guest with the identity of a local profile" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString dir = FreshDirectory( "ssoGuest" );
            CORE::CDataNode config;
            MakeSsoConfig( dir, config );

            MULTIPLAY::CIIdentityProviderPtr provider = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
            REQUIRE_TRUE( !provider.IsNULL() );
            ASSERT_TRUE( "sso" == provider->GetProviderName() );
            ASSERT_TRUE( provider->LoadConfig( config ) );
            MULTIPLAY::CPlayerIdentity identity;
            REQUIRE_TRUE( provider->GetIdentity( identity ) );
            ASSERT_TRUE( identity.HasKeyPair() && identity.IsUserIdOfKey() && "Newcomer" == identity.displayName );
            ASSERT_TRUE( 0 == identity.accounts.GetKeyCount() && identity.linkProofs.empty() );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == AsSso( provider )->GetLoginState() );

            // The local provider ignores logins, so does anything that is not an sso provider
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            MULTIPLAY::CIIdentityProviderPtr local = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "local" );
            REQUIRE_TRUE( !local.IsNULL() );
            ASSERT_FALSE( local->Login( "fakesso", future ) );
            ASSERT_FALSE( local->AutoLogin( future ) );

            // A provider that is not there is not logged in with
            ASSERT_FALSE( provider->Login( "nosuchprovider", future ) );

            // The same player on the next run
            MULTIPLAY::CIIdentityProviderPtr again = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
            ASSERT_TRUE( again->LoadConfig( config ) );
            MULTIPLAY::CPlayerIdentity second;
            REQUIRE_TRUE( again->GetIdentity( second ) );
            ASSERT_TRUE( second.userId == identity.userId && second.privateKey == identity.privateKey );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Logging in links the account: accounts, the name of the account for a default name, proofs, events, unlinking, and the links survive a restart" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString dir = FreshDirectory( "ssoLink" );
            CORE::CDataNode config;
            MakeSsoConfig( dir, config );
            CORE::CString userId;
            {
                MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
                REQUIRE_TRUE( !product.IsNULL() && product->LoadConfig( config ) );
                MULTIPLAY::CAuthIdentityProvider& provider = *AsSso( product );
                CSsoRecorder recorder;
                recorder.SubscribeTo( &provider );

                CFakeSsoProvider* fakeA = AddFake( provider.GetAuthManager(), "fakesso", "fake", "player-1", "Fake Player" );
                REQUIRE_TRUE( GUCEF_NULL != fakeA );
                CORE::CString::StringVector offered;
                provider.GetOfferedProviders( offered );
                bool offersFake = false;
                for ( size_t i=0; i<offered.size(); ++i )
                    offersFake = offersFake || "fakesso" == offered[ i ];
                ASSERT_TRUE( offersFake );

                // Nothing linked: a guest, with the name of the profile
                MULTIPLAY::CPlayerIdentity guest;
                REQUIRE_TRUE( provider.GetIdentity( guest ) );
                userId = guest.userId;
                ASSERT_TRUE( "Newcomer" == guest.displayName && guest.accounts.GetKeyCount() == 0 );
                ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == provider.GetLoginState() );

                // Logged in: linked to the profile, the account is on the identity, the name is that of the account
                ASSERT_TRUE( LoginWith( provider, "fakesso" ) );
                ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == provider.GetLoginState() );
                ASSERT_TRUE( PumpUntilAtLeast( recorder.changes, 1, 2000 ) );
                MULTIPLAY::CPlayerIdentity linked;
                REQUIRE_TRUE( provider.GetIdentity( linked ) );
                ASSERT_TRUE( userId == linked.userId );
                ASSERT_TRUE( "player-1" == linked.accounts.GetValueAlways( "fake" ).AsString() );
                ASSERT_TRUE( "Fake Player" == linked.displayName );
                REQUIRE_TRUE( 1 == linked.linkProofs.size() );
                ASSERT_TRUE( "fakesso" == linked.linkProofs[ 0 ].providerName && AUTH::CAuthProof::KindOidcIdToken == linked.linkProofs[ 0 ].proof.kind );
                ASSERT_TRUE( linked.linkProofs[ 0 ].IsSendable() );
                AUTH::TAccountRefVector accounts;
                REQUIRE_TRUE( provider.GetLinkedAccounts( accounts ) && 1 == accounts.size() );
                ASSERT_TRUE( "fake:player-1" == accounts[ 0 ].ToString() && accounts[ 0 ] == provider.GetPrimaryAccount() );

                // The event is the public identity: the accounts, never the key or the proofs
                ASSERT_TRUE( userId == recorder.last.userId && "player-1" == recorder.last.accounts.GetValueAlways( "fake" ).AsString() );
                ASSERT_TRUE( recorder.last.privateKey.IsNULLOrEmpty() && recorder.last.linkProofs.empty() );

                // A name the player chose is not replaced by the name of the account
                UInt32 before = recorder.changes;
                ASSERT_TRUE( provider.SetDisplayName( "Alice" ) );
                ASSERT_TRUE( PumpUntilAtLeast( recorder.changes, before + 1, 2000 ) );
                ASSERT_TRUE( "Alice" == recorder.last.displayName && "player-1" == recorder.last.accounts.GetValueAlways( "fake" ).AsString() );
                REQUIRE_TRUE( provider.GetIdentity( linked ) );
                ASSERT_TRUE( "Alice" == linked.displayName );

                // Accounts of the local provider stay, and a second account at one provider is a second value
                ASSERT_TRUE( provider.SetAccount( "irc", "alice" ) );
                CFakeSsoProvider* fakeB = AddFake( provider.GetAuthManager(), "fakesso2", "fake", "player-2", "Second" );
                REQUIRE_TRUE( GUCEF_NULL != fakeB );
                ASSERT_TRUE( LoginWith( provider, "fakesso2" ) );
                REQUIRE_TRUE( provider.GetIdentity( linked ) );
                ASSERT_TRUE( "alice" == linked.accounts.GetValueAlways( "irc" ).AsString() );
                ASSERT_TRUE( 2 == linked.accounts.GetValueVectorAlways( "fake" ).size() && 2 == linked.linkProofs.size() );
                ASSERT_TRUE( 2 == linked.accounts.GetKeyCount() );

                // Unlinked: the account is gone from the identity, the profile is the same
                before = recorder.changes;
                REQUIRE_TRUE( provider.Unlink( AUTH::CAccountRef( "fake", "player-2" ) ) );
                ASSERT_TRUE( PumpUntilAtLeast( recorder.changes, before + 1, 2000 ) );
                REQUIRE_TRUE( provider.GetIdentity( linked ) );
                ASSERT_TRUE( 1 == linked.accounts.GetValueVectorAlways( "fake" ).size() && 1 == linked.linkProofs.size() );
                ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == provider.GetLoginState() );
                before = recorder.changes;
                REQUIRE_TRUE( provider.Unlink( AUTH::CAccountRef( "fake", "player-1" ) ) );
                ASSERT_TRUE( PumpUntilAtLeast( recorder.changes, before + 1, 2000 ) );
                REQUIRE_TRUE( provider.GetIdentity( linked ) );
                ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == provider.GetLoginState() );
                ASSERT_TRUE( 0 == linked.accounts.GetValueVectorAlways( "fake" ).size() && linked.linkProofs.empty() && userId == linked.userId );
                ASSERT_TRUE( provider.Unlink( AUTH::CAccountRef( "fake", "player-1" ) ) );   // nothing to unlink is not a failure

                // Linked again for the next run
                ASSERT_TRUE( LoginWith( provider, "fakesso" ) );
            }

            // The next run: the link is there (the file of the link store), the login is not
            MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
            REQUIRE_TRUE( !product.IsNULL() && product->LoadConfig( config ) );
            MULTIPLAY::CAuthIdentityProvider& again = *AsSso( product );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == again.GetLoginState() );
            MULTIPLAY::CPlayerIdentity identity;
            REQUIRE_TRUE( again.GetIdentity( identity ) );
            ASSERT_TRUE( userId == identity.userId && "player-1" == identity.accounts.GetValueAlways( "fake" ).AsString() );
            ASSERT_TRUE( identity.linkProofs.empty() && "Alice" == identity.displayName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A profile that still has the default name takes the name of the linked account, and the name of the profile when the login is not there" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString dir = FreshDirectory( "ssoName" );
            CORE::CDataNode config;
            MakeSsoConfig( dir, config );
            {
                MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
                REQUIRE_TRUE( product->LoadConfig( config ) );
                REQUIRE_TRUE( GUCEF_NULL != AddFake( AsSso( product )->GetAuthManager(), "fakesso", "fake", "player-1", "Fake Player" ) );
                ASSERT_TRUE( LoginWith( *AsSso( product ), "fakesso" ) );
                MULTIPLAY::CPlayerIdentity identity;
                REQUIRE_TRUE( product->GetIdentity( identity ) );
                ASSERT_TRUE( "Fake Player" == identity.displayName );
            }
            // Not persisted as the name of the profile: the next run without the login has the default again
            MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
            REQUIRE_TRUE( product->LoadConfig( config ) );
            MULTIPLAY::CPlayerIdentity identity;
            REQUIRE_TRUE( product->GetIdentity( identity ) );
            ASSERT_TRUE( "Newcomer" == identity.displayName && 1 == identity.accounts.GetKeyCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The local provider is as it was: the key in the profile file, no accounts but its own, no login" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString dir = FreshDirectory( "localCompat" );
            MULTIPLAY::CPlayerIdentity identity;
            REQUIRE_TRUE( MakePlainProfile( dir, identity ) );
            CORE::CString profile = ReadText( CORE::CombinePath( dir, "PlayerProfile.json" ) );
            ASSERT_TRUE( profile.HasSubstr( "privateKey" ) >= 0 && profile.HasSubstr( identity.privateKey ) >= 0 );
            ASSERT_TRUE( profile.HasSubstr( "keyLocation" ) < 0 );
            ASSERT_TRUE( identity.linkProofs.empty() && 0 == identity.accounts.GetKeyCount() );
            ASSERT_FALSE( CORE::DirExists( CORE::CombinePath( dir, "cred" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The private key of a new sso profile is kept in the credential store, not in the profile file, and is the same key on the next run" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The AEAD codec the credential store needs is not available" );
        try
        {
            CORE::CString dir = FreshDirectory( "keyNew" );
            CORE::CDataNode config;
            MakeSsoConfig( dir, config );
            CORE::CString profilePath = CORE::CombinePath( dir, "PlayerProfile.json" );
            MULTIPLAY::CPlayerIdentity first;
            {
                MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
                REQUIRE_TRUE( product->LoadConfig( config ) && product->GetIdentity( first ) );
                ASSERT_TRUE( first.HasKeyPair() && first.IsUserIdOfKey() );
                ASSERT_TRUE( AsSso( product )->GetLocalProvider().IsKeyInCredentialStore() );
            }
            CORE::CString profile = ReadText( profilePath );
            ASSERT_TRUE( profile.HasSubstr( first.userId ) >= 0 && profile.HasSubstr( first.publicKey ) >= 0 );
            ASSERT_TRUE( profile.HasSubstr( first.privateKey ) < 0 && profile.HasSubstr( "privateKey" ) < 0 );
            ASSERT_TRUE( profile.HasSubstr( "credentialStore" ) >= 0 );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( dir, "cred" ), AUTH::CFileCredentialStore::DataFileName ) ) );
            ASSERT_FALSE( FileHasBytes( CORE::CombinePath( CORE::CombinePath( dir, "cred" ), AUTH::CFileCredentialStore::DataFileName ), first.privateKey ) );

            // The next run has the same key, which signs
            MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
            MULTIPLAY::CPlayerIdentity second;
            REQUIRE_TRUE( product->LoadConfig( config ) && product->GetIdentity( second ) );
            ASSERT_TRUE( first.userId == second.userId && first.privateKey == second.privateKey && second.IsUserIdOfKey() );
            UInt8 signature[ MULTIPLAY::CPlayerIdentity::SignatureSize ];
            ASSERT_TRUE( second.Sign( "x", 1, signature ) );

            // The name of the player still lands in the profile file, the key stays out of it
            ASSERT_TRUE( product->SetDisplayName( "Alice" ) );
            profile = ReadText( profilePath );
            ASSERT_TRUE( profile.HasSubstr( "Alice" ) >= 0 && profile.HasSubstr( first.privateKey ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A profile that exists is never rewritten because the key could live in the store, only the option moves the key, and back" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The AEAD codec the credential store needs is not available" );
        try
        {
            CORE::CString dir = FreshDirectory( "keyMigrate" );
            CORE::CString profilePath = CORE::CombinePath( dir, "PlayerProfile.json" );
            MULTIPLAY::CPlayerIdentity original;
            REQUIRE_TRUE( MakePlainProfile( dir, original ) );
            CORE::CString before = ReadText( profilePath );

            // The sso provider with nothing said about the key: the file is as it was, byte for byte, the key is where it was
            {
                CORE::CDataNode config;
                MakeSsoConfig( dir, config );
                MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
                MULTIPLAY::CPlayerIdentity identity;
                REQUIRE_TRUE( product->LoadConfig( config ) && product->GetIdentity( identity ) );
                ASSERT_TRUE( original.userId == identity.userId && original.privateKey == identity.privateKey );
                ASSERT_FALSE( AsSso( product )->GetLocalProvider().IsKeyInCredentialStore() );
                ASSERT_TRUE( before == ReadText( profilePath ) );
                ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( dir, "cred" ), AUTH::CFileCredentialStore::DataFileName ) ) );
            }

            // The option says yes: the key moves, the identity is the same, nothing of the old file is left
            {
                CORE::CDataNode config;
                MakeSsoConfig( dir, config, "true" );
                MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
                MULTIPLAY::CPlayerIdentity identity;
                REQUIRE_TRUE( product->LoadConfig( config ) && product->GetIdentity( identity ) );
                ASSERT_TRUE( original.userId == identity.userId && original.privateKey == identity.privateKey );
                ASSERT_TRUE( AsSso( product )->GetLocalProvider().IsKeyInCredentialStore() );
                CORE::CString after = ReadText( profilePath );
                ASSERT_TRUE( after.HasSubstr( original.privateKey ) < 0 && after.HasSubstr( original.userId ) >= 0 && after.HasSubstr( "credentialStore" ) >= 0 );
                ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, "moving-PlayerProfile.json" ) ) );
                ASSERT_FALSE( FileHasBytes( CORE::CombinePath( CORE::CombinePath( dir, "cred" ), AUTH::CFileCredentialStore::DataFileName ), original.privateKey ) );
            }

            // Next run, no option said: the key is found in the store
            CORE::CString moved = ReadText( profilePath );
            {
                CORE::CDataNode config;
                MakeSsoConfig( dir, config );
                MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
                MULTIPLAY::CPlayerIdentity identity;
                REQUIRE_TRUE( product->LoadConfig( config ) && product->GetIdentity( identity ) );
                ASSERT_TRUE( original.userId == identity.userId && original.privateKey == identity.privateKey );
                ASSERT_TRUE( moved == ReadText( profilePath ) );
            }

            // The local provider has no store: it does not find the key, and does not make another identity for it
            {
                CORE::CDataNode config;
                MakeSsoConfig( dir, config );
                MULTIPLAY::CLocalIdentityProvider local;
                MULTIPLAY::CPlayerIdentity identity;
                REQUIRE_TRUE( local.LoadConfig( config ) && local.GetIdentity( identity ) );
                ASSERT_TRUE( original.userId == identity.userId && identity.privateKey.IsNULLOrEmpty() && !local.HasKey() );
                ASSERT_TRUE( moved == ReadText( profilePath ) );
            }

            // The option says no: the key is back in the profile file and gone from the store
            {
                CORE::CDataNode config;
                MakeSsoConfig( dir, config, "false" );
                MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
                MULTIPLAY::CPlayerIdentity identity;
                REQUIRE_TRUE( product->LoadConfig( config ) && product->GetIdentity( identity ) );
                ASSERT_TRUE( original.userId == identity.userId && original.privateKey == identity.privateKey );
                ASSERT_FALSE( AsSso( product )->GetLocalProvider().IsKeyInCredentialStore() );
                CORE::CString back = ReadText( profilePath );
                ASSERT_TRUE( back.HasSubstr( original.privateKey ) >= 0 && back.HasSubstr( "credentialStore" ) < 0 );
                AUTH::CFileCredentialStore store( CORE::CombinePath( dir, "cred" ) );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( MULTIPLAY::CLocalIdentityProvider::KeySlotName, original.userId, secret ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The option for the place of the key can be in the section Auth, for an application that keeps its settings there" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The AEAD codec the credential store needs is not available" );
        try
        {
            CORE::CString dir = FreshDirectory( "keyInAuth" );
            MULTIPLAY::CPlayerIdentity original;
            REQUIRE_TRUE( MakePlainProfile( dir, original ) );
            CORE::CDataNode config;
            MakeSsoConfig( dir, config );
            config.FindChild( "Auth" )->SetAttribute( "keyInCredentialStore", CORE::CString( "true" ) );
            MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
            MULTIPLAY::CPlayerIdentity identity;
            REQUIRE_TRUE( product->LoadConfig( config ) && product->GetIdentity( identity ) );
            ASSERT_TRUE( original.userId == identity.userId && original.privateKey == identity.privateKey );
            ASSERT_TRUE( AsSso( product )->GetLocalProvider().IsKeyInCredentialStore() );
            ASSERT_TRUE( ReadText( CORE::CombinePath( dir, "PlayerProfile.json" ) ).HasSubstr( original.privateKey ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A move of the key that does not verify leaves the profile file and the key as they were and nothing in the store" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CFlakyCredentialStore::EMode modes[ 2 ] = { CFlakyCredentialStore::MODE_REFUSES_STORE, CFlakyCredentialStore::MODE_WRONG_READBACK };
            for ( UInt32 m=0; m<2; ++m )
            {
                CORE::CString dir = FreshDirectory( "keyRollback" + CORE::ToString( m ) );
                CORE::CString profilePath = CORE::CombinePath( dir, "PlayerProfile.json" );
                MULTIPLAY::CPlayerIdentity original;
                REQUIRE_TRUE( MakePlainProfile( dir, original ) );
                CORE::CString before = ReadText( profilePath );

                CFlakyCredentialStore store( modes[ m ] );
                CORE::CDataNode config;
                MakeSsoConfig( dir, config );
                MULTIPLAY::CLocalIdentityProvider local;
                REQUIRE_TRUE( local.LoadConfig( config ) );
                local.SetCredentialStore( &store );
                local.SetKeyStorage( MULTIPLAY::CLocalIdentityProvider::KEYSTORAGE_CREDENTIAL_STORE );
                MULTIPLAY::CPlayerIdentity identity;
                REQUIRE_TRUE( local.GetIdentity( identity ) );
                ASSERT_TRUE( original.userId == identity.userId && original.privateKey == identity.privateKey );
                ASSERT_FALSE( local.IsKeyInCredentialStore() );
                ASSERT_FALSE( local.MigrateKeyToCredentialStore() );
                ASSERT_TRUE( before == ReadText( profilePath ) );
                ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, "moving-PlayerProfile.json" ) ) );
                CFlakyCredentialStore::EMode keep = store.mode;
                store.mode = CFlakyCredentialStore::MODE_WORKS;
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( MULTIPLAY::CLocalIdentityProvider::KeySlotName, original.userId, secret ) );
                store.mode = keep;
            }

            // The new profile cannot be written next to the old one: the same
            {
                CORE::CString dir = FreshDirectory( "keyRollbackWrite" );
                CORE::CString profilePath = CORE::CombinePath( dir, "PlayerProfile.json" );
                MULTIPLAY::CPlayerIdentity original;
                REQUIRE_TRUE( MakePlainProfile( dir, original ) );
                CORE::CString before = ReadText( profilePath );
                CORE::CString blocked = CORE::CombinePath( dir, "moving-PlayerProfile.json" );
                CORE::CreateDirs( blocked );
                REQUIRE_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( blocked, "keep" ), "x" ) );

                CFlakyCredentialStore store( CFlakyCredentialStore::MODE_WORKS );
                CORE::CDataNode config;
                MakeSsoConfig( dir, config );
                MULTIPLAY::CLocalIdentityProvider local;
                REQUIRE_TRUE( local.LoadConfig( config ) );
                local.SetCredentialStore( &store );
                local.SetKeyStorage( MULTIPLAY::CLocalIdentityProvider::KEYSTORAGE_CREDENTIAL_STORE );
                MULTIPLAY::CPlayerIdentity identity;
                REQUIRE_TRUE( local.GetIdentity( identity ) );
                ASSERT_FALSE( local.IsKeyInCredentialStore() );
                ASSERT_TRUE( before == ReadText( profilePath ) && original.privateKey == identity.privateKey );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( MULTIPLAY::CLocalIdentityProvider::KeySlotName, original.userId, secret ) );
            }

            // A local provider told to use the store when it has none keeps the key where it is
            {
                CORE::CString dir = FreshDirectory( "keyNoStore" );
                MULTIPLAY::CPlayerIdentity original;
                REQUIRE_TRUE( MakePlainProfile( dir, original ) );
                CORE::CString before = ReadText( CORE::CombinePath( dir, "PlayerProfile.json" ) );
                CORE::CDataNode config;
                MakeSsoConfig( dir, config, "true" );
                MULTIPLAY::CLocalIdentityProvider local;
                MULTIPLAY::CPlayerIdentity identity;
                REQUIRE_TRUE( local.LoadConfig( config ) && local.GetIdentity( identity ) );
                ASSERT_TRUE( original.privateKey == identity.privateKey && before == ReadText( CORE::CombinePath( dir, "PlayerProfile.json" ) ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A key that is gone from the store is not replaced by a new identity" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The AEAD codec the credential store needs is not available" );
        try
        {
            CORE::CString dir = FreshDirectory( "keyLost" );
            CORE::CString profilePath = CORE::CombinePath( dir, "PlayerProfile.json" );
            CORE::CDataNode config;
            MakeSsoConfig( dir, config );
            MULTIPLAY::CPlayerIdentity first;
            {
                MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
                REQUIRE_TRUE( product->LoadConfig( config ) && product->GetIdentity( first ) );
                ASSERT_TRUE( AsSso( product )->GetLocalProvider().IsKeyInCredentialStore() );
            }
            CORE::CString before = ReadText( profilePath );
            {
                AUTH::CFileCredentialStore store( CORE::CombinePath( dir, "cred" ) );
                REQUIRE_TRUE( store.Wipe() );
            }
            MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
            MULTIPLAY::CPlayerIdentity second;
            REQUIRE_TRUE( product->LoadConfig( config ) && product->GetIdentity( second ) );
            ASSERT_TRUE( first.userId == second.userId && first.publicKey == second.publicKey );
            ASSERT_TRUE( second.privateKey.IsNULLOrEmpty() && !second.HasKeyPair() );
            ASSERT_FALSE( AsSso( product )->GetLocalProvider().HasKey() );
            ASSERT_TRUE( before == ReadText( profilePath ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The settings of the manager are saved with the last used providers when a login changes them" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString dir = FreshDirectory( "ssoSettings" );
            CORE::CDataNode config;
            MakeSsoConfig( dir, config );
            MULTIPLAY::CIIdentityProviderPtr product = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "sso" );
            REQUIRE_TRUE( product->LoadConfig( config ) );
            MULTIPLAY::CAuthIdentityProvider& provider = *AsSso( product );
            CSsoRecorder recorder;
            recorder.SubscribeTo( &provider );
            REQUIRE_TRUE( GUCEF_NULL != AddFake( provider.GetAuthManager(), "fakesso", "fake", "player-1", "Fake Player" ) );
            ASSERT_TRUE( LoginWith( provider, "fakesso" ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.settingsChanges, 1, 2000 ) );

            CORE::CDataNode saved( "Auth" );
            ASSERT_TRUE( provider.SaveAuthConfig( saved ) );
            CORE::CDataNode wrapper( "Settings" );
            ASSERT_TRUE( product->SaveConfig( wrapper ) && GUCEF_NULL != wrapper.FindChild( "Auth" ) );
            ASSERT_TRUE( wrapper.GetAttributeValueOrChildValueByName( "defaultDisplayName" ).AsString() == "Newcomer" );
            const CORE::CDataNode* lastUsed = saved.FindChild( "lastUsedProviders" );
            REQUIRE_TRUE( GUCEF_NULL != lastUsed );
            CORE::CDataNode::TVariantVector lastUsedNames = lastUsed->GetChildrenValues();
            ASSERT_TRUE( 1 == lastUsedNames.size() && "fakesso" == lastUsedNames[ 0 ].AsString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
