/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CNATIVEHTTPCLIENTBACKEND_H
#define GUCEF_WEB_CNATIVEHTTPCLIENTBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#include "gucefWEB_CIHttpClientBackend.h"
#define GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#endif /* GUCEF_WEB_CIHTTPCLIENTBACKEND_H ? */

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
 *  HTTP client backend which only uses the sockets of the platform: HTTP/1.1 with one connection per request.
 *  Always available, it is the fallback when no plugin provides a backend.
 *  Request and response bodies of any size are streamed, responses can use Content-Length, chunked transfer
 *  encoding or end at the close of the connection.
 *
 *  https goes through the TLS layer of gucefCOMCORE (CTlsClientConnection): the certificate is verified against the
 *  trust store of the system, or the CA of the options, and has to name the host. SupportsScheme( "https" ) is true
 *  when a TLS provider is available at that moment: on Linux that is the OpenSSL of the system, loaded when first
 *  asked for. Without one https requests fail with HTTPERRORCLASS_NO_BACKEND and "no HTTPS support: <reason>".
 *
 *  Proxies: the proxy of the options, else the one of the environment (HTTPS_PROXY, HTTP_PROXY, NO_PROXY: see
 *  CHttpProxyEnvironment; switched off with options.useEnvironmentProxy). A https request goes through a proxy with a
 *  CONNECT tunnel and TLS inside of it; a http request is sent to the proxy with the absolute URL.
 *  Not supported: SOCKS and PAC proxies, proxies that need NTLM or Negotiate.
 */
class GUCEF_WEB_PUBLIC_CPP CNativeHttpClientBackend : public CIHttpClientBackend
{
    public:

    static const CString BackendName;

    CNativeHttpClientBackend( void );
    virtual ~CNativeHttpClientBackend() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SupportsScheme( const CString& scheme ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether https can be carried out right now: a TLS provider is available. When not, whyNot says why */
    static bool IsHttpsAvailable( CString& whyNot );

    virtual CIHttpClientBackendRequestPtr StartRequest( const CHttpRequestData& request          ,
                                                        const CHttpClientRequestOptions& options ,
                                                        CHttpResponseData& response              ,
                                                        CHttpClientTransferInfo& transferInfo    ,
                                                        CORE::PulseGeneratorPtr pulseGenerator   ,
                                                        CIHttpClientBackendObserver& observer    ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CNATIVEHTTPCLIENTBACKEND_H ? */
