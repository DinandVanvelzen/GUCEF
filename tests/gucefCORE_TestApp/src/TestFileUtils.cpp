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

#include <iostream>

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "TestFileUtils.h"

using namespace GUCEF;

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
TestRelativePathFunction( void )
{
    try
    {
        // test that the $CURWORKDIR$ macro gets converted
        CORE::CString relPath = CORE::RelativePath( "$CURWORKDIR$/test.txt", true );
        ASSERT_TRUE( !relPath.IsNULLOrEmpty() );
        ASSERT_TRUE( !relPath.StartsWith( "$CURWORKDIR$" ) );
        ASSERT_TRUE( relPath.EndsWith( "test.txt" ) );
        CORE::CString::StringVector elements = relPath.ParseElements( GUCEF_DIRSEPCHAR );
        ASSERT_TRUE( elements.size() > 1 );

        // test that the $MODULEDIR$ macro gets converted
        relPath = CORE::RelativePath( "$MODULEDIR$/test.txt", true );
        ASSERT_TRUE( !relPath.IsNULLOrEmpty() );
        ASSERT_TRUE( !relPath.StartsWith( "$MODULEDIR$" ) );
        ASSERT_TRUE( relPath.EndsWith( "test.txt" ) );
        elements = relPath.ParseElements( GUCEF_DIRSEPCHAR );
        ASSERT_TRUE( elements.size() > 1 );

        // test that the function converts paths to use the O/S native dir separator
        relPath = CORE::RelativePath( "root/dir1/dir2/dir3/test.txt", true );
        ASSERT_TRUE( !relPath.IsNULLOrEmpty() );
        ASSERT_TRUE( relPath == CORE::CString( "root/dir1/dir2/dir3/test.txt" ).ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
        elements = relPath.ParseElements( GUCEF_DIRSEPCHAR );
        ASSERT_TRUE( elements.size() == 5 );

        // test that the function simplifies paths to reduce 'up dir' segments aka ../ if feasible
        relPath = CORE::RelativePath( "root/dir1/../dir3/../test.txt", true );
        ASSERT_TRUE( !relPath.IsNULLOrEmpty() );
        ASSERT_TRUE( relPath == CORE::CString( "root/test.txt" ).ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
        elements = relPath.ParseElements( GUCEF_DIRSEPCHAR );
        ASSERT_TRUE( elements.size() == 2 );
    }
    catch (...)
    {
        ERRORHERE;
    }
}
/*-------------------------------------------------------------------------*/

void
PerformFileUtilsTests( void )
{
    std::cout << "\n\n**** COMMENCING File Utils TESTS ****\n";
    
    try
    {
        TestRelativePathFunction();
        
    }
    catch( ... )
    {
        ERRORHERE;
    }

    std::cout << "\n\n**** FINISHED File Utils TESTS ****\n";
}

/*-------------------------------------------------------------------------*/
