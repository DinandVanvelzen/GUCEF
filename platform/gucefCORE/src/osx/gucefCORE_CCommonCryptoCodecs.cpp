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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_COsCryptoCodecs.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

#include <string.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonHMAC.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CINSTANCEBASEDCODEC_H
#include "gucefCORE_CInstanceBasedCodec.h"
#define GUCEF_CORE_CINSTANCEBASEDCODEC_H
#endif /* GUCEF_CORE_CINSTANCEBASEDCODEC_H ? */

#ifndef GUCEF_CORE_CCODECINSTANCEBASE_H
#include "gucefCORE_CCodecInstanceBase.h"
#define GUCEF_CORE_CCODECINSTANCEBASE_H
#endif /* GUCEF_CORE_CCODECINSTANCEBASE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANCES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

enum ECommonCryptoAlgorithm
{
    COMMONCRYPTO_SHA256 = 0,
    COMMONCRYPTO_SHA512,
    COMMONCRYPTO_HMAC_SHA256
};

/*-------------------------------------------------------------------------*/

/** A hash or HMAC in progress, the contexts are plain structs so a call does not allocate */
class CCommonCryptoHashInstance : public CCodecInstanceBase
{
    public:

    CCommonCryptoHashInstance( ECommonCryptoAlgorithm algorithm, const void* key, UInt32 keySize, bool hexOutput )
        : CCodecInstanceBase()
        , m_algorithm( algorithm )
        , m_hexOutput( hexOutput )
    {GUCEF_TRACE;

        memset( &m_sha256, 0, sizeof( m_sha256 ) );
        memset( &m_sha512, 0, sizeof( m_sha512 ) );
        memset( &m_hmac, 0, sizeof( m_hmac ) );
        memset( &m_hmacStart, 0, sizeof( m_hmacStart ) );
        if ( COMMONCRYPTO_HMAC_SHA256 == m_algorithm )
            CCHmacInit( &m_hmacStart, kCCHmacAlgSHA256, key, keySize );
        Reset();
    }

    virtual ~CCommonCryptoHashInstance() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // the key is in the HMAC contexts
        memset( &m_hmac, 0, sizeof( m_hmac ) );
        memset( &m_hmacStart, 0, sizeof( m_hmacStart ) );
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool encode = true;
        if ( !CheckDirection( flags, true, false, encode ) )
            return false;

        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
            Reset();
        if ( GUCEF_NULL != input && inputSize > 0 )
        {
            switch ( m_algorithm )
            {
                case COMMONCRYPTO_SHA256 : CC_SHA256_Update( &m_sha256, input, (CC_LONG) inputSize ); break;
                case COMMONCRYPTO_SHA512 : CC_SHA512_Update( &m_sha512, input, (CC_LONG) inputSize ); break;
                default : CCHmacUpdate( &m_hmac, input, inputSize ); break;
            }
        }
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
        {
            const UInt32 digestSize = GetDigestSize();
            UInt8* out = GUCEF_NULL;
            if ( !ReserveOutput( output, m_hexOutput ? 2 * digestSize : digestSize, out ) )
                return false;

            unsigned char digest[ CC_SHA512_DIGEST_LENGTH ];
            switch ( m_algorithm )
            {
                case COMMONCRYPTO_SHA256 : CC_SHA256_Final( digest, &m_sha256 ); break;
                case COMMONCRYPTO_SHA512 : CC_SHA512_Final( digest, &m_sha512 ); break;
                default : CCHmacFinal( &m_hmac, digest ); break;
            }
            Reset();

            if ( m_hexOutput )
                CInstanceBasedCodec::AppendHex( digest, digestSize, output );
            else
                output.Append( digest, digestSize, true );
        }
        return SetOk();
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return 0;
    }

    private:

    UInt32 GetDigestSize( void ) const
    {GUCEF_TRACE;

        return COMMONCRYPTO_SHA512 == m_algorithm ? CC_SHA512_DIGEST_LENGTH : CC_SHA256_DIGEST_LENGTH;
    }

    void Reset( void )
    {GUCEF_TRACE;

        switch ( m_algorithm )
        {
            case COMMONCRYPTO_SHA256 : CC_SHA256_Init( &m_sha256 ); break;
            case COMMONCRYPTO_SHA512 : CC_SHA512_Init( &m_sha512 ); break;
            default : m_hmac = m_hmacStart; break;
        }
    }

    ECommonCryptoAlgorithm m_algorithm;
    bool m_hexOutput;
    CC_SHA256_CTX m_sha256;
    CC_SHA512_CTX m_sha512;
    CCHmacContext m_hmac;
    CCHmacContext m_hmacStart;      /**< the HMAC with only the key taken in, where every message starts */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CODECS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A hash ("sha256", "sha512") or HMAC ("hmac-sha256") of CommonCrypto */
