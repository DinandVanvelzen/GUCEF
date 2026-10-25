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

static const char FileMagic[ 4 ] = { 'G', 'A', 'U', 'C' };
static const UInt8 FileVersion = 1;
static const UInt32 HeaderSize = 5;
static const UInt32 MaxSlotCount = 1024;
static const UInt32 MaxNameSize = 1024;
static const UInt32 MaxSecretSize = 65536;
static const char KeyInfo[] = "gucefAUTH credentials v1";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
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

/*-------------------------------------------------------------------------*/

CFileCredentialStore::CFileCredentialStore( void )
    : ICredentialStore()
    , CORE::CIConfigurable()
    , m_lock()
    , m_appName( "GUCEF" )
    , m_directory()
    , m_slots()
    , m_loaded( false )
    , m_tamperDetected( false )
    , m_newerVersion( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::CFileCredentialStore( const CORE::CString& directory )
    : ICredentialStore()
    , CORE::CIConfigurable()
    , m_lock()
    , m_appName( "GUCEF" )
    , m_directory( directory )
    , m_slots()
    , m_loaded( false )
    , m_tamperDetected( false )
    , m_newerVersion( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::~CFileCredentialStore()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::SetDirectory( const CORE::CString& directory )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_directory = directory;
    m_slots.clear();
    m_loaded = false;
    m_tamperDetected = false;
    m_newerVersion = false;
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

bool
CFileCredentialStore::IsTamperDetected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
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

bool
CFileCredentialStore::ReadKey( UInt8* derivedKey )
{GUCEF_TRACE;

    CORE::CString keyPath = CORE::CombinePath( GetDirectory(), KeyFileName );
    if ( !CORE::FileExists( keyPath ) )
        return false;
    CORE::CDynamicBuffer keyFile;
    if ( !keyFile.LoadContentFromFile( keyPath ) || CAuthCrypto::KeySize != keyFile.GetDataSize() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The key file " << keyPath << " cannot be used" );
        return false;
    }
    bool derived = CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, keyFile.GetConstBufferPtr(), CAuthCrypto::KeySize, KeyInfo, (UInt32) sizeof( KeyInfo ) - 1, derivedKey, CAuthCrypto::KeySize );
    CAuthCrypto::Wipe( keyFile.GetBufferPtr(), keyFile.GetDataSize() );
    return derived;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::EnsureKey( UInt8* derivedKey )
{GUCEF_TRACE;

    if ( ReadKey( derivedKey ) )
        return true;

    // A new key: what was sealed with a key that is gone cannot be read anymore
    UInt8 keyMaterial[ CAuthCrypto::KeySize ];
    if ( !CAuthCrypto::RandomBytes( keyMaterial, sizeof( keyMaterial ) ) )
        return false;
    CORE::CString keyPath = CORE::CombinePath( GetDirectory(), KeyFileName );
    bool written = CORE::WriteFileAtomically( keyPath, keyMaterial, sizeof( keyMaterial ) );
    bool restricted = written && CORE::SetOwnerOnlyAccess( keyPath );
    bool derived = written && CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, keyMaterial, sizeof( keyMaterial ), KeyInfo, (UInt32) sizeof( KeyInfo ) - 1, derivedKey, CAuthCrypto::KeySize );
    CAuthCrypto::Wipe( keyMaterial, sizeof( keyMaterial ) );
    if ( !written )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot write the key file " << keyPath );
        return false;
    }
    if ( !restricted )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot restrict the key file " << keyPath << " to the user" );
    return derived;
}

/*-------------------------------------------------------------------------*/

