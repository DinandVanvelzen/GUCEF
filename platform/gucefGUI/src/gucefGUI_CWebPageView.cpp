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

#include <math.h>

#ifndef GUCEF_WEB_CWEBVIEWGLOBAL_H
#include "gucefWEB_CWebViewGlobal.h"
#define GUCEF_WEB_CWEBVIEWGLOBAL_H
#endif /* GUCEF_WEB_CWEBVIEWGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#include "gucefWEB_CWebViewCommandResult.h"
#define GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#endif /* GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

#ifndef GUCEF_GUI_CWEBVIEWINPUTTRANSLATOR_H
#include "gucefGUI_CWebViewInputTranslator.h"
#define GUCEF_GUI_CWEBVIEWINPUTTRANSLATOR_H
#endif /* GUCEF_GUI_CWEBVIEWINPUTTRANSLATOR_H ? */

#include "gucefGUI_CWebPageView.h"

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

const CORE::CEvent CWebPageView::ViewSizeRequestedEvent = "GUCEF::GUI::CWebPageView::ViewSizeRequestedEvent";
const Int32 CWebPageView::MinimumWidth;
const Int32 CWebPageView::MinimumHeight;
const Int32 CWebPageView::PreferredWidth;
const Int32 CWebPageView::PreferredHeight;

