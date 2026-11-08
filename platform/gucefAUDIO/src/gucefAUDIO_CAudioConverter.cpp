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

#include <string.h>
#include <math.h>

#ifndef GUCEF_AUDIO_CAUDIOBUFFER_H
#include "gucefAUDIO_CAudioBuffer.h"
#define GUCEF_AUDIO_CAUDIOBUFFER_H
#endif /* GUCEF_AUDIO_CAUDIOBUFFER_H ? */

#ifndef GUCEF_AUDIO_CLINEARRESAMPLER_H
#include "gucefAUDIO_CLinearResampler.h"
#define GUCEF_AUDIO_CLINEARRESAMPLER_H
#endif /* GUCEF_AUDIO_CLINEARRESAMPLER_H ? */

#include "gucefAUDIO_CAudioConverter.h"

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

namespace {

const Float32 SqrtHalf = 0.70710678f;

/** Float to integer with rounding to nearest and limiting to the range, for a type with 'bits' bits */
inline Int64
FloatToInteger( Float64 value, Float64 scale, Int64 minValue, Int64 maxValue )
{
    if ( value != value )               // not a number: silence
        return 0;
    const Float64 scaled = value * scale;
    if ( scaled >= (Float64) maxValue )
        return maxValue;
    if ( scaled <= (Float64) minValue )
        return minValue;
    return (Int64) floor( scaled + 0.5 );
}

inline Int32
ReadInt24( const UInt8* p )
{
    Int32 v = (Int32) ( (UInt32) p[ 0 ] | ( (UInt32) p[ 1 ] << 8 ) | ( (UInt32) p[ 2 ] << 16 ) );
    if ( v & 0x800000 )
        v -= 0x1000000;
    return v;
}

inline void
WriteInt24( UInt8* p, Int32 v )
{
    p[ 0 ] = (UInt8) ( v & 0xFF );
    p[ 1 ] = (UInt8) ( ( v >> 8 ) & 0xFF );
    p[ 2 ] = (UInt8) ( ( v >> 16 ) & 0xFF );
}

struct SSpeakers
{
    GUCEF::vector< UInt32 > positions;      // the speaker of each channel in channel order

    explicit SSpeakers( const CAudioFormat& format )
    {
        const UInt32 layout = format.GetChannelLayout();
        for ( UInt32 bit=0; bit<32; ++bit )
            if ( layout & ( 1U << bit ) )
                positions.push_back( 1U << bit );
    }

