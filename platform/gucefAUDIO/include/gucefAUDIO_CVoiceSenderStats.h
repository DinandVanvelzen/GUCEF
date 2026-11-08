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

#ifndef GUCEF_AUDIO_CVOICESENDERSTATS_H
#define GUCEF_AUDIO_CVOICESENDERSTATS_H

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
 *  Counters and the latest values of a CVoiceSender, since it was prepared. A value class (a snapshot): the capture
 *  thread publishes one after every frame, CVoiceSender::GetStats() hands out the latest.
 *
 *  For an RTCP sender report (RFC 3550 section 6.4.1) the network side has the packet count (framesSent), the octet
 *  count (octetsSent: RTP payload octets as CRtpVoicePacketCodec writes them, RFC 2198 headers included) and the RTP
 *  timestamp of the last frame with its capture time, to relate the RTP time line to its wall clock.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceSenderStats
{
    public:

    UInt32 streamId;                    /**< the RTP SSRC of the stream */
    UInt64 framesCaptured;              /**< frames of audio completed on the capture thread, sent or not */
    UInt64 framesEncoded;               /**< frames the encoder made a payload of */
    UInt64 framesSent;                  /**< frames put in the outbound queue (each took a sequence number) */
    UInt64 framesDropped;               /**< frames that took a sequence number but found the outbound queue full */
    UInt64 framesEmpty;                 /**< frames the encoder had nothing to send for (no frame, a gap in the timestamps) */
    UInt64 encodeErrors;                /**< frames the encoder refused (not sent) */
    UInt64 octetsSent;                  /**< RTP payload octets of the frames sent */
    UInt64 preRollFramesSent;           /**< frames of the pre-roll sent at the start of a talkspurt (voice activated mode) */
    UInt64 redundantBlocksSent;         /**< frames that carried the previous frame as RFC 2198 block */
    UInt64 redundancyOmitted;           /**< frames that would have carried a block but the packet would have exceeded MaxPacketSize */
    UInt64 talkspurts;                  /**< frames sent with the marker bit */
    UInt64 discontinuities;             /**< breaks in the captured audio (overruns): the frame in progress was lost */
    UInt64 codecSwitches;               /**< codec changes carried out on the capture thread */
    bool hasLastFrame;                  /**< a frame was sent: the two values below are valid */
    UInt32 lastTimestamp;               /**< RTP timestamp of the last frame sent */
    UInt64 lastCaptureTickCount;        /**< its capture time, MT::PrecisionTickCount() */
    UInt16 lastSequenceNumber;          /**< of the last frame sent */
    Float64 meanEncodeTimeInUs;         /**< mean time of Encode() */
    Float64 maxEncodeTimeInUs;          /**< worst time of Encode() */
    bool transmitting;                  /**< the last completed frame was sent */

    CVoiceSenderStats( void );

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

#endif /* GUCEF_AUDIO_CVOICESENDERSTATS_H ? */
