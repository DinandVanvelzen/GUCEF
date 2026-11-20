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

#ifndef GUCEF_WEB_COBSERVEDUSERACTION_H
#include "gucefWEB_CObservedUserAction.h"
#define GUCEF_WEB_COBSERVEDUSERACTION_H
#endif /* GUCEF_WEB_COBSERVEDUSERACTION_H ? */

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
 *  after their scheme, the values of secret headers, query items and body fields. The journal never loses a line to it:
 *  while the journal writes (SetJournalMode()) only the values of the credential classes (cookie and credential-header
 *  values, password fields) are masked where they stand, and the value rules are off, so the journal keeps what was
 *  observed. When the report is made a report part in which a registered value occurs is replaced by a placeholder that
 *  names the value by its class, field name, length and entropy, never by the value, see FindKnownSecret() and
 *  DescribeSecret(). The registry and the key of the value hashes exist in memory only.
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
 *  after it, a percent encoded or backslash escaped neighbour counts as a boundary); longer values are found anywhere,
 *  except the values of body fields, which always count as whole tokens only.
 *  Public values are not looked for either: a value that looks like a URL, a path or a host name (its path segments and
 *  query values are looked at one by one instead), a value of a field of ignoredSecretFields, a value that was seen in
 *  the host, a path segment or the value of a query item with a name that is not secret of a request URL (NotePublicUrl(),
 *  public because it is in an address bar) and, at report time, a value that appears in more than tripwireMaxLineShare
 *  percent of the journal lines (AssessJournal()). Credentials (cookie and credential-header values, password fields) are
 *  never taken for public values.
 *  Values that are not looked for are still redacted where they stand, by the name rules and the query, header and body
 *  handling: only this last line of defence skips them.
 *
 *  The value rules mask the data of the user that sits where a report expects structure (see the settings of the same
 *  names): member names of JSON bodies that are identifiers, tokens or values of the user ({key}); query parameter names
 *  that look like encoded tokens ({token-like name}); the values of free text query parameters, path segments after a
 *  search segment, and values of the dictionary, the patterns and (when switched on) the short capital tokens, in labels,
 *  paths, query values and file names ({value}); the words between masked parts of a file name ({text}). They are applied
 *  when a journal line is written (CSiteDiscoveryJournal) and again when an observation enters a catalog
 *  (CSiteEndpointCatalog), so a report rebuilt from an older journal follows the rules of today; RedactObservation() and
 *  RedactAction() tell whether they changed anything, which the scrub of a journal uses. The rules are idempotent: a
 *  placeholder ({...}) is never masked again.
 *
 *  The recorder, the journal and the catalog of one session share one redactor (SetRedactor() of each), so what the
 *  recorder registered is known when the journal and the report are written. Not thread-safe: use it on one thread.
 */
class GUCEF_WEB_PUBLIC_CPP CSiteDiscoveryRedactor
{
    public:

    static const CORE::CString NotRecordedMarker;       /**< "<not recorded>": the value of a secret header */
    static const CORE::CString SecretMarker;            /**< "{secret}": a secret value in strip mode and in bodies */
    static const CORE::CString ValueMarker;             /**< "{value}": a value of the user in a label, path, query or file name */
    static const CORE::CString KeyMarker;               /**< "{key}": a member name that is data */
    static const CORE::CString TokenLikeNameMarker;     /**< "{token-like name}": a parameter or cookie name that looks like an encoded token */
    static const CORE::CString TextMarker;              /**< "{text}": the words between masked parts of a file name */
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

    /** What is known of a registered value, for diagnostics: never the value itself */
    struct SSecretInfo
    {
        ESecretClass secretClass;
        CORE::CString fieldName;            /**< the field, query item or header it came from; empty when not known */
        UInt32 length;                      /**< bytes */
        UInt32 entropyBits;                 /**< Shannon entropy over the whole value */
        bool credential;                    /**< a cookie or credential-header value or a password field: always looked for, also while the journal writes */
        bool publicValue;                   /**< seen in a request URL or host: not looked for */
        bool suspectedPublic;               /**< in more than tripwireMaxLineShare percent of the journal lines: not looked for */
        UInt32 journalLines;                /**< the journal lines it appears in, as AssessJournal() counted them */
        GUCEF::vector< UInt32 > firstJournalLines;  /**< the first of those line numbers (1 based), at most MaxListedJournalLines */

        SSecretInfo( void );
    };

    static const UInt32 MaxListedJournalLines = 10;

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
     *  A value it does not accept, a value of a field of ignoredSecretFields and a URL, path or host name (whose path
     *  segments and query values are registered one by one instead) are counted (GetIgnoredSecretCount()) and not looked
     *  for. fieldName is the name of the field, query item or header, for the diagnostics and the password rule.
     */
    void RegisterSecret( const CORE::CString& value, ESecretClass secretClass = SECRET_CLASS_OTHER, const CORE::CString& fieldName = CORE::CString::Empty );

