/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTH_TESTAPP_TESTFAKEOIDC_H
#define GUCEF_AUTH_TESTAPP_TESTFAKEOIDC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#include "TestRsaSigner.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What an id token of the fake provider looks like; the default is a good token. Tests change what they want to be wrong */
class CFakeTokenSpec
{
    public:

    GUCEF::CORE::CString issuer;                        /**< empty: the issuer of the provider */
    GUCEF::CORE::CString subject;                       /**< default sub-1000 */
    GUCEF::CORE::CString::StringVector audiences;       /**< empty: the client id */
    bool setAzp;
    GUCEF::CORE::CString azp;
    GUCEF::Int64 expiresInSeconds;                      /**< from now, default 3600 */
    bool setNotBefore;
    GUCEF::Int64 notBeforeInSeconds;                    /**< from now */
    bool setIssuedAt;
    GUCEF::Int64 issuedAtInSeconds;                     /**< from now, default is now */
    bool includeNonce;
    GUCEF::CORE::CString nonceOverride;                 /**< empty: the nonce of the login */
    GUCEF::CORE::CString algorithm;                     /**< default RS256 */
    GUCEF::CORE::CString keyId;                         /**< empty: the id of the key that signs */
    bool omitKeyId;
    bool tamperSignature;
    GUCEF::CORE::CString signWith;                      /**< "evil" for the key that is not in the key set */
    GUCEF::CORE::CString tenantId;                      /**< the tid claim when set */
    GUCEF::CORE::CString objectId;                      /**< the oid claim when set */
    GUCEF::CORE::CString name;
    GUCEF::CORE::CString picture;
    bool includeAtHash;
    GUCEF::CORE::CString atHashOverride;
    bool addCrit;
    GUCEF::CORE::CString extraPayloadMembers;           /**< raw json members, no braces */
    bool duplicateAlg;

    CFakeTokenSpec( void );
};

/** How the endpoints of the fake behave */
class CFakeBehavior
{
    public:

    bool denyAuthorization;                             /**< authorize answers with error=access_denied */
    bool requirePkce;
    GUCEF::UInt32 tokenDelayInMs;
    GUCEF::CORE::CString tokenError;                    /**< when set the token endpoint answers with this error */
    int tokenErrorStatus;
    bool rotateRefreshTokens;
    bool issueIdToken;
    bool refreshIssuesIdToken;
    GUCEF::UInt32 devicePendingPolls;                   /**< polls answered authorization_pending before the outcome */
    GUCEF::UInt32 deviceSlowDownAtPoll;                 /**< the poll (1 based) that is answered slow_down, 0 for none */
    GUCEF::CORE::CString deviceOutcome;                 /**< "ok", "denied" or "expired" after the pending polls */
    GUCEF::UInt32 deviceInterval;                       /**< seconds in the answer, in units of the engine settings */
    GUCEF::UInt32 deviceExpiresIn;
    bool deviceOmitInterval;
    GUCEF::CORE::CString jwksCacheControl;
    GUCEF::UInt32 jwksDelayInMs;
    bool openIdAlwaysInvalid;
    GUCEF::CORE::CString openIdSubject;
    GUCEF::CORE::CString openIdName;                    /**< the field openid.test_name of the assertion: an extension that carries the name of the person, none when empty */
    GUCEF::CORE::CString openIdClaimedIdOverride;
    GUCEF::CORE::CString openIdReturnToOverride;
    GUCEF::CORE::CString openIdReturnToEdit;            /**< changes the return address the login came with in the assertion: "port", "state", "host", "path", "extra", "scheme" or "loopback" */
    bool openIdCancel;                                  /**< the player cancels at the provider: the answer is openid.mode=cancel */
    GUCEF::CORE::CString openIdOpEndpointOverride;
    bool openIdOmitSignedReturnTo;
    GUCEF::Int64 openIdNonceAgeInSeconds;
    int openIdCheckStatus;                              /**< the HTTP status of the answer to check_authentication, 0 for 200 with the verdict */
    GUCEF::CORE::CString userinfoSubjectOverride;       /**< the subject the user info endpoint claims, when set */
    bool userinfoFails;

    CFakeBehavior( void );
};

/** What the fake was asked, counted */
class CFakeStats
{
    public:

