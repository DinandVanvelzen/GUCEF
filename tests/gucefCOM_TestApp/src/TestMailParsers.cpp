/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COM_CEMAILSTREAMREADER_H
#include "gucefCOM_CEMailStreamReader.h"
#define GUCEF_COM_CEMAILSTREAMREADER_H
#endif /* GUCEF_COM_CEMAILSTREAMREADER_H ? */

#ifndef GUCEF_COM_CIEMAILSASLMECHANISM_H
#include "gucefCOM_CIEMailSaslMechanism.h"
#define GUCEF_COM_CIEMAILSASLMECHANISM_H
#endif /* GUCEF_COM_CIEMAILSASLMECHANISM_H ? */
#ifndef GUCEF_COM_CEMAILSASLPLAIN_H
#include "gucefCOM_CEMailSaslPlain.h"
#define GUCEF_COM_CEMAILSASLPLAIN_H
#endif /* GUCEF_COM_CEMAILSASLPLAIN_H ? */
#ifndef GUCEF_COM_CEMAILSASLLOGIN_H
#include "gucefCOM_CEMailSaslLogin.h"
#define GUCEF_COM_CEMAILSASLLOGIN_H
#endif /* GUCEF_COM_CEMAILSASLLOGIN_H ? */
#ifndef GUCEF_COM_CEMAILSASLXOAUTH2_H
#include "gucefCOM_CEMailSaslXOAuth2.h"
#define GUCEF_COM_CEMAILSASLXOAUTH2_H
#endif /* GUCEF_COM_CEMAILSASLXOAUTH2_H ? */
#ifndef GUCEF_COM_CEMAILSASLSCRAMSHA256_H
#include "gucefCOM_CEMailSaslScramSha256.h"
#define GUCEF_COM_CEMAILSASLSCRAMSHA256_H
#endif /* GUCEF_COM_CEMAILSASLSCRAMSHA256_H ? */
#ifndef GUCEF_COM_CEMAILSASLFACTORY_H
#include "gucefCOM_CEMailSaslFactory.h"
#define GUCEF_COM_CEMAILSASLFACTORY_H
#endif /* GUCEF_COM_CEMAILSASLFACTORY_H ? */

#ifndef GUCEF_COM_CIMAPRESPONSE_H
#include "gucefCOM_CImapResponse.h"
#define GUCEF_COM_CIMAPRESPONSE_H
#endif /* GUCEF_COM_CIMAPRESPONSE_H ? */

#ifndef GUCEF_COM_CIMAPVALUEPARSER_H
#include "gucefCOM_CImapValueParser.h"
#define GUCEF_COM_CIMAPVALUEPARSER_H
#endif /* GUCEF_COM_CIMAPVALUEPARSER_H ? */

#ifndef GUCEF_COM_CIMAPFETCHEDITEM_H
#include "gucefCOM_CImapFetchedItem.h"
#define GUCEF_COM_CIMAPFETCHEDITEM_H
#endif /* GUCEF_COM_CIMAPFETCHEDITEM_H ? */

#ifndef GUCEF_COM_CIMAPSEARCHCRITERIA_H
#include "gucefCOM_CImapSearchCriteria.h"
#define GUCEF_COM_CIMAPSEARCHCRITERIA_H
#endif /* GUCEF_COM_CIMAPSEARCHCRITERIA_H ? */

#ifndef GUCEF_COM_CSMTPCLIENT_H
#include "gucefCOM_CSmtpClient.h"
#define GUCEF_COM_CSMTPCLIENT_H
#endif /* GUCEF_COM_CSMTPCLIENT_H ? */

