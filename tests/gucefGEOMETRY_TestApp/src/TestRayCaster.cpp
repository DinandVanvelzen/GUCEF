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
#include <algorithm>

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_GEOSPACES_H
#include "gucefGEOMETRY_geospaces.h"
#define GUCEF_GEOMETRY_GEOSPACES_H
#endif /* GUCEF_GEOMETRY_GEOSPACES_H ? */

#ifndef GUCEF_GEOMETRY_CTRAYCASTER_H
#include "gucefGEOMETRY_CTRayCaster.h"
#define GUCEF_GEOMETRY_CTRAYCASTER_H
#endif /* GUCEF_GEOMETRY_CTRAYCASTER_H ? */

#ifndef GUCEF_GEOMETRY_MESHQUERIES_H
#include "gucefGEOMETRY_MeshQueries.h"
#define GUCEF_GEOMETRY_MESHQUERIES_H
#endif /* GUCEF_GEOMETRY_MESHQUERIES_H ? */

#include "TestMeshReference.h"
#include "TestConvexReference.h"
#include "TestRayCaster.h"

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
using namespace MeshReference;
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Runs a function on its own thread */
class CRayCasterTestThread : public MT::CActiveObject
{
    public:

    typedef void ( *TFunction )( void* argument );

    CRayCasterTestThread( TFunction function, void* argument ) : MT::CActiveObject(), m_function( function ), m_argument( argument ), m_done( false ) {}

    virtual bool OnThreadStart( void* /*taskdata*/ ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* /*taskdata*/ ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_function( m_argument );
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitUntilDone( UInt32 timeoutInMs )
    {
        UInt32 waited = 0;
        while ( ( !m_done.Load( MT::MEMORDER_ACQUIRE ) || IsActive() ) && waited < timeoutInMs )
        {
            MT::PrecisionDelay( 1 );
            ++waited;
        }
        return m_done.Load( MT::MEMORDER_ACQUIRE ) && !IsActive();
    }

    private:

    TFunction m_function;
    void* m_argument;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------*/

/** Rejects the objects with one user id */
template < typename T >
class CRejectUserId : public CTGeoRayFilter< T >
{
    public:

    explicit CRejectUserId( UInt64 id ) : m_id( id ) {}

    virtual bool Accept( const CTGeoObjectBase< T >& object ) const GUCEF_VIRTUAL_OVERRIDE { return object.GetUserId() != m_id; }

    private:

    UInt64 m_id;
};

/*-------------------------------------------------------------------------*/

/** A random placement: uniform scale (similarity) or, when 'shear' allows, a non-uniform scale */
template < typename T >
static MATH::CTTransform3D< T >
RandomPlacement( CRandom& random, Float64 range, bool nonUniform )
{
    const V3 position = random.Vector( range );
    const Q4 rotation = random.Rotation();
    const Float64 s = random.Uniform( 0.5, 2.0 );
    const Float64 sy = nonUniform ? random.Uniform( 0.5, 2.0 ) : s;
    const Float64 sz = nonUniform ? random.Uniform( 0.5, 2.0 ) : s;
    return MATH::CTTransform3D< T >( ToV< T >( position ), ToQ< T >( rotation ), CTVector3D< T >( (T) s, (T) sy, (T) sz ) );
}

/*-------------------------------------------------------------------------*/

/** A scene of linked objects of mixed shapes (and meshes) owned by the test */
template < typename T >
struct SScene
{
    typedef CTGeoObjectBase< T > TObject;
    typedef CTGeoTriangleMesh< T > TMeshObject;

    GUCEF::vector< TObject* > objects;
    GUCEF::vector< bool > isMesh;
    typename TMeshObject::TMeshPtr boxMesh;
    typename TMeshObject::TBvhPtr boxBvh;

    SScene( void ) : objects(), isMesh(), boxMesh(), boxBvh() {}

    ~SScene()
    {
        for ( size_t i=0; i<objects.size(); ++i )
            GUCEF_DELETE objects[ i ];
    }

    void Build( CRandom& random, UInt32 count, Float64 range, bool nonUniform, bool spheresOnly, CTRayCaster< T >& caster )
    {
        boxMesh = typename TMeshObject::TMeshPtr( GUCEF_NEW CTTriangleMesh< T >() );
        MakeBox( *boxMesh, CTPoint3D< T >( -1, -1, -1 ), CTPoint3D< T >( 1, 1, 1 ), 3 );
        boxBvh = typename TMeshObject::TBvhPtr( GUCEF_NEW CTMeshBvh< T >() );
        boxBvh->Build( *boxMesh );
        for ( UInt32 i=0; i<count; ++i )
        {
            const Int32 kind = spheresOnly ? 0 : random.Index( 5 );
            const MATH::CTTransform3D< T > placement = RandomPlacement< T >( random, range, nonUniform && !spheresOnly );
            TObject* object = GUCEF_NULL;
            if ( 0 == kind )
            {
                const Float64 radius = random.Uniform( 0.2, 1.5 );
                object = GUCEF_NEW CTGeoObject< CTSphere< T > >( CTSphere< T >( CTPoint3D< T >(), (T) radius ), placement );
            }
            else if ( 1 == kind )
            {
                object = GUCEF_NEW CTGeoObject< CTAxisAlignedBox3D< T > >( CTAxisAlignedBox3D< T >( CTPoint3D< T >( -1, (T) -0.5, (T) -0.25 ), CTPoint3D< T >( 1, (T) 0.5, (T) 0.25 ) ), placement );
            }
            else if ( 2 == kind )
            {
                object = GUCEF_NEW CTGeoObject< CTCapsule< T > >( CTCapsule< T >( CTPoint3D< T >( 0, -1, 0 ), CTPoint3D< T >( 0, 1, 0 ), (T) 0.4 ), placement );
            }
            else if ( 3 == kind )
            {
                object = GUCEF_NEW CTGeoObject< CTOrientedBox3D< T > >( CTOrientedBox3D< T >( CTPoint3D< T >( (T) 0.5, 0, 0 ), ToQ< T >( random.Rotation() ), CTVector3D< T >( (T) 0.3, (T) 0.6, (T) 0.9 ) ), placement );
            }
            else
            {
                TMeshObject* mesh = GUCEF_NEW TMeshObject( boxMesh, boxBvh, placement );
                object = mesh;
                caster.AddObject( *mesh );
            }
            object->SetUserId( i );
            object->SetLayerMask( 1u << ( i % 4 ) );
            objects.push_back( object );
            isMesh.push_back( 4 == kind );
            if ( 4 != kind )
                caster.AddObject( *object );
        }
    }
};

/*-------------------------------------------------------------------------*/

/** The nearest hit over every object by its own ray cast (the caster must find the same) */
template < typename T >
static bool
BruteRaycast( const GUCEF::vector< CTGeoObjectBase< T >* >& objects, const CTRay3D< T >& ray, const T& maxDistance, UInt32 layerMask, SGeoRayHit< T >& best )
{
    bool found = false;
    for ( size_t i=0; i<objects.size(); ++i )
    {
        if ( 0 == ( objects[ i ]->GetLayerMask() & layerMask ) )
            continue;
        SRayCastResult< T > hit;
        if ( objects[ i ]->RayCast( ray, maxDistance, hit ) && ( !found || hit.distance < best.distance ) )
        {
            best = SGeoRayHit< T >( hit, objects[ i ] );
            found = true;
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

/** Orders hits by object */
template < typename T >
struct SByObject
{
    bool operator()( const SGeoRayHit< T >& a, const SGeoRayHit< T >& b ) const { return a.object->GetUserId() < b.object->GetUserId(); }
};

/*-------------------------------------------------------------------------*/

/** Ray casts, all-hit casts, lines of sight, sphere overlaps and batches over a random scene equal the scans, before and after moves */
template < typename T >
static bool
SceneSweep( const char* name, UInt64 seed, UInt32 objectCount, UInt32 rayCount, UInt32 rounds )
{
    CRandom random( seed );
    CTRayCaster< T > caster;
    SScene< T > scene;
    scene.Build( random, objectCount, 30.0, true, false, caster );
    UInt32 bad = 0;
    UInt32 hits = 0;
    GUCEF::vector< SGeoRayHit< T > > all;
    GUCEF::vector< CTGeoObjectBase< T >* > overlaps;
    GUCEF::vector< CTRay3D< T > > batchRays;
    for ( UInt32 round=0; round<rounds && bad<10; ++round )
    {
        batchRays.clear();
        for ( UInt32 n=0; n<rayCount && bad<10; ++n )
        {
            const P3 origin = random.Point( 40.0 );
            const D3 direction = random.Direction();
            const CTRay3D< T > ray( ToP< T >( origin ), ToD< T >( direction ) );
            const Float64 limit = random.Uniform( 1.0, 80.0 );
            const T maxDistance = (T) limit;
            const Int32 maskPick = random.Index( 3 );
            const UInt32 mask = 0 == maskPick ? 0xFFFFFFFF : ( 1 == maskPick ? 0x5u : 0x2u );
            batchRays.push_back( ray );

            SGeoRayHit< T > expected;
            SGeoRayHit< T > actual;
            const bool expectHit = BruteRaycast( scene.objects, ray, maxDistance, mask, expected );
            const bool gotHit = caster.Raycast( ray, maxDistance, mask, actual );
            if ( expectHit != gotHit || ( expectHit && ( !( expected.distance == actual.distance ) || ( expected.object != actual.object && !( expected.distance == actual.distance ) ) ) ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": round " << round << " ray " << n << " (seed " << seed << ") hit " << gotHit << " at " << actual.distance << ", scan " << expectHit << " at " << expected.distance );
                ++bad;
                continue;
            }
            if ( expectHit )
            {
                ++hits;
                if ( 0 == ( actual.object->GetLayerMask() & mask ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": ray " << n << " hit a masked object (seed " << seed << ")" );
                    ++bad;
                }
            }
            if ( caster.HasLineOfSight( ray.origin, ray.GetPoint( maxDistance ), mask ) == expectHit )
            {
                /* a hit exactly at the end point is a boundary case of the segment length */
                if ( !( expectHit && fabs( (Float64) expected.distance - limit ) < 1e-4 * ( 1.0 + limit ) ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": line of sight of ray " << n << " (seed " << seed << ") disagrees" );
                    ++bad;
                }
            }

            if ( 0 == n % 4 )
            {
                /* every object hit, its nearest hit */
                caster.RaycastAll( ray, maxDistance, mask, all, false );
                GUCEF::vector< SGeoRayHit< T > > reference;
                for ( size_t i=0; i<scene.objects.size(); ++i )
                {
                    SRayCastResult< T > hit;
                    if ( 0 != ( scene.objects[ i ]->GetLayerMask() & mask ) && scene.objects[ i ]->RayCast( ray, maxDistance, hit ) )
                        reference.push_back( SGeoRayHit< T >( hit, scene.objects[ i ] ) );
                }
                std::sort( all.begin(), all.end(), SByObject< T >() );
                bool same = all.size() == reference.size();
                for ( size_t i=0; same && i<all.size(); ++i )
                    same = all[ i ].object == reference[ i ].object && all[ i ].distance == reference[ i ].distance;
                caster.RaycastAll( ray, maxDistance, mask, all, true );
                for ( size_t i=1; same && i<all.size(); ++i )
                    same = !( all[ i ].distance < all[ i - 1 ].distance );
                if ( !same )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": RaycastAll " << n << " (seed " << seed << ") " << all.size() << " hits, the scan " << reference.size() );
                    ++bad;
                }

                /* sphere overlap: the distance of the centre against the radius (meshes by their world triangles) */
                const Float64 radius = random.Uniform( 0.5, 6.0 );
                const CTSphere< T > sphere( ray.origin, (T) radius );
                overlaps.clear();
                caster.OverlapSphere( sphere, mask, overlaps );
                UInt32 expectedCount = 0;
                for ( size_t i=0; i<scene.objects.size(); ++i )
                {
                    CTGeoObjectBase< T >& object = *scene.objects[ i ];
                    if ( 0 == ( object.GetLayerMask() & mask ) )
                        continue;
                    bool overlap = false;
                    if ( scene.isMesh[ i ] )
                    {
                        const CTMatrix4D< T >& world = object.GetWorldMatrix();
                        for ( UInt32 t=0; t<scene.boxMesh->GetTriangleCount() && !overlap; ++t )
                        {
                            const CTTriangle3D< T > local = scene.boxMesh->GetTriangle( t );
                            overlap = Intersects( CTTriangle3D< T >( world.TransformPoint( local.a ), world.TransformPoint( local.b ), world.TransformPoint( local.c ) ), sphere );
                        }
                    }
                    else
                    {
                        overlap = !( sphere.radius < object.GetDistance( sphere.centre ) );
                    }
                    if ( overlap )
                    {
                        ++expectedCount;
                        if ( std::find( overlaps.begin(), overlaps.end(), &object ) == overlaps.end() )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": OverlapSphere " << n << " misses object " << object.GetUserId() << " (seed " << seed << ")" );
                            ++bad;
                        }
                    }
                }
                if ( expectedCount != overlaps.size() || ( 0 < expectedCount ) != caster.CheckSphere( sphere, mask ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": OverlapSphere " << n << " (seed " << seed << ") " << overlaps.size() << " objects, the scan " << expectedCount );
                    ++bad;
                }
            }
        }

        /* the batch equals single casts */
        GUCEF::vector< SGeoRayHit< T > > batchHits( batchRays.size() );
        GUCEF::vector< UInt8 > flagStore( batchRays.size() + 1 );
        bool* flags = reinterpret_cast< bool* >( &flagStore[ 0 ] );
        caster.RaycastBatch( &batchRays[ 0 ], (UInt32) batchRays.size(), (T) 50, 0xFFFFFFFF, &batchHits[ 0 ], flags );
        for ( size_t i=0; i<batchRays.size(); ++i )
        {
            SGeoRayHit< T > single;
            const bool found = caster.Raycast( batchRays[ i ], (T) 50, 0xFFFFFFFF, single );
            if ( found != flags[ i ] || ( found && ( single.object != batchHits[ i ].object || !( single.distance == batchHits[ i ].distance ) ) ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": batch ray " << i << " differs from the single cast (seed " << seed << ")" );
                ++bad;
            }
        }

        /* move a third of the objects (directly or by a new placement) and refit */
        for ( size_t i=0; i<scene.objects.size(); ++i )
        {
            if ( 0 != random.Index( 3 ) )
                continue;
            scene.objects[ i ]->SetLocalTransform( RandomPlacement< T >( random, 30.0, true ) );
        }
        caster.Update();
        if ( 0 != caster.GetMovedCount() || !caster.GetTree().Validate() || caster.GetTree().GetProxyCount() != scene.objects.size() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": after Update of round " << round << " the tree is inconsistent (seed " << seed << ")" );
            ++bad;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << objectCount << " objects, " << rounds << " rounds of " << rayCount << " rays, " << hits << " hits, tree height " << caster.GetTree().GetHeight() );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Sphere casts, box casts and box overlaps against a scene of spheres compared with the value queries of the world spheres */
template < typename T >
static bool
SweepSweep( const char* name, UInt64 seed, UInt32 objectCount, UInt32 castCount, Float64 tolerance )
{
    CRandom random( seed );
    CTRayCaster< T > caster;
    SScene< T > scene;
    scene.Build( random, objectCount, 25.0, false, true, caster );
    /* the world spheres (similarity placements) */
    GUCEF::vector< CTSphere< T > > spheres;
    for ( size_t i=0; i<scene.objects.size(); ++i )
    {
        const CTGeoObject< CTSphere< T > >& object = static_cast< const CTGeoObject< CTSphere< T > >& >( *scene.objects[ i ] );
        spheres.push_back( object.GetShape().Transformed( object.GetWorldMatrix() ) );
    }
    UInt32 bad = 0;
    UInt32 hits = 0;
    GUCEF::vector< CTGeoObjectBase< T >* > overlaps;
    for ( UInt32 n=0; n<castCount && bad<10; ++n )
    {
        const P3 origin = random.Point( 30.0 );
        const D3 direction = random.Direction();
        const CTRay3D< T > ray( ToP< T >( origin ), ToD< T >( direction ) );
        const Float64 radius = random.Uniform( 0.1, 2.0 );
        const T maxDistance = (T) random.Uniform( 1.0, 60.0 );

        bool expectHit = false;
        Float64 expectDistance = 0;
        for ( size_t i=0; i<spheres.size(); ++i )
        {
            SRayCastResult< T > hit;
            if ( SphereCast( ray, (T) radius, spheres[ i ], maxDistance, hit ) && ( !expectHit || (Float64) hit.distance < expectDistance ) )
            {
                expectHit = true;
                expectDistance = (Float64) hit.distance;
            }
        }
        SGeoRayHit< T > hit;
        const bool gotHit = caster.SphereCast( ray, (T) radius, maxDistance, 0xFFFFFFFF, hit );
        if ( gotHit != expectHit || ( gotHit && fabs( (Float64) hit.distance - expectDistance ) > tolerance * ( 1.0 + expectDistance ) ) )
        {
            /* a graze within the tolerance of the maximum distance may go either way */
            if ( !( gotHit != expectHit && fabs( expectDistance - (Float64) maxDistance ) < tolerance * 10.0 * ( 1.0 + expectDistance ) ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": sphere cast " << n << " (seed " << seed << ") hit " << gotHit << " at " << hit.distance << ", spheres " << expectHit << " at " << expectDistance );
                ++bad;
            }
        }
        if ( gotHit )
            ++hits;

        /* box cast */
        const Q4 rotation = random.Rotation();
        const CTVector3D< T > half( (T) radius, (T) ( radius * 0.5 ), (T) ( radius * 0.7 ) );
        const CTOrientedBox3D< T > localBox( CTPoint3D< T >(), CTQuaternion< T >(), half );
        const MATH::CTTransform3D< T > start( ToV< T >( origin.ToPositionVector() ), ToQ< T >( rotation ), CTVector3D< T >( 1, 1, 1 ) );
        const CTOrientedBox3D< T > worldBox( ToP< T >( origin ), ToQ< T >( rotation ), half );
        expectHit = false;
        for ( size_t i=0; i<spheres.size(); ++i )
        {
            SRayCastResult< T > cast;
            if ( ShapeCast( worldBox, ToD< T >( direction ), maxDistance, spheres[ i ], cast ) && ( !expectHit || (Float64) cast.distance < expectDistance ) )
            {
                expectHit = true;
                expectDistance = (Float64) cast.distance;
            }
        }
        const bool gotBox = caster.ShapeCast( localBox, start, ToD< T >( direction ).AsVector() * maxDistance, 0xFFFFFFFF, hit );
        if ( gotBox != expectHit || ( gotBox && fabs( (Float64) hit.distance - expectDistance ) > tolerance * ( 1.0 + expectDistance ) ) )
        {
            if ( !( gotBox != expectHit && fabs( expectDistance - (Float64) maxDistance ) < tolerance * 10.0 * ( 1.0 + expectDistance ) ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": box cast " << n << " (seed " << seed << ") hit " << gotBox << " at " << hit.distance << ", spheres " << expectHit << " at " << expectDistance );
                ++bad;
            }
        }

        /* box overlap: the distance of each sphere centre to the box against its radius, away from the boundary */
        overlaps.clear();
        caster.OverlapBox( worldBox, 0xFFFFFFFF, overlaps );
        for ( size_t i=0; i<spheres.size(); ++i )
        {
            const Float64 gap = (Float64) worldBox.GetDistance( spheres[ i ].centre ) - (Float64) spheres[ i ].radius;
            const bool listed = std::find( overlaps.begin(), overlaps.end(), scene.objects[ i ] ) != overlaps.end();
            if ( ( gap < -tolerance && !listed ) || ( gap > tolerance && listed ) )
                ++bad;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << castCount << " sphere and box casts, " << hits << " sphere hits" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Layer masks exclude objects, filters reject them, RaycastAll and overlaps honour both */
static bool
LayersAndFilters( void )
{
    bool ok = true;
    CRayCaster caster;
    CGeoSphere near( TFloat32Sphere( CTPoint3D< Float32 >( 5, 0, 0 ), 1 ) );
    CGeoSphere far( TFloat32Sphere( CTPoint3D< Float32 >( 10, 0, 0 ), 1 ) );
    near.SetLayerMask( 0x1 );
    near.SetUserId( 1 );
    far.SetLayerMask( 0x2 );
    far.SetUserId( 2 );
    ok = caster.AddObject( near ) && caster.AddObject( far ) && ok;
    if ( caster.AddObject( near ) || !caster.IsRegistered( far ) )
        ok = false;
    const TFloat32Ray3D ray( CTPoint3D< Float32 >( 0, 0, 0 ), CTDirectionVector3D< Float32 >::UnitX() );
    TFloat32GeoRayHit hit;
    if ( !caster.Raycast( ray, 100, 0xFFFFFFFF, hit ) || hit.object != &near || fabs( hit.distance - 4.0f ) > 1e-5f )
        ok = false;
    if ( !caster.Raycast( ray, 100, 0x2, hit ) || hit.object != &far || fabs( hit.distance - 9.0f ) > 1e-5f )
        ok = false;
    if ( caster.Raycast( ray, 100, 0x4, hit ) || caster.Raycast( ray, 3.9f, 0xFFFFFFFF, hit ) )
        ok = false;
    const CRejectUserId< Float32 > rejectNear( 1 );
    if ( !caster.Raycast( ray, 100, 0xFFFFFFFF, hit, &rejectNear ) || hit.object != &far )
        ok = false;
    GUCEF::vector< TFloat32GeoRayHit > all;
    if ( 2 != caster.RaycastAll( ray, 100, 0xFFFFFFFF, all, true ) || all[ 0 ].object != &near || all[ 1 ].object != &far )
        ok = false;
    if ( 1 != caster.RaycastAll( ray, 100, 0xFFFFFFFF, all, true, &rejectNear ) || all[ 0 ].object != &far )
        ok = false;
    if ( caster.HasLineOfSight( CTPoint3D< Float32 >( 0, 0, 0 ), CTPoint3D< Float32 >( 20, 0, 0 ), 0xFFFFFFFF ) ||
         !caster.HasLineOfSight( CTPoint3D< Float32 >( 0, 0, 0 ), CTPoint3D< Float32 >( 20, 0, 0 ), 0x4 ) ||
         !caster.HasLineOfSight( CTPoint3D< Float32 >( 0, 0, 0 ), CTPoint3D< Float32 >( 3, 0, 0 ), 0xFFFFFFFF ) ||
         !caster.HasLineOfSight( CTPoint3D< Float32 >( 1, 1, 1 ), CTPoint3D< Float32 >( 1, 1, 1 ), 0xFFFFFFFF ) )
        ok = false;
    GUCEF::vector< CGeoObjectBase* > results;
    if ( 2 != caster.OverlapSphere( TFloat32Sphere( CTPoint3D< Float32 >( 7.5f, 0, 0 ), 2 ), 0xFFFFFFFF, results ) )
        ok = false;
    results.clear();
    if ( 1 != caster.OverlapSphere( TFloat32Sphere( CTPoint3D< Float32 >( 7.5f, 0, 0 ), 2 ), 0x2, results ) || results[ 0 ] != &far )
        ok = false;
    results.clear();
    if ( 1 != caster.OverlapSphere( TFloat32Sphere( CTPoint3D< Float32 >( 7.5f, 0, 0 ), 2 ), 0xFFFFFFFF, results, &rejectNear ) )
        ok = false;
    if ( caster.CheckSphere( TFloat32Sphere( CTPoint3D< Float32 >( 7.5f, 0, 0 ), 1 ), 0xFFFFFFFF ) || !caster.CheckSphere( TFloat32Sphere( CTPoint3D< Float32 >( 7.5f, 0, 0 ), 1.6f ), 0xFFFFFFFF ) )
        ok = false;
    results.clear();
    const TFloat32OrientedBox3D box( CTPoint3D< Float32 >( 10, 0, 0 ), CTQuaternion< Float32 >(), CTVector3D< Float32 >( 0.5f, 0.5f, 0.5f ) );
    if ( 1 != caster.OverlapBox( box, 0xFFFFFFFF, results ) || results[ 0 ] != &far )
        ok = false;
    results.clear();
    if ( 0 != caster.OverlapBox( box, 0x1, results ) )
        ok = false;
    if ( !caster.RemoveObject( near ) || caster.RemoveObject( near ) || caster.IsRegistered( near ) || 0 != near.GetStructureCount() )
        ok = false;
    if ( !caster.Raycast( ray, 100, 0xFFFFFFFF, hit ) || hit.object != &far )
        ok = false;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: layers and filters failed" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Moving a parent node: before Update the broadphase still holds the old bounds, after Update rays hit the moved children */
static bool
ParentMoveAndUpdate( void )
{
    bool ok = true;
    CRayCaster caster;
    CGeoSpace parent;
    CGeoSpace middle;
    middle.SetParent( &parent, false );
    CGeoSphere a( TFloat32Sphere( CTPoint3D< Float32 >( 0, 0, 0 ), 1 ) );
    CGeoSphere b( TFloat32Sphere( CTPoint3D< Float32 >( 0, 0, 0 ), 1 ) );
    a.SetParent( &parent, false );
    b.SetParent( &middle, false );
    a.SetLocalPosition( CTVector3D< Float32 >( 5, 0, 0 ) );
    b.SetLocalPosition( CTVector3D< Float32 >( 5, 0, 5 ) );
    caster.AddObject( a );
    caster.AddObject( b );
    const TFloat32Ray3D rayA( CTPoint3D< Float32 >( 0, 0, 0 ), CTDirectionVector3D< Float32 >::UnitX() );
    const TFloat32Ray3D rayB( CTPoint3D< Float32 >( 0, 0, 5 ), CTDirectionVector3D< Float32 >::UnitX() );
    TFloat32GeoRayHit hit;
    if ( !caster.Raycast( rayA, 100, 0xFFFFFFFF, hit ) || hit.object != &a || !caster.Raycast( rayB, 100, 0xFFFFFFFF, hit ) || hit.object != &b )
        ok = false;

    parent.SetLocalPosition( CTVector3D< Float32 >( 0, 20, 0 ) );
    if ( 2 != caster.GetMovedCount() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: " << caster.GetMovedCount() << " objects reported moved instead of 2" );
        ok = false;
    }
    caster.Update();
    if ( 0 != caster.GetMovedCount() || !caster.GetTree().Validate() )
        ok = false;
    const TFloat32Ray3D movedA( CTPoint3D< Float32 >( 0, 20, 0 ), CTDirectionVector3D< Float32 >::UnitX() );
    const TFloat32Ray3D movedB( CTPoint3D< Float32 >( 0, 20, 5 ), CTDirectionVector3D< Float32 >::UnitX() );
    if ( caster.Raycast( rayA, 100, 0xFFFFFFFF, hit ) || caster.Raycast( rayB, 100, 0xFFFFFFFF, hit ) )
        ok = false;
    if ( !caster.Raycast( movedA, 100, 0xFFFFFFFF, hit ) || hit.object != &a || fabs( hit.distance - 4.0f ) > 1e-4f )
        ok = false;
    if ( !caster.Raycast( movedB, 100, 0xFFFFFFFF, hit ) || hit.object != &b || fabs( hit.distance - 4.0f ) > 1e-4f )
        ok = false;

    /* a second move (only the middle node), and a shape change */
    middle.SetLocalPosition( CTVector3D< Float32 >( 0, 0, 10 ) );
    a.SetShape( TFloat32Sphere( CTPoint3D< Float32 >( 0, 0, 0 ), 2 ) );
    if ( 2 != caster.GetMovedCount() )
        ok = false;
    caster.Update();
    const TFloat32Ray3D movedAgainB( CTPoint3D< Float32 >( 0, 20, 15 ), CTDirectionVector3D< Float32 >::UnitX() );
    if ( !caster.Raycast( movedAgainB, 100, 0xFFFFFFFF, hit ) || hit.object != &b )
        ok = false;
    if ( !caster.Raycast( movedA, 100, 0xFFFFFFFF, hit ) || hit.object != &a || fabs( hit.distance - 3.0f ) > 1e-4f )
        ok = false;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: parent move and update failed" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A destroyed object leaves the caster; a destroyed caster leaves its objects; moved-then-destroyed objects are forgotten */
static bool
DestructionCases( void )
{
    bool ok = true;
    CGeoSphere survivor( TFloat32Sphere( CTPoint3D< Float32 >( 10, 0, 0 ), 1 ) );
    {
        CRayCaster caster;
        CGeoSphere* doomed = GUCEF_NEW CGeoSphere( TFloat32Sphere( CTPoint3D< Float32 >( 5, 0, 0 ), 1 ) );
        CGeoSphere* moved = GUCEF_NEW CGeoSphere( TFloat32Sphere( CTPoint3D< Float32 >( 7, 0, 0 ), 0.5f ) );
        caster.AddObject( *doomed );
        caster.AddObject( *moved );
        caster.AddObject( survivor );
        if ( 3 != caster.GetTree().GetProxyCount() || 3 != caster.GetObjectCount() )
            ok = false;
        GUCEF_DELETE doomed;
        moved->SetLocalPosition( CTVector3D< Float32 >( 1, 2, 3 ) );
        if ( 1 != caster.GetMovedCount() )
            ok = false;
        GUCEF_DELETE moved;
        if ( 0 != caster.GetMovedCount() || 1 != caster.GetTree().GetProxyCount() || 1 != caster.GetObjectCount() || !caster.GetTree().Validate() )
            ok = false;
        caster.Update();
        TFloat32GeoRayHit hit;
        const TFloat32Ray3D ray( CTPoint3D< Float32 >( 0, 0, 0 ), CTDirectionVector3D< Float32 >::UnitX() );
        if ( !caster.Raycast( ray, 100, 0xFFFFFFFF, hit ) || hit.object != &survivor )
            ok = false;
        if ( 1 != survivor.GetStructureCount() )
            ok = false;
    }
    /* the caster is gone: the object no longer links to it and can move freely */
    if ( 0 != survivor.GetStructureCount() )
        ok = false;
    survivor.SetLocalPosition( CTVector3D< Float32 >( 3, 3, 3 ) );
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: destruction cases failed" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Mesh objects: triangle index and barycentrics in the hit, sweeps and overlaps by world triangles, solid mode, unbounded planes */
static bool
MeshAndPlaneObjects( void )
{
    bool ok = true;
    typedef CTGeoTriangleMesh< Float64 > TMeshObject;
    TMeshObject::TMeshPtr mesh( GUCEF_NEW CTTriangleMesh< Float64 >() );
    MakeBox( *mesh, CTPoint3D< Float64 >( -1, -1, -1 ), CTPoint3D< Float64 >( 1, 1, 1 ), 2 );
    TMeshObject::TBvhPtr bvh( GUCEF_NEW CTMeshBvh< Float64 >() );
    bvh->Build( *mesh );
    /* a non-uniform scale: the box spans x in [ 7, 13 ], y and z in [ -1, 1 ] */
    TMeshObject placed( mesh, bvh, MATH::CTTransform3D< Float64 >( CTVector3D< Float64 >( 10, 0, 0 ), CTQuaternion< Float64 >(), CTVector3D< Float64 >( 3, 1, 1 ) ) );
    CRayCaster64 caster;
    caster.AddObject( placed );
    const TFloat64Ray3D ray( CTPoint3D< Float64 >( 0, 0.25, 0.25 ), CTDirectionVector3D< Float64 >::UnitX() );
    TFloat64GeoRayHit hit;
    if ( !caster.Raycast( ray, 100, 0xFFFFFFFF, hit ) || fabs( hit.distance - 7.0 ) > 1e-12 || hit.featureIndex < 0 || hit.object != &placed || hit.IsBackFace() || fabs( hit.normal.x + 1.0 ) > 1e-12 )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    /* from inside a surface mesh the far wall is hit from behind */
    const TFloat64Ray3D inside( CTPoint3D< Float64 >( 10, 0.25, 0.25 ), CTDirectionVector3D< Float64 >::UnitX() );
    if ( !caster.Raycast( inside, 100, 0xFFFFFFFF, hit ) || fabs( hit.distance - 3.0 ) > 1e-12 || !hit.IsBackFace() || hit.StartedInside() )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    /* solid: started inside */
    placed.SetSolid( true );
    caster.Update();
    if ( !caster.Raycast( inside, 100, 0xFFFFFFFF, hit ) || !hit.StartedInside() || 0.0 != hit.distance || !placed.Contains( CTPoint3D< Float64 >( 12, 0, 0 ) ) || placed.Contains( CTPoint3D< Float64 >( 14, 0, 0 ) ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    placed.SetSolid( false );
    caster.Update();
    if ( placed.Contains( CTPoint3D< Float64 >( 12, 0, 0 ) ) || !placed.Contains( CTPoint3D< Float64 >( 13, 0, 0 ) ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }

    /* a sphere cast along x stops at radius before the wall; a box cast likewise */
    if ( !caster.SphereCast( ray, 0.5, 100, 0xFFFFFFFF, hit ) || fabs( hit.distance - 6.5 ) > 1e-9 || hit.object != &placed )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    const TFloat64OrientedBox3D localBox( CTPoint3D< Float64 >(), CTQuaternion< Float64 >(), CTVector3D< Float64 >( 0.25, 0.1, 0.1 ) );
    const MATH::CTTransform3D< Float64 > start( CTVector3D< Float64 >( 0, 0.25, 0.25 ), CTQuaternion< Float64 >(), CTVector3D< Float64 >( 1, 1, 1 ) );
    if ( !caster.ShapeCast( localBox, start, CTVector3D< Float64 >( 100, 0, 0 ), 0xFFFFFFFF, hit ) || fabs( hit.distance - 6.75 ) > 1e-6 )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    /* overlaps by world triangles: a sphere that reaches the wall, one inside that does not (surface mesh) */
    GUCEF::vector< CGeoObjectBase64* > results;
    if ( 1 != caster.OverlapSphere( TFloat64Sphere( CTPoint3D< Float64 >( 6.5, 0, 0 ), 0.6 ), 0xFFFFFFFF, results ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    results.clear();
    if ( 0 != caster.OverlapSphere( TFloat64Sphere( CTPoint3D< Float64 >( 10, 0, 0 ), 0.5 ), 0xFFFFFFFF, results ) || caster.CheckSphere( TFloat64Sphere( CTPoint3D< Float64 >( 6.5, 0, 0 ), 0.4 ), 0xFFFFFFFF ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    if ( 1 != caster.OverlapBox( TFloat64OrientedBox3D( CTPoint3D< Float64 >( 13, 0, 0 ), CTQuaternion< Float64 >(), CTVector3D< Float64 >( 0.1, 0.1, 0.1 ) ), 0xFFFFFFFF, results ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }

    /* an unbounded plane (local XY, normal +Z) kept outside the tree */
    CTGeoPlane< Float64 > ground( MATH::CTTransform3D< Float64 >( CTVector3D< Float64 >( 0, 0, -5 ), CTQuaternion< Float64 >(), CTVector3D< Float64 >( 1, 1, 1 ) ) );
    caster.AddObject( ground );
    if ( 1 != caster.GetUnboundedCount() || 1 != caster.GetTree().GetProxyCount() )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    const TFloat64Ray3D down( CTPoint3D< Float64 >( 100, 100, 0 ), CTDirectionVector3D< Float64 >( CTVector3D< Float64 >( 0, 0, -1 ) ) );
    if ( !caster.Raycast( down, 100, 0xFFFFFFFF, hit ) || hit.object != &ground || fabs( hit.distance - 5.0 ) > 1e-12 )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    if ( !caster.CheckSphere( TFloat64Sphere( CTPoint3D< Float64 >( 100, 100, -4.5 ), 0.6 ), 0xFFFFFFFF ) || caster.CheckSphere( TFloat64Sphere( CTPoint3D< Float64 >( 100, 100, -4.5 ), 0.4 ), 0xFFFFFFFF ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    if ( caster.HasLineOfSight( CTPoint3D< Float64 >( 100, 100, 0 ), CTPoint3D< Float64 >( 100, 100, -10 ), 0xFFFFFFFF ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    GUCEF::vector< TFloat64GeoRayHit > all;
    const TFloat64Ray3D slanted( CTPoint3D< Float64 >( 0, 0.25, 0 ), CTDirectionVector3D< Float64 >( CTVector3D< Float64 >( 1, 0, -0.1 ) ) );
    if ( 2 != caster.RaycastAll( slanted, 100, 0xFFFFFFFF, all, true ) || all[ 0 ].object != &placed || all[ 1 ].object != &ground )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    caster.RemoveObject( ground );
    if ( 0 != caster.GetUnboundedCount() )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane check at line " << __LINE__ << " failed" ); }
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh and plane objects failed" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Counts the triangles QueryBox visits and the highest index */
struct STriangleIndexRange
{
    UInt32 count;
    UInt32 highest;

    STriangleIndexRange( void ) : count( 0 ), highest( 0 ) {}

    bool Visit( UInt32 triangle )
    {
        ++count;
        if ( highest < triangle )
            highest = triangle;
        return true;
    }
};

/**
 *  A hierarchy shared between mesh objects is never rebuilt in place: object A holds (mesh1, bvh), object B is created
 *  with (mesh2, the same bvh): A keeps a hierarchy over mesh1 (bounds, ray casts, QueryBox indices within mesh1), B
 *  gets a private one over mesh2, and the shared one is untouched. An unbuilt BVH given to an object stays unbuilt.
 */
static bool
SharedBvhOwnership( void )
{
    bool ok = true;
    typedef CTGeoTriangleMesh< Float64 > TMeshObject;
    typedef CTPoint3D< Float64 > P;
    TMeshObject::TMeshPtr mesh1( GUCEF_NEW CTTriangleMesh< Float64 >() );
    mesh1->AddVertex( P( 0, 0, 0 ) );
    mesh1->AddVertex( P( 1, 0, 0 ) );
    mesh1->AddVertex( P( 0, 1, 0 ) );
    mesh1->AddTriangle( 0, 1, 2 );
    TMeshObject::TMeshPtr mesh2( GUCEF_NEW CTTriangleMesh< Float64 >() );
    for ( UInt32 i=0; i<200; ++i )
    {
        const UInt32 a = mesh2->AddVertex( P( (Float64) i, 0, 0 ) );
        const UInt32 b = mesh2->AddVertex( P( (Float64) i + 1.0, 0, 0 ) );
        const UInt32 c = mesh2->AddVertex( P( (Float64) i, 1, 0 ) );
        mesh2->AddTriangle( a, b, c );
    }
    TMeshObject::TBvhPtr shared( GUCEF_NEW CTMeshBvh< Float64 >() );
    shared->Build( *mesh1 );
    TMeshObject objectA( mesh1, shared );
    TMeshObject objectB( mesh2, shared );
    if ( shared->GetMesh() != mesh1.GetPointerAlways() || objectA.GetBvh()->GetMesh() != mesh1.GetPointerAlways() || objectB.GetBvh()->GetMesh() != mesh2.GetPointerAlways() || objectB.GetBvh() == shared )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: shared BVH check at line " << __LINE__ << " failed" ); }
    const CTAxisAlignedBox3D< Float64 >& boundsA = objectA.GetWorldBounds();
    if ( !( boundsA.max.x == 1.0 ) || !( objectB.GetWorldBounds().max.x == 200.0 ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: shared BVH check at line " << __LINE__ << " failed" ); }
    SRayCastResult< Float64 > hit;
    const CTRay3D< Float64 > far( P( 150.2, 0.2, 5 ), CTDirectionVector3D< Float64 >( CTVector3D< Float64 >( 0, 0, -1 ) ) );
    if ( objectA.RayCast( far, 100, hit ) || !objectB.RayCast( far, 100, hit ) || 150 != hit.featureIndex )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: shared BVH check at line " << __LINE__ << " failed" ); }
    STriangleIndexRange range;
    objectA.GetBvh()->QueryBox( CTAxisAlignedBox3D< Float64 >( P( -10, -10, -10 ), P( 300, 10, 10 ) ), range );
    if ( 1 != range.count || 0 != range.highest )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: shared BVH check at line " << __LINE__ << " failed" ); }

    /* the ray caster over A finds nothing near x = 150 (A spans x in [ 0, 1 ]) */
    CRayCaster64 caster;
    caster.AddObject( objectA );
    caster.Update();
    GUCEF::vector< CGeoObjectBase64* > results;
    if ( 0 != caster.OverlapSphere( TFloat64Sphere( P( 150, 0.2, 0 ), 0.5 ), 0xFFFFFFFF, results ) || 0 != caster.OverlapBox( TFloat64OrientedBox3D( P( 150, 0.2, 0 ), CTQuaternion< Float64 >(), CTVector3D< Float64 >( 0.5, 0.5, 0.5 ) ), 0xFFFFFFFF, results ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: shared BVH check at line " << __LINE__ << " failed" ); }

    /* SetMesh to the other mesh with the shared hierarchy: a private one again, the shared one untouched */
    objectA.SetMesh( mesh2, shared );
    if ( shared->GetMesh() != mesh1.GetPointerAlways() || objectA.GetBvh()->GetMesh() != mesh2.GetPointerAlways() || !( objectA.GetWorldBounds().max.x == 200.0 ) )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: shared BVH check at line " << __LINE__ << " failed" ); }

    /* an unbuilt hierarchy is left unbuilt; the object builds its own */
    TMeshObject::TBvhPtr unbuilt( GUCEF_NEW CTMeshBvh< Float64 >() );
    TMeshObject objectC( mesh1, unbuilt );
    if ( unbuilt->IsBuilt() || objectC.GetBvh()->GetMesh() != mesh1.GetPointerAlways() || objectC.IsEmpty() )
        { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: shared BVH check at line " << __LINE__ << " failed" ); }
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  A mesh registered through the base reference (the AddObject of section 7.7) takes part in every query exactly as
 *  the same mesh registered through its own type: box overlaps, sphere overlaps, sphere and shape casts, ray casts
 */
static bool
MeshRegisteredThroughBase( void )
{
    bool ok = true;
    typedef CTPoint3D< Float64 > P;
    CTTriangleMesh< Float64 > ground;
    ground.AddVertex( P( -5, 0, -5 ) );
    ground.AddVertex( P( 5, 0, -5 ) );
    ground.AddVertex( P( 0, 0, 5 ) );
    ground.AddTriangle( 0, 2, 1 );
    UInt32 answers[ 2 ][ 5 ];
    Float64 distances[ 2 ][ 3 ];
    for ( UInt32 viaBase=0; viaBase<2; ++viaBase )
    {
        CTGeoTriangleMesh< Float64 > object( ground );
        CRayCaster64 caster;
        if ( 1 == viaBase )
        {
            CGeoObjectBase64& base = object;
            caster.AddObject( base );
        }
        else
        {
            caster.AddObject( object );
        }
        caster.Update();
        GUCEF::vector< CGeoObjectBase64* > results;
        answers[ viaBase ][ 0 ] = caster.OverlapBox( TFloat64OrientedBox3D( P( 0, 0, 0 ), CTQuaternion< Float64 >(), CTVector3D< Float64 >( 1, 1, 1 ) ), 0xFFFFFFFF, results );
        answers[ viaBase ][ 1 ] = caster.OverlapSphere( TFloat64Sphere( P( 0, 0.5, 0 ), 0.6 ), 0xFFFFFFFF, results );
        const TFloat64Ray3D down( P( 0, 3, 0 ), CTDirectionVector3D< Float64 >( CTVector3D< Float64 >( 0, -1, 0 ) ) );
        TFloat64GeoRayHit hit;
        answers[ viaBase ][ 2 ] = caster.SphereCast( down, 0.5, 10, 0xFFFFFFFF, hit ) ? 1 : 0;
        distances[ viaBase ][ 0 ] = hit.distance;
        const TFloat64OrientedBox3D cube( P(), CTQuaternion< Float64 >(), CTVector3D< Float64 >( 0.25, 0.25, 0.25 ) );
        const MATH::CTTransform3D< Float64 > start( CTVector3D< Float64 >( 0, 3, 0 ), CTQuaternion< Float64 >(), CTVector3D< Float64 >( 1, 1, 1 ) );
        answers[ viaBase ][ 3 ] = caster.ShapeCast( cube, start, CTVector3D< Float64 >( 0, -10, 0 ), 0xFFFFFFFF, hit ) ? 1 : 0;
        distances[ viaBase ][ 1 ] = hit.distance;
        answers[ viaBase ][ 4 ] = caster.Raycast( down, 10, 0xFFFFFFFF, hit ) ? 1 : 0;
        distances[ viaBase ][ 2 ] = hit.distance;
    }
    for ( UInt32 q=0; q<5; ++q )
        if ( 1 != answers[ 0 ][ q ] || answers[ 1 ][ q ] != answers[ 0 ][ q ] )
            { ok = false; GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: mesh through the base reference: query " << q << " answers " << answers[ 0 ][ q ] << " (own type) and " << answers[ 1 ][ q ] << " (base)" ); }
    if ( fabs( distances[ 0 ][ 0 ] - 2.5 ) > 1e-9 || fabs( distances[ 0 ][ 1 ] - 2.75 ) > 1e-6 || fabs( distances[ 0 ][ 2 ] - 3.0 ) > 1e-12 )
        ok = false;
    for ( UInt32 q=0; q<3; ++q )
        if ( !( distances[ 0 ][ q ] == distances[ 1 ][ q ] ) )
            ok = false;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The work of one query thread: its rays against the shared caster, compared with the owner's answers afterwards */
struct SConcurrentRays
{
    const CRayCaster* caster;
    const GUCEF::vector< TFloat32Ray3D >* rays;
    GUCEF::vector< TFloat32GeoRayHit > hits;
    GUCEF::vector< UInt8 > found;

    static void Run( void* argument )
    {
        SConcurrentRays& work = *static_cast< SConcurrentRays* >( argument );
        work.hits.resize( work.rays->size() );
        work.found.resize( work.rays->size() );
        for ( size_t i=0; i<work.rays->size(); ++i )
            work.found[ i ] = work.caster->Raycast( ( *work.rays )[ i ], 60.0f, 0xFFFFFFFF, work.hits[ i ] ) ? 1 : 0;
    }
};

/**
 *  Four threads cast rays concurrently against one caster without mutation; each gets the owner's answers. After the
 *  owner's answers every object moves away and back and the owner calls Update only: the query threads then read
 *  caches that Update alone brought up to date (no warm-up by owner queries), the documented contract of CTRayCaster.
 */
static bool
ConcurrentRaycasts( UInt64 seed, UInt32 rayCount )
{
    CRandom random( seed );
    CRayCaster caster;
    SScene< Float32 > scene;
    scene.Build( random, 300, 30.0, true, false, caster );
    caster.Update();
    GUCEF::vector< TFloat32Ray3D > rays;
    for ( UInt32 n=0; n<rayCount; ++n )
    {
        const P3 origin = random.Point( 40.0 );
        const D3 direction = random.Direction();
        rays.push_back( TFloat32Ray3D( ToP< Float32 >( origin ), ToD< Float32 >( direction ) ) );
    }
    /* the owner answers first, which brings every lazily computed cache up to date */
    SConcurrentRays reference;
    reference.caster = &caster;
    reference.rays = &rays;
    SConcurrentRays::Run( &reference );

    /* away and back: the same placements, every world cache invalid until Update */
    for ( size_t i=0; i<scene.objects.size(); ++i )
    {
        const MATH::CTTransform3D< Float32 > placement = scene.objects[ i ]->GetLocalTransform();
        scene.objects[ i ]->SetLocalTransform( MATH::CTTransform3D< Float32 >() );
        scene.objects[ i ]->SetLocalTransform( placement );
    }
    if ( caster.GetMovedCount() != (UInt32) scene.objects.size() )
        return false;
    caster.Update();

    SConcurrentRays work[ 4 ];
    CRayCasterTestThread* threads[ 4 ];
    bool ok = true;
    for ( UInt32 t=0; t<4; ++t )
    {
        work[ t ].caster = &caster;
        work[ t ].rays = &rays;
        threads[ t ] = GUCEF_NEW CRayCasterTestThread( &SConcurrentRays::Run, &work[ t ] );
        ok = threads[ t ]->Activate( GUCEF_NULL, 0, 0 ) && ok;
    }
    for ( UInt32 t=0; t<4; ++t )
    {
        ok = threads[ t ]->WaitUntilDone( 120000 ) && ok;
        GUCEF_DELETE threads[ t ];
    }
    UInt32 hits = 0;
    for ( UInt32 t=0; t<4 && ok; ++t )
    {
        for ( size_t i=0; i<rays.size(); ++i )
        {
            if ( work[ t ].found[ i ] != reference.found[ i ] || ( 0 != reference.found[ i ] && ( work[ t ].hits[ i ].object != reference.hits[ i ].object || !( work[ t ].hits[ i ].distance == reference.hits[ i ].distance ) ) ) )
            {
                ok = false;
                break;
            }
            if ( 0 == t && 0 != reference.found[ i ] )
                ++hits;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RayCaster: 4 threads cast " << rayCount << " rays each concurrently, " << hits << " hits, agree with the owner: " << ok );
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformRayCasterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "RayCaster" );

    GUCEF_TESTFW_TESTCASE( "RayCaster: layer masks exclude masked objects, CTGeoRayFilter rejections are honoured, RemoveObject" )
    try
    {
        ASSERT_TRUE( LayersAndFilters() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCaster: after moving a parent node and calling Update, rays hit the moved children" )
    try
    {
        ASSERT_TRUE( ParentMoveAndUpdate() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCaster: destroying a registered object unregisters it (also after a move); a destroyed caster leaves its objects" )
    try
    {
        ASSERT_TRUE( DestructionCases() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCaster: random scenes: Raycast, RaycastAll, HasLineOfSight, OverlapSphere, CheckSphere and RaycastBatch equal the scans, before and after moves" )
    try
    {
        ASSERT_TRUE( SceneSweep< Float64 >( "scene Float64", 40701, ScaledCount( 400 ), ScaledCount( 2000 ), 4 ) );
        ASSERT_TRUE( SceneSweep< Float32 >( "scene Float32", 40702, ScaledCount( 400 ), ScaledCount( 2000 ), 4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCaster: SphereCast, ShapeCast and OverlapBox over spheres equal the value queries of the world spheres (1e-6 Float64, 1e-3 Float32)" )
    try
    {
        ASSERT_TRUE( SweepSweep< Float64 >( "sweeps Float64", 40711, ScaledCount( 300 ), ScaledCount( 2000 ), 1e-6 ) );
        ASSERT_TRUE( SweepSweep< Float32 >( "sweeps Float32", 40712, ScaledCount( 300 ), ScaledCount( 1000 ), 1e-3 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCaster: mesh objects (triangle hits, back faces, solid mode, sweeps and overlaps by world triangles) and unbounded planes" )
    try
    {
        ASSERT_TRUE( MeshAndPlaneObjects() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCaster: a BVH shared between mesh objects is never rebuilt in place over another mesh" )
    try
    {
        ASSERT_TRUE( SharedBvhOwnership() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCaster: a mesh registered through AddObject( CTGeoObjectBase& ) is swept and overlapped like one registered through its own type" )
    try
    {
        ASSERT_TRUE( MeshRegisteredThroughBase() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCaster: concurrent Raycast from four threads after Update (no owner warm-up) gives the owner's answers" )
    try
    {
        ASSERT_TRUE( ConcurrentRaycasts( 40721, ScaledCount( 5000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
