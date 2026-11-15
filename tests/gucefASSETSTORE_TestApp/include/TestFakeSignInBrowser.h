/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTFAKESIGNINBROWSER_H
#define TESTFAKESIGNINBROWSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_WEB_CHTTPCOOKIEJAR_H
#include "gucefWEB_CHttpCookieJar.h"
#define GUCEF_WEB_CHTTPCOOKIEJAR_H
#endif /* GUCEF_WEB_CHTTPCOOKIEJAR_H ? */
#ifndef GUCEF_ASSETSTORE_CIWEBSIGNINBROWSER_H
#include "gucefASSETSTORE_CIWebSignInBrowser.h"
#define GUCEF_ASSETSTORE_CIWEBSIGNINBROWSER_H
#endif /* GUCEF_ASSETSTORE_CIWEBSIGNINBROWSER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A test double of the browser that the DevTools client drives: it loads pages of the fake login site with the
 *  gucefWEB HTTP client (cookies in a jar, redirects followed) and keeps a tiny model of the page: the elements with
 *  their attributes and the form they are in, and the text. Selectors: tag, #id, .class, [attr=value] and their
 *  combinations without spaces; anything else is refused as the page would refuse it. A click on a submit button and
 *  a submit send the form (GET or POST, urlencoded).
 *
 *  The Person...() members are what the owner does in the window, from the thread of the test; every member takes the
 *  lock, so the sign in (on its own thread) and the person can both use it.
 */
class CFakeSignInBrowser : public GUCEF::ASSETSTORE::CIWebSignInBrowser
{
    public:

    CFakeSignInBrowser( void );
    virtual ~CFakeSignInBrowser() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Start( const GUCEF::ASSETSTORE::CWebSignInBrowserSettings& settings, GUCEF::CORE::CString& problem ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Release( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsAlive( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Navigate( const GUCEF::CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetLocation( GUCEF::CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TWebElementState GetElementState( const GUCEF::CORE::CString& selector ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ContainsText( const GUCEF::CORE::CString& text, bool& contains ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool FillField( const GUCEF::CORE::CString& selector, const GUCEF::AUTH::CSecretString& value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Click( const GUCEF::CORE::CString& selector ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Submit( const GUCEF::CORE::CString& selector ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetCookies( GUCEF::ASSETSTORE::TWebSessionCookieVector& cookies ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ClearCookies( void ) GUCEF_VIRTUAL_OVERRIDE;

    // the owner in the window
    bool PersonFill( const GUCEF::CORE::CString& selector, const GUCEF::CORE::CString& value );
    bool PersonClick( const GUCEF::CORE::CString& selector );
    void PersonClosesWindow( void );

    /** A cookie of another site in the profile, which a session must not take */
    void AddCookie( const GUCEF::WEB::CHttpCookie& cookie );

    void SetFailStart( bool failStart );

    GUCEF::UInt32 GetStartCount( void ) const;
    GUCEF::UInt32 GetStopCount( void ) const;
    GUCEF::UInt32 GetReleaseCount( void ) const;
    GUCEF::UInt32 GetFillCount( void ) const;
    GUCEF::UInt32 GetClearCookiesCount( void ) const;
    GUCEF::ASSETSTORE::CWebSignInBrowserSettings GetLastSettings( void ) const;

    private:

    struct SElement
    {
        GUCEF::CORE::CString tag;
        GUCEF::CORE::CString::StringMap attributes;
        GUCEF::Int32 form;              /**< index of the form element it is in, -1 for none */
        GUCEF::CORE::CString value;     /**< what was typed into it */
    };
    typedef GUCEF::vector< SElement > TElementVector;

    bool LoadNoLock( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& url, const GUCEF::CORE::CString& body );
    void ParsePageNoLock( const GUCEF::CORE::CString& html );
    GUCEF::Int32 FindNoLock( const GUCEF::CORE::CString& selector, bool& invalid ) const;
    bool FillNoLock( const GUCEF::CORE::CString& selector, const GUCEF::CORE::CString& value );
    bool ClickNoLock( const GUCEF::CORE::CString& selector );
    bool SubmitFormNoLock( GUCEF::Int32 formIndex );
    GUCEF::CORE::CString ResolveNoLock( const GUCEF::CORE::CString& reference ) const;

    CFakeSignInBrowser( const CFakeSignInBrowser& src );              /**< not implemented */
    CFakeSignInBrowser& operator=( const CFakeSignInBrowser& src );   /**< not implemented */

    private:

    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::WEB::CHttpCookieJarPtr m_jar;
    GUCEF::CORE::CString m_location;
    TElementVector m_elements;
    GUCEF::CORE::CString m_text;
    bool m_running;
    bool m_windowClosed;
    bool m_failStart;
    GUCEF::UInt32 m_startCount;
    GUCEF::UInt32 m_stopCount;
    GUCEF::UInt32 m_releaseCount;
    GUCEF::UInt32 m_fillCount;
    GUCEF::UInt32 m_clearCookiesCount;
    GUCEF::ASSETSTORE::CWebSignInBrowserSettings m_lastSettings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTFAKESIGNINBROWSER_H ? */
