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

#ifndef GUCEF_AUDIO_CAUDIOCONVERTER_H
#define GUCEF_AUDIO_CAUDIOCONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

#ifndef GUCEF_AUDIO_IAUDIORESAMPLER_H
#include "gucefAUDIO_IAudioResampler.h"
#define GUCEF_AUDIO_IAUDIORESAMPLER_H
#endif /* GUCEF_AUDIO_IAUDIORESAMPLER_H ? */

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

class CAudioBuffer;

/*-------------------------------------------------------------------------*/

/**
 *  All conversions between audio formats. Every sample type goes through 32 bit float (hub and spoke), so a new
 *  sample type needs one pair of routines, not one for every other type.
 *
 *  Integer samples map to floats by dividing by 2^(bits-1), so -32768 is exactly -1.0 for 16 bit, and back by
 *  multiplying, rounding to nearest and limiting to the range of the type. A conversion to a float and back to
 *  the same integer type therefore gives the same integers for 8, 16 and 24 bit. The 32 bit types keep the
 *  precision of a 32 bit float (24 bits of mantissa) on the way through. Floats beyond -1.0 .. 1.0 are limited
 *  when converted to integers and left alone when converted to floats, not a number becomes silence.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioConverter
{
    public:

    /** Converts 'sampleCount' samples (frames x channels, interleaved or not) to floats */
    static void ConvertToFloat32( const void* source, TSampleType sourceType, Float32* destination, UInt64 sampleCount );

    /** Converts 'sampleCount' float samples to the given type */
    static void ConvertFromFloat32( const Float32* source, void* destination, TSampleType destinationType, UInt64 sampleCount );

    /** Converts between any two sample types. The areas may be the same when the types have the same size */
    static void ConvertSamples( const void* source, TSampleType sourceType, void* destination, TSampleType destinationType, UInt64 sampleCount );

    /**
     *  Makes the matrix that remixes the channels of one layout into another: matrix[ d * sourceChannels + s ] is the
     *  share of source channel s in destination channel d. Speakers that exist in both layouts are copied, the others
     *  are folded in the usual way (centre into left and right, back and side channels forward, mono to both fronts,
     *  anything to mono as the average, the low frequency channel is left out when the destination has none),
     *  and rows that would add up to more than full level are scaled back so mixing down cannot overload.
     */
    static bool BuildChannelMatrix( const CAudioFormat& source, const CAudioFormat& destination, GUCEF::vector< Float32 >& matrix );

    /** Applies a matrix from BuildChannelMatrix() to interleaved float frames */
    static void RemixChannels( const Float32* source, UInt32 sourceChannels, Float32* destination, UInt32 destinationChannels, UInt64 frameCount, const GUCEF::vector< Float32 >& matrix );

    /**
     *  Converts a buffer to another format: channels, then sample rate, then sample type, each only as far as it differs.
     *  The resampler is used for a rate change, the default is a linear one. The input is not changed.
     *  Returns false for an invalid format.
     */
    static bool Convert( const CAudioBuffer& input, const CAudioFormat& outputFormat, CAudioBuffer& output, IAudioResampler* resampler = GUCEF_NULL );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOCONVERTER_H ? */
