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

#ifndef GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H
#define GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_COM_CMCPPRINCIPAL_H
#include "gucefCOM_CMcpPrincipal.h"
#define GUCEF_COM_CMCPPRINCIPAL_H
#endif /* GUCEF_COM_CMCPPRINCIPAL_H ? */

#ifndef GUCEF_WEB_CHTTPREQUESTDATA_H
#include "gucefWEB_CHttpRequestData.h"
#define GUCEF_WEB_CHTTPREQUESTDATA_H
#endif /* GUCEF_WEB_CHTTPREQUESTDATA_H ? */

#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */

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
 *  Decides who sends a request to the Streamable HTTP transport of an MCP server, from what accompanies the request:
 *  the principal of its channel is never taken from the message. Authenticators are made by name through the
 *  factory of CWebGlobal, GetMcpHttpAuthenticatorFactory(), so that other modules can add their own (an OAuth
 *  resource server for example). Built in are "static-bearer" and "none".
 *
 *  The transport calls an authenticator on the thread of the server, for the header block of every request, before
 *  its body is read.
 */
class GUCEF_WEB_PUBLIC_CPP CIMcpHttpAuthenticator : public CORE::CIConfigurable
{
    public:

    static const CORE::CString StaticBearerAuthenticatorName;  /**< "static-bearer" */
    static const CORE::CString NoAuthenticatorName;            /**< "none": for tests, every request is anonymous */

    CIMcpHttpAuthenticator( void );

    virtual ~CIMcpHttpAuthenticator() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetAuthenticatorName( void ) const = 0;

    /**
     *  Gets the authenticator ready when the transport starts, for example by reading or making its credentials.
     *  @return false refuses the start of the transport, with the reason in 'problem'
     */
    virtual bool Prepare( CORE::CString& problem ) = 0;

    /**
     *  @param principal    filled in when the request is accepted; its transport is set already
     *  @param challenge    filled in when the request is refused: the status (401 for missing or wrong credentials)
     *                      and header fields such as WWW-Authenticate
     *  @return whether the request is accepted
     */
    virtual bool Authenticate( const CHttpRequestData& request ,
                               COM::CMcpPrincipal& principal   ,
                               CHttpResponseData& challenge    ) = 0;

    private:

    CIMcpHttpAuthenticator( const CIMcpHttpAuthenticator& src );
    CIMcpHttpAuthenticator& operator=( const CIMcpHttpAuthenticator& src );
};

typedef CORE::CTAbstractFactory< CORE::CString, CIMcpHttpAuthenticator, MT::CMutex >   TMcpHttpAuthenticatorFactory;
typedef TMcpHttpAuthenticatorFactory::TProductPtr                                       CIMcpHttpAuthenticatorPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H ? */
