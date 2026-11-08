/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CBINARYWIREWRITER_H
#include "gucefCORE_CBinaryWireWriter.h"
#define GUCEF_CORE_CBINARYWIREWRITER_H
#endif /* GUCEF_CORE_CBINARYWIREWRITER_H ? */

#ifndef GUCEF_CORE_CBINARYWIREREADER_H
#include "gucefCORE_CBinaryWireReader.h"
#define GUCEF_CORE_CBINARYWIREREADER_H
#endif /* GUCEF_CORE_CBINARYWIREREADER_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestBinaryWire.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
BytesEqual( const CDynamicBuffer& buffer, const UInt8* expected, UInt32 size )
{
    if ( buffer.GetDataSize() != size )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "BinaryWire: expected " << size << " bytes, got " << buffer.GetDataSize() );
        return false;
    }
    return 0 == memcmp( buffer.GetConstBufferPtr(), expected, size );
}

bool
VarUInt64Encodes( UInt64 value, const UInt8* expected, UInt32 size )
{
    CDynamicBuffer buffer;
    CBinaryWireWriter writer( buffer );
    writer.WriteVarUInt64( value );
    if ( !BytesEqual( buffer, expected, size ) )
        return false;
    if ( CBinaryWireWriter::GetVarUInt64Size( value ) != size )
        return false;
    CBinaryWireReader reader( buffer );
    UInt64 back = 0;
    return reader.ReadVarUInt64( back ) && back == value && reader.IsAtEnd();
}

bool
VariantRoundTrips( const CVariant& value )
{
    CDynamicBuffer buffer;
    CBinaryWireWriter writer( buffer );
    if ( !writer.WriteVariant( value ) )
        return false;
    CBinaryWireReader reader( buffer );
    CVariant back;
    if ( !reader.ReadVariant( back ) || !reader.IsAtEnd() )
        return false;
    if ( back != value )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "BinaryWire: variant of type " << value.GetTypeName() << " came back as " << back.GetTypeName()
                         << " \"" << back.AsString() << "\" instead of \"" << value.AsString() << "\"" );
        return false;
    }
    return true;
}

