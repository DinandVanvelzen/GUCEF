/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNRESOURCEIOACCESS_H
#define GUCEF_DRN_CDRNRESOURCEIOACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHANDLE_H
#include "gucefDRN_CDRNResourceHandle.h"
#define GUCEF_DRN_CDRNRESOURCEHANDLE_H
#endif /* GUCEF_DRN_CDRNRESOURCEHANDLE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A resource of a member read as a CORE::CIOAccess, through an open handle (CDRNResourceHandle): any code that takes a
 *  CIOAccess (a VFS resource, a decoder, CIOAccess::Write( CIOAccess& )) reads the remote resource with Read, Seek,
 *  Setpos, Tell, GetChar, Eof and GetSize. Read only: writes do nothing and IsWriteable() is false.
 *
 *  Semantics follow the f-functions as CFileAccess does: Seek() and Setpos() give 0 on success and may go beyond the
 *  end, Read() gives whole elements and stops at the end, Eof() is true at or beyond the end. Reads fetch whole chunks
 *  (DRN_RESOURCE_CHUNK_SIZE), checked against the hash tree when the options of the handle ask for it, into a small
 *  cache of recent chunks (cacheChunks); a read that continues where the last one ended also asks for the chunks after
 *  it (readAheadChunks). A read blocks the calling thread (see CDRNResourceHandle); a failed read gives 0 elements and
 *  GetLastResult() tells why.
 *
 *  Open() opens the handle when it is not open yet (waiting up to the open timeout of its options), Close() lets go of
 *  the access's reference to the handle; the handle closes when the last reference goes.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceIOAccess : public CORE::CIOAccess
{
    public:

    static const UInt32 DefaultCacheChunks = 8;
    static const UInt32 DefaultReadAheadChunks = 4;

    explicit CDRNResourceIOAccess( const CDRNResourceHandlePtr& handle                ,
                                   UInt32 cacheChunks = DefaultCacheChunks            ,
                                   UInt32 readAheadChunks = DefaultReadAheadChunks    );

    virtual ~CDRNResourceIOAccess() GUCEF_VIRTUAL_OVERRIDE;

    const CDRNResourceHandlePtr& GetHandle( void ) const;

    /** What the last failed operation came to, OK when none failed */
    const CDRNResourceTransferResult& GetLastResult( void ) const;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    using CORE::CIOAccess::Read;
    using CORE::CIOAccess::Write;

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE;
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    /** Another access on the same handle, at the same position, with a cache of its own */
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Chunks fetched over the link, and reads answered from the cache alone */
    UInt64 GetChunksFetched( void ) const;
    UInt64 GetCacheHits( void ) const;

    private:

    CDRNResourceIOAccess( const CDRNResourceIOAccess& src );               /**< not implemented */
    CDRNResourceIOAccess& operator=( const CDRNResourceIOAccess& src );    /**< not implemented */

    typedef GUCEF::map< UInt64, CORE::CDynamicBuffer > TChunkCache;
    typedef GUCEF::vector< UInt64 > TChunkOrder;

    const CORE::CDynamicBuffer* GetChunk( UInt64 chunk, bool sequential );
    void Remember( UInt64 chunk, const CORE::CDynamicBuffer& data );

    CDRNResourceHandlePtr m_handle;
    UInt32 m_cacheChunks;
    UInt32 m_readAheadChunks;
    bool m_opened;
    UInt64 m_size;
    UInt64 m_position;
    UInt64 m_lastReadEnd;
    TChunkCache m_cache;
    TChunkOrder m_order;                /**< the chunks of the cache, least recently used first */
    CDRNResourceTransferResult m_lastResult;
    UInt64 m_chunksFetched;
    UInt64 m_cacheHits;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCEIOACCESS_H ? */
