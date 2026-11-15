/*
 *  gucefCORE_TestApp: GUCEF test application
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
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <signal.h>

#include <sys/stat.h>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CIUTCCLOCK_H
#include "gucefCORE_CIUtcClock.h"
#define GUCEF_CORE_CIUTCCLOCK_H
#endif /* GUCEF_CORE_CIUTCCLOCK_H ? */

#ifndef GUCEF_CORE_CMONTHLYJSONLINESFILE_H
#include "gucefCORE_CMonthlyJsonLinesFile.h"
#define GUCEF_CORE_CMONTHLYJSONLINESFILE_H
#endif /* GUCEF_CORE_CMONTHLYJSONLINESFILE_H ? */

#ifndef GUCEF_CORE_CSERVICELIFECYCLE_H
#include "gucefCORE_CServiceLifecycle.h"
#define GUCEF_CORE_CSERVICELIFECYCLE_H
#endif /* GUCEF_CORE_CSERVICELIFECYCLE_H ? */

#ifndef GUCEF_CORE_CSERVICESTOPSIGNALS_H
#include "gucefCORE_CServiceStopSignals.h"
#define GUCEF_CORE_CSERVICESTOPSIGNALS_H
#endif /* GUCEF_CORE_CSERVICESTOPSIGNALS_H ? */

#ifndef GUCEF_CORE_CSERVICERESOURCEWATCH_H
#include "gucefCORE_CServiceResourceWatch.h"
#define GUCEF_CORE_CSERVICERESOURCEWATCH_H
#endif /* GUCEF_CORE_CSERVICERESOURCEWATCH_H ? */

#ifndef GUCEF_CORE_CISERVICERESOURCESOURCE_H
#include "gucefCORE_CIServiceResourceSource.h"
#define GUCEF_CORE_CISERVICERESOURCESOURCE_H
#endif /* GUCEF_CORE_CISERVICERESOURCESOURCE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestServiceProcessKit.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A wall clock the test sets */
class CKitTestUtcClock : public CIUtcClock
{
    public:
    Int64 nowMs;
    CKitTestUtcClock( void ) : CIUtcClock(), nowMs( 0 ) {}
    virtual Int64 GetUtcTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE { return nowMs; }
};

/** UTC milliseconds of the start of a day */
Int64
KitUtcMs( Int32 year, Int32 month, Int32 day )
{GUCEF_TRACE;

    // days from 1970-01-01 of a civil date (proleptic Gregorian)
    const Int32 y = month <= 2 ? year - 1 : year;
    const Int32 era = ( y >= 0 ? y : y - 399 ) / 400;
    const Int32 yearOfEra = y - era * 400;
    const Int32 dayOfYear = ( 153 * ( month + ( month > 2 ? -3 : 9 ) ) + 2 ) / 5 + day - 1;
    const Int32 dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
    const Int64 days = (Int64) era * 146097 + dayOfEra - 719468;
    return days * 86400000 + 12 * 3600000;
}

/** A fresh directory below the work directory */
CString
KitTestDirectory( const char* name )
{GUCEF_TRACE;

    CString path = RelativePath( "$CURWORKDIR$" );
    AppendToPath( path, "ServiceProcessKitTest" );
    AppendToPath( path, name );
    if ( DirExists( path ) )
        DeleteDir( path, true );
    CreateDirs( path );
    return path;
}

bool
KitWriteFile( const CString& path, const char* content )
{GUCEF_TRACE;

    CDynamicBuffer buffer;
    buffer.CopyFrom( (UInt32) strlen( content ), content );
    return buffer.WriteContentToFile( path, 0, true );
}

CString
KitReadFile( const CString& path )
{GUCEF_TRACE;

    CDynamicBuffer buffer;
    if ( !buffer.LoadContentFromFile( path ) )
        return CString::Empty;
    CString text;
    text.Set( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
    return text;
}

/** Counts the events of a notifier */
class CKitEventCounter : public CObserver
{
    public:
    UInt32 stateChanges;
    UInt32 conditionChanges;
    UInt32 stopRequests;
    Int32 lastSignal;
    SServiceLifecycleChange lastChange;

    CKitEventCounter( void ) : CObserver(), stateChanges( 0 ), conditionChanges( 0 ), stopRequests( 0 ), lastSignal( 0 ), lastChange() {}
    virtual ~CKitEventCounter() { SignalUpcomingObserverDestruction(); }

    protected:

    virtual void OnNotify( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CServiceLifecycle::StateChangedEvent == eventId )
        {
            ++stateChanges;
            if ( GUCEF_NULL != eventData )
                lastChange = static_cast< TServiceLifecycleChangeEventData* >( eventData )->GetData();
        }
        else
        if ( CServiceLifecycle::ConditionsChangedEvent == eventId )
        {
            ++conditionChanges;
        }
        else
        if ( CServiceStopSignals::StopRequestedEvent == eventId )
        {
            ++stopRequests;
            if ( GUCEF_NULL != eventData )
                lastSignal = static_cast< TCloneableInt32* >( eventData )->GetData();
        }
    }
};

/** A resource watch whose process values the test sets */
class CKitTestResourceWatch : public CServiceResourceWatch
{
    public:
    UInt64 fakeRssBytes;
    CKitTestResourceWatch( void ) : CServiceResourceWatch(), fakeRssBytes( 0 ) {}

    protected:

    virtual bool SampleProcess( SServiceProcessResources& resources ) GUCEF_VIRTUAL_OVERRIDE
    {
        resources.rssBytes = fakeRssBytes;
        resources.hwmBytes = fakeRssBytes;
        resources.threadCount = 3;
        resources.openDescriptors = 7;
        return true;
    }
};

/** A source of counters */
class CKitTestSource : public CIServiceResourceSource
{
    public:
    UInt32 sessions;
    CKitTestSource( void ) : CIServiceResourceSource(), sessions( 3 ) {}
    virtual CString GetResourceSourceName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "test"; }
    virtual void GetResourceCounters( CValueList& counters ) const GUCEF_VIRTUAL_OVERRIDE
    {
        counters.Set( "sessions", CVariant( sessions ) );
    }
};

const UInt64 KitMB = 1024 * 1024;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformMonthlyJsonLinesFileTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: MonthOfFileName takes real months of yyyy-mm.jsonl only" )
        try
        {
            ASSERT_TRUE( "2026-11" == CMonthlyJsonLinesFile::MonthOfFileName( "2026-11.jsonl" ) );
            ASSERT_TRUE( "1970-01" == CMonthlyJsonLinesFile::MonthOfFileName( "1970-01.jsonl" ) );
            ASSERT_TRUE( CMonthlyJsonLinesFile::MonthOfFileName( "2026-13.jsonl" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CMonthlyJsonLinesFile::MonthOfFileName( "2026-00.jsonl" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CMonthlyJsonLinesFile::MonthOfFileName( "1969-12.jsonl" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CMonthlyJsonLinesFile::MonthOfFileName( "2026-11.json" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CMonthlyJsonLinesFile::MonthOfFileName( "2026_11.jsonl" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CMonthlyJsonLinesFile::MonthOfFileName( "x026-11.jsonl" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CMonthlyJsonLinesFile::MonthOfFileName( "2026-11.jsonl.prev" ).IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: lines go into the file of the clock's month, a new month starts a new file, a clock set back stays in the newest" )
        try
        {
            const CString dir = KitTestDirectory( "months" );
            CKitTestUtcClock clock;
            clock.nowMs = KitUtcMs( 2026, 1, 31 );
            CMonthlyJsonLinesFile file;
            file.SetUtcClock( &clock );
            ASSERT_TRUE( file.SetDirectory( dir ) );
            CString error;
            ASSERT_TRUE( file.Append( "{\"n\":1}", error ) );
            ASSERT_TRUE( file.Append( "{\"n\":2}", error ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "2026-01.jsonl" ) ) );
            ASSERT_TRUE( "{\"n\":1}\n{\"n\":2}\n" == KitReadFile( CombinePath( dir, "2026-01.jsonl" ) ) );

            clock.nowMs = KitUtcMs( 2026, 2, 1 );
            ASSERT_TRUE( CombinePath( dir, "2026-02.jsonl" ) == file.GetCurrentFilePath() );
            ASSERT_TRUE( file.Append( "{\"n\":3}", error ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "2026-02.jsonl" ) ) );

            // the clock goes back a month: the line goes on into the newest file
            clock.nowMs = KitUtcMs( 2026, 1, 20 );
            ASSERT_TRUE( file.Append( "{\"n\":4}", error ) );
            ASSERT_TRUE( "{\"n\":3}\n{\"n\":4}\n" == KitReadFile( CombinePath( dir, "2026-02.jsonl" ) ) );

            // a year boundary of the civil calendar
            clock.nowMs = KitUtcMs( 2027, 12, 31 );
            ASSERT_TRUE( file.Append( "{\"n\":5}", error ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "2027-12.jsonl" ) ) );

            CString::StringVector lines;
            ASSERT_TRUE( file.ReadLines( 0, lines ) );
            ASSERT_TRUE( 5 == lines.size() );
            if ( 5 == lines.size() )
            {
                ASSERT_TRUE( "{\"n\":1}" == lines[ 0 ] );
                ASSERT_TRUE( "{\"n\":5}" == lines[ 4 ] );
            }
            ASSERT_TRUE( file.ReadLines( 1, lines ) );
            ASSERT_TRUE( 1 == lines.size() );

            CString::StringVector files;
            file.GetMonthFiles( files );
            ASSERT_TRUE( 3 == files.size() );

            // a line break inside a line is refused
            ASSERT_FALSE( file.Append( "a\nb", error ) );

            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
            struct stat info;
            ASSERT_TRUE( 0 == stat( CombinePath( dir, "2026-01.jsonl" ).C_String(), &info ) );
            ASSERT_TRUE( 0600 == ( info.st_mode & 0777 ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: SetDirectory cuts off a fragment at the end of the newest file and keeps the lines before it" )
        try
        {
            const CString dir = KitTestDirectory( "repair" );
            ASSERT_TRUE( KitWriteFile( CombinePath( dir, "2026-03.jsonl" ), "{\"a\":1}\n" ) );
            ASSERT_TRUE( KitWriteFile( CombinePath( dir, "2026-04.jsonl" ), "{\"b\":1}\n{\"b\":2}\n{\"b\":3" ) );
            CKitTestUtcClock clock;
            clock.nowMs = KitUtcMs( 2026, 4, 10 );
            CMonthlyJsonLinesFile file;
            file.SetUtcClock( &clock );
            ASSERT_TRUE( file.SetDirectory( dir ) );
            ASSERT_TRUE( 6 == file.GetRepairedBytesAtOpen() );
            ASSERT_TRUE( "{\"b\":2}" == file.GetLastLineAtOpen() );
            ASSERT_TRUE( "{\"b\":1}\n{\"b\":2}\n" == KitReadFile( CombinePath( dir, "2026-04.jsonl" ) ) );

            CString error;
            ASSERT_TRUE( file.Append( "{\"b\":4}", error ) );
            ASSERT_TRUE( "{\"b\":1}\n{\"b\":2}\n{\"b\":4}\n" == KitReadFile( CombinePath( dir, "2026-04.jsonl" ) ) );

            // a fragment that appears later is skipped by ReadLines()
            CDynamicBuffer tail;
            tail.CopyFrom( 5, "{\"b\":" );
            CFileAccess raw( CombinePath( dir, "2026-04.jsonl" ), "ab" );
            raw.Write( tail.GetConstBufferPtr(), 1, tail.GetDataSize() );
            raw.Close();
            CString::StringVector lines;
            ASSERT_TRUE( file.ReadLines( 0, lines ) );
            ASSERT_TRUE( 4 == lines.size() );
            if ( 4 == lines.size() )
                ASSERT_TRUE( "{\"b\":4}" == lines[ 3 ] );

            // a whole file has nothing to repair
            CMonthlyJsonLinesFile clean;
            const CString cleanDir = KitTestDirectory( "repairclean" );
            ASSERT_TRUE( KitWriteFile( CombinePath( cleanDir, "2026-05.jsonl" ), "{\"c\":1}\n" ) );
            ASSERT_TRUE( clean.SetDirectory( cleanDir ) );
            ASSERT_TRUE( 0 == clean.GetRepairedBytesAtOpen() );
            ASSERT_TRUE( "{\"c\":1}" == clean.GetLastLineAtOpen() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: retention keeps the newest months by their names, also against a clock set far ahead or back" )
        try
        {
            const CString dir = KitTestDirectory( "retention" );
            const char* names[ 5 ] = { "2025-10.jsonl", "2025-11.jsonl", "2025-12.jsonl", "2026-01.jsonl", "2026-02.jsonl" };
            for ( UInt32 i=0; i<5; ++i )
                ASSERT_TRUE( KitWriteFile( CombinePath( dir, names[ i ] ), "{}\n" ) );
            ASSERT_TRUE( KitWriteFile( CombinePath( dir, "notes.txt" ), "other\n" ) );

            CKitTestUtcClock clock;
            clock.nowMs = KitUtcMs( 2030, 6, 1 );
            CMonthlyJsonLinesFile file;
            file.SetUtcClock( &clock );
            file.SetKeepMonths( 3 );
            ASSERT_TRUE( file.SetDirectory( dir ) );
            ASSERT_FALSE( FileExists( CombinePath( dir, "2025-10.jsonl" ) ) );
            ASSERT_FALSE( FileExists( CombinePath( dir, "2025-11.jsonl" ) ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "2025-12.jsonl" ) ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "2026-02.jsonl" ) ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "notes.txt" ) ) );

            // a new month removes one more
            CString error;
            ASSERT_TRUE( file.Append( "{}", error ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "2030-06.jsonl" ) ) );
            ASSERT_FALSE( FileExists( CombinePath( dir, "2025-12.jsonl" ) ) );
            ASSERT_TRUE( 0 == file.ApplyRetention() );

            // keep 0 keeps everything
            CMonthlyJsonLinesFile all;
            all.SetKeepMonths( 0 );
            ASSERT_TRUE( all.SetDirectory( dir ) );
            ASSERT_TRUE( 0 == all.ApplyRetention() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: an append that cannot be made fails, is counted, and leaves nothing behind" )
        try
        {
            const CString dir = KitTestDirectory( "refused" );
            CKitTestUtcClock clock;
            clock.nowMs = KitUtcMs( 2026, 7, 4 );

            // something that is not a file stands where the month file goes
            ASSERT_TRUE( CreateDirs( CombinePath( dir, "2026-07.jsonl" ) ) );
            CMonthlyJsonLinesFile file;
            file.SetUtcClock( &clock );
            ASSERT_TRUE( file.SetDirectory( dir ) );
            CString error;
            ASSERT_FALSE( file.Append( "{\"x\":1}", error ) );
            ASSERT_FALSE( error.IsNULLOrEmpty() );
            ASSERT_TRUE( 1 == file.GetFailedAppendCount() );

            CMonthlyJsonLinesFile noDirectory;
            ASSERT_FALSE( noDirectory.Append( "{}", error ) );
            ASSERT_FALSE( noDirectory.SetDirectory( CString::Empty ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformServiceLifecycleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: the transition table of the service states, every pair" )
        try
        {
            // rows: from, columns: to, in the order of the enumeration
            static const char* table[ 9 ] =
            {
                "011000101",    // STARTING
                "001000101",    // LOCK_WAIT
                "010110101",    // LOADING
                "000011101",    // READY
                "000101101",    // DEGRADED
                "000000101",    // READ_ONLY
                "000000011",    // DRAINING
                "000000000",    // STOPPED
                "000000000"     // FAILED
            };
            for ( UInt32 from=0; from<9; ++from )
            {
                for ( UInt32 to=0; to<9; ++to )
                {
                    const bool expected = '1' == table[ from ][ to ];
                    const bool allowed = CServiceLifecycle::IsTransitionAllowed( (TServiceLifecycleState) from, (TServiceLifecycleState) to );
                    if ( expected != allowed )
                    {
                        GUCEF_ERROR_LOG( LOGLEVEL_CRITICAL, "ServiceLifecycle test: " << ServiceLifecycleStateToString( (TServiceLifecycleState) from )
                                         << " -> " << ServiceLifecycleStateToString( (TServiceLifecycleState) to ) << " expected " << expected );
                        ERRORHERE;
                    }
                }
                TServiceLifecycleState parsed = SERVICELIFECYCLE_FAILED;
                ASSERT_TRUE( ServiceLifecycleStateFromString( ServiceLifecycleStateToString( (TServiceLifecycleState) from ), parsed ) );
                ASSERT_TRUE( (UInt32) parsed == from );
            }
            TServiceLifecycleState parsed = SERVICELIFECYCLE_READY;
            ASSERT_FALSE( ServiceLifecycleStateFromString( "RUNNING", parsed ) );
            ASSERT_TRUE( ServiceLifecycleStateFromString( "draining", parsed ) && SERVICELIFECYCLE_DRAINING == parsed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: conditions move a serving service between READY and DEGRADED, one event per change" )
        try
        {
            CServiceLifecycle lifecycle;
            CKitEventCounter counter;
            lifecycle.Subscribe( &counter );
            ASSERT_TRUE( SERVICELIFECYCLE_STARTING == lifecycle.GetState() );
            ASSERT_FALSE( lifecycle.IsServing() );
            ASSERT_TRUE( lifecycle.Transition( SERVICELIFECYCLE_LOADING, "lock taken" ) );
            ASSERT_TRUE( 1 == counter.stateChanges );

            // a condition before READY: READY becomes DEGRADED
            lifecycle.SetCondition( "DATABASE_UNAVAILABLE", "the database does not answer" );
            ASSERT_TRUE( 1 == counter.conditionChanges );
            ASSERT_TRUE( SERVICELIFECYCLE_LOADING == lifecycle.GetState() );
            ASSERT_TRUE( lifecycle.Transition( SERVICELIFECYCLE_READY ) );
            ASSERT_TRUE( SERVICELIFECYCLE_DEGRADED == lifecycle.GetState() );
            ASSERT_TRUE( lifecycle.IsServing() );
            ASSERT_TRUE( 2 == counter.stateChanges );

            // a second condition changes no state; clearing one of two neither
            lifecycle.SetCondition( "LOCATION_OFFLINE", "main" );
            ASSERT_TRUE( 2 == counter.stateChanges );
            ASSERT_TRUE( 2 == lifecycle.GetConditions().size() );
            ASSERT_TRUE( lifecycle.ClearCondition( "DATABASE_UNAVAILABLE" ) );
            ASSERT_TRUE( SERVICELIFECYCLE_DEGRADED == lifecycle.GetState() );
            ASSERT_FALSE( lifecycle.ClearCondition( "DATABASE_UNAVAILABLE" ) );

            // the last one cleared: READY
            ASSERT_TRUE( lifecycle.ClearCondition( "LOCATION_OFFLINE" ) );
            ASSERT_TRUE( SERVICELIFECYCLE_READY == lifecycle.GetState() );
            ASSERT_TRUE( 3 == counter.stateChanges );
            ASSERT_TRUE( SERVICELIFECYCLE_DEGRADED == counter.lastChange.from && SERVICELIFECYCLE_READY == counter.lastChange.to );

            // while READY a condition makes it DEGRADED at once
            lifecycle.SetCondition( "DISK", "90% used" );
            ASSERT_TRUE( SERVICELIFECYCLE_DEGRADED == lifecycle.GetState() );
            ASSERT_TRUE( 4 == counter.stateChanges );

            // the same condition again with the same text is no change
            const UInt32 conditionEvents = counter.conditionChanges;
            lifecycle.SetCondition( "DISK", "90% used" );
            ASSERT_TRUE( conditionEvents == counter.conditionChanges );

            // a refused change changes nothing and raises nothing
            ASSERT_FALSE( lifecycle.Transition( SERVICELIFECYCLE_LOADING ) );
            ASSERT_TRUE( 4 == counter.stateChanges );
            ASSERT_TRUE( SERVICELIFECYCLE_DEGRADED == lifecycle.GetState() );
            lifecycle.Unsubscribe( &counter );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: drain with a deadline, then STOPPED; final states take nothing more" )
        try
        {
            CServiceLifecycle lifecycle;
            ASSERT_TRUE( lifecycle.Transition( SERVICELIFECYCLE_LOADING ) );
            ASSERT_TRUE( lifecycle.Transition( SERVICELIFECYCLE_READY ) );
            ASSERT_TRUE( 0 == lifecycle.GetDrainRemainingMs() );
            ASSERT_FALSE( lifecycle.IsDrainExpired() );
            ASSERT_TRUE( lifecycle.BeginDrain( "SIGTERM", 200 ) );
            ASSERT_TRUE( SERVICELIFECYCLE_DRAINING == lifecycle.GetState() );
            ASSERT_TRUE( "SIGTERM" == lifecycle.GetReason() );
            ASSERT_TRUE( lifecycle.GetDrainRemainingMs() > 0 && lifecycle.GetDrainRemainingMs() <= 200 );
            ASSERT_FALSE( lifecycle.IsDrainExpired() );
            ASSERT_FALSE( lifecycle.BeginDrain( "again", 100 ) );
            MT::PrecisionDelay( 250 );
            ASSERT_TRUE( lifecycle.IsDrainExpired() );
            ASSERT_TRUE( 0 == lifecycle.GetDrainRemainingMs() );
            ASSERT_TRUE( lifecycle.Transition( SERVICELIFECYCLE_STOPPED, "drained" ) );
            ASSERT_TRUE( lifecycle.IsFinal() );
            ASSERT_FALSE( lifecycle.Fail( "too late" ) );
            ASSERT_TRUE( SERVICELIFECYCLE_STOPPED == lifecycle.GetState() );
            ASSERT_TRUE( lifecycle.GetUptimeInMs() >= lifecycle.GetStateAgeInMs() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Fail ends in FAILED with its reason from any state that is not final" )
        try
        {
            CServiceLifecycle lifecycle;
            CKitEventCounter counter;
            lifecycle.Subscribe( &counter );
            ASSERT_TRUE( lifecycle.Fail( "the data directory is not writable" ) );
            ASSERT_TRUE( SERVICELIFECYCLE_FAILED == lifecycle.GetState() );
            ASSERT_TRUE( "the data directory is not writable" == lifecycle.GetReason() );
            ASSERT_TRUE( 1 == counter.stateChanges );
            ASSERT_FALSE( lifecycle.Transition( SERVICELIFECYCLE_DRAINING ) );
            ASSERT_TRUE( 1 == counter.stateChanges );
            lifecycle.Unsubscribe( &counter );

            CServiceLifecycle readOnly;
            ASSERT_TRUE( readOnly.Transition( SERVICELIFECYCLE_LOADING ) && readOnly.Transition( SERVICELIFECYCLE_READY ) );
            ASSERT_TRUE( readOnly.Transition( SERVICELIFECYCLE_READ_ONLY, "the writer lock was lost" ) );
            ASSERT_TRUE( readOnly.IsServing() );
            ASSERT_FALSE( readOnly.Transition( SERVICELIFECYCLE_READY ) );
            ASSERT_TRUE( readOnly.BeginDrain( "restart", 1000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformServiceStopSignalsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: a noted stop signal raises StopRequestedEvent once, at the next pulse" )
        try
        {
            CServiceStopSignals::Reset();
            PulseGeneratorPtr pulse = CCoreGlobal::Instance()->GetPulseGenerator();
            CServiceStopSignals signals( pulse );
            CKitEventCounter counter;
            signals.Subscribe( &counter );

            pulse->ForceDirectPulse();
            ASSERT_TRUE( 0 == counter.stopRequests );
            ASSERT_FALSE( signals.IsStopRequested() );

            CServiceStopSignals::NoteSignal( SIGTERM );
            ASSERT_TRUE( SIGTERM == CServiceStopSignals::GetPendingSignal() );
            pulse->ForceDirectPulse();
            ASSERT_TRUE( 1 == counter.stopRequests );
            ASSERT_TRUE( SIGTERM == counter.lastSignal );
            ASSERT_TRUE( signals.IsStopRequested() );
            pulse->ForceDirectPulse();
            pulse->ForceDirectPulse();
            ASSERT_TRUE( 1 == counter.stopRequests );

            // a signal noted before an instance existed is seen by it; 0 counts as SIGTERM
            CServiceStopSignals::Reset();
            CServiceStopSignals::NoteSignal( 0 );
            ASSERT_TRUE( SIGTERM == CServiceStopSignals::GetPendingSignal() );
            CServiceStopSignals later( pulse );
            CKitEventCounter laterCounter;
            later.Subscribe( &laterCounter );
            pulse->ForceDirectPulse();
            ASSERT_TRUE( 1 == laterCounter.stopRequests );

            later.Unsubscribe( &laterCounter );
            signals.Unsubscribe( &counter );
            CServiceStopSignals::Reset();
            ASSERT_TRUE( 0 == CServiceStopSignals::GetPendingSignal() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    GUCEF_TESTFW_TESTCASE( "Test: installed handlers note a real SIGTERM and SIGINT without stopping the application" )
        try
        {
            CServiceStopSignals::Reset();
            const bool applicationStopRequestedBefore = CGUCEFApplication::IsStopRequestedFromSignal();
            ASSERT_TRUE( CServiceStopSignals::InstallHandlers() );
            raise( SIGTERM );
            ASSERT_TRUE( SIGTERM == CServiceStopSignals::GetPendingSignal() );
            raise( SIGINT );
            ASSERT_TRUE( SIGINT == CServiceStopSignals::GetPendingSignal() );
            ASSERT_TRUE( applicationStopRequestedBefore == CGUCEFApplication::IsStopRequestedFromSignal() );
            ASSERT_TRUE( CServiceStopSignals::RestoreHandlers() );
            ASSERT_FALSE( CServiceStopSignals::RestoreHandlers() );
            CServiceStopSignals::Reset();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif
}

/*-------------------------------------------------------------------------*/

static void
PerformServiceResourceWatchTests( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    GUCEF_TESTFW_TESTCASE( "Test: the process resources are read: resident memory, its peak, threads, descriptors" )
        try
        {
            SServiceProcessResources resources;
            ASSERT_TRUE( CServiceResourceWatch::ReadProcessResources( resources ) );
            ASSERT_TRUE( resources.rssBytes > 0 );
            ASSERT_TRUE( resources.hwmBytes >= resources.rssBytes );
            ASSERT_TRUE( resources.threadCount > 0 );
            ASSERT_TRUE( resources.openDescriptors > 0 );

            CServiceResourceWatch watch;
            watch.SampleNow( 1000, false );
            ASSERT_TRUE( watch.GetLastProcessSample().rssBytes > 0 );
            ASSERT_TRUE( 0 == watch.FormatLine().HasSubstr( "ResourceWatch: rssMB=" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    GUCEF_TESTFW_TESTCASE( "Test: the counters of the sources appear in the sample, the line and the info; samples follow the interval" )
        try
        {
            CKitTestResourceWatch watch;
            CKitTestSource source;
            watch.fakeRssBytes = 40 * KitMB;
            watch.Configure( 1000, 0, 0 );
            watch.AddSource( &source );
            watch.AddSource( &source );
            watch.Tick( 5000 );
            ASSERT_TRUE( 5000 == watch.GetLastProcessSample().sampledAtMs );
            ASSERT_TRUE( 3 == watch.GetLastCounters().GetValueAlways( "test.sessions" ).AsUInt32() );
            ASSERT_TRUE( 1 == watch.GetLastCounters().GetKeyCount() );
            const CString line = watch.FormatLine();
            ASSERT_TRUE( 0 <= line.HasSubstr( "rssMB=40" ) );
            ASSERT_TRUE( 0 <= line.HasSubstr( "threads=3" ) );
            ASSERT_TRUE( 0 <= line.HasSubstr( "fds=7" ) );
            ASSERT_TRUE( 0 <= line.HasSubstr( "test.sessions=3" ) );
            CValueList info;
            watch.GetInfo( info );
            ASSERT_TRUE( 40 == info.GetValueAlways( "rssMB" ).AsUInt64() );
            ASSERT_TRUE( 3 == info.GetValueAlways( "test.sessions" ).AsUInt32() );

            // within the interval nothing is sampled, after it again
            source.sessions = 5;
            watch.Tick( 5500 );
            ASSERT_TRUE( 5000 == watch.GetLastProcessSample().sampledAtMs );
            watch.Tick( 6000 );
            ASSERT_TRUE( 6000 == watch.GetLastProcessSample().sampledAtMs );
            ASSERT_TRUE( 5 == watch.GetLastCounters().GetValueAlways( "test.sessions" ).AsUInt32() );

            watch.RemoveSource( &source );
            watch.SampleNow( 7000, true );
            ASSERT_TRUE( 0 == watch.GetLastCounters().GetKeyCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the soft memory limit holds above the limit until the memory is below 90 percent of it" )
        try
        {
            CKitTestResourceWatch watch;
            watch.Configure( 1000, 0, 100 * KitMB );
            watch.fakeRssBytes = 50 * KitMB;
            watch.SampleNow( 1000, false );
            ASSERT_FALSE( watch.IsAboveSoftLimit() );
            watch.fakeRssBytes = 120 * KitMB;
            watch.SampleNow( 2000, false );
            ASSERT_TRUE( watch.IsAboveSoftLimit() );
            watch.fakeRssBytes = 95 * KitMB;
            watch.SampleNow( 3000, false );
            ASSERT_TRUE( watch.IsAboveSoftLimit() );
            watch.fakeRssBytes = 85 * KitMB;
            watch.SampleNow( 4000, false );
            ASSERT_FALSE( watch.IsAboveSoftLimit() );

            // no limit: never above
            watch.Configure( 1000, 0, 0 );
            watch.fakeRssBytes = 500 * KitMB;
            watch.SampleNow( 5000, false );
            ASSERT_FALSE( watch.IsAboveSoftLimit() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformServiceProcessKitTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING SERVICE PROCESS KIT TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "ServiceProcessKit" );

    PerformMonthlyJsonLinesFileTests();
    PerformServiceLifecycleTests();
    PerformServiceStopSignalsTests();
    PerformServiceResourceWatchTests();
}

/*-------------------------------------------------------------------------*/
