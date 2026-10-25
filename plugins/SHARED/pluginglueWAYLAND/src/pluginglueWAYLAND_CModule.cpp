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

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#include "pluginglueWAYLAND_CWaylandConnection.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#include "pluginglueWAYLAND_CEventGuard.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H ? */

#include "pluginglueWAYLAND_CModule.h"

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

bool
CModule::Load( void )
{GUCEF_TRACE;

    // nothing is connected here: the connection is made when the first window or input context needs it
    CWaylandConnection::RegisterEvents();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CModule::Unload( void )
{GUCEF_TRACE;

    CWaylandConnection::Deinstance();
    // the handler of fatal signals is code of this library: it does not stay in a process that unloads it
    CEventGuard::RemoveFatalSignalReporter();
    return true;
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
