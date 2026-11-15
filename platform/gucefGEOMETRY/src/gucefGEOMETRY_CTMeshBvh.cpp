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

/*
 *  The compiled build of the mesh bounding volume hierarchy (section 7.8.2 of the design): binned SAH, Float32 and
 *  Float64 overloads, the asynchronous build as a task of the task manager, and the exported instantiations of the
 *  mesh and the hierarchy, compiled here without floating-point contraction.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#include "gucefGEOMETRY_CTTriangleMesh.h"
#define GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLEMESH_H ? */

#ifndef GUCEF_GEOMETRY_CTMESHBVH_H
#include "gucefGEOMETRY_CTMeshBvh.h"
#define GUCEF_GEOMETRY_CTMESHBVH_H
#endif /* GUCEF_GEOMETRY_CTMESHBVH_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/*
 *  The builder: ranges of the triangle order are split by the best of the 15 bin boundaries on each of the three axes
 *  (cost: triangle count times half the surface area on each side), into leaves of at most MaxLeafSize triangles. The
 *  ranges are processed from an explicit stack in depth-first order, so the first child of a node is the next node;
 *  the second child's index is written into its parent when that child is created.
 */
template < typename T >
class CTMeshBvhBuilder
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTMeshBvh< T > TBvh;
    typedef typename TBvh::SNode TNode;

    enum { BinCount = TBvh::BinCount, MaxLeafSize = TBvh::MaxLeafSize };

    static void Build( const CTTriangleMesh< T >& mesh, GUCEF::vector< TNode >& nodes, GUCEF::vector< UInt32 >& primitives, UInt32& depth );

    private:

    struct STask
    {
        UInt32 begin;
        UInt32 end;
        UInt32 parent;          /**< the node whose offset becomes this node's index (NoParent for a first child) */
        UInt32 depth;
    };

    static const UInt32 NoParent = 0xFFFFFFFF;

    static T HalfArea( const TBox& box )
    {
        if ( box.IsEmpty() )
            return TTraits::Zero();
        const TVector e = box.max - box.min;
        return ( e.x * e.y + e.y * e.z ) + e.z * e.x;
    }

    static UInt32 BinOf( const T& coordinate, const T& low, const T& scale )
    {
        const T position = ( coordinate - low ) * scale;
        if ( !( TTraits::Zero() < position ) )
            return 0;
        /* checked before the conversion: an infinite or huge position (a scale that overflowed) is not cast */
        const Float64 bin = TTraits::ToFloat64( position );
        if ( !( bin < (Float64) BinCount ) )
            return (UInt32) BinCount - 1;
        return (UInt32) bin;
    }
};

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTMeshBvhBuilder< T >::Build( const CTTriangleMesh< T >& mesh, GUCEF::vector< TNode >& nodes, GUCEF::vector< UInt32 >& primitives, UInt32& depth )
{
    const UInt32 count = mesh.GetTriangleCount();
    nodes.clear();
    primitives.resize( count );
    depth = 0;
    if ( 0 == count )
        return;

    GUCEF::vector< TBox > boxes( count );
    GUCEF::vector< TPoint > centroids( count );
    const T third = TTraits::One() / TTraits::FromInt32( 3 );
    for ( UInt32 t=0; t<count; ++t )
    {
        const CTTriangle3D< T > triangle = mesh.GetTriangle( t );
        boxes[ t ] = triangle.GetBounds();
        centroids[ t ] = TPoint( ( ( triangle.a.ToPositionVector() + triangle.b.ToPositionVector() ) + triangle.c.ToPositionVector() ) * third );
        primitives[ t ] = t;
    }
    nodes.reserve( 2 * (size_t) count );

    GUCEF::vector< STask > tasks;
    STask root;
    root.begin = 0;
    root.end = count;
    root.parent = NoParent;
    root.depth = 1;
    tasks.push_back( root );

    while ( !tasks.empty() )
    {
        const STask task = tasks.back();
        tasks.pop_back();

        const UInt32 index = (UInt32) nodes.size();
        nodes.push_back( TNode() );
        if ( NoParent != task.parent )
            nodes[ task.parent ].offset = index;
        if ( task.depth > depth )
            depth = task.depth;

        TBox bounds;
        TBox centroidBounds;
        for ( UInt32 i=task.begin; i<task.end; ++i )
        {
            bounds = bounds.Merged( boxes[ primitives[ i ] ] );
            centroidBounds = centroidBounds.ExpandedToInclude( centroids[ primitives[ i ] ] );
        }
        nodes[ index ].bounds = bounds;
        const UInt32 rangeCount = task.end - task.begin;
        if ( rangeCount <= (UInt32) MaxLeafSize )
        {
            nodes[ index ].offset = task.begin;
            nodes[ index ].count = rangeCount;
            continue;
        }

        /* the best bin boundary over the three axes; the first one wins a tie */
        bool found = false;
        T bestCost = TTraits::Max();
        UInt32 bestAxis = 0;
        UInt32 bestSplit = 0;
        T bestLow = TTraits::Zero();
        T bestScale = TTraits::Zero();
        for ( UInt32 axis=0; axis<3; ++axis )
        {
            const T low = centroidBounds.min.GetComponent( axis );
            const T extent = centroidBounds.max.GetComponent( axis ) - low;
            if ( !( TTraits::Zero() < extent ) )
                continue;
            const T scale = TTraits::FromInt32( BinCount ) / extent;
            UInt32 binCounts[ BinCount ];
            TBox binBoxes[ BinCount ];
            for ( UInt32 b=0; b<(UInt32) BinCount; ++b )
                binCounts[ b ] = 0;
            for ( UInt32 i=task.begin; i<task.end; ++i )
            {
                const UInt32 p = primitives[ i ];
                const UInt32 bin = BinOf( centroids[ p ].GetComponent( axis ), low, scale );
                ++binCounts[ bin ];
                binBoxes[ bin ] = binBoxes[ bin ].Merged( boxes[ p ] );
            }
            /* sweeps: the cost of the left part up to each boundary, then the right part */
            T leftCost[ BinCount ];
            UInt32 leftCount = 0;
            TBox leftBox;
            for ( UInt32 b=0; b+1<(UInt32) BinCount; ++b )
            {
                leftCount += binCounts[ b ];
                leftBox = leftBox.Merged( binBoxes[ b ] );
                leftCost[ b ] = TTraits::FromInt32( (Int32) leftCount ) * HalfArea( leftBox );
            }
            UInt32 rightCount = 0;
            TBox rightBox;
            for ( UInt32 b=(UInt32) BinCount-1; b>0; --b )
            {
                rightCount += binCounts[ b ];
                rightBox = rightBox.Merged( binBoxes[ b ] );
                const UInt32 leftPart = rangeCount - rightCount;
                if ( 0 == leftPart || 0 == rightCount )
                    continue;
                const T cost = leftCost[ b - 1 ] + TTraits::FromInt32( (Int32) rightCount ) * HalfArea( rightBox );
                if ( !found || cost < bestCost || ( !( bestCost < cost ) && ( axis < bestAxis || ( axis == bestAxis && b < bestSplit ) ) ) )
                {
                    found = true;
                    bestCost = cost;
                    bestAxis = axis;
                    bestSplit = b;
                    bestLow = low;
                    bestScale = scale;
                }
            }
        }

        UInt32 middle = task.begin + rangeCount / 2;
        if ( found )
        {
            /* in-place partition: bins below the boundary to the front, the order within each side kept */
            UInt32 write = task.begin;
            GUCEF::vector< UInt32 > right;
            right.reserve( rangeCount );
            for ( UInt32 i=task.begin; i<task.end; ++i )
            {
                const UInt32 p = primitives[ i ];
                if ( BinOf( centroids[ p ].GetComponent( bestAxis ), bestLow, bestScale ) < bestSplit )
                    primitives[ write++ ] = p;
                else
                    right.push_back( p );
            }
            for ( size_t i=0; i<right.size(); ++i )
                primitives[ write + (UInt32) i ] = right[ i ];
            if ( write != task.begin && write != task.end )
                middle = write;
        }

        nodes[ index ].count = 0;
        nodes[ index ].offset = 0;
        STask second;
        second.begin = middle;
        second.end = task.end;
        second.parent = index;
        second.depth = task.depth + 1;
        STask first;
        first.begin = task.begin;
        first.end = middle;
        first.parent = NoParent;
        first.depth = task.depth + 1;
        tasks.push_back( second );
        tasks.push_back( first );
    }
}

