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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "TestGeoSpaceReference.h"
#include "TestGeoSpace.h"

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

/** A node that counts its invalidation notifications (the hook of linked objects and gucefPHYSICS bodies) */
class CCountingSpace : public CGeoSpace
{
    public:

    CCountingSpace( void ) : CGeoSpace(), notifications( 0 ) {}

    UInt32 notifications;

    protected:

    virtual void OnWorldPlacementInvalidated( void ) GUCEF_VIRTUAL_OVERRIDE { ++notifications; }
};

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
NearPoint( const CTPoint3D< T >& p, const P3& expected, Float64 tolerance )
{
    return fabs( (Float64) p.x - expected.x ) <= tolerance && fabs( (Float64) p.y - expected.y ) <= tolerance && fabs( (Float64) p.z - expected.z ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 1: the hand case of the design */
template < typename T >
static bool
HandCaseChildFollows( void )
{
    CTGeoSpace< T > parent( CTTransform3D< T >( CTVector3D< T >( T( 0 ), T( 5 ), T( 0 ) ),
                                                CTQuaternion< T >::FromAxisAngle( CTDirectionVector3D< T >::UnitY(), Degrees( T( 90 ) ) ), CTVector3D< T >::One() ) );
    CTGeoSpace< T > child( CTTransform3D< T >( CTVector3D< T >( T( 1 ), T( 0 ), T( 0 ) ), CTQuaternion< T >(), CTVector3D< T >::One() ) );
    if ( !child.SetParent( &parent, false ) )
        return false;
    const CTPoint3D< T > world = child.GetWorldPosition();
    const bool ok = NearPoint( world, P3( 0.0, 5.0, -1.0 ), 1.0e-6 );
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: hand case child at " << (Float64) world.x << ", " << (Float64) world.y << ", " << (Float64) world.z );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 1: three-level chains with non-uniform scales and mirrors equal the explicit matrix products */
template < typename T >
static bool
ThreeLevelChains( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    Float64 worst = 0.0;
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTGeoSpace< T > root( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T > middle( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T > leaf( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
        middle.SetParent( &root, false );
        leaf.SetParent( &middle, false );
        const CTGeoSpace< T >* nodes[ 3 ] = { &root, &middle, &leaf };
        for ( UInt32 k=0; k<3; ++k )
        {
            const Float64 d = RelativeDifference( Widen( nodes[ k ]->GetWorldMatrix() ), ReferenceWorld( *nodes[ k ] ) );
            worst = d > worst ? d : worst;
            if ( d > tolerance )
                ++bad;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: " << count << " three-level chains (kind size " << (UInt32) sizeof( T ) << "), seed " << seed << ": worst relative difference to the Float64 product " << worst );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 1: moving the root moves every descendant by the same world delta */
template < typename T >
static bool
MovingTheRootMovesDescendants( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTGeoSpace< T > root( RandomTransform< T >( r, 5.0, false, 0.5, 2.0, 0.0 ) );
        CTGeoSpace< T > nodes[ 8 ];
        for ( UInt32 k=0; k<8; ++k )
        {
            nodes[ k ].SetLocalTransform( RandomTransform< T >( r, 5.0, false, 0.5, 2.0, 0.0 ) );
            const Int32 parentIndex = r.Index( (Int32) k + 1 ) - 1;
            nodes[ k ].SetParent( parentIndex < 0 ? &root : &nodes[ parentIndex ], false );
        }
        CTPoint3D< T > before[ 8 ];
        for ( UInt32 k=0; k<8; ++k )
            before[ k ] = nodes[ k ].GetWorldPosition();
        const V3 delta = r.Vector( 5.0 );
        root.Translate( ToV< T >( delta ), GEOSPACE_RELATIVETO_WORLD );
        for ( UInt32 k=0; k<8; ++k )
        {
            if ( !NearPoint( nodes[ k ].GetWorldPosition(), Widen( before[ k ] ) + delta, tolerance ) )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: descendant " << k << " did not follow its root, case " << i << ", seed " << seed );
            }
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 2: SetParent( p, true ) keeps world matrices of seeded hierarchies without shear */
template < typename T >
static bool
ReparentKeepsWorld( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    UInt32 refused = 0;
    Float64 worst = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        /* uniform scales (mirrors included) everywhere: no world matrix has shear */
        CTGeoSpace< T > nodes[ 6 ];
        for ( UInt32 k=0; k<6; ++k )
        {
            nodes[ k ].SetLocalTransform( RandomTransform< T >( r, 10.0, true, 0.5, 2.0, 0.2 ) );
            const Int32 parentIndex = r.Index( (Int32) k + 1 ) - 1;
            if ( parentIndex >= 0 )
                nodes[ k ].SetParent( &nodes[ parentIndex ], false );
        }
        const UInt32 moved = (UInt32) r.Index( 6 );
        const Int32 target = r.Index( 7 ) - 1;
        CTGeoSpace< T >* newParent = target < 0 ? GUCEF_NULL : &nodes[ target ];
        const CTMatrix4D< T > before = nodes[ moved ].GetWorldMatrix();
        const bool cycle = GUCEF_NULL != newParent && ( newParent == &nodes[ moved ] || nodes[ moved ].IsAncestorOf( *newParent ) );
        const CTGeoSpace< T >* oldParent = nodes[ moved ].GetParent();
        const bool linked = nodes[ moved ].SetParent( newParent, true );
        if ( cycle )
        {
            ++refused;
            if ( linked || nodes[ moved ].GetParent() != oldParent )
                ++bad;
            continue;
        }
        const Float64 d = RelativeDifference( Widen( nodes[ moved ].GetWorldMatrix() ), Widen( before ) );
        worst = d > worst ? d : worst;
        if ( !linked || nodes[ moved ].GetParent() != newParent || d > tolerance )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: re-parenting changed the world matrix by " << d << ", case " << i << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: re-parenting seed " << seed << ": " << count << " cases, " << refused << " cycles refused, worst relative world change " << worst );
    return 0 == bad && refused > 0;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 2: SetParent( p, false ) keeps the local transform; cycles are refused */
static bool
ReparentKeepsLocalAndRefusesCycles( void )
{
    CRandom r( 6205 );
    CGeoSpace a( RandomTransform< Float32 >( r, 3.0, false, 0.5, 2.0, 0.0 ) );
    CGeoSpace b( RandomTransform< Float32 >( r, 3.0, false, 0.5, 2.0, 0.0 ) );
    CGeoSpace c( RandomTransform< Float32 >( r, 3.0, false, 0.5, 2.0, 0.0 ) );
    CGeoSpace other( RandomTransform< Float32 >( r, 3.0, false, 0.5, 2.0, 0.0 ) );
    bool ok = b.SetParent( &a, false ) && c.SetParent( &b, false );
    const CGeoSpace::TTransform local = c.GetLocalTransform();
    ok = ok && c.SetParent( &other, false ) && c.GetLocalTransform() == local && c.GetParent() == &other;
    ok = ok && 0 == b.GetChildCount() && 1 == other.GetChildCount();
    ok = ok && c.SetParent( &b, false );
    /* cycles: a node under itself or under its descendants */
    ok = ok && !a.SetParent( &a, true ) && !a.SetParent( &b, false ) && !a.SetParent( &c, true ) && GUCEF_NULL == a.GetParent();
    ok = ok && !b.SetParent( &c, false ) && b.GetParent() == &a;
    /* re-linking to the current parent changes nothing */
    ok = ok && b.SetParent( &a, true ) && 1 == a.GetChildCount();
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 2: destroying a parent leaves its children as roots at their former world placement */
template < typename T >
static bool
DestroyingParentKeepsChildrenWorld( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTGeoSpace< T > grandParent( RandomTransform< T >( r, 10.0, true, 0.5, 2.0, 0.2 ) );
        CTGeoSpace< T >* parent = new CTGeoSpace< T >( RandomTransform< T >( r, 10.0, true, 0.5, 2.0, 0.2 ) );
        parent->SetParent( &grandParent, false );
        CTGeoSpace< T > children[ 3 ];
        CTGeoSpace< T > grandChild( RandomTransform< T >( r, 2.0, false, 0.5, 2.0, 0.0 ) );
        CTMatrix4D< T > before[ 3 ];
        for ( UInt32 k=0; k<3; ++k )
        {
            /* the children may scale non-uniformly: under a similarity parent their world has no shear */
            children[ k ].SetLocalTransform( RandomTransform< T >( r, 10.0, false, 0.5, 2.0, 0.2 ) );
            children[ k ].SetParent( parent, false );
            before[ k ] = children[ k ].GetWorldMatrix();
        }
        grandChild.SetParent( &children[ 1 ], false );
        const CTMatrix4D< T > grandChildBefore = grandChild.GetWorldMatrix();
        delete parent;
        bool ok = 0 == grandParent.GetChildCount();
        for ( UInt32 k=0; k<3; ++k )
            ok = ok && GUCEF_NULL == children[ k ].GetParent() && RelativeDifference( Widen( children[ k ].GetWorldMatrix() ), Widen( before[ k ] ) ) <= tolerance;
        ok = ok && grandChild.GetParent() == &children[ 1 ] && RelativeDifference( Widen( grandChild.GetWorldMatrix() ), Widen( grandChildBefore ) ) <= tolerance * 10.0;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: destroying a parent moved its children, case " << i << ", seed " << seed );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 2: destroying a child removes it from its parent; the order of the others stays */
static bool
DestroyingChildDetaches( void )
{
    CGeoSpace parent;
    CGeoSpace first;
    CGeoSpace* middle = new CGeoSpace();
    CGeoSpace last;
    first.SetParent( &parent, false );
    middle->SetParent( &parent, false );
    last.SetParent( &parent, false );
    bool ok = 3 == parent.GetChildCount() && parent.GetChild( 1 ) == middle;
    delete middle;
    ok = ok && 2 == parent.GetChildCount() && parent.GetChild( 0 ) == &first && parent.GetChild( 1 ) == &last && GUCEF_NULL == parent.GetChild( 2 );
    ok = ok && parent.GetFirstChild() == &first && first.GetNextSibling() == &last && GUCEF_NULL == last.GetNextSibling();
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The hierarchy queries: root, depth, ancestors and the lowest common ancestor */
static bool
HierarchyQueries( void )
{
    CGeoSpace root;
    CGeoSpace a;
    CGeoSpace b;
    CGeoSpace a1;
    CGeoSpace a2;
    CGeoSpace b1;
    CGeoSpace stranger;
    a.SetParent( &root, false );
    b.SetParent( &root, false );
    a1.SetParent( &a, false );
    a2.SetParent( &a, false );
    b1.SetParent( &b, false );
    bool ok = a1.GetRoot() == &root && root.GetRoot() == &root && 2 == a1.GetDepth() && 0 == root.GetDepth();
    ok = ok && root.IsAncestorOf( a1 ) && a.IsAncestorOf( a2 ) && !a.IsAncestorOf( b1 ) && !a1.IsAncestorOf( a1 ) && !a1.IsAncestorOf( a );
    ok = ok && a1.GetLowestCommonAncestor( a2 ) == &a && a1.GetLowestCommonAncestor( b1 ) == &root && a1.GetLowestCommonAncestor( a ) == &a;
    ok = ok && a.GetLowestCommonAncestor( a1 ) == &a && a1.GetLowestCommonAncestor( a1 ) == &a1 && GUCEF_NULL == a1.GetLowestCommonAncestor( stranger );
    ok = ok && 2 == root.GetChildCount() && root.GetChild( 0 ) == &a && root.GetChild( 1 ) == &b;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 3: updated values after a change, no recomputation without a change, eager equals lazy */
static bool
CachingAndRecomputeCounter( UInt64 seed )
{
    CRandom r( seed );
    bool ok = true;

    /* a chain: changing the root updates the leaf, reading twice recomputes once */
    CGeoSpace chain[ 4 ];
    for ( UInt32 k=0; k<4; ++k )
    {
        chain[ k ].SetLocalTransform( RandomTransform< Float32 >( r, 5.0, false, 0.5, 2.0, 0.0 ) );
        if ( k > 0 )
            chain[ k ].SetParent( &chain[ k - 1 ], false );
    }
    chain[ 3 ].GetWorldMatrix();
    const UInt32 leafCount = chain[ 3 ].GetWorldRecomputeCount();
    const UInt32 rootCount = chain[ 0 ].GetWorldRecomputeCount();
    chain[ 3 ].GetWorldMatrix();
    chain[ 3 ].GetWorldPosition();
    chain[ 3 ].IsWorldTransformSimilarity();
    ok = ok && leafCount == chain[ 3 ].GetWorldRecomputeCount() && rootCount == chain[ 0 ].GetWorldRecomputeCount();
    chain[ 0 ].SetLocalTransform( RandomTransform< Float32 >( r, 5.0, false, 0.5, 2.0, 0.0 ) );
    ok = ok && RelativeDifference( Widen( chain[ 3 ].GetWorldMatrix() ), ReferenceWorld( chain[ 3 ] ) ) <= 1.0e-5;
    ok = ok && leafCount + 1 == chain[ 3 ].GetWorldRecomputeCount() && rootCount + 1 == chain[ 0 ].GetWorldRecomputeCount();
    chain[ 3 ].GetWorldMatrix();
    ok = ok && leafCount + 1 == chain[ 3 ].GetWorldRecomputeCount();

    /* a large tree: a change recomputes only the chain that is read */
    const UInt32 count = 1000;
    GUCEF::vector< CGeoSpace* > nodes;
    for ( UInt32 k=0; k<count; ++k )
    {
        nodes.push_back( new CGeoSpace( RandomTransform< Float32 >( r, 3.0, false, 0.8, 1.25, 0.0 ) ) );
        if ( k > 0 )
            nodes[ k ]->SetParent( nodes[ (UInt32) r.Index( (Int32) k ) ], false );
    }
    nodes[ 0 ]->UpdateWorldMatrices();
    UInt32 totalBefore = 0;
    for ( UInt32 k=0; k<count; ++k )
        totalBefore += nodes[ k ]->GetWorldRecomputeCount();
    ok = ok && count == totalBefore;
    CGeoSpace* leaf = nodes[ count - 1 ];
    CGeoSpace* changed = leaf->GetParent();
    changed->Translate( CGeoSpace::TVector( 1.0f, 0.0f, 0.0f ), GEOSPACE_RELATIVETO_PARENT );
    leaf->GetWorldMatrix();
    UInt32 totalAfter = 0;
    for ( UInt32 k=0; k<count; ++k )
        totalAfter += nodes[ k ]->GetWorldRecomputeCount();
    ok = ok && totalBefore + 2 == totalAfter;

    /* eager equals lazy, bit for bit, for world and inverse world matrices */
    GUCEF::vector< CGeoSpace* > lazy;
    for ( UInt32 k=0; k<count; ++k )
    {
        lazy.push_back( new CGeoSpace( nodes[ k ]->GetLocalTransform() ) );
        if ( k > 0 )
        {
            UInt32 parentIndex = 0;
            while ( nodes[ parentIndex ] != nodes[ k ]->GetParent() )
                ++parentIndex;
            lazy[ k ]->SetParent( lazy[ parentIndex ], false );
        }
    }
    nodes[ 0 ]->UpdateWorldMatrices();
    UInt32 mismatches = 0;
    for ( UInt32 k=count; k>0; --k )
    {
        if ( !SameMatrixBits( lazy[ k - 1 ]->GetWorldMatrix(), nodes[ k - 1 ]->GetWorldMatrix() ) ||
             !SameMatrixBits( lazy[ k - 1 ]->GetInverseWorldMatrix(), nodes[ k - 1 ]->GetInverseWorldMatrix() ) )
            ++mismatches;
    }
    ok = ok && 0 == mismatches;

    /* the inverse world matrix inverts the world matrix */
    Float64 worstInverse = 0.0;
    for ( UInt32 k=0; k<count; ++k )
    {
        const Float64 d = RelativeDifference( Widen( nodes[ k ]->GetInverseWorldMatrix() * nodes[ k ]->GetWorldMatrix() ), M4() );
        worstInverse = d > worstInverse ? d : worstInverse;
    }
    ok = ok && worstInverse <= 1.0e-4;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: caching seed " << seed << ": eager / lazy mismatches " << mismatches << ", worst |inverse * world - I| " << worstInverse );

    for ( UInt32 k=count; k>0; --k )
    {
        delete lazy[ k - 1 ];
        delete nodes[ k - 1 ];
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The invalidation hook: always for a directly changed node, once per valid cache for descendants */
static bool
InvalidationHook( void )
{
    CCountingSpace root;
    CCountingSpace child;
    CCountingSpace grandChild;
    child.SetParent( &root, false );
    grandChild.SetParent( &child, false );
    root.notifications = child.notifications = grandChild.notifications = 0;

    /* fresh nodes are armed: the first change reaches the whole chain */
    root.SetLocalPosition( CGeoSpace::TVector( 1.0f, 0.0f, 0.0f ) );
    bool ok = 1 == root.notifications && 1 == child.notifications && 1 == grandChild.notifications;
    grandChild.GetWorldMatrix();
    root.SetLocalPosition( CGeoSpace::TVector( 2.0f, 0.0f, 0.0f ) );
    ok = ok && 2 == root.notifications && 2 == child.notifications && 2 == grandChild.notifications;
    /* no read in between: the descendants are not told again, the changed node is */
    root.SetLocalPosition( CGeoSpace::TVector( 3.0f, 0.0f, 0.0f ) );
    ok = ok && 3 == root.notifications && 2 == child.notifications && 2 == grandChild.notifications;
    /* a teleport of a node whose cache is invalid (a body after a guarded sync-out write) is still reported */
    child.SetLocalPosition( CGeoSpace::TVector( 0.0f, 1.0f, 0.0f ) );
    ok = ok && 3 == child.notifications && 2 == grandChild.notifications;
    /* after a read the subtree hears of the next change once */
    grandChild.GetWorldMatrix();
    child.Translate( CGeoSpace::TVector( 0.0f, 1.0f, 0.0f ), GEOSPACE_RELATIVETO_LOCAL );
    ok = ok && 3 == root.notifications && 4 == child.notifications && 3 == grandChild.notifications;
    /* only the inverse read also arms the notification */
    grandChild.GetInverseWorldMatrix();
    root.SetLocalScale( CGeoSpace::TVector( 2.0f, 2.0f, 2.0f ) );
    ok = ok && 4 == root.notifications && 5 == child.notifications && 4 == grandChild.notifications;
    ok = ok && grandChild.IsWorldPlacementDirty();
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Translate, Rotate, LookAt, SetWorldTransform, SetWorldPosition, world rotation and scale, the similarity flag */
template < typename T >
static bool
PlacementMembers( UInt64 seed, UInt32 count, Float64 tolerance )
{
    typedef CTGeoSpace< T > TSpace;
    typedef CTVector3D< T > TVector;
    typedef CTQuaternion< T > TQuaternion;
    CRandom r( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TSpace parent( RandomTransform< T >( r, 5.0, true, 0.5, 2.0, 0.0 ) );
        TSpace node( RandomTransform< T >( r, 5.0, true, 0.5, 2.0, 0.0 ) );
        node.SetParent( &parent, false );
        bool ok = node.IsWorldTransformSimilarity();

        /* WORLD translation moves the world position by the delta */
        const V3 delta = r.Vector( 2.0 );
        const P3 before = Widen( node.GetWorldPosition() );
        node.Translate( ToV< T >( delta ), GEOSPACE_RELATIVETO_WORLD );
        ok = ok && NearPoint( node.GetWorldPosition(), before + delta, tolerance * 20.0 );

        /* LOCAL translation moves along the node's own rotated axes, PARENT along the parent's */
        const typename TSpace::TTransform local = node.GetLocalTransform();
        node.Translate( TVector( T( 1 ), T( 0 ), T( 0 ) ), GEOSPACE_RELATIVETO_LOCAL );
        const V3 movedLocal = Widen( node.GetLocalTransform().position - local.position );
        const V3 axisX = Widen( local.rotation.Rotate( TVector( T( 1 ), T( 0 ), T( 0 ) ) ) );
        ok = ok && ( movedLocal - axisX ).GetLength() <= tolerance * 10.0;
        node.Translate( TVector( T( 0 ), T( 2 ), T( 0 ) ), GEOSPACE_RELATIVETO_PARENT );
        ok = ok && fabs( (Float64) ( node.GetLocalTransform().position.y - local.position.y ) - ( 2.0 + axisX.y ) ) <= tolerance * 10.0;

        /* WORLD rotation: the world rotation becomes q * the old world rotation */
        const Q4 q = r.Rotation();
        const Q4 worldBefore = Widen( node.GetWorldRotation() );
        node.Rotate( ToQ< T >( q ), GEOSPACE_RELATIVETO_WORLD );
        ok = ok && Widen( node.GetWorldRotation() ).EqualsRotation( q * worldBefore, CTRadian< Float64 >( tolerance * 100.0 ) );
        /* LOCAL and PARENT rotations compose on the right and on the left */
        const TQuaternion localRotation = node.GetLocalTransform().rotation;
        const Q4 q2 = r.Rotation();
        node.Rotate( ToQ< T >( q2 ), GEOSPACE_RELATIVETO_LOCAL );
        ok = ok && Widen( node.GetLocalTransform().rotation ).EqualsRotation( Widen( localRotation ) * q2, CTRadian< Float64 >( tolerance * 100.0 ) );
        node.Rotate( ToQ< T >( q2 ), GEOSPACE_RELATIVETO_PARENT );
        ok = ok && Widen( node.GetLocalTransform().rotation ).EqualsRotation( q2 * Widen( localRotation ) * q2, CTRadian< Float64 >( tolerance * 100.0 ) );

        /* LookAt: the forward axis of the system points at the target */
        const P3 target = r.Point( 20.0 );
        if ( ( target - Widen( node.GetWorldPosition() ) ).GetLength() > 1.0 && node.LookAt( ToP< T >( target ), TVector( T( 0 ), T( 1 ), T( 0 ) ), CCoordinateSystem::GucefDefault() ) )
        {
            const D3 forward = WidenD( node.GetWorldRotation().GetForward( CCoordinateSystem::GucefDefault() ) );
            V3 toTarget = target - Widen( node.GetWorldPosition() );
            toTarget = toTarget * ( 1.0 / toTarget.GetLength() );
            ok = ok && ( forward.AsVector() - toTarget ).GetLength() <= tolerance * 100.0;
        }

        /* SetWorldTransform and SetWorldPosition place the node in world coordinates */
        const typename TSpace::TTransform wanted = RandomTransform< T >( r, 5.0, false, 0.5, 2.0, 0.0 );
        node.SetWorldTransform( wanted );
        ok = ok && RelativeDifference( Widen( node.GetWorldMatrix() ), WideMatrix( wanted ) ) <= tolerance * 10.0;
        const P3 place = r.Point( 5.0 );
        node.SetWorldPosition( ToP< T >( place ) );
        ok = ok && NearPoint( node.GetWorldPosition(), place, tolerance * 20.0 );

        /* world rotation and scale of a shear-free world, and the similarity flag */
        const V3 scale = Widen( node.GetWorldScaleApproximation() );
        const V3 wantedScale = Widen( wanted.scale );
        ok = ok && fabs( fabs( scale.x ) - fabs( wantedScale.x ) ) <= tolerance * 10.0 && fabs( scale.y - wantedScale.y ) <= tolerance * 10.0;
        ok = ok && !node.IsWorldTransformSimilarity() && node.GetWorldRotation().IsFinite();
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: placement members failed in case " << i << ", seed " << seed );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/**
 *  The notification contract: a consumer that only reads through GetTransformToPrecise (gucefPHYSICS
 *  sync-in of kinematic bodies) hears of every ancestor move once; a node that was never read hears of the first one;
 *  RearmPlacementNotification arms explicitly.
 */
static bool
NotificationForPreciseReadersAndNeverReadNodes( void )
{
    CCountingSpace root;
    CCountingSpace ship;
    CCountingSpace kinematic;
    ship.SetParent( &root, false );
    kinematic.SetParent( &ship, false );
    kinematic.GetWorldMatrix();
    kinematic.notifications = 0;
    bool ok = true;
    for ( UInt32 move=1; move<=5; ++move )
    {
        ship.SetLocalPosition( CGeoSpace::TVector( (Float32) move, 0.0f, 0.0f ) );
        ok = ok && move == kinematic.notifications;
        /* sync-in: the precise read re-arms */
        kinematic.GetTransformToPrecise( root );
        /* a second move before the next read is not reported again */
    }
    ship.SetLocalPosition( CGeoSpace::TVector( 9.0f, 0.0f, 0.0f ) );
    ship.SetLocalPosition( CGeoSpace::TVector( 10.0f, 0.0f, 0.0f ) );
    ok = ok && 6 == kinematic.notifications;

    /* a node linked and never read */
    CCountingSpace parent;
    CCountingSpace child;
    child.SetParent( &parent, false );
    child.notifications = 0;
    parent.SetLocalPosition( CGeoSpace::TVector( 1.0f, 0.0f, 0.0f ) );
    ok = ok && 1 == child.notifications && !child.IsPlacementNotificationArmed();
    parent.SetLocalPosition( CGeoSpace::TVector( 2.0f, 0.0f, 0.0f ) );
    ok = ok && 1 == child.notifications;
    child.RearmPlacementNotification();
    ok = ok && child.IsPlacementNotificationArmed();
    parent.SetLocalPosition( CGeoSpace::TVector( 3.0f, 0.0f, 0.0f ) );
    ok = ok && 2 == child.notifications;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The world image of an axis of a node, normalised, in Float64 */
template < typename T >
static V3
AxisImage( const CTGeoSpace< T >& node, const CTVector3D< T >& axis )
{
    V3 v = Widen( node.ToWorld( axis ) );
    return v * ( 1.0 / v.GetLength() );
}

/**
 *  Rotate( WORLD ) and LookAt under non-uniform and mirrored parents: the world matrix after a WORLD rotation is the
 *  rotation applied to the old one (or the call refuses when that needs shear, leaving the node unchanged); after LookAt
 *  the image of the forward axis points at the target, the image of the up axis leans towards the hint, and the
 *  snapshot frame agrees with the matrix
 */
template < typename T >
static bool
RotateAndLookAtUnderScaledParents( UInt64 seed, UInt32 count, Float64 tolerance )
{
    typedef CTGeoSpace< T > TSpace;
    typedef CTVector3D< T > TVector;
    CRandom r( seed );
    const CCoordinateSystem system = CCoordinateSystem::GucefDefault();
    UInt32 bad = 0;
    UInt32 refused = 0;
    UInt32 solved = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        /* parents: mirrored, axis-aligned non-uniform, rotated non-uniform (shear for most children), uniform */
        const UInt32 kind = i % 4;
        const V3 scales[ 4 ] = { V3( -1.0, 1.0, 1.0 ), V3( 4.0, 1.0, 1.0 ), V3( 1.0, 3.0, 0.5 ), V3( 2.0, 2.0, 2.0 ) };
        const Q4 parentRotation = 2 == kind ? r.Rotation() : Q4();
        const P3 parentPosition = r.Point( 5.0 );
        TSpace parent( typename TSpace::TTransform( ToV< T >( parentPosition.ToPositionVector() ), ToQ< T >( parentRotation ), ToV< T >( scales[ kind ] ) ) );
        const P3 nodePosition = r.Point( 3.0 );
        const Q4 nodeRotation = r.Rotation();
        TSpace node( typename TSpace::TTransform( ToV< T >( nodePosition.ToPositionVector() ), ToQ< T >( nodeRotation ), TVector::One() ) );
        node.SetParent( &parent, false );
        bool ok = true;
        UInt32 failedCheck = 0;

        /* Rotate WORLD: about the world position, applied after the old world matrix */
        const Q4 q = r.Rotation();
        const M4 before = Widen( node.GetWorldMatrix() );
        const typename TSpace::TTransform localBefore = node.GetLocalTransform();
        if ( node.Rotate( ToQ< T >( q ), GEOSPACE_RELATIVETO_WORLD ) )
        {
            ++solved;
            M4 expected( q.ToRotationMatrix3D() * before.GetLinear() );
            expected.SetTranslation( before.GetTranslation() );
            ok = ok && RelativeDifference( Widen( node.GetWorldMatrix() ), expected ) <= tolerance * 10.0;
            failedCheck = ok ? 0 : 1;
        }
        else
        {
            ++refused;
            /* only a non-uniform parent can make the rotated child need shear */
            ok = ok && node.GetLocalTransform() == localBefore && ( 1 == kind || 2 == kind );
        }

        /* LookAt: exact forward, up towards the hint, snapshot consistent with the matrix */
        const P3 target = r.Point( 20.0 );
        const V3 toTarget = target - Widen( node.GetWorldPosition() );
        if ( toTarget.GetLength() > 1.0 )
        {
            const bool okBefore = ok;
            ok = ok && node.LookAt( ToP< T >( target ), TVector( T( 0 ), T( 1 ), T( 0 ) ), system );
            const V3 forward = AxisImage( node, system.GetForward< T >() );
            const V3 wanted = toTarget * ( 1.0 / toTarget.GetLength() );
            ok = ok && ( forward - wanted ).GetLength() <= tolerance * 10.0;
            const bool forwardOk = ok;
            ok = ok && AxisImage( node, system.GetUp< T >() ).Dot( V3( 0.0, 1.0, 0.0 ) ) > 0.0;
            const bool upOk = ok;
            CTGeoSpaceSnapshot< T > snapshot;
            node.CaptureSnapshot( snapshot, system );
            ok = ok && ( WidenD( snapshot.forward ).AsVector() - forward ).GetLength() <= tolerance * 10.0;
            ok = ok && fabs( WidenD( snapshot.up ).AsVector().Dot( forward ) ) <= tolerance * 10.0;
            if ( okBefore && !ok )
                failedCheck = !forwardOk ? 2 : ( !upOk ? 3 : 4 );
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: rotate / look at under parent kind " << kind << " failed in case " << i << " (check " << failedCheck << "), seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: WORLD rotations under scaled and mirrored parents, seed " << seed << ": " << solved << " solved exactly, " << refused << " refused (they need shear)" );
    return 0 == bad && solved >= count / 2;
}

/*-------------------------------------------------------------------------*/

/**
 *  Placements that need shear are reported, and bounded: the position and the axis lengths stay exact, the first axis
 *  direction too; the other axes differ by the dropped shear. Covers SetParent( keepWorld ), SetWorldTransform and the
 *  re-rooting of children by a destroyed parent.
 */
static bool
ShearPlacementsReported( UInt64 seed, UInt32 count )
{
    CRandom r( seed );
    UInt32 bad = 0;
    UInt32 approximated = 0;
    Float64 worstShear = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CGeoSpace64* parent = new CGeoSpace64( RandomTransform< Float64 >( r, 5.0, false, 0.5, 3.0, 0.2 ) );
        CGeoSpace64 child( RandomTransform< Float64 >( r, 5.0, false, 0.5, 3.0, 0.2 ) );
        child.SetParent( parent, false );
        const M4 world = child.GetWorldMatrix();
        bool exact = true;
        CGeoSpace64::TTransform trs;
        exact = child.GetWorldTransform( trs );
        bool approximate = false;
        child.SetParent( GUCEF_NULL, true, &approximate );
        const M4 after = child.GetWorldMatrix();
        bool ok = approximate == !exact;
        approximated += approximate ? 1 : 0;
        /* position, axis lengths and the first axis exact */
        ok = ok && ( after.GetTranslation() - world.GetTranslation() ).GetLength() <= 1.0e-12 * ( 1.0 + world.GetTranslation().GetLength() );
        for ( UInt32 c=0; c<3; ++c )
            ok = ok && fabs( after.GetLinear().GetColumn( c ).GetLength() - world.GetLinear().GetColumn( c ).GetLength() ) <= 1.0e-12 * 10.0;
        ok = ok && ( after.GetLinear().GetColumn( 0 ) - world.GetLinear().GetColumn( 0 ) ).GetLength() <= 1.0e-11;
        const Float64 shear = RelativeDifference( after, world );
        worstShear = shear > worstShear ? shear : worstShear;
        ok = ok && ( approximate || shear <= 1.0e-12 );

        /* SetWorldTransform into the sheared parent space reports the approximation when one is needed */
        child.SetParent( parent, false );
        const CGeoSpace64::TTransform wanted = RandomTransform< Float64 >( r, 5.0, false, 0.5, 3.0, 0.0 );
        const bool placedExactly = child.SetWorldTransform( wanted );
        const Float64 placementError = RelativeDifference( child.GetWorldMatrix(), WideMatrix( wanted ) );
        ok = ok && ( placedExactly ? placementError <= 1.0e-11 : true ) && ( child.GetWorldPosition() - P3( wanted.position ) ).GetLength() <= 1.0e-11;

        /* destroying the parent keeps the child's position and axis lengths */
        const M4 beforeDestroy = child.GetWorldMatrix();
        delete parent;
        const M4 afterDestroy = child.GetWorldMatrix();
        ok = ok && GUCEF_NULL == child.GetParent() && ( afterDestroy.GetTranslation() - beforeDestroy.GetTranslation() ).GetLength() <= 1.0e-11;
        for ( UInt32 c=0; c<3; ++c )
            ok = ok && fabs( afterDestroy.GetLinear().GetColumn( c ).GetLength() - beforeDestroy.GetLinear().GetColumn( c ).GetLength() ) <= 1.0e-11;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: shear placement case " << i << " failed, seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: placements needing shear, seed " << seed << ": " << approximated << " of " << count << " reported approximated, worst dropped shear " << worstShear );
    return 0 == bad && approximated > 0;
}

/*-------------------------------------------------------------------------*/

/** The Float64 image of the rotation q applied after the linear part of a world matrix, about its position */
static M4
RotatedAfter( const Q4& q, const M4& world )
{
    M4 expected( q.ToRotationMatrix3D() * world.GetLinear() );
    expected.SetTranslation( world.GetTranslation() );
    return expected;
}

/**
 *  Fixed point (amendment A-1) tolerates the rounding of its products: under chains of rotated, uniformly scaled
 *  parents every node is a similarity, GetWorldTransform and SetParent( keepWorld ) are exact (both ways), Rotate( WORLD )
 *  and LookAt are solved; under a rotated parent with a non-uniform scale a rotated child is reported (not a
 *  similarity, GetWorldTransform false, 'approximated' from SetParent) and a WORLD rotation that needs shear is
 *  refused, leaving the node unchanged
 */
static bool
FixedPointSimilarityTolerance( UInt64 seed, UInt32 count )
{
    typedef CTGeoSpace< TFixed32_32 > TSpace;
    typedef TSpace::TTransform TTrs;
    typedef TSpace::TVector TVector;
    CRandom r( seed );
    const CCoordinateSystem system = CCoordinateSystem::GucefDefault();
    const Float64 tolerance = 1.0e-7;
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        bool ok = true;
        UInt32 failedCheck = 0;

        /* similarity chain: grand and parent rotated with uniform scales, the node rotated with unit or uniform scale */
        const TTrs grandTrs = RandomTransform< TFixed32_32 >( r, 20.0, true, 0.25, 4.0, 0.0 );
        const TTrs parentTrs = RandomTransform< TFixed32_32 >( r, 20.0, true, 0.25, 4.0, 0.0 );
        const TTrs nodeTrs = RandomTransform< TFixed32_32 >( r, 10.0, true, 0.5, 2.0, 0.0 );
        TSpace grand( grandTrs );
        TSpace parent( parentTrs );
        TSpace node( nodeTrs );
        parent.SetParent( &grand, false );
        node.SetParent( &parent, false );
        ok = ok && parent.IsWorldTransformSimilarity() && node.IsWorldTransformSimilarity();
        failedCheck = ok || 0 != failedCheck ? failedCheck : 1;
        TTrs world;
        ok = ok && node.GetWorldTransform( world ) && RelativeDifference( Widen( world.ToMatrix() ), Widen( node.GetWorldMatrix() ) ) <= tolerance;
        failedCheck = ok || 0 != failedCheck ? failedCheck : 2;

        /* keepWorld re-parenting to a root and back is exact and keeps the world matrix */
        TSpace other( RandomTransform< TFixed32_32 >( r, 5.0, true, 1.0, 1.0, 0.0 ) );
        const M4 before = Widen( node.GetWorldMatrix() );
        bool approximated = true;
        ok = ok && node.SetParent( &other, true, &approximated ) && !approximated && RelativeDifference( Widen( node.GetWorldMatrix() ), before ) <= tolerance;
        approximated = true;
        ok = ok && node.SetParent( &parent, true, &approximated ) && !approximated && RelativeDifference( Widen( node.GetWorldMatrix() ), before ) <= tolerance;
        failedCheck = ok || 0 != failedCheck ? failedCheck : 3;

        /* Rotate( WORLD ): solved, the rotation applied after the old world matrix */
        const Q4 q = r.Rotation();
        const M4 beforeRotate = Widen( node.GetWorldMatrix() );
        ok = ok && node.Rotate( ToQ< TFixed32_32 >( q ), GEOSPACE_RELATIVETO_WORLD ) && RelativeDifference( Widen( node.GetWorldMatrix() ), RotatedAfter( q, beforeRotate ) ) <= tolerance * 10.0;
        failedCheck = ok || 0 != failedCheck ? failedCheck : 4;

        /* LookAt: the image of the forward axis points at the target */
        const P3 target = r.Point( 50.0 );
        const M4 placed = Widen( node.GetWorldMatrix() );
        const V3 toTarget = target - P3( placed.GetTranslation() );
        if ( toTarget.GetLength() > 1.0 )
        {
            ok = ok && node.LookAt( ToP< TFixed32_32 >( target ), TVector( TFixed32_32( 0 ), TFixed32_32( 1 ), TFixed32_32( 0 ) ), system );
            const CTVector3D< TFixed32_32 > forwardAxis = system.GetForward< TFixed32_32 >();
            V3 forward = Widen( node.GetWorldMatrix() ).GetLinear() * V3( FixedToFloat64( forwardAxis.x ), FixedToFloat64( forwardAxis.y ), FixedToFloat64( forwardAxis.z ) );
            forward = forward * ( 1.0 / forward.GetLength() );
            ok = ok && ( forward - toTarget * ( 1.0 / toTarget.GetLength() ) ).GetLength() <= tolerance * 10.0 && node.IsWorldTransformSimilarity();
            failedCheck = ok || 0 != failedCheck ? failedCheck : 5;
        }

        /* a rotated parent with a non-uniform scale: a rotated child needs shear */
        const Q4 shearParentRotation = r.Rotation();
        const Float64 sx = r.Uniform( 0.5, 1.0 );
        const Float64 sy = r.Uniform( 1.5, 3.0 );
        TSpace shearParent( TTrs( TVector(), ToQ< TFixed32_32 >( shearParentRotation ), ToV< TFixed32_32 >( V3( sx, sy, 1.0 ) ) ) );
        const Q4 childRotation = r.Rotation();
        TSpace child( TTrs( TVector(), ToQ< TFixed32_32 >( childRotation ), TVector::One() ) );
        child.SetParent( &shearParent, false );
        TTrs childWorld;
        ok = ok && !child.IsWorldTransformSimilarity() && !child.GetWorldTransform( childWorld );
        failedCheck = ok || 0 != failedCheck ? failedCheck : 6;
        const TTrs childLocal = child.GetLocalTransform();
        const Q4 shearRotation = r.Rotation();
        ok = ok && !child.Rotate( ToQ< TFixed32_32 >( shearRotation ), GEOSPACE_RELATIVETO_WORLD ) && child.GetLocalTransform() == childLocal;
        failedCheck = ok || 0 != failedCheck ? failedCheck : 7;
        approximated = false;
        ok = ok && child.SetParent( GUCEF_NULL, true, &approximated ) && approximated;
        failedCheck = ok || 0 != failedCheck ? failedCheck : 8;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: fixed point similarity case " << i << " failed (check " << failedCheck << "), seed " << seed );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** MoveChildrenTo moves every child in order (iterating children while re-parenting needs the saved next sibling) */
static bool
MoveChildrenKeepsOrder( void )
{
    CGeoSpace from;
    CGeoSpace to;
    CGeoSpace children[ 5 ];
    for ( UInt32 k=0; k<5; ++k )
    {
        children[ k ].SetLocalPosition( CGeoSpace::TVector( (Float32) k, 0.0f, 0.0f ) );
        children[ k ].SetParent( &from, false );
    }
    from.SetLocalPosition( CGeoSpace::TVector( 0.0f, 7.0f, 0.0f ) );
    bool ok = 5 == from.MoveChildrenTo( &to, true ) && 0 == from.GetChildCount() && 5 == to.GetChildCount();
    for ( UInt32 k=0; k<5; ++k )
        ok = ok && to.GetChild( k ) == &children[ k ] && fabs( children[ k ].GetWorldPosition().y - 7.0f ) <= 1.0e-6f;
    /* a child that would become its own ancestor stays */
    CGeoSpace grandChild;
    grandChild.SetParent( &children[ 0 ], false );
    ok = ok && 4 == to.MoveChildrenTo( &grandChild, false ) && 1 == to.GetChildCount() && to.GetFirstChild() == &children[ 0 ];
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Seconds of a monotonic clock */
static Float64
NowSeconds( void )
{
    return MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() ) / 1000.0;
}

/**
 *  Deep chains and wide trees: a 40 000 deep chain is built in linear time and read without recursion; a change of a
 *  root with 10^5 children of which one is read costs about as much as with one child; destroying a parent of many
 *  children is linear
 */
static bool
DeepChainsAndWideTrees( void )
{
    bool ok = true;
    const UInt32 depth = 40000;
    CGeoSpace64* chain = new CGeoSpace64[ depth ];
    const Float64 t0 = NowSeconds();
    for ( UInt32 k=1; k<depth; ++k )
    {
        chain[ k ].SetLocalPosition( CGeoSpace64::TVector( 0.0001, 0.0, 0.0 ) );
        chain[ k ].SetParent( &chain[ k - 1 ], false );
    }
    const Float64 t1 = NowSeconds();
    const Float64 reach = 0.0001 * (Float64) ( depth - 1 );
    ok = ok && fabs( chain[ depth - 1 ].GetWorldPosition().x - reach ) <= 1.0e-9;
    ok = ok && fabs( chain[ depth - 1 ].ToLocal( CGeoSpace64::TPoint() ).x + reach ) <= 1.0e-9;
    ok = ok && fabs( chain[ depth - 1 ].ToSpacePrecise( chain[ 0 ], CGeoSpace64::TPoint() ).x - reach ) <= 1.0e-9;
    const Float64 t2 = NowSeconds();
    delete [] chain;

    /* wide: one read child of 10^5 against one read child of 1 */
    const UInt32 width = 100000;
    CGeoSpace wideRoot;
    CGeoSpace* kids = new CGeoSpace[ width ];
    for ( UInt32 k=0; k<width; ++k )
        kids[ k ].SetParent( &wideRoot, false );
    wideRoot.SetLocalPosition( CGeoSpace::TVector( 1.0f, 0.0f, 0.0f ) );   /* the first change tells the never read children once */
    kids[ width / 2 ].GetWorldMatrix();
    CGeoSpace narrowRoot;
    CGeoSpace narrowKid;
    narrowKid.SetParent( &narrowRoot, false );
    narrowKid.GetWorldMatrix();
    const UInt32 rounds = 2000;
    const Float64 w0 = NowSeconds();
    for ( UInt32 k=0; k<rounds; ++k )
    {
        wideRoot.SetLocalPosition( CGeoSpace::TVector( (Float32) k, 0.0f, 0.0f ) );
        kids[ width / 2 ].GetWorldMatrix();
    }
    const Float64 w1 = NowSeconds();
    for ( UInt32 k=0; k<rounds; ++k )
    {
        narrowRoot.SetLocalPosition( CGeoSpace::TVector( (Float32) k, 0.0f, 0.0f ) );
        narrowKid.GetWorldMatrix();
    }
    const Float64 w2 = NowSeconds();
    const Float64 wideCost = ( w1 - w0 ) / rounds;
    const Float64 narrowCost = ( w2 - w1 ) / rounds;
    ok = ok && wideCost <= narrowCost * 20.0 + 1.0e-6;

    /* destroying a parent of many children: linear (10^5 against 10^4 children) */
    Float64 destroyCost[ 2 ] = { 0.0, 0.0 };
    const UInt32 counts[ 2 ] = { 10000, 100000 };
    for ( UInt32 run=0; run<2; ++run )
    {
        CGeoSpace* parent = new CGeoSpace( CGeoSpace::TTransform( CGeoSpace::TVector( 1.0f, 2.0f, 3.0f ), CGeoSpace::TQuaternion(), CGeoSpace::TVector( 2.0f, 2.0f, 2.0f ) ) );
        for ( UInt32 k=0; k<counts[ run ]; ++k )
            kids[ k ].SetParent( parent, false );
        const Float64 d0 = NowSeconds();
        delete parent;
        destroyCost[ run ] = NowSeconds() - d0;
    }
    /* kids beyond the first run kept their zero local position: at the parent's position after its destruction */
    ok = ok && destroyCost[ 1 ] <= destroyCost[ 0 ] * 30.0 + 1.0e-3 && fabs( kids[ 50007 ].GetWorldPosition().x - 1.0f ) <= 1.0e-6f;
    delete [] kids;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoSpace: a chain of " << depth << " built in " << ( t1 - t0 ) * 1000.0 << " ms, its leaf read in " << ( t2 - t1 ) * 1000.0
               << " ms; a root change with 1 of " << width << " children read " << wideCost * 1.0e6 << " us (with 1 child " << narrowCost * 1.0e6
               << " us); destroying a parent of 10^4 / 10^5 children " << destroyCost[ 0 ] * 1000.0 << " / " << destroyCost[ 1 ] * 1000.0 << " ms" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  The similarity flag comes from the world matrix: a 60 node Float32 chain whose every node scales y by 1 + 0.95e-5
 *  (each within tolerance) is not a similarity at the end, a chain of uniform scales is
 */
static bool
SimilarityFlagOfLongChains( void )
{
    const UInt32 n = 60;
    CGeoSpace drifting[ 60 ];
    CGeoSpace uniform[ 60 ];
    for ( UInt32 k=0; k<n; ++k )
    {
        drifting[ k ].SetLocalTransform( CGeoSpace::TTransform( CGeoSpace::TVector(), CGeoSpace::TQuaternion::FromAxisAngle( CGeoSpace::TDirection::UnitZ(), CTRadian< Float32 >( 0.7f ) ),
                                                                CGeoSpace::TVector( 1.0f, 1.0000095f, 1.0f ) ) );
        uniform[ k ].SetLocalTransform( CGeoSpace::TTransform( CGeoSpace::TVector(), CGeoSpace::TQuaternion::FromAxisAngle( CGeoSpace::TDirection::UnitZ(), CTRadian< Float32 >( 0.7f ) ),
                                                               CGeoSpace::TVector( 1.01f, 1.01f, 1.01f ) ) );
        if ( k > 0 )
        {
            drifting[ k ].SetParent( &drifting[ k - 1 ], false );
            uniform[ k ].SetParent( &uniform[ k - 1 ], false );
        }
    }
    return drifting[ 0 ].IsWorldTransformSimilarity() && !drifting[ n - 1 ].IsWorldTransformSimilarity() && uniform[ n - 1 ].IsWorldTransformSimilarity();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeoSpaceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeoSpace" );

    GUCEF_TESTFW_TESTCASE( "GeoSpace: children follow: the hand case, three-level chains with non-uniform scales against the explicit products, moving the root moves every descendant" )
    try
    {
        ASSERT_TRUE( HandCaseChildFollows< Float32 >() );
        ASSERT_TRUE( HandCaseChildFollows< Float64 >() );
        ASSERT_TRUE( ThreeLevelChains< Float32 >( 6101, ScaledCount( 1000 ), 1.0e-5 ) );
        ASSERT_TRUE( ThreeLevelChains< Float64 >( 6102, ScaledCount( 1000 ), 1.0e-13 ) );
        ASSERT_TRUE( MovingTheRootMovesDescendants< Float32 >( 6103, ScaledCount( 1000 ), 1.0e-4 ) );
        ASSERT_TRUE( MovingTheRootMovesDescendants< Float64 >( 6104, ScaledCount( 1000 ), 1.0e-11 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpace: re-parenting keeps world matrices (keepWorldPlacement) or local transforms, refuses cycles; destroying a parent or child" )
    try
    {
        ASSERT_TRUE( ReparentKeepsWorld< Float32 >( 6201, 1000, 1.0e-5 ) );
        ASSERT_TRUE( ReparentKeepsWorld< Float64 >( 6202, 1000, 1.0e-12 ) );
        ASSERT_TRUE( ReparentKeepsLocalAndRefusesCycles() );
        ASSERT_TRUE( DestroyingParentKeepsChildrenWorld< Float32 >( 6203, ScaledCount( 500 ), 1.0e-5 ) );
        ASSERT_TRUE( DestroyingParentKeepsChildrenWorld< Float64 >( 6204, ScaledCount( 500 ), 1.0e-12 ) );
        ASSERT_TRUE( DestroyingChildDetaches() );
        ASSERT_TRUE( HierarchyQueries() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpace: caching: updated descendants after a change, no recomputation without one (recompute counter), eager equals lazy" )
    try
    {
        ASSERT_TRUE( CachingAndRecomputeCounter( 6301 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpace: the invalidation hook: the changed node always, descendants once while their caches are invalid" )
    try
    {
        ASSERT_TRUE( InvalidationHook() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpace: notifications reach consumers that read only precisely, never-read nodes, and re-armed nodes" )
    try
    {
        ASSERT_TRUE( NotificationForPreciseReadersAndNeverReadNodes() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpace: Rotate( WORLD ) and LookAt under mirrored and non-uniformly scaled parents: exact or refused; snapshot frame consistent; fixed point tolerates its rounding under similarities, refuses or reports shear" )
    try
    {
        ASSERT_TRUE( RotateAndLookAtUnderScaledParents< Float32 >( 6501, ScaledCount( 800 ), 1.0e-5 ) );
        ASSERT_TRUE( RotateAndLookAtUnderScaledParents< Float64 >( 6502, ScaledCount( 800 ), 1.0e-10 ) );
        ASSERT_TRUE( FixedPointSimilarityTolerance( 6504, ScaledCount( 500 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpace: placements that need shear are reported and keep position, axis lengths and the first axis (re-parent, world placement, destroyed parent)" )
    try
    {
        ASSERT_TRUE( ShearPlacementsReported( 6601, ScaledCount( 1000 ) ) );
        ASSERT_TRUE( MoveChildrenKeepsOrder() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpace: a 40 000 deep chain without recursion, wide trees invalidate only read children, destruction of many children is linear, the similarity flag of long chains" )
    try
    {
        ASSERT_TRUE( DeepChainsAndWideTrees() );
        ASSERT_TRUE( SimilarityFlagOfLongChains() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoSpace: Translate, Rotate, LookAt, SetWorldTransform, SetWorldPosition, world rotation, scale and similarity" )
    try
    {
        ASSERT_TRUE( PlacementMembers< Float32 >( 6401, ScaledCount( 500 ), 1.0e-5 ) );
        ASSERT_TRUE( PlacementMembers< Float64 >( 6402, ScaledCount( 500 ), 1.0e-12 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
