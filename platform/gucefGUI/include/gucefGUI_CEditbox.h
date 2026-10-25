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

#ifndef GUCEF_GUI_CEDITBOX_H
#define GUCEF_GUI_CEDITBOX_H

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
 *  One line of text the user types. Typing, Backspace and Enter are handled by gucefGUI, so they behave
 *  the same on every backend. Properties: Text, Font, TextColour, BackgroundColour, MaxLength (code
 *  points, 0 for no limit), ReadOnly, SelectAllOnFocus (the text is selected when the box gets the focus,
 *  so the first key replaces it). Runtime: CaretVisible (blinks while focused), Selected.
 */
class GUCEF_GUI_PUBLIC_CPP CEditbox : public CWidget
{
    public:

    static const CORE::CEvent SelectionChangedEvent;
    static const CORE::CEvent CaratPositionChangedEvent;
    static const CORE::CEvent TextChangedEvent;
    static const CORE::CEvent TextEnteredEvent;     /**< Enter was pressed */

    static void RegisterEvents( void );

    public:

    CEditbox( void );

    virtual ~CEditbox();

    virtual bool SetText( const CString& text );

    virtual CString GetText( void ) const;

    /** Replaces the selection (all of the text when selected) with the given text */
    virtual bool SetSelectedText( const CString& text );

    virtual bool GetSelectedText( CString& text ) const;

    virtual void Clear( void );

    virtual bool GetNaturalSize( Int32& width, Int32& height ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "Editbox"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUpdate( UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool NeedsUpdate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CString LimitText( const CString& text ) const;

    CEditbox( const CEditbox& src );
    CEditbox& operator=( const CEditbox& src );

    private:

    UInt32 m_blinkTime;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CEDITBOX_H ? */
