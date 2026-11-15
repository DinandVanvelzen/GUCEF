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

#ifndef GUCEF_GEOMETRY_VOLUMEQUERIES_H
#define GUCEF_GEOMETRY_VOLUMEQUERIES_H

/*
 *  The cells of the query matrix for the two convex regions that are bounded by planes, the frustum and the
 *  plane-bounded volume (sections 7.5.2 and 7.5.5 of the design): classification, overlap and containment of
 *  segments, triangles, spheres, capsules, boxes, cylinders and cones. The pairs with the volumes are answered here, in
 *  the header of the volumes, so that the headers of the lighter shapes need not include them. See
 *  gucefGEOMETRY_PointQueries.h for the conventions of the facade.
 *
 *  - Classify( volume, shape ): OUTSIDE when the shape lies entirely behind one plane, INSIDE when it is in front of all
 *    planes, INTERSECTING otherwise. The test is per plane, so it is conservative near the edges and corners of the
 *    region: a shape that is reported INTERSECTING may still miss the region, but a shape that overlaps the region is
 *    never reported OUTSIDE.
 *  - Intersects( volume, shape ): not OUTSIDE (conservative) for spheres, boxes, capsules, cylinders and cones; exact
 *    for segments (clipping against the planes) and triangles (polygon clipping, Sutherland-Hodgman). The exact
 *    tests for spheres and boxes are IntersectsExact, which builds the convex body and allocates.
 *  - Contains( volume, shape ): true when the shape lies entirely inside; exact, since the region is the intersection
 *    of the half spaces.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_H
#include "gucefGEOMETRY_OrientedBoxQueries.h"
#define GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_H
#endif /* GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CTCYLINDER_H
#include "gucefGEOMETRY_CTCylinder.h"
#define GUCEF_GEOMETRY_CTCYLINDER_H
#endif /* GUCEF_GEOMETRY_CTCYLINDER_H ? */

#ifndef GUCEF_GEOMETRY_CTCONE_H
#include "gucefGEOMETRY_CTCone.h"
#define GUCEF_GEOMETRY_CTCONE_H
#endif /* GUCEF_GEOMETRY_CTCONE_H ? */

#ifndef GUCEF_GEOMETRY_CTFRUSTUM_H
#include "gucefGEOMETRY_CTFrustum.h"
#define GUCEF_GEOMETRY_CTFRUSTUM_H
#endif /* GUCEF_GEOMETRY_CTFRUSTUM_H ? */

#ifndef GUCEF_GEOMETRY_CTPLANEBOUNDEDVOLUME_H
#include "gucefGEOMETRY_CTPlaneBoundedVolume.h"
#define GUCEF_GEOMETRY_CTPLANEBOUNDEDVOLUME_H
#endif /* GUCEF_GEOMETRY_CTPLANEBOUNDEDVOLUME_H ? */

#ifndef GUCEF_GEOMETRY_PLANESETFUNCTIONS_H
#include "gucefGEOMETRY_PlaneSetFunctions.h"
#define GUCEF_GEOMETRY_PLANESETFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_PLANESETFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The planes of a volume and their number (the normals point into the volume) */
template < typename T > GUCEF_GEOMETRY_FORCE_INLINE const CTPlane< T >* GetVolumePlanes( const CTFrustum< T >& volume ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return volume.GetPlanes(); }
template < typename T > GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetVolumePlaneCount( const CTFrustum< T >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return 6; }
template < typename T > GUCEF_GEOMETRY_FORCE_INLINE const CTPlane< T >* GetVolumePlanes( const CTPlaneBoundedVolume< T >& volume ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return volume.GetPlanes(); }
template < typename T > GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetVolumePlaneCount( const CTPlaneBoundedVolume< T >& volume ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return volume.GetPlaneCount(); }

/*-------------------------------------------------------------------------*/

/**
 *  The classification of a convex shape that has GetSupport against the planes: per plane the support points along the
 *  normal and against it give the highest and lowest signed distance of the shape. Conservative as described above.
 */
