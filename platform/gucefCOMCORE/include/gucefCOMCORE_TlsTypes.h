/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_TLSTYPES_H
#define GUCEF_COMCORE_TLSTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The value SocketErrorEvent carries for a failure of the TLS layer is this base plus the ETlsErrorClass, so that
 *  a consumer which only knows about socket errors still sees a distinct, non-zero value. The errors of the
 *  socket itself (DNS, connect) are passed on unchanged.
 */
#define GUCEF_COMCORE_TLS_ERRORCODE_BASE  700000

/*-------------------------------------------------------------------------*/

/** The versions of the protocol by their number on the wire. There is nothing below 1.2: no setting enables it */
enum ETlsVersion
{
    TLSVERSION_ANY = 0,         /**< as a maximum: the newest the provider offers */
    TLSVERSION_1_2 = 0x0303,
    TLSVERSION_1_3 = 0x0304
};
typedef enum ETlsVersion TTlsVersion;

/*-------------------------------------------------------------------------*/

/** What a session is doing */
enum ETlsState
{
    TLSSTATE_IDLE = 0,          /**< created, Start() was not called */
    TLSSTATE_HANDSHAKING,       /**< the handshake is in progress: ciphertext in, ciphertext out */
    TLSSTATE_ESTABLISHED,       /**< application data can flow both ways */
    TLSSTATE_CLOSING,           /**< our close_notify was produced, waiting for the one of the peer */
    TLSSTATE_CLOSED,            /**< closed in an orderly way, by the peer or by both sides */
    TLSSTATE_FAILED             /**< a fatal error, GetError() says which */
};
typedef enum ETlsState TTlsState;

/*-------------------------------------------------------------------------*/

/** What kind of failure ended a TLS connection, the same for every provider */
enum ETlsErrorClass
{
    TLSERR_NONE = 0,                    /**< no failure */
    TLSERR_NO_PROVIDER,                 /**< no TLS provider is available on this system: HTTPS and the like cannot be used */
    TLSERR_CONNECT,                     /**< DNS or TCP connect failed or the connection broke: the socket reports the details */
    TLSERR_SETTINGS,                    /**< the settings cannot be used, for example a CA file that cannot be read. Verification is never skipped because of it */
    TLSERR_HANDSHAKE_TIMEOUT,           /**< the handshake did not finish in time */
    TLSERR_PROTOCOL_VERSION,            /**< the peer only speaks versions older than the minimum (TLS 1.2) */
    TLSERR_HANDSHAKE_FAILURE,           /**< no common parameters, malformed records, a peer that does not speak TLS */
    TLSERR_PROTOCOL_ALERT,              /**< a fatal alert of the peer */
    TLSERR_CERT_UNTRUSTED,              /**< no path to a trust anchor, self signed, unknown issuer */
    TLSERR_CERT_EXPIRED,                /**< past its end of validity (the clock of this computer is part of the picture) */
    TLSERR_CERT_NOT_YET_VALID,          /**< before its start of validity (the clock of this computer is part of the picture) */
    TLSERR_CERT_HOSTNAME_MISMATCH,      /**< the name asked for is not in the certificate */
    TLSERR_CERT_NO_SAN,                 /**< the certificate has no subject alternative names: the common name is never used */
    TLSERR_CERT_REVOKED,                /**< the certificate was revoked */
    TLSERR_CERT_OTHER,                  /**< usage, constraints, signature or any other fault of the certificate chain */
    TLSERR_PIN_MISMATCH,                /**< the chain verified but contains none of the pinned keys */
    TLSERR_PEER_CLOSED,                 /**< the peer closed the connection during the handshake */
    TLSERR_TRUNCATED,                   /**< the peer closed the connection without close_notify while data was expected */
    TLSERR_INTERNAL                     /**< a failure of the TLS library */
};
typedef enum ETlsErrorClass TTlsErrorClass;

/*-------------------------------------------------------------------------*/

/** Capabilities of a provider, a bit mask */
enum ETlsCapability
{
    TLSCAP_NONE         = 0,
    TLSCAP_TLS13        = 1,
    TLSCAP_ALPN         = 2,
    TLSCAP_RESUMPTION   = 4,
    TLSCAP_CUSTOM_TRUST = 8,
    TLSCAP_SPKI_PINS    = 16,
    TLSCAP_SERVER       = 32,
    TLSCAP_CLIENT_CERT  = 64,
    TLSCAP_REVOCATION   = 128
};

/*-------------------------------------------------------------------------*/

/** A stable lower case name for logs, for example "certificate-expired" */
GUCEF_COMCORE_EXPORT_CPP const char*
TlsErrorClassToString( ETlsErrorClass errorClass );

GUCEF_COMCORE_EXPORT_CPP const char*
TlsStateToString( ETlsState state );

/*-------------------------------------------------------------------------*/

/**
 *  A TLS failure with what is known about it. Never holds keys, secrets or plaintext.
 */
class GUCEF_COMCORE_EXPORT_CPP CTlsError
{
    public:

    ETlsErrorClass errorClass;
    Int32 providerCode;             /**< the code of the TLS library, 0 when there is none */
    CORE::CString message;          /**< what the library said, English, no host names or URLs */
    Int32 verifyCode;               /**< the result of the certificate verification of the provider, 0 when it was not the cause */
    CORE::CString peerSubject;      /**< the subject of the certificate of the peer when it was received */
    CORE::CString hint;             /**< what the user can do about it, for example which package to install */

    CTlsError( void );
    CTlsError( ETlsErrorClass errorClass, const CORE::CString& message );
    CTlsError( const CTlsError& src );
    ~CTlsError();
    CTlsError& operator=( const CTlsError& src );

    void Clear( void );

    bool IsError( void ) const;

    /**
     *  A sentence for the user in the terms of the class, for example
     *  "the certificate of example.com is not trusted (unable to get local issuer certificate)".
     *  The host is only what the caller passes in: errors never contain it themselves.
     */
    CORE::CString GetDescription( const CORE::CString& host ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  What was negotiated, valid from the establishment of a session
 */
class GUCEF_COMCORE_EXPORT_CPP CTlsSessionInfo
{
    public:

    CORE::CString protocolVersion;      /**< "TLSv1.3" */
    CORE::CString cipherName;           /**< "TLS_AES_256_GCM_SHA384" */
    CORE::CString alpnProtocol;         /**< the selected application protocol, empty when none */
    CORE::CString peerSubject;          /**< subject of the certificate of the peer */
    CORE::CString providerName;         /**< "openssl-dynamic" */
    CORE::CString providerVersion;      /**< "OpenSSL 3.5.5 27 Jan 2026" */
    bool resumed;                       /**< an earlier session was resumed: no full handshake took place */
    bool peerClosedCleanly;             /**< the peer sent close_notify. False after a disconnect means the stream may be truncated */
    UInt32 handshakeDurationInMs;

    CTlsSessionInfo( void );
    CTlsSessionInfo( const CTlsSessionInfo& src );
    ~CTlsSessionInfo();
    CTlsSessionInfo& operator=( const CTlsSessionInfo& src );

    void Clear( void );

    /** one line for the log: "TLSv1.3 TLS_AES_256_GCM_SHA384 alpn=http/1.1 peer="CN=..." resumed=false provider=openssl-dynamic OpenSSL 3.5.5" */
    CORE::CString GetSummary( void ) const;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTCloneableObj< CTlsError >        TTlsErrorEventData;
typedef CORE::CTCloneableObj< CTlsSessionInfo >  TTlsSessionInfoEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TLSTYPES_H ? */
