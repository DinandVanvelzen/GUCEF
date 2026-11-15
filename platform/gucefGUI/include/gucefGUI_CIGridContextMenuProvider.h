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

#ifndef GUCEF_GUI_CIGRIDCONTEXTMENUPROVIDER_H
#define GUCEF_GUI_CIGRIDCONTEXTMENUPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CCONTEXTMENUITEM_H
#include "gucefGUI_CContextMenuItem.h"
#define GUCEF_GUI_CCONTEXTMENUITEM_H
#endif /* GUCEF_GUI_CCONTEXTMENUITEM_H ? */

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

class CGridView;

/**
 *  What a grid asks the application for its context menu (CGridView::SetContextMenuProvider()): the items of the menu
 *  of a cell when the menu opens (a right click, the menu key, Shift+F10), and the choice of the user. The provider is
 *  not owned by the grid; one provider can serve several grids
 */
class GUCEF_GUI_PUBLIC_CPP CIGridContextMenuProvider
{
    public:

    CIGridContextMenuProvider( void );

    virtual ~CIGridContextMenuProvider();

    /**
     *  The items of the menu for a cell: the row in the order of the grid and the column of the data, -1 for either when
     *  the menu is not for a cell (opened over the header or below the rows). False, or no items: no menu
     */
    virtual bool GetContextMenuItems( const CGridView& grid, Int32 row, Int32 column, TContextMenuItemVector& items ) = 0;

    /** The user chose the item with the id in the menu of the cell; the menu is closed already */
    virtual void OnContextMenuItem( CGridView& grid, Int32 row, Int32 column, const CString& itemId ) = 0;

    private:

    CIGridContextMenuProvider( const CIGridContextMenuProvider& src );
    CIGridContextMenuProvider& operator=( const CIGridContextMenuProvider& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CIGRIDCONTEXTMENUPROVIDER_H ? */
