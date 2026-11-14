/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_COLOURFUNCTIONS_H
#define GUCEF_MATH_COLOURFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_CTMATHCONSTANTS_H
#include "gucefMATH_CTMathConstants.h"
#define GUCEF_MATH_CTMATHCONSTANTS_H
#endif /* GUCEF_MATH_CTMATHCONSTANTS_H ? */

#ifndef GUCEF_MATH_CTDEGREE_H
#include "gucefMATH_CTDegree.h"
#define GUCEF_MATH_CTDEGREE_H
#endif /* GUCEF_MATH_CTDEGREE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Colour conversions on floating point components (T is Float32 or Float64). Unless a function says otherwise the
 *  red, green and blue components are in [ 0, 1 ] and the functions do not clamp. Hues are strong angles
 *  (CTDegree, in [ 0, 360 ) ); saturation, value and lightness are in [ 0, 1 ]. All functions are realtime safe.
 */

/*-------------------------------------------------------------------------*/

/**
 *  sRGB transfer function to linear light (IEC 61966-2-1): c / 12.92 up to 0.04045, ( ( c + 0.055 ) / 1.055 )^2.4 above.
 *  Defined for every value: negative values and values above 1 continue the curve, the negative side mirrored.
 */
template< typename T >
inline T
SrgbToLinear( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T magnitude = value < T( 0 ) ? -value : value;
    const T linear = magnitude <= T( 0.04045 ) ? magnitude / T( 12.92 ) : std::pow( ( magnitude + T( 0.055 ) ) / T( 1.055 ), T( 2.4 ) );
    return value < T( 0 ) ? -linear : linear;
}

/*-------------------------------------------------------------------------*/

/**
 *  Linear light to the sRGB transfer function (IEC 61966-2-1): 12.92 c up to 0.0031308, 1.055 c^( 1 / 2.4 ) - 0.055 above.
 *  Defined for every value like SrgbToLinear.
 */
template< typename T >
inline T
LinearToSrgb( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T magnitude = value < T( 0 ) ? -value : value;
    const T encoded = magnitude <= T( 0.0031308 ) ? magnitude * T( 12.92 ) : T( 1.055 ) * std::pow( magnitude, T( 1 ) / T( 2.4 ) ) - T( 0.055 );
    return value < T( 0 ) ? -encoded : encoded;
}

/*-------------------------------------------------------------------------*/

/** The relative luminance of linear sRGB components: 0.2126 r + 0.7152 g + 0.0722 b */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
GetRelativeLuminance( const T& r, const T& g, const T& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return T( 0.2126 ) * r + T( 0.7152 ) * g + T( 0.0722 ) * b;
}

/*-------------------------------------------------------------------------*/

/** The value of [ 0, 1 ] rounded to the nearest of the 'steps' + 1 integers 0 to 'steps', halves to even; NaN gives 0 */
template< typename T >
inline UInt8
ColourQuantize( const T& value, UInt32 steps ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !( value > T( 0 ) ) )
        return 0;
    if ( value >= T( 1 ) )
        return (UInt8) steps;
    const T scaled = value * (T) steps;
    const T floored = std::floor( scaled );
    const T fraction = scaled - floored;
    UInt32 result = (UInt32) floored;
    if ( fraction > T( 0.5 ) || ( fraction == T( 0.5 ) && 0 != ( result & 1 ) ) )
        ++result;
    return (UInt8) result;
}

/*-------------------------------------------------------------------------*/

/** Hue in degrees [ 0, 360 ) of the components, and the largest and smallest of them; the hue of a grey is 0 */
template< typename T >
inline T
ColourHueDegrees( const T& r, const T& g, const T& b, const T& largest, const T& smallest ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T delta = largest - smallest;
    if ( !( delta > T( 0 ) ) )
        return T( 0 );
    T hue;
    if ( largest == r )
        hue = ( g - b ) / delta;
    else if ( largest == g )
        hue = T( 2 ) + ( b - r ) / delta;
    else
        hue = T( 4 ) + ( r - g ) / delta;
    hue *= T( 60 );
    if ( hue < T( 0 ) )
        hue += T( 360 );
    if ( hue >= T( 360 ) )
        hue -= T( 360 );
    return hue;
}

