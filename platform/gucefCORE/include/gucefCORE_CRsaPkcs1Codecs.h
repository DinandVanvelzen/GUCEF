/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CRSAPKCS1CODECS_H
#define GUCEF_CORE_CRSAPKCS1CODECS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CINSTANCEBASEDCODEC_H
#include "gucefCORE_CInstanceBasedCodec.h"
#define GUCEF_CORE_CINSTANCEBASEDCODEC_H
#endif /* GUCEF_CORE_CINSTANCEBASEDCODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  SignatureCodec "rsa-pkcs1-sha256", "rsa-pkcs1-sha384" and "rsa-pkcs1-sha512" in portable code:
 *  RSASSA-PKCS1-v1_5 signature verification (RFC 8017, section 8.2), the RS256, RS384 and RS512 of JWS.
 *  Only Decode / verification: Encode is not supported (GUCEF_CODEC_STATUS_UNSUPPORTED).
 *
 *  Parameters (all big endian bytes, as a JWK gives them after base64url decoding):
 *    "modulus"        the public modulus n (required)
 *    "exponent"       the public exponent e (required): odd, 65537 up to 64 bits
 *    "signature"      the signature, exactly as long as the modulus. For instances it can instead come per call
 *                     as the call argument "aad", which overrides the creation parameter
 *    "allowShortKey"  true to accept a modulus of 512 to 2047 bits. For tests and legacy keys only, never taken from
 *                     data an attacker can influence. Default false: the modulus must be 2048 to 8192 bits
 *
 *  Decode takes the message as input and gives no output. Failures: MISSING_PARAMETER, INVALID_PARAMETER (key not
 *  acceptable, signature of the wrong length), AUTHENTICATION_FAILED (signature value not below the modulus, or the
 *  encoded message does not match: the whole padding and the DigestInfo are compared, nothing is parsed leniently).
 *  Instances make no heap allocation per call once created.
 */
class GUCEF_CORE_PUBLIC_CPP CRsaPkcs1VerifyCodec : public CInstanceBasedCodec
{
    public:

    enum EHashAlgorithm
    {
        HASH_SHA256 = 0 ,
        HASH_SHA384     ,
        HASH_SHA512
    };

    static const UInt32 MinModulusBits = 2048;
    static const UInt32 MaxModulusBits = 8192;
    static const UInt32 MinShortKeyModulusBits = 512;
    static const UInt32 MaxExponentBits = 64;
    static const UInt32 MinExponent = 65537;

    /** The type name of the hash variant: "rsa-pkcs1-sha256", "rsa-pkcs1-sha384", "rsa-pkcs1-sha512" */
    static const CString& GetTypeName( EHashAlgorithm hash );

    CRsaPkcs1VerifyCodec( EHashAlgorithm hash );

    CRsaPkcs1VerifyCodec( const CRsaPkcs1VerifyCodec& src );

    virtual ~CRsaPkcs1VerifyCodec() GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    EHashAlgorithm GetHashAlgorithm( void ) const;

    protected:

    virtual TCodecStatus CreateInstanceImpl( const CValueList& params       ,
                                             CICodecInstancePtr& instance   ,
                                             CString& errorMessage          ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRsaPkcs1VerifyCodec& operator=( const CRsaPkcs1VerifyCodec& src );   /**< not implemented */

    private:

    EHashAlgorithm m_hash;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CRSAPKCS1CODECS_H ? */
