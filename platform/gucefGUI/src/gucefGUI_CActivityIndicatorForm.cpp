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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CPROGRESSBAR_H
#include "gucefGUI_CProgressBar.h"
#define GUCEF_GUI_CPROGRESSBAR_H
#endif /* GUCEF_GUI_CPROGRESSBAR_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CActivityIndicatorForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CActivityIndicatorForm::CActivityIndicatorForm( void )
    : CFormEx()
    , m_service( GUCEF_NULL )
    , m_displayed( 0 )
    , m_indeterminate( false )
    , m_phaseInMs( 0 )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CActivityIndicatorForm::CActivityIndicatorForm( CIGUIContext* context )
    : CFormEx( context )
    , m_service( GUCEF_NULL )
    , m_displayed( 0 )
    , m_indeterminate( false )
    , m_phaseInMs( 0 )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CActivityIndicatorForm::~CActivityIndicatorForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CActivityIndicatorForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );

    // from the right: the button, the count; the label and the bar share what is left (see the header)
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    CORE::CDataNode* root = CFormLayout::AddWidgetNode( layout, "Panel", "ActivityIndicatorForm", "0,0 0,0", "1,0 1,0" );
    CORE::CDataNode* text = CFormLayout::AddWidgetNode( *root, "Label", "Text", "0,6 0,2", "0.5,-97 1,-4" );
    CFormLayout::SetWidgetNodeProperty( *text, EWidgetProperty_TextOverflow, "Ellipsis" );
    CORE::CDataNode* bar = CFormLayout::AddWidgetNode( *root, "ProgressBar", "Bar", "0.5,-85 0,8", "0.5,-97 1,-16" );
    CFormLayout::SetWidgetNodeProperty( *bar, EWidgetProperty_Min, "0" );
    CFormLayout::SetWidgetNodeProperty( *bar, EWidgetProperty_Max, "1" );
    CFormLayout::SetWidgetNodeProperty( *bar, EWidgetProperty_Value, "0" );
    CORE::CDataNode* count = CFormLayout::AddWidgetNode( *root, "Label", "Count", "1,-176 0,2", "0,90 1,-4" );
    CFormLayout::SetWidgetNodeProperty( *count, EWidgetProperty_HTextAlign, "Right" );
    CORE::CDataNode* cancel = CFormLayout::AddWidgetNode( *root, "Button", "Cancel", "1,-86 0,2", "0,80 1,-4" );
    CFormLayout::SetWidgetNodeProperty( *cancel, EWidgetProperty_Text, "Cancel" );
    CFormLayout::SetWidgetNodeProperty( *cancel, EWidgetProperty_Tooltip, "Stop this operation" );
    if ( !LoadLayout( layout ) )
        return false;
    Refresh();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CActivityIndicatorForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* cancel = GetWidget( "Cancel" );
    if ( GUCEF_NULL != cancel )
    {
        TEventCallback callback( this, &CActivityIndicatorForm::OnCancelActivated );
        SubscribeTo( cancel, CWidget::ActivatedEvent, callback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        TEventCallback callback( this, &CActivityIndicatorForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, callback );
        m_frameSubscribed = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CActivityIndicatorForm::SetService( CActivityService* service )
{GUCEF_TRACE;

    if ( service == m_service )
        return;
    if ( GUCEF_NULL != m_service )
        UnsubscribeFrom( m_service );
    m_service = service;
    if ( GUCEF_NULL != m_service )
    {
        TEventCallback changedCallback( this, &CActivityIndicatorForm::OnServiceChanged );
        SubscribeTo( m_service, CActivityService::ActivityChangedEvent, changedCallback );
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

CActivityService*
CActivityIndicatorForm::GetService( void ) const
{GUCEF_TRACE;

    return m_service;
}

/*-------------------------------------------------------------------------*/

void
CActivityIndicatorForm::Refresh( void )
{GUCEF_TRACE;

    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root )
        return;

    CActivityService::TActivityInfoVector shown;
    if ( GUCEF_NULL != m_service )
        m_service->GetShownOperations( shown );
    if ( shown.empty() )
    {
        m_displayed = 0;
        m_indeterminate = false;
        root->SetVisibility( false );
        return;
    }

    // the newest operation: what the user did last is what they wait for
    const CActivityInfo& info = shown.back();
    m_displayed = info.id;
    m_indeterminate = !info.determinate;
    CWidget* text = GetWidget( "Text" );
    if ( GUCEF_NULL != text )
        text->SetProperty( EWidgetProperty_Text, info.label );
    CWidget* count = GetWidget( "Count" );
    if ( GUCEF_NULL != count )
        count->SetProperty( EWidgetProperty_Text, shown.size() > 1 ? CORE::UInt32ToString( (UInt32) shown.size() ) + " operations" : CString::Empty );
    CWidget* cancel = GetWidget( "Cancel" );
    if ( GUCEF_NULL != cancel )
    {
        cancel->SetVisibility( info.cancellable );
        cancel->SetProperty( EWidgetProperty_Enabled, info.cancelRequested ? "false" : "true" );
        cancel->SetProperty( EWidgetProperty_Text, info.cancelRequested ? "Stopping..." : "Cancel" );
    }
    CProgressBar* bar = GetWidgetAs< CProgressBar >( "Bar" );
    if ( GUCEF_NULL != bar && info.determinate )
        bar->SetValue( info.progress );
    else
        ShowIndeterminatePhase();
    root->SetVisibility( true );
}

/*-------------------------------------------------------------------------*/

void
CActivityIndicatorForm::ShowIndeterminatePhase( void )
{GUCEF_TRACE;

    // to and fro: up in the first half of the period, down in the second
    CProgressBar* bar = GetWidgetAs< CProgressBar >( "Bar" );
    if ( GUCEF_NULL == bar )
        return;
    const UInt32 half = (UInt32) IndeterminatePeriodInMs / 2;
    const UInt32 phase = m_phaseInMs % (UInt32) IndeterminatePeriodInMs;
    const Float32 fraction = phase < half ? (Float32) phase / (Float32) half : (Float32) ( (UInt32) IndeterminatePeriodInMs - phase ) / (Float32) half;
    bar->SetValue( fraction );
}

/*-------------------------------------------------------------------------*/

bool
CActivityIndicatorForm::IsIndicatorShown( void ) const
{GUCEF_TRACE;

    return 0 != m_displayed;
}

/*-------------------------------------------------------------------------*/

UInt32
CActivityIndicatorForm::GetDisplayedOperation( void ) const
{GUCEF_TRACE;

    return m_displayed;
}

/*-------------------------------------------------------------------------*/

void
CActivityIndicatorForm::OnServiceChanged( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CActivityIndicatorForm::OnNotify( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventId && GUCEF_NULL != notifier && notifier == m_service )
    {
        m_service = GUCEF_NULL;
        Refresh();
        return;
    }
    CFormEx::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CActivityIndicatorForm::OnCancelActivated( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service && 0 != m_displayed )
    {
        m_service->RequestCancel( m_displayed );
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

void
CActivityIndicatorForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_indeterminate || 0 == m_displayed )
        return;
    CORE::TCloneableUInt32* frameTime = static_cast< CORE::TCloneableUInt32* >( eventData );
    if ( GUCEF_NULL != frameTime )
        m_phaseInMs = ( m_phaseInMs + frameTime->GetData() ) % (UInt32) IndeterminatePeriodInMs;
    ShowIndeterminatePhase();
}

/*-------------------------------------------------------------------------*/

const CString&
CActivityIndicatorForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CActivityIndicatorForm";
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
