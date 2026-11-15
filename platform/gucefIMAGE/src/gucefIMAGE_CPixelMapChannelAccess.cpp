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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefIMAGE_CPixelMapChannelAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

template< typename T >
inline Float32
ReadUnsigned( const void* valuePtr )
{
    T value;
    memcpy( &value, valuePtr, sizeof( T ) );
    return (Float32) ( (Float64) value / (Float64) std::numeric_limits< T >::max() );
}

template< typename T >
inline Float32
ReadSigned( const void* valuePtr )
{
    T value;
    memcpy( &value, valuePtr, sizeof( T ) );
    Float64 normalised = (Float64) value / (Float64) std::numeric_limits< T >::max();
    return (Float32) ( normalised < -1.0 ? -1.0 : normalised );
}

template< typename T >
inline void
WriteUnsigned( Float32 value, void* valuePtr )
{
    T result;
    if ( !( value > 0.0f ) )
        result = 0;
    else
    if ( value >= 1.0f )
        result = std::numeric_limits< T >::max();
    else
        result = (T) ( (Float64) value * (Float64) std::numeric_limits< T >::max() + 0.5 );
    memcpy( valuePtr, &result, sizeof( T ) );
}

template< typename T >
inline void
WriteSigned( Float32 value, void* valuePtr )
{
    T result;
    if ( !( value > -1.0f ) )
        result = -std::numeric_limits< T >::max();
    else
    if ( value >= 1.0f )
        result = std::numeric_limits< T >::max();
    else
    {
        Float64 scaled = (Float64) value * (Float64) std::numeric_limits< T >::max();
        result = (T) ( scaled < 0.0 ? scaled - 0.5 : scaled + 0.5 );
    }
    memcpy( valuePtr, &result, sizeof( T ) );
}

/** h, s, v and l in 0 .. 1 */
void
HsvToRgb( Float32 h, Float32 s, Float32 v, Float32& r, Float32& g, Float32& b )
{
    h = h - (Float32) floor( h );
    Float32 h6 = h * 6.0f;
    Int32 sector = (Int32) h6;
    Float32 f = h6 - (Float32) sector;
    Float32 p = v * ( 1.0f - s );
    Float32 q = v * ( 1.0f - s * f );
    Float32 t = v * ( 1.0f - s * ( 1.0f - f ) );
    switch ( sector % 6 )
    {
        case 0 : r = v; g = t; b = p; break;
        case 1 : r = q; g = v; b = p; break;
        case 2 : r = p; g = v; b = t; break;
        case 3 : r = p; g = q; b = v; break;
        case 4 : r = t; g = p; b = v; break;
        default : r = v; g = p; b = q; break;
    }
}

void
RgbToHsv( Float32 r, Float32 g, Float32 b, Float32& h, Float32& s, Float32& v )
{
    Float32 maxValue = r > g ? r : g; maxValue = maxValue > b ? maxValue : b;
    Float32 minValue = r < g ? r : g; minValue = minValue < b ? minValue : b;
    Float32 delta = maxValue - minValue;
    v = maxValue;
    s = maxValue > 0.0f ? delta / maxValue : 0.0f;
    if ( delta <= 0.0f )
    {
        h = 0.0f;
        return;
    }
    if ( maxValue == r )
        h = ( g - b ) / delta;
    else
    if ( maxValue == g )
        h = 2.0f + ( b - r ) / delta;
    else
        h = 4.0f + ( r - g ) / delta;
    h /= 6.0f;
    if ( h < 0.0f )
        h += 1.0f;
}

void
HslToRgb( Float32 h, Float32 s, Float32 l, Float32& r, Float32& g, Float32& b )
{
    Float32 c = ( 1.0f - (Float32) fabs( 2.0f * l - 1.0f ) ) * s;
    Float32 minValue = l - c * 0.5f;
    // The same hue circle as HSV, with the chroma c in place of s * v
    Float32 v = minValue + c;
    Float32 sv = v > 0.0f ? c / v : 0.0f;
    HsvToRgb( h, sv, v, r, g, b );
}

