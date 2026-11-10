/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CBINARYWIREWRITER_H
#include "gucefCORE_CBinaryWireWriter.h"
#define GUCEF_CORE_CBINARYWIREWRITER_H
#endif /* GUCEF_CORE_CBINARYWIREWRITER_H ? */

#ifndef GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#include "gucefCORE_CDataNodeBinarySerializer.h"
#define GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CDATANODEBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_CBINARYWIREREADER_H
#include "gucefCORE_CBinaryWireReader.h"
#define GUCEF_CORE_CBINARYWIREREADER_H
#endif /* GUCEF_CORE_CBINARYWIREREADER_H ? */

#ifndef GUCEF_COMCORE_CRPCFRAMECODEC_H
#include "gucefCOMCORE_CRpcFrameCodec.h"
#define GUCEF_COMCORE_CRPCFRAMECODEC_H
#endif /* GUCEF_COMCORE_CRPCFRAMECODEC_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGECODEC_H
#include "gucefCOMCORE_CRpcMessageCodec.h"
#define GUCEF_COMCORE_CRPCMESSAGECODEC_H
#endif /* GUCEF_COMCORE_CRPCMESSAGECODEC_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRpcSupport.h"

#include "TestRpcCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

namespace {

COMCORE::CRpcContractPtr
BuildCodecContract( void )
{
    COMCORE::CRpcContractBuilder b( "gucef.test.codec", 1, 0 );
    b.AddEnum( "Color" );
    b.AddEnumValue( "Color", 0, "COLOR_NONE" );
    b.AddEnumValue( "Color", 1, "COLOR_RED" );
    b.AddEnumValue( "Color", -5, "COLOR_NEGATIVE" );

    b.AddType( "Inner" );
    b.AddField( "Inner", 1, "id", "uint32" );
    b.AddField( "Inner", 2, "label", "string" );

    b.AddType( "AllTypes" );
    b.AddField( "AllTypes", 1, "flag", "bool" );
    b.AddField( "AllTypes", 2, "i8", "int8" );
    b.AddField( "AllTypes", 3, "i16", "int16" );
    b.AddField( "AllTypes", 4, "i32", "int32" );
    b.AddField( "AllTypes", 5, "i64", "int64" );
    b.AddField( "AllTypes", 6, "u8", "uint8" );
    b.AddField( "AllTypes", 7, "u16", "uint16" );
    b.AddField( "AllTypes", 8, "u32", "uint32" );
    b.AddField( "AllTypes", 9, "u64", "uint64" );
    b.AddField( "AllTypes", 10, "f32", "float32" );
    b.AddField( "AllTypes", 11, "f64", "float64" );
    b.AddField( "AllTypes", 12, "text", "string" );
    b.AddField( "AllTypes", 13, "blob", "bytes" );
    b.AddField( "AllTypes", 14, "amount", "decimal" );
    b.AddField( "AllTypes", 15, "price", "decimal(4)" );
    b.AddField( "AllTypes", 16, "day", "date" );
    b.AddField( "AllTypes", 17, "at", "timestamp" );
    b.AddField( "AllTypes", 18, "local", "datetime" );
    b.AddField( "AllTypes", 19, "took", "duration" );
    b.AddField( "AllTypes", 20, "color", "Color" );
    b.AddField( "AllTypes", 21, "inner", "Inner" );
    b.AddField( "AllTypes", 22, "cell", "variant" );
    b.AddField( "AllTypes", 23, "doc", "datanode" );
    b.AddField( "AllTypes", 24, "numbers", "int32", COMCORE::RPC_CARDINALITY_REPEATED );
    b.AddField( "AllTypes", 25, "names", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    b.AddField( "AllTypes", 26, "inners", "Inner", COMCORE::RPC_CARDINALITY_REPEATED );
    b.AddField( "AllTypes", 27, "byName", "map<string,uint32>" );
    b.AddField( "AllTypes", 28, "byId", "map<int64,Inner>" );
    b.AddField( "AllTypes", 29, "secret", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, GUCEF_RPC_FIELDFLAG_SENSITIVE );
    b.AddField( "AllTypes", 30, "prices", "decimal(2)", COMCORE::RPC_CARDINALITY_REPEATED );
    b.AddField( "AllTypes", 100000, "far", "uint32" );

    b.AddType( "Node" );
    b.AddField( "Node", 1, "child", "Node" );
    b.AddField( "Node", 2, "value", "uint32" );

    // the same type as seen by an older peer: only some fields
    b.AddType( "AllTypesOld" );
    b.AddField( "AllTypesOld", 1, "flag", "bool" );
    b.AddField( "AllTypesOld", 12, "text", "string" );

    // protobuf's classic examples
    b.AddType( "Test1" );
    b.AddField( "Test1", 1, "a", "uint32" );
    b.AddType( "Test2" );
    b.AddField( "Test2", 2, "b", "string" );

    CORE::CString error;
    COMCORE::CRpcContractPtr contract = b.Build( error );
    if ( contract.IsNULL() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcCodec: test contract does not build: " << error );
    return contract;
}

bool
Encoded( const COMCORE::CRpcMessage& message, CORE::CDynamicBuffer& out )
{
    out.Clear( true );
    return message.Encode( out );
}

bool
BytesAre( const CORE::CDynamicBuffer& buffer, const UInt8* expected, UInt32 size )
{
    bool same = buffer.GetDataSize() == size && 0 == memcmp( buffer.GetConstBufferPtr(), expected, size );
    if ( !same )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcCodec: got " << RpcTestHex( buffer.GetConstBufferPtr(), buffer.GetDataSize() ) <<
                         " expected " << RpcTestHex( expected, size ) );
    return same;
}

bool
DecodeFails( const COMCORE::CRpcContractPtr& contract, const char* type, const UInt8* bytes, UInt32 size )
{
    COMCORE::CRpcMessage message = contract->NewMessage( type );
    COMCORE::CRpcStatus status;
    bool ok = message.Decode( bytes, size, status );
    if ( ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcCodec: expected a refusal for " << RpcTestHex( bytes, size ) );
    return !ok && COMCORE::RPC_STATUS_INVALID_ARGUMENT == status.code;
}

/** count entries of the map< string, uint32 > field 27 of AllTypes, keys in descending order ("k<count-1>" first) */
void
WriteMapEntries( CORE::CDynamicBuffer& buffer, UInt32 count )
{
    buffer.Clear( true );
    CORE::CBinaryWireWriter w( buffer );
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString key = "k" + CORE::ToString( count - 1 - i );
        UInt32 keySize = key.Length();
        w.WriteVarUInt32( ( 27 << 3 ) | 2 );
        w.WriteVarUInt32( 2 + keySize + 1 + CORE::CBinaryWireWriter::GetVarUInt32Size( i ) );
        w.WriteVarUInt32( ( 1 << 3 ) | 2 );
        w.WriteUtf8( key.C_String(), keySize );
        w.WriteVarUInt32( ( 2 << 3 ) | 0 );
        w.WriteVarUInt32( i );
    }
}

/** count fields with the number 1000, unknown to AllTypes: every one is kept behind the earlier ones */
void
WriteUnknownFields( CORE::CDynamicBuffer& buffer, UInt32 count )
{
    buffer.Clear( true );
    CORE::CBinaryWireWriter w( buffer );
    for ( UInt32 i=0; i<count; ++i )
    {
        w.WriteVarUInt32( ( 1000 << 3 ) | 0 );
        w.WriteVarUInt32( i );
    }
}

/** The shortest of a few decodes (or encodes) of the buffer, in milliseconds */
double
BestCodecMs( const COMCORE::CRpcContractPtr& contract, const CORE::CDynamicBuffer& buffer, bool encode, bool& ok )
{
    double best = 0;
    ok = true;
    COMCORE::CRpcMessage message = contract->NewMessage( "AllTypes" );
    COMCORE::CRpcStatus status;
    if ( encode && !message.Decode( buffer.GetConstBufferPtr(), buffer.GetDataSize(), status ) )
        ok = false;
    for ( UInt32 round=0; round<3 && ok; ++round )
    {
        CORE::CDynamicBuffer encoded;
        UInt64 start = MT::PrecisionTickCount();
        if ( encode )
            ok = message.Encode( encoded );
        else
            ok = message.Decode( buffer.GetConstBufferPtr(), buffer.GetDataSize(), status );
        double ms = (double) ( MT::PrecisionTickCount() - start ) * 1000.0 / (double) MT::PrecisionTimerResolution();
        if ( 0 == round || ms < best )
            best = ms;
    }
    return best;
}

}

/*-------------------------------------------------------------------------*/

void
PerformRpcCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "rpc_codec" );

    COMCORE::CRpcContractPtr contract = BuildCodecContract();

    GUCEF_TESTFW_TESTCASE( "rpc_codec: the key layer is protobuf's (fixed vectors), so --decode_raw can read payloads" )
    try
    {
        ASSERT_FALSE( contract.IsNULL() );
        COMCORE::CRpcMessage t1 = contract->NewMessage( "Test1" );
        ASSERT_TRUE( t1.SetUInt32( 1, 150 ) );
        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( Encoded( t1, bytes ) );
        const UInt8 expected1[] = { 0x08, 0x96, 0x01 };
        ASSERT_TRUE( BytesAre( bytes, expected1, sizeof( expected1 ) ) );

        COMCORE::CRpcMessage t2 = contract->NewMessage( "Test2" );
        ASSERT_TRUE( t2.SetString( 2, "testing" ) );
        ASSERT_TRUE( Encoded( t2, bytes ) );
        const UInt8 expected2[] = { 0x12, 0x07, 't', 'e', 's', 't', 'i', 'n', 'g' };
        ASSERT_TRUE( BytesAre( bytes, expected2, sizeof( expected2 ) ) );

        // a set field holding its zero value travels (presence), an unset one does not
        COMCORE::CRpcMessage zero = contract->NewMessage( "Test1" );
        ASSERT_TRUE( Encoded( zero, bytes ) && 0 == bytes.GetDataSize() );
        ASSERT_TRUE( zero.SetUInt32( 1, 0 ) );
        const UInt8 expectedZero[] = { 0x08, 0x00 };
        ASSERT_TRUE( Encoded( zero, bytes ) && BytesAre( bytes, expectedZero, sizeof( expectedZero ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: every field type round trips with its value and type" )
    try
    {
        COMCORE::CRpcMessage m = contract->NewMessage( "AllTypes" );
        ASSERT_TRUE( m.SetBool( 1, true ) );
        ASSERT_TRUE( m.SetInt32( 2, -128 ) );
        ASSERT_TRUE( m.SetInt32( 3, -32768 ) );
        ASSERT_TRUE( m.SetInt32( 4, -2147483647 - 1 ) );
        ASSERT_TRUE( m.SetInt64( 5, (Int64) 0x8000000000000000ULL ) );
        ASSERT_TRUE( m.SetUInt32( 6, 255 ) );
        ASSERT_TRUE( m.SetUInt32( 7, 65535 ) );
        ASSERT_TRUE( m.SetUInt32( 8, 4294967295UL ) );
        ASSERT_TRUE( m.SetUInt64( 9, 0xFFFFFFFFFFFFFFFFULL ) );
        ASSERT_TRUE( m.Set( 10, CORE::CVariant( (Float32) -1.5f ) ) );
        ASSERT_TRUE( m.SetFloat64( 11, 3.25 ) );
        CORE::CUtf8String text( "caf" );
        text.Append( 0xE9 );
        ASSERT_TRUE( m.SetString( 12, CORE::ToString( text ) ) );
        const UInt8 blob[] = { 0, 1, 2, 3, 254, 255 };
        ASSERT_TRUE( m.SetBytes( 13, blob, sizeof( blob ) ) );
        ASSERT_TRUE( m.SetDecimal( 14, CORE::CDecimal::Parse( "-12.3400" ) ) );
        ASSERT_TRUE( m.SetDecimal( 15, CORE::CDecimal::Parse( "1.5" ) ) );
        ASSERT_TRUE( m.SetDate( 16, -10 ) );
        ASSERT_TRUE( m.SetTimestamp( 17, 1700000000123456LL ) );
        CORE::CDateTime local( CORE::CDate( 2026, 11, 8 ), CORE::CTime( 9, 15, 30, 500 ), 120 );
        ASSERT_TRUE( m.SetDateTime( 18, local ) );
        ASSERT_TRUE( m.SetDuration( 19, -2500000 ) );
        ASSERT_TRUE( m.SetEnum( 20, -5 ) );
        COMCORE::CRpcMessage* inner = m.GetMutableMessage( 21 );
        ASSERT_TRUE( GUCEF_NULL != inner );
        if ( GUCEF_NULL != inner )
        {
            inner->SetUInt32( 1, 7 );
            inner->SetString( 2, "seven" );
        }
        ASSERT_TRUE( m.Set( 22, CORE::CVariant( CORE::CDecimal::Parse( "9.99" ) ) ) );
        CORE::CDataNode doc;
        doc.SetName( "details" );
        doc.SetAttribute( "reason", "testing" );
        ASSERT_TRUE( m.SetDataNode( 23, doc ) );
        ASSERT_TRUE( m.SetUInt32( 100000, 42 ) );

        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( Encoded( m, bytes ) );

        COMCORE::CRpcMessage back = contract->NewMessage( "AllTypes" );
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( back.Decode( bytes.GetConstBufferPtr(), bytes.GetDataSize(), status ) );
        ASSERT_TRUE( back.GetBool( 1 ) );
        ASSERT_TRUE( -128 == back.GetInt32( 2 ) );
        ASSERT_TRUE( -32768 == back.GetInt32( 3 ) );
        ASSERT_TRUE( -2147483647 - 1 == back.GetInt32( 4 ) );
        ASSERT_TRUE( (Int64) 0x8000000000000000ULL == back.GetInt64( 5 ) );
        ASSERT_TRUE( 255 == back.GetUInt32( 6 ) );
        ASSERT_TRUE( 65535 == back.GetUInt32( 7 ) );
        ASSERT_TRUE( 4294967295UL == back.GetUInt32( 8 ) );
        ASSERT_TRUE( 0xFFFFFFFFFFFFFFFFULL == back.GetUInt64( 9 ) );
        CORE::CVariant f32;
        ASSERT_TRUE( back.Get( 10, f32 ) && GUCEF_DATATYPE_FLOAT32 == f32.GetTypeId() && -1.5f == f32.AsFloat32() );
        ASSERT_TRUE( 3.25 == back.GetFloat64( 11 ) );
        ASSERT_TRUE( CORE::ToUtf8String( back.GetString( 12 ) ) == text );
        CORE::CDynamicBuffer blobBack;
        ASSERT_TRUE( back.GetBytes( 13, blobBack ) && sizeof( blob ) == blobBack.GetDataSize() && 0 == memcmp( blob, blobBack.GetConstBufferPtr(), sizeof( blob ) ) );
        ASSERT_TRUE( back.GetDecimal( 14 ).IsIdentical( CORE::CDecimal::Parse( "-12.3400" ) ) );
        ASSERT_TRUE( back.GetDecimal( 15 ).IsIdentical( CORE::CDecimal::Parse( "1.5000" ) ) );
        ASSERT_TRUE( -10 == back.GetDate( 16 ) );
        ASSERT_TRUE( 1700000000123456LL == back.GetTimestamp( 17 ) );
        CORE::CDateTime localBack = back.GetDateTime( 18 );
        ASSERT_TRUE( localBack == local && 120 == localBack.GetTimeZoneUTCOffsetInMins() );
        ASSERT_TRUE( -2500000 == back.GetDuration( 19 ) );
        ASSERT_TRUE( -5 == back.GetEnum( 20 ) );
        const COMCORE::CRpcMessage* innerBack = back.GetMessage( 21 );
        ASSERT_TRUE( GUCEF_NULL != innerBack && 7 == innerBack->GetUInt32( 1 ) && "seven" == innerBack->GetString( 2 ) );
        CORE::CVariant cell;
        CORE::CDecimal cellDecimal;
        ASSERT_TRUE( back.Get( 22, cell ) && CORE::DECIMALSTATUS_OK == cell.GetDecimal( cellDecimal ) && cellDecimal.IsIdentical( CORE::CDecimal::Parse( "9.99" ) ) );
        CORE::CDataNode docBack;
        ASSERT_TRUE( back.GetDataNode( 23, docBack ) && "testing" == docBack.GetAttributeValue( "reason" ).AsString() );
        ASSERT_TRUE( 42 == back.GetUInt32( 100000 ) );

        // and the re-encoding is byte identical
        CORE::CDynamicBuffer again;
        ASSERT_TRUE( Encoded( back, again ) && again.GetDataSize() == bytes.GetDataSize() &&
                     0 == memcmp( again.GetConstBufferPtr(), bytes.GetConstBufferPtr(), bytes.GetDataSize() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: decimals keep their scale; a fixed scale refuses inexact values" )
    try
    {
        COMCORE::CRpcMessage m = contract->NewMessage( "AllTypes" );
        ASSERT_TRUE( m.SetDecimal( 14, CORE::CDecimal::Parse( "2.200" ) ) );
        ASSERT_TRUE( 3 == m.GetDecimal( 14 ).GetScale() );
        ASSERT_TRUE( m.SetDecimal( 15, CORE::CDecimal::Parse( "2.2" ) ) );            // rescales exactly to 2.2000
        ASSERT_TRUE( m.GetDecimal( 15 ).IsIdentical( CORE::CDecimal::Parse( "2.2000" ) ) );
        ASSERT_FALSE( m.SetDecimal( 15, CORE::CDecimal::Parse( "2.12345" ) ) );       // would lose a digit
        ASSERT_FALSE( m.Add( 30, CORE::CVariant( CORE::CDecimal::Parse( "0.001" ) ) ) );
        ASSERT_TRUE( m.Add( 30, CORE::CVariant( CORE::CDecimal::Parse( "0.1" ) ) ) );

        // fixed scale travels as a VARINT of the unscaled value: field 15, wire type 0, 22000 zig-zag
        COMCORE::CRpcMessage only = contract->NewMessage( "AllTypes" );
        ASSERT_TRUE( only.SetDecimal( 15, CORE::CDecimal::Parse( "2.2" ) ) );
        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( Encoded( only, bytes ) );
        const UInt8 expected[] = { 0x78, 0xE0, 0xD7, 0x02 };
        ASSERT_TRUE( BytesAre( bytes, expected, sizeof( expected ) ) );

        // a free scale above 18 on the wire is refused: field 14 LEN { unscaled 1, scale 19 }
        const UInt8 badScale[] = { 0x72, 0x02, 0x02, 0x13 };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", badScale, sizeof( badScale ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: repeated scalars are packed; readers accept both packed and unpacked" )
    try
    {
        COMCORE::CRpcMessage m = contract->NewMessage( "AllTypes" );
        ASSERT_TRUE( m.Add( 24, CORE::CVariant( (Int32) 1 ) ) );
        ASSERT_TRUE( m.Add( 24, CORE::CVariant( (Int32) -1 ) ) );
        ASSERT_TRUE( m.Add( 24, CORE::CVariant( (Int32) 150 ) ) );
        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( Encoded( m, bytes ) );
        // key 24<<3|2 = 0xC2 0x01, length 4, zig-zag 1 -> 02, -1 -> 01, 150 -> 300 = AC 02
        const UInt8 packed[] = { 0xC2, 0x01, 0x04, 0x02, 0x01, 0xAC, 0x02 };
        ASSERT_TRUE( BytesAre( bytes, packed, sizeof( packed ) ) );

        const UInt8 unpacked[] = { 0xC0, 0x01, 0x02, 0xC0, 0x01, 0x01, 0xC0, 0x01, 0xAC, 0x02 };
        COMCORE::CRpcMessage back = contract->NewMessage( "AllTypes" );
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( back.Decode( unpacked, sizeof( unpacked ), status ) );
        ASSERT_TRUE( 3 == back.GetCount( 24 ) );
        CORE::CVariant v;
        ASSERT_TRUE( back.GetAt( 24, 2, v ) && 150 == v.AsInt32() );

        // repeated strings and messages are one field per element
        COMCORE::CRpcMessage names = contract->NewMessage( "AllTypes" );
        names.Add( 25, CORE::CVariant( "a" ) );
        names.Add( 25, CORE::CVariant( "b" ) );
        names.AddMessage( 26 )->SetUInt32( 1, 1 );
        names.AddMessage( 26 )->SetUInt32( 1, 2 );
        ASSERT_TRUE( Encoded( names, bytes ) );
        COMCORE::CRpcMessage namesBack = contract->NewMessage( "AllTypes" );
        ASSERT_TRUE( namesBack.Decode( bytes.GetConstBufferPtr(), bytes.GetDataSize(), status ) );
        ASSERT_TRUE( 2 == namesBack.GetCount( 25 ) && 2 == namesBack.GetCount( 26 ) );
        ASSERT_TRUE( 2 == namesBack.GetMessageAt( 26, 1 )->GetUInt32( 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: map entries are sorted by key, so encoding is deterministic" )
    try
    {
        COMCORE::CRpcMessage a = contract->NewMessage( "AllTypes" );
        a.SetMapEntry( 27, CORE::CVariant( "zeta" ), CORE::CVariant( (UInt32) 1 ) );
        a.SetMapEntry( 27, CORE::CVariant( "alpha" ), CORE::CVariant( (UInt32) 2 ) );
        a.SetMapEntry( 27, CORE::CVariant( "Beta" ), CORE::CVariant( (UInt32) 3 ) );
        a.GetMutableMapMessage( 28, CORE::CVariant( (Int64) 10 ) )->SetUInt32( 1, 10 );
        a.GetMutableMapMessage( 28, CORE::CVariant( (Int64) -3 ) )->SetUInt32( 1, 3 );

        COMCORE::CRpcMessage b = contract->NewMessage( "AllTypes" );
        b.GetMutableMapMessage( 28, CORE::CVariant( (Int64) -3 ) )->SetUInt32( 1, 3 );
        b.SetMapEntry( 27, CORE::CVariant( "Beta" ), CORE::CVariant( (UInt32) 3 ) );
        b.SetMapEntry( 27, CORE::CVariant( "alpha" ), CORE::CVariant( (UInt32) 2 ) );
        b.GetMutableMapMessage( 28, CORE::CVariant( (Int64) 10 ) )->SetUInt32( 1, 10 );
        b.SetMapEntry( 27, CORE::CVariant( "zeta" ), CORE::CVariant( (UInt32) 1 ) );

        CORE::CDynamicBuffer ba, bb;
        ASSERT_TRUE( Encoded( a, ba ) && Encoded( b, bb ) );
        ASSERT_TRUE( ba.GetDataSize() == bb.GetDataSize() && 0 == memcmp( ba.GetConstBufferPtr(), bb.GetConstBufferPtr(), ba.GetDataSize() ) );

        COMCORE::CRpcMessage::TVariantVector keys;
        ASSERT_TRUE( a.GetMapKeys( 27, keys ) && 3 == keys.size() );
        ASSERT_TRUE( "Beta" == keys[ 0 ].AsString() && "alpha" == keys[ 1 ].AsString() && "zeta" == keys[ 2 ].AsString() );

        COMCORE::CRpcMessage back = contract->NewMessage( "AllTypes" );
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( back.Decode( ba.GetConstBufferPtr(), ba.GetDataSize(), status ) );
        CORE::CVariant value;
        ASSERT_TRUE( back.GetMapEntry( 27, CORE::CVariant( "alpha" ), value ) && 2 == value.AsUInt32() );
        const COMCORE::CRpcMessage* entry = back.GetMapMessage( 28, CORE::CVariant( (Int64) -3 ) );
        ASSERT_TRUE( GUCEF_NULL != entry && 3 == entry->GetUInt32( 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: every decode refusal of design 5.3 gives INVALID_ARGUMENT" )
    try
    {
        // wire type mismatch: field 1 (bool, VARINT) as LEN
        const UInt8 mismatch[] = { 0x0A, 0x01, 0x00 };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", mismatch, sizeof( mismatch ) ) );
        // int8 out of its width: field 2 = 200 zig-zag 400
        const UInt8 wide[] = { 0x10, 0x90, 0x03 };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", wide, sizeof( wide ) ) );
        // uint16 out of its width
        const UInt8 wideU[] = { 0x38, 0x80, 0x80, 0x04 };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", wideU, sizeof( wideU ) ) );
        // LEN beyond the buffer
        const UInt8 longLen[] = { 0x62, 0x05, 'a' };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", longLen, sizeof( longLen ) ) );
        // varint longer than 10 bytes
        const UInt8 longVarint[] = { 0x48, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01 };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", longVarint, sizeof( longVarint ) ) );
        // wire types 3, 4, 6, 7 are invalid, also for unknown fields
        const UInt8 group[] = { 0x0B };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", group, sizeof( group ) ) );
        const UInt8 unknownGroup[] = { 0xFB, 0x07 };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", unknownGroup, sizeof( unknownGroup ) ) );
        // field number 0
        const UInt8 zeroField[] = { 0x00, 0x01 };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", zeroField, sizeof( zeroField ) ) );
        // invalid UTF-8 in a string
        const UInt8 badUtf8[] = { 0x62, 0x02, 0xC0, 0x80 };
        ASSERT_TRUE( DecodeFails( contract, "AllTypes", badUtf8, sizeof( badUtf8 ) ) );

        // nesting deeper than 32
        COMCORE::CRpcMessage root = contract->NewMessage( "Node" );
        COMCORE::CRpcMessage* node = &root;
        for ( UInt32 i=0; i<40; ++i )
            node = node->GetMutableMessage( 1 );
        node->SetUInt32( 2, 1 );
        CORE::CDynamicBuffer deep;
        ASSERT_TRUE( Encoded( root, deep ) );
        ASSERT_TRUE( DecodeFails( contract, "Node", static_cast< const UInt8* >( deep.GetConstBufferPtr() ), deep.GetDataSize() ) );

        COMCORE::CRpcMessage shallowRoot = contract->NewMessage( "Node" );
        node = &shallowRoot;
        for ( UInt32 i=0; i<30; ++i )
            node = node->GetMutableMessage( 1 );
        CORE::CDynamicBuffer shallow;
        ASSERT_TRUE( Encoded( shallowRoot, shallow ) );
        COMCORE::CRpcMessage shallowBack = contract->NewMessage( "Node" );
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( shallowBack.Decode( shallow.GetConstBufferPtr(), shallow.GetDataSize(), status ) );

        // last value wins for a repeated single field
        const UInt8 twice[] = { 0x20, 0x02, 0x20, 0x04 };
        COMCORE::CRpcMessage lastWins = contract->NewMessage( "AllTypes" );
        ASSERT_TRUE( lastWins.Decode( twice, sizeof( twice ), status ) && 2 == lastWins.GetInt32( 4 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: unknown fields are skipped by wire type and kept through a relay" )
    try
    {
        COMCORE::CRpcMessage full = contract->NewMessage( "AllTypes" );
        full.SetBool( 1, true );
        full.SetUInt64( 9, 99 );
        full.SetString( 12, "kept" );
        full.SetFloat64( 11, 1.0 );
        full.Set( 10, CORE::CVariant( (Float32) 2.0f ) );
        full.SetDecimal( 14, CORE::CDecimal::Parse( "1.25" ) );
        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( Encoded( full, bytes ) );

        // an older peer knows fields 1 and 12 only
        COMCORE::CRpcMessage old = contract->NewMessage( "AllTypesOld" );
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( old.Decode( bytes.GetConstBufferPtr(), bytes.GetDataSize(), status ) );
        ASSERT_TRUE( old.GetBool( 1 ) && "kept" == old.GetString( 12 ) );
        ASSERT_TRUE( 4 == old.GetUnknownFieldCount() );

        // relayed: the re-encoding holds the fields the relay does not know, in number order
        CORE::CDynamicBuffer relayed;
        ASSERT_TRUE( Encoded( old, relayed ) );
        ASSERT_TRUE( relayed.GetDataSize() == bytes.GetDataSize() && 0 == memcmp( relayed.GetConstBufferPtr(), bytes.GetConstBufferPtr(), bytes.GetDataSize() ) );

        COMCORE::CRpcMessage dropping = contract->NewMessage( "AllTypesOld" );
        dropping.SetPreserveUnknownFields( false );
        ASSERT_TRUE( dropping.Decode( bytes.GetConstBufferPtr(), bytes.GetDataSize(), status ) );
        ASSERT_TRUE( 0 == dropping.GetUnknownFieldCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: setters refuse values outside the declared type" )
    try
    {
        COMCORE::CRpcMessage m = contract->NewMessage( "AllTypes" );
        ASSERT_FALSE( m.SetInt32( 2, 128 ) );           // int8
        ASSERT_FALSE( m.SetInt64( 6, -1 ) );            // uint8
        ASSERT_FALSE( m.SetUInt64( 4, 0x80000000ULL ) ); // int32
        ASSERT_FALSE( m.Set( 21, CORE::CVariant( 1 ) ) );  // a message field
        ASSERT_FALSE( m.SetUInt32( 999, 1 ) );          // no such field
        ASSERT_TRUE( m.Set( 20, CORE::CVariant( "COLOR_RED" ) ) && 1 == m.GetEnum( 20 ) );
        ASSERT_TRUE( m.Set( 16, CORE::CVariant( "1969-12-31" ) ) && -1 == m.GetDate( 16 ) );
        ASSERT_FALSE( m.IsBound() == false );
        ASSERT_FALSE( m.Has( 1 ) );
        m.SetBool( 1, false );
        ASSERT_TRUE( m.Has( 1 ) );
        m.Clear( 1 );
        ASSERT_FALSE( m.Has( 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: the JSON form round trips and redacts sensitive fields" )
    try
    {
        COMCORE::CRpcMessage m = contract->NewMessage( "AllTypes" );
        m.SetString( 12, "hello" );
        m.SetString( 29, "the password" );
        m.SetDecimal( 14, CORE::CDecimal::Parse( "-0.50" ) );
        m.SetDate( 16, 20000 );
        m.SetTimestamp( 17, 1700000000000001LL );
        m.SetEnum( 20, 1 );
        const UInt8 blob[] = { 1, 2, 3 };
        m.SetBytes( 13, blob, 3 );
        m.Add( 25, CORE::CVariant( "x" ) );
        m.GetMutableMessage( 21 )->SetUInt32( 1, 5 );
        m.SetMapEntry( 27, CORE::CVariant( "k" ), CORE::CVariant( (UInt32) 9 ) );

        CORE::CDataNode redacted;
        ASSERT_TRUE( m.ToDataNode( redacted ) );
        ASSERT_TRUE( GUCEF_CORE_REDACTION_MARKER == redacted.GetAttributeValue( "secret" ).AsString() );
        ASSERT_TRUE( "COLOR_RED" == redacted.GetAttributeValue( "color" ).AsString() );
        ASSERT_TRUE( "-0.50" == redacted.GetAttributeValue( "amount" ).AsString() );

        CORE::CDataNode full;
        ASSERT_TRUE( m.ToDataNode( full, false ) );
        ASSERT_TRUE( "the password" == full.GetAttributeValue( "secret" ).AsString() );
        COMCORE::CRpcMessage back = contract->NewMessage( "AllTypes" );
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( back.FromDataNode( full, status ) );
        CORE::CDynamicBuffer a, b;
        ASSERT_TRUE( Encoded( m, a ) && Encoded( back, b ) );
        ASSERT_TRUE( a.GetDataSize() == b.GetDataSize() && 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );

        // the required field rule
        COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
        ASSERT_FALSE( notes.IsNULL() );
        if ( !notes.IsNULL() )
        {
            COMCORE::CRpcMessage note = notes->NewMessage( "Note" );
            ASSERT_FALSE( note.Validate( status ) );
            note.SetString( 2, "title" );
            ASSERT_TRUE( note.Validate( status ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: a repeated map key keeps its first place and its last value; unknown fields keep number order" )
    try
    {
        // entries "b" = 1, "a" = 2, "b" = 3, "c" = 4, "a" = 5: decoded as b = 3, a = 5, c = 4 in that order
        const UInt8 entries[] = { 0xDA, 0x01, 0x05, 0x0A, 0x01, 'b', 0x10, 0x01,
                                  0xDA, 0x01, 0x05, 0x0A, 0x01, 'a', 0x10, 0x02,
                                  0xDA, 0x01, 0x05, 0x0A, 0x01, 'b', 0x10, 0x03,
                                  0xDA, 0x01, 0x05, 0x0A, 0x01, 'c', 0x10, 0x04,
                                  0xDA, 0x01, 0x05, 0x0A, 0x01, 'a', 0x10, 0x05 };
        COMCORE::CRpcMessage m = contract->NewMessage( "AllTypes" );
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( m.Decode( entries, sizeof( entries ), status ) );
        ASSERT_TRUE( 3 == m.GetCount( 27 ) );
        CORE::CVariant value;
        ASSERT_TRUE( m.GetMapEntry( 27, CORE::CVariant( "b" ), value ) && 3 == value.AsUInt32() );
        ASSERT_TRUE( m.GetMapEntry( 27, CORE::CVariant( "a" ), value ) && 5 == value.AsUInt32() );
        ASSERT_TRUE( m.GetMapEntry( 27, CORE::CVariant( "c" ), value ) && 4 == value.AsUInt32() );

        // message values: the replaced ones are released (address sanitizer), the last one is kept
        COMCORE::CRpcMessage byId = contract->NewMessage( "AllTypes" );
        byId.GetMutableMapMessage( 28, CORE::CVariant( (Int64) 7 ) )->SetUInt32( 1, 1 );
        CORE::CDynamicBuffer one;
        ASSERT_TRUE( Encoded( byId, one ) );
        byId.GetMutableMapMessage( 28, CORE::CVariant( (Int64) 7 ) )->SetUInt32( 1, 2 );
        CORE::CDynamicBuffer two;
        ASSERT_TRUE( Encoded( byId, two ) );
        CORE::CDynamicBuffer joined;
        joined.Append( one.GetConstBufferPtr(), one.GetDataSize() );
        joined.Append( two.GetConstBufferPtr(), two.GetDataSize() );
        joined.Append( one.GetConstBufferPtr(), one.GetDataSize() );
        joined.Append( two.GetConstBufferPtr(), two.GetDataSize() );
        COMCORE::CRpcMessage messages = contract->NewMessage( "AllTypes" );
        ASSERT_TRUE( messages.Decode( joined.GetConstBufferPtr(), joined.GetDataSize(), status ) );
        const COMCORE::CRpcMessage* entry = messages.GetMapMessage( 28, CORE::CVariant( (Int64) 7 ) );
        ASSERT_TRUE( 1 == messages.GetCount( 28 ) && GUCEF_NULL != entry && 2 == entry->GetUInt32( 1 ) );

        // unknown fields 200 = 1, 100 = 2, 200 = 3, 100 = 4 (an older peer knows 1 and 12 only): re-encoded in number
        // order, the ones of one number in the order they came
        const UInt8 unknown[] = { 0xC0, 0x0C, 0x01, 0xA0, 0x06, 0x02, 0xC0, 0x0C, 0x03, 0xA0, 0x06, 0x04 };
        COMCORE::CRpcMessage old = contract->NewMessage( "AllTypesOld" );
        ASSERT_TRUE( old.Decode( unknown, sizeof( unknown ), status ) && 4 == old.GetUnknownFieldCount() );
        CORE::CDynamicBuffer relayed;
        ASSERT_TRUE( Encoded( old, relayed ) );
        const UInt8 expected[] = { 0xA0, 0x06, 0x02, 0xA0, 0x06, 0x04, 0xC0, 0x0C, 0x01, 0xC0, 0x0C, 0x03 };
        ASSERT_TRUE( BytesAre( relayed, expected, sizeof( expected ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: decoding and encoding grow linearly with map entries and unknown fields" )
    try
    {
        // 8 times the entries may take about 8 times as long (n log n for the sort); a scan per entry took 64 times
        // (20000 map entries: 86 s). Timing is left to builds without sanitizers.
        const UInt32 small = 1000;
        const UInt32 large = 8000;
        CORE::CDynamicBuffer smallMap, largeMap, smallUnknown, largeUnknown;
        WriteMapEntries( smallMap, small );
        WriteMapEntries( largeMap, large );
        WriteUnknownFields( smallUnknown, small );
        WriteUnknownFields( largeUnknown, large );

        COMCORE::CRpcMessage m = contract->NewMessage( "AllTypes" );
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( m.Decode( largeMap.GetConstBufferPtr(), largeMap.GetDataSize(), status ) && large == m.GetCount( 27 ) );
        COMCORE::CRpcMessage::TVariantVector keys;
        ASSERT_TRUE( m.GetMapKeys( 27, keys ) && large == keys.size() && "k0" == keys[ 0 ].AsString() );
        CORE::CDynamicBuffer reencoded;
        ASSERT_TRUE( Encoded( m, reencoded ) && reencoded.GetDataSize() == largeMap.GetDataSize() );

        #if !defined( __SANITIZE_THREAD__ ) && !defined( __SANITIZE_ADDRESS__ )
        const char* what[] = { "decode map entries", "decode unknown fields", "encode map entries" };
        const CORE::CDynamicBuffer* smallBuffers[] = { &smallMap, &smallUnknown, &smallMap };
        const CORE::CDynamicBuffer* largeBuffers[] = { &largeMap, &largeUnknown, &largeMap };
        for ( UInt32 i=0; i<3; ++i )
        {
            bool smallOk = false;
            bool largeOk = false;
            double smallMs = BestCodecMs( contract, *smallBuffers[ i ], 2 == i, smallOk );
            double largeMs = BestCodecMs( contract, *largeBuffers[ i ], 2 == i, largeOk );
            ASSERT_TRUE( smallOk && largeOk );
            double ratio = largeMs / ( smallMs > 0.05 ? smallMs : 0.05 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcCodec: " << what[ i ] << ": " << small << " in " << smallMs << " ms, " << large <<
                       " in " << largeMs << " ms (x" << ratio << ")" );
            ASSERT_TRUE( ratio < 24.0 );
            ASSERT_TRUE( largeMs < 2000.0 );
        }
        #endif
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: status details from the peer that are cut short are dropped, never read beyond" )
    try
    {
        // details of 200 nested levels, cut at several points: the status is read, its details are left empty
        CORE::CDataNode root( "d", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* node = &root;
        for ( UInt32 i=0; i<200; ++i )
            node = node->AddChild( "c", GUCEF_DATATYPE_OBJECT );
        node->SetValue( CORE::CVariant( "leaf value" ) );
        CORE::CDynamicBuffer details;
        UInt32 written = 0;
        CORE::CDataNodeBinarySerializerOptions options;
        ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Serialize( options, root, 0, details, written ) && written > 13 );
        const UInt32 cuts[] = { 3, 7, 13, written / 2, written - 1, written };
        for ( UInt32 c=0; c<sizeof( cuts ) / sizeof( cuts[ 0 ] ); ++c )
        {
            CORE::CDynamicBuffer frame;
            CORE::CBinaryWireWriter writer( frame );
            writer.WriteVarUInt32( COMCORE::RPC_STATUS_INTERNAL );
            writer.WriteString( "m" );
            writer.WriteSizedBytes( details.GetConstBufferPtr(), cuts[ c ] );
            CORE::CBinaryWireReader reader( frame.GetConstBufferPtr(), frame.GetDataSize() );
            COMCORE::CRpcStatus status;
            ASSERT_TRUE( COMCORE::CRpcFrameCodec::ReadStatus( reader, status ) );
            ASSERT_TRUE( COMCORE::RPC_STATUS_INTERNAL == status.code && "m" == status.message );
            if ( cuts[ c ] < written )
                ASSERT_TRUE( 0 == status.details.GetNrOfChildNodes() );
        }

        // a length beyond the frame refuses the status
        CORE::CDynamicBuffer frame;
        CORE::CBinaryWireWriter writer( frame );
        writer.WriteVarUInt32( COMCORE::RPC_STATUS_INTERNAL );
        writer.WriteString( "m" );
        writer.WriteVarUInt32( 1000 );
        writer.WriteUInt8( 1 );
        CORE::CBinaryWireReader reader( frame.GetConstBufferPtr(), frame.GetDataSize() );
        COMCORE::CRpcStatus status;
        ASSERT_FALSE( COMCORE::CRpcFrameCodec::ReadStatus( reader, status ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_codec: bytes can link to the decoded buffer and Detach() copies them" )
    try
    {
        COMCORE::CRpcMessage m = contract->NewMessage( "AllTypes" );
        const UInt8 blob[] = { 9, 8, 7 };
        m.SetBytes( 13, blob, 3 );
        CORE::CDynamicBuffer bytes;
        ASSERT_TRUE( Encoded( m, bytes ) );
        COMCORE::CRpcMessage linked = contract->NewMessage( "AllTypes" );
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( linked.Decode( bytes.GetConstBufferPtr(), bytes.GetDataSize(), status, true ) );
        CORE::CVariant v;
        ASSERT_TRUE( linked.Get( 13, v ) && v.IsDynamicMemoryLinked() );
        linked.Detach();
        ASSERT_TRUE( linked.Get( 13, v ) && !v.IsDynamicMemoryLinked() );
        bytes.Clear( false );
        ASSERT_TRUE( linked.Get( 13, v ) && 3 == v.ByteSize( false ) && 0 == memcmp( v.AsVoidPtr(), blob, 3 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
