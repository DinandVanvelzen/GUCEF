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

#ifndef GUCEF_GUI_CTREEVIEW_H
#define GUCEF_GUI_CTREEVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

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
 *  A tree of items. Items holds one path per item with '/' between the levels ("Ships/Hulls/Scout");
 *  levels in between exist without being listed themselves. ExpandedItems holds the paths that are open.
 *  A click on the indent area of an item with children opens or closes it, elsewhere it selects the item;
 *  a double click or Enter activates it. Properties: Items, ExpandedItems, SelectedItem, Font, TextColour,
 *  SelectionColour, HoverColour, BackgroundColour, RowHeight, Indent, ScrollPosition. Runtime: HoverIndex.
 */
class GUCEF_GUI_PUBLIC_CPP CTreeview : public CWidget
{
    public:

    struct SRow
    {
        CString path;
        CString label;
        Int32 depth;
        bool hasChildren;
        bool expanded;

        SRow( void );
    };
    typedef struct SRow TRow;
    typedef GUCEF::vector< TRow > TRowVector;
    typedef GUCEF::vector< CString > TStringVector;

    CTreeview( void );

    virtual ~CTreeview();

    /** The rows that are shown (below open parents only), top to bottom */
    TRowVector GetVisibleRows( void ) const;

    bool IsExpanded( const CString& path ) const;

    bool SetExpanded( const CString& path, bool expanded );

    CString GetSelectedItem( void ) const;

    bool SetSelectedItem( const CString& path );

    Int32 GetRowHeight( void ) const;

    /** The index into GetVisibleRows() at a position relative to the tree, -1 when none */
    Int32 GetRowAt( Int32 localY ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "Treeview"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTreeview( const CTreeview& src );
    CTreeview& operator=( const CTreeview& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTREEVIEW_H ? */