void
RgbToHsl( Float32 r, Float32 g, Float32 b, Float32& h, Float32& s, Float32& l )
{
    Float32 maxValue = r > g ? r : g; maxValue = maxValue > b ? maxValue : b;
    Float32 minValue = r < g ? r : g; minValue = minValue < b ? minValue : b;
    Float32 hsvS, hsvV;
    RgbToHsv( r, g, b, h, hsvS, hsvV );
    l = ( maxValue + minValue ) * 0.5f;
    Float32 delta = maxValue - minValue;
    Float32 divider = 1.0f - (Float32) fabs( 2.0f * l - 1.0f );
    s = divider > 0.00001f ? delta / divider : 0.0f;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CPixelMapChannelAccess::IsSupported( TPixelStorageFormat format, TBuildinDataType dataType )
{GUCEF_TRACE;

    switch ( format )
    {
        case PSF_RGB : case PSF_RGBA : case PSF_BGR : case PSF_BGRA :
        case PSF_HSV : case PSF_HSVA : case PSF_HSL : case PSF_HSLA :
        case PSF_SINGLE_CHANNEL_GRAYSCALE : case PSF_SINGLE_CHANNEL_RED : case PSF_SINGLE_CHANNEL_GREEN :
        case PSF_SINGLE_CHANNEL_BLUE : case PSF_SINGLE_CHANNEL_ALPHA : case PSF_SINGLE_CHANNEL_STD_LUMINANCE :
        case PSF_SINGLE_CHANNEL_P1_LUMINANCE : case PSF_SINGLE_CHANNEL_P2_LUMINANCE : case PSF_LUMINANCE_ALPHA :
            break;
        default :
            return false;
    }
    switch ( dataType )
    {
        case DATATYPE_INT8 : case DATATYPE_UINT8 :
        case DATATYPE_LE_INT16 : case DATATYPE_LE_UINT16 :
        case DATATYPE_LE_INT32 : case DATATYPE_LE_UINT32 :
        case DATATYPE_LE_INT64 : case DATATYPE_LE_UINT64 :
        case DATATYPE_LE_FLOAT32 : case DATATYPE_LE_FLOAT64 :
            return true;
        default :
            return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPixelMapChannelAccess::IsSupported( const CPixelMap& map )
{GUCEF_TRACE;

    return map.GetWidthInPixels() > 0 && map.GetHeightInPixels() > 0 && GUCEF_NULL != map.GetDataPtr() &&
           IsSupported( map.GetPixelStorageFormat(), map.GetPixelComponentDataType() );
}

/*-------------------------------------------------------------------------*/

UInt32
CPixelMapChannelAccess::GetColourChannelCount( TPixelStorageFormat format )
{GUCEF_TRACE;

    switch ( format )
    {
        case PSF_RGB : case PSF_RGBA : case PSF_BGR : case PSF_BGRA :
        case PSF_HSV : case PSF_HSVA : case PSF_HSL : case PSF_HSLA :
            return 3;
        default :
            return 0;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CPixelMapChannelAccess::GetAlphaChannelIndex( TPixelStorageFormat format )
{GUCEF_TRACE;

    switch ( format )
    {
        case PSF_RGBA : case PSF_BGRA : case PSF_HSVA : case PSF_HSLA :
            return 3;
        case PSF_SINGLE_CHANNEL_ALPHA :
            return 0;
        default :
            return -1;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPixelMapChannelAccess::IsHueBased( TPixelStorageFormat format )
{GUCEF_TRACE;

    return PSF_HSV == format || PSF_HSVA == format || PSF_HSL == format || PSF_HSLA == format;
}

/*-------------------------------------------------------------------------*/

bool
CPixelMapChannelAccess::IsSignedInteger( TBuildinDataType dataType )
{GUCEF_TRACE;

    return DATATYPE_INT8 == dataType || DATATYPE_LE_INT16 == dataType || DATATYPE_LE_INT32 == dataType || DATATYPE_LE_INT64 == dataType;
}

/*-------------------------------------------------------------------------*/

bool
CPixelMapChannelAccess::IsFloatingPoint( TBuildinDataType dataType )
{GUCEF_TRACE;

    return DATATYPE_LE_FLOAT32 == dataType || DATATYPE_LE_FLOAT64 == dataType;
}

/*-------------------------------------------------------------------------*/

Float32
CPixelMapChannelAccess::ReadValue( const void* valuePtr, TBuildinDataType dataType )
{GUCEF_TRACE;

    switch ( dataType )
    {
        case DATATYPE_UINT8 : return ReadUnsigned< UInt8 >( valuePtr );
        case DATATYPE_INT8 : return ReadSigned< Int8 >( valuePtr );
        case DATATYPE_LE_UINT16 : return ReadUnsigned< UInt16 >( valuePtr );
        case DATATYPE_LE_INT16 : return ReadSigned< Int16 >( valuePtr );
        case DATATYPE_LE_UINT32 : return ReadUnsigned< UInt32 >( valuePtr );
        case DATATYPE_LE_INT32 : return ReadSigned< Int32 >( valuePtr );
        case DATATYPE_LE_UINT64 : return ReadUnsigned< UInt64 >( valuePtr );
        case DATATYPE_LE_INT64 : return ReadSigned< Int64 >( valuePtr );
        case DATATYPE_LE_FLOAT32 : { Float32 value; memcpy( &value, valuePtr, sizeof( value ) ); return value; }
        case DATATYPE_LE_FLOAT64 : { Float64 value; memcpy( &value, valuePtr, sizeof( value ) ); return (Float32) value; }
        default : return 0.0f;
    }
}

/*-------------------------------------------------------------------------*/

void
CPixelMapChannelAccess::WriteValue( Float32 value, TBuildinDataType dataType, void* valuePtr )
{GUCEF_TRACE;

    switch ( dataType )
    {
        case DATATYPE_UINT8 : WriteUnsigned< UInt8 >( value, valuePtr ); break;
        case DATATYPE_INT8 : WriteSigned< Int8 >( value, valuePtr ); break;
        case DATATYPE_LE_UINT16 : WriteUnsigned< UInt16 >( value, valuePtr ); break;
        case DATATYPE_LE_INT16 : WriteSigned< Int16 >( value, valuePtr ); break;
        case DATATYPE_LE_UINT32 : WriteUnsigned< UInt32 >( value, valuePtr ); break;
        case DATATYPE_LE_INT32 : WriteSigned< Int32 >( value, valuePtr ); break;
        case DATATYPE_LE_UINT64 : WriteUnsigned< UInt64 >( value, valuePtr ); break;
        case DATATYPE_LE_INT64 : WriteSigned< Int64 >( value, valuePtr ); break;
        case DATATYPE_LE_FLOAT32 : memcpy( valuePtr, &value, sizeof( value ) ); break;
        case DATATYPE_LE_FLOAT64 : { Float64 wide = value; memcpy( valuePtr, &wide, sizeof( wide ) ); break; }
        default : break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPixelMapChannelAccess::Read( const CPixelMap& map, TFloatVector& values )
{GUCEF_TRACE;

    if ( !IsSupported( map ) )
        return false;

    UInt64 valueCount = map.GetTotalNumberOfChannelComponentValues();
    values.resize( (size_t) valueCount );
    const UInt8* source = static_cast< const UInt8* >( map.GetDataPtr() );
    TBuildinDataType dataType = map.GetPixelComponentDataType();
    UInt32 valueSize = CPixelMap::GetPixelChannelSize( dataType );
    Float32* target = values.empty() ? GUCEF_NULL : &values[ 0 ];

    if ( DATATYPE_UINT8 == dataType )
    {
        for ( UInt64 i=0; i<valueCount; ++i )
            target[ i ] = (Float32) source[ i ] * ( 1.0f / 255.0f );
    }
    else
    if ( DATATYPE_LE_FLOAT32 == dataType )
    {
        if ( valueCount > 0 )
            memcpy( target, source, (size_t) valueCount * sizeof( Float32 ) );
    }
    else
    {
        for ( UInt64 i=0; i<valueCount; ++i )
            target[ i ] = ReadValue( source + i * valueSize, dataType );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPixelMapChannelAccess::Write( const Float32* values, CPixelMap& map )
{GUCEF_TRACE;

    if ( GUCEF_NULL == values || !IsSupported( map ) )
        return false;

    UInt64 valueCount = map.GetTotalNumberOfChannelComponentValues();
    UInt8* target = static_cast< UInt8* >( map.GetDataPtr() );
    TBuildinDataType dataType = map.GetPixelComponentDataType();
    UInt32 valueSize = CPixelMap::GetPixelChannelSize( dataType );

    if ( DATATYPE_LE_FLOAT32 == dataType )
    {
        memcpy( target, values, (size_t) valueCount * sizeof( Float32 ) );
    }
    else
    if ( DATATYPE_UINT8 == dataType )
    {
        for ( UInt64 i=0; i<valueCount; ++i )
        {
            Float32 value = values[ i ];
            target[ i ] = value > 0.0f ? ( value >= 1.0f ? (UInt8) 255 : (UInt8) ( value * 255.0f + 0.5f ) ) : (UInt8) 0;
        }
    }
    else
    {
        for ( UInt64 i=0; i<valueCount; ++i )
            WriteValue( values[ i ], dataType, target + i * valueSize );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Float32
CPixelMapChannelAccess::SrgbToLinear( Float32 value )
{GUCEF_TRACE;

    Float32 magnitude = value < 0.0f ? -value : value;
    Float32 result = magnitude <= 0.04045f ? magnitude / 12.92f : (Float32) pow( ( (Float64) magnitude + 0.055 ) / 1.055, 2.4 );
    return value < 0.0f ? -result : result;
}

/*-------------------------------------------------------------------------*/

Float32
CPixelMapChannelAccess::LinearToSrgb( Float32 value )
{GUCEF_TRACE;

    Float32 magnitude = value < 0.0f ? -value : value;
    Float32 result = magnitude <= 0.0031308f ? magnitude * 12.92f : (Float32) ( 1.055 * pow( (Float64) magnitude, 1.0 / 2.4 ) - 0.055 );
    return value < 0.0f ? -result : result;
}

/*-------------------------------------------------------------------------*/

void
CPixelMapChannelAccess::ConvertColourSpace( Float32* values, UInt32 pixelCount, TPixelStorageFormat format, TColourSpace from, TColourSpace to )
{GUCEF_TRACE;

    if ( from == to || IsHueBased( format ) || 0 == GetColourChannelCount( format ) )
        return;

    UInt32 channelCount = CPixelMap::GetChannelCountForFormat( format );
    if ( COLOURSPACE_SRGB == from )
    {
        for ( UInt32 i=0; i<pixelCount; ++i )
            for ( UInt32 c=0; c<3; ++c )
                values[ i * channelCount + c ] = SrgbToLinear( values[ i * channelCount + c ] );
    }
    else
    {
        for ( UInt32 i=0; i<pixelCount; ++i )
            for ( UInt32 c=0; c<3; ++c )
                values[ i * channelCount + c ] = LinearToSrgb( values[ i * channelCount + c ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CPixelMapChannelAccess::ToWorkingSpace( Float32* values, UInt32 pixelCount, TPixelStorageFormat format, TColourSpace storedSpace )
{GUCEF_TRACE;

    UInt32 channelCount = CPixelMap::GetChannelCountForFormat( format );
    if ( IsHueBased( format ) )
    {
        bool isHsv = PSF_HSV == format || PSF_HSVA == format;
        for ( UInt32 i=0; i<pixelCount; ++i )
        {
            Float32* pixel = values + i * channelCount;
            if ( isHsv )
                HsvToRgb( pixel[ 0 ], pixel[ 1 ], pixel[ 2 ], pixel[ 0 ], pixel[ 1 ], pixel[ 2 ] );
            else
                HslToRgb( pixel[ 0 ], pixel[ 1 ], pixel[ 2 ], pixel[ 0 ], pixel[ 1 ], pixel[ 2 ] );
        }
    }
    if ( COLOURSPACE_SRGB == storedSpace && GetColourChannelCount( format ) > 0 )
    {
        for ( UInt32 i=0; i<pixelCount; ++i )
            for ( UInt32 c=0; c<3; ++c )
                values[ i * channelCount + c ] = SrgbToLinear( values[ i * channelCount + c ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CPixelMapChannelAccess::FromWorkingSpace( Float32* values, UInt32 pixelCount, TPixelStorageFormat format, TColourSpace storedSpace )
{GUCEF_TRACE;

    UInt32 channelCount = CPixelMap::GetChannelCountForFormat( format );
    if ( COLOURSPACE_SRGB == storedSpace && GetColourChannelCount( format ) > 0 )
    {
        for ( UInt32 i=0; i<pixelCount; ++i )
            for ( UInt32 c=0; c<3; ++c )
                values[ i * channelCount + c ] = LinearToSrgb( values[ i * channelCount + c ] );
    }
    if ( IsHueBased( format ) )
    {
        bool isHsv = PSF_HSV == format || PSF_HSVA == format;
        for ( UInt32 i=0; i<pixelCount; ++i )
        {
            Float32* pixel = values + i * channelCount;
            if ( isHsv )
                RgbToHsv( pixel[ 0 ], pixel[ 1 ], pixel[ 2 ], pixel[ 0 ], pixel[ 1 ], pixel[ 2 ] );
            else
                RgbToHsl( pixel[ 0 ], pixel[ 1 ], pixel[ 2 ], pixel[ 0 ], pixel[ 1 ], pixel[ 2 ] );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
