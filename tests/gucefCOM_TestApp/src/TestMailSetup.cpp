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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COM_CMAILSETUPVALIDATION_H
#include "gucefCOM_CMailSetupValidation.h"
#define GUCEF_COM_CMAILSETUPVALIDATION_H
#endif /* GUCEF_COM_CMAILSETUPVALIDATION_H ? */

#ifndef GUCEF_COM_CMAILPROVIDERPRESETS_H
#include "gucefCOM_CMailProviderPresets.h"
#define GUCEF_COM_CMAILPROVIDERPRESETS_H
#endif /* GUCEF_COM_CMAILPROVIDERPRESETS_H ? */

#ifndef GUCEF_COM_CMAILSETUPDIAGNOSIS_H
#include "gucefCOM_CMailSetupDiagnosis.h"
#define GUCEF_COM_CMAILSETUPDIAGNOSIS_H
#endif /* GUCEF_COM_CMAILSETUPDIAGNOSIS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestMailSetup.h"

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

bool
Contains( const CORE::CString& text, const char* part )
{
    return text.HasSubstr( part, true ) >= 0;
}

/** The id of the provider the address is recognized as, "custom" for unknown, "invalid" for an address that is no address */
CORE::CString
Recognized( const CMailProviderPresets& presets, const CORE::CString& address )
{
    const CMailProviderPreset* preset = GUCEF_NULL;
    const CMailProviderPresets::ERecognition found = presets.Recognize( address, preset );
    if ( GUCEF_NULL == preset )
        return "null";
    if ( CMailProviderPresets::RECOGNIZE_INVALID_ADDRESS == found )
        return "invalid";
    return preset->id;
}

CORE::CString
Repeat( char c, UInt32 count )
{
    GUCEF::vector< char > chars( count, c );
    return count > 0 ? CORE::CString( &chars[ 0 ], count ) : CORE::CString();
}

