/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include "gucefGUI_CChartPalette.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Machado, Oliveira and Fernandes (2009) at severity 1, on linear RGB: protanopia, deuteranopia, tritanopia */
const Float64 CvdMatrices[ 3 ][ 3 ][ 3 ] =
{
    { {  0.152286,  1.052583, -0.204868 },
      {  0.114503,  0.786281,  0.099216 },
      { -0.003882, -0.048116,  1.051998 } },
    { {  0.367322,  0.860646, -0.227968 },
      {  0.280085,  0.672501,  0.047413 },
      { -0.011820,  0.042940,  0.968881 } },
    { {  1.255528, -0.076749, -0.178779 },
      { -0.078411,  0.930809,  0.147602 },
      {  0.004733,  0.691367,  0.303900 } }
};

Float64
SrgbToLinear( UInt8 channel )
{
    Float64 c = (Float64) channel / 255.0;
    return c <= 0.04045 ? c / 12.92 : pow( ( c + 0.055 ) / 1.055, 2.4 );
}

inline Float64
Clamp01( Float64 value )
{
    return value < 0.0 ? 0.0 : ( value > 1.0 ? 1.0 : value );
}

inline Float64
CubeRoot( Float64 value )
{
    return value < 0.0 ? -pow( -value, 1.0 / 3.0 ) : pow( value, 1.0 / 3.0 );
}

/** OKLab of a linear RGB colour */
void
LinearToOklab( const Float64 rgb[ 3 ], Float64 lab[ 3 ] )
{
    Float64 l = CubeRoot( 0.4122214708 * rgb[ 0 ] + 0.5363325363 * rgb[ 1 ] + 0.0514459929 * rgb[ 2 ] );
    Float64 m = CubeRoot( 0.2119034982 * rgb[ 0 ] + 0.6806995451 * rgb[ 1 ] + 0.1073969566 * rgb[ 2 ] );
    Float64 s = CubeRoot( 0.0883024619 * rgb[ 0 ] + 0.2817188376 * rgb[ 1 ] + 0.6299787005 * rgb[ 2 ] );
    lab[ 0 ] = 0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s;
    lab[ 1 ] = 1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s;
    lab[ 2 ] = 0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s;
}

