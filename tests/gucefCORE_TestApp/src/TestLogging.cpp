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

#include <vector>

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

#ifndef GUCEF_CORE_CSTDLOGGER_H
#include "CStdLogger.h"
#define GUCEF_CORE_CSTDLOGGER_H
#endif /* GUCEF_CORE_CSTDLOGGER_H ? */

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

    std::vector< SEntry > m_entries;
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

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL logging backend TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