/*-------------------------------------------------------------------------*/

/** Red, green, blue in [ 0, 1 ] to hue, saturation and value. A grey has saturation 0 and hue 0 */
template< typename T >
inline void
RgbToHsv( const T& r, const T& g, const T& b, CTDegree< T >& hue, T& saturation, T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T largest = r > g ? ( r > b ? r : b ) : ( g > b ? g : b );
    const T smallest = r < g ? ( r < b ? r : b ) : ( g < b ? g : b );
    hue = CTDegree< T >( ColourHueDegrees( r, g, b, largest, smallest ) );
    saturation = largest > T( 0 ) ? ( largest - smallest ) / largest : T( 0 );
    value = largest;
}

/*-------------------------------------------------------------------------*/

/** The hue wraps: any angle is accepted */
template< typename T >
inline void
HsvToRgb( const CTDegree< T >& hue, const T& saturation, const T& value, T& r, T& g, T& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    T sector = hue.GetValue() / T( 60 );
    sector -= T( 6 ) * std::floor( sector / T( 6 ) );
    const T whole = std::floor( sector );
    const T fraction = sector - whole;
    const T p = value * ( T( 1 ) - saturation );
    const T q = value * ( T( 1 ) - saturation * fraction );
    const T t = value * ( T( 1 ) - saturation * ( T( 1 ) - fraction ) );
    switch ( (Int32) whole )
    {
        case 0:  r = value; g = t;     b = p;     break;
        case 1:  r = q;     g = value; b = p;     break;
        case 2:  r = p;     g = value; b = t;     break;
        case 3:  r = p;     g = q;     b = value; break;
        case 4:  r = t;     g = p;     b = value; break;
        default: r = value; g = p;     b = q;     break;
    }
}

/*-------------------------------------------------------------------------*/

/** Red, green, blue in [ 0, 1 ] to hue, saturation and lightness. A grey has saturation 0 and hue 0 */
template< typename T >
inline void
RgbToHsl( const T& r, const T& g, const T& b, CTDegree< T >& hue, T& saturation, T& lightness ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T largest = r > g ? ( r > b ? r : b ) : ( g > b ? g : b );
    const T smallest = r < g ? ( r < b ? r : b ) : ( g < b ? g : b );
    hue = CTDegree< T >( ColourHueDegrees( r, g, b, largest, smallest ) );
    lightness = T( 0.5 ) * ( largest + smallest );
    const T delta = largest - smallest;
    if ( !( delta > T( 0 ) ) )
        saturation = T( 0 );
    else
        saturation = delta / ( T( 1 ) - std::fabs( T( 2 ) * lightness - T( 1 ) ) );
}

/*-------------------------------------------------------------------------*/

/** The hue wraps: any angle is accepted */
template< typename T >
inline void
HslToRgb( const CTDegree< T >& hue, const T& saturation, const T& lightness, T& r, T& g, T& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T chroma = ( T( 1 ) - std::fabs( T( 2 ) * lightness - T( 1 ) ) ) * saturation;
    const T low = lightness - T( 0.5 ) * chroma;
    // the value form with chroma as the range: HSV of ( hue, 1, chroma ) shifted up by 'low'
    HsvToRgb( hue, T( 1 ), chroma, r, g, b );
    r += low;
    g += low;
    b += low;
}

/*-------------------------------------------------------------------------*/

