/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_TIMEOUT_EXCEPTION_H
#include "gucef_timeout_exception.h"
#define GUCEF_TIMEOUT_EXCEPTION_H
#endif /* GUCEF_TIMEOUT_EXCEPTION_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#include "gucefCORE_CLogStreamScope.h"
#define GUCEF_CORE_CLOGSTREAMSCOPE_H
#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestLogManagerLockOrder.h"

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

/** How long the two threads of a case run side by side */
static const UInt32 LockOrderRunTimeInMs = 1500;

/** A thread that makes no progress for this long is waiting on a lock the other thread will not give up */
static const UInt32 LockOrderStallTimeInMs = 5000;

/** After a deadlock a lock wait ends with an exception after the lock timeout of gucefMT: the threads end after that */
static const UInt32 LockOrderThreadEndTimeInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS + 15000;

/*-------------------------------------------------------------------------*/

/** A logger that only counts what reaches it */
class CLockOrderCountingLogger : public CORE::CILogger
{
    public:

    CLockOrderCountingLogger( void )
        : CORE::CILogger()
        , m_count( 0 )
    {}

    virtual ~CLockOrderCountingLogger()
    {}

    virtual void Log( const CORE::TLogMsgType /*logMsgType*/ ,
                      const Int32 /*logLevel*/               ,
                      const CORE::CString& /*logMessage*/    ,
                      const UInt32 /*threadId*/              ,
                      const CORE::CTimestamp& /*timestamp*/  ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_count.FetchAdd( 1 );
    }

    virtual void Log( const CORE::TLogMsgType /*logMsgType*/     ,
                      const Int32 /*logLevel*/                   ,
                      const CORE::CVariantStream& /*logMessage*/ ,
                      const UInt32 /*threadId*/                  ,
                      const CORE::CTimestamp& /*timestamp*/      ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_count.FetchAdd( 1 );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType /*logMsgType*/ ,
                                       const Int32 /*logLevel*/               ,
                                       const CORE::CString& /*logMessage*/    ,
                                       const UInt32 /*threadId*/              ,
                                       const CORE::CTimestamp& /*timestamp*/  ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_count.FetchAdd( 1 );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType /*logMsgType*/     ,
                                       const Int32 /*logLevel*/                   ,
                                       const CORE::CVariantStream& /*logMessage*/ ,
                                       const UInt32 /*threadId*/                  ,
                                       const CORE::CTimestamp& /*timestamp*/      ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_count.FetchAdd( 1 );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE
    {}

    virtual void SetMinimalLogLevel( const Int32 /*logLevel*/ ) GUCEF_VIRTUAL_OVERRIDE
    {}

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return CORE::LOGLEVEL_EVERYTHING;
    }

    UInt32 GetCount( void ) const
    {
        return m_count.Load();
    }

    private:

    MT::CTAtomic< UInt32 > m_count;
};

/*-------------------------------------------------------------------------*/

/** What one thread of a case shares with the test thread */
struct SLockOrderThread
{
    MT::CTAtomic< bool > stop;
    MT::CTAtomic< UInt32 > progress;           /**< rounds the thread finished */
    MT::CTAtomic< UInt32 > lockTimeouts;       /**< lock waits that ended with a timeout exception */
    MT::CTAtomic< UInt32 > otherExceptions;

    SLockOrderThread( void )
        : stop( false )
        , progress( 0 )
        , lockTimeouts( 0 )
        , otherExceptions( 0 )
    {}
};

/*-------------------------------------------------------------------------*/

/** What the threads of the cases work with. The threads may outlive a case that saw a deadlock, so it is not on a stack */
struct SLockOrderCase
{
    SLockOrderThread changer;                   /**< changes the loggers of the log manager */
    SLockOrderThread logger;                    /**< logs and flushes */
    CLockOrderCountingLogger countingLogger;
};

static SLockOrderCase g_loggerChangeCase;

/*-------------------------------------------------------------------------*/

static CORE::CLogManager&
GetLogManager( void )
{
    return CORE::CCoreGlobal::Instance()->GetLogManager();
}

/*-------------------------------------------------------------------------*/

