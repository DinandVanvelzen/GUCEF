/*
 *  webviewpluginNULL: gucefWEB plugin with the web view engine "Null"
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

#ifndef GUCEF_WEBVIEWPLUGIN_NULL_CNULLWEBVIEW_H
#define GUCEF_WEBVIEWPLUGIN_NULL_CNULLWEBVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_WEB_CWEBVIEWBASE_H
#include "gucefWEB_CWebViewBase.h"
#define GUCEF_WEB_CWEBVIEWBASE_H
#endif /* GUCEF_WEB_CWEBVIEWBASE_H ? */

#ifndef GUCEF_WEBVIEWPLUGIN_NULL_MACROS_H
#include "webviewpluginNULL_macros.h"
#define GUCEF_WEBVIEWPLUGIN_NULL_MACROS_H
#endif /* GUCEF_WEBVIEWPLUGIN_NULL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWPLUGINNULL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A view of the engine "Null": a blank page of its size. It loads about:blank only (anything else fails with
 *  WEBVIEWERROR_NOT_SUPPORTED), takes input and counts it, and has no scripts, cookies, zoom or edit commands
 *  (its capabilities say so). Engine thread only, as every view.
 */
class GUCEF_WEBVIEWPLUGIN_NULL_EXPORT_CPP CNullWebView : public WEB::CWebViewBase
{
    public:

    CNullWebView( WEB::CWebViewEngineBase& engine, UInt32 viewId, const WEB::CWebViewSettings& settings, const WEB::CWebViewCapabilities& capabilities );

    virtual ~CNullWebView();

    /** The number of input events the view took (engine thread, or after the view closed) */
    UInt32 GetInputCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnCreated( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNavigate( const CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnReload( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnStop( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnGoBack( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnGoForward( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnEvaluateScript( const CORE::CString& script, const CORE::CString& world, WEB::CWebViewCommand& command ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnGetCookies( const CORE::CString& urlFilter, WEB::CWebViewCommand& command ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClearCookies( WEB::CWebViewCommand& command ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSetCookie( const WEB::CWebViewCookie& cookie, WEB::CWebViewCommand& command ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSetSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 deviceScale ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSetVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSetFocus( bool focused ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnInjectInput( const WEB::CWebViewInputEvent& inputEvent ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnSetZoom( Float32 factor ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnDecisionResolved( UInt32 decisionId, WEB::TWebViewDecisionKind kind, bool allow, const CORE::CString& text, bool timedOut ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnEditCommand( WEB::TWebViewEditCommand command, CORE::CString& copiedText ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClose( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** Fills the page with its background at the size of the view and publishes all of it */
    void RenderBlank( void );

    CNullWebView( const CNullWebView& src );
    CNullWebView& operator=( const CNullWebView& src );

    private:

    IMAGE::CPixelMap m_page;
    bool m_visible;
    UInt32 m_inputCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWPLUGINNULL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEBVIEWPLUGIN_NULL_CNULLWEBVIEW_H ? */
