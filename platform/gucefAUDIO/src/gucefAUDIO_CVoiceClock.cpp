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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefAUDIO_CVoiceClock.h"

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

CVoiceClock::CVoiceClock( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceClock::~CVoiceClock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceClock::GetTimeInUs( void ) const
{
    // no trace macro: this runs on the audio thread
    return GetSystemTimeInUs();
}

/*-------------------------------------------------------------------------*/

bool
CVoiceClock::CaptureTickToTimeInUs( UInt64 tickCount, UInt64& timeInUs ) const
{
    // no trace macro: this runs on the audio thread
    timeInUs = TicksToMicroseconds( tickCount );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceClock::TicksToMicroseconds( UInt64 tickCount )
{
    // no trace macro: this runs on the audio thread
    const UInt64 resolution = MT::PrecisionTimerResolution();
    if ( 1000000 == resolution || 0 == resolution )
        return tickCount;
    // whole seconds and the rest apart, so a large tick count of a fast timer does not overflow
    return ( tickCount / resolution ) * 1000000 + ( ( tickCount % resolution ) * 1000000 ) / resolution;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceClock::GetSystemTimeInUs( void )
{
    // no trace macro: this runs on the audio thread
    return TicksToMicroseconds( MT::PrecisionTickCount() );
}

/*-------------------------------------------------------------------------*/

const CVoiceClock&
CVoiceClock::GetSystemClock( void )
{
    static const CVoiceClock systemClock;
    return systemClock;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
