/*
 *  gucefGUI_TestApp: GUCEF test application for the GUI module
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

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#include "gucefCORE_CLogStreamScope.h"
#define GUCEF_CORE_CLOGSTREAMSCOPE_H
#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_INPUT_H
#include "gucefINPUT.h"
#define GUCEF_INPUT_H
#endif /* GUCEF_INPUT_H ? */

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

#ifndef GUCEF_GUI_CGUIUSERACTIONLOG_H
#include "gucefGUI_CGUIUserActionLog.h"
#define GUCEF_GUI_CGUIUSERACTIONLOG_H
#endif /* GUCEF_GUI_CGUIUSERACTIONLOG_H ? */

#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** A logger that keeps the messages of the user log */
class CUserMessageRecorder : public CORE::CILogger
{
    public:

    GUCEF::vector< CORE::CString > lines;

    CUserMessageRecorder( void ) : CORE::CILogger(), lines(), m_minLogLevel( CORE::LOGLEVEL_EVERYTHING ) {}

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CString& message, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Keep( logMsgType, message ); }
    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CVariantStream& message, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Keep( logMsgType, message.ToString() ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CString& message, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Keep( logMsgType, message ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CVariantStream& message, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Keep( logMsgType, message.ToString() ); }
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE { m_minLogLevel = logLevel; }
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_minLogLevel; }

    /** The number of lines that have the text */
    UInt32 Count( const char* text ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<lines.size(); ++i )
            if ( lines[ i ].HasSubstr( text ) >= 0 )
                ++count;
        return count;
    }

    /** The lines kept so far, after the log manager handed on what it holds */
    void Flush( void ) { CORE::CLogStreamScope::FlushLogs(); }

    private:

    void Keep( const CORE::TLogMsgType logMsgType, const CORE::CString& message ) { if ( CORE::LOG_USER == logMsgType ) lines.push_back( message ); }

    Int32 m_minLogLevel;
};

/*-------------------------------------------------------------------------*/

/** Adds the recorder to the log manager and the user log on, for the time of a scope; puts back what was there */
class CRecordingScope
{
    public:

    explicit CRecordingScope( CUserMessageRecorder& recorder )
        : m_recorder( recorder )
        , m_wasEnabled( CORE::CCoreGlobal::Instance()->GetLogManager().IsUserLogEnabled() )
    {
        CORE::CLogStreamScope::FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_recorder );
        CORE::CCoreGlobal::Instance()->GetLogManager().SetUserLogEnabled( true );
    }

    ~CRecordingScope()
    {
        CORE::CLogStreamScope::FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &m_recorder );
        CORE::CCoreGlobal::Instance()->GetLogManager().SetUserLogEnabled( m_wasEnabled );
    }

    private:

    CRecordingScope( const CRecordingScope& src );
    CRecordingScope& operator=( const CRecordingScope& src );

    CUserMessageRecorder& m_recorder;
    bool m_wasEnabled;
};

/*-------------------------------------------------------------------------*/

void
ClickAt( GUI::CGUIContext& context, Int32 x, Int32 y )
{
    context.FlushChanges();
    context.InjectMouseMove( x, y );
    context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
    context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
    context.RenderFrame( 600 );
}

/*-------------------------------------------------------------------------*/

