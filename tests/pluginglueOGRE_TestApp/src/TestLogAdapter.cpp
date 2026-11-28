/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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

#include "OgreLogManager.h"
#include "OgreLog.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGRELOGLEVELMAPPER_H
#include "pluginglueOGRECOMMON_COgreLogLevelMapper.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGRELOGLEVELMAPPER_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGRELOGLEVELMAPPER_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRELOGADAPTER_H
#include "pluginglueOGRE_COgreLogAdapter.h"
#define GUCEF_PLUGINGLUE_OGRE_COGRELOGADAPTER_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGRELOGADAPTER_H ? */

#include "TestSupport.h"
#include "TestLogAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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

namespace {

/** Keeps the type, level and text of every message that is logged while it exists */
class CRecordingLogger : public CORE::CILogger
{
    public:

    struct SRecord
    {
        CORE::TLogMsgType type;
        Int32 level;
        CORE::CString text;
    };
    typedef GUCEF::vector< SRecord > TRecordVector;

    CRecordingLogger( void ) : CORE::CILogger(), m_records(), m_lock()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this );
    }

    virtual ~CRecordingLogger() GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this );
    }

    /** The records of the messages that contain the text, once the logs on their way are delivered */
    TRecordVector Find( const CORE::CString& text )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        MT::CScopeMutex lock( m_lock );
        TRecordVector found;
        for ( UInt32 i=0; i<(UInt32) m_records.size(); ++i )
        {
            if ( m_records[ i ].text.HasSubstr( text, true ) >= 0 )
                found.push_back( m_records[ i ] );
        }
        return found;
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        SRecord record;
        record.type = logMsgType;
        record.level = logLevel;
        record.text = logMessage;
        m_records.push_back( record );
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage, threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    TRecordVector m_records;
    MT::CMutex m_lock;
};

} // anonymous namespace

/*-------------------------------------------------------------------------*/

