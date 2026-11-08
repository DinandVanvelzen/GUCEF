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

#ifndef GUCEF_AUDIO_CVOICERECEIVERSTATS_H
#define GUCEF_AUDIO_CVOICERECEIVERSTATS_H

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
 *  Counters and the current values of a CVoiceReceiver. A value class (a snapshot): the audio thread publishes the
 *  playout part once per render request, the receiver adds the counters of the network side.
 *
 *  An RTCP receiver report block (RFC 3550 section 6.4.1) takes: the extended highest sequence number, the cumulative
 *  loss (appendix A.3), the fraction lost since the previous report (fractionLost, which CVoiceReceiver::GetStats()
 *  computes over the interval since its previous call) and the interarrival jitter in timestamp units. The counters of
 *  the playout run from CVoiceReceiver::Prepare() and start over at Reset() and Start().
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceReceiverStats
{
    public:

    // network side (PushFrame)
    UInt64 framesPushed;                    /**< frames handed to PushFrame() */
    UInt64 framesQueued;                    /**< frames that went into the inbound queue */
    UInt64 inboundQueueFull;                /**< frames dropped because the inbound queue was full */
    UInt64 wrongStream;                     /**< frames of another stream id than the accepted one, dropped */
    UInt64 wrongPayloadType;                /**< frames of another payload type than the codec's, dropped */
    UInt32 streamId;                        /**< the stream accepted, 0 while none */

    // jitter buffer: RFC 3550
    UInt64 framesReceived;                  /**< frames that reached the jitter buffer, without duplicates */
    UInt64 framesExpected;                  /**< RFC 3550 A.3 */
    Int64 cumulativeLost;                   /**< RFC 3550 A.3: expected minus received */
    UInt8 fractionLost;                     /**< RFC 3550 A.3: lost in the interval since the previous GetStats() / expected, in 1/256 */
    UInt64 extendedHighestSequenceNumber;
    Float64 interarrivalJitter;             /**< RFC 3550 6.4.1, in timestamp units */
    Float64 interarrivalJitterInMs;
    UInt64 duplicates;
    UInt64 reordered;
    UInt64 lateDropped;                     /**< dropped by the jitter buffer because their time had passed */
    UInt64 redundantStored;                 /**< frames filled in from an RFC 2198 block */
    UInt64 overflowDropped;                 /**< pushed out of the slot window by a frame too far ahead */
    UInt64 restarts;                        /**< stream restarts after a jump of the sequence number */
    UInt32 bufferedFrames;                  /**< frames in the jitter buffer now */

    // playout
    TVoicePlayoutState state;
    UInt64 framesDecoded;
    UInt64 framesConcealed;
    UInt64 framesRecovered;                 /**< recovered with forward error correction from the next frame */
    UInt64 framesAccelerated;               /**< silent or concealed frames dropped to lower the delay */
    UInt64 framesStretched;                 /**< frames played after concealment had stood in for them */
    UInt64 lateFrames;                      /**< frames whose time had passed while playing (concealment replaced them) */
    UInt64 underruns;                       /**< times the buffer ran dry during a talkspurt */
    UInt64 decodeErrors;
    UInt64 talkspurts;                      /**< playout starts */
    Float64 marginInMs;                     /**< the margin M */
    bool hasMinimumLevel;
    Float64 minimumLevelInMs;               /**< Lmin */
    Float64 levelInMs;                      /**< audio buffered beyond the current render request */
    Float64 ratioInPpm;                     /**< ( playout ratio - 1 ) in parts per million; positive consumes faster */
    Float64 integralInPpm;                  /**< the integral term: the estimate of the clock drift */
    Float64 lastPlayoutDelayInMs;           /**< from the arrival of the last decoded frame to the output of its first sample */
    Float64 meanPlayoutDelayInMs;           /**< since the start of the talkspurt */
    Float64 maxPlayoutDelayInMs;

    // one-way delay: from the sender's capture time to the output, when the clocks compare (CVoiceClock)
    UInt64 oneWayDelayCount;                /**< decoded frames that had a comparable capture time */
    Float64 lastOneWayDelayInMs;
    Float64 meanOneWayDelayInMs;
    Float64 maxOneWayDelayInMs;

    // playout source
    UInt64 renderRequests;                  /**< ReadFrames() calls */
    UInt64 framesOutput;                    /**< frames handed to the voice */
    bool resampling;                        /**< the drift resampler is in use (otherwise the decoded audio passes bit for bit) */
    UInt64 resamplerSwitches;               /**< changes from the direct path to the resampler (one per playout at most, a reset goes back to the direct path) */
    UInt64 resets;                          /**< resets carried out (Reset(), Start()) */
    UInt64 outputShortfalls;                /**< render requests that could not be filled from the decoded audio (silence in their place) */

    CVoiceReceiverStats( void );

    /** Sets every counter and value to 0 */
    void Clear( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICERECEIVERSTATS_H ? */
