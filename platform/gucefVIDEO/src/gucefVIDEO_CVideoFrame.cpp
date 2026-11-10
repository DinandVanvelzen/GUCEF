/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#include "gucefVIDEO_CVideoFrame.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVideoFrame::CVideoFrame( void )
    : m_picture()
    , m_paletteChanged( false )
    , m_frameIndex( 0 )
    , m_timeInMicroSeconds( 0 )
{GUCEF_TRACE;

    memset( m_palette, 0, sizeof( m_palette ) );
}

/*-------------------------------------------------------------------------*/

CVideoFrame::CVideoFrame( const CVideoFrame& src )
    : m_picture()
    , m_paletteChanged( src.m_paletteChanged )
    , m_frameIndex( src.m_frameIndex )
    , m_timeInMicroSeconds( src.m_timeInMicroSeconds )
{GUCEF_TRACE;

    memcpy( m_palette, src.m_palette, sizeof( m_palette ) );
    if ( !src.m_picture.IsNULL() )
        m_picture = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( *src.m_picture ) );
}

/*-------------------------------------------------------------------------*/

CVideoFrame&
CVideoFrame::operator=( const CVideoFrame& src )
{GUCEF_TRACE;

    if ( this == &src )
        return *this;

    memcpy( m_palette, src.m_palette, sizeof( m_palette ) );
    m_paletteChanged = src.m_paletteChanged;
    m_frameIndex = src.m_frameIndex;
    m_timeInMicroSeconds = src.m_timeInMicroSeconds;

    if ( src.m_picture.IsNULL() )
    {
        m_picture.Unlink();
    }
    else
    if ( !m_picture.IsNULL() && m_picture->GetTotalSizeInBytes() == src.m_picture->GetTotalSizeInBytes() &&
         m_picture->GetPixelStorageFormat() == src.m_picture->GetPixelStorageFormat() &&
         m_picture->GetWidthInPixels() == src.m_picture->GetWidthInPixels() )
    {
        // the same shape: copy the pixels, keep the memory
        memcpy( m_picture->GetDataPtr(), src.m_picture->GetDataPtr(), src.m_picture->GetTotalSizeInBytes() );
    }
    else
    {
        m_picture = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( *src.m_picture ) );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVideoFrame::~CVideoFrame()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVideoFrame::Prepare( const CVideoFormat& format )
{GUCEF_TRACE;

    if ( !format.IsValid() )
        return false;

    if ( !m_picture.IsNULL() && m_picture->GetWidthInPixels() == format.GetWidth() &&
         m_picture->GetHeightInPixels() == format.GetHeight() && m_picture->GetPixelStorageFormat() == format.GetPixelFormat() )
        return true;

    m_picture = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, format.GetWidth(), format.GetHeight(), format.GetPixelFormat(), GUCEF::DATATYPE_UINT8 ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoFrame::HasPicture( void ) const
{GUCEF_TRACE;

    return !m_picture.IsNULL();
}

/*-------------------------------------------------------------------------*/

IMAGE::CPixelMap&
CVideoFrame::GetPicture( void )
{GUCEF_TRACE;

    return *m_picture;
}

/*-------------------------------------------------------------------------*/

const IMAGE::CPixelMap&
CVideoFrame::GetPicture( void ) const
{GUCEF_TRACE;

    return *m_picture;
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
CVideoFrame::GetPicturePtr( void ) const
{GUCEF_TRACE;

    return m_picture;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoFrame::GetWidth( void ) const
{GUCEF_TRACE;

    return m_picture.IsNULL() ? 0 : m_picture->GetWidthInPixels();
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoFrame::GetHeight( void ) const
{GUCEF_TRACE;

    return m_picture.IsNULL() ? 0 : m_picture->GetHeightInPixels();
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelStorageFormat
CVideoFrame::GetPixelFormat( void ) const
{GUCEF_TRACE;

    return m_picture.IsNULL() ? IMAGE::PSF_UNKNOWN : m_picture->GetPixelStorageFormat();
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoFrame::GetPitch( void ) const
{GUCEF_TRACE;

    return m_picture.IsNULL() ? 0 : m_picture->GetWidthInBytes();
}

/*-------------------------------------------------------------------------*/

UInt8*
CVideoFrame::GetPalette( void )
{GUCEF_TRACE;

    return m_palette;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CVideoFrame::GetPalette( void ) const
{GUCEF_TRACE;

    return m_palette;
}

/*-------------------------------------------------------------------------*/

bool
CVideoFrame::IsPaletteChanged( void ) const
{GUCEF_TRACE;

    return m_paletteChanged;
}

/*-------------------------------------------------------------------------*/

void
CVideoFrame::SetPaletteChanged( bool changed )
{GUCEF_TRACE;

    m_paletteChanged = changed;
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoFrame::GetFrameIndex( void ) const
{GUCEF_TRACE;

    return m_frameIndex;
}

/*-------------------------------------------------------------------------*/

void
CVideoFrame::SetFrameIndex( UInt64 frameIndex )
{GUCEF_TRACE;

    m_frameIndex = frameIndex;
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoFrame::GetTimeInMicroSeconds( void ) const
{GUCEF_TRACE;

    return m_timeInMicroSeconds;
}

/*-------------------------------------------------------------------------*/

void
CVideoFrame::SetTimeInMicroSeconds( UInt64 microSeconds )
{GUCEF_TRACE;

    m_timeInMicroSeconds = microSeconds;
}

/*-------------------------------------------------------------------------*/

bool
CVideoFrame::ConvertTo32Bit( IMAGE::TPixelStorageFormat targetFormat, void* destination, UInt32 destinationPitch ) const
{GUCEF_TRACE;

    if ( m_picture.IsNULL() || GUCEF_NULL == destination || ( IMAGE::PSF_RGBA != targetFormat && IMAGE::PSF_BGRA != targetFormat ) )
        return false;

    const UInt32 width = m_picture->GetWidthInPixels();
    const UInt32 height = m_picture->GetHeightInPixels();
    if ( destinationPitch < width * 4 )
        return false;

    // where R and B go in the target
    const UInt32 r = IMAGE::PSF_RGBA == targetFormat ? 0 : 2;
    const UInt32 b = 2 - r;
    const IMAGE::TPixelStorageFormat sourceFormat = m_picture->GetPixelStorageFormat();
    const UInt8* in = static_cast< const UInt8* >( m_picture->GetDataPtr() );
    const UInt32 inPitch = m_picture->GetWidthInBytes();

    for ( UInt32 y=0; y<height; ++y )
    {
        const UInt8* src = in + (size_t) y * inPitch;
        UInt8* dst = static_cast< UInt8* >( destination ) + (size_t) y * destinationPitch;
        switch ( sourceFormat )
        {
            case IMAGE::PSF_PALETTE_INDICES:
            {
                for ( UInt32 x=0; x<width; ++x, dst+=4 )
                {
                    const UInt8* colour = m_palette + 3 * (UInt32) src[ x ];
                    dst[ r ] = colour[ 0 ];
                    dst[ 1 ] = colour[ 1 ];
                    dst[ b ] = colour[ 2 ];
                    dst[ 3 ] = 255;
                }
                break;
            }
            case IMAGE::PSF_RGB:
            {
                for ( UInt32 x=0; x<width; ++x, src+=3, dst+=4 )
                {
                    dst[ r ] = src[ 0 ];
                    dst[ 1 ] = src[ 1 ];
                    dst[ b ] = src[ 2 ];
                    dst[ 3 ] = 255;
                }
                break;
            }
            case IMAGE::PSF_RGBA:
            case IMAGE::PSF_BGRA:
            {
                if ( sourceFormat == targetFormat )
                {
                    memcpy( dst, src, (size_t) width * 4 );
                }
                else
                {
                    for ( UInt32 x=0; x<width; ++x, src+=4, dst+=4 )
                    {
                        dst[ 0 ] = src[ 2 ];
                        dst[ 1 ] = src[ 1 ];
                        dst[ 2 ] = src[ 0 ];
                        dst[ 3 ] = src[ 3 ];
                    }
                }
                break;
            }
            default:
            {
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoFrame::ConvertTo32Bit( IMAGE::TPixelStorageFormat targetFormat, IMAGE::TPixelMapPtr& destination ) const
{GUCEF_TRACE;

    if ( m_picture.IsNULL() || ( IMAGE::PSF_RGBA != targetFormat && IMAGE::PSF_BGRA != targetFormat ) )
        return false;

    if ( destination.IsNULL() || destination->GetWidthInPixels() != GetWidth() || destination->GetHeightInPixels() != GetHeight() ||
         destination->GetPixelStorageFormat() != targetFormat || destination->GetPixelComponentDataType() != GUCEF::DATATYPE_UINT8 )
    {
        destination = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, GetWidth(), GetHeight(), targetFormat, GUCEF::DATATYPE_UINT8 ) );
    }
    return ConvertTo32Bit( targetFormat, destination->GetDataPtr(), destination->GetWidthInBytes() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
