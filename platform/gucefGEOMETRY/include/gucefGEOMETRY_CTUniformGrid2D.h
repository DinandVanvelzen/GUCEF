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

#ifndef GUCEF_GEOMETRY_CTUNIFORMGRID2D_H
#define GUCEF_GEOMETRY_CTUNIFORMGRID2D_H

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTCELLGRID_H
#include "gucefGEOMETRY_CTCellGrid.h"
#define GUCEF_GEOMETRY_CTCELLGRID_H
#endif /* GUCEF_GEOMETRY_CTCELLGRID_H ? */

#ifndef GUCEF_GEOMETRY_SPATIALQUERYREGIONS_H
#include "gucefGEOMETRY_SpatialQueryRegions.h"
#define GUCEF_GEOMETRY_SPATIALQUERYREGIONS_H
#endif /* GUCEF_GEOMETRY_SPATIALQUERYREGIONS_H ? */

#ifndef GUCEF_GEOMETRY_CTRECTANGLE_H
#include "gucefGEOMETRY_CTRectangle.h"
#define GUCEF_GEOMETRY_CTRECTANGLE_H
#endif /* GUCEF_GEOMETRY_CTRECTANGLE_H ? */

#ifndef GUCEF_GEOMETRY_CTCIRCLE_H
#include "gucefGEOMETRY_CTCircle.h"
#define GUCEF_GEOMETRY_CTCIRCLE_H
#endif /* GUCEF_GEOMETRY_CTCIRCLE_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY2D_H
#include "gucefGEOMETRY_CTRay2D.h"
#define GUCEF_GEOMETRY_CTRAY2D_H
#endif /* GUCEF_GEOMETRY_CTRAY2D_H ? */

namespace GUCEF {
namespace GEOMETRY {

/**
 *  A bounded dense grid of cell lists over a rectangle (section 7.8.4 of the design): items with CTRectangle bounds,
 *  inserted into every cell they touch; see CTCellGrid for the cell rules, the exactness of the queries against a scan of
 *  all items, the visitor forms and the ray query (7.8.6). Items outside the area are clamped into its border cells.
 *
 *  - Kinds: Float32 and Float64. Items with bounds that are not finite or inverted, or that touch more than
 *    MaxCellsPerItem cells, are refused (InvalidHandle). One owner thread.
 */
template < typename T, typename TItem >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTUniformGrid2D
{
    public:

    typedef T TValue;
    typedef CTRectangle< T > TBox;
    typedef CTCircle< T > TBall;
    typedef CTCellGrid< T, TItem, 2, false > TGrid;

    enum { InvalidHandle = TGrid::InvalidHandle };    /**< an enumerator: no definition is needed in C++98 */

    /** Sets the cell origin and size and the number of cells per axis; removes every item. False for invalid arguments */
    bool Initialize( const MATH::CTPoint2D< T >& origin, const MATH::CTVector2D< T >& cellSize, UInt32 cellsX, UInt32 cellsY )
    {
        const T o[ 2 ] = { origin.x, origin.y };
        const T s[ 2 ] = { cellSize.x, cellSize.y };
        const UInt32 counts[ 2 ] = { cellsX, cellsY };
        return m_grid.Initialize( o, s, counts );
    }

    void Clear( void ) { m_grid.Clear(); }

    UInt32 Insert( const TItem& item, const TBox& bounds )
    {
        T lo[ 2 ], hi[ 2 ];
        ToArrays( bounds, lo, hi );
        return m_grid.Insert( item, lo, hi );
    }

    bool Remove( UInt32 handle ) { return m_grid.Remove( handle ); }

    bool Move( UInt32 handle, const TBox& bounds )
    {
        T lo[ 2 ], hi[ 2 ];
        ToArrays( bounds, lo, hi );
        return m_grid.Move( handle, lo, hi );
    }

    UInt32 GetCount( void ) const { return m_grid.GetCount(); }
    bool IsValid( UInt32 handle ) const { return m_grid.IsValid( handle ); }
    const TItem& GetItem( UInt32 handle ) const { return m_grid.GetItem( handle ); }
    UInt32 GetOccupiedCellCount( void ) const { return m_grid.GetOccupiedCellCount(); }
    bool Validate( void ) const { return m_grid.Validate(); }

    TBox GetBounds( UInt32 handle ) const
    {
        TBox result;
        if ( m_grid.IsValid( handle ) )
        {
            const T* lo = m_grid.GetLow( handle );
            const T* hi = m_grid.GetHigh( handle );
            result.min.x = lo[ 0 ]; result.min.y = lo[ 1 ];
            result.max.x = hi[ 0 ]; result.max.y = hi[ 1 ];
        }
        return result;
    }

    /** 'bool Visit( UInt32 handle, const TItem& item )' per item whose bounds touch or overlap the box; false stops */
    template < typename TVisitor >
    void Query( const TBox& box, TVisitor& visitor ) const
    {
        T lo[ 2 ], hi[ 2 ];
        ToArrays( box, lo, hi );
        m_grid.Query( CTBoxQueryRegion< T, 2 >( lo, hi ), visitor );
    }

    /** The items whose bounds are at most the radius from the centre */
    template < typename TVisitor >
    void Query( const TBall& ball, TVisitor& visitor ) const
    {
        const T centre[ 2 ] = { ball.centre.x, ball.centre.y };
        m_grid.Query( CTSphereQueryRegion< T, 2 >( centre, ball.radius ), visitor );
    }

    /** The items the ray touches within 'maxDistance' (finite), along the ray: 'bool Visit( UInt32 handle, const TItem& item, const T& enterDistance )' */
    template < typename TVisitor >
    void QueryRay( const CTRay2D< T >& ray, const T& maxDistance, TVisitor& visitor ) const
    {
        const T o[ 2 ] = { ray.origin.x, ray.origin.y };
        const T d[ 2 ] = { ray.direction.AsVector().x, ray.direction.AsVector().y };
        m_grid.QueryRay( o, d, maxDistance, visitor );
    }

    private:

    static void ToArrays( const TBox& box, T* lo, T* hi )
    {
        lo[ 0 ] = box.min.x; lo[ 1 ] = box.min.y;
        hi[ 0 ] = box.max.x; hi[ 1 ] = box.max.y;
    }

    TGrid m_grid;
};

typedef CTUniformGrid2D< Float32, UInt32 > TFloat32UniformGrid2DUInt32;
typedef CTUniformGrid2D< Float64, UInt32 > TFloat64UniformGrid2DUInt32;

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

#endif /* GUCEF_GEOMETRY_CTUNIFORMGRID2D_H ? */
