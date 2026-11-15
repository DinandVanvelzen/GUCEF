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

#ifndef GUCEF_GEOMETRY_TIMEOFIMPACT_H
#define GUCEF_GEOMETRY_TIMEOFIMPACT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CONVEXQUERIES_H
#include "gucefGEOMETRY_ConvexQueries.h"
#define GUCEF_GEOMETRY_CONVEXQUERIES_H
#endif /* GUCEF_GEOMETRY_CONVEXQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CTMOTION3D_H
#include "gucefGEOMETRY_CTMotion3D.h"
#define GUCEF_GEOMETRY_CTMOTION3D_H
#endif /* GUCEF_GEOMETRY_CTMOTION3D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The outcome of a time of impact query */
enum ETimeOfImpactState
{
    TOISTATE_SEPARATED = 0 ,    /**< the shapes stay further apart than the target distance during the whole interval */
    TOISTATE_HIT           ,    /**< they come within the target distance at 'time' */
    TOISTATE_OVERLAPPING   ,    /**< they are within the target distance already at time 0 */
    TOISTATE_NOT_CONVERGED      /**< the advancement stopped at 'time' (iteration cap, or no more progress), which is still safe: no contact before it */
};
typedef enum ETimeOfImpactState TTimeOfImpactState;

/*-------------------------------------------------------------------------*/

/**
 *  The result of TimeOfImpact: the state, the time (fraction of the interval, in [0, 1]) up to which the shapes stay
 *  apart, and at that time the distance of the shapes, their nearest points and the unit normal from A towards B
 *  (the contact normal of a hit). iterations counts the GJK queries.
 */
template < typename T >
struct SToiResult
{
    TTimeOfImpactState state;
    MATH::TMathStatus status;
    T time;
    T distance;
    MATH::CTVector3D< T > normal;
    MATH::CTPoint3D< T > pointOnA;
    MATH::CTPoint3D< T > pointOnB;
    UInt32 iterations;

