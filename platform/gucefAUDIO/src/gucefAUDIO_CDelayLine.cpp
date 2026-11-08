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

#ifndef GUCEF_AUDIO_CDELAYLINE_H
#include "gucefAUDIO_CDelayLine.h"
#define GUCEF_AUDIO_CDELAYLINE_H
#endif /* GUCEF_AUDIO_CDELAYLINE_H ? */

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

CDelayLine::CDelayLine( void )
    : m_buffers()
    , m_writePositions()
    , m_capacity( 0 )
    , m_mask( 0 )
    , m_maxDelay( 0 )
    , m_channelCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDelayLine::~CDelayLine()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDelayLine::Prepare( UInt32 channelCount, UInt32 maxDelayInFrames )
{GUCEF_TRACE;

    if ( 0 == channelCount || maxDelayInFrames > 0x20000000 )
        return false;

    // room for the longest delay and the samples that cubic reads use around it
    UInt32 capacity = 4;
    while ( capacity < maxDelayInFrames + 4 )
        capacity <<= 1;

    m_buffers.assign( (size_t) capacity * channelCount, 0.0f );
    m_writePositions.assign( channelCount, 0 );
    m_capacity = capacity;
    m_mask = capacity - 1;
    m_maxDelay = maxDelayInFrames;
    m_channelCount = channelCount;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDelayLine::GetChannelCount( void ) const
{
    return m_channelCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CDelayLine::GetMaxDelayInFrames( void ) const
{
    return m_maxDelay;
}

/*-------------------------------------------------------------------------*/

void
CDelayLine::Reset( void )
{
    for ( size_t i=0; i<m_buffers.size(); ++i )
        m_buffers[ i ] = 0.0f;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
