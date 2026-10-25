/*
 *  guidriverWAYLAND: GUCEF window driver for Wayland: native windows with OpenGL through EGL
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


#ifndef GUCEF_GUIDRIVERWAYLAND_CWAYLANDDISPLAYMODES_H
#define GUCEF_GUIDRIVERWAYLAND_CWAYLANDDISPLAYMODES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CVIDEOOPTIONS_H
#include "gucefGUI_CVideoOptions.h"
#define GUCEF_GUI_CVIDEOOPTIONS_H
#endif /* GUCEF_GUI_CVIDEOOPTIONS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#include "pluginglueWAYLAND_CWaylandConnection.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_MACROS_H
#include "guidriverWAYLAND_macros.h"
#define GUCEF_GUIDRIVERWAYLAND_MACROS_H
#endif /* GUCEF_GUIDRIVERWAYLAND_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the monitors of the compositor say, as the display modes of CVideoOptions. There is no primary monitor in Wayland:
 *  the one at the origin of the layout (that is where compositors put the primary one) stands for it, else the first.
 *
 *  A mode is the size of the monitor in surface units where the compositor told it (xdg_output: the physical mode divided by
 *  its scale, which is the size of a full screen window), else the physical mode. The depth is 24 as for X11, the
 *  frequency the refresh rate of the mode rounded to whole frames a second. A window cannot change the mode of a
 *  monitor in Wayland: the modes are the monitor as it is now (and the modes it advertises, which are normally just that one).
 */
class GUIDRIVERWAYLAND_PUBLIC_CPP CWaylandDisplayModes
{
    public:

    /** The monitor that stands for the display; NULL when there are none */
    static const PLUGINGLUE::WAYLAND::SWaylandOutput* SelectPrimaryOutput( const PLUGINGLUE::WAYLAND::CWaylandConnection& connection );

    /** The mode the primary monitor is in */
    static bool GetCurrentMode( const PLUGINGLUE::WAYLAND::CWaylandConnection& connection, GUI::CVideoOptions::TDisplayMode& mode );

    /** The modes of the primary monitor, the current one included */
    static bool GetOptions( const PLUGINGLUE::WAYLAND::CWaylandConnection& connection, GUI::CVideoOptions& options );

    /** Refresh rate in millihertz as whole frames a second, 60 when the compositor said nothing */
    static UInt32 FrequencyOf( Int32 refreshInMilliHertz );

    /** The source CVideoOptions asks when the application chose Wayland: connects for the duration of the question */
    static const GUI::CVideoOptions::SDisplayModeSource& GetSource( void );

    private:

    CWaylandDisplayModes( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERWAYLAND_CWAYLANDDISPLAYMODES_H ? */