    SToiResult( void )
        : state( TOISTATE_SEPARATED ), status( MATH::MATHSTATUS_OK ), time( MATH::CTNumericTraits< T >::Zero() ), distance( MATH::CTNumericTraits< T >::Zero() )
        , normal(), pointOnA(), pointOnB(), iterations( 0 )
    {
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The iteration cap of TimeOfImpact: GJK queries per call */
enum { TIMEOFIMPACT_MAX_ITERATIONS = 256 };

/*-------------------------------------------------------------------------*/

/** The largest distance from the origin of the local frame to a point of the whole shape (bounds corner, conservative) */
template < typename TSupport >
GUCEF_GEOMETRY_PRECISE_INLINE typename TSupport::TValue
GetSupportRadius( const TSupport& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TSupport::TValue T;
    typedef MATH::CTNumericTraits< T > TTraits;
    const CTAxisAlignedBox3D< T > bounds = GetSupportBounds( support );
    const MATH::CTVector3D< T > corner( TTraits::Max( TTraits::Abs( bounds.min.x ), TTraits::Abs( bounds.max.x ) ),
                                        TTraits::Max( TTraits::Abs( bounds.min.y ), TTraits::Abs( bounds.max.y ) ),
                                        TTraits::Max( TTraits::Abs( bounds.min.z ), TTraits::Abs( bounds.max.z ) ) );
    return corner.GetLength();
}

/*-------------------------------------------------------------------------*/

/**
 *  An upper bound of the distance from the origin of the local frame to a point of the core (the shape without its
 *  margin): the radius that bounds how fast a point of the core moves when the shape rotates about that origin. The
 *  margin does not count, since a ball is invariant under rotation: the gap of the whole shapes changes exactly as the
 *  gap of the cores. Exact (up to rounding) for points, segments, boxes, triangles and hulls (the longest vertex);
 *  cylinders and cones use cap centre plus radius; other support mappings the corner of their core bounds.
 */
template < typename TSupport >
GUCEF_GEOMETRY_PRECISE_INLINE typename TSupport::TValue
GetSupportCoreRadius( const TSupport& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TSupport::TValue T;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    const T one = TTraits::One();
    const T zero = TTraits::Zero();
    const TVector corner( TTraits::Max( TTraits::Abs( support.GetSupport( TVector( one, zero, zero ) ).x ), TTraits::Abs( support.GetSupport( TVector( -one, zero, zero ) ).x ) ),
                          TTraits::Max( TTraits::Abs( support.GetSupport( TVector( zero, one, zero ) ).y ), TTraits::Abs( support.GetSupport( TVector( zero, -one, zero ) ).y ) ),
                          TTraits::Max( TTraits::Abs( support.GetSupport( TVector( zero, zero, one ) ).z ), TTraits::Abs( support.GetSupport( TVector( zero, zero, -one ) ).z ) ) );
    return corner.GetLength();
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
GetSupportCoreRadius( const CTPointSupport< T >& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return support.point.ToPositionVector().GetLength();
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
GetSupportCoreRadius( const CTSegmentSupport< T >& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return MATH::CTNumericTraits< T >::Max( support.start.ToPositionVector().GetLength(), support.end.ToPositionVector().GetLength() );
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
GetSupportCoreRadius( const CTCylinderSupport< T >& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return MATH::CTNumericTraits< T >::Max( support.start.ToPositionVector().GetLength(), support.end.ToPositionVector().GetLength() ) + MATH::CTNumericTraits< T >::Abs( support.radius );
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
GetSupportCoreRadius( const CTConeSupport< T >& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return MATH::CTNumericTraits< T >::Max( support.apex.ToPositionVector().GetLength(), support.base.ToPositionVector().GetLength() + MATH::CTNumericTraits< T >::Abs( support.baseRadius ) );
}

template < typename TCore >
GUCEF_GEOMETRY_PRECISE_INLINE typename TCore::TValue
GetSupportCoreRadius( const CTMarginSupport< TCore >& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return GetSupportCoreRadius( support.core );
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
GetSupportCoreRadius( const CTShapeSupport< CTOrientedBox3D< T > >& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    MATH::CTPoint3D< T > corners[ 8 ];
    support.GetShape().GetCorners( corners );
    T radius = MATH::CTNumericTraits< T >::Zero();
    for ( UInt32 i=0; i<8; ++i )
        radius = MATH::CTNumericTraits< T >::Max( radius, corners[ i ].ToPositionVector().GetLength() );
    return radius;
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
GetSupportCoreRadius( const CTShapeSupport< CTTriangle3D< T > >& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const CTTriangle3D< T >& triangle = support.GetShape();
    return MATH::CTNumericTraits< T >::Max( MATH::CTNumericTraits< T >::Max( triangle.a.ToPositionVector().GetLength(), triangle.b.ToPositionVector().GetLength() ),
                                            triangle.c.ToPositionVector().GetLength() );
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
GetSupportCoreRadius( const CTShapeSupport< CTConvexHull3D< T > >& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const CTConvexHull3D< T >& hull = support.GetShape();
    T radius = MATH::CTNumericTraits< T >::Zero();
    for ( UInt32 i=0; i<hull.GetVertexCount(); ++i )
        radius = MATH::CTNumericTraits< T >::Max( radius, hull.GetVertex( i ).ToPositionVector().GetLength() );
    return radius;
}

/*-------------------------------------------------------------------------*/

/**
 *  Time of impact of two moving support mappings given in their body frames (conservative advancement, Mirtich): GJK
 *  gives the distance d and the normal n at time t; no point of A can approach B along n faster than
 *  ( vA - vB ) . n + |wA| rA + |wB| rB (rA, rB the core radii around the body positions, see GetSupportCoreRadius), so
 *  t may advance by ( d - gap / 2 ) / that bound without contact. The gap is the target distance, at least the
 *  minimum gap ( rA + rB ) / 2^20 (a target of 0 would otherwise only be approached, never reached); the impact is
 *  reported when d <= gap. When GJK stops at its cap its distance is an upper bound: the advance then uses its lower
 *  bound (SGjkResult::distanceLowerBound), so the time stays safe. When the time can no longer advance (the advance
 *  rounds away) or the iteration cap is reached the state is TOISTATE_NOT_CONVERGED at the last safe time.
 *  Returns the state (also in result.state).
 */
template < typename TSA, typename TSB >
GUCEF_GEOMETRY_PRECISE_INLINE TTimeOfImpactState
SupportTimeOfImpact( const TSA& a, const CTMotion3D< typename TSA::TValue >& motionA, const TSB& b, const CTMotion3D< typename TSA::TValue >& motionB,
                     const typename TSA::TValue& targetDistance, SToiResult< typename TSA::TValue >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TSA::TValue T;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    result = SToiResult< T >();
    const T radiusA = GetSupportCoreRadius( a );
    const T radiusB = GetSupportCoreRadius( b );
    const T minimumGap = ( radiusA + radiusB ) / TTraits::FromInt32( 1048576 );
    const T target = TTraits::Max( TTraits::IsNegative( targetDistance ) ? TTraits::Zero() : targetDistance, minimumGap );
    const T step = target * TTraits::Half();
    const T rotationBound = motionA.angularVelocity.GetLength() * radiusA + motionB.angularVelocity.GetLength() * radiusB;
    const TVector relativeVelocity = motionA.linearVelocity - motionB.linearVelocity;
    const CTGeometryTolerance< T > tolerance;
    SGjkCache< T > cache;

    T t = TTraits::Zero();
    for ( UInt32 iteration=0; ; ++iteration )
    {
        if ( iteration >= (UInt32) TIMEOFIMPACT_MAX_ITERATIONS )
        {
            result.state = TOISTATE_NOT_CONVERGED;
            result.status |= MATH::MATHSTATUS_NOT_CONVERGED;
            return result.state;
        }
        const CTTransformedSupport< TSA > placedA( a, motionA.GetRotation( t ), motionA.GetPosition( t ) );
        const CTTransformedSupport< TSB > placedB( b, motionB.GetRotation( t ), motionB.GetPosition( t ) );
        SGjkResult< T > gjk;
        const MATH::TMathStatus gjkStatus = CTGjk< T >::ComputeDistance( placedA, placedB, tolerance, &cache, gjk );
        result.status |= gjkStatus;
        result.iterations = iteration + 1;
        result.time = t;
        result.distance = gjk.distance;
        result.normal = gjk.normal;
        result.pointOnA = gjk.pointOnA;
        result.pointOnB = gjk.pointOnB;

        if ( gjk.overlapping || !( target < gjk.distance ) )
        {
            result.state = 0 == iteration ? TOISTATE_OVERLAPPING : TOISTATE_HIT;
            return result.state;
        }
        const T approach = relativeVelocity.Dot( gjk.normal ) + rotationBound;
        if ( !( TTraits::Zero() < approach ) )
        {
            result.state = TOISTATE_SEPARATED;
            return result.state;
        }
        /* a GJK stopped at its cap reports an upper bound of the distance: advance by its lower bound */
        const T safeDistance = 0 != ( gjkStatus & MATH::MATHSTATUS_NOT_CONVERGED ) ? gjk.distanceLowerBound : gjk.distance;
        const T advance = ( safeDistance - step ) / approach;
        if ( TTraits::One() - t < advance )
        {
            result.state = TOISTATE_SEPARATED;
            return result.state;
        }
        const T next = t + advance;
        if ( !( t < next ) )
        {
            /* no progress (the lower bound is within the gap, or the advance rounds away): t is the last safe time */
            result.state = TOISTATE_NOT_CONVERGED;
            result.status |= MATH::MATHSTATUS_NOT_CONVERGED;
            return result.state;
        }
        t = next;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Time of impact of two bounded convex shapes given in their body frames, moving over one interval (section 7.5.4 of
 *  the design): the first time in [0, 1] at which their distance falls to 'tolerance' (the target separation, for
 *  example the linear slop of a physics world; > 0 keeps the shapes apart at the reported time). Rotation is
 *  handled through the angular bound of CTMotion3D, so thin rotating shapes do not tunnel. The state, time, normal
 *  and nearest points are in 'result'. Shapes without points never hit.
 *
 *  Returns true when the motion must stop at result.time: TOISTATE_HIT, TOISTATE_OVERLAPPING and
 *  TOISTATE_NOT_CONVERGED (the advancement stopped at the iteration cap TIMEOFIMPACT_MAX_ITERATIONS or without
 *  progress: no contact before result.time, but a contact after it is not ruled out). False only for
 *  TOISTATE_SEPARATED (the whole interval is free) and for shapes without points.
 */
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE bool
TimeOfImpact( const TA& a, const CTMotion3D< typename TA::TValue >& motionA, const TB& b, const CTMotion3D< typename TA::TValue >& motionB,
              const typename TA::TValue& tolerance, SToiResult< typename TA::TValue >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsQueryableConvex( a ) || !IsQueryableConvex( b ) )
    {
        result = SToiResult< typename TA::TValue >();
        return false;
    }
    const TTimeOfImpactState state = SupportTimeOfImpact( CTSupportMapping< TA >::Make( a ), motionA, CTSupportMapping< TB >::Make( b ), motionB, tolerance, result );
    return TOISTATE_SEPARATED != state;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32, Float64 and TFixed32_32 the module's instantiations (gucefGEOMETRY_ConvexQueryInstantiations.cpp) are used */

#define GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, A, B ) \
    prefix bool TimeOfImpact< A< kind >, B< kind > >( const A< kind >&, const CTMotion3D< kind >&, const B< kind >&, const CTMotion3D< kind >&, const kind&, SToiResult< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_TIMEOFIMPACT_KIND( prefix, kind ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, CTSphere, CTSphere ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, CTSphere, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, CTSphere, CTConvexHull3D ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, CTCapsule, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, CTCapsule, CTConvexHull3D ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, CTOrientedBox3D, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, CTOrientedBox3D, CTConvexHull3D ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, CTCylinder, CTOrientedBox3D ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_PAIR( prefix, kind, CTConvexHull3D, CTConvexHull3D )

#define GUCEF_GEOMETRY_TIMEOFIMPACT_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_KIND( prefix, Float64 ) \
    GUCEF_GEOMETRY_TIMEOFIMPACT_KIND( prefix, MATH::TFixed32_32 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_TIMEOFIMPACT_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TIMEOFIMPACT_H ? */
