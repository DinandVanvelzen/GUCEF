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

#include <string.h>

#include "gucefAUDIO_CAudioFrameRing.h"

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

CAudioFrameRing::CAudioFrameRing( void )
    : m_frames( GUCEF_NULL )
    , m_capacity( 0 )
    , m_write( 0 )
    , m_read( 0 )
    , m_dropped( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioFrameRing::~CAudioFrameRing()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CAudioFrameRing::Prepare( UInt32 capacityFrames )
{GUCEF_TRACE;

    Release();
    if ( 0 == capacityFrames || capacityFrames > 0x40000000UL )
        return false;
    UInt32 capacity = 1;
    while ( capacity < capacityFrames )
        capacity <<= 1;
    m_frames = GUCEF_NEW Float32[ capacity ];
    memset( m_frames, 0, (size_t) capacity * sizeof( Float32 ) );
    m_capacity = capacity;
    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioFrameRing::Release( void )
{GUCEF_TRACE;

    GUCEF_DELETE[] m_frames;
    m_frames = GUCEF_NULL;
    m_capacity = 0;
    Reset();
}

/*-------------------------------------------------------------------------*/

bool
CAudioFrameRing::IsPrepared( void ) const
{
    // no trace macro: the real time threads use it
    return GUCEF_NULL != m_frames;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFrameRing::GetCapacity( void ) const
{
    // no trace macro: the real time threads use it
    return m_capacity;
}

/*-------------------------------------------------------------------------*/

void
CAudioFrameRing::Reset( void )
{GUCEF_TRACE;

    m_write.Store( 0 );
    m_read.Store( 0 );
    m_dropped.Store( 0 );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFrameRing::GetFree( void ) const
{
    // no trace macro: the real time threads use it
    if ( 0 == m_capacity )
        return 0;
    const UInt64 used = m_write.Load( MT::MEMORDER_RELAXED ) - m_read.Load( MT::MEMORDER_ACQUIRE );
    return m_capacity - (UInt32) used;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFrameRing::GetAvailable( void ) const
{
    if ( 0 == m_capacity )
        return 0;
    return (UInt32) ( m_write.Load( MT::MEMORDER_ACQUIRE ) - m_read.Load( MT::MEMORDER_RELAXED ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFrameRing::Write( const Float32* frames, UInt32 count )
{
    if ( 0 == m_capacity || 0 == count )
        return 0;

    const UInt64 write = m_write.Load( MT::MEMORDER_RELAXED );
    const UInt32 free = m_capacity - (UInt32) ( write - m_read.Load( MT::MEMORDER_ACQUIRE ) );
    const UInt32 stored = count < free ? count : free;
    if ( stored < count )
        m_dropped.FetchAdd( count - stored, MT::MEMORDER_RELAXED );
    if ( 0 == stored )
        return 0;

    const UInt32 mask = m_capacity - 1;
    const UInt32 start = (UInt32) write & mask;
    const UInt32 first = ( m_capacity - start < stored ) ? m_capacity - start : stored;
    if ( GUCEF_NULL != frames )
    {
        memcpy( m_frames + start, frames, (size_t) first * sizeof( Float32 ) );
        if ( first < stored )
            memcpy( m_frames, frames + first, (size_t) ( stored - first ) * sizeof( Float32 ) );
    }
    else
    {
        memset( m_frames + start, 0, (size_t) first * sizeof( Float32 ) );
        if ( first < stored )
            memset( m_frames, 0, (size_t) ( stored - first ) * sizeof( Float32 ) );
    }
    m_write.Store( write + stored, MT::MEMORDER_RELEASE );
    return stored;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFrameRing::WriteSilence( UInt32 count )
{
    return Write( GUCEF_NULL, count );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFrameRing::Read( Float32* destination, UInt32 count )
{
    if ( 0 == m_capacity || 0 == count )
        return 0;

    const UInt64 read = m_read.Load( MT::MEMORDER_RELAXED );
    const UInt32 available = (UInt32) ( m_write.Load( MT::MEMORDER_ACQUIRE ) - read );
    const UInt32 taken = count < available ? count : available;
    if ( 0 == taken )
        return 0;

    if ( GUCEF_NULL != destination )
    {
        const UInt32 mask = m_capacity - 1;
        const UInt32 start = (UInt32) read & mask;
        const UInt32 first = ( m_capacity - start < taken ) ? m_capacity - start : taken;
        memcpy( destination, m_frames + start, (size_t) first * sizeof( Float32 ) );
        if ( first < taken )
            memcpy( destination + first, m_frames, (size_t) ( taken - first ) * sizeof( Float32 ) );
    }
    m_read.Store( read + taken, MT::MEMORDER_RELEASE );
    return taken;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFrameRing::Skip( UInt32 count )
{
    return Read( GUCEF_NULL, count );
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioFrameRing::GetWritePosition( void ) const
{
    return m_write.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioFrameRing::GetReadPosition( void ) const
{
    return m_read.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioFrameRing::GetDroppedFrameCount( void ) const
{
    return m_dropped.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
