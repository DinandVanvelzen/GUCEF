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

#ifndef GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#define GUCEF_WEB_CSITEDISCOVERYREDACTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#include "gucefWEB_CSiteDiscoverySettings.h"
#define GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#endif /* GUCEF_WEB_CSITEDISCOVERYSETTINGS_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#ifndef GUCEF_WEB_COBSERVEDWEBREQUEST_H
#include "gucefWEB_CObservedWebRequest.h"
#define GUCEF_WEB_COBSERVEDWEBREQUEST_H
#endif /* GUCEF_WEB_COBSERVEDWEBREQUEST_H ? */

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
 *  The privacy rules of site discovery, applied in one place: which requests are recorded at all (host filter,
 *  allow-list of path prefixes), which are never recorded beyond the fact that they happened (sign-in, security and
 *  credential pages and hosts), which names are secret, and how URLs, headers and bodies are redacted before they go
 *  into a journal or a report. See CSiteDiscoverySettings for the rules and their defaults.
 *
 *  It also holds the registry of the secret values that passed through during this session: cookie values, credentials
 *  after their scheme, the values of secret headers, query items and body fields. A journal line or report entry in which
 *  one of them occurs is replaced by a placeholder that names the secret class and never the value, see
 *  FindKnownSecret(). The registry and the key of the value hashes exist in memory only.
 *
 *  The tripwire is precise on purpose, because a value that is looked for in ordinary text trips on ordinary text: the
 *  name rule is a substring rule (keyImages holds "key", author holds "auth"), so the values of such fields are registered
 *  too, and words, numbers and path pieces among them would be found in URL templates, field names and sizes. A value is
 *  therefore only looked for when IsPlausibleSecret() accepts it:
 *   - at least knownSecretMinLength bytes (default 12, never below MinSecretLength = 8);
 *   - made like a token: some run of letters and digits of 8 or more characters that mixes letters and digits, or a
 *     run of 16 or more digits, or a run of 32 or more letters. Words, identifiers, numbers of ordinary length, URLs and
 *     paths of words (cdn-cgi/challenge-platform, unreal-engine, application/json, true) are never secrets here;
 *   - at least knownSecretMinEntropyBits (default 32) of Shannon entropy over the whole value, which drops repeated
 *     characters and patterns.
 *  A value shorter than 24 bytes only counts where it stands as a whole token (no letter or digit directly before or
 *  after it, a percent encoded or backslash escaped neighbour counts as a boundary); longer values are found anywhere.
 *  Values that are not looked for are still redacted where they stand, by the name rules and the query, header and body
 *  handling: only this last line of defence skips them.
 *
 *  The recorder, the journal and the catalog of one session share one redactor (SetRedactor() of each), so what the
 *  recorder registered is known when the journal and the report are written. Not thread-safe: use it on one thread.
 */
class GUCEF_WEB_PUBLIC_CPP CSiteDiscoveryRedactor
{
    public:

    static const CORE::CString NotRecordedMarker;       /**< "<not recorded>": the value of a secret header */
    static const CORE::CString SecretMarker;            /**< "{secret}": a secret value in strip mode and in bodies */
    static const UInt32 MinSecretLength = 8;            /**< the lowest value of the knownSecretMinLength setting: shorter values are redacted where they are, but not looked for elsewhere */

    /** Where a registered secret came from; the class is shown in placeholders, the value never */
    enum ESecretClass
    {
        SECRET_CLASS_COOKIE                 ,   /**< "cookie": a value of a Cookie or Set-Cookie header */
        SECRET_CLASS_CREDENTIAL_HEADER      ,   /**< "credential-header": the value of a secret header, such as Authorization */
        SECRET_CLASS_QUERY_VALUE            ,   /**< "query-value": the value of a secret query item */
        SECRET_CLASS_REQUEST_BODY_FIELD     ,   /**< "request-body-field": a secret field of a request body */
        SECRET_CLASS_RESPONSE_BODY_FIELD    ,   /**< "response-body-field": a secret field of a response body, such as a token a response hands out */
        SECRET_CLASS_OTHER                      /**< "other" */
    };

    typedef GUCEF::map< CORE::CString, UInt32 > TClassCountMap;

    CSiteDiscoveryRedactor( void );

    explicit CSiteDiscoveryRedactor( const CSiteDiscoverySettings& settings );

    void SetSettings( const CSiteDiscoverySettings& settings );

    const CSiteDiscoverySettings& GetSettings( void ) const;

    /** Does the host contain one of the terms (case insensitive)? No terms: every host does */
    static bool MatchesHostFilter( const CORE::CString::StringVector& terms, const CORE::CString& host );

    /** Is the request in scope: the host passes the host filter and the path the allow-list of path prefixes */
    bool IsInScope( const CUrlPathTemplate& parsed ) const;

    /** Is this a sign-in, security or credential request (never-record rules of the settings) */
    bool IsNeverRecord( const CUrlPathTemplate& parsed ) const;

    bool IsNeverRecordUrl( const CORE::CString& url ) const;

    /** Is the value of a header, query item or body field with this name secret: the word rule of CORE::CRedactor or a secret name pattern */
    bool IsSecretName( const CORE::CString& name ) const;

    /** The name rule as a CORE::CRedactor, for CWebBodyShape */
    const CORE::CRedactor& GetNameRedactor( void ) const;

    /**
     *  What is kept of a request header at capture: the value of a secret header is registered and becomes the
     *  NotRecordedMarker, other values are kept as they are (the journal decides what it writes of them)
     */
    CORE::CString CaptureHeaderValue( const CORE::CString& lowerCaseName, const CORE::CString& value );