    GUCEF::UInt32 discovery;
    GUCEF::UInt32 jwks;
    GUCEF::UInt32 authorize;
    GUCEF::UInt32 token;
    GUCEF::UInt32 userinfo;
    GUCEF::UInt32 device;
    GUCEF::UInt32 devicePolls;
    GUCEF::UInt32 revoke;
    GUCEF::UInt32 openIdLogin;
    GUCEF::UInt32 openIdCheck;
    GUCEF::UInt32 verifyRequests;
    GUCEF::UInt32 pkceFailures;
    GUCEF::UInt32 codeReuse;

    CFakeStats( void );
};

/**
 *  A provider that lives in the test: an HTTP server on 127.0.0.1 and an ephemeral port, on a thread and a pulse generator
 *  of its own so that a test thread can block on the engine. It serves discovery, the key set, authorization (the test
 *  acts as the browser), token (it checks PKCE, the redirect uri, the client and that a code is used once), user
 *  info, the device flow, revocation, OpenID 2.0 and the token checks of Facebook and Amazon.
 */
class CFakeOidcProvider : public GUCEF::MT::CActiveObject
{
    public:

    CFakeOidcProvider( void );

    virtual ~CFakeOidcProvider();

    bool Start( void );

    void Stop( void );

    GUCEF::UInt16 GetPort( void ) const;

    /** http://127.0.0.1:port */
    GUCEF::CORE::CString GetBaseUrl( void ) const;

    /** The issuer in the tokens: the base address and the issuer path */
    GUCEF::CORE::CString GetIssuer( void ) const;

    /** For a tenant issuer: the path with {tid}, for example /{tid}/v2.0 */
    void SetIssuerPath( const GUCEF::CORE::CString& path );

    void SetClientId( const GUCEF::CORE::CString& clientId );

    /** The key that signs, with its id; and the keys in the key set (kid -> key) */
    void SetSigningKey( const GUCEF::CORE::CString& keyId, const CTestRsaKey* key );

    void PublishKey( const GUCEF::CORE::CString& keyId, const CTestRsaKey* key );

    void UnpublishKey( const GUCEF::CORE::CString& keyId );

    void SetTokenSpec( const CFakeTokenSpec& spec );

    void SetBehavior( const CFakeBehavior& behavior );

    CFakeBehavior GetBehavior( void ) const;

    CFakeStats GetStats( void ) const;

    GUCEF::CORE::CString::StringMap GetLastAuthorizeParameters( void ) const;

    GUCEF::CORE::CString::StringMap GetLastTokenParameters( void ) const;

    GUCEF::CORE::CString GetLastTokenAuthorization( void ) const;

    GUCEF::CORE::CString GetLastTokenContentType( void ) const;

    /** The form of the last check_authentication request as it was received, and its content type */
    GUCEF::CORE::CString::StringMap GetLastOpenIdCheck( void ) const;
    GUCEF::CORE::CString GetLastOpenIdCheckContentType( void ) const;

    GUCEF::CORE::CString::StringVector GetRevokedTokens( void ) const;

    /** Times (ms on a monotonic clock) at which the device code was polled */
    GUCEF::vector< GUCEF::UInt64 > GetDevicePollTimes( void ) const;

    /** Makes a signed id token */
    GUCEF::CORE::CString MintIdToken( const CFakeTokenSpec& spec, const GUCEF::CORE::CString& nonce, const GUCEF::CORE::CString& accessToken ) const;

    /** A refresh token the provider accepts for the subject */
    GUCEF::CORE::CString IssueRefreshToken( const GUCEF::CORE::CString& subject );

    /** An access token the provider knows, for the subject; for the userinfo endpoints */
    GUCEF::CORE::CString IssueAccessToken( const GUCEF::CORE::CString& subject );

    bool IsRefreshTokenValid( const GUCEF::CORE::CString& refreshToken ) const;

    /** Registers an access token for the checks of Facebook (debug_token) and Amazon (tokeninfo) */
    void RegisterVerifyToken( const GUCEF::CORE::CString& token, const GUCEF::CORE::CString& appId, const GUCEF::CORE::CString& userId, bool valid, GUCEF::Int64 expiresAt );

    /** A positive OpenID 2.0 assertion for the return address, as the query parameters the provider would redirect with; its nonce is known to the provider */
    GUCEF::CORE::CString::StringMap MakeAssertion( const GUCEF::CORE::CString& returnTo );

