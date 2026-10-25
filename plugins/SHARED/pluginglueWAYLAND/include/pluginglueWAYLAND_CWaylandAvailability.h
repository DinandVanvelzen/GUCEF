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


#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDAVAILABILITY_H
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDAVAILABILITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#include "pluginglueWAYLAND_macros.h"
#define GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A global object the compositor offers (wl_registry.global) */
struct SWaylandGlobal
{
    UInt32 name;
    CORE::CString interfaceName;
    UInt32 version;

    SWaylandGlobal( void ) : name( 0 ), interfaceName(), version( 0 ) {}
};

typedef GUCEF::vector< SWaylandGlobal > TWaylandGlobalList;

/** What a decision about the use of Wayland came to: whether it can be used and, if not, why not in words for a log */
struct SWaylandAvailability
{
    bool available;
    CORE::CString reason;

    SWaylandAvailability( void ) : available( false ), reason() {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The decisions about the availability of Wayland, free of any connection so they can be made from fake lists of globals.
 */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP CWaylandAvailability
{
    public:

    /** The path of the socket of the compositor as libwayland resolves it: WAYLAND_DISPLAY (an absolute path, or a name in XDG_RUNTIME_DIR), "wayland-0" when unset. Empty when it cannot be resolved */
    static CORE::CString ResolveSocketPath( const char* waylandDisplay, const char* xdgRuntimeDir );

    /** The socket of the session of this process: resolved from the environment and there. Empty when there is none */
    static CORE::CString FindSessionSocket( void );

    /** Whether the compositor offers a global at that version or newer */
    static bool HasGlobal( const TWaylandGlobalList& globals, const char* interfaceName, UInt32 minimumVersion = 1 );

    /** The version of a global, 0 when it is not offered */
    static UInt32 GetGlobalVersion( const TWaylandGlobalList& globals, const char* interfaceName );

    /**
     *  A window needs wl_compositor (version 4 or newer) and xdg_wm_base; input needs a wl_seat. 'missing' lists what is
     *  not offered (comma separated), empty when everything is there
     */
    static bool HasRequiredGlobals( const TWaylandGlobalList& globals, bool needSeat, CORE::CString& missing );

    /** Joins the facts into the decision, with the reason of the first thing that fails */
    static SWaylandAvailability Decide( bool sessionSocketFound   ,
                                        bool clientLibraryLoaded  ,
                                        bool connected            ,
                                        const TWaylandGlobalList& globals ,
                                        bool needSeat             );

    private:

    CWaylandAvailability( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDAVAILABILITY_H ? */
