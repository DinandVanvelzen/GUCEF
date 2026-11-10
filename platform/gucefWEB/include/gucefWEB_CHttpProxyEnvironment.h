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

#ifndef GUCEF_WEB_CHTTPPROXYENVIRONMENT_H
#define GUCEF_WEB_CHTTPPROXYENVIRONMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

/**
 *  The proxy the environment asks for, the way curl and most other tools read it: HTTPS_PROXY for https URLs,
 *  HTTP_PROXY for http URLs (also in lower case), and NO_PROXY with the hosts that are reached directly.
 *
 *  - The proxy is a URL of the form http://[user:password@]host[:port]. The port is 1080 when it is not given.
 *    Other schemes (https://, socks5://) are not supported: such a proxy is ignored, with a warning.
 *  - NO_PROXY is a comma separated list of host names (a name also matches its subdomains, a leading dot is
 *    allowed), IPv4 addresses, IPv4 networks (10.0.0.0/8), each optionally with a port, or a "*" for everything.
 *  - A host that is this computer (localhost, 127.x.x.x, ::1) never goes through a proxy.
 *  - HTTP_PROXY in upper case is ignored in a process that runs as a CGI program (REQUEST_METHOD is set): the
 *    request could have set it through a header ("httpoxy").
 *  - Read through CORE::GetEnv() so overrides of the application apply.
 */
class GUCEF_WEB_PUBLIC_CPP CHttpProxyEnvironment
{
    public:

    struct SProxy
    {
        CORE::CString host;
        UInt16 port;
        CORE::CString userName;         /**< empty when the proxy URL has none */
        CORE::CString password;

        SProxy( void ) : host(), port( 0 ), userName(), password() {}
    };
    typedef struct SProxy TProxy;

    /**
     *  @param scheme       "http" or "https", the scheme of the URL that is requested
     *  @param targetHost   the host of that URL
     *  @param targetPort   its port
     *  @return whether the environment has a proxy for it, which is then in 'proxy'
     */
    static bool Lookup( const CORE::CString& scheme     ,
                        const CORE::CString& targetHost ,
                        UInt16 targetPort               ,
                        TProxy& proxy                   );

    /** Reads "http://user:pass@host:8080", "host:8080" or "host". @return false for a proxy that cannot be used */
    static bool ParseProxyUrl( const CORE::CString& text, TProxy& proxy );

    /** Whether the host (and port) is in the NO_PROXY style list */
    static bool IsExcluded( const CORE::CString& host, UInt16 port, const CORE::CString& noProxyList );

    /** Whether the host is this computer: localhost, an address in 127.0.0.0/8 or ::1 */
    static bool IsLoopback( const CORE::CString& host );

    /** The value for the header Proxy-Authorization (Basic ...) for the credentials of the proxy, empty when it has none */
    static CORE::CString GetBasicAuthorization( const TProxy& proxy );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPPROXYENVIRONMENT_H ? */
