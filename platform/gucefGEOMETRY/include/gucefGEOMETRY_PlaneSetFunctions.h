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

#ifndef GUCEF_GEOMETRY_PLANESETFUNCTIONS_H
#define GUCEF_GEOMETRY_PLANESETFUNCTIONS_H

/*
 *  Building blocks of the convex regions that are bounded by a set of planes (the frustum and the plane-bounded
 *  volume, section 7.2 of the design). Every plane of a set has its normal pointing into the region: a point is inside
 *  when its signed distance to every plane is not negative.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTPLANE_H
#include "gucefGEOMETRY_CTPlane.h"
#define GUCEF_GEOMETRY_CTPLANE_H
#endif /* GUCEF_GEOMETRY_CTPLANE_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#include "gucefGEOMETRY_CTGeometryTolerance.h"
#define GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

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
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  True when the point is not behind any of the 'count' planes (planes[ i ] has its normal into the region), within the
 *  tolerance: a point up to the length tolerance behind a plane (for the magnitude of its coordinates) still counts.
 *  A point that is not finite is never inside.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
PlaneSetContains( const CTPlane< T >* planes, UInt32 count, const MATH::CTPoint3D< T >& point, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( !point.IsFinite() )
        return false;
    const T scale = TTraits::Max( TTraits::Max( TTraits::Abs( point.x ), TTraits::Abs( point.y ) ), TTraits::Abs( point.z ) );
    const T slack = tolerance.GetLengthTolerance( scale );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !( MATH::ComponentNegate( slack ) <= planes[ i ].GetSignedDistance( point ) ) )
            return false;                   /* behind the plane, or not a number */
    }
    return true;
}

