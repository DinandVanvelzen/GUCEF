/*
 *  gucefDRNUTILSGUI_TestApp: GUCEF test application for the gucefDRNUTILSGUI module
 *
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
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <time.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#include "TestDrnUtilsGuiFixtures.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The rules of two zones, as the application would supply them; GUCEF ships no zone data */
const char* const g_rulesJson =
    "{ \"rules\": ["
    "  { \"name\": \"US\", \"periods\": [ { \"fromYear\": 2007, \"toYear\": 9999, \"saveMinutes\": 60, \"start\": \"Mar Sun>=8 2:00\", \"end\": \"Nov Sun>=1 2:00\" } ] },"
    "  { \"name\": \"EU\", \"periods\": [ { \"fromYear\": 1996, \"toYear\": 9999, \"saveMinutes\": 60, \"start\": \"Mar lastSun 1:00u\", \"end\": \"Oct lastSun 1:00u\" } ] } ],"
    "  \"zones\": ["
    "  { \"name\": \"America/New_York\", \"standardOffsetMinutes\": -300, \"periods\": [ { \"fromYear\": 1987, \"toYear\": 9999, \"rule\": \"US\" } ] },"
    "  { \"name\": \"Europe/Amsterdam\", \"standardOffsetMinutes\": 60, \"periods\": [ { \"fromYear\": 1996, \"toYear\": 9999, \"rule\": \"EU\" } ] } ] }";

bool
LoadRules( CORE::CTimeZoneRules& rules )
{
    CORE::CString error;
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( (UInt32) strlen( g_rulesJson ), g_rulesJson );
    CORE::CDynamicBufferAccess access( buffer );
    if ( rules.LoadFromIOAccess( access, "json", error ) )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TimeDisplay test: the rules cannot be loaded: " << error );
    return false;
}

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

/** Sets the TZ variable of the process for a scope and puts back what it was */
class CTimeZoneScope
{
    public:

    explicit CTimeZoneScope( const char* zone )
        : m_hadValue( false ), m_previous()
    {
        const char* previous = getenv( "TZ" );
        if ( GUCEF_NULL != previous )
        {
            m_hadValue = true;
            m_previous = previous;
        }
        setenv( "TZ", zone, 1 );
        tzset();
    }

    ~CTimeZoneScope()
    {
        if ( m_hadValue )
            setenv( "TZ", m_previous.C_String(), 1 );
        else
            unsetenv( "TZ" );
        tzset();
    }

    private:

    bool m_hadValue;
    CORE::CString m_previous;
};

#endif /* TZ follows the variable ? */

DRNUTILSGUI::CTimeDisplaySettings
MakeSettings( const char* zone, const CORE::CTimeZoneRules* rules )
{
    DRNUTILSGUI::CTimeDisplaySettings settings;
    settings.SetDisplayTimeZone( zone );
    settings.SetTimeZoneRules( rules );
    return settings;
}

/** The ticks of a timestamp cell, as the table holds it */
bool
TicksOf( DRNUTILSGUI::CTableGridBinding& binding, UInt32 row, const CORE::CString& column, Int64& ticks )
{
    CORE::CVariant value;
    return binding.GetCellValue( row, column, value ) && CORE::CDataTableValues::TicksFromVariant( value, CORE::DATATABLE_TIMEUNIT_MILLISECONDS, ticks );
}

/** The job ids in the order the grid has them */
CORE::CString
OrderOf( DRNUTILSGUI::CTableGridBinding& binding )
{
    CORE::CString order;
    for ( UInt32 row=0; row<binding.GetRowCount(); ++row )
    {
        CORE::CString key;
        binding.GetCellText( row, "jobId", key );
        if ( row > 0 )
            order += ',';
        order += key;
    }
    return order;
}

