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

#ifndef GUCEF_GEOMETRY_CTPLANEBOUNDEDVOLUME_H
#define GUCEF_GEOMETRY_CTPLANEBOUNDEDVOLUME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>
#include <utility>

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

#ifndef GUCEF_GEOMETRY_CTPLANE_H
#include "gucefGEOMETRY_CTPlane.h"
#define GUCEF_GEOMETRY_CTPLANE_H
#endif /* GUCEF_GEOMETRY_CTPLANE_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#include "gucefGEOMETRY_CTGeometryTolerance.h"
#define GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

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

template < typename T > class CTSphere;
template < typename T > class CTAxisAlignedBox3D;
template < typename T > class CTOrientedBox3D;
template < typename T > class CTFrustum;
template < typename T > class CTRay3D;
template < typename T > class CTSegment3D;

/*-------------------------------------------------------------------------*/

/** A face of the convex body of a plane-bounded volume: the plane it lies in and its corners, counter-clockwise seen from outside */
struct SConvexBodyFace
{
    UInt32 planeIndex;                              /**< the index of the plane of the volume that the face lies in */
    GUCEF::vector< UInt32 > vertexIndices;          /**< indices into the vertex list of the body, in order around the face */

    SConvexBodyFace( void ) : planeIndex( 0 ), vertexIndices() {}
};

/*-------------------------------------------------------------------------*/

