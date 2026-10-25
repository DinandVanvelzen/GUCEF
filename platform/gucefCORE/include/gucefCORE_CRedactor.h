/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CREDACTOR_H
#define GUCEF_CORE_CREDACTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What replaces everything that is redacted, unless CRedactor::SetMarker() says otherwise */
#define GUCEF_CORE_REDACTION_MARKER     "***"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A set of ERedactionFlag values or-ed together: what a CRedactor redacts and how it sanitizes what it keeps */
typedef UInt32 TRedactionFlags;

/**
 *  What a CRedactor can redact. Every flag says what is replaced by the marker and what is kept, with an example
 *  for the default marker "***". The URL flags apply to RedactUrl(), to the URLs of the other functions and
 *  to the URLs that RedactText() finds. A flag that is not set leaves that part as it is.
 */
enum ERedactionFlag
{
    REDACT_NONE                     = 0,

    /// URL: the user info. "http://user:pw@host/" becomes "http://***@host/". An '@' in the password is no problem, the last '@' of the authority ends the user info
    REDACT_URL_USERINFO             = 0x00000001,

    /// URL: every value of the query. "?key=abc&appid=7" becomes "?key=***&appid=***". Names are kept, an item without a name/value split is a value and is replaced whole: "?token" becomes "?***"
    REDACT_URL_QUERY_VALUES         = 0x00000002,

    /// URL: the values of the query items of which the name is sensitive: IsSensitiveName() says so or it matches AddQueryName(). "?access_token=abc&page=2" becomes "?access_token=***&page=2". Not needed when REDACT_URL_QUERY_VALUES is set.
    REDACT_URL_QUERY_LISTED         = 0x00000004,

    /// URL: the fragment. "http://host/p#access_token=abc" becomes "http://host/p#***"
    REDACT_URL_FRAGMENT             = 0x00000008,

    /// URL: the path segments that match a pattern of AddPathSegment(). With the pattern "reset-*": "/users/reset-9f3a/confirm" becomes "/users/***/confirm". Percent encoding is undone for the comparison, the output keeps the original.
    REDACT_URL_PATH_SEGMENTS        = 0x00000010,

    /// URL: a host which is not a valid host name, IPv4 address, [IPv6] address or one of those with ":port", together with the path, since it is then not certain that it holds no credentials. "http://user:pa/ss@host/x" becomes "http://***/***". Non ASCII bytes are accepted in a host name when REDACT_SANITIZE_NON_ASCII is not set.
    REDACT_URL_INVALID_HOST         = 0x00000020,

    /// Headers: the value of a header of which the name is sensitive, see IsSensitiveName(). "Authorization: Bearer abc" becomes "Authorization: ***"
    REDACT_HEADER_VALUES            = 0x00000040,

    /// Headers: the URLs of a header block: the target of the request line and the values of the headers that hold a URL (Location, Content-Location, Referer, Origin, Refresh, Link, SourceMap, X-SourceMap) are redacted as URLs, with the URL flags. "GET /api?key=abc HTTP/1.1" becomes "GET /api?key=*** HTTP/1.1"
    REDACT_HEADER_URLS              = 0x00000080,

    /// Headers: a line of a header block which is not a "name: value" line (and not the start line), since it is not known to be harmless. "not a header" becomes "***"
    REDACT_HEADER_UNPARSEABLE_LINES = 0x00000100,

    /// Names: whether IsSensitiveName() applies its own rule: names which contain auth, cookie, token, secret, passw, pwd, key, credential, session, signature, bearer or jwt, and the names pw, sig, sid and otp. Without it only the names of AddSensitiveName() count.
    REDACT_NAME_WORDS               = 0x00000200,

    /// Text: URLs inside free text, which are redacted with the URL flags. "see http://h/p?k=abc." becomes "see http://h/p?k=***." The URL ends at a space, a control character or one of " ' < > `
    REDACT_TEXT_URLS                = 0x00000400,

    /// Text: the credential after Bearer, Basic, Digest, Negotiate or NTLM, any case. "Authorization: Bearer abc.def" becomes "Authorization: Bearer ***". For Basic, Negotiate and NTLM only a credential that looks like encoded data (8 characters or more, with a digit or an inner capital or one of = + /) is replaced, so that "basic setup" stays. For Digest everything after it up to the end of the line is replaced when it holds a name=value pair.
    REDACT_TEXT_AUTH_SCHEMES        = 0x00000800,