CMailProviderPreset
MakePreset( const char* id, const char* host )
{
    CMailProviderPreset preset;
    preset.id = id;
    preset.displayName = "Test provider";
    preset.domains.push_back( "test-provider.example" );
    preset.imap.host = host;
    preset.imap.port = 993;
    preset.imap.security = MAILTLS_IMPLICIT;
    preset.smtp.host = host;
    preset.smtp.port = 587;
    preset.smtp.security = MAILTLS_STARTTLS;
    return preset;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailSetupTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MailSetup" );

    GUCEF_TESTFW_TESTCASE( "Mail setup validation: addresses, hostile and malformed ones" )
    try
    {
        CORE::CString problem, local, domain;
        const char* const good[] = { "me@gmail.com", "first.last@example.org", "name+tag@example.org", "a@b.co", "UPPER@Example.ORG", "x_y-z@sub.domain.example.com", "o'brien@example.org" };
        for ( size_t i=0; i<sizeof( good ) / sizeof( good[ 0 ] ); ++i )
        {
            ASSERT_TRUE( CMailSetupValidation::CheckAddress( good[ i ], problem ) );
            ASSERT_TRUE( CMailSetupValidation::SplitAddress( good[ i ], local, domain ) && !local.IsNULLOrEmpty() && !domain.IsNULLOrEmpty() );
        }
        ASSERT_TRUE( CMailSetupValidation::GetDomain( "Name+Tag@MAIL.Example.ORG" ) == "mail.example.org" );

        const char* const bad[] = {
            "", " ", "me", "me@", "@gmail.com", "me@@gmail.com", "me@gmail", "me@gmail..com", "me@.gmail.com", "me@gmail.com.", "me@-gmail.com", "me@gmail-.com",
            "me gmail@gmail.com", "me@gmail .com", " me@gmail.com", "me@gmail.com ", ".me@gmail.com", "me.@gmail.com", "m..e@gmail.com", "me@gmail.com\r\nBcc: you@example.org",
            "me@gmail.com\n", "me@gmail.com\r", "me\r\n@gmail.com", "me@gmail.com\x01", "me\x7f@gmail.com", "me@gmail.c\xC3\xB6m", "m\xC3\xA9@gmail.com", "me@gmail.com,you@gmail.com",
            "me@gmail.com;you@gmail.com", "<me@gmail.com>", "\"me\"@gmail.com", "me(comment)@gmail.com", "me@[127.0.0.1]", "me@gma_il.com", "me@gmail.com/path", "me@gm ail.com"
        };
        for ( size_t i=0; i<sizeof( bad ) / sizeof( bad[ 0 ] ); ++i )
        {
            problem.Clear();
            ASSERT_FALSE( CMailSetupValidation::CheckAddress( bad[ i ], problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );
            ASSERT_FALSE( CMailSetupValidation::SplitAddress( bad[ i ], local, domain ) );
            ASSERT_TRUE( CMailSetupValidation::GetDomain( bad[ i ] ).IsNULLOrEmpty() );
        }
        // a NUL inside the text of a string with a length
        const char withNul[] = { 'm', 'e', '\0', '@', 'g', 'm', 'a', 'i', 'l', '.', 'c', 'o', 'm' };
        ASSERT_FALSE( CMailSetupValidation::CheckAddress( CORE::CString( withNul, sizeof( withNul ) ), problem ) );

        // length bounds: 64 characters of local part, 254 in all, labels of 63, a domain of 253
        ASSERT_TRUE( CMailSetupValidation::CheckAddress( Repeat( 'a', 64 ) + "@example.org", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckAddress( Repeat( 'a', 65 ) + "@example.org", problem ) );
        ASSERT_TRUE( CMailSetupValidation::CheckAddress( "a@" + Repeat( 'b', 63 ) + ".org", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckAddress( "a@" + Repeat( 'b', 64 ) + ".org", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckAddress( Repeat( 'a', 300 ) + "@example.org", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckAddress( "a@" + Repeat( 'b', 100000 ) + ".org", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckAddress( Repeat( 'a', 1000000 ), problem ) );
        CORE::CString longDomain = "a@";
        while ( longDomain.Length() < 260 )
            longDomain += "abcdefghij.";
        longDomain += "org";
        ASSERT_FALSE( CMailSetupValidation::CheckAddress( longDomain, problem ) );
        // the sentence never repeats the input
        ASSERT_FALSE( CMailSetupValidation::CheckAddress( "secret-person@", problem ) || Contains( problem, "secret-person" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail setup validation: host names, ports, user names, folders and display names" )
    try
    {
        CORE::CString problem;
        const char* const goodHosts[] = { "imap.gmail.com", "localhost", "127.0.0.1", "mail-1.example.org", "a.b", "MAIL.Example.ORG", "10.0.0.255" };
        for ( size_t i=0; i<sizeof( goodHosts ) / sizeof( goodHosts[ 0 ] ); ++i )
        {
            ASSERT_TRUE( CMailSetupValidation::IsValidHostName( goodHosts[ i ] ) );
            ASSERT_TRUE( CMailSetupValidation::CheckHost( goodHosts[ i ], problem ) );
        }
        const char* const badHosts[] = {
            "", " ", "imap gmail.com", "imap.gmail.com\r\n", "imap.gmail.com\n", "imap..gmail.com", ".imap.gmail.com", "imap.gmail.com.", "-imap.gmail.com", "imap-.gmail.com",
            "imap.gmail.com:993", "imap.gmail.com/", "http://imap.gmail.com", "user@imap.gmail.com", "imap.g\xC3\xA4mail.com", "imap.gmail.com\x01", "imap_gmail.com", "[::1]", "999.999.999.999x", "1.2.3.4.5.6x"
        };
        for ( size_t i=0; i<sizeof( badHosts ) / sizeof( badHosts[ 0 ] ); ++i )
        {
            ASSERT_FALSE( CMailSetupValidation::IsValidHostName( badHosts[ i ] ) );
            problem.Clear();
            ASSERT_FALSE( CMailSetupValidation::CheckHost( badHosts[ i ], problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );
        }
        ASSERT_FALSE( CMailSetupValidation::IsValidHostName( Repeat( 'a', 64 ) + ".com" ) );
        ASSERT_FALSE( CMailSetupValidation::IsValidHostName( Repeat( 'a', 300 ) ) );
        ASSERT_FALSE( CMailSetupValidation::IsValidHostName( Repeat( 'a', 500000 ) ) );

        UInt16 port = 77;
        ASSERT_TRUE( CMailSetupValidation::CheckPortText( "", port, problem ) && 0 == port );
        ASSERT_TRUE( CMailSetupValidation::CheckPortText( "993", port, problem ) && 993 == port );
        ASSERT_TRUE( CMailSetupValidation::CheckPortText( "1", port, problem ) && 1 == port );
        ASSERT_TRUE( CMailSetupValidation::CheckPortText( "65535", port, problem ) && 65535 == port );
        const char* const badPorts[] = { "0", "65536", "-1", "99999999999999999999", "abc", "99a", " 993", "993 ", "9 93", "0x3E1", "993\r\n", "+993", "1e3", "\xD9\xA9\xD9\xA9\xD9\xA3" };
        for ( size_t i=0; i<sizeof( badPorts ) / sizeof( badPorts[ 0 ] ); ++i )
        {
            port = 77;
            problem.Clear();
            ASSERT_FALSE( CMailSetupValidation::CheckPortText( badPorts[ i ], port, problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );
        }
        ASSERT_FALSE( CMailSetupValidation::CheckPortText( Repeat( '9', 100000 ), port, problem ) );

        ASSERT_TRUE( CMailSetupValidation::CheckUserName( "", problem ) );
        ASSERT_TRUE( CMailSetupValidation::CheckUserName( "me@example.org", problem ) );
        ASSERT_TRUE( CMailSetupValidation::CheckUserName( "DOMAIN\\me", problem ) );
        ASSERT_TRUE( CMailSetupValidation::CheckUserName( Repeat( 'u', 256 ), problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckUserName( Repeat( 'u', 257 ), problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckUserName( "me\r\nRCPT TO:<x@example.org>", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckUserName( "me\x01", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckUserName( Repeat( 'u', 2000000 ), problem ) );

        ASSERT_TRUE( CMailSetupValidation::CheckFolderName( "", problem ) );
        ASSERT_TRUE( CMailSetupValidation::CheckFolderName( "INBOX", problem ) );
        ASSERT_TRUE( CMailSetupValidation::CheckFolderName( "Games/Redux", problem ) );
        ASSERT_TRUE( CMailSetupValidation::CheckFolderName( "Spiele \xC3\xBC" "ber", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckFolderName( "INBOX\r\nA2 DELETE INBOX", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckFolderName( "INBOX\x7f", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckFolderName( Repeat( 'f', 256 ), problem ) );

        ASSERT_TRUE( CMailSetupValidation::CheckDisplayName( "", problem ) );
        ASSERT_TRUE( CMailSetupValidation::CheckDisplayName( "Dinand \xC3\xA9", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckDisplayName( "Dinand\r\nBcc: x@example.org", problem ) );
        ASSERT_FALSE( CMailSetupValidation::CheckDisplayName( Repeat( 'n', 101 ), problem ) );
        ASSERT_TRUE( CMailSetupValidation::IsPlainText( "abc", 3 ) );
        ASSERT_FALSE( CMailSetupValidation::IsPlainText( "abcd", 3 ) );
        ASSERT_FALSE( CMailSetupValidation::IsPlainText( "a\tb", 10 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail provider presets: the addresses of the known providers are recognized, in any case, with a plus tag and as a subdomain" )
    try
    {
        CMailProviderPresets presets;
        ASSERT_TRUE( presets.GetCount() >= 8 );
        const struct { const char* address; const char* id; } known[] =
        {
            { "me@gmail.com", "gmail" }, { "me@googlemail.com", "gmail" },
            { "me@outlook.com", "microsoft" }, { "me@hotmail.com", "microsoft" }, { "me@live.com", "microsoft" }, { "me@msn.com", "microsoft" },
            { "me@yahoo.com", "yahoo" }, { "me@ymail.com", "yahoo" },
            { "me@icloud.com", "icloud" }, { "me@me.com", "icloud" },
            { "me@gmx.net", "gmx" }, { "me@gmx.de", "gmx" },
            { "me@web.de", "webde" },
            { "me@fastmail.com", "fastmail" }, { "me@fastmail.fm", "fastmail" },
            { "ME@GMAIL.COM", "gmail" }, { "Me@GMail.Com", "gmail" }, { "me@Outlook.COM", "microsoft" },
            { "me+redux@gmail.com", "gmail" }, { "first.last+a+b@gmail.com", "gmail" },
            { "me@mail.fastmail.com", "fastmail" }, { "me@eu.web.de", "webde" }
        };
        for ( size_t i=0; i<sizeof( known ) / sizeof( known[ 0 ] ); ++i )
        {
            const CMailProviderPreset* preset = GUCEF_NULL;
            ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_KNOWN == presets.Recognize( known[ i ].address, preset ) );
            ASSERT_TRUE( GUCEF_NULL != preset && preset->id == known[ i ].id );
        }

        // unknown domains, and domains that only look like a known one, are the custom entry
        const char* const unknown[] = { "me@example.org", "me@company.com", "me@evilgmail.com", "me@gmail.com.evil.example", "me@notgmail.com", "me@gmail.co", "me@gmailcom.com", "me@xoutlook.com", "me@hotmail.com.au.example" };
        for ( size_t i=0; i<sizeof( unknown ) / sizeof( unknown[ 0 ] ); ++i )
        {
            const CMailProviderPreset* preset = GUCEF_NULL;
            ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_UNKNOWN == presets.Recognize( unknown[ i ], preset ) );
            ASSERT_TRUE( GUCEF_NULL != preset && preset->id == CMailProviderPresets::CustomId && preset->serverFieldsEditable );
        }

        // addresses that are no addresses recognize nothing
        const char* const hostile[] = { "", "gmail.com", "@gmail.com", "me@", "me@@gmail.com", "me@gmail.com\r\nBcc: x@example.org", "me@gmail.com\n", "me gmail@gmail.com", "me@gmail.com\x01", "me@gma\xC3\xADl.com" };
        for ( size_t i=0; i<sizeof( hostile ) / sizeof( hostile[ 0 ] ); ++i )
        {
            const CMailProviderPreset* preset = GUCEF_NULL;
            ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_INVALID_ADDRESS == presets.Recognize( hostile[ i ], preset ) );
            ASSERT_TRUE( GUCEF_NULL != preset && preset->id == CMailProviderPresets::CustomId );
        }
        const CMailProviderPreset* preset = GUCEF_NULL;
        ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_INVALID_ADDRESS == presets.Recognize( Repeat( 'a', 100000 ) + "@gmail.com", preset ) && GUCEF_NULL != preset );
        ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_INVALID_ADDRESS == presets.Recognize( "me@" + Repeat( 'a', 100000 ) + ".gmail.com", preset ) && GUCEF_NULL != preset );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail provider presets: the servers, ports, security and sign-in of the defaults" )
    try
    {
        CMailProviderPresets presets;
        const CMailProviderPreset* gmail = presets.FindById( "gmail" );
        ASSERT_TRUE( GUCEF_NULL != gmail );
        ASSERT_TRUE( gmail->imap.host == "imap.gmail.com" && 993 == gmail->imap.port && MAILTLS_IMPLICIT == gmail->imap.security );
        ASSERT_TRUE( gmail->smtp.host == "smtp.gmail.com" && 587 == gmail->smtp.port && MAILTLS_STARTTLS == gmail->smtp.security );
        ASSERT_TRUE( gmail->pop3.host == "pop.gmail.com" && 995 == gmail->pop3.port && MAILTLS_IMPLICIT == gmail->pop3.security );
        ASSERT_TRUE( MAILPASSWORD_APP_PASSWORD == gmail->passwordPolicy && gmail->oauthProvider == "google" && gmail->HasOAuth() );
        bool mailScope = false;
        for ( size_t i=0; i<gmail->oauthScopes.size(); ++i )
            mailScope = mailScope || gmail->oauthScopes[ i ] == "https://mail.google.com/";
        ASSERT_TRUE( mailScope );
        ASSERT_FALSE( gmail->serverFieldsEditable );

        const CMailProviderPreset* microsoft = presets.FindById( "microsoft" );
        ASSERT_TRUE( GUCEF_NULL != microsoft );
        ASSERT_TRUE( microsoft->imap.host == "outlook.office365.com" && 993 == microsoft->imap.port && microsoft->smtp.host == "smtp.office365.com" && 587 == microsoft->smtp.port && MAILTLS_STARTTLS == microsoft->smtp.security );
        ASSERT_TRUE( microsoft->oauthProvider == "microsoft" && MAILPASSWORD_MAY_BE_REFUSED == microsoft->passwordPolicy );
        ASSERT_FALSE( microsoft->oauthScopes.empty() );

        const char* const passwordProviders[] = { "yahoo", "icloud", "gmx", "webde", "fastmail" };
        for ( size_t i=0; i<sizeof( passwordProviders ) / sizeof( passwordProviders[ 0 ] ); ++i )
        {
            const CMailProviderPreset* p = presets.FindById( passwordProviders[ i ] );
            ASSERT_TRUE( GUCEF_NULL != p );
            CORE::CString reason;
            ASSERT_TRUE( p->IsValid( reason ) );
            ASSERT_TRUE( p->imap.IsSet() && p->smtp.IsSet() && !p->HasOAuth() && !p->serverFieldsEditable );
            ASSERT_FALSE( p->passwordHelp.IsNULLOrEmpty() );
            ASSERT_TRUE( 993 == p->imap.GetEffectivePort( MAILPROTOCOL_IMAP ) );
        }
        ASSERT_TRUE( MAILPASSWORD_APP_PASSWORD == presets.FindById( "yahoo" )->passwordPolicy && MAILPASSWORD_APP_PASSWORD == presets.FindById( "icloud" )->passwordPolicy && MAILPASSWORD_APP_PASSWORD == presets.FindById( "fastmail" )->passwordPolicy );
        ASSERT_TRUE( presets.FindById( "gmx" )->imap.host == "imap.gmx.net" && presets.FindById( "webde" )->imap.host == "imap.web.de" && presets.FindById( "fastmail" )->imap.host == "imap.fastmail.com" );

        const CMailProviderPreset& custom = presets.GetCustomPreset();
        ASSERT_TRUE( custom.id == "custom" && custom.serverFieldsEditable && custom.domains.empty() && !custom.HasOAuth() );
        ASSERT_TRUE( GUCEF_NULL != presets.FindById( "custom" ) );

        // the usual port of a protocol and security when the preset leaves it out
        CMailServerPreset server;
        server.host = "x.example.org";
        server.security = MAILTLS_IMPLICIT;
        ASSERT_TRUE( 993 == server.GetEffectivePort( MAILPROTOCOL_IMAP ) && 995 == server.GetEffectivePort( MAILPROTOCOL_POP3 ) && 465 == server.GetEffectivePort( MAILPROTOCOL_SMTP ) );
        server.security = MAILTLS_STARTTLS;
        ASSERT_TRUE( 143 == server.GetEffectivePort( MAILPROTOCOL_IMAP ) && 110 == server.GetEffectivePort( MAILPROTOCOL_POP3 ) && 587 == server.GetEffectivePort( MAILPROTOCOL_SMTP ) );
        server.port = 2525;
        ASSERT_TRUE( 2525 == server.GetEffectivePort( MAILPROTOCOL_SMTP ) );

        // every default entry is valid and has domains that do not clash with another entry
        for ( UInt32 i=0; i<presets.GetCount(); ++i )
        {
            CORE::CString reason;
            ASSERT_TRUE( presets.GetPreset( i )->IsValid( reason ) );
            for ( size_t d=0; d<presets.GetPreset( i )->domains.size(); ++d )
            {
                const CMailProviderPreset* other = GUCEF_NULL;
                ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_KNOWN == presets.Recognize( CORE::CString( "me@" ) + presets.GetPreset( i )->domains[ d ], other ) && other->id == presets.GetPreset( i )->id );
            }
        }
        ASSERT_TRUE( GUCEF_NULL == presets.GetPreset( presets.GetCount() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail provider presets: the table is data: saved, loaded, overridden, extended, reduced, and garbage is skipped" )
    try
    {
        CMailProviderPresets defaults;

        // a saved table loads back as the same table
        CORE::CDataNode saved( "MailProviders" );
        ASSERT_TRUE( defaults.SaveConfig( saved ) );
        CMailProviderPresets copy;
        copy.Clear();
        ASSERT_TRUE( 0 == copy.GetCount() );
        ASSERT_TRUE( copy.LoadConfig( saved ) );
        ASSERT_TRUE( copy.GetCount() == defaults.GetCount() && 0 == copy.GetSkippedCount() );
        for ( UInt32 i=0; i<defaults.GetCount(); ++i )
        {
            const CMailProviderPreset* a = defaults.GetPreset( i );
            const CMailProviderPreset* b = copy.FindById( a->id );
            ASSERT_TRUE( GUCEF_NULL != b );
            ASSERT_TRUE( a->displayName == b->displayName && a->domains == b->domains && a->imap.host == b->imap.host && a->imap.port == b->imap.port && a->imap.security == b->imap.security );
            ASSERT_TRUE( a->smtp.host == b->smtp.host && a->smtp.port == b->smtp.port && a->smtp.security == b->smtp.security && a->pop3.host == b->pop3.host );
            ASSERT_TRUE( a->passwordPolicy == b->passwordPolicy && a->oauthProvider == b->oauthProvider && a->oauthScopes == b->oauthScopes );
            ASSERT_TRUE( a->serverFieldsEditable == b->serverFieldsEditable && a->passwordHelp == b->passwordHelp && a->helpUrl == b->helpUrl );
        }

        // an entry of the configuration overrides the default of the same id, a new id is added, the others stay
        CORE::CDataNode config( "MailProviders" );
        CORE::CDataNode* override = config.AddChild( "gmail" );
        override->SetAttribute( "imapHost", "imap.override.example" );
        override->SetAttribute( "imapPort", (UInt16) 1993 );
        override->SetAttribute( "domains", "gmail.com,override-mail.example" );
        CORE::CDataNode* added = config.AddChild( "corpmail" );
        added->SetAttribute( "displayName", "Corp mail" );
        added->SetAttribute( "domains", "corp.example, mail.corp.example" );
        added->SetAttribute( "imapHost", "imap.corp.example" );
        added->SetAttribute( "imapSecurity", "starttls" );
        added->SetAttribute( "smtpHost", "smtp.corp.example" );
        added->SetAttribute( "smtpPort", (UInt16) 2525 );
        added->SetAttribute( "smtpSecurity", "none" );
        added->SetAttribute( "passwordPolicy", "app" );
        CMailProviderPresets table;
        const UInt32 before = table.GetCount();
        ASSERT_TRUE( table.LoadConfig( config ) );
        ASSERT_TRUE( table.GetCount() == before + 1 && 0 == table.GetSkippedCount() );
        const CMailProviderPreset* gmail = table.FindById( "gmail" );
        ASSERT_TRUE( GUCEF_NULL != gmail && gmail->imap.host == "imap.override.example" && 1993 == gmail->imap.port );
        ASSERT_TRUE( gmail->smtp.host == "smtp.gmail.com" );     // what the entry does not say stays
        ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_KNOWN == table.Recognize( "x@override-mail.example", gmail ) && gmail->id == "gmail" );
        const CMailProviderPreset* corp = GUCEF_NULL;
        ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_KNOWN == table.Recognize( "x@mail.corp.example", corp ) && GUCEF_NULL != corp && corp->id == "corpmail" );
        ASSERT_TRUE( corp->imap.host == "imap.corp.example" && MAILTLS_STARTTLS == corp->imap.security && 143 == corp->imap.GetEffectivePort( MAILPROTOCOL_IMAP ) );
        ASSERT_TRUE( 2525 == corp->smtp.port && MAILTLS_NONE == corp->smtp.security && MAILPASSWORD_APP_PASSWORD == corp->passwordPolicy && corp->domains.size() == 2 );

        // remove, and replace the defaults
        CORE::CDataNode removal( "MailProviders" );
        CORE::CDataNode* gone = removal.AddChild( "yahoo" );
        gone->SetAttribute( "remove", true );
        ASSERT_TRUE( table.LoadConfig( removal ) );
        ASSERT_TRUE( GUCEF_NULL == table.FindById( "yahoo" ) );
        const CMailProviderPreset* unknownNow = GUCEF_NULL;
        ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_UNKNOWN == table.Recognize( "x@yahoo.com", unknownNow ) && unknownNow->id == "custom" );
        CORE::CDataNode replacement( "MailProviders" );
        replacement.SetAttribute( "replaceDefaults", true );
        replacement.AddChild( "only" )->SetAttribute( "imapHost", "imap.only.example" );
        replacement.FindChild( "only" )->SetAttribute( "smtpHost", "smtp.only.example" );
        replacement.FindChild( "only" )->SetAttribute( "domains", "only.example" );
        ASSERT_TRUE( table.LoadConfig( replacement ) );
        ASSERT_TRUE( GUCEF_NULL != table.FindById( "only" ) && GUCEF_NULL == table.FindById( "gmail" ) );
        // the table never loses the custom entry for unknown domains
        ASSERT_TRUE( CMailProviderPresets::RECOGNIZE_UNKNOWN == table.Recognize( "x@gmail.com", unknownNow ) && GUCEF_NULL != unknownNow && unknownNow->id == "custom" );
        table.LoadDefaults();
        ASSERT_TRUE( table.GetCount() == before && GUCEF_NULL != table.FindById( "yahoo" ) && table.FindById( "gmail" )->imap.host == "imap.gmail.com" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail provider presets: entries that are not usable are skipped and counted, the rest is taken" )
    try
    {
        CMailProviderPresets table;
        const UInt32 before = table.GetCount();
        CORE::CDataNode config( "MailProviders" );
        // no server at all, a port out of range, a host with a line break, a host with a path, an id with upper case and a space, a policy that does not exist, too long a host
        config.AddChild( "noserver" )->SetAttribute( "domains", "x.example" );
        CORE::CDataNode* badPort = config.AddChild( "badport" );
        badPort->SetAttribute( "imapHost", "imap.x.example" );
        badPort->SetAttribute( "smtpHost", "smtp.x.example" );
        badPort->SetAttribute( "imapPort", (UInt32) 70000 );
        CORE::CDataNode* badHost = config.AddChild( "badhost" );
        badHost->SetAttribute( "imapHost", "imap.x.example\r\nevil" );
        badHost->SetAttribute( "smtpHost", "smtp.x.example" );
        CORE::CDataNode* path = config.AddChild( "path" );
        path->SetAttribute( "imapHost", "imap.x.example/../../" );
        path->SetAttribute( "smtpHost", "smtp.x.example" );
        CORE::CDataNode* badId = config.AddChild( "Bad Id" );
        badId->SetAttribute( "imapHost", "imap.x.example" );
        badId->SetAttribute( "smtpHost", "smtp.x.example" );
        CORE::CDataNode* badPolicy = config.AddChild( "badpolicy" );
        badPolicy->SetAttribute( "imapHost", "imap.x.example" );
        badPolicy->SetAttribute( "smtpHost", "smtp.x.example" );
        badPolicy->SetAttribute( "passwordPolicy", "whatever" );
        CORE::CDataNode* longHost = config.AddChild( "longhost" );
        longHost->SetAttribute( "imapHost", Repeat( 'a', 300 ) );
        longHost->SetAttribute( "smtpHost", "smtp.x.example" );
        CORE::CDataNode* badDomain = config.AddChild( "baddomain" );
        badDomain->SetAttribute( "imapHost", "imap.x.example" );
        badDomain->SetAttribute( "smtpHost", "smtp.x.example" );
        badDomain->SetAttribute( "domains", "good.example,bad domain" );
        CORE::CDataNode* fine = config.AddChild( "fine" );
        fine->SetAttribute( "imapHost", "imap.fine.example" );
        fine->SetAttribute( "smtpHost", "smtp.fine.example" );
        fine->SetAttribute( "domains", "fine.example" );
        ASSERT_TRUE( table.LoadConfig( config ) );
        ASSERT_TRUE( 8 == table.GetSkippedCount() );
        ASSERT_TRUE( table.GetCount() == before + 1 && GUCEF_NULL != table.FindById( "fine" ) );
        ASSERT_TRUE( GUCEF_NULL == table.FindById( "noserver" ) && GUCEF_NULL == table.FindById( "badhost" ) && GUCEF_NULL == table.FindById( "baddomain" ) );

        // an entry that does not load leaves the entry of the table as it was
        CORE::CDataNode override( "MailProviders" );
        CORE::CDataNode* bad = override.AddChild( "gmail" );
        bad->SetAttribute( "imapHost", "imap.evil.example\n" );
        ASSERT_TRUE( table.LoadConfig( override ) );
        ASSERT_TRUE( 1 == table.GetSkippedCount() && table.FindById( "gmail" )->imap.host == "imap.gmail.com" );

        // the table is bounded
        CMailProviderPresets full;
        full.Clear();
        UInt32 added = 0;
        for ( UInt32 i=0; i<CMailProviderPresets::MaxEntries + 10; ++i )
        {
            CMailProviderPreset preset = MakePreset( ( CORE::CString( "p" ) + CORE::UInt32ToString( i ) ).C_String(), "imap.x.example" );
            preset.domains.clear();
            if ( full.Add( preset ) )
                ++added;
        }
        ASSERT_TRUE( CMailProviderPresets::MaxEntries == added && CMailProviderPresets::MaxEntries == full.GetCount() );
        // a preset with too many domains is refused
        CMailProviderPreset many = MakePreset( "many", "imap.x.example" );
        many.domains.clear();
        for ( UInt32 i=0; i<CMailProviderPresets::MaxDomainsPerEntry + 1; ++i )
            many.domains.push_back( CORE::CString( "d" ) + CORE::UInt32ToString( i ) + ".example" );
        CORE::CString reason;
        ASSERT_FALSE( many.IsValid( reason ) || reason.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail provider presets: the sign-in text in plain words, with and without an OAuth client id" )
    try
    {
        CMailProviderPresets presets;
        const CMailProviderPreset& gmail = *presets.FindById( "gmail" );
        const CORE::CString gmailNoOAuth = CMailProviderPresets::DescribeSignIn( gmail, false );
        ASSERT_TRUE( Contains( gmailNoOAuth, "2-step verification" ) && Contains( gmailNoOAuth, "app password" ) );
        ASSERT_TRUE( Contains( gmailNoOAuth, "Sign-in with Google is not available in this build" ) );
        ASSERT_FALSE( Contains( gmailNoOAuth, "Sign in with Google" ) );
        const CORE::CString gmailOAuth = CMailProviderPresets::DescribeSignIn( gmail, true );
        ASSERT_TRUE( Contains( gmailOAuth, "Sign in with Google" ) && Contains( gmailOAuth, "app password" ) );
        ASSERT_FALSE( Contains( gmailOAuth, "not available in this build" ) );

        const CMailProviderPreset& microsoft = *presets.FindById( "microsoft" );
        const CORE::CString msNoOAuth = CMailProviderPresets::DescribeSignIn( microsoft, false );
        ASSERT_TRUE( Contains( msNoOAuth, "Sign-in with Microsoft is not available in this build" ) && Contains( msNoOAuth, "password" ) );
        const CORE::CString msOAuth = CMailProviderPresets::DescribeSignIn( microsoft, true );
        ASSERT_TRUE( Contains( msOAuth, "Sign in with Microsoft" ) && !Contains( msOAuth, "not available in this build" ) );

        const CORE::CString yahoo = CMailProviderPresets::DescribeSignIn( *presets.FindById( "yahoo" ), false );
        ASSERT_TRUE( Contains( yahoo, "app password" ) && !Contains( yahoo, "Sign-in with" ) );
        const CORE::CString customText = CMailProviderPresets::DescribeSignIn( presets.GetCustomPreset(), false );
        ASSERT_TRUE( Contains( customText, "server" ) && !customText.IsNULLOrEmpty() );
        // oauth availability is irrelevant for a provider that has none
        ASSERT_TRUE( CMailProviderPresets::DescribeSignIn( presets.GetCustomPreset(), true ) == customText );

        for ( UInt32 i=0; i<presets.GetCount(); ++i )
        {
            const CORE::CString text = CMailProviderPresets::DescribeSignIn( *presets.GetPreset( i ), false );
            ASSERT_FALSE( text.IsNULLOrEmpty() );
            ASSERT_TRUE( text.HasSubstr( "@", true ) < 0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail setup diagnosis: each failure of a step is a problem in plain words, with what to try for the kind of provider" )
    try
    {
        // classification
        ASSERT_TRUE( MAILSETUP_UNREACHABLE == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_INCOMING, MAILERR_CONNECTION ) );
        ASSERT_TRUE( MAILSETUP_UNREACHABLE == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_OUTGOING, MAILERR_CONNECTION ) );
        ASSERT_TRUE( MAILSETUP_TLS_PROBLEM == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_INCOMING, MAILERR_TLS ) );
        ASSERT_TRUE( MAILSETUP_TLS_PROBLEM == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_OUTGOING, MAILERR_TLS ) );
        ASSERT_TRUE( MAILSETUP_LOGIN_REFUSED == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_INCOMING, MAILERR_AUTH ) );
        ASSERT_TRUE( MAILSETUP_SMTP_REFUSED == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_OUTGOING, MAILERR_AUTH ) );
        ASSERT_TRUE( MAILSETUP_SMTP_REFUSED == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_OUTGOING, MAILERR_PERMANENT ) );
        ASSERT_TRUE( MAILSETUP_FOLDER_NOT_FOUND == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_FOLDER, MAILERR_PERMANENT ) );
        ASSERT_TRUE( MAILSETUP_NO_ANSWER == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_INCOMING, MAILERR_TIMEOUT ) );
        ASSERT_TRUE( MAILSETUP_NO_ANSWER == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_OUTGOING, MAILERR_TIMEOUT ) );
        ASSERT_TRUE( MAILSETUP_OTHER == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_INCOMING, MAILERR_PROTOCOL ) );
        ASSERT_TRUE( MAILSETUP_OTHER == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_INCOMING, MAILERR_CANCELLED ) );
        ASSERT_TRUE( MAILSETUP_OK == CMailSetupDiagnosis::Classify( MAILSETUP_STEP_INCOMING, MAILERR_NONE ) );

        CMailProviderPresets presets;
        const CMailProviderPreset* gmail = presets.FindById( "gmail" );
        const CMailProviderPreset* microsoft = presets.FindById( "microsoft" );
        const CMailProviderPreset* fastmail = presets.FindById( "fastmail" );
        const CMailProviderPreset* custom = &presets.GetCustomPreset();

        // every problem at every step has a text, for every kind of provider, without anything secret
        const EMailSetupProblem problems[] = { MAILSETUP_OK, MAILSETUP_UNREACHABLE, MAILSETUP_TLS_PROBLEM, MAILSETUP_LOGIN_REFUSED, MAILSETUP_FOLDER_NOT_FOUND, MAILSETUP_SMTP_REFUSED, MAILSETUP_NO_ANSWER, MAILSETUP_OTHER };
        const EMailSetupStep steps[] = { MAILSETUP_STEP_INCOMING, MAILSETUP_STEP_FOLDER, MAILSETUP_STEP_OUTGOING };
        const CMailProviderPreset* kinds[] = { GUCEF_NULL, gmail, microsoft, fastmail, custom };
        for ( size_t p=0; p<sizeof( problems ) / sizeof( problems[ 0 ] ); ++p )
            for ( size_t s=0; s<sizeof( steps ) / sizeof( steps[ 0 ] ); ++s )
                for ( size_t k=0; k<sizeof( kinds ) / sizeof( kinds[ 0 ] ); ++k )
                    for ( int oauth=0; oauth<2; ++oauth )
                    {
                        const CORE::CString text = CMailSetupDiagnosis::Describe( steps[ s ], problems[ p ], kinds[ k ], 0 != oauth, "imap.example.org", 993 );
                        ASSERT_FALSE( text.IsNULLOrEmpty() );
                        ASSERT_TRUE( text.HasSubstr( "@", true ) < 0 );
                        ASSERT_TRUE( text.Length() < 900 );
                    }

        // what each problem says
        CORE::CString text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_INCOMING, MAILSETUP_UNREACHABLE, custom, false, "imap.example.org", 993 );
        ASSERT_TRUE( Contains( text, "could not be reached" ) && Contains( text, "imap.example.org" ) && Contains( text, "993" ) && Contains( text, "internet" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_OUTGOING, MAILSETUP_UNREACHABLE, gmail, false, "smtp.gmail.com", 587 );
        ASSERT_TRUE( Contains( text, "outgoing" ) && Contains( text, "smtp.gmail.com" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_INCOMING, MAILSETUP_TLS_PROBLEM, custom, false, "imap.example.org", 143 );
        ASSERT_TRUE( Contains( text, "secure connection" ) && Contains( text, "security" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_INCOMING, MAILSETUP_LOGIN_REFUSED, gmail, false, "imap.gmail.com", 993 );
        ASSERT_TRUE( Contains( text, "refused" ) && Contains( text, "app password" ) && Contains( text, "2-step verification" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_INCOMING, MAILSETUP_LOGIN_REFUSED, gmail, true, "imap.gmail.com", 993 );
        ASSERT_TRUE( Contains( text, "refused" ) && Contains( text, "sign in" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_INCOMING, MAILSETUP_LOGIN_REFUSED, microsoft, false, "outlook.office365.com", 993 );
        ASSERT_TRUE( Contains( text, "refused" ) && Contains( text, "Microsoft" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_INCOMING, MAILSETUP_LOGIN_REFUSED, custom, false, "imap.example.org", 993 );
        ASSERT_TRUE( Contains( text, "user name" ) && Contains( text, "password" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_FOLDER, MAILSETUP_FOLDER_NOT_FOUND, custom, false, "imap.example.org", 993 );
        ASSERT_TRUE( Contains( text, "folder" ) && Contains( text, "INBOX" ) && Contains( text, "login" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_OUTGOING, MAILSETUP_SMTP_REFUSED, gmail, false, "smtp.gmail.com", 587 );
        ASSERT_TRUE( Contains( text, "outgoing" ) && Contains( text, "refused" ) && Contains( text, "app password" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_OUTGOING, MAILSETUP_NO_ANSWER, custom, false, "smtp.example.org", 587 );
        ASSERT_TRUE( Contains( text, "did not answer" ) && Contains( text, "outgoing" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_INCOMING, MAILSETUP_OK, custom, false, "imap.example.org", 993 );
        ASSERT_TRUE( Contains( text, "incoming" ) && Contains( text, "accepted" ) );
        text = CMailSetupDiagnosis::Describe( MAILSETUP_STEP_FOLDER, MAILSETUP_OK, custom, false, "", 0 );
        ASSERT_TRUE( Contains( text, "folder" ) );

        // which problems mean new settings
        ASSERT_TRUE( CMailSetupDiagnosis::NeedsNewSettings( MAILSETUP_LOGIN_REFUSED ) && CMailSetupDiagnosis::NeedsNewSettings( MAILSETUP_SMTP_REFUSED ) );
        ASSERT_TRUE( CMailSetupDiagnosis::NeedsNewSettings( MAILSETUP_TLS_PROBLEM ) && CMailSetupDiagnosis::NeedsNewSettings( MAILSETUP_FOLDER_NOT_FOUND ) );
        ASSERT_FALSE( CMailSetupDiagnosis::NeedsNewSettings( MAILSETUP_UNREACHABLE ) || CMailSetupDiagnosis::NeedsNewSettings( MAILSETUP_NO_ANSWER ) || CMailSetupDiagnosis::NeedsNewSettings( MAILSETUP_OK ) || CMailSetupDiagnosis::NeedsNewSettings( MAILSETUP_OTHER ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
