/*
 *  gucefPUBSUB_TestApp: GUCEF test application for the PUBSUB module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_PUBSUB_H
#include "gucefPUBSUB.h"
#define GUCEF_PUBSUB_H
#endif /* GUCEF_PUBSUB_H ? */

#ifndef GUCEF_PUBSUB_CBINARYPUBSUBJOURNAL_H
#include "gucefPUBSUB_CBinaryPubSubJournal.h"
#define GUCEF_PUBSUB_CBINARYPUBSUBJOURNAL_H
#endif /* GUCEF_PUBSUB_CBINARYPUBSUBJOURNAL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestBinaryPubSubJournal.h"

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
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Keeps the text of every log message that is logged while it is attached to the log manager */
class CJournalLogCapture : public CORE::CILogger
{
    public:

    CJournalLogCapture( void )
        : CORE::CILogger()
        , m_text()
        , m_lock()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this );
    }

    virtual ~CJournalLogCapture()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this );
    }

    /** logs that are still on their way to the loggers are delivered first */
    CORE::CString GetText( void )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        MT::CScopeMutex lock( m_lock );
        return m_text;
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        m_text += logMessage;
        m_text += "\n";
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage, threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    CORE::CString m_text;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformBinaryPubSubJournalTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CBinaryPubSubJournal TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CBinaryPubSubJournal" );

    GUCEF_TESTFW_TESTCASE( "Test 1: The log of a journal resource which cannot be opened holds no secrets of its uri" )
        try
        {
            CJournalLogCapture capture;

            PUBSUB::CPubSubJournalConfig config;
            config.useJournal = true;
            config.journalType = PUBSUB::CBinaryPubSubJournal::JournalType;

            // nobody handles this scheme, so there is no resource accessor for it
            config.journalResource = CORE::CUri( "nosuchscheme://user:SECRETPASSWORD@127.0.0.1/journal?token=SECRETTOKEN#SECRETFRAGMENT" );
            PUBSUB::CBinaryPubSubJournal journal;
            ASSERT_FALSE( journal.LoadConfig( config ) );

            // the accessor of the scheme exists but cannot open what the uri points to
            config.journalResource = CORE::CUri( "file:///nonexistent-directory-for-test/journal.bin?token=SECRETTOKEN" );
            ASSERT_FALSE( journal.LoadConfig( config ) );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( logged.HasSubstr( "Failed to create resource accessor for journal resource nosuchscheme://***@127.0.0.1/journal?token=***#***", true ) >= 0 );
            ASSERT_TRUE( logged.HasSubstr( "SECRET", true ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
