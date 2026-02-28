/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

#ifndef GUCEF_CORE_CGEOLOCATION_H
#include "gucefCORE_CGeoLocation.h"
#define GUCEF_CORE_CGEOLOCATION_H
#endif /* GUCEF_CORE_CGEOLOCATION_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVariantStream.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

void
PerformVariantStreamTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CVariantStream TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CVariantStream" );

    // Test 1: Basic construction and state
    GUCEF_TESTFW_TESTCASE( "Test 1: Basic construction and state" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Basic construction and state" );
        CORE::CVariantStream stream;
        ASSERT_TRUE( stream.GetDataSize() == 0 );
        ASSERT_TRUE( stream.GetWritePosition() == 0 );
        ASSERT_TRUE( stream.GetReadPosition() == 0 );
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_FALSE( stream.HasUnreadData() );
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Construction with initial size
    GUCEF_TESTFW_TESTCASE( "Test 2: Construction with initial size" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Construction with initial size" );
        CORE::CVariantStream stream( 1024 );
        ASSERT_TRUE( stream.GetDataSize() == 0 );
        ASSERT_TRUE( stream.IsValid() );
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Write and read boolean
    GUCEF_TESTFW_TESTCASE( "Test 3: Write and read boolean" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Write and read boolean" );
        CORE::CVariantStream stream;
        stream << true << false;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( stream.GetDataSize() > 0 );
        
        stream.ResetReadPosition();
        bool val1 = false, val2 = true;
        stream >> val1 >> val2;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( val1 == true );
        ASSERT_TRUE( val2 == false );
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Write and read integers
    GUCEF_TESTFW_TESTCASE( "Test 4: Write and read integers" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Write and read integers" );
        CORE::CVariantStream stream;
        Int8 i8 = -42;
        UInt8 u8 = 200;
        Int16 i16 = -1000;
        UInt16 u16 = 50000;
        Int32 i32 = -100000;
        UInt32 u32 = 3000000000UL;
        Int64 i64 = -9000000000000LL;
        UInt64 u64 = 18000000000000000000ULL;
        
        stream << i8 << u8 << i16 << u16 << i32 << u32 << i64 << u64;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        Int8 ri8 = 0;
        UInt8 ru8 = 0;
        Int16 ri16 = 0;
        UInt16 ru16 = 0;
        Int32 ri32 = 0;
        UInt32 ru32 = 0;
        Int64 ri64 = 0;
        UInt64 ru64 = 0;
        
        stream >> ri8 >> ru8 >> ri16 >> ru16 >> ri32 >> ru32 >> ri64 >> ru64;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( ri8 == i8 );
        ASSERT_TRUE( ru8 == u8 );
        ASSERT_TRUE( ri16 == i16 );
        ASSERT_TRUE( ru16 == u16 );
        ASSERT_TRUE( ri32 == i32 );
        ASSERT_TRUE( ru32 == u32 );
        ASSERT_TRUE( ri64 == i64 );
        ASSERT_TRUE( ru64 == u64 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Write and read floats
    GUCEF_TESTFW_TESTCASE( "Test 5: Write and read floats" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Write and read floats" );
        CORE::CVariantStream stream;
        Float32 f32 = 3.14159f;
        Float64 f64 = 2.71828182845904523536;
        
        stream << f32 << f64;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        Float32 rf32 = 0.0f;
        Float64 rf64 = 0.0;
        
        stream >> rf32 >> rf64;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( rf32 == f32 );
        ASSERT_TRUE( rf64 == f64 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Write and read strings
    GUCEF_TESTFW_TESTCASE( "Test 6: Write and read strings" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Write and read strings" );
        CORE::CVariantStream stream;
        const char* cstr = "Hello C-string";
        CORE::CAsciiString astr( "Hello ASCII" );
        CORE::CUtf8String ustr( "Hello UTF8" );
        std::string stdstr = "Hello std::string";
        
        stream << cstr << astr << ustr << stdstr;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        CORE::CUtf8String rcstr;
        CORE::CAsciiString rastr;
        CORE::CUtf8String rustr;
        std::string rstdstr;
        
        stream >> rcstr >> rastr >> rustr >> rstdstr;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( rcstr == cstr );
        ASSERT_TRUE( rastr == astr );
        ASSERT_TRUE( rustr == ustr );
        ASSERT_TRUE( rstdstr == stdstr );
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: Write and read CVariant
    GUCEF_TESTFW_TESTCASE( "Test 7: Write and read CVariant" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Write and read CVariant" );
        CORE::CVariantStream stream;
        CORE::CVariant var1( (Int32)12345 );
        CORE::CVariant var2( "Test variant string" );
        CORE::CVariant var3( 99.99 );
        
        stream << var1 << var2 << var3;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        CORE::CVariant rvar1, rvar2, rvar3;
        
        stream >> rvar1 >> rvar2 >> rvar3;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( rvar1.AsInt32() == 12345 );
        ASSERT_TRUE( rvar2.AsString() == "Test variant string" );
        ASSERT_TRUE( rvar3.AsFloat64() == 99.99 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: Write and read CDateTime
    GUCEF_TESTFW_TESTCASE( "Test 8: Write and read CDateTime" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: Write and read CDateTime" );
        CORE::CVariantStream stream;
        CORE::CDateTime dt = CORE::CDateTime::NowUTCDateTime();
        
        stream << dt;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        CORE::CDateTime rdt;
        
        stream >> rdt;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( rdt == dt );
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: Mixed types streaming (like logging)
    GUCEF_TESTFW_TESTCASE( "Test 9: Mixed types streaming (like logging)" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: Mixed types streaming (like logging)" );
        CORE::CVariantStream stream;
        
        // Simulate a log message: timestamp, level, message, value
        CORE::CDateTime timestamp = CORE::CDateTime::NowUTCDateTime();
        UInt32 logLevel = 3;
        const char* message = "Processing completed";
        Float64 duration = 1.234;
        Int32 itemCount = 42;
        
        stream << timestamp << logLevel << message << duration << itemCount;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( stream.GetDataSize() > 0 );
        
        // Read back
        stream.ResetReadPosition();
        CORE::CDateTime rTimestamp;
        UInt32 rLogLevel = 0;
        CORE::CUtf8String rMessage;
        Float64 rDuration = 0.0;
        Int32 rItemCount = 0;
        
        stream >> rTimestamp >> rLogLevel >> rMessage >> rDuration >> rItemCount;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( rTimestamp == timestamp );
        ASSERT_TRUE( rLogLevel == logLevel );
        ASSERT_TRUE( rMessage == message );
        ASSERT_TRUE( rDuration == duration );
        ASSERT_TRUE( rItemCount == itemCount );
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: Clear and reuse
    GUCEF_TESTFW_TESTCASE( "Test 10: Clear and reuse" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: Clear and reuse" );
        CORE::CVariantStream stream;
        stream << (Int32)100 << (Int32)200;
        ASSERT_TRUE( stream.GetDataSize() > 0 );
        
        stream.Clear();
        ASSERT_TRUE( stream.GetDataSize() == 0 );
        ASSERT_TRUE( stream.GetWritePosition() == 0 );
        ASSERT_TRUE( stream.GetReadPosition() == 0 );
        ASSERT_TRUE( stream.IsValid() );
        
        stream << (Int32)300;
        stream.ResetReadPosition();
        Int32 val = 0;
        stream >> val;
        ASSERT_TRUE( val == 300 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: Copy constructor
    GUCEF_TESTFW_TESTCASE( "Test 11: Copy constructor" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: Copy constructor" );
        CORE::CVariantStream stream1;
        stream1 << (Int32)111 << (Int32)222 << (Int32)333;
        
        CORE::CVariantStream stream2( stream1 );
        ASSERT_TRUE( stream2.GetDataSize() == stream1.GetDataSize() );
        
        stream2.ResetReadPosition();
        Int32 v1 = 0, v2 = 0, v3 = 0;
        stream2 >> v1 >> v2 >> v3;
        ASSERT_TRUE( v1 == 111 );
        ASSERT_TRUE( v2 == 222 );
        ASSERT_TRUE( v3 == 333 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: Assignment operator
    GUCEF_TESTFW_TESTCASE( "Test 12: Assignment operator" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: Assignment operator" );
        CORE::CVariantStream stream1;
        stream1 << (Float32)1.1f << (Float32)2.2f;
        
        CORE::CVariantStream stream2;
        stream2 = stream1;
        ASSERT_TRUE( stream2.GetDataSize() == stream1.GetDataSize() );
        
        stream2.ResetReadPosition();
        Float32 v1 = 0.0f, v2 = 0.0f;
        stream2 >> v1 >> v2;
        ASSERT_TRUE( v1 == 1.1f );
        ASSERT_TRUE( v2 == 2.2f );
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: HasUnreadData and GetUnreadDataSize
    GUCEF_TESTFW_TESTCASE( "Test 13: HasUnreadData and GetUnreadDataSize" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: HasUnreadData and GetUnreadDataSize" );
        CORE::CVariantStream stream;
        stream << (Int32)1 << (Int32)2 << (Int32)3;
        
        stream.ResetReadPosition();
        ASSERT_TRUE( stream.HasUnreadData() );
        UInt32 totalSize = stream.GetUnreadDataSize();
        ASSERT_TRUE( totalSize > 0 );
        
        Int32 val = 0;
        stream >> val;
        ASSERT_TRUE( stream.HasUnreadData() );
        ASSERT_TRUE( stream.GetUnreadDataSize() < totalSize );
        
        stream >> val >> val;
        ASSERT_FALSE( stream.HasUnreadData() );
        ASSERT_TRUE( stream.GetUnreadDataSize() == 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: SetReadPosition
    GUCEF_TESTFW_TESTCASE( "Test 14: SetReadPosition" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: SetReadPosition" );
        CORE::CVariantStream stream;
        stream << (Int32)100 << (Int32)200;
        
        // Read first value
        stream.ResetReadPosition();
        Int32 val = 0;
        stream >> val;
        ASSERT_TRUE( val == 100 );
        UInt32 positionAfterFirst = stream.GetReadPosition();
        
        // Read second value
        stream >> val;
        ASSERT_TRUE( val == 200 );
        
        // Go back and re-read second value
        stream.SetReadPosition( positionAfterFirst );
        stream >> val;
        ASSERT_TRUE( val == 200 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: Buffer access
    GUCEF_TESTFW_TESTCASE( "Test 15: Buffer access" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: Buffer access" );
        CORE::CVariantStream stream;
        stream << (Int32)42;
        
        const CORE::CDynamicBuffer& constBuf = stream.GetBuffer();
        ASSERT_TRUE( constBuf.GetDataSize() > 0 );
        
        CORE::CDynamicBuffer& mutBuf = stream.GetBuffer();
        ASSERT_TRUE( mutBuf.GetDataSize() > 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: CDynamicBuffer as value
    GUCEF_TESTFW_TESTCASE( "Test 16: CDynamicBuffer as value" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: CDynamicBuffer as value" );
        CORE::CVariantStream stream;
        CORE::CDynamicBuffer srcBuf( 16, true );
        srcBuf.AppendValue( (UInt32)0xDEADBEEF );
        srcBuf.AppendValue( (UInt32)0xCAFEBABE );
        
        stream << srcBuf;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        CORE::CDynamicBuffer dstBuf;
        stream >> dstBuf;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( dstBuf.GetDataSize() == srcBuf.GetDataSize() );
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: Pointer types
    GUCEF_TESTFW_TESTCASE( "Test 17: Pointer types" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: Pointer types" );
        CORE::CVariantStream stream;
        int testValue = 42;
        void* ptr = &testValue;
        
        stream << ptr;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( stream.GetDataSize() > 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: Chained operations
    GUCEF_TESTFW_TESTCASE( "Test 18: Chained operations" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: Chained operations" );
        CORE::CVariantStream stream;
        
        // Chain multiple writes
        stream << (Int32)1 << (Int32)2 << (Int32)3 << "test" << 4.5f << true;
        ASSERT_TRUE( stream.IsValid() );
        
        // Chain multiple reads
        stream.ResetReadPosition();
        Int32 a = 0, b = 0, c = 0;
        CORE::CUtf8String s;
        Float32 f = 0.0f;
        bool bl = false;
        
        stream >> a >> b >> c >> s >> f >> bl;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( a == 1 );
        ASSERT_TRUE( b == 2 );
        ASSERT_TRUE( c == 3 );
        ASSERT_TRUE( s == "test" );
        ASSERT_TRUE( f == 4.5f );
        ASSERT_TRUE( bl == true );
    GUCEF_TESTFW_TESTCASE_END

    // Test 19: ToString basic
    GUCEF_TESTFW_TESTCASE( "Test 19: ToString basic" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: ToString basic" );
        CORE::CVariantStream stream;
        stream << "Hello" << "World";
        
        CORE::CString result = stream.ToString();
        ASSERT_TRUE( result == "HelloWorld" );
    GUCEF_TESTFW_TESTCASE_END

    // Test 20: ToString with mixed types
    GUCEF_TESTFW_TESTCASE( "Test 20: ToString with mixed types" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 20: ToString with mixed types" );
        CORE::CVariantStream stream;
        stream << "Value=" << (Int32)42 << " Done";
        
        CORE::CString result = stream.ToString();
        ASSERT_TRUE( result == "Value=42 Done" );
    GUCEF_TESTFW_TESTCASE_END

    // Test 21: ToString with explicit spaces
    GUCEF_TESTFW_TESTCASE( "Test 21: ToString with explicit spaces" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: ToString with explicit spaces" );
        CORE::CVariantStream stream;
        stream << "A" << " " << "B" << " " << "C";
        
        CORE::CString result = stream.ToString();
        ASSERT_TRUE( result == "A B C" );
    GUCEF_TESTFW_TESTCASE_END

    // Test 22: ToString on empty stream
    GUCEF_TESTFW_TESTCASE( "Test 22: ToString on empty stream" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: ToString on empty stream" );
        CORE::CVariantStream stream;
        CORE::CString result = stream.ToString();
        ASSERT_TRUE( result.IsNULLOrEmpty() );
    GUCEF_TESTFW_TESTCASE_END

    // Test 23: WriteAsStringTo basic
    GUCEF_TESTFW_TESTCASE( "Test 23: WriteAsStringTo basic" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 23: WriteAsStringTo basic" );
        CORE::CVariantStream stream;
        stream << "Test" << (Int32)123;
        
        CORE::CDynamicBuffer destBuffer;
        UInt32 bytesWritten = stream.WriteAsStringTo( destBuffer );
        
        ASSERT_TRUE( bytesWritten > 0 );
        ASSERT_TRUE( destBuffer.GetDataSize() == bytesWritten );
        
        // Verify content matches ToString
        CORE::CString fromToString = stream.ToString();
        CORE::CString fromBuffer( destBuffer.AsConstTypePtr<char>(), destBuffer.GetDataSize() );
        ASSERT_TRUE( fromBuffer == fromToString );
    GUCEF_TESTFW_TESTCASE_END

    // Test 24: WriteAsStringTo with existing buffer content
    GUCEF_TESTFW_TESTCASE( "Test 24: WriteAsStringTo with existing buffer content" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 24: WriteAsStringTo with existing buffer content" );
        CORE::CVariantStream stream;
        stream << "Appended";
        
        CORE::CDynamicBuffer destBuffer;
        destBuffer.Append( "Prefix:", 7 );
        UInt32 initialSize = destBuffer.GetDataSize();
        
        UInt32 bytesWritten = stream.WriteAsStringTo( destBuffer );
        
        ASSERT_TRUE( bytesWritten > 0 );
        ASSERT_TRUE( destBuffer.GetDataSize() == initialSize + bytesWritten );
        
        CORE::CString fromBuffer( destBuffer.AsConstTypePtr<char>(), destBuffer.GetDataSize() );
        ASSERT_TRUE( fromBuffer == "Prefix:Appended" );
    GUCEF_TESTFW_TESTCASE_END

    // Test 25: WriteAsStringTo on empty stream
    GUCEF_TESTFW_TESTCASE( "Test 25: WriteAsStringTo on empty stream" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 25: WriteAsStringTo on empty stream" );
        CORE::CVariantStream stream;
        CORE::CDynamicBuffer destBuffer;
        
        UInt32 bytesWritten = stream.WriteAsStringTo( destBuffer );
        ASSERT_TRUE( bytesWritten == 0 );
        ASSERT_TRUE( destBuffer.GetDataSize() == 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 26: ToString preserves numeric precision
    GUCEF_TESTFW_TESTCASE( "Test 26: ToString preserves numeric formatting" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 26: ToString preserves numeric formatting" );
        CORE::CVariantStream stream;
        stream << (Float64)3.14159;
        
        CORE::CString result = stream.ToString();
        ASSERT_TRUE( result.HasSubstr( "3.14159" ) >= 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 27: Write std::stringstream to CVariantStream
    GUCEF_TESTFW_TESTCASE( "Test 27: Write std::stringstream to CVariantStream" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 27: Write std::stringstream to CVariantStream" );
        CORE::CVariantStream stream;
        std::stringstream ss;
        ss << "Hello from stringstream";
        
        stream << ss;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        CORE::CUtf8String result;
        stream >> result;
        ASSERT_TRUE( result == "Hello from stringstream" );
    GUCEF_TESTFW_TESTCASE_END

    // Test 28: Read CVariantStream to std::stringstream
    GUCEF_TESTFW_TESTCASE( "Test 28: Read CVariantStream to std::stringstream" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 28: Read CVariantStream to std::stringstream" );
        CORE::CVariantStream stream;
        stream << "Content for stringstream";
        
        stream.ResetReadPosition();
        std::stringstream ss;
        stream >> ss;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( ss.str() == "Content for stringstream" );
    GUCEF_TESTFW_TESTCASE_END

    // Test 29: Roundtrip std::stringstream
    GUCEF_TESTFW_TESTCASE( "Test 29: Roundtrip std::stringstream" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 29: Roundtrip std::stringstream" );
        CORE::CVariantStream stream;
        std::stringstream ssIn;
        ssIn << "Roundtrip test data";
        
        stream << ssIn;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        std::stringstream ssOut;
        stream >> ssOut;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( ssOut.str() == ssIn.str() );
    GUCEF_TESTFW_TESTCASE_END

    // Test 30: std::stringstream with numbers
    GUCEF_TESTFW_TESTCASE( "Test 30: std::stringstream with numbers" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 30: std::stringstream with numbers" );
        CORE::CVariantStream stream;
        std::stringstream ss;
        ss << "Value: " << 42 << " and " << 3.14;
        
        stream << ss;
        ASSERT_TRUE( stream.IsValid() );
        
        CORE::CString result = stream.ToString();
        ASSERT_TRUE( result == ss.str() );
    GUCEF_TESTFW_TESTCASE_END

    // Test 31: Write and read CTimestamp
    GUCEF_TESTFW_TESTCASE( "Test 31: Write and read CTimestamp" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 31: Write and read CTimestamp" );
        CORE::CVariantStream stream;
        CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
        
        stream << ts;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        CORE::CTimestamp rts;
        
        stream >> rts;
        ASSERT_TRUE( stream.IsValid() );
        // Compare as milliseconds since epoch for precision
        ASSERT_TRUE( rts.ToMillisecondsSinceEpoch() == ts.ToMillisecondsSinceEpoch() );
    GUCEF_TESTFW_TESTCASE_END

    // Test 32: CTimestamp roundtrip with multiple values
    GUCEF_TESTFW_TESTCASE( "Test 32: CTimestamp roundtrip with multiple values" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 32: CTimestamp roundtrip with multiple values" );
        CORE::CVariantStream stream;
        CORE::CTimestamp ts1 = CORE::CTimestamp::NowUTCTime();
        CORE::CTimestamp ts2 = CORE::CTimestamp::NowLocalTime();
        
        stream << ts1 << "separator" << ts2;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        CORE::CTimestamp rts1, rts2;
        CORE::CUtf8String sep;
        
        stream >> rts1 >> sep >> rts2;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( rts1.ToMillisecondsSinceEpoch() == ts1.ToMillisecondsSinceEpoch() );
        ASSERT_TRUE( sep == "separator" );
        ASSERT_TRUE( rts2.ToMillisecondsSinceEpoch() == ts2.ToMillisecondsSinceEpoch() );
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();

    // Test 33: CTimestamp ToString output
    GUCEF_TESTFW_TESTCASE( "Test 33: CTimestamp ToString output" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 33: CTimestamp ToString output" );
        CORE::CVariantStream stream;
        CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
        
        stream << ts;
        CORE::CString result = stream.ToString();

        // CTimestamp is stored as a UInt64 tick count; ToString() returns that numeric value
        ASSERT_TRUE( !result.IsNULLOrEmpty() );
    GUCEF_TESTFW_TESTCASE_END

    // Test 34: Write and read CGeoLocation
    GUCEF_TESTFW_TESTCASE( "Test 34: Write and read CGeoLocation" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 34: Write and read CGeoLocation" );
        CORE::CVariantStream stream;
        // New York City coordinates
        CORE::CGeoLocation geo( 40.7128, -74.0060, 10.0 );
        
        stream << geo;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        CORE::CGeoLocation rgeo;
        
        stream >> rgeo;
        ASSERT_TRUE( stream.IsValid() );
        // Compare with small tolerance for floating point
        Float64 latDiff = geo.GetLatitude() - rgeo.GetLatitude();
        Float64 lonDiff = geo.GetLongitude() - rgeo.GetLongitude();
        ASSERT_TRUE( latDiff < 0.0001 && latDiff > -0.0001 );
        ASSERT_TRUE( lonDiff < 0.0001 && lonDiff > -0.0001 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 35: CGeoLocation roundtrip with multiple values
    GUCEF_TESTFW_TESTCASE( "Test 35: CGeoLocation roundtrip with multiple values" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 35: CGeoLocation roundtrip with multiple values" );
        CORE::CVariantStream stream;
        // London and Paris
        CORE::CGeoLocation london( 51.5074, -0.1278, 11.0 );
        CORE::CGeoLocation paris( 48.8566, 2.3522, 35.0 );
        
        stream << london << "to" << paris;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        CORE::CGeoLocation rlondon, rparis;
        CORE::CUtf8String sep;
        
        stream >> rlondon >> sep >> rparis;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( sep == "to" );
        
        Float64 latDiff = london.GetLatitude() - rlondon.GetLatitude();
        ASSERT_TRUE( latDiff < 0.0001 && latDiff > -0.0001 );
        latDiff = paris.GetLatitude() - rparis.GetLatitude();
        ASSERT_TRUE( latDiff < 0.0001 && latDiff > -0.0001 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 36: CGeoLocation ToString output
    GUCEF_TESTFW_TESTCASE( "Test 36: CGeoLocation ToString output" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 36: CGeoLocation ToString output" );
        CORE::CVariantStream stream;
        CORE::CGeoLocation geo( 52.5200, 13.4050, 34.0 ); // Berlin
        
        stream << geo;
        CORE::CString result = stream.ToString();
        
        // ISO 6709 format starts with +/- for lat/lon
        ASSERT_TRUE( !result.IsNULLOrEmpty() );
        ASSERT_TRUE( result.HasChar( '+' ) >= 0 || result.HasChar( '-' ) >= 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 37: Write and read std::wstring
    GUCEF_TESTFW_TESTCASE( "Test 37: Write and read std::wstring" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 37: Write and read std::wstring" );
        CORE::CVariantStream stream;
        std::wstring wstr = L"Hello Wide World";
        
        stream << wstr;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        std::wstring rwstr;
        stream >> rwstr;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( rwstr == wstr );
    GUCEF_TESTFW_TESTCASE_END

    // Test 38: std::wstring with unicode characters
    GUCEF_TESTFW_TESTCASE( "Test 38: std::wstring with unicode characters" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 38: std::wstring with unicode characters" );
        CORE::CVariantStream stream;
        std::wstring wstr = L"Unicode: \u00C4\u00D6\u00DC\u00E4\u00F6\u00FC\u00DF"; // German umlauts
        
        stream << wstr;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        std::wstring rwstr;
        stream >> rwstr;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( rwstr == wstr );
    GUCEF_TESTFW_TESTCASE_END

    // Test 39: std::wstring mixed with other types
    GUCEF_TESTFW_TESTCASE( "Test 39: std::wstring mixed with other types" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 39: std::wstring mixed with other types" );
        CORE::CVariantStream stream;
        std::wstring wstr = L"Wide";
        std::string str = "Narrow";
        Int32 num = 42;
        
        stream << wstr << str << num;
        ASSERT_TRUE( stream.IsValid() );
        
        stream.ResetReadPosition();
        std::wstring rwstr;
        std::string rstr;
        Int32 rnum = 0;
        
        stream >> rwstr >> rstr >> rnum;
        ASSERT_TRUE( stream.IsValid() );
        ASSERT_TRUE( rwstr == wstr );
        ASSERT_TRUE( rstr == str );
        ASSERT_TRUE( rnum == num );
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CVariantStream TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
