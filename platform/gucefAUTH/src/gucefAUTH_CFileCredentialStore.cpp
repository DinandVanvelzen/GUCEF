/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CFILELOCK_H
#include "gucefAUTH_CFileLock.h"
#define GUCEF_AUTH_CFILELOCK_H
#endif /* GUCEF_AUTH_CFILELOCK_H ? */

#include "gucefAUTH_CFileCredentialStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CFileCredentialStore::DataFileName = "credentials.bin";
const CORE::CString CFileCredentialStore::KeyFileName = "credentials.key";
const CORE::CString CFileCredentialStore::LockFileName = "credentials.lock";
const UInt32 CFileCredentialStore::DefaultLockTimeoutInMs = 10000;
const CORE::CString CFileCredentialStore::KeyringFileName = "keyring.bin";
const UInt32 CFileCredentialStore::DefaultKeyringMemoryInKiB = 65536;
const UInt32 CFileCredentialStore::DefaultKeyringPasses = 3;
const UInt32 CFileCredentialStore::DefaultKeyringLanes = 1;
const UInt32 CFileCredentialStore::MaxKeyringMemoryInKiB = 1048576;
const UInt32 CFileCredentialStore::MaxKeyringPasses = 32;
const UInt32 CFileCredentialStore::MaxKeyringLanes = 16;
const UInt32 CFileCredentialStore::MaxKeyringCodeLength = 1024;

static const char FileMagic[ 4 ] = { 'G', 'A', 'U', 'C' };
static const UInt8 FileVersion = 1;
static const UInt32 HeaderSize = 5;
static const UInt32 MaxSlotCount = 1024;
static const UInt32 MaxNameSize = 1024;
static const UInt32 MaxSecretSize = 65536;
static const char KeyInfo[] = "gucefAUTH credentials v1";
static const char KeyFileTrustOwnerOnlyText[] = "owner-only";
static const char KeyFileTrustMountedSecretText[] = "mounted-secret";
static const char KeyModeKeyFileText[] = "key-file";
static const char KeyModeKeyringCodeText[] = "keyring-code";

/*
 *  The keyring file (format version 2), all numbers little endian:
 *      0   4   magic "GAUC"
 *      4   1   version 2
 *      5   1   derivation: 1 = Argon2id
 *      6   4   memory in KiB
 *     10   4   passes
 *     14   4   lanes
 *     18  16   salt
 *     34  32   check value
 *     66  24   nonce
 *     90   ..  the slots sealed with XChaCha20-Poly1305 (as in version 1), the 66 bytes of the header as additional data
 */
static const UInt8 KeyringFileVersion = 2;
static const UInt8 KeyringKdfArgon2id = 1;
static const UInt32 KeyringSaltSize = 16;
static const UInt32 KeyringCheckSize = 32;
static const UInt32 KeyringHeaderSize = 4 + 1 + 1 + 4 + 4 + 4 + KeyringSaltSize + KeyringCheckSize;
static const char KeyringSealInfo[] = "gucefAUTH keyring v2 seal";
static const char KeyringCheckInfo[] = "gucefAUTH keyring v2 check";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
AppendUInt16( CORE::CDynamicBuffer& buffer, UInt16 value )
{GUCEF_TRACE;

    UInt8 bytes[ 2 ] = { (UInt8) value, (UInt8) ( value >> 8 ) };
    buffer.Append( bytes, 2 );
}

/*-------------------------------------------------------------------------*/

static void
AppendUInt32( CORE::CDynamicBuffer& buffer, UInt32 value )
{GUCEF_TRACE;

    UInt8 bytes[ 4 ] = { (UInt8) value, (UInt8) ( value >> 8 ), (UInt8) ( value >> 16 ), (UInt8) ( value >> 24 ) };
    buffer.Append( bytes, 4 );
}

/*-------------------------------------------------------------------------*/

static void
AppendUInt64( CORE::CDynamicBuffer& buffer, UInt64 value )
{GUCEF_TRACE;

    UInt8 bytes[ 8 ];
    for ( UInt32 i=0; i<8; ++i )
        bytes[ i ] = (UInt8) ( value >> ( 8 * i ) );
    buffer.Append( bytes, 8 );
}

/*-------------------------------------------------------------------------*/

