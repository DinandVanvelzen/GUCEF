/*
 *  assetpluginUNREAL: Reverses the row prediction of texture sources stored as TSCF_UEDELTA
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H
#include "assetpluginUNREAL_CUnrealDeltaTextureDecoder.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CUnrealDeltaTextureDecoder::BytesPerPixel( const CORE::CString& format )
{GUCEF_TRACE;

    if ( "TSF_G8" == format )
        return 1;
    if ( "TSF_BGRA8" == format )
        return 4;
    if ( "TSF_RGBA16" == format )
        return 8;
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealDeltaTextureDecoder::StripRows( const CORE::CString& format, UInt32 width, UInt32 height, UInt32& rows, CORE::CString& error )
{GUCEF_TRACE;

    rows = 0;
    error.Clear();
    const UInt32 bytesPerPixel = BytesPerPixel( format );
    if ( 0 == bytesPerPixel )
    {
        error = "The TSCF_UEDELTA layout of the format " + format + " is not established (only TSF_BGRA8, TSF_G8 and TSF_RGBA16 occur in the sources this reader was verified on)";
        return false;
    }
    if ( 0 == width || 0 == height || width > MaxDimension || height > MaxDimension )
    {
        error = "The TSCF_UEDELTA source has sizes that are not valid: " + CORE::UInt32ToString( width ) + " x " + CORE::UInt32ToString( height );
        return false;
    }

    // the minimum rows per strip: 8 per byte of a pixel, seen for TSF_BGRA8 (32) and TSF_RGBA16 (64); for TSF_G8 only an upper bound is
    // known (the pixel count gave 16 rows at width 2048), so a TSF_G8 source whose strips the minimum would decide is refused
    const UInt32 minimum = 8 * bytesPerPixel;
    const UInt32 knownMinimumUpTo = 1 == bytesPerPixel ? 16 : minimum;
    const UInt32 byPixelsFloor = StripPixels / width;
    const UInt32 byPixelsCeil = ( StripPixels + width - 1 ) / width;
    const UInt32 low = byPixelsFloor > minimum ? byPixelsFloor : minimum;
    const UInt32 high = byPixelsCeil > minimum ? byPixelsCeil : minimum;
    if ( low != high && height > low )
    {
        error = "The TSCF_UEDELTA strip height of a source " + CORE::UInt32ToString( width ) + " pixels wide is not established (the width does not divide " +
                CORE::UInt32ToString( (UInt32) StripPixels ) + "; the sources this reader was verified on all have widths that do)";
        return false;
    }
    if ( minimum < knownMinimumUpTo && byPixelsFloor < knownMinimumUpTo && height > byPixelsFloor )
    {
        error = "The TSCF_UEDELTA strip height of a " + format + " source " + CORE::UInt32ToString( width ) + " pixels wide is not established (it depends on a minimum row count of the format that was not observed)";
        return false;
    }
    rows = low;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealDeltaTextureDecoder::Decode( const CORE::CString& format, UInt32 width, UInt32 height, UInt8* data, UInt64 size, CORE::CString& error )
{GUCEF_TRACE;

    UInt32 rows = 0;
    if ( !StripRows( format, width, height, rows, error ) )
        return false;
    const UInt32 bytesPerPixel = BytesPerPixel( format );
    const UInt64 pitch = (UInt64) width * bytesPerPixel;
    const UInt64 needed = pitch * height;
    if ( GUCEF_NULL == data || size < needed )
    {
        error = "The TSCF_UEDELTA source holds " + CORE::UInt64ToString( GUCEF_NULL == data ? 0 : size ) + " bytes where " + CORE::UInt64ToString( needed ) + " are needed for the first image";
        return false;
    }

    for ( UInt32 y=1; y<height; ++y )
    {
        if ( 0 == y % rows )
            continue;   // the first row of a strip holds its pixels as they are
        UInt8* row = data + pitch * y;
        const UInt8* above = row - pitch;
        if ( 8 == bytesPerPixel )
        {
            // little-endian 16 bit samples, the difference stored plus 0x8080
            for ( UInt64 i=0; i<pitch; i+=2 )
            {
                const UInt32 stored = (UInt32) row[ i ] | ( (UInt32) row[ i + 1 ] << 8 );
                const UInt32 prior = (UInt32) above[ i ] | ( (UInt32) above[ i + 1 ] << 8 );
                const UInt32 value = ( stored + prior + 0x10000 - 0x8080 ) & 0xFFFF;
                row[ i ] = (UInt8) ( value & 0xFF );
                row[ i + 1 ] = (UInt8) ( value >> 8 );
            }
        }
        else
        {
            for ( UInt64 i=0; i<pitch; ++i )
                row[ i ] = (UInt8) ( row[ i ] + above[ i ] );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
