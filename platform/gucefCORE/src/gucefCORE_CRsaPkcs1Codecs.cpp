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

#include <string.h>

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

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CSHA512_H
#include "gucefCORE_CSha512.h"
#define GUCEF_CORE_CSHA512_H
#endif /* GUCEF_CORE_CSHA512_H ? */

#ifndef GUCEF_CORE_CBIGUNSIGNED_H
#include "gucefCORE_CBigUnsigned.h"
#define GUCEF_CORE_CBIGUNSIGNED_H
#endif /* GUCEF_CORE_CBIGUNSIGNED_H ? */

#ifndef GUCEF_CORE_CCODECINSTANCEBASE_H
#include "gucefCORE_CCodecInstanceBase.h"
#define GUCEF_CORE_CCODECINSTANCEBASE_H
#endif /* GUCEF_CORE_CCODECINSTANCEBASE_H ? */

#include "gucefCORE_CRsaPkcs1Codecs.h"

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

/*-------------------------------------------------------------------------*/

/** The DER of DigestInfo up to the digest, RFC 8017 section 9.2 note 1 */
static const UInt8 g_digestInfoPrefixSha256[ 19 ] =
{
    0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
};
static const UInt8 g_digestInfoPrefixSha384[ 19 ] =
{
    0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30
};
static const UInt8 g_digestInfoPrefixSha512[ 19 ] =
{
    0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40
};

static const UInt32 g_digestInfoPrefixSize = 19;

/*-------------------------------------------------------------------------*/

/** A message hash that is verified against an RSA signature, with its key */
class CRsaPkcs1VerifyInstance : public CCodecInstanceBase
{
    public:

    CRsaPkcs1VerifyInstance( CRsaPkcs1VerifyCodec::EHashAlgorithm hash )
        : CCodecInstanceBase()
        , m_hash( hash )
        , m_sha256()
        , m_sha384( true )
        , m_sha512( false )
        , m_modulus()
        , m_exponent()
        , m_modulusBytes( 0 )
        , m_digestSize( 0 )
        , m_signature()
        , m_hasSignature( false )
        , m_signatureValue()
        , m_recovered()
        , m_recoveredBytes()
        , m_expected()
    {GUCEF_TRACE;

    }

