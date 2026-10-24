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

#ifndef GUCEF_CORE_CCODECINSTANCEBASE_H
#include "gucefCORE_CCodecInstanceBase.h"
#define GUCEF_CORE_CCODECINSTANCEBASE_H
#endif /* GUCEF_CORE_CCODECINSTANCEBASE_H ? */

#include "gucefCORE_CSoftwareCryptoCodecs.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CSha256Codec::TypeName = "sha256";
const CString CHmacSha256Codec::TypeName = "hmac-sha256";
const CString CHkdfSha256Codec::TypeName = "hkdf-sha256";

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANCES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/*-------------------------------------------------------------------------*/

/** SHA-256 in progress */
class CSha256CodecInstance : public CCodecInstanceBase
{
    public:

    CSha256CodecInstance( bool hexOutput )
        : CCodecInstanceBase()
        , m_sha()
        , m_hexOutput( hexOutput )
    {GUCEF_TRACE;
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool encode = true;
        if ( !CheckDirection( flags, true, false, encode ) )
            return false;

        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
            m_sha.Reset();
        if ( GUCEF_NULL != input && inputSize > 0 )
            m_sha.Update( input, inputSize );
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
        {
            UInt8* out = GUCEF_NULL;
            if ( !ReserveOutput( output, m_hexOutput ? 2 * CSha256::DigestSize : CSha256::DigestSize, out ) )
                return false;

            UInt8 digest[ CSha256::DigestSize ];
            m_sha.Final( digest );
            m_sha.Reset();
            if ( m_hexOutput )
                CInstanceBasedCodec::AppendHex( digest, CSha256::DigestSize, output );
            else
                output.Append( digest, CSha256::DigestSize, true );
        }
        return SetOk();
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return 0;
    }

    private:

    CSha256 m_sha;
    bool m_hexOutput;
};

/*-------------------------------------------------------------------------*/

/** HMAC-SHA256 in progress, with its key */
class CHmacSha256CodecInstance : public CCodecInstanceBase
{
    public:

    CHmacSha256CodecInstance( const void* key, UInt32 keySize )
        : CCodecInstanceBase()
        , m_hmac( key, keySize )
    {GUCEF_TRACE;
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool encode = true;
        if ( !CheckDirection( flags, true, false, encode ) )
            return false;

        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
            m_hmac.Reset();
        if ( GUCEF_NULL != input && inputSize > 0 )
            m_hmac.Update( input, inputSize );
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
        {
            UInt8* out = GUCEF_NULL;
            if ( !ReserveOutput( output, CHmacSha256::MacSize, out ) )
                return false;
            m_hmac.Final( out );
            output.SetDataSize( output.GetDataSize() + CHmacSha256::MacSize );
        }
        return SetOk();
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return 0;
    }

    private:

    CHmacSha256 m_hmac;
};

/*-------------------------------------------------------------------------*/

/** HKDF-SHA256: the input secret goes into the extract step as it comes, FINAL expands */
class CHkdfSha256CodecInstance : public CCodecInstanceBase
{
    public:

    CHkdfSha256CodecInstance( const void* salt, UInt32 saltSize, const void* info, UInt32 infoSize, UInt32 length )
        : CCodecInstanceBase()
        , m_extract( salt, saltSize )
        , m_info()
        , m_length( length )
    {GUCEF_TRACE;

        if ( infoSize > 0 )
            m_info.CopyFrom( infoSize, info );
    }

