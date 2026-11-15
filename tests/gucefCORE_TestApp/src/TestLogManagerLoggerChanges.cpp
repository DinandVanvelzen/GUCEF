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

#include <stdlib.h>
#include <string.h>

#ifdef GUCEF_LINUX_BUILD
#include <unistd.h>
#endif /* GUCEF_LINUX_BUILD ? */

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

#ifndef GUCEF_CORE_CMULTILOGGER_H
#include "gucefCORE_CMultiLogger.h"
#define GUCEF_CORE_CMULTILOGGER_H
#endif /* GUCEF_CORE_CMULTILOGGER_H ? */

#ifndef GUCEF_CORE_CLOGGINGTASK_H
#include "gucefCORE_CLoggingTask.h"
#define GUCEF_CORE_CLOGGINGTASK_H
#endif /* GUCEF_CORE_CLOGGINGTASK_H ? */

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#include "gucefCORE_CLogStreamScope.h"
#define GUCEF_CORE_CLOGSTREAMSCOPE_H
#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CSTDLOGGER_H
#include "CStdLogger.h"
#define GUCEF_CORE_CSTDLOGGER_H
#endif /* GUCEF_CORE_CSTDLOGGER_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include "TestLogManagerLoggerChanges.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/** What a case found wrong, combined into the exit code of its process */
enum ELoggerChangeFailure
{
    LOGGERCHANGE_LOST                   = 1,    /**< a message never reached the logger that stayed registered */
    LOGGERCHANGE_ORDER                  = 2,    /**< a message came twice, or out of the order of its thread */
    LOGGERCHANGE_CONCURRENT_CALLS       = 4,    /**< a logger was called by two threads at once */
    LOGGERCHANGE_CALL_AFTER_REMOVAL     = 8,    /**< a logger was called after its removal returned */
    LOGGERCHANGE_STALLED                = 16,   /**< a thread of the case did not end in time */
    LOGGERCHANGE_SETUP                  = 32,   /**< the case could not be set up */
    LOGGERCHANGE_REENTRANT_CHANGE       = 64    /**< a change made from within a log call did not take effect */
};

/** Threads of a case that log, half through the log macros (thread buffers), half through CLogManager::Log() */
static const UInt32 LoggerChangeLoggingThreads = 4;

/** Threads of a case that add and remove loggers */
static const UInt32 LoggerChangeChangerThreads = 2;

/** The most threads a checking logger keeps the order of */
static const UInt32 LoggerChangeMaxThreads = 8;

/** Messages each logging thread makes */
static const UInt32 LoggerChangeMessagesPerThread = 5000;

/** The longest a case waits for its threads, or for its messages to arrive */
static const UInt32 LoggerChangeWaitInMs = 120000;

/** The prefix of the messages a checking logger looks at */
static const char* const LoggerChangePrefix = "LCR ";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Reads "LCR <thread> <seq>"; false for any other message */
static bool
ParseLoggerChangeMessage( const CORE::CString& message ,
                          UInt32& threadIndex          ,
                          UInt32& seqNr                )
{
    const char* text = message.C_String();
    if ( GUCEF_NULL == text || 0 != strncmp( text, LoggerChangePrefix, 4 ) )
        return false;
    char* end = GUCEF_NULL;
    threadIndex = (UInt32) strtoul( text + 4, &end, 10 );
    if ( GUCEF_NULL == end || end == text + 4 )
        return false;
    const char* seqText = end;
    seqNr = (UInt32) strtoul( seqText, &end, 10 );
    return GUCEF_NULL != end && end != seqText;
}

/*-------------------------------------------------------------------------*/

/** Base of the loggers of the cases: every log call ends up in OnMessage() */
class CLoggerChangeLoggerBase : public CORE::CILogger
{
    public:

    CLoggerChangeLoggerBase( void )
        : CORE::CILogger()
    {}

    virtual ~CLoggerChangeLoggerBase()
    {}

    virtual void Log( const CORE::TLogMsgType /*logMsgType*/ ,
                      const Int32 /*logLevel*/               ,
                      const CORE::CString& logMessage        ,
                      const TSystemThreadId /*threadId*/     ,
                      const CORE::CTimestamp& /*timestamp*/  ) GUCEF_VIRTUAL_OVERRIDE
    {
        OnMessage( logMessage );
    }

    virtual void Log( const CORE::TLogMsgType /*logMsgType*/ ,
                      const Int32 /*logLevel*/               ,
                      const CORE::CVariantStream& logMessage ,
                      const TSystemThreadId /*threadId*/     ,
                      const CORE::CTimestamp& /*timestamp*/  ) GUCEF_VIRTUAL_OVERRIDE
    {
        OnMessage( logMessage.ToString() );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType /*logMsgType*/ ,
                                       const Int32 /*logLevel*/               ,
                                       const CORE::CString& logMessage        ,
                                       const TSystemThreadId /*threadId*/     ,
                                       const CORE::CTimestamp& /*timestamp*/  ) GUCEF_VIRTUAL_OVERRIDE
    {
        OnMessage( logMessage );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType /*logMsgType*/ ,
                                       const Int32 /*logLevel*/               ,
                                       const CORE::CVariantStream& logMessage ,
                                       const TSystemThreadId /*threadId*/     ,
                                       const CORE::CTimestamp& /*timestamp*/  ) GUCEF_VIRTUAL_OVERRIDE
    {
        OnMessage( logMessage.ToString() );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE
    {}

    virtual void SetMinimalLogLevel( const Int32 /*logLevel*/ ) GUCEF_VIRTUAL_OVERRIDE
    {}

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return CORE::LOGLEVEL_EVERYTHING;
    }