static int
HexDigitValue( UInt8 c )
{GUCEF_TRACE;

    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

/**
 *  The 32 key bytes of the content of an external key file: the 32 bytes themselves, or 64 hexadecimal characters
 *  with an optional "\n" or "\r\n" at the end. False for anything else
 */
static bool
DecodeExternalKey( const CORE::CDynamicBuffer& content, UInt8* key )
{GUCEF_TRACE;

    const UInt8* bytes = static_cast< const UInt8* >( content.GetConstBufferPtr() );
    UInt32 size = content.GetDataSize();
    if ( CAuthCrypto::KeySize == size )
    {
        memcpy( key, bytes, CAuthCrypto::KeySize );
        return true;
    }
    if ( size == 2 * CAuthCrypto::KeySize + 1 && '\n' == bytes[ size - 1 ] )
        size -= 1;
    else
    if ( size == 2 * CAuthCrypto::KeySize + 2 && '\r' == bytes[ size - 2 ] && '\n' == bytes[ size - 1 ] )
        size -= 2;
    if ( 2 * CAuthCrypto::KeySize != size )
        return false;
    for ( UInt32 i=0; i<CAuthCrypto::KeySize; ++i )
    {
        const int high = HexDigitValue( bytes[ 2 * i ] );
        const int low = HexDigitValue( bytes[ 2 * i + 1 ] );
        if ( high < 0 || low < 0 )
        {
            CAuthCrypto::Wipe( key, CAuthCrypto::KeySize );
            return false;
        }
        key[ i ] = (UInt8) ( ( high << 4 ) | low );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static const char*
KeyStateText( CFileCredentialStore::EKeyState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case CFileCredentialStore::KEYSTATE_OK: return "usable";
        case CFileCredentialStore::KEYSTATE_MISSING: return "missing";
        case CFileCredentialStore::KEYSTATE_UNTRUSTED: return "refused by its trust check";
        default: return "not a key";
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
ReadUInt32At( const UInt8* bytes )
{GUCEF_TRACE;

    return (UInt32) bytes[ 0 ] | ( (UInt32) bytes[ 1 ] << 8 ) | ( (UInt32) bytes[ 2 ] << 16 ) | ( (UInt32) bytes[ 3 ] << 24 );
}

/*-------------------------------------------------------------------------*/

static void
WriteUInt32At( UInt8* bytes, UInt32 value )
{GUCEF_TRACE;

    bytes[ 0 ] = (UInt8) value;
    bytes[ 1 ] = (UInt8) ( value >> 8 );
    bytes[ 2 ] = (UInt8) ( value >> 16 );
    bytes[ 3 ] = (UInt8) ( value >> 24 );
}

/*-------------------------------------------------------------------------*/

/** The header of a keyring file: how its key is derived and the check value of that key; nothing in it is secret */
struct SKeyringHeader
{
    UInt32 memoryInKiB;
    UInt32 passes;
    UInt32 lanes;
    UInt8 salt[ KeyringSaltSize ];
    UInt8 check[ KeyringCheckSize ];
    UInt8 bytes[ KeyringHeaderSize ];       /**< the header as it is in the file, the additional data of the seal */
};

/*-------------------------------------------------------------------------*/

/** The parameters of a keyring are within what a keyring file may ask for */
static bool
AreKeyringParametersValid( UInt32 memoryInKiB, UInt32 passes, UInt32 lanes )
{GUCEF_TRACE;

    return lanes >= 1 && lanes <= CFileCredentialStore::MaxKeyringLanes &&
           passes >= 1 && passes <= CFileCredentialStore::MaxKeyringPasses &&
           memoryInKiB >= 8 * lanes && memoryInKiB <= CFileCredentialStore::MaxKeyringMemoryInKiB;
}

/*-------------------------------------------------------------------------*/

/** Reads the header of a keyring file: KEYRING_OK, KEYRING_NEWER_VERSION or KEYRING_TAMPERED (not a keyring file) */
static TKeyringResult
ParseKeyringHeader( const CORE::CDynamicBuffer& file, SKeyringHeader& header )
{GUCEF_TRACE;

    const UInt8* bytes = static_cast< const UInt8* >( file.GetConstBufferPtr() );
    if ( file.GetDataSize() < 5 || 0 != memcmp( bytes, FileMagic, sizeof( FileMagic ) ) )
        return KEYRING_TAMPERED;
    if ( bytes[ 4 ] > KeyringFileVersion )
        return KEYRING_NEWER_VERSION;
    if ( bytes[ 4 ] != KeyringFileVersion || file.GetDataSize() < KeyringHeaderSize + CAuthCrypto::NonceSize + CAuthCrypto::TagSize ||
         KeyringKdfArgon2id != bytes[ 5 ] )
        return KEYRING_TAMPERED;
    header.memoryInKiB = ReadUInt32At( bytes + 6 );
    header.passes = ReadUInt32At( bytes + 10 );
    header.lanes = ReadUInt32At( bytes + 14 );
    if ( !AreKeyringParametersValid( header.memoryInKiB, header.passes, header.lanes ) )
        return KEYRING_TAMPERED;
    memcpy( header.salt, bytes + 18, KeyringSaltSize );
    memcpy( header.check, bytes + 18 + KeyringSaltSize, KeyringCheckSize );
    memcpy( header.bytes, bytes, KeyringHeaderSize );
    return KEYRING_OK;
}

/*-------------------------------------------------------------------------*/

/** A new header with a random salt; the check value is filled in by DeriveKeyringKeys() */
static bool
MakeKeyringHeader( UInt32 memoryInKiB, UInt32 passes, UInt32 lanes, SKeyringHeader& header )
{GUCEF_TRACE;

    header.memoryInKiB = memoryInKiB;
    header.passes = passes;
    header.lanes = lanes;
    memset( header.check, 0, sizeof( header.check ) );
    if ( !CAuthCrypto::RandomBytes( header.salt, KeyringSaltSize ) )
        return false;
    memcpy( header.bytes, FileMagic, sizeof( FileMagic ) );
    header.bytes[ 4 ] = KeyringFileVersion;
    header.bytes[ 5 ] = KeyringKdfArgon2id;
    WriteUInt32At( header.bytes + 6, memoryInKiB );
    WriteUInt32At( header.bytes + 10, passes );
    WriteUInt32At( header.bytes + 14, lanes );
    memcpy( header.bytes + 18, header.salt, KeyringSaltSize );
    memset( header.bytes + 18 + KeyringSaltSize, 0, KeyringCheckSize );
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Argon2id of the code with the salt and parameters of the header gives 32 bytes, HKDF-SHA256 of those the sealing key and
 *  the check value. Nothing of it is kept here: every intermediate is wiped. False when the derivation is not available
 */
static bool
DeriveKeyringKeys( const CSecretString& code, const SKeyringHeader& header, UInt8* sealKey, UInt8* check )
{GUCEF_TRACE;

    UInt8 master[ CAuthCrypto::KeySize ];
    bool derived = CAuthCrypto::DeriveKeyFromPassphrase( code.Reveal().C_String(), code.Length(), header.salt, KeyringSaltSize,
                                                         header.memoryInKiB, header.passes, header.lanes, master, sizeof( master ) ) &&
                   CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, master, sizeof( master ), KeyringSealInfo, (UInt32) sizeof( KeyringSealInfo ) - 1, sealKey, CAuthCrypto::KeySize ) &&
                   CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, master, sizeof( master ), KeyringCheckInfo, (UInt32) sizeof( KeyringCheckInfo ) - 1, check, KeyringCheckSize );
    CAuthCrypto::Wipe( master, sizeof( master ) );
    if ( !derived )
    {
        CAuthCrypto::Wipe( sealKey, CAuthCrypto::KeySize );
        CAuthCrypto::Wipe( check, KeyringCheckSize );
    }
    return derived;
}

/*-------------------------------------------------------------------------*/

/** Reads what the Append functions wrote and refuses to read past the end */
class CBoundedReader
{
    public:

    CBoundedReader( const UInt8* data, UInt32 size ) : m_data( data ), m_size( size ), m_pos( 0 ) {}

    bool ReadUInt16( UInt16& value )
    {
        if ( m_size - m_pos < 2 )
            return false;
        value = (UInt16) ( m_data[ m_pos ] | ( m_data[ m_pos + 1 ] << 8 ) );
        m_pos += 2;
        return true;
    }

    bool ReadUInt32( UInt32& value )
    {
        if ( m_size - m_pos < 4 )
            return false;
        value = (UInt32) m_data[ m_pos ] | ( (UInt32) m_data[ m_pos + 1 ] << 8 ) | ( (UInt32) m_data[ m_pos + 2 ] << 16 ) | ( (UInt32) m_data[ m_pos + 3 ] << 24 );
        m_pos += 4;
        return true;
    }

    bool ReadUInt64( UInt64& value )
    {
        if ( m_size - m_pos < 8 )
            return false;
        value = 0;
        for ( UInt32 i=0; i<8; ++i )
            value |= (UInt64) m_data[ m_pos + i ] << ( 8 * i );
        m_pos += 8;
        return true;
    }

    bool ReadText( UInt32 length, CORE::CString& text )
    {
        if ( m_size - m_pos < length )
            return false;
        text = CORE::CString( reinterpret_cast< const char* >( m_data + m_pos ), length );
        m_pos += length;
        return true;
    }

    bool AtEnd( void ) const { return m_pos == m_size; }

    private:

    const UInt8* m_data;
    UInt32 m_size;
    UInt32 m_pos;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What was read of the files of the store while the lock was held */
struct CFileCredentialStore::SFileImage
{
    bool hasData;
    bool hasKey;
    CORE::CDynamicBuffer data;
    CORE::CDynamicBuffer key;               /**< the key material (of an external key: decoded to 32 bytes), wiped when the image goes */
    bool externalKey;                       /**< the key is the external key file */
    EKeyState keyState;
    CORE::CString keyPath;
    CORE::CString keyProblem;
    UInt8 digest[ CORE::CSha256::DigestSize ];

    SFileImage( void )
        : hasData( false )
        , hasKey( false )
        , data()
        , key()
        , externalKey( false )
        , keyState( KEYSTATE_MISSING )
        , keyPath()
        , keyProblem()
    {GUCEF_TRACE;

        memset( digest, 0, sizeof( digest ) );
    }

    ~SFileImage()
    {GUCEF_TRACE;

        if ( key.GetDataSize() > 0 )
            CAuthCrypto::Wipe( key.GetBufferPtr(), key.GetDataSize() );
    }

    /** The digest over the presence, the size and the bytes of both files */
    void ComputeDigest( void )
    {GUCEF_TRACE;

        CORE::CSha256 sha;
        AddToDigest( sha, hasData, data );
        AddToDigest( sha, hasKey, key );
        // a key that becomes trusted or usable without a change of its bytes is a change too
        const UInt8 keyFacts[ 2 ] = { (UInt8) ( externalKey ? 1 : 0 ), (UInt8) keyState };
        sha.Update( keyFacts, sizeof( keyFacts ) );
        sha.Final( digest );
    }

    private:

    static void AddToDigest( CORE::CSha256& sha, bool present, const CORE::CDynamicBuffer& content )
    {GUCEF_TRACE;

        UInt32 size = content.GetDataSize();
        UInt8 prefix[ 5 ] = { (UInt8) ( present ? 1 : 0 ), (UInt8) size, (UInt8) ( size >> 8 ), (UInt8) ( size >> 16 ), (UInt8) ( size >> 24 ) };
        sha.Update( prefix, sizeof( prefix ) );
        if ( size > 0 )
            sha.Update( content.GetConstBufferPtr(), size );
    }

    SFileImage( const SFileImage& src );
    SFileImage& operator=( const SFileImage& src );
};

/*-------------------------------------------------------------------------*/

CFileCredentialStore::CFileCredentialStore( void )
    : ICredentialStore()
    , CORE::CIConfigurable()
    , m_lock()
    , m_appName( "GUCEF" )
    , m_directory()
    , m_lockTimeoutInMs( DefaultLockTimeoutInMs )
    , m_keyFile()
    , m_keyFileTrust( KEYFILETRUST_OWNER_ONLY )
    , m_assumeReadOnlyKeyMountForTesting( false )
    , m_slots()
    , m_loaded( false )
    , m_tamperDetected( false )
    , m_newerVersion( false )
    , m_keyRefused( false )
    , m_keyState( KEYSTATE_MISSING )
    , m_keyProblem()
    , m_keyStateLogged( false )
    , m_loggedKeyState( KEYSTATE_OK )
    , m_strayKeyWarned( false )
    , m_keyMode( KEYMODE_KEY_FILE )
    , m_keyringMemoryInKiB( DefaultKeyringMemoryInKiB )
    , m_keyringPasses( DefaultKeyringPasses )
    , m_keyringLanes( DefaultKeyringLanes )
    , m_keyringKey( CAuthCrypto::KeySize )
    , m_keyringHeader()
    , m_keyringUnlocked( false )
    , m_keyringLastUseMs( 0 )
    , m_keyringLockReason( KEYRINGLOCK_REQUESTED )
{GUCEF_TRACE;

    memset( m_fileDigest, 0, sizeof( m_fileDigest ) );
    memset( m_keyringCheck, 0, sizeof( m_keyringCheck ) );
}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::CFileCredentialStore( const CORE::CString& directory )
    : ICredentialStore()
    , CORE::CIConfigurable()
    , m_lock()
    , m_appName( "GUCEF" )
    , m_directory( directory )
    , m_lockTimeoutInMs( DefaultLockTimeoutInMs )
    , m_keyFile()
    , m_keyFileTrust( KEYFILETRUST_OWNER_ONLY )
    , m_assumeReadOnlyKeyMountForTesting( false )
    , m_slots()
    , m_loaded( false )
    , m_tamperDetected( false )
    , m_newerVersion( false )
    , m_keyRefused( false )
    , m_keyState( KEYSTATE_MISSING )
    , m_keyProblem()
    , m_keyStateLogged( false )
    , m_loggedKeyState( KEYSTATE_OK )
    , m_strayKeyWarned( false )
    , m_keyMode( KEYMODE_KEY_FILE )
    , m_keyringMemoryInKiB( DefaultKeyringMemoryInKiB )
    , m_keyringPasses( DefaultKeyringPasses )
    , m_keyringLanes( DefaultKeyringLanes )
    , m_keyringKey( CAuthCrypto::KeySize )
    , m_keyringHeader()
    , m_keyringUnlocked( false )
    , m_keyringLastUseMs( 0 )
    , m_keyringLockReason( KEYRINGLOCK_REQUESTED )
{GUCEF_TRACE;

    memset( m_fileDigest, 0, sizeof( m_fileDigest ) );
    memset( m_keyringCheck, 0, sizeof( m_keyringCheck ) );
}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::~CFileCredentialStore()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    LockKeyringWhileLocked( KEYRINGLOCK_APPLICATION_END );
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::ForgetView( void ) const
{GUCEF_TRACE;

    // The lock is held by the caller
    m_slots.clear();
    m_loaded = false;
    m_tamperDetected = false;
    m_newerVersion = false;
    m_keyRefused = false;
    m_keyState = KEYSTATE_MISSING;
    m_keyProblem.Clear();
    memset( m_fileDigest, 0, sizeof( m_fileDigest ) );
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::SetDirectory( const CORE::CString& directory )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_directory = directory;
    ForgetView();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFileCredentialStore::GetDirectory( void ) const
{GUCEF_TRACE;

    if ( !m_directory.IsNULLOrEmpty() )
        return CORE::RelativePath( m_directory );
    CORE::CString path = CORE::RelativePath( "$APPDATADIR$" );
    path = CORE::CombinePath( path, m_appName );
    return CORE::CombinePath( path, "Auth" );
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::SetLockTimeoutInMs( UInt32 lockTimeoutInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_lockTimeoutInMs = lockTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CFileCredentialStore::GetLockTimeoutInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lockTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::IsTamperDetected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    RefreshView();
    return m_tamperDetected;
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::SetKeyFile( const CORE::CString& keyFile )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_keyFile = keyFile;
    m_strayKeyWarned = false;
    ForgetView();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFileCredentialStore::GetKeyFile( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_keyFile.IsNULLOrEmpty() )
        return CORE::CombinePath( GetDirectory(), KeyFileName );
    return CORE::RelativePath( m_keyFile );
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::HasExternalKeyFileWhileLocked( void ) const
{GUCEF_TRACE;

    return !m_keyFile.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::HasExternalKeyFile( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return HasExternalKeyFileWhileLocked();
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::SetKeyFileTrust( EKeyFileTrust trust )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_keyFileTrust = trust;
    ForgetView();
}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::EKeyFileTrust
CFileCredentialStore::GetKeyFileTrust( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_keyFileTrust;
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::SetAssumeReadOnlyKeyMountForTesting( bool assumeReadOnly )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_assumeReadOnlyKeyMountForTesting = assumeReadOnly;
    ForgetView();
}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::EKeyState
CFileCredentialStore::GetKeyState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    RefreshView();
    return m_keyState;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFileCredentialStore::GetKeyProblem( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_keyProblem;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFileCredentialStore::KeyFileTrustToString( EKeyFileTrust trust )
{GUCEF_TRACE;

    static const CORE::CString ownerOnly( KeyFileTrustOwnerOnlyText );
    static const CORE::CString mountedSecret( KeyFileTrustMountedSecretText );
    return KEYFILETRUST_MOUNTED_SECRET == trust ? mountedSecret : ownerOnly;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::KeyFileTrustFromString( const CORE::CString& text, EKeyFileTrust& trust )
{GUCEF_TRACE;

    if ( KeyFileTrustOwnerOnlyText == text )
    {
        trust = KEYFILETRUST_OWNER_ONLY;
        return true;
    }
    if ( KeyFileTrustMountedSecretText == text )
    {
        trust = KEYFILETRUST_MOUNTED_SECRET;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::MakeKeyFile( const CORE::CString& path, CORE::CString& problem )
{GUCEF_TRACE;

    problem.Clear();
    UInt8 keyMaterial[ CAuthCrypto::KeySize ];
    if ( !CAuthCrypto::RandomBytes( keyMaterial, sizeof( keyMaterial ) ) )
    {
        problem = "no random bytes could be had";
        return false;
    }
    bool made = CORE::CreateOwnerOnlyFile( CORE::RelativePath( path ), keyMaterial, sizeof( keyMaterial ), problem );
    CAuthCrypto::Wipe( keyMaterial, sizeof( keyMaterial ) );
    return made;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::IsAvailable( void )
{GUCEF_TRACE;

    return CAuthCrypto::IsAeadAvailable();
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::PrepareDirectory( void )
{GUCEF_TRACE;

    CORE::CString dir = GetDirectory();
    if ( !CORE::DirExists( dir ) && !CORE::CreateDirs( dir ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot create the directory " << dir );
        return false;
    }
    if ( !CORE::SetOwnerOnlyAccess( dir ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot restrict the directory " << dir << " to the user" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::ReadFiles( const CORE::CString& directory, SFileImage& image ) const
{GUCEF_TRACE;

    // The lock of the store file is held by the caller. A file that is there but cannot be read is there with no
    // content: the data file is then not a credential file, the key file is not a key
    CORE::CString dataPath = CORE::CombinePath( directory, GetDataFileNameWhileLocked() );
    image.hasData = CORE::FileExists( dataPath );
    if ( image.hasData && !image.data.LoadContentFromFile( dataPath ) )
        image.data.Clear();
    if ( IsKeyringModeWhileLocked() )
    {
        // the key is the one in memory: there is no key file to read
        image.externalKey = false;
        image.hasKey = false;
        image.keyState = KEYSTATE_OK;
    }
    else
    {
        ReadKey( directory, image );
    }
    image.ComputeDigest();
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::ReadKey( const CORE::CString& directory, SFileImage& image ) const
{GUCEF_TRACE;

    // The lock is held by the caller
    const CORE::CString localKeyPath = CORE::CombinePath( directory, KeyFileName );
    if ( !HasExternalKeyFileWhileLocked() )
    {
        image.externalKey = false;
        image.keyPath = localKeyPath;
        image.hasKey = CORE::FileExists( localKeyPath );
        if ( image.hasKey && !image.key.LoadContentFromFile( localKeyPath ) )
            image.key.Clear();
        image.keyState = !image.hasKey ? KEYSTATE_MISSING : ( CAuthCrypto::KeySize == image.key.GetDataSize() ? KEYSTATE_OK : KEYSTATE_INVALID );
        return;
    }

    // The external key: provisioned by someone else, read only through its trust check, never the local file
    image.externalKey = true;
    image.keyPath = CORE::RelativePath( m_keyFile );
    if ( !m_strayKeyWarned && CORE::FileExists( localKeyPath ) )
    {
        m_strayKeyWarned = true;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << localKeyPath << " is not used, the external key " <<
            image.keyPath << " is: remove it" );
    }

    CORE::CDynamicBuffer content;
    CORE::CString problem;
    bool isMissing = false;
    bool read = false;
    if ( KEYFILETRUST_MOUNTED_SECRET == m_keyFileTrust )
        read = CORE::ReadMountedSecretFile( image.keyPath, content, problem, &isMissing, m_assumeReadOnlyKeyMountForTesting );
    else
        read = CORE::ReadOwnerOnlyFile( image.keyPath, content, problem, &isMissing );

    image.hasKey = false;
    image.key.Clear();
    if ( !read )
    {
        image.keyState = isMissing ? KEYSTATE_MISSING : KEYSTATE_UNTRUSTED;
        image.keyProblem = problem;
    }
    else
    {
        UInt8 key[ CAuthCrypto::KeySize ];
        if ( DecodeExternalKey( content, key ) )
        {
            image.key.Append( key, sizeof( key ) );
            image.hasKey = true;
            image.keyState = KEYSTATE_OK;
        }
        else
        {
            image.keyState = KEYSTATE_INVALID;
            image.keyProblem = "the file holds neither 32 bytes nor 64 hexadecimal characters";
        }
        CAuthCrypto::Wipe( key, sizeof( key ) );
    }
    if ( content.GetDataSize() > 0 )
        CAuthCrypto::Wipe( content.GetBufferPtr(), content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::NoteKeyState( const SFileImage& image ) const
{GUCEF_TRACE;

    // The lock is held by the caller. An external key's state is logged once per change, with the path and the
    // reason of the check, never the content
    m_keyState = image.keyState;
    m_keyProblem = image.keyProblem;
    if ( !image.externalKey || ( m_keyStateLogged && m_loggedKeyState == image.keyState ) )
        return;
    m_keyStateLogged = true;
    m_loggedKeyState = image.keyState;
    if ( KEYSTATE_OK == image.keyState )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The external key " << image.keyPath << " is usable" );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The external key " << image.keyPath << " is " << KeyStateText( image.keyState ) <<
            ( image.keyProblem.IsNULLOrEmpty() ? CORE::CString() : CORE::CString( ": " ) + image.keyProblem ) << "; the store reads nothing and changes nothing until it is" );
    }
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::UpdateView( const SFileImage& image ) const
{GUCEF_TRACE;

    // The lock is held by the caller. The files are what they were at the last read or write: the view stands
    if ( m_loaded && 0 == memcmp( m_fileDigest, image.digest, sizeof( m_fileDigest ) ) )
        return;
    ParseImage( image );
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::ParseImage( const SFileImage& image ) const
{GUCEF_TRACE;

    // The lock is held by the caller
    m_slots.clear();
    m_tamperDetected = false;
    m_newerVersion = false;
    m_keyRefused = false;
    m_loaded = true;
    memcpy( m_fileDigest, image.digest, sizeof( m_fileDigest ) );
    NoteKeyState( image );

    // An external key that is missing, refused or not a key is a fault of the deployment, not tampering: nothing is
    // read and nothing is written, with or without a data file
    if ( image.externalKey && KEYSTATE_OK != image.keyState )
    {
        m_keyRefused = true;
        return;
    }
    if ( IsKeyringModeWhileLocked() )
    {
        ParseKeyringImage( image );
        return;
    }

    CORE::CString dataPath = CORE::CombinePath( GetDirectory(), DataFileName );
    if ( !image.hasData )
        return;

    const CORE::CDynamicBuffer& file = image.data;
    if ( file.GetDataSize() < HeaderSize + CAuthCrypto::NonceSize + CAuthCrypto::TagSize ||
         0 != memcmp( file.GetConstBufferPtr(), FileMagic, sizeof( FileMagic ) ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << dataPath << " is not a credential file" );
        m_tamperDetected = true;
        return;
    }
    const UInt8* bytes = static_cast< const UInt8* >( file.GetConstBufferPtr() );
    if ( bytes[ 4 ] > FileVersion )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << dataPath << " is of a newer version, it is left alone" );
        m_newerVersion = true;
        return;
    }
    if ( !IsAvailable() )
    {
        // Nothing can be read without the codec: that is not a sign of tampering, the files are looked at again next time
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: There is no \"" << CAuthCrypto::AeadCipher << "\" codec, " << dataPath << " cannot be read" );
        m_loaded = false;
        return;
    }

    UInt8 key[ CAuthCrypto::KeySize ];
    if ( !image.hasKey || CAuthCrypto::KeySize != image.key.GetDataSize() ||
         !CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, image.key.GetConstBufferPtr(), CAuthCrypto::KeySize, KeyInfo, (UInt32) sizeof( KeyInfo ) - 1, key, CAuthCrypto::KeySize ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The key of " << dataPath << " is gone or cannot be used, its content cannot be read" );
        CAuthCrypto::Wipe( key, sizeof( key ) );
        m_tamperDetected = true;
        return;
    }
    CORE::CDynamicBuffer plain;
    bool opened = CAuthCrypto::AeadOpen( key, bytes + HeaderSize, bytes, HeaderSize, bytes + HeaderSize + CAuthCrypto::NonceSize,
                                         file.GetDataSize() - HeaderSize - CAuthCrypto::NonceSize, plain );
    CAuthCrypto::Wipe( key, sizeof( key ) );
    if ( !opened )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << dataPath << " failed authentication, it was changed or is not made with this key" );
        if ( plain.GetDataSize() > 0 )
            CAuthCrypto::Wipe( plain.GetBufferPtr(), plain.GetDataSize() );
        m_tamperDetected = true;
        return;
    }

    TSlotVector slots;
    bool valid = DeserializeSlots( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize(), slots );
    CAuthCrypto::Wipe( plain.GetBufferPtr(), plain.GetDataSize() );
    if ( !valid )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << dataPath << " has content that does not make sense" );
        m_tamperDetected = true;
        return;
    }
    m_slots = slots;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::RefreshView( void ) const
{GUCEF_TRACE;

    // The lock is held by the caller. Nothing is made for a read: without the files there is nothing to lock
    CORE::CString dir = GetDirectory();
    if ( IsKeyringModeWhileLocked() )
    {
        if ( !CORE::FileExists( CORE::CombinePath( dir, KeyringFileName ) ) )
        {
            SFileImage none;
            none.keyState = KEYSTATE_OK;
            none.ComputeDigest();
            UpdateView( none );
            return true;
        }
    }
    else
    if ( HasExternalKeyFileWhileLocked() && !CORE::FileExists( CORE::CombinePath( dir, DataFileName ) ) )
    {
        // no data: what is to know is the state of the external key, which is read without touching the directory
        SFileImage keyOnly;
        ReadFiles( dir, keyOnly );
        UpdateView( keyOnly );
        return true;
    }
    else
    if ( !CORE::FileExists( CORE::CombinePath( dir, DataFileName ) ) && !CORE::FileExists( CORE::CombinePath( dir, KeyFileName ) ) )
    {
        SFileImage none;
        none.ComputeDigest();
        UpdateView( none );
        return true;
    }

    CFileLock fileLock;
    CFileLock::TLockOutcome outcome = fileLock.Acquire( CORE::CombinePath( dir, LockFileName ), false, m_lockTimeoutInMs );
    if ( CFileLock::LOCKOUTCOME_TIMED_OUT == outcome )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The store in " << dir << " stayed locked by another instance for " << m_lockTimeoutInMs << " ms, it is not read" );
        return false;
    }
    // LOCKOUTCOME_UNAVAILABLE: a directory in which the lock file cannot be made is not written by anyone, it is read as it is

    SFileImage image;
    ReadFiles( dir, image );
    UpdateView( image );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::WriteSlots( const CORE::CString& directory, const TSlotVector& slots, SFileImage& image )
{GUCEF_TRACE;

    // The exclusive lock of the store file is held by the caller, 'image' is what was read under it
    CORE::CString keyPath = CORE::CombinePath( directory, KeyFileName );
    if ( image.externalKey && ( !image.hasKey || CAuthCrypto::KeySize != image.key.GetDataSize() ) )
    {
        // never made here: an external key is provisioned by someone else, nothing is sealed with a made-up key
        return false;
    }
    if ( !image.hasKey || CAuthCrypto::KeySize != image.key.GetDataSize() )
    {
        // Only without a data file: a data file without a usable key is tamper detected and never written
        UInt8 keyMaterial[ CAuthCrypto::KeySize ];
        if ( !CAuthCrypto::RandomBytes( keyMaterial, sizeof( keyMaterial ) ) )
            return false;
        bool written = CORE::WriteFileAtomically( keyPath, keyMaterial, sizeof( keyMaterial ) );
        if ( written )
        {
            if ( image.key.GetDataSize() > 0 )
                CAuthCrypto::Wipe( image.key.GetBufferPtr(), image.key.GetDataSize() );
            image.key.Clear();
            image.key.Append( keyMaterial, sizeof( keyMaterial ) );
            image.hasKey = true;
        }
        CAuthCrypto::Wipe( keyMaterial, sizeof( keyMaterial ) );
        if ( !written )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot write the key file " << keyPath );
            return false;
        }
        if ( !CORE::SetOwnerOnlyAccess( keyPath ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot restrict the key file " << keyPath << " to the user" );
    }

    UInt8 key[ CAuthCrypto::KeySize ];
    if ( !CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, image.key.GetConstBufferPtr(), CAuthCrypto::KeySize, KeyInfo, (UInt32) sizeof( KeyInfo ) - 1, key, CAuthCrypto::KeySize ) )
    {
        CAuthCrypto::Wipe( key, sizeof( key ) );
        return false;
    }

    CORE::CDynamicBuffer plain;
    SerializeSlots( slots, plain );

    UInt8 header[ HeaderSize ];
    memcpy( header, FileMagic, sizeof( FileMagic ) );
    header[ 4 ] = FileVersion;
    UInt8 nonce[ CAuthCrypto::NonceSize ];
    CORE::CDynamicBuffer file;
    bool sealed = CAuthCrypto::RandomBytes( nonce, sizeof( nonce ) );
    if ( sealed )
    {
        file.Append( header, HeaderSize );
        file.Append( nonce, sizeof( nonce ) );
        sealed = CAuthCrypto::AeadSeal( key, nonce, header, HeaderSize, plain.GetConstBufferPtr(), plain.GetDataSize(), file );
    }
    CAuthCrypto::Wipe( key, sizeof( key ) );
    CAuthCrypto::Wipe( plain.GetBufferPtr(), plain.GetDataSize() );
    if ( !sealed )
        return false;

    CORE::CString dataPath = CORE::CombinePath( directory, DataFileName );
    if ( !CORE::WriteFileAtomically( dataPath, file ) )
        return false;
    if ( !CORE::SetOwnerOnlyAccess( dataPath ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot restrict " << dataPath << " to the user" );

    // the files as they are now
    image.hasData = true;
    image.data = file;
    image.ComputeDigest();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::Change( TChangeKind kind                     ,
                              const CORE::CString& providerName    ,
                              const CORE::CString& accountKey      ,
                              const CSecretString* secret          )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString dir = GetDirectory();
    if ( CHANGE_STORE != kind && !CORE::DirExists( dir ) )
    {
        // nothing to remove, and nothing is made for it
        SFileImage none;
        none.ComputeDigest();
        UpdateView( none );
        return true;
    }
    if ( !IsAvailable() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: There is no \"" << CAuthCrypto::AeadCipher << "\" codec, nothing is changed in the store in " << dir );
        return false;
    }
    if ( IsKeyringModeWhileLocked() && !m_keyringUnlocked )
    {
        // nothing is made for a keyring that is locked, not even the directory
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The keyring in " << dir << " is locked, nothing is changed" );
        return false;
    }
    if ( !IsKeyringModeWhileLocked() && HasExternalKeyFileWhileLocked() )
    {
        // without a usable external key nothing is made, not even the directory or the lock file
        SFileImage keyProbe;
        ReadKey( dir, keyProbe );
        if ( KEYSTATE_OK != keyProbe.keyState )
        {
            NoteKeyState( keyProbe );
            m_keyRefused = true;
            m_loaded = false;
            return false;
        }
    }
    if ( !PrepareDirectory() )
        return false;

    CFileLock fileLock;
    CFileLock::TLockOutcome outcome = fileLock.Acquire( CORE::CombinePath( dir, LockFileName ), true, m_lockTimeoutInMs );
    if ( CFileLock::LOCKOUTCOME_TAKEN != outcome && CFileLock::IsSupported() )
    {
        if ( CFileLock::LOCKOUTCOME_TIMED_OUT == outcome )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The store in " << dir << " stayed locked by another instance for " << m_lockTimeoutInMs << " ms, nothing is changed" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot lock the store in " << dir << " (" << LockFileName << "), nothing is changed" );
        }
        return false;
    }

    // the change applies to what is in the file now, not to what this instance read before
    SFileImage image;
    ReadFiles( dir, image );
    UpdateView( image );
    if ( m_keyRefused )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The external key of the store in " << dir << " is " << KeyStateText( m_keyState ) << ", nothing is changed" );
        return false;
    }
    if ( m_tamperDetected )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The store in " << dir << " failed its integrity check, nothing is written to it until it is wiped" );
        return false;
    }
    if ( m_newerVersion || !m_loaded )
        return false;
    if ( IsKeyringModeWhileLocked() && ( !m_keyringUnlocked || !CORE::FileExists( CORE::CombinePath( dir, KeyringFileName ) ) ) )
    {
        // locked meanwhile (another instance sealed it with another code, or it was forgotten), or never set up
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The keyring in " << dir << " is locked or not set up, nothing is changed" );
        return false;
    }

    TSlotVector slots( m_slots );
    bool changed = false;
    switch ( kind )
    {
        case CHANGE_STORE:
        {
            // The vector is in order of storing: a slot that is stored again moves to the back
            for ( size_t i=0; i<slots.size(); ++i )
            {
                if ( slots[ i ].providerName == providerName && slots[ i ].accountKey == accountKey )
                {
                    slots.erase( slots.begin() + i );
                    break;
                }
            }
            if ( slots.size() >= MaxSlotCount || GUCEF_NULL == secret )
                return false;
            SSlot added;
            added.providerName = providerName;
            added.accountKey = accountKey;
            added.secret = *secret;
            added.storedAtMs = GetEpochTimeInMs();
            slots.push_back( added );
            changed = true;
            break;
        }
        case CHANGE_REMOVE:
        {
            for ( size_t i=0; i<slots.size(); ++i )
            {
                if ( slots[ i ].providerName == providerName && slots[ i ].accountKey == accountKey )
                {
                    slots.erase( slots.begin() + i );
                    changed = true;
                    break;
                }
            }
            break;
        }
        case CHANGE_REMOVE_PROVIDER:
        {
            TSlotVector kept;
            for ( size_t i=0; i<slots.size(); ++i )
                if ( slots[ i ].providerName != providerName )
                    kept.push_back( slots[ i ] );
            changed = kept.size() != slots.size();
            slots = kept;
            break;
        }
    }
    if ( IsKeyringModeWhileLocked() )
        m_keyringLastUseMs = GetMonotonicTimeInMs();
    if ( !changed )
        return true;

    if ( IsKeyringModeWhileLocked() )
    {
        if ( !WriteKeyringSlots( dir, slots, static_cast< const UInt8* >( m_keyringHeader.GetConstBufferPtr() ), m_keyringHeader.GetDataSize(),
                                 m_keyringKey.GetData(), image ) )
            return false;
    }
    else
    if ( !WriteSlots( dir, slots, image ) )
        return false;
    m_slots = slots;
    m_loaded = true;
    m_tamperDetected = false;
    memcpy( m_fileDigest, image.digest, sizeof( m_fileDigest ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CFileCredentialStore::SSlot*
CFileCredentialStore::FindSlot( const CORE::CString& providerName, const CORE::CString& accountKey ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_slots.size(); ++i )
        if ( m_slots[ i ].providerName == providerName && m_slots[ i ].accountKey == accountKey )
            return &m_slots[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::Store( const CORE::CString& providerName ,
                             const CORE::CString& accountKey   ,
                             const CSecretString& secret       )
{GUCEF_TRACE;

    if ( providerName.IsNULLOrEmpty() || accountKey.IsNULLOrEmpty() || GetByteLength( providerName ) > MaxNameSize ||
         GetByteLength( accountKey ) > MaxNameSize || secret.Length() > MaxSecretSize )
        return false;
    return Change( CHANGE_STORE, providerName, accountKey, &secret );
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::Load( const CORE::CString& providerName ,
                            const CORE::CString& accountKey   ,
                            CSecretString& secret             )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !RefreshView() )
        return false;
    if ( IsKeyringModeWhileLocked() )
    {
        if ( !m_keyringUnlocked )
            return false;
        m_keyringLastUseMs = GetMonotonicTimeInMs();
    }
    const SSlot* slot = FindSlot( providerName, accountKey );
    if ( GUCEF_NULL == slot )
        return false;
    secret = slot->secret;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::Remove( const CORE::CString& providerName ,
                              const CORE::CString& accountKey   )
{GUCEF_TRACE;

    return Change( CHANGE_REMOVE, providerName, accountKey, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::ListAccounts( const CORE::CString& providerName        ,
                                    CORE::CString::StringVector& accountKeys )
{GUCEF_TRACE;

    accountKeys.clear();
    MT::CScopeMutex lock( m_lock );
    if ( !RefreshView() )
        return false;
    if ( IsKeyringModeWhileLocked() )
    {
        // a locked keyring cannot be read: that is not the same as having no accounts
        if ( !m_keyringUnlocked )
            return false;
        m_keyringLastUseMs = GetMonotonicTimeInMs();
    }
    // the slot stored last first
    for ( size_t i=m_slots.size(); i>0; --i )
        if ( m_slots[ i-1 ].providerName == providerName )
            accountKeys.push_back( m_slots[ i-1 ].accountKey );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::RemoveProvider( const CORE::CString& providerName )
{GUCEF_TRACE;

    return Change( CHANGE_REMOVE_PROVIDER, providerName, CORE::CString::Empty, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::Wipe( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    ForgetView();
    CORE::CString dir = GetDirectory();
    if ( !CORE::DirExists( dir ) )
    {
        if ( IsKeyringModeWhileLocked() )
            LockKeyringWhileLocked( KEYRINGLOCK_FORGOTTEN );
        return true;
    }

    // Under the exclusive lock, which another instance may hold open: the lock file itself stays
    CFileLock fileLock;
    if ( CFileLock::LOCKOUTCOME_TIMED_OUT == fileLock.Acquire( CORE::CombinePath( dir, LockFileName ), true, m_lockTimeoutInMs ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The store in " << dir << " stayed locked by another instance for " << m_lockTimeoutInMs << " ms, it is not wiped" );
        return false;
    }

    bool wiped = true;
    if ( IsKeyringModeWhileLocked() )
    {
        // forgetting the code: the saved credentials go with the keyring; the files of the key file mode are not of this store
        LockKeyringWhileLocked( KEYRINGLOCK_FORGOTTEN );
        CORE::CString keyringPath = CORE::CombinePath( dir, KeyringFileName );
        if ( CORE::FileExists( keyringPath ) )
            wiped = CORE::DeleteFile( keyringPath );
        if ( wiped )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Deleted the keyring in " << dir << " and the credentials saved in it" );
        return wiped;
    }
    CORE::CString dataPath = CORE::CombinePath( dir, DataFileName );
    CORE::CString keyPath = CORE::CombinePath( dir, KeyFileName );
    if ( CORE::FileExists( dataPath ) )
        wiped = CORE::DeleteFile( dataPath ) && wiped;
    if ( HasExternalKeyFileWhileLocked() )
    {
        // the external key belongs to whoever provisioned it, as does a local key file that it replaced
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Wiped the data of the store in " << dir << ", the external key " << CORE::RelativePath( m_keyFile ) << " stays" );
        return wiped;
    }
    if ( CORE::FileExists( keyPath ) )
        wiped = CORE::DeleteFile( keyPath ) && wiped;
    return wiped;
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::SerializeSlots( const TSlotVector& slots, CORE::CDynamicBuffer& plain )
{GUCEF_TRACE;

    AppendUInt32( plain, (UInt32) slots.size() );
    for ( size_t i=0; i<slots.size(); ++i )
    {
        const SSlot& slot = slots[ i ];
        AppendUInt16( plain, (UInt16) GetByteLength( slot.providerName ) );
        plain.Append( slot.providerName.C_String(), GetByteLength( slot.providerName ) );
        AppendUInt16( plain, (UInt16) GetByteLength( slot.accountKey ) );
        plain.Append( slot.accountKey.C_String(), GetByteLength( slot.accountKey ) );
        AppendUInt64( plain, slot.storedAtMs );
        AppendUInt32( plain, slot.secret.Length() );
        plain.Append( slot.secret.Reveal().C_String(), slot.secret.Length() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::DeserializeSlots( const UInt8* data, UInt32 size, TSlotVector& slots )
{GUCEF_TRACE;

    CBoundedReader reader( data, size );
    UInt32 count = 0;
    bool valid = reader.ReadUInt32( count ) && count <= MaxSlotCount;
    for ( UInt32 i=0; valid && i<count; ++i )
    {
        SSlot slot;
        UInt16 providerLength = 0;
        UInt16 accountLength = 0;
        UInt32 secretLength = 0;
        CORE::CString secretText;
        valid = reader.ReadUInt16( providerLength ) && providerLength <= MaxNameSize && reader.ReadText( providerLength, slot.providerName ) &&
                reader.ReadUInt16( accountLength ) && accountLength <= MaxNameSize && reader.ReadText( accountLength, slot.accountKey ) &&
                reader.ReadUInt64( slot.storedAtMs ) &&
                reader.ReadUInt32( secretLength ) && secretLength <= MaxSecretSize && reader.ReadText( secretLength, secretText );
        if ( valid )
        {
            slot.secret.Set( secretText );
            slots.push_back( slot );
        }
        if ( GetByteLength( secretText ) > 0 )
            CAuthCrypto::Wipe( secretText.C_String(), GetByteLength( secretText ) );
    }
    if ( !valid || !reader.AtEnd() )
    {
        slots.clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::IsKeyringModeWhileLocked( void ) const
{GUCEF_TRACE;

    return KEYMODE_KEYRING_CODE == m_keyMode;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFileCredentialStore::GetDataFileNameWhileLocked( void ) const
{GUCEF_TRACE;

    return IsKeyringModeWhileLocked() ? KeyringFileName : DataFileName;
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::SetKeyMode( EKeyMode keyMode )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( keyMode != m_keyMode )
        LockKeyringWhileLocked( KEYRINGLOCK_REQUESTED );
    m_keyMode = keyMode;
    ForgetView();
}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::EKeyMode
CFileCredentialStore::GetKeyMode( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_keyMode;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFileCredentialStore::KeyModeToString( EKeyMode keyMode )
{GUCEF_TRACE;

    static const CORE::CString keyFile( KeyModeKeyFileText );
    static const CORE::CString keyringCode( KeyModeKeyringCodeText );
    return KEYMODE_KEYRING_CODE == keyMode ? keyringCode : keyFile;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::KeyModeFromString( const CORE::CString& text, EKeyMode& keyMode )
{GUCEF_TRACE;

    if ( KeyModeKeyFileText == text )
    {
        keyMode = KEYMODE_KEY_FILE;
        return true;
    }
    if ( KeyModeKeyringCodeText == text )
    {
        keyMode = KEYMODE_KEYRING_CODE;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::IsKeyringAvailable( void )
{GUCEF_TRACE;

    return CAuthCrypto::IsAeadAvailable() && CAuthCrypto::IsPassphraseKdfAvailable();
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::SetKeyringParameters( UInt32 memoryInKiB, UInt32 passes, UInt32 lanes )
{GUCEF_TRACE;

    if ( lanes < 1 )
        lanes = 1;
    if ( lanes > MaxKeyringLanes )
        lanes = MaxKeyringLanes;
    if ( passes < 1 )
        passes = 1;
    if ( passes > MaxKeyringPasses )
        passes = MaxKeyringPasses;
    if ( memoryInKiB < 8 * lanes )
        memoryInKiB = 8 * lanes;
    if ( memoryInKiB > MaxKeyringMemoryInKiB )
        memoryInKiB = MaxKeyringMemoryInKiB;
    MT::CScopeMutex lock( m_lock );
    m_keyringMemoryInKiB = memoryInKiB;
    m_keyringPasses = passes;
    m_keyringLanes = lanes;
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::GetKeyringParameters( UInt32& memoryInKiB, UInt32& passes, UInt32& lanes ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    memoryInKiB = m_keyringMemoryInKiB;
    passes = m_keyringPasses;
    lanes = m_keyringLanes;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::GetSealedKeyringParameters( UInt32& memoryInKiB, UInt32& passes, UInt32& lanes ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer file;
    {
        MT::CScopeMutex lock( m_lock );
        if ( KEYRING_OK != ReadKeyringFile( file ) )
            return false;
    }
    SKeyringHeader header;
    if ( KEYRING_OK != ParseKeyringHeader( file, header ) )
        return false;
    memoryInKiB = header.memoryInKiB;
    passes = header.passes;
    lanes = header.lanes;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::IsKeyringSetUp( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return CORE::FileExists( CORE::CombinePath( GetDirectory(), KeyringFileName ) );
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::IsKeyringLocked( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return IsKeyringModeWhileLocked() && !m_keyringUnlocked;
}

/*-------------------------------------------------------------------------*/

TKeyringLockReason
CFileCredentialStore::GetKeyringLockReason( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_keyringLockReason;
}

/*-------------------------------------------------------------------------*/

UInt64
CFileCredentialStore::GetKeyringLastUseInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_keyringUnlocked ? m_keyringLastUseMs : 0;
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CFileCredentialStore::CheckKeyringUse( const CSecretString& code ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( !IsKeyringModeWhileLocked() )
            return KEYRING_NOT_KEYRING_MODE;
    }
    if ( code.IsEmpty() || code.Length() > MaxKeyringCodeLength )
        return KEYRING_INVALID_CODE;
    if ( !IsKeyringAvailable() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: There is no \"" << CAuthCrypto::PassphraseKdf << "\" or \"" <<
            CAuthCrypto::AeadCipher << "\" codec, the keyring cannot be used" );
        return KEYRING_UNAVAILABLE;
    }
    return KEYRING_OK;
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CFileCredentialStore::ReadKeyringFile( CORE::CDynamicBuffer& file ) const
{GUCEF_TRACE;

    // The mutex of the instance is held by the caller
    CORE::CString dir = GetDirectory();
    CORE::CString path = CORE::CombinePath( dir, KeyringFileName );
    if ( !CORE::FileExists( path ) )
        return KEYRING_NOT_SET_UP;
    CFileLock fileLock;
    if ( CFileLock::LOCKOUTCOME_TIMED_OUT == fileLock.Acquire( CORE::CombinePath( dir, LockFileName ), false, m_lockTimeoutInMs ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The store in " << dir << " stayed locked by another instance for " << m_lockTimeoutInMs << " ms, the keyring is not read" );
        return KEYRING_FAILED;
    }
    if ( !CORE::FileExists( path ) )
        return KEYRING_NOT_SET_UP;
    if ( !file.LoadContentFromFile( path ) )
        return KEYRING_FAILED;
    return KEYRING_OK;
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::InstallKeyringKey( const UInt8* sealKey, const UInt8* check, const UInt8* header, UInt32 headerSize ) const
{GUCEF_TRACE;

    // The mutex of the instance is held by the caller
    m_keyringKey.Assign( sealKey, CAuthCrypto::KeySize );
    memcpy( m_keyringCheck, check, KeyringCheckSize );
    m_keyringHeader.Clear();
    m_keyringHeader.Append( header, headerSize );
    m_keyringUnlocked = true;
    m_keyringLastUseMs = GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::LockKeyringWhileLocked( TKeyringLockReason reason ) const
{GUCEF_TRACE;

    // The mutex of the instance is held by the caller. The secrets read with the key go with it
    bool wasUnlocked = m_keyringUnlocked;
    m_keyringKey.Clear();
    CAuthCrypto::Wipe( m_keyringCheck, sizeof( m_keyringCheck ) );
    m_keyringHeader.Clear();
    m_keyringUnlocked = false;
    m_keyringLastUseMs = 0;
    if ( IsKeyringModeWhileLocked() )
        m_slots.clear();
    if ( wasUnlocked )
    {
        m_keyringLockReason = reason;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The keyring in " << GetDirectory() << " is locked (" << KeyringLockReasonToString( reason ) << ")" );
    }
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::ParseKeyringImage( const SFileImage& image ) const
{GUCEF_TRACE;

    // The mutex of the instance is held by the caller; ParseImage() has emptied the view
    CORE::CString dataPath = CORE::CombinePath( GetDirectory(), KeyringFileName );
    if ( !image.hasData )
    {
        if ( m_keyringUnlocked )
            LockKeyringWhileLocked( KEYRINGLOCK_CHANGED_ELSEWHERE );
        return;
    }

    SKeyringHeader header;
    TKeyringResult headerResult = ParseKeyringHeader( image.data, header );
    if ( KEYRING_NEWER_VERSION == headerResult )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << dataPath << " is of a newer version, it is left alone" );
        m_newerVersion = true;
        LockKeyringWhileLocked( KEYRINGLOCK_CHANGED_ELSEWHERE );
        return;
    }
    if ( KEYRING_OK != headerResult )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << dataPath << " is not a keyring file" );
        m_tamperDetected = true;
        LockKeyringWhileLocked( KEYRINGLOCK_CHANGED_ELSEWHERE );
        return;
    }
    if ( !m_keyringUnlocked )
        return;     // locked: the header is all that can be looked at
    if ( !CAuthCrypto::ConstantTimeEquals( header.check, m_keyringCheck, KeyringCheckSize ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << dataPath << " was sealed with another code since the keyring was unlocked" );
        LockKeyringWhileLocked( KEYRINGLOCK_CHANGED_ELSEWHERE );
        return;
    }
    if ( !IsAvailable() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: There is no \"" << CAuthCrypto::AeadCipher << "\" codec, " << dataPath << " cannot be read" );
        m_loaded = false;
        return;
    }

    const UInt8* bytes = static_cast< const UInt8* >( image.data.GetConstBufferPtr() );
    CORE::CDynamicBuffer plain;
    bool opened = CAuthCrypto::AeadOpen( m_keyringKey.GetData(), bytes + KeyringHeaderSize, header.bytes, KeyringHeaderSize,
                                         bytes + KeyringHeaderSize + CAuthCrypto::NonceSize,
                                         image.data.GetDataSize() - KeyringHeaderSize - CAuthCrypto::NonceSize, plain );
    if ( !opened )
    {
        // the code is right (the check value matches): the data was changed
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << dataPath << " failed authentication with the right code, it was changed" );
        if ( plain.GetDataSize() > 0 )
            CAuthCrypto::Wipe( plain.GetBufferPtr(), plain.GetDataSize() );
        m_tamperDetected = true;
        return;
    }
    TSlotVector slots;
    bool valid = DeserializeSlots( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize(), slots );
    CAuthCrypto::Wipe( plain.GetBufferPtr(), plain.GetDataSize() );
    if ( !valid )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: " << dataPath << " has content that does not make sense" );
        m_tamperDetected = true;
        return;
    }
    m_slots = slots;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::WriteKeyringSlots( const CORE::CString& directory ,
                                         const TSlotVector& slots       ,
                                         const UInt8* header            ,
                                         UInt32 headerSize              ,
                                         const UInt8* sealKey           ,
                                         SFileImage& image              ) const
{GUCEF_TRACE;

    // The exclusive lock of the store file is held by the caller
    if ( KeyringHeaderSize != headerSize || GUCEF_NULL == sealKey )
        return false;
    CORE::CDynamicBuffer plain;
    SerializeSlots( slots, plain );
    UInt8 nonce[ CAuthCrypto::NonceSize ];
    CORE::CDynamicBuffer file;
    bool sealed = CAuthCrypto::RandomBytes( nonce, sizeof( nonce ) );
    if ( sealed )
    {
        file.Append( header, headerSize );
        file.Append( nonce, sizeof( nonce ) );
        sealed = CAuthCrypto::AeadSeal( sealKey, nonce, header, headerSize, plain.GetConstBufferPtr(), plain.GetDataSize(), file );
    }
    if ( plain.GetDataSize() > 0 )
        CAuthCrypto::Wipe( plain.GetBufferPtr(), plain.GetDataSize() );
    if ( !sealed )
        return false;

    CORE::CString dataPath = CORE::CombinePath( directory, KeyringFileName );
    if ( !CORE::WriteFileAtomically( dataPath, file ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot write " << dataPath );
        return false;
    }
    if ( !CORE::SetOwnerOnlyAccess( dataPath ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot restrict " << dataPath << " to the user" );

    image.hasData = true;
    image.data = file;
    image.hasKey = false;
    image.keyState = KEYSTATE_OK;
    image.ComputeDigest();
    return true;
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CFileCredentialStore::SetUpKeyring( const CSecretString& code )
{GUCEF_TRACE;

    TKeyringResult result = CheckKeyringUse( code );
    if ( KEYRING_OK != result )
        return result;

    SKeyringHeader header;
    {
        MT::CScopeMutex lock( m_lock );
        if ( CORE::FileExists( CORE::CombinePath( GetDirectory(), KeyringFileName ) ) )
            return KEYRING_ALREADY_SET_UP;
        if ( !MakeKeyringHeader( m_keyringMemoryInKiB, m_keyringPasses, m_keyringLanes, header ) )
            return KEYRING_FAILED;
    }

    // The derivation takes its time: without the lock of the instance, so that other threads are answered meanwhile
    UInt8 sealKey[ CAuthCrypto::KeySize ];
    if ( !DeriveKeyringKeys( code, header, sealKey, header.check ) )
        return KEYRING_UNAVAILABLE;
    memcpy( header.bytes + 18 + KeyringSaltSize, header.check, KeyringCheckSize );

    MT::CScopeMutex lock( m_lock );
    result = KEYRING_FAILED;
    CORE::CString dir = GetDirectory();
    if ( IsKeyringModeWhileLocked() && PrepareDirectory() )
    {
        CFileLock fileLock;
        CFileLock::TLockOutcome outcome = fileLock.Acquire( CORE::CombinePath( dir, LockFileName ), true, m_lockTimeoutInMs );
        if ( CFileLock::LOCKOUTCOME_TAKEN != outcome && CFileLock::IsSupported() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot lock the store in " << dir << ", the keyring is not set up" );
        }
        else
        if ( CORE::FileExists( CORE::CombinePath( dir, KeyringFileName ) ) )
        {
            result = KEYRING_ALREADY_SET_UP;
        }
        else
        {
            SFileImage image;
            TSlotVector none;
            if ( WriteKeyringSlots( dir, none, header.bytes, KeyringHeaderSize, sealKey, image ) )
            {
                LockKeyringWhileLocked( KEYRINGLOCK_REQUESTED );
                ForgetView();
                InstallKeyringKey( sealKey, header.check, header.bytes, KeyringHeaderSize );
                m_loaded = true;
                memcpy( m_fileDigest, image.digest, sizeof( m_fileDigest ) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Set up a keyring code for the store in " << dir );
                result = KEYRING_OK;
            }
        }
    }
    CAuthCrypto::Wipe( sealKey, sizeof( sealKey ) );
    return result;
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CFileCredentialStore::UnlockKeyring( const CSecretString& code )
{GUCEF_TRACE;

    TKeyringResult result = CheckKeyringUse( code );
    if ( KEYRING_INVALID_CODE == result )
        return KEYRING_WRONG_CODE;
    if ( KEYRING_OK != result )
        return result;

    CORE::CDynamicBuffer file;
    {
        MT::CScopeMutex lock( m_lock );
        result = ReadKeyringFile( file );
    }
    if ( KEYRING_OK != result )
        return result;
    SKeyringHeader header;
    result = ParseKeyringHeader( file, header );
    if ( KEYRING_OK != result )
    {
        MT::CScopeMutex lock( m_lock );
        RefreshView();     // notes the state of the file: tampered or newer
        return result;
    }

    // The derivation takes its time: without the lock of the instance, so that other threads are answered (locked) meanwhile
    UInt8 sealKey[ CAuthCrypto::KeySize ];
    UInt8 check[ KeyringCheckSize ];
    if ( !DeriveKeyringKeys( code, header, sealKey, check ) )
        return KEYRING_UNAVAILABLE;
    if ( !CAuthCrypto::ConstantTimeEquals( check, header.check, KeyringCheckSize ) )
    {
        CAuthCrypto::Wipe( sealKey, sizeof( sealKey ) );
        CAuthCrypto::Wipe( check, sizeof( check ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: A wrong code was given for the keyring" );
        return KEYRING_WRONG_CODE;
    }

    MT::CScopeMutex lock( m_lock );
    if ( !IsKeyringModeWhileLocked() )
    {
        CAuthCrypto::Wipe( sealKey, sizeof( sealKey ) );
        CAuthCrypto::Wipe( check, sizeof( check ) );
        return KEYRING_NOT_KEYRING_MODE;
    }
    InstallKeyringKey( sealKey, check, header.bytes, KeyringHeaderSize );
    CAuthCrypto::Wipe( sealKey, sizeof( sealKey ) );
    CAuthCrypto::Wipe( check, sizeof( check ) );
    ForgetView();
    if ( !RefreshView() )
    {
        LockKeyringWhileLocked( KEYRINGLOCK_REQUESTED );
        ForgetView();
        return KEYRING_FAILED;
    }
    if ( m_tamperDetected )
    {
        // the view stays as read (tampered) until the file changes; the key goes
        LockKeyringWhileLocked( KEYRINGLOCK_REQUESTED );
        return KEYRING_TAMPERED;
    }
    if ( !m_keyringUnlocked )
        return KEYRING_WRONG_CODE;      // sealed with another code between the derivation and now
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The keyring in " << GetDirectory() << " is unlocked" );
    return KEYRING_OK;
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::LockKeyring( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_keyringUnlocked )
        return;
    LockKeyringWhileLocked( KEYRINGLOCK_REQUESTED );
    ForgetView();
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CFileCredentialStore::ChangeKeyringCode( const CSecretString& currentCode, const CSecretString& newCode )
{GUCEF_TRACE;

    TKeyringResult result = CheckKeyringUse( newCode );
    if ( KEYRING_OK != result )
        return result;
    if ( currentCode.IsEmpty() || currentCode.Length() > MaxKeyringCodeLength )
        return KEYRING_WRONG_CODE;

    CORE::CDynamicBuffer file;
    SKeyringHeader newHeader;
    {
        MT::CScopeMutex lock( m_lock );
        result = ReadKeyringFile( file );
        if ( KEYRING_OK == result && !MakeKeyringHeader( m_keyringMemoryInKiB, m_keyringPasses, m_keyringLanes, newHeader ) )
            result = KEYRING_FAILED;
    }
    if ( KEYRING_OK != result )
        return result;
    SKeyringHeader header;
    result = ParseKeyringHeader( file, header );
    if ( KEYRING_OK != result )
        return result;

    // Both derivations without the lock of the instance
    UInt8 currentKey[ CAuthCrypto::KeySize ];
    UInt8 currentCheck[ KeyringCheckSize ];
    if ( !DeriveKeyringKeys( currentCode, header, currentKey, currentCheck ) )
        return KEYRING_UNAVAILABLE;
    if ( !CAuthCrypto::ConstantTimeEquals( currentCheck, header.check, KeyringCheckSize ) )
    {
        CAuthCrypto::Wipe( currentKey, sizeof( currentKey ) );
        CAuthCrypto::Wipe( currentCheck, sizeof( currentCheck ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: A wrong code was given to change the code of the keyring" );
        return KEYRING_WRONG_CODE;
    }
    UInt8 newKey[ CAuthCrypto::KeySize ];
    if ( !DeriveKeyringKeys( newCode, newHeader, newKey, newHeader.check ) )
    {
        CAuthCrypto::Wipe( currentKey, sizeof( currentKey ) );
        CAuthCrypto::Wipe( currentCheck, sizeof( currentCheck ) );
        return KEYRING_UNAVAILABLE;
    }
    memcpy( newHeader.bytes + 18 + KeyringSaltSize, newHeader.check, KeyringCheckSize );

    MT::CScopeMutex lock( m_lock );
    result = KEYRING_FAILED;
    CORE::CString dir = GetDirectory();
    CFileLock fileLock;
    CFileLock::TLockOutcome outcome = fileLock.Acquire( CORE::CombinePath( dir, LockFileName ), true, m_lockTimeoutInMs );
    if ( !IsKeyringModeWhileLocked() )
    {
        result = KEYRING_NOT_KEYRING_MODE;
    }
    else
    if ( CFileLock::LOCKOUTCOME_TAKEN != outcome && CFileLock::IsSupported() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot lock the store in " << dir << ", the code of the keyring is not changed" );
    }
    else
    {
        // what is in the file now, which has to be what the current code opens
        SFileImage image;
        ReadFiles( dir, image );
        SKeyringHeader nowHeader;
        TSlotVector slots;
        if ( !image.hasData )
        {
            result = KEYRING_NOT_SET_UP;
        }
        else
        if ( KEYRING_OK != ( result = ParseKeyringHeader( image.data, nowHeader ) ) )
        {
            // tampered or newer: as the view of the instance says after the next read
        }
        else
        if ( !CAuthCrypto::ConstantTimeEquals( nowHeader.check, currentCheck, KeyringCheckSize ) )
        {
            result = KEYRING_WRONG_CODE;    // sealed with another code meanwhile
        }
        else
        {
            const UInt8* bytes = static_cast< const UInt8* >( image.data.GetConstBufferPtr() );
            CORE::CDynamicBuffer plain;
            bool opened = CAuthCrypto::AeadOpen( currentKey, bytes + KeyringHeaderSize, nowHeader.bytes, KeyringHeaderSize,
                                                 bytes + KeyringHeaderSize + CAuthCrypto::NonceSize,
                                                 image.data.GetDataSize() - KeyringHeaderSize - CAuthCrypto::NonceSize, plain );
            bool valid = opened && DeserializeSlots( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize(), slots );
            if ( plain.GetDataSize() > 0 )
                CAuthCrypto::Wipe( plain.GetBufferPtr(), plain.GetDataSize() );
            if ( !valid )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The keyring in " << dir << " failed authentication with the right code, its code is not changed" );
                result = KEYRING_TAMPERED;
            }
            else
            if ( WriteKeyringSlots( dir, slots, newHeader.bytes, KeyringHeaderSize, newKey, image ) )
            {
                LockKeyringWhileLocked( KEYRINGLOCK_REQUESTED );
                ForgetView();
                InstallKeyringKey( newKey, newHeader.check, newHeader.bytes, KeyringHeaderSize );
                m_slots = slots;
                m_loaded = true;
                memcpy( m_fileDigest, image.digest, sizeof( m_fileDigest ) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The code of the keyring in " << dir << " was changed, the keyring was sealed again" );
                result = KEYRING_OK;
            }
        }
    }
    CAuthCrypto::Wipe( currentKey, sizeof( currentKey ) );
    CAuthCrypto::Wipe( currentCheck, sizeof( currentCheck ) );
    CAuthCrypto::Wipe( newKey, sizeof( newKey ) );
    if ( KEYRING_TAMPERED == result || KEYRING_NEWER_VERSION == result )
    {
        ForgetView();
        RefreshView();
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    config.SetAttribute( "directory", m_directory );
    config.SetAttribute( "appName", m_appName );
    config.SetAttribute( "lockTimeoutInMs", m_lockTimeoutInMs );
    if ( !m_keyFile.IsNULLOrEmpty() )
        config.SetAttribute( "keyFile", m_keyFile );
    if ( KEYFILETRUST_OWNER_ONLY != m_keyFileTrust )
        config.SetAttribute( "keyFileTrust", KeyFileTrustToString( m_keyFileTrust ) );
    if ( KEYMODE_KEY_FILE != m_keyMode )
        config.SetAttribute( "keyMode", KeyModeToString( m_keyMode ) );
    if ( DefaultKeyringMemoryInKiB != m_keyringMemoryInKiB )
        config.SetAttribute( "keyringMemoryInKiB", m_keyringMemoryInKiB );
    if ( DefaultKeyringPasses != m_keyringPasses )
        config.SetAttribute( "keyringPasses", m_keyringPasses );
    if ( DefaultKeyringLanes != m_keyringLanes )
        config.SetAttribute( "keyringLanes", m_keyringLanes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString appName = config.GetAttributeValueOrChildValueByName( "appName" ).AsString( m_appName, true );
    CORE::CString directory = config.GetAttributeValueOrChildValueByName( "directory" ).AsString( m_directory, false );
    UInt32 lockTimeoutInMs = config.GetAttributeValueOrChildValueByName( "lockTimeoutInMs" ).AsUInt32( m_lockTimeoutInMs, true );
    CORE::CString keyFile = config.GetAttributeValueOrChildValueByName( "keyFile" ).AsString( CORE::CString::Empty, false );
    CORE::CString keyFileTrustText = config.GetAttributeValueOrChildValueByName( "keyFileTrust" ).AsString( CORE::CString::Empty, true );
    EKeyFileTrust keyFileTrust = KEYFILETRUST_OWNER_ONLY;
    if ( !keyFileTrustText.IsNULLOrEmpty() && !KeyFileTrustFromString( keyFileTrustText, keyFileTrust ) )
    {
        // the strict check is the safe one
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Unknown keyFileTrust \"" << keyFileTrustText << "\", \"" <<
            KeyFileTrustOwnerOnlyText << "\" is used" );
        keyFileTrust = KEYFILETRUST_OWNER_ONLY;
    }
    CORE::CString keyModeText = config.GetAttributeValueOrChildValueByName( "keyMode" ).AsString( CORE::CString::Empty, true );
    EKeyMode keyMode = KEYMODE_KEY_FILE;
    if ( !keyModeText.IsNULLOrEmpty() && !KeyModeFromString( keyModeText, keyMode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Unknown keyMode \"" << keyModeText << "\", \"" <<
            KeyModeKeyFileText << "\" is used" );
        keyMode = KEYMODE_KEY_FILE;
    }
    UInt32 keyringMemoryInKiB = config.GetAttributeValueOrChildValueByName( "keyringMemoryInKiB" ).AsUInt32( DefaultKeyringMemoryInKiB, true );
    UInt32 keyringPasses = config.GetAttributeValueOrChildValueByName( "keyringPasses" ).AsUInt32( DefaultKeyringPasses, true );
    UInt32 keyringLanes = config.GetAttributeValueOrChildValueByName( "keyringLanes" ).AsUInt32( DefaultKeyringLanes, true );
    SetKeyringParameters( keyringMemoryInKiB, keyringPasses, keyringLanes );
    MT::CScopeMutex lock( m_lock );
    if ( keyMode != m_keyMode )
        LockKeyringWhileLocked( KEYRINGLOCK_REQUESTED );
    m_keyMode = keyMode;
    m_appName = appName;
    m_directory = directory;
    m_lockTimeoutInMs = lockTimeoutInMs;
    m_keyFile = keyFile;
    m_keyFileTrust = keyFileTrust;
    m_strayKeyWarned = false;
    ForgetView();
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFileCredentialStore::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CFileCredentialStore";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
