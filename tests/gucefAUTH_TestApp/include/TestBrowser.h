/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTH_TESTAPP_TESTBROWSER_H
#define GUCEF_AUTH_TESTAPP_TESTBROWSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#include "TestPump.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The browser of the tests. It is the handler behind CORE::OpenUrlInSystemBrowser(): no window ever opens. It takes the
 *  address and, on a thread of its own, does what a person and a browser do: asks the provider to authorize and goes to the
 *  address that the provider redirects to, which is the loopback listener of the application.
 */
class CTestBrowser : public GUCEF::CORE::CIBrowserLauncherHandler ,
                     public GUCEF::MT::CActiveObject
{
    public:

    enum EMode
    {
        MODE_FOLLOW = 0,                /**< the valid callback */
        MODE_SEQUENCE,                  /**< a callback with another host, a POST, another path, a wrong state, then the right one, and the right one again */
        MODE_WRONG_STATE_ONLY,          /**< a callback with a wrong state and nothing else */
        MODE_CAPTURE,                   /**< asks for authorization and keeps the address it is redirected to, does not go there */
        MODE_RELAY,                     /**< the provider redirects to a page of a relay: the relay (this browser plays its page) checks the query and goes on to the loopback address */
        MODE_RELAY_WRONG_STATE,         /**< a relay page that changes the state on its way to the loopback address */
        MODE_RELAY_STATE_RENAMED,       /**< a relay page that gives the state under the name state and not s */
        MODE_IGNORE                     /**< does nothing */
    };

    CTestBrowser( void );

    virtual ~CTestBrowser();

    bool Start( void );

    void Stop( void );

    virtual bool OpenUrl( const GUCEF::CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE;

    /** What a person does who got the address some other way: copied from a window of the application */
    void Browse( const GUCEF::CORE::CString& url );

    void SetMode( EMode mode );

    /** What OpenUrl() returns: false makes the launch fail */
    void SetLaunchResult( bool launched );

    GUCEF::UInt32 GetOpenCount( void ) const;

    GUCEF::CORE::CString GetLastUrl( void ) const;

    /** The addresses the provider redirected to (a callback address), in order */
    GUCEF::CORE::CString::StringVector GetRedirects( void ) const;

    /** The addresses the relay page sent the browser on to, in order */
    GUCEF::CORE::CString::StringVector GetRelayed( void ) const;
    /** The status codes of the attempts at the loopback listener, in order; 0 for a request that did not get an answer */
    GUCEF::vector< int > GetStatuses( void ) const;

    /** The scripts that were started are all done */
    bool IsIdle( void ) const;

    void ClearHistory( void );

    /** Installs this browser as the handler of CORE::OpenUrlInSystemBrowser() */
    void Install( void );

    void Uninstall( void );

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void RunScript( const GUCEF::CORE::CString& url, EMode mode );
    int Fetch( const GUCEF::CORE::CString& url, const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& hostHeaderUrl, GUCEF::CORE::CString& location );

    CTestBrowser( const CTestBrowser& src );                /**< not implemented */
    CTestBrowser& operator=( const CTestBrowser& src );     /**< not implemented */

    private:

    mutable GUCEF::MT::CMutex m_lock;
    EMode m_mode;
    bool m_launchResult;
    GUCEF::CORE::CString::StringVector m_queue;
    GUCEF::CORE::CString::StringVector m_urls;
    GUCEF::CORE::CString::StringVector m_redirects;
    GUCEF::CORE::CString::StringVector m_relayed;
    GUCEF::vector< int > m_statuses;
    GUCEF::UInt32 m_running;
    GUCEF::CORE::CIBrowserLauncherHandler* m_previous;
    bool m_installed;
    bool m_started;
    GUCEF::WEB::CHTTPClient m_client;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTBROWSER_H ? */
