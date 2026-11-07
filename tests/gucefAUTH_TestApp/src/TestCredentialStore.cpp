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

#include <stdlib.h>
#include <string.h>
#include <map>

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
  #include <signal.h>
  #include <fcntl.h>
  #include <sys/file.h>
  #include <sys/resource.h>
#endif

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

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
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

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

static CORE::CDynamicBuffer
ReadBytes( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    content.LoadContentFromFile( path );
    return content;
}

/*-------------------------------------------------------------------------*/

static bool
SameBytes( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{GUCEF_TRACE;

    return a.GetDataSize() == b.GetDataSize() && ( 0 == a.GetDataSize() || 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
WriterAccount( const CORE::CString& prefix, UInt32 index )
{GUCEF_TRACE;

    return prefix + ":" + CORE::ToString( index );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
WriterSecret( const CORE::CString& prefix, UInt32 index )
{GUCEF_TRACE;

    return "secret-" + prefix + "-" + CORE::ToString( index );
}

/*-------------------------------------------------------------------------*/

/**
 *  What one writer of the contention tests does with one instance of the store, kept for all its changes: stores the
 *  slots "writer" / <prefix>:<i>, every third one with a slot "scratch" stored and removed again, and reads its own
 *  slots back at the end. Returns the number of changes and reads that did not work
 */
static UInt32
WriteSlots( const CORE::CString& directory, const CORE::CString& prefix, UInt32 count )
{GUCEF_TRACE;

    UInt32 failures = 0;
    AUTH::CFileCredentialStore store( directory );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !store.Store( "writer", WriterAccount( prefix, i ), AUTH::CSecretString( WriterSecret( prefix, i ) ) ) )
            ++failures;
        if ( 0 == i % 3 )
        {
            if ( !store.Store( "scratch", WriterAccount( prefix, i ), AUTH::CSecretString( "scratch" ) ) )
                ++failures;
            if ( !store.Remove( "scratch", WriterAccount( prefix, i ) ) )
                ++failures;
        }
    }
    for ( UInt32 i=0; i<count; ++i )
    {
        AUTH::CSecretString secret;
        if ( !store.Load( "writer", WriterAccount( prefix, i ), secret ) || WriterSecret( prefix, i ) != secret.Reveal() )
            ++failures;
    }
    return failures;
}

/*-------------------------------------------------------------------------*/

/** Counts the slots of the writers that a new instance finds: all of them with the right secret, none of "scratch" */
static UInt32
CountWrittenSlots( const CORE::CString& directory, const CORE::CString& prefix, UInt32 count )
{GUCEF_TRACE;

    AUTH::CFileCredentialStore store( directory );
    UInt32 found = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        AUTH::CSecretString secret;
        if ( store.Load( "writer", WriterAccount( prefix, i ), secret ) && WriterSecret( prefix, i ) == secret.Reveal() )
            ++found;
    }
    return found;
}

/*-------------------------------------------------------------------------*/

/** A thread with an instance of the store of its own that runs WriteSlots() */
class CStoreWriterThread : public MT::CActiveObject
{
    public:

    CStoreWriterThread( const CORE::CString& directory, const CORE::CString& prefix, UInt32 count )
        : MT::CActiveObject(), m_directory( directory ), m_prefix( prefix ), m_count( count ), m_lock(), m_done( false ), m_failures( 0 )
    {
    }

    bool IsDone( void ) const { MT::CScopeMutex lock( m_lock ); return m_done; }
    UInt32 GetFailures( void ) const { MT::CScopeMutex lock( m_lock ); return m_failures; }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 failures = WriteSlots( m_directory, m_prefix, m_count );
        MT::CScopeMutex lock( m_lock );
        m_failures = failures;
        m_done = true;
        return true;
    }

    private:

    CORE::CString m_directory;
    CORE::CString m_prefix;
    UInt32 m_count;
    mutable MT::CMutex m_lock;
    bool m_done;
    UInt32 m_failures;
};

/*-------------------------------------------------------------------------*/

bool
IsCredentialStoreHelperRun( void )
{GUCEF_TRACE;

    return "credstore-writer" == GetEnvText( "GUCEF_AUTH_TEST_HELPER" );
}

/*-------------------------------------------------------------------------*/

