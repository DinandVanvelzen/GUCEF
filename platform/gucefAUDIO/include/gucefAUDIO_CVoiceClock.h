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

#ifndef GUCEF_AUDIO_CVOICECLOCK_H
#define GUCEF_AUDIO_CVOICECLOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The microphone-to-speaker time line of a voice receiver: the clock in which the network side stamps the arrival
 *  of frames (CVoiceReceiver::PushFrame) and in which the playout source of the receiver places its render requests.
 *  Both must use the same clock, or the playout control measures the difference of two clocks instead of its buffer.
 *
 *  This class is the system clock: MT::PrecisionTickCount() in microseconds. Tests and simulations derive from it to
 *  drive a receiver in simulated time.
 *
 *  Real time contract: GetTimeInUs() and CaptureTickToTimeInUs() are called on the audio thread at every render
 *  request, so an implementation must not allocate, lock, wait or log. They are called from several threads at once.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceClock
{
    public:

    CVoiceClock( void );

    virtual ~CVoiceClock();

    /** Microseconds of a monotonic clock, from an arbitrary but fixed start */
    virtual UInt64 GetTimeInUs( void ) const;

    /**
     *  Converts the capture time a sender gave a frame (CVoiceFrame::GetCaptureTickCount(), a tick count of
     *  MT::PrecisionTickCount() on the sending side) to this clock. False when the two clocks cannot be compared;
     *  the receiver then reports no one-way delay. The system clock converts: sender and receiver in one process (tests)
     *  or on machines whose clocks are synchronised give a meaningful delay
     */
    virtual bool CaptureTickToTimeInUs( UInt64 tickCount, UInt64& timeInUs ) const;

    /** MT::PrecisionTickCount() converted to microseconds: the time base of the system clock */
    static UInt64 GetSystemTimeInUs( void );

    /** A tick count of MT::PrecisionTickCount() in microseconds */
    static UInt64 TicksToMicroseconds( UInt64 tickCount );

    /** The process wide system clock, the default of every receiver */
    static const CVoiceClock& GetSystemClock( void );

    private:

    CVoiceClock( const CVoiceClock& src );              /**< not implemented */
    CVoiceClock& operator=( const CVoiceClock& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICECLOCK_H ? */
