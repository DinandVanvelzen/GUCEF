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

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#include "TestUri.h"

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
PerformUriTests( void )
{
    std::cout << "\n\n**** COMMENCING CUri TESTS ****\n";
    
    try
    {
        CORE::CUri emptyUri;
        ASSERT_TRUE( emptyUri.GetScheme().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetAuthority().IsNULLOrEmpty() );
        ASSERT_TRUE( !emptyUri.HasAuthority() );
        ASSERT_TRUE( emptyUri.GetAuthorityUsername().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetAuthorityPassword().IsNULLOrEmpty() );        
        ASSERT_TRUE( emptyUri.GetAuthorityHost().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetAuthorityPort().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetPath().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetQuery().IsNULLOrEmpty() );
        ASSERT_TRUE( emptyUri.GetFragment().IsNULLOrEmpty() );

        CORE::CUri uri( "scheme://user:pass@host:port/path/path/path?query#fragment" );
        ASSERT_TRUE( uri.GetScheme() == "scheme" );
        ASSERT_TRUE( uri.GetAuthority() == "user:pass@host:port" );
        ASSERT_TRUE( uri.HasAuthority() );
        ASSERT_TRUE( uri.GetAuthorityUsername() == "user" );
        ASSERT_TRUE( uri.GetAuthorityPassword() == "pass" );        
        ASSERT_TRUE( uri.GetAuthorityHost() == "host" );
        ASSERT_TRUE( uri.GetAuthorityPort() == "port" );
        ASSERT_TRUE( uri.GetPath() == "path/path/path" );
        ASSERT_TRUE( uri.GetQuery() == "query" );
        ASSERT_TRUE( uri.GetFragment() == "fragment" );
        ASSERT_TRUE( "scheme://user:pass@host:port/path/path/path?query#fragment" == CORE::ToString( uri ) );

        CORE::CUri uri2( "scheme:path/path/path?query#fragment" );
        ASSERT_TRUE( uri2.GetScheme() == "scheme" );
        ASSERT_TRUE( uri2.GetAuthority().IsNULLOrEmpty() );
        ASSERT_TRUE( !uri2.HasAuthority() );
        ASSERT_TRUE( uri2.GetAuthorityUsername().IsNULLOrEmpty() );
        ASSERT_TRUE( uri2.GetAuthorityPassword().IsNULLOrEmpty() );        
        ASSERT_TRUE( uri2.GetAuthorityHost().IsNULLOrEmpty() );
        ASSERT_TRUE( uri2.GetAuthorityPort().IsNULLOrEmpty() );
        ASSERT_TRUE( uri2.GetPath() == "path/path/path" );
        ASSERT_TRUE( uri2.GetQuery() == "query" );
        ASSERT_TRUE( uri2.GetFragment() == "fragment" );

        CORE::CUri uri3( "scheme:path/path/path" );
        ASSERT_TRUE( uri3.GetScheme() == "scheme" );
        ASSERT_TRUE( uri3.GetAuthority().IsNULLOrEmpty() );
        ASSERT_TRUE( !uri3.HasAuthority() );
        ASSERT_TRUE( uri3.GetAuthorityUsername().IsNULLOrEmpty() );
        ASSERT_TRUE( uri3.GetAuthorityPassword().IsNULLOrEmpty() );        
        ASSERT_TRUE( uri3.GetAuthorityHost().IsNULLOrEmpty() );
        ASSERT_TRUE( uri3.GetAuthorityPort().IsNULLOrEmpty() );
        ASSERT_TRUE( uri3.GetPath() == "path/path/path" );
        ASSERT_TRUE( uri3.GetQuery().IsNULLOrEmpty() );
        ASSERT_TRUE( uri3.GetFragment().IsNULLOrEmpty() );

        CORE::CUri uri4( "scheme:path/path/path?query" );
        ASSERT_TRUE( uri4.GetScheme() == "scheme" );
        ASSERT_TRUE( uri4.GetAuthority().IsNULLOrEmpty() );
        ASSERT_TRUE( !uri4.HasAuthority() );
        ASSERT_TRUE( uri4.GetAuthorityUsername().IsNULLOrEmpty() );
        ASSERT_TRUE( uri4.GetAuthorityPassword().IsNULLOrEmpty() );        
        ASSERT_TRUE( uri4.GetAuthorityHost().IsNULLOrEmpty() );
        ASSERT_TRUE( uri4.GetAuthorityPort().IsNULLOrEmpty() );
        ASSERT_TRUE( uri4.GetPath() == "path/path/path" );
        ASSERT_TRUE( uri4.GetQuery() == "query" );
        ASSERT_TRUE( uri4.GetFragment().IsNULLOrEmpty() );

        ASSERT_TRUE( emptyUri != uri );
        ASSERT_TRUE( uri != uri2 );
        ASSERT_TRUE( uri2 != uri3 );
        ASSERT_TRUE( uri3 != uri4 );
    }
    catch( ... )
    {
        ERRORHERE;
    }

    std::cout << "\n\n**** FINISHED CUri TESTS ****\n";
}

/*-------------------------------------------------------------------------*/
