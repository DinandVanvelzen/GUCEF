/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOCODECMP3SUPPORT_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOCODECMP3SUPPORT_H

/*
 *  MP3 files made in code for the tests (there is no MP3 encoder in GUCEF): frames with a spectrum made of
 *  Huffman table 1 values, and the Info frame with the optional LAME tag. Shared by the MP3 plugin tests and the
 *  streaming tests.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestAudioCodecSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace CodecTest {
namespace Mp3 {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 FrameBytes = 208;              // 144 * 64000 / 44100, no padding
const UInt32 FrameSamples = 1152;
const UInt32 SampleRate = 44100;

/*-------------------------------------------------------------------------*/

inline void
PutBits( UInt8* data, UInt32& bitPosition, UInt32 value, UInt32 count )
{
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 bit = ( value >> ( count - 1 - i ) ) & 1;
        if ( bit )
            data[ bitPosition / 8 ] |= (UInt8) ( 0x80 >> ( bitPosition % 8 ) );
        ++bitPosition;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Writes the Huffman data of one granule of one channel at a bit position, which moves on to the end of it, and returns the number of bits it took.
 *  The values are pairs of 0 and 1, which table 1 codes: (0,0) 1, (0,1) 001, (1,0) 01, (1,1) 000, and a sign bit
 *  per value that is not zero.
 */
inline UInt32
WriteGranuleData( UInt8* data, UInt32& bitPosition, CRandom& random, UInt32 pairs )
{
    const UInt32 start = bitPosition;
    for ( UInt32 p=0; p<pairs; ++p )
    {
        const UInt32 x = random.Below( 2 );
        const UInt32 y = ( 0 == p && 0 == x ) ? 1 : random.Below( 2 );      // not all silent
        if ( 0 == x && 0 == y ) PutBits( data, bitPosition, 1, 1 );
        else if ( 0 == x && 1 == y ) PutBits( data, bitPosition, 1, 3 );
        else if ( 1 == x && 0 == y ) PutBits( data, bitPosition, 1, 2 );
        else PutBits( data, bitPosition, 0, 3 );
        if ( x ) PutBits( data, bitPosition, random.Below( 2 ), 1 );
        if ( y ) PutBits( data, bitPosition, random.Below( 2 ), 1 );
    }
    return bitPosition - start;
}

/*-------------------------------------------------------------------------*/

/** One frame with audio: lines of the spectrum, different for every frame and channel */
inline void
MakeFrame( UInt8* frame, UInt32 channels, UInt32 index, UInt32 seed )
{
    memset( frame, 0, FrameBytes );
    frame[ 0 ] = 0xFF;      // sync, MPEG 1, layer III, no CRC
    frame[ 1 ] = 0xFB;
    frame[ 2 ] = 0x50;      // 64 kbit/s, 44100 Hz, no padding
    frame[ 3 ] = 1 == channels ? 0xC0 : 0x00;

    const UInt32 sideInfoBytes = 1 == channels ? 17 : 32;
    UInt8* sideInfo = frame + 4;
    UInt8* mainData = frame + 4 + sideInfoBytes;
    CRandom random( seed * 7919 + index * 104729 + 1 );

    // the Huffman data of the four granules (two granules of every channel), one after the other
    UInt32 dataBit = 0;
    UInt32 lengths[ 4 ];
    UInt32 pairs[ 4 ];
    for ( UInt32 g=0; g<2 * channels; ++g )
    {
        pairs[ g ] = 3 + random.Below( 6 );
        lengths[ g ] = WriteGranuleData( mainData, dataBit, random, pairs[ g ] );
    }

    // the side information: main data begins in this frame, the granules have the length and values they were given
    UInt32 bit = 0;
    PutBits( sideInfo, bit, 0, 9 );                         // main_data_begin
    PutBits( sideInfo, bit, 0, 1 == channels ? 5 : 3 );     // private bits
    PutBits( sideInfo, bit, 0, 4 * channels );              // scfsi
    UInt32 g = 0;
    for ( UInt32 granule=0; granule<2; ++granule )
    {
        for ( UInt32 channel=0; channel<channels; ++channel, ++g )
        {
            PutBits( sideInfo, bit, lengths[ g ], 12 );     // part2_3_length: just the Huffman data
            PutBits( sideInfo, bit, pairs[ g ], 9 );        // big_values
            PutBits( sideInfo, bit, 190, 8 );               // global_gain: a level that leaves headroom
            PutBits( sideInfo, bit, 0, 4 );                 // scalefac_compress: no scale factors
            PutBits( sideInfo, bit, 0, 1 );                 // window_switching_flag
            PutBits( sideInfo, bit, 1, 5 );                 // table_select 0..2: table 1
            PutBits( sideInfo, bit, 1, 5 );
            PutBits( sideInfo, bit, 1, 5 );
            PutBits( sideInfo, bit, 0, 4 );                 // region0_count
            PutBits( sideInfo, bit, 0, 3 );                 // region1_count
            PutBits( sideInfo, bit, 0, 3 );                 // preflag, scalefac_scale, count1table_select
        }
    }
    (void) index;
}

/*-------------------------------------------------------------------------*/

inline void
PutBE32( UInt8* out, UInt32 value )
{
    out[ 0 ] = (UInt8) ( value >> 24 );
    out[ 1 ] = (UInt8) ( value >> 16 );
    out[ 2 ] = (UInt8) ( value >> 8 );
    out[ 3 ] = (UInt8) value;
}

/*-------------------------------------------------------------------------*/

/** The first frame of many files: a silent frame with the Info tag (the Xing tag of constant bit rate) that says how many frames there are, and for the LAME tag the delay and padding of the encoder */
inline void
MakeInfoFrame( UInt8* frame, UInt32 channels, UInt32 audioFrames, UInt32 encoderDelay, UInt32 encoderPadding, bool hasLameTag, bool isXing = false )
{
    memset( frame, 0, FrameBytes );
    frame[ 0 ] = 0xFF;
    frame[ 1 ] = 0xFB;
    frame[ 2 ] = 0x50;
    frame[ 3 ] = 1 == channels ? 0xC0 : 0x00;
    UInt8* tag = frame + 4 + ( 1 == channels ? 17 : 32 );
    memcpy( tag, isXing ? "Xing" : "Info", 4 );
    PutBE32( tag + 4, 0x00000001 );                     // flags: the frame count follows
    PutBE32( tag + 8, audioFrames );
    if ( hasLameTag )
    {
        memcpy( tag + 12, "LAME3.99r", 9 );
        UInt8* delays = tag + 12 + 21;                  // after the version, revision, lowpass, replay gain, flags and bit rate
        delays[ 0 ] = (UInt8) ( encoderDelay >> 4 );
        delays[ 1 ] = (UInt8) ( ( ( encoderDelay & 0xF ) << 4 ) | ( ( encoderPadding >> 8 ) & 0xF ) );
        delays[ 2 ] = (UInt8) ( encoderPadding & 0xFF );
    }
}

/*-------------------------------------------------------------------------*/

inline void
AppendFrames( CORE::CDynamicBuffer& file, UInt32 channels, UInt32 first, UInt32 count, UInt32 seed )
{
    UInt8 frame[ FrameBytes ];
    for ( UInt32 i=0; i<count; ++i )
    {
        MakeFrame( frame, channels, first + i, seed );
        file.Append( frame, FrameBytes );
    }
}

/*-------------------------------------------------------------------------*/

/** A stream of audio frames, with an Info frame in front when asked for */
inline void
MakeStream( CORE::CDynamicBuffer& file, UInt32 channels, UInt32 audioFrames, bool withInfo, UInt32 delay = 576, UInt32 padding = 1500, UInt32 seed = 1 )
{
    file.Clear();
    if ( withInfo )
    {
        UInt8 info[ FrameBytes ];
        MakeInfoFrame( info, channels, audioFrames, delay, padding, true );
        file.Append( info, FrameBytes );
    }
    AppendFrames( file, channels, 0, audioFrames, seed );
}

/*-------------------------------------------------------------------------*/

/** The number of frames dr_mp3 gives for a stream with an Info frame: whole MPEG frames, less the delay and padding it takes from the LAME tag */
inline UInt64
LengthWithInfo( UInt32 audioFrames, UInt32 delay, UInt32 padding )
{
    return (UInt64) audioFrames * FrameSamples - ( delay + 529 ) - ( padding - 529 );
}

/*-------------------------------------------------------------------------*/

}; /* namespace Mp3 */
}; /* namespace CodecTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOCODECMP3SUPPORT_H ? */
