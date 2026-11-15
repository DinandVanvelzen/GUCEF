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

#ifndef GUCEF_GUI_ETEXTOVERFLOW_H
#define GUCEF_GUI_ETEXTOVERFLOW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_ETYPES_H
#include "gucefGUI_ETypes.h"
#define GUCEF_GUI_ETYPES_H
#endif /* GUCEF_GUI_ETYPES_H ? */

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

/**
 *  What a line of text that is wider than its room does (the TextOverflow property): it is cut at the edge (Clip), or
 *  shortened with "..." at its end, its start or in its middle. The middle suits file names and paths, which differ at
 *  both ends; the middle form keeps the extension of a file name whole. An elided text has its whole text as tooltip.
 */
enum ETextOverflow
{
    TEXTOVERFLOW_CLIP           = 0 ,
    TEXTOVERFLOW_ELLIPSISEND    = 1 ,   /**< "Ellipsis" and "EllipsisEnd" */
    TEXTOVERFLOW_ELLIPSISSTART  = 2 ,
    TEXTOVERFLOW_ELLIPSISMIDDLE = 3 ,

    TEXTOVERFLOW_COUNT
};
typedef enum ETextOverflow TTextOverflow;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** "Clip", "EllipsisEnd", "EllipsisStart", "EllipsisMiddle"; empty for a value outside the enumeration */
GUCEF_GUI_PUBLIC_CPP const CString& TextOverflowToString( ETextOverflow overflow );

/** The mode of a TextOverflow value ("Ellipsis" is "EllipsisEnd"); 'fallback' for an empty or unknown value */
GUCEF_GUI_PUBLIC_CPP ETextOverflow StringToTextOverflow( const CString& value, ETextOverflow fallback );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_ETEXTOVERFLOW_H ? */
