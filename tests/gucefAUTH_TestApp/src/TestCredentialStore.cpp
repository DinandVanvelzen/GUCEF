/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestLogCapture.h"
#include "TestFiles.h"
#include "TestCredentialStore.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What every credential store has to do, for the memory store and the file store alike */
static void
ExerciseStore( AUTH::ICredentialStore& store )
{GUCEF_TRACE;

    AUTH::CSecretString secret;
    ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
    CORE::CString::StringVector accounts;
    ASSERT_TRUE( store.ListAccounts( "google", accounts ) && accounts.empty() );

    ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
    ASSERT_TRUE( store.Store( "google", "google:2", AUTH::CSecretString( "refresh-two" ) ) );
    ASSERT_TRUE( store.Store( "testopenid", "epic:9", AUTH::CSecretString( "ticket-nine" ) ) );
    ASSERT_TRUE( store.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
    ASSERT_TRUE( store.Load( "google", "google:2", secret ) && "refresh-two" == secret.Reveal() );
    // a slot is per provider and account
    ASSERT_FALSE( store.Load( "testopenid", "google:1", secret ) );
    ASSERT_FALSE( store.Load( "google", "epic:9", secret ) );

    // the slot stored last comes first
    ASSERT_TRUE( store.ListAccounts( "google", accounts ) && 2 == accounts.size() && "google:2" == accounts[ 0 ] && "google:1" == accounts[ 1 ] );

    // storing again replaces and makes it the newest
    ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one-rotated" ) ) );
    ASSERT_TRUE( store.Load( "google", "google:1", secret ) && "refresh-one-rotated" == secret.Reveal() );
    ASSERT_TRUE( store.ListAccounts( "google", accounts ) && 2 == accounts.size() && "google:1" == accounts[ 0 ] );

    ASSERT_FALSE( store.Store( CORE::CString::Empty, "google:1", AUTH::CSecretString( "x" ) ) );
    ASSERT_FALSE( store.Store( "google", CORE::CString::Empty, AUTH::CSecretString( "x" ) ) );

    ASSERT_TRUE( store.Remove( "google", "google:2" ) );
    ASSERT_FALSE( store.Load( "google", "google:2", secret ) );
    ASSERT_TRUE( store.Remove( "google", "google:2" ) );   // already gone is fine
    ASSERT_TRUE( store.Load( "google", "google:1", secret ) );

    ASSERT_TRUE( store.RemoveProvider( "google" ) );
    ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
    ASSERT_TRUE( store.Load( "testopenid", "epic:9", secret ) && "ticket-nine" == secret.Reveal() );

    ASSERT_TRUE( store.Wipe() );
    ASSERT_FALSE( store.Load( "testopenid", "epic:9", secret ) );
    ASSERT_TRUE( store.ListAccounts( "testopenid", accounts ) && accounts.empty() );
    // and it works again after a wipe
    ASSERT_TRUE( store.Store( "google", "google:3", AUTH::CSecretString( "after-wipe" ) ) );
    ASSERT_TRUE( store.Load( "google", "google:3", secret ) && "after-wipe" == secret.Reveal() );
}

/*-------------------------------------------------------------------------*/

void
PerformCredentialStoreTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING credential store TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "CredentialStore" );

    GUCEF_TESTFW_TESTCASE( "Test: The memory store keeps slots per provider and account" )
        try
        {
            AUTH::CMemoryCredentialStore store;
            ExerciseStore( store );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The file store keeps slots per provider and account" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "cred_basic" );
            AUTH::CFileCredentialStore store( dir );
            ExerciseStore( store );
            ASSERT_FALSE( store.IsTamperDetected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The file store writes nothing in the clear and survives a restart" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "cred_restart" );
            CORE::CString dataPath = CORE::CombinePath( dir, AUTH::CFileCredentialStore::DataFileName );
            CORE::CString keyPath = CORE::CombinePath( dir, AUTH::CFileCredentialStore::KeyFileName );
            {
                AUTH::CFileCredentialStore store( dir );
                ASSERT_TRUE( store.Store( "google", "google:alice-12345", AUTH::CSecretString( "REFRESH-SECRET-ALICE-0001" ) ) );
                ASSERT_TRUE( store.Store( "microsoft", "microsoft:tenant:bob-678", AUTH::CSecretString( "REFRESH-SECRET-BOB-0002" ) ) );
                ASSERT_TRUE( CORE::FileExists( dataPath ) && CORE::FileExists( keyPath ) );
                ASSERT_TRUE( AUTH::CAuthCrypto::KeySize == CORE::FileSize( keyPath ) );
            }

            // neither the secrets nor the names of the slots are in the file
            ASSERT_FALSE( FileContains( dataPath, "REFRESH-SECRET" ) );
            ASSERT_FALSE( FileContains( dataPath, "alice-12345" ) );
            ASSERT_FALSE( FileContains( dataPath, "google" ) );
            ASSERT_FALSE( FileContains( dataPath, "microsoft" ) );
            ASSERT_TRUE( FileContains( dataPath, "GAUC" ) );

            // the directory holds the two files and no leftovers of the atomic writes
            CORE::CString::StringVector names = ListDirectory( dir );
            ASSERT_TRUE( 2 == names.size() && HasName( names, AUTH::CFileCredentialStore::DataFileName ) && HasName( names, AUTH::CFileCredentialStore::KeyFileName ) );

            // another run reads it
            AUTH::CFileCredentialStore again( dir );
            AUTH::CSecretString secret;
            ASSERT_TRUE( again.Load( "google", "google:alice-12345", secret ) && "REFRESH-SECRET-ALICE-0001" == secret.Reveal() );
            ASSERT_TRUE( again.Load( "microsoft", "microsoft:tenant:bob-678", secret ) && "REFRESH-SECRET-BOB-0002" == secret.Reveal() );
            CORE::CString::StringVector accounts;
            ASSERT_TRUE( again.ListAccounts( "microsoft", accounts ) && 1 == accounts.size() && "microsoft:tenant:bob-678" == accounts[ 0 ] );
            ASSERT_FALSE( again.IsTamperDetected() );

            // the order of storing survives too
            ASSERT_TRUE( again.Store( "google", "google:carol-9", AUTH::CSecretString( "c" ) ) );
            AUTH::CFileCredentialStore third( dir );
            ASSERT_TRUE( third.ListAccounts( "google", accounts ) && 2 == accounts.size() && "google:carol-9" == accounts[ 0 ] && "google:alice-12345" == accounts[ 1 ] );

            // every write is sealed with a new nonce: the same content gives other bytes
            CORE::CDynamicBuffer first;
            first.LoadContentFromFile( dataPath );
            ASSERT_TRUE( third.Store( "google", "google:carol-9", AUTH::CSecretString( "c" ) ) );
            CORE::CDynamicBuffer second;
            second.LoadContentFromFile( dataPath );
            ASSERT_TRUE( first.GetDataSize() == second.GetDataSize() && 0 != memcmp( first.GetConstBufferPtr(), second.GetConstBufferPtr(), first.GetDataSize() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The key file and the directory are for the owner only" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_LINUX )
        GUCEF_TESTFW_SKIP( "The permission bits are checked on Linux" );
        #endif
        try
        {
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            CORE::CString dir = MakeTestDirectory( "cred_perms" );
            AUTH::CFileCredentialStore store( dir );
            ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "x" ) ) );
            struct stat info;
            ASSERT_TRUE( 0 == stat( dir.C_String(), &info ) && 0700 == ( info.st_mode & 0777 ) );
            ASSERT_TRUE( 0 == stat( CORE::CombinePath( dir, AUTH::CFileCredentialStore::KeyFileName ).C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );
            ASSERT_TRUE( 0 == stat( CORE::CombinePath( dir, AUTH::CFileCredentialStore::DataFileName ).C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );

            // writing again keeps it so
            ASSERT_TRUE( store.Store( "google", "google:2", AUTH::CSecretString( "y" ) ) );
            ASSERT_TRUE( 0 == stat( CORE::CombinePath( dir, AUTH::CFileCredentialStore::DataFileName ).C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A changed credential file is detected and refused" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "cred_tamper" );
            CORE::CString dataPath = CORE::CombinePath( dir, AUTH::CFileCredentialStore::DataFileName );
            {
                AUTH::CFileCredentialStore store( dir );
                ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            }
            UInt32 size = (UInt32) CORE::FileSize( dataPath );
            ASSERT_TRUE( size > 5 + 24 + 16 );

            // a bit flipped in the header, the nonce, the ciphertext and the tag: each is noticed
            UInt32 offsets[] = { 0, 4, 5, 5 + 24, size - 17, size - 1 };
            for ( size_t i=0; i<sizeof( offsets ) / sizeof( offsets[ 0 ] ); ++i )
            {
                CORE::CString copyDir = MakeTestDirectory( "cred_tamper_copy" );
                CORE::CreateDirs( copyDir );
                CORE::CopyFile( dataPath, CORE::CombinePath( copyDir, AUTH::CFileCredentialStore::DataFileName ), true );
                CORE::CopyFile( CORE::CombinePath( dir, AUTH::CFileCredentialStore::KeyFileName ), CORE::CombinePath( copyDir, AUTH::CFileCredentialStore::KeyFileName ), true );
                ASSERT_TRUE( FlipFileByte( CORE::CombinePath( copyDir, AUTH::CFileCredentialStore::DataFileName ), offsets[ i ] ) );

                AUTH::CFileCredentialStore tampered( copyDir );
                AUTH::CSecretString secret;
                CLogCapture capture;
                ASSERT_FALSE( tampered.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( tampered.IsTamperDetected() );
                ASSERT_TRUE( secret.IsEmpty() );
                ASSERT_FALSE( capture.Contains( "refresh-one" ) );
            }

            // a file cut short
            {
                CORE::CString copyDir = MakeTestDirectory( "cred_tamper_cut" );
                CORE::CreateDirs( copyDir );
                CORE::CDynamicBuffer content;
                content.LoadContentFromFile( dataPath );
                CORE::WriteFileAtomically( CORE::CombinePath( copyDir, AUTH::CFileCredentialStore::DataFileName ), content.GetConstBufferPtr(), size - 3 );
                CORE::CopyFile( CORE::CombinePath( dir, AUTH::CFileCredentialStore::KeyFileName ), CORE::CombinePath( copyDir, AUTH::CFileCredentialStore::KeyFileName ), true );
                AUTH::CFileCredentialStore cut( copyDir );
                AUTH::CSecretString secret;
                ASSERT_FALSE( cut.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( cut.IsTamperDetected() );
            }

            // the data of another key
            {
                CORE::CString otherDir = MakeTestDirectory( "cred_tamper_otherkey" );
                AUTH::CFileCredentialStore other( otherDir );
                ASSERT_TRUE( other.Store( "google", "google:9", AUTH::CSecretString( "other" ) ) );
                CORE::CopyFile( dataPath, CORE::CombinePath( otherDir, AUTH::CFileCredentialStore::DataFileName ), true );
                AUTH::CFileCredentialStore mixed( otherDir );
                AUTH::CSecretString secret;
                ASSERT_FALSE( mixed.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( mixed.IsTamperDetected() );
            }

            // the key gone: the data cannot be read
            {
                CORE::CString lostDir = MakeTestDirectory( "cred_tamper_nokey" );
                CORE::CreateDirs( lostDir );
                CORE::CopyFile( dataPath, CORE::CombinePath( lostDir, AUTH::CFileCredentialStore::DataFileName ), true );
                AUTH::CFileCredentialStore lost( lostDir );
                AUTH::CSecretString secret;
                ASSERT_FALSE( lost.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( lost.IsTamperDetected() );
            }

            // storing after a detection starts over with a good file
            {
                CORE::CString againDir = MakeTestDirectory( "cred_tamper_recover" );
                CORE::CreateDirs( againDir );
                CORE::CopyFile( dataPath, CORE::CombinePath( againDir, AUTH::CFileCredentialStore::DataFileName ), true );
                FlipFileByte( CORE::CombinePath( againDir, AUTH::CFileCredentialStore::DataFileName ), 40 );
                AUTH::CFileCredentialStore recover( againDir );
                AUTH::CSecretString secret;
                ASSERT_FALSE( recover.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( recover.IsTamperDetected() );
                ASSERT_TRUE( recover.Store( "google", "google:2", AUTH::CSecretString( "fresh" ) ) );
                ASSERT_FALSE( recover.IsTamperDetected() );
                AUTH::CFileCredentialStore reopened( againDir );
                ASSERT_TRUE( reopened.Load( "google", "google:2", secret ) && "fresh" == secret.Reveal() && !reopened.IsTamperDetected() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A credential file of a newer version is left alone" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "cred_newer" );
            CORE::CString dataPath = CORE::CombinePath( dir, AUTH::CFileCredentialStore::DataFileName );
            {
                AUTH::CFileCredentialStore store( dir );
                ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            }
            // the version byte follows the magic
            CORE::CDynamicBuffer content;
            content.LoadContentFromFile( dataPath );
            static_cast< UInt8* >( content.GetBufferPtr() )[ 4 ] = 2;
            CORE::WriteFileAtomically( dataPath, content );

            AUTH::CFileCredentialStore store( dir );
            AUTH::CSecretString secret;
            ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
            ASSERT_FALSE( store.IsTamperDetected() );
            ASSERT_FALSE( store.Store( "google", "google:2", AUTH::CSecretString( "x" ) ) );
            CORE::CDynamicBuffer after;
            after.LoadContentFromFile( dataPath );
            ASSERT_TRUE( after.GetDataSize() == content.GetDataSize() && 0 == memcmp( after.GetConstBufferPtr(), content.GetConstBufferPtr(), content.GetDataSize() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Wiping the file store takes the data and the key away" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "cred_wipe" );
            AUTH::CFileCredentialStore store( dir );
            ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            ASSERT_TRUE( store.Wipe() );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, AUTH::CFileCredentialStore::DataFileName ) ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, AUTH::CFileCredentialStore::KeyFileName ) ) );
            AUTH::CFileCredentialStore reopened( dir );
            AUTH::CSecretString secret;
            ASSERT_FALSE( reopened.Load( "google", "google:1", secret ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The file store follows its config and its default location" )
        try
        {
            CORE::CDataNode config( "CredentialStore", GUCEF_DATATYPE_OBJECT );
            config.SetAttribute( "directory", CORE::CString( "$CURWORKDIR$/authtemp/cred_config" ) );
            config.SetAttribute( "appName", CORE::CString( "SomeGame" ) );
            AUTH::CFileCredentialStore store;
            ASSERT_TRUE( store.LoadConfig( config ) );
            ASSERT_TRUE( store.GetDirectory().HasSubstr( "cred_config" ) >= 0 );
            CORE::CDataNode saved( "CredentialStore", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( store.SaveConfig( saved ) );
            ASSERT_TRUE( "SomeGame" == saved.GetAttributeValueOrChildValueByName( "appName" ).AsString() );

            AUTH::CFileCredentialStore defaults;
            CORE::CString defaultDir = defaults.GetDirectory();
            ASSERT_TRUE( defaultDir.HasSubstr( "GUCEF" ) >= 0 && defaultDir.HasSubstr( "Auth" ) >= 0 );
            CORE::CDataNode appConfig( "CredentialStore", GUCEF_DATATYPE_OBJECT );
            appConfig.SetAttribute( "appName", CORE::CString( "SomeGame" ) );
            ASSERT_TRUE( defaults.LoadConfig( appConfig ) && defaults.GetDirectory().HasSubstr( "SomeGame" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