/** The exact test: PlaneSetContains with a tolerance of zero */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
PlaneSetContains( const CTPlane< T >* planes, UInt32 count, const MATH::CTPoint3D< T >& point ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return PlaneSetContains( planes, count, point, CTGeometryTolerance< T >( MATH::CTNumericTraits< T >::Zero(), MATH::CTNumericTraits< T >::Zero(), MATH::CTNumericTraits< T >::Zero() ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Cyrus-Beck clipping of the line p( t ) = origin + t * direction (direction of unit length) to the region, for t in
 *  [ tMin, tMax ]. On success tEnter and tExit give the clipped interval (tEnter is tMin when the start lies inside)
 *  and enterPlane the index of the plane that is crossed at tEnter, or -1 when the start is inside. A line parallel to a
 *  plane is inside that plane's half space exactly when its origin is not behind it, so lines in the boundary plane
 *  are accepted and no division by zero happens. False when nothing of the line is inside, or when the origin, the direction or a plane is not finite.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClipLineToPlaneSet( const CTPlane< T >* planes, UInt32 count, const MATH::CTPoint3D< T >& origin,
                    const MATH::CTVector3D< T >& direction, const T& tMin, const T& tMax, T& tEnter, T& tExit, Int32& enterPlane ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( !origin.IsFinite() || !direction.IsFinite() )
        return false;                       /* also for a region without planes: a ray that is not a ray hits nothing */
    T enter = tMin;
    T leave = tMax;
    Int32 plane = -1;
    for ( UInt32 i=0; i<count; ++i )
    {
        const T distance = planes[ i ].GetSignedDistance( origin );
        const T denominator = planes[ i ].normal.Dot( direction );
        if ( !TTraits::IsFinite( distance ) || !TTraits::IsFinite( denominator ) )
            return false;                   /* an origin, direction or plane that is not a number hits nothing */
        if ( TTraits::Zero() == denominator )
        {
            if ( distance < TTraits::Zero() )
                return false;
            continue;
        }
        const T t = ( TTraits::Zero() - distance ) / denominator;
        if ( !( t == t ) )
            return false;
        if ( TTraits::Zero() < denominator )
        {
            /* moving to the inside of this plane: an entry */
            if ( enter < t )
            {
                enter = t;
                plane = (Int32) i;
            }
        }
        else if ( t < leave )
        {
            leave = t;
        }
        if ( leave < enter )
            return false;
    }
    if ( !( enter <= leave ) )
        return false;                       /* NaN */
    tEnter = enter;
    tExit = leave;
    enterPlane = plane;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The ray cast of a region bounded by planes: a ray that starts inside hits at distance 0 with RAYHIT_STARTED_INSIDE
 *  and the normal opposite to the ray; otherwise the hit is where the ray enters, with the outward normal of the plane
 *  crossed there and its index as the feature index.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
RayCastPlaneSet( const CTPlane< T >* planes, UInt32 count, const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    const MATH::CTVector3D< T > direction = ray.direction.AsVector();
    T enter = TTraits::Zero();
    T leave = TTraits::Zero();
    Int32 plane = -1;
    if ( !ClipLineToPlaneSet( planes, count, ray.origin, direction, TTraits::Zero(), maxDistance, enter, leave, plane ) )
        return false;
    SRayCastResult< T > hit;
    if ( plane < 0 )
    {
        hit.distance = TTraits::Zero();
        hit.point = ray.origin;
        hit.normal = -direction;
        hit.flags = RAYHIT_STARTED_INSIDE;
    }
    else
    {
        hit.distance = enter;
        hit.point = ray.origin + direction * enter;
        hit.normal = -planes[ plane ].normal.AsVector();
        hit.featureIndex = plane;
    }
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Sutherland-Hodgman clipping of a polygon ( 'count' points ) to the region (against every plane in order). 'out' and
 *  'scratch' must each have room for 'capacity' points; a convex polygon needs at most count + planeCount. The result
 *  ends in 'out'. False (outCount 0) when the capacity is exceeded or the input does not fit; true with outCount 0
 *  when nothing is left. A polygon on a boundary plane is kept. Allocation-free.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClipPolygonToPlaneSet( const CTPlane< T >* planes, UInt32 planeCount, const MATH::CTPoint3D< T >* points, UInt32 count,
                       MATH::CTPoint3D< T >* out, MATH::CTPoint3D< T >* scratch, UInt32 capacity, UInt32& outCount ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    outCount = 0;
    if ( count > capacity || ( count > 0 && GUCEF_NULL == points ) )
        return false;
    /* the buffers alternate between the input and the output of a plane; the last result must end in 'out' */
    TPoint* buffers[ 2 ] = { scratch, out };
    UInt32 current = 0;
    for ( UInt32 i=0; i<count; ++i )
        buffers[ 0 ][ i ] = points[ i ];
    UInt32 n = count;
    for ( UInt32 p=0; p<planeCount && n>0; ++p )
    {
        const CTPlane< T >& plane = planes[ p ];
        const TPoint* in = buffers[ current ];
        TPoint* result = buffers[ 1 - current ];
        UInt32 written = 0;
        TPoint previous = in[ n - 1 ];
        T previousDistance = plane.GetSignedDistance( previous );
        if ( !( previousDistance == previousDistance ) )
            return false;
        for ( UInt32 i=0; i<n; ++i )
        {
            const TPoint& currentPoint = in[ i ];
            const T currentDistance = plane.GetSignedDistance( currentPoint );
            if ( !( currentDistance == currentDistance ) )
                return false;               /* a point that is not a number: nothing can be said, the output stays empty */
            const bool currentInside = !( currentDistance < TTraits::Zero() );
            /* only an edge that strictly crosses the plane gets a new point: a vertex on the plane is its own crossing */
            if ( ( previousDistance < TTraits::Zero() && TTraits::Zero() < currentDistance ) || ( TTraits::Zero() < previousDistance && currentDistance < TTraits::Zero() ) )
            {
                if ( written >= capacity )
                    return false;
                const T t = previousDistance / ( previousDistance - currentDistance );
                result[ written++ ] = previous + ( currentPoint - previous ) * t;
            }
            if ( currentInside )
            {
                if ( written >= capacity )
                    return false;
                result[ written++ ] = currentPoint;
            }
            previous = currentPoint;
            previousDistance = currentDistance;
        }
        n = written;
        current = 1 - current;
    }
    if ( 1 != current )
    {
        for ( UInt32 i=0; i<n; ++i )
            out[ i ] = buffers[ current ][ i ];
    }
    outCount = n;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The classification of a ball (centre and radius) against the planes: OUTSIDE when it lies entirely behind one plane,
 *  INSIDE when it is entirely in front of all, INTERSECTING otherwise. The test is per plane, so a ball near an edge or
 *  corner of the region can be reported as INTERSECTING although it touches nothing: the result is conservative
 *  (never OUTSIDE for a ball that overlaps the region), which is the right answer for culling. The tolerance widens the
 *  region: a ball up to the length tolerance behind a plane is not OUTSIDE and one up to the tolerance short of being in
 *  front counts as INSIDE. A ball that is not finite is OUTSIDE.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
ClassifySphereToPlaneSet( const CTPlane< T >* planes, UInt32 count, const MATH::CTPoint3D< T >& centre, const T& radius, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( !centre.IsFinite() || !TTraits::IsFinite( radius ) )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    const T slack = tolerance.GetLengthTolerance( TTraits::Max( TTraits::Max( TTraits::Abs( centre.x ), TTraits::Abs( centre.y ) ), TTraits::Max( TTraits::Abs( centre.z ), radius ) ) );
    bool straddles = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        const T distance = planes[ i ].GetSignedDistance( centre );
        if ( !TTraits::IsFinite( distance ) )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < MATH::ComponentNegate( radius ) - slack )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < radius - slack )
            straddles = true;
    }
    return straddles ? FRUSTUMCLASSIFICATION_INTERSECTING : FRUSTUMCLASSIFICATION_INSIDE;
}

/** The exact classification: a tolerance of zero */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE TFrustumClassification
ClassifySphereToPlaneSet( const CTPlane< T >* planes, UInt32 count, const MATH::CTPoint3D< T >& centre, const T& radius ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const T zero = MATH::CTNumericTraits< T >::Zero();
    return ClassifySphereToPlaneSet( planes, count, centre, radius, CTGeometryTolerance< T >( zero, zero, zero ) );
}

/*-------------------------------------------------------------------------*/

/** As ClassifySphereToPlaneSet for the axis aligned box with the given centre and half extents (not negative) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
ClassifyBoxToPlaneSet( const CTPlane< T >* planes, UInt32 count, const MATH::CTPoint3D< T >& centre, const MATH::CTVector3D< T >& halfExtents, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( !centre.IsFinite() || !halfExtents.IsFinite() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    const T slack = tolerance.GetLengthTolerance( TTraits::Max( TTraits::Max( TTraits::Abs( centre.x ), TTraits::Abs( centre.y ) ), TTraits::Max( TTraits::Abs( centre.z ), halfExtents.GetChebyshevLength() ) ) );
    bool straddles = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        const MATH::CTVector3D< T > n = planes[ i ].normal.AsVector();
        const T radius = ( TTraits::Abs( n.x ) * halfExtents.x + TTraits::Abs( n.y ) * halfExtents.y ) + TTraits::Abs( n.z ) * halfExtents.z;
        const T distance = planes[ i ].GetSignedDistance( centre );
        if ( !TTraits::IsFinite( distance ) || !TTraits::IsFinite( radius ) )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < MATH::ComponentNegate( radius ) - slack )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < radius - slack )
            straddles = true;
    }
    return straddles ? FRUSTUMCLASSIFICATION_INTERSECTING : FRUSTUMCLASSIFICATION_INSIDE;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE TFrustumClassification
ClassifyBoxToPlaneSet( const CTPlane< T >* planes, UInt32 count, const MATH::CTPoint3D< T >& centre, const MATH::CTVector3D< T >& halfExtents ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const T zero = MATH::CTNumericTraits< T >::Zero();
    return ClassifyBoxToPlaneSet( planes, count, centre, halfExtents, CTGeometryTolerance< T >( zero, zero, zero ) );
}

/*-------------------------------------------------------------------------*/

/** As ClassifySphereToPlaneSet for the oriented box with the given centre, unit axes and half extents (not negative) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
ClassifyOrientedBoxToPlaneSet( const CTPlane< T >* planes, UInt32 count, const MATH::CTPoint3D< T >& centre,
                               const MATH::CTVector3D< T > ( &axes )[ 3 ], const MATH::CTVector3D< T >& halfExtents, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( !centre.IsFinite() || !halfExtents.IsFinite() || !axes[ 0 ].IsFinite() || !axes[ 1 ].IsFinite() || !axes[ 2 ].IsFinite() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    const T slack = tolerance.GetLengthTolerance( TTraits::Max( TTraits::Max( TTraits::Abs( centre.x ), TTraits::Abs( centre.y ) ), TTraits::Max( TTraits::Abs( centre.z ), halfExtents.GetChebyshevLength() ) ) );
    bool straddles = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        const MATH::CTVector3D< T > n = planes[ i ].normal.AsVector();
        const T radius = ( TTraits::Abs( n.Dot( axes[ 0 ] ) ) * halfExtents.x + TTraits::Abs( n.Dot( axes[ 1 ] ) ) * halfExtents.y ) +
                         TTraits::Abs( n.Dot( axes[ 2 ] ) ) * halfExtents.z;
        const T distance = planes[ i ].GetSignedDistance( centre );
        if ( !TTraits::IsFinite( distance ) || !TTraits::IsFinite( radius ) )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < MATH::ComponentNegate( radius ) - slack )
            return FRUSTUMCLASSIFICATION_OUTSIDE;
        if ( distance < radius - slack )
            straddles = true;
    }
    return straddles ? FRUSTUMCLASSIFICATION_INTERSECTING : FRUSTUMCLASSIFICATION_INSIDE;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE TFrustumClassification
ClassifyOrientedBoxToPlaneSet( const CTPlane< T >* planes, UInt32 count, const MATH::CTPoint3D< T >& centre,
                               const MATH::CTVector3D< T > ( &axes )[ 3 ], const MATH::CTVector3D< T >& halfExtents ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const T zero = MATH::CTNumericTraits< T >::Zero();
    return ClassifyOrientedBoxToPlaneSet( planes, count, centre, axes, halfExtents, CTGeometryTolerance< T >( zero, zero, zero ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_PLANESETFUNCTIONS_KIND( prefix, kind ) \
    prefix bool PlaneSetContains< kind >( const CTPlane< kind >*, UInt32, const MATH::CTPoint3D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClipLineToPlaneSet< kind >( const CTPlane< kind >*, UInt32, const MATH::CTPoint3D< kind >&, const MATH::CTVector3D< kind >&, const kind&, const kind&, kind&, kind&, Int32& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClipPolygonToPlaneSet< kind >( const CTPlane< kind >*, UInt32, const MATH::CTPoint3D< kind >*, UInt32, MATH::CTPoint3D< kind >*, MATH::CTPoint3D< kind >*, UInt32, UInt32& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool RayCastPlaneSet< kind >( const CTPlane< kind >*, UInt32, const CTRay3D< kind >&, const kind&, SRayCastResult< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix TFrustumClassification ClassifySphereToPlaneSet< kind >( const CTPlane< kind >*, UInt32, const MATH::CTPoint3D< kind >&, const kind&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix TFrustumClassification ClassifyBoxToPlaneSet< kind >( const CTPlane< kind >*, UInt32, const MATH::CTPoint3D< kind >&, const MATH::CTVector3D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix TFrustumClassification ClassifyOrientedBoxToPlaneSet< kind >( const CTPlane< kind >*, UInt32, const MATH::CTPoint3D< kind >&, const MATH::CTVector3D< kind > ( & )[ 3 ], const MATH::CTVector3D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_PLANESETFUNCTIONS_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_PLANESETFUNCTIONS_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_PLANESETFUNCTIONS_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_PLANESETFUNCTIONS_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_PLANESETFUNCTIONS_H ? */
