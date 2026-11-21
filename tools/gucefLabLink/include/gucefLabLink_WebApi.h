/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_WEBAPI_H
#define GUCEF_LABLINK_WEBAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CHTTPREQUESTDATA_H
#include "gucefWEB_CHttpRequestData.h"
#define GUCEF_WEB_CHTTPREQUESTDATA_H
#endif /* GUCEF_WEB_CHTTPREQUESTDATA_H ? */

#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_HubView.h"
#include "gucefLabLink_WebSettings.h"
#include "gucefLabLink_WebUsers.h"
#include "gucefLabLink_WebSessions.h"
#include "gucefLabLink_LoginThrottle.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The header every POST of the website carries: a page of another site cannot send it without the consent of this one */
extern const char* WebCsrfHeader;           /**< "X-LabLink-Csrf", value "1" */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A password that has to be hashed before a request can be answered: a login, or a user an admin adds. It is done
 *  by CLabWebApi::RunPasswordJob(), on a thread of a pool in the website, and then given back to
 *  CLabWebApi::CompletePasswordJob() on the thread of the website.
 */
struct SPasswordJob
{
    enum EKind
    {
        JOB_LOGIN = 0,
        JOB_ADD_USER
    };

    EKind kind;
    GUCEF::CORE::CString user;                  /**< login: the name given; add: the new user */
    GUCEF::CORE::CString role;                  /**< add */
    GUCEF::CORE::CString password;              /**< wiped once the job ran */
    GUCEF::CORE::CString record;                /**< login: the record to verify against, a dummy one for a user that is not there */
    bool knownUser;                             /**< login: the user is there and enabled */
    bool persistent;                            /**< login: stay signed in */
    GUCEF::CORE::CString peer;                  /**< the address of the client */
    GUCEF::CORE::CString actor;                 /**< add: the admin */
    SPasswordParams params;                     /**< what new records are made with */

    bool verified;                              /**< login: the password is that of the record; add: the record was made */
    GUCEF::CORE::CString newRecord;             /**< login: the record with the current parameters when the old ones differ; add: the record */

    SPasswordJob( void )
        : kind( JOB_LOGIN ), user(), role(), password(), record(), knownUser( false ), persistent( false ), peer(), actor(), params()
        , verified( false ), newRecord()
    {}
};

/**
 *  The website of the hub without its transport: what a request is answered with. CLabWebSite gives it the requests
 *  of its HTTP server; the tests give it requests of their own. Every function runs on the thread of the website,
 *  except RunPasswordJob(), and the times are passed in as monotonic milliseconds.
 *
 *  Who may do what: the static files of the page are served to everyone in the allowed ranges, everything under
 *  /api/v1 except the login needs a session (cookie) or the bearer token; the users of the site are for admins
 *  (the token counts as admin). Every POST needs the header WebCsrfHeader. On plain HTTP to a loopback address the
 *  Host has to name this machine (a page that rebinds its DNS name to 127.0.0.1 still names its own host there).
 */
class CLabWebApi
{
    public:

    /**
     *  @param tls   whether the site is served over TLS: the cookie is Secure then
     *  @param token the bearer token, empty when there is none
     */
    CLabWebApi( const SWebSettings& settings, const CAddressRanges& allowedRanges, ILabHubView& hub, CWebUserStore& users,
                bool tls, const GUCEF::CORE::CString& token );

    /** The port and address the site is reached on, for the check of the Host on plain HTTP */
    void SetListening( const GUCEF::CORE::CString& address, GUCEF::UInt16 port );

    /**
     *  The checks that can be made before the body of a request is read: the address of the client, the Host, the
     *  path and method, the CSRF header, the content type, the wait of a client that failed to log in, the
     *  credentials. @param peer the address of the client, NULL when the transport does not tell
     *  @return true to go on; false: the response is the refusal
     */
    bool CheckHead( const GUCEF::WEB::CHttpRequestData& request, const GUCEF::CORE::CString* peer, GUCEF::UInt64 nowInMs,
                    GUCEF::WEB::CHttpResponseData& response );

    /**
     *  Answers a request whose body is in; CheckHead() is part of it.
     *  @return true: the response is the answer; false: the job has to run first, then CompletePasswordJob() answers
     */
    bool Handle( const GUCEF::WEB::CHttpRequestData& request, const GUCEF::CORE::CString* peer, GUCEF::UInt64 nowInMs,
                 GUCEF::WEB::CHttpResponseData& response, SPasswordJob& job );

    /** The hashing of a job; on any thread. Wipes the password */
    static void RunPasswordJob( SPasswordJob& job );

    /** Answers the request of a job that ran */
    void CompletePasswordJob( SPasswordJob& job, GUCEF::UInt64 nowInMs, GUCEF::WEB::CHttpResponseData& response );

    /** The answer when a job cannot be run now: too many are waiting */
    void RefusePasswordJob( SPasswordJob& job, GUCEF::WEB::CHttpResponseData& response );

    /** The sweep of ended sessions and of quiet throttle entries, once a minute; call about once a second */
    void OnTimer( GUCEF::UInt64 nowInMs );

    /** The name of the session cookie: with the __Host- prefix over TLS */
    GUCEF::CORE::CString GetCookieName( void ) const;

    /** A Set-Cookie value for the session id; maxAgeInSecs 0: a cookie that ends with the browser; clear: one that ends it */
    static GUCEF::CORE::CString MakeCookie( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& id, bool tls,
                                            GUCEF::UInt64 maxAgeInSecs, bool clear );

    /** The fields of an application/x-www-form-urlencoded body */
    static void ParseForm( const GUCEF::CORE::CString& body, GUCEF::CORE::CString::StringMap& fields );

    bool IsTls( void ) const;
    CWebSessionTable& GetSessions( void );
    CLoginThrottle& GetAddressThrottle( void );
    CLoginThrottle& GetUserThrottle( void );
    const SWebSettings& GetSettings( void ) const;

    private:

    enum ERoute
    {
        ROUTE_NONE = 0,
        ROUTE_PAGE,
        ROUTE_SCRIPT,
        ROUTE_STYLE,
        ROUTE_LOGIN,
        ROUTE_LOGOUT,
        ROUTE_SESSION,
        ROUTE_NODES,
        ROUTE_NODE,
        ROUTE_ENDPOINTS,
        ROUTE_USERS,
        ROUTE_USERS_ADD,
        ROUTE_USERS_DISABLE,
        ROUTE_USERS_ENABLE
    };

    struct SRoute
    {
        const char* path;
        const char* method;
        bool prefix;
        bool needsLogin;
        bool needsAdmin;
        ERoute route;
    };

    struct SCaller
    {
        GUCEF::CORE::CString user;
        GUCEF::CORE::CString role;
        GUCEF::CORE::CString sessionId;         /**< empty for a call with the bearer token */
        bool viaToken;

        SCaller( void ) : user(), role(), sessionId(), viaToken( false ) {}
    };

    CLabWebApi( const CLabWebApi& src );
    CLabWebApi& operator=( const CLabWebApi& src );

    static const SRoute* FindRoute( const GUCEF::CORE::CString& path, bool& pathKnown );
    bool Authenticate( const GUCEF::WEB::CHttpRequestData& request, GUCEF::UInt64 nowInMs, SCaller& caller );
    bool IsTokenValid( const GUCEF::CORE::CString& given ) const;
    /** Reads the users file again, at most once a second unless now; the sessions of users that changed end */
    void RefreshUsers( GUCEF::UInt64 nowInMs, bool now );
    void Respond( GUCEF::WEB::CHttpResponseData& response, int statusCode, const GUCEF::CORE::CString& json ) const;
    void RespondError( GUCEF::WEB::CHttpResponseData& response, int statusCode, const char* text ) const;
    void SessionAnswer( GUCEF::WEB::CHttpResponseData& response, const SCaller& caller ) const;
    bool StartLogin( const GUCEF::WEB::CHttpRequestData& request, const GUCEF::CORE::CString& peer, GUCEF::UInt64 nowInMs,
                     GUCEF::WEB::CHttpResponseData& response, SPasswordJob& job );
    void LoggedIn( const GUCEF::CORE::CString& user, bool viaToken, bool persistent, const GUCEF::CORE::CString& peer, GUCEF::UInt64 nowInMs,
                   GUCEF::WEB::CHttpResponseData& response );
    void LoginFailed( const GUCEF::CORE::CString& userKey, const GUCEF::CORE::CString& logName, const GUCEF::CORE::CString& peer,
                      GUCEF::UInt64 nowInMs, GUCEF::WEB::CHttpResponseData& response );
    bool ChangeUser( ERoute route, const GUCEF::WEB::CHttpRequestData& request, const SCaller& caller, const GUCEF::CORE::CString& peer,
                     GUCEF::WEB::CHttpResponseData& response, SPasswordJob& job );

    static GUCEF::CORE::CString UserKey( const GUCEF::CORE::CString& user );
    static GUCEF::CORE::CString BodyOf( const GUCEF::WEB::CHttpRequestData& request );

    SWebSettings m_settings;
    const CAddressRanges& m_allowedRanges;
    ILabHubView& m_hub;
    CWebUserStore& m_users;
    CWebSessionTable m_sessions;
    CLoginThrottle m_addressThrottle;
    CLoginThrottle m_userThrottle;
    bool m_tls;
    bool m_hasToken;
    GUCEF::UInt8 m_tokenDigest[ 32 ];
    GUCEF::CORE::CString m_dummyRecord;
    GUCEF::CORE::CString m_listenAddress;
    GUCEF::UInt16 m_port;
    GUCEF::UInt64 m_nextSweepInMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_WEBAPI_H ? */
