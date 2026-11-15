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

#ifndef GUCEF_GUI_CWEBPAGEVIEW_H
#define GUCEF_GUI_CWEBPAGEVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_WEB_IWEBVIEW_H
#include "gucefWEB_IWebView.h"
#define GUCEF_WEB_IWEBVIEW_H
#endif /* GUCEF_WEB_IWEBVIEW_H ? */

#ifndef GUCEF_GUI_CIMAGEFRAME_H
#include "gucefGUI_CImageFrame.h"
#define GUCEF_GUI_CIMAGEFRAME_H
#endif /* GUCEF_GUI_CIMAGEFRAME_H ? */

#ifndef GUCEF_GUI_CWEBVIEWIMAGEBINDING_H
#include "gucefGUI_CWebViewImageBinding.h"
#define GUCEF_GUI_CWEBVIEWIMAGEBINDING_H
#endif /* GUCEF_GUI_CWEBVIEWIMAGEBINDING_H ? */

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
 *  Shows the page of a web view (WEB::IWebView) and gives it the input: the widget type "WebPageView", an image
 *  frame as CVideoView is. The form CWebViewForm puts one in its middle; any layout can hold one.
 *
 *  - Frames: a CWebViewImageBinding puts the newest frame of the view into an image of the widget's own, each frame of
 *    the context; only what changed is copied and uploaded. While a frame of an older size is shown it is stretched.
 *  - Size: after the layout gave the widget a new rectangle the view is told its size once (SetSize()): one size at a
 *    time is in flight, the newest wins. The device scale is the render scale of the context; the view renders at the
 *    density of that scale rounded up and a fraction left is the page zoom.
 *  - Pointer: moves, buttons with their click counts and the wheel go to the view in coordinates of the view (logical
 *    pixels, the widget's top left is 0,0); a press captures the pointer, so drags may leave the widget; a lost capture
 *    sends the release and pointer-leave, leaving the widget sends pointer-leave.
 *  - Keys and text: while the widget has the keyboard focus it takes every key (Tab, arrows, Enter and Escape go to
 *    the page, not to the navigation of gucefGUI) except F6, which the form uses to move between page and chrome.
 *    Text goes as text input. Repeats are those of the driver. Keys, modifiers and buttons are translated to the web
 *    standard input of gucefWEB by CWebViewInputTranslator (physical key, key value of the US layout, DOM numbering).
 *  - Clipboard: engines that do not use the system clipboard themselves (no CAP_NATIVE_CLIPBOARD) get Ctrl+V as text
 *    from CSecretInput::ReadClipboardText(), and Ctrl+C / Ctrl+X as edit commands whose text goes to
 *    CSecretInput::WriteClipboardText(). The page view never puts page content on the clipboard by itself.
 *  - Cursor: the page's pointer shape while the pointer is over the widget, the context's shape again when it leaves.
 *  - Focus: gaining and losing the keyboard focus is told to the view (SetFocus() and focus input).
 *  - Privacy: nothing typed into a page is logged; the user action log treats the widget as a secret input.
 *
 *  GUI thread only; the view's events are not used here (the form relays them), the widget never waits for the view.
 */
class GUCEF_GUI_PUBLIC_CPP CWebPageView : public CImageFrame
{
    public:

    static const CORE::CEvent ViewSizeRequestedEvent;   /**< the view was told a new size; no data */

    static void RegisterEvents( void );

    /** The size the widget asks for: a usable sign-in page */
    static const Int32 MinimumWidth = 320;
    static const Int32 MinimumHeight = 240;
    static const Int32 PreferredWidth = 1024;
    static const Int32 PreferredHeight = 768;

    CWebPageView( void );

    virtual ~CWebPageView();

    /** Shows the view and gives it the input of the widget; a view shown before is let go */
    bool Attach( const WEB::TWebViewPtr& view );

    /** Lets go of the view; the widget shows its background */
    void Detach( void );

    WEB::TWebViewPtr GetView( void ) const;

    bool HasView( void ) const;

    /** Takes the newest frame and tells the view a new size when there is one; the widget does this every frame itself */
    void Refresh( void );

    /** The size the view was told last: logical pixels and the device scale */
    void GetRequestedSize( UInt32& width, UInt32& height, Float32& deviceScale ) const;

    /** How many times the view was told a size */
    UInt32 GetSizeRequestCount( void ) const;

    /** True while the view has not finished the size it was told last; the newest size waits until then */
    bool IsSizeRequestPending( void ) const;

    CWebViewImageBinding& GetBinding( void );

    virtual bool GetNaturalSize( Int32& width, Int32& height ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CWidget* Create( void );

    /** The registered widget type, for CWidget::As() */
    static const char* GetRegisteredTypeName( void ) { return "WebPageView"; }

    protected:

    virtual void OnUpdate( UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool NeedsUpdate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWebPageView > TEventCallback;

    void OnKeyUp( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void SendSizeIfChanged( void );

    void ForwardPointer( const CPointerEventData& pointer, WEB::TWebViewInputKind kind );

    bool HandleClipboardKey( INPUT::KeyCode keyCode, UInt32 modifiers );

    void ApplyCursor( void );

    void RestoreCursor( void );

    void FinishPendingCopy( void );

    CWebPageView( const CWebPageView& src );
    CWebPageView& operator=( const CWebPageView& src );

    private:

    WEB::TWebViewPtr m_view;
    CWebViewImageBinding m_binding;
    UInt32 m_requestedWidth;
    UInt32 m_requestedHeight;
    Float32 m_requestedScale;
    UInt32 m_sizeRequests;
    bool m_sizeInFlight;
    CORE::CFutureResult m_sizeFuture;
    bool m_copyPending;
    CORE::CFutureResult m_copyFuture;
    bool m_hovered;
    bool m_cursorApplied;
    UInt32 m_previousCursor;
    UInt32 m_appliedCursor;
    UInt32 m_buttonsDown;
    Int32 m_lastX;
    Int32 m_lastY;
    UInt32 m_watchdogMs;
    UInt32 m_clockMs;
    UInt32 m_lastDownMs;
    Int32 m_lastDownX;
    Int32 m_lastDownY;
    UInt32 m_clickCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWEBPAGEVIEW_H ? */
