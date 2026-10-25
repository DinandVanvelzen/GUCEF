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

#include <algorithm>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#include "gucefGUI_CFontRasterizerRegistry.h"
#define GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#endif /* GUCEF_GUI_CFONTRASTERIZERREGISTRY_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#ifndef GUCEF_GUI_CGUIDRIVER_H
#include "gucefGUI_CGUIDriver.h"
#define GUCEF_GUI_CGUIDRIVER_H
#endif /* GUCEF_GUI_CGUIDRIVER_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CGROUPBOX_H
#include "gucefGUI_CGroupBox.h"
#define GUCEF_GUI_CGROUPBOX_H
#endif /* GUCEF_GUI_CGROUPBOX_H ? */

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#include "gucefGUI_CScrollPanel.h"
#define GUCEF_GUI_CSCROLLPANEL_H
#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */

#include "gucefGUI_CGUIContext.h"

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

const CORE::CEvent CGUIContext::FrameStartedEvent = "GUCEF::GUI::CGUIContext::FrameStartedEvent";
const CORE::CEvent CGUIContext::FrameRenderedEvent = "GUCEF::GUI::CGUIContext::FrameRenderedEvent";
const CORE::CEvent CGUIContext::FormShownEvent = "GUCEF::GUI::CGUIContext::FormShownEvent";
const CORE::CEvent CGUIContext::FormHiddenEvent = "GUCEF::GUI::CGUIContext::FormHiddenEvent";

/** Two clicks within this time and distance make a double click */
static const UInt32 DOUBLECLICK_TIME_MS = 400;

/** A widget as the user action log names it: where it is in the tree and what it is */
static CString
DescribeWidget( const CWidget* widget )
{
    if ( GUCEF_NULL == widget )
        return "nothing";
    return widget->GetNamePath() + " (" + widget->GetClassTypeName() + ")";
}
static const Int32 DOUBLECLICK_DISTANCE = 4;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt32
StateFromName( const CString& name )
{
    CString lower = name.Trim( true ).Trim( false ).Lowercase();
    if ( lower == "hover" || lower == "hovered" ) return CWidget::STATE_HOVERED;
    if ( lower == "pressed" ) return CWidget::STATE_PRESSED;
    if ( lower == "focused" || lower == "focus" ) return CWidget::STATE_FOCUSED;
    if ( lower == "disabled" ) return CWidget::STATE_DISABLED;
    if ( lower == "checked" ) return CWidget::STATE_CHECKED;
    return 0;
}

bool
HasStyleClass( const CWidget& widget, const CString& styleClass )
{
    // only what is set on the widget itself counts; the classes are separated by spaces, empty ones do not count
    CString classes;
    if ( !widget.GetExplicitProperty( EWidgetProperty_StyleClass, classes ) || classes.IsNULLOrEmpty() )
        return false;
    const UInt32 length = classes.Length();
    const UInt32 wanted = styleClass.Length();
    UInt32 start = 0;
    while ( start <= length )
    {
        UInt32 end = start;
        while ( end < length && ' ' != classes[ end ] )
            ++end;
        if ( end - start == wanted && 0 == strncmp( classes.C_String() + start, styleClass.C_String(), wanted ) )
            return true;
        start = end + 1;
    }
    return false;
}

Int32
Depth( const CWidget* widget )
{
    Int32 depth = 0;
    for ( const CWidget* p=widget->GetParentWidget(); GUCEF_NULL != p; p=p->GetParentWidget() )
        ++depth;
    return depth;
}

