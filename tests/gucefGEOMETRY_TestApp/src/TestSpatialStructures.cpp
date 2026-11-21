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

#ifndef GUCEF_GEOMETRY_CTOCTREE_H
#include "gucefGEOMETRY_CTOctree.h"
#define GUCEF_GEOMETRY_CTOCTREE_H
#endif /* GUCEF_GEOMETRY_CTOCTREE_H ? */

#ifndef GUCEF_GEOMETRY_CTQUADTREE_H
#include "gucefGEOMETRY_CTQuadtree.h"
#define GUCEF_GEOMETRY_CTQUADTREE_H
#endif /* GUCEF_GEOMETRY_CTQUADTREE_H ? */

#ifndef GUCEF_GEOMETRY_CTUNIFORMGRID2D_H
#include "gucefGEOMETRY_CTUniformGrid2D.h"
#define GUCEF_GEOMETRY_CTUNIFORMGRID2D_H
#endif /* GUCEF_GEOMETRY_CTUNIFORMGRID2D_H ? */

#ifndef GUCEF_GEOMETRY_CTUNIFORMGRID3D_H
#include "gucefGEOMETRY_CTUniformGrid3D.h"
#define GUCEF_GEOMETRY_CTUNIFORMGRID3D_H
#endif /* GUCEF_GEOMETRY_CTUNIFORMGRID3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSPATIALHASH_H
#include "gucefGEOMETRY_CTSpatialHash.h"
#define GUCEF_GEOMETRY_CTSPATIALHASH_H
#endif /* GUCEF_GEOMETRY_CTSPATIALHASH_H ? */

#ifndef GUCEF_GEOMETRY_FRUSTUMCULLING_H
#include "gucefGEOMETRY_FrustumCulling.h"
#define GUCEF_GEOMETRY_FRUSTUMCULLING_H
#endif /* GUCEF_GEOMETRY_FRUSTUMCULLING_H ? */


#include "TestSpatialReference.h"
#include "TestConvexReference.h"
#include "TestSpatialStructures.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/* near and far are macros of the Windows headers, undefine them so they can be used as identifiers */
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;
using namespace QueryReference;
using namespace SpatialReference;
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The box and ball types of a dimension */
template < typename T, UInt32 Dim > struct STypes;

template < typename T >
struct STypes< T, 3 >
{
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTSphere< T > TBall;
    static TBox MakeBox( const T* lo, const T* hi ) { return TBox( CTPoint3D< T >( lo[ 0 ], lo[ 1 ], lo[ 2 ] ), CTPoint3D< T >( hi[ 0 ], hi[ 1 ], hi[ 2 ] ) ); }
    static TBall MakeBall( const T* c, const T& r ) { return TBall( CTPoint3D< T >( c[ 0 ], c[ 1 ], c[ 2 ] ), r ); }
};

template < typename T >
struct STypes< T, 2 >
{
    typedef CTRectangle< T > TBox;
    typedef CTCircle< T > TBall;
    static TBox MakeBox( const T* lo, const T* hi ) { return TBox( CTPoint2D< T >( lo[ 0 ], lo[ 1 ] ), CTPoint2D< T >( hi[ 0 ], hi[ 1 ] ) ); }
    static TBall MakeBall( const T* c, const T& r ) { return TBall( CTPoint2D< T >( c[ 0 ], c[ 1 ] ), r ); }
};

/** An item of the brute force twin */
template < typename T, UInt32 Dim >
struct SItem
{
    UInt32 handle;
    T lo[ Dim ];
    T hi[ Dim ];
    bool alive;
};

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Dim >
static void
RandomItemBounds( CRandom& random, T* lo, T* hi, Float64 extent, bool inside )
{
    // sizes: mostly small, some medium, a few large
    const Float64 pick = random.Uniform( 0.0, 1.0 );
    const Float64 size = pick < 0.7 ? random.Uniform( 0.01, 0.5 ) : ( pick < 0.95 ? random.Uniform( 0.5, 3.0 ) : random.Uniform( 3.0, 12.0 ) );
    // a few outside the world
    const Float64 range = inside ? extent : ( random.Chance( 0.04 ) ? extent * 1.3 : extent );
    for ( UInt32 a=0; a<Dim; ++a )
    {
        const T c = (T) random.Uniform( -range, range );
        const T h = (T) ( size * random.Uniform( 0.2, 1.0 ) );
        lo[ a ] = c - h;
        hi[ a ] = c + h;
    }
}

