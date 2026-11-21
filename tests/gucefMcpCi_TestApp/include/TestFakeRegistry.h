/*
 *  gucefMcpCi_TestApp: tests of the gucefMcpCi MCP server
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

#ifndef GUCEF_MCPCI_TESTAPP_TESTFAKEREGISTRY_H
#define GUCEF_MCPCI_TESTAPP_TESTFAKEREGISTRY_H

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

namespace MCPCITEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A stand-in for an OCI container registry on 127.0.0.1 with what gucefMcpCi reads, in one of two ways of
 *  authentication:
 *
 *    MODE_BEARER   a manifest without "Authorization: Bearer <issued token>" is 401 with
 *                  Www-Authenticate: Bearer realm="<url>/v2/token",service="fake-registry"; GET /v2/token with
 *                  Basic <user>:<token> and scope=repository:<repo>:pull issues the token ({"token": ...})
 *    MODE_BASIC    a manifest needs Basic <user>:<token>, else 401 with Www-Authenticate: Basic realm="fake"
 *
 *    HEAD|GET /v2/<owner>/<name>/manifests/<tag>   200 with Docker-Content-Digest (unless switched off), 404 for others
 */
class CFakeRegistry : public GUCEF::MT::CActiveObject
{
    public:

    enum EMode
    {
        MODE_BEARER = 0,
        MODE_BASIC
    };

    CFakeRegistry( EMode mode, const GUCEF::CORE::CString& user, const GUCEF::CORE::CString& token );

    virtual ~CFakeRegistry();

    bool Start( void );

    void Stop( void );

    /** 127.0.0.1:<port>, the registryHost of ci.json */
    GUCEF::CORE::CString GetHost( void ) const;

    void AddManifest( const GUCEF::CORE::CString& repository, const GUCEF::CORE::CString& tag, const GUCEF::CORE::CString& digest );

    /** Without the header the client takes the SHA-256 of the manifest of a GET */
    void SetSendDigestHeader( bool send );

    /** The manifest a GET answers with */
    static GUCEF::CORE::CString ManifestBody( const GUCEF::CORE::CString& digest );

    GUCEF::UInt32 GetTokenRequestCount( void ) const;

    GUCEF::UInt32 GetUnauthorizedCount( void ) const;

    /** "METHOD path?query auth=<scheme>" of every request, one per line */
    GUCEF::CORE::CString GetRequestLog( void ) const;

    void Handle( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CString > TStringMap;

    CFakeRegistry( const CFakeRegistry& src );                  /**< not implemented */
    CFakeRegistry& operator=( const CFakeRegistry& src );       /**< not implemented */

    EMode m_mode;
    GUCEF::CORE::CString m_user;
    GUCEF::CORE::CString m_token;
    GUCEF::CORE::CString m_issuedToken;
    TStringMap m_manifests;                 /**< "<repository>:<tag>" to the digest */
    bool m_sendDigestHeader;
    GUCEF::UInt32 m_tokenRequests;
    GUCEF::UInt32 m_unauthorized;
    GUCEF::CORE::CString m_requestLog;
    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::WEB::CHTTPServer* m_server;
    void* m_factory;
    bool m_running;
};

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_TESTAPP_TESTFAKEREGISTRY_H ? */