static void
TestLevelMapping( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Log adapter: the Ogre message levels map to the GUCEF logs and levels of the design" )
        try
        {
            using PLUGINGLUE::OGRECOMMON::COgreLogLevelMapper;
            COgreLogLevelMapper::TLogKind kind;
            Int32 level = 0;

            COgreLogLevelMapper::Map( Ogre::LML_TRIVIAL, kind, level );
            ASSERT_TRUE( COgreLogLevelMapper::LOGKIND_SYSTEM == kind && CORE::LOGLEVEL_BELOW_NORMAL == level );
            COgreLogLevelMapper::Map( Ogre::LML_NORMAL, kind, level );
            ASSERT_TRUE( COgreLogLevelMapper::LOGKIND_SYSTEM == kind && CORE::LOGLEVEL_NORMAL == level );
            COgreLogLevelMapper::Map( Ogre::LML_WARNING, kind, level );
            ASSERT_TRUE( COgreLogLevelMapper::LOGKIND_WARNING == kind && CORE::LOGLEVEL_NORMAL == level );
            COgreLogLevelMapper::Map( Ogre::LML_CRITICAL, kind, level );
            ASSERT_TRUE( COgreLogLevelMapper::LOGKIND_ERROR == kind && CORE::LOGLEVEL_CRITICAL == level );

            // the program logs of a driver with warnings only are no errors, a driver log with an error is
            ASSERT_TRUE( COgreLogLevelMapper::IsDriverLogWithWarningsOnly( "A\nB\nGLSL program pipeline validation result: \nWARNING: 1:1: '' :  #version directive missing" ) );
            ASSERT_FALSE( COgreLogLevelMapper::IsDriverLogWithWarningsOnly( "A GLSL compile result: \nWARNING: x\nERROR: 0:3: syntax error" ) );
            ASSERT_FALSE( COgreLogLevelMapper::IsDriverLogWithWarningsOnly( "OGRE EXCEPTION(3:RenderingAPIException): out of memory" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestAdapterOnOgreLog( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Log adapter: messages of the Ogre log arrive in the GUCEF logs with the prefix Ogre:, the log of the type and the level, and no file is written" )
        try
        {
            ASSERT_TRUE( GUCEF_NULL == Ogre::LogManager::getSingletonPtr() );

            // the adapter is made without a Root, as the support library does it first of all
            const CORE::CString dir = MakeCleanTestDir( "logadapter" );
            CRecordingLogger recorder;
            {
                PLUGINGLUE::OGRE::COgreLogAdapter adapter;
                ASSERT_TRUE( GUCEF_NULL != Ogre::LogManager::getSingletonPtr() );
                ASSERT_TRUE( GUCEF_NULL != adapter.GetLog() );
                ASSERT_TRUE( adapter.GetLog() == Ogre::LogManager::getSingleton().getDefaultLog() );

                Ogre::LogManager& logManager = Ogre::LogManager::getSingleton();
                logManager.logMessage( "adaptertest trivial message", Ogre::LML_TRIVIAL );
                logManager.logMessage( "adaptertest normal message", Ogre::LML_NORMAL );
                logManager.logMessage( "adaptertest warning message", Ogre::LML_WARNING );
                logManager.logMessage( "adaptertest critical message", Ogre::LML_CRITICAL );

                CRecordingLogger::TRecordVector found = recorder.Find( "Ogre: adaptertest trivial message" );
                ASSERT_TRUE( 1 == found.size() );
                if ( 1 == found.size() )
                    ASSERT_TRUE( CORE::LOG_SYSTEM == found[ 0 ].type && CORE::LOGLEVEL_BELOW_NORMAL == found[ 0 ].level );

                found = recorder.Find( "Ogre: adaptertest normal message" );
                ASSERT_TRUE( 1 == found.size() );
                if ( 1 == found.size() )
                    ASSERT_TRUE( CORE::LOG_SYSTEM == found[ 0 ].type && CORE::LOGLEVEL_NORMAL == found[ 0 ].level );

                found = recorder.Find( "Ogre: adaptertest warning message" );
                ASSERT_TRUE( 1 == found.size() );
                if ( 1 == found.size() )
                    ASSERT_TRUE( CORE::LOG_WARNING == found[ 0 ].type && CORE::LOGLEVEL_NORMAL == found[ 0 ].level );

                found = recorder.Find( "Ogre: adaptertest critical message" );
                ASSERT_TRUE( 1 == found.size() );
                if ( 1 == found.size() )
                    ASSERT_TRUE( CORE::LOG_ERROR == found[ 0 ].type && CORE::LOGLEVEL_CRITICAL == found[ 0 ].level );
            }

            // the manager of the adapter goes with it and the log of Ogre does not make files
            ASSERT_TRUE( GUCEF_NULL == Ogre::LogManager::getSingletonPtr() );
            ASSERT_FALSE( CORE::FileExists( "OgreGucef" ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, "OgreGucef" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestAdapterOnExistingManager( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Log adapter: an Ogre log manager that exists already is used and left to its owner" )
        try
        {
            Ogre::LogManager* ownerManager = new Ogre::LogManager();
            Ogre::Log* ownerLog = ownerManager->createLog( "owner", true, false, true );
            ASSERT_TRUE( GUCEF_NULL != ownerLog );

            {
                PLUGINGLUE::OGRE::COgreLogAdapter adapter;
                ASSERT_TRUE( ownerManager == Ogre::LogManager::getSingletonPtr() );
                ASSERT_TRUE( adapter.GetLog() == ownerManager->getDefaultLog() );
            }

            // the manager of the owner is still there, with its own log
            ASSERT_TRUE( ownerManager == Ogre::LogManager::getSingletonPtr() );
            ASSERT_TRUE( ownerLog == ownerManager->getLog( "owner" ) );
            delete ownerManager;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformLogAdapterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "pluginglueOGRE.LogAdapter" );

    TestLevelMapping();
    TestAdapterOnOgreLog();
    TestAdapterOnExistingManager();
}

/*-------------------------------------------------------------------------*/
