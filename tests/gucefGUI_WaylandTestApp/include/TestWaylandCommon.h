/*
 *  gucefGUI_WaylandTestApp: Tests of the Wayland window and input drivers against a fake compositor
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


#ifndef GUCEF_GUI_WAYLANDTESTAPP_TESTWAYLANDCOMMON_H
#define GUCEF_GUI_WAYLANDTESTAPP_TESTWAYLANDCOMMON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H
#include "inputdriverpluginstructs.h"
#define GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H
#endif /* GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#include "pluginglueWAYLAND_CWaylandConnection.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H ? */

#include "TestFakeWayland.h"

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
//      HELPERS                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TESTWAYLAND {

/** Counts the events of a notifier by event, for tests of what a window context tells the GUI */
class CEventCounter : public CORE::CObserver
{
    public:

    CEventCounter( void ) : CORE::CObserver(), m_names(), m_counts() {}

    void Watch( CORE::CNotifier& notifier, const CORE::CEvent& eventId );

    UInt32 Count( const CORE::CEvent& eventId ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< CEventCounter > TEventCallback;

    void OnCounted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    GUCEF::vector< CORE::CString > m_names;
    GUCEF::vector< UInt32 > m_counts;
};

/** Keeps the log messages of one type (the user actions, by default) to look at in a test; add it to the log manager and remove it again */
class CLogCapture : public CORE::CILogger
{
    public:

    explicit CLogCapture( CORE::TLogMsgType type = CORE::LOG_USER ) : CORE::CILogger(), m_type( type ), m_messages() {}

    GUCEF::vector< CORE::CString > m_messages;

    bool Has( const char* text ) const;
    UInt32 Count( const char* text ) const;

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    CORE::TLogMsgType m_type;
};

/** The callbacks of an input driver written down as text, in the order they were called */
class CInputRecorder
{
    public:

    CInputRecorder( void );

    INPUT::TInputCallbacks callbacks;
    GUCEF::vector< CORE::CString > events;

    bool Has( const char* text ) const;
    UInt32 Count( const char* prefix ) const;
    CORE::CString Last( void ) const;
    void Clear( void );
};

/** The connection gets a cursor theme that has no cursors, so a pointer entering a window never calls into the real library of cursor themes with the objects of a fake compositor */
void UseNoCursorTheme( PLUGINGLUE::WAYLAND::CWaylandConnection& connection );

/** The keymap of the tests: a latin and a cyrillic layout of a few keys, with Shift and Control and a dead key */
const char* GetTestKeymap( void );

/** The evdev codes of the keys of that keymap */
enum ETestKey
{
    TESTKEY_1       = 2  ,
    TESTKEY_A       = 30 ,
    TESTKEY_S       = 31 ,
    TESTKEY_DEAD    = 40 ,
    TESTKEY_ENTER   = 28 ,
    TESTKEY_LCTRL   = 29 ,
    TESTKEY_LSHIFT  = 42 ,
    TESTKEY_SPACE   = 57 ,
    TESTKEY_LEFT    = 105
};

} // namespace TESTWAYLAND
} // namespace GUCEF

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_WAYLANDTESTAPP_TESTWAYLANDCOMMON_H ? */
