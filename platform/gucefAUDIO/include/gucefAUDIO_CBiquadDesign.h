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

#ifndef GUCEF_AUDIO_CBIQUADDESIGN_H
#define GUCEF_AUDIO_CBIQUADDESIGN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIO_EFILTERRESPONSE_H
#include "gucefAUDIO_EFilterResponse.h"
#define GUCEF_AUDIO_EFILTERRESPONSE_H
#endif /* GUCEF_AUDIO_EFILTERRESPONSE_H ? */

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
 *  Coefficients of a second order digital filter section (a "biquad"), normalised so that a0 is 1:
 *
 *      y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
 */
struct SBiquadCoefficients
{
    Float64 b0;
    Float64 b1;
    Float64 b2;
    Float64 a1;
    Float64 a2;
};

/*-------------------------------------------------------------------------*/

/**
 *  Design of second order filters after the "Audio EQ Cookbook" of R. Bristow-Johnson (W3C Working Group Note
 *  version), and the exact magnitude response of the result.
 *  Meant for filters that are set once and for checking other filter implementations against (CSvfFilter has
 *  the same magnitude response). Everything is computed in Float64 and nothing is kept: all functions are static
 *  and thread safe.
 */
class GUCEF_AUDIO_PUBLIC_CPP CBiquadDesign
{
    public:

    /**
     *  Computes the coefficients of the response at the given settings.
     *  @param response         shape of the response
     *  @param sampleRate       sample rate in Hz
     *  @param frequency        corner or center frequency in Hz, limited to a little below half the sample rate
     *  @param q                quality factor, limited to a small positive value
     *  @param gainInDecibels   boost or cut of peaking and shelving responses, ignored by the others
     *  @param coefficients     receives the result
     *  @return false when the sample rate is not positive, 'coefficients' is then left alone
     */
    static bool Design( TFilterResponse response         ,
                        Float64 sampleRate               ,
                        Float64 frequency                ,
                        Float64 q                        ,
                        Float64 gainInDecibels           ,
                        SBiquadCoefficients& coefficients );

    /**
     *  Magnitude |H(e^jw)| (a linear factor) of the filter section at the given frequency
     */
    static Float64 GetMagnitude( const SBiquadCoefficients& coefficients ,
                                 Float64 sampleRate                      ,
                                 Float64 frequency                       );

    /**
     *  Same as GetMagnitude(), in decibels
     */
    static Float64 GetMagnitudeInDecibels( const SBiquadCoefficients& coefficients ,
                                           Float64 sampleRate                      ,
                                           Float64 frequency                       );

    /**
     *  Convenience for the analytic response of a design: magnitude in decibels at 'probeFrequency'
     *  of the response designed with the other arguments (see Design())
     */
    static Float64 GetMagnitudeInDecibels( TFilterResponse response ,
                                           Float64 sampleRate       ,
                                           Float64 frequency        ,
                                           Float64 q                ,
                                           Float64 gainInDecibels   ,
                                           Float64 probeFrequency   );

    /** The frequency limit used by Design(): a little below half the sample rate keeps the filter stable and finite */
    static Float64 GetMaxFrequency( Float64 sampleRate );

    private:

    CBiquadDesign( void );      /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CBIQUADDESIGN_H ? */
