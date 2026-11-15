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

#ifndef GUCEF_GUI_CWEBVIEWIMAGEBINDING_H
#define GUCEF_GUI_CWEBVIEWIMAGEBINDING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_WEB_IWEBVIEW_H
#include "gucefWEB_IWebView.h"
#define GUCEF_WEB_IWEBVIEW_H
#endif /* GUCEF_WEB_IWEBVIEW_H ? */

#ifndef GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H
#include "gucefWEB_CWebViewFrameExchange.h"
#define GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H
#endif /* GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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

class CGUIContext;

/**
 *  Puts the frames of a web view into a GUI context as a named image, without a widget: an ImageFrame, a DrawSurface
 *  command or (through a driver's texture binding) a material then shows the page wherever an image goes. A page view
 *  (CWebPageView) uses one for itself.
 *
 *  The binding is the one reader of the frame exchange of the view. It keeps a pixel map of its own that is registered
 *  with the context, copies only what changed into it and tells the context which part changed
 *  (CGUIContext::UpdateImageRegion()), so drivers that can upload a part of a texture upload that part. Nothing is
 *  allocated per frame, only when the size of the frames changes. The image is registered with a density of the device
 *  scale of the frames rounded up, so its logical size is the logical size of the view.
 *
 *  GUI thread only. With automatic updates it takes the newest frame at every FrameStartedEvent of the context; the
 *  binding is let go (Unbind()) before the context goes. Input is the application's: it calls
 *  IWebView::InjectInput() with coordinates it maps from its own hit test.
 */
class GUCEF_GUI_PUBLIC_CPP CWebViewImageBinding : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ImageUpdatedEvent;        /**< the image got a new frame; TRegionEventData: the part that changed, in pixels of the image */

    static void RegisterEvents( void );

    typedef CORE::CTCloneableObj< CRect > TRegionEventData;

    CWebViewImageBinding( void );

    virtual ~CWebViewImageBinding();

    /** Binds the frames of the view to an image alias of the context; a binding made before is let go first */
    bool Bind( CGUIContext* context, const CString& imageAlias, const WEB::TWebViewPtr& view, bool autoUpdate = true );

    /** Lets go of the view and unregisters the image */
    void Unbind( void );

    /** Takes the newest frame into the image; true when the image changed. Called by itself with automatic updates */
    bool Update( void );

    bool IsBound( void ) const;

    WEB::TWebViewPtr GetView( void ) const;

    const CString& GetImageAlias( void ) const;

    /** Whether a frame was put in the image */
    bool HasImage( void ) const;

    /** The size of the image in device pixels and its density */
    void GetImageSize( UInt32& width, UInt32& height, UInt32& density ) const;

    /** What the frame put in last was */
    const WEB::CWebViewFrameInfo& GetFrameInfo( void ) const;

    /** Frames put in the image, the pixels copied for them, and the registrations (one per size) */
    UInt32 GetFrameCount( void ) const;
    UInt64 GetCopiedPixelCount( void ) const;
    UInt32 GetRegistrationCount( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWebViewImageBinding > TEventCallback;

    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CWebViewImageBinding( const CWebViewImageBinding& src );
    CWebViewImageBinding& operator=( const CWebViewImageBinding& src );

    private:

    CGUIContext* m_context;
    CString m_alias;
    WEB::TWebViewPtr m_view;
    IMAGE::TPixelMapPtr m_pixels;
    UInt32 m_density;
    bool m_hasImage;
    bool m_autoUpdate;
    WEB::CWebViewFrameInfo m_info;
    UInt32 m_frames;
    UInt64 m_copiedPixels;
    UInt32 m_registrations;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWEBVIEWIMAGEBINDING_H ? */
