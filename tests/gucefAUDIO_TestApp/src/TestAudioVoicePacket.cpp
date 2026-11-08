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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

#ifndef GUCEF_AUDIO_CRTPHEADER_H
#include "gucefAUDIO_CRtpHeader.h"
#define GUCEF_AUDIO_CRTPHEADER_H
#endif /* GUCEF_AUDIO_CRTPHEADER_H ? */

#ifndef GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H
#include "gucefAUDIO_CRtpVoicePacketCodec.h"
#define GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H
#endif /* GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioVoicePacket.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< UInt8 > TBytes;

/*-------------------------------------------------------------------------*/

TBytes
MakeBytes( UInt32 count, UInt8 seed )
{
    TBytes bytes( count );
    for ( UInt32 i=0; i<count; ++i )
        bytes[ i ] = (UInt8) ( seed + i * 7 );
    return bytes;
}

/*-------------------------------------------------------------------------*/

AUDIO::CVoiceFrame
MakeFrame( UInt16 sequence, UInt32 timestamp, UInt32 ssrc, UInt8 payloadType, bool marker, const TBytes& payload )
{
    AUDIO::CVoiceFrame frame;
    frame.SetSequenceNumber( sequence );
    frame.SetTimestamp( timestamp );
    frame.SetStreamId( ssrc );
    frame.SetPayloadType( payloadType );
    frame.SetMarker( marker );
    frame.SetPayload( payload.empty() ? GUCEF_NULL : &payload[ 0 ], (UInt32) payload.size() );
    return frame;
}

/*-------------------------------------------------------------------------*/

