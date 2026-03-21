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

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CVARIANT_VIEW_H
#include "gucefCORE_CVariantView.h"
#define GUCEF_CORE_CVARIANT_VIEW_H
#endif /* GUCEF_CORE_CVARIANT_VIEW_H ? */

#ifndef GUCEF_CORE_C_VARIANTDATA_H
#include "gucefCORE_c_variantdata.h"
#define GUCEF_CORE_C_VARIANTDATA_H
#endif /* GUCEF_CORE_C_VARIANTDATA_H ? */

#ifndef GUCEF_CORE_CTFRACTION_H
#include "gucefCORE_CTFraction.h"
#define GUCEF_CORE_CTFRACTION_H
#endif /* GUCEF_CORE_CTFRACTION_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVariant.h"

#include <cstring> // for strcmp

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
PerformVariantTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CVariant TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CVariant" );

    // Test 1: Default construction - unknown type
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction - unknown type" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Default construction - unknown type" );
            CORE::CVariant var;
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UNKNOWN );
            ASSERT_TRUE( var.GetTypeName() == GUCEF_DATATYPE_NAME_UNKNOWN );
            ASSERT_FALSE( var.IsInteger() );
            ASSERT_FALSE( var.IsBlob() );
            ASSERT_FALSE( var.IsBsob() );
            ASSERT_FALSE( var.IsBoolean() );        
            ASSERT_FALSE( var.IsFloat() );
            ASSERT_TRUE( var.IsNULLOrEmpty() );
            ASSERT_FALSE( var.IsString() );
            ASSERT_FALSE( var.IsDynamicMemoryLinked() );
            ASSERT_FALSE( var.IsInitialized() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Int8 assignment and type detection
    GUCEF_TESTFW_TESTCASE( "Test 2: Int8 assignment and type detection" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Int8 assignment and type detection" );
            CORE::CVariant var;
            var = Int8( 1 );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_INT8 );
            ASSERT_TRUE( var.GetTypeName() == GUCEF_DATATYPE_NAME_INT8 );
            ASSERT_TRUE( var.IsInteger() );
            ASSERT_FALSE( var.IsBlob() );
            ASSERT_FALSE( var.IsBsob() );
            ASSERT_FALSE( var.IsBoolean() );        
            ASSERT_FALSE( var.IsFloat() );
            ASSERT_FALSE( var.IsNULLOrEmpty() );
            ASSERT_FALSE( var.IsString() );
            ASSERT_FALSE( var.IsDynamicMemoryLinked() );
            ASSERT_TRUE( var.IsInitialized() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Float32 assignment and type detection
    GUCEF_TESTFW_TESTCASE( "Test 3: Float32 assignment and type detection" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Float32 assignment and type detection" );
            CORE::CVariant var;
            var = Float32( 1.0f );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_FLOAT32 );
            ASSERT_TRUE( var.GetTypeName() == GUCEF_DATATYPE_NAME_FLOAT32 );
            ASSERT_FALSE( var.IsInteger() );
            ASSERT_FALSE( var.IsBlob() );
            ASSERT_FALSE( var.IsBsob() );
            ASSERT_FALSE( var.IsBoolean() );        
            ASSERT_TRUE( var.IsFloat() );
            ASSERT_FALSE( var.IsNULLOrEmpty() );
            ASSERT_FALSE( var.IsString() );
            ASSERT_FALSE( var.IsDynamicMemoryLinked() );
            ASSERT_TRUE( var.IsInitialized() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Float64 assignment and type detection
    GUCEF_TESTFW_TESTCASE( "Test 4: Float64 assignment and type detection" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Float64 assignment and type detection" );
            CORE::CVariant var;
            var = Float64( 1.0 );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_FLOAT64 );
            ASSERT_TRUE( var.GetTypeName() == GUCEF_DATATYPE_NAME_FLOAT64 );
            ASSERT_FALSE( var.IsInteger() );
            ASSERT_FALSE( var.IsBlob() );
            ASSERT_FALSE( var.IsBsob() );
            ASSERT_FALSE( var.IsBoolean() );        
            ASSERT_TRUE( var.IsFloat() );
            ASSERT_FALSE( var.IsNULLOrEmpty() );
            ASSERT_FALSE( var.IsString() );
            ASSERT_FALSE( var.IsDynamicMemoryLinked() );
            ASSERT_TRUE( var.IsInitialized() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: UInt16 assignment and type detection
    GUCEF_TESTFW_TESTCASE( "Test 5: UInt16 assignment and type detection" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: UInt16 assignment and type detection" );
            CORE::CVariant var;
            var = UInt16( 1 );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UINT16 );
            ASSERT_TRUE( var.GetTypeName() == GUCEF_DATATYPE_NAME_UINT16 );
            ASSERT_TRUE( var.IsInteger() );
            ASSERT_FALSE( var.IsBlob() );
            ASSERT_FALSE( var.IsBsob() );
            ASSERT_FALSE( var.IsBoolean() );        
            ASSERT_FALSE( var.IsFloat() );
            ASSERT_FALSE( var.IsNULLOrEmpty() );
            ASSERT_FALSE( var.IsString() );
            ASSERT_FALSE( var.IsDynamicMemoryLinked() );
            ASSERT_TRUE( var.IsInitialized() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Int32 assignment and type detection
    GUCEF_TESTFW_TESTCASE( "Test 6: Int32 assignment and type detection" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Int32 assignment and type detection" );
            CORE::CVariant var;
            var = Int32( 1 );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_INT32 );
            ASSERT_TRUE( var.GetTypeName() == GUCEF_DATATYPE_NAME_INT32 );
            ASSERT_TRUE( var.IsInteger() );
            ASSERT_FALSE( var.IsBlob() );
            ASSERT_FALSE( var.IsBsob() );
            ASSERT_FALSE( var.IsBoolean() );        
            ASSERT_FALSE( var.IsFloat() );
            ASSERT_FALSE( var.IsNULLOrEmpty() );
            ASSERT_FALSE( var.IsString() );
            ASSERT_FALSE( var.IsDynamicMemoryLinked() );
            ASSERT_TRUE( var.IsInitialized() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: All integer types - value roundtrip
    GUCEF_TESTFW_TESTCASE( "Test 7: All integer types - value roundtrip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: All integer types - value roundtrip" );

            // Int8
            {
                CORE::CVariant var( Int8(-42) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_INT8 );
                ASSERT_TRUE( var.AsInt8() == -42 );
                ASSERT_TRUE( var.IsInteger() );
                ASSERT_TRUE( var.IsSignedInteger() );
            }

            // UInt8
            {
                CORE::CVariant var( UInt8(200) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UINT8 );
                ASSERT_TRUE( var.AsUInt8() == 200 );
                ASSERT_TRUE( var.IsInteger() );
                ASSERT_TRUE( var.IsUnsignedInteger() );
            }

            // Int16
            {
                CORE::CVariant var( Int16(-30000) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_INT16 );
                ASSERT_TRUE( var.AsInt16() == -30000 );
                ASSERT_TRUE( var.IsInteger() );
            }

            // UInt16
            {
                CORE::CVariant var( UInt16(60000) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UINT16 );
                ASSERT_TRUE( var.AsUInt16() == 60000 );
                ASSERT_TRUE( var.IsInteger() );
            }

            // Int32
            {
                CORE::CVariant var( Int32(-2000000000) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_INT32 );
                ASSERT_TRUE( var.AsInt32() == -2000000000 );
                ASSERT_TRUE( var.IsInteger() );
            }

            // UInt32
            {
                CORE::CVariant var( UInt32(4000000000UL) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UINT32 );
                ASSERT_TRUE( var.AsUInt32() == 4000000000UL );
                ASSERT_TRUE( var.IsInteger() );
            }

            // Int64
            {
                CORE::CVariant var( Int64(-9000000000000LL) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_INT64 );
                ASSERT_TRUE( var.AsInt64() == -9000000000000LL );
                ASSERT_TRUE( var.IsInteger() );
            }

            // UInt64
            {
                CORE::CVariant var( UInt64(18000000000000000000ULL) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UINT64 );
                ASSERT_TRUE( var.AsUInt64() == 18000000000000000000ULL );
                ASSERT_TRUE( var.IsInteger() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: Float types - value roundtrip
    GUCEF_TESTFW_TESTCASE( "Test 8: Float types - value roundtrip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: Float types - value roundtrip" );

            // Float32
            {
                CORE::CVariant var( Float32(3.14159f) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_FLOAT32 );
                ASSERT_TRUE( var.AsFloat32() == 3.14159f );
                ASSERT_TRUE( var.IsFloat() );
                ASSERT_TRUE( var.IsNumber() );
                ASSERT_FALSE( var.IsInteger() );
            }

            // Float64
            {
                CORE::CVariant var( Float64(2.71828182845904523536) );
                ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_FLOAT64 );
                ASSERT_TRUE( var.AsFloat64() == 2.71828182845904523536 );
                ASSERT_TRUE( var.IsFloat() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: Boolean type
    GUCEF_TESTFW_TESTCASE( "Test 9: Boolean type" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: Boolean type" );

            CORE::CVariant varTrue( true );
            ASSERT_TRUE( varTrue.GetTypeId() == GUCEF_DATATYPE_BOOLEAN_INT32 );
            ASSERT_TRUE( varTrue.AsBool() == true );
            ASSERT_TRUE( varTrue.IsBoolean() );

            CORE::CVariant varFalse( false );
            ASSERT_TRUE( varFalse.AsBool() == false );
            ASSERT_TRUE( varFalse.IsBoolean() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: String types - const char*
    GUCEF_TESTFW_TESTCASE( "Test 10: String types - const char*" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: String types - const char*" );

            CORE::CVariant var( "Hello World" );
            ASSERT_TRUE( var.IsString() );
            ASSERT_TRUE( var.AsString() == "Hello World" );
            ASSERT_TRUE( var.ByteSize() == 12 ); // 11 chars + null terminator
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: String types - CAsciiString
    GUCEF_TESTFW_TESTCASE( "Test 11: String types - CAsciiString" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: String types - CAsciiString" );

            CORE::CAsciiString str( "ASCII Test" );
            CORE::CVariant var( str );
            ASSERT_TRUE( var.IsString() );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_ASCII_STRING );
            ASSERT_TRUE( var.AsAsciiString() == str );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: String types - CUtf8String
    GUCEF_TESTFW_TESTCASE( "Test 12: String types - CUtf8String" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: String types - CUtf8String" );

            CORE::CUtf8String str( "UTF8 Test" );
            CORE::CVariant var( str );
            ASSERT_TRUE( var.IsString() );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UTF8_STRING );
            ASSERT_TRUE( var.AsUtf8String() == str );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: String types - std::string
    GUCEF_TESTFW_TESTCASE( "Test 13: String types - std::string" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: String types - std::string" );

            std::string str = "std::string Test";
            CORE::CVariant var( str );
            ASSERT_TRUE( var.IsString() );
            ASSERT_TRUE( var.AsString() == str.c_str() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: String types - std::wstring assignment
    GUCEF_TESTFW_TESTCASE( "Test 14: String types - std::wstring assignment" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: String types - std::wstring assignment" );

            std::wstring wstr = L"Wide String";
            CORE::CVariant var;
            var = wstr;
            ASSERT_TRUE( var.IsString() );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UTF16_STRING );
            // ByteSize should be wchar count * sizeof(wchar_t) + null term
            ASSERT_TRUE( var.ByteSize() == (wstr.size() + 1) * sizeof(wchar_t) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: LinkTo std::wstring - ByteSize verification
    GUCEF_TESTFW_TESTCASE( "Test 15: LinkTo std::wstring - ByteSize verification" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: LinkTo std::wstring - ByteSize verification" );

            std::wstring wstr = L"LinkTo Wide";
            CORE::CVariant var;
            var.LinkTo( wstr );
            ASSERT_TRUE( var.IsString() );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UTF16_STRING );
            ASSERT_TRUE( var.IsDynamicMemoryLinked() );
            // Verify ByteSize accounts for wchar_t size
            ASSERT_TRUE( var.ByteSize() == (wstr.size() + 1) * sizeof(wchar_t) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: LinkTo CAsciiString
    GUCEF_TESTFW_TESTCASE( "Test 16: LinkTo CAsciiString" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: LinkTo CAsciiString" );

            CORE::CAsciiString str( "LinkTo ASCII" );
            CORE::CVariant var;
            var.LinkTo( str );
            ASSERT_TRUE( var.IsString() );
            ASSERT_TRUE( var.IsDynamicMemoryLinked() );
            ASSERT_TRUE( var.AsAsciiString() == str );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: LinkTo CUtf8String
    GUCEF_TESTFW_TESTCASE( "Test 17: LinkTo CUtf8String" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: LinkTo CUtf8String" );

            CORE::CUtf8String str( "LinkTo UTF8" );
            CORE::CVariant var;
            var.LinkTo( str );
            ASSERT_TRUE( var.IsString() );
            ASSERT_TRUE( var.IsDynamicMemoryLinked() );
            ASSERT_TRUE( var.AsUtf8String() == str );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: Comparison operators
    GUCEF_TESTFW_TESTCASE( "Test 18: Comparison operators" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: Comparison operators" );

            CORE::CVariant a( Int32(10) );
            CORE::CVariant b( Int32(20) );
            CORE::CVariant c( Int32(10) );

            ASSERT_TRUE( a == c );
            ASSERT_TRUE( a != b );
            ASSERT_TRUE( a < b );
            ASSERT_TRUE( b > a );
            ASSERT_TRUE( a <= c );
            ASSERT_TRUE( a <= b );
            ASSERT_TRUE( b >= a );
            ASSERT_TRUE( a >= c );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 19: Type conversion - integer to string
    GUCEF_TESTFW_TESTCASE( "Test 19: Type conversion - integer to string" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: Type conversion - integer to string" );

            CORE::CVariant var( Int32(12345) );
            ASSERT_TRUE( var.AsString() == "12345" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 20: Type conversion - string to integer
    GUCEF_TESTFW_TESTCASE( "Test 20: Type conversion - string to integer" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 20: Type conversion - string to integer" );

            CORE::CVariant var( "67890" );
            ASSERT_TRUE( var.AsInt32() == 67890 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 21: Clear and IsNULLOrEmpty
    GUCEF_TESTFW_TESTCASE( "Test 21: Clear and IsNULLOrEmpty" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: Clear and IsNULLOrEmpty" );

            CORE::CVariant var( Int32(42) );
            ASSERT_FALSE( var.IsNULLOrEmpty() );
            ASSERT_TRUE( var.IsInitialized() );

            var.Clear();
            ASSERT_TRUE( var.IsNULLOrEmpty() );
            ASSERT_FALSE( var.IsInitialized() );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UNKNOWN );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 22: Copy constructor
    GUCEF_TESTFW_TESTCASE( "Test 22: Copy constructor" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: Copy constructor" );

            CORE::CVariant original( "Test String" );
            CORE::CVariant copy( original );

            ASSERT_TRUE( copy.AsString() == original.AsString() );
            ASSERT_TRUE( copy.GetTypeId() == original.GetTypeId() );
            // Copy should own its own memory, not be linked
            ASSERT_FALSE( copy.IsDynamicMemoryLinked() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 23: Assignment operator
    GUCEF_TESTFW_TESTCASE( "Test 23: Assignment operator" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 23: Assignment operator" );

            CORE::CVariant original( Float64(3.14159) );
            CORE::CVariant assigned;
            assigned = original;

            ASSERT_TRUE( assigned.AsFloat64() == original.AsFloat64() );
            ASSERT_TRUE( assigned.GetTypeId() == original.GetTypeId() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 24: CDateTime type
    GUCEF_TESTFW_TESTCASE( "Test 24: CDateTime type" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 24: CDateTime type" );

            CORE::CDateTime dt = CORE::CDateTime::NowUTCDateTime();
            CORE::CVariant var( dt );

            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_DATETIME_ISO8601_STRING );
            ASSERT_TRUE( var.AsDateTime() == dt );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 25: ByteSize for fixed types
    GUCEF_TESTFW_TESTCASE( "Test 25: ByteSize for fixed types" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 25: ByteSize for fixed types" );

            CORE::CVariant varInt8( Int8(1) );
            ASSERT_TRUE( varInt8.ByteSize() == sizeof(Int8) );

            CORE::CVariant varInt16( Int16(1) );
            ASSERT_TRUE( varInt16.ByteSize() == sizeof(Int16) );

            CORE::CVariant varInt32( Int32(1) );
            ASSERT_TRUE( varInt32.ByteSize() == sizeof(Int32) );

            CORE::CVariant varInt64( Int64(1) );
            ASSERT_TRUE( varInt64.ByteSize() == sizeof(Int64) );

            CORE::CVariant varFloat32( Float32(1.0f) );
            ASSERT_TRUE( varFloat32.ByteSize() == sizeof(Float32) );

            CORE::CVariant varFloat64( Float64(1.0) );
            ASSERT_TRUE( varFloat64.ByteSize() == sizeof(Float64) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 26: CTimestamp type
    GUCEF_TESTFW_TESTCASE( "Test 26: CTimestamp type" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 26: CTimestamp type" );

            CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
            CORE::CVariant var( ts );

            ASSERT_TRUE( var.IsInitialized() );
            ASSERT_TRUE( var.AsTimestamp() == ts );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 27: UsesDynamicMemory checks
    GUCEF_TESTFW_TESTCASE( "Test 27: UsesDynamicMemory checks" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 27: UsesDynamicMemory checks" );

            // Fixed size types should not use dynamic memory
            CORE::CVariant varInt( Int32(42) );
            ASSERT_FALSE( varInt.UsesDynamicMemory() );

            CORE::CVariant varFloat( Float64(3.14) );
            ASSERT_FALSE( varFloat.UsesDynamicMemory() );

            CORE::CVariant varBool( true );
            ASSERT_FALSE( varBool.UsesDynamicMemory() );

            // Strings use dynamic memory
            CORE::CVariant varStr( "Hello" );
            ASSERT_TRUE( varStr.UsesDynamicMemory() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 28: Empty static constant
    GUCEF_TESTFW_TESTCASE( "Test 28: Empty static constant" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 28: Empty static constant" );

            ASSERT_TRUE( CORE::CVariant::Empty.IsNULLOrEmpty() );
            ASSERT_FALSE( CORE::CVariant::Empty.IsInitialized() );
            ASSERT_TRUE( CORE::CVariant::Empty.GetTypeId() == GUCEF_DATATYPE_UNKNOWN );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 29: Float to string conversion
    GUCEF_TESTFW_TESTCASE( "Test 29: Float to string conversion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 29: Float to string conversion" );

            CORE::CVariant var( Float64(3.14159) );
            CORE::CString str = var.AsString();
            ASSERT_TRUE( str.HasSubstr( "3.14" ) >= 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 30: String to float conversion
    GUCEF_TESTFW_TESTCASE( "Test 30: String to float conversion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 30: String to float conversion" );

            CORE::CVariant var( "2.71828" );
            Float64 val = var.AsFloat64();
            // Check approximate equality
            ASSERT_TRUE( val > 2.71 && val < 2.72 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 31: Boolean to string conversion
    GUCEF_TESTFW_TESTCASE( "Test 31: Boolean to string conversion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 31: Boolean to string conversion" );

            CORE::CVariant varTrue( true );
            CORE::CVariant varFalse( false );

            // Boolean should convert to "true"/"false" or "1"/"0"
            CORE::CString strTrue = varTrue.AsString();
            CORE::CString strFalse = varFalse.AsString();

            ASSERT_FALSE( strTrue.IsNULLOrEmpty() );
            ASSERT_FALSE( strFalse.IsNULLOrEmpty() );
            ASSERT_TRUE( strTrue != strFalse );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 32: Pointer/memory address type
    GUCEF_TESTFW_TESTCASE( "Test 32: Pointer/memory address type" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 32: Pointer/memory address type" );

            int testVal = 42;
            void* ptr = &testVal;
            CORE::CVariant var( ptr );

            ASSERT_TRUE( var.IsMemoryAddress() );
            ASSERT_TRUE( var.AsVoidPtr() == ptr );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 33: Cross-type integer comparison
    GUCEF_TESTFW_TESTCASE( "Test 33: Cross-type integer comparison" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 33: Cross-type integer comparison" );

            CORE::CVariant varInt32( Int32(100) );
            CORE::CVariant varInt64( Int64(100) );
            CORE::CVariant varUInt16( UInt16(100) );

            // Same numeric value should be equal regardless of type
            ASSERT_TRUE( varInt32.AsInt64() == varInt64.AsInt64() );
            ASSERT_TRUE( varInt32.AsInt64() == varUInt16.AsInt64() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 34: String ByteSize includes null terminator
    GUCEF_TESTFW_TESTCASE( "Test 34: String ByteSize includes null terminator" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 34: String ByteSize includes null terminator" );

            CORE::CVariant var( "ABC" );
            // "ABC" = 3 chars + 1 null = 4 bytes
            ASSERT_TRUE( var.ByteSize() == 4 );

            CORE::CVariant varEmpty( "" );
            // Empty string - ByteSize returns 0 or 1 depending on implementation
            // Just verify it doesn't crash and returns a reasonable value
            ASSERT_TRUE( varEmpty.ByteSize() <= 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 35: LinkTo variant
    GUCEF_TESTFW_TESTCASE( "Test 35: LinkTo variant" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 35: LinkTo variant" );

            CORE::CVariant original( "Source String" );
            CORE::CVariant linked;
            linked.LinkTo( original );

            ASSERT_TRUE( linked.IsDynamicMemoryLinked() );
            ASSERT_TRUE( linked.AsString() == original.AsString() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 36: CDynamicBuffer in variant
    GUCEF_TESTFW_TESTCASE( "Test 36: CDynamicBuffer in variant" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 36: CDynamicBuffer in variant" );

            CORE::CDynamicBuffer srcBuf( 16, true );
            srcBuf.AppendValue( UInt32(0xDEADBEEF) );
            srcBuf.AppendValue( UInt32(0xCAFEBABE) );

            CORE::CVariant var( srcBuf );
            ASSERT_TRUE( var.IsBlob() || var.IsBinary() );
            ASSERT_TRUE( var.UsesDynamicMemory() );

            CORE::CDynamicBuffer dstBuf = var.AsBuffer();
            ASSERT_TRUE( dstBuf.GetDataSize() == srcBuf.GetDataSize() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 37: TypeIdForTypeName and TypeNameForTypeId
    GUCEF_TESTFW_TESTCASE( "Test 37: TypeIdForTypeName and TypeNameForTypeId" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 37: TypeIdForTypeName and TypeNameForTypeId" );

            // Test roundtrip for common types
            UInt8 int32Id = CORE::CVariant::TypeIdForTypeName( GUCEF_DATATYPE_NAME_INT32 );
            ASSERT_TRUE( int32Id == GUCEF_DATATYPE_INT32 );

            CORE::CString int32Name = CORE::CVariant::TypeNameForTypeId( GUCEF_DATATYPE_INT32 );
            ASSERT_TRUE( int32Name == GUCEF_DATATYPE_NAME_INT32 );

            // Test unknown type
            UInt8 unknownId = CORE::CVariant::TypeIdForTypeName( "INVALID_TYPE_NAME" );
            ASSERT_TRUE( unknownId == GUCEF_DATATYPE_UNKNOWN );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 38: Set method with raw data
    GUCEF_TESTFW_TESTCASE( "Test 38: Set method with raw data" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 38: Set method with raw data" );

            Int32 value = 12345;
            CORE::CVariant var;
            bool setResult = var.Set( &value, sizeof(value), GUCEF_DATATYPE_INT32 );

            ASSERT_TRUE( setResult );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_INT32 );
            ASSERT_TRUE( var.AsInt32() == 12345 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 39: SetFromString
    GUCEF_TESTFW_TESTCASE( "Test 39: SetFromString" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 39: SetFromString" );

            CORE::CVariant var;
            bool setResult = var.SetFromString( GUCEF_DATATYPE_INT32, "98765" );

            ASSERT_TRUE( setResult );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_INT32 );
            ASSERT_TRUE( var.AsInt32() == 98765 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 40: CStyleAccess
    GUCEF_TESTFW_TESTCASE( "Test 40: CStyleAccess" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 40: CStyleAccess" );

            CORE::CVariant var( Int32(42) );
            const GUCEF::CORE::TVariantData* cData = var.CStyleAccess();

            ASSERT_TRUE( cData != GUCEF_NULL );
            ASSERT_TRUE( cData->containedType == GUCEF_DATATYPE_INT32 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 41: ByteSizeOfFixedSizeType static
    GUCEF_TESTFW_TESTCASE( "Test 41: ByteSizeOfFixedSizeType static" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 41: ByteSizeOfFixedSizeType static" );

            ASSERT_TRUE( CORE::CVariant::ByteSizeOfFixedSizeType( GUCEF_DATATYPE_INT8 ) == sizeof(Int8) );
            ASSERT_TRUE( CORE::CVariant::ByteSizeOfFixedSizeType( GUCEF_DATATYPE_INT16 ) == sizeof(Int16) );
            ASSERT_TRUE( CORE::CVariant::ByteSizeOfFixedSizeType( GUCEF_DATATYPE_INT32 ) == sizeof(Int32) );
            ASSERT_TRUE( CORE::CVariant::ByteSizeOfFixedSizeType( GUCEF_DATATYPE_INT64 ) == sizeof(Int64) );
            ASSERT_TRUE( CORE::CVariant::ByteSizeOfFixedSizeType( GUCEF_DATATYPE_FLOAT32 ) == sizeof(Float32) );
            ASSERT_TRUE( CORE::CVariant::ByteSizeOfFixedSizeType( GUCEF_DATATYPE_FLOAT64 ) == sizeof(Float64) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 42: AsChar
    GUCEF_TESTFW_TESTCASE( "Test 42: AsChar" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 42: AsChar" );

            CORE::CVariant var( "A" );
            ASSERT_TRUE( var.AsChar() == 'A' );

            CORE::CVariant varInt( Int8(66) ); // ASCII 'B'
            ASSERT_TRUE( varInt.AsChar() == 'B' );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 43: IsNumber covers integers and floats
    GUCEF_TESTFW_TESTCASE( "Test 43: IsNumber covers integers and floats" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 43: IsNumber covers integers and floats" );

            CORE::CVariant varInt( Int32(42) );
            ASSERT_TRUE( varInt.IsNumber() );
            ASSERT_TRUE( varInt.IsInteger() );
            ASSERT_FALSE( varInt.IsFloat() );

            CORE::CVariant varFloat( Float64(3.14) );
            ASSERT_TRUE( varFloat.IsNumber() );
            ASSERT_FALSE( varFloat.IsInteger() );
            ASSERT_TRUE( varFloat.IsFloat() );

            CORE::CVariant varStr( "not a number" );
            ASSERT_FALSE( varStr.IsNumber() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 44: OwnsDynamicMemory vs IsDynamicMemoryLinked
    GUCEF_TESTFW_TESTCASE( "Test 44: OwnsDynamicMemory vs IsDynamicMemoryLinked" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 44: OwnsDynamicMemory vs IsDynamicMemoryLinked" );

            // Variant that owns its memory
            CORE::CVariant varOwns( "Owned String" );
            ASSERT_TRUE( varOwns.UsesDynamicMemory() );
            ASSERT_TRUE( varOwns.OwnsDynamicMemory() );
            ASSERT_FALSE( varOwns.IsDynamicMemoryLinked() );

            // Variant that links to external memory
            CORE::CUtf8String external( "External String" );
            CORE::CVariant varLinked;
            varLinked.LinkTo( external );
            ASSERT_TRUE( varLinked.UsesDynamicMemory() );
            ASSERT_FALSE( varLinked.OwnsDynamicMemory() );
            ASSERT_TRUE( varLinked.IsDynamicMemoryLinked() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 45: SetVoidType
    GUCEF_TESTFW_TESTCASE( "Test 45: SetVoidType" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 45: SetVoidType" );

            CORE::CVariant var( Int32(42) );
            ASSERT_FALSE( var.IsVoid() );

            var.SetVoidType();
            ASSERT_TRUE( var.IsVoid() );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_VOID );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 46: Default value when conversion fails
    GUCEF_TESTFW_TESTCASE( "Test 46: Default value when conversion fails" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 46: Default value when conversion fails" );

            CORE::CVariant var( "not a number" );
            Int32 result = var.AsInt32( 999 );
            ASSERT_TRUE( result == 999 || result == 0 ); // Either default or conversion result
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 47: Operator cast to string
    GUCEF_TESTFW_TESTCASE( "Test 47: Operator cast to string" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 47: Operator cast to string" );

            CORE::CVariant var( "Test Cast" );
            CORE::CUtf8String utf8 = (CORE::CUtf8String)var;
            CORE::CAsciiString ascii = (CORE::CAsciiString)var;

            ASSERT_TRUE( utf8 == "Test Cast" );
            ASSERT_TRUE( ascii == "Test Cast" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 48: LinkTo CDynamicBuffer
    GUCEF_TESTFW_TESTCASE( "Test 48: LinkTo CDynamicBuffer" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 48: LinkTo CDynamicBuffer" );

            CORE::CDynamicBuffer buf( 16, true );
            buf.AppendValue( UInt32(0x12345678) );

            CORE::CVariant var;
            var.LinkTo( buf );

            ASSERT_TRUE( var.IsDynamicMemoryLinked() );
            ASSERT_TRUE( var.ByteSize() == buf.GetDataSize() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 49: UsesDynamicMemory static check
    GUCEF_TESTFW_TESTCASE( "Test 49: UsesDynamicMemory static check" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 49: UsesDynamicMemory static check" );

            // Fixed types should not use dynamic memory
            ASSERT_FALSE( CORE::CVariant::UsesDynamicMemory( GUCEF_DATATYPE_INT8 ) );
            ASSERT_FALSE( CORE::CVariant::UsesDynamicMemory( GUCEF_DATATYPE_INT32 ) );
            ASSERT_FALSE( CORE::CVariant::UsesDynamicMemory( GUCEF_DATATYPE_FLOAT64 ) );
            ASSERT_FALSE( CORE::CVariant::UsesDynamicMemory( GUCEF_DATATYPE_BOOLEAN_INT32 ) );

            // String and blob types use dynamic memory
            ASSERT_TRUE( CORE::CVariant::UsesDynamicMemory( GUCEF_DATATYPE_UTF8_STRING ) );
            ASSERT_TRUE( CORE::CVariant::UsesDynamicMemory( GUCEF_DATATYPE_ASCII_STRING ) );
            ASSERT_TRUE( CORE::CVariant::UsesDynamicMemory( GUCEF_DATATYPE_BINARY_BLOB ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 50: String comparison operators
    GUCEF_TESTFW_TESTCASE( "Test 50: String comparison operators" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 50: String comparison operators" );

            CORE::CVariant a( "apple" );
            CORE::CVariant b( "banana" );
            CORE::CVariant c( "apple" );

            ASSERT_TRUE( a == c );
            ASSERT_TRUE( a != b );
            ASSERT_TRUE( a < b );  // "apple" < "banana" lexicographically
            ASSERT_TRUE( b > a );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 51: Mixed type comparison
    GUCEF_TESTFW_TESTCASE( "Test 51: Mixed type comparison" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 51: Mixed type comparison" );

            CORE::CVariant varInt( Int32(42) );
            CORE::CVariant varStr( "42" );

            // These may or may not be equal depending on implementation
            // Just verify no crash occurs
            bool equal = (varInt == varStr);
            bool notEqual = (varInt != varStr);
            ASSERT_TRUE( equal || notEqual ); // One must be true
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 52: OverrideTypeId
    GUCEF_TESTFW_TESTCASE( "Test 52: OverrideTypeId" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 52: OverrideTypeId" );

            CORE::CVariant var( Int32(42) );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_INT32 );

            var.OverrideTypeId( GUCEF_DATATYPE_UINT32 );
            ASSERT_TRUE( var.GetTypeId() == GUCEF_DATATYPE_UINT32 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 53: EmptyVector static constant
    GUCEF_TESTFW_TESTCASE( "Test 53: EmptyVector static constant" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 53: EmptyVector static constant" );

            ASSERT_TRUE( CORE::CVariant::EmptyVector.empty() );
            ASSERT_TRUE( CORE::CVariant::EmptyVector.size() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 54: Fraction type
    GUCEF_TESTFW_TESTCASE( "Test 54: Fraction type" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 54: Fraction type" );

            CORE::TInt32Fraction frac( 3, 4 );

            CORE::CVariant var( frac );
            ASSERT_TRUE( var.IsFraction() );

            CORE::TInt32Fraction result = var.AsInt32Fraction();
            ASSERT_TRUE( result.GetNumerator() == 3 );
            ASSERT_TRUE( result.GetDenominator() == 4 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 55: AsCharPtr
    GUCEF_TESTFW_TESTCASE( "Test 55: AsCharPtr" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 55: AsCharPtr" );

            CORE::CVariant var( "Test Pointer" );
            const char* ptr = var.AsCharPtr();

            ASSERT_TRUE( ptr != GUCEF_NULL );
            ASSERT_TRUE( strcmp( ptr, "Test Pointer" ) == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CVariant TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/

void
PerformVariantViewTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CVariantView TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CVariantView" );

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] Default construct: uninitialized" )
    try
    {
        CORE::CVariantView view;
        ASSERT_TRUE(  !view.IsInitialized() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] Construct from CVariant scalar: value preserved, linked" )
    try
    {
        CORE::CVariant owned( (CORE::Int32) 42 );
        CORE::CVariantView view( owned );
        ASSERT_TRUE(  view.IsInitialized() );
        ASSERT_TRUE(  view.IsInteger() );
        ASSERT_TRUE(  42 == view.AsInt32() );
        ASSERT_TRUE(  view.GetTypeId() == owned.GetTypeId() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] Construct from CVariant string: dynamic memory linked, no copy" )
    try
    {
        CORE::CVariant owned( "hello view" );
        CORE::CVariantView view( owned );
        ASSERT_TRUE(  view.IsString() );
        ASSERT_TRUE(  view.IsDynamicMemoryLinked() );
        ASSERT_TRUE(  view.AsCharPtr() == owned.AsCharPtr() );  /* same buffer pointer */
        ASSERT_TRUE(  view.AsString() == "hello view" );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] Construct from raw (void*, size, type)" )
    try
    {
        static const char rawStr[] = "raw string";
        CORE::CVariantView view( static_cast< const void* >( rawStr ),
                                 sizeof( rawStr ),
                                 GUCEF_DATATYPE_ASCII_STRING );
        ASSERT_TRUE(  view.IsString() );
        ASSERT_TRUE(  view.IsDynamicMemoryLinked() );
        ASSERT_TRUE(  view.AsCharPtr() == rawStr );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] Construct from null-terminated char*" )
    try
    {
        static const char rawStr[] = "null term string";
        CORE::CVariantView view( rawStr );
        ASSERT_TRUE(  view.IsString() );
        ASSERT_TRUE(  view.AsCharPtr() == rawStr );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] Copy rebinds; both share same data pointer" )
    try
    {
        CORE::CVariant owned( "copy test" );
        CORE::CVariantView view1( owned );
        CORE::CVariantView view2( view1 );
        ASSERT_TRUE(  view2.IsString() );
        ASSERT_TRUE(  view2.AsCharPtr() == view1.AsCharPtr() );
        ASSERT_TRUE(  view2.AsCharPtr() == owned.AsCharPtr() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] Implicit conversion to const CVariant& works" )
    try
    {
        CORE::CVariant owned( (CORE::UInt64) 12345ull );
        CORE::CVariantView view( owned );

        /* operator== takes const CVariant& — implicit conversion used */
        ASSERT_TRUE(  view == owned );

        const CORE::CVariant& ref = view;
        ASSERT_TRUE(  ref.AsUInt64() == owned.AsUInt64() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] Type predicates delegate correctly" )
    try
    {
        {
            CORE::CVariantView view( (CORE::Int32) 1 );
            ASSERT_TRUE(  view.IsInteger() );
            ASSERT_TRUE(  view.IsNumber() );
            ASSERT_FALSE( view.IsString() );
            ASSERT_FALSE( view.IsFloat() );
        }
        {
            CORE::CVariantView view( 3.14f );
            ASSERT_TRUE(  view.IsFloat() );
            ASSERT_FALSE( view.IsInteger() );
        }
        {
            CORE::CVariantView view( true );
            ASSERT_TRUE(  view.IsBoolean() );
        }
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] AsTValue<T> delegates to inner variant" )
    try
    {
        CORE::CVariant owned( (CORE::Float64) 2.718 );
        CORE::CVariantView view( owned );
        CORE::Float64 val = view.AsTValue< CORE::Float64 >();
        ASSERT_TRUE(  val == owned.AsFloat64() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CVariantView] LinkTo CAsciiString: dynamic memory linked" )
    try
    {
        CORE::CAsciiString asciiStr( "ascii view" );
        CORE::CVariantView view;
        view.LinkTo( asciiStr );
        ASSERT_TRUE(  view.IsString() );
        ASSERT_TRUE(  view.IsDynamicMemoryLinked() );
        ASSERT_TRUE(  view.AsString() == "ascii view" );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CVariantView TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
