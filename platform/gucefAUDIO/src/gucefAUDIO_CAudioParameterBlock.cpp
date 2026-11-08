/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H
#include "gucefAUDIO_CAudioParameterBlock.h"
#define GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H
#endif /* GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioParameterBlock::CAudioParameterBlock( UInt32 count )
    : m_count( count )
    , m_values( GUCEF_NULL )
    , m_sequence( 0 )
    , m_writeLock()
    , m_writeSequence( 0 )
    , m_snapshot( GUCEF_NULL )
    , m_staging( GUCEF_NULL )
    , m_snapshotSequence( 0 )
{GUCEF_TRACE;

    if ( m_count > 0 )
    {
        m_values = GUCEF_NEW CAtomicFloat[ m_count ];
        m_snapshot = GUCEF_NEW Float32[ m_count ];
        m_staging = GUCEF_NEW Float32[ m_count ];
        for ( UInt32 i=0; i<m_count; ++i )
        {
            m_snapshot[ i ] = 0.0f;
            m_staging[ i ] = 0.0f;
        }
    }
}

/*-------------------------------------------------------------------------*/

CAudioParameterBlock::~CAudioParameterBlock()
{GUCEF_TRACE;

    GUCEF_DELETE[] m_values;
    m_values = GUCEF_NULL;
    GUCEF_DELETE[] m_snapshot;
    m_snapshot = GUCEF_NULL;
    GUCEF_DELETE[] m_staging;
    m_staging = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioParameterBlock::GetCount( void ) const
{GUCEF_TRACE;

    return m_count;
}

/*-------------------------------------------------------------------------*/

void
CAudioParameterBlock::BeginWrite( void )
{GUCEF_TRACE;

    // odd: a write is under way. Acquire keeps the value stores below from moving above this mark
    m_writeSequence = m_sequence.FetchAdd( 1, MT::MEMORDER_ACQ_REL );
}

/*-------------------------------------------------------------------------*/

void
CAudioParameterBlock::EndWrite( void )
{GUCEF_TRACE;

    // even again: the publication is complete. Release keeps the value stores above from moving below it
    m_sequence.Store( m_writeSequence + 2, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

void
CAudioParameterBlock::Set( UInt32 index, Float32 value )
{GUCEF_TRACE;

    if ( index >= m_count )
        return;

    MT::CScopeMutex lock( m_writeLock );
    BeginWrite();
    m_values[ index ].Store( value, MT::MEMORDER_RELEASE );
    EndWrite();
}

/*-------------------------------------------------------------------------*/

void
CAudioParameterBlock::Set( const UInt32* indices, const Float32* values, UInt32 count )
{GUCEF_TRACE;

    if ( GUCEF_NULL == indices || GUCEF_NULL == values || 0 == count )
        return;

    MT::CScopeMutex lock( m_writeLock );
    BeginWrite();
    for ( UInt32 i=0; i<count; ++i )
        if ( indices[ i ] < m_count )
            m_values[ indices[ i ] ].Store( values[ i ], MT::MEMORDER_RELEASE );
    EndWrite();
}

/*-------------------------------------------------------------------------*/

Float32
CAudioParameterBlock::Get( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_count ? m_values[ index ].Load() : 0.0f;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioParameterBlock::GetPublicationCount( void ) const
{GUCEF_TRACE;

    return m_sequence.Load( MT::MEMORDER_ACQUIRE ) / 2;
}

/*-------------------------------------------------------------------------*/

bool
CAudioParameterBlock::Refresh( void )
{
    const UInt32 before = m_sequence.Load( MT::MEMORDER_ACQUIRE );
    if ( before == m_snapshotSequence || 0 != ( before & 1 ) )
        return false;                       // nothing new, or a writer is busy: the previous snapshot stays

    // Acquire loads: a value stored by a writer that started after 'before' makes that writer's odd mark visible below
    for ( UInt32 i=0; i<m_count; ++i )
        m_staging[ i ] = m_values[ i ].Load( MT::MEMORDER_ACQUIRE );

    const UInt32 after = m_sequence.Load( MT::MEMORDER_RELAXED );
    if ( after != before )
        return false;                       // a write overlapped the copy, it may be torn: the next refresh takes a clean one

    for ( UInt32 i=0; i<m_count; ++i )
        m_snapshot[ i ] = m_staging[ i ];
    m_snapshotSequence = before;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioParameterBlock::SynchronizeSnapshot( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writeLock );
    for ( UInt32 i=0; i<m_count; ++i )
        m_snapshot[ i ] = m_values[ i ].Load( MT::MEMORDER_ACQUIRE );
    m_snapshotSequence = m_sequence.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
