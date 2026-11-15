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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_GEOOBJECTQUERIES_H
#include "gucefGEOMETRY_GeoObjectQueries.h"
#define GUCEF_GEOMETRY_GEOOBJECTQUERIES_H
#endif /* GUCEF_GEOMETRY_GEOOBJECTQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#include "gucefGEOMETRY_PolygonTriangulation.h"
#define GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#endif /* GUCEF_GEOMETRY_POLYGONTRIANGULATION_H ? */

#include "TestGeoSpaceReference.h"
#include "TestGeoPolygonQueries.h"

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

static const Float64 PolygonPi = 3.14159265358979323846;

enum EPolygonKind
{
    POLY_L = 0, POLY_STAR, POLY_HOLES, POLY_COUNT
};

static const char* const PolygonKindNames[ POLY_COUNT ] = { "L shape", "star", "square with a square hole" };

enum EOtherKind
{
    OTHER_SPHERE = 0, OTHER_BOX, OTHER_CAPSULE, OTHER_POLYGON, OTHER_COUNT
};

static const char* const OtherKindNames[ OTHER_COUNT ] = { "sphere", "box", "capsule", "polygon" };

/*-------------------------------------------------------------------------*/

/** The region of a polygon kind in plane coordinates: an L (4 x 3, notch x in ( 1, 4 ], y in ( 1, 3 ]), a ten pointed star, a square with a square hole */
template < typename T >
static CTPolygonWithHoles2D< T >
MakeRegion( UInt32 kind )
{
    typedef CTPoint2D< T > TP2;
    CTPolygonWithHoles2D< T > region;
    switch ( kind )
    {
        case POLY_L:
        {
            const TP2 ring[ 6 ] = { TP2( T( 0 ), T( 0 ) ), TP2( T( 4 ), T( 0 ) ), TP2( T( 4 ), T( 1 ) ), TP2( T( 1 ), T( 1 ) ), TP2( T( 1 ), T( 3 ) ), TP2( T( 0 ), T( 3 ) ) };
            region.outer = CTPolygon2D< T >( ring, 6 );
            break;
        }
        case POLY_STAR:
        {
            TP2 ring[ 10 ];
            for ( UInt32 i=0; i<10; ++i )
            {
                const Float64 radius = 0 == i % 2 ? 2.2 : 0.9;
                const Float64 angle = (Float64) i * PolygonPi / 5.0 + PolygonPi / 10.0;
                ring[ i ] = TP2( (T) ( radius * cos( angle ) ), (T) ( radius * sin( angle ) ) );
            }
            region.outer = CTPolygon2D< T >( ring, 10 );
            break;
        }
        default:
        {
            const TP2 outer[ 4 ] = { TP2( T( -4 ), T( -4 ) ), TP2( T( 4 ), T( -4 ) ), TP2( T( 4 ), T( 4 ) ), TP2( T( -4 ), T( 4 ) ) };
            const TP2 hole[ 4 ] = { TP2( T( -2 ), T( -2 ) ), TP2( T( -2 ), T( 2 ) ), TP2( T( 2 ), T( 2 ) ), TP2( T( 2 ), T( -2 ) ) };
            region.outer = CTPolygon2D< T >( outer, 4 );
            region.holes.push_back( CTPolygon2D< T >( hole, 4 ) );
            break;
        }
    }
    return region;
}

/*-------------------------------------------------------------------------*/

/** Even-odd ray crossing of one ring in Float64, written out here (a horizontal ray to +x, edges half-open in y) */
template < typename T >
static bool
RingCrossesOdd( const CTPolygon2D< T >& ring, Float64 x, Float64 y )
{
    bool odd = false;
    const UInt32 n = ring.GetPointCount();
    for ( UInt32 i=0, j=n-1; i<n; j=i++ )
    {
        const Float64 xi = (Float64) ring.GetPoint( i ).x;
        const Float64 yi = (Float64) ring.GetPoint( i ).y;
        const Float64 xj = (Float64) ring.GetPoint( j ).x;
        const Float64 yj = (Float64) ring.GetPoint( j ).y;
        if ( ( yi > y ) != ( yj > y ) && x < xj + ( y - yj ) * ( xi - xj ) / ( yi - yj ) )
            odd = !odd;
    }
    return odd;
}

/** True when ( x, y ) is inside the region: odd crossings of the outer ring and of the holes together */
template < typename T >
static bool
RegionContainsReference( const CTPolygonWithHoles2D< T >& region, Float64 x, Float64 y )
{
    bool inside = RingCrossesOdd( region.outer, x, y );
    for ( size_t i=0; i<region.holes.size(); ++i )
        inside = RingCrossesOdd( region.holes[ i ], x, y ) ? !inside : inside;
    return inside;
}

/*-------------------------------------------------------------------------*/

/** A polygon object on a plane under a root node: root ( rotated, scaled ) -> plane ( rotated, scaled ) -> polygon */
template < typename T >
class CPolygonHost
{
    public:

    CPolygonHost( UInt32 kind, const CTTransform3D< T >& rootTransform, const CTTransform3D< T >& planeTransform )
        : m_kind( kind ), m_region( MakeRegion< T >( kind ) ), m_root( rootTransform ), m_plane( planeTransform ), m_object( GUCEF_NULL )
    {
        m_plane.SetParent( &m_root, false );
        if ( POLY_HOLES == kind )
            m_object = new CTGeoObject2D< CTPolygonWithHoles2D< T > >( m_region );
        else
            m_object = new CTGeoObject2D< CTPolygon2D< T > >( m_region.outer );
        m_object->SetParent( &m_plane, false );
    }

    ~CPolygonHost()
    {
        delete m_object;
    }

    CTGeoObjectBase< T >& Object( void ) { return *m_object; }
    const CTGeoObjectBase< T >& Object( void ) const { return *m_object; }
    CTGeoSpace< T >& Root( void ) { return m_root; }
    CTGeoPlane< T >& Plane( void ) { return m_plane; }
    const CTPolygonWithHoles2D< T >& Region( void ) const { return m_region; }

    /** The centre of the bounds of the region in world coordinates, from the Float64 reference chain */
    P3 WorldCentre( void ) const
    {
        const CTRectangle< T > bounds = m_region.outer.GetBounds();
        const Float64 cx = 0.5 * ( (Float64) bounds.min.x + (Float64) bounds.max.x );
        const Float64 cy = 0.5 * ( (Float64) bounds.min.y + (Float64) bounds.max.y );
        return ReferenceWorld( *m_object ).TransformPoint( P3( cx, cy, 0.0 ) );
    }

    /** The triangle count of any triangulation of the region without added vertices: vertices - 2 + 2 * holes */
    UInt32 GetTriangulationCount( void ) const
    {
        UInt32 vertices = m_region.outer.GetPointCount();
        for ( size_t i=0; i<m_region.holes.size(); ++i )
            vertices += m_region.holes[ i ].GetPointCount();
        return vertices - 2 + 2 * (UInt32) m_region.holes.size();
    }

    /**
     *  The region as triangles in world coordinates, from the Float64 reference chain. The decomposition is written
     *  out here per kind, independent of the triangulators of the library: the L is two rectangles, the star a fan
     *  from its centre (it is star-shaped around the origin), the square with a hole four rectangles.
     */
    void GetWorldTriangles( GUCEF::vector< CTTriangle3D< Float64 > >& triangles ) const
    {
        triangles.clear();
        GUCEF::vector< P3 > corners;
        switch ( m_kind )
        {
            case POLY_L:
            {
                AddRectangle( corners, 0.0, 0.0, 4.0, 1.0 );
                AddRectangle( corners, 0.0, 1.0, 1.0, 3.0 );
                break;
            }
            case POLY_STAR:
            {
                const CTPolygon2D< T >& outer = m_region.outer;
                const UInt32 n = outer.GetPointCount();
                for ( UInt32 i=0; i<n; ++i )
                {
                    const CTPoint2D< T >& p = outer.GetPoint( i );
                    const CTPoint2D< T >& q = outer.GetPoint( ( i + 1 ) % n );
                    corners.push_back( P3( 0.0, 0.0, 0.0 ) );
                    corners.push_back( P3( (Float64) p.x, (Float64) p.y, 0.0 ) );
                    corners.push_back( P3( (Float64) q.x, (Float64) q.y, 0.0 ) );
                }
                break;
            }
            default:
            {
                AddRectangle( corners, -4.0, -4.0, 4.0, -2.0 );
                AddRectangle( corners, -4.0, 2.0, 4.0, 4.0 );
                AddRectangle( corners, -4.0, -2.0, -2.0, 2.0 );
                AddRectangle( corners, 2.0, -2.0, 4.0, 2.0 );
                break;
            }
        }
        const M4 world = ReferenceWorld( *m_object );
        for ( size_t i=0; i+2<corners.size(); i+=3 )
            triangles.push_back( CTTriangle3D< Float64 >( world.TransformPoint( corners[ i ] ), world.TransformPoint( corners[ i + 1 ] ), world.TransformPoint( corners[ i + 2 ] ) ) );
    }

    /**
     *  The convex pieces of the object in world coordinates: the triangulation of the library, which defines the
     *  pieces (the deepest contact is taken over them, so it depends on the decomposition). Returns false unless the
     *  triangulation is verified independently: the triangle count of a triangulation, the area of the hand-written
     *  decomposition, and every triangle centroid inside the region by ray crossing.
     */
    bool GetPieceTriangles( GUCEF::vector< CTTriangle3D< Float64 > >& triangles ) const
    {
        triangles.clear();
        GUCEF::vector< CTTriangle2D< T > > local;
        GUCEF::vector< UInt32 > indices;
        CTPolygonTriangulator< T > triangulator;
        TriangulatePolygon( m_region, local, triangulator, indices );
        GUCEF::vector< CTTriangle3D< Float64 > > reference;
        GetWorldTriangles( reference );
        const M4 world = ReferenceWorld( *m_object );
        Float64 referenceArea = 0.0;
        for ( size_t i=0; i<reference.size(); ++i )
            referenceArea += ( reference[ i ].b - reference[ i ].a ).Cross( reference[ i ].c - reference[ i ].a ).GetLength() * 0.5;
        Float64 area = 0.0;
        bool ok = (UInt32) local.size() == GetTriangulationCount();
        for ( size_t i=0; i<local.size(); ++i )
        {
            const P3 a( (Float64) local[ i ].a.x, (Float64) local[ i ].a.y, 0.0 );
            const P3 b( (Float64) local[ i ].b.x, (Float64) local[ i ].b.y, 0.0 );
            const P3 c( (Float64) local[ i ].c.x, (Float64) local[ i ].c.y, 0.0 );
            ok = ok && RegionContainsReference( m_region, ( a.x + b.x + c.x ) / 3.0, ( a.y + b.y + c.y ) / 3.0 );
            triangles.push_back( CTTriangle3D< Float64 >( world.TransformPoint( a ), world.TransformPoint( b ), world.TransformPoint( c ) ) );
            const CTTriangle3D< Float64 >& t = triangles.back();
            area += ( t.b - t.a ).Cross( t.c - t.a ).GetLength() * 0.5;
        }
        return ok && fabs( area - referenceArea ) <= 1.0e-9 * ( 1.0 + referenceArea );
    }

    private:

    /** The two triangles of the axis-aligned rectangle [ x0, x1 ] x [ y0, y1 ] in plane coordinates */
    static void AddRectangle( GUCEF::vector< P3 >& corners, Float64 x0, Float64 y0, Float64 x1, Float64 y1 )
    {
        corners.push_back( P3( x0, y0, 0.0 ) );
        corners.push_back( P3( x1, y0, 0.0 ) );
        corners.push_back( P3( x1, y1, 0.0 ) );
        corners.push_back( P3( x0, y0, 0.0 ) );
        corners.push_back( P3( x1, y1, 0.0 ) );
        corners.push_back( P3( x0, y1, 0.0 ) );
    }

    CPolygonHost( const CPolygonHost& src );                /**< not implemented */
    CPolygonHost& operator=( const CPolygonHost& src );     /**< not implemented */

    UInt32 m_kind;
    CTPolygonWithHoles2D< T > m_region;
    CTGeoSpace< T > m_root;
    CTGeoPlane< T > m_plane;
    CTGeoObjectBase< T >* m_object;
};

/*-------------------------------------------------------------------------*/

/** Moves a root node so that the world point 'from' lands on 'to' */
template < typename T >
static void
MoveRoot( CTGeoSpace< T >& root, const P3& from, const P3& to )
{
    CTTransform3D< T > transform = root.GetLocalTransform();
    transform.position = transform.position + ToV< T >( to - from );
    root.SetLocalTransform( transform );
}

/*-------------------------------------------------------------------------*/

/** The brute force answer of a polygon against another object: over every pair of world triangle and world convex piece */
struct SBrute
{
    Float64 distance;
    Float64 depth;          /**< the deepest penetration over the touching pairs; negative without one */
    bool intersects;

    SBrute( void ) : distance( 1.0e30 ), depth( -1.0 ), intersects( false ) {}
};

template < typename TB >
static void
BruteAccumulate( const CTTriangle3D< Float64 >& triangle, const TB& other, SBrute& result )
{
    P3 onA;
    P3 onB;
    if ( !ClosestPoints( triangle, other, onA, onB ) )
        return;
    const bool touching = Intersects( triangle, other );
    const Float64 d = touching ? 0.0 : ( onB - onA ).GetLength();
    if ( d < result.distance )
        result.distance = d;
    if ( touching )
    {
        result.intersects = true;
        CTContactManifold< Float64 > manifold;
        if ( ComputeContact( triangle, other, manifold ) && manifold.GetMaxDepth() > result.depth )
            result.depth = manifold.GetMaxDepth();
    }
}

template < typename TB >
static void
BruteAccumulate( const GUCEF::vector< CTTriangle3D< Float64 > >& triangles, const TB& other, SBrute& result )
{
    for ( size_t i=0; i<triangles.size(); ++i )
        BruteAccumulate( triangles[ i ], other, result );
}

/*-------------------------------------------------------------------------*/

/** The object to test the polygon against, placed so that its world centre lands on a target, with the world pieces for the brute force */
template < typename T >
class COther
{
    public:

    COther( UInt32 kind, UInt32 polygonKind, CRandom& r, bool similar, const P3& target )
        : m_kind( kind ), m_parent(), m_object( GUCEF_NULL ), m_host( GUCEF_NULL ), m_sphere(), m_capsule(), m_hull(), m_triangles(), m_pieceTriangles(), m_piecesVerified( true )
    {
        const CTTransform3D< T > parentTransform = RandomTransform< T >( r, 1.0, true, 0.6, 1.6, 0.0 );
        m_parent.SetLocalTransform( parentTransform );
        const P3 c = r.Point( 0.6 );
        const V3 e = r.Vector( 0.5 );
        const Float64 radius = r.Uniform( 0.3, 1.0 );
        const Q4 q = r.Rotation();
        const Float64 hx = r.Uniform( 0.3, 0.9 );
        const Float64 hy = r.Uniform( 0.3, 0.9 );
        const Float64 hz = r.Uniform( 0.3, 0.9 );
        P3 centre = c;
        switch ( kind )
        {
            case OTHER_SPHERE:
            {
                m_object = new CTGeoObject< CTSphere< T > >( CTSphere< T >( ToP< T >( c ), (T) radius ) );
                break;
            }
            case OTHER_BOX:
            {
                /* a box under a possibly non-uniform scale: its world image is a parallelepiped, which the brute force takes as a hull */
                if ( !similar )
                    m_parent.SetLocalTransform( RandomTransform< T >( r, 1.0, false, 0.6, 1.6, 0.0 ) );
                m_object = new CTGeoObject< CTOrientedBox3D< T > >( CTOrientedBox3D< T >( ToP< T >( c ), ToQ< T >( q ), ToV< T >( V3( hx, hy, hz ) ) ) );
                break;
            }
            case OTHER_CAPSULE:
            {
                m_object = new CTGeoObject< CTCapsule< T > >( CTCapsule< T >( ToP< T >( c ), ToP< T >( P3( c.ToPositionVector() + e ) ), (T) ( radius * 0.5 ) ) );
                centre = P3( c.ToPositionVector() + e * 0.5 );
                break;
            }
            default:
            {
                const CTTransform3D< T > rootTransform = RandomTransform< T >( r, 1.0, similar, 0.6, 1.6, 0.0 );
                const CTTransform3D< T > planeTransform = RandomTransform< T >( r, 1.0, true, 0.8, 1.25, 0.0 );
                m_host = new CPolygonHost< T >( polygonKind, rootTransform, planeTransform );
                m_object = &m_host->Object();
                break;
            }
        }
        if ( OTHER_POLYGON != kind )
            m_object->SetParent( &m_parent, false );

        /* move the whole chain so that the world centre of the object lands on the target */
        P3 worldCentre;
        if ( OTHER_POLYGON == kind )
            worldCentre = m_host->WorldCentre();
        else
            worldCentre = ReferenceWorld( *m_object ).TransformPoint( centre );
        if ( OTHER_POLYGON == kind )
            MoveRoot( m_host->Root(), worldCentre, target );
        else
            MoveRoot( m_parent, worldCentre, target );

        /* the world pieces from the Float64 reference chain */
        const M4 world = ReferenceWorld( *m_object );
        const Float64 scale = world.GetLinear().GetColumn( 0 ).GetLength();
        switch ( kind )
        {
            case OTHER_SPHERE:
            {
                const CTSphere< T >& s = static_cast< CTGeoObject< CTSphere< T > >* >( m_object )->GetShape();
                m_sphere = CTSphere< Float64 >( world.TransformPoint( Widen( s.centre ) ), (Float64) s.radius * scale );
                break;
            }
            case OTHER_BOX:
            {
                const CTOrientedBox3D< T >& b = static_cast< CTGeoObject< CTOrientedBox3D< T > >* >( m_object )->GetShape();
                const CTOrientedBox3D< Float64 > wide = Widen( b );
                P3 corners[ 8 ];
                for ( UInt32 k=0; k<8; ++k )
                {
                    const V3 local( ( k & 1 ) ? wide.halfExtents.x : -wide.halfExtents.x, ( k & 2 ) ? wide.halfExtents.y : -wide.halfExtents.y, ( k & 4 ) ? wide.halfExtents.z : -wide.halfExtents.z );
                    corners[ k ] = world.TransformPoint( P3( wide.centre.ToPositionVector() + wide.rotation.Rotate( local ) ) );
                }
                m_hull = CTConvexHull3D< Float64 >( corners, 8 );
                break;
            }
            case OTHER_CAPSULE:
            {
                const CTCapsule< T >& cap = static_cast< CTGeoObject< CTCapsule< T > >* >( m_object )->GetShape();
                m_capsule = CTCapsule< Float64 >( world.TransformPoint( Widen( cap.start ) ), world.TransformPoint( Widen( cap.end ) ), (Float64) cap.radius * scale );
                break;
            }
            default:
            {
                m_host->GetWorldTriangles( m_triangles );
                m_piecesVerified = m_host->GetPieceTriangles( m_pieceTriangles );
                break;
            }
        }
    }

    ~COther()
    {
        if ( GUCEF_NULL != m_host )
            delete m_host;
        else
            delete m_object;
    }

    CTGeoObjectBase< T >& Object( void ) { return *m_object; }

