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

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CTSLOTDATAEXCHANGE_H
#include "gucefMT_CTSlotDataExchange.h"
#define GUCEF_MT_CTSLOTDATAEXCHANGE_H
#endif /* GUCEF_MT_CTSLOTDATAEXCHANGE_H ? */

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

#ifndef GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H
#include "gucefGEOMETRY_CTAudioOcclusionEstimator.h"
#define GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H
#endif /* GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H ? */

#include "TestConvexReference.h"
#include "TestAudioOcclusion.h"

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
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A ray caster with a few objects owned by the test */
template < typename T >
struct SOcclusionScene
{
    typedef CTGeoObjectBase< T > TObject;

    CTRayCaster< T > caster;
    GUCEF::vector< TObject* > objects;

    SOcclusionScene( void ) : caster(), objects() {}

    ~SOcclusionScene()
    {
        for ( size_t i=0; i<objects.size(); ++i )
            GUCEF_DELETE objects[ i ];
    }

    /** A bounded wall in the plane z = 'z' (normal +Z) over the extents x0 .. x1, y0 .. y1 */
    TObject* AddWall( Float64 z, Float64 x0, Float64 x1, Float64 y0, Float64 y1, UInt32 layerMask, UInt64 userId )
    {
        const CTTransform3D< T > placement( CTVector3D< T >( (T) 0, (T) 0, (T) z ), CTQuaternion< T >(), CTVector3D< T >( (T) 1, (T) 1, (T) 1 ) );
        CTGeoPlane< T >* wall = GUCEF_NEW CTGeoPlane< T >( placement );
        wall->SetExtents( CTRectangle< T >( CTPoint2D< T >( (T) x0, (T) y0 ), CTPoint2D< T >( (T) x1, (T) y1 ) ) );
        wall->SetLayerMask( layerMask );
        wall->SetUserId( userId );
        objects.push_back( wall );
        caster.AddObject( *wall );
        caster.Update();
        return wall;
    }

    /** A box of half size 'half' centred on a point */
    TObject* AddBox( Float64 x, Float64 y, Float64 z, Float64 half, UInt32 layerMask, UInt64 userId )
    {
        const CTAxisAlignedBox3D< T > box( CTPoint3D< T >( (T) ( x - half ), (T) ( y - half ), (T) ( z - half ) ), CTPoint3D< T >( (T) ( x + half ), (T) ( y + half ), (T) ( z + half ) ) );
        CTGeoObject< CTAxisAlignedBox3D< T > >* object = GUCEF_NEW CTGeoObject< CTAxisAlignedBox3D< T > >( box, CTTransform3D< T >() );
        object->SetLayerMask( layerMask );
        object->SetUserId( userId );
        objects.push_back( object );
        caster.AddObject( *object );
        caster.Update();
        return object;
    }
};

/*-------------------------------------------------------------------------*/

/** Rejects the objects with one user id */
template < typename T >
class COcclusionRejectUserId : public CTGeoRayFilter< T >
{
    public:

    explicit COcclusionRejectUserId( UInt64 id ) : m_id( id ) {}

    virtual bool Accept( const CTGeoObjectBase< T >& object ) const GUCEF_VIRTUAL_OVERRIDE { return object.GetUserId() != m_id; }

    private:

