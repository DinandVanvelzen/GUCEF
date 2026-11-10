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


#ifndef GUCEF_GUIDRIVERWAYLAND_CDECORATIONPOLICY_H
#define GUCEF_GUIDRIVERWAYLAND_CDECORATIONPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUIDRIVERWAYLAND_MACROS_H
#include "guidriverWAYLAND_macros.h"
#define GUCEF_GUIDRIVERWAYLAND_MACROS_H
#endif /* GUCEF_GUIDRIVERWAYLAND_MACROS_H ? */

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
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Who draws the title bar and borders of the window */
enum EDecorationMode
{
    DECORATION_NONE = 0 ,       /**< nobody: the window is a plain rectangle */
    DECORATION_SERVER   ,       /**< the compositor (zxdg_toplevel_decoration_v1 server side) */
    DECORATION_LIBDECOR         /**< the application, with libdecor */
};

/** What is wanted from the application side */
enum EDecorationPreference
{
    DECORATIONPREFERENCE_AUTO = 0 ,
    DECORATIONPREFERENCE_NONE     ,
    DECORATIONPREFERENCE_SERVER   ,
    DECORATIONPREFERENCE_LIBDECOR
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The choice of decorations. In order: the compositor draws them when it offers server side decorations (KDE, wlroots
 *  compositors), else libdecor draws them when it is installed (GNOME, which leaves it to the application), else the window
 *  has none. The preference of the user can leave out a step.
 */
class GUIDRIVERWAYLAND_PUBLIC_CPP CDecorationPolicy
{
    public:

    static EDecorationMode Choose( EDecorationPreference preference ,
                                   bool compositorOffersServerSide  ,
                                   bool libdecorUsable              );

    /** What the window ends up with after the compositor answered a request for server side decorations: it may say it wants the client to draw them */
    static EDecorationMode Effective( EDecorationMode chosen, bool compositorChoseClientSide );

    static EDecorationPreference ParsePreference( const CORE::CString& text );

    static const char* ToString( EDecorationMode mode );

    private:

    CDecorationPolicy( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERWAYLAND_CDECORATIONPOLICY_H ? */
