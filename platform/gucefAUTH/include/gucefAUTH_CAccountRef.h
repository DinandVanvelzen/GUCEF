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

#ifndef GUCEF_AUTH_CACCOUNTREF_H
#define GUCEF_AUTH_CACCOUNTREF_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

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

/**
 *  How far an id of an identity provider reaches. Ids that are scoped to an app only identify the same person in two
 *  products when the products share one registered app at the provider.
 */
enum EAuthIdScope
{
    AUTHIDSCOPE_GLOBAL = 0,             /**< the same for every app of every developer: a platform account id, Epic account id, Google, Microsoft tid:oid */
    AUTHIDSCOPE_PER_ORGANISATION = 1,   /**< the same for every app of one developer, tenant or pool: Amazon, Cognito pool, Identity Center instance */
    AUTHIDSCOPE_PER_APP = 2             /**< different for every registered app: Facebook, the Epic product user id */
};

typedef enum EAuthIdScope TAuthIdScope;

/*-------------------------------------------------------------------------*/

/** What a provider can do, a set of flags */
enum EAuthProviderCaps
{
    AUTHCAP_NONE = 0,
    AUTHCAP_BROWSER = 1,                /**< interactive login in the system browser with a redirect back to the app */
    AUTHCAP_DEVICE_CODE = 2,            /**< interactive login on another device with a code (RFC 8628) */
    AUTHCAP_SILENT = 4,                 /**< can log in without the user doing anything: platform sessions such as Play Games or the client of a store that runs */
    AUTHCAP_SERVER_VERIFIABLE = 8,      /**< a server can verify the proof of a login with Verify() */
    AUTHCAP_REFRESHABLE = 16,           /**< a login can be renewed from a stored refresh token */
    AUTHCAP_REQUIRES_CLIENT_ID = 32     /**< offered only when the application configured a client id for it */
};

typedef enum EAuthProviderCaps TAuthProviderCaps;

/*-------------------------------------------------------------------------*/

/** The ids of the identity namespaces the module itself knows, the provider part of an account reference. The namespaces of backends are those of their plugins */
class GUCEF_AUTH_EXPORT_CPP CAuthProviderIds
{
    public:

    static const CORE::CString Epic;        /**< "epic": Epic account id */
    static const CORE::CString Facebook;    /**< "facebook": app scoped user id */
    static const CORE::CString Google;      /**< "google": the sub claim */
    static const CORE::CString Microsoft;   /**< "microsoft": tid:oid */
    static const CORE::CString Amazon;      /**< "amazon": Login with Amazon user id */
    static const CORE::CString Aws;         /**< "aws": poolId:sub for Cognito, identity store id:user id for IAM Identity Center */
};

/*-------------------------------------------------------------------------*/

/**
 *  The identity of a person at an identity provider: "provider:subject", for example "epic:0123456789abcdef0123456789abcdef" or
 *  "aws:eu-west-1_AbCdEf:2f6c...". The text form is what is stored, linked and banned.
 *
 *  The provider is the namespace of the ids (not the login mechanism, "aws" is both Cognito and IAM Identity Center),
 *  lower case, 1 to MaxProviderLength characters of a-z, 0-9 and '-', starting with a letter. The subject is case
 *  sensitive, 1 to MaxSubjectLength characters of A-Z, a-z, 0-9 and . _ - : @ ~ + = |, and may contain ':'.
 *  A subject with other characters goes through EncodeSubject() first, which writes them as % and two upper case hex
 *  digits (and so does the % itself); a % in a subject has to be such an escape. The text form is split at the first ':'.
 *  An invalid reference is empty and IsValid() says false.
 */
class GUCEF_AUTH_EXPORT_CPP CAccountRef
{
    public:

    static const UInt32 MaxProviderLength = 32;
    static const UInt32 MaxSubjectLength = 512;

    CAccountRef( void );

    /** Normalises the provider to lower case; the reference is empty when provider or subject are not valid */
    CAccountRef( const CORE::CString& provider, const CORE::CString& subject );

    CAccountRef( const CAccountRef& src );

    CAccountRef& operator=( const CAccountRef& src );

    /** Parses "provider:subject"; false and an empty reference when the text is not a valid reference */
    static bool TryParse( const CORE::CString& text, CAccountRef& ref );

    static bool IsValidProvider( const CORE::CString& provider );

    static bool IsValidSubject( const CORE::CString& subject );

    /** Percent encodes every byte of a raw provider subject that is not allowed in a reference, '%' included: different raw subjects stay different */
    static CORE::CString EncodeSubject( const CORE::CString& rawSubject );

    /** Reverses EncodeSubject(); false when the text has a broken escape */
    static bool DecodeSubject( const CORE::CString& encodedSubject, CORE::CString& rawSubject );

    bool IsValid( void ) const;

    bool IsEmpty( void ) const;

    const CORE::CString& GetProvider( void ) const;

    const CORE::CString& GetSubject( void ) const;

    /** "provider:subject", empty for an invalid reference */
    CORE::CString ToString( void ) const;

    /** A hash of the text form (FNV-1a), for hash tables */
    UInt32 GetHash( void ) const;

    bool operator==( const CAccountRef& other ) const;

    bool operator!=( const CAccountRef& other ) const;

    /** Orders by provider then by subject, so it can be a key in a map or set */
    bool operator<( const CAccountRef& other ) const;

    private:

    CORE::CString m_provider;
    CORE::CString m_subject;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CAccountRef > TAccountRefVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CACCOUNTREF_H ? */
