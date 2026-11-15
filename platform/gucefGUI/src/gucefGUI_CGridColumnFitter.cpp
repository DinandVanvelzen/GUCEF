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

#include "gucefGUI_CGridColumnFitter.h"

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

CGridColumnFitter::SColumn::SColumn( void )
    : nominal( 0 )
    , content( 0 )
    , numeric( false )
    , fit()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnFitter::GetMinimumWidth( const TColumn& column )
{GUCEF_TRACE;

    Int32 minimum = column.fit.minWidth > 0 ? column.fit.minWidth : (Int32) MIN_WIDTH;
    switch ( column.fit.mode )
    {
        case CGridColumnFit::MODE_FIXED:
        {
            const Int32 fixed = column.nominal > 0 ? column.nominal : (Int32) DEFAULT_WIDTH;
            if ( fixed > minimum )
                minimum = fixed;
            break;
        }
        case CGridColumnFit::MODE_CONTENT:
        {
            if ( column.content > minimum )
                minimum = column.content;
            break;
        }
        case CGridColumnFit::MODE_AUTO:
        case CGridColumnFit::MODE_FILL:
        default:
        {
            // numbers and dates are never cut: their content is their minimum
            if ( column.numeric && column.content > minimum )
                minimum = column.content;
            break;
        }
    }
    return minimum;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnFitter::GetMaximumWidth( const TColumn& column )
{GUCEF_TRACE;

    if ( column.fit.maxWidth <= 0 )
        return 0;
    const Int32 minimum = GetMinimumWidth( column );
    return column.fit.maxWidth > minimum ? column.fit.maxWidth : minimum;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnFitter::GetNominalWidth( const TColumn& column )
{GUCEF_TRACE;

    Int32 width = 0;
    switch ( column.fit.mode )
    {
        case CGridColumnFit::MODE_CONTENT:
        {
            width = column.content > 0 ? column.content : ( column.nominal > 0 ? column.nominal : (Int32) DEFAULT_WIDTH );
            break;
        }
        case CGridColumnFit::MODE_FIXED:
        {
            width = column.nominal > 0 ? column.nominal : (Int32) DEFAULT_WIDTH;
            break;
        }
        case CGridColumnFit::MODE_AUTO:
        case CGridColumnFit::MODE_FILL:
        default:
        {
            width = column.nominal > 0 ? column.nominal : ( column.content > 0 ? column.content : (Int32) DEFAULT_WIDTH );
            break;
        }
    }
    const Int32 minimum = GetMinimumWidth( column );
    if ( width < minimum )
        width = minimum;
    const Int32 maximum = GetMaximumWidth( column );
    if ( maximum > 0 && width > maximum )
        width = maximum;
    return width;
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnFitter::IsGrower( const TColumn& column, bool anyFill, bool anyAutoText )
{GUCEF_TRACE;

    if ( anyFill )
        return CGridColumnFit::MODE_FILL == column.fit.mode;
    if ( CGridColumnFit::MODE_AUTO != column.fit.mode )
        return false;
    return anyAutoText ? !column.numeric : true;
}

/*-------------------------------------------------------------------------*/

Int32
CGridColumnFitter::Fit( const TColumnVector& columns, Int32 available, bool fill, TInt32Vector& widths )
{GUCEF_TRACE;

    const UInt32 count = (UInt32) columns.size();
    widths.assign( count, 0 );
    if ( 0 == count )
        return 0;

    TInt32Vector minimums( count, 0 );
    TInt32Vector maximums( count, 0 );
    Int64 total = 0;
    bool anyFill = false;
    bool anyAutoText = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        minimums[ i ] = GetMinimumWidth( columns[ i ] );
        maximums[ i ] = GetMaximumWidth( columns[ i ] );
        widths[ i ] = GetNominalWidth( columns[ i ] );
        total += widths[ i ];
        if ( CGridColumnFit::MODE_FILL == columns[ i ].fit.mode )
            anyFill = true;
        if ( CGridColumnFit::MODE_AUTO == columns[ i ].fit.mode && !columns[ i ].numeric )
            anyAutoText = true;
    }

    if ( fill && total < (Int64) available )
    {
        // the room to spare goes to the growers, in proportion to their widths, then in column order, up to their maximum
        const Int64 extra = (Int64) available - total;
        GUCEF::vector< UInt32 > growers;
        Int64 weightSum = 0;
        for ( UInt32 i=0; i<count; ++i )
        {
            if ( IsGrower( columns[ i ], anyFill, anyAutoText ) )
            {
                growers.push_back( i );
                weightSum += widths[ i ];
            }
        }
        Int64 given = 0;
        if ( weightSum > 0 )
        {
            for ( UInt32 g=0; g<(UInt32)growers.size(); ++g )
            {
                const UInt32 i = growers[ g ];
                Int64 share = extra * (Int64) widths[ i ] / weightSum;
                if ( maximums[ i ] > 0 && (Int64) widths[ i ] + share > (Int64) maximums[ i ] )
                    share = (Int64) maximums[ i ] - widths[ i ];
                if ( share < 0 )
                    share = 0;
                widths[ i ] += (Int32) share;
                given += share;
            }
        }
        Int64 left = extra - given;
        for ( UInt32 g=0; g<(UInt32)growers.size() && left > 0; ++g )
        {
            const UInt32 i = growers[ g ];
            Int64 room = maximums[ i ] > 0 ? (Int64) maximums[ i ] - widths[ i ] : left;
            if ( room > left )
                room = left;
            if ( room <= 0 )
                continue;
            widths[ i ] += (Int32) room;
            left -= room;
        }
    }
    else
    if ( total > (Int64) available )
    {
        // too little room: the lowest priority gives first, each column down to its minimum
        Int64 deficit = total - ( available > 0 ? (Int64) available : 0 );
        GUCEF::set< Int32 > priorities;
        for ( UInt32 i=0; i<count; ++i )
            priorities.insert( columns[ i ].fit.priority );
        for ( GUCEF::set< Int32 >::const_iterator p=priorities.begin(); p!=priorities.end() && deficit > 0; ++p )
        {
            Int64 roomSum = 0;
            for ( UInt32 i=0; i<count; ++i )
                if ( columns[ i ].fit.priority == *p && widths[ i ] > minimums[ i ] )
                    roomSum += widths[ i ] - minimums[ i ];
            if ( roomSum <= 0 )
                continue;
            if ( roomSum <= deficit )
            {
                for ( UInt32 i=0; i<count; ++i )
                    if ( columns[ i ].fit.priority == *p && widths[ i ] > minimums[ i ] )
                        widths[ i ] = minimums[ i ];
                deficit -= roomSum;
                continue;
            }
            Int64 taken = 0;
            for ( UInt32 i=0; i<count; ++i )
            {
                if ( columns[ i ].fit.priority != *p || widths[ i ] <= minimums[ i ] )
                    continue;
                const Int64 take = deficit * (Int64) ( widths[ i ] - minimums[ i ] ) / roomSum;
                widths[ i ] -= (Int32) take;
                taken += take;
            }
            Int64 left = deficit - taken;
            for ( UInt32 i=0; i<count && left > 0; ++i )
            {
                if ( columns[ i ].fit.priority != *p )
                    continue;
                Int64 room = (Int64) widths[ i ] - minimums[ i ];
                if ( room > left )
                    room = left;
                if ( room <= 0 )
                    continue;
                widths[ i ] -= (Int32) room;
                left -= room;
            }
            deficit = left;
        }
    }

    Int64 result = 0;
    for ( UInt32 i=0; i<count; ++i )
        result += widths[ i ];
    return (Int32) result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