/** The exact predicates of the regions, written again on arrays */
template < typename T, UInt32 Dim >
static bool
BoxTouches( const T* lo, const T* hi, const T* qlo, const T* qhi )
{
    for ( UInt32 a=0; a<Dim; ++a )
        if ( hi[ a ] < qlo[ a ] || qhi[ a ] < lo[ a ] )
            return false;
    return true;
}

template < typename T, UInt32 Dim >
static bool
BallReaches( const T* lo, const T* hi, const T* centre, const T& radius )
{
    T sum = (T) 0;
    for ( UInt32 a=0; a<Dim; ++a )
    {
        const T below = lo[ a ] - centre[ a ];
        const T above = centre[ a ] - hi[ a ];
        const T excess = below > (T) 0 ? below : ( above > (T) 0 ? above : (T) 0 );
        sum = sum + excess * excess;
    }
    return sum <= radius * radius;
}

/*-------------------------------------------------------------------------*/

template < UInt32 Dim >
struct SFrustumPart
{
    template < typename T, typename TStructure >
    static bool Run( CRandom&, TStructure&, const GUCEF::vector< SItem< T, Dim > >&, UInt32, const char*, UInt64 ) { return true; }
};

template <>
struct SFrustumPart< 3 >
{
    template < typename T, typename TStructure >
    static bool Run( CRandom& random, TStructure& structure, const GUCEF::vector< SItem< T, 3 > >& items, UInt32 queries, const char* name, UInt64 seed )
    {
        UInt32 nonEmpty = 0;
        for ( UInt32 q=0; q<queries; ++q )
        {
            CTFrustum< T > frustum;
            RandomShape( random, frustum );
            CCollector found;
            structure.Query( frustum, found );
            GUCEF::vector< UInt32 > expected;
            for ( size_t i=0; i<items.size(); ++i )
            {
                if ( !items[ i ].alive )
                    continue;
                const CTAxisAlignedBox3D< T > box( CTPoint3D< T >( items[ i ].lo[ 0 ], items[ i ].lo[ 1 ], items[ i ].lo[ 2 ] ), CTPoint3D< T >( items[ i ].hi[ 0 ], items[ i ].hi[ 1 ], items[ i ].hi[ 2 ] ) );
                if ( FRUSTUMCLASSIFICATION_OUTSIDE != frustum.Classify( box ) )
                    expected.push_back( items[ i ].handle );
            }
            if ( !SameSet( found.handles, expected ) || !NoDuplicates( found.handles ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": frustum query returns " << found.handles.size() << " items instead of " << expected.size() << ", seed " << seed << " query " << q );
                return false;
            }
            if ( !expected.empty() )
                ++nonEmpty;
        }
        return nonEmpty > 0;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The harness: insert, remove, move and insert again, then compare box, ball and (3D) frustum queries with the scan of the
 *  live items; 'prepare' sets the structure up (the world and the cells).
 */
template < typename T, UInt32 Dim, typename TStructure >
static bool
StructureSweep( const char* name, UInt64 seed, UInt32 itemCount, UInt32 queryCount, TStructure& structure, Float64 extent )
{
    typedef STypes< T, Dim > TTypes;
    CRandom random( seed );
    GUCEF::vector< SItem< T, Dim > > items;
    GUCEF::vector< UInt32 > handleToItem;
    const T nan = (T) ( 0.0 / ( (Float64) itemCount - (Float64) itemCount ) );

    // refused input
    {
        T lo[ Dim ], hi[ Dim ];
        for ( UInt32 a=0; a<Dim; ++a )
        {
            lo[ a ] = (T) 1;
            hi[ a ] = (T) 0;
        }
        if ( TStructure::InvalidHandle != structure.Insert( 7, TTypes::MakeBox( lo, hi ) ) )
            return false;
        lo[ 0 ] = nan;
        hi[ 0 ] = (T) 1;
        for ( UInt32 a=1; a<Dim; ++a )
        {
            lo[ a ] = (T) 0;
            hi[ a ] = (T) 1;
        }
        if ( TStructure::InvalidHandle != structure.Insert( 7, TTypes::MakeBox( lo, hi ) ) || 0 != structure.GetCount() )
            return false;
    }

    // insertions
    for ( UInt32 i=0; i<itemCount; ++i )
    {
        SItem< T, Dim > item;
        RandomItemBounds< T, Dim >( random, item.lo, item.hi, extent, false );
        item.handle = structure.Insert( i, TTypes::MakeBox( item.lo, item.hi ) );
        item.alive = true;
        if ( TStructure::InvalidHandle == item.handle )
            return false;
        if ( item.handle >= handleToItem.size() )
            handleToItem.resize( item.handle + 1, 0xFFFFFFFFU );
        handleToItem[ item.handle ] = (UInt32) items.size();
        items.push_back( item );
    }
    if ( structure.GetCount() != itemCount || !structure.Validate() )
        return false;

    // removals, moves and insertions again (the freed handles are reused)
    for ( UInt32 round=0; round<3; ++round )
    {
        for ( UInt32 k=0; k<itemCount / 5; ++k )
        {
            const UInt32 index = (UInt32) random.Index( (Int32) items.size() );
            if ( !items[ index ].alive )
                continue;
            if ( !structure.Remove( items[ index ].handle ) || structure.IsValid( items[ index ].handle ) || structure.Remove( items[ index ].handle ) )
                return false;
            items[ index ].alive = false;
        }
        for ( UInt32 k=0; k<itemCount / 3; ++k )
        {
            const UInt32 index = (UInt32) random.Index( (Int32) items.size() );
            if ( !items[ index ].alive )
                continue;
            T lo[ Dim ], hi[ Dim ];
            RandomItemBounds< T, Dim >( random, lo, hi, extent, false );
            if ( !structure.Move( items[ index ].handle, TTypes::MakeBox( lo, hi ) ) )
                return false;
            for ( UInt32 a=0; a<Dim; ++a )
            {
                items[ index ].lo[ a ] = lo[ a ];
                items[ index ].hi[ a ] = hi[ a ];
            }
        }
        for ( UInt32 k=0; k<itemCount / 8; ++k )
        {
            SItem< T, Dim > item;
            RandomItemBounds< T, Dim >( random, item.lo, item.hi, extent, false );
            item.handle = structure.Insert( 1000000 + k, TTypes::MakeBox( item.lo, item.hi ) );
            item.alive = true;
            if ( TStructure::InvalidHandle == item.handle )
                return false;
            if ( item.handle >= handleToItem.size() )
                handleToItem.resize( item.handle + 1, 0xFFFFFFFFU );
            // a reused handle replaces the dead item it came from
            handleToItem[ item.handle ] = (UInt32) items.size();
            items.push_back( item );
        }
        UInt32 alive = 0;
        for ( size_t i=0; i<items.size(); ++i )
            alive += items[ i ].alive ? 1 : 0;
        if ( structure.GetCount() != alive || !structure.Validate() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the structure does not validate after round " << round << ", seed " << seed );
            return false;
        }
    }
    // dead items whose handle was reused must not count: only the newest item of a handle is alive
    for ( size_t i=0; i<items.size(); ++i )
        if ( items[ i ].alive && handleToItem[ items[ i ].handle ] != (UInt32) i )
            items[ i ].alive = false;
    {
        UInt32 alive = 0;
        for ( size_t i=0; i<items.size(); ++i )
            alive += items[ i ].alive ? 1 : 0;
        if ( alive != structure.GetCount() )
            return false;
    }
    // the bounds and the items stored under the handles
    for ( size_t i=0; i<items.size(); ++i )
        if ( items[ i ].alive )
        {
            if ( !structure.IsValid( items[ i ].handle ) )
                return false;
            const typename TTypes::TBox box = structure.GetBounds( items[ i ].handle );
            if ( box.min.x != items[ i ].lo[ 0 ] || box.max.y != items[ i ].hi[ 1 ] )
                return false;
        }

    UInt32 boxResults = 0;
    UInt32 ballResults = 0;
    for ( UInt32 q=0; q<queryCount; ++q )
    {
        T qlo[ Dim ], qhi[ Dim ];
        const Float64 size = random.Chance( 0.1 ) ? random.Uniform( 5.0, 40.0 ) : random.Uniform( 0.1, 4.0 );
        for ( UInt32 a=0; a<Dim; ++a )
        {
            const T c = (T) random.Uniform( -extent * 1.2, extent * 1.2 );
            qlo[ a ] = c - (T) size;
            qhi[ a ] = c + (T) ( size * random.Uniform( 0.1, 1.0 ) );
        }
        CCollector found;
        structure.Query( TTypes::MakeBox( qlo, qhi ), found );
        GUCEF::vector< UInt32 > expected;
        for ( size_t i=0; i<items.size(); ++i )
            if ( items[ i ].alive && BoxTouches< T, Dim >( items[ i ].lo, items[ i ].hi, qlo, qhi ) )
                expected.push_back( items[ i ].handle );
        if ( !SameSet( found.handles, expected ) || !NoDuplicates( found.handles ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": box query returns " << found.handles.size() << " items instead of " << expected.size() << ", seed " << seed << " query " << q );
            return false;
        }
        boxResults += (UInt32) expected.size();

        T centre[ Dim ];
        for ( UInt32 a=0; a<Dim; ++a )
            centre[ a ] = (T) random.Uniform( -extent * 1.2, extent * 1.2 );
        const T radius = (T) ( random.Chance( 0.1 ) ? random.Uniform( 5.0, 30.0 ) : random.Uniform( 0.05, 5.0 ) );
        CCollector ballFound;
        structure.Query( TTypes::MakeBall( centre, radius ), ballFound );
        expected.clear();
        for ( size_t i=0; i<items.size(); ++i )
            if ( items[ i ].alive && BallReaches< T, Dim >( items[ i ].lo, items[ i ].hi, centre, radius ) )
                expected.push_back( items[ i ].handle );
        if ( !SameSet( ballFound.handles, expected ) || !NoDuplicates( ballFound.handles ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": sphere query returns " << ballFound.handles.size() << " items instead of " << expected.size() << ", seed " << seed << " query " << q );
            return false;
        }
        ballResults += (UInt32) expected.size();

        // a visitor that stops after three
        CCollector limited;
        limited.limit = 3;
        structure.Query( TTypes::MakeBox( qlo, qhi ), limited );
        if ( limited.handles.size() > 3 || ( found.handles.size() >= 3 && 3 != limited.handles.size() ) )
            return false;
    }
    if ( !SFrustumPart< Dim >::template Run< T >( random, structure, items, queryCount, name, seed ) )
        return false;

    // everything out again
    for ( size_t i=0; i<items.size(); ++i )
        if ( items[ i ].alive && !structure.Remove( items[ i ].handle ) )
            return false;
    if ( 0 != structure.GetCount() || !structure.Validate() )
        return false;
    CCollector none;
    structure.Query( TTypes::MakeBox( items[ 0 ].lo, items[ 0 ].hi ), none );
    if ( !none.handles.empty() )
        return false;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SpatialStructures: " << name << ": " << queryCount << " box and sphere queries" << ( 3 == Dim ? " and frustum queries" : "" ) << " after insertions, removals and moves equal the scan of " << items.size() << " items (" << boxResults << " box results, " << ballResults << " sphere results)" );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The ray query of a grid by dimension */
template < UInt32 Dim > struct SRayQuery;

template <>
struct SRayQuery< 3 >
{
    template < typename T, typename TStructure >
    static void Run( TStructure& structure, const T* o, const T* d, const T& maxDistance, CCollector& collector )
    {
        structure.QueryRay( CTRay3D< T >( CTPoint3D< T >( o[ 0 ], o[ 1 ], o[ 2 ] ), CTDirectionVector3D< T >( CTVector3D< T >( d[ 0 ], d[ 1 ], d[ 2 ] ) ) ), maxDistance, collector );
    }
};

template <>
struct SRayQuery< 2 >
{
    template < typename T, typename TStructure >
    static void Run( TStructure& structure, const T* o, const T* d, const T& maxDistance, CCollector& collector )
    {
        structure.QueryRay( CTRay2D< T >( CTPoint2D< T >( o[ 0 ], o[ 1 ] ), CTDirectionVector2D< T >( CTVector2D< T >( d[ 0 ], d[ 1 ] ) ) ), maxDistance, collector );
    }
};

/*-------------------------------------------------------------------------*/

/** Rays against the grids: the visited items equal the items whose bounds the ray touches, each once, with the entering distance */
template < typename T, UInt32 Dim, typename TStructure >
static bool
RaySweep( const char* name, UInt64 seed, UInt32 itemCount, UInt32 rayCount, TStructure& structure, Float64 extent )
{
    typedef STypes< T, Dim > TTypes;
    CRandom random( seed );
    GUCEF::vector< SItem< T, Dim > > items;
    for ( UInt32 i=0; i<itemCount; ++i )
    {
        SItem< T, Dim > item;
        RandomItemBounds< T, Dim >( random, item.lo, item.hi, extent * 0.7, true );
        // the ray query of a bounded grid finds the items inside its area: keep them inside
        for ( UInt32 a=0; a<Dim; ++a )
        {
            if ( item.lo[ a ] < (T) -extent ) item.lo[ a ] = (T) -extent;
            if ( item.hi[ a ] > (T) extent ) item.hi[ a ] = (T) extent;
            if ( item.hi[ a ] < item.lo[ a ] ) item.hi[ a ] = item.lo[ a ];
        }
        item.handle = structure.Insert( i, TTypes::MakeBox( item.lo, item.hi ) );
        item.alive = true;
        if ( TStructure::InvalidHandle == item.handle )
            return false;
        items.push_back( item );
    }
    UInt32 totalHits = 0;
    for ( UInt32 n=0; n<rayCount; ++n )
    {
        T o[ Dim ], d[ Dim ];
        Float64 length = 0.0;
        Float64 dir[ 3 ];
        for ( UInt32 a=0; a<Dim; ++a )
        {
            o[ a ] = (T) random.Uniform( -extent * 1.1, extent * 1.1 );
            dir[ a ] = random.Uniform( -1.0, 1.0 );
            length += dir[ a ] * dir[ a ];
        }
        length = sqrt( length );
        if ( length < 1.0e-3 )
            continue;
        for ( UInt32 a=0; a<Dim; ++a )
            d[ a ] = (T) ( dir[ a ] / length );
        const T maxDistance = (T) random.Uniform( 1.0, 3.0 * extent );
        CCollector found;
        SRayQuery< Dim >::template Run< T >( structure, o, d, maxDistance, found );
        // the scan: the slab test on the closed bounds
        GUCEF::vector< UInt32 > expected;
        for ( size_t i=0; i<items.size(); ++i )
        {
            Float64 tEnter = 0.0, tLeave = (Float64) maxDistance;
            bool hit = true;
            for ( UInt32 a=0; a<Dim && hit; ++a )
            {
                if ( d[ a ] == (T) 0 )
                {
                    hit = !( (Float64) o[ a ] < (Float64) items[ i ].lo[ a ] || (Float64) o[ a ] > (Float64) items[ i ].hi[ a ] );
                    continue;
                }
                Float64 t1 = ( (Float64) items[ i ].lo[ a ] - (Float64) o[ a ] ) / (Float64) d[ a ];
                Float64 t2 = ( (Float64) items[ i ].hi[ a ] - (Float64) o[ a ] ) / (Float64) d[ a ];
                if ( t2 < t1 )
                {
                    const Float64 s = t1; t1 = t2; t2 = s;
                }
                tEnter = tEnter > t1 ? tEnter : t1;
                tLeave = tLeave < t2 ? tLeave : t2;
                hit = tEnter <= tLeave;
            }
            if ( hit )
                expected.push_back( items[ i ].handle );
        }
        if ( !SameSet( found.handles, expected ) || !NoDuplicates( found.handles ) )
        {
            // a ray that grazes a bound can differ by the rounding of the slab test in the kind T: only accept when the differing items are within a hair
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": ray query returns " << found.handles.size() << " items instead of " << expected.size() << ", seed " << seed << " ray " << n
                             << " origin ( " << o[ 0 ] << ", " << o[ 1 ] << ", " << ( Dim > 2 ? o[ 2 ] : (T) 0 ) << " ) direction ( " << d[ 0 ] << ", " << d[ 1 ] << ", " << ( Dim > 2 ? d[ 2 ] : (T) 0 ) << " ) distance " << maxDistance );
            for ( size_t i=0; i<expected.size() && i<4; ++i )
                if ( std::find( found.handles.begin(), found.handles.end(), expected[ i ] ) == found.handles.end() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "    missing item " << expected[ i ] << " lo ( " << items[ expected[ i ] ].lo[ 0 ] << ", " << items[ expected[ i ] ].lo[ 1 ] << " ) hi ( " << items[ expected[ i ] ].hi[ 0 ] << ", " << items[ expected[ i ] ].hi[ 1 ] << " )" );
            return false;
        }
        totalHits += (UInt32) expected.size();
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SpatialStructures: " << name << ": " << rayCount << " ray queries equal the slab test of the scan (" << totalHits << " hits)" );
    return totalHits > 0;
}

/*-------------------------------------------------------------------------*/

/** The octree places items by size: small ones deep, large ones near the root; and empties back to the root */
static bool
OctreeStructure( void )
{
    CTOctree< Float64, UInt32 > tree;
    CTAxisAlignedBox3D< Float64 > world( CTPoint3D< Float64 >( 0, 0, 0 ), CTPoint3D< Float64 >( 64, 64, 64 ) );
    if ( !tree.Initialize( world, 6, 1, 2.0 ) )
        return false;
    if ( tree.Initialize( world, 6, 1, 1.0 ) || tree.Initialize( world, 25, 1, 2.0 ) || tree.Initialize( world, 6, 0, 2.0 ) )
        return false;
    if ( !tree.Initialize( world, 6, 1, 2.0 ) )
        return false;
    // a big item stays near the root, tiny ones go down
    const UInt32 big = tree.Insert( 1, CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( 10, 10, 10 ), CTPoint3D< Float64 >( 40, 40, 40 ) ) );
    // with capacity 1 the root keeps the first item
    if ( 0 != tree.GetItemDepth( big ) )
        return false;
    UInt32 deepest = 0;
    for ( UInt32 i=0; i<50; ++i )
    {
        const Float64 x = 1.0 + ( i % 7 ) * 8.5;
        const Float64 y = 1.0 + ( i % 5 ) * 11.0;
        const Float64 z = 1.0 + ( i % 3 ) * 20.0;
        const UInt32 h = tree.Insert( 100 + i, CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( x, y, z ), CTPoint3D< Float64 >( x + 0.5, y + 0.5, z + 0.5 ) ) );
        if ( tree.GetItemDepth( h ) > deepest )
            deepest = tree.GetItemDepth( h );
    }
    if ( deepest < 3 || deepest > 6 || !tree.Validate() || tree.GetNodeCount() < 5 )
        return false;
    // an item outside the world lives in the root and is found
    const UInt32 far = tree.Insert( 2, CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( 500, 500, 500 ), CTPoint3D< Float64 >( 501, 501, 501 ) ) );
    if ( 0 != tree.GetItemDepth( far ) )
        return false;
    CCollector found;
    tree.Query( CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( 499, 499, 499 ), CTPoint3D< Float64 >( 502, 502, 502 ) ), found );
    if ( 1 != found.handles.size() || found.handles[ 0 ] != far )
        return false;
    // Move keeps the handle
    if ( !tree.Move( far, CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( 1, 1, 1 ), CTPoint3D< Float64 >( 1.1, 1.1, 1.1 ) ) ) || !tree.IsValid( far ) || tree.GetItem( far ) != 2 )
        return false;
    // everything removed: only the root is left
    for ( UInt32 h=0; h<tree.GetCount() + 60; ++h )
        tree.Remove( h );
    return 0 == tree.GetCount() && 1 == tree.GetNodeCount() && tree.Validate();
}

