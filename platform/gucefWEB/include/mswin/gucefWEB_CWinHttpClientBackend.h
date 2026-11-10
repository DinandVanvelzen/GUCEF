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

#ifndef GUCEF_WEB_CWINHTTPCLIENTBACKEND_H
#define GUCEF_WEB_CWINHTTPCLIENTBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#include "gucefWEB_CIHttpClientBackend.h"
#define GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#endif /* GUCEF_WEB_CIHTTPCLIENTBACKEND_H ? */

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

class CWinHttpSession;

/**
 *  HTTP client backend which uses WinHTTP, the HTTP stack of Microsoft Windows. As an operating system facility it
 *  is available to platform modules, unlike libraries such as curl. It supports http and https, the latter using
 *  the TLS stack and the certificate store of Windows, without any plugin needing to be loaded.
 *
 *  WinHTTP runs the requests on its own threads. They only note down what happens, which is then handled on the
 *  pulses of the pulse generator the request runs on, so everything the observer sees happens on that thread.
 *
 *  The backend carries out exactly one HTTP exchange per request: redirects, cookies, authentication and
 *  decompression of WinHTTP are switched off, they are taken care of for all backends alike by CHTTPClient.
 *  The size of a request body has to be known up front for WinHTTP, a body of unknown size is read into memory first.
 */
class GUCEF_WEB_PUBLIC_CPP CWinHttpClientBackend : public CIHttpClientBackend
{
    public:

    static const CString BackendName;

    CWinHttpClientBackend( void );

    virtual ~CWinHttpClientBackend() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SupportsScheme( const CString& scheme ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CIHttpClientBackendRequestPtr StartRequest( const CHttpRequestData& request          ,
                                                        const CHttpClientRequestOptions& options ,
                                                        CHttpResponseData& response              ,
                                                        CHttpClientTransferInfo& transferInfo    ,
                                                        CORE::PulseGeneratorPtr pulseGenerator   ,
                                                        CIHttpClientBackendObserver& observer    ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWinHttpClientBackend( const CWinHttpClientBackend& src );              /**< not supported */
    CWinHttpClientBackend& operator=( const CWinHttpClientBackend& src );   /**< not supported */

    private:

    CORE::CTSharedPtr< CWinHttpSession, MT::CMutex > m_session;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWINHTTPCLIENTBACKEND_H ? */