/** OKLab of a colour, seen with the given simulation (0 none, 1..3 the CVD matrices) */
void
ColourToOklab( const CColour& colour, Int32 simulation, Float64 lab[ 3 ] )
{
    Float64 rgb[ 3 ] = { SrgbToLinear( colour.r ), SrgbToLinear( colour.g ), SrgbToLinear( colour.b ) };
    if ( simulation >= 1 && simulation <= 3 )
    {
        const Float64 (*m)[ 3 ] = CvdMatrices[ simulation - 1 ];
        Float64 simulated[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
            simulated[ i ] = Clamp01( m[ i ][ 0 ] * rgb[ 0 ] + m[ i ][ 1 ] * rgb[ 1 ] + m[ i ][ 2 ] * rgb[ 2 ] );
        rgb[ 0 ] = simulated[ 0 ];
        rgb[ 1 ] = simulated[ 1 ];
        rgb[ 2 ] = simulated[ 2 ];
    }
    LinearToOklab( rgb, lab );
}

CChartPalette::TColourVector
MakeDarkPalette( void )
{
    CChartPalette::TColourVector colours;
    colours.push_back( CColour( 57, 135, 229, 255 ) );     /* blue     #3987e5 */
    colours.push_back( CColour( 217, 89, 38, 255 ) );      /* orange   #d95926 */
    colours.push_back( CColour( 25, 158, 112, 255 ) );     /* aqua     #199e70 */
    colours.push_back( CColour( 201, 133, 0, 255 ) );      /* yellow   #c98500 */
    colours.push_back( CColour( 213, 81, 129, 255 ) );     /* magenta  #d55181 */
    colours.push_back( CColour( 0, 131, 0, 255 ) );        /* green    #008300 */
    colours.push_back( CColour( 144, 133, 233, 255 ) );    /* violet   #9085e9 */
    colours.push_back( CColour( 230, 103, 103, 255 ) );    /* red      #e66767 */
    return colours;
}

CChartPalette::TColourVector
MakeLightPalette( void )
{
    CChartPalette::TColourVector colours;
    colours.push_back( CColour( 42, 120, 214, 255 ) );     /* blue     #2a78d6 */
    colours.push_back( CColour( 235, 104, 52, 255 ) );     /* orange   #eb6834 */
    colours.push_back( CColour( 27, 175, 122, 255 ) );     /* aqua     #1baf7a */
    colours.push_back( CColour( 237, 161, 0, 255 ) );      /* yellow   #eda100 */
    colours.push_back( CColour( 232, 123, 164, 255 ) );    /* magenta  #e87ba4 */
    colours.push_back( CColour( 0, 131, 0, 255 ) );        /* green    #008300 */
    colours.push_back( CColour( 98, 80, 214, 255 ) );      /* violet   #6250d6 */
    colours.push_back( CColour( 227, 73, 72, 255 ) );      /* red      #e34948 */
    return colours;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const Float64 CChartPalette::CvdTarget = 8.0;
const Float64 CChartPalette::NormalVisionFloor = 15.0;
const Float64 CChartPalette::ChromaFloor = 0.10;

/*-------------------------------------------------------------------------*/

const CChartPalette::TColourVector&
CChartPalette::GetDarkPalette( void )
{GUCEF_TRACE;

    static const TColourVector palette = MakeDarkPalette();
    return palette;
}

/*-------------------------------------------------------------------------*/

const CChartPalette::TColourVector&
CChartPalette::GetLightPalette( void )
{GUCEF_TRACE;

    static const TColourVector palette = MakeLightPalette();
    return palette;
}

/*-------------------------------------------------------------------------*/

CChartPalette::TStringVector
CChartPalette::ToStringList( const TColourVector& colours )
{GUCEF_TRACE;

    TStringVector entries;
    entries.reserve( colours.size() );
    for ( UInt32 i=0; i<(UInt32)colours.size(); ++i )
        entries.push_back( colours[ i ].ToString() );
    return entries;
}

/*-------------------------------------------------------------------------*/

bool
CChartPalette::FromStringList( const TStringVector& entries, TColourVector& colours )
{GUCEF_TRACE;

    bool allValid = true;
    for ( UInt32 i=0; i<(UInt32)entries.size(); ++i )
    {
        CColour colour;
        if ( CColour::FromString( entries[ i ], colour ) )
            colours.push_back( colour );
        else
            allValid = false;
    }
    return allValid;
}

/*-------------------------------------------------------------------------*/

Float64
CChartPalette::DeltaE( const CColour& a, const CColour& b, Int32 simulation )
{GUCEF_TRACE;

    Float64 labA[ 3 ], labB[ 3 ];
    ColourToOklab( a, simulation, labA );
    ColourToOklab( b, simulation, labB );
    Float64 dl = labA[ 0 ] - labB[ 0 ];
    Float64 da = labA[ 1 ] - labB[ 1 ];
    Float64 db = labA[ 2 ] - labB[ 2 ];
    return 100.0 * sqrt( dl * dl + da * da + db * db );
}

/*-------------------------------------------------------------------------*/

void
CChartPalette::LightnessAndChroma( const CColour& colour, Float64& lightness, Float64& chroma )
{GUCEF_TRACE;

    Float64 lab[ 3 ];
    ColourToOklab( colour, 0, lab );
    lightness = lab[ 0 ];
    chroma = sqrt( lab[ 1 ] * lab[ 1 ] + lab[ 2 ] * lab[ 2 ] );
}

/*-------------------------------------------------------------------------*/

CChartPalette::TValidation
CChartPalette::Validate( const TColourVector& colours, bool darkSurface )
{GUCEF_TRACE;

    const Float64 bandLow = darkSurface ? 0.48 : 0.43;
    const Float64 bandHigh = darkSurface ? 0.67 : 0.77;

    TValidation result;
    result.passed = !colours.empty();
    result.worstAdjacentCvdDeltaE = 1000.0;
    result.worstAdjacentNormalDeltaE = 1000.0;
    result.minLightness = 1.0;
    result.maxLightness = 0.0;
    result.minChroma = 1.0;

    for ( UInt32 i=0; i<(UInt32)colours.size(); ++i )
    {
        Float64 lightness = 0.0, chroma = 0.0;
        LightnessAndChroma( colours[ i ], lightness, chroma );
        if ( lightness < result.minLightness )
            result.minLightness = lightness;
        if ( lightness > result.maxLightness )
            result.maxLightness = lightness;
        if ( chroma < result.minChroma )
            result.minChroma = chroma;

        if ( i > 0 )
        {
            Float64 protan = DeltaE( colours[ i-1 ], colours[ i ], 1 );
            Float64 deutan = DeltaE( colours[ i-1 ], colours[ i ], 2 );
            Float64 normal = DeltaE( colours[ i-1 ], colours[ i ], 0 );
            Float64 cvd = protan < deutan ? protan : deutan;
            if ( cvd < result.worstAdjacentCvdDeltaE )
                result.worstAdjacentCvdDeltaE = cvd;
            if ( normal < result.worstAdjacentNormalDeltaE )
                result.worstAdjacentNormalDeltaE = normal;
        }
    }

    if ( result.minLightness < bandLow || result.maxLightness > bandHigh )
        result.passed = false;
    if ( result.minChroma < ChromaFloor )
        result.passed = false;
    if ( colours.size() > 1 )
    {
        if ( result.worstAdjacentCvdDeltaE < CvdTarget || result.worstAdjacentNormalDeltaE < NormalVisionFloor )
            result.passed = false;
    }
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
