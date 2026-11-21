/*
 *  AssetConverterLib: the files of a folder or an archive, for a pack recogniser
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
#include <string.h>

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CAssetPackListing.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPackListing::CAssetPackListing( void )
    : m_vfsRoot()
    , m_packName()
    , m_files()
    , m_lowerToListed()
    , m_texts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetPackListing::Clear( void )
{GUCEF_TRACE;

    m_vfsRoot.Clear();
    m_packName.Clear();
    m_files.clear();
    m_lowerToListed.clear();
    m_texts.clear();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackListing::LoadFromVfs( const CORE::CString& vfsFolder, UInt32 maxFiles )
{GUCEF_TRACE;

    Clear();
    m_vfsRoot = CVfsPaths::Normalize( vfsFolder );
    VFS::CVFS::TStringVector listed;
    VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( listed, m_vfsRoot, true, true, CORE::CString::Empty, maxFiles );
    for ( size_t i=0; i<listed.size(); ++i )
    {
        CORE::CString path;
        if ( !CVfsPaths::MakeRelative( m_vfsRoot, CVfsPaths::Normalize( listed[ i ] ), path ) )
            path = CVfsPaths::Normalize( listed[ i ] );
        if ( !path.IsNULLOrEmpty() )
            AddFile( path );
    }
    std::sort( m_files.begin(), m_files.end() );
    return !m_files.empty();
}

/*-------------------------------------------------------------------------*/

void
CAssetPackListing::AddFile( const CORE::CString& path )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return;
    if ( m_lowerToListed.insert( TLowerMap::value_type( path.Lowercase(), path ) ).second )
        m_files.push_back( path );
}

/*-------------------------------------------------------------------------*/

