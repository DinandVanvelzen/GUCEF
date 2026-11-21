/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_VFS_TESTAPP_TESTVFSBLOCKCACHEUTILS_H
#define GUCEF_VFS_TESTAPP_TESTVFSBLOCKCACHEUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_CVFSBLOCKCACHE_H
#include "gucefVFS_CVfsBlockCache.h"
#define GUCEF_VFS_CVFSBLOCKCACHE_H
#endif /* GUCEF_VFS_CVFSBLOCKCACHE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A folder for a test of the block cache, below GUCEF_TEST_SCRATCH_DIR when that is set and below the temporary directory
 *  otherwise. It is emptied when it is made and removed again when the object goes.
 */
class CBlockCacheTestFolder
{
    public:

    explicit CBlockCacheTestFolder( const char* name );

    ~CBlockCacheTestFolder();

    const GUCEF::CORE::CString& GetPath( void ) const;

    /** A path below the folder */
    GUCEF::CORE::CString GetChildPath( const GUCEF::CORE::CString& relativePath ) const;

    /** Removes everything in the folder and makes it again */
    void Reset( void );

    private:

    CBlockCacheTestFolder( const CBlockCacheTestFolder& src );
    CBlockCacheTestFolder& operator=( const CBlockCacheTestFolder& src );

    GUCEF::CORE::CString m_path;
};

/*-------------------------------------------------------------------------*/

namespace BlockCacheTest {

/** The size of a block in the tests: the smallest the cache allows */
static const GUCEF::UInt32 BlockBytes = 65536;

/** Bytes that depend on a seed and an offset in the file, so that a block of one file never equals a block of another or of another place */
void
FillPattern( GUCEF::UInt32 seed, GUCEF::UInt64 offset, void* dest, GUCEF::UInt32 length );

/** Stores the blocks 'firstBlock' to 'firstBlock' + 'count' - 1 of the pattern of 'seed' */
bool
StoreBlocks( GUCEF::VFS::CVfsBlockCache& cache, GUCEF::VFS::CVfsBlockCache::TEntryHandle entry, GUCEF::UInt32 seed, GUCEF::UInt32 firstBlock, GUCEF::UInt32 count );

/** Stores every block of the file */
bool
StoreAllBlocks( GUCEF::VFS::CVfsBlockCache& cache, GUCEF::VFS::CVfsBlockCache::TEntryHandle entry, GUCEF::UInt32 seed );

/** Whether a read of the whole block is a hit with the bytes of the pattern */
bool
BlockMatches( GUCEF::VFS::CVfsBlockCache& cache, GUCEF::VFS::CVfsBlockCache::TEntryHandle entry, GUCEF::UInt32 seed, GUCEF::UInt32 index );

/** The result of a read of the whole block, whatever the bytes */
GUCEF::VFS::CVfsBlockCache::EReadResult
ReadWholeBlock( GUCEF::VFS::CVfsBlockCache& cache, GUCEF::VFS::CVfsBlockCache::TEntryHandle entry, GUCEF::UInt32 index );

/** The digest of the whole pattern file of a size */
GUCEF::VFS::CVfsDigest
PatternDigest( GUCEF::VFS::CVfsDigest::EAlgorithm algorithm, GUCEF::UInt32 seed, GUCEF::UInt64 fileSize );

/** The paths of the files with an extension (".vbi", ".vbd") in the fan out folders of a cache folder */
GUCEF::CORE::CString::StringVector
ListEntryFiles( const GUCEF::CORE::CString& cacheFolder, const char* extension );

/** Changes one byte of a file (adds one to it); false when the file is shorter */
bool
DamageByte( const GUCEF::CORE::CString& path, GUCEF::UInt64 offset );

/** The content of a file, empty when it cannot be read */
GUCEF::CORE::CDynamicBuffer
ReadWholeFile( const GUCEF::CORE::CString& path );

/** Writes a file with the content */
bool
WriteWholeFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CDynamicBuffer& content );

/** The number of file descriptors the process has open (Linux: /proc/self/fd), 0 where that is not known */
GUCEF::UInt32
CountOpenDescriptors( void );

/** Whether the process runs with the rights of a super user (the permission cases mean nothing then) */
bool
IsSuperUser( void );

/** Whether a file system keeps a file sparse: a byte written at 1 GiB in a new file in the folder takes far less than 1 GiB (Linux: stat; elsewhere false, it was not looked at) */
bool
IsFolderSparse( const GUCEF::CORE::CString& folder );

}; /* namespace BlockCacheTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTVFSBLOCKCACHEUTILS_H ? */