    UInt64 m_id;
};

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
Near( const T& a, Float64 b, Float64 tolerance )
{
    return fabs( (Float64) a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

/** Without a ray caster or with an empty one nothing blocks: 0 and 0, the number of rays follows the settings */
template < typename T >
static bool
NoGeometry( void )
{
    CTAudioOcclusionEstimator< T > estimator;
    SAudioOcclusionEstimate< T > estimate;
    const CTPoint3D< T > source( (T) 0, (T) 0, (T) 0 );
    const CTPoint3D< T > listener( (T) 0, (T) 0, (T) 10 );
    estimator.Estimate( source, listener, estimate );
    if ( !Near( estimate.occlusion, 0.0, 0.0 ) || !Near( estimate.obstruction, 0.0, 0.0 ) || estimate.directBlocked || 0 != estimate.raysCast )
        return false;

    SOcclusionScene< T > scene;
    estimator.SetRayCaster( &scene.caster );
    estimator.SetSourceRadius( (T) 0.5 );
    estimator.SetListenerRadius( (T) 0.5 );
    estimator.SetRayCount( 64 );
    estimator.Estimate( source, listener, estimate );
    if ( !Near( estimate.occlusion, 0.0, 0.0 ) || !Near( estimate.obstruction, 0.0, 0.0 ) || estimate.directBlocked || 64 != estimate.raysCast || 0 != estimate.raysBlocked )
        return false;

    // a wall that is next to the path and one behind the listener do not count
    scene.AddWall( 5.0, 20.0, 40.0, -5.0, 5.0, 0xFFFFFFFFU, 1 );
    scene.AddWall( 15.0, -50.0, 50.0, -50.0, 50.0, 0xFFFFFFFFU, 2 );
    estimator.Estimate( source, listener, estimate );
    return Near( estimate.occlusion, 0.0, 0.0 ) && Near( estimate.obstruction, 0.0, 0.0 ) && !estimate.directBlocked;
}

/*-------------------------------------------------------------------------*/

/** A wall that covers every path gives 1 and 1, with point radii after a single ray, with spheres after N rays */
template < typename T >
static bool
FullWall( void )
{
    SOcclusionScene< T > scene;
    scene.AddWall( 5.0, -50.0, 50.0, -50.0, 50.0, 0xFFFFFFFFU, 1 );
    CTAudioOcclusionEstimator< T > estimator( &scene.caster );
    SAudioOcclusionEstimate< T > estimate;
    const CTPoint3D< T > source( (T) 0, (T) 0, (T) 0 );
    const CTPoint3D< T > listener( (T) 0, (T) 0, (T) 10 );

    estimator.Estimate( source, listener, estimate );
    if ( !Near( estimate.occlusion, 1.0, 0.0 ) || !Near( estimate.obstruction, 1.0, 0.0 ) || !estimate.directBlocked || 1 != estimate.raysCast || 1 != estimate.raysBlocked )
        return false;

    estimator.SetSourceRadius( (T) 1 );
    estimator.SetListenerRadius( (T) 1 );
    estimator.SetRayCount( 200 );
    estimator.SetOffsetPathCount( 16 );
    estimator.Estimate( source, listener, estimate );
    return Near( estimate.occlusion, 1.0, 0.0 ) && Near( estimate.obstruction, 1.0, 0.0 ) && estimate.directBlocked && 200 == estimate.raysCast && 200 == estimate.raysBlocked &&
           16 == estimate.offsetPathsCast && 0 == estimate.offsetPathsOpen;
}

/*-------------------------------------------------------------------------*/

/**
 *  A wall over the half plane x > 0 half way between two spheres of the same radius blocks the rays with s.x + l.x
 *  larger than 0 (up to the small tilt of the rays): by the symmetry x -> -x exactly half of the ray space. The
 *  estimator promises |estimate - 0.5| <= 1 / sqrt( N ); the estimates of 8 pattern start indices per N are checked.
 */
template < typename T >
static bool
HalfWall( Float64& worstRatio )
{
    SOcclusionScene< T > scene;
    scene.AddWall( 5.0, 0.0, 100.0, -100.0, 100.0, 0xFFFFFFFFU, 1 );
    CTAudioOcclusionEstimator< T > estimator( &scene.caster );
    estimator.SetSourceRadius( (T) 1 );
    estimator.SetListenerRadius( (T) 1 );
    const CTPoint3D< T > source( (T) 0, (T) 0, (T) 0 );
    const CTPoint3D< T > listener( (T) 0, (T) 0, (T) 10 );

    // the same ray space seen by an independent count: the share of ( s, l ) pairs that cross the plane at x > 0, from a much larger set of Sobol points
    UInt32 sizes[] = { 8, 16, 64, 256, 1024, 4096 };
    bool ok = true;
    for ( UInt32 n=0; n<sizeof( sizes ) / sizeof( sizes[ 0 ] ); ++n )
    {
        estimator.SetRayCount( sizes[ n ] );
        const Float64 bound = 1.0 / sqrt( (Float64) sizes[ n ] );
        for ( UInt32 start=0; start<8; ++start )
        {
            SAudioOcclusionEstimate< T > estimate;
            estimator.Estimate( source, listener, estimate, start * 5003 );
            const Float64 error = fabs( (Float64) estimate.occlusion - 0.5 );
            if ( error > worstRatio * bound )
                worstRatio = error / bound;
            if ( error > bound || sizes[ n ] != estimate.raysCast || !Near( estimate.occlusion, (Float64) estimate.raysBlocked / (Float64) estimate.raysCast, 1e-6 ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioOcclusion: half wall N " << sizes[ n ] << " start " << start << " estimate " << estimate.occlusion << " outside 0.5 +- " << bound );
                ok = false;
            }
        }
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A thin pillar blocks the direct path but most of the surroundings of the listener hear the source: a small obstruction */
template < typename T >
static bool
Pillar( void )
{
    SOcclusionScene< T > scene;
    scene.AddBox( 0.0, 0.0, 5.0, 0.1, 0xFFFFFFFFU, 1 );
    CTAudioOcclusionEstimator< T > estimator( &scene.caster );
    estimator.SetOffsetPathCount( 64 );
    estimator.SetOffsetRadius( (T) 1 );
    SAudioOcclusionEstimate< T > estimate;
    const CTPoint3D< T > source( (T) 0, (T) 0, (T) 0 );
    const CTPoint3D< T > listener( (T) 0, (T) 0, (T) 10 );
    estimator.Estimate( source, listener, estimate );
    if ( !estimate.directBlocked || !Near( estimate.occlusion, 1.0, 0.0 ) || 64 != estimate.offsetPathsCast )
        return false;
    // offset paths pass the pillar when the point is more than about 0.2 away from the axis: of the unit sphere that is most of the area
    const Float64 expectedOpen = (Float64) estimate.offsetPathsOpen / 64.0;
    if ( !( expectedOpen > 0.7 && expectedOpen < 1.0 ) || !Near( estimate.obstruction, 1.0 - expectedOpen, 1e-6 ) )
        return false;
    // counting open as soon as half of the surroundings hear the source removes the obstruction
    estimator.SetOpenShareForNoObstruction( (T) 0.5 );
    estimator.Estimate( source, listener, estimate );
    if ( !Near( estimate.obstruction, 0.0, 0.0 ) )
        return false;
    // invalid shares are refused, a share above 1 is limited to 1
    estimator.SetOpenShareForNoObstruction( (T) 0 );
    estimator.SetOpenShareForNoObstruction( (T) -1 );
    if ( !Near( estimator.GetOpenShareForNoObstruction(), 0.5, 0.0 ) )
        return false;
    estimator.SetOpenShareForNoObstruction( (T) 3 );
    return Near( estimator.GetOpenShareForNoObstruction(), 1.0, 0.0 );
}

/*-------------------------------------------------------------------------*/

/** Layer masks and filters decide what blocks */
template < typename T >
static bool
MasksAndFilters( void )
{
    SOcclusionScene< T > scene;
    scene.AddWall( 5.0, -50.0, 50.0, -50.0, 50.0, 0x4U, 77 );
    CTAudioOcclusionEstimator< T > estimator( &scene.caster );
    estimator.SetSourceRadius( (T) 0.5 );
    estimator.SetListenerRadius( (T) 0.5 );
    estimator.SetRayCount( 32 );
    SAudioOcclusionEstimate< T > estimate;
    const CTPoint3D< T > source( (T) 0, (T) 0, (T) 0 );
    const CTPoint3D< T > listener( (T) 0, (T) 0, (T) 10 );

    estimator.SetLayerMask( 0x4U );
    estimator.Estimate( source, listener, estimate );
    if ( !Near( estimate.occlusion, 1.0, 0.0 ) )
        return false;
    estimator.SetLayerMask( 0x3U );
    estimator.Estimate( source, listener, estimate );
    if ( !Near( estimate.occlusion, 0.0, 0.0 ) || estimate.directBlocked )
        return false;

    estimator.SetLayerMask( 0xFFFFFFFFU );
    COcclusionRejectUserId< T > filter( 77 );
    estimator.SetFilter( &filter );
    estimator.Estimate( source, listener, estimate );
    if ( !Near( estimate.occlusion, 0.0, 0.0 ) || estimate.directBlocked )
        return false;
    COcclusionRejectUserId< T > other( 78 );
    estimator.SetFilter( &other );
    estimator.Estimate( source, listener, estimate );
    return Near( estimate.occlusion, 1.0, 0.0 ) && estimator.GetFilter() == &other && 0xFFFFFFFFU == estimator.GetLayerMask();
}

/*-------------------------------------------------------------------------*/

/** The same arguments give bit identical estimates, the start index moves the pattern, settings are limited */
template < typename T >
static bool
DeterminismAndSettings( void )
{
    SOcclusionScene< T > scene;
    scene.AddWall( 5.0, 0.0, 100.0, -100.0, 100.0, 0xFFFFFFFFU, 1 );
    CTAudioOcclusionEstimator< T > estimator( &scene.caster );
    estimator.SetSourceRadius( (T) 1 );
    estimator.SetListenerRadius( (T) 1 );
    estimator.SetRayCount( 20 );
    const CTPoint3D< T > source( (T) 0, (T) 0, (T) 0 );
    const CTPoint3D< T > listener( (T) 0, (T) 0, (T) 10 );
    SAudioOcclusionEstimate< T > a;
    SAudioOcclusionEstimate< T > b;
    estimator.Estimate( source, listener, a, 3 );
    estimator.Estimate( source, listener, b, 3 );
    if ( a.raysBlocked != b.raysBlocked || a.occlusion != b.occlusion || a.offsetPathsOpen != b.offsetPathsOpen )
        return false;
    bool moved = false;
    for ( UInt32 start=1; start<40 && !moved; ++start )
    {
        estimator.Estimate( source, listener, b, 3 + start * 20 );
        moved = a.raysBlocked != b.raysBlocked;
    }
    if ( !moved )
        return false;

    estimator.SetRayCount( 0 );
    if ( 1 != estimator.GetRayCount() )
        return false;
    estimator.SetRayCount( 1000000 );
    if ( CTAudioOcclusionEstimator< T >::MaxSampleCount != estimator.GetRayCount() )
        return false;
    estimator.SetOffsetPathCount( 0 );
    if ( 1 != estimator.GetOffsetPathCount() )
        return false;
    estimator.SetSourceRadius( (T) -3 );
    estimator.SetListenerRadius( (T) -3 );
    estimator.SetOffsetRadius( (T) -3 );
    if ( !Near( estimator.GetSourceRadius(), 0.0, 0.0 ) || !Near( estimator.GetListenerRadius(), 0.0, 0.0 ) || !Near( estimator.GetOffsetRadius(), 0.0, 0.0 ) )
        return false;

    // the occlusion alone leaves the obstruction fields alone
    SAudioOcclusionEstimate< T > only;
    only.obstruction = (T) 0.25;
    estimator.EstimateOcclusion( source, listener, only, 0 );
    return Near( only.obstruction, 0.25, 0.0 ) && 1 == only.raysCast;
}

/*-------------------------------------------------------------------------*/

/** The owner thread of a scene that estimates and publishes in a loop */
class COcclusionOwnerThread : public MT::CActiveObject
{
    public:

    typedef SAudioOcclusionEstimate< Float32 > TEstimate;
    typedef MT::CTSlotDataExchange< TEstimate, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, 1 > TExchange;

    COcclusionOwnerThread( TExchange& exchange, UInt32 rounds ) : MT::CActiveObject(), m_exchange( exchange ), m_rounds( rounds ), m_done( false ), m_publishedBlocked( 0 ), m_publishedOpen( 0 ) {}

    virtual bool OnThreadStart( void* /*taskdata*/ ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* /*taskdata*/ ) GUCEF_VIRTUAL_OVERRIDE
    {
        // the scene lives and is queried on this thread alone
        SOcclusionScene< Float32 > scene;
        scene.AddWall( 5.0, -50.0, 50.0, -50.0, 50.0, 0xFFFFFFFFU, 1 );
        CTAudioOcclusionEstimator< Float32 > estimator( &scene.caster );
        estimator.SetSourceRadius( 0.5f );
        estimator.SetListenerRadius( 0.5f );
        estimator.SetRayCount( 16 );
        const CTPoint3D< Float32 > listener( 0.0f, 0.0f, 10.0f );
        for ( UInt32 i=0; i<m_rounds; ++i )
        {
            // the source alternates between behind the wall (blocked) and in front of it (open)
            const bool blocked = 0 == ( i & 1 );
            const CTPoint3D< Float32 > source( 0.0f, 0.0f, blocked ? 0.0f : 8.0f );
            TEstimate estimate;
            estimator.Estimate( source, listener, estimate, i );
            m_exchange.Write( estimate );
            if ( blocked )
                ++m_publishedBlocked;
            else
                ++m_publishedOpen;
        }
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsDone( void ) const { return m_done.Load( MT::MEMORDER_ACQUIRE ); }

    UInt32 GetPublishedBlocked( void ) const { return m_publishedBlocked; }
    UInt32 GetPublishedOpen( void ) const { return m_publishedOpen; }

    private:

    TExchange& m_exchange;
    UInt32 m_rounds;
    MT::CTAtomic< bool > m_done;
    UInt32 m_publishedBlocked;
    UInt32 m_publishedOpen;
};

/*-------------------------------------------------------------------------*/

/**
 *  The estimates are made on the owner thread and handed to a reader thread (the audio thread of the consumer) through
 *  a latest-only exchange: every value read is a whole estimate of one state (never torn) and the last one is the
 *  last published
 */
static bool
PublishedThroughExchange( UInt32 rounds )
{
    COcclusionOwnerThread::TExchange exchange;
    COcclusionOwnerThread owner( exchange, rounds );
    if ( !owner.Activate( GUCEF_NULL, 0, 0 ) )
        return false;

    UInt32 reads = 0;
    UInt32 torn = 0;
    COcclusionOwnerThread::TEstimate estimate;
    UInt32 waited = 0;
    bool finished = false;
    while ( !finished && waited < 30000 )
    {
        finished = owner.IsDone();
        while ( exchange.Read( estimate ) )
        {
            ++reads;
            const bool wholeBlocked = 16 == estimate.raysCast && 16 == estimate.raysBlocked && estimate.occlusion == 1.0f && estimate.directBlocked;
            const bool wholeOpen = 16 == estimate.raysCast && 0 == estimate.raysBlocked && estimate.occlusion == 0.0f && !estimate.directBlocked;
            if ( !wholeBlocked && !wholeOpen )
                ++torn;
        }
        if ( !finished )
        {
            MT::PrecisionDelay( 1 );
            ++waited;
        }
    }
    owner.WaitForThreadToFinish( 5000 );
    // after the owner is done the last published value is the last one that can be read
    while ( exchange.Read( estimate ) )
        ++reads;
    const bool lastWasOpen = 1 == ( ( rounds - 1 ) & 1 );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioOcclusion: " << reads << " of " << rounds << " published estimates were read, " << torn << " torn" );
    return finished && 0 == torn && reads > 0 && owner.GetPublishedBlocked() + owner.GetPublishedOpen() == rounds &&
           ( lastWasOpen ? 0.0f == estimate.occlusion : 1.0f == estimate.occlusion );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioOcclusionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AudioOcclusion" );

    GUCEF_TESTFW_TESTCASE( "AudioOcclusion: no geometry gives occlusion 0 and obstruction 0 (Float32, Float64)" )
    try
    {
        ASSERT_TRUE( NoGeometry< Float32 >() );
        ASSERT_TRUE( NoGeometry< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioOcclusion: a wall that covers every path gives occlusion 1 and obstruction 1" )
    try
    {
        ASSERT_TRUE( FullWall< Float32 >() );
        ASSERT_TRUE( FullWall< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioOcclusion: a half blocking wall gives 0.5 within 1 / sqrt( N ) for N = 8 to 4096 rays, several pattern start indices" )
    try
    {
        Float64 worst32 = 0.0;
        Float64 worst64 = 0.0;
        ASSERT_TRUE( HalfWall< Float32 >( worst32 ) );
        ASSERT_TRUE( HalfWall< Float64 >( worst64 ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioOcclusion: the worst error was " << worst32 << " (Float32) and " << worst64 << " (Float64) of the bound 1 / sqrt( N )" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioOcclusion: a thin pillar blocks the direct path but only partly obstructs; the open share setting removes it" )
    try
    {
        ASSERT_TRUE( Pillar< Float32 >() );
        ASSERT_TRUE( Pillar< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioOcclusion: layer masks and filters decide what blocks" )
    try
    {
        ASSERT_TRUE( MasksAndFilters< Float32 >() );
        ASSERT_TRUE( MasksAndFilters< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioOcclusion: estimates are deterministic, the start index moves the pattern, settings are limited" )
    try
    {
        ASSERT_TRUE( DeterminismAndSettings< Float32 >() );
        ASSERT_TRUE( DeterminismAndSettings< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioOcclusion: estimates made on the owner thread reach the reader through a latest-only exchange whole and in order" )
    try
    {
        ASSERT_TRUE( PublishedThroughExchange( ScaledCount( 20000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
