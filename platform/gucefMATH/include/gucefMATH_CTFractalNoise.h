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

#ifndef GUCEF_MATH_CTFRACTALNOISE_H
#define GUCEF_MATH_CTFRACTALNOISE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CPERLINNOISE_H
#include "gucefMATH_CPerlinNoise.h"
#define GUCEF_MATH_CPERLINNOISE_H
#endif /* GUCEF_MATH_CPERLINNOISE_H ? */

#ifndef GUCEF_MATH_COPENSIMPLEX2NOISE_H
#include "gucefMATH_COpenSimplex2Noise.h"
#define GUCEF_MATH_COPENSIMPLEX2NOISE_H
#endif /* GUCEF_MATH_COPENSIMPLEX2NOISE_H ? */

#ifndef GUCEF_MATH_CVALUENOISE_H
#include "gucefMATH_CValueNoise.h"
#define GUCEF_MATH_CVALUENOISE_H
#endif /* GUCEF_MATH_CVALUENOISE_H ? */

#ifndef GUCEF_MATH_CWORLEYNOISE_H
#include "gucefMATH_CWorleyNoise.h"
#define GUCEF_MATH_CWORLEYNOISE_H
#endif /* GUCEF_MATH_CWORLEYNOISE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How the octaves of a CTFractalNoise are combined */
enum EFractalType
{
    FRACTALTYPE_FBM = 0,        /**< fractional Brownian motion: the octaves are added as they are */
    FRACTALTYPE_RIDGED,         /**< 1 - 2 |n| per octave: sharp ridges where the noise crosses zero */
    FRACTALTYPE_BILLOW,         /**< 2 |n| - 1 per octave: rounded billows with creases in the valleys */
    FRACTALTYPE_PINGPONG        /**< the octave is folded back and forth ( ping-pong of ( n + 1 ) * pingPongStrength ) */
};

typedef enum EFractalType TFractalType;

/** The parameters of a CTFractalNoise */
struct SFractalNoiseSettings
{
    TFractalType type;              /**< default FRACTALTYPE_FBM */
    UInt32 octaves;                 /**< number of noise layers, default 4, at least 1 */
    Float64 frequency;              /**< the scale of the first octave: the noise is sampled at position times frequency, default 1 */
    Float64 lacunarity;             /**< frequency factor from one octave to the next, default 2 */
    Float64 gain;                   /**< amplitude factor from one octave to the next, default 0.5 */
    Float64 weightedStrength;       /**< 0: every octave has its full amplitude; 1: an octave's amplitude is scaled by the height of the layer below it, so detail appears only on high ground, default 0 */
    Float64 pingPongStrength;       /**< factor of ( n + 1 ) in FRACTALTYPE_PINGPONG, default 2 */

