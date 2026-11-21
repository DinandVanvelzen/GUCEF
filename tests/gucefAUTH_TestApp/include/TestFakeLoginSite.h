/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_TESTAPP_TESTFAKELOGINSITE_H
#define GUCEF_AUTH_TESTAPP_TESTFAKELOGINSITE_H

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
#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */
#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace AUTHTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the fake login site was asked */
class CFakeLoginSiteStats
{
    public:

    GUCEF::UInt32 loginPages;
    GUCEF::UInt32 loginPosts;
    GUCEF::UInt32 wrongPasswords;
    GUCEF::UInt32 goodPasswords;
    GUCEF::UInt32 challengePages;
    GUCEF::UInt32 codePages;
    GUCEF::UInt32 sessionsIssued;
    GUCEF::UInt32 apiAnswered;
    GUCEF::UInt32 apiRefused;

    CFakeLoginSiteStats( void );
};

/**
 *  A web site with a login, in the test: an HTTP server on 127.0.0.1 and an ephemeral port, on a thread and a pulse
 *  generator of its own. Pages are plain HTML forms without scripts:
 *  - /login: a form (#login, input[name=email], input[name=password], button#sign-in); a wrong password shows
 *    p#error.login-error; a good one goes on to the challenge (when on), the code page (when on) or signs in
 *  - /challenge: a puzzle a person solves (div#captcha, input#answer "person", button#solve); the page reloads itself
 *    every second, and ApproveChallenge() lets the next reload pass, as a person solving it in another way would
 *  - /verify: a code sent by e-mail (input#code, button#verify)
 *  - signing in sets the HttpOnly cookie "sid" and goes to /account (div#account); /api/library answers JSON with a
 *    valid "sid" and 401 without; ExpireSessions() ends every session
 *  - /elsewhere redirects to the login page under the host name "localhost" (another host than 127.0.0.1)
 */
class CFakeLoginSite : public GUCEF::MT::CActiveObject
{
    public:

    CFakeLoginSite( void );
    virtual ~CFakeLoginSite();

    bool Start( void );
    void Stop( void );

    /** http://127.0.0.1:<port> */
    GUCEF::CORE::CString GetBaseUrl( void ) const;
    GUCEF::UInt16 GetPort( void ) const;

    void SetAccount( const GUCEF::CORE::CString& email, const GUCEF::CORE::CString& password );
    void SetChallenge( bool challenge );
    void SetEmailCode( const GUCEF::CORE::CString& code );
    void ApproveChallenge( void );
    void ExpireSessions( void );

    CFakeLoginSiteStats GetStats( void ) const;

    /** Every session cookie value the site issued, for the checks that they appear in no log and no file */
    GUCEF::CORE::CString::StringVector GetIssuedSessionValues( void ) const;

    void HandleRequest( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void SendHtml( GUCEF::WEB::CHttpResponseData& response, int status, const GUCEF::CORE::CString& title, const GUCEF::CORE::CString& body, bool refresh );
    void Redirect( GUCEF::WEB::CHttpResponseData& response, const GUCEF::CORE::CString& path );
    void IssueSession( GUCEF::WEB::CHttpResponseData& response );
    bool HasValidSession( const GUCEF::WEB::CHttpRequestData& request ) const;
    bool HasPending( const GUCEF::WEB::CHttpRequestData& request ) const;
    GUCEF::CORE::CString LoginPage( bool wrongPassword ) const;

    CFakeLoginSite( const CFakeLoginSite& src );              /**< not implemented */
    CFakeLoginSite& operator=( const CFakeLoginSite& src );   /**< not implemented */

    private:

    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::WEB::CHTTPServer* m_server;
    void* m_factory;
    GUCEF::UInt16 m_port;
    bool m_running;
    GUCEF::CORE::CString m_email;
    GUCEF::CORE::CString m_password;
    bool m_challenge;
    bool m_challengeApproved;
    GUCEF::CORE::CString m_emailCode;
    GUCEF::CORE::CString::StringVector m_validSessions;
    GUCEF::CORE::CString::StringVector m_issuedSessions;
    GUCEF::CORE::CString m_pending;
    GUCEF::UInt32 m_counter;
    CFakeLoginSiteStats m_stats;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTFAKELOGINSITE_H ? */