void
CAssetPackListing::AddFile( const CORE::CString& path, const CORE::CString& text )
{GUCEF_TRACE;

    AddFile( path );
    m_texts[ path.Lowercase() ] = text;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CAssetPackListing::GetFiles( void ) const
{GUCEF_TRACE;

    return m_files;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetPackListing::GetFileCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_files.size();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackListing::FindFile( const CORE::CString& path, CORE::CString& listedPath ) const
{GUCEF_TRACE;

    TLowerMap::const_iterator i = m_lowerToListed.find( path.Lowercase() );
    if ( i == m_lowerToListed.end() )
        return false;
    listedPath = i->second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackListing::HasFile( const CORE::CString& path ) const
{GUCEF_TRACE;

    return m_lowerToListed.find( path.Lowercase() ) != m_lowerToListed.end();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetPackListing::FindFiles( const CORE::CString& pattern, CORE::CString::StringVector& found ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<m_files.size(); ++i )
    {
        if ( m_files[ i ].WildcardEquals( pattern, '*', false ) )
        {
            found.push_back( m_files[ i ] );
            ++count;
        }
    }
    return count;
}

/*-------------------------------------------------------------------------*/

namespace {

struct SRanked
{
    Int32 score;
    UInt32 depth;
    UInt32 order;
    const CORE::CString* path;
};

bool
RankedBefore( const SRanked& a, const SRanked& b )
{
    if ( a.score != b.score )
        return a.score > b.score;
    if ( a.depth != b.depth )
        return a.depth < b.depth;
    return a.order < b.order;
}

}

/*-------------------------------------------------------------------------*/

UInt32
CAssetPackListing::FindFilesRanked( const CORE::CString& pattern, const CORE::CString::StringVector& keywords, CORE::CString::StringVector& found, UInt32 maxCount ) const
{GUCEF_TRACE;

    GUCEF::vector< SRanked > ranked;
    for ( size_t i=0; i<m_files.size(); ++i )
    {
        if ( !m_files[ i ].WildcardEquals( pattern, '*', false ) )
            continue;
        const Int32 slash = m_files[ i ].HasChar( '/', false );
        const CORE::CString name = ( slash < 0 ? m_files[ i ] : m_files[ i ].SubstrFromRange( (UInt32) slash+1, m_files[ i ].Length() ) ).Lowercase();
        SRanked entry;
        entry.score = 0;
        for ( size_t k=0; k<keywords.size(); ++k )
            if ( name.HasSubstr( keywords[ k ].Lowercase() ) >= 0 )
                ++entry.score;
        entry.depth = m_files[ i ].GetCharacterCount( '/' );
        entry.order = (UInt32) i;
        entry.path = &m_files[ i ];
        ranked.push_back( entry );
    }
    std::sort( ranked.begin(), ranked.end(), RankedBefore );
    UInt32 count = 0;
    for ( size_t i=0; i<ranked.size() && count < maxCount; ++i, ++count )
        found.push_back( *ranked[ i ].path );
    return count;
}

/*-------------------------------------------------------------------------*/

namespace {

bool
IsUpper( char c ) { return c >= 'A' && c <= 'Z'; }

bool
IsLower( char c ) { return c >= 'a' && c <= 'z'; }

bool
IsDigit( char c ) { return c >= '0' && c <= '9'; }

bool
IsLetterOrDigit( char c ) { return IsUpper( c ) || IsLower( c ) || IsDigit( c ) || ( (unsigned char) c ) >= 0x80; }

/** Words of a name that say nothing about which pack it is: the engine, the kind of download and the version */
bool
IsNoiseWord( const CORE::CString& word )
{
    static const char* const noise[] = { "unity", "unreal", "godot", "pack", "package", "unitypackage", "source", "sources", "files", "lite", "demo", "sample",
                                         "samples", "the", "and", "for", "assets", "asset", "version", "ver", "free", "zip", "ue", "ue4", "ue5", GUCEF_NULL };
    if ( word.Length() < 3 )
        return true;
    for ( const char* const* n = noise; GUCEF_NULL != *n; ++n )
        if ( word == *n )
            return true;
    // 2022, 2022_3 parts, v1, v1_6_6 and similar: a word that is only digits, or a version marker
    bool digitsOnly = true;
    for ( UInt32 i=0; i<word.Length(); ++i )
        digitsOnly = digitsOnly && IsDigit( word[ i ] );
    if ( digitsOnly )
        return true;
    return 'v' == word[ 0 ] && IsDigit( word[ 1 ] );
}

struct SPackRanked
{
    UInt32 folderScore;
    UInt32 keywordRank;
    UInt32 nameLength;
    UInt32 depth;
    UInt32 order;
    const CORE::CString* path;
};

bool
PackRankedBefore( const SPackRanked& a, const SPackRanked& b )
{
    if ( a.folderScore != b.folderScore )
        return a.folderScore > b.folderScore;
    if ( a.keywordRank != b.keywordRank )
        return a.keywordRank < b.keywordRank;
    if ( a.nameLength != b.nameLength )
        return a.nameLength < b.nameLength;
    if ( a.depth != b.depth )
        return a.depth < b.depth;
    return a.order < b.order;
}

}

/*-------------------------------------------------------------------------*/

void
CAssetPackListing::SplitWords( const CORE::CString& name, CORE::CString::StringVector& words )
{GUCEF_TRACE;

    CORE::CString word;
    const UInt32 length = name.Length();
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = name[ i ];
        if ( !IsLetterOrDigit( c ) )
        {
            if ( !word.IsNULLOrEmpty() )
                words.push_back( word.Lowercase() );
            word.Clear();
            continue;
        }
        if ( !word.IsNULLOrEmpty() )
        {
            const char previous = name[ i - 1 ];
            const bool lowerToUpper = IsLower( previous ) && IsUpper( c );
            const bool endOfUpperRun = IsUpper( previous ) && IsUpper( c ) && i + 1 < length && IsLower( name[ i + 1 ] );
            if ( lowerToUpper || endOfUpperRun )
            {
                words.push_back( word.Lowercase() );
                word.Clear();
            }
        }
        word += c;
    }
    if ( !word.IsNULLOrEmpty() )
        words.push_back( word.Lowercase() );
}

/*-------------------------------------------------------------------------*/

void
CAssetPackListing::SetPackName( const CORE::CString& name )
{GUCEF_TRACE;

    m_packName = name;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackListing::SetVfsRoot( const CORE::CString& vfsRoot )
{GUCEF_TRACE;

    m_vfsRoot = CVfsPaths::Normalize( vfsRoot );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackListing::GetPackName( void ) const
{GUCEF_TRACE;

    if ( !m_packName.IsNULLOrEmpty() )
        return m_packName;
    if ( m_vfsRoot.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return CVfsPaths::StemOf( m_vfsRoot );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetPackListing::FindFilesRankedForPack( const CORE::CString& pattern, const CORE::CString::StringVector& keywords, CORE::CString::StringVector& found, UInt32 maxCount ) const
{GUCEF_TRACE;

    // the words of the name of the pack
    CORE::CString::StringVector allWords;
    SplitWords( GetPackName(), allWords );
    GUCEF::set< CORE::CString > packWords;
    for ( size_t w=0; w<allWords.size(); ++w )
        if ( !IsNoiseWord( allWords[ w ] ) )
            packWords.insert( allWords[ w ] );

    GUCEF::vector< SPackRanked > ranked;
    for ( size_t i=0; i<m_files.size(); ++i )
    {
        if ( !m_files[ i ].WildcardEquals( pattern, '*', false ) )
            continue;
        const Int32 slash = m_files[ i ].HasChar( '/', false );
        const CORE::CString folder = slash < 0 ? CORE::CString::Empty : m_files[ i ].SubstrFromRange( 0, (UInt32) slash );
        const CORE::CString name = ( slash < 0 ? m_files[ i ] : m_files[ i ].SubstrFromRange( (UInt32) slash+1, m_files[ i ].Length() ) );
        const CORE::CString lowerName = name.Lowercase();

        SPackRanked entry;
        entry.folderScore = 0;
        if ( !packWords.empty() )
        {
            CORE::CString::StringVector folderWords;
            SplitWords( folder, folderWords );
            GUCEF::set< CORE::CString > met;
            for ( size_t w=0; w<folderWords.size(); ++w )
                if ( packWords.find( folderWords[ w ] ) != packWords.end() )
                    met.insert( folderWords[ w ] );
            entry.folderScore = (UInt32) met.size();
        }
        entry.keywordRank = (UInt32) keywords.size();
        for ( size_t k=0; k<keywords.size(); ++k )
        {
            if ( lowerName.HasSubstr( keywords[ k ].Lowercase() ) >= 0 )
            {
                entry.keywordRank = (UInt32) k;
                break;
            }
        }
        entry.nameLength = name.Length();
        entry.depth = m_files[ i ].GetCharacterCount( '/' );
        entry.order = (UInt32) i;
        entry.path = &m_files[ i ];
        ranked.push_back( entry );
    }
    std::sort( ranked.begin(), ranked.end(), PackRankedBefore );
    UInt32 count = 0;
    for ( size_t i=0; i<ranked.size() && count < maxCount; ++i, ++count )
        found.push_back( *ranked[ i ].path );
    return count;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackListing::GetSubFolders( const CORE::CString& folder, CORE::CString::StringVector& folders ) const
{GUCEF_TRACE;

    const CORE::CString prefix = folder.IsNULLOrEmpty() ? CORE::CString::Empty : folder.Lowercase() + "/";
    GUCEF::set< CORE::CString > seen;
    GUCEF::set< CORE::CString > listed;
    for ( size_t i=0; i<m_files.size(); ++i )
    {
        const CORE::CString lower = m_files[ i ].Lowercase();
        if ( lower.Length() <= prefix.Length() || 0 != memcmp( lower.C_String(), prefix.C_String(), prefix.Length() ) )
            continue;
        const CORE::CString rest = m_files[ i ].SubstrFromRange( prefix.Length(), m_files[ i ].Length() );
        const Int32 slash = rest.HasChar( '/', true );
        if ( slash <= 0 )
            continue;
        const CORE::CString name = rest.SubstrFromRange( 0, (UInt32) slash );
        if ( seen.insert( name.Lowercase() ).second )
            listed.insert( folder.IsNULLOrEmpty() ? name : folder + "/" + name );
    }
    for ( GUCEF::set< CORE::CString >::const_iterator f = listed.begin(); f != listed.end(); ++f )
        folders.push_back( *f );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackListing::ReadText( const CORE::CString& path, CORE::CString& text, UInt32 maxBytes ) const
{GUCEF_TRACE;

    CORE::CString listed;
    if ( !FindFile( path, listed ) )
        return false;
    TTextMap::const_iterator t = m_texts.find( listed.Lowercase() );
    if ( t != m_texts.end() )
    {
        text = t->second.Length() > maxBytes ? t->second.SubstrFromRange( 0, maxBytes ) : t->second;
        return true;
    }
    if ( m_vfsRoot.IsNULLOrEmpty() )
        return false;
    VFS::TBasicVfsResourcePtr resource = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( CVfsPaths::Join( m_vfsRoot, listed ), "rb", false );
    if ( resource.IsNULL() )
        return false;
    CORE::IOAccessPtr access = resource->GetAccess();
    if ( access.IsNULL() )
        return false;
    CORE::CString content;
    char buffer[ 4097 ];
    UInt32 total = 0;
    while ( total < maxBytes )
    {
        const UInt32 want = maxBytes - total < 4096 ? maxBytes - total : 4096;
        const UInt32 read = access->Read( buffer, 1, want );
        if ( 0 == read )
            break;
        buffer[ read ] = '\0';
        content.Append( buffer, read );
        total += read;
    }
    text = content;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackListing::TrimText( const CORE::CString& text )
{GUCEF_TRACE;

    UInt32 start = 0;
    UInt32 end = text.Length();
    while ( start < end && ( ' ' == text[ start ] || '\t' == text[ start ] || '\r' == text[ start ] || '\n' == text[ start ] ) )
        ++start;
    while ( end > start && ( ' ' == text[ end-1 ] || '\t' == text[ end-1 ] || '\r' == text[ end-1 ] || '\n' == text[ end-1 ] ) )
        --end;
    return text.SubstrFromRange( start, end );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetPackListing::GetVfsRoot( void ) const
{GUCEF_TRACE;

    return m_vfsRoot;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
