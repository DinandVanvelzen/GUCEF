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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H
#include "gucefGEOMETRY_CTDynamicAabbTree.h"
#define GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H
#endif /* GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H ? */

#include "TestQueryReference.h"
#include "TestConvexReference.h"
#include "TestDynamicAabbTree.h"

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
using ConvexReference::ScaledCount;
using ConvexReference::CRawHash;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The pinned FNV-1a hash of the TFixed32_32 run (tree heights, root boxes, query and ray results and the final fat
 *  boxes, as raw values): identical in every build (gcc, clang, ASan, GUCEF_MATH_INLINE_ALL)
 */
static const UInt64 PinnedFixedTreeHash = 2773269860564694738ULL;

/*-------------------------------------------------------------------------*/

/** Values of a kind from multiples of 1/64 (exact in every kind) */
template < typename T >
struct SKind
{
    static T FromSixtyFourths( Int32 k ) { return (T) k / (T) 64; }
    static void Hash( CRawHash& hash, const T& value ) { hash.AddRaw( (Int64) ( (Float64) value * 4096.0 ) ); }
};

template <>
struct SKind< TFixed32_32 >
{
    static TFixed32_32 FromSixtyFourths( Int32 k ) { return TFixed32_32::FromRaw( (Int64) k * ( (Int64) 1 << 26 ) ); }
    static void Hash( CRawHash& hash, const TFixed32_32& value ) { hash.Add( value ); }
};

/*-------------------------------------------------------------------------*/

/** Collects the visited proxies; optionally stops after 'stopAfter' visits, or clips the ray to 'clipTo' at the first visit */
template < typename T >
struct SProxyCollector
{
    GUCEF::vector< Int32 > ids;
    UInt32 stopAfter;
    bool clip;
    T clipTo;

    SProxyCollector( void ) : ids(), stopAfter( 0xFFFFFFFF ), clip( false ), clipTo() {}

    bool Visit( Int32 proxyId )
    {
        ids.push_back( proxyId );
        return (UInt32) ids.size() < stopAfter;
    }

