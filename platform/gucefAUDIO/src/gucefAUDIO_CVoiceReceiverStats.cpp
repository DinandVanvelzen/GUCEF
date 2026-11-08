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

#include "gucefAUDIO_CVoiceReceiverStats.h"

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

CVoiceReceiverStats::CVoiceReceiverStats( void )
{
    // no trace macro: snapshots are made on the audio thread
    Clear();
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverStats::Clear( void )
{
    // no trace macro: snapshots are made on the audio thread
    framesPushed = 0;
    framesQueued = 0;
    inboundQueueFull = 0;
    wrongStream = 0;
    wrongPayloadType = 0;
    streamId = 0;

    framesReceived = 0;
    framesExpected = 0;
    cumulativeLost = 0;
    fractionLost = 0;
    extendedHighestSequenceNumber = 0;
    interarrivalJitter = 0.0;
    interarrivalJitterInMs = 0.0;
    duplicates = 0;
    reordered = 0;
    lateDropped = 0;
    redundantStored = 0;
    overflowDropped = 0;
    restarts = 0;
    bufferedFrames = 0;

    state = VOICEPLAYOUTSTATE_IDLE;
    framesDecoded = 0;
    framesConcealed = 0;
    framesRecovered = 0;
    framesAccelerated = 0;
    framesStretched = 0;
    lateFrames = 0;
    underruns = 0;
    decodeErrors = 0;
    talkspurts = 0;
    marginInMs = 0.0;
    hasMinimumLevel = false;
    minimumLevelInMs = 0.0;
    levelInMs = 0.0;
    ratioInPpm = 0.0;
    integralInPpm = 0.0;
    lastPlayoutDelayInMs = 0.0;
    meanPlayoutDelayInMs = 0.0;
    maxPlayoutDelayInMs = 0.0;

    oneWayDelayCount = 0;
    lastOneWayDelayInMs = 0.0;
    meanOneWayDelayInMs = 0.0;
    maxOneWayDelayInMs = 0.0;

    renderRequests = 0;
    framesOutput = 0;
    resampling = false;
    resamplerSwitches = 0;
    resets = 0;
    outputShortfalls = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
