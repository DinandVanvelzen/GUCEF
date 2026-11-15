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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "gucefWEB_CWebViewFrameExchange.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebViewFrameExchange::CWebViewFrameExchange( void )
    : m_exchange()
    , m_frameBegun( false )
    , m_beginIndex( 0 )
    , m_beginWidth( 0 )
    , m_beginHeight( 0 )
    , m_frameNumber( 0 )
    , m_lastWidth( 0 )
    , m_lastHeight( 0 )
    , m_lastPublishedIndex( -1 )
    , m_lastTokenDirty()
    , m_readerInfo()
    , m_readerHas( false )
    , m_published( 0 )
    , m_replaced( 0 )
    , m_allocations( 0 )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<3; ++i )
        m_buffers[ i ] = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWebViewFrameExchange::~CWebViewFrameExchange()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<3; ++i )
    {
        GUCEF_DELETE m_buffers[ i ];
        m_buffers[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

IMAGE::CPixelMap&
CWebViewFrameExchange::BeginFrame( UInt32 width, UInt32 height, CWebViewRect& mustRepaint )
{GUCEF_TRACE;

    if ( 0 == width )
        width = 1;
    if ( 0 == height )
        height = 1;
    const UInt32 index = m_exchange.GetWriterSlotIndex();
    IMAGE::CPixelMap*& buffer = m_buffers[ index ];
    if ( GUCEF_NULL == buffer )
    {
        buffer = GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        ++m_allocations;
        m_stale[ index ] = CWebViewRect::Whole( width, height );
    }
    else
    if ( buffer->GetWidthInPixels() != width || buffer->GetHeightInPixels() != height )
    {
        // the buffer is the writer's own: neither the reader nor the hand over place refers to it
        buffer->Assign( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        ++m_allocations;
        m_stale[ index ] = CWebViewRect::Whole( width, height );
    }
    mustRepaint = m_stale[ index ];
    mustRepaint.ClipTo( width, height );
    m_frameBegun = true;
    m_beginIndex = index;
    m_beginWidth = width;
    m_beginHeight = height;
    return *buffer;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFrameExchange::PublishFrame( const CWebViewRect& dirty, Float32 deviceScale )
{GUCEF_TRACE;

    if ( !m_frameBegun )
        return false;
    m_frameBegun = false;

    const CWebViewRect whole = CWebViewRect::Whole( m_beginWidth, m_beginHeight );
    CWebViewRect changed = dirty;
    changed.ClipTo( m_beginWidth, m_beginHeight );
    const bool sizeChanged = m_beginWidth != m_lastWidth || m_beginHeight != m_lastHeight;
    if ( sizeChanged )
        changed = whole;

    // the frame waiting now is replaced by this one: what it changed is part of what this one changes for the reader
    CWebViewRect forReader = changed;
    if ( m_exchange.HasNewData() )
    {
        if ( !sizeChanged )
            forReader.Unite( m_lastTokenDirty );
        ++m_replaced;
    }
    forReader.ClipTo( m_beginWidth, m_beginHeight );

    CWebViewFrameInfo info;
    info.bufferIndex = m_beginIndex;
    info.width = m_beginWidth;
    info.height = m_beginHeight;
    info.deviceScale = deviceScale;
    info.frameNumber = ++m_frameNumber;
    info.dirty = forReader;
    m_exchange.Write( info );

    for ( UInt32 i=0; i<3; ++i )
    {
        if ( i == m_beginIndex )
            m_stale[ i ].Clear();
        else
            m_stale[ i ].Unite( changed );
    }
    m_lastTokenDirty = forReader;
    m_lastWidth = m_beginWidth;
    m_lastHeight = m_beginHeight;
    m_lastPublishedIndex = (Int32) m_beginIndex;
    ++m_published;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFrameExchange::UpdateFromRgba( const IMAGE::CPixelMap& frame, const CWebViewRect& dirty, Float32 deviceScale )
{GUCEF_TRACE;

    if ( IMAGE::PSF_RGBA != frame.GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != frame.GetPixelComponentDataType() )
        return false;
    CWebViewRect region;
    IMAGE::CPixelMap& target = BeginFrame( frame.GetWidthInPixels(), frame.GetHeightInPixels(), region );
    CWebViewRect changed = dirty;
    changed.ClipTo( frame.GetWidthInPixels(), frame.GetHeightInPixels() );
    // copied separately when they are apart: a caret and an old corner make no full frame copy
    if ( region.IsEmpty() || changed.IsEmpty() || !region.Intersection( changed ).IsEmpty() )
    {
        region.Unite( changed );
        CopyRegion( frame, target, region );
    }
    else
    {
        CopyRegion( frame, target, region );
        CopyRegion( frame, target, changed );
    }
    return PublishFrame( dirty, deviceScale );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFrameExchange::UpdateFromBgraPremultiplied( const UInt8* pixels, UInt32 strideInBytes, UInt32 width, UInt32 height, const CWebViewRect& dirty, Float32 deviceScale )
{GUCEF_TRACE;

    if ( GUCEF_NULL == pixels || strideInBytes < width * 4 )
        return false;
    CWebViewRect region;
    IMAGE::CPixelMap& target = BeginFrame( width, height, region );
    CWebViewRect changed = dirty;
    changed.ClipTo( width, height );
    if ( region.IsEmpty() || changed.IsEmpty() || !region.Intersection( changed ).IsEmpty() )
    {
        region.Unite( changed );
        CopyRegionFromBgraPremultiplied( pixels, strideInBytes, target, region );
    }
    else
    {
        CopyRegionFromBgraPremultiplied( pixels, strideInBytes, target, region );
        CopyRegionFromBgraPremultiplied( pixels, strideInBytes, target, changed );
    }
    return PublishFrame( dirty, deviceScale );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFrameExchange::CopyLastPublished( IMAGE::CPixelMap& out ) const
{GUCEF_TRACE;

    // the writer never writes the buffer it published last until it comes back to it, so reading it here is safe
    if ( m_lastPublishedIndex < 0 || GUCEF_NULL == m_buffers[ m_lastPublishedIndex ] )
        return false;
    out = *m_buffers[ m_lastPublishedIndex ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFrameExchange::TakeLatest( CWebViewFrameInfo& info )
{GUCEF_TRACE;

    const bool isNew = m_exchange.Read( m_readerInfo );
    if ( isNew )
        m_readerHas = true;
    info = m_readerInfo;
    return isNew;
}

/*-------------------------------------------------------------------------*/

const IMAGE::CPixelMap*
CWebViewFrameExchange::GetReaderPixels( void ) const
{GUCEF_TRACE;

    return m_readerHas ? m_buffers[ m_readerInfo.bufferIndex ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFrameExchange::TakeLatestInto( IMAGE::CPixelMap& target, CWebViewFrameInfo& info, CWebViewRect& changed )
{GUCEF_TRACE;

    changed.Clear();
    if ( !TakeLatest( info ) )
        return false;
    const IMAGE::CPixelMap* source = GetReaderPixels();
    if ( GUCEF_NULL == source )
        return false;
    if ( target.GetWidthInPixels() != source->GetWidthInPixels() || target.GetHeightInPixels() != source->GetHeightInPixels() ||
         IMAGE::PSF_RGBA != target.GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != target.GetPixelComponentDataType() )
    {
        target.Assign( GUCEF_NULL, source->GetWidthInPixels(), source->GetHeightInPixels(), IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        changed = CWebViewRect::Whole( source->GetWidthInPixels(), source->GetHeightInPixels() );
    }
    else
    {
        changed = info.dirty;
        changed.ClipTo( source->GetWidthInPixels(), source->GetHeightInPixels() );
    }
    if ( changed.IsEmpty() )
        return false;
    CopyRegion( *source, target, changed );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFrameExchange::HasNewFrame( void ) const
{GUCEF_TRACE;

    return m_exchange.HasNewData();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFrameExchange::HasFrame( void ) const
{GUCEF_TRACE;

    return m_exchange.HasData();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewFrameExchange::GetPublishedCount( void ) const
{GUCEF_TRACE;

    return m_published.Load();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewFrameExchange::GetReplacedCount( void ) const
{GUCEF_TRACE;

    return m_replaced.Load();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewFrameExchange::GetAllocationCount( void ) const
{GUCEF_TRACE;

    return m_allocations.Load();
}

/*-------------------------------------------------------------------------*/

void
CWebViewFrameExchange::CopyRegion( const IMAGE::CPixelMap& source, IMAGE::CPixelMap& target, const CWebViewRect& region )
{GUCEF_TRACE;

    CWebViewRect r = region;
    if ( !r.ClipTo( source.GetWidthInPixels(), source.GetHeightInPixels() ) || !r.ClipTo( target.GetWidthInPixels(), target.GetHeightInPixels() ) )
        return;
    const UInt32 sourceStride = source.GetWidthInPixels() * 4;
    const UInt32 targetStride = target.GetWidthInPixels() * 4;
    const UInt8* from = static_cast< const UInt8* >( source.GetDataPtr() ) + (size_t) r.top * sourceStride + (size_t) r.left * 4;
    UInt8* to = static_cast< UInt8* >( target.GetDataPtr() ) + (size_t) r.top * targetStride + (size_t) r.left * 4;
    const size_t rowBytes = (size_t) r.width * 4;
    for ( Int32 row=0; row<r.height; ++row )
    {
        memcpy( to, from, rowBytes );
        from += sourceStride;
        to += targetStride;
    }
}

/*-------------------------------------------------------------------------*/

void
CWebViewFrameExchange::CopyRegionFromBgraPremultiplied( const UInt8* source, UInt32 strideInBytes, IMAGE::CPixelMap& target, const CWebViewRect& region )
{GUCEF_TRACE;

    CWebViewRect r = region;
    if ( GUCEF_NULL == source || !r.ClipTo( target.GetWidthInPixels(), target.GetHeightInPixels() ) )
        return;
    const UInt32 targetStride = target.GetWidthInPixels() * 4;
    for ( Int32 row=0; row<r.height; ++row )
    {
        const UInt8* from = source + (size_t) ( r.top + row ) * strideInBytes + (size_t) r.left * 4;
        UInt8* to = static_cast< UInt8* >( target.GetDataPtr() ) + (size_t) ( r.top + row ) * targetStride + (size_t) r.left * 4;
        for ( Int32 column=0; column<r.width; ++column, from+=4, to+=4 )
        {
            const UInt32 a = from[ 3 ];
            if ( 255 == a )
            {
                to[ 0 ] = from[ 2 ];
                to[ 1 ] = from[ 1 ];
                to[ 2 ] = from[ 0 ];
            }
            else
            if ( 0 == a )
            {
                to[ 0 ] = to[ 1 ] = to[ 2 ] = 0;
            }
            else
            {
                // undo the premultiplication, rounded
                to[ 0 ] = (UInt8) ( ( from[ 2 ] * 255u + a / 2 ) / a > 255u ? 255u : ( from[ 2 ] * 255u + a / 2 ) / a );
                to[ 1 ] = (UInt8) ( ( from[ 1 ] * 255u + a / 2 ) / a > 255u ? 255u : ( from[ 1 ] * 255u + a / 2 ) / a );
                to[ 2 ] = (UInt8) ( ( from[ 0 ] * 255u + a / 2 ) / a > 255u ? 255u : ( from[ 0 ] * 255u + a / 2 ) / a );
            }
            to[ 3 ] = (UInt8) a;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