template < typename T, typename TShape >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
ClassifyConvexToPlaneSet( const CTPlane< T >* planes, UInt32 count, const TShape& shape, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    bool straddles = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        const MATH::CTVector3D< T > n = planes[ i ].normal.AsVector();
        const MATH::CTPoint3D< T > high = shape.GetSupport( n );
        const MATH::CTPoint3D< T > low = shape.GetSupport( -n );
        const T highDistance = planes[ i ].GetSignedDistance( high );
        const T lowDistance = planes[ i ].GetSignedDistance( low );
        if ( !TTraits::IsFinite( highDistance ) || !TTraits::IsFinite( lowDistance ) )
            return FRUSTUMCLASSIFICATION_OUTSIDE;       /* a shape that is not finite is nowhere */
        const T slack = tolerance.GetLengthTolerance( TTraits::Max( TTraits::Max( TTraits::Abs( high.x ), TTraits::Abs( high.y ) ), TTraits::Abs( high.z ) ) );
        if ( highDistance < MATH::ComponentNegate( slack ) )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( lowDistance < MATH::ComponentNegate( slack ) )
            straddles = true;
    }
    return straddles ? FRUSTUMCLASSIFICATION_INTERSECTING : FRUSTUMCLASSIFICATION_INSIDE;
}

/** The exact classification: a tolerance of zero */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE TFrustumClassification
ClassifyConvexToPlaneSet( const CTPlane< T >* planes, UInt32 count, const TShape& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const T zero = MATH::CTNumericTraits< T >::Zero();
    return ClassifyConvexToPlaneSet( planes, count, shape, CTGeometryTolerance< T >( zero, zero, zero ) );
}

/*-------------------------------------------------------------------------*/

/** True when the segment shares a point with the region: clipping against the planes (Cyrus-Beck) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
SegmentIntersectsPlaneSet( const CTPlane< T >* planes, UInt32 count, const CTSegment3D< T >& segment ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( !segment.start.IsFinite() || !segment.end.IsFinite() )
        return false;
    const MATH::CTVector3D< T > delta = segment.end - segment.start;
    const T length = delta.GetLength();
    if ( !( TTraits::Zero() < length ) )
        return PlaneSetContains( planes, count, segment.start );
    T enter = TTraits::Zero();
    T leave = TTraits::Zero();
    Int32 plane = -1;
    return ClipLineToPlaneSet( planes, count, segment.start, delta / length, TTraits::Zero(), length, enter, leave, plane );
}

/*-------------------------------------------------------------------------*/

/** True when the triangle shares a point with the region: the triangle clipped against the planes is not empty (Sutherland-Hodgman); a volume of more than MaxPlanes planes is not used here, the classification decides then */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
TriangleIntersectsPlaneSet( const CTPlane< T >* planes, UInt32 count, const CTTriangle3D< T >& triangle ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTPoint3D< T > TPoint;
    enum { Capacity = 3 + CTPlaneBoundedVolume< T >::MaxPlanes + 1 };
    if ( !triangle.a.IsFinite() || !triangle.b.IsFinite() || !triangle.c.IsFinite() )
        return false;
    if ( count + 4 > (UInt32) Capacity )
        return FRUSTUMCLASSIFICATION_OUTSIDE != ClassifyConvexToPlaneSet( planes, count, triangle );
    TPoint corners[ 3 ] = { triangle.a, triangle.b, triangle.c };
    TPoint out[ Capacity ];
    TPoint scratch[ Capacity ];
    UInt32 outCount = 0;
    if ( !ClipPolygonToPlaneSet( planes, count, corners, 3, out, scratch, (UInt32) Capacity, outCount ) )
        return true;
    return outCount > 0;
}

/*-------------------------------------------------------------------------*/

