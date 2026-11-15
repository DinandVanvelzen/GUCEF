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

#ifndef GUCEF_GEOMETRY_CTSPATIALITEMSTORE_H
#define GUCEF_GEOMETRY_CTSPATIALITEMSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

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
 *  The items of a spatial structure with their bounds, addressed by a handle (section 7.8.4 of the design): the octree,
 *  quadtree, uniform grids and spatial hash keep their items here and their own placement data in arrays indexed by
 *  the handle. A handle is an index into a record array with a free list, so it stays valid until the item is removed
 *  and is reused afterwards (the most recently released one first: deterministic). Insert, remove and move by handle
 *  match the "moved object" and "removed object" notifications of the geo object bounds invalidation hooks (section
 *  7.4.3 of the design) that the structures will consume.
 *
 *  - Bounds: 'Dim' lower and upper values per item, closed; an item with a lower value above the upper one or a value
 *    that is not finite is refused (InvalidHandle).
 *  - Realtime: no member is realtime-safe (the arrays grow).
 */
template < typename T, typename TItem, UInt32 Dim >
class CTSpatialItemStore
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;

    enum { InvalidHandle = 0xFFFFFFFFU };    /**< an enumerator: no definition is needed in C++98 */

    CTSpatialItemStore( void ) : m_records(), m_firstFree( InvalidHandle ), m_count( 0 ) {}

    /** The number of items */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetCount( void ) const { return m_count; }

    /** One more than the largest handle ever issued: the size of arrays indexed by handle */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetHandleLimit( void ) const { return (UInt32) m_records.size(); }

    GUCEF_GEOMETRY_FORCE_INLINE bool IsValid( UInt32 handle ) const { return handle < m_records.size() && m_records[ handle ].used; }

    GUCEF_GEOMETRY_FORCE_INLINE const TItem& GetItem( UInt32 handle ) const { return m_records[ handle ].item; }
    GUCEF_GEOMETRY_FORCE_INLINE const T* GetLow( UInt32 handle ) const { return m_records[ handle ].lo; }
    GUCEF_GEOMETRY_FORCE_INLINE const T* GetHigh( UInt32 handle ) const { return m_records[ handle ].hi; }

    /** True when the bounds are acceptable: finite and not inverted */
    static bool AreBoundsValid( const T* lo, const T* hi )
    {
        for ( UInt32 a=0; a<Dim; ++a )
            if ( !TTraits::IsFinite( lo[ a ] ) || !TTraits::IsFinite( hi[ a ] ) || hi[ a ] < lo[ a ] )
                return false;
        return true;
    }

    /** Adds an item; InvalidHandle when the bounds are not valid */
    UInt32 Allocate( const TItem& item, const T* lo, const T* hi )
    {
        if ( !AreBoundsValid( lo, hi ) )
            return InvalidHandle;
        UInt32 handle;
        if ( InvalidHandle != m_firstFree )
        {
            handle = m_firstFree;
            m_firstFree = m_records[ handle ].nextFree;
        }
        else
        {
            handle = (UInt32) m_records.size();
            m_records.push_back( SRecord() );
        }
        SRecord& record = m_records[ handle ];
        record.item = item;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            record.lo[ a ] = lo[ a ];
            record.hi[ a ] = hi[ a ];
        }
        record.used = true;
        record.nextFree = InvalidHandle;
        ++m_count;
        return handle;
    }

    /** Replaces the bounds of an item; false (unchanged) for an invalid handle or invalid bounds */
    bool SetBounds( UInt32 handle, const T* lo, const T* hi )
    {
        if ( !IsValid( handle ) || !AreBoundsValid( lo, hi ) )
            return false;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            m_records[ handle ].lo[ a ] = lo[ a ];
            m_records[ handle ].hi[ a ] = hi[ a ];
        }
        return true;
    }

    /** Removes an item; false for an invalid handle */
    bool Release( UInt32 handle )
    {
        if ( !IsValid( handle ) )
            return false;
        m_records[ handle ].used = false;
        m_records[ handle ].nextFree = m_firstFree;
        m_firstFree = handle;
        --m_count;
        return true;
    }

    void Clear( void )
    {
        m_records.clear();
        m_firstFree = InvalidHandle;
        m_count = 0;
    }

    private:

    struct SRecord
    {
        TItem item;
        T lo[ Dim ];
        T hi[ Dim ];
        bool used;
        UInt32 nextFree;

        SRecord( void ) : item(), used( false ), nextFree( InvalidHandle )
        {
            for ( UInt32 a=0; a<Dim; ++a )
                lo[ a ] = hi[ a ] = T( 0 );
        }
    };

    GUCEF::vector< SRecord > m_records;
    UInt32 m_firstFree;
    UInt32 m_count;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTSPATIALITEMSTORE_H ? */
