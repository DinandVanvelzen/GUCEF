/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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
#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "TestArchiveConformanceTree.h"

using namespace GUCEF;

typedef CORE::CString CString;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The names with non-ASCII characters are written as UTF-8 byte escapes, split in separate string literals so that
 *  a hexadecimal escape never swallows the character that follows it
 */
#define CONFORMANCE_UTF8_U_UMLAUT      "\xC3\xBC"
#define CONFORMANCE_UTF8_I_UMLAUT      "\xC3\xAF"
#define CONFORMANCE_UTF8_E_ACUTE       "\xC3\xA9"
#define CONFORMANCE_UTF8_KATAKANA_FU   "\xE3\x83\x95"
#define CONFORMANCE_UTF8_KATAKANA_A    "\xE3\x82\xA1"
#define CONFORMANCE_UTF8_CYRILLIC_FA   "\xD1\x84\xD0\xB0"

const char* const CArchiveConformanceTree::EmptyDir = "empty_dir";
const char* const CArchiveConformanceTree::EmptyFile = "empty.bin";
const char* const CArchiveConformanceTree::OneByteFile = "one.bin";
const char* const CArchiveConformanceTree::BigFile = "big.bin";
const char* const CArchiveConformanceTree::SpacesFile = "name with spaces.txt";
const char* const CArchiveConformanceTree::SpacesDir = "dir with spaces";
const char* const CArchiveConformanceTree::FilterDir = "filter";
const char* const CArchiveConformanceTree::ManyFilesDir = "many";
const char* const CArchiveConformanceTree::DeepFile = "deep/d01/d02/d03/d04/d05/d06/d07/d08/d09/d10/d11/d12/d13/d14/d15/d16/deep.bin";
const char* const CArchiveConformanceTree::DupDir = "dup";
const char* const CArchiveConformanceTree::MutateDir = "mutate";
const char* const CArchiveConformanceTree::LargeFile = "large/sparse.bin";

const UInt32 CArchiveConformanceTree::ManyFilesCount = 600;

/*-------------------------------------------------------------------------*/

