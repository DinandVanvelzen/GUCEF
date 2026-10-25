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

#include <stdio.h>   /* snprintf */

#include <typeinfo>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CUTF16STRING_H
#include "gucefCORE_CUtf16String.h"
#define GUCEF_CORE_CUTF16STRING_H
#endif /* GUCEF_CORE_CUTF16STRING_H ? */

#ifndef GUCEF_CORE_CUTF32STRING_H
#include "gucefCORE_CUtf32String.h"
#define GUCEF_CORE_CUTF32STRING_H
#endif /* GUCEF_CORE_CUTF32STRING_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestString.h"

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

/* Returns a short human-readable name for the string type T.
   Explicit specialisations are defined below after the template function. */
template< typename T >
const char* StringTypeName();

template<> const char* StringTypeName< CORE::CAsciiString >() { return "CAsciiString"; }
template<> const char* StringTypeName< CORE::CUtf8String  >() { return "CUtf8String";  }
template<> const char* StringTypeName< CORE::CUtf16String >() { return "CUtf16String"; }
template<> const char* StringTypeName< CORE::CUtf32String >() { return "CUtf32String"; }

/*-------------------------------------------------------------------------*/

template< typename StringType >
void
PerformTypedStringTests( void )
{
    static const char* alphabetLc    = "abcdefghijklmnopqrstuvwxyz";
    static const char* alphabetUc    = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char* digits        = "1234567890";
    static const char* digitsRepeat3 = "1234567890\n1234567890\n1234567890\n";

    const char* typeName = StringTypeName< StringType >();
    char tcName[ 128 ];

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Assignment and equality", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr1;
        testStr1 = alphabetLc;
        ASSERT_TRUE( testStr1 == alphabetLc );
        ASSERT_TRUE( testStr1.Equals( alphabetLc ) );
        ASSERT_TRUE( testStr1.IsFormattingValid() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] WildcardEquals matches the whole string against the pattern", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        // '*' stands for any characters, the rest of the pattern matches from the start to the end of the string
        ASSERT_TRUE( StringType( "a.txt" ).WildcardEquals( StringType( "*.txt" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "b.txt.bak" ).WildcardEquals( StringType( "*.txt" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "xa.txt" ).WildcardEquals( StringType( "a*" ), '*', true, false ) );
        ASSERT_TRUE( StringType( "abc" ).WildcardEquals( StringType( "a*" ), '*', true, false ) );
        ASSERT_TRUE( StringType( "abc" ).WildcardEquals( StringType( "a*c" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "abcd" ).WildcardEquals( StringType( "a*c" ), '*', true, false ) );
        ASSERT_TRUE( StringType( "ac" ).WildcardEquals( StringType( "a*c" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "a" ).WildcardEquals( StringType( "a*a" ), '*', true, false ) );     // the ends do not overlap
        ASSERT_TRUE( StringType( "x_neon.c" ).WildcardEquals( StringType( "*_neon.c" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "x_neon.cpp" ).WildcardEquals( StringType( "*_neon.c" ), '*', true, false ) );
        ASSERT_TRUE( StringType( "abXcdYef" ).WildcardEquals( StringType( "ab*cd*ef" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "abXefYcd" ).WildcardEquals( StringType( "ab*cd*ef" ), '*', true, false ) );
        ASSERT_TRUE( StringType( "anything" ).WildcardEquals( StringType( "*" ), '*', true, false ) );
        ASSERT_TRUE( StringType( "" ).WildcardEquals( StringType( "*" ), '*', true, false ) );
        ASSERT_TRUE( StringType( "a.b" ).WildcardEquals( StringType( "**.b" ), '*', true, false ) );

        // Without a '*' it is a plain comparison
        ASSERT_TRUE( StringType( "abc" ).WildcardEquals( StringType( "abc" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "abcd" ).WildcardEquals( StringType( "abc" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "abc" ).WildcardEquals( StringType( "b" ), '*', true, false ) );

        // A string that has a '*' itself is no pattern: it does not match everything
        ASSERT_FALSE( StringType( "*" ).WildcardEquals( StringType( "*.txt" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "x*" ).WildcardEquals( StringType( "*.txt" ), '*', true, false ) );

        // Case
        ASSERT_FALSE( StringType( "A.TXT" ).WildcardEquals( StringType( "*.txt" ), '*', true, false ) );
        ASSERT_TRUE( StringType( "A.TXT" ).WildcardEquals( StringType( "*.txt" ), '*', false, false ) );
        ASSERT_TRUE( StringType( "ABC" ).WildcardEquals( StringType( "abc" ), '*', false, false ) );

        // Both ways: either string may be the pattern
        ASSERT_TRUE( StringType( "*.txt" ).WildcardEquals( StringType( "a.txt" ), '*', true, true ) );
        ASSERT_TRUE( StringType( "a.txt" ).WildcardEquals( StringType( "*.txt" ), '*', true, true ) );
        ASSERT_FALSE( StringType( "*.txt" ).WildcardEquals( StringType( "a.txt" ), '*', true, false ) );
        ASSERT_FALSE( StringType( "a.dat" ).WildcardEquals( StringType( "*.txt" ), '*', true, true ) );

        // A set of patterns: any of them, so no patterns match nothing
        typename StringType::StringSet patterns;
        patterns.insert( StringType( "*.txt" ) );
        patterns.insert( StringType( "*.dat" ) );
        ASSERT_TRUE( StringType( "a.dat" ).WildcardEquals( patterns, '*', true, false ) );
        ASSERT_FALSE( StringType( "a.bin" ).WildcardEquals( patterns, '*', true, false ) );
        ASSERT_FALSE( StringType( "a.bin" ).WildcardEquals( typename StringType::StringSet(), '*', true, false ) );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Append", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr1( alphabetLc );
        StringType testStr2( alphabetLc );
        testStr1 += digits;
        testStr2.Append( digits );
        ASSERT_TRUE( testStr1 == testStr2 );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Index operator", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr1( alphabetLc );
        for ( CORE::UInt32 i = 0; i < 26; ++i )
        {
            ASSERT_TRUE( testStr1[ i ] == (CORE::UInt32)(unsigned char)alphabetLc[ i ] );
        }
        ASSERT_TRUE( 0 == testStr1[ 1000 ] );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Uppercase and Lowercase", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr1( alphabetLc );
        ASSERT_TRUE( testStr1.Uppercase() == alphabetUc );

        StringType testStr3 = testStr1;
        ASSERT_TRUE( testStr3 == testStr1 );
        testStr3 = testStr3.Uppercase();
        ASSERT_TRUE( testStr3 != testStr1 );
        ASSERT_TRUE( testStr3.Equals( testStr1, false ) );
        ASSERT_TRUE( !testStr3.NotEquals( testStr1, false ) );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] HasSubstr", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr1( alphabetLc );
        CORE::Int32 testIndex = testStr1.HasSubstr( "fghi", true );
        ASSERT_TRUE( 5 == testIndex );
        testIndex = testStr1.HasSubstr( "fghi", false );
        ASSERT_TRUE( 5 == testIndex );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] SubstrToSubstr", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr1( alphabetLc );
        StringType testStr2;

        testStr2 = testStr1.SubstrToSubstr( "fghi", true );
        ASSERT_TRUE( "abcde" == testStr2 );

        testStr2 = testStr1.SubstrToSubstr( "fghi", false );
        ASSERT_TRUE( "jklmnopqrstuvwxyz" == testStr2 );

        testStr2 = testStr1.SubstrToSubstr( "ghi", 3, true );
        ASSERT_TRUE( "def" == testStr2 );

        testStr2 = testStr1.SubstrToSubstr( "rst", 22, false );
        ASSERT_TRUE( "uvw" == testStr2 );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Set and basic queries", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr3;
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
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] ReplaceChar", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr3( digits );

        /* Replace '3' -> '6': string is "1264567890" */
        testStr3 = testStr3.ReplaceChar( '3', '6' );
        ASSERT_TRUE( digits != testStr3 );
        ASSERT_TRUE( 10 == testStr3.Length() );
        ASSERT_TRUE( 0 == testStr3.GetCharacterCount( '3' ) );
        ASSERT_TRUE( 2 == testStr3.GetCharacterCount( '6' ) );
        ASSERT_TRUE( 0 == testStr3.GetCharacterRepeatCount( '6' ) );
        ASSERT_TRUE( testStr3 == testStr3.Lowercase() );
        ASSERT_TRUE( testStr3 == testStr3.Uppercase() );
        ASSERT_TRUE( 10 == testStr3.DetermineLength() );
        ASSERT_TRUE( !testStr3.IsNULLOrEmpty() );
        ASSERT_TRUE( -1 == testStr3.HasChar( '3' ) );

        /* Replace '4' -> '6': string is "1266567890", two adjacent '6' */
        testStr3 = testStr3.ReplaceChar( '4', '6' );
        ASSERT_TRUE( 0 == testStr3.GetCharacterCount( '3' ) );
        ASSERT_TRUE( 0 == testStr3.GetCharacterCount( '4' ) );
        ASSERT_TRUE( 3 == testStr3.GetCharacterCount( '6' ) );
        ASSERT_TRUE( 1 == testStr3.GetCharacterRepeatCount( '6' ) );
        ASSERT_TRUE( 2 == testStr3.HasChar( '6', true ) );
        ASSERT_TRUE( 5 == testStr3.HasChar( '6', false ) );
        ASSERT_TRUE( 10 == testStr3.Length() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] CompactRepeatingChar", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        /* Build up the same state as after two ReplaceChar calls above */
        StringType testStr3( digits );
        testStr3 = testStr3.ReplaceChar( '3', '6' );
        testStr3 = testStr3.ReplaceChar( '4', '6' );

        /* Now compact: "1266567890" -> "126567890" (9 chars) */
        testStr3 = testStr3.CompactRepeatingChar( '6' );
        ASSERT_TRUE( 9 == testStr3.Length() );
        ASSERT_TRUE( 0 == testStr3.GetCharacterRepeatCount( '6' ) );
        ASSERT_TRUE( 2 == testStr3.GetCharacterCount( '6' ) );
        ASSERT_TRUE( 9 == testStr3.DetermineLength() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] ParseElements", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr3( digitsRepeat3 );
        typename StringType::StringVector vlist;

        vlist = testStr3.ParseElements( '\n', true );
        ASSERT_TRUE( 4 == vlist.size() );   /* 3 content + 1 empty after trailing \n */

        vlist = testStr3.ParseElements( '\n', false );
        ASSERT_TRUE( 3 == vlist.size() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Combine and CutChars (length check)", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr2( digitsRepeat3 );
        ASSERT_TRUE( 33 == testStr2.Length() );

        /* Remove the trailing '\n' */
        testStr2 = testStr2.CutChars( 1, false, 0 );
        ASSERT_TRUE( 32 == testStr2.Length() );

        /* Parse and re-combine */
        typename StringType::StringVector vlist = testStr2.ParseElements( '\n', true );
        ASSERT_TRUE( 3 == vlist.size() );

        StringType empty;
        StringType testStr1 = empty.Combine( vlist, '\n' );
        ASSERT_TRUE( testStr1 == testStr2 );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] ParseUniqueElements", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr3( digitsRepeat3 );

        typename StringType::StringSet slist;
        slist = testStr3.ParseUniqueElements( '\n', true );
        ASSERT_TRUE( 2 == slist.size() );   /* content row + empty string */

        slist = testStr3.ParseUniqueElements( '\n', false );
        ASSERT_TRUE( 1 == slist.size() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] FindMaxSubstrEquality", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr1( alphabetLc );
        StringType testStr2( "jklmnopqrstuvwxyz" );
        ASSERT_TRUE( testStr2 != testStr1 );
        CORE::UInt32 equality = testStr1.FindMaxSubstrEquality( testStr2, 0, true, true );
        ASSERT_TRUE( 0 == equality );

        // front to back: counts the matching leading code points
        StringType testStr3( "abcxyz" );
        ASSERT_TRUE( 3 == testStr3.FindMaxSubstrEquality( StringType( "abcq" ), 0, true, true ) );

        // back to front: counts the matching trailing code points
        ASSERT_TRUE( 3 == testStr3.FindMaxSubstrEquality( StringType( "qxyz" ), 0, false, true ) );
        ASSERT_TRUE( 0 == testStr3.FindMaxSubstrEquality( StringType( "xyq" ), 0, false, true ) );

        // back to front with an offset from the end of this string
        StringType testStr4( "abcxyzQ" );
        ASSERT_TRUE( 3 == testStr4.FindMaxSubstrEquality( StringType( "qxyz" ), 1, false, true ) );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] ReplaceEnvelopingSubstr", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr1( "$MYVAR:VALUE$" );
        StringType testStr2 = testStr1.ReplaceEnvelopingSubstr( "$MYVAR:", "$", "{{", "}}" );
        ASSERT_TRUE( testStr2 == "{{VALUE}}" );

        testStr1 = "blablabla$MYVAR:VALUE$foobar";
        testStr2 = testStr1.ReplaceEnvelopingSubstr( "$MYVAR:", "$", "{{", "}}" );
        ASSERT_TRUE( testStr2 == "blablabla{{VALUE}}foobar" );

        testStr2 = testStr1.ReplaceEnvelopingSubstr( "$NOT_HERE:", "$", "{{", "}}" );
        ASSERT_TRUE( testStr2 == testStr1 );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] CutChars and ByteSize contract", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType testStr1( "foobar" );
        StringType testStr2 = testStr1.CutChars( 3, true, 0 );
        ASSERT_TRUE( testStr2 == "bar" );
        ASSERT_TRUE( 3 == testStr2.Length() );
        ASSERT_TRUE( ( testStr2.Length() + 1u ) * (CORE::UInt32)sizeof( *(testStr2.C_String()) ) == testStr2.ByteSize() );
        ASSERT_TRUE( testStr2 != testStr1 );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformUtf8SpecificStringTests( void )
{
    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf8String] Reserve and Set" )
    try
    {
        const char* testStrContent1 = "0123456789";

        CORE::CUtf8String testStr;
        char* emptyBuffer = testStr.Reserve( 11 );
        ASSERT_TRUE( 11 == testStr.ByteSize() );
        ASSERT_TRUE( 0 == testStr.Length() );
        ASSERT_TRUE( "" == testStr );
        ASSERT_TRUE( CORE::CUtf8String::Empty == testStr );

        testStr.Set( testStrContent1, 10, false );
        ASSERT_TRUE( 11 == testStr.ByteSize() );
        ASSERT_TRUE( 10 == testStr.Length() );
        ASSERT_TRUE( testStr == testStrContent1 );
        ASSERT_TRUE( emptyBuffer == testStr.C_String() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformUtf32SpecificStringTests( void )
{
    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf32String] ByteSize contract and raw buffer" )
    try
    {
        CORE::CUtf32String testStr( "abc" );
        ASSERT_TRUE( 3 == testStr.Length() );
        ASSERT_TRUE( 16 == testStr.ByteSize() );    /* (3+1) * 4 = 16 */

        const CORE::UInt32* rawBuf = testStr.C_String();
        ASSERT_TRUE( GUCEF_NULL != rawBuf );
        ASSERT_TRUE( (CORE::UInt32)'a' == rawBuf[ 0 ] );
        ASSERT_TRUE( (CORE::UInt32)'b' == rawBuf[ 1 ] );
        ASSERT_TRUE( (CORE::UInt32)'c' == rawBuf[ 2 ] );
        ASSERT_TRUE( 0u == rawBuf[ 3 ] );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf32String] Multi-byte UTF-8 decode (euro sign)" )
    try
    {
        const char euroUtf8[] = { '\xE2', '\x82', '\xAC', '\0' };
        CORE::CUtf32String euroStr( euroUtf8 );
        ASSERT_TRUE( 1 == euroStr.Length() );
        ASSERT_TRUE( 0x20ACu == euroStr[ 0 ] );
        ASSERT_TRUE( 8 == euroStr.ByteSize() );     /* (1+1) * 4 = 8 */
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf32String] UTF-8 round-trip" )
    try
    {
        const char euroAndAsciiUtf8[] = { '\xE2', '\x82', '\xAC', '4', '2', '\0' };
        CORE::CUtf32String mixedEuro( euroAndAsciiUtf8 );
        ASSERT_TRUE( 3 == mixedEuro.Length() );
        ASSERT_TRUE( 0x20ACu == mixedEuro[ 0 ] );
        ASSERT_TRUE( (CORE::UInt32)'4' == mixedEuro[ 1 ] );
        ASSERT_TRUE( (CORE::UInt32)'2' == mixedEuro[ 2 ] );

        std::string utf8Roundtrip = mixedEuro.STL_String();
        CORE::CUtf32String backToUtf32( utf8Roundtrip );
        ASSERT_TRUE( backToUtf32 == mixedEuro );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf32String] Conversion from CAsciiString and CUtf8String" )
    try
    {
        CORE::CAsciiString asciiSrc( "hello" );
        CORE::CUtf32String fromAscii( asciiSrc );
        ASSERT_TRUE( 5 == fromAscii.Length() );
        ASSERT_TRUE( fromAscii == "hello" );
        ASSERT_TRUE( (CORE::UInt32)'h' == fromAscii[ 0 ] );

        CORE::CUtf8String utf8Src( "world" );
        CORE::CUtf32String fromUtf8( utf8Src );
        ASSERT_TRUE( 5 == fromUtf8.Length() );
        ASSERT_TRUE( fromUtf8 == "world" );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf32String] Reserve and DetermineLength" )
    try
    {
        CORE::CUtf32String reserveStr;
        CORE::UInt32* buf = reserveStr.Reserve( 6 );
        ASSERT_TRUE( GUCEF_NULL != buf );
        buf[ 0 ] = (CORE::UInt32)'X';
        buf[ 1 ] = (CORE::UInt32)'Y';
        buf[ 2 ] = (CORE::UInt32)'Z';
        buf[ 3 ] = 0u;
        CORE::UInt32 detectedLen = reserveStr.DetermineLength();
        ASSERT_TRUE( 3 == detectedLen );
        ASSERT_TRUE( 3 == reserveStr.Length() );
        ASSERT_TRUE( reserveStr == "XYZ" );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf32String] IsFormattingValid" )
    try
    {
        /* Surrogate halves are invalid scalar values in UTF-32 */
        CORE::CUtf32String surrogateStr;
        CORE::UInt32* surBuf = surrogateStr.Reserve( 2 );
        surBuf[ 0 ] = 0xD800u;
        surBuf[ 1 ] = 0u;
        surrogateStr.DetermineLength();
        ASSERT_TRUE( !surrogateStr.IsFormattingValid() );

        /* Code point above U+10FFFF is invalid */
        CORE::CUtf32String outOfRangeStr;
        CORE::UInt32* oorBuf = outOfRangeStr.Reserve( 2 );
        oorBuf[ 0 ] = 0x110000u;
        oorBuf[ 1 ] = 0u;
        outOfRangeStr.DetermineLength();
        ASSERT_TRUE( !outOfRangeStr.IsFormattingValid() );

        /* Valid BMP and supplementary code points */
        CORE::CUtf32String validStr;
        CORE::UInt32* valBuf = validStr.Reserve( 4 );
        valBuf[ 0 ] = 0x0041u;     /* U+0041 LATIN CAPITAL LETTER A    */
        valBuf[ 1 ] = 0x4E2Du;     /* U+4E2D CJK UNIFIED IDEOGRAPH     */
        valBuf[ 2 ] = 0x1F600u;    /* U+1F600 GRINNING FACE            */
        valBuf[ 3 ] = 0u;
        validStr.DetermineLength();
        ASSERT_TRUE( 3 == validStr.Length() );
        ASSERT_TRUE( validStr.IsFormattingValid() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformUtf16SpecificStringTests( void )
{
    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16String] ByteSize contract and raw buffer" )
    try
    {
        CORE::CUtf16String testStr( "abc" );
        ASSERT_TRUE( 3 == testStr.Length() );
        ASSERT_TRUE( 8 == testStr.ByteSize() );     /* (3+1) * 2 = 8 */
        ASSERT_TRUE( ( testStr.Length() + 1u ) * (CORE::UInt32)sizeof( *(testStr.C_String()) ) == testStr.ByteSize() );

        const CORE::UInt16* rawBuf = testStr.C_String();
        ASSERT_TRUE( GUCEF_NULL != rawBuf );
        ASSERT_TRUE( (CORE::UInt16)'a' == rawBuf[ 0 ] );
        ASSERT_TRUE( (CORE::UInt16)'b' == rawBuf[ 1 ] );
        ASSERT_TRUE( (CORE::UInt16)'c' == rawBuf[ 2 ] );
        ASSERT_TRUE( 0u == rawBuf[ 3 ] );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16String] BMP decode (euro sign)" )
    try
    {
        /* U+20AC EURO SIGN: 3-byte UTF-8, 1 UTF-16 code unit */
        const char euroUtf8[] = { '\xE2', '\x82', '\xAC', '\0' };
        CORE::CUtf16String euroStr( euroUtf8 );
        ASSERT_TRUE( 1 == euroStr.Length() );
        ASSERT_TRUE( 0x20ACu == euroStr[ 0 ] );
        ASSERT_TRUE( 4 == euroStr.ByteSize() );     /* (1+1) * 2 = 4 */
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16String] Supplementary code point (surrogate pair)" )
    try
    {
        /* U+1F600 GRINNING FACE: 4-byte UTF-8, 2 UTF-16 code units */
        const char smileyUtf8[] = { '\xF0', '\x9F', '\x98', '\x80', '\0' };
        CORE::CUtf16String smileyStr( smileyUtf8 );
        ASSERT_TRUE( 1 == smileyStr.Length() );
        ASSERT_TRUE( 0x1F600u == smileyStr[ 0 ] );
        ASSERT_TRUE( 6 == smileyStr.ByteSize() );   /* (2+1) * 2 = 6 */
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16String] UTF-8 round-trip" )
    try
    {
        const char euroAndAsciiUtf8[] = { '\xE2', '\x82', '\xAC', '4', '2', '\0' };
        CORE::CUtf16String mixedEuro( euroAndAsciiUtf8 );
        ASSERT_TRUE( 3 == mixedEuro.Length() );
        ASSERT_TRUE( 0x20ACu == mixedEuro[ 0 ] );
        ASSERT_TRUE( (CORE::UInt32)'4' == mixedEuro[ 1 ] );
        ASSERT_TRUE( (CORE::UInt32)'2' == mixedEuro[ 2 ] );

        std::string utf8Roundtrip = mixedEuro.STL_String();
        CORE::CUtf16String backToUtf16( utf8Roundtrip );
        ASSERT_TRUE( backToUtf16 == mixedEuro );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16String] Conversion from CAsciiString and CUtf8String" )
    try
    {
        CORE::CAsciiString asciiSrc( "hello" );
        CORE::CUtf16String fromAscii( asciiSrc );
        ASSERT_TRUE( 5 == fromAscii.Length() );
        ASSERT_TRUE( fromAscii == "hello" );
        ASSERT_TRUE( (CORE::UInt32)'h' == fromAscii[ 0 ] );

        CORE::CUtf8String utf8Src( "world" );
        CORE::CUtf16String fromUtf8( utf8Src );
        ASSERT_TRUE( 5 == fromUtf8.Length() );
        ASSERT_TRUE( fromUtf8 == "world" );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16String] Reserve and DetermineLength" )
    try
    {
        CORE::CUtf16String reserveStr;
        CORE::UInt16* buf = reserveStr.Reserve( 6 );
        ASSERT_TRUE( GUCEF_NULL != buf );
        buf[ 0 ] = (CORE::UInt16)'X';
        buf[ 1 ] = (CORE::UInt16)'Y';
        buf[ 2 ] = (CORE::UInt16)'Z';
        buf[ 3 ] = 0u;
        CORE::UInt32 detectedLen = reserveStr.DetermineLength();
        ASSERT_TRUE( 3 == detectedLen );
        ASSERT_TRUE( 3 == reserveStr.Length() );
        ASSERT_TRUE( reserveStr == "XYZ" );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16String] IsFormattingValid" )
    try
    {
        /* Lone high surrogate */
        CORE::CUtf16String badStr;
        CORE::UInt16* badBuf = badStr.Reserve( 2 );
        badBuf[ 0 ] = 0xD800u;
        badBuf[ 1 ] = 0u;
        badStr.DetermineLength();
        ASSERT_TRUE( !badStr.IsFormattingValid() );

        /* Lone low surrogate */
        CORE::CUtf16String badStr2;
        CORE::UInt16* badBuf2 = badStr2.Reserve( 2 );
        badBuf2[ 0 ] = 0xDC00u;
        badBuf2[ 1 ] = 0u;
        badStr2.DetermineLength();
        ASSERT_TRUE( !badStr2.IsFormattingValid() );

        /* Valid: BMP chars + a proper surrogate pair for U+1F600 */
        CORE::CUtf16String validStr;
        CORE::UInt16* valBuf = validStr.Reserve( 5 );
        valBuf[ 0 ] = 0x0041u;     /* U+0041 LATIN CAPITAL LETTER A */
        valBuf[ 1 ] = 0x4E2Du;     /* U+4E2D CJK UNIFIED IDEOGRAPH  */
        valBuf[ 2 ] = 0xD83Du;     /* high surrogate of U+1F600      */
        valBuf[ 3 ] = 0xDE00u;     /* low  surrogate of U+1F600      */
        valBuf[ 4 ] = 0u;
        validStr.DetermineLength();
        ASSERT_TRUE( 3 == validStr.Length() );
        ASSERT_TRUE( validStr.IsFormattingValid() );
        ASSERT_TRUE( 0x1F600u == validStr[ 2 ] );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16String] Static encode/decode helpers" )
    try
    {
        /* BMP code point: 1 code unit */
        CORE::UInt16 encBuf[ 2 ] = { 0u, 0u };
        ASSERT_TRUE( 1 == CORE::CUtf16String::EncodeUtf32CodePointToUtf16( 0x0041u, encBuf, 2 ) );
        ASSERT_TRUE( 0x0041u == encBuf[ 0 ] );

        /* Supplementary code point: 2 code units (surrogate pair) */
        ASSERT_TRUE( 2 == CORE::CUtf16String::EncodeUtf32CodePointToUtf16( 0x1F600u, encBuf, 2 ) );
        ASSERT_TRUE( 0xD83Du == encBuf[ 0 ] );
        ASSERT_TRUE( 0xDE00u == encBuf[ 1 ] );

        /* Decode the surrogate pair back to U+1F600 */
        CORE::UInt32 decodedCp = 0u;
        ASSERT_TRUE( 2 == CORE::CUtf16String::EncodeUtf16CodePointToUtf32( encBuf, 2, decodedCp ) );
        ASSERT_TRUE( 0x1F600u == decodedCp );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Tests the linked (non-owning) mode added to each string class.
 *  Works for all four string types because:
 *    - All have LinkTo( const SameType& )
 *    - All have IsLinked()
 *    - Copy ctor/assign never propagate linked state
 *    - Append / operator+= trigger PromoteToOwned
 */
template< typename StringType >
void
PerformTypedStringLinkedModeTests( void )
{
    const char* typeName = StringTypeName< StringType >();
    char tcName[ 128 ];

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] LinkTo(same type): IsLinked true, content matches", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType owned( "hello linked world" );
        ASSERT_FALSE( owned.IsLinked() );

        StringType linked;
        linked.LinkTo( owned );
        ASSERT_TRUE(  linked.IsLinked() );
        ASSERT_FALSE( owned.IsLinked() );
        ASSERT_TRUE(  linked == owned );
        /* Both should point to the same underlying raw buffer */
        /* Note: the const overload must be used: the non-const C_String() hands out a writable buffer and thus promotes a linked string to an owned copy */
        const StringType& constLinked = linked;
        const StringType& constOwned = owned;
        ASSERT_TRUE(  constLinked.C_String() == constOwned.C_String() );
        ASSERT_TRUE(  linked.IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Copy of linked string is independent (non-linked)", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType owned( "hello linked world" );
        StringType linked;
        linked.LinkTo( owned );
        ASSERT_TRUE( linked.IsLinked() );

        /* Copy constructor must deep-copy and NOT propagate linked state */
        StringType copied( linked );
        ASSERT_FALSE( copied.IsLinked() );
        ASSERT_TRUE(  copied == owned );
        /* Independent buffer */
        ASSERT_TRUE(  copied.C_String() != owned.C_String() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Assignment of linked string is independent (non-linked)", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType owned( "hello linked world" );
        StringType linked;
        linked.LinkTo( owned );
        ASSERT_TRUE( linked.IsLinked() );

        StringType assigned;
        assigned = linked;
        ASSERT_FALSE( assigned.IsLinked() );
        ASSERT_TRUE(  assigned == owned );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Mutation auto-promotes linked string; original unchanged", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType owned( "hello" );
        StringType linked;
        linked.LinkTo( owned );
        ASSERT_TRUE( linked.IsLinked() );

        /* Append triggers PromoteToOwned inside Append() */
        linked += StringType( "!" );
        ASSERT_FALSE( linked.IsLinked() );          /* promoted to owned  */
        ASSERT_TRUE(  owned == "hello" );            /* original unchanged */
        ASSERT_TRUE(  linked != owned );             /* content diverged   */
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Re-linking clears previous link", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType owned1( "first" );
        StringType owned2( "second" );
        StringType linked;
        linked.LinkTo( owned1 );
        ASSERT_TRUE( linked.IsLinked() );
        ASSERT_TRUE( linked == "first" );

        linked.LinkTo( owned2 );
        ASSERT_TRUE( linked.IsLinked() );
        ASSERT_TRUE( linked == "second" );
        ASSERT_TRUE( owned1 == "first" );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    snprintf( tcName, sizeof( tcName ), "[%s] Set on linked string promotes then overwrites", typeName );
    GUCEF_TESTFW_TESTCASE( tcName )
    try
    {
        StringType owned( "original" );
        StringType linked;
        linked.LinkTo( owned );
        ASSERT_TRUE( linked.IsLinked() );

        linked.Set( "overwritten" );
        ASSERT_FALSE( linked.IsLinked() );       /* no longer linked   */
        ASSERT_TRUE(  linked == "overwritten" ); /* new content        */
        ASSERT_TRUE(  owned  == "original" );    /* original unchanged */
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAsciiStringLinkedAndViewTests( void )
{
    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CAsciiString] LinkTo(const char*, UInt32): buffer borrowed" )
    try
    {
        static const char rawBuf[] = "raw buffer content";
        CORE::CAsciiString linked;
        linked.LinkTo( rawBuf, 18 );
        ASSERT_TRUE(  linked.IsLinked() );
        ASSERT_TRUE(  linked == "raw buffer content" );
        ASSERT_TRUE(  linked.Length() == 18 );
        /* Note: the const overload must be used: the non-const C_String() promotes a linked string to an owned copy */
        const CORE::CAsciiString& constLinked = linked;
        ASSERT_TRUE(  constLinked.C_String() == rawBuf );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CAsciiStringView] Default construct: empty, not crashing" )
    try
    {
        CORE::CAsciiStringView view;
        ASSERT_TRUE(  view.IsNULLOrEmpty() );
        ASSERT_TRUE(  0 == view.Length() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CAsciiStringView] Construct from (char*, length): no copy" )
    try
    {
        static const char rawBuf[] = "view test";
        CORE::CAsciiStringView view( rawBuf, 9 );
        ASSERT_FALSE( view.IsNULLOrEmpty() );
        ASSERT_TRUE(  9  == view.Length() );
        ASSERT_TRUE(  view.C_String() == rawBuf );   /* same pointer — no copy */
        ASSERT_TRUE(  view.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CAsciiStringView] Construct from CAsciiString: no copy" )
    try
    {
        CORE::CAsciiString owned( "owned string" );
        CORE::CAsciiStringView view( owned );
        ASSERT_TRUE(  view.C_String() == owned.C_String() );
        ASSERT_TRUE(  view.Length()   == owned.Length() );
        ASSERT_TRUE(  view.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CAsciiStringView] Copy rebinds link; original buffer shared" )
    try
    {
        static const char rawBuf[] = "shared";
        CORE::CAsciiStringView view1( rawBuf, 6 );
        CORE::CAsciiStringView view2( view1 );
        ASSERT_TRUE(  view2.C_String() == rawBuf );
        ASSERT_TRUE(  view2.C_String() == view1.C_String() );
        ASSERT_TRUE(  view2.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CAsciiStringView] Implicit conversion to const CAsciiString& works" )
    try
    {
        CORE::CAsciiString owned( "implicit test" );
        CORE::CAsciiStringView view( owned );

        /* Equals() takes const CAsciiString& — implicit conversion used here */
        ASSERT_TRUE(  owned.Equals( view ) );
        ASSERT_TRUE(  view.Equals( owned ) );

        /* Assign to const ref — zero copy */
        const CORE::CAsciiString& ref = view;
        ASSERT_TRUE(  ref.C_String() == owned.C_String() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CAsciiStringView] Const read operations delegate correctly" )
    try
    {
        static const char rawBuf[] = "Hello World";
        CORE::CAsciiStringView view( rawBuf, 11 );

        ASSERT_TRUE(  11 == view.Length() );
        ASSERT_FALSE( view.IsNULLOrEmpty() );
        ASSERT_TRUE(  4 == view.HasChar( 'o' ) );
        ASSERT_TRUE(  6 == view.HasSubstr( "World" ) );
        ASSERT_TRUE(  view.Lowercase() == "hello world" );
        ASSERT_TRUE(  view.Uppercase() == "HELLO WORLD" );
        /* endIndex is exclusive per the SubstrFromRange() contract */
        ASSERT_TRUE(  view.SubstrFromRange( 0, 5 ) == "Hello" );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformUtf8StringViewTests( void )
{
    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf8StringView] Construct from (char*, byteSize, length): no copy" )
    try
    {
        static const char rawBuf[] = "utf8 view";
        CORE::CUtf8StringView view( rawBuf, 9, 9 );
        ASSERT_FALSE( view.IsNULLOrEmpty() );
        ASSERT_TRUE(  9 == view.Length() );
        ASSERT_TRUE(  view.C_String() == rawBuf );
        ASSERT_TRUE(  view.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf8StringView] Construct from null-terminated char*: no copy" )
    try
    {
        static const char rawBuf[] = "null term";
        CORE::CUtf8StringView view( rawBuf );
        ASSERT_TRUE(  9 == view.Length() );
        ASSERT_TRUE(  view.C_String() == rawBuf );
        ASSERT_TRUE(  view.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf8StringView] Construct from CUtf8String: no copy" )
    try
    {
        CORE::CUtf8String owned( "owned utf8" );
        CORE::CUtf8StringView view( owned );
        ASSERT_TRUE(  view.C_String() == owned.C_String() );
        ASSERT_TRUE(  view.Length()   == owned.Length() );
        ASSERT_TRUE(  view.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf8StringView] Implicit conversion to const CUtf8String& works" )
    try
    {
        CORE::CUtf8String owned( "utf8 implicit" );
        CORE::CUtf8StringView view( owned );

        ASSERT_TRUE(  owned.Equals( view ) );

        const CORE::CUtf8String& ref = view;
        ASSERT_TRUE(  ref.C_String() == owned.C_String() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformUtf16StringViewTests( void )
{
    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16StringView] Construct from CUtf16String: no copy" )
    try
    {
        CORE::CUtf16String owned( "utf16 view" );
        CORE::CUtf16StringView view( owned );
        ASSERT_TRUE(  view.C_String() == owned.C_String() );
        ASSERT_TRUE(  view.Length()   == owned.Length() );
        ASSERT_TRUE(  view.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16StringView] Implicit conversion to const CUtf16String& works" )
    try
    {
        CORE::CUtf16String owned( "utf16 implicit" );
        CORE::CUtf16StringView view( owned );

        ASSERT_TRUE(  owned.Equals( view ) );

        const CORE::CUtf16String& ref = view;
        ASSERT_TRUE(  ref.C_String() == owned.C_String() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf16StringView] Construct from (UInt16*, codeUnits, length)" )
    try
    {
        CORE::CUtf16String owned( "xyz" );
        const CORE::UInt16* rawBuf = owned.C_String();
        CORE::CUtf16StringView view( rawBuf, 3, 3 );
        ASSERT_TRUE(  view.C_String() == rawBuf );
        ASSERT_TRUE(  3 == view.Length() );
        ASSERT_TRUE(  view.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformUtf32StringViewTests( void )
{
    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf32StringView] Construct from CUtf32String: no copy" )
    try
    {
        CORE::CUtf32String owned( "utf32 view" );
        CORE::CUtf32StringView view( owned );
        ASSERT_TRUE(  view.C_String() == owned.C_String() );
        ASSERT_TRUE(  view.Length()   == owned.Length() );
        ASSERT_TRUE(  view.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf32StringView] Implicit conversion to const CUtf32String& works" )
    try
    {
        CORE::CUtf32String owned( "utf32 implicit" );
        CORE::CUtf32StringView view( owned );

        ASSERT_TRUE(  owned.Equals( view ) );

        const CORE::CUtf32String& ref = view;
        ASSERT_TRUE(  ref.C_String() == owned.C_String() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CUtf32StringView] Construct from (UInt32*, length)" )
    try
    {
        CORE::CUtf32String owned( "abc" );
        const CORE::UInt32* rawBuf = owned.C_String();
        CORE::CUtf32StringView view( rawBuf, 3 );
        ASSERT_TRUE(  view.C_String() == rawBuf );
        ASSERT_TRUE(  3 == view.Length() );
        ASSERT_TRUE(  view.AsString().IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Regression: the reverse (back to front) operations of CUtf8String read one byte before the string buffer
 *  when they reached the first code point. This is only reliably detected with AddressSanitizer enabled
 */
void
PerformUtf8ReverseIterationRegressionTests( void )
{
    GUCEF_TESTFW_TESTCASE( "[CUtf8String] Reverse operations reaching the first code point" )
    try
    {
        CORE::CUtf8String str( "abc" );
        ASSERT_TRUE( 0 == str.HasChar( 'a', false ) );
        ASSERT_TRUE( -1 == str.HasChar( 'z', false ) );
        ASSERT_TRUE( 2 == str.HasChar( 'c', false ) );

        CORE::CUtf8String whitespace( "   " );
        ASSERT_TRUE( whitespace.Trim( false ).IsNULLOrEmpty() );

        CORE::CUtf8String path( "a/bc" );
        ASSERT_TRUE( path.SubstrToChar( 'a', false ) == "/bc" );

        CORE::CUtf8String noMatch( "xyz" );
        // back to front comparisons reaching the first code point, also of interest under AddressSanitizer
        ASSERT_TRUE( 2 == noMatch.FindMaxSubstrEquality( "qyz", 0, false, true ) );
        ASSERT_TRUE( 3 == noMatch.FindMaxSubstrEquality( "xyz", 0, false, true ) );

        // multi byte code points: U+00E9 is encoded as 2 bytes
        CORE::CUtf8String multiByte( "a\xC3\xA9z" );
        ASSERT_TRUE( 2 == multiByte.FindMaxSubstrEquality( CORE::CUtf8String( "b\xC3\xA9z" ), 0, false, true ) );
        ASSERT_TRUE( 1 == multiByte.FindMaxSubstrEquality( CORE::CUtf8String( "\xC3\xA8z" ), 0, false, true ) );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Regression: constructing a string from a byte range which does not include a null terminator looked at the
 *  byte just beyond the range to find one. When that byte happened to be 0, as is the case for any string literal,
 *  the final content byte was mistaken for the terminator: ByteSize() came out one too small and the last
 *  code point was cut in half
 */
static void
PerformUtf8StringFromByteRangeRegressionTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: CUtf8String from a byte range without terminator keeps all content" )
        try
        {
            // 'h' + e-acute + 'llo w' + u-circumflex + 'rld ' + euro sign: 17 bytes, 13 code points
            const char* literal = "h\xC3\xA9llo w\xC3\xBBrld \xE2\x82\xAC";
            const UInt32 contentBytes = 17;

            // The range excludes the terminator while the byte after it, the literal's own terminator, is 0
            CORE::CUtf8String exactRange( literal, contentBytes );
            ASSERT_TRUE( contentBytes + 1 == exactRange.ByteSize() );
            ASSERT_TRUE( 13 == exactRange.Length() );
            ASSERT_TRUE( contentBytes == exactRange.STL_String().size() );
            ASSERT_TRUE( 0 == memcmp( literal, exactRange.C_String(), contentBytes + 1 ) );

            // Same outcome as a string created from the terminated literal
            CORE::CUtf8String fromLiteral( literal );
            ASSERT_TRUE( fromLiteral.ByteSize() == exactRange.ByteSize() );
            ASSERT_TRUE( fromLiteral == exactRange );

            // A range that does include the terminator
            CORE::CUtf8String withTerminator( literal, contentBytes + 1 );
            ASSERT_TRUE( contentBytes + 1 == withTerminator.ByteSize() );
            ASSERT_TRUE( 13 == withTerminator.Length() );
            ASSERT_TRUE( fromLiteral == withTerminator );

            // A buffer where the byte beyond the range is non zero
            char buffer[ 18 ];
            memcpy( buffer, literal, contentBytes );
            buffer[ contentBytes ] = 'X';
            CORE::CUtf8String fromBuffer( buffer, contentBytes );
            ASSERT_TRUE( contentBytes + 1 == fromBuffer.ByteSize() );
            ASSERT_TRUE( fromLiteral == fromBuffer );

            // ASCII only content, where the cut off was just as wrong
            CORE::CUtf8String ascii( "plain text", 10 );
            ASSERT_TRUE( 11 == ascii.ByteSize() );
            ASSERT_TRUE( 10 == ascii.Length() );
            ASSERT_TRUE( ascii == "plain text" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Appending a UTF-8 sequence in pieces, as a streaming decoder does, must give the same string as appending
 *  it in one go whatever the split points are. Every piece is its own exact size heap allocation so that
 *  any read or write beyond it is reported by AddressSanitizer
 */
static void
AppendPiecesAndCompare( const CORE::CUtf8String& prefix  ,
                        const char* bytes                ,
                        UInt32 byteCount                 ,
                        UInt32 cutMask                   ,
                        bool includeTerminatorInPieces   ,
                        bool& allOk                      )
{
    // reference: the whole content in one go
    CORE::CUtf8String expected( prefix );
    expected.Append( bytes, byteCount );

    CORE::CUtf8String pieced( prefix );
    UInt32 start = 0;
    for ( UInt32 i=1; i<=byteCount; ++i )
    {
        // bit n of the mask set means: cut after byte n+1
        if ( i == byteCount || 0 != ( cutMask & ( 1 << ( i-1 ) ) ) )
        {
            UInt32 pieceSize = i - start;
            UInt32 allocSize = includeTerminatorInPieces ? pieceSize+1 : pieceSize;
            char* piece = new char[ allocSize ];
            memcpy( piece, bytes+start, pieceSize );
            if ( includeTerminatorInPieces )
                piece[ pieceSize ] = '\0';
            pieced.Append( piece, allocSize );
            delete[] piece;
            start = i;
        }
    }

    // The result must be self consistent and equal to the one go result
    CORE::CUtf8String fromScratch( pieced.C_String(), pieced.ByteSize()-1 );
    if ( pieced.ByteSize() != expected.ByteSize()             ||
         0 != memcmp( pieced.C_String(), expected.C_String(), expected.ByteSize() ) ||
         pieced.Length() != expected.Length()                 ||
         pieced.Length() != fromScratch.Length()              ||
         pieced.Length() != pieced.Uppercase().Length()       ||
         !( pieced == expected ) )
    {
        allOk = false;
    }
}

static void
PerformUtf8AppendIncompleteSequenceTests( void )
{
    GUCEF_TESTFW_TESTCASE( "[CUtf8String] Append of a sequence in pieces at every split point equals Append in one go" )
    try
    {
        static const char ascii[]  = { 'A', 0 };
        static const char twoB[]   = { '\xC3', '\xA9', 0 };                      // U+00E9
        static const char threeB[] = { '\xE2', '\x82', '\xAC', 0 };              // euro sign
        static const char fourB[]  = { '\xF0', '\x9F', '\x98', '\x80', 0 };      // U+1F600
        static const char mixed[]  = { 'a', '\xC3', '\xA9', '\xE2', '\x82', '\xAC', '\xF0', '\x9F', '\x98', '\x80', 'z', 0 };
        static const char* const sequences[] = { ascii, twoB, threeB, fourB, mixed };
        static const UInt32 sequenceSizes[] = { 1, 2, 3, 4, 11 };
        static const char* const prefixes[] = { "", "ab", "\xC3\xA9", "\xE2\x82\xAC\xC3\xA9x" };

        bool allOk = true;
        for ( UInt32 s=0; s<5; ++s )
        {
            for ( UInt32 p=0; p<4; ++p )
            {
                UInt32 combinations = 1 << ( sequenceSizes[ s ]-1 );
                for ( UInt32 mask=0; mask<combinations; ++mask )
                {
                    AppendPiecesAndCompare( CORE::CUtf8String( prefixes[ p ] ), sequences[ s ], sequenceSizes[ s ], mask, false, allOk );
                    AppendPiecesAndCompare( CORE::CUtf8String( prefixes[ p ] ), sequences[ s ], sequenceSizes[ s ], mask, true, allOk );
                }
            }
        }
        ASSERT_TRUE( allOk );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "[CUtf8String] Append of the euro sign byte by byte" )
    try
    {
        CORE::CUtf8String str;
        str.Append( "\xE2", 1 );
        ASSERT_TRUE( 2 == str.ByteSize() );
        str.Append( "\x82", 1 );
        ASSERT_TRUE( 3 == str.ByteSize() );
        str.Append( "\xAC", 1 );
        ASSERT_TRUE( 4 == str.ByteSize() );
        ASSERT_TRUE( 1 == str.Length() );
        ASSERT_TRUE( 0 == memcmp( str.C_String(), "\xE2\x82\xAC", 4 ) );

        // and the string keeps working as a string afterwards
        str.Append( "x", 1 );
        ASSERT_TRUE( 2 == str.Length() );
        ASSERT_TRUE( 5 == str.ByteSize() );
        ASSERT_TRUE( str == CORE::CUtf8String( "\xE2\x82\xAC" "x" ) );
        ASSERT_TRUE( 1 == str.HasChar( (char) 'x', true ) );
        ASSERT_TRUE( 0x20AC == str.CodepointAtIndex( 0 ) );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "[CUtf8String] Append of the other overloads on an incomplete sequence" )
    try
    {
        // Append( const char* ) and Append( CUtf8String ) with the continuation bytes
        CORE::CUtf8String viaCString( "a\xE2" );
        viaCString.Append( "\x82\xAC" );
        ASSERT_TRUE( 2 == viaCString.Length() );
        ASSERT_TRUE( viaCString == CORE::CUtf8String( "a\xE2\x82\xAC" ) );

        CORE::CUtf8String viaString( "\xC3" );
        viaString.Append( CORE::CUtf8String( "\xA9" "b" ) );
        ASSERT_TRUE( 2 == viaString.Length() );
        ASSERT_TRUE( viaString == CORE::CUtf8String( "\xC3\xA9" "b" ) );

        CORE::CUtf8String viaOperator( "\xF0\x9F" );
        viaOperator += CORE::CUtf8String( "\x98\x80" );
        ASSERT_TRUE( 1 == viaOperator.Length() );
        ASSERT_TRUE( 5 == viaOperator.ByteSize() );

        CORE::CUtf8String viaCodePoint( "\xE2\x82" );
        viaCodePoint.Append( (UInt32) 'q' );
        ASSERT_TRUE( 0 == memcmp( viaCodePoint.C_String(), "\xE2\x82q", 4 ) );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "[CUtf8String] Append of the string to itself and of a part of itself" )
    try
    {
        // growing the buffer moves it, the bytes to append must not be read from the old one
        CORE::CUtf8String str( "ab\xC3\xA9" );
        str.Append( str );
        ASSERT_TRUE( str == CORE::CUtf8String( "ab\xC3\xA9" "ab\xC3\xA9" ) );
        ASSERT_TRUE( 6 == str.Length() );
        str += str;
        ASSERT_TRUE( 12 == str.Length() );
        ASSERT_TRUE( 17 == str.ByteSize() );

        CORE::CUtf8String part( "0123456789" );
        part.Append( part.C_String()+5, 5 );
        ASSERT_TRUE( part == "012345678956789" );
        ASSERT_TRUE( 15 == part.Length() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "[CUtf8String] Append to a string with spare room and to a linked string" )
    try
    {
        CORE::CUtf8String roomy;
        roomy.Reserve( 64 );
        roomy.Set( "\xE2\x82", 2, false );          // cut off, with plenty of unused room behind it
        roomy.Append( "\xAC" "x", 2 );
        ASSERT_TRUE( 2 == roomy.Length() );
        ASSERT_TRUE( roomy == CORE::CUtf8String( "\xE2\x82\xAC" "x" ) );

        const char linkedContent[] = "h\xC3\xA9";
        CORE::CUtf8String linked;
        linked.LinkTo( linkedContent );
        linked.Append( "llo", 3 );
        ASSERT_TRUE( 5 == linked.Length() );
        ASSERT_TRUE( linked == CORE::CUtf8String( "h\xC3\xA9llo" ) );
        ASSERT_TRUE( 0 == strcmp( linkedContent, "h\xC3\xA9" ) );   // the external buffer is left alone
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "[CUtf8String] A string ending in a cut off sequence stays inside its buffer for the string operations" )
    try
    {
        static const char* const cutOff[] = { "\xE2", "\xE2\x82", "\xF0", "\xF0\x9F", "\xF0\x9F\x98", "\xC3", "a\xC3", "ab\xE2\x82", "aab\xE2" };
        for ( UInt32 i=0; i<9; ++i )
        {
            CORE::CUtf8String str( cutOff[ i ] );
            UInt32 byteLength = (UInt32) strlen( cutOff[ i ] );
            ASSERT_TRUE( str.ByteSize() == byteLength+1 );

            // none of these may walk beyond the terminator of the string
            CORE::CUtf8String upper = str.Uppercase();
            CORE::CUtf8String lower = str.Lowercase();
            ASSERT_TRUE( upper.ByteSize() == str.ByteSize() );
            ASSERT_TRUE( lower.ByteSize() == str.ByteSize() );
            str.HasChar( 'z', true );
            str.HasChar( 'z', false );
            str.Trim( true );
            str.Trim( false );
            str.ReplaceChar( 'a', 'b' );
            str.RemoveChar( 'a' );
            str.CompactRepeatingChar( 'a' );
            str.SubstrToChar( 'a', true );
            str.SubstrToChar( 'a', false );
            str.SubstrFromRange( 0, str.Length() );
            str.CodepointAtIndex( 0 );
            str.CodepointAtIndex( str.Length()-1 );
            str.STL_String();
            ASSERT_TRUE( str == CORE::CUtf8String( cutOff[ i ] ) );
        }

        // the bytes of the cut off sequence are carried over as they are
        ASSERT_TRUE( CORE::CUtf8String( "ab\xE2\x82" ).ReplaceChar( 'a', 'z' ) == CORE::CUtf8String( "zb\xE2\x82" ) );
        ASSERT_TRUE( CORE::CUtf8String( "ab\xE2\x82" ).RemoveChar( 'a' ) == CORE::CUtf8String( "b\xE2\x82" ) );
        ASSERT_TRUE( CORE::CUtf8String( "aab\xF0\x9F" ).CompactRepeatingChar( 'a' ) == CORE::CUtf8String( "ab\xF0\x9F" ) );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "[CUtf8String] Append of invalid sequences is memory safe and consistent with building the string in one go" )
    try
    {
        static const char overlong2[] = { '\xC0', '\x80' };
        static const char overlong3[] = { '\xE0', '\x80', '\x80' };
        static const char strayCont[] = { '\x80', '\x80', 'a' };
        static const char leadThenAscii[] = { '\xE2', 'a', 'b', 'c' };
        static const char leadAtEnd[] = { 'a', 'b', '\xF0' };
        static const char leadLeadCont[] = { '\xE2', '\xE2', '\x82', '\xAC' };
        static const char* const inputs[] = { overlong2, overlong3, strayCont, leadThenAscii, leadAtEnd, leadLeadCont };
        static const UInt32 inputSizes[] = { 2, 3, 3, 4, 3, 4 };

        bool allOk = true;
        for ( UInt32 i=0; i<6; ++i )
        {
            for ( UInt32 mask=0; mask < (UInt32) ( 1 << ( inputSizes[ i ]-1 ) ); ++mask )
            {
                // The one go reference is a string created from the same bytes: both must give the same string
                CORE::CUtf8String reference( inputs[ i ], inputSizes[ i ] );

                CORE::CUtf8String pieced;
                UInt32 start = 0;
                for ( UInt32 n=1; n<=inputSizes[ i ]; ++n )
                {
                    if ( n == inputSizes[ i ] || 0 != ( mask & ( 1 << ( n-1 ) ) ) )
                    {
                        char* piece = new char[ n-start ];
                        memcpy( piece, inputs[ i ]+start, n-start );
                        pieced.Append( piece, n-start );
                        delete[] piece;
                        start = n;
                    }
                }
                if ( pieced.ByteSize() != reference.ByteSize() ||
                     0 != memcmp( pieced.C_String(), reference.C_String(), reference.ByteSize() ) ||
                     pieced.Length() != reference.Length() )
                {
                    allOk = false;
                }
                // walking all code points must stay inside the buffer (checked by AddressSanitizer)
                pieced.Uppercase();
                pieced.STL_String();
            }
        }
        ASSERT_TRUE( allOk );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  UTF-8 input that is cut off (the continuation bytes of its last sequence are missing) must not be read beyond its
 *  terminator or the end of the byte range it was given. The cut off sequence is no code point, it decodes as U+FFFD
 */
template < class TString >
static void
PerformTypedCutOffUtf8InputTests( void )
{
    GUCEF_TESTFW_TESTCASE( ( CORE::CString( "[" ) + StringTypeName< TString >() + "] UTF-8 input with a cut off last sequence stays inside its buffer" ).C_String() )
    try
    {
        static const char* const cutOff[] = { "\xE2", "\xE2\x82", "\xF0", "\xF0\x9F", "\xF0\x9F\x98", "\xC3", "a\xC3", "ab\xE2\x82" };
        static const UInt32 sizes[] = { 1, 2, 1, 2, 3, 1, 2, 4 };
        static const UInt32 lengths[] = { 1, 1, 1, 1, 1, 1, 2, 3 };
        for ( UInt32 i=0; i<8; ++i )
        {
            // exact size heap buffers: any read beyond is reported by AddressSanitizer
            char* terminated = new char[ sizes[ i ]+1 ];
            memcpy( terminated, cutOff[ i ], sizes[ i ] );
            terminated[ sizes[ i ] ] = '\0';
            char* unterminated = new char[ sizes[ i ] ];
            memcpy( unterminated, cutOff[ i ], sizes[ i ] );

            TString fromTerminated( terminated );
            ASSERT_TRUE( lengths[ i ] == fromTerminated.Length() );
            ASSERT_TRUE( 0xFFFD == fromTerminated[ lengths[ i ]-1 ] );

            TString appended( "x" );
            appended.Append( terminated );
            ASSERT_TRUE( lengths[ i ]+1 == appended.Length() );
            ASSERT_TRUE( 0xFFFD == appended[ lengths[ i ] ] );

            TString fromRange( unterminated, sizes[ i ] );
            ASSERT_TRUE( fromRange.Length() <= lengths[ i ] );   // a range ending in the middle of a sequence may leave that sequence out

            delete[] terminated;
            delete[] unterminated;
        }
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  A UTF-16 surrogate pair appended in two pieces is one code point, as it is when appended in one go
 */
static void
PerformUtf16AppendSplitSurrogatePairTests( void )
{
    GUCEF_TESTFW_TESTCASE( "[CUtf16String] Append of a surrogate pair in pieces equals Append in one go" )
    try
    {
        const UInt16 pair[] = { 0xD83D, 0xDE00 };      // U+1F600

        CORE::CUtf16String oneGo( "ab" );
        oneGo.Append( pair, 2, -1 );
        ASSERT_TRUE( 3 == oneGo.Length() );

        CORE::CUtf16String inPieces( "ab" );
        inPieces.Append( pair, 1, -1 );
        inPieces.Append( pair+1, 1, -1 );
        ASSERT_TRUE( inPieces.ByteSize() == oneGo.ByteSize() );
        ASSERT_TRUE( 3 == inPieces.Length() );
        ASSERT_TRUE( inPieces == oneGo );
        ASSERT_TRUE( 0x1F600 == inPieces[ 2 ] );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  A string can be appended to itself
 */
template < class TString >
static void
PerformTypedSelfAppendTests( void )
{
    GUCEF_TESTFW_TESTCASE( ( CORE::CString( "[" ) + StringTypeName< TString >() + "] A string appended to itself doubles its content" ).C_String() )
    try
    {
        TString str( "abc" );
        str += str;
        ASSERT_TRUE( 6 == str.Length() );
        ASSERT_TRUE( str == TString( "abcabc" ) );

        str += str;
        ASSERT_TRUE( 12 == str.Length() );
        ASSERT_TRUE( str == TString( "abcabcabcabc" ) );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  An empty string is empty whatever storage it has: the writable C string of an empty string is "", not NULL,
 *  and empty strings with and without storage are equal
 */
template < class TString >
static void
PerformTypedEmptyStringTests( void )
{
    GUCEF_TESTFW_TESTCASE( ( CORE::CString( "[" ) + StringTypeName< TString >() + "] An empty string gives a writable empty C string and equals any other empty string" ).C_String() )
    try
    {
        TString empty;
        ASSERT_TRUE( GUCEF_NULL != empty.C_String() );                                   // used to be NULL for the non-const accessor
        ASSERT_TRUE( 0 == empty.C_String()[ 0 ] );
        ASSERT_TRUE( empty.IsNULLOrEmpty() && 0 == empty.Length() );

        const TString neverTouched;
        const TString fromLiteral( "" );
        ASSERT_TRUE( empty == neverTouched && neverTouched == empty );                   // storage or not, both are empty
        ASSERT_TRUE( empty == fromLiteral && fromLiteral == empty );
        ASSERT_TRUE( neverTouched == fromLiteral && fromLiteral == neverTouched );
        ASSERT_FALSE( empty == TString( "a" ) || TString( "a" ) == empty );

        // and it stays usable as a normal string
        empty += "xy";
        ASSERT_TRUE( empty == TString( "xy" ) && 2 == empty.Length() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformStringTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CString TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CString" );

    PerformTypedStringTests< CORE::CAsciiString >();
    PerformTypedStringTests< CORE::CUtf8String  >();
    PerformTypedStringTests< CORE::CUtf16String >();
    PerformTypedStringTests< CORE::CUtf32String >();
    PerformUtf8SpecificStringTests();
    PerformUtf16SpecificStringTests();
    PerformUtf32SpecificStringTests();
    PerformTypedStringLinkedModeTests< CORE::CAsciiString >();
    PerformTypedStringLinkedModeTests< CORE::CUtf8String  >();
    PerformTypedStringLinkedModeTests< CORE::CUtf16String >();
    PerformTypedStringLinkedModeTests< CORE::CUtf32String >();
    PerformAsciiStringLinkedAndViewTests();
    PerformUtf8StringViewTests();
    PerformUtf16StringViewTests();
    PerformUtf32StringViewTests();
    PerformUtf8ReverseIterationRegressionTests();
    PerformUtf8StringFromByteRangeRegressionTests();
    PerformUtf8AppendIncompleteSequenceTests();
    PerformTypedCutOffUtf8InputTests< CORE::CUtf16String >();
    PerformTypedCutOffUtf8InputTests< CORE::CUtf32String >();
    PerformUtf16AppendSplitSurrogatePairTests();
    PerformTypedSelfAppendTests< CORE::CAsciiString >();
    PerformTypedSelfAppendTests< CORE::CUtf8String  >();
    PerformTypedSelfAppendTests< CORE::CUtf16String >();
    PerformTypedSelfAppendTests< CORE::CUtf32String >();
    PerformTypedEmptyStringTests< CORE::CAsciiString >();
    PerformTypedEmptyStringTests< CORE::CUtf8String  >();
    PerformTypedEmptyStringTests< CORE::CUtf16String >();
    PerformTypedEmptyStringTests< CORE::CUtf32String >();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CString TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
