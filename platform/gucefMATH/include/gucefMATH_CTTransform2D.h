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

#ifndef GUCEF_MATH_CTTRANSFORM2D_H
#define GUCEF_MATH_CTTRANSFORM2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX3D_H
#include "gucefMATH_CTMatrix3D.h"
#define GUCEF_MATH_CTMATRIX3D_H
#endif /* GUCEF_MATH_CTMATRIX3D_H ? */

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#include "gucefMATH_AngleFunctions.h"
#define GUCEF_MATH_ANGLEFUNCTIONS_H
#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */

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
 *  Translation, rotation and scale of an object in the plane (section 4.5.5 of the design): a point p maps to
 *  position + R( rotation ) ( scale * p ), with the rotation counter-clockwise positive (about +Z).
 *
 *  - Kind: T needs a square root and transcendentals (floats and fixed point).
 *  - Combine and Inverse are exact when the scale magnitudes are uniform (HasUniformScale), mirrors included (a mirror
 *    reverses the sense of the rotation it is moved past), and return the TRS approximation otherwise (see
 *    CTTransform3D); ToMatrix3D gives the exact homogeneous matrix.
 *  - Layout: the public members position, rotation, scale; no virtual functions.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "T(x, y) R(<angle> rad) S(x, y)" ("deg" accepted on input); DataNode child nodes position and scale (x, y)
 *    and rotation (attribute radians, degrees accepted on input).
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTTransform2D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTVector2D< T > TVector;
    typedef CTPoint2D< T > TPoint;

    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt && TTraits::HasTranscendentals, Transforms_require_numeric_kinds_with_sqrt_and_transcendentals );

    TVector position;
    CTRadian< T > rotation;
    TVector scale;

    /** The identity: zero position, zero rotation, unit scale */
    CTTransform2D( void )
        : position(), rotation(), scale( TVector::One() )
    {
    }

    CTTransform2D( const TVector& p, const CTRadian< T >& r, const TVector& s )
        : position( p ), rotation( r ), scale( s )
    {
    }

    static CTTransform2D Identity( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTTransform2D(); }

    /** The homogeneous matrix Translation2D * Rotation2D * Scale2D */
    CTMatrix3D< T > ToMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** position + R( rotation ) ( scale * p ) */
    TPoint TransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** R( rotation ) ( scale * v ) */
    TVector TransformVector( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** scale^-1 * R( -rotation ) ( p - position ); a zero scale component gives 0 for that component */
    TPoint InverseTransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The inverse transform: exact when HasUniformScale (mirrors included), the TRS approximation otherwise; zero scale components stay 0 */
    CTTransform2D Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** child in the space of 'parent's parent; the rotation is wrapped to (-pi, pi] */
    static CTTransform2D Combine( const CTTransform2D& parent, const CTTransform2D& child ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Lerp of position and scale, LerpAngle (shortest way) of the rotation; t = 0 gives a, t = 1 gives b */
    static CTTransform2D Interpolate( const CTTransform2D& a, const CTTransform2D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /** True when |s.x| and |s.y| differ by at most tolerance * the larger: then Combine (as parent) and Inverse are exact; signs may differ */
    bool HasUniformScale( const T& tolerance ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        const TVector magnitudes = scale.Abs();
        const T largest = magnitudes.GetMaxComponent();
        return !( tolerance * largest < largest - magnitudes.GetMinComponent() );
    }

    friend bool operator==( const CTTransform2D& a, const CTTransform2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.position == b.position && a.rotation == b.rotation && a.scale == b.scale;
    }

    friend bool operator!=( const CTTransform2D& a, const CTTransform2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "T(x, y) R(<angle> rad) S(x, y)" */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( "T" );
        text += position.ToString();
        text += " R(";
        text += MathAngleToString( rotation );
        text += ") S";
        text += scale.ToString();
        return text;
    }

    /** Parses the ToString form (white space allowed); false leaves this transform unchanged */
    bool FromString( const CORE::CString& text )
    {
        UInt32 cursor = 0;
        CORE::CString groups[ 3 ];
        if ( !MathTakeLabeledGroup( text, cursor, "T", groups[ 0 ] ) || !MathTakeLabeledGroup( text, cursor, "R", groups[ 1 ] ) ||
             !MathTakeLabeledGroup( text, cursor, "S", groups[ 2 ] ) || !MathIsTextEnd( text, cursor ) )
            return false;
        CORE::CString angleText[ 1 ];
        CTTransform2D parsed;
        if ( !parsed.position.FromString( groups[ 0 ] ) || !MathSplitParenthesisedList( groups[ 1 ], angleText, 1 ) ||
             !MathAngleFromString( angleText[ 0 ], parsed.rotation ) || !parsed.scale.FromString( groups[ 2 ] ) )
            return false;
        *this = parsed;
        return true;
    }

    /** Writes the child nodes position, rotation (attribute radians) and scale */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        CORE::CDataNode* positionNode = node.FindOrAddChild( "position" );
        CORE::CDataNode* rotationNode = node.FindOrAddChild( "rotation" );
        CORE::CDataNode* scaleNode = node.FindOrAddChild( "scale" );
        return GUCEF_NULL != positionNode && GUCEF_NULL != rotationNode && GUCEF_NULL != scaleNode &&
               position.Serialize( *positionNode, settings ) && MathAngleSerialize( *rotationNode, "radians", rotation ) &&
               scale.Serialize( *scaleNode, settings );
    }

    /** Reads the child nodes of Serialize; a missing or invalid one leaves this transform unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        const CORE::CDataNode* positionNode = node.FindChild( "position" );
        const CORE::CDataNode* rotationNode = node.FindChild( "rotation" );
        const CORE::CDataNode* scaleNode = node.FindChild( "scale" );
        if ( GUCEF_NULL == positionNode || GUCEF_NULL == rotationNode || GUCEF_NULL == scaleNode )
            return false;
        CTTransform2D parsed;
        if ( !parsed.position.Deserialize( *positionNode, settings ) || !MathAngleDeserialize( *rotationNode, "radians", "degrees", parsed.rotation ) ||
             !parsed.scale.Deserialize( *scaleNode, settings ) )
            return false;
        *this = parsed;
        return true;
    }

    private:

    /** True when exactly one scale component is negative (a reflection) */
    static GUCEF_MATH_FORCE_INLINE bool IsMirror( const TVector& s ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsNegative( s.x ) != TTraits::IsNegative( s.y );
    }

    /** 1 / s per component, 0 for a zero component */
    static TVector ReciprocalScale( const TVector& s ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const T zero = TTraits::Zero();
        const T one = TTraits::One();
        return TVector( zero == s.x ? zero : one / s.x, zero == s.y ? zero : one / s.y );
    }
};

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
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTTransform2D< T >::ToMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T c = TTraits::Cos( rotation.GetValue() );
    const T s = TTraits::Sin( rotation.GetValue() );
    const T values[ 9 ] = { c * scale.x, s * scale.x, TTraits::Zero(),
                            ComponentNegate( s ) * scale.y, c * scale.y, TTraits::Zero(),
                            position.x, position.y, TTraits::One() };
    return CTMatrix3D< T >::FromColumnMajor( values );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTTransform2D< T >::TPoint
CTTransform2D< T >::TransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return TPoint::Origin() + ( position + TransformVector( p.ToPositionVector() ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTTransform2D< T >::TVector
CTTransform2D< T >::TransformVector( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix2D< T >::Rotation( rotation ).Transform( scale.ComponentMultiply( v ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTTransform2D< T >::TPoint
CTTransform2D< T >::InverseTransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector local = CTMatrix2D< T >::Rotation( -rotation ).Transform( p - ( TPoint::Origin() + position ) );
    return TPoint::Origin() + ReciprocalScale( scale ).ComponentMultiply( local );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTTransform2D< T >
CTTransform2D< T >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    // ( R S )^-1 = S^-1 R( -a ) = R( +-a ) S^-1: with a mirror ( one negative scale component ) M R( -a ) M = R( a )
    const TVector inverseScale = ReciprocalScale( scale );
    const TVector p = -inverseScale.ComponentMultiply( CTMatrix2D< T >::Rotation( -rotation ).Transform( position ) );
    return CTTransform2D( p, IsMirror( scale ) ? rotation : -rotation, inverseScale );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTTransform2D< T >
CTTransform2D< T >::Combine( const CTTransform2D& parent, const CTTransform2D& child ) GUCEF_MATH_REALTIME_CONTEXT
{
    const TPoint origin = parent.TransformPoint( TPoint::Origin() + child.position );
    // R( a ) S R( b ) = R( a ) R( +-b ) S for S = s M: a mirror M reverses the child's rotation
    const CTRadian< T > childRotation = IsMirror( parent.scale ) ? -child.rotation : child.rotation;
    return CTTransform2D( origin.ToPositionVector(), WrapAngle( parent.rotation + childRotation ), parent.scale.ComponentMultiply( child.scale ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTTransform2D< T >
CTTransform2D< T >::Interpolate( const CTTransform2D& a, const CTTransform2D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( TTraits::Zero() == t )
        return a;
    if ( TTraits::One() == t )
        return b;
    return CTTransform2D( TVector::Lerp( a.position, b.position, t ), LerpAngle( a.rotation, b.rotation, t ), TVector::Lerp( a.scale, b.scale, t ) );
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

#define GUCEF_MATH_CTTRANSFORM2D_REAL_KIND( prefix, kind ) \
    prefix CTMatrix3D< kind > CTTransform2D< kind >::ToMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint2D< kind > CTTransform2D< kind >::TransformPoint( const CTPoint2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind > CTTransform2D< kind >::TransformVector( const CTVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint2D< kind > CTTransform2D< kind >::InverseTransformPoint( const CTPoint2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTTransform2D< kind > CTTransform2D< kind >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTTransform2D< kind > CTTransform2D< kind >::Combine( const CTTransform2D< kind >&, const CTTransform2D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTTransform2D< kind > CTTransform2D< kind >::Interpolate( const CTTransform2D< kind >&, const CTTransform2D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTTRANSFORM2D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTTRANSFORM2D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTTRANSFORM2D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTTRANSFORM2D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTTRANSFORM2D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTTRANSFORM2D_REAL_KIND( prefix, TFixed48_16 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTTRANSFORM2D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTTRANSFORM2D_H ? */
