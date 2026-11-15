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

#ifndef GUCEF_MATH_CTSPHERICALHARMONICS_H
#define GUCEF_MATH_CTSPHERICALHARMONICS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_ETYPES_H
#include "gucefMATH_ETypes.h"
#define GUCEF_MATH_ETYPES_H
#endif /* GUCEF_MATH_ETYPES_H ? */

#ifndef GUCEF_MATH_CTMATRIX_H
#include "gucefMATH_CTMatrix.h"
#define GUCEF_MATH_CTMATRIX_H
#endif /* GUCEF_MATH_CTMATRIX_H ? */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#include "gucefMATH_CTDirectionVector3D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The highest spherical-harmonic order of the evaluation functions (the normalisation table covers it) */
#define GUCEF_MATH_SH_MAX_ORDER     15

/** The highest order of the FuMa normalisation and channel order */
#define GUCEF_MATH_SH_FUMA_MAX_ORDER    3

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The basis and normalisation of the real spherical harmonics (shared by CTSphericalHarmonics and CAmbisonicFormat).
 *
 *  Convention: real spherical harmonics without the Condon-Shortley phase (as in AmbiX), the polar axis +Z, azimuth
 *  measured from +X towards +Y; ACN channel index l^2 + l + m. In SN3D: Y_00 = 1, Y_1,-1 = y, Y_10 = z, Y_11 = x for a
 *  unit direction (x, y, z). For ambisonics the frame is x front, y left, z up (CAmbisonicFormat converts azimuths).
 *  Normalisations, as factors relative to SN3D (Schmidt semi-normalised):
 *  - SHNORM_N3D: sqrt( 2 l + 1 ) (the mean of Y^2 over the sphere is 1)
 *  - SHNORM_ORTHONORMAL: sqrt( ( 2 l + 1 ) / 4 pi ) (the integral of Y^2 over the sphere is 1; Y_00 = 0.28209479)
 *  - SHNORM_FUMA: the Furse-Malham weights (maximum magnitude 1, W 1 / sqrt( 2 )), defined up to order 3; higher orders
 *    assert in debug builds and use the SN3D value.
 *  Evaluation uses the stable associated-Legendre recurrences in z with the Cartesian recurrence for
 *  ( x + i y )^m: no trigonometric calls. Realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTSphericalHarmonicsBasis
{
    public:

    typedef CTNumericTraits< T > TTraits;

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Spherical_harmonics_require_a_floating_point_kind );

    /** The ACN index of (l, m), -l <= m <= l */
    static GUCEF_MATH_FORCE_INLINE UInt32 GetIndex( UInt32 l, Int32 m ) GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) ( (Int32) ( l * l + l ) + m ); }

    /** The degree l of ACN index 'index' */
    static GUCEF_MATH_FORCE_INLINE UInt32 GetDegree( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt32 l = 0;
        while ( ( l + 1 ) * ( l + 1 ) <= index )
            ++l;
        return l;
    }

    /** The factor of normalisation 'norm' relative to SN3D for ACN index 'index' */
    static GUCEF_MATH_FORCE_INLINE T GetNormalisationFactor( UInt32 index, TShNormalisation norm ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return GetFactor( GetDegree( index ), index, norm );
    }

    /**
     *  The ( order + 1 )^2 basis values in ACN order for the unit direction (x, y, z) (order <= GUCEF_MATH_SH_MAX_ORDER;
     *  higher orders assert and write nothing)
     */
    static void Evaluate( const T& x, const T& y, const T& z, UInt32 order, TShNormalisation norm, T* out ) GUCEF_MATH_REALTIME_CONTEXT;

    private:

    /** sqrt( ( 2 - delta_m0 ) ( l - m )! / ( l + m )! ) for l <= 15, 0 <= m <= l, at l ( l + 1 ) / 2 + m */
    static Float64 GetSn3dFactor( UInt32 l, UInt32 m ) GUCEF_MATH_REALTIME_CONTEXT;

    /** GetNormalisationFactor for the ACN index 'index' of degree l */
    static T GetFactor( UInt32 l, UInt32 index, TShNormalisation norm ) GUCEF_MATH_REALTIME_CONTEXT;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A function on the sphere as real spherical-harmonic coefficients up to order Order (bands 0 ... Order), in ACN order.
 *
 *  - Kind: T is Float32 or Float64. Order 0 ... GUCEF_MATH_SH_MAX_ORDER (15); tested to order 7.
 *  - Layout: the public T coefficients[ ( Order + 1 )^2 ], index l^2 + l + m; no allocation.
 *  - Basis and normalisations: CTSphericalHarmonicsBasis (no Condon-Shortley phase, polar axis +Z). The normalisation is
 *    a parameter of the members that evaluate the basis; the coefficients carry no normalisation tag.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "(c0, c1, ...)"; DataNode attributes c0 ... c<n - 1>.
 */
