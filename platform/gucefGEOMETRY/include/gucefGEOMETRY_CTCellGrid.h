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

#ifndef GUCEF_GEOMETRY_CTCELLGRID_H
#define GUCEF_GEOMETRY_CTCELLGRID_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_MATH_BITFUNCTIONS_H
#include "gucefMATH_BitFunctions.h"
#define GUCEF_MATH_BITFUNCTIONS_H
#endif /* GUCEF_MATH_BITFUNCTIONS_H ? */

#ifndef GUCEF_GEOMETRY_CTSPATIALITEMSTORE_H
#include "gucefGEOMETRY_CTSpatialItemStore.h"
#define GUCEF_GEOMETRY_CTSPATIALITEMSTORE_H
#endif /* GUCEF_GEOMETRY_CTSPATIALITEMSTORE_H ? */

#ifndef GUCEF_GEOMETRY_CTGRIDTRAVERSALCORE_H
#include "gucefGEOMETRY_CTGridTraversalCore.h"
#define GUCEF_GEOMETRY_CTGRIDTRAVERSALCORE_H
#endif /* GUCEF_GEOMETRY_CTGRIDTRAVERSALCORE_H ? */

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
 *  The cell lists behind CTUniformGrid2D, CTUniformGrid3D (Hashed false: a dense array of cells over a bounded area) and
 *  CTSpatialHash (Hashed true: unbounded, the cells in an open addressing table keyed by the hash of the cell
 *  coordinates), section 7.8.4 of the design. An item is entered in every cell its bounds touch; a cell holds its
 *  items as a list in a pooled entry array. Cell i of an axis covers [ origin + i * size, origin + ( i + 1 ) * size ).
 *
 *  - Dense grid: an item or query outside the area is clamped into the border cells, so every item can be stored and
 *    found; the ray query clips the ray to the area and finds items inside it.
 *  - Hashed grid: cell coordinates are Int32 clamped to +-2^30; a table slot is kept for an emptied cell until the
 *    next rehash. The table doubles at half load.
 *  - Query: a region (SpatialQueryRegions.h) visits all cells of the box of cells that its bounds cover (the table
 *    slots, when that box has more cells than the table has slots) and reports an item once, in the cell where the
 *    cell box of the item meets the cell box of the query in its lowest corner, so no mark or set is needed and the
 *    query is const; items are accepted by the exact test of the region. Order: cells with x fastest, entries of a cell
 *    newest first; for the hashed table in slot order. An item that covers more than MaxCellsPerItem cells is refused.
 *  - Ray query: the cells along the ray (supercover traversal, ray segments cut to a maximum distance), each item
 *    once at the first cell of the ray in its cell box, tested exactly against the ray (slab test on its bounds).
 *    The visitor gets 'Visit( UInt32 handle, const TItem& item, const T& enterDistance )' where the distance is where
 *    the ray enters the bounds of the item.
 *  - Kinds: float kinds. One owner thread. Query is const and allocation free; QueryRay marks items in a mutable array
 *    (it grows to the number of items), so two QueryRay calls must not run at the same time.
 */
