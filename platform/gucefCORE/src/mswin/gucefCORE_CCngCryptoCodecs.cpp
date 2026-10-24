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

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <bcrypt.h>

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

#ifndef NT_SUCCESS
  #define NT_SUCCESS( status ) ( ( (NTSTATUS) (status) ) >= 0 )
#endif
#ifndef BCRYPT_HASH_REUSABLE_FLAG
  #define BCRYPT_HASH_REUSABLE_FLAG 0x00000020     /* Windows 8 and later */
#endif
#ifndef BCRYPT_ECC_CURVE_25519
  #define BCRYPT_ECC_CURVE_25519 L"curve25519"                /* Windows 10 and later */
#endif
#ifndef BCRYPT_ECC_CURVE_NAME
  #define BCRYPT_ECC_CURVE_NAME L"ECCCurveName"
#endif
#ifndef BCRYPT_ECDH_PUBLIC_GENERIC_MAGIC
  #define BCRYPT_ECDH_PUBLIC_GENERIC_MAGIC 0x504B4345         /* ECKP */
#endif
#ifndef BCRYPT_ECDH_PRIVATE_GENERIC_MAGIC
  #define BCRYPT_ECDH_PRIVATE_GENERIC_MAGIC 0x564B4345        /* ECKV */
#endif
#ifndef STATUS_AUTH_TAG_MISMATCH
  #define STATUS_AUTH_TAG_MISMATCH ( (NTSTATUS) 0xC000A002L )
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const UInt32 AeadKeySize = 32;
static const UInt32 KeySize = 32;            /* x25519 keys and secrets */
static const UInt32 AeadNonceSize = 12;
static const UInt32 AeadTagSize = 16;
static const UInt32 MaxDigestSize = 64;

/*-------------------------------------------------------------------------*/

/** Writes "<what>: NTSTATUS 0x..." into 'text' */
void
DescribeStatus( const char* what, NTSTATUS status, char* text, size_t textSize )
{GUCEF_TRACE;

    _snprintf_s( text, textSize, _TRUNCATE, "%s: NTSTATUS 0x%08lX", what, (unsigned long) status );
}

/*-------------------------------------------------------------------------*/

/** An algorithm provider of CNG, NULL when Windows does not have it */
BCRYPT_ALG_HANDLE
OpenAlgorithm( LPCWSTR algorithmId, ULONG flags )
{GUCEF_TRACE;

    BCRYPT_ALG_HANDLE algorithm = GUCEF_NULL;
    if ( !NT_SUCCESS( BCryptOpenAlgorithmProvider( &algorithm, algorithmId, GUCEF_NULL, flags ) ) )
        return GUCEF_NULL;
    return algorithm;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANCES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A hash or HMAC in progress: a reusable CNG hash object, FINAL finishes it and it is ready for the next message */
class CCngHashInstance : public CCodecInstanceBase
{
    public:

    CCngHashInstance( BCRYPT_ALG_HANDLE algorithm, BCRYPT_HASH_HANDLE hash, UInt32 digestSize, bool hexOutput )
        : CCodecInstanceBase()
        , m_algorithm( algorithm )
        , m_hash( hash )
        , m_digestSize( digestSize )
        , m_hexOutput( hexOutput )
        , m_inMessage( false )
    {GUCEF_TRACE;
    }

    virtual ~CCngHashInstance() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_hash )
            BCryptDestroyHash( m_hash );
        if ( GUCEF_NULL != m_algorithm )
            BCryptCloseAlgorithmProvider( m_algorithm, 0 );
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool encode = true;
        if ( !CheckDirection( flags, true, false, encode ) )
            return false;

        UCHAR digest[ MaxDigestSize ];
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) && m_inMessage )
        {
            // an unfinished message is dropped: finish it and throw the digest away
            m_inMessage = false;
            NTSTATUS status = BCryptFinishHash( m_hash, digest, m_digestSize, 0 );
            if ( !NT_SUCCESS( status ) )
                return SetNtStatus( "Unable to drop the unfinished message", status );
        }

        if ( GUCEF_NULL != input && inputSize > 0 )
        {
            NTSTATUS status = BCryptHashData( m_hash, (PUCHAR) input, inputSize, 0 );
            if ( !NT_SUCCESS( status ) )
                return SetNtStatus( "Unable to hash the input", status );
            m_inMessage = true;
        }

        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
        {
            UInt8* out = GUCEF_NULL;
            if ( !ReserveOutput( output, m_hexOutput ? 2 * m_digestSize : m_digestSize, out ) )
                return false;

            m_inMessage = false;
            NTSTATUS status = BCryptFinishHash( m_hash, digest, m_digestSize, 0 );
            if ( !NT_SUCCESS( status ) )
                return SetNtStatus( "Unable to finish the hash", status );

            if ( m_hexOutput )
                CInstanceBasedCodec::AppendHex( digest, m_digestSize, output );
            else
                output.Append( digest, m_digestSize, true );
        }
        return SetOk();
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return 0;
    }

    private:

    bool SetNtStatus( const char* what, NTSTATUS status )
    {GUCEF_TRACE;

        char text[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
        DescribeStatus( what, status, text, sizeof( text ) );
        return SetStatus( GUCEF_CODEC_STATUS_FAILED, text );
    }

    BCRYPT_ALG_HANDLE m_algorithm;
    BCRYPT_HASH_HANDLE m_hash;
    UInt32 m_digestSize;
    bool m_hexOutput;
    bool m_inMessage;
};

