/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */
#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */
#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */
#ifndef GUCEF_IMAGE_CIIMAGECODEC_H
#include "gucefIMAGE_CIImageCodec.h"
#define GUCEF_IMAGE_CIIMAGECODEC_H
#endif /* GUCEF_IMAGE_CIIMAGECODEC_H ? */
#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */
#ifndef GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#include "gucefIMAGE_CImageCodecRegistry.h"
#define GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGECODECREGISTRY_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREIMAGECACHE_H
#include "gucefASSETSTORE_CAssetStoreImageCache.h"
#define GUCEF_ASSETSTORE_CASSETSTOREIMAGECACHE_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREIMAGECACHE_H ? */

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */
#ifndef TESTFAKESTORESERVER_H
#include "TestFakeStoreServer.h"
#define TESTFAKESTORESERVER_H
#endif /* TESTFAKESTORESERVER_H ? */
#include "TestImageCache.h"

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;
using namespace ASSETSTORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CAssetStoreImageCache::SImageReadyData TReadyData;
typedef GUCEF::vector< TReadyData > TReadyVector;

/** Notes the pictures a cache announces and on which thread */
class CImageObserver : public CObserver
{
    public:

    TReadyVector events;
    bool allOnThisThread;

    CImageObserver( void )
        : CObserver()
        , events()
        , allOnThisThread( true )
        , m_thread( MT::GetCurrentThreadID() )
    {
    }

    virtual ~CImageObserver()
    {
        UnsubscribeAllFromObserver();
    }

    void Observe( CAssetStoreImageCache& cache )
    {
        TEventCallback onReady( this, &CImageObserver::OnImageReady );
        SubscribeTo( &cache, CAssetStoreImageCache::ImageReadyEvent, onReady );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString classTypeName = "ASSETSTORETEST::CImageObserver";
        return classTypeName;
    }

    /** The announcement of an address at a size, false when there is none */
    bool Find( const CString& url, UInt32 maxSize, TReadyData& found ) const
    {
        for ( size_t e=0; e<events.size(); ++e )
        {
            if ( events[ e ].url == url && events[ e ].maxSize == maxSize )
            {
                found = events[ e ];
                return true;
            }
        }
        return false;
    }

    private:

    typedef CTEventHandlerFunctor< CImageObserver > TEventCallback;

    void OnImageReady( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        allOnThisThread = allOnThisThread && m_thread == MT::GetCurrentThreadID();
        const CAssetStoreImageCache::TImageReadyEventData* data = static_cast< const CAssetStoreImageCache::TImageReadyEventData* >( eventData );
        if ( GUCEF_NULL != data )
            events.push_back( data->GetData() );
    }

    private:

    TThreadId m_thread;
};

/**
 *  A decoder of a plain picture format of the tests: "TPIX", the width and the height as 32 bit numbers, then the 8 bit RGBA
 *  pixels. The tests do not depend on an image codec plugin that decodes PNG, which the application loads.
 */
class CPlainPictureCodec : public IMAGE::CIImageCodec
{
    public:

    static const char* const TypeName;

    CPlainPictureCodec( void ) : IMAGE::CIImageCodec() {}
    virtual ~CPlainPictureCodec() GUCEF_VIRTUAL_OVERRIDE {}

    static void Make( UInt32 width, UInt32 height, UInt8 seed, CDynamicBuffer& content )
    {
        content.Clear();
        content.Append( "TPIX", 4 );
        content.Append( &width, 4 );
        content.Append( &height, 4 );
        GUCEF::vector< UInt8 > pixels( width * height * 4 );
        for ( UInt32 p=0; p<width*height; ++p )
        {
            pixels[ p*4 ] = (UInt8) ( ( p % width ) * 255 / ( width > 1 ? width - 1 : 1 ) );
            pixels[ p*4+1 ] = (UInt8) ( ( p / width ) * 255 / ( height > 1 ? height - 1 : 1 ) );
            pixels[ p*4+2 ] = seed;
            pixels[ p*4+3 ] = 255;
        }
        content.Append( &pixels[ 0 ], (UInt32) pixels.size() );
    }

