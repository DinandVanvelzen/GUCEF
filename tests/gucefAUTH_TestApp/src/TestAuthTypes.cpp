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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestLogCapture.h"
#include "TestFiles.h"
#include "TestSigner.h"
#include "TestAuthTypes.h"

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

void
PerformAuthTypesTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING auth types and crypto TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "AuthTypes" );

    GUCEF_TESTFW_TESTCASE( "Test: A secret string never shows in a stream or a log" )
        try
        {
            const CORE::CString token = "tok-9f8e7d6c5b4a-never-in-a-log";
            AUTH::CSecretString secret( token );
            AUTH::CSecretString empty;
            ASSERT_TRUE( !secret.IsEmpty() && empty.IsEmpty() && token.Length() == secret.Length() );
            ASSERT_TRUE( token == secret.Reveal() );

            CORE::CVariantStream stream;
            stream << "token=" << secret << " none=" << empty;
            CORE::CString text = stream.ToString();
            ASSERT_TRUE( text.HasSubstr( token ) < 0 );
            ASSERT_TRUE( text.HasSubstr( "[redacted]" ) >= 0 && text.HasSubstr( "[empty]" ) >= 0 );

            CLogCapture capture;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "the login gave " << secret << " and more" );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "failed with " << secret );
            ASSERT_TRUE( capture.Contains( "the login gave [redacted] and more" ) );
            ASSERT_FALSE( capture.Contains( token ) );
            ASSERT_FALSE( capture.Contains( "9f8e7d6c5b4a" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A secret string compares in constant time, copies and clears" )
        try
        {
            AUTH::CSecretString a( "abc" );
            AUTH::CSecretString b( "abc" );
            AUTH::CSecretString c( "abd" );
            AUTH::CSecretString d( "abcd" );
            ASSERT_TRUE( a == b && a != c && a != d && !( a == d ) );

            AUTH::CSecretString copy( a );
            ASSERT_TRUE( copy == a );
            copy.Clear();
            ASSERT_TRUE( copy.IsEmpty() && "abc" == a.Reveal() );
            AUTH::CSecretString assigned;
            assigned = c;
            ASSERT_TRUE( assigned == c );
            assigned.Set( "x" );
            ASSERT_TRUE( "x" == assigned.Reveal() && "abd" == c.Reveal() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An auth result keeps its tokens out of serialization and logs" )
        try
        {
            AUTH::CAuthResult result;
            result.providerName = "google";
            result.accountRef = AUTH::CAccountRef( "google", "112233" );
            result.displayName = "Alice";
            result.avatarUrl = "https://example.invalid/a.png";
            result.accessToken.Set( "ACCESS-0123456789" );
            result.idToken.Set( "IDTOKEN-0123456789" );
            result.refreshToken.Set( "REFRESH-0123456789" );
            result.expiresAtMs = 1700000000000ULL;
            result.scopes.push_back( "openid" );
            result.scopes.push_back( "profile" );
            result.proof = AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "PROOF-0123456789" );
            ASSERT_TRUE( result.IsValid() );

            CORE::CDataNode node( "result", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( result.Serialize( node, CORE::CDataNodeSerializableSettings() ) );
            CORE::CDataNode dump;
            dump.Copy( node );
            // nothing of a secret is in the tree
            CORE::CDataNode::TAttributeMap::const_iterator i = node.AttributeBegin();
            while ( i != node.AttributeEnd() )
            {
                ASSERT_TRUE( (*i).second.AsString().HasSubstr( "0123456789" ) < 0 );
                ++i;
            }
            ASSERT_TRUE( GUCEF_NULL == node.FindChild( "accessToken" ) && GUCEF_NULL == node.FindChild( "refreshToken" ) );

            AUTH::CAuthResult back;
            ASSERT_TRUE( back.Deserialize( node, CORE::CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( back.accountRef == result.accountRef && "Alice" == back.displayName && result.expiresAtMs == back.expiresAtMs );
            ASSERT_TRUE( 2 == back.scopes.size() && "profile" == back.scopes[ 1 ] );
            ASSERT_TRUE( back.accessToken.IsEmpty() && back.idToken.IsEmpty() && back.refreshToken.IsEmpty() && back.proof.IsEmpty() );
            ASSERT_TRUE( AUTH::CAuthProof::KindOidcIdToken == back.proof.kind );

            CLogCapture capture;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "result tokens " << result.accessToken << " " << result.idToken << " " << result.refreshToken << " " << result.proof.data );
            ASSERT_FALSE( capture.Contains( "0123456789" ) );

            // a clone carries everything, a wipe takes the secrets away
            CORE::CICloneable* cloned = result.Clone();
            const AUTH::CAuthResult* clonedResult = CORE::CICloneable::CloneableAs< AUTH::CAuthResult >( cloned );
            ASSERT_TRUE( GUCEF_NULL != clonedResult && "REFRESH-0123456789" == clonedResult->refreshToken.Reveal() );
            GUCEF_DELETE cloned;
            result.WipeSecrets();
            ASSERT_TRUE( result.accessToken.IsEmpty() && result.idToken.IsEmpty() && result.refreshToken.IsEmpty() && result.proof.IsEmpty() );
            ASSERT_TRUE( result.IsValid() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Expiry with a skew, the proof digest and a result from a future" )
        try
        {
            AUTH::CAuthResult result;
            ASSERT_FALSE( result.IsExpired( 1000000 ) );  // no expiry known
            result.expiresAtMs = 10000;
            ASSERT_FALSE( result.IsExpired( 9999 ) );
            ASSERT_TRUE( result.IsExpired( 10000 ) && result.IsExpired( 10001 ) );
            ASSERT_TRUE( result.IsExpired( 9000, 1000 ) && !result.IsExpired( 8999, 1000 ) );

            AUTH::CAuthProof proof( "oidc-id-token", "a.b.c" );
            CORE::CString digest = proof.GetDigestHex();
            ASSERT_TRUE( 64 == digest.Length() );
            // SHA-256 of "oidc-id-token:a.b.c"
            ASSERT_TRUE( digest == AUTH::CAuthCrypto::Sha256Hex( "oidc-id-token:a.b.c", 19 ) );
            ASSERT_TRUE( AUTH::CAuthProof( "other", "a.b.c" ).GetDigestHex() != digest );
            ASSERT_TRUE( AUTH::CAuthProof().GetDigestHex().IsNULLOrEmpty() );

            CORE::CFutureResultSource source;
            AUTH::CAuthResult none;
            ASSERT_FALSE( AUTH::CAuthResult::TryGetFromFuture( source.GetFuture(), none ) );
            AUTH::CAuthResult done;
            done.accountRef = AUTH::CAccountRef( "epic", "1" );
            done.refreshToken.Set( "r" );
            source.Complete( done.Clone(), true );
            AUTH::CAuthResult fetched;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( source.GetFuture(), fetched ) && fetched.accountRef == done.accountRef && "r" == fetched.refreshToken.Reveal() );

            CORE::CFutureResultSource failed;
            failed.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "denied: no" );
            ASSERT_FALSE( AUTH::CAuthResult::TryGetFromFuture( failed.GetFuture(), fetched ) );

            // a future that finished with data of another type, or with data that does not tell its type, is refused
            CORE::CFutureResultSource wrongType;
            wrongType.Complete( GUCEF_NEW CORE::TCloneableString( "not an auth result" ), true );
            AUTH::CAuthResult untouched;
            ASSERT_FALSE( AUTH::CAuthResult::TryGetFromFuture( wrongType.GetFuture(), untouched ) );
            ASSERT_FALSE( untouched.IsValid() );
            CORE::CFutureResultSource noData;
            noData.Complete( GUCEF_NULL, true );
            ASSERT_FALSE( AUTH::CAuthResult::TryGetFromFuture( noData.GetFuture(), untouched ) );
            ASSERT_TRUE( 0 == strcmp( "GUCEF::AUTH::CAuthResult", done.GetCloneableTypeName() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Auth options round trip and the failure reason names" )
        try
        {
            AUTH::CAuthOptions defaults;
            ASSERT_TRUE( defaults.preferBrowser && !defaults.preferDeviceCode && !defaults.forceInteractive && defaults.linkToProfile );
            ASSERT_TRUE( AUTH::CAuthOptions::DefaultTimeoutInMs == defaults.timeoutInMs );

            AUTH::CAuthOptions options;
            options.scopes.push_back( "email" );
            options.loginHint = "alice@example.invalid";
            options.forceInteractive = true;
            options.timeoutInMs = 4500;
            options.preferBrowser = false;
            options.preferDeviceCode = true;
            options.linkToProfile = false;
            CORE::CDataNode node( "options", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( options.Serialize( node, CORE::CDataNodeSerializableSettings() ) );
            AUTH::CAuthOptions back;
            ASSERT_TRUE( back.Deserialize( node, CORE::CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( 1 == back.scopes.size() && "email" == back.scopes[ 0 ] && "alice@example.invalid" == back.loginHint );
            ASSERT_TRUE( back.forceInteractive && 4500 == back.timeoutInMs && !back.preferBrowser && back.preferDeviceCode && !back.linkToProfile );

            ASSERT_TRUE( 0 == strcmp( "cancelled", AUTH::AuthFailureReasonToString( AUTH::AUTHFAIL_CANCELLED ) ) );
            ASSERT_TRUE( 0 == strcmp( "invalid-grant", AUTH::AuthFailureReasonToString( AUTH::AUTHFAIL_INVALID_GRANT ) ) );
            ASSERT_TRUE( 0 == strcmp( "unknown", AUTH::AuthFailureReasonToString( AUTH::AUTHFAIL_UNKNOWN ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Hashing, hex, constant time compare and random bytes" )
        try
        {
            // FIPS 180-4 vectors
            ASSERT_TRUE( "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" == AUTH::CAuthCrypto::Sha256Hex( "abc", 3 ) );
            ASSERT_TRUE( "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" == AUTH::CAuthCrypto::Sha256Hex( "", 0 ) );

            const UInt8 bytes[] = { 0x00, 0x01, 0xAB, 0xFF };
            ASSERT_TRUE( "0001abff" == AUTH::CAuthCrypto::ToHex( bytes, 4 ) );
            CORE::CDynamicBuffer decoded;
            ASSERT_TRUE( AUTH::CAuthCrypto::FromHex( "0001ABff", decoded ) && 4 == decoded.GetDataSize() && 0 == memcmp( decoded.GetConstBufferPtr(), bytes, 4 ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::FromHex( "abc", decoded ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::FromHex( "zz", decoded ) );
            ASSERT_TRUE( AUTH::CAuthCrypto::FromHex( "", decoded ) && 0 == decoded.GetDataSize() );
            UInt8 fixed[ 2 ];
            ASSERT_TRUE( AUTH::CAuthCrypto::FromHexFixed( "beef", fixed, 2 ) && 0xBE == fixed[ 0 ] && 0xEF == fixed[ 1 ] );
            ASSERT_FALSE( AUTH::CAuthCrypto::FromHexFixed( "beef00", fixed, 2 ) );

            ASSERT_TRUE( AUTH::CAuthCrypto::ConstantTimeEquals( "abcd", "abcd", 4 ) && !AUTH::CAuthCrypto::ConstantTimeEquals( "abcd", "abce", 4 ) );

            UInt8 a[ 32 ];
            UInt8 b[ 32 ];
            ASSERT_TRUE( AUTH::CAuthCrypto::RandomBytes( a, 32 ) && AUTH::CAuthCrypto::RandomBytes( b, 32 ) );
            ASSERT_TRUE( 0 != memcmp( a, b, 32 ) );
            AUTH::CAuthCrypto::Wipe( a, 32 );
            UInt8 zero[ 32 ];
            memset( zero, 0, 32 );
            ASSERT_TRUE( 0 == memcmp( a, zero, 32 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Sealing and opening with the AEAD codec, tampering is refused" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsAeadAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            UInt8 key[ 32 ];
            UInt8 nonce[ 24 ];
            ASSERT_TRUE( AUTH::CAuthCrypto::RandomBytes( key, 32 ) && AUTH::CAuthCrypto::RandomBytes( nonce, 24 ) );
            const char plain[] = "refresh token that must stay secret";
            const char header[] = "GAUC1";

            CORE::CDynamicBuffer sealed;
            ASSERT_TRUE( AUTH::CAuthCrypto::AeadSeal( key, nonce, header, 5, plain, (UInt32) sizeof( plain ) - 1, sealed ) );
            ASSERT_TRUE( sizeof( plain ) - 1 + AUTH::CAuthCrypto::TagSize == sealed.GetDataSize() );
            ASSERT_FALSE( BufferContains( sealed, "secret" ) );

            CORE::CDynamicBuffer opened;
            ASSERT_TRUE( AUTH::CAuthCrypto::AeadOpen( key, nonce, header, 5, sealed.GetConstBufferPtr(), sealed.GetDataSize(), opened ) );
            ASSERT_TRUE( sizeof( plain ) - 1 == opened.GetDataSize() && 0 == memcmp( opened.GetConstBufferPtr(), plain, sizeof( plain ) - 1 ) );

            // a changed byte, a changed header, a changed key and a changed nonce are all refused and nothing is output
            CORE::CDynamicBuffer tampered( sealed );
            static_cast< UInt8* >( tampered.GetBufferPtr() )[ 3 ] ^= 0x01;
            CORE::CDynamicBuffer none;
            ASSERT_FALSE( AUTH::CAuthCrypto::AeadOpen( key, nonce, header, 5, tampered.GetConstBufferPtr(), tampered.GetDataSize(), none ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::AeadOpen( key, nonce, "GAUC2", 5, sealed.GetConstBufferPtr(), sealed.GetDataSize(), none ) );
            UInt8 otherKey[ 32 ];
            memcpy( otherKey, key, 32 );
            otherKey[ 0 ] ^= 0x80;
            ASSERT_FALSE( AUTH::CAuthCrypto::AeadOpen( otherKey, nonce, header, 5, sealed.GetConstBufferPtr(), sealed.GetDataSize(), none ) );
            UInt8 otherNonce[ 24 ];
            memcpy( otherNonce, nonce, 24 );
            otherNonce[ 23 ] ^= 0x01;
            ASSERT_FALSE( AUTH::CAuthCrypto::AeadOpen( key, otherNonce, header, 5, sealed.GetConstBufferPtr(), sealed.GetDataSize(), none ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::AeadOpen( key, nonce, header, 5, sealed.GetConstBufferPtr(), 8, none ) );
            ASSERT_TRUE( 0 == none.GetDataSize() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Verifying Ed25519 signatures by public key in hex" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CTestSigner signer;
            ASSERT_TRUE( signer.IsValid() );
            const char message[] = "link this account";
            CORE::CDynamicBuffer signature;
            ASSERT_TRUE( signer.Sign( message, (UInt32) sizeof( message ) - 1, signature ) && 64 == signature.GetDataSize() );
            CORE::CString signatureHex = AUTH::CAuthCrypto::ToHex( signature.GetConstBufferPtr(), 64 );

            ASSERT_TRUE( AUTH::CAuthCrypto::VerifySignature( signer.GetPublicKeyHex(), message, (UInt32) sizeof( message ) - 1, signatureHex ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::VerifySignature( signer.GetPublicKeyHex(), message, (UInt32) sizeof( message ) - 2, signatureHex ) );
            CTestSigner other;
            ASSERT_FALSE( AUTH::CAuthCrypto::VerifySignature( other.GetPublicKeyHex(), message, (UInt32) sizeof( message ) - 1, signatureHex ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::VerifySignature( signer.GetPublicKeyHex(), message, (UInt32) sizeof( message ) - 1, "00" ) );
            ASSERT_FALSE( AUTH::CAuthCrypto::VerifySignature( "zz", message, (UInt32) sizeof( message ) - 1, signatureHex ) );
            CORE::CString flipped = signatureHex;
            flipped.C_String()[ 0 ] = '0' == flipped.C_String()[ 0 ] ? '1' : '0';
            ASSERT_FALSE( AUTH::CAuthCrypto::VerifySignature( signer.GetPublicKeyHex(), message, (UInt32) sizeof( message ) - 1, flipped ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
