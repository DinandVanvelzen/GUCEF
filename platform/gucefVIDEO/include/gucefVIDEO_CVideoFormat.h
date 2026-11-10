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

#ifndef GUCEF_VIDEO_CVIDEOFORMAT_H
#define GUCEF_VIDEO_CVIDEOFORMAT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_IMAGEDATA_H
#include "gucefIMAGE_c_imagedata.h"     /* TPixelStorageFormat */
#define GUCEF_IMAGE_IMAGEDATA_H
#endif /* GUCEF_IMAGE_IMAGEDATA_H ? */

#ifndef GUCEF_VIDEO_MACROS_H
#include "gucefVIDEO_macros.h"
#define GUCEF_VIDEO_MACROS_H
#endif /* GUCEF_VIDEO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the pictures of a video are and how fast they come: size, pixel format, frame rate and, when known,
 *  the number of frames. The pixel format is one of IMAGE::PSF_PALETTE_INDICES (8 bit indices with a palette of
 *  256 RGB entries), IMAGE::PSF_RGB, IMAGE::PSF_RGBA or IMAGE::PSF_BGRA, with 8 bit components.
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoFormat
{
    public:

    CVideoFormat( void );

    CVideoFormat( UInt32 width                                  ,
                  UInt32 height                                 ,
                  IMAGE::TPixelStorageFormat pixelFormat        ,
                  UInt32 frameRateNumerator                     ,
                  UInt32 frameRateDenominator                   ,
                  UInt64 frameCount = GUCEF_VIDEO_FRAMECOUNT_UNKNOWN );

    UInt32 GetWidth( void ) const;
    UInt32 GetHeight( void ) const;
    IMAGE::TPixelStorageFormat GetPixelFormat( void ) const;

    /** Frames per second as the fraction numerator / denominator */
    UInt32 GetFrameRateNumerator( void ) const;
    UInt32 GetFrameRateDenominator( void ) const;
    Float64 GetFrameRate( void ) const;

    /** GUCEF_VIDEO_FRAMECOUNT_UNKNOWN when the video does not tell */
    UInt64 GetFrameCount( void ) const;
    void SetFrameCount( UInt64 frameCount );

    /** When frame 'frameIndex' is shown, in microseconds from the start */
    UInt64 GetFrameTimeInMicroSeconds( UInt64 frameIndex ) const;

    /** The frame that is shown at the given time */
    UInt64 GetFrameAtTime( UInt64 microSeconds ) const;

    /** Length in milliseconds, 0 when the frame count is not known */
    UInt64 GetDurationInMs( void ) const;

    /** Bytes per pixel of the pixel format: 1, 3 or 4; 0 for a format a video cannot have */
    UInt32 GetBytesPerPixel( void ) const;

    bool IsPalettized( void ) const;

    /** A size above zero, a frame rate above zero and a pixel format a video can have */
    bool IsValid( void ) const;

    bool operator==( const CVideoFormat& other ) const;
    bool operator!=( const CVideoFormat& other ) const;

    private:

    UInt32 m_width;
    UInt32 m_height;
    IMAGE::TPixelStorageFormat m_pixelFormat;
    UInt32 m_frameRateNumerator;
    UInt32 m_frameRateDenominator;
    UInt64 m_frameCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEOFORMAT_H ? */
