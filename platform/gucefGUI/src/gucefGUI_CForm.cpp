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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CForm.h"

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

const CORE::CEvent CForm::LayoutLoadedEvent = "GUCEF::GUI::CForm::LayoutLoadedEvent";
const CORE::CEvent CForm::LayoutSavedEvent = "GUCEF::GUI::CForm::LayoutSavedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CForm::RegisterEvents( void )
{GUCEF_TRACE;

    LayoutLoadedEvent.Initialize();
    LayoutSavedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CForm::CForm( void )
    : CORE::CObservingNotifier()
    , m_backend( GUCEF_NULL )
    , m_parentWidget( GUCEF_NULL )
    , m_context( GUCEF_NULL )
    , m_root( GUCEF_NULL )
    , m_description()
    , m_sources()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CForm::CForm( CIGUIContext* context )
    : CORE::CObservingNotifier()
    , m_backend( GUCEF_NULL )
    , m_parentWidget( GUCEF_NULL )
    , m_context( context )
    , m_root( GUCEF_NULL )
    , m_description()
    , m_sources()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CForm::~CForm()
{GUCEF_TRACE;

    DestroyContent();
}

/*-------------------------------------------------------------------------*/

void
CForm::DestroyContent( void )
{GUCEF_TRACE;

    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != m_root && GUCEF_NULL != context )
        context->DestroyWidget( m_root );
    m_root = GUCEF_NULL;
    m_sources.clear();
    if ( GUCEF_NULL != m_backend )
    {
        if ( GUCEF_NULL != m_context )
            m_context->DestroyFormBackend( m_backend );
        else
            delete m_backend;
        m_backend = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CForm::SetContext( CIGUIContext* context )
{GUCEF_TRACE;

    m_context = context;
}

/*-------------------------------------------------------------------------*/

CIGUIContext*
CForm::GetContext( void )
{GUCEF_TRACE;

    return m_context;
}

/*-------------------------------------------------------------------------*/

CGUIContext*
CForm::GetGUIContext( void )
{GUCEF_TRACE;

    return GUCEF_NULL != m_context ? m_context->AsGUIContext() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CForm::GetDescription( void ) const
{GUCEF_TRACE;

    return m_description;
}

/*-------------------------------------------------------------------------*/

bool
CForm::LoadLayout( const CString& layoutStoragePath )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Form: Loading layout from " << layoutStoragePath );
    CORE::CDataNode tree;
    if ( CFormLayout::LoadTree( layoutStoragePath, tree ) && GUCEF_NULL != CFormLayout::FindLayoutRoot( tree ) )
        return LoadLayout( tree );
    return LoadNativeLayout( layoutStoragePath );
}

/*-------------------------------------------------------------------------*/

bool
CForm::LoadLayout( CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;

    return LoadLayout( layoutStorage, "xml" );
}

/*-------------------------------------------------------------------------*/

bool
CForm::LoadLayout( CORE::CIOAccess& layoutStorage, const CString& codecType )
{GUCEF_TRACE;

    UInt64 start = layoutStorage.Tell();
    CORE::CDataNode tree;
    if ( CFormLayout::LoadTree( layoutStorage, codecType, tree ) && GUCEF_NULL != CFormLayout::FindLayoutRoot( tree ) )
        return LoadLayout( tree );
    layoutStorage.Setpos( start );
    return LoadNativeLayout( layoutStorage );
}

/*-------------------------------------------------------------------------*/

bool
CForm::LoadLayout( const CORE::CDataNode& layout )
{GUCEF_TRACE;

    const CORE::CDataNode* layoutRoot = CFormLayout::FindLayoutRoot( layout );
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == layoutRoot || GUCEF_NULL == context )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Form: Cannot load the layout: " << ( GUCEF_NULL == layoutRoot ? "no GUILayout node" : "no gucefGUI context" ) );
        return false;
    }

    OnPreLayoutLoad();
    DestroyContent();

    m_description = *layoutRoot;
    CFormLayout::TStringVector notes;
    CFormLayout::Migrate( m_description, notes );
    for ( UInt32 i=0; i<(UInt32)notes.size(); ++i )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Form: layout: " << notes[ i ] );

    CFormLayout::ApplyResources( *context, m_description );

    UInt32 roots = 0;
    CORE::CDataNode::iterator i = m_description.Begin();
    while ( i != m_description.End() )
    {
        CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() != "Widget" )
            continue;
        if ( ++roots > 1 )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Form: The layout has more than one top widget, only the first is used" );
            break;
        }
        m_root = CFormLayout::Build( *context, *child, m_parentWidget, m_sources );
    }
    if ( GUCEF_NULL == m_root )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Form: The layout did not give a widget" );
        return false;
    }

    OnPostLayoutLoad();
    NotifyObservers( LayoutLoadedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForm::LoadNativeLayout( const CString& layoutStoragePath )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_backend && GUCEF_NULL != m_context )
        m_backend = m_context->CreateFormBackend();
    if ( GUCEF_NULL == m_backend )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Form: " << layoutStoragePath << " is no gucefGUI layout and the driver has no loader for native layouts" );
        return false;
    }
    OnPreLayoutLoad();
    if ( !m_backend->LoadLayout( layoutStoragePath ) )
        return false;
    if ( GUCEF_NULL != m_parentWidget )
        SetParent( m_parentWidget );
    OnPostLayoutLoad();
    NotifyObservers( LayoutLoadedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForm::LoadNativeLayout( CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_backend && GUCEF_NULL != m_context )
        m_backend = m_context->CreateFormBackend();
    if ( GUCEF_NULL == m_backend )
        return false;
    OnPreLayoutLoad();
    if ( !m_backend->LoadLayout( layoutStorage ) )
        return false;
    if ( GUCEF_NULL != m_parentWidget )
        SetParent( m_parentWidget );
    OnPostLayoutLoad();
    NotifyObservers( LayoutLoadedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForm::SaveLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
        return false;

    if ( m_description.GetName() != "GUILayout" && m_description.GetAttributeValue( "format" ).AsString() != "GUILayout" )
        m_description.Clear();
    m_description.SetName( "GUILayout" );
    m_description.SetAttribute( "format", "GUILayout" );
    m_description.SetAttribute( "version", CORE::UInt32ToString( CFormLayout::CurrentVersion ) );

    CORE::CDataNode* rootNode = GUCEF_NULL;
    CFormLayout::TWidgetSourceMap::iterator s = m_sources.find( m_root );
    if ( s != m_sources.end() && GUCEF_NULL != (*s).second.node && (*s).second.node->GetParent() == &m_description )
        rootNode = (*s).second.node;
    if ( GUCEF_NULL == rootNode )
        CFormLayout::WriteWidget( *m_root, m_description, m_sources );
    else
        CFormLayout::UpdateNode( *m_root, *rootNode, m_sources );

    layout = m_description;
    NotifyObservers( LayoutSavedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForm::SaveLayout( const CString& layoutStoragePath )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
    {
        if ( GUCEF_NULL != m_backend && m_backend->SaveLayout( layoutStoragePath ) )
        {
            NotifyObservers( LayoutSavedEvent );
            return true;
        }
        return false;
    }
    CORE::CDataNode layout;
    return SaveLayout( layout ) && CFormLayout::SaveTree( layout, layoutStoragePath );
}

/*-------------------------------------------------------------------------*/

bool
CForm::SaveLayout( CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;

    return SaveLayout( layoutStorage, "xml" );
}

/*-------------------------------------------------------------------------*/

bool
CForm::SaveLayout( CORE::CIOAccess& layoutStorage, const CString& codecType )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
    {
        if ( GUCEF_NULL != m_backend && m_backend->SaveLayout( layoutStorage ) )
        {
            NotifyObservers( LayoutSavedEvent );
            return true;
        }
        return false;
    }
    CORE::CDataNode layout;
    return SaveLayout( layout ) && CFormLayout::SaveTree( layout, layoutStorage, codecType );
}

/*-------------------------------------------------------------------------*/

const CWidget*
CForm::GetRootWidget( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_root )
        return m_root;
    if ( GUCEF_NULL != m_backend )
        return m_backend->GetRootWidget();
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWidget*
CForm::GetRootWidget( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_root )
        return m_root;
    if ( GUCEF_NULL != m_backend )
        return m_backend->GetRootWidget();
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWidget*
CForm::GetWidget( const CString& widgetName )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_root )
    {
        if ( m_root->GetName() == widgetName )
            return m_root;
        return m_root->FindWidget( widgetName );
    }
    if ( GUCEF_NULL != m_backend )
        return m_backend->GetWidget( widgetName );
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CForm::IsVisible( void ) const
{GUCEF_TRACE;

    const CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root )
        return false;
    if ( GUCEF_NULL == root->GetParentWidget() && GUCEF_NULL != root->GetContext() )
        return root->GetContext()->IsRootShown( root ) && root->IsVisible();
    return root->IsVisible();
}

