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

static const char FileMagic[ 4 ] = { 'G', 'A', 'U', 'C' };
static const UInt8 FileVersion = 1;
static const UInt32 HeaderSize = 5;
static const UInt32 MaxSlotCount = 1024;
static const UInt32 MaxNameSize = 1024;
static const UInt32 MaxSecretSize = 65536;
static const char KeyInfo[] = "gucefAUTH credentials v1";

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
    CORE::CDynamicBuffer key;               /**< the key material as it is in the key file, wiped when the image goes */
    UInt8 digest[ CORE::CSha256::DigestSize ];

    SFileImage( void )
        : hasData( false )
        , hasKey( false )
        , data()
        , key()
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
    , m_slots()
    , m_loaded( false )
    , m_tamperDetected( false )
    , m_newerVersion( false )
{GUCEF_TRACE;

    memset( m_fileDigest, 0, sizeof( m_fileDigest ) );
}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::CFileCredentialStore( const CORE::CString& directory )
    : ICredentialStore()
    , CORE::CIConfigurable()
    , m_lock()
    , m_appName( "GUCEF" )
    , m_directory( directory )
    , m_lockTimeoutInMs( DefaultLockTimeoutInMs )
    , m_slots()
    , m_loaded( false )
    , m_tamperDetected( false )
    , m_newerVersion( false )
{GUCEF_TRACE;

    memset( m_fileDigest, 0, sizeof( m_fileDigest ) );
}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::~CFileCredentialStore()
{GUCEF_TRACE;

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
    CORE::CString dataPath = CORE::CombinePath( directory, DataFileName );
    CORE::CString keyPath = CORE::CombinePath( directory, KeyFileName );
    image.hasData = CORE::FileExists( dataPath );
    if ( image.hasData && !image.data.LoadContentFromFile( dataPath ) )
        image.data.Clear();
    image.hasKey = CORE::FileExists( keyPath );
    if ( image.hasKey && !image.key.LoadContentFromFile( keyPath ) )
        image.key.Clear();
    image.ComputeDigest();
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
    m_loaded = true;
    memcpy( m_fileDigest, image.digest, sizeof( m_fileDigest ) );

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

    CBoundedReader reader( static_cast< const UInt8* >( plain.GetConstBufferPtr() ), plain.GetDataSize() );
    UInt32 count = 0;
    bool valid = reader.ReadUInt32( count ) && count <= MaxSlotCount;
    TSlotVector slots;
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
    CAuthCrypto::Wipe( plain.GetBufferPtr(), plain.GetDataSize() );
    if ( !valid || !reader.AtEnd() )
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
    if ( m_tamperDetected )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The store in " << dir << " failed its integrity check, nothing is written to it until it is wiped" );
        return false;
    }
    if ( m_newerVersion || !m_loaded )
        return false;

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
    if ( !changed )
        return true;

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
        return true;

    // Under the exclusive lock, which another instance may hold open: the lock file itself stays
    CFileLock fileLock;
    if ( CFileLock::LOCKOUTCOME_TIMED_OUT == fileLock.Acquire( CORE::CombinePath( dir, LockFileName ), true, m_lockTimeoutInMs ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The store in " << dir << " stayed locked by another instance for " << m_lockTimeoutInMs << " ms, it is not wiped" );
        return false;
    }

    bool wiped = true;
    CORE::CString dataPath = CORE::CombinePath( dir, DataFileName );
    CORE::CString keyPath = CORE::CombinePath( dir, KeyFileName );
    if ( CORE::FileExists( dataPath ) )
        wiped = CORE::DeleteFile( dataPath ) && wiped;
    if ( CORE::FileExists( keyPath ) )
        wiped = CORE::DeleteFile( keyPath ) && wiped;
    return wiped;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    config.SetAttribute( "directory", m_directory );
    config.SetAttribute( "appName", m_appName );
    config.SetAttribute( "lockTimeoutInMs", m_lockTimeoutInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString appName = config.GetAttributeValueOrChildValueByName( "appName" ).AsString( m_appName, true );
    CORE::CString directory = config.GetAttributeValueOrChildValueByName( "directory" ).AsString( m_directory, false );
    UInt32 lockTimeoutInMs = config.GetAttributeValueOrChildValueByName( "lockTimeoutInMs" ).AsUInt32( m_lockTimeoutInMs, true );
    MT::CScopeMutex lock( m_lock );
    m_appName = appName;
    m_directory = directory;
    m_lockTimeoutInMs = lockTimeoutInMs;
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
