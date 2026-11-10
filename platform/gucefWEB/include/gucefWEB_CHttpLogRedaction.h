/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#define GUCEF_WEB_CHTTPLOGREDACTION_H

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
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The environment variable that has the same effect as SetFullUrlsInDebugLogs( true ) for a process that has
 *  not called it: set to 1, true or yes. Only debug builds look at it.
 */
#define GUCEF_WEB_LOG_FULL_URLS_ENVVAR      "GUCEF_WEB_LOG_FULL_URLS"

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
 *  The one way that the HTTP stack (the client, its backends, the server, the WebSocket and MCP transports) puts a
 *  URL, a header or a piece of text that came from the network in a log line or an error message:
 *
 *      GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Request to " << CHttpLogRedaction::RedactUrl( url ) << " failed" );
 *
 *  What is redacted by default, at every log level and in every build, is what CORE::CRedactor makes of it with:
 *  - URLs: the user info (also of a proxy URL), the value of every item of the query, the fragment, a host that
 *    is not a valid host, and the path segments that are identifiers or tokens (UUIDs, long hexadecimal and
 *    token like strings of 20 or more characters, numbers of 7 or more digits, see CORE::CRedactor::IsOpaqueIdentifier())
 *    which become "<redacted-id>". The path "/trader/v1/accounts/ABCDEF0123456789ABCDEF0123456789/transactions?x=1"
 *    is logged as "/trader/v1/accounts/<redacted-id>/transactions?x=***": the words, versions and short
 *    numbers of the path stay readable.
 *  - Header blocks and header values: the value of every sensitive header (Authorization, Proxy-Authorization,
 *    Cookie, Set-Cookie, X-Api-Key and every name with auth, token, secret, key, session and the like in it), the
 *    URLs in the request line and in Location, Referer and the like, and the lines that are not headers.
 *  - Text (error descriptions of a backend and of the operating system, a host or an origin as a client sent it): URLs
 *    in it as above, credentials after Bearer/Basic and the like, name=value pairs of a sensitive name, and long
 *    tokens.
 *  - Every secret an application registered with AddSecret(), wherever it is in the above.
 *
 *  A path segment that is a secret but does not look like an identifier (an account name, a short code, a hash of
 *  letters only) is not found by the rules of the heuristic. An application that learns such a value, for
 *  example the account hash a server hands out in one request and wants in the path of the next, registers it:
 *
 *      WEB::CHttpLogRedaction::AddSecret( accountHash );
 *
 *  after which it is "***" in every log line of the HTTP stack, in whatever form it is written in a URL, until
 *  the application takes it back with RemoveSecret().
 *
 *  Opt-out for developers: an application or developer who has to read the full URLs in the log of a debug
 *  build can call SetFullUrlsInDebugLogs( true ) or start the process with the environment variable
 *  GUCEF_WEB_LOG_FULL_URLS=1. Then the path identifiers, the values of the query and the fragment are logged as they are.
 *  The user info (passwords), the values of sensitive headers, the registered secrets and the other credentials
 *  stay redacted. The setting does nothing in a release build, where everything is always redacted, so an
 *  application cannot ship with it turned on by mistake.
 *
 *  The functions can be used from any thread. They never throw and take any input.
 */
class GUCEF_WEB_PUBLIC_CPP CHttpLogRedaction
{
    public:

    /** @return the URL as it can be logged, see the description of the class */
    static CORE::CString RedactUrl( const CORE::CString& url );

    /** @return the header section of a message (the start line and the header lines) as it can be logged */
    static CORE::CString RedactHeaders( const CORE::CString& headerBlock );

    /** @return the value of the header with the given name as it can be logged */
    static CORE::CString RedactHeaderValue( const CORE::CString& name  ,
                                            const CORE::CString& value );

    /** @return free text (an error description, a host name, an origin) as it can be logged */
    static CORE::CString RedactText( const CORE::CString& text );

    /**
     *  Registers a literal secret that is replaced wherever it occurs in what the functions above redact, in the
     *  form it is written as well as percent encoded. For values that are secret but do not look like it.
     *  See CORE::CRedactor::AddSecret()
     *
     *  @return false when the secret is empty
     */
    static bool AddSecret( const CORE::CString& secret );

    /**
     *  Forgets a secret that AddSecret() registered, for a component that registers what it learns for as long as it
     *  holds it and takes it back when it lets go (a login that ends, a plugin that unloads). A secret that several
     *  components registered stays registered until each of them has removed it: every AddSecret() is paired with one
     *  RemoveSecret().
     *
     *  @return false when the secret is not registered
     */
    static bool RemoveSecret( const CORE::CString& secret );

    /** Forgets all the secrets that AddSecret() registered, those of other components included */
    static void ClearSecrets( void );

    /**
     *  The opt-out for developers, see the description of the class. Default off, unless the environment variable
     *  GUCEF_WEB_LOG_FULL_URLS is set to 1, true or yes. Has no effect in a release build.
     */
    static void SetFullUrlsInDebugLogs( bool enabled );

    /** @return whether full URLs are logged: asked for and allowed by the build. Always false in a release build */
    static bool GetFullUrlsInDebugLogs( void );

    private:

    CHttpLogRedaction( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */
