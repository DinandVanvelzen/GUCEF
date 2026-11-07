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
#include <algorithm>

#include "gucefGUI_CChartSeries.h"

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

/** Orders indexes of points by their x, for a stable sort */
class CXLess
{
    public:

    explicit CXLess( const CChartSeries::TInt64Vector& x ) : m_x( x ) {}

    bool operator()( UInt32 a, UInt32 b ) const { return m_x[ a ] < m_x[ b ]; }

    private:

    const CChartSeries::TInt64Vector& m_x;
};

const Int64 Int64Max = (Int64) ( ( ~(UInt64) 0 ) >> 1 );

/** NaN is the only value that differs from itself */
inline bool IsNaN( Float64 value ) { return value != value; }

/** Neither NaN nor infinite: the difference of an infinite value with itself is NaN */
inline bool IsFinite( Float64 value ) { return value == value && ( value - value ) == 0.0; }

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CChartSeries::CChartSeries( void )
    : m_name()
    , m_type( SERIESTYPE_LINE )
    , m_colour()
    , m_hasColour( false )
    , m_stackGroup()
    , m_visible( true )
    , m_lineWidth( 2 )
    , m_x()
    , m_values()
    , m_revision( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChartSeries::CChartSeries( const CString& name, TSeriesType type )
    : m_name( name )
    , m_type( type )
    , m_colour()
    , m_hasColour( false )
    , m_stackGroup()
    , m_visible( true )
    , m_lineWidth( 2 )
    , m_x()
    , m_values()
    , m_revision( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChartSeries::CChartSeries( const CChartSeries& src )
    : m_name( src.m_name )
    , m_type( src.m_type )
    , m_colour( src.m_colour )
    , m_hasColour( src.m_hasColour )
    , m_stackGroup( src.m_stackGroup )
    , m_visible( src.m_visible )
    , m_lineWidth( src.m_lineWidth )
    , m_x( src.m_x )
    , m_values( src.m_values )
    , m_revision( src.m_revision )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChartSeries&
CChartSeries::operator=( const CChartSeries& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_name = src.m_name;
        m_type = src.m_type;
        m_colour = src.m_colour;
        m_hasColour = src.m_hasColour;
        m_stackGroup = src.m_stackGroup;
        m_visible = src.m_visible;
        m_lineWidth = src.m_lineWidth;
        m_x = src.m_x;
        m_values = src.m_values;
        Changed();
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CChartSeries::~CChartSeries()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CChartSeries::Changed( void )
{GUCEF_TRACE;

    ++m_revision;
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::SetName( const CString& name )
{GUCEF_TRACE;

    if ( m_name != name )
    {
        m_name = name;
        Changed();
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CChartSeries::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::SetType( TSeriesType type )
{GUCEF_TRACE;

    if ( m_type != type )
    {
        m_type = type;
        Changed();
    }
}

/*-------------------------------------------------------------------------*/

CChartSeries::TSeriesType
CChartSeries::GetType( void ) const
{GUCEF_TRACE;

    return m_type;
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::SetColour( const CColour& colour )
{GUCEF_TRACE;

    m_colour = colour;
    m_hasColour = true;
    Changed();
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::ResetColour( void )
{GUCEF_TRACE;

    m_hasColour = false;
    Changed();
}

/*-------------------------------------------------------------------------*/

bool
CChartSeries::HasColour( void ) const
{GUCEF_TRACE;

    return m_hasColour;
}

/*-------------------------------------------------------------------------*/

const CColour&
CChartSeries::GetColour( void ) const
{GUCEF_TRACE;

    return m_colour;
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::SetStackGroup( const CString& stackGroup )
{GUCEF_TRACE;

    if ( m_stackGroup != stackGroup )
    {
        m_stackGroup = stackGroup;
        Changed();
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CChartSeries::GetStackGroup( void ) const
{GUCEF_TRACE;

    return m_stackGroup;
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::SetVisible( bool visible )
{GUCEF_TRACE;

    if ( m_visible != visible )
    {
        m_visible = visible;
        Changed();
    }
}

/*-------------------------------------------------------------------------*/

bool
CChartSeries::IsVisible( void ) const
{GUCEF_TRACE;

    return m_visible;
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::SetLineWidth( Int32 width )
{GUCEF_TRACE;

    if ( width < 1 )
        width = 1;
    if ( m_lineWidth != width )
    {
        m_lineWidth = width;
        Changed();
    }
}

/*-------------------------------------------------------------------------*/

Int32
CChartSeries::GetLineWidth( void ) const
{GUCEF_TRACE;

    return m_lineWidth;
}

/*-------------------------------------------------------------------------*/

bool
CChartSeries::SetData( const Int64* x, const Float64* values, UInt32 count )
{GUCEF_TRACE;

    if ( count > 0 && ( GUCEF_NULL == x || GUCEF_NULL == values ) )
        return false;

    // assign() keeps the capacity of the previous data when the new data fits in it
    m_x.assign( x, x + count );
    m_values.assign( values, values + count );
    SortByX();
    Changed();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChartSeries::SetData( const UInt64* x, const Float64* values, UInt32 count )
{GUCEF_TRACE;

    if ( count > 0 && ( GUCEF_NULL == x || GUCEF_NULL == values ) )
        return false;
    for ( UInt32 i=0; i<count; ++i )
        if ( x[ i ] > (UInt64) Int64Max )
            return false;

    m_x.resize( count );
    for ( UInt32 i=0; i<count; ++i )
        m_x[ i ] = (Int64) x[ i ];
    m_values.assign( values, values + count );
    SortByX();
    Changed();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::SortByX( void )
{GUCEF_TRACE;

    const UInt32 count = (UInt32) m_x.size();
    bool sorted = true;
    for ( UInt32 i=1; i<count; ++i )
    {
        if ( m_x[ i ] < m_x[ i-1 ] )
        {
            sorted = false;
            break;
        }
    }
    if ( sorted )
        return;

    // only for data that comes in out of order: sort an index, then reorder both arrays by it
    GUCEF::vector< UInt32 > order( count );
    for ( UInt32 i=0; i<count; ++i )
        order[ i ] = i;
    std::stable_sort( order.begin(), order.end(), CXLess( m_x ) );
    TInt64Vector x( count );
    TFloat64Vector values( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        x[ i ] = m_x[ order[ i ] ];
        values[ i ] = m_values[ order[ i ] ];
    }
    m_x.swap( x );
    m_values.swap( values );
}

/*-------------------------------------------------------------------------*/

bool
CChartSeries::Append( Int64 x, Float64 value )
{GUCEF_TRACE;

    if ( !m_x.empty() && x < m_x.back() )
        return false;
    m_x.push_back( x );
    m_values.push_back( value );
    Changed();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::Reserve( UInt32 count )
{GUCEF_TRACE;

    m_x.reserve( count );
    m_values.reserve( count );
}

/*-------------------------------------------------------------------------*/

void
CChartSeries::Clear( void )
{GUCEF_TRACE;

    m_x.clear();
    m_values.clear();
    Changed();
}

/*-------------------------------------------------------------------------*/

UInt32
CChartSeries::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_x.size();
}

/*-------------------------------------------------------------------------*/

Int64
CChartSeries::GetX( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_x.size() ? m_x[ index ] : 0;
}

/*-------------------------------------------------------------------------*/

Float64
CChartSeries::GetValue( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_values.size() ? m_values[ index ] : 0.0;
}

/*-------------------------------------------------------------------------*/

const Int64*
CChartSeries::GetXData( void ) const
{GUCEF_TRACE;

    return m_x.empty() ? GUCEF_NULL : &m_x[ 0 ];
}

/*-------------------------------------------------------------------------*/

const Float64*
CChartSeries::GetValueData( void ) const
{GUCEF_TRACE;

    return m_values.empty() ? GUCEF_NULL : &m_values[ 0 ];
}

/*-------------------------------------------------------------------------*/

UInt32
CChartSeries::LowerBound( Int64 x ) const
{GUCEF_TRACE;

    return (UInt32) ( std::lower_bound( m_x.begin(), m_x.end(), x ) - m_x.begin() );
}

/*-------------------------------------------------------------------------*/

UInt32
CChartSeries::UpperBound( Int64 x ) const
{GUCEF_TRACE;

    return (UInt32) ( std::upper_bound( m_x.begin(), m_x.end(), x ) - m_x.begin() );
}

/*-------------------------------------------------------------------------*/

UInt32
CChartSeries::FindNearestIndex( Int64 x ) const
{GUCEF_TRACE;

    const UInt32 count = (UInt32) m_x.size();
    if ( 0 == count )
        return 0;
    UInt32 above = LowerBound( x );
    if ( above >= count )
        return count - 1;
    if ( 0 == above )
        return 0;
    // the first of equal x values on the left, so a tie goes to the lower index
    UInt32 below = above - 1;
    while ( below > 0 && m_x[ below - 1 ] == m_x[ below ] )
        --below;
    UInt64 distanceBelow = (UInt64) x - (UInt64) m_x[ below ];
    UInt64 distanceAbove = (UInt64) m_x[ above ] - (UInt64) x;
    return distanceBelow <= distanceAbove ? below : above;
}

/*-------------------------------------------------------------------------*/

UInt32
CChartSeries::FindNearestValidIndex( Int64 x ) const
{GUCEF_TRACE;

    const UInt32 count = (UInt32) m_x.size();
    UInt32 nearest = FindNearestIndex( x );
    if ( nearest >= count )
        return count;
    if ( !IsNaN( m_values[ nearest ] ) )
        return nearest;

    // walk outwards to the nearest point with a value on either side
    Int32 left = (Int32) nearest - 1;
    while ( left >= 0 && IsNaN( m_values[ left ] ) )
        --left;
    UInt32 right = nearest + 1;
    while ( right < count && IsNaN( m_values[ right ] ) )
        ++right;
    if ( left < 0 && right >= count )
        return count;
    if ( left < 0 )
        return right;
    if ( right >= count )
        return (UInt32) left;
    UInt64 distanceLeft = x >= m_x[ left ] ? (UInt64) x - (UInt64) m_x[ left ] : 0;
    UInt64 distanceRight = m_x[ right ] >= x ? (UInt64) m_x[ right ] - (UInt64) x : 0;
    return distanceLeft <= distanceRight ? (UInt32) left : right;
}

/*-------------------------------------------------------------------------*/

bool
CChartSeries::GetXRange( Int64& minX, Int64& maxX ) const
{GUCEF_TRACE;

    if ( m_x.empty() )
        return false;
    minX = m_x.front();
    maxX = m_x.back();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChartSeries::GetValueRange( Float64& minValue, Float64& maxValue ) const
{GUCEF_TRACE;

    bool found = false;
    for ( UInt32 i=0; i<(UInt32)m_values.size(); ++i )
    {
        Float64 value = m_values[ i ];
        if ( !IsFinite( value ) )
            continue;
        if ( !found )
        {
            minValue = maxValue = value;
            found = true;
        }
        else
        {
            if ( value < minValue )
                minValue = value;
            if ( value > maxValue )
                maxValue = value;
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

UInt32
CChartSeries::GetRevision( void ) const
{GUCEF_TRACE;

    return m_revision;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