    /** False when the other object is a polygon whose library triangulation failed the independent check */
    bool ArePiecesVerified( void ) const { return m_piecesVerified; }

    /** The brute force against the given triangles; a polygon other uses its hand decomposition, or with 'pieces' its convex pieces */
    void Brute( const GUCEF::vector< CTTriangle3D< Float64 > >& triangles, bool pieces, SBrute& result ) const
    {
        const GUCEF::vector< CTTriangle3D< Float64 > >& otherTriangles = pieces ? m_pieceTriangles : m_triangles;
        switch ( m_kind )
        {
            case OTHER_SPHERE: BruteAccumulate( triangles, m_sphere, result ); break;
            case OTHER_BOX: BruteAccumulate( triangles, m_hull, result ); break;
            case OTHER_CAPSULE: BruteAccumulate( triangles, m_capsule, result ); break;
            default:
            {
                for ( size_t i=0; i<otherTriangles.size(); ++i )
                    BruteAccumulate( triangles, otherTriangles[ i ], result );
                break;
            }
        }
    }

    private:

    COther( const COther& src );                /**< not implemented */
    COther& operator=( const COther& src );     /**< not implemented */

    UInt32 m_kind;
    CTGeoSpace< T > m_parent;
    CTGeoObjectBase< T >* m_object;
    CPolygonHost< T >* m_host;
    CTSphere< Float64 > m_sphere;
    CTCapsule< Float64 > m_capsule;
    CTConvexHull3D< Float64 > m_hull;
    GUCEF::vector< CTTriangle3D< Float64 > > m_triangles;
    GUCEF::vector< CTTriangle3D< Float64 > > m_pieceTriangles;
    bool m_piecesVerified;
};

/*-------------------------------------------------------------------------*/

/**
 *  Concave polygons on planes under rotated and scaled parents against spheres, boxes, capsules and other polygons:
 *  distance, nearest points, intersection and contact equal the brute force over the world-space triangles
 */
template < typename T >
static bool
PolygonsAgainstObjects( UInt64 seed, UInt32 count, Float64 tolerance, Float64 contactTolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    UInt32 overlapping = 0;
    UInt32 separated = 0;
    UInt32 contactsChecked = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 polygonKind = i % POLY_COUNT;
        const UInt32 otherKind = ( i / POLY_COUNT ) % OTHER_COUNT;
        const UInt32 otherPolygonKind = ( i / 3 ) % POLY_COUNT;
        const bool similar = 0 == ( i / ( POLY_COUNT * OTHER_COUNT ) ) % 2;
        const CTTransform3D< T > rootTransform = RandomTransform< T >( r, 1.0, similar, 0.6, 1.6, 0.0 );
        const CTTransform3D< T > planeTransform = RandomTransform< T >( r, 1.0, true, 0.8, 1.25, 0.0 );
        CPolygonHost< T > host( polygonKind, rootTransform, planeTransform );
        const P3 centre = host.WorldCentre();
        const V3 offset = r.Vector( 2.6 );
        COther< T > other( otherKind, otherPolygonKind, r, similar, P3( centre.ToPositionVector() + offset ) );

        /*
         *  distance and intersection do not depend on the decomposition: the brute force runs over the hand-written one;
         *  the deepest contact is taken over the convex pieces, so its brute force runs over the pieces (the library
         *  triangulation, verified independently); the distances of both must agree
         */
        GUCEF::vector< CTTriangle3D< Float64 > > triangles;
        host.GetWorldTriangles( triangles );
        GUCEF::vector< CTTriangle3D< Float64 > > pieces;
        const bool piecesVerified = host.GetPieceTriangles( pieces ) && other.ArePiecesVerified();
        SBrute brute;
        other.Brute( triangles, false, brute );
        SBrute brutePieces;
        other.Brute( pieces, true, brutePieces );
        brute.depth = brutePieces.depth;

        CTGeoObjectBase< T >& a = host.Object();
        CTGeoObjectBase< T >& b = other.Object();
        bool ok = piecesVerified && fabs( brute.distance - brutePieces.distance ) <= 1.0e-9 && brute.intersects == brutePieces.intersects;
        ok = ok && !a.IsConvex() && a.GetConvexPieceCount() == host.GetTriangulationCount() && triangles.size() >= 4;
        ok = ok && IsGeoPairQueryOffered( a, b, GEOPAIRQUERY_CLOSESTPOINTS ) && IsGeoPairQueryOffered( a, b, GEOPAIRQUERY_INTERSECTS ) && IsGeoPairQueryOffered( a, b, GEOPAIRQUERY_CONTACT );
        ok = ok && IsGeoPairQueryOffered( b, a, GEOPAIRQUERY_CLOSESTPOINTS ) && IsGeoPairQueryOffered( b, a, GEOPAIRQUERY_CONTACT );

        /* nearest points: the minimum over the triangles */
        CTPoint3D< T > onA;
        CTPoint3D< T > onB;
        T distance = T( 0 );
        TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
        ok = ok && CTGeoObjectQueries< T >::ClosestPoints( a, b, onA, onB, distance, path ) && GEOPAIRPATH_SUPPORT == path;
        const Float64 size = 1.0 + Widen( onA ).ToPositionVector().GetLength();
        ok = ok && fabs( (Float64) distance - brute.distance ) <= tolerance * size;
        ok = ok && fabs( ( Widen( onB ) - Widen( onA ) ).GetLength() - (Float64) distance ) <= tolerance * size;
        ok = ok && (Float64) a.GetDistance( onA ) <= tolerance * size && (Float64) b.GetDistance( onB ) <= tolerance * size;
        ok = ok && Distance( a, b ) == distance;
        ok = ok && fabs( (Float64) Distance( b, a ) - (Float64) distance ) <= tolerance * size;

        /* any: the intersection test, where the brute force is clear of the touching boundary */
        const bool intersects = Intersects( a, b );
        const bool clearlyOverlapping = brute.intersects && brute.depth > 1.0e-3;
        const bool clearlySeparated = brute.distance > 1.0e-3;
        if ( clearlyOverlapping )
            ok = ok && intersects && Intersects( b, a );
        if ( clearlySeparated )
            ok = ok && !intersects && !Intersects( b, a );

        /* deepest: the contact */
        CTContactManifold< T > manifold;
        TGeoPairPath contactPath = GEOPAIRPATH_NOT_OFFERED;
        const bool touching = CTGeoObjectQueries< T >::ComputeContact( a, b, manifold, contactPath );
        ok = ok && GEOPAIRPATH_SUPPORT == contactPath;
        if ( clearlyOverlapping )
        {
            ++overlapping;
            ok = ok && touching && manifold.GetPointCount() > 0;
            if ( touching && manifold.GetPointCount() > 0 )
            {
                ++contactsChecked;
                ok = ok && fabs( (Float64) manifold.GetMaxDepth() - brute.depth ) <= contactTolerance * size;
                const SContactPoint< T >& point = manifold.GetPoint( 0 );
                const V3 n = Widen( manifold.normal );
                ok = ok && fabs( n.GetLength() - 1.0 ) <= tolerance * 10.0;
                ok = ok && ( Widen( point.pointOnA ) - Widen( point.pointOnB ) - n * (Float64) point.depth ).GetLength() <= contactTolerance * size * 10.0;
            }
        }
        if ( clearlySeparated )
        {
            ++separated;
            ok = ok && !touching && 0 == manifold.GetPointCount();
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoPolygonQueries: case " << i << " " << PolygonKindNames[ polygonKind ] << " against " << OtherKindNames[ otherKind ]
                             << ( similar ? " (similar)" : " (affine)" ) << ": distance " << (Float64) distance << " brute " << brute.distance << ", intersects " << ( intersects ? 1 : 0 )
                             << " brute " << ( brute.intersects ? 1 : 0 ) << ", touching " << ( touching ? 1 : 0 ) << " depth " << (Float64) manifold.GetMaxDepth() << " brute depth " << brute.depth
                             << ", pieces " << a.GetConvexPieceCount() << "/" << host.GetTriangulationCount() << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoPolygonQueries: polygons against objects (kind size " << (UInt32) sizeof( T ) << "), seed " << seed << ": " << count << " cases, " << overlapping
               << " overlapping, " << separated << " separated, " << contactsChecked << " contacts compared, " << bad << " bad" );
    return 0 == bad && overlapping > count / 20 && separated > count / 20 && contactsChecked > count / 20;
}

