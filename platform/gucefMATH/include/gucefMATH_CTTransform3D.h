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

#ifndef GUCEF_MATH_CTTRANSFORM3D_H
#define GUCEF_MATH_CTTRANSFORM3D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

#ifndef GUCEF_MATH_CTMATRIX4D_H
#include "gucefMATH_CTMatrix4D.h"
#define GUCEF_MATH_CTMATRIX4D_H
#endif /* GUCEF_MATH_CTMATRIX4D_H ? */

#ifndef GUCEF_MATH_CTPOINT3D_H
#include "gucefMATH_CTPoint3D.h"
#define GUCEF_MATH_CTPOINT3D_H
#endif /* GUCEF_MATH_CTPOINT3D_H ? */

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
 *  Translation, rotation and scale (TRS) of an object in 3D (section 4.5.5 of the design): a point p of the object maps
 *  to position + rotation( scale * p ).
 *
 *  - Kinds: T (rotation and scale) and TPosition (position, default T) need a square root and transcendentals.
 *    Mixed precision, for example CTTransform3D< Float32, Float64 >: positions in Float64 for large worlds, rotation and
 *    scale in Float32; point transforms are computed in TPosition and ToMatrixRelativeTo( origin ) gives an exact
 *    origin-relative matrix in T for rendering and audio snapshots.
 *  - TRS cannot represent shear: a rotation followed by a non-uniform scale under a rotated parent produces shear.
 *    Combine and Inverse are exact when the scale magnitudes are uniform (HasUniformScale), mirrored axes included:
 *    a scale s M with M a diagonal of signs is moved past the rotation as M R M. Otherwise they return the TRS
 *    approximation (the "lossy scale" of other engines); exact composition uses matrices (ToMatrix), as CGeoSpace does.
 *  - Layout: the public members position, rotation, scale; no virtual functions.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "T(x, y, z) R(x, y, z, w) S(x, y, z)"; DataNode child nodes position, rotation, scale.
 */
template < typename T, typename TPosition = T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTTransform3D
{
    public:

    typedef T TValue;
    typedef TPosition TPositionValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTNumericTraits< TPosition > TPositionTraits;
    typedef CTVector3D< T > TVector;
    typedef CTVector3D< TPosition > TPositionVector;
    typedef CTPoint3D< TPosition > TPoint;

    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt && TTraits::HasTranscendentals && TPositionTraits::HasSqrt, Transforms_require_numeric_kinds_with_sqrt_and_transcendentals );

    TPositionVector position;
    CTQuaternion< T > rotation;
    TVector scale;

    /** The identity: zero position, identity rotation, unit scale */
    CTTransform3D( void )
        : position(), rotation(), scale( TVector::One() )
    {
    }

    CTTransform3D( const TPositionVector& p, const CTQuaternion< T >& r, const TVector& s )
        : position( p ), rotation( r ), scale( s )
    {
    }

    static CTTransform3D Identity( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTTransform3D(); }

    /** The TRS of an affine matrix (CTMatrix4D::Decompose); false for shear, perspective or zero scale, 'out' unchanged */
    static bool FromMatrix( const CTMatrix4D< T >& m, CTTransform3D& out ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Translation * Rotation * Scale with the position converted to T */
    CTMatrix4D< T > ToMatrix( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The matrix of this transform relative to 'origin': the translation position - origin is formed in TPosition and
     *  only then converted to T, so a camera-relative Float32 matrix of a Float64 world position is exact to Float32
     *  precision
     */
    CTMatrix4D< T > ToMatrixRelativeTo( const TPositionVector& origin ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** position + rotation( scale * p ), computed in TPosition */
    TPoint TransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** rotation( scale * v ): the translation does not apply */
    TVector TransformVector( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The direction of rotation( scale * d ); rotation( d ) when the scale collapses d */
    CTDirectionVector3D< T > TransformDirection( const CTDirectionVector3D< T >& d ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The inverse map of a point: scale^-1 * rotation^-1( p - position ); a zero scale component gives 0 for that component */
    TPoint InverseTransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The inverse transform: exact when HasUniformScale (mirrors included), the TRS approximation otherwise; zero scale components stay 0 */
    CTTransform3D Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** child expressed in the space of 'parent's parent: exact when parent.HasUniformScale (mirrors included), the TRS approximation otherwise */
    static CTTransform3D Combine( const CTTransform3D& parent, const CTTransform3D& child ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Lerp of position and scale, Slerp of rotation; t = 0 gives a, t = 1 gives b */
    static CTTransform3D Interpolate( const CTTransform3D& a, const CTTransform3D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  True when the scale magnitudes |s.x|, |s.y|, |s.z| differ by at most tolerance * the largest: then Combine (as
     *  parent) and Inverse are exact. Signs may differ: ( -2, 2, 2 ) is a uniform scale with a mirror.
     */
    bool HasUniformScale( const T& tolerance ) const GUCEF_MATH_REALTIME_CONTEXT;

    friend bool operator==( const CTTransform3D& a, const CTTransform3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.position == b.position && a.rotation == b.rotation && a.scale == b.scale;
    }

    friend bool operator!=( const CTTransform3D& a, const CTTransform3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "T(x, y, z) R(x, y, z, w) S(x, y, z)" */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( "T" );
        text += position.ToString();
        text += " R";
        text += rotation.ToString();
        text += " S";
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
        CTTransform3D parsed;
        if ( !parsed.position.FromString( groups[ 0 ] ) || !parsed.rotation.FromString( groups[ 1 ] ) || !parsed.scale.FromString( groups[ 2 ] ) )
            return false;
        *this = parsed;
        return true;
    }

    /** Writes the child nodes position, rotation and scale */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        CORE::CDataNode* positionNode = node.FindOrAddChild( "position" );
        CORE::CDataNode* rotationNode = node.FindOrAddChild( "rotation" );
        CORE::CDataNode* scaleNode = node.FindOrAddChild( "scale" );
        return GUCEF_NULL != positionNode && GUCEF_NULL != rotationNode && GUCEF_NULL != scaleNode &&
               position.Serialize( *positionNode, settings ) && rotation.Serialize( *rotationNode, settings ) && scale.Serialize( *scaleNode, settings );
    }

    /** Reads the child nodes position, rotation and scale; a missing or invalid one leaves this transform unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        const CORE::CDataNode* positionNode = node.FindChild( "position" );
        const CORE::CDataNode* rotationNode = node.FindChild( "rotation" );
        const CORE::CDataNode* scaleNode = node.FindChild( "scale" );
        if ( GUCEF_NULL == positionNode || GUCEF_NULL == rotationNode || GUCEF_NULL == scaleNode )
            return false;
        CTTransform3D parsed;
        if ( !parsed.position.Deserialize( *positionNode, settings ) || !parsed.rotation.Deserialize( *rotationNode, settings ) ||
             !parsed.scale.Deserialize( *scaleNode, settings ) )
            return false;
        *this = parsed;
        return true;
    }

    private:

    /**
     *  M R M for the rotation q and M = diag( signs of s ) (a zero component counts as positive): a rotation again,
     *  with the quaternion ( det( M ) M q.xyz, q.w )
     */
    static CTQuaternion< T > ReflectRotation( const CTQuaternion< T >& q, const TVector& s ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const bool nx = TTraits::IsNegative( s.x );
        const bool ny = TTraits::IsNegative( s.y );
        const bool nz = TTraits::IsNegative( s.z );
        const bool mirror = ( nx != ny ) != nz;
        // component i keeps its sign when its own sign equals det( M )
        return CTQuaternion< T >::FromXYZW( ( nx != mirror ) ? ComponentNegate( q.x ) : q.x, ( ny != mirror ) ? ComponentNegate( q.y ) : q.y,
                                            ( nz != mirror ) ? ComponentNegate( q.z ) : q.z, q.w );
    }

    /** 1 / s per component, 0 for a zero component */
    template < typename U >
    static CTVector3D< U > ReciprocalScale( const CTVector3D< U >& s ) GUCEF_MATH_REALTIME_CONTEXT
    {
        typedef CTNumericTraits< U > TUTraits;
        const U zero = TUTraits::Zero();
        const U one = TUTraits::One();
        return CTVector3D< U >( zero == s.x ? zero : one / s.x, zero == s.y ? zero : one / s.y, zero == s.z ? zero : one / s.z );
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

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE bool
CTTransform3D< T, TPosition >::FromMatrix( const CTMatrix4D< T >& m, CTTransform3D& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    TVector t;
    CTQuaternion< T > r;
    TVector s;
    if ( !m.Decompose( t, r, s ) )
        return false;
    out = CTTransform3D( TPositionVector( t ), r, s );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTTransform3D< T, TPosition >::ToMatrix( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix4D< T >::FromTRS( TVector( position ), rotation, scale );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTTransform3D< T, TPosition >::ToMatrixRelativeTo( const TPositionVector& origin ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix4D< T >::FromTRS( TVector( position - origin ), rotation, scale );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE typename CTTransform3D< T, TPosition >::TPoint
CTTransform3D< T, TPosition >::TransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const CTQuaternion< TPosition > r( rotation );
    const TPositionVector s( scale );
    return TPoint::Origin() + ( position + r.Rotate( s.ComponentMultiply( p.ToPositionVector() ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE typename CTTransform3D< T, TPosition >::TVector
CTTransform3D< T, TPosition >::TransformVector( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return rotation.Rotate( scale.ComponentMultiply( v ) );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTTransform3D< T, TPosition >::TransformDirection( const CTDirectionVector3D< T >& d ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTDirectionVector3D< T > result( d );
    if ( !CTDirectionVector3D< T >::TryCreate( TransformVector( d.AsVector() ), result ) )
        return rotation.Rotate( d );
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE typename CTTransform3D< T, TPosition >::TPoint
CTTransform3D< T, TPosition >::InverseTransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const CTQuaternion< TPosition > inverseRotation( rotation.Conjugate() );
    const TPositionVector inverseScale = ReciprocalScale( TPositionVector( scale ) );
    return TPoint::Origin() + inverseScale.ComponentMultiply( inverseRotation.Rotate( p.ToPositionVector() - position ) );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE CTTransform3D< T, TPosition >
CTTransform3D< T, TPosition >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    // ( R S )^-1 = S^-1 R^-1 = ( M R^-1 M ) S^-1 for S = s M with M the signs of the scale
    const CTQuaternion< T > inverseRotation = ReflectRotation( rotation.Conjugate(), scale );
    const TVector inverseScale = ReciprocalScale( scale );
    const CTQuaternion< TPosition > r( rotation.Conjugate() );
    const TPositionVector s( inverseScale );
    return CTTransform3D( -s.ComponentMultiply( r.Rotate( position ) ), inverseRotation, inverseScale );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE CTTransform3D< T, TPosition >
CTTransform3D< T, TPosition >::Combine( const CTTransform3D& parent, const CTTransform3D& child ) GUCEF_MATH_REALTIME_CONTEXT
{
    // Rp Sp Rc Sc = Rp ( M Rc M ) Sp Sc for Sp = s M with M the signs of the parent scale
    const TPoint childOrigin = TPoint::Origin() + child.position;
    return CTTransform3D( parent.TransformPoint( childOrigin ).ToPositionVector(), parent.rotation * ReflectRotation( child.rotation, parent.scale ),
                          parent.scale.ComponentMultiply( child.scale ) );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE CTTransform3D< T, TPosition >
CTTransform3D< T, TPosition >::Interpolate( const CTTransform3D& a, const CTTransform3D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( TTraits::Zero() == t )
        return a;
    if ( TTraits::One() == t )
        return b;
    TPosition positionT = TPositionTraits::Zero();
    CTNumericConverter< T, TPosition >::Convert( t, positionT, CORE::ROUNDINGMODE_HALF_EVEN );
    return CTTransform3D( TPositionVector::Lerp( a.position, b.position, positionT ), CTQuaternion< T >::Slerp( a.rotation, b.rotation, t ),
                          TVector::Lerp( a.scale, b.scale, t ) );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
GUCEF_MATH_PRECISE_INLINE bool
CTTransform3D< T, TPosition >::HasUniformScale( const T& tolerance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector magnitudes = scale.Abs();
    const T largest = magnitudes.GetMaxComponent();
    const T allowed = tolerance * largest;
    return !( allowed < largest - magnitudes.x ) && !( allowed < largest - magnitudes.y ) && !( allowed < largest - magnitudes.z );
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
 *  The precision-sensitive members of the predefined kinds (section 5.10 of the design; see gucefMATH_CTVector3D.h).
 *  The list macro takes the kind of rotation and scale and the kind of the position (names without commas).
 */

#define GUCEF_MATH_CTTRANSFORM3D_REAL_KIND( prefix, kind, positionKind ) \
    prefix bool CTTransform3D< kind, positionKind >::FromMatrix( const CTMatrix4D< kind >&, CTTransform3D< kind, positionKind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTTransform3D< kind, positionKind >::ToMatrix( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTTransform3D< kind, positionKind >::ToMatrixRelativeTo( const CTVector3D< positionKind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint3D< positionKind > CTTransform3D< kind, positionKind >::TransformPoint( const CTPoint3D< positionKind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTTransform3D< kind, positionKind >::TransformVector( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTTransform3D< kind, positionKind >::TransformDirection( const CTDirectionVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint3D< positionKind > CTTransform3D< kind, positionKind >::InverseTransformPoint( const CTPoint3D< positionKind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTTransform3D< kind, positionKind > CTTransform3D< kind, positionKind >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTTransform3D< kind, positionKind > CTTransform3D< kind, positionKind >::Combine( const CTTransform3D< kind, positionKind >&, const CTTransform3D< kind, positionKind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTTransform3D< kind, positionKind > CTTransform3D< kind, positionKind >::Interpolate( const CTTransform3D< kind, positionKind >&, const CTTransform3D< kind, positionKind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTTransform3D< kind, positionKind >::HasUniformScale( const kind& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTTRANSFORM3D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTTRANSFORM3D_REAL_KIND( prefix, Float32, Float32 ) \
    GUCEF_MATH_CTTRANSFORM3D_REAL_KIND( prefix, Float64, Float64 ) \
    GUCEF_MATH_CTTRANSFORM3D_REAL_KIND( prefix, Float32, Float64 ) \
    GUCEF_MATH_CTTRANSFORM3D_REAL_KIND( prefix, TFixed16_16, TFixed16_16 ) \
    GUCEF_MATH_CTTRANSFORM3D_REAL_KIND( prefix, TFixed32_32, TFixed32_32 ) \
    GUCEF_MATH_CTTRANSFORM3D_REAL_KIND( prefix, TFixed48_16, TFixed48_16 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTTRANSFORM3D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTTRANSFORM3D_H ? */