    bool Visit( Int32 proxyId, T& maxDistance )
    {
        ids.push_back( proxyId );
        if ( clip && clipTo < maxDistance )
            maxDistance = clipTo;
        return (UInt32) ids.size() < stopAfter;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The reference of the ray casts, independent of the tree (no ClampRayLength, no RayOverlapsBox): the Float64 slab
 *  test of the segment from the ray origin over [ 0, length ] along the ray direction (both as Float64) against the
 *  box grown by 'grow' on every side (shrunk for a negative 'grow'). Touching counts.
 */
template < typename T >
static bool
ReferenceSegmentTouchesBox( const CTRay3D< T >& ray, Float64 length, const CTAxisAlignedBox3D< T >& box, Float64 grow )
{
    typedef CTNumericTraits< T > TTraits;
    if ( box.IsEmpty() )
        return false;
    Float64 tMin = 0.0;
    Float64 tMax = length;
    const CTVector3D< T > d = ray.direction.AsVector();
    for ( UInt32 a=0; a<3; ++a )
    {
        const Float64 o = TTraits::ToFloat64( ray.origin.GetComponent( a ) );
        const Float64 dir = TTraits::ToFloat64( d.GetComponent( a ) );
        const Float64 low = TTraits::ToFloat64( box.min.GetComponent( a ) ) - grow;
        const Float64 high = TTraits::ToFloat64( box.max.GetComponent( a ) ) + grow;
        if ( high < low )
            return false;
        if ( 0.0 == dir )
        {
            if ( o < low || high < o )
                return false;
            continue;
        }
        Float64 t1 = ( low - o ) / dir;
        Float64 t2 = ( high - o ) / dir;
        if ( t2 < t1 )
            std::swap( t1, t2 );
        if ( tMin < t1 )
            tMin = t1;
        if ( t2 < tMax )
            tMax = t2;
        if ( tMax < tMin )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Compares the proxies a ray cast visited with the reference over every proxy in 'proxies': a proxy the reference
 *  decides differently counts only when it is not a boundary case (the reference with the boxes grown by 'tolerance'
 *  and shrunk by it agree). Returns the number of real differences.
 */
template < typename T >
static UInt32
RayCastDifferences( const CTDynamicAabbTree< T, UInt32 >& tree, const GUCEF::vector< Int32 >& proxies, const CTRay3D< T >& ray, const T& maxDistance, GUCEF::vector< Int32 >& visited, Float64 tolerance )
{
    typedef CTNumericTraits< T > TTraits;
    std::sort( visited.begin(), visited.end() );
    const Float64 length = TTraits::ToFloat64( maxDistance );
    UInt32 differences = 0;
    for ( size_t i=0; i<proxies.size(); ++i )
    {
        const CTAxisAlignedBox3D< T >& fat = tree.GetFatBounds( proxies[ i ] );
        const bool found = std::binary_search( visited.begin(), visited.end(), proxies[ i ] );
        if ( found == ReferenceSegmentTouchesBox( ray, length, fat, 0.0 ) )
            continue;
        const bool grown = ReferenceSegmentTouchesBox( ray, length, fat, tolerance );
        const bool shrunk = ReferenceSegmentTouchesBox( ray, length, fat, -tolerance );
        if ( grown == shrunk )
            ++differences;
    }
    /* every visit names a distinct proxy */
    for ( size_t i=1; i<visited.size(); ++i )
        if ( visited[ i - 1 ] == visited[ i ] )
            ++differences;
    return differences;
}

/*-------------------------------------------------------------------------*/

/** The boundary tolerance of the ray reference per kind (Float32 rounds the segment and the cross products coarsely) */
template < typename T > struct SRayTolerance { static Float64 Get( void ) { return 1e-6; } };
template <> struct SRayTolerance< Float32 > { static Float64 Get( void ) { return 1e-3; } };

/*-------------------------------------------------------------------------*/

/** A moving item of the brute-force twin */
template < typename T >
struct SMover
{
    Int32 proxy;
    CTPoint3D< T > centre;
    CTVector3D< T > half;
    CTVector3D< T > velocity;
    UInt32 tag;

    CTAxisAlignedBox3D< T > GetBox( void ) const { return CTAxisAlignedBox3D< T >( centre - half, centre + half ); }
};

/*-------------------------------------------------------------------------*/

template < typename T >
static SMover< T >
RandomMover( CRandom& random, Int32 world, UInt32 tag )
{
    typedef SKind< T > K;
    SMover< T > m;
    m.proxy = -1;
    m.tag = tag;
    const Int32 cx = random.Index( 2 * world ) - world;
    const Int32 cy = random.Index( 2 * world ) - world;
    const Int32 cz = random.Index( 2 * world ) - world;
    m.centre = CTPoint3D< T >( K::FromSixtyFourths( cx ), K::FromSixtyFourths( cy ), K::FromSixtyFourths( cz ) );
    const Int32 size = random.Chance( 0.9 ) ? 8 : 64;
    const Int32 hx = 1 + random.Index( size );
    const Int32 hy = 1 + random.Index( size );
    const Int32 hz = 1 + random.Index( size );
    m.half = CTVector3D< T >( K::FromSixtyFourths( hx ), K::FromSixtyFourths( hy ), K::FromSixtyFourths( hz ) );
    const Int32 vx = random.Index( 9 ) - 4;
    const Int32 vy = random.Index( 9 ) - 4;
    const Int32 vz = random.Index( 9 ) - 4;
    m.velocity = CTVector3D< T >( K::FromSixtyFourths( vx ), K::FromSixtyFourths( vy ), K::FromSixtyFourths( vz ) );
    return m;
}

/*-------------------------------------------------------------------------*/

/** A random unit direction of the kind from integer draws */
template < typename T >
static CTDirectionVector3D< T >
RandomTreeDirection( CRandom& random )
{
    for ( ;; )
    {
        const Int32 x = random.Index( 129 ) - 64;
        const Int32 y = random.Index( 129 ) - 64;
        const Int32 z = random.Index( 129 ) - 64;
        const CTVector3D< T > v( SKind< T >::FromSixtyFourths( x ), SKind< T >::FromSixtyFourths( y ), SKind< T >::FromSixtyFourths( z ) );
        CTDirectionVector3D< T > d;
        if ( x * x + y * y + z * z > 64 && CTDirectionVector3D< T >::TryCreate( v, d ) )
            return d;
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
HeightBound( UInt32 count )
{
    /* 2 log2( n ) + 4 */
    UInt32 log = 0;
    while ( ( (UInt64) 1 << ( log + 1 ) ) <= (UInt64) count )
        ++log;
    return 2 * ( log + 1 ) + 4;
}

/*-------------------------------------------------------------------------*/

/**
 *  'count' seeded proxies moving over 'steps' steps (a few destroyed and created again each step): after every step
 *  Validate passes, the height is within 2 log2( n ) + 4, every fat box holds its proxy's box, and box queries and ray
 *  casts equal the scan over the fat boxes. The hash (when given) records the run as raw values.
 */
template < typename T >
static bool
MovingProxies( const char* name, UInt64 seed, UInt32 count, UInt32 steps, UInt32 queriesPerStep, CRawHash* hash )
{
    typedef CTDynamicAabbTree< T, UInt32 > TTree;
    typedef SKind< T > K;
    const Int32 world = 64 * 64;        /* coordinates within +-64 units */
    CRandom random( seed );
    TTree tree;
    tree.Reserve( count );
    GUCEF::vector< SMover< T > > movers;
    for ( UInt32 i=0; i<count; ++i )
    {
        SMover< T > m = RandomMover< T >( random, world, i );
        m.proxy = tree.CreateProxy( m.GetBox(), i );
        movers.push_back( m );
    }

    UInt32 bad = 0;
    UInt32 reinserted = 0;
    GUCEF::vector< Int32 > expected;
    const T limit = K::FromSixtyFourths( world );
    for ( UInt32 step=0; step<steps && bad<10; ++step )
    {
        /* move every proxy, bounce off the walls */
        for ( size_t i=0; i<movers.size(); ++i )
        {
            SMover< T >& m = movers[ i ];
            m.centre = m.centre + m.velocity;
            for ( UInt32 a=0; a<3; ++a )
            {
                const T c = m.centre.GetComponent( a );
                if ( limit < c || c < -limit )
                    m.velocity.SetComponent( a, -m.velocity.GetComponent( a ) );
            }
            if ( tree.MoveProxy( m.proxy, m.GetBox(), m.velocity ) )
                ++reinserted;
        }
        /* churn: a few proxies destroyed and created again (their ids may be reused) */
        for ( UInt32 c=0; c<3; ++c )
        {
            const Int32 victim = random.Index( (Int32) movers.size() );
            tree.DestroyProxy( movers[ victim ].proxy );
            if ( tree.IsProxy( movers[ victim ].proxy ) )
                ++bad;
            SMover< T > fresh = RandomMover< T >( random, world, movers[ victim ].tag );
            fresh.proxy = tree.CreateProxy( fresh.GetBox(), fresh.tag );
            movers[ victim ] = fresh;
        }

        if ( !tree.Validate() || tree.GetProxyCount() != count || tree.GetHeight() > HeightBound( count ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": step " << step << " (seed " << seed << ") valid " << tree.Validate() << " height " << tree.GetHeight() << " bound " << HeightBound( count ) );
            ++bad;
        }
        for ( size_t i=0; i<movers.size(); ++i )
        {
            if ( !tree.GetFatBounds( movers[ i ].proxy ).Contains( movers[ i ].GetBox() ) || tree.GetUserData( movers[ i ].proxy ) != movers[ i ].tag )
            {
                ++bad;
                break;
            }
        }

        for ( UInt32 q=0; q<queriesPerStep; ++q )
        {
            /* a box query */
            const SMover< T > probe = RandomMover< T >( random, world, 0 );
            CTAxisAlignedBox3D< T > box = probe.GetBox().Expanded( K::FromSixtyFourths( random.Index( 256 ) ) );
            SProxyCollector< T > collector;
            tree.Query( box, collector );
            expected.clear();
            for ( size_t i=0; i<movers.size(); ++i )
                if ( tree.GetFatBounds( movers[ i ].proxy ).Intersects( box ) )
                    expected.push_back( movers[ i ].proxy );
            std::sort( collector.ids.begin(), collector.ids.end() );
            std::sort( expected.begin(), expected.end() );
            if ( collector.ids != expected )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": query " << q << " of step " << step << " (seed " << seed << ") found " << collector.ids.size() << ", the scan " << expected.size() );
                ++bad;
            }
            if ( GUCEF_NULL != hash )
            {
                hash->Add( (UInt32) collector.ids.size() );
                for ( size_t i=0; i<collector.ids.size(); ++i )
                    hash->Add( (UInt32) collector.ids[ i ] );
            }

            /* a ray cast */
            const CTRay3D< T > ray( probe.centre, RandomTreeDirection< T >( random ) );
            const bool unlimited = random.Chance( 0.2 );
            const T maxDistance = unlimited ? K::FromSixtyFourths( 1 << 20 ) : K::FromSixtyFourths( random.Index( 64 * 64 ) );
            SProxyCollector< T > rays;
            tree.RayCast( ray, maxDistance, rays );
            expected.clear();
            for ( size_t i=0; i<movers.size(); ++i )
                expected.push_back( movers[ i ].proxy );
            const UInt32 differences = RayCastDifferences( tree, expected, ray, maxDistance, rays.ids, SRayTolerance< T >::Get() );
            if ( 0 != differences )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": ray " << q << " of step " << step << " (seed " << seed << ") found " << rays.ids.size() << ", " << differences << " differ from the reference" );
                ++bad;
            }
            if ( GUCEF_NULL != hash )
            {
                hash->Add( (UInt32) rays.ids.size() );
                for ( size_t i=0; i<rays.ids.size(); ++i )
                    hash->Add( (UInt32) rays.ids[ i ] );
            }
        }
        if ( GUCEF_NULL != hash )
        {
            hash->Add( tree.GetHeight() );
            const CTAxisAlignedBox3D< T > root = tree.GetRootBounds();
            for ( UInt32 a=0; a<3; ++a )
            {
                K::Hash( *hash, root.min.GetComponent( a ) );
                K::Hash( *hash, root.max.GetComponent( a ) );
            }
        }
    }
    if ( GUCEF_NULL != hash )
    {
        hash->Add( reinserted );
        for ( size_t i=0; i<movers.size(); ++i )
        {
            hash->Add( (UInt32) movers[ i ].proxy );
            const CTAxisAlignedBox3D< T >& fat = tree.GetFatBounds( movers[ i ].proxy );
            for ( UInt32 a=0; a<3; ++a )
            {
                K::Hash( *hash, fat.min.GetComponent( a ) );
                K::Hash( *hash, fat.max.GetComponent( a ) );
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << count << " proxies over " << steps << " steps, " << reinserted << " reinsertions, final height " << tree.GetHeight() << " (bound " << HeightBound( count ) << ")" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** The members one by one: ids, user data, moves inside and outside the fat box, prediction, margins, clipping, stops, shifts */
template < typename T >
static bool
TreeMembers( const char* name )
{
    typedef CTDynamicAabbTree< T, UInt32 > TTree;
    typedef SKind< T > K;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTPoint3D< T > TPoint;
    typedef CTVector3D< T > TVector;
    bool ok = true;
    const T one = K::FromSixtyFourths( 64 );
    const T zero = K::FromSixtyFourths( 0 );

    TTree tree( one / K::FromSixtyFourths( 4 * 64 ), K::FromSixtyFourths( 128 ) );        /* margin 1/4, multiplier 2 */
    if ( !( tree.GetMargin() == K::FromSixtyFourths( 16 ) ) || !( tree.GetDisplacementMultiplier() == K::FromSixtyFourths( 128 ) ) )
        ok = false;
    if ( 0 != tree.GetHeight() || !tree.GetRootBounds().IsEmpty() || !tree.Validate() )
        ok = false;
    SProxyCollector< T > none;
    tree.Query( TBox( TPoint( -one, -one, -one ), TPoint( one, one, one ) ), none );
    tree.RayCast( CTRay3D< T >( TPoint(), CTDirectionVector3D< T >::UnitX() ), one, none );
    if ( !none.ids.empty() )
        ok = false;

    const TBox unit( TPoint( zero, zero, zero ), TPoint( one, one, one ) );
    const Int32 a = tree.CreateProxy( unit, 7 );
    const Int32 b = tree.CreateProxy( TBox( TPoint( one * K::FromSixtyFourths( 5 * 64 ), zero, zero ), TPoint( one * K::FromSixtyFourths( 6 * 64 ), one, one ) ), 8 );
    if ( !tree.IsProxy( a ) || !tree.IsProxy( b ) || tree.IsProxy( -1 ) || tree.IsProxy( 1000 ) || 2 != tree.GetProxyCount() || 7 != tree.GetUserData( a ) || 1 != tree.GetHeight() )
        ok = false;
    /* the stored box is the given one grown by the margin */
    const T quarter = K::FromSixtyFourths( 16 );
    if ( !( tree.GetFatBounds( a ) == unit.Expanded( quarter ) ) )
        ok = false;
    tree.SetUserData( a, 9 );
    if ( 9 != tree.GetUserData( a ) )
        ok = false;

    /* a small move stays inside the fat box: no reinsertion; a large one reinserts, stretched along the displacement */
    const TVector small( K::FromSixtyFourths( 4 ), zero, zero );
    if ( tree.MoveProxy( a, TBox( unit.min + small, unit.max + small ), small ) )
        ok = false;
    const TVector big( one * K::FromSixtyFourths( 2 * 64 ), zero, zero );
    if ( !tree.MoveProxy( a, TBox( unit.min + big, unit.max + big ), big ) )
        ok = false;
    const TBox fat = tree.GetFatBounds( a );
    /* x grows by margin plus 2 x displacement on the positive side only */
    if ( !( fat.max.x == unit.max.x + big.x + quarter + big.x * K::FromSixtyFourths( 128 ) ) || !( fat.min.x == unit.min.x + big.x - quarter ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the predicted fat box is " << fat.ToString() );
        ok = false;
    }
    /* a shrink far below the fat box reinserts (the box would be too large) */
    if ( !tree.MoveProxy( a, TBox( unit.min + big, unit.min + big ), TVector() ) )
        ok = false;
    if ( !tree.Validate() )
        ok = false;

    /* clipping: after the first visit the segment is shorter, later visits satisfy the clipped segment */
    for ( Int32 i=0; i<40; ++i )
    {
        const TPoint at( one * K::FromSixtyFourths( 64 * ( i % 10 ) ), one * K::FromSixtyFourths( 64 * ( i / 10 ) ), zero );
        tree.CreateProxy( TBox( at, at + TVector( quarter, quarter, quarter ) ), (UInt32) i );
    }
    const CTRay3D< T > ray( TPoint( -one, K::FromSixtyFourths( 8 ), K::FromSixtyFourths( 8 ) ), CTDirectionVector3D< T >::UnitX() );
    SProxyCollector< T > clipped;
    clipped.clip = true;
    clipped.clipTo = one * K::FromSixtyFourths( 4 * 64 );
    tree.RayCast( ray, one * K::FromSixtyFourths( 100 * 64 ), clipped );
    for ( size_t i=1; i<clipped.ids.size(); ++i )
        if ( !TTree::RayOverlapsBox( ray, clipped.clipTo, tree.GetFatBounds( clipped.ids[ i ] ) ) )
            ok = false;
    SProxyCollector< T > stopped;
    stopped.stopAfter = 1;
    tree.RayCast( ray, one * K::FromSixtyFourths( 100 * 64 ), stopped );
    tree.Query( TBox( TPoint( -one * K::FromSixtyFourths( 100 * 64 ), -one * K::FromSixtyFourths( 100 * 64 ), -one * K::FromSixtyFourths( 100 * 64 ) ),
                      TPoint( one * K::FromSixtyFourths( 100 * 64 ), one * K::FromSixtyFourths( 100 * 64 ), one * K::FromSixtyFourths( 100 * 64 ) ) ), stopped );
    if ( 2 != stopped.ids.size() )
        ok = false;
    /* a negative maximum distance visits nothing */
    SProxyCollector< T > negative;
    tree.RayCast( ray, -one, negative );
    if ( !negative.ids.empty() )
        ok = false;

    /* origin shift: every box moves by -offset, queries in the shifted frame find the same proxies */
    const TVector offset( one * K::FromSixtyFourths( 3 * 64 ), -one, quarter );
    const TBox probe( TPoint( zero, zero, zero ), TPoint( one * K::FromSixtyFourths( 4 * 64 ), one * K::FromSixtyFourths( 2 * 64 ), one ) );
    SProxyCollector< T > before;
    tree.Query( probe, before );
    const TBox fatBefore = tree.GetFatBounds( b );
    tree.ShiftOrigin( offset );
    SProxyCollector< T > after;
    tree.Query( TBox( probe.min - offset, probe.max - offset ), after );
    std::sort( before.ids.begin(), before.ids.end() );
    std::sort( after.ids.begin(), after.ids.end() );
    if ( before.ids != after.ids || !( tree.GetFatBounds( b ).min == fatBefore.min - offset ) || !tree.Validate() )
        ok = false;

    /* destroy: ids are reused from the free list; Clear empties */
    tree.DestroyProxy( b );
    tree.DestroyProxy( TTree::NullProxy );
    const Int32 reused = tree.CreateProxy( unit, 1 );
    if ( !tree.IsProxy( reused ) || !tree.Validate() || 42 != tree.GetProxyCount() )
        ok = false;
    tree.SetMargin( zero );
    const Int32 tight = tree.CreateProxy( unit, 2 );
    if ( !( tree.GetFatBounds( tight ) == unit ) )
        ok = false;
    tree.SetDisplacementMultiplier( zero );
    tree.Clear();
    if ( 0 != tree.GetProxyCount() || 0 != tree.GetNodeCapacity() || !tree.Validate() || tree.IsProxy( reused ) )
        ok = false;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": tree members failed" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  The edge of the documented range (fat boxes inside +-2^29): two proxies at opposite corners of the range and a ray
 *  along the diagonal from the first (both visited; the length bound of the ray must not wrap in fixed point), then
 *  'count' unit boxes spread uniformly over the range: Validate, the height within 2 log2( n ) + 4 (the insertion cost
 *  must not wrap), box queries equal to the scan and ray casts equal to the independent Float64 reference.
 */
template < typename T >
static bool
RangeEdge( const char* name, UInt64 seed, UInt32 count, UInt32 queries )
{
    typedef CTDynamicAabbTree< T, UInt32 > TTree;
    typedef CTNumericTraits< T > TTraits;
    typedef CTPoint3D< T > TPoint;
    typedef CTAxisAlignedBox3D< T > TBox;
    bool ok = true;
    const Int32 edge = 1 << 29;
    const T zero = TTraits::Zero();
    const T one = TTraits::One();
    const T maxDistance = TTraits::FromInt32( 2000000000 );

    /* the corners: margin 0 so the fat boxes reach the range edge; the three axis terms of the length bound sum past 2^31 */
    {
        TTree tree( zero );
        const T low = TTraits::FromInt32( -edge + 1 );
        const T high = TTraits::FromInt32( edge - 1 );
        const Int32 a = tree.CreateProxy( TBox( TPoint( low, low, low ), TPoint( low + one, low + one, low + one ) ), 1 );
        const Int32 b = tree.CreateProxy( TBox( TPoint( high - one, high - one, high - one ), TPoint( high, high, high ) ), 2 );
        CTDirectionVector3D< T > diagonal;
        if ( !CTDirectionVector3D< T >::TryCreate( CTVector3D< T >( one, one, one ), diagonal ) )
            ok = false;
        const CTRay3D< T > ray( TPoint( low, low, low ), diagonal );
        SProxyCollector< T > visited;
        tree.RayCast( ray, maxDistance, visited );
        std::sort( visited.ids.begin(), visited.ids.end() );
        if ( 2 != visited.ids.size() || !std::binary_search( visited.ids.begin(), visited.ids.end(), a ) || !std::binary_search( visited.ids.begin(), visited.ids.end(), b ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the diagonal ray over the range visited " << visited.ids.size() << " of the 2 corner proxies" );
            ok = false;
        }
        /* the length bound never exceeds maxDistance and is never negative */
        const T length = tree.ClampRayLength( ray, maxDistance );
        if ( TTraits::IsNegative( length ) || maxDistance < length )
            ok = false;
        if ( !TTree::RayOverlapsBox( ray, maxDistance, tree.GetFatBounds( b ) ) )
            ok = false;
    }

    /* unit boxes uniform over the range */
    CRandom random( seed );
    TTree tree;
    tree.Reserve( count );
    GUCEF::vector< Int32 > proxies;
    const Int32 spread = edge - 4;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Int32 x = random.Index( 2 * spread ) - spread;
        const Int32 y = random.Index( 2 * spread ) - spread;
        const Int32 z = random.Index( 2 * spread ) - spread;
        const TPoint at( TTraits::FromInt32( x ), TTraits::FromInt32( y ), TTraits::FromInt32( z ) );
        proxies.push_back( tree.CreateProxy( TBox( at, TPoint( at.x + one, at.y + one, at.z + one ) ), i ) );
    }
    if ( !tree.Validate() || tree.GetHeight() > HeightBound( count ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << count << " proxies over +-2^29: valid " << tree.Validate() << " height " << tree.GetHeight() << " bound " << HeightBound( count ) );
        ok = false;
    }

    UInt32 bad = 0;
    GUCEF::vector< Int32 > expected;
    for ( UInt32 q=0; q<queries; ++q )
    {
        /* a box query around a proxy, a few units up to a large part of the range */
        const Int32 around = random.Index( (Int32) proxies.size() );
        const Int32 growBits = 2 + random.Index( 27 );
        const Int32 grow = 1 + random.Index( 1 << growBits );
        const TBox box = tree.GetFatBounds( proxies[ around ] ).Expanded( TTraits::FromInt32( grow ) );
        SProxyCollector< T > found;
        tree.Query( box, found );
        expected.clear();
        for ( size_t i=0; i<proxies.size(); ++i )
            if ( tree.GetFatBounds( proxies[ i ] ).Intersects( box ) )
                expected.push_back( proxies[ i ] );
        std::sort( found.ids.begin(), found.ids.end() );
        std::sort( expected.begin(), expected.end() );
        if ( found.ids != expected )
            ++bad;

        /* a ray from the centre of a proxy towards the centre of another, long enough to cross the range */
        const Int32 from = random.Index( (Int32) proxies.size() );
        const Int32 to = random.Index( (Int32) proxies.size() );
        const TPoint origin = tree.GetFatBounds( proxies[ from ] ).GetCentre();
        const TPoint target = tree.GetFatBounds( proxies[ to ] ).GetCentre();
        CTDirectionVector3D< T > direction;
        if ( !CTDirectionVector3D< T >::TryCreate( target - origin, direction ) )
            continue;
        const CTRay3D< T > ray( origin, direction );
        SProxyCollector< T > rays;
        tree.RayCast( ray, maxDistance, rays );
        /* the target is on the ray: it must be visited */
        if ( rays.ids.end() == std::find( rays.ids.begin(), rays.ids.end(), proxies[ to ] ) )
            ++bad;
        if ( 0 != RayCastDifferences( tree, proxies, ray, maxDistance, rays.ids, 1e-5 ) )
            ++bad;
    }
    if ( 0 != bad )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << bad << " differences from the references over " << queries << " box queries and ray casts over +-2^29" );
        ok = false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << count << " unit proxies over +-2^29, height " << tree.GetHeight() << " (bound " << HeightBound( count ) << ")" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** RayOverlapsBox against the Float64 slab test on random boxes and segments away from the boundary cases */
static bool
RayBoxPredicate( UInt64 seed, UInt32 count )
{
    typedef CTDynamicAabbTree< Float64, UInt32 > TTree;
    CRandom random( seed );
    UInt32 bad = 0;
    for ( UInt32 n=0; n<count; ++n )
    {
        const P3 lo = random.Point( 5.0 );
        const V3 size = V3( random.Uniform( 0.0, 3.0 ), random.Uniform( 0.0, 3.0 ), random.Uniform( 0.0, 3.0 ) );
        const CTAxisAlignedBox3D< Float64 > box( lo, lo + size );
        const CTRay3D< Float64 > ray( random.Point( 8.0 ), random.Direction() );
        const Float64 length = random.Uniform( 0.0, 20.0 );
        /* the reference: the slab test of the box against the segment, decided away from the boundary */
        SRayCastResult< Float64 > hit;
        const bool slab = box.RayCast( ray, length, hit );
        const bool predicate = TTree::RayOverlapsBox( ray, length, box );
        if ( slab != predicate )
        {
            /* accept only boundary cases: the slab distance within 1e-9 of the length */
            if ( !( box.RayCast( ray, length + 1e-9, hit ) && !box.RayCast( ray, length - 1e-9, hit ) ) )
                ++bad;
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDynamicAabbTreeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DynamicAabbTree" );

    GUCEF_TESTFW_TESTCASE( "DynamicAabbTree: 10^4 seeded proxies over 10^3 steps: Query and RayCast equal the scan, height within 2 log2( n ) + 4, Validate every step" )
    try
    {
        ASSERT_TRUE( MovingProxies< Float64 >( "moving Float64", 30701, ScaledCount( 10000 ), ScaledCount( 1000 ), 4, GUCEF_NULL ) );
        ASSERT_TRUE( MovingProxies< Float32 >( "moving Float32", 30702, ScaledCount( 10000 ), ScaledCount( 300 ), 4, GUCEF_NULL ) );
        ASSERT_TRUE( MovingProxies< TFixed32_32 >( "moving TFixed32_32", 30703, ScaledCount( 10000 ), ScaledCount( 300 ), 4, GUCEF_NULL ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicAabbTree: TFixed32_32 run (2000 proxies, 200 steps) equals the scan and matches the pinned hash" )
    try
    {
        CRawHash hash;
        ASSERT_TRUE( MovingProxies< TFixed32_32 >( "pinned TFixed32_32", 30704, 2000, 200, 3, &hash ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DynamicAabbTree: TFixed32_32 run: " << hash.GetCount() << " raw values, hash " << CORE::ToString( hash.GetHash() ) );
        ASSERT_TRUE( PinnedFixedTreeHash == hash.GetHash() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicAabbTree: members (ids, user data, fat boxes and prediction, clipping and stops, ShiftOrigin, reuse, Clear) for every kind" )
    try
    {
        ASSERT_TRUE( TreeMembers< Float64 >( "members Float64" ) );
        ASSERT_TRUE( TreeMembers< Float32 >( "members Float32" ) );
        ASSERT_TRUE( TreeMembers< TFixed32_32 >( "members TFixed32_32" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicAabbTree: the edge of the documented range (+-2^29): corner-to-corner ray visits both, 20000 unit proxies keep the height bound, queries and rays equal the references" )
    try
    {
        ASSERT_TRUE( RangeEdge< Float64 >( "range edge Float64", 30706, 20000, ScaledCount( 200 ) ) );
        ASSERT_TRUE( RangeEdge< TFixed32_32 >( "range edge TFixed32_32", 30706, 20000, ScaledCount( 200 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicAabbTree: RayOverlapsBox equals the slab test of the box except within 1e-9 of the segment end" )
    try
    {
        ASSERT_TRUE( RayBoxPredicate( 30705, ScaledCount( 100000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
