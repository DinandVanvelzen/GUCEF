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


#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_COM_CIEMAILSASLMECHANISM_H
#include "gucefCOM_CIEMailSaslMechanism.h"
#define GUCEF_COM_CIEMAILSASLMECHANISM_H
#endif /* GUCEF_COM_CIEMAILSASLMECHANISM_H ? */

#include "gucefCOM_CEMailSaslScramSha256.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}
/*-------------------------------------------------------------------------*/

void
SetBuffer( CORE::CDynamicBuffer& buffer, const CORE::CString& text )
{
    buffer.Clear();
    if ( ByteLength( text ) > 0 )
        buffer.CopyFrom( ByteLength( text ), text.C_String() );
}
/*-------------------------------------------------------------------------*/

CORE::CString
GetText( const CORE::CDynamicBuffer& buffer )
{
    return buffer.GetDataSize() > 0 ? CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailSaslScramSha256::CEMailSaslScramSha256( const CORE::CString& userName, const CORE::CString& password )
    : CIEMailSaslMechanism()
    , m_userName( userName )
    , m_password( password )
    , m_clientNonce()
    , m_clientFirstBare()
    , m_step( 0 )
{GUCEF_TRACE;

    memset( m_serverSignature, 0, sizeof( m_serverSignature ) );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailSaslScramSha256::GetName( void ) const
{GUCEF_TRACE;

    static const CORE::CString name = "SCRAM-SHA-256";
    return name;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslScramSha256::HasInitialResponse( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslScramSha256::RevealsSecret( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CEMailSaslScramSha256::SetClientNonceForTesting( const CORE::CString& nonce )
{GUCEF_TRACE;

    m_clientNonce = nonce;
}

/*-------------------------------------------------------------------------*/

void
CEMailSaslScramSha256::Pbkdf2HmacSha256( const void* password, UInt32 passwordSize, const void* salt, UInt32 saltSize, UInt32 iterations, UInt8* output )
{GUCEF_TRACE;

    // one block of 32 bytes is all that is needed: T1 = U1 xor U2 xor ... Un
    CORE::CHmacSha256 hmac( password, passwordSize );
    GUCEF::vector< UInt8 > first( saltSize + 4 );
    if ( saltSize > 0 )
        memcpy( &first[ 0 ], salt, saltSize );
    first[ saltSize ] = 0;
    first[ saltSize + 1 ] = 0;
    first[ saltSize + 2 ] = 0;
    first[ saltSize + 3 ] = 1;

    UInt8 u[ 32 ];
    hmac.Reset();
    hmac.Update( &first[ 0 ], saltSize + 4 );
    hmac.Final( u );
    memcpy( output, u, 32 );
    for ( UInt32 i=1; i<iterations; ++i )
    {
        hmac.Reset();
        hmac.Update( u, 32 );
        hmac.Final( u );
        for ( int k=0; k<32; ++k )
            output[ k ] ^= u[ k ];
    }
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslScramSha256::Start( CORE::CDynamicBuffer& initialResponse )
{GUCEF_TRACE;

    if ( m_userName.IsNULLOrEmpty() )
    {
        SetFailure( "No user name" );
        return false;
    }
    if ( m_clientNonce.IsNULLOrEmpty() )
    {
        UInt8 random[ 18 ];
        if ( !CORE::CRandom::GetRandomBytes( random, sizeof( random ) ) )
        {
            SetFailure( "No random source for the nonce" );
            return false;
        }
        m_clientNonce = CORE::Base64Encode( random, sizeof( random ) );
    }

    // = and , in the name are escaped (RFC 5802 section 5.1)
    GUCEF::vector< char > escaped;
    const char* u = m_userName.C_String();
    for ( UInt32 i=0; i<ByteLength( m_userName ); ++i )
    {
        if ( '=' == u[ i ] ) escaped.insert( escaped.end(), "=3D", "=3D" + 3 );
        else if ( ',' == u[ i ] ) escaped.insert( escaped.end(), "=2C", "=2C" + 3 );
        else escaped.push_back( u[ i ] );
    }
    CORE::CString name( &escaped[ 0 ], (UInt32) escaped.size() );
    m_clientFirstBare = "n=" + name + ",r=" + m_clientNonce;
    SetBuffer( initialResponse, "n,," + m_clientFirstBare );
    m_step = 1;
    return true;
}

/*-------------------------------------------------------------------------*/

EMailSaslState
CEMailSaslScramSha256::Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response )
{GUCEF_TRACE;

    response.Clear();
    const CORE::CString text = GetText( challenge );

    if ( 1 == m_step )
    {
        // r=<nonce>,s=<salt>,i=<iterations>
        CORE::CString nonce, saltText;
        UInt32 iterations = 0;
        CORE::CString::StringVector fields = text.ParseElements( ',', false );
        for ( size_t i=0; i<fields.size(); ++i )
        {
            if ( fields[ i ].StartsWith( "r=" ) ) nonce = fields[ i ].SubstrFromRange( 2, fields[ i ].Length() );
            else if ( fields[ i ].StartsWith( "s=" ) ) saltText = fields[ i ].SubstrFromRange( 2, fields[ i ].Length() );
            else if ( fields[ i ].StartsWith( "i=" ) )
            {
                const char* digits = fields[ i ].C_String() + 2;
                UInt64 value = 0;
                for ( ; *digits >= '0' && *digits <= '9' && value < 100000000; ++digits )
                    value = value * 10 + (UInt64)( *digits - '0' );
                iterations = value > 100000000 ? 100000000 : (UInt32) value;
            }
            else if ( fields[ i ].StartsWith( "m=" ) )
            {
                SetFailure( "The server demands an extension that is not supported" );
                return MAILSASL_FAILED;
            }
        }
        if ( !nonce.StartsWith( m_clientNonce ) || ByteLength( nonce ) <= ByteLength( m_clientNonce ) )
        {
            SetFailure( "The nonce of the server does not extend ours" );
            return MAILSASL_FAILED;
        }
        if ( iterations < MinIterations || iterations > MaxIterations )
        {
            SetFailure( "The iteration count of the server is outside of what is accepted" );
            return MAILSASL_FAILED;
        }
        CORE::CDynamicBuffer salt;
        bool saltIsBase64 = !saltText.IsNULLOrEmpty();
        for ( UInt32 i=0; saltIsBase64 && i<ByteLength( saltText ); ++i )
        {
            const char c = saltText.C_String()[ i ];
            saltIsBase64 = ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '+' == c || '/' == c || '=' == c;
        }
        if ( !saltIsBase64 )
        {
            SetFailure( "The server sent no salt" );
            return MAILSASL_FAILED;
        }
        salt.SetBufferSize( ByteLength( saltText ), false );
        UInt32 saltSize = 0;
        if ( !CORE::Base64Decode( saltText, salt.GetBufferPtr(), salt.GetBufferSize(), saltSize ) || 0 == saltSize )
        {
            SetFailure( "The salt of the server is not base64" );
            return MAILSASL_FAILED;
        }

        UInt8 salted[ 32 ];
        Pbkdf2HmacSha256( m_password.C_String(), ByteLength( m_password ), salt.GetConstBufferPtr(), saltSize, iterations, salted );

        UInt8 clientKey[ 32 ], storedKey[ 32 ], serverKey[ 32 ];
        CORE::CHmacSha256::Mac( salted, 32, "Client Key", 10, clientKey );
        CORE::CSha256::Hash( clientKey, 32, storedKey );
        CORE::CHmacSha256::Mac( salted, 32, "Server Key", 10, serverKey );

        const CORE::CString finalWithoutProof = "c=biws,r=" + nonce;
        const CORE::CString authMessage = m_clientFirstBare + "," + text + "," + finalWithoutProof;
        UInt8 clientSignature[ 32 ], proof[ 32 ];
        CORE::CHmacSha256::Mac( storedKey, 32, authMessage.C_String(), ByteLength( authMessage ), clientSignature );
        for ( int k=0; k<32; ++k )
            proof[ k ] = clientKey[ k ] ^ clientSignature[ k ];
        CORE::CHmacSha256::Mac( serverKey, 32, authMessage.C_String(), ByteLength( authMessage ), m_serverSignature );

        SetBuffer( response, finalWithoutProof + ",p=" + CORE::Base64Encode( proof, 32 ) );
        m_step = 2;
        return MAILSASL_CONTINUE;
    }

    if ( 2 == m_step )
    {
        // v=<server signature>; e=<reason> is the failure of the server
        if ( text.StartsWith( "e=" ) )
        {
            SetFailure( "The server refused the credentials" );
            return MAILSASL_FAILED;
        }
        if ( !text.StartsWith( "v=" ) )
        {
            SetFailure( "The final message of the server is not valid" );
            return MAILSASL_FAILED;
        }
        CORE::CString signatureText = text.SubstrFromRange( 2, text.Length() );
        Int32 comma = signatureText.HasChar( ',' );
        if ( comma >= 0 )
            signatureText = signatureText.SubstrToIndex( (UInt32) comma, true );
        UInt8 signature[ 64 ];
        UInt32 signatureSize = 0;
        if ( !CORE::Base64Decode( signatureText, signature, sizeof( signature ), signatureSize ) || 32 != signatureSize ||
             !CORE::CSha256::ConstantTimeEquals( signature, m_serverSignature, 32 ) )
        {
            SetFailure( "The server could not prove that it knows the password" );
            return MAILSASL_FAILED;
        }
        m_step = 3;
        return MAILSASL_DONE;
    }

    SetFailure( "The server sent more than SCRAM has" );
    return MAILSASL_FAILED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
