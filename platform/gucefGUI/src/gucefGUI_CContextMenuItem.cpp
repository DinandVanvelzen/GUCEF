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

#include "gucefGUI_CContextMenuItem.h"

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

CContextMenuItem::CContextMenuItem( void )
    : id()
    , text()
    , shortcut()
    , enabled( true )
    , separator( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CContextMenuItem::CContextMenuItem( const CString& itemId, const CString& itemText, const CString& shortcutText, bool isEnabled )
    : id( itemId )
    , text( itemText )
    , shortcut( shortcutText )
    , enabled( isEnabled )
    , separator( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CContextMenuItem
CContextMenuItem::Separator( void )
{GUCEF_TRACE;

    CContextMenuItem item;
    item.separator = true;
    item.enabled = false;
    return item;
}

/*-------------------------------------------------------------------------*/

CString
CContextMenuItem::GetDisplayText( void ) const
{GUCEF_TRACE;

    if ( separator )
        return "-";
    if ( shortcut.IsNULLOrEmpty() )
        return text;
    return text + "   " + shortcut;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
