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

#ifndef GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#define GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPES_H
#include "gucefWEB_WebViewTypes.h"
#define GUCEF_WEB_WEBVIEWTYPES_H
#endif /* GUCEF_WEB_WEBVIEWTYPES_H ? */

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
 *  The hosts a view may load: exact hosts, and "this domain and below" entries, each with a scheme and a port.
 *
 *  Patterns, as written in settings:
 *      example.com                 exactly that host, http or https, on the default port of the scheme
 *      .example.com                example.com and every host below it (a.example.com, b.a.example.com)
 *      https://example.com         only https
 *      example.com:8443            only that port
 *      http://127.0.0.1:41234      a loopback listener
 *      [::1]:8080                  an IPv6 literal
 *  There are no wildcards: '*' anywhere, user information, paths, queries and empty labels make a pattern invalid.
 *  Matching is on the address the engine reports after its own parsing, so international host names arrive in their
 *  ASCII form; a host with bytes outside ASCII is refused. Host names are compared without case and with one trailing
 *  dot removed. An address with user information (user:password@host) is always refused.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewHostAllowList : public CORE::CIConfigurable
{
    public:

    /** One entry; scheme empty for http and https, port 0 for the default port of the scheme */
    struct GUCEF_WEB_PUBLIC_CPP SEntry
    {
        CString scheme;
        CString host;
        bool includeSubdomains;
        UInt16 port;

        SEntry( void );
    };
    typedef struct SEntry TEntry;
    typedef GUCEF::vector< TEntry > TEntryVector;

    CWebViewHostAllowList( void );

    virtual ~CWebViewHostAllowList();

    /** Adds an entry from a pattern; false (and nothing added) for a pattern that is not valid */
    bool AddEntry( const CString& pattern );

    /** Adds every pattern; false when one or more were not valid (the valid ones are added) */
    bool AddEntries( const CString::StringVector& patterns );

    void Clear( void );

    bool IsEmpty( void ) const;

    const TEntryVector& GetEntries( void ) const;

    /** The entries as patterns again */
    CString::StringVector GetPatterns( void ) const;

    /** Whether an http or https address is allowed. Other schemes, malformed addresses and user information are refused */
    bool IsUrlAllowed( const CString& url ) const;

    /** Whether a normalised host on a port with a scheme is allowed (port 0: the default port of the scheme) */
    bool IsHostAllowed( const CString& scheme, const CString& host, UInt16 port ) const;

    /**
     *  Splits an absolute address "scheme://authority/..." strictly: the scheme in lower case, the host normalised
     *  (NormaliseHost()), the port (the default port of the scheme when the address has none, 0 when the scheme has
     *  no default). False for an address that is not of that form or whose host or port is not valid.
     */
    static bool SplitUrl( const CString& url, CString& scheme, CString& host, UInt16& port, bool& hasUserInfo );

    /**
     *  Lower case, one trailing dot removed; false for an empty host, bytes outside ASCII, characters that are not
     *  letters, digits, '-', '.' (or the hex digits and ':' of an IPv6 literal in brackets), empty labels and wildcards
     */
    static bool NormaliseHost( CString& host );

    /** 80 for http and ws, 443 for https and wss, 0 for others */
    static UInt16 GetDefaultPort( const CString& scheme );

    /** Saves the patterns as the attribute "hosts" (comma separated) */
    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Replaces the entries with the patterns of "hosts" (comma separated) and of the children named "host"; invalid ones are logged and left out */
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    static bool ParsePattern( const CString& pattern, TEntry& entry );

    static bool SplitHostAndPort( const CString& authority, CString& host, UInt16& port, bool& hasPort );

    static bool IsIpLiteral( const CString& host );

    private:

    TEntryVector m_entries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H ? */
