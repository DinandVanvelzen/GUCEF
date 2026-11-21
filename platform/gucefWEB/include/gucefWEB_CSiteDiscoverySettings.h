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

#ifndef GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#define GUCEF_WEB_CSITEDISCOVERYSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

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
 *  The settings of site discovery: recording what a web site requests while a user browses it by hand, see
 *  CChromeNetworkRecorder, CSiteDiscoveryJournal, CSiteEndpointCatalog and CSiteDiscoverySession.
 *
 *  Every member is a configuration key of the same name (attributes or child values of one data node, lists as arrays
 *  or comma separated text). A list that is configured replaces the default list; an empty array gives an empty list.
 *  The privacy defaults are the safe ones: no example values, no saved bodies, the never-record lists filled.
 */
class GUCEF_WEB_PUBLIC_CPP CSiteDiscoverySettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 MinKnownSecretLengthFloor = 8;      /**< the lowest knownSecretMinLength that is accepted */

    enum ESecretValueMode
    {
        SECRET_VALUE_HASH  ,    /**< {secret:<12 hex>}, a keyed hash with a key that only exists in memory: equal values can be matched within a session */
        SECRET_VALUE_STRIP      /**< {secret} */
    };

    enum EBrowserRunMode
    {
        BROWSER_RUNMODE_ATTACH           ,   /**< a browser the user runs, with remote debugging allowed: never started or closed by us */
        BROWSER_RUNMODE_RECORDED_SESSION     /**< a browser started with a profile of its own, which the user drives by hand */
    };

    // what is recorded
    CORE::CString::StringVector hostFilter;                 /**< a host must contain one of these terms, case insensitive; empty: all hosts */
    CORE::CString::StringVector allowPathPrefixes;          /**< when not empty only paths starting with one of these are recorded */
    CORE::CString::StringVector neverRecordPathPatterns;    /**< parts of the lower cased path of sign-in, security and credential pages */
    CORE::CString::StringVector neverRecordHosts;           /**< host glob patterns of identity providers and sign-in hosts */
    bool noteNeverRecorded;                                 /**< note that a never-record request happened (method, host, path template, status); default true */

    // privacy
    CORE::CString::StringVector secretNamePatterns;         /**< glob patterns of secret header, query and field names on top of the CORE::CRedactor word rule */
    ESecretValueMode secretValueMode;                       /**< default SECRET_VALUE_HASH */
    bool recordExampleValues;                               /**< keep redacted example values in journal and report; private use only, default false */
    bool recordEnumQueryValues;                             /**< record the value of a query parameter when it is enum-like (at most 32 characters of [A-Za-z0-9_.,:+-], not a secret name, not token or id like), with counts per endpoint; default false */
    UInt32 knownSecretMinLength;                            /**< the tripwire only looks for registered secret values of at least this many bytes, default 12, minimum MinKnownSecretLengthFloor (8); see CSiteDiscoveryRedactor */
    UInt32 knownSecretMinEntropyBits;                       /**< and with at least this much Shannon entropy in total (bits), default 32; 0: no entropy rule */
    CORE::CString::StringVector ignoredSecretFields;        /**< glob patterns (case insensitive) of field, query and header names whose values the report check never looks for; not cookies, Authorization or password fields; default empty */
    UInt32 tripwireMaxLineShare;                            /**< percent: a registered value that is not a credential and appears in more of the journal lines than this is taken for a public value and not looked for in the report; default 20, 0: off */

    // values where structure is expected: member names, path segments, query names and values, labels, file names
    bool maskIdLikeMemberNames;                             /**< JSON member names that are data (digit runs, GUIDs, hex runs, generated ids, token-like names) are shown as {key}, so a map keyed by identifiers gives one line; default true */
    bool maskTokenLikeParameterNames;                       /**< query parameter names that look like an encoded token (a plausible secret, a run of base64 characters mixing cases and digits, or longer than maxParameterNameLength) are shown as {token-like name}; default true */
    UInt32 maxParameterNameLength;                          /**< see maskTokenLikeParameterNames; default 40 */
    CORE::CString::StringVector valueQueryNames;            /**< query parameters whose values are free text of the user (a search), never recorded, also not with example or enum values: {value}; case insensitive; default q, query, search, searchterm, term, keyword, keywords, text */
    CORE::CString::StringVector enumQueryNames;             /**< when not empty, recordEnumQueryValues records the values of these parameters only (case insensitive); default empty: of every parameter */
    CORE::CString::StringVector valuePathSegmentsAfter;     /**< a path segment that follows a segment of one of these names is a value of the user: {value}; case insensitive; default search, lookup, autocomplete, suggest, typeahead */
    bool maskUppercaseTokens;                               /**< treat short tokens of capital letters (1 to uppercaseTokenMaxLength letters, such as codes and symbols, also with a class suffix such as XY.Z) as values: {value} in labels, path segments, query values and file names, {key} as member names; the words of nonSensitiveValues excepted; default false */
    UInt32 uppercaseTokenMaxLength;                         /**< see maskUppercaseTokens; default 5 */
    CORE::CString::StringVector nonSensitiveValues;         /**< words maskUppercaseTokens leaves alone, case insensitive; default OK, ID, PDF, CSV, TXT, XLS, XLSX, JSON, XML, HTML, URL, API, FAQ, AM, PM, UTC, GMT */
    CORE::CString::StringVector sensitiveValues;            /**< a dictionary of values of the user (names, labels, codes) shown as {value} or {key} wherever they occur as whole words, case insensitive; default empty */
    CORE::CString::StringVector sensitiveValuePatterns;     /**< glob patterns of tokens (runs of letters and digits) treated the same way, case insensitive; default empty */
    bool maskFileNameWords;                                 /**< in the names of downloaded files, the words between two masked parts ({n}, {value}) become {text}; default true */

    // bodies
    bool recordResponseShapes;                              /**< describe the data responses of each endpoint: the first and every later one that adds something (see maxShapesPerEndpoint), default true */
    bool saveResponseBodies;                                /**< save response bodies as owner-only files, default false */
    UInt64 maxResponseBodyBytes;                            /**< larger responses are not saved or shaped; 0: no maximum. Default 4 MiB */
    CORE::CString::StringVector responseBodyContentTypes;   /**< parts of the content types of bodies that are saved: json, xml, text/plain */

    // output
    CORE::CString outputDirectory;                          /**< folder of journal, report and bodies, owner-only */
    CORE::CString journalFileName;                          /**< default requests.jsonl */
    CORE::CString reportFileName;                           /**< default endpoints.md */
    CORE::CString bodiesDirectoryName;                      /**< default bodies */
    bool continueJournal;                                   /**< a session reads the existing journal first, default true */

    // behaviour towards the browser
    bool disableBrowserCache;                               /**< Network.setCacheDisabled, default false so the recorder only listens */
    bool reportTableCellControlNames;                       /**< label a click on a control inside a table cell with the accessible name of the control (the plain text of a cell is never recorded), default true; false: such a click is recorded by its role only, as <button in table> */
    bool captureClicks;                                     /**< install a passive click and submit listener in watched pages, default false */

    // grouping of requests by user action
    UInt32 actionWindowMs;                                  /**< default 3000 */
    UInt32 actionMaxDurationMs;                             /**< default 15000; 0: no maximum */
    UInt32 quietGapMs;                                      /**< default 1500 */
    UInt32 maxActions;                                      /**< actions kept by the catalog, default 500; 0: no maximum */

    // shapes
    UInt32 maxShapeDepth;                                   /**< default 5 */
    UInt32 maxShapeLines;                                   /**< default 150 */
    UInt32 maxShapesPerEndpoint;                            /**< responses of one endpoint whose shape is merged into the endpoint's union shape: one is kept when it adds a path, a type or a first non-empty array; default 12; 0: no maximum */

    // the browser
    EBrowserRunMode browserRunMode;                         /**< default BROWSER_RUNMODE_ATTACH */
    UInt16 browserDebugPort;                                /**< default 9222 */
    CORE::CString::StringVector browserUserDataDirs;        /**< profile folders searched for DevToolsActivePort; empty: the default ones */
    CORE::CString browserExecutablePath;                    /**< empty: searched */
    CORE::CString browserProfileDirectory;                  /**< empty: <outputDirectory>/browser-profile */
    CORE::CString browserStartUrl;
    bool browserHeadless;                                   /**< for tests only, default false */
    UInt32 browserConnectTimeoutMs;                         /**< default 90000: Chrome asks the user to allow the connection */
    bool closeLaunchedBrowser;                              /**< default false; an attached browser is never closed */

    CSiteDiscoverySettings( void );

    CSiteDiscoverySettings( const CSiteDiscoverySettings& src );

    virtual ~CSiteDiscoverySettings() GUCEF_VIRTUAL_OVERRIDE;

    CSiteDiscoverySettings& operator=( const CSiteDiscoverySettings& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The default never-record path patterns: generic sign-in, security and credential paths */
    static CORE::CString::StringVector GetDefaultNeverRecordPathPatterns( void );

    /** The default never-record host patterns: identity providers and sign-in hosts */
    static CORE::CString::StringVector GetDefaultNeverRecordHosts( void );

    /** The default secret name patterns on top of the word rule of CORE::CRedactor */
    static CORE::CString::StringVector GetDefaultSecretNamePatterns( void );

    static CORE::CString::StringVector GetDefaultValueQueryNames( void );
    static CORE::CString::StringVector GetDefaultValuePathSegmentsAfter( void );
    static CORE::CString::StringVector GetDefaultNonSensitiveValues( void );

    static CORE::CString SecretValueModeToString( ESecretValueMode mode );
    static bool SecretValueModeFromString( const CORE::CString& text, ESecretValueMode& mode );
    static CORE::CString BrowserRunModeToString( EBrowserRunMode mode );
    static bool BrowserRunModeFromString( const CORE::CString& text, EBrowserRunMode& mode );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CSITEDISCOVERYSETTINGS_H ? */
