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

#include <math.h>

#ifndef GUCEF_AUDIO_CBIQUADDESIGN_H
#include "gucefAUDIO_CBiquadDesign.h"
#define GUCEF_AUDIO_CBIQUADDESIGN_H
#endif /* GUCEF_AUDIO_CBIQUADDESIGN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Float64 Pi = 3.14159265358979323846;
static const Float64 MinQ = 0.001;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Float64
CBiquadDesign::GetMaxFrequency( Float64 sampleRate )
{
    return 0.49 * sampleRate;
}

/*-------------------------------------------------------------------------*/

bool
CBiquadDesign::Design( TFilterResponse response         ,
                       Float64 sampleRate               ,
                       Float64 frequency                ,
                       Float64 q                        ,
                       Float64 gainInDecibels           ,
                       SBiquadCoefficients& coefficients )
{
    if ( !( sampleRate > 0.0 ) )
        return false;

    const Float64 maxFrequency = GetMaxFrequency( sampleRate );
    if ( frequency > maxFrequency ) frequency = maxFrequency;
    if ( !( frequency > 0.0 ) ) frequency = 1.0e-3;
    if ( !( q > MinQ ) ) q = MinQ;

    const Float64 w0 = 2.0 * Pi * frequency / sampleRate;
    const Float64 cosW0 = cos( w0 );
    const Float64 sinW0 = sin( w0 );
    const Float64 alpha = sinW0 / ( 2.0 * q );
    const Float64 a = pow( 10.0, gainInDecibels / 40.0 );
    const Float64 sqrtA2alpha = 2.0 * sqrt( a ) * alpha;

    Float64 b0, b1, b2, a0, a1, a2;
    switch ( response )
    {
        case FILTERRESPONSE_LOWPASS :
        {
            b0 = ( 1.0 - cosW0 ) / 2.0; b1 = 1.0 - cosW0; b2 = b0;
            a0 = 1.0 + alpha; a1 = -2.0 * cosW0; a2 = 1.0 - alpha;
            break;
        }
        case FILTERRESPONSE_HIGHPASS :
        {
            b0 = ( 1.0 + cosW0 ) / 2.0; b1 = -( 1.0 + cosW0 ); b2 = b0;
            a0 = 1.0 + alpha; a1 = -2.0 * cosW0; a2 = 1.0 - alpha;
            break;
        }
        case FILTERRESPONSE_BANDPASS :
        {
            b0 = sinW0 / 2.0; b1 = 0.0; b2 = -b0;
            a0 = 1.0 + alpha; a1 = -2.0 * cosW0; a2 = 1.0 - alpha;
            break;
        }
        case FILTERRESPONSE_BANDPASS_PEAK0DB :
        {
            b0 = alpha; b1 = 0.0; b2 = -alpha;
            a0 = 1.0 + alpha; a1 = -2.0 * cosW0; a2 = 1.0 - alpha;
            break;
        }
        case FILTERRESPONSE_NOTCH :
        {
            b0 = 1.0; b1 = -2.0 * cosW0; b2 = 1.0;
            a0 = 1.0 + alpha; a1 = -2.0 * cosW0; a2 = 1.0 - alpha;
            break;
        }
        case FILTERRESPONSE_ALLPASS :
        {
            b0 = 1.0 - alpha; b1 = -2.0 * cosW0; b2 = 1.0 + alpha;
            a0 = 1.0 + alpha; a1 = -2.0 * cosW0; a2 = 1.0 - alpha;
            break;
        }
        case FILTERRESPONSE_PEAKING :
        {
            b0 = 1.0 + alpha * a; b1 = -2.0 * cosW0; b2 = 1.0 - alpha * a;
            a0 = 1.0 + alpha / a; a1 = -2.0 * cosW0; a2 = 1.0 - alpha / a;
            break;
        }
        case FILTERRESPONSE_LOWSHELF :
        {
            b0 =        a * ( ( a + 1.0 ) - ( a - 1.0 ) * cosW0 + sqrtA2alpha );
            b1 =  2.0 * a * ( ( a - 1.0 ) - ( a + 1.0 ) * cosW0 );
            b2 =        a * ( ( a + 1.0 ) - ( a - 1.0 ) * cosW0 - sqrtA2alpha );
            a0 =              ( a + 1.0 ) + ( a - 1.0 ) * cosW0 + sqrtA2alpha;
            a1 = -2.0 *     ( ( a - 1.0 ) + ( a + 1.0 ) * cosW0 );
            a2 =              ( a + 1.0 ) + ( a - 1.0 ) * cosW0 - sqrtA2alpha;
            break;
        }
        case FILTERRESPONSE_HIGHSHELF :
        {
            b0 =         a * ( ( a + 1.0 ) + ( a - 1.0 ) * cosW0 + sqrtA2alpha );
            b1 = -2.0 *  a * ( ( a - 1.0 ) + ( a + 1.0 ) * cosW0 );
            b2 =         a * ( ( a + 1.0 ) + ( a - 1.0 ) * cosW0 - sqrtA2alpha );
            a0 =               ( a + 1.0 ) - ( a - 1.0 ) * cosW0 + sqrtA2alpha;
            a1 =  2.0 *      ( ( a - 1.0 ) - ( a + 1.0 ) * cosW0 );
            a2 =               ( a + 1.0 ) - ( a - 1.0 ) * cosW0 - sqrtA2alpha;
            break;
        }
        default :
        {
            // unknown response: pass through
            b0 = 1.0; b1 = 0.0; b2 = 0.0;
            a0 = 1.0; a1 = 0.0; a2 = 0.0;
            break;
        }
    }

    const Float64 inverseA0 = 1.0 / a0;
    coefficients.b0 = b0 * inverseA0;
    coefficients.b1 = b1 * inverseA0;
    coefficients.b2 = b2 * inverseA0;
    coefficients.a1 = a1 * inverseA0;
    coefficients.a2 = a2 * inverseA0;
    return true;
}