    Int32 IndexOf( UInt32 speaker ) const
    {
        for ( size_t i=0; i<positions.size(); ++i )
            if ( positions[ i ] == speaker )
                return (Int32) i;
        return -1;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAudioConverter::ConvertToFloat32( const void* source, TSampleType sourceType, Float32* destination, UInt64 sampleCount )
{
    // no trace macro: this runs where the audio is mixed

    switch ( sourceType )
    {
        case SAMPLETYPE_UINT8 :
        {
            const UInt8* s = static_cast< const UInt8* >( source );
            for ( UInt64 i=0; i<sampleCount; ++i )
                destination[ i ] = (Float32) ( (Int32) s[ i ] - 128 ) / 128.0f;
            break;
        }
        case SAMPLETYPE_INT16 :
        {
            const UInt8* s = static_cast< const UInt8* >( source );
            for ( UInt64 i=0; i<sampleCount; ++i )
            {
                Int16 v;
                memcpy( &v, s + i * 2, 2 );
                destination[ i ] = (Float32) v / 32768.0f;
            }
            break;
        }
        case SAMPLETYPE_INT24 :
        {
            const UInt8* s = static_cast< const UInt8* >( source );
            for ( UInt64 i=0; i<sampleCount; ++i )
                destination[ i ] = (Float32) ( (Float64) ReadInt24( s + i * 3 ) / 8388608.0 );
            break;
        }
        case SAMPLETYPE_INT32 :
        {
            const UInt8* s = static_cast< const UInt8* >( source );
            for ( UInt64 i=0; i<sampleCount; ++i )
            {
                Int32 v;
                memcpy( &v, s + i * 4, 4 );
                destination[ i ] = (Float32) ( (Float64) v / 2147483648.0 );
            }
            break;
        }
        case SAMPLETYPE_FLOAT32 :
        {
            if ( static_cast< const void* >( destination ) != source )
                memcpy( destination, source, (size_t) sampleCount * 4 );
            break;
        }
        case SAMPLETYPE_FLOAT64 :
        {
            const UInt8* s = static_cast< const UInt8* >( source );
            for ( UInt64 i=0; i<sampleCount; ++i )
            {
                Float64 v;
                memcpy( &v, s + i * 8, 8 );
                destination[ i ] = (Float32) v;
            }
            break;
        }
        default :
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioConverter::ConvertFromFloat32( const Float32* source, void* destination, TSampleType destinationType, UInt64 sampleCount )
{
    // no trace macro: this runs where the audio is mixed

    switch ( destinationType )
    {
        case SAMPLETYPE_UINT8 :
        {
            UInt8* d = static_cast< UInt8* >( destination );
            for ( UInt64 i=0; i<sampleCount; ++i )
                d[ i ] = (UInt8) ( FloatToInteger( source[ i ], 128.0, -128, 127 ) + 128 );
            break;
        }
        case SAMPLETYPE_INT16 :
        {
            UInt8* d = static_cast< UInt8* >( destination );
            for ( UInt64 i=0; i<sampleCount; ++i )
            {
                const Int16 v = (Int16) FloatToInteger( source[ i ], 32768.0, -32768, 32767 );
                memcpy( d + i * 2, &v, 2 );
            }
            break;
        }
        case SAMPLETYPE_INT24 :
        {
            UInt8* d = static_cast< UInt8* >( destination );
            for ( UInt64 i=0; i<sampleCount; ++i )
                WriteInt24( d + i * 3, (Int32) FloatToInteger( source[ i ], 8388608.0, -8388608, 8388607 ) );
            break;
        }
        case SAMPLETYPE_INT32 :
        {
            UInt8* d = static_cast< UInt8* >( destination );
            for ( UInt64 i=0; i<sampleCount; ++i )
            {
                const Int32 v = (Int32) FloatToInteger( source[ i ], 2147483648.0, -2147483647LL - 1, 2147483647LL );
                memcpy( d + i * 4, &v, 4 );
            }
            break;
        }
        case SAMPLETYPE_FLOAT32 :
        {
            if ( static_cast< const void* >( source ) != destination )
                memcpy( destination, source, (size_t) sampleCount * 4 );
            break;
        }
        case SAMPLETYPE_FLOAT64 :
        {
            UInt8* d = static_cast< UInt8* >( destination );
            for ( UInt64 i=0; i<sampleCount; ++i )
            {
                const Float64 v = (Float64) source[ i ];
                memcpy( d + i * 8, &v, 8 );
            }
            break;
        }
        default :
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioConverter::ConvertSamples( const void* source, TSampleType sourceType, void* destination, TSampleType destinationType, UInt64 sampleCount )
{
    // no trace macro: this runs where the audio is mixed

    if ( sourceType == destinationType )
    {
        if ( source != destination )
            memmove( destination, source, (size_t) ( sampleCount * CAudioFormat::GetBytesPerSample( sourceType ) ) );
        return;
    }

    // Work in blocks so no large temporary is needed
    const UInt64 blockSize = 4096;
    Float32 block[ 4096 ];
    const UInt32 sourceBytes = CAudioFormat::GetBytesPerSample( sourceType );
    const UInt32 destinationBytes = CAudioFormat::GetBytesPerSample( destinationType );
    if ( 0 == sourceBytes || 0 == destinationBytes )
        return;

    const UInt8* in = static_cast< const UInt8* >( source );
    UInt8* out = static_cast< UInt8* >( destination );
    UInt64 done = 0;
    while ( done < sampleCount )
    {
        const UInt64 n = ( sampleCount - done < blockSize ) ? ( sampleCount - done ) : blockSize;
        ConvertToFloat32( in + done * sourceBytes, sourceType, block, n );
        ConvertFromFloat32( block, out + done * destinationBytes, destinationType, n );
        done += n;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioConverter::BuildChannelMatrix( const CAudioFormat& source, const CAudioFormat& destination, GUCEF::vector< Float32 >& matrix )
{GUCEF_TRACE;

    const UInt32 s = source.GetChannelCount();
    const UInt32 d = destination.GetChannelCount();
    matrix.assign( (size_t) s * d, 0.0f );
    if ( 0 == s || 0 == d )
        return false;

    #define MATRIX( dst, src ) matrix[ (size_t) (dst) * s + (src) ]

    const SSpeakers from( source );
    const SSpeakers to( destination );
    if ( from.positions.size() != s || to.positions.size() != d )
    {
        // Layout masks that do not match the channel counts: the channels in order, as far as both have them
        for ( UInt32 c=0; c<s && c<d; ++c )
            MATRIX( c, c ) = 1.0f;
        return true;
    }

    // Identical layouts are a plain copy
    if ( from.positions == to.positions )
    {
        for ( UInt32 c=0; c<s; ++c )
            MATRIX( c, c ) = 1.0f;
        return true;
    }

    const Int32 dFL = to.IndexOf( AUDIOSPEAKER_FRONT_LEFT ), dFR = to.IndexOf( AUDIOSPEAKER_FRONT_RIGHT ), dFC = to.IndexOf( AUDIOSPEAKER_FRONT_CENTER );

    // Mixing down to one channel: the average of everything that is not the low frequency effects channel
    if ( 1 == d )
    {
        UInt32 contributing = 0;
        for ( UInt32 c=0; c<s; ++c )
            if ( AUDIOSPEAKER_LOW_FREQUENCY != from.positions[ c ] )
                ++contributing;
        if ( 0 == contributing )
            contributing = s;
        for ( UInt32 c=0; c<s; ++c )
            if ( AUDIOSPEAKER_LOW_FREQUENCY != from.positions[ c ] || contributing == s )
                MATRIX( 0, c ) = 1.0f / (Float32) contributing;
        return true;
    }

    // Mono (a single centre channel or a single channel of any other position) goes to both front speakers
    if ( 1 == s && dFL >= 0 && dFR >= 0 && dFC < 0 )
    {
        MATRIX( dFL, 0 ) = 1.0f;
        MATRIX( dFR, 0 ) = 1.0f;
        return true;
    }

    // Speakers present on both sides are copied straight
    for ( UInt32 c=0; c<s; ++c )
    {
        const Int32 target = to.IndexOf( from.positions[ c ] );
        if ( target >= 0 )
            MATRIX( target, c ) = 1.0f;
    }

    // Speakers that the destination lacks are folded into the ones it has
    for ( UInt32 c=0; c<s; ++c )
    {
        const UInt32 speaker = from.positions[ c ];
        if ( to.IndexOf( speaker ) >= 0 )
            continue;

        switch ( speaker )
        {
            case AUDIOSPEAKER_FRONT_CENTER :
            {
                if ( dFL >= 0 ) MATRIX( dFL, c ) += SqrtHalf;
                if ( dFR >= 0 ) MATRIX( dFR, c ) += SqrtHalf;
                break;
            }
            case AUDIOSPEAKER_BACK_LEFT :
            case AUDIOSPEAKER_SIDE_LEFT :
            case AUDIOSPEAKER_FRONT_LEFT_OF_CENTER :
            {
                // back and side channels stand in for each other when only one of the pair exists
                const Int32 sideTarget = ( AUDIOSPEAKER_BACK_LEFT == speaker ) ? to.IndexOf( AUDIOSPEAKER_SIDE_LEFT ) : ( ( AUDIOSPEAKER_SIDE_LEFT == speaker ) ? to.IndexOf( AUDIOSPEAKER_BACK_LEFT ) : -1 );
                if ( sideTarget >= 0 )
                    MATRIX( sideTarget, c ) += 1.0f;
                else if ( dFL >= 0 )
                    MATRIX( dFL, c ) += SqrtHalf;
                break;
            }
            case AUDIOSPEAKER_BACK_RIGHT :
            case AUDIOSPEAKER_SIDE_RIGHT :
            case AUDIOSPEAKER_FRONT_RIGHT_OF_CENTER :
            {
                const Int32 sideTarget = ( AUDIOSPEAKER_BACK_RIGHT == speaker ) ? to.IndexOf( AUDIOSPEAKER_SIDE_RIGHT ) : ( ( AUDIOSPEAKER_SIDE_RIGHT == speaker ) ? to.IndexOf( AUDIOSPEAKER_BACK_RIGHT ) : -1 );
                if ( sideTarget >= 0 )
                    MATRIX( sideTarget, c ) += 1.0f;
                else if ( dFR >= 0 )
                    MATRIX( dFR, c ) += SqrtHalf;
                break;
            }
            case AUDIOSPEAKER_BACK_CENTER :
            {
                const Int32 bl = to.IndexOf( AUDIOSPEAKER_BACK_LEFT ), br = to.IndexOf( AUDIOSPEAKER_BACK_RIGHT );
                if ( bl >= 0 && br >= 0 )
                {
                    MATRIX( bl, c ) += SqrtHalf;
                    MATRIX( br, c ) += SqrtHalf;
                }
                else
                {
                    if ( dFL >= 0 ) MATRIX( dFL, c ) += 0.5f;
                    if ( dFR >= 0 ) MATRIX( dFR, c ) += 0.5f;
                }
                break;
            }
            default :
                // The low frequency channel and the height channels are left out of a mix to a layout without them
                break;
        }
    }

    // Rows must not add up to more than full level or a loud mix could overload
    for ( UInt32 row=0; row<d; ++row )
    {
        Float32 sum = 0.0f;
        for ( UInt32 c=0; c<s; ++c )
            sum += MATRIX( row, c );
        if ( sum > 1.0f )
            for ( UInt32 c=0; c<s; ++c )
                MATRIX( row, c ) /= sum;
    }

    #undef MATRIX
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioConverter::RemixChannels( const Float32* source, UInt32 sourceChannels, Float32* destination, UInt32 destinationChannels, UInt64 frameCount, const GUCEF::vector< Float32 >& matrix )
{
    // no trace macro: this runs where the audio is mixed

    for ( UInt64 f=0; f<frameCount; ++f )
    {
        const Float32* in = source + f * sourceChannels;
        Float32* out = destination + f * destinationChannels;
        for ( UInt32 d=0; d<destinationChannels; ++d )
        {
            Float32 sum = 0.0f;
            const Float32* row = &matrix[ (size_t) d * sourceChannels ];
            for ( UInt32 s=0; s<sourceChannels; ++s )
                sum += row[ s ] * in[ s ];
            out[ d ] = sum;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioConverter::Convert( const CAudioBuffer& input, const CAudioFormat& outputFormat, CAudioBuffer& output, IAudioResampler* resampler )
{GUCEF_TRACE;

    const CAudioFormat& inputFormat = input.GetFormat();
    if ( !inputFormat.IsValid() || !outputFormat.IsValid() )
        return false;

    const UInt64 inputFrames = input.GetFrameCount();
    const UInt32 inChannels = inputFormat.GetChannelCount();
    const UInt32 outChannels = outputFormat.GetChannelCount();

    // 1. to float
    GUCEF::vector< Float32 > work( (size_t) ( inputFrames * inChannels ) );
    if ( inputFrames > 0 )
        ConvertToFloat32( input.GetDataPtr(), inputFormat.GetSampleType(), &work[ 0 ], inputFrames * inChannels );

    // 2. channels
    if ( inputFormat.GetChannelLayout() != outputFormat.GetChannelLayout() || inChannels != outChannels )
    {
        GUCEF::vector< Float32 > matrix;
        if ( !BuildChannelMatrix( inputFormat, outputFormat, matrix ) )
            return false;
        GUCEF::vector< Float32 > remixed( (size_t) ( inputFrames * outChannels ) );
        if ( inputFrames > 0 )
            RemixChannels( &work[ 0 ], inChannels, &remixed[ 0 ], outChannels, inputFrames, matrix );
        work.swap( remixed );
    }

    // 3. sample rate
    UInt64 outputFrames = inputFrames;
    if ( inputFormat.GetSampleRate() != outputFormat.GetSampleRate() && inputFrames > 0 )
    {
        CLinearResampler defaultResampler;
        IAudioResampler* activeResampler = ( GUCEF_NULL != resampler ) ? resampler : &defaultResampler;
        if ( !activeResampler->Configure( outChannels, (Float64) inputFormat.GetSampleRate(), (Float64) outputFormat.GetSampleRate() ) )
            return false;
        activeResampler->Reset();

        outputFrames = (UInt64) floor( (Float64) inputFrames * (Float64) outputFormat.GetSampleRate() / (Float64) inputFormat.GetSampleRate() + 0.5 );

        // One frame that holds the last value lets the resampler finish the tail of the audio
        GUCEF::vector< Float32 > padded( work );
        padded.insert( padded.end(), &work[ ( inputFrames - 1 ) * outChannels ], &work[ ( inputFrames - 1 ) * outChannels ] + outChannels );

        GUCEF::vector< Float32 > resampled( (size_t) ( ( outputFrames + 8 ) * outChannels ) );
        UInt32 consumed = 0, produced = 0;
        if ( padded.size() / outChannels > 0xFFFFFFFFULL || resampled.size() / outChannels > 0xFFFFFFFFULL )
            return false;
        activeResampler->Process( &padded[ 0 ], (UInt32) ( padded.size() / outChannels ), consumed, &resampled[ 0 ], (UInt32) ( resampled.size() / outChannels ), produced );
        if ( produced < outputFrames )
        {
            // keep the last value for any frames the resampler could not make
            for ( UInt64 f=produced; f<outputFrames; ++f )
                for ( UInt32 c=0; c<outChannels; ++c )
                    resampled[ (size_t) ( f * outChannels + c ) ] = ( produced > 0 ) ? resampled[ (size_t) ( ( produced - 1 ) * outChannels + c ) ] : 0.0f;
        }
        resampled.resize( (size_t) ( outputFrames * outChannels ) );
        work.swap( resampled );
    }

    // 4. to the output sample type
    if ( outputFrames > 0xFFFFFFFFULL )
        return false;
    output = CAudioBuffer( outputFormat, (UInt32) outputFrames );
    if ( outputFrames > 0 )
        ConvertFromFloat32( &work[ 0 ], output.GetDataPtr(), outputFormat.GetSampleType(), outputFrames * outChannels );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
