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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CUSERACTIONLOGGER_H
#include "gucefCORE_CUserActionLogger.h"
#define GUCEF_CORE_CUSERACTIONLOGGER_H
#endif /* GUCEF_CORE_CUSERACTIONLOGGER_H ? */

#ifndef GUCEF_CORE_CSTDLOGGER_H
#include "CStdLogger.h"
#define GUCEF_CORE_CSTDLOGGER_H
#endif /* GUCEF_CORE_CSTDLOGGER_H ? */

#ifndef GUCEF_CORE_CMULTILOGGER_H
#include "gucefCORE_CMultiLogger.h"
#define GUCEF_CORE_CMULTILOGGER_H
#endif /* GUCEF_CORE_CMULTILOGGER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestLogging.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Simple capture logger for verifying what the logging backend receives.
 *  Implements CILogger and stores all log calls in a vector for inspection.
 */
class CTestCaptureLogger : public CORE::CILogger
{
    public:

    struct SEntry
    {
        CORE::TLogMsgType logMsgType;
        Int32 logLevel;
        CORE::CString logMessage;
        UInt32 threadId;
        bool wasFormatted;
    };

    GUCEF::vector< SEntry > m_entries;
    MT::CMutex m_lock;                  /**< loggers can be invoked from multiple threads */
    Int32 m_minLogLevel;

    CTestCaptureLogger( void )
        : CORE::CILogger()
        , m_entries()
        , m_minLogLevel( CORE::LOGLEVEL_EVERYTHING )
    {}

    virtual ~CTestCaptureLogger( void )
    {}

    virtual void Log( const CORE::TLogMsgType logMsgType ,
                      const Int32 logLevel               ,
                      const CORE::CString& logMessage    ,
                      const UInt32 threadId              ,
                      const CORE::CTimestamp& /*timestamp*/ ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( logLevel >= m_minLogLevel )
        {
            MT::CScopeMutex lock( m_lock );
            SEntry entry;
            entry.logMsgType = logMsgType;
            entry.logLevel = logLevel;
            entry.logMessage = logMessage;
            entry.threadId = threadId;
            entry.wasFormatted = true;
            m_entries.push_back( entry );
        }
    }

