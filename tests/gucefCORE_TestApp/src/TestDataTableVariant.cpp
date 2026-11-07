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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATE_H
#include "gucefCORE_CDate.h"
#define GUCEF_CORE_CDATE_H
#endif /* GUCEF_CORE_CDATE_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#include "gucefCORE_CVariantBinarySerializer.h"
#define GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CVARIANTBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDataTableSupport.h"
#include "TestDataTableVariant.h"

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
using namespace GUCEF::CORE;

#if ( GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_LITTLE_ENDIAN )
static const UInt8 g_nativeDateId = GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH;
static const UInt8 g_foreignDateId = GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH;
static const UInt8 g_nativeMicrosId = GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH;
static const UInt8 g_foreignMicrosId = GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH;
#else
static const UInt8 g_nativeDateId = GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH;
static const UInt8 g_foreignDateId = GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH;
static const UInt8 g_nativeMicrosId = GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH;
static const UInt8 g_foreignMicrosId = GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH;
#endif

static const Int64 g_microsPerDay = 86400000000LL;

/*-------------------------------------------------------------------------*/

static Int32
DaysOf( Int32 year, UInt32 month, UInt32 day )
{
    Int32 days = 0;
    CDataTableValues::DaysFromCivil( year, month, day, days );
    return days;
}

/*-------------------------------------------------------------------------*/

static bool
ParseTs( const char* text, TDataTableTimeUnit unit, Int64& ticks )
{
    return CDataTableValues::ParseTimestamp( text, (UInt32) strlen( text ), unit, ticks );
}

/*-------------------------------------------------------------------------*/

