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

#ifndef GUCEF_MATH_CTEULERANGLES_H
#define GUCEF_MATH_CTEULERANGLES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX2D_H
#include "gucefMATH_CTMatrix2D.h"
#define GUCEF_MATH_CTMATRIX2D_H
#endif /* GUCEF_MATH_CTMATRIX2D_H ? */

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

template < typename T > class CTQuaternion;
template < typename T > class CTMatrix3D;

/*-------------------------------------------------------------------------*/

/**
 *  Three rotation angles with their order and frame (sections 4.1 and 4.5.4 of the design).
 *
 *  - Kind: T needs a square root and transcendentals (floats and fixed point).
 *  - Meaning: the angles are stored in sequence order. For the order ( A, B, C ), intrinsic angles give
 *    R = R_A( first ) * R_B( second ) * R_C( third ) (rotations about the axes of the rotating body), extrinsic angles
 *    give R = R_C( third ) * R_B( second ) * R_A( first ) (rotations about the fixed axes). Elementary rotations follow
 *    the right-hand rule. All 6 Tait-Bryan and 6 proper Euler orders are supported.
 *  - YawPitchRoll( yaw, pitch, roll ) is intrinsic Y-X-Z: in the GUCEF default convention a positive yaw turns left.
 *  - Extraction (FromQuaternion, FromRotationMatrix): first and third in (-pi, pi], second in [-pi/2, pi/2]
 *    (Tait-Bryan) or [0, pi] (proper). At the singularity (second = +-90 degrees, or 0 / 180 degrees for proper
 *    orders) the third angle is set to 0 and the rotation is folded into the first, deterministically.
 *  - Layout: three CTRadian< T > then the order and the frame; no virtual functions.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "(YXZ, intrinsic, <a> rad, <b> rad, <c> rad)" ("deg" accepted on input); DataNode attributes order, frame,
 *    first, second, third (radians; firstDegrees, secondDegrees, thirdDegrees accepted on input).
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTEulerAngles
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;

    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt && TTraits::HasTranscendentals, Euler_angles_require_a_numeric_kind_with_sqrt_and_transcendentals );

    CTRadian< T > first;
    CTRadian< T > second;
    CTRadian< T > third;
    TEulerOrder order;
    TEulerFrame frame;

    /** Zero angles, intrinsic Y-X-Z (the YawPitchRoll order) */
    CTEulerAngles( void )
        : first(), second(), third(), order( EULERORDER_YXZ ), frame( EULERFRAME_INTRINSIC )
    {
    }

    CTEulerAngles( const CTRadian< T >& firstAngle, const CTRadian< T >& secondAngle, const CTRadian< T >& thirdAngle, TEulerOrder angleOrder, TEulerFrame angleFrame )
        : first( firstAngle ), second( secondAngle ), third( thirdAngle ), order( angleOrder ), frame( angleFrame )
    {
    }

    /** Intrinsic Y-X-Z: yaw about up, then pitch about the turned right axis, then roll about the turned view axis */
    static CTEulerAngles YawPitchRoll( const CTRadian< T >& yaw, const CTRadian< T >& pitch, const CTRadian< T >& roll ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTEulerAngles( yaw, pitch, roll, EULERORDER_YXZ, EULERFRAME_INTRINSIC );
    }

    /** The rotation as a quaternion: the product of the elementary quaternions in the order of the frame */
    CTQuaternion< T > ToQuaternion( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The rotation as a matrix: the product of the elementary rotations in the order of the frame */
    CTMatrix3D< T > ToRotationMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The angles of rotation q (unit) in the given order and frame (see the class comment for ranges and singularities) */
    static CTEulerAngles FromQuaternion( const CTQuaternion< T >& q, TEulerOrder angleOrder, TEulerFrame angleFrame ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The angles of rotation matrix m (orthonormal, determinant 1) in the given order and frame */
    static CTEulerAngles FromRotationMatrix( const CTMatrix3D< T >& m, TEulerOrder angleOrder, TEulerFrame angleFrame ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The axes ( 0 = X, 1 = Y, 2 = Z ) of the order in sequence; proper orders repeat the first axis */
    static void GetAxes( TEulerOrder angleOrder, UInt32& a, UInt32& b, UInt32& c ) GUCEF_MATH_REALTIME_CONTEXT
    {
        static const UInt8 axes[ 12 ][ 3 ] = { { 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 }, { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 },
                                               { 0, 1, 0 }, { 0, 2, 0 }, { 1, 0, 1 }, { 1, 2, 1 }, { 2, 0, 2 }, { 2, 1, 2 } };
        UInt32 index = (UInt32) angleOrder < 12 ? (UInt32) angleOrder : 0;
        a = axes[ index ][ 0 ];
        b = axes[ index ][ 1 ];
        c = axes[ index ][ 2 ];
    }

    /** True for the proper Euler orders (first and third axis equal) */
    static GUCEF_MATH_FORCE_INLINE bool IsProperOrder( TEulerOrder angleOrder ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return (UInt32) angleOrder >= (UInt32) EULERORDER_XYX;
    }

    friend bool operator==( const CTEulerAngles& a, const CTEulerAngles& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.first == b.first && a.second == b.second && a.third == b.third && a.order == b.order && a.frame == b.frame;
    }

    friend bool operator!=( const CTEulerAngles& a, const CTEulerAngles& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** The name of an order, "XYZ" ... "ZYZ" */
    static const char* GetOrderName( TEulerOrder angleOrder )
    {
        static const char* const names[ 12 ] = { "XYZ", "XZY", "YXZ", "YZX", "ZXY", "ZYX", "XYX", "XZX", "YXY", "YZY", "ZXZ", "ZYZ" };
        return (UInt32) angleOrder < 12 ? names[ angleOrder ] : names[ 0 ];
    }

    /** The order of a name of GetOrderName; false leaves 'angleOrder' unchanged */
    static bool OrderFromName( const CORE::CString& name, TEulerOrder& angleOrder )
    {
        for ( UInt32 i=0; i<12; ++i )
        {
            if ( name == GetOrderName( (TEulerOrder) i ) )
            {
                angleOrder = (TEulerOrder) i;
                return true;
            }
        }
        return false;
    }

    /** "(YXZ, intrinsic, <first> rad, <second> rad, <third> rad)" */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( "(" );
        text += GetOrderName( order );
        text += EULERFRAME_EXTRINSIC == frame ? ", extrinsic, " : ", intrinsic, ";
        text += MathAngleToString( first );
        text += ", ";
        text += MathAngleToString( second );
        text += ", ";
        text += MathAngleToString( third );
        text += ')';
        return text;
    }

    /** Parses the ToString form (angles in "rad" or "deg"); false leaves this value unchanged */
    bool FromString( const CORE::CString& text )
    {
        CORE::CString parts[ 5 ];
        if ( !MathSplitParenthesisedList( text, parts, 5 ) )
            return false;
        CTEulerAngles parsed;
        if ( !OrderFromName( parts[ 0 ], parsed.order ) || !FrameFromName( parts[ 1 ], parsed.frame ) )
            return false;
        if ( !MathAngleFromString( parts[ 2 ], parsed.first ) || !MathAngleFromString( parts[ 3 ], parsed.second ) || !MathAngleFromString( parts[ 4 ], parsed.third ) )
            return false;
        *this = parsed;
        return true;
    }

    /** Writes the attributes order, frame, first, second and third (radians) */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        return node.SetAttribute( "order", CORE::CVariant( GetOrderName( order ) ) ) &&
               node.SetAttribute( "frame", CORE::CVariant( EULERFRAME_EXTRINSIC == frame ? "extrinsic" : "intrinsic" ) ) &&
               MathAngleSerialize( node, "first", first ) &&
               MathAngleSerialize( node, "second", second ) &&
               MathAngleSerialize( node, "third", third );
    }

    /** Reads the attributes of Serialize; a missing or invalid attribute fails and leaves this value unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        const CORE::CDataNode::TKeyValuePair* orderAttribute = node.GetAttribute( "order" );
        const CORE::CDataNode::TKeyValuePair* frameAttribute = node.GetAttribute( "frame" );
        if ( GUCEF_NULL == orderAttribute || GUCEF_NULL == frameAttribute )
            return false;
        CTEulerAngles parsed;
        if ( !OrderFromName( orderAttribute->second.AsString(), parsed.order ) || !FrameFromName( frameAttribute->second.AsString(), parsed.frame ) )
            return false;
        if ( !MathAngleDeserialize( node, "first", "firstDegrees", parsed.first ) ||
             !MathAngleDeserialize( node, "second", "secondDegrees", parsed.second ) ||
             !MathAngleDeserialize( node, "third", "thirdDegrees", parsed.third ) )
            return false;
        *this = parsed;
        return true;
    }

    private:

    static bool FrameFromName( const CORE::CString& name, TEulerFrame& angleFrame )
    {
        if ( name == "intrinsic" ) { angleFrame = EULERFRAME_INTRINSIC; return true; }
        if ( name == "extrinsic" ) { angleFrame = EULERFRAME_EXTRINSIC; return true; }
        return false;
    }

    /**
     *  Intrinsic angles ( a1, a2, a3 ) with R = R_i( a1 ) R_j( a2 ) R_k( a3 ) for the axes ( i, j, k ) of a Tait-Bryan
     *  order or ( i, j, i ) of a proper order (k == i). The sign s is +1 when ( i, j, remaining axis ) is a cyclic
     *  permutation of ( X, Y, Z ). At the singularity a3 is set to 0 (foldIntoFirst) or a1 is (otherwise).
     */
    static void ExtractIntrinsic( const CTMatrix3D< T >& m, UInt32 i, UInt32 j, UInt32 k, bool foldIntoFirst, T& a1, T& a2, T& a3 ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The elementary rotation about axis 0, 1 or 2 by the right-hand rule */
    static CTMatrix3D< T > Elementary( UInt32 axis, const T& angle ) GUCEF_MATH_REALTIME_CONTEXT
    {
        switch ( axis )
        {
            case 0: return CTMatrix3D< T >::RotationX( CTRadian< T >( angle ) );
            case 1: return CTMatrix3D< T >::RotationY( CTRadian< T >( angle ) );
            default: return CTMatrix3D< T >::RotationZ( CTRadian< T >( angle ) );
        }
    }

    /** The angle of a rotation n about axis 0, 1 or 2 (n is that elementary rotation up to rounding) */
    static T ResidualAngle( const CTMatrix3D< T >& n, UInt32 axis ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const UInt32 j = ( axis + 1 ) % 3;
        const UInt32 k = ( axis + 2 ) % 3;
        return TTraits::Atan2( n( k, j ), n( j, j ) );
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

/* The related classes, included after the class (any header of the family can be included first) */

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

#ifndef GUCEF_MATH_CTMATRIX3D_H
#include "gucefMATH_CTMatrix3D.h"
#define GUCEF_MATH_CTMATRIX3D_H
#endif /* GUCEF_MATH_CTMATRIX3D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members, out of the class: with GUCEF_MATH_PRECISE_INLINE (empty by default) they are not
 *  inline, so for the predefined kinds the copy compiled in the module (without floating-point contraction) runs.
 */

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTEulerAngles< T >::ToQuaternion( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 axes[ 3 ];
    GetAxes( order, axes[ 0 ], axes[ 1 ], axes[ 2 ] );
    const CTRadian< T > angles[ 3 ] = { first, second, third };
    CTQuaternion< T > elementary[ 3 ];
    for ( UInt32 n=0; n<3; ++n )
    {
        const T half = angles[ n ].GetValue() * TTraits::Half();
        T components[ 4 ] = { TTraits::Zero(), TTraits::Zero(), TTraits::Zero(), TTraits::Cos( half ) };
        components[ axes[ n ] ] = TTraits::Sin( half );
        elementary[ n ] = CTQuaternion< T >::FromXYZW( components[ 0 ], components[ 1 ], components[ 2 ], components[ 3 ] );
    }
    if ( EULERFRAME_EXTRINSIC == frame )
        return elementary[ 2 ] * ( elementary[ 1 ] * elementary[ 0 ] );
    return elementary[ 0 ] * ( elementary[ 1 ] * elementary[ 2 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTEulerAngles< T >::ToRotationMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 axes[ 3 ];
    GetAxes( order, axes[ 0 ], axes[ 1 ], axes[ 2 ] );
    const CTRadian< T > angles[ 3 ] = { first, second, third };
    CTMatrix3D< T > elementary[ 3 ];
    for ( UInt32 n=0; n<3; ++n )
    {
        switch ( axes[ n ] )
        {
            case 0: elementary[ n ] = CTMatrix3D< T >::RotationX( angles[ n ] ); break;
            case 1: elementary[ n ] = CTMatrix3D< T >::RotationY( angles[ n ] ); break;
            default: elementary[ n ] = CTMatrix3D< T >::RotationZ( angles[ n ] ); break;
        }
    }
    if ( EULERFRAME_EXTRINSIC == frame )
        return elementary[ 2 ] * ( elementary[ 1 ] * elementary[ 0 ] );
    return elementary[ 0 ] * ( elementary[ 1 ] * elementary[ 2 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTEulerAngles< T >
CTEulerAngles< T >::FromQuaternion( const CTQuaternion< T >& q, TEulerOrder angleOrder, TEulerFrame angleFrame ) GUCEF_MATH_REALTIME_CONTEXT
{
    return FromRotationMatrix( q.ToRotationMatrix3D(), angleOrder, angleFrame );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTEulerAngles< T >
CTEulerAngles< T >::FromRotationMatrix( const CTMatrix3D< T >& m, TEulerOrder angleOrder, TEulerFrame angleFrame ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 a = 0;
    UInt32 b = 0;
    UInt32 c = 0;
    GetAxes( angleOrder, a, b, c );
    T a1 = TTraits::Zero();
    T a2 = TTraits::Zero();
    T a3 = TTraits::Zero();
    if ( EULERFRAME_EXTRINSIC == angleFrame )
    {
        // extrinsic ( A, B, C ) with ( first, second, third ) is intrinsic ( C, B, A ) with ( third, second, first )
        // extrinsic ( A, B, C ) with ( first, second, third ) is intrinsic ( C, B, A ) with ( third, second, first ); at
        // the singularity the extrinsic third angle (the intrinsic first) is the one set to 0
        ExtractIntrinsic( m, c, b, a, false, a3, a2, a1 );
    }
    else
    {
        ExtractIntrinsic( m, a, b, c, true, a1, a2, a3 );
    }
    return CTEulerAngles( CTRadian< T >( a1 ), CTRadian< T >( a2 ), CTRadian< T >( a3 ), angleOrder, angleFrame );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTEulerAngles< T >::ExtractIntrinsic( const CTMatrix3D< T >& m, UInt32 i, UInt32 j, UInt32 k, bool foldIntoFirst, T& a1, T& a2, T& a3 ) GUCEF_MATH_REALTIME_CONTEXT
{
    // the parity of the axis permutation ( i, j, remaining axis )
    const bool cyclic = ( ( i + 1 ) % 3 == j );
    const T one = TTraits::One();
    const T s = cyclic ? one : ComponentNegate( one );
    const T threshold = TTraits::Epsilon() * TTraits::FromInt32( 16 );
    const UInt32 o = 3 - i - j;
    bool singular = false;
    if ( k != i )
    {
        // Tait-Bryan: R( i, k ) = s sin( a2 )
        const T cosine = CTVector2D< T >( m( i, i ), m( i, j ) ).GetLength();
        a2 = TTraits::Atan2( s * m( i, k ), cosine );
        singular = !( threshold < cosine );
        if ( !singular )
        {
            if ( foldIntoFirst )
                a1 = TTraits::Atan2( ComponentNegate( s * m( j, k ) ), m( k, k ) );
            else
                a3 = TTraits::Atan2( ComponentNegate( s * m( i, j ) ), m( i, i ) );
        }
    }
    else
    {
        // proper Euler ( i, j, i ): R( i, i ) = cos( a2 )
        const T sine = CTVector2D< T >( m( i, j ), m( i, o ) ).GetLength();
        a2 = TTraits::Atan2( sine, m( i, i ) );
        singular = !( threshold < sine );
        if ( !singular )
        {
            if ( foldIntoFirst )
                a1 = TTraits::Atan2( m( j, i ), ComponentNegate( s * m( o, i ) ) );
            else
                a3 = TTraits::Atan2( m( i, j ), s * m( i, o ) );
        }
    }
    // The other outer angle comes from the rotation that remains once the known ones are removed, so the returned
    // angles always rebuild the rotation, also close to gimbal lock where the two outer angles are ill-conditioned
    // separately. At the singularity the angle to fold away is exactly 0.
    if ( foldIntoFirst )
    {
        if ( singular )
        {
            a3 = TTraits::Zero();
            a1 = ResidualAngle( m * Elementary( j, a2 ).Transposed(), i );
        }
        else
        {
            a3 = ResidualAngle( ( Elementary( i, a1 ) * Elementary( j, a2 ) ).Transposed() * m, k );
        }
    }
    else
    {
        if ( singular )
        {
            a1 = TTraits::Zero();
            a3 = ResidualAngle( Elementary( j, a2 ).Transposed() * m, k );
        }
        else
        {
            a1 = ResidualAngle( m * ( Elementary( j, a2 ) * Elementary( k, a3 ) ).Transposed(), i );
        }
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

/* The precision-sensitive members of the predefined kinds (section 5.10 of the design; see gucefMATH_CTVector3D.h) */

#define GUCEF_MATH_CTEULERANGLES_REAL_KIND( prefix, kind ) \
    prefix CTQuaternion< kind > CTEulerAngles< kind >::ToQuaternion( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTEulerAngles< kind >::ToRotationMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTEulerAngles< kind > CTEulerAngles< kind >::FromQuaternion( const CTQuaternion< kind >&, TEulerOrder, TEulerFrame ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTEulerAngles< kind > CTEulerAngles< kind >::FromRotationMatrix( const CTMatrix3D< kind >&, TEulerOrder, TEulerFrame ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTEULERANGLES_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTEULERANGLES_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTEULERANGLES_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTEULERANGLES_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTEULERANGLES_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTEULERANGLES_REAL_KIND( prefix, TFixed48_16 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTEULERANGLES_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTEULERANGLES_H ? */
