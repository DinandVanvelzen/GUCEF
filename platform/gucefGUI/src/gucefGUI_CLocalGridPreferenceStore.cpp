/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefGUI_CLocalGridPreferenceStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CLocalGridPreferenceStore::JsonCodecName = "json";
const CString CLocalGridPreferenceStore::BinaryCodecName = "dnc";
const UInt32 CLocalGridPreferenceStore::MaxRecordBytes = 1024 * 1024;
const UInt32 CLocalGridPreferenceStore::MaxFileNameBytes = 120;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsRegisteredCodec( const CString& codecName )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    return !codecName.IsNULLOrEmpty() && CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------*/

/** FNV-1a over the bytes of the text: a short, stable tag for a long id */
CString
HashTag( const CString& text )
{
    // the offset basis 0xcbf29ce484222325 and the prime 0x100000001b3, made of 32 bit halves (no 64 bit literals in C++98)
    UInt64 hash = ( (UInt64) 0xcbf29ce4UL << 32 ) | (UInt64) 0x84222325UL;
    const UInt64 prime = ( (UInt64) 0x100UL << 32 ) | (UInt64) 0x000001b3UL;
    const CORE::CUtf8String utf8 = CORE::ToUtf8String( text );
    const char* bytes = utf8.C_String();
    const UInt32 count = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        hash ^= (UInt64) (unsigned char) bytes[ i ];
        hash *= prime;
    }
    static const char digits[] = "0123456789abcdef";
    char tag[ 17 ];
    for ( Int32 i=15; i>=0; --i )
    {
        tag[ i ] = digits[ hash & 0xF ];
        hash >>= 4;
    }
    tag[ 16 ] = '\0';
    return CString( tag );
}

/*-------------------------------------------------------------------------*/

bool
IsReservedDeviceName( const CString& name )
{
    static const char* const reserved[] = { "con", "prn", "aux", "nul",
                                            "com1", "com2", "com3", "com4", "com5", "com6", "com7", "com8", "com9",
                                            "lpt1", "lpt2", "lpt3", "lpt4", "lpt5", "lpt6", "lpt7", "lpt8", "lpt9" };
    const CString lower = name.Lowercase();
    for ( UInt32 i=0; i<(UInt32)( sizeof( reserved ) / sizeof( reserved[ 0 ] ) ); ++i )
        if ( lower == reserved[ i ] )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLocalGridPreferenceStore::CLocalGridPreferenceStore( const CString& directory, const CString& codecName )
    : CIGridPreferenceStore()
    , m_directory( directory )
    , m_codecName( codecName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLocalGridPreferenceStore::~CLocalGridPreferenceStore()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CLocalGridPreferenceStore::GetDirectory( void ) const
{GUCEF_TRACE;

    return m_directory;
}

/*-------------------------------------------------------------------------*/

CString
CLocalGridPreferenceStore::GetResolvedDirectory( void ) const
{GUCEF_TRACE;

    return CORE::RelativePath( m_directory );
}

/*-------------------------------------------------------------------------*/

void
CLocalGridPreferenceStore::SetCodecName( const CString& codecName )
{GUCEF_TRACE;

    m_codecName = codecName;
}

/*-------------------------------------------------------------------------*/

const CString&
CLocalGridPreferenceStore::GetCodecName( void ) const
{GUCEF_TRACE;

    return m_codecName;
}

/*-------------------------------------------------------------------------*/

CString
CLocalGridPreferenceStore::GetEffectiveCodecName( void ) const
{GUCEF_TRACE;

    if ( !m_codecName.IsNULLOrEmpty() )
        return m_codecName;
    return IsRegisteredCodec( JsonCodecName ) ? JsonCodecName : BinaryCodecName;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CLocalGridPreferenceStore::GetCandidateCodecs( void ) const
{GUCEF_TRACE;

    CString::StringVector codecs;
    codecs.push_back( GetEffectiveCodecName() );
    if ( codecs[ 0 ] != JsonCodecName )
        codecs.push_back( JsonCodecName );
    if ( codecs[ 0 ] != BinaryCodecName )
        codecs.push_back( BinaryCodecName );
    return codecs;
}

/*-------------------------------------------------------------------------*/

CString
CLocalGridPreferenceStore::GetRecordFileName( const CString& gridId )
{GUCEF_TRACE;

    static const char hex[] = "0123456789ABCDEF";
    const CORE::CUtf8String utf8 = CORE::ToUtf8String( gridId );
    const char* bytes = utf8.C_String();
    const UInt32 count = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    CString name;
    for ( UInt32 i=0; i<count; ++i )
    {
        const unsigned char c = (unsigned char) bytes[ i ];
        const bool plain = ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '_' == c || '-' == c || ( '.' == c && i > 0 );
        if ( plain )
            name += (char) c;
        else
        {
            name += '%';
            name += hex[ c >> 4 ];
            name += hex[ c & 0xF ];
        }
    }
    if ( name.IsNULLOrEmpty() )
        name = "%00";
    if ( IsReservedDeviceName( name ) )
        name += '_';
    if ( name.Length() > MaxFileNameBytes )
        name = name.SubstrToIndex( MaxFileNameBytes, true ) + "~" + HashTag( gridId );
    return name;
}

/*-------------------------------------------------------------------------*/

CString
CLocalGridPreferenceStore::GetRecordPath( const CString& gridId ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetResolvedDirectory(), GetRecordFileName( gridId ) + "." + GetEffectiveCodecName() );
}

/*-------------------------------------------------------------------------*/

bool
CLocalGridPreferenceStore::LoadFile( const CString& path, const CString& codecName, CGridPreferences& preferences ) const
{GUCEF_TRACE;

    const UInt64 size = CORE::FileSize( path );
    if ( size > (UInt64) MaxRecordBytes )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalGridPreferenceStore: \"" << path << "\" has " << size << " bytes, more than a record can have; it is not read" );
        return false;
    }
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LocalGridPreferenceStore: No codec \"" << codecName << "\" is registered to read \"" << path << "\"" );
        return false;
    }
    CORE::CDynamicBuffer content;
    if ( !content.LoadContentFromFile( path ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalGridPreferenceStore: \"" << path << "\" cannot be read" );
        return false;
    }
    CORE::CDataNode tree;
    bool decoded = false;
    if ( content.GetDataSize() > 0 )
    {
        CORE::CDynamicBufferAccess access( content );
        decoded = codec->BuildDataTree( &tree, &access );
    }
    if ( !decoded || !preferences.LoadFrom( tree ) )
    {
        preferences.Clear();
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalGridPreferenceStore: \"" << path << "\" holds no grid preferences that can be read; the grid keeps its defaults and the next save replaces the file" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalGridPreferenceStore::Load( const CString& gridId, CGridPreferences& preferences )
{GUCEF_TRACE;

    preferences.Clear();
    const CString base = CORE::CombinePath( GetResolvedDirectory(), GetRecordFileName( gridId ) );
    const CString::StringVector codecs = GetCandidateCodecs();
    for ( UInt32 i=0; i<(UInt32)codecs.size(); ++i )
    {
        const CString path = base + "." + codecs[ i ];
        if ( !CORE::FileExists( path ) )
            continue;
        return LoadFile( path, codecs[ i ], preferences );
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LocalGridPreferenceStore: No preferences for grid \"" << gridId << "\" in \"" << GetResolvedDirectory() << "\"" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLocalGridPreferenceStore::Save( const CString& gridId, const CGridPreferences& preferences )
{GUCEF_TRACE;

    const CString codecName = GetEffectiveCodecName();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalGridPreferenceStore: No codec \"" << codecName << "\" is registered: the preferences of grid \"" << gridId << "\" are not saved" );
        return false;
    }

    CGridPreferences record( preferences );
    record.gridId = gridId;
    CORE::CDataNode tree;
    record.SaveTo( tree );
    CORE::CDynamicBuffer content;
    {
        CORE::CDynamicBufferAccess access( content );
        if ( !codec->StoreDataTree( &tree, &access ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalGridPreferenceStore: The preferences of grid \"" << gridId << "\" cannot be encoded with codec \"" << codecName << "\"" );
            return false;
        }
    }

    const CString directory = GetResolvedDirectory();
    if ( !CORE::DirExists( directory ) && !CORE::CreateDirs( directory ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalGridPreferenceStore: The directory \"" << directory << "\" cannot be made: the preferences of grid \"" << gridId << "\" are not saved" );
        return false;
    }
    const CString path = GetRecordPath( gridId );
    if ( !CORE::WriteFileAtomically( path, content ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalGridPreferenceStore: \"" << path << "\" cannot be written: the preferences of grid \"" << gridId << "\" are not saved" );
        return false;
    }

    // a record of another codec would be found first after a change of the codecs: only one file per grid
    const CString base = CORE::CombinePath( directory, GetRecordFileName( gridId ) );
    const CString::StringVector codecs = GetCandidateCodecs();
    for ( UInt32 i=1; i<(UInt32)codecs.size(); ++i )
    {
        const CString other = base + "." + codecs[ i ];
        if ( CORE::FileExists( other ) )
            CORE::DeleteFile( other );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalGridPreferenceStore::Remove( const CString& gridId )
{GUCEF_TRACE;

    bool removed = true;
    const CString base = CORE::CombinePath( GetResolvedDirectory(), GetRecordFileName( gridId ) );
    const CString::StringVector codecs = GetCandidateCodecs();
    for ( UInt32 i=0; i<(UInt32)codecs.size(); ++i )
    {
        const CString path = base + "." + codecs[ i ];
        if ( CORE::FileExists( path ) && !CORE::DeleteFile( path ) )
            removed = false;
    }
    return removed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
