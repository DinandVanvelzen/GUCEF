/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CWEBLOGINSCRIPT_H
#define GUCEF_ASSETSTORE_CWEBLOGINSCRIPT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_ASSETSTORE_CWEBLOGINSCRIPTSTEP_H
#include "gucefASSETSTORE_CWebLoginScriptStep.h"
#define GUCEF_ASSETSTORE_CWEBLOGINSCRIPTSTEP_H
#endif /* GUCEF_ASSETSTORE_CWEBLOGINSCRIPTSTEP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How to sign in on the web site of a store, as data (section 12.1 of the design): a JSON file in the data folder of
 *  the store's plugin, so a changed login page needs no new build. CWebSessionSignIn carries it out in a browser.
 *
 *  A script holds selectors, addresses, texts and a fixed vocabulary of steps (CWebLoginScriptStep), never code and
 *  never a value to type: a `fill` step names a credential slot, whose value CWebSessionSignIn takes from the saved
 *  credentials of the store (CWebSignInCredentials). The page is only ever asked fixed, built-in questions
 *  (CWebPageScriptHelpers) in which the selectors and texts are escaped as string literals.
 *
 *  @code
 *  {
 *    "format": "gucef.webLoginScript",
 *    "version": 1,
 *    "name": "Example store",
 *    "startUrl": "https://shop.example.com/login",
 *    "cookieDomains": [ "shop.example.com" ],
 *    "allowedHosts": [ "accounts.example.com" ],
 *    "credentials": [ "username", "password" ],
 *    "signedIn": { "selector": "#account-menu", "cookie": "session_id" },
 *    "sessionEnded": { "statusCodes": [ 401 ], "redirectPattern": "https://shop.example.com/login*" },
 *    "stepTimeoutMs": 15000,
 *    "steps": [
 *      { "action": "click", "selector": "#accept-cookies", "optional": true, "timeoutMs": 3000 },
 *      { "action": "fill", "selector": "input[name=email]", "credential": "username" },
 *      { "action": "fill", "selector": "input[name=password]", "credential": "password" },
 *      { "action": "submit", "selector": "form#login" },
 *      { "action": "failCheck", "selector": ".login-error", "reason": "The store refused the e-mail address or password" },
 *      { "action": "personCheck", "kind": "captcha", "selector": "iframe[src*=captcha]", "prompt": "Solve the puzzle in the browser window" },
 *      { "action": "personCheck", "kind": "code", "selector": "input[name=code]", "prompt": "Type the code the store sent to your e-mail address" },
 *      { "action": "waitForSignedIn", "timeoutMs": 30000 }
 *    ]
 *  }
 *  @endcode
 *
 *  Rules checked by Validate() (and by every Load...()): the format name and version 1; http or https addresses
 *  without user information whose host is one of the cookie domains or allowed hosts (or below one of them); at least
 *  one cookie domain; a signed in condition; credential slots declared before use; lengths, counts and time limits;
 *  no control characters. Unknown keys are refused, with a message that says what is wrong.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CWebLoginScript
{
    public:

    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    static const CORE::CString FormatName;          /**< "gucef.webLoginScript" */
    static const UInt32 CurrentVersion;             /**< 1 */
    static const UInt32 MaxSelectorLength;          /**< 512 bytes */
    static const UInt32 MaxTextLength;              /**< 256 bytes: names, texts, reasons */
    static const UInt32 MaxUrlLength;               /**< 2048 bytes */
    static const UInt32 MaxPromptLength;            /**< 512 bytes */
    static const UInt32 MaxSteps;                   /**< 64 */
    static const UInt32 MaxDomains;                 /**< 16 cookie domains, 16 allowed hosts */
    static const UInt32 MaxCredentialSlots;         /**< 8 */
    static const UInt32 MaxTimeoutInMs;             /**< 600000: ten minutes */
    static const UInt32 MaxDelayInMs;               /**< 10000 */
    static const UInt32 DefaultStepTimeoutInMs;     /**< 15000 */
    static const UInt32 MaxScriptBytes;             /**< 65536: a larger file is not read */

    UInt32 version;
    CORE::CString name;                             /**< the name of the store or site, for messages */
    CORE::CString comment;
    CORE::CString startUrl;                         /**< the login page */
    CORE::CString::StringVector cookieDomains;      /**< the cookies of these domains and below are the session */
    CORE::CString::StringVector allowedHosts;       /**< further hosts the browser may be sent to, for example of a sign in service */
    CORE::CString::StringVector credentialSlots;    /**< the names of the credential slots that fill steps use */
    CWebLoginScriptCondition signedIn;              /**< what the page shows once signed in */
    TUInt32Vector sessionEndedStatusCodes;          /**< answers that mean the session ended, 401 when none are given */
    CORE::CString sessionEndedRedirectPattern;      /**< a redirect to an address like this means the session ended */
    UInt32 stepTimeoutInMs;
    TWebLoginScriptStepVector steps;

    CWebLoginScript( void );

    void Clear( void );

    /** Reads the script strictly and validates it; false with the problems when it cannot be used */
    bool LoadFromDataNode( const CORE::CDataNode& node, CORE::CString::StringVector& problems );

    void SaveToDataNode( CORE::CDataNode& node ) const;

    /** Reads JSON text (through the "json" DStore codec, dstorepluginJSONPARSER) strictly and validates it */
    bool LoadFromText( const CORE::CString& jsonText, CORE::CString::StringVector& problems );

    bool SaveToText( CORE::CString& jsonText ) const;

    /** Reads a JSON file of at most MaxScriptBytes strictly and validates it; the path may hold path variables */
    bool LoadFromFile( const CORE::CString& path, CORE::CString::StringVector& problems );

    bool SaveToFile( const CORE::CString& path ) const;

    /** Checks every rule of the class comment; the problems say where and what */
    bool Validate( CORE::CString::StringVector& problems ) const;

    /** Whether the host (lower case, no port) is a cookie domain or allowed host, or below one */
    bool IsHostAllowed( const CORE::CString& host ) const;

    /** Whether the address is http or https, without user information, to an allowed host */
    bool IsUrlAllowed( const CORE::CString& url ) const;

    /** Whether a cookie of the domain (as a browser gives it, maybe with a leading dot) belongs to the store */
    bool IsStoreCookieDomain( const CORE::CString& cookieDomain ) const;

    /** Whether a step fills a credential */
    bool NeedsCredentials( void ) const;

    /** The time limit of the step: its own, else the step timeout of the script */
    UInt32 GetStepTimeout( const CWebLoginScriptStep& step ) const;

    /** The problems as one text, one per line, for a log line or a message */
    static CORE::CString DescribeProblems( const CORE::CString::StringVector& problems );

    // Helpers shared with the steps and conditions

    /** Reads a text attribute; a value that is there but no text is a problem */
    static bool ReadText( const CORE::CDataNode& node, const CORE::CString& key, const CORE::CString& where, CORE::CString& value, CORE::CString::StringVector& problems );

    /** Reads a whole number from 0 up; another value is a problem */
    static bool ReadUInt32( const CORE::CDataNode& node, const CORE::CString& key, const CORE::CString& where, UInt32& value, CORE::CString::StringVector& problems );

    /** Reads a switch (true or false); another value is a problem */
    static bool ReadBool( const CORE::CDataNode& node, const CORE::CString& key, const CORE::CString& where, bool& value, CORE::CString::StringVector& problems );

    /** Checks the length and that there are no control characters; problems are added */
    static bool CheckText( const CORE::CString& text, const CORE::CString& what, UInt32 maxLength, CORE::CString::StringVector& problems );

    /** The token characters of RFC 6265, at most 128 */
    static bool IsValidCookieName( const CORE::CString& name );

    /** A letter, then letters, digits, '_' or '-', at most 32 */
    static bool IsValidCredentialSlotName( const CORE::CString& name );

    /** A host name or IPv4 address in lower case without a leading dot; "localhost" is the only name without a dot */
    static bool NormaliseDomain( const CORE::CString& domain, CORE::CString& normalised );

    /** The host of an http or https address in lower case without the port; empty for another scheme, user information or IPv6 */
    static CORE::CString HostOfUrl( const CORE::CString& url );

    /** Whether the host is the domain or below it */
    static bool IsHostWithin( const CORE::CString& host, const CORE::CString& domain );

    private:

    bool ReadTextList( const CORE::CDataNode& node, const CORE::CString& key, UInt32 maxCount, CORE::CString::StringVector& values, CORE::CString::StringVector& problems ) const;
    bool ValidateUrl( const CORE::CString& url, const CORE::CString& where, CORE::CString::StringVector& problems ) const;
    bool ValidateStep( const CWebLoginScriptStep& step, const CORE::CString& where, CORE::CString::StringVector& problems ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CWEBLOGINSCRIPT_H ? */
