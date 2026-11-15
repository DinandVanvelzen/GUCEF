/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#define GUCEF_GEOMETRY_CTORIENTEDBOX3D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#include "gucefGEOMETRY_SShapeTextReader.h"
#define GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#endif /* GUCEF_GEOMETRY_SSHAPETEXTREADER_H ? */

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#include "gucefGEOMETRY_SRayCastResult.h"
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H
#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {
template < typename T > class CTMatrix4D;
}
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T > class CTRay3D;
template < typename T > class CTSegment3D;
template < typename T > class CTAxisAlignedBox3D;

/*-------------------------------------------------------------------------*/

/**
 *  A solid box with any orientation: centre, rotation and half extents along the rotated axes (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: centre point, unit quaternion and half extent vector, 10 * sizeof( T ), no virtual functions. The default
 *    box is the unit cube (half extents 0.5) at the origin without rotation. The rotation must be a unit quaternion
 *    (Normalized() it after accumulating rotations); the box axes are the images of +X, +Y and +Z.
 *  - A negative half extent is not valid: the box contains nothing. A zero half extent is a flat box and valid.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "OrientedBox(centre (x, y, z), rotation (x, y, z, w), halfExtents (x, y, z))"; DataNode: child nodes
 *    centre, rotation and halfExtents.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTOrientedBox3D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;
    typedef MATH::CTQuaternion< T > TRotation;

    TPoint centre;
    TRotation rotation;
    TVector halfExtents;

    GUCEF_GEOMETRY_FORCE_INLINE CTOrientedBox3D( void ) : centre(), rotation(), halfExtents( TTraits::Half(), TTraits::Half(), TTraits::Half() ) {}

    GUCEF_GEOMETRY_FORCE_INLINE CTOrientedBox3D( const TPoint& centrePoint, const TRotation& unitRotation, const TVector& halfSize )
        : centre( centrePoint ), rotation( unitRotation ), halfExtents( halfSize )
    {
    }

    /** The box of an axis aligned box, without rotation (an empty box gives a box with negative half extents) */
    static CTOrientedBox3D FromAxisAlignedBox( const CTAxisAlignedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The three unit axes of the box: the images of +X, +Y and +Z under the rotation */
    void GetAxes( TVector ( &axes )[ 3 ] ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The corner for the bits of 'index': bit 0 selects +X half extent, bit 1 +Y, bit 2 +Z (higher bits are ignored) */
    TPoint GetCorner( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** All eight corners, in the order of GetCorner */
    void GetCorners( TPoint ( &corners )[ 8 ] ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The coordinates of a point in the frame of the box: along the box axes, relative to the centre */
    TVector ToLocal( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The point with the given coordinates in the frame of the box */
    TPoint FromLocal( const TVector& local ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when no half extent is negative (a NaN counts as negative) */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsValid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return halfExtents.x >= TTraits::Zero() && halfExtents.y >= TTraits::Zero() && halfExtents.z >= TTraits::Zero();
    }

    /** True when the point is inside or on the surface; false for an invalid box */
    bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the solid box: the point itself when inside; the centre for an invalid box */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Distance from the solid to the point, 0 inside */
    T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The corner furthest along 'direction' (the positive side for a zero component) */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The volume: 8 times the product of the half extents; 0 for an invalid box */
    T GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The area of the six faces; 0 for an invalid box */
    T GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The axis aligned box around the oriented box (empty for an invalid box) */
    CTAxisAlignedBox3D< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Hit of the ray with the solid box: the ray is moved into the frame of the box and cast on the axis aligned
     *  box (slab test), so axis parallel rays have no division by zero. A ray starting inside hits at distance 0 with
     *  RAYHIT_STARTED_INSIDE and the normal opposite to the ray; the feature index is the box axis of the entry face.
     *  An invalid box is missed.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends the twelve edges */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /**
     *  The exact image of the box under an affine matrix that maps the box axes to mutually orthogonal directions
     *  (every similarity, and a scale along the box axes of a rotated box); false (out unchanged) for any other
     *  matrix, whose image is not a box, and for a matrix that flattens an axis. A reflection is expressed through
     *  the symmetry of the box.
     */
    bool TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTOrientedBox3D& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The exact image when there is one (TryTransformed), else an axis aligned box around the eight transformed corners */
    CTOrientedBox3D Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTOrientedBox3D& a, const CTOrientedBox3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return a.centre == b.centre && a.rotation == b.rotation && a.halfExtents == b.halfExtents;
    }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTOrientedBox3D& a, const CTOrientedBox3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** Centre, rotation and half extents equal within the default tolerances of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTOrientedBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return centre.Equals( other.centre ) && rotation.Equals( other.rotation ) && halfExtents.Equals( other.halfExtents );
    }

    /** "OrientedBox(centre (x, y, z), rotation (x, y, z, w), halfExtents (x, y, z))" */
    CORE::CString ToString( void ) const
    {
        return "OrientedBox(centre " + centre.ToString() + ", rotation " + rotation.ToString() + ", halfExtents " + halfExtents.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this box unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedCentre;
        TRotation parsedRotation;
        TVector parsedExtents;
        if ( !reader.TryLiteral( "OrientedBox" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "centre" ) || !reader.TryValue( parsedCentre ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "rotation" ) || !reader.TryValue( parsedRotation ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "halfExtents" ) || !reader.TryValue( parsedExtents ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        centre = parsedCentre;
        rotation = parsedRotation;
        halfExtents = parsedExtents;
        return true;
    }

    /** Writes the child nodes centre, rotation and halfExtents into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "centre", centre, settings ) && SerializeShapeChild( node, "rotation", rotation, settings ) &&
               SerializeShapeChild( node, "halfExtents", halfExtents, settings );
    }

    /** Reads the child nodes centre, rotation and halfExtents; a missing or invalid one fails and leaves this box unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedCentre;
        TRotation parsedRotation;
        TVector parsedExtents;
        if ( !DeserializeShapeChild( node, "centre", parsedCentre, settings ) || !DeserializeShapeChild( node, "rotation", parsedRotation, settings ) ||
             !DeserializeShapeChild( node, "halfExtents", parsedExtents, settings ) )
            return false;
        centre = parsedCentre;
        rotation = parsedRotation;
        halfExtents = parsedExtents;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTOrientedBox3D< T >
CTOrientedBox3D< T >::FromAxisAlignedBox( const CTAxisAlignedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( box.IsEmpty() )
        return CTOrientedBox3D( TPoint(), TRotation(), TVector( MATH::ComponentNegate( TTraits::One() ), MATH::ComponentNegate( TTraits::One() ), MATH::ComponentNegate( TTraits::One() ) ) );
    return CTOrientedBox3D( box.GetCentre(), TRotation(), box.GetHalfExtents() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTOrientedBox3D< T >::GetAxes( TVector ( &axes )[ 3 ] ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const MATH::CTMatrix3D< T > m = rotation.ToRotationMatrix3D();
    axes[ 0 ] = m.GetColumn( 0 );
    axes[ 1 ] = m.GetColumn( 1 );
    axes[ 2 ] = m.GetColumn( 2 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTOrientedBox3D< T >::TPoint
CTOrientedBox3D< T >::GetCorner( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector axes[ 3 ];
    GetAxes( axes );
    const T sx = 0 != ( index & 1 ) ? halfExtents.x : MATH::ComponentNegate( halfExtents.x );
    const T sy = 0 != ( index & 2 ) ? halfExtents.y : MATH::ComponentNegate( halfExtents.y );
    const T sz = 0 != ( index & 4 ) ? halfExtents.z : MATH::ComponentNegate( halfExtents.z );
    return centre + ( axes[ 0 ] * sx + axes[ 1 ] * sy ) + axes[ 2 ] * sz;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTOrientedBox3D< T >::GetCorners( TPoint ( &corners )[ 8 ] ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector axes[ 3 ];
    GetAxes( axes );
    const TVector ex = axes[ 0 ] * halfExtents.x;
    const TVector ey = axes[ 1 ] * halfExtents.y;
    const TVector ez = axes[ 2 ] * halfExtents.z;
    for ( UInt32 i=0; i<8; ++i )
    {
        TVector offset = 0 != ( i & 1 ) ? ex : -ex;
        offset = offset + ( 0 != ( i & 2 ) ? ey : -ey );
        offset = offset + ( 0 != ( i & 4 ) ? ez : -ez );
        corners[ i ] = centre + offset;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTOrientedBox3D< T >::TVector
CTOrientedBox3D< T >::ToLocal( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector axes[ 3 ];
    GetAxes( axes );
    const TVector d = point - centre;
    return TVector( d.Dot( axes[ 0 ] ), d.Dot( axes[ 1 ] ), d.Dot( axes[ 2 ] ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTOrientedBox3D< T >::TPoint
CTOrientedBox3D< T >::FromLocal( const TVector& local ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector axes[ 3 ];
    GetAxes( axes );
    return centre + ( axes[ 0 ] * local.x + axes[ 1 ] * local.y ) + axes[ 2 ] * local.z;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTOrientedBox3D< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return false;
    const TVector local = ToLocal( point );
    return TTraits::Abs( local.x ) <= halfExtents.x && TTraits::Abs( local.y ) <= halfExtents.y && TTraits::Abs( local.z ) <= halfExtents.z;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTOrientedBox3D< T >::TPoint
CTOrientedBox3D< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return centre;
    TVector axes[ 3 ];
    GetAxes( axes );
    const TVector d = point - centre;
    const T lx = d.Dot( axes[ 0 ] );
    const T ly = d.Dot( axes[ 1 ] );
    const T lz = d.Dot( axes[ 2 ] );
    const T cx = TTraits::Max( TTraits::Min( lx, halfExtents.x ), MATH::ComponentNegate( halfExtents.x ) );
    const T cy = TTraits::Max( TTraits::Min( ly, halfExtents.y ), MATH::ComponentNegate( halfExtents.y ) );
    const T cz = TTraits::Max( TTraits::Min( lz, halfExtents.z ), MATH::ComponentNegate( halfExtents.z ) );
    if ( cx == lx && cy == ly && cz == lz )
        return point;
    return centre + ( axes[ 0 ] * cx + axes[ 1 ] * cy ) + axes[ 2 ] * cz;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTOrientedBox3D< T >::GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( point - ClosestPoint( point ) ).GetLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTOrientedBox3D< T >::GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( point - ClosestPoint( point ) ).GetSquaredLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTOrientedBox3D< T >::TPoint
CTOrientedBox3D< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector axes[ 3 ];
    GetAxes( axes );
    const T sx = direction.Dot( axes[ 0 ] ) < TTraits::Zero() ? MATH::ComponentNegate( halfExtents.x ) : halfExtents.x;
    const T sy = direction.Dot( axes[ 1 ] ) < TTraits::Zero() ? MATH::ComponentNegate( halfExtents.y ) : halfExtents.y;
    const T sz = direction.Dot( axes[ 2 ] ) < TTraits::Zero() ? MATH::ComponentNegate( halfExtents.z ) : halfExtents.z;
    return centre + ( axes[ 0 ] * sx + axes[ 1 ] * sy ) + axes[ 2 ] * sz;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTOrientedBox3D< T >::GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return TTraits::Zero();
    return ( ( halfExtents.x * halfExtents.y ) * halfExtents.z ) * TTraits::FromInt32( 8 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTOrientedBox3D< T >::GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return TTraits::Zero();
    const TVector& e = halfExtents;
    return ( ( e.x * e.y + e.y * e.z ) + e.z * e.x ) * TTraits::FromInt32( 8 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTAxisAlignedBox3D< T >
CTOrientedBox3D< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return CTAxisAlignedBox3D< T >();
    TVector axes[ 3 ];
    GetAxes( axes );
    const TVector extent( ( TTraits::Abs( axes[ 0 ].x ) * halfExtents.x + TTraits::Abs( axes[ 1 ].x ) * halfExtents.y ) + TTraits::Abs( axes[ 2 ].x ) * halfExtents.z,
                          ( TTraits::Abs( axes[ 0 ].y ) * halfExtents.x + TTraits::Abs( axes[ 1 ].y ) * halfExtents.y ) + TTraits::Abs( axes[ 2 ].y ) * halfExtents.z,
                          ( TTraits::Abs( axes[ 0 ].z ) * halfExtents.x + TTraits::Abs( axes[ 1 ].z ) * halfExtents.y ) + TTraits::Abs( axes[ 2 ].z ) * halfExtents.z );
    return CTAxisAlignedBox3D< T >( centre - extent, centre + extent );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTOrientedBox3D< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() )
        return false;                       /* a ray that is not a ray (a NaN or infinity in it) hits nothing */
    if ( !centre.IsFinite() || !rotation.IsFinite() )
        return false;
    if ( !IsValid() )
        return false;
    TVector axes[ 3 ];
    GetAxes( axes );
    const TVector direction = ray.direction.AsVector();
    const TVector o = ray.origin - centre;
    const TPoint localOrigin( o.Dot( axes[ 0 ] ), o.Dot( axes[ 1 ] ), o.Dot( axes[ 2 ] ) );
    const TVector localDirection( direction.Dot( axes[ 0 ] ), direction.Dot( axes[ 1 ] ), direction.Dot( axes[ 2 ] ) );
    TDirection unitDirection;
    if ( !TDirection::TryCreate( localDirection, unitDirection ) )
        return false;
    const CTRay3D< T > localRay( localOrigin, unitDirection );
    SRayCastResult< T > local;
    if ( !CTAxisAlignedBox3D< T >( TPoint( MATH::ComponentNegate( halfExtents.x ), MATH::ComponentNegate( halfExtents.y ), MATH::ComponentNegate( halfExtents.z ) ),
                                   TPoint( halfExtents.x, halfExtents.y, halfExtents.z ) ).RayCast( localRay, maxDistance, local ) )
        return false;
    SRayCastResult< T > hit;
    hit.distance = local.distance;
    hit.point = ray.origin + direction * local.distance;
    hit.featureIndex = local.featureIndex;
    hit.flags = local.flags;
    if ( local.StartedInside() )
    {
        hit.point = ray.origin;
        hit.normal = -direction;
    }
    else
    {
        hit.normal = ( axes[ 0 ] * local.normal.x + axes[ 1 ] * local.normal.y ) + axes[ 2 ] * local.normal.z;
    }
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTOrientedBox3D< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    if ( !IsValid() )
        return;
    TPoint corners[ 8 ];
    GetCorners( corners );
    for ( UInt32 corner=0; corner<8; ++corner )
    {
        for ( UInt32 bit=1; bit<=4; bit<<=1 )
        {
            if ( 0 == ( corner & bit ) )
                lines.push_back( CTSegment3D< T >( corners[ corner ], corners[ corner | bit ] ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTOrientedBox3D< T >::TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTOrientedBox3D& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !matrix.IsAffine() )
        return false;
    TVector axes[ 3 ];
    GetAxes( axes );
    const MATH::CTMatrix3D< T > linear = matrix.GetLinear();
    TVector images[ 3 ];
    T lengths[ 3 ];
    for ( UInt32 i=0; i<3; ++i )
    {
        images[ i ] = linear.Transform( axes[ i ] );
        lengths[ i ] = images[ i ].GetLength();
        if ( !( TTraits::Zero() < lengths[ i ] ) || !TTraits::IsFinite( lengths[ i ] ) )
            return false;
    }
    const T tolerance = TTraits::DefaultRelativeTolerance();
    for ( UInt32 i=0; i<3; ++i )
    {
        const UInt32 j = ( i + 1 ) % 3;
        if ( tolerance * ( lengths[ i ] * lengths[ j ] ) < TTraits::Abs( images[ i ].Dot( images[ j ] ) ) )
            return false;
    }
    TVector unit[ 3 ];
    for ( UInt32 i=0; i<3; ++i )
        unit[ i ] = images[ i ] / lengths[ i ];
    /* a reflection is the same box with one axis turned around */
    if ( TTraits::IsNegative( unit[ 0 ].Cross( unit[ 1 ] ).Dot( unit[ 2 ] ) ) )
        unit[ 2 ] = -unit[ 2 ];
    const MATH::CTMatrix3D< T > frame = MATH::CTMatrix3D< T >::FromColumns( unit[ 0 ], unit[ 1 ], unit[ 2 ] );
    out = CTOrientedBox3D( matrix.TransformPoint( centre ), frame.ToQuaternion().Normalized(),
                           TVector( halfExtents.x * lengths[ 0 ], halfExtents.y * lengths[ 1 ], halfExtents.z * lengths[ 2 ] ) );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTOrientedBox3D< T >
CTOrientedBox3D< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CTOrientedBox3D result;
    if ( TryTransformed( matrix, result ) )
        return result;
    TPoint corners[ 8 ];
    GetCorners( corners );
    CTAxisAlignedBox3D< T > bounds;
    for ( UInt32 i=0; i<8; ++i )
    {
        const TPoint p = matrix.TransformPoint( corners[ i ] );
        bounds.min = bounds.min.Min( p );
        bounds.max = bounds.max.Max( p );
    }
    return FromAxisAlignedBox( bounds );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTOrientedBox3D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTOrientedBox3D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*
 *  The related classes used by the members; included after the class so that each header can be included first.
 */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSEGMENT3D_H
#include "gucefGEOMETRY_CTSegment3D.h"
#define GUCEF_GEOMETRY_CTSEGMENT3D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT3D_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTORIENTEDBOX3D_H ? */
