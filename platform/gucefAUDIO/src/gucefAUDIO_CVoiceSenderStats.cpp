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

#include "gucefAUDIO_CVoiceSenderStats.h"

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

CVoiceSenderStats::CVoiceSenderStats( void )
{
    // no trace macro: snapshots are made on the capture thread
    Clear();
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderStats::Clear( void )
{
    // no trace macro: snapshots are made on the capture thread
    streamId = 0;
    framesCaptured = 0;
    framesEncoded = 0;
    framesSent = 0;
    framesDropped = 0;
    framesEmpty = 0;
    encodeErrors = 0;
    octetsSent = 0;
    preRollFramesSent = 0;
    redundantBlocksSent = 0;
    redundancyOmitted = 0;
    talkspurts = 0;
    discontinuities = 0;
    codecSwitches = 0;
    hasLastFrame = false;
    lastTimestamp = 0;
    lastCaptureTickCount = 0;
    lastSequenceNumber = 0;
    meanEncodeTimeInUs = 0.0;
    maxEncodeTimeInUs = 0.0;
    transmitting = false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