    /// Text: the value of a name=value, name: value or "name":"value" pair of which the name is sensitive, see IsSensitiveName(). "password=hunter2 user=bob" becomes "password=*** user=bob", "Cookie: a=1; b=2" becomes "Cookie: ***", {"token":"abc"} becomes {"token":"***"}. A value after ':' goes to the end of the line (to the next ',' '}' or ']' when the name is quoted), a value after '=' to the next white space; a quoted value goes to its closing quote.
    REDACT_TEXT_KEY_VALUES          = 0x00001000,

    /// Text: runs of token like characters (letters, digits and + / _ -, with the = of base64 padding) of at least GetTokenMinLength() characters (32 unless set otherwise) that are all hexadecimal, or have a digit and a letter (and, when they hold a '/', both lower and upper case, so that a path is not one). A hex digest or a base64 key becomes "***", "sha=0123..." becomes "sha=***". A long identifier that holds a digit looks like that too and is replaced as well.
    REDACT_TEXT_TOKEN_RUNS          = 0x00002000,

    /// Text: e-mail addresses. "mail bob.smith@example.com now" becomes "mail *** now"
    REDACT_TEXT_EMAILS              = 0x00004000,

    /// Text: IPv4 addresses (1.2.3.4) and IPv6 addresses (a run of hex groups with "::" or six or more groups). "from 192.168.1.20:80" becomes "from ***:80". A version number with four parts looks like an address too.
    REDACT_TEXT_IPS                 = 0x00008000,

    /// Text: runs of 13 to 19 digits, which may be separated by single spaces or '-', that pass the Luhn check, as payment card numbers do. "card 4111 1111 1111 1111" becomes "card ***"
    REDACT_TEXT_CARD_NUMBERS        = 0x00010000,

    /// Output: control characters (below 0x20 and 0x7F) of what is kept are replaced by '?' so that the result is one line which cannot fake log lines. NUL is replaced always. The line ends of a header block are not part of what is replaced.
    REDACT_SANITIZE_CONTROL         = 0x00100000,

    /// Output: bytes of 0x80 and above of what is kept are replaced by '?' so that the result is plain ASCII. Without it text in UTF-8 is kept, and a cut at the maximum length is made between characters.
    REDACT_SANITIZE_NON_ASCII       = 0x00200000,

    /// The flags that apply to URLs
    REDACT_URL_ALL                  = REDACT_URL_USERINFO | REDACT_URL_QUERY_VALUES | REDACT_URL_QUERY_LISTED | REDACT_URL_FRAGMENT | REDACT_URL_PATH_SEGMENTS | REDACT_URL_INVALID_HOST,

    /// The flags that apply to header blocks and header values
    REDACT_HEADER_ALL               = REDACT_HEADER_VALUES | REDACT_HEADER_URLS | REDACT_HEADER_UNPARSEABLE_LINES,

    /// The flags that apply to free text
    REDACT_TEXT_ALL                 = REDACT_TEXT_URLS | REDACT_TEXT_AUTH_SCHEMES | REDACT_TEXT_KEY_VALUES | REDACT_TEXT_TOKEN_RUNS | REDACT_TEXT_EMAILS | REDACT_TEXT_IPS | REDACT_TEXT_CARD_NUMBERS,

    /// Both sanitation flags
    REDACT_SANITIZE_ALL             = REDACT_SANITIZE_CONTROL | REDACT_SANITIZE_NON_ASCII
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Replaces what must not be shown by a marker, wherever it is shown: in a log, in a user interface, in an
 *  error message that is handed to a caller. URLs carry secrets (API keys, access tokens, OAuth codes,
 *  passwords) in the user info, the query and the fragment, HTTP headers carry them in Authorization,
 *  Cookie and the like, and free text carries them in any form, so nothing that shows such a thing
 *  may show it as it is.
 *
 *  A redactor does exactly what it is told to: its ERedactionFlag set says WHAT is redacted, the lists say
 *  WHICH names and path segments count (AddSensitiveName(), AddQueryName(), AddPathSegment(); simple glob
 *  patterns as CGlobPattern knows them, case insensitive, matched against the whole name or segment,
 *  so "*key*" is a name that holds "key"), the marker, the maximum length and the sanitation say HOW the
 *  result looks. A redactor that is made without flags redacts nothing. The presets make the common
 *  cases one expression:
 *
 *      GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Failed to fetch " << CRedactor::ForUrl().RedactUrl( url ) );
 *      GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Sent: " << CRedactor::ForHeaders().RedactHeaders( headerText ) );
 *      GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Server said: " << CRedactor::ForLogLine().RedactText( answer ) );
 *      label.SetText( CRedactor::ForDisplay().RedactUrl( url ) );
 *
 *  and a preset is the start of a redactor of your own:
 *
 *      CRedactor redactor( CRedactor::ForUrl() );
 *      redactor.SetMarker( "[hidden]" ).SetMaxLength( 200 ).AddPathSegment( "reset-*" );
 *
 *  What goes wrong goes toward redacting more: the functions accept any input (empty, malformed, binary,
 *  megabytes), never throw because of it and never let a part through that they cannot tell is harmless.
 *  Percent encoding is not decoded in the output, so it cannot hide or reveal anything; names are decoded
 *  for the comparison with the lists, so "ke%79" is the name "key". The result is limited to GetMaxLength()
 *  bytes, of which the last three are "..." when something was cut, and is built in one allocation.
 *  Redacting what is redacted already gives the same text, except where the maximum length cut it.
 *
 *  The const functions only read the redactor, so a redactor that is not changed any more can be used
 *  by any number of threads at once; the presets are made on first use and are never changed.
 */
class GUCEF_CORE_PUBLIC_CPP CRedactor
{
    public:

    /** No flags, no lists, the marker "***", no maximum length: redacts nothing */
    CRedactor( void );

    CRedactor( const CRedactor& src );

    explicit CRedactor( TRedactionFlags flags );

    ~CRedactor();

    CRedactor& operator=( const CRedactor& src );

    /**
     *  A redactor for URLs that are logged or put in an error message: REDACT_URL_USERINFO,
     *  REDACT_URL_QUERY_VALUES, REDACT_URL_FRAGMENT, REDACT_URL_INVALID_HOST and both sanitations,
     *  at most 1024 bytes. What is kept: the scheme, the host, the port, the path and the names of
     *  the query items.
     */
    static const CRedactor& ForUrl( void );

    /**
     *  A redactor for the header section of a HTTP message that is logged: everything of REDACT_HEADER_ALL,
     *  REDACT_NAME_WORDS, what ForUrl() has for the URLs in it and control character sanitation, at
     *  most 16384 bytes.
     */
    static const CRedactor& ForHeaders( void );

    /**
     *  A redactor for one line of log text of which not known what it holds: what ForUrl() has for URLs and what
     *  ForHeaders() has for sensitive names, everything of REDACT_TEXT_ALL except e-mail addresses,
     *  IP addresses and card numbers (which are not secrets of the program), control character sanitation
     *  and at most 4096 bytes. Non ASCII text is kept.
     */
    static const CRedactor& ForLogLine( void );

    /**
     *  A redactor for what a user interface shows to the user: the user info, the fragment and the values of
     *  query items with a sensitive name of URLs, URLs in text, credentials after Bearer and the like, and
     *  the values of sensitive names, control character sanitation, at most 256 bytes. Non ASCII
     *  text is kept, so the interface can show the host and the path as they are written.
     */
    static const CRedactor& ForDisplay( void );

    /** @return what is redacted, an or-ed set of ERedactionFlag values */
    TRedactionFlags GetFlags( void ) const;

    /** Sets everything that is redacted and sanitized at once */
    CRedactor& SetFlags( TRedactionFlags flags );

    /** Adds flags to what is redacted */
    CRedactor& AddFlags( TRedactionFlags flags );

    /** Removes flags from what is redacted */
    CRedactor& RemoveFlags( TRedactionFlags flags );

    /**
     *  Sets what replaces what is redacted. Default "***". Control characters of the marker are replaced
     *  by '?' and it is limited to 64 bytes. It is not changed by sanitation of the rest.
     */
    CRedactor& SetMarker( const CString& marker );

    const CString& GetMarker( void ) const;

    /**
     *  Sets the most bytes a result takes, 0 for no limit. A result that is cut ends in "..." and is as
     *  long as the limit (a limit of less than four gives a cut without the dots), and the cut is made
     *  between UTF-8 characters, not through one. The buffer for the result is made once, at the
     *  smaller of the limit and what the input can give (on the stack up to 1024 bytes), so a limit
     *  also limits what a hostile input can cost.
     */
    CRedactor& SetMaxLength( UInt32 maxLength );

    UInt32 GetMaxLength( void ) const;

    /** Sets the least length of a run for REDACT_TEXT_TOKEN_RUNS, default 32, at least 8 */
    CRedactor& SetTokenMinLength( UInt32 minLength );

    UInt32 GetTokenMinLength( void ) const;

    /**
     *  Adds a pattern of the names that are sensitive in addition to the rule of REDACT_NAME_WORDS: header
     *  names (REDACT_HEADER_VALUES), names of query items (REDACT_URL_QUERY_LISTED) and names in
     *  text (REDACT_TEXT_KEY_VALUES). Case insensitive, matched against the whole name: "x-my-*" and
     *  "*pin*" are patterns, "x-my-" is only that name.
     *
     *  @return false when the pattern is not valid (an open '[' or '{'), in which case it is not added
     */
    bool AddSensitiveName( const CString& pattern );

    /** Same as AddSensitiveName() for names of query items only (REDACT_URL_QUERY_LISTED), for example "code" or "state" */
    bool AddQueryName( const CString& pattern );

    /** Adds a pattern of the path segments to redact (REDACT_URL_PATH_SEGMENTS), matched against one segment at a time, so "*" does not need to cross a '/' */
    bool AddPathSegment( const CString& pattern );

    /** Removes the patterns of AddSensitiveName(), AddQueryName() and AddPathSegment() */
    CRedactor& ClearPatterns( void );

    /**
     *  Redacts a URL as the URL flags say; see ERedactionFlag. The URL can be anything that is used as one: with
     *  a scheme, without (//host/path, /path?query, a bare host:port) and malformed. An empty URL gives an
     *  empty result.
     */
    CString RedactUrl( const CString& url ) const;

    /**
     *  Same as RedactUrl() for text which is not a string of its own, for example the target of a request line
     *  that is part of a larger text.
     *
     *  @param url the bytes of the URL, no terminator needed
     *  @param length number of bytes of the URL
     */
    CString RedactUrlBytes( const char* url, UInt32 length ) const;

    /**
     *  Redacts the value of a header: the marker for a header of which the name is sensitive
     *  (REDACT_HEADER_VALUES), the URL redacted for a header that holds a URL (REDACT_HEADER_URLS), the
     *  value, with the text flags applied, for any other.
     */
    CString RedactHeaderValue( const CString& name, const CString& value ) const;

    /**
     *  Redacts the header section of a HTTP message: the start line and the header lines, separated by CRLF or LF.
     *  The line ends are kept, the white space after the ':' of a header line becomes a single space and a header line
     *  that continues on the next line (which starts with white space) is redacted as the header is. The target
     *  of a request line and the values of the header lines go through the URL and header flags. An empty
     *  text gives an empty result. Do not hand it the body of a message.
     */
    CString RedactHeaders( const CString& block ) const;

    /**
     *  Redacts free text with the text flags, see ERedactionFlag, and the URL and name flags that
     *  they use. Text is not made to be understood: a secret is found by what it looks like (a URL, a
     *  credential after a scheme, a name that says it is a secret, a long token, an address, a card number), so what is not
     *  one of those is not found. Where a secret has a fixed place use the function for that place.
     */
    CString RedactText( const CString& text ) const;

    /**
     *  @return whether the name says that its value is a credential or otherwise secret: the rule of
     *          REDACT_NAME_WORDS when that flag is set (Authorization, Cookie, Set-Cookie, X-Api-Key,
     *          X-WebApi-Key, X-Auth-Token, password, client_secret and the like) or a pattern of
     *          AddSensitiveName(). Case insensitive. An empty name is not.
     */
    bool IsSensitiveName( const CString& name ) const;

    private:

    class COutput;

    typedef GUCEF::vector< CGlobPattern, gucef_allocator< CGlobPattern > > PatternVector;

    void WriteUrl( COutput& out, const char* url, UInt32 length ) const;

    void WriteQuery( COutput& out, const char* url, UInt32 length, UInt32& position ) const;

    void WritePath( COutput& out, const char* url, UInt32 length, UInt32& position ) const;

    void WriteHeaders( COutput& out, const char* text, UInt32 length ) const;

    void WriteHeaderValue( COutput& out, const CString& name, const char* value, UInt32 length ) const;

    void WriteText( COutput& out, const char* text, UInt32 length ) const;

    UInt32 WriteKeyValue( COutput& out, const char* text, UInt32 length, UInt32 position ) const;

    static UInt32 WriteAuthScheme( COutput& out, const char* text, UInt32 length, UInt32 position );

    bool IsSensitiveNameBytes( const char* name, UInt32 length ) const;

    bool IsListedQueryName( const char* name, UInt32 length ) const;

    bool IsPathSegmentToRedact( const char* segment, UInt32 length ) const;

    UInt32 ReserveSize( UInt32 inputLength ) const;

    private:

    TRedactionFlags m_flags;
    CString m_marker;
    UInt32 m_maxLength;
    UInt32 m_tokenMinLength;
    PatternVector m_sensitiveNames;
    PatternVector m_queryNames;
    PatternVector m_pathSegments;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CREDACTOR_H ? */