    /**
     *  Registers a secret value, and its JSON escaped and percent encoded forms, when IsPlausibleSecret() accepts it.
     *  A value it does not accept is counted (GetIgnoredSecretCount()) and not looked for.
     */
    void RegisterSecret( const CORE::CString& value, ESecretClass secretClass = SECRET_CLASS_OTHER );

    /** Is this value one that the tripwire looks for: the length, token and entropy rules of the settings, see the class description */
    bool IsPlausibleSecret( const CORE::CString& value ) const;

    static CORE::CString SecretClassToString( ESecretClass secretClass );

    /** "cookie: 2, query-value: 1", the classes in alphabetical order; empty for no counts */
    static CORE::CString FormatClassCounts( const TClassCountMap& counts );

    /** Registers the values of the secret query items of a URL */
    void RegisterQuerySecrets( const CORE::CString& url );

    /** Registers the values of the secret fields of a JSON or form body; other bodies are not looked into */
    void RegisterBodySecrets( const CORE::CString& contentType, const CORE::CString& body, ESecretClass secretClass = SECRET_CLASS_REQUEST_BODY_FIELD );

    /** Registers the value of a Set-Cookie header (the part after name= up to ';') */
    void RegisterSetCookie( const CORE::CString& headerValue );

    UInt32 GetKnownSecretCount( void ) const;

    /** How many registered values were not accepted by IsPlausibleSecret() since the registry was cleared: not looked for, still redacted where they stand */
    UInt32 GetIgnoredSecretCount( void ) const;

    /** Forgets the secrets registered so far, and draws a new hash key */
    void ClearKnownSecrets( void );

    /** Does the text contain a registered secret value? */
    bool ContainsKnownSecret( const CORE::CString& text ) const;

    /** Does the text contain a registered secret value, and of which class is the first one found (the value is never given) */
    bool FindKnownSecret( const CORE::CString& text, ESecretClass& secretClass ) const;

    /** Replaces every registered secret value in the text by the SecretMarker */
    CORE::CString ScrubKnownSecrets( const CORE::CString& text ) const;

    /** A secret value as the journal and the report show it: {secret:<12 hex>} or {secret}, see the secretValueMode setting */
    CORE::CString RedactSecretValue( const CORE::CString& value ) const;

    /**
     *  The URL as the journal keeps it. Always without user info and fragment, secret query values redacted. Without
     *  example values the path is the template and query items keep their names only; never-record requests always so.
     */
    CORE::CString RedactUrl( const CORE::CString& url, bool neverRecorded ) const;

    /** A query value for an example: the value, or the redacted secret when the name is secret */
    CORE::CString RedactQueryValue( const CORE::CString& decodedName, const CORE::CString& value ) const;

    /** The value of a request header as the journal keeps it; empty means only the name */
    CORE::CString RedactHeaderValue( const CORE::CString& lowerCaseName, const CORE::CString& value ) const;

    /** A request body for an example (with example values only): secret field values and registered secrets replaced */
    CORE::CString RedactBody( const CORE::CString& body ) const;

    /**
     *  Is this query value enum-like, so that it may be recorded with recordEnumQueryValues: not empty, at most 32
     *  characters of [A-Za-z0-9_.,:+-], the name is not secret, the value is not token like (IsPlausibleSecret(), or a
     *  run of 8 or more letters and digits that mixes both), not a registered secret value, and not an identifier
     *  (a GUID, a long hex run, a generated id, a number of five digits or more: those stay templated)
     */
    bool IsEnumLikeQueryValue( const CORE::CString& decodedName, const CORE::CString& decodedValue ) const;

    /** The name and value of the enum-like query items of a URL, decoded, in order; nothing unless recordEnumQueryValues is on */
    void ExtractEnumQueryValues( const CUrlPathTemplate& parsed, GUCEF::vector< std::pair< CORE::CString, CORE::CString > >& result ) const;

    /** The names of the cookies of a Cookie header (name=value; name=value), never a value, in order, each once */
    static void ParseCookieNames( const CORE::CString& cookieHeader, CORE::CString::StringVector& names );

    /** The names of the cookies of a Set-Cookie header value (lines joined by a line end, name=value; attributes), never a value */
    static void ParseSetCookieNames( const CORE::CString& setCookieHeader, CORE::CString::StringVector& names );

    /** A cookie name as the journal and the report show it: identifier-like parts templated, a token-like name replaced, at most 64 characters */
    CORE::CString RedactCookieName( const CORE::CString& name ) const;

    /** A label of a clicked element: secrets and text that looks like one replaced, digit runs as {n}, at most 60 characters */
    CORE::CString RedactLabel( const CORE::CString& label ) const;

    private:

    void Rebuild( void );
    void RegisterJsonSecrets( const CORE::CDataNode& node, bool parentSecret, ESecretClass secretClass );
    void NewHashKey( void );

    typedef GUCEF::vector< CORE::CGlobPattern > TPatternVector;

    CSiteDiscoverySettings m_settings;
    CORE::CRedactor m_names;
    CORE::CRedactor m_text;
    TPatternVector m_neverRecordHosts;
    struct SKnownSecret
    {
        ESecretClass secretClass;
        bool wholeToken;            /**< only counts as a whole token, see the class description */
    };

    typedef GUCEF::map< CORE::CString, SKnownSecret > TSecretMap;

    TSecretMap m_secrets;
    UInt32 m_ignoredCount;
    UInt8 m_hashKey[ 32 ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CSITEDISCOVERYREDACTOR_H ? */
