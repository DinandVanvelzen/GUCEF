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

#ifndef GUCEF_GUI_CCONTEXTMENUITEM_H
#define GUCEF_GUI_CCONTEXTMENUITEM_H

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One entry of a context menu as an application describes it: an id the application recognises the choice by, the
 *  text shown, optionally the text of the key that does the same (shown after the text, so the menu teaches the keys),
 *  whether it can be chosen, or a separator line
 */
class GUCEF_GUI_PUBLIC_CPP CContextMenuItem
{
    public:

    CString id;             /**< what the choice reports; unique within one menu */
    CString text;           /**< what the menu shows */
    CString shortcut;       /**< the key that does the same, as text ("Ctrl+D"); empty for none */
    bool enabled;           /**< false: shown, not selectable */
    bool separator;         /**< a line between groups of items; id and text are not used */

    CContextMenuItem( void );

    CContextMenuItem( const CString& itemId, const CString& itemText, const CString& shortcutText = CString::Empty, bool isEnabled = true );

    /** A separator line */
    static CContextMenuItem Separator( void );

    /** What a menu shows for the item: the text, and the shortcut after three blanks; "-" for a separator */
    CString GetDisplayText( void ) const;
};

typedef GUCEF::vector< CContextMenuItem > TContextMenuItemVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCONTEXTMENUITEM_H ? */
