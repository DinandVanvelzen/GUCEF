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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefVFS_CVfsBlockRunFetcher.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsBlockRunFetcher::SRun::SRun( void )
    : firstByte( 0 )
    , data()
    , blocksStored( 0 )
    , bytesStored( 0 )
    , blocksNotKept( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsBlockRunFetcher::FetchRun( CVfsBlockCache* cache, CVfsBlockCache::TEntryHandle entry, UInt32 blockBytes, UInt64 fileSize, CIVfsBlockSource& source,
                               UInt32 firstBlock, UInt32 blockCount, CVfsRemoteCounters* counters, SRun& run )
{GUCEF_TRACE;

    run.blocksStored = 0;
    run.bytesStored = 0;
    run.blocksNotKept = 0;

    const UInt64 start = (UInt64) firstBlock * blockBytes;
    if ( 0 == blockCount || start >= fileSize )
        return CVfsCallResult();

    UInt64 end = start + (UInt64) blockCount * blockBytes;
    if ( end > fileSize )
        end = fileSize;
    const UInt32 length = (UInt32) ( end - start );

    run.firstByte = start;
    run.data.SetDataSize( length );

    UInt32 bytesRead = 0;
    if ( GUCEF_NULL != counters )
        counters->fetchCalls.FetchAdd( 1 );
    CVfsCallResult result = source.FetchRange( start, length, run.data.GetBufferPtr(), bytesRead );
    if ( bytesRead > length )
        bytesRead = length;
    run.data.SetDataSize( bytesRead );
    if ( GUCEF_NULL != counters )
        counters->bytesFetched.FetchAdd( bytesRead );

    if ( result.IsSuccess() && bytesRead != length )
        result = CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_PROTOCOL_ERROR, "the source delivered fewer bytes than it was asked for and did not say why" );

    // every block that arrived whole and is not held yet is kept: the blocks before a break stay
    if ( 0 != entry && GUCEF_NULL != cache )
    {
        const UInt32 firstIndex = firstBlock;
        const UInt32 wholeBlocks = (UInt32) ( ( (UInt64) bytesRead + blockBytes - 1 ) / blockBytes );
        for ( UInt32 n=0; n<wholeBlocks; ++n )
        {
            const UInt32 index = firstIndex + n;
            const UInt32 size = cache->GetBlockSize( entry, index );
            const UInt64 offsetInRun = (UInt64) n * blockBytes;
            if ( 0 == size || offsetInRun + size > bytesRead )
                break;                              // a block that did not arrive whole
            if ( cache->HasBlock( entry, index ) )
                continue;
            if ( cache->StoreBlock( entry, index, static_cast< const UInt8* >( run.data.GetConstBufferPtr() ) + offsetInRun, size ) )
            {
                ++run.blocksStored;
                run.bytesStored += size;
            }
            else
            {
                ++run.blocksNotKept;
            }
        }
    }
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
