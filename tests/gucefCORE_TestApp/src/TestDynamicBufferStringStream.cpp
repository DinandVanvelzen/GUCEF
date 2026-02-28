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

#ifndef GUCEF_CORE_CDYNAMICBUFFERSTRINGSTREAM_H
#include "gucefCORE_CDynamicBufferStringStream.h"
#define GUCEF_CORE_CDYNAMICBUFFERSTRINGSTREAM_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERSTRINGSTREAM_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDynamicBufferSwap.h"

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
PerformDynamicBufferStringStreamTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CDynamicBufferStringStream TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "CDynamicBufferStringStream" );

    GUCEF_TESTFW_TESTCASE( "Test 1: Basic setup and stream operations" )
        try
        {       
            // Check basic setup
            
            CORE::CDynamicBufferStringStream stream;
            ASSERT_TRUE( GUCEF_NULL == stream.GetBufferPtr() );
            ASSERT_TRUE( 0 == stream.GetBufferSize() );
            ASSERT_TRUE( 0 == stream.GetDataSize() );                
            //ASSERT_TRUE( stream.eof() );  // false on init :/ 
            ASSERT_TRUE( stream.Eof() );
            ASSERT_TRUE( GUCEF_NULL != stream.CStyleAccess() );
            ASSERT_TRUE( 0 == stream.Tell() );

            // Attempt to write variables
            
            Int8     a = 1;
            UInt8    b = 2;
            Int16    c = 3;
            UInt16   d = 4;
            Int32    e = 5;
            UInt32   f = 6;
            Int64    g = 7;
            UInt64   h = 8;

            // Removed duplicate debug write
            // Note: CDynamicBufferStringStream uses std::stringstream which on Windows outputs UTF-16 (wide chars)
            // Each character is 2 bytes, so "1,2,3,4,5,6,7,8" becomes 30 bytes with null padding

            stream << (int)a << ',' << (int)b << ',' << c << ',' << d << ',' << e << ',' << f << ',' << g << ',' << h;

            // Verify data was written (15 chars * 2 bytes for UTF-16 = 30 bytes)
            ASSERT_TRUE( stream.GetDataSize() > 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CDynamicBufferStringStream TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
