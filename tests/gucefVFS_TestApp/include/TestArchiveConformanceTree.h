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

#ifndef GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCETREE_H
#define GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCETREE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The reference tree of the archive conformance suite, and the generator of the content of its files.
 *
 *  The fixture of a backend puts exactly this tree in its archive, the suite checks the archive against it: both sides
 *  take the names, sizes and content from here, so the expectations are the same for every backend. Paths are relative
 *  to the root of the archive, use '/' as separator and have no leading or trailing separator.
 *
 *  The tree holds: nested directories, an empty directory, an empty file, files of 1 byte, 64 KiB - 1, 64 KiB and
 *  64 KiB + 1, a file of a few MiB, names with spaces and with non-ASCII characters (UTF-8), a directory of several
 *  hundred files (to see paging of a list), a deep path, files with equal content and files to filter on. Optionally a
 *  file above 4 GiB that is zero filled, which a local file system can hold as a sparse file.
 *
 *  The content of a file is a function of its content id and the position in it only, so a read at any offset can be
 *  checked without the file at hand. Files that share a content id have equal content.
 */
class CArchiveConformanceTree
{
    public:

    struct SEntry
    {
        GUCEF::CORE::CString path;      /**< relative to the archive root */
        bool isDirectory;
        GUCEF::UInt64 size;             /**< in bytes, zero for a directory */
        GUCEF::UInt32 contentId;        /**< seeds the content, see FillContent() */
        bool isZeroFilled;              /**< the content is all zeros, whatever the content id */

        SEntry( void );
    };

    typedef GUCEF::vector< SEntry >                 TEntryVector;
    typedef GUCEF::CORE::CString::StringVector    TStringVector;

    /** Paths used by name in the cases of the suite */
    static const char* const EmptyDir;              /**< a directory without content */
    static const char* const EmptyFile;             /**< a file of 0 bytes */
    static const char* const OneByteFile;           /**< a file of 1 byte */
    static const char* const BigFile;               /**< a file of a few MiB */
    static const char* const SpacesFile;            /**< a name with spaces */
    static const char* const SpacesDir;             /**< a directory name with spaces */
    static const char* const FilterDir;             /**< files to filter on: alpha.txt, beta.txt, gamma.dat, delta.dat, noext */
    static const char* const ManyFilesDir;          /**< a directory with several hundred files */
    static const char* const DeepFile;              /**< a file below sixteen levels of directories */
    static const char* const DupDir;                /**< same_a.bin and same_b.bin with equal content, other.bin with other content */
    static const char* const MutateDir;             /**< empty in the tree: the change detection cases make and remove files here */
    static const char* const LargeFile;             /**< above 4 GiB, only in a tree made with the large file */

    static const GUCEF::UInt32 ManyFilesCount;      /**< the number of files in ManyFilesDir */

    /**
     *  @param includeLargeFile whether the file above 4 GiB (LargeFile, in a directory of its own) is part of the tree
     */
    explicit CArchiveConformanceTree( bool includeLargeFile );

    const TEntryVector& GetEntries( void ) const;

    /** The entry of the path (a directory without a trailing separator) or GUCEF_NULL when the tree has none */
    const SEntry* FindEntry( const GUCEF::CORE::CString& path ) const;

    /** Whether the tree holds the file above 4 GiB */
    bool HasLargeFile( void ) const;

    /**
     *  The paths of the entries below a directory, relative to the archive root, sorted. A directory is listed
     *  without a trailing separator.
     *  @param dirPath the directory, empty for the root
     *  @param recursive false: the direct children only, true: every depth
     */
    void GetDescendantPaths( const GUCEF::CORE::CString& dirPath ,
                             bool recursive                      ,
                             bool wantFiles                      ,
                             bool wantDirs                       ,
                             TStringVector& paths                ) const;

    /** The last segment of a path */
    static GUCEF::CORE::CString LeafName( const GUCEF::CORE::CString& path );

    /** The path without its last segment, empty for a path of one segment */
    static GUCEF::CORE::CString ParentPath( const GUCEF::CORE::CString& path );

    /** The byte of the content with the given id at the given position */
    static GUCEF::UInt8 ContentByte( GUCEF::UInt32 contentId, GUCEF::UInt64 position );

    /** Writes the 'count' bytes of the content with the given id that start at the position 'offset' */
    static void FillContent( GUCEF::UInt32 contentId, GUCEF::UInt64 offset, void* dest, GUCEF::UInt32 count );

    /**
     *  Checks that the 'count' bytes at 'data' are the content with the given id that starts at the position 'offset'
     *  @param firstMismatch when given: set to the position of the first byte that differs
     */
    static bool CheckContent( GUCEF::UInt32 contentId, GUCEF::UInt64 offset, const void* data, GUCEF::UInt32 count, GUCEF::UInt64* firstMismatch = GUCEF_NULL );

    /** FillContent() for an entry: zeros for a zero filled one */
    static void FillEntryContent( const SEntry& entry, GUCEF::UInt64 offset, void* dest, GUCEF::UInt32 count );

    /** CheckContent() for an entry: zeros for a zero filled one */
    static bool CheckEntryContent( const SEntry& entry, GUCEF::UInt64 offset, const void* data, GUCEF::UInt32 count, GUCEF::UInt64* firstMismatch = GUCEF_NULL );

    private:

    void AddDirectory( const GUCEF::CORE::CString& path );

    void AddFile( const GUCEF::CORE::CString& path, GUCEF::UInt64 size, GUCEF::UInt32 contentId, bool isZeroFilled = false );

    TEntryVector m_entries;
    bool m_hasLargeFile;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCETREE_H ? */
