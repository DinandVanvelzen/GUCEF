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

#ifndef GUCEF_GEOMETRY_CTGEOSPACESNAPSHOTSET_H
#define GUCEF_GEOMETRY_CTGEOSPACESNAPSHOTSET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTGEOSPACE_H
#include "gucefGEOMETRY_CTGeoSpace.h"
#define GUCEF_GEOMETRY_CTGEOSPACE_H
#endif /* GUCEF_GEOMETRY_CTGEOSPACE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Snapshots of many CTGeoSpace nodes captured in one pass for bulk publication (section 7.3.6 of the design).
 *
 *  - The storage is sized by the caller (SetCapacity, the only member that allocates); Capture never allocates and
 *    fails when given more nodes than the capacity. CaptureInto writes into a caller-owned array instead.
 *  - Captured on the owner thread of the nodes; the set is then published as a value, for example through
 *    MT::CTSlotDataExchange, which copies values in and out by assignment. The set owns its storage (a vector of
 *    GetCapacity() snapshots; a copy copies the full capacity), so it is not the plain value that the exchange
 *    assumes: a copy into a set of smaller capacity allocates. Give every set the same capacity; once each slot
 *    of the exchange and the reader's set have received one set (the warm-up: the first three writes and the
 *    first read of the latest-only exchange, one write per slot of the in-order exchange) a copy reuses the
 *    storage of its destination and publish and read do not allocate (tested). The slots of the exchange start
 *    empty, so the warm-up does allocate; publish single CTGeoSpaceSnapshot values where that is not acceptable.
 *  - Kinds: Float32, Float64 and TFixed32_32 are exported.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoSpaceSnapshotSet
{
    public:

    typedef CTGeoSpace< T > TGeoSpace;
    typedef CTGeoSpaceSnapshot< T > TSnapshot;
    typedef MATH::CTVector3D< T > TVector;

    CTGeoSpaceSnapshotSet( void ) : m_snapshots(), m_count( 0 ) {}

    /** Room for 'capacity' snapshots */
    explicit CTGeoSpaceSnapshotSet( UInt32 capacity ) : m_snapshots( capacity ), m_count( 0 ) {}

    /** Resizes the storage (allocates); the captured count is clamped to the new capacity */
    void SetCapacity( UInt32 capacity )
    {
        m_snapshots.resize( capacity );
        if ( m_count > capacity )
            m_count = capacity;
    }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetCapacity( void ) const { return (UInt32) m_snapshots.size(); }

    /** The number of snapshots of the last capture */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetCount( void ) const { return m_count; }

    /** Snapshot 'index' of the last capture (below GetCount) */
    GUCEF_GEOMETRY_FORCE_INLINE const TSnapshot& GetSnapshot( UInt32 index ) const { return m_snapshots[ index ]; }

    /** The snapshots of the last capture as a contiguous array (NULL when the capacity is 0) */
    GUCEF_GEOMETRY_FORCE_INLINE const TSnapshot* GetData( void ) const { return m_snapshots.empty() ? GUCEF_NULL : &m_snapshots[ 0 ]; }

    /**
     *  Captures nodes[ 0 .. count ) with the axes of 'system'; velocities (NULL for zero) gives one per node. False,
     *  nothing captured, when count exceeds the capacity or a node pointer is NULL.
     */
    bool Capture( const TGeoSpace* const* nodes, UInt32 count, const MATH::CCoordinateSystem& system, const TVector* velocities )
    {
        if ( count > GetCapacity() || !CaptureInto( nodes, count, system, velocities, count > 0 ? &m_snapshots[ 0 ] : GUCEF_NULL ) )
            return false;
        m_count = count;
        return true;
    }

    /** As Capture into a caller-owned array of at least 'count' snapshots; false for a NULL node or a NULL array with count > 0 */
    static bool CaptureInto( const TGeoSpace* const* nodes, UInt32 count, const MATH::CCoordinateSystem& system, const TVector* velocities, TSnapshot* out )
    {
        if ( count > 0 && ( GUCEF_NULL == nodes || GUCEF_NULL == out ) )
            return false;
        for ( UInt32 i=0; i<count; ++i )
            if ( GUCEF_NULL == nodes[ i ] )
                return false;
        for ( UInt32 i=0; i<count; ++i )
        {
            if ( GUCEF_NULL != velocities )
                nodes[ i ]->CaptureSnapshot( out[ i ], system, velocities[ i ] );
            else
                nodes[ i ]->CaptureSnapshot( out[ i ], system );
        }
        return true;
    }

    private:

    GUCEF::vector< TSnapshot > m_snapshots;
    UInt32 m_count;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoSpaceSnapshotSet< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoSpaceSnapshotSet< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoSpaceSnapshotSet< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOSPACESNAPSHOTSET_H ? */
