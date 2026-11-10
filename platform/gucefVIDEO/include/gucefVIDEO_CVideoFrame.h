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

#ifndef GUCEF_VIDEO_CVIDEOFRAME_H
#define GUCEF_VIDEO_CVIDEOFRAME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_VIDEO_CVIDEOFORMAT_H
#include "gucefVIDEO_CVideoFormat.h"
#define GUCEF_VIDEO_CVIDEOFORMAT_H
#endif /* GUCEF_VIDEO_CVIDEOFORMAT_H ? */

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
 *  One frame of a video: its number, when it is shown, the picture and, for palette videos, the palette.
 *  A frame is meant to be reused: Prepare() keeps the picture memory when the format stays the same, and
 *  decoders write into the picture that is there (formats with delta frames build on the previous picture).
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoFrame
{
    public:

    static const UInt32 PaletteEntryCount = 256;
    static const UInt32 PaletteSizeInBytes = 256 * 3;

    CVideoFrame( void );

    CVideoFrame( const CVideoFrame& src );

    CVideoFrame& operator=( const CVideoFrame& src );

    virtual ~CVideoFrame();

    /** Gives the frame a picture of the size and pixel format of the video. Keeps the picture when it already fits. False for an invalid format */
    bool Prepare( const CVideoFormat& format );

    bool HasPicture( void ) const;

    /** The picture: palette indices or pixels as the format says. Only valid after Prepare() */
    IMAGE::CPixelMap& GetPicture( void );
    const IMAGE::CPixelMap& GetPicture( void ) const;

    /** Shares the picture, for consumers that keep it */
    IMAGE::TPixelMapPtr GetPicturePtr( void ) const;

    UInt32 GetWidth( void ) const;
    UInt32 GetHeight( void ) const;
    IMAGE::TPixelStorageFormat GetPixelFormat( void ) const;

    /** Bytes from one line of the picture to the next */
    UInt32 GetPitch( void ) const;

    /** 256 entries of R, G, B; used when the picture holds palette indices */
    UInt8* GetPalette( void );
    const UInt8* GetPalette( void ) const;

    /** True when the palette changed with this frame */
    bool IsPaletteChanged( void ) const;
    void SetPaletteChanged( bool changed );

    UInt64 GetFrameIndex( void ) const;
    void SetFrameIndex( UInt64 frameIndex );

    /** When the frame is shown, in microseconds from the start of the video */
    UInt64 GetTimeInMicroSeconds( void ) const;
    void SetTimeInMicroSeconds( UInt64 microSeconds );

    /**
     *  Writes the picture as 32 bit pixels into memory of the caller: 'targetFormat' is IMAGE::PSF_RGBA or
     *  IMAGE::PSF_BGRA, 'destination' holds GetHeight() lines of 'destinationPitch' bytes. The palette is applied
     *  for palette pictures, alpha is 255. False for another target format or when there is no picture.
     */
    bool ConvertTo32Bit( IMAGE::TPixelStorageFormat targetFormat, void* destination, UInt32 destinationPitch ) const;

    /** The same into a pixel map, which is made (again) when it does not have the size and format already */
    bool ConvertTo32Bit( IMAGE::TPixelStorageFormat targetFormat, IMAGE::TPixelMapPtr& destination ) const;

    private:

    IMAGE::TPixelMapPtr m_picture;
    UInt8 m_palette[ PaletteSizeInBytes ];
    bool m_paletteChanged;
    UInt64 m_frameIndex;
    UInt64 m_timeInMicroSeconds;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEOFRAME_H ? */
