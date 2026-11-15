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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */
#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */
#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */
#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */
#ifndef GUCEF_WEB_CHTTPCLIENTREQUESTOPTIONS_H
#include "gucefWEB_CHttpClientRequestOptions.h"
#define GUCEF_WEB_CHTTPCLIENTREQUESTOPTIONS_H
#endif /* GUCEF_WEB_CHTTPCLIENTREQUESTOPTIONS_H ? */
#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */
#ifndef GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#include "gucefIMAGE_CPixelMapResampler.h"
#define GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#endif /* GUCEF_IMAGE_CPIXELMAPRESAMPLER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#include "gucefASSETSTORE_CAssetStoreImageCache.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAssetStoreImageCache::ImageReadyEvent = "GUCEF::ASSETSTORE::CAssetStoreImageCache::ImageReadyEvent";
const CORE::CString CAssetStoreImageCache::DefaultThreadPoolName = "AssetStoreImages";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The task data of the task that fetches one address for a CAssetStoreImageCache */
class CAssetStoreImageCacheTaskData : public CORE::CICloneable
{
    public:

    CAssetStoreImageCache* cache;
    CORE::CString url;

    CAssetStoreImageCacheTaskData( void )
        : CORE::CICloneable()
        , cache( GUCEF_NULL )
        , url()
    {GUCEF_TRACE;

    }

    virtual ~CAssetStoreImageCacheTaskData() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW CAssetStoreImageCacheTaskData( *this );
    }

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return CloneableTypeName();
    }

    static const char* CloneableTypeName( void )
    {GUCEF_TRACE;

        return "GUCEF::ASSETSTORE::CAssetStoreImageCacheTaskData";
    }
};

/*-------------------------------------------------------------------------*/

/** The consumer of the pool task that fetches, decodes and scales the picture of one address: the address is the task data */
class CAssetStoreImageCacheTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CAssetStoreImageCacheTaskConsumer( void )
        : CORE::CTaskConsumer()
    {GUCEF_TRACE;

    }

    virtual ~CAssetStoreImageCacheTaskConsumer() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return TaskType;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreImageCacheTaskConsumer";
        return classTypeName;
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( task.IsNULL() )
            return true;
        CAssetStoreImageCacheTaskData* data = const_cast< CAssetStoreImageCacheTaskData* >( CORE::CICloneable::CloneableAs< CAssetStoreImageCacheTaskData >( task->GetTaskData() ) );
        if ( GUCEF_NULL == data || GUCEF_NULL == data->cache )
            return true;
        CAssetStoreImageCache* cache = data->cache;
        data->cache = GUCEF_NULL;       // the cache is told once that the fetch ended
        cache->RunFetch( data->url );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // a task that ended without running its fetch still lets the cache go on
        if ( !task.IsNULL() )
        {
            CAssetStoreImageCacheTaskData* data = const_cast< CAssetStoreImageCacheTaskData* >( CORE::CICloneable::CloneableAs< CAssetStoreImageCacheTaskData >( task->GetTaskData() ) );
            if ( GUCEF_NULL != data && GUCEF_NULL != data->cache )
            {
                CAssetStoreImageCache* cache = data->cache;
                data->cache = GUCEF_NULL;
                cache->OnFetchAborted( data->url );
            }
        }
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }
};

const CORE::CString CAssetStoreImageCacheTaskConsumer::TaskType = "GUCEF::ASSETSTORE::CAssetStoreImageCacheTaskConsumer";

typedef CORE::CTFactory< CORE::CTaskConsumer, CAssetStoreImageCacheTaskConsumer, MT::CMutex > TAssetStoreImageCacheTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The task type is registered while at least one cache exists */
MT::CMutex g_taskRegistrationLock;
UInt32 g_taskRegistrationCount = 0;
TAssetStoreImageCacheTaskConsumerFactory* g_taskFactory = GUCEF_NULL;

