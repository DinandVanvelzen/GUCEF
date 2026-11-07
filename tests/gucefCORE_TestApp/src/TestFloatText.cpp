/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <math.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestFloatText.h"

using namespace GUCEF;

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

/** Makes the C library use a decimal comma while it exists, restores the locale at the end of the scope */
class CCommaLocaleScope
{
    public:

    CCommaLocaleScope( void )
        : m_active( false )
        , m_previous()
    {
        const char* previous = setlocale( LC_NUMERIC, GUCEF_NULL );
        m_previous = GUCEF_NULL != previous ? previous : "C";

        const char* candidates[] = { "de_DE.UTF-8", "de_DE.utf8", "de_DE", "fr_FR.UTF-8", "fr_FR.utf8", "nl_NL.UTF-8", "nl_NL.utf8", "en_DK.UTF-8", "en_DK.utf8", "da_DK.UTF-8", "da_DK.utf8" };
        for ( size_t i=0; i<sizeof( candidates ) / sizeof( candidates[ 0 ] ) && !m_active; ++i )
        {
            if ( GUCEF_NULL != setlocale( LC_NUMERIC, candidates[ i ] ) )
            {
                const struct lconv* conv = localeconv();
                m_active = GUCEF_NULL != conv && GUCEF_NULL != conv->decimal_point && ',' == conv->decimal_point[ 0 ];
            }
        }
        if ( !m_active )
            setlocale( LC_NUMERIC, m_previous.C_String() );
    }

    ~CCommaLocaleScope()
    {
        setlocale( LC_NUMERIC, m_previous.C_String() );
    }

    /** True when a locale with a decimal comma could be selected */
    bool IsActive( void ) const { return m_active; }

    private:

    bool m_active;
    CORE::CString m_previous;
};

/*-------------------------------------------------------------------------*/

static UInt64
NextRandom( UInt64& state )
{
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

/*-------------------------------------------------------------------------*/

/** Reads the text with the decimal point '.' with the C library, which has to be in the C locale for this */
static bool
ReadWithCLibrary( const CORE::CString& text, Float64& value )
{
    char* end = GUCEF_NULL;
    value = strtod( text.C_String(), &end );
    return GUCEF_NULL != end && '\0' == *end && end != text.C_String();
}

/*-------------------------------------------------------------------------*/

static void
CheckKnownTexts( bool& ok )
{
    ok = ok && CORE::Float64ToRoundTripString( 0.5 ) == "0.5";
    ok = ok && CORE::Float64ToRoundTripString( 2.25 ) == "2.25";
    ok = ok && CORE::Float64ToRoundTripString( -1234.5 ) == "-1234.5";
    ok = ok && CORE::Float64ToRoundTripString( 3.0 ) == "3";
    ok = ok && CORE::Float64ToRoundTripString( 0.1 ) == "0.1";
    ok = ok && CORE::Float64ToRoundTripString( 1e22 ) == "1e+22";
    ok = ok && CORE::Float64ToRoundTripString( -0.0 ) == "-0";
    ok = ok && CORE::Float64ToRoundTripString( 0.0 ) == "0";
}

/*-------------------------------------------------------------------------*/

static void
CheckRoundTrips( bool& ok, UInt32 iterations )
{
    UInt64 state = 0x9E3779B97F4A7C15ULL;
    for ( UInt32 i=0; i<iterations && ok; ++i )
    {
        UInt64 bits = NextRandom( state );
        Float64 value = 0.0;
        memcpy( &value, &bits, sizeof( value ) );
        if ( value != value || value - value != 0.0 )
            continue;       // NaN and the infinities have no text form
        CORE::CString text = CORE::Float64ToRoundTripString( value );
        Float64 read = 0.0;
        ok = ok && !text.IsNULLOrEmpty() && ReadWithCLibrary( text, read ) && read == value && 0 > text.HasChar( ',' ) && 0 > text.HasChar( ' ' );
        Float64 invariant = 0.0;
        ok = ok && CORE::TryParseFloat64Invariant( text, invariant ) && invariant == value;

        // values with few digits: the text is short
        Float64 rounded = floor( value / 1e150 * 1000.0 ) / 1000.0;
        text = CORE::Float64ToRoundTripString( rounded );
        ok = ok && ReadWithCLibrary( text, read ) && read == rounded;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFloatTextTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Float64 text TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "FloatText" );

    GUCEF_TESTFW_TESTCASE( "Test: Float64ToRoundTripString gives the shortest text that reads back as the same double" )
        try
        {
            bool ok = true;
            CheckKnownTexts( ok );
            ASSERT_TRUE( ok );

            ASSERT_TRUE( CORE::Float64ToRoundTripString( HUGE_VAL ).IsNULLOrEmpty() );
            ASSERT_TRUE( CORE::Float64ToRoundTripString( -HUGE_VAL ).IsNULLOrEmpty() );
            ASSERT_TRUE( CORE::Float64ToRoundTripString( ( HUGE_VAL - HUGE_VAL ) ).IsNULLOrEmpty() );

            ok = true;
            CheckRoundTrips( ok, 20000 );
            ASSERT_TRUE( ok );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: TryParseFloat64Invariant reads only plain decimal numbers with a point" )
        try
        {
            Float64 value = 0.0;
            ASSERT_TRUE( CORE::TryParseFloat64Invariant( "0.5", value ) && 0.5 == value );
            ASSERT_TRUE( CORE::TryParseFloat64Invariant( "-12.25e2", value ) && -1225.0 == value );
            ASSERT_TRUE( CORE::TryParseFloat64Invariant( "42", value ) && 42.0 == value );
            ASSERT_TRUE( CORE::TryParseFloat64Invariant( "7.", value ) && 7.0 == value );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( "", value ) );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( "0,5", value ) );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( " 1", value ) );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( "1 ", value ) );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( "1x", value ) );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( "nan", value ) );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( "inf", value ) );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( "0x10", value ) );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( "1e999", value ) );       // not finite
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( ".", value ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Float64 text does not depend on the decimal separator of the C library locale" )
        try
        {
            CCommaLocaleScope comma;
            if ( !comma.IsActive() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FloatText: no locale with a decimal comma is installed, the locale part of this test is skipped" );
            }
            else
            {
                // the locale really changes the C library
                char probe[ 32 ];
                snprintf( probe, sizeof( probe ), "%g", 0.5 );
                ASSERT_TRUE( 0 == strcmp( probe, "0,5" ) );
            }

            bool ok = true;
            CheckKnownTexts( ok );
            ASSERT_TRUE( ok );

            UInt64 state = 0x2545F4914F6CDD1DULL;
            for ( UInt32 i=0; i<5000; ++i )
            {
                UInt64 bits = NextRandom( state );
                Float64 random = 0.0;
                memcpy( &random, &bits, sizeof( random ) );
                if ( random != random || random - random != 0.0 )
                    continue;
                CORE::CString text = CORE::Float64ToRoundTripString( random );
                Float64 read = 0.0;
                ASSERT_TRUE( 0 > text.HasChar( ',' ) && CORE::TryParseFloat64Invariant( text, read ) && read == random );
            }

            Float64 value = 0.0;
            ASSERT_TRUE( CORE::TryParseFloat64Invariant( "0.5", value ) && 0.5 == value );
            ASSERT_TRUE( CORE::TryParseFloat64Invariant( "1234.5678", value ) && 1234.5678 == value );
            ASSERT_FALSE( CORE::TryParseFloat64Invariant( "0,5", value ) );
            ASSERT_TRUE( CORE::Float64ToRoundTripString( 1234.5678 ) == "1234.5678" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Float64 text TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
