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

#ifndef GUCEF_AUTH_COPENID2_H
#define GUCEF_AUTH_COPENID2_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

#ifndef GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H
#include "gucefAUTH_IOpenId2ProviderProfile.h"
#define GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H
#endif /* GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H ? */

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

/** Why an OpenID 2.0 assertion was refused before the provider was asked */
enum EOpenId2Status
{
    OPENID2_OK = 0,
    OPENID2_MALFORMED,              /**< fields missing, too long, or the wrong namespace or mode */
    OPENID2_WRONG_ENDPOINT,         /**< op_endpoint is not the provider's endpoint */
    OPENID2_BAD_CLAIMED_ID,         /**< the profile of the provider does not accept claimed_id and identity */
    OPENID2_BAD_RETURN_TO,          /**< return_to is not the one that was sent, or is not a loopback address of the application (a page of the relay, when there is one) */
    OPENID2_UNSIGNED_FIELD,         /**< a field that has to be signed is not */
    OPENID2_BAD_NONCE,              /**< response_nonce is not a time followed by text */
    OPENID2_STALE_NONCE,            /**< the nonce is too old or from the future */
    OPENID2_REPLAYED_NONCE          /**< the nonce was seen before */
};
typedef enum EOpenId2Status TOpenId2Status;

GUCEF_AUTH_EXPORT_CPP const char*
OpenId2StatusToString( TOpenId2Status status );

/*-------------------------------------------------------------------------*/

/** The nonces of assertions that were accepted, kept for as long as an assertion with that nonce is still fresh */
class GUCEF_AUTH_EXPORT_CPP COpenId2NonceCache
{
    public:

    static const UInt32 MaxEntries = 4096;

    COpenId2NonceCache( void );
    ~COpenId2NonceCache();

    /** Records the nonce; false when it is there already */
    bool Reserve( const CORE::CString& nonce, UInt64 nowSeconds, UInt32 keepForSeconds );

    /** Forgets the nonce: the check at the provider did not happen */
    void Release( const CORE::CString& nonce );

    void Clear( void );

    UInt32 GetCount( void ) const;

    private:

    COpenId2NonceCache( const COpenId2NonceCache& src );                /**< not implemented */
    COpenId2NonceCache& operator=( const COpenId2NonceCache& src );     /**< not implemented */

    private:

    typedef GUCEF::map< CORE::CString, UInt64 > TNonceMap;

    mutable MT::CMutex m_lock;
    TNonceMap m_nonces;
};

/*-------------------------------------------------------------------------*/

/**
 *  The positive assertion of an OpenID 2.0 provider (the response of checkid_setup): the fields this module reads, and any other
 *  openid.* field of the response (an extension), because the check at the provider wants every field back as it came.
 */
class GUCEF_AUTH_EXPORT_CPP COpenId2Assertion
{
    public:

    static const UInt32 MaxFieldBytes = 2048;
    static const UInt32 MaxExtraFields = 32;        /**< the most openid.* fields besides the ones this module knows; an assertion with more is refused */

    static const CORE::CString Namespace;           /**< http://specs.openid.net/auth/2.0 */
    static const CORE::CString IdentifierSelect;    /**< http://specs.openid.net/auth/2.0/identifier_select */

    CORE::CString::StringMap fields;                /**< the openid.* fields by full name, as received */

    COpenId2Assertion( void );

    /** Takes the openid.* fields of a set of parameters (query or form); false when a required one is missing or any is too long, or there are too many */
    bool Load( const CORE::CString::StringMap& parameters );

    /** The assertion as a query string, which is the proof data */
    CORE::CString ToQueryString( void ) const;

    /** The reverse of ToQueryString() */
    bool LoadFromQueryString( const CORE::CString& query );

    CORE::CString Get( const CORE::CString& field ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  The relay of an OpenID 2.0 login. The provider shows the player the host of the realm, and a loopback address looks
 *  wrong there. The realm and the return address of the login are then on a page of the developer's own domain, a static
 *  page without server code that sends the browser on to the loopback address of the same computer with the parameters it
 *  got. The relay only transports: the application still checks the state at its own listener and the assertion at the
 *  provider, and requires the return address in the assertion to be the one it sent.
 *
 *  The return address is the address of the relay page with the query port=<port of the listener>&s=<state>.
 */
class GUCEF_AUTH_EXPORT_CPP COpenId2Relay
{
    public:

    static const UInt32 MinPort = 1024;                     /**< the relay page sends the browser to ports from here on only */
    static const UInt32 MinStateBytes = 16;
    static const UInt32 MaxStateBytes = 128;
    static const UInt32 MaxUrlBytes = 512;
    static const CORE::CString PortParameter;               /**< "port" */
    static const CORE::CString StateParameter;              /**< "s": the name of the state in the query of the return address and of the callback */
    static const CORE::CString CallbackPath;                /**< "/callback": where the relay page sends the browser to on the loopback address */

    /**
     *  Reads the address of the relay page, the value of the parameter relayUrl. Acceptable is a plain address: https, a
     *  host name with a dot, port 443, a path of letters, digits and - . _ ~ with slashes. Not acceptable: user information,
     *  a query, a fragment, escapes, anything that is not ASCII, a path with an empty segment or a . or .. segment, an IP
     *  address. With allowInsecure (tests) another port, and a loopback host with http, are acceptable too.
     *  baseUrl is the normalized address (scheme and host in lower case, the port 443 left out), realm the origin with a
     *  slash, which is what the provider shows. The reason has no part of the address in it.
     */
    static bool ParseRelayUrl( const CORE::CString& url, bool allowInsecure, CORE::CString& baseUrl, CORE::CString& realm, CORE::CString& reason );

    /** The return address of a login: baseUrl?port=<port>&s=<state> */
    static CORE::CString BuildReturnTo( const CORE::CString& baseUrl, UInt16 port, const CORE::CString& state );

    /** Whether the return address is exactly what BuildReturnTo() makes for baseUrl, a port from MinPort to 65535 and a valid state */
    static bool IsRelayReturnTo( const CORE::CString& returnTo, const CORE::CString& baseUrl );

    /** Whether the text is MinStateBytes to MaxStateBytes of A-Z a-z 0-9 - _ */
    static bool IsValidState( const CORE::CString& state );

    private:

    COpenId2Relay( void );
};

/*-------------------------------------------------------------------------*/

/** The OpenID 2.0 rules, as functions. What differs per provider comes from its profile, see IOpenId2ProviderProfile */
class GUCEF_AUTH_EXPORT_CPP COpenId2
{
    public:

    /** The url the browser is sent to: checkid_setup with identifier_select, the return_to and the realm */
    static CORE::CString BuildAuthenticationUrl( const CORE::CString& endpoint, const CORE::CString& returnTo, const CORE::CString& realm );

    /**
     *  The checks that need no network: namespace, mode, endpoint, the claimed_id and identity (the profile of the
     *  provider decides what it accepts and what the subject is), the signed fields and the nonce (form, age within
     *  maxAgeSeconds, and not seen before: reserved in the cache when everything else is right).
     *  With expectedReturnTo the return_to is that value; without it, it has to be an http address on the loopback
     *  interface with the path loopbackPath. With relayBase (the normalized address of the relay page) the return_to has to
     *  be that page with the query of COpenId2Relay in both cases, and the loopback form is not acceptable. On OPENID2_OK
     *  the nonce is reserved: Release() it when the provider could not be asked.
     */
    static TOpenId2Status Validate( const COpenId2Assertion& assertion       ,
                                    const CORE::CString& endpoint            ,
                                    const IOpenId2ProviderProfile& profile   ,
                                    const CORE::CString& expectedReturnTo    ,
                                    const CORE::CString& loopbackPath        ,
                                    UInt64 nowSeconds                        ,
                                    UInt32 maxAgeSeconds                     ,
                                    COpenId2NonceCache& nonces               ,
                                    const CORE::CString& relayBase = CORE::CString::Empty );

    /** The form body of the check_authentication request: the fields of the assertion with the mode changed */
    static CORE::CString BuildCheckAuthenticationBody( const COpenId2Assertion& assertion );

    /** Whether the answer of check_authentication says is_valid:true, and does not say anything else about it */
    static bool IsValidAnswer( const void* data, UInt32 size );

    /** The time of a response_nonce (the text before the unique part) as seconds since 1970; false when the form is not right */
    static bool ParseNonceTime( const CORE::CString& nonce, UInt64& epochSeconds );

    private:

    COpenId2( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_COPENID2_H ? */