void
AcquireTaskType( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_taskRegistrationLock );
    if ( 0 == g_taskRegistrationCount )
    {
        g_taskFactory = GUCEF_NEW TAssetStoreImageCacheTaskConsumerFactory();
        CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CAssetStoreImageCacheTaskConsumer::TaskType, g_taskFactory );
    }
    ++g_taskRegistrationCount;
}

void
ReleaseTaskType( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_taskRegistrationLock );
    if ( g_taskRegistrationCount > 0 )
        --g_taskRegistrationCount;
    if ( 0 == g_taskRegistrationCount && GUCEF_NULL != g_taskFactory )
    {
        CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CAssetStoreImageCacheTaskConsumer::TaskType );
        GUCEF_DELETE g_taskFactory;
        g_taskFactory = GUCEF_NULL;
    }
}

/** The bytes of the text without the terminating zero, for every string format of CString */
inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

/** The extension of the file name in the path of an address, lower case and without the dot; empty when there is none */
CORE::CString
ExtensionOfUrl( const CORE::CString& url )
{GUCEF_TRACE;

    const char* chars = url.C_String();
    const UInt32 length = ByteLengthOf( url );
    UInt32 pathStart = 0;
    for ( UInt32 i=0; i+2<length; ++i )
    {
        if ( ':' == chars[ i ] && '/' == chars[ i+1 ] && '/' == chars[ i+2 ] )
        {
            pathStart = i + 3;
            break;
        }
    }
    // the path starts at the first separator after the host
    while ( pathStart < length && '/' != chars[ pathStart ] )
        ++pathStart;
    UInt32 pathEnd = pathStart;
    while ( pathEnd < length && '?' != chars[ pathEnd ] && '#' != chars[ pathEnd ] )
        ++pathEnd;

    UInt32 nameStart = pathStart;
    for ( UInt32 i=pathStart; i<pathEnd; ++i )
        if ( '/' == chars[ i ] )
            nameStart = i + 1;
    Int32 dot = -1;
    for ( UInt32 i=nameStart; i<pathEnd; ++i )
        if ( '.' == chars[ i ] )
            dot = (Int32) i;
    if ( dot < 0 || (UInt32) dot + 1 >= pathEnd || pathEnd - (UInt32) dot - 1 > 5 )
        return CORE::CString::Empty;

    GUCEF::vector< char > extension;
    for ( UInt32 i=(UInt32) dot+1; i<pathEnd; ++i )
    {
        char c = chars[ i ];
        if ( c >= 'A' && c <= 'Z' )
            c = (char) ( c - 'A' + 'a' );
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) ) )
            return CORE::CString::Empty;
        extension.push_back( c );
    }
    return CORE::CString( &extension[ 0 ], (UInt32) extension.size() );
}

/** The type of an encoded picture by its first bytes; empty when they say nothing */
CORE::CString
SniffImageType( const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    if ( size >= 8 && 0x89 == data[ 0 ] && 'P' == data[ 1 ] && 'N' == data[ 2 ] && 'G' == data[ 3 ] )
        return "png";
    if ( size >= 3 && 0xFF == data[ 0 ] && 0xD8 == data[ 1 ] && 0xFF == data[ 2 ] )
        return "jpg";
    if ( size >= 6 && 'G' == data[ 0 ] && 'I' == data[ 1 ] && 'F' == data[ 2 ] && '8' == data[ 3 ] )
        return "gif";
    if ( size >= 14 && 'B' == data[ 0 ] && 'M' == data[ 1 ] )
        return "bmp";
    if ( size >= 12 && 'R' == data[ 0 ] && 'I' == data[ 1 ] && 'F' == data[ 2 ] && 'F' == data[ 3 ] && 'W' == data[ 8 ] && 'E' == data[ 9 ] && 'B' == data[ 10 ] && 'P' == data[ 11 ] )
        return "webp";
    if ( size >= 4 && 'D' == data[ 0 ] && 'D' == data[ 1 ] && 'S' == data[ 2 ] && ' ' == data[ 3 ] )
        return "dds";
    if ( size >= 4 && ( ( 'I' == data[ 0 ] && 'I' == data[ 1 ] && 42 == data[ 2 ] && 0 == data[ 3 ] ) || ( 'M' == data[ 0 ] && 'M' == data[ 1 ] && 0 == data[ 2 ] && 42 == data[ 3 ] ) ) )
        return "tif";
    return CORE::CString::Empty;
}

bool
IsHttpUrl( const CORE::CString& url )
{GUCEF_TRACE;

    return 0 == url.HasSubstr( "http://", false ) || 0 == url.HasSubstr( "https://", false );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreImageCache::SImageReadyData::SImageReadyData( void )
    : url()
    , maxSize( 0 )
    , pixels()
    , status( ASSETSTORE_STATUS_OK )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreImageCache::SPending::SPending( void )
    : sizes()
    , running( false )
    , sequence( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreImageCache::SMemoryEntry::SMemoryEntry( void )
    : pixels()
    , bytes( 0 )
    , lastUse( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::RegisterEvents( void )
{GUCEF_TRACE;

    ImageReadyEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetStoreImageCache::CAssetStoreImageCache( void )
    : CORE::CTSGNotifier()
    , m_threadPoolName( DefaultThreadPoolName )
    , m_maxConcurrentFetches( DefaultMaxConcurrentFetches )
    , m_maxMemoryEntries( DefaultMaxMemoryEntries )
    , m_maxMemoryBytes( DefaultMaxMemoryBytes )
    , m_timeoutInMs( DefaultTimeoutInMs )
    , m_maxDownloadBytes( DefaultMaxDownloadBytes )
    , m_cacheFolder()
    , m_pending()
    , m_nextSequence( 1 )
    , m_activeFetches( 0 )
    , m_shuttingDown( false )
    , m_memory()
    , m_memoryBytes( 0 )
    , m_useCounter( 0 )
    , m_networkRequests( 0 )
    , m_diskHits( 0 )
    , m_memoryHits( 0 )
    , m_lock()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetStoreImageCache::CAssetStoreImageCache( CORE::PulseGeneratorPtr pulseGenerator )
    : CORE::CTSGNotifier( pulseGenerator )
    , m_threadPoolName( DefaultThreadPoolName )
    , m_maxConcurrentFetches( DefaultMaxConcurrentFetches )
    , m_maxMemoryEntries( DefaultMaxMemoryEntries )
    , m_maxMemoryBytes( DefaultMaxMemoryBytes )
    , m_timeoutInMs( DefaultTimeoutInMs )
    , m_maxDownloadBytes( DefaultMaxDownloadBytes )
    , m_cacheFolder()
    , m_pending()
    , m_nextSequence( 1 )
    , m_activeFetches( 0 )
    , m_shuttingDown( false )
    , m_memory()
    , m_memoryBytes( 0 )
    , m_useCounter( 0 )
    , m_networkRequests( 0 )
    , m_diskHits( 0 )
    , m_memoryHits( 0 )
    , m_lock()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::Initialize( void )
{GUCEF_TRACE;

    RegisterEvents();
    AcquireTaskType();
}

/*-------------------------------------------------------------------------*/

CAssetStoreImageCache::~CAssetStoreImageCache()
{GUCEF_TRACE;

    // requests that did not start are dropped, the fetches that run end first: they raise their events through this object
    {
        MT::CScopeMutex lock( m_lock );
        m_shuttingDown = true;
        TPendingMap::iterator i = m_pending.begin();
        while ( i != m_pending.end() )
        {
            if ( !(*i).second.running )
                m_pending.erase( i++ );
            else
                ++i;
        }
    }
    for ( ;; )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( 0 == m_activeFetches )
                break;
        }
        MT::ThreadDelay( 2 );
    }
    SignalUpcomingDestruction();
    ReleaseTaskType();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::SetThreadPoolName( const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_threadPoolName = threadPoolName.IsNULLOrEmpty() ? DefaultThreadPoolName : threadPoolName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreImageCache::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::SetMaxConcurrentFetches( UInt32 maxFetches )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_maxConcurrentFetches = 0 == maxFetches ? (UInt32) DefaultMaxConcurrentFetches : maxFetches;
    }
    Schedule();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreImageCache::GetMaxConcurrentFetches( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxConcurrentFetches;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::SetMemoryLimits( UInt32 maxEntries, UInt64 maxBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxMemoryEntries = 0 == maxEntries ? (UInt32) DefaultMaxMemoryEntries : maxEntries;
    m_maxMemoryBytes = 0 == maxBytes ? (UInt64) DefaultMaxMemoryBytes : maxBytes;
    EvictLocked();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreImageCache::GetMaxMemoryEntries( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxMemoryEntries;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreImageCache::GetMaxMemoryBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxMemoryBytes;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::SetTimeoutInMs( UInt32 timeoutInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_timeoutInMs = 0 == timeoutInMs ? (UInt32) DefaultTimeoutInMs : timeoutInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreImageCache::GetTimeoutInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_timeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::SetMaxDownloadBytes( UInt32 maxBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxDownloadBytes = 0 == maxBytes ? (UInt32) DefaultMaxDownloadBytes : maxBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreImageCache::GetMaxDownloadBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxDownloadBytes;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::SetCacheFolder( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_cacheFolder = vfsPath;
    while ( m_cacheFolder.Length() > 0 && '/' == m_cacheFolder[ m_cacheFolder.Length()-1 ] )
        m_cacheFolder = m_cacheFolder.CutChars( 1, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreImageCache::GetCacheFolder( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_cacheFolder;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreImageCache::MakeCacheFileName( const CORE::CString& url )
{GUCEF_TRACE;

    CAssetDigest digest( CAssetDigest::Sha256 );
    digest.Update( url.C_String(), ByteLengthOf( url ) );
    const CORE::CString extension = ExtensionOfUrl( url );
    return digest.FinalHex() + "." + ( extension.IsNULLOrEmpty() ? CORE::CString( "bin" ) : extension );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreImageCache::MakeMemoryKey( const CORE::CString& url, UInt32 maxSize )
{GUCEF_TRACE;

    return CORE::ToString( maxSize ) + "|" + url;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreImageCache::TryGetImage( const CORE::CString& url, UInt32 maxSize, IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    const CORE::CString key = MakeMemoryKey( url, maxSize );
    MT::CScopeMutex lock( m_lock );
    TMemoryMap::iterator i = m_memory.find( key );
    if ( i == m_memory.end() )
        return false;
    (*i).second.lastUse = ++m_useCounter;
    ++m_memoryHits;
    pixels = (*i).second.pixels;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::RequestImage( const CORE::CString& url, UInt32 maxSize )
{GUCEF_TRACE;

    if ( url.IsNULLOrEmpty() )
        return;

    // a picture that is in memory is announced right away
    IMAGE::TPixelMapPtr cached;
    if ( TryGetImage( url, maxSize, cached ) )
    {
        Announce( url, maxSize, cached, ASSETSTORE_STATUS_OK, CORE::CString::Empty );
        return;
    }

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_shuttingDown )
            return;
        TPendingMap::iterator i = m_pending.find( url );
        if ( i != m_pending.end() )
        {
            (*i).second.sizes[ maxSize ] = true;
            return;
        }
        SPending pending;
        pending.sizes[ maxSize ] = true;
        pending.sequence = m_nextSequence++;
        m_pending[ url ] = pending;
    }
    Schedule();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::CancelRequest( const CORE::CString& url, UInt32 maxSize )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TPendingMap::iterator i = m_pending.find( url );
    if ( i == m_pending.end() )
        return;
    SPending& pending = (*i).second;
    GUCEF::map< UInt32, bool >::iterator s = pending.sizes.find( maxSize );
    if ( s == pending.sizes.end() )
        return;
    if ( pending.running )
    {
        // the fetch is under way: its picture is kept, nobody is told
        (*s).second = false;
        return;
    }
    pending.sizes.erase( s );
    if ( pending.sizes.empty() )
        m_pending.erase( i );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreImageCache::GetPendingCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_pending.size();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_memory.clear();
    m_memoryBytes = 0;
    TPendingMap::iterator i = m_pending.begin();
    while ( i != m_pending.end() )
    {
        if ( !(*i).second.running )
            m_pending.erase( i++ );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreImageCache::GetMemoryEntryCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_memory.size();
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreImageCache::GetMemoryBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_memoryBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreImageCache::GetNetworkRequestCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_networkRequests;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreImageCache::GetDiskHitCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_diskHits;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreImageCache::GetMemoryHitCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_memoryHits;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::ResetCounters( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_networkRequests = 0;
    m_diskHits = 0;
    m_memoryHits = 0;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::PickStartableLocked( CORE::CString::StringVector& urls )
{GUCEF_TRACE;

    while ( !m_shuttingDown && m_activeFetches < m_maxConcurrentFetches )
    {
        TPendingMap::iterator oldest = m_pending.end();
        TPendingMap::iterator i = m_pending.begin();
        while ( i != m_pending.end() )
        {
            if ( !(*i).second.running && ( oldest == m_pending.end() || (*i).second.sequence < (*oldest).second.sequence ) )
                oldest = i;
            ++i;
        }
        if ( oldest == m_pending.end() )
            break;
        (*oldest).second.running = true;
        ++m_activeFetches;
        urls.push_back( (*oldest).first );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::Schedule( void )
{GUCEF_TRACE;

    CORE::CString::StringVector urls;
    {
        MT::CScopeMutex lock( m_lock );
        PickStartableLocked( urls );
    }
    StartFetches( urls );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::StartFetches( const CORE::CString::StringVector& urls )
{GUCEF_TRACE;

    for ( size_t u=0; u<urls.size(); ++u )
    {
        if ( !LaunchTask( urls[ u ] ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreImageCache: The fetch of " << urls[ u ] << " could not be queued" );
            FinishPending( urls[ u ], IMAGE::TPixelMapPtr(), ASSETSTORE_STATUS_FAILED, "the fetch could not be queued" );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreImageCache::LaunchTask( const CORE::CString& url )
{GUCEF_TRACE;

    CORE::CString poolName;
    UInt32 workers = 0;
    {
        MT::CScopeMutex lock( m_lock );
        poolName = m_threadPoolName;
        workers = m_maxConcurrentFetches;
    }
    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( poolName );
    if ( threadPool.IsNULL() )
        return false;

    // a pool has one worker unless asked for more: the fetches run side by side
    if ( threadPool->GetDesiredMinNrOfWorkerThreads() < workers )
        threadPool->SetDesiredMinNrOfWorkerThreads( workers );

    CAssetStoreImageCacheTaskData* data = GUCEF_NEW CAssetStoreImageCacheTaskData();
    data->cache = this;
    data->url = url;
    CORE::CFutureResult queued = threadPool->QueueTask( CAssetStoreImageCacheTaskConsumer::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
    {
        GUCEF_DELETE data;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::OnFetchAborted( const CORE::CString& url )
{GUCEF_TRACE;

    FinishPending( url, IMAGE::TPixelMapPtr(), ASSETSTORE_STATUS_CANCELLED, "the fetch did not run" );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::RunFetch( const CORE::CString& url )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    IMAGE::TPixelMapPtr decoded;
    TAssetStoreStatus status = ASSETSTORE_STATUS_OK;
    CORE::CString message;
    try
    {
        bool found = false;
        if ( ReadFromDisk( url, bytes ) )
        {
            if ( Decode( bytes, url, decoded ) )
            {
                MT::CScopeMutex lock( m_lock );
                ++m_diskHits;
                found = true;
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreImageCache: The cached file of " << url << " cannot be decoded and is replaced" );
            }
        }
        if ( !found )
        {
            decoded.Unlink();
            bytes.Clear();
            status = Download( url, bytes, message );
            if ( ASSETSTORE_STATUS_OK == status )
            {
                if ( Decode( bytes, url, decoded ) )
                {
                    WriteToDisk( url, bytes );
                }
                else
                {
                    status = ASSETSTORE_STATUS_UNSUPPORTED;
                    message = "the picture cannot be decoded";
                }
            }
        }
    }
    catch ( ... )
    {
        decoded.Unlink();
        status = ASSETSTORE_STATUS_FAILED;
        message = "the fetch ended with an exception";
    }
    bytes.Clear();

    if ( ASSETSTORE_STATUS_OK != status )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreImageCache: The picture of " << url << " failed with " << AssetStoreStatusToString( status ) << ": " << message );
    }
    FinishPending( url, decoded, status, message );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::FinishPending( const CORE::CString& url                  ,
                                      const IMAGE::TPixelMapPtr& decoded        ,
                                      TAssetStoreStatus status                  ,
                                      const CORE::CString& message              )
{GUCEF_TRACE;

    // the sizes that were asked for while the fetch ran are served too; the entry goes when none is left
    GUCEF::set< UInt32 > served;
    CORE::CString::StringVector next;
    for ( ;; )
    {
        GUCEF::vector< std::pair< UInt32, bool > > todo;
        {
            MT::CScopeMutex lock( m_lock );
            TPendingMap::iterator p = m_pending.find( url );
            if ( p != m_pending.end() )
            {
                GUCEF::map< UInt32, bool >::const_iterator s = (*p).second.sizes.begin();
                while ( s != (*p).second.sizes.end() )
                {
                    if ( served.end() == served.find( (*s).first ) )
                        todo.push_back( std::pair< UInt32, bool >( (*s).first, (*s).second ) );
                    ++s;
                }
            }
            if ( todo.empty() )
            {
                if ( p != m_pending.end() )
                    m_pending.erase( p );
                if ( m_activeFetches > 0 )
                    --m_activeFetches;
                PickStartableLocked( next );
                break;
            }
        }

        for ( size_t t=0; t<todo.size(); ++t )
        {
            const UInt32 maxSize = todo[ t ].first;
            served.insert( maxSize );
            IMAGE::TPixelMapPtr scaled;
            TAssetStoreStatus sizeStatus = status;
            CORE::CString sizeMessage = message;
            if ( ASSETSTORE_STATUS_OK == status )
            {
                bool done = false;
                try
                {
                    done = ScaleToRgba8( decoded, maxSize, scaled );
                }
                catch ( ... )
                {
                    done = false;
                }
                if ( done )
                {
                    StoreInMemory( url, maxSize, scaled );
                }
                else
                {
                    scaled.Unlink();
                    sizeStatus = ASSETSTORE_STATUS_UNSUPPORTED;
                    sizeMessage = "the picture cannot be scaled";
                }
            }
            if ( todo[ t ].second )
                Announce( url, maxSize, scaled, sizeStatus, sizeMessage );
        }
    }

    // nothing of this object is used after the last fetch ended unless another fetch was picked: it keeps the object alive
    if ( !next.empty() )
        StartFetches( next );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::Announce( const CORE::CString& url, UInt32 maxSize, const IMAGE::TPixelMapPtr& pixels, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    SImageReadyData data;
    data.url = url;
    data.maxSize = maxSize;
    data.pixels = pixels;
    data.status = status;
    data.message = message;
    TImageReadyEventData eventData( data );
    NotifyObserversFromThread( ImageReadyEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::StoreInMemory( const CORE::CString& url, UInt32 maxSize, const IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( pixels.IsNULL() )
        return;
    const CORE::CString key = MakeMemoryKey( url, maxSize );
    MT::CScopeMutex lock( m_lock );
    TMemoryMap::iterator i = m_memory.find( key );
    if ( i != m_memory.end() )
    {
        m_memoryBytes -= (*i).second.bytes;
        m_memory.erase( i );
    }
    SMemoryEntry entry;
    entry.pixels = pixels;
    entry.bytes = pixels->GetTotalSizeInBytes();
    entry.lastUse = ++m_useCounter;
    m_memoryBytes += entry.bytes;
    m_memory[ key ] = entry;
    EvictLocked();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::EvictLocked( void )
{GUCEF_TRACE;

    // the least recently used goes first; the newest picture stays even when it alone is over the bound
    while ( m_memory.size() > 1 && ( m_memory.size() > m_maxMemoryEntries || m_memoryBytes > m_maxMemoryBytes ) )
    {
        TMemoryMap::iterator oldest = m_memory.begin();
        TMemoryMap::iterator i = m_memory.begin();
        while ( i != m_memory.end() )
        {
            if ( (*i).second.lastUse < (*oldest).second.lastUse )
                oldest = i;
            ++i;
        }
        m_memoryBytes -= (*oldest).second.bytes;
        m_memory.erase( oldest );
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreImageCache::Download( const CORE::CString& url, CORE::CDynamicBuffer& bytes, CORE::CString& message )
{GUCEF_TRACE;

    if ( !IsHttpUrl( url ) )
    {
        message = "the address is not an http or https address";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    WEB::CHttpRequestData request;
    if ( !request.SetUrl( url ) )
    {
        message = "the address cannot be used";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    request.requestType = "GET";

    UInt32 timeoutInMs = 0;
    UInt32 maxBytes = 0;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_networkRequests;
        timeoutInMs = m_timeoutInMs;
        maxBytes = m_maxDownloadBytes;
    }

    WEB::CHttpClientRequestOptions options;
    options.connectTimeoutInMs = timeoutInMs;
    options.totalTimeoutInMs = timeoutInMs;
    options.followRedirects = true;
    options.maxRedirects = 5;
    options.maxResponseBodySize = maxBytes;

    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo info;
    WEB::CHTTPClient client;
    client.Execute( request, options, response, &info );

    if ( info.responseBodyTooLarge )
    {
        message = "the picture is larger than " + CORE::ToString( maxBytes ) + " bytes";
        return ASSETSTORE_STATUS_FAILED;
    }
    if ( !info.success )
    {
        message = "no response: " + CORE::CString( WEB::HttpTransferErrorClassToString( info.errorClass ) );
        return ASSETSTORE_STATUS_NETWORK_ERROR;
    }
    const int code = response.statusCode;
    if ( 200 != code )
    {
        message = "the server answered HTTP " + CORE::ToString( code );
        return 404 == code ? ASSETSTORE_STATUS_NOT_FOUND : ASSETSTORE_STATUS_HTTP_ERROR;
    }
    if ( 0 == response.content.GetDataSize() )
    {
        message = "the server answered without a picture";
        return ASSETSTORE_STATUS_FAILED;
    }
    bytes = response.content;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreImageCache::ReadFromDisk( const CORE::CString& url, CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    CORE::CString folder;
    {
        MT::CScopeMutex lock( m_lock );
        folder = m_cacheFolder;
    }
    if ( folder.IsNULLOrEmpty() )
        return false;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString path = folder + "/" + MakeCacheFileName( url );
    if ( !vfs.FileExists( path ) )
        return false;
    bytes.Clear();
    return vfs.LoadFile( bytes, path, "rb" ) && bytes.GetDataSize() > 0;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreImageCache::WriteToDisk( const CORE::CString& url, const CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    CORE::CString folder;
    {
        MT::CScopeMutex lock( m_lock );
        folder = m_cacheFolder;
    }
    if ( folder.IsNULLOrEmpty() )
        return;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( folder ) )
        vfs.CreateDir( folder );
    const CORE::CString path = folder + "/" + MakeCacheFileName( url );
    if ( !vfs.StoreAsFile( path, bytes, 0, true ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreImageCache: The picture of " << url << " could not be stored in " << path );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreImageCache::Decode( const CORE::CDynamicBuffer& bytes, const CORE::CString& url, IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( 0 == bytes.GetDataSize() )
        return false;

    // the type by the bytes first, then by the address
    CORE::CString::StringVector types;
    const CORE::CString sniffed = SniffImageType( bytes.AsConstTypePtr< UInt8 >(), bytes.GetDataSize() );
    if ( !sniffed.IsNULLOrEmpty() )
    {
        types.push_back( sniffed );
        if ( "jpg" == sniffed )
            types.push_back( "jpeg" );
        if ( "tif" == sniffed )
            types.push_back( "tiff" );
    }
    const CORE::CString extension = ExtensionOfUrl( url );
    if ( !extension.IsNULLOrEmpty() && "bin" != extension )
    {
        bool known = false;
        for ( size_t t=0; t<types.size(); ++t )
            known = known || types[ t ] == extension;
        if ( !known )
            types.push_back( extension );
    }

    for ( size_t t=0; t<types.size(); ++t )
    {
        try
        {
            CORE::CDynamicBufferAccess access( bytes );
            IMAGE::CImage image;
            if ( image.Load( access, types[ t ], true, false ) && image.GetFrameCount() > 0 )
            {
                IMAGE::TPixelMapPtr map;
                if ( image.TryGetPixelMap( 0, 0, map ) && !map.IsNULL() && map->GetWidthInPixels() > 0 && map->GetHeightInPixels() > 0 )
                {
                    pixels = map;
                    return true;
                }
            }
        }
        catch ( ... )
        {
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreImageCache::ScaleToRgba8( const IMAGE::TPixelMapPtr& source, UInt32 maxSize, IMAGE::TPixelMapPtr& result )
{GUCEF_TRACE;

    if ( source.IsNULL() )
        return false;
    const UInt32 width = source->GetWidthInPixels();
    const UInt32 height = source->GetHeightInPixels();
    if ( 0 == width || 0 == height )
        return false;

    IMAGE::TPixelMapPtr work = source;
    if ( IMAGE::PSF_PALETTE_INDICES == work->GetPixelStorageFormat() )
    {
        IMAGE::TPixelMapPtr converted;
        if ( !work->ConvertFormatTo( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, converted ) || converted.IsNULL() )
            return false;
        work = converted;
    }

    // the longest side gets maxSize, the other keeps the aspect ratio; pictures are not enlarged
    const UInt32 longest = width > height ? width : height;
    if ( maxSize > 0 && longest > maxSize )
    {
        UInt32 targetWidth = (UInt32) ( ( (UInt64) width * maxSize + longest / 2 ) / longest );
        UInt32 targetHeight = (UInt32) ( ( (UInt64) height * maxSize + longest / 2 ) / longest );
        if ( targetWidth < 1 )
            targetWidth = 1;
        if ( targetHeight < 1 )
            targetHeight = 1;
        IMAGE::TPixelMapPtr scaled;
        if ( !IMAGE::CPixelMapResampler::Resample( *work, targetWidth, targetHeight, IMAGE::RESAMPLEFILTER_TRIANGLE, IMAGE::COLOURSPACE_SRGB, IMAGE::WRAPMODE_CLAMP, scaled ) || scaled.IsNULL() )
            return false;
        work = scaled;
    }

    if ( IMAGE::PSF_RGBA != work->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != work->GetPixelComponentDataType() )
    {
        IMAGE::TPixelMapPtr converted;
        if ( !work->ConvertFormatTo( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, converted ) || converted.IsNULL() )
            return false;
        work = converted;
    }
    else if ( work.GetPointerAlways() == source.GetPointerAlways() )
    {
        // the picture is shared between the sizes of a request: every size keeps a map of its own
        work = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( *source ) );
    }
    result = work;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreImageCache::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreImageCache";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
