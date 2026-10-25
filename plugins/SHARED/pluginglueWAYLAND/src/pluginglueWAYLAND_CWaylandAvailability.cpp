/*
 *  pluginglueWAYLAND: Library that owns the connection to the Wayland compositor for the plugins that share it
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
#include <sys/stat.h>

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "pluginglueWAYLAND_CWaylandAvailability.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CWaylandAvailability::ResolveSocketPath( const char* waylandDisplay, const char* xdgRuntimeDir )
{GUCEF_TRACE;

    const CORE::CString display = ( GUCEF_NULL == waylandDisplay || '\0' == waylandDisplay[ 0 ] ) ? CORE::CString( "wayland-0" ) : CORE::CString( waylandDisplay );
    if ( '/' == display[ 0 ] )
        return display;
    if ( GUCEF_NULL == xdgRuntimeDir || '\0' == xdgRuntimeDir[ 0 ] )
        return CORE::CString::Empty;
    CORE::CString dir = xdgRuntimeDir;
    if ( '/' != dir[ dir.Length() - 1 ] )
        dir += '/';
    return dir + display;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWaylandAvailability::FindSessionSocket( void )
{GUCEF_TRACE;

    CORE::CString path = ResolveSocketPath( ::getenv( "WAYLAND_DISPLAY" ), ::getenv( "XDG_RUNTIME_DIR" ) );
    if ( path.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    struct stat info;
    if ( 0 != ::stat( path.C_String(), &info ) || !S_ISSOCK( info.st_mode ) )
        return CORE::CString::Empty;
    return path;
}

/*-------------------------------------------------------------------------*/

UInt32
CWaylandAvailability::GetGlobalVersion( const TWaylandGlobalList& globals, const char* interfaceName )
{GUCEF_TRACE;

    UInt32 version = 0;
    for ( UInt32 i=0; i<globals.size(); ++i )
        if ( globals[ i ].interfaceName == interfaceName && globals[ i ].version > version )
            version = globals[ i ].version;
    return version;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandAvailability::HasGlobal( const TWaylandGlobalList& globals, const char* interfaceName, UInt32 minimumVersion )
{GUCEF_TRACE;

    return GetGlobalVersion( globals, interfaceName ) >= minimumVersion && 0 != GetGlobalVersion( globals, interfaceName );
}

/*-------------------------------------------------------------------------*/

bool
CWaylandAvailability::HasRequiredGlobals( const TWaylandGlobalList& globals, bool needSeat, CORE::CString& missing )
{GUCEF_TRACE;

    missing.Clear();
    if ( !HasGlobal( globals, "wl_compositor", 4 ) )
        missing = "wl_compositor (version 4 or newer)";
    if ( !HasGlobal( globals, "xdg_wm_base", 1 ) )
        missing += ( missing.IsNULLOrEmpty() ? "" : ", " ) + CORE::CString( "xdg_wm_base" );
    if ( needSeat && !HasGlobal( globals, "wl_seat", 1 ) )
        missing += ( missing.IsNULLOrEmpty() ? "" : ", " ) + CORE::CString( "wl_seat" );
    return missing.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

SWaylandAvailability
CWaylandAvailability::Decide( bool sessionSocketFound              ,
                              bool clientLibraryLoaded             ,
                              bool connected                       ,
                              const TWaylandGlobalList& globals    ,
                              bool needSeat                        )
{GUCEF_TRACE;

    SWaylandAvailability result;
    if ( !sessionSocketFound )
    {
        result.reason = "there is no Wayland session: WAYLAND_DISPLAY does not name a socket of the compositor";
        return result;
    }
    if ( !clientLibraryLoaded )
    {
        result.reason = "libwayland-client.so.0 (version 1.19 or newer) could not be loaded";
        return result;
    }
    if ( !connected )
    {
        result.reason = "the compositor did not accept the connection";
        return result;
    }
    CORE::CString missing;
    if ( !HasRequiredGlobals( globals, needSeat, missing ) )
    {
        result.reason = "the compositor does not offer " + missing;
        return result;
    }
    result.available = true;
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
