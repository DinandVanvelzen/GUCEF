/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
GetCodec( const CORE::CString& codecName, CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchStore::CMatchStore( void )
    : m_directory()
    , m_codecName( "json" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchStore::~CMatchStore()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMatchStore::SetDirectory( const CORE::CString& directory )
{GUCEF_TRACE;

    m_directory = directory;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchStore::GetDirectory( void ) const
{GUCEF_TRACE;

    return m_directory;
}

/*-------------------------------------------------------------------------*/

void
CMatchStore::SetCodecName( const CORE::CString& codecName )
{GUCEF_TRACE;

    m_codecName = codecName.Lowercase();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchStore::GetCodecName( void ) const
{GUCEF_TRACE;

    return m_codecName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchStore::GetRecordPath( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    if ( m_directory.IsNULLOrEmpty() || !CMatchId::IsValid( matchId ) )
        return CORE::CString::Empty;
    return CORE::CombinePath( m_directory, matchId + "." + m_codecName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchStore::GetTemporaryPath( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    CORE::CString path = GetRecordPath( matchId );
    return path.IsNULLOrEmpty() ? path : path + ".tmp";
}

/*-------------------------------------------------------------------------*/

bool
CMatchStore::WriteTemporary( const CMatchEntry& entry )
{GUCEF_TRACE;

    const CORE::CString path = GetRecordPath( entry.GetMatchId() );
    const CORE::CString tempPath = GetTemporaryPath( entry.GetMatchId() );
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( path.IsNULLOrEmpty() || !GetCodec( m_codecName, codec ) )
        return false;

    CORE::CDataNode root( "MatchEntry" );
    CORE::CDataNodeSerializableSettings settings;
    entry.Serialize( root, settings );

    CORE::CreateDirs( m_directory );
    CORE::DeleteFile( tempPath );
    bool verified = codec->StoreDataTree( &root, tempPath );
    if ( verified )
    {
        // what was written is read back before it can replace anything
        CORE::CDataNode check;
        CMatchEntry readBack;
        verified = codec->BuildDataTree( &check, tempPath ) && readBack.Deserialize( check, settings ) &&
                   readBack.GetMatchId() == entry.GetMatchId() && readBack.preview.revision == entry.preview.revision;
    }
    if ( !verified )
    {
        CORE::DeleteFile( tempPath );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MatchStore: A record of a match could not be written to " << m_directory );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchStore::Commit( const CORE::CString& matchId )
{GUCEF_TRACE;

    const CORE::CString path = GetRecordPath( matchId );
    const CORE::CString tempPath = GetTemporaryPath( matchId );
    if ( path.IsNULLOrEmpty() || !CORE::FileExists( tempPath ) )
        return false;
    if ( !CORE::MoveFile( tempPath, path, true ) )
    {
        CORE::DeleteFile( tempPath );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MatchStore: A record of a match could not be put in place in " << m_directory );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchStore::Save( const CMatchEntry& entry )
{GUCEF_TRACE;

    return WriteTemporary( entry ) && Commit( entry.GetMatchId() );
}

/*-------------------------------------------------------------------------*/

bool
CMatchStore::Load( const CORE::CString& matchId, CMatchEntry& entry ) const
{GUCEF_TRACE;

    const CORE::CString path = GetRecordPath( matchId );
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( path.IsNULLOrEmpty() || !CORE::FileExists( path ) || CORE::FileSize( path ) > MaxRecordFileSize || !GetCodec( m_codecName, codec ) )
        return false;

    CORE::CDataNode root;
    CMatchEntry loaded;
    CORE::CDataNodeSerializableSettings settings;
    if ( !codec->BuildDataTree( &root, path ) || !loaded.Deserialize( root, settings ) || loaded.GetMatchId() != matchId )
        return false;
    entry = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchStore::LoadAll( TMatchEntryVector& entries, UInt32* skipped ) const
{GUCEF_TRACE;

    entries.clear();
    if ( GUCEF_NULL != skipped )
        *skipped = 0;
    if ( m_directory.IsNULLOrEmpty() )
        return false;
    if ( !CORE::DirExists( m_directory ) )
        return true;

    RemoveLeftoverTemporaries();

    CORE::CString::StringVector names;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( m_directory ) )
    {
        do
        {
            if ( iterator.IsAFile() )
                names.push_back( iterator.GetResourceName() );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }

    const CORE::CString extension = "." + m_codecName;
    for ( size_t i=0; i<names.size(); ++i )
    {
        const CORE::CString& name = names[ i ];
        if ( name.Length() <= extension.Length() || name.Lowercase().SubstrFromRange( name.Length() - extension.Length(), name.Length() ) != extension )
            continue;
        CORE::CString matchId = name.SubstrFromRange( 0, name.Length() - extension.Length() );
        CMatchEntry entry;
        if ( CMatchId::IsValid( matchId ) && Load( matchId, entry ) )
            entries.push_back( entry );
        else
        if ( GUCEF_NULL != skipped )
            ++(*skipped);
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchStore::RemoveLeftoverTemporaries( void ) const
{GUCEF_TRACE;

    if ( m_directory.IsNULLOrEmpty() || !CORE::DirExists( m_directory ) )
        return 0;

    CORE::CString::StringVector names;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( m_directory ) )
    {
        do
        {
            if ( iterator.IsAFile() )
                names.push_back( iterator.GetResourceName() );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }

    const CORE::CString suffix = "." + m_codecName + ".tmp";
    UInt32 removed = 0;
    for ( size_t i=0; i<names.size(); ++i )
    {
        const CORE::CString& name = names[ i ];
        if ( name.Length() > suffix.Length() && name.Lowercase().SubstrFromRange( name.Length() - suffix.Length(), name.Length() ) == suffix &&
             CMatchId::IsValid( name.SubstrFromRange( 0, name.Length() - suffix.Length() ) ) )
        {
            if ( CORE::DeleteFile( CORE::CombinePath( m_directory, name ) ) )
                ++removed;
        }
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

bool
CMatchStore::Remove( const CORE::CString& matchId )
{GUCEF_TRACE;

    const CORE::CString path = GetRecordPath( matchId );
    if ( path.IsNULLOrEmpty() )
        return false;
    CORE::DeleteFile( GetTemporaryPath( matchId ) );
    return !CORE::FileExists( path ) || CORE::DeleteFile( path );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
