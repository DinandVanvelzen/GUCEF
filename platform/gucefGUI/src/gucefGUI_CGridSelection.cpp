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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefGUI_CGridSelection.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Where a row (or position) goes when 'count' of them were inserted before 'first' */
Int32
ShiftForInsert( Int32 index, Int32 first, Int32 count )
{
    return index >= first ? index + count : index;
}

/** Where a row (or position) goes when 'count' of them were removed from 'first' on; one that was removed goes to the place the rows after it moved to */
Int32
ShiftForRemove( Int32 index, Int32 first, Int32 count )
{
    if ( index < first )
        return index;
    if ( index >= first + count )
        return index - count;
    return first;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridSelection::CGridSelection( void )
    : m_ranges()
    , m_anchorRow( -1 )
    , m_anchorPosition( -1 )
    , m_extentRow( -1 )
    , m_extentPosition( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridSelection::SRange
CGridSelection::MakeRange( Int32 rowA, Int32 positionA, Int32 rowB, Int32 positionB )
{GUCEF_TRACE;

    SRange range;
    range.firstRow = rowA < rowB ? rowA : rowB;
    range.lastRow = rowA < rowB ? rowB : rowA;
    range.firstColumn = positionA < positionB ? positionA : positionB;
    range.lastColumn = positionA < positionB ? positionB : positionA;
    range.wholeRows = false;
    range.wholeColumns = false;
    return range;
}

/*-------------------------------------------------------------------------*/

bool
CGridSelection::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_ranges.empty();
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::Clear( void )
{GUCEF_TRACE;

    m_ranges.clear();
    m_anchorRow = -1;
    m_anchorPosition = -1;
    m_extentRow = -1;
    m_extentPosition = -1;
}

/*-------------------------------------------------------------------------*/

UInt32
CGridSelection::GetRangeCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_ranges.size();
}

/*-------------------------------------------------------------------------*/

const CGridSelection::SRange&
CGridSelection::GetRange( UInt32 index ) const
{GUCEF_TRACE;

    static const SRange none = { 0, -1, 0, -1, false, false };
    return index < m_ranges.size() ? m_ranges[ index ] : none;
}

/*-------------------------------------------------------------------------*/

const CGridSelection::TRangeVector&
CGridSelection::GetRanges( void ) const
{GUCEF_TRACE;

    return m_ranges;
}

/*-------------------------------------------------------------------------*/

bool
CGridSelection::Contains( Int32 row, Int32 position ) const
{GUCEF_TRACE;

    for ( size_t i=m_ranges.size(); i>0; --i )
    {
        const SRange& range = m_ranges[ i - 1 ];
        const bool rowIn = range.wholeColumns || ( row >= range.firstRow && row <= range.lastRow );
        const bool columnIn = range.wholeRows || ( position >= range.firstColumn && position <= range.lastColumn );
        if ( rowIn && columnIn )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridSelection::ContainsWholeRow( Int32 row ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_ranges.size(); ++i )
        if ( m_ranges[ i ].wholeRows && ( m_ranges[ i ].wholeColumns || ( row >= m_ranges[ i ].firstRow && row <= m_ranges[ i ].lastRow ) ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridSelection::ContainsWholeColumn( Int32 position ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_ranges.size(); ++i )
        if ( m_ranges[ i ].wholeColumns && ( m_ranges[ i ].wholeRows || ( position >= m_ranges[ i ].firstColumn && position <= m_ranges[ i ].lastColumn ) ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridSelection::TouchesRow( Int32 row ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_ranges.size(); ++i )
        if ( m_ranges[ i ].wholeColumns || ( row >= m_ranges[ i ].firstRow && row <= m_ranges[ i ].lastRow ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

Int32
CGridSelection::GetAnchorRow( void ) const
{GUCEF_TRACE;

    return m_anchorRow;
}

/*-------------------------------------------------------------------------*/

Int32
CGridSelection::GetAnchorPosition( void ) const
{GUCEF_TRACE;

    return m_anchorPosition;
}

/*-------------------------------------------------------------------------*/

Int32
CGridSelection::GetExtentRow( void ) const
{GUCEF_TRACE;

    return m_extentRow;
}

/*-------------------------------------------------------------------------*/

Int32
CGridSelection::GetExtentPosition( void ) const
{GUCEF_TRACE;

    return m_extentPosition;
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::SelectCell( Int32 row, Int32 position )
{GUCEF_TRACE;

    SelectRect( row, position, row, position );
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::SelectRect( Int32 anchorRow, Int32 anchorPosition, Int32 row, Int32 position )
{GUCEF_TRACE;

    m_ranges.clear();
    m_ranges.push_back( MakeRange( anchorRow, anchorPosition, row, position ) );
    m_anchorRow = anchorRow;
    m_anchorPosition = anchorPosition;
    m_extentRow = row;
    m_extentPosition = position;
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::AddCell( Int32 row, Int32 position )
{GUCEF_TRACE;

    m_ranges.push_back( MakeRange( row, position, row, position ) );
    m_anchorRow = row;
    m_anchorPosition = position;
    m_extentRow = row;
    m_extentPosition = position;
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::ExtendTo( Int32 row, Int32 position )
{GUCEF_TRACE;

    if ( m_ranges.empty() || m_anchorRow < 0 )
    {
        SelectCell( row, position );
        return;
    }
    SRange& last = m_ranges.back();
    if ( last.wholeRows && last.wholeColumns )
    {
        const SRange range = MakeRange( m_anchorRow, m_anchorPosition, row, position );
        last = range;
    }
    else
    if ( last.wholeRows )
    {
        last.firstRow = m_anchorRow < row ? m_anchorRow : row;
        last.lastRow = m_anchorRow < row ? row : m_anchorRow;
    }
    else
    if ( last.wholeColumns )
    {
        last.firstColumn = m_anchorPosition < position ? m_anchorPosition : position;
        last.lastColumn = m_anchorPosition < position ? position : m_anchorPosition;
    }
    else
    {
        const SRange range = MakeRange( m_anchorRow, m_anchorPosition, row, position );
        last = range;
    }
    m_extentRow = row;
    m_extentPosition = position;
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::SelectRows( Int32 anchorRow, Int32 row, bool add )
{GUCEF_TRACE;

    SRange range = MakeRange( anchorRow, 0, row, 0 );
    range.firstColumn = 0;
    range.lastColumn = 0;
    range.wholeRows = true;
    if ( !add )
        m_ranges.clear();
    m_ranges.push_back( range );
    m_anchorRow = anchorRow;
    m_extentRow = row;
    if ( m_anchorPosition < 0 )
        m_anchorPosition = 0;
    m_extentPosition = m_anchorPosition;
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::SelectColumns( Int32 anchorPosition, Int32 position, bool add )
{GUCEF_TRACE;

    SRange range = MakeRange( 0, anchorPosition, 0, position );
    range.firstRow = 0;
    range.lastRow = 0;
    range.wholeColumns = true;
    if ( !add )
        m_ranges.clear();
    m_ranges.push_back( range );
    m_anchorPosition = anchorPosition;
    m_extentPosition = position;
    if ( m_anchorRow < 0 )
        m_anchorRow = 0;
    m_extentRow = m_anchorRow;
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::SelectAll( void )
{GUCEF_TRACE;

    SRange range = MakeRange( 0, 0, 0, 0 );
    range.wholeRows = true;
    range.wholeColumns = true;
    m_ranges.clear();
    m_ranges.push_back( range );
    if ( m_anchorRow < 0 )
        m_anchorRow = 0;
    if ( m_anchorPosition < 0 )
        m_anchorPosition = 0;
    m_extentRow = m_anchorRow;
    m_extentPosition = m_anchorPosition;
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::OnRowsInserted( Int32 firstRow, Int32 count )
{GUCEF_TRACE;

    if ( count <= 0 )
        return;
    for ( size_t i=0; i<m_ranges.size(); ++i )
    {
        if ( m_ranges[ i ].wholeColumns )
            continue;
        m_ranges[ i ].firstRow = ShiftForInsert( m_ranges[ i ].firstRow, firstRow, count );
        m_ranges[ i ].lastRow = ShiftForInsert( m_ranges[ i ].lastRow, firstRow, count );
    }
    if ( m_anchorRow >= 0 )
        m_anchorRow = ShiftForInsert( m_anchorRow, firstRow, count );
    if ( m_extentRow >= 0 )
        m_extentRow = ShiftForInsert( m_extentRow, firstRow, count );
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::OnRowsRemoved( Int32 firstRow, Int32 count )
{GUCEF_TRACE;

    if ( count <= 0 )
        return;
    TRangeVector kept;
    for ( size_t i=0; i<m_ranges.size(); ++i )
    {
        SRange range = m_ranges[ i ];
        if ( !range.wholeColumns )
        {
            const Int32 first = ShiftForRemove( range.firstRow, firstRow, count );
            Int32 last = range.lastRow;
            last = last < firstRow ? last : ( last >= firstRow + count ? last - count : firstRow - 1 );
            if ( last < first )
                continue;
            range.firstRow = first;
            range.lastRow = last;
        }
        kept.push_back( range );
    }
    m_ranges = kept;
    if ( m_anchorRow >= 0 )
        m_anchorRow = ShiftForRemove( m_anchorRow, firstRow, count );
    if ( m_extentRow >= 0 )
        m_extentRow = ShiftForRemove( m_extentRow, firstRow, count );
    if ( m_ranges.empty() )
        Clear();
}

/*-------------------------------------------------------------------------*/

void
CGridSelection::Clamp( Int32 rowCount, Int32 positionCount )
{GUCEF_TRACE;

    TRangeVector kept;
    for ( size_t i=0; i<m_ranges.size(); ++i )
    {
        SRange range = m_ranges[ i ];
        if ( !range.wholeColumns )
        {
            if ( range.lastRow >= rowCount )
                range.lastRow = rowCount - 1;
            if ( range.firstRow > range.lastRow )
                continue;
        }
        if ( !range.wholeRows )
        {
            if ( range.lastColumn >= positionCount )
                range.lastColumn = positionCount - 1;
            if ( range.firstColumn > range.lastColumn )
                continue;
        }
        kept.push_back( range );
    }
    m_ranges = kept;
    if ( m_ranges.empty() )
    {
        Clear();
        return;
    }
    if ( m_anchorRow >= rowCount ) m_anchorRow = rowCount - 1;
    if ( m_extentRow >= rowCount ) m_extentRow = rowCount - 1;
    if ( m_anchorPosition >= positionCount ) m_anchorPosition = positionCount - 1;
    if ( m_extentPosition >= positionCount ) m_extentPosition = positionCount - 1;
}

/*-------------------------------------------------------------------------*/

GUCEF::vector< CString >
CGridSelection::ToText( void ) const
{GUCEF_TRACE;

    GUCEF::vector< CString > lines;
    for ( size_t i=0; i<m_ranges.size(); ++i )
    {
        const SRange& range = m_ranges[ i ];
        const Int32 firstRow = range.wholeColumns ? 0 : range.firstRow;
        const Int32 lastRow = range.wholeColumns ? -1 : range.lastRow;
        const Int32 firstColumn = range.wholeRows ? 0 : range.firstColumn;
        const Int32 lastColumn = range.wholeRows ? -1 : range.lastColumn;
        lines.push_back( CORE::Int32ToString( firstRow ) + ',' + CORE::Int32ToString( firstColumn ) + ',' + CORE::Int32ToString( lastRow ) + ',' + CORE::Int32ToString( lastColumn ) );
    }
    return lines;
}

/*-------------------------------------------------------------------------*/

bool
CGridSelection::operator==( const CGridSelection& other ) const
{GUCEF_TRACE;

    if ( m_ranges.size() != other.m_ranges.size() )
        return false;
    for ( size_t i=0; i<m_ranges.size(); ++i )
    {
        const SRange& a = m_ranges[ i ];
        const SRange& b = other.m_ranges[ i ];
        if ( a.firstRow != b.firstRow || a.lastRow != b.lastRow || a.firstColumn != b.firstColumn || a.lastColumn != b.lastColumn ||
             a.wholeRows != b.wholeRows || a.wholeColumns != b.wholeColumns )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridSelection::operator!=( const CGridSelection& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
