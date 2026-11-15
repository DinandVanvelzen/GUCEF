/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOM_CMcpCallGate.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpCallGate::CMcpCallGate( void )
    : m_lock()
    , m_open( true )
    , m_nextTicket( 1 )
    , m_calls()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpCallGate::~CMcpCallGate()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_open = false;
    if ( !m_calls.empty() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "McpCallGate: The gate goes while " << (UInt32) m_calls.size() << " calls are still inside" );
}

/*-------------------------------------------------------------------------*/

bool
CMcpCallGate::Enter( CMcpRequestContext* context, UInt64& ticket )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    ticket = 0;
    if ( !m_open )
        return false;
    ticket = m_nextTicket++;
    m_calls[ ticket ] = GUCEF_NULL != context ? context->GetSharedPtr() : CMcpRequestContextPtr();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpCallGate::Leave( UInt64 ticket )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_calls.erase( ticket );
}

/*-------------------------------------------------------------------------*/

bool
CMcpCallGate::Close( UInt32 timeoutMs )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_open = false;
        for ( TCallMap::iterator i = m_calls.begin(); i != m_calls.end(); ++i )
        {
            if ( !(*i).second.IsNULL() )
                (*i).second->RequestCancellation();
        }
    }

    // the calls run on other threads: wait for them to leave, bounded
    const UInt64 start = MT::PrecisionTickCount();
    while ( true )
    {
        UInt32 inside = GetActiveCount();
        if ( 0 == inside )
            return true;
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) >= timeoutMs )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "McpCallGate: " << inside << " calls did not leave within " << timeoutMs << " ms after the gate closed" );
            return false;
        }
        MT::PrecisionDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpCallGate::Open( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_open = true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpCallGate::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CMcpCallGate::IsCloseRequested( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_open;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpCallGate::GetActiveCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_calls.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
