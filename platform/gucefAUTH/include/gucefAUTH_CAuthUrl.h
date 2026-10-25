/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CAUTHURL_H
#define GUCEF_AUTH_CAUTHURL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** URLs, query strings and form bodies for the flows: strict, byte based and without surprises */
class GUCEF_AUTH_EXPORT_CPP CAuthUrl
{
    public:

    static const UInt32 MaxUrlBytes = 2048;
    static const UInt32 MaxQueryBytes = 8192;
    static const UInt32 MaxQueryParameters = 64;

    /** Decodes %XX; with form set a '+' is a space. False for a % that is not followed by two hex digits */
    static bool Decode( const CORE::CString& text, bool form, CORE::CString& decoded );

    /** name=value&name=value, names and values percent encoded (CORE::CUrlEncoder), in the order of the names */
    static CORE::CString EncodeForm( const CORE::CString::StringMap& values );

    /**
     *  Parses a query string or form body. False when it is longer than MaxQueryBytes, has more than MaxQueryParameters
     *  parameters or has an encoding error. A name that occurs twice sets duplicate; the first value is kept.
     */
    static bool ParseQuery( const CORE::CString& query, CORE::CString::StringMap& values, bool& duplicate );

    /** The url with the parameters appended to its query */
    static CORE::CString AppendQuery( const CORE::CString& url, const CORE::CString::StringMap& values );

    /** The parts of an absolute url; false when it is not one or has user information or control characters */
    static bool Split( const CORE::CString& url        ,
                       CORE::CString& scheme           ,
                       CORE::CString& host             ,
                       UInt16& port                    ,
                       CORE::CString& path             ,
                       CORE::CString& query            );

    /** 127.0.0.1 (any 127.x.y.z), localhost and the IPv6 loopback address */
    static bool IsLoopbackHost( const CORE::CString& host );

    /**
     *  Whether the url may be used for a request of the engine: https, or http when insecureLoopbackAllowed and the host
     *  is a loopback address. No user information, no fragment, at most MaxUrlBytes.
     */
    static bool IsAcceptableEndpoint( const CORE::CString& url, bool insecureLoopbackAllowed );

    /** Whether the url starts with the scheme https, whatever the case; no other check */
    static bool IsHttpsUrl( const CORE::CString& url );

    /** The origin of the url: scheme://host[:port] */
    static CORE::CString GetOrigin( const CORE::CString& url );

    /** Whether the text is made of characters that need no encoding in a url (A-Z a-z 0-9 - . _ ~) only */
    static bool IsUnreservedText( const CORE::CString& text );

    private:

    CAuthUrl( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHURL_H ? */
