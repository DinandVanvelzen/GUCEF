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

#ifndef GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#define GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTSPHERICALHARMONICS_H
#include "gucefMATH_CTSphericalHarmonics.h"
#define GUCEF_MATH_CTSPHERICALHARMONICS_H
#endif /* GUCEF_MATH_CTSPHERICALHARMONICS_H ? */

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

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
 *  The rotation of spherical-harmonic coefficients up to order Order: one (2l + 1) x (2l + 1) matrix per band, built
 *  from a 3x3 rotation with the Ivanic-Ruedenberg recursion (1996, with the 1998 corrections), O( Order^3 ).
 *
 *  - Meaning: for the rotation R, Apply maps the projection of a function f to the projection of f( R^-1 . ): the
 *    projection of a delta in direction d becomes the projection of a delta in R d. Band energies are preserved.
 *  - The band matrices act on the orthonormal, N3D and SN3D coefficients alike (they differ by a factor per band);
 *    FuMa coefficients must be converted first (CAmbisonicFormat).
 *  - Kind: T is Float32 or Float64; Order 0 ... GUCEF_MATH_SH_MAX_ORDER.
 *  - Layout: all band matrices in one fixed-size member array (row-major per band); no allocation. Building (Set) has
 *    bounded cost and is realtime-safe (block-rate use on the audio thread for low orders); Apply is a block-diagonal
 *    multiply, realtime-safe, and may run in place (in and out the same object).
 */
template < typename T, UInt32 Order >
class GUCEF_MATH_TEMPLATE_PUBLIC CTSphericalHarmonicsRotation
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTSphericalHarmonics< T, Order > TCoefficients;

    /** sum over l <= Order of ( 2l + 1 )^2 */
    enum { ElementCount = ( Order + 1 ) * ( 2 * Order + 1 ) * ( 2 * Order + 3 ) / 3 };

    GUCEF_MATH_STATIC_ASSERT( Order <= GUCEF_MATH_SH_MAX_ORDER, Spherical_harmonics_order_above_15 );
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Spherical_harmonics_require_a_floating_point_kind );

    /** The identity rotation */
    CTSphericalHarmonicsRotation( void ) GUCEF_MATH_REALTIME_CONTEXT { Set( CTMatrix3D< T >() ); }

    /** The rotation of the orthonormal matrix 'rotation' */
    explicit CTSphericalHarmonicsRotation( const CTMatrix3D< T >& rotation ) GUCEF_MATH_REALTIME_CONTEXT { Set( rotation ); }

    /** The rotation of the quaternion q (normalised by the conversion) */
    explicit CTSphericalHarmonicsRotation( const CTQuaternion< T >& q ) GUCEF_MATH_REALTIME_CONTEXT { Set( CTMatrix3D< T >::FromQuaternion( q ) ); }

    /** Builds the band matrices of the orthonormal matrix 'rotation' (a matrix that is not a rotation gives no rotation) */
    void Set( const CTMatrix3D< T >& rotation ) GUCEF_MATH_REALTIME_CONTEXT;

    /** out = the rotated coefficients of 'in'; in and out may be the same object */
    void Apply( const TCoefficients& in, TCoefficients& out ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Element (m, n) of the matrix of band l (-l <= m, n <= l); 0 outside the bands */
    GUCEF_MATH_FORCE_INLINE T GetElement( UInt32 l, Int32 m, Int32 n ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( l > Order || m < -(Int32) l || m > (Int32) l || n < -(Int32) l || n > (Int32) l )
            return TTraits::Zero();
        return m_elements[ Element( l, m, n ) ];
    }

    private:

    /** The offset of band l: sum over k < l of ( 2k + 1 )^2 = l ( 4 l^2 - 1 ) / 3 */
    static GUCEF_MATH_FORCE_INLINE UInt32 BandOffset( UInt32 l ) GUCEF_MATH_REALTIME_CONTEXT { return l * ( 4 * l * l - 1 ) / 3; }

    static GUCEF_MATH_FORCE_INLINE UInt32 Element( UInt32 l, Int32 m, Int32 n ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return BandOffset( l ) + (UInt32) ( m + (Int32) l ) * ( 2 * l + 1 ) + (UInt32) ( n + (Int32) l );
    }

    /** The helper P of Ivanic-Ruedenberg: i in -1 ... 1 selects a row of band 1, (a, b) an element of band l - 1 */
    T P( Int32 i, UInt32 l, Int32 a, Int32 b ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        const T ri1 = m_elements[ Element( 1, i, 1 ) ];
        const T rim1 = m_elements[ Element( 1, i, -1 ) ];
        const T ri0 = m_elements[ Element( 1, i, 0 ) ];
        const Int32 lp = (Int32) l - 1;
        if ( b == (Int32) l )
            return ri1 * m_elements[ Element( l - 1, a, lp ) ] - rim1 * m_elements[ Element( l - 1, a, -lp ) ];
        if ( b == -(Int32) l )
            return ri1 * m_elements[ Element( l - 1, a, -lp ) ] + rim1 * m_elements[ Element( l - 1, a, lp ) ];
        return ri0 * m_elements[ Element( l - 1, a, b ) ];
    }

    T m_elements[ ElementCount ];
};

/*-------------------------------------------------------------------------*/

