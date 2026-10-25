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

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CLocalIdentityProvider.h"
#define GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestIdentity.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Counts identity changes */
class CIdentityRecorder : public CORE::CObserver
{
    public:

    UInt32 changes;
    MULTIPLAY::CPlayerIdentity last;

    CIdentityRecorder( void ) : CORE::CObserver(), changes( 0 ), last() {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CIdentityRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CIIdentityProvider::IdentityChangedEvent == eventId )
        {
            ++changes;
            last = *static_cast< const MULTIPLAY::CPlayerIdentity* >( eventData );
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformIdentityTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING identity TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Identity" );

    GUCEF_TESTFW_TESTCASE( "Test: A user id is a version 4 GUID, different every time" )
        try
        {
            CORE::CString a = MULTIPLAY::CPlayerIdentity::MakeGuid();
            CORE::CString b = MULTIPLAY::CPlayerIdentity::MakeGuid();
            ASSERT_TRUE( 36 == a.Length() && a != b );
            ASSERT_TRUE( '-' == a.C_String()[ 8 ] && '4' == a.C_String()[ 14 ] );
            char variant = a.C_String()[ 19 ];
            ASSERT_TRUE( '8' == variant || '9' == variant || 'a' == variant || 'b' == variant );

            MULTIPLAY::CPlayerIdentity identity;
            identity.userId = a;
            identity.displayName = "Alice";
            identity.accounts.Set( "steam", "7656" );
            CORE::CDataNode tree;
            ASSERT_TRUE( identity.Serialize( tree, CORE::CDataNodeSerializableSettings() ) );
            MULTIPLAY::CPlayerIdentity copy;
            ASSERT_TRUE( copy.Deserialize( tree, CORE::CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( a == copy.userId && "Alice" == copy.displayName && "7656" == copy.accounts.GetValueAlways( "steam" ).AsString() );
            ASSERT_FALSE( MULTIPLAY::CPlayerIdentity().Deserialize( CORE::CDataNode(), CORE::CDataNodeSerializableSettings() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The local profile is made once and kept" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" );
            CORE::CString profilePath = CORE::CombinePath( CORE::CombinePath( workDir, "profile" ), "PlayerProfile.json" );
            CORE::DeleteFile( profilePath );

            CORE::CDataNode config;
            config.SetAttribute( "profilePath", profilePath );
            config.SetAttribute( "defaultDisplayName", CORE::CString( "Newcomer" ) );

            CORE::CString userId;
            {
                MULTIPLAY::CIIdentityProviderPtr provider = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "local" );
                REQUIRE_TRUE( !provider.IsNULL() );
                ASSERT_TRUE( provider->LoadConfig( config ) );
                MULTIPLAY::CPlayerIdentity identity;
                ASSERT_TRUE( provider->GetIdentity( identity ) );
                ASSERT_TRUE( 36 == identity.userId.Length() && "Newcomer" == identity.displayName && identity.IsUserIdOfKey() );
                ASSERT_TRUE( CORE::FileExists( profilePath ) );
                userId = identity.userId;

                CIdentityRecorder recorder;
                recorder.SubscribeTo( provider.GetPointerAlways() );
                ASSERT_TRUE( provider->SetDisplayName( "Alice" ) );
                ASSERT_TRUE( 1 == recorder.changes && "Alice" == recorder.last.displayName && userId == recorder.last.userId );
                ASSERT_FALSE( provider->SetDisplayName( CORE::CString::Empty ) );
                ASSERT_TRUE( static_cast< MULTIPLAY::CLocalIdentityProvider* >( provider.GetPointerAlways() )->SetAccount( "irc", "alice" ) );
            }

            // Another run: the same identity
            MULTIPLAY::CLocalIdentityProvider again;
            ASSERT_TRUE( again.LoadConfig( config ) );
            MULTIPLAY::CPlayerIdentity identity;
            ASSERT_TRUE( again.GetIdentity( identity ) );
            ASSERT_TRUE( userId == identity.userId && "Alice" == identity.displayName );
            ASSERT_TRUE( "alice" == identity.accounts.GetValueAlways( "irc" ).AsString() );
            ASSERT_TRUE( profilePath == again.GetProfilePath() );
            ASSERT_TRUE( identity.HasKeyPair() && identity.IsUserIdOfKey() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An identity provider is the profile of an account page: name, id, changes" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" );
            CORE::CString profilePath = CORE::CombinePath( CORE::CombinePath( workDir, "accountprofile" ), "PlayerProfile.json" );
            CORE::DeleteFile( profilePath );
            CORE::CDataNode config;
            config.SetAttribute( "profilePath", profilePath );
            config.SetAttribute( "defaultDisplayName", CORE::CString( "Newcomer" ) );

            MULTIPLAY::CIIdentityProviderPtr provider = MULTIPLAY::CMultiplayGlobal::Instance()->GetIdentityProviderFactory().Create( "local" );
            REQUIRE_TRUE( !provider.IsNULL() );
            ASSERT_TRUE( provider->LoadConfig( config ) );
            AUTH::IAccountProfile& profile = *provider.GetPointerAlways();
            MULTIPLAY::CPlayerIdentity identity;
            REQUIRE_TRUE( provider->GetIdentity( identity ) );
            ASSERT_TRUE( "Newcomer" == profile.GetDisplayName() && identity.userId == profile.GetProfileId() );
            ASSERT_TRUE( GUCEF_NULL != profile.GetProfileNotifier() && MULTIPLAY::CIIdentityProvider::IdentityChangedEvent == profile.GetProfileChangedEvent() );

            CIdentityRecorder recorder;
            recorder.SubscribeTo( profile.GetProfileNotifier() );
            ASSERT_TRUE( profile.SetDisplayName( "Bob" ) && "Bob" == profile.GetDisplayName() && 1 == recorder.changes );
            ASSERT_FALSE( profile.SetDisplayName( CORE::CString::Empty ) );
            ASSERT_TRUE( "Bob" == profile.GetDisplayName() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An identity has an Ed25519 key pair, its user id is the one of the key and the private key stays in the profile" )
        try
        {
            REQUIRE_TRUE( MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable() );
            MULTIPLAY::CPlayerIdentity alice;
            MULTIPLAY::CPlayerIdentity bob;
            REQUIRE_TRUE( alice.GenerateKeyPair() && bob.GenerateKeyPair() );
            ASSERT_TRUE( 64 == alice.publicKey.Length() && 64 == alice.privateKey.Length() && 36 == alice.userId.Length() );
            ASSERT_TRUE( alice.userId != bob.userId && alice.IsUserIdOfKey() );
            ASSERT_TRUE( alice.userId == MULTIPLAY::CPlayerIdentity::MakeUserIdFromHex( alice.publicKey ) );

            // The same private key gives the same identity
            MULTIPLAY::CPlayerIdentity restored;
            ASSERT_TRUE( restored.SetPrivateKey( alice.privateKey ) );
            ASSERT_TRUE( alice.userId == restored.userId && alice.publicKey == restored.publicKey );
            ASSERT_FALSE( restored.SetPrivateKey( "not a key" ) );

            // Signatures of the key
            UInt8 signature[ MULTIPLAY::CPlayerIdentity::SignatureSize ];
            UInt8 publicKey[ MULTIPLAY::CPlayerIdentity::KeySize ];
            ASSERT_TRUE( alice.Sign( "hello", 5, signature ) && alice.GetPublicKey( publicKey ) );
            ASSERT_TRUE( DRN::CDRNCrypto::Verify( publicKey, "hello", 5, signature ) );
            ASSERT_FALSE( DRN::CDRNCrypto::Verify( publicKey, "hellp", 5, signature ) );

            // What is passed on has no private key, a user id that is not the key's is not the key's
            CORE::CDataNode tree;
            ASSERT_TRUE( alice.Serialize( tree, CORE::CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( tree.GetAttributeValueOrChildValueByName( "privateKey" ).AsString( CORE::CString::Empty, true ).IsNULLOrEmpty() );
            MULTIPLAY::CPlayerIdentity copy;
            ASSERT_TRUE( copy.Deserialize( tree, CORE::CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( alice.userId == copy.userId && alice.publicKey == copy.publicKey && !copy.HasKeyPair() );
            ASSERT_TRUE( alice.GetPublicIdentity().privateKey.IsNULLOrEmpty() );
            copy.userId = bob.userId;
            ASSERT_FALSE( copy.IsUserIdOfKey() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A profile from before identities had keys gets a key pair when it is loaded and keeps its old user id" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" );
            CORE::CString profilePath = CORE::CombinePath( CORE::CombinePath( workDir, "profile" ), "OldProfile.json" );
            CORE::CreateDirs( CORE::StripFilename( profilePath ) );
            CORE::CString oldUserId = MULTIPLAY::CPlayerIdentity::MakeGuid();
            REQUIRE_TRUE( CORE::WriteStringAsTextFile( profilePath, "{ \"userId\": \"" + oldUserId + "\", \"displayName\": \"Veteran\" }" ) );

            CORE::CDataNode config;
            config.SetAttribute( "profilePath", profilePath );
            MULTIPLAY::CPlayerIdentity upgraded;
            {
                MULTIPLAY::CLocalIdentityProvider provider;
                ASSERT_TRUE( provider.LoadConfig( config ) );
                ASSERT_TRUE( provider.GetIdentity( upgraded ) );
            }
            ASSERT_TRUE( upgraded.HasKeyPair() && upgraded.IsUserIdOfKey() );
            ASSERT_TRUE( oldUserId == upgraded.previousUserId && oldUserId != upgraded.userId && "Veteran" == upgraded.displayName );

            // Kept: the next run has the same key
            MULTIPLAY::CLocalIdentityProvider again;
            ASSERT_TRUE( again.LoadConfig( config ) );
            MULTIPLAY::CPlayerIdentity identity;
            ASSERT_TRUE( again.GetIdentity( identity ) );
            ASSERT_TRUE( upgraded.userId == identity.userId && upgraded.privateKey == identity.privateKey && oldUserId == identity.previousUserId );
            CORE::DeleteFile( profilePath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A profile that still has the name earlier versions gave every profile gets the default name once, and everything else of the profile stays, also what this version does not know" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" );
            CORE::CString dir = CORE::CombinePath( workDir, "legacyname" );
            CORE::DeleteDir( dir, true );
            CORE::CString profilePath = CORE::CombinePath( dir, "PlayerProfile.json" );

            // the profile an earlier version made: named after the machine, with an account, and a value only a newer version writes
            CORE::CDataNode legacyConfig;
            legacyConfig.SetAttribute( "profilePath", profilePath );
            legacyConfig.SetAttribute( "defaultDisplayName", CORE::CString( "MY MACHINE" ) );
            MULTIPLAY::CPlayerIdentity original;
            {
                MULTIPLAY::CLocalIdentityProvider provider;
                REQUIRE_TRUE( provider.LoadConfig( legacyConfig ) && provider.GetIdentity( original ) );
                REQUIRE_TRUE( provider.SetAccount( "irc", "alice" ) );
                REQUIRE_TRUE( provider.GetIdentity( original ) );
            }
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            REQUIRE_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL() );
            {
                CORE::CDataNode tree;
                REQUIRE_TRUE( codec->BuildDataTree( &tree, profilePath ) );
                REQUIRE_TRUE( tree.SetAttribute( "futureValue", CORE::CString( "keep-me" ) ) );
                REQUIRE_TRUE( codec->StoreDataTree( &tree, profilePath ) );
            }
            CORE::CString originalBytes;
            REQUIRE_TRUE( CORE::LoadTextFileAsString( profilePath, originalBytes, false ) && originalBytes.HasSubstr( "MY MACHINE" ) >= 0 );

            // with no name to replace it with nothing changes; neither when it is the name itself
            for ( UInt32 variant=0; variant<3; ++variant )
            {
                CORE::CDataNode config;
                config.SetAttribute( "profilePath", profilePath );
                config.SetAttribute( "defaultDisplayName", CORE::CString( 2 == variant ? "MY MACHINE" : "Dinand" ) );
                if ( 0 != variant )
                    config.SetAttribute( "legacyDefaultDisplayName", CORE::CString( 1 == variant ? "" : "MY MACHINE" ) );
                MULTIPLAY::CLocalIdentityProvider provider;
                MULTIPLAY::CPlayerIdentity identity;
                ASSERT_TRUE( provider.LoadConfig( config ) && provider.GetIdentity( identity ) && "MY MACHINE" == identity.displayName );
                CORE::CString bytes;
                ASSERT_TRUE( CORE::LoadTextFileAsString( profilePath, bytes, false ) && bytes == originalBytes );
            }

            // the name earlier versions gave: the default name, in the identity and in the file, once
            CORE::CDataNode config;
            config.SetAttribute( "profilePath", profilePath );
            config.SetAttribute( "defaultDisplayName", CORE::CString( "Dinand" ) );
            config.SetAttribute( "legacyDefaultDisplayName", CORE::CString( "MY MACHINE" ) );
            {
                MULTIPLAY::CLocalIdentityProvider provider;
                MULTIPLAY::CPlayerIdentity identity;
                ASSERT_TRUE( provider.LoadConfig( config ) && provider.GetIdentity( identity ) );
                ASSERT_TRUE( "Dinand" == identity.displayName );
                ASSERT_TRUE( identity.userId == original.userId && identity.publicKey == original.publicKey && identity.privateKey == original.privateKey );
                ASSERT_TRUE( "alice" == identity.accounts.GetValueAlways( "irc" ).AsString() && identity.HasKeyPair() && identity.IsUserIdOfKey() );
            }
            CORE::CDataNode after;
            ASSERT_TRUE( codec->BuildDataTree( &after, profilePath ) );
            ASSERT_TRUE( "Dinand" == after.GetAttributeValueOrChildValueByName( "displayName" ).AsString() );
            ASSERT_TRUE( "keep-me" == after.GetAttributeValueOrChildValueByName( "futureValue" ).AsString() );
            ASSERT_TRUE( original.userId == after.GetAttributeValueOrChildValueByName( "userId" ).AsString() && original.privateKey == after.GetAttributeValueOrChildValueByName( "privateKey" ).AsString() );
            ASSERT_TRUE( original.publicKey == after.GetAttributeValueOrChildValueByName( "publicKey" ).AsString() );
            // next to the profile there is nothing left over from the replacement
            CORE::CString::StringVector names;
            CORE::CFileSystemIterator iterator;
            if ( iterator.FindFirst( dir ) )
            {
                do { names.push_back( iterator.GetResourceName() ); } while ( iterator.FindNext() );
                iterator.FindClose();
            }
            ASSERT_TRUE( 1 == names.size() && "PlayerProfile.json" == names[ 0 ] );

            // once: the next start leaves the file as it is
            CORE::CString replacedBytes;
            ASSERT_TRUE( CORE::LoadTextFileAsString( profilePath, replacedBytes, false ) );
            {
                MULTIPLAY::CLocalIdentityProvider provider;
                MULTIPLAY::CPlayerIdentity identity;
                ASSERT_TRUE( provider.LoadConfig( config ) && provider.GetIdentity( identity ) && "Dinand" == identity.displayName );
            }
            CORE::CString againBytes;
            ASSERT_TRUE( CORE::LoadTextFileAsString( profilePath, againBytes, false ) && againBytes == replacedBytes );

            // the config of the provider has the name
            CORE::CDataNode saved;
            MULTIPLAY::CLocalIdentityProvider saver;
            ASSERT_TRUE( saver.LoadConfig( config ) && saver.SaveConfig( saved ) );
            ASSERT_TRUE( "MY MACHINE" == saved.GetAttributeValueOrChildValueByName( "legacyDefaultDisplayName" ).AsString() );

            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A name the player chose is never replaced, not in any other case than exactly the name earlier versions gave" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" );
            CORE::CString dir = CORE::CombinePath( workDir, "chosenname" );
            CORE::DeleteDir( dir, true );

            const char* const names[] = { "Chosen", "my machine", "MY MACHINE ", " MY MACHINE", "MY  MACHINE", "Dinand", "MY MACHINE 2" };
            for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            {
                CORE::CString profilePath = CORE::CombinePath( dir, "p" + CORE::UInt32ToString( i ) + ".json" );
                CORE::CDataNode config;
                config.SetAttribute( "profilePath", profilePath );
                config.SetAttribute( "defaultDisplayName", CORE::CString( names[ i ] ) );
                MULTIPLAY::CPlayerIdentity first;
                {
                    MULTIPLAY::CLocalIdentityProvider provider;
                    ASSERT_TRUE( provider.LoadConfig( config ) && provider.GetIdentity( first ) && names[ i ] == first.displayName );
                }
                CORE::CString bytes;
                ASSERT_TRUE( CORE::LoadTextFileAsString( profilePath, bytes, false ) );

                CORE::CDataNode withLegacy;
                withLegacy.SetAttribute( "profilePath", profilePath );
                withLegacy.SetAttribute( "defaultDisplayName", CORE::CString( "Dinand" ) );
                withLegacy.SetAttribute( "legacyDefaultDisplayName", CORE::CString( "MY MACHINE" ) );
                MULTIPLAY::CLocalIdentityProvider provider;
                MULTIPLAY::CPlayerIdentity second;
                ASSERT_TRUE( provider.LoadConfig( withLegacy ) && provider.GetIdentity( second ) );
                ASSERT_TRUE( names[ i ] == second.displayName && first.userId == second.userId );
                CORE::CString afterBytes;
                ASSERT_TRUE( CORE::LoadTextFileAsString( profilePath, afterBytes, false ) && afterBytes == bytes );
            }
            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A profile whose key is in the credential store is renamed too, with the key where it was; a rewrite that cannot be made leaves the profile as it was and is tried again" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" );
            CORE::CString dir = CORE::CombinePath( workDir, "legacystore" );
            CORE::DeleteDir( dir, true );
            CORE::CString profilePath = CORE::CombinePath( dir, "PlayerProfile.json" );
            AUTH::CMemoryCredentialStore store;

            CORE::CDataNode legacyConfig;
            legacyConfig.SetAttribute( "profilePath", profilePath );
            legacyConfig.SetAttribute( "defaultDisplayName", CORE::CString( "MY MACHINE" ) );
            MULTIPLAY::CPlayerIdentity original;
            {
                MULTIPLAY::CLocalIdentityProvider provider;
                provider.SetCredentialStore( &store );
                REQUIRE_TRUE( provider.LoadConfig( legacyConfig ) && provider.GetIdentity( original ) && provider.IsKeyInCredentialStore() );
            }
            CORE::CString originalBytes;
            REQUIRE_TRUE( CORE::LoadTextFileAsString( profilePath, originalBytes, false ) && originalBytes.HasSubstr( "privateKey" ) < 0 );

            CORE::CDataNode config;
            config.SetAttribute( "profilePath", profilePath );
            config.SetAttribute( "defaultDisplayName", CORE::CString( "Dinand" ) );
            config.SetAttribute( "legacyDefaultDisplayName", CORE::CString( "MY MACHINE" ) );

            // a rewrite that cannot be made: what the new profile is written to is a folder
            const CORE::CString blocker = CORE::CombinePath( dir, "moving-PlayerProfile.json" );
            REQUIRE_TRUE( CORE::CreateDirs( blocker ) && CORE::WriteStringAsTextFile( CORE::CombinePath( blocker, "keep.txt" ), "not empty" ) );
            {
                MULTIPLAY::CLocalIdentityProvider provider;
                provider.SetCredentialStore( &store );
                MULTIPLAY::CPlayerIdentity identity;
                ASSERT_TRUE( provider.LoadConfig( config ) && provider.GetIdentity( identity ) );
                // the name is shown, the file is as it was
                ASSERT_TRUE( "Dinand" == identity.displayName && identity.userId == original.userId && identity.privateKey == original.privateKey );
                CORE::CString bytes;
                ASSERT_TRUE( CORE::LoadTextFileAsString( profilePath, bytes, false ) && bytes == originalBytes );
            }
            CORE::DeleteDir( blocker, true );

            // the next start does it, and the key stays in the store
            {
                MULTIPLAY::CLocalIdentityProvider provider;
                provider.SetCredentialStore( &store );
                MULTIPLAY::CPlayerIdentity identity;
                ASSERT_TRUE( provider.LoadConfig( config ) && provider.GetIdentity( identity ) );
                ASSERT_TRUE( "Dinand" == identity.displayName && identity.userId == original.userId && identity.privateKey == original.privateKey );
                ASSERT_TRUE( provider.IsKeyInCredentialStore() && provider.HasKey() );
            }
            CORE::CString bytes;
            ASSERT_TRUE( CORE::LoadTextFileAsString( profilePath, bytes, false ) && bytes.HasSubstr( "MY MACHINE" ) < 0 && bytes.HasSubstr( "Dinand" ) >= 0 && bytes.HasSubstr( "privateKey" ) < 0 && bytes.HasSubstr( "credentialStore" ) >= 0 );
            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
