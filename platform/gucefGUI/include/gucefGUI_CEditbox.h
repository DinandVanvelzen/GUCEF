/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

    /** For widget types that derive from the edit box and are registered under a name of their own */
    CEditbox( const CString& widgetTypeName );

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
