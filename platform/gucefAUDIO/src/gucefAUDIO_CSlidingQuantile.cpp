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

#ifndef GUCEF_AUDIO_CSLIDINGQUANTILE_H
#include "gucefAUDIO_CSlidingQuantile.h"
#define GUCEF_AUDIO_CSLIDINGQUANTILE_H
#endif /* GUCEF_AUDIO_CSLIDINGQUANTILE_H ? */

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

CSlidingQuantile::CSlidingQuantile( void )
    : m_ring()
    , m_sorted()
    , m_windowSize( 0 )
    , m_head( 0 )
    , m_count( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSlidingQuantile::~CSlidingQuantile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CSlidingQuantile::Prepare( UInt32 windowSize )
{GUCEF_TRACE;

    if ( 0 == windowSize )
        return false;
    m_ring.assign( windowSize, 0.0 );
    m_sorted.assign( windowSize, 0.0 );
    m_windowSize = windowSize;
    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CSlidingQuantile::GetWindowSize( void ) const
{
    return m_windowSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CSlidingQuantile::GetCount( void ) const
{
    return m_count;
}

/*-------------------------------------------------------------------------*/

void
CSlidingQuantile::Reset( void )
{
    m_head = 0;
    m_count = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CSlidingQuantile::LowerBound( Float64 value ) const
{
    UInt32 low = 0;
    UInt32 high = m_count;
    while ( low < high )
    {
        const UInt32 middle = low + ( high - low ) / 2;
        if ( m_sorted[ middle ] < value )
            low = middle + 1;
        else
            high = middle;
    }
    return low;
}

/*-------------------------------------------------------------------------*/

void
CSlidingQuantile::Push( Float64 value )
{
    if ( 0 == m_windowSize )
        return;

    // the oldest value leaves a full window: it is in the sorted list exactly as it was pushed
    if ( m_count == m_windowSize )
    {
        const Float64 leaving = m_ring[ m_head ];
        m_head = ( m_head + 1 == m_windowSize ) ? 0 : m_head + 1;
        UInt32 at = LowerBound( leaving );
        if ( at >= m_count )
            at = m_count - 1;     // cannot happen for values that compare equal to themselves; keeps the list consistent otherwise
        Float64* sorted = &m_sorted[ 0 ];
        memmove( sorted + at, sorted + at + 1, (size_t) ( m_count - at - 1 ) * sizeof( Float64 ) );
        --m_count;
    }

    // a value that is not a number would break the order: it counts as 0
    if ( !( value == value ) )
        value = 0.0;

    UInt32 slot = m_head + m_count;
    if ( slot >= m_windowSize )
        slot -= m_windowSize;
    m_ring[ slot ] = value;

    // after the values that are not larger, so equal values keep their order of arrival
    UInt32 low = 0;
    UInt32 high = m_count;
    while ( low < high )
    {
        const UInt32 middle = low + ( high - low ) / 2;
        if ( m_sorted[ middle ] <= value )
            low = middle + 1;
        else
            high = middle;
    }
    Float64* sorted = &m_sorted[ 0 ];
    memmove( sorted + low + 1, sorted + low, (size_t) ( m_count - low ) * sizeof( Float64 ) );
    sorted[ low ] = value;
    ++m_count;
}

/*-------------------------------------------------------------------------*/

Float64
CSlidingQuantile::GetMinimum( void ) const
{
    return m_count > 0 ? m_sorted[ 0 ] : 0.0;
}

/*-------------------------------------------------------------------------*/

Float64
CSlidingQuantile::GetRanked( UInt32 rank ) const
{
    if ( 0 == m_count )
        return 0.0;
    return m_sorted[ rank < m_count ? rank : m_count - 1 ];
}

/*-------------------------------------------------------------------------*/

UInt32
CSlidingQuantile::GetQuantileRank( Float64 fraction ) const
{
    if ( !( fraction > 0.0 ) || 0 == m_count )
        return 0;
    if ( fraction >= 1.0 )
        return m_count - 1;
    const UInt32 rank = (UInt32) ( fraction * (Float64) m_count );
    return rank < m_count ? rank : m_count - 1;
}

/*-------------------------------------------------------------------------*/

Float64
CSlidingQuantile::GetQuantile( Float64 fraction ) const
{
    return GetRanked( GetQuantileRank( fraction ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CSlidingQuantile::CountBelow( Float64 limit ) const
{
    return LowerBound( limit );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