/*-------------------------------------------------------------------------*/

/** A root node rotated about ( 1, 2, 3 ) and scaled by 1.5, with a plane in it: the setting of the hand-checked cases */
template < typename T >
class CHandSetting
{
    public:

    CHandSetting( UInt32 kind )
        : m_scale( 1.5 )
        , m_root( CTTransform3D< T >( CTVector3D< T >( T( 1 ), T( -2 ), T( 3 ) ), RotationAboutDiagonal(), CTVector3D< T >( T( 1.5 ), T( 1.5 ), T( 1.5 ) ) ) )
        , m_plane()
        , m_region( MakeRegion< T >( kind ) )
        , m_simple( m_region.outer )
        , m_holed( m_region )
    {
        m_plane.SetParent( &m_root, false );
        m_simple.SetParent( &m_plane, false );
        m_holed.SetParent( &m_plane, false );
    }

    static CTQuaternion< T > RotationAboutDiagonal( void )
    {
        CTDirectionVector3D< T > axis;
        CTDirectionVector3D< T >::TryCreate( CTVector3D< T >( T( 1 ), T( 2 ), T( 3 ) ), axis );
        return CTQuaternion< T >::FromAxisAngle( axis, CTRadian< T >( T( 0.7 ) ) );
    }

    /** The polygon object of the kind (the one with holes for POLY_HOLES) */
    CTGeoObjectBase< T >& Polygon( UInt32 kind ) { return POLY_HOLES == kind ? static_cast< CTGeoObjectBase< T >& >( m_holed ) : static_cast< CTGeoObjectBase< T >& >( m_simple ); }

    /** The world point of the plane coordinates, lifted along the world normal by 'height' (world units) */
    CTPoint3D< T > World( Float64 x, Float64 y, Float64 height )
    {
        const CTPoint3D< T > p = m_plane.FromPlane( CTPoint2D< T >( (T) x, (T) y ) );
        return CTPoint3D< T >( p.ToPositionVector() + m_plane.GetPlane().normal.AsVector() * (T) height );
    }

    CTVector3D< T > Normal( void ) { return m_plane.GetPlane().normal.AsVector(); }

    Float64 m_scale;
    CTGeoSpace< T > m_root;
    CTGeoPlane< T > m_plane;
    CTPolygonWithHoles2D< T > m_region;
    CTGeoObject2D< CTPolygon2D< T > > m_simple;
    CTGeoObject2D< CTPolygonWithHoles2D< T > > m_holed;
};

/*-------------------------------------------------------------------------*/

/**
 *  A point in the hole is not contained, objects in the hole of the polygon or in the notch of the L do not touch it
 *  (their distances are the analytic ones, scaled by the parent) and objects over the ring do, where the convex hull would say otherwise
 */