template < typename T, typename TItem, UInt32 Dim, bool Hashed >
class CTCellGrid
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;
    typedef CTSpatialItemStore< T, TItem, Dim > TStore;

    enum { InvalidHandle = TStore::InvalidHandle };    /**< an enumerator: no definition is needed in C++98 */
    enum { MaxCellsPerItem = 1 << 20 };    /**< an enumerator: no definition is needed in C++98 */

    CTCellGrid( void ) : m_store(), m_itemCells(), m_entries(), m_firstFreeEntry( InvalidHandle ), m_heads(), m_slots(), m_usedSlots( 0 ), m_initialized( false ), m_stamps(), m_stampCounter( 0 )
    {
        for ( UInt32 a=0; a<Dim; ++a )
        {
            m_origin[ a ] = T( 0 );
            m_size[ a ] = T( 1 );
            m_counts[ a ] = 1;
            m_strides[ a ] = 1;
        }
    }

    /**
     *  Sets the origin and the cell size per axis and, for the dense grid, the number of cells per axis ('counts', ignored
     *  by the hashed grid), and removes every item. False for a cell size that is not positive and finite, an origin that
     *  is not finite or a dense grid of no cells or of more than 2^28 cells.
     */
    bool Initialize( const T* origin, const T* cellSize, const UInt32* counts )
    {
        UInt64 total = 1;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            if ( !TTraits::IsFinite( origin[ a ] ) || !TTraits::IsFinite( cellSize[ a ] ) || !( cellSize[ a ] > TTraits::Zero() ) )
                return false;
            if ( !Hashed )
            {
                if ( 0 == counts[ a ] )
                    return false;
                total *= counts[ a ];
                if ( total > ( (UInt64) 1 << 28 ) )
                    return false;
            }
        }
        m_initialized = false;
        ClearAll();
        UInt32 stride = 1;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            m_origin[ a ] = origin[ a ];
            m_size[ a ] = cellSize[ a ];
            m_counts[ a ] = Hashed ? 0 : counts[ a ];
            m_strides[ a ] = stride;
            stride *= m_counts[ a ];
        }
        if ( Hashed )
            m_slots.resize( 64 );
        else
            m_heads.assign( (size_t) total, InvalidHandle );
        m_initialized = true;
        return true;
    }

    /** Removes every item (the area stays) */
    void Clear( void )
    {
        m_store.Clear();
        m_itemCells.clear();
        m_entries.clear();
        m_firstFreeEntry = InvalidHandle;
        m_usedSlots = 0;
        if ( Hashed )
        {
            m_slots.clear();
            if ( m_initialized )
                m_slots.resize( 64 );
        }
        else
            m_heads.assign( m_heads.size(), InvalidHandle );
    }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetCount( void ) const { return m_store.GetCount(); }
    GUCEF_GEOMETRY_FORCE_INLINE bool IsValid( UInt32 handle ) const { return m_store.IsValid( handle ); }
    GUCEF_GEOMETRY_FORCE_INLINE const TItem& GetItem( UInt32 handle ) const { return m_store.GetItem( handle ); }
    GUCEF_GEOMETRY_FORCE_INLINE const T* GetLow( UInt32 handle ) const { return m_store.GetLow( handle ); }
    GUCEF_GEOMETRY_FORCE_INLINE const T* GetHigh( UInt32 handle ) const { return m_store.GetHigh( handle ); }

    /** The number of cells that hold at least one item */
    UInt32 GetOccupiedCellCount( void ) const
    {
        UInt32 occupied = 0;
        if ( Hashed )
        {
            for ( size_t i=0; i<m_slots.size(); ++i )
                if ( m_slots[ i ].used && InvalidHandle != m_slots[ i ].head )
                    ++occupied;
        }
        else
        {
            for ( size_t i=0; i<m_heads.size(); ++i )
                if ( InvalidHandle != m_heads[ i ] )
                    ++occupied;
        }
        return occupied;
    }

    UInt32 Insert( const TItem& item, const T* lo, const T* hi )
    {
        if ( !m_initialized || !TStore::AreBoundsValid( lo, hi ) )
            return InvalidHandle;
        SItemCells cells;
        if ( !ComputeCells( lo, hi, cells ) )
            return InvalidHandle;
        const UInt32 handle = m_store.Allocate( item, lo, hi );
        if ( handle >= m_itemCells.size() )
            m_itemCells.resize( handle + 1 );
        m_itemCells[ handle ] = cells;
        AddToCells( handle );
        return handle;
    }

    bool Remove( UInt32 handle )
    {
        if ( !m_store.IsValid( handle ) )
            return false;
        RemoveFromCells( handle );
        return m_store.Release( handle );
    }

    bool Move( UInt32 handle, const T* lo, const T* hi )
    {
        if ( !m_store.IsValid( handle ) || !TStore::AreBoundsValid( lo, hi ) )
            return false;
        SItemCells cells;
        if ( !ComputeCells( lo, hi, cells ) )
            return false;
        RemoveFromCells( handle );
        m_store.SetBounds( handle, lo, hi );
        m_itemCells[ handle ] = cells;
        AddToCells( handle );
        return true;
    }

    /** Visits the items that 'region' accepts (see the class description): 'bool Visit( UInt32 handle, const TItem& item )' */
    template < typename TRegion, typename TVisitor >
    void Query( const TRegion& region, TVisitor& visitor ) const
    {
        if ( !m_initialized )
            return;
        T lo[ Dim ];
        T hi[ Dim ];
        region.GetBounds( lo, hi );
        SItemCells query;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            query.lo[ a ] = CellOf( a, lo[ a ] );
            query.hi[ a ] = CellOf( a, hi[ a ] );
        }
        if ( Hashed )
        {
            // the cells of the query, or when there are more of them than slots in the table, the slots
            Float64 cellCount = 1.0;
            for ( UInt32 a=0; a<Dim; ++a )
                cellCount *= (Float64) ( (Int64) query.hi[ a ] - (Int64) query.lo[ a ] + 1 );
            if ( cellCount > (Float64) m_slots.size() )
            {
                for ( size_t i=0; i<m_slots.size(); ++i )
                {
                    const SSlot& slot = m_slots[ i ];
                    if ( !slot.used || InvalidHandle == slot.head )
                        continue;
                    bool inside = true;
                    for ( UInt32 a=0; a<Dim; ++a )
                        if ( slot.key[ a ] < query.lo[ a ] || slot.key[ a ] > query.hi[ a ] )
                            inside = false;
                    if ( inside && !VisitCell( slot.key, slot.head, query, region, visitor ) )
                        return;
                }
                return;
            }
        }
        Int32 cell[ Dim ];
        for ( UInt32 a=0; a<Dim; ++a )
            cell[ a ] = query.lo[ a ];
        do
        {
            const UInt32 head = Hashed ? FindHashedHead( cell ) : m_heads[ DenseIndex( cell ) ];
            if ( InvalidHandle != head && !VisitCell( cell, head, query, region, visitor ) )
                return;
        }
        while ( NextCell( cell, query.lo, query.hi ) );
    }

    /**
     *  Visits the items whose bounds the ray from 'origin' along 'direction' touches within the parameter range 0 to
     *  'maxDistance' (the distance when 'direction' has length 1), along the ray, each item once: 'bool Visit( UInt32
     *  handle, const TItem& item, const T& enterParameter )' returns false to stop. 'maxDistance' must be finite.
     */
    template < typename TVisitor >
    void QueryRay( const T* origin, const T* direction, const T& maxDistance, TVisitor& visitor ) const
    {
        if ( !m_initialized || !( maxDistance >= TTraits::Zero() ) || !TTraits::IsFinite( maxDistance ) )
            return;
        T start[ Dim ];
        for ( UInt32 a=0; a<Dim; ++a )
        {
            if ( !TTraits::IsFinite( origin[ a ] ) || !TTraits::IsFinite( direction[ a ] ) )
                return;
            start[ a ] = origin[ a ];
        }
        T tStart = TTraits::Zero();
        T tEnd = maxDistance;
        CTGridTraversalCore< T, Dim > traversal;
        if ( !Hashed )
        {
            // clip to the area of the grid
            for ( UInt32 a=0; a<Dim; ++a )
            {
                const T areaLo = m_origin[ a ];
                const T areaHi = m_origin[ a ] + (T) m_counts[ a ] * m_size[ a ];
                if ( direction[ a ] == TTraits::Zero() )
                {
                    if ( origin[ a ] < areaLo || origin[ a ] > areaHi )
                        return;
                }
                else
                {
                    T t1 = ( areaLo - origin[ a ] ) / direction[ a ];
                    T t2 = ( areaHi - origin[ a ] ) / direction[ a ];
                    if ( t2 < t1 )
                    {
                        const T swap = t1; t1 = t2; t2 = swap;
                    }
                    if ( t1 > tStart ) tStart = t1;
                    if ( t2 < tEnd ) tEnd = t2;
                }
            }
            if ( tStart > tEnd )
                return;
            for ( UInt32 a=0; a<Dim; ++a )
                start[ a ] = origin[ a ] + direction[ a ] * tStart;
            Int32 cellMin[ Dim ];
            Int32 cellMax[ Dim ];
            for ( UInt32 a=0; a<Dim; ++a )
            {
                cellMin[ a ] = 0;
                cellMax[ a ] = (Int32) m_counts[ a ] - 1;
            }
            traversal.SetCellBounds( cellMin, cellMax );
        }
        // the traversal is clamped to the area: a start point on the far edge belongs to the last cell
        T gridDelta[ Dim ];
        for ( UInt32 a=0; a<Dim; ++a )
            gridDelta[ a ] = direction[ a ];
        if ( !Hashed )
        {
            for ( UInt32 a=0; a<Dim; ++a )
            {
                const T areaHi = m_origin[ a ] + (T) m_counts[ a ] * m_size[ a ];
                if ( start[ a ] > areaHi ) start[ a ] = areaHi;
                if ( start[ a ] < m_origin[ a ] ) start[ a ] = m_origin[ a ];
            }
        }
        if ( !traversal.Start( m_origin, m_size, start, gridDelta, tEnd - tStart, GRIDTRAVERSAL_SUPERCOVER ) )
            return;
        Int32 cell[ Dim ];
        T enter;
        // a stamp per item marks it as reported in this query (the supercover order is not contiguous at corners)
        if ( m_stamps.size() < m_itemCells.size() )
            m_stamps.resize( m_itemCells.size(), 0 );
        if ( 0xFFFFFFFFU == ++m_stampCounter )
        {
            m_stamps.assign( m_stamps.size(), 0 );
            m_stampCounter = 1;
        }
        const UInt32 stamp = m_stampCounter;
        while ( traversal.Next( cell, &enter ) )
        {
            const UInt32 head = Hashed ? FindHashedHead( cell ) : m_heads[ DenseIndex( cell ) ];
            for ( UInt32 e=head; InvalidHandle!=e; e=m_entries[ e ].next )
            {
                const UInt32 handle = m_entries[ e ].item;
                if ( stamp == m_stamps[ handle ] )
                    continue;
                T t0, t1;
                if ( !RayBox( origin, direction, maxDistance, m_store.GetLow( handle ), m_store.GetHigh( handle ), t0, t1 ) )
                    continue;
                m_stamps[ handle ] = stamp;
                if ( !visitor.Visit( handle, m_store.GetItem( handle ), t0 ) )
                    return;
            }
        }
    }

    /** Checks that every item is in exactly the cells of its stored range and nowhere else */
    bool Validate( void ) const
    {
        UInt64 expected = 0;
        for ( UInt32 h=0; h<m_store.GetHandleLimit(); ++h )
        {
            if ( !m_store.IsValid( h ) )
                continue;
            const SItemCells& range = m_itemCells[ h ];
            Int32 cell[ Dim ];
            for ( UInt32 a=0; a<Dim; ++a )
                cell[ a ] = range.lo[ a ];
            do
            {
                const UInt32 head = Hashed ? FindHashedHead( cell ) : m_heads[ DenseIndex( cell ) ];
                UInt32 found = 0;
                for ( UInt32 e=head; InvalidHandle!=e; e=m_entries[ e ].next )
                    if ( m_entries[ e ].item == h )
                        ++found;
                if ( 1 != found )
                    return false;
                ++expected;
            }
            while ( NextCell( cell, range.lo, range.hi ) );
        }
        UInt64 total = 0;
        if ( Hashed )
        {
            for ( size_t i=0; i<m_slots.size(); ++i )
                if ( m_slots[ i ].used )
                    for ( UInt32 e=m_slots[ i ].head; InvalidHandle!=e; e=m_entries[ e ].next )
                        ++total;
        }
        else
        {
            for ( size_t i=0; i<m_heads.size(); ++i )
                for ( UInt32 e=m_heads[ i ]; InvalidHandle!=e; e=m_entries[ e ].next )
                    ++total;
        }
        return total == expected;
    }

    private:

    struct SItemCells
    {
        Int32 lo[ Dim ];
        Int32 hi[ Dim ];
    };

    struct SEntry
    {
        UInt32 item;
        UInt32 next;
    };

    struct SSlot
    {
        Int32 key[ Dim ];
        UInt32 head;
        bool used;

        SSlot( void ) : head( InvalidHandle ), used( false )
        {
            for ( UInt32 a=0; a<Dim; ++a )
                key[ a ] = 0;
        }
    };

    void ClearAll( void )
    {
        m_store.Clear();
        m_itemCells.clear();
        m_entries.clear();
        m_heads.clear();
        m_slots.clear();
        m_firstFreeEntry = InvalidHandle;
        m_usedSlots = 0;
    }

    /** The cell index of a coordinate: the floor of its cell position, clamped to the grid (dense) or to +-2^30 */
    Int32 CellOf( UInt32 axis, const T& value ) const
    {
        const T f = TTraits::Floor( ( value - m_origin[ axis ] ) / m_size[ axis ] );
        const T low = Hashed ? (T) -1073741824.0 : TTraits::Zero();
        const T high = Hashed ? (T) 1073741824.0 : (T) ( (Int32) m_counts[ axis ] - 1 );
        if ( !( f > low ) )
            return (Int32) low;
        if ( !( f < high ) )
            return (Int32) high;
        return (Int32) f;
    }

    bool ComputeCells( const T* lo, const T* hi, SItemCells& cells ) const
    {
        Float64 count = 1.0;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            cells.lo[ a ] = CellOf( a, lo[ a ] );
            cells.hi[ a ] = CellOf( a, hi[ a ] );
            count *= (Float64) ( (Int64) cells.hi[ a ] - (Int64) cells.lo[ a ] + 1 );
        }
        return count <= (Float64) MaxCellsPerItem;
    }

    static bool NextCell( Int32* cell, const Int32* lo, const Int32* hi )
    {
        for ( UInt32 a=0; a<Dim; ++a )
        {
            if ( cell[ a ] < hi[ a ] )
            {
                ++cell[ a ];
                return true;
            }
            cell[ a ] = lo[ a ];
        }
        return false;
    }

    UInt32 DenseIndex( const Int32* cell ) const
    {
        UInt32 index = 0;
        for ( UInt32 a=0; a<Dim; ++a )
            index += (UInt32) cell[ a ] * m_strides[ a ];
        return index;
    }

    static UInt64 HashCell( const Int32* cell )
    {
        UInt64 h = MATH::HashUInt64( (UInt64) (UInt32) cell[ 0 ] | ( (UInt64) (UInt32) cell[ 1 ] << 32 ) );
        if ( Dim > 2 )
            h = MATH::HashCombine( h, (UInt64) (UInt32) cell[ 2 ] );
        return h;
    }

    static bool SameKey( const Int32* a, const Int32* b )
    {
        for ( UInt32 i=0; i<Dim; ++i )
            if ( a[ i ] != b[ i ] )
                return false;
        return true;
    }

    /** The head of the cell list of a hashed cell, InvalidHandle when the cell is not in the table */
    UInt32 FindHashedHead( const Int32* cell ) const
    {
        const size_t mask = m_slots.size() - 1;
        for ( size_t i=(size_t) HashCell( cell ) & mask; ; i=( i + 1 ) & mask )
        {
            if ( !m_slots[ i ].used )
                return InvalidHandle;
            if ( SameKey( m_slots[ i ].key, cell ) )
                return m_slots[ i ].head;
        }
    }

    /** The head of the list of a cell for change: the cell is created in the hashed table when it is missing */
    UInt32& HeadFor( const Int32* cell )
    {
        if ( !Hashed )
            return m_heads[ DenseIndex( cell ) ];
        if ( ( m_usedSlots + 1 ) * 2 > m_slots.size() )
            Rehash();
        const size_t mask = m_slots.size() - 1;
        for ( size_t i=(size_t) HashCell( cell ) & mask; ; i=( i + 1 ) & mask )
        {
            if ( !m_slots[ i ].used )
            {
                m_slots[ i ].used = true;
                for ( UInt32 a=0; a<Dim; ++a )
                    m_slots[ i ].key[ a ] = cell[ a ];
                m_slots[ i ].head = InvalidHandle;
                ++m_usedSlots;
                return m_slots[ i ].head;
            }
            if ( SameKey( m_slots[ i ].key, cell ) )
                return m_slots[ i ].head;
        }
    }

    /** Rebuilds the table without the emptied cells; it doubles when more than a quarter of the slots hold items */
    void Rehash( void )
    {
        GUCEF::vector< SSlot > old;
        old.swap( m_slots );
        UInt32 live = 0;
        for ( size_t i=0; i<old.size(); ++i )
            if ( old[ i ].used && InvalidHandle != old[ i ].head )
                ++live;
        size_t capacity = old.size();
        while ( (size_t) live * 4 > capacity )
            capacity *= 2;
        m_slots.assign( capacity, SSlot() );
        m_usedSlots = 0;
        const size_t mask = capacity - 1;
        for ( size_t i=0; i<old.size(); ++i )
        {
            if ( !old[ i ].used || InvalidHandle == old[ i ].head )
                continue;
            size_t j = (size_t) HashCell( old[ i ].key ) & mask;
            while ( m_slots[ j ].used )
                j = ( j + 1 ) & mask;
            m_slots[ j ] = old[ i ];
            ++m_usedSlots;
        }
    }

    void AddToCells( UInt32 handle )
    {
        const SItemCells range = m_itemCells[ handle ];
        Int32 cell[ Dim ];
        for ( UInt32 a=0; a<Dim; ++a )
            cell[ a ] = range.lo[ a ];
        do
        {
            UInt32 entry;
            if ( InvalidHandle != m_firstFreeEntry )
            {
                entry = m_firstFreeEntry;
                m_firstFreeEntry = m_entries[ entry ].next;
            }
            else
            {
                entry = (UInt32) m_entries.size();
                m_entries.push_back( SEntry() );
            }
            UInt32& head = HeadFor( cell );
            m_entries[ entry ].item = handle;
            m_entries[ entry ].next = head;
            head = entry;
        }
        while ( NextCell( cell, range.lo, range.hi ) );
    }

    void RemoveFromCells( UInt32 handle )
    {
        const SItemCells range = m_itemCells[ handle ];
        Int32 cell[ Dim ];
        for ( UInt32 a=0; a<Dim; ++a )
            cell[ a ] = range.lo[ a ];
        do
        {
            UInt32& head = HeadFor( cell );
            UInt32* link = &head;
            while ( InvalidHandle != *link && m_entries[ *link ].item != handle )
                link = &m_entries[ *link ].next;
            if ( InvalidHandle != *link )
            {
                const UInt32 entry = *link;
                *link = m_entries[ entry ].next;
                m_entries[ entry ].next = m_firstFreeEntry;
                m_firstFreeEntry = entry;
            }
        }
        while ( NextCell( cell, range.lo, range.hi ) );
    }

    template < typename TRegion, typename TVisitor >
    bool VisitCell( const Int32* cell, UInt32 head, const SItemCells& query, const TRegion& region, TVisitor& visitor ) const
    {
        // no test of the cell against the region: an item is reported in the corner cell of the overlap of its cells and the
        // cells of the query, which the region itself need not touch (a ball), so skipping such a cell would lose the item
        for ( UInt32 e=head; InvalidHandle!=e; e=m_entries[ e ].next )
        {
            const UInt32 handle = m_entries[ e ].item;
            const SItemCells& range = m_itemCells[ handle ];
            // the item is reported where its cell box meets the cell box of the query in the lowest corner
            bool reference = true;
            for ( UInt32 a=0; a<Dim; ++a )
            {
                const Int32 corner = range.lo[ a ] > query.lo[ a ] ? range.lo[ a ] : query.lo[ a ];
                if ( corner != cell[ a ] )
                    reference = false;
            }
            if ( !reference || !region.Accepts( m_store.GetLow( handle ), m_store.GetHigh( handle ) ) )
                continue;
            if ( !visitor.Visit( handle, m_store.GetItem( handle ) ) )
                return false;
        }
        return true;
    }

    /** The slab test of the ray against a closed box: true with the parameters of entering and leaving when it touches the box within 0 to maxDistance */
    static bool RayBox( const T* origin, const T* direction, const T& maxDistance, const T* lo, const T* hi, T& tEnter, T& tLeave )
    {
        tEnter = TTraits::Zero();
        tLeave = maxDistance;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            if ( direction[ a ] == TTraits::Zero() )
            {
                if ( origin[ a ] < lo[ a ] || origin[ a ] > hi[ a ] )
                    return false;
                continue;
            }
            T t1 = ( lo[ a ] - origin[ a ] ) / direction[ a ];
            T t2 = ( hi[ a ] - origin[ a ] ) / direction[ a ];
            if ( t2 < t1 )
            {
                const T swap = t1; t1 = t2; t2 = swap;
            }
            if ( t1 > tEnter ) tEnter = t1;
            if ( t2 < tLeave ) tLeave = t2;
            if ( tEnter > tLeave )
                return false;
        }
        return true;
    }

    TStore m_store;
    GUCEF::vector< SItemCells > m_itemCells;
    GUCEF::vector< SEntry > m_entries;
    UInt32 m_firstFreeEntry;
    GUCEF::vector< UInt32 > m_heads;        /**< dense: the list head per cell */
    GUCEF::vector< SSlot > m_slots;         /**< hashed: the table, a power of two in size */
    UInt32 m_usedSlots;
    bool m_initialized;
    mutable GUCEF::vector< UInt32 > m_stamps;   /**< QueryRay: the query number in which an item was reported */
    mutable UInt32 m_stampCounter;
    T m_origin[ Dim ];
    T m_size[ Dim ];
    UInt32 m_counts[ Dim ];
    UInt32 m_strides[ Dim ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTCELLGRID_H ? */
