/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H
#define GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H
#include "gucefWEB_CIMcpHttpAuthenticator.h"
#define GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H
#endif /* GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H ? */

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
 *  "static-bearer": one secret token that the client sends as Authorization: Bearer <token> on every request.
 *
 *  The token comes from SetToken(), else from the file of SetTokenFile() (bearerTokenFile). A file that does not exist
 *  is made at Prepare() for its owner only from its creation on (CORE::CreateOwnerOnlyFile()), and holds a random
 *  32-byte token as base64url. An existing file is only read when it is a regular file of the user that runs the
 *  process that nobody else can access (CORE::ReadOwnerOnlyFile()), never through a symbolic link. Tokens are compared as SHA-256 digests in constant time, so the length of a token does not leak. A
 *  token in the query of the URI is refused (400), as the MCP authorization rules forbid it there. Missing or wrong
 *  credentials get 401 with WWW-Authenticate: Bearer. The token is never logged.
 *
 *  The principal of an accepted request is subject "bearer:<tokenLabel>", authenticated, with the configured scopes.
 *  Configuration: bearerTokenFile, tokenLabel (default "default"), scopes (a list).
 */
class GUCEF_WEB_PUBLIC_CPP CMcpStaticBearerAuthenticator : public CIMcpHttpAuthenticator
{
    public:

    CMcpStaticBearerAuthenticator( void );

    virtual ~CMcpStaticBearerAuthenticator() GUCEF_VIRTUAL_OVERRIDE;

    void SetToken( const CORE::CString& token );

    /** The token in use after Prepare(), for the application that hands it to its client */
    CORE::CString GetToken( void ) const;

    void SetTokenFile( const CORE::CString& tokenFile );

    const CORE::CString& GetTokenFile( void ) const;

    void SetTokenLabel( const CORE::CString& label );

    void SetScopes( const CORE::CString::StringVector& scopes );

    virtual const CORE::CString& GetAuthenticatorName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Prepare( CORE::CString& problem ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Authenticate( const CHttpRequestData& request ,
                               COM::CMcpPrincipal& principal   ,
                               CHttpResponseData& challenge    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the query of a request URI carries an access token, which it must not */
    static bool HasTokenInQuery( const CORE::CString& queryText );

    private:

    bool IsTokenValid( const CORE::CString& given ) const;

    CORE::CString m_token;
    CORE::CString m_tokenFile;
    CORE::CString m_tokenLabel;
    CORE::CString::StringVector m_scopes;
    UInt8 m_tokenDigest[ 32 ];
    bool m_prepared;
};

typedef CORE::CTFactory< CIMcpHttpAuthenticator, CMcpStaticBearerAuthenticator, MT::CMutex > TMcpStaticBearerAuthenticatorFactory;

/*-------------------------------------------------------------------------*/

/** "none": every request is accepted as subject "anonymous", not authenticated. For tests; it logs a warning when prepared */
class GUCEF_WEB_PUBLIC_CPP CMcpNoHttpAuthenticator : public CIMcpHttpAuthenticator
{
    public:

    CMcpNoHttpAuthenticator( void );

    virtual ~CMcpNoHttpAuthenticator() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetAuthenticatorName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Prepare( CORE::CString& problem ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Authenticate( const CHttpRequestData& request ,
                               COM::CMcpPrincipal& principal   ,
                               CHttpResponseData& challenge    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef CORE::CTFactory< CIMcpHttpAuthenticator, CMcpNoHttpAuthenticator, MT::CMutex > TMcpNoHttpAuthenticatorFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H ? */
