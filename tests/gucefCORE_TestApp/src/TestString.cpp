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

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include "TestString.h"

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

template< typename StringType >
void
PerformTypedStringTests( void )
{
    std::cout << "\n\n**** COMMENCING CString TESTS ****\n";
    
    try
    {
        StringType testStr1;
        StringType testStr2;
        StringType testStr3;
        static const char* alphabetLc = "abcdefghijklmnopqrstuvwxyz";
        static const char* alphabetUc = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        static const char* digits = "1234567890";
        static const char* digitsRepeat3 = "1234567890\n1234567890\n1234567890\n";

        // test const char* assignment and the == operator   
        testStr1 = alphabetLc;
        ASSERT_TRUE( testStr1 == alphabetLc );
        ASSERT_TRUE( testStr1.Equals( alphabetLc ) );
        ASSERT_TRUE( testStr1.IsFormattingValid() );
        
        // test appending
        testStr1 = alphabetLc;
        testStr2 = alphabetLc;
        testStr1 += digits;
        testStr2.Append( digits );
        ASSERT_TRUE( testStr1 == testStr2 );
         
        // test index operator
        testStr1 = alphabetLc;
        for ( CORE::UInt32 i=0; i<26; ++i )
        {
            ASSERT_TRUE( testStr1[ i ] == alphabetLc[ i ] );
        }
        ASSERT_TRUE( 0 == testStr1[ 1000 ] );

        // test uppercase conversion
        ASSERT_TRUE( testStr1.Uppercase() == alphabetUc );

        // test obj to obj assignment
        testStr3 = testStr1;
        ASSERT_TRUE( testStr3 == testStr1 );
        testStr3 = testStr3.Uppercase();
        ASSERT_TRUE( testStr3 != testStr1 );

        // Test case insensitive compare
        ASSERT_TRUE( testStr3.Equals( testStr1, false ) );
        ASSERT_TRUE( !testStr3.NotEquals( testStr1, false ) );

        // test HasSubstr
        testStr1 = alphabetLc;
        CORE::Int32 testIndex = testStr1.HasSubstr( "fghi", true );
        ASSERT_TRUE( 5 == testIndex );
        testIndex = testStr1.HasSubstr( "fghi", false );
        ASSERT_TRUE( 5 == testIndex );
                
        // test SubstrToSubstr
        testStr1 = alphabetLc;
        testStr2 = testStr1.SubstrToSubstr( "fghi", true );
        ASSERT_TRUE( "abcde" == testStr2 );
        testStr2 = testStr1.SubstrToSubstr( "fghi", false );
        ASSERT_TRUE( "jklmnopqrstuvwxyz" == testStr2 );
        testStr2 = testStr1.SubstrToSubstr( "ghi", 3, true );
        ASSERT_TRUE( "def" == testStr2 );
        testStr2 = testStr1.SubstrToSubstr( "rst", 22, false );
        ASSERT_TRUE( "uvw" == testStr2 );

        testStr3.Set( digits );
        ASSERT_TRUE( digits == testStr3 );
        ASSERT_TRUE( 10 == testStr3.Length() );
        ASSERT_TRUE( 1 == testStr3.GetCharacterCount( '3' ) );
        ASSERT_TRUE( 0 == testStr3.GetCharacterRepeatCount( '6' ) );
        ASSERT_TRUE( testStr3 == testStr3.Lowercase() );
        ASSERT_TRUE( testStr3 == testStr3.Uppercase() );
        ASSERT_TRUE( 10 == testStr3.DetermineLength() );
        ASSERT_TRUE( !testStr3.IsNULLOrEmpty() );
        ASSERT_TRUE( 2 == testStr3.HasChar( '3' ) );
        ASSERT_TRUE( 5 == testStr3.HasChar( '6' ) );

        testStr3 = testStr3.ReplaceChar( '3', '6' );
        ASSERT_TRUE( digits != testStr3 );
        ASSERT_TRUE( 10 == testStr3.Length() );
        ASSERT_TRUE( 0 == testStr3.GetCharacterCount( '3' ) );
        ASSERT_TRUE( 0 == testStr3.GetCharacterRepeatCount( '6' ) );
        ASSERT_TRUE( 2 == testStr3.GetCharacterCount( '6' ) );
        ASSERT_TRUE( testStr3 == testStr3.Lowercase() );
        ASSERT_TRUE( testStr3 == testStr3.Uppercase() );
        ASSERT_TRUE( 10 == testStr3.DetermineLength() );
        ASSERT_TRUE( !testStr3.IsNULLOrEmpty() );
        ASSERT_TRUE( -1 == testStr3.HasChar( '3' ) );

        testStr3 = testStr3.ReplaceChar( '4', '6' );
        ASSERT_TRUE( 0 == testStr3.GetCharacterCount( '3' ) );
        ASSERT_TRUE( 0 == testStr3.GetCharacterCount( '4' ) );
        ASSERT_TRUE( 1 == testStr3.GetCharacterRepeatCount( '6' ) );
        ASSERT_TRUE( 3 == testStr3.GetCharacterCount( '6' ) );
        ASSERT_TRUE( 2 == testStr3.HasChar( '6', true ) );
        ASSERT_TRUE( 5 == testStr3.HasChar( '6', false ) );
        ASSERT_TRUE( 10 == testStr3.Length() );
                        
        testStr3 = testStr3.CompactRepeatingChar( '6' );
        ASSERT_TRUE( 9 == testStr3.Length() );
        ASSERT_TRUE( 0 == testStr3.GetCharacterRepeatCount( '6' ) );
        ASSERT_TRUE( 2 == testStr3.GetCharacterCount( '6' ) );
        ASSERT_TRUE( 9 == testStr3.DetermineLength() );
        
        testStr3 = digitsRepeat3;
        StringType::StringVector vlist = testStr3.ParseElements( '\n', true );
        ASSERT_TRUE( 4 == vlist.size() );
        vlist = testStr3.ParseElements( '\n', false );
        ASSERT_TRUE( 3 == vlist.size() );

        testStr3.Clear();
        testStr1 = testStr3.Combine( vlist, '\n' );
        testStr2 = digitsRepeat3;
        ASSERT_TRUE( 33 == testStr2.Length() );
        testStr2 = testStr2.CutChars( 1, false, 0 );
        ASSERT_TRUE( 32 == testStr2.Length() );
        vlist = testStr2.ParseElements( '\n', true );
        ASSERT_TRUE( 3 == vlist.size() );
        ASSERT_TRUE( testStr1 == testStr2 );

        testStr3 = digitsRepeat3;
        StringType::StringSet slist = testStr3.ParseUniqueElements( '\n', true );
        ASSERT_TRUE( 2 == slist.size() );
        slist = testStr3.ParseUniqueElements( '\n', false );
        ASSERT_TRUE( 1 == slist.size() );
        
        // test FindMaxSubstrEquality
        testStr2 = "jklmnopqrstuvwxyz";
        testStr1.FindMaxSubstrEquality( testStr2, 0, true, true );

        // test ReplaceEnvelopingSubstr
        testStr1 = "$MYVAR:VALUE$";
        testStr2 = testStr1.ReplaceEnvelopingSubstr( "$MYVAR:", "$", "{{", "}}" );
        ASSERT_TRUE( testStr2 == "{{VALUE}}" );
        testStr1 = "blablabla$MYVAR:VALUE$foobar";
        testStr2 = testStr1.ReplaceEnvelopingSubstr( "$MYVAR:", "$", "{{", "}}" );
        ASSERT_TRUE( testStr2 == "blablabla{{VALUE}}foobar" );
        testStr2 = testStr1.ReplaceEnvelopingSubstr( "$NOT_HERE:", "$", "{{", "}}" );
        ASSERT_TRUE( testStr2 == testStr1 );
    }
    catch( ... )
    {
        ERRORHERE;
    }

    std::cout << "\n\n**** FINISHED CString TESTS ****\n";
}

/*-------------------------------------------------------------------------*/

void
PerformStringTests( void )
{
    PerformTypedStringTests< CORE::CAsciiString >();
    PerformTypedStringTests< CORE::CUtf8String >();
}

/*-------------------------------------------------------------------------*/