    virtual ~CHkdfSha256CodecInstance() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_info.GetBufferSize() > 0 )
            memset( m_info.GetBufferPtr(), 0, m_info.GetBufferSize() );
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool encode = true;
        if ( !CheckDirection( flags, true, false, encode ) )
            return false;

        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
            m_extract.Reset();
        if ( GUCEF_NULL != input && inputSize > 0 )
            m_extract.Update( input, inputSize );
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
        {
            UInt8* out = GUCEF_NULL;
            if ( !ReserveOutput( output, m_length, out ) )
                return false;

            UInt8 prk[ CSha256::DigestSize ];
            m_extract.Final( prk );
            bool expanded = CHkdfSha256::Expand( prk, m_info.GetConstBufferPtr(), m_info.GetDataSize(), out, m_length );
            memset( prk, 0, sizeof( prk ) );
            if ( !expanded )
                return SetStatus( GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The length is larger than HKDF-SHA256 can give" );
            output.SetDataSize( output.GetDataSize() + m_length );
        }
        return SetOk();
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return 0;
    }

    private:

    CHmacSha256 m_extract;
    CDynamicBuffer m_info;
    UInt32 m_length;
};

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSha256Codec::CSha256Codec( void )
    : CInstanceBasedCodec( CoreCodecTypes::CryptographicHashCodec, TypeName, GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSha256Codec::CSha256Codec( const CSha256Codec& src )
    : CInstanceBasedCodec( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSha256Codec::~CSha256Codec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSha256Codec::GetParameterInfo( CCodecParameterInfoList& info ) const
{GUCEF_TRACE;

    info.push_back( CCodecParameterInfo( "output", GUCEF_DATATYPE_STRING, false, CVariant( "hex" ), "\"hex\" for the digest as lower case hexadecimal text, \"raw\" for the 32 digest bytes" ) );
}

/*-------------------------------------------------------------------------*/

TCodecStatus
CSha256Codec::CreateInstanceImpl( const CValueList& params       ,
                                  CICodecInstancePtr& instance   ,
                                  CString& errorMessage          )
{GUCEF_TRACE;

    bool isValid = true;
    bool hexOutput = IsHexOutputRequested( params, true, isValid );
    if ( !isValid )
    {
        errorMessage = "The parameter \"output\" must be \"hex\" or \"raw\"";
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    }
    instance = CICodecInstancePtr( GUCEF_NEW CSha256CodecInstance( hexOutput ) );
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CSha256Codec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CSha256Codec( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CSha256Codec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CSha256Codec";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CHmacSha256Codec::CHmacSha256Codec( void )
    : CInstanceBasedCodec( CoreCodecTypes::MacCodec, TypeName, GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHmacSha256Codec::CHmacSha256Codec( const CHmacSha256Codec& src )
    : CInstanceBasedCodec( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHmacSha256Codec::~CHmacSha256Codec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CHmacSha256Codec::GetParameterInfo( CCodecParameterInfoList& info ) const
{GUCEF_TRACE;

    info.push_back( CCodecParameterInfo( "key", GUCEF_DATATYPE_BINARY_BLOB, true, CVariant(), "The secret key, of any length" ) );
}

/*-------------------------------------------------------------------------*/

TCodecStatus
CHmacSha256Codec::CreateInstanceImpl( const CValueList& params       ,
                                      CICodecInstancePtr& instance   ,
                                      CString& errorMessage          )
{GUCEF_TRACE;

    const void* key = GUCEF_NULL;
    UInt32 keySize = 0;
    if ( !TryGetBinaryParam( params, "key", key, keySize ) )
    {
        errorMessage = "The parameter \"key\" is required";
        return GUCEF_CODEC_STATUS_MISSING_PARAMETER;
    }
    instance = CICodecInstancePtr( GUCEF_NEW CHmacSha256CodecInstance( key, keySize ) );
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CHmacSha256Codec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CHmacSha256Codec( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CHmacSha256Codec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CHmacSha256Codec";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CHkdfSha256Codec::CHkdfSha256Codec( void )
    : CInstanceBasedCodec( CoreCodecTypes::KeyDerivationCodec, TypeName, GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHkdfSha256Codec::CHkdfSha256Codec( const CHkdfSha256Codec& src )
    : CInstanceBasedCodec( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHkdfSha256Codec::~CHkdfSha256Codec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CHkdfSha256Codec::GetParameterInfo( CCodecParameterInfoList& info ) const
{GUCEF_TRACE;

    info.push_back( CCodecParameterInfo( "salt", GUCEF_DATATYPE_BINARY_BLOB, false, CVariant(), "A non secret random value, none is the same as 32 zero bytes" ) );
    info.push_back( CCodecParameterInfo( "info", GUCEF_DATATYPE_BINARY_BLOB, false, CVariant(), "What the key is for, binds the key to its use" ) );
    info.push_back( CCodecParameterInfo( "length", GUCEF_DATATYPE_UINT32, false, CVariant( (UInt32) 32 ), "The size of the derived key in bytes, at most 8160" ) );
}

/*-------------------------------------------------------------------------*/

TCodecStatus
CHkdfSha256Codec::CreateInstanceImpl( const CValueList& params       ,
                                      CICodecInstancePtr& instance   ,
                                      CString& errorMessage          )
{GUCEF_TRACE;

    const void* salt = GUCEF_NULL;
    UInt32 saltSize = 0;
    TryGetBinaryParam( params, "salt", salt, saltSize );
    const void* info = GUCEF_NULL;
    UInt32 infoSize = 0;
    TryGetBinaryParam( params, "info", info, infoSize );

    UInt32 length = CSha256::DigestSize;
    if ( params.HasKey( "length" ) && !TryGetUInt32Param( params, "length", length ) )
    {
        errorMessage = "The parameter \"length\" must be a number";
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    }
    if ( 0 == length || length > CHkdfSha256::MaxOutputSize )
    {
        errorMessage = "The parameter \"length\" must be 1 to 8160";
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    }

    instance = CICodecInstancePtr( GUCEF_NEW CHkdfSha256CodecInstance( salt, saltSize, info, infoSize, length ) );
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CHkdfSha256Codec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CHkdfSha256Codec( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CHkdfSha256Codec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CHkdfSha256Codec";
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
