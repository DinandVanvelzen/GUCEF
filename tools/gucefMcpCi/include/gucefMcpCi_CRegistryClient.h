/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MCPCI_CREGISTRYCLIENT_H
#define GUCEF_MCPCI_CREGISTRYCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads from an OCI container registry (the distribution API, /v2/) with CHTTPClient: the digest of a tag.
 *
 *    HEAD /v2/<repository>/manifests/<tag>, Accept: OCI image index and manifest, docker manifest v2 and list; the
 *    digest is the Docker-Content-Digest header, or the SHA-256 of the manifest of a GET when the header is missing.
 *
 *  Authentication, as registries ask for it in their 401 answer:
 *
 *    Bearer realm="...",service="..."   the token handshake: GET <realm>?service=<service>&scope=repository:<repo>:pull
 *                                       with Basic credentials (user and token), then the request again with the bearer
 *                                       token of the answer ("token" or "access_token"). The realm must be on the same
 *                                       scheme, host and port as the registry: the credentials are never sent elsewhere
 *    Basic, or the handshake failed     the request again with Basic credentials
 *
 *  Plain HTTP is accepted (the registry of the lab); HTTPS verifies the certificate. Requests follow no redirects, use no
 *  proxy of the environment, and have time and size limits. The token is never part of a message.
 *
 *  Thread-safe once set up.
 */
class CRegistryClient
{
    public:

    CRegistryClient( void );

    /** The token of Basic and of the token handshake; it is registered with the redaction of the HTTP logs */
    void SetToken( const GUCEF::CORE::CString& token );

    /** PEM file of the authorities for an HTTPS registry, empty for the trust store of the system */
    void SetCaBundlePath( const GUCEF::CORE::CString& caBundlePath );

    /**
     *  The digest of an image tag.
     *  @param registryUrl  scheme and host[:port] of the registry ("http://172.16.1.125:32001")
     *  @param user         the user the token belongs to
     *  @param repository   <owner>/<name>
     *  @param notFound     set when the registry has no such repository or tag
     */
    bool ResolveDigest( const GUCEF::CORE::CString& registryUrl  ,
                        const GUCEF::CORE::CString& user         ,
                        const GUCEF::CORE::CString& repository   ,
                        const GUCEF::CORE::CString& tag          ,
                        GUCEF::CORE::CString& digest             ,
                        bool& notFound                           ,
                        GUCEF::CORE::CString& error              ) const;

    /** Whether the text is a tag the distribution API takes: [A-Za-z0-9_][A-Za-z0-9_.-]{0,127} */
    static bool IsTag( const GUCEF::CORE::CString& text );

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CString > TStringMap;

    /** One request. @return false when no answer came */
    bool Execute( const GUCEF::CORE::CString& method          ,
                  const GUCEF::CORE::CString& url             ,
                  const GUCEF::CORE::CString& authorization   ,
                  bool acceptManifests                        ,
                  GUCEF::UInt64 maxBytes                      ,
                  GUCEF::Int32& status                        ,
                  TStringMap& headers                         ,
                  GUCEF::CORE::CDynamicBuffer& body           ,
                  GUCEF::CORE::CString& error                 ) const;

    /** The bearer token of the handshake the challenge asks for. @return false with the reason */
    bool FetchBearerToken( const GUCEF::CORE::CString& registryUrl   ,
                           const GUCEF::CORE::CString& user          ,
                           const GUCEF::CORE::CString& challenge     ,
                           const GUCEF::CORE::CString& repository    ,
                           GUCEF::CORE::CString& bearer              ,
                           GUCEF::CORE::CString& error               ) const;

    GUCEF::CORE::CString BasicAuthorization( const GUCEF::CORE::CString& user ) const;

    /** A header of an answer by its name, without regard to case */
    static GUCEF::CORE::CString Header( const TStringMap& headers, const char* name );

    /** The parameters of an authentication challenge: Bearer realm="a",service="b" */
    static bool ParseChallenge( const GUCEF::CORE::CString& challenge, GUCEF::CORE::CString& scheme, TStringMap& parameters );

    /** "scheme://host:port" of a URL, lower case, with the default port of the scheme filled in */
    static GUCEF::CORE::CString Origin( const GUCEF::CORE::CString& url );

    GUCEF::CORE::CString m_token;
    GUCEF::CORE::CString m_caBundlePath;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CREGISTRYCLIENT_H ? */
