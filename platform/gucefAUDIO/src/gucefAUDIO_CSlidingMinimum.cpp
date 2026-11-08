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

#ifndef GUCEF_AUDIO_CSLIDINGMINIMUM_H
#include "gucefAUDIO_CSlidingMinimum.h"
#define GUCEF_AUDIO_CSLIDINGMINIMUM_H
#endif /* GUCEF_AUDIO_CSLIDINGMINIMUM_H ? */

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

CSlidingMinimum::CSlidingMinimum( void )
    : m_values()
    , m_indices()
    , m_windowSize( 0 )
    , m_head( 0 )
    , m_count( 0 )
    , m_position( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSlidingMinimum::~CSlidingMinimum()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CSlidingMinimum::Prepare( UInt32 windowSize )
{GUCEF_TRACE;

    if ( 0 == windowSize )
        return false;

    m_values.assign( windowSize, 0.0f );
    m_indices.assign( windowSize, 0 );
    m_windowSize = windowSize;
    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSlidingMinimum::SetWindowSize( UInt32 windowSize )
{
    if ( 0 == windowSize || windowSize > (UInt32) m_values.size() )
        return false;

    // the ring wraps at the window size, inside the memory of the larger window
    m_windowSize = windowSize;
    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CSlidingMinimum::GetWindowSize( void ) const
{
    return m_windowSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CSlidingMinimum::GetCapacity( void ) const
{
    return (UInt32) m_values.size();
}

/*-------------------------------------------------------------------------*/

void
CSlidingMinimum::Reset( void )
{
    m_head = 0;
    m_count = 0;
    m_position = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