    /** Takes in the key and the optional signature, and makes the part of the expected encoding that does not change */
    TCodecStatus Setup( const CBigUnsigned& modulus, const CBigUnsigned& exponent, const void* signature, UInt32 signatureSize, CString& errorMessage )
    {GUCEF_TRACE;

        switch ( m_hash )
        {
            case CRsaPkcs1VerifyCodec::HASH_SHA256 : m_digestSize = CSha256::DigestSize; break;
            case CRsaPkcs1VerifyCodec::HASH_SHA384 : m_digestSize = CSha512::Digest384Size; break;
            default : m_digestSize = CSha512::DigestSize; break;
        }

        m_modulusBytes = modulus.GetByteLength();
        if ( m_modulusBytes < g_digestInfoPrefixSize + m_digestSize + 11 )
        {
            errorMessage = "The modulus is too small for the hash";
            return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
        }
        if ( !m_modulus.Initialize( modulus ) )
        {
            errorMessage = "The modulus must be odd";
            return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
        }
        m_exponent = exponent;

        // EM = 0x00 0x01 FF ... FF 0x00 DigestInfo, only the digest at the end differs per message
        m_expected.assign( m_modulusBytes, 0xFF );
        m_expected[ 0 ] = 0x00;
        m_expected[ 1 ] = 0x01;
        const UInt32 tLength = g_digestInfoPrefixSize + m_digestSize;
        m_expected[ m_modulusBytes - tLength - 1 ] = 0x00;
        const UInt8* prefix = g_digestInfoPrefixSha512;
        if ( CRsaPkcs1VerifyCodec::HASH_SHA256 == m_hash )
            prefix = g_digestInfoPrefixSha256;
        else
        if ( CRsaPkcs1VerifyCodec::HASH_SHA384 == m_hash )
            prefix = g_digestInfoPrefixSha384;
        memcpy( &m_expected[ m_modulusBytes - tLength ], prefix, g_digestInfoPrefixSize );

        m_recoveredBytes.assign( m_modulusBytes, 0 );
        if ( signatureSize > 0 )
        {
            m_signature.assign( static_cast< const UInt8* >( signature ), static_cast< const UInt8* >( signature ) + signatureSize );
            m_hasSignature = true;
        }
        return GUCEF_CODEC_STATUS_OK;
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool encode = true;
        if ( !CheckDirection( flags, false, true, encode ) )
            return false;

        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
            ResetHash();
        if ( GUCEF_NULL != input && inputSize > 0 )
            UpdateHash( input, inputSize );
        if ( 0 == ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
            return SetOk();

        UInt8 digest[ CSha512::DigestSize ];
        FinalHash( digest );

        // the signature of this call wins over the one given at creation
        const UInt8* signature = GUCEF_NULL;
        UInt32 signatureSize = 0;
        if ( GUCEF_NULL != args && args->structSize >= (UInt32) sizeof( SCodecCallArgs ) && GUCEF_NULL != args->aad && args->aadSize > 0 )
        {
            signature = static_cast< const UInt8* >( args->aad );
            signatureSize = args->aadSize;
        }
        else
        if ( m_hasSignature )
        {
            signature = &m_signature[ 0 ];
            signatureSize = (UInt32) m_signature.size();
        }
        else
        {
            return SetStatus( GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The signature is required: parameter \"signature\" or the call argument \"aad\"" );
        }

        if ( signatureSize != m_modulusBytes )
            return SetStatus( GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The signature must be as long as the modulus" );

        m_signatureValue.SetFromBigEndian( signature, signatureSize );
        if ( m_signatureValue.Compare( m_modulus.GetModulus() ) >= 0 )
            return SetStatus( GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED, "The signature does not match the message and the public key" );

        if ( !m_modulus.ModExp( m_signatureValue, m_exponent, m_recovered ) ||
             !m_recovered.ToBigEndian( &m_recoveredBytes[ 0 ], m_modulusBytes ) )
            return SetStatus( GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED, "The signature does not match the message and the public key" );

        // the whole encoded message is compared, the digest goes at the end of the expected one
        memcpy( &m_expected[ m_modulusBytes - m_digestSize ], digest, m_digestSize );
        if ( !CSha256::ConstantTimeEquals( &m_recoveredBytes[ 0 ], &m_expected[ 0 ], m_modulusBytes ) )
            return SetStatus( GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED, "The signature does not match the message and the public key" );
        return SetOk();
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return 0;
    }

    private:

    void ResetHash( void )
    {GUCEF_TRACE;

        switch ( m_hash )
        {
            case CRsaPkcs1VerifyCodec::HASH_SHA256 : m_sha256.Reset(); break;
            case CRsaPkcs1VerifyCodec::HASH_SHA384 : m_sha384.Reset(); break;
            default : m_sha512.Reset(); break;
        }
    }

    void UpdateHash( const void* data, UInt32 size )
    {GUCEF_TRACE;

        switch ( m_hash )
        {
            case CRsaPkcs1VerifyCodec::HASH_SHA256 : m_sha256.Update( data, size ); break;
            case CRsaPkcs1VerifyCodec::HASH_SHA384 : m_sha384.Update( data, size ); break;
            default : m_sha512.Update( data, size ); break;
        }
    }

    /** Gives the digest and readies the hash for the next message */
    void FinalHash( UInt8* digest )
    {GUCEF_TRACE;

        switch ( m_hash )
        {
            case CRsaPkcs1VerifyCodec::HASH_SHA256 : m_sha256.Final( digest ); m_sha256.Reset(); break;
            case CRsaPkcs1VerifyCodec::HASH_SHA384 : m_sha384.Final( digest ); m_sha384.Reset(); break;
            default : m_sha512.Final( digest ); m_sha512.Reset(); break;
        }
    }

    private:

    CRsaPkcs1VerifyCodec::EHashAlgorithm m_hash;
    CSha256 m_sha256;
    CSha512 m_sha384;
    CSha512 m_sha512;
    CMontgomeryModulus m_modulus;
    CBigUnsigned m_exponent;
    UInt32 m_modulusBytes;
    UInt32 m_digestSize;
    GUCEF::vector< UInt8 > m_signature;
    bool m_hasSignature;
    CBigUnsigned m_signatureValue;
    CBigUnsigned m_recovered;
    GUCEF::vector< UInt8 > m_recoveredBytes;
    GUCEF::vector< UInt8 > m_expected;
};

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
CRsaPkcs1VerifyCodec::GetTypeName( EHashAlgorithm hash )
{GUCEF_TRACE;

    static const CString sha256TypeName = "rsa-pkcs1-sha256";
    static const CString sha384TypeName = "rsa-pkcs1-sha384";
    static const CString sha512TypeName = "rsa-pkcs1-sha512";
    switch ( hash )
    {
        case HASH_SHA256 : return sha256TypeName;
        case HASH_SHA384 : return sha384TypeName;
        default : return sha512TypeName;
    }
}

/*-------------------------------------------------------------------------*/

CRsaPkcs1VerifyCodec::CRsaPkcs1VerifyCodec( EHashAlgorithm hash )
    : CInstanceBasedCodec( CoreCodecTypes::SignatureCodec, GetTypeName( hash ), GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
    , m_hash( hash )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRsaPkcs1VerifyCodec::CRsaPkcs1VerifyCodec( const CRsaPkcs1VerifyCodec& src )
    : CInstanceBasedCodec( src )
    , m_hash( src.m_hash )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRsaPkcs1VerifyCodec::~CRsaPkcs1VerifyCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRsaPkcs1VerifyCodec::EHashAlgorithm
CRsaPkcs1VerifyCodec::GetHashAlgorithm( void ) const
{GUCEF_TRACE;

    return m_hash;
}

/*-------------------------------------------------------------------------*/

void
CRsaPkcs1VerifyCodec::GetParameterInfo( CCodecParameterInfoList& info ) const
{GUCEF_TRACE;

    info.push_back( CCodecParameterInfo( "modulus", GUCEF_DATATYPE_BINARY_BLOB, true, CVariant(), "The public modulus n as big endian bytes, 2048 to 8192 bits and odd" ) );
    info.push_back( CCodecParameterInfo( "exponent", GUCEF_DATATYPE_BINARY_BLOB, true, CVariant(), "The public exponent e as big endian bytes: odd, from 65537 up to 64 bits" ) );
    info.push_back( CCodecParameterInfo( "signature", GUCEF_DATATYPE_BINARY_BLOB, false, CVariant(), "Decode: the signature as big endian bytes, as long as the modulus. Instances can take it per call as the call argument \"aad\" instead" ) );
    info.push_back( CCodecParameterInfo( "allowShortKey", GUCEF_DATATYPE_BOOLEAN_INT32, false, CVariant( false ), "Accept a modulus of 512 to 2047 bits. For tests and legacy keys only, never from data an attacker can influence" ) );
}

/*-------------------------------------------------------------------------*/

TCodecStatus
CRsaPkcs1VerifyCodec::CreateInstanceImpl( const CValueList& params       ,
                                          CICodecInstancePtr& instance   ,
                                          CString& errorMessage          )
{GUCEF_TRACE;

    const void* modulusBytes = GUCEF_NULL;
    UInt32 modulusSize = 0;
    if ( !TryGetBinaryParam( params, "modulus", modulusBytes, modulusSize ) )
    {
        errorMessage = "The parameter \"modulus\" is required";
        return GUCEF_CODEC_STATUS_MISSING_PARAMETER;
    }
    const void* exponentBytes = GUCEF_NULL;
    UInt32 exponentSize = 0;
    if ( !TryGetBinaryParam( params, "exponent", exponentBytes, exponentSize ) )
    {
        errorMessage = "The parameter \"exponent\" is required";
        return GUCEF_CODEC_STATUS_MISSING_PARAMETER;
    }
    const void* signatureBytes = GUCEF_NULL;
    UInt32 signatureSize = 0;
    TryGetBinaryParam( params, "signature", signatureBytes, signatureSize );

    bool allowShortKey = false;
    if ( params.HasKey( "allowShortKey" ) )
        allowShortKey = params.GetValue( "allowShortKey" ).AsBool( false );

    CBigUnsigned modulus;
    CBigUnsigned exponent;
    modulus.SetFromBigEndian( modulusBytes, modulusSize );
    exponent.SetFromBigEndian( exponentBytes, exponentSize );

    const UInt32 minBits = allowShortKey ? MinShortKeyModulusBits : MinModulusBits;
    const UInt32 modulusBits = modulus.GetBitLength();
    if ( modulusBits < minBits || modulusBits > MaxModulusBits )
    {
        errorMessage = allowShortKey ? "The modulus must be 512 to 8192 bits" : "The modulus must be 2048 to 8192 bits";
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    }
    if ( !modulus.IsOdd() )
    {
        errorMessage = "The modulus must be odd";
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    }

    UInt64 exponentValue = 0;
    if ( !exponent.TryGetUInt64( exponentValue ) || exponentValue < MinExponent || !exponent.IsOdd() )
    {
        errorMessage = "The exponent must be odd, at least 65537 and at most 64 bits";
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    }

    CRsaPkcs1VerifyInstance* verifier = GUCEF_NEW CRsaPkcs1VerifyInstance( m_hash );
    CICodecInstancePtr verifierPtr( verifier );
    TCodecStatus status = verifier->Setup( modulus, exponent, signatureBytes, signatureSize, errorMessage );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;
    instance = verifierPtr;
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CRsaPkcs1VerifyCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRsaPkcs1VerifyCodec( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CRsaPkcs1VerifyCodec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CRsaPkcs1VerifyCodec";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
