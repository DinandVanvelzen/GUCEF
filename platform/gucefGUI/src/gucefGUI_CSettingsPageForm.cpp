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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#include "gucefGUI_CSettingsPageForm.h"

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

const CORE::CEvent CSettingsPageForm::PageChangedEvent = "GUCEF::GUI::CSettingsPageForm::PageChangedEvent";
const CORE::CEvent CSettingsPageForm::SettingsAppliedEvent = "GUCEF::GUI::CSettingsPageForm::SettingsAppliedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CSettingsPageForm::RegisterEvents( void )
{GUCEF_TRACE;

    PageChangedEvent.Initialize();
    SettingsAppliedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CSettingsPageForm::CSettingsPageForm( void )
    : CFormEx()
    , m_hasChanges( false )
    , m_readingSettings( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSettingsPageForm::~CSettingsPageForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CSettingsPageForm::BuildPage( CWidget* parent )
{GUCEF_TRACE;

    if ( GUCEF_NULL != parent )
    {
        SetContext( parent->GetContext() );
        SetParent( parent );
    }

    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;

    ReadSettings();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSettingsPageForm::HasChanges( void ) const
{GUCEF_TRACE;

    return m_hasChanges;
}

/*-------------------------------------------------------------------------*/

void
CSettingsPageForm::SetHasChanges( bool hasChanges )
{GUCEF_TRACE;

    m_hasChanges = hasChanges;
}

/*-------------------------------------------------------------------------*/

void
CSettingsPageForm::SetReadingSettings( bool readingSettings )
{GUCEF_TRACE;

    m_readingSettings = readingSettings;
}

/*-------------------------------------------------------------------------*/

bool
CSettingsPageForm::IsReadingSettings( void ) const
{GUCEF_TRACE;

    return m_readingSettings;
}

/*-------------------------------------------------------------------------*/

CString
CSettingsPageForm::GetWidgetProperty( const CString& widgetName, EWidgetProperty property )
{GUCEF_TRACE;

    CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL == widget )
        return CString::Empty;
    return widget->GetProperty( property );
}

/*-------------------------------------------------------------------------*/

CString
CSettingsPageForm::GetWidgetProperty( const CString& widgetName, const CString& propertyName )
{GUCEF_TRACE;

    CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL == widget )
        return CString::Empty;
    return widget->GetProperty( propertyName );
}

/*-------------------------------------------------------------------------*/

bool
CSettingsPageForm::SetWidgetProperty( const CString& widgetName, EWidgetProperty property, const CString& value )
{GUCEF_TRACE;

    CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL == widget )
        return false;
    return widget->SetProperty( property, value );
}

/*-------------------------------------------------------------------------*/

bool
CSettingsPageForm::SetWidgetProperty( const CString& widgetName, const CString& propertyName, const CString& value )
{GUCEF_TRACE;

    CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL == widget )
        return false;
    return widget->SetProperty( propertyName, value );
}

/*-------------------------------------------------------------------------*/

void
CSettingsPageForm::OnValueChanged( CWidget& widget )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSettingsPageForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    // the value changes of every widget of the page bubble up to its root
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback valueChangedCallback( this, &CSettingsPageForm::OnRootValueChanged );
        SubscribeTo( root, CWidget::ValueChangedEvent, valueChangedCallback );
    }
}

/*-------------------------------------------------------------------------*/

void
CSettingsPageForm::OnRootValueChanged( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;

    // the data of this event is always a CValueChangedEventData
    const CValueChangedEventData* changed = static_cast< const CValueChangedEventData* >( eventData );
    if ( EVENTPHASE_TUNNEL == changed->phase || GUCEF_NULL == changed->source )
        return;
    if ( EVENTPHASE_TARGET == changed->phase && changed->source != GetRootWidget() )
        return;

    // where a list or the page is scrolled to is not a setting
    if ( changed->property == EWidgetProperty_ScrollPosition )
        return;

    // the page reacts to changes made while settings are read too (lists that depend on other values)
    OnValueChanged( *changed->source );
    if ( !m_readingSettings )
    {
        m_hasChanges = true;
        NotifyObservers( PageChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CSettingsPageForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CSettingsPageForm";
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
