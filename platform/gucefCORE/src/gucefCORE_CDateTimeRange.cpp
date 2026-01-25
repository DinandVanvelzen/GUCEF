/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_CDateTimeRange.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CDateTimeRange CDateTimeRange::Empty;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDateTimeRange::CDateTimeRange( void )
    : m_start()
    , m_end()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDateTimeRange::CDateTimeRange( const CDateTime& start, const CDateTime& end )
    : m_start( start )
    , m_end( end )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDateTimeRange::CDateTimeRange( const CDateTimeRange& src )
    : m_start( src.m_start )
    , m_end( src.m_end )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDateTimeRange::~CDateTimeRange()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDateTimeRange&
CDateTimeRange::operator=( const CDateTimeRange& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_start = src.m_start;
        m_end = src.m_end;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator==( const CDateTimeRange& other ) const
{GUCEF_TRACE;

    return m_start == other.m_start && m_end == other.m_end;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator!=( const CDateTimeRange& other ) const
{GUCEF_TRACE;

    return m_start != other.m_start || m_end != other.m_end;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator<( const CDateTimeRange& other ) const
{GUCEF_TRACE;

    if ( m_start < other.m_start )
        return true;
    if ( m_start > other.m_start )
        return false;
    return m_end < other.m_end;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator<=( const CDateTimeRange& other ) const
{GUCEF_TRACE;

    return (*this) < other || (*this) == other;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator>( const CDateTimeRange& other ) const
{GUCEF_TRACE;

    if ( m_start > other.m_start )
        return true;
    if ( m_start < other.m_start )
        return false;
    return m_end > other.m_end;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator>=( const CDateTimeRange& other ) const
{GUCEF_TRACE;

    return (*this) > other || (*this) == other;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator<( const CDateTime& dt ) const
{GUCEF_TRACE;

    // Range is less than datetime: range is entirely before the datetime
    return m_end < dt;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator<=( const CDateTime& dt ) const
{GUCEF_TRACE;

    // Range is less than or equal to datetime: range ends at or before the datetime
    return m_end <= dt;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator>( const CDateTime& dt ) const
{GUCEF_TRACE;

    // Range is greater than datetime: range is entirely after the datetime
    return m_start > dt;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::operator>=( const CDateTime& dt ) const
{GUCEF_TRACE;

    // Range is greater than or equal to datetime: range starts at or after the datetime
    return m_start >= dt;
}

/*-------------------------------------------------------------------------*/

void
CDateTimeRange::SetStart( const CDateTime& start )
{GUCEF_TRACE;

    m_start = start;
}

/*-------------------------------------------------------------------------*/

const CDateTime&
CDateTimeRange::GetStart( void ) const
{GUCEF_TRACE;

    return m_start;
}

/*-------------------------------------------------------------------------*/

CDateTime&
CDateTimeRange::GetStart( void )
{GUCEF_TRACE;

    return m_start;
}

/*-------------------------------------------------------------------------*/

void
CDateTimeRange::SetEnd( const CDateTime& end )
{GUCEF_TRACE;

    m_end = end;
}

/*-------------------------------------------------------------------------*/

const CDateTime&
CDateTimeRange::GetEnd( void ) const
{GUCEF_TRACE;

    return m_end;
}

/*-------------------------------------------------------------------------*/

CDateTime&
CDateTimeRange::GetEnd( void )
{GUCEF_TRACE;

    return m_end;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::HasRangeBegun( void ) const
{GUCEF_TRACE;

    CDateTime now = CDateTime::NowUTCDateTime();
    return now >= m_start;
}

/*-------------------------------------------------------------------------*/

bool
CDateTimeRange::HasRangeEnded( void ) const
{GUCEF_TRACE;

    CDateTime now = CDateTime::NowUTCDateTime();
    return now >= m_end;
}

/*-------------------------------------------------------------------------*/

UInt64
CDateTimeRange::TimeTillStartInMilliSecs( void ) const
{GUCEF_TRACE;

    if ( HasRangeBegun() )
        return 0;

    CDateTime now = CDateTime::NowUTCDateTime();
    Int64 diff = now.GetTimeDifferenceInMillisecondsTowards( m_start );
    
    if ( diff < 0 )
        return 0;
    
    return (UInt64) diff;
}

/*-------------------------------------------------------------------------*/

UInt64
CDateTimeRange::TimeTillEndInMilliSecs( void ) const
{GUCEF_TRACE;

    if ( HasRangeEnded() )
        return 0;

    CDateTime now = CDateTime::NowUTCDateTime();
    Int64 diff = now.GetTimeDifferenceInMillisecondsTowards( m_end );
    
    if ( diff < 0 )
        return 0;
    
    return (UInt64) diff;
}

/*-------------------------------------------------------------------------*/

UInt64
CDateTimeRange::GetTimeRangeTimeInMilliSecs( void ) const
{GUCEF_TRACE;

    Int64 diff = m_start.GetTimeDifferenceInMillisecondsTowards( m_end );
    
    if ( diff < 0 )
        diff = -diff;
    
    return (UInt64) diff;
}

/*-------------------------------------------------------------------------*/

void
CDateTimeRange::Clear( void )
{GUCEF_TRACE;

    m_start.Clear();
    m_end.Clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
