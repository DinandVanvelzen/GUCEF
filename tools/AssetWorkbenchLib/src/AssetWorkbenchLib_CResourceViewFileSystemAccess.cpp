/*
 *  AssetWorkbenchLib: the resources of a pack, grouped by function, as a file system for the content browser
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

#include <algorithm>

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXREADER_H
#include "AssetConverterLib_CPackIndexReader.h"
#define ASSETCONVERTERLIB_CPACKINDEXREADER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXREADER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H
#include "AssetConverterLib_CPackGroupVocabulary.h"
#define ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H
#endif /* ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetWorkbenchLib_CResourceViewFileSystemAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The most flat results a search lists at the root (the groups say how many there are in all) */
const UInt32 MaxSearchResults = 5000;

CORE::CString
Conform( const CORE::CString& path )
{
    CORE::CString conformed = path.ReplaceChar( '\\', '/' );
    while ( conformed.Length() > 1 && '/' == conformed[ conformed.Length()-1 ] )
        conformed = conformed.CutChars( 1, false );
    if ( conformed.IsNULLOrEmpty() || '/' != conformed[ 0 ] )
        conformed = "/" + conformed;
    return conformed;
}

void
Split( const CORE::CString& conformed, CORE::CString::StringVector& segments )
{
    segments.clear();
    CORE::CString::StringVector parts = conformed.ParseElements( '/', false );
    for ( size_t i=0; i<parts.size(); ++i )
        if ( !parts[ i ].IsNULLOrEmpty() )
            segments.push_back( parts[ i ] );
}

CORE::CString
Join( const CORE::CString::StringVector& segments, size_t from, size_t to )
{
    CORE::CString text;
    for ( size_t i=from; i<to; ++i )
    {
        if ( !text.IsNULLOrEmpty() )
            text += '/';
        text += segments[ i ];
    }
    return text;
}

bool
StartsWith( const CORE::CString& text, const CORE::CString& prefix )
{
    return text.Length() >= prefix.Length() && text.SubstrFromRange( 0, prefix.Length() ) == prefix;
}

}

/*-------------------------------------------------------------------------*/

CResourceViewFileSystemAccess::SGroupData::SGroupData( void )
    : loaded( false )
    , resources()
    , subCounts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CResourceViewFileSystemAccess::CResourceViewFileSystemAccess( const CORE::CString& displayName )
    : CORE::IFileSystemAccess()
    , GUI::CIContentItemOrder()
    , m_displayName( displayName )
    , m_manifest()
    , m_indexFolder()
    , m_packVfsRoot()
    , m_search()
    , m_groups()
    , m_listings()
    , m_shardStore()
    , m_materialised( 0 )
    , m_shardReads( 0 )
    , m_listingCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CResourceViewFileSystemAccess::~CResourceViewFileSystemAccess()
{GUCEF_TRACE;

    ReleaseStore();
}

/*-------------------------------------------------------------------------*/

void
CResourceViewFileSystemAccess::ReleaseStore( void )
{GUCEF_TRACE;

    m_groups.clear();
    m_listings.clear();
    for ( size_t i=0; i<m_shardStore.size(); ++i )
        GUCEF_DELETE m_shardStore[ i ];
    m_shardStore.clear();
}

/*-------------------------------------------------------------------------*/

void
CResourceViewFileSystemAccess::SetPack( const ASSETCONVERTER::CPackIndexDocument& document, const CORE::CString& indexFolder, const CORE::CString& packVfsRoot )
{GUCEF_TRACE;

    ReleaseStore();
    m_manifest = document;
    m_indexFolder = indexFolder;
    m_packVfsRoot = packVfsRoot;
    m_materialised = 0;
    m_shardReads = 0;
    m_listingCount = 0;
}

