/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
 *  Copyright (C) 2002 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#define GUCEF_TESTFW_IMPL
#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_TESTAPP_TESTMUTEX_H
#include "TestMutex.h"
#define GUCEF_MT_TESTAPP_TESTMUTEX_H
#endif /* GUCEF_MT_TESTAPP_TESTMUTEX_H ? */

#ifndef GUCEF_MT_TESTAPP_TESTREADERWRITERLOCK_H
#include "TestReaderWriterLock.h"
#define GUCEF_MT_TESTAPP_TESTREADERWRITERLOCK_H
#endif /* GUCEF_MT_TESTAPP_TESTREADERWRITERLOCK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Global hard-deadline for the entire test suite.
 *  If tests are still running after this many milliseconds, the watchdog
 *  kills the process with exit code GUCEF_MT_TESTAPP_WATCHDOG_EXIT_CODE.
 *
 *  This is the last line of defence: even if a test forgets to use a bounded
 *  Lock() timeout and gets stuck, the process will not hang CI/CD pipelines
 *  indefinitely.
 *
 *  Should be set to comfortably exceed the expected total test duration but
 *  short enough to still be useful as a hang-detector.
 */
#define GUCEF_MT_TESTAPP_WATCHDOG_TIMEOUT_IN_MS     600000  /* 10 minutes  */
#define GUCEF_MT_TESTAPP_WATCHDOG_EXIT_CODE         99      /* distinct code so CI can tell it from a normal failure */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/**
 *  Watchdog thread.
 *
 *  Started before the tests begin and cancelled when they complete normally.
 *  If the deadline expires (tests got stuck), it writes whatever partial
 *  results have been accumulated so far, logs an error, and hard-exits the
 *  process so CI never hangs waiting for it.
 *
 *  Design rationale:
 *    - We are specifically testing synchronization primitives that can break
 *      in ways that cause infinite blocking (e.g. a mutex that never unlocks).
 *    - Test code SHOULD use bounded Lock() timeouts (see MUTEX_TEST_LOCK_TIMEOUT_IN_MS),
 *      but the watchdog is a second safety net in case a timeout is missed,
 *      or in case a future test is added without a proper bounded timeout.
 */
class CTestWatchdog : public MT::CActiveObject
{
    public:

    CTestWatchdog( UInt32 timeoutInMs, const CORE::CString& resultsXmlPath )
        : MT::CActiveObject()
        , m_timeoutInMs( timeoutInMs )
        , m_resultsXmlPath( resultsXmlPath )
        , m_cancelled( false )
    {}

    void Cancel( void ) { m_cancelled = true; }

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual void OnThreadStarted( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnThreadEnding( void* taskdata, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // Sleep in small slices so we can react quickly to cancellation
        UInt32 elapsed = 0;
        while ( !m_cancelled && elapsed < m_timeoutInMs )
        {
            MT::PrecisionDelay( 250 );
            elapsed += 250;
        }

        if ( !m_cancelled )
        {
            // Tests are stuck — write whatever partial results we have and exit hard
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL,
                "WATCHDOG FIRED: test suite did not complete within " +
                CORE::ToString( m_timeoutInMs ) + "ms. "
                "A synchronization primitive is likely stuck (deadlock). "
                "Writing partial results and terminating with exit code " +
                CORE::ToString( (Int32) GUCEF_MT_TESTAPP_WATCHDOG_EXIT_CODE ) );

            CORE::CLogStreamScope::FlushLogs();

            GUCEF_TESTFW_WRITE_RESULTS( m_resultsXmlPath.C_String() );

            ::exit( GUCEF_MT_TESTAPP_WATCHDOG_EXIT_CODE );
        }

        return false; // stop cycling (cancelled path)
    }

    private:

    UInt32          m_timeoutInMs;
    CORE::CString   m_resultsXmlPath;
    volatile bool   m_cancelled;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefMT_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );

        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF_TESTFW_INIT( "gucefMT_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( GUCEF::CORE::TestFrameworkLogCallback, GUCEF_NULL );

        GUCEF::CORE::CString xmlPath = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( xmlPath, "gucefMT_TestApp_Results.xml" );

        // Start the watchdog before any test runs.
        // It will kill the process if the suite takes longer than the deadline,
        // which protects against broken synchronization primitives that deadlock.
        CTestWatchdog watchdog( GUCEF_MT_TESTAPP_WATCHDOG_TIMEOUT_IN_MS, xmlPath );
        watchdog.Activate( GUCEF_NULL, true );

        PerformMutexTests();
        PerformReaderWriterLockTests();

        // Tests finished normally — disarm the watchdog before it fires
        watchdog.Cancel();
        watchdog.WaitForThreadToFinish( 5000 );

        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );
        return GUCEF_TESTFW_EXITCODE;
    }
    catch ( ... )
    {
        GUCEF::CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                       "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
