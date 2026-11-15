/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H
#define GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTSLOTDATAEXCHANGE_H
#include "gucefMT_CTSlotDataExchange.h"
#define GUCEF_MT_CTSLOTDATAEXCHANGE_H
#endif /* GUCEF_MT_CTSLOTDATAEXCHANGE_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_WEB_CWEBVIEWFRAMEINFO_H
#include "gucefWEB_CWebViewFrameInfo.h"
#define GUCEF_WEB_CWEBVIEWFRAMEINFO_H
#endif /* GUCEF_WEB_CWEBVIEWFRAMEINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Hands the frames of a view from the engine thread (the one writer) to one reader thread (the GUI, a frame streamer)
 *  without locks and without allocation in steady state.
 *
 *  Three RGBA pixel maps (8 bits per channel, straight alpha, device pixels) and an MT::CTSlotDataExchange with latest
 *  only delivery for one reader. The value that goes through the exchange is a small CWebViewFrameInfo that names the
 *  buffer. Values stay in the slot whose index they name (see CTSlotDataExchange::GetWriterSlotIndex()), so the buffer
 *  the writer fills is never the one the reader uses nor the one waiting to be taken: the reader never sees a frame
 *  that is half written. A frame the reader did not take is replaced, never queued: memory stays at three frames.
 *  The buffers are allocated when the size changes, never per frame.
 *
 *  Writer (engine thread):
 *      CWebViewRect mustRepaint;
 *      IMAGE::CPixelMap& pixels = exchange.BeginFrame( width, height, mustRepaint );
 *      // write the pixels of mustRepaint and of what changed (dirty) into 'pixels'
 *      exchange.PublishFrame( dirty, scale );
 *  The buffer handed out may be two frames old: mustRepaint is what changed in the frames published since it was
 *  written (all of it after a change of size). Engines that keep a complete frame of their own (a cairo surface, a
 *  pixel map) use UpdateFromRgba() or UpdateFromBgraPremultiplied(), which do both steps and convert only the
 *  rectangles that are needed.
 *
 *  Reader (one thread): TakeLatest() and GetReaderPixels(), or TakeLatestInto() which copies only what changed into a
 *  pixel map of the reader's own. The dirty rectangle of a frame the reader takes covers every frame it did not take.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewFrameExchange
{
    public:

    CWebViewFrameExchange( void );

    ~CWebViewFrameExchange();

    /*
     *  Writer side: the engine thread
     */

    /** The buffer to write the next frame into, at the given size; 'mustRepaint' is the part that is not up to date */
    IMAGE::CPixelMap& BeginFrame( UInt32 width, UInt32 height, CWebViewRect& mustRepaint );

    /** Publishes the frame begun with BeginFrame(); 'dirty' is what changed since the frame published before. False without BeginFrame() */
    bool PublishFrame( const CWebViewRect& dirty, Float32 deviceScale );

    /** Publishes the changed part of a complete RGBA frame (8 bits per channel) */
    bool UpdateFromRgba( const IMAGE::CPixelMap& frame, const CWebViewRect& dirty, Float32 deviceScale );

    /** Publishes the changed part of a complete frame of BGRA premultiplied pixels (cairo ARGB32 on little endian, CEF OnPaint) */
    bool UpdateFromBgraPremultiplied( const UInt8* pixels, UInt32 strideInBytes, UInt32 width, UInt32 height, const CWebViewRect& dirty, Float32 deviceScale );

    /** Writer thread only: a copy of the frame published last; false before the first */
    bool CopyLastPublished( IMAGE::CPixelMap& out ) const;

    /*
     *  Reader side: one thread
     */

    /** The newest frame; true when it is new since the previous call. 'info' is the frame the reader holds now (also when not new) */
    bool TakeLatest( CWebViewFrameInfo& info );

    /** The pixels of the frame the last TakeLatest() gave; valid until the next TakeLatest(). NULL before the first frame */
    const IMAGE::CPixelMap* GetReaderPixels( void ) const;

    /**
     *  Takes the newest frame and copies what changed into 'target' (resized to the frame when its size differs, then
     *  all of it is copied). True when 'target' changed; 'changed' is the part that was copied.
     */
    bool TakeLatestInto( IMAGE::CPixelMap& target, CWebViewFrameInfo& info, CWebViewRect& changed );

    /*
     *  Any thread
     */

    /** True when a frame waits that the reader did not take yet */
    bool HasNewFrame( void ) const;

    /** True once a frame was published */
    bool HasFrame( void ) const;

    UInt32 GetPublishedCount( void ) const;

    /** Frames that were replaced before the reader took them */
    UInt32 GetReplacedCount( void ) const;

    /** Buffer allocations so far: three for a view whose size never changed */
    UInt32 GetAllocationCount( void ) const;

    /*
     *  Helpers
     */

    /** Copies a rectangle between RGBA pixel maps (8 bits per channel) of the same size */
    static void CopyRegion( const IMAGE::CPixelMap& source, IMAGE::CPixelMap& target, const CWebViewRect& region );

    /** Converts a rectangle of BGRA premultiplied pixels into straight alpha RGBA */
    static void CopyRegionFromBgraPremultiplied( const UInt8* source, UInt32 strideInBytes, IMAGE::CPixelMap& target, const CWebViewRect& region );

    private:

    CWebViewFrameExchange( const CWebViewFrameExchange& src );              /**< not implemented */
    CWebViewFrameExchange& operator=( const CWebViewFrameExchange& src );   /**< not implemented */

    typedef MT::CTSlotDataExchange< CWebViewFrameInfo, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, 1 > TExchange;

    private:

    TExchange m_exchange;
    IMAGE::CPixelMap* m_buffers[ 3 ];

    // writer thread only
    CWebViewRect m_stale[ 3 ];          /**< per buffer: what changed since it was written */
    bool m_frameBegun;
    UInt32 m_beginIndex;
    UInt32 m_beginWidth;
    UInt32 m_beginHeight;
    UInt64 m_frameNumber;
    UInt32 m_lastWidth;
    UInt32 m_lastHeight;
    Int32 m_lastPublishedIndex;
    CWebViewRect m_lastTokenDirty;

    // reader thread only
    CWebViewFrameInfo m_readerInfo;
    bool m_readerHas;

    MT::CTAtomic< UInt32 > m_published;
    MT::CTAtomic< UInt32 > m_replaced;
    MT::CTAtomic< UInt32 > m_allocations;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H ? */