/** Cube root that keeps the sign */
template< typename T >
GUCEF_MATH_FORCE_INLINE T
ColourCubeRoot( const T& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    return x < T( 0 ) ? -std::pow( -x, T( 1 ) / T( 3 ) ) : std::pow( x, T( 1 ) / T( 3 ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Linear sRGB (not gamma encoded) to OKLab ( Ottosson, 2020 ): L in [ 0, 1 ] for lightness, a and b the green-red
 *  and blue-yellow axes. White ( 1, 1, 1 ) is ( 1, 0, 0 ) up to the precision of the published matrices.
 */
template< typename T >
inline void
LinearRgbToOkLab( const T& r, const T& g, const T& b, T& lightness, T& a, T& bAxis ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T l = ColourCubeRoot( T( 0.4122214708 ) * r + T( 0.5363325363 ) * g + T( 0.0514459929 ) * b );
    const T m = ColourCubeRoot( T( 0.2119034982 ) * r + T( 0.6806995451 ) * g + T( 0.1073969566 ) * b );
    const T s = ColourCubeRoot( T( 0.0883024619 ) * r + T( 0.2817188376 ) * g + T( 0.6299787005 ) * b );
    lightness = T( 0.2104542553 ) * l + T( 0.7936177850 ) * m - T( 0.0040720468 ) * s;
    a = T( 1.9779984951 ) * l - T( 2.4285922050 ) * m + T( 0.4505937099 ) * s;
    bAxis = T( 0.0259040371 ) * l + T( 0.7827717662 ) * m - T( 0.8086757660 ) * s;
}

/*-------------------------------------------------------------------------*/

/** The inverse of LinearRgbToOkLab; the result is not clamped to the gamut */
template< typename T >
inline void
OkLabToLinearRgb( const T& lightness, const T& a, const T& bAxis, T& r, T& g, T& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T l = lightness + T( 0.3963377774 ) * a + T( 0.2158037573 ) * bAxis;
    const T m = lightness - T( 0.1055613458 ) * a - T( 0.0638541728 ) * bAxis;
    const T s = lightness - T( 0.0894841775 ) * a - T( 1.2914855480 ) * bAxis;
    const T l3 = l * l * l;
    const T m3 = m * m * m;
    const T s3 = s * s * s;
    r = T( 4.0767416621 ) * l3 - T( 3.3077115913 ) * m3 + T( 0.2309699292 ) * s3;
    g = T( -1.2684380046 ) * l3 + T( 2.6097574011 ) * m3 - T( 0.3413193965 ) * s3;
    b = T( -0.0041960863 ) * l3 - T( 0.7034186147 ) * m3 + T( 1.7076147010 ) * s3;
}

/*-------------------------------------------------------------------------*/

/** OKLab to the polar form: chroma sqrt( a^2 + b^2 ) and the hue angle in [ 0, 360 ) degrees (0 for zero chroma) */
template< typename T >
inline void
OkLabToOkLch( const T& lightness, const T& a, const T& bAxis, T& outLightness, T& chroma, CTDegree< T >& hue ) GUCEF_MATH_REALTIME_CONTEXT
{
    outLightness = lightness;
    chroma = std::sqrt( a * a + bAxis * bAxis );
    T degrees = T( 0 );
    if ( chroma > T( 0 ) )
    {
        degrees = std::atan2( bAxis, a ) * CTMathConstants< T >::DegreesPerRadian();
        if ( degrees < T( 0 ) )
            degrees += T( 360 );
        if ( degrees >= T( 360 ) )
            degrees -= T( 360 );
    }
    hue = CTDegree< T >( degrees );
}

/*-------------------------------------------------------------------------*/

/** The inverse of OkLabToOkLch */
template< typename T >
inline void
OkLchToOkLab( const T& lightness, const T& chroma, const CTDegree< T >& hue, T& outLightness, T& a, T& bAxis ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T radians = hue.GetRadians();
    outLightness = lightness;
    a = chroma * std::cos( radians );
    bAxis = chroma * std::sin( radians );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_COLOURFUNCTIONS_H ? */