CORE::CString
TextOfJob( DRNUTILSGUI::CTableGridBinding& binding, const CORE::CString& jobId, const CORE::CString& column )
{
    for ( UInt32 row=0; row<binding.GetRowCount(); ++row )
    {
        CORE::CString key, text;
        binding.GetCellText( row, "jobId", key );
        if ( key == jobId && binding.GetCellText( row, column, text ) )
            return text;
    }
    return "(no such row)";
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformTimeDisplayTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsGui.TimeDisplay" );

    GUCEF_TESTFW_TESTCASE( "The settings: local zone with a suffix by default (Q18), saved and loaded" )
    try
    {
        DRNUTILSGUI::CTimeDisplaySettings settings;
        ASSERT_TRUE( "local" == settings.GetDisplayTimeZone() && settings.GetShowZoneSuffix() && GUCEF_NULL == settings.GetTimeZoneRules() );

        settings.SetDisplayTimeZone( "America/New_York" );
        settings.SetShowZoneSuffix( false );
        settings.SetDateTimeFormat( "%d/%m/%Y %H:%M" );
        CORE::CDataNode tree;
        ASSERT_TRUE( settings.SaveConfig( tree ) );

        DRNUTILSGUI::CTimeDisplaySettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) );
        ASSERT_TRUE( "America/New_York" == loaded.GetDisplayTimeZone() && !loaded.GetShowZoneSuffix() && "%d/%m/%Y %H:%M" == loaded.GetDateTimeFormat() );

        // a tree without the section is not the settings
        CORE::CDataNode other;
        DRNUTILSGUI::CTimeDisplaySettings untouched;
        ASSERT_TRUE( !untouched.LoadConfig( other ) && "local" == untouched.GetDisplayTimeZone() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UI-T4: the formatter in UTC and in a named zone across the daylight saving changes of 2026" )
    try
    {
        ASSERT_TRUE( LoadTestPlugin( "dstorepluginJSONPARSER", "dstore" ) || true );
        CORE::CTimeZoneRules rules;
        ASSERT_TRUE( LoadRules( rules ) );

        DRNUTILSGUI::CTimeDisplayFormatter utc( MakeSettings( "utc", &rules ) );
        ASSERT_TRUE( utc.IsDisplayZoneKnown() );
        ASSERT_TRUE( "2026-03-08 06:59:59 UTC" == utc.FormatUtcMs( UtcMs( 2026, 3, 8, 6, 59, 59 ) ) );
        ASSERT_TRUE( "UTC" == utc.GetZoneSuffix( UtcMs( 2026, 3, 8, 6, 59, 59 ) ) && "UTC" == utc.GetZoneDescription( 0 ) );
        ASSERT_TRUE( utc.FormatUtcMs( 0 ).IsNULLOrEmpty() );
        ASSERT_TRUE( "2026-03-08T06:59:59.250Z" == utc.FormatUtcTooltip( UtcMs( 2026, 3, 8, 6, 59, 59 ) + 250 ) );
        ASSERT_TRUE( "1969-12-31T23:59:59Z" == utc.FormatUtcTooltip( -1000 ) );

        DRNUTILSGUI::CTimeDisplaySettings plain = MakeSettings( "utc", &rules );
        plain.SetShowZoneSuffix( false );
        plain.SetDateTimeFormat( "%d/%m/%Y %H:%M:%S.%f" );
        ASSERT_TRUE( "08/03/2026 06:59:59.250" == DRNUTILSGUI::CTimeDisplayFormatter( plain ).FormatUtcMs( UtcMs( 2026, 3, 8, 6, 59, 59 ) + 250 ) );

        DRNUTILSGUI::CTimeDisplayFormatter newYork( MakeSettings( "America/New_York", &rules ) );
        ASSERT_TRUE( newYork.IsDisplayZoneKnown() );
        ASSERT_TRUE( "2026-03-08 01:59:59 UTC-05:00" == newYork.FormatUtcMs( UtcMs( 2026, 3, 8, 6, 59, 59 ) ) );
        ASSERT_TRUE( "2026-03-08 03:00:00 UTC-04:00" == newYork.FormatUtcMs( UtcMs( 2026, 3, 8, 7, 0, 0 ) ) );
        ASSERT_TRUE( "2026-11-01 01:59:59 UTC-04:00" == newYork.FormatUtcMs( UtcMs( 2026, 11, 1, 5, 59, 59 ) ) );
        ASSERT_TRUE( "2026-11-01 01:00:00 UTC-05:00" == newYork.FormatUtcMs( UtcMs( 2026, 11, 1, 6, 0, 0 ) ) );
        ASSERT_TRUE( "UTC-04:00" == newYork.GetZoneSuffix( UtcMs( 2026, 7, 1, 12, 0, 0 ) ) );
        ASSERT_TRUE( "America/New_York (UTC-05:00)" == newYork.GetZoneDescription( UtcMs( 2026, 1, 1, 12, 0, 0 ) ) );

        DRNUTILSGUI::CTimeDisplayFormatter amsterdam( MakeSettings( "Europe/Amsterdam", &rules ) );
        ASSERT_TRUE( "2026-03-29 01:59:59 UTC+01:00" == amsterdam.FormatUtcMs( UtcMs( 2026, 3, 29, 0, 59, 59 ) ) );
        ASSERT_TRUE( "2026-03-29 03:00:00 UTC+02:00" == amsterdam.FormatUtcMs( UtcMs( 2026, 3, 29, 1, 0, 0 ) ) );

        // a zone nobody has the rules for is shown as UTC and says so
        DRNUTILSGUI::CTimeDisplayFormatter unknown( MakeSettings( "Mars/Olympus", &rules ) );
        ASSERT_TRUE( !unknown.IsDisplayZoneKnown() );
        ASSERT_TRUE( "2026-03-08 06:59:59 UTC" == unknown.FormatUtcMs( UtcMs( 2026, 3, 8, 6, 59, 59 ) ) );
        ASSERT_TRUE( unknown.GetZoneDescription( 0 ).HasSubstr( "unknown zone" ) >= 0 );
        DRNUTILSGUI::CTimeDisplayFormatter noRules( MakeSettings( "America/New_York", GUCEF_NULL ) );
        ASSERT_TRUE( !noRules.IsDisplayZoneKnown() && "2026-03-08 06:59:59 UTC" == noRules.FormatUtcMs( UtcMs( 2026, 3, 8, 6, 59, 59 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

    GUCEF_TESTFW_TESTCASE( "UI-T4: the local zone of the process (TZ set by the test) gives the same texts as the named zone" )
    try
    {
        CTimeZoneScope zone( "EST5EDT,M3.2.0/2,M11.1.0/2" );
        DRNUTILSGUI::CTimeDisplayFormatter local( MakeSettings( "local", GUCEF_NULL ) );
        ASSERT_TRUE( local.IsDisplayZoneKnown() );
        ASSERT_TRUE( "2026-03-08 01:59:59 UTC-05:00" == local.FormatUtcMs( UtcMs( 2026, 3, 8, 6, 59, 59 ) ) );
        ASSERT_TRUE( "2026-03-08 03:00:00 UTC-04:00" == local.FormatUtcMs( UtcMs( 2026, 3, 8, 7, 0, 0 ) ) );
        ASSERT_TRUE( "2026-11-01 01:59:59 UTC-04:00" == local.FormatUtcMs( UtcMs( 2026, 11, 1, 5, 59, 59 ) ) );
        ASSERT_TRUE( "2026-11-01 01:00:00 UTC-05:00" == local.FormatUtcMs( UtcMs( 2026, 11, 1, 6, 0, 0 ) ) );
        ASSERT_TRUE( "local (UTC-04:00)" == local.GetZoneDescription( UtcMs( 2026, 7, 1, 12, 0, 0 ) ) );

        // the zone changes while the process runs: the formatter follows
        setenv( "TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1 );
        ASSERT_TRUE( "2026-07-01 14:00:00 UTC+02:00" == local.FormatUtcMs( UtcMs( 2026, 7, 1, 12, 0, 0 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #endif /* TZ follows the variable ? */

    GUCEF_TESTFW_TESTCASE( "UI-T4: a local time typed by the operator becomes UTC; a time that does not exist or occurs twice is refused" )
    try
    {
        CORE::CTimeZoneRules rules;
        ASSERT_TRUE( LoadRules( rules ) );
        Int64 utcMs = 0;
        CORE::CString error;

        DRNUTILSGUI::CTimeDisplayFormatter utc( MakeSettings( "utc", &rules ) );
        ASSERT_TRUE( utc.ParseDisplayTime( "2026-03-08 02:30", utcMs, error ) && UtcMs( 2026, 3, 8, 2, 30, 0 ) == utcMs );
        ASSERT_TRUE( utc.ParseDisplayTime( "2026-03-08T02:30:15.250", utcMs, error ) && UtcMs( 2026, 3, 8, 2, 30, 15 ) + 250 == utcMs );
        ASSERT_TRUE( !utc.ParseDisplayTime( "yesterday", utcMs, error ) && error.HasSubstr( "YYYY-MM-DD" ) >= 0 );
        ASSERT_TRUE( !utc.ParseDisplayTime( "2026-02-30 10:00", utcMs, error ) && error.HasSubstr( "not a valid" ) >= 0 );
        ASSERT_TRUE( !utc.ParseDisplayTime( "2026-03-08 02:30:15.25", utcMs, error ) && error.HasSubstr( "three digits" ) >= 0 );
        ASSERT_TRUE( !utc.ParseDisplayTime( "2026-03-08 02:30 and more", utcMs, error ) );

        DRNUTILSGUI::CTimeDisplayFormatter newYork( MakeSettings( "America/New_York", &rules ) );
        ASSERT_TRUE( newYork.ParseDisplayTime( "2026-11-01 00:30", utcMs, error ) && UtcMs( 2026, 11, 1, 4, 30, 0 ) == utcMs );
        ASSERT_TRUE( newYork.ParseDisplayTime( "2026-03-08 03:30:00.250", utcMs, error ) && UtcMs( 2026, 3, 8, 7, 30, 0 ) + 250 == utcMs );
        ASSERT_TRUE( newYork.ParseDisplayTime( "2026-01-15 09:00", utcMs, error ) && UtcMs( 2026, 1, 15, 14, 0, 0 ) == utcMs );
        ASSERT_TRUE( !newYork.ParseDisplayTime( "2026-03-08 02:30", utcMs, error ) && error.HasSubstr( "does not exist" ) >= 0 );
        ASSERT_TRUE( !newYork.ParseDisplayTime( "2026-11-01 01:30", utcMs, error ) && error.HasSubstr( "occurs twice" ) >= 0 );

        DRNUTILSGUI::CTimeDisplayFormatter unknown( MakeSettings( "Mars/Olympus", &rules ) );
        ASSERT_TRUE( !unknown.ParseDisplayTime( "2026-03-08 03:30", utcMs, error ) && error.HasSubstr( "not known" ) >= 0 );

        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
        {
            CTimeZoneScope zone( "EST5EDT,M3.2.0/2,M11.1.0/2" );
            DRNUTILSGUI::CTimeDisplayFormatter local( MakeSettings( "local", GUCEF_NULL ) );
            ASSERT_TRUE( local.ParseDisplayTime( "2026-11-01 00:30", utcMs, error ) && UtcMs( 2026, 11, 1, 4, 30, 0 ) == utcMs );
            ASSERT_TRUE( local.ParseDisplayTime( "2026-03-08 03:30:00.250", utcMs, error ) && UtcMs( 2026, 3, 8, 7, 30, 0 ) + 250 == utcMs );
            ASSERT_TRUE( !local.ParseDisplayTime( "2026-03-08 02:30", utcMs, error ) && error.HasSubstr( "does not exist" ) >= 0 );
            ASSERT_TRUE( !local.ParseDisplayTime( "2026-11-01 01:30", utcMs, error ) && error.HasSubstr( "occurs twice" ) >= 0 );
            // the edges of the changes
            ASSERT_TRUE( local.ParseDisplayTime( "2026-03-08 01:59:59", utcMs, error ) && UtcMs( 2026, 3, 8, 6, 59, 59 ) == utcMs );
            ASSERT_TRUE( local.ParseDisplayTime( "2026-03-08 03:00:00", utcMs, error ) && UtcMs( 2026, 3, 8, 7, 0, 0 ) == utcMs );
            ASSERT_TRUE( local.ParseDisplayTime( "2026-11-01 02:00:00", utcMs, error ) && UtcMs( 2026, 11, 1, 7, 0, 0 ) == utcMs );
        }
        #endif /* TZ follows the variable ? */
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UI-T4: the same UTC table in UTC, a named zone and the local zone: only the texts change, not the values, the order or the filters" )
    try
    {
        CORE::CTimeZoneRules rules;
        ASSERT_TRUE( LoadRules( rules ) );
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        STestSchedulerTables tables;
        ASSERT_TRUE( GUCEF_NULL != context && tables.Fill() );
        if ( GUCEF_NULL != context )
        {
            DRNUTILSGUI::CSchedulerForm form( context );
            form.SetTables( tables.GetSet() );
            form.SetTimeDisplay( MakeSettings( "utc", &rules ) );
            ASSERT_TRUE( form.Open() );
            DRNUTILSGUI::CTableGridBinding& jobs = form.GetJobsBinding();

            // job-1 is due 2026-03-08 06:59:59Z, job-2 at 07:00:00Z, job-3 2026-11-01 06:00:00Z
            const Int64 due1 = UtcMs( 2026, 3, 8, 6, 59, 59 ), due2 = UtcMs( 2026, 3, 8, 7, 0, 0 ), due3 = UtcMs( 2026, 11, 1, 6, 0, 0 );
            ASSERT_TRUE( "2026-03-08 06:59:59 UTC" == TextOfJob( jobs, "job-1", "nextDueUtc" ) );

            // sort order and the filter, in UTC
            ASSERT_TRUE( jobs.SortBy( "nextDueUtc", true ) && "job-1,job-2,job-3" == OrderOf( jobs ) );
            ASSERT_TRUE( jobs.SortBy( "nextDueUtc", false ) && "job-3,job-2,job-1" == OrderOf( jobs ) );
            CORE::CDataTableQuery::TPredicateVector later;
            later.push_back( CORE::CDataTablePredicate( "nextDueUtc", CORE::DATATABLE_PREDICATE_GREATER_OR_EQUAL, CORE::CDataTableValues::Timestamp( due2, CORE::DATATABLE_TIMEUNIT_MILLISECONDS ) ) );
            ASSERT_TRUE( jobs.SetFilter( later ) && 2 == jobs.GetRowCount() );
            ASSERT_TRUE( jobs.SetFilter( CORE::CDataTableQuery::TPredicateVector() ) && 3 == jobs.GetRowCount() );

            // a named zone: the texts have the zone and its offset at that instant, nothing else changes
            form.SetTimeDisplay( MakeSettings( "America/New_York", &rules ) );
            ASSERT_TRUE( "2026-03-08 01:59:59 UTC-05:00" == TextOfJob( jobs, "job-1", "nextDueUtc" ) );
            ASSERT_TRUE( "2026-03-08 03:00:00 UTC-04:00" == TextOfJob( jobs, "job-2", "nextDueUtc" ) );
            ASSERT_TRUE( "2026-11-01 01:00:00 UTC-05:00" == TextOfJob( jobs, "job-3", "nextDueUtc" ) );
            Int64 ticks = 0;
            for ( UInt32 row=0; row<jobs.GetRowCount(); ++row )
            {
                CORE::CString key;
                jobs.GetCellText( row, "jobId", key );
                ASSERT_TRUE( TicksOf( jobs, row, "nextDueUtc", ticks ) );
                ASSERT_TRUE( ticks == ( "job-1" == key ? due1 : ( "job-2" == key ? due2 : due3 ) ) );
            }
            ASSERT_TRUE( jobs.SortBy( "nextDueUtc", true ) && "job-1,job-2,job-3" == OrderOf( jobs ) );
            ASSERT_TRUE( jobs.SetFilter( later ) && 2 == jobs.GetRowCount() );
            jobs.SetFilter( CORE::CDataTableQuery::TPredicateVector() );

            // the model gives the UTC text for the tooltip of a cell
            const DRNUTILSGUI::CTimeDisplayTableModel* model = jobs.GetModel();
            ASSERT_TRUE( GUCEF_NULL != model );
            if ( GUCEF_NULL != model )
            {
                const UInt32 column = model->FindShownColumn( "nextDueUtc" );
                ASSERT_TRUE( GUCEF_UINT32MAX != column && "2026-03-08T06:59:59Z" == model->GetCellUtcText( 0, column ) );
                ASSERT_TRUE( model->GetCellUtcText( 0, 0 ).IsNULLOrEmpty() );
            }

            // the status line of the form names the zone
            ASSERT_TRUE( form.GetStatusText().HasSubstr( "times in America/New_York (UTC" ) >= 0 );

            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
            {
                // the local zone of the machine: the process zone set by the test
                CTimeZoneScope zone( "EST5EDT,M3.2.0/2,M11.1.0/2" );
                form.SetTimeDisplay( MakeSettings( "local", GUCEF_NULL ) );
                ASSERT_TRUE( "2026-03-08 01:59:59 UTC-05:00" == TextOfJob( jobs, "job-1", "nextDueUtc" ) );
                ASSERT_TRUE( "2026-03-08 03:00:00 UTC-04:00" == TextOfJob( jobs, "job-2", "nextDueUtc" ) );
                ASSERT_TRUE( "2026-11-01 01:00:00 UTC-05:00" == TextOfJob( jobs, "job-3", "nextDueUtc" ) );
                ASSERT_TRUE( jobs.SortBy( "nextDueUtc", false ) && "job-3,job-2,job-1" == OrderOf( jobs ) );
                ASSERT_TRUE( jobs.SetFilter( later ) && 2 == jobs.GetRowCount() );
                jobs.SetFilter( CORE::CDataTableQuery::TPredicateVector() );
                ASSERT_TRUE( TicksOf( jobs, 0, "nextDueUtc", ticks ) );
                ASSERT_TRUE( form.GetStatusText().HasSubstr( "times in local (UTC" ) >= 0 );

                // another zone for the same process: the grid follows when the settings are given again
                setenv( "TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1 );
                form.SetTimeDisplay( MakeSettings( "local", GUCEF_NULL ) );
                ASSERT_TRUE( "2026-03-08 07:59:59 UTC+01:00" == TextOfJob( jobs, "job-1", "nextDueUtc" ) );
            }
            #endif /* TZ follows the variable ? */

            // and back to UTC: the original texts, the values never changed
            form.SetTimeDisplay( MakeSettings( "utc", &rules ) );
            ASSERT_TRUE( "2026-03-08 06:59:59 UTC" == TextOfJob( jobs, "job-1", "nextDueUtc" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
