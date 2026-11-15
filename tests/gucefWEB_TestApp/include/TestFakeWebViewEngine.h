/*
 *  gucefWEB_TestApp: tests of the gucefWEB module, its web views
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

#ifndef GUCEF_WEBVIEWTEST_TESTFAKEWEBVIEWENGINE_H
#define GUCEF_WEBVIEWTEST_TESTFAKEWEBVIEWENGINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

#include "TestFakeWebSite.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A web view engine for tests: the whole interface, deterministically, on the fake site of CFakeWebSite. Navigation
 *  events in order, redirects, titles and addresses, the helper expressions of the page queries of a sign-in evaluated
 *  on the page model, cookies per profile, input applied to the page (trusted, unlike script clicks), frames drawn from
 *  the page model with dirty rectangles, popups, downloads, permissions, certificate errors, script dialogs, the
 *  allow-list of the policy. The conformance suite runs against it and against every real engine, and the GUI tests
 *  show its pages in forms.
 */
class CFakeWebViewEngine : public WEB::CWebViewEngineBase
{
    public:

    static const CORE::CString EngineName;

    explicit CFakeWebViewEngine( const CORE::CString& engineName = EngineName );

    virtual ~CFakeWebViewEngine();

    virtual CORE::CString GetEngineVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetAvailabilityDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual WEB::CWebViewCapabilities GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** For tests of the choice of engines */
    void SetAvailable( bool available );

    void SetCapabilities( UInt32 capabilities );

    /** The site; change it before views load pages of it */
    CFakeWebSite& GetSite( void );

    /** How long the page /loop holds the engine thread */
    void SetLoopMs( UInt32 loopMs );

    UInt32 GetLoopMs( void ) const;

    /** Engine thread: the cookies of a profile (kept while the engine lives) */
    WEB::TWebViewCookieVector& GetCookieJar( const CORE::CString& profileId );

    /** Engine thread: a number for the next session cookie */
    UInt32 NextSessionNumber( void );

    UInt32 GetCreatedViewCount( void ) const;

    /** Whether OnEngineThreadStart() ran, and on which thread */
    bool WasEngineThreadStarted( void ) const;

    TThreadId GetStartedThreadId( void ) const;

    /** The capabilities of the fake engine without changes */
    static UInt32 GetDefaultCapabilities( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual WEB::CWebViewBase* CreateViewOnEngineThread( const WEB::CWebViewSettings& settings, UInt32 viewId, CORE::CString& problem ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnEngineThreadStart( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnEngineThreadStop( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CORE::CString, WEB::TWebViewCookieVector > TCookieJarMap;

    CFakeWebSite m_site;
    TCookieJarMap m_cookieJars;
    UInt32 m_sessionNumber;
    MT::CTAtomic< UInt32 > m_available;
    MT::CTAtomic< UInt32 > m_capabilities;
    MT::CTAtomic< UInt32 > m_loopMs;
    MT::CTAtomic< UInt32 > m_createdViews;
    MT::CTAtomic< UInt32 > m_threadStarted;
    MT::CTAtomic< TThreadId > m_threadId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEBVIEWTEST_TESTFAKEWEBVIEWENGINE_H ? */
