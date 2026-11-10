/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#include "gucefWEB_CWebSocketFrameCodec.h"
#define GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#endif /* GUCEF_WEB_CWEBSOCKETFRAMECODEC_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H
#include "gucefWEB_CWebSocketMessageParser.h"
#define GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H
#endif /* GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestWebSocketCodec.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

static bool
EnsureHashCodecPluginLoaded( void )
{GUCEF_TRACE;

    if ( WEB::CWebSocketHandshake::IsAvailable() )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH";
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
    return WEB::CWebSocketHandshake::IsAvailable();
}

/*-------------------------------------------------------------------------*/

static bool
BufferEquals( const CORE::CDynamicBuffer& buffer, const UInt8* expected, UInt32 expectedSize )
{
    return buffer.GetDataSize() == expectedSize &&
           ( 0 == expectedSize || 0 == memcmp( buffer.GetConstBufferPtr(), expected, expectedSize ) );
}

/*-------------------------------------------------------------------------*/

static bool
FeedMessage( WEB::CWebSocketMessageParser& parser, const UInt8* data, UInt32 size, WEB::CWebSocketMessageParser::TMessageVector& messages )
{
    return WEB::CWebSocketMessageParser::PARSE_OK == parser.Feed( data, size, messages );
}

/*-------------------------------------------------------------------------*/

