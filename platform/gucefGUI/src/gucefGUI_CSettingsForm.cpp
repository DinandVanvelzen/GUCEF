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

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#ifndef GUCEF_GUI_CTABCONTENTPANE_H
#include "gucefGUI_CTabContentPane.h"
#define GUCEF_GUI_CTABCONTENTPANE_H
#endif /* GUCEF_GUI_CTABCONTENTPANE_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGSFORM_H
#include "gucefGUI_CVideoSettingsForm.h"
#define GUCEF_GUI_CVIDEOSETTINGSFORM_H
#endif /* GUCEF_GUI_CVIDEOSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CGENERALSETTINGSFORM_H
#include "gucefGUI_CGeneralSettingsForm.h"
#define GUCEF_GUI_CGENERALSETTINGSFORM_H
#endif /* GUCEF_GUI_CGENERALSETTINGSFORM_H ? */

#include "gucefGUI_CSettingsForm.h"

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

const CORE::CEvent CSettingsForm::SettingsAppliedEvent = "GUCEF::GUI::CSettingsForm::SettingsAppliedEvent";
const CORE::CEvent CSettingsForm::ClosedEvent = "GUCEF::GUI::CSettingsForm::ClosedEvent";

namespace {

CORE::CTFactory< CSettingsPageForm, CVideoSettingsForm, MT::CMutex > g_videoSettingsPageFactory;
CORE::CTFactory< CSettingsPageForm, CGeneralSettingsForm, MT::CMutex > g_generalSettingsPageFactory;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CSettingsForm::RegisterEvents( void )
{GUCEF_TRACE;

    SettingsAppliedEvent.Initialize();
    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CSettingsForm::CSettingsForm( void )
    : CFormEx()
    , m_title( "Settings" )
    , m_pages()
    , m_ownedPages()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSettingsForm::CSettingsForm( CIGUIContext* context )
    : CFormEx( context )
    , m_title( "Settings" )
    , m_pages()
    , m_ownedPages()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSettingsForm::~CSettingsForm()
{GUCEF_TRACE;

    // the pages built their widgets in the window of this form: they are taken out of it before it goes,
    // so a page of the application keeps widgets of its own to destroy
    UnsubscribeAllFromObserver();
    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
        m_pages[ i ]->SetParent( (CWidget*) GUCEF_NULL );
    m_pages.clear();
    m_ownedPages.clear();
}

/*-------------------------------------------------------------------------*/

CSettingsPageFactory&
CSettingsForm::GetPageFactory( void )
{GUCEF_TRACE;

    static CSettingsPageFactory pageFactory;
    return pageFactory;
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::RegisterStandardPages( void )
{GUCEF_TRACE;

    GetPageFactory().RegisterConcreteFactory( "Video", &g_videoSettingsPageFactory );
    GetPageFactory().RegisterConcreteFactory( "General", &g_generalSettingsPageFactory );
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::UnregisterStandardPages( void )
{GUCEF_TRACE;

    GetPageFactory().UnregisterConcreteFactory( "Video" );
    GetPageFactory().UnregisterConcreteFactory( "General" );
}

/*-------------------------------------------------------------------------*/

CSettingsPageForm*
CSettingsForm::AddPage( const CString& pageTypeName )
{GUCEF_TRACE;

    TSettingsPageFormPtr page = GetPageFactory().Create( pageTypeName );
    if ( page.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SettingsForm: There is no settings page of type \"" << pageTypeName << "\"" );
        return GUCEF_NULL;
    }
    m_ownedPages.push_back( page );
    AddPage( page.GetPointerAlways() );
    return page.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

bool
CSettingsForm::AddPage( CSettingsPageForm* page )
{GUCEF_TRACE;

    if ( GUCEF_NULL == page )
        return false;
    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
        if ( m_pages[ i ] == page )
            return false;
    m_pages.push_back( page );
    return true;
}

/*-------------------------------------------------------------------------*/

const CSettingsForm::TPageVector&
CSettingsForm::GetPages( void ) const
{GUCEF_TRACE;

    return m_pages;
}

/*-------------------------------------------------------------------------*/

CSettingsPageForm*
CSettingsForm::GetPage( const CString& title ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
        if ( m_pages[ i ]->GetTitle() == title )
            return m_pages[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::SetTitle( const CString& title )
{GUCEF_TRACE;

    m_title = title;
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
        root->SetProperty( EWidgetProperty_Title, title );
}

/*-------------------------------------------------------------------------*/

const CString&
CSettingsForm::GetTitle( void ) const
{GUCEF_TRACE;

    return m_title;
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "SettingsForm", "0.08,0 0.06,0", "0.84,0 0.88,0" );
    CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, m_title );

    CString tabs;
    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
    {
        if ( i > 0 )
            tabs += '\n';
        tabs += m_pages[ i ]->GetTitle();
    }
    CORE::CDataNode* pages = CFormLayout::AddWidgetNode( *window, "TabControl", "Pages", "0,16 0,56", "1,-32 1,-128" );
    CFormLayout::SetWidgetNodeProperty( *pages, EWidgetProperty_Tabs, tabs );
    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
        CFormLayout::AddWidgetNode( *pages, "TabContentPane", "Page" + CORE::UInt32ToString( i ) );

    CORE::CDataNode* ok = CFormLayout::AddWidgetNode( *window, "Button", "Ok", "1,-376 1,-56", "0,112 0,38" );
    CFormLayout::SetWidgetNodeProperty( *ok, EWidgetProperty_Text, "OK" );
    CFormLayout::SetWidgetNodeProperty( *ok, EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* apply = CFormLayout::AddWidgetNode( *window, "Button", "Apply", "1,-252 1,-56", "0,112 0,38" );
    CFormLayout::SetWidgetNodeProperty( *apply, EWidgetProperty_Text, "Apply" );
    CORE::CDataNode* cancel = CFormLayout::AddWidgetNode( *window, "Button", "Cancel", "1,-128 1,-56", "0,112 0,38" );
    CFormLayout::SetWidgetNodeProperty( *cancel, EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CSettingsForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CTabControl* tabs = GetWidgetAs< CTabControl >( "Pages" );
    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
    {
        CSettingsPageForm* page = m_pages[ i ];
        CTabContentPane* pane = GUCEF_NULL != tabs ? tabs->GetTabContentPane( i ) : GUCEF_NULL;
        if ( GUCEF_NULL == pane || !page->BuildPage( pane ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SettingsForm: Cannot build the settings page \"" << page->GetTitle() << "\"" );
            continue;
        }
        TEventCallback pageChangedCallback( this, &CSettingsForm::OnPageChanged );
        SubscribeTo( page, CSettingsPageForm::PageChangedEvent, pageChangedCallback );
    }

    CWidget* ok = GetWidget( "Ok" );
    if ( GUCEF_NULL != ok )
    {
        TEventCallback okCallback( this, &CSettingsForm::OnOkActivated );
        SubscribeTo( ok, CWidget::ActivatedEvent, okCallback );
    }
    CWidget* apply = GetWidget( "Apply" );
    if ( GUCEF_NULL != apply )
    {
        TEventCallback applyCallback( this, &CSettingsForm::OnApplyActivated );
        SubscribeTo( apply, CWidget::ActivatedEvent, applyCallback );
    }
    CWidget* cancel = GetWidget( "Cancel" );
    if ( GUCEF_NULL != cancel )
    {
        TEventCallback cancelCallback( this, &CSettingsForm::OnCancelActivated );
        SubscribeTo( cancel, CWidget::ActivatedEvent, cancelCallback );
    }
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback navigationCallback( this, &CSettingsForm::OnNavigation );
        SubscribeTo( root, CWidget::NavigationEvent, navigationCallback );
    }
    UpdateButtons();
}

/*-------------------------------------------------------------------------*/

bool
CSettingsForm::HasChanges( void ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
        if ( m_pages[ i ]->HasChanges() )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::UpdateButtons( void )
{GUCEF_TRACE;

    CWidget* apply = GetWidget( "Apply" );
    if ( GUCEF_NULL != apply )
        apply->SetEnabled( HasChanges() );
}

/*-------------------------------------------------------------------------*/

bool
CSettingsForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
        m_pages[ i ]->ReadSettings();
    UpdateButtons();
    return ShowModal();
}

/*-------------------------------------------------------------------------*/

bool
CSettingsForm::Open( const CString& pageTitle )
{GUCEF_TRACE;

    if ( !Open() )
        return false;
    SelectPage( pageTitle );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSettingsForm::SelectPage( const CString& title )
{GUCEF_TRACE;

    CTabControl* tabs = GetWidgetAs< CTabControl >( "Pages" );
    if ( GUCEF_NULL == tabs )
        return false;
    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
        if ( m_pages[ i ]->GetTitle() == title )
            return tabs->SetActiveTab( i );
    return false;
}

/*-------------------------------------------------------------------------*/

CString
CSettingsForm::GetSelectedPageTitle( void ) const
{GUCEF_TRACE;

    const CTabControl* tabs = const_cast< CSettingsForm* >( this )->GetWidgetAs< CTabControl >( "Pages" );
    if ( GUCEF_NULL == tabs )
        return CString::Empty;
    const Int32 active = tabs->GetActiveTab();
    if ( active < 0 || active >= (Int32)m_pages.size() )
        return CString::Empty;
    return m_pages[ (UInt32) active ]->GetTitle();
}

/*-------------------------------------------------------------------------*/

bool
CSettingsForm::Apply( void )
{GUCEF_TRACE;

    bool success = true;
    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
    {
        if ( m_pages[ i ]->HasChanges() && !m_pages[ i ]->ApplySettings() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SettingsForm: The settings of page \"" << m_pages[ i ]->GetTitle() << "\" could not be applied" );
            success = false;
        }
    }
    UpdateButtons();
    NotifyObservers( SettingsAppliedEvent );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CSettingsForm::Ok( void )
{GUCEF_TRACE;

    bool success = Apply();
    Close();
    return success;
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::Cancel( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
        m_pages[ i ]->ReadSettings();
    UpdateButtons();
    Close();
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::Close( void )
{GUCEF_TRACE;

    Hide();
    NotifyObservers( ClosedEvent );
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::OnPageChanged( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdateButtons();
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::OnOkActivated( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Ok();
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::OnApplyActivated( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Apply();
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::OnCancelActivated( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Cancel();
}

/*-------------------------------------------------------------------------*/

void
CSettingsForm::OnNavigation( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CNavigationEventData* navigation = static_cast< CNavigationEventData* >( eventData );
    if ( NAVIGATION_CANCEL == navigation->direction && EVENTPHASE_TUNNEL != navigation->phase && IsVisible() )
    {
        navigation->handled = true;
        Cancel();
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CSettingsForm";
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
