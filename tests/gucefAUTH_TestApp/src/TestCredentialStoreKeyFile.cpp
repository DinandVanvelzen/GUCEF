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

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <sys/types.h>
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

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
#include "TestCredentialStoreKeyFile.h"

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/** The test seam of CFileCredentialStore: only tests reach the emulation of a read only key mount */
class CFileCredentialStoreTestAccess
{
    public:

    static void AssumeReadOnlyKeyMount( CFileCredentialStore& store, bool assumeReadOnly )
    {
        store.SetAssumeReadOnlyKeyMountForTesting( assumeReadOnly );
    }
};

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

namespace {

/** Whether code that is not a friend of T can call T::SetAssumeReadOnlyKeyMountForTesting( bool ) (access is part of SFINAE) */
template < class T >
struct TypeHasReachableReadOnlyMountSetter
{
    typedef char yes[ 1 ];
    typedef char no[ 2 ];

    template < typename U, U u > struct reallyHas;
    template < typename TestClass > static yes& test( reallyHas< void (TestClass::*)( bool ), &TestClass::SetAssumeReadOnlyKeyMountForTesting >* /*unused*/ ) { static yes result; return result; }
    template < typename TestClass > static no&  test( ... ) { static no result; return result; }

    enum { value = sizeof( test< T >( 0 ) ) == sizeof( yes ) };
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

namespace {

CORE::CDynamicBuffer
KeyFileTestRead( const CORE::CString& path )
{
    CORE::CDynamicBuffer content;
    content.LoadContentFromFile( path );
    return content;
}

/*-------------------------------------------------------------------------*/

bool
KeyFileTestSame( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{
    return a.GetDataSize() == b.GetDataSize() && ( 0 == a.GetDataSize() || 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );
}

/*-------------------------------------------------------------------------*/

/** Writes a file with exactly the given mode, replacing what is there */
bool
KeyFileTestWrite( const CORE::CString& path, const void* data, UInt32 size, mode_t mode )
{
    ::unlink( path.C_String() );
    int fd = ::open( path.C_String(), O_WRONLY | O_CREAT | O_EXCL, 0600 );
    if ( fd < 0 )
        return false;
    bool written = size == 0 || (ssize_t) size == ::write( fd, data, size );
    written = 0 == ::fchmod( fd, mode ) && written;
    ::close( fd );
    return written;
}

/*-------------------------------------------------------------------------*/

bool
KeyFileTestWriteText( const CORE::CString& path, const CORE::CString& text, mode_t mode )
{
    return KeyFileTestWrite( path, text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0, mode );
}

/*-------------------------------------------------------------------------*/

CORE::CString
KeyFileTestHex( const CORE::CDynamicBuffer& bytes, bool upperCase )
{
    const char* digits = upperCase ? "0123456789ABCDEF" : "0123456789abcdef";
    CORE::CString text;
    const UInt8* data = static_cast< const UInt8* >( bytes.GetConstBufferPtr() );
    for ( UInt32 i=0; i<bytes.GetDataSize(); ++i )
    {
        char pair[ 3 ] = { digits[ data[ i ] >> 4 ], digits[ data[ i ] & 0x0F ], '\0' };
        text += pair;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

/** Counts how often a text occurs in another */
UInt32
KeyFileTestCount( const CORE::CString& all, const CORE::CString& text )
{
    UInt32 count = 0;
    Int32 at = all.HasSubstr( text, 0, true );
    while ( at >= 0 )
    {
        ++count;
        at = all.HasSubstr( text, (Int32) at + 1, true );
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/** A store on 'dataDir' with an external key */
void
KeyFileTestSetup( AUTH::CFileCredentialStore& store, const CORE::CString& keyPath, AUTH::CFileCredentialStore::EKeyFileTrust trust = AUTH::CFileCredentialStore::KEYFILETRUST_OWNER_ONLY )
{
    store.SetKeyFile( keyPath );
    store.SetKeyFileTrust( trust );
}

/*-------------------------------------------------------------------------*/

/**
 *  The layout of a Kubernetes Secret volume in 'mountDir': the file in a timestamped directory, "..data" a link to
 *  that directory, and the name of the key a link to "..data/<name>". Returns the path to give to the store
 */
CORE::CString
MakeSecretVolume( const CORE::CString& mountDir, const CORE::CString& name, const CORE::CDynamicBuffer& key, mode_t mode )
{
    const CORE::CString timestamped = CORE::CombinePath( mountDir, "..2026_11_08_12_00_00.123456789" );
    if ( !CORE::CreateDirs( timestamped ) )
        return CORE::CString::Empty;
    if ( !KeyFileTestWrite( CORE::CombinePath( timestamped, name ), key.GetConstBufferPtr(), key.GetDataSize(), mode ) )
        return CORE::CString::Empty;
    const CORE::CString dataLink = CORE::CombinePath( mountDir, "..data" );
    const CORE::CString keyLink = CORE::CombinePath( mountDir, name );
    ::unlink( dataLink.C_String() );
    ::unlink( keyLink.C_String() );
    if ( 0 != ::symlink( "..2026_11_08_12_00_00.123456789", dataLink.C_String() ) )
        return CORE::CString::Empty;
    if ( 0 != ::symlink( ( "..data/" + name ).C_String(), keyLink.C_String() ) )
        return CORE::CString::Empty;
    return keyLink;
}

}

#endif /* GUCEF_PLATFORM_LINUX ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCredentialStoreKeyFileTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING credential store key file TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "CredentialStoreKeyFile" );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    typedef AUTH::CFileCredentialStore TStore;
    const CORE::CString dataName = AUTH::CFileCredentialStore::DataFileName;
    const CORE::CString localKeyName = AUTH::CFileCredentialStore::KeyFileName;

    GUCEF_TESTFW_TESTCASE( "Test: MakeKeyFile() makes 32 random bytes for the owner only and refuses what exists, a link included" )
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyfile_make" );
            REQUIRE_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString path = CORE::CombinePath( dir, "store.key" );
            CORE::CString problem;
            ASSERT_TRUE( TStore::MakeKeyFile( path, problem ) );
            CORE::CDynamicBuffer key = KeyFileTestRead( path );
            ASSERT_TRUE( 32 == key.GetDataSize() );
            struct stat info;
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );

            // never over what is there: a file, or a link (also one that points nowhere)
            ASSERT_FALSE( TStore::MakeKeyFile( path, problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );
            ASSERT_TRUE( KeyFileTestSame( key, KeyFileTestRead( path ) ) );
            CORE::CString link = CORE::CombinePath( dir, "link.key" );
            ASSERT_TRUE( 0 == ::symlink( "nowhere.key", link.C_String() ) );
            ASSERT_FALSE( TStore::MakeKeyFile( link, problem ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, "nowhere.key" ) ) );

            // two keys are not the same
            CORE::CString second = CORE::CombinePath( dir, "second.key" );
            ASSERT_TRUE( TStore::MakeKeyFile( second, problem ) );
            ASSERT_FALSE( KeyFileTestSame( key, KeyFileTestRead( second ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An owner-only external key seals the store; no local key is made and the key file is never changed" )
        GUCEF_TESTFW_SKIP_IF( !TStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString keyDir = MakeTestDirectory( "keyfile_ext_k" );
            CORE::CString dataDir = MakeTestDirectory( "keyfile_ext_d" );
            REQUIRE_TRUE( CORE::CreateDirs( keyDir ) );
            CORE::CString keyPath = CORE::CombinePath( keyDir, "store.key" );
            CORE::CString problem;
            REQUIRE_TRUE( TStore::MakeKeyFile( keyPath, problem ) );
            const CORE::CDynamicBuffer key = KeyFileTestRead( keyPath );

            TStore store( dataDir );
            KeyFileTestSetup( store, keyPath );
            ASSERT_TRUE( store.HasExternalKeyFile() && store.GetKeyFile() == CORE::RelativePath( keyPath ) );
            // no data yet: the state of the key is known without anything being made
            ASSERT_TRUE( TStore::KEYSTATE_OK == store.GetKeyState() );
            ASSERT_FALSE( CORE::DirExists( dataDir ) );

            ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            ASSERT_TRUE( store.Store( "google", "google:2", AUTH::CSecretString( "refresh-two" ) ) );
            ASSERT_TRUE( store.Remove( "google", "google:2" ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dataDir, localKeyName ) ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( dataDir, dataName ) ) );
            ASSERT_TRUE( KeyFileTestSame( key, KeyFileTestRead( keyPath ) ) );

            TStore second( dataDir );
            KeyFileTestSetup( second, keyPath );
            AUTH::CSecretString secret;
            ASSERT_TRUE( second.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
            ASSERT_FALSE( second.Load( "google", "google:2", secret ) );
            ASSERT_FALSE( second.IsTamperDetected() );
            ASSERT_TRUE( TStore::KEYSTATE_OK == second.GetKeyState() && second.GetKeyProblem().IsNULLOrEmpty() );

            // without the key setting the data cannot be read: it has no local key
            TStore withoutKey( dataDir );
            ASSERT_FALSE( withoutKey.Load( "google", "google:1", secret ) );
            ASSERT_TRUE( withoutKey.IsTamperDetected() );
            ASSERT_TRUE( KeyFileTestSame( key, KeyFileTestRead( keyPath ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A missing external key is its own state: nothing read, nothing written anywhere, no tampering; the key coming back restores the store" )
        GUCEF_TESTFW_SKIP_IF( !TStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString keyDir = MakeTestDirectory( "keyfile_missing_k" );
            CORE::CString dataDir = MakeTestDirectory( "keyfile_missing_d" );
            CORE::CString freshDir = MakeTestDirectory( "keyfile_missing_fresh" );
            REQUIRE_TRUE( CORE::CreateDirs( keyDir ) );
            CORE::CString keyPath = CORE::CombinePath( keyDir, "store.key" );
            CORE::CString problem;
            REQUIRE_TRUE( TStore::MakeKeyFile( keyPath, problem ) );
            const CORE::CDynamicBuffer key = KeyFileTestRead( keyPath );
            {
                TStore store( dataDir );
                KeyFileTestSetup( store, keyPath );
                REQUIRE_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            }
            const CORE::CString dataPath = CORE::CombinePath( dataDir, dataName );
            const CORE::CDynamicBuffer data = KeyFileTestRead( dataPath );
            REQUIRE_TRUE( 0 == ::unlink( keyPath.C_String() ) );

            CLogCapture capture;
            TStore store( dataDir );
            KeyFileTestSetup( store, keyPath );
            AUTH::CSecretString secret;
            ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
            ASSERT_FALSE( store.Store( "google", "google:2", AUTH::CSecretString( "refresh-two" ) ) );
            ASSERT_FALSE( store.Remove( "google", "google:1" ) );
            ASSERT_TRUE( TStore::KEYSTATE_MISSING == store.GetKeyState() );
            ASSERT_FALSE( store.IsTamperDetected() );
            ASSERT_TRUE( KeyFileTestSame( data, KeyFileTestRead( dataPath ) ) );
            ASSERT_FALSE( CORE::FileExists( keyPath ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dataDir, localKeyName ) ) );
            // logged once for the state, with the path, never the content
            ASSERT_TRUE( 1 == KeyFileTestCount( capture.Text(), "is missing" ) );
            ASSERT_TRUE( capture.Contains( keyPath ) );

            // no data and no key: refused, and not even the directory is made
            TStore fresh( freshDir );
            KeyFileTestSetup( fresh, keyPath );
            ASSERT_FALSE( fresh.Store( "google", "google:1", AUTH::CSecretString( "x" ) ) );
            ASSERT_TRUE( TStore::KEYSTATE_MISSING == fresh.GetKeyState() );
            ASSERT_FALSE( fresh.IsTamperDetected() );
            ASSERT_FALSE( CORE::DirExists( freshDir ) );

            // the key is back: the next read sees the slots
            REQUIRE_TRUE( CORE::CreateOwnerOnlyFile( keyPath, key.GetConstBufferPtr(), key.GetDataSize(), problem ) );
            ASSERT_TRUE( store.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
            ASSERT_TRUE( TStore::KEYSTATE_OK == store.GetKeyState() );
            ASSERT_TRUE( store.Store( "google", "google:2", AUTH::CSecretString( "refresh-two" ) ) );
            ASSERT_TRUE( 1 == KeyFileTestCount( capture.Text(), "is usable" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An owner-only external key that others can read or write, a link or a FIFO is untrusted: nothing read or written" )
        GUCEF_TESTFW_SKIP_IF( !TStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString keyDir = MakeTestDirectory( "keyfile_untrusted_k" );
            CORE::CString dataDir = MakeTestDirectory( "keyfile_untrusted_d" );
            REQUIRE_TRUE( CORE::CreateDirs( keyDir ) );
            CORE::CString goodPath = CORE::CombinePath( keyDir, "good.key" );
            CORE::CString problem;
            REQUIRE_TRUE( TStore::MakeKeyFile( goodPath, problem ) );
            const CORE::CDynamicBuffer key = KeyFileTestRead( goodPath );
            {
                TStore store( dataDir );
                KeyFileTestSetup( store, goodPath );
                REQUIRE_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            }
            const CORE::CString dataPath = CORE::CombinePath( dataDir, dataName );
            const CORE::CDynamicBuffer data = KeyFileTestRead( dataPath );

            const mode_t modes[ 4 ] = { 0640, 0604, 0620, 0602 };
            CORE::CString badPath = CORE::CombinePath( keyDir, "bad.key" );
            for ( UInt32 i=0; i<4; ++i )
            {
                REQUIRE_TRUE( KeyFileTestWrite( badPath, key.GetConstBufferPtr(), key.GetDataSize(), modes[ i ] ) );
                TStore store( dataDir );
                KeyFileTestSetup( store, badPath );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
                ASSERT_FALSE( store.Store( "google", "google:2", AUTH::CSecretString( "x" ) ) );
                ASSERT_TRUE( TStore::KEYSTATE_UNTRUSTED == store.GetKeyState() && !store.GetKeyProblem().IsNULLOrEmpty() );
                ASSERT_FALSE( store.IsTamperDetected() );
            }

            // a symbolic link to the good key, with owner-only
            CORE::CString linkPath = CORE::CombinePath( keyDir, "link.key" );
            REQUIRE_TRUE( 0 == ::symlink( "good.key", linkPath.C_String() ) );
            {
                TStore store( dataDir );
                KeyFileTestSetup( store, linkPath );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( TStore::KEYSTATE_UNTRUSTED == store.GetKeyState() );
            }

            // a FIFO does not hold the read up
            CORE::CString fifoPath = CORE::CombinePath( keyDir, "fifo.key" );
            REQUIRE_TRUE( 0 == ::mkfifo( fifoPath.C_String(), 0600 ) );
            {
                TStore store( dataDir );
                KeyFileTestSetup( store, fifoPath );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( TStore::KEYSTATE_UNTRUSTED == store.GetKeyState() );
            }

            // the data and the good key are as they were
            ASSERT_TRUE( KeyFileTestSame( data, KeyFileTestRead( dataPath ) ) );
            ASSERT_TRUE( KeyFileTestSame( key, KeyFileTestRead( goodPath ) ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dataDir, localKeyName ) ) );

            // a key file of another owner needs a second user: to verify where the test can run as root
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CredentialStoreKeyFile: the owner rule (a key file of another user) is to verify as root" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An external key of the wrong size or format is invalid; 64 hexadecimal characters with or without a line break are the key" )
        GUCEF_TESTFW_SKIP_IF( !TStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString keyDir = MakeTestDirectory( "keyfile_format_k" );
            CORE::CString dataDir = MakeTestDirectory( "keyfile_format_d" );
            REQUIRE_TRUE( CORE::CreateDirs( keyDir ) );
            CORE::CString rawPath = CORE::CombinePath( keyDir, "raw.key" );
            CORE::CString problem;
            REQUIRE_TRUE( TStore::MakeKeyFile( rawPath, problem ) );
            const CORE::CDynamicBuffer key = KeyFileTestRead( rawPath );
            {
                TStore store( dataDir );
                KeyFileTestSetup( store, rawPath );
                REQUIRE_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            }
            const CORE::CString dataPath = CORE::CombinePath( dataDir, dataName );
            const CORE::CDynamicBuffer data = KeyFileTestRead( dataPath );
            const CORE::CString hex = KeyFileTestHex( key, false );

            CORE::CString formPath = CORE::CombinePath( keyDir, "form.key" );
            const char* invalidForms[ 6 ] = { "31", "33", "63hex", "hexx", "hexspace", "empty" };
            for ( UInt32 i=0; i<6; ++i )
            {
                const CORE::CString form = invalidForms[ i ];
                if ( "31" == form || "33" == form )
                {
                    CORE::CDynamicBuffer bytes( key );
                    if ( "33" == form )
                        bytes.Append( "x", 1 );
                    else
                        bytes.SetDataSize( 31 );
                    REQUIRE_TRUE( KeyFileTestWrite( formPath, bytes.GetConstBufferPtr(), bytes.GetDataSize(), 0600 ) );
                }
                else
                if ( "63hex" == form )
                    REQUIRE_TRUE( KeyFileTestWriteText( formPath, hex.SubstrToIndex( 63, true ), 0600 ) );
                else
                if ( "hexx" == form )
                    REQUIRE_TRUE( KeyFileTestWriteText( formPath, hex.SubstrToIndex( 63, true ) + "x", 0600 ) );
                else
                if ( "hexspace" == form )
                    REQUIRE_TRUE( KeyFileTestWriteText( formPath, hex + " \n", 0600 ) );
                else
                    REQUIRE_TRUE( KeyFileTestWrite( formPath, "", 0, 0600 ) );

                TStore store( dataDir );
                KeyFileTestSetup( store, formPath );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
                ASSERT_FALSE( store.Store( "google", "google:2", AUTH::CSecretString( "x" ) ) );
                ASSERT_TRUE( TStore::KEYSTATE_INVALID == store.GetKeyState() );
                ASSERT_FALSE( store.IsTamperDetected() );
            }
            ASSERT_TRUE( KeyFileTestSame( data, KeyFileTestRead( dataPath ) ) );

            // the text forms are the same key as the raw one
            const CORE::CString textForms[ 4 ] = { hex, hex + "\n", hex + "\r\n", KeyFileTestHex( key, true ) };
            for ( UInt32 i=0; i<4; ++i )
            {
                REQUIRE_TRUE( KeyFileTestWriteText( formPath, textForms[ i ], 0600 ) );
                TStore store( dataDir );
                KeyFileTestSetup( store, formPath );
                AUTH::CSecretString secret;
                ASSERT_TRUE( store.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
                ASSERT_TRUE( TStore::KEYSTATE_OK == store.GetKeyState() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A wrong external key is tamper detected and nothing is written; the right key again reads the store" )
        GUCEF_TESTFW_SKIP_IF( !TStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString keyDir = MakeTestDirectory( "keyfile_wrong_k" );
            CORE::CString dataDir = MakeTestDirectory( "keyfile_wrong_d" );
            REQUIRE_TRUE( CORE::CreateDirs( keyDir ) );
            CORE::CString rightPath = CORE::CombinePath( keyDir, "right.key" );
            CORE::CString wrongPath = CORE::CombinePath( keyDir, "wrong.key" );
            CORE::CString problem;
            REQUIRE_TRUE( TStore::MakeKeyFile( rightPath, problem ) && TStore::MakeKeyFile( wrongPath, problem ) );
            {
                TStore store( dataDir );
                KeyFileTestSetup( store, rightPath );
                REQUIRE_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            }
            const CORE::CString dataPath = CORE::CombinePath( dataDir, dataName );
            const CORE::CDynamicBuffer data = KeyFileTestRead( dataPath );

            TStore store( dataDir );
            KeyFileTestSetup( store, wrongPath );
            AUTH::CSecretString secret;
            ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
            ASSERT_TRUE( store.IsTamperDetected() );
            ASSERT_TRUE( TStore::KEYSTATE_OK == store.GetKeyState() );
            ASSERT_FALSE( store.Store( "google", "google:2", AUTH::CSecretString( "x" ) ) );
            ASSERT_TRUE( KeyFileTestSame( data, KeyFileTestRead( dataPath ) ) );

            store.SetKeyFile( rightPath );
            ASSERT_TRUE( store.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
            ASSERT_FALSE( store.IsTamperDetected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Wipe() with an external key takes the data away and leaves the key file as it is" )
        GUCEF_TESTFW_SKIP_IF( !TStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString keyDir = MakeTestDirectory( "keyfile_wipe_k" );
            CORE::CString dataDir = MakeTestDirectory( "keyfile_wipe_d" );
            REQUIRE_TRUE( CORE::CreateDirs( keyDir ) );
            CORE::CString keyPath = CORE::CombinePath( keyDir, "store.key" );
            CORE::CString problem;
            REQUIRE_TRUE( TStore::MakeKeyFile( keyPath, problem ) );
            const CORE::CDynamicBuffer key = KeyFileTestRead( keyPath );

            TStore store( dataDir );
            KeyFileTestSetup( store, keyPath );
            REQUIRE_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            ASSERT_TRUE( store.Wipe() );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dataDir, dataName ) ) );
            ASSERT_TRUE( KeyFileTestSame( key, KeyFileTestRead( keyPath ) ) );
            AUTH::CSecretString secret;
            ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
            // and it works again with the same key
            ASSERT_TRUE( store.Store( "google", "google:2", AUTH::CSecretString( "after-wipe" ) ) );
            ASSERT_TRUE( store.Load( "google", "google:2", secret ) && "after-wipe" == secret.Reveal() );
            ASSERT_TRUE( KeyFileTestSame( key, KeyFileTestRead( keyPath ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A stray local credentials.key next to an external key is never read and is named once in a warning" )
        GUCEF_TESTFW_SKIP_IF( !TStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString keyDir = MakeTestDirectory( "keyfile_stray_k" );
            CORE::CString dataDir = MakeTestDirectory( "keyfile_stray_d" );
            REQUIRE_TRUE( CORE::CreateDirs( keyDir ) );
            CORE::CString keyPath = CORE::CombinePath( keyDir, "store.key" );
            CORE::CString problem;
            REQUIRE_TRUE( TStore::MakeKeyFile( keyPath, problem ) );
            {
                TStore store( dataDir );
                KeyFileTestSetup( store, keyPath );
                REQUIRE_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
            }
            // another key where the local key would be
            const CORE::CString strayPath = CORE::CombinePath( dataDir, localKeyName );
            REQUIRE_TRUE( TStore::MakeKeyFile( strayPath, problem ) );
            const CORE::CDynamicBuffer stray = KeyFileTestRead( strayPath );

            CLogCapture capture;
            TStore store( dataDir );
            KeyFileTestSetup( store, keyPath );
            AUTH::CSecretString secret;
            ASSERT_TRUE( store.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
            ASSERT_TRUE( store.Store( "google", "google:2", AUTH::CSecretString( "refresh-two" ) ) );
            ASSERT_TRUE( store.Load( "google", "google:2", secret ) );
            ASSERT_FALSE( store.IsTamperDetected() );
            ASSERT_TRUE( 1 == KeyFileTestCount( capture.Text(), "is not used, the external key" ) );
            ASSERT_TRUE( KeyFileTestSame( stray, KeyFileTestRead( strayPath ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: mounted-secret reads a key through the ..data links of a simulated Kubernetes Secret volume and keeps the trust rules" )
        GUCEF_TESTFW_SKIP_IF( !TStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString mountDir = MakeTestDirectory( "keyfile_secret_mount" );
            CORE::CString dataDir = MakeTestDirectory( "keyfile_secret_d" );
            CORE::CString otherDir = MakeTestDirectory( "keyfile_secret_other" );
            REQUIRE_TRUE( CORE::CreateDirs( mountDir ) && CORE::CreateDirs( otherDir ) );
            CORE::CString problem;
            CORE::CString seedPath = CORE::CombinePath( otherDir, "seed.key" );
            REQUIRE_TRUE( TStore::MakeKeyFile( seedPath, problem ) );
            const CORE::CDynamicBuffer key = KeyFileTestRead( seedPath );

            // defaultMode 0440 with fsGroup: the group may read
            CORE::CString keyPath = MakeSecretVolume( mountDir, "credentials.key", key, 0440 );
            REQUIRE_TRUE( !keyPath.IsNULLOrEmpty() );

            {
                // the links within the volume are followed; the read only mount is emulated (a test cannot mount)
                TStore store( dataDir );
                KeyFileTestSetup( store, keyPath, TStore::KEYFILETRUST_MOUNTED_SECRET );
                AUTH::CFileCredentialStoreTestAccess::AssumeReadOnlyKeyMount( store, true );
                ASSERT_TRUE( TStore::KEYSTATE_OK == store.GetKeyState() );
                ASSERT_TRUE( store.Store( "google", "google:1", AUTH::CSecretString( "refresh-one" ) ) );
                TStore second( dataDir );
                KeyFileTestSetup( second, keyPath, TStore::KEYFILETRUST_MOUNTED_SECRET );
                AUTH::CFileCredentialStoreTestAccess::AssumeReadOnlyKeyMount( second, true );
                AUTH::CSecretString secret;
                ASSERT_TRUE( second.Load( "google", "google:1", secret ) && "refresh-one" == secret.Reveal() );
                ASSERT_TRUE( KeyFileTestSame( key, KeyFileTestRead( keyPath ) ) );
                ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dataDir, localKeyName ) ) );
            }
            {
                // the real rule: the test volume is writable, so the key is refused
                TStore store( dataDir );
                KeyFileTestSetup( store, keyPath, TStore::KEYFILETRUST_MOUNTED_SECRET );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( TStore::KEYSTATE_UNTRUSTED == store.GetKeyState() );
                ASSERT_TRUE( store.GetKeyProblem().HasSubstr( "read only" ) >= 0 );
                ASSERT_FALSE( store.IsTamperDetected() );
            }
            {
                // owner-only does not follow the link of the volume
                TStore store( dataDir );
                KeyFileTestSetup( store, keyPath, TStore::KEYFILETRUST_OWNER_ONLY );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( "google", "google:1", secret ) );
                ASSERT_TRUE( TStore::KEYSTATE_UNTRUSTED == store.GetKeyState() );
            }

            // a link that leads out of the volume
            CORE::CString escapePath = CORE::CombinePath( mountDir, "escape.key" );
            REQUIRE_TRUE( 0 == ::symlink( seedPath.C_String(), escapePath.C_String() ) );
            CORE::CDynamicBuffer content;
            bool isMissing = true;
            ASSERT_FALSE( CORE::ReadMountedSecretFile( escapePath, content, problem, &isMissing, true ) );
            ASSERT_TRUE( problem.HasSubstr( "out of the directory" ) >= 0 && !isMissing && 0 == content.GetDataSize() );
            {
                TStore store( dataDir );
                KeyFileTestSetup( store, escapePath, TStore::KEYFILETRUST_MOUNTED_SECRET );
                AUTH::CFileCredentialStoreTestAccess::AssumeReadOnlyKeyMount( store, true );
                ASSERT_TRUE( TStore::KEYSTATE_UNTRUSTED == store.GetKeyState() );
            }

            // permissions: others may not read, group and others may not write
            const mode_t refusedModes[ 3 ] = { 0444, 0460, 0442 };
            for ( UInt32 i=0; i<3; ++i )
            {
                CORE::CString otherMount = MakeTestDirectory( "keyfile_secret_mode" );
                REQUIRE_TRUE( CORE::CreateDirs( otherMount ) );
                CORE::CString modePath = MakeSecretVolume( otherMount, "credentials.key", key, refusedModes[ i ] );
                REQUIRE_TRUE( !modePath.IsNULLOrEmpty() );
                ASSERT_FALSE( CORE::ReadMountedSecretFile( modePath, content, problem, &isMissing, true ) );
                ASSERT_FALSE( problem.IsNULLOrEmpty() );
            }

            // the accepted modes; a FIFO in the volume is refused
            const mode_t acceptedModes[ 3 ] = { 0400, 0440, 0600 };
            for ( UInt32 i=0; i<3; ++i )
            {
                CORE::CString otherMount = MakeTestDirectory( "keyfile_secret_mode" );
                REQUIRE_TRUE( CORE::CreateDirs( otherMount ) );
                CORE::CString modePath = MakeSecretVolume( otherMount, "credentials.key", key, acceptedModes[ i ] );
                ASSERT_TRUE( CORE::ReadMountedSecretFile( modePath, content, problem, &isMissing, true ) );
                ASSERT_TRUE( KeyFileTestSame( key, content ) );
            }
            CORE::CString fifoPath = CORE::CombinePath( mountDir, "fifo.key" );
            REQUIRE_TRUE( 0 == ::mkfifo( fifoPath.C_String(), 0400 ) );
            ASSERT_FALSE( CORE::ReadMountedSecretFile( fifoPath, content, problem, &isMissing, true ) );
            ASSERT_TRUE( problem.HasSubstr( "regular" ) >= 0 );

            // missing, also behind a link that points nowhere
            ASSERT_FALSE( CORE::ReadMountedSecretFile( CORE::CombinePath( mountDir, "none.key" ), content, problem, &isMissing, true ) );
            ASSERT_TRUE( isMissing );
            CORE::CString dangling = CORE::CombinePath( mountDir, "dangling.key" );
            REQUIRE_TRUE( 0 == ::symlink( "..data/none.key", dangling.C_String() ) );
            isMissing = false;
            ASSERT_FALSE( CORE::ReadMountedSecretFile( dangling, content, problem, &isMissing, true ) );
            ASSERT_TRUE( isMissing );

            // root as the owner of the key needs root to make it: to verify in a real Kubernetes Secret mount
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CredentialStoreKeyFile: a root owned key file and a real read only mount are to verify in a Kubernetes pod" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #else

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Credential store key file TESTS of the file rules skipped: they are for Linux" );

    #endif /* GUCEF_PLATFORM_LINUX ? */

    GUCEF_TESTFW_TESTCASE( "Test: keyFile and keyFileTrust go through the config, are saved only when not the default, and an unknown trust falls back to owner-only" )
        try
        {
            AUTH::CFileCredentialStore defaults( "$CURWORKDIR$/authtemp/keyfile_config" );
            ASSERT_FALSE( defaults.HasExternalKeyFile() );
            ASSERT_TRUE( AUTH::CFileCredentialStore::KEYFILETRUST_OWNER_ONLY == defaults.GetKeyFileTrust() );
            ASSERT_TRUE( defaults.GetKeyFile() == CORE::CombinePath( defaults.GetDirectory(), AUTH::CFileCredentialStore::KeyFileName ) );
            CORE::CDataNode saved( "CredentialStore", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( defaults.SaveConfig( saved ) );
            ASSERT_TRUE( GUCEF_NULL == saved.GetAttribute( "keyFile" ) && GUCEF_NULL == saved.GetAttribute( "keyFileTrust" ) );

            CORE::CDataNode config( "CredentialStore", GUCEF_DATATYPE_OBJECT );
            config.SetAttribute( "directory", CORE::CString( "$CURWORKDIR$/authtemp/keyfile_config" ) );
            config.SetAttribute( "keyFile", CORE::CString( "$CURWORKDIR$/authtemp/keyfile_config_secret/credentials.key" ) );
            config.SetAttribute( "keyFileTrust", CORE::CString( "mounted-secret" ) );
            AUTH::CFileCredentialStore store;
            ASSERT_TRUE( store.LoadConfig( config ) );
            ASSERT_TRUE( store.HasExternalKeyFile() && store.GetKeyFile().HasSubstr( "keyfile_config_secret" ) >= 0 );
            ASSERT_TRUE( store.GetKeyFile().HasSubstr( "$CURWORKDIR$" ) < 0 );
            ASSERT_TRUE( AUTH::CFileCredentialStore::KEYFILETRUST_MOUNTED_SECRET == store.GetKeyFileTrust() );
            CORE::CDataNode roundTrip( "CredentialStore", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( store.SaveConfig( roundTrip ) );
            ASSERT_TRUE( "mounted-secret" == roundTrip.GetAttributeValueOrChildValueByName( "keyFileTrust" ).AsString() );
            ASSERT_TRUE( "$CURWORKDIR$/authtemp/keyfile_config_secret/credentials.key" == roundTrip.GetAttributeValueOrChildValueByName( "keyFile" ).AsString() );

            config.SetAttribute( "keyFileTrust", CORE::CString( "trust-me" ) );
            ASSERT_TRUE( store.LoadConfig( config ) );
            ASSERT_TRUE( AUTH::CFileCredentialStore::KEYFILETRUST_OWNER_ONLY == store.GetKeyFileTrust() );

            AUTH::CFileCredentialStore::EKeyFileTrust trust = AUTH::CFileCredentialStore::KEYFILETRUST_OWNER_ONLY;
            ASSERT_TRUE( AUTH::CFileCredentialStore::KeyFileTrustFromString( "mounted-secret", trust ) && AUTH::CFileCredentialStore::KEYFILETRUST_MOUNTED_SECRET == trust );
            ASSERT_FALSE( AUTH::CFileCredentialStore::KeyFileTrustFromString( "Owner-Only", trust ) );
            ASSERT_TRUE( "owner-only" == AUTH::CFileCredentialStore::KeyFileTrustToString( AUTH::CFileCredentialStore::KEYFILETRUST_OWNER_ONLY ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Code other than the tests cannot turn off the read only mount rule of mounted-secret" )
        try
        {
            // only the test seam (a friend) reaches the emulation of a read only mount
            const bool reachable = 0 != TypeHasReachableReadOnlyMountSetter< AUTH::CFileCredentialStore >::value;
            ASSERT_FALSE( reachable );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
