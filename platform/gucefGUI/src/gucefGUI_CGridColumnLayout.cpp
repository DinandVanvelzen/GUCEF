/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include "gucefGUI_CGridColumnLayout.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridColumnLayout::CGridColumnLayout( void )
    : m_columns()
    , m_lefts()
    , m_widths()
    , m_positionOf()
    , m_frozenCount( 0 )
    , m_frozenWidth( 0 )
    , m_rowHeaderWidth( 0 )
    , m_contentWidth( 0 )
    , m_viewWidth( 0 )
    , m_scrollX( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGridColumnLayout::Compute( const TInt32Vector& baseWidths ,
                            const TInt32Vector& order      ,
                            const TInt32Vector& hidden     ,
                            Int32 frozenColumns            ,
                            Int32 rowHeaderWidth           ,
                            Int32 viewWidth                ,
                            Int32 scrollX                  )
{GUCEF_TRACE;

    const Int32 columnCount = (Int32) baseWidths.size();
    m_columns.clear();
    m_lefts.clear();
    m_widths.clear();
    m_positionOf.assign( baseWidths.size(), -1 );

    // the order: the columns of the list that exist, once each, then the others as they are
    GUCEF::vector< bool > placed( baseWidths.size(), false );
    GUCEF::vector< bool > isHidden( baseWidths.size(), false );
    for ( size_t i=0; i<hidden.size(); ++i )
        if ( hidden[ i ] >= 0 && hidden[ i ] < columnCount )
            isHidden[ hidden[ i ] ] = true;
    TInt32Vector sequence;
    for ( size_t i=0; i<order.size(); ++i )
    {
        const Int32 column = order[ i ];
        if ( column >= 0 && column < columnCount && !placed[ column ] )
        {
            placed[ column ] = true;
            sequence.push_back( column );
        }
    }
    for ( Int32 column=0; column<columnCount; ++column )
        if ( !placed[ column ] )
            sequence.push_back( column );

    Int32 left = 0;
    for ( size_t i=0; i<sequence.size(); ++i )
    {
        const Int32 column = sequence[ i ];
        if ( isHidden[ column ] )
            continue;
        m_positionOf[ column ] = (Int32) m_columns.size();
        m_columns.push_back( column );
        m_lefts.push_back( left );
        const Int32 width = baseWidths[ column ] > 0 ? baseWidths[ column ] : 0;
        m_widths.push_back( width );
        left += width;
    }
    m_contentWidth = left;
    m_rowHeaderWidth = rowHeaderWidth > 0 ? rowHeaderWidth : 0;
    m_viewWidth = viewWidth;

    // at least one column scrolls
    const Int32 positions = (Int32) m_columns.size();
    m_frozenCount = frozenColumns > 0 ? frozenColumns : 0;
    if ( m_frozenCount > positions - 1 )
        m_frozenCount = positions > 0 ? positions - 1 : 0;
    m_frozenWidth = m_frozenCount > 0 ? m_lefts[ m_frozenCount - 1 ] + m_widths[ m_frozenCount - 1 ] : 0;

    const Int32 max = GetMaxScrollX();
    m_scrollX = scrollX < 0 ? 0 : ( scrollX > max ? max : scrollX );
}

/*-------------------------------------------------------------------------*/

UInt32
CGridColumnLayout::GetPositionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_columns.size();
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetColumn( Int32 position ) const
{GUCEF_TRACE;

    return position >= 0 && position < (Int32) m_columns.size() ? m_columns[ position ] : -1;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::FindPosition( Int32 column ) const
{GUCEF_TRACE;

    return column >= 0 && column < (Int32) m_positionOf.size() ? m_positionOf[ column ] : -1;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetWidth( Int32 position ) const
{GUCEF_TRACE;

    return position >= 0 && position < (Int32) m_widths.size() ? m_widths[ position ] : 0;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetContentLeft( Int32 position ) const
{GUCEF_TRACE;

    if ( position < 0 )
        return 0;
    return position < (Int32) m_lefts.size() ? m_lefts[ position ] : m_contentWidth;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetScreenLeft( Int32 position ) const
{GUCEF_TRACE;

    return m_rowHeaderWidth + GetContentLeft( position ) - ( IsFrozen( position ) ? 0 : m_scrollX );
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnLayout::IsFrozen( Int32 position ) const
{GUCEF_TRACE;

    return position >= 0 && position < m_frozenCount;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetFrozenCount( void ) const
{GUCEF_TRACE;

    return m_frozenCount;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetFrozenWidth( void ) const
{GUCEF_TRACE;

    return m_frozenWidth;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetRowHeaderWidth( void ) const
{GUCEF_TRACE;

    return m_rowHeaderWidth;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetContentWidth( void ) const
{GUCEF_TRACE;

    return m_contentWidth;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetScrollViewWidth( void ) const
{GUCEF_TRACE;

    const Int32 width = m_viewWidth - m_rowHeaderWidth - m_frozenWidth;
    return width > 0 ? width : 0;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetScrollX( void ) const
{GUCEF_TRACE;

    return m_scrollX;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetMaxScrollX( void ) const
{GUCEF_TRACE;

    const Int32 max = ( m_contentWidth - m_frozenWidth ) - GetScrollViewWidth();
    return max > 0 ? max : 0;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::PositionAt( Int32 x ) const
{GUCEF_TRACE;

    if ( x < m_rowHeaderWidth || m_columns.empty() )
        return -1;
    Int32 content = x - m_rowHeaderWidth;
    if ( content >= m_frozenWidth )
    {
        // the columns that scroll start where the frozen ones end
        if ( x >= m_viewWidth )
            return -1;
        content += m_scrollX;
    }
    if ( content >= m_contentWidth )
        return -1;

    // the last column that starts at or before the point
    Int32 low = 0, high = (Int32) m_lefts.size() - 1;
    Int32 found = -1;
    while ( low <= high )
    {
        const Int32 middle = low + ( high - low ) / 2;
        if ( m_lefts[ middle ] <= content )
        {
            found = middle;
            low = middle + 1;
        }
        else
            high = middle - 1;
    }
    // zero width columns have no point: the point belongs to the last column that starts there and has room
    while ( found >= 0 && content >= m_lefts[ found ] + m_widths[ found ] )
    {
        if ( found + 1 < (Int32) m_lefts.size() && m_lefts[ found + 1 ] <= content )
            ++found;
        else
            return -1;
    }
    return found;
}

/*-------------------------------------------------------------------------*/

void
CGridColumnLayout::GetVisibleScrolledPositions( Int32& first, Int32& last ) const
{GUCEF_TRACE;

    first = m_frozenCount;
    last = m_frozenCount - 1;
    const Int32 positions = (Int32) m_columns.size();
    if ( m_frozenCount >= positions )
        return;
    const Int32 viewLeft = m_frozenWidth + m_scrollX;
    const Int32 viewRight = viewLeft + GetScrollViewWidth();

    // the first column that ends after the left of the view
    Int32 low = m_frozenCount, high = positions - 1;
    first = positions;
    while ( low <= high )
    {
        const Int32 middle = low + ( high - low ) / 2;
        if ( m_lefts[ middle ] + m_widths[ middle ] > viewLeft )
        {
            first = middle;
            high = middle - 1;
        }
        else
            low = middle + 1;
    }
    last = first - 1;
    for ( Int32 p=first; p<positions && m_lefts[ p ] < viewRight; ++p )
        last = p;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnLayout::GetScrollToShow( Int32 position ) const
{GUCEF_TRACE;

    if ( position < 0 || position >= (Int32) m_columns.size() || IsFrozen( position ) )
        return m_scrollX;
    const Int32 left = m_lefts[ position ];
    const Int32 right = left + m_widths[ position ];
    const Int32 viewLeft = m_frozenWidth + m_scrollX;
    const Int32 viewWidth = GetScrollViewWidth();
    Int32 scroll = m_scrollX;
    if ( left < viewLeft )
        scroll = left - m_frozenWidth;
    else
    if ( right > viewLeft + viewWidth )
        scroll = right - m_frozenWidth - viewWidth;
    // a column wider than the view shows its left side
    if ( m_widths[ position ] > viewWidth )
        scroll = left - m_frozenWidth;
    const Int32 max = GetMaxScrollX();
    return scroll < 0 ? 0 : ( scroll > max ? max : scroll );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