    protected:

    virtual void OnMessage( const CORE::CString& message ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  The logger that stays registered: every message of the logging threads has to reach it exactly once, in the order
 *  of its thread, and it must never be called by two threads at once
 */
class CLoggerChangeCheckLogger : public CLoggerChangeLoggerBase
{
    public:

    CLoggerChangeCheckLogger( void )
        : CLoggerChangeLoggerBase()
        , m_inCall( false )
        , m_concurrentCalls( 0 )
        , m_received( 0 )
        , m_outOfOrder( 0 )
        , m_onOtherThreads( 0 )
        , m_lock()
        , m_testThreadCount( 0 )
    {
        for ( UInt32 i=0; i<LoggerChangeMaxThreads; ++i )
            m_nextSeqNr[ i ] = 0;
    }

    /** A thread of the case: a message delivered on any other thread was delivered by the logging thread */
    void RegisterTestThread( void )
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_testThreadCount < MaxTestThreads )
        {
            m_testThreads[ m_testThreadCount ] = MT::GetCurrentThreadID();
            ++m_testThreadCount;
        }
    }

    /** Messages of the case delivered on a thread that is not one of the case: the logging thread */
    UInt32 GetDeliveredByLoggingThread( void ) const { return m_onOtherThreads.Load( MT::MEMORDER_ACQUIRE ); }

    UInt32 GetReceived( void ) const { return m_received.Load( MT::MEMORDER_ACQUIRE ); }
    UInt32 GetOutOfOrder( void ) const { return m_outOfOrder.Load(); }
    UInt32 GetConcurrentCalls( void ) const { return m_concurrentCalls.Load(); }

    protected:

    virtual void OnMessage( const CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE
    {
        bool notInCall = false;
        const bool entered = m_inCall.CompareExchangeStrong( notInCall, true );
        if ( !entered )
            m_concurrentCalls.FetchAdd( 1 );

        UInt32 threadIndex = 0;
        UInt32 seqNr = 0;
        if ( ParseLoggerChangeMessage( message, threadIndex, seqNr ) && threadIndex < LoggerChangeMaxThreads )
        {
            MT::CScopeMutex lock( m_lock );
            if ( seqNr != m_nextSeqNr[ threadIndex ] )
                m_outOfOrder.FetchAdd( 1 );
            m_nextSeqNr[ threadIndex ] = seqNr + 1;
            m_received.FetchAdd( 1 );

            const TThreadId threadId = MT::GetCurrentThreadID();
            bool onTestThread = false;
            for ( UInt32 i=0; i<m_testThreadCount; ++i )
                onTestThread = onTestThread || threadId == m_testThreads[ i ];
            if ( !onTestThread )
                m_onOtherThreads.FetchAdd( 1 );
        }

        if ( entered )
            m_inCall.Store( false );
    }

    private:

    MT::CTAtomic< bool > m_inCall;
    MT::CTAtomic< UInt32 > m_concurrentCalls;
    MT::CTAtomic< UInt32 > m_received;
    MT::CTAtomic< UInt32 > m_outOfOrder;
    MT::CTAtomic< UInt32 > m_onOtherThreads;
    MT::CMutex m_lock;
    UInt32 m_nextSeqNr[ LoggerChangeMaxThreads ];
    static const UInt32 MaxTestThreads = 16;
    TThreadId m_testThreads[ MaxTestThreads ];
    UInt32 m_testThreadCount;
};

/*-------------------------------------------------------------------------*/

/** A logger that is added and removed: it counts the calls it gets after it was marked removed */
class CLoggerChangeTransientLogger : public CLoggerChangeLoggerBase
{
    public:

    CLoggerChangeTransientLogger( MT::CTAtomic< UInt32 >& callsAfterRemoval )
        : CLoggerChangeLoggerBase()
        , m_work( 0 )
        , m_removed( false )
        , m_caseMessages( 0 )
        , m_callsAfterRemoval( callsAfterRemoval )
    {}

    /** Called once the removal of the logger returned: from then on it must not be called */
    void MarkRemoved( void ) { m_removed.Store( true, MT::MEMORDER_RELEASE ); }

    /** The messages of the logging threads ("LCR ...") it got; others, such as those of the log manager, not counted */
    UInt32 GetCaseMessages( void ) const { return m_caseMessages.Load(); }

    protected:

    virtual void OnMessage( const CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 threadIndex = 0;
        UInt32 seqNr = 0;
        if ( ParseLoggerChangeMessage( message, threadIndex, seqNr ) )
            m_caseMessages.FetchAdd( 1 );
        if ( m_removed.Load( MT::MEMORDER_ACQUIRE ) )
        {
            m_callsAfterRemoval.FetchAdd( 1 );
            return;
        }

        // a logger with some work to do (formatting, writing): a removal that returns while it still runs is as wrong
        // as a call after the removal, the remover may delete the logger at once
        for ( UInt32 i=0; i<WorkRounds; ++i )
            m_work.FetchAdd( 1, MT::MEMORDER_RELAXED );
        if ( m_removed.Load( MT::MEMORDER_ACQUIRE ) )
            m_callsAfterRemoval.FetchAdd( 1 );
    }

