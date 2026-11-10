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

#ifndef GUCEF_GUI_CPOPUPMENU_H
#define GUCEF_GUI_CPOPUPMENU_H

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
 *  A menu that opens on top of everything (as the popup of the context) and closes when an item is
 *  chosen or the user clicks elsewhere. An item "-" is a separator. Properties: Items, DisabledItems,
 *  Font, TextColour, DisabledTextColour, HoverColour, BackgroundColour, RowHeight. Runtime: HoverIndex.
 *  The menu must be a root widget (made without a parent).
 */
class GUCEF_GUI_PUBLIC_CPP CPopupMenu : public CWidget
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    CPopupMenu( void );

    virtual ~CPopupMenu();

    /** Opens the menu with its top left at the given position */
    bool ShowAt( Int32 x, Int32 y );

    bool Close( void );

    TStringVector GetItems( void ) const;

    bool SetItems( const TStringVector& items );

    bool IsItemSelectable( Int32 index ) const;

    Int32 GetRowHeight( void ) const;

    Int32 GetRowAt( Int32 localY ) const;

    virtual bool GetNaturalSize( Int32& width, Int32& height ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "PopupMenu"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CPopupMenu( const CPopupMenu& src );
    CPopupMenu& operator=( const CPopupMenu& src );
};
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CPOPUPMENU_H ? */
