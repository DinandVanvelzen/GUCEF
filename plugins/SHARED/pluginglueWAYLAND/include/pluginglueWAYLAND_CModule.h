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

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CMODULE_H
#define GUCEF_PLUGINGLUE_WAYLAND_CMODULE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#include "pluginglueWAYLAND_macros.h"
#define GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_MACROS_H ? */

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
 *  Entry points called when the library is loaded and unloaded. The connection to the compositor is let go of
 *  when the library is unloaded, which happens after the plugins that use it have gone.
 */
class CModule
{
    public:

    static bool Load( void );

    static bool Unload( void );

    private:

    CModule( void );
    CModule( const CModule& src );
    ~CModule();
    CModule& operator=( const CModule& src );
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

#endif /* GUCEF_PLUGINGLUE_WAYLAND_CMODULE_H ? */