class CCommonCryptoHashCodec : public CInstanceBasedCodec
{
    public:

    CCommonCryptoHashCodec( const CString& familyName, const CString& typeName, ECommonCryptoAlgorithm algorithm )
        : CInstanceBasedCodec( familyName, typeName, GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
        , m_algorithm( algorithm )
    {GUCEF_TRACE;
    }

    CCommonCryptoHashCodec( const CCommonCryptoHashCodec& src )
        : CInstanceBasedCodec( src )
        , m_algorithm( src.m_algorithm )
    {GUCEF_TRACE;
    }

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( COMMONCRYPTO_HMAC_SHA256 == m_algorithm )
            info.push_back( CCodecParameterInfo( "key", GUCEF_DATATYPE_BINARY_BLOB, true, CVariant(), "The secret key" ) );
        else
            info.push_back( CCodecParameterInfo( "output", GUCEF_DATATYPE_STRING, false, CVariant( "hex" ), "\"hex\" for the digest as lower case hexadecimal text, \"raw\" for the digest bytes" ) );
    }

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW CCommonCryptoHashCodec( *this );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CString classTypeName = "GUCEF::CORE::CCommonCryptoHashCodec";
        return classTypeName;
    }

    protected:

    virtual TCodecStatus CreateInstanceImpl( const CValueList& params, CICodecInstancePtr& instance, CString& errorMessage ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool hexOutput = false;
        const void* key = GUCEF_NULL;
        UInt32 keySize = 0;
        if ( COMMONCRYPTO_HMAC_SHA256 == m_algorithm )
        {
            if ( !TryGetBinaryParam( params, "key", key, keySize ) )
            {
                errorMessage = "The parameter \"key\" is required";
                return GUCEF_CODEC_STATUS_MISSING_PARAMETER;
            }
        }
        else
        {
            bool isValid = true;
            hexOutput = IsHexOutputRequested( params, true, isValid );
            if ( !isValid )
            {
                errorMessage = "The parameter \"output\" must be \"hex\" or \"raw\"";
                return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
            }
        }

        instance = CICodecInstancePtr( GUCEF_NEW CCommonCryptoHashInstance( m_algorithm, key, keySize, hexOutput ) );
        return GUCEF_CODEC_STATUS_OK;
    }

    private:

    ECommonCryptoAlgorithm m_algorithm;
};

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
COsCryptoCodecs::CreateCodecs( CCodecRegistry::TICodecPtrVector& codecs )
{GUCEF_TRACE;

    // CommonCrypto is part of the system library, its algorithms are always there
    codecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CCommonCryptoHashCodec( CoreCodecTypes::CryptographicHashCodec, "sha256", COMMONCRYPTO_SHA256 ) ) );
    codecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CCommonCryptoHashCodec( CoreCodecTypes::CryptographicHashCodec, "sha512", COMMONCRYPTO_SHA512 ) ) );
    codecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CCommonCryptoHashCodec( CoreCodecTypes::MacCodec, "hmac-sha256", COMMONCRYPTO_HMAC_SHA256 ) ) );
}

/*-------------------------------------------------------------------------*/

const char*
COsCryptoCodecs::GetProviderName( void )
{GUCEF_TRACE;

    return "CommonCrypto";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ? */
