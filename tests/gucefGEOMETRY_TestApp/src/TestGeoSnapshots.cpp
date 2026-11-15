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

#ifndef GUCEF_MT_CTSLOTDATAEXCHANGE_H
#include "gucefMT_CTSlotDataExchange.h"
#define GUCEF_MT_CTSLOTDATAEXCHANGE_H
#endif /* GUCEF_MT_CTSLOTDATAEXCHANGE_H ? */

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

#include "TestGeometryAllocationCounter.h"
#include "TestGeoSpaceReference.h"
#include "TestGeoSnapshots.h"

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

/** Runs a function on its own thread */
class CSnapshotTestThread : public MT::CActiveObject
{
    public:

    typedef void ( *TFunction )( void* argument );

    CSnapshotTestThread( TFunction function, void* argument ) : MT::CActiveObject(), m_function( function ), m_argument( argument ), m_done( false ) {}

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

typedef MT::CTSlotDataExchange< TFloat32GeoSpaceSnapshot, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, 1 > TSnapshotExchange;

/** The state shared by the owner (main thread) and the reader thread */
struct SSnapshotStress
{
    TSnapshotExchange exchange;
    MT::CTAtomic< bool > stop;
    UInt32 reads;
    UInt32 newReads;
    UInt32 torn;
    UInt32 backwards;
    Float32 lastStep;

    SSnapshotStress( void ) : exchange(), stop( false ), reads( 0 ), newReads( 0 ), torn( 0 ), backwards( 0 ), lastStep( -1.0f ) {}

    /** The local rotation of owner step k: k / 64 radians about +Y */
    static CGeoSpace::TQuaternion StepRotation( Float32 k )
    {
        return CGeoSpace::TQuaternion::FromAxisAngle( CGeoSpace::TDirection::UnitY(), CTRadian< Float32 >( k / 64.0f ) );
    }

    /**
     *  A snapshot is consistent when every member comes from one owner step k: the node is at ( k, 2k, -k ) rotated
     *  k / 64 radians about +Y, so the position, the matrix translation, the velocity and the forward axis derived from
     *  the rotation agree bit for bit, and the world rotation is that of step k (decomposed from the world matrix, so
     *  within rounding: the cosine of half the angle between them above 1 - 10^-6, where one step apart gives
     *  1 - 3.8 * 10^-5)
     */
    static bool IsConsistentStep( const TFloat32GeoSpaceSnapshot& snapshot )
    {
        const Float32 k = snapshot.worldPosition.x;
        const CGeoSpace::TQuaternion expected = StepRotation( k );
        const Float64 alignment = fabs( (Float64) snapshot.worldRotation.x * expected.x + (Float64) snapshot.worldRotation.y * expected.y +
                                        (Float64) snapshot.worldRotation.z * expected.z + (Float64) snapshot.worldRotation.w * expected.w );
        return snapshot.worldPosition.y == 2.0f * k && snapshot.worldPosition.z == -k &&
               snapshot.worldMatrix.GetTranslation() == snapshot.worldPosition.ToPositionVector() &&
               snapshot.forward == snapshot.worldRotation.GetForward( CCoordinateSystem::GucefDefault() ) &&
               snapshot.velocity.x == k && alignment >= 1.0 - 1.0e-6;
    }

