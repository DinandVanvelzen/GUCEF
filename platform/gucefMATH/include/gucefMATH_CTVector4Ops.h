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

#ifndef GUCEF_MATH_CTVECTOR4OPS_H
#define GUCEF_MATH_CTVECTOR4OPS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

#ifndef GUCEF_MATH_COMPONENTFUNCTIONS_H
#include "gucefMATH_ComponentFunctions.h"
#define GUCEF_MATH_COMPONENTFUNCTIONS_H
#endif /* GUCEF_MATH_COMPONENTFUNCTIONS_H ? */

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

template < typename T > class CTVector4D;

/** Tag of the scalar backend; the SIMD configuration (section 5.2 of the design) adds the other tags */
struct SMathSimdScalar;

/*-------------------------------------------------------------------------*/

/**
 *  The canonical float evaluation order of the 4 component dot product, separate so that the primary template and
 *  every SIMD specialisation share one definition: ( px + pz ) + ( py + pw ) with p the component products.
 *  Other kinds use their component kernel (exact for fixed point, wrapping for integers).
 */
template < typename T, int IsFloat >
class CTVector4DotOrder
{
    public:

    static GUCEF_MATH_FORCE_INLINE T Dot( const T ( &a )[ 4 ], const T ( &b )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTMathComponentTraits< T >::TKernels::Dot( a, b );
    }
};

template < typename T >
class CTVector4DotOrder< T, 1 >
{
    public:

    static GUCEF_MATH_FORCE_INLINE T Dot( const T ( &a )[ 4 ], const T ( &b )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T px = a[ 0 ] * b[ 0 ];
        T py = a[ 1 ] * b[ 1 ];
        T pz = a[ 2 ] * b[ 2 ];
        T pw = a[ 3 ] * b[ 3 ];
        return ( px + pz ) + ( py + pw );
    }
};

/*-------------------------------------------------------------------------*/

/** Normalisation: floats with the canonical squared length and Chebyshev scaling outside the safe range */
template < typename T, int IsFloat >
class CTVector4Normalization
{
    public:

    static GUCEF_MATH_FORCE_INLINE bool Normalize( const T ( &a )[ 4 ], T ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTMathComponentTraits< T >::TKernels::Normalize( a, out );
    }
};

template < typename T >
class CTVector4Normalization< T, 1 >
{
    public:

    static GUCEF_MATH_FORCE_INLINE bool Normalize( const T ( &a )[ 4 ], T ( &out )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTMathComponentTraits< T >::TKernels::NormalizeWithSquaredLength( a, CTVector4DotOrder< T, 1 >::Dot( a, a ), out );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The operations of CTVector4D that SIMD backends may specialise (section 5.3 of the design), as static functions over
 *  the public data. This primary template is the scalar implementation for every kind and backend, and it defines the
 *  canonical evaluation order that every specialisation must reproduce bit for bit:
 *  - Add, Subtract, Negate, Multiply, Min, Max: per component (the component kernels: integers wrap);
 *  - Dot: floats ( px + pz ) + ( py + pw ) with p the component products (CTVector4DotOrder); other kinds their
 *    component kernel;
 *  - Normalize: floats divide each component by Sqrt( Dot( a, a ) ) when that squared length is in the safe range,
 *    otherwise they first divide by the largest magnitude; a zero or non-finite vector gives the zero vector.
 *  Specialisations exist for Float32 only and change implementations, never results, layouts or signatures.
 *  All functions are realtime-safe. In-place use (out aliasing an input) is allowed.
 */
template < typename T, typename TBackend = SMathSimdScalar >
class GUCEF_MATH_TEMPLATE_PUBLIC CTVector4Ops
{
    public:

    typedef CTVector4D< T > TVector;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;

    static GUCEF_MATH_FORCE_INLINE void Add( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        out = TVector( TKernels::Add( a.x, b.x ), TKernels::Add( a.y, b.y ), TKernels::Add( a.z, b.z ), TKernels::Add( a.w, b.w ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Subtract( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        out = TVector( TKernels::Subtract( a.x, b.x ), TKernels::Subtract( a.y, b.y ), TKernels::Subtract( a.z, b.z ), TKernels::Subtract( a.w, b.w ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Negate( const TVector& a, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        out = TVector( TKernels::Negate( a.x ), TKernels::Negate( a.y ), TKernels::Negate( a.z ), TKernels::Negate( a.w ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Multiply( const TVector& a, const T& factor, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        out = TVector( TKernels::Multiply( a.x, factor ), TKernels::Multiply( a.y, factor ), TKernels::Multiply( a.z, factor ), TKernels::Multiply( a.w, factor ) );
    }

    static GUCEF_MATH_FORCE_INLINE T Dot( const TVector& a, const TVector& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const T ca[ 4 ] = { a.x, a.y, a.z, a.w };
        const T cb[ 4 ] = { b.x, b.y, b.z, b.w };
        return CTVector4DotOrder< T, TTraits::IsFloatingPoint >::Dot( ca, cb );
    }

    static GUCEF_MATH_FORCE_INLINE void Min( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        out = TVector( TTraits::Min( a.x, b.x ), TTraits::Min( a.y, b.y ), TTraits::Min( a.z, b.z ), TTraits::Min( a.w, b.w ) );
    }

    static GUCEF_MATH_FORCE_INLINE void Max( const TVector& a, const TVector& b, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        out = TVector( TTraits::Max( a.x, b.x ), TTraits::Max( a.y, b.y ), TTraits::Max( a.z, b.z ), TTraits::Max( a.w, b.w ) );
    }

    static inline void Normalize( const TVector& a, TVector& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const T ca[ 4 ] = { a.x, a.y, a.z, a.w };
        T unit[ 4 ];
        if ( !CTVector4Normalization< T, TTraits::IsFloatingPoint >::Normalize( ca, unit ) )
        {
            out = TVector();
            return;
        }
        out = TVector( unit[ 0 ], unit[ 1 ], unit[ 2 ], unit[ 3 ] );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTVECTOR4OPS_H ? */
