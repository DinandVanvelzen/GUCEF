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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COREECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREECODECTYPES_H
#endif /* GUCEF_CORE_COREECODECTYPES_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#include "gucefCOM_CMcpRequestStateSealer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMcpRequestStateSealer::DefaultAeadCodecTypes = "xchacha20-poly1305,chacha20-poly1305,aes-256-gcm";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt8 VersionHmac = 0x01;
const UInt8 VersionAead = 0x02;
const UInt8 FlagNonce = 0x01;
const UInt32 BindingSize = 32;
const UInt32 MacSize = 32;
const UInt32 TagSize = 16;
const char* const HmacKeyInfo = "GUCEF MCP requestState HMAC-SHA256";
const char* const AeadKeyInfo = "GUCEF MCP requestState AEAD";

void
PutUInt64BigEndian( UInt8* out, UInt64 value )
{GUCEF_TRACE;

    for ( int i=7; i>=0; --i )
    {
        out[ i ] = (UInt8)( value & 0xFF );
        value >>= 8;
    }
}

UInt64
GetUInt64BigEndian( const UInt8* in )
{GUCEF_TRACE;

    UInt64 value = 0;
    for ( int i=0; i<8; ++i )
        value = ( value << 8 ) | in[ i ];
    return value;
}

CORE::CString
ToHex( const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    static const char* digits = "0123456789abcdef";
    GUCEF::vector< char > text( size * 2, '0' );
    for ( UInt32 i=0; i<size; ++i )
    {
        text[ i*2 ] = digits[ data[ i ] >> 4 ];
        text[ i*2+1 ] = digits[ data[ i ] & 0x0F ];
    }
    return 0 == size ? CORE::CString() : CORE::CString( &text[ 0 ], size * 2 );
}

bool
FromHex( const CORE::CString& text, CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    const char* data = text.C_String();
    UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    if ( 0 != size % 2 )
        return false;
    bytes.SetDataSize( 0 );
    for ( UInt32 i=0; i<size; i+=2 )
    {
        UInt8 value = 0;
        for ( UInt32 n=0; n<2; ++n )
        {
            char c = data[ i+n ];
            value = (UInt8)( value << 4 );
            if ( c >= '0' && c <= '9' ) value |= (UInt8)( c - '0' );
            else if ( c >= 'a' && c <= 'f' ) value |= (UInt8)( c - 'a' + 10 );
            else if ( c >= 'A' && c <= 'F' ) value |= (UInt8)( c - 'A' + 10 );
            else return false;
        }
        bytes.Append( &value, 1 );
    }
    return true;
}

bool
AeadAlgorithmOf( const CORE::CString& type, UInt8& algorithmId, UInt32& nonceSize )
{GUCEF_TRACE;

    if ( type.Equals( "xchacha20-poly1305", false ) ) { algorithmId = 1; nonceSize = 24; return true; }
    if ( type.Equals( "chacha20-poly1305", false ) ) { algorithmId = 2; nonceSize = 12; return true; }
    if ( type.Equals( "aes-256-gcm", false ) ) { algorithmId = 3; nonceSize = 12; return true; }
    return false;
}

