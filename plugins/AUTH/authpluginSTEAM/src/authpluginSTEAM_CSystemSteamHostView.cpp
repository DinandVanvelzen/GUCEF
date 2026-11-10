/*
 *  authpluginSTEAM: gucefAUTH plugin for logging in with the Steam client
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

#include "gucef_platform.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #undef  WIN32_LEAN_AND_MEAN
  #include <windows.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )

  #include <sys/types.h>
  #include <signal.h>
  #include <errno.h>

#endif

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "authpluginSTEAM_CSystemSteamHostView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace STEAM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSystemSteamHostView::CSystemSteamHostView( void )
    : ISteamHostView()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSystemSteamHostView::~CSystemSteamHostView()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CSystemSteamHostView&
CSystemSteamHostView::Instance( void )
{GUCEF_TRACE;

    static const CSystemSteamHostView view;
    return view;
}

/*-------------------------------------------------------------------------*/

TSteamHostPlatform
CSystemSteamHostView::GetPlatform( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return STEAMHOST_WINDOWS;
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    return STEAMHOST_LINUX;
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS )
    return STEAMHOST_MACOS;
    #else
    return STEAMHOST_OTHER;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSystemSteamHostView::GetHomeDirectory( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    CORE::CString home;
    if ( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, home ) && !home.IsNULLOrEmpty() )
        return home;
    return CORE::GetEnv( "USERPROFILE" );
    #else
    // $HOME first: it is what the user's session says, and it is what a test points at a made up home
    CORE::CString home = CORE::GetEnv( "HOME" );
    if ( home.IsNULLOrEmpty() )
        CORE::TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, home );
    size_t bytes = strlen( home.C_String() );
    while ( bytes > 1 && '/' == home.C_String()[ bytes - 1 ] )
        --bytes;
    if ( bytes != strlen( home.C_String() ) )
        home = CORE::CString( home.C_String(), (UInt32) bytes );
    return home;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSystemSteamHostView::GetEnvironmentValue( const CORE::CString& name ) const
{GUCEF_TRACE;

    return CORE::GetEnv( name );
}

/*-------------------------------------------------------------------------*/

bool
CSystemSteamHostView::DirectoryExists( const CORE::CString& path ) const
{GUCEF_TRACE;

    return !path.IsNULLOrEmpty() && CORE::DirExists( path );
}

/*-------------------------------------------------------------------------*/

bool
CSystemSteamHostView::FileExists( const CORE::CString& path ) const
{GUCEF_TRACE;

    return !path.IsNULLOrEmpty() && CORE::FileExists( path );
}

/*-------------------------------------------------------------------------*/

bool
CSystemSteamHostView::ReadTextFile( const CORE::CString& path, UInt32 maxBytes, CORE::CString& content ) const
{GUCEF_TRACE;

    content.Clear();
    if ( path.IsNULLOrEmpty() || !CORE::FileExists( path ) )
        return false;
    // the size is looked at first: a file of any size is not read into memory
    const UInt32 size = CORE::FileSize( path );
    if ( size > maxBytes )
        return false;
    return CORE::LoadTextFileAsString( path, content, false );
}

/*-------------------------------------------------------------------------*/

bool
CSystemSteamHostView::IsProcessRunning( UInt32 processId, const CORE::CString& nameHint ) const
{GUCEF_TRACE;

    if ( 0 == processId )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    // the process table of the kernel: the directory of the process, and its name where that can be read
    const CORE::CString processDir = "/proc/" + CORE::UInt32ToString( processId );
    if ( !CORE::DirExists( processDir ) )
        return false;
    if ( nameHint.IsNULLOrEmpty() )
        return true;
    CORE::CString name;
    if ( !CORE::LoadTextFileAsString( processDir + "/comm", name, false ) )
        return true;
    return name.Lowercase().HasSubstr( nameHint.Lowercase(), true ) >= 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS )

    // signal 0 only asks whether the process is there; a process of another user answers EPERM
    return 0 == kill( (pid_t) processId, 0 ) || EPERM == errno;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    HANDLE process = ::OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD) processId );
    if ( NULL == process )
        return false;
    bool running = false;
    DWORD exitCode = 0;
    if ( ::GetExitCodeProcess( process, &exitCode ) && STILL_ACTIVE == exitCode )
    {
        running = true;
        if ( !nameHint.IsNULLOrEmpty() )
        {
            WCHAR imagePath[ 1024 ];
            DWORD imagePathLength = 1024;
            if ( ::QueryFullProcessImageNameW( process, 0, imagePath, &imagePathLength ) && imagePathLength > 0 )
            {
                // the Win32 wide API forces a wide string at this boundary only
                const CORE::CString imageName = CORE::ToUtf8String( std::wstring( imagePath, (size_t) imagePathLength ) );
                running = imageName.Lowercase().HasSubstr( nameHint.Lowercase(), true ) >= 0;
            }
        }
    }
    ::CloseHandle( process );
    return running;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CSystemSteamHostView::ReadRegistryString( const CORE::CString& key, const CORE::CString& valueName, CORE::CString& value ) const
{GUCEF_TRACE;

    value.Clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // the Win32 wide registry API forces wide strings at this boundary only
    const std::wstring wideKey = CORE::ToWString( key );
    const std::wstring wideValue = CORE::ToWString( valueName );
    HKEY handle = NULL;
    if ( ERROR_SUCCESS != ::RegOpenKeyExW( HKEY_CURRENT_USER, wideKey.c_str(), 0, KEY_READ, &handle ) )
        return false;
    WCHAR buffer[ 2048 ];
    DWORD bufferSize = sizeof( buffer ) - sizeof( WCHAR );
    DWORD type = 0;
    bool found = false;
    if ( ERROR_SUCCESS == ::RegQueryValueExW( handle, wideValue.c_str(), NULL, &type, (LPBYTE) buffer, &bufferSize ) && REG_SZ == type )
    {
        buffer[ bufferSize / sizeof( WCHAR ) ] = 0;
        value = CORE::ToUtf8String( std::wstring( buffer ) );
        found = !value.IsNULLOrEmpty();
    }
    ::RegCloseKey( handle );
    return found;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CSystemSteamHostView::ReadRegistryDword( const CORE::CString& key, const CORE::CString& valueName, UInt32& value ) const
{GUCEF_TRACE;

    value = 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // the Win32 wide registry API forces wide strings at this boundary only
    const std::wstring wideKey = CORE::ToWString( key );
    const std::wstring wideValue = CORE::ToWString( valueName );
    HKEY handle = NULL;
    if ( ERROR_SUCCESS != ::RegOpenKeyExW( HKEY_CURRENT_USER, wideKey.c_str(), 0, KEY_READ, &handle ) )
        return false;
    DWORD data = 0;
    DWORD dataSize = sizeof( data );
    DWORD type = 0;
    bool found = ERROR_SUCCESS == ::RegQueryValueExW( handle, wideValue.c_str(), NULL, &type, (LPBYTE) &data, &dataSize ) && REG_DWORD == type;
    ::RegCloseKey( handle );
    if ( found )
        value = (UInt32) data;
    return found;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
