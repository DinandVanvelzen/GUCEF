/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_GUI_CTABCONTROL_H
#define GUCEF_GUI_CTABCONTROL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

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

class CTabContentPane;

/**
 *  Tabs along the top (the Tabs property, one entry per tab) above pages: the children of type
 *  TabContentPane, in order. The page of ActiveTab is shown. Properties: Tabs, ActiveTab, TabHeight,
 *  Font, TextColour, TabColour, ActiveTabColour, BackgroundColour. A click on a tab or Left / Right
 *  while focused switches the tab (ValueChangedEvent and ActiveTabChangedEvent).
 */
class GUCEF_GUI_PUBLIC_CPP CTabControl : public CWidget
{
    public:

    static const CORE::CEvent ActiveTabChangedEvent;
    typedef CORE::TCloneableUInt32 TActiveTabChangedEventData;

    static void RegisterEvents( void );

    public:

    CTabControl( void );

    virtual ~CTabControl();

    virtual bool SetActiveTab( const UInt32 tabIndex );

    virtual Int32 GetActiveTab( void ) const;

    virtual CTabContentPane* GetTabContentPane( const UInt32 tabIndex );

    UInt32 GetTabCount( void ) const;

    /** Where the header of a tab is, relative to the control; tabs share the width equally */
    CRect GetTabRect( UInt32 tabIndex ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "TabControl"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    /** A page added or taken away: the active one is shown, the others not */
    virtual void OnChildrenChanged( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void ShowActivePane( void );

    CTabControl( const CTabControl& src );
    CTabControl& operator=( const CTabControl& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABCONTROL_H ? */