    virtual bool Encode( const IMAGE::CImage& inputImage, CIOAccess& encodedOutput ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool Decode( CIOAccess& encodedInput, IMAGE::CImage& outputImage ) GUCEF_VIRTUAL_OVERRIDE
    {
        char magic[ 4 ];
        UInt32 size[ 2 ];
        if ( 1 != encodedInput.Read( magic, 4, 1 ) || 0 != memcmp( magic, "TPIX", 4 ) || 1 != encodedInput.Read( size, 8, 1 ) )
            return false;
        if ( 0 == size[ 0 ] || 0 == size[ 1 ] || size[ 0 ] > 8192 || size[ 1 ] > 8192 )
            return false;
        GUCEF::vector< UInt8 > pixels( size[ 0 ] * size[ 1 ] * 4 );
        if ( 1 != encodedInput.Read( &pixels[ 0 ], (UInt32) pixels.size(), 1 ) )
            return false;
        IMAGE::TPixelMapPtr map( GUCEF_NEW IMAGE::CPixelMap( &pixels[ 0 ], size[ 0 ], size[ 1 ], IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        outputImage.Assign( map );
        return true;
    }

    virtual bool DecodeMetaData( CIOAccess& encodedInput, CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Encode( CIOAccess& source, CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Decode( CIOAccess& source, CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_CODEC_CAPABILITY_DECODE; }
    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TypeName; }
    virtual CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "ImageCodec"; }
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CPlainPictureCodec( *this ); }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString classTypeName = "ASSETSTORETEST::CPlainPictureCodec";
        return classTypeName;
    }
};

const char* const CPlainPictureCodec::TypeName = "tpix";

/** The fake store with the pictures */
struct SImageWorld
{
    CFakeStoreServer server;

    SImageWorld( void ) : server() {}
};

CString
UrlOf( const SImageWorld& world, const CString& fileId )
{
    return world.server.GetBaseUrl() + "/files/" + fileId;
}

bool
AddPicture( SImageWorld& world, const CString& fileId, UInt32 width, UInt32 height )
{
    CDynamicBuffer picture;
    CPlainPictureCodec::Make( width, height, (UInt8) fileId.Length(), picture );
    world.server.SetFileContent( fileId, picture );
    return true;
}

/** Pulses until the observer has the number of announcements; true when it got there in time */
bool
WaitForImages( const CImageObserver& observer, size_t count, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=2 )
    {
        CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        if ( observer.events.size() >= count )
            return true;
        MT::ThreadDelay( 2 );
    }
    return observer.events.size() >= count;
}

