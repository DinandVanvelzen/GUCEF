/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CVFSBLOCKRUNFETCHER_H
#define GUCEF_VFS_CVFSBLOCKRUNFETCHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSBLOCKCACHE_H
#include "gucefVFS_CVfsBlockCache.h"
#define GUCEF_VFS_CVFSBLOCKCACHE_H
#endif /* GUCEF_VFS_CVFSBLOCKCACHE_H ? */

#ifndef GUCEF_VFS_CIVFSBLOCKSOURCE_H
#include "gucefVFS_CIVfsBlockSource.h"
#define GUCEF_VFS_CIVFSBLOCKSOURCE_H
#endif /* GUCEF_VFS_CIVFSBLOCKSOURCE_H ? */

#ifndef GUCEF_VFS_CVFSREMOTECOUNTERS_H
#include "gucefVFS_CVfsRemoteCounters.h"
#define GUCEF_VFS_CVFSREMOTECOUNTERS_H
#endif /* GUCEF_VFS_CVFSREMOTECOUNTERS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Fetches a run of blocks of a file from a source into the entry of the block cache: the one place where bytes of a
 *  miss are asked of a remote place, whether a reading thread waits for them or a background job does the work.
 *
 *  One call of the source covers the whole run. Every block that arrived complete and is not held yet is stored; a run that
 *  ended early (the link dropped) keeps the complete blocks before the break and drops the partial one. The bytes of the
 *  run are given back as they arrived, so a caller can serve them even when the cache could not keep a block.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsBlockRunFetcher
{
    public:

    struct GUCEF_VFS_PUBLIC_CPP SRun
    {
        UInt64 firstByte;                   /**< where the bytes of 'data' start in the file */
        CORE::CDynamicBuffer data;          /**< the bytes that arrived: complete blocks and possibly a partial one after them */
        UInt32 blocksStored;                /**< blocks that were not held and are now */
        UInt64 bytesStored;                 /**< their bytes */
        UInt32 blocksNotKept;               /**< complete blocks that the cache could not keep */
        SRun( void );
    };

    /**
     *  @param cache        the cache of the entry; may be NULL when there is no entry
     *  @param entry        the entry of the file at the revision of the source; 0 for none: nothing is stored, 'run.data' has the bytes
     *  @param blockBytes   the size of a block (that of the cache)
     *  @param fileSize     the size of the file
     *  @param source       the bytes come from here
     *  @param firstBlock   the first block of the run
     *  @param blockCount   the blocks of the run (past the end of the file they are left off)
     *  @param counters     counts the fetch and the bytes; may be NULL
     */
    static CVfsCallResult FetchRun( CVfsBlockCache* cache, CVfsBlockCache::TEntryHandle entry, UInt32 blockBytes, UInt64 fileSize, CIVfsBlockSource& source,
                                    UInt32 firstBlock, UInt32 blockCount, CVfsRemoteCounters* counters, SRun& run );

    private:

    CVfsBlockRunFetcher( void );            /**< not implemented: only static members */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSBLOCKRUNFETCHER_H ? */
