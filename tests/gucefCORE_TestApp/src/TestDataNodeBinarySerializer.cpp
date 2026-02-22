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

#ifndef GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#include "gucefCORE_CDataNodeBinarySerializer.h"
#define GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CDATANODEBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDataNodeBinarySerializer.h"

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
PerformDataNodeBinarySerializerTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CDataNodeBinarySerializer TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "CDataNodeBinarySerializer" );

    GUCEF_TESTFW_TESTCASE( "Test 1: Empty DOM serialization" )
        try
        {                              
            CORE::CDynamicBuffer buffer;
            CORE::CDataNode dom1, dom2;
            UInt32 bytesWritten = 0;
            UInt32 byteRead = 0;

            CORE::CDataNodeBinarySerializerOptions options;

            // test empty DOM
            dom1.Clear();
            buffer.Clear();
            bytesWritten = 0;
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Serialize( options, dom1, 0, buffer, bytesWritten ) );
            ASSERT_TRUE( bytesWritten == buffer.GetDataSize() );
            ASSERT_TRUE( bytesWritten > 0 );
            
            byteRead = 0;
            dom2.Clear();
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Deserialize( options, true, dom2, 0, buffer, byteRead ) );
            ASSERT_TRUE( byteRead == bytesWritten );
            ASSERT_TRUE( byteRead == buffer.GetDataSize() );
            ASSERT_TRUE( byteRead > 0 );
            ASSERT_TRUE( dom1 == dom2 );

            // Add rubbish at the end and see if that breaks it
            dom2.Clear();
            buffer.AppendValue( (UInt64) 1223345566 );
            byteRead = 0;
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Deserialize( options, true, dom2, 0, buffer, byteRead ) );
            ASSERT_TRUE( byteRead == bytesWritten );
            ASSERT_TRUE( byteRead + sizeof(UInt64) == buffer.GetDataSize() );
            ASSERT_TRUE( byteRead > 0 );
            ASSERT_TRUE( dom1 == dom2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 2: Simple DOM with attributes" )
        try
        {
            CORE::CDynamicBuffer buffer;
            CORE::CDataNode dom1, dom2;
            UInt32 bytesWritten = 0;
            UInt32 byteRead = 0;
            CORE::CDataNodeBinarySerializerOptions options;

            // Hardcode generate a simple test DOM for a more in-depth test       
            dom1.Clear();
            dom2.Clear();
            buffer.Clear();
            ASSERT_TRUE( dom1.SetAttribute( "foo", "bar" ) );
            ASSERT_TRUE( dom1.SetAttribute( "fi", "fo" ) );
            ASSERT_TRUE( dom1.SetAttribute( "fa", "fum" ) );
            dom1.SetName( "test-root" );
            dom1.SetValue( "test-value" );

            // Try to serialize our generated DOM
            bytesWritten = 0;
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Serialize( options, dom1, 0, buffer, bytesWritten ) );
            ASSERT_TRUE( bytesWritten == buffer.GetDataSize() );
            ASSERT_TRUE( bytesWritten > 0 );

            // Now read it back again
            byteRead = 0;
            dom2.Clear();
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Deserialize( options, true, dom2, 0, buffer, byteRead ) );
            ASSERT_TRUE( byteRead == bytesWritten );
            ASSERT_TRUE( byteRead == buffer.GetDataSize() );
            ASSERT_TRUE( byteRead > 0 );
            ASSERT_TRUE( dom1 == dom2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 3: DOM with array child" )
        try
        {
            CORE::CDynamicBuffer buffer;
            CORE::CDataNode dom1, dom2;
            UInt32 bytesWritten = 0;
            UInt32 byteRead = 0;
            CORE::CDataNodeBinarySerializerOptions options;

            // Hardcode generate a slightly more complex test DOM for a more in-depth test       
            dom1.Clear();
            dom2.Clear();
            buffer.Clear();
            ASSERT_TRUE( dom1.SetAttribute( "foo", "bar" ) );
            ASSERT_TRUE( dom1.SetAttribute( "fi", "fo" ) );
            ASSERT_TRUE( dom1.SetAttribute( "fa", "fum" ) );
            dom1.SetName( "test-root" );
            dom1.SetValue( "test-value" );
            CORE::CDataNode* numbersArray = dom1.AddChild( "numbers", GUCEF_DATATYPE_ARRAY );
            ASSERT_TRUE( GUCEF_NULL != numbersArray );
            for ( UInt32 i=0; i<10; ++i ) 
            { 
                ASSERT_TRUE( GUCEF_NULL != numbersArray->AddValueAsChild( 1 ) ); 
            }

            // Try to serialize our generated DOM
            bytesWritten = 0;
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Serialize( options, dom1, 0, buffer, bytesWritten ) );
            ASSERT_TRUE( bytesWritten == buffer.GetDataSize() );
            ASSERT_TRUE( bytesWritten > 0 );

            // Now read it back again
            byteRead = 0;
            dom2.Clear();
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Deserialize( options, true, dom2, 0, buffer, byteRead ) );
            ASSERT_TRUE( byteRead == bytesWritten );
            ASSERT_TRUE( byteRead == buffer.GetDataSize() );
            ASSERT_TRUE( byteRead > 0 );
            ASSERT_TRUE( dom1 == dom2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 4: Complex deep tree DOM" )
        try
        {
            CORE::CDynamicBuffer buffer;
            CORE::CDataNode dom1, dom2;
            UInt32 bytesWritten = 0;
            UInt32 byteRead = 0;
            CORE::CDataNodeBinarySerializerOptions options;

            // Hardcode generate a more complex test DOM for a more in-depth test   
            dom1.Clear();
            dom2.Clear();
            buffer.Clear();
            ASSERT_TRUE( dom1.SetAttribute( "foo", "bar" ) );
            ASSERT_TRUE( dom1.SetAttribute( "fi", "fo" ) );
            ASSERT_TRUE( dom1.SetAttribute( "fa", "fum" ) );
            dom1.SetName( "test-root" );
            dom1.SetValue( "test-value" );
            CORE::CDataNode* numbersArray = dom1.AddChild( "numbers", GUCEF_DATATYPE_ARRAY );
            ASSERT_TRUE( GUCEF_NULL != numbersArray );
            for ( UInt32 i=0; i<10; ++i ) 
            { 
                ASSERT_TRUE( GUCEF_NULL != numbersArray->AddValueAsChild( 1 ) ); 
            }
            CORE::CDataNode* deepTree = dom1.AddChild( "childDeepTree", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* node = deepTree;
            ASSERT_TRUE( GUCEF_NULL != deepTree );
            for ( UInt32 i=0; i<100; ++i ) 
            { 
                for ( UInt32 n=0; n<100; ++n )
                {
                    CORE::CDataNode* siblingNode = node->AddChild( "childDeepTree-SiblingNode-L" + CORE::ToString( i ) + "-Nr" + CORE::ToString( n ) );
                    ASSERT_TRUE( GUCEF_NULL != siblingNode );
                    siblingNode->SetValue( n );
                }
                
                node = node->AddChild( "childDeepTree-L" + CORE::ToString( i ) );
                ASSERT_TRUE( GUCEF_NULL != node ); 
            }

            // Try to serialize our generated DOM
            bytesWritten = 0;
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Serialize( options, dom1, 0, buffer, bytesWritten ) );
            ASSERT_TRUE( bytesWritten == buffer.GetDataSize() );
            ASSERT_TRUE( bytesWritten > 0 );

            // Now read it back again
            byteRead = 0;
            dom2.Clear();
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Deserialize( options, true, dom2, 0, buffer, byteRead ) );
            ASSERT_TRUE( byteRead == bytesWritten );
            ASSERT_TRUE( byteRead == buffer.GetDataSize() );
            ASSERT_TRUE( byteRead > 0 );
            ASSERT_TRUE( dom1 == dom2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CDataNodeBinarySerializer TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
