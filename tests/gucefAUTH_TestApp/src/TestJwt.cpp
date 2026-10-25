/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestText.h"
#include "TestRsaKeys.h"
#include "TestJwt.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The moment the tokens of the tests are checked at */
static const UInt64 TestNow = 1800000000;

/*-------------------------------------------------------------------------*/

static bool
Prerequisites( void )
{GUCEF_TRACE;

    return JsonCodecAvailable() && AUTH::CAuthCrypto::IsRsaAvailable();
}

/*-------------------------------------------------------------------------*/

static bool
LoadJwk( const CORE::CString& jwkJson, AUTH::CJwk& jwk )
{GUCEF_TRACE;

    CORE::CDataNode root;
    return AUTH::CAuthJson::Parse( jwkJson, 65536, 6, true, root ) && jwk.Load( AUTH::CAuthJson::GetDocument( root ) );
}

/*-------------------------------------------------------------------------*/

static AUTH::CJwk
KeyOf( const CTestRsaKey& key, const CORE::CString& keyId, const CORE::CString& algorithm = "RS256" )
{GUCEF_TRACE;

    AUTH::CJwk jwk;
    LoadJwk( key.ToJwk( keyId, algorithm ), jwk );
    return jwk;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Header( const CORE::CString& algorithm = "RS256", const CORE::CString& keyId = "key-a" )
{GUCEF_TRACE;

    return "{\"alg\":\"" + algorithm + "\",\"kid\":\"" + keyId + "\",\"typ\":\"JWT\"}";
}

/*-------------------------------------------------------------------------*/

/** A payload of a good token at TestNow; members can be replaced or left out by the caller building its own */
static CORE::CString
Payload( const CORE::CString& extra = CORE::CString::Empty )
{GUCEF_TRACE;

    CORE::CString payload = "{\"iss\":\"https://issuer.example\",\"sub\":\"s-1\",\"aud\":\"client-1\",\"exp\":" + CORE::UInt64ToString( TestNow + 600 ) +
                            ",\"iat\":" + CORE::UInt64ToString( TestNow - 10 );
    if ( !extra.IsNULLOrEmpty() )
        payload += "," + extra;
    return payload + "}";
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Sign( const CORE::CString& header, const CORE::CString& payload )
{GUCEF_TRACE;

    return TestKeyA().MakeToken( header, payload );
}

/*-------------------------------------------------------------------------*/

static AUTH::CJwtValidationOptions
Options( void )
{GUCEF_TRACE;

    AUTH::CJwtValidationOptions options;
    options.issuer = "https://issuer.example";
    options.audience = "client-1";
    options.nowEpochSeconds = TestNow;
    return options;
}

/*-------------------------------------------------------------------------*/

/** The outcome of parsing and verifying, then validating, a token */
static AUTH::TJwtStatus
Check( const CORE::CString& token, const AUTH::CJwtValidationOptions& options, const AUTH::CJwk& key )
{GUCEF_TRACE;

    AUTH::CJwt jwt;
    AUTH::TJwtStatus status = jwt.Parse( token );
    if ( AUTH::JWT_OK != status )
        return status;
    status = jwt.VerifySignature( key );
    if ( AUTH::JWT_OK != status )
        return status;
    return jwt.ValidateClaims( options );
}

/*-------------------------------------------------------------------------*/

static void
PerformJsonTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: JSON from the network is limited in size and depth, a NUL byte is refused" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            CORE::CDataNode node;
            ASSERT_TRUE( AUTH::CAuthJson::IsAvailable() );
            ASSERT_TRUE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":\"b\",\"n\":12,\"t\":true,\"l\":[\"x\",\"y\"]}" ), 1000, 4, true, node ) );
            CORE::CString text;
            Int64 number = 0;
            bool flag = false;
            CORE::CString::StringVector list;
            ASSERT_TRUE( AUTH::CAuthJson::GetString( node, "a", text ) && "b" == text );
            ASSERT_TRUE( AUTH::CAuthJson::GetInt64( node, "n", number ) && 12 == number );
            ASSERT_TRUE( AUTH::CAuthJson::GetBool( node, "t", flag ) && flag );
            ASSERT_TRUE( AUTH::CAuthJson::GetStringList( node, "l", list ) && 2 == list.size() && "x" == list[ 0 ] && "y" == list[ 1 ] );
            // a number is not a string and a string is not a number
            ASSERT_FALSE( AUTH::CAuthJson::GetString( node, "n", text ) );
            ASSERT_FALSE( AUTH::CAuthJson::GetInt64( node, "a", number ) );
            ASSERT_TRUE( AUTH::CAuthJson::GetStringList( node, "a", list ) && 1 == list.size() );

            // size
            ASSERT_FALSE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":\"b\"}" ), 8, 4, true, node ) );
            ASSERT_FALSE( AUTH::CAuthJson::Parse( CORE::CString( "" ), 8, 4, true, node ) );

            // depth: arrays in arrays
            ASSERT_TRUE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":[[[1]]]}" ), 1000, 4, false, node ) );
            ASSERT_FALSE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":[[[[1]]]]}" ), 1000, 4, false, node ) );
            CORE::CString deep = RepeatedText( "[", 5000 ) + RepeatedText( "]", 5000 );
            ASSERT_FALSE( AUTH::CAuthJson::Parse( deep, 100000, 12, false, node ) );
            CORE::CString deepObjects = RepeatedText( "{\"a\":", 3000 ) + "1" + RepeatedText( "}", 3000 );
            ASSERT_FALSE( AUTH::CAuthJson::Parse( deepObjects, 100000, 12, false, node ) );

            // a NUL byte, an unbalanced document, text that is not JSON
            const char withNul[] = { '{', '"', 'a', '"', ':', '"', 'b', '\0', '"', '}' };
            ASSERT_FALSE( AUTH::CAuthJson::Parse( withNul, sizeof( withNul ), 1000, 4, false, node ) );
            ASSERT_FALSE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":\"b\"" ), 1000, 4, false, node ) );
            ASSERT_FALSE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":\"b\"}}" ), 1000, 4, false, node ) );
            ASSERT_FALSE( AUTH::CAuthJson::Parse( CORE::CString( "this is not json" ), 1000, 4, false, node ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A member that occurs twice, or is written with an escape, is refused where tokens and key sets are read" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            CORE::CDataNode node;
            ASSERT_FALSE( AUTH::CAuthJson::Parse( CORE::CString( "{\"alg\":\"RS256\",\"alg\":\"none\"}" ), 1000, 4, true, node ) );
            ASSERT_TRUE( AUTH::CAuthJson::Parse( CORE::CString( "{\"alg\":\"RS256\",\"alg\":\"none\"}" ), 1000, 4, false, node ) );
            // the same name in two objects is no duplicate
            ASSERT_TRUE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":{\"x\":1},\"b\":{\"x\":2},\"c\":[{\"x\":3},{\"x\":4}]}" ), 1000, 4, true, node ) );
            // a string value that looks like a key is no key
            ASSERT_TRUE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":\"a\",\"b\":\"a\"}" ), 1000, 4, true, node ) );
            // an escaped name hides a duplicate: refused
            ASSERT_FALSE( AUTH::CAuthJson::Parse( CORE::CString( "{\"alg\":\"RS256\",\"\\u0061lg\":\"none\"}" ), 1000, 4, true, node ) );
            ASSERT_TRUE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":\"b\\\"c\"}" ), 1000, 4, true, node ) );
            // commas and braces inside strings are not structure
            ASSERT_TRUE( AUTH::CAuthJson::Parse( CORE::CString( "{\"a\":\"x,\\\"b\\\":{\",\"b\":\"y\"}" ), 1000, 4, true, node ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Nested members are flattened into dotted names, arrays of text into one entry, quoting keeps JSON valid" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            CORE::CDataNode node;
            ASSERT_TRUE( AUTH::CAuthJson::Parse( CORE::CString( "{\"id\":\"42\",\"picture\":{\"data\":{\"url\":\"https://p.example/x.png\"}},\"amr\":[\"pwd\",\"mfa\"],\"n\":7,\"ok\":true,\"nothing\":null}" ), 1000, 8, false, node ) );
            CORE::CString::StringMap flat;
            AUTH::CAuthJson::Flatten( node, flat );
            ASSERT_TRUE( "42" == flat[ "id" ] && "https://p.example/x.png" == flat[ "picture.data.url" ] && "pwd mfa" == flat[ "amr" ] && "7" == flat[ "n" ] );
            ASSERT_TRUE( flat.end() == flat.find( "nothing" ) );

            // text beyond ASCII goes through as it is
            CORE::CString unicode = "Zo\xC3\xAB \xE2\x98\x83 \xF0\x9F\x8E\xAE";
            CORE::CDataNode unicodeBack;
            ASSERT_TRUE( AUTH::CAuthJson::Parse( "{\"v\":" + QuoteJson( unicode ) + "}", 1000, 4, true, unicodeBack ) );
            CORE::CString unicodeText;
            ASSERT_TRUE( AUTH::CAuthJson::GetString( unicodeBack, "v", unicodeText ) && unicode == unicodeText && AUTH::GetByteLength( unicodeText ) == AUTH::GetByteLength( unicode ) );

            CORE::CString quoted = QuoteJson( "a\"b\\c\nd\x01" );
            CORE::CDataNode back;
            ASSERT_TRUE( AUTH::CAuthJson::Parse( "{\"v\":" + quoted + "}", 1000, 4, true, back ) );
            CORE::CString text;
            ASSERT_TRUE( AUTH::CAuthJson::GetString( back, "v", text ) && "a\"b\\c\nd\x01" == text );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformJwkTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A key set holds the RSA signature keys, other key types are skipped and counted" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            CORE::CString document = "{\"keys\":[" + TestKeyA().ToJwk( "key-a" ) + "," + TestKeyB().ToJwk( "key-b", "" ) +
                                     ",{\"kty\":\"EC\",\"kid\":\"ec-1\",\"crv\":\"P-256\",\"x\":\"AAAA\",\"y\":\"AAAA\"}" +
                                     ",{\"kty\":\"oct\",\"kid\":\"hmac-1\",\"k\":\"c2VjcmV0\"}]}";
            AUTH::CJwks jwks;
            ASSERT_TRUE( jwks.Parse( document ) );
            ASSERT_TRUE( 2 == jwks.GetCount() && 2 == jwks.GetSkippedCount() );
            const AUTH::CJwk* a = jwks.Find( "key-a" );
            const AUTH::CJwk* b = jwks.Find( "key-b" );
            ASSERT_TRUE( GUCEF_NULL != a && GUCEF_NULL != b && GUCEF_NULL == jwks.Find( "ec-1" ) && GUCEF_NULL == jwks.Find( "hmac-1" ) && GUCEF_NULL == jwks.Find( "nobody" ) );
            if ( GUCEF_NULL != a && GUCEF_NULL != b )
            {
                ASSERT_TRUE( 2048 == a->GetModulusBits() && "RSA" == a->keyType && "RS256" == a->algorithm && "sig" == a->use );
                ASSERT_TRUE( a->CanVerify( "RS256" ) && !a->CanVerify( "RS384" ) && !a->CanVerify( "HS256" ) && !a->CanVerify( "none" ) );
                // a key without a pinned algorithm may do the three
                ASSERT_TRUE( b->CanVerify( "RS256" ) && b->CanVerify( "RS384" ) && b->CanVerify( "RS512" ) && !b->CanVerify( "ES256" ) );
                ASSERT_TRUE( 3 == a->exponent.GetDataSize() && 256 == a->modulus.GetDataSize() && 0x01 == static_cast< const UInt8* >( a->exponent.GetConstBufferPtr() )[ 0 ] );
            }

            // the document is refused when it is not one, or is not clear
            AUTH::CJwks bad;
            ASSERT_FALSE( bad.Parse( CORE::CString( "{\"keys\":\"no\"}" ) ) );
            ASSERT_FALSE( bad.Parse( CORE::CString( "{}" ) ) );
            ASSERT_FALSE( bad.Parse( CORE::CString( "[]" ) ) );
            ASSERT_FALSE( bad.Parse( "{\"keys\":[" + TestKeyA().ToJwk( "same" ) + "," + TestKeyB().ToJwk( "same" ) + "]}" ) );
            ASSERT_TRUE( 0 == bad.GetCount() );
            ASSERT_FALSE( bad.Parse( "{\"keys\":[" + TestKeyA().ToJwk( "" ) + "]}" ) );
            // a document that lists a key id twice in one key is read as a duplicate member
            ASSERT_FALSE( bad.Parse( CORE::CString( "{\"keys\":[{\"kty\":\"RSA\",\"kid\":\"a\",\"kid\":\"b\"}]}" ) ) );

            // too many keys
            CORE::CString many = "{\"keys\":[";
            for ( UInt32 i=0; i<=AUTH::CJwks::MaxKeys; ++i )
                many += ( i > 0 ? "," : "" ) + TestKeyA().ToJwk( "k" + CORE::UInt32ToString( i ) );
            many += "]}";
            ASSERT_FALSE( bad.Parse( many ) );
            ASSERT_TRUE( 0 == bad.GetCount() );

            // a document over the limit
            ASSERT_FALSE( bad.Parse( "{\"keys\":[],\"pad\":\"" + RepeatedText( "x", AUTH::CJwks::MaxDocumentBytes ) + "\"}" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A key is not usable when it is not RSA, is too short, is for encryption or pins an algorithm that is not accepted" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwk jwk;
            CORE::CString good = TestKeyA().ToJwk( "k" );
            ASSERT_TRUE( LoadJwk( good, jwk ) );

            ASSERT_FALSE( LoadJwk( good.ReplaceSubstr( "\"kty\":\"RSA\"", "\"kty\":\"EC\"" ), jwk ) );
            ASSERT_FALSE( LoadJwk( good.ReplaceSubstr( "\"kty\":\"RSA\"", "\"kty\":\"oct\"" ), jwk ) );
            ASSERT_FALSE( LoadJwk( good.ReplaceSubstr( "\"use\":\"sig\"", "\"use\":\"enc\"" ), jwk ) );
            ASSERT_FALSE( LoadJwk( good.ReplaceSubstr( "\"alg\":\"RS256\"", "\"alg\":\"HS256\"" ), jwk ) );
            ASSERT_FALSE( LoadJwk( good.ReplaceSubstr( "\"alg\":\"RS256\"", "\"alg\":\"none\"" ), jwk ) );
            ASSERT_FALSE( LoadJwk( "{\"kty\":\"RSA\",\"kid\":\"k\",\"n\":\"AQAB\",\"e\":\"AQAB\"}", jwk ) );                     // a 17 bit modulus
            ASSERT_FALSE( LoadJwk( "{\"kty\":\"RSA\",\"kid\":\"k\",\"e\":\"AQAB\"}", jwk ) );                                        // no modulus
            ASSERT_FALSE( LoadJwk( good.ReplaceSubstr( "\"e\":\"AQAB\"", "\"e\":\"\"" ), jwk ) );                                    // no exponent
            ASSERT_FALSE( LoadJwk( good.ReplaceSubstr( "\"e\":\"AQAB\"", "\"e\":\"AQA+\"" ), jwk ) );                                // not base64url
            ASSERT_FALSE( LoadJwk( good.ReplaceSubstr( "\"e\":\"AQAB\"", "\"e\":\"AQAB=\"" ), jwk ) );                               // padded

            // leading zero bytes of the integers are dropped
            CORE::CString n, e;
            CORE::CDynamicBuffer padded;
            UInt8 zero = 0;
            padded.Append( &zero, 1 );
            padded.Append( TestKeyA().GetModulus().GetConstBufferPtr(), TestKeyA().GetModulus().GetDataSize() );
            CORE::Base64UrlEncode( padded.GetConstBufferPtr(), padded.GetDataSize(), n );
            CORE::Base64UrlEncode( TestKeyA().GetExponent().GetConstBufferPtr(), TestKeyA().GetExponent().GetDataSize(), e );
            ASSERT_TRUE( LoadJwk( "{\"kty\":\"RSA\",\"kid\":\"k\",\"n\":\"" + n + "\",\"e\":\"" + e + "\"}", jwk ) && 256 == jwk.modulus.GetDataSize() && 2048 == jwk.GetModulusBits() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSignatureTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The test signer makes RS256 signatures that the codec of gucefCORE verifies, with every key, and not with another one" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwk a = KeyOf( TestKeyA(), "key-a" );
            AUTH::CJwk b = KeyOf( TestKeyB(), "key-b" );
            AUTH::CJwk c = KeyOf( TestKeyC(), "key-c" );
            ASSERT_TRUE( AUTH::JWT_OK == Check( TestKeyA().MakeToken( Header( "RS256", "key-a" ), Payload() ), Options(), a ) );
            ASSERT_TRUE( AUTH::JWT_OK == Check( TestKeyB().MakeToken( Header( "RS256", "key-b" ), Payload() ), Options(), b ) );
            ASSERT_TRUE( AUTH::JWT_OK == Check( TestKeyC().MakeToken( Header( "RS256", "key-c" ), Payload() ), Options(), c ) );

            // a token signed by one key and presented with the id of another: the key id says a, the signature is b's
            ASSERT_TRUE( AUTH::JWT_BAD_SIGNATURE == Check( TestKeyB().MakeToken( Header( "RS256", "key-a" ), Payload() ), Options(), a ) );
            ASSERT_TRUE( AUTH::JWT_BAD_SIGNATURE == Check( TestKeyC().MakeToken( Header( "RS256", "key-a" ), Payload() ), Options(), a ) );

            // the vectors made by openssl: RS384 and RS512 with the key of the tests
            AUTH::CJwk vector = KeyOf( TestKeyA(), "vector-a", "" );
            AUTH::CJwtValidationOptions options;
            options.issuer = "https://vectors.invalid";
            options.audience = "client";
            options.nowEpochSeconds = TestNow;
            ASSERT_TRUE( AUTH::JWT_OK == Check( g_testVectorRS384, options, vector ) );
            ASSERT_TRUE( AUTH::JWT_OK == Check( g_testVectorRS512, options, vector ) );

            // the algorithm of the token has to be the one the key allows
            AUTH::CJwk pinned = KeyOf( TestKeyA(), "vector-a", "RS256" );
            ASSERT_TRUE( AUTH::JWT_KEY_MISMATCH == Check( g_testVectorRS384, options, pinned ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Anything but RS256, RS384 and RS512 is refused: none, the HMAC algorithms, other families, other spellings" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwk a = KeyOf( TestKeyA(), "key-a", "" );
            const char* refused[] = { "none", "None", "NONE", "HS256", "HS384", "HS512", "ES256", "ES384", "PS256", "EdDSA", "rs256", "Rs256", "RS256 ", "RS1", "", "RS" };
            for ( size_t i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                CORE::CString token = Sign( Header( refused[ i ] ), Payload() );
                ASSERT_TRUE( AUTH::JWT_ALG_NOT_ALLOWED == Check( token, Options(), a ) );
            }

            // alg none with the shape of an unsigned token, and an HMAC token whose "signature" is a HMAC with the public key as the secret
            CORE::CString header, payload;
            CORE::CString h = Header( "none" ), p = Payload();
            CORE::Base64UrlEncode( h.C_String(), AUTH::GetByteLength( h ), header );
            CORE::Base64UrlEncode( p.C_String(), AUTH::GetByteLength( p ), payload );
            AUTH::CJwt jwt;
            ASSERT_TRUE( AUTH::JWT_ALG_NOT_ALLOWED == jwt.Parse( header + "." + payload + "." ) );
            ASSERT_TRUE( AUTH::JWT_ALG_NOT_ALLOWED == jwt.Parse( header + "." + payload + ".AAAA" ) );
            CORE::CString hmacHeader;
            h = Header( "HS256" );
            CORE::Base64UrlEncode( h.C_String(), AUTH::GetByteLength( h ), hmacHeader );
            // the MAC of the signing input with the modulus of the public key as the secret, which a library that takes the algorithm from the token accepts
            UInt8 macBytes[ CORE::CHmacSha256::MacSize ];
            CORE::CString hmacInput = hmacHeader + "." + payload;
            CORE::CHmacSha256::Mac( TestKeyA().GetModulus().GetConstBufferPtr(), TestKeyA().GetModulus().GetDataSize(), hmacInput.C_String(), AUTH::GetByteLength( hmacInput ), macBytes );
            CORE::CString mac;
            CORE::Base64UrlEncode( macBytes, sizeof( macBytes ), mac );
            ASSERT_TRUE( AUTH::JWT_ALG_NOT_ALLOWED == jwt.Parse( hmacInput + "." + mac ) );
            ASSERT_TRUE( AUTH::JWT_ALG_NOT_ALLOWED == Check( hmacInput + "." + mac, Options(), a ) );

            // a key set with a symmetric key does not make a key
            AUTH::CJwks jwks;
            ASSERT_TRUE( jwks.Parse( CORE::CString( "{\"keys\":[{\"kty\":\"oct\",\"kid\":\"key-a\",\"k\":\"" ) + mac + "\"}]}" ) && 0 == jwks.GetCount() );

            // the alg of a member written twice
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( Sign( "{\"alg\":\"RS256\",\"alg\":\"none\",\"kid\":\"key-a\"}", Payload() ) ) );
            // a header that asks for something that is not understood
            ASSERT_TRUE( AUTH::JWT_UNSUPPORTED_HEADER == jwt.Parse( Sign( "{\"alg\":\"RS256\",\"kid\":\"key-a\",\"crit\":[\"exp\"]}", Payload() ) ) );
            // the key is not taken from the token
            ASSERT_TRUE( AUTH::JWT_OK == jwt.Parse( Sign( "{\"alg\":\"RS256\",\"kid\":\"key-a\",\"jwk\":{\"kty\":\"RSA\"},\"jku\":\"https://evil.example/keys\",\"x5u\":\"https://evil.example/c\"}", Payload() ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The key id is required and has to be the one of the key" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwk a = KeyOf( TestKeyA(), "key-a" );
            AUTH::CJwt jwt;
            ASSERT_TRUE( AUTH::JWT_KEY_ID_MISSING == jwt.Parse( Sign( "{\"alg\":\"RS256\"}", Payload() ) ) );
            ASSERT_TRUE( AUTH::JWT_KEY_ID_MISSING == jwt.Parse( Sign( "{\"alg\":\"RS256\",\"kid\":\"\"}", Payload() ) ) );
            ASSERT_TRUE( AUTH::JWT_KEY_ID_MISSING == jwt.Parse( Sign( "{\"alg\":\"RS256\",\"kid\":5}", Payload() ) ) );
            ASSERT_TRUE( AUTH::JWT_KEY_ID_MISSING == jwt.Parse( Sign( "{\"alg\":\"RS256\",\"kid\":\"" + RepeatedText( "k", 300 ) + "\"}", Payload() ) ) );
            ASSERT_TRUE( AUTH::JWT_OK == jwt.Parse( Sign( Header( "RS256", "key-other" ), Payload() ) ) && "key-other" == jwt.GetKeyId() && "RS256" == jwt.GetAlgorithm() );
            // the signature of a good key does not count for a token that names another key id
            ASSERT_TRUE( AUTH::JWT_KEY_MISMATCH == jwt.VerifySignature( a ) );
            // a token that was not parsed is not verified
            AUTH::CJwt empty;
            ASSERT_TRUE( AUTH::JWT_MALFORMED == empty.VerifySignature( a ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A token that is not a compact JWS is malformed; sizes are limited; the signature is checked to the bit" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwk a = KeyOf( TestKeyA(), "key-a" );
            CORE::CString good = Sign( Header(), Payload() );
            AUTH::CJwt jwt;
            ASSERT_TRUE( AUTH::JWT_OK == jwt.Parse( good ) );

            // the shape
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( CORE::CString::Empty ) );
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( CORE::CString( "abc" ) ) );
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( CORE::CString( "a.b" ) ) );
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( good + ".extra" ) );
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( CORE::CString( ".." ) ) );
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( "." + good ) );
            CORE::CString::StringVector parts = good.ParseElements( '.', true );
            ASSERT_TRUE( 3 == parts.size() );
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( parts[ 0 ] + "." + parts[ 1 ] + "." ) );                          // an empty signature of a good header
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( parts[ 0 ] + ".." + parts[ 2 ] ) );                               // an empty payload
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( parts[ 0 ] + "." + parts[ 1 ] + "." + parts[ 2 ] + "=" ) );        // padding
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( parts[ 0 ] + "." + parts[ 1 ] + "+." + parts[ 2 ] ) );             // the standard alphabet
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( parts[ 0 ] + "." + parts[ 1 ] + "/." + parts[ 2 ] ) );
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( parts[ 0 ] + ". " + parts[ 1 ] + "." + parts[ 2 ] ) );
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( parts[ 0 ] + "." + CORE::CString( "bm90IGpzb24" ) + "." + parts[ 2 ] ) );    // "not json"
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( CORE::CString( "bm90IGpzb24." ) + parts[ 1 ] + "." + parts[ 2 ] ) );
            // a payload that is JSON but not an object
            CORE::CString arrayPayload;
            CORE::Base64UrlEncode( "[1,2]", 5, arrayPayload );
            AUTH::CJwt arrayJwt;
            AUTH::TJwtStatus arrayStatus = arrayJwt.Parse( parts[ 0 ] + "." + arrayPayload + "." + parts[ 2 ] );
            ASSERT_TRUE( AUTH::JWT_OK != arrayStatus || AUTH::JWT_OK != arrayJwt.VerifySignature( a ) || AUTH::JWT_OK != arrayJwt.ValidateClaims( Options() ) );

            // sizes: the token, the header and the payload
            ASSERT_TRUE( AUTH::JWT_TOO_LARGE == jwt.Parse( good, 100 ) );
            ASSERT_TRUE( AUTH::JWT_TOO_LARGE == jwt.Parse( RepeatedText( "A", AUTH::CJwt::MaxTokenBytes + 1 ) ) );
            ASSERT_TRUE( AUTH::JWT_TOO_LARGE == jwt.Parse( Sign( "{\"alg\":\"RS256\",\"kid\":\"key-a\",\"x\":\"" + RepeatedText( "h", AUTH::CJwt::MaxHeaderBytes ) + "\"}", Payload() ) ) );
            ASSERT_TRUE( AUTH::JWT_TOO_LARGE == jwt.Parse( Sign( Header(), Payload( "\"pad\":\"" + RepeatedText( "p", AUTH::CJwt::MaxPayloadBytes ) + "\"" ) ) ) );
            // a payload just under the limit is fine (the limit of the whole token binds first for a payload this big)
            ASSERT_TRUE( AUTH::JWT_OK == jwt.Parse( Sign( Header(), Payload( "\"pad\":\"" + RepeatedText( "p", AUTH::CJwt::MaxPayloadBytes - 1500 ) + "\"" ) ) ) );
            // and deep payloads are not
            ASSERT_TRUE( AUTH::JWT_MALFORMED == jwt.Parse( Sign( Header(), Payload( "\"deep\":" + RepeatedText( "[", 30 ) + RepeatedText( "]", 30 ) ) ) ) );

            // a signature is valid or not: change every character of the signature part, and bytes of the payload
            ASSERT_TRUE( AUTH::JWT_OK == Check( good, Options(), a ) );
            const CORE::CString& signature = parts[ 2 ];
            UInt32 flipped = 0, rejected = 0;
            for ( UInt32 i=0; i<signature.Length(); i+=7 )
            {
                CORE::CString changed = AUTH::MakeString( signature.C_String(), i );
                changed += signature.C_String()[ i ] == 'A' ? 'B' : 'A';
                changed += signature.C_String() + i + 1;
                AUTH::TJwtStatus status = Check( parts[ 0 ] + "." + parts[ 1 ] + "." + changed, Options(), a );
                ++flipped;
                if ( AUTH::JWT_OK != status )
                    ++rejected;
            }
            ASSERT_TRUE( flipped > 30 && flipped == rejected );

            CORE::CString otherPayload;
            CORE::CString changedClaims = Payload().ReplaceSubstr( "s-1", "s-2" );
            CORE::Base64UrlEncode( changedClaims.C_String(), AUTH::GetByteLength( changedClaims ), otherPayload );
            ASSERT_TRUE( AUTH::JWT_BAD_SIGNATURE == Check( parts[ 0 ] + "." + otherPayload + "." + parts[ 2 ], Options(), a ) );

            // a signature that is too long, or too short, for the key
            ASSERT_TRUE( AUTH::JWT_BAD_SIGNATURE == Check( parts[ 0 ] + "." + parts[ 1 ] + "." + AUTH::MakeString( signature.C_String(), 300 ), Options(), a ) );
            ASSERT_TRUE( AUTH::JWT_TOO_LARGE == Check( parts[ 0 ] + "." + parts[ 1 ] + "." + RepeatedText( "A", 2000 ), Options(), a ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static AUTH::TJwtStatus
Validate( const CORE::CString& payload, const AUTH::CJwtValidationOptions& options, CORE::CString* tenant = GUCEF_NULL )
{GUCEF_TRACE;

    AUTH::CJwt jwt;
    AUTH::TJwtStatus status = jwt.Parse( Sign( Header(), payload ) );
    if ( AUTH::JWT_OK != status )
        return status;
    return jwt.ValidateClaims( options, tenant );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Claims( const CORE::CString& issuer, const CORE::CString& audience, const CORE::CString& exp, const CORE::CString& extra = CORE::CString::Empty )
{GUCEF_TRACE;

    CORE::CString payload = "{\"sub\":\"s-1\"";
    if ( !issuer.IsNULLOrEmpty() )
        payload += ",\"iss\":" + issuer;
    if ( !audience.IsNULLOrEmpty() )
        payload += ",\"aud\":" + audience;
    if ( !exp.IsNULLOrEmpty() )
        payload += ",\"exp\":" + exp;
    if ( !extra.IsNULLOrEmpty() )
        payload += "," + extra;
    return payload + "}";
}

/*-------------------------------------------------------------------------*/

static void
PerformClaimTests( void )
{GUCEF_TRACE;

    const CORE::CString goodExp = CORE::UInt64ToString( TestNow + 600 );

    GUCEF_TESTFW_TESTCASE( "Test: The issuer is compared exactly; an issuer with {tid} takes the tenant of the token and nothing that is not a tenant" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwtValidationOptions options = Options();
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Payload(), options ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://issuer.example/\"", "\"client-1\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://ISSUER.example\"", "\"client-1\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://issuer.example.evil\"", "\"client-1\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"http://issuer.example\"", "\"client-1\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"\"", "\"client-1\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( CORE::CString::Empty, "\"client-1\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( "5", "\"client-1\"", goodExp ), options ) );
            options.issuer = CORE::CString::Empty;
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Payload(), options ) );

            // the tenant
            AUTH::CJwtValidationOptions tenantOptions = Options();
            tenantOptions.issuer = "https://login.example/{tid}/v2.0";
            CORE::CString tenant;
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( "\"https://login.example/9188040d-6c67-4c5b-b112-36a304b66dad/v2.0\"", "\"client-1\"", goodExp, "\"tid\":\"9188040d-6c67-4c5b-b112-36a304b66dad\"" ), tenantOptions, &tenant ) );
            ASSERT_TRUE( "9188040d-6c67-4c5b-b112-36a304b66dad" == tenant );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( "\"https://login.example/tenant-1/v2.0\"", "\"client-1\"", goodExp ), tenantOptions, &tenant ) && "tenant-1" == tenant );
            // the tid claim has to agree with the issuer
            ASSERT_TRUE( AUTH::JWT_TENANT == Validate( Claims( "\"https://login.example/tenant-1/v2.0\"", "\"client-1\"", goodExp, "\"tid\":\"tenant-2\"" ), tenantOptions ) );
            // the tenant is a name: not a path, a query, an empty text or something with a host in it
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://login.example//v2.0\"", "\"client-1\"", goodExp ), tenantOptions ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://login.example/a/b/v2.0\"", "\"client-1\"", goodExp ), tenantOptions ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://login.example/a?x=1/v2.0\"", "\"client-1\"", goodExp ), tenantOptions ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://login.example/a%2Fb/v2.0\"", "\"client-1\"", goodExp ), tenantOptions ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://login.example/tenant-1/v3.0\"", "\"client-1\"", goodExp ), tenantOptions ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://other.example/tenant-1/v2.0\"", "\"client-1\"", goodExp ), tenantOptions ) );
            ASSERT_TRUE( AUTH::JWT_ISSUER == Validate( Claims( "\"https://login.example/tenant-1/v2.0/\"", "\"client-1\"", goodExp ), tenantOptions ) );

            CORE::CString id;
            ASSERT_TRUE( AUTH::CJwt::MatchIssuer( "https://x/{tid}", "https://x/abc", id ) && "abc" == id );
            ASSERT_FALSE( AUTH::CJwt::MatchIssuer( "https://x/{tid}", "https://x/", id ) );
            ASSERT_TRUE( AUTH::CJwt::MatchIssuer( "https://x/y", "https://x/y", id ) && id.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The audience is a string or a list that contains the client id; with several the authorized party is the client" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwtValidationOptions options = Options();
            const CORE::CString iss = "\"https://issuer.example\"";
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "\"client-1\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "[\"client-1\"]", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"client-2\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"client-1x\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"CLIENT-1\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "[\"client-2\",\"client-3\"]", goodExp, "\"azp\":\"client-1\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( iss, CORE::CString::Empty, goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( iss, "[]", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( iss, "7", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( iss, "[\"client-1\",7]", goodExp ), options ) );
            options.audience = CORE::CString::Empty;
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"\"", goodExp ), options ) );
            options.audience = "client-1";

            // several audiences need the authorized party
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "[\"client-1\",\"other\"]", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "[\"client-1\",\"other\"]", goodExp, "\"azp\":\"other\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "[\"client-1\",\"other\"]", goodExp, "\"azp\":\"client-1\"" ), options ) );
            // an authorized party that is there is the client, also with one audience
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "\"client-1\"", goodExp, "\"azp\":\"client-1\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "\"client-1\"", goodExp, "\"azp\":\"other\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "\"client-1\"", goodExp, "\"azp\":5" ), options ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Several client ids are accepted as audience: any of them, and the authorized party is one of them" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            const CORE::CString iss = "\"https://issuer.example\"";
            AUTH::CJwtValidationOptions options = Options();
            options.audience = CORE::CString::Empty;
            options.audiences.push_back( "client-desktop" );
            options.audiences.push_back( "client-android" );

            // one audience, any of the client ids
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "\"client-desktop\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "\"client-android\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "[\"client-android\"]", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"client-web\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"client-android2\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"CLIENT-ANDROID\"", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "[\"client-web\",\"client-other\"]", goodExp, "\"azp\":\"client-android\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( iss, "[]", goodExp ), options ) );

            // the client id of the build next to the list is one of them too
            AUTH::CJwtValidationOptions both = Options();
            both.audiences.push_back( "client-android" );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "\"client-1\"", goodExp ), both ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "\"client-android\"", goodExp ), both ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"client-desktop\"", goodExp ), both ) );

            // a list of audiences: one of them is ours and the authorized party is one of ours
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "[\"client-android\",\"client-web\"]", goodExp, "\"azp\":\"client-android\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "[\"client-web\",\"client-desktop\"]", goodExp, "\"azp\":\"client-desktop\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "[\"client-desktop\",\"client-android\"]", goodExp, "\"azp\":\"client-android\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "[\"client-android\",\"client-web\"]", goodExp ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "[\"client-android\",\"client-web\"]", goodExp, "\"azp\":\"client-web\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "[\"client-android\",\"client-web\"]", goodExp, "\"azp\":\"\"" ), options ) );
            // the authorized party is a client of ours, it need not be one of the audiences: the token of the Android client of a game for its web client
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "[\"client-android\",\"client-web\"]", goodExp, "\"azp\":\"client-desktop\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "\"client-desktop\"", goodExp, "\"azp\":\"client-android\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "[\"client-android\",\"client-web\"]", goodExp, "\"azp\":7" ), options ) );

            // one audience: an authorized party that is there has to be one of ours
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, "\"client-android\"", goodExp, "\"azp\":\"client-android\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "\"client-android\"", goodExp, "\"azp\":\"client-web\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_AUTHORIZED_PARTY == Validate( Claims( iss, "\"client-android\"", goodExp, "\"azp\":\"\"" ), options ) );

            // nothing configured accepts nothing, also not an empty audience; empty entries are not client ids
            AUTH::CJwtValidationOptions none = Options();
            none.audience = CORE::CString::Empty;
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"client-1\"", goodExp ), none ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"\"", goodExp ), none ) );
            none.audiences.push_back( CORE::CString::Empty );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"\"", goodExp ), none ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "[\"\",\"client-1\"]", goodExp, "\"azp\":\"\"" ), none ) );
            ASSERT_TRUE( AUTH::JWT_AUDIENCE == Validate( Claims( iss, "\"client-1\"", goodExp ), none ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: exp, nbf and iat hold with a clock skew that is 120 seconds, can be set and is capped" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwtValidationOptions options = Options();
            ASSERT_TRUE( 120 == options.clockSkewInSeconds && 120 == AUTH::CJwtValidationOptions::DefaultClockSkewInSeconds );
            const CORE::CString iss = "\"https://issuer.example\"", aud = "\"client-1\"";

            // exp: required, a number, and in the past only by the skew
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( iss, aud, CORE::CString::Empty ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( iss, aud, "\"soon\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( iss, aud, "\"" + goodExp + "\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow + 1 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow - 119 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_EXPIRED == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow - 120 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_EXPIRED == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow - 100000 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_EXPIRED == Validate( Claims( iss, aud, "0" ), options ) );
            ASSERT_TRUE( AUTH::JWT_EXPIRED == Validate( Claims( iss, aud, "-5" ), options ) );
            // a fractional time is a time
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow + 600 ) + ".5" ), options ) );

            // nbf: not yet valid by more than the skew
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, goodExp, "\"nbf\":" + CORE::UInt64ToString( TestNow - 5 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, goodExp, "\"nbf\":" + CORE::UInt64ToString( TestNow + 120 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_NOT_YET_VALID == Validate( Claims( iss, aud, goodExp, "\"nbf\":" + CORE::UInt64ToString( TestNow + 121 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_NOT_YET_VALID == Validate( Claims( iss, aud, goodExp, "\"nbf\":" + CORE::UInt64ToString( TestNow + 3600 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_CLAIM_MISSING == Validate( Claims( iss, aud, goodExp, "\"nbf\":\"later\"" ), options ) );

            // iat: not in the future by more than the skew
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, goodExp, "\"iat\":" + CORE::UInt64ToString( TestNow + 120 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_ISSUED_IN_FUTURE == Validate( Claims( iss, aud, goodExp, "\"iat\":" + CORE::UInt64ToString( TestNow + 121 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, goodExp, "\"iat\":" + CORE::UInt64ToString( TestNow - 100000 ) ), options ) );

            // the skew is configurable, down to nothing, and capped
            options.clockSkewInSeconds = 0;
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow + 1 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_EXPIRED == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_NOT_YET_VALID == Validate( Claims( iss, aud, goodExp, "\"nbf\":" + CORE::UInt64ToString( TestNow + 1 ) ), options ) );
            options.clockSkewInSeconds = 300;
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow - 299 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_EXPIRED == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow - 300 ) ), options ) );
            options.clockSkewInSeconds = 100000;
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow - 599 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_EXPIRED == Validate( Claims( iss, aud, CORE::UInt64ToString( TestNow - 600 ) ), options ) );

            // the clock when no moment is given
            options = Options();
            options.nowEpochSeconds = 0;
            UInt64 realNow = AUTH::GetEpochTimeInMs() / 1000;
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, CORE::UInt64ToString( realNow + 600 ) ), options ) );
            ASSERT_TRUE( AUTH::JWT_EXPIRED == Validate( Claims( iss, aud, CORE::UInt64ToString( realNow - 600 ) ), options ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The nonce has to be the one of the login, and the at_hash the hash of the access token, with the hash of the algorithm" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            const CORE::CString iss = "\"https://issuer.example\"", aud = "\"client-1\"";
            AUTH::CJwtValidationOptions options = Options();
            options.checkNonce = true;
            options.nonce = "n-0S6_WzA2Mj";
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, goodExp, "\"nonce\":\"n-0S6_WzA2Mj\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_NONCE == Validate( Claims( iss, aud, goodExp, "\"nonce\":\"n-0S6_WzA2Mk\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_NONCE == Validate( Claims( iss, aud, goodExp, "\"nonce\":\"n-0S6_WzA2M\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_NONCE == Validate( Claims( iss, aud, goodExp, "\"nonce\":\"n-0S6_WzA2Mjx\"" ), options ) );
            ASSERT_TRUE( AUTH::JWT_NONCE == Validate( Claims( iss, aud, goodExp, "\"nonce\":5" ), options ) );
            ASSERT_TRUE( AUTH::JWT_NONCE == Validate( Claims( iss, aud, goodExp ), options ) );
            // a login without a nonce to compare with does not accept a token with one
            options.nonce = CORE::CString::Empty;
            ASSERT_TRUE( AUTH::JWT_NONCE == Validate( Claims( iss, aud, goodExp, "\"nonce\":\"\"" ), options ) );
            options.checkNonce = false;
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, goodExp, "\"nonce\":\"anything\"" ), options ) );

            // OpenID Connect Core, appendix A.3: the access token and its at_hash
            CORE::CString half;
            ASSERT_TRUE( AUTH::CJwt::ComputeHalfHash( "RS256", "jHkWEdUXMU1BwAsC4vtUsZwnNvTIxEl0z9K3vx5KF0Y", half ) && "77QmUPtjPfzWtF2AnpK9RQ" == half );
            ASSERT_TRUE( AUTH::CJwt::ComputeHalfHash( "RS384", "x", half ) && 32 == half.Length() );     // 24 bytes
            ASSERT_TRUE( AUTH::CJwt::ComputeHalfHash( "RS512", "x", half ) && 43 == half.Length() );     // 32 bytes
            ASSERT_FALSE( AUTH::CJwt::ComputeHalfHash( "HS256", "x", half ) );

            AUTH::CJwtValidationOptions hashOptions = Options();
            hashOptions.accessToken.Set( "jHkWEdUXMU1BwAsC4vtUsZwnNvTIxEl0z9K3vx5KF0Y" );
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, goodExp, "\"at_hash\":\"77QmUPtjPfzWtF2AnpK9RQ\"" ), hashOptions ) );
            ASSERT_TRUE( AUTH::JWT_AT_HASH == Validate( Claims( iss, aud, goodExp, "\"at_hash\":\"77QmUPtjPfzWtF2AnpK9RR\"" ), hashOptions ) );
            ASSERT_TRUE( AUTH::JWT_AT_HASH == Validate( Claims( iss, aud, goodExp, "\"at_hash\":5" ), hashOptions ) );
            // without an at_hash it is not checked, unless asked
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, goodExp ), hashOptions ) );
            hashOptions.requireAtHash = true;
            ASSERT_TRUE( AUTH::JWT_AT_HASH == Validate( Claims( iss, aud, goodExp ), hashOptions ) );
            // without an access token there is nothing to check
            AUTH::CJwtValidationOptions noToken = Options();
            ASSERT_TRUE( AUTH::JWT_OK == Validate( Claims( iss, aud, goodExp, "\"at_hash\":\"whatever\"" ), noToken ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static AUTH::CJwks
SetOf( const char* const* ids, UInt32 count )
{GUCEF_TRACE;

    AUTH::CJwks jwks;
    for ( UInt32 i=0; i<count; ++i )
        jwks.Add( KeyOf( TestKeyA(), ids[ i ] ) );
    return jwks;
}

/*-------------------------------------------------------------------------*/

static void
PerformCacheTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Cache-Control max-age is read; a header that forbids caching gives the shortest life, none gives nothing" )
        try
        {
            ASSERT_TRUE( 3600 == AUTH::CJwksCache::ParseMaxAge( "public, max-age=3600" ) );
            ASSERT_TRUE( 19921 == AUTH::CJwksCache::ParseMaxAge( "public, max-age=19921, must-revalidate, no-transform" ) );
            ASSERT_TRUE( 60 == AUTH::CJwksCache::ParseMaxAge( "MAX-AGE=60" ) );
            ASSERT_TRUE( 5 == AUTH::CJwksCache::ParseMaxAge( "private,max-age=5" ) );
            ASSERT_TRUE( 1 == AUTH::CJwksCache::ParseMaxAge( "no-store, max-age=100" ) );
            ASSERT_TRUE( 1 == AUTH::CJwksCache::ParseMaxAge( "no-cache" ) );
            ASSERT_TRUE( 0 == AUTH::CJwksCache::ParseMaxAge( "public" ) );
            ASSERT_TRUE( 0 == AUTH::CJwksCache::ParseMaxAge( CORE::CString::Empty ) );
            ASSERT_TRUE( 0 == AUTH::CJwksCache::ParseMaxAge( "max-age=abc" ) );
            ASSERT_TRUE( 0 == AUTH::CJwksCache::ParseMaxAge( "max-age=" ) );
            ASSERT_TRUE( 0 == AUTH::CJwksCache::ParseMaxAge( "s-maxage=500" ) );
            ASSERT_TRUE( GUCEF_UINT32MAX == AUTH::CJwksCache::ParseMaxAge( "max-age=99999999999" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A key set lives as long as the provider says within bounds, and is fetched again after that" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwksCache cache;
            const char* ids[] = { "a", "b" };
            AUTH::CJwks jwks = SetOf( ids, 2 );
            AUTH::CJwk key;
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://p/keys", "a", 1000, key ) );

            // no max-age: the default of an hour
            cache.Store( "https://p/keys", jwks, 0, 1000 );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( "https://p/keys", "a", 1000, key ) && "a" == key.keyId );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( "https://p/keys", "b", 1000 + 3599 * 1000, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://p/keys", "a", 1000 + 3600 * 1000, key ) );

            // the max-age of the provider
            cache.Store( "https://p/keys", jwks, 300, 5000 );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( "https://p/keys", "a", 5000 + 299 * 1000, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://p/keys", "a", 5000 + 300 * 1000, key ) );

            // within bounds: not shorter than the minimum, not longer than the maximum
            cache.Store( "https://p/keys", jwks, 1, 0 );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( "https://p/keys", "a", 59 * 1000, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://p/keys", "a", 60 * 1000, key ) );
            cache.Store( "https://p/keys", jwks, 10000000, 0 );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( "https://p/keys", "a", 86399 * 1000, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://p/keys", "a", 86400 * 1000, key ) );

            // the bounds are settings
            cache.SetPolicy( 1, 10, 5, 100 );
            cache.Store( "https://p/keys", jwks, 0, 0 );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( "https://p/keys", "a", 4999, key ) && AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://p/keys", "a", 5000, key ) );
            cache.Store( "https://p/keys", jwks, 1000, 0 );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( "https://p/keys", "a", 9999, key ) && AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://p/keys", "a", 10000, key ) );

            // one set per address
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://other/keys", "a", 0, key ) );
            cache.Remove( "https://p/keys" );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://p/keys", "a", 0, key ) );

            // the number of sets is limited
            for ( UInt32 i=0; i<AUTH::CJwksCache::MaxEntries + 8; ++i )
                cache.Store( "https://p/" + CORE::UInt32ToString( i ), jwks, 0, i );
            UInt32 present = 0;
            for ( UInt32 i=0; i<AUTH::CJwksCache::MaxEntries + 8; ++i )
                if ( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( "https://p/" + CORE::UInt32ToString( i ), "a", AUTH::CJwksCache::MaxEntries + 8, key ) )
                    ++present;
            ASSERT_TRUE( AUTH::CJwksCache::MaxEntries == present );
            ASSERT_TRUE( 0 < cache.GetStoreCount() );
            cache.Clear();
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FETCH_NEEDED == cache.Lookup( "https://p/1", "a", 0, key ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A key id that is not in the set allows a refresh, but not right after a fetch and not more than once per interval" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::CJwksCache cache;
            cache.SetPolicy( 60, 86400, 3600, 10000 );
            const char* ids[] = { "a" };
            AUTH::CJwks jwks = SetOf( ids, 1 );
            AUTH::CJwk key;
            const CORE::CString address = "https://p/keys";

            cache.Store( address, jwks, 0, 100000 );
            // right after the fetch the answer is that the key is not there
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_UNKNOWN_KEY == cache.Lookup( address, "new", 100000, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_UNKNOWN_KEY == cache.Lookup( address, "new", 109999, key ) );
            // after the interval the set is refreshed, once
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_REFRESH_NEEDED == cache.Lookup( address, "new", 110000, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_UNKNOWN_KEY == cache.Lookup( address, "new", 110001, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_UNKNOWN_KEY == cache.Lookup( address, "other", 119999, key ) );
            // known keys are not touched by any of it
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( address, "a", 110002, key ) );

            // the refresh stored a set again: the interval starts at that fetch, a key that is in it is found
            const char* rotated[] = { "a", "new" };
            cache.Store( address, SetOf( rotated, 2 ), 0, 110500 );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_FOUND == cache.Lookup( address, "new", 110600, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_UNKNOWN_KEY == cache.Lookup( address, "newer", 120499, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_REFRESH_NEEDED == cache.Lookup( address, "newer", 120500, key ) );

            // an interval of nothing allows a refresh for every unknown key
            cache.SetPolicy( 60, 86400, 3600, 0 );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_REFRESH_NEEDED == cache.Lookup( address, "x", 120501, key ) );
            ASSERT_TRUE( AUTH::CJwksCache::LOOKUP_REFRESH_NEEDED == cache.Lookup( address, "x", 120501, key ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformJwtTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING JWT TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Jwt" );

    PerformJsonTests();
    PerformJwkTests();
    PerformSignatureTests();
    PerformClaimTests();
    PerformCacheTests();
}

/*-------------------------------------------------------------------------*/
