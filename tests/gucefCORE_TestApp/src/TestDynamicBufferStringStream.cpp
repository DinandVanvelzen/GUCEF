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

#include "TestDynamicBufferSwap.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX || GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID
  #define DEBUGBREAK __builtin_trap()
#elif GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
  #define DEBUGBREAK DebugBreak()
#else
  #define DEBUGBREAK
#endif

#define ERRORHERE { std::cout << "Test failed @ " << __FILE__ << "(" << __LINE__ << ")\n"; DEBUGBREAK; }
#define ASSERT_TRUE( test ) if ( !(test) ) { ERRORHERE; } 
#define ASSERT_FALSE( test ) if ( (test) ) { ERRORHERE; }

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

    std::cout << "\n\n**** COMMENCING CDynamicBufferStringStream TESTS ****\n";
    
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
        
        Int8     a = '1';
        UInt8    b = '2';
        Int16    c = 3;
        UInt16   d = 4;
        Int32    e = 5;
        UInt32   f = 6;
        Int64    g = 7;
        UInt64   h = 8;

stream << c ;//<< ',' ;//<< d << ',' << e << ',' << f << ',' << g << ',' << h;

        stream << a << ',' << b << ',' << c << ',' << d << ',' << e << ',' << f << ',' << g << ',' << h;

        ASSERT_TRUE( "1,2,3,4,5,6,7,8" == stream );

        CORE::CUtf8String testString1 = "foobar";

        //stream << testString1;

        ASSERT_TRUE( 2 == stream.GetDataSize() );

        //stream << d;
        //stream << e;
        //stream << f;
    }
    catch( ... )
    {
        ERRORHERE;
    }

    std::cout << "\n\n**** FINISHED CDynamicBufferStringStream TESTS ****\n";
}

/*-------------------------------------------------------------------------*/
