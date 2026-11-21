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

#ifndef GUCEF_WEB_CWRITEREQUESTRULE_H
#define GUCEF_WEB_CWRITEREQUESTRULE_H

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
 *  One allow rule of a CWriteRequestGuard: the exact method, scheme, host and path (or path template) of a request
 *  other than GET, HEAD or OPTIONS that a page may send. As text:
 *
 *      METHOD [http://|https://]host[:port]/path[?query]
 *
 *  - METHOD is compared case sensitively, as HTTP does: "POST" does not allow "post" or "Post".
 *  - Without a scheme only https is allowed; "http://" has to be written to allow plain http.
 *  - The host is compared exactly (no patterns, no sub domains), without case; a port is part of the host: "h" does not
 *    allow "h:8443" and the reverse.
 *  - The path is compared segment by segment, case sensitively. A segment written as a placeholder matches one segment
 *    of the request: {n} a run of 1 to 20 digits, {guid}, {hex} and {id} what CUrlPathTemplate templates as such, {any}
 *    any segment of letters, digits and "-._~". A trailing slash is a segment of its own: "/a/b" and "/a/b/" differ.
 *  - Without a query part the request must have no query; "?*" allows any query; any other query must equal the
 *    request's query byte for byte.
 *
 *  Paths are read strictly, the rule's and the request's alike (SplitStrictPath()): percent escapes, backslashes, ";",
 *  dot segments, empty segments other than a trailing slash, and characters outside the path grammar make a path that no
 *  rule matches. That leaves no room for normalisation tricks: what a rule names is exactly what the server receives.
 */
class GUCEF_WEB_PUBLIC_CPP CWriteRequestRule
{
    public:

    enum EQueryMode
    {
        QUERY_NONE  ,   /**< the request must have no query */
        QUERY_ANY   ,   /**< "?*": any query */
        QUERY_EXACT     /**< the query must equal the rule's */
    };

    CORE::CString method;                       /**< as written, compared case sensitively */
    CORE::CString scheme;                       /**< "https" unless the rule names "http://" */
    CORE::CString host;                         /**< lower case, with ":port" when the rule has one */
    CORE::CString path;                         /**< as written */
    CORE::CString::StringVector segments;       /**< the segments of the path, placeholders included */
    EQueryMode queryMode;
    CORE::CString query;                        /**< for QUERY_EXACT, without the "?" */

    CWriteRequestRule( void );

    /** False, with the reason in problem, when the text is not a rule; the rule is then unchanged */
    static bool Parse( const CORE::CString& text, CWriteRequestRule& rule, CORE::CString* problem = GUCEF_NULL );

    /** The rule as text, as Parse() reads it */
    CORE::CString ToString( void ) const;

    bool MatchesMethod( const CORE::CString& requestMethod ) const;

    /** @param requestHost lower case, with ":port" when the address has one */
    bool MatchesHost( const CORE::CString& requestHost ) const;

    bool MatchesScheme( const CORE::CString& requestScheme ) const;

    /** @param requestSegments the segments of a path SplitStrictPath() accepted */
    bool MatchesPath( const CORE::CString::StringVector& requestSegments ) const;

    /** @param hasQuery whether the address holds a query that is not empty */
    bool MatchesQuery( bool hasQuery, const CORE::CString& requestQuery ) const;

    /**
     *  Splits an absolute path in its segments, refusing anything that could be read in more than one way: a path that
     *  does not start with "/", percent escapes, backslashes, ";", control or non-ASCII characters, characters outside
     *  the path grammar, "." and ".." segments, and empty segments other than one trailing slash.
     *  @param allowPlaceholders accept segments written as "{name}" (for rules)
     */
    static bool SplitStrictPath( const CORE::CString& path, CORE::CString::StringVector& segments, bool allowPlaceholders = false );

    /** Is this a placeholder segment a rule may hold: {n}, {guid}, {hex}, {id} or {any} */
    static bool IsPlaceholder( const CORE::CString& segment );

    /** Does one segment of a request match one segment of a rule (literal or placeholder) */
    static bool SegmentMatches( const CORE::CString& ruleSegment, const CORE::CString& requestSegment );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CWriteRequestRule > TWriteRequestRuleVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWRITEREQUESTRULE_H ? */
