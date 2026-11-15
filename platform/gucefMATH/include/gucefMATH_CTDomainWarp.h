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

#ifndef GUCEF_MATH_CTDOMAINWARP_H
#define GUCEF_MATH_CTDOMAINWARP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

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

/** The parameters of a CTDomainWarp */
struct SDomainWarpSettings
{
    Float64 amplitude;          /**< largest displacement of the first pass, in position units, default 1 */
    Float64 frequency;          /**< the scale of the warp noise of the first pass, default 1 */
    UInt32 octaves;             /**< 1: one displacement ( single warp ); more: progressive warp, default 1 */
    Float64 lacunarity;         /**< progressive warp: frequency factor per pass, default 2 */
    Float64 gain;               /**< progressive warp: amplitude factor per pass, default 0.5 */

    SDomainWarpSettings( void )
        : amplitude( 1.0 ) , frequency( 1.0 ) , octaves( 1 ) , lacunarity( 2.0 ) , gain( 0.5 )
    {
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Domain warping: the position is displaced by a noise before it is used to sample a texture, a terrain function or
 *  another noise, which turns regular noise into flowing, organic shapes. The displacement of each coordinate is
 *  amplitude * noise at the position times the frequency, the noise sampled at a different offset for every
 *  coordinate so that the displacements are independent.
 *
 *  - Single warp ( octaves = 1 ): p' = p + amplitude * ( n1( p f ), n2( p f ) [, n3( p f )] ).
 *  - Progressive warp ( octaves > 1 ): the passes follow each other, each pass samples at the position displaced by
 *    the passes before it, with the frequency multiplied by the lacunarity and the amplitude by the gain per pass.
 *  - TNoise is CPerlinNoise, COpenSimplex2Noise, CValueNoise or CWorleyNoise ( predefined, exported ) or any class with
 *    Float32 / Float64 Noise2D( x, y ) const and Noise3D( x, y, z ) const returning values in [-1, 1]; the object is held
 *    by value. The largest displacement per axis is amplitude times the sum of the gains.
 *  - Float32 and Float64 overloads, predefined noise types are exported instantiations compiled without
 *    floating-point contraction. Realtime-safe, allocates nothing; concurrent const use is safe.
 */
template < typename TNoise >
class GUCEF_MATH_TEMPLATE_PUBLIC CTDomainWarp
{
    public:

    typedef TNoise TNoiseType;

    CTDomainWarp( void ) : m_noise() , m_settings() {}
    explicit CTDomainWarp( const TNoise& noise ) : m_noise( noise ) , m_settings() {}
    CTDomainWarp( const TNoise& noise, const SDomainWarpSettings& settings ) : m_noise( noise ) , m_settings( settings ) {}

    GUCEF_MATH_FORCE_INLINE const TNoise& GetNoise( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_noise; }
    GUCEF_MATH_FORCE_INLINE TNoise& GetNoise( void ) GUCEF_MATH_REALTIME_CONTEXT { return m_noise; }
    GUCEF_MATH_FORCE_INLINE const SDomainWarpSettings& GetSettings( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_settings; }
    GUCEF_MATH_FORCE_INLINE void SetSettings( const SDomainWarpSettings& settings ) GUCEF_MATH_REALTIME_CONTEXT { m_settings = settings; }

    /** Displaces the position in place */
    void Warp2D( Float32& x, Float32& y ) const GUCEF_MATH_REALTIME_CONTEXT;
    void Warp2D( Float64& x, Float64& y ) const GUCEF_MATH_REALTIME_CONTEXT;
    void Warp3D( Float32& x, Float32& y, Float32& z ) const GUCEF_MATH_REALTIME_CONTEXT;
    void Warp3D( Float64& x, Float64& y, Float64& z ) const GUCEF_MATH_REALTIME_CONTEXT;

    private:

    template < typename T > void Evaluate( T& x, T& y, T& z, bool threeD ) const GUCEF_MATH_REALTIME_CONTEXT;

    TNoise m_noise;
    SDomainWarpSettings m_settings;
};

/*-------------------------------------------------------------------------*/

typedef CTDomainWarp< CPerlinNoise > TPerlinDomainWarp;
typedef CTDomainWarp< COpenSimplex2Noise > TOpenSimplex2DomainWarp;
typedef CTDomainWarp< CValueNoise > TValueDomainWarp;
typedef CTDomainWarp< CWorleyNoise > TWorleyDomainWarp;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename TNoise >
template < typename T >
void
CTDomainWarp< TNoise >::Evaluate( T& x, T& y, T& z, bool threeD ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const SDomainWarpSettings& s = m_settings;
    const UInt32 passes = s.octaves < 1 ? 1 : s.octaves;
    T amplitude = (T) s.amplitude;
    T frequency = (T) s.frequency;
    const T gain = (T) s.gain;
    const T lacunarity = (T) s.lacunarity;
    for ( UInt32 i=0; i<passes; ++i )
    {
        const T fx = x * frequency;
        const T fy = y * frequency;
        const T fz = z * frequency;
        // the displacement of each coordinate is the noise sampled at its own offset: independent patterns
        T dx, dy, dz = T( 0 );
        if ( threeD )
        {
            dx = m_noise.Noise3D( fx, fy, fz );
            dy = m_noise.Noise3D( fx + T( 43.7 ), fy + T( 29.1 ), fz + T( 17.3 ) );
            dz = m_noise.Noise3D( fx + T( 17.9 ), fy + T( 61.3 ), fz + T( 37.1 ) );
        }
        else
        {
            dx = m_noise.Noise2D( fx, fy );
            dy = m_noise.Noise2D( fx + T( 43.7 ), fy + T( 29.1 ) );
        }
        x = x + dx * amplitude;
        y = y + dy * amplitude;
        if ( threeD )
            z = z + dz * amplitude;
        amplitude = amplitude * gain;
        frequency = frequency * lacunarity;
    }
}

/*-------------------------------------------------------------------------*/

template < typename TNoise > GUCEF_MATH_PRECISE_INLINE void CTDomainWarp< TNoise >::Warp2D( Float32& x, Float32& y ) const GUCEF_MATH_REALTIME_CONTEXT { Float32 z = 0.0f; Evaluate< Float32 >( x, y, z, false ); }
template < typename TNoise > GUCEF_MATH_PRECISE_INLINE void CTDomainWarp< TNoise >::Warp2D( Float64& x, Float64& y ) const GUCEF_MATH_REALTIME_CONTEXT { Float64 z = 0.0; Evaluate< Float64 >( x, y, z, false ); }
template < typename TNoise > GUCEF_MATH_PRECISE_INLINE void CTDomainWarp< TNoise >::Warp3D( Float32& x, Float32& y, Float32& z ) const GUCEF_MATH_REALTIME_CONTEXT { Evaluate< Float32 >( x, y, z, true ); }
template < typename TNoise > GUCEF_MATH_PRECISE_INLINE void CTDomainWarp< TNoise >::Warp3D( Float64& x, Float64& y, Float64& z ) const GUCEF_MATH_REALTIME_CONTEXT { Evaluate< Float64 >( x, y, z, true ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTDOMAINWARP_MEMBERS( prefix, noise )                                                                         \
    prefix void CTDomainWarp< noise >::Warp2D( Float32&, Float32& ) const GUCEF_MATH_REALTIME_CONTEXT;                          \
    prefix void CTDomainWarp< noise >::Warp2D( Float64&, Float64& ) const GUCEF_MATH_REALTIME_CONTEXT;                          \
    prefix void CTDomainWarp< noise >::Warp3D( Float32&, Float32&, Float32& ) const GUCEF_MATH_REALTIME_CONTEXT;                \
    prefix void CTDomainWarp< noise >::Warp3D( Float64&, Float64&, Float64& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTDOMAINWARP_PREDEFINED( prefix )                    \
    GUCEF_MATH_CTDOMAINWARP_MEMBERS( prefix, CPerlinNoise )             \
    GUCEF_MATH_CTDOMAINWARP_MEMBERS( prefix, COpenSimplex2Noise )       \
    GUCEF_MATH_CTDOMAINWARP_MEMBERS( prefix, CValueNoise )              \
    GUCEF_MATH_CTDOMAINWARP_MEMBERS( prefix, CWorleyNoise )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_CTDOMAINWARP_PREDEFINED( GUCEF_MATH_EXTERN_TEMPLATE )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTDOMAINWARP_H ? */
