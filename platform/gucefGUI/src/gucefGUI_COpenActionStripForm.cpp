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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#include "gucefGUI_COpenActionStripForm.h"

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

const CORE::CEvent COpenActionStripForm::ActionRequestedEvent = "GUCEF::GUI::COpenActionStripForm::ActionRequestedEvent";
const CORE::CEvent COpenActionStripForm::RecentChosenEvent = "GUCEF::GUI::COpenActionStripForm::RecentChosenEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
COpenActionStripForm::RegisterEvents( void )
{GUCEF_TRACE;

    ActionRequestedEvent.Initialize();
    RecentChosenEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

COpenActionStripForm::COpenActionStripForm( CIGUIContext* context )
    : CFormEx( context )
    , m_actions()
    , m_recentLabel( "Open recent..." )
    , m_recent( GUCEF_NULL )
    , m_updatingWidgets( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

COpenActionStripForm::~COpenActionStripForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
COpenActionStripForm::AddAction( const CString& id, const CString& label, const CString& tooltip )
{GUCEF_TRACE;

    TAction action;
    action.id = id;
    action.label = label;
    action.tooltip = tooltip;
    m_actions.push_back( action );
}

/*-------------------------------------------------------------------------*/

void
COpenActionStripForm::SetRecentLabel( const CString& label )
{GUCEF_TRACE;

    m_recentLabel = label;
    FillRecent();
}

/*-------------------------------------------------------------------------*/

CString
COpenActionStripForm::MakeFraction( UInt32 numerator, UInt32 denominator )
{GUCEF_TRACE;

    // a scale of a layout is a decimal number: thousandths are exact enough for a row of buttons
    const UInt32 thousandths = denominator > 0 ? ( numerator * 1000 ) / denominator : 0;
    CString digits = CORE::UInt32ToString( thousandths % 1000 );
    while ( digits.Length() < 3 )
        digits = "0" + digits;
    return CORE::UInt32ToString( thousandths / 1000 ) + "." + digits;
}

/*-------------------------------------------------------------------------*/

bool
COpenActionStripForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder || m_actions.empty() )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );

    // the buttons share the first row, the list of recent places takes the second
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    CORE::CDataNode* strip = CFormLayout::AddWidgetNode( layout, "Panel", "OpenActionStrip", "0,0 0,0", "1,0 1,0" );
    // the buttons share the width by the length of their text, with a little more for the edges, so that none of the texts is cut off
    const UInt32 count = (UInt32) m_actions.size();
    UInt32 totalWeight = 0;
    for ( UInt32 i=0; i<count; ++i )
        totalWeight += m_actions[ i ].label.Length() + 6;
    UInt32 start = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 weight = m_actions[ i ].label.Length() + 6;
        const CString position = MakeFraction( start, totalWeight ) + ",0 0,0";
        const CString size = MakeFraction( weight, totalWeight ) + ",-" + CORE::UInt32ToString( i+1 < count ? ROW_GAP : 0 ) + " 0," + CORE::UInt32ToString( ROW_HEIGHT );
        start += weight;
        CORE::CDataNode* button = CFormLayout::AddWidgetNode( *strip, "Button", "Action" + CORE::UInt32ToString( i ), position, size );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Text, m_actions[ i ].label );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Tooltip, m_actions[ i ].tooltip );
    }
    CORE::CDataNode* recent = CFormLayout::AddWidgetNode( *strip, "Combobox", "Recent", "0,0 0," + CORE::UInt32ToString( ROW_HEIGHT + ROW_GAP ), "1,0 0," + CORE::UInt32ToString( ROW_HEIGHT ) );
    CFormLayout::SetWidgetNodeProperty( *recent, EWidgetProperty_Tooltip, "The folders, archives and files that were opened lately" );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
COpenActionStripForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();
    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
    {
        CWidget* button = GetWidget( "Action" + CORE::UInt32ToString( i ) );
        if ( GUCEF_NULL != button )
        {
            TEventCallback callback( this, &COpenActionStripForm::OnActionActivated );
            SubscribeTo( button, CWidget::ActivatedEvent, callback );
        }
    }
    CWidget* recent = GetWidget( "Recent" );
    if ( GUCEF_NULL != recent )
    {
        TEventCallback callback( this, &COpenActionStripForm::OnRecentValueChanged );
        SubscribeTo( recent, CWidget::ValueChangedEvent, callback );
    }
    FillRecent();
}

