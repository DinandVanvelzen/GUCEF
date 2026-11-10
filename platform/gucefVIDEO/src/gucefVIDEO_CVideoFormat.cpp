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

#include "gucefVIDEO_CVideoFormat.h"

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

CVideoFormat::CVideoFormat( void )
    : m_width( 0 )
    , m_height( 0 )
    , m_pixelFormat( IMAGE::PSF_UNKNOWN )
    , m_frameRateNumerator( 0 )
    , m_frameRateDenominator( 1 )
    , m_frameCount( GUCEF_VIDEO_FRAMECOUNT_UNKNOWN )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoFormat::CVideoFormat( UInt32 width                            ,
                            UInt32 height                           ,
                            IMAGE::TPixelStorageFormat pixelFormat  ,
                            UInt32 frameRateNumerator               ,
                            UInt32 frameRateDenominator             ,
                            UInt64 frameCount                       )
    : m_width( width )
    , m_height( height )
    , m_pixelFormat( pixelFormat )
    , m_frameRateNumerator( frameRateNumerator )
    , m_frameRateDenominator( 0 == frameRateDenominator ? 1 : frameRateDenominator )
    , m_frameCount( frameCount )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CVideoFormat::GetWidth( void ) const
{GUCEF_TRACE;

    return m_width;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoFormat::GetHeight( void ) const
{GUCEF_TRACE;

    return m_height;
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelStorageFormat
CVideoFormat::GetPixelFormat( void ) const
{GUCEF_TRACE;

    return m_pixelFormat;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoFormat::GetFrameRateNumerator( void ) const
{GUCEF_TRACE;

    return m_frameRateNumerator;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoFormat::GetFrameRateDenominator( void ) const
{GUCEF_TRACE;

    return m_frameRateDenominator;
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoFormat::GetFrameCount( void ) const
{GUCEF_TRACE;

    return m_frameCount;
}

/*-------------------------------------------------------------------------*/

void
CVideoFormat::SetFrameCount( UInt64 frameCount )
{GUCEF_TRACE;

    m_frameCount = frameCount;
}

/*-------------------------------------------------------------------------*/

Float64
CVideoFormat::GetFrameRate( void ) const
{GUCEF_TRACE;

    return (Float64) m_frameRateNumerator / (Float64) m_frameRateDenominator;
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoFormat::GetFrameTimeInMicroSeconds( UInt64 frameIndex ) const
{GUCEF_TRACE;

    if ( 0 == m_frameRateNumerator )
        return 0;
    // frame / ( num / den ) seconds, in integers so long videos do not drift
    return frameIndex * (UInt64) m_frameRateDenominator * 1000000 / (UInt64) m_frameRateNumerator;
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoFormat::GetFrameAtTime( UInt64 microSeconds ) const
{GUCEF_TRACE;

    if ( 0 == m_frameRateNumerator )
        return 0;
    return microSeconds * (UInt64) m_frameRateNumerator / ( (UInt64) m_frameRateDenominator * 1000000 );
}

/*-------------------------------------------------------------------------*/

UInt64
CVideoFormat::GetDurationInMs( void ) const
{GUCEF_TRACE;

    if ( GUCEF_VIDEO_FRAMECOUNT_UNKNOWN == m_frameCount )
        return 0;
    return GetFrameTimeInMicroSeconds( m_frameCount ) / 1000;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoFormat::GetBytesPerPixel( void ) const
{GUCEF_TRACE;

    switch ( m_pixelFormat )
    {
        case IMAGE::PSF_PALETTE_INDICES: return 1;
        case IMAGE::PSF_RGB: return 3;
        case IMAGE::PSF_RGBA:
        case IMAGE::PSF_BGRA: return 4;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

bool
CVideoFormat::IsPalettized( void ) const
{GUCEF_TRACE;

    return IMAGE::PSF_PALETTE_INDICES == m_pixelFormat;
}

/*-------------------------------------------------------------------------*/

bool
CVideoFormat::IsValid( void ) const
{GUCEF_TRACE;

    return m_width > 0 && m_height > 0 && m_frameRateNumerator > 0 && GetBytesPerPixel() > 0;
}

/*-------------------------------------------------------------------------*/

bool
CVideoFormat::operator==( const CVideoFormat& other ) const
{GUCEF_TRACE;

    return m_width == other.m_width && m_height == other.m_height && m_pixelFormat == other.m_pixelFormat &&
           m_frameRateNumerator == other.m_frameRateNumerator && m_frameRateDenominator == other.m_frameRateDenominator &&
           m_frameCount == other.m_frameCount;
}

/*-------------------------------------------------------------------------*/

bool
CVideoFormat::operator!=( const CVideoFormat& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