#ifndef GUCEF_COM_CSMTPCLIENTSETTINGS_H
#include "gucefCOM_CSmtpClientSettings.h"
#define GUCEF_COM_CSMTPCLIENTSETTINGS_H
#endif /* GUCEF_COM_CSMTPCLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CPOP3CLIENTSETTINGS_H
#include "gucefCOM_CPop3ClientSettings.h"
#define GUCEF_COM_CPOP3CLIENTSETTINGS_H
#endif /* GUCEF_COM_CPOP3CLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CIMAPCLIENTSETTINGS_H
#include "gucefCOM_CImapClientSettings.h"
#define GUCEF_COM_CIMAPCLIENTSETTINGS_H
#endif /* GUCEF_COM_CIMAPCLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CEMAILPROTOCOLLOG_H
#include "gucefCOM_CEMailProtocolLog.h"
#define GUCEF_COM_CEMAILPROTOCOLLOG_H
#endif /* GUCEF_COM_CEMAILPROTOCOLLOG_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestMailParsers.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COM;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
AsText( const CORE::CDynamicBuffer& buffer )
{
    return buffer.GetDataSize() > 0 ? CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

void
FeedText( CEMailStreamReader& reader, const char* text )
{
    reader.Feed( text, (UInt32) strlen( text ) );
}

/*-------------------------------------------------------------------------*/

void
SetBytes( CORE::CDynamicBuffer& buffer, const CORE::CString& text )
{
    buffer.Clear();
    if ( text.ByteSize() > 1 )
        buffer.CopyFrom( text.ByteSize() - 1, text.C_String() );
}

/*-------------------------------------------------------------------------*/

/** Runs a SCRAM exchange against the example of RFC 7677, with the nonce of the example */
bool
RunScramExample( const char* serverFirst, const char* serverFinal, EMailSaslState& state1, EMailSaslState& state2, CORE::CString& clientFirst, CORE::CString& clientFinal, CEMailSaslScramSha256** keepMechanism = GUCEF_NULL )
{
    CEMailSaslScramSha256* mechanism = GUCEF_NEW CEMailSaslScramSha256( "user", "pencil" );
    mechanism->SetClientNonceForTesting( "rOprNGfwEbeRWgbNEkqO" );
    CORE::CDynamicBuffer initial, challenge, response;
    if ( !mechanism->Start( initial ) )
    {
        GUCEF_DELETE mechanism;
        return false;
    }
    clientFirst = AsText( initial );
    SetBytes( challenge, serverFirst );
    state1 = mechanism->Step( challenge, response );
    clientFinal = AsText( response );
    state2 = MAILSASL_FAILED;
    if ( MAILSASL_CONTINUE == state1 )
    {
        SetBytes( challenge, serverFinal );
        state2 = mechanism->Step( challenge, response );
    }
    if ( GUCEF_NULL != keepMechanism )
        *keepMechanism = mechanism;
    else
        GUCEF_DELETE mechanism;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailParserTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MailParsers" );

    GUCEF_TESTFW_TESTCASE( "Mail stream reader: lines across feeds, CR LF and LF, text that is not UTF-8" )
    try
    {
        CEMailStreamReader reader( 100, 1000 );
        CORE::CString line;
        CORE::CDynamicBuffer bytes;
        FeedText( reader, "220 hel" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_NONE == reader.Next( line, bytes ) );
        FeedText( reader, "lo\r\n250 second\nthird\r" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) && line == "220 hello" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) && line == "250 second" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_NONE == reader.Next( line, bytes ) && reader.HasBufferedData() );
        FeedText( reader, "\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) && line == "third" );
        ASSERT_FALSE( reader.HasBufferedData() );

        reader.Feed( "caf\xE9 \0 x\r\n", 10 );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) );
        ASSERT_TRUE( line == "caf\xEF\xBF\xBD \xEF\xBF\xBD x" );

        FeedText( reader, "\r\n\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) && line.IsNULLOrEmpty() );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) && line.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail stream reader: counted blocks, dot terminated blocks and the limits" )
    try
    {
        CEMailStreamReader reader( 50, 20 );
        CORE::CString line;
        CORE::CDynamicBuffer bytes;

        // a literal: the line before, the block, the rest of the line
        FeedText( reader, "* 1 FETCH (BODY[] {5}\r\nab" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) );
        ASSERT_TRUE( reader.ExpectBytes( 5 ) );
        ASSERT_TRUE( CEMailStreamReader::ITEM_NONE == reader.Next( line, bytes ) );
        FeedText( reader, "c\r\nx)\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_BYTES == reader.Next( line, bytes ) && AsText( bytes ) == "abc\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) && line == "x)" );

        // a block of zero bytes
        ASSERT_TRUE( reader.ExpectBytes( 0 ) );
        FeedText( reader, "next\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_BYTES == reader.Next( line, bytes ) && 0 == bytes.GetDataSize() );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) && line == "next" );

        // dot terminated: stuffing is not undone, the terminator may be split over feeds
        reader.ExpectDotTerminatedBytes( 1000 );
        FeedText( reader, "Subject: x\r\n\r\n..dot\r\nline\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_NONE == reader.Next( line, bytes ) );
        FeedText( reader, "." );
        ASSERT_TRUE( CEMailStreamReader::ITEM_NONE == reader.Next( line, bytes ) );
        FeedText( reader, "\r" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_NONE == reader.Next( line, bytes ) );
        FeedText( reader, "\nafter\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_BYTES == reader.Next( line, bytes ) && AsText( bytes ) == "Subject: x\r\n\r\n..dot\r\nline\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_LINE == reader.Next( line, bytes ) && line == "after" );

        // an empty message: the terminator right away; a line that only starts with a dot is no terminator
        reader.ExpectDotTerminatedBytes( 1000 );
        FeedText( reader, ".\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_BYTES == reader.Next( line, bytes ) && 0 == bytes.GetDataSize() );
        reader.ExpectDotTerminatedBytes( 1000 );
        FeedText( reader, ".x\r\n.\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_BYTES == reader.Next( line, bytes ) && AsText( bytes ) == ".x\r\n" );

        // a message that never ends
        reader.ExpectDotTerminatedBytes( 30 );
        FeedText( reader, "0123456789012345678901234567890123456789\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_OVERFLOW == reader.Next( line, bytes ) && reader.HasFailed() );
        reader.Reset();
        ASSERT_FALSE( reader.HasFailed() );

        // a line without end, a line that is too long, a block that is too large
        FeedText( reader, "0123456789012345678901234567890123456789012345678901234567890" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_OVERFLOW == reader.Next( line, bytes ) );
        ASSERT_FALSE( reader.Feed( "x", 1 ) );
        reader.Reset();
        FeedText( reader, "01234567890123456789012345678901234567890123456789012\r\n" );
        ASSERT_TRUE( CEMailStreamReader::ITEM_OVERFLOW == reader.Next( line, bytes ) );
        reader.Reset();
        ASSERT_FALSE( reader.ExpectBytes( 21 ) );
        ASSERT_TRUE( reader.HasFailed() && !reader.GetOverflowReason().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail SASL: PLAIN, LOGIN and XOAUTH2 messages, the failures that make no secret known" )
    try
    {
        CORE::CDynamicBuffer initial, challenge, response;

        CEMailSaslPlain plain( "alice", "pw" );
        ASSERT_TRUE( plain.HasInitialResponse() && plain.RevealsSecret() && plain.Start( initial ) );
        ASSERT_TRUE( 9 == initial.GetDataSize() && 0 == memcmp( initial.GetConstBufferPtr(), "\0alice\0pw", 9 ) );
        CEMailSaslPlain noUser( "", "pw" );
        ASSERT_FALSE( noUser.Start( initial ) );

        CEMailSaslLogin login( "alice", "pw" );
        ASSERT_FALSE( login.HasInitialResponse() );
        ASSERT_TRUE( login.Start( initial ) );
        SetBytes( challenge, "Username:" );
        ASSERT_TRUE( MAILSASL_CONTINUE == login.Step( challenge, response ) && AsText( response ) == "alice" );
        SetBytes( challenge, "Password:" );
        ASSERT_TRUE( MAILSASL_DONE == login.Step( challenge, response ) && AsText( response ) == "pw" );
        ASSERT_TRUE( MAILSASL_FAILED == login.Step( challenge, response ) );

        CEMailSaslXOAuth2 xoauth( "alice@example.org", "ya29.token" );
        ASSERT_TRUE( xoauth.Start( initial ) );
        ASSERT_TRUE( AsText( initial ) == "user=alice@example.org\x01" "auth=Bearer ya29.token\x01\x01" );
        SetBytes( challenge, "{\"status\":\"401\"}" );
        ASSERT_TRUE( MAILSASL_FAILED == xoauth.Step( challenge, response ) && 0 == response.GetDataSize() );
        CEMailSaslXOAuth2 injected( "alice", "tok\x01" "auth=Bearer other" );
        ASSERT_FALSE( injected.Start( initial ) );
        CEMailSaslXOAuth2 noToken( "alice", "" );
        ASSERT_FALSE( noToken.Start( initial ) );

        CORE::CString::StringVector supported;
        CEMailSaslFactory::GetSupportedMechanisms( supported );
        ASSERT_TRUE( 4 == supported.size() );
        CIEMailSaslMechanism* made = CEMailSaslFactory::Create( "scram-sha-256", "u", "p" );
        ASSERT_TRUE( GUCEF_NULL != made && made->GetName() == "SCRAM-SHA-256" && !made->RevealsSecret() );
        GUCEF_DELETE made;
        ASSERT_TRUE( GUCEF_NULL == CEMailSaslFactory::Create( "CRAM-MD5", "u", "p" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail SASL: SCRAM-SHA-256 with the example of RFC 7677, PBKDF2 vectors, and a server that cannot be trusted" )
    try
    {
        // PBKDF2-HMAC-SHA256
        UInt8 key[ 32 ];
        CEMailSaslScramSha256::Pbkdf2HmacSha256( "password", 8, "salt", 4, 1, key );
        ASSERT_TRUE( CORE::Base16Encode( key, 32 ).Lowercase() == "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b" );
        CEMailSaslScramSha256::Pbkdf2HmacSha256( "password", 8, "salt", 4, 2, key );
        ASSERT_TRUE( CORE::Base16Encode( key, 32 ).Lowercase() == "ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43" );
        CEMailSaslScramSha256::Pbkdf2HmacSha256( "password", 8, "salt", 4, 4096, key );
        ASSERT_TRUE( CORE::Base16Encode( key, 32 ).Lowercase() == "c5e478d59288c841aa530db6845c4c8d962893a001ce4e11a4963873aa98134a" );

        const char* serverFirst = "r=rOprNGfwEbeRWgbNEkqO%hvYDpWUa2RaTCAfuxFIlj)hNlF$k0,s=W22ZaJ0SNY7soEsUEjb6gQ==,i=4096";
        EMailSaslState s1, s2;
        CORE::CString clientFirst, clientFinal;
        ASSERT_TRUE( RunScramExample( serverFirst, "v=6rriTRBi23WpRR/wtup+mMhUZUn/dB5nLTJRsjl95G4=", s1, s2, clientFirst, clientFinal ) );
        ASSERT_TRUE( clientFirst == "n,,n=user,r=rOprNGfwEbeRWgbNEkqO" );
        ASSERT_TRUE( MAILSASL_CONTINUE == s1 );
        ASSERT_TRUE( clientFinal == "c=biws,r=rOprNGfwEbeRWgbNEkqO%hvYDpWUa2RaTCAfuxFIlj)hNlF$k0,p=dHzbZapWIk4jUhN+Ute9ytag9zjfMHgsqmmiz7AndVQ=" );
        ASSERT_TRUE( MAILSASL_DONE == s2 );

        // a server that does not know the password signs wrongly
        ASSERT_TRUE( RunScramExample( serverFirst, "v=AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=", s1, s2, clientFirst, clientFinal ) );
        ASSERT_TRUE( MAILSASL_FAILED == s2 );
        ASSERT_TRUE( RunScramExample( serverFirst, "e=invalid-proof", s1, s2, clientFirst, clientFinal ) );
        ASSERT_TRUE( MAILSASL_FAILED == s2 );

        // a nonce that does not start with ours, no extension of the nonce, bad iteration counts, a salt that is no base64, a mandatory extension
        ASSERT_TRUE( RunScramExample( "r=otherNONCEabc,s=W22ZaJ0SNY7soEsUEjb6gQ==,i=4096", "", s1, s2, clientFirst, clientFinal ) && MAILSASL_FAILED == s1 );
        ASSERT_TRUE( RunScramExample( "r=rOprNGfwEbeRWgbNEkqO,s=W22ZaJ0SNY7soEsUEjb6gQ==,i=4096", "", s1, s2, clientFirst, clientFinal ) && MAILSASL_FAILED == s1 );
        ASSERT_TRUE( RunScramExample( "r=rOprNGfwEbeRWgbNEkqOxyz,s=W22ZaJ0SNY7soEsUEjb6gQ==,i=100", "", s1, s2, clientFirst, clientFinal ) && MAILSASL_FAILED == s1 );
        ASSERT_TRUE( RunScramExample( "r=rOprNGfwEbeRWgbNEkqOxyz,s=W22ZaJ0SNY7soEsUEjb6gQ==,i=99999999", "", s1, s2, clientFirst, clientFinal ) && MAILSASL_FAILED == s1 );
        ASSERT_TRUE( RunScramExample( "r=rOprNGfwEbeRWgbNEkqOxyz,s=!!!,i=4096", "", s1, s2, clientFirst, clientFinal ) && MAILSASL_FAILED == s1 );
        ASSERT_TRUE( RunScramExample( "m=ext,r=rOprNGfwEbeRWgbNEkqOxyz,s=W22ZaJ0SNY7soEsUEjb6gQ==,i=4096", "", s1, s2, clientFirst, clientFinal ) && MAILSASL_FAILED == s1 );

        // a user name with = and , is escaped
        CEMailSaslScramSha256 odd( "a=b,c", "pw" );
        CORE::CDynamicBuffer initial;
        ASSERT_TRUE( odd.Start( initial ) && AsText( initial ).HasSubstr( "n=a=3Db=2Cc," ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "IMAP tokenizer: atoms, numbers, strings, literals, lists and sections, and input that is hostile" )
    try
    {
        {
            const char* text = "(\\Seen \\Answered) NIL 42 \"quoted \\\"text\\\" \\\\\" {5}\r\nab\r\nc BODY[HEADER.FIELDS (FROM TO)]<0> nil";
            CImapValueParser parser( text, (UInt32) strlen( text ) );
            CImapValue list, value;
            ASSERT_TRUE( parser.ParseValue( list ) && list.IsList() && 2 == list.GetListSize() && list.GetElement( 0 ).GetString() == "\\Seen" );
            ASSERT_TRUE( parser.ParseValue( value ) && value.IsNil() );
            ASSERT_TRUE( parser.ParseValue( value ) && value.IsNumber() && 42 == value.GetNumber() );
            ASSERT_TRUE( parser.ParseValue( value ) && value.IsString() && value.GetString() == "quoted \"text\" \\" );
            ASSERT_TRUE( parser.ParseValue( value ) && value.IsString() && value.GetString() == "ab\r\nc" && 5 == value.GetBytes().GetDataSize() );
            ASSERT_TRUE( parser.ParseValue( value ) && value.IsAtom() && value.GetString() == "BODY[HEADER.FIELDS (FROM TO)]<0>" );
            ASSERT_TRUE( parser.ParseValue( value ) && value.IsNil() && parser.AtEnd() );
            ASSERT_FALSE( parser.ParseValue( value ) );
        }

        // literals are checked against the data that came with them, whatever they claim
        const char* lies[] = { "{4294967295}\r\nabc", "{99999999999999999999}\r\nabc", "{10}\r\nabc", "{3}abc", "{}\r\n", "{3", "{3+", "(unclosed", "\"unclosed", "\"line\r\nbreak\"", ")", "BODY[unclosed" };
        for ( size_t i=0; i<sizeof( lies ) / sizeof( lies[ 0 ] ); ++i )
        {
            CImapValueParser parser( lies[ i ], (UInt32) strlen( lies[ i ] ) );
            CImapValue value;
            ASSERT_FALSE( parser.ParseValue( value ) );
        }

        // nesting
        CORE::CString deep;
        for ( int i=0; i<40; ++i ) deep += "(";
        CImapValueParser deepParser( deep.C_String(), deep.ByteSize() - 1 );
        CImapValue deepValue;
        ASSERT_FALSE( deepParser.ParseValue( deepValue ) );
        CORE::CString ok;
        for ( int i=0; i<30; ++i ) ok += "(";
        for ( int i=0; i<30; ++i ) ok += ")";
        CImapValueParser okParser( ok.C_String(), ok.ByteSize() - 1 );
        ASSERT_TRUE( okParser.ParseValue( deepValue ) );

        // a limit on the number of values
        CORE::CString many = "(";
        for ( int i=0; i<2000; ++i ) many += "1 ";
        many += ")";
        CImapValueParser limited( many.C_String(), many.ByteSize() - 1, 32, 100 );
        ASSERT_FALSE( limited.ParseValue( deepValue ) );

        // noise
        UInt32 state = 12345;
        for ( int round=0; round<300; ++round )
        {
            char noise[ 200 ];
            for ( int i=0; i<200; ++i )
            {
                state = state * 1103515245u + 12345u;
                const char alphabet[] = "(){}[]\"\\ \r\n+0123456789abcNIL<>.";
                noise[ i ] = alphabet[ ( state >> 16 ) % ( sizeof( alphabet ) - 1 ) ];
            }
            CImapValueParser parser( noise, 200 );
            CImapValue list;
            parser.ParseValuesUntilEnd( list );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "IMAP responses: tagged, untagged, status with codes, continuation, FETCH with envelope and body structure" )
    try
    {
        CImapResponse response;
        CORE::CString error;
        const UInt32 maxValues = 100000;

        CORE::CString text = "A0001 OK [READ-WRITE] SELECT completed";
        ASSERT_TRUE( CImapResponse::Parse( text.C_String(), text.ByteSize() - 1, maxValues, response, error ) );
        ASSERT_TRUE( CImapResponse::KIND_TAGGED == response.kind && response.tag == "A0001" && response.status == "OK" && response.code == "READ-WRITE" && response.text == "SELECT completed" );

        text = "* OK [UIDVALIDITY 3857529045] UIDs valid";
        ASSERT_TRUE( CImapResponse::Parse( text.C_String(), text.ByteSize() - 1, maxValues, response, error ) );
        ASSERT_TRUE( response.isStatus && response.code == "UIDVALIDITY" && response.codeArguments == "3857529045" );

        text = "* OK [PERMANENTFLAGS (\\Deleted \\Seen \\*)] Limited";
        ASSERT_TRUE( CImapResponse::Parse( text.C_String(), text.ByteSize() - 1, maxValues, response, error ) );
        ASSERT_TRUE( response.code == "PERMANENTFLAGS" && response.codeArguments == "(\\Deleted \\Seen \\*)" );

        text = "* 172 EXISTS";
        ASSERT_TRUE( CImapResponse::Parse( text.C_String(), text.ByteSize() - 1, maxValues, response, error ) );
        ASSERT_TRUE( response.hasNumber && 172 == response.number && response.keyword == "EXISTS" && !response.isStatus );

        text = "* LIST (\\Noselect) \"/\" \"Entw&APw-rfe\"";
        ASSERT_TRUE( CImapResponse::Parse( text.C_String(), text.ByteSize() - 1, maxValues, response, error ) );
        ASSERT_TRUE( response.keyword == "LIST" && 3 == response.data.GetListSize() && response.data.GetElement( 2 ).GetString() == "Entw&APw-rfe" );

        text = "* CAPABILITY IMAP4rev1 STARTTLS AUTH=PLAIN";
        ASSERT_TRUE( CImapResponse::Parse( text.C_String(), text.ByteSize() - 1, maxValues, response, error ) );
        ASSERT_TRUE( 3 == response.data.GetListSize() && response.data.GetElement( 2 ).GetString() == "AUTH=PLAIN" );

        text = "+ idling";
        ASSERT_TRUE( CImapResponse::Parse( text.C_String(), text.ByteSize() - 1, maxValues, response, error ) );
        ASSERT_TRUE( CImapResponse::KIND_CONTINUATION == response.kind && response.text == "idling" );

        // the example of RFC 3501 section 7.4.2
        text = "* 12 FETCH (FLAGS (\\Seen) UID 4827 INTERNALDATE \"17-Jul-1996 02:44:25 -0700\" RFC822.SIZE 4286 "
               "ENVELOPE (\"Wed, 17 Jul 1996 02:23:25 -0700 (PDT)\" \"IMAP4rev1 WG mtg summary and minutes\" "
               "((\"Terry Gray\" NIL \"gray\" \"cac.washington.edu\")) ((\"Terry Gray\" NIL \"gray\" \"cac.washington.edu\")) "
               "((\"Terry Gray\" NIL \"gray\" \"cac.washington.edu\")) ((NIL NIL \"imap\" \"cac.washington.edu\")) "
               "((NIL NIL \"minutes\" \"CNRI.Reston.VA.US\") (\"John Klensin\" NIL \"KLENSIN\" \"MIT.EDU\")) NIL NIL "
               "\"<B27397-0100000@cac.washington.edu>\") "
               "BODYSTRUCTURE ((\"TEXT\" \"PLAIN\" (\"CHARSET\" \"US-ASCII\") NIL NIL \"7BIT\" 3028 92)"
               "(\"APPLICATION\" \"OCTET-STREAM\" (\"NAME\" \"turn.rdx\") NIL NIL \"BASE64\" 4000 NIL (\"ATTACHMENT\" (\"FILENAME\" \"turn.rdx\"))) \"MIXED\"))";
        ASSERT_TRUE( CImapResponse::Parse( text.C_String(), text.ByteSize() - 1, maxValues, response, error ) );
        CImapFetchedItem item;
        ASSERT_TRUE( CImapFetchedItem::FromResponse( response, item ) );
        ASSERT_TRUE( 12 == item.sequence && item.hasUid && 4827 == item.uid && item.HasFlag( "\\seen" ) && item.hasSize && 4286 == item.size );
        ASSERT_TRUE( item.internalDate == "17-Jul-1996 02:44:25 -0700" );
        ASSERT_TRUE( item.hasEnvelope && item.envelope.subject == "IMAP4rev1 WG mtg summary and minutes" && 1 == item.envelope.from.size() );
        ASSERT_TRUE( item.envelope.from[ 0 ].GetAddress() == "gray@cac.washington.edu" && item.envelope.from[ 0 ].displayName == "Terry Gray" );
        ASSERT_TRUE( 2 == item.envelope.cc.size() && item.envelope.messageId == "<B27397-0100000@cac.washington.edu>" );
        ASSERT_TRUE( item.hasBodyStructure && item.bodyStructure.isMultipart && item.bodyStructure.subtype == "mixed" && 2 == item.bodyStructure.children.size() );
        ASSERT_TRUE( item.bodyStructure.children[ 0 ]->GetContentType() == "text/plain" && 92 == item.bodyStructure.children[ 0 ]->lines );
        ASSERT_TRUE( item.bodyStructure.children[ 1 ]->GetFileName() == "turn.rdx" && item.bodyStructure.children[ 1 ]->disposition == "attachment" && 4000 == item.bodyStructure.children[ 1 ]->size );
        CImapBodyStructure copy = item.bodyStructure;
        ASSERT_TRUE( 2 == copy.children.size() && copy.children[ 1 ]->GetFileName() == "turn.rdx" );

        // sections with literals
        CORE::CDynamicBuffer wire;
        const CORE::CString withLiteral = "* 1 FETCH (UID 7 BODY[HEADER] {13}\r\nSubject: hi\r\n BODY[TEXT]<0> \"abc\")";
        CImapResponse literalResponse;
        ASSERT_TRUE( CImapResponse::Parse( withLiteral.C_String(), withLiteral.ByteSize() - 1, maxValues, literalResponse, error ) );
        CImapFetchedItem literalItem;
        ASSERT_TRUE( CImapFetchedItem::FromResponse( literalResponse, literalItem ) );
        ASSERT_TRUE( 2 == literalItem.sections.size() );
        ASSERT_TRUE( GUCEF_NULL != literalItem.FindSection( "HEADER" ) && AsText( literalItem.FindSection( "HEADER" )->data ) == "Subject: hi\r\n" );
        ASSERT_TRUE( GUCEF_NULL != literalItem.FindSection( "TEXT" ) && literalItem.FindSection( "TEXT" )->hasOrigin && AsText( literalItem.FindSection( "TEXT" )->data ) == "abc" );

        // what is not a response
        const char* bad[] = { "", "A0001", "A0001 MAYBE fine", "* ", "<html>", "* 99999999999 EXISTS", "* 3" };
        for ( size_t i=0; i<sizeof( bad ) / sizeof( bad[ 0 ] ); ++i )
            ASSERT_FALSE( CImapResponse::Parse( bad[ i ], (UInt32) strlen( bad[ i ] ), maxValues, response, error ) );

        // literal announcements at the end of a line
        UInt32 size = 0;
        bool plus = false;
        ASSERT_TRUE( CImapResponse::EndsWithLiteral( "* 1 FETCH (BODY[] {342}", size, plus ) && 342 == size && !plus );
        ASSERT_TRUE( CImapResponse::EndsWithLiteral( "A1 APPEND INBOX {10+}", size, plus ) && 10 == size && plus );
        ASSERT_FALSE( CImapResponse::EndsWithLiteral( "* OK text {abc}", size, plus ) );
        ASSERT_FALSE( CImapResponse::EndsWithLiteral( "{}", size, plus ) );
        ASSERT_FALSE( CImapResponse::EndsWithLiteral( "text", size, plus ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "IMAP mailbox names: modified UTF-7 both ways" )
    try
    {
        CORE::CString wire, text;
        ASSERT_TRUE( CImapMailboxName::Encode( "Entw\xC3\xBC" "rfe", wire ) && wire == "Entw&APw-rfe" );
        ASSERT_TRUE( CImapMailboxName::Decode( wire, text ) && text == "Entw\xC3\xBC" "rfe" );
        ASSERT_TRUE( CImapMailboxName::Encode( "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E", wire ) && wire == "&ZeVnLIqe-" );
        ASSERT_TRUE( CImapMailboxName::Decode( "&ZeVnLIqe-", text ) && text == "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E" );
        ASSERT_TRUE( CImapMailboxName::Encode( "R&D", wire ) && wire == "R&-D" );
        ASSERT_TRUE( CImapMailboxName::Decode( "R&-D", text ) && text == "R&D" );
        ASSERT_TRUE( CImapMailboxName::Encode( "A\xF0\x9F\x98\x80" "B", wire ) && CImapMailboxName::Decode( wire, text ) && text == "A\xF0\x9F\x98\x80" "B" );
        ASSERT_TRUE( CImapMailboxName::Encode( "INBOX.Sent Items", wire ) && wire == "INBOX.Sent Items" );
        ASSERT_FALSE( CImapMailboxName::Encode( "bad\r\nname", wire ) );
        ASSERT_FALSE( CImapMailboxName::Decode( "&AAA", text ) );
        ASSERT_FALSE( CImapMailboxName::Decode( "&!!-", text ) );
        ASSERT_FALSE( CImapMailboxName::Decode( "&A-", text ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "IMAP commands: arguments are quoted, sent as literals or refused, never pasted in" )
    try
    {
        CImapCommandParts parts;
        parts.AddWord( "SELECT " );
        ASSERT_TRUE( parts.AddMailbox( "My \"box\" \\ x" ) );
        ASSERT_TRUE( parts.ToLogText() == "SELECT \"My \\\"box\\\" \\\\ x\"" && !parts.HasLiterals() );

        CImapCommandParts bad;
        ASSERT_FALSE( bad.AddString( "a\r\nA1 DELETE INBOX" ) );
        ASSERT_FALSE( bad.AddString( "a\nb" ) );
        ASSERT_FALSE( bad.AddMailbox( "x\r\n" ) );
        ASSERT_TRUE( bad.texts.size() == 1 );

        CImapCommandParts literal;
        literal.AddWord( "SEARCH SUBJECT " );
        ASSERT_TRUE( literal.AddString( "caf\xC3\xA9" ) );
        ASSERT_TRUE( literal.HasLiterals() && 1 == literal.literals.size() && 5 == literal.literals[ 0 ].GetDataSize() && literal.texts[ 0 ] == "SEARCH SUBJECT {5}" && literal.texts[ 1 ].IsNULLOrEmpty() );

        ASSERT_TRUE( CImapCommandParts::IsValidSequenceSet( "1:5,9,12:*" ) );
        ASSERT_FALSE( CImapCommandParts::IsValidSequenceSet( "1:5 DELETE" ) );
        ASSERT_FALSE( CImapCommandParts::IsValidSequenceSet( "" ) );
        ASSERT_FALSE( CImapCommandParts::IsValidSequenceSet( "1\r\n" ) );

        CImapSearchCriteria criteria;
        CImapCommandParts built;
        CORE::CString error;
        ASSERT_TRUE( criteria.Build( built, error ) && built.ToLogText() == "ALL" );
        criteria.AddUnseen();
        criteria.AddUidRange( 15, 0 );
        criteria.AddHeader( "X-Redux-Game", "g-1234" );
        criteria.AddSince( 2020, 11, 2 );
        ASSERT_TRUE( criteria.Build( built, error ) );
        ASSERT_TRUE( built.ToLogText() == "UNSEEN UID 15:* HEADER \"X-Redux-Game\" \"g-1234\" SINCE 2-Nov-2020" );
        CImapSearchCriteria unicode;
        unicode.AddSubject( "r\xC3\xA9sum\xC3\xA9" );
        ASSERT_TRUE( unicode.Build( built, error ) && built.texts[ 0 ].StartsWith( "CHARSET UTF-8 SUBJECT {" ) && 1 == built.literals.size() );
        CImapSearchCriteria evil;
        evil.AddHeader( "X-A\r\nB", "v" );
        ASSERT_FALSE( evil.Build( built, error ) );
        CImapSearchCriteria evilText;
        evilText.AddSubject( "x\r\nA2 LOGOUT" );
        ASSERT_FALSE( evilText.Build( built, error ) );
        CImapSearchCriteria evilDate;
        evilDate.AddSince( 2020, 13, 1 );
        ASSERT_FALSE( evilDate.Build( built, error ) );

        CImapFetchSpec spec;
        CORE::CString items;
        ASSERT_TRUE( spec.Build( items ) && items == "UID" );
        spec.uid = false;
        ASSERT_FALSE( spec.Build( items ) );
        spec.uid = true;
        spec.flags = true;
        spec.sections.push_back( "HEADER.FIELDS (FROM TO)" );
        spec.sections.push_back( "" );
        spec.partialOffset = 0;
        spec.partialLength = 1024;
        ASSERT_TRUE( spec.Build( items ) && items == "UID FLAGS BODY.PEEK[HEADER.FIELDS (FROM TO)]<0.1024> BODY.PEEK[]" );
        spec.markSeen = true;
        ASSERT_TRUE( spec.Build( items ) && items.HasSubstr( "BODY[]" ) >= 0 && items.HasSubstr( "PEEK" ) < 0 );
        spec.sections.push_back( "1]\r\nA3 DELETE" );
        ASSERT_FALSE( spec.Build( items ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SMTP replies and message data: the reply parser, enhanced codes, dot stuffing and the end of lines" )
    try
    {
        UInt32 code = 0;
        bool last = false;
        CORE::CString text;
        ASSERT_TRUE( CSmtpReply::ParseLine( "250-PIPELINING", code, last, text ) && 250 == code && !last && text == "PIPELINING" );
        ASSERT_TRUE( CSmtpReply::ParseLine( "250 OK", code, last, text ) && last );
        ASSERT_TRUE( CSmtpReply::ParseLine( "354", code, last, text ) && 354 == code && last && text.IsNULLOrEmpty() );
        ASSERT_FALSE( CSmtpReply::ParseLine( "25", code, last, text ) );
        ASSERT_FALSE( CSmtpReply::ParseLine( "250OK", code, last, text ) );
        ASSERT_FALSE( CSmtpReply::ParseLine( "abc def", code, last, text ) );
        ASSERT_FALSE( CSmtpReply::ParseLine( "999 nope", code, last, text ) );
        ASSERT_FALSE( CSmtpReply::ParseLine( "150 nope", code, last, text ) );

        CORE::CString enhanced, rest;
        ASSERT_TRUE( CSmtpReply::SplitEnhancedCode( "5.1.1 No such user", 550, enhanced, rest ) && enhanced == "5.1.1" && rest == "No such user" );
        ASSERT_TRUE( CSmtpReply::SplitEnhancedCode( "2.0.0", 250, enhanced, rest ) && rest.IsNULLOrEmpty() );
        ASSERT_FALSE( CSmtpReply::SplitEnhancedCode( "5.1.1 mismatch", 250, enhanced, rest ) );
        ASSERT_FALSE( CSmtpReply::SplitEnhancedCode( "2.x.0 no", 250, enhanced, rest ) );
        ASSERT_TRUE( MAILERR_TEMPORARY == CSmtpReply::GetErrorClass( 451 ) && MAILERR_PERMANENT == CSmtpReply::GetErrorClass( 554 ) && MAILERR_NONE == CSmtpReply::GetErrorClass( 250 ) );

        const char* source = "Subject: x\r\n\r\n.hidden\r\n..two\nbare lf\rbare cr\r\n.\r\nafter dot line\r\nno end";
        CORE::CDynamicBuffer stuffed;
        CSmtpClient::PrepareData( source, (UInt32) strlen( source ), stuffed );
        ASSERT_TRUE( AsText( stuffed ) == "Subject: x\r\n\r\n..hidden\r\n...two\r\nbare lf\r\nbare cr\r\n..\r\nafter dot line\r\nno end\r\n.\r\n" );
        CSmtpClient::PrepareData( "", 0, stuffed );
        ASSERT_TRUE( AsText( stuffed ) == ".\r\n" );
        CSmtpClient::PrepareData( "a\r\n", 3, stuffed );
        ASSERT_TRUE( AsText( stuffed ) == "a\r\n.\r\n" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail settings: defaults per protocol and TLS mode, configuration round trip without secrets, invalid modes" )
    try
    {
        CSmtpClientSettings smtp;
        ASSERT_TRUE( 587 == smtp.GetEffectivePort() && MAILTLS_STARTTLS == smtp.tlsMode );
        smtp.tlsMode = MAILTLS_IMPLICIT;
        ASSERT_TRUE( 465 == smtp.GetEffectivePort() );
        smtp.tlsMode = MAILTLS_NONE;
        ASSERT_TRUE( 25 == smtp.GetEffectivePort() );
        smtp.port = 2525;
        ASSERT_TRUE( 2525 == smtp.GetEffectivePort() );

        CPop3ClientSettings pop3;
        ASSERT_TRUE( 110 == pop3.GetEffectivePort() );
        pop3.tlsMode = MAILTLS_IMPLICIT;
        ASSERT_TRUE( 995 == pop3.GetEffectivePort() );
        CImapClientSettings imap;
        ASSERT_TRUE( 143 == imap.GetEffectivePort() && 65536 == imap.maxLineLength );
        imap.tlsMode = MAILTLS_IMPLICIT;
        ASSERT_TRUE( 993 == imap.GetEffectivePort() );

        CSmtpClientSettings original;
        original.host = "smtp.example.org";
        original.tlsMode = MAILTLS_IMPLICIT;
        original.userName = "alice";
        original.password = "TOPSECRET-password";
        original.passwordIsToken = true;
        original.authMechanisms.push_back( "XOAUTH2" );
        original.authMechanisms.push_back( "PLAIN" );
        original.heloName = "client.example.org";
        original.usePipelining = false;
        original.failIfAnyRecipientRejected = true;
        original.maxLineLength = 4096;
        original.commandTimeoutInMs = 1234;
        original.tls.SetCaBundlePath( "/etc/ca.pem" );

        CORE::CDataNode config;
        ASSERT_TRUE( original.SaveConfig( config ) );
        ASSERT_TRUE( config.GetAttributeValueOrChildValueByName( "password" ).IsNULLOrEmpty() );
        ASSERT_TRUE( config.GetAttributeValueOrChildValueByName( "tlsMode" ).AsString( "", true ) == "implicit" );

        CSmtpClientSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( loaded.host == "smtp.example.org" && MAILTLS_IMPLICIT == loaded.tlsMode && loaded.userName == "alice" && loaded.password.IsNULLOrEmpty() );
        ASSERT_TRUE( loaded.passwordIsToken && 2 == loaded.authMechanisms.size() && loaded.authMechanisms[ 1 ] == "PLAIN" );
        ASSERT_TRUE( loaded.heloName == "client.example.org" && !loaded.usePipelining && loaded.failIfAnyRecipientRejected && 4096 == loaded.maxLineLength && 1234 == loaded.commandTimeoutInMs );
        ASSERT_TRUE( loaded.tls.GetCaBundlePath() == "/etc/ca.pem" && 465 == loaded.GetEffectivePort() );

        original.saveSecrets = true;
        CORE::CDataNode withSecret;
        ASSERT_TRUE( original.SaveConfig( withSecret ) );
        CSmtpClientSettings withPassword;
        ASSERT_TRUE( withPassword.LoadConfig( withSecret ) && withPassword.password == "TOPSECRET-password" );

        CSmtpClientSettings copy( original );
        ASSERT_TRUE( copy.password == original.password && copy.heloName == original.heloName && copy.tls.GetCaBundlePath() == "/etc/ca.pem" );

        // a mode that does not exist, and no host
        CORE::CDataNode wrongMode;
        wrongMode.SetAttribute( "host", "x.example.org" );
        wrongMode.SetAttribute( "tlsMode", "maybe" );
        CPop3ClientSettings wrong;
        ASSERT_FALSE( wrong.LoadConfig( wrongMode ) );
        CORE::CDataNode noHost;
        CImapClientSettings none;
        ASSERT_FALSE( none.LoadConfig( noHost ) );

        // the default list of mechanisms
        CORE::CString::StringVector mechanisms;
        CSmtpClientSettings defaults;
        defaults.GetPreferredMechanisms( mechanisms );
        ASSERT_TRUE( 3 == mechanisms.size() && mechanisms[ 0 ] == "SCRAM-SHA-256" );
        defaults.passwordIsToken = true;
        defaults.GetPreferredMechanisms( mechanisms );
        ASSERT_TRUE( 1 == mechanisms.size() && mechanisms[ 0 ] == "XOAUTH2" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail protocol log: credentials, tokens and the lines of an authentication exchange are never shown" )
    try
    {
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "AUTH PLAIN AGFsaWNlAHBhc3N3b3Jk", false ) == "AUTH PLAIN ***" );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "AUTH LOGIN", false ) == "AUTH LOGIN ***" );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "A0002 AUTHENTICATE XOAUTH2 dXNlcj1hbGljZQ==", false ) == "A0002 AUTHENTICATE XOAUTH2 ***" );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "A0001 LOGIN alice hunter2", false ) == "A0001 LOGIN ***" );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "PASS hunter2", false ) == "PASS ***" );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "USER alice", false ) == "USER ***" );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "APOP alice c4c9334bac560ecc979e58001b3e22fb", false ) == "APOP ***" );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "dXNlcj1hbGljZQ==", true ) == "***" );
        ASSERT_TRUE( CEMailProtocolLog::RedactServerLine( "dGhlIGNoYWxsZW5nZQ==", true ) == "***" );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "EHLO client.example.org", false ) == "EHLO client.example.org" );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "MAIL FROM:<bob@example.org>", false ).HasSubstr( "bob@example.org" ) < 0 );
        ASSERT_TRUE( CEMailProtocolLog::RedactServerLine( "250 2.1.5 Ok <carol@example.org>", false ).HasSubstr( "carol@example.org" ) < 0 );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( "Authorization: Bearer abcdefghijklmnopqrstuvwxyz0123456789", false ).HasSubstr( "abcdefghij" ) < 0 );
        ASSERT_TRUE( CEMailProtocolLog::RedactClientLine( CORE::CString( "x" ) + "\r\n" + "y", false ).HasSubstr( "\r" ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