/*-------------------------------------------------------------------------*/

/** The query regions of a grid when the area is much smaller than the items (items clamped to the border cells) and the hashed grid with a huge item */
static bool
GridEdgeCases( void )
{
    CTUniformGrid2D< Float64, UInt32 > grid;
    if ( grid.Initialize( CTPoint2D< Float64 >( 0, 0 ), CTVector2D< Float64 >( 1, 1 ), 0, 4 ) || grid.Initialize( CTPoint2D< Float64 >( 0, 0 ), CTVector2D< Float64 >( 0, 1 ), 4, 4 ) )
        return false;
    if ( !grid.Initialize( CTPoint2D< Float64 >( 0, 0 ), CTVector2D< Float64 >( 1, 1 ), 4, 4 ) )
        return false;
    const UInt32 outside = grid.Insert( 1, CTRectangle< Float64 >( CTPoint2D< Float64 >( 50, 50 ), CTPoint2D< Float64 >( 51, 51 ) ) );
    const UInt32 inside = grid.Insert( 2, CTRectangle< Float64 >( CTPoint2D< Float64 >( 1.5, 1.5 ), CTPoint2D< Float64 >( 2.5, 2.5 ) ) );
    CCollector found;
    grid.Query( CTRectangle< Float64 >( CTPoint2D< Float64 >( 49, 49 ), CTPoint2D< Float64 >( 52, 52 ) ), found );
    if ( 1 != found.handles.size() || found.handles[ 0 ] != outside )
        return false;
    // the clamped item in the corner cell, the other in four cells
    if ( 5 != grid.GetOccupiedCellCount() )
        return false;
    if ( !grid.Remove( inside ) || 1 != grid.GetOccupiedCellCount() )
        return false;
    // the hashed grid: a huge item is refused, a far item is found
    CTSpatialHash< Float64, UInt32 > hash;
    if ( !hash.Initialize( CTPoint3D< Float64 >( 0, 0, 0 ), CTVector3D< Float64 >( 1, 1, 1 ) ) )
        return false;
    if ( CTSpatialHash< Float64, UInt32 >::InvalidHandle != hash.Insert( 1, CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( -1000, -1000, -1000 ), CTPoint3D< Float64 >( 1000, 1000, 1000 ) ) ) )
        return false;
    const UInt32 far = hash.Insert( 2, CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( 1.0e6, -1.0e6, 5.5 ), CTPoint3D< Float64 >( 1.0e6 + 0.5, -1.0e6 + 0.5, 5.7 ) ) );
    CCollector hashFound;
    hash.Query( CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( 1.0e6 - 1, -1.0e6 - 1, 5 ), CTPoint3D< Float64 >( 1.0e6 + 1, -1.0e6 + 1, 6 ) ), hashFound );
    if ( 1 != hashFound.handles.size() || hashFound.handles[ 0 ] != far )
        return false;
    // a query box that covers more cells than the table has slots goes through the table
    CCollector wide;
    hash.Query( CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( -2.0e6, -2.0e6, -2.0e6 ), CTPoint3D< Float64 >( 2.0e6, 2.0e6, 2.0e6 ) ), wide );
    if ( 1 != wide.handles.size() )
        return false;
    // many cells: the table grows and drops emptied cells
    for ( UInt32 i=0; i<2000; ++i )
        hash.Insert( i, CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( (Float64) i, 0.1, 0.1 ), CTPoint3D< Float64 >( i + 0.2, 0.2, 0.2 ) ) );
    if ( !hash.Validate() || 2001 != hash.GetCount() )
        return false;
    for ( UInt32 h=0; h<2001; ++h )
        hash.Remove( h );
    hash.Insert( 9, CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( 3, 3, 3 ), CTPoint3D< Float64 >( 3.1, 3.1, 3.1 ) ) );
    return hash.Validate() && 1 == hash.GetCount();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSpatialStructuresTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "SpatialStructures" );

    GUCEF_TESTFW_TESTCASE( "SpatialStructures: octree and quadtree queries equal the scan of the items after insertions, removals and moves" )
    try
    {
        const UInt32 items = ScaledCount( 10000 );
        const UInt32 queries = ScaledCount( 1000 );
        CTOctree< Float64, UInt32 > octree64;
        CTAxisAlignedBox3D< Float64 > world64( CTPoint3D< Float64 >( -10, -10, -10 ), CTPoint3D< Float64 >( 10, 10, 10 ) );
        ASSERT_TRUE( octree64.Initialize( world64, 8, 8, 2.0 ) );
        ASSERT_TRUE( ( StructureSweep< Float64, 3 >( "Octree Float64", 9101, items, queries, octree64, 10.0 ) ) );
        CTOctree< Float32, UInt32 > octree32;
        CTAxisAlignedBox3D< Float32 > world32( CTPoint3D< Float32 >( -10, -10, -10 ), CTPoint3D< Float32 >( 10, 10, 10 ) );
        ASSERT_TRUE( octree32.Initialize( world32, 6, 4, 1.5f ) );
        ASSERT_TRUE( ( StructureSweep< Float32, 3 >( "Octree Float32 loose 1.5", 9102, items, queries, octree32, 10.0 ) ) );
        CTQuadtree< Float64, UInt32 > quadtree;
        ASSERT_TRUE( quadtree.Initialize( CTRectangle< Float64 >( CTPoint2D< Float64 >( -10, -10 ), CTPoint2D< Float64 >( 10, 10 ) ), 10, 8, 2.0 ) );
        ASSERT_TRUE( ( StructureSweep< Float64, 2 >( "Quadtree Float64", 9103, items, queries, quadtree, 10.0 ) ) );
        CTQuadtree< Float32, UInt32 > quadtree32;
        ASSERT_TRUE( quadtree32.Initialize( CTRectangle< Float32 >( CTPoint2D< Float32 >( -10, -10 ), CTPoint2D< Float32 >( 10, 10 ) ), 5, 2, 3.0f ) );
        ASSERT_TRUE( ( StructureSweep< Float32, 2 >( "Quadtree Float32 loose 3", 9104, items, queries, quadtree32, 10.0 ) ) );
        ASSERT_TRUE( OctreeStructure() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpatialStructures: uniform grids and the spatial hash queries equal the scan of the items after insertions, removals and moves" )
    try
    {
        const UInt32 items = ScaledCount( 10000 );
        const UInt32 queries = ScaledCount( 1000 );
        CTUniformGrid3D< Float64, UInt32 > grid3;
        ASSERT_TRUE( grid3.Initialize( CTPoint3D< Float64 >( -10, -10, -10 ), CTVector3D< Float64 >( 1.0, 1.0, 1.0 ), 20, 20, 20 ) );
        ASSERT_TRUE( ( StructureSweep< Float64, 3 >( "UniformGrid3D Float64", 9201, items, queries, grid3, 10.0 ) ) );
        CTUniformGrid3D< Float32, UInt32 > grid3f;
        ASSERT_TRUE( grid3f.Initialize( CTPoint3D< Float32 >( -10, -10, -10 ), CTVector3D< Float32 >( 2.5f, 1.5f, 0.75f ), 8, 14, 27 ) );
        ASSERT_TRUE( ( StructureSweep< Float32, 3 >( "UniformGrid3D Float32 uneven cells", 9202, items, queries, grid3f, 10.0 ) ) );
        CTUniformGrid2D< Float64, UInt32 > grid2;
        ASSERT_TRUE( grid2.Initialize( CTPoint2D< Float64 >( -10, -10 ), CTVector2D< Float64 >( 0.5, 0.5 ), 40, 40 ) );
        ASSERT_TRUE( ( StructureSweep< Float64, 2 >( "UniformGrid2D Float64", 9203, items, queries, grid2, 10.0 ) ) );
        CTSpatialHash< Float64, UInt32 > hash;
        ASSERT_TRUE( hash.Initialize( CTPoint3D< Float64 >( 0.3, 0.3, 0.3 ), CTVector3D< Float64 >( 1.0, 1.0, 1.0 ) ) );
        ASSERT_TRUE( ( StructureSweep< Float64, 3 >( "SpatialHash Float64", 9204, items, queries, hash, 10.0 ) ) );
        CTSpatialHash< Float32, UInt32 > hashf;
        ASSERT_TRUE( hashf.Initialize( CTPoint3D< Float32 >( 0, 0, 0 ), CTVector3D< Float32 >( 0.7f, 1.3f, 2.1f ) ) );
        ASSERT_TRUE( ( StructureSweep< Float32, 3 >( "SpatialHash Float32 uneven cells", 9205, items, queries, hashf, 10.0 ) ) );
        ASSERT_TRUE( GridEdgeCases() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpatialStructures: ray queries of the grids and the hash visit each item whose bounds the ray touches once" )
    try
    {
        const UInt32 items = ScaledCount( 3000 );
        const UInt32 rays = ScaledCount( 1000 );
        CTUniformGrid3D< Float64, UInt32 > grid3;
        ASSERT_TRUE( grid3.Initialize( CTPoint3D< Float64 >( -10, -10, -10 ), CTVector3D< Float64 >( 1.0, 1.0, 1.0 ), 20, 20, 20 ) );
        ASSERT_TRUE( ( RaySweep< Float64, 3 >( "UniformGrid3D Float64", 9301, items, rays, grid3, 10.0 ) ) );
        CTUniformGrid2D< Float64, UInt32 > grid2;
        ASSERT_TRUE( grid2.Initialize( CTPoint2D< Float64 >( -10, -10 ), CTVector2D< Float64 >( 0.7, 0.9 ), 29, 23 ) );
        ASSERT_TRUE( ( RaySweep< Float64, 2 >( "UniformGrid2D Float64", 9302, items, rays, grid2, 10.0 ) ) );
        CTSpatialHash< Float64, UInt32 > hash;
        ASSERT_TRUE( hash.Initialize( CTPoint3D< Float64 >( 0.1, 0.2, 0.3 ), CTVector3D< Float64 >( 0.8, 0.8, 0.8 ) ) );
        ASSERT_TRUE( ( RaySweep< Float64, 3 >( "SpatialHash Float64", 9303, items, rays, hash, 10.0 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