bool
ReadFails( const UInt8* bytes, UInt32 size, int what )
{
    CBinaryWireReader reader( bytes, size );
    bool ok = true;
    switch ( what )
    {
        case 0: { UInt64 v = 0; ok = reader.ReadVarUInt64( v ); break; }
        case 1: { UInt32 v = 0; ok = reader.ReadVarUInt32( v ); break; }
        case 2: { CString v; ok = reader.ReadString( v ); break; }
        case 3: { CDecimal v; ok = reader.ReadDecimal( v ); break; }
        case 4: { bool v = false; ok = reader.ReadBool( v ); break; }
        case 5: { CVariant v; ok = reader.ReadVariant( v ); break; }
        default: break;
    }
    return !ok && reader.HasFailed();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformBinaryWireTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "BinaryWire" );

    GUCEF_TESTFW_TESTCASE( "BinaryWire: varints encode as unsigned LEB128 (protobuf vectors)" )
    try
    {
        const UInt8 v0[] = { 0x00 };
        const UInt8 v1[] = { 0x01 };
        const UInt8 v127[] = { 0x7F };
        const UInt8 v128[] = { 0x80, 0x01 };
        const UInt8 v150[] = { 0x96, 0x01 };
        const UInt8 v300[] = { 0xAC, 0x02 };
        const UInt8 v16383[] = { 0xFF, 0x7F };
        const UInt8 v16384[] = { 0x80, 0x80, 0x01 };
        const UInt8 vMax32[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x0F };
        const UInt8 vMax64[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01 };
        ASSERT_TRUE( VarUInt64Encodes( 0, v0, 1 ) );
        ASSERT_TRUE( VarUInt64Encodes( 1, v1, 1 ) );
        ASSERT_TRUE( VarUInt64Encodes( 127, v127, 1 ) );
        ASSERT_TRUE( VarUInt64Encodes( 128, v128, 2 ) );
        ASSERT_TRUE( VarUInt64Encodes( 150, v150, 2 ) );
        ASSERT_TRUE( VarUInt64Encodes( 300, v300, 2 ) );
        ASSERT_TRUE( VarUInt64Encodes( 16383, v16383, 2 ) );
        ASSERT_TRUE( VarUInt64Encodes( 16384, v16384, 3 ) );
        ASSERT_TRUE( VarUInt64Encodes( 0xFFFFFFFFULL, vMax32, 5 ) );
        ASSERT_TRUE( VarUInt64Encodes( 0xFFFFFFFFFFFFFFFFULL, vMax64, 10 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: zig-zag maps small magnitudes to small codes, both widths" )
    try
    {
        ASSERT_TRUE( 0 == CBinaryWireWriter::ZigZagEncode32( 0 ) );
        ASSERT_TRUE( 1 == CBinaryWireWriter::ZigZagEncode32( -1 ) );
        ASSERT_TRUE( 2 == CBinaryWireWriter::ZigZagEncode32( 1 ) );
        ASSERT_TRUE( 3 == CBinaryWireWriter::ZigZagEncode32( -2 ) );
        ASSERT_TRUE( 0xFFFFFFFEUL == CBinaryWireWriter::ZigZagEncode32( 2147483647 ) );
        ASSERT_TRUE( 0xFFFFFFFFUL == CBinaryWireWriter::ZigZagEncode32( (Int32) 0x80000000UL ) );
        ASSERT_TRUE( 0xFFFFFFFFFFFFFFFFULL == CBinaryWireWriter::ZigZagEncode64( (Int64) 0x8000000000000000ULL ) );

        const Int64 values[] = { 0, -1, 1, -64, 64, -65, 2147483647LL, -2147483647LL - 1, 9223372036854775807LL, (Int64) 0x8000000000000000ULL };
        for ( UInt32 i=0; i<sizeof( values ) / sizeof( values[ 0 ] ); ++i )
        {
            CDynamicBuffer buffer;
            CBinaryWireWriter writer( buffer );
            writer.WriteVarInt64( values[ i ] );
            CBinaryWireReader reader( buffer );
            Int64 back = 1;
            ASSERT_TRUE( reader.ReadVarInt64( back ) );
            ASSERT_TRUE( back == values[ i ] );
        }

        // -1 is one byte: 0x01
        CDynamicBuffer buffer;
        CBinaryWireWriter writer( buffer );
        writer.WriteVarInt32( -1 );
        const UInt8 minusOne[] = { 0x01 };
        ASSERT_TRUE( BytesEqual( buffer, minusOne, 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: malformed varints are refused and the failure sticks" )
    try
    {
        const UInt8 elevenBytes[] = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01 };
        ASSERT_TRUE( ReadFails( elevenBytes, sizeof( elevenBytes ), 0 ) );

        // the 10th byte may only hold the 64th bit
        const UInt8 tenthTooBig[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x02 };
        ASSERT_TRUE( ReadFails( tenthTooBig, sizeof( tenthTooBig ), 0 ) );

        const UInt8 truncated[] = { 0x80, 0x80 };
        ASSERT_TRUE( ReadFails( truncated, sizeof( truncated ), 0 ) );

        // 2^32 does not fit 32 bits
        const UInt8 above32[] = { 0x80, 0x80, 0x80, 0x80, 0x10 };
        ASSERT_TRUE( ReadFails( above32, sizeof( above32 ), 1 ) );

        const UInt8 boolTwo[] = { 0x02 };
        ASSERT_TRUE( ReadFails( boolTwo, sizeof( boolTwo ), 4 ) );

        // sticky: after a failure a valid read fails too
        const UInt8 bytes[] = { 0x80, 0x80 };
        CBinaryWireReader reader( bytes, sizeof( bytes ) );
        UInt64 v = 0;
        ASSERT_FALSE( reader.ReadVarUInt64( v ) );
        UInt8 b = 0;
        ASSERT_FALSE( reader.ReadUInt8( b ) );
        ASSERT_TRUE( reader.HasFailed() );

        // a length beyond the buffer
        const UInt8 longLength[] = { 0x05, 'a', 'b' };
        CBinaryWireReader lengthReader( longLength, sizeof( longLength ) );
        const UInt8* view = GUCEF_NULL;
        UInt32 viewSize = 0;
        ASSERT_FALSE( lengthReader.ReadSizedBytes( view, viewSize ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: fixed values and floats are little endian" )
    try
    {
        CDynamicBuffer buffer;
        CBinaryWireWriter writer( buffer );
        writer.WriteFixed32( 0x01020304UL );
        writer.WriteFixed64( 0x0102030405060708ULL );
        writer.WriteFloat32( 1.0f );
        writer.WriteFloat64( -2.0 );
        writer.WriteUInt64BigEndian( 0x0102030405060708ULL );
        const UInt8 expected[] = { 0x04, 0x03, 0x02, 0x01,
                                   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
                                   0x00, 0x00, 0x80, 0x3F,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
        ASSERT_TRUE( BytesEqual( buffer, expected, sizeof( expected ) ) );

        CBinaryWireReader reader( buffer );
        UInt32 a = 0; UInt64 b = 0; Float32 c = 0; Float64 d = 0; UInt64 e = 0;
        ASSERT_TRUE( reader.ReadFixed32( a ) && 0x01020304UL == a );
        ASSERT_TRUE( reader.ReadFixed64( b ) && 0x0102030405060708ULL == b );
        ASSERT_TRUE( reader.ReadFloat32( c ) && 1.0f == c );
        ASSERT_TRUE( reader.ReadFloat64( d ) && -2.0 == d );
        ASSERT_TRUE( reader.ReadUInt64BigEndian( e ) && 0x0102030405060708ULL == e );
        ASSERT_TRUE( reader.IsAtEnd() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: strings are UTF-8 with a varint length, invalid UTF-8 is refused" )
    try
    {
        CDynamicBuffer buffer;
        CBinaryWireWriter writer( buffer );
        CUtf8String euro;
        euro.Append( 0x20AC );
        euro.Append( 'x' );
        writer.WriteString( ToString( euro ) );
        writer.WriteString( CString::Empty );
        const UInt8 expected[] = { 0x04, 0xE2, 0x82, 0xAC, 'x', 0x00 };
        ASSERT_TRUE( BytesEqual( buffer, expected, sizeof( expected ) ) );

        CBinaryWireReader reader( buffer );
        CString back;
        ASSERT_TRUE( reader.ReadString( back ) );
        ASSERT_TRUE( ToUtf8String( back ) == euro );
        ASSERT_TRUE( reader.ReadString( back ) && back.IsNULLOrEmpty() );

        const UInt8 overlong[] = { 0x02, 0xC0, 0x80 };
        ASSERT_TRUE( ReadFails( overlong, sizeof( overlong ), 2 ) );
        const UInt8 surrogate[] = { 0x03, 0xED, 0xA0, 0x80 };
        ASSERT_TRUE( ReadFails( surrogate, sizeof( surrogate ), 2 ) );
        const UInt8 aboveMax[] = { 0x04, 0xF4, 0x90, 0x80, 0x80 };
        ASSERT_TRUE( ReadFails( aboveMax, sizeof( aboveMax ), 2 ) );
        const UInt8 cut[] = { 0x02, 0xE2, 0x82 };
        ASSERT_TRUE( ReadFails( cut, sizeof( cut ), 2 ) );
        const UInt8 strayContinuation[] = { 0x01, 0x80 };
        ASSERT_TRUE( ReadFails( strayContinuation, sizeof( strayContinuation ), 2 ) );

        const UInt8 fourByte[] = { 0xF0, 0x9F, 0x98, 0x80 };
        ASSERT_TRUE( CBinaryWireReader::IsValidUtf8( fourByte, 4 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: decimals keep their scale; a scale above 18 is refused" )
    try
    {
        CDecimal price = CDecimal::Parse( "-12.3400" );
        CDynamicBuffer buffer;
        CBinaryWireWriter writer( buffer );
        writer.WriteDecimal( price );
        // unscaled -123400 zig-zag = 246799 = 0x3C40F -> 8F 88 0F, scale 4
        const UInt8 expected[] = { 0x8F, 0x88, 0x0F, 0x04 };
        ASSERT_TRUE( BytesEqual( buffer, expected, sizeof( expected ) ) );

        CBinaryWireReader reader( buffer );
        CDecimal back;
        ASSERT_TRUE( reader.ReadDecimal( back ) );
        ASSERT_TRUE( back.IsIdentical( price ) );
        ASSERT_TRUE( 4 == back.GetScale() );

        const UInt8 scale19[] = { 0x02, 0x13 };
        ASSERT_TRUE( ReadFails( scale19, sizeof( scale19 ), 3 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: dates, timestamps and date times with an offset round trip, before 1970 too" )
    try
    {
        CDynamicBuffer buffer;
        CBinaryWireWriter writer( buffer );
        writer.WriteDate( -1 );
        writer.WriteTimestamp( -1500000 );
        CDateTime local( CDate( 2026, 11, 8 ), CTime( 14, 30, 15, 250 ), -300 );
        ASSERT_TRUE( writer.WriteDateTime( local ) );
        CDateTime old( CDate( 1950, 2, 3 ), CTime( 1, 2, 3, 4 ), 60 );
        ASSERT_TRUE( writer.WriteDateTime( old ) );

        CBinaryWireReader reader( buffer );
        Int32 days = 0;
        Int64 micros = 0;
        ASSERT_TRUE( reader.ReadDate( days ) && -1 == days );
        ASSERT_TRUE( reader.ReadTimestamp( micros ) && -1500000 == micros );
        CDateTime back;
        ASSERT_TRUE( reader.ReadDateTime( back ) );
        ASSERT_TRUE( back == local );
        ASSERT_TRUE( -300 == back.GetTimeZoneUTCOffsetInMins() );
        ASSERT_TRUE( 14 == back.GetTime().GetHours() && 30 == back.GetTime().GetMinutes() );
        CDateTime backOld;
        ASSERT_TRUE( reader.ReadDateTime( backOld ) );
        if ( !( backOld == old ) )
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "BinaryWire: " << old.ToIso8601DateTimeString( true, true ) << " came back as " << backOld.ToIso8601DateTimeString( true, true ) );
        ASSERT_TRUE( backOld == old );
        ASSERT_TRUE( 60 == backOld.GetTimeZoneUTCOffsetInMins() );
        ASSERT_TRUE( reader.IsAtEnd() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: variants of every supported type round trip with their type" )
    try
    {
        ASSERT_TRUE( VariantRoundTrips( CVariant( true ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (UInt8) 200 ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (Int8) -100 ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (UInt16) 65000 ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (Int16) -32000 ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (UInt32) 4000000000UL ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (Int32) -2000000000 ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (UInt64) 0xFFFFFFFFFFFFFFFFULL ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (Int64) -9000000000000LL ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (Float32) 1.5f ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( (Float64) -3.25 ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( CUtf8String( "hello" ) ) ) );
        ASSERT_TRUE( VariantRoundTrips( CVariant( CDecimal::Parse( "1.50" ) ) ) );
        ASSERT_TRUE( VariantRoundTrips( CDataTableValues::Date( -400 ) ) );
        ASSERT_TRUE( VariantRoundTrips( CDataTableValues::Timestamp( -123456789LL, DATATABLE_TIMEUNIT_MICROSECONDS ) ) );
        ASSERT_TRUE( VariantRoundTrips( CDataTableValues::Timestamp( 1700000000123LL, DATATABLE_TIMEUNIT_MILLISECONDS ) ) );
        const UInt8 blob[] = { 0, 1, 2, 255 };
        ASSERT_TRUE( VariantRoundTrips( CVariant( blob, sizeof( blob ) ) ) );

        // the decimal keeps its scale through a variant as well
        CDynamicBuffer buffer;
        CBinaryWireWriter writer( buffer );
        writer.WriteVariant( CVariant( CDecimal::Parse( "2.200" ) ) );
        CBinaryWireReader reader( buffer );
        CVariant back;
        ASSERT_TRUE( reader.ReadVariant( back ) );
        CDecimal backDecimal;
        ASSERT_TRUE( DECIMALSTATUS_OK == back.GetDecimal( backDecimal ) && 3 == backDecimal.GetScale() );

        // a linked binary view points into the buffer read
        CDynamicBuffer blobBuffer;
        CBinaryWireWriter blobWriter( blobBuffer );
        blobWriter.WriteVariant( CVariant( blob, sizeof( blob ) ) );
        CBinaryWireReader blobReader( blobBuffer );
        CVariant linked;
        ASSERT_TRUE( blobReader.ReadVariant( linked, true ) );
        ASSERT_TRUE( linked.IsDynamicMemoryLinked() );
        ASSERT_TRUE( 4 == linked.ByteSize( false ) && 0 == memcmp( linked.AsVoidPtr(), blob, 4 ) );

        // memory addresses mean nothing elsewhere
        CDynamicBuffer addressBuffer;
        CBinaryWireWriter addressWriter( addressBuffer );
        int local = 0;
        ASSERT_FALSE( addressWriter.WriteVariant( CVariant( (void*) &local ) ) );

        // a declared int8 outside its range is refused
        const UInt8 int8TooBig[] = { GUCEF_DATATYPE_INT8, 0x80, 0x02 };
        ASSERT_TRUE( ReadFails( int8TooBig, sizeof( int8TooBig ), 5 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: a writer at an offset grows the buffer and keeps the bytes before it" )
    try
    {
        CDynamicBuffer buffer;
        buffer.CopyFrom( 3, "abc" );
        CBinaryWireWriter writer( buffer, 3 );
        for ( UInt32 i=0; i<1000; ++i )
            writer.WriteVarUInt32( i );
        ASSERT_TRUE( 0 == memcmp( buffer.GetConstBufferPtr(), "abc", 3 ) );
        CBinaryWireReader reader( buffer, 3 );
        bool allOk = true;
        for ( UInt32 i=0; i<1000; ++i )
        {
            UInt32 v = 0;
            if ( !reader.ReadVarUInt32( v ) || v != i )
                allOk = false;
        }
        ASSERT_TRUE( allOk );
        ASSERT_TRUE( reader.IsAtEnd() );
        ASSERT_TRUE( writer.GetOffset() == buffer.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: a write past the end of the 32 bit range fails instead of wrapping around" )
    try
    {
        // the end of the write wrapped to 16: the bytes went 4 GB beyond the buffer
        CDynamicBuffer buffer;
        CBinaryWireWriter writer( buffer );
        writer.SetOffset( 0xFFFFFFF0 );
        UInt8 data[ 32 ];
        memset( data, 7, sizeof( data ) );
        writer.WriteBytes( data, sizeof( data ) );
        ASSERT_TRUE( writer.HasFailed() );
        ASSERT_TRUE( 0xFFFFFFF0 == writer.GetOffset() );
        ASSERT_TRUE( 0 == buffer.GetDataSize() );
        ASSERT_TRUE( GUCEF_NULL == writer.Reserve( 1 ) );
        ASSERT_FALSE( writer.WriteVariant( CVariant( (UInt32) 5 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: growth beyond 2 GB ends at the maximum size instead of doubling forever" )
    try
    {
        // a write at 2^31 doubled the size until it wrapped to 0, then looped forever; the maximum keeps the test from
        // allocating 2 GB
        CDynamicBuffer buffer;
        CBinaryWireWriter writer( buffer );
        writer.SetMaxSize( 1024 * 1024 );
        writer.SetOffset( 0x80000000 );
        writer.WriteUInt8( 1 );
        ASSERT_TRUE( writer.HasFailed() );
        ASSERT_TRUE( 0 == buffer.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "BinaryWire: a write the maximum size refuses fails, the earlier bytes stay and later writes add nothing" )
    try
    {
        CDynamicBuffer buffer;
        CBinaryWireWriter writer( buffer );
        writer.SetMaxSize( 100 );
        UInt8 data[ 64 ];
        memset( data, 3, sizeof( data ) );
        writer.WriteBytes( data, sizeof( data ) );
        ASSERT_FALSE( writer.HasFailed() );
        ASSERT_TRUE( 64 == writer.GetOffset() && 64 == buffer.GetDataSize() );
        ASSERT_TRUE( buffer.GetBufferSize() <= 100 );
        writer.WriteBytes( data, sizeof( data ) );
        ASSERT_TRUE( writer.HasFailed() );
        ASSERT_TRUE( 64 == writer.GetOffset() && 64 == buffer.GetDataSize() );
        writer.WriteUInt8( 1 );
        writer.WriteVarUInt64( 1000 );
        writer.WriteFixed64( 1 );
        ASSERT_TRUE( 64 == writer.GetOffset() && 64 == buffer.GetDataSize() );

        // up to the maximum exactly
        CDynamicBuffer exact;
        CBinaryWireWriter exactWriter( exact );
        exactWriter.SetMaxSize( 100 );
        exactWriter.WriteBytes( data, 64 );
        exactWriter.WriteBytes( data, 36 );
        ASSERT_FALSE( exactWriter.HasFailed() );
        ASSERT_TRUE( 100 == exact.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
