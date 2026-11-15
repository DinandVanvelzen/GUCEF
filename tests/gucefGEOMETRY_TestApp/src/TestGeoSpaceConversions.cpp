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
#include "TestGeoSpaceConversions.h"

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

typedef TFixed32_32 TFixed;
typedef CTGeoSpace< TFixed > TFixedSpace;

/**
 *  The pinned FNV-1a hash of the TFixed32_32 transform tree run (world and inverse world matrices, precise relative
 *  transforms, conversions, snapshots, a rebase): identical in every build (gcc, clang, ASan, TSan,
 *  GUCEF_MATH_INLINE_ALL, RTSan)
 */
static const UInt64 PinnedFixedGeoSpaceHash = 11974490426398452110ULL;

/*-------------------------------------------------------------------------*/

/** A random forest: node k hangs under a random earlier node or under 'root' */
template < typename T >
static void
BuildTree( CRandom& r, CTGeoSpace< T >& root, CTGeoSpace< T >* nodes, UInt32 count, Float64 range, bool uniform, Float64 mirror )
{
    for ( UInt32 k=0; k<count; ++k )
    {
        nodes[ k ].SetLocalTransform( RandomTransform< T >( r, range, uniform, 0.5, 2.0, mirror ) );
        const Int32 parentIndex = r.Index( (Int32) k + 1 ) - 1;
        nodes[ k ].SetParent( parentIndex < 0 ? &root : &nodes[ parentIndex ], false );
    }
}

/*-------------------------------------------------------------------------*/

static Float64
PointError( const P3& value, const P3& reference )
{
    const Float64 scale = 1.0 + reference.ToPositionVector().GetLength();
    return ( value - reference ).GetLength() / scale;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 4: ToLocal( ToWorld( p ) ) = p for seeded points and hierarchies */
template < typename T >
static bool
RoundTrips( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    Float64 worst = 0.0;
    UInt32 bad = 0;
    UInt32 done = 0;
    while ( done < count )
    {
        CTGeoSpace< T > root( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T > nodes[ 4 ];
        BuildTree( r, root, nodes, 4, 10.0, false, 0.2 );
        for ( UInt32 i=0; i<100 && done < count; ++i, ++done )
        {
            const CTGeoSpace< T >& node = nodes[ r.Index( 4 ) ];
            const P3 p = r.Point( 10.0 );
            const Float64 d = PointError( Widen( node.ToLocal( node.ToWorld( ToP< T >( p ) ) ) ), Widen( ToP< T >( p ) ) );
            worst = d > worst ? d : worst;
            bad += d > tolerance ? 1 : 0;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpaceConversions: " << count << " round trips (kind size " << (UInt32) sizeof( T ) << "), seed " << seed << ": worst relative error " << worst );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 4: ToSpace between cousins equals the route through world bit for bit, and both ToSpace and
 *  ToSpacePrecise agree with the Float64 product of the reference matrices
 */
template < typename T >
static bool
CousinConversions( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    Float64 worstWorld = 0.0;
    Float64 worstPrecise = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTGeoSpace< T > root( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T > left( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T > right( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T > a( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T > b( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
        left.SetParent( &root, false );
        right.SetParent( &root, false );
        a.SetParent( &left, false );
        b.SetParent( &right, false );
        const P3 p = r.Point( 10.0 );
        const V3 v = r.Vector( 10.0 );
        const CTPoint3D< T > viaSpace = a.ToSpace( b, ToP< T >( p ) );
        const CTPoint3D< T > viaWorld = b.ToLocal( a.ToWorld( ToP< T >( p ) ) );
        const CTVector3D< T > vectorViaSpace = a.ToSpace( b, ToV< T >( v ) );
        const CTVector3D< T > vectorViaWorld = b.ToLocal( a.ToWorld( ToV< T >( v ) ) );
        bool ok = viaSpace == viaWorld && vectorViaSpace == vectorViaWorld;

        const M4 relative = ReferenceWorld( b ).InverseAffine() * ReferenceWorld( a );
        const P3 reference = relative.TransformPoint( Widen( ToP< T >( p ) ) );
        const Float64 worldError = PointError( Widen( viaSpace ), reference );
        const Float64 preciseError = PointError( Widen( a.ToSpacePrecise( b, ToP< T >( p ) ) ), reference );
        const Float64 vectorError = ( Widen( a.ToSpacePrecise( b, ToV< T >( v ) ) ) - relative.TransformVector( Widen( ToV< T >( v ) ) ) ).GetLength() / ( 1.0 + v.GetLength() );
        const Float64 matrixError = RelativeDifference( Widen( a.GetTransformToPrecise( b ) ), relative );
        worstWorld = worldError > worstWorld ? worldError : worstWorld;
        worstPrecise = preciseError > worstPrecise ? preciseError : worstPrecise;
        ok = ok && worldError <= tolerance && preciseError <= tolerance && vectorError <= tolerance && matrixError <= tolerance;
        /* the identity for itself, the direct relations to an ancestor in both directions */
        ok = ok && a.GetTransformToPrecise( a ).IsIdentity();
        ok = ok && RelativeDifference( Widen( a.GetTransformToPrecise( root ) ), ReferenceWorld( root ).InverseAffine() * ReferenceWorld( a ) ) <= tolerance;
        ok = ok && RelativeDifference( Widen( root.GetTransformToPrecise( a ) ), ReferenceWorld( a ).InverseAffine() * ReferenceWorld( root ) ) <= tolerance;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpaceConversions: cousins case " << i << " world error " << worldError << " precise error " << preciseError << " matrix error " << matrixError << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpaceConversions: cousins seed " << seed << ": worst relative error through world " << worstWorld << ", precise " << worstPrecise );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 4: two Float32 siblings 10^6 from the origin. Under a far parent the precise route never forms the
 *  large world coordinates; as far children of one root their translations are subtracted first. The error of the
 *  route through world is reported.
 */
static bool
FarSiblings( UInt64 seed, UInt32 count )
{
    CRandom r( seed );
    UInt32 bad = 0;
    Float64 worstPrecise[ 2 ] = { 0.0, 0.0 };
    Float64 worstWorld[ 2 ] = { 0.0, 0.0 };
    for ( UInt32 i=0; i<count; ++i )
    {
        for ( UInt32 layout=0; layout<2; ++layout )
        {
            const V3 far( 1.0e6, -1.0e6, 1.0e6 );
            CGeoSpace root;
            CGeoSpace parent( CGeoSpace::TTransform( ToV< Float32 >( layout == 0 ? far : V3() ), ToQ< Float32 >( r.Rotation() ), CGeoSpace::TVector::One() ) );
            parent.SetParent( &root, false );
            CGeoSpace a( RandomTransform< Float32 >( r, 10.0, true, 1.0, 1.0, 0.0 ) );
            CGeoSpace b( RandomTransform< Float32 >( r, 10.0, true, 1.0, 1.0, 0.0 ) );
            if ( 1 == layout )
            {
                a.SetLocalPosition( a.GetLocalTransform().position + ToV< Float32 >( far ) );
                b.SetLocalPosition( b.GetLocalTransform().position + ToV< Float32 >( far ) );
            }
            a.SetParent( &parent, false );
            b.SetParent( &parent, false );
            const P3 p = r.Point( 5.0 );
            const P3 reference = ( ReferenceWorld( b ).InverseAffine() * ReferenceWorld( a ) ).TransformPoint( Widen( ToP< Float32 >( p ) ) );
            const Float64 precise = ( Widen( a.ToSpacePrecise( b, ToP< Float32 >( p ) ) ) - reference ).GetLength();
            const Float64 world = ( Widen( a.ToSpace( b, ToP< Float32 >( p ) ) ) - reference ).GetLength();
            worstPrecise[ layout ] = precise > worstPrecise[ layout ] ? precise : worstPrecise[ layout ];
            worstWorld[ layout ] = world > worstWorld[ layout ] ? world : worstWorld[ layout ];
            bad += precise > 1.0e-3 ? 1 : 0;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpaceConversions: Float32 siblings 10^6 from the origin, seed " << seed << ": under a far parent the precise error is "
               << worstPrecise[ 0 ] << " (through world " << worstWorld[ 0 ] << "), as far children of one parent " << worstPrecise[ 1 ] << " (through world " << worstWorld[ 1 ] << ")" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 4: normals transform with the inverse transpose under non-uniform scale and shear */
template < typename T >
static bool
NormalsUnderNonUniformScale( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    UInt32 directionsOff = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTGeoSpace< T > parent( RandomTransform< T >( r, 5.0, false, 0.3, 3.0, 0.2 ) );
        CTGeoSpace< T > node( RandomTransform< T >( r, 5.0, false, 0.3, 3.0, 0.2 ) );
        node.SetParent( &parent, false );
        const D3 n = r.Direction();
        const V3 t1 = n.AsVector().Cross( fabs( n.GetX() ) < 0.9 ? V3( 1.0, 0.0, 0.0 ) : V3( 0.0, 1.0, 0.0 ) );
        const V3 t2 = n.AsVector().Cross( t1 );
        const D3 worldNormal = WidenD( node.NormalToWorld( ToD< T >( n ) ) );
        const V3 w1 = Widen( node.ToWorld( ToV< T >( t1 ) ) );
        const V3 w2 = Widen( node.ToWorld( ToV< T >( t2 ) ) );
        const Float64 d1 = fabs( worldNormal.AsVector().Dot( w1 ) ) / w1.GetLength();
        const Float64 d2 = fabs( worldNormal.AsVector().Dot( w2 ) ) / w2.GetLength();
        /* and back: the local normal of the world normal is the original */
        const D3 back = WidenD( node.NormalToLocal( ToD< T >( worldNormal ) ) );
        const bool ok = d1 <= tolerance && d2 <= tolerance && fabs( worldNormal.AsVector().GetLength() - 1.0 ) <= tolerance && ( back.AsVector() - n.AsVector() ).GetLength() <= tolerance * 10.0;
        /* a direction transformed as a direction is generally not perpendicular: the reason for the separate member */
        const D3 asDirection = WidenD( node.ToWorld( ToD< T >( n ) ) );
        directionsOff += fabs( asDirection.AsVector().Dot( w1 ) ) / w1.GetLength() > 1.0e-3 ? 1 : 0;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpaceConversions: normal case " << i << " dots " << d1 << ", " << d2 << ", seed " << seed );
        }
    }
    return 0 == bad && directionsOff > count / 2;
}

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 5 in Float32: RebaseOrigin changes only the local positions of the root's children, moves every
 *  world position by -offset, and keeps every precise relative transform between non-root nodes bit for bit (the
 *  positions and offsets are multiples of 1/8, so every subtraction of the rebase is exact)
 */
static bool
RebaseFloat32( UInt64 seed, UInt32 count )
{
    CRandom r( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 n = 24;
        CGeoSpace root;
        CGeoSpace nodes[ 24 ];
        for ( UInt32 k=0; k<n; ++k )
        {
            CGeoSpace::TTransform t = RandomTransform< Float32 >( r, 1.0, false, 0.5, 2.0, 0.1 );
            const Float64 x = floor( r.Uniform( -8000.0, 8000.0 ) ) / 8.0;
            const Float64 y = floor( r.Uniform( -8000.0, 8000.0 ) ) / 8.0;
            const Float64 z = floor( r.Uniform( -8000.0, 8000.0 ) ) / 8.0;
            t.position = CGeoSpace::TVector( (Float32) x, (Float32) y, (Float32) z );
            nodes[ k ].SetLocalTransform( t );
            const Int32 parentIndex = r.Index( (Int32) k + 1 ) - 1;
            nodes[ k ].SetParent( parentIndex < 0 ? &root : &nodes[ parentIndex ], false );
        }
        CGeoSpace::TTransform locals[ 24 ];
        CGeoSpace::TPoint worlds[ 24 ];
        CGeoSpace::TMatrix relatives[ 24 ];
        for ( UInt32 k=0; k<n; ++k )
        {
            locals[ k ] = nodes[ k ].GetLocalTransform();
            worlds[ k ] = nodes[ k ].GetWorldPosition();
            relatives[ k ] = nodes[ k ].GetTransformToPrecise( nodes[ ( k * 7 + 3 ) % n ] );
        }
        const Float64 ox = floor( r.Uniform( -8000.0, 8000.0 ) ) / 8.0;
        const Float64 oy = floor( r.Uniform( -8000.0, 8000.0 ) ) / 8.0;
        const Float64 oz = floor( r.Uniform( -8000.0, 8000.0 ) ) / 8.0;
        const CGeoSpace::TVector offset( (Float32) ox, (Float32) oy, (Float32) oz );
        CGeoSpace::RebaseOrigin( root, offset );
        bool ok = true;
        for ( UInt32 k=0; k<n; ++k )
        {
            const bool rootChild = nodes[ k ].GetParent() == &root;
            CGeoSpace::TTransform expected = locals[ k ];
            if ( rootChild )
                expected.position = expected.position - offset;
            ok = ok && nodes[ k ].GetLocalTransform() == expected;
            ok = ok && PointError( Widen( nodes[ k ].GetWorldPosition() ), Widen( worlds[ k ] ) - Widen( offset ) ) <= 1.0e-5;
            ok = ok && SameMatrixBits( nodes[ k ].GetTransformToPrecise( nodes[ ( k * 7 + 3 ) % n ] ), relatives[ k ] );
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpaceConversions: Float32 rebase case " << i << " failed, seed " << seed );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 5: CGeoSpace64 keeps 0.01 resolution at 10^9 */
static bool
Float64Resolution( void )
{
    CGeoSpace64 root;
    CGeoSpace64 far( CGeoSpace64::TTransform( CGeoSpace64::TVector( 1.0e9, -1.0e9, 1.0e9 ), ToQ< Float64 >( Q4::FromAxisAngle( D3::UnitY(), CTRadian< Float64 >( 0.7 ) ) ),
                                              CGeoSpace64::TVector::One() ) );
    far.SetParent( &root, false );
    CGeoSpace64 steps[ 10 ];
    bool ok = true;
    for ( UInt32 k=0; k<10; ++k )
    {
        steps[ k ].SetLocalPosition( CGeoSpace64::TVector( 0.01 * (Float64) k, 0.0, 0.0 ) );
        steps[ k ].SetParent( &far, false );
    }
    for ( UInt32 k=1; k<10; ++k )
    {
        const Float64 step = ( steps[ k ].GetWorldPosition() - steps[ k - 1 ].GetWorldPosition() ).GetLength();
        ok = ok && fabs( step - 0.01 ) <= 1.0e-6;
        const CGeoSpace64::TPoint local = far.ToLocal( steps[ k ].GetWorldPosition() );
        ok = ok && fabs( local.x - 0.01 * (Float64) k ) <= 1.0e-6 && fabs( local.y ) <= 1.0e-6;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 5: Float32 snapshots of a Float64 tree relative to an origin are exact to Float32 precision */
static bool
RelativeSnapshots( UInt64 seed, UInt32 count )
{
    CRandom r( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const P3 rootPosition = r.Point( 1.0e9 );
        const Q4 rootRotation = r.Rotation();
        CGeoSpace64 root( CGeoSpace64::TTransform( ToV< Float64 >( rootPosition.ToPositionVector() ), ToQ< Float64 >( rootRotation ), CGeoSpace64::TVector::One() ) );
        CGeoSpace64 node( RandomTransform< Float64 >( r, 100.0, true, 0.5, 2.0, 0.0 ) );
        node.SetParent( &root, false );
        const CGeoSpace64::TPoint world = node.GetWorldPosition();
        const Float64 dx = r.Uniform( -50.0, 50.0 );
        const Float64 dy = r.Uniform( -50.0, 50.0 );
        const Float64 dz = r.Uniform( -50.0, 50.0 );
        const CGeoSpace64::TPoint origin( world.x + dx, world.y + dy, world.z + dz );
        TFloat32GeoSpaceSnapshot snapshot;
        node.CaptureSnapshotRelativeTo( snapshot, origin, CCoordinateSystem::GucefDefault() );
        const CTVector3D< Float64 > relative = world - origin;
        bool ok = snapshot.worldPosition.x == (Float32) relative.x && snapshot.worldPosition.y == (Float32) relative.y && snapshot.worldPosition.z == (Float32) relative.z;
        ok = ok && snapshot.worldMatrix.GetTranslation() == snapshot.worldPosition.ToPositionVector();
        ok = ok && Widen( snapshot.worldRotation ).EqualsRotation( Widen( node.GetWorldRotation() ), CTRadian< Float64 >( 1.0e-6 ) );
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpaceConversions: relative snapshot case " << i << " not exact, seed " << seed );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** The Float64 reference of a fixed-point node chain */
static M4
FixedReferenceWorld( const TFixedSpace& node )
{
    M4 world;
    for ( const TFixedSpace* p = &node; GUCEF_NULL != p; p = p->GetParent() )
    {
        const TFixedSpace::TTransform& t = p->GetLocalTransform();
        const M4 local = M4::FromTRS( V3( FixedToFloat64( t.position.x ), FixedToFloat64( t.position.y ), FixedToFloat64( t.position.z ) ),
                                      Q4::FromXYZW( FixedToFloat64( t.rotation.x ), FixedToFloat64( t.rotation.y ), FixedToFloat64( t.rotation.z ), FixedToFloat64( t.rotation.w ) ),
                                      V3( FixedToFloat64( t.scale.x ), FixedToFloat64( t.scale.y ), FixedToFloat64( t.scale.z ) ) );
        world = local * world;
    }
    return world;
}

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 5 with amendment A-1: a TFixed32_32 tree from integer draws agrees with Float64, its caches,
 *  conversions and snapshots hash to the pinned value, and RebaseOrigin moves every world position by exactly -offset
 *  and keeps every precise relative transform between non-root nodes bit for bit
 */
static bool
FixedPointTreeRun( UInt64& hash )
{
    CFixedRandom random( 9601 );
    CRawHash raw;
    const UInt32 n = 200;
    TFixedSpace root;
    GUCEF::vector< TFixedSpace* > nodes;
    for ( UInt32 k=0; k<n; ++k )
    {
        const TFixed s = random.Uniform( 32768, 131072 );
        const TFixed sy = random.Uniform( 32768, 131072 );
        const bool uniform = 0 == random.Index( 2 );
        /* every draw into a local first: the order of evaluation of arguments differs between compilers */
        const TFixedSpace::TVector position = random.Vector( 100 * 65536 );
        const CTQuaternion< TFixed > rotation = random.Rotation();
        const TFixedSpace::TTransform t( position, rotation, TFixedSpace::TVector( s, uniform ? s : sy, s ) );
        nodes.push_back( new TFixedSpace( t ) );
        const UInt32 parentIndex = random.Index( k + 1 );
        nodes[ k ]->SetParent( 0 == parentIndex ? &root : nodes[ parentIndex - 1 ], false );
    }
    UInt32 bad = 0;
    Float64 worst = 0.0;
    for ( UInt32 k=0; k<n; ++k )
    {
        const Float64 d = RelativeDifference( Widen( nodes[ k ]->GetWorldMatrix() ), FixedReferenceWorld( *nodes[ k ] ) );
        worst = d > worst ? d : worst;
        bad += d > 1.0e-6 ? 1 : 0;
        HashMatrix( raw, nodes[ k ]->GetWorldMatrix() );
        HashMatrix( raw, nodes[ k ]->GetInverseWorldMatrix() );
        HashMatrix( raw, nodes[ k ]->GetTransformToPrecise( *nodes[ random.Index( n ) ] ) );
        const TFixedSpace::TPoint p = random.Point( 50 * 65536 );
        raw.Add3( nodes[ k ]->ToLocal( nodes[ k ]->ToWorld( p ) ) );
        raw.Add3( nodes[ k ]->ToSpace( *nodes[ random.Index( n ) ], p ) );
        raw.Add3( nodes[ k ]->NormalToWorld( TFixedSpace::TDirection::UnitY() ).AsVector() );
        TFixed32_32GeoSpaceSnapshot snapshot;
        nodes[ k ]->CaptureSnapshot( snapshot, CCoordinateSystem::GucefDefault() );
        HashMatrix( raw, snapshot.worldMatrix );
        raw.Add3( snapshot.worldPosition );
        raw.Add( snapshot.worldRotation.x );
        raw.Add( snapshot.worldRotation.y );
        raw.Add( snapshot.worldRotation.z );
        raw.Add( snapshot.worldRotation.w );
        raw.Add3( snapshot.forward.AsVector() );
        raw.Add3( snapshot.up.AsVector() );
    }

    /* rebase: exact shift of every world position, precise relative transforms unchanged bit for bit */
    GUCEF::vector< TFixedSpace::TPoint > worlds;
    GUCEF::vector< TFixedSpace::TMatrix > relatives;
    GUCEF::vector< UInt32 > partners;
    for ( UInt32 k=0; k<n; ++k )
    {
        worlds.push_back( nodes[ k ]->GetWorldPosition() );
        partners.push_back( random.Index( n ) );
        relatives.push_back( nodes[ k ]->GetTransformToPrecise( *nodes[ partners[ k ] ] ) );
    }
    const TFixedSpace::TVector offset = random.Vector( 1000 * 65536 );
    TFixedSpace::RebaseOrigin( root, offset );
    UInt32 shifted = 0;
    UInt32 unchanged = 0;
    for ( UInt32 k=0; k<n; ++k )
    {
        shifted += nodes[ k ]->GetWorldPosition() == worlds[ k ] - offset ? 1 : 0;
        unchanged += SameMatrixBits( nodes[ k ]->GetTransformToPrecise( *nodes[ partners[ k ] ] ), relatives[ k ] ) ? 1 : 0;
        HashMatrix( raw, nodes[ k ]->GetWorldMatrix() );
    }
    hash = raw.GetHash();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpaceConversions: TFixed32_32 tree of " << n << " nodes: worst relative difference to Float64 " << worst << ", after the rebase "
               << shifted << " world positions moved by exactly -offset and " << unchanged << " precise relative transforms unchanged bit for bit; "
               << raw.GetCount() << " raw values, hash " << CORE::ToString( hash ) );
    for ( UInt32 k=n; k>0; --k )
        delete nodes[ k - 1 ];
    return 0 == bad && n == shifted && n == unchanged;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeoSpaceConversionsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeoSpaceConversions" );

    GUCEF_TESTFW_TESTCASE( "GeoSpaceConversions: ToLocal( ToWorld( p ) ) = p for 10^4 seeded points and hierarchies (1e-4 relative, Float32)" )
    try
    {
        ASSERT_TRUE( RoundTrips< Float32 >( 7101, 10000, 1.0e-4 ) );
        ASSERT_TRUE( RoundTrips< Float64 >( 7102, 10000, 1.0e-12 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpaceConversions: ToSpace between cousins equals the route through world; ToSpacePrecise and GetTransformToPrecise agree with the Float64 products" )
    try
    {
        ASSERT_TRUE( CousinConversions< Float32 >( 7201, ScaledCount( 2000 ), 1.0e-4 ) );
        ASSERT_TRUE( CousinConversions< Float64 >( 7202, ScaledCount( 2000 ), 1.0e-12 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpaceConversions: ToSpacePrecise for Float32 siblings 10^6 from the origin errs at most 1e-3 (the route through world is reported)" )
    try
    {
        ASSERT_TRUE( FarSiblings( 7301, ScaledCount( 1000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpaceConversions: normals transform with the inverse transpose under non-uniform scale" )
    try
    {
        ASSERT_TRUE( NormalsUnderNonUniformScale< Float32 >( 7401, ScaledCount( 2000 ), 1.0e-5 ) );
        ASSERT_TRUE( NormalsUnderNonUniformScale< Float64 >( 7402, ScaledCount( 2000 ), 1.0e-12 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpaceConversions: RebaseOrigin changes only the root's children, shifts world positions by -offset, keeps relative transforms bit for bit (Float32)" )
    try
    {
        ASSERT_TRUE( RebaseFloat32( 7501, ScaledCount( 500 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpaceConversions: CGeoSpace64 keeps 0.01 resolution at 10^9; Float32 snapshots relative to an origin are exact to Float32 precision" )
    try
    {
        ASSERT_TRUE( Float64Resolution() );
        ASSERT_TRUE( RelativeSnapshots( 7601, ScaledCount( 2000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpaceConversions: TFixed32_32 tree (A-1): agrees with Float64, rebases bit-exactly and matches the pinned hash" )
    try
    {
        UInt64 hash = 0;
        ASSERT_TRUE( FixedPointTreeRun( hash ) );
        ASSERT_TRUE( PinnedFixedGeoSpaceHash == hash );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