int
RunCredentialStoreHelper( void )
{GUCEF_TRACE;

    CORE::CString directory = GetEnvText( "GUCEF_AUTH_TEST_STORE_DIR" );
    CORE::CString syncDirectory = GetEnvText( "GUCEF_AUTH_TEST_SYNC_DIR" );
    CORE::CString prefix = GetEnvText( "GUCEF_AUTH_TEST_PREFIX" );
    UInt32 count = CORE::StringToUInt32( GetEnvText( "GUCEF_AUTH_TEST_COUNT" ) );
    if ( directory.IsNULLOrEmpty() || syncDirectory.IsNULLOrEmpty() || prefix.IsNULLOrEmpty() || 0 == count )
        return 3;
    if ( !AUTH::CFileCredentialStore::IsAvailable() )
        return 4;

    // both processes start writing at the same moment: this one says it is ready and waits for the word of the test
    CORE::CString ready = "ready";
    if ( !CORE::WriteFileAtomically( CORE::CombinePath( syncDirectory, "ready-" + prefix ), ready.C_String(), ready.Length() ) )
        return 5;
    if ( !WaitForFile( CORE::CombinePath( syncDirectory, "go" ), 60000 ) )
        return 6;
    return 0 == WriteSlots( directory, prefix, count ) ? 0 : 2;
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

            // the directory holds the two files and the lock file, no leftovers of the atomic writes
            CORE::CString::StringVector names = ListDirectory( dir );
            ASSERT_TRUE( 3 == names.size() && HasName( names, AUTH::CFileCredentialStore::DataFileName ) && HasName( names, AUTH::CFileCredentialStore::KeyFileName ) &&
                         HasName( names, AUTH::CFileCredentialStore::LockFileName ) );

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

            // a detected store is not written by any change, only Wipe() starts over with a good file
            {
                CORE::CString againDir = MakeTestDirectory( "cred_tamper_recover" );
                CORE::CreateDirs( againDir );
                CORE::CString againData = CORE::CombinePath( againDir, AUTH::CFileCredentialStore::DataFileName );
                CORE::CString againKey = CORE::CombinePath( againDir, AUTH::CFileCredentialStore::KeyFileName );
                CORE::CopyFile( dataPath, againData, true );
                CORE::CopyFile( CORE::CombinePath( dir, AUTH::CFileCredentialStore::KeyFileName ), againKey, true );
                FlipFileByte( againData, 40 );
                CORE::CDynamicBuffer tamperedBytes = ReadBytes( againData );
                CORE::CDynamicBuffer keyBytes = ReadBytes( againKey );
                AUTH::CFileCredentialStore recover( againDir );
                AUTH::CSecretString secret;
                ASSERT_FALSE( recover.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( recover.IsTamperDetected() );
                ASSERT_FALSE( recover.Store( "google", "google:2", AUTH::CSecretString( "fresh" ) ) );
                ASSERT_FALSE( recover.Remove( "google", "google:1" ) );
                ASSERT_FALSE( recover.RemoveProvider( "google" ) );
                ASSERT_TRUE( recover.IsTamperDetected() );
                ASSERT_TRUE( SameBytes( tamperedBytes, ReadBytes( againData ) ) && SameBytes( keyBytes, ReadBytes( againKey ) ) );

                // the lost key: the data is not replaced either
                CORE::CString lostDir = MakeTestDirectory( "cred_tamper_recover_nokey" );
                CORE::CreateDirs( lostDir );
                CORE::CString lostData = CORE::CombinePath( lostDir, AUTH::CFileCredentialStore::DataFileName );
                CORE::CopyFile( dataPath, lostData, true );
                CORE::CDynamicBuffer lostBytes = ReadBytes( lostData );
                AUTH::CFileCredentialStore lost( lostDir );
                ASSERT_FALSE( lost.Store( "google", "google:2", AUTH::CSecretString( "fresh" ) ) );
                ASSERT_TRUE( SameBytes( lostBytes, ReadBytes( lostData ) ) );
                ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( lostDir, AUTH::CFileCredentialStore::KeyFileName ) ) );

                ASSERT_TRUE( recover.Wipe() );
                ASSERT_FALSE( recover.IsTamperDetected() );
                ASSERT_TRUE( recover.Store( "google", "google:2", AUTH::CSecretString( "fresh" ) ) );
                ASSERT_FALSE( recover.IsTamperDetected() );
                AUTH::CFileCredentialStore reopened( againDir );
                ASSERT_TRUE( reopened.Load( "google", "google:2", secret ) && "fresh" == secret.Reveal() && !reopened.IsTamperDetected() );
                ASSERT_FALSE( reopened.Load( "google", "google:1", secret ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: Two instances on one store see each other's changes and never undo them" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "cred_two_instances" );
            AUTH::CFileCredentialStore first( dir );
            AUTH::CFileCredentialStore second( dir );
            AUTH::CSecretString secret;
            CORE::CString::StringVector accounts;

            ASSERT_TRUE( first.Store( "broker", "a1", AUTH::CSecretString( "secret-a1" ) ) );
            ASSERT_TRUE( second.Store( "broker", "b1", AUTH::CSecretString( "secret-b1" ) ) );
            // the first instance read the file before the second stored b1: its next store keeps b1
            ASSERT_TRUE( first.Store( "broker", "a2", AUTH::CSecretString( "secret-a2" ) ) );
            {
                AUTH::CFileCredentialStore check( dir );
                ASSERT_TRUE( check.Load( "broker", "a1", secret ) && "secret-a1" == secret.Reveal() );
                ASSERT_TRUE( check.Load( "broker", "b1", secret ) && "secret-b1" == secret.Reveal() );
                ASSERT_TRUE( check.Load( "broker", "a2", secret ) && "secret-a2" == secret.Reveal() );
            }

            // a removal by the second keeps what the first stored meanwhile
            ASSERT_TRUE( second.Remove( "broker", "a1" ) );
            ASSERT_FALSE( first.Load( "broker", "a1", secret ) );
            ASSERT_TRUE( first.Load( "broker", "a2", secret ) && "secret-a2" == secret.Reveal() );
            ASSERT_TRUE( first.Load( "broker", "b1", secret ) && "secret-b1" == secret.Reveal() );

            // reads follow the file: the order of storing is the one in the file
            ASSERT_TRUE( second.Store( "broker", "b2", AUTH::CSecretString( "secret-b2" ) ) );
            ASSERT_TRUE( first.ListAccounts( "broker", accounts ) && 3 == accounts.size() && "b2" == accounts[ 0 ] && "a2" == accounts[ 1 ] && "b1" == accounts[ 2 ] );

            // a replacement by one is what the other loads, and the other's next change keeps it
            ASSERT_TRUE( first.Store( "broker", "b1", AUTH::CSecretString( "secret-b1-rotated" ) ) );
            ASSERT_TRUE( second.Load( "broker", "b1", secret ) && "secret-b1-rotated" == secret.Reveal() );
            ASSERT_TRUE( first.Store( "other", "x", AUTH::CSecretString( "secret-x" ) ) );
            ASSERT_TRUE( second.RemoveProvider( "broker" ) );
            ASSERT_TRUE( first.Load( "other", "x", secret ) && "secret-x" == secret.Reveal() );
            ASSERT_FALSE( first.Load( "broker", "b1", secret ) );
            ASSERT_TRUE( second.Store( "broker", "b3", AUTH::CSecretString( "secret-b3" ) ) );
            ASSERT_TRUE( first.Remove( "other", "x" ) );
            ASSERT_TRUE( second.Load( "broker", "b3", secret ) && "secret-b3" == secret.Reveal() );
            ASSERT_FALSE( second.Load( "other", "x", secret ) );

            // a wipe by one is seen by the other, which does not bring back what it had read
            ASSERT_TRUE( first.Wipe() );
            ASSERT_FALSE( second.Load( "broker", "b3", secret ) );
            ASSERT_TRUE( second.Store( "broker", "b4", AUTH::CSecretString( "secret-b4" ) ) );
            ASSERT_TRUE( first.ListAccounts( "broker", accounts ) && 1 == accounts.size() && "b4" == accounts[ 0 ] );
            ASSERT_TRUE( first.Load( "broker", "b4", secret ) && "secret-b4" == secret.Reveal() );
            ASSERT_FALSE( first.IsTamperDetected() || second.IsTamperDetected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two threads with an instance each store into one store at the same time without losing a slot" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "cred_threads" );
            const UInt32 SlotCount = 100;
            {
                AUTH::CFileCredentialStore seed( dir );
                ASSERT_TRUE( seed.Store( "seed", "seed", AUTH::CSecretString( "seed" ) ) );
            }
            CStoreWriterThread left( dir, "left", SlotCount );
            CStoreWriterThread right( dir, "right", SlotCount );
            ASSERT_TRUE( left.Activate( GUCEF_NULL, 1, 1 ) );
            ASSERT_TRUE( right.Activate( GUCEF_NULL, 1, 1 ) );
            for ( UInt32 waited=0; waited<120000 && !( left.IsDone() && right.IsDone() ); waited+=5 )
                MT::PrecisionDelay( 5 );
            ASSERT_TRUE( left.IsDone() && right.IsDone() );
            left.Deactivate( false, true );
            right.Deactivate( false, true );
            ASSERT_TRUE( 0 == left.GetFailures() && 0 == right.GetFailures() );

            ASSERT_TRUE( SlotCount == CountWrittenSlots( dir, "left", SlotCount ) );
            ASSERT_TRUE( SlotCount == CountWrittenSlots( dir, "right", SlotCount ) );
            AUTH::CFileCredentialStore check( dir );
            CORE::CString::StringVector accounts;
            AUTH::CSecretString secret;
            ASSERT_TRUE( check.ListAccounts( "writer", accounts ) && 2 * SlotCount == accounts.size() );
            ASSERT_TRUE( check.ListAccounts( "scratch", accounts ) && accounts.empty() );
            ASSERT_TRUE( check.Load( "seed", "seed", secret ) && "seed" == secret.Reveal() );
            ASSERT_FALSE( check.IsTamperDetected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A second process stores into one store at the same time as this one without losing a slot" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_LINUX )
        GUCEF_TESTFW_SKIP( "The test application is started again as a second process on Linux" );
        #endif
        try
        {
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            CORE::CString dir = MakeTestDirectory( "cred_process" );
            CORE::CString syncDir = MakeTestDirectory( "cred_process_sync" );
            ASSERT_TRUE( CORE::CreateDirs( syncDir ) );
            const UInt32 SlotCount = 100;
            {
                AUTH::CFileCredentialStore seed( dir );
                ASSERT_TRUE( seed.Store( "seed", "seed", AUTH::CSecretString( "seed" ) ) );
            }

            CORE::CString exePath = GetOwnExecutablePath();
            ASSERT_FALSE( exePath.IsNULLOrEmpty() );
            CORE::CProcess child;
            child.SetIsolateFromParent( true );
            CORE::CProcess::TStringMap environment;
            environment[ "GUCEF_AUTH_TEST_HELPER" ] = "credstore-writer";
            environment[ "GUCEF_AUTH_TEST_STORE_DIR" ] = dir;
            environment[ "GUCEF_AUTH_TEST_SYNC_DIR" ] = syncDir;
            environment[ "GUCEF_AUTH_TEST_PREFIX" ] = "child";
            environment[ "GUCEF_AUTH_TEST_COUNT" ] = CORE::ToString( SlotCount );
            environment[ "GUCEF_CONSOLE_LOGGER" ] = "none";
            REQUIRE_TRUE( child.Launch( exePath, CORE::CProcess::TStringVector(), CORE::CString::Empty, environment ) );
            bool childReady = WaitForFile( CORE::CombinePath( syncDir, "ready-child" ), 60000 );
            ASSERT_TRUE( childReady );
            CORE::CString go = "go";
            ASSERT_TRUE( CORE::WriteFileAtomically( CORE::CombinePath( syncDir, "go" ), go.C_String(), go.Length() ) );
            UInt32 failures = WriteSlots( dir, "parent", SlotCount );
            Int32 exitCode = -1;
            ASSERT_TRUE( child.WaitForExit( 120000, exitCode ) );
            ASSERT_TRUE( 0 == exitCode );
            ASSERT_TRUE( 0 == failures );

            ASSERT_TRUE( SlotCount == CountWrittenSlots( dir, "parent", SlotCount ) );
            ASSERT_TRUE( SlotCount == CountWrittenSlots( dir, "child", SlotCount ) );
            AUTH::CFileCredentialStore check( dir );
            CORE::CString::StringVector accounts;
            AUTH::CSecretString secret;
            ASSERT_TRUE( check.ListAccounts( "writer", accounts ) && 2 * SlotCount == accounts.size() );
            ASSERT_TRUE( check.ListAccounts( "scratch", accounts ) && accounts.empty() );
            ASSERT_TRUE( check.Load( "seed", "seed", secret ) && "seed" == secret.Reveal() );
            ASSERT_FALSE( check.IsTamperDetected() );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The integrity of the store is known right after construction and follows the file" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "cred_tamper_first" );
            CORE::CString dataPath = CORE::CombinePath( dir, AUTH::CFileCredentialStore::DataFileName );
            CORE::CString keyPath = CORE::CombinePath( dir, AUTH::CFileCredentialStore::KeyFileName );
            {
                AUTH::CFileCredentialStore store( dir );
                ASSERT_FALSE( store.IsTamperDetected() );
                ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            }
            CORE::CDynamicBuffer good = ReadBytes( dataPath );

            // a good store: no detection, also before anything was read
            {
                AUTH::CFileCredentialStore fresh( dir );
                ASSERT_FALSE( fresh.IsTamperDetected() );
            }

            // the instance that read the good file
            AUTH::CFileCredentialStore holder( dir );
            AUTH::CSecretString secret;
            ASSERT_TRUE( holder.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );

            // a changed file: a new instance says so before anything else is asked of it
            ASSERT_TRUE( FlipFileByte( dataPath, (UInt32) good.GetDataSize() - 5 ) );
            CORE::CDynamicBuffer tampered = ReadBytes( dataPath );
            {
                AUTH::CFileCredentialStore fresh( dir );
                ASSERT_TRUE( fresh.IsTamperDetected() );
            }

            // the instance that read the good file sees the change as well, reads nothing from it and writes nothing to it
            ASSERT_TRUE( holder.IsTamperDetected() );
            ASSERT_FALSE( holder.Load( "google", "google:1", secret ) );
            ASSERT_FALSE( holder.Store( "google", "google:2", AUTH::CSecretString( "refresh-two" ) ) );
            ASSERT_TRUE( SameBytes( tampered, ReadBytes( dataPath ) ) );

            // the good file back: read again
            ASSERT_TRUE( CORE::WriteFileAtomically( dataPath, good ) );
            ASSERT_FALSE( holder.IsTamperDetected() );
            ASSERT_TRUE( holder.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );

            // the key gone: detected before the first read
            CORE::CDynamicBuffer key = ReadBytes( keyPath );
            ASSERT_TRUE( CORE::DeleteFile( keyPath ) );
            {
                AUTH::CFileCredentialStore fresh( dir );
                ASSERT_TRUE( fresh.IsTamperDetected() );
            }
            ASSERT_TRUE( holder.IsTamperDetected() );
            ASSERT_TRUE( CORE::WriteFileAtomically( keyPath, key ) );
            ASSERT_FALSE( holder.IsTamperDetected() );

            // a store without files is not tampered with, and asking does not make the directory
            CORE::CString emptyDir = MakeTestDirectory( "cred_tamper_first_empty" );
            {
                AUTH::CFileCredentialStore fresh( emptyDir );
                ASSERT_FALSE( fresh.IsTamperDetected() );
                ASSERT_FALSE( fresh.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( fresh.Remove( "google", "google:1" ) );
            }
            ASSERT_FALSE( CORE::DirExists( emptyDir ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An instance waits for the lock of another one and gives up after the lock timeout without changing anything" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_LINUX )
        GUCEF_TESTFW_SKIP( "The lock is held with flock() by the test on Linux" );
        #endif
        try
        {
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            CORE::CString dir = MakeTestDirectory( "cred_lock_timeout" );
            CORE::CString dataPath = CORE::CombinePath( dir, AUTH::CFileCredentialStore::DataFileName );
            CORE::CString lockPath = CORE::CombinePath( dir, AUTH::CFileCredentialStore::LockFileName );
            AUTH::CFileCredentialStore store( dir );
            ASSERT_TRUE( AUTH::CFileCredentialStore::DefaultLockTimeoutInMs == store.GetLockTimeoutInMs() );
            ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            ASSERT_TRUE( CORE::FileExists( lockPath ) );
            CORE::CDynamicBuffer before = ReadBytes( dataPath );

            // the lock timeout is configuration
            CORE::CDataNode config( "CredentialStore", GUCEF_DATATYPE_OBJECT );
            config.SetAttribute( "directory", dir );
            config.SetAttribute( "lockTimeoutInMs", CORE::CString( "150" ) );
            ASSERT_TRUE( store.LoadConfig( config ) && 150 == store.GetLockTimeoutInMs() );
            CORE::CDataNode saved( "CredentialStore", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( store.SaveConfig( saved ) && 150 == saved.GetAttributeValueOrChildValueByName( "lockTimeoutInMs" ).AsUInt32() );

            // another holder of the exclusive lock: a change and a read wait for the timeout, then give up
            int holder = open( lockPath.C_String(), O_RDWR | O_CLOEXEC );
            ASSERT_TRUE( holder >= 0 && 0 == flock( holder, LOCK_EX ) );
            UInt64 startedAt = AUTH::GetMonotonicTimeInMs();
            ASSERT_FALSE( store.Store( "google", "google:2", AUTH::CSecretString( "refresh-two" ) ) );
            UInt64 waited = AUTH::GetMonotonicTimeInMs() - startedAt;
            ASSERT_TRUE( waited >= 140 && waited < 5000 );
            ASSERT_FALSE( store.Remove( "google", "google:1" ) );
            ASSERT_FALSE( store.RemoveProvider( "google" ) );
            AUTH::CSecretString secret;
            ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
            CORE::CString::StringVector accounts;
            ASSERT_FALSE( store.ListAccounts( "google", accounts ) );
            ASSERT_FALSE( store.IsTamperDetected() );
            ASSERT_TRUE( SameBytes( before, ReadBytes( dataPath ) ) );

            // a shared holder: reads go on, a change still waits
            ASSERT_TRUE( 0 == flock( holder, LOCK_SH ) );
            ASSERT_TRUE( store.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
            ASSERT_TRUE( store.ListAccounts( "google", accounts ) && 1 == accounts.size() );
            ASSERT_FALSE( store.Store( "google", "google:2", AUTH::CSecretString( "refresh-two" ) ) );
            ASSERT_TRUE( SameBytes( before, ReadBytes( dataPath ) ) );

            // let go: the change goes through, and the wipe keeps the lock file
            ASSERT_TRUE( 0 == flock( holder, LOCK_UN ) );
            ASSERT_TRUE( store.Store( "google", "google:2", AUTH::CSecretString( "refresh-two" ) ) );
            ASSERT_TRUE( store.Load( "google", "google:2", secret ) && "refresh-two" == secret.Reveal() );
            ASSERT_TRUE( store.Wipe() );
            ASSERT_TRUE( CORE::FileExists( lockPath ) && !CORE::FileExists( dataPath ) );
            if ( holder >= 0 )
                close( holder );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A write that fails leaves the old file and what the instance knows as they were" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_LINUX )
        GUCEF_TESTFW_SKIP( "The limit on the file size that makes a write fail is set on Linux" );
        #endif
        try
        {
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            CORE::CString dir = MakeTestDirectory( "cred_write_fails" );
            CORE::CString dataPath = CORE::CombinePath( dir, AUTH::CFileCredentialStore::DataFileName );
            AUTH::CFileCredentialStore store( dir );
            ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            CORE::CDynamicBuffer before = ReadBytes( dataPath );
            CORE::CString large;
            for ( UInt32 i=0; i<512; ++i )
                large += "0123456789abcdef";

            // the process may not write files larger than the store of now: the new content does not fit
            struct rlimit previous;
            ASSERT_TRUE( 0 == getrlimit( RLIMIT_FSIZE, &previous ) );
            struct rlimit limited = previous;
            limited.rlim_cur = (rlim_t) before.GetDataSize() + 64;
            void ( *previousHandler )( int ) = signal( SIGXFSZ, SIG_IGN );
            bool stored = true;
            if ( 0 == setrlimit( RLIMIT_FSIZE, &limited ) )
            {
                stored = store.Store( "google", "google:2", AUTH::CSecretString( large ) );
                setrlimit( RLIMIT_FSIZE, &previous );
            }
            signal( SIGXFSZ, previousHandler );

            ASSERT_FALSE( stored );
            ASSERT_TRUE( SameBytes( before, ReadBytes( dataPath ) ) );
            AUTH::CSecretString secret;
            ASSERT_FALSE( store.Load( "google", "google:2", secret ) );
            ASSERT_TRUE( store.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
            CORE::CString::StringVector names = ListDirectory( dir );
            for ( size_t i=0; i<names.size(); ++i )
                ASSERT_TRUE( names[ i ].HasSubstr( ".tmp" ) < 0 );

            // without the limit the same change works
            ASSERT_TRUE( store.Store( "google", "google:2", AUTH::CSecretString( large ) ) );
            AUTH::CFileCredentialStore check( dir );
            ASSERT_TRUE( check.Load( "google", "google:2", secret ) && large == secret.Reveal() );
            ASSERT_TRUE( check.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