    private:

    static const UInt32 WorkRounds = 2000;

    MT::CTAtomic< UInt32 > m_work;
    MT::CTAtomic< bool > m_removed;
    MT::CTAtomic< UInt32 > m_caseMessages;
    MT::CTAtomic< UInt32 >& m_callsAfterRemoval;
};

/*-------------------------------------------------------------------------*/

/** What the threads of a case share. It is not on a stack: a thread that does not end may still use it */
struct SLoggerChangeCase
{
    CORE::CLogManager* logManager;
    CLoggerChangeCheckLogger checkLogger;
    MT::CTAtomic< bool > stopChangers;
    MT::CTAtomic< UInt32 > changeRounds;
    MT::CTAtomic< UInt32 > callsAfterRemoval;
    MT::CTAtomic< UInt32 > exceptions;
    MT::CMutex removedLock;
    GUCEF::vector< CLoggerChangeTransientLogger* > removed;     /**< deleted when the case is over */
    CORE::CString notes;                                        /**< details of what went wrong, for the report */

    SLoggerChangeCase( void )
        : logManager( GUCEF_NULL )
        , checkLogger()
        , stopChangers( false )
        , changeRounds( 0 )
        , callsAfterRemoval( 0 )
        , exceptions( 0 )
        , removedLock()
        , removed()
        , notes()
    {}

