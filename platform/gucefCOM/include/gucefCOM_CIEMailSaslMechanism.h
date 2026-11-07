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

#ifndef GUCEF_COM_CIEMAILSASLMECHANISM_H
#define GUCEF_COM_CIEMAILSASLMECHANISM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
class GUCEF_COM_PUBLIC_CPP CIEMailSaslMechanism
{
    public:

    CIEMailSaslMechanism( void );

    virtual ~CIEMailSaslMechanism();

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

    CIEMailSaslMechanism( const CIEMailSaslMechanism& src );
    CIEMailSaslMechanism& operator=( const CIEMailSaslMechanism& src );

    CORE::CString m_failureReason;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIEMAILSASLMECHANISM_H ? */
