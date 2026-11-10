/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNAllocationCounter.h"
#include "TestDRNWire.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

static CORE::CString
ToHex( const UInt8* data, UInt32 size )
{
    static const char digits[] = "0123456789abcdef";
    CORE::CString hex;
    for ( UInt32 i=0; i<size; ++i )
    {
        char pair[ 3 ] = { digits[ data[ i ] >> 4 ], digits[ data[ i ] & 0xF ], 0 };
        hex += pair;
    }
    return hex;
}

/*-------------------------------------------------------------------------*/

static void
FillPattern( CORE::CDynamicBuffer& buffer, UInt32 size, UInt32 seed )
{
    buffer.SetDataSize( size );
    if ( 0 == size )
        return;
    UInt8* p = static_cast< UInt8* >( buffer.GetBufferPtr() );
    for ( UInt32 i=0; i<size; ++i )
        p[ i ] = (UInt8) ( i * 131 + seed * 7 + ( i >> 8 ) );
}

/*-------------------------------------------------------------------------*/

/** Writes a variant with the codec and reads it back, size gets the bytes it took */
static bool
CodecRoundTrip( const CORE::CVariant& value, CORE::CVariant& result, UInt32& size, const DRN::CDRNQuantisation* quantisation = GUCEF_NULL )
{
    CORE::CDynamicBuffer buffer;
    DRN::CDRNWireWriter writer( buffer );
    DRN::CDRNVariantCodec::Write( writer, value, quantisation );
    size = writer.GetOffset();
    DRN::CDRNWireReader reader( buffer.GetConstBufferPtr(), size );
    bool removed = true;
    return DRN::CDRNVariantCodec::Read( reader, result, &removed ) && !removed && 0 == reader.GetRemaining();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNWireTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Wire" );

    GUCEF_TESTFW_TESTCASE( "Wire: Varints round trip at every byte length boundary, signed values zig-zag" )
    try
    {
        const UInt64 values[] = { 0, 1, 127, 128, 255, 16383, 16384, 2097151, 2097152, 268435455, 268435456,
                                  0xFFFFFFFFULL, 0x100000000ULL, 0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL };
        const UInt32 expectedSizes[] = { 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 5, 9, 10 };
        const UInt32 count = sizeof( values ) / sizeof( values[ 0 ] );

        CORE::CDynamicBuffer buffer;
        DRN::CDRNWireWriter writer( buffer );
        for ( UInt32 i=0; i<count; ++i )
        {
            UInt32 before = writer.GetOffset();
            writer.WriteVarUInt64( values[ i ] );
            ASSERT_TRUE( expectedSizes[ i ] == writer.GetOffset() - before );
            ASSERT_TRUE( expectedSizes[ i ] == DRN::CDRNWireWriter::GetVarUInt64Size( values[ i ] ) );
        }
        const Int64 signedValues[] = { 0, -1, 1, -64, 64, -65, 2147483647LL, -2147483647LL - 1, 0x7FFFFFFFFFFFFFFFLL, -0x7FFFFFFFFFFFFFFFLL - 1 };
        const UInt32 signedCount = sizeof( signedValues ) / sizeof( signedValues[ 0 ] );
        for ( UInt32 i=0; i<signedCount; ++i )
            writer.WriteVarInt64( signedValues[ i ] );
        writer.WriteVarInt32( -1 );
        writer.WriteVarUInt32( 0xFFFFFFFF );
        ASSERT_TRUE( buffer.GetDataSize() == writer.GetOffset() );

        DRN::CDRNWireReader reader( buffer );
        for ( UInt32 i=0; i<count; ++i )
        {
            UInt64 value = 1;
            ASSERT_TRUE( reader.ReadVarUInt64( value ) );
            ASSERT_TRUE( values[ i ] == value );
        }
        for ( UInt32 i=0; i<signedCount; ++i )
        {
            Int64 value = 1;
            ASSERT_TRUE( reader.ReadVarInt64( value ) );
            ASSERT_TRUE( signedValues[ i ] == value );
        }
        Int32 minusOne = 0;
        UInt32 maxUInt32 = 0;
        ASSERT_TRUE( reader.ReadVarInt32( minusOne ) && -1 == minusOne );
        ASSERT_TRUE( reader.ReadVarUInt32( maxUInt32 ) && 0xFFFFFFFF == maxUInt32 );
        ASSERT_TRUE( 0 == reader.GetRemaining() );
        ASSERT_FALSE( reader.HasFailed() );

        // small zig-zag values stay small
        ASSERT_TRUE( 1 == DRN::CDRNWireWriter::ZigZagEncode32( -1 ) && 2 == DRN::CDRNWireWriter::ZigZagEncode32( 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wire: Fixed size integers are big endian, floats, strings and byte blocks round trip" )
    try
    {
        CORE::CDynamicBuffer buffer;
        DRN::CDRNWireWriter writer( buffer );
        writer.WriteUInt8( 0xAB );
        writer.WriteUInt16( 0x1234 );
        writer.WriteUInt32( 0x89ABCDEF );
        writer.WriteUInt64( 0x0102030405060708ULL );
        writer.WriteFloat32( 1.5f );
        writer.WriteFloat64( -2.25 );
        writer.WriteBool( true );
        writer.WriteString( "h\xC3\xA9llo" );
        writer.WriteString( CORE::CString::Empty );
        const UInt8 blob[] = { 0, 1, 2, 0xFF };
        writer.WriteSizedBytes( blob, sizeof( blob ) );

        const UInt8* raw = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
        ASSERT_TRUE( 0xAB == raw[ 0 ] && 0x12 == raw[ 1 ] && 0x34 == raw[ 2 ] );
        ASSERT_TRUE( 0x89 == raw[ 3 ] && 0xEF == raw[ 6 ] );
        ASSERT_TRUE( 0x01 == raw[ 7 ] && 0x08 == raw[ 14 ] );

        DRN::CDRNWireReader reader( buffer );
        UInt8 u8 = 0; UInt16 u16 = 0; UInt32 u32 = 0; UInt64 u64 = 0; Float32 f32 = 0; Float64 f64 = 0; bool b = false;
        CORE::CString text, empty( "x" );
        const UInt8* blobView = GUCEF_NULL;
        UInt32 blobSize = 0;
        ASSERT_TRUE( reader.ReadUInt8( u8 ) && 0xAB == u8 );
        ASSERT_TRUE( reader.ReadUInt16( u16 ) && 0x1234 == u16 );
        ASSERT_TRUE( reader.ReadUInt32( u32 ) && 0x89ABCDEF == u32 );
        ASSERT_TRUE( reader.ReadUInt64( u64 ) && 0x0102030405060708ULL == u64 );
        ASSERT_TRUE( reader.ReadFloat32( f32 ) && 1.5f == f32 );
        ASSERT_TRUE( reader.ReadFloat64( f64 ) && -2.25 == f64 );
        ASSERT_TRUE( reader.ReadBool( b ) && b );
        ASSERT_TRUE( reader.ReadString( text ) && text == "h\xC3\xA9llo" );
        ASSERT_TRUE( reader.ReadString( empty ) && empty.IsNULLOrEmpty() );
        ASSERT_TRUE( reader.ReadSizedBytes( blobView, blobSize ) && sizeof( blob ) == blobSize && 0 == memcmp( blob, blobView, blobSize ) );
        ASSERT_TRUE( 0 == reader.GetRemaining() );

        // reading past the end fails and keeps failing
        ASSERT_FALSE( reader.ReadUInt8( u8 ) );
        ASSERT_TRUE( reader.HasFailed() );
        UInt32 dummy = 0;
        ASSERT_FALSE( reader.ReadVarUInt32( dummy ) );

        // a truncated varint fails, a sized block longer than the data fails
        const UInt8 truncated[] = { 0x80, 0x80 };
        DRN::CDRNWireReader truncatedReader( truncated, sizeof( truncated ) );
        ASSERT_FALSE( truncatedReader.ReadVarUInt32( dummy ) );
        const UInt8 tooLong[] = { 0x05, 0x01, 0x02 };
        DRN::CDRNWireReader tooLongReader( tooLong, sizeof( tooLong ) );
        ASSERT_FALSE( tooLongReader.ReadSizedBytes( blobView, blobSize ) );
        // more than 32 bits for a 32 bit varint fails
        const UInt8 wide[] = { 0x80, 0x80, 0x80, 0x80, 0x10 };
        DRN::CDRNWireReader wideReader( wide, sizeof( wide ) );
        ASSERT_FALSE( wideReader.ReadVarUInt32( dummy ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wire: The writer does not allocate once its buffer is large enough" )
    try
    {
        CORE::CDynamicBuffer buffer;
        buffer.SetBufferSize( 4096, false );
        const void* before = buffer.GetConstBufferPtr();
        for ( UInt32 round=0; round<100; ++round )
        {
            DRN::CDRNWireWriter writer( buffer );
            for ( UInt32 i=0; i<200; ++i )
                writer.WriteVarUInt32( i * 1000 );
            ASSERT_TRUE( buffer.GetConstBufferPtr() == before );
            buffer.SetDataSize( 0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wire: Variants round trip through the codec with their type, numbers compact and quantised when asked, without allocating" )
    try
    {
        // every type comes back as itself
        CORE::CVariant values[ 15 ];
        values[ 0 ] = CORE::CVariant( (UInt8) 200 );
        values[ 1 ] = CORE::CVariant( (UInt16) 60000 );
        values[ 2 ] = CORE::CVariant( (UInt32) 4000000000u );
        values[ 3 ] = CORE::CVariant( (UInt64) 1000000000000000000ull );
        values[ 4 ] = CORE::CVariant( (Int8) -100 );
        values[ 5 ] = CORE::CVariant( (Int16) -30000 );
        values[ 6 ] = CORE::CVariant( (Int32) -2000000000 );
        values[ 7 ] = CORE::CVariant( (Int64) -9000000000000000000ll );
        values[ 8 ] = CORE::CVariant( (Float32) 3.25f );
        values[ 9 ] = CORE::CVariant( (Float64) -1.0e100 );
        values[ 10 ] = CORE::CVariant( true );
        values[ 11 ] = CORE::CVariant( "plain text" );
        values[ 12 ] = CORE::CVariant( CORE::CString( "utf8 text" ) );
        const UInt8 blob[ 5 ] = { 0, 1, 2, 254, 255 };
        values[ 13 ].Set( blob, sizeof( blob ), GUCEF_DATATYPE_BINARY_BLOB );
        // values[ 14 ] stays an empty variant
        for ( UInt32 i=0; i<15; ++i )
        {
            CORE::CVariant result;
            UInt32 size = 0;
            ASSERT_TRUE( CodecRoundTrip( values[ i ], result, size ) );
            ASSERT_TRUE( result.GetTypeId() == values[ i ].GetTypeId() );
            ASSERT_TRUE( DRN::CDRNVariantCodec::IsSame( values[ i ], result ) );
        }

        // small numbers are small: a tag and a varint
        CORE::CVariant result;
        UInt32 size = 0;
        ASSERT_TRUE( CodecRoundTrip( CORE::CVariant( (UInt32) 5 ), result, size ) && 2 == size );
        ASSERT_TRUE( CodecRoundTrip( CORE::CVariant( (Int32) -1 ), result, size ) && 2 == size && -1 == result.AsInt32() );

        // quantised floats: the decimal places that were asked for, fewer bytes
        DRN::CDRNQuantisation twoDecimals( 2, 0 );
        ASSERT_TRUE( CodecRoundTrip( CORE::CVariant( (Float32) 12.3456f ), result, size, &twoDecimals ) );
        ASSERT_TRUE( GUCEF_DATATYPE_FLOAT32 == result.GetTypeId() && result.AsFloat32() > 12.349f && result.AsFloat32() < 12.351f && size <= 4 );
        CORE::CVariant quantised( (Float32) 12.3456f );
        DRN::CDRNVariantCodec::Quantise( quantised, twoDecimals );
        ASSERT_TRUE( DRN::CDRNVariantCodec::IsSame( quantised, result ) );
        ASSERT_TRUE( CodecRoundTrip( CORE::CVariant( (Float64) -7.000049 ), result, size, &twoDecimals ) && -7.0 == result.AsFloat64() );

        // integers kept to a number of bits: unsigned masked, signed clamped
        DRN::CDRNQuantisation eightBits( -1, 8 );
        ASSERT_TRUE( CodecRoundTrip( CORE::CVariant( (UInt32) 0x1FF ), result, size, &eightBits ) && 0xFF == result.AsUInt32() );
        ASSERT_TRUE( CodecRoundTrip( CORE::CVariant( (Int32) 300 ), result, size, &eightBits ) && 127 == result.AsInt32() );
        ASSERT_TRUE( CodecRoundTrip( CORE::CVariant( (Int32) -300 ), result, size, &eightBits ) && -128 == result.AsInt32() );

        // the removal marker of a key
        CORE::CDynamicBuffer buffer;
        DRN::CDRNWireWriter writer( buffer );
        DRN::CDRNVariantCodec::WriteRemoved( writer );
        DRN::CDRNWireReader reader( buffer.GetConstBufferPtr(), writer.GetOffset() );
        bool removed = false;
        ASSERT_TRUE( DRN::CDRNVariantCodec::Read( reader, result, &removed ) && removed );

        // different types are not the same, even with the same number
        ASSERT_FALSE( DRN::CDRNVariantCodec::IsSame( CORE::CVariant( (UInt32) 5 ), CORE::CVariant( (Int32) 5 ) ) );

        // numbers are written and read without allocating
        CORE::CDynamicBuffer hot;
        hot.SetBufferSize( 4096, false );
        CORE::CVariant hotValue( (Float32) 1.5f );
        CORE::CVariant hotResult( (Float32) 0 );
        DRNTEST::StartAllocationCounting();
        for ( UInt32 i=0; i<100; ++i )
        {
            hot.SetDataSize( 0 );
            DRN::CDRNWireWriter hotWriter( hot );
            DRN::CDRNVariantCodec::Write( hotWriter, hotValue, &twoDecimals );
            DRN::CDRNVariantCodec::Write( hotWriter, values[ 6 ] );
            DRN::CDRNWireReader hotReader( hot.GetConstBufferPtr(), hotWriter.GetOffset() );
            DRN::CDRNVariantCodec::Read( hotReader, hotResult );
            DRN::CDRNVariantCodec::Read( hotReader, hotResult );
        }
        UInt32 allocations = DRNTEST::StopAllocationCounting();
        if ( DRNTEST::IsAllocationCountingAvailable() )
            ASSERT_TRUE( 0 == allocations );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    // the round trips were still checked, only the allocation count could not be
    GUCEF_TESTFW_SKIP_IF( !DRNTEST::IsAllocationCountingAvailable(), DRNTEST::GetAllocationCountingUnavailableReason() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wire: the SHA-256 and HMAC-SHA256 of gucefCORE that DRN uses give the published test vectors (FIPS 180-4, RFC 4231)" )
    try
    {
        UInt8 digest[ 32 ];
        CORE::CSha256::Hash( "", 0, digest );
        ASSERT_TRUE( ToHex( digest, 32 ) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" );
        CORE::CSha256::Hash( "abc", 3, digest );
        ASSERT_TRUE( ToHex( digest, 32 ) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
        const char* twoBlocks = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
        CORE::CSha256::Hash( twoBlocks, (UInt32) strlen( twoBlocks ), digest );
        ASSERT_TRUE( ToHex( digest, 32 ) == "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" );

        // a million times 'a', fed in uneven pieces
        CORE::CSha256 sha;
        char chunk[ 997 ];
        memset( chunk, 'a', sizeof( chunk ) );
        UInt32 remaining = 1000000;
        while ( remaining > 0 )
        {
            UInt32 n = remaining < sizeof( chunk ) ? remaining : (UInt32) sizeof( chunk );
            sha.Update( chunk, n );
            remaining -= n;
        }
        sha.Final( digest );
        ASSERT_TRUE( ToHex( digest, 32 ) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0" );

        // RFC 4231 test case 1
        UInt8 key1[ 20 ];
        memset( key1, 0x0b, sizeof( key1 ) );
        CORE::CHmacSha256::Mac( key1, 20, "Hi There", 8, digest );
        ASSERT_TRUE( ToHex( digest, 32 ) == "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7" );
        // test case 2
        CORE::CHmacSha256::Mac( "Jefe", 4, "what do ya want for nothing?", 28, digest );
        ASSERT_TRUE( ToHex( digest, 32 ) == "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843" );
        // test case 3
        UInt8 key3[ 20 ];
        UInt8 data3[ 50 ];
        memset( key3, 0xaa, sizeof( key3 ) );
        memset( data3, 0xdd, sizeof( data3 ) );
        CORE::CHmacSha256::Mac( key3, 20, data3, 50, digest );
        ASSERT_TRUE( ToHex( digest, 32 ) == "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe" );
        // test case 6: a key longer than the block size
        UInt8 key6[ 131 ];
        memset( key6, 0xaa, sizeof( key6 ) );
        const char* data6 = "Test Using Larger Than Block-Size Key - Hash Key First";
        CORE::CHmacSha256::Mac( key6, 131, data6, (UInt32) strlen( data6 ), digest );
        ASSERT_TRUE( ToHex( digest, 32 ) == "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54" );

        UInt8 a[ 4 ] = { 1, 2, 3, 4 };
        UInt8 b[ 4 ] = { 1, 2, 3, 5 };
        ASSERT_TRUE( CORE::CSha256::ConstantTimeEquals( a, a, 4 ) );
        ASSERT_FALSE( CORE::CSha256::ConstantTimeEquals( a, b, 4 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wire: Endpoints parse and write their URI form, endpoint lists as string and data node" )
    try
    {
        DRN::CDRNEndpoint tcp( "tcp://example.org:4000" );
        ASSERT_TRUE( tcp.IsValid() );
        ASSERT_TRUE( tcp.GetChannelType() == "tcp" );
        ASSERT_TRUE( tcp.GetHost() == "example.org" && 4000 == tcp.GetPort() );
        ASSERT_TRUE( tcp.ToString() == "tcp://example.org:4000" );

        DRN::CDRNEndpoint ws( "ws://127.0.0.1:8080/game/lobby?region=eu&max=8" );
        ASSERT_TRUE( ws.GetChannelType() == DRN::CDRNEndpoint::ChannelTypeWebSocket );
        ASSERT_TRUE( ws.GetHost() == "127.0.0.1" && 8080 == ws.GetPort() );
        ASSERT_TRUE( ws.GetPath() == "/game/lobby" );
        ASSERT_TRUE( ws.GetParams().GetValueAlways( "region" ).AsString() == "eu" );
        ASSERT_TRUE( 8 == ws.GetParams().GetValueAlways( "max" ).AsUInt32() );
        DRN::CDRNEndpoint wsAgain( ws.ToString() );
        ASSERT_TRUE( wsAgain == ws );

        DRN::CDRNEndpoint relay( "relay://relay.example.org:7000/session1/42" );
        ASSERT_TRUE( relay.GetChannelType() == "relay" && relay.GetPath() == "/session1/42" );

        ASSERT_FALSE( DRN::CDRNEndpoint( "no scheme here" ).IsValid() );
        ASSERT_FALSE( DRN::CDRNEndpoint( "udp://" ).IsValid() );

        DRN::CDRNEndpointList list;
        ASSERT_TRUE( list.FromString( "udp://10.0.0.2:5000; tcp://203.0.113.9:5000 ;relay://relay:7000/s/3" ) );
        ASSERT_TRUE( 3 == list.GetCount() );
        ASSERT_TRUE( list.Get( 0 ).GetChannelType() == "udp" && list.Get( 2 ).GetChannelType() == "relay" );
        DRN::CDRNEndpointList fromString;
        fromString.FromString( list.ToString() );
        ASSERT_TRUE( fromString == list );

        CORE::CDataNode node( "Endpoints" );
        ASSERT_TRUE( list.SaveConfig( node ) );
        DRN::CDRNEndpointList fromNode;
        ASSERT_TRUE( fromNode.LoadConfig( node ) );
        ASSERT_TRUE( fromNode == list );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wire: Node and session settings round trip through a data node" )
    try
    {
        DRN::CDRNNodeSettings settings;
        settings.serviceName = "game";
        settings.listenEndpoints.FromString( "udp://0.0.0.0:5000;ws://0.0.0.0:5001/drn" );
        settings.echoIntervalMs = 77;
        settings.ownThread = true;
        settings.firstSequence = 12345;
        settings.auth.authenticatorType = "shared-secret";
        settings.auth.SetSetting( "secret", "pw" );

        CORE::CDataNode node( "DRNNodeSettings" );
        ASSERT_TRUE( settings.SaveConfig( node ) );
        DRN::CDRNNodeSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( node ) );
        ASSERT_TRUE( loaded.nodeId == settings.nodeId && loaded.serviceName == "game" );
        ASSERT_TRUE( loaded.listenEndpoints == settings.listenEndpoints );
        ASSERT_TRUE( 77 == loaded.echoIntervalMs && loaded.ownThread && 12345 == loaded.firstSequence );
        ASSERT_TRUE( loaded.auth.authenticatorType == "shared-secret" );
        ASSERT_FALSE( loaded.auth.CreateAuthenticator().IsNULL() );

        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.sessionName = "lobby";
        sessionSettings.topology = DRN::DRN_TOPOLOGY_PEER_TO_PEER;
        sessionSettings.maxMembers = 16;
        CORE::CDataNode sessionNode( "DRNSessionSettings" );
        ASSERT_TRUE( sessionSettings.SaveConfig( sessionNode ) );
        DRN::CDRNSessionSettings loadedSession;
        ASSERT_TRUE( loadedSession.LoadConfig( sessionNode ) );
        ASSERT_TRUE( loadedSession.sessionName == "lobby" && DRN::DRN_TOPOLOGY_PEER_TO_PEER == loadedSession.topology && 16 == loadedSession.maxMembers );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wire: Stream framing gives the same packets for split, merged and byte by byte reads" )
    try
    {
        // a stream of frames of various sizes, including empty and multi-byte length prefixes
        const UInt32 sizes[] = { 0, 1, 2, 127, 128, 300, 16383, 16384, 70000, 5 };
        const UInt32 count = sizeof( sizes ) / sizeof( sizes[ 0 ] );
        CORE::CDynamicBuffer stream;
        GUCEF::vector< CORE::CDynamicBuffer > packets( count );
        UInt32 streamSize = 0;
        for ( UInt32 i=0; i<count; ++i )
        {
            FillPattern( packets[ i ], sizes[ i ], i );
            UInt8 header[ DRN::CDRNStreamFramer::MaxFrameHeaderSize ];
            UInt32 headerSize = DRN::CDRNStreamFramer::WriteFrameHeader( sizes[ i ], header );
            stream.Append( header, headerSize, true );
            if ( sizes[ i ] > 0 )
                stream.Append( packets[ i ].GetConstBufferPtr(), sizes[ i ], true );
            streamSize += headerSize + sizes[ i ];
        }
        ASSERT_TRUE( streamSize == stream.GetDataSize() );

        const UInt32 chunkSizes[] = { 1, 3, 7, 64, 1000, 65536, streamSize };
        for ( UInt32 c=0; c<sizeof( chunkSizes ) / sizeof( chunkSizes[ 0 ] ); ++c )
        {
            DRN::CDRNStreamFramer framer;
            UInt32 received = 0;
            bool allEqual = true;
            UInt32 offset = 0;
            while ( offset < streamSize )
            {
                UInt32 n = streamSize - offset < chunkSizes[ c ] ? streamSize - offset : chunkSizes[ c ];
                framer.BeginChunk( stream.GetConstBufferPtr( offset ), n );
                const UInt8* packet = GUCEF_NULL;
                UInt32 packetSize = 0;
                DRN::CDRNStreamFramer::TFrameResult result;
                while ( DRN::CDRNStreamFramer::FRAME_PACKET == ( result = framer.NextPacket( packet, packetSize ) ) )
                {
                    if ( received >= count || packetSize != sizes[ received ] ||
                         ( packetSize > 0 && 0 != memcmp( packet, packets[ received ].GetConstBufferPtr(), packetSize ) ) )
                        allEqual = false;
                    ++received;
                }
                ASSERT_TRUE( DRN::CDRNStreamFramer::FRAME_ERROR != result );
                framer.EndChunk();
                offset += n;
            }
            ASSERT_TRUE( allEqual );
            ASSERT_TRUE( count == received );
            ASSERT_TRUE( 0 == framer.GetPendingByteCount() );
        }

        // bad magic and a size over the limit are errors
        DRN::CDRNStreamFramer framer( 1024 );
        const UInt8 badMagic[] = { 0x47, 0x45, 0x01, 0x00 };
        const UInt8* packet = GUCEF_NULL;
        UInt32 packetSize = 0;
        framer.BeginChunk( badMagic, sizeof( badMagic ) );
        ASSERT_TRUE( DRN::CDRNStreamFramer::FRAME_ERROR == framer.NextPacket( packet, packetSize ) );
        framer.Reset();
        UInt8 tooLarge[ DRN::CDRNStreamFramer::MaxFrameHeaderSize ];
        UInt32 tooLargeSize = DRN::CDRNStreamFramer::WriteFrameHeader( 2048, tooLarge );
        framer.BeginChunk( tooLarge, tooLargeSize );
        ASSERT_TRUE( DRN::CDRNStreamFramer::FRAME_ERROR == framer.NextPacket( packet, packetSize ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wire: Packet headers and messages round trip" )
    try
    {
        DRN::CDRNPacketHeader header;
        header.sequence = 0xFFFFFFFE;
        header.sendTime = 123456789012ULL;
        header.ackSequence = 77;
        header.ackBits = 0xA5A5A5A5;
        header.echoTime = 987654321ULL;
        header.echoHold = 1500;

        CORE::CDynamicBuffer packet;
        DRN::CDRNWireWriter writer( packet );
        DRN::CDRNPacketCodec::WriteHeader( writer, header );
        ASSERT_TRUE( writer.GetOffset() <= DRN::DRN_MAX_PACKET_HEADER_SIZE );
        const char* text = "payload";
        DRN::CDRNPacketCodec::WriteMessage( writer, DRN::DRN_MSGCLASS_APP, 300, DRN::DRN_LANE_RELIABLE_ORDERED, 70000, text, 7 );
        DRN::CDRNPacketCodec::WriteMessage( writer, DRN::DRN_MSGCLASS_REPL, DRN::DRN_REPL_GRID_DELTA, DRN::DRN_LANE_UNRELIABLE_SEQUENCED, 0, GUCEF_NULL, 0 );
        ASSERT_TRUE( DRN::CDRNPacketCodec::GetMessageSize( 300, DRN::DRN_LANE_RELIABLE_ORDERED, 70000, 7 ) + DRN::CDRNPacketCodec::GetMessageSize( DRN::DRN_REPL_GRID_DELTA, DRN::DRN_LANE_UNRELIABLE_SEQUENCED, 0, 0 )
                     == writer.GetOffset() - ( 1 + 5 + 6 + 1 + 4 + 5 + 2 ) );

        DRN::CDRNWireReader reader( packet );
        DRN::CDRNPacketHeader read;
        ASSERT_TRUE( DRN::CDRNPacketCodec::ReadHeader( reader, read ) );
        ASSERT_TRUE( read.sequence == header.sequence && read.sendTime == header.sendTime && read.ackSequence == header.ackSequence );
        ASSERT_TRUE( read.ackBits == header.ackBits && read.echoTime == header.echoTime && read.echoHold == header.echoHold );
        DRN::CDRNMessageView message;
        ASSERT_TRUE( DRN::CDRNPacketCodec::ReadMessage( reader, message ) );
        ASSERT_TRUE( DRN::DRN_MSGCLASS_APP == message.msgClass && 300 == message.opcode && 70000 == message.laneSequence );
        ASSERT_TRUE( 7 == message.payloadSize && 0 == memcmp( message.payload, text, 7 ) );
        ASSERT_TRUE( DRN::CDRNPacketCodec::ReadMessage( reader, message ) );
        ASSERT_TRUE( DRN::DRN_LANE_UNRELIABLE_SEQUENCED == message.lane && 0 == message.payloadSize );
        ASSERT_TRUE( 0 == reader.GetRemaining() );

        // a packet of another protocol is not read
        UInt8 other[] = { 0x47, 0x01, 0x02 };
        DRN::CDRNWireReader otherReader( other, sizeof( other ) );
        ASSERT_FALSE( DRN::CDRNPacketCodec::ReadHeader( otherReader, read ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Wire: Sequence numbers compare wrap-safe and the receive window drops duplicates and stale ones" )
    try
    {
        ASSERT_TRUE( DRN::DRNIsNewerSequence( 1, 0xFFFFFFFF ) );
        ASSERT_TRUE( DRN::DRNIsNewerSequence( 5, 3 ) );
        ASSERT_FALSE( DRN::DRNIsNewerSequence( 0xFFFFFFF0, 2 ) );
        ASSERT_FALSE( DRN::DRNIsNewerSequence( 7, 7 ) );

        DRN::CDRNSequenceWindow window;
        UInt32 gap = 0;
        UInt32 sequence = 0xFFFFFFF0;
        ASSERT_TRUE( DRN::CDRNSequenceWindow::SEQUENCE_NEWEST == window.Accept( sequence, gap ) );
        for ( UInt32 i=0; i<40; ++i )
        {
            ++sequence;
            if ( 0 == sequence )
                continue;
            if ( 5 == i )
                continue;   // one lost on the way, before the wrap
            ASSERT_TRUE( DRN::CDRNSequenceWindow::SEQUENCE_NEWEST == window.Accept( sequence, gap ) );
        }
        ASSERT_TRUE( window.GetHighest() == sequence );
        // a duplicate, the late one, a stale one
        ASSERT_TRUE( DRN::CDRNSequenceWindow::SEQUENCE_DUPLICATE == window.Accept( sequence - 3, gap ) );
        ASSERT_TRUE( DRN::CDRNSequenceWindow::SEQUENCE_LATE == window.Accept( 0xFFFFFFF0 + 6, gap ) );
        ASSERT_TRUE( DRN::CDRNSequenceWindow::SEQUENCE_DUPLICATE == window.Accept( 0xFFFFFFF0 + 6, gap ) );
        ASSERT_TRUE( DRN::CDRNSequenceWindow::SEQUENCE_TOO_OLD == window.Accept( sequence - 2000, gap ) );

        // gaps are reported, ack bits name the received ones
        DRN::CDRNSequenceWindow window2;
        window2.Accept( 10, gap );
        ASSERT_TRUE( DRN::CDRNSequenceWindow::SEQUENCE_NEWEST == window2.Accept( 13, gap ) && 2 == gap );
        ASSERT_TRUE( 0x4 == ( window2.GetAckBits() & 0x7 ) );     // 12 and 11 missing, 10 received
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