    /** The reader: about 1 kHz; every new snapshot is consistent and k never goes back */
    static void Read( void* argument )
    {
        SSnapshotStress& s = *static_cast< SSnapshotStress* >( argument );
        TFloat32GeoSpaceSnapshot snapshot;
        while ( !s.stop.Load( MT::MEMORDER_ACQUIRE ) )
        {
            const bool isNew = s.exchange.Read( snapshot );
            ++s.reads;
            if ( isNew )
            {
                ++s.newReads;
                const Float32 k = snapshot.worldPosition.x;
                s.torn += IsConsistentStep( snapshot ) ? 0 : 1;
                s.backwards += k < s.lastStep ? 1 : 0;
                s.lastStep = k;
            }
            MT::PrecisionDelay( 1 );
        }
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 8: the owner moves a node and publishes its snapshot after every move; a reader thread at about
 *  1 kHz never sees a torn snapshot (run under TSan in the w6b_tsan build)
 */
static bool
PublishToReaderThread( UInt32 durationMs )
{
    SSnapshotStress stress;
    CGeoSpace parent( CGeoSpace::TTransform( CGeoSpace::TVector(), CGeoSpace::TQuaternion(), CGeoSpace::TVector::One() ) );
    CGeoSpace node;
    node.SetParent( &parent, false );
    CSnapshotTestThread reader( &SSnapshotStress::Read, &stress );
    bool ok = reader.Activate( GUCEF_NULL, 0, 0 );
    const UInt64 start = MT::PrecisionTickCount();
    UInt32 steps = 0;
    while ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < (Float64) durationMs )
    {
        const Float32 k = (Float32) steps;
        node.SetLocalTransform( CGeoSpace::TTransform( CGeoSpace::TVector( k, 2.0f * k, -k ),
                                                       SSnapshotStress::StepRotation( k ), CGeoSpace::TVector::One() ) );
        TFloat32GeoSpaceSnapshot snapshot;
        node.CaptureSnapshot( snapshot, CCoordinateSystem::GucefDefault(), CGeoSpace::TVector( k, 0.0f, 0.0f ) );
        stress.exchange.Write( snapshot );
        ++steps;
        if ( 0 == steps % 64 )
            MT::AtomicCpuRelax();
    }
    stress.stop.Store( true, MT::MEMORDER_RELEASE );
    ok = reader.WaitUntilDone( 20000 ) && ok;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSnapshots: " << steps << " owner moves published in " << durationMs << " ms; the reader read " << stress.reads << " times, " << stress.newReads
               << " new snapshots, " << stress.torn << " torn, " << stress.backwards << " out of order" );
    return ok && 0 == stress.torn && 0 == stress.backwards && stress.newReads > 10 && steps > stress.newReads;
}

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 8: a listener node attached to a camera node: rotating the camera's parent 180 degrees about +Y
 *  flips the snapshot's forward from -Z to +Z (GUCEF default axes)
 */
template < typename T >
static bool
ListenerOnCamera( Float64 tolerance )
{
    typedef CTGeoSpace< T > TSpace;
    typedef CTVector3D< T > TVector;
    TSpace rig;
    TSpace camera( typename TSpace::TTransform( TVector( T( 0 ), T( 2 ), T( 5 ) ), CTQuaternion< T >(), TVector::One() ) );
    TSpace listener( typename TSpace::TTransform( TVector( T( 0 ), T( 0 ), T( -0.1 ) ), CTQuaternion< T >(), TVector::One() ) );
    camera.SetParent( &rig, false );
    listener.SetParent( &camera, false );
    CTGeoSpaceSnapshot< T > before;
    listener.CaptureSnapshot( before, CCoordinateSystem::GucefDefault() );
    rig.Rotate( CTQuaternion< T >::FromAxisAngle( CTDirectionVector3D< T >::UnitY(), CTRadian< T >( CTNumericTraits< T >::Pi() ) ), GEOSPACE_RELATIVETO_WORLD );
    CTGeoSpaceSnapshot< T > after;
    listener.CaptureSnapshot( after, CCoordinateSystem::GucefDefault() );
    const D3 f0 = WidenD( before.forward );
    const D3 f1 = WidenD( after.forward );
    const D3 r1 = WidenD( after.right );
    bool ok = ( f0.AsVector() - V3( 0.0, 0.0, -1.0 ) ).GetLength() <= tolerance && ( f1.AsVector() - V3( 0.0, 0.0, 1.0 ) ).GetLength() <= tolerance;
    ok = ok && ( r1.AsVector() - V3( -1.0, 0.0, 0.0 ) ).GetLength() <= tolerance && ( WidenD( after.up ).AsVector() - V3( 0.0, 1.0, 0.0 ) ).GetLength() <= tolerance;
    /* the listener moved with the rig: from ( 0, 2, 4.9 ) to ( 0, 2, -4.9 ) */
    ok = ok && ( Widen( after.worldPosition ) - P3( 0.0, 2.0, -4.9 ) ).GetLength() <= tolerance * 10.0;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Snapshot sets: capture without allocation into the sized storage, caller arrays, velocities and the capacity limit */
static bool
SnapshotSets( void )
{
    CRandom r( 9801 );
    CGeoSpace root( RandomTransform< Float32 >( r, 5.0, false, 0.5, 2.0, 0.0 ) );
    CGeoSpace nodes[ 8 ];
    const CGeoSpace* pointers[ 8 ];
    CGeoSpace::TVector velocities[ 8 ];
    for ( UInt32 k=0; k<8; ++k )
    {
        nodes[ k ].SetLocalTransform( RandomTransform< Float32 >( r, 5.0, false, 0.5, 2.0, 0.0 ) );
        nodes[ k ].SetParent( 0 == k ? &root : &nodes[ k - 1 ], false );
        pointers[ k ] = &nodes[ k ];
        velocities[ k ] = CGeoSpace::TVector( (Float32) k, 0.0f, 1.0f );
    }
    CGeoSpaceSnapshotSet set( 4 );
    bool ok = !set.Capture( pointers, 8, CCoordinateSystem::GucefDefault(), GUCEF_NULL ) && 0 == set.GetCount();
    set.SetCapacity( 8 );
    ok = ok && set.Capture( pointers, 8, CCoordinateSystem::GucefDefault(), velocities ) && 8 == set.GetCount();
    TFloat32GeoSpaceSnapshot single;
    for ( UInt32 k=0; k<8; ++k )
    {
        nodes[ k ].CaptureSnapshot( single, CCoordinateSystem::GucefDefault(), velocities[ k ] );
        ok = ok && single == set.GetSnapshot( k );
    }
    TFloat32GeoSpaceSnapshot array[ 3 ];
    ok = ok && CGeoSpaceSnapshotSet::CaptureInto( pointers + 2, 3, CCoordinateSystem::GucefDefault(), GUCEF_NULL, array ) && array[ 1 ].worldMatrix == nodes[ 3 ].GetWorldMatrix();
    const CGeoSpace* withNull[ 2 ] = { &nodes[ 0 ], GUCEF_NULL };
    ok = ok && !CGeoSpaceSnapshotSet::CaptureInto( withNull, 2, CCoordinateSystem::GucefDefault(), GUCEF_NULL, array );
    /* a moved node is seen in the next capture */
    root.Translate( CGeoSpace::TVector( 1.0f, 0.0f, 0.0f ), GEOSPACE_RELATIVETO_WORLD );
    ok = ok && set.Capture( pointers, 8, CCoordinateSystem::GucefDefault(), GUCEF_NULL ) && set.GetData()[ 7 ].worldMatrix == nodes[ 7 ].GetWorldMatrix();
    ok = ok && CGeoSpace::TVector() == set.GetSnapshot( 7 ).velocity;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Snapshots of other coordinate systems and of fixed-point trees */
static bool
SnapshotAxesAndKinds( void )
{
    CGeoSpace node( CGeoSpace::TTransform( CGeoSpace::TVector( 1.0f, 2.0f, 3.0f ), CGeoSpace::TQuaternion(), CGeoSpace::TVector::One() ) );
    TFloat32GeoSpaceSnapshot unreal;
    node.CaptureSnapshot( unreal, CCoordinateSystem::Unreal() );
    const CTVector3D< Float32 > forward = CCoordinateSystem::Unreal().GetForward< Float32 >();
    bool ok = unreal.forward.AsVector() == forward && unreal.worldPosition == CGeoSpace::TPoint( 1.0f, 2.0f, 3.0f );

    CGeoSpaceFixed fixedRoot( CGeoSpaceFixed::TTransform( CGeoSpaceFixed::TVector( TFixed32_32( 3 ), TFixed32_32( 0 ), TFixed32_32( 0 ) ),
                                                          CTQuaternion< TFixed32_32 >::FromAxisAngle( CTDirectionVector3D< TFixed32_32 >::UnitY(), CTRadian< TFixed32_32 >( CTNumericTraits< TFixed32_32 >::Pi() ) ),
                                                          CGeoSpaceFixed::TVector::One() ) );
    CGeoSpaceFixed fixedChild( CGeoSpaceFixed::TTransform( CGeoSpaceFixed::TVector( TFixed32_32( 1 ), TFixed32_32( 0 ), TFixed32_32( 0 ) ), CTQuaternion< TFixed32_32 >(),
                                                           CGeoSpaceFixed::TVector::One() ) );
    fixedChild.SetParent( &fixedRoot, false );
    TFixed32_32GeoSpaceSnapshot fixedSnapshot;
    fixedChild.CaptureSnapshot( fixedSnapshot, CCoordinateSystem::GucefDefault() );
    ok = ok && fabs( FixedToFloat64( fixedSnapshot.worldPosition.x ) - 2.0 ) <= 1.0e-8 && fabs( FixedToFloat64( fixedSnapshot.forward.GetZ() ) - 1.0 ) <= 1.0e-8;
    TFloat32GeoSpaceSnapshot converted;
    fixedChild.CaptureSnapshotRelativeTo( converted, CGeoSpaceFixed::TPoint( TFixed32_32( 2 ), TFixed32_32( 0 ), TFixed32_32( 0 ) ), CCoordinateSystem::GucefDefault() );
    ok = ok && fabs( converted.worldPosition.x ) <= 1.0e-6f;
    CGeoSpaceSnapshotSetFixed fixedSet( 1 );
    const CGeoSpaceFixed* fixedPointers[ 1 ] = { &fixedChild };
    ok = ok && fixedSet.Capture( fixedPointers, 1, CCoordinateSystem::GucefDefault(), GUCEF_NULL ) && fixedSet.GetSnapshot( 0 ) == fixedSnapshot;
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  The consistency check of the reader detects a tear in any group of fields: snapshots of one step pass, a snapshot
 *  whose rotation fields (world rotation, matrix, axes) come from the next step fails, and so does one whose
 *  position comes from another step
 */
static bool
TornSnapshotsAreDetected( void )
{
    CGeoSpace node;
    bool ok = true;
    for ( UInt32 step=0; step<2000; step+=37 )
    {
        const Float32 k = (Float32) step;
        TFloat32GeoSpaceSnapshot current;
        TFloat32GeoSpaceSnapshot next;
        node.SetLocalTransform( CGeoSpace::TTransform( CGeoSpace::TVector( k, 2.0f * k, -k ), SSnapshotStress::StepRotation( k ), CGeoSpace::TVector::One() ) );
        node.CaptureSnapshot( current, CCoordinateSystem::GucefDefault(), CGeoSpace::TVector( k, 0.0f, 0.0f ) );
        node.SetLocalTransform( CGeoSpace::TTransform( CGeoSpace::TVector( k, 2.0f * k, -k ), SSnapshotStress::StepRotation( k + 1.0f ), CGeoSpace::TVector::One() ) );
        node.CaptureSnapshot( next, CCoordinateSystem::GucefDefault(), CGeoSpace::TVector( k, 0.0f, 0.0f ) );
        ok = ok && SSnapshotStress::IsConsistentStep( current );
        /* the position of step k with every rotation field of step k + 1: only the world rotation reveals it */
        ok = ok && !SSnapshotStress::IsConsistentStep( next );
        TFloat32GeoSpaceSnapshot movedPosition = current;
        movedPosition.worldPosition = CGeoSpace::TPoint( k + 1.0f, 2.0f * k, -k );
        ok = ok && !SSnapshotStress::IsConsistentStep( movedPosition );
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

typedef MT::CTSlotDataExchange< CGeoSpaceSnapshotSet, MT::SSlotDeliveryLatestOnly, MT::SSlotFullFail, 1 > TSnapshotSetExchange;

/** Captures, publishes and reads one set of 'count' nodes; true when the read set equals the published one */
static bool
PublishSnapshotSet( TSnapshotSetExchange& exchange, CGeoSpaceSnapshotSet& published, CGeoSpaceSnapshotSet& received, const CGeoSpace* const* nodes, UInt32 count )
{
    if ( !published.Capture( nodes, count, CCoordinateSystem::GucefDefault(), GUCEF_NULL ) )
        return false;
    exchange.Write( published );
    if ( !exchange.Read( received ) || received.GetCount() != count )
        return false;
    for ( UInt32 i=0; i<count; ++i )
        if ( !( received.GetSnapshot( i ) == published.GetSnapshot( i ) ) )
            return false;
    return true;
}

/**
 *  A snapshot set published through a latest-only slot exchange: after the warm-up (each slot and the reader's set
 *  received one set of the same capacity) capture, publish and read do not allocate; the warm-up does
 */
static bool
SnapshotSetExchangeSteadyStateDoesNotAllocate( UInt32& warmUpAllocations, UInt32& allocations )
{
    const UInt32 count = 16;
    CGeoSpace root;
    CGeoSpace nodes[ 16 ];
    const CGeoSpace* pointers[ 16 ];
    for ( UInt32 k=0; k<count; ++k )
    {
        nodes[ k ].SetLocalPosition( CGeoSpace::TVector( (Float32) k, 1.0f, 0.0f ) );
        nodes[ k ].SetParent( &root, false );
        pointers[ k ] = &nodes[ k ];
    }
    TSnapshotSetExchange exchange;
    CGeoSpaceSnapshotSet published( count );
    CGeoSpaceSnapshotSet received( count );
    bool ok = true;
    /* the slots of the exchange start empty: the first copies into them allocate */
    GeometryAllocationCounter::StartCounting();
    for ( UInt32 i=0; i<8; ++i )
    {
        root.Translate( CGeoSpace::TVector( 0.5f, 0.0f, 0.0f ), GEOSPACE_RELATIVETO_WORLD );
        ok = ok && PublishSnapshotSet( exchange, published, received, pointers, count );
    }
    warmUpAllocations = GeometryAllocationCounter::StopCounting();
    GeometryAllocationCounter::StartCounting();
    for ( UInt32 i=0; i<200; ++i )
    {
        root.Translate( CGeoSpace::TVector( 0.5f, 0.0f, 0.0f ), GEOSPACE_RELATIVETO_WORLD );
        ok = ok && PublishSnapshotSet( exchange, published, received, pointers, count );
    }
    allocations = GeometryAllocationCounter::StopCounting();
    return ok && received.GetSnapshot( count - 1 ).worldMatrix == nodes[ count - 1 ].GetWorldMatrix();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeoSnapshotsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeoSnapshots" );

    GUCEF_TESTFW_TESTCASE( "GeoSnapshots: published through a latest-only slot exchange to a 1 kHz reader thread while the owner moves the node: never torn (the check detects torn position and rotation fields)" )
    try
    {
        ASSERT_TRUE( TornSnapshotsAreDetected() );
        ASSERT_TRUE( PublishToReaderThread( 1000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSnapshots: a listener on a camera: rotating the camera's parent 180 degrees about +Y flips forward from -Z to +Z" )
    try
    {
        ASSERT_TRUE( ListenerOnCamera< Float32 >( 1.0e-5 ) );
        ASSERT_TRUE( ListenerOnCamera< Float64 >( 1.0e-12 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSnapshots: a snapshot set published through a latest-only slot exchange does not allocate after the warm-up" )
    GUCEF_TESTFW_SKIP_IF( !GeometryAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator" );
    try
    {
        GeometryAllocationCounter::StartCounting();
        GeometryAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == GeometryAllocationCounter::StopCounting() );
        UInt32 warmUpAllocations = 0;
        UInt32 allocations = 0;
        ASSERT_TRUE( SnapshotSetExchangeSteadyStateDoesNotAllocate( warmUpAllocations, allocations ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSnapshots: " << warmUpAllocations << " allocations in the warm-up, " << allocations << " in 200 steady-state publications of a snapshot set" );
        ASSERT_TRUE( warmUpAllocations > 0 );
        ASSERT_TRUE( 0 == allocations );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSnapshots: snapshot sets, caller arrays, velocities, other axes and the TFixed32_32 snapshots" )
    try
    {
        ASSERT_TRUE( SnapshotSets() );
        ASSERT_TRUE( SnapshotAxesAndKinds() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