    /** The handler's entry point */
    void HandleRequest( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SCode
    {
        GUCEF::CORE::CString challenge;
        GUCEF::CORE::CString redirectUri;
        GUCEF::CORE::CString clientId;
        GUCEF::CORE::CString nonce;
        GUCEF::CORE::CString subject;
        bool used;
    };

    struct SDevice
    {
        GUCEF::UInt32 polls;
        bool slowDownSent;
        GUCEF::CORE::CString clientId;
    };

    struct SVerifyToken
    {
        GUCEF::CORE::CString appId;
        GUCEF::CORE::CString userId;
        bool valid;
        GUCEF::Int64 expiresAt;
    };

    GUCEF::CORE::CString::StringMap MakeAssertionLocked( const GUCEF::CORE::CString& returnTo );
    void HandleAuthorize( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );
    void HandleToken( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );
    void HandleDevice( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );
    void HandleUserInfo( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response, const GUCEF::CORE::CString& shape );
    void HandleOpenId( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );
    void HandleVerify( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response, bool facebook );
    void HandleJwks( GUCEF::WEB::CHttpResponseData& response );
    void HandleDiscovery( GUCEF::WEB::CHttpResponseData& response );
    void HandleRevoke( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );
    void SendJson( GUCEF::WEB::CHttpResponseData& response, int status, const GUCEF::CORE::CString& body );
    void SendError( GUCEF::WEB::CHttpResponseData& response, int status, const GUCEF::CORE::CString& error );
    void Redirect( GUCEF::WEB::CHttpResponseData& response, const GUCEF::CORE::CString& location );
    bool ReadParameters( const GUCEF::WEB::CHttpRequestData& request, GUCEF::CORE::CString::StringMap& parameters ) const;
    GUCEF::CORE::CString MakeRandom( void ) const;
    GUCEF::CORE::CString MintLocked( const CFakeTokenSpec& spec, const GUCEF::CORE::CString& nonce, const GUCEF::CORE::CString& accessToken ) const;
    GUCEF::CORE::CString IssuerFor( const GUCEF::CORE::CString& tenant ) const;
    GUCEF::CORE::CString IssueTokens( const SCode* code, const GUCEF::CORE::CString& subject, const GUCEF::CORE::CString& nonce, bool withIdToken, bool newRefreshToken, const GUCEF::CORE::CString& oldRefreshToken );

    CFakeOidcProvider( const CFakeOidcProvider& src );              /**< not implemented */
    CFakeOidcProvider& operator=( const CFakeOidcProvider& src );   /**< not implemented */

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, const CTestRsaKey* > TKeyMap;
    typedef GUCEF::map< GUCEF::CORE::CString, SCode > TCodeMap;
    typedef GUCEF::map< GUCEF::CORE::CString, SDevice > TDeviceMap;
    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CString > TTextMap;
    typedef GUCEF::map< GUCEF::CORE::CString, SVerifyToken > TVerifyMap;

    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::WEB::CHTTPServer* m_server;
    void* m_factory;
    GUCEF::UInt16 m_port;
    bool m_running;
    GUCEF::CORE::CString m_issuerPath;
    GUCEF::CORE::CString m_clientId;
    GUCEF::CORE::CString m_signingKeyId;
    TKeyMap m_keys;
    TKeyMap m_publishedKeys;
    CFakeTokenSpec m_spec;
    CFakeBehavior m_behavior;
    CFakeStats m_stats;
    GUCEF::CORE::CString::StringMap m_lastAuthorize;
    GUCEF::CORE::CString::StringMap m_lastToken;
    GUCEF::CORE::CString m_lastTokenAuthorization;
    GUCEF::CORE::CString m_lastTokenContentType;
    GUCEF::CORE::CString::StringMap m_lastOpenIdCheck;
    GUCEF::CORE::CString m_lastOpenIdCheckContentType;
    GUCEF::CORE::CString::StringVector m_revoked;
    GUCEF::vector< GUCEF::UInt64 > m_devicePollTimes;
    TCodeMap m_codes;
    TDeviceMap m_devices;
    TTextMap m_refreshTokens;
    TTextMap m_accessTokens;
    TTextMap m_openIdNonces;
    TVerifyMap m_verifyTokens;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTFAKEOIDC_H ? */