UInt32
TextSize( const CORE::CString& text )
{GUCEF_TRACE;

    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpRequestStateSealer::SBinding::SBinding( void )
    : subject()
    , method()
    , target()
{GUCEF_TRACE;

    ::memset( argumentsDigest, 0, sizeof( argumentsDigest ) );
}

/*-------------------------------------------------------------------------*/

CMcpRequestStateSealer::CMcpRequestStateSealer( void )
    : m_initialized( false )
    , m_protection( PROTECTION_HMAC_SHA256 )
    , m_aeadCodecType()
    , m_aeadAlgorithmId( 0 )
    , m_aeadNonceSize( 0 )
    , m_lock()
    , m_mac()
    , m_aead()
    , m_work()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpRequestStateSealer::~CMcpRequestStateSealer()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_mac.Unlink();
    m_aead.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestStateSealer::ParseProtection( const CORE::CString& text, EProtection& protection )
{GUCEF_TRACE;

    if ( text.Equals( "auto", false ) ) { protection = PROTECTION_AUTO; return true; }
    if ( text.Equals( "hmac-sha256", false ) ) { protection = PROTECTION_HMAC_SHA256; return true; }
    if ( text.Equals( "aead", false ) ) { protection = PROTECTION_AEAD; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

const char*
CMcpRequestStateSealer::ProtectionToString( EProtection protection )
{GUCEF_TRACE;

    switch ( protection )
    {
        case PROTECTION_AUTO: return "auto";
        case PROTECTION_HMAC_SHA256: return "hmac-sha256";
        case PROTECTION_AEAD: return "aead";
        default: return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestStateSealer::Initialize( EProtection protection                          ,
                                    const CORE::CString::StringVector& aeadCodecTypes  ,
                                    const CORE::CDynamicBuffer* key                 ,
                                    CORE::CString& error                            )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_initialized = false;
    m_mac.Unlink();
    m_aead.Unlink();
    m_aeadCodecType.Clear();

    UInt8 masterKey[ KeySize ];
    if ( GUCEF_NULL != key )
    {
        if ( key->GetDataSize() != KeySize )
        {
            error = "the requestState key must be 32 bytes";
            return false;
        }
        ::memcpy( masterKey, key->GetConstBufferPtr(), KeySize );
    }
    else
    if ( !CORE::CRandom::GetRandomBytes( masterKey, KeySize ) )
    {
        error = "the secure random source failed to give a requestState key";
        return false;
    }

    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

    if ( PROTECTION_HMAC_SHA256 != protection )
    {
        UInt8 aeadKey[ KeySize ];
        CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, masterKey, KeySize, AeadKeyInfo, (UInt32) ::strlen( AeadKeyInfo ), aeadKey, KeySize );
        for ( size_t i=0; i<aeadCodecTypes.size() && m_aead.IsNULL(); ++i )
        {
            UInt8 algorithmId = 0;
            UInt32 nonceSize = 0;
            if ( !AeadAlgorithmOf( aeadCodecTypes[ i ], algorithmId, nonceSize ) )
                continue;
            CORE::CCodecRegistry::TICodecPtr codec = registry.TryGetCodec( CORE::CoreCodecTypes::EncryptionCodec, aeadCodecTypes[ i ] );
            if ( codec.IsNULL() )
                continue;
            CORE::CValueList params;
            params.Set( "key", CORE::CVariant( aeadKey, KeySize ) );
            CORE::CICodecInstancePtr instance;
            if ( !codec->CreateInstance( params, instance ) || instance.IsNULL() )
                continue;

            // a provider that is registered but fails a round trip is skipped
            UInt8 nonce[ 24 ];
            ::memset( nonce, 0x5A, sizeof( nonce ) );
            const char probe[] = "probe";
            CORE::SCodecCallArgs args;
            ::memset( &args, 0, sizeof( args ) );
            args.structSize = sizeof( args );
            args.nonce = nonce;
            args.nonceSize = nonceSize;
            args.aad = probe;
            args.aadSize = 5;
            CORE::CDynamicBuffer sealed;
            CORE::CDynamicBuffer opened;
            if ( !instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, probe, 5, &args, sealed ) ||
                 !instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, sealed.GetConstBufferPtr(), sealed.GetDataSize(), &args, opened ) ||
                 opened.GetDataSize() != 5 || 0 != ::memcmp( opened.GetConstBufferPtr(), probe, 5 ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpRequestStateSealer: The AEAD codec \"" << aeadCodecTypes[ i ] << "\" failed a round trip and is not used" );
                continue;
            }
            m_aead = instance;
            m_aeadCodecType = aeadCodecTypes[ i ];
            m_aeadAlgorithmId = algorithmId;
            m_aeadNonceSize = nonceSize;
        }
        ::memset( aeadKey, 0, sizeof( aeadKey ) );
        if ( m_aead.IsNULL() && PROTECTION_AEAD == protection )
        {
            ::memset( masterKey, 0, sizeof( masterKey ) );
            error = "requestState protection \"aead\" was asked for, but no AEAD codec is registered";
            return false;
        }
    }

    if ( m_aead.IsNULL() )
    {
        UInt8 macKey[ KeySize ];
        CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, masterKey, KeySize, HmacKeyInfo, (UInt32) ::strlen( HmacKeyInfo ), macKey, KeySize );
        CORE::CCodecRegistry::TICodecPtr codec = registry.TryGetCodec( CORE::CoreCodecTypes::MacCodec, "hmac-sha256" );
        CORE::CValueList params;
        params.Set( "key", CORE::CVariant( macKey, KeySize ) );
        ::memset( macKey, 0, sizeof( macKey ) );
        if ( codec.IsNULL() || !codec->CreateInstance( params, m_mac ) || m_mac.IsNULL() )
        {
            ::memset( masterKey, 0, sizeof( masterKey ) );
            m_mac.Unlink();
            error = "the hmac-sha256 codec is not available";
            return false;
        }
        m_protection = PROTECTION_HMAC_SHA256;
    }
    else
    {
        m_protection = PROTECTION_AEAD;
    }
    ::memset( masterKey, 0, sizeof( masterKey ) );
    m_initialized = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestStateSealer::IsInitialized( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_initialized;
}

/*-------------------------------------------------------------------------*/

CMcpRequestStateSealer::EProtection
CMcpRequestStateSealer::GetProtection( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_protection;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestStateSealer::GetAeadCodecType( void ) const
{GUCEF_TRACE;

    return m_aeadCodecType;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestStateSealer::ComputeBinding( const SBinding& binding, UInt8* bindingDigest )
{GUCEF_TRACE;

    static const UInt8 separator = 0x00;
    CORE::CSha256 sha;
    sha.Update( binding.subject.C_String(), TextSize( binding.subject ) );
    sha.Update( &separator, 1 );
    sha.Update( binding.method.C_String(), TextSize( binding.method ) );
    sha.Update( &separator, 1 );
    sha.Update( binding.target.C_String(), TextSize( binding.target ) );
    sha.Update( &separator, 1 );
    sha.Update( binding.argumentsDigest, sizeof( binding.argumentsDigest ) );
    sha.Update( &separator, 1 );
    sha.Final( bindingDigest );
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestStateSealer::DigestArguments( const CORE::CDataNode* arguments, UInt8* digest )
{GUCEF_TRACE;

    CORE::CString jsonText( "{}" );
    if ( GUCEF_NULL != arguments )
        CJsonRpcEndpoint::ToJsonText( *arguments, jsonText );
    CORE::CSha256::Hash( jsonText.C_String(), TextSize( jsonText ), digest );
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestStateSealer::Seal( const SBinding& binding           ,
                              const CORE::CDataNode& state      ,
                              bool withNonce                    ,
                              UInt64 expiryUnixMs               ,
                              CORE::CString& sealed             ,
                              CORE::CString& error              ) const
{GUCEF_TRACE;

    sealed.Clear();
    if ( GUCEF_DATATYPE_OBJECT != state.GetNodeType() && GUCEF_DATATYPE_ARRAY != state.GetNodeType() )
    {
        error = "the handler state must be a JSON object or array";
        return false;
    }
    CORE::CString jsonText;
    CJsonRpcEndpoint::ToJsonText( state, jsonText );
    const UInt32 jsonSize = TextSize( jsonText );
    if ( jsonSize > MaxSealedBytes )
    {
        error = "the handler state is larger than 16 KiB";
        return false;
    }

    UInt8 bindingDigest[ BindingSize ];
    ComputeBinding( binding, bindingDigest );
    UInt8 header[ 8 ];
    PutUInt64BigEndian( header, expiryUnixMs );
    UInt8 nonce[ NonceSize ];
    if ( withNonce && !CORE::CRandom::GetRandomBytes( nonce, NonceSize ) )
    {
        error = "the secure random source failed";
        return false;
    }
    const UInt8 flags = withNonce ? FlagNonce : 0;

    MT::CScopeMutex lock( m_lock );
    if ( !m_initialized )
    {
        error = "the requestState sealer is not initialized";
        return false;
    }

    if ( PROTECTION_HMAC_SHA256 == m_protection )
    {
        CORE::CDynamicBuffer payload;
        payload.Append( &VersionHmac, 1 );
        payload.Append( &flags, 1 );
        payload.Append( header, 8 );
        payload.Append( bindingDigest, BindingSize );
        if ( withNonce )
            payload.Append( nonce, NonceSize );
        payload.Append( jsonText.C_String(), jsonSize );

        m_work.SetDataSize( 0 );
        if ( !m_mac->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, payload.GetConstBufferPtr(), payload.GetDataSize(), GUCEF_NULL, m_work ) || m_work.GetDataSize() != MacSize )
        {
            error = "computing the MAC of the requestState failed: " + m_mac->GetLastErrorMessage();
            return false;
        }
        CORE::CString payloadText;
        CORE::CString macText;
        if ( !CORE::Base64UrlEncode( payload.GetConstBufferPtr(), payload.GetDataSize(), payloadText ) ||
             !CORE::Base64UrlEncode( m_work.GetConstBufferPtr(), MacSize, macText ) )
        {
            error = "encoding the requestState failed";
            return false;
        }
        sealed = payloadText + "." + macText;
    }
    else
    {
        UInt8 aad[ 2 + BindingSize ];
        aad[ 0 ] = VersionAead;
        aad[ 1 ] = m_aeadAlgorithmId;
        ::memcpy( aad + 2, bindingDigest, BindingSize );

        CORE::CDynamicBuffer plaintext;
        plaintext.Append( &flags, 1 );
        plaintext.Append( header, 8 );
        if ( withNonce )
            plaintext.Append( nonce, NonceSize );
        plaintext.Append( jsonText.C_String(), jsonSize );

        UInt8 aeadNonce[ 24 ];
        if ( !CORE::CRandom::GetRandomBytes( aeadNonce, m_aeadNonceSize ) )
        {
            error = "the secure random source failed";
            return false;
        }
        CORE::SCodecCallArgs args;
        ::memset( &args, 0, sizeof( args ) );
        args.structSize = sizeof( args );
        args.nonce = aeadNonce;
        args.nonceSize = m_aeadNonceSize;
        args.aad = aad;
        args.aadSize = sizeof( aad );

        CORE::CDynamicBuffer token;
        token.Append( aad, 2 );
        token.Append( aeadNonce, m_aeadNonceSize );
        if ( !m_aead->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, plaintext.GetConstBufferPtr(), plaintext.GetDataSize(), &args, token ) )
        {
            error = "encrypting the requestState failed: " + m_aead->GetLastErrorMessage();
            return false;
        }
        if ( !CORE::Base64UrlEncode( token.GetConstBufferPtr(), token.GetDataSize(), sealed ) )
        {
            error = "encoding the requestState failed";
            return false;
        }
    }

    if ( TextSize( sealed ) > MaxSealedBytes )
    {
        sealed.Clear();
        error = "the sealed requestState is larger than 16 KiB";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CMcpRequestStateSealer::EUnsealResult
CMcpRequestStateSealer::Unseal( const CORE::CString& sealed     ,
                                const SBinding& binding         ,
                                UInt64 nowUnixMs                ,
                                CORE::CDataNode& state          ,
                                CORE::CString& nonceHex         ) const
{GUCEF_TRACE;

    state.Clear();
    nonceHex.Clear();
    const UInt32 sealedSize = TextSize( sealed );
    if ( 0 == sealedSize || sealedSize > MaxSealedBytes )
        return UNSEAL_MALFORMED;

    UInt8 bindingDigest[ BindingSize ];
    ComputeBinding( binding, bindingDigest );

    // the fields of the plain part: flags, expiry, nonce, JSON
    CORE::CDynamicBuffer plain;
    bool bindingMatches = true;

    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_initialized )
            return UNSEAL_NOT_READY;

        if ( PROTECTION_HMAC_SHA256 == m_protection )
        {
            const char* text = sealed.C_String();
            UInt32 dot = 0;
            while ( dot < sealedSize && '.' != text[ dot ] )
                ++dot;
            if ( 0 == dot || dot+1 >= sealedSize )
                return UNSEAL_MALFORMED;
            CORE::CDynamicBuffer payload;
            CORE::CDynamicBuffer mac;
            if ( !CORE::Base64UrlDecode( CORE::CString( text, dot ), payload ) ||
                 !CORE::Base64UrlDecode( CORE::CString( text + dot + 1, sealedSize - dot - 1 ), mac ) ||
                 mac.GetDataSize() != MacSize || payload.GetDataSize() < 2 + 8 + BindingSize )
                return UNSEAL_MALFORMED;

            m_work.SetDataSize( 0 );
            if ( !m_mac->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, payload.GetConstBufferPtr(), payload.GetDataSize(), GUCEF_NULL, m_work ) || m_work.GetDataSize() != MacSize )
                return UNSEAL_MALFORMED;
            if ( !CORE::CSha256::ConstantTimeEquals( m_work.GetConstBufferPtr(), mac.GetConstBufferPtr(), MacSize ) )
                return UNSEAL_TAMPERED;

            const UInt8* bytes = static_cast< const UInt8* >( payload.GetConstBufferPtr() );
            if ( VersionHmac != bytes[ 0 ] )
                return UNSEAL_MALFORMED;
            bindingMatches = CORE::CSha256::ConstantTimeEquals( bytes + 10, bindingDigest, BindingSize );
            plain.Append( bytes + 1, 9 );
            plain.Append( bytes + 10 + BindingSize, payload.GetDataSize() - 10 - BindingSize );
        }
        else
        {
            CORE::CDynamicBuffer token;
            if ( !CORE::Base64UrlDecode( sealed, token ) || token.GetDataSize() < 2 + m_aeadNonceSize + TagSize + 9 )
                return UNSEAL_MALFORMED;
            const UInt8* bytes = static_cast< const UInt8* >( token.GetConstBufferPtr() );
            if ( VersionAead != bytes[ 0 ] || m_aeadAlgorithmId != bytes[ 1 ] )
                return UNSEAL_MALFORMED;

            UInt8 aad[ 2 + BindingSize ];
            aad[ 0 ] = VersionAead;
            aad[ 1 ] = m_aeadAlgorithmId;
            ::memcpy( aad + 2, bindingDigest, BindingSize );
            CORE::SCodecCallArgs args;
            ::memset( &args, 0, sizeof( args ) );
            args.structSize = sizeof( args );
            args.nonce = bytes + 2;
            args.nonceSize = m_aeadNonceSize;
            args.aad = aad;
            args.aadSize = sizeof( aad );
            const UInt32 offset = 2 + m_aeadNonceSize;
            if ( !m_aead->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, bytes + offset, token.GetDataSize() - offset, &args, plain ) )
                return UNSEAL_TAMPERED;     // also another binding: it is in the associated data
            if ( plain.GetDataSize() < 9 )
                return UNSEAL_MALFORMED;
        }
    }

    const UInt8* fields = static_cast< const UInt8* >( plain.GetConstBufferPtr() );
    const UInt8 flags = fields[ 0 ];
    const UInt64 expiry = GetUInt64BigEndian( fields + 1 );
    UInt32 offset = 9;
    if ( 0 != ( flags & ~FlagNonce ) )
        return UNSEAL_MALFORMED;
    if ( 0 != ( flags & FlagNonce ) )
    {
        if ( plain.GetDataSize() < offset + NonceSize )
            return UNSEAL_MALFORMED;
        nonceHex = ToHex( fields + offset, NonceSize );
        offset += NonceSize;
    }
    if ( nowUnixMs >= expiry )
    {
        nonceHex.Clear();
        return UNSEAL_EXPIRED;
    }
    if ( !bindingMatches )
    {
        nonceHex.Clear();
        return UNSEAL_WRONG_BINDING;
    }

    const UInt32 jsonSize = plain.GetDataSize() - offset;
    CORE::CString jsonText( reinterpret_cast< const char* >( fields + offset ), jsonSize );
    if ( 0 == jsonSize || !CJsonRpcEndpoint::ParseJsonText( jsonText, state ) )
    {
        nonceHex.Clear();
        state.Clear();
        return UNSEAL_MALFORMED;
    }
    return UNSEAL_OK;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestStateSealer::LoadKeyFile( const CORE::CString& path, CORE::CDynamicBuffer& key, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    if ( !content.LoadContentFromFile( path ) )
    {
        error = "the requestState key file could not be read";
        return false;
    }
    if ( content.GetDataSize() == KeySize )
    {
        key = content;
        return true;
    }
    // text, without the white space and line ends an editor leaves around it
    const char* data = static_cast< const char* >( content.GetConstBufferPtr() );
    UInt32 begin = 0;
    UInt32 end = content.GetDataSize();
    while ( begin < end && ( ' ' == data[ begin ] || '\t' == data[ begin ] || '\r' == data[ begin ] || '\n' == data[ begin ] ) )
        ++begin;
    while ( end > begin && ( ' ' == data[ end-1 ] || '\t' == data[ end-1 ] || '\r' == data[ end-1 ] || '\n' == data[ end-1 ] ) )
        --end;
    CORE::CString text( data + begin, end - begin );
    if ( ( CORE::Base64UrlDecode( text, key ) && key.GetDataSize() == KeySize ) ||
         ( FromHex( text, key ) && key.GetDataSize() == KeySize ) )
        return true;
    key.SetDataSize( 0 );
    error = "the requestState key file must hold 32 bytes, raw, as base64url or as hexadecimal text";
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
