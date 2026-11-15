/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include "gucefGUI_CIViewportController.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIViewportController::CIViewportController( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIViewportController::~CIViewportController()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIViewportController::OnPointerMove( Int32 x, Int32 y, UInt32 buttonStates, UInt32 modifiers )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIViewportController::OnPointerButton( Int32 x, Int32 y, TMouseButton button, bool pressed, UInt32 clickCount, UInt32 modifiers )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIViewportController::OnWheel( Int32 x, Int32 y, Int32 delta, UInt32 modifiers )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIViewportController::OnKey( INPUT::KeyCode keyCode, bool pressed, UInt32 modifiers )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIViewportController::OnPointerCaptureLost( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIViewportController::OnSurfaceResized( UInt32 width, UInt32 height )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
