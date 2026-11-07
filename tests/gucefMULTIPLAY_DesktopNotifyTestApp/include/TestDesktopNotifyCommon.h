/*
 *  gucefMULTIPLAY_DesktopNotifyTestApp: Application for testing the desktop match alert sink plugin of gucefMULTIPLAY
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

#ifndef GUCEF_MULTIPLAY_DESKTOPNOTIFYTESTAPP_TESTDESKTOPNOTIFYCOMMON_H
#define GUCEF_MULTIPLAY_DESKTOPNOTIFYTESTAPP_TESTDESKTOPNOTIFYCOMMON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdarg.h>
#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H
#include "notifysinkpluginDESKTOP_INotifyLauncher.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Records the argument vectors it is given and starts nothing: the only launcher the tests of the sink use */
class CFakeNotifyLauncher : public GUCEF::MULTIPLAYPLUGIN::DESKTOP::INotifyLauncher
{
    public:

    bool available;
    bool launchResult;
    bool throwOnLaunch;
    bool throwOnAvailable;
    GUCEF::UInt32 running;
    GUCEF::UInt32 availableChecks;
    GUCEF::vector< TArgumentVector > launched;

    CFakeNotifyLauncher( void )
        : GUCEF::MULTIPLAYPLUGIN::DESKTOP::INotifyLauncher()
        , available( true ), launchResult( true ), throwOnLaunch( false ), throwOnAvailable( false ), running( 0 ), availableChecks( 0 ), launched()
    {}

    virtual bool IsAvailable( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++availableChecks;
        if ( throwOnAvailable )
            throw 1;
        return available;
    }

    virtual bool Launch( const TArgumentVector& argv ) GUCEF_VIRTUAL_OVERRIDE
    {
        launched.push_back( argv );
        if ( throwOnLaunch )
            throw 1;
        return launchResult;
    }

    virtual GUCEF::UInt32 GetRunningCount( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        return running;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** An argument vector of the given entries, the list ends with NULL */
inline GUCEF::MULTIPLAYPLUGIN::DESKTOP::INotifyLauncher::TArgumentVector
MakeArgv( const char* first, ... )
{
    GUCEF::MULTIPLAYPLUGIN::DESKTOP::INotifyLauncher::TArgumentVector argv;
    va_list args;
    va_start( args, first );
    for ( const char* entry = first; GUCEF_NULL != entry; entry = va_arg( args, const char* ) )
        argv.push_back( GUCEF::CORE::CString( entry ) );
    va_end( args );
    return argv;
}

/*-------------------------------------------------------------------------*/

/** The bytes of the string without the terminator the string keeps in its storage */
inline GUCEF::UInt32
TestByteCount( const GUCEF::CORE::CString& text )
{
    GUCEF::UInt32 size = text.ByteSize();
    return ( size > 0 ) ? size - 1 : 0;
}

/*-------------------------------------------------------------------------*/

/** Whether the bytes are well formed UTF-8: shortest forms only, no surrogates, nothing above U+10FFFF */
inline bool
TestIsValidUtf8( const GUCEF::CORE::CString& text )
{
    const unsigned char* b = (const unsigned char*) text.C_String();
    GUCEF::UInt32 n = TestByteCount( text );
    GUCEF::UInt32 i = 0;
    while ( i < n )
    {
        unsigned char c = b[ i ];
        GUCEF::UInt32 extra = 0;
        unsigned char low = 0x80, high = 0xBF;
        if ( c < 0x80 ) { ++i; continue; }
        else if ( c >= 0xC2 && c <= 0xDF ) extra = 1;
        else if ( c == 0xE0 ) { extra = 2; low = 0xA0; }
        else if ( c == 0xED ) { extra = 2; high = 0x9F; }
        else if ( c >= 0xE1 && c <= 0xEF ) extra = 2;
        else if ( c == 0xF0 ) { extra = 3; low = 0x90; }
        else if ( c == 0xF4 ) { extra = 3; high = 0x8F; }
        else if ( c >= 0xF1 && c <= 0xF3 ) extra = 3;
        else return false;
        if ( i + extra >= n ) return false;
        if ( b[ i + 1 ] < low || b[ i + 1 ] > high ) return false;
        for ( GUCEF::UInt32 k=2; k<=extra; ++k )
            if ( b[ i + k ] < 0x80 || b[ i + k ] > 0xBF ) return false;
        i += extra + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Whether the text holds a control character: a byte below 0x20, 0x7F, or the encoded forms of U+0080 to U+009F */
inline bool
TestHasControl( const GUCEF::CORE::CString& text )
{
    const unsigned char* b = (const unsigned char*) text.C_String();
    GUCEF::UInt32 n = TestByteCount( text );
    for ( GUCEF::UInt32 i=0; i<n; ++i )
    {
        if ( b[ i ] < 0x20 || 0x7F == b[ i ] )
            return true;
        if ( 0xC2 == b[ i ] && i + 1 < n && b[ i + 1 ] >= 0x80 && b[ i + 1 ] <= 0x9F )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

inline GUCEF::UInt32
TestCountOccurrences( const GUCEF::CORE::CString& text, const char* needle )
{
    GUCEF::UInt32 count = 0;
    const char* at = text.C_String();
    size_t needleLength = strlen( needle );
    while ( GUCEF_NULL != at && GUCEF_NULL != ( at = strstr( at, needle ) ) )
    {
        ++count;
        at += needleLength;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/** Loads a plugin from the directory of the module, debug or release build */
inline bool
LoadTestPlugin( const GUCEF::CORE::CString& pluginName, const GUCEF::CORE::CString& group = "test" )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    GUCEF::CORE::CPluginControl& pluginControl = GUCEF::CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, group, true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, group, true );
}

/*-------------------------------------------------------------------------*/

/** A notification of the kind for the sink: matchId is generated, the rest as given */
inline GUCEF::MULTIPLAY::CMatchAlert
MakeMatchAlert( GUCEF::MULTIPLAY::TMatchAlertKind kind, const char* title, const char* previousMover, GUCEF::UInt32 count )
{
    GUCEF::MULTIPLAY::CMatchAlert alert;
    alert.kind = kind;
    alert.matchId = "match-1";
    alert.revision = 1;
    alert.title = title;
    alert.previousMover = previousMover;
    alert.count = count;
    return alert;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_DESKTOPNOTIFYTESTAPP_TESTDESKTOPNOTIFYCOMMON_H ? */