/*-------------------------------------------------------------------------*/

bool
CForm::SetVisibility( const bool isVisible )
{GUCEF_TRACE;

    return isVisible ? Show() : Hide();
}

/*-------------------------------------------------------------------------*/

bool
CForm::Show( void )
{GUCEF_TRACE;

    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root )
        return false;
    root->SetVisibility( true );
    if ( GUCEF_NULL == root->GetParentWidget() && GUCEF_NULL != root->GetContext() )
        return root->GetContext()->ShowRoot( root, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForm::ShowModal( void )
{GUCEF_TRACE;

    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root )
        return false;
    return root->ShowModal();
}

/*-------------------------------------------------------------------------*/

bool
CForm::Hide( void )
{GUCEF_TRACE;

    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root )
        return false;
    if ( GUCEF_NULL == root->GetParentWidget() && GUCEF_NULL != root->GetContext() )
        return root->GetContext()->HideRoot( root );
    return root->SetVisibility( false );
}

/*-------------------------------------------------------------------------*/

bool
CForm::SetParent( CForm* parentForm )
{GUCEF_TRACE;

    if ( GUCEF_NULL == parentForm )
        return SetParent( (CWidget*) GUCEF_NULL );
    return SetParent( parentForm->GetRootWidget() );
}

/*-------------------------------------------------------------------------*/

bool
CForm::SetParent( CWidget* parentWidget )
{GUCEF_TRACE;

    m_parentWidget = parentWidget;
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root )
        return true;
    return root->SetParentWidget( parentWidget );
}

/*-------------------------------------------------------------------------*/

CWidget*
CForm::GetParent( void )
{GUCEF_TRACE;

    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
        return root->GetParentWidget();
    return m_parentWidget;
}

/*-------------------------------------------------------------------------*/

void
CForm::OnPreLayoutLoad( void )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

void
CForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

const CString&
CForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::GUI::CForm";
    return typeName;
}

/*-------------------------------------------------------------------------*/

const CString&
CForm::GetType( void ) const
{GUCEF_TRACE;

    static const CString typeName = "CForm";
    return typeName;
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