/** Adds a logger to the log manager and takes it out again, round after round */
static UInt32 GUCEF_CALLSPEC_PREFIX
LoggerChangerMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SLockOrderCase& lockCase = *static_cast< SLockOrderCase* >( data );
    while ( !lockCase.changer.stop.Load( MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            GetLogManager().AddLogger( &lockCase.countingLogger );
            GetLogManager().RemoveLogger( &lockCase.countingLogger );
            lockCase.changer.progress.FetchAdd( 1 );
        }
        catch ( const timeout_exception& )
        {
            lockCase.changer.lockTimeouts.FetchAdd( 1 );
            break;
        }
        catch ( ... )
        {
            lockCase.changer.otherExceptions.FetchAdd( 1 );
            break;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Logs from its own thread, which has a thread buffer, and flushes the thread buffers of all threads, round after round */
static UInt32 GUCEF_CALLSPEC_PREFIX
LogAndFlushMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SLockOrderCase& lockCase = *static_cast< SLockOrderCase* >( data );
    UInt32 lineNr = 0;
    while ( !lockCase.logger.stop.Load( MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_EVERYTHING, "log manager lock order: line " << lineNr );
            ++lineNr;
            CORE::CLogStreamScope::FlushLogs();
            lockCase.logger.progress.FetchAdd( 1 );
        }
        catch ( const timeout_exception& )
        {
            lockCase.logger.lockTimeouts.FetchAdd( 1 );
            break;
        }
        catch ( ... )
        {
            lockCase.logger.otherExceptions.FetchAdd( 1 );
            break;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** The outcome of running the two threads of a case side by side */
struct SLockOrderOutcome
{
    bool started;
    bool stalled;               /**< one of the threads made no progress for LockOrderStallTimeInMs */
    bool threadsEnded;
    UInt32 changerRounds;
    UInt32 loggerRounds;
    UInt32 lockTimeouts;
    UInt32 otherExceptions;
};

/*-------------------------------------------------------------------------*/

static SLockOrderOutcome
RunLockOrderCase( SLockOrderCase& lockCase  ,
                  MT::TThreadFunc changer   ,
                  MT::TThreadFunc logger    )
{
    SLockOrderOutcome outcome;
    outcome.started = false;
    outcome.stalled = false;
    outcome.threadsEnded = false;
    outcome.changerRounds = 0;
    outcome.loggerRounds = 0;
    outcome.lockTimeouts = 0;
    outcome.otherExceptions = 0;

    MT::SThreadData* changerThread = MT::ThreadDataReserve();
    MT::SThreadData* loggerThread = MT::ThreadDataReserve();
    if ( GUCEF_NULL == changerThread || GUCEF_NULL == loggerThread )
        return outcome;
    if ( 0 == MT::ThreadCreate( changerThread, changer, &lockCase ) )
        return outcome;
    if ( 0 == MT::ThreadCreate( loggerThread, logger, &lockCase ) )
    {
        lockCase.changer.stop.Store( true, MT::MEMORDER_RELEASE );
        MT::ThreadWait( changerThread, LockOrderThreadEndTimeInMs );
        return outcome;
    }
    outcome.started = true;

    // both threads have to keep making progress for the whole run
    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    const UInt64 startTicks = MT::PrecisionTickCount();
    UInt64 changerProgressTicks = startTicks;
    UInt64 loggerProgressTicks = startTicks;
    UInt32 lastChangerRounds = 0;
    UInt32 lastLoggerRounds = 0;
    while ( true )
    {
        MT::ThreadDelay( 10 );
        const UInt64 nowTicks = MT::PrecisionTickCount();

        const UInt32 changerRounds = lockCase.changer.progress.Load();
        const UInt32 loggerRounds = lockCase.logger.progress.Load();
        if ( changerRounds != lastChangerRounds )
        {
            lastChangerRounds = changerRounds;
            changerProgressTicks = nowTicks;
        }
        if ( loggerRounds != lastLoggerRounds )
        {
            lastLoggerRounds = loggerRounds;
            loggerProgressTicks = nowTicks;
        }

        if ( ( nowTicks - changerProgressTicks ) > LockOrderStallTimeInMs * ticksPerMs ||
             ( nowTicks - loggerProgressTicks ) > LockOrderStallTimeInMs * ticksPerMs  )
        {
            outcome.stalled = true;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Log manager lock order: a thread made no progress for "
                << LockOrderStallTimeInMs << " ms, changer rounds " << changerRounds << ", logger rounds " << loggerRounds );
            break;
        }
        if ( ( nowTicks - startTicks ) > LockOrderRunTimeInMs * ticksPerMs )
            break;
    }

    lockCase.changer.stop.Store( true, MT::MEMORDER_RELEASE );
    lockCase.logger.stop.Store( true, MT::MEMORDER_RELEASE );
    const bool changerEnded = 1 == MT::ThreadWait( changerThread, LockOrderThreadEndTimeInMs );
    const bool loggerEnded = 1 == MT::ThreadWait( loggerThread, LockOrderThreadEndTimeInMs );
    outcome.threadsEnded = changerEnded && loggerEnded;

    // the thread data of a thread that did not end stays: that thread may still use it
    if ( changerEnded )
        MT::ThreadDataCleanup( changerThread );
    if ( loggerEnded )
        MT::ThreadDataCleanup( loggerThread );

    outcome.changerRounds = lockCase.changer.progress.Load();
    outcome.loggerRounds = lockCase.logger.progress.Load();
    outcome.lockTimeouts = lockCase.changer.lockTimeouts.Load() + lockCase.logger.lockTimeouts.Load();
    outcome.otherExceptions = lockCase.changer.otherExceptions.Load() + lockCase.logger.otherExceptions.Load();
    return outcome;
}

/*-------------------------------------------------------------------------*/

void
PerformLogManagerLockOrderTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING log manager lock order TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "LogManagerLockOrder" );

    GUCEF_TESTFW_TESTCASE( "Loggers added and removed on one thread while another logs and flushes the thread buffers: no deadlock" )
        try
        {
            SLockOrderCase& lockCase = g_loggerChangeCase;
            SLockOrderOutcome outcome = RunLockOrderCase( lockCase, &LoggerChangerMain, &LogAndFlushMain );

            ASSERT_TRUE( outcome.started );
            ASSERT_FALSE( outcome.stalled );
            ASSERT_TRUE( outcome.threadsEnded );
            ASSERT_TRUE( 0 == outcome.lockTimeouts );
            ASSERT_TRUE( 0 == outcome.otherExceptions );
            ASSERT_TRUE( outcome.changerRounds > 0 );
            ASSERT_TRUE( outcome.loggerRounds > 0 );

            // a round that ended with an exception may have left the logger in
            if ( outcome.threadsEnded )
                GetLogManager().RemoveLogger( &lockCase.countingLogger );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
