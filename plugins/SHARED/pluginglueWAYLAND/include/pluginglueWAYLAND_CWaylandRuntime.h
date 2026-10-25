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


#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#include "pluginglueWAYLAND_macros.h"
#define GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_MACROS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_ABI_H
#include "pluginglueWAYLAND_Abi.h"
#define GUCEF_PLUGINGLUE_WAYLAND_ABI_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_ABI_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Loads the system libraries of Wayland when they are asked for, so the plugins build and load on a system without them
 *  and report that the library is not there. A library stays loaded for the life of the process: the functions of it
 *  are kept in tables that outlive any window, and the libraries start no threads of their own.
 */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP CWaylandRuntime
{
    public:

    /** The module of a shared library by its file name ("libEGL.so.1"), NULL when it is not there; loaded once */
    static void* LoadLibraryOnce( const char* fileName );

    /** A function or object of a loaded library, NULL when it is not there */
    static void* GetSymbol( void* module, const char* name );

    /** libwayland-client.so.0 with the marshalling call of version 1.19; false (with the reason) when it is not usable */
    static bool LoadClientApi( SWaylandApi& api, CORE::CString* problem = GUCEF_NULL );

    /** libwayland-cursor.so.0 */
    static bool LoadCursorApi( SWaylandCursorApi& api );

    /** libwayland-egl.so.1 */
    static bool LoadEglWindowApi( SWaylandEglWindowApi& api );

    /** The table of the process: loaded on the first call, NULL when the library is not usable */
    static const SWaylandApi* GetClientApi( void );

    private:

    CWaylandRuntime( void );
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

#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H ? */