/*-------------------------------------------------------------------------*/

Float64
CBiquadDesign::GetMagnitude( const SBiquadCoefficients& coefficients ,
                             Float64 sampleRate                      ,
                             Float64 frequency                       )
{
    const Float64 w = 2.0 * Pi * frequency / sampleRate;
    const Float64 cosW = cos( w );
    const Float64 sinW = sin( w );
    const Float64 cos2W = cos( 2.0 * w );
    const Float64 sin2W = sin( 2.0 * w );

    // H( e^jw ) = ( b0 + b1 e^-jw + b2 e^-j2w ) / ( 1 + a1 e^-jw + a2 e^-j2w )
    const Float64 numRe = coefficients.b0 + coefficients.b1 * cosW + coefficients.b2 * cos2W;
    const Float64 numIm = -( coefficients.b1 * sinW + coefficients.b2 * sin2W );
    const Float64 denRe = 1.0 + coefficients.a1 * cosW + coefficients.a2 * cos2W;
    const Float64 denIm = -( coefficients.a1 * sinW + coefficients.a2 * sin2W );

    const Float64 numSquared = numRe * numRe + numIm * numIm;
    const Float64 denSquared = denRe * denRe + denIm * denIm;
    return sqrt( numSquared / denSquared );
}

/*-------------------------------------------------------------------------*/

Float64
CBiquadDesign::GetMagnitudeInDecibels( const SBiquadCoefficients& coefficients ,
                                       Float64 sampleRate                      ,
                                       Float64 frequency                       )
{
    return 20.0 * log10( GetMagnitude( coefficients, sampleRate, frequency ) );
}

/*-------------------------------------------------------------------------*/

Float64
CBiquadDesign::GetMagnitudeInDecibels( TFilterResponse response ,
                                       Float64 sampleRate       ,
                                       Float64 frequency        ,
                                       Float64 q                ,
                                       Float64 gainInDecibels   ,
                                       Float64 probeFrequency   )
{
    SBiquadCoefficients coefficients;
    if ( !Design( response, sampleRate, frequency, q, gainInDecibels, coefficients ) )
        return 0.0;
    return GetMagnitudeInDecibels( coefficients, sampleRate, probeFrequency );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