typedef CTSphericalHarmonicsRotation< Float32, 1 > TFloat32SphericalHarmonicsRotation1;
typedef CTSphericalHarmonicsRotation< Float32, 2 > TFloat32SphericalHarmonicsRotation2;
typedef CTSphericalHarmonicsRotation< Float32, 3 > TFloat32SphericalHarmonicsRotation3;
typedef CTSphericalHarmonicsRotation< Float64, 1 > TFloat64SphericalHarmonicsRotation1;
typedef CTSphericalHarmonicsRotation< Float64, 2 > TFloat64SphericalHarmonicsRotation2;
typedef CTSphericalHarmonicsRotation< Float64, 3 > TFloat64SphericalHarmonicsRotation3;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T, UInt32 Order >
GUCEF_MATH_PRECISE_INLINE void
CTSphericalHarmonicsRotation< T, Order >::Set( const CTMatrix3D< T >& rotation ) GUCEF_MATH_REALTIME_CONTEXT
{
    m_elements[ 0 ] = TTraits::One();
    if ( 0 == Order )
        return;
    // band 1 acts on ( Y_1,-1, Y_10, Y_11 ) = ( y, z, x ): element (m, n) = R( axis( m ), axis( n ) )
    const UInt32 axis[ 3 ] = { 1, 2, 0 };
    for ( Int32 m=-1; m<=1; ++m )
        for ( Int32 n=-1; n<=1; ++n )
            m_elements[ Element( 1, m, n ) ] = rotation( axis[ m + 1 ], axis[ n + 1 ] );

    for ( UInt32 l=2; l<=Order; ++l )
    {
        const Int32 li = (Int32) l;
        for ( Int32 m=-li; m<=li; ++m )
        {
            const Int32 am = m < 0 ? -m : m;
            const bool mIsZero = 0 == m;
            for ( Int32 n=-li; n<=li; ++n )
            {
                const Int32 an = n < 0 ? -n : n;
                const Float64 denominator = an == li ? (Float64) ( 2 * li ) * (Float64) ( 2 * li - 1 ) : (Float64) ( li + n ) * (Float64) ( li - n );
                const Float64 u = sqrt( (Float64) ( li + m ) * (Float64) ( li - m ) / denominator );
                const Float64 v = 0.5 * sqrt( ( mIsZero ? 2.0 : 1.0 ) * (Float64) ( li + am - 1 ) * (Float64) ( li + am ) / denominator ) * ( mIsZero ? -1.0 : 1.0 );
                const Float64 w = mIsZero ? 0.0 : -0.5 * sqrt( (Float64) ( li - am - 1 ) * (Float64) ( li - am ) / denominator );

                T value = TTraits::Zero();
                if ( am < li )    // u = 0 for |m| = l
                    value = value + (T) u * P( 0, l, m, n );
                if ( 0.0 != v )   // v is never 0
                {
                    T vTerm;
                    if ( mIsZero )
                        vTerm = P( 1, l, 1, n ) + P( -1, l, -1, n );
                    else if ( m > 0 )
                        vTerm = 1 == m ? P( 1, l, m - 1, n ) * TTraits::Sqrt2() : P( 1, l, m - 1, n ) - P( -1, l, -m + 1, n );
                    else
                        vTerm = -1 == m ? P( -1, l, -m - 1, n ) * TTraits::Sqrt2() : P( 1, l, m + 1, n ) + P( -1, l, -m - 1, n );
                    value = value + (T) v * vTerm;
                }
                if ( !mIsZero && am < li - 1 )    // w = 0 otherwise
                {
                    const T wTerm = m > 0 ? P( 1, l, m + 1, n ) + P( -1, l, -m - 1, n ) : P( 1, l, m - 1, n ) - P( -1, l, -m + 1, n );
                    value = value + (T) w * wTerm;
                }
                m_elements[ Element( l, m, n ) ] = value;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Order >
GUCEF_MATH_PRECISE_INLINE void
CTSphericalHarmonicsRotation< T, Order >::Apply( const TCoefficients& in, TCoefficients& out ) const GUCEF_MATH_REALTIME_CONTEXT
{
    T band[ 2 * Order + 1 ];
    out.coefficients[ 0 ] = in.coefficients[ 0 ];
    for ( UInt32 l=1; l<=Order; ++l )
    {
        const UInt32 width = 2 * l + 1;
        const UInt32 first = l * l;
        for ( UInt32 i=0; i<width; ++i )
            band[ i ] = in.coefficients[ first + i ];
        const T* matrix = m_elements + BandOffset( l );
        for ( UInt32 row=0; row<width; ++row )
            out.coefficients[ first + row ] = CTMatrixKernels< T >::Dot( matrix + row * width, 1, band, 1, width );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTSPHERICALHARMONICSROTATION_MEMBERS( prefix, kind, order ) \
    prefix void CTSphericalHarmonicsRotation< kind, order >::Set( const CTMatrix3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTSphericalHarmonicsRotation< kind, order >::Apply( const CTSphericalHarmonics< kind, order >&, CTSphericalHarmonics< kind, order >& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTSPHERICALHARMONICSROTATION_KIND( prefix, kind ) \
    GUCEF_MATH_CTSPHERICALHARMONICSROTATION_MEMBERS( prefix, kind, 1 ) \
    GUCEF_MATH_CTSPHERICALHARMONICSROTATION_MEMBERS( prefix, kind, 2 ) \
    GUCEF_MATH_CTSPHERICALHARMONICSROTATION_MEMBERS( prefix, kind, 3 )

#define GUCEF_MATH_CTSPHERICALHARMONICSROTATION_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTSPHERICALHARMONICSROTATION_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTSPHERICALHARMONICSROTATION_KIND( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTSPHERICALHARMONICSROTATION_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H ? */
