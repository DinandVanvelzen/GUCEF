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

#ifndef GUCEF_GUI_CMENUBAR_H
#define GUCEF_GUI_CMENUBAR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

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

class CPopupMenu;

/**
 *  A bar of menus. Items holds one path per menu item, "File/Open": the first part is the menu on the
 *  bar, the second the item in it. Choosing an item raises ItemActivatedEvent on the bar with the whole
 *  path. Properties: Items, Font, TextColour, HoverColour, BackgroundColour. Runtime: OpenMenu, HoverIndex.
 */
class GUCEF_GUI_PUBLIC_CPP CMenuBar : public CWidget
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    CMenuBar( void );

    virtual ~CMenuBar();

    /** The menus on the bar, in the order they first appear */
    TStringVector GetMenus( void ) const;

    /** The items of a menu */
    TStringVector GetMenuItems( const CString& menu ) const;

    /** Where a menu title is, relative to the bar */
    CRect GetMenuRect( UInt32 menuIndex ) const;

    bool OpenMenu( Int32 menuIndex );

    bool CloseMenu( void );

    virtual bool GetNaturalSize( Int32& width, Int32& height ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "MenuBar"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMenuBar > TEventCallback;

    /** An item of the menu that is open was chosen */
    void OnPopupItemActivated( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    /** A form was hidden: the menu goes with it */
    void OnFormHidden( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    CMenuBar( const CMenuBar& src );
    CMenuBar& operator=( const CMenuBar& src );

    private:

    CPopupMenu* m_popup;
    CString m_openMenuName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CMENUBAR_H ? */
