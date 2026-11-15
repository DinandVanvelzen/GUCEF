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

#ifndef MONOCYPHER_H
#include "monocypher.h"
#endif /* MONOCYPHER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestLogCapture.h"
#include "TestFiles.h"
#include "TestKeyring.h"

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

namespace {

typedef AUTH::CFileCredentialStore TStore;

/** Cheap Argon2id parameters for the tests: the file keeps what it was sealed with, so they are as real as the defaults */
const UInt32 TestMemoryInKiB = 64;
const UInt32 TestPasses = 1;
const UInt32 TestLanes = 1;

/** The codes of the tests; none of them may ever be seen in a log or a file */
const char* const RightCode = "correct horse battery staple 7f3a";
const char* const WrongCode = "correct horse battery stapel 7f3a";
const char* const NewCode = "a new keyring code, quite long 91c2";
const char* const SavedPassword = "s4ved-passw0rd-of-a-store-5e1d";

/** The offsets of the keyring file (format version 2) */
const UInt32 SaltOffset = 18;
const UInt32 CheckOffset = 34;
const UInt32 HeaderSize = 66;

/*-------------------------------------------------------------------------*/

void
SetUpTestStore( TStore& store )
{GUCEF_TRACE;

    store.SetKeyMode( TStore::KEYMODE_KEYRING_CODE );
    store.SetKeyringParameters( TestMemoryInKiB, TestPasses, TestLanes );
}

/*-------------------------------------------------------------------------*/

CORE::CDynamicBuffer
ReadFile( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    content.LoadContentFromFile( path );
    return content;
}

/*-------------------------------------------------------------------------*/

bool
WriteFile( const CORE::CString& path, const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    return content.WriteContentToFile( path, 0, true );
}

/*-------------------------------------------------------------------------*/

CORE::CDynamicBuffer
Repeated( UInt8 value, UInt32 count )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    for ( UInt32 i=0; i<count; ++i )
        buffer.Append( &value, 1 );
    return buffer;
}

/*-------------------------------------------------------------------------*/

/** Argon2id with Monocypher's own API: the reference the module is held to */
bool
MonocypherArgon2id( const void* pass, UInt32 passSize, const void* salt, UInt32 saltSize, UInt32 memoryInKiB, UInt32 passes, UInt32 lanes,
                    const void* key, UInt32 keySize, const void* ad, UInt32 adSize, UInt8* hash, UInt32 hashSize )
{GUCEF_TRACE;

    void* workArea = malloc( (size_t) memoryInKiB * 1024 );
    if ( GUCEF_NULL == workArea )
        return false;
    crypto_argon2_config config;
    config.algorithm = CRYPTO_ARGON2_ID;
    config.nb_blocks = memoryInKiB;
    config.nb_passes = passes;
    config.nb_lanes = lanes;
    crypto_argon2_inputs inputs;
    inputs.pass = static_cast< const uint8_t* >( pass );
    inputs.pass_size = passSize;
    inputs.salt = static_cast< const uint8_t* >( salt );
    inputs.salt_size = saltSize;
    crypto_argon2_extras extras = crypto_argon2_no_extras;
    if ( GUCEF_NULL != key )
    {
        extras.key = static_cast< const uint8_t* >( key );
        extras.key_size = keySize;
    }
    if ( GUCEF_NULL != ad )
    {
        extras.ad = static_cast< const uint8_t* >( ad );
        extras.ad_size = adSize;
    }
    crypto_argon2( hash, hashSize, workArea, config, inputs, extras );
    crypto_wipe( workArea, (size_t) memoryInKiB * 1024 );
    free( workArea );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
ReadUInt32At( const UInt8* bytes )
{GUCEF_TRACE;

    return (UInt32) bytes[ 0 ] | ( (UInt32) bytes[ 1 ] << 8 ) | ( (UInt32) bytes[ 2 ] << 16 ) | ( (UInt32) bytes[ 3 ] << 24 );
}

/*-------------------------------------------------------------------------*/

/**
 *  What the module derives from a code and the header of a keyring file, computed here from Monocypher's Argon2id and
 *  HKDF-SHA256: the 32 byte result of Argon2id, the sealing key and the check value
 */
bool
ReferenceKeyringKeys( const char* code, const CORE::CDynamicBuffer& file, UInt8* master, UInt8* sealKey, UInt8* check )
{GUCEF_TRACE;

    if ( file.GetDataSize() < HeaderSize )
        return false;
    const UInt8* bytes = static_cast< const UInt8* >( file.GetConstBufferPtr() );
    static const char sealInfo[] = "gucefAUTH keyring v2 seal";
    static const char checkInfo[] = "gucefAUTH keyring v2 check";
    return MonocypherArgon2id( code, (UInt32) strlen( code ), bytes + SaltOffset, 16, ReadUInt32At( bytes + 6 ), ReadUInt32At( bytes + 10 ),
                               ReadUInt32At( bytes + 14 ), GUCEF_NULL, 0, GUCEF_NULL, 0, master, 32 ) &&
           CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, master, 32, sealInfo, (UInt32) sizeof( sealInfo ) - 1, sealKey, 32 ) &&
           CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, master, 32, checkInfo, (UInt32) sizeof( checkInfo ) - 1, check, 32 );
}

/*-------------------------------------------------------------------------*/

bool
ContainsBytes( const CORE::CDynamicBuffer& content, const void* bytes, UInt32 size )
{GUCEF_TRACE;

    const UInt8* data = static_cast< const UInt8* >( content.GetConstBufferPtr() );
    for ( UInt32 i=0; i+size<=content.GetDataSize(); ++i )
        if ( 0 == memcmp( data + i, bytes, size ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** Records the events of a CKeyringSession */
class CKeyringEventRecorder : public CORE::CObservingNotifier
{
    public:

    UInt32 unlocked;
    UInt32 locked;
    UInt32 wrongCode;
    UInt32 retryAllowed;
    UInt32 setUp;
    UInt32 changed;
    UInt32 forgotten;
    UInt32 failed;
    AUTH::CKeyringEventData last;
    AUTH::TKeyringLockReason lastLockReason;

    explicit CKeyringEventRecorder( AUTH::CKeyringSession& session )
        : CORE::CObservingNotifier()
        , unlocked( 0 ), locked( 0 ), wrongCode( 0 ), retryAllowed( 0 ), setUp( 0 ), changed( 0 ), forgotten( 0 ), failed( 0 )
        , last()
        , lastLockReason( AUTH::KEYRINGLOCK_REQUESTED )
    {
        TCallback unlockedCallback( this, &CKeyringEventRecorder::OnUnlocked );
        SubscribeTo( &session, AUTH::CKeyringSession::UnlockedEvent, unlockedCallback );
        TCallback lockedCallback( this, &CKeyringEventRecorder::OnLocked );
        SubscribeTo( &session, AUTH::CKeyringSession::LockedEvent, lockedCallback );
        TCallback wrongCodeCallback( this, &CKeyringEventRecorder::OnWrongCode );
        SubscribeTo( &session, AUTH::CKeyringSession::WrongCodeEvent, wrongCodeCallback );
        TCallback retryCallback( this, &CKeyringEventRecorder::OnRetryAllowed );
        SubscribeTo( &session, AUTH::CKeyringSession::RetryAllowedEvent, retryCallback );
        TCallback setUpCallback( this, &CKeyringEventRecorder::OnSetUp );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeSetUpEvent, setUpCallback );
        TCallback changedCallback( this, &CKeyringEventRecorder::OnChanged );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeChangedEvent, changedCallback );
        TCallback forgottenCallback( this, &CKeyringEventRecorder::OnForgotten );
        SubscribeTo( &session, AUTH::CKeyringSession::ForgottenEvent, forgottenCallback );
        TCallback failedCallback( this, &CKeyringEventRecorder::OnFailed );
        SubscribeTo( &session, AUTH::CKeyringSession::FailedEvent, failedCallback );
    }

    virtual ~CKeyringEventRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CKeyringEventRecorder";
        return classTypeName;
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CKeyringEventRecorder > TCallback;

    void Take( CORE::CICloneable* eventData )
    {
        AUTH::CKeyringEventData* data = static_cast< AUTH::CKeyringEventData* >( eventData );
        if ( GUCEF_NULL != data )
            last = *data;
    }

    void OnUnlocked( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++unlocked; Take( eventData ); }
    void OnLocked( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++locked; Take( eventData ); lastLockReason = last.lockReason; }
    void OnWrongCode( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++wrongCode; Take( eventData ); }
    void OnRetryAllowed( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++retryAllowed; Take( eventData ); }
    void OnSetUp( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++setUp; Take( eventData ); }
    void OnChanged( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++changed; Take( eventData ); }
    void OnForgotten( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++forgotten; Take( eventData ); }
    void OnFailed( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++failed; Take( eventData ); }
};

/*-------------------------------------------------------------------------*/

/** Pumps until the future is ready; true when it was in time */
bool
PumpUntilReady( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return future.IsReady();
}

/*-------------------------------------------------------------------------*/

/** The monotonic clock of the module runs ahead for as long as this lives */
class CClockAhead
{
    public:
    CClockAhead( void ) {}
    ~CClockAhead() { AUTH::SetMonotonicClockOffsetInMs( 0 ); }
    void Set( UInt64 offsetInMs ) { AUTH::SetMonotonicClockOffsetInMs( offsetInMs ); }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformKeyringTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING keyring code TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Keyring" );

    const bool available = TStore::IsKeyringAvailable();

    GUCEF_TESTFW_TESTCASE( "Test: The argon2id codec gives the Argon2id answer of RFC 9106 (5.3) and the one of Monocypher itself" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsPassphraseKdfAvailable(), "No argon2id codec is registered" );
        try
        {
            CORE::CCodecRegistry::TICodecPtr codec = CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::KeyDerivationCodec, "argon2id" );
            REQUIRE_TRUE( !codec.IsNULL() );
            const CORE::CDynamicBuffer password = Repeated( 0x01, 32 );
            const CORE::CDynamicBuffer salt = Repeated( 0x02, 16 );
            const CORE::CDynamicBuffer secret = Repeated( 0x03, 8 );
            const CORE::CDynamicBuffer ad = Repeated( 0x04, 12 );
            CORE::CValueList params;
            params.Set( "salt", CORE::CVariant( salt ) );
            params.Set( "key", CORE::CVariant( secret ) );
            params.Set( "ad", CORE::CVariant( ad ) );
            params.Set( "iterations", CORE::CVariant( (UInt32) 3 ) );
            params.Set( "memory", CORE::CVariant( (UInt32) 32 ) );
            params.Set( "lanes", CORE::CVariant( (UInt32) 4 ) );
            params.Set( "length", CORE::CVariant( (UInt32) 32 ) );
            CORE::CDynamicBuffer input( password );
            CORE::CDynamicBuffer output;
            {
                CORE::CDynamicBufferAccess inputAccess( input );
                CORE::CDynamicBufferAccess outputAccess( output );
                REQUIRE_TRUE( codec->Encode( inputAccess, outputAccess, params ) );
            }
            REQUIRE_TRUE( 32 == output.GetDataSize() );
            ASSERT_TRUE( "0d640df58d78766c08c037a34a8b53c9d01ef0452d75b65eb52520e96b01e659" == AUTH::CAuthCrypto::ToHex( output.GetConstBufferPtr(), 32 ) );

            UInt8 reference[ 32 ];
            REQUIRE_TRUE( MonocypherArgon2id( password.GetConstBufferPtr(), 32, salt.GetConstBufferPtr(), 16, 32, 3, 4,
                                              secret.GetConstBufferPtr(), 8, ad.GetConstBufferPtr(), 12, reference, 32 ) );
            ASSERT_TRUE( 0 == memcmp( reference, output.GetConstBufferPtr(), 32 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CAuthCrypto::DeriveKeyFromPassphrase() is Argon2id as Monocypher computes it, for fixed inputs and several parameter sets" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsPassphraseKdfAvailable(), "No argon2id codec is registered" );
        try
        {
            UInt8 salt[ 16 ];
            for ( UInt32 i=0; i<16; ++i )
                salt[ i ] = (UInt8) ( 0xa0 + i );
            const char* code = RightCode;
            const UInt32 sets[ 4 ][ 4 ] = { { 64, 1, 1, 32 }, { 256, 2, 1, 32 }, { 64, 3, 2, 32 }, { 1024, 1, 4, 64 } };
            for ( UInt32 s=0; s<4; ++s )
            {
                UInt8 derived[ 64 ];
                UInt8 reference[ 64 ];
                REQUIRE_TRUE( AUTH::CAuthCrypto::DeriveKeyFromPassphrase( code, (UInt32) strlen( code ), salt, 16, sets[ s ][ 0 ], sets[ s ][ 1 ], sets[ s ][ 2 ], derived, sets[ s ][ 3 ] ) );
                REQUIRE_TRUE( MonocypherArgon2id( code, (UInt32) strlen( code ), salt, 16, sets[ s ][ 0 ], sets[ s ][ 1 ], sets[ s ][ 2 ], GUCEF_NULL, 0, GUCEF_NULL, 0, reference, sets[ s ][ 3 ] ) );
                ASSERT_TRUE( 0 == memcmp( derived, reference, sets[ s ][ 3 ] ) );
            }
            // another code or another salt gives another key; parameters the codec refuses give nothing
            UInt8 first[ 32 ];
            UInt8 second[ 32 ];
            REQUIRE_TRUE( AUTH::CAuthCrypto::DeriveKeyFromPassphrase( code, (UInt32) strlen( code ), salt, 16, 64, 1, 1, first, 32 ) );
            REQUIRE_TRUE( AUTH::CAuthCrypto::DeriveKeyFromPassphrase( WrongCode, (UInt32) strlen( WrongCode ), salt, 16, 64, 1, 1, second, 32 ) );
            ASSERT_FALSE( 0 == memcmp( first, second, 32 ) );
            salt[ 0 ] ^= 1;
            REQUIRE_TRUE( AUTH::CAuthCrypto::DeriveKeyFromPassphrase( code, (UInt32) strlen( code ), salt, 16, 64, 1, 1, second, 32 ) );
            ASSERT_FALSE( 0 == memcmp( first, second, 32 ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::DeriveKeyFromPassphrase( code, (UInt32) strlen( code ), salt, 4, 64, 1, 1, second, 32 ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::DeriveKeyFromPassphrase( code, (UInt32) strlen( code ), salt, 16, 4, 1, 1, second, 32 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The keyring file is version 2 with Argon2id, its parameters, a salt and the check value Monocypher computes from the code; it opens with the key derived the same way" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_format" );
            TStore store( dir );
            SetUpTestStore( store );
            store.SetKeyringParameters( 128, 2, 2 );
            ASSERT_FALSE( store.IsKeyringSetUp() );
            ASSERT_TRUE( store.IsKeyringLocked() );
            REQUIRE_TRUE( AUTH::KEYRING_OK == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
            ASSERT_FALSE( store.IsKeyringLocked() );
            ASSERT_TRUE( store.Store( "fab", "owner@example.org", AUTH::CSecretString( SavedPassword ) ) );

            CORE::CString path = CORE::CombinePath( dir, TStore::KeyringFileName );
            CORE::CDynamicBuffer file = ReadFile( path );
            REQUIRE_TRUE( file.GetDataSize() > HeaderSize + 24 + 16 );
            const UInt8* bytes = static_cast< const UInt8* >( file.GetConstBufferPtr() );
            ASSERT_TRUE( 0 == memcmp( bytes, "GAUC", 4 ) );
            ASSERT_TRUE( 2 == bytes[ 4 ] && 1 == bytes[ 5 ] );
            ASSERT_TRUE( 128 == ReadUInt32At( bytes + 6 ) && 2 == ReadUInt32At( bytes + 10 ) && 2 == ReadUInt32At( bytes + 14 ) );
            UInt32 memoryInKiB = 0, passes = 0, lanes = 0;
            ASSERT_TRUE( store.GetSealedKeyringParameters( memoryInKiB, passes, lanes ) && 128 == memoryInKiB && 2 == passes && 2 == lanes );

            UInt8 master[ 32 ];
            UInt8 sealKey[ 32 ];
            UInt8 check[ 32 ];
            REQUIRE_TRUE( ReferenceKeyringKeys( RightCode, file, master, sealKey, check ) );
            ASSERT_TRUE( 0 == memcmp( bytes + CheckOffset, check, 32 ) );
            // the reference key opens the data, with the header as additional data
            CORE::CDynamicBuffer plain;
            ASSERT_TRUE( AUTH::CAuthCrypto::AeadOpen( sealKey, bytes + HeaderSize, bytes, HeaderSize, bytes + HeaderSize + 24, file.GetDataSize() - HeaderSize - 24, plain ) );
            ASSERT_TRUE( BufferContains( plain, SavedPassword ) );
            // no key file, no data of the key file mode
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, TStore::KeyFileName ) ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, TStore::DataFileName ) ) );

            // a second set up is refused; the salt of two set ups differs
            ASSERT_TRUE( AUTH::KEYRING_ALREADY_SET_UP == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
            CORE::CString otherDir = MakeTestDirectory( "keyring_format2" );
            TStore other( otherDir );
            SetUpTestStore( other );
            REQUIRE_TRUE( AUTH::KEYRING_OK == other.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
            CORE::CDynamicBuffer otherFile = ReadFile( CORE::CombinePath( otherDir, TStore::KeyringFileName ) );
            ASSERT_FALSE( 0 == memcmp( bytes + SaltOffset, static_cast< const UInt8* >( otherFile.GetConstBufferPtr() ) + SaltOffset, 16 ) );

            // codes that cannot be codes
            ASSERT_TRUE( AUTH::KEYRING_INVALID_CODE == other.ChangeKeyringCode( AUTH::CSecretString( RightCode ), AUTH::CSecretString() ) );
            CORE::CString tooLong;
            for ( UInt32 i=0; i<TStore::MaxKeyringCodeLength + 1; ++i )
                tooLong += 'x';
            ASSERT_TRUE( AUTH::KEYRING_INVALID_CODE == other.ChangeKeyringCode( AUTH::CSecretString( RightCode ), AUTH::CSecretString( tooLong ) ) );
            // a store of the key file mode has no keyring
            TStore keyFileStore( otherDir );
            ASSERT_TRUE( AUTH::KEYRING_NOT_KEYRING_MODE == keyFileStore.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
            ASSERT_FALSE( keyFileStore.IsKeyringLocked() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A locked keyring reads and changes nothing; a wrong code leaves it locked and is no tampering; the right code opens it" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_unlock" );
            {
                TStore store( dir );
                SetUpTestStore( store );
                ASSERT_TRUE( AUTH::KEYRING_NOT_SET_UP == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
                ASSERT_FALSE( store.Store( "fab", "owner", AUTH::CSecretString( SavedPassword ) ) );
                REQUIRE_TRUE( AUTH::KEYRING_OK == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
                ASSERT_TRUE( store.Store( "fab", "owner", AUTH::CSecretString( SavedPassword ) ) );
                ASSERT_TRUE( store.Store( "cosmos", "owner", AUTH::CSecretString( "another-one" ) ) );
                store.LockKeyring();
                ASSERT_TRUE( store.IsKeyringLocked() );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( "fab", "owner", secret ) );
            }

            TStore store( dir );
            SetUpTestStore( store );
            ASSERT_TRUE( store.IsKeyringSetUp() && store.IsKeyringLocked() );
            CORE::CDynamicBuffer before = ReadFile( CORE::CombinePath( dir, TStore::KeyringFileName ) );
            AUTH::CSecretString secret;
            CORE::CString::StringVector accounts;
            ASSERT_FALSE( store.Load( "fab", "owner", secret ) );
            ASSERT_TRUE( secret.IsEmpty() );
            ASSERT_FALSE( store.ListAccounts( "fab", accounts ) );
            ASSERT_FALSE( store.Store( "fab", "other", AUTH::CSecretString( "x" ) ) );
            ASSERT_FALSE( store.Remove( "fab", "owner" ) );
            ASSERT_FALSE( store.RemoveProvider( "fab" ) );
            ASSERT_FALSE( store.IsTamperDetected() );

            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == store.UnlockKeyring( AUTH::CSecretString( WrongCode ) ) );
            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == store.UnlockKeyring( AUTH::CSecretString() ) );
            ASSERT_TRUE( store.IsKeyringLocked() );
            ASSERT_FALSE( store.IsTamperDetected() );
            ASSERT_FALSE( store.Load( "fab", "owner", secret ) );
            ASSERT_TRUE( 0 == store.GetKeyringLastUseInMs() );
            // nothing of the above changed the file
            CORE::CDynamicBuffer after = ReadFile( CORE::CombinePath( dir, TStore::KeyringFileName ) );
            ASSERT_TRUE( before.GetDataSize() == after.GetDataSize() && 0 == memcmp( before.GetConstBufferPtr(), after.GetConstBufferPtr(), before.GetDataSize() ) );

            ASSERT_TRUE( AUTH::KEYRING_OK == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
            ASSERT_FALSE( store.IsKeyringLocked() );
            ASSERT_TRUE( store.Load( "fab", "owner", secret ) && SavedPassword == secret.Reveal() );
            ASSERT_TRUE( store.ListAccounts( "cosmos", accounts ) && 1 == accounts.size() );
            ASSERT_TRUE( store.GetKeyringLastUseInMs() > 0 );
            ASSERT_TRUE( store.Store( "fab", "second", AUTH::CSecretString( "two" ) ) );
            ASSERT_TRUE( store.Remove( "cosmos", "owner" ) );
            ASSERT_FALSE( store.IsTamperDetected() );

            // a second instance sees the changes once it is unlocked too
            TStore second( dir );
            SetUpTestStore( second );
            ASSERT_TRUE( AUTH::KEYRING_OK == second.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( second.Load( "fab", "second", secret ) && "two" == secret.Reveal() );
            ASSERT_FALSE( second.Load( "cosmos", "owner", secret ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A wrong code is not tampering; changed data with the right code is tampering; a file cut short or of a newer version is reported as such" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_tamper" );
            CORE::CString path = CORE::CombinePath( dir, TStore::KeyringFileName );
            {
                TStore store( dir );
                SetUpTestStore( store );
                REQUIRE_TRUE( AUTH::KEYRING_OK == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
                REQUIRE_TRUE( store.Store( "fab", "owner", AUTH::CSecretString( SavedPassword ) ) );
            }
            const CORE::CDynamicBuffer good = ReadFile( path );

            // a byte of the sealed data changed
            CORE::CDynamicBuffer changed( good );
            static_cast< UInt8* >( changed.GetBufferPtr() )[ changed.GetDataSize() - 20 ] ^= 0x40;
            REQUIRE_TRUE( WriteFile( path, changed ) );
            {
                TStore store( dir );
                SetUpTestStore( store );
                ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == store.UnlockKeyring( AUTH::CSecretString( WrongCode ) ) );
                ASSERT_FALSE( store.IsTamperDetected() );
                ASSERT_TRUE( AUTH::KEYRING_TAMPERED == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
                ASSERT_TRUE( store.IsTamperDetected() );
                ASSERT_TRUE( store.IsKeyringLocked() );
                AUTH::CSecretString secret;
                ASSERT_FALSE( store.Load( "fab", "owner", secret ) );
                ASSERT_FALSE( store.Store( "fab", "owner", AUTH::CSecretString( "x" ) ) );
                // nothing is written to it
                CORE::CDynamicBuffer now = ReadFile( path );
                ASSERT_TRUE( now.GetDataSize() == changed.GetDataSize() && 0 == memcmp( now.GetConstBufferPtr(), changed.GetConstBufferPtr(), now.GetDataSize() ) );
            }

            // the authenticated header changed (a parameter raised by hand): the check value of the code no longer matches
            CORE::CDynamicBuffer header( good );
            static_cast< UInt8* >( header.GetBufferPtr() )[ 10 ] = 2;
            REQUIRE_TRUE( WriteFile( path, header ) );
            {
                TStore store( dir );
                SetUpTestStore( store );
                ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
            }

            // cut short, a header that asks for absurd parameters, another magic: not a keyring file
            CORE::CDynamicBuffer shortFile( static_cast< const char* >( good.GetConstBufferPtr() ), 40, true );
            REQUIRE_TRUE( WriteFile( path, shortFile ) );
            {
                TStore store( dir );
                SetUpTestStore( store );
                ASSERT_TRUE( AUTH::KEYRING_TAMPERED == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
                ASSERT_TRUE( store.IsTamperDetected() );
            }
            CORE::CDynamicBuffer absurd( good );
            static_cast< UInt8* >( absurd.GetBufferPtr() )[ 13 ] = 0x7f;   // passes far above the maximum
            REQUIRE_TRUE( WriteFile( path, absurd ) );
            {
                TStore store( dir );
                SetUpTestStore( store );
                ASSERT_TRUE( AUTH::KEYRING_TAMPERED == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
            }
            CORE::CDynamicBuffer newer( good );
            static_cast< UInt8* >( newer.GetBufferPtr() )[ 4 ] = 3;
            REQUIRE_TRUE( WriteFile( path, newer ) );
            {
                TStore store( dir );
                SetUpTestStore( store );
                ASSERT_TRUE( AUTH::KEYRING_NEWER_VERSION == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
                ASSERT_FALSE( store.IsTamperDetected() );
            }
            // the original file opens again
            REQUIRE_TRUE( WriteFile( path, good ) );
            {
                TStore store( dir );
                SetUpTestStore( store );
                ASSERT_TRUE( AUTH::KEYRING_OK == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
                ASSERT_FALSE( store.IsTamperDetected() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The key file mode and the keyring live side by side in one directory; neither reads, writes or wipes the files of the other" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_side" );
            TStore cookies( dir );
            ASSERT_TRUE( TStore::KEYMODE_KEY_FILE == cookies.GetKeyMode() );
            ASSERT_TRUE( cookies.Store( "cosmos", "session", AUTH::CSecretString( "cookie-value" ) ) );
            TStore passwords( dir );
            SetUpTestStore( passwords );
            REQUIRE_TRUE( AUTH::KEYRING_OK == passwords.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( passwords.Store( "cosmos", "owner", AUTH::CSecretString( SavedPassword ) ) );
            CORE::CDynamicBuffer data = ReadFile( CORE::CombinePath( dir, TStore::DataFileName ) );
            CORE::CDynamicBuffer key = ReadFile( CORE::CombinePath( dir, TStore::KeyFileName ) );

            AUTH::CSecretString secret;
            ASSERT_TRUE( cookies.Load( "cosmos", "session", secret ) && "cookie-value" == secret.Reveal() );
            ASSERT_FALSE( cookies.Load( "cosmos", "owner", secret ) );
            ASSERT_TRUE( passwords.Load( "cosmos", "owner", secret ) && SavedPassword == secret.Reveal() );
            ASSERT_FALSE( passwords.Load( "cosmos", "session", secret ) );

            // forgetting the code leaves the cookies; wiping the cookies leaves the keyring
            ASSERT_TRUE( passwords.Wipe() );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, TStore::KeyringFileName ) ) );
            ASSERT_TRUE( passwords.IsKeyringLocked() && TStore::KEYMODE_KEYRING_CODE == passwords.GetKeyMode() );
            ASSERT_TRUE( AUTH::KEYRINGLOCK_FORGOTTEN == passwords.GetKeyringLockReason() );
            CORE::CDynamicBuffer dataAfter = ReadFile( CORE::CombinePath( dir, TStore::DataFileName ) );
            CORE::CDynamicBuffer keyAfter = ReadFile( CORE::CombinePath( dir, TStore::KeyFileName ) );
            ASSERT_TRUE( data.GetDataSize() == dataAfter.GetDataSize() && 0 == memcmp( data.GetConstBufferPtr(), dataAfter.GetConstBufferPtr(), data.GetDataSize() ) );
            ASSERT_TRUE( key.GetDataSize() == keyAfter.GetDataSize() && 0 == memcmp( key.GetConstBufferPtr(), keyAfter.GetConstBufferPtr(), key.GetDataSize() ) );
            ASSERT_TRUE( cookies.Load( "cosmos", "session", secret ) );

            REQUIRE_TRUE( AUTH::KEYRING_OK == passwords.SetUpKeyring( AUTH::CSecretString( NewCode ) ) );
            ASSERT_TRUE( cookies.Wipe() );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( dir, TStore::KeyringFileName ) ) );
            ASSERT_FALSE( passwords.IsKeyringLocked() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A change of code needs the current code, reseals with a new salt and the current parameters and keeps the credentials; the old code no longer opens it" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_change" );
            CORE::CString path = CORE::CombinePath( dir, TStore::KeyringFileName );
            TStore store( dir );
            SetUpTestStore( store );
            REQUIRE_TRUE( AUTH::KEYRING_OK == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
            REQUIRE_TRUE( store.Store( "fab", "owner", AUTH::CSecretString( SavedPassword ) ) );
            CORE::CDynamicBuffer before = ReadFile( path );

            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == store.ChangeKeyringCode( AUTH::CSecretString( WrongCode ), AUTH::CSecretString( NewCode ) ) );
            CORE::CDynamicBuffer unchanged = ReadFile( path );
            ASSERT_TRUE( before.GetDataSize() == unchanged.GetDataSize() && 0 == memcmp( before.GetConstBufferPtr(), unchanged.GetConstBufferPtr(), before.GetDataSize() ) );

            // the parameters are raised with the change
            store.SetKeyringParameters( 256, 2, 1 );
            ASSERT_TRUE( AUTH::KEYRING_OK == store.ChangeKeyringCode( AUTH::CSecretString( RightCode ), AUTH::CSecretString( NewCode ) ) );
            ASSERT_FALSE( store.IsKeyringLocked() );
            CORE::CDynamicBuffer after = ReadFile( path );
            ASSERT_FALSE( 0 == memcmp( static_cast< const UInt8* >( before.GetConstBufferPtr() ) + SaltOffset, static_cast< const UInt8* >( after.GetConstBufferPtr() ) + SaltOffset, 16 ) );
            UInt32 memoryInKiB = 0, passes = 0, lanes = 0;
            ASSERT_TRUE( store.GetSealedKeyringParameters( memoryInKiB, passes, lanes ) && 256 == memoryInKiB && 2 == passes && 1 == lanes );
            UInt8 master[ 32 ], sealKey[ 32 ], check[ 32 ];
            REQUIRE_TRUE( ReferenceKeyringKeys( NewCode, after, master, sealKey, check ) );
            ASSERT_TRUE( 0 == memcmp( static_cast< const UInt8* >( after.GetConstBufferPtr() ) + CheckOffset, check, 32 ) );

            AUTH::CSecretString secret;
            ASSERT_TRUE( store.Load( "fab", "owner", secret ) && SavedPassword == secret.Reveal() );
            ASSERT_TRUE( store.Store( "fab", "later", AUTH::CSecretString( "after the change" ) ) );

            TStore other( dir );
            SetUpTestStore( other );
            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == other.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( AUTH::KEYRING_OK == other.UnlockKeyring( AUTH::CSecretString( NewCode ) ) );
            ASSERT_TRUE( other.Load( "fab", "owner", secret ) && SavedPassword == secret.Reveal() );
            ASSERT_TRUE( other.Load( "fab", "later", secret ) && "after the change" == secret.Reveal() );

            // a change by another instance locks this one at its next use: its key no longer fits the file
            ASSERT_TRUE( AUTH::KEYRING_OK == other.ChangeKeyringCode( AUTH::CSecretString( NewCode ), AUTH::CSecretString( RightCode ) ) );
            ASSERT_FALSE( store.Load( "fab", "owner", secret ) );
            ASSERT_TRUE( store.IsKeyringLocked() );
            ASSERT_TRUE( AUTH::KEYRINGLOCK_CHANGED_ELSEWHERE == store.GetKeyringLockReason() );
            ASSERT_FALSE( store.IsTamperDetected() );
            ASSERT_TRUE( AUTH::KEYRING_OK == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A keyring copied to another machine (another directory, no key file anywhere) opens with the code and nothing else" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_here" );
            {
                TStore store( dir );
                SetUpTestStore( store );
                REQUIRE_TRUE( AUTH::KEYRING_OK == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
                REQUIRE_TRUE( store.Store( "humble", "owner", AUTH::CSecretString( SavedPassword ) ) );
            }
            CORE::CString elsewhere = MakeTestDirectory( "keyring_elsewhere" );
            REQUIRE_TRUE( CORE::CreateDirs( elsewhere ) );
            REQUIRE_TRUE( WriteFile( CORE::CombinePath( elsewhere, TStore::KeyringFileName ), ReadFile( CORE::CombinePath( dir, TStore::KeyringFileName ) ) ) );
            CORE::DeleteDir( dir, true );

            TStore moved( elsewhere );
            SetUpTestStore( moved );
            moved.SetKeyringParameters( 4096, 4, 2 );     // the file says how it was sealed, not the settings
            ASSERT_TRUE( moved.IsKeyringLocked() && moved.IsKeyringSetUp() );
            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == moved.UnlockKeyring( AUTH::CSecretString( WrongCode ) ) );
            ASSERT_TRUE( AUTH::KEYRING_OK == moved.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
            AUTH::CSecretString secret;
            ASSERT_TRUE( moved.Load( "humble", "owner", secret ) && SavedPassword == secret.Reveal() );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( elsewhere, TStore::KeyFileName ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The session reports a wrong code apart from tampering, waits after freeAttempts wrong codes (doubling, capped), refuses without trying meanwhile and says when to try again" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_throttle" );
            TStore store( dir );
            SetUpTestStore( store );
            REQUIRE_TRUE( AUTH::KEYRING_OK == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
            store.LockKeyring();

            AUTH::CKeyringSession session( &store );
            AUTH::CKeyringSettings settings;
            settings.freeAttempts = 2;
            settings.retryDelayInMs = 300;
            settings.maxRetryDelayInMs = 1000;
            settings.checkIntervalInMs = 10;
            session.SetSettings( settings );
            CKeyringEventRecorder events( session );

            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == session.UnlockWithCode( AUTH::CSecretString( WrongCode ) ) );
            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == session.UnlockWithCode( AUTH::CSecretString( WrongCode ) ) );
            ASSERT_TRUE( 2 == events.wrongCode && 0 == session.GetRetryWaitInMs() && 2 == session.GetFailedAttemptCount() );
            // the third: a wait of retryDelayInMs
            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == session.UnlockWithCode( AUTH::CSecretString( WrongCode ) ) );
            ASSERT_TRUE( 3 == events.wrongCode && 3 == events.last.failedAttempts );
            ASSERT_TRUE( events.last.retryWaitInMs > 200 && events.last.retryWaitInMs <= 300 );
            ASSERT_TRUE( session.GetRetryWaitInMs() > 0 );
            // meanwhile even the right code is refused at once, without a derivation, and does not count
            UInt64 before = AUTH::GetMonotonicTimeInMs();
            ASSERT_TRUE( AUTH::KEYRING_THROTTLED == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( AUTH::GetMonotonicTimeInMs() - before < 50 );
            ASSERT_TRUE( AUTH::KEYRING_THROTTLED == session.ChangeCode( AUTH::CSecretString( RightCode ), AUTH::CSecretString( NewCode ) ) );
            ASSERT_TRUE( AUTH::KEYRING_THROTTLED == session.GetLastResult() );
            ASSERT_TRUE( 3 == session.GetFailedAttemptCount() && 3 == events.wrongCode );
            ASSERT_TRUE( session.IsLocked() );
            // an asynchronous attempt is refused the same way: its future has ended in an error status and says why
            CORE::CFutureResult refused = session.UnlockWithCodeAsync( AUTH::CSecretString( RightCode ) );
            ASSERT_TRUE( refused.IsReady() && refused.HasNoFuture() && AUTH::KEYRING_THROTTLED == session.GetLastResult() );
            ASSERT_TRUE( AUTH::KEYRING_THROTTLED == AUTH::CKeyringSession::GetResultOfFuture( refused ) );

            ASSERT_TRUE( PumpUntilAtLeast( events.retryAllowed, 1, 2000 ) );
            ASSERT_TRUE( 0 == session.GetRetryWaitInMs() );
            // the fourth wrong code waits twice as long, the fifth would be capped at maxRetryDelayInMs
            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == session.UnlockWithCode( AUTH::CSecretString( WrongCode ) ) );
            ASSERT_TRUE( events.last.retryWaitInMs > 500 && events.last.retryWaitInMs <= 600 );
            ASSERT_TRUE( PumpUntilAtLeast( events.retryAllowed, 2, 3000 ) );
            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == session.UnlockWithCode( AUTH::CSecretString( WrongCode ) ) );
            ASSERT_TRUE( events.last.retryWaitInMs > 900 && events.last.retryWaitInMs <= 1000 );
            ASSERT_TRUE( PumpUntilAtLeast( events.retryAllowed, 3, 3000 ) );

            // the right code opens and resets the count; nothing was tampered with
            ASSERT_TRUE( AUTH::KEYRING_OK == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( 1 == events.unlocked && 0 == session.GetFailedAttemptCount() && 0 == events.failed );
            ASSERT_FALSE( session.IsLocked() );
            ASSERT_FALSE( store.IsTamperDetected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The session locks after idleLockInSeconds without use of the key, any use of the store counts; never with 0; LockNow and Shutdown lock with their reason" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CClockAhead clock;
            CORE::CString dir = MakeTestDirectory( "keyring_idle" );
            TStore store( dir );
            SetUpTestStore( store );
            REQUIRE_TRUE( AUTH::KEYRING_OK == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
            REQUIRE_TRUE( store.Store( "fab", "owner", AUTH::CSecretString( SavedPassword ) ) );
            store.LockKeyring();

            AUTH::CKeyringSession session( &store );
            AUTH::CKeyringSettings settings;
            settings.idleLockInSeconds = 60;
            settings.checkIntervalInMs = 5;
            session.SetSettings( settings );
            CKeyringEventRecorder events( session );
            REQUIRE_TRUE( AUTH::KEYRING_OK == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( 1 == events.unlocked );

            // 40 s later the key is used: the idle time starts again
            clock.Set( 40000 );
            PumpFor( 30 );
            AUTH::CSecretString secret;
            ASSERT_TRUE( store.Load( "fab", "owner", secret ) );
            clock.Set( 80000 );
            PumpFor( 30 );
            ASSERT_FALSE( session.IsLocked() );
            ASSERT_TRUE( 0 == events.locked );
            // 60 s after the last use it locks
            clock.Set( 101000 );
            ASSERT_TRUE( PumpUntilAtLeast( events.locked, 1, 2000 ) );
            ASSERT_TRUE( AUTH::KEYRINGLOCK_IDLE == events.lastLockReason );
            ASSERT_TRUE( session.IsLocked() && store.IsKeyringLocked() );
            ASSERT_FALSE( store.Load( "fab", "owner", secret ) );

            // 0: never
            settings.idleLockInSeconds = 0;
            session.SetSettings( settings );
            REQUIRE_TRUE( AUTH::KEYRING_OK == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            clock.Set( 101000 + 7 * 24 * 3600 * (UInt64) 1000 );
            PumpFor( 30 );
            ASSERT_FALSE( session.IsLocked() );

            session.LockNow();
            ASSERT_TRUE( 2 == events.locked && AUTH::KEYRINGLOCK_REQUESTED == events.lastLockReason );
            REQUIRE_TRUE( AUTH::KEYRING_OK == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            session.Shutdown();
            ASSERT_TRUE( 3 == events.locked && AUTH::KEYRINGLOCK_APPLICATION_END == events.lastLockReason );
            ASSERT_TRUE( store.IsKeyringLocked() );
            ASSERT_TRUE( AUTH::KEYRING_FAILED == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( store.IsKeyringLocked() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The session sets up, changes and forgets the code with its events; a lock by another instance reaches the session as LockedEvent" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_session" );
            TStore store( dir );
            SetUpTestStore( store );
            AUTH::CKeyringSession session( &store );
            AUTH::CKeyringSettings settings;
            settings.checkIntervalInMs = 5;
            session.SetSettings( settings );
            CKeyringEventRecorder events( session );
            ASSERT_FALSE( session.IsSetUp() );
            ASSERT_TRUE( session.IsLocked() );
            ASSERT_TRUE( AUTH::KEYRING_NOT_SET_UP == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( 1 == events.failed && AUTH::KEYRING_NOT_SET_UP == events.last.result );

            ASSERT_TRUE( AUTH::KEYRING_OK == session.SetUpCode( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( 1 == events.setUp && 1 == events.unlocked && session.IsSetUp() && !session.IsLocked() );
            ASSERT_TRUE( store.Store( "fab", "owner", AUTH::CSecretString( SavedPassword ) ) );

            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == session.ChangeCode( AUTH::CSecretString( WrongCode ), AUTH::CSecretString( NewCode ) ) );
            ASSERT_TRUE( 1 == events.wrongCode && 1 == session.GetFailedAttemptCount() );
            ASSERT_TRUE( AUTH::KEYRING_OK == session.ChangeCode( AUTH::CSecretString( RightCode ), AUTH::CSecretString( NewCode ) ) );
            ASSERT_TRUE( 1 == events.changed && 1 == events.unlocked && 0 == session.GetFailedAttemptCount() );

            // another instance changes the code: this keyring locks at its next use and the session says so
            TStore other( dir );
            SetUpTestStore( other );
            REQUIRE_TRUE( AUTH::KEYRING_OK == other.ChangeKeyringCode( AUTH::CSecretString( NewCode ), AUTH::CSecretString( RightCode ) ) );
            AUTH::CSecretString secret;
            ASSERT_FALSE( store.Load( "fab", "owner", secret ) );
            ASSERT_TRUE( PumpUntilAtLeast( events.locked, 1, 2000 ) );
            ASSERT_TRUE( AUTH::KEYRINGLOCK_CHANGED_ELSEWHERE == events.lastLockReason );

            ASSERT_TRUE( AUTH::KEYRING_OK == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( 2 == events.unlocked );
            ASSERT_TRUE( session.Forget() );
            ASSERT_TRUE( 1 == events.forgotten && 2 == events.locked && AUTH::KEYRINGLOCK_FORGOTTEN == events.lastLockReason );
            ASSERT_FALSE( session.IsSetUp() );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, TStore::KeyringFileName ) ) );
            // the credentials are gone with the code: a new code starts empty
            ASSERT_TRUE( AUTH::KEYRING_OK == session.SetUpCode( AUTH::CSecretString( NewCode ) ) );
            ASSERT_FALSE( store.Load( "fab", "owner", secret ) );
            ASSERT_TRUE( 2 == events.setUp );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The asynchronous unlock runs on a pool thread: reads on this thread are answered (locked) meanwhile, a second operation is busy, the events come on the pulse" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_async" );
            TStore store( dir );
            SetUpTestStore( store );
            // a derivation that takes a noticeable time
            store.SetKeyringParameters( 32768, 3, 1 );
            REQUIRE_TRUE( AUTH::KEYRING_OK == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
            REQUIRE_TRUE( store.Store( "fab", "owner", AUTH::CSecretString( SavedPassword ) ) );
            store.LockKeyring();

            AUTH::CKeyringSession session( &store );
            CKeyringEventRecorder events( session );
            UInt64 started = AUTH::GetMonotonicTimeInMs();
            CORE::CFutureResult future = session.UnlockWithCodeAsync( AUTH::CSecretString( RightCode ) );
            ASSERT_TRUE( future.HasAFuture() );
            ASSERT_TRUE( session.IsBusy() );
            ASSERT_TRUE( AUTH::KEYRING_BUSY == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            CORE::CFutureResult second = session.UnlockWithCodeAsync( AUTH::CSecretString( RightCode ) );
            ASSERT_TRUE( second.IsReady() && second.HasNoFuture() && AUTH::KEYRING_BUSY == session.GetLastResult() );
            ASSERT_TRUE( AUTH::KEYRING_BUSY == AUTH::CKeyringSession::GetResultOfFuture( second ) );

            // reads while the derivation runs: answered at once, as locked; a second instance reads its own way meanwhile
            UInt32 readsWhileBusy = 0;
            UInt64 longestRead = 0;
            bool sawUnlocked = false;
            AUTH::CSecretString secret;
            while ( !future.IsReady() && AUTH::GetMonotonicTimeInMs() - started < 60000 )
            {
                UInt64 readStart = AUTH::GetMonotonicTimeInMs();
                bool read = store.Load( "fab", "owner", secret );
                UInt64 took = AUTH::GetMonotonicTimeInMs() - readStart;
                if ( took > longestRead )
                    longestRead = took;
                if ( read )
                    sawUnlocked = true;
                else
                    ++readsWhileBusy;
                CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                MT::ThreadDelay( 1 );
            }
            UInt64 duration = AUTH::GetMonotonicTimeInMs() - started;
            REQUIRE_TRUE( future.IsReady() );
            // the operation ran: the future finished, it is no refusal, and holds the result
            ASSERT_TRUE( future.HasAFuture() && AUTH::KEYRING_OK == AUTH::CKeyringSession::GetResultOfFuture( future ) );
            ASSERT_TRUE( readsWhileBusy > 0 );
            ASSERT_TRUE( longestRead < duration || duration < 5 );
            ASSERT_TRUE( PumpUntilAtLeast( events.unlocked, 1, 2000 ) );
            ASSERT_TRUE( AUTH::KEYRING_OK == session.GetLastResult() );
            ASSERT_FALSE( session.IsBusy() );
            ASSERT_TRUE( store.Load( "fab", "owner", secret ) && SavedPassword == secret.Reveal() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Keyring test: the unlock took " << duration << " ms, " << readsWhileBusy << " reads answered meanwhile, the longest " << longestRead << " ms" );

            // a wrong code asynchronously
            session.LockNow();
            CORE::CFutureResult wrong = session.UnlockWithCodeAsync( AUTH::CSecretString( WrongCode ) );
            ASSERT_TRUE( PumpUntilReady( wrong, 60000 ) );
            ASSERT_TRUE( wrong.HasAFuture() && AUTH::KEYRING_WRONG_CODE == AUTH::CKeyringSession::GetResultOfFuture( wrong ) );
            ASSERT_TRUE( PumpUntilAtLeast( events.wrongCode, 1, 2000 ) );
            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == session.GetLastResult() && session.IsLocked() );
            (void) sawUnlocked;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Neither the code nor anything derived from it reaches the log or a file; the saved secret is not in the file in the clear" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_secrets" );
            CORE::CString text;
            {
                CLogCapture capture;
                TStore store( dir );
                SetUpTestStore( store );
                AUTH::CKeyringSession session( &store );
                AUTH::CKeyringSettings settings;
                settings.freeAttempts = 0;
                settings.retryDelayInMs = 1;
                settings.maxRetryDelayInMs = 1;
                session.SetSettings( settings );
                REQUIRE_TRUE( AUTH::KEYRING_OK == session.SetUpCode( AUTH::CSecretString( RightCode ) ) );
                REQUIRE_TRUE( store.Store( "fab", "owner", AUTH::CSecretString( SavedPassword ) ) );
                session.LockNow();
                ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == session.UnlockWithCode( AUTH::CSecretString( WrongCode ) ) );
                MT::ThreadDelay( 5 );
                ASSERT_TRUE( AUTH::KEYRING_OK == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
                ASSERT_TRUE( AUTH::KEYRING_OK == session.ChangeCode( AUTH::CSecretString( RightCode ), AUTH::CSecretString( NewCode ) ) );
                CORE::CFutureResult future = session.UnlockWithCodeAsync( AUTH::CSecretString( NewCode ) );
                PumpUntilReady( future, 60000 );
                PumpFor( 50 );
                AUTH::CSecretString secret;
                ASSERT_TRUE( store.Load( "fab", "owner", secret ) );
                text = capture.Text();
            }
            ASSERT_TRUE( text.Length() > 0 );
            CORE::CDynamicBuffer file = ReadFile( CORE::CombinePath( dir, TStore::KeyringFileName ) );
            REQUIRE_TRUE( file.GetDataSize() > HeaderSize );
            UInt8 master[ 32 ], sealKey[ 32 ], check[ 32 ];
            REQUIRE_TRUE( ReferenceKeyringKeys( NewCode, file, master, sealKey, check ) );
            const char* codes[ 3 ] = { RightCode, WrongCode, NewCode };
            CORE::CDynamicBuffer logBytes( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0, true );
            for ( UInt32 i=0; i<3; ++i )
            {
                ASSERT_FALSE( text.HasSubstr( codes[ i ] ) >= 0 );
                ASSERT_FALSE( ContainsBytes( file, codes[ i ], (UInt32) strlen( codes[ i ] ) ) );
            }
            ASSERT_FALSE( text.HasSubstr( SavedPassword ) >= 0 );
            ASSERT_FALSE( ContainsBytes( file, SavedPassword, (UInt32) strlen( SavedPassword ) ) );
            // the derived keys, raw and as hex in either case
            const UInt8* keys[ 2 ] = { master, sealKey };
            for ( UInt32 k=0; k<2; ++k )
            {
                CORE::CString hex = AUTH::CAuthCrypto::ToHex( keys[ k ], 32 );
                ASSERT_FALSE( text.HasSubstr( hex ) >= 0 );
                ASSERT_FALSE( text.HasSubstr( hex.Uppercase() ) >= 0 );
                ASSERT_FALSE( ContainsBytes( logBytes, keys[ k ], 32 ) );
                ASSERT_FALSE( ContainsBytes( file, keys[ k ], 32 ) );
                ASSERT_FALSE( ContainsBytes( file, hex.C_String(), hex.Length() ) );
            }
            // the directory has nothing but the keyring and the lock
            CORE::CString::StringVector names = ListDirectory( dir );
            for ( size_t i=0; i<names.size(); ++i )
                ASSERT_TRUE( TStore::KeyringFileName == names[ i ] || TStore::LockFileName == names[ i ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The default Argon2id parameters (64 MiB, 3 passes, 1 lane) seal and open a keyring" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "keyring_defaults" );
            TStore store( dir );
            store.SetKeyMode( TStore::KEYMODE_KEYRING_CODE );
            UInt32 memoryInKiB = 0, passes = 0, lanes = 0;
            store.GetKeyringParameters( memoryInKiB, passes, lanes );
            ASSERT_TRUE( 65536 == memoryInKiB && 3 == passes && 1 == lanes );
            UInt64 started = AUTH::GetMonotonicTimeInMs();
            REQUIRE_TRUE( AUTH::KEYRING_OK == store.SetUpKeyring( AUTH::CSecretString( RightCode ) ) );
            UInt64 setUpTook = AUTH::GetMonotonicTimeInMs() - started;
            store.LockKeyring();
            started = AUTH::GetMonotonicTimeInMs();
            ASSERT_TRUE( AUTH::KEYRING_OK == store.UnlockKeyring( AUTH::CSecretString( RightCode ) ) );
            UInt64 unlockTook = AUTH::GetMonotonicTimeInMs() - started;
            ASSERT_TRUE( store.GetSealedKeyringParameters( memoryInKiB, passes, lanes ) && 65536 == memoryInKiB && 3 == passes && 1 == lanes );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Keyring test: with the default parameters the set up took " << setUpTook << " ms, the unlock " << unlockTook << " ms" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The config of the store (keyMode, keyring parameters) and of the session round trip; an unknown keyMode is the key file mode; parameters are kept in range" )
        try
        {
            TStore store;
            CORE::CDataNode empty;
            store.SaveConfig( empty );
            ASSERT_TRUE( GUCEF_NULL == empty.GetAttribute( "keyMode" ) );
            ASSERT_TRUE( GUCEF_NULL == empty.GetAttribute( "keyringMemoryInKiB" ) );

            store.SetKeyMode( TStore::KEYMODE_KEYRING_CODE );
            store.SetKeyringParameters( 131072, 4, 2 );
            CORE::CDataNode config;
            ASSERT_TRUE( store.SaveConfig( config ) );
            ASSERT_TRUE( "keyring-code" == config.GetAttributeValueOrChildValueByName( "keyMode" ).AsString() );
            TStore loaded;
            ASSERT_TRUE( loaded.LoadConfig( config ) );
            ASSERT_TRUE( TStore::KEYMODE_KEYRING_CODE == loaded.GetKeyMode() );
            UInt32 memoryInKiB = 0, passes = 0, lanes = 0;
            loaded.GetKeyringParameters( memoryInKiB, passes, lanes );
            ASSERT_TRUE( 131072 == memoryInKiB && 4 == passes && 2 == lanes );

            config.SetAttribute( "keyMode", "fingerprint" );
            ASSERT_TRUE( loaded.LoadConfig( config ) );
            ASSERT_TRUE( TStore::KEYMODE_KEY_FILE == loaded.GetKeyMode() );
            TStore::EKeyMode mode = TStore::KEYMODE_KEY_FILE;
            ASSERT_TRUE( TStore::KeyModeFromString( "keyring-code", mode ) && TStore::KEYMODE_KEYRING_CODE == mode );
            ASSERT_FALSE( TStore::KeyModeFromString( "Keyring-Code", mode ) );
            ASSERT_TRUE( "key-file" == TStore::KeyModeToString( TStore::KEYMODE_KEY_FILE ) );

            loaded.SetKeyringParameters( 1, 0, 99 );
            loaded.GetKeyringParameters( memoryInKiB, passes, lanes );
            ASSERT_TRUE( TStore::MaxKeyringLanes == lanes && 1 == passes && 8 * lanes == memoryInKiB );
            loaded.SetKeyringParameters( 0xffffffff, 1000, 1 );
            loaded.GetKeyringParameters( memoryInKiB, passes, lanes );
            ASSERT_TRUE( TStore::MaxKeyringMemoryInKiB == memoryInKiB && TStore::MaxKeyringPasses == passes && 1 == lanes );

            AUTH::CKeyringSettings settings;
            ASSERT_TRUE( 900 == settings.idleLockInSeconds && 3 == settings.freeAttempts );
            settings.idleLockInSeconds = 120;
            settings.freeAttempts = 5;
            settings.retryDelayInMs = 2000;
            settings.maxRetryDelayInMs = 1000;
            CORE::CDataNode sessionConfig;
            ASSERT_TRUE( settings.SaveConfig( sessionConfig ) );
            AUTH::CKeyringSession session;
            ASSERT_TRUE( session.LoadConfig( sessionConfig ) );
            AUTH::CKeyringSettings read = session.GetSettings();
            ASSERT_TRUE( 120 == read.idleLockInSeconds && 5 == read.freeAttempts && 2000 == read.retryDelayInMs && 2000 == read.maxRetryDelayInMs );
            ASSERT_TRUE( session.IsLocked() && !session.IsSetUp() );
            ASSERT_TRUE( AUTH::KEYRING_FAILED == session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );

            ASSERT_TRUE( CORE::CString( "wrong code" ) == AUTH::KeyringResultToString( AUTH::KEYRING_WRONG_CODE ) );
            ASSERT_TRUE( CORE::CString( "idle" ) == AUTH::KeyringLockReasonToString( AUTH::KEYRINGLOCK_IDLE ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
