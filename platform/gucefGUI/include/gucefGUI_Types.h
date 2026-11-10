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

#ifndef GUCEF_GUI_TYPES_H
#define GUCEF_GUI_TYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A rectangle in logical pixels: the coordinate space of a GUI context (its logical resolution) */
class GUCEF_GUI_PUBLIC_CPP CRect
{
    public:

    Int32 left;
    Int32 top;
    Int32 width;
    Int32 height;

    CRect( void );
    CRect( Int32 l, Int32 t, Int32 w, Int32 h );

    bool Contains( Int32 x, Int32 y ) const;
    bool Intersects( const CRect& other ) const;
    CRect Intersection( const CRect& other ) const;
    bool IsEmpty( void ) const;
    Int32 Right( void ) const;
    Int32 Bottom( void ) const;

    bool operator==( const CRect& other ) const;
    bool operator!=( const CRect& other ) const;

    /** "left,top,width,height" */
    CString ToString( void ) const;
    static bool FromString( const CString& str, CRect& rect );
};

/*-------------------------------------------------------------------------*/

/** A colour with 8 bits per channel, not premultiplied */
class GUCEF_GUI_PUBLIC_CPP CColour
{
    public:

    UInt8 r;
    UInt8 g;
    UInt8 b;
    UInt8 a;

    CColour( void );
    CColour( UInt8 red, UInt8 green, UInt8 blue, UInt8 alpha = 255 );

    bool operator==( const CColour& other ) const;
    bool operator!=( const CColour& other ) const;

    /** "r,g,b,a" with decimal channels */
    CString ToString( void ) const;

    /** Accepts "r,g,b", "r,g,b,a" and "#rrggbb" / "#rrggbbaa" */
    static bool FromString( const CString& str, CColour& colour );
};

/*-------------------------------------------------------------------------*/

/** Which mouse button, the same numbering on every platform and backend */
enum EMouseButton
{
    MOUSEBUTTON_NONE    = 0 ,
    MOUSEBUTTON_LEFT    = 1 ,
    MOUSEBUTTON_RIGHT   = 2 ,
    MOUSEBUTTON_MIDDLE  = 3 ,
    MOUSEBUTTON_X1      = 4 ,
    MOUSEBUTTON_X2      = 5
};
typedef enum EMouseButton TMouseButton;

/*-------------------------------------------------------------------------*/

/** Semantic navigation, what a gamepad, the arrow keys, Tab, Enter and Escape produce */
enum ENavigation
{
    NAVIGATION_UP       = 0 ,
    NAVIGATION_DOWN     = 1 ,
    NAVIGATION_LEFT     = 2 ,
    NAVIGATION_RIGHT    = 3 ,
    NAVIGATION_NEXT     = 4 ,
    NAVIGATION_PREVIOUS = 5 ,
    NAVIGATION_ACCEPT   = 6 ,
    NAVIGATION_CANCEL   = 7
};
typedef enum ENavigation TNavigation;

/*-------------------------------------------------------------------------*/

/** The phase in which a routed event reaches an observer */
enum EEventPhase
{
    EVENTPHASE_TUNNEL   = 0 ,   /**< from the root down to the parent of the target */
    EVENTPHASE_TARGET   = 1 ,   /**< at the target itself */
    EVENTPHASE_BUBBLE   = 2     /**< from the parent of the target up to the root */
};
typedef enum EEventPhase TEventPhase;

/*-------------------------------------------------------------------------*/

/** How the picture at the logical resolution is presented in a window of another size */
enum EPresentMode
{
    PRESENTMODE_NEAREST = 0 ,   /**< aspect kept, nearest pixel */
    PRESENTMODE_LINEAR  = 1 ,   /**< aspect kept, bilinear filtering */
    PRESENTMODE_INTEGER = 2 ,   /**< aspect kept, whole multiples only, nearest pixel */
    PRESENTMODE_STRETCH = 3     /**< fills the window, aspect not kept, bilinear */
};
typedef enum EPresentMode TPresentMode;

/*-------------------------------------------------------------------------*/

/** Where a picture of the logical resolution lands in a window of the given size for the given mode */
GUCEF_GUI_PUBLIC_CPP CRect
CalculatePresentRect( UInt32 logicalWidth  ,
                      UInt32 logicalHeight ,
                      UInt32 windowWidth   ,
                      UInt32 windowHeight  ,
                      TPresentMode mode    );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_TYPES_H ? */