/*-------------------------------------------------------------------------*/
void
CResourceViewFileSystemAccess::AdoptPack( ASSETCONVERTER::CPackIndexDocument& document, const CORE::CString& indexFolder, const CORE::CString& packVfsRoot )
{GUCEF_TRACE;

    ReleaseStore();
    ASSETCONVERTER::CPackIndexResource::TResourceVector records;
    records.swap( document.resources );
    m_manifest = document;
    if ( records.size() <= MaxResidentResources )
        m_manifest.resources.swap( records );
    // the records are not given back: the caller does not need them
    m_indexFolder = indexFolder;
    m_packVfsRoot = packVfsRoot;
    m_materialised = 0;
    m_shardReads = 0;
    m_listingCount = 0;
}

/*-------------------------------------------------------------------------*/


void
CResourceViewFileSystemAccess::Clear( void )
{GUCEF_TRACE;

    SetPack( ASSETCONVERTER::CPackIndexDocument(), CORE::CString::Empty, CORE::CString::Empty );
    m_search.Clear();
}

/*-------------------------------------------------------------------------*/

bool CResourceViewFileSystemAccess::HasPack( void ) const { return !m_manifest.identity.packKey.IsNULLOrEmpty(); }
const ASSETCONVERTER::CPackIndexDocument& CResourceViewFileSystemAccess::GetManifest( void ) const { return m_manifest; }
const CORE::CString& CResourceViewFileSystemAccess::GetIndexFolder( void ) const { return m_indexFolder; }
const CORE::CString& CResourceViewFileSystemAccess::GetPackVfsRoot( void ) const { return m_packVfsRoot; }
const CORE::CString& CResourceViewFileSystemAccess::GetSearch( void ) const { return m_search; }
UInt32 CResourceViewFileSystemAccess::GetMaterialisedGroupCount( void ) const { return m_materialised; }
UInt32 CResourceViewFileSystemAccess::GetShardReadCount( void ) const { return m_shardReads; }
UInt32 CResourceViewFileSystemAccess::GetListingCount( void ) const { return m_listingCount; }

/*-------------------------------------------------------------------------*/

UInt32
CResourceViewFileSystemAccess::GetLoadedResourceCount( void ) const
{GUCEF_TRACE;

    UInt32 total = 0;
    for ( TGroupMap::const_iterator g = m_groups.begin(); g != m_groups.end(); ++g )
        total += (UInt32) (*g).second.resources.size();
    return total;
}

/*-------------------------------------------------------------------------*/

void
CResourceViewFileSystemAccess::SetSearch( const CORE::CString& text )
{GUCEF_TRACE;

    if ( text == m_search )
        return;
    m_search = text;
    m_listings.clear();
    for ( TGroupMap::iterator g = m_groups.begin(); g != m_groups.end(); ++g )
        if ( (*g).second.loaded )
            RecountGroup( (*g).second );
}

/*-------------------------------------------------------------------------*/

bool
CResourceViewFileSystemAccess::Matches( const ASSETCONVERTER::CPackIndexResource& resource ) const
{GUCEF_TRACE;

    if ( m_search.IsNULLOrEmpty() )
        return true;
    if ( m_search.HasChar( '*', true ) >= 0 || m_search.HasChar( '?', true ) >= 0 )
        return resource.name.WildcardEquals( m_search, '*', false );
    return resource.name.Lowercase().HasSubstr( m_search.Lowercase() ) >= 0;
}

/*-------------------------------------------------------------------------*/

void
CResourceViewFileSystemAccess::RecountGroup( SGroupData& data ) const
{GUCEF_TRACE;

    data.subCounts.clear();
    if ( m_search.IsNULLOrEmpty() )
    {
        for ( size_t i=0; i<data.resources.size(); ++i )
            ++data.subCounts[ data.resources[ i ]->subGroup ];
        return;
    }
    for ( size_t i=0; i<data.resources.size(); ++i )
        if ( Matches( *data.resources[ i ] ) )
            ++data.subCounts[ data.resources[ i ]->subGroup ];
}

/*-------------------------------------------------------------------------*/

CResourceViewFileSystemAccess::SGroupData*
CResourceViewFileSystemAccess::Materialise( const CORE::CString& groupId ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manifest.FindGroup( groupId ) )
        return GUCEF_NULL;
    SGroupData& data = m_groups[ groupId ];
    if ( data.loaded )
        return &data;
    if ( m_manifest.shards.empty() || !m_manifest.resources.empty() )
    {
        for ( size_t i=0; i<m_manifest.resources.size(); ++i )
            if ( m_manifest.resources[ i ].group == groupId )
                data.resources.push_back( &m_manifest.resources[ i ] );
    }
    else
    {
        ASSETCONVERTER::CPackIndexReader reader;
        for ( size_t s=0; s<m_manifest.shards.size(); ++s )
        {
            const ASSETCONVERTER::CPackIndexShardInfo& shard = m_manifest.shards[ s ];
            if ( !shard.group.IsNULLOrEmpty() && shard.group != groupId )
                continue;
            ASSETCONVERTER::CPackIndexResource::TResourceVector* read = GUCEF_NEW ASSETCONVERTER::CPackIndexResource::TResourceVector();
            if ( ASSETCONVERTER::CPackIndexReader::RESULT_OK != reader.ReadShard( m_indexFolder, shard, *read ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ResourceView: " << reader.GetMessage() );
                GUCEF_DELETE read;
                continue;
            }
            ++m_shardReads;
            m_shardStore.push_back( read );
            for ( size_t i=0; i<read->size(); ++i )
                if ( (*read)[ i ].group == groupId )
                    data.resources.push_back( &(*read)[ i ] );
        }
    }
    data.loaded = true;
    ++m_materialised;
    RecountGroup( data );
    return &data;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CResourceViewFileSystemAccess::DisplayOfSegment( const CORE::CString& segment )
{GUCEF_TRACE;

    CORE::CString text = segment.ReplaceChar( '-', ' ' ).ReplaceChar( '_', ' ' );
    if ( text.Length() > 0 )
    {
        CORE::CString head;
        head += (char) ( text[ 0 ] >= 'a' && text[ 0 ] <= 'z' ? text[ 0 ] - ( 'a' - 'A' ) : text[ 0 ] );
        text = head + text.SubstrFromRange( 1, text.Length() );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CResourceViewFileSystemAccess::FormatCount( UInt32 count )
{GUCEF_TRACE;

    const CORE::CString digits = CORE::UInt32ToString( count );
    CORE::CString text;
    for ( UInt32 i=0; i<digits.Length(); ++i )
    {
        if ( i > 0 && 0 == ( digits.Length() - i ) % 3 )
            text += ',';
        text += digits[ i ];
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CResourceViewFileSystemAccess::FolderTagOf( const CORE::CString& packPath )
{GUCEF_TRACE;

    CORE::CString::StringVector segments;
    Split( ASSETCONVERTER::CVfsPaths::FolderOf( packPath ), segments );
    const size_t from = segments.size() > 2 ? segments.size() - 2 : 0;
    CORE::CString tag;
    for ( size_t i=from; i<segments.size(); ++i )
    {
        if ( !tag.IsNULLOrEmpty() )
            tag += " > ";
        tag += segments[ i ];
    }
    return tag;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CResourceViewFileSystemAccess::MakeGroupPath( const CORE::CString& groupId )
{GUCEF_TRACE;

    return "/" + groupId;
}

/*-------------------------------------------------------------------------*/

bool
CResourceViewFileSystemAccess::GetGroupOfPath( const CORE::CString& path, CORE::CString& groupId, bool& isGroupFolder )
{GUCEF_TRACE;

    CORE::CString::StringVector segments;
    Split( Conform( path ), segments );
    if ( segments.empty() )
        return false;
    groupId = segments[ 0 ];
    isGroupFolder = 1 == segments.size();
    return true;
}

/*-------------------------------------------------------------------------*/

const CResourceViewFileSystemAccess::SListing*
CResourceViewFileSystemAccess::GetListing( const CORE::CString& groupId, const CORE::CString& subPath ) const
{GUCEF_TRACE;

    const CORE::CString key = groupId + "\n" + subPath;
    TListingMap::const_iterator cached = m_listings.find( key );
    if ( cached != m_listings.end() )
        return &(*cached).second;
    const SGroupData* data = Materialise( groupId );
    if ( GUCEF_NULL == data )
        return GUCEF_NULL;
    ++m_listingCount;

    SListing& listing = m_listings[ key ];
    listing = SListing();

    // the folders below: the next segment of every sub-group id below this folder
    GUCEF::map< CORE::CString, UInt32 > folders;
    const CORE::CString prefix = subPath.IsNULLOrEmpty() ? CORE::CString::Empty : subPath + "/";
    for ( GUCEF::map< CORE::CString, UInt32 >::const_iterator s = data->subCounts.begin(); s != data->subCounts.end(); ++s )
    {
        const CORE::CString& id = (*s).first;
        if ( id.IsNULLOrEmpty() || id == subPath || !StartsWith( id, prefix ) )
            continue;
        const CORE::CString rest = id.SubstrFromRange( prefix.Length(), id.Length() );
        const Int32 slash = rest.HasChar( '/', true );
        folders[ slash < 0 ? rest : rest.SubstrFromRange( 0, (UInt32) slash ) ] += (*s).second;
    }
    for ( GUCEF::map< CORE::CString, UInt32 >::const_iterator f = folders.begin(); f != folders.end(); ++f )
    {
        SListing::SFolder folder;
        folder.segment = (*f).first;
        folder.count = (*f).second;
        listing.folders.push_back( folder );
    }

    // the files: the records of this sub-group, with a leaf name that is unique in the folder
    GUCEF::map< CORE::CString, UInt32 > nameCounts;
    for ( size_t i=0; i<data->resources.size(); ++i )
    {
        const ASSETCONVERTER::CPackIndexResource& resource = *data->resources[ i ];
        if ( resource.subGroup != subPath || !Matches( resource ) )
            continue;
        listing.files.push_back( (UInt32) i );
        ++nameCounts[ resource.name ];
    }
    GUCEF::set< CORE::CString > taken;
    listing.leafNames.reserve( listing.files.size() );
    for ( size_t f=0; f<listing.files.size(); ++f )
    {
        const ASSETCONVERTER::CPackIndexResource& resource = *data->resources[ listing.files[ f ] ];
        CORE::CString leaf = resource.name;
        if ( leaf.HasChar( '/', true ) >= 0 || leaf.HasChar( '\\', true ) >= 0 )
            leaf = leaf.ReplaceChar( '/', '_' ).ReplaceChar( '\\', '_' );
        if ( nameCounts[ resource.name ] > 1 )
            leaf += " [" + FolderTagOf( resource.path ).ReplaceChar( '/', '_' ) + "]";
        if ( !taken.insert( leaf ).second )
        {
            leaf += " #" + resource.id.SubstrFromRange( 2, resource.id.Length() < 8 ? resource.id.Length() : 8 );
            taken.insert( leaf );
        }
        listing.leafNames.push_back( leaf );
        listing.leafToFile[ leaf ] = (UInt32) f;
    }
    if ( listing.files.size() > PageSize )
        listing.pageCount = (UInt32) ( ( listing.files.size() + PageSize - 1 ) / PageSize );
    return &listing;
}

/*-------------------------------------------------------------------------*/

Int32
CResourceViewFileSystemAccess::GetRank( const CORE::CString& path, bool isDirectory ) const
{GUCEF_TRACE;

    CORE::CString groupId;
    bool isGroupFolder = false;
    if ( !isDirectory || !GetGroupOfPath( path, groupId, isGroupFolder ) || !isGroupFolder )
        return 0;
    return 1 + (Int32) ASSETCONVERTER::CPackGroupVocabulary::GetGroupOrder( groupId );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CResourceViewFileSystemAccess::GetDisplayName( void ) const
{GUCEF_TRACE;

    return m_displayName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CResourceViewFileSystemAccess::GetRootPath( void ) const
{GUCEF_TRACE;

    return "/";
}

/*-------------------------------------------------------------------------*/

bool
CResourceViewFileSystemAccess::IsWriteable( const CORE::CString& path ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult CResourceViewFileSystemAccess::TryCreateDirectory( const CORE::CString&, bool ) { return CORE::FILESYSTEMACCESS_READ_ONLY; }
CORE::TFileSystemAccessResult CResourceViewFileSystemAccess::TryDeleteDirectory( const CORE::CString&, bool ) { return CORE::FILESYSTEMACCESS_READ_ONLY; }
CORE::TFileSystemAccessResult CResourceViewFileSystemAccess::TryMoveDirectory( const CORE::CString&, const CORE::CString& ) { return CORE::FILESYSTEMACCESS_READ_ONLY; }
CORE::TFileSystemAccessResult CResourceViewFileSystemAccess::TryMoveFile( const CORE::CString&, const CORE::CString&, bool ) { return CORE::FILESYSTEMACCESS_READ_ONLY; }
CORE::TFileSystemAccessResult CResourceViewFileSystemAccess::TryRenameFile( const CORE::CString&, const CORE::CString& ) { return CORE::FILESYSTEMACCESS_READ_ONLY; }
CORE::TFileSystemAccessResult CResourceViewFileSystemAccess::TryDeleteFile( const CORE::CString& ) { return CORE::FILESYSTEMACCESS_READ_ONLY; }

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CResourceViewFileSystemAccess::TryMoveFiles( const CORE::CString::StringVector& filePaths     ,
                                             const CORE::CString& destinationDir             ,
                                             bool overwrite                                  ,
                                             TFileSystemAccessResultVector* perFileResults   )
{GUCEF_TRACE;

    if ( GUCEF_NULL != perFileResults )
        perFileResults->assign( filePaths.size(), CORE::FILESYSTEMACCESS_READ_ONLY );
    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

namespace {

/** The page a path segment of the form "~p12" names; 0 when it is none */
UInt32
PageOfSegment( const CORE::CString& segment )
{
    if ( segment.Length() < 3 || '~' != segment[ 0 ] || 'p' != segment[ 1 ] )
        return 0;
    for ( UInt32 i=2; i<segment.Length(); ++i )
        if ( segment[ i ] < '0' || segment[ i ] > '9' )
            return 0;
    return CORE::StringToUInt32( segment.SubstrFromRange( 2, segment.Length() ), 0 );
}

}

/*-------------------------------------------------------------------------*/

bool
CResourceViewFileSystemAccess::Locate( const CORE::CString& path, SLocation& location ) const
{GUCEF_TRACE;

    location = SLocation();
    CORE::CString::StringVector segments;
    Split( Conform( path ), segments );
    if ( segments.empty() )
    {
        location.type = 1;
        return true;
    }
    const SGroupData* data = Materialise( segments[ 0 ] );
    if ( GUCEF_NULL == data )
        return false;
    location.group = segments[ 0 ];
    if ( 1 == segments.size() )
    {
        location.type = 1;
        return true;
    }
    const size_t last = segments.size() - 1;

    // a page of a folder that has many files
    const UInt32 page = PageOfSegment( segments[ last ] );
    if ( page > 0 )
    {
        const SListing* listing = GetListing( location.group, Join( segments, 1, last ) );
        if ( GUCEF_NULL == listing || page > listing->pageCount )
            return false;
        location.type = 1;
        location.sub = Join( segments, 1, last );
        location.page = page;
        return true;
    }

    // a folder: a sub-group, or the first part of one
    const CORE::CString asFolder = Join( segments, 1, segments.size() );
    for ( GUCEF::map< CORE::CString, UInt32 >::const_iterator s = data->subCounts.begin(); s != data->subCounts.end(); ++s )
    {
        if ( (*s).first == asFolder || StartsWith( (*s).first, asFolder + "/" ) )
        {
            location.type = 1;
            location.sub = asFolder;
            return true;
        }
    }

    // a file: of a folder, or of a page of a folder
    CORE::CString sub = Join( segments, 1, last );
    UInt32 filePage = 0;
    if ( last >= 2 && PageOfSegment( segments[ last-1 ] ) > 0 )
    {
        filePage = PageOfSegment( segments[ last-1 ] );
        sub = Join( segments, 1, last-1 );
    }
    const SListing* listing = GetListing( location.group, sub );
    if ( GUCEF_NULL == listing )
        return false;
    GUCEF::map< CORE::CString, UInt32 >::const_iterator found = listing->leafToFile.find( segments[ last ] );
    if ( found == listing->leafToFile.end() )
        return false;
    const UInt32 position = (*found).second;
    if ( listing->pageCount > 0 ? filePage != 1 + position / PageSize : 0 != filePage )
        return false;
    location.type = 2;
    location.sub = sub;
    location.page = filePage;
    location.leaf = segments[ last ];
    location.position = position;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CResourceViewFileSystemAccess::TryGetEntryType( const CORE::CString& path, CORE::TFileSystemEntryType& entryType ) const
{GUCEF_TRACE;

    entryType = CORE::FILESYSTEMENTRYTYPE_NONE;
    SLocation location;
    if ( Locate( path, location ) )
        entryType = 1 == location.type ? CORE::FILESYSTEMENTRYTYPE_DIRECTORY : CORE::FILESYSTEMENTRYTYPE_FILE;
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

bool
CResourceViewFileSystemAccess::TryResolve( const CORE::CString& path, ASSETCONVERTER::CPackIndexResource& resource ) const
{GUCEF_TRACE;

    SLocation location;
    if ( !Locate( path, location ) || 2 != location.type )
        return false;
    const SListing* listing = GetListing( location.group, location.sub );
    const SGroupData* data = Materialise( location.group );
    resource = *data->resources[ listing->files[ location.position ] ];
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CResourceViewFileSystemAccess::GetVfsPathOf( const ASSETCONVERTER::CPackIndexResource& resource ) const
{GUCEF_TRACE;

    return ASSETCONVERTER::CVfsPaths::Join( m_packVfsRoot, resource.path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CResourceViewFileSystemAccess::GetViewPathOf( const ASSETCONVERTER::CPackIndexResource& resource ) const
{GUCEF_TRACE;

    const SListing* listing = GetListing( resource.group, resource.subGroup );
    const SGroupData* data = Materialise( resource.group );
    if ( GUCEF_NULL == listing || GUCEF_NULL == data )
        return CORE::CString::Empty;
    for ( size_t f=0; f<listing->files.size(); ++f )
    {
        if ( data->resources[ listing->files[ f ] ]->path != resource.path )
            continue;
        CORE::CString path = "/" + resource.group + ( resource.subGroup.IsNULLOrEmpty() ? CORE::CString::Empty : "/" + resource.subGroup );
        if ( listing->pageCount > 0 )
            path += "/~p" + CORE::UInt32ToString( 1 + (UInt32) f / PageSize );
        return path + "/" + listing->leafNames[ f ];
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CResourceViewFileSystemAccess::GetGroupTextOf( const ASSETCONVERTER::CPackIndexResource& resource ) const
{GUCEF_TRACE;

    CORE::CString text = ASSETCONVERTER::CPackGroupVocabulary::GetGroupName( resource.group );
    CORE::CString::StringVector segments;
    Split( "/" + resource.subGroup, segments );
    for ( size_t i=0; i<segments.size(); ++i )
        text += " / " + DisplayOfSegment( segments[ i ] );
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CResourceViewFileSystemAccess::GetFolderSummary( const CORE::CString& path, CORE::CString& text, UInt32& count, UInt64& bytes ) const
{GUCEF_TRACE;

    count = 0;
    bytes = 0;
    CORE::CString::StringVector segments;
    Split( Conform( path ), segments );
    if ( segments.empty() )
    {
        text = m_displayName;
        count = m_manifest.counts.resources;
        bytes = m_manifest.counts.bytes;
        return HasPack();
    }
    const SGroupData* data = Materialise( segments[ 0 ] );
    if ( GUCEF_NULL == data )
        return false;
    const CORE::CString sub = Join( segments, 1, segments.size() );
    text = ASSETCONVERTER::CPackGroupVocabulary::GetGroupName( segments[ 0 ] );
    for ( size_t i=1; i<segments.size(); ++i )
        if ( 0 == PageOfSegment( segments[ i ] ) )
            text += " / " + DisplayOfSegment( segments[ i ] );
    for ( size_t i=0; i<data->resources.size(); ++i )
    {
        const ASSETCONVERTER::CPackIndexResource& resource = *data->resources[ i ];
        if ( sub.IsNULLOrEmpty() || resource.subGroup == sub || StartsWith( resource.subGroup, sub + "/" ) )
        {
            ++count;
            bytes += resource.size;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CResourceViewFileSystemAccess::TryGetMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    SLocation location;
    if ( !Locate( path, location ) )
        return CORE::FILESYSTEMACCESS_NOT_FOUND;
    const CORE::CString conformed = Conform( path );
    metaData.resourceExists = true;
    metaData.name = "/" == conformed ? m_displayName : conformed.SubstrFromRange( (UInt32) ( conformed.HasChar( '/', false ) + 1 ), conformed.Length() );
    metaData.hasName = true;
    if ( 2 == location.type )
    {
        ASSETCONVERTER::CPackIndexResource resource;
        if ( TryResolve( path, resource ) )
        {
            metaData.hasResourceSizeInBytes = true;
            metaData.resourceSizeInBytes = resource.size;
        }
    }
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

void
CResourceViewFileSystemAccess::AddFileEntries( const SGroupData& data, const SListing& listing, const CORE::CString& dirPath, UInt32 from, UInt32 to, bool withGroupText, CORE::CFileSystemEntryListIterator::TEntryVector& entries ) const
{GUCEF_TRACE;

    if ( to > (UInt32) listing.files.size() )
        to = (UInt32) listing.files.size();
    const CORE::CString prefix = ( "/" == dirPath ? CORE::CString::Empty : dirPath ) + "/";
    entries.reserve( entries.size() + ( to > from ? to - from : 0 ) );
    for ( UInt32 f=from; f<to; ++f )
    {
        const ASSETCONVERTER::CPackIndexResource& resource = *data.resources[ listing.files[ f ] ];
        CORE::CFileSystemEntryListIterator::CEntry entry;
        entry.path = prefix + listing.leafNames[ f ];
        entry.name = withGroupText ? listing.leafNames[ f ] + "  (" + GetGroupTextOf( resource ).ReplaceChar( '/', '>' ) + ")" : listing.leafNames[ f ];
        entry.isFile = true;
        entry.hasMetaData = true;
        entry.metaData.resourceExists = true;
        entry.metaData.hasResourceSizeInBytes = true;
        entry.metaData.resourceSizeInBytes = resource.size;
        entries.push_back( entry );
    }
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CResourceViewFileSystemAccess::TryGetFileSystemIterator( const CORE::CString& dirPath, CORE::TFileSystemIteratorPtr& iterator ) const
{GUCEF_TRACE;

    const CORE::CString conformed = Conform( dirPath );
    CORE::CFileSystemEntryListIterator::TEntryVector entries;
    SLocation location;
    if ( !Locate( conformed, location ) )
        return CORE::FILESYSTEMACCESS_NOT_FOUND;
    if ( 1 != location.type )
        return CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY;

    if ( location.group.IsNULLOrEmpty() )
    {
        // the groups, with the counts of the manifest, or those of the matches of a search
        for ( size_t g=0; g<m_manifest.groups.size(); ++g )
        {
            const ASSETCONVERTER::CPackIndexGroup& group = m_manifest.groups[ g ];
            UInt32 count = group.count;
            if ( !m_search.IsNULLOrEmpty() )
            {
                count = 0;
                const SGroupData* data = Materialise( group.id );
                if ( GUCEF_NULL != data )
                    for ( GUCEF::map< CORE::CString, UInt32 >::const_iterator s = data->subCounts.begin(); s != data->subCounts.end(); ++s )
                        count += (*s).second;
            }
            if ( 0 == count )
                continue;
            CORE::CFileSystemEntryListIterator::CEntry entry;
            entry.name = group.name + " (" + FormatCount( count ) + ")";
            entry.path = MakeGroupPath( group.id );
            entry.isDirectory = true;
            entries.push_back( entry );
        }
        // the matches of a search, flat, whatever the group (the first of them: the groups say how many there are)
        if ( !m_search.IsNULLOrEmpty() )
        {
            UInt32 listed = 0;
            for ( size_t g=0; g<m_manifest.groups.size() && listed < MaxSearchResults; ++g )
            {
                const SGroupData* data = Materialise( m_manifest.groups[ g ].id );
                if ( GUCEF_NULL == data )
                    continue;
                for ( GUCEF::map< CORE::CString, UInt32 >::const_iterator s = data->subCounts.begin(); s != data->subCounts.end() && listed < MaxSearchResults; ++s )
                {
                    const SListing* listing = GetListing( m_manifest.groups[ g ].id, (*s).first );
                    if ( GUCEF_NULL == listing )
                        continue;
                    const CORE::CString folder = "/" + m_manifest.groups[ g ].id + ( (*s).first.IsNULLOrEmpty() ? CORE::CString::Empty : "/" + (*s).first );
                    const UInt32 room = MaxSearchResults - listed;
                    const UInt32 take = (UInt32) listing->files.size() < room ? (UInt32) listing->files.size() : room;
                    // a leaf of a folder that is paged is reached through its page: the flat result names the page
                    for ( UInt32 f=0; f<take; f += PageSize )
                    {
                        const CORE::CString where = listing->pageCount > 0 ? folder + "/~p" + CORE::UInt32ToString( 1 + f / PageSize ) : folder;
                        AddFileEntries( *data, *listing, where, f, f + PageSize < take ? f + PageSize : take, true, entries );
                    }
                    listed += take;
                }
            }
        }
    }
    else
    {
        const SListing* listing = GetListing( location.group, location.sub );
        const SGroupData* data = Materialise( location.group );
        if ( GUCEF_NULL == listing || GUCEF_NULL == data )
            return CORE::FILESYSTEMACCESS_NOT_FOUND;
        if ( 0 == location.page )
        {
            for ( size_t f=0; f<listing->folders.size(); ++f )
            {
                CORE::CFileSystemEntryListIterator::CEntry entry;
                entry.name = DisplayOfSegment( listing->folders[ f ].segment ) + " (" + FormatCount( listing->folders[ f ].count ) + ")";
                entry.path = conformed + "/" + listing->folders[ f ].segment;
                entry.isDirectory = true;
                entries.push_back( entry );
            }
            if ( listing->pageCount > 0 )
            {
                // too many files for one folder: the pages are the folders ("Items 0001 - 0500"), made when they are opened
                UInt32 width = (UInt32) CORE::UInt32ToString( (UInt32) listing->files.size() ).Length();
                for ( UInt32 p=0; p<listing->pageCount; ++p )
                {
                    const UInt32 first = p * PageSize + 1;
                    const UInt32 lastFile = ( p + 1 ) * PageSize < (UInt32) listing->files.size() ? ( p + 1 ) * PageSize : (UInt32) listing->files.size();
                    CORE::CString from = CORE::UInt32ToString( first );
                    CORE::CString to = CORE::UInt32ToString( lastFile );
                    while ( from.Length() < width )
                        from = "0" + from;
                    while ( to.Length() < width )
                        to = "0" + to;
                    CORE::CFileSystemEntryListIterator::CEntry entry;
                    entry.name = "Items " + from + " - " + to + " (" + CORE::UInt32ToString( lastFile - first + 1 ) + ")";
                    entry.path = conformed + "/~p" + CORE::UInt32ToString( p + 1 );
                    entry.isDirectory = true;
                    entries.push_back( entry );
                }
            }
            else
            {
                AddFileEntries( *data, *listing, conformed, 0, (UInt32) listing->files.size(), false, entries );
            }
        }
        else
        {
            AddFileEntries( *data, *listing, conformed, ( location.page - 1 ) * PageSize, location.page * PageSize, false, entries );
        }
    }
    iterator = CORE::TFileSystemIteratorPtr( GUCEF_NEW CORE::CFileSystemEntryListIterator( conformed, entries ) );
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CResourceViewFileSystemAccess::ListAvailableSpecialDirs( CORE::TSpecialDirSet& specialDirs ) const
{GUCEF_TRACE;

    specialDirs.clear();
    return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CResourceViewFileSystemAccess::TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
