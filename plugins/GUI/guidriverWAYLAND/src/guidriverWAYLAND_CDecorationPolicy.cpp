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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "guidriverWAYLAND_CDecorationPolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

EDecorationMode
CDecorationPolicy::Choose( EDecorationPreference preference ,
                           bool compositorOffersServerSide  ,
                           bool libdecorUsable              )
{GUCEF_TRACE;

    if ( DECORATIONPREFERENCE_NONE == preference )
        return DECORATION_NONE;
    if ( DECORATIONPREFERENCE_LIBDECOR == preference && libdecorUsable )
        return DECORATION_LIBDECOR;
    if ( compositorOffersServerSide && DECORATIONPREFERENCE_LIBDECOR != preference )
        return DECORATION_SERVER;
    if ( libdecorUsable )
        return DECORATION_LIBDECOR;
    if ( compositorOffersServerSide )
        return DECORATION_SERVER;
    return DECORATION_NONE;
}

/*-------------------------------------------------------------------------*/

EDecorationMode
CDecorationPolicy::Effective( EDecorationMode chosen, bool compositorChoseClientSide )
{GUCEF_TRACE;

    if ( DECORATION_SERVER == chosen && compositorChoseClientSide )
        return DECORATION_NONE;
    return chosen;
}

/*-------------------------------------------------------------------------*/

EDecorationPreference
CDecorationPolicy::ParsePreference( const CORE::CString& text )
{GUCEF_TRACE;

    const CORE::CString lower = text.Lowercase();
    if ( "none" == lower ) return DECORATIONPREFERENCE_NONE;
    if ( "server" == lower ) return DECORATIONPREFERENCE_SERVER;
    if ( "libdecor" == lower ) return DECORATIONPREFERENCE_LIBDECOR;
    return DECORATIONPREFERENCE_AUTO;
}

/*-------------------------------------------------------------------------*/

const char*
CDecorationPolicy::ToString( EDecorationMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case DECORATION_SERVER :    return "server";
        case DECORATION_LIBDECOR :  return "libdecor";
        default :                   return "none";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
