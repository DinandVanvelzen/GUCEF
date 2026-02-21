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

#ifndef GUCEF_CORE_CSTOPWATCH_H
#include "gucefCORE_CStopwatch.h"
#define GUCEF_CORE_CSTOPWATCH_H
#endif /* GUCEF_CORE_CSTOPWATCH_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStopwatch::CStopwatch( void )
    : m_startTimestamp()
    , m_stopTimestamp()
    , m_isRunning( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStopwatch::CStopwatch( const CStopwatch& src )
    : m_startTimestamp( src.m_startTimestamp )
    , m_stopTimestamp( src.m_stopTimestamp )
    , m_isRunning( src.m_isRunning )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStopwatch::CStopwatch( bool startImmediately )
    : m_startTimestamp()
    , m_stopTimestamp()
    , m_isRunning( false )
{GUCEF_TRACE;

    if ( startImmediately )
    {
        Start();
    }
}

/*-------------------------------------------------------------------------*/

CStopwatch::~CStopwatch()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStopwatch&
CStopwatch::operator=( const CStopwatch& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_startTimestamp = src.m_startTimestamp;
        m_stopTimestamp = src.m_stopTimestamp;
        m_isRunning = src.m_isRunning;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CStopwatch::Start( void )
{GUCEF_TRACE;

    m_startTimestamp = CTimestamp::NowUTCTime();
    m_stopTimestamp.Clear();
    m_isRunning = true;
}

/*-------------------------------------------------------------------------*/

void
CStopwatch::Stop( void )
{GUCEF_TRACE;

    if ( m_isRunning )
    {
        m_stopTimestamp = CTimestamp::NowUTCTime();
        m_isRunning = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CStopwatch::Reset( void )
{GUCEF_TRACE;

    m_startTimestamp.Clear();
    m_stopTimestamp.Clear();
    m_isRunning = false;
}

/*-------------------------------------------------------------------------*/

void
CStopwatch::Restart( void )
{GUCEF_TRACE;

    Reset();
    Start();
}

/*-------------------------------------------------------------------------*/

bool
CStopwatch::IsRunning( void ) const
{GUCEF_TRACE;

    return m_isRunning;
}

/*-------------------------------------------------------------------------*/

bool
CStopwatch::HasStarted( void ) const
{GUCEF_TRACE;

    return m_startTimestamp.IsValid();
}

/*-------------------------------------------------------------------------*/

bool
CStopwatch::HasStopped( void ) const
{GUCEF_TRACE;

    return m_stopTimestamp.IsValid();
}

/*-------------------------------------------------------------------------*/

CTimestamp
CStopwatch::GetEffectiveEndTimestamp( void ) const
{GUCEF_TRACE;

    if ( !m_startTimestamp.IsValid() )
    {
        return CTimestamp::Empty;
    }
    
    if ( m_isRunning )
    {
        return CTimestamp::NowUTCTime();
    }
    
    return m_stopTimestamp;
}

/*-------------------------------------------------------------------------*/

Int64
CStopwatch::GetElapsedTicks( void ) const
{GUCEF_TRACE;

    if ( !m_startTimestamp.IsValid() )
    {
        return 0;
    }
    
    CTimestamp endTs = GetEffectiveEndTimestamp();
    return endTs - m_startTimestamp;
}

/*-------------------------------------------------------------------------*/

UInt64
CStopwatch::GetElapsedMilliseconds( void ) const
{GUCEF_TRACE;

    if ( !m_startTimestamp.IsValid() )
    {
        return 0;
    }
    
    CTimestamp endTs = GetEffectiveEndTimestamp();
    Int64 diffMs = m_startTimestamp.GetTimeDifferenceInMillisecondsTo( endTs );
    return diffMs > 0 ? (UInt64)diffMs : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CStopwatch::GetElapsedMicroseconds( void ) const
{GUCEF_TRACE;

    if ( !m_startTimestamp.IsValid() )
    {
        return 0;
    }
    
    CTimestamp endTs = GetEffectiveEndTimestamp();
    Int64 diffUs = m_startTimestamp.GetTimeDifferenceInMicrosecondsTo( endTs );
    return diffUs > 0 ? (UInt64)diffUs : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CStopwatch::GetElapsedNanoseconds( void ) const
{GUCEF_TRACE;

    if ( !m_startTimestamp.IsValid() )
    {
        return 0;
    }
    
    CTimestamp endTs = GetEffectiveEndTimestamp();
    
    // Calculate difference in nanoseconds
    UInt64 startNs = m_startTimestamp.ToNanosecondsSinceEpoch();
    UInt64 endNs = endTs.ToNanosecondsSinceEpoch();
    
    if ( endNs > startNs )
    {
        return endNs - startNs;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

Float64
CStopwatch::GetElapsedSeconds( void ) const
{GUCEF_TRACE;

    UInt64 elapsedNs = GetElapsedNanoseconds();
    return (Float64)elapsedNs / 1000000000.0;
}

/*-------------------------------------------------------------------------*/

const CTimestamp&
CStopwatch::GetStartTimestamp( void ) const
{GUCEF_TRACE;

    return m_startTimestamp;
}

/*-------------------------------------------------------------------------*/

const CTimestamp&
CStopwatch::GetStopTimestamp( void ) const
{GUCEF_TRACE;

    return m_stopTimestamp;
}

/*-------------------------------------------------------------------------*/

CStopwatch
CStopwatch::StartNew( void )
{GUCEF_TRACE;

    CStopwatch sw;
    sw.Start();
    return sw;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