/** Defines Classify and Contains of a volume against a shape through the member Classify of the volume (spheres and boxes) */
#define GUCEF_GEOMETRY_VOLUME_MEMBER_QUERIES( TVolume, TShape )                                                           \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE TFrustumClassification                                                                    \
    Classify( const TVolume< T >& volume, const TShape< T >& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT                      \
    {                                                                                                                     \
        return volume.Classify( shape );                                                                                  \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Contains( const TVolume< T >& volume, const TShape< T >& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT                      \
    {                                                                                                                     \
        return FRUSTUMCLASSIFICATION_INSIDE == volume.Classify( shape );                                                  \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Intersects( const TVolume< T >& volume, const TShape< T >& shape, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        return FRUSTUMCLASSIFICATION_OUTSIDE != volume.Classify( shape, tolerance );                                      \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Intersects( const TShape< T >& shape, const TVolume< T >& volume, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        return Intersects( volume, shape, tolerance );                                                                    \
    }                                                                                                                     \
    GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( TVolume, TShape )

/** Defines Classify and Contains of a volume against a convex shape that has GetSupport */
#define GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( TVolume, TShape )                                                           \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE TFrustumClassification                                                                    \
    Classify( const TVolume< T >& volume, const TShape< T >& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT                      \
    {                                                                                                                     \
        return ClassifyConvexToPlaneSet( GetVolumePlanes( volume ), GetVolumePlaneCount( volume ), shape );               \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Contains( const TVolume< T >& volume, const TShape< T >& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT                      \
    {                                                                                                                     \
        return FRUSTUMCLASSIFICATION_INSIDE == Classify( volume, shape );                                                 \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Intersects( const TVolume< T >& volume, const TShape< T >& shape, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        return FRUSTUMCLASSIFICATION_OUTSIDE != ClassifyConvexToPlaneSet( GetVolumePlanes( volume ), GetVolumePlaneCount( volume ), shape, tolerance ); \
    }

/** Defines the conservative Intersects of a volume against a shape in both orders: true unless Classify says OUTSIDE */
#define GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( TVolume, TShape )                                                   \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Intersects( const TVolume< T >& volume, const TShape< T >& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT                    \
    {                                                                                                                     \
        return FRUSTUMCLASSIFICATION_OUTSIDE != Classify( volume, shape );                                                \
    }                                                                                                                     \
    GUCEF_GEOMETRY_SWAPPED_INTERSECTS( TVolume, TShape )

GUCEF_GEOMETRY_VOLUME_MEMBER_QUERIES( CTFrustum, CTSphere )
GUCEF_GEOMETRY_VOLUME_MEMBER_QUERIES( CTFrustum, CTAxisAlignedBox3D )
GUCEF_GEOMETRY_VOLUME_MEMBER_QUERIES( CTFrustum, CTOrientedBox3D )
GUCEF_GEOMETRY_VOLUME_MEMBER_QUERIES( CTPlaneBoundedVolume, CTSphere )
GUCEF_GEOMETRY_VOLUME_MEMBER_QUERIES( CTPlaneBoundedVolume, CTAxisAlignedBox3D )
GUCEF_GEOMETRY_VOLUME_MEMBER_QUERIES( CTPlaneBoundedVolume, CTOrientedBox3D )

GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTFrustum, CTSegment3D )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTFrustum, CTTriangle3D )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTFrustum, CTCapsule )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTFrustum, CTCylinder )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTFrustum, CTCone )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTPlaneBoundedVolume, CTSegment3D )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTPlaneBoundedVolume, CTTriangle3D )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTPlaneBoundedVolume, CTCapsule )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTPlaneBoundedVolume, CTCylinder )
GUCEF_GEOMETRY_VOLUME_CONVEX_QUERIES( CTPlaneBoundedVolume, CTCone )

GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( CTFrustum, CTCapsule )
GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( CTFrustum, CTCylinder )
GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( CTFrustum, CTCone )
GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( CTPlaneBoundedVolume, CTCapsule )
GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( CTPlaneBoundedVolume, CTCylinder )
GUCEF_GEOMETRY_VOLUME_INTERSECTS_BY_CLASSIFY( CTPlaneBoundedVolume, CTCone )

/*-------------------------------------------------------------------------*/