    /** Is a value of a field of this name a credential: a password, passphrase or passcode field */
    static bool IsPasswordFieldName( const CORE::CString& name );

    /** Is the value of a field of this name never looked for (ignoredSecretFields) */
    bool IsIgnoredSecretField( const CORE::CString& name ) const;

    /**
     *  Notes the parts of a request URL that anyone sees in an address bar: the host, the path segments and the values of
     *  query items whose name is not secret. A registered value that is not a credential and equals one of them, or stands
     *  in a host, is a public value from then on and not looked for. Not for never-record requests
     */
    void NotePublicUrl( const CORE::CString& url );

    /** The same for a host alone */
    void NotePublicHost( const CORE::CString& host );

    /**
     *  While the journal writes a line: only the values of the credential classes are looked for and the value rules are
     *  off, so a journal line holds what was observed with only cookie, credential-header and password values masked
     */
    void SetJournalMode( bool journalMode );

    bool IsJournalMode( void ) const;

    /**
     *  Counts in which lines of the journal each registered value appears and takes a value that is not a credential and
     *  appears in more than tripwireMaxLineShare percent of them for a public value (suspectedPublic); logs each such value
     *  by its class, field, length and entropy, never the value. The report uses the counts in its diagnostics.
     *  @return the number of values taken for public values
     */
    UInt32 AssessJournal( const CORE::CString& journalText );

    /** The number of lines AssessJournal() read */
    UInt32 GetAssessedJournalLineCount( void ) const;

    UInt32 GetSecretInfoCount( void ) const;

    const SSecretInfo& GetSecretInfo( UInt32 index ) const;

    /** "class response-body-field, field `token`, length 17, entropy 52 bits", never the value */
    CORE::CString DescribeSecret( UInt32 index ) const;

    /**
     *  The diagnostics of a registered value for a report or a log: DescribeSecret(), the journal lines it appears in and,
     *  when that is more than 5 percent of them, the advice that it is probably not a secret
     */
    CORE::CString DescribeSecretUse( UInt32 index ) const;

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

    /** The same, giving the index of the value for GetSecretInfo() */
    bool FindKnownSecretIndex( const CORE::CString& text, UInt32& index ) const;

    /** Replaces every registered secret value in the text by the SecretMarker */
    CORE::CString ScrubKnownSecrets( const CORE::CString& text ) const;

    /** A secret value as the journal and the report show it: {secret:<12 hex>} or {secret}, see the secretValueMode setting */
    CORE::CString RedactSecretValue( const CORE::CString& value ) const;

    /**
     *  The URL as the journal keeps it. Always without user info and fragment, secret query values redacted, the value
     *  rules applied (MaskUrl()). Without example values the path is the template and query items keep their names
     *  only; never-record requests always so.
     */
    CORE::CString RedactUrl( const CORE::CString& url, bool neverRecorded ) const;

    /** A query value for an example: the redacted secret when the name is secret, {value} for a value name or a value of the user, else the value */
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

    /** The name and value of the enum-like query items of a URL, decoded, in order, for the names IsEnumQueryName() accepts and with values of the user as {value}; nothing unless recordEnumQueryValues is on */
    void ExtractEnumQueryValues( const CUrlPathTemplate& parsed, GUCEF::vector< std::pair< CORE::CString, CORE::CString > >& result ) const;

    /** The names of the cookies of a Cookie header (name=value; name=value), never a value, in order, each once */
    static void ParseCookieNames( const CORE::CString& cookieHeader, CORE::CString::StringVector& names );

    /** The names of the cookies of a Set-Cookie header value (lines joined by a line end, name=value; attributes), never a value */
    static void ParseSetCookieNames( const CORE::CString& setCookieHeader, CORE::CString::StringVector& names );

    /** A cookie name as the journal and the report show it: identifier-like parts templated, a token-like name replaced, at most 64 characters */
    CORE::CString RedactCookieName( const CORE::CString& name ) const;

    /** A label of a clicked element: secrets and text that looks like one replaced, values of the user as {value}, digit runs as {n}, at most 60 characters */
    CORE::CString RedactLabel( const CORE::CString& label ) const;

    /**
     *  Is this token (a run of letters and digits) a value of the user: a word of the dictionary (sensitiveValues), a
     *  match of a pattern (sensitiveValuePatterns), or with maskUppercaseTokens a short run of capital letters that is
     *  not one of the nonSensitiveValues
     */
    bool IsSensitiveToken( const CORE::CString& token ) const;

    /** Does a value (a member name, a path segment, a query value) hold a value of the user, as MaskValues() sees it */
    bool IsSensitiveValue( const CORE::CString& value ) const;

    /** The text with the values of the user replaced by {value}: dictionary entries as whole words, sensitive tokens; text in braces is left as it is */
    CORE::CString MaskValues( const CORE::CString& text ) const;

    /** A JSON member name as a report shows it: {key} for identifiers, tokens, secrets and values of the user (see maskIdLikeMemberNames) */
    CORE::CString RedactMemberName( const CORE::CString& name ) const;

    /** A field path of a body shape ("items[].prices.ABC: number") with each member name through RedactMemberName() */
    CORE::CString RedactShapePath( const CORE::CString& path ) const;

    /** A line of a body shape: its path, and its sample value or URL template; see CWebBodyShape */
    CORE::CString RedactShapeLine( const CORE::CString& line ) const;

    /** Applies RedactShapeLine() to the lines, paging fields and URL fields of a shape of one body, dropping the lines that became the same; true when it changed */
    bool RedactShape( CWebBodyShape& shape ) const;

    /** A path (or a path template) with the segments that are values of the user as {value}: after a valuePathSegmentsAfter segment, or a sensitive value */
    CORE::CString RedactPath( const CORE::CString& path ) const;

    /** "scheme://host/path" or "host/path" (a page or redirect template) with its path through RedactPath() */
    CORE::CString RedactAddressTemplate( const CORE::CString& address ) const;

    /** Does a query parameter name look like an encoded token, see maskTokenLikeParameterNames; secrets always do */
    bool IsTokenLikeParameterName( const CORE::CString& decodedName ) const;

    /** A query parameter name as a report shows it: {token-like name} or the name */
    CORE::CString RedactQueryName( const CORE::CString& decodedName ) const;

    /** Is this a parameter whose values are free text of the user (valueQueryNames) */
    bool IsValueQueryName( const CORE::CString& decodedName ) const;

    /** May recordEnumQueryValues record the values of this parameter (not a value or secret name, in enumQueryNames when that is set) */
    bool IsEnumQueryName( const CORE::CString& decodedName ) const;

    /** The URL with the value rules applied, still a URL: path segments, query names and query values; without user info and fragment */
    CORE::CString MaskUrl( const CORE::CString& url ) const;

    /**
     *  The name of a downloaded file: values of the user as {value}, digit runs as {n}, and with maskFileNameWords the
     *  words between masked parts as {text}. A name holding a registered secret is returned whole, for the tripwire.
     */
    CORE::CString RedactFileName( const CORE::CString& fileName ) const;

    /** Drops the enum query values of names that may not be recorded and masks names and values; true when it changed */
    bool RedactEnumValues( CObservedWebRequest::TStringPairVector& values ) const;

    /** Applies the value rules to everything of an observation that reaches a journal or report; true when it changed */
    bool RedactObservation( CObservedWebRequest& observation ) const;

    /** Applies the value rules to the label and page of a user action; true when it changed */
    bool RedactAction( CObservedUserAction& action ) const;

    private:

    void Rebuild( void );
    void RegisterJsonSecrets( const CORE::CDataNode& node, bool parentSecret, const CORE::CString& secretFieldName, ESecretClass secretClass );
    void RegisterUrlParts( const CORE::CString& value, ESecretClass secretClass, const CORE::CString& fieldName );
    bool IsLookedFor( UInt32 index ) const;
    bool IsPublicText( const CORE::CString& value ) const;
    void NotePublicPart( const CORE::CString& part );
    void NewHashKey( void );

    typedef GUCEF::vector< CORE::CGlobPattern > TPatternVector;

    CORE::CString MaskDictionary( const CORE::CString& text, const CORE::CString& marker ) const;
    bool IsInList( const CORE::CString::StringVector& list, const CORE::CString& value ) const;

    CSiteDiscoverySettings m_settings;
    CORE::CRedactor m_names;
    CORE::CRedactor m_text;
    TPatternVector m_neverRecordHosts;
    TPatternVector m_valuePatterns;             /**< sensitiveValuePatterns, lower case */
    CORE::CString::StringVector m_dictionary;   /**< sensitiveValues, lower case, longer first */
    typedef GUCEF::map< CORE::CString, UInt32 > TSecretMap;      /**< a registered value and its escaped forms: the index of its SSecretInfo */
    typedef GUCEF::vector< SSecretInfo > TSecretInfoVector;
    typedef GUCEF::set< CORE::CString > TStringSet;

    TSecretMap m_secrets;
    TSecretInfoVector m_secretInfos;
    TPatternVector m_ignoredFields;             /**< ignoredSecretFields, lower case */
    TStringSet m_publicParts;                   /**< hosts, path segments and query values of request URLs */
    TStringSet m_publicHosts;
    UInt32 m_ignoredCount;
    UInt32 m_assessedLines;
    bool m_journalMode;
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
