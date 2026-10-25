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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#include "gucefGUI_CCombobox.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CCombobox::TextChangedEvent = "GUCEF::GUI::CCombobox::TextChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CCombobox::RegisterEvents( void )
{GUCEF_TRACE;

    TextChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CCombobox::CCombobox( void )
    : CWidget( "Combobox" )
    , m_list( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CCombobox::~CCombobox()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_list && GUCEF_NULL != GetContext() )
        GetContext()->DestroyWidget( m_list );
    m_list = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWidget*
CCombobox::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CCombobox();
}

/*-------------------------------------------------------------------------*/

bool
CCombobox::SetText( const CString& text )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

CString
CCombobox::GetText( void ) const
{GUCEF_TRACE;

    return GetProperty( EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

bool
CCombobox::SetListItems( const TStringVector& items )
{GUCEF_TRACE;

    return SetStringListProperty( EWidgetProperty_Items, items );
}

/*-------------------------------------------------------------------------*/

bool
CCombobox::GetListItems( TStringVector& items ) const
{GUCEF_TRACE;

    items = GetStringListProperty( EWidgetProperty_Items );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCombobox::ClearList( void )
{GUCEF_TRACE;

    SetIntProperty( EWidgetProperty_SelectedIndex, -1 );
    return SetProperty( EWidgetProperty_Items, CString::Empty );
}

/*-------------------------------------------------------------------------*/

Int32
CCombobox::GetSelectedIndex( void ) const
{GUCEF_TRACE;

    return GetIntProperty( EWidgetProperty_SelectedIndex );
}

/*-------------------------------------------------------------------------*/

bool
CCombobox::SetSelectedIndex( Int32 index )
{GUCEF_TRACE;

    return SetIntProperty( EWidgetProperty_SelectedIndex, index );
}

/*-------------------------------------------------------------------------*/

bool
CCombobox::SetExpanded( bool expanded )
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context )
        return false;

    if ( !expanded )
    {
        if ( GUCEF_NULL != m_list && context->GetPopup() == m_list )
            context->ClosePopup();
        return SetBoolProperty( EWidgetProperty_Expanded, false );
    }

    if ( GUCEF_NULL == m_list )
    {
        m_list = static_cast< CListbox* >( context->CreateWidget( "Listbox", GetName() + "#List", GUCEF_NULL ) );
        if ( GUCEF_NULL == m_list )
            return false;
        TEventCallback valueChangedCallback( this, &CCombobox::OnListValueChanged );
        SubscribeTo( m_list, CWidget::ValueChangedEvent, valueChangedCallback );
        TEventCallback itemActivatedCallback( this, &CCombobox::OnListItemActivated );
        SubscribeTo( m_list, CWidget::ItemActivatedEvent, itemActivatedCallback );
        TEventCallback formHiddenCallback( this, &CCombobox::OnFormHidden );
        SubscribeTo( context, CGUIContext::FormHiddenEvent, formHiddenCallback );
    }
    TStringVector items = GetStringListProperty( EWidgetProperty_Items );
    Int32 rows = GetIntProperty( EWidgetProperty_DropDownRows );
    if ( rows <= 0 || rows > (Int32) items.size() )
        rows = (Int32) items.size();
    m_list->SetItems( items );
    m_list->SetProperty( EWidgetProperty_Font, GetProperty( EWidgetProperty_Font ) );
    m_list->SetProperty( EWidgetProperty_TextColour, GetProperty( EWidgetProperty_TextColour ) );
    m_list->SetProperty( EWidgetProperty_BackgroundColour, GetProperty( EWidgetProperty_BackgroundColour ) );
    m_list->SetSelectedIndex( GetSelectedIndex() );
    m_list->SetSize( CUVector( CUDim( 0.0f, GetRect().width ), CUDim( 0.0f, rows * m_list->GetRowHeight() ) ) );
    context->ShowPopup( m_list, GetRect().left, GetRect().Bottom() );
    return SetBoolProperty( EWidgetProperty_Expanded, true );
}

/*-------------------------------------------------------------------------*/

bool
CCombobox::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    width = height = 0;
    if ( GUCEF_NULL == GetContext() )
        return false;
    TStringVector items = GetStringListProperty( EWidgetProperty_Items );
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
    {
        Int32 w = 0, h = 0;
        GetContext()->MeasureText( GetProperty( EWidgetProperty_Font ), items[ i ], 0, w, h );
        if ( w > width ) width = w;
    }
    width += 24;
    height = GetContext()->GetLineHeight( GetProperty( EWidgetProperty_Font ) ) + 4;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCombobox::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( property == EWidgetProperty_SelectedIndex )
    {
        TStringVector items = GetStringListProperty( EWidgetProperty_Items );
        Int32 index = CORE::StringToInt32( newValue, -1 );
        SetProperty( EWidgetProperty_Text, index >= 0 && index < (Int32) items.size() ? items[ index ] : CString::Empty );
    }
    else
    if ( property == EWidgetProperty_Text )
        NotifyObservers( TextChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CCombobox::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( ActivatedEvent == eventId || ( MouseClickEvent == eventId && MOUSEBUTTON_LEFT == static_cast< CPointerEventData* >( &eventData )->button ) )
    {
        SetExpanded( !GetBoolProperty( EWidgetProperty_Expanded ) );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CCombobox::PickFromList( void )
{GUCEF_TRACE;

    Int32 picked = m_list->GetSelectedIndex();
    SetExpanded( false );
    ChangeValueByUser( EWidgetProperty_SelectedIndex, CORE::Int32ToString( picked ) );
}

/*-------------------------------------------------------------------------*/

void
CCombobox::OnListValueChanged( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_list )
        return;

    // scrolling the list is not picking from it
    const CValueChangedEventData* changed = static_cast< const CValueChangedEventData* >( eventData );
    if ( GUCEF_NULL != changed && changed->property == EWidgetProperty_ScrollPosition )
        return;
    PickFromList();
}

/*-------------------------------------------------------------------------*/

void
CCombobox::OnListItemActivated( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_list )
        PickFromList();
}

/*-------------------------------------------------------------------------*/

void
CCombobox::OnFormHidden( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_list && GUCEF_NULL != GetContext() && GetContext()->GetPopup() != m_list )
        SetBoolProperty( EWidgetProperty_Expanded, false );
}

/*-------------------------------------------------------------------------*/

const CString&
CCombobox::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CCombobox";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