void
PressKey( GUI::CGUIContext& context, INPUT::KeyCode keyCode, UInt32 modifiers )
{
    context.InjectKey( keyCode, true, modifiers );
    context.InjectKey( keyCode, false, modifiers );
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
AddWidget( GUI::CGUIContext& context, const char* type, const char* name, GUI::CWidget* parent, const char* position, const char* size )
{
    GUI::CWidget* widget = context.CreateWidget( type, name, parent );
    if ( GUCEF_NULL != widget )
    {
        widget->SetProperty( "Position", position );
        widget->SetProperty( "Size", size );
    }
    return widget;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiUserActionLogTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.UserActionLog" );

    GUCEF_TESTFW_TESTCASE( "UserActionLog: forms opened and closed, keys, typed text, buttons, check boxes and chosen items are logged with the name of the context, nothing while the user log is off" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CWidget* edit = AddWidget( *context, "Editbox", "Name", root, "0,10 0,10", "0,200 0,24" );
        GUI::CWidget* button = AddWidget( *context, "Button", "Ok", root, "0,10 0,100", "0,80 0,30" );
        button->SetProperty( "Text", "OK" );
        GUI::CWidget* option = AddWidget( *context, "Checkbox", "Option", root, "0,10 0,200", "0,200 0,24" );
        option->SetProperty( "Text", "An option" );
        ASSERT_TRUE( context->ShowRoot( root, false ) );

        CUserMessageRecorder recorder;
        CRecordingScope recording( recorder );

        // the root shown before Attach is watched as well
        GUI::CGUIUserActionLog log;
        ASSERT_TRUE( log.Attach( contextPtr, "test" ) );
        ASSERT_TRUE( log.IsAttached( contextPtr ) && 1 == log.GetAttachedContextCount() );

        // off: not a line
        CORE::CCoreGlobal::Instance()->GetLogManager().SetUserLogEnabled( false );
        ASSERT_TRUE( context->SetFocus( edit ) );
        PressKey( *context, INPUT::KEYCODE_A, 0 );
        context->InjectText( "off" );
        recorder.Flush();
        ASSERT_TRUE( recorder.lines.empty() );
        CORE::CCoreGlobal::Instance()->GetLogManager().SetUserLogEnabled( true );

        PressKey( *context, INPUT::KEYCODE_B, INPUT::KEYMOD_SHIFT );
        context->InjectText( "hello" );
        context->InjectKey( INPUT::KEYCODE_LSHIFT, true, INPUT::KEYMOD_SHIFT );
        context->InjectKey( INPUT::KEYCODE_LSHIFT, false, 0 );
        PressKey( *context, INPUT::KEYCODE_R, INPUT::KEYMOD_CTRL );
        recorder.Flush();
        ASSERT_TRUE( 1 == recorder.Count( "GUI test: key Shift+B in Screen/Name (Editbox)" ) );
        ASSERT_TRUE( 1 == recorder.Count( "GUI test: typed \"hello\" into Screen/Name (Editbox)" ) );
        ASSERT_TRUE( 1 == recorder.Count( "GUI test: key Ctrl+R in Screen/Name (Editbox)" ) );
        // a key that only modifies is no line; the change of the text is said by the typed text alone
        ASSERT_TRUE( 0 == recorder.Count( "LSHIFT" ) && 0 == recorder.Count( "changed Screen/Name" ) && 0 == recorder.Count( "off" ) );

        // Enter on the button activates it
        ASSERT_TRUE( context->SetFocus( button ) );
        PressKey( *context, INPUT::KEYCODE_RETURN, 0 );
        recorder.Flush();
        ASSERT_TRUE( 1 == recorder.Count( "GUI test: activated Screen/Ok (Button) \"OK\"" ) );

        // a click on the check box changes its value
        ClickAt( *context, 20, 210 );
        recorder.Flush();
        ASSERT_TRUE( 1 == recorder.Count( "GUI test: changed Screen/Option (Checkbox), Checked \"true\"" ) );

        // a form that opens and closes, shown after Attach
        GUI::CWidget* dialog = context->CreateWidget( "Panel", "Question", GUCEF_NULL );
        dialog->SetProperty( "Size", "0,300 0,200" );
        GUI::CWidget* yes = AddWidget( *context, "Button", "Yes", dialog, "0,10 0,10", "0,80 0,30" );
        yes->SetProperty( "Text", "Yes" );
        ASSERT_TRUE( context->ShowRoot( dialog, true ) );
        ASSERT_TRUE( context->SetFocus( yes ) );
        PressKey( *context, INPUT::KEYCODE_RETURN, 0 );
        ASSERT_TRUE( context->HideRoot( dialog ) );
        recorder.Flush();
        ASSERT_TRUE( 1 == recorder.Count( "GUI test: opened Question (Panel), modal" ) );
        ASSERT_TRUE( 1 == recorder.Count( "GUI test: activated Question/Yes (Button) \"Yes\"" ) );
        ASSERT_TRUE( 1 == recorder.Count( "GUI test: closed Question" ) );

        // a list: the chosen item
        GUI::CWidget* list = AddWidget( *context, "Listbox", "Choices", root, "0,300 0,10", "0,200 0,120" );
        list->SetProperty( "Items", "first\nsecond\nthird" );
        context->FlushChanges();
        ASSERT_TRUE( context->SetFocus( list ) );
        PressKey( *context, INPUT::KEYCODE_DOWN, 0 );
        PressKey( *context, INPUT::KEYCODE_RETURN, 0 );
        recorder.Flush();
        ASSERT_TRUE( 1 == recorder.Count( "GUI test: key DOWN in Screen/Choices (Listbox)" ) );
        ASSERT_TRUE( recorder.Count( "Screen/Choices (Listbox)" ) >= 2 );

        log.Detach( contextPtr );
        ASSERT_FALSE( log.IsAttached( contextPtr ) );
        const size_t before = recorder.lines.size();
        ASSERT_TRUE( context->SetFocus( edit ) );
        PressKey( *context, INPUT::KEYCODE_C, 0 );
        recorder.Flush();
        // only what the context itself logs: no key
        ASSERT_TRUE( 0 == recorder.Count( "key C in" ) );
        ASSERT_TRUE( recorder.lines.size() >= before );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UserActionLog: what is typed into a secret input is never logged, nor how much of it; Tab, Enter and Escape in it are, also in the file of the user action log" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Login", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CWidget* secret = AddWidget( *context, "SecretInput", "Password", root, "0,10 0,10", "0,200 0,24" );
        GUI::CWidget* user = AddWidget( *context, "Editbox", "User", root, "0,10 0,60", "0,200 0,24" );
        ASSERT_TRUE( GUCEF_NULL != secret && GUCEF_NULL != user );
        ASSERT_TRUE( GUI::CGUIUserActionLog::IsSecretInput( secret ) && !GUI::CGUIUserActionLog::IsSecretInput( user ) && !GUI::CGUIUserActionLog::IsSecretInput( GUCEF_NULL ) );
        ASSERT_TRUE( context->ShowRoot( root, false ) );

        CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
        const CORE::CString logFile = CORE::RelativePath( "$TEMPDIR$/gucefGUI_TestApp_UserActionLog/UserActions.log" );
        CORE::DeleteFile( logFile );
        CUserMessageRecorder recorder;
        CRecordingScope recording( recorder );
        ASSERT_TRUE( logManager.SetUserActionLogEnabled( true, logFile ) );

        GUI::CGUIUserActionLog log;
        ASSERT_TRUE( log.Attach( contextPtr, "login" ) );
        ASSERT_TRUE( context->SetFocus( secret ) );
        const char* typed[] = { "s", "3", "c", "r", "3", "t", "-", "X", "Y", "Z" };
        const INPUT::KeyCode keys[] = { INPUT::KEYCODE_S, INPUT::KEYCODE_3, INPUT::KEYCODE_C, INPUT::KEYCODE_R, INPUT::KEYCODE_3, INPUT::KEYCODE_T,
                                        INPUT::KEYCODE_MINUS, INPUT::KEYCODE_X, INPUT::KEYCODE_Y, INPUT::KEYCODE_Z };
        for ( UInt32 i=0; i<10; ++i )
        {
            context->InjectKey( keys[ i ], true, 'X' == typed[ i ][ 0 ] || 'Y' == typed[ i ][ 0 ] || 'Z' == typed[ i ][ 0 ] ? (UInt32) INPUT::KEYMOD_SHIFT : 0 );
            context->InjectText( typed[ i ] );
            context->InjectKey( keys[ i ], false, 0 );
        }
        PressKey( *context, INPUT::KEYCODE_BACK, 0 );
        context->InjectText( "Z" );
        PressKey( *context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( static_cast< GUI::CSecretInput* >( secret )->GetValueLength() >= 10 );
        PressKey( *context, INPUT::KEYCODE_TAB, 0 );
        ASSERT_TRUE( user->HasFocus() );
        context->InjectText( "visible" );
        recorder.Flush();

        // of the secret input only the Tab that left it
        ASSERT_TRUE( 1 == recorder.Count( "Login/Password" ) );
        ASSERT_TRUE( 1 == recorder.Count( "GUI login: key TAB in Login/Password (SecretInput)" ) );
        ASSERT_TRUE( 0 == recorder.Count( "s3cr3t" ) && 0 == recorder.Count( "XYZ" ) && 0 == recorder.Count( "\"Z\"" ) && 0 == recorder.Count( "\"s\"" ) );
        ASSERT_TRUE( 1 == recorder.Count( "GUI login: typed \"visible\" into Login/User (Editbox)" ) );

        // the file of the user action log has the same lines: the text of the field, not the secret
        ASSERT_TRUE( logManager.SetUserActionLogEnabled( false, CORE::CString::Empty ) );
        CORE::CString content;
        ASSERT_TRUE( CORE::LoadTextFileAsString( logFile, content ) );
        ASSERT_TRUE( content.HasSubstr( "visible" ) >= 0 && content.HasSubstr( "key TAB in Login/Password" ) >= 0 );
        ASSERT_TRUE( content.HasSubstr( "s3cr3t" ) < 0 && content.HasSubstr( "XYZ" ) < 0 && content.HasSubstr( "typed \"s\"" ) < 0 && content.HasSubstr( "key S in Login/Password" ) < 0 );
        CORE::DeleteFile( logFile );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UserActionLog: root widgets and contexts that go while attached are forgotten, several contexts keep their own names" )
    try
    {
        CUserMessageRecorder recorder;
        CRecordingScope recording( recorder );
        GUI::CGUIUserActionLog log;

        GUI::TGuiContextPtr firstPtr = CreateNullContext();
        GUI::TGuiContextPtr secondPtr = CreateNullContext();
        GUI::CGUIContext* first = static_cast< GUI::CGUIContext* >( firstPtr.GetPointerAlways() );
        GUI::CGUIContext* second = static_cast< GUI::CGUIContext* >( secondPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != first && GUCEF_NULL != second );
        ASSERT_TRUE( log.Attach( firstPtr, "main" ) && log.Attach( secondPtr, "overlay" ) && log.Attach( secondPtr, "settings" ) );
        ASSERT_FALSE( log.Attach( GUI::TGuiContextPtr(), "nothing" ) );
        ASSERT_TRUE( 2 == log.GetAttachedContextCount() );

        GUI::CWidget* a = first->CreateWidget( "Panel", "A", GUCEF_NULL );
        a->SetProperty( "Size", "1,0 1,0" );
        GUI::CWidget* b = second->CreateWidget( "Panel", "B", GUCEF_NULL );
        b->SetProperty( "Size", "1,0 1,0" );
        ASSERT_TRUE( first->ShowRoot( a, false ) && second->ShowRoot( b, false ) );
        PressKey( *first, INPUT::KEYCODE_F5, 0 );
        PressKey( *second, INPUT::KEYCODE_F11, 0 );
        recorder.Flush();
        ASSERT_TRUE( 1 == recorder.Count( "GUI main: opened A (Panel)" ) && 1 == recorder.Count( "GUI settings: opened B (Panel)" ) );
        ASSERT_TRUE( 1 == recorder.Count( "GUI main: key F5 in A (Panel)" ) && 1 == recorder.Count( "GUI settings: key F11 in B (Panel)" ) );

        // the root goes: no key reaches anything, nothing breaks
        first->DestroyWidget( a );
        first->FlushChanges();
        PressKey( *first, INPUT::KEYCODE_F5, 0 );

        // the context goes: forgotten with what is left of its roots
        secondPtr.Unlink();
        ASSERT_TRUE( 1 == log.GetAttachedContextCount() && log.IsAttached( firstPtr ) );

        GUI::CWidget* c = first->CreateWidget( "Panel", "C", GUCEF_NULL );
        c->SetProperty( "Size", "1,0 1,0" );
        ASSERT_TRUE( first->ShowRoot( c, false ) );
        PressKey( *first, INPUT::KEYCODE_F6, 0 );
        recorder.Flush();
        ASSERT_TRUE( 1 == recorder.Count( "GUI main: key F6 in C (Panel)" ) );

        ASSERT_TRUE( "Ctrl+Alt+Shift+DELETE" == GUI::CGUIUserActionLog::DescribeKey( INPUT::KEYCODE_DELETE, INPUT::KEYMOD_CTRL | INPUT::KEYMOD_ALT | INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( GUI::CGUIUserActionLog::IsModifierKey( INPUT::KEYCODE_RCONTROL ) && !GUI::CGUIUserActionLog::IsModifierKey( INPUT::KEYCODE_COMMA ) );
        ASSERT_TRUE( "nothing" == GUI::CGUIUserActionLog::DescribeWidget( GUCEF_NULL ) );

        // a context made after the other went (perhaps at its address) is attached as a context of its own
        GUI::TGuiContextPtr thirdPtr = CreateNullContext();
        ASSERT_TRUE( log.Attach( thirdPtr, "third" ) && 2 == log.GetAttachedContextCount() );
        GUI::CGUIContext* third = static_cast< GUI::CGUIContext* >( thirdPtr.GetPointerAlways() );
        GUI::CWidget* d = third->CreateWidget( "Panel", "D", GUCEF_NULL );
        d->SetProperty( "Size", "1,0 1,0" );
        ASSERT_TRUE( third->ShowRoot( d, false ) );
        PressKey( *third, INPUT::KEYCODE_F7, 0 );
        recorder.Flush();
        ASSERT_TRUE( 1 == recorder.Count( "GUI third: key F7 in D (Panel)" ) );

        log.DetachAll();
        ASSERT_TRUE( 0 == log.GetAttachedContextCount() );
        PressKey( *third, INPUT::KEYCODE_F8, 0 );
        recorder.Flush();
        ASSERT_TRUE( 0 == recorder.Count( "key F8" ) );
        thirdPtr.Unlink();
        firstPtr.Unlink();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