/*-------------------------------------------------------------------------*/

void
COpenActionStripForm::SetRecentLocations( CRecentLocations* recent )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_recent )
        UnsubscribeFrom( m_recent );
    m_recent = recent;
    if ( GUCEF_NULL != m_recent )
    {
        TEventCallback callback( this, &COpenActionStripForm::OnRecentChanged );
        SubscribeTo( m_recent, CRecentLocations::ChangedEvent, callback );
    }
    FillRecent();
}

/*-------------------------------------------------------------------------*/

CRecentLocations*
COpenActionStripForm::GetRecentLocations( void ) const
{GUCEF_TRACE;

    return m_recent;
}

/*-------------------------------------------------------------------------*/

void
COpenActionStripForm::FillRecent( void )
{GUCEF_TRACE;

    CCombobox* list = GetWidgetAs< CCombobox >( "Recent" );
    if ( GUCEF_NULL == list )
        return;
    CCombobox::TStringVector rows;
    rows.push_back( GUCEF_NULL != m_recent && m_recent->GetCount() > 0 ? m_recentLabel : m_recentLabel + " (nothing yet)" );
    if ( GUCEF_NULL != m_recent )
    {
        for ( UInt32 i=0; i<m_recent->GetCount(); ++i )
        {
            const CString& path = m_recent->Get( i );
            rows.push_back( CRecentLocations::GetShortName( path ) + "   " + path );
        }
    }
    m_updatingWidgets = true;
    list->SetListItems( rows );
    list->SetSelectedIndex( 0 );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

bool
COpenActionStripForm::RequestAction( const CString& id )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_actions.size(); ++i )
    {
        if ( m_actions[ i ].id == id )
        {
            CORE::TCloneableString data( id );
            NotifyObservers( ActionRequestedEvent, &data );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
COpenActionStripForm::ChooseRecent( UInt32 index )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_recent || index >= m_recent->GetCount() )
        return false;
    CORE::TCloneableString data( m_recent->Get( index ) );
    NotifyObservers( RecentChosenEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

const COpenActionStripForm::TActionVector&
COpenActionStripForm::GetActions( void ) const
{GUCEF_TRACE;

    return m_actions;
}

/*-------------------------------------------------------------------------*/

CWidget*
COpenActionStripForm::GetActionWidget( const CString& id )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_actions.size(); ++i )
        if ( m_actions[ i ].id == id )
            return GetWidget( "Action" + CORE::UInt32ToString( (UInt32) i ) );
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
COpenActionStripForm::GetRecentRowCount( void )
{GUCEF_TRACE;

    CCombobox* list = GetWidgetAs< CCombobox >( "Recent" );
    CCombobox::TStringVector rows;
    return GUCEF_NULL != list && list->GetListItems( rows ) ? (UInt32) rows.size() : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
COpenActionStripForm::GetPreferredHeight( void )
{GUCEF_TRACE;

    return PREFERRED_HEIGHT;
}

/*-------------------------------------------------------------------------*/

void
COpenActionStripForm::OnActionActivated( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
    {
        if ( notifier == GetWidget( "Action" + CORE::UInt32ToString( i ) ) )
        {
            RequestAction( m_actions[ i ].id );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
COpenActionStripForm::OnRecentValueChanged( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CCombobox* list = GetWidgetAs< CCombobox >( "Recent" );
    if ( m_updatingWidgets || GUCEF_NULL == list )
        return;
    const Int32 selected = list->GetSelectedIndex();
    if ( selected <= 0 )
        return;

    // the first row is shown again, so the same place can be chosen once more
    m_updatingWidgets = true;
    list->SetSelectedIndex( 0 );
    m_updatingWidgets = false;
    ChooseRecent( (UInt32) selected - 1 );
}

/*-------------------------------------------------------------------------*/

void
COpenActionStripForm::OnRecentChanged( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    FillRecent();
}

/*-------------------------------------------------------------------------*/

const CString&
COpenActionStripForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::COpenActionStripForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