void
CFileCredentialStore::EnsureLoaded( void )
{GUCEF_TRACE;

    // The lock is held by the caller
    if ( m_loaded )
        return;
    m_loaded = true;
    m_slots.clear();
    m_tamperDetected = false;
    m_newerVersion = false;

    CORE::CString dataPath = CORE::CombinePath( GetDirectory(), DataFileName );
    if ( !CORE::FileExists( dataPath ) )
        return;

    CORE::CDynamicBuffer file;
    if ( !file.LoadContentFromFile( dataPath ) || file.GetDataSize() < HeaderSize + CAuthCrypto::NonceSize + CAuthCrypto::TagSize ||
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

    UInt8 key[ CAuthCrypto::KeySize ];
    if ( !ReadKey( key ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: The key of " << dataPath << " is gone, its content cannot be read" );
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
            if ( GetByteLength( secretText ) > 0 )
                CAuthCrypto::Wipe( secretText.C_String(), GetByteLength( secretText ) );
            slots.push_back( slot );
        }
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
CFileCredentialStore::Save( void )
{GUCEF_TRACE;

    // The lock is held by the caller
    if ( m_newerVersion )
        return false;
    if ( !IsAvailable() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: There is no \"" << CAuthCrypto::AeadCipher << "\" codec, nothing is stored" );
        return false;
    }
    if ( !PrepareDirectory() )
        return false;

    UInt8 key[ CAuthCrypto::KeySize ];
    if ( !EnsureKey( key ) )
        return false;

    CORE::CDynamicBuffer plain;
    AppendUInt32( plain, (UInt32) m_slots.size() );
    for ( size_t i=0; i<m_slots.size(); ++i )
    {
        const SSlot& slot = m_slots[ i ];
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

    CORE::CString dataPath = CORE::CombinePath( GetDirectory(), DataFileName );
    if ( !CORE::WriteFileAtomically( dataPath, file ) )
        return false;
    if ( !CORE::SetOwnerOnlyAccess( dataPath ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileCredentialStore: Cannot restrict " << dataPath << " to the user" );
    m_tamperDetected = false;
    return true;
}

/*-------------------------------------------------------------------------*/

CFileCredentialStore::SSlot*
CFileCredentialStore::FindSlot( const CORE::CString& providerName, const CORE::CString& accountKey )
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

    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
    if ( m_newerVersion )
        return false;

    // The vector is in order of storing: a slot that is stored again moves to the back
    TSlotVector previous = m_slots;
    for ( size_t i=0; i<m_slots.size(); ++i )
    {
        if ( m_slots[ i ].providerName == providerName && m_slots[ i ].accountKey == accountKey )
        {
            m_slots.erase( m_slots.begin() + i );
            break;
        }
    }
    if ( m_slots.size() >= MaxSlotCount )
    {
        m_slots = previous;
        return false;
    }

    SSlot added;
    added.providerName = providerName;
    added.accountKey = accountKey;
    added.secret = secret;
    added.storedAtMs = GetEpochTimeInMs();
    m_slots.push_back( added );
    if ( !Save() )
    {
        m_slots = previous;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::Load( const CORE::CString& providerName ,
                            const CORE::CString& accountKey   ,
                            CSecretString& secret             )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
    SSlot* slot = FindSlot( providerName, accountKey );
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

    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
    for ( size_t i=0; i<m_slots.size(); ++i )
    {
        if ( m_slots[ i ].providerName == providerName && m_slots[ i ].accountKey == accountKey )
        {
            SSlot removed = m_slots[ i ];
            m_slots.erase( m_slots.begin() + i );
            if ( !Save() )
            {
                m_slots.insert( m_slots.begin() + i, removed );
                return false;
            }
            return true;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::ListAccounts( const CORE::CString& providerName        ,
                                    CORE::CString::StringVector& accountKeys )
{GUCEF_TRACE;

    accountKeys.clear();
    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
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

    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
    TSlotVector previous = m_slots;
    TSlotVector kept;
    for ( size_t i=0; i<m_slots.size(); ++i )
        if ( m_slots[ i ].providerName != providerName )
            kept.push_back( m_slots[ i ] );
    if ( kept.size() == m_slots.size() )
        return true;
    m_slots = kept;
    if ( !Save() )
    {
        m_slots = previous;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::Wipe( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_slots.clear();
    m_loaded = true;
    m_tamperDetected = false;
    m_newerVersion = false;

    bool wiped = true;
    CORE::CString dataPath = CORE::CombinePath( GetDirectory(), DataFileName );
    CORE::CString keyPath = CORE::CombinePath( GetDirectory(), KeyFileName );
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

    config.SetAttribute( "directory", m_directory );
    config.SetAttribute( "appName", m_appName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileCredentialStore::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString appName = config.GetAttributeValueOrChildValueByName( "appName" ).AsString( m_appName, true );
    CORE::CString directory = config.GetAttributeValueOrChildValueByName( "directory" ).AsString( m_directory, false );
    MT::CScopeMutex lock( m_lock );
    m_appName = appName;
    m_directory = directory;
    m_slots.clear();
    m_loaded = false;
    m_tamperDetected = false;
    m_newerVersion = false;
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
