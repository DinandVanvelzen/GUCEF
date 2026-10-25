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

#ifndef GUCEF_AUTH_CAUTHOPTIONS_H
#define GUCEF_AUTH_CAUTHOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_AUTH_CACCOUNTREF_H
#include "gucefAUTH_CAccountRef.h"
#define GUCEF_AUTH_CACCOUNTREF_H
#endif /* GUCEF_AUTH_CACCOUNTREF_H ? */

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

/** Why a login did not happen */
enum EAuthFailureReason
{
    AUTHFAIL_UNKNOWN = 0,
    AUTHFAIL_CANCELLED,                 /**< the requester or the user cancelled */
    AUTHFAIL_TIMEOUT,                   /**< the user did not finish in time, or the device code expired */
    AUTHFAIL_DENIED,                    /**< the user or the provider refused */
    AUTHFAIL_NOT_AVAILABLE,             /**< the provider cannot be used here and now: not configured, no platform session, no codec */
    AUTHFAIL_NOT_SUPPORTED,             /**< the provider does not have that operation or flow */
    AUTHFAIL_NETWORK,                   /**< the provider could not be reached, trying again later can work */
    AUTHFAIL_INVALID_GRANT,             /**< a refresh token or code is no longer accepted: logging in again is needed */
    AUTHFAIL_INVALID_RESPONSE,          /**< the provider answered with something that failed validation (signature, issuer, audience, nonce, expiry) */
    AUTHFAIL_IN_PROGRESS,               /**< a login with the provider is already under way */
    AUTHFAIL_ACCOUNT_LINKED_ELSEWHERE,  /**< the account is linked to another profile */
    AUTHFAIL_INTERNAL                   /**< a local failure such as a store that cannot be written */
};

typedef enum EAuthFailureReason TAuthFailureReason;

/** A stable lower case name of a reason, for logs and configuration */
GUCEF_AUTH_EXPORT_CPP const char*
AuthFailureReasonToString( TAuthFailureReason reason );

/*-------------------------------------------------------------------------*/

/** What the application asks of a login */
class GUCEF_AUTH_EXPORT_CPP CAuthOptions : public CORE::CIDataNodeSerializable
{
    public:

    /**
     *  How long a person gets to finish an interactive login in the browser (the authorization code flow, OpenID 2.0): ten minutes.
     *  There is no standard for how long a client waits. RFC 6749 section 4.1.2 recommends that an authorization code lives at most ten
     *  minutes at the server, which bounds what a login can need after the person has agreed; before that come the password manager,
     *  the second factor and the captcha, which take what they take. A device login is bound by what the server says (RFC 8628,
     *  "expires_in", often 5 to 30 minutes), never by this, see MaxDeviceCodeTimeoutInMs. CAuthManager replaces this default by the
     *  setting loginTimeoutInSeconds (CAuthSettings); a caller that wants another time sets timeoutInMs to it.
     */
    static const UInt32 DefaultTimeoutInMs = 600000;

    /** The time a login with a device code may take at the most, when the options say the default: the provider's expires_in is what ends it, this is the ceiling */
    static const UInt32 MaxDeviceCodeTimeoutInMs = 1800000;

    CORE::CString::StringVector scopes;     /**< additional to the scopes of the provider's settings */
    CORE::CString loginHint;                /**< an account name or email to prefill at the provider */
    bool forceInteractive;                  /**< ask the user even when a session or token could be used */
    UInt32 timeoutInMs;                     /**< how long the user has to finish, the provider ends the login with AUTHFAIL_TIMEOUT after it; the default (DefaultTimeoutInMs) is replaced by the setting loginTimeoutInSeconds of the manager, and for a device code by the expiry the provider gives */
    bool preferBrowser;                     /**< use the system browser with a redirect back to the app when the provider can */
    bool preferDeviceCode;                  /**< show a code for another device instead of opening a browser; wins over preferBrowser */
    bool linkToProfile;                     /**< CAuthManager: link the account to the profile after the login, when there is a signer */

    CAuthOptions( void );

    CAuthOptions( const CAuthOptions& src );

    virtual ~CAuthOptions();

    CAuthOptions& operator=( const CAuthOptions& src );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHOPTIONS_H ? */
