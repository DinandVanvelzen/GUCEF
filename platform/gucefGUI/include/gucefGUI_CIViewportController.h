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
#ifndef GUCEF_GUI_CIVIEWPORTCONTROLLER_H
#define GUCEF_GUI_CIVIEWPORTCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the application gives a CViewportForm to move a view with: orbit, pan, zoom or pick. The form passes the pointer,
 *  the wheel and the keys of its surface on, in the coordinates of the surface (pixels from its top left corner, which can be
 *  outside the surface while a button that was pressed on it is held), and tells when the size of the surface changed. Every
 *  function does nothing by default, so a controller overrides what it uses.
 *
 *  The controller is not owned by the form and has to be there for as long as the form has it (SetController( NULL ) lets go).
 */
class GUCEF_GUI_PUBLIC_CPP CIViewportController
{
    public:

    CIViewportController( void );

    virtual ~CIViewportController();

    /** The pointer moved over the surface, or while a button pressed on it is held; 'buttonStates' has bit ( 1 << TMouseButton ) for every button held down */
    virtual void OnPointerMove( Int32 x, Int32 y, UInt32 buttonStates, UInt32 modifiers );

    /** A button went down or up on the surface */
    virtual void OnPointerButton( Int32 x, Int32 y, TMouseButton button, bool pressed, UInt32 clickCount, UInt32 modifiers );

    /** The wheel turned by 'delta' notches, positive away from the user */
    virtual void OnWheel( Int32 x, Int32 y, Int32 delta, UInt32 modifiers );

    /** A key went down or up while the surface had the keyboard focus */
    virtual void OnKey( INPUT::KeyCode keyCode, bool pressed, UInt32 modifiers );

    /** A press ended without a release (the window lost the focus, the surface was hidden): a drag in progress ends */
    virtual void OnPointerCaptureLost( void );

    /** The surface has this size now, in pixels; also told once when the size is first known */
    virtual void OnSurfaceResized( UInt32 width, UInt32 height );

    private:

    CIViewportController( const CIViewportController& src );
    CIViewportController& operator=( const CIViewportController& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CIVIEWPORTCONTROLLER_H ? */