/** True when the segment shares a point with the frustum (exact: clipping against the planes) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTFrustum< T >& volume, const CTSegment3D< T >& segment ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return SegmentIntersectsPlaneSet( GetVolumePlanes( volume ), GetVolumePlaneCount( volume ), segment );
}

/** True when the segment shares a point with the plane-bounded volume (exact: clipping against the planes) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTPlaneBoundedVolume< T >& volume, const CTSegment3D< T >& segment ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return SegmentIntersectsPlaneSet( GetVolumePlanes( volume ), GetVolumePlaneCount( volume ), segment );
}

/** True when the triangle shares a point with the frustum (exact: polygon clipping against the planes) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTFrustum< T >& volume, const CTTriangle3D< T >& triangle ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return TriangleIntersectsPlaneSet( GetVolumePlanes( volume ), GetVolumePlaneCount( volume ), triangle );
}

/** True when the triangle shares a point with the plane-bounded volume (exact: polygon clipping against the planes) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTPlaneBoundedVolume< T >& volume, const CTTriangle3D< T >& triangle ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return TriangleIntersectsPlaneSet( GetVolumePlanes( volume ), GetVolumePlaneCount( volume ), triangle );
}

GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTFrustum, CTSegment3D )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTPlaneBoundedVolume, CTSegment3D )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTFrustum, CTTriangle3D )
GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTPlaneBoundedVolume, CTTriangle3D )

/*-------------------------------------------------------------------------*/

/**
 *  The exact overlap tests of a volume with a sphere or a box: the volume is converted to a plane-bounded volume and the
 *  tests of its convex body decide. Allocates (not realtime-safe); see CTPlaneBoundedVolume::IntersectsExact.
 */
template < typename T >
bool
IntersectsExact( const CTFrustum< T >& volume, const CTSphere< T >& sphere )
{
    return CTPlaneBoundedVolume< T >::FromFrustum( volume ).IntersectsExact( sphere );
}

template < typename T >
bool
IntersectsExact( const CTFrustum< T >& volume, const CTAxisAlignedBox3D< T >& box )
{
    return CTPlaneBoundedVolume< T >::FromFrustum( volume ).IntersectsExact( box );
}

template < typename T >
bool
IntersectsExact( const CTFrustum< T >& volume, const CTOrientedBox3D< T >& box )
{
    return CTPlaneBoundedVolume< T >::FromFrustum( volume ).IntersectsExact( box );
}

template < typename T >
bool
IntersectsExact( const CTPlaneBoundedVolume< T >& volume, const CTSphere< T >& sphere )
{
    return volume.IntersectsExact( sphere );
}

template < typename T >
bool
IntersectsExact( const CTPlaneBoundedVolume< T >& volume, const CTAxisAlignedBox3D< T >& box )
{
    return volume.IntersectsExact( box );
}

template < typename T >
bool
IntersectsExact( const CTPlaneBoundedVolume< T >& volume, const CTOrientedBox3D< T >& box )
{
    return volume.IntersectsExact( box );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_VOLUMEQUERIES_SHAPE( prefix, kind, TShapeClass ) \
    prefix TFrustumClassification ClassifyConvexToPlaneSet< kind, TShapeClass< kind > >( const CTPlane< kind >*, UInt32, const TShapeClass< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_VOLUMEQUERIES_KIND( prefix, kind ) \
    GUCEF_GEOMETRY_VOLUMEQUERIES_SHAPE( prefix, kind, CTSegment3D ) \
    GUCEF_GEOMETRY_VOLUMEQUERIES_SHAPE( prefix, kind, CTTriangle3D ) \
    GUCEF_GEOMETRY_VOLUMEQUERIES_SHAPE( prefix, kind, CTCapsule ) \
    GUCEF_GEOMETRY_VOLUMEQUERIES_SHAPE( prefix, kind, CTCylinder ) \
    GUCEF_GEOMETRY_VOLUMEQUERIES_SHAPE( prefix, kind, CTCone ) \
    prefix bool SegmentIntersectsPlaneSet< kind >( const CTPlane< kind >*, UInt32, const CTSegment3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool TriangleIntersectsPlaneSet< kind >( const CTPlane< kind >*, UInt32, const CTTriangle3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_VOLUMEQUERIES_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_VOLUMEQUERIES_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_VOLUMEQUERIES_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_VOLUMEQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_VOLUMEQUERIES_H ? */
