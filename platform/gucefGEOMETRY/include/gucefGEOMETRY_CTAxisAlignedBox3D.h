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

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H

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

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#include "gucefGEOMETRY_SRayCastResult.h"
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H
#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */

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

/*-------------------------------------------------------------------------*/

/**
 *  A closed axis aligned box in 3D, given by its minimum and maximum corner (section 7.2 of the design).
 *
 *  - Kind: floats (Float32, Float64 are exported); the members that measure need a square root.
 *  - Layout: min and max, 6 * sizeof( T ), no virtual functions.
 *  - Empty box: min greater than max on some axis. The default box is the empty box and the identity of Merged.
 *    Its distance queries give the largest value of the kind, its ClosestPoint is the query point.
 *  - Realtime: every member except the text, DataNode and wireframe members is realtime-safe.
 *  - Text: "AxisAlignedBox3D(min (x, y, z), max (x, y, z))"; DataNode: child nodes min and max.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTAxisAlignedBox3D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;

    TPoint min;
    TPoint max;

    /** The empty box */
    GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D( void )
        : min( TTraits::Max(), TTraits::Max(), TTraits::Max() )
        , max( TTraits::Lowest(), TTraits::Lowest(), TTraits::Lowest() )
    {
    }

    GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D( const TPoint& minCorner, const TPoint& maxCorner )
        : min( minCorner ), max( maxCorner )
    {
    }

    /** The box converted from another kind component-wise (see CTVector3D) */
    template < typename U >
    explicit CTAxisAlignedBox3D( const CTAxisAlignedBox3D< U >& other )
        : min( other.min ), max( other.max )
    {
    }

    /** The empty box: the identity of Merged */
    static GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D Empty( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return CTAxisAlignedBox3D(); }

    /** The smallest box around the points; the empty box for count 0 */
    static CTAxisAlignedBox3D FromPoints( const TPoint* points, UInt32 count ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( 0 == count || GUCEF_NULL != points );
        CTAxisAlignedBox3D box;
        if ( GUCEF_NULL == points )
            return box;
        for ( UInt32 i=0; i<count; ++i )
        {
            box.min = box.min.Min( points[ i ] );
            box.max = box.max.Max( points[ i ] );
        }
        return box;
    }

    /** The box around 'centre' that extends 'halfExtents' (non-negative) along each axis */
    static GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D FromCentreHalfExtents( const TPoint& centre, const TVector& halfExtents ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return CTAxisAlignedBox3D( centre - halfExtents, centre + halfExtents );
    }

    /*---------------------------------------------------------------------*/
    /*  Properties                                                          */
    /*---------------------------------------------------------------------*/

    /** True when min is greater than max on some axis (a NaN component counts as empty) */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !( min.x <= max.x && min.y <= max.y && min.z <= max.z );
    }

    /** The centre of the box; the origin for the empty box */
    TPoint GetCentre( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TPoint();
        return TPoint::Midpoint( min, max );
    }

    /** The size, max - min; the zero vector for the empty box */
    GUCEF_GEOMETRY_FORCE_INLINE TVector GetExtents( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return IsEmpty() ? TVector() : max - min;
    }

    /** Half the size; the zero vector for the empty box */
    GUCEF_GEOMETRY_FORCE_INLINE TVector GetHalfExtents( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return GetExtents() * TTraits::Half();
    }

    /** The corner for the bits of 'index': bit 0 selects max x, bit 1 max y, bit 2 max z (higher bits are ignored) */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetCorner( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TPoint( 0 != ( index & 1 ) ? max.x : min.x, 0 != ( index & 2 ) ? max.y : min.y, 0 != ( index & 4 ) ? max.z : min.z );
    }

    /** The volume; 0 for the empty box */
    GUCEF_GEOMETRY_FORCE_INLINE T GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TTraits::Zero();
        TVector e = max - min;
        return ( e.x * e.y ) * e.z;
    }

    /** The area of the six faces; 0 for the empty box */
    T GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Combination                                                         */
    /*---------------------------------------------------------------------*/

    /** The box grown by 'margin' on every side (shrunk for a negative margin; may become empty); the empty box stays empty */
    GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D Expanded( const T& margin ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return *this;
        TVector m( margin, margin, margin );
        return CTAxisAlignedBox3D( min - m, max + m );
    }

    /** The smallest box that holds this box and the point; the box of that single point for any empty box */
    GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D ExpandedToInclude( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return CTAxisAlignedBox3D( point, point );
        return CTAxisAlignedBox3D( min.Min( point ), max.Max( point ) );
    }

    /** The smallest box that holds both; any empty box (not only the default one) is the identity */
    GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D Merged( const CTAxisAlignedBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return other;
        if ( other.IsEmpty() )
            return *this;
        return CTAxisAlignedBox3D( min.Min( other.min ), max.Max( other.max ) );
    }

    /** The common part; the empty box when the boxes do not touch */
    GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D Intersection( const CTAxisAlignedBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        CTAxisAlignedBox3D result( min.Max( other.min ), max.Min( other.max ) );
        return result.IsEmpty() ? CTAxisAlignedBox3D() : result;
    }

    /** True when the closed boxes share a point; false when either is empty */
    GUCEF_GEOMETRY_FORCE_INLINE bool Intersects( const CTAxisAlignedBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !IsEmpty() && !other.IsEmpty() && min.x <= other.max.x && other.min.x <= max.x && min.y <= other.max.y && other.min.y <= max.y && min.z <= other.max.z && other.min.z <= max.z;
    }

    /*---------------------------------------------------------------------*/
    /*  Point queries                                                       */
    /*---------------------------------------------------------------------*/

    /** True when the point is inside or on the boundary; false for the empty box */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return min.x <= point.x && point.x <= max.x && min.y <= point.y && point.y <= max.y && min.z <= point.z && point.z <= max.z;
    }

    /** True when 'other' lies inside this box; an empty 'other' is inside every box, nothing is inside the empty box */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const CTAxisAlignedBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( other.IsEmpty() )
            return true;
        return Contains( other.min ) && Contains( other.max );
    }

    /** The point of the box nearest to 'point': the point itself when inside, otherwise clamped per axis; 'point' for the empty box */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return point;
        return point.Max( min ).Min( max );
    }

    /** Squared distance from the box to the point, 0 inside; the largest value of the kind for the empty box */
    T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TTraits::Max();
        return ( point - ClosestPoint( point ) ).GetSquaredLength();
    }

    /** Distance from the box to the point, 0 inside; the largest value of the kind for the empty box */
    T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TTraits::Max();
        return ( point - ClosestPoint( point ) ).GetLength();
    }

    /** The axis aligned bounds of the box: the box itself */
    GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return *this; }

    /** The point of the box furthest along 'direction' (the corner); the origin for the empty box */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TPoint();
        return TPoint( TTraits::IsNegative( direction.x ) ? min.x : max.x, TTraits::IsNegative( direction.y ) ? min.y : max.y, TTraits::IsNegative( direction.z ) ? min.z : max.z );
    }

    /**
     *  Slab test with the inverse direction. An axis the ray runs parallel to is handled without IEEE infinities
     *  (outside the slab it is a miss), so a ray that starts on a face never produces NaN. A ray starting inside the
     *  box (or on its boundary) hits at distance 0 with RAYHIT_STARTED_INSIDE and the normal opposite to the ray.
     *  False for the empty box, for boxes behind the ray and for hits further than maxDistance.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends the 12 edges of the box; nothing for the empty box. 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /*---------------------------------------------------------------------*/
    /*  Comparison                                                          */
    /*---------------------------------------------------------------------*/

    /** Exact comparison of both corners */
    /** The box around the transformed box by the method of Arvo (Graphics Gems I): exact for the box as a set of points under an affine matrix, so the result is the tightest axis aligned box around the image; the empty box stays empty. Not a rigid image of the box: rotated boxes grow */
    CTAxisAlignedBox3D Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTAxisAlignedBox3D& a, const CTAxisAlignedBox3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return a.min == b.min && a.max == b.max;
    }

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTAxisAlignedBox3D& a, const CTAxisAlignedBox3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /** Both corners equal within the default tolerances of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTAxisAlignedBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return min.Equals( other.min ) && max.Equals( other.max );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "AxisAlignedBox3D(min (x, y, z), max (x, y, z))" */
    CORE::CString ToString( void ) const
    {
        return "AxisAlignedBox3D(min " + min.ToString() + ", max " + max.ToString() + ")";
    }

    /** Parses the ToString form (white space allowed); false leaves this box unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedMin;
        TPoint parsedMax;
        if ( !reader.TryLiteral( "AxisAlignedBox3D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "min" ) || !reader.TryValue( parsedMin ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "max" ) || !reader.TryValue( parsedMax ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        min = parsedMin;
        max = parsedMax;
        return true;
    }

    /** Writes the child nodes min and max into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "min", min, settings ) && SerializeShapeChild( node, "max", max, settings );
    }

    /** Reads the child nodes min and max; a missing or invalid one fails and leaves this box unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedMin;
        TPoint parsedMax;
        if ( !DeserializeShapeChild( node, "min", parsedMin, settings ) || !DeserializeShapeChild( node, "max", parsedMax, settings ) )
            return false;
        min = parsedMin;
        max = parsedMax;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTAxisAlignedBox3D< T >::GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsEmpty() )
        return TTraits::Zero();
    TVector e = max - min;
    return TTraits::Two() * e.Dot( TVector( e.y, e.z, e.x ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTAxisAlignedBox3D< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() )
        return false;                       /* a ray that is not a ray (a NaN or infinity in it) hits nothing */
    if ( IsEmpty() )
        return false;

    const TVector direction = ray.direction.AsVector();
    T tNear = TTraits::Lowest();
    T tFar = TTraits::Max();
    Int32 nearAxis = -1;
    T nearSign = TTraits::One();

    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const T o = ray.origin.GetComponent( axis );
        const T d = direction.GetComponent( axis );
        const T lo = min.GetComponent( axis );
        const T hi = max.GetComponent( axis );

        T inverse = TTraits::Zero();
        bool parallel = TTraits::Zero() == d;
        if ( !parallel )
        {
            inverse = TTraits::One() / d;
            parallel = !TTraits::IsFinite( inverse );
        }
        if ( parallel )
        {
            if ( o < lo || hi < o )
                return false;
            continue;
        }

        T t1 = ( lo - o ) * inverse;
        T t2 = ( hi - o ) * inverse;
        /* the entry face is the min face for a positive inverse (normal -1 along the axis), else the max face (+1) */
        const T sign = TTraits::IsNegative( inverse ) ? TTraits::One() : MATH::ComponentNegate( TTraits::One() );
        if ( t2 < t1 )
        {
            T swap = t1;
            t1 = t2;
            t2 = swap;
        }
        if ( tNear < t1 )
        {
            tNear = t1;
            nearAxis = (Int32) axis;
            nearSign = sign;
        }
        if ( t2 < tFar )
            tFar = t2;
        if ( tFar < tNear )
            return false;
    }

    if ( tFar < TTraits::Zero() )
        return false;

    SRayCastResult< T > hit;
    if ( !( TTraits::Zero() < tNear ) )
    {
        hit.distance = TTraits::Zero();
        hit.flags = RAYHIT_STARTED_INSIDE;
        hit.normal = -direction;
        hit.featureIndex = -1;
    }
    else
    {
        if ( maxDistance < tNear )
            return false;
        hit.distance = tNear;
        TVector normal;
        normal.SetComponent( (UInt32) nearAxis, nearSign );
        hit.normal = normal;
        hit.featureIndex = nearAxis;
    }
    hit.point = ray.origin + direction * hit.distance;
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTAxisAlignedBox3D< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    if ( IsEmpty() )
        return;
    for ( UInt32 corner=0; corner<8; ++corner )
    {
        for ( UInt32 bit=1; bit<=4; bit<<=1 )
        {
            if ( 0 == ( corner & bit ) )
                lines.push_back( CTSegment3D< T >( GetCorner( corner ), GetCorner( corner | bit ) ) );
        }
    }
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTAxisAlignedBox3D< T >
CTAxisAlignedBox3D< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsEmpty() )
        return *this;
    TPoint newMin( matrix( 0, 3 ), matrix( 1, 3 ), matrix( 2, 3 ) );
    TPoint newMax = newMin;
    for ( UInt32 i=0; i<3; ++i )
    {
        T low = newMin.GetComponent( i );
        T high = low;
        for ( UInt32 j=0; j<3; ++j )
        {
            const T e = matrix( i, j );
            const T a = e * min.GetComponent( j );
            const T b = e * max.GetComponent( j );
            low = low + TTraits::Min( a, b );
            high = high + TTraits::Max( a, b );
        }
        newMin.SetComponent( i, low );
        newMax.SetComponent( i, high );
    }
    return CTAxisAlignedBox3D( newMin, newMax );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTAxisAlignedBox3D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTAxisAlignedBox3D< Float64 > )

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

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSEGMENT3D_H
#include "gucefGEOMETRY_CTSegment3D.h"
#define GUCEF_GEOMETRY_CTSEGMENT3D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT3D_H ? */

#ifndef GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#include "gucefGEOMETRY_TransformFunctions.h"
#define GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */
