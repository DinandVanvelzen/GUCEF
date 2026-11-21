/*
 *  AssetWorkbenchLib: the arrangement of the views of a stage: one view or several, each with its own camera and display
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

#include "AssetWorkbenchLib_CWorkbenchViewLayout.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWorkbenchViewLayout::SCell::SCell( void )
    : name()
    , left( 0.0f )
    , top( 0.0f )
    , width( 1.0f )
    , height( 1.0f )
    , projection( PROJECTION_PERSPECTIVE )
    , direction( DIRECTION_FREE )
    , showGrid( true )
    , showAxes( true )
    , showBounds( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWorkbenchViewLayout::CWorkbenchViewLayout( void )
    : m_name()
    , m_cells()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchViewLayout::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchViewLayout::SetName( const CORE::CString& name )
{GUCEF_TRACE;

    m_name = name;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchViewLayout::Clear( void )
{GUCEF_TRACE;

    m_cells.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchViewLayout::AddCell( const SCell& cell )
{GUCEF_TRACE;

    m_cells.push_back( cell );
    return (UInt32) m_cells.size() - 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchViewLayout::GetCellCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_cells.size();
}

/*-------------------------------------------------------------------------*/

const CWorkbenchViewLayout::SCell&
CWorkbenchViewLayout::GetCell( UInt32 index ) const
{GUCEF_TRACE;

    static const SCell none;
    return index < (UInt32) m_cells.size() ? m_cells[ index ] : none;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchViewLayout::SetCellDisplay( UInt32 index, bool showGrid, bool showAxes, bool showBounds )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_cells.size() )
        return false;
    m_cells[ index ].showGrid = showGrid;
    m_cells[ index ].showAxes = showAxes;
    m_cells[ index ].showBounds = showBounds;
    return true;
}

/*-------------------------------------------------------------------------*/

CWorkbenchViewLayout
CWorkbenchViewLayout::MakeSingle( void )
{GUCEF_TRACE;

    CWorkbenchViewLayout layout;
    layout.SetName( "Single" );
    SCell cell;
    cell.name = "Perspective";
    layout.AddCell( cell );
    return layout;
}

/*-------------------------------------------------------------------------*/

CWorkbenchViewLayout
CWorkbenchViewLayout::MakeFourViews( void )
{GUCEF_TRACE;

    CWorkbenchViewLayout layout;
    layout.SetName( "Four views" );

    SCell cell;
    cell.width = 0.5f;
    cell.height = 0.5f;

    cell.name = "Perspective";
    cell.left = 0.0f;
    cell.top = 0.0f;
    layout.AddCell( cell );

    // the orthographic views show the axes, the grid is of the ground and so of the top view
    cell.projection = PROJECTION_ORTHOGRAPHIC;
    cell.name = "Front";
    cell.direction = DIRECTION_FRONT;
    cell.left = 0.5f;
    cell.top = 0.0f;
    cell.showGrid = false;
    layout.AddCell( cell );

    cell.name = "Side";
    cell.direction = DIRECTION_SIDE;
    cell.left = 0.0f;
    cell.top = 0.5f;
    layout.AddCell( cell );

    cell.name = "Top";
    cell.direction = DIRECTION_TOP;
    cell.left = 0.5f;
    cell.top = 0.5f;
    cell.showGrid = true;
    layout.AddCell( cell );
    return layout;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchViewLayout::operator==( const CWorkbenchViewLayout& other ) const
{GUCEF_TRACE;

    if ( m_cells.size() != other.m_cells.size() )
        return false;
    for ( size_t i=0; i<m_cells.size(); ++i )
    {
        const SCell& a = m_cells[ i ];
        const SCell& b = other.m_cells[ i ];
        if ( a.left != b.left || a.top != b.top || a.width != b.width || a.height != b.height || a.projection != b.projection || a.direction != b.direction ||
             a.showGrid != b.showGrid || a.showAxes != b.showAxes || a.showBounds != b.showBounds )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchViewLayout::operator!=( const CWorkbenchViewLayout& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