bool
IsRgba8( const IMAGE::TPixelMapPtr& pixels, UInt32 width, UInt32 height )
{
    return !pixels.IsNULL() && width == pixels->GetWidthInPixels() && height == pixels->GetHeightInPixels() &&
           IMAGE::PSF_RGBA == pixels->GetPixelStorageFormat() && GUCEF::DATATYPE_UINT8 == pixels->GetPixelComponentDataType() &&
           width * height * 4 == pixels->GetTotalSizeInBytes();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformImageCacheTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ImageCache" );

    // the decoder of the test pictures is a codec like the ones of the plugins
    IMAGE::CImageCodecRegistry& codecs = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry();
    CTSharedPtr< CPlainPictureCodec, MT::CMutex > plainCodec( GUCEF_NEW CPlainPictureCodec() );
    codecs.Register( CPlainPictureCodec::TypeName, plainCodec );

    SImageWorld world;
    const bool serverStarted = world.server.Start();
    const bool picturesMade = AddPicture( world, "img-big.tpix", 200, 100 ) && AddPicture( world, "img-small.tpix", 40, 30 ) && AddPicture( world, "img-tall.tpix", 30, 90 ) &&
                              AddPicture( world, "img-a.tpix", 16, 16 ) && AddPicture( world, "img-b.tpix", 16, 16 ) && AddPicture( world, "img-c.tpix", 16, 16 ) &&
                              AddPicture( world, "img-huge.tpix", 1400, 1000 );
    {
        CDynamicBuffer text;
        text.CopyFrom( CString( "this is a text and not a picture" ), 0, false );
        world.server.SetFileContent( "not-image", text );
    }
    MountFreshDirectory( "imagecache", "astest/imagecache" );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    GUCEF_TESTFW_TESTCASE( "Image cache fetch: the picture is downloaded, decoded, scaled to the size with its aspect and kept as 8 bit RGBA" )
        try
        {
            ASSERT_TRUE( serverStarted && picturesMade );
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            const CString url = UrlOf( world, "img-big.tpix" );

            IMAGE::TPixelMapPtr pixels;
            ASSERT_FALSE( cache.TryGetImage( url, 100, pixels ) );
            cache.RequestImage( url, 100 );
            ASSERT_TRUE( 1 == cache.GetPendingCount() );
            ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
            PumpFor( 30 );
            ASSERT_TRUE( 1 == observer.events.size() && observer.allOnThisThread );
            const TReadyData& ready = observer.events[ 0 ];
            ASSERT_TRUE( url == ready.url && 100 == ready.maxSize && ASSETSTORE_STATUS_OK == ready.status && ready.message.IsNULLOrEmpty() );
            ASSERT_TRUE( IsRgba8( ready.pixels, 100, 50 ) );
            ASSERT_TRUE( 0 == cache.GetPendingCount() && 1 == cache.GetNetworkRequestCount() && 0 == cache.GetDiskHitCount() );

            ASSERT_TRUE( cache.TryGetImage( url, 100, pixels ) && IsRgba8( pixels, 100, 50 ) );
            ASSERT_FALSE( cache.TryGetImage( url, 50, pixels ) );
            ASSERT_TRUE( 1 == cache.GetMemoryEntryCount() && 100 * 50 * 4 == cache.GetMemoryBytes() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache scaling: the longest side gets the size, pictures are not enlarged, 0 keeps the picture as it is" )
        try
        {
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            cache.RequestImage( UrlOf( world, "img-tall.tpix" ), 45 );
            cache.RequestImage( UrlOf( world, "img-small.tpix" ), 100 );
            cache.RequestImage( UrlOf( world, "img-small.tpix" ), 0 );
            cache.RequestImage( UrlOf( world, "img-big.tpix" ), 0 );
            cache.RequestImage( UrlOf( world, "img-big.tpix" ), 20 );
            ASSERT_TRUE( WaitForImages( observer, 5, 20000 ) );
            TReadyData found;
            ASSERT_TRUE( observer.Find( UrlOf( world, "img-tall.tpix" ), 45, found ) && IsRgba8( found.pixels, 15, 45 ) );
            ASSERT_TRUE( observer.Find( UrlOf( world, "img-small.tpix" ), 100, found ) && IsRgba8( found.pixels, 40, 30 ) );
            ASSERT_TRUE( observer.Find( UrlOf( world, "img-small.tpix" ), 0, found ) && IsRgba8( found.pixels, 40, 30 ) );
            ASSERT_TRUE( observer.Find( UrlOf( world, "img-big.tpix" ), 0, found ) && IsRgba8( found.pixels, 200, 100 ) );
            ASSERT_TRUE( observer.Find( UrlOf( world, "img-big.tpix" ), 20, found ) && IsRgba8( found.pixels, 20, 10 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache pending: an address that is requested again while it is pending is fetched once, for every size asked" )
        try
        {
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            const CString url = UrlOf( world, "img-huge.tpix" );
            cache.RequestImage( url, 64 );
            cache.RequestImage( url, 64 );
            cache.RequestImage( url, 32 );
            ASSERT_TRUE( 1 == cache.GetPendingCount() );
            ASSERT_TRUE( WaitForImages( observer, 2, 30000 ) );
            PumpFor( 50 );
            ASSERT_TRUE( 1 == cache.GetNetworkRequestCount() );
            TReadyData found;
            ASSERT_TRUE( observer.Find( url, 64, found ) && IsRgba8( found.pixels, 64, 46 ) );
            ASSERT_TRUE( observer.Find( url, 32, found ) && IsRgba8( found.pixels, 32, 23 ) );
            ASSERT_TRUE( 0 == cache.GetPendingCount() && 2 == cache.GetMemoryEntryCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache memory: a picture in memory is returned at once and announced again, without the network; Clear and the counters" )
        try
        {
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            const CString url = UrlOf( world, "img-small.tpix" );
            cache.RequestImage( url, 20 );
            ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
            ASSERT_TRUE( 1 == cache.GetNetworkRequestCount() && 0 == cache.GetMemoryHitCount() );

            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( cache.TryGetImage( url, 20, pixels ) && IsRgba8( pixels, 20, 15 ) );
            ASSERT_TRUE( 1 == cache.GetMemoryHitCount() );
            cache.RequestImage( url, 20 );
            ASSERT_TRUE( 0 == cache.GetPendingCount() && 2 == cache.GetMemoryHitCount() && 1 == cache.GetNetworkRequestCount() );
            ASSERT_TRUE( WaitForImages( observer, 2, 20000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == observer.events[ 1 ].status && IsRgba8( observer.events[ 1 ].pixels, 20, 15 ) && observer.allOnThisThread );

            cache.ResetCounters();
            ASSERT_TRUE( 0 == cache.GetNetworkRequestCount() && 0 == cache.GetMemoryHitCount() && 0 == cache.GetDiskHitCount() );
            cache.Clear();
            ASSERT_TRUE( 0 == cache.GetMemoryEntryCount() && 0 == cache.GetMemoryBytes() && !cache.TryGetImage( url, 20, pixels ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache disk: the downloaded bytes are kept as a file and read back, a file that cannot be decoded is replaced" )
        try
        {
            // the name of the file: the SHA-256 of the address, then its extension or .bin
            const CString named = "http://store.example/pictures/Cover.PNG?size=large";
            CAssetDigest digest( CAssetDigest::Sha256 );
            digest.Update( named.C_String(), named.Length() );
            const CString hash = digest.FinalHex();
            ASSERT_TRUE( 64 == hash.Length() && hash + ".png" == CAssetStoreImageCache::MakeCacheFileName( named ) );
            const CString plain = CAssetStoreImageCache::MakeCacheFileName( "http://store.example/pictures/cover" );
            ASSERT_TRUE( 68 == plain.Length() && ".bin" == plain.SubstrFromRange( 64, 68 ) );
            ASSERT_TRUE( CAssetStoreImageCache::MakeCacheFileName( "http://store.example/a.jpg" ) != CAssetStoreImageCache::MakeCacheFileName( "http://store.example/b.jpg" ) );
            ASSERT_TRUE( ".bin" == CAssetStoreImageCache::MakeCacheFileName( "http://store.example.com" ).SubstrFromRange( 64, 68 ) );

            const CString url = UrlOf( world, "img-big.tpix" );
            const CString folder = "astest/imagecache/pictures";
            const CString file = folder + "/" + CAssetStoreImageCache::MakeCacheFileName( url );
            {
                CAssetStoreImageCache cache;
                CImageObserver observer;
                observer.Observe( cache );
                cache.SetCacheFolder( folder + "/" );
                ASSERT_TRUE( folder == cache.GetCacheFolder() );
                ASSERT_FALSE( vfs.FileExists( file ) );
                cache.RequestImage( url, 50 );
                ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
                ASSERT_TRUE( 1 == cache.GetNetworkRequestCount() && 0 == cache.GetDiskHitCount() && vfs.FileExists( file ) );
            }
            {
                // another cache: the bytes come from the disk, also for another size
                CAssetStoreImageCache cache;
                CImageObserver observer;
                observer.Observe( cache );
                cache.SetCacheFolder( folder );
                cache.RequestImage( url, 80 );
                ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
                ASSERT_TRUE( 0 == cache.GetNetworkRequestCount() && 1 == cache.GetDiskHitCount() );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == observer.events[ 0 ].status && IsRgba8( observer.events[ 0 ].pixels, 80, 40 ) );
                // the disk is not asked for a picture that is in memory
                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( cache.TryGetImage( url, 80, pixels ) && 1 == cache.GetDiskHitCount() );
            }
            {
                // a file that is not a picture is ignored: the network is used and the file is replaced
                ASSERT_TRUE( WriteVfsFile( file, CString( "garbage that no codec reads" ) ) );
                CAssetStoreImageCache cache;
                CImageObserver observer;
                observer.Observe( cache );
                cache.SetCacheFolder( folder );
                cache.RequestImage( url, 80 );
                ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == observer.events[ 0 ].status && IsRgba8( observer.events[ 0 ].pixels, 80, 40 ) );
                ASSERT_TRUE( 1 == cache.GetNetworkRequestCount() && 0 == cache.GetDiskHitCount() );
            }
            {
                CAssetStoreImageCache cache;
                CImageObserver observer;
                observer.Observe( cache );
                cache.SetCacheFolder( folder );
                cache.RequestImage( url, 80 );
                ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
                ASSERT_TRUE( 0 == cache.GetNetworkRequestCount() && 1 == cache.GetDiskHitCount() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache bound: the least recently used pictures go first, by number and by pixel bytes" )
        try
        {
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            cache.SetMemoryLimits( 2, 0 );
            ASSERT_TRUE( 2 == cache.GetMaxMemoryEntries() && CAssetStoreImageCache::DefaultMaxMemoryBytes == cache.GetMaxMemoryBytes() );
            const CString a = UrlOf( world, "img-a.tpix" );
            const CString b = UrlOf( world, "img-b.tpix" );
            const CString c = UrlOf( world, "img-c.tpix" );
            cache.RequestImage( a, 0 );
            ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
            cache.RequestImage( b, 0 );
            ASSERT_TRUE( WaitForImages( observer, 2, 20000 ) );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( cache.TryGetImage( a, 0, pixels ) );        // a is used last, b is the one to go
            cache.RequestImage( c, 0 );
            ASSERT_TRUE( WaitForImages( observer, 3, 20000 ) );
            ASSERT_TRUE( 2 == cache.GetMemoryEntryCount() );
            ASSERT_TRUE( cache.TryGetImage( a, 0, pixels ) && cache.TryGetImage( c, 0, pixels ) && !cache.TryGetImage( b, 0, pixels ) );

            // 16 x 16 pixels are 1024 bytes: 2500 bytes hold two of them
            cache.Clear();
            cache.SetMemoryLimits( 100, 2500 );
            ASSERT_TRUE( 100 == cache.GetMaxMemoryEntries() && 2500 == cache.GetMaxMemoryBytes() );
            cache.RequestImage( a, 0 );
            ASSERT_TRUE( WaitForImages( observer, 4, 20000 ) );
            cache.RequestImage( b, 0 );
            ASSERT_TRUE( WaitForImages( observer, 5, 20000 ) );
            cache.RequestImage( c, 0 );
            ASSERT_TRUE( WaitForImages( observer, 6, 20000 ) );
            ASSERT_TRUE( 2 == cache.GetMemoryEntryCount() && 2048 == cache.GetMemoryBytes() );
            ASSERT_TRUE( !cache.TryGetImage( a, 0, pixels ) && cache.TryGetImage( b, 0, pixels ) && cache.TryGetImage( c, 0, pixels ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache failures: a missing picture, a body that is not a picture and an address that is not http fail with their event" )
        try
        {
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            const CString missing = UrlOf( world, "missing" );
            const CString text = UrlOf( world, "not-image" );
            const CString ftp = "ftp://store.example/picture.png";
            cache.RequestImage( missing, 64 );
            cache.RequestImage( text, 64 );
            cache.RequestImage( ftp, 64 );
            ASSERT_TRUE( WaitForImages( observer, 3, 20000 ) );
            PumpFor( 30 );
            ASSERT_TRUE( 3 == observer.events.size() );
            TReadyData found;
            ASSERT_TRUE( observer.Find( missing, 64, found ) && ASSETSTORE_STATUS_NOT_FOUND == found.status && found.pixels.IsNULL() && !found.message.IsNULLOrEmpty() );
            ASSERT_TRUE( observer.Find( text, 64, found ) && ASSETSTORE_STATUS_UNSUPPORTED == found.status && found.pixels.IsNULL() && !found.message.IsNULLOrEmpty() );
            ASSERT_TRUE( observer.Find( ftp, 64, found ) && ASSETSTORE_STATUS_INVALID_ARGUMENT == found.status && found.pixels.IsNULL() );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( !cache.TryGetImage( missing, 64, pixels ) && !cache.TryGetImage( text, 64, pixels ) && 0 == cache.GetMemoryEntryCount() );
            ASSERT_TRUE( 0 == cache.GetPendingCount() && 2 == cache.GetNetworkRequestCount() );

            // a failure is not remembered: the next request asks again
            cache.RequestImage( missing, 64 );
            ASSERT_TRUE( WaitForImages( observer, 4, 20000 ) );
            ASSERT_TRUE( 3 == cache.GetNetworkRequestCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache limits: a download over the size limit fails, a server that is out of reach fails the picture" )
        try
        {
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            cache.SetMaxDownloadBytes( 500 );
            ASSERT_TRUE( 500 == cache.GetMaxDownloadBytes() );
            const CString url = UrlOf( world, "img-huge.tpix" );
            cache.RequestImage( url, 64 );
            ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_FAILED == observer.events[ 0 ].status && observer.events[ 0 ].pixels.IsNULL() && observer.events[ 0 ].message.HasSubstr( "500" ) >= 0 );
            cache.SetMaxDownloadBytes( 0 );
            ASSERT_TRUE( 16 * 1024 * 1024 == cache.GetMaxDownloadBytes() );
            cache.RequestImage( url, 64 );
            ASSERT_TRUE( WaitForImages( observer, 2, 30000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == observer.events[ 1 ].status && IsRgba8( observer.events[ 1 ].pixels, 64, 46 ) );

            // nothing listens on the address of the stopped store
            const CString gone = UrlOf( world, "img-a.tpix" );
            world.server.Stop();
            cache.RequestImage( gone, 16 );
            ASSERT_TRUE( WaitForImages( observer, 3, 30000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NETWORK_ERROR == observer.events[ 2 ].status && observer.events[ 2 ].pixels.IsNULL() );
            ASSERT_TRUE( world.server.Start() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache cancel: a request that did not start is dropped, one that runs finishes into memory without an event" )
        try
        {
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            cache.SetMaxConcurrentFetches( 1 );
            ASSERT_TRUE( 1 == cache.GetMaxConcurrentFetches() );
            const CString running = UrlOf( world, "img-huge.tpix" );
            const CString queued = UrlOf( world, "img-a.tpix" );
            const CString kept = UrlOf( world, "img-b.tpix" );
            cache.RequestImage( running, 64 );
            cache.RequestImage( queued, 16 );
            cache.RequestImage( kept, 16 );
            ASSERT_TRUE( 3 == cache.GetPendingCount() );
            cache.CancelRequest( queued, 16 );
            cache.CancelRequest( queued, 99 );
            cache.CancelRequest( UrlOf( world, "nothing" ), 16 );
            ASSERT_TRUE( 2 == cache.GetPendingCount() );
            cache.CancelRequest( running, 64 );

            for ( UInt32 waited=0; waited<30000 && cache.GetPendingCount() > 0; waited+=2 )
            {
                PumpFor( 2 );
            }
            PumpFor( 100 );
            ASSERT_TRUE( 0 == cache.GetPendingCount() );
            ASSERT_TRUE( 1 == observer.events.size() && kept == observer.events[ 0 ].url );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( cache.TryGetImage( running, 64, pixels ) && IsRgba8( pixels, 64, 46 ) );
            ASSERT_FALSE( cache.TryGetImage( queued, 16, pixels ) );
            ASSERT_TRUE( 2 == cache.GetNetworkRequestCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache event: the picture is in memory when the fetch ends, the event comes on the thread that pumps" )
        try
        {
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            const CString url = UrlOf( world, "img-tall.tpix" );
            cache.RequestImage( url, 30 );

            // the pulse generator is not pumped: the fetch ends, the event waits
            IMAGE::TPixelMapPtr pixels;
            for ( UInt32 waited=0; waited<20000 && !cache.TryGetImage( url, 30, pixels ); waited+=2 )
            {
                MT::ThreadDelay( 2 );
            }
            MT::ThreadDelay( 100 );
            ASSERT_TRUE( IsRgba8( pixels, 10, 30 ) && 0 == observer.events.size() && 0 == cache.GetPendingCount() );
            ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
            ASSERT_TRUE( 1 == observer.events.size() && observer.allOnThisThread && IsRgba8( observer.events[ 0 ].pixels, 10, 30 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache pool: several fetches run side by side on the thread pool and every picture arrives" )
        try
        {
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            cache.SetThreadPoolName( "AssetStoreImagesTest" );
            ASSERT_TRUE( "AssetStoreImagesTest" == cache.GetThreadPoolName() && 4 == cache.GetMaxConcurrentFetches() );
            for ( UInt32 p=0; p<12; ++p )
                cache.RequestImage( UrlOf( world, "img-big.tpix" ) + "?n=" + ToString( p ), 40 );
            ASSERT_TRUE( 12 == cache.GetPendingCount() );
            ASSERT_TRUE( WaitForImages( observer, 12, 30000 ) );
            PumpFor( 30 );
            ASSERT_TRUE( 12 == observer.events.size() && 12 == cache.GetNetworkRequestCount() && 0 == cache.GetPendingCount() );
            for ( size_t e=0; e<observer.events.size(); ++e )
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == observer.events[ e ].status && IsRgba8( observer.events[ e ].pixels, 40, 20 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image cache PNG: a PNG from the store is decoded and scaled when an image codec plugin that decodes PNG is loaded" )
        CDynamicBuffer png;
        bool canDecode = false;
        if ( MakePng( 120, 60, false, png ) )
        {
            CDynamicBufferAccess access( png );
            IMAGE::CImage image;
            canDecode = image.Load( access, "png", true, false ) && image.GetFrameCount() > 0;
        }
        GUCEF_TESTFW_SKIP_IF( !canDecode, "no image codec plugin that decodes PNG is loaded" );
        try
        {
            world.server.SetFileContent( "img-png", png );
            CAssetStoreImageCache cache;
            CImageObserver observer;
            observer.Observe( cache );
            cache.RequestImage( UrlOf( world, "img-png" ), 30 );
            ASSERT_TRUE( WaitForImages( observer, 1, 20000 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == observer.events[ 0 ].status && IsRgba8( observer.events[ 0 ].pixels, 30, 15 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    world.server.Stop();
    UnmountDirectory( "imagecache" );
    codecs.Unregister( CPlainPictureCodec::TypeName );
}

/*-------------------------------------------------------------------------*/
