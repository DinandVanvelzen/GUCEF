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

#ifndef GUCEF_MATH_CTLINEARCOLOUR_H
#define GUCEF_MATH_CTLINEARCOLOUR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_COLOURFUNCTIONS_H
#include "gucefMATH_ColourFunctions.h"
#define GUCEF_MATH_COLOURFUNCTIONS_H
#endif /* GUCEF_MATH_COLOURFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A colour in linear light: red, green, blue and alpha as floating point values (T is Float32 or Float64), the
 *  space in which blending and lighting is correct. Colours as stored in 8 bit assets are sRGB encoded: FromRgba8
 *  decodes them, ToRgba8 encodes and rounds them (half to even, clamped to [ 0, 1 ]). Alpha is a coverage and is not
 *  gamma encoded: alpha 255 is 1.
 *
 *  Layout: four values of T in the order r, g, b, a. No allocation, no virtual functions: realtime safe.
 *  Components are not clamped by arithmetic, so HDR values are kept.
 */
template < typename T >
class CTLinearColour
{
    public:

    typedef T TValue;

    T r;
    T g;
    T b;
    T a;

    /** Transparent black */
    GUCEF_MATH_FORCE_INLINE CTLinearColour( void )
        : r( T( 0 ) )
        , g( T( 0 ) )
        , b( T( 0 ) )
        , a( T( 0 ) )
    {
    }

    GUCEF_MATH_FORCE_INLINE CTLinearColour( const T& red, const T& green, const T& blue, const T& alpha )
        : r( red )
        , g( green )
        , b( blue )
        , a( alpha )
    {
    }

    /** From sRGB encoded 8 bit components: red, green and blue are decoded to linear light, alpha is divided by 255 */
    static GUCEF_MATH_FORCE_INLINE CTLinearColour FromRgba8( UInt8 red, UInt8 green, UInt8 blue, UInt8 alpha )
    {
        return CTLinearColour( SrgbToLinear( (T) red / T( 255 ) ), SrgbToLinear( (T) green / T( 255 ) ), SrgbToLinear( (T) blue / T( 255 ) ), (T) alpha / T( 255 ) );
    }

    /** To sRGB encoded 8 bit components, each clamped to [ 0, 1 ] and rounded half to even; not a number gives 0 */
    GUCEF_MATH_FORCE_INLINE void ToRgba8( UInt8& red, UInt8& green, UInt8& blue, UInt8& alpha ) const
    {
        red = ColourQuantize( LinearToSrgb( r ), 255 );
        green = ColourQuantize( LinearToSrgb( g ), 255 );
        blue = ColourQuantize( LinearToSrgb( b ), 255 );
        alpha = ColourQuantize( a, 255 );
    }

    /** a + ( b - a ) t on every component, in linear light and without clamping t */
    static GUCEF_MATH_FORCE_INLINE CTLinearColour Lerp( const CTLinearColour& from, const CTLinearColour& to, const T& t )
    {
        return CTLinearColour( from.r + ( to.r - from.r ) * t, from.g + ( to.g - from.g ) * t, from.b + ( to.b - from.b ) * t, from.a + ( to.a - from.a ) * t );
    }

    /** Red, green and blue multiplied by alpha */
    GUCEF_MATH_FORCE_INLINE CTLinearColour Premultiplied( void ) const
    {
        return CTLinearColour( r * a, g * a, b * a, a );
    }

    /** Red, green and blue divided by alpha; alpha 0 gives transparent black */
    GUCEF_MATH_FORCE_INLINE CTLinearColour Unpremultiplied( void ) const
    {
        return T( 0 ) == a ? CTLinearColour() : CTLinearColour( r / a, g / a, b / a, a );
    }

    /** The relative luminance of the colour (alpha ignored) */
    GUCEF_MATH_FORCE_INLINE T GetRelativeLuminance( void ) const
    {
        return MATH::GetRelativeLuminance( r, g, b );
    }

    friend GUCEF_MATH_FORCE_INLINE CTLinearColour operator+( const CTLinearColour& x, const CTLinearColour& y ) { return CTLinearColour( x.r + y.r, x.g + y.g, x.b + y.b, x.a + y.a ); }
    friend GUCEF_MATH_FORCE_INLINE CTLinearColour operator-( const CTLinearColour& x, const CTLinearColour& y ) { return CTLinearColour( x.r - y.r, x.g - y.g, x.b - y.b, x.a - y.a ); }
    friend GUCEF_MATH_FORCE_INLINE CTLinearColour operator*( const CTLinearColour& x, const T& factor ) { return CTLinearColour( x.r * factor, x.g * factor, x.b * factor, x.a * factor ); }
    friend GUCEF_MATH_FORCE_INLINE CTLinearColour operator*( const T& factor, const CTLinearColour& x ) { return CTLinearColour( factor * x.r, factor * x.g, factor * x.b, factor * x.a ); }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTLinearColour& x, const CTLinearColour& y ) { return x.r == y.r && x.g == y.g && x.b == y.b && x.a == y.a; }
    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTLinearColour& x, const CTLinearColour& y ) { return !( x == y ); }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTLinearColour< Float32 > TFloat32LinearColour;
typedef CTLinearColour< Float64 > TFloat64LinearColour;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTLINEARCOLOUR_H ? */
