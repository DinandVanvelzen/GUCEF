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

#ifndef GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H
#define GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

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
 *  Picks the HTTP client backend to use for a URL scheme from an ordered list of preferred backends,
 *  creating each backend once via the factory of CWebGlobal and reusing it for all later requests so that
 *  a backend can keep connections and caches alive across requests.
 *  Thread-safe, shared between a CHTTPClient and the requests it has in progress.
 */
class GUCEF_WEB_PUBLIC_CPP CHttpClientBackendSelection
{
    public:

    typedef CString::StringVector TStringVector;

    /**
     *  the default order: "curl" (the plugin httpclientpluginCURL) when loaded, "winhttp" on Windows, the native backend otherwise.
     *  What carries https: the native backend does when a TLS provider is available (on Linux the OpenSSL of the system, loaded when
     *  first asked for), and a plugin backend such as curl does when it is loaded; on Windows "winhttp" is part of the build and uses the
     *  certificate store of the system. All of them verify the certificate of the server against the trust store of the system, or
     *  SSL_CERT_FILE, or CHttpClientRequestOptions::caBundlePath. Loading a plugin is a choice, so it comes first.
     *  See IsSchemeSupported().
     */
    static TStringVector GetDefaultPreferredBackends( void );

    CHttpClientBackendSelection( void );
    virtual ~CHttpClientBackendSelection();

    /** changing the preference also releases the backend objects that were created so far */
    void SetPreferredBackends( const TStringVector& backendNames );

    TStringVector GetPreferredBackends( void ) const;

    /**
     *  @return the first preferred backend that is registered and supports the scheme, or a NULL pointer
     */
    CIHttpClientBackendPtr GetBackendForScheme( const CString& scheme );

    /** whether any preferred backend that is registered right now supports the scheme; unlike GetBackendForScheme() nothing is logged when none does */
    bool IsSchemeSupported( const CString& scheme );

    /**
     *  Why no backend carries the scheme, for a message: for https the reason the TLS provider gives ("no HTTPS support: libssl.so.3:
     *  cannot open shared object file ..."), for anything else that the scheme is not known
     */
    static CString DescribeUnsupportedScheme( const CString& scheme );

    /**
     *  The preferred backends that are registered right now, the most basic first, each followed by "(https)" when it
     *  carries https as well: "native, curl (https)"
     */
    CString DescribeAvailableBackends( void );

    private:

    CIHttpClientBackendPtr FindBackendForScheme( const CString& scheme, bool logWhenNone );


    CHttpClientBackendSelection( const CHttpClientBackendSelection& src );            /**< not supported */
    CHttpClientBackendSelection& operator=( const CHttpClientBackendSelection& src ); /**< not supported */

    typedef std::map< CString, CIHttpClientBackendPtr > TBackendMap;

    TStringVector m_preferredBackends;
    TBackendMap m_backends;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CHttpClientBackendSelection, MT::CMutex > CHttpClientBackendSelectionPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H ? */
