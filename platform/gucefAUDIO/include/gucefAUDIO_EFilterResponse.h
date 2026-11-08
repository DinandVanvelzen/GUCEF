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

#ifndef GUCEF_AUDIO_EFILTERRESPONSE_H
#define GUCEF_AUDIO_EFILTERRESPONSE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The shape of the response of a second order filter, with the names and meaning of the "Audio EQ Cookbook"
 *  of R. Bristow-Johnson. 'frequency' is the corner or center frequency, 'q' the quality factor and 'gain'
 *  the boost or cut in decibels for the responses that have one.
 *  The numeric values are stable, new responses are only ever appended.
 */
enum EFilterResponse
{
    FILTERRESPONSE_LOWPASS = 0      ,   /**< 12 dB per octave low pass, -3 dB + Q resonance at 'frequency' */
    FILTERRESPONSE_HIGHPASS         ,   /**< 12 dB per octave high pass */
    FILTERRESPONSE_BANDPASS         ,   /**< band pass with constant skirt gain: the peak at 'frequency' has a gain of Q */
    FILTERRESPONSE_NOTCH            ,   /**< band reject */
    FILTERRESPONSE_ALLPASS          ,   /**< unity magnitude, phase shift only */
    FILTERRESPONSE_PEAKING          ,   /**< bell: 'gain' decibels at 'frequency', 0 dB far away */
    FILTERRESPONSE_LOWSHELF         ,   /**< 'gain' decibels below 'frequency', 0 dB above */
    FILTERRESPONSE_HIGHSHELF        ,   /**< 'gain' decibels above 'frequency', 0 dB below */
    FILTERRESPONSE_BANDPASS_PEAK0DB     /**< band pass with a peak gain of 0 dB at 'frequency' */
};
typedef enum EFilterResponse TFilterResponse;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_EFILTERRESPONSE_H ? */
