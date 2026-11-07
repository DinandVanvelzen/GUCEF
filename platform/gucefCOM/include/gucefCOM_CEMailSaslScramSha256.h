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

#ifndef GUCEF_COM_CEMAILSASLSCRAMSHA256_H
#define GUCEF_COM_CEMAILSASLSCRAMSHA256_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

#ifndef GUCEF_COM_CIEMAILSASLMECHANISM_H
#include "gucefCOM_CIEMailSaslMechanism.h"
#define GUCEF_COM_CIEMAILSASLMECHANISM_H
#endif /* GUCEF_COM_CIEMAILSASLMECHANISM_H ? */

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

/**
 *  SCRAM-SHA-256, RFC 5802 and RFC 7677, without channel binding. The server has to prove that it knows the
 *  password too: its signature is verified. The password is used as it is (SASLprep is not applied, which
 *  makes no difference for ASCII passwords). The iteration count the server asks for is limited
 *  (MinIterations to MaxIterations) so that a server cannot make the client compute for long.
 */
class GUCEF_COM_PUBLIC_CPP CEMailSaslScramSha256 : public CIEMailSaslMechanism
{
    public:

    static const UInt32 MinIterations = 4096;
    static const UInt32 MaxIterations = 262144;

    CEMailSaslScramSha256( const CORE::CString& userName, const CORE::CString& password );
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILSASLSCRAMSHA256_H ? */