template < typename T, UInt32 Order >
class GUCEF_MATH_TEMPLATE_PUBLIC CTSphericalHarmonics
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTSphericalHarmonicsBasis< T > TBasis;

    enum { OrderValue = Order, BandCount = Order + 1, CoefficientCount = ( Order + 1 ) * ( Order + 1 ) };

    GUCEF_MATH_STATIC_ASSERT( Order <= GUCEF_MATH_SH_MAX_ORDER, Spherical_harmonics_order_above_15 );
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Spherical_harmonics_require_a_floating_point_kind );

    T coefficients[ CoefficientCount ];

    /** All coefficients zero */
    CTSphericalHarmonics( void ) GUCEF_MATH_REALTIME_CONTEXT { SetZero(); }

    /** The ACN index of (l, m) */
    static GUCEF_MATH_FORCE_INLINE UInt32 GetIndex( UInt32 l, Int32 m ) GUCEF_MATH_REALTIME_CONTEXT { return TBasis::GetIndex( l, m ); }

    /** The CoefficientCount basis values of 'direction' in ACN order */
    static void EvaluateBasis( const CTDirectionVector3D< T >& direction, TShNormalisation norm, T* out ) GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE void SetZero( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<CoefficientCount; ++i )
            coefficients[ i ] = TTraits::Zero();
    }

    /** Adds the projection of a delta function in 'direction' with the given weight: coefficients += weight Y( direction ) */
    void AddDirection( const CTDirectionVector3D< T >& direction, const T& weight, TShNormalisation norm ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The value of the function in 'direction': sum c_i Y_i( direction ) */
    T Evaluate( const CTDirectionVector3D< T >& direction, TShNormalisation norm ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Coefficient-wise sum */
    GUCEF_MATH_FORCE_INLINE void Add( const CTSphericalHarmonics& other ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<CoefficientCount; ++i )
            coefficients[ i ] = coefficients[ i ] + other.coefficients[ i ];
    }

    /** Every coefficient times 'factor' */
    GUCEF_MATH_FORCE_INLINE void Scale( const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<CoefficientCount; ++i )
            coefficients[ i ] = coefficients[ i ] * factor;
    }

    /** sum a_i b_i: the integral of the product of the two functions for orthonormal coefficients */
    T Dot( const CTSphericalHarmonics& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Convolution with the clamped cosine lobe max( 0, cos ): radiance to irradiance (Ramamoorthi and Hanrahan 2001).
     *  Band l is multiplied by A_0 = pi, A_1 = 2 pi / 3, A_l = 0 for odd l > 1 and
     *  A_l = 2 pi ( -1 )^( l / 2 - 1 ) / ( ( l + 2 ) ( l - 1 ) ) l! / ( 2^l ( ( l / 2 )! )^2 ) for even l. The factors are
     *  per coefficient, so the result holds in every normalisation.
     */
    void ConvolveWithCosineLobe( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /** sum over m of c_lm^2 (invariant under rotation for the orthonormal, N3D and SN3D normalisations); 0 beyond Order */
    T GetBandEnergy( UInt32 l ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Every coefficient within |a - b| <= max( absolute, relative max( |a|, |b| ) ) */
    bool Equals( const CTSphericalHarmonics& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<CoefficientCount; ++i )
            if ( !TTraits::EqualsWithin( coefficients[ i ], other.coefficients[ i ], absolute, relative ) )
                return false;
        return true;
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(c0, c1, ...)" in ACN order */
    CORE::CString ToString( void ) const { return ComponentsToString( coefficients ); }

    /** Parses the ToString form (exactly CoefficientCount values); false leaves the coefficients unchanged */
    bool FromString( const CORE::CString& text ) { return ComponentsFromString( text, coefficients ); }

    /** Writes the attributes c0 ... c<n - 1> */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        char storage[ CoefficientCount ][ 5 ];
        const char* names[ CoefficientCount ];
        MakeNames( storage, names );
        return SerializeComponents( node, names, coefficients );
    }

    /** Reads the attributes c0 ... c<n - 1>; a missing or invalid attribute fails and leaves the coefficients unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        char storage[ CoefficientCount ][ 5 ];
        const char* names[ CoefficientCount ];
        MakeNames( storage, names );
        return DeserializeComponents( node, names, coefficients );
    }

    private:

    static void MakeNames( char ( &storage )[ CoefficientCount ][ 5 ], const char* ( &names )[ CoefficientCount ] )
    {
        for ( UInt32 i=0; i<CoefficientCount; ++i )
        {
            char* name = storage[ i ];
            UInt32 position = 0;
            name[ position++ ] = 'c';
            if ( i >= 100 )
                name[ position++ ] = (char) ( '0' + i / 100 );
            if ( i >= 10 )
                name[ position++ ] = (char) ( '0' + ( i / 10 ) % 10 );
            name[ position++ ] = (char) ( '0' + i % 10 );
            name[ position ] = '\0';
            names[ i ] = name;
        }
    }
};

/*-------------------------------------------------------------------------*/

typedef CTSphericalHarmonics< Float32, 1 > TFloat32SphericalHarmonics1;
typedef CTSphericalHarmonics< Float32, 2 > TFloat32SphericalHarmonics2;
typedef CTSphericalHarmonics< Float32, 3 > TFloat32SphericalHarmonics3;
typedef CTSphericalHarmonics< Float64, 1 > TFloat64SphericalHarmonics1;
typedef CTSphericalHarmonics< Float64, 2 > TFloat64SphericalHarmonics2;
typedef CTSphericalHarmonics< Float64, 3 > TFloat64SphericalHarmonics3;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE Float64
CTSphericalHarmonicsBasis< T >::GetSn3dFactor( UInt32 l, UInt32 m ) GUCEF_MATH_REALTIME_CONTEXT
{
    // sqrt( ( 2 - delta_m0 ) ( l - m )! / ( l + m )! ), generated with exact rational arithmetic
    static const Float64 factors[ 136 ] =
    {
        1, 1, 1, 1, 0.57735026918962573, 0.28867513459481287,
        1, 0.40824829046386302, 0.12909944487358058, 0.052704627669472988, 1, 0.31622776601683794,
        0.074535599249992993, 0.019920476822239894, 0.0070429521227376385, 1, 0.25819888974716115, 0.048795003647426657,
        0.0099602384111199468, 0.0023476507075792128, 0.00074239233864562329, 1, 0.21821789023599239, 0.034503277967117711,
        0.0057505463278529519, 0.00104990131391452, 0.00022383971222927231, 6.4616959055449361e-05, 1, 0.18898223650461363,
        0.025717224993681984, 0.0036369648372665399, 0.00054829307913314084, 9.1382179855523473e-05, 1.7921519933767681e-05, 4.7897276744570195e-06,
        1, 0.16666666666666666, 0.019920476822239894, 0.0024520411930687488, 0.00031655715683232764, 4.3898579252848194e-05,
        6.7736978372908615e-06, 1.2367023677360544e-06, 3.0917559193401361e-07, 1, 0.14907119849998599, 0.015891043154093204,
        0.0017338549553676647, 0.00019632041465006097, 2.3464777618614389e-05, 3.0292897646451355e-06, 4.3724031526781175e-07, 7.4986095435694221e-08,
        1.7674392192427002e-08, 1, 0.13483997249264842, 0.012974982402692049, 0.0012723017011509569, 0.00012852188008557455,
        1.3547395674581723e-05, 1.5146448823225678e-06, 1.8367767162109348e-07, 2.4995365145231406e-08, 4.0547836555417679e-09, 9.0667718878464845e-10,
        1, 0.12309149097933274, 0.010795837927188263, 0.0009617696839685294, 8.7797158505696388e-05, 8.2960516865782808e-06,
        8.2143151938729805e-07, 8.6586484770554052e-08, 9.9321509734544384e-09, 1.2822351770735609e-09, 1.9785318326168655e-10, 4.2182440404629442e-11,
        1, 0.11322770341445958, 0.009124148357522649, 0.00074498359377945698, 6.2081966148288086e-05, 5.3234846786482678e-06,
        4.742537088390998e-07, 4.4417929478869668e-08, 4.4417929478869666e-09, 4.8463934297650154e-10, 5.9654979314221791e-11, 8.7956468444687844e-12,
        1.7954038938891261e-12, 1, 0.10482848367219183, 0.0078134538489749231, 0.0005889612439501829, 4.5171265389778545e-05,
        3.5489897857655119e-06, 2.8786108331272091e-07, 2.4328701933250604e-08, 2.1673730309325855e-09, 2.0665091019340504e-10, 2.1544846726669515e-11,
        2.5390845366754677e-12, 3.5908077877782525e-13, 7.0421534536316344e-14, 1, 0.097590007294853315, 0.0067666495245095847,
        0.00047376022823841866, 3.3668673347076119e-05, 2.4425822886015681e-06, 1.8205933459833719e-07, 1.4046182610196406e-08, 1.1318736504190221e-09,
        9.6351483691293726e-11, 8.7956468444687844e-12, 8.795646844468784e-13, 9.9591089224383868e-14, 1.3552630640429589e-14, 2.5612064489493378e-15,
        1, 0.091287092917527679, 0.0059172633541393861, 0.00038682360653620798, 2.5618019166690232e-05, 1.7271664998763253e-06,
        1.1918581168145326e-07, 8.4701668099702388e-09, 6.2442898171014845e-10, 4.8175741845646863e-11, 3.9335328500627447e-12, 3.4499365303551638e-13,
        3.3197029741461294e-14, 3.6220928961415874e-15, 4.7560409767889337e-16, 8.6833030913540107e-17
    };
    return factors[ l * ( l + 1 ) / 2 + m ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTSphericalHarmonicsBasis< T >::GetFactor( UInt32 l, UInt32 index, TShNormalisation norm ) GUCEF_MATH_REALTIME_CONTEXT
{
    // sqrt( 2 l + 1 ) and 1 / sqrt( 4 pi )
    static const Float64 sqrtTwoLPlusOne[ GUCEF_MATH_SH_MAX_ORDER + 1 ] =
    {
        1.0, 1.7320508075688772, 2.2360679774997898, 2.6457513110645907, 3.0, 3.3166247903553998, 3.6055512754639891,
        3.872983346207417, 4.1231056256176606, 4.358898943540674, 4.5825756949558398, 4.7958315233127191, 5.0,
        5.196152422706632, 5.3851648071345037, 5.5677643628300215
    };
    // the Furse-Malham weights relative to SN3D by ACN index (orders 0 to 3)
    static const Float64 fuma[ 16 ] =
    {
        0.70710678118654752,
        1.0, 1.0, 1.0,
        1.1547005383792515, 1.1547005383792515, 1.0, 1.1547005383792515, 1.1547005383792515,
        1.2649110640673518, 1.3416407864998738, 1.1858541225631423, 1.0, 1.1858541225631423, 1.3416407864998738, 1.2649110640673518
    };
    const Float64 inverseSqrtFourPi = 0.28209479177387814;
    if ( l > GUCEF_MATH_SH_MAX_ORDER )
        return TTraits::One();
    switch ( norm )
    {
        case SHNORM_N3D: return (T) sqrtTwoLPlusOne[ l ];
        case SHNORM_ORTHONORMAL: return (T) ( sqrtTwoLPlusOne[ l ] * inverseSqrtFourPi );
        case SHNORM_FUMA:
        {
            GUCEF_MATH_ASSERT( index < 16 );
            return index < 16 ? (T) fuma[ index ] : TTraits::One();
        }
        case SHNORM_SN3D:
        default: return TTraits::One();
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTSphericalHarmonicsBasis< T >::Evaluate( const T& x, const T& y, const T& z, UInt32 order, TShNormalisation norm, T* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( order <= GUCEF_MATH_SH_MAX_ORDER );
    if ( order > GUCEF_MATH_SH_MAX_ORDER )
        return;
    // Re and Im of ( x + i y )^m
    T cosine[ GUCEF_MATH_SH_MAX_ORDER + 1 ];
    T sine[ GUCEF_MATH_SH_MAX_ORDER + 1 ];
    cosine[ 0 ] = TTraits::One();
    sine[ 0 ] = TTraits::Zero();
    for ( UInt32 m=1; m<=order; ++m )
    {
        cosine[ m ] = x * cosine[ m - 1 ] - y * sine[ m - 1 ];
        sine[ m ] = x * sine[ m - 1 ] + y * cosine[ m - 1 ];
    }
    // P_l^m( z ) / sin^m: P_m^m = ( 2m - 1 )!!, P_m+1^m = ( 2m + 1 ) z P_m^m,
    // P_l^m = ( ( 2l - 1 ) z P_l-1^m - ( l + m - 1 ) P_l-2^m ) / ( l - m )
    T diagonal = TTraits::One();
    for ( UInt32 m=0; m<=order; ++m )
    {
        if ( m > 0 )
            diagonal = diagonal * (T) ( 2 * m - 1 );
        T previous = TTraits::Zero();
        T current = diagonal;
        for ( UInt32 l=m; l<=order; ++l )
        {
            if ( l == m + 1 )
            {
                previous = current;
                current = (T) ( 2 * m + 1 ) * z * diagonal;
            }
            else if ( l > m + 1 )
            {
                const T next = ( (T) ( 2 * l - 1 ) * z * current - (T) ( l + m - 1 ) * previous ) / (T) ( l - m );
                previous = current;
                current = next;
            }
            const T k = (T) GetSn3dFactor( l, m ) * current;
            if ( 0 == m )
            {
                const UInt32 index = l * l + l;
                out[ index ] = k * GetFactor( l, index, norm );
            }
            else
            {
                const UInt32 positive = l * l + l + m;
                const UInt32 negative = l * l + l - m;
                out[ positive ] = k * cosine[ m ] * GetFactor( l, positive, norm );
                out[ negative ] = k * sine[ m ] * GetFactor( l, negative, norm );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Order >
GUCEF_MATH_PRECISE_INLINE void
CTSphericalHarmonics< T, Order >::EvaluateBasis( const CTDirectionVector3D< T >& direction, TShNormalisation norm, T* out ) GUCEF_MATH_REALTIME_CONTEXT
{
    TBasis::Evaluate( direction.GetX(), direction.GetY(), direction.GetZ(), Order, norm, out );
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Order >
GUCEF_MATH_PRECISE_INLINE void
CTSphericalHarmonics< T, Order >::AddDirection( const CTDirectionVector3D< T >& direction, const T& weight, TShNormalisation norm ) GUCEF_MATH_REALTIME_CONTEXT
{
    T basis[ CoefficientCount ];
    TBasis::Evaluate( direction.GetX(), direction.GetY(), direction.GetZ(), Order, norm, basis );
    for ( UInt32 i=0; i<CoefficientCount; ++i )
        coefficients[ i ] = coefficients[ i ] + weight * basis[ i ];
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Order >
GUCEF_MATH_PRECISE_INLINE T
CTSphericalHarmonics< T, Order >::Evaluate( const CTDirectionVector3D< T >& direction, TShNormalisation norm ) const GUCEF_MATH_REALTIME_CONTEXT
{
    T basis[ CoefficientCount ];
    TBasis::Evaluate( direction.GetX(), direction.GetY(), direction.GetZ(), Order, norm, basis );
    return CTMatrixKernels< T >::Dot( coefficients, 1, basis, 1, CoefficientCount );
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Order >
GUCEF_MATH_PRECISE_INLINE T
CTSphericalHarmonics< T, Order >::Dot( const CTSphericalHarmonics& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrixKernels< T >::Dot( coefficients, 1, other.coefficients, 1, CoefficientCount );
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Order >
GUCEF_MATH_PRECISE_INLINE void
CTSphericalHarmonics< T, Order >::ConvolveWithCosineLobe( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    const Float64 pi = 3.14159265358979323846264338327950288;
    for ( UInt32 l=0; l<=Order; ++l )
    {
        Float64 factor = 0.0;
        if ( 0 == l )
            factor = pi;
        else if ( 1 == l )
            factor = 2.0 * pi / 3.0;
        else if ( 0 == l % 2 )
        {
            // l! / ( 2^l ( ( l / 2 )! )^2 ) = prod_{k=1}^{l/2} ( 2k - 1 ) / ( 2k )
            Float64 ratio = 1.0;
            for ( UInt32 k=1; k<=l/2; ++k )
                ratio *= (Float64) ( 2 * k - 1 ) / (Float64) ( 2 * k );
            const Float64 sign = 0 == ( l / 2 ) % 2 ? -1.0 : 1.0;
            factor = 2.0 * pi * sign * ratio / ( (Float64) ( l + 2 ) * (Float64) ( l - 1 ) );
        }
        for ( Int32 m=-(Int32) l; m<=(Int32) l; ++m )
        {
            const UInt32 index = GetIndex( l, m );
            coefficients[ index ] = coefficients[ index ] * (T) factor;
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Order >
GUCEF_MATH_PRECISE_INLINE T
CTSphericalHarmonics< T, Order >::GetBandEnergy( UInt32 l ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( l > Order )
        return TTraits::Zero();
    const UInt32 first = l * l;
    return CTMatrixKernels< T >::Dot( coefficients + first, 1, coefficients + first, 1, 2 * l + 1 );
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

/*
 *  The basis functions for Float32 and Float64 (they serve every order) and the members of the orders 1 to 3 (the
 *  orders of the typedefs and of first- to third-order ambisonics); other orders instantiate in their user's code.
 */

#define GUCEF_MATH_CTSPHERICALHARMONICSBASIS_MEMBERS( prefix, kind ) \
    prefix Float64 CTSphericalHarmonicsBasis< kind >::GetSn3dFactor( UInt32, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTSphericalHarmonicsBasis< kind >::GetFactor( UInt32, UInt32, TShNormalisation ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTSphericalHarmonicsBasis< kind >::Evaluate( const kind&, const kind&, const kind&, UInt32, TShNormalisation, kind* ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTSPHERICALHARMONICS_MEMBERS( prefix, kind, order ) \
    prefix void CTSphericalHarmonics< kind, order >::EvaluateBasis( const CTDirectionVector3D< kind >&, TShNormalisation, kind* ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTSphericalHarmonics< kind, order >::AddDirection( const CTDirectionVector3D< kind >&, const kind&, TShNormalisation ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTSphericalHarmonics< kind, order >::Evaluate( const CTDirectionVector3D< kind >&, TShNormalisation ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTSphericalHarmonics< kind, order >::Dot( const CTSphericalHarmonics< kind, order >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTSphericalHarmonics< kind, order >::ConvolveWithCosineLobe( void ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTSphericalHarmonics< kind, order >::GetBandEnergy( UInt32 ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTSPHERICALHARMONICS_KIND( prefix, kind ) \
    GUCEF_MATH_CTSPHERICALHARMONICSBASIS_MEMBERS( prefix, kind ) \
    GUCEF_MATH_CTSPHERICALHARMONICS_MEMBERS( prefix, kind, 1 ) \
    GUCEF_MATH_CTSPHERICALHARMONICS_MEMBERS( prefix, kind, 2 ) \
    GUCEF_MATH_CTSPHERICALHARMONICS_MEMBERS( prefix, kind, 3 )

#define GUCEF_MATH_CTSPHERICALHARMONICS_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTSPHERICALHARMONICS_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTSPHERICALHARMONICS_KIND( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTSPHERICALHARMONICS_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTSPHERICALHARMONICS_H ? */
