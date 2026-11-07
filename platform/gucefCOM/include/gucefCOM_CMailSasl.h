/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CMAILSASL_H
#define GUCEF_COM_CMAILSASL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Where an authentication exchange stands */
enum EMailSaslState
{
    MAILSASL_CONTINUE = 0,      /**< the response is to be sent, the exchange goes on */
    MAILSASL_DONE,              /**< the exchange is complete as far as the client knows; the server still has the last word */
    MAILSASL_FAILED             /**< the exchange failed: a server that could not be verified, a challenge that makes no sense */
};

/*-------------------------------------------------------------------------*/

/**
 *  One SASL mechanism (RFC 4422) of the client side, independent of the protocol that carries it: the protocol
 *  layer base64 decodes the challenges, the mechanism works on the bytes. A mechanism object is used for one exchange.
 */
class GUCEF_COM_PUBLIC_CPP CIMailSaslMechanism
{
    public:

    CIMailSaslMechanism( void );

    virtual ~CIMailSaslMechanism();

    /** The registered name, upper case: PLAIN, LOGIN, XOAUTH2, SCRAM-SHA-256 */
    virtual const CORE::CString& GetName( void ) const = 0;

    /** Whether the client speaks first: the first response is sent with the command that starts the exchange (SASL-IR) */
    virtual bool HasInitialResponse( void ) const = 0;

    /** Whether the secret is sent in a form that an eavesdropper can use (PLAIN, LOGIN, XOAUTH2 tokens): not to be used without TLS */
    virtual bool RevealsSecret( void ) const = 0;

    /** The initial response. @return false when the exchange cannot start (a user name or secret that the mechanism cannot carry) */
    virtual bool Start( CORE::CDynamicBuffer& initialResponse ) = 0;

    /** Answers a challenge of the server. 'response' is empty when there is nothing to send */
    virtual EMailSaslState Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response ) = 0;

    /** Why the exchange failed on this side, no secrets in it */
    const CORE::CString& GetFailureReason( void ) const;

    protected:

    void SetFailure( const char* reason );

    private:

    CIMailSaslMechanism( const CIMailSaslMechanism& src );
    CIMailSaslMechanism& operator=( const CIMailSaslMechanism& src );

    CORE::CString m_failureReason;
};

/*-------------------------------------------------------------------------*/

/** PLAIN, RFC 4616: NUL user NUL password */
class GUCEF_COM_PUBLIC_CPP CMailSaslPlain : public CIMailSaslMechanism
{
    public:

    CMailSaslPlain( const CORE::CString& userName, const CORE::CString& password );
    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool HasInitialResponse( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RevealsSecret( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Start( CORE::CDynamicBuffer& initialResponse ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EMailSaslState Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_userName;
    CORE::CString m_password;
};

/*-------------------------------------------------------------------------*/

/** LOGIN: the server asks for the user name and then for the password */
class GUCEF_COM_PUBLIC_CPP CMailSaslLogin : public CIMailSaslMechanism
{
    public:

    CMailSaslLogin( const CORE::CString& userName, const CORE::CString& password );
    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool HasInitialResponse( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RevealsSecret( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Start( CORE::CDynamicBuffer& initialResponse ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EMailSaslState Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_userName;
    CORE::CString m_password;
    UInt32 m_step;
};

/*-------------------------------------------------------------------------*/

/** XOAUTH2 (Google, Microsoft): the user and an OAuth 2.0 access token. A challenge is the error of the server in JSON and is answered with an empty response */
class GUCEF_COM_PUBLIC_CPP CMailSaslXOAuth2 : public CIMailSaslMechanism
{
    public:

    CMailSaslXOAuth2( const CORE::CString& userName, const CORE::CString& accessToken );
    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool HasInitialResponse( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RevealsSecret( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Start( CORE::CDynamicBuffer& initialResponse ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EMailSaslState Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_userName;
    CORE::CString m_accessToken;
};

/*-------------------------------------------------------------------------*/

/**
 *  SCRAM-SHA-256, RFC 5802 and RFC 7677, without channel binding. The server has to prove that it knows the
 *  password too: its signature is verified. The password is used as it is (SASLprep is not applied, which
 *  makes no difference for ASCII passwords). The iteration count the server asks for is limited
 *  (MinIterations to MaxIterations) so that a server cannot make the client compute for long.
 */
class GUCEF_COM_PUBLIC_CPP CMailSaslScramSha256 : public CIMailSaslMechanism
{
    public:

    static const UInt32 MinIterations = 4096;
    static const UInt32 MaxIterations = 262144;

    CMailSaslScramSha256( const CORE::CString& userName, const CORE::CString& password );
    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool HasInitialResponse( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RevealsSecret( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Start( CORE::CDynamicBuffer& initialResponse ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EMailSaslState Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response ) GUCEF_VIRTUAL_OVERRIDE;

    /** The nonce of the client; for tests that replay the example of the RFC. Without it a random nonce is made by Start() */
    void SetClientNonceForTesting( const CORE::CString& nonce );

    /** PBKDF2 with HMAC-SHA256 (RFC 8018), 32 bytes of output */
    static void Pbkdf2HmacSha256( const void* password, UInt32 passwordSize, const void* salt, UInt32 saltSize, UInt32 iterations, UInt8* output );

    private:

    CORE::CString m_userName;
    CORE::CString m_password;
    CORE::CString m_clientNonce;
    CORE::CString m_clientFirstBare;
    UInt32 m_step;
    UInt8 m_serverSignature[ 32 ];
};

/*-------------------------------------------------------------------------*/

/** Makes the mechanism of a name */
class GUCEF_COM_PUBLIC_CPP CMailSaslFactory
{
    public:

    /** The mechanism of the name (any case) for a password or token, GUCEF_NULL for a name that is not known. 'isToken' selects XOAUTH2 behaviour for the secret */
    static CIMailSaslMechanism* Create( const CORE::CString& name, const CORE::CString& userName, const CORE::CString& secret );

    /** The mechanisms that can be made, in the order of preference for a password */
    static void GetSupportedMechanisms( CORE::CString::StringVector& names );

    private:

    CMailSaslFactory( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILSASL_H ? */
