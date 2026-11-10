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

#ifndef GUCEF_AUTH_CACCOUNTREF_H
#include "gucefAUTH_CAccountRef.h"
#define GUCEF_AUTH_CACCOUNTREF_H
#endif /* GUCEF_AUTH_CACCOUNTREF_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAccountRef.h"

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
PerformAccountRefTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING account reference TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "AccountRef" );

    GUCEF_TESTFW_TESTCASE( "Test: Format and parse provider:subject, the provider in lower case" )
        try
        {
            AUTH::CAccountRef epic( "Epic", "76561198000000000" );
            ASSERT_TRUE( epic.IsValid() && !epic.IsEmpty() );
            ASSERT_TRUE( "epic" == epic.GetProvider() && "76561198000000000" == epic.GetSubject() );
            ASSERT_TRUE( "epic:76561198000000000" == epic.ToString() );

            AUTH::CAccountRef parsed;
            ASSERT_TRUE( AUTH::CAccountRef::TryParse( "GOOGLE:1128394650123", parsed ) );
            ASSERT_TRUE( "google:1128394650123" == parsed.ToString() );
            // The subject is case sensitive
            ASSERT_TRUE( AUTH::CAccountRef::TryParse( "epic:0A1b2C3d", parsed ) && "0A1b2C3d" == parsed.GetSubject() );

            AUTH::CAccountRef empty;
            ASSERT_TRUE( empty.IsEmpty() && !empty.IsValid() && empty.ToString().IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A subject may contain colons, the text is split at the first one" )
        try
        {
            AUTH::CAccountRef cognito;
            ASSERT_TRUE( AUTH::CAccountRef::TryParse( "aws:eu-west-1_AbCdEfGhI:2f6c9d1e-5a3b-4c7d-8e9f-0a1b2c3d4e5f", cognito ) );
            ASSERT_TRUE( "aws" == cognito.GetProvider() );
            ASSERT_TRUE( "eu-west-1_AbCdEfGhI:2f6c9d1e-5a3b-4c7d-8e9f-0a1b2c3d4e5f" == cognito.GetSubject() );
            ASSERT_TRUE( "aws:eu-west-1_AbCdEfGhI:2f6c9d1e-5a3b-4c7d-8e9f-0a1b2c3d4e5f" == cognito.ToString() );

            AUTH::CAccountRef microsoft( "microsoft", "9188040d-6c67-4c5b-b112-36a304b66dad:0000000000000000-1111" );
            ASSERT_TRUE( microsoft.IsValid() );
            AUTH::CAccountRef again;
            ASSERT_TRUE( AUTH::CAccountRef::TryParse( microsoft.ToString(), again ) && again == microsoft );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Invalid references are refused and come out empty" )
        try
        {
            AUTH::CAccountRef ref;
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "", ref ) );
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "epic", ref ) );
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( ":123", ref ) );
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "epic:", ref ) );
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "1epic:123", ref ) );
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "ste am:123", ref ) );
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "epic:12 3", ref ) );
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "epic:12\n3", ref ) );
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "epic:12\"3", ref ) );
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "epic:12/3", ref ) );
            ASSERT_TRUE( ref.IsEmpty() );

            ASSERT_FALSE( AUTH::CAccountRef( "epic", "" ).IsValid() );
            ASSERT_FALSE( AUTH::CAccountRef( "", "123" ).IsValid() );
            ASSERT_FALSE( AUTH::CAccountRef( "st_eam", "123" ).IsValid() );

            // limits of the provider and the subject
            CORE::CString provider32;
            for ( int i=0; i<32; ++i )
                provider32 += 'a';
            ASSERT_TRUE( AUTH::CAccountRef( provider32, "x" ).IsValid() );
            ASSERT_FALSE( AUTH::CAccountRef( provider32 + "a", "x" ).IsValid() );
            CORE::CString subject512;
            for ( int i=0; i<512; ++i )
                subject512 += 'k';
            ASSERT_TRUE( AUTH::CAccountRef( "epic", subject512 ).IsValid() );
            ASSERT_FALSE( AUTH::CAccountRef( "epic", subject512 + "k" ).IsValid() );
            ASSERT_TRUE( AUTH::CAccountRef::IsValidProvider( "aws-cognito" ) && !AUTH::CAccountRef::IsValidProvider( "AWS" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Subjects with other characters are encoded one to one" )
        try
        {
            CORE::CString raw = "auth0|user 42/\xC3\xBC%";
            CORE::CString encoded = AUTH::CAccountRef::EncodeSubject( raw );
            ASSERT_TRUE( AUTH::CAccountRef::IsValidSubject( encoded ) );
            CORE::CString decoded;
            ASSERT_TRUE( AUTH::CAccountRef::DecodeSubject( encoded, decoded ) && decoded == raw );
            // different raw subjects stay different
            ASSERT_TRUE( AUTH::CAccountRef::EncodeSubject( "a b" ) != AUTH::CAccountRef::EncodeSubject( "a%20b" ) );
            ASSERT_TRUE( "a%20b" == AUTH::CAccountRef::EncodeSubject( "a b" ) );
            ASSERT_TRUE( "a%2520b" == AUTH::CAccountRef::EncodeSubject( "a%20b" ) );
            // an allowed subject is not changed
            ASSERT_TRUE( "eu-west-1_Pool:sub.1@x~+=|" == AUTH::CAccountRef::EncodeSubject( "eu-west-1_Pool:sub.1@x~+=|" ) );
            // broken escapes
            CORE::CString dummy;
            ASSERT_FALSE( AUTH::CAccountRef::DecodeSubject( "abc%", dummy ) );
            ASSERT_FALSE( AUTH::CAccountRef::DecodeSubject( "abc%2", dummy ) );
            ASSERT_FALSE( AUTH::CAccountRef::DecodeSubject( "abc%zz", dummy ) );
            ASSERT_FALSE( AUTH::CAccountRef::DecodeSubject( "abc%00", dummy ) );

            // in a stored subject a % is an escape of two upper case hex digits and nothing else
            ASSERT_TRUE( AUTH::CAccountRef::IsValidSubject( "a%20b" ) && AUTH::CAccountRef::IsValidSubject( "a%C3%BC" ) );
            ASSERT_FALSE( AUTH::CAccountRef::IsValidSubject( "a%zzb" ) || AUTH::CAccountRef::IsValidSubject( "a%2" ) || AUTH::CAccountRef::IsValidSubject( "a%" ) || AUTH::CAccountRef::IsValidSubject( "a%2fb" ) );
            ASSERT_TRUE( AUTH::CAccountRef( "google", "weird%20sub" ).IsValid() && !AUTH::CAccountRef( "google", "weird%2sub" ).IsValid() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: References are ordered, equal and hashed by their text" )
        try
        {
            AUTH::CAccountRef a( "microsoft", "100" );
            AUTH::CAccountRef b( "MICROSOFT", "100" );
            AUTH::CAccountRef c( "microsoft", "200" );
            AUTH::CAccountRef d( "google", "100" );
            ASSERT_TRUE( a == b && !( a != b ) && a != c && a != d );
            ASSERT_TRUE( a.GetHash() == b.GetHash() && a.GetHash() != c.GetHash() && a.GetHash() != d.GetHash() );
            ASSERT_TRUE( d < a && a < c && !( c < a ) && !( a < b ) && !( b < a ) );

            GUCEF::map< AUTH::CAccountRef, int > byRef;
            byRef[ a ] = 1;
            byRef[ c ] = 2;
            byRef[ b ] = 3;
            byRef[ d ] = 4;
            ASSERT_TRUE( 3 == byRef.size() && 3 == byRef[ a ] );
            ASSERT_TRUE( (*byRef.begin()).first == d );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The provider ids, id scopes and capability flags" )
        try
        {
            ASSERT_TRUE( AUTH::CAccountRef::IsValidProvider( AUTH::CAuthProviderIds::Epic ) );
            ASSERT_TRUE( AUTH::CAccountRef::IsValidProvider( AUTH::CAuthProviderIds::Facebook ) );
            ASSERT_TRUE( AUTH::CAccountRef::IsValidProvider( AUTH::CAuthProviderIds::Google ) );
            ASSERT_TRUE( AUTH::CAccountRef::IsValidProvider( AUTH::CAuthProviderIds::Microsoft ) );
            ASSERT_TRUE( AUTH::CAccountRef::IsValidProvider( AUTH::CAuthProviderIds::Amazon ) );
            ASSERT_TRUE( AUTH::CAccountRef::IsValidProvider( AUTH::CAuthProviderIds::Aws ) );
            ASSERT_TRUE( "epic" == AUTH::CAuthProviderIds::Epic && "aws" == AUTH::CAuthProviderIds::Aws );

            // every capability is a flag of its own
            UInt32 all = 0;
            UInt32 flags[] = { AUTH::AUTHCAP_BROWSER, AUTH::AUTHCAP_DEVICE_CODE, AUTH::AUTHCAP_SILENT, AUTH::AUTHCAP_SERVER_VERIFIABLE, AUTH::AUTHCAP_REFRESHABLE, AUTH::AUTHCAP_REQUIRES_CLIENT_ID };
            for ( size_t i=0; i<sizeof( flags ) / sizeof( flags[ 0 ] ); ++i )
            {
                ASSERT_TRUE( 0 == ( all & flags[ i ] ) && 0 != flags[ i ] );
                all |= flags[ i ];
            }
            ASSERT_TRUE( AUTH::AUTHIDSCOPE_GLOBAL != AUTH::AUTHIDSCOPE_PER_ORGANISATION && AUTH::AUTHIDSCOPE_PER_ORGANISATION != AUTH::AUTHIDSCOPE_PER_APP );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
