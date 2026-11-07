/*
 *  gucefMULTIPLAY_DesktopNotifyTestApp: Application for testing the desktop match alert sink plugin of gucefMULTIPLAY
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXT_H
#include "notifysinkpluginDESKTOP_CDesktopMatchAlertText.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXT_H ? */

#include "TestDesktopNotifyCommon.h"
#include "TestDesktopNotify.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::MULTIPLAYPLUGIN::DESKTOP;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** The ellipsis the text is cut with: U+2026 in UTF-8 */
#define ELLIPSIS "\xE2\x80\xA6"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A string of the given bytes, which may hold NUL and bytes that are not UTF-8 */
static CORE::CString
Bytes( const char* data, UInt32 size )
{
    return CORE::CString( data, size );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Repeat( const char* unit, UInt32 times )
{
    CORE::CString result;
    for ( UInt32 i=0; i<times; ++i )
        result += unit;
    return result;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Clean( const char* text )
{
    return CDesktopMatchAlertText::Sanitize( CORE::CString( text ), 160, false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDesktopNotifyTextTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "notifysinkpluginDESKTOP: the text of a notification" );

    GUCEF_TESTFW_TESTCASE( "Test: Well formed text stays as it is, white space is collapsed and trimmed" )
        try
        {
            ASSERT_TRUE( "Friday game" == Clean( "Friday game" ) );
            ASSERT_TRUE( (Clean( "" )).IsNULLOrEmpty() );
            ASSERT_TRUE( (Clean( "   \t\r\n  " )).IsNULLOrEmpty() );
            ASSERT_TRUE( "a b" == Clean( "  a \t\r\n b  " ) );
            ASSERT_TRUE( "Z\xC3\xBCrich \xE2\x82\xAC \xF0\x9F\x98\x80" == Clean( "Z\xC3\xBCrich \xE2\x82\xAC \xF0\x9F\x98\x80" ) );
            ASSERT_TRUE( "$(touch x); |&" == Clean( "$(touch x); |&" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Control characters, line breaks, ESC, NUL and the characters that hide or reorder text become a space" )
        try
        {
            ASSERT_TRUE( "a b c" == Clean( "a\nb\rc" ) );
            ASSERT_TRUE( "a [31mred" == Clean( "a\x1b[31mred" ) );
            // a string holds no NUL inside, so what a NUL leaves of the text is up to the string; it is never a NUL or a control in the result
            CORE::CString withNul = CDesktopMatchAlertText::Sanitize( Bytes( "a\0b", 3 ), 160, false );
            ASSERT_TRUE( ( "a" == withNul || "a b" == withNul ) && !TestHasControl( withNul ) );
            ASSERT_TRUE( "ab" == CDesktopMatchAlertText::Sanitize( Bytes( "ab\0\0", 4 ), 160, false ) );
            ASSERT_TRUE( (CDesktopMatchAlertText::Sanitize( Bytes( "\0\0\0", 3 ), 160, false )).IsNULLOrEmpty() );
            ASSERT_TRUE( "a b" == Clean( "a\x7f" "b" ) );
            ASSERT_TRUE( "a b" == Clean( "a\xC2\x85" "b" ) );                // U+0085, a C1 control
            ASSERT_TRUE( "a b" == Clean( "a\xE2\x80\xAE" "b" ) );            // U+202E right-to-left override
            ASSERT_TRUE( "a b" == Clean( "a\xE2\x80\x8B" "b" ) );            // U+200B zero width space
            ASSERT_TRUE( "a b" == Clean( "a\xEF\xBB\xBF" "b" ) );            // U+FEFF
            ASSERT_TRUE( "a b" == Clean( "a\xE2\x80\xA8" "b" ) );            // U+2028 line separator
            ASSERT_TRUE( "a b" == Clean( "a\xE2\x81\xA9" "b" ) );            // U+2069 pop directional isolate
            for ( int c=1; c<0x20; ++c )
            {
                char text[ 3 ] = { 'x', (char) c, 'y' };
                CORE::CString cleaned = CDesktopMatchAlertText::Sanitize( Bytes( text, 3 ), 160, false );
                ASSERT_TRUE( "x y" == cleaned && !TestHasControl( cleaned ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Invalid UTF-8 is dropped and what is around it is kept" )
        try
        {
            ASSERT_TRUE( "ab" == Clean( "a\xFF" "b" ) );
            ASSERT_TRUE( "ab" == Clean( "a\x80" "b" ) );                      // a lone continuation byte
            ASSERT_TRUE( "ab" == Clean( "a\xC0\x80" "b" ) );                  // overlong NUL
            ASSERT_TRUE( "ab" == Clean( "a\xC1\xBF" "b" ) );                  // overlong
            ASSERT_TRUE( "ab" == Clean( "a\xE0\x80\x80" "b" ) );              // overlong
            ASSERT_TRUE( "ab" == Clean( "a\xF0\x80\x80\x80" "b" ) );          // overlong
            ASSERT_TRUE( "ab" == Clean( "a\xED\xA0\x80" "b" ) );              // a surrogate
            ASSERT_TRUE( "ab" == Clean( "a\xF4\x90\x80\x80" "b" ) );          // above U+10FFFF
            ASSERT_TRUE( "ab" == Clean( "a\xF8\x88\x80\x80\x80" "b" ) );      // five byte form
            ASSERT_TRUE( "ab" == Clean( "a\xE2\x82" "b" ) );                  // cut short, then ASCII
            ASSERT_TRUE( "ab" == Clean( "ab\xE2\x82" ) );                     // cut short at the end
            ASSERT_TRUE( "ab" == Clean( "ab\xF0\x9F\x98" ) );
            ASSERT_TRUE( "a\xC3\xBC" "b" == Clean( "a\xC3\xBC\xFF" "b" ) );   // valid text around it
            ASSERT_TRUE( "\xF0\x9F\x98\x80" == Clean( "\xF0\x9F\x98\x80\xF0\x9F" ) );
            // however the bytes are mixed, what comes out is well formed
            UInt32 state = 12345;
            for ( int round=0; round<200; ++round )
            {
                char data[ 64 ];
                for ( int i=0; i<64; ++i )
                {
                    state = state * 1664525u + 1013904223u;
                    data[ i ] = (char)( state >> 24 );
                }
                CORE::CString cleaned = CDesktopMatchAlertText::Sanitize( Bytes( data, 64 ), 160, true );
                ASSERT_TRUE( TestIsValidUtf8( cleaned ) && !TestHasControl( cleaned ) && TestByteCount( cleaned ) <= 160 );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Text that is too long is cut with an ellipsis, never inside a character" )
        try
        {
            // the summary: at most 64 bytes
            ASSERT_TRUE( Repeat( "a", 64 ) == CDesktopMatchAlertText::SanitizeSummary( Repeat( "a", 64 ) ) );
            ASSERT_TRUE( Repeat( "a", 61 ) + ELLIPSIS == CDesktopMatchAlertText::SanitizeSummary( Repeat( "a", 65 ) ) );
            ASSERT_TRUE( Repeat( "a", 61 ) + ELLIPSIS == CDesktopMatchAlertText::SanitizeSummary( Repeat( "a", 100 ) ) );
            ASSERT_TRUE( 64 == TestByteCount( CDesktopMatchAlertText::SanitizeSummary( Repeat( "a", 100 ) ) ) );
            // the body: at most 160 bytes
            ASSERT_TRUE( Repeat( "b", 160 ) == CDesktopMatchAlertText::SanitizeBody( Repeat( "b", 160 ) ) );
            ASSERT_TRUE( Repeat( "b", 157 ) + ELLIPSIS == CDesktopMatchAlertText::SanitizeBody( Repeat( "b", 161 ) ) );

            // two byte characters: 30 of them and the ellipsis are 63 bytes, a 31st would make 65
            CORE::CString twoByte = CDesktopMatchAlertText::SanitizeSummary( Repeat( "\xC3\xA9", 50 ) );
            ASSERT_TRUE( Repeat( "\xC3\xA9", 30 ) + ELLIPSIS == twoByte && TestIsValidUtf8( twoByte ) );
            // four byte characters: 15 and the ellipsis
            CORE::CString fourByte = CDesktopMatchAlertText::SanitizeSummary( Repeat( "\xF0\x9F\x98\x80", 30 ) );
            ASSERT_TRUE( Repeat( "\xF0\x9F\x98\x80", 15 ) + ELLIPSIS == fourByte && TestIsValidUtf8( fourByte ) );
            // three byte characters, with every offset of the cut against the characters
            for ( UInt32 lead=0; lead<4; ++lead )
            {
                CORE::CString text = Repeat( "a", lead ) + Repeat( "\xE2\x82\xAC", 40 );
                CORE::CString cut = CDesktopMatchAlertText::SanitizeSummary( text );
                ASSERT_TRUE( TestIsValidUtf8( cut ) && TestByteCount( cut ) <= 64 && TestByteCount( cut ) >= 61 );
                ASSERT_TRUE( cut.HasSubstr( ELLIPSIS ) >= 0 );
            }
            // a character which does not fit whole is left out
            ASSERT_TRUE( Repeat( "a", 60 ) + ELLIPSIS == CDesktopMatchAlertText::SanitizeSummary( Repeat( "a", 60 ) + Repeat( "\xC3\xA9", 10 ) ) );
            // no space in front of the ellipsis
            ASSERT_TRUE( Repeat( "a", 60 ) + ELLIPSIS == CDesktopMatchAlertText::SanitizeSummary( Repeat( "a", 60 ) + " bbbbbbbbbb" ) );
            // limits too small for an ellipsis
            ASSERT_TRUE( (CDesktopMatchAlertText::Sanitize( "abcdef", 0, false )).IsNULLOrEmpty() );
            ASSERT_TRUE( "ab" == CDesktopMatchAlertText::Sanitize( "abcdef", 2, false ) );
            ASSERT_TRUE( "abc" == CDesktopMatchAlertText::Sanitize( "abcdef", 3, false ) );
            ASSERT_TRUE( CORE::CString( "a" ELLIPSIS ) == CDesktopMatchAlertText::Sanitize( "abcdef", 4, false ) );
            ASSERT_TRUE( "abcdef" == CDesktopMatchAlertText::Sanitize( "abcdef", 6, false ) );
            ASSERT_TRUE( (CDesktopMatchAlertText::Sanitize( "\xC3\xA9", 1, false )).IsNULLOrEmpty() );
            // an enormous text costs no more than a small one and stays in bounds
            CORE::CString hugeUnit = Repeat( "xyz\n\xFF\xE2", 1000 );
            CORE::CString huge = Repeat( hugeUnit.C_String(), 200 );
            CORE::CString hugeCut = CDesktopMatchAlertText::SanitizeBody( huge );
            ASSERT_TRUE( TestIsValidUtf8( hugeCut ) && TestByteCount( hugeCut ) <= 160 && !TestHasControl( hugeCut ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The body escapes < > and &, the summary does not, and the limit counts what is escaped without cutting an entity" )
        try
        {
            ASSERT_TRUE( "a&lt;b&gt;&amp;c" == CDesktopMatchAlertText::SanitizeBody( "a<b>&c" ) );
            ASSERT_TRUE( "a<b>&c" == CDesktopMatchAlertText::SanitizeSummary( "a<b>&c" ) );
            ASSERT_TRUE( "&lt;b&gt;hello&lt;/b&gt;" == CDesktopMatchAlertText::SanitizeBody( "<b>hello</b>" ) );
            ASSERT_TRUE( "&amp;lt;" == CDesktopMatchAlertText::SanitizeBody( "&lt;" ) );             // already an entity: still escaped, shown as typed
            // exactly 160 bytes escaped fits
            ASSERT_TRUE( Repeat( "&lt;", 40 ) == CDesktopMatchAlertText::SanitizeBody( Repeat( "<", 40 ) ) );
            // a longer text is cut after whole entities
            CORE::CString amps = CDesktopMatchAlertText::SanitizeBody( Repeat( "&", 100 ) );
            ASSERT_TRUE( Repeat( "&amp;", 31 ) + ELLIPSIS == amps );
            ASSERT_TRUE( TestCountOccurrences( amps, "&" ) == TestCountOccurrences( amps, "&amp;" ) );
            CORE::CString lts = CDesktopMatchAlertText::SanitizeBody( Repeat( "<", 100 ) );
            ASSERT_TRUE( Repeat( "&lt;", 39 ) + ELLIPSIS == lts );
            ASSERT_TRUE( TestByteCount( lts ) <= 160 && TestByteCount( amps ) <= 160 );
            // without escaping nothing changes
            ASSERT_TRUE( "a<b>&c" == CDesktopMatchAlertText::Sanitize( "a<b>&c", 160, false ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The wording is filled in once, left to right, and what is put in is not looked at again" )
        try
        {
            ASSERT_TRUE( "It is M's turn in T (3)" == CDesktopMatchAlertText::Expand( "It is {mover}'s turn in {title} ({count})", "T", "M", 3 ) );
            ASSERT_TRUE( "{mover} vs X" == CDesktopMatchAlertText::Expand( "{title} vs {mover}", "{mover}", "X", 1 ) );
            ASSERT_TRUE( "{count}{title}" == CDesktopMatchAlertText::Expand( "{mover}{title}", "{title}", "{count}", 9 ) );
            ASSERT_TRUE( "T T T" == CDesktopMatchAlertText::Expand( "{title} {title} {title}", "T", "", 1 ) );
            ASSERT_TRUE( "{foo} {title {count" == CDesktopMatchAlertText::Expand( "{foo} {title {count", "T", "M", 2 ) );
            ASSERT_TRUE( "{T}" == CDesktopMatchAlertText::Expand( "{{title}}", "T", "M", 2 ) );
            ASSERT_TRUE( "12 games" == CDesktopMatchAlertText::Expand( "{count} games", "T", "M", 12 ) );
            ASSERT_TRUE( (CDesktopMatchAlertText::Expand( "", "T", "M", 2 )).IsNULLOrEmpty() );
            ASSERT_TRUE( "no placeholders" == CDesktopMatchAlertText::Expand( "no placeholders", "T", "M", 2 ) );
            ASSERT_TRUE( "after " == CDesktopMatchAlertText::Expand( "after {mover}", "T", "", 2 ) );
            // a value that is very long is cut before it is put in
            CORE::CString big = CDesktopMatchAlertText::Expand( "{title}", Repeat( "a", 100000 ), "", 1 );
            ASSERT_TRUE( TestByteCount( big ) <= 512 && TestByteCount( big ) > 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A category is letters, digits, dot, underscore and dash up to 64 bytes" )
        try
        {
            ASSERT_TRUE( CDesktopMatchAlertText::IsValidCategory( "network.message" ) );
            ASSERT_TRUE( CDesktopMatchAlertText::IsValidCategory( "x-game_turn.1" ) );
            ASSERT_TRUE( CDesktopMatchAlertText::IsValidCategory( Repeat( "c", 64 ) ) );
            ASSERT_FALSE( CDesktopMatchAlertText::IsValidCategory( Repeat( "c", 65 ) ) );
            ASSERT_FALSE( CDesktopMatchAlertText::IsValidCategory( "" ) );
            ASSERT_FALSE( CDesktopMatchAlertText::IsValidCategory( "a b" ) );
            ASSERT_FALSE( CDesktopMatchAlertText::IsValidCategory( "a;b" ) );
            ASSERT_FALSE( CDesktopMatchAlertText::IsValidCategory( "a=b" ) );
            ASSERT_FALSE( CDesktopMatchAlertText::IsValidCategory( "a\nb" ) );
            ASSERT_FALSE( CDesktopMatchAlertText::IsValidCategory( "\xC3\xA9" ) );
            ASSERT_FALSE( CDesktopMatchAlertText::IsValidCategory( Bytes( "a\0b", 3 ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
