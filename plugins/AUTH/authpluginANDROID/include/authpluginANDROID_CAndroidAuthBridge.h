/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_ANDROIDSHIM_H
#include "authpluginANDROID_AndroidShim.h"
#define GUCEF_AUTHPLUGIN_ANDROID_ANDROIDSHIM_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_ANDROIDSHIM_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDREDIRECTRECEIVER_H
#include "authpluginANDROID_CAndroidRedirectReceiver.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDREDIRECTRECEIVER_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDREDIRECTRECEIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Takes the end of a request from the bridge. Called on the thread that delivers (a Java thread), never with a lock of the bridge held */
class GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP IAndroidAuthResultSink
{
    public:

    IAndroidAuthResultSink( void );

    virtual ~IAndroidAuthResultSink();

    virtual void OnAndroidAuthResult( const SAndroidAuthResult& result ) = 0;

    private:

    IAndroidAuthResultSink( const IAndroidAuthResultSink& src );
    IAndroidAuthResultSink& operator=( const IAndroidAuthResultSink& src );
};

/*-------------------------------------------------------------------------*/

/**
 *  The meeting point of the C++ providers and the Java side. It holds the table of functions that start things in
 *  Java (filled by the JNI code when Android is there, by a test otherwise), the requests that wait for an answer,
 *  and the receiver of redirects. Everything on it is thread safe: answers and redirects arrive on Java threads.
 *
 *  A request is made by a provider with BeginRequest(), which gives the id that goes to Java. The answer comes with
 *  DeliverResult() and is handed to the provider that made the request; an answer for a request that is not
 *  waited for any more (cancelled, provider gone) is dropped. A provider that goes away calls ReleaseSink() first,
 *  which waits for a delivery to it that is under way.
 *
 *  The browser launcher: InstallBrowserLauncher() makes CORE::OpenUrlInSystemBrowser() open Custom Tabs through
 *  the table (openCustomTab), which is what the OAuth/OIDC engine uses to open its authorization url.
 */
class GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP CAndroidAuthBridge
{
    public:

    static CAndroidAuthBridge* Instance( void );

    /** Installs the table of functions (the JNI code does when the Java side initializes, tests with a fake). An empty table removes it */
    void SetShim( const TAndroidShim& shim );

    /** A copy of the table; empty when none is installed */
    TAndroidShim GetShim( void ) const;

    /** Whether a table is installed: the Java side is there */
    bool HasShim( void ) const;

    /** Registers a request of the sink and gives its id (never 0) */
    UInt32 BeginRequest( IAndroidAuthResultSink* sink );

    /** Forgets a request */
    void EndRequest( UInt32 requestId );

    /** Forgets all requests of the sink and waits for a delivery to it that is under way. Call it before the sink is destroyed, without a lock the sink's OnAndroidAuthResult() takes */
    void ReleaseSink( IAndroidAuthResultSink* sink );

    /** The number of requests that wait for an answer */
    UInt32 GetRequestCount( void ) const;

    /** Hands an answer to the sink of the request, which is forgotten. False when nobody waits for the request */
    bool DeliverResult( const SAndroidAuthResult& result );

    /** Sets the receiver of redirect addresses, not owned; returns the one that was set before */
    CAndroidRedirectReceiver* SetRedirectReceiver( CAndroidRedirectReceiver* receiver );

    /** Hands a redirect address to the receiver. False when there is none or it did not take the address */
    bool DeliverRedirect( const CORE::CString& redirectUrl );

    /** Makes CORE::OpenUrlInSystemBrowser() use Custom Tabs through the table. False when it is installed already */
    bool InstallBrowserLauncher( void );

    /** Puts back the launcher that was there before. Only when the plugin's launcher is the installed one */
    void RemoveBrowserLauncher( void );

    private:

    CAndroidAuthBridge( void );
    ~CAndroidAuthBridge();
    CAndroidAuthBridge( const CAndroidAuthBridge& src );
    CAndroidAuthBridge& operator=( const CAndroidAuthBridge& src );

    class CLauncher : public CORE::CIBrowserLauncherHandler
    {
        public:
        CLauncher( void );
        virtual ~CLauncher();
        virtual bool OpenUrl( const CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE;
    };

    typedef GUCEF::map< UInt32, IAndroidAuthResultSink* > TRequestMap;

    private:

    mutable MT::CMutex m_lock;
    TAndroidShim m_shim;
    TRequestMap m_requests;
    UInt32 m_nextRequestId;
    UInt32 m_deliveriesInFlight;
    CAndroidRedirectReceiver* m_redirectReceiver;
    CLauncher m_launcher;
    CORE::CIBrowserLauncherHandler* m_previousLauncher;
    bool m_launcherInstalled;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H ? */