bool
SameFrame( const AUDIO::CVoiceFrame& a, const AUDIO::CVoiceFrame& b )
{
    if ( a.GetStreamId() != b.GetStreamId() || a.GetSequenceNumber() != b.GetSequenceNumber() || a.GetTimestamp() != b.GetTimestamp() ||
         a.GetMarker() != b.GetMarker() || a.GetPayloadType() != b.GetPayloadType() || a.GetPayloadSize() != b.GetPayloadSize() ||
         a.GetRedundantSize() != b.GetRedundantSize() )
        return false;
    if ( 0 != a.GetPayloadSize() && 0 != memcmp( a.GetPayload(), b.GetPayload(), a.GetPayloadSize() ) )
        return false;
    if ( a.HasRedundantBlock() )
    {
        if ( a.GetRedundantTimestampOffset() != b.GetRedundantTimestampOffset() || a.GetRedundantPayloadType() != b.GetRedundantPayloadType() )
            return false;
        if ( 0 != memcmp( a.GetRedundantPayload(), b.GetRedundantPayload(), a.GetRedundantSize() ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
Append32( TBytes& bytes, UInt32 value )
{
    bytes.push_back( (UInt8) ( value >> 24 ) );
    bytes.push_back( (UInt8) ( ( value >> 16 ) & 0xFF ) );
    bytes.push_back( (UInt8) ( ( value >> 8 ) & 0xFF ) );
    bytes.push_back( (UInt8) ( value & 0xFF ) );
}

/** A header of 12 bytes by hand */
TBytes
HandHeader( UInt8 firstByte, UInt8 secondByte, UInt16 sequence, UInt32 timestamp, UInt32 ssrc )
{
    TBytes bytes;
    bytes.push_back( firstByte );
    bytes.push_back( secondByte );
    bytes.push_back( (UInt8) ( sequence >> 8 ) );
    bytes.push_back( (UInt8) ( sequence & 0xFF ) );
    Append32( bytes, timestamp );
    Append32( bytes, ssrc );
    return bytes;
}

/*-------------------------------------------------------------------------*/

void
PerformFrameTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: A voice frame holds its fields, limits its payloads and copies only what is in use" )
    try
    {
        AUDIO::CVoiceFrame frame;
        ASSERT_TRUE( 0 == frame.GetPayloadSize() && !frame.HasRedundantBlock() && !frame.GetMarker() && 0 == frame.GetCaptureTickCount() );
        ASSERT_TRUE( 1440 == AUDIO::CVoiceFrame::MaxPayloadSize );

        const TBytes big = MakeBytes( 1440, 3 );
        ASSERT_TRUE( frame.SetPayload( &big[ 0 ], 1440 ) && 1440 == frame.GetPayloadSize() );
        ASSERT_TRUE( 0 == memcmp( frame.GetPayload(), &big[ 0 ], 1440 ) );
        const TBytes tooBig = MakeBytes( 1441, 3 );
        ASSERT_FALSE( frame.SetPayload( &tooBig[ 0 ], 1441 ) );
        ASSERT_TRUE( 1440 == frame.GetPayloadSize() );
        ASSERT_FALSE( frame.SetPayloadSize( 1441 ) );
        ASSERT_TRUE( frame.SetPayloadSize( 10 ) && 10 == frame.GetPayloadSize() );

        // the redundant block: limits of RFC 2198 (14 bit offset, 10 bit length)
        const TBytes block = MakeBytes( 1023, 9 );
        ASSERT_TRUE( frame.SetRedundantBlock( 16383, 111, &block[ 0 ], 1023 ) && frame.HasRedundantBlock() );
        ASSERT_TRUE( 16383 == frame.GetRedundantTimestampOffset() && 111 == frame.GetRedundantPayloadType() && 1023 == frame.GetRedundantSize() );
        const TBytes more = MakeBytes( 1024, 9 );
        ASSERT_FALSE( frame.SetRedundantBlock( 480, 111, &more[ 0 ], 1024 ) );
        ASSERT_FALSE( frame.SetRedundantBlock( 16384, 111, &block[ 0 ], 10 ) );
        ASSERT_FALSE( frame.SetRedundantBlock( 480, 128, &block[ 0 ], 10 ) );
        ASSERT_FALSE( frame.SetRedundantBlock( 480, 111, &block[ 0 ], 0 ) );
        ASSERT_TRUE( 16383 == frame.GetRedundantTimestampOffset() && 1023 == frame.GetRedundantSize() );

        // copies carry what is in use
        frame.SetCaptureTickCount( 123456789012345ULL );
        frame.SetStreamId( 0xCAFEBABE );
        AUDIO::CVoiceFrame copy( frame );
        ASSERT_TRUE( SameFrame( frame, copy ) && 123456789012345ULL == copy.GetCaptureTickCount() );
        AUDIO::CVoiceFrame assigned;
        assigned = frame;
        ASSERT_TRUE( SameFrame( frame, assigned ) );
        frame.ClearRedundantBlock();
        ASSERT_FALSE( frame.HasRedundantBlock() );
        ASSERT_TRUE( copy.HasRedundantBlock() );
        frame.Clear();
        ASSERT_TRUE( 0 == frame.GetPayloadSize() && 0 == frame.GetStreamId() && 0 == frame.GetCaptureTickCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformHeaderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: The RTP header has the fields of RFC 3550 section 5.1 at their bit positions" )
    try
    {
        AUDIO::CRtpHeader header;
        header.SetMarker( true );
        header.SetPayloadType( 111 );
        header.SetSequenceNumber( 0xABCD );
        header.SetTimestamp( 0x01234567 );
        header.SetSsrc( 0x89ABCDEF );
        UInt8 bytes[ 12 ];
        ASSERT_TRUE( 12 == header.Write( bytes, sizeof( bytes ) ) );
        const UInt8 expected1[ 12 ] = { 0x80, 0xEF, 0xAB, 0xCD, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        ASSERT_TRUE( 0 == memcmp( bytes, expected1, 12 ) );

        header.SetMarker( false );
        header.SetPayloadType( 0 );
        header.SetSequenceNumber( 1 );
        header.SetTimestamp( 160 );
        header.SetSsrc( 1 );
        ASSERT_TRUE( 12 == header.Write( bytes, sizeof( bytes ) ) );
        const UInt8 expected2[ 12 ] = { 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x01 };
        ASSERT_TRUE( 0 == memcmp( bytes, expected2, 12 ) );

        // payload type 127 with the marker: every bit of the second byte
        header.SetMarker( true );
        header.SetPayloadType( 127 );
        ASSERT_TRUE( 12 == header.Write( bytes, sizeof( bytes ) ) && 0xFF == bytes[ 1 ] );
        header.SetPayloadType( 128 );
        ASSERT_TRUE( header.Write( bytes, sizeof( bytes ) ) < 0 );
        header.SetPayloadType( 3 );
        ASSERT_TRUE( header.Write( bytes, 11 ) < 0 );

        // the golden bytes read back
        AUDIO::CRtpHeader read;
        ASSERT_TRUE( read.Read( expected1, 12 ) );
        ASSERT_TRUE( read.GetMarker() && 111 == read.GetPayloadType() && 0xABCD == read.GetSequenceNumber() );
        ASSERT_TRUE( 0x01234567 == read.GetTimestamp() && 0x89ABCDEF == read.GetSsrc() );
        ASSERT_TRUE( !read.HasPadding() && !read.HasExtension() && 0 == read.GetCsrcCount() );
        ASSERT_TRUE( 12 == read.GetPayloadOffset() && 0 == read.GetPayloadSize() && 0 == read.GetPaddingSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: Reading skips the CSRC list, the header extension and the padding and tells where the payload is" )
    try
    {
        // CSRC count 2: V=2, P=0, X=0, CC=2 is 0x82; 8 bytes of CSRC entries, then 5 bytes of payload
        TBytes packet = HandHeader( 0x82, 0x6F, 7, 1000, 0x11223344 );
        Append32( packet, 0xAAAAAAAA );
        Append32( packet, 0xBBBBBBBB );
        const TBytes payload = MakeBytes( 5, 40 );
        packet.insert( packet.end(), payload.begin(), payload.end() );
        AUDIO::CRtpHeader header;
        ASSERT_TRUE( header.Read( &packet[ 0 ], (UInt32) packet.size() ) );
        ASSERT_TRUE( 2 == header.GetCsrcCount() && 20 == header.GetPayloadOffset() && 5 == header.GetPayloadSize() && 111 == header.GetPayloadType() );
        ASSERT_TRUE( 0 == memcmp( &packet[ header.GetPayloadOffset() ], &payload[ 0 ], 5 ) );

        // the extension: X bit (0x10), identifier 0xBEDE, length 2 words, 8 bytes of extension data
        TBytes extended = HandHeader( 0x90, 0x6F, 8, 2000, 0x11223344 );
        extended.push_back( 0xBE ); extended.push_back( 0xDE ); extended.push_back( 0x00 ); extended.push_back( 0x02 );
        Append32( extended, 0x01020304 );
        Append32( extended, 0x05060708 );
        extended.insert( extended.end(), payload.begin(), payload.end() );
        ASSERT_TRUE( header.Read( &extended[ 0 ], (UInt32) extended.size() ) );
        ASSERT_TRUE( header.HasExtension() && 0 == header.GetCsrcCount() && 24 == header.GetPayloadOffset() && 5 == header.GetPayloadSize() );
        // an extension of zero words is just its 4 bytes
        TBytes emptyExtension = HandHeader( 0x90, 0x6F, 8, 2000, 0x11223344 );
        emptyExtension.push_back( 0x12 ); emptyExtension.push_back( 0x34 ); emptyExtension.push_back( 0x00 ); emptyExtension.push_back( 0x00 );
        emptyExtension.insert( emptyExtension.end(), payload.begin(), payload.end() );
        ASSERT_TRUE( header.Read( &emptyExtension[ 0 ], (UInt32) emptyExtension.size() ) && 16 == header.GetPayloadOffset() && 5 == header.GetPayloadSize() );

        // padding: P bit (0x20), the last byte counts the padding bytes including itself
        TBytes padded = HandHeader( 0xA0, 0x6F, 9, 3000, 0x11223344 );
        padded.insert( padded.end(), payload.begin(), payload.end() );
        padded.push_back( 0 ); padded.push_back( 0 ); padded.push_back( 3 );
        ASSERT_TRUE( header.Read( &padded[ 0 ], (UInt32) padded.size() ) );
        ASSERT_TRUE( header.HasPadding() && 3 == header.GetPaddingSize() && 12 == header.GetPayloadOffset() && 5 == header.GetPayloadSize() );

        // all three together: CC=1, X, P is 0xB1
        TBytes all = HandHeader( 0xB1, 0xEF, 10, 4000, 0x11223344 );
        Append32( all, 0xCCCCCCCC );
        all.push_back( 0x00 ); all.push_back( 0x01 ); all.push_back( 0x00 ); all.push_back( 0x01 );
        Append32( all, 0x0A0B0C0D );
        all.insert( all.end(), payload.begin(), payload.end() );
        all.push_back( 2 ); all.push_back( 0 );
        all[ all.size() - 1 ] = 2;
        all[ all.size() - 2 ] = 0;
        ASSERT_TRUE( header.Read( &all[ 0 ], (UInt32) all.size() ) );
        ASSERT_TRUE( header.GetMarker() && 1 == header.GetCsrcCount() && header.HasExtension() && header.HasPadding() );
        ASSERT_TRUE( 12 + 4 + 4 + 4 == header.GetPayloadOffset() && 5 == header.GetPayloadSize() && 2 == header.GetPaddingSize() );

        // padding that takes the whole payload leaves nothing
        TBytes onlyPadding = HandHeader( 0xA0, 0x6F, 9, 3000, 0x11223344 );
        onlyPadding.push_back( 0 ); onlyPadding.push_back( 0 ); onlyPadding.push_back( 0 ); onlyPadding.push_back( 4 );
        ASSERT_TRUE( header.Read( &onlyPadding[ 0 ], (UInt32) onlyPadding.size() ) && 0 == header.GetPayloadSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: A header that is not valid is refused and leaves the object as it was" )
    try
    {
        AUDIO::CRtpHeader header;
        header.SetSequenceNumber( 4242 );
        header.SetSsrc( 99 );

        const TBytes good = HandHeader( 0x80, 0x6F, 1, 2, 3 );
        // wrong versions
        for ( UInt32 version=0; version<4; ++version )
        {
            if ( 2 == version )
                continue;
            TBytes bad = good;
            bad[ 0 ] = (UInt8) ( version << 6 );
            ASSERT_FALSE( header.Read( &bad[ 0 ], (UInt32) bad.size() ) );
        }
        // every truncation of the fixed header
        for ( UInt32 size=0; size<12; ++size )
            ASSERT_FALSE( header.Read( &good[ 0 ], size ) );

        // a CSRC list that does not fit, an extension that does not fit, padding that does not fit
        TBytes csrc = HandHeader( 0x8F, 0x6F, 1, 2, 3 );
        for ( UInt32 i=0; i<14; ++i )
            Append32( csrc, i );
        ASSERT_FALSE( header.Read( &csrc[ 0 ], (UInt32) csrc.size() ) );               // 15 entries announced, 14 there
        Append32( csrc, 14 );
        ASSERT_TRUE( header.Read( &csrc[ 0 ], (UInt32) csrc.size() ) && 15 == header.GetCsrcCount() && 72 == header.GetPayloadOffset() );
        AUDIO::CRtpHeader untouched;
        untouched.SetSequenceNumber( 4242 );
        untouched.SetSsrc( 99 );

        TBytes extension = HandHeader( 0x90, 0x6F, 1, 2, 3 );
        ASSERT_FALSE( untouched.Read( &extension[ 0 ], (UInt32) extension.size() ) );  // no extension header at all
        extension.push_back( 0 ); extension.push_back( 0 ); extension.push_back( 0xFF ); extension.push_back( 0xFF );
        ASSERT_FALSE( untouched.Read( &extension[ 0 ], (UInt32) extension.size() ) );  // 65535 words announced
        extension[ 14 ] = 0; extension[ 15 ] = 1;
        ASSERT_FALSE( untouched.Read( &extension[ 0 ], (UInt32) extension.size() ) );  // 1 word announced, none there
        Append32( extension, 0 );
        ASSERT_TRUE( untouched.Read( &extension[ 0 ], (UInt32) extension.size() ) );
        untouched = AUDIO::CRtpHeader();
        untouched.SetSequenceNumber( 4242 );
        untouched.SetSsrc( 99 );

        TBytes padding = HandHeader( 0xA0, 0x6F, 1, 2, 3 );
        ASSERT_FALSE( untouched.Read( &padding[ 0 ], (UInt32) padding.size() ) );      // the count byte is the last header byte: 3 is not a count
        padding.push_back( 0 );
        ASSERT_FALSE( untouched.Read( &padding[ 0 ], (UInt32) padding.size() ) );      // a padding count of 0
        padding[ 12 ] = 2;
        ASSERT_FALSE( untouched.Read( &padding[ 0 ], (UInt32) padding.size() ) );      // 2 bytes of padding, 1 there
        padding[ 12 ] = 1;
        ASSERT_TRUE( untouched.Read( &padding[ 0 ], (UInt32) padding.size() ) && 0 == untouched.GetPayloadSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: A failed read does not change the header" )
    try
    {
        AUDIO::CRtpHeader header;
        const TBytes first = HandHeader( 0x80, 0xEF, 0x1234, 0x01020304, 0xDEADBEEF );
        ASSERT_TRUE( header.Read( &first[ 0 ], 12 ) );
        TBytes second = first;
        second[ 0 ] = 0x40;                                    // version 1
        second[ 3 ] = 0x99;
        ASSERT_FALSE( header.Read( &second[ 0 ], 12 ) );
        ASSERT_TRUE( 0x1234 == header.GetSequenceNumber() && 0xDEADBEEF == header.GetSsrc() && header.GetMarker() && 111 == header.GetPayloadType() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformPacketTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: A frame without redundancy is a plain RTP packet with the encoded frame as its payload" )
    try
    {
        const AUDIO::CRtpVoicePacketCodec codec;
        const TBytes payload = MakeBytes( 20, 100 );
        const AUDIO::CVoiceFrame frame = MakeFrame( 0x0102, 0xA0B0C0D0, 0x11223344, 111, true, payload );
        UInt8 packet[ 2048 ];
        ASSERT_TRUE( 32 == codec.Write( frame, packet, sizeof( packet ) ) );

        const UInt8 header[ 12 ] = { 0x80, 0xEF, 0x01, 0x02, 0xA0, 0xB0, 0xC0, 0xD0, 0x11, 0x22, 0x33, 0x44 };
        ASSERT_TRUE( 0 == memcmp( packet, header, 12 ) );
        ASSERT_TRUE( 0 == memcmp( packet + 12, &payload[ 0 ], 20 ) );

        AUDIO::CVoiceFrame read;
        read.SetCaptureTickCount( 77 );
        ASSERT_TRUE( codec.Read( packet, 32, read ) );
        ASSERT_TRUE( SameFrame( frame, read ) && !read.HasRedundantBlock() && 0 == read.GetCaptureTickCount() );

        // Opus and L16 payload types are not special
        const AUDIO::CVoiceFrame pcm = MakeFrame( 5, 6, 7, 112, false, MakeBytes( 1440, 1 ) );
        ASSERT_TRUE( 1452 == codec.Write( pcm, packet, sizeof( packet ) ) );
        ASSERT_TRUE( 0x70 == packet[ 1 ] );
        AUDIO::CVoiceFrame pcmRead;
        ASSERT_TRUE( codec.Read( packet, 1452, pcmRead ) && SameFrame( pcm, pcmRead ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: Sequence numbers and timestamps wrap and stay exact across the wrap" )
    try
    {
        const AUDIO::CRtpVoicePacketCodec codec;
        UInt16 sequence = 0xFFFC;
        UInt32 timestamp = 0xFFFFFE00;
        UInt8 packet[ 256 ];
        UInt16 previousRead = 0;
        for ( UInt32 i=0; i<12; ++i )
        {
            const AUDIO::CVoiceFrame frame = MakeFrame( sequence, timestamp, 5, 111, 0 == i, MakeBytes( 10, (UInt8) i ) );
            const Int32 size = codec.Write( frame, packet, sizeof( packet ) );
            ASSERT_TRUE( 22 == size );
            ASSERT_TRUE( (UInt8) ( sequence >> 8 ) == packet[ 2 ] && (UInt8) ( sequence & 0xFF ) == packet[ 3 ] );
            AUDIO::CVoiceFrame read;
            ASSERT_TRUE( codec.Read( packet, (UInt32) size, read ) );
            ASSERT_TRUE( read.GetSequenceNumber() == sequence && read.GetTimestamp() == timestamp );
            if ( i > 0 )
                ASSERT_TRUE( (UInt16) ( read.GetSequenceNumber() - previousRead ) == 1 );
            previousRead = read.GetSequenceNumber();
            ++sequence;
            timestamp += 480;
        }
        ASSERT_TRUE( 8 == sequence && 0xFFFFFE00 + 12 * 480 == timestamp );

        // the extremes
        const AUDIO::CVoiceFrame top = MakeFrame( 0xFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 127, true, MakeBytes( 1, 1 ) );
        ASSERT_TRUE( 13 == codec.Write( top, packet, sizeof( packet ) ) );
        const UInt8 expected[ 12 ] = { 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        ASSERT_TRUE( 0 == memcmp( packet, expected, 12 ) );
        AUDIO::CVoiceFrame read;
        ASSERT_TRUE( codec.Read( packet, 13, read ) && SameFrame( top, read ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: Packets with CSRC entries, an extension or padding read as their payload" )
    try
    {
        const AUDIO::CRtpVoicePacketCodec codec;
        const TBytes payload = MakeBytes( 33, 5 );

        TBytes packet = HandHeader( 0xB2, 0x6F, 42, 43000, 0xFEEDF00D );          // P, X, CC=2
        Append32( packet, 1 );
        Append32( packet, 2 );
        packet.push_back( 0x10 ); packet.push_back( 0x00 ); packet.push_back( 0x00 ); packet.push_back( 0x01 );
        Append32( packet, 0x99999999 );
        packet.insert( packet.end(), payload.begin(), payload.end() );
        packet.push_back( 0 ); packet.push_back( 0 ); packet.push_back( 0 ); packet.push_back( 4 );

        AUDIO::CVoiceFrame frame;
        ASSERT_TRUE( codec.Read( &packet[ 0 ], (UInt32) packet.size(), frame ) );
        ASSERT_TRUE( 42 == frame.GetSequenceNumber() && 43000 == frame.GetTimestamp() && 0xFEEDF00D == frame.GetStreamId() && 111 == frame.GetPayloadType() );
        ASSERT_TRUE( 33 == frame.GetPayloadSize() && 0 == memcmp( frame.GetPayload(), &payload[ 0 ], 33 ) && !frame.GetMarker() );

        // padding that leaves no payload is not a voice frame
        TBytes empty = HandHeader( 0xA0, 0x6F, 1, 1, 1 );
        empty.push_back( 0 ); empty.push_back( 0 ); empty.push_back( 0 ); empty.push_back( 4 );
        AUDIO::CVoiceFrame keep = MakeFrame( 9, 9, 9, 9, true, MakeBytes( 3, 3 ) );
        const AUDIO::CVoiceFrame before = keep;
        ASSERT_FALSE( codec.Read( &empty[ 0 ], (UInt32) empty.size(), keep ) );
        ASSERT_TRUE( SameFrame( before, keep ) );
        // so is a packet that is only a header
        ASSERT_FALSE( codec.Read( &empty[ 0 ], 12, keep ) );
        ASSERT_TRUE( SameFrame( before, keep ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: Writing refuses what cannot be sent and a capacity that is too small, at every size" )
    try
    {
        AUDIO::CRtpVoicePacketCodec codec;
        UInt8 packet[ 2048 ];
        AUDIO::CVoiceFrame frame = MakeFrame( 1, 2, 3, 111, false, MakeBytes( 50, 1 ) );
        ASSERT_TRUE( 62 == codec.Write( frame, packet, sizeof( packet ) ) );
        for ( UInt32 capacity=0; capacity<62; ++capacity )
            ASSERT_TRUE( codec.Write( frame, packet, capacity ) < 0 );
        ASSERT_TRUE( 62 == codec.Write( frame, packet, 62 ) );

        frame.SetPayloadType( 128 );
        ASSERT_TRUE( codec.Write( frame, packet, sizeof( packet ) ) < 0 );
        ASSERT_TRUE( codec.Write( AUDIO::CVoiceFrame(), packet, sizeof( packet ) ) < 0 );     // no payload

        // with redundancy: the sizes add up, and the RED payload type must be valid
        frame = MakeFrame( 1, 2, 3, 111, false, MakeBytes( 50, 1 ) );
        const TBytes block = MakeBytes( 40, 8 );
        ASSERT_TRUE( frame.SetRedundantBlock( 480, 111, &block[ 0 ], 40 ) );
        ASSERT_TRUE( 12 + 5 + 40 + 50 == codec.Write( frame, packet, sizeof( packet ) ) );
        for ( UInt32 capacity=0; capacity<107; ++capacity )
            ASSERT_TRUE( codec.Write( frame, packet, capacity ) < 0 );
        codec.SetRedPayloadType( 200 );
        ASSERT_TRUE( codec.Write( frame, packet, sizeof( packet ) ) < 0 );
        codec.SetRedPayloadType( 113 );
        ASSERT_TRUE( 113 == codec.GetRedPayloadType() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRedundancyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: Redundancy is written as RFC 2198 and equals a hand-assembled packet" )
    try
    {
        const AUDIO::CRtpVoicePacketCodec codec;
        const TBytes primary = MakeBytes( 30, 11 );
        const TBytes redundant = MakeBytes( 25, 77 );
        AUDIO::CVoiceFrame frame = MakeFrame( 1001, 96000, 0x0A0B0C0D, 111, false, primary );
        ASSERT_TRUE( frame.SetRedundantBlock( 480, 111, &redundant[ 0 ], 25 ) );

        // by hand: the RTP header with payload type 113 (RED); the header of the redundant block: F=1 and payload type 111 (0xEF),
        // the timestamp offset 480 (0b00000111100000) in 14 bits and the length 25 (0b0000011001) in 10 bits: byte 1 is offset >> 6 = 0x07,
        // byte 2 is ( offset & 0x3F ) << 2 = 0x80 with the 2 high bits of the length (0), byte 3 is the length & 0xFF = 0x19;
        // the header of the primary block: F=0 and payload type 111 (0x6F)
        TBytes expected = HandHeader( 0x80, 113, 1001, 96000, 0x0A0B0C0D );
        expected.push_back( 0xEF );
        expected.push_back( 0x07 );
        expected.push_back( 0x80 );
        expected.push_back( 0x19 );
        expected.push_back( 0x6F );
        expected.insert( expected.end(), redundant.begin(), redundant.end() );
        expected.insert( expected.end(), primary.begin(), primary.end() );

        UInt8 packet[ 2048 ];
        const Int32 size = codec.Write( frame, packet, sizeof( packet ) );
        ASSERT_TRUE( size == (Int32) expected.size() && 12 + 5 + 25 + 30 == size );
        ASSERT_TRUE( 0 == memcmp( packet, &expected[ 0 ], expected.size() ) );

        // and the hand-assembled packet reads as the frame
        AUDIO::CVoiceFrame read;
        ASSERT_TRUE( codec.Read( &expected[ 0 ], (UInt32) expected.size(), read ) );
        ASSERT_TRUE( SameFrame( frame, read ) );
        ASSERT_TRUE( 480 == read.GetRedundantTimestampOffset() && 111 == read.GetRedundantPayloadType() && 25 == read.GetRedundantSize() );

        // the marker is on the RTP header, the payload types of the blocks are their own
        frame.SetMarker( true );
        frame.SetPayloadType( 112 );
        ASSERT_TRUE( frame.SetRedundantBlock( 960, 96, &redundant[ 0 ], 25 ) );
        ASSERT_TRUE( size == codec.Write( frame, packet, sizeof( packet ) ) );
        ASSERT_TRUE( 0xF1 == packet[ 1 ] && 0xE0 == packet[ 12 ] && 0x70 == packet[ 16 ] );
        ASSERT_TRUE( codec.Read( packet, (UInt32) size, read ) && SameFrame( frame, read ) && read.GetMarker() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: Redundancy round trips at the limits of the fields and with other RED payload types" )
    try
    {
        AUDIO::CRtpVoicePacketCodec codec;
        const UInt32 sizes[] = { 1, 2, 255, 256, 511, 512, 1022, 1023 };
        const UInt16 offsets[] = { 0, 1, 63, 64, 480, 8191, 8192, 16383 };
        const UInt32 primarySizes[] = { 1, 100, 1440 };
        UInt8 packet[ 4096 ];
        for ( UInt32 s=0; s<8; ++s )
        {
            for ( UInt32 p=0; p<3; ++p )
            {
                const TBytes primary = MakeBytes( primarySizes[ p ], (UInt8) s );
                const TBytes redundant = MakeBytes( sizes[ s ], (UInt8) ( 100 + p ) );
                AUDIO::CVoiceFrame frame = MakeFrame( (UInt16) ( 65530 + s ), 0xFFFFFF00 + s * 100, 0x80000000 + s, 100, 0 == ( s & 1 ), primary );
                ASSERT_TRUE( frame.SetRedundantBlock( offsets[ s ], (UInt8) ( 100 + s ), &redundant[ 0 ], sizes[ s ] ) );
                const Int32 size = codec.Write( frame, packet, sizeof( packet ) );
                ASSERT_TRUE( size == (Int32) ( 17 + sizes[ s ] + primarySizes[ p ] ) );
                ASSERT_TRUE( size <= (Int32) AUDIO::CRtpVoicePacketCodec::MaxPacketSize );
                AUDIO::CVoiceFrame read;
                ASSERT_TRUE( codec.Read( packet, (UInt32) size, read ) );
                ASSERT_TRUE( SameFrame( frame, read ) );
            }
        }

        // another RED payload type: a packet is redundant when its payload type is that one
        codec.SetRedPayloadType( 99 );
        const TBytes primary = MakeBytes( 10, 1 );
        const TBytes redundant = MakeBytes( 10, 2 );
        AUDIO::CVoiceFrame frame = MakeFrame( 1, 2, 3, 111, false, primary );
        ASSERT_TRUE( frame.SetRedundantBlock( 480, 111, &redundant[ 0 ], 10 ) );
        const Int32 size = codec.Write( frame, packet, sizeof( packet ) );
        ASSERT_TRUE( 99 == ( packet[ 1 ] & 0x7F ) );
        AUDIO::CVoiceFrame read;
        ASSERT_TRUE( codec.Read( packet, (UInt32) size, read ) && read.HasRedundantBlock() );
        const AUDIO::CRtpVoicePacketCodec standard;
        ASSERT_TRUE( standard.Read( packet, (UInt32) size, read ) && !read.HasRedundantBlock() && 111 != read.GetPayloadType() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: Redundancy with more than one block returns the block nearest to the primary, malformed RED is refused" )
    try
    {
        const AUDIO::CRtpVoicePacketCodec codec;
        const TBytes older = MakeBytes( 12, 1 );
        const TBytes nearer = MakeBytes( 14, 2 );
        const TBytes primary = MakeBytes( 16, 3 );

        // two redundant blocks: 2 frames back and 1 frame back
        TBytes packet = HandHeader( 0x80, 113, 50, 48000, 77 );
        packet.push_back( 0xEF ); packet.push_back( 0x0F ); packet.push_back( 0x00 ); packet.push_back( 12 );     // offset 960 (0x3C0): 0x0F, ( 0 << 2 ) | 0
        packet.push_back( 0xEF ); packet.push_back( 0x07 ); packet.push_back( 0x80 ); packet.push_back( 14 );     // offset 480: 0x07 and ( 0x20 << 2 ) = 0x80
        packet.push_back( 0x6F );
        packet.insert( packet.end(), older.begin(), older.end() );
        packet.insert( packet.end(), nearer.begin(), nearer.end() );
        packet.insert( packet.end(), primary.begin(), primary.end() );
        AUDIO::CVoiceFrame read;
        ASSERT_TRUE( codec.Read( &packet[ 0 ], (UInt32) packet.size(), read ) );
        ASSERT_TRUE( 16 == read.GetPayloadSize() && 0 == memcmp( read.GetPayload(), &primary[ 0 ], 16 ) );
        ASSERT_TRUE( 14 == read.GetRedundantSize() && 0 == memcmp( read.GetRedundantPayload(), &nearer[ 0 ], 14 ) );
        ASSERT_TRUE( 480 == read.GetRedundantTimestampOffset() );

        AUDIO::CVoiceFrame sentinel = MakeFrame( 7, 7, 7, 7, true, MakeBytes( 7, 7 ) );
        const AUDIO::CVoiceFrame kept = sentinel;

        // a block that announces more data than there is
        TBytes tooLong = HandHeader( 0x80, 113, 50, 48000, 77 );
        tooLong.push_back( 0xEF ); tooLong.push_back( 0x07 ); tooLong.push_back( 0x00 ); tooLong.push_back( 40 );
        tooLong.push_back( 0x6F );
        tooLong.insert( tooLong.end(), nearer.begin(), nearer.end() );
        ASSERT_FALSE( codec.Read( &tooLong[ 0 ], (UInt32) tooLong.size(), sentinel ) );
        // no primary data after the redundant block
        TBytes noPrimary = HandHeader( 0x80, 113, 50, 48000, 77 );
        noPrimary.push_back( 0xEF ); noPrimary.push_back( 0x07 ); noPrimary.push_back( 0x00 ); noPrimary.push_back( 14 );
        noPrimary.push_back( 0x6F );
        noPrimary.insert( noPrimary.end(), nearer.begin(), nearer.end() );
        ASSERT_FALSE( codec.Read( &noPrimary[ 0 ], (UInt32) noPrimary.size(), sentinel ) );
        // headers that never end: the F bit set to the end of the payload
        TBytes endless = HandHeader( 0x80, 113, 50, 48000, 77 );
        for ( UInt32 i=0; i<6; ++i )
        {
            endless.push_back( 0xEF ); endless.push_back( 0x07 ); endless.push_back( 0x00 ); endless.push_back( 0 );
        }
        ASSERT_FALSE( codec.Read( &endless[ 0 ], (UInt32) endless.size(), sentinel ) );
        // an empty RED payload, and a RED payload of only the primary header
        TBytes bare = HandHeader( 0x80, 113, 50, 48000, 77 );
        ASSERT_FALSE( codec.Read( &bare[ 0 ], (UInt32) bare.size(), sentinel ) );
        bare.push_back( 0x6F );
        ASSERT_FALSE( codec.Read( &bare[ 0 ], (UInt32) bare.size(), sentinel ) );
        bare.push_back( 0x55 );
        ASSERT_TRUE( codec.Read( &bare[ 0 ], (UInt32) bare.size(), sentinel ) && 1 == sentinel.GetPayloadSize() && !sentinel.HasRedundantBlock() );
        // a block header cut short
        TBytes cut = HandHeader( 0x80, 113, 50, 48000, 77 );
        cut.push_back( 0xEF ); cut.push_back( 0x07 ); cut.push_back( 0x00 );
        sentinel = kept;
        ASSERT_FALSE( codec.Read( &cut[ 0 ], (UInt32) cut.size(), sentinel ) );
        ASSERT_TRUE( SameFrame( kept, sentinel ) );
        // a redundant block of length 0 carries nothing: the frame has none
        TBytes zero = HandHeader( 0x80, 113, 50, 48000, 77 );
        zero.push_back( 0xEF ); zero.push_back( 0x07 ); zero.push_back( 0x00 ); zero.push_back( 0 );
        zero.push_back( 0x6F );
        zero.insert( zero.end(), primary.begin(), primary.end() );
        ASSERT_TRUE( codec.Read( &zero[ 0 ], (UInt32) zero.size(), sentinel ) && !sentinel.HasRedundantBlock() && 16 == sentinel.GetPayloadSize() );
        // a primary larger than a frame can be
        TBytes huge = HandHeader( 0x80, 113, 50, 48000, 77 );
        huge.push_back( 0x6F );
        const TBytes hugeData = MakeBytes( 1441, 1 );
        huge.insert( huge.end(), hugeData.begin(), hugeData.end() );
        sentinel = kept;
        ASSERT_FALSE( codec.Read( &huge[ 0 ], (UInt32) huge.size(), sentinel ) );
        ASSERT_TRUE( SameFrame( kept, sentinel ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** A valid frame, or the same frame refused: no output change then. Used on every input of the sweeps */
bool
ReadIsSane( const AUDIO::CRtpVoicePacketCodec& codec, const UInt8* data, UInt32 size )
{
    AUDIO::CVoiceFrame sentinel = MakeFrame( 0x1234, 0x9ABCDEF0, 0x0F0F0F0F, 55, true, MakeBytes( 9, 9 ) );
    const AUDIO::CVoiceFrame before = sentinel;
    const bool ok = codec.Read( data, size, sentinel );
    if ( !ok )
        return SameFrame( before, sentinel );

    // a valid frame fits its fields and the packet
    if ( 0 == sentinel.GetPayloadSize() || sentinel.GetPayloadSize() > AUDIO::CVoiceFrame::MaxPayloadSize || sentinel.GetPayloadType() > 127 )
        return false;
    if ( sentinel.GetPayloadSize() + sentinel.GetRedundantSize() > size )
        return false;
    if ( sentinel.HasRedundantBlock() && ( sentinel.GetRedundantSize() > AUDIO::CVoiceFrame::MaxRedundantSize || sentinel.GetRedundantPayloadType() > 127 ||
                                           sentinel.GetRedundantTimestampOffset() > AUDIO::CVoiceFrame::MaxRedundantTimestampOffset ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
PerformHostileInputTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: Truncation at every length returns false or a valid frame, never more than was sent" )
    try
    {
        const AUDIO::CRtpVoicePacketCodec codec;
        UInt8 packet[ 4096 ];

        AUDIO::CVoiceFrame plain = MakeFrame( 100, 200, 300, 111, true, MakeBytes( 120, 3 ) );
        Int32 size = codec.Write( plain, packet, sizeof( packet ) );
        ASSERT_TRUE( size > 0 );
        for ( Int32 length=0; length<=size; ++length )
            ASSERT_TRUE( ReadIsSane( codec, packet, (UInt32) length ) );
        // a plain packet cut inside the payload is a valid frame, shorter: the receiver of the codec rejects it, not the RTP layer
        AUDIO::CVoiceFrame read;
        ASSERT_TRUE( codec.Read( packet, 12 + 60, read ) && 60 == read.GetPayloadSize() );
        ASSERT_FALSE( codec.Read( packet, 11, read ) );
        ASSERT_FALSE( codec.Read( packet, 12, read ) );

        // the same with redundancy: every cut before the end of the redundant block's data is refused
        AUDIO::CVoiceFrame red = MakeFrame( 100, 200, 300, 111, false, MakeBytes( 80, 4 ) );
        const TBytes block = MakeBytes( 60, 5 );
        ASSERT_TRUE( red.SetRedundantBlock( 480, 111, &block[ 0 ], 60 ) );
        size = codec.Write( red, packet, sizeof( packet ) );
        ASSERT_TRUE( size == 12 + 5 + 60 + 80 );
        for ( Int32 length=0; length<=size; ++length )
            ASSERT_TRUE( ReadIsSane( codec, packet, (UInt32) length ) );
        for ( Int32 length=0; length<=12+5+60; ++length )
            ASSERT_FALSE( codec.Read( packet, (UInt32) length, read ) );
        ASSERT_TRUE( codec.Read( packet, (UInt32) size, read ) && read.HasRedundantBlock() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoicePacket: 10,000 random corruptions of valid packets return false or a valid frame (ASan and UBSan watch)" )
    try
    {
        const AUDIO::CRtpVoicePacketCodec codec;
        AudioEffectTest::CXorShift random( 20260117 );
        UInt8 packet[ 4096 ];
        UInt8 damaged[ 4096 ];

        // valid packets of several shapes
        TBytes shapes[ 4 ];
        {
            const AUDIO::CVoiceFrame plain = MakeFrame( 1, 2, 3, 111, true, MakeBytes( 100, 1 ) );
            const Int32 size = codec.Write( plain, packet, sizeof( packet ) );
            shapes[ 0 ].assign( packet, packet + size );

            AUDIO::CVoiceFrame red = MakeFrame( 4, 5, 6, 111, false, MakeBytes( 100, 2 ) );
            const TBytes block = MakeBytes( 100, 3 );
            red.SetRedundantBlock( 480, 111, &block[ 0 ], 100 );
            shapes[ 1 ].assign( packet, packet + codec.Write( red, packet, sizeof( packet ) ) );

            TBytes padded = HandHeader( 0xB1, 0x6F, 9, 9, 9 );
            Append32( padded, 1 );
            padded.push_back( 0 ); padded.push_back( 0 ); padded.push_back( 0 ); padded.push_back( 1 );
            const TBytes data = MakeBytes( 50, 6 );
            padded.insert( padded.end(), data.begin(), data.end() );
            padded.push_back( 0 ); padded.push_back( 3 );
            shapes[ 2 ] = padded;

            AUDIO::CVoiceFrame big = MakeFrame( 7, 8, 9, 112, false, MakeBytes( 1440, 7 ) );
            shapes[ 3 ].assign( packet, packet + codec.Write( big, packet, sizeof( packet ) ) );
        }

        for ( UInt32 round=0; round<10000; ++round )
        {
            const TBytes& source = shapes[ round & 3 ];
            UInt32 size = (UInt32) source.size();
            memcpy( damaged, &source[ 0 ], size );
            const UInt32 changes = 1 + random.Next() % 4;
            for ( UInt32 c=0; c<changes; ++c )
            {
                const UInt32 where = random.Next() % ( size < 40 ? size : 40 );          // the headers are where it matters
                damaged[ where ] = (UInt8) random.Next();
            }
            if ( 0 == ( random.Next() & 3 ) )
                size = random.Next() % ( size + 1 );
            ASSERT_TRUE( ReadIsSane( codec, damaged, size ) );
        }

        // and random bytes of random lengths
        for ( UInt32 round=0; round<2000; ++round )
        {
            const UInt32 size = random.Next() % 1600;
            for ( UInt32 i=0; i<size; ++i )
                damaged[ i ] = (UInt8) random.Next();
            if ( 0 != ( round & 1 ) && size > 1 )
            {
                damaged[ 0 ] = (UInt8) ( 0x80 | ( damaged[ 0 ] & 0x3F ) );               // version 2: gets past the first check
                damaged[ 1 ] = (UInt8) ( 0x71 );                                          // payload type 113: RED
            }
            ASSERT_TRUE( ReadIsSane( codec, damaged, size ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}

/*-------------------------------------------------------------------------*/

void
PerformAudioVoicePacketTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioVoicePacket TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.VoicePacket" );

    PerformFrameTests();
    PerformHeaderTests();
    PerformPacketTests();
    PerformRedundancyTests();
    PerformHostileInputTests();
}

/*-------------------------------------------------------------------------*/
