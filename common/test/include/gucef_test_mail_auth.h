/*
 *  gucef common header: header-only server side of the SASL mechanisms for tests of mail clients
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

/*
 *  Test support: what the fake SMTP, POP3 and IMAP servers need to check the authentication of a client: base64 helpers, the
 *  decoding of the PLAIN and XOAUTH2 messages, and the server side of SCRAM-SHA-256 (RFC 5802, RFC 7677), written on the
 *  hash functions of gucefCORE and independent of the client code that is being tested.
 */

#ifndef GUCEF_TEST_MAIL_AUTH_H
#define GUCEF_TEST_MAIL_AUTH_H

#ifdef __cplusplus

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

/** The bytes of base64 text as a string (the text may have a NUL in it: the string is cut at it, use DecodeBase64Bytes for those) */
inline CORE::CString
DecodeBase64Text( const CORE::CString& base64 )
{
    GUCEF::vector< char > bytes( base64.ByteSize() + 4 );
    CORE::UInt32 used = 0;
    if ( !CORE::Base64Decode( base64, &bytes[ 0 ], (CORE::UInt32) bytes.size(), used ) || 0 == used )
        return CORE::CString();
    return CORE::CString( &bytes[ 0 ], used );
}

/*-------------------------------------------------------------------------*/

inline bool
DecodeBase64Bytes( const CORE::CString& base64, GUCEF::vector< char >& bytes )
{
    bytes.assign( base64.ByteSize() + 4, '\0' );
    CORE::UInt32 used = 0;
    if ( !CORE::Base64Decode( base64, &bytes[ 0 ], (CORE::UInt32) bytes.size(), used ) )
        return false;
    bytes.resize( used );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Splits a PLAIN message, authzid NUL authcid NUL password. @return false when it is not in that form */
inline bool
SplitPlain( const CORE::CString& base64, CORE::CString& authzid, CORE::CString& user, CORE::CString& password )
{
    GUCEF::vector< char > bytes;
    if ( !DecodeBase64Bytes( base64, bytes ) )
        return false;
    size_t first = bytes.size(), second = bytes.size();
    for ( size_t i=0; i<bytes.size(); ++i )
    {
        if ( '\0' != bytes[ i ] )
            continue;
        if ( first == bytes.size() ) first = i;
        else { second = i; break; }
    }
    if ( first == bytes.size() || second == bytes.size() )
        return false;
    authzid = first > 0 ? CORE::CString( &bytes[ 0 ], (CORE::UInt32) first ) : CORE::CString();
    user = second > first + 1 ? CORE::CString( &bytes[ first + 1 ], (CORE::UInt32)( second - first - 1 ) ) : CORE::CString();
    password = bytes.size() > second + 1 ? CORE::CString( &bytes[ second + 1 ], (CORE::UInt32)( bytes.size() - second - 1 ) ) : CORE::CString();
    return true;
}

/*-------------------------------------------------------------------------*/

/** Splits an XOAUTH2 message, user=NAME ^A auth=Bearer TOKEN ^A ^A */
inline bool
SplitXOAuth2( const CORE::CString& base64, CORE::CString& user, CORE::CString& token )
{
    const CORE::CString text = DecodeBase64Text( base64 );
    if ( !text.StartsWith( "user=" ) )
        return false;
    const Int32 first = text.HasChar( '\x01' );
    if ( first < 0 )
        return false;
    user = text.SubstrFromRange( 5, (CORE::UInt32) first );
    const CORE::CString rest = text.SubstrFromRange( (CORE::UInt32) first + 1, text.Length() );
    if ( !rest.StartsWith( "auth=Bearer " ) )
        return false;
    const Int32 second = rest.HasChar( '\x01' );
    if ( second < 0 )
        return false;
    token = rest.SubstrFromRange( 12, (CORE::UInt32) second );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The server side of SCRAM-SHA-256 for one user */
class CTestScramServer
{
    public:

    CTestScramServer( const CORE::CString& user, const CORE::CString& password, CORE::UInt32 iterations = 4096 )
        : m_user( user )
        , m_password( password )
        , m_iterations( iterations )
        , m_salt( "gucef-test-salt-0123" )
        , m_serverNonceTail( "srvNONCE+tail" )
        , m_clientFirstBare()
        , m_serverFirst()
        , m_clientNonce()
    {
        memset( m_serverSignature, 0, sizeof( m_serverSignature ) );
        memset( m_storedKey, 0, sizeof( m_storedKey ) );
    }

    /** The first message of the client (decoded), answered with the first message of the server. False when the client message is not valid or names another user */
    bool ProcessFirst( const CORE::CString& clientFirst, CORE::CString& serverFirst )
    {
        if ( !clientFirst.StartsWith( "n,," ) )
            return false;
        m_clientFirstBare = clientFirst.SubstrFromRange( 3, clientFirst.Length() );
        CORE::CString::StringVector fields = m_clientFirstBare.ParseElements( ',', false );
        CORE::CString name;
        for ( size_t i=0; i<fields.size(); ++i )
        {
            if ( fields[ i ].StartsWith( "n=" ) ) name = fields[ i ].SubstrFromRange( 2, fields[ i ].Length() );
            if ( fields[ i ].StartsWith( "r=" ) ) m_clientNonce = fields[ i ].SubstrFromRange( 2, fields[ i ].Length() );
        }
        if ( name != m_user || m_clientNonce.IsNULLOrEmpty() )
            return false;

        UInt8 salted[ 32 ];
        Pbkdf2( salted );
        UInt8 clientKey[ 32 ], serverKey[ 32 ];
        CORE::CHmacSha256::Mac( salted, 32, "Client Key", 10, clientKey );
        CORE::CSha256::Hash( clientKey, 32, m_storedKey );
        CORE::CHmacSha256::Mac( salted, 32, "Server Key", 10, serverKey );
        memcpy( m_serverKey, serverKey, 32 );

        m_serverFirst = "r=" + m_clientNonce + m_serverNonceTail + ",s=" + CORE::Base64Encode( m_salt.C_String(), m_salt.ByteSize() - 1 ) + ",i=" + CORE::UInt32ToString( m_iterations );
        serverFirst = m_serverFirst;
        return true;
    }

    /** The last message of the client; answered with the signature of the server when the proof is right */
    bool ProcessFinal( const CORE::CString& clientFinal, CORE::CString& serverFinal )
    {
        const Int32 proofAt = clientFinal.HasSubstr( ",p=" );
        if ( proofAt < 0 )
            return false;
        const CORE::CString withoutProof = clientFinal.SubstrToIndex( (CORE::UInt32) proofAt, true );
        const CORE::CString proofText = clientFinal.SubstrFromRange( (CORE::UInt32) proofAt + 3, clientFinal.Length() );
        if ( !withoutProof.EndsWith( m_clientNonce + m_serverNonceTail ) )
            return false;

        const CORE::CString authMessage = m_clientFirstBare + "," + m_serverFirst + "," + withoutProof;
        UInt8 clientSignature[ 32 ];
        CORE::CHmacSha256::Mac( m_storedKey, 32, authMessage.C_String(), authMessage.ByteSize() - 1, clientSignature );

        UInt8 proof[ 64 ];
        CORE::UInt32 proofSize = 0;
        if ( !CORE::Base64Decode( proofText, proof, sizeof( proof ), proofSize ) || 32 != proofSize )
            return false;
        UInt8 clientKey[ 32 ], storedKey[ 32 ];
        for ( int i=0; i<32; ++i )
            clientKey[ i ] = proof[ i ] ^ clientSignature[ i ];
        CORE::CSha256::Hash( clientKey, 32, storedKey );
        if ( !CORE::CSha256::ConstantTimeEquals( storedKey, m_storedKey, 32 ) )
            return false;

        UInt8 serverSignature[ 32 ];
        CORE::CHmacSha256::Mac( m_serverKey, 32, authMessage.C_String(), authMessage.ByteSize() - 1, serverSignature );
        serverFinal = "v=" + CORE::Base64Encode( serverSignature, 32 );
        return true;
    }

    private:

    void Pbkdf2( UInt8* output ) const
    {
        CORE::CHmacSha256 hmac( m_password.C_String(), m_password.ByteSize() - 1 );
        GUCEF::vector< UInt8 > first( m_salt.ByteSize() - 1 + 4 );
        memcpy( &first[ 0 ], m_salt.C_String(), m_salt.ByteSize() - 1 );
        first[ first.size() - 4 ] = 0; first[ first.size() - 3 ] = 0; first[ first.size() - 2 ] = 0; first[ first.size() - 1 ] = 1;
        UInt8 u[ 32 ];
        hmac.Reset();
        hmac.Update( &first[ 0 ], (CORE::UInt32) first.size() );
        hmac.Final( u );
        memcpy( output, u, 32 );
        for ( CORE::UInt32 i=1; i<m_iterations; ++i )
        {
            hmac.Reset();
            hmac.Update( u, 32 );
            hmac.Final( u );
            for ( int k=0; k<32; ++k )
                output[ k ] ^= u[ k ];
        }
    }

    CORE::CString m_user;
    CORE::CString m_password;
    CORE::UInt32 m_iterations;
    CORE::CString m_salt;
    CORE::CString m_serverNonceTail;
    CORE::CString m_clientFirstBare;
    CORE::CString m_serverFirst;
    CORE::CString m_clientNonce;
    UInt8 m_storedKey[ 32 ];
    UInt8 m_serverKey[ 32 ];
    UInt8 m_serverSignature[ 32 ];
};

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_MAIL_AUTH_H ? */
