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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#include "gucefCORE_CLogStreamScope.h"
#define GUCEF_CORE_CLOGSTREAMSCOPE_H
#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#include "gucefGUI_CGUIUserActionLog.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The routed event as the root sees it first: on its way down (tunnel), or at the root when it is the target */
CRoutedEventData*
AtFirstSight( CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CRoutedEventData* routed = CRoutedEventData::FromCloneable( eventData );
    if ( GUCEF_NULL == routed || GUCEF_NULL == routed->source || EVENTPHASE_BUBBLE == routed->phase )
        return GUCEF_NULL;
    return routed;
}

/*-------------------------------------------------------------------------*/

bool
IsWidgetOfType( const CWidget* widget, const char* baseTypeName )
{GUCEF_TRACE;

    return GUCEF_NULL != widget && CGuiGlobal::Instance()->GetWidgetTypeRegistry().IsA( widget->GetType(), baseTypeName );
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUIUserActionLog::CGUIUserActionLog( void )
    : CORE::CObserver()
    , m_contexts()
    , m_roots()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGUIUserActionLog::~CGUIUserActionLog()
{GUCEF_TRACE;

    DetachAll();
}

/*-------------------------------------------------------------------------*/

const CString&
CGUIUserActionLog::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CGUIUserActionLog";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CGUIUserActionLog::Attach( const TGuiContextPtr& contextPtr, const CString& contextName )
{GUCEF_TRACE;

    CIGUIContext* generic = const_cast< TGuiContextPtr& >( contextPtr ).GetPointerAlways();
    CGUIContext* context = GUCEF_NULL != generic ? generic->AsGUIContext() : GUCEF_NULL;
    if ( GUCEF_NULL == context )
        return false;

    // a context that went may have left its address to this one
    ForgetExpiredContexts();
    CORE::CNotifier* notifier = static_cast< CORE::CNotifier* >( context );
    TContextMap::iterator i = m_contexts.find( notifier );
    if ( i != m_contexts.end() )
    {
        (*i).second.name = contextName;
        return true;
    }

    SAttachedContext attached;
    attached.weak = contextPtr;
    attached.context = context;
    attached.name = contextName;
    m_contexts[ notifier ] = attached;

    TEventCallback shown( this, &CGUIUserActionLog::OnFormShown );
    SubscribeTo( notifier, CGUIContext::FormShownEvent, shown );
    TEventCallback hidden( this, &CGUIUserActionLog::OnFormHidden );
    SubscribeTo( notifier, CGUIContext::FormHiddenEvent, hidden );
    WatchRoots( *context );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::ForgetContext( CORE::CNotifier* contextNotifier, bool unsubscribe )
{GUCEF_TRACE;

    TContextMap::iterator i = m_contexts.find( contextNotifier );
    if ( i == m_contexts.end() )
        return;
    const CGUIContext* context = (*i).second.context;
    TRootMap::iterator r = m_roots.begin();
    while ( r != m_roots.end() )
    {
        if ( (*r).second.context == context )
        {
            // a root notifies its destruction: what is left here exists
            if ( unsubscribe )
                UnsubscribeFrom( *(*r).first );
            m_roots.erase( r++ );
        }
        else
        {
            ++r;
        }
    }
    if ( unsubscribe )
        UnsubscribeFrom( *contextNotifier );
    m_contexts.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::ForgetExpiredContexts( void )
{GUCEF_TRACE;

    TContextMap::iterator i = m_contexts.begin();
    while ( i != m_contexts.end() )
    {
        if ( (*i).second.weak.IsExpired() )
        {
            CORE::CNotifier* gone = (*i).first;
            ++i;
            ForgetContext( gone, false );
        }
        else
        {
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::Detach( const TGuiContextPtr& contextPtr )
{GUCEF_TRACE;

    ForgetExpiredContexts();
    CIGUIContext* generic = const_cast< TGuiContextPtr& >( contextPtr ).GetPointerAlways();
    CGUIContext* context = GUCEF_NULL != generic ? generic->AsGUIContext() : GUCEF_NULL;
    if ( GUCEF_NULL != context )
        ForgetContext( static_cast< CORE::CNotifier* >( context ), true );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::DetachAll( void )
{GUCEF_TRACE;

    // the observer knows the notifiers that still exist
    UnsubscribeAllFromObserver();
    m_roots.clear();
    m_contexts.clear();
}

/*-------------------------------------------------------------------------*/

bool
CGUIUserActionLog::IsAttached( const TGuiContextPtr& contextPtr )
{GUCEF_TRACE;

    ForgetExpiredContexts();
    CIGUIContext* generic = const_cast< TGuiContextPtr& >( contextPtr ).GetPointerAlways();
    CGUIContext* context = GUCEF_NULL != generic ? generic->AsGUIContext() : GUCEF_NULL;
    return GUCEF_NULL != context && m_contexts.find( static_cast< CORE::CNotifier* >( context ) ) != m_contexts.end();
}

/*-------------------------------------------------------------------------*/

UInt32
CGUIUserActionLog::GetAttachedContextCount( void )
{GUCEF_TRACE;

    ForgetExpiredContexts();
    return (UInt32) m_contexts.size();
}

/*-------------------------------------------------------------------------*/

bool
CGUIUserActionLog::IsSecretInput( const CWidget* widget )
{GUCEF_TRACE;

    for ( const CWidget* w=widget; GUCEF_NULL != w; w=w->GetParentWidget() )
        if ( IsWidgetOfType( w, "SecretInput" ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CString
CGUIUserActionLog::DescribeWidget( const CWidget* widget )
{GUCEF_TRACE;

    if ( GUCEF_NULL == widget )
        return "nothing";
    return widget->GetNamePath() + " (" + widget->GetType() + ")";
}

/*-------------------------------------------------------------------------*/

bool
CGUIUserActionLog::IsModifierKey( INPUT::KeyCode keyCode )
{GUCEF_TRACE;

    switch ( keyCode )
    {
        case INPUT::KEYCODE_LSHIFT:
        case INPUT::KEYCODE_RSHIFT:
        case INPUT::KEYCODE_LCONTROL:
        case INPUT::KEYCODE_RCONTROL:
        case INPUT::KEYCODE_LMENU:
        case INPUT::KEYCODE_RMENU:
        case INPUT::KEYCODE_LWIN:
        case INPUT::KEYCODE_RWIN:
        case INPUT::KEYCODE_CAPITAL:
        case INPUT::KEYCODE_NUMLOCK:
        case INPUT::KEYCODE_SCROLL:
            return true;
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

CString
CGUIUserActionLog::DescribeKey( INPUT::KeyCode keyCode, UInt32 modifiers )
{GUCEF_TRACE;

    CString text;
    if ( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) )
        text += "Ctrl+";
    if ( 0 != ( modifiers & INPUT::KEYMOD_ALT ) )
        text += "Alt+";
    if ( 0 != ( modifiers & INPUT::KEYMOD_META ) )
        text += "Meta+";
    if ( 0 != ( modifiers & INPUT::KEYMOD_SHIFT ) )
        text += "Shift+";
    return text + INPUT::KeyCodeToString( keyCode );
}

/*-------------------------------------------------------------------------*/

CString
CGUIUserActionLog::GetContextName( CORE::CNotifier* notifier ) const
{GUCEF_TRACE;

    TRootMap::const_iterator r = m_roots.find( notifier );
    if ( r != m_roots.end() )
        notifier = static_cast< CORE::CNotifier* >( (*r).second.context );
    TContextMap::const_iterator i = m_contexts.find( notifier );
    return i != m_contexts.end() ? (*i).second.name : CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::WatchRoot( CWidget& root, CGUIContext& context )
{GUCEF_TRACE;

    CORE::CNotifier* notifier = static_cast< CORE::CNotifier* >( &root );
    if ( m_roots.find( notifier ) != m_roots.end() )
        return;

    SWatchedRoot watched;
    watched.root = &root;
    watched.context = &context;
    m_roots[ notifier ] = watched;

    TEventCallback keyDown( this, &CGUIUserActionLog::OnKeyDown );
    SubscribeTo( notifier, CWidget::KeyDownEvent, keyDown );
    TEventCallback textInput( this, &CGUIUserActionLog::OnTextInput );
    SubscribeTo( notifier, CWidget::TextInputEvent, textInput );
    TEventCallback activated( this, &CGUIUserActionLog::OnActivated );
    SubscribeTo( notifier, CWidget::ActivatedEvent, activated );
    TEventCallback valueChanged( this, &CGUIUserActionLog::OnValueChanged );
    SubscribeTo( notifier, CWidget::ValueChangedEvent, valueChanged );
    TEventCallback itemActivated( this, &CGUIUserActionLog::OnItemActivated );
    SubscribeTo( notifier, CWidget::ItemActivatedEvent, itemActivated );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::WatchRoots( CGUIContext& context )
{GUCEF_TRACE;

    const CGUIContext::TWidgetVector& roots = context.GetRootWidgets();
    for ( CGUIContext::TWidgetVector::const_iterator i = roots.begin(); i != roots.end(); ++i )
        if ( GUCEF_NULL != (*i) )
            WatchRoot( *(*i), context );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent != eventId )
        return;

    // a root widget that goes: it is forgotten; it notifies nothing any more
    TRootMap::iterator r = m_roots.find( notifier );
    if ( r != m_roots.end() )
    {
        m_roots.erase( r );
        return;
    }

    // a context that tells it goes (a context of gucefGUI does not, see ForgetExpiredContexts()): forgotten with what is left of its roots
    if ( m_contexts.find( notifier ) != m_contexts.end() )
        ForgetContext( notifier, false );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::OnFormShown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    TContextMap::iterator i = m_contexts.find( notifier );
    if ( i == m_contexts.end() )
        return;

    // the roots are watched also while the user log is off, so that the log is complete once it is switched on
    CGUIContext& context = *(*i).second.context;
    WatchRoots( context );

    if ( !CORE::CLogStreamScope::IsUserLogEnabled() )
        return;
    const CString name = GUCEF_NULL != eventData ? static_cast< CORE::TCloneableString* >( eventData )->GetData() : CString::Empty;

    // the root that was shown is the topmost one with that name
    const CWidget* shown = GUCEF_NULL;
    const CGUIContext::TWidgetVector& roots = context.GetRootWidgets();
    for ( CGUIContext::TWidgetVector::const_reverse_iterator r = roots.rbegin(); r != roots.rend(); ++r )
    {
        if ( GUCEF_NULL != (*r) && (*r)->GetName() == name )
        {
            shown = (*r);
            break;
        }
    }
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI " << (*i).second.name << ": opened " << ( GUCEF_NULL != shown ? DescribeWidget( shown ) : name )
                    << ( GUCEF_NULL != shown && context.IsModalRoot( shown ) ? ", modal" : "" ) );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::OnFormHidden( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !CORE::CLogStreamScope::IsUserLogEnabled() )
        return;
    TContextMap::iterator i = m_contexts.find( notifier );
    if ( i == m_contexts.end() )
        return;
    const CString name = GUCEF_NULL != eventData ? static_cast< CORE::TCloneableString* >( eventData )->GetData() : CString::Empty;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI " << (*i).second.name << ": closed " << name );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::OnKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !CORE::CLogStreamScope::IsUserLogEnabled() )
        return;
    CRoutedEventData* routed = AtFirstSight( eventData );
    CKeyEventData* key = CORE::CICloneable::CloneableAs< CKeyEventData >( eventData );
    if ( GUCEF_NULL == routed || GUCEF_NULL == key || IsModifierKey( key->keyCode ) )
        return;

    // of a secret only the keys that leave or end the input: neither what is typed nor how much
    if ( IsSecretInput( routed->source ) )
    {
        const bool leaves = INPUT::KEYCODE_TAB == key->keyCode || INPUT::KEYCODE_RETURN == key->keyCode ||
                            INPUT::KEYCODE_NUMPADENTER == key->keyCode || INPUT::KEYCODE_ESCAPE == key->keyCode;
        if ( !leaves )
            return;
    }
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI " << GetContextName( notifier ) << ": key " << DescribeKey( key->keyCode, key->modifiers )
                    << " in " << DescribeWidget( routed->source ) );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::OnTextInput( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !CORE::CLogStreamScope::IsUserLogEnabled() )
        return;
    CRoutedEventData* routed = AtFirstSight( eventData );
    CTextEventData* text = CORE::CICloneable::CloneableAs< CTextEventData >( eventData );
    if ( GUCEF_NULL == routed || GUCEF_NULL == text || IsSecretInput( routed->source ) )
        return;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI " << GetContextName( notifier ) << ": typed \"" << text->text << "\" into " << DescribeWidget( routed->source ) );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::OnActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !CORE::CLogStreamScope::IsUserLogEnabled() )
        return;
    CRoutedEventData* routed = AtFirstSight( eventData );
    if ( GUCEF_NULL == routed )
        return;
    const CWidget* widget = routed->source;
    const bool showText = !IsSecretInput( widget ) && IsWidgetOfType( widget, "Button" );
    const CString& caption = showText ? widget->GetPropertyRef( EWidgetProperty_Text ) : CString::Empty;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI " << GetContextName( notifier ) << ": activated " << DescribeWidget( widget )
                    << ( caption.IsNULLOrEmpty() ? CString::Empty : " \"" + caption + "\"" ) );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::OnValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !CORE::CLogStreamScope::IsUserLogEnabled() )
        return;
    CRoutedEventData* routed = AtFirstSight( eventData );
    CValueChangedEventData* changed = CORE::CICloneable::CloneableAs< CValueChangedEventData >( eventData );
    if ( GUCEF_NULL == routed || GUCEF_NULL == changed )
        return;

    // a secret never; what is typed into a text field is logged as it is typed
    const CWidget* widget = routed->source;
    if ( IsSecretInput( widget ) || IsWidgetOfType( widget, "Editbox" ) || IsWidgetOfType( widget, "Textbox" ) )
        return;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI " << GetContextName( notifier ) << ": changed " << DescribeWidget( widget )
                    << ", " << changed->GetPropertyName() << " \"" << changed->newValue << "\"" );
}

/*-------------------------------------------------------------------------*/

void
CGUIUserActionLog::OnItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !CORE::CLogStreamScope::IsUserLogEnabled() )
        return;
    CRoutedEventData* routed = AtFirstSight( eventData );
    CItemEventData* item = CORE::CICloneable::CloneableAs< CItemEventData >( eventData );
    if ( GUCEF_NULL == routed || GUCEF_NULL == item || IsSecretInput( routed->source ) )
        return;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI " << GetContextName( notifier ) << ": chose item " << item->index << " \"" << item->item
                    << "\" of " << DescribeWidget( routed->source ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