/*-------------------------------------------------------------------------*/

/** AES-256-GCM with its key: one BCryptEncrypt / BCryptDecrypt per message, the tag after the ciphertext */
class CCngAesGcmInstance : public CCodecInstanceBase
{
    public:

    CCngAesGcmInstance( BCRYPT_ALG_HANDLE algorithm, BCRYPT_KEY_HANDLE key )
        : CCodecInstanceBase()
        , m_algorithm( algorithm )
        , m_key( key )
    {GUCEF_TRACE;
    }

    virtual ~CCngAesGcmInstance() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_key )
            BCryptDestroyKey( m_key );
        if ( GUCEF_NULL != m_algorithm )
            BCryptCloseAlgorithmProvider( m_algorithm, 0 );
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool encode = true;
        if ( !CheckDirection( flags, true, true, encode ) )
            return false;
        if ( ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL ) != ( flags & ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL ) ) )
            return SetStatus( GUCEF_CODEC_STATUS_UNSUPPORTED, "An AEAD cipher takes a whole message per call: BEGIN and FINAL" );
        if ( GUCEF_NULL == args || args->structSize < sizeof( SCodecCallArgs ) || GUCEF_NULL == args->nonce )
            return SetStatus( GUCEF_CODEC_STATUS_MISSING_PARAMETER, "A nonce is required" );
        if ( AeadNonceSize != args->nonceSize )
            return SetStatus( GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The nonce must be 12 bytes" );
        if ( GUCEF_NULL == input && inputSize > 0 )
            return SetStatus( GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No input data" );
        if ( !encode && inputSize < AeadTagSize )
            return SetStatus( GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is shorter than the tag" );

        const UInt32 dataSize = encode ? inputSize : inputSize - AeadTagSize;
        const UInt32 outputSize = encode ? inputSize + AeadTagSize : dataSize;
        if ( encode && outputSize < inputSize )
            return SetStatus( GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is too large" );

        UInt8* out = GUCEF_NULL;
        if ( !ReserveOutput( output, outputSize, out ) )
            return false;

        BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO modeInfo;
        BCRYPT_INIT_AUTH_MODE_INFO( modeInfo );
        modeInfo.pbNonce = (PUCHAR) args->nonce;
        modeInfo.cbNonce = AeadNonceSize;
        modeInfo.pbAuthData = (PUCHAR) ( GUCEF_NULL != args->aad && args->aadSize > 0 ? args->aad : GUCEF_NULL );
        modeInfo.cbAuthData = GUCEF_NULL != args->aad ? args->aadSize : 0;
        modeInfo.cbTag = AeadTagSize;

        ULONG written = 0;
        NTSTATUS status = 0;
        if ( encode )
        {
            modeInfo.pbTag = out + dataSize;
            status = BCryptEncrypt( m_key, (PUCHAR) input, dataSize, &modeInfo, GUCEF_NULL, 0, out, dataSize, &written, 0 );
        }
        else
        {
            modeInfo.pbTag = (PUCHAR) static_cast< const UInt8* >( input ) + dataSize;
            status = BCryptDecrypt( m_key, (PUCHAR) input, dataSize, &modeInfo, GUCEF_NULL, 0, out, dataSize, &written, 0 );
        }

        if ( STATUS_AUTH_TAG_MISMATCH == status )
            return SetStatus( GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED, "The tag does not match: the message is not authentic" );
        if ( !NT_SUCCESS( status ) || written != dataSize )
        {
            char text[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
            DescribeStatus( encode ? "CNG: unable to encrypt" : "CNG: unable to decrypt", status, text, sizeof( text ) );
            return SetStatus( GUCEF_CODEC_STATUS_FAILED, text );
        }

        output.SetDataSize( output.GetDataSize() + outputSize );
        return SetOk();
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return AeadTagSize;
    }

    private:

    BCRYPT_ALG_HANDLE m_algorithm;
    BCRYPT_KEY_HANDLE m_key;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CODECS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A hash ("sha256", "sha512") or HMAC ("hmac-sha256") of CNG */
class CCngHashCodec : public CInstanceBasedCodec
{
    public:

    CCngHashCodec( const CString& familyName, const CString& typeName, LPCWSTR algorithmId, UInt32 digestSize, bool isMac )
        : CInstanceBasedCodec( familyName, typeName, GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
        , m_algorithmId( algorithmId )
        , m_digestSize( digestSize )
        , m_isMac( isMac )
    {GUCEF_TRACE;
    }

    CCngHashCodec( const CCngHashCodec& src )
        : CInstanceBasedCodec( src )
        , m_algorithmId( src.m_algorithmId )
        , m_digestSize( src.m_digestSize )
        , m_isMac( src.m_isMac )
    {GUCEF_TRACE;
    }

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_isMac )
            info.push_back( CCodecParameterInfo( "key", GUCEF_DATATYPE_BINARY_BLOB, true, CVariant(), "The secret key" ) );
        else
            info.push_back( CCodecParameterInfo( "output", GUCEF_DATATYPE_STRING, false, CVariant( "hex" ), "\"hex\" for the digest as lower case hexadecimal text, \"raw\" for the digest bytes" ) );
    }

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW CCngHashCodec( *this );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CString classTypeName = "GUCEF::CORE::CCngHashCodec";
        return classTypeName;
    }

    protected:

    virtual TCodecStatus CreateInstanceImpl( const CValueList& params, CICodecInstancePtr& instance, CString& errorMessage ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool hexOutput = false;
        const void* key = GUCEF_NULL;
        UInt32 keySize = 0;
        if ( m_isMac )
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

        BCRYPT_ALG_HANDLE algorithm = OpenAlgorithm( m_algorithmId, m_isMac ? BCRYPT_ALG_HANDLE_HMAC_FLAG : 0 );
        if ( GUCEF_NULL == algorithm )
        {
            errorMessage = "CNG: the algorithm is not available";
            return GUCEF_CODEC_STATUS_FAILED;
        }

        // reusable: after BCryptFinishHash the object starts the next message with the same key, no allocation per message
        BCRYPT_HASH_HANDLE hash = GUCEF_NULL;
        NTSTATUS status = BCryptCreateHash( algorithm, &hash, GUCEF_NULL, 0, (PUCHAR) key, keySize, BCRYPT_HASH_REUSABLE_FLAG );
        if ( !NT_SUCCESS( status ) )
        {
            char text[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
            DescribeStatus( "CNG: unable to create the hash", status, text, sizeof( text ) );
            errorMessage = text;
            BCryptCloseAlgorithmProvider( algorithm, 0 );
            return m_isMac ? GUCEF_CODEC_STATUS_INVALID_PARAMETER : GUCEF_CODEC_STATUS_FAILED;
        }

        instance = CICodecInstancePtr( GUCEF_NEW CCngHashInstance( algorithm, hash, m_digestSize, hexOutput ) );
        return GUCEF_CODEC_STATUS_OK;
    }

    private:

    LPCWSTR m_algorithmId;
    UInt32 m_digestSize;
    bool m_isMac;
};

/*-------------------------------------------------------------------------*/

/** "aes-256-gcm" of CNG: a 32 byte key, a 12 byte nonce and a 16 byte tag */
class CCngAesGcmCodec : public CInstanceBasedCodec
{
    public:

    CCngAesGcmCodec( void )
        : CInstanceBasedCodec( CoreCodecTypes::EncryptionCodec, "aes-256-gcm", GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC | GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY )
    {GUCEF_TRACE;
    }

    CCngAesGcmCodec( const CCngAesGcmCodec& src )
        : CInstanceBasedCodec( src )
    {GUCEF_TRACE;
    }

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        info.push_back( CCodecParameterInfo( "key", GUCEF_DATATYPE_BINARY_BLOB, true, CVariant(), "The secret key, 32 bytes" ) );
        info.push_back( CCodecParameterInfo( "nonce", GUCEF_DATATYPE_BINARY_BLOB, false, CVariant(), "12 bytes, never twice with the same key; required for one shot use, per call for instances" ) );
        info.push_back( CCodecParameterInfo( "aad", GUCEF_DATATYPE_BINARY_BLOB, false, CVariant(), "Associated data: authenticated, not encrypted" ) );
    }

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW CCngAesGcmCodec( *this );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CString classTypeName = "GUCEF::CORE::CCngAesGcmCodec";
        return classTypeName;
    }

    /** An AES provider in GCM mode, NULL when Windows does not have it */
    static BCRYPT_ALG_HANDLE OpenGcm( void )
    {GUCEF_TRACE;

        BCRYPT_ALG_HANDLE algorithm = OpenAlgorithm( BCRYPT_AES_ALGORITHM, 0 );
        if ( GUCEF_NULL == algorithm )
            return GUCEF_NULL;
        if ( !NT_SUCCESS( BCryptSetProperty( algorithm, BCRYPT_CHAINING_MODE, (PUCHAR) BCRYPT_CHAIN_MODE_GCM, sizeof( BCRYPT_CHAIN_MODE_GCM ), 0 ) ) )
        {
            BCryptCloseAlgorithmProvider( algorithm, 0 );
            return GUCEF_NULL;
        }
        return algorithm;
    }

    protected:

    virtual TCodecStatus CreateInstanceImpl( const CValueList& params, CICodecInstancePtr& instance, CString& errorMessage ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const void* key = GUCEF_NULL;
        UInt32 keySize = 0;
        if ( !TryGetBinaryParam( params, "key", key, keySize ) )
        {
            errorMessage = "The parameter \"key\" is required";
            return GUCEF_CODEC_STATUS_MISSING_PARAMETER;
        }
        if ( AeadKeySize != keySize )
        {
            errorMessage = "The parameter \"key\" must be 32 bytes";
            return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
        }

        BCRYPT_ALG_HANDLE algorithm = OpenGcm();
        if ( GUCEF_NULL == algorithm )
        {
            errorMessage = "CNG: AES-GCM is not available";
            return GUCEF_CODEC_STATUS_FAILED;
        }
        BCRYPT_KEY_HANDLE keyHandle = GUCEF_NULL;
        NTSTATUS status = BCryptGenerateSymmetricKey( algorithm, &keyHandle, GUCEF_NULL, 0, (PUCHAR) key, keySize, 0 );
        if ( !NT_SUCCESS( status ) )
        {
            char text[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
            DescribeStatus( "CNG: the key was refused", status, text, sizeof( text ) );
            errorMessage = text;
            BCryptCloseAlgorithmProvider( algorithm, 0 );
            return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
        }

        instance = CICodecInstancePtr( GUCEF_NEW CCngAesGcmInstance( algorithm, keyHandle ) );
        return GUCEF_CODEC_STATUS_OK;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  "x25519" of CNG: ECDH on curve25519 (Windows 10 and later).
 *  Keys and the shared secret are in the little endian form of RFC 7748: the private scalar goes in as 'd' of a generic
 *  ECDH private blob, the public key is the X of a public blob (Y is not used on this curve), and the raw secret of
 *  BCRYPT_KDF_RAW_SECRET is little endian already.
 */
class CCngX25519Codec : public CICodec
{
    public:

    CCngX25519Codec( void )
        : CICodec()
    {GUCEF_TRACE;
    }

    CCngX25519Codec( const CCngX25519Codec& src )
        : CICodec( src )
    {GUCEF_TRACE;
    }

    virtual bool Encode( CIOAccess& source, CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CValueList noParams;
        return Encode( source, dest, noParams );
    }

    virtual bool Decode( CIOAccess& source, CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CValueList noParams;
        return Decode( source, dest, noParams );
    }

    /** own private key -> own public key */
    virtual bool Encode( CIOAccess& source, CIOAccess& dest, const CValueList& params ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        UInt8 privateKey[ KeySize ];
        if ( !ReadKey( source, privateKey ) )
            return Fail( GUCEF_CODEC_STATUS_INVALID_INPUT, "The input must be a private key of 32 bytes" );

        BCRYPT_ALG_HANDLE algorithm = GUCEF_NULL;
        BCRYPT_KEY_HANDLE key = GUCEF_NULL;
        TCodecStatus status = ImportPrivateKey( privateKey, algorithm, key );
        SecureZeroMemory( privateKey, sizeof( privateKey ) );
        if ( GUCEF_CODEC_STATUS_OK != status )
            return Fail( status, "CNG: the private key was refused" );

        UCHAR blob[ sizeof( BCRYPT_ECCKEY_BLOB ) + 2 * KeySize ];
        ULONG blobSize = 0;
        NTSTATUS ntStatus = BCryptExportKey( key, GUCEF_NULL, BCRYPT_ECCPUBLIC_BLOB, blob, sizeof( blob ), &blobSize, 0 );
        BCryptDestroyKey( key );
        BCryptCloseAlgorithmProvider( algorithm, 0 );
        if ( !NT_SUCCESS( ntStatus ) || blobSize < sizeof( BCRYPT_ECCKEY_BLOB ) + KeySize )
            return Fail( GUCEF_CODEC_STATUS_FAILED, "CNG: unable to export the public key" );

        if ( KeySize != dest.Write( blob + sizeof( BCRYPT_ECCKEY_BLOB ), 1, KeySize ) )
            return Fail( GUCEF_CODEC_STATUS_FAILED, "Unable to write the output" );
        SetLastStatus( GUCEF_CODEC_STATUS_OK );
        return true;
    }

    /** peer public key with the parameter "privateKey" -> the shared secret */
    virtual bool Decode( CIOAccess& source, CIOAccess& dest, const CValueList& params ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const void* privateKeyParam = GUCEF_NULL;
        UInt32 privateKeySize = 0;
        if ( !CInstanceBasedCodec::TryGetBinaryParam( params, "privateKey", privateKeyParam, privateKeySize ) )
            return Fail( GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"privateKey\" is required" );
        if ( KeySize != privateKeySize )
            return Fail( GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"privateKey\" must be 32 bytes" );
        UInt8 peerPublicKey[ KeySize ];
        if ( !ReadKey( source, peerPublicKey ) )
            return Fail( GUCEF_CODEC_STATUS_INVALID_INPUT, "The input must be a public key of 32 bytes" );

        BCRYPT_ALG_HANDLE algorithm = GUCEF_NULL;
        BCRYPT_KEY_HANDLE ownKey = GUCEF_NULL;
        TCodecStatus status = ImportPrivateKey( static_cast< const UInt8* >( privateKeyParam ), algorithm, ownKey );
        if ( GUCEF_CODEC_STATUS_OK != status )
            return Fail( status, "CNG: the private key was refused" );

        UCHAR publicBlob[ sizeof( BCRYPT_ECCKEY_BLOB ) + 2 * KeySize ];
        memset( publicBlob, 0, sizeof( publicBlob ) );
        BCRYPT_ECCKEY_BLOB* header = reinterpret_cast< BCRYPT_ECCKEY_BLOB* >( publicBlob );
        header->dwMagic = BCRYPT_ECDH_PUBLIC_GENERIC_MAGIC;
        header->cbKey = KeySize;
        memcpy( publicBlob + sizeof( BCRYPT_ECCKEY_BLOB ), peerPublicKey, KeySize );

        BCRYPT_KEY_HANDLE peerKey = GUCEF_NULL;
        BCRYPT_SECRET_HANDLE secret = GUCEF_NULL;
        UInt8 sharedSecret[ KeySize ];
        ULONG secretSize = 0;
        status = GUCEF_CODEC_STATUS_OK;
        if ( !NT_SUCCESS( BCryptImportKeyPair( algorithm, GUCEF_NULL, BCRYPT_ECCPUBLIC_BLOB, &peerKey, publicBlob, sizeof( publicBlob ), 0 ) ) )
            status = GUCEF_CODEC_STATUS_INVALID_INPUT;
        else
        if ( !NT_SUCCESS( BCryptSecretAgreement( ownKey, peerKey, &secret, 0 ) ) ||
             !NT_SUCCESS( BCryptDeriveKey( secret, BCRYPT_KDF_RAW_SECRET, GUCEF_NULL, sharedSecret, KeySize, &secretSize, 0 ) ) ||
             KeySize != secretSize )
            status = GUCEF_CODEC_STATUS_INVALID_INPUT;
        else
        {
            UInt8 bits = 0;
            for ( UInt32 i=0; i<KeySize; ++i )
                bits |= sharedSecret[ i ];
            if ( 0 == bits )
                status = GUCEF_CODEC_STATUS_INVALID_INPUT;
        }

        if ( GUCEF_NULL != secret )
            BCryptDestroySecret( secret );
        if ( GUCEF_NULL != peerKey )
            BCryptDestroyKey( peerKey );
        BCryptDestroyKey( ownKey );
        BCryptCloseAlgorithmProvider( algorithm, 0 );

        if ( GUCEF_CODEC_STATUS_OK != status )
        {
            SecureZeroMemory( sharedSecret, sizeof( sharedSecret ) );
            return Fail( status, "The public key is refused or a point of low order: no shared secret" );
        }
        bool written = KeySize == dest.Write( sharedSecret, 1, KeySize );
        SecureZeroMemory( sharedSecret, sizeof( sharedSecret ) );
        if ( !written )
            return Fail( GUCEF_CODEC_STATUS_FAILED, "Unable to write the output" );
        SetLastStatus( GUCEF_CODEC_STATUS_OK );
        return true;
    }

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        info.push_back( CCodecParameterInfo( "privateKey", GUCEF_DATATYPE_BINARY_BLOB, false, CVariant(), "Decode: own private key of 32 bytes. Encode takes the private key as input and gives the public key" ) );
    }

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_DETERMINISTIC;
    }

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return "x25519";
    }

    virtual CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return CoreCodecTypes::KeyAgreementCodec;
    }

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW CCngX25519Codec( *this );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CString classTypeName = "GUCEF::CORE::CCngX25519Codec";
        return classTypeName;
    }

    /** An ECDH provider on curve25519, NULL when Windows does not have it */
    static BCRYPT_ALG_HANDLE OpenCurve25519( void )
    {GUCEF_TRACE;

        BCRYPT_ALG_HANDLE algorithm = OpenAlgorithm( BCRYPT_ECDH_ALGORITHM, 0 );
        if ( GUCEF_NULL == algorithm )
            return GUCEF_NULL;
        if ( !NT_SUCCESS( BCryptSetProperty( algorithm, BCRYPT_ECC_CURVE_NAME, (PUCHAR) BCRYPT_ECC_CURVE_25519, sizeof( BCRYPT_ECC_CURVE_25519 ), 0 ) ) )
        {
            BCryptCloseAlgorithmProvider( algorithm, 0 );
            return GUCEF_NULL;
        }
        return algorithm;
    }

    private:

    bool Fail( TCodecStatus status, const char* message )
    {GUCEF_TRACE;

        SetLastStatus( status, message );
        return false;
    }

    /** Exactly 32 bytes, no more */
    static bool ReadKey( CIOAccess& source, UInt8* key )
    {GUCEF_TRACE;

        UInt32 total = 0;
        while ( total < KeySize )
        {
            UInt32 bytesRead = source.Read( key + total, 1, KeySize - total );
            if ( 0 == bytesRead )
                break;
            total += bytesRead;
        }
        UInt8 extra = 0;
        return KeySize == total && 0 == source.Read( &extra, 1, 1 );
    }

    static TCodecStatus ImportPrivateKey( const UInt8* privateKey, BCRYPT_ALG_HANDLE& algorithm, BCRYPT_KEY_HANDLE& key )
    {GUCEF_TRACE;

        algorithm = OpenCurve25519();
        if ( GUCEF_NULL == algorithm )
            return GUCEF_CODEC_STATUS_FAILED;

        // the public part is left to CNG to compute, hence no validation against it
        UCHAR blob[ sizeof( BCRYPT_ECCKEY_BLOB ) + 3 * KeySize ];
        memset( blob, 0, sizeof( blob ) );
        BCRYPT_ECCKEY_BLOB* header = reinterpret_cast< BCRYPT_ECCKEY_BLOB* >( blob );
        header->dwMagic = BCRYPT_ECDH_PRIVATE_GENERIC_MAGIC;
        header->cbKey = KeySize;
        memcpy( blob + sizeof( BCRYPT_ECCKEY_BLOB ) + 2 * KeySize, privateKey, KeySize );
        NTSTATUS status = BCryptImportKeyPair( algorithm, GUCEF_NULL, BCRYPT_ECCPRIVATE_BLOB, &key, blob, sizeof( blob ), BCRYPT_NO_KEY_VALIDATION );
        SecureZeroMemory( blob, sizeof( blob ) );
        if ( !NT_SUCCESS( status ) )
        {
            BCryptCloseAlgorithmProvider( algorithm, 0 );
            algorithm = GUCEF_NULL;
            key = GUCEF_NULL;
            return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
        }
        return GUCEF_CODEC_STATUS_OK;
    }
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

    struct SHashAlgorithm
    {
        const CString* familyName;
        const char* typeName;
        LPCWSTR algorithmId;
        UInt32 digestSize;
        bool isMac;
    };
    static const SHashAlgorithm hashAlgorithms[] =
    {
        { &CoreCodecTypes::CryptographicHashCodec, "sha256", BCRYPT_SHA256_ALGORITHM, 32, false },
        { &CoreCodecTypes::CryptographicHashCodec, "sha512", BCRYPT_SHA512_ALGORITHM, 64, false },
        { &CoreCodecTypes::MacCodec, "hmac-sha256", BCRYPT_SHA256_ALGORITHM, 32, true }
    };
    for ( UInt32 i=0; i<sizeof( hashAlgorithms ) / sizeof( hashAlgorithms[ 0 ] ); ++i )
    {
        const SHashAlgorithm& algorithm = hashAlgorithms[ i ];
        BCRYPT_ALG_HANDLE probe = OpenAlgorithm( algorithm.algorithmId, algorithm.isMac ? BCRYPT_ALG_HANDLE_HMAC_FLAG : 0 );
        if ( GUCEF_NULL == probe )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "OsCryptoCodecs: CNG does not offer \"" << algorithm.typeName << "\", not registered" );
            continue;
        }
        BCryptCloseAlgorithmProvider( probe, 0 );
        codecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CCngHashCodec( *algorithm.familyName, algorithm.typeName, algorithm.algorithmId, algorithm.digestSize, algorithm.isMac ) ) );
    }

    BCRYPT_ALG_HANDLE curve25519 = CCngX25519Codec::OpenCurve25519();
    if ( GUCEF_NULL != curve25519 )
    {
        BCryptCloseAlgorithmProvider( curve25519, 0 );
        codecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CCngX25519Codec() ) );
    }
    else
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "OsCryptoCodecs: CNG does not offer curve25519 (Windows 10 and later), \"x25519\" not registered" );
    }

    BCRYPT_ALG_HANDLE gcm = CCngAesGcmCodec::OpenGcm();
    if ( GUCEF_NULL != gcm )
    {
        BCryptCloseAlgorithmProvider( gcm, 0 );
        codecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CCngAesGcmCodec() ) );
    }
    else
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "OsCryptoCodecs: CNG does not offer AES-GCM, \"aes-256-gcm\" not registered" );
    }
}

/*-------------------------------------------------------------------------*/

const char*
COsCryptoCodecs::GetProviderName( void )
{GUCEF_TRACE;

    return "CNG";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */
