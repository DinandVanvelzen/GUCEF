/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_CHTTPREQUESTDATA_H
#define GUCEF_WEB_CHTTPREQUESTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

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

class GUCEF_WEB_PUBLIC_CPP CHttpRequestData : public CORE::CICloneable
{
    public:

    typedef CORE::CString::StringVector                TStringVector;
    typedef GUCEF::map< CORE::CString, CORE::CString >   TStringMap;

    TStringVector resourceRepresentations;
    TStringVector encodingRepresentations;
    TStringVector resourceVersions;
    CString requestProtocol;
    CString requestProtocolVersion;
    CString requestType;
    CString requestHost;
    CString requestUri;
    CString requestUriParams;
    CORE::CDynamicBuffer content;
    CString contentRepresentation;
    CString transactionID;
    bool keepConnectionsAlive;
    bool isWebSocketUpgrade;
    TStringMap otherHeaders;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CString ConstructUriWithAuthority( void ) const;

    /**
     *  Sets the request target from an absolute URL of the form scheme://host[:port]/path[?query]
     *  as used by a client: requestProtocol, requestHost (including the port if given), requestUri (the path)
     *  and requestUriParams (the query, without '?'). A fragment is dropped.
     *
     *  @return whether the URL could be parsed
     */
    bool SetUrl( const CString& url );

    /**
     *  @return the absolute URL of the request including the query, the inverse of SetUrl()
     */
    CString GetUrl( void ) const;

    /**
     *  @return the host name of requestHost without the port, IPv6 addresses without brackets
     */
    CString GetHostName( void ) const;

    /**
     *  @return the port of requestHost, or the default port of the protocol if none is given
     */
    UInt16 GetPort( void ) const;

    /**
     *  Whether two request targets are of the same origin (RFC 6454): the same scheme, host and port.
     *  The scheme and host are compared ignoring case, a port which is not given is the default port of the
     *  scheme, the user info in front of a host is not part of the origin. A request target which does not
     *  have a scheme, a host, or has a port which is not a number is never of the same origin as any other.
     *
     *  @param protocolA scheme of the first request target, for example "https"
     *  @param hostA authority of the first request target as in requestHost: [user@]host[:port]
     */
    static bool IsSameOrigin( const CString& protocolA, const CString& hostA ,
                              const CString& protocolB, const CString& hostB );

    CHttpRequestData( void );
    CHttpRequestData( const CHttpRequestData& src );
    virtual ~CHttpRequestData();

    private:

    mutable CString m_requestUriWithAuthority;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPREQUESTDATA_H ? */
