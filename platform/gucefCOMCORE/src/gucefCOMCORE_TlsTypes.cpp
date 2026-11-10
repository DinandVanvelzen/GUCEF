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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCOMCORE_TlsTypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
TlsErrorClassToString( ETlsErrorClass errorClass )
{GUCEF_TRACE;

    switch ( errorClass )
    {
        case TLSERR_NONE:                   return "none";
        case TLSERR_NO_PROVIDER:            return "no-provider";
        case TLSERR_CONNECT:                return "connect";
        case TLSERR_SETTINGS:               return "settings";
        case TLSERR_HANDSHAKE_TIMEOUT:      return "handshake-timeout";
        case TLSERR_PROTOCOL_VERSION:       return "protocol-version";
        case TLSERR_HANDSHAKE_FAILURE:      return "handshake-failure";
        case TLSERR_PROTOCOL_ALERT:         return "protocol-alert";
        case TLSERR_CERT_UNTRUSTED:         return "certificate-untrusted";
        case TLSERR_CERT_EXPIRED:           return "certificate-expired";
        case TLSERR_CERT_NOT_YET_VALID:     return "certificate-not-yet-valid";
        case TLSERR_CERT_HOSTNAME_MISMATCH: return "certificate-hostname-mismatch";
        case TLSERR_CERT_NO_SAN:            return "certificate-no-san";
        case TLSERR_CERT_REVOKED:           return "certificate-revoked";
        case TLSERR_CERT_OTHER:             return "certificate-other";
        case TLSERR_PIN_MISMATCH:           return "pin-mismatch";
        case TLSERR_PEER_CLOSED:            return "peer-closed";
        case TLSERR_TRUNCATED:              return "truncated";
        case TLSERR_INTERNAL:               return "internal";
    }
    return "unknown";
}

/*-------------------------------------------------------------------------*/

const char*
TlsStateToString( ETlsState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case TLSSTATE_IDLE:         return "idle";
        case TLSSTATE_HANDSHAKING:  return "handshaking";
        case TLSSTATE_ESTABLISHED:  return "established";
        case TLSSTATE_CLOSING:      return "closing";
        case TLSSTATE_CLOSED:       return "closed";
        case TLSSTATE_FAILED:       return "failed";
    }
    return "unknown";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTlsError::CTlsError( void )
    : errorClass( TLSERR_NONE )
    , providerCode( 0 )
    , message()
    , verifyCode( 0 )
    , peerSubject()
    , hint()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsError::CTlsError( ETlsErrorClass errClass, const CORE::CString& msg )
    : errorClass( errClass )
    , providerCode( 0 )
    , message( msg )
    , verifyCode( 0 )
    , peerSubject()
    , hint()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsError::CTlsError( const CTlsError& src )
    : errorClass( src.errorClass )
    , providerCode( src.providerCode )
    , message( src.message )
    , verifyCode( src.verifyCode )
    , peerSubject( src.peerSubject )
    , hint( src.hint )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsError::~CTlsError()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsError&
CTlsError::operator=( const CTlsError& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        errorClass = src.errorClass;
        providerCode = src.providerCode;
        message = src.message;
        verifyCode = src.verifyCode;
        peerSubject = src.peerSubject;
        hint = src.hint;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CTlsError::Clear( void )
{GUCEF_TRACE;

    errorClass = TLSERR_NONE;
    providerCode = 0;
    message.Clear();
    verifyCode = 0;
    peerSubject.Clear();
    hint.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CTlsError::IsError( void ) const
{GUCEF_TRACE;

    return TLSERR_NONE != errorClass;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTlsError::GetDescription( const CORE::CString& host ) const
{GUCEF_TRACE;

    CORE::CString who = host.IsNULLOrEmpty() ? CORE::CString( "the server" ) : host;
    CORE::CString detail = message.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + message + ")";
    CORE::CString text;

    switch ( errorClass )
    {
        case TLSERR_NONE:
            return CORE::CString::Empty;
        case TLSERR_NO_PROVIDER:
            text = "HTTPS and other secure connections need a TLS provider: " + message;
            break;
        case TLSERR_CONNECT:
            text = "the connection to " + who + " failed before TLS started" + detail;
            break;
        case TLSERR_SETTINGS:
            text = "the TLS settings cannot be used" + detail;
            break;
        case TLSERR_HANDSHAKE_TIMEOUT:
            text = "the TLS handshake with " + who + " did not finish in time";
            break;
        case TLSERR_PROTOCOL_VERSION:
            text = "the server " + who + " only supports TLS older than 1.2" + detail;
            break;
        case TLSERR_HANDSHAKE_FAILURE:
            text = "the TLS handshake with " + who + " failed, the server may not speak TLS on this port" + detail;
            break;
        case TLSERR_PROTOCOL_ALERT:
            text = who + " sent a TLS alert" + detail;
            break;
        case TLSERR_CERT_UNTRUSTED:
            text = "the certificate of " + who + " is not trusted" + detail;
            break;
        case TLSERR_CERT_EXPIRED:
            text = "the certificate of " + who + " has expired" + detail + ", check the clock of this computer";
            break;
        case TLSERR_CERT_NOT_YET_VALID:
            text = "the certificate of " + who + " is not valid yet" + detail + ", check the clock of this computer";
            break;
        case TLSERR_CERT_HOSTNAME_MISMATCH:
            text = "the certificate is for other names, not for " + who + detail;
            break;
        case TLSERR_CERT_NO_SAN:
            text = "the certificate of " + who + " has no subject alternative names, the common name is not used" + detail;
            break;
        case TLSERR_CERT_REVOKED:
            text = "the certificate of " + who + " was revoked" + detail;
            break;
        case TLSERR_CERT_OTHER:
            text = "the certificate of " + who + " is not acceptable" + detail;
            break;
        case TLSERR_PIN_MISMATCH:
            text = "the certificate chain of " + who + " does not contain a pinned key";
            break;
        case TLSERR_PEER_CLOSED:
            text = who + " closed the connection during the TLS handshake";
            break;
        case TLSERR_TRUNCATED:
            text = who + " closed the connection without a TLS close notification, the data may be incomplete";
            break;
        case TLSERR_INTERNAL:
            text = "the TLS library failed" + detail;
            break;
    }
    if ( !hint.IsNULLOrEmpty() )
        text += ". " + hint;
    return text;
}

/*-------------------------------------------------------------------------*/

CTlsSessionInfo::CTlsSessionInfo( void )
    : protocolVersion()
    , cipherName()
    , alpnProtocol()
    , peerSubject()
    , providerName()
    , providerVersion()
    , resumed( false )
    , peerClosedCleanly( false )
    , handshakeDurationInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsSessionInfo::CTlsSessionInfo( const CTlsSessionInfo& src )
    : protocolVersion( src.protocolVersion )
    , cipherName( src.cipherName )
    , alpnProtocol( src.alpnProtocol )
    , peerSubject( src.peerSubject )
    , providerName( src.providerName )
    , providerVersion( src.providerVersion )
    , resumed( src.resumed )
    , peerClosedCleanly( src.peerClosedCleanly )
    , handshakeDurationInMs( src.handshakeDurationInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsSessionInfo::~CTlsSessionInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsSessionInfo&
CTlsSessionInfo::operator=( const CTlsSessionInfo& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        protocolVersion = src.protocolVersion;
        cipherName = src.cipherName;
        alpnProtocol = src.alpnProtocol;
        peerSubject = src.peerSubject;
        providerName = src.providerName;
        providerVersion = src.providerVersion;
        resumed = src.resumed;
        peerClosedCleanly = src.peerClosedCleanly;
        handshakeDurationInMs = src.handshakeDurationInMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CTlsSessionInfo::Clear( void )
{GUCEF_TRACE;

    *this = CTlsSessionInfo();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTlsSessionInfo::GetSummary( void ) const
{GUCEF_TRACE;

    CORE::CString summary = protocolVersion + " " + cipherName;
    if ( !alpnProtocol.IsNULLOrEmpty() )
        summary += " alpn=" + alpnProtocol;
    summary += " peer=\"" + peerSubject + "\" resumed=" + ( resumed ? "true" : "false" ) + " provider=" + providerName;
    if ( !providerVersion.IsNULLOrEmpty() )
        summary += " " + providerVersion;
    return summary;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
