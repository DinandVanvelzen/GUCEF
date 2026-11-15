/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#ifndef GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H
#define GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads and writes the channel values of a pixel map as Float32, whatever the storage format and data type of
 *  the map are, for the image processing classes (resampler, mip chain generator, channel operations).
 *
 *  The Float32 values are normalised: unsigned integers map 0 .. max to 0 .. 1, signed integers map
 *  -max .. max to -1 .. 1 and floating point values are taken as they are. Writing rounds and clamps integer
 *  values to the range of the data type; floating point values are not clamped. Pixel maps of palette indices,
 *  of unknown formats and of big endian data types are not supported.
 */
class GUCEF_IMAGE_EXPORT_CPP CPixelMapChannelAccess
{
    public:

    typedef GUCEF::vector< Float32 > TFloatVector;

    /** True when the values of a pixel map of this format and data type can be read and written here */
    static bool IsSupported( TPixelStorageFormat format, TBuildinDataType dataType );

    static bool IsSupported( const CPixelMap& map );

    /** The number of colour channels at the start of a pixel: 3 for RGB, BGR, HSV and HSL formats (with or without alpha), 0 for the others */
    static UInt32 GetColourChannelCount( TPixelStorageFormat format );

    /** The index of the alpha channel in the storage order of the format, -1 when there is none */
    static Int32 GetAlphaChannelIndex( TPixelStorageFormat format );

    /** True for the HSV and HSL formats, whose first three channels are not a red, green and blue */
    static bool IsHueBased( TPixelStorageFormat format );

    /** True for the integer data types that can hold negative values */
    static bool IsSignedInteger( TBuildinDataType dataType );

    /** True for the floating point data types */
    static bool IsFloatingPoint( TBuildinDataType dataType );

    /** The value at 'valuePtr', which holds a value of the data type */
    static Float32 ReadValue( const void* valuePtr, TBuildinDataType dataType );

    /** Writes 'value' as a value of the data type at 'valuePtr'; integer types round and clamp */
    static void WriteValue( Float32 value, TBuildinDataType dataType, void* valuePtr );

    /** All channel values of the map, pixel by pixel in storage order: pixel count times channel count values */
    static bool Read( const CPixelMap& map, TFloatVector& values );

    /** Writes the values back as read by Read(); 'values' must hold the right number of them for the map */
    static bool Write( const Float32* values, CPixelMap& map );

    /** The exact transfer function of the sRGB standard; negative values mirror the positive side */
    static Float32 SrgbToLinear( Float32 value );

    static Float32 LinearToSrgb( Float32 value );

    /**
     *  Changes read values of a format to the working space of filtering: HSV and HSL become RGB and when
     *  'storedSpace' is sRGB the colour channels become linear. 'values' keeps the channel count per pixel.
     */
    static void ToWorkingSpace( Float32* values, UInt32 pixelCount, TPixelStorageFormat format, TColourSpace storedSpace );

    /** The opposite of ToWorkingSpace() */
    static void FromWorkingSpace( Float32* values, UInt32 pixelCount, TPixelStorageFormat format, TColourSpace storedSpace );

    /** Converts the colour channels of read values from one colour space to the other, for formats that are not hue based */
    static void ConvertColourSpace( Float32* values, UInt32 pixelCount, TPixelStorageFormat format, TColourSpace from, TColourSpace to );

    private:

    CPixelMapChannelAccess( void );
    ~CPixelMapChannelAccess();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H ? */
