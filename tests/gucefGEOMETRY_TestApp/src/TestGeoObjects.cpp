/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestGeoSpaceReference.h"
#include "TestGeoObjects.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;
using namespace QueryReference;
using namespace ConvexReference;
using namespace GeoSpaceReference;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A user shape (GEOSHAPE_UNKNOWN): an axis aligned cube of half size 0.5 through the virtual primitives only */
template < typename T >
class CTUserCube : public CTGeoObjectBase< T >
{
    public:

    typedef CTGeoObjectBase< T > TBase;
    typedef CTPoint3D< T > TPoint;
    typedef CTVector3D< T > TVector;

    CTUserCube( void ) : TBase(), m_box( TPoint( T( -0.5 ), T( -0.5 ), T( -0.5 ) ), TPoint( T( 0.5 ), T( 0.5 ), T( 0.5 ) ) ) {}

    virtual TGeoShapeType GetShapeType( void ) const GUCEF_VIRTUAL_OVERRIDE { return GEOSHAPE_UNKNOWN; }
    virtual void GetLocalBounds( CTAxisAlignedBox3D< T >& bounds ) const GUCEF_VIRTUAL_OVERRIDE { bounds = m_box; }
    virtual TPoint GetLocalSupport( const TVector& d ) const GUCEF_VIRTUAL_OVERRIDE { return m_box.GetSupport( d ); }
    virtual bool LocalRayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& hit ) const GUCEF_VIRTUAL_OVERRIDE { return m_box.RayCast( ray, maxDistance, hit ); }
    virtual TPoint GetLocalClosestPoint( const TPoint& p ) const GUCEF_VIRTUAL_OVERRIDE { return m_box.ClosestPoint( p ); }
    virtual bool LocalContains( const TPoint& p ) const GUCEF_VIRTUAL_OVERRIDE { return m_box.Contains( p ); }
    virtual bool IsConvex( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    private:

    CTAxisAlignedBox3D< T > m_box;
};

/*-------------------------------------------------------------------------*/

enum ETestObjectKind
{
    OBJ_SPHERE = 0, OBJ_AABB, OBJ_OBB, OBJ_CAPSULE, OBJ_CYLINDER, OBJ_CONE, OBJ_TRIANGLE, OBJ_HULL, OBJ_PBV, OBJ_CIRCLE, OBJ_RECTANGLE,
    OBJ_ORIENTED_RECTANGLE, OBJ_TRIANGLE_2D, OBJ_POLYGON_CONVEX, OBJ_POLYGON_CONCAVE, OBJ_POLYGON_HOLES, OBJ_POLYLINE, OBJ_PLANE, OBJ_PLANE_BOUNDED,
    OBJ_USER, OBJ_COUNT
};

static const char* const ObjectKindNames[ OBJ_COUNT ] =
{
    "sphere", "aabb", "obb", "capsule", "cylinder", "cone", "triangle", "hull", "plane-bounded volume", "circle", "rectangle", "oriented rectangle",
    "triangle 2D", "convex polygon", "concave polygon", "polygon with holes", "polyline", "plane", "bounded plane", "user cube"
};

/** A linked object of each kind near the local origin with a size of about 1 */
template < typename T >
static CTGeoObjectBase< T >*
MakeObject( UInt32 kind, CRandom& r )
{
    typedef CTPoint3D< T > TP;
    typedef CTPoint2D< T > TP2;
    typedef CTVector2D< T > TV2;
    /* every draw into a local first (the order of evaluation of arguments differs between compilers) */
    const P3 p0 = r.Point( 0.9 );
    const P3 p1 = r.Point( 0.9 );
    const P3 p2 = r.Point( 0.9 );
    const Q4 q0 = r.Rotation();
    const Float64 u0 = r.Uniform( 0.3, 1.0 );
    switch ( kind )
    {
        case OBJ_SPHERE: return new CTGeoObject< CTSphere< T > >( CTSphere< T >( ToP< T >( P3( p0.ToPositionVector() * 0.33 ) ), T( u0 ) ) );
        case OBJ_AABB: return new CTGeoObject< CTAxisAlignedBox3D< T > >( CTAxisAlignedBox3D< T >( TP( T( -0.6 ), T( -0.4 ), T( -0.5 ) ), TP( T( 0.5 ), T( 0.7 ), T( 0.3 ) ) ) );
        case OBJ_OBB: return new CTGeoObject< CTOrientedBox3D< T > >( CTOrientedBox3D< T >( ToP< T >( P3( p0.ToPositionVector() * 0.33 ) ), ToQ< T >( q0 ), CTVector3D< T >( T( 0.5 ), T( 0.3 ), T( 0.7 ) ) ) );
        case OBJ_CAPSULE: return new CTGeoObject< CTCapsule< T > >( CTCapsule< T >( ToP< T >( P3( p0.ToPositionVector() * 0.66 ) ), ToP< T >( P3( p1.ToPositionVector() * 0.66 ) ), T( u0 * 0.5 ) ) );
        case OBJ_CYLINDER: return new CTGeoObject< CTCylinder< T > >( CTCylinder< T >( TP( T( 0 ), T( -0.6 ), T( 0.1 ) ), TP( T( 0.2 ), T( 0.6 ), T( 0 ) ), T( 0.4 ) ) );
        case OBJ_CONE: return new CTGeoObject< CTCone< T > >( CTCone< T >( TP( T( 0 ), T( 0.7 ), T( 0 ) ), ToD< T >( D3( V3( 0.1, -1.0, 0.2 ) * ( 1.0 / V3( 0.1, -1.0, 0.2 ).GetLength() ) ) ), T( 1.3 ), T( 0.5 ) ) );
        case OBJ_TRIANGLE: return new CTGeoObject< CTTriangle3D< T > >( CTTriangle3D< T >( ToP< T >( p0 ), ToP< T >( p1 ), ToP< T >( p2 ) ) );
        case OBJ_HULL:
        {
            CTPoint3D< T > points[ 10 ];
            for ( UInt32 i=0; i<10; ++i )
                points[ i ] = ToP< T >( r.Point( 0.7 ) );
            return new CTGeoObject< CTConvexHull3D< T > >( CTConvexHull3D< T >( points, 10 ) );
        }
        case OBJ_PBV: return new CTGeoObject< CTPlaneBoundedVolume< T > >( CTPlaneBoundedVolume< T >::FromOrientedBox( CTOrientedBox3D< T >( TP(), ToQ< T >( q0 ), CTVector3D< T >( T( 0.4 ), T( 0.6 ), T( 0.5 ) ) ) ) );
        case OBJ_CIRCLE: return new CTGeoObject2D< CTCircle< T > >( CTCircle< T >( TP2( T( 0.1 ), T( -0.2 ) ), T( 0.8 ) ) );
        case OBJ_RECTANGLE: return new CTGeoObject2D< CTRectangle< T > >( CTRectangle< T >( TP2( T( -0.7 ), T( -0.4 ) ), TP2( T( 0.6 ), T( 0.5 ) ) ) );
        case OBJ_ORIENTED_RECTANGLE: return new CTGeoObject2D< CTOrientedRectangle< T > >( CTOrientedRectangle< T >( TP2(), CTRadian< T >( T( 0.6 ) ), TV2( T( 0.7 ), T( 0.3 ) ) ) );
        case OBJ_TRIANGLE_2D: return new CTGeoObject2D< CTTriangle2D< T > >( CTTriangle2D< T >( TP2( T( -0.8 ), T( -0.5 ) ), TP2( T( 0.9 ), T( -0.4 ) ), TP2( T( 0.1 ), T( 0.8 ) ) ) );
        case OBJ_POLYGON_CONVEX:
        {
            const TP2 ring[ 5 ] = { TP2( T( -0.6 ), T( -0.6 ) ), TP2( T( 0.6 ), T( -0.6 ) ), TP2( T( 0.9 ), T( 0.1 ) ), TP2( T( 0 ), T( 0.8 ) ), TP2( T( -0.9 ), T( 0.1 ) ) };
            return new CTGeoObject2D< CTPolygon2D< T > >( CTPolygon2D< T >( ring, 5 ) );
        }
        case OBJ_POLYGON_CONCAVE:
        {
            const TP2 ring[ 6 ] = { TP2( T( -0.8 ), T( -0.8 ) ), TP2( T( 0.8 ), T( -0.8 ) ), TP2( T( 0.8 ), T( -0.3 ) ), TP2( T( -0.3 ), T( -0.3 ) ), TP2( T( -0.3 ), T( 0.8 ) ), TP2( T( -0.8 ), T( 0.8 ) ) };
            return new CTGeoObject2D< CTPolygon2D< T > >( CTPolygon2D< T >( ring, 6 ) );
        }
        case OBJ_POLYGON_HOLES:
        {
            const TP2 outer[ 4 ] = { TP2( T( -0.8 ), T( -0.8 ) ), TP2( T( 0.8 ), T( -0.8 ) ), TP2( T( 0.8 ), T( 0.8 ) ), TP2( T( -0.8 ), T( 0.8 ) ) };
            const TP2 hole[ 4 ] = { TP2( T( -0.3 ), T( -0.3 ) ), TP2( T( -0.3 ), T( 0.3 ) ), TP2( T( 0.3 ), T( 0.3 ) ), TP2( T( 0.3 ), T( -0.3 ) ) };
            CTPolygonWithHoles2D< T > shape;
            shape.outer = CTPolygon2D< T >( outer, 4 );
            shape.holes.push_back( CTPolygon2D< T >( hole, 4 ) );
            return new CTGeoObject2D< CTPolygonWithHoles2D< T > >( shape );
        }
        case OBJ_POLYLINE:
        {
            const TP2 line[ 4 ] = { TP2( T( -0.8 ), T( 0 ) ), TP2( T( -0.2 ), T( 0.5 ) ), TP2( T( 0.3 ), T( -0.4 ) ), TP2( T( 0.9 ), T( 0.2 ) ) };
            return new CTGeoObject2D< CTPolyline2D< T > >( CTPolyline2D< T >( line, 4 ) );
        }
        case OBJ_PLANE: return new CTGeoPlane< T >();
        case OBJ_PLANE_BOUNDED:
        {
            CTGeoPlane< T >* plane = new CTGeoPlane< T >();
            plane->SetExtents( CTRectangle< T >( TP2( T( -0.9 ), T( -0.6 ) ), TP2( T( 0.8 ), T( 0.7 ) ) ) );
            return plane;
        }
        default: return new CTUserCube< T >();
    }
}

/*-------------------------------------------------------------------------*/

/** The world sphere of a linked sphere under a similarity, in Float64 from the reference matrices */
template < typename T >
static CTSphere< Float64 >
WorldSphere( const CTGeoObject< CTSphere< T > >& object )
{
    const M4 world = ReferenceWorld( object );
    const CTSphere< T >& s = object.GetShape();
    return CTSphere< Float64 >( world.TransformPoint( Widen( s.centre ) ), (Float64) s.radius * world.GetLinear().GetColumn( 0 ).GetLength() );
}

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 6: linked spheres under uniformly scaled (and mirrored) parents use the analytic path and equal the
 *  value-shape results of the world spheres
 */
template < typename T >
static bool
LinkedSpheresAnalytic( UInt64 seed, UInt32 count, Float64 tolerance )
{
    typedef CTGeoObject< CTSphere< T > > TSphereObject;
    CRandom r( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTGeoSpace< T > root( RandomTransform< T >( r, 3.0, true, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T > parentA( RandomTransform< T >( r, 3.0, true, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T > parentB( RandomTransform< T >( r, 3.0, true, 0.5, 2.0, 0.2 ) );
        parentA.SetParent( &root, false );
        parentB.SetParent( &root, false );
        const P3 centreA = r.Point( 1.0 );
        const Float64 radiusA = r.Uniform( 0.2, 1.5 );
        const CTTransform3D< T > placeA = RandomTransform< T >( r, 3.0, true, 0.5, 2.0, 0.2 );
        TSphereObject a( CTSphere< T >( ToP< T >( centreA ), T( radiusA ) ), placeA );
        const P3 centreB = r.Point( 1.0 );
        const Float64 radiusB = r.Uniform( 0.2, 1.5 );
        const CTTransform3D< T > placeB = RandomTransform< T >( r, 3.0, true, 0.5, 2.0, 0.2 );
        TSphereObject b( CTSphere< T >( ToP< T >( centreB ), T( radiusB ) ), placeB );
        a.SetParent( &parentA, false );
        b.SetParent( &parentB, false );

        CTPoint3D< T > onA;
        CTPoint3D< T > onB;
        T distance = T( 0 );
        TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
        const bool answered = CTGeoObjectQueries< T >::ClosestPoints( a, b, onA, onB, distance, path );
        const CTSphere< Float64 > wa = WorldSphere( a );
        const CTSphere< Float64 > wb = WorldSphere( b );
        P3 pa;
        P3 pb;
        ClosestPoints( wa, wb, pa, pb );
        const Float64 referenceDistance = Distance( wa, wb );
        const Float64 scale = 1.0 + wa.radius + wb.radius + wa.centre.ToPositionVector().GetLength();
        bool ok = answered && GEOPAIRPATH_ANALYTIC == path && fabs( (Float64) distance - referenceDistance ) <= tolerance * scale;
        ok = ok && ( Widen( onA ) - pa ).GetLength() <= tolerance * scale && ( Widen( onB ) - pb ).GetLength() <= tolerance * scale;
        TGeoPairPath intersectsPath = GEOPAIRPATH_NOT_OFFERED;
        const bool intersects = CTGeoObjectQueries< T >::Intersects( a, b, intersectsPath );
        if ( fabs( referenceDistance ) > tolerance * scale * 10.0 || 0.0 == referenceDistance )
            ok = ok && GEOPAIRPATH_ANALYTIC == intersectsPath && intersects == Intersects( wa, wb );
        CTContactManifold< T > manifold;
        TGeoPairPath contactPath = GEOPAIRPATH_NOT_OFFERED;
        const bool touching = CTGeoObjectQueries< T >::ComputeContact( a, b, manifold, contactPath );
        CTContactManifold< Float64 > reference;
        const bool referenceTouching = ComputeContact( wa, wb, reference );
        if ( touching && referenceTouching )
            ok = ok && GEOPAIRPATH_ANALYTIC == contactPath && 1 == manifold.GetPointCount() && fabs( (Float64) manifold.GetPoint( 0 ).depth - reference.GetPoint( 0 ).depth ) <= tolerance * scale * 10.0 &&
                 ( Widen( manifold.normal ) - reference.normal ).GetLength() <= tolerance * scale * 100.0;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: linked spheres case " << i << " path " << (UInt32) path << " distance " << (Float64) distance << " reference " << referenceDistance << ", seed " << seed );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** The world point of a sphere surface parameter ( theta, phi ) under a Float64 placement */
static P3
SurfacePoint( const M4& world, const CTSphere< Float64 >& s, Float64 theta, Float64 phi )
{
    const V3 u( sin( theta ) * cos( phi ), sin( theta ) * sin( phi ), cos( theta ) );
    return world.TransformPoint( s.centre + u * s.radius );
}

/**
 *  The distance of two placed (ellipsoidal) spheres by dense sampling: a 48 x 24 grid on each surface, then a pattern
 *  search on the four surface parameters from the best pair down to steps of 1e-9
 */
static Float64
SampledEllipsoidDistance( const M4& worldA, const CTSphere< Float64 >& a, const M4& worldB, const CTSphere< Float64 >& b )
{
    const Float64 pi = 3.141592653589793;
    const UInt32 nt = 24;
    const UInt32 np = 48;
    GUCEF::vector< P3 > sa;
    GUCEF::vector< P3 > sb;
    for ( UInt32 i=0; i<nt; ++i )
    {
        for ( UInt32 j=0; j<np; ++j )
        {
            const Float64 theta = pi * ( (Float64) i + 0.5 ) / (Float64) nt;
            const Float64 phi = 2.0 * pi * (Float64) j / (Float64) np;
            sa.push_back( SurfacePoint( worldA, a, theta, phi ) );
            sb.push_back( SurfacePoint( worldB, b, theta, phi ) );
        }
    }
    UInt32 bestA = 0;
    UInt32 bestB = 0;
    Float64 best = 1.0e300;
    for ( UInt32 i=0; i<(UInt32) sa.size(); ++i )
    {
        for ( UInt32 j=0; j<(UInt32) sb.size(); ++j )
        {
            const Float64 d = ( sa[ i ] - sb[ j ] ).GetSquaredLength();
            if ( d < best )
            {
                best = d;
                bestA = i;
                bestB = j;
            }
        }
    }
    Float64 params[ 4 ] = { pi * ( (Float64) ( bestA / np ) + 0.5 ) / (Float64) nt, 2.0 * pi * (Float64) ( bestA % np ) / (Float64) np,
                            pi * ( (Float64) ( bestB / np ) + 0.5 ) / (Float64) nt, 2.0 * pi * (Float64) ( bestB % np ) / (Float64) np };
    Float64 step = pi / (Float64) nt;
    best = ( SurfacePoint( worldA, a, params[ 0 ], params[ 1 ] ) - SurfacePoint( worldB, b, params[ 2 ], params[ 3 ] ) ).GetLength();
    while ( step > 1.0e-9 )
    {
        bool improved = false;
        for ( UInt32 k=0; k<4; ++k )
        {
            for ( Int32 sign=-1; sign<=1; sign+=2 )
            {
                Float64 trial[ 4 ] = { params[ 0 ], params[ 1 ], params[ 2 ], params[ 3 ] };
                trial[ k ] += sign * step;
                const Float64 d = ( SurfacePoint( worldA, a, trial[ 0 ], trial[ 1 ] ) - SurfacePoint( worldB, b, trial[ 2 ], trial[ 3 ] ) ).GetLength();
                if ( d < best )
                {
                    best = d;
                    for ( UInt32 m=0; m<4; ++m )
                        params[ m ] = trial[ m ];
                    improved = true;
                }
            }
        }
        if ( !improved )
            step *= 0.5;
    }
    return best;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 6: under non-uniform scale the support (GJK) path agrees with a dense-sampling reference (1e-3 relative) */
template < typename T >
static bool
EllipsoidsAgainstSampling( UInt64 seed, UInt32 count )
{
    typedef CTGeoObject< CTSphere< T > > TSphereObject;
    CRandom r( seed );
    UInt32 bad = 0;
    UInt32 tested = 0;
    Float64 worst = 0.0;
    while ( tested < count )
    {
        CTGeoSpace< T > parent( RandomTransform< T >( r, 1.0, false, 0.5, 2.0, 0.0 ) );
        const P3 centreA = r.Point( 0.5 );
        const Float64 radiusA = r.Uniform( 0.5, 1.0 );
        const CTTransform3D< T > placeA = RandomTransform< T >( r, 4.0, false, 0.5, 2.0, 0.0 );
        TSphereObject a( CTSphere< T >( ToP< T >( centreA ), T( radiusA ) ), placeA );
        const P3 centreB = r.Point( 0.5 );
        const Float64 radiusB = r.Uniform( 0.5, 1.0 );
        const CTTransform3D< T > placeB = RandomTransform< T >( r, 4.0, false, 0.5, 2.0, 0.0 );
        TSphereObject b( CTSphere< T >( ToP< T >( centreB ), T( radiusB ) ), placeB );
        a.SetParent( &parent, false );
        CTPoint3D< T > onA;
        CTPoint3D< T > onB;
        T distance = T( 0 );
        TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
        CTGeoObjectQueries< T >::ClosestPoints( a, b, onA, onB, distance, path );
        if ( distance < T( 0.05 ) )
            continue;
        ++tested;
        const CTSphere< Float64 > sa( Widen( a.GetShape().centre ), (Float64) a.GetShape().radius );
        const CTSphere< Float64 > sb( Widen( b.GetShape().centre ), (Float64) b.GetShape().radius );
        const Float64 reference = SampledEllipsoidDistance( ReferenceWorld( a ), sa, ReferenceWorld( b ), sb );
        const Float64 error = fabs( (Float64) distance - reference ) / reference;
        worst = error > worst ? error : worst;
        if ( GEOPAIRPATH_SUPPORT != path || error > 1.0e-3 )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: ellipsoid case " << tested << " path " << (UInt32) path << " distance " << (Float64) distance << " sampled " << reference << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: " << count << " non-uniformly scaled sphere pairs (kind size " << (UInt32) sizeof( T ) << "), seed " << seed << ": worst relative difference of the GJK path to dense sampling " << worst );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** A placement of an object: similar (uniform scale) or not, under a parent */
template < typename T >
static void
Place( CRandom& r, CTGeoSpace< T >& parent, CTGeoObjectBase< T >& object, bool similar )
{
    parent.SetLocalTransform( RandomTransform< T >( r, 1.5, similar, 0.6, 1.6, 0.2 ) );
    object.SetLocalTransform( RandomTransform< T >( r, 1.5, similar, 0.6, 1.6, 0.2 ) );
    object.SetParent( &parent, false );
}

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 6: every ordered pair of linked object kinds returns a consistent result (nearest points on the
 *  objects at the reported distance, Intersects, ComputeContact and Contains consistent with it) or, where not offered,
 *  the documented response (a result in every build)
 */
template < typename T >
static bool
EveryPairType( UInt64 seed, UInt32 repeats, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    UInt32 offered[ 4 ] = { 0, 0, 0, 0 };
    UInt32 notOffered = 0;
    UInt32 paths[ 5 ] = { 0, 0, 0, 0, 0 };
    for ( UInt32 ka=0; ka<OBJ_COUNT; ++ka )
    {
        for ( UInt32 kb=0; kb<OBJ_COUNT; ++kb )
        {
            for ( UInt32 rep=0; rep<repeats; ++rep )
            {
                CTGeoObjectBase< T >* a = MakeObject< T >( ka, r );
                CTGeoObjectBase< T >* b = MakeObject< T >( kb, r );
                CTGeoSpace< T > parentA;
                CTGeoSpace< T > parentB;
                Place( r, parentA, *a, 0 == rep % 2 );
                Place( r, parentB, *b, 0 == rep % 3 );
                bool ok = true;

                const bool closestOffered = IsGeoPairQueryOffered( *a, *b, GEOPAIRQUERY_CLOSESTPOINTS );
                T distance = T( 0 );
                bool intersects = false;
                if ( closestOffered )
                {
                    ++offered[ 0 ];
                    CTPoint3D< T > onA;
                    CTPoint3D< T > onB;
                    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
                    ok = ok && CTGeoObjectQueries< T >::ClosestPoints( *a, *b, onA, onB, distance, path );
                    ++paths[ path ];
                    const Float64 size = 1.0 + Widen( onA ).ToPositionVector().GetLength();
                    ok = ok && CTNumericTraits< T >::IsFinite( distance ) && !( distance < T( 0 ) ) && fabs( ( Widen( onB ) - Widen( onA ) ).GetLength() - (Float64) distance ) <= tolerance * size;
                    ok = ok && (Float64) a->GetDistance( onA ) <= tolerance * size && (Float64) b->GetDistance( onB ) <= tolerance * size;
                    ok = ok && Distance( *a, *b ) == distance;
                    TGeoPairPath intersectsPath = GEOPAIRPATH_NOT_OFFERED;
                    intersects = CTGeoObjectQueries< T >::Intersects( *a, *b, intersectsPath );
                    if ( (Float64) distance > tolerance * size )
                        ok = ok && !intersects;
                    if ( T( 0 ) == distance )
                        ok = ok && intersects;
                    /* (A, B) and (B, A) agree */
                    CTPoint3D< T > swappedOnA;
                    CTPoint3D< T > swappedOnB;
                    T swappedDistance = T( 0 );
                    TGeoPairPath swappedPath = GEOPAIRPATH_NOT_OFFERED;
                    ok = ok && CTGeoObjectQueries< T >::ClosestPoints( *b, *a, swappedOnB, swappedOnA, swappedDistance, swappedPath );
                    ok = ok && fabs( (Float64) swappedDistance - (Float64) distance ) <= tolerance * size;
                    if ( (Float64) distance > tolerance * size * 10.0 || T( 0 ) == distance )
                        ok = ok && intersects == Intersects( *b, *a );
                    if ( !ok )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: pair " << ObjectKindNames[ ka ] << " - " << ObjectKindNames[ kb ] << " distance " << (Float64) distance << " path " << (UInt32) path
                                         << " intersects " << ( intersects ? 1 : 0 ) << " |onB - onA| " << ( Widen( onB ) - Widen( onA ) ).GetLength() << " on A " << (Float64) a->GetDistance( onA ) << " on B " << (Float64) b->GetDistance( onB ) << ", seed " << seed );
                }
                if ( IsGeoPairQueryOffered( *a, *b, GEOPAIRQUERY_CONTACT ) )
                {
                    ++offered[ 1 ];
                    CTContactManifold< T > manifold;
                    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
                    const bool touching = CTGeoObjectQueries< T >::ComputeContact( *a, *b, manifold, path );
                    const Float64 size = 2.0;
                    if ( (Float64) distance > tolerance * size * 10.0 )
                        ok = ok && !touching;
                    /* a pair that does not touch leaves no points */
                    if ( !touching )
                        ok = ok && 0 == manifold.GetPointCount();
                    if ( T( 0 ) == distance && touching )
                    {
                        ok = ok && manifold.GetPointCount() > 0 && !( manifold.GetPoint( 0 ).depth < T( 0 ) );
                        /* the normal is a unit vector from A towards B: pointOnA - pointOnB = normal * depth */
                        const SContactPoint< T >& point = manifold.GetPoint( 0 );
                        const V3 n = Widen( manifold.normal );
                        ok = ok && fabs( n.GetLength() - 1.0 ) <= tolerance * 10.0;
                        ok = ok && ( Widen( point.pointOnA ) - Widen( point.pointOnB ) - n * (Float64) point.depth ).GetLength() <= tolerance * 100.0 * ( 1.0 + (Float64) point.depth );
                        /* the other order: the opposite normal and the same depth */
                        CTContactManifold< T > swapped;
                        TGeoPairPath swappedPath = GEOPAIRPATH_NOT_OFFERED;
                        if ( CTGeoObjectQueries< T >::ComputeContact( *b, *a, swapped, swappedPath ) && swapped.GetPointCount() > 0 )
                        {
                            const Float64 depthDifference = fabs( (Float64) swapped.GetPoint( 0 ).depth - (Float64) point.depth );
                            ok = ok && depthDifference <= tolerance * 100.0 * ( 1.0 + (Float64) point.depth );
                            if ( (Float64) point.depth > tolerance * 1000.0 )
                                ok = ok && ( Widen( swapped.normal ) + n ).GetLength() <= 0.05;
                        }
                    }
                    if ( !ok )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: contact of " << ObjectKindNames[ ka ] << " - " << ObjectKindNames[ kb ] << " touching " << ( touching ? 1 : 0 ) << " distance " << (Float64) distance << ", seed " << seed );
                }
                if ( IsGeoPairQueryOffered( *a, *b, GEOPAIRQUERY_CONTAINS ) )
                {
                    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
                    const bool contains = CTGeoObjectQueries< T >::Contains( *a, *b, path );
                    /* offered means answered */
                    ok = ok && GEOPAIRPATH_NOT_OFFERED != path;
                    ++offered[ 2 ];
                    if ( contains && closestOffered )
                        ok = ok && intersects;
                }
                if ( !closestOffered )
                {
                    ++notOffered;
                    CTPoint3D< T > onA;
                    CTPoint3D< T > onB;
                    T d = T( 0 );
                    TGeoPairPath path = GEOPAIRPATH_ANALYTIC;
                    CTContactManifold< T > manifold;
                    ok = ok && !CTGeoObjectQueries< T >::ClosestPoints( *a, *b, onA, onB, d, path ) && GEOPAIRPATH_NOT_OFFERED == path;
                    ok = ok && CTNumericTraits< T >::Max() == Distance( *a, *b ) && !Intersects( *a, *b ) && !ComputeContact( *a, *b, manifold ) && 0 == manifold.GetPointCount();
                }
                if ( !IsGeoPairQueryOffered( *a, *b, GEOPAIRQUERY_CONTAINS ) )
                {
                    TGeoPairPath path = GEOPAIRPATH_ANALYTIC;
                    ok = ok && !CTGeoObjectQueries< T >::Contains( *a, *b, path ) && GEOPAIRPATH_NOT_OFFERED == path;
                }
                if ( !ok )
                    ++bad;
                delete a;
                delete b;
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: every pair of " << (UInt32) OBJ_COUNT << " object kinds x " << repeats << " placements (kind size " << (UInt32) sizeof( T ) << "), seed " << seed
               << ": nearest points offered " << offered[ 0 ] << ", contact " << offered[ 1 ] << ", containment " << offered[ 2 ] << ", not offered " << notOffered
               << "; paths analytic " << paths[ GEOPAIRPATH_ANALYTIC ] << ", support " << paths[ GEOPAIRPATH_SUPPORT ] << ", plane " << paths[ GEOPAIRPATH_PLANE ] << ", " << bad << " bad" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** The world box of an AABB or OBB object in Float64, its centre relative to 'origin' */
template < typename T >
static bool
WorldBoxOf( const CTGeoObjectBase< T >& object, UInt32 kind, const CTVector3D< T >& origin, CTOrientedBox3D< Float64 >& box )
{
    CTOrientedBox3D< T > image;
    bool ok = false;
    if ( OBJ_AABB == kind )
        ok = CTShapeTraits< CTAxisAlignedBox3D< T > >::TryImage( static_cast< const CTGeoObject< CTAxisAlignedBox3D< T > >& >( object ).GetShape(), object.GetWorldMatrix(), image );
    else if ( OBJ_OBB == kind )
        ok = CTShapeTraits< CTOrientedBox3D< T > >::TryImage( static_cast< const CTGeoObject< CTOrientedBox3D< T > >& >( object ).GetShape(), object.GetWorldMatrix(), image );
    if ( !ok )
        return false;
    box = Widen( image );
    box.centre = box.centre - Widen( origin );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The analytic path agrees with GJK on the placed support mappings for every pair of the dispatch table's shapes */
template < typename T >
static bool
AnalyticAgreesWithSupport( UInt64 seed, UInt32 repeats, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    UInt32 analytic = 0;
    UInt32 depthChecked = 0;
    UInt32 capped = 0;
    Float64 worst = 0.0;
    for ( UInt32 ka=OBJ_SPHERE; ka<=OBJ_TRIANGLE; ++ka )
    {
        for ( UInt32 kb=OBJ_SPHERE; kb<=OBJ_TRIANGLE; ++kb )
        {
            for ( UInt32 rep=0; rep<repeats; ++rep )
            {
                CTGeoObjectBase< T >* a = MakeObject< T >( ka, r );
                CTGeoObjectBase< T >* b = MakeObject< T >( kb, r );
                CTGeoSpace< T > parentA;
                CTGeoSpace< T > parentB;
                Place( r, parentA, *a, true );
                Place( r, parentB, *b, true );
                CTPoint3D< T > onA;
                CTPoint3D< T > onB;
                T distance = T( 0 );
                TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
                CTGeoObjectQueries< T >::ClosestPoints( *a, *b, onA, onB, distance, path );
                analytic += GEOPAIRPATH_ANALYTIC == path ? 1 : 0;
                const CTVector3D< T > origin = a->GetWorldMatrix().GetTranslation();
                SGjkResult< T > gjk;
                CTGjk< T >::ComputeDistance( a->GetPlacedSupport( origin ), b->GetPlacedSupport( origin ), CTGeometryTolerance< T >(), GUCEF_NULL, gjk );
                const Float64 reference = gjk.overlapping ? 0.0 : (Float64) gjk.distance;
                /*
                 *  touching pairs: EPA on the placed supports against the exact depth. That is the analytic contact
                 *  depth, except for box pairs: the box manifold prefers face axes over a slightly shallower axis
                 *  (feature coherence, see CTSatBoxes) and clips points, so its deepest point is not the minimum
                 *  translation; their exact depth is the smallest SAT overlap of the world boxes. A converged EPA
                 *  is within its termination band max( absolute, relative * depth ) of it; a run stopped at a cap
                 *  (NOT_CONVERGED, curved pairs only) is a lower bound. Both up to rounding at the coordinate scale.
                 */
                bool depthOk = true;
                Float64 exactDepth = 0.0;
                Float64 supportDepth = 0.0;
                CTContactManifold< T > analyticContact;
                TGeoPairPath contactPath = GEOPAIRPATH_NOT_OFFERED;
                if ( CTGeoObjectQueries< T >::ComputeContact( *a, *b, analyticContact, contactPath ) && GEOPAIRPATH_ANALYTIC == contactPath )
                {
                    CTContactManifold< T > supportContact;
                    const MATH::TMathStatus status = ComputeSupportContact( a->GetPlacedSupport( origin ), b->GetPlacedSupport( origin ), CTGeometryTolerance< T >(), supportContact );
                    T deepest = T( 0 );
                    for ( UInt32 k=0; k<analyticContact.GetPointCount(); ++k )
                        deepest = analyticContact.GetPoint( k ).depth > deepest ? analyticContact.GetPoint( k ).depth : deepest;
                    exactDepth = (Float64) deepest;
                    const bool boxPair = ( OBJ_AABB == ka || OBJ_OBB == ka ) && ( OBJ_AABB == kb || OBJ_OBB == kb );
                    CTOrientedBox3D< Float64 > boxA;
                    CTOrientedBox3D< Float64 > boxB;
                    SSatBoxesResult< Float64 > sat;
                    if ( boxPair )
                    {
                        depthOk = WorldBoxOf( *a, ka, origin, boxA ) && WorldBoxOf( *b, kb, origin, boxB ) && CTSatBoxes< Float64 >::Test( boxA, boxB, CTGeometryTolerance< Float64 >(), sat );
                        exactDepth = sat.depth;
                    }
                    supportDepth = supportContact.GetPointCount() > 0 ? (Float64) supportContact.GetPoint( 0 ).depth : 0.0;
                    const CTGeometryTolerance< T > epaTolerance;
                    const Float64 band = (Float64) epaTolerance.relative * exactDepth > (Float64) epaTolerance.absolute ? (Float64) epaTolerance.relative * exactDepth : (Float64) epaTolerance.absolute;
                    /* the supports are evaluated relative to the origin of A: coordinates below 8 */
                    const Float64 rounding = 16.0 * (Float64) CTNumericTraits< T >::Epsilon() * 8.0;
                    const bool converged = 0 == ( status & MATHSTATUS_NOT_CONVERGED );
                    capped += converged ? 0 : 1;
                    depthOk = depthOk && supportContact.GetPointCount() > 0 && ( converged || !boxPair );
                    /*
                     *  boxes: EPA within the band below the exact SAT depth. Curved pairs: within the band of the exact
                     *  depth from either side (see CTEpa, Accuracy); the analytic depth of the pairs without a closed
                     *  form (cone, cylinder, triangle) is itself EPA on the cores in the frame of A, so the two
                     *  differ by up to two bands.
                     */
                    const Float64 limit = boxPair ? band + rounding : 2.0 * band + rounding;
                    depthOk = depthOk && supportDepth <= exactDepth + ( boxPair ? rounding : limit );
                    if ( converged )
                        depthOk = depthOk && exactDepth - supportDepth <= limit;
                    const Float64 error = fabs( exactDepth - supportDepth ) / limit;
                    worst = converged && error > worst ? error : worst;
                    ++depthChecked;
                }
                if ( GEOPAIRPATH_ANALYTIC != path || fabs( (Float64) distance - reference ) > tolerance * ( 1.0 + reference ) || !depthOk )
                {
                    ++bad;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: analytic " << ObjectKindNames[ ka ] << " - " << ObjectKindNames[ kb ] << " distance " << (Float64) distance << " support path " << reference
                                     << " exact depth " << exactDepth << " EPA on the placed supports " << supportDepth << " (EPA - exact in 1e-12: " << ( supportDepth - exactDepth ) * 1.0e12 << "), seed " << seed );
                }
                delete a;
                delete b;
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: analytic against support (kind size " << (UInt32) sizeof( T ) << "), seed " << seed << ": " << analytic << " analytic paths, " << depthChecked
               << " contact depths against EPA (worst converged error " << worst << " of the limit, " << capped << " at a cap), " << bad << " bad" );
    return 0 == bad && analytic > 0 && depthChecked > 0;
}

/*-------------------------------------------------------------------------*/

/** Containment through the planes of the outer object and through the analytic sphere cells */
static bool
LinkedContainment( void )
{
    CGeoSpace parent( CGeoSpace::TTransform( CGeoSpace::TVector( 1.0f, 2.0f, 3.0f ), CGeoSpace::TQuaternion::FromAxisAngle( CGeoSpace::TDirection::UnitZ(), CTRadian< Float32 >( 0.4f ) ),
                                             CGeoSpace::TVector( 2.0f, 1.0f, 3.0f ) ) );
    CGeoAxisAlignedBox box( CTAxisAlignedBox3D< Float32 >( CGeoSpace::TPoint( -1.0f, -1.0f, -1.0f ), CGeoSpace::TPoint( 1.0f, 1.0f, 1.0f ) ) );
    box.SetParent( &parent, false );
    CGeoSphere inside( CTSphere< Float32 >( CGeoSpace::TPoint(), 0.4f ) );
    inside.SetParent( &parent, false );
    CGeoSphere crossing( CTSphere< Float32 >( CGeoSpace::TPoint( 0.9f, 0.0f, 0.0f ), 0.4f ) );
    crossing.SetParent( &parent, false );
    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
    bool ok = CTGeoObjectQueries< Float32 >::Contains( box, inside, path ) && GEOPAIRPATH_PLANE_SET == path;
    ok = ok && !Contains( box, crossing );

    CGeoSphere big( CTSphere< Float32 >( CGeoSpace::TPoint(), 3.0f ) );
    CGeoOrientedBox small( CTOrientedBox3D< Float32 >( CGeoSpace::TPoint( 0.5f, 0.0f, 0.0f ), CGeoSpace::TQuaternion(), CGeoSpace::TVector( 0.5f, 0.5f, 0.5f ) ) );
    CGeoSpace scaled( CGeoSpace::TTransform( CGeoSpace::TVector(), CGeoSpace::TQuaternion(), CGeoSpace::TVector( 2.0f, 2.0f, 2.0f ) ) );
    small.SetParent( &scaled, false );
    ok = ok && CTGeoObjectQueries< Float32 >::Contains( big, small, path ) && GEOPAIRPATH_ANALYTIC == path;
    small.SetLocalPosition( CGeoSpace::TVector( 0.6f, 0.0f, 0.0f ) );
    ok = ok && !Contains( big, small );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** World point and ray queries through the value facade on an ellipsoid (a sphere under non-uniform scale and rotation) */
template < typename T >
static bool
PointAndRayQueries( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTGeoSpace< T > parent( RandomTransform< T >( r, 2.0, false, 0.5, 2.0, 0.0 ) );
        const P3 sphereCentre = r.Point( 0.5 );
        const Float64 sphereRadius = r.Uniform( 0.5, 1.0 );
        const CTTransform3D< T > place = RandomTransform< T >( r, 2.0, false, 0.5, 2.0, 0.0 );
        CTGeoObject< CTSphere< T > > ellipsoid( CTSphere< T >( ToP< T >( sphereCentre ), T( sphereRadius ) ), place );
        ellipsoid.SetParent( &parent, false );
        const M4 world = ReferenceWorld( ellipsoid );
        const M4 inverse = world.InverseAffine();
        const P3 centre = Widen( ellipsoid.GetShape().centre );
        const Float64 radius = (Float64) ellipsoid.GetShape().radius;

        /* ray: the analytic hit of the local unit sphere, mapped back */
        const P3 origin = world.TransformPoint( centre ) + r.Vector( 8.0 );
        const D3 towards = r.Direction();
        const Float64 reach = r.Uniform( 0.0, 0.9 ) * radius;
        const P3 aim = world.TransformPoint( centre + towards.AsVector() * reach );
        D3 direction;
        D3::TryCreate( aim - origin, direction );
        const P3 lo = inverse.TransformPoint( origin );
        const V3 ld = inverse.TransformVector( direction.AsVector() );
        const V3 oc = lo - centre;
        const Float64 qa = ld.Dot( ld );
        const Float64 qb = 2.0 * oc.Dot( ld );
        const Float64 qc = oc.Dot( oc ) - radius * radius;
        const Float64 disc = qb * qb - 4.0 * qa * qc;
        const Float64 tReference = qc < 0.0 ? 0.0 : ( -qb - sqrt( disc ) ) / ( 2.0 * qa );
        SRayCastResult< T > hit;
        const bool didHit = RayCast( CTRay3D< T >( ToP< T >( origin ), ToD< T >( direction ) ), ellipsoid, CTNumericTraits< T >::Max(), hit );
        bool ok = didHit && disc >= 0.0 && fabs( (Float64) hit.distance - tReference ) <= tolerance * ( 1.0 + tReference );
        const bool rayOk = ok;
        /* the world normal is perpendicular to the surface: the gradient of the local sphere mapped by the inverse transpose */
        const V3 localGradient = inverse.TransformPoint( Widen( hit.point ) ) - centre;
        V3 gradient = inverse.GetLinear().Transposed() * localGradient;
        gradient = gradient * ( 1.0 / gradient.GetLength() );
        if ( 0.0 == tReference )
            ok = ok && hit.StartedInside() && ( Widen( hit.normal ) + direction.AsVector() ).GetLength() <= tolerance * 100.0;
        else
            ok = ok && ( Widen( hit.normal ) - gradient ).GetLength() <= tolerance * 100.0;
        const bool normalOk = ok;

        /* closest point: on the surface, and the offset to the point is along the surface normal there */
        const P3 query = world.TransformPoint( centre ) + r.Vector( 6.0 );
        const bool inside = ( inverse.TransformPoint( query ) - centre ).GetLength() <= radius;
        const CTPoint3D< T > closest = ellipsoid.ClosestPoint( ToP< T >( query ) );
        Float64 surfaceError = 0.0;
        Float64 sineError = 0.0;
        Float64 distanceError = 0.0;
        if ( inside )
        {
            ok = ok && Intersects( ToP< T >( query ), ellipsoid ) && T( 0 ) == Distance( ToP< T >( query ), ellipsoid );
        }
        else
        {
            const V3 g = inverse.GetLinear().Transposed() * ( inverse.TransformPoint( Widen( closest ) ) - centre );
            const V3 offset = query - Widen( closest );
            const Float64 sine = g.Cross( offset ).GetLength() / ( g.GetLength() * offset.GetLength() );
            surfaceError = fabs( ( inverse.TransformPoint( Widen( closest ) ) - centre ).GetLength() - radius );
            sineError = sine;
            distanceError = fabs( (Float64) Distance( ToP< T >( query ), ellipsoid ) - offset.GetLength() );
            /* GJK stops on the distance: the witness direction of a curved surface is accurate to about the square root of that */
            ok = ok && surfaceError <= tolerance * 10.0 && sine <= sqrt( tolerance ) * 10.0;
            ok = ok && !Intersects( ToP< T >( query ), ellipsoid ) && distanceError <= tolerance * 10.0;
        }

        /* world bounds of an ellipsoid: centre +- radius * the row lengths of the linear part (exact) */
        const CTAxisAlignedBox3D< T > bounds = ellipsoid.GetWorldBounds();
        const P3 wc = world.TransformPoint( centre );
        const M4 linear = world;
        for ( UInt32 axis=0; axis<3; ++axis )
        {
            const Float64 extent = radius * V3( linear( axis, 0 ), linear( axis, 1 ), linear( axis, 2 ) ).GetLength();
            ok = ok && fabs( (Float64) bounds.min.GetComponent( axis ) - ( wc.GetComponent( axis ) - extent ) ) <= tolerance * 10.0 * ( 1.0 + extent );
            ok = ok && fabs( (Float64) bounds.max.GetComponent( axis ) - ( wc.GetComponent( axis ) + extent ) ) <= tolerance * 10.0 * ( 1.0 + extent );
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: point and ray case " << i << " hit " << ( didHit ? 1 : 0 ) << " distance " << (Float64) hit.distance << " reference " << tReference
                             << " ray ok " << ( rayOk ? 1 : 0 ) << " normal ok " << ( normalOk ? 1 : 0 ) << " inside " << ( inside ? 1 : 0 ) << " surface " << surfaceError << " sine " << sineError
                             << " distance error " << distanceError << ", seed " << seed );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/**
 *  A structure for acceptance item 9: a moved list refitted in Update, entries removed when objects die. The object
 *  list is the base's (it also unregisters from the remaining objects when the structure is destroyed).
 */
class CTestStructure : public CGeoObjectStructure
{
    public:

    CTestStructure( void ) : moved(), refits( 0 ) {}

    void Insert( CGeoObjectBase& object )
    {
        object.RegisterStructure( *this );
    }

    /** Refits the moved objects; returns how often each object of 'probe' was in the moved list */
    void Update( void )
    {
        for ( UInt32 i=0; i<(UInt32) moved.size(); ++i )
        {
            const CTAxisAlignedBox3D< Float32 >& bounds = moved[ i ]->AcknowledgeMove( *this );
            refits += bounds.IsEmpty() ? 0 : 1;
        }
        moved.clear();
    }

    UInt32 CountMoved( const CGeoObjectBase& object ) const
    {
        UInt32 count = 0;
        for ( UInt32 i=0; i<(UInt32) moved.size(); ++i )
            count += moved[ i ] == &object ? 1 : 0;
        return count;
    }

    GUCEF::vector< CGeoObjectBase* > moved;
    UInt32 refits;

    protected:

    virtual void OnGeoObjectMoved( CGeoObjectBase& object ) GUCEF_VIRTUAL_OVERRIDE { moved.push_back( &object ); }

    virtual void OnGeoObjectDestroyed( CGeoObjectBase& object ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=(UInt32) moved.size(); i>0; --i )
            if ( moved[ i - 1 ] == &object )
                moved.erase( moved.begin() + ( i - 1 ) );
    }
};

/*-------------------------------------------------------------------------*/

/** Acceptance item 9: each moved object exactly once per change batch; destroyed objects leave the structure */
static bool
BoundsInvalidation( void )
{
    CGeoSpace root;
    CGeoSpace parentA;
    parentA.SetParent( &root, false );
    CGeoSphere o1( CTSphere< Float32 >( CGeoSpace::TPoint(), 1.0f ) );
    CGeoSphere* o2 = new CGeoSphere( CTSphere< Float32 >( CGeoSpace::TPoint(), 0.5f ) );
    CGeoOrientedBox o3;
    CGeoCapsule o4;
    o1.SetParent( &parentA, false );
    o2->SetParent( &parentA, false );
    o3.SetParent( &root, false );
    CTestStructure* structure = new CTestStructure();
    CTestStructure second;
    structure->Insert( o1 );
    structure->Insert( *o2 );
    structure->Insert( o3 );
    structure->Insert( o4 );
    second.Insert( o1 );
    bool ok = 0 == structure->moved.size() && 4 == structure->GetObjectCount() && 2 == o1.GetStructureCount();
    ok = ok && !o1.RegisterStructure( *structure );

    /* batch 1: repeated moves of o1, a read in between, then its parent rotates: o1 and o2 once each */
    o1.Translate( CGeoSpace::TVector( 1.0f, 0.0f, 0.0f ), GEOSPACE_RELATIVETO_PARENT );
    o1.Translate( CGeoSpace::TVector( 1.0f, 0.0f, 0.0f ), GEOSPACE_RELATIVETO_PARENT );
    o1.GetWorldPosition();
    o1.Translate( CGeoSpace::TVector( 1.0f, 0.0f, 0.0f ), GEOSPACE_RELATIVETO_PARENT );
    parentA.Rotate( CGeoSpace::TQuaternion::FromAxisAngle( CGeoSpace::TDirection::UnitY(), CTRadian< Float32 >( 0.5f ) ), GEOSPACE_RELATIVETO_LOCAL );
    ok = ok && 2 == structure->moved.size() && 1 == structure->CountMoved( o1 ) && 1 == structure->CountMoved( *o2 ) && 0 == structure->CountMoved( o3 ) && 0 == structure->CountMoved( o4 );
    ok = ok && 1 == second.moved.size() && o1.IsMoveQueued( *structure ) && !o3.IsMoveQueued( *structure );
    structure->Update();
    second.Update();
    ok = ok && 2 == structure->refits && !o1.IsMoveQueued( *structure );

    /* batch 2: the root moves (o1, o2, o3), a shape change of an already moved object adds nothing */
    root.SetLocalPosition( CGeoSpace::TVector( 0.0f, 3.0f, 0.0f ) );
    o2->SetShape( CTSphere< Float32 >( CGeoSpace::TPoint(), 0.75f ) );
    ok = ok && 3 == structure->moved.size() && 1 == structure->CountMoved( o1 ) && 1 == structure->CountMoved( *o2 ) && 1 == structure->CountMoved( o3 ) && 0 == structure->CountMoved( o4 );
    structure->Update();
    second.Update();

    /* batch 3: a shape change alone */
    o4.SetShape( CTCapsule< Float32 >( CGeoSpace::TPoint(), CGeoSpace::TPoint( 0.0f, 2.0f, 0.0f ), 0.25f ) );
    ok = ok && 1 == structure->moved.size() && 1 == structure->CountMoved( o4 );
    structure->Update();
    ok = ok && fabs( o4.GetWorldBounds().max.y - 2.25f ) <= 1.0e-6f;

    /* batch 4: a queued object is destroyed: the structure forgets it */
    parentA.Translate( CGeoSpace::TVector( 0.0f, 0.0f, 1.0f ), GEOSPACE_RELATIVETO_PARENT );
    ok = ok && 2 == structure->moved.size();
    delete o2;
    ok = ok && 1 == structure->moved.size() && 3 == structure->GetObjectCount() && 1 == structure->CountMoved( o1 );
    structure->Update();

    /* the structure goes first: it unregisters from its objects */
    delete structure;
    ok = ok && 1 == o1.GetStructureCount() && 0 == o3.GetStructureCount();
    o3.SetLocalPosition( CGeoSpace::TVector( 5.0f, 0.0f, 0.0f ) );
    ok = ok && o1.UnregisterStructure( second ) && !o1.UnregisterStructure( second );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A structure that unregisters itself from every object it hears of */
class CUnregisteringStructure : public CGeoObjectStructure
{
    public:

    CUnregisteringStructure( void ) : moved( 0 ) {}

    UInt32 moved;

    protected:

    virtual void OnGeoObjectMoved( CGeoObjectBase& object ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++moved;
        object.UnregisterStructure( *this );
    }

    virtual void OnGeoObjectDestroyed( CGeoObjectBase& /*object*/ ) GUCEF_VIRTUAL_OVERRIDE {}
};

/*-------------------------------------------------------------------------*/

/**
 *  Structures that change the links while being told: the next structure is still told; a structure destroyed
 *  without unregistering takes its links with it (ASan checks the next move)
 */
static bool
StructureLinksDuringCallbacks( void )
{
    CGeoSphere object( CTSphere< Float32 >( CGeoSpace::TPoint(), 1.0f ) );
    CUnregisteringStructure leaving;
    CTestStructure staying;
    object.RegisterStructure( leaving );
    staying.Insert( object );
    object.SetLocalPosition( CGeoSpace::TVector( 1.0f, 0.0f, 0.0f ) );
    bool ok = 1 == leaving.moved && 1 == staying.CountMoved( object ) && 1 == object.GetStructureCount() && 0 == leaving.GetObjectCount() && 1 == staying.GetObjectCount();
    staying.Update();
    object.SetLocalPosition( CGeoSpace::TVector( 2.0f, 0.0f, 0.0f ) );
    ok = ok && 1 == leaving.moved && 1 == staying.CountMoved( object );
    staying.Update();
    {
        CUnregisteringStructure temporary;
        object.RegisterStructure( temporary );
        ok = ok && 2 == object.GetStructureCount() && 1 == temporary.GetObjectCount() && temporary.GetObject( 0 ) == &object;
    }
    /* the temporary structure is gone: the object no longer links to it */
    ok = ok && 1 == object.GetStructureCount();
    object.SetLocalPosition( CGeoSpace::TVector( 3.0f, 0.0f, 0.0f ) );
    ok = ok && 1 == staying.CountMoved( object );

    /* the base keeps its object list consistent under removal in any order */
    CTestStructure many;
    CGeoSphere spheres[ 6 ];
    for ( UInt32 k=0; k<6; ++k )
        many.Insert( spheres[ k ] );
    spheres[ 1 ].UnregisterStructure( many );
    spheres[ 4 ].UnregisterStructure( many );
    ok = ok && 4 == many.GetObjectCount();
    for ( UInt32 k=0; k<many.GetObjectCount(); ++k )
        ok = ok && many.GetObject( k )->IsRegisteredWith( many ) && many.GetObject( k ) != &spheres[ 1 ] && many.GetObject( k ) != &spheres[ 4 ];
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Empty shapes have no points: not offered, empty bounds, no hits, the largest distance (items 3 and 4) */
static bool
EmptyShapes( void )
{
    CGeoSphere sphere( CTSphere< Float32 >( CGeoSpace::TPoint(), 1.0f ) );
    CGeoSphere farSphere( CTSphere< Float32 >( CGeoSpace::TPoint( 1000.0f, 1000.0f, 0.0f ), 1.0f ) );
    CGeoPlane boundedToNothing;
    boundedToNothing.SetExtents( CTRectangle< Float32 >() );
    const CTAxisAlignedBox3D< Float32 > noBox;
    CGeoAxisAlignedBox emptyBox( noBox );
    emptyBox.SetLocalScale( CGeoSpace::TVector( 1.0f, 3.0f, 1.0f ) );
    emptyBox.SetLocalRotation( CGeoSpace::TQuaternion::FromAxisAngle( CGeoSpace::TDirection::UnitZ(), CTRadian< Float32 >( 0.5f ) ) );
    const CTRectangle< Float32 > noRectangle;
    CGeoRectangle emptyRectangle( noRectangle );
    CGeoCircle negativeCircle( CTCircle< Float32 >( CTPoint2D< Float32 >(), -1.0f ) );
    CTGeoObjectBase< Float32 >* empties[ 4 ] = { &boundedToNothing, &emptyBox, &emptyRectangle, &negativeCircle };
    bool ok = true;
    for ( UInt32 k=0; k<4; ++k )
    {
        CTGeoObjectBase< Float32 >& e = *empties[ k ];
        ok = ok && e.IsEmpty() && e.GetWorldBounds().IsEmpty() && !e.Contains( CGeoSpace::TPoint() );
        ok = ok && CTNumericTraits< Float32 >::Max() == Distance( CGeoSpace::TPoint(), e );
        SRayCastResult< Float32 > hit;
        ok = ok && !e.RayCast( CTRay3D< Float32 >( CGeoSpace::TPoint( 0.0f, 0.0f, 5.0f ), -CGeoSpace::TDirection::UnitZ() ), 100.0f, hit );
        for ( UInt32 q=GEOPAIRQUERY_CLOSESTPOINTS; q<=GEOPAIRQUERY_CONTAINS; ++q )
            ok = ok && !IsGeoPairQueryOffered( sphere, e, (TGeoPairQuery) q ) && !IsGeoPairQueryOffered( e, sphere, (TGeoPairQuery) q );
        TGeoPairPath path = GEOPAIRPATH_ANALYTIC;
        ok = ok && !CTGeoObjectQueries< Float32 >::Intersects( sphere, e, path ) && GEOPAIRPATH_NOT_OFFERED == path;
        ok = ok && !Intersects( farSphere, e ) && CTNumericTraits< Float32 >::Max() == Distance( e, sphere );
    }
    /* a default rectangle is empty too, a bounded plane with extents is not */
    CGeoRectangle defaultRectangle;
    CGeoPlane bounded;
    bounded.SetExtents( CTRectangle< Float32 >( CTPoint2D< Float32 >( -1.0f, -1.0f ), CTPoint2D< Float32 >( 1.0f, 1.0f ) ) );
    ok = ok && defaultRectangle.IsEmpty() && !bounded.IsEmpty() && !Intersects( bounded, farSphere ) && Intersects( bounded, sphere );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** IsGeoPairQueryOffered( CONTAINS ) runs the same image test as Contains (item 7) */
static bool
ContainsOfferedMatchesAnswer( void )
{
    CGeoSphere outer( CTSphere< Float32 >( CGeoSpace::TPoint(), 10.0f ) );
    CGeoSphere inner( CTSphere< Float32 >( CGeoSpace::TPoint(), 1.0f ) );
    inner.SetLocalScale( CGeoSpace::TVector( 1.0f, 2.0f, 1.0f ) );
    bool ok = !IsGeoPairQueryOffered( outer, inner, GEOPAIRQUERY_CONTAINS );
    inner.SetLocalScale( CGeoSpace::TVector( 2.0f, 2.0f, 2.0f ) );
    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
    ok = ok && IsGeoPairQueryOffered( outer, inner, GEOPAIRQUERY_CONTAINS ) && CTGeoObjectQueries< Float32 >::Contains( outer, inner, path ) && GEOPAIRPATH_ANALYTIC == path;
    /* an oriented box under shear has no image in the sphere's space */
    CGeoSpace shearing( CGeoSpace::TTransform( CGeoSpace::TVector(), CGeoSpace::TQuaternion(), CGeoSpace::TVector( 3.0f, 1.0f, 1.0f ) ) );
    CGeoOrientedBox box( CTOrientedBox3D< Float32 >( CGeoSpace::TPoint(), CGeoSpace::TQuaternion::FromAxisAngle( CGeoSpace::TDirection::UnitZ(), CTRadian< Float32 >( 0.6f ) ),
                                                     CGeoSpace::TVector( 0.5f, 0.5f, 0.5f ) ) );
    box.SetParent( &shearing, false );
    ok = ok && !IsGeoPairQueryOffered( outer, box, GEOPAIRQUERY_CONTAINS );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A user class that claims a library shape type */
class CLyingUserCube : public CTUserCube< Float32 >
{
    public:

    virtual TGeoShapeType GetShapeType( void ) const GUCEF_VIRTUAL_OVERRIDE { return GEOSHAPE_SPHERE; }
};

/**
 *  The dispatch casts by the library tag (item 14): a user class claiming GEOSHAPE_SPHERE is answered as a user shape
 *  through its support mapping (debug builds assert, so only release builds run the claim)
 */
static bool
DispatchByLibraryTag( void )
{
    CTUserCube< Float32 > honest;
    CGeoSphere sphere( CTSphere< Float32 >( CGeoSpace::TPoint( 3.0f, 0.0f, 0.0f ), 1.0f ) );
    bool ok = GEOSHAPE_UNKNOWN == honest.GetDispatchShapeType() && GEOSHAPE_SPHERE == sphere.GetDispatchShapeType();
    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
    CTPoint3D< Float32 > onA;
    CTPoint3D< Float32 > onB;
    Float32 distance = 0.0f;
    ok = ok && CTGeoObjectQueries< Float32 >::ClosestPoints( honest, sphere, onA, onB, distance, path ) && GEOPAIRPATH_SUPPORT == path && fabs( distance - 1.5f ) <= 1.0e-4f;
    #ifndef GUCEF_GEOMETRY_DEBUG_MODE
    CLyingUserCube lying;
    ok = ok && GEOSHAPE_UNKNOWN == lying.GetDispatchShapeType();
    ok = ok && CTGeoObjectQueries< Float32 >::ClosestPoints( lying, sphere, onA, onB, distance, path ) && GEOPAIRPATH_SUPPORT == path && fabs( distance - 1.5f ) <= 1.0e-4f;
    #endif
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Coplanar flat pairs: in-plane contact normal and depth; apart: false with an empty manifold (item 15) */
template < typename T >
static bool
CoplanarFlatContact( UInt64 seed, UInt32 count, Float64 tolerance )
{
    typedef CTPoint2D< T > TP2;
    CRandom r( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTGeoPlane< T > plane( RandomTransform< T >( r, 5.0, true, 0.5, 2.0, 0.0 ) );
        CTGeoObject2D< CTRectangle< T > > a( CTRectangle< T >( TP2( T( -1 ), T( -1 ) ), TP2( T( 1 ), T( 1 ) ) ) );
        CTGeoObject2D< CTRectangle< T > > b( CTRectangle< T >( TP2( T( -1 ), T( -1 ) ), TP2( T( 1 ), T( 1 ) ) ) );
        a.SetParent( &plane, false );
        b.SetParent( &plane, false );
        const Float64 overlap = r.Uniform( 0.1, 0.9 );
        b.SetLocalTransform2D( CTTransform2D< T >( CTVector2D< T >( T( 2.0 - overlap ), T( 0 ) ), CTRadian< T >( T( 0 ) ), CTVector2D< T >::One() ) );
        CTContactManifold< T > manifold;
        TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
        bool ok = CTGeoObjectQueries< T >::ComputeContact( a, b, manifold, path ) && manifold.GetPointCount() > 0;
        const Float64 scale = (Float64) plane.GetLocalTransform().scale.x;
        const V3 expectedNormal = Widen( plane.ToWorld( CTVector3D< T >( T( 1 ), T( 0 ), T( 0 ) ) ) ) * ( 1.0 / scale );
        ok = ok && ( Widen( manifold.normal ) - expectedNormal ).GetLength() <= tolerance;
        ok = ok && fabs( (Float64) manifold.GetPoint( 0 ).depth - overlap * scale ) <= tolerance * ( 1.0 + scale );
        ok = ok && fabs( (Float64) plane.GetPlane().GetSignedDistance( manifold.GetPoint( 0 ).pointOnA ) ) <= tolerance * ( 1.0 + scale ) * 10.0;
        /* apart: no contact, no points */
        b.SetLocalTransform2D( CTTransform2D< T >( CTVector2D< T >( T( 2.0 + overlap ), T( 0 ) ), CTRadian< T >( T( 0 ) ), CTVector2D< T >::One() ) );
        ok = ok && !CTGeoObjectQueries< T >::ComputeContact( a, b, manifold, path ) && 0 == manifold.GetPointCount();
        ok = ok && fabs( (Float64) Distance( a, b ) - overlap * scale ) <= tolerance * ( 1.0 + scale ) * 10.0;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: coplanar contact case " << i << " normal (" << (Float64) manifold.normal.x << ", " << (Float64) manifold.normal.y << ", "
                             << (Float64) manifold.normal.z << ") expected (" << expectedNormal.x << ", " << expectedNormal.y << ", " << expectedNormal.z << ") depth "
                             << ( manifold.GetPointCount() > 0 ? (Float64) manifold.GetPoint( 0 ).depth : -1.0 ) << " overlap " << overlap * scale << ", seed " << seed );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/**
 *  Two 4 x 4 plates through one centre, the second tilted about the x axis by angles from 10^-10 to 0.2 radians, under
 *  a seeded rotated parent: the pair counts as coplanar (in-plane depth 4) only up to 'coplanarAngle', in one range
 *  starting at 0; above it the depth is that of the crossing plates, 2 sin( angle ), so it grows continuously from
 *  the coplanar tolerance instead of jumping from 4 at a few degrees (just above the tolerance a crossing that shallow
 *  may also be reported apart)
 */
template < typename T >
static bool
TiltedPlatesDepthIsContinuous( UInt64 seed, Float64 coplanarAngle, Float64 depthTolerance )
{
    typedef CTPoint2D< T > TP2;
    typedef CTGeoObject2D< CTRectangle< T > > TPlate;
    CRandom r( seed );
    bool ok = true;
    UInt32 coplanarCount = 0;
    Float64 largestCoplanar = -1.0;
    Float64 worstDepthError = 0.0;
    for ( UInt32 pose=0; pose<4; ++pose )
    {
        const Q4 parentRotation = r.Rotation();
        CTGeoSpace< T > parent( typename CTGeoSpace< T >::TTransform( CTVector3D< T >(), ToQ< T >( parentRotation ), CTVector3D< T >::One() ) );
        const CTRectangle< T > square( TP2( T( -2 ), T( -2 ) ), TP2( T( 2 ), T( 2 ) ) );
        TPlate a( square );
        TPlate b( square );
        a.SetParent( &parent, false );
        b.SetParent( &parent, false );
        bool coplanarRange = true;
        for ( UInt32 k=0; k<=104; ++k )
        {
            const Float64 angle = 0 == k ? 0.0 : 1.0e-10 * pow( 10.0, ( k - 1 ) / 11.0 );
            b.SetLocalRotation( CTQuaternion< T >::FromAxisAngle( CTDirectionVector3D< T >::UnitX(), CTRadian< T >( T( angle ) ) ) );
            CTContactManifold< T > manifold;
            TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
            const bool touching = CTGeoObjectQueries< T >::ComputeContact( a, b, manifold, path );
            const Float64 depth = (Float64) manifold.GetMaxDepth();
            const bool coplanar = fabs( depth - 4.0 ) <= 1.0e-2;
            /* crossing plates whose depth is near the coplanar tolerance may be reported apart: depth about 0 either way */
            bool caseOk = touching || ( 0 != k && 2.0 * sin( angle ) <= 4.0 * coplanarAngle );
            if ( !touching )
            {
                coplanarRange = false;
            }
            else
            if ( coplanar )
            {
                ++coplanarCount;
                largestCoplanar = angle > largestCoplanar ? angle : largestCoplanar;
                caseOk = caseOk && coplanarRange && angle <= coplanarAngle;
            }
            else
            {
                coplanarRange = false;
                const Float64 error = fabs( depth - 2.0 * sin( angle ) );
                worstDepthError = error > worstDepthError ? error : worstDepthError;
                caseOk = caseOk && error <= depthTolerance * ( 1.0 + 2.0 * sin( angle ) ) && 0 != k;
            }
            if ( !caseOk )
            {
                ok = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: tilted plates (kind size " << sizeof( T ) << ") pose " << pose << " step " << k << " (log10 of the angle " << ( 0 == k ? -99.0 : log10( angle ) )
                                 << ") depth " << ( touching ? depth : -1.0 ) << " 2 sin " << 2.0 * sin( angle ) << ( coplanar ? " (coplanar)" : "" ) << ", seed " << seed );
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoObjects: tilted plates (kind size " << sizeof( T ) << "), seed " << seed << ": " << coplanarCount << " coplanar up to " << largestCoplanar
               << " radians, worst crossing depth error " << worstDepthError );
    return ok && coplanarCount >= 4;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeoObjectsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeoObjects" );

    GUCEF_TESTFW_TESTCASE( "GeoObjects: linked spheres under uniformly scaled parents use the analytic path and equal the value-shape results" )
    try
    {
        ASSERT_TRUE( LinkedSpheresAnalytic< Float32 >( 8101, ScaledCount( 2000 ), 1.0e-5 ) );
        ASSERT_TRUE( LinkedSpheresAnalytic< Float64 >( 8102, ScaledCount( 2000 ), 1.0e-12 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoObjects: under non-uniform scale the GJK path agrees with a dense-sampling reference (1e-3 relative)" )
    try
    {
        ASSERT_TRUE( EllipsoidsAgainstSampling< Float32 >( 8201, 20 ) );
        ASSERT_TRUE( EllipsoidsAgainstSampling< Float64 >( 8202, 20 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoObjects: every pair type of the dispatch table returns a consistent result or the documented not-offered response" )
    try
    {
        ASSERT_TRUE( EveryPairType< Float32 >( 8301, 6, 2.0e-3 ) );
        ASSERT_TRUE( EveryPairType< Float64 >( 8302, 6, 1.0e-5 ) );
        ASSERT_TRUE( AnalyticAgreesWithSupport< Float32 >( 8303, ScaledCount( 40 ), 2.0e-3 ) );
        ASSERT_TRUE( AnalyticAgreesWithSupport< Float64 >( 8304, ScaledCount( 40 ), 1.0e-5 ) );
        ASSERT_TRUE( LinkedContainment() );
        ASSERT_TRUE( ContainsOfferedMatchesAnswer() );
        ASSERT_TRUE( DispatchByLibraryTag() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoObjects: empty shapes have no points: not offered, empty bounds, no hits" )
    try
    {
        ASSERT_TRUE( EmptyShapes() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoObjects: coplanar flat objects: in-plane contact normal and depth, no points when apart; tilted plates count as coplanar only within the angle tolerance" )
    try
    {
        ASSERT_TRUE( CoplanarFlatContact< Float32 >( 8501, ScaledCount( 400 ), 1.0e-4 ) );
        ASSERT_TRUE( CoplanarFlatContact< Float64 >( 8502, ScaledCount( 400 ), 1.0e-9 ) );
        ASSERT_TRUE( TiltedPlatesDepthIsContinuous< Float32 >( 8503, 2.0e-3, 1.0e-4 ) );
        ASSERT_TRUE( TiltedPlatesDepthIsContinuous< Float64 >( 8504, 1.0e-7, 1.0e-9 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoObjects: world point, ray and bounds queries through the value facade, exact under non-uniform scale" )
    try
    {
        ASSERT_TRUE( PointAndRayQueries< Float32 >( 8401, ScaledCount( 2000 ), 1.0e-5 ) );
        ASSERT_TRUE( PointAndRayQueries< Float64 >( 8402, ScaledCount( 2000 ), 1.0e-10 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoObjects: a registered structure receives each moved object once per change batch and forgets destroyed objects" )
    try
    {
        ASSERT_TRUE( BoundsInvalidation() );
        ASSERT_TRUE( StructureLinksDuringCallbacks() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
