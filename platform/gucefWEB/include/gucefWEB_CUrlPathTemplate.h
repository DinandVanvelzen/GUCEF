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

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#define GUCEF_WEB_CURLPATHTEMPLATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <utility>

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
 *  A URL reduced to the kind of request it is. The endpoints of a web application are shaped like
 *  /api/accounts/1234567/orders/89012345, to describe the endpoint instead of one request the identifiers in the path
 *  are replaced: /api/accounts/{n}/orders/{n}. The parts that matter for that are public members, Parse() fills them.
 *
 *  Placeholders: {n} numbers (3 digits or more, also composed: 123-456), {guid} GUIDs with or without dashes and other
 *  32 digit hex runs, {hex} hex runs of 16 or more with a digit, {id} generated ids of 12 or more letters and digits
 *  with both. A file extension of up to 5 letters and digits is kept: /img/{n}.jpg
 */
class GUCEF_WEB_PUBLIC_CPP CUrlPathTemplate
{
    public:

    typedef std::pair< CORE::CString, CORE::CString >   TQueryParam;
    typedef GUCEF::vector< TQueryParam >                TQueryParamVector;

    CORE::CString scheme;                   /**< lower case */
    CORE::CString host;                     /**< lower case, without user info, including the port if there is one */
    CORE::CString path;                     /**< as it appeared in the URL */
    CORE::CString pathTemplate;             /**< the path with identifiers replaced by placeholders */
    TQueryParamVector queryParams;          /**< in the order they appeared, names and values not decoded */

    CUrlPathTemplate( void );

    /** host + path template, what requests of the same kind have in common */
    CORE::CString GetKey( void ) const;

    void Clear( void );

    /** Splits a URL, fails if it has no scheme (data:, blob:, relative paths) */
    static bool Parse( const CORE::CString& url, CUrlPathTemplate& result );

    /** Replaces identifier like path segments by placeholders */
    static CORE::CString TemplatePath( const CORE::CString& path );

    /** Replaces one segment by its placeholder if it is an identifier, a file extension is kept */
    static CORE::CString TemplateSegment( const CORE::CString& segment );

    /** Is this a segment which varies per object: a number, a GUID, a hash, a generated id */
    static bool LooksLikeId( const CORE::CString& segment );

    /** The segment following the given one when that is an identifier, for example 12345 in /tree/12345/person */
    static CORE::CString FindIdAfterSegment( const CORE::CString& path, const CORE::CString& segmentName );

    /** Decodes '%' escapes (upper and lower case hex digits) and '+'; an escape that is not valid is kept as is */
    static CORE::CString PercentDecode( const CORE::CString& text );

    /** Escapes a string for use inside a JSON string: quotes, backslashes and control characters, UTF-8 passes */
    static CORE::CString EscapeJson( const CORE::CString& text );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */
