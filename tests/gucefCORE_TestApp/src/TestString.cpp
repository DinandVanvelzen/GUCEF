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
        ASSERT_TRUE(  linked.C_String() == owned.C_String() );
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
        ASSERT_TRUE(  linked.C_String() == rawBuf );
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
        ASSERT_TRUE(  view.SubstrFromRange( 0, 4 ) == "Hello" );
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

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CString TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