    virtual void Log( const CORE::TLogMsgType logMsgType     ,
                      const Int32 logLevel                   ,
                      const CORE::CVariantStream& logMessage ,
                      const UInt32 threadId                  ,
                      const CORE::CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType ,
                                       const Int32 logLevel               ,
                                       const CORE::CString& logMessage    ,
                                       const UInt32 threadId              ,
                                       const CORE::CTimestamp& /*timestamp*/ ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( logLevel >= m_minLogLevel )
        {
            MT::CScopeMutex lock( m_lock );
            SEntry entry;
            entry.logMsgType = logMsgType;
            entry.logLevel = logLevel;
            entry.logMessage = logMessage;
            entry.threadId = threadId;
            entry.wasFormatted = false;
            m_entries.push_back( entry );
        }
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType     ,
                                       const Int32 logLevel                   ,
                                       const CORE::CVariantStream& logMessage ,
                                       const UInt32 threadId                  ,
                                       const CORE::CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE
    {
        LogWithoutFormatting( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE
    {}

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_minLogLevel = logLevel;
    }

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_minLogLevel;
    }

    void Clear( void )
    {
        m_entries.clear();
    }

    bool HasEntryWith( const CORE::CString& substr ) const
    {
        for ( size_t i=0; i<m_entries.size(); ++i )
        {
            if ( m_entries[ i ].logMessage.HasSubstr( substr ) >= 0 )
                return true;
        }
        return false;
    }

    bool HasEntryOfType( const CORE::TLogMsgType msgType ) const
    {
        for ( size_t i=0; i<m_entries.size(); ++i )
        {
            if ( m_entries[ i ].logMsgType == msgType )
                return true;
        }
        return false;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Drains any segments that have accumulated in the current thread's front buffer
 *  into the currently registered loggers. Used to get a clean slate before adding
 *  a capture logger so that pre-existing buffered entries do not pollute results.
 */
static void
PreDrainThreadBuffer( void )
{
    CORE::CLogStreamScope::FlushLogs();
}

/*-------------------------------------------------------------------------*/

/**
 *  Counts the captured entries whose message contains the given substring
 *  and verifies the entries that do are attributed to the expected thread
 */
static UInt32
CountEntriesWith( CTestCaptureLogger& logger         ,
                  const CORE::CString& substr       ,
                  UInt32 expectedThreadId           ,
                  UInt32& entriesWithWrongThreadId  )
{
    MT::CScopeMutex lock( logger.m_lock );
    UInt32 count = 0;
    entriesWithWrongThreadId = 0;
    for ( size_t i=0; i<logger.m_entries.size(); ++i )
    {
        if ( logger.m_entries[ i ].logMessage.HasSubstr( substr ) >= 0 )
        {
            ++count;
            if ( logger.m_entries[ i ].threadId != expectedThreadId )
                ++entriesWithWrongThreadId;
        }
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/**
 *  Counts the lines in the given text which contain the given substring
 */
static UInt32
CountLinesWith( const CORE::CString& text   ,
                const CORE::CString& substr )
{
    UInt32 count = 0;
    CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        if ( lines[ i ].HasSubstr( substr ) >= 0 )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

static const UInt32 LoggingTestNrOfWorkerThreads = 4;
static const UInt32 LoggingTestNrOfMsgsPerThread = 200;

/**
 *  Worker thread which logs a fixed set of messages and then finishes
 */
class CLoggingTestWorker : public MT::CActiveObject
{
    public:

    UInt32 workerIndex;
    UInt32 threadId;     /**< set by the worker thread itself */

    CLoggingTestWorker( UInt32 index )
        : MT::CActiveObject()
        , workerIndex( index )
        , threadId( 0 )
    {}

    virtual ~CLoggingTestWorker()
    {}

    /** blocks until the worker thread finished or the timeout expired, returns whether it finished */
    bool WaitForCompletion( UInt32 timeoutInMs )
    {
        UInt32 waitedInMs = 0;
        while ( IsActive() && waitedInMs < timeoutInMs )
        {
            MT::ThreadDelay( 5 );
            waitedInMs += 5;
        }
        return !IsActive();
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return this;
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        threadId = MT::GetCurrentTaskID();
        CORE::CString prefix = "MTLogTest-Worker" + CORE::ToString( workerIndex ) + "-Msg";
        for ( UInt32 i=0; i<LoggingTestNrOfMsgsPerThread; ++i )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, prefix + CORE::ToString( i ) + "-End" );
        }
        return true; // done, the thread ends
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Thread which does one thing for the log manager and then finishes: log a message, or flush the logs
 */
class CLoggingTestOneShot : public MT::CActiveObject
{
    public:

    enum EAction
    {
        ACTION_LOG   ,
        ACTION_FLUSH
    };

    CLoggingTestOneShot( EAction action, const CORE::CString& message )
        : MT::CActiveObject()
        , m_action( action )
        , m_message( message )
    {}

    virtual ~CLoggingTestOneShot()
    {}

    /** blocks until the thread finished or the timeout expired, returns whether it finished */
    bool WaitForCompletion( UInt32 timeoutInMs )
    {
        UInt32 waitedInMs = 0;
        while ( IsActive() && waitedInMs < timeoutInMs )
        {
            MT::ThreadDelay( 5 );
            waitedInMs += 5;
        }
        return !IsActive();
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return this;
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( ACTION_LOG == m_action )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, m_message );
        }
        else
        {
            CORE::CLogStreamScope::FlushLogs();
        }
        return true; // done, the thread ends
    }

    private:

    EAction m_action;
    CORE::CString m_message;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

/** Counts how often the argument of a user log message was built: a disabled message must not build it */
static UInt32 g_userLogArgumentEvaluations = 0;

static CORE::CString
UserLogExpensiveArgument( void )
{
    ++g_userLogArgumentEvaluations;
    return CORE::CString( "an argument that is expensive to build and that nobody reads when the user log is off" );
}

/*-------------------------------------------------------------------------*/

/**
 *  Thread which logs a number of user messages and then finishes
 */
class CUserLogSwitchWorker : public MT::CActiveObject
{
    public:

    explicit CUserLogSwitchWorker( UInt32 nrOfMessages )
        : MT::CActiveObject()
        , m_nrOfMessages( nrOfMessages )
    {}

    virtual ~CUserLogSwitchWorker()
    {}

    bool WaitForCompletion( UInt32 timeoutInMs )
    {
        UInt32 waitedInMs = 0;
        while ( IsActive() && waitedInMs < timeoutInMs )
        {
            MT::ThreadDelay( 5 );
            waitedInMs += 5;
        }
        return !IsActive();
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return this;
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<m_nrOfMessages; ++i )
        {
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "UserLogSwitchWorkerMessage " << UserLogExpensiveArgument() );
        }
        return true; // done, the thread ends
    }

    private:

    UInt32 m_nrOfMessages;
};

/*-------------------------------------------------------------------------*/

static bool
RunUserLogSwitchWorker( UInt32 nrOfMessages )
{
    CUserLogSwitchWorker* worker = GUCEF_NEW CUserLogSwitchWorker( nrOfMessages );
    if ( !worker->Activate() )
    {
        GUCEF_DELETE worker;
        return false;
    }
    bool done = worker->WaitForCompletion( 10000 );
    if ( done )
        GUCEF_DELETE worker;
    return done;
}

void
PerformLoggingTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING logging backend TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CLogging" );

    // Test 1: CStdLogger construction and min log level get/set
    GUCEF_TESTFW_TESTCASE( "Test 1: CStdLogger construction and min log level" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: CStdLogger construction and min log level" );
        CORE::CStdLogger stdLogger;
        ASSERT_TRUE( stdLogger.GetMinimalLogLevel() == CORE::LOGLEVEL_BELOW_NORMAL );

        stdLogger.SetMinimalLogLevel( CORE::LOGLEVEL_NORMAL );
        ASSERT_TRUE( stdLogger.GetMinimalLogLevel() == CORE::LOGLEVEL_NORMAL );

        stdLogger.SetMinimalLogLevel( CORE::LOGLEVEL_CRITICAL );
        ASSERT_TRUE( stdLogger.GetMinimalLogLevel() == CORE::LOGLEVEL_CRITICAL );

        stdLogger.SetMinimalLogLevel( CORE::LOGLEVEL_EVERYTHING );
        ASSERT_TRUE( stdLogger.GetMinimalLogLevel() == CORE::LOGLEVEL_EVERYTHING );
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: CStdLogger string Log writes formatted output to CIOAccess
    GUCEF_TESTFW_TESTCASE( "Test 2: CStdLogger string Log writes to CIOAccess" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: CStdLogger string Log writes to CIOAccess" );
        CORE::CDynamicBuffer outputBuffer;
        CORE::CDynamicBufferAccess outputAccess( outputBuffer );
        outputAccess.Open();
        CORE::CStdLogger stdLogger( outputAccess );
        stdLogger.SetMinimalLogLevel( CORE::LOGLEVEL_EVERYTHING );

        CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
        stdLogger.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, "TestStringLogMessage", 1, ts );

        ASSERT_TRUE( outputBuffer.GetDataSize() > 0 );
        CORE::CString logOutput( outputBuffer.AsConstTypePtr< char >(), outputBuffer.GetDataSize() );
        ASSERT_TRUE( logOutput.HasSubstr( "TestStringLogMessage" ) >= 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: CStdLogger CVariantStream Log converts stream to string and writes it
    GUCEF_TESTFW_TESTCASE( "Test 3: CStdLogger CVariantStream Log" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: CStdLogger CVariantStream Log" );
        CORE::CDynamicBuffer outputBuffer;
        CORE::CDynamicBufferAccess outputAccess( outputBuffer );
        outputAccess.Open();
        CORE::CStdLogger stdLogger( outputAccess );
        stdLogger.SetMinimalLogLevel( CORE::LOGLEVEL_EVERYTHING );

        CORE::CVariantStream logStream;
        logStream << "StreamedMsg:" << (Int32) 42;

        CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
        stdLogger.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, logStream, 1, ts );

        ASSERT_TRUE( outputBuffer.GetDataSize() > 0 );
        CORE::CString logOutput( outputBuffer.AsConstTypePtr< char >(), outputBuffer.GetDataSize() );
        ASSERT_TRUE( logOutput.HasSubstr( "StreamedMsg:" ) >= 0 );
        ASSERT_TRUE( logOutput.HasSubstr( "42" ) >= 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: CStdLogger LogWithoutFormatting writes message directly without metadata prefix
    GUCEF_TESTFW_TESTCASE( "Test 4: CStdLogger LogWithoutFormatting string" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: CStdLogger LogWithoutFormatting string" );
        CORE::CDynamicBuffer outputBuffer;
        CORE::CDynamicBufferAccess outputAccess( outputBuffer );
        outputAccess.Open();
        CORE::CStdLogger stdLogger( outputAccess );
        stdLogger.SetMinimalLogLevel( CORE::LOGLEVEL_EVERYTHING );

        CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
        stdLogger.LogWithoutFormatting( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, "PlainMessage", 1, ts );

        ASSERT_TRUE( outputBuffer.GetDataSize() > 0 );
        CORE::CString logOutput( outputBuffer.AsConstTypePtr< char >(), outputBuffer.GetDataSize() );
        ASSERT_TRUE( logOutput.HasSubstr( "PlainMessage" ) >= 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: CStdLogger CVariantStream LogWithoutFormatting converts and writes without metadata
    GUCEF_TESTFW_TESTCASE( "Test 5: CStdLogger CVariantStream LogWithoutFormatting" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: CStdLogger CVariantStream LogWithoutFormatting" );
        CORE::CDynamicBuffer outputBuffer;
        CORE::CDynamicBufferAccess outputAccess( outputBuffer );
        outputAccess.Open();
        CORE::CStdLogger stdLogger( outputAccess );
        stdLogger.SetMinimalLogLevel( CORE::LOGLEVEL_EVERYTHING );

        CORE::CVariantStream logStream;
        logStream << "PlainStreamMsg";

        CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
        stdLogger.LogWithoutFormatting( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, logStream, 1, ts );

        ASSERT_TRUE( outputBuffer.GetDataSize() > 0 );
        CORE::CString logOutput( outputBuffer.AsConstTypePtr< char >(), outputBuffer.GetDataSize() );
        ASSERT_TRUE( logOutput.HasSubstr( "PlainStreamMsg" ) >= 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: CStdLogger min level filtering - messages below the threshold are dropped
    GUCEF_TESTFW_TESTCASE( "Test 6: CStdLogger min level filtering" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: CStdLogger min level filtering" );
        CORE::CDynamicBuffer outputBuffer;
        CORE::CDynamicBufferAccess outputAccess( outputBuffer );
        outputAccess.Open();
        CORE::CStdLogger stdLogger( outputAccess );
        stdLogger.SetMinimalLogLevel( CORE::LOGLEVEL_NORMAL );

        CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();

        // LOGLEVEL_EVERYTHING (0) < LOGLEVEL_NORMAL (50000) - should be filtered
        stdLogger.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_EVERYTHING, "FilteredMessage", 1, ts );
        ASSERT_TRUE( outputBuffer.GetDataSize() == 0 );

        // LOGLEVEL_CRITICAL >> LOGLEVEL_NORMAL - should pass
        stdLogger.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_CRITICAL, "PassedMessage", 1, ts );
        ASSERT_TRUE( outputBuffer.GetDataSize() > 0 );

        CORE::CString logOutput( outputBuffer.AsConstTypePtr< char >(), outputBuffer.GetDataSize() );
        ASSERT_FALSE( logOutput.HasSubstr( "FilteredMessage" ) >= 0 );
        ASSERT_TRUE( logOutput.HasSubstr( "PassedMessage" ) >= 0 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: CLogManager direct string logging dispatches to registered capture logger
    GUCEF_TESTFW_TESTCASE( "Test 7: CLogManager direct string logging" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: CLogManager direct string logging" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
        logMgr.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, "DirectStringMessage", 1, ts );

        ASSERT_TRUE( captureLogger.HasEntryWith( "DirectStringMessage" ) );
        ASSERT_TRUE( captureLogger.HasEntryOfType( CORE::LOG_STANDARD ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: CLogManager direct CVariantStream logging dispatches correctly
    GUCEF_TESTFW_TESTCASE( "Test 8: CLogManager direct CVariantStream logging" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: CLogManager direct CVariantStream logging" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        CORE::CVariantStream logStream;
        logStream << "DirectStreamMsg:" << (Int32) 99;

        CORE::CTimestamp ts = CORE::CTimestamp::NowUTCTime();
        logMgr.Log( CORE::LOG_WARNING, CORE::LOGLEVEL_NORMAL, logStream, 1, ts );

        ASSERT_TRUE( captureLogger.HasEntryWith( "DirectStreamMsg:" ) );
        ASSERT_TRUE( captureLogger.HasEntryOfType( CORE::LOG_WARNING ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: CLogManager streaming API - Log() returns stream, data arrives after flush
    // This tests the core streaming path: thread buffer -> FlushThreadStreamBuffers -> loggers
    GUCEF_TESTFW_TESTCASE( "Test 9: CLogManager streaming API via thread buffer" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: CLogManager streaming API via thread buffer" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        {
            // Get the thread's front buffer for this log entry
            CORE::CLogStreamPtr stream = logMgr.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL );
            ASSERT_FALSE( stream.IsNULL() );
            if ( !stream.IsNULL() )
            {
                (*stream) << "StreamingApiMessage";
                stream->WriteSegmentEnd();
            }
        } // stream ref released here; front buffer ref count drops back to 1

        // Note: In single-threaded apps WriteSegmentEnd() auto-flushes, so the message
        // may already be dispatched here. We just ensure it arrives (after explicit flush too).
        logMgr.FlushLogs();

        ASSERT_TRUE( captureLogger.HasEntryWith( "StreamingApiMessage" ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: CLogStreamScope RAII - segment end written on destruction, message arrives after flush
    GUCEF_TESTFW_TESTCASE( "Test 10: CLogStreamScope RAII segment marking" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: CLogStreamScope RAII segment marking" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        {
            CORE::CLogStreamScope scope( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL );
            scope << "ScopeTestMessage";
        } // WriteSegmentEnd() called automatically in ~CLogStreamScope

        CORE::CLogStreamScope::FlushLogs();

        ASSERT_TRUE( captureLogger.HasEntryWith( "ScopeTestMessage" ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: CLogStreamScope with typed mixed data arrives correctly after flush
    GUCEF_TESTFW_TESTCASE( "Test 11: CLogStreamScope mixed typed data" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: CLogStreamScope mixed typed data" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        {
            CORE::CLogStreamScope scope( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL );
            scope << "key=" << (Int32) 42 << " val=" << (Float64) 3.14;
        }

        CORE::CLogStreamScope::FlushLogs();

        ASSERT_TRUE( captureLogger.HasEntryWith( "key=42" ) );
        ASSERT_TRUE( captureLogger.HasEntryWith( "3.14" ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: GUCEF_LOG macro delivers message to capture logger after flush
    GUCEF_TESTFW_TESTCASE( "Test 12: GUCEF_LOG macro through streaming path" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: GUCEF_LOG macro through streaming path" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MacroLogTestMessage" );

        CORE::CLogStreamScope::FlushLogs();

        ASSERT_TRUE( captureLogger.HasEntryWith( "MacroLogTestMessage" ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: GUCEF_ERROR_LOG macro delivers correct message type after flush
    GUCEF_TESTFW_TESTCASE( "Test 13: GUCEF_ERROR_LOG macro message type" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: GUCEF_ERROR_LOG macro message type" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ErrorMacroTestMessage" );

        CORE::CLogStreamScope::FlushLogs();

        ASSERT_TRUE( captureLogger.HasEntryWith( "ErrorMacroTestMessage" ) );
        ASSERT_TRUE( captureLogger.HasEntryOfType( CORE::LOG_ERROR ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: GUCEF_WARNING_LOG macro delivers correct message type after flush
    GUCEF_TESTFW_TESTCASE( "Test 14: GUCEF_WARNING_LOG macro message type" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: GUCEF_WARNING_LOG macro message type" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WarningMacroTestMessage" );

        CORE::CLogStreamScope::FlushLogs();

        ASSERT_TRUE( captureLogger.HasEntryWith( "WarningMacroTestMessage" ) );
        ASSERT_TRUE( captureLogger.HasEntryOfType( CORE::LOG_WARNING ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: Min level filtering in the streaming path
    // Segments are always written to the thread buffer but filtered at dispatch time
    GUCEF_TESTFW_TESTCASE( "Test 15: Streaming path min level filtering" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: Streaming path min level filtering" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        captureLogger.SetMinimalLogLevel( CORE::LOGLEVEL_NORMAL );
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        // LOGLEVEL_EVERYTHING (0) < LOGLEVEL_NORMAL (50000) - should be filtered by capture logger
        {
            CORE::CLogStreamScope scope( CORE::LOG_STANDARD, CORE::LOGLEVEL_EVERYTHING );
            scope << "FilteredStreamMessage";
        }

        CORE::CLogStreamScope::FlushLogs();
        ASSERT_FALSE( captureLogger.HasEntryWith( "FilteredStreamMessage" ) );

        // LOGLEVEL_CRITICAL >> LOGLEVEL_NORMAL - should be dispatched
        {
            CORE::CLogStreamScope scope( CORE::LOG_STANDARD, CORE::LOGLEVEL_CRITICAL );
            scope << "PassedStreamMessage";
        }

        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( captureLogger.HasEntryWith( "PassedStreamMessage" ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: Multiple segments in a single flush cycle all arrive
    GUCEF_TESTFW_TESTCASE( "Test 16: Multiple segments per flush cycle" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: Multiple segments per flush cycle" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        {
            CORE::CLogStreamScope scope1( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL );
            scope1 << "MultiSegment1";
        }
        {
            CORE::CLogStreamScope scope2( CORE::LOG_WARNING, CORE::LOGLEVEL_NORMAL );
            scope2 << "MultiSegment2";
        }
        {
            CORE::CLogStreamScope scope3( CORE::LOG_ERROR, CORE::LOGLEVEL_NORMAL );
            scope3 << "MultiSegment3";
        }

        CORE::CLogStreamScope::FlushLogs();

        ASSERT_TRUE( captureLogger.HasEntryWith( "MultiSegment1" ) );
        ASSERT_TRUE( captureLogger.HasEntryWith( "MultiSegment2" ) );
        ASSERT_TRUE( captureLogger.HasEntryWith( "MultiSegment3" ) );
        // Check message types were preserved through the segment metadata
        ASSERT_TRUE( captureLogger.HasEntryOfType( CORE::LOG_STANDARD ) );
        ASSERT_TRUE( captureLogger.HasEntryOfType( CORE::LOG_WARNING ) );
        ASSERT_TRUE( captureLogger.HasEntryOfType( CORE::LOG_ERROR ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: CMultiLogger dispatches to all registered backends
    GUCEF_TESTFW_TESTCASE( "Test 17: CMultiLogger dispatches to multiple backends" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: CMultiLogger dispatches to multiple backends" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger1;
        CTestCaptureLogger captureLogger2;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger1 );
        logMgr.AddLogger( &captureLogger2 );

        {
            CORE::CLogStreamScope scope( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL );
            scope << "MultiBackendMessage";
        }

        CORE::CLogStreamScope::FlushLogs();

        ASSERT_TRUE( captureLogger1.HasEntryWith( "MultiBackendMessage" ) );
        ASSERT_TRUE( captureLogger2.HasEntryWith( "MultiBackendMessage" ) );

        logMgr.RemoveLogger( &captureLogger1 );
        logMgr.RemoveLogger( &captureLogger2 );
    GUCEF_TESTFW_TESTCASE_END

    // Test 17b: CMultiLogger says a message is logged exactly when it logs it
    GUCEF_TESTFW_TESTCASE( "Test 17b: CMultiLogger tells which messages it logs as it logs them" )
        CTestCaptureLogger capture;
        CORE::CMultiLogger multi;
        multi.AddLogger( &capture );
        multi.SetMinimalLogLevel( CORE::LOGLEVEL_NORMAL );

        multi.Log( CORE::LOG_USER, CORE::LOGLEVEL_CRITICAL, "AboveTheMinimum" );
        multi.Log( CORE::LOG_USER, CORE::LOGLEVEL_BELOW_NORMAL, "BelowTheMinimum" );
        ASSERT_TRUE( capture.HasEntryWith( "AboveTheMinimum" ) );
        ASSERT_FALSE( capture.HasEntryWith( "BelowTheMinimum" ) );
        ASSERT_TRUE( multi.IsLoggingEnabled( CORE::LOG_USER, CORE::LOGLEVEL_CRITICAL ) );
        ASSERT_FALSE( multi.IsLoggingEnabled( CORE::LOG_USER, CORE::LOGLEVEL_BELOW_NORMAL ) );
        multi.ClearLoggers();
    GUCEF_TESTFW_TESTCASE_END

    // Test 17c: the user action log has what the user did and nothing else, in a file of its own
    GUCEF_TESTFW_TESTCASE( "Test 17c: The user action log keeps only user actions, in its own file" )
        PreDrainThreadBuffer();
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        const CORE::CString logFile = CORE::RelativePath( "$TEMPDIR$/gucefCORE_TestApp/UserActions.log" );
        CORE::DeleteFile( logFile );
        ASSERT_TRUE( logMgr.SetUserActionLogEnabled( true, logFile ) );
        ASSERT_TRUE( logMgr.IsUserActionLogEnabled() && logFile == logMgr.GetUserActionLogFile() );

        GUCEF_USER_LOG( CORE::LOGLEVEL_CRITICAL, "UserClickedOk" );
        GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "NotAUserAction" );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( logMgr.SetUserActionLogEnabled( false, CORE::CString::Empty ) );
        ASSERT_FALSE( logMgr.IsUserActionLogEnabled() );
        GUCEF_USER_LOG( CORE::LOGLEVEL_CRITICAL, "AfterTheLogWasSwitchedOff" );
        CORE::CLogStreamScope::FlushLogs();

        CORE::CString content;
        ASSERT_TRUE( CORE::LoadTextFileAsString( logFile, content ) );
        ASSERT_TRUE( content.HasSubstr( "a new session" ) >= 0 && content.HasSubstr( "UserClickedOk" ) >= 0 );
        ASSERT_TRUE( content.HasSubstr( "NotAUserAction" ) < 0 && content.HasSubstr( "AfterTheLogWasSwitchedOff" ) < 0 );

        // each action is in the file as soon as it is logged: a program that ends without its shutdown (a crash, killed) keeps them
        CORE::DeleteFile( logFile );
        {
            CORE::CUserActionLogger direct( logFile );
            ASSERT_TRUE( direct.IsOpen() );
            direct.Log( CORE::LOG_USER, CORE::LOGLEVEL_NORMAL, "ClickedBeforeTheCrash", MT::GetCurrentTaskID(), CORE::CTimestamp::NowUTCTime() );
            CORE::CString unflushed;
            ASSERT_TRUE( CORE::LoadTextFileAsString( logFile, unflushed ) );
            ASSERT_TRUE( unflushed.HasSubstr( "a new session" ) >= 0 && unflushed.HasSubstr( "ClickedBeforeTheCrash" ) >= 0 );
        }

        // a file that cannot be written: not switched on
        ASSERT_FALSE( logMgr.SetUserActionLogEnabled( true, "/proc/no/such/dir/UserActions.log" ) );
        ASSERT_FALSE( logMgr.IsUserActionLogEnabled() );
        CORE::DeleteFile( logFile );
    GUCEF_TESTFW_TESTCASE_END

    // Test 17d: the user log is a switch that is looked at before anything of the message is built
    GUCEF_TESTFW_TESTCASE( "Test 17d: GUCEF_USER_LOG disabled builds nothing and reaches no logger" )
        PreDrainThreadBuffer();
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        ASSERT_TRUE( logMgr.IsUserLogEnabled() );       // the default keeps the behaviour of programs that never set it

        CTestCaptureLogger captureLogger;
        logMgr.AddLogger( &captureLogger );

        // on: the message arrives
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "UserLogSwitchOnMessage" );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( captureLogger.HasEntryWith( "UserLogSwitchOnMessage" ) );

        // off: not the user messages, nothing else is touched
        logMgr.SetUserLogEnabled( false );
        ASSERT_FALSE( logMgr.IsUserLogEnabled() );
        g_userLogArgumentEvaluations = 0;
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "UserLogSwitchOffMessage " << UserLogExpensiveArgument() );
        GUCEF_USER_LOG( CORE::LOGLEVEL_CRITICAL, "UserLogSwitchOffMessage " << UserLogExpensiveArgument() )     // no semicolon, as it is used in the code
        GUCEF_USER_LOGSTR( CORE::LOGLEVEL_NORMAL, "UserLogSwitchOffMessage " + UserLogExpensiveArgument() );
        logMgr.Log( CORE::LOG_USER, CORE::LOGLEVEL_NORMAL, CORE::CString( "UserLogSwitchOffMessageDirect" ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UserLogSwitchOffOtherMessage" );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UserLogSwitchOffWarning" );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( 0 == g_userLogArgumentEvaluations );
        ASSERT_FALSE( captureLogger.HasEntryWith( "UserLogSwitchOffMessage" ) );
        ASSERT_TRUE( captureLogger.HasEntryWith( "UserLogSwitchOffOtherMessage" ) );
        ASSERT_TRUE( captureLogger.HasEntryWith( "UserLogSwitchOffWarning" ) );

        // as the body of an if without braces, as the macros are used in code
        if ( 0 == g_userLogArgumentEvaluations )
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "UserLogSwitchOffMessage " << UserLogExpensiveArgument() )
        else
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "UserLogSwitchOffMessage " << UserLogExpensiveArgument() )
        ASSERT_TRUE( 0 == g_userLogArgumentEvaluations );

        // back on, all of it again
        logMgr.SetUserLogEnabled( true );
        ASSERT_TRUE( logMgr.IsUserLogEnabled() );
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "UserLogSwitchOnAgain " << UserLogExpensiveArgument() );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( 1 == g_userLogArgumentEvaluations );
        ASSERT_TRUE( captureLogger.HasEntryWith( "UserLogSwitchOnAgain" ) );

        // the other threads see the switch too
        logMgr.SetUserLogEnabled( false );
        g_userLogArgumentEvaluations = 0;
        ASSERT_TRUE( RunUserLogSwitchWorker( 1000 ) );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( 0 == g_userLogArgumentEvaluations );
        ASSERT_FALSE( captureLogger.HasEntryWith( "UserLogSwitchWorkerMessage" ) );
        logMgr.SetUserLogEnabled( true );
        ASSERT_TRUE( RunUserLogSwitchWorker( 1 ) );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( 1 == g_userLogArgumentEvaluations );
        ASSERT_TRUE( captureLogger.HasEntryWith( "UserLogSwitchWorkerMessage" ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 17e: the cost of a user log message that is switched off: a test of a flag, no message built
    GUCEF_TESTFW_TESTCASE( "Test 17e: A million disabled GUCEF_USER_LOG calls evaluate nothing and cost nanoseconds each" )
        PreDrainThreadBuffer();
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        CTestCaptureLogger captureLogger;
        logMgr.AddLogger( &captureLogger );
        logMgr.SetUserLogEnabled( false );

        const UInt32 nrOfCalls = 1000000;
        g_userLogArgumentEvaluations = 0;
        const UInt64 start = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<nrOfCalls; ++i )
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Expensive to build " << UserLogExpensiveArgument() << " and " << i << " and " << UserLogExpensiveArgument() )
        const UInt64 ticks = MT::PrecisionTickCount() - start;
        const Float64 nsPerCall = ( ticks * 1000000000.0 / MT::PrecisionTimerResolution() ) / nrOfCalls;
        logMgr.SetUserLogEnabled( true );
        CORE::CLogStreamScope::FlushLogs();

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GUCEF_USER_LOG disabled: " << nsPerCall << " ns per call over " << nrOfCalls << " calls" );
        ASSERT_TRUE( 0 == g_userLogArgumentEvaluations );
        ASSERT_FALSE( captureLogger.HasEntryWith( "Expensive to build" ) );
        ASSERT_TRUE( nsPerCall < 50.0 );

        // for comparison: the same message enabled does build the whole thing
        g_userLogArgumentEvaluations = 0;
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Expensive to build " << UserLogExpensiveArgument() );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( 1 == g_userLogArgumentEvaluations );
        ASSERT_TRUE( captureLogger.HasEntryWith( "Expensive to build" ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 17f: a user action log that is switched on has user messages to log
    GUCEF_TESTFW_TESTCASE( "Test 17f: Switching the user action log file on switches the user messages on" )
        PreDrainThreadBuffer();
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        const CORE::CString logFile = CORE::RelativePath( "$TEMPDIR$/gucefCORE_TestApp/UserActionsSwitch.log" );
        CORE::DeleteFile( logFile );
        logMgr.SetUserLogEnabled( false );
        ASSERT_TRUE( logMgr.SetUserActionLogEnabled( true, logFile ) );
        ASSERT_TRUE( logMgr.IsUserLogEnabled() );
        GUCEF_USER_LOG( CORE::LOGLEVEL_CRITICAL, "UserActionAfterTheFileWasSwitchedOn" );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( logMgr.SetUserActionLogEnabled( false, CORE::CString::Empty ) );
        CORE::CString content;
        ASSERT_TRUE( CORE::LoadTextFileAsString( logFile, content ) );
        ASSERT_TRUE( content.HasSubstr( "UserActionAfterTheFileWasSwitchedOn" ) >= 0 );
        CORE::DeleteFile( logFile );
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: Thread ID is correctly captured in the segment metadata
    GUCEF_TESTFW_TESTCASE( "Test 18: Thread ID preserved through streaming path" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: Thread ID preserved through streaming path" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        UInt32 thisThreadId = MT::GetCurrentTaskID();

        {
            CORE::CLogStreamScope scope( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL );
            scope << "ThreadIdCheckMessage";
        }

        CORE::CLogStreamScope::FlushLogs();

        ASSERT_TRUE( captureLogger.HasEntryWith( "ThreadIdCheckMessage" ) );
        // Find the entry and verify threadId matches
        bool foundMatchingThreadId = false;
        for ( size_t i=0; i<captureLogger.m_entries.size(); ++i )
        {
            if ( captureLogger.m_entries[ i ].logMessage.HasSubstr( "ThreadIdCheckMessage" ) >= 0 )
            {
                foundMatchingThreadId = ( captureLogger.m_entries[ i ].threadId == thisThreadId );
                break;
            }
        }
        ASSERT_TRUE( foundMatchingThreadId );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 19: Flush on empty thread buffer is a no-op (no crash)
    GUCEF_TESTFW_TESTCASE( "Test 19: FlushLogs on empty buffer is safe" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: FlushLogs on empty buffer is safe" );
        PreDrainThreadBuffer();

        CTestCaptureLogger captureLogger;
        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
        logMgr.AddLogger( &captureLogger );

        // Flush without writing any new segments
        CORE::CLogStreamScope::FlushLogs();
        CORE::CLogStreamScope::FlushLogs();

        // No crash is the primary goal; no entries specific to this test should appear
        ASSERT_FALSE( captureLogger.HasEntryWith( "EmptyFlushShouldNotExist" ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 20: Redirect to bootstrap log and flush to logger
    // Verifies that messages logged before any logger is registered are preserved
    GUCEF_TESTFW_TESTCASE( "Test 20: Bootstrap log redirect and flush" )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 20: Bootstrap log redirect and flush" );
        PreDrainThreadBuffer();

        CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();

        // Redirect to bootstrap queue (simulates startup before loggers are attached)
        logMgr.RedirectToBootstrapLogQueue( true );

        // This message will go to the bootstrap queue, not to loggers
        logMgr.Log( CORE::LOG_STANDARD, CORE::LOGLEVEL_NORMAL, "BootstrapQueueMessage" );

        // Add capture logger and turn off redirect - this also flushes bootstrap log
        CTestCaptureLogger captureLogger;
        logMgr.AddLogger( &captureLogger );
        logMgr.RedirectToBootstrapLogQueue( false );

        // The bootstrap flush goes through the direct (non-streaming) log path
        ASSERT_TRUE( captureLogger.HasEntryWith( "BootstrapQueueMessage" ) );

        logMgr.RemoveLogger( &captureLogger );
    GUCEF_TESTFW_TESTCASE_END

    // Test 21: Single threaded application, all logging from the main thread
    // Verifies that every message reaches both an in-memory logger and a file based CStdLogger
    GUCEF_TESTFW_TESTCASE( "Test 21: Single threaded logging from the main thread reaches all loggers" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: Single threaded logging from the main thread reaches all loggers" );
            PreDrainThreadBuffer();

            CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
            CORE::CString logFilePath = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_LoggingTest21_" + CORE::ToString( MT::GetCurrentTaskID() ) + ".txt" );

            CTestCaptureLogger captureLogger;
            {
                CORE::CFileAccess logFile( logFilePath, "w" );
                ASSERT_TRUE( logFile.IsValid() );
                CORE::CStdLogger fileLogger( logFile );

                logMgr.AddLogger( &captureLogger );
                logMgr.AddLogger( &fileLogger );

                for ( UInt32 i=0; i<LoggingTestNrOfMsgsPerThread; ++i )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "STLogTest-Main-Msg" + CORE::ToString( i ) + "-End" );

                logMgr.FlushLogs();
                logMgr.RemoveLogger( &fileLogger );
                logMgr.RemoveLogger( &captureLogger );
            }

            UInt32 wrongThreadIds = 0;
            ASSERT_TRUE( LoggingTestNrOfMsgsPerThread == CountEntriesWith( captureLogger, "STLogTest-Main-Msg", MT::GetCurrentTaskID(), wrongThreadIds ) );
            ASSERT_TRUE( 0 == wrongThreadIds );
            for ( UInt32 i=0; i<LoggingTestNrOfMsgsPerThread; ++i )
                ASSERT_TRUE( captureLogger.HasEntryWith( "STLogTest-Main-Msg" + CORE::ToString( i ) + "-End" ) );

            CORE::CString fileContent;
            ASSERT_TRUE( CORE::LoadTextFileAsString( logFilePath, fileContent ) );
            UInt32 fileLineCount = CountLinesWith( fileContent, "STLogTest-Main-Msg" );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: Found " + CORE::ToString( fileLineCount ) + " of " + CORE::ToString( LoggingTestNrOfMsgsPerThread ) + " expected lines in log file " + logFilePath );
            ASSERT_TRUE( LoggingTestNrOfMsgsPerThread == fileLineCount );
            if ( LoggingTestNrOfMsgsPerThread == fileLineCount )
                CORE::DeleteFile( logFilePath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 22: Multi threaded application, logging from the main thread and from worker threads concurrently
    // Verifies that no message is lost or duplicated, that each is attributed to the thread that logged it
    // and that the file based CStdLogger receives everything as well
    GUCEF_TESTFW_TESTCASE( "Test 22: Multi threaded logging from main and worker threads reaches all loggers" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: Multi threaded logging from main and worker threads reaches all loggers" );
            PreDrainThreadBuffer();

            CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
            CORE::CString logFilePath = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_LoggingTest22_" + CORE::ToString( MT::GetCurrentTaskID() ) + ".txt" );
            UInt32 mainThreadId = MT::GetCurrentTaskID();

            GUCEF::vector< CLoggingTestWorker* > workers;
            for ( UInt32 w=0; w<LoggingTestNrOfWorkerThreads; ++w )
                workers.push_back( GUCEF_NEW CLoggingTestWorker( w ) );

            CTestCaptureLogger captureLogger;
            {
                CORE::CFileAccess logFile( logFilePath, "w" );
                ASSERT_TRUE( logFile.IsValid() );
                CORE::CStdLogger fileLogger( logFile );

                logMgr.AddLogger( &captureLogger );
                logMgr.AddLogger( &fileLogger );

                for ( UInt32 w=0; w<LoggingTestNrOfWorkerThreads; ++w )
                    ASSERT_TRUE( workers[ w ]->Activate() );

                // The main thread logs concurrently with the workers
                for ( UInt32 i=0; i<LoggingTestNrOfMsgsPerThread; ++i )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MTLogTest-Main-Msg" + CORE::ToString( i ) + "-End" );

                for ( UInt32 w=0; w<LoggingTestNrOfWorkerThreads; ++w )
                    ASSERT_TRUE( workers[ w ]->WaitForCompletion( 30000 ) );

                // Drain everything that was logged, by all threads, into the loggers
                logMgr.FlushLogs();
                logMgr.RemoveLogger( &fileLogger );
                logMgr.RemoveLogger( &captureLogger );
            }

            UInt32 wrongThreadIds = 0;
            UInt32 totalMismatches = 0;
            UInt32 captured = CountEntriesWith( captureLogger, "MTLogTest-Main-Msg", mainThreadId, wrongThreadIds );
            CORE::CString fileContent;
            ASSERT_TRUE( CORE::LoadTextFileAsString( logFilePath, fileContent ) );
            UInt32 inFile = CountLinesWith( fileContent, "MTLogTest-Main-Msg" );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: main thread: captured " + CORE::ToString( captured ) + " (wrong thread id: " + CORE::ToString( wrongThreadIds ) +
                       "), in file " + CORE::ToString( inFile ) + ", expected " + CORE::ToString( LoggingTestNrOfMsgsPerThread ) );
            ASSERT_TRUE( LoggingTestNrOfMsgsPerThread == captured );
            ASSERT_TRUE( 0 == wrongThreadIds );
            ASSERT_TRUE( LoggingTestNrOfMsgsPerThread == inFile );
            if ( LoggingTestNrOfMsgsPerThread != captured || 0 != wrongThreadIds || LoggingTestNrOfMsgsPerThread != inFile )
                ++totalMismatches;

            for ( UInt32 w=0; w<LoggingTestNrOfWorkerThreads; ++w )
            {
                CORE::CString prefix = "MTLogTest-Worker" + CORE::ToString( w ) + "-Msg";
                ASSERT_TRUE( 0 != workers[ w ]->threadId && mainThreadId != workers[ w ]->threadId );
                captured = CountEntriesWith( captureLogger, prefix, workers[ w ]->threadId, wrongThreadIds );
                inFile = CountLinesWith( fileContent, prefix );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: worker " + CORE::ToString( w ) + ": captured " + CORE::ToString( captured ) + " (wrong thread id: " + CORE::ToString( wrongThreadIds ) +
                           "), in file " + CORE::ToString( inFile ) + ", expected " + CORE::ToString( LoggingTestNrOfMsgsPerThread ) );
                ASSERT_TRUE( LoggingTestNrOfMsgsPerThread == captured );
                ASSERT_TRUE( 0 == wrongThreadIds );
                ASSERT_TRUE( LoggingTestNrOfMsgsPerThread == inFile );
                if ( LoggingTestNrOfMsgsPerThread != captured || 0 != wrongThreadIds || LoggingTestNrOfMsgsPerThread != inFile )
                    ++totalMismatches;
            }
            for ( UInt32 w=0; w<LoggingTestNrOfWorkerThreads; ++w )
                GUCEF_DELETE workers[ w ];
            if ( 0 == totalMismatches )
                CORE::DeleteFile( logFilePath );
            else
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: kept log file for inspection: " + logFilePath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 23: Multi threaded application, worker threads log while the main thread keeps pulsing
    // In a real application nobody calls FlushLogs() explicitly: the main thread pulse drains the per-thread buffers.
    // Verifies worker output reaches the loggers through that pulse driven path alone.
    GUCEF_TESTFW_TESTCASE( "Test 23: Worker thread logging reaches loggers via the main thread pulse" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 23: Worker thread logging reaches loggers via the main thread pulse" );
            PreDrainThreadBuffer();

            CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
            ASSERT_TRUE( !pulseGenerator.IsNULL() );

            CTestCaptureLogger captureLogger;
            logMgr.AddLogger( &captureLogger );

            CLoggingTestWorker worker( 100 );
            ASSERT_TRUE( worker.Activate() );
            ASSERT_TRUE( worker.WaitForCompletion( 30000 ) );

            // Pulse the main thread pulse generator instead of flushing explicitly
            UInt32 wrongThreadIds = 0;
            CORE::CString prefix = "MTLogTest-Worker100-Msg";
            for ( UInt32 p=0; p<100; ++p )
            {
                pulseGenerator->ForceDirectPulse();
                if ( LoggingTestNrOfMsgsPerThread == CountEntriesWith( captureLogger, prefix, worker.threadId, wrongThreadIds ) )
                    break;
                MT::ThreadDelay( 10 );
            }

            ASSERT_TRUE( LoggingTestNrOfMsgsPerThread == CountEntriesWith( captureLogger, prefix, worker.threadId, wrongThreadIds ) );
            ASSERT_TRUE( 0 == wrongThreadIds );

            logMgr.RemoveLogger( &captureLogger );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 24: Flushing from several different threads, as happens when the pulse thread and other threads
    // flush the logs, must leave the buffers of the logging threads usable and must not lose messages.
    // The buffers of a thread are swapped under the lock of the buffer being swapped out. A flush which hangs
    // is reported as failed here, the threads that hang are left behind (that is the reason they are not deleted).
    GUCEF_TESTFW_TESTCASE( "Test 24: Flushing the logs from different threads keeps the buffers of a logging thread usable" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 24: Flushing the logs from different threads keeps the buffers of a logging thread usable" );
            PreDrainThreadBuffer();

            CORE::CLogManager& logMgr = CORE::CCoreGlobal::Instance()->GetLogManager();
            CTestCaptureLogger captureLogger;
            logMgr.AddLogger( &captureLogger );

            // A thread which logged and is gone, its buffers stay behind in the log manager
            CLoggingTestOneShot* logger = GUCEF_NEW CLoggingTestOneShot( CLoggingTestOneShot::ACTION_LOG, "LogFlushThreadsTest-Msg" );
            ASSERT_TRUE( logger->Activate() );
            bool loggerDone = logger->WaitForCompletion( 30000 );
            ASSERT_TRUE( loggerDone );
            if ( loggerDone )
                GUCEF_DELETE logger;

            // Flushes from one thread after the other, each of them swaps the buffers of the thread which logged
            bool allFlushesDone = true;
            for ( UInt32 i=0; i<6 && allFlushesDone; ++i )
            {
                CLoggingTestOneShot* flusher = GUCEF_NEW CLoggingTestOneShot( CLoggingTestOneShot::ACTION_FLUSH, CORE::CString::Empty );
                ASSERT_TRUE( flusher->Activate() );
                allFlushesDone = flusher->WaitForCompletion( 10000 );
                if ( allFlushesDone )
                    GUCEF_DELETE flusher;
            }
            ASSERT_TRUE( allFlushesDone );

            if ( allFlushesDone )
            {
                ASSERT_TRUE( captureLogger.HasEntryWith( "LogFlushThreadsTest-Msg" ) );

                // and the thread running the tests can still flush
                CORE::CLogStreamScope::FlushLogs();
            }
            logMgr.RemoveLogger( &captureLogger );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL logging backend TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