template < typename T >
static bool
HolesAndNotches( Float64 tolerance )
{
    bool ok = true;
    {
        CHandSetting< T > setting( POLY_HOLES );
        const Float64 s = setting.m_scale;
        CTGeoObjectBase< T >& polygon = setting.Polygon( POLY_HOLES );
        const CTPoint3D< T > holeCentre = setting.World( 0.0, 0.0, 0.0 );
        ok = ok && !polygon.IsConvex() && polygon.GetConvexPieceCount() >= 8;
        ok = ok && !polygon.Contains( holeCentre ) && polygon.Contains( setting.World( 3.0, 0.0, 0.0 ) ) && polygon.Contains( setting.World( -3.0, 3.0, 0.0 ) );
        ok = ok && !polygon.Contains( setting.World( 3.0, 0.0, 0.1 ) ) && !polygon.Contains( setting.World( 5.0, 0.0, 0.0 ) );
        ok = ok && fabs( (Float64) polygon.GetDistance( holeCentre ) - 2.0 * s ) <= tolerance * 10.0;
        ok = ok && fabs( ( Widen( polygon.ClosestPoint( holeCentre ) ) - Widen( holeCentre ) ).GetLength() - 2.0 * s ) <= tolerance * 10.0;

        /* a sphere in the hole: the hole is empty, the nearest ring point is the edge of the hole */
        CTGeoObject< CTSphere< T > > sphere( CTSphere< T >( holeCentre, (T) ( 0.5 * s ) ) );
        TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
        ok = ok && IsGeoPairQueryOffered( polygon, sphere, GEOPAIRQUERY_CONTACT ) && !Intersects( polygon, sphere ) && !Intersects( sphere, polygon );
        ok = ok && fabs( (Float64) Distance( polygon, sphere ) - 1.5 * s ) <= tolerance * 10.0 && fabs( (Float64) Distance( sphere, polygon ) - 1.5 * s ) <= tolerance * 10.0;
        CTContactManifold< T > manifold;
        ok = ok && !ComputeContact( polygon, sphere, manifold ) && 0 == manifold.GetPointCount();
        /* raised over the hole: sqrt( h^2 + ( 2 s )^2 ) - r */
        sphere.SetShape( CTSphere< T >( setting.World( 0.0, 0.0, 1.0 * s ), (T) ( 0.5 * s ) ) );
        ok = ok && fabs( (Float64) Distance( polygon, sphere ) - ( sqrt( s * s + 4.0 * s * s ) - 0.5 * s ) ) <= tolerance * 10.0 && !Intersects( polygon, sphere );
        /* over the ring */
        sphere.SetShape( CTSphere< T >( setting.World( 3.0, 0.0, 0.2 * s ), (T) ( 0.5 * s ) ) );
        ok = ok && Intersects( polygon, sphere ) && ComputeContact( polygon, sphere, manifold ) && manifold.GetPointCount() > 0 && Distance( polygon, sphere ) == T( 0 );

        /* a box in the hole, then over the ring */
        CTGeoObject< CTAxisAlignedBox3D< T > > box( CTAxisAlignedBox3D< T >::FromCentreHalfExtents( holeCentre, CTVector3D< T >( (T) ( 0.4 * s ), (T) ( 0.4 * s ), (T) ( 0.4 * s ) ) ) );
        const Float64 boxDistance = (Float64) Distance( polygon, box );
        ok = ok && !Intersects( polygon, box ) && boxDistance >= 2.0 * s - 0.4 * s * 1.7321 - tolerance * 10.0 && boxDistance <= 2.0 * s - 0.4 * s + tolerance * 10.0;
        box.SetShape( CTAxisAlignedBox3D< T >::FromCentreHalfExtents( setting.World( 3.0, 0.0, 0.0 ), CTVector3D< T >( (T) ( 0.4 * s ), (T) ( 0.4 * s ), (T) ( 0.4 * s ) ) ) );
        ok = ok && Intersects( polygon, box ) && ComputeContact( polygon, box, manifold ) && manifold.GetPointCount() > 0;

        /* a capsule that runs through the hole without touching the ring, and a fat one that does */
        const CTPoint3D< T > below = setting.World( 0.0, 0.0, -3.0 * s );
        const CTPoint3D< T > above = setting.World( 0.0, 0.0, 3.0 * s );
        CTGeoObject< CTCapsule< T > > capsule( CTCapsule< T >( below, above, (T) ( 0.5 * s ) ) );
        ok = ok && !Intersects( polygon, capsule ) && fabs( (Float64) Distance( polygon, capsule ) - 1.5 * s ) <= tolerance * 10.0;
        capsule.SetShape( CTCapsule< T >( below, above, (T) ( 2.5 * s ) ) );
        ok = ok && Intersects( polygon, capsule ) && ComputeContact( polygon, capsule, manifold ) && manifold.GetPointCount() > 0;

        /* a convex polygon on the same plane, coplanar: in the hole it is clear of the ring, overlapping the ring it touches */
        const CTPoint2D< T > square[ 4 ] = { CTPoint2D< T >( T( -0.5 ), T( -0.5 ) ), CTPoint2D< T >( T( 0.5 ), T( -0.5 ) ), CTPoint2D< T >( T( 0.5 ), T( 0.5 ) ), CTPoint2D< T >( T( -0.5 ), T( 0.5 ) ) };
        CTGeoObject2D< CTPolygon2D< T > > small( CTPolygon2D< T >( square, 4 ) );
        small.SetParent( &setting.m_plane, false );
        ok = ok && small.IsConvex() && 0 == small.GetConvexPieceCount();
        ok = ok && !Intersects( polygon, small ) && !Intersects( small, polygon ) && fabs( (Float64) Distance( polygon, small ) - 1.5 * s ) <= tolerance * 10.0;
        ok = ok && !ComputeContact( polygon, small, manifold ) && 0 == manifold.GetPointCount();
        small.SetLocalTransform2D( CTTransform2D< T >( CTVector2D< T >( T( 2 ), T( 0 ) ), CTRadian< T >( T( 0 ) ), CTVector2D< T >::One() ) );
        ok = ok && Intersects( polygon, small ) && Intersects( small, polygon ) && Distance( polygon, small ) == T( 0 );
        ok = ok && ComputeContact( polygon, small, manifold ) && manifold.GetPointCount() > 0;
        if ( manifold.GetPointCount() > 0 )
        {
            /* the contact of coplanar flat objects is in their plane: the normal lies in it and the points are on it */
            ok = ok && fabs( (Float64) manifold.normal.Dot( setting.Normal() ) ) <= tolerance * 100.0;
            ok = ok && (Float64) manifold.GetMaxDepth() > 0.0 && (Float64) manifold.GetMaxDepth() <= 1.0 * s + tolerance * 100.0;
            ok = ok && fabs( (Float64) setting.m_plane.GetPlane().GetSignedDistance( manifold.GetPoint( 0 ).pointOnA ) ) <= tolerance * 100.0;
        }
    }
    {
        /* the L: a sphere in the notch, where its hull would have it touch */
        CHandSetting< T > setting( POLY_L );
        const Float64 s = setting.m_scale;
        CTGeoObjectBase< T >& polygon = setting.Polygon( POLY_L );
        ok = ok && !polygon.IsConvex() && !polygon.Contains( setting.World( 3.0, 2.0, 0.0 ) ) && polygon.Contains( setting.World( 0.5, 2.0, 0.0 ) ) && polygon.Contains( setting.World( 3.0, 0.5, 0.0 ) );
        CTGeoObject< CTSphere< T > > sphere( CTSphere< T >( setting.World( 3.0, 1.5, 0.0 ), (T) ( 0.4 * s ) ) );
        ok = ok && !Intersects( polygon, sphere ) && fabs( (Float64) Distance( polygon, sphere ) - 0.1 * s ) <= tolerance * 10.0;
        sphere.SetShape( CTSphere< T >( setting.World( 3.0, 1.5, 0.0 ), (T) ( 0.6 * s ) ) );
        CTContactManifold< T > manifold;
        ok = ok && Intersects( polygon, sphere ) && ComputeContact( polygon, sphere, manifold ) && manifold.GetPointCount() > 0 && fabs( (Float64) manifold.GetMaxDepth() - 0.1 * s ) <= tolerance * 100.0;
        /* the normal points from the polygon towards the sphere: away from the bar, up the notch */
        ok = ok && fabs( (Float64) manifold.normal.Dot( setting.Normal() ) ) <= 0.05;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Setting the shape clears the cache: a sphere in the notch of an L, a U with an arm there, the L again, and a convex polygon */
template < typename T >
static bool
ShapeChangeInvalidatesCache( Float64 tolerance )
{
    typedef CTPoint2D< T > TP2;
    CHandSetting< T > setting( POLY_L );
    const Float64 s = setting.m_scale;
    CTGeoObject2D< CTPolygon2D< T > > polygon( MakeRegion< T >( POLY_L ).outer );
    polygon.SetParent( &setting.m_plane, false );
    CTGeoObject< CTSphere< T > > sphere( CTSphere< T >( setting.World( 3.5, 2.0, 0.0 ), (T) ( 0.4 * s ) ) );

    GUCEF::vector< UInt32 > indices;
    GUCEF::vector< CTTriangle2D< T > > triangles;
    CTPolygonTriangulator< T > triangulator;

    /* the L: the sphere is in the notch, 0.6 s from the floor */
    TriangulatePolygon( MakeRegion< T >( POLY_L ), triangles, triangulator, indices );
    bool ok = polygon.GetConvexPieceCount() == (UInt32) triangles.size() && 4 == triangles.size();
    ok = ok && !Intersects( polygon, sphere ) && fabs( (Float64) Distance( polygon, sphere ) - 0.6 * s ) <= tolerance * 10.0;

    /* a U with its right arm at the sphere: 6 triangles, the pair now touches */
    const TP2 u[ 8 ] = { TP2( T( 0 ), T( 0 ) ), TP2( T( 4 ), T( 0 ) ), TP2( T( 4 ), T( 3 ) ), TP2( T( 3 ), T( 3 ) ), TP2( T( 3 ), T( 1 ) ), TP2( T( 1 ), T( 1 ) ), TP2( T( 1 ), T( 3 ) ), TP2( T( 0 ), T( 3 ) ) };
    const CTPolygon2D< T > uShape( u, 8 );
    CTPolygonWithHoles2D< T > uRegion( uShape );
    TriangulatePolygon( uRegion, triangles, triangulator, indices );
    polygon.SetShape( uShape );
    ok = ok && 6 == triangles.size() && polygon.GetConvexPieceCount() == (UInt32) triangles.size() && !polygon.IsConvex();
    ok = ok && Intersects( polygon, sphere ) && Distance( polygon, sphere ) == T( 0 );
    CTContactManifold< T > manifold;
    ok = ok && ComputeContact( polygon, sphere, manifold ) && manifold.GetPointCount() > 0;

    /* the sphere in the notch of the U */
    sphere.SetShape( CTSphere< T >( setting.World( 2.0, 2.0, 0.0 ), (T) ( 0.4 * s ) ) );
    ok = ok && !Intersects( polygon, sphere ) && fabs( (Float64) Distance( polygon, sphere ) - 0.6 * s ) <= tolerance * 10.0;

    /* back to the L: 4 triangles again, and the same notch answer as the first time */
    sphere.SetShape( CTSphere< T >( setting.World( 3.5, 2.0, 0.0 ), (T) ( 0.4 * s ) ) );
    polygon.SetShape( MakeRegion< T >( POLY_L ).outer );
    ok = ok && 4 == polygon.GetConvexPieceCount() && !Intersects( polygon, sphere ) && fabs( (Float64) Distance( polygon, sphere ) - 0.6 * s ) <= tolerance * 10.0;

    /* a convex shape has no pieces and answers through its support: the filled rectangle contains the sphere's place */
    const TP2 filled[ 4 ] = { TP2( T( 0 ), T( 0 ) ), TP2( T( 4 ), T( 0 ) ), TP2( T( 4 ), T( 3 ) ), TP2( T( 0 ), T( 3 ) ) };
    polygon.SetShape( CTPolygon2D< T >( filled, 4 ) );
    ok = ok && polygon.IsConvex() && 0 == polygon.GetConvexPieceCount() && Intersects( polygon, sphere );

    /* an empty polygon has no points and no pieces: the pair is not offered */
    polygon.SetShape( CTPolygon2D< T >() );
    ok = ok && polygon.IsEmpty() && 0 == polygon.GetConvexPieceCount() && !IsGeoPairQueryOffered( polygon, sphere, GEOPAIRQUERY_CLOSESTPOINTS ) && !Intersects( polygon, sphere );

    /* and the L once more after the empty one */
    polygon.SetShape( MakeRegion< T >( POLY_L ).outer );
    ok = ok && 4 == polygon.GetConvexPieceCount() && !Intersects( polygon, sphere );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A box outer contains a polygon inner exactly when it contains the vertices of the polygon; a polygon is not offered as the outer object */
template < typename T >
static bool
ContainmentOfPolygons( UInt64 seed, UInt32 count )
{
    CRandom r( seed );
    UInt32 bad = 0;
    UInt32 contained = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 polygonKind = i % POLY_COUNT;
        const CTTransform3D< T > rootTransform = RandomTransform< T >( r, 1.0, 0 == i % 2, 0.6, 1.6, 0.0 );
        const CTTransform3D< T > planeTransform = RandomTransform< T >( r, 1.0, true, 0.8, 1.25, 0.0 );
        CPolygonHost< T > host( polygonKind, rootTransform, planeTransform );
        GUCEF::vector< CTTriangle3D< Float64 > > triangles;
        host.GetWorldTriangles( triangles );
        const P3 centre = host.WorldCentre();
        const V3 offset = r.Vector( 0.6 );
        const Float64 half = r.Uniform( 2.5, 6.0 );
        const P3 boxCentre( centre.ToPositionVector() + offset );
        CTGeoObject< CTAxisAlignedBox3D< T > > box( CTAxisAlignedBox3D< T >::FromCentreHalfExtents( ToP< T >( boxCentre ), ToV< T >( V3( half, half, half ) ) ) );
        const CTAxisAlignedBox3D< Float64 > wide = Widen( box.GetShape() );
        bool expected = true;
        Float64 margin = 1.0e30;
        for ( size_t t=0; t<triangles.size(); ++t )
        {
            const P3 corners[ 3 ] = { triangles[ t ].a, triangles[ t ].b, triangles[ t ].c };
            for ( UInt32 k=0; k<3; ++k )
            {
                const Float64 inside = std::min( std::min( std::min( corners[ k ].x - wide.min.x, wide.max.x - corners[ k ].x ), std::min( corners[ k ].y - wide.min.y, wide.max.y - corners[ k ].y ) ),
                                                 std::min( corners[ k ].z - wide.min.z, wide.max.z - corners[ k ].z ) );
                if ( inside < 0.0 )
                    expected = false;
                margin = std::min( margin, fabs( inside ) );
            }
        }
        contained += expected ? 1 : 0;
        bool ok = IsGeoPairQueryOffered( box, host.Object(), GEOPAIRQUERY_CONTAINS ) && !IsGeoPairQueryOffered( host.Object(), box, GEOPAIRQUERY_CONTAINS );
        TGeoPairPath path = GEOPAIRPATH_ANALYTIC;
        ok = ok && !CTGeoObjectQueries< T >::Contains( host.Object(), box, path ) && GEOPAIRPATH_NOT_OFFERED == path;
        if ( margin > 1.0e-3 )
            ok = ok && expected == Contains( box, host.Object() );
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoPolygonQueries: containment case " << i << " " << PolygonKindNames[ polygonKind ] << " expected " << ( expected ? 1 : 0 ) << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoPolygonQueries: boxes containing polygons, seed " << seed << ": " << count << " cases, " << contained << " contained, " << bad << " bad" );
    return 0 == bad && contained > count / 10 && contained < count - count / 10;
}

/*-------------------------------------------------------------------------*/

/** An unbounded plane against a polygon: the distance from the plane to the nearest vertex, 0 when the vertices lie on both sides */
template < typename T >
static bool
PolygonsAgainstPlanes( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    UInt32 touching = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 polygonKind = i % POLY_COUNT;
        const CTTransform3D< T > rootTransform = RandomTransform< T >( r, 1.0, 0 == i % 2, 0.6, 1.6, 0.0 );
        const CTTransform3D< T > planeTransform = RandomTransform< T >( r, 1.0, true, 0.8, 1.25, 0.0 );
        CPolygonHost< T > host( polygonKind, rootTransform, planeTransform );
        GUCEF::vector< CTTriangle3D< Float64 > > triangles;
        host.GetWorldTriangles( triangles );
        const P3 centre = host.WorldCentre();
        const V3 offset = r.Vector( 3.0 );
        CTTransform3D< T > planePlacement = RandomTransform< T >( r, 0.0, true, 0.8, 1.25, 0.0 );
        planePlacement.position = ToV< T >( centre.ToPositionVector() + offset );
        CTGeoPlane< T > plane( planePlacement );
        const CTPlane< Float64 > world = Widen( plane.GetPlane() );
        Float64 lowest = 1.0e30;
        Float64 highest = -1.0e30;
        for ( size_t t=0; t<triangles.size(); ++t )
        {
            const P3 corners[ 3 ] = { triangles[ t ].a, triangles[ t ].b, triangles[ t ].c };
            for ( UInt32 k=0; k<3; ++k )
            {
                const Float64 h = world.GetSignedDistance( corners[ k ] );
                lowest = std::min( lowest, h );
                highest = std::max( highest, h );
            }
        }
        const Float64 expected = lowest > 0.0 ? lowest : ( highest < 0.0 ? -highest : 0.0 );
        CTGeoObjectBase< T >& polygon = host.Object();
        bool ok = IsGeoPairQueryOffered( plane, polygon, GEOPAIRQUERY_CLOSESTPOINTS ) && IsGeoPairQueryOffered( polygon, plane, GEOPAIRQUERY_INTERSECTS ) && !IsGeoPairQueryOffered( plane, polygon, GEOPAIRQUERY_CONTACT );
        const Float64 size = 1.0 + fabs( lowest ) + fabs( highest );
        ok = ok && fabs( (Float64) Distance( plane, polygon ) - expected ) <= tolerance * size && fabs( (Float64) Distance( polygon, plane ) - expected ) <= tolerance * size;
        CTPoint3D< T > onPlane;
        CTPoint3D< T > onPolygon;
        ok = ok && ClosestPoints( plane, polygon, onPlane, onPolygon ) && fabs( world.GetSignedDistance( Widen( onPlane ) ) ) <= tolerance * size && (Float64) polygon.GetDistance( onPolygon ) <= tolerance * size;
        if ( expected > 1.0e-3 )
            ok = ok && !Intersects( plane, polygon ) && !Intersects( polygon, plane );
        if ( lowest < -1.0e-3 && highest > 1.0e-3 )
        {
            ++touching;
            ok = ok && Intersects( plane, polygon ) && Intersects( polygon, plane );
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoPolygonQueries: plane case " << i << " " << PolygonKindNames[ polygonKind ] << " expected distance " << expected << " got " << (Float64) Distance( plane, polygon ) << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoPolygonQueries: planes against polygons, seed " << seed << ": " << count << " cases, " << touching << " crossing, " << bad << " bad" );
    return 0 == bad && touching > count / 10;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeoPolygonQueriesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeoPolygonQueries" );

    GUCEF_TESTFW_TESTCASE( "GeoPolygonQueries: L, star and holed polygons on rotated, scaled planes against spheres, boxes, capsules and polygons equal the brute force over the world triangles" )
    try
    {
        ASSERT_TRUE( PolygonsAgainstObjects< Float32 >( 9501, ScaledCount( 600 ), 5.0e-3, 5.0e-2 ) );
        ASSERT_TRUE( PolygonsAgainstObjects< Float64 >( 9502, ScaledCount( 600 ), 1.0e-6, 1.0e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoPolygonQueries: a point in the hole is not contained; objects in the hole or the notch do not touch, objects over the ring do (analytic distances)" )
    try
    {
        ASSERT_TRUE( HolesAndNotches< Float32 >( 1.0e-4 ) );
        ASSERT_TRUE( HolesAndNotches< Float64 >( 1.0e-9 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoPolygonQueries: setting the shape clears the triangle cache (L, U, L, convex, empty, L)" )
    try
    {
        ASSERT_TRUE( ShapeChangeInvalidatesCache< Float32 >( 1.0e-4 ) );
        ASSERT_TRUE( ShapeChangeInvalidatesCache< Float64 >( 1.0e-9 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoPolygonQueries: boxes contain polygons exactly when they contain their vertices; a polygon is not offered as outer object; unbounded planes against polygons" )
    try
    {
        ASSERT_TRUE( ContainmentOfPolygons< Float32 >( 9601, ScaledCount( 300 ) ) );
        ASSERT_TRUE( ContainmentOfPolygons< Float64 >( 9602, ScaledCount( 300 ) ) );
        ASSERT_TRUE( PolygonsAgainstPlanes< Float32 >( 9701, ScaledCount( 300 ), 1.0e-4 ) );
        ASSERT_TRUE( PolygonsAgainstPlanes< Float64 >( 9702, ScaledCount( 300 ), 1.0e-9 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