/*-------------------------------------------------------------------------*/

/** The asynchronous build: one cycle that runs the synchronous build on the pool thread */
template < typename T >
class CTMeshBvhBuildTask : public CORE::CTaskConsumer
{
    public:

    CTMeshBvhBuildTask( const CTTriangleMesh< T >& mesh, CTMeshBvh< T >& bvh )
        : CORE::CTaskConsumer(), m_mesh( &mesh ), m_bvh( &bvh )
    {GUCEF_TRACE;
    }

    virtual ~CTMeshBvhBuildTask() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return "GUCEF::GEOMETRY::CMeshBvhBuildTask";
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        static const CORE::CString typeName = "GUCEF::GEOMETRY::CMeshBvhBuildTask";
        return typeName;
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        m_bvh->Build( *m_mesh );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    private:

    const CTTriangleMesh< T >* m_mesh;
    CTMeshBvh< T >* m_bvh;
};

/*-------------------------------------------------------------------------*/

template < typename T >
bool
BuildMeshBvhImpl( const CTTriangleMesh< T >& mesh, CTMeshBvh< T >& bvh, GUCEF::vector< typename CTMeshBvh< T >::SNode >& nodes,
                  GUCEF::vector< UInt32 >& primitives, UInt32& depth )
{
    if ( !mesh.IsValid() )
    {
        bvh.Clear();
        return false;
    }
    CTMeshBvhBuilder< T >::Build( mesh, nodes, primitives, depth );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CORE::CFutureResult
BuildMeshBvhAsyncImpl( const CTTriangleMesh< T >& mesh, CTMeshBvh< T >& bvh, CORE::ThreadPoolPtr threadPool )
{
    /* an invalid mesh fails at once, as Build: the hierarchy is cleared and the future has no future */
    if ( !mesh.IsValid() )
    {
        bvh.Clear();
        return CORE::CFutureResult( CORE::TASKSTATUS_TASKDATA_INVALID );
    }
    CORE::ThreadPoolPtr pool = threadPool;
    if ( pool.IsNULL() )
        pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();
    if ( pool.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_SETUP_FAILED );
    CORE::CTSharedPtr< CTMeshBvhBuildTask< T >, MT::CMutex > task( GUCEF_NEW CTMeshBvhBuildTask< T >( mesh, bvh ) );
    return pool->StartTaskWithConsumer( task );
}

}  /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
BuildMeshBvh( const CTTriangleMesh< Float32 >& mesh, CTMeshBvh< Float32 >& bvh )
{
    if ( !BuildMeshBvhImpl( mesh, bvh, bvh.m_nodes, bvh.m_primitives, bvh.m_depth ) )
        return false;
    bvh.FinishBuild( mesh );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
BuildMeshBvh( const CTTriangleMesh< Float64 >& mesh, CTMeshBvh< Float64 >& bvh )
{
    if ( !BuildMeshBvhImpl( mesh, bvh, bvh.m_nodes, bvh.m_primitives, bvh.m_depth ) )
        return false;
    bvh.FinishBuild( mesh );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
BuildMeshBvhAsync( const CTTriangleMesh< Float32 >& mesh, CTMeshBvh< Float32 >& bvh, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    return BuildMeshBvhAsyncImpl( mesh, bvh, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
BuildMeshBvhAsync( const CTTriangleMesh< Float64 >& mesh, CTMeshBvh< Float64 >& bvh, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    return BuildMeshBvhAsyncImpl( mesh, bvh, threadPool );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANTIATIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTTriangleMesh< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTTriangleMesh< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTMeshBvh< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTMeshBvh< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
