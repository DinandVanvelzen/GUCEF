/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_GUI_CCOMBOBOX_H
#define GUCEF_GUI_CCOMBOBOX_H

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

class CListbox;

/**
 *  A box showing one choice; a click opens the list of Items below it (as a popup of the context),
 *  a click on a row picks it. Properties: Items, SelectedIndex, Text, Font, TextColour, BackgroundColour,
 *  DropDownRows. Runtime: Expanded.
 */
class GUCEF_GUI_PUBLIC_CPP CCombobox : public CWidget
{
    public:

    static const CORE::CEvent TextChangedEvent;

    static void RegisterEvents( void );

    public:

    typedef GUCEF::vector< CString > TStringVector;

    CCombobox( void );

    virtual ~CCombobox();

    virtual bool SetText( const CString& text );

    virtual CString GetText( void ) const;

    virtual bool SetListItems( const TStringVector& items );

    virtual bool GetListItems( TStringVector& items ) const;

    virtual bool ClearList( void );

    Int32 GetSelectedIndex( void ) const;

    bool SetSelectedIndex( Int32 index );

    /** Opens or closes the list */
    bool SetExpanded( bool expanded );

    virtual bool GetNaturalSize( Int32& width, Int32& height ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "Combobox"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CCombobox > TEventCallback;

    /** The user picked a row of the list */
    void OnListValueChanged( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

    /** A row of the list was activated */
    void OnListItemActivated( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData );

    /** A form was hidden: the list goes with it */
    void OnFormHidden( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    /** Closes the list and changes the selection to the row that was picked */
    void PickFromList( void );

    CCombobox( const CCombobox& src );
    CCombobox& operator=( const CCombobox& src );

    private:

    CListbox* m_list;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCOMBOBOX_H ? */
