/*
 *  AssetWorkbenchLib: the arrangement of the models of a generated showcase
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

#include "AssetWorkbenchLib_CPackShowcaseLayout.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
ItemLess( const CPackShowcaseLayout::SItem& a, const CPackShowcaseLayout::SItem& b )
{
    const CORE::CString groupA = a.group.Lowercase();
    const CORE::CString groupB = b.group.Lowercase();
    if ( groupA != groupB )
        return groupA < groupB;
    const CORE::CString pathA = a.path.Lowercase();
    const CORE::CString pathB = b.path.Lowercase();
    if ( pathA != pathB )
        return pathA < pathB;
    return a.path < b.path;
}

Float32
Footprint( const CPackShowcaseLayout::SItem& item )
{
    const Float32 f = item.sizeX > item.sizeZ ? item.sizeX : item.sizeZ;
    return f > 1e-4f ? f : 1e-4f;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackShowcaseLayout::SItem::SItem( void )
    : path(), group(), sizeX( 1.0f ), sizeY( 1.0f ), sizeZ( 1.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackShowcaseLayout::SCell::SCell( void )
    : item( 0 ), x( 0.0f ), z( 0.0f ), scale( 1.0f ), row( 0 ), column( 0 ), groupIndex( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackShowcaseLayout::SOptions::SOptions( void )
    : maxColumns( 8 ), gap( 0.35f ), maxScaleRatio( 2.5f ), minShare( 0.0f ), maxUpscale( 4.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackShowcaseLayout::CPackShowcaseLayout( void )
    : m_cells(), m_rows( 0 ), m_groups( 0 ), m_pitch( 1.0f )
    , m_minX( 0.0f ), m_maxX( 0.0f ), m_minZ( 0.0f ), m_maxZ( 0.0f ), m_maxHeight( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackShowcaseLayout::GetGroupOfPath( const CORE::CString& path )
{GUCEF_TRACE;

    const Int32 slash = path.HasChar( '/', false );
    return slash > 0 ? path.SubstrFromRange( 0, (UInt32) slash ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CPackShowcaseLayout::SortItems( TItemVector& items )
{GUCEF_TRACE;

    std::stable_sort( items.begin(), items.end(), ItemLess );
}

/*-------------------------------------------------------------------------*/

bool
CPackShowcaseLayout::Compute( TItemVector& items, const SOptions& options )
{GUCEF_TRACE;

    m_cells.clear();
    m_rows = 0;
    m_groups = 0;
    if ( items.empty() )
        return false;
    SortItems( items );

    // the median footprint decides which models are outliers
    GUCEF::vector< Float32 > footprints;
    for ( size_t i=0; i<items.size(); ++i )
        footprints.push_back( Footprint( items[ i ] ) );
    GUCEF::vector< Float32 > sorted = footprints;
    std::sort( sorted.begin(), sorted.end() );
    const Float32 median = sorted[ sorted.size() / 2 ];
    const Float32 limit = median * ( options.maxScaleRatio > 1.0f ? options.maxScaleRatio : 1.0f );

    Float32 biggest = 0.0f;
    GUCEF::vector< Float32 > scales;
    for ( size_t i=0; i<items.size(); ++i )
    {
        const Float32 scale = footprints[ i ] > limit ? limit / footprints[ i ] : 1.0f;
        scales.push_back( scale );
        const Float32 scaled = footprints[ i ] * scale;
        if ( scaled > biggest )
            biggest = scaled;
    }
    m_pitch = biggest * ( 1.0f + options.gap );

    // small models are made bigger, up to a limit, so that they do not stand lost in cells that the biggest model decided the size of
    if ( options.minShare > 0.0f && biggest > 0.0f )
    {
        const Float32 smallest = biggest * ( options.minShare < 1.0f ? options.minShare : 1.0f );
        for ( size_t i=0; i<items.size(); ++i )
        {
            const Float32 scaled = footprints[ i ] * scales[ i ];
            if ( footprints[ i ] > 1e-6f && scaled < smallest )
            {
                Float32 up = smallest / footprints[ i ];
                if ( up > options.maxUpscale )
                    up = options.maxUpscale;
                // a tall thin model does not become a tower: its biggest side stays within one and a half of the biggest footprint
                const Float32 side = std::max( items[ i ].sizeX, std::max( items[ i ].sizeY, items[ i ].sizeZ ) );
                if ( side > 1e-6f && up * side > 1.5f * biggest )
                    up = 1.5f * biggest / side;
                scales[ i ] = up > scales[ i ] ? up : scales[ i ];
            }
        }
    }

    // every folder starts a row; a row has at most maxColumns models
    const UInt32 columns = options.maxColumns > 0 ? options.maxColumns : 1;
    UInt32 row = 0, column = 0, group = 0;
    for ( size_t i=0; i<items.size(); ++i )
    {
        if ( i > 0 && items[ i ].group != items[ i-1 ].group )
        {
            ++group;
            ++row;
            column = 0;
        }
        else
        if ( column >= columns )
        {
            ++row;
            column = 0;
        }
        SCell cell;
        cell.item = (UInt32) i;
        cell.row = row;
        cell.column = column;
        cell.groupIndex = group;
        cell.scale = scales[ i ];
        cell.x = (Float32) column * m_pitch;
        cell.z = (Float32) row * m_pitch;
        m_cells.push_back( cell );
        ++column;
    }
    m_rows = row + 1;
    m_groups = group + 1;

    // centred on the origin
    Float32 lowX = 1e30f, highX = -1e30f, lowZ = 1e30f, highZ = -1e30f;
    m_maxHeight = 0.0f;
    for ( size_t i=0; i<m_cells.size(); ++i )
    {
        if ( m_cells[ i ].x < lowX ) lowX = m_cells[ i ].x;
        if ( m_cells[ i ].x > highX ) highX = m_cells[ i ].x;
        if ( m_cells[ i ].z < lowZ ) lowZ = m_cells[ i ].z;
        if ( m_cells[ i ].z > highZ ) highZ = m_cells[ i ].z;
        const Float32 height = items[ m_cells[ i ].item ].sizeY * m_cells[ i ].scale;
        if ( height > m_maxHeight )
            m_maxHeight = height;
    }
    const Float32 middleX = 0.5f * ( lowX + highX );
    const Float32 middleZ = 0.5f * ( lowZ + highZ );
    for ( size_t i=0; i<m_cells.size(); ++i )
    {
        m_cells[ i ].x -= middleX;
        m_cells[ i ].z -= middleZ;
    }
    const Float32 half = 0.5f * m_pitch;
    m_minX = lowX - middleX - half;
    m_maxX = highX - middleX + half;
    m_minZ = lowZ - middleZ - half;
    m_maxZ = highZ - middleZ + half;
    return true;
}

/*-------------------------------------------------------------------------*/

const CPackShowcaseLayout::TCellVector& CPackShowcaseLayout::GetCells( void ) const { return m_cells; }
UInt32 CPackShowcaseLayout::GetRowCount( void ) const { return m_rows; }
UInt32 CPackShowcaseLayout::GetGroupCount( void ) const { return m_groups; }
Float32 CPackShowcaseLayout::GetPitch( void ) const { return m_pitch; }

/*-------------------------------------------------------------------------*/

void
CPackShowcaseLayout::GetExtent( Float32& minX, Float32& maxX, Float32& minZ, Float32& maxZ, Float32& maxHeight ) const
{GUCEF_TRACE;

    minX = m_minX;
    maxX = m_maxX;
    minZ = m_minZ;
    maxZ = m_maxZ;
    maxHeight = m_maxHeight;
}

/*-------------------------------------------------------------------------*/

void
CPackShowcaseLayout::GetBoundingSphere( Float32& centerX, Float32& centerY, Float32& centerZ, Float32& radius ) const
{GUCEF_TRACE;

    centerX = 0.5f * ( m_minX + m_maxX );
    centerZ = 0.5f * ( m_minZ + m_maxZ );
    centerY = 0.5f * m_maxHeight;
    const Float32 dx = m_maxX - m_minX, dz = m_maxZ - m_minZ;
    radius = 0.5f * sqrtf( dx * dx + dz * dz + m_maxHeight * m_maxHeight );
    if ( radius < 1e-4f )
        radius = 1.0f;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
