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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "TestWaylandCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TESTWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CEventCounter::Watch( CORE::CNotifier& notifier, const CORE::CEvent& eventId )
{GUCEF_TRACE;

    TEventCallback callback( this, &CEventCounter::OnCounted );
    SubscribeTo( &notifier, eventId, callback );
    m_names.push_back( eventId.GetName() );
    m_counts.push_back( 0 );
}

/*-------------------------------------------------------------------------*/

UInt32
CEventCounter::Count( const CORE::CEvent& eventId ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_names.size(); ++i )
        if ( m_names[ i ] == eventId.GetName() )
            return m_counts[ i ];
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CEventCounter::OnCounted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_names.size(); ++i )
        if ( m_names[ i ] == eventId.GetName() )
            ++m_counts[ i ];
}

/*-------------------------------------------------------------------------*/

bool
CLogCapture::Has( const char* text ) const
{GUCEF_TRACE;

    return Count( text ) > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CLogCapture::Count( const char* text ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<m_messages.size(); ++i )
        if ( m_messages[ i ].HasSubstr( text ) >= 0 )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp )
{GUCEF_TRACE;

    if ( logMsgType == m_type )
        m_messages.push_back( logMessage );
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp )
{GUCEF_TRACE;

    Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp )
{GUCEF_TRACE;

    Log( logMsgType, logLevel, logMessage, threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp )
{GUCEF_TRACE;

    Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

namespace {

CInputRecorder* g_recorder = GUCEF_NULL;

void R_down( void*, const Int32, const UInt32 button ) { g_recorder->events.push_back( "down " + CORE::UInt32ToString( button ) ); }
void R_up( void*, const Int32, const UInt32 button ) { g_recorder->events.push_back( "up " + CORE::UInt32ToString( button ) ); }

void
R_move( void*, const Int32, const Int32 x, const Int32 y, const Int32 dx, const Int32 dy )
{
    g_recorder->events.push_back( "move " + CORE::Int32ToString( x ) + "," + CORE::Int32ToString( y ) + " d" + CORE::Int32ToString( dx ) + "," + CORE::Int32ToString( dy ) );
}

void
R_var( void*, const Int32, const UInt32 index, const Int32 value, const Int32 delta )
{
    g_recorder->events.push_back( "var " + CORE::UInt32ToString( index ) + " " + CORE::Int32ToString( value ) + " " + ( delta > 0 ? "+" : "" ) + CORE::Int32ToString( delta ) );
}

void
R_keyDown( void*, const Int32, const INPUT::KeyCode code, const UInt32 mods )
{
    g_recorder->events.push_back( "keydown " + CORE::UInt32ToString( (UInt32) code ) + " m" + CORE::UInt32ToString( mods ) );
}

void
R_keyUp( void*, const Int32, const INPUT::KeyCode code, const UInt32 mods )
{
    g_recorder->events.push_back( "keyup " + CORE::UInt32ToString( (UInt32) code ) + " m" + CORE::UInt32ToString( mods ) );
}

void R_text( void*, const Int32, const char* text ) { g_recorder->events.push_back( CORE::CString( "text " ) + text ); }
void R_boolOn( void*, const Int32, const UInt32 ) {}
void R_boolOff( void*, const Int32, const UInt32 ) {}
void R_devVar( void*, const Int32, const UInt32, const Float32 ) {}
void R_mouseAttached( void*, const Int32 ) { g_recorder->events.push_back( "mouse attached" ); }
void R_keyboardAttached( void*, const Int32 ) { g_recorder->events.push_back( "keyboard attached" ); }
void R_deviceAttached( void*, const Int32 ) {}
void R_mouseDetached( void*, const Int32 ) { g_recorder->events.push_back( "mouse detached" ); }
void R_keyboardDetached( void*, const Int32 ) { g_recorder->events.push_back( "keyboard detached" ); }
void R_deviceDetached( void*, const Int32 ) {}

}

/*-------------------------------------------------------------------------*/

CInputRecorder::CInputRecorder( void )
    : events()
{GUCEF_TRACE;

    g_recorder = this;
    memset( &callbacks, 0, sizeof( callbacks ) );
    callbacks.onMouseButtonDown = R_down;
    callbacks.onMouseButtonUp = R_up;
    callbacks.onMouseMove = R_move;
    callbacks.onMouseVarChanged = R_var;
    callbacks.onKeyboardKeyDown = R_keyDown;
    callbacks.onKeyboardKeyUp = R_keyUp;
    callbacks.onDeviceBooleanOn = R_boolOn;
    callbacks.onDeviceBooleanOff = R_boolOff;
    callbacks.onDeviceVarChanged = R_devVar;
    callbacks.onMouseAttached = R_mouseAttached;
    callbacks.onKeyboardAttached = R_keyboardAttached;
    callbacks.onDeviceAttached = R_deviceAttached;
    callbacks.onMouseDetached = R_mouseDetached;
    callbacks.onKeyboardDetached = R_keyboardDetached;
    callbacks.onDeviceDetached = R_deviceDetached;
    callbacks.userData = this;
    callbacks.onKeyboardText = R_text;
}

/*-------------------------------------------------------------------------*/

bool
CInputRecorder::Has( const char* text ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<events.size(); ++i )
        if ( events[ i ] == text )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CInputRecorder::Count( const char* prefix ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    const size_t length = strlen( prefix );
    for ( UInt32 i=0; i<events.size(); ++i )
        if ( 0 == strncmp( events[ i ].C_String(), prefix, length ) )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CInputRecorder::Last( void ) const
{GUCEF_TRACE;

    return events.empty() ? CORE::CString::Empty : events.back();
}

/*-------------------------------------------------------------------------*/

void
CInputRecorder::Clear( void )
{GUCEF_TRACE;

    events.clear();
}

/*-------------------------------------------------------------------------*/

namespace {

struct wl_cursor_theme* N_load( const char*, int, struct wl_shm* ) { return GUCEF_NULL; }
void N_destroy( struct wl_cursor_theme* ) {}
struct wl_cursor* N_getCursor( struct wl_cursor_theme*, const char* ) { return GUCEF_NULL; }
struct wl_buffer* N_getBuffer( struct wl_cursor_image* ) { return GUCEF_NULL; }

}

/*-------------------------------------------------------------------------*/

void
UseNoCursorTheme( PLUGINGLUE::WAYLAND::CWaylandConnection& connection )
{GUCEF_TRACE;

    SWaylandCursorApi api;
    api.theme_load = N_load;
    api.theme_destroy = N_destroy;
    api.theme_get_cursor = N_getCursor;
    api.image_get_buffer = N_getBuffer;
    connection.GetCursor().SetCursorApi( api );
}

/*-------------------------------------------------------------------------*/

const char*
GetTestKeymap( void )
{GUCEF_TRACE;

    return
    "xkb_keymap {\n"
    "xkb_keycodes \"test\" {\n"
    "    minimum = 8; maximum = 255;\n"
    "    <AE01> = 10; <RTRN> = 36; <LCTL> = 37; <AC01> = 38; <AC02> = 39; <AC11> = 48; <LFSH> = 50; <SPCE> = 65; <LEFT> = 113;\n"
    "};\n"
    "xkb_types \"test\" {\n"
    "    type \"ONE_LEVEL\" { modifiers = none; map[none] = Level1; level_name[Level1] = \"Any\"; };\n"
    "    type \"TWO_LEVEL\" { modifiers = Shift; map[Shift] = Level2; level_name[Level1] = \"Base\"; level_name[Level2] = \"Shift\"; };\n"
    "    type \"ALPHABETIC\" { modifiers = Shift+Lock; map[Shift] = Level2; map[Lock] = Level2; level_name[Level1] = \"Base\"; level_name[Level2] = \"Caps\"; };\n"
    "};\n"
    "xkb_compat \"test\" { };\n"
    "xkb_symbols \"test\" {\n"
    "    name[Group1] = \"Latin\"; name[Group2] = \"Cyrillic\";\n"
    "    key <AE01> { [ 1, exclam ] };\n"
    "    key <RTRN> { [ Return ] };\n"
    "    key <LCTL> { [ Control_L ] };\n"
    "    key <AC01> { type[Group1] = \"ALPHABETIC\", symbols[Group1] = [ a, A ], type[Group2] = \"ALPHABETIC\", symbols[Group2] = [ Cyrillic_ef, Cyrillic_EF ] };\n"
    "    key <AC02> { type = \"ALPHABETIC\", symbols[Group1] = [ s, S ] };\n"
    "    key <AC11> { [ dead_acute, dead_diaeresis ] };\n"
    "    key <LFSH> { [ Shift_L ] };\n"
    "    key <SPCE> { [ space ] };\n"
    "    key <LEFT> { [ Left ] };\n"
    "    modifier_map Shift { <LFSH> };\n"
    "    modifier_map Control { <LCTL> };\n"
    "};\n"
    "};\n";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} // namespace TESTWAYLAND
} // namespace GUCEF

/*-------------------------------------------------------------------------*/
