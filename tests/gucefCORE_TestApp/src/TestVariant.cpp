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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVariant.h"

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

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CVariant TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