    SFractalNoiseSettings( void )
        : type( FRACTALTYPE_FBM ) , octaves( 4 ) , frequency( 1.0 ) , lacunarity( 2.0 ) , gain( 0.5 ) ,
          weightedStrength( 0.0 ) , pingPongStrength( 2.0 )
    {
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Several octaves of a base noise added into one fractal ( fBm, ridged, billow, ping-pong ), section 4.9 of the design.
 *
 *  - TNoise is CPerlinNoise, COpenSimplex2Noise, CValueNoise or CWorleyNoise ( predefined, exported ) or any class with
 *    the overloads Float32 / Float64 Noise2D( x, y ) const and Noise3D( x, y, z ) const returning values in [-1, 1].
 *    The noise object is held by value ( its seed and tables belong to this object ).
 *  - Octave i samples the noise at position * frequency * lacunarity^i plus a fixed offset ( 0 for the first octave,
 *    so the lattice zeros of different octaves do not line up ) and is weighted with gain^i; the sum is divided by the
 *    sum of the weights, so the result stays in [-1, 1]. With one octave and the type FRACTALTYPE_FBM the result is
 *    the base noise at position * frequency exactly.
 *  - Float32 and Float64 overloads, predefined noise types are exported instantiations compiled without
 *    floating-point contraction. Sampling allocates nothing and is realtime-safe; concurrent const use is safe.
 */
template < typename TNoise >
class GUCEF_MATH_TEMPLATE_PUBLIC CTFractalNoise
{
    public:

    typedef TNoise TNoiseType;

    CTFractalNoise( void ) : m_noise() , m_settings() {}
    explicit CTFractalNoise( const TNoise& noise ) : m_noise( noise ) , m_settings() {}
    CTFractalNoise( const TNoise& noise, const SFractalNoiseSettings& settings ) : m_noise( noise ) , m_settings( settings ) {}

    GUCEF_MATH_FORCE_INLINE const TNoise& GetNoise( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_noise; }
    GUCEF_MATH_FORCE_INLINE TNoise& GetNoise( void ) GUCEF_MATH_REALTIME_CONTEXT { return m_noise; }
    GUCEF_MATH_FORCE_INLINE const SFractalNoiseSettings& GetSettings( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_settings; }
    GUCEF_MATH_FORCE_INLINE void SetSettings( const SFractalNoiseSettings& settings ) GUCEF_MATH_REALTIME_CONTEXT { m_settings = settings; }

    Float32 Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float32 Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT;
    Float64 Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT;

    private:

    template < typename T > T Evaluate( T x, T y, T z, bool threeD ) const GUCEF_MATH_REALTIME_CONTEXT;

    TNoise m_noise;
    SFractalNoiseSettings m_settings;
};

/*-------------------------------------------------------------------------*/

typedef CTFractalNoise< CPerlinNoise > TFractalPerlinNoise;
typedef CTFractalNoise< COpenSimplex2Noise > TFractalOpenSimplex2Noise;
typedef CTFractalNoise< CValueNoise > TFractalValueNoise;
typedef CTFractalNoise< CWorleyNoise > TFractalWorleyNoise;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename TNoise >
template < typename T >
T
CTFractalNoise< TNoise >::Evaluate( T x, T y, T z, bool threeD ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const SFractalNoiseSettings& s = m_settings;
    const UInt32 octaves = s.octaves < 1 ? 1 : s.octaves;
    const T one = T( 1 );
    const T gain = (T) s.gain;
    const T lacunarity = (T) s.lacunarity;
    T frequency = (T) s.frequency;
    T amplitude = one;          // weighted amplitude of the octave
    T limit = one;              // amplitude without the weighting: the bound of the sum
    T limitSum = T( 0 );
    T sum = T( 0 );
    for ( UInt32 i=0; i<octaves; ++i )
    {
        // offsets so that the lattices of the octaves do not line up; none for the first octave
        const T offset = (T) i;
        const T fx = x * frequency + offset * T( 37.3 );
        const T fy = y * frequency + offset * T( 53.1 );
        const T fz = z * frequency + offset * T( 71.7 );
        const T n = threeD ? m_noise.Noise3D( fx, fy, fz ) : m_noise.Noise2D( fx, fy );

        T value = n;
        const T magnitude = n < T( 0 ) ? -n : n;
        switch ( s.type )
        {
            case FRACTALTYPE_RIDGED: value = one - T( 2 ) * magnitude; break;
            case FRACTALTYPE_BILLOW: value = T( 2 ) * magnitude - one; break;
            case FRACTALTYPE_PINGPONG: value = ( PingPong( ( n + one ) * (T) s.pingPongStrength, one ) - T( 0.5 ) ) * T( 2 ); break;
            default: break;
        }

        sum = sum + value * amplitude;
        limitSum = limitSum + limit;
        limit = limit * gain;
        frequency = frequency * lacunarity;

        // weighted octaves: the next layer is only as strong as this one is high
        const T height = T( 0.5 ) * ( value + one );
        const T clamped = height > T( 0 ) ? ( height < one ? height : one ) : T( 0 );
        amplitude = amplitude * gain * Lerp( one, clamped, (T) s.weightedStrength );
    }
    if ( !( limitSum > T( 0 ) ) )
        return T( 0 );
    const T result = sum / limitSum;
    return result > one ? one : ( result < -one ? -one : result );
}

/*-------------------------------------------------------------------------*/

template < typename TNoise > GUCEF_MATH_PRECISE_INLINE Float32 CTFractalNoise< TNoise >::Noise2D( Float32 x, Float32 y ) const GUCEF_MATH_REALTIME_CONTEXT { return Evaluate< Float32 >( x, y, 0.0f, false ); }
template < typename TNoise > GUCEF_MATH_PRECISE_INLINE Float64 CTFractalNoise< TNoise >::Noise2D( Float64 x, Float64 y ) const GUCEF_MATH_REALTIME_CONTEXT { return Evaluate< Float64 >( x, y, 0.0, false ); }
template < typename TNoise > GUCEF_MATH_PRECISE_INLINE Float32 CTFractalNoise< TNoise >::Noise3D( Float32 x, Float32 y, Float32 z ) const GUCEF_MATH_REALTIME_CONTEXT { return Evaluate< Float32 >( x, y, z, true ); }
template < typename TNoise > GUCEF_MATH_PRECISE_INLINE Float64 CTFractalNoise< TNoise >::Noise3D( Float64 x, Float64 y, Float64 z ) const GUCEF_MATH_REALTIME_CONTEXT { return Evaluate< Float64 >( x, y, z, true ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTFRACTALNOISE_MEMBERS( prefix, noise )                                                                        \
    prefix Float32 CTFractalNoise< noise >::Noise2D( Float32, Float32 ) const GUCEF_MATH_REALTIME_CONTEXT;                       \
    prefix Float64 CTFractalNoise< noise >::Noise2D( Float64, Float64 ) const GUCEF_MATH_REALTIME_CONTEXT;                       \
    prefix Float32 CTFractalNoise< noise >::Noise3D( Float32, Float32, Float32 ) const GUCEF_MATH_REALTIME_CONTEXT;              \
    prefix Float64 CTFractalNoise< noise >::Noise3D( Float64, Float64, Float64 ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTFRACTALNOISE_PREDEFINED( prefix )                  \
    GUCEF_MATH_CTFRACTALNOISE_MEMBERS( prefix, CPerlinNoise )           \
    GUCEF_MATH_CTFRACTALNOISE_MEMBERS( prefix, COpenSimplex2Noise )     \
    GUCEF_MATH_CTFRACTALNOISE_MEMBERS( prefix, CValueNoise )            \
    GUCEF_MATH_CTFRACTALNOISE_MEMBERS( prefix, CWorleyNoise )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_CTFRACTALNOISE_PREDEFINED( GUCEF_MATH_EXTERN_TEMPLATE )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTFRACTALNOISE_H ? */
