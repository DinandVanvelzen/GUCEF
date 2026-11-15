/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_IWEBVIEWENGINE_H
#define GUCEF_WEB_IWEBVIEWENGINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCAPABILITIES_H
#include "gucefWEB_CWebViewCapabilities.h"
#define GUCEF_WEB_CWEBVIEWCAPABILITIES_H
#endif /* GUCEF_WEB_CWEBVIEWCAPABILITIES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWSETTINGS_H
#include "gucefWEB_CWebViewSettings.h"
#define GUCEF_WEB_CWEBVIEWSETTINGS_H
#endif /* GUCEF_WEB_CWEBVIEWSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A web view engine: WebKitGTK, WPE, WebView2, CEF, WKWebView, Null ... An engine is registered by its plugin with
 *  CWebViewGlobal::GetEngineRegistry() when the plugin loads. Registration starts no thread and touches no engine
 *  library: the engine thread starts with the first view (CreateViewAsync()). Engine implementations derive from
 *  CWebViewEngineBase, which provides the engine thread, the command marshalling and the view bookkeeping.
 */
class GUCEF_WEB_PUBLIC_CPP IWebViewEngine : public CORE::CNotifier
{
    public:

    /** The version of this interface; an engine built against another version is refused at registration */
    static const UInt32 InterfaceVersion = 1;

    static const CORE::CEvent EngineStalledEvent;   /**< the engine thread did not answer within the watchdog time; notified on the thread that noticed */

    static void RegisterEvents( void );

    IWebViewEngine( void );

    virtual ~IWebViewEngine();

    /** "WebKitGTK", "WPE", "WebView2", "CEF", "WKWebView", "Null", "ChromeDevTools" */
    virtual const CString& GetEngineName( void ) const = 0;

    /** The engine's own version text, for reports and logs */
    virtual CString GetEngineVersion( void ) const = 0;

    /** Whether the engine can run here: the plugin loaded, but a display server, a runtime or the sandbox may be missing */
    virtual bool IsAvailable( void ) const = 0;

    /** Why the engine is or is not available, for the report of an engine choice */
    virtual CString GetAvailabilityDescription( void ) const = 0;

    virtual CWebViewCapabilities GetCapabilities( void ) const = 0;

    /**
     *  Creates a view on the engine thread. The future completes with a CWebViewCommandResult whose 'view' is the view,
     *  or fails (engine not available, shutting down, the engine refused the settings). Any thread
     */
    virtual CORE::CFutureResult CreateViewAsync( const CWebViewSettings& settings ) = 0;

    /**
     *  Closes every view (their pending commands fail with WEBVIEWERROR_SHUTTING_DOWN), stops the engine thread and waits
     *  for it up to 'timeoutMs'. A thread that does not stop in time is logged and abandoned. The engine can not create
     *  views afterwards. Not on the engine thread
     */
    virtual bool Shutdown( UInt32 timeoutMs ) = 0;

    /** False for engines that cannot be initialised again in the same process (GTK): their plugins stay loaded */
    virtual bool CanUnload( void ) const = 0;

    /** Any thread: true when the engine thread has not answered a command for 'watchdogMs'. The first time it notifies EngineStalledEvent */
    virtual bool CheckWatchdog( UInt32 watchdogMs ) = 0;

    virtual bool IsStalled( void ) const = 0;

    /** Whether the calling thread is the engine thread */
    virtual bool IsEngineThread( void ) const = 0;

    /** Views that are open */
    virtual UInt32 GetViewCount( void ) const = 0;

    /** InterfaceVersion as this engine was built */
    virtual UInt32 GetInterfaceVersion( void ) const;

    private:

    IWebViewEngine( const IWebViewEngine& src );                /**< not implemented */
    IWebViewEngine& operator=( const IWebViewEngine& src );     /**< not implemented */
};

typedef CORE::CTSharedPtr< IWebViewEngine, MT::CMutex > TWebViewEnginePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_IWEBVIEWENGINE_H ? */