void
PerformDataTableVariantTests( void )
{
    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING data table CVariant type id TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DataTableVariant" );

    GUCEF_TESTFW_TESTCASE( "Test: date and microsecond type ids, names and fixed sizes" )
        try
        {
            ASSERT_TRUE( 80 == GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH && 81 == GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH );
            ASSERT_TRUE( 82 == GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH && 83 == GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH );
            ASSERT_TRUE( g_nativeDateId == GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH );
            ASSERT_TRUE( g_nativeMicrosId == GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH );
            ASSERT_TRUE( 0 == strcmp( "LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH", TypeNameForGucefTypeId( GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH ) ) );
            ASSERT_TRUE( 0 == strcmp( "BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH", TypeNameForGucefTypeId( GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH ) ) );
            ASSERT_TRUE( 0 == strcmp( "LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH", TypeNameForGucefTypeId( GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH ) ) );
            ASSERT_TRUE( 0 == strcmp( "BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH", TypeNameForGucefTypeId( GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH ) ) );
            ASSERT_TRUE( GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == GucefTypeIdForTypeName( "BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH == GucefTypeIdForTypeName( GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH ) );
            ASSERT_TRUE( GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == CVariant::TypeIdForTypeName( GUCEF_DATATYPE_NAME_DATE_IN_DAYS_SINCE_UNIX_EPOCH ) );
            ASSERT_TRUE( GUCEF_DATATYPE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH == CVariant::TypeIdForTypeName( GUCEF_DATATYPE_NAME_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH ) );
            ASSERT_TRUE( 4 == GucefByteSizeOfFixedSizeType( GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH ) );
            ASSERT_TRUE( 4 == GucefByteSizeOfFixedSizeType( GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH ) );
            ASSERT_TRUE( 8 == GucefByteSizeOfFixedSizeType( GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH ) );
            ASSERT_TRUE( 8 == CVariant::ByteSizeOfFixedSizeType( GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH ) );
            ASSERT_FALSE( CVariant::UsesDynamicMemory( GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH ) );
            ASSERT_FALSE( CVariant::UsesDynamicMemory( GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a date in a variant: classification, payload, text form and calendar range" )
        try
        {
            CDate date( 2026, 11, 3 );
            CVariant var = CDataTableValues::Date( date );
            ASSERT_TRUE( g_nativeDateId == var.GetTypeId() );
            ASSERT_TRUE( 4 == var.ByteSize() );
            ASSERT_TRUE( var.IsInteger() && var.IsSignedInteger() && var.IsNumber() );
            ASSERT_FALSE( var.IsUnsignedInteger() || var.IsFloat() || var.IsString() || var.UsesDynamicMemory() || var.IsEndianAgnostic() );
            ASSERT_TRUE( date.ToDaysSinceUnixEpoch() == var.AsInt32() );
            ASSERT_TRUE( (Int64) date.ToDaysSinceUnixEpoch() == var.AsInt64() );
            ASSERT_TRUE( var.AsString() == "2026-11-03" && var.AsUtf8String() == "2026-11-03" );
            ASSERT_TRUE( var.AsString( CString::Empty, true ) == "2026-11-03" );
            Int32 payload = 0;
            memcpy( &payload, var.AsVoidPtr(), sizeof( payload ) );
            ASSERT_TRUE( payload == date.ToDaysSinceUnixEpoch() );

            ASSERT_TRUE( CDataTableValues::FromDays( var.AsInt32() ) == date );
            ASSERT_TRUE( CDataTableValues::ToDays( CDate( 1970, 1, 1 ) ) == 0 );
            ASSERT_TRUE( CDataTableValues::Date( -1 ).AsString() == "1969-12-31" );
            ASSERT_TRUE( CDataTableValues::Date( DaysOf( 1962, 1, 2 ) ).AsString() == "1962-01-02" );
            ASSERT_TRUE( CDataTableValues::Date( DaysOf( 0, 1, 1 ) ).AsString() == "0000-01-01" );
            ASSERT_TRUE( CDataTableValues::Date( DaysOf( -1, 12, 31 ) ).AsString() == "-0001-12-31" );
            ASSERT_TRUE( CDataTableValues::Date( DaysOf( 9999, 12, 31 ) ).AsString() == "9999-12-31" );
            ASSERT_TRUE( CDataTableValues::Date( DaysOf( 9999, 12, 31 ) + 1 ).AsString() == "+10000-01-01" );
            ASSERT_TRUE( 2932896 == DaysOf( 9999, 12, 31 ) && -719528 == DaysOf( 0, 1, 1 ) );

            // an invalid date makes no variant
            ASSERT_FALSE( CDataTableValues::Date( CDate( 2026, 2, 30 ) ).IsInitialized() );

            // every Int32 day count formats and parses back
            const Int32 extremes[] = { GUCEF_INT32MIN, GUCEF_INT32MIN + 1, -1, 0, 1, GUCEF_INT32MAX - 1, GUCEF_INT32MAX };
            for ( UInt32 i=0; i<sizeof( extremes ) / sizeof( extremes[ 0 ] ); ++i )
            {
                char text[ CDataTableValues::MaxDateTextLength + 1 ];
                UInt32 length = CDataTableValues::FormatDate( extremes[ i ], text, sizeof( text ) );
                ASSERT_TRUE( length > 0 && length <= CDataTableValues::MaxDateTextLength && strlen( text ) == length );
                Int32 back = 0;
                ASSERT_TRUE( CDataTableValues::ParseDate( text, length, back ) && back == extremes[ i ] );
            }
            char tooSmall[ 8 ];
            ASSERT_TRUE( 0 == CDataTableValues::FormatDate( 0, tooSmall, sizeof( tooSmall ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: date text parsing, SetFromString and byte order conversions" )
        try
        {
            Int32 days = 0;
            ASSERT_TRUE( CDataTableValues::ParseDate( "2024-02-29", 10, days ) && DaysOf( 2024, 2, 29 ) == days );
            ASSERT_FALSE( CDataTableValues::ParseDate( "2026-02-29", 10, days ) );
            ASSERT_FALSE( CDataTableValues::ParseDate( "2026-13-01", 10, days ) );
            ASSERT_FALSE( CDataTableValues::ParseDate( "2026-00-10", 10, days ) );
            ASSERT_FALSE( CDataTableValues::ParseDate( "2026-1-10", 9, days ) );
            ASSERT_FALSE( CDataTableValues::ParseDate( "26-01-10", 8, days ) );
            ASSERT_FALSE( CDataTableValues::ParseDate( "20260110", 8, days ) );
            ASSERT_FALSE( CDataTableValues::ParseDate( "12026-01-10", 11, days ) );            // more than 4 digits needs a sign
            ASSERT_FALSE( CDataTableValues::ParseDate( "2026-01-10T", 11, days ) );
            ASSERT_TRUE( CDataTableValues::ParseDate( "+12026-01-10", 12, days ) && DaysOf( 12026, 1, 10 ) == days );

            CVariant var;
            ASSERT_TRUE( var.SetFromString( GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, "1962-01-02" ) );
            ASSERT_TRUE( g_nativeDateId == var.GetTypeId() && DaysOf( 1962, 1, 2 ) == var.AsInt32() );
            ASSERT_FALSE( var.SetFromString( GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, "not a date" ) );
            ASSERT_TRUE( var.SetFromString( GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, "not a date", CDataTableValues::Date( 5 ) ) && 5 == var.AsInt32() );

            CVariant constructed( GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, CString( "2000-03-01" ) );
            ASSERT_TRUE( DaysOf( 2000, 3, 1 ) == constructed.AsInt32() );

            // the foreign byte order id holds swapped bytes and still reads correctly
            CVariant foreign;
            ASSERT_TRUE( foreign.SetFromString( g_foreignDateId, "1962-01-02" ) );
            ASSERT_TRUE( g_foreignDateId == foreign.GetTypeId() );
            ASSERT_TRUE( DaysOf( 1962, 1, 2 ) == foreign.AsInt32() && foreign.AsString() == "1962-01-02" );
            CVariant swapped( foreign );
            swapped.ToNativeEndian();
            ASSERT_TRUE( g_nativeDateId == swapped.GetTypeId() && DaysOf( 1962, 1, 2 ) == swapped.AsInt32() );
            swapped.ToBigEndian();
            ASSERT_TRUE( GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == swapped.GetTypeId() && swapped.IsBigEndian() && !swapped.IsLittleEndian() );
            swapped.ToLittleEndian();
            ASSERT_TRUE( GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == swapped.GetTypeId() && swapped.IsLittleEndian() );
            ASSERT_TRUE( DaysOf( 1962, 1, 2 ) == swapped.AsInt32() );

            // dates as date times and timestamps
            CDateTime dateTime = CDataTableValues::Date( DaysOf( 2026, 11, 3 ) ).AsDateTime();
            ASSERT_TRUE( 2026 == dateTime.GetYear() && 11 == dateTime.GetMonth() && 3 == dateTime.GetDay() && 0 == dateTime.GetHours() );
            ASSERT_TRUE( CDataTableValues::Date( -3 ).AsDateTime().GetYear() == 1969 );
            ASSERT_TRUE( CDataTableValues::Date( DaysOf( 2026, 11, 3 ) ).AsTimestamp().ToMillisecondsSinceEpoch() == (UInt64) DaysOf( 2026, 11, 3 ) * 86400000ULL );

            // dates compare as numbers, also against plain integers
            ASSERT_TRUE( CDataTableValues::Date( -10 ) < CDataTableValues::Date( 3 ) );
            ASSERT_FALSE( CDataTableValues::Date( 3 ) < CDataTableValues::Date( -10 ) );
            ASSERT_TRUE( CDataTableValues::Date( 7 ) == CDataTableValues::Date( 7 ) );
            ASSERT_TRUE( CDataTableValues::Date( -2 ) <= CVariant( (Int32) -2 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: microsecond timestamps: signed ticks, ISO 8601 text forms and byte order" )
        try
        {
            CVariant var = CDataTableValues::Timestamp( -1, DATATABLE_TIMEUNIT_MICROSECONDS );
            ASSERT_TRUE( g_nativeMicrosId == var.GetTypeId() && 8 == var.ByteSize() );
            ASSERT_TRUE( -1 == var.AsInt64() );
            ASSERT_TRUE( var.IsInteger() && var.IsSignedInteger() && !var.IsUnsignedInteger() );
            ASSERT_TRUE( var.AsString() == "1969-12-31T23:59:59.999999Z" );

            Int64 ticks = 0;
            ASSERT_TRUE( ParseTs( "2026-11-03T14:30:00.123456Z", DATATABLE_TIMEUNIT_MICROSECONDS, ticks ) );
            const Int64 expected = (Int64) DaysOf( 2026, 11, 3 ) * g_microsPerDay + ( 14LL * 3600 + 30 * 60 ) * 1000000LL + 123456;
            ASSERT_TRUE( expected == ticks );
            ASSERT_TRUE( CDataTableValues::Timestamp( ticks, DATATABLE_TIMEUNIT_MICROSECONDS ).AsString() == "2026-11-03T14:30:00.123456Z" );

            ASSERT_TRUE( var.SetFromString( GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH, "2026-11-03T15:30:00.123456+01:00" ) );
            ASSERT_TRUE( g_nativeMicrosId == var.GetTypeId() && expected == var.AsInt64() );
            ASSERT_FALSE( var.SetFromString( GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH, "2026-11-03T14:30:00.1234567Z" ) );
            ASSERT_TRUE( var.SetFromString( GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH, "2026-11-03T14:30:00.1234560Z" ) && expected == var.AsInt64() );
            ASSERT_TRUE( var.SetFromString( GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH, "1962-01-02" ) && (Int64) DaysOf( 1962, 1, 2 ) * g_microsPerDay == var.AsInt64() );
            ASSERT_TRUE( var.AsString() == "1962-01-02T00:00:00.000000Z" );

            CVariant foreign;
            ASSERT_TRUE( foreign.SetFromString( g_foreignMicrosId, "1969-12-31T23:59:59.999999Z" ) );
            ASSERT_TRUE( g_foreignMicrosId == foreign.GetTypeId() && -1 == foreign.AsInt64() );
            foreign.ToNativeEndian();
            ASSERT_TRUE( g_nativeMicrosId == foreign.GetTypeId() && -1 == foreign.AsInt64() );
            foreign.ToBigEndian();
            ASSERT_TRUE( foreign.IsBigEndian() && -1 == foreign.AsInt64() );
            foreign.ToLittleEndian();
            ASSERT_TRUE( foreign.IsLittleEndian() && -1 == foreign.AsInt64() );

            // as date time (milliseconds, floor) and as timestamp (positive instants only)
            CVariant afternoon = CDataTableValues::Timestamp( expected, DATATABLE_TIMEUNIT_MICROSECONDS );
            CDateTime dateTime = afternoon.AsDateTime();
            ASSERT_TRUE( 2026 == dateTime.GetYear() && 14 == dateTime.GetHours() && 30 == dateTime.GetMinutes() && 123 == dateTime.GetMilliseconds() );
            ASSERT_TRUE( afternoon.AsTimestamp().ToMicrosecondsSinceEpoch() == (UInt64) expected );
            CDateTime beforeEpoch = CDataTableValues::Timestamp( -1, DATATABLE_TIMEUNIT_MICROSECONDS ).AsDateTime();
            ASSERT_TRUE( 1969 == beforeEpoch.GetYear() && 23 == beforeEpoch.GetHours() && 999 == beforeEpoch.GetMilliseconds() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the existing timestamp ids are signed: ticks before 1970 round trip and order correctly" )
        try
        {
            const Int64 dayBefore = -86400000LL;
            CVariant ms = CDataTableValues::Timestamp( dayBefore, DATATABLE_TIMEUNIT_MILLISECONDS );
            ASSERT_TRUE( GUCEF_DATATYPE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH == ms.GetTypeId() );
            ASSERT_TRUE( ms.IsSignedInteger() && !ms.IsUnsignedInteger() && ms.IsInteger() );
            ASSERT_TRUE( dayBefore == ms.AsInt64() );
            ASSERT_TRUE( ms.AsString() == "-86400000" );

            CVariant nanos = CDataTableValues::Timestamp( -5, DATATABLE_TIMEUNIT_NANOSECONDS );
            ASSERT_TRUE( GUCEF_DATATYPE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH == nanos.GetTypeId() && -5 == nanos.AsInt64() );

            // ordering of negative ticks, which an unsigned reading would put after every positive one
            CVariant later = CDataTableValues::Timestamp( 3, DATATABLE_TIMEUNIT_MILLISECONDS );
            ASSERT_TRUE( ms < later );
            ASSERT_FALSE( later < ms );
            ASSERT_TRUE( ms <= later && later > ms && later >= ms );
            ASSERT_TRUE( nanos < CDataTableValues::Timestamp( 0, DATATABLE_TIMEUNIT_NANOSECONDS ) );
            ASSERT_TRUE( ms == CVariant( dayBefore ) );

            // seconds too
            CVariant seconds;
            ASSERT_TRUE( seconds.SetFromString( GUCEF_DATATYPE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH, "-1234" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH == seconds.GetTypeId() && -1234 == seconds.AsInt64() );
            ASSERT_TRUE( seconds.AsString() == "-1234" );

            // SetFromString keeps the id of either byte order
            CVariant msText;
            ASSERT_TRUE( msText.SetFromString( GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH, "-42" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH == msText.GetTypeId() );
            msText.ToNativeEndian();
            ASSERT_TRUE( GUCEF_DATATYPE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH == msText.GetTypeId() && -42 == msText.AsInt64() );

            // a timestamp of CTimestamp still converts as before
            CTimestamp now = CTimestamp::NowUTCTime();
            CVariant fromTimestamp( now );
            ASSERT_TRUE( fromTimestamp.AsTimestamp() == now );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CVariantBinarySerializer round trips dates and timestamps of both byte orders" )
        try
        {
            const UInt8 ids[] = { GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH,
                                  GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH,
                                  GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH };
            const char* texts[] = { "1962-01-02", "2026-11-03", "1969-12-31T23:59:59.999999Z", "2026-11-03T14:30:00.000001Z", "-86400000", "-5" };
            const UInt32 offsets[] = { 0, 3 };
            for ( UInt32 i=0; i<sizeof( ids ); ++i )
            {
                CVariant var;
                ASSERT_TRUE( var.SetFromString( ids[ i ], texts[ i ] ) );
                ASSERT_TRUE( ids[ i ] == var.GetTypeId() );
                for ( UInt32 o=0; o<2; ++o )
                {
                    CDynamicBuffer buffer;
                    UInt32 bytesWritten = 0;
                    ASSERT_TRUE( CVariantBinarySerializer::Serialize( var, offsets[ o ], buffer, bytesWritten ) );
                    ASSERT_TRUE( 1 + var.ByteSize() == bytesWritten );
                    CVariant back;
                    UInt32 bytesRead = 0;
                    ASSERT_TRUE( CVariantBinarySerializer::Deserialize( back, offsets[ o ], buffer, false, bytesRead ) );
                    ASSERT_TRUE( bytesRead == bytesWritten );
                    ASSERT_TRUE( back.GetTypeId() == var.GetTypeId() && back.AsInt64() == var.AsInt64() );
                    ASSERT_TRUE( back.AsString() == var.AsString() );
                }

                CDynamicBuffer raw;
                UInt32 rawWritten = 0;
                ASSERT_TRUE( CVariantBinarySerializer::SerializeRaw( var, 0, raw, rawWritten ) && var.ByteSize() == rawWritten );
                CVariant rawBack;
                UInt32 rawRead = 0;
                ASSERT_TRUE( CVariantBinarySerializer::DeserializeRaw( rawBack, var.GetTypeId(), 0, raw, false, rawRead ) );
                ASSERT_TRUE( rawRead == rawWritten && rawBack.AsInt64() == var.AsInt64() );

                // a truncated payload is refused
                CDynamicBuffer truncated;
                UInt32 written = 0;
                CVariantBinarySerializer::Serialize( var, 0, truncated, written );
                truncated.SetDataSize( written - 1 );
                CVariant none;
                UInt32 read = 0;
                ASSERT_FALSE( CVariantBinarySerializer::Deserialize( none, 0, truncated, false, read ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: JSON text carries dates and microsecond timestamps as ISO 8601 strings" )
        try
        {
            CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORETEST::GetDataTableTestCodec( "json", "dstorepluginJSONPARSER", codec ) );
            if ( !codec.IsNULL() )
            {
                CDataNode tree;
                CDataNode* main = tree.Structure( "Main", '/' );
                main->SetAttribute( "date", CDataTableValues::Date( DaysOf( 1962, 1, 2 ) ) );
                CVariant foreign;
                foreign.SetFromString( g_foreignMicrosId, "2026-11-03T14:30:00.123456Z" );
                main->SetAttribute( "ts", foreign );

                CDynamicBuffer output;
                {
                    CDynamicBufferAccess outputAccess( &output, false );
                    ASSERT_TRUE( codec->StoreDataTree( &tree, &outputAccess ) );
                }
                CString text = CORETEST::DataTableTestBufferText( output );
                ASSERT_TRUE( text.HasSubstr( "\"1962-01-02\"" ) >= 0 );
                ASSERT_TRUE( text.HasSubstr( "\"2026-11-03T14:30:00.123456Z\"" ) >= 0 );

                CDynamicBufferAccess rereadAccess( output );
                CDataNode reread;
                ASSERT_TRUE( codec->BuildDataTree( &reread, &rereadAccess ) );
                const CDataNode* readMain = reread.Search( "Main", '/', true, true );
                ASSERT_TRUE( GUCEF_NULL != readMain );
                if ( GUCEF_NULL != readMain )
                {
                    CVariant date;
                    ASSERT_TRUE( date.SetFromString( GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, readMain->GetAttributeValueOrChildValueByName( "date" ).AsString() ) );
                    ASSERT_TRUE( DaysOf( 1962, 1, 2 ) == date.AsInt32() );
                    CVariant ts;
                    ASSERT_TRUE( ts.SetFromString( GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH, readMain->GetAttributeValueOrChildValueByName( "ts" ).AsString() ) );
                    ASSERT_TRUE( foreign.AsInt64() == ts.AsInt64() );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CDataTableValues tick conversions, date time conversions and timestamp parsing" )
        try
        {
            Int64 result = 0;
            ASSERT_TRUE( CDataTableValues::ConvertTicks( 5, DATATABLE_TIMEUNIT_MILLISECONDS, DATATABLE_TIMEUNIT_NANOSECONDS, result ) && 5000000 == result );
            ASSERT_TRUE( CDataTableValues::ConvertTicks( -7000, DATATABLE_TIMEUNIT_MICROSECONDS, DATATABLE_TIMEUNIT_MILLISECONDS, result ) && -7 == result );
            ASSERT_FALSE( CDataTableValues::ConvertTicks( -7001, DATATABLE_TIMEUNIT_MICROSECONDS, DATATABLE_TIMEUNIT_MILLISECONDS, result ) );
            ASSERT_FALSE( CDataTableValues::ConvertTicks( GUCEF_INT64MAX / 10, DATATABLE_TIMEUNIT_MILLISECONDS, DATATABLE_TIMEUNIT_MICROSECONDS, result ) );
            ASSERT_TRUE( CDataTableValues::ConvertTicksRounded( -7001, DATATABLE_TIMEUNIT_MICROSECONDS, DATATABLE_TIMEUNIT_MILLISECONDS, false, result ) && -8 == result );
            ASSERT_TRUE( CDataTableValues::ConvertTicksRounded( -7001, DATATABLE_TIMEUNIT_MICROSECONDS, DATATABLE_TIMEUNIT_MILLISECONDS, true, result ) && -7 == result );
            ASSERT_TRUE( CDataTableValues::ConvertTicksRounded( 7001, DATATABLE_TIMEUNIT_MICROSECONDS, DATATABLE_TIMEUNIT_MILLISECONDS, true, result ) && 8 == result );
            ASSERT_TRUE( 1000LL == CDataTableValues::TicksPerSecond( DATATABLE_TIMEUNIT_MILLISECONDS ) && g_microsPerDay == CDataTableValues::TicksPerDay( DATATABLE_TIMEUNIT_MICROSECONDS ) );

            TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MILLISECONDS;
            ASSERT_TRUE( CDataTableValues::TimeUnitFromTypeId( GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH, unit ) && DATATABLE_TIMEUNIT_MICROSECONDS == unit );
            ASSERT_TRUE( CDataTableValues::TimeUnitFromTypeId( GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH, unit ) && DATATABLE_TIMEUNIT_NANOSECONDS == unit );
            ASSERT_FALSE( CDataTableValues::TimeUnitFromTypeId( GUCEF_DATATYPE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH, unit ) );
            ASSERT_FALSE( CDataTableValues::TimeUnitFromTypeId( GUCEF_DATATYPE_INT64, unit ) );
            ASSERT_TRUE( GUCEF_DATATYPE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH == CDataTableValues::TimestampTypeId( DATATABLE_TIMEUNIT_MILLISECONDS ) );

            // a date time with a zone converts to UTC ticks
            CDateTime local( CDate( 2026, 11, 3 ), CTime( 15, 30, 0, 250 ), 60 );
            ASSERT_TRUE( CDataTableValues::TicksFromDateTime( local, DATATABLE_TIMEUNIT_MICROSECONDS, result ) );
            ASSERT_TRUE( (Int64) DaysOf( 2026, 11, 3 ) * g_microsPerDay + ( 14LL * 3600 + 30 * 60 ) * 1000000LL + 250000 == result );
            ASSERT_TRUE( CDataTableValues::Timestamp( local, DATATABLE_TIMEUNIT_MILLISECONDS ).AsInt64() == result / 1000 );
            CDateTime before1970( CDate( 1962, 1, 2 ), CTime( 9, 30, 0, 0 ), 0 );
            ASSERT_TRUE( CDataTableValues::TicksFromDateTime( before1970, DATATABLE_TIMEUNIT_MILLISECONDS, result ) && result < 0 );
            CDateTime back;
            ASSERT_TRUE( CDataTableValues::DateTimeFromTicks( result, DATATABLE_TIMEUNIT_MILLISECONDS, back ) );
            ASSERT_TRUE( 1962 == back.GetYear() && 1 == back.GetMonth() && 2 == back.GetDay() && 9 == back.GetHours() && 30 == back.GetMinutes() );

            // variants of any unit or a date to ticks, exactly
            ASSERT_TRUE( CDataTableValues::TicksFromVariant( CDataTableValues::Date( -1 ), DATATABLE_TIMEUNIT_MICROSECONDS, result ) && -g_microsPerDay == result );
            CVariant seconds;
            seconds.SetFromString( GUCEF_DATATYPE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH, "-2" );
            ASSERT_TRUE( CDataTableValues::TicksFromVariant( seconds, DATATABLE_TIMEUNIT_MILLISECONDS, result ) && -2000 == result );
            ASSERT_FALSE( CDataTableValues::TicksFromVariant( CDataTableValues::Timestamp( 1500, DATATABLE_TIMEUNIT_NANOSECONDS ), DATATABLE_TIMEUNIT_MICROSECONDS, result ) );
            ASSERT_TRUE( CDataTableValues::TicksFromVariant( CDataTableValues::Timestamp( 2000, DATATABLE_TIMEUNIT_NANOSECONDS ), DATATABLE_TIMEUNIT_MICROSECONDS, result ) && 2 == result );
            ASSERT_FALSE( CDataTableValues::TicksFromVariant( CVariant( (Int64) 5 ), DATATABLE_TIMEUNIT_MICROSECONDS, result ) );

            // ISO 8601 forms
            Int64 ticks = 0;
            const Int64 midnight = (Int64) DaysOf( 2026, 11, 3 ) * 86400000LL;
            ASSERT_TRUE( ParseTs( "2026-11-03 14:30", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) && midnight + ( 14LL * 3600 + 30 * 60 ) * 1000 == ticks );
            ASSERT_TRUE( ParseTs( "2026-11-03T14:30:00,5Z", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) && midnight + ( 14LL * 3600 + 30 * 60 ) * 1000 + 500 == ticks );
            ASSERT_TRUE( ParseTs( "2026-11-03T09:00:00-05:30", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) && midnight + ( 14LL * 3600 + 30 * 60 ) * 1000 == ticks );
            ASSERT_TRUE( ParseTs( "2026-11-03T09:00:00-0530", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) && midnight + ( 14LL * 3600 + 30 * 60 ) * 1000 == ticks );
            ASSERT_TRUE( ParseTs( "2026-11-03T14:30:00.123456789Z", DATATABLE_TIMEUNIT_NANOSECONDS, ticks ) );
            ASSERT_TRUE( CDataTableValues::TimestampToString( ticks, DATATABLE_TIMEUNIT_NANOSECONDS ) == "2026-11-03T14:30:00.123456789Z" );
            ASSERT_TRUE( CDataTableValues::TimestampToString( 0, DATATABLE_TIMEUNIT_MILLISECONDS, false ) == "1970-01-01T00:00:00.000" );
            ASSERT_FALSE( ParseTs( "2026-11-03T24:00:00Z", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) );
            ASSERT_FALSE( ParseTs( "2026-11-03T14:60:00Z", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) );
            ASSERT_FALSE( ParseTs( "2026-11-03T14:30:00Zjunk", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) );
            ASSERT_FALSE( ParseTs( "2026-11-03T14:30:00.Z", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) );
            ASSERT_FALSE( ParseTs( "2026-11-03T1430", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) );
            ASSERT_FALSE( ParseTs( "+300000-01-01T00:00:00Z", DATATABLE_TIMEUNIT_NANOSECONDS, ticks ) );    // beyond Int64 nanoseconds
            ASSERT_FALSE( ParseTs( "", DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) );

            // the extremes of every unit format within the maximum length and parse back
            const TDataTableTimeUnit units[] = { DATATABLE_TIMEUNIT_MILLISECONDS, DATATABLE_TIMEUNIT_MICROSECONDS, DATATABLE_TIMEUNIT_NANOSECONDS };
            const Int64 extremes[] = { GUCEF_INT64MIN, GUCEF_INT64MIN + 1, -1, 0, 1, GUCEF_INT64MAX };
            for ( UInt32 u=0; u<3; ++u )
            {
                for ( UInt32 e=0; e<sizeof( extremes ) / sizeof( extremes[ 0 ] ); ++e )
                {
                    char text[ CDataTableValues::MaxTimestampTextLength + 1 ];
                    UInt32 length = CDataTableValues::FormatTimestamp( extremes[ e ], units[ u ], true, text, sizeof( text ) );
                    ASSERT_TRUE( length > 0 && length <= CDataTableValues::MaxTimestampTextLength );
                    Int64 parsed = 0;
                    ASSERT_TRUE( CDataTableValues::ParseTimestamp( text, length, units[ u ], parsed ) && parsed == extremes[ e ] );
                }
            }

            // civil date helpers
            Int32 days = 0;
            ASSERT_FALSE( CDataTableValues::DaysFromCivil( 2026, 13, 1, days ) );
            ASSERT_FALSE( CDataTableValues::DaysFromCivil( 2026, 4, 31, days ) );
            Int32 year = 0;
            UInt32 month = 0, day = 0;
            CDataTableValues::CivilFromDays( GUCEF_INT32MIN, year, month, day );
            ASSERT_TRUE( CDataTableValues::DaysFromCivil( year, month, day, days ) && GUCEF_INT32MIN == days );
            CDataTableValues::CivilFromDays( GUCEF_INT32MAX, year, month, day );
            ASSERT_TRUE( CDataTableValues::DaysFromCivil( year, month, day, days ) && GUCEF_INT32MAX == days );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: nanosecond, second and millisecond timestamps of both byte orders read back the value that was set" )
        try
        {
            const UInt8 ids[] = { GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH,
                                  GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH,
                                  GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH };
            const char* texts[] = { "0", "1000", "-5", "1700000000123", "-86400000", "9223372036854775807", "-9223372036854775808" };
            const Int64 values[] = { 0, 1000, -5, 1700000000123LL, -86400000LL, GUCEF_INT64MAX, GUCEF_INT64MIN };
            for ( UInt32 i=0; i<sizeof( ids ); ++i )
            {
                for ( UInt32 t=0; t<sizeof( values ) / sizeof( values[ 0 ] ); ++t )
                {
                    CVariant var;
                    ASSERT_TRUE( var.SetFromString( ids[ i ], texts[ t ] ) );
                    ASSERT_TRUE( ids[ i ] == var.GetTypeId() );
                    ASSERT_TRUE( var.IsSignedInteger() && !var.IsUnsignedInteger() );
                    ASSERT_TRUE( values[ t ] == var.AsInt64() );
                    ASSERT_TRUE( (UInt64) values[ t ] == var.AsUInt64() );
                    ASSERT_TRUE( (size_t) values[ t ] == var.AsSizeT() );
                    ASSERT_TRUE( var.AsString() == texts[ t ] );

                    // the same bytes handed to Set in the byte order of the id
                    Int64 ticks = values[ t ];
                    if ( var.IsBigEndian() != ( GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_BIG_ENDIAN ) )
                        ticks = SwapEndianInt64( ticks );
                    CVariant viaSet;
                    ASSERT_TRUE( viaSet.Set( &ticks, sizeof( ticks ), ids[ i ], false ) );
                    ASSERT_TRUE( values[ t ] == viaSet.AsInt64() );
                    ASSERT_TRUE( var == viaSet );

                    // the byte order conversions keep the value
                    CVariant native = var;
                    native.ToNativeEndian();
                    ASSERT_TRUE( values[ t ] == native.AsInt64() && native.AsString() == texts[ t ] );
                    ASSERT_TRUE( native == var );
                    CVariant swapped = native;
                    #if ( GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_LITTLE_ENDIAN )
                    swapped.ToBigEndian();
                    #else
                    swapped.ToLittleEndian();
                    #endif
                    ASSERT_TRUE( values[ t ] == swapped.AsInt64() && swapped.AsString() == texts[ t ] );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