void
PerformWebSocketCodecTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING WebSocket codec TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WebSocketCodec" );

    typedef WEB::CWebSocketFrameCodec Codec;
    typedef WEB::CWebSocketMessageParser Parser;

    // Regression (F4): the accept key was computed from the hex string of the SHA-1 instead of the raw digest
    GUCEF_TESTFW_TESTCASE( "Test: Handshake accept key matches the RFC 6455 example" )
        try
        {
            if ( !EnsureHashCodecPluginLoaded() )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "sha1-raw codec not available, handshake tests skipped" );
            }
            else
            {
                CORE::CString acceptKey;
                ASSERT_TRUE( WEB::CWebSocketHandshake::ComputeAcceptKey( "dGhlIHNhbXBsZSBub25jZQ==", acceptKey ) );
                ASSERT_TRUE( acceptKey == "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=" );

                CORE::CString keyA;
                CORE::CString keyB;
                ASSERT_TRUE( WEB::CWebSocketHandshake::GenerateClientKey( keyA ) );
                ASSERT_TRUE( WEB::CWebSocketHandshake::GenerateClientKey( keyB ) );
                ASSERT_TRUE( keyA != keyB );

                // 16 random bytes in base64 is 24 chars ending in ==
                ASSERT_TRUE( 24 == keyA.Length() );
                ASSERT_TRUE( keyA.EndsWith( "==" ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Frame codec decodes the RFC 6455 section 5.7 examples" )
        try
        {
            Codec::SFrame frame;
            UInt64 consumed = 0;

            // single frame unmasked text message
            const UInt8 unmaskedHello[] = { 0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f };
            ASSERT_TRUE( Codec::DECODE_OK == Codec::DecodeFrame( unmaskedHello, sizeof( unmaskedHello ), 1024, frame, consumed ) );
            ASSERT_TRUE( sizeof( unmaskedHello ) == consumed );
            ASSERT_TRUE( frame.fin && Codec::OPCODE_TEXT == frame.opcode && !frame.wasMasked );
            ASSERT_TRUE( BufferEquals( frame.payload, (const UInt8*) "Hello", 5 ) );

            // single frame masked text message
            const UInt8 maskedHello[] = { 0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58 };
            ASSERT_TRUE( Codec::DECODE_OK == Codec::DecodeFrame( maskedHello, sizeof( maskedHello ), 1024, frame, consumed ) );
            ASSERT_TRUE( frame.wasMasked );
            ASSERT_TRUE( BufferEquals( frame.payload, (const UInt8*) "Hello", 5 ) );

            // fragmented unmasked text message
            const UInt8 fragment1[] = { 0x01, 0x03, 0x48, 0x65, 0x6c };
            ASSERT_TRUE( Codec::DECODE_OK == Codec::DecodeFrame( fragment1, sizeof( fragment1 ), 1024, frame, consumed ) );
            ASSERT_FALSE( frame.fin );
            ASSERT_TRUE( Codec::OPCODE_TEXT == frame.opcode );
            const UInt8 fragment2[] = { 0x80, 0x02, 0x6c, 0x6f };
            ASSERT_TRUE( Codec::DECODE_OK == Codec::DecodeFrame( fragment2, sizeof( fragment2 ), 1024, frame, consumed ) );
            ASSERT_TRUE( frame.fin && Codec::OPCODE_CONTINUATION == frame.opcode );

            // ping and masked pong
            const UInt8 ping[] = { 0x89, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f };
            ASSERT_TRUE( Codec::DECODE_OK == Codec::DecodeFrame( ping, sizeof( ping ), 1024, frame, consumed ) );
            ASSERT_TRUE( Codec::OPCODE_PING == frame.opcode );
            const UInt8 maskedPong[] = { 0x8a, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58 };
            ASSERT_TRUE( Codec::DECODE_OK == Codec::DecodeFrame( maskedPong, sizeof( maskedPong ), 1024, frame, consumed ) );
            ASSERT_TRUE( Codec::OPCODE_PONG == frame.opcode && frame.wasMasked );
            ASSERT_TRUE( BufferEquals( frame.payload, (const UInt8*) "Hello", 5 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Frame codec encodes exactly like the RFC examples" )
        try
        {
            CORE::CDynamicBuffer out;
            ASSERT_TRUE( Codec::EncodeFrame( true, Codec::OPCODE_TEXT, "Hello", 5, false, out ) );
            const UInt8 expected[] = { 0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f };
            ASSERT_TRUE( BufferEquals( out, expected, sizeof( expected ) ) );

            // 256 bytes binary uses the 16 bit length form
            CORE::CDynamicBuffer payload256;
            payload256.SetDataSize( 256 );
            memset( payload256.GetBufferPtr(), 0xAB, 256 );
            out.Clear( true );
            ASSERT_TRUE( Codec::EncodeFrame( true, Codec::OPCODE_BINARY, payload256.GetConstBufferPtr(), 256, false, out ) );
            ASSERT_TRUE( 4 + 256 == out.GetDataSize() );
            ASSERT_TRUE( 0x82 == out.AsConstType< UInt8 >( 0 ) && 0x7E == out.AsConstType< UInt8 >( 1 ) );
            ASSERT_TRUE( 0x01 == out.AsConstType< UInt8 >( 2 ) && 0x00 == out.AsConstType< UInt8 >( 3 ) );

            // 64 KiB uses the 64 bit length form
            CORE::CDynamicBuffer payload64k;
            payload64k.SetDataSize( 65536 );
            memset( payload64k.GetBufferPtr(), 0xCD, 65536 );
            out.Clear( true );
            ASSERT_TRUE( Codec::EncodeFrame( true, Codec::OPCODE_BINARY, payload64k.GetConstBufferPtr(), 65536, false, out ) );
            ASSERT_TRUE( 10 + 65536 == out.GetDataSize() );
            ASSERT_TRUE( 0x7F == out.AsConstType< UInt8 >( 1 ) );
            ASSERT_TRUE( 0x01 == out.AsConstType< UInt8 >( 7 ) && 0x00 == out.AsConstType< UInt8 >( 8 ) && 0x00 == out.AsConstType< UInt8 >( 9 ) );

            // control frames are limited to 125 bytes and may not be fragmented
            UInt8 big[ 126 ];
            memset( big, 0, sizeof( big ) );
            out.Clear( true );
            ASSERT_FALSE( Codec::EncodeFrame( true, Codec::OPCODE_PING, big, 126, false, out ) );
            ASSERT_FALSE( Codec::EncodeFrame( false, Codec::OPCODE_PING, big, 10, false, out ) );
            ASSERT_FALSE( Codec::EncodeFrame( true, Codec::OPCODE_TEXT, GUCEF_NULL, 5, false, out ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Frame codec round trips with masking and all length forms" )
        try
        {
            const UInt32 sizes[] = { 0, 1, 125, 126, 127, 65535, 65536, 70000 };
            for ( UInt32 i=0; i<sizeof( sizes ) / sizeof( sizes[ 0 ] ); ++i )
            {
                CORE::CDynamicBuffer payload;
                payload.SetDataSize( sizes[ i ] );
                for ( UInt32 n=0; n<sizes[ i ]; ++n )
                    payload.AsTypePtr< UInt8 >()[ n ] = (UInt8) ( n * 7 + 3 );

                for ( UInt32 masked=0; masked<2; ++masked )
                {
                    CORE::CDynamicBuffer wire;
                    ASSERT_TRUE( Codec::EncodeFrame( true, Codec::OPCODE_BINARY, payload.GetConstBufferPtr(), sizes[ i ], 0 != masked, wire ) );

                    Codec::SFrame frame;
                    UInt64 consumed = 0;
                    ASSERT_TRUE( Codec::DECODE_OK == Codec::DecodeFrame( wire.GetConstBufferPtr(), wire.GetDataSize(), 1000000, frame, consumed ) );
                    ASSERT_TRUE( wire.GetDataSize() == consumed );
                    ASSERT_TRUE( ( 0 != masked ) == frame.wasMasked );
                    ASSERT_TRUE( BufferEquals( frame.payload, static_cast< const UInt8* >( payload.GetConstBufferPtr() ), sizes[ i ] ) );

                    // every proper prefix is an incomplete frame
                    if ( wire.GetDataSize() > 1 )
                    {
                        ASSERT_TRUE( Codec::DECODE_NEED_MORE_DATA == Codec::DecodeFrame( wire.GetConstBufferPtr(), wire.GetDataSize() - 1, 1000000, frame, consumed ) );
                        ASSERT_TRUE( 0 == consumed );
                    }
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Frame codec rejects invalid frames" )
        try
        {
            Codec::SFrame frame;
            UInt64 consumed = 0;

            const UInt8 reservedBit[] = { 0xC1, 0x00 };
            ASSERT_TRUE( Codec::DECODE_PROTOCOL_ERROR == Codec::DecodeFrame( reservedBit, sizeof( reservedBit ), 1024, frame, consumed ) );
            const UInt8 badOpcode[] = { 0x83, 0x00 };
            ASSERT_TRUE( Codec::DECODE_PROTOCOL_ERROR == Codec::DecodeFrame( badOpcode, sizeof( badOpcode ), 1024, frame, consumed ) );
            const UInt8 badControlOpcode[] = { 0x8B, 0x00 };
            ASSERT_TRUE( Codec::DECODE_PROTOCOL_ERROR == Codec::DecodeFrame( badControlOpcode, sizeof( badControlOpcode ), 1024, frame, consumed ) );
            const UInt8 fragmentedPing[] = { 0x09, 0x00 };
            ASSERT_TRUE( Codec::DECODE_PROTOCOL_ERROR == Codec::DecodeFrame( fragmentedPing, sizeof( fragmentedPing ), 1024, frame, consumed ) );
            const UInt8 bigPing[] = { 0x89, 0x7E, 0x00, 0x7E };
            ASSERT_TRUE( Codec::DECODE_PROTOCOL_ERROR == Codec::DecodeFrame( bigPing, sizeof( bigPing ), 1024, frame, consumed ) );
            const UInt8 hugeLengthMsb[] = { 0x82, 0x7F, 0x80, 0, 0, 0, 0, 0, 0, 0 };
            ASSERT_TRUE( Codec::DECODE_PROTOCOL_ERROR == Codec::DecodeFrame( hugeLengthMsb, sizeof( hugeLengthMsb ), 1024, frame, consumed ) );

            // a declared length beyond the maximum is rejected without waiting for the data
            const UInt8 tooBig[] = { 0x82, 0x7F, 0, 0, 0, 0, 0, 0x10, 0, 0 };
            ASSERT_TRUE( Codec::DECODE_TOO_BIG == Codec::DecodeFrame( tooBig, sizeof( tooBig ), 1024, frame, consumed ) );
            // a length beyond 4 GiB is rejected whatever the maximum
            const UInt8 beyond32bit[] = { 0x82, 0x7F, 0, 0, 0, 1, 0, 0, 0, 0 };
            ASSERT_TRUE( Codec::DECODE_TOO_BIG == Codec::DecodeFrame( beyond32bit, sizeof( beyond32bit ), (UInt64) 1 << 40, frame, consumed ) );

            ASSERT_TRUE( Codec::DECODE_NEED_MORE_DATA == Codec::DecodeFrame( GUCEF_NULL, 0, 1024, frame, consumed ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Close frame encoding" )
        try
        {
            CORE::CDynamicBuffer out;
            ASSERT_TRUE( Codec::EncodeCloseFrame( 1000, "bye", false, out ) );
            const UInt8 expected[] = { 0x88, 0x05, 0x03, 0xE8, 'b', 'y', 'e' };
            ASSERT_TRUE( BufferEquals( out, expected, sizeof( expected ) ) );

            out.Clear( true );
            ASSERT_TRUE( Codec::EncodeCloseFrame( 1005, "ignored", false, out ) );
            const UInt8 noCode[] = { 0x88, 0x00 };
            ASSERT_TRUE( BufferEquals( out, noCode, sizeof( noCode ) ) );

            // an over long reason is truncated so the frame stays valid
            out.Clear( true );
            CORE::CString overLongReason;
            for ( UInt32 x=0; x<200; ++x )
                overLongReason += 'x';
            ASSERT_TRUE( Codec::EncodeCloseFrame( 1000, overLongReason, false, out ) );
            ASSERT_TRUE( 2 + 125 == out.GetDataSize() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Message parser reassembles fragments and handles interleaved control frames" )
        try
        {
            Parser parser( false );
            Parser::TMessageVector messages;

            // RFC example fragmented text message with a ping in between, all in one read
            const UInt8 stream[] = { 0x01, 0x03, 0x48, 0x65, 0x6c,
                                     0x89, 0x01, 0x21,
                                     0x80, 0x02, 0x6c, 0x6f };
            ASSERT_TRUE( FeedMessage( parser, stream, sizeof( stream ), messages ) );
            ASSERT_TRUE( 2 == messages.size() );
            if ( 2 == messages.size() )
            {
                ASSERT_TRUE( Parser::MESSAGE_PING == messages[ 0 ].type );
                ASSERT_TRUE( Parser::MESSAGE_TEXT == messages[ 1 ].type );
                ASSERT_TRUE( BufferEquals( messages[ 1 ].payload, (const UInt8*) "Hello", 5 ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Message parser handles data split over reads and many frames in one read" )
        try
        {
            // build 3 messages as a client would send them
            CORE::CDynamicBuffer wire;
            ASSERT_TRUE( Codec::EncodeFrame( true, Codec::OPCODE_TEXT, "first", 5, true, wire ) );
            ASSERT_TRUE( Codec::EncodeFrame( true, Codec::OPCODE_BINARY, "\x01\x02\x03", 3, true, wire ) );
            ASSERT_TRUE( Codec::EncodeFrame( true, Codec::OPCODE_TEXT, "third", 5, true, wire ) );

            // one byte at a time
            Parser byteParser( true );
            Parser::TMessageVector messages;
            const UInt8* bytes = static_cast< const UInt8* >( wire.GetConstBufferPtr() );
            for ( UInt32 i=0; i<wire.GetDataSize(); ++i )
                ASSERT_TRUE( FeedMessage( byteParser, bytes + i, 1, messages ) );
            ASSERT_TRUE( 3 == messages.size() );

            // everything at once
            Parser allParser( true );
            Parser::TMessageVector allMessages;
            ASSERT_TRUE( FeedMessage( allParser, bytes, wire.GetDataSize(), allMessages ) );
            ASSERT_TRUE( 3 == allMessages.size() );
            if ( 3 == allMessages.size() )
            {
                ASSERT_TRUE( Parser::MESSAGE_TEXT == allMessages[ 0 ].type );
                ASSERT_TRUE( Parser::MESSAGE_BINARY == allMessages[ 1 ].type );
                ASSERT_TRUE( BufferEquals( allMessages[ 2 ].payload, (const UInt8*) "third", 5 ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Message parser enforces the masking direction" )
        try
        {
            CORE::CDynamicBuffer unmasked;
            CORE::CDynamicBuffer masked;
            ASSERT_TRUE( Codec::EncodeFrame( true, Codec::OPCODE_TEXT, "hi", 2, false, unmasked ) );
            ASSERT_TRUE( Codec::EncodeFrame( true, Codec::OPCODE_TEXT, "hi", 2, true, masked ) );

            Parser::TMessageVector messages;
            Parser serverParser( true );
            ASSERT_TRUE( Parser::PARSE_PROTOCOL_ERROR == serverParser.Feed( unmasked.GetConstBufferPtr(), unmasked.GetDataSize(), messages ) );
            ASSERT_TRUE( 1002 == serverParser.GetErrorCloseCode() );

            Parser clientParser( false );
            ASSERT_TRUE( Parser::PARSE_PROTOCOL_ERROR == clientParser.Feed( masked.GetConstBufferPtr(), masked.GetDataSize(), messages ) );

            // the failed state sticks until a reset
            ASSERT_TRUE( Parser::PARSE_PROTOCOL_ERROR == serverParser.Feed( masked.GetConstBufferPtr(), masked.GetDataSize(), messages ) );
            serverParser.Reset();
            ASSERT_TRUE( Parser::PARSE_OK == serverParser.Feed( masked.GetConstBufferPtr(), masked.GetDataSize(), messages ) );
            ASSERT_TRUE( 1 == messages.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Message parser fragmentation rules" )
        try
        {
            Parser::TMessageVector messages;

            // continuation without a start
            Parser parser1( false );
            const UInt8 orphanContinuation[] = { 0x80, 0x01, 0x41 };
            ASSERT_TRUE( Parser::PARSE_PROTOCOL_ERROR == parser1.Feed( orphanContinuation, sizeof( orphanContinuation ), messages ) );

            // a new data message while another is unfinished
            Parser parser2( false );
            const UInt8 startTwice[] = { 0x01, 0x01, 0x41, 0x01, 0x01, 0x42 };
            ASSERT_TRUE( Parser::PARSE_PROTOCOL_ERROR == parser2.Feed( startTwice, sizeof( startTwice ), messages ) );

            // binary fragmentation works too
            Parser parser3( false );
            const UInt8 binaryFragments[] = { 0x02, 0x01, 0x01, 0x00, 0x01, 0x02, 0x80, 0x01, 0x03 };
            messages.clear();
            ASSERT_TRUE( Parser::PARSE_OK == parser3.Feed( binaryFragments, sizeof( binaryFragments ), messages ) );
            ASSERT_TRUE( 1 == messages.size() );
            if ( 1 == messages.size() )
            {
                const UInt8 expected[] = { 1, 2, 3 };
                ASSERT_TRUE( Parser::MESSAGE_BINARY == messages[ 0 ].type );
                ASSERT_TRUE( BufferEquals( messages[ 0 ].payload, expected, 3 ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Message parser validates UTF-8, close frames and maximum size" )
        try
        {
            Parser::TMessageVector messages;

            // invalid UTF-8 in a text message: 0xC0 0x80 is an overlong encoding
            Parser parser1( false );
            const UInt8 badText[] = { 0x81, 0x02, 0xC0, 0x80 };
            ASSERT_TRUE( Parser::PARSE_INVALID_PAYLOAD == parser1.Feed( badText, sizeof( badText ), messages ) );
            ASSERT_TRUE( 1007 == parser1.GetErrorCloseCode() );

            // the same bytes are fine as binary
            Parser parser2( false );
            const UInt8 binary[] = { 0x82, 0x02, 0xC0, 0x80 };
            ASSERT_TRUE( Parser::PARSE_OK == parser2.Feed( binary, sizeof( binary ), messages ) );

            // message size limit, also across fragments
            Parser parser3( false, 4 );
            const UInt8 fragments[] = { 0x02, 0x03, 1, 2, 3, 0x80, 0x03, 4, 5, 6 };
            ASSERT_TRUE( Parser::PARSE_MESSAGE_TOO_BIG == parser3.Feed( fragments, sizeof( fragments ), messages ) );
            ASSERT_TRUE( 1009 == parser3.GetErrorCloseCode() );

            // close frame with code and reason
            Parser parser4( false );
            messages.clear();
            const UInt8 closeFrame[] = { 0x88, 0x05, 0x03, 0xE8, 'b', 'y', 'e' };
            ASSERT_TRUE( Parser::PARSE_OK == parser4.Feed( closeFrame, sizeof( closeFrame ), messages ) );
            ASSERT_TRUE( 1 == messages.size() );
            if ( 1 == messages.size() )
            {
                ASSERT_TRUE( Parser::MESSAGE_CLOSE == messages[ 0 ].type );
                ASSERT_TRUE( 1000 == messages[ 0 ].closeCode );
                ASSERT_TRUE( messages[ 0 ].closeReason == "bye" );
            }

            // close without payload has no code
            Parser parser5( false );
            messages.clear();
            const UInt8 emptyClose[] = { 0x88, 0x00 };
            ASSERT_TRUE( Parser::PARSE_OK == parser5.Feed( emptyClose, sizeof( emptyClose ), messages ) );
            ASSERT_TRUE( 1 == messages.size() && 1005 == messages[ 0 ].closeCode );

            // a one byte close payload and reserved close codes are protocol errors
            Parser parser6( false );
            const UInt8 oneByteClose[] = { 0x88, 0x01, 0x03 };
            ASSERT_TRUE( Parser::PARSE_PROTOCOL_ERROR == parser6.Feed( oneByteClose, sizeof( oneByteClose ), messages ) );
            Parser parser7( false );
            const UInt8 reservedCode[] = { 0x88, 0x02, 0x03, 0xED };    // 1005 must never be sent
            ASSERT_TRUE( Parser::PARSE_PROTOCOL_ERROR == parser7.Feed( reservedCode, sizeof( reservedCode ), messages ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Strict UTF-8 validation" )
        try
        {
            ASSERT_TRUE( Parser::IsValidUtf8( "", 0 ) );
            ASSERT_TRUE( Parser::IsValidUtf8( "plain ascii", 11 ) );
            ASSERT_TRUE( Parser::IsValidUtf8( "\xC3\xBB", 2 ) );                // u with circumflex
            ASSERT_TRUE( Parser::IsValidUtf8( "\xE2\x82\xAC", 3 ) );            // euro sign
            ASSERT_TRUE( Parser::IsValidUtf8( "\xF0\x9F\x98\x80", 4 ) );        // emoji
            ASSERT_FALSE( Parser::IsValidUtf8( "\xC3", 1 ) );                   // truncated
            ASSERT_FALSE( Parser::IsValidUtf8( "\xE2\x82", 2 ) );               // truncated
            ASSERT_FALSE( Parser::IsValidUtf8( "\x80", 1 ) );                   // stray continuation byte
            ASSERT_FALSE( Parser::IsValidUtf8( "\xC0\x80", 2 ) );               // overlong
            ASSERT_FALSE( Parser::IsValidUtf8( "\xE0\x80\x80", 3 ) );           // overlong
            ASSERT_FALSE( Parser::IsValidUtf8( "\xED\xA0\x80", 3 ) );           // surrogate
            ASSERT_FALSE( Parser::IsValidUtf8( "\xF4\x90\x80\x80", 4 ) );       // beyond U+10FFFF
            ASSERT_FALSE( Parser::IsValidUtf8( "\xFF", 1 ) );
            ASSERT_FALSE( Parser::IsValidUtf8( "ok\xC3", 3 ) );                 // truncated at the end of the data
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL WebSocket codec TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
