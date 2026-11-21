/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#include "TestThreads.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

struct CTestThreads::SSlot
{
    CTestThreadBody* body;
    UInt32 index;
    MT::SThreadData* thread;
    bool created;
    SSlot( void ) : body( GUCEF_NULL ), index( 0 ), thread( GUCEF_NULL ), created( false ) {}
};

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
CTestThreads::Main( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SSlot* slot = static_cast< SSlot* >( data );
    slot->body->Run( slot->index );
    return 0;
}

/*-------------------------------------------------------------------------*/

CTestThreads::CTestThreads( CTestThreadBody& body, UInt32 count )
    : m_body( body )
    , m_count( count )
    , m_slots( GUCEF_NULL )
    , m_started( false )
    , m_joined( false )
{GUCEF_TRACE;

    m_slots = GUCEF_NEW SSlot[ count > 0 ? count : 1 ];
    for ( UInt32 i=0; i<count; ++i )
    {
        m_slots[ i ].body = &body;
        m_slots[ i ].index = i;
        m_slots[ i ].thread = MT::ThreadDataReserve();
    }
}

/*-------------------------------------------------------------------------*/

CTestThreads::~CTestThreads()
{GUCEF_TRACE;

    if ( m_started && !m_joined )
        Join( 120000 );
    for ( UInt32 i=0; i<m_count; ++i )
    {
        if ( m_joined || !m_slots[ i ].created )
            MT::ThreadDataCleanup( m_slots[ i ].thread );
    }
    GUCEF_DELETE[] m_slots;
}

/*-------------------------------------------------------------------------*/

bool
CTestThreads::Start( void )
{GUCEF_TRACE;

    bool allStarted = true;
    for ( UInt32 i=0; i<m_count; ++i )
    {
        if ( 0 != MT::ThreadCreate( m_slots[ i ].thread, &Main, &m_slots[ i ] ) )
            m_slots[ i ].created = true;
        else
            allStarted = false;
    }
    m_started = true;
    return allStarted;
}

/*-------------------------------------------------------------------------*/

bool
CTestThreads::Join( UInt32 timeoutMs )
{GUCEF_TRACE;

    bool allEnded = true;
    for ( UInt32 i=0; i<m_count; ++i )
    {
        if ( m_slots[ i ].created && 1 != MT::ThreadWait( m_slots[ i ].thread, timeoutMs ) )
            allEnded = false;
    }
    m_joined = allEnded;
    return allEnded;
}

/*-------------------------------------------------------------------------*/

TThreadId
CTestThreads::GetThreadId( UInt32 index ) const
{GUCEF_TRACE;

    if ( index >= m_count || !m_slots[ index ].created )
        return 0;
    return MT::ThreadIdentity( m_slots[ index ].thread );
}

/*-------------------------------------------------------------------------*/