bool
IsDescendantOf( const CWidget* widget, const CWidget* ancestor )
{
    for ( const CWidget* p=widget; GUCEF_NULL != p; p=p->GetParentWidget() )
        if ( p == ancestor )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUIContext::SImageEntry::SImageEntry( void )
    : pixels()
    , revision( 0 )
    , density( 1 )
{
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::RegisterEvents( void )
{GUCEF_TRACE;

    FrameStartedEvent.Initialize();
    FrameRenderedEvent.Initialize();
    FormShownEvent.Initialize();
    FormHiddenEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CGUIContext::CGUIContext( CGUIDriver* driver, TWindowContextPtr windowContext )
    : CIGUIContext()
    , CORE::CObservingNotifier()
    , m_driver( driver )
    , m_windowContext( windowContext )
    , m_widgets()
    , m_forms()
    , m_roots()
    , m_modalRoots()
    , m_dirtyWidgets()
    , m_updateWidgets()
    , m_images()
    , m_fonts()
    , m_handlerCache()
    , m_theme()
    , m_themeStyles()
    , m_themeStateStyles()
    , m_themeIndex()
    , m_themeCustomIndex()
    , m_logicalWidth( 0 )
    , m_logicalHeight( 0 )
    , m_renderScale( 1 )
    , m_layer( 0 )
    , m_presentMode( PRESENTMODE_NEAREST )
    , m_presentFilter()
    , m_presentFilterParameters()
    , m_cursorImage()
    , m_cursorHotspotX( 0 )
    , m_cursorHotspotY( 0 )
    , m_cursorVisible( true )
    , m_pointerX( 0 )
    , m_pointerY( 0 )
    , m_buttonStates( 0 )
    , m_modifiers( 0 )
    , m_hovered( GUCEF_NULL )
    , m_pressed( GUCEF_NULL )
    , m_pressedButton( MOUSEBUTTON_NONE )
    , m_pressedMoveCount( 0 )
    , m_focus( GUCEF_NULL )
    , m_popup( GUCEF_NULL )
    , m_lastClickTarget( GUCEF_NULL )
    , m_lastClickTime( 0 )
    , m_lastClickX( 0 )
    , m_lastClickY( 0 )
    , m_clickCount( 0 )
    , m_timeInMs( 0 )
    , m_hoverStartTime( 0 )
    , m_lastRealTick( 0 )
    , m_uniqueCounter( 0 )
    , m_layoutDirty( true )
    , m_rootOrderDirty( false )
    , m_inFlush( false )
    , m_dispatchDepth( 0 )
    , m_pendingDestroy()
{GUCEF_TRACE;

    RegisterEvents();
    if ( !m_windowContext.IsNULL() )
    {
        TEventCallback redrawCallback( this, &CGUIContext::OnWindowRedraw );
        SubscribeTo( m_windowContext.GetPointerAlways(), CWindowContext::WindowContextRedrawEvent, redrawCallback );
        TEventCallback sizeCallback( this, &CGUIContext::OnWindowSize );
        SubscribeTo( m_windowContext.GetPointerAlways(), CWindowContext::WindowContextSizeEvent, sizeCallback );
        TEventCallback activateCallback( this, &CGUIContext::OnWindowActivate );
        SubscribeTo( m_windowContext.GetPointerAlways(), CWindowContext::WindowContextActivateEvent, activateCallback );
        m_windowContext->RegisterGuiLayer( this );
    }
}

/*-------------------------------------------------------------------------*/

CGUIContext::~CGUIContext()
{GUCEF_TRACE;

    if ( !m_windowContext.IsNULL() )
        m_windowContext->UnregisterGuiLayer( this );
    UnsubscribeAllFromNotifier();
    DestroyAllWidgets();
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::DestroyAllWidgets( void )
{GUCEF_TRACE;

    // forms first (they destroy their root widgets), then whatever is left, parents first
    TFormSet forms = m_forms;
    for ( TFormSet::iterator i=forms.begin(); i!=forms.end(); ++i )
        delete (*i);
    m_forms.clear();

    m_dispatchDepth = 0;
    ProcessPendingDestroys();
    while ( !m_widgets.empty() )
    {
        CWidget* widget = *m_widgets.begin();
        while ( GUCEF_NULL != widget->GetParentWidget() )
            widget = widget->GetParentWidget();
        DestroyWidget( widget );
    }
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::CreateWidget( const CString& widgetType, const CString& widgetName, CWidget* parent )
{GUCEF_TRACE;

    CWidgetTypeRegistry& registry = CGuiGlobal::Instance()->GetWidgetTypeRegistry();
    CWidget* widget = registry.CreateWidget( widgetType );
    if ( GUCEF_NULL == widget )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GUIContext: Cannot create a widget of unknown type \"" << widgetType << "\"" );
        return GUCEF_NULL;
    }
    if ( GUCEF_NULL != parent && parent->m_context != this )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GUIContext: Cannot create widget \"" << widgetName << "\" below a widget of another context" );
        delete widget;
        return GUCEF_NULL;
    }

    widget->m_widgetTypeName = widgetType;
    widget->m_resolvedType = GUCEF_NULL;
    widget->m_name = widgetName;
    widget->m_context = this;
    widget->m_handler = GetWidgetHandler( widgetType );
    if ( GUCEF_NULL == widget->m_handler )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GUIContext: The driver has no handler for widget type \"" << widgetType << "\", widget \"" << widgetName << "\" will not be shown" );
    }
    m_widgets.insert( widget );

    if ( GUCEF_NULL != parent )
    {
        parent->m_children.push_back( widget );
        widget->m_parent = parent;
    }

    if ( GUCEF_NULL != widget->m_handler )
        widget->m_handler->CreateNative( *widget );

    // the native object starts at the values of every property
    widget->MarkAllPropertiesDirty();
    widget->m_stateDirty = true;
    widget->m_rectDirty = true;
    m_dirtyWidgets.insert( widget );
    m_layoutDirty = true;

    if ( GUCEF_NULL != parent )
        parent->OnChildrenChanged();
    return widget;
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::CreateWidget( const CString& widgetType )
{GUCEF_TRACE;

    return CreateWidget( widgetType, CString::Empty, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::DestroyWidget( CWidget* widget )
{GUCEF_TRACE;

    if ( GUCEF_NULL == widget || m_widgets.find( widget ) == m_widgets.end() )
        return;

    if ( m_dispatchDepth > 0 )
    {
        // an observer is still running for an event of this or another widget: take it out of
        // the tree now, delete it once the event is done
        for ( UInt32 i=0; i<(UInt32)m_pendingDestroy.size(); ++i )
            if ( m_pendingDestroy[ i ] == widget )
                return;
        ForgetInteraction( widget, true );
        if ( GUCEF_NULL == widget->m_parent )
            HideRoot( widget );
        else
        {
            widget->DetachFromParent();
            m_layoutDirty = true;
        }
        m_pendingDestroy.push_back( widget );
        return;
    }

    TWidgetVector children = widget->m_children;
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        DestroyWidget( children[ i ] );

    ForgetInteraction( widget, true );
    if ( GUCEF_NULL == widget->m_parent )
        HideRoot( widget );
    if ( GUCEF_NULL != widget->m_handler )
        widget->m_handler->DestroyNative( *widget );
    delete widget;
    m_layoutDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::ProcessPendingDestroys( void )
{GUCEF_TRACE;

    if ( m_dispatchDepth > 0 )
        return;
    while ( !m_pendingDestroy.empty() )
    {
        TWidgetVector pending = m_pendingDestroy;
        m_pendingDestroy.clear();
        for ( UInt32 i=0; i<(UInt32)pending.size(); ++i )
            DestroyWidget( pending[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::ForgetInteraction( CWidget* widget, bool widgetIsIntact )
{GUCEF_TRACE;

    // the widget or one below it is going away: no hover, press, focus or popup may point into it
    if ( GUCEF_NULL != m_hovered && IsDescendantOf( m_hovered, widget ) )
        SetHovered( GUCEF_NULL );
    if ( GUCEF_NULL != m_pressed && IsDescendantOf( m_pressed, widget ) )
    {
        if ( widgetIsIntact )
        {
            CancelPointerCapture( "the widget is going away" );
        }
        else
        {
            m_pressed->SetStateFlag( CWidget::STATE_PRESSED, false );
            m_pressed = GUCEF_NULL;
        }
    }
    if ( GUCEF_NULL != m_focus && IsDescendantOf( m_focus, widget ) )
        SetFocus( GUCEF_NULL );
    if ( GUCEF_NULL != m_popup && IsDescendantOf( m_popup, widget ) )
        m_popup = GUCEF_NULL;
    if ( GUCEF_NULL != m_lastClickTarget && IsDescendantOf( m_lastClickTarget, widget ) )
        m_lastClickTarget = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CForm*
CGUIContext::CreateForm( const CString& formType )
{GUCEF_TRACE;

    CForm* form = GUCEF_NULL;
    if ( formType == "FormEx" )
        form = GUCEF_NEW CFormEx( this );
    else
    if ( formType == "Form" || formType.IsNULLOrEmpty() )
        form = GUCEF_NEW CForm( this );
    else
        return GUCEF_NULL;

    m_forms.insert( form );
    return form;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::DestroyForm( CForm* form )
{GUCEF_TRACE;

    if ( GUCEF_NULL == form )
        return;
    m_forms.erase( form );
    delete form;
}

/*-------------------------------------------------------------------------*/

CGUIContext::TStringSet
CGUIContext::GetAvailableFormTypes( void )
{GUCEF_TRACE;

    TStringSet types;
    types.insert( "Form" );
    types.insert( "FormEx" );
    return types;
}

/*-------------------------------------------------------------------------*/

CGUIContext::TStringSet
CGUIContext::GetAvailableWidgetTypes( void )
{GUCEF_TRACE;

    TStringSet available;
    CWidgetTypeRegistry::TStringSet types = CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetTypeNames();
    for ( CWidgetTypeRegistry::TStringSet::iterator i=types.begin(); i!=types.end(); ++i )
        if ( GUCEF_NULL != GetWidgetHandler( *i ) )
            available.insert( *i );
    return available;
}

/*-------------------------------------------------------------------------*/

CFormBackend*
CGUIContext::CreateFormBackend( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::DestroyFormBackend( CFormBackend* formBackend )
{GUCEF_TRACE;

    delete formBackend;
}

/*-------------------------------------------------------------------------*/

CGUIDriver*
CGUIContext::GetDriver( void )
{GUCEF_TRACE;

    return m_driver;
}

/*-------------------------------------------------------------------------*/

CGUIContext::TWidgetSet
CGUIContext::GetOwnedWidgets( void )
{GUCEF_TRACE;

    return m_widgets;
}

/*-------------------------------------------------------------------------*/

CGUIContext::TFormSet
CGUIContext::GetOwnedForms( void )
{GUCEF_TRACE;

    return m_forms;
}

/*-------------------------------------------------------------------------*/

TWindowContextPtr
CGUIContext::GetWindowContext( void ) const
{GUCEF_TRACE;

    return m_windowContext;
}

/*-------------------------------------------------------------------------*/

const CGUIContext::TWidgetVector&
CGUIContext::GetRootWidgets( void ) const
{GUCEF_TRACE;

    return m_roots;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::ShowRoot( CWidget* root, bool modal )
{GUCEF_TRACE;

    if ( GUCEF_NULL == root || root->m_context != this || GUCEF_NULL != root->m_parent )
        return false;

    for ( TWidgetVector::iterator i=m_roots.begin(); i!=m_roots.end(); ++i )
    {
        if ( (*i) == root )
        {
            m_roots.erase( i );
            break;
        }
    }
    m_roots.push_back( root );
    if ( modal )
        m_modalRoots.insert( root );
    else
        m_modalRoots.erase( root );

    // a modal root takes the keyboard focus away from everything outside of it
    if ( modal && GUCEF_NULL != m_focus && !IsDescendantOf( m_focus, root ) )
        SetFocus( GUCEF_NULL );

    m_rootOrderDirty = true;
    m_layoutDirty = true;
    CORE::TCloneableString eventData( root->GetName() );
    NotifyObservers( FormShownEvent, &eventData );
    UpdatePointerTarget();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::HideRoot( CWidget* root )
{GUCEF_TRACE;

    for ( TWidgetVector::iterator i=m_roots.begin(); i!=m_roots.end(); ++i )
    {
        if ( (*i) == root )
        {
            m_roots.erase( i );
            m_modalRoots.erase( root );
            if ( m_popup == root )
                m_popup = GUCEF_NULL;
            ForgetInteraction( root, true );
            m_rootOrderDirty = true;
            CORE::TCloneableString eventData( root->GetName() );
            NotifyObservers( FormHiddenEvent, &eventData );
            UpdatePointerTarget();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::IsModalRoot( const CWidget* root ) const
{GUCEF_TRACE;

    return m_modalRoots.find( const_cast< CWidget* >( root ) ) != m_modalRoots.end();
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::IsRootShown( const CWidget* root ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_roots.size(); ++i )
        if ( m_roots[ i ] == root )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::GetModalRoot( void ) const
{GUCEF_TRACE;

    for ( Int32 i=(Int32)m_roots.size()-1; i>=0; --i )
        if ( m_modalRoots.find( m_roots[ i ] ) != m_modalRoots.end() && m_roots[ i ]->IsVisible() )
            return m_roots[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::ShowPopup( CWidget* popup, Int32 x, Int32 y )
{GUCEF_TRACE;

    if ( GUCEF_NULL == popup || GUCEF_NULL != popup->m_parent )
        return false;
    if ( GUCEF_NULL != m_popup && m_popup != popup )
        ClosePopup();

    popup->SetPosition( CUVector( CUDim( 0.0f, x ), CUDim( 0.0f, y ) ) );
    popup->SetVisibility( true );
    if ( !ShowRoot( popup, false ) )
        return false;
    m_popup = popup;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::ClosePopup( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_popup )
        return false;
    CWidget* popup = m_popup;
    m_popup = GUCEF_NULL;
    return HideRoot( popup );
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::GetPopup( void ) const
{GUCEF_TRACE;

    return m_popup;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::RegisterImage( const CString& alias, const IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    return RegisterImage( alias, pixels, 1 );
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::RegisterImage( const CString& alias, const IMAGE::TPixelMapPtr& pixels, UInt32 density )
{GUCEF_TRACE;

    if ( alias.IsNULLOrEmpty() || pixels.IsNULL() || 0 == density )
        return false;

    IMAGE::TPixelMapPtr shared( pixels );
    if ( IMAGE::PSF_RGBA != shared->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != shared->GetPixelComponentDataType() )
    {
        // converted in place: the pixels are shared with the caller
        if ( !shared->ConvertFormatTo( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GUIContext: Image \"" << alias << "\" cannot be converted to RGBA" );
            return false;
        }
    }

    TImageEntry& entry = m_images[ alias ];
    entry.pixels = pixels;
    entry.density = density;
    ++entry.revision;
    m_layoutDirty = true;
    OnImageChanged( alias );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CGUIContext::GetImageDensity( const CString& alias ) const
{GUCEF_TRACE;

    TImageMap::const_iterator i = m_images.find( alias );
    return i == m_images.end() ? 1 : (*i).second.density;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::GetImageLogicalSize( const CString& reference, Int32& width, Int32& height )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr pixels;
    UInt32 revision = 0;
    if ( !ResolveImage( reference, pixels, revision ) || pixels.IsNULL() )
        return false;
    UInt32 density = GetImageDensity( reference );
    width = (Int32) ( pixels->GetWidthInPixels() / density );
    height = (Int32) ( pixels->GetHeightInPixels() / density );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::UpdateImage( const CString& alias )
{GUCEF_TRACE;

    TImageMap::iterator i = m_images.find( alias );
    if ( i == m_images.end() )
        return false;
    ++(*i).second.revision;
    OnImageChanged( alias );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::UnregisterImage( const CString& alias )
{GUCEF_TRACE;

    if ( 0 == m_images.erase( alias ) )
        return false;
    OnImageChanged( alias );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::HasImage( const CString& alias ) const
{GUCEF_TRACE;

    return m_images.find( alias ) != m_images.end();
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::ResolveImage( const CString& reference, IMAGE::TPixelMapPtr& pixels, UInt32& revision )
{GUCEF_TRACE;

    if ( reference.IsNULLOrEmpty() )
        return false;

    TImageMap::iterator i = m_images.find( reference );
    if ( i != m_images.end() )
    {
        pixels = (*i).second.pixels;
        revision = (*i).second.revision;
        return true;
    }

    // not registered: a file in the VFS
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::TBasicVfsResourcePtr file = vfs.GetFile( reference );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
        return false;

    IMAGE::CImage image;
    CString imageType = CORE::ExtractFileExtention( reference ).Lowercase();
    if ( !image.Load( *file->GetAccess(), imageType ) || 0 == image.GetFrameCount() || image.GetFrame( 0 ).empty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GUIContext: Cannot load image \"" << reference << "\"" );
        return false;
    }

    if ( !RegisterImage( reference, image.GetFrame( 0 )[ 0 ] ) )
        return false;
    TImageEntry& entry = m_images[ reference ];
    pixels = entry.pixels;
    revision = entry.revision;
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CGUIContext::MakeUniqueImageAlias( const CString& prefix )
{GUCEF_TRACE;

    CString alias;
    do
    {
        alias = prefix + '#' + CORE::UInt32ToString( ++m_uniqueCounter );
    }
    while ( HasImage( alias ) );
    return alias;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::RegisterFont( const CString& name, const TBitmapFontPtr& font )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() || font.IsNULL() )
        return false;
    m_fonts[ name ] = font;
    m_layoutDirty = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::RegisterFont( const CString& name, const CString& fontPath, UInt32 pixelSize )
{GUCEF_TRACE;

    TBitmapFontPtr font = CGuiGlobal::Instance()->GetFontRasterizerRegistry().RasterizeFont( fontPath, pixelSize, CFontRasterizer::GetLatin1Ranges() );
    return RegisterFont( name, font );
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::UnregisterFont( const CString& name )
{GUCEF_TRACE;

    return m_fonts.erase( name ) > 0;
}

/*-------------------------------------------------------------------------*/

TBitmapFontPtr
CGUIContext::GetBitmapFont( const CString& name ) const
{GUCEF_TRACE;

    TFontMap::const_iterator i = m_fonts.find( name );
    if ( i != m_fonts.end() )
        return (*i).second;
    return TBitmapFontPtr();
}

/*-------------------------------------------------------------------------*/

Int32
CGUIContext::GetLineHeight( const CString& fontName )
{GUCEF_TRACE;

    TBitmapFontPtr font = GetBitmapFont( fontName );
    if ( !font.IsNULL() && font->GetLineHeight() > 0 )
        return font->GetLineHeight();
    Int32 width = 0, height = 0;
    if ( MeasureText( fontName, "Mg", 0, width, height ) && height > 0 )
        return height;
    return 16;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::MeasureText( const CString& fontName, const CString& text, Int32 maxWidth, Int32& width, Int32& height )
{GUCEF_TRACE;

    TBitmapFontPtr font = GetBitmapFont( fontName );
    if ( font.IsNULL() )
    {
        width = 0;
        height = 0;
        return false;
    }
    font->MeasureText( text, maxWidth, width, height );
    return true;
}

/*-------------------------------------------------------------------------*/

CGUIContext::SThemeStyle::SThemeStyle( void )
    : type()
    , styleClass()
    , name()
    , hasState( false )
    , stateFlags()
    , properties()
{
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::CompileTheme( void )
{GUCEF_TRACE;

    m_themeStyles.clear();
    m_themeStateStyles.clear();
    m_themeCustomIndex.clear();
    for ( UInt32 i=0; i<(UInt32)EWidgetProperty_Count; ++i )
        m_themeIndex[ i ].clear();

    for ( CORE::CDataNode::const_iterator s=m_theme.ConstBegin(); s!=m_theme.ConstEnd(); ++s )
    {
        const CORE::CDataNode* styleNode = (*s);
        if ( styleNode->GetName() != "Style" )
            continue;

        SThemeStyle style;
        style.type = styleNode->GetAttributeValue( "type" ).AsString();
        style.styleClass = styleNode->GetAttributeValue( "class" ).AsString();
        style.name = styleNode->GetAttributeValue( "name" ).AsString();
        CString states = styleNode->GetAttributeValue( "state" ).AsString();
        style.hasState = !states.IsNULLOrEmpty();
        if ( style.hasState )
        {
            CString::StringVector parts = states.ParseElements( ',', false );
            for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
                style.stateFlags.push_back( StateFromName( parts[ i ] ) );
        }
        for ( CORE::CDataNode::const_iterator p=styleNode->ConstBegin(); p!=styleNode->ConstEnd(); ++p )
        {
            const CORE::CDataNode* propertyNode = (*p);
            if ( propertyNode->GetName() != "Property" )
                continue;
            SThemeProperty property;
            property.name = propertyNode->GetAttributeValue( "name" ).AsString();
            property.property = StringToWidgetProperty( property.name );
            property.value.SetText( propertyNode->GetAttributeValue( "value" ).AsString() );
            property.value.ReadAll();
            style.properties.push_back( property );
        }

        // a style is in the index of a property once, with the last mention of the property in the style
        const UInt32 styleIndex = (UInt32) m_themeStyles.size();
        for ( UInt32 p=0; p<(UInt32)style.properties.size(); ++p )
        {
            TThemeIndexVector& index = EWidgetProperty_Unknown != style.properties[ p ].property ?
                                       m_themeIndex[ style.properties[ p ].property ] : m_themeCustomIndex[ style.properties[ p ].name ];
            if ( !index.empty() && index.back().style == styleIndex )
                index.back().property = p;
            else
            {
                SThemeIndexEntry entry;
                entry.style = styleIndex;
                entry.property = p;
                index.push_back( entry );
            }
        }
        if ( style.hasState )
            m_themeStateStyles.push_back( styleIndex );
        m_themeStyles.push_back( style );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::SetTheme( const CORE::CDataNode& theme )
{GUCEF_TRACE;

    m_theme = theme;
    m_layoutDirty = true;

    // the styles by the properties they set: the values of the widgets are asked for all the time (also by the hit test of every pointer move)
    CompileTheme();

    // every property of every widget may now have another value
    for ( TWidgetSet::iterator i=m_widgets.begin(); i!=m_widgets.end(); ++i )
    {
        CWidget* widget = *i;
        widget->MarkAllPropertiesDirty();
        m_dirtyWidgets.insert( widget );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CGUIContext::GetTheme( void ) const
{GUCEF_TRACE;

    return m_theme;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::StyleMatches( const SThemeStyle& style, const CWidget& widget, bool ignoreState, Int32& score, UInt32& widgetState, bool& widgetStateKnown ) const
{GUCEF_TRACE;

    score = 0;
    if ( !style.type.IsNULLOrEmpty() )
    {
        const CResolvedWidgetType* widgetType = widget.GetResolvedType();
        if ( GUCEF_NULL == widgetType || !widgetType->IsA( style.type ) )
            return false;
        score += 1;
    }
    if ( !style.styleClass.IsNULLOrEmpty() )
    {
        if ( !HasStyleClass( widget, style.styleClass ) )
            return false;
        score += 100;
    }
    if ( !style.name.IsNULLOrEmpty() )
    {
        if ( widget.GetName() != style.name )
            return false;
        score += 10000;
    }
    if ( style.hasState )
    {
        if ( ignoreState )
            return true;
        if ( !widgetStateKnown )
        {
            widgetState = widget.GetState();
            widgetStateKnown = true;
        }
        for ( UInt32 i=0; i<(UInt32)style.stateFlags.size(); ++i )
        {
            UInt32 flag = style.stateFlags[ i ];
            if ( 0 == flag || 0 == ( widgetState & flag ) )
                return false;
            score += 10;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CWidgetPropertyValue*
CGUIContext::FindThemeValue( const CWidget& widget, const TThemeIndexVector& index ) const
{GUCEF_TRACE;

    // only the styles that set the property matter; of the styles that fit the widget equally well the last one wins
    const CWidgetPropertyValue* found = GUCEF_NULL;
    Int32 bestScore = -1;
    UInt32 widgetState = 0;
    bool widgetStateKnown = false;
    for ( UInt32 i=0; i<(UInt32)index.size(); ++i )
    {
        const SThemeStyle& style = m_themeStyles[ index[ i ].style ];
        Int32 score = 0;
        if ( !StyleMatches( style, widget, false, score, widgetState, widgetStateKnown ) || score < bestScore )
            continue;
        found = &style.properties[ index[ i ].property ].value;
        bestScore = score;
    }
    return found;
}

/*-------------------------------------------------------------------------*/

const CWidgetPropertyValue*
CGUIContext::FindThemeValue( const CWidget& widget, EWidgetProperty property ) const
{GUCEF_TRACE;

    if ( (UInt32) property >= (UInt32) EWidgetProperty_Count || m_themeIndex[ property ].empty() )
        return GUCEF_NULL;
    return FindThemeValue( widget, m_themeIndex[ property ] );
}

/*-------------------------------------------------------------------------*/

const CWidgetPropertyValue*
CGUIContext::FindThemeValue( const CWidget& widget, const CString& name ) const
{GUCEF_TRACE;

    EWidgetProperty property = StringToWidgetProperty( name );
    if ( EWidgetProperty_Unknown != property )
        return FindThemeValue( widget, property );
    TThemeCustomIndex::const_iterator indexed = m_themeCustomIndex.find( name );
    if ( indexed == m_themeCustomIndex.end() )
        return GUCEF_NULL;
    return FindThemeValue( widget, (*indexed).second );
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::GetThemeProperty( const CWidget& widget, EWidgetProperty property, CString& value ) const
{GUCEF_TRACE;

    const CWidgetPropertyValue* found = FindThemeValue( widget, property );
    if ( GUCEF_NULL == found )
        return false;
    value = found->GetText();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::GetThemeProperty( const CWidget& widget, const CString& name, CString& value ) const
{GUCEF_TRACE;

    const CWidgetPropertyValue* found = FindThemeValue( widget, name );
    if ( GUCEF_NULL == found )
        return false;
    value = found->GetText();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::MarkThemeDependentDirty( CWidget& widget )
{GUCEF_TRACE;

    // the styles that select on a state: what they set may now be another value
    for ( UInt32 i=0; i<(UInt32)m_themeStateStyles.size(); ++i )
    {
        const SThemeStyle& style = m_themeStyles[ m_themeStateStyles[ i ] ];
        Int32 score = 0;
        UInt32 widgetState = 0;
        bool widgetStateKnown = false;
        if ( !StyleMatches( style, widget, true, score, widgetState, widgetStateKnown ) )
            continue;
        for ( UInt32 p=0; p<(UInt32)style.properties.size(); ++p )
        {
            if ( EWidgetProperty_Unknown != style.properties[ p ].property )
                widget.m_dirtyMask.Set( style.properties[ p ].property );
            else
                widget.m_dirtyCustomProperties.insert( style.properties[ p ].name );
        }
        m_dirtyWidgets.insert( &widget );
    }
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::SetLogicalResolution( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    m_logicalWidth = width;
    m_logicalHeight = height;
    m_layoutDirty = true;
}

/*-------------------------------------------------------------------------*/

UInt32
CGUIContext::GetLogicalWidth( void ) const
{GUCEF_TRACE;

    if ( 0 != m_logicalWidth || m_windowContext.IsNULL() )
        return m_logicalWidth;
    return m_windowContext->GetWidth();
}

/*-------------------------------------------------------------------------*/

UInt32
CGUIContext::GetLogicalHeight( void ) const
{GUCEF_TRACE;

    if ( 0 != m_logicalHeight || m_windowContext.IsNULL() )
        return m_logicalHeight;
    return m_windowContext->GetHeight();
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::SetPresentMode( TPresentMode mode )
{GUCEF_TRACE;

    m_presentMode = mode;
}

/*-------------------------------------------------------------------------*/

TPresentMode
CGUIContext::GetPresentMode( void ) const
{GUCEF_TRACE;

    return m_presentMode;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::SetRenderScale( UInt32 scale )
{GUCEF_TRACE;

    m_renderScale = scale > 0 ? scale : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CGUIContext::GetRenderScale( void ) const
{GUCEF_TRACE;

    return m_renderScale;
}

/*-------------------------------------------------------------------------*/

UInt32
CGUIContext::GetRenderWidth( void ) const
{GUCEF_TRACE;

    return GetLogicalWidth() * m_renderScale;
}

/*-------------------------------------------------------------------------*/

UInt32
CGUIContext::GetRenderHeight( void ) const
{GUCEF_TRACE;

    return GetLogicalHeight() * m_renderScale;
}

/*-------------------------------------------------------------------------*/

CGUIContext::TPresentFilterNames
CGUIContext::GetAvailablePresentFilters( void ) const
{GUCEF_TRACE;

    return TPresentFilterNames();
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::SetPresentFilter( const CString& filterName, const TPresentFilterParameters& parameters )
{GUCEF_TRACE;

    if ( !filterName.IsNULLOrEmpty() )
    {
        TPresentFilterNames available = GetAvailablePresentFilters();
        bool known = false;
        for ( UInt32 i=0; i<(UInt32)available.size() && !known; ++i )
            known = available[ i ] == filterName;
        if ( !known )
            return false;
    }
    m_presentFilter = filterName;
    m_presentFilterParameters = parameters;
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CGUIContext::GetPresentFilter( void ) const
{GUCEF_TRACE;

    return m_presentFilter;
}

/*-------------------------------------------------------------------------*/

const CGUIContext::TPresentFilterParameters&
CGUIContext::GetPresentFilterParameters( void ) const
{GUCEF_TRACE;

    return m_presentFilterParameters;
}

/*-------------------------------------------------------------------------*/

CRect
CGUIContext::GetPresentRect( void ) const
{GUCEF_TRACE;

    if ( m_windowContext.IsNULL() )
        return CRect( 0, 0, (Int32) GetLogicalWidth(), (Int32) GetLogicalHeight() );
    return CalculatePresentRect( GetLogicalWidth(), GetLogicalHeight(), m_windowContext->GetWidth(), m_windowContext->GetHeight(), m_presentMode );
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::WindowToLogical( Int32 windowX, Int32 windowY, Int32& x, Int32& y ) const
{GUCEF_TRACE;

    CRect present = GetPresentRect();
    if ( present.IsEmpty() )
    {
        x = windowX;
        y = windowY;
        return false;
    }
    x = (Int32) ( ( (Int64) ( windowX - present.left ) * (Int64) GetLogicalWidth() ) / present.width );
    y = (Int32) ( ( (Int64) ( windowY - present.top ) * (Int64) GetLogicalHeight() ) / present.height );
    return present.Contains( windowX, windowY );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::SetCursor( const CString& imageAlias, Int32 hotspotX, Int32 hotspotY )
{GUCEF_TRACE;

    m_cursorImage = imageAlias;
    m_cursorHotspotX = hotspotX;
    m_cursorHotspotY = hotspotY;

    // a picture of our own replaces the pointer of the system, while this layer takes the input
    if ( !m_windowContext.IsNULL() )
        m_windowContext->UpdateSystemCursor();
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::SetCursorVisible( bool visible )
{GUCEF_TRACE;

    m_cursorVisible = visible;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::IsCursorVisible( void ) const
{GUCEF_TRACE;

    return m_cursorVisible;
}

/*-------------------------------------------------------------------------*/

const CString&
CGUIContext::GetCursorImage( void ) const
{GUCEF_TRACE;

    return m_cursorImage;
}

/*-------------------------------------------------------------------------*/

Int32
CGUIContext::GetCursorHotspotX( void ) const
{GUCEF_TRACE;

    return m_cursorHotspotX;
}

/*-------------------------------------------------------------------------*/

Int32
CGUIContext::GetCursorHotspotY( void ) const
{GUCEF_TRACE;

    return m_cursorHotspotY;
}

/*-------------------------------------------------------------------------*/

Int32
CGUIContext::GetPointerX( void ) const
{GUCEF_TRACE;

    return m_pointerX;
}

/*-------------------------------------------------------------------------*/

Int32
CGUIContext::GetPointerY( void ) const
{GUCEF_TRACE;

    return m_pointerY;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::DispatchPointer( CWidget* target, const CORE::CEvent& eventId, TMouseButton button, UInt32 clickCount )
{GUCEF_TRACE;

    CPointerEventData eventData;
    eventData.x = m_pointerX;
    eventData.y = m_pointerY;
    eventData.button = button;
    eventData.buttonStates = m_buttonStates;
    eventData.modifiers = m_modifiers;
    eventData.clickCount = clickCount;
    if ( !target->DispatchEvent( eventId, eventData ) )
        target->OnDefaultAction( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

/**
 *  While observers run for an event the widgets they run for must not be deleted: a widget that is destroyed meanwhile is taken
 *  out of the tree and deleted when the outermost dispatch is done. This is the dispatch: the destruction of what waits is
 *  at the end of the scope, also when the dispatch returns early or an exception unwinds it
 */
class CGUIContext::CDispatchScope
{
    public:

    explicit CDispatchScope( CGUIContext& context )
        : m_context( context )
    {
        ++m_context.m_dispatchDepth;
    }

    ~CDispatchScope()
    {
        --m_context.m_dispatchDepth;
        m_context.ProcessPendingDestroys();
    }

    private:

    CDispatchScope( const CDispatchScope& src );
    CDispatchScope& operator=( const CDispatchScope& src );

    CGUIContext& m_context;
};

/*-------------------------------------------------------------------------*/

void
CGUIContext::CancelPointerCapture( const char* reason )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pressed )
        return;

    CString cause = "cancelled";
    if ( GUCEF_NULL != reason )
        cause = reason;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI layer " << m_layer << ": the pointer capture of " << DescribeWidget( m_pressed ) << " is lost after "
                    << m_pressedMoveCount << " moves: " << cause );

    // the widget that was pressed loses the pointer without a release (the button state of the pointer is forgotten too)
    CWidget* pressed = m_pressed;
    TMouseButton button = m_pressedButton;
    m_pressed = GUCEF_NULL;
    m_pressedButton = MOUSEBUTTON_NONE;
    m_pressedMoveCount = 0;
    if ( MOUSEBUTTON_NONE != button )
        m_buttonStates &= ~( 1u << (UInt32) button );
    pressed->SetStateFlag( CWidget::STATE_PRESSED, false );

    CDispatchScope dispatchScope( *this );
    CPointerEventData eventData;
    eventData.x = m_pointerX;
    eventData.y = m_pointerY;
    eventData.button = button;
    eventData.buttonStates = m_buttonStates;
    eventData.modifiers = m_modifiers;
    pressed->DispatchDirectEvent( CWidget::MouseCaptureLostEvent, eventData );
    pressed->OnDefaultAction( CWidget::MouseCaptureLostEvent, eventData );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::InjectMouseMove( Int32 x, Int32 y )
{GUCEF_TRACE;

    FlushLayoutIfNeeded();
    m_pointerX = x;
    m_pointerY = y;

    ++m_dispatchDepth;
    // the hit test is the costly part of a move: once is enough, unless the handlers of a change of hover changed what is there
    CWidget* previousHover = m_hovered;
    CWidget* found = UpdatePointerTarget();
    CWidget* target = GUCEF_NULL != m_pressed ? m_pressed : ( previousHover != m_hovered ? HitTest( x, y ) : found );
    if ( GUCEF_NULL != m_pressed )
        ++m_pressedMoveCount;
    if ( GUCEF_NULL != target && AcceptsInput( target ) )
        DispatchPointer( target, CWidget::MouseMoveEvent, MOUSEBUTTON_NONE, 0 );
    --m_dispatchDepth;
    ProcessPendingDestroys();
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::InjectMouseButton( TMouseButton button, bool down )
{GUCEF_TRACE;

    FlushLayoutIfNeeded();
    UInt32 bit = 1u << (UInt32) button;
    ++m_dispatchDepth;

    if ( down )
    {
        m_buttonStates |= bit;
        CWidget* target = HitTest( m_pointerX, m_pointerY );
        CString inputNote;
        if ( GUCEF_NULL != target && !AcceptsInput( target ) )
            inputNote = ", which takes no input";
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI layer " << m_layer << ": mouse button " << (int) button << " down at " << m_pointerX << "," << m_pointerY
                        << " over " << DescribeWidget( target ) << inputNote );

        // a press outside the popup closes it and is used up by that
        if ( GUCEF_NULL != m_popup && ( GUCEF_NULL == target || !IsDescendantOf( target, m_popup ) ) )
        {
            ClosePopup();
            --m_dispatchDepth;
            ProcessPendingDestroys();
            return;
        }

        if ( GUCEF_NULL != target && AcceptsInput( target ) )
        {
            // the press gives the focus to the first focusable widget at or above the target
            for ( CWidget* w=target; GUCEF_NULL != w; w=w->m_parent )
            {
                if ( w->IsFocusable() )
                {
                    SetFocus( w );
                    break;
                }
            }

            if ( GUCEF_NULL == m_pressed )
            {
                m_pressed = target;
                m_pressedButton = button;
                m_pressedMoveCount = 0;
                target->SetStateFlag( CWidget::STATE_PRESSED, true );
            }
            DispatchPointer( target, CWidget::MouseDownEvent, button, 0 );
        }
    }
    else
    {
        m_buttonStates &= ~bit;
        if ( GUCEF_NULL != m_pressed && button == m_pressedButton )
        {
            CWidget* pressed = m_pressed;
            m_pressed = GUCEF_NULL;
            m_pressedButton = MOUSEBUTTON_NONE;
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI layer " << m_layer << ": mouse button " << (int) button << " up at " << m_pointerX << "," << m_pointerY
                            << " after " << m_pressedMoveCount << " moves, pressed " << DescribeWidget( pressed ) );
            m_pressedMoveCount = 0;
            pressed->SetStateFlag( CWidget::STATE_PRESSED, false );
            DispatchPointer( pressed, CWidget::MouseUpEvent, button, 0 );

            // a click: released over the widget that was pressed (it may have gone away meanwhile)
            CWidget* hit = HitTest( m_pointerX, m_pointerY );
            if ( m_widgets.find( pressed ) != m_widgets.end() && GUCEF_NULL != hit && IsDescendantOf( hit, pressed ) && AcceptsInput( pressed ) )
            {
                Int32 dx = m_pointerX - m_lastClickX;
                Int32 dy = m_pointerY - m_lastClickY;
                if ( pressed == m_lastClickTarget && m_timeInMs - m_lastClickTime <= DOUBLECLICK_TIME_MS &&
                     dx >= -DOUBLECLICK_DISTANCE && dx <= DOUBLECLICK_DISTANCE && dy >= -DOUBLECLICK_DISTANCE && dy <= DOUBLECLICK_DISTANCE )
                {
                    m_clickCount = m_clickCount >= 3 ? 1 : m_clickCount + 1;
                }
                else
                {
                    m_clickCount = 1;
                }
                m_lastClickTarget = pressed;
                m_lastClickTime = m_timeInMs;
                m_lastClickX = m_pointerX;
                m_lastClickY = m_pointerY;

                UInt32 clickCount = m_clickCount;
                DispatchPointer( pressed, CWidget::MouseClickEvent, button, clickCount );
                if ( 2 == clickCount && m_widgets.find( pressed ) != m_widgets.end() )
                    DispatchPointer( pressed, CWidget::MouseDoubleClickEvent, button, clickCount );
                if ( 3 == clickCount && m_widgets.find( pressed ) != m_widgets.end() )
                    DispatchPointer( pressed, CWidget::MouseTripleClickEvent, button, clickCount );

                if ( MOUSEBUTTON_LEFT == button && m_widgets.find( pressed ) != m_widgets.end() && pressed->IsActivatable() )
                {
                    CRoutedEventData eventData;
                    if ( !pressed->DispatchEvent( CWidget::ActivatedEvent, eventData ) )
                        pressed->OnDefaultAction( CWidget::ActivatedEvent, eventData );
                }
            }
        }
        else
        {
            CWidget* target = HitTest( m_pointerX, m_pointerY );
            if ( GUCEF_NULL != target && AcceptsInput( target ) )
                DispatchPointer( target, CWidget::MouseUpEvent, button, 0 );
        }
    }

    UpdatePointerTarget();
    --m_dispatchDepth;
    ProcessPendingDestroys();
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::InjectMouseWheel( Int32 delta )
{GUCEF_TRACE;

    // no wheel turns this far in one event; the widgets multiply the notches into distances and add them to positions
    const Int32 maxDelta = 65536;
    if ( delta > maxDelta )
        delta = maxDelta;
    else
    if ( delta < -maxDelta )
        delta = -maxDelta;

    FlushLayoutIfNeeded();
    CWidget* target = HitTest( m_pointerX, m_pointerY );

    // a widget that takes no input (disabled, or in something disabled) does not stop the wheel: it goes to what it is in
    while ( GUCEF_NULL != target && !AcceptsInput( target ) )
        target = target->m_parent;
    if ( GUCEF_NULL == target || 0 == delta )
    {
        if ( 0 != delta )
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI layer " << m_layer << ": wheel " << delta << " at " << m_pointerX << "," << m_pointerY << " over nothing that takes input" );
        return;
    }

    ++m_dispatchDepth;
    CWheelEventData eventData;
    eventData.x = m_pointerX;
    eventData.y = m_pointerY;
    eventData.delta = delta;
    eventData.modifiers = m_modifiers;
    CWidget* taker = GUCEF_NULL;
    if ( !target->DispatchEvent( CWidget::MouseWheelEvent, eventData ) )
    {
        // the first widget at or above the target that does something with the wheel (scrolling), one that has nothing to scroll leaves it to the next
        for ( CWidget* w=target; GUCEF_NULL != w; w=w->m_parent )
            if ( w->OnDefaultAction( CWidget::MouseWheelEvent, eventData ) )
            {
                taker = w;
                break;
            }
    }
    else
        taker = target;
    CString takerNote = ", not used by anything";
    if ( GUCEF_NULL != taker )
        takerNote = ", used by " + DescribeWidget( taker );
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI layer " << m_layer << ": wheel " << delta << " at " << m_pointerX << "," << m_pointerY << " over " << DescribeWidget( target )
                    << takerNote );
    --m_dispatchDepth;
    ProcessPendingDestroys();
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::GetKeyboardTarget( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_focus )
        return m_focus;
    if ( GUCEF_NULL != m_popup )
        return m_popup;
    CWidget* modal = GetModalRoot();
    if ( GUCEF_NULL != modal )
        return modal;
    for ( Int32 i=(Int32)m_roots.size()-1; i>=0; --i )
        if ( m_roots[ i ]->IsVisible() )
            return m_roots[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::InjectKey( INPUT::KeyCode keyCode, bool down, UInt32 modifiers )
{GUCEF_TRACE;

    FlushLayoutIfNeeded();
    m_modifiers = modifiers;
    CWidget* target = GetKeyboardTarget();
    if ( GUCEF_NULL == target )
        return;

    ++m_dispatchDepth;
    CKeyEventData eventData;
    eventData.keyCode = keyCode;
    eventData.modifiers = modifiers;
    const CORE::CEvent& eventId = down ? CWidget::KeyDownEvent : CWidget::KeyUpEvent;
    if ( !target->DispatchEvent( eventId, eventData ) && down && !target->OnDefaultAction( eventId, eventData ) )
    {
        // keys nobody wanted scroll the page they are on, else they become navigation
        bool shift = 0 != ( modifiers & INPUT::KEYMOD_SHIFT );
        if ( !ScrollPanelByKey( keyCode, modifiers ) ) switch ( keyCode )
        {
            case INPUT::KEYCODE_TAB: InjectNavigation( shift ? NAVIGATION_PREVIOUS : NAVIGATION_NEXT ); break;
            case INPUT::KEYCODE_UP: InjectNavigation( NAVIGATION_UP ); break;
            case INPUT::KEYCODE_DOWN: InjectNavigation( NAVIGATION_DOWN ); break;
            case INPUT::KEYCODE_LEFT: InjectNavigation( NAVIGATION_LEFT ); break;
            case INPUT::KEYCODE_RIGHT: InjectNavigation( NAVIGATION_RIGHT ); break;
            case INPUT::KEYCODE_RETURN:
            case INPUT::KEYCODE_NUMPADENTER: InjectNavigation( NAVIGATION_ACCEPT ); break;
            case INPUT::KEYCODE_ESCAPE: InjectNavigation( NAVIGATION_CANCEL ); break;
            default: break;
        }
    }
    --m_dispatchDepth;
    ProcessPendingDestroys();
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::ScrollPanelByKey( INPUT::KeyCode keyCode, UInt32 modifiers )
{GUCEF_TRACE;

    if ( 0 != ( modifiers & ( INPUT::KEYMOD_CTRL | INPUT::KEYMOD_ALT | INPUT::KEYMOD_META ) ) )
        return false;

    // the scroll panel around the focused widget, else the one below the pointer; the arrows are for moving the focus when there is one
    CWidget* start = m_focus;
    const bool lines = GUCEF_NULL == start;
    if ( GUCEF_NULL != start )
    {
        // a widget that types or picks has these keys for itself
        CWidgetTypeRegistry& registry = CGuiGlobal::Instance()->GetWidgetTypeRegistry();
        const CString& type = start->GetType();
        if ( registry.IsA( type, "Editbox" ) || registry.IsA( type, "Textbox" ) || registry.IsA( type, "Spinner" ) || registry.IsA( type, "Combobox" ) )
            return false;
    }
    else
    {
        start = HitTest( m_pointerX, m_pointerY );
    }
    for ( CWidget* w=start; GUCEF_NULL != w; w=w->m_parent )
    {
        CScrollPanel* scrollPanel = CWidget::ToScrollPanel( w );
        if ( GUCEF_NULL != scrollPanel && scrollPanel->IsScrollable() && AcceptsInput( scrollPanel ) )
            return scrollPanel->ScrollByKey( keyCode, lines );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::InjectText( const CString& text )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return;
    FlushLayoutIfNeeded();
    CWidget* target = GetKeyboardTarget();
    if ( GUCEF_NULL == target )
        return;

    ++m_dispatchDepth;
    CTextEventData eventData;
    eventData.text = text;
    if ( !target->DispatchEvent( CWidget::TextInputEvent, eventData ) )
        target->OnDefaultAction( CWidget::TextInputEvent, eventData );
    --m_dispatchDepth;
    ProcessPendingDestroys();
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::InjectNavigation( TNavigation direction )
{GUCEF_TRACE;

    FlushLayoutIfNeeded();
    CWidget* target = GetKeyboardTarget();
    if ( GUCEF_NULL == target )
        return;

    ++m_dispatchDepth;
    CNavigationEventData eventData;
    eventData.direction = direction;
    if ( !target->DispatchEvent( CWidget::NavigationEvent, eventData ) && !target->OnDefaultAction( CWidget::NavigationEvent, eventData ) )
    {
        if ( NAVIGATION_ACCEPT == direction )
        {
            if ( GUCEF_NULL != m_focus && m_focus->IsActivatable() && AcceptsInput( m_focus ) )
            {
                CWidget* focus = m_focus;
                CRoutedEventData activated;
                if ( !focus->DispatchEvent( CWidget::ActivatedEvent, activated ) )
                    focus->OnDefaultAction( CWidget::ActivatedEvent, activated );
            }
        }
        else
        if ( NAVIGATION_CANCEL == direction )
        {
            if ( GUCEF_NULL != m_popup )
                ClosePopup();
        }
        else
        {
            Navigate( direction );
        }
    }
    --m_dispatchDepth;
    ProcessPendingDestroys();
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::ProcessWindowMouseMove( Int32 windowX, Int32 windowY )
{GUCEF_TRACE;

    Int32 x = 0, y = 0;
    WindowToLogical( windowX, windowY, x, y );
    InjectMouseMove( x, y );
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::HitTestWidget( CWidget* widget, Int32 x, Int32 y ) const
{GUCEF_TRACE;

    if ( !widget->IsVisible() )
        return GUCEF_NULL;
    bool inside = widget->m_rect.Contains( x, y );
    if ( !inside && widget->GetBoolProperty( EWidgetProperty_Clip ) )
        return GUCEF_NULL;

    if ( widget->AreChildrenShown() )
    {
        for ( Int32 i=(Int32)widget->m_children.size()-1; i>=0; --i )
        {
            CWidget* hit = HitTestWidget( widget->m_children[ i ], x, y );
            if ( GUCEF_NULL != hit )
                return hit;
        }
    }
    if ( inside && !widget->GetBoolProperty( EWidgetProperty_InputTransparent ) )
        return widget;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::HitTest( Int32 x, Int32 y ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_popup )
    {
        CWidget* hit = HitTestWidget( m_popup, x, y );
        if ( GUCEF_NULL != hit )
            return hit;
    }

    CWidget* modal = GetModalRoot();
    if ( GUCEF_NULL != modal )
        return HitTestWidget( modal, x, y );

    for ( Int32 i=(Int32)m_roots.size()-1; i>=0; --i )
    {
        if ( m_roots[ i ] == m_popup )
            continue;
        CWidget* hit = HitTestWidget( m_roots[ i ], x, y );
        if ( GUCEF_NULL != hit )
            return hit;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::AcceptsInput( const CWidget* widget ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == widget || !widget->IsEffectivelyVisible() || !widget->IsEffectivelyEnabled() )
        return false;
    const CWidget* root = widget;
    while ( GUCEF_NULL != root->m_parent )
        root = root->m_parent;
    if ( !IsRootShown( root ) )
        return false;
    CWidget* modal = GetModalRoot();
    return GUCEF_NULL == modal || root == modal || root == m_popup;
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::UpdatePointerTarget( void )
{GUCEF_TRACE;

    // a press that can no longer be released into the widget (it was disabled or hidden, a layer above took the input)
    if ( GUCEF_NULL != m_pressed && ( !AcceptsInput( m_pressed ) || HasHigherLayerTakingInput() ) )
        CancelPointerCapture( HasHigherLayerTakingInput() ? "a layer above takes the input" : "the widget takes no input any more" );

    CWidget* found = HitTest( m_pointerX, m_pointerY );
    CWidget* hit = found;
    if ( GUCEF_NULL != hit && !AcceptsInput( hit ) )
        hit = GUCEF_NULL;
    if ( GUCEF_NULL != m_pressed && hit != m_pressed )
        hit = GUCEF_NULL;  // while a widget is pressed only that one shows hover
    SetHovered( hit );
    return found;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::SetHovered( CWidget* widget )
{GUCEF_TRACE;

    if ( widget == m_hovered )
        return;

    CWidget* old = m_hovered;
    m_hovered = widget;
    m_hoverStartTime = m_timeInMs;
    if ( GUCEF_NULL != old )
    {
        old->SetStateFlag( CWidget::STATE_HOVERED, false );
        CPointerEventData eventData;
        eventData.x = m_pointerX;
        eventData.y = m_pointerY;
        old->DispatchDirectEvent( CWidget::MouseFocusLostEvent, eventData );
        old->OnDefaultAction( CWidget::MouseFocusLostEvent, eventData );
    }
    if ( GUCEF_NULL != widget )
    {
        widget->SetStateFlag( CWidget::STATE_HOVERED, true );
        CPointerEventData eventData;
        eventData.x = m_pointerX;
        eventData.y = m_pointerY;
        widget->DispatchDirectEvent( CWidget::MouseFocusReceivedEvent, eventData );
        widget->OnDefaultAction( CWidget::MouseFocusReceivedEvent, eventData );
    }
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::GetFocus( void ) const
{GUCEF_TRACE;

    return m_focus;
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::GetHovered( void ) const
{GUCEF_TRACE;

    return m_hovered;
}

/*-------------------------------------------------------------------------*/

UInt64
CGUIContext::GetHoverDurationInMs( void ) const
{GUCEF_TRACE;

    return m_timeInMs - m_hoverStartTime;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::SetFocus( CWidget* widget )
{GUCEF_TRACE;

    if ( widget == m_focus )
        return true;
    if ( GUCEF_NULL != widget && ( widget->m_context != this || !widget->IsFocusable() ) )
        return false;

    CWidget* old = m_focus;
    m_focus = widget;
    if ( GUCEF_NULL != old )
    {
        old->SetStateFlag( CWidget::STATE_FOCUSED, false );
        CFocusEventData eventData;
        eventData.other = widget;
        old->DispatchDirectEvent( CWidget::KeyboardFocusLostEvent, eventData );
        old->OnDefaultAction( CWidget::KeyboardFocusLostEvent, eventData );
    }
    if ( GUCEF_NULL != widget && widget == m_focus )
    {
        widget->SetStateFlag( CWidget::STATE_FOCUSED, true );
        CFocusEventData eventData;
        eventData.other = old;
        widget->DispatchDirectEvent( CWidget::KeyboardFocusReceivedEvent, eventData );
        widget->OnDefaultAction( CWidget::KeyboardFocusReceivedEvent, eventData );

        // containers that scroll show what got the focus
        if ( widget == m_focus )
        {
            FlushLayoutIfNeeded();
            for ( CWidget* ancestor=widget->m_parent; GUCEF_NULL != ancestor; ancestor=ancestor->m_parent )
                ancestor->OnDescendantFocusReceived( widget );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::CollectFocusable( CWidget* widget, TWidgetVector& focusable ) const
{GUCEF_TRACE;

    if ( !widget->IsVisible() || !widget->IsEnabled() )
        return;
    if ( widget->IsFocusable() )
        focusable.push_back( widget );
    if ( !widget->AreChildrenShown() )
        return;
    for ( UInt32 i=0; i<(UInt32)widget->m_children.size(); ++i )
        CollectFocusable( widget->m_children[ i ], focusable );
}

/*-------------------------------------------------------------------------*/

CWidget*
CGUIContext::GetRootOf( CWidget* widget ) const
{GUCEF_TRACE;

    while ( GUCEF_NULL != widget && GUCEF_NULL != widget->m_parent )
        widget = widget->m_parent;
    return widget;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::Navigate( TNavigation direction )
{GUCEF_TRACE;

    FlushLayoutIfNeeded();

    // the scope: the popup, the modal root, the root of the focus, else the topmost root
    CWidget* scope = m_popup;
    if ( GUCEF_NULL == scope )
        scope = GetModalRoot();
    if ( GUCEF_NULL == scope && GUCEF_NULL != m_focus )
        scope = GetRootOf( m_focus );
    if ( GUCEF_NULL == scope && !m_roots.empty() )
        scope = m_roots.back();
    if ( GUCEF_NULL == scope )
        return false;

    TWidgetVector focusable;
    CollectFocusable( scope, focusable );
    if ( focusable.empty() )
        return false;

    // tab order: widgets with a TabIndex >= 0 by that index, then the rest in tree order
    TWidgetVector ordered;
    GUCEF::vector< std::pair< Int32, UInt32 > > indexed;
    for ( UInt32 i=0; i<(UInt32)focusable.size(); ++i )
    {
        Int32 tabIndex = focusable[ i ]->GetIntProperty( EWidgetProperty_TabIndex );
        if ( tabIndex >= 0 )
            indexed.push_back( std::pair< Int32, UInt32 >( tabIndex, i ) );
    }
    std::stable_sort( indexed.begin(), indexed.end() );
    for ( UInt32 i=0; i<(UInt32)indexed.size(); ++i )
        ordered.push_back( focusable[ indexed[ i ].second ] );
    for ( UInt32 i=0; i<(UInt32)focusable.size(); ++i )
        if ( focusable[ i ]->GetIntProperty( EWidgetProperty_TabIndex ) < 0 )
            ordered.push_back( focusable[ i ] );

    if ( GUCEF_NULL == m_focus || !IsDescendantOf( m_focus, scope ) )
        return SetFocus( NAVIGATION_PREVIOUS == direction ? ordered.back() : ordered.front() );

    Int32 current = -1;
    for ( UInt32 i=0; i<(UInt32)ordered.size(); ++i )
        if ( ordered[ i ] == m_focus )
            current = (Int32) i;

    if ( NAVIGATION_NEXT == direction || NAVIGATION_PREVIOUS == direction )
    {
        Int32 count = (Int32) ordered.size();
        Int32 next = NAVIGATION_NEXT == direction ? ( current + 1 ) % count : ( current - 1 + count ) % count;
        return SetFocus( ordered[ next ] );
    }

    // an explicit neighbour first
    const EWidgetProperty neighbourProperty = NAVIGATION_UP == direction ? EWidgetProperty_NavUp : NAVIGATION_DOWN == direction ? EWidgetProperty_NavDown :
                                              NAVIGATION_LEFT == direction ? EWidgetProperty_NavLeft : EWidgetProperty_NavRight;
    CString neighbourName = m_focus->GetProperty( neighbourProperty );
    if ( !neighbourName.IsNULLOrEmpty() )
    {
        CWidget* neighbour = scope->GetName() == neighbourName ? scope : scope->FindWidget( neighbourName );
        if ( GUCEF_NULL != neighbour && neighbour->IsFocusable() )
            return SetFocus( neighbour );
    }

    // else the nearest focusable widget in that direction, measured between centres; leaving the line costs double
    const CRect& from = m_focus->m_rect;
    Int32 fx = from.left + from.width / 2;
    Int32 fy = from.top + from.height / 2;
    CWidget* best = GUCEF_NULL;
    Int64 bestCost = 0;
    for ( UInt32 i=0; i<(UInt32)ordered.size(); ++i )
    {
        CWidget* candidate = ordered[ i ];
        if ( candidate == m_focus )
            continue;
        const CRect& to = candidate->m_rect;
        Int32 cx = to.left + to.width / 2;
        Int32 cy = to.top + to.height / 2;
        Int32 primary = 0;
        Int32 secondary = 0;
        switch ( direction )
        {
            case NAVIGATION_UP: primary = fy - cy; secondary = cx - fx; break;
            case NAVIGATION_DOWN: primary = cy - fy; secondary = cx - fx; break;
            case NAVIGATION_LEFT: primary = fx - cx; secondary = cy - fy; break;
            default: primary = cx - fx; secondary = cy - fy; break;
        }
        if ( primary <= 0 )
            continue;
        if ( secondary < 0 )
            secondary = -secondary;
        Int64 cost = (Int64) primary + 2 * (Int64) secondary;
        if ( GUCEF_NULL == best || cost < bestCost )
        {
            best = candidate;
            bestCost = cost;
        }
    }
    if ( GUCEF_NULL == best )
        return false;
    return SetFocus( best );
}

/*-------------------------------------------------------------------------*/

UInt64
CGUIContext::GetTimeInMs( void ) const
{GUCEF_TRACE;

    return m_timeInMs;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::RenderFrame( UInt32 deltaInMs )
{GUCEF_TRACE;

    m_timeInMs += deltaInMs;

    ++m_dispatchDepth;
    CORE::TCloneableUInt32 eventData( deltaInMs );
    NotifyObservers( FrameStartedEvent, &eventData );

    // widgets with time based behaviour: animations, video, the caret
    TWidgetVector updating;
    for ( TWidgetSet::iterator i=m_widgets.begin(); i!=m_widgets.end(); ++i )
        if ( (*i)->NeedsUpdate() )
            updating.push_back( *i );
    for ( UInt32 i=0; i<(UInt32)updating.size(); ++i )
        if ( m_widgets.find( updating[ i ] ) != m_widgets.end() )
            updating[ i ]->OnUpdate( deltaInMs );
    --m_dispatchDepth;
    ProcessPendingDestroys();

    FlushChanges();
    UpdatePointerTarget();
    FlushChanges();
    OnRender();
    NotifyObservers( FrameRenderedEvent );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::FlushLayoutIfNeeded( void )
{GUCEF_TRACE;

    if ( m_layoutDirty && !m_inFlush )
        UpdateLayout();
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::FlushChanges( void )
{GUCEF_TRACE;

    if ( m_inFlush )
        return;
    m_inFlush = true;

    if ( m_layoutDirty )
        UpdateLayout();

    // parents before children, so a native parent exists before its children are put in it
    UInt32 rounds = 0;
    while ( !m_dirtyWidgets.empty() && rounds++ < 8 )
    {
        TWidgetVector dirty( m_dirtyWidgets.begin(), m_dirtyWidgets.end() );
        m_dirtyWidgets.clear();
        GUCEF::vector< std::pair< Int32, CWidget* > > byDepth;
        for ( UInt32 i=0; i<(UInt32)dirty.size(); ++i )
            if ( m_widgets.find( dirty[ i ] ) != m_widgets.end() )
                byDepth.push_back( std::pair< Int32, CWidget* >( Depth( dirty[ i ] ), dirty[ i ] ) );
        std::stable_sort( byDepth.begin(), byDepth.end() );
        for ( UInt32 i=0; i<(UInt32)byDepth.size(); ++i )
            FlushWidget( *byDepth[ i ].second );
        if ( m_layoutDirty )
            UpdateLayout();
    }

    if ( m_rootOrderDirty )
    {
        m_rootOrderDirty = false;
        OnRootOrderChanged();
    }
    m_inFlush = false;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::FlushWidget( CWidget& widget )
{GUCEF_TRACE;

    CWidgetHandler* handler = widget.m_handler;
    CWidget::SPropertyMask dirtyMask = widget.m_dirtyMask;
    CWidget::TStringSet dirtyCustom;
    dirtyCustom.swap( widget.m_dirtyCustomProperties );
    widget.m_dirtyMask = CWidget::SPropertyMask();

    if ( GUCEF_NULL == handler )
    {
        widget.m_parentDirty = widget.m_stateDirty = widget.m_rectDirty = widget.m_childOrderDirty = false;
        return;
    }

    if ( widget.m_parentDirty )
    {
        widget.m_parentDirty = false;
        handler->ApplyParent( widget );
    }

    // the drivers take the names: the id turns into the name here, once per change; the order is that of the names (the ids are in it)
    CWidget::TStringSet::const_iterator custom = dirtyCustom.begin();
    if ( !dirtyMask.IsEmpty() || !dirtyCustom.empty() )
    {
        for ( UInt32 id=dirtyMask.NextSet( 1 ); id<(UInt32)EWidgetProperty_Count; id=dirtyMask.NextSet( id + 1 ) )
        {
            const EWidgetProperty property = (EWidgetProperty) id;
            const CString& name = WidgetPropertyToString( property );
            while ( custom != dirtyCustom.end() && (*custom) < name )
            {
                CString customValue = widget.GetPropertyRef( *custom );
                ApplyPropertyToHandler( widget, *handler, *custom, customValue );
                ++custom;
            }
            CString value = widget.GetPropertyRef( property );
            ApplyPropertyToHandler( widget, *handler, name, value );
        }
        while ( custom != dirtyCustom.end() )
        {
            CString customValue = widget.GetPropertyRef( *custom );
            ApplyPropertyToHandler( widget, *handler, *custom, customValue );
            ++custom;
        }
    }
    if ( widget.m_stateDirty )
    {
        widget.m_stateDirty = false;
        handler->ApplyState( widget );
    }
    if ( widget.m_rectDirty )
    {
        widget.m_rectDirty = false;
        handler->ApplyLayout( widget, widget.m_rect );
    }
    if ( widget.m_childOrderDirty )
    {
        widget.m_childOrderDirty = false;
        handler->ApplyChildOrder( widget );
    }
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::ApplyPropertyToHandler( CWidget& widget, CWidgetHandler& handler, const CString& name, const CString& value )
{GUCEF_TRACE;

    if ( !widget.m_applierOverrides.empty() )
    {
        CWidget::TApplierOverrideMap::iterator o = widget.m_applierOverrides.find( name );
        if ( o != widget.m_applierOverrides.end() && GUCEF_NULL != (*o).second )
        {
            (*o).second( handler, widget, value );
            return;
        }
    }
    handler.ApplyProperty( widget, name, value );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::InvalidateLayout( void )
{GUCEF_TRACE;

    m_layoutDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::UpdateLayout( void )
{GUCEF_TRACE;

    m_layoutDirty = false;
    CRect screen( 0, 0, (Int32) GetLogicalWidth(), (Int32) GetLogicalHeight() );
    for ( TWidgetSet::iterator i=m_widgets.begin(); i!=m_widgets.end(); ++i )
    {
        CWidget* widget = *i;
        if ( GUCEF_NULL == widget->m_parent )
            PlaceWidget( *widget, screen, false );
    }
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::ResolveSize( const CWidget& widget, const CRect& area, Int32& width, Int32& height ) const
{GUCEF_TRACE;

    CString sizeValue = widget.GetProperty( EWidgetProperty_Size );
    Int32 naturalWidth = 0, naturalHeight = 0;
    bool natural = sizeValue.IsNULLOrEmpty();
    if ( natural )
        widget.GetNaturalSize( naturalWidth, naturalHeight );

    if ( natural )
    {
        width = naturalWidth;
        height = naturalHeight;
        widget.AdjustSize( width, height, area );
        return;
    }
    CUVector size;
    CUVector::FromString( sizeValue, size );
    width = size.GetX().Resolve( area.width );
    height = size.GetY().Resolve( area.height );

    // a widget that sizes its height to its content (a group box) keeps the width of its Size
    if ( widget.GetBoolProperty( EWidgetProperty_AutoHeight ) && widget.GetNaturalSize( naturalWidth, naturalHeight ) )
        height = naturalHeight;
    widget.AdjustSize( width, height, area );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::PlaceWidget( CWidget& widget, const CRect& area, bool sizeFromArea )
{GUCEF_TRACE;

    CRect rect;
    if ( sizeFromArea )
    {
        rect = area;
    }
    else
    {
        Int32 width = 0, height = 0;
        ResolveSize( widget, area, width, height );

        // a size that is the part of a small parent less pixels can come out negative: there is nothing of the widget then
        if ( width < 0 )
            width = 0;
        if ( height < 0 )
            height = 0;

        CUVector position = widget.GetUVectorProperty( EWidgetProperty_Position );
        Int32 x = position.GetX().Resolve( area.width );
        Int32 y = position.GetY().Resolve( area.height );

        // the position may be the centre or far edge of the widget instead of its top left
        CString hAlign = widget.GetProperty( EWidgetProperty_HAlign );
        if ( hAlign == "Center" )
            x -= width / 2;
        else if ( hAlign == "Right" )
            x -= width;
        CString vAlign = widget.GetProperty( EWidgetProperty_VAlign );
        if ( vAlign == "Center" )
            y -= height / 2;
        else if ( vAlign == "Bottom" )
            y -= height;

        rect = CRect( area.left + x, area.top + y, width, height );
    }

    widget.AdjustPlacement( rect, area, sizeFromArea );
    ApplyLayoutResult( widget, rect );
    LayoutChildren( widget );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::ApplyLayoutResult( CWidget& widget, const CRect& rect )
{GUCEF_TRACE;

    if ( widget.m_rect == rect )
        return;
    widget.m_rect = rect;
    widget.m_rectDirty = true;
    m_dirtyWidgets.insert( &widget );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::ShiftSubtree( CWidget& widget, Int32 dy )
{GUCEF_TRACE;

    const CRect& rect = widget.m_rect;
    ApplyLayoutResult( widget, CRect( rect.left, rect.top + dy, rect.width, rect.height ) );
    for ( UInt32 i=0; i<(UInt32)widget.m_children.size(); ++i )
        ShiftSubtree( *widget.m_children[ i ], dy );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::LayoutScrollPanel( CScrollPanel& panel )
{GUCEF_TRACE;

    // the children are placed in the area of the column left of the scroll bar, moved up by the scroll position
    const CRect& rect = panel.m_rect;
    const CRect view = panel.GetViewportRect();
    Int32 viewWidth = view.width;
    Int32 viewHeight = rect.height < 0 ? 0 : rect.height;
    Int32 requested = panel.GetIntProperty( EWidgetProperty_ScrollPosition );
    Int32 scroll = requested < 0 ? 0 : requested;

    CRect area( rect.left + view.left, rect.top - scroll, viewWidth, viewHeight );
    Int32 bottom = rect.top - scroll;
    for ( UInt32 i=0; i<(UInt32)panel.m_children.size(); ++i )
    {
        CWidget& child = *panel.m_children[ i ];
        PlaceWidget( child, area, false );
        if ( child.IsVisible() && child.m_rect.Bottom() > bottom )
            bottom = child.m_rect.Bottom();
    }

    // the content from its top edge, as if it were not scrolled
    Int32 content = bottom - ( rect.top - scroll );
    if ( content > 0 )
        content += panel.GetIntProperty( EWidgetProperty_BottomPadding );
    panel.SetExtent( content, viewHeight );

    Int32 maxScroll = panel.GetMaxScrollPosition();
    if ( scroll > maxScroll )
    {
        // the content became smaller than what was scrolled out
        for ( UInt32 i=0; i<(UInt32)panel.m_children.size(); ++i )
            ShiftSubtree( *panel.m_children[ i ], scroll - maxScroll );
        scroll = maxScroll;
    }
    if ( scroll != requested )
        panel.SetIntProperty( EWidgetProperty_ScrollPosition, scroll );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::LayoutChildren( CWidget& widget )
{GUCEF_TRACE;

    // what a collapsed window hides is not laid out, it is when the window is expanded
    if ( !widget.AreChildrenShown() )
        return;

    CWidgetTypeRegistry& registry = CGuiGlobal::Instance()->GetWidgetTypeRegistry();
    const CString& type = widget.GetType();
    if ( registry.IsA( type, "ScrollPanel" ) )
    {
        CScrollPanel* scrollPanel = widget.AsScrollPanel();
        if ( GUCEF_NULL != scrollPanel )
        {
            LayoutScrollPanel( *scrollPanel );
            return;
        }
    }

    if ( widget.m_children.empty() )
        return;
    bool hbox = registry.IsA( type, "HBox" );
    bool vbox = registry.IsA( type, "VBox" );
    bool grid = registry.IsA( type, "Grid" );
    bool margin = registry.IsA( type, "Margin" );

    if ( registry.IsA( type, "TabControl" ) )
    {
        // the pages fill the area below the tabs
        Int32 header = widget.GetIntProperty( EWidgetProperty_TabHeight );
        CRect pages( widget.m_rect.left, widget.m_rect.top + header, widget.m_rect.width, widget.m_rect.height - header );
        if ( pages.height < 0 )
            pages.height = 0;
        for ( UInt32 i=0; i<(UInt32)widget.m_children.size(); ++i )
        {
            CWidget& child = *widget.m_children[ i ];
            if ( registry.IsA( child.GetType(), "TabContentPane" ) )
                PlaceWidget( child, pages, true );
            else
                PlaceWidget( child, widget.m_rect, false );
        }
        return;
    }

    if ( !hbox && !vbox && !grid && !margin )
    {
        for ( UInt32 i=0; i<(UInt32)widget.m_children.size(); ++i )
            PlaceWidget( *widget.m_children[ i ], widget.m_rect, false );
        return;
    }

    // the containers: children are put in the content area (the rectangle less the padding)
    Int32 padding = widget.GetIntProperty( EWidgetProperty_Padding );
    Int32 spacing = widget.GetIntProperty( EWidgetProperty_Spacing );
    CRect content( widget.m_rect.left + padding, widget.m_rect.top + padding, widget.m_rect.width - 2*padding, widget.m_rect.height - 2*padding );
    if ( registry.IsA( type, "GroupBox" ) )
    {
        // the caption is in the top line of the frame: the children start below it
        CGroupBox* groupBox = CWidget::As< CGroupBox >( &widget );
        Int32 caption = GUCEF_NULL != groupBox ? groupBox->GetCaptionHeight() : 0;
        content.top += caption;
        content.height -= caption;
    }
    if ( content.width < 0 ) content.width = 0;
    if ( content.height < 0 ) content.height = 0;

    if ( margin )
    {
        for ( UInt32 i=0; i<(UInt32)widget.m_children.size(); ++i )
            PlaceWidget( *widget.m_children[ i ], content, true );
        return;
    }

    if ( hbox || vbox )
    {
        Int32 offset = 0;
        for ( UInt32 i=0; i<(UInt32)widget.m_children.size(); ++i )
        {
            CWidget& child = *widget.m_children[ i ];
            Int32 w = 0, h = 0;
            ResolveSize( child, content, w, h );
            CRect cell;
            if ( hbox )
            {
                Int32 y = content.top;
                CString align = child.GetProperty( EWidgetProperty_VAlign );
                if ( align == "Center" ) y = content.top + ( content.height - h ) / 2;
                else if ( align == "Bottom" ) y = content.Bottom() - h;
                cell = CRect( content.left + offset, y, w, h );
                if ( child.IsVisible() )
                    offset += w + spacing;
            }
            else
            {
                Int32 x = content.left;
                CString align = child.GetProperty( EWidgetProperty_HAlign );
                if ( align == "Center" ) x = content.left + ( content.width - w ) / 2;
                else if ( align == "Right" ) x = content.Right() - w;
                cell = CRect( x, content.top + offset, w, h );
                if ( child.IsVisible() )
                    offset += h + spacing;
            }
            PlaceWidget( child, cell, true );
        }
        return;
    }

    // grid: equal columns, each row as high as its highest child
    Int32 columns = widget.GetIntProperty( EWidgetProperty_Columns );
    if ( columns < 1 )
        columns = 1;
    Int32 cellWidth = ( content.width - ( columns - 1 ) * spacing ) / columns;
    Int32 y = content.top;
    UInt32 index = 0;
    TWidgetVector visible;
    for ( UInt32 i=0; i<(UInt32)widget.m_children.size(); ++i )
        if ( widget.m_children[ i ]->IsVisible() )
            visible.push_back( widget.m_children[ i ] );
        else
            PlaceWidget( *widget.m_children[ i ], CRect( content.left, content.top, 0, 0 ), true );
    while ( index < (UInt32) visible.size() )
    {
        Int32 rowHeight = 0;
        for ( Int32 c=0; c<columns && index + c < (UInt32) visible.size(); ++c )
        {
            Int32 w = 0, h = 0;
            ResolveSize( *visible[ index + c ], CRect( 0, 0, cellWidth, content.height ), w, h );
            if ( h > rowHeight )
                rowHeight = h;
        }
        for ( Int32 c=0; c<columns && index < (UInt32) visible.size(); ++c, ++index )
            PlaceWidget( *visible[ index ], CRect( content.left + c * ( cellWidth + spacing ), y, cellWidth, rowHeight ), true );
        y += rowHeight + spacing;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::CaptureFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::SetLayer( Int32 layer )
{GUCEF_TRACE;

    m_layer = layer;
    if ( !m_windowContext.IsNULL() )
        m_windowContext->SortGuiLayers();
}

/*-------------------------------------------------------------------------*/

Int32
CGUIContext::GetLayer( void ) const
{GUCEF_TRACE;

    return m_layer;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::IsBottomLayer( void ) const
{GUCEF_TRACE;

    if ( m_windowContext.IsNULL() )
        return true;
    const CWindowContext::TGuiContextLayers& layers = m_windowContext->GetGuiLayers();
    return layers.empty() || layers.front() == this;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::HasVisibleRoots( void ) const
{GUCEF_TRACE;

    for ( TWidgetVector::const_iterator i=m_roots.begin(); i!=m_roots.end(); ++i )
        if ( GUCEF_NULL != (*i) && (*i)->IsVisible() )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGUIContext::HasHigherLayerTakingInput( void ) const
{GUCEF_TRACE;

    if ( m_windowContext.IsNULL() )
        return false;
    const CWindowContext::TGuiContextLayers& layers = m_windowContext->GetGuiLayers();
    bool above = false;
    for ( CWindowContext::TGuiContextLayers::const_iterator i=layers.begin(); i!=layers.end(); ++i )
    {
        if ( (*i) == this )
            above = true;
        else
        if ( above && (*i)->HasVisibleRoots() )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::RenderWindowLayers( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( m_windowContext.IsNULL() )
    {
        RenderFrame( deltaInMs );
        return;
    }

    // the layer that takes the input can be another one than last frame
    m_windowContext->UpdateSystemCursor();

    // a copy: a frame may make or take away contexts
    CWindowContext::TGuiContextLayers layers = m_windowContext->GetGuiLayers();
    for ( CWindowContext::TGuiContextLayers::iterator i=layers.begin(); i!=layers.end(); ++i )
    {
        const CWindowContext::TGuiContextLayers& current = m_windowContext->GetGuiLayers();
        if ( std::find( current.begin(), current.end(), (*i) ) != current.end() )
            (*i)->RenderFrame( deltaInMs );
    }
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnRender( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnRootOrderChanged( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnImageChanged( const CString& alias )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWidgetHandler*
CGUIContext::GetWidgetHandler( const CString& widgetType )
{GUCEF_TRACE;

    THandlerCache::iterator i = m_handlerCache.find( widgetType );
    if ( i != m_handlerCache.end() )
        return (*i).second;

    CWidgetHandler* handler = GUCEF_NULL;
    if ( GUCEF_NULL != m_driver )
    {
        GUCEF::vector< CString > chain = CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetTypeChain( widgetType );
        for ( UInt32 c=0; c<(UInt32)chain.size() && GUCEF_NULL == handler; ++c )
            handler = m_driver->GetWidgetHandler( chain[ c ] );
    }
    m_handlerCache[ widgetType ] = handler;
    return handler;
}

/*-------------------------------------------------------------------------*/

CGUIContext*
CGUIContext::AsGUIContext( void )
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnWindowRedraw( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the lowest layer draws all layers of the window, in their order
    if ( !IsBottomLayer() )
        return;
    UInt64 tick = MT::PrecisionTickCount();
    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt32 delta = 0;
    if ( 0 != m_lastRealTick && 0 != resolution )
        delta = (UInt32) ( ( tick - m_lastRealTick ) * 1000 / resolution );
    m_lastRealTick = tick;
    RenderWindowLayers( delta );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnWindowSize( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_layoutDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnWindowActivate( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the window was activated or deactivated: a press whose release goes to another window never ends
    CancelPointerCapture( "the window was activated or deactivated" );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnWidgetDirty( CWidget* widget )
{GUCEF_TRACE;

    m_dirtyWidgets.insert( widget );
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnWidgetDestroyed( CWidget* widget )
{GUCEF_TRACE;

    ForgetInteraction( widget, false );
    m_widgets.erase( widget );
    m_dirtyWidgets.erase( widget );
    m_updateWidgets.erase( widget );
    for ( TWidgetVector::iterator i=m_roots.begin(); i!=m_roots.end(); ++i )
    {
        if ( (*i) == widget )
        {
            m_roots.erase( i );
            m_rootOrderDirty = true;
            break;
        }
    }
    m_modalRoots.erase( widget );
    for ( TWidgetVector::iterator i=m_pendingDestroy.begin(); i!=m_pendingDestroy.end(); ++i )
    {
        if ( (*i) == widget )
        {
            m_pendingDestroy.erase( i );
            break;
        }
    }
    widget->m_context = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnWidgetParentChanged( CWidget* widget, CWidget* oldParent )
{GUCEF_TRACE;

    // a widget that moves below another one is no root any more
    if ( GUCEF_NULL != widget->m_parent )
    {
        for ( TWidgetVector::iterator i=m_roots.begin(); i!=m_roots.end(); ++i )
        {
            if ( (*i) == widget )
            {
                m_roots.erase( i );
                m_modalRoots.erase( widget );
                m_rootOrderDirty = true;
                break;
            }
        }
    }
    m_layoutDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::OnWidgetVisibilityChanged( CWidget* widget )
{GUCEF_TRACE;

    if ( !widget->IsVisible() )
        ForgetInteraction( widget, true );
    m_layoutDirty = true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
