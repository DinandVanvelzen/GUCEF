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

#ifndef GUCEF_GUI_CLISTBOX_H
#define GUCEF_GUI_CLISTBOX_H

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
 *  A list of text rows. Properties: Items, SelectedIndex, MultiSelect, SelectedIndices, DisabledItems,
 *  RowHeight, Font, TextColour, DisabledTextColour, SelectionColour, HoverColour, BackgroundColour,
 *  ScrollPosition (the first row shown). Runtime: HoverIndex.
 *  A click selects a row (not a disabled one), a double click or Enter activates it (ItemActivatedEvent),
 *  the wheel and Up / Down scroll and move the selection.
 */
class GUCEF_GUI_PUBLIC_CPP CListbox : public CWidget
{
    public:

    static const CORE::CEvent SelectionChangedEvent;
    static const CORE::CEvent ItemAddedEvent;
    static const CORE::CEvent ItemRemovedEvent;
    static const CORE::CEvent ItemSelectedEvent;
    static const CORE::CEvent ItemDeselectedEvent;
    static const CORE::CEvent ItemClickedEvent;
    static const CORE::CEvent ItemDoubleClickedEvent;
    static const CORE::CEvent ItemTripleClickedEvent;

    static void RegisterEvents( void );

    public:

    typedef GUCEF::vector< CString > TStringVector;

    CListbox( void );

    virtual ~CListbox();

    virtual bool SetAllowMultiSelect( bool allowMultiselect );

    virtual bool GetAllowMultiSelect( void ) const;

    virtual UInt32 GetLineCount( void );

    virtual bool Clear( void );

    virtual bool GetLineAtIndex( const UInt32 lineIndex ,
                                 CString& lineText      ) const;

    virtual bool InsertLineAtIndex( const UInt32 lineIndex ,
                                    const CString& text    );

    virtual bool AppendLine( const CString& text );

    /** All rows joined with '\n' */
    virtual bool GetText( CString& text ) const;

    virtual bool SetLineSelection( const UInt32 lineIndex ,
                                   const bool isSelected  );

    virtual bool GetLineSelection( const UInt32 lineIndex ,
                                   bool& isSelected       ) const;

    virtual bool GetFirstSelectedLine( CString& text ) const;

    virtual bool GetLastSelectedLine( CString& text ) const;

    virtual bool GetSelectedLines( TStringVector& selectedLines ) const;

    TStringVector GetItems( void ) const;

    bool SetItems( const TStringVector& items );

    Int32 GetSelectedIndex( void ) const;

    bool SetSelectedIndex( Int32 index );

    bool IsItemDisabled( Int32 index ) const;

    bool IsItemSelected( Int32 index ) const;

    Int32 GetRowHeight( void ) const;

    Int32 GetVisibleRowCount( void ) const;

    /** The row at a position relative to the list, -1 when none */
    Int32 GetRowAt( Int32 localY ) const;

    /** Scrolls so the row is visible */
    void ScrollToRow( Int32 index );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "Listbox"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void SelectByUser( Int32 index );

    CListbox( const CListbox& src );
    CListbox& operator=( const CListbox& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLISTBOX_H ? */
