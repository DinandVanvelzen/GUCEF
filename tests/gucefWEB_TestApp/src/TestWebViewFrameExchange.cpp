/*
 *  gucefWEB_TestApp: tests of the gucefWEB module, its web views
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

#include <math.h>
#include <string.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

#include "TestWebViewSupport.h"
#include "TestWorkerThread.h"
#include "TestWebViewSuites.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::WEB;
using namespace GUCEF::WEBVIEWTEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

IMAGE::CPixelMap*
MakeFrame( UInt32 width, UInt32 height, UInt8 value )
{
    IMAGE::CPixelMap* frame = GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
    memset( frame->GetDataPtr(), value, (size_t) width * height * 4 );
    return frame;
}

void
Paint( IMAGE::CPixelMap& frame, const CWebViewRect& rect, UInt8 value )
{
    CWebViewRect r = rect;
    if ( !r.ClipTo( frame.GetWidthInPixels(), frame.GetHeightInPixels() ) )
        return;
    for ( Int32 y=r.top; y<r.GetBottom(); ++y )
        memset( static_cast< UInt8* >( frame.GetDataPtr() ) + ( (size_t) y * frame.GetWidthInPixels() + r.left ) * 4, value, (size_t) r.width * 4 );
}

bool
SamePixels( const IMAGE::CPixelMap& a, const IMAGE::CPixelMap& b )
{
    return a.GetWidthInPixels() == b.GetWidthInPixels() && a.GetHeightInPixels() == b.GetHeightInPixels() &&
           0 == memcmp( a.GetDataPtr(), b.GetDataPtr(), (size_t) a.GetWidthInPixels() * a.GetHeightInPixels() * 4 );
}

/** Pseudo random numbers that are the same on every run */
UInt32
NextRandom( UInt32& state )
{
    state = state * 1664525u + 1013904223u;
    return state >> 8;
}

/** The writer of the concurrency tests: whole frames of one value, or changed rectangles of a frame of its own */
struct SFrameWriter
{
    CWebViewFrameExchange* exchange;
    UInt32 frames;
    bool rectangles;
    IMAGE::CPixelMap* source;       /**< rectangles: the frame the writer keeps */
    MT::CTAtomic< UInt32 > written;

    SFrameWriter( void ) : exchange( GUCEF_NULL ), frames( 0 ), rectangles( false ), source( GUCEF_NULL ), written( 0 ) {}
};

