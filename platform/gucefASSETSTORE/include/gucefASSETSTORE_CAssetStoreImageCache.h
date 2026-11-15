/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREIMAGECACHE_H
#define GUCEF_ASSETSTORE_CASSETSTOREIMAGECACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */
#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */
#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAssetStoreImageCacheTaskConsumer;

/**
 *  The listing pictures of the items of the stores (the thumbnail addresses of CAssetStoreItem): fetched with a plain
 *  HTTP or HTTPS GET, without sign in, decoded with the image codecs of gucefIMAGE (the codec plugins are loaded by the
 *  application), scaled down and kept as 8 bit RGBA pixel maps.
 *
 *  The pictures are kept in memory by address and size, bounded by a number of pictures and a number of pixel bytes: the
 *  least recently used picture goes first. Optionally the downloaded bytes are kept as files in a folder of the VFS
 *  (SetCacheFolder()), named by the SHA-256 of the address and the extension of the address (".bin" without one), and
 *  read back before the network is used; a file that cannot be decoded is ignored and replaced by a new download.
 *
 *  TryGetImage() answers from memory only: it never waits and never touches the network or the disk. RequestImage()
 *  queues a fetch as a task on a thread pool of the task manager (SetThreadPoolName(), "AssetStoreImages" by default), at
 *  most GetMaxConcurrentFetches() at once. A picture that is requested again while it is pending is fetched once; the
 *  address is also downloaded and decoded once for the sizes that are requested of it. The download, the decode and the
 *  scaling happen on the pool thread, only the small result is kept. The result is raised as ImageReadyEvent on the
 *  thread of the pulse generator of the cache (the global one by default), never from the pool thread itself; the
 *  picture is in memory by then. Requesting a picture that is in memory raises the event at once (also from the pulse
 *  generator thread).
 *
 *  The event data is a TImageReadyEventData (a CORE::CTCloneableObj of SImageReadyData, read with a static_cast): the address,
 *  the size asked for, the pixels (null when the picture failed) and the status and message of a failure. A picture that
 *  failed is not remembered. Pictures that are not a plain 200 answer with an image, that are larger than
 *  GetMaxDownloadBytes() or that no codec decodes, fail with ASSETSTORE_STATUS_NOT_FOUND, _HTTP_ERROR, _FAILED
 *  or _UNSUPPORTED.
 *
 *  Thread safe. Destroying the cache waits for the fetches that run.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreImageCache : public CORE::CTSGNotifier
{
    public:

    /** The result of a request */
    struct SImageReadyData
    {
        CORE::CString url;
        UInt32 maxSize;                     /**< the size asked for: the most pixels on the longest side, 0 for the picture as it is */
        IMAGE::TPixelMapPtr pixels;         /**< 8 bit RGBA; null when the picture failed */
        TAssetStoreStatus status;
        CORE::CString message;              /**< what went wrong, empty on success */

        SImageReadyData( void );
    };
    typedef CORE::CTCloneableObj< SImageReadyData > TImageReadyEventData;

    static const CORE::CEvent ImageReadyEvent;              /**< TImageReadyEventData */

    static const CORE::CString DefaultThreadPoolName;       /**< "AssetStoreImages" */

    static const UInt32 DefaultMaxConcurrentFetches = 4;
    static const UInt32 DefaultMaxMemoryEntries = 512;
    static const UInt32 DefaultTimeoutInMs = 20000;
    static const UInt32 DefaultMaxDownloadBytes = 16 * 1024 * 1024;
    static const UInt64 DefaultMaxMemoryBytes = 96 * 1024 * 1024;

    static void RegisterEvents( void );

    /** The event is raised on the thread of the global pulse generator */
    CAssetStoreImageCache( void );

    /** The event is raised on the thread of the given pulse generator */
    explicit CAssetStoreImageCache( CORE::PulseGeneratorPtr pulseGenerator );

    virtual ~CAssetStoreImageCache() GUCEF_VIRTUAL_OVERRIDE;

    /** The thread pool of the fetches; takes effect for fetches that start after it */
    void SetThreadPoolName( const CORE::CString& threadPoolName );
    CORE::CString GetThreadPoolName( void ) const;

    /** The most fetches that run at once; 0 gives the default */
    void SetMaxConcurrentFetches( UInt32 maxFetches );
    UInt32 GetMaxConcurrentFetches( void ) const;

    /** The bounds of the pictures in memory: their number and their pixel bytes; 0 gives the default */
    void SetMemoryLimits( UInt32 maxEntries, UInt64 maxBytes );
    UInt32 GetMaxMemoryEntries( void ) const;
    UInt64 GetMaxMemoryBytes( void ) const;

    /** The time a download may take, connecting included; 0 gives the default */
    void SetTimeoutInMs( UInt32 timeoutInMs );
    UInt32 GetTimeoutInMs( void ) const;

    /** The largest download that is accepted, in bytes; 0 gives the default */
    void SetMaxDownloadBytes( UInt32 maxBytes );
    UInt32 GetMaxDownloadBytes( void ) const;

    /** The VFS folder of the downloaded bytes; empty for no disk cache (the default). The folder is made when it is first written to */
    void SetCacheFolder( const CORE::CString& vfsPath );
    CORE::CString GetCacheFolder( void ) const;

    /** The name of the file of an address in the folder of the disk cache: the SHA-256 of the address, then its extension or ".bin" */
    static CORE::CString MakeCacheFileName( const CORE::CString& url );

    /** A picture that is in memory, at once; it counts as the most recently used. False when it is not there */
    bool TryGetImage( const CORE::CString& url, UInt32 maxSize, IMAGE::TPixelMapPtr& pixels );

    /** Queues the fetch of a picture at the size; ImageReadyEvent follows when it is done, also for a failure */
    void RequestImage( const CORE::CString& url, UInt32 maxSize );

    /**
     *  Takes the request back: no event follows. A request that was not started is dropped; a fetch that runs finishes
     *  and its picture is kept in memory without an event
     */
    void CancelRequest( const CORE::CString& url, UInt32 maxSize );

    /** The number of addresses that are queued or being fetched */
    UInt32 GetPendingCount( void ) const;

    /** Drops the pictures in memory and the requests that were not started; fetches that run finish. The files of the disk cache stay */
    void Clear( void );

    /** The number of pictures and of pixel bytes in memory */
    UInt32 GetMemoryEntryCount( void ) const;
    UInt64 GetMemoryBytes( void ) const;

    /** Requests to the network, pictures found on the disk, pictures found in memory (TryGetImage() and RequestImage()) */
    UInt32 GetNetworkRequestCount( void ) const;
    UInt32 GetDiskHitCount( void ) const;
    UInt32 GetMemoryHitCount( void ) const;
    void ResetCounters( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CAssetStoreImageCacheTaskConsumer;

    /** What is asked of one address: the sizes, and for each whether the result is announced */
    struct SPending
    {
        GUCEF::map< UInt32, bool > sizes;
        bool running;
        UInt64 sequence;

        SPending( void );
    };
    typedef GUCEF::map< CORE::CString, SPending > TPendingMap;

    struct SMemoryEntry
    {
        IMAGE::TPixelMapPtr pixels;
        UInt64 bytes;
        UInt64 lastUse;

        SMemoryEntry( void );
    };
    typedef GUCEF::map< CORE::CString, SMemoryEntry > TMemoryMap;

    void Initialize( void );
    static CORE::CString MakeMemoryKey( const CORE::CString& url, UInt32 maxSize );
    void Schedule( void );
    void StartFetches( const CORE::CString::StringVector& urls );
    bool LaunchTask( const CORE::CString& url );
    void RunFetch( const CORE::CString& url );
    void OnFetchAborted( const CORE::CString& url );
    void FinishPending( const CORE::CString& url, const IMAGE::TPixelMapPtr& decoded, TAssetStoreStatus status, const CORE::CString& message );
    void PickStartableLocked( CORE::CString::StringVector& urls );
    void Announce( const CORE::CString& url, UInt32 maxSize, const IMAGE::TPixelMapPtr& pixels, TAssetStoreStatus status, const CORE::CString& message );
    void StoreInMemory( const CORE::CString& url, UInt32 maxSize, const IMAGE::TPixelMapPtr& pixels );
    void EvictLocked( void );
    TAssetStoreStatus Download( const CORE::CString& url, CORE::CDynamicBuffer& bytes, CORE::CString& message );
    bool ReadFromDisk( const CORE::CString& url, CORE::CDynamicBuffer& bytes ) const;
    void WriteToDisk( const CORE::CString& url, const CORE::CDynamicBuffer& bytes ) const;
    static bool Decode( const CORE::CDynamicBuffer& bytes, const CORE::CString& url, IMAGE::TPixelMapPtr& pixels );
    static bool ScaleToRgba8( const IMAGE::TPixelMapPtr& source, UInt32 maxSize, IMAGE::TPixelMapPtr& result );

    CAssetStoreImageCache( const CAssetStoreImageCache& src );              /**< not implemented */
    CAssetStoreImageCache& operator=( const CAssetStoreImageCache& src );   /**< not implemented */

    private:

    CORE::CString m_threadPoolName;
    UInt32 m_maxConcurrentFetches;
    UInt32 m_maxMemoryEntries;
    UInt64 m_maxMemoryBytes;
    UInt32 m_timeoutInMs;
    UInt32 m_maxDownloadBytes;
    CORE::CString m_cacheFolder;
    TPendingMap m_pending;
    UInt64 m_nextSequence;
    UInt32 m_activeFetches;
    bool m_shuttingDown;
    TMemoryMap m_memory;
    UInt64 m_memoryBytes;
    UInt64 m_useCounter;
    UInt32 m_networkRequests;
    UInt32 m_diskHits;
    UInt32 m_memoryHits;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREIMAGECACHE_H ? */