namespace {

UInt64
MakeUInt64( UInt32 high, UInt32 low )
{
    return ( (UInt64) high << 32 ) | (UInt64) low;
}

/** A mixing function of the splitmix64 family: every input gives a well spread output, so the content has no repeating pattern */
UInt64
MixWord( UInt32 contentId, UInt64 wordIndex )
{
    UInt64 x = wordIndex + MakeUInt64( 0x9E3779B9, 0x7F4A7C15 ) * ( (UInt64) contentId + 1 );
    x = ( x ^ ( x >> 30 ) ) * MakeUInt64( 0xBF58476D, 0x1CE4E5B9 );
    x = ( x ^ ( x >> 27 ) ) * MakeUInt64( 0x94D049BB, 0x133111EB );
    return x ^ ( x >> 31 );
}

bool
EntryPathLess( const CArchiveConformanceTree::SEntry& a, const CArchiveConformanceTree::SEntry& b )
{
    return a.path < b.path;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

CArchiveConformanceTree::SEntry::SEntry( void )
    : path()
    , isDirectory( false )
    , size( 0 )
    , contentId( 0 )
    , isZeroFilled( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CArchiveConformanceTree::CArchiveConformanceTree( bool includeLargeFile )
    : m_entries()
    , m_hasLargeFile( includeLargeFile )
{GUCEF_TRACE;

    // the content ids are unique per file, except where files are meant to be equal
    UInt32 id = 1;

    AddFile( EmptyFile, 0, id++ );
    AddFile( OneByteFile, 1, id++ );
    AddFile( "size_65535.bin", 65535, id++ );
    AddFile( "size_65536.bin", 65536, id++ );
    AddFile( "size_65537.bin", 65537, id++ );
    AddFile( BigFile, 3 * 1048576 + 12345, id++ );
    AddFile( SpacesFile, 100, id++ );
    AddFile( CString( "uni" CONFORMANCE_UTF8_U_UMLAUT "-" ) + CString( CONFORMANCE_UTF8_KATAKANA_FU ) + CString( CONFORMANCE_UTF8_KATAKANA_A ) + ".bin", 4097, id++ );

    AddDirectory( EmptyDir );
    AddDirectory( MutateDir );

    AddFile( "nested/top.bin", 10, id++ );
    AddFile( "nested/level1/mid.bin", 300, id++ );
    AddFile( "nested/level1/level2/leaf.bin", 70000, id++ );

    // equal content under three names, and one that differs
    const UInt32 dupId = id++;
    AddFile( CString( DupDir ) + "/same_a.bin", 5000, dupId );
    AddFile( CString( DupDir ) + "/same_b.bin", 5000, dupId );
    AddFile( CString( DupDir ) + "/other.bin", 5000, id++ );

    AddFile( CString( SpacesDir ) + "/inner file.txt", 42, id++ );
    AddFile( CString( "dir-" CONFORMANCE_UTF8_I_UMLAUT "-" ) + CString( CONFORMANCE_UTF8_KATAKANA_FU ) + "/" + CString( CONFORMANCE_UTF8_E_ACUTE ) + CString( "file-" ) + CString( CONFORMANCE_UTF8_CYRILLIC_FA ) + ".bin", 777, id++ );

    // no file name in here contains the pattern of another one, so a pattern matches the same names whether it is
    // matched on the whole name or as a part of it
    AddFile( CString( FilterDir ) + "/alpha.txt", 20, id++ );
    AddFile( CString( FilterDir ) + "/beta.txt", 21, id++ );
    AddFile( CString( FilterDir ) + "/gamma.dat", 22, id++ );
    AddFile( CString( FilterDir ) + "/delta.dat", 23, id++ );
    AddFile( CString( FilterDir ) + "/noext", 24, id++ );

    for ( UInt32 i=0; i<ManyFilesCount; ++i )
    {
        CString number = CORE::UInt32ToString( i );
        while ( number.Length() < 4 )
            number = "0" + number;
        AddFile( CString( ManyFilesDir ) + "/file_" + number + ".bin", 16 + ( i % 32 ), id++ );
    }

    AddFile( DeepFile, 1234, id++ );

    if ( includeLargeFile )
    {
        AddFile( LargeFile, ( (UInt64) 1 << 32 ) + 4096, id++, true );
    }

    std::sort( m_entries.begin(), m_entries.end(), EntryPathLess );
}

/*-------------------------------------------------------------------------*/

void
CArchiveConformanceTree::AddDirectory( const CString& path )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return;
    if ( GUCEF_NULL != FindEntry( path ) )
        return;

    AddDirectory( ParentPath( path ) );

    SEntry entry;
    entry.path = path;
    entry.isDirectory = true;
    m_entries.push_back( entry );
}

/*-------------------------------------------------------------------------*/

void
CArchiveConformanceTree::AddFile( const CString& path, UInt64 size, UInt32 contentId, bool isZeroFilled )
{GUCEF_TRACE;

    AddDirectory( ParentPath( path ) );

    SEntry entry;
    entry.path = path;
    entry.isDirectory = false;
    entry.size = size;
    entry.contentId = contentId;
    entry.isZeroFilled = isZeroFilled;
    m_entries.push_back( entry );
}

/*-------------------------------------------------------------------------*/

const CArchiveConformanceTree::TEntryVector&
CArchiveConformanceTree::GetEntries( void ) const
{GUCEF_TRACE;

    return m_entries;
}

/*-------------------------------------------------------------------------*/

const CArchiveConformanceTree::SEntry*
CArchiveConformanceTree::FindEntry( const CString& path ) const
{GUCEF_TRACE;

    for ( TEntryVector::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        if ( (*i).path == path )
            return &(*i);
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CArchiveConformanceTree::HasLargeFile( void ) const
{GUCEF_TRACE;

    return m_hasLargeFile;
}

/*-------------------------------------------------------------------------*/

void
CArchiveConformanceTree::GetDescendantPaths( const CString& dirPath ,
                                             bool recursive         ,
                                             bool wantFiles         ,
                                             bool wantDirs          ,
                                             TStringVector& paths   ) const
{GUCEF_TRACE;

    const CString prefix = dirPath.IsNULLOrEmpty() ? CString::Empty : dirPath + "/";
    for ( TEntryVector::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
    {
        const SEntry& entry = (*i);
        if ( entry.isDirectory ? !wantDirs : !wantFiles )
            continue;
        if ( entry.path.Length() <= prefix.Length() || ( !prefix.IsNULLOrEmpty() && 0 != entry.path.HasSubstr( prefix, true ) ) )
            continue;
        if ( !recursive && entry.path.SubstrToIndex( prefix.Length(), false ).HasChar( '/' ) >= 0 )
            continue;
        paths.push_back( entry.path );
    }
    std::sort( paths.begin(), paths.end() );
}

/*-------------------------------------------------------------------------*/

CString
CArchiveConformanceTree::LeafName( const CString& path )
{GUCEF_TRACE;

    const Int32 slash = path.HasChar( '/', false );
    if ( slash < 0 )
        return path;
    return path.SubstrToIndex( (UInt32) slash + 1, false );
}

/*-------------------------------------------------------------------------*/

CString
CArchiveConformanceTree::ParentPath( const CString& path )
{GUCEF_TRACE;

    const Int32 slash = path.HasChar( '/', false );
    if ( slash < 0 )
        return CString::Empty;
    return path.SubstrToIndex( (UInt32) slash, true );
}

/*-------------------------------------------------------------------------*/

UInt8
CArchiveConformanceTree::ContentByte( UInt32 contentId, UInt64 position )
{GUCEF_TRACE;

    return (UInt8) ( MixWord( contentId, position >> 3 ) >> ( ( position & 7 ) * 8 ) );
}

/*-------------------------------------------------------------------------*/

void
CArchiveConformanceTree::FillContent( UInt32 contentId, UInt64 offset, void* dest, UInt32 count )
{GUCEF_TRACE;

    UInt8* out = static_cast< UInt8* >( dest );
    UInt64 currentWordIndex = 0;
    UInt64 currentWord = 0;
    bool haveWord = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt64 position = offset + i;
        const UInt64 wordIndex = position >> 3;
        if ( !haveWord || wordIndex != currentWordIndex )
        {
            currentWord = MixWord( contentId, wordIndex );
            currentWordIndex = wordIndex;
            haveWord = true;
        }
        out[ i ] = (UInt8) ( currentWord >> ( ( position & 7 ) * 8 ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CArchiveConformanceTree::CheckContent( UInt32 contentId, UInt64 offset, const void* data, UInt32 count, UInt64* firstMismatch )
{GUCEF_TRACE;

    const UInt8* in = static_cast< const UInt8* >( data );
    UInt64 currentWordIndex = 0;
    UInt64 currentWord = 0;
    bool haveWord = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt64 position = offset + i;
        const UInt64 wordIndex = position >> 3;
        if ( !haveWord || wordIndex != currentWordIndex )
        {
            currentWord = MixWord( contentId, wordIndex );
            currentWordIndex = wordIndex;
            haveWord = true;
        }
        if ( in[ i ] != (UInt8) ( currentWord >> ( ( position & 7 ) * 8 ) ) )
        {
            if ( GUCEF_NULL != firstMismatch )
                *firstMismatch = position;
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CArchiveConformanceTree::FillEntryContent( const SEntry& entry, UInt64 offset, void* dest, UInt32 count )
{GUCEF_TRACE;

    if ( entry.isZeroFilled )
        memset( dest, 0, count );
    else
        FillContent( entry.contentId, offset, dest, count );
}

/*-------------------------------------------------------------------------*/

bool
CArchiveConformanceTree::CheckEntryContent( const SEntry& entry, UInt64 offset, const void* data, UInt32 count, UInt64* firstMismatch )
{GUCEF_TRACE;

    if ( entry.isZeroFilled )
    {
        const UInt8* in = static_cast< const UInt8* >( data );
        for ( UInt32 i=0; i<count; ++i )
        {
            if ( 0 != in[ i ] )
            {
                if ( GUCEF_NULL != firstMismatch )
                    *firstMismatch = offset + i;
                return false;
            }
        }
        return true;
    }
    return CheckContent( entry.contentId, offset, data, count, firstMismatch );
}

/*-------------------------------------------------------------------------*/