void
WriteFrames( void* argument )
{
    SFrameWriter* writer = static_cast< SFrameWriter* >( argument );
    UInt32 random = 12345;
    for ( UInt32 i=1; i<=writer->frames; ++i )
    {
        if ( writer->rectangles )
        {
            const UInt32 w = writer->source->GetWidthInPixels(), h = writer->source->GetHeightInPixels();
            CWebViewRect rect( (Int32) ( NextRandom( random ) % w ), (Int32) ( NextRandom( random ) % h ), 1 + (Int32) ( NextRandom( random ) % 40 ), 1 + (Int32) ( NextRandom( random ) % 30 ) );
            Paint( *writer->source, rect, (UInt8) ( i & 0xFF ) );
            writer->exchange->UpdateFromRgba( *writer->source, rect, 1.0f );
        }
        else
        {
            CWebViewRect mustRepaint;
            IMAGE::CPixelMap& pixels = writer->exchange->BeginFrame( 128, 64, mustRepaint );
            memset( pixels.GetDataPtr(), (int) ( i & 0xFF ), (size_t) 128 * 64 * 4 );
            writer->exchange->PublishFrame( CWebViewRect::Whole( 128, 64 ), 1.0f );
        }
        ++writer->written;
        // now and then the reader gets the processor, also when both share one
        if ( 0 == i % 50 )
            MT::PrecisionDelay( 1 );
    }
}

}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformWebViewFrameExchangeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "WebView.FrameExchange" );

    GUCEF_TESTFW_TESTCASE( "FrameExchange: three buffers, allocated once per size, never per frame" )
    try
    {
        CWebViewFrameExchange exchange;
        ASSERT_FALSE( exchange.HasFrame() );
        CWebViewFrameInfo info;
        ASSERT_FALSE( exchange.TakeLatest( info ) );
        ASSERT_TRUE( GUCEF_NULL == exchange.GetReaderPixels() );
        IMAGE::CPixelMap* source = MakeFrame( 64, 32, 10 );
        for ( UInt32 i=0; i<100; ++i )
        {
            Paint( *source, CWebViewRect( i % 60, 0, 4, 4 ), (UInt8) i );
            ASSERT_TRUE( exchange.UpdateFromRgba( *source, CWebViewRect( i % 60, 0, 4, 4 ), 1.0f ) );
            if ( 0 == i % 3 )
                exchange.TakeLatest( info );
        }
        ASSERT_TRUE( 3 == exchange.GetAllocationCount() );
        ASSERT_TRUE( 100 == exchange.GetPublishedCount() );
        ASSERT_TRUE( exchange.HasFrame() );

        // another size: the buffers are made again as they come back to the writer, and no more
        IMAGE::CPixelMap* bigger = MakeFrame( 80, 40, 20 );
        for ( UInt32 i=0; i<10; ++i )
            exchange.UpdateFromRgba( *bigger, CWebViewRect( 0, 0, 1, 1 ), 2.0f );
        ASSERT_TRUE( 5 == exchange.GetAllocationCount() );          // the buffer the reader holds is not the writer's to change
        ASSERT_TRUE( exchange.TakeLatest( info ) && 80 == info.width && 40 == info.height && 2.0f == info.deviceScale );
        exchange.UpdateFromRgba( *bigger, CWebViewRect( 0, 0, 1, 1 ), 2.0f );
        exchange.UpdateFromRgba( *bigger, CWebViewRect( 0, 0, 1, 1 ), 2.0f );
        ASSERT_TRUE( 6 == exchange.GetAllocationCount() );          // back with the writer: made at the new size
        GUCEF_DELETE bigger;
        GUCEF_DELETE source;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FrameExchange: the dirty rectangle of a taken frame covers every frame the reader did not take" )
    try
    {
        CWebViewFrameExchange exchange;
        IMAGE::CPixelMap* source = MakeFrame( 100, 100, 0 );
        CWebViewFrameInfo info;
        exchange.UpdateFromRgba( *source, CWebViewRect::Whole( 100, 100 ), 1.0f );
        ASSERT_TRUE( exchange.TakeLatest( info ) && CWebViewRect::Whole( 100, 100 ) == info.dirty && 1 == info.frameNumber );   // the first: all of it

        exchange.UpdateFromRgba( *source, CWebViewRect( 10, 10, 5, 5 ), 1.0f );
        ASSERT_TRUE( exchange.TakeLatest( info ) && CWebViewRect( 10, 10, 5, 5 ) == info.dirty );
        ASSERT_FALSE( exchange.TakeLatest( info ) );                                     // nothing new
        ASSERT_TRUE( CWebViewRect( 10, 10, 5, 5 ) == info.dirty );                       // the frame held stays

        exchange.UpdateFromRgba( *source, CWebViewRect( 0, 0, 2, 2 ), 1.0f );
        exchange.UpdateFromRgba( *source, CWebViewRect( 50, 60, 10, 10 ), 1.0f );
        exchange.UpdateFromRgba( *source, CWebViewRect( 90, 5, 5, 5 ), 1.0f );
        ASSERT_TRUE( exchange.GetReplacedCount() >= 2 );
        ASSERT_TRUE( exchange.TakeLatest( info ) && 5 == info.frameNumber );
        ASSERT_TRUE( info.dirty.Contains( CWebViewRect( 0, 0, 2, 2 ) ) && info.dirty.Contains( CWebViewRect( 50, 60, 10, 10 ) ) && info.dirty.Contains( CWebViewRect( 90, 5, 5, 5 ) ) );

        // a dirty rectangle outside the frame is cut to it
        exchange.UpdateFromRgba( *source, CWebViewRect( 95, 95, 50, 50 ), 1.0f );
        ASSERT_TRUE( exchange.TakeLatest( info ) && CWebViewRect( 95, 95, 5, 5 ) == info.dirty );
        GUCEF_DELETE source;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FrameExchange: a reader copying only what changed has the writer's frame, also after skipped frames and a resize" )
    try
    {
        CWebViewFrameExchange exchange;
        IMAGE::CPixelMap* source = MakeFrame( 120, 90, 7 );
        IMAGE::CPixelMap target( GUCEF_NULL, 1, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        CWebViewFrameInfo info;
        CWebViewRect changed;
        UInt32 random = 99;
        UInt32 compared = 0;
        for ( UInt32 i=0; i<400; ++i )
        {
            CWebViewRect rect( (Int32) ( NextRandom( random ) % 120 ), (Int32) ( NextRandom( random ) % 90 ), 1 + (Int32) ( NextRandom( random ) % 30 ), 1 + (Int32) ( NextRandom( random ) % 20 ) );
            Paint( *source, rect, (UInt8) ( NextRandom( random ) & 0xFF ) );
            exchange.UpdateFromRgba( *source, rect, 1.0f );
            if ( 0 == NextRandom( random ) % 4 )
            {
                exchange.TakeLatestInto( target, info, changed );
                ASSERT_TRUE( SamePixels( *source, target ) );
                ++compared;
            }
            if ( 200 == i )
            {
                GUCEF_DELETE source;
                source = MakeFrame( 60, 140, 3 );
                exchange.UpdateFromRgba( *source, CWebViewRect( 0, 0, 1, 1 ), 1.0f );
            }
        }
        exchange.TakeLatestInto( target, info, changed );
        ASSERT_TRUE( SamePixels( *source, target ) && compared > 50 );
        GUCEF_DELETE source;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FrameExchange: no torn frames while a writer thread publishes and the reader takes them" )
    try
    {
        CWebViewFrameExchange exchange;
        SFrameWriter writer;
        writer.exchange = &exchange;
        writer.frames = 4000;
        CTestWorkerThread thread( &WriteFrames, &writer );
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, 0, 0 ) );
        UInt32 taken = 0;
        UInt32 torn = 0;
        UInt64 lastNumber = 0;
        bool ordered = true;
        CWebViewFrameInfo info;
        while ( !thread.IsDone() || exchange.HasNewFrame() )
        {
            if ( !exchange.TakeLatest( info ) )
                continue;
            ++taken;
            if ( info.frameNumber <= lastNumber )
                ordered = false;
            lastNumber = info.frameNumber;
            const IMAGE::CPixelMap* pixels = exchange.GetReaderPixels();
            const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() );
            const UInt8 expected = (UInt8) ( info.frameNumber & 0xFF );
            for ( UInt32 n=0; n<128*64*4; n+=7 )
            {
                if ( data[ n ] != expected )
                {
                    ++torn;
                    break;
                }
            }
        }
        ASSERT_TRUE( thread.WaitUntilDone( 60000 ) );
        ASSERT_TRUE( 0 == torn );
        ASSERT_TRUE( ordered );
        ASSERT_TRUE( taken > 10 );
        ASSERT_TRUE( 4000 == exchange.GetPublishedCount() );
        ASSERT_TRUE( 4000 == lastNumber );
        ASSERT_TRUE( 3 == exchange.GetAllocationCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FrameExchange: changed rectangles from a writer thread add up to the writer's frame at the reader" )
    try
    {
        CWebViewFrameExchange exchange;
        IMAGE::CPixelMap* source = MakeFrame( 200, 150, 0 );
        SFrameWriter writer;
        writer.exchange = &exchange;
        writer.frames = 3000;
        writer.rectangles = true;
        writer.source = source;
        IMAGE::CPixelMap target( GUCEF_NULL, 1, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        CTestWorkerThread thread( &WriteFrames, &writer );
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, 0, 0 ) );
        CWebViewFrameInfo info;
        CWebViewRect changed;
        UInt32 takes = 0;
        while ( !thread.IsDone() )
        {
            if ( exchange.TakeLatestInto( target, info, changed ) )
                ++takes;
        }
        ASSERT_TRUE( thread.WaitUntilDone( 60000 ) );
        exchange.TakeLatestInto( target, info, changed );
        ASSERT_TRUE( 3000 == info.frameNumber );
        ASSERT_TRUE( SamePixels( *source, target ) );
        ASSERT_TRUE( takes > 10 );
        GUCEF_DELETE source;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FrameExchange: BGRA premultiplied pixels become straight alpha RGBA; the last frame can be copied" )
    try
    {
        CWebViewFrameExchange exchange;
        const UInt8 bgra[ 4 * 4 ] = { 30, 20, 10, 255,      0, 0, 0, 0,     50, 25, 0, 128,     255, 255, 255, 255 };
        ASSERT_TRUE( exchange.UpdateFromBgraPremultiplied( bgra, 8, 2, 2, CWebViewRect::Whole( 2, 2 ), 1.0f ) );
        CWebViewFrameInfo info;
        ASSERT_TRUE( exchange.TakeLatest( info ) );
        const IMAGE::CPixelMap* pixels = exchange.GetReaderPixels();
        ASSERT_TRUE( GUCEF_NULL != pixels && 2 == pixels->GetWidthInPixels() );
        const UInt8* p = static_cast< const UInt8* >( pixels->GetDataPtr() );
        ASSERT_TRUE( 10 == p[ 0 ] && 20 == p[ 1 ] && 30 == p[ 2 ] && 255 == p[ 3 ] );
        ASSERT_TRUE( 0 == p[ 4 ] && 0 == p[ 7 ] );
        ASSERT_TRUE( 0 == p[ 8 ] && 50 == p[ 9 ] && 100 == p[ 10 ] && 128 == p[ 11 ] );     // 25 / 128 * 255 = 50, 50 / 128 * 255 = 100 (rounded)
        ASSERT_TRUE( 255 == p[ 12 ] && 255 == p[ 15 ] );
        IMAGE::CPixelMap copy( GUCEF_NULL, 1, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        ASSERT_TRUE( exchange.CopyLastPublished( copy ) && SamePixels( copy, *pixels ) );
        CWebViewFrameExchange empty;
        ASSERT_FALSE( empty.CopyLastPublished( copy ) );
        ASSERT_FALSE( empty.PublishFrame( CWebViewRect(), 1.0f ) );                      // nothing begun
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
