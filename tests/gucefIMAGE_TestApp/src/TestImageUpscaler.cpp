/*
 *  gucefIMAGE_TestApp: Test application for the gucefIMAGE module
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#include "TestImageUpscaler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** An RGBA picture filled with 'background' */
IMAGE::TPixelMapPtr
MakePicture( UInt32 width, UInt32 height, UInt32 background )
{
    IMAGE::TPixelMapPtr picture( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( picture->GetDataPtr() );
    for ( UInt32 i=0; i<width*height; ++i )
        memcpy( data + i * 4, &background, 4 );
    return picture;
}

UInt32
PixelAt( const IMAGE::CPixelMap& picture, UInt32 x, UInt32 y )
{
    UInt32 pixel;
    memcpy( &pixel, static_cast< const UInt8* >( picture.GetDataPtr() ) + ( (size_t) y * picture.GetWidthInPixels() + x ) * 4, 4 );
    return pixel;
}

void
SetPixel( IMAGE::CPixelMap& picture, UInt32 x, UInt32 y, UInt32 pixel )
{
    memcpy( static_cast< UInt8* >( picture.GetDataPtr() ) + ( (size_t) y * picture.GetWidthInPixels() + x ) * 4, &pixel, 4 );
}

/** The picture changed by a symmetry of the square: mirrored left to right and / or transposed */
IMAGE::TPixelMapPtr
Transformed( const IMAGE::CPixelMap& picture, bool mirror, bool transpose )
{
    UInt32 width = picture.GetWidthInPixels(), height = picture.GetHeightInPixels();
    IMAGE::TPixelMapPtr result = MakePicture( transpose ? height : width, transpose ? width : height, 0 );
    for ( UInt32 y=0; y<height; ++y )
        for ( UInt32 x=0; x<width; ++x )
        {
            UInt32 tx = mirror ? width - 1 - x : x;
            UInt32 ty = y;
            if ( transpose )
                SetPixel( *result, ty, tx, PixelAt( picture, x, y ) );
            else
                SetPixel( *result, tx, ty, PixelAt( picture, x, y ) );
        }
    return result;
}

bool
SamePixels( const IMAGE::CPixelMap& a, const IMAGE::CPixelMap& b )
{
    if ( a.GetWidthInPixels() != b.GetWidthInPixels() || a.GetHeightInPixels() != b.GetHeightInPixels() )
        return false;
    return 0 == memcmp( a.GetDataPtr(), b.GetDataPtr(), (size_t) a.GetWidthInPixels() * a.GetHeightInPixels() * 4 );
}

/** Pixel art: lines of several slopes, blocks, single pixels and dithering in 4 colours */
IMAGE::TPixelMapPtr
MakePixelArt( void )
{
    const UInt32 colours[] = { 0xFF402010, 0xFFFFFFFF, 0xFF2040E0, 0xFF20C040 };
    IMAGE::TPixelMapPtr art = MakePicture( 48, 40, colours[ 0 ] );
    for ( UInt32 i=0; i<30; ++i )
    {
        SetPixel( *art, 2 + i, 2 + i, colours[ 1 ] );
        SetPixel( *art, 4 + i, 20 + i / 2, colours[ 2 ] );
        SetPixel( *art, 40 - i / 3, 4 + i, colours[ 3 ] );
    }
    for ( UInt32 y=8; y<14; ++y )
        for ( UInt32 x=20; x<30; ++x )
            SetPixel( *art, x, y, ( x + y ) % 2 ? colours[ 2 ] : colours[ 3 ] );
    for ( UInt32 y=28; y<36; ++y )
        for ( UInt32 x=30; x<44; ++x )
            SetPixel( *art, x, y, colours[ 1 ] );
    SetPixel( *art, 10, 36, colours[ 3 ] );
    return art;
}

/** Records the progress it is told; stops the work at 'stopAt' */
class CProgressRecorder : public IMAGE::CIUpscaleProgress
{
    public:

    GUCEF::vector< Float32 > fractions;
    Float32 stopAt;

    CProgressRecorder( Float32 stop ) : fractions(), stopAt( stop ) {}

    virtual bool OnUpscaleProgress( Float32 fraction ) GUCEF_VIRTUAL_OVERRIDE
    {
        fractions.push_back( fraction );
        return fraction < stopAt;
    }
};

/** Loads the ncnn upscaler plugin, the debug or release build of it; true when "realesrgan" is registered */
bool
LoadNcnnUpscalerPlugin( void )
{
    IMAGE::CImageUpscalerRegistry& registry = IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry();
    if ( GUCEF_NULL == registry.Get( "realesrgan" ) )
    {
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        if ( !pluginControl.AddPluginFromDir( "$MODULEDIR$/imgupscalepluginNCNN_d", "imgupscaleplugin", true ) )
            pluginControl.AddPluginFromDir( "$MODULEDIR$/imgupscalepluginNCNN", "imgupscaleplugin", true );
    }
    return GUCEF_NULL != registry.Get( "realesrgan" );
}

/** A drawn picture: soft gradients, hard edges, a circle */
IMAGE::TPixelMapPtr
MakeDrawing( UInt32 width, UInt32 height )
{
    IMAGE::TPixelMapPtr picture = MakePicture( width, height, 0xFF000000 );
    for ( UInt32 y=0; y<height; ++y )
        for ( UInt32 x=0; x<width; ++x )
        {
            UInt32 r = 40 + x * 160 / width, g = 60 + y * 140 / height, b = 120;
            Int32 dx = (Int32) x - (Int32) width / 2, dy = (Int32) y - (Int32) height / 2;
            if ( dx * dx + dy * dy < (Int32) ( height * height / 9 ) )
            {
                r = 240; g = 200; b = 40;
            }
            if ( x > width * 3 / 4 && y < height / 4 )
            {
                r = 20; g = 20; b = 30;
            }
            SetPixel( *picture, x, y, 0xFF000000 | ( b << 16 ) | ( g << 8 ) | r );
        }
    return picture;
}

/** An upscaler that only has a name, for the registry */
class CNamedUpscaler : public IMAGE::CImageUpscaler
{
    public:

    CNamedUpscaler( const CORE::CString& name ) : m_name( name ) {}

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }

    virtual TFactorVector GetFactors( void ) const GUCEF_VIRTUAL_OVERRIDE { TFactorVector factors; factors.push_back( 3 ); return factors; }

    virtual bool Upscale( const IMAGE::CPixelMap&, UInt32, IMAGE::TPixelMapPtr&, IMAGE::CIUpscaleProgress* ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    private:

    CORE::CString m_name;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformImageUpscalerTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "gucefIMAGE.Upscalers" );

    GUCEF_TESTFW_TESTCASE( "Upscalers: The registry knows mmpx and the upscalers that are registered, by name" )
    try
    {
        IMAGE::CImageUpscalerRegistry& registry = IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry();
        IMAGE::CImageUpscaler* mmpx = registry.Get( "mmpx" );
        ASSERT_TRUE( GUCEF_NULL != mmpx && "mmpx" == mmpx->GetName() );
        if ( GUCEF_NULL != mmpx )
        {
            ASSERT_TRUE( 2 == mmpx->GetFactorOfAtLeast( 1 ) && 2 == mmpx->GetFactorOfAtLeast( 2 ) && 4 == mmpx->GetFactorOfAtLeast( 3 ) && 4 == mmpx->GetFactorOfAtLeast( 7 ) );
        }

        CNamedUpscaler other( "other" ), again( "mmpx" );
        ASSERT_TRUE( registry.Register( &other ) );
        ASSERT_FALSE( registry.Register( &again ) );
        IMAGE::CImageUpscalerRegistry::TNameVector names = registry.GetNames();
        ASSERT_TRUE( 2 == names.size() && "mmpx" == names[ 0 ] && "other" == names[ 1 ] );
        ASSERT_TRUE( &other == registry.Get( "other" ) && 3 == other.GetFactorOfAtLeast( 2 ) );
        ASSERT_TRUE( registry.Unregister( "other" ) );
        ASSERT_FALSE( registry.Unregister( "other" ) );
        ASSERT_TRUE( GUCEF_NULL == registry.Get( "other" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Upscalers: mmpx doubles pixel art with only its colours, the same way in every orientation" )
    try
    {
        IMAGE::CImageUpscaler* mmpx = IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry().Get( "mmpx" );
        ASSERT_TRUE( GUCEF_NULL != mmpx );
        if ( GUCEF_NULL != mmpx )
        {
            // flat stays flat, a pixel on its own stays a block
            IMAGE::TPixelMapPtr flat = MakePicture( 6, 5, 0xFF336699 );
            SetPixel( *flat, 2, 2, 0xFFFFFFFF );
            IMAGE::TPixelMapPtr doubled;
            ASSERT_TRUE( mmpx->Upscale( *flat, 2, doubled, GUCEF_NULL ) && 12 == doubled->GetWidthInPixels() && 10 == doubled->GetHeightInPixels() );
            UInt32 wrong = 0;
            for ( UInt32 y=0; y<10; ++y )
                for ( UInt32 x=0; x<12; ++x )
                    if ( PixelAt( *doubled, x, y ) != PixelAt( *flat, x / 2, y / 2 ) )
                        ++wrong;
            ASSERT_TRUE( 0 == wrong );

            // pixel art: only colours it has, lines that are not staircases of blocks anymore
            IMAGE::TPixelMapPtr art = MakePixelArt();
            ASSERT_TRUE( mmpx->Upscale( *art, 2, doubled, GUCEF_NULL ) );
            const UInt32 colours[] = { 0xFF402010, 0xFFFFFFFF, 0xFF2040E0, 0xFF20C040 };
            UInt32 notInPalette = 0, notNearest = 0;
            for ( UInt32 y=0; y<doubled->GetHeightInPixels(); ++y )
                for ( UInt32 x=0; x<doubled->GetWidthInPixels(); ++x )
                {
                    UInt32 pixel = PixelAt( *doubled, x, y );
                    if ( pixel != colours[ 0 ] && pixel != colours[ 1 ] && pixel != colours[ 2 ] && pixel != colours[ 3 ] )
                        ++notInPalette;
                    if ( pixel != PixelAt( *art, x / 2, y / 2 ) )
                        ++notNearest;
                }
            ASSERT_TRUE( 0 == notInPalette && notNearest > 50 );

            // the rules come in mirrored and turned sets: mirroring or transposing the picture does the same to the result
            for ( UInt32 t=1; t<4; ++t )
            {
                bool mirror = 0 != ( t & 1 ), transpose = 0 != ( t & 2 );
                IMAGE::TPixelMapPtr turnedResult;
                ASSERT_TRUE( mmpx->Upscale( *Transformed( *art, mirror, transpose ), 2, turnedResult, GUCEF_NULL ) );
                ASSERT_TRUE( SamePixels( *turnedResult, *Transformed( *doubled, mirror, transpose ) ) );
            }

            // 4 times, other factors not, and the picture of the caller stays as it was
            IMAGE::TPixelMapPtr fourTimes;
            ASSERT_TRUE( mmpx->Upscale( *art, 4, fourTimes, GUCEF_NULL ) && 192 == fourTimes->GetWidthInPixels() && 160 == fourTimes->GetHeightInPixels() );
            ASSERT_FALSE( mmpx->Upscale( *art, 3, fourTimes, GUCEF_NULL ) );
            IMAGE::CPixelMap rgb( *art );
            ASSERT_TRUE( rgb.ConvertFormatTo( IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
            IMAGE::TPixelMapPtr fromRgb;
            ASSERT_TRUE( mmpx->Upscale( rgb, 2, fromRgb, GUCEF_NULL ) && SamePixels( *fromRgb, *doubled ) );
            ASSERT_TRUE( IMAGE::PSF_RGB == rgb.GetPixelStorageFormat() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Upscalers: The ncnn plugin upscales with Real-ESRGAN 2, 3 and 4 times, keeping the picture and its alpha" )
    if ( !LoadNcnnUpscalerPlugin() )
        GUCEF_TESTFW_SKIP( "the ncnn upscaler plugin is not there" );
    try
    {
        IMAGE::CImageUpscaler* realesrgan = IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry().Get( "realesrgan" );
        ASSERT_TRUE( GUCEF_NULL != realesrgan );
        if ( GUCEF_NULL != realesrgan )
        {
            IMAGE::CImageUpscaler::TFactorVector factors = realesrgan->GetFactors();
            ASSERT_TRUE( 3 == factors.size() && 2 == factors[ 0 ] && 4 == factors[ 2 ] );

            IMAGE::TPixelMapPtr drawing = MakeDrawing( 96, 72 );
            // a see through corner
            for ( UInt32 y=60; y<72; ++y )
                for ( UInt32 x=0; x<12; ++x )
                    SetPixel( *drawing, x, y, PixelAt( *drawing, x, y ) & 0x00FFFFFF );

            for ( UInt32 factor=2; factor<=4; ++factor )
            {
                IMAGE::TPixelMapPtr upscaled;
                CProgressRecorder progress( 2.0f );
                ASSERT_TRUE( realesrgan->Upscale( *drawing, factor, upscaled, &progress ) );
                if ( upscaled.IsNULL() )
                    continue;
                ASSERT_TRUE( 96 * factor == upscaled->GetWidthInPixels() && 72 * factor == upscaled->GetHeightInPixels() );
                ASSERT_TRUE( !progress.fractions.empty() && 1.0f == progress.fractions.back() );

                // the picture is the picture: close to the original pixel at every place, not a copy of its blocks
                UInt64 difference = 0;
                UInt32 notNearest = 0, alphaWrong = 0;
                for ( UInt32 y=0; y<upscaled->GetHeightInPixels(); ++y )
                    for ( UInt32 x=0; x<upscaled->GetWidthInPixels(); ++x )
                    {
                        UInt32 a = PixelAt( *upscaled, x, y ), n = PixelAt( *drawing, x / factor, y / factor );
                        for ( UInt32 c=0; c<3; ++c )
                        {
                            Int32 d = (Int32) ( ( a >> ( c * 8 ) ) & 0xFF ) - (Int32) ( ( n >> ( c * 8 ) ) & 0xFF );
                            difference += (UInt64) ( d < 0 ? -d : d );
                        }
                        if ( ( a & 0x00FFFFFF ) != ( n & 0x00FFFFFF ) )
                            ++notNearest;
                        // alpha away from the edge of the see through corner is what it was
                        UInt32 sx = x / factor, sy = y / factor;
                        bool nearCornerEdge = ( sx >= 11 && sx <= 12 ) || ( sy >= 59 && sy <= 60 );
                        if ( !nearCornerEdge && ( a >> 24 ) != ( n >> 24 ) )
                            ++alphaWrong;
                    }
                Float64 meanDifference = (Float64) difference / ( 3.0 * upscaled->GetWidthInPixels() * upscaled->GetHeightInPixels() );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Real-ESRGAN " << factor << "x: mean difference with the original pixels " << meanDifference << ", " << notNearest << " pixels not a copy" );
                ASSERT_TRUE( meanDifference < 12.0 );
                ASSERT_TRUE( notNearest > upscaled->GetWidthInPixels() * upscaled->GetHeightInPixels() / 20 );
                ASSERT_TRUE( 0 == alphaWrong );
            }

            // stopped, there is no result
            IMAGE::TPixelMapPtr none;
            CProgressRecorder stopped( 0.0f );
            ASSERT_FALSE( realesrgan->Upscale( *drawing, 2, none, &stopped ) );
            ASSERT_TRUE( none.IsNULL() );
            ASSERT_FALSE( realesrgan->Upscale( *drawing, 5, none, GUCEF_NULL ) );

            // a picture of more than one tile has no seams: neighbours across a tile edge are as alike as elsewhere
            IMAGE::TPixelMapPtr large = MakeDrawing( 420, 260 ), largeUpscaled;
            ASSERT_TRUE( realesrgan->Upscale( *large, 2, largeUpscaled, GUCEF_NULL ) && 840 == largeUpscaled->GetWidthInPixels() );
            if ( !largeUpscaled.IsNULL() )
            {
                UInt32 seamJumps = 0;
                for ( UInt32 y=0; y<largeUpscaled->GetHeightInPixels(); ++y )
                {
                    // the tiles are 200 pixels: the edge between them is at 400 after doubling
                    UInt32 a = PixelAt( *largeUpscaled, 399, y ), b = PixelAt( *largeUpscaled, 400, y );
                    Int32 d = 0;
                    for ( UInt32 c=0; c<3; ++c )
                    {
                        Int32 e = (Int32) ( ( a >> ( c * 8 ) ) & 0xFF ) - (Int32) ( ( b >> ( c * 8 ) ) & 0xFF );
                        d += e < 0 ? -e : e;
                    }
                    if ( d > 30 )
                        ++seamJumps;
                }
                ASSERT_TRUE( seamJumps < 10 );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Upscalers: mmpx tells how far it is and stops when told" )
    try
    {
        IMAGE::CImageUpscaler* mmpx = IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry().Get( "mmpx" );
        ASSERT_TRUE( GUCEF_NULL != mmpx );
        if ( GUCEF_NULL != mmpx )
        {
            IMAGE::TPixelMapPtr picture = MakePicture( 64, 64, 0xFF000000 ), result;
            CProgressRecorder all( 2.0f );
            ASSERT_TRUE( mmpx->Upscale( *picture, 4, result, &all ) );
            ASSERT_TRUE( all.fractions.size() > 2 && 1.0f == all.fractions.back() );
            for ( UInt32 i=1; i<(UInt32)all.fractions.size(); ++i )
                ASSERT_TRUE( all.fractions[ i ] >= all.fractions[ i - 1 ] );

            CProgressRecorder stopped( 0.1f );
            IMAGE::TPixelMapPtr none;
            ASSERT_FALSE( mmpx->Upscale( *picture, 4, none, &stopped ) );
            ASSERT_TRUE( none.IsNULL() && stopped.fractions.back() >= 0.1f && stopped.fractions.back() < 1.0f );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Upscalers: An upscale on a thread of a pool gives the same picture as on the calling thread" )
    try
    {
        IMAGE::CImageUpscaler* mmpx = IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry().Get( "mmpx" );
        ASSERT_TRUE( GUCEF_NULL != mmpx );
        if ( GUCEF_NULL != mmpx )
        {
            IMAGE::TPixelMapPtr art = MakePixelArt(), here, there;
            ASSERT_TRUE( mmpx->Upscale( *art, 2, here, GUCEF_NULL ) );
            CORE::CFutureResult future = mmpx->UpscaleAsync( art, 2 );
            ASSERT_TRUE( future.HasAFuture() );
            future.Await( 30000 );
            ASSERT_TRUE( future.IsReady() );
            ASSERT_TRUE( IMAGE::CImageUpscaler::GetAsyncResult( future, there ) );
            ASSERT_TRUE( !here.IsNULL() && !there.IsNULL() && SamePixels( *here, *there ) );

            // a factor it does not have: the task ends without a picture
            CORE::CFutureResult wrongFactor = mmpx->UpscaleAsync( art, 3 );
            wrongFactor.Await( 30000 );
            IMAGE::TPixelMapPtr none;
            ASSERT_TRUE( wrongFactor.IsReady() && !IMAGE::CImageUpscaler::GetAsyncResult( wrongFactor, none ) && none.IsNULL() );
        }

        // Real-ESRGAN takes long enough to be asked to stop
        if ( LoadNcnnUpscalerPlugin() )
        {
            IMAGE::CImageUpscaler* realesrgan = IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry().Get( "realesrgan" );
            ASSERT_TRUE( GUCEF_NULL != realesrgan );
            if ( GUCEF_NULL != realesrgan )
            {
                IMAGE::TPixelMapPtr drawing = MakeDrawing( 96, 72 ), here, there;
                ASSERT_TRUE( realesrgan->Upscale( *drawing, 2, here, GUCEF_NULL ) );
                CORE::CFutureResult future = realesrgan->UpscaleAsync( drawing, 2 );
                future.Await( 120000 );
                ASSERT_TRUE( IMAGE::CImageUpscaler::GetAsyncResult( future, there ) );
                ASSERT_TRUE( !here.IsNULL() && !there.IsNULL() && SamePixels( *here, *there ) );

                // pictures upscaled two at a time on two threads (a game builds its art cache that way): each as when alone
                const CORE::CString poolName = "gucefIMAGE_TestApp.Upscalers";
                CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( poolName );
                ASSERT_TRUE( !pool.IsNULL() );
                if ( !pool.IsNULL() )
                    pool->SetDesiredMinNrOfWorkerThreads( 2 );
                IMAGE::TPixelMapPtr picture = MakeDrawing( 640, 480 ), alone;
                ASSERT_TRUE( realesrgan->Upscale( *picture, 2, alone, GUCEF_NULL ) );
                UInt32 different = 0;
                for ( UInt32 round=0; round<12; ++round )
                {
                    CORE::CFutureResult first = realesrgan->UpscaleAsync( picture, 2, poolName );
                    CORE::CFutureResult second = realesrgan->UpscaleAsync( picture, 2, poolName );
                    CORE::CFutureResult* futures[ 2 ] = { &first, &second };
                    for ( UInt32 i=0; i<2; ++i )
                    {
                        futures[ i ]->Await( 300000 );
                        IMAGE::TPixelMapPtr result;
                        ASSERT_TRUE( IMAGE::CImageUpscaler::GetAsyncResult( *futures[ i ], result ) );
                        if ( alone.IsNULL() || result.IsNULL() || !SamePixels( *alone, *result ) )
                            ++different;
                    }
                }
                ASSERT_TRUE( 0 == different );
                CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( poolName );

                IMAGE::TPixelMapPtr large = MakeDrawing( 800, 600 ), none;
                CORE::CFutureResult cancelled = realesrgan->UpscaleAsync( large, 4 );
                ASSERT_TRUE( cancelled.RequestCancellation() );
                cancelled.Await( 120000 );
                ASSERT_TRUE( cancelled.IsReady() && !IMAGE::CImageUpscaler::GetAsyncResult( cancelled, none ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