namespace {

/** Clicks of a double click: close in time and place */
const UInt32 MultiClickTimeMs = 500;
const Int32 MultiClickDistance = 4;

/** How often the page view runs the watchdog of the engines */
const UInt32 WatchdogIntervalMs = 1000;

bool
IsClipboardKey( INPUT::KeyCode keyCode )
{
    return INPUT::KEYCODE_V == keyCode || INPUT::KEYCODE_C == keyCode || INPUT::KEYCODE_X == keyCode || INPUT::KEYCODE_A == keyCode;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWebPageView::RegisterEvents( void )
{GUCEF_TRACE;

    ViewSizeRequestedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWebPageView::CWebPageView( void )
    : CImageFrame( "WebPageView" )
    , m_view()
    , m_binding()
    , m_requestedWidth( 0 )
    , m_requestedHeight( 0 )
    , m_requestedScale( 0.0f )
    , m_sizeRequests( 0 )
    , m_sizeInFlight( false )
    , m_sizeFuture( CORE::TASKSTATUS_FINISHED )
    , m_copyPending( false )
    , m_copyFuture( CORE::TASKSTATUS_FINISHED )
    , m_hovered( false )
    , m_cursorApplied( false )
    , m_previousCursor( CURSORSHAPE_ARROW )
    , m_appliedCursor( CURSORSHAPE_ARROW )
    , m_buttonsDown( 0 )
    , m_lastX( 0 )
    , m_lastY( 0 )
    , m_watchdogMs( 0 )
    , m_clockMs( 0 )
    , m_lastDownMs( 0 )
    , m_lastDownX( 0 )
    , m_lastDownY( 0 )
    , m_clickCount( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    // a key up has no default action: the page view hears its own
    TEventCallback keyUpCallback( this, &CWebPageView::OnKeyUp );
    SubscribeTo( this, CWidget::KeyUpEvent, keyUpCallback );
}

/*-------------------------------------------------------------------------*/

CWebPageView::~CWebPageView()
{GUCEF_TRACE;

    Detach();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

CWidget*
CWebPageView::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CWebPageView();
}

/*-------------------------------------------------------------------------*/

bool
CWebPageView::Attach( const WEB::TWebViewPtr& view )
{GUCEF_TRACE;

    Detach();
    CGUIContext* context = GetContext();
    if ( view.IsNULL() || GUCEF_NULL == context )
        return false;
    m_view = view;
    m_requestedWidth = m_requestedHeight = 0;
    m_requestedScale = 0.0f;
    m_sizeInFlight = false;
    const CString& alias = GetOwnImageName();
    if ( !m_binding.Bind( context, alias, view, false ) )
    {
        m_view.Unlink();
        return false;
    }
    SetImageName( alias );
    if ( HasFocus() )
        m_view->SetFocus( true );
    SendSizeIfChanged();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::Detach( void )
{GUCEF_TRACE;

    if ( m_view.IsNULL() )
        return;
    RestoreCursor();
    if ( 0 != m_buttonsDown || m_hovered )
        m_view->InjectInput( WEB::CWebViewInputEvent::PointerLeave() );
    m_buttonsDown = 0;
    m_hovered = false;
    m_binding.Unbind();
    m_view.Unlink();
    m_copyPending = false;
    m_sizeInFlight = false;
}

/*-------------------------------------------------------------------------*/

WEB::TWebViewPtr
CWebPageView::GetView( void ) const
{GUCEF_TRACE;

    return m_view;
}

/*-------------------------------------------------------------------------*/

bool
CWebPageView::HasView( void ) const
{GUCEF_TRACE;

    return !m_view.IsNULL();
}

/*-------------------------------------------------------------------------*/

CWebViewImageBinding&
CWebPageView::GetBinding( void )
{GUCEF_TRACE;

    return m_binding;
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::GetRequestedSize( UInt32& width, UInt32& height, Float32& deviceScale ) const
{GUCEF_TRACE;

    width = m_requestedWidth;
    height = m_requestedHeight;
    deviceScale = m_requestedScale;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebPageView::GetSizeRequestCount( void ) const
{GUCEF_TRACE;

    return m_sizeRequests;
}

/*-------------------------------------------------------------------------*/

bool
CWebPageView::IsSizeRequestPending( void ) const
{GUCEF_TRACE;

    return m_sizeInFlight && !m_sizeFuture.IsReady();
}

/*-------------------------------------------------------------------------*/

bool
CWebPageView::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    width = PreferredWidth;
    height = PreferredHeight;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebPageView::NeedsUpdate( void ) const
{GUCEF_TRACE;

    return !m_view.IsNULL() || CImageFrame::NeedsUpdate();
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::Refresh( void )
{GUCEF_TRACE;

    if ( m_view.IsNULL() )
        return;
    FinishPendingCopy();
    SendSizeIfChanged();
    m_binding.Update();
    ApplyCursor();
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::OnUpdate( UInt32 deltaInMs )
{GUCEF_TRACE;

    CImageFrame::OnUpdate( deltaInMs );
    m_clockMs += deltaInMs;
    if ( m_view.IsNULL() )
        return;
    Refresh();
    m_watchdogMs += deltaInMs;
    if ( m_watchdogMs >= WatchdogIntervalMs )
    {
        m_watchdogMs = 0;
        WEB::CWebViewGlobal::Instance()->CheckEngineWatchdogs();
    }
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::SendSizeIfChanged( void )
{GUCEF_TRACE;

    const CRect& rect = GetRect();
    CGUIContext* context = GetContext();
    if ( m_view.IsNULL() || GUCEF_NULL == context || rect.width <= 0 || rect.height <= 0 )
        return;

    // one size at a time: the newest is sent when the one in flight is done
    if ( m_sizeInFlight )
    {
        if ( !m_sizeFuture.IsReady() )
            return;
        m_sizeInFlight = false;
    }

    const Float32 scale = (Float32) ( 0 == context->GetRenderScale() ? 1 : context->GetRenderScale() );
    Float32 density = (Float32) ceil( scale - 0.001f );
    if ( density < 1.0f )
        density = 1.0f;
    if ( (UInt32) rect.width == m_requestedWidth && (UInt32) rect.height == m_requestedHeight && density == m_requestedScale )
        return;

    m_requestedWidth = (UInt32) rect.width;
    m_requestedHeight = (UInt32) rect.height;
    m_requestedScale = density;
    m_sizeFuture = m_view->SetSize( m_requestedWidth, m_requestedHeight, density );
    m_sizeInFlight = true;
    ++m_sizeRequests;
    // a scale the integer density does not cover is the page zoom
    if ( scale != density )
        m_view->SetZoom( scale / density );
    NotifyObservers( ViewSizeRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::ForwardPointer( const CPointerEventData& pointer, WEB::TWebViewInputKind kind )
{GUCEF_TRACE;

    const CRect& rect = GetRect();
    m_lastX = pointer.x - rect.left;
    m_lastY = pointer.y - rect.top;
    WEB::CWebViewInputEvent input;
    input.kind = kind;
    input.x = m_lastX;
    input.y = m_lastY;
    input.modifiers = CWebViewInputTranslator::GetWebModifiers( pointer.modifiers );
    input.buttonStates = CWebViewInputTranslator::GetWebMouseButtonStates( pointer.buttonStates );
    input.button = CWebViewInputTranslator::GetWebMouseButton( pointer.button );
    if ( WEB::WEBVIEWINPUT_POINTER_DOWN == kind )
    {
        // a second press soon after and close to the first is a double click
        const Int32 dx = m_lastX - m_lastDownX, dy = m_lastY - m_lastDownY;
        if ( m_clickCount > 0 && m_clockMs - m_lastDownMs <= MultiClickTimeMs && dx >= -MultiClickDistance && dx <= MultiClickDistance &&
             dy >= -MultiClickDistance && dy <= MultiClickDistance )
            m_clickCount = m_clickCount >= 3 ? 1 : m_clickCount + 1;
        else
            m_clickCount = 1;
        m_lastDownMs = m_clockMs;
        m_lastDownX = m_lastX;
        m_lastDownY = m_lastY;
    }
    input.clickCount = WEB::WEBVIEWINPUT_POINTER_MOVE == kind ? 0 : m_clickCount;
    m_view->InjectInput( input );
}

/*-------------------------------------------------------------------------*/

bool
CWebPageView::HandleClipboardKey( INPUT::KeyCode keyCode, UInt32 modifiers )
{GUCEF_TRACE;

    if ( m_view.IsNULL() || m_view->GetCapabilities().Has( WEB::CWebViewCapabilities::CAP_NATIVE_CLIPBOARD ) )
        return false;
    if ( 0 == ( modifiers & INPUT::KEYMOD_CTRL ) || 0 != ( modifiers & INPUT::KEYMOD_ALT ) || !IsClipboardKey( keyCode ) )
        return false;
    switch ( keyCode )
    {
        case INPUT::KEYCODE_V:
        {
            CString text;
            if ( CSecretInput::ReadClipboardText( text ) && !text.IsNULLOrEmpty() )
                m_view->InjectInput( WEB::CWebViewInputEvent::Text( text ) );
            return true;
        }
        case INPUT::KEYCODE_C:
        case INPUT::KEYCODE_X:
        {
            m_copyFuture = m_view->ExecuteEditCommand( INPUT::KEYCODE_C == keyCode ? WEB::WEBVIEWEDIT_COPY : WEB::WEBVIEWEDIT_CUT );
            m_copyPending = true;
            return true;
        }
        default:
        {
            m_view->ExecuteEditCommand( WEB::WEBVIEWEDIT_SELECTALL );
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::FinishPendingCopy( void )
{GUCEF_TRACE;

    if ( !m_copyPending || !m_copyFuture.IsReady() )
        return;
    m_copyPending = false;
    WEB::CWebViewCommandResult result;
    if ( WEB::CWebViewCommandResult::TryGetFromFuture( m_copyFuture, result ) && result.IsSuccess() && !result.text.IsNULLOrEmpty() )
        CSecretInput::WriteClipboardText( result.text );
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::ApplyCursor( void )
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( !m_hovered || m_view.IsNULL() || GUCEF_NULL == context )
        return;
    const UInt32 shape = (UInt32) m_view->GetCursorShape();
    if ( !m_cursorApplied )
    {
        m_previousCursor = (UInt32) context->GetSystemCursorShape();
        m_cursorApplied = true;
        m_appliedCursor = shape;
        context->SetSystemCursorShape( (TCursorShape) shape );
        return;
    }
    if ( shape != m_appliedCursor )
    {
        m_appliedCursor = shape;
        context->SetSystemCursorShape( (TCursorShape) shape );
    }
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::RestoreCursor( void )
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( m_cursorApplied && GUCEF_NULL != context )
        context->SetSystemCursorShape( (TCursorShape) m_previousCursor );
    m_cursorApplied = false;
}

/*-------------------------------------------------------------------------*/

bool
CWebPageView::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( m_view.IsNULL() )
        return CImageFrame::OnDefaultAction( eventId, eventData );

    if ( MouseMoveEvent == eventId || MouseDownEvent == eventId || MouseUpEvent == eventId )
    {
        CPointerEventData* pointer = CORE::CICloneable::CloneableAs< CPointerEventData >( &eventData );
        if ( GUCEF_NULL == pointer )
            return false;
        if ( MouseMoveEvent == eventId )
        {
            ForwardPointer( *pointer, WEB::WEBVIEWINPUT_POINTER_MOVE );
        }
        else
        if ( MouseDownEvent == eventId )
        {
            m_buttonsDown |= 1u << (UInt32) pointer->button;
            ForwardPointer( *pointer, WEB::WEBVIEWINPUT_POINTER_DOWN );
        }
        else
        {
            m_buttonsDown &= ~( 1u << (UInt32) pointer->button );
            ForwardPointer( *pointer, WEB::WEBVIEWINPUT_POINTER_UP );
        }
        return true;
    }
    if ( MouseWheelEvent == eventId )
    {
        CWheelEventData* wheel = CORE::CICloneable::CloneableAs< CWheelEventData >( &eventData );
        if ( GUCEF_NULL == wheel )
            return false;
        const CRect& rect = GetRect();
        m_view->InjectInput( WEB::CWebViewInputEvent::Wheel( wheel->x - rect.left, wheel->y - rect.top, 0.0f, (Float32) wheel->delta, CWebViewInputTranslator::GetWebModifiers( wheel->modifiers ) ) );
        return true;
    }
    if ( MouseFocusReceivedEvent == eventId )
    {
        m_hovered = true;
        ApplyCursor();
        return false;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        m_hovered = false;
        if ( 0 == m_buttonsDown )
            m_view->InjectInput( WEB::CWebViewInputEvent::PointerLeave() );
        RestoreCursor();
        return false;
    }
    if ( MouseCaptureLostEvent == eventId )
    {
        // the press ended without a release: the page gets the releases, and the pointer leaves
        for ( UInt32 button=1; button<=5; ++button )
            if ( 0 != ( m_buttonsDown & ( 1u << button ) ) )
                m_view->InjectInput( WEB::CWebViewInputEvent::PointerButton( m_lastX, m_lastY, CWebViewInputTranslator::GetWebMouseButton( (TMouseButton) button ), false, 1 ) );
        m_buttonsDown = 0;
        m_view->InjectInput( WEB::CWebViewInputEvent::PointerLeave() );
        return false;
    }
    if ( KeyDownEvent == eventId )
    {
        CKeyEventData* key = CORE::CICloneable::CloneableAs< CKeyEventData >( &eventData );
        if ( GUCEF_NULL == key || INPUT::KEYCODE_F6 == key->keyCode )
            return false;
        if ( HandleClipboardKey( key->keyCode, key->modifiers ) )
            return true;
        m_view->InjectInput( CWebViewInputTranslator::KeyEvent( key->keyCode, true, key->modifiers ) );
        return true;
    }
    if ( TextInputEvent == eventId )
    {
        CTextEventData* text = CORE::CICloneable::CloneableAs< CTextEventData >( &eventData );
        if ( GUCEF_NULL != text && !text->text.IsNULLOrEmpty() )
            m_view->InjectInput( WEB::CWebViewInputEvent::Text( text->text ) );
        return true;
    }
    if ( KeyboardFocusReceivedEvent == eventId || KeyboardFocusLostEvent == eventId )
    {
        const bool focused = KeyboardFocusReceivedEvent == eventId;
        m_view->SetFocus( focused );
        m_view->InjectInput( WEB::CWebViewInputEvent::Focus( focused ) );
        return false;
    }
    return CImageFrame::OnDefaultAction( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebPageView::OnKeyUp( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = CORE::CICloneable::CloneableAs< CKeyEventData >( eventData );
    if ( m_view.IsNULL() || GUCEF_NULL == key || key->source != this || INPUT::KEYCODE_F6 == key->keyCode )
        return;
    key->handled = true;
    // the release of a clipboard shortcut the page view carried out itself is not the page's
    const bool interceptedShortcut = !m_view->GetCapabilities().Has( WEB::CWebViewCapabilities::CAP_NATIVE_CLIPBOARD ) &&
                                     0 != ( key->modifiers & INPUT::KEYMOD_CTRL ) && 0 == ( key->modifiers & INPUT::KEYMOD_ALT ) && IsClipboardKey( key->keyCode );
    if ( !interceptedShortcut )
        m_view->InjectInput( CWebViewInputTranslator::KeyEvent( key->keyCode, false, key->modifiers ) );
}

/*-------------------------------------------------------------------------*/

const CString&
CWebPageView::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CWebPageView";
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