    ~SLoggerChangeCase()
    {
        for ( UInt32 i=0; i<(UInt32)removed.size(); ++i )
            GUCEF_DELETE removed[ i ];
    }
};

/*-------------------------------------------------------------------------*/

struct SLoggerChangeThread
{
    SLoggerChangeCase* lcCase;
    UInt32 index;
    MT::SThreadData* thread;
};

/*-------------------------------------------------------------------------*/

/** Logs the messages of its thread: even threads through CLogManager::Log(), odd ones through the log macros */
static UInt32 GUCEF_CALLSPEC_PREFIX
LoggerChangeLoggingMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SLoggerChangeThread& arg = *static_cast< SLoggerChangeThread* >( data );
    try
    {
        arg.lcCase->checkLogger.RegisterTestThread();
        const bool viaMacros = 1 == ( arg.index % 2 );
        for ( UInt32 seqNr=0; seqNr<LoggerChangeMessagesPerThread; ++seqNr )
        {
            if ( viaMacros )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, LoggerChangePrefix << arg.index << " " << seqNr );
                if ( 0 == ( seqNr % 64 ) )
                    CORE::CLogStreamScope::FlushLogs();
            }
            else
            {
                CORE::CString message( LoggerChangePrefix );
                message += CORE::ToString( arg.index );
                message += " ";
                message += CORE::ToString( seqNr );
                arg.lcCase->logManager->Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_IMPORTANT, message );
            }
        }
        if ( viaMacros )
            CORE::CLogStreamScope::FlushLogs();
    }
    catch ( ... )
    {
        arg.lcCase->exceptions.FetchAdd( 1 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Adds a new logger and removes it again, round after round; a removed logger is kept until the case is over */
static UInt32 GUCEF_CALLSPEC_PREFIX
LoggerChangeChangerMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SLoggerChangeThread& arg = *static_cast< SLoggerChangeThread* >( data );
    SLoggerChangeCase& lcCase = *arg.lcCase;
    try
    {
        lcCase.checkLogger.RegisterTestThread();
        while ( !lcCase.stopChangers.Load( MT::MEMORDER_ACQUIRE ) )
        {
            CLoggerChangeTransientLogger* logger = GUCEF_NEW CLoggerChangeTransientLogger( lcCase.callsAfterRemoval );
            lcCase.logManager->AddLogger( logger );
            for ( UInt32 i=0; i<( 1 + ( arg.index * 7 ) ) * 64; ++i )
            {
                // a short while with the logger in the set
            }
            lcCase.logManager->RemoveLogger( logger );
            logger->MarkRemoved();
            {
                MT::CScopeMutex lock( lcCase.removedLock );
                lcCase.removed.push_back( logger );
            }
            lcCase.changeRounds.FetchAdd( 1 );
        }
    }
    catch ( ... )
    {
        lcCase.exceptions.FetchAdd( 1 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

static bool
StartLoggerChangeThread( SLoggerChangeThread& arg, MT::TThreadFunc func )
{
    arg.thread = MT::ThreadDataReserve();
    if ( GUCEF_NULL == arg.thread )
        return false;
    if ( 0 == MT::ThreadCreate( arg.thread, func, &arg ) )
    {
        MT::ThreadDataCleanup( arg.thread );
        arg.thread = GUCEF_NULL;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Waits for the thread; false when it did not end in time, its thread data then stays (the thread may use it) */
static bool
EndLoggerChangeThread( SLoggerChangeThread& arg )
{
    if ( GUCEF_NULL == arg.thread )
        return true;
    if ( 1 != MT::ThreadWait( arg.thread, LoggerChangeWaitInMs ) )
        return false;
    MT::ThreadDataCleanup( arg.thread );
    arg.thread = GUCEF_NULL;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Waits until the checking logger got the given number of messages; false when they did not come in time */
static bool
WaitForLoggerChangeMessages( SLoggerChangeCase& lcCase, UInt32 expected, UInt32 waitInMs )
{
    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    const UInt64 startTicks = MT::PrecisionTickCount();
    while ( lcCase.checkLogger.GetReceived() < expected )
    {
        if ( ( MT::PrecisionTickCount() - startTicks ) > waitInMs * ticksPerMs )
            return false;
        CORE::CLogStreamScope::FlushLogs();
        MT::ThreadDelay( 5 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Writes what a case found to a log file of its own in the working directory, once its loggers are gone */
static void
ReportLoggerChangeCase( const char* caseName, int failures, SLoggerChangeCase& lcCase, UInt32 expected )
{
    CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( logFilename, CORE::CString( "gucefCORE_TestApp_LoggerChange_" ) + caseName + ".txt" );
    CORE::CFileAccess logFileAccess( logFilename, "w" );
    CORE::CStdLogger logger( logFileAccess );
    lcCase.logManager->AddLogger( &logger );
    GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "Logger change case " << caseName << ": failures " << failures
        << ", messages " << lcCase.checkLogger.GetReceived() << " of " << expected
        << ", out of order " << lcCase.checkLogger.GetOutOfOrder()
        << ", concurrent calls " << lcCase.checkLogger.GetConcurrentCalls()
        << ", calls after removal " << lcCase.callsAfterRemoval.Load()
        << ", change rounds " << lcCase.changeRounds.Load()
        << ", by the logging thread " << lcCase.checkLogger.GetDeliveredByLoggingThread()
        << ", exceptions " << lcCase.exceptions.Load() << lcCase.notes );
    CORE::CLogStreamScope::FlushLogs();
    lcCase.logManager->RemoveLogger( &logger );
}

/*-------------------------------------------------------------------------*/

/**
 *  "race" and "stop": logging threads and changer threads side by side, with the logging thread on ("race"), or
 *  switched off and on again meanwhile ("stop")
 */
static int
RunLoggerChangeThreadsCase( const char* caseName, bool toggleLoggingThread )
{
    // the case and its threads stay when a thread does not end: that thread may still use them
    SLoggerChangeCase* lcCase = GUCEF_NEW SLoggerChangeCase();
    lcCase->logManager = &CORE::CCoreGlobal::Instance()->GetLogManager();
    CORE::CLogManager& logManager = *lcCase->logManager;
    lcCase->logManager->FlushLogs();

    lcCase->checkLogger.RegisterTestThread();
    logManager.AddLogger( &lcCase->checkLogger );
    logManager.SetUseLoggingThread( true );
    if ( !logManager.GetUseLoggingThread() )
        return LOGGERCHANGE_SETUP;

    SLoggerChangeThread* loggingThreads = GUCEF_NEW SLoggerChangeThread[ LoggerChangeLoggingThreads ];
    SLoggerChangeThread* changerThreads = GUCEF_NEW SLoggerChangeThread[ LoggerChangeChangerThreads ];
    int failures = 0;
    for ( UInt32 i=0; i<LoggerChangeChangerThreads; ++i )
    {
        changerThreads[ i ].lcCase = lcCase;
        changerThreads[ i ].index = i;
        changerThreads[ i ].thread = GUCEF_NULL;
        if ( !StartLoggerChangeThread( changerThreads[ i ], &LoggerChangeChangerMain ) )
            failures |= LOGGERCHANGE_SETUP;
    }
    for ( UInt32 i=0; i<LoggerChangeLoggingThreads; ++i )
    {
        loggingThreads[ i ].lcCase = lcCase;
        loggingThreads[ i ].index = i;
        loggingThreads[ i ].thread = GUCEF_NULL;
        if ( !StartLoggerChangeThread( loggingThreads[ i ], &LoggerChangeLoggingMain ) )
            failures |= LOGGERCHANGE_SETUP;
    }

    if ( toggleLoggingThread )
    {
        // off and on again while the threads log and change the loggers
        for ( UInt32 i=0; i<10; ++i )
        {
            // off once the logging thread delivered some since it was switched on: its end has messages to hand over
            const UInt32 deliveredBefore = lcCase->checkLogger.GetDeliveredByLoggingThread();
            for ( UInt32 w=0; w<200 && lcCase->checkLogger.GetDeliveredByLoggingThread() == deliveredBefore; ++w )
                MT::ThreadDelay( 5 );
            logManager.SetUseLoggingThread( false );
            MT::ThreadDelay( 15 );
            logManager.SetUseLoggingThread( true );
        }
    }

    bool allEnded = true;
    for ( UInt32 i=0; i<LoggerChangeLoggingThreads; ++i )
        allEnded = EndLoggerChangeThread( loggingThreads[ i ] ) && allEnded;
    lcCase->stopChangers.Store( true, MT::MEMORDER_RELEASE );
    for ( UInt32 i=0; i<LoggerChangeChangerThreads; ++i )
        allEnded = EndLoggerChangeThread( changerThreads[ i ] ) && allEnded;
    if ( !allEnded )
        return failures | LOGGERCHANGE_STALLED;

    const UInt32 expected = LoggerChangeLoggingThreads * LoggerChangeMessagesPerThread;
    if ( toggleLoggingThread )
    {
        // once the logging thread is off every message was handed to the loggers
        logManager.SetUseLoggingThread( false );
        CORE::CLogStreamScope::FlushLogs();
        if ( !WaitForLoggerChangeMessages( *lcCase, expected, 10000 ) )
            failures |= LOGGERCHANGE_LOST;
    }
    else
    {
        if ( !WaitForLoggerChangeMessages( *lcCase, expected, LoggerChangeWaitInMs ) )
            failures |= LOGGERCHANGE_LOST;
        logManager.SetUseLoggingThread( false );
    }

    if ( 0 != lcCase->checkLogger.GetOutOfOrder() || lcCase->checkLogger.GetReceived() > expected )
        failures |= LOGGERCHANGE_ORDER;
    if ( 0 != lcCase->checkLogger.GetConcurrentCalls() )
        failures |= LOGGERCHANGE_CONCURRENT_CALLS;
    if ( 0 != lcCase->callsAfterRemoval.Load() )
        failures |= LOGGERCHANGE_CALL_AFTER_REMOVAL;
    if ( 0 != lcCase->exceptions.Load() || 0 == lcCase->changeRounds.Load() || 0 == lcCase->checkLogger.GetDeliveredByLoggingThread() )
        failures |= LOGGERCHANGE_SETUP;

    logManager.RemoveLogger( &lcCase->checkLogger );
    ReportLoggerChangeCase( caseName, failures, *lcCase, expected );

    GUCEF_DELETE[] loggingThreads;
    GUCEF_DELETE[] changerThreads;
    GUCEF_DELETE lcCase;
    return failures;
}

/*-------------------------------------------------------------------------*/

/**
 *  A logger that changes the loggers from within its log call: at one message it adds a logger, at another it
 *  removes one, as a logger that reacts to what it logs might
 */
class CLoggerChangeReentrantLogger : public CLoggerChangeLoggerBase
{
    public:

    CLoggerChangeReentrantLogger( CORE::CLogManager& logManager           ,
                                  CORE::CILogger& toAdd                   ,
                                  CLoggerChangeTransientLogger& toRemove  ,
                                  UInt32 addAtSeqNr                       ,
                                  UInt32 removeAtSeqNr                    )
        : CLoggerChangeLoggerBase()
        , m_logManager( logManager )
        , m_toAdd( toAdd )
        , m_toRemove( toRemove )
        , m_addAtSeqNr( addAtSeqNr )
        , m_removeAtSeqNr( removeAtSeqNr )
        , m_changes( 0 )
    {}

    UInt32 GetChanges( void ) const { return m_changes.Load(); }

    protected:

    virtual void OnMessage( const CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 threadIndex = 0;
        UInt32 seqNr = 0;
        if ( !ParseLoggerChangeMessage( message, threadIndex, seqNr ) )
            return;
        if ( seqNr == m_addAtSeqNr )
        {
            m_logManager.AddLogger( &m_toAdd );
            m_changes.FetchAdd( 1 );
        }
        if ( seqNr == m_removeAtSeqNr )
        {
            m_logManager.RemoveLogger( &m_toRemove );
            m_toRemove.MarkRemoved();
            m_changes.FetchAdd( 1 );
        }
    }

    private:

    CORE::CLogManager& m_logManager;
    CORE::CILogger& m_toAdd;
    CLoggerChangeTransientLogger& m_toRemove;
    UInt32 m_addAtSeqNr;
    UInt32 m_removeAtSeqNr;
    MT::CTAtomic< UInt32 > m_changes;
};

/*-------------------------------------------------------------------------*/

/** One round of the "reentrant" case, with or without the logging thread */
static int
RunLoggerChangeReentrantRound( SLoggerChangeCase& lcCase, UInt32 threadIndex, bool useLoggingThread )
{
    CORE::CLogManager& logManager = *lcCase.logManager;
    int failures = 0;

    MT::CTAtomic< UInt32 > addedCallsAfterRemoval( 0 );
    CLoggerChangeTransientLogger added( addedCallsAfterRemoval );
    CLoggerChangeTransientLogger removed( lcCase.callsAfterRemoval );

    // the logger that is removed from within the log call comes after the one that removes it, so that the removal
    // happens while the delivery of that message is still to reach it
    CLoggerChangeReentrantLogger changer( logManager, added, removed, 2, 5 );
    logManager.AddLogger( &changer );
    logManager.AddLogger( &removed );

    logManager.SetUseLoggingThread( useLoggingThread );
    if ( useLoggingThread != logManager.GetUseLoggingThread() )
        failures |= LOGGERCHANGE_SETUP;

    const UInt32 messages = 10;
    const UInt32 receivedBefore = lcCase.checkLogger.GetReceived();
    for ( UInt32 seqNr=0; seqNr<messages; ++seqNr )
    {
        CORE::CString message( LoggerChangePrefix );
        message += CORE::ToString( threadIndex );
        message += " ";
        message += CORE::ToString( seqNr );
        logManager.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_IMPORTANT, message );
    }
    if ( !WaitForLoggerChangeMessages( lcCase, receivedBefore + messages, 30000 ) )
        failures |= LOGGERCHANGE_LOST;

    // a change of the loggers made later, outside any log call, and messages after it
    CLoggerChangeTransientLogger later( lcCase.callsAfterRemoval );
    logManager.AddLogger( &later );
    for ( UInt32 seqNr=messages; seqNr<messages+5; ++seqNr )
    {
        CORE::CString message( LoggerChangePrefix );
        message += CORE::ToString( threadIndex );
        message += " ";
        message += CORE::ToString( seqNr );
        logManager.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_IMPORTANT, message );
    }
    if ( !WaitForLoggerChangeMessages( lcCase, receivedBefore + messages + 5, 30000 ) )
        failures |= LOGGERCHANGE_LOST;
    logManager.RemoveLogger( &later );
    later.MarkRemoved();

    logManager.SetUseLoggingThread( false );
    logManager.RemoveLogger( &changer );
    logManager.RemoveLogger( &added );

    // the added logger got the messages from the one that added it on (seq nr 2 or 3 to 14), the removed one never
    // got one after its removal
    if ( 2 != changer.GetChanges() || added.GetCaseMessages() < 12 || added.GetCaseMessages() > 13 )
    {
        failures |= LOGGERCHANGE_REENTRANT_CHANGE;
        lcCase.notes += CORE::CString( "; round " ) + ( useLoggingThread ? "with" : "without" ) + " logging thread: changes "
            + CORE::ToString( changer.GetChanges() ) + ", calls of the added logger " + CORE::ToString( added.GetCaseMessages() );
    }
    if ( 0 != addedCallsAfterRemoval.Load() )
        failures |= LOGGERCHANGE_CALL_AFTER_REMOVAL;
    return failures;
}

/*-------------------------------------------------------------------------*/

static int
RunLoggerChangeReentrantCase( void )
{
    SLoggerChangeCase* lcCase = GUCEF_NEW SLoggerChangeCase();
    lcCase->logManager = &CORE::CCoreGlobal::Instance()->GetLogManager();
    lcCase->logManager->FlushLogs();
    lcCase->checkLogger.RegisterTestThread();
    lcCase->logManager->AddLogger( &lcCase->checkLogger );

    int failures = RunLoggerChangeReentrantRound( *lcCase, 0, true );
    failures |= RunLoggerChangeReentrantRound( *lcCase, 1, false );

    if ( 0 != lcCase->checkLogger.GetOutOfOrder() )
        failures |= LOGGERCHANGE_ORDER;
    if ( 0 != lcCase->checkLogger.GetConcurrentCalls() )
        failures |= LOGGERCHANGE_CONCURRENT_CALLS;
    if ( 0 != lcCase->callsAfterRemoval.Load() )
        failures |= LOGGERCHANGE_CALL_AFTER_REMOVAL;

    // the round with the logging thread was delivered by it, the other one here
    if ( 15 != lcCase->checkLogger.GetDeliveredByLoggingThread() )
        failures |= LOGGERCHANGE_SETUP;

    lcCase->logManager->RemoveLogger( &lcCase->checkLogger );
    ReportLoggerChangeCase( "reentrant", failures, *lcCase, 30 );
    GUCEF_DELETE lcCase;
    return failures;
}

/*-------------------------------------------------------------------------*/

static int RunLoggerChangeBenchCase( void );

/*-------------------------------------------------------------------------*/

int
RunLoggerChangeCase( const char* caseName )
{GUCEF_TRACE;

    if ( 0 == strcmp( caseName, "race" ) )
        return RunLoggerChangeThreadsCase( caseName, false );
    if ( 0 == strcmp( caseName, "stop" ) )
        return RunLoggerChangeThreadsCase( caseName, true );
    if ( 0 == strcmp( caseName, "reentrant" ) )
        return RunLoggerChangeReentrantCase();
    if ( 0 == strcmp( caseName, "bench" ) )
        return RunLoggerChangeBenchCase();
    return LOGGERCHANGE_SETUP;
}

/*-------------------------------------------------------------------------*/

/** A logger that does nothing but count, the cheapest logger there is */
class CLoggerChangeNullLogger : public CLoggerChangeLoggerBase
{
    public:

    CLoggerChangeNullLogger( void )
        : CLoggerChangeLoggerBase()
        , m_calls( 0 )
    {}

    using CLoggerChangeLoggerBase::Log;

    /** without the conversion to text of the base: only the delivery is measured */
    virtual void Log( const CORE::TLogMsgType /*logMsgType*/     ,
                      const Int32 /*logLevel*/                   ,
                      const CORE::CVariantStream& /*logMessage*/ ,
                      const TSystemThreadId /*threadId*/         ,
                      const CORE::CTimestamp& /*timestamp*/      ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++m_calls;
    }

    UInt32 GetCalls( void ) const { return m_calls; }

    protected:

    virtual void OnMessage( const CORE::CString& /*message*/ ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++m_calls;
    }

    private:

    UInt32 m_calls;
};

/*-------------------------------------------------------------------------*/

/**
 *  A logger on the logging thread that measures the time between two messages of the same batch: what the logging
 *  thread spends per message besides the logger itself (taking the message, the delivery to the loggers, its deletion)
 */
class CLoggerChangeGapLogger : public CLoggerChangeLoggerBase
{
    public:

    CLoggerChangeGapLogger( void )
        : CLoggerChangeLoggerBase()
        , m_calls( 0 )
        , m_lastTicks( 0 )
        , m_gapTicks( 0 )
        , m_gaps( 0 )
        , m_maxGapTicks( MT::PrecisionTimerResolution() / 10000 )      /* 100 us: longer is a wait between batches */
    {}

    UInt32 GetCalls( void ) const { return m_calls.Load( MT::MEMORDER_ACQUIRE ); }

    /** The mean time between two messages of a batch in ns, read once GetCalls() returned all */
    Float64 GetMeanGapInNs( void ) const
    {
        if ( 0 == m_gaps )
            return 0.0;
        return ( (Float64) m_gapTicks * 1000000000.0 / (Float64) MT::PrecisionTimerResolution() ) / (Float64) m_gaps;
    }

    protected:

    virtual void OnMessage( const CORE::CString& /*message*/ ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt64 nowTicks = MT::PrecisionTickCount();
        if ( 0 != m_lastTicks && ( nowTicks - m_lastTicks ) < m_maxGapTicks )
        {
            m_gapTicks += nowTicks - m_lastTicks;
            ++m_gaps;
        }
        m_lastTicks = nowTicks;
        m_calls.FetchAdd( 1 );
    }

    private:

    MT::CTAtomic< UInt32 > m_calls;
    UInt64 m_lastTicks;
    UInt64 m_gapTicks;
    UInt64 m_gaps;
    UInt64 m_maxGapTicks;
};

/*-------------------------------------------------------------------------*/

static Float64
TicksToNsPer( UInt64 ticks, UInt32 count )
{
    return ( (Float64) ticks * 1000000000.0 / (Float64) MT::PrecisionTimerResolution() ) / (Float64) count;
}

/*-------------------------------------------------------------------------*/

/** Logs the cost per message of CMultiLogger::Log() with the given number of loggers; false when a message was lost */
static bool
MeasureMultiLoggerDelivery( UInt32 loggerCount )
{
    const UInt32 messages = 200000;
    const UInt32 runs = 5;
    CORE::CMultiLogger multiLogger;
    multiLogger.SetMinimalLogLevel( CORE::LOGLEVEL_EVERYTHING );
    CLoggerChangeNullLogger nullLoggers[ 4 ];
    if ( loggerCount > 4 )
        return false;
    for ( UInt32 i=0; i<loggerCount; ++i )
        multiLogger.AddLogger( &nullLoggers[ i ] );

    const CORE::CString text( "a log message of an ordinary length, with a number 12345" );
    CORE::CVariantStream stream;
    stream << text << 12345;
    const CORE::CTimestamp timestamp = CORE::CTimestamp::NowUTCTime();

    // the cheapest of the runs: other work on the machine only ever adds
    Float64 bestString = 0.0;
    Float64 bestStream = 0.0;
    for ( UInt32 run=0; run<runs; ++run )
    {
        UInt64 startTicks = MT::PrecisionTickCount();
        for ( UInt32 m=0; m<messages; ++m )
            multiLogger.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, text, 1, timestamp );
        Float64 perMessage = TicksToNsPer( MT::PrecisionTickCount() - startTicks, messages );
        if ( 0 == run || perMessage < bestString )
            bestString = perMessage;

        startTicks = MT::PrecisionTickCount();
        for ( UInt32 m=0; m<messages; ++m )
            multiLogger.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, stream, 1, timestamp );
        perMessage = TicksToNsPer( MT::PrecisionTickCount() - startTicks, messages );
        if ( 0 == run || perMessage < bestStream )
            bestStream = perMessage;
    }
    GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "LogManagerLoggerChanges: CMultiLogger with " << loggerCount << " loggers: "
        << bestString << " ns per text message, " << bestStream << " ns per stream message" );

    for ( UInt32 i=0; i<loggerCount; ++i )
        multiLogger.RemoveLogger( &nullLoggers[ i ] );

    // every logger got every message of every run
    for ( UInt32 i=0; i<loggerCount; ++i )
        if ( nullLoggers[ i ].GetCalls() != 2 * runs * messages )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Logs the cost per message of a logging thread (CLoggingTask with a CMultiLogger as its backend): handing a message
 *  over on the thread that logs, and the time on the logging thread from one logger call to the next within a batch.
 *  False when not every message arrived
 */
static bool
MeasureLoggingTask( void )
{
    const UInt32 messages = 20000;
    CORE::CMultiLogger multiLogger;
    multiLogger.SetMinimalLogLevel( CORE::LOGLEVEL_EVERYTHING );
    CLoggerChangeGapLogger gapLogger;
    multiLogger.AddLogger( &gapLogger );

    CORE::CLoggingTaskPtr loggingTask( GUCEF_NEW CORE::CLoggingTask( multiLogger ) );
    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();
    if ( !threadPool->StartTaskWithConsumer( loggingTask ).HasAFuture() )
        return false;

    const CORE::CString text( "a log message of an ordinary length, with a number 12345" );
    const CORE::CTimestamp timestamp = CORE::CTimestamp::NowUTCTime();
    const UInt64 startTicks = MT::PrecisionTickCount();
    for ( UInt32 m=0; m<messages; ++m )
        loggingTask->Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, text, 1, timestamp );
    const Float64 handOver = TicksToNsPer( MT::PrecisionTickCount() - startTicks, messages );

    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    const UInt64 waitStartTicks = MT::PrecisionTickCount();
    while ( gapLogger.GetCalls() < messages && ( MT::PrecisionTickCount() - waitStartTicks ) < LoggerChangeWaitInMs * ticksPerMs )
        MT::ThreadDelay( 5 );
    const bool allDelivered = gapLogger.GetCalls() == messages;

    // the task ends before the loggers it delivers to go
    const bool stopped = threadPool->RequestTaskToStop( loggingTask->GetCurrentTaskId(), true );

    GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "LogManagerLoggerChanges: CLoggingTask: " << handOver
        << " ns per message handed over, " << gapLogger.GetMeanGapInNs() << " ns per message on the logging thread from one logger call to the next" );
    return allDelivered && stopped;
}

/*-------------------------------------------------------------------------*/

/** "bench": the measurements of the test cases, written to a log file of their own */
static int
RunLoggerChangeBenchCase( void )
{
    CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
    CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( logFilename, "gucefCORE_TestApp_LoggerChange_bench.txt" );
    CORE::CFileAccess logFileAccess( logFilename, "a" );
    CORE::CStdLogger logger( logFileAccess );
    logger.SetMinimalLogLevel( CORE::LOGLEVEL_CRITICAL );
    logManager.AddLogger( &logger );

    int failures = 0;
    if ( !MeasureMultiLoggerDelivery( 1 ) || !MeasureMultiLoggerDelivery( 4 ) || !MeasureLoggingTask() )
        failures |= LOGGERCHANGE_LOST;

    // the cost of the rare path: a logger added and removed while the logging thread runs and has nothing to deliver
    logManager.SetUseLoggingThread( true );
    if ( !logManager.GetUseLoggingThread() )
        failures |= LOGGERCHANGE_SETUP;
    MT::ThreadDelay( 100 );
    CLoggerChangeNullLogger nullLogger;
    const UInt32 rounds = 200;
    const UInt64 startTicks = MT::PrecisionTickCount();
    for ( UInt32 r=0; r<rounds; ++r )
    {
        logManager.AddLogger( &nullLogger );
        logManager.RemoveLogger( &nullLogger );
    }
    const Float64 perChange = TicksToNsPer( MT::PrecisionTickCount() - startTicks, 2 * rounds ) / 1000.0;
    logManager.SetUseLoggingThread( false );
    GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "LogManagerLoggerChanges: " << perChange << " us per logger added or removed with the logging thread" );

    CORE::CLogStreamScope::FlushLogs();
    logManager.RemoveLogger( &logger );
    return failures;
}

/*-------------------------------------------------------------------------*/

void
PerformLogManagerLoggerChangeTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING log manager logger change TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "LogManagerLoggerChanges" );

    #ifdef GUCEF_LINUX_BUILD

    // Each case runs in a process of its own: a log manager with only the loggers of the case, and a failure that
    // crashes or hangs cannot take the logging of this process with it
    const char* caseNames[] = { "race", "stop", "reentrant" };
    for ( UInt32 c=0; c<3; ++c )
    {
        const char* caseName = caseNames[ c ];
        const CORE::CString caseTitle = CORE::CString( "Loggers added and removed while the log is delivered, case " ) + caseName + ": every message once and in order, no logger called after its removal";
        GUCEF_TESTFW_TESTCASE( caseTitle.C_String() )
            try
            {
                char exePath[ 4096 ];
                ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
                ASSERT_TRUE( exePathLength > 0 );
                if ( exePathLength > 0 )
                {
                    exePath[ exePathLength ] = '\0';

                    CORE::CProcess::TStringVector args;
                    args.push_back( GUCEF_CORE_TESTAPP_LOGGER_CHANGE_CASE_ARG );
                    args.push_back( caseName );
                    CORE::CProcess child;
                    child.SetIsolateFromParent( true );
                    ASSERT_TRUE( child.Launch( exePath, args, CORE::CString(), TESTFW::ChildEnvironmentWithOwnExitCode() ) );

                    Int32 exitCode = -1;
                    const bool ended = child.WaitForExit( 600000, exitCode );
                    if ( !ended )
                        child.Terminate( false );
                    if ( 0 != exitCode )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Logger change case " << caseName << " ended with " << exitCode );
                    ASSERT_TRUE( ended );
                    ASSERT_TRUE( 0 == exitCode );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }

    #endif /* GUCEF_LINUX_BUILD ? */

    GUCEF_TESTFW_TESTCASE( "Cost per message of the delivery to the loggers (CMultiLogger), 1 and 4 loggers" )
        try
        {
            ASSERT_TRUE( MeasureMultiLoggerDelivery( 1 ) );
            ASSERT_TRUE( MeasureMultiLoggerDelivery( 4 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cost per message of the logging thread (CLoggingTask): handing a message over, and its delivery on that thread" )
        try
        {
            ASSERT_TRUE( MeasureLoggingTask() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL log manager logger change TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
