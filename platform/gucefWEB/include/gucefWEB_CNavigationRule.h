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

#ifndef GUCEF_WEB_CNAVIGATIONRULE_H
#define GUCEF_WEB_CNAVIGATIONRULE_H

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
 *  One allow or deny rule of a CNavigationGuard: a host glob pattern and an optional path pattern.
 *
 *  As text: "host" or "host/path", for example "*.example.com", "app.example.com/accounts/positions",
 *  "app.example.com/reports/export-*.csv", "127.0.0.1:8080/". The host pattern is case insensitive and is matched against the
 *  host name, or against host:port when the pattern holds a colon. The path pattern is a glob (CORE::CGlobPattern) when
 *  it holds '*', '?' or '[': '*' matches within one path segment, '**' across segments, braces are literal (so a
 *  template such as {n} can be written as it is). Otherwise it is a prefix that ends at a segment boundary:
 *  "/accounts/positions" matches "/accounts/positions" and "/accounts/positions/lots" but not "/accounts/positionsX".
 *  An empty path pattern matches every path. Paths are matched in their normalised form, see NormalisePath().
 */
class GUCEF_WEB_PUBLIC_CPP CNavigationRule
{
    public:

    CORE::CString hostPattern;
    CORE::CString pathPattern;      /**< empty: any path */

    CNavigationRule( void );

    CNavigationRule( const CORE::CString& hostPattern, const CORE::CString& pathPattern );

    /** Reads "host" or "host/path"; a leading http:// or https:// is dropped. False for an empty host or a host with characters a host pattern cannot have */
    static bool Parse( const CORE::CString& text, CNavigationRule& rule );

    CORE::CString ToString( void ) const;

    /** @param hostName lower case, without the port. @param hostAndPort lower case, with the port when the URL has one */
    bool MatchesHost( const CORE::CString& hostName, const CORE::CString& hostAndPort ) const;

    /** @param normalisedPath a path as NormalisePath() gives it. @param caseSensitive allow rules are, deny rules are not */
    bool MatchesPath( const CORE::CString& normalisedPath, bool caseSensitive ) const;

    bool Matches( const CORE::CString& hostName, const CORE::CString& hostAndPort, const CORE::CString& normalisedPath, bool caseSensitivePath ) const;

    /**
     *  The path as rules see it: percent-encoded unreserved characters decoded (A-Z a-z 0-9 - . _ ~), other escapes kept
     *  with upper case hex digits, dot segments removed, "/" for an empty path. Sets 'ambiguous' when the path holds an
     *  encoded slash or backslash (%2F, %5C) or a backslash: such a path matches no allow rule.
     */
    static CORE::CString NormalisePath( const CORE::CString& path, bool& ambiguous );

    /** A path pattern with its braces escaped for CORE::CGlobPattern, so that {n} of a template matches itself */
    static CORE::CString EscapeBraces( const CORE::CString& pattern );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CNavigationRule >   TNavigationRuleVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CNAVIGATIONRULE_H ? */
