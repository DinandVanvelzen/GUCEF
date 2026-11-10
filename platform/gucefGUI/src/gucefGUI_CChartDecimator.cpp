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

#include <math.h>

#include "gucefGUI_CChartDecimator.h"

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

inline bool IsNaN( Float64 value ) { return value != value; }

/** The points kept of one column so far */
struct SColumnGroup
{
    bool active;
    Int32 column;
    UInt32 first;
    UInt32 lowest;
    UInt32 highest;
    UInt32 last;
};

/** Adds the kept points of a column in their original order, each once */
void
Flush( SColumnGroup& group, CChartDecimator::TIndexVector& keep )
{
    if ( !group.active )
        return;
    group.active = false;

    UInt32 indexes[ 4 ] = { group.first, group.lowest, group.highest, group.last };
    // four values: an insertion sort
    for ( UInt32 i=1; i<4; ++i )
    {
        UInt32 value = indexes[ i ];
        UInt32 j = i;
        while ( j > 0 && indexes[ j-1 ] > value )
        {
            indexes[ j ] = indexes[ j-1 ];
            --j;
        }
        indexes[ j ] = value;
    }
    for ( UInt32 i=0; i<4; ++i )
        if ( 0 == i || indexes[ i ] != indexes[ i-1 ] )
            keep.push_back( indexes[ i ] );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Int32
CChartDecimator::ColumnOf( Int64 x, Float64 xMin, Float64 xMax, Int32 pixelColumns )
{GUCEF_TRACE;

    if ( !( xMax > xMin ) || pixelColumns < 2 )
        return 0;
    Float64 column = floor( ( (Float64) x - xMin ) * (Float64) ( pixelColumns - 1 ) / ( xMax - xMin ) + 0.5 );
    // far outside the plot the column only has to keep its side
    if ( column > 1e9 )
        return 1000000000;
    if ( column < -1e9 )
        return -1000000000;
    return (Int32) column;
}

/*-------------------------------------------------------------------------*/

void
CChartDecimator::Decimate( const Int64* x, const Float64* values, UInt32 first, UInt32 last,
                           Float64 xMin, Float64 xMax, Int32 pixelColumns, TIndexVector& keep )
{GUCEF_TRACE;

    keep.clear();
    if ( GUCEF_NULL == x || GUCEF_NULL == values || last <= first )
        return;

    SColumnGroup group;
    group.active = false;
    group.column = 0;
    group.first = group.lowest = group.highest = group.last = 0;

    for ( UInt32 i=first; i<last; ++i )
    {
        const Float64 value = values[ i ];
        if ( IsNaN( value ) )
        {
            Flush( group, keep );
            keep.push_back( i );
            continue;
        }
        const Int32 column = ColumnOf( x[ i ], xMin, xMax, pixelColumns );
        if ( group.active && column == group.column )
        {
            if ( value < values[ group.lowest ] )
                group.lowest = i;
            if ( value > values[ group.highest ] )
                group.highest = i;
            group.last = i;
        }
        else
        {
            Flush( group, keep );
            group.active = true;
            group.column = column;
            group.first = group.lowest = group.highest = group.last = i;
        }
    }
    Flush( group, keep );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