/**
 *  A convex region bounded by up to MaxPlanes planes: the intersection of their front half spaces (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: the plane count and a fixed array of MaxPlanes planes (no allocation, ever, for the volume itself).
 *    A volume without planes is the whole space.
 *  - The normals of the planes point into the region: a point is inside when its signed distance to every plane is not
 *    negative. The frustum converts to a plane-bounded volume with FromFrustum.
 *  - Realtime: Contains, Classify, ClipPolygon (the caller provided buffer form), ClipLine, RayCast and the transforms
 *    are realtime-safe; the exact intersection tests, BuildConvexBody, the vector form of ClipPolygon, the wireframe, text
 *    and DataNode members allocate.
 *  - Text: "PlaneBoundedVolume(Plane(...), Plane(...), ...)"; DataNode: child node planes with one child per plane.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTPlaneBoundedVolume
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;
    typedef CTPlane< T > TPlane;

    enum { MaxPlanes = 32 };

    /** The whole space: no planes */
    CTPlaneBoundedVolume( void ) : m_count( 0 ) {}

    /** The region in front of the given planes ( at most MaxPlanes are taken ) */
    CTPlaneBoundedVolume( const TPlane* planes, UInt32 count );

    /** The six planes of a frustum */
    static CTPlaneBoundedVolume FromFrustum( const CTFrustum< T >& frustum ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The six planes of an axis aligned box (an empty box gives an empty volume: its min and max planes exclude every point) */
    static CTPlaneBoundedVolume FromAxisAlignedBox( const CTAxisAlignedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The six planes of an oriented box */
    static CTPlaneBoundedVolume FromOrientedBox( const CTOrientedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetPlaneCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_count; }

    /** The plane of an index (the first plane for an index at or above the count) */
    GUCEF_GEOMETRY_FORCE_INLINE const TPlane& GetPlane( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_planes[ index < m_count ? index : 0 ]; }

    GUCEF_GEOMETRY_FORCE_INLINE const TPlane* GetPlanes( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_planes; }

    /** Adds a plane; false (volume unchanged) when MaxPlanes planes are present */
    bool AddPlane( const TPlane& plane ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Removes all planes: the whole space */
    GUCEF_GEOMETRY_FORCE_INLINE void Clear( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT { m_count = 0; }

    /** True when the point is not behind any plane */
    bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** OUTSIDE when the shape lies entirely behind one plane, INSIDE when it is in front of all, else INTERSECTING; conservative near edges and corners (see ClassifySphereToPlaneSet) */
    TFrustumClassification Classify( const CTSphere< T >& sphere ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    TFrustumClassification Classify( const CTAxisAlignedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    TFrustumClassification Classify( const CTOrientedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  The tolerant forms (section 7.10): the region is widened by the length tolerance for the magnitude of the operands,
     *  so a point or shape within it of the boundary counts as inside; the forms without tolerance are exact.
     */
    bool Contains( const TPoint& point, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    TFrustumClassification Classify( const CTSphere< T >& sphere, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    TFrustumClassification Classify( const CTAxisAlignedBox3D< T >& box, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    TFrustumClassification Classify( const CTOrientedBox3D< T >& box, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Conservative overlap test: false only when Classify says OUTSIDE (never false for a shape that overlaps the volume) */
    GUCEF_GEOMETRY_FORCE_INLINE bool Intersects( const CTSphere< T >& sphere ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return FRUSTUMCLASSIFICATION_OUTSIDE != Classify( sphere ); }
    GUCEF_GEOMETRY_FORCE_INLINE bool Intersects( const CTAxisAlignedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return FRUSTUMCLASSIFICATION_OUTSIDE != Classify( box ); }
    GUCEF_GEOMETRY_FORCE_INLINE bool Intersects( const CTOrientedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return FRUSTUMCLASSIFICATION_OUTSIDE != Classify( box ); }

    /**
     *  Exact overlap tests: the plane classification decides the clear cases, the others are decided on the polytope of
     *  the planes of the volume and of the shape's bounding box (the volume cut down to where the shape can be): a box
     *  overlaps when that polytope has a corner, a sphere when its centre is within the radius of that polytope. This
     *  also works for unbounded volumes (open ones, frusta with an infinite far plane). Allocates.
     */
    bool IntersectsExact( const CTSphere< T >& sphere ) const;
    bool IntersectsExact( const CTAxisAlignedBox3D< T >& box ) const;
    bool IntersectsExact( const CTOrientedBox3D< T >& box ) const;

    /**
     *  Clips the polygon ( 'count' points ) to the volume (Sutherland-Hodgman against every plane, in order). The
     *  result has at most count + GetPlaneCount() points when the polygon is convex; 'out' and 'scratch' must each have
     *  room for 'capacity' points ( capacity at least count + GetPlaneCount() is always enough for a convex polygon ).
     *  False (outCount 0) when the capacity is exceeded; true with outCount 0 when nothing is left. A polygon on a
     *  boundary plane is kept. Allocation-free.
     */
    bool ClipPolygon( const TPoint* points, UInt32 count, TPoint* out, TPoint* scratch, UInt32 capacity, UInt32& outCount ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** As above, appending to a vector (cleared first); allocates */
    void ClipPolygon( const TPoint* points, UInt32 count, GUCEF::vector< TPoint >& out ) const;

    /**
     *  Clips the segment to the volume: true with the clipped parameter interval [ tEnter, tExit ] within [ 0, 1 ] when
     *  part of the segment is inside.
     */
    bool ClipSegment( const CTSegment3D< T >& segment, T& tEnter, T& tExit ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Cyrus-Beck clipping (see RayCastPlaneSet): hit where the ray enters with the outward plane normal, or distance 0 and RAYHIT_STARTED_INSIDE; the feature index is the plane index */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  The polyhedron of the volume (the equivalent of the convex body of Ogre): the corners (the points where three
     *  planes meet that are inside the volume, duplicates merged within the tolerance) and the faces (one per plane that
     *  has at least three corners, corners counter-clockwise seen from outside, a plane equal to an earlier one gives no
     *  second face). False (outputs cleared) when the volume has fewer than four planes, no corner, or is not a closed
     *  solid (an unbounded volume: some edge is not shared by exactly two faces). Allocates.
     */
    bool BuildConvexBody( GUCEF::vector< TPoint >& vertices, GUCEF::vector< SConvexBodyFace >& faces ) const;

    /** The axis aligned box around the corners of the convex body; false (out unchanged) when there is no closed body (also for unbounded volumes) */
    bool TryGetBounds( CTAxisAlignedBox3D< T >& out ) const;

    /** Appends the edges of the convex body */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /** The image under an affine matrix (the region stays a region): false (out unchanged) when the linear part is singular */
    bool TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTPlaneBoundedVolume& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend bool operator==( const CTPlaneBoundedVolume& a, const CTPlaneBoundedVolume& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( a.m_count != b.m_count )
            return false;
        for ( UInt32 i=0; i<a.m_count; ++i )
        {
            if ( !( a.m_planes[ i ].normal == b.m_planes[ i ].normal ) || !( a.m_planes[ i ].offset == b.m_planes[ i ].offset ) )
                return false;
        }
        return true;
    }
    friend bool operator!=( const CTPlaneBoundedVolume& a, const CTPlaneBoundedVolume& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** The same number of planes, equal within the default tolerances of the kind in order */
    bool Equals( const CTPlaneBoundedVolume& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( m_count != other.m_count )
            return false;
        for ( UInt32 i=0; i<m_count; ++i )
        {
            if ( !m_planes[ i ].Equals( other.m_planes[ i ] ) )
                return false;
        }
        return true;
    }

    /** "PlaneBoundedVolume(Plane(...), Plane(...), ...)" */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( "PlaneBoundedVolume(" );
        for ( UInt32 i=0; i<m_count; ++i )
        {
            if ( i > 0 )
                text += ", ";
            text += m_planes[ i ].ToString();
        }
        text += ')';
        return text;
    }

    /** Parses the ToString form; false leaves this volume unchanged (also for more than MaxPlanes planes) */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        if ( !reader.TryLiteral( "PlaneBoundedVolume" ) || !reader.TryChar( '(' ) )
            return false;
        CTPlaneBoundedVolume parsed;
        if ( !reader.TryChar( ')' ) )
        {
            for ( ;; )
            {
                TPlane plane;
                if ( !reader.TryShape( plane ) || !parsed.AddPlane( plane ) )
                    return false;
                if ( reader.TryChar( ',' ) )
                    continue;
                if ( reader.TryChar( ')' ) )
                    break;
                return false;
            }
        }
        if ( !reader.AtEnd() )
            return false;
        *this = parsed;
        return true;
    }

    /** Writes the child node planes with one child node per plane into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        GUCEF::vector< TPlane > planes( m_planes, m_planes + m_count );
        return SerializeShapePoints( node, "planes", planes, settings );
    }

    /** Reads the child node planes; a missing or invalid one (or more than MaxPlanes planes) fails and leaves this volume unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        GUCEF::vector< TPlane > planes;
        if ( !DeserializeShapePoints( node, "planes", planes, settings ) || planes.size() > (size_t) MaxPlanes )
            return false;
        CTPlaneBoundedVolume parsed( planes.empty() ? GUCEF_NULL : &planes[ 0 ], (UInt32) planes.size() );
        *this = parsed;
        return true;
    }

    private:

    /** The corners and faces of the polytope of 'count' planes; with 'requireClosed' false when the polytope is not a closed solid (unbounded) */
    static bool BuildBody( const TPlane* planes, UInt32 count, bool requireClosed, GUCEF::vector< TPoint >& vertices, GUCEF::vector< SConvexBodyFace >& faces );

    /** The planes of the volume followed by the six planes of the box: the volume cut down to the box */
    UInt32 CombineWithBox( const TPoint& low, const TPoint& high, TPlane ( &planes )[ MaxPlanes + 6 ] ) const;

    TPlane m_planes[ MaxPlanes ];
    UInt32 m_count;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTPlaneBoundedVolume< T >::CTPlaneBoundedVolume( const TPlane* planes, UInt32 count )
    : m_count( 0 )
{
    if ( GUCEF_NULL == planes )
        return;
    for ( UInt32 i=0; i<count && i<(UInt32) MaxPlanes; ++i )
        m_planes[ m_count++ ] = planes[ i ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTPlaneBoundedVolume< T >
CTPlaneBoundedVolume< T >::FromFrustum( const CTFrustum< T >& frustum ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CTPlaneBoundedVolume volume;
    for ( UInt32 i=0; i<6; ++i )
        volume.AddPlane( frustum.GetPlane( i ) );
    return volume;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTPlaneBoundedVolume< T >
CTPlaneBoundedVolume< T >::FromAxisAlignedBox( const CTAxisAlignedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CTPlaneBoundedVolume volume;
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        TVector unit;
        unit.SetComponent( axis, TTraits::One() );
        const TDirection normal = TDirection::FromNormalized( unit );
        volume.AddPlane( TPlane( normal, box.min.GetComponent( axis ) ) );                                         /* inside: x >= min */
        volume.AddPlane( TPlane( -normal, MATH::ComponentNegate( box.max.GetComponent( axis ) ) ) );               /* inside: x <= max */
    }
    return volume;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTPlaneBoundedVolume< T >
CTPlaneBoundedVolume< T >::FromOrientedBox( const CTOrientedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CTPlaneBoundedVolume volume;
    TVector axes[ 3 ];
    box.GetAxes( axes );
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const TDirection normal = TDirection::FromNormalized( axes[ axis ] );
        const T centreAlong = normal.Dot( box.centre.ToPositionVector() );
        const T half = box.halfExtents.GetComponent( axis );
        volume.AddPlane( TPlane( normal, centreAlong - half ) );
        volume.AddPlane( TPlane( -normal, MATH::ComponentNegate( centreAlong + half ) ) );
    }
    return volume;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPlaneBoundedVolume< T >::AddPlane( const TPlane& plane ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_count >= (UInt32) MaxPlanes )
        return false;
    m_planes[ m_count++ ] = plane;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlaneBoundedVolume< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return PlaneSetContains( m_planes, m_count, point );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTPlaneBoundedVolume< T >::Classify( const CTSphere< T >& sphere ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( TTraits::IsNegative( sphere.radius ) )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    return ClassifySphereToPlaneSet( m_planes, m_count, sphere.centre, sphere.radius );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTPlaneBoundedVolume< T >::Classify( const CTAxisAlignedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( box.IsEmpty() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    return ClassifyBoxToPlaneSet( m_planes, m_count, box.GetCentre(), box.GetHalfExtents() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTPlaneBoundedVolume< T >::Classify( const CTOrientedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !box.IsValid() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    TVector axes[ 3 ];
    box.GetAxes( axes );
    return ClassifyOrientedBoxToPlaneSet( m_planes, m_count, box.centre, axes, box.halfExtents );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlaneBoundedVolume< T >::ClipPolygon( const TPoint* points, UInt32 count, TPoint* out, TPoint* scratch, UInt32 capacity, UInt32& outCount ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ClipPolygonToPlaneSet( m_planes, m_count, points, count, out, scratch, capacity, outCount );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPlaneBoundedVolume< T >::ClipPolygon( const TPoint* points, UInt32 count, GUCEF::vector< TPoint >& out ) const
{
    /* for a non convex polygon the capacity grows until the result fits */
    UInt32 capacity = count + m_count + 1;
    for ( ;; )
    {
        GUCEF::vector< TPoint > a( capacity );
        GUCEF::vector< TPoint > b( capacity );
        UInt32 outCount = 0;
        if ( ClipPolygon( points, count, &a[ 0 ], &b[ 0 ], capacity, outCount ) )
        {
            out.assign( a.begin(), a.begin() + outCount );
            return;
        }
        if ( capacity > 1000000 )
            break;
        capacity *= 2;
    }
    out.clear();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlaneBoundedVolume< T >::ClipSegment( const CTSegment3D< T >& segment, T& tEnter, T& tExit ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !segment.start.IsFinite() || !segment.end.IsFinite() )
        return false;
    const TVector delta = segment.end - segment.start;
    const T length = delta.GetLength();
    if ( !( TTraits::Zero() < length ) )
    {
        if ( !Contains( segment.start ) )
            return false;
        tEnter = TTraits::Zero();
        tExit = TTraits::One();
        return true;
    }
    T enter = TTraits::Zero();
    T leave = TTraits::Zero();
    Int32 plane = -1;
    if ( !ClipLineToPlaneSet( m_planes, m_count, segment.start, delta / length, TTraits::Zero(), length, enter, leave, plane ) )
        return false;
    tEnter = enter / length;
    tExit = leave / length;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlaneBoundedVolume< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return RayCastPlaneSet( m_planes, m_count, ray, maxDistance, result );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPlaneBoundedVolume< T >::BuildBody( const TPlane* planes, UInt32 count, bool requireClosed, GUCEF::vector< TPoint >& vertices, GUCEF::vector< SConvexBodyFace >& faces )
{
    vertices.clear();
    faces.clear();
    if ( count < 4 )
        return false;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !TTraits::IsFinite( planes[ i ].offset ) || !planes[ i ].normal.AsVector().IsFinite() )
            return false;
    }
    const CTGeometryTolerance< T > tolerance;

    /*
     *  Every corner has its own tolerance, scaled by its own magnitude and not by the largest plane offset: a frustum with
     *  an infinite far plane has an offset of 1e18, and with that scale the corners at the near plane would all merge.
     */
    for ( UInt32 i=0; i<count; ++i )
    {
        for ( UInt32 j=i+1; j<count; ++j )
        {
            for ( UInt32 k=j+1; k<count; ++k )
            {
                TPoint corner;
                if ( !TPlane::IntersectPlanes( planes[ i ], planes[ j ], planes[ k ], corner ) || !corner.IsFinite() )
                    continue;
                const T cornerScale = TTraits::Max( TTraits::One(), TTraits::Max( TTraits::Abs( corner.x ), TTraits::Max( TTraits::Abs( corner.y ), TTraits::Abs( corner.z ) ) ) );
                const T length = tolerance.GetLengthTolerance( cornerScale );
                bool inside = true;
                for ( UInt32 l=0; l<count && inside; ++l )
                {
                    if ( planes[ l ].GetSignedDistance( corner ) < MATH::ComponentNegate( length ) )
                        inside = false;
                }
                if ( !inside )
                    continue;
                bool duplicate = false;
                for ( size_t v=0; v<vertices.size() && !duplicate; ++v )
                {
                    const T pairScale = TTraits::Max( cornerScale, TTraits::Max( TTraits::One(), vertices[ v ].ToPositionVector().GetChebyshevLength() ) );
                    duplicate = ( corner - vertices[ v ] ).GetLength() <= tolerance.GetLengthTolerance( pairScale );
                }
                if ( !duplicate )
                    vertices.push_back( corner );
            }
        }
    }
    if ( vertices.empty() )
        return false;

    for ( UInt32 p=0; p<count; ++p )
    {
        bool repeated = false;
        for ( UInt32 q=0; q<p && !repeated; ++q )
            repeated = planes[ q ].normal.Equals( planes[ p ].normal ) && TTraits::EqualsWithin( planes[ q ].offset, planes[ p ].offset, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
        if ( repeated )
            continue;

        const TVector outward = -planes[ p ].normal.AsVector();
        GUCEF::vector< UInt32 > onPlane;
        TVector centroid;
        for ( size_t v=0; v<vertices.size(); ++v )
        {
            const T length = tolerance.GetLengthTolerance( TTraits::Max( TTraits::One(), vertices[ v ].ToPositionVector().GetChebyshevLength() ) );
            if ( TTraits::Abs( planes[ p ].GetSignedDistance( vertices[ v ] ) ) <= length )
            {
                onPlane.push_back( (UInt32) v );
                centroid = centroid + vertices[ v ].ToPositionVector();
            }
        }
        if ( onPlane.size() < 3 )
            continue;
        centroid = centroid / TTraits::FromInt32( (Int32) onPlane.size() );

        /* the corners around the centroid, counter-clockwise about the outward normal */
        const TVector u = outward.GetAnyPerpendicular().Normalized();
        const TVector w = outward.Cross( u );
        GUCEF::vector< std::pair< T, UInt32 > > keyed;
        keyed.reserve( onPlane.size() );
        for ( size_t v=0; v<onPlane.size(); ++v )
        {
            const TVector d = vertices[ onPlane[ v ] ].ToPositionVector() - centroid;
            keyed.push_back( std::pair< T, UInt32 >( TTraits::Atan2( d.Dot( w ), d.Dot( u ) ), onPlane[ v ] ) );
        }
        std::sort( keyed.begin(), keyed.end() );
        SConvexBodyFace face;
        face.planeIndex = p;
        for ( size_t v=0; v<keyed.size(); ++v )
            face.vertexIndices.push_back( keyed[ v ].second );
        faces.push_back( face );
    }

    if ( requireClosed )
    {
        /* a closed solid: every edge is shared by exactly two faces; an unbounded volume has open edges (or no complete face) */
        GUCEF::vector< std::pair< UInt32, UInt32 > > edges;
        for ( size_t f=0; f<faces.size(); ++f )
        {
            const size_t n = faces[ f ].vertexIndices.size();
            for ( size_t e=0; e<n; ++e )
            {
                const UInt32 a = faces[ f ].vertexIndices[ e ];
                const UInt32 b = faces[ f ].vertexIndices[ ( e + 1 ) % n ];
                edges.push_back( a < b ? std::pair< UInt32, UInt32 >( a, b ) : std::pair< UInt32, UInt32 >( b, a ) );
            }
        }
        std::sort( edges.begin(), edges.end() );
        bool closed = faces.size() >= 4 && !edges.empty();
        for ( size_t e=0; e<edges.size() && closed; )
        {
            size_t run = 1;
            while ( e + run < edges.size() && edges[ e + run ] == edges[ e ] )
                ++run;
            if ( 2 != run )
                closed = false;
            e += run;
        }
        if ( !closed )
        {
            vertices.clear();
            faces.clear();
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPlaneBoundedVolume< T >::BuildConvexBody( GUCEF::vector< TPoint >& vertices, GUCEF::vector< SConvexBodyFace >& faces ) const
{
    return BuildBody( m_planes, m_count, true, vertices, faces );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPlaneBoundedVolume< T >::TryGetBounds( CTAxisAlignedBox3D< T >& out ) const
{
    GUCEF::vector< TPoint > vertices;
    GUCEF::vector< SConvexBodyFace > faces;
    if ( !BuildConvexBody( vertices, faces ) )
        return false;
    out = CTAxisAlignedBox3D< T >::FromPoints( &vertices[ 0 ], (UInt32) vertices.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTPlaneBoundedVolume< T >::CombineWithBox( const TPoint& low, const TPoint& high, TPlane ( &planes )[ MaxPlanes + 6 ] ) const
{
    UInt32 n = 0;
    for ( UInt32 i=0; i<m_count; ++i )
        planes[ n++ ] = m_planes[ i ];
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        TVector unit;
        unit.SetComponent( axis, TTraits::One() );
        const TDirection normal = TDirection::FromNormalized( unit );
        planes[ n++ ] = TPlane( normal, low.GetComponent( axis ) );
        planes[ n++ ] = TPlane( -normal, MATH::ComponentNegate( high.GetComponent( axis ) ) );
    }
    return n;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPlaneBoundedVolume< T >::IntersectsExact( const CTSphere< T >& sphere ) const
{
    const TFrustumClassification coarse = Classify( sphere );
    if ( FRUSTUMCLASSIFICATION_INTERSECTING != coarse )
        return FRUSTUMCLASSIFICATION_OUTSIDE != coarse;
    if ( Contains( sphere.centre ) )
        return true;

    /*
     *  The overlap lies within the bounding box of the sphere, so the volume is cut down to that box: the result is a
     *  bounded polytope even when the volume is not (open, or with an infinite far plane), and the distance from the
     *  centre to the polytope decides.
     */
    const TVector r( sphere.radius, sphere.radius, sphere.radius );
    TPlane planes[ MaxPlanes + 6 ];
    const UInt32 n = CombineWithBox( sphere.centre - r, sphere.centre + r, planes );
    GUCEF::vector< TPoint > vertices;
    GUCEF::vector< SConvexBodyFace > faces;
    if ( !BuildBody( planes, n, false, vertices, faces ) || faces.empty() )
        return false;                                   /* the volume does not reach the box of the sphere */
    const T limit = sphere.radius * sphere.radius;
    for ( size_t f=0; f<faces.size(); ++f )
    {
        const SConvexBodyFace& face = faces[ f ];
        const TPlane& plane = planes[ face.planeIndex ];
        const TPoint projected = plane.Project( sphere.centre );
        /* the point is inside the face when it is on the inner side of every edge */
        bool insideFace = true;
        T best = ( sphere.centre - projected ).GetSquaredLength();
        const size_t count = face.vertexIndices.size();
        for ( size_t e=0; e<count; ++e )
        {
            const TPoint& a = vertices[ face.vertexIndices[ e ] ];
            const TPoint& b = vertices[ face.vertexIndices[ ( e + 1 ) % count ] ];
            if ( TTraits::IsNegative( ( b - a ).Cross( projected - a ).Dot( -plane.normal.AsVector() ) ) )
            {
                insideFace = false;
                break;
            }
        }
        if ( !insideFace )
        {
            best = TTraits::Max();
            for ( size_t e=0; e<count; ++e )
            {
                const CTSegment3D< T > edge( vertices[ face.vertexIndices[ e ] ], vertices[ face.vertexIndices[ ( e + 1 ) % count ] ] );
                best = TTraits::Min( best, edge.GetSquaredDistance( sphere.centre ) );
            }
        }
        if ( best <= limit )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPlaneBoundedVolume< T >::IntersectsExact( const CTAxisAlignedBox3D< T >& box ) const
{
    if ( box.IsEmpty() )
        return false;
    const TFrustumClassification coarse = Classify( box );
    if ( FRUSTUMCLASSIFICATION_INTERSECTING != coarse )
        return FRUSTUMCLASSIFICATION_OUTSIDE != coarse;

    /* the volume and the box share a point exactly when the polytope of the planes of both has a corner (the box makes it bounded) */
    TPlane planes[ MaxPlanes + 6 ];
    const UInt32 n = CombineWithBox( box.min, box.max, planes );
    GUCEF::vector< TPoint > vertices;
    GUCEF::vector< SConvexBodyFace > faces;
    return BuildBody( planes, n, false, vertices, faces );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPlaneBoundedVolume< T >::IntersectsExact( const CTOrientedBox3D< T >& box ) const
{
    if ( !box.IsValid() )
        return false;
    const TFrustumClassification coarse = Classify( box );
    if ( FRUSTUMCLASSIFICATION_INTERSECTING != coarse )
        return FRUSTUMCLASSIFICATION_OUTSIDE != coarse;

    TPlane planes[ MaxPlanes + 6 ];
    UInt32 n = 0;
    for ( UInt32 i=0; i<m_count; ++i )
        planes[ n++ ] = m_planes[ i ];
    const CTPlaneBoundedVolume boxVolume = FromOrientedBox( box );
    for ( UInt32 i=0; i<boxVolume.GetPlaneCount(); ++i )
        planes[ n++ ] = boxVolume.GetPlane( i );
    GUCEF::vector< TPoint > vertices;
    GUCEF::vector< SConvexBodyFace > faces;
    return BuildBody( planes, n, false, vertices, faces );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPlaneBoundedVolume< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    GUCEF::vector< TPoint > vertices;
    GUCEF::vector< SConvexBodyFace > faces;
    if ( !BuildConvexBody( vertices, faces ) )
        return;
    for ( size_t f=0; f<faces.size(); ++f )
    {
        const size_t n = faces[ f ].vertexIndices.size();
        for ( size_t e=0; e<n; ++e )
        {
            const UInt32 a = faces[ f ].vertexIndices[ e ];
            const UInt32 b = faces[ f ].vertexIndices[ ( e + 1 ) % n ];
            if ( a < b )                                /* every edge is shared by two faces: once is enough */
                lines.push_back( CTSegment3D< T >( vertices[ a ], vertices[ b ] ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlaneBoundedVolume< T >::TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTPlaneBoundedVolume& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CTPlaneBoundedVolume result;
    for ( UInt32 i=0; i<m_count; ++i )
    {
        TPlane moved;
        if ( !m_planes[ i ].TryTransformed( matrix, moved ) )
            return false;
        result.AddPlane( moved );
    }
    out = result;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlaneBoundedVolume< T >::Contains( const TPoint& point, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return PlaneSetContains( m_planes, m_count, point, tolerance );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTPlaneBoundedVolume< T >::Classify( const CTSphere< T >& sphere, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( TTraits::IsNegative( sphere.radius ) )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    return ClassifySphereToPlaneSet( m_planes, m_count, sphere.centre, sphere.radius, tolerance );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTPlaneBoundedVolume< T >::Classify( const CTAxisAlignedBox3D< T >& box, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( box.IsEmpty() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    return ClassifyBoxToPlaneSet( m_planes, m_count, box.GetCentre(), box.GetHalfExtents(), tolerance );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTPlaneBoundedVolume< T >::Classify( const CTOrientedBox3D< T >& box, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !box.IsValid() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    TVector axes[ 3 ];
    box.GetAxes( axes );
    return ClassifyOrientedBoxToPlaneSet( m_planes, m_count, box.centre, axes, box.halfExtents, tolerance );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPlaneBoundedVolume< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPlaneBoundedVolume< Float64 > )

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

#ifndef GUCEF_GEOMETRY_PLANESETFUNCTIONS_H
#include "gucefGEOMETRY_PlaneSetFunctions.h"
#define GUCEF_GEOMETRY_PLANESETFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_PLANESETFUNCTIONS_H ? */

#ifndef GUCEF_GEOMETRY_CTFRUSTUM_H
#include "gucefGEOMETRY_CTFrustum.h"
#define GUCEF_GEOMETRY_CTFRUSTUM_H
#endif /* GUCEF_GEOMETRY_CTFRUSTUM_H ? */

#ifndef GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#include "gucefGEOMETRY_CTOrientedBox3D.h"
#define GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTORIENTEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSPHERE_H
#include "gucefGEOMETRY_CTSphere.h"
#define GUCEF_GEOMETRY_CTSPHERE_H
#endif /* GUCEF_GEOMETRY_CTSPHERE_H ? */

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

#endif /* GUCEF_GEOMETRY_CTPLANEBOUNDEDVOLUME_H ? */
