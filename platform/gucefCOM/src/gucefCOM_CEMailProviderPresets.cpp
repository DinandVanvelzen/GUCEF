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


#include "gucefCOM_CEMailProviderPresets.h"

#ifndef GUCEF_COM_CEMAILSETUPVALIDATION_H
#include "gucefCOM_CEMailSetupValidation.h"
#define GUCEF_COM_CEMAILSETUPVALIDATION_H
#endif /* GUCEF_COM_CEMAILSETUPVALIDATION_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

const char* const CEMailProviderPresets::CustomId = "custom";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
SetServer( CEMailServerPreset& server, const char* host, UInt16 port, EMailTlsMode security )
{
    server.host = host;
    server.port = port;
    server.security = security;
}

void
AddDomains( CEMailProviderPreset& preset, const char* const* domains )
{
    for ( ; GUCEF_NULL != *domains; ++domains )
        preset.domains.push_back( *domains );
}

CEMailProviderPreset
MakePreset( const char* id, const char* displayName )
{
    CEMailProviderPreset preset;
    preset.id = id;
    preset.displayName = displayName;
    return preset;
}

/** The entry for the providers that are not in the table: the person fills in the servers */
CEMailProviderPreset
MakeCustomPreset( void )
{
    CEMailProviderPreset preset = MakePreset( CEMailProviderPresets::CustomId, "Other / custom" );
    preset.serverFieldsEditable = true;
    preset.passwordHelp = "Use the user name and the password that your mail provider gave you. If your provider asks for an app password or a special password for mail programs, use that one.";
    return preset;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailProviderPresets::CEMailProviderPresets( void )
    : CORE::CIConfigurable()
    , m_presets()
    , m_fallbackCustom( MakeCustomPreset() )
    , m_skipped( 0 )
{GUCEF_TRACE;

    LoadDefaults();
}

CEMailProviderPresets::~CEMailProviderPresets()
{GUCEF_TRACE;

}

void
CEMailProviderPresets::Clear( void )
{GUCEF_TRACE;

    m_presets.clear();
    m_skipped = 0;
}

void
CEMailProviderPresets::LoadDefaults( void )
{GUCEF_TRACE;

    Clear();

    {
        static const char* const domains[] = { "gmail.com", "googlemail.com", GUCEF_NULL };
        CEMailProviderPreset p = MakePreset( "gmail", "Gmail and Google Workspace" );
        AddDomains( p, domains );
        SetServer( p.imap, "imap.gmail.com", 993, MAILTLS_IMPLICIT );
        SetServer( p.smtp, "smtp.gmail.com", 587, MAILTLS_STARTTLS );
        SetServer( p.pop3, "pop.gmail.com", 995, MAILTLS_IMPLICIT );
        p.passwordPolicy = MAILPASSWORD_APP_PASSWORD;
        p.oauthProvider = "google";
        p.oauthScopes.push_back( "https://mail.google.com/" );
        p.passwordHelp = "Gmail does not accept your usual Google password in a mail program. Turn on 2-step verification for your Google account, then make an app password in the security settings of the account and type that password below.";
        p.helpUrl = "https://support.google.com/accounts/answer/185833";
        Add( p );
    }
    {
        static const char* const domains[] = { "outlook.com", "hotmail.com", "live.com", "msn.com", "outlook.de", "hotmail.de", "hotmail.co.uk", "live.co.uk", "outlook.fr", "hotmail.fr",
                                               "live.nl", "hotmail.nl", "outlook.es", "hotmail.es", "outlook.it", "hotmail.it", "outlook.com.au", "hotmail.com.au", "live.ca", "hotmail.ca", GUCEF_NULL };
        CEMailProviderPreset p = MakePreset( "microsoft", "Outlook, Hotmail, Live and Microsoft 365" );
        AddDomains( p, domains );
        SetServer( p.imap, "outlook.office365.com", 993, MAILTLS_IMPLICIT );
        SetServer( p.smtp, "smtp.office365.com", 587, MAILTLS_STARTTLS );
        SetServer( p.pop3, "outlook.office365.com", 995, MAILTLS_IMPLICIT );
        p.passwordPolicy = MAILPASSWORD_MAY_BE_REFUSED;
        p.oauthProvider = "microsoft";
        p.oauthScopes.push_back( "https://outlook.office.com/IMAP.AccessAsUser.All" );
        p.oauthScopes.push_back( "https://outlook.office.com/SMTP.Send" );
        p.oauthScopes.push_back( "offline_access" );
        p.passwordHelp = "Outlook, Hotmail, Live and Microsoft 365 turned off password sign-in for mail programs on many accounts. If the test says the login was refused, this account only accepts the sign-in with Microsoft. A work or school address has a domain of its own: pick this provider from the list.";
        Add( p );
    }
    {
        static const char* const domains[] = { "yahoo.com", "ymail.com", "rocketmail.com", "yahoo.co.uk", "yahoo.de", "yahoo.fr", "yahoo.ca", "yahoo.com.au", "yahoo.es", "yahoo.it", "yahoo.in", GUCEF_NULL };
        CEMailProviderPreset p = MakePreset( "yahoo", "Yahoo Mail" );
        AddDomains( p, domains );
        SetServer( p.imap, "imap.mail.yahoo.com", 993, MAILTLS_IMPLICIT );
        SetServer( p.smtp, "smtp.mail.yahoo.com", 465, MAILTLS_IMPLICIT );
        SetServer( p.pop3, "pop.mail.yahoo.com", 995, MAILTLS_IMPLICIT );
        p.passwordPolicy = MAILPASSWORD_APP_PASSWORD;
        p.passwordHelp = "Yahoo needs an app password for mail programs, not your usual password. Open the account security page of your Yahoo account, make an app password for the game and type that password below.";
        p.helpUrl = "https://help.yahoo.com/kb/SLN15241.html";
        Add( p );
    }
    {
        static const char* const domains[] = { "icloud.com", "me.com", "mac.com", GUCEF_NULL };
        CEMailProviderPreset p = MakePreset( "icloud", "iCloud Mail" );
        AddDomains( p, domains );
        SetServer( p.imap, "imap.mail.me.com", 993, MAILTLS_IMPLICIT );
        SetServer( p.smtp, "smtp.mail.me.com", 587, MAILTLS_STARTTLS );
        p.passwordPolicy = MAILPASSWORD_APP_PASSWORD;
        p.passwordHelp = "iCloud needs an app password (Apple calls it an app-specific password) for mail programs, not your Apple account password. Turn on two-factor authentication for your Apple account, make an app password in its sign-in and security page and type that password below.";
        p.helpUrl = "https://support.apple.com/102654";
        Add( p );
    }
    {
        static const char* const domains[] = { "gmx.net", "gmx.de", "gmx.at", "gmx.ch", GUCEF_NULL };
        CEMailProviderPreset p = MakePreset( "gmx", "GMX" );
        AddDomains( p, domains );
        SetServer( p.imap, "imap.gmx.net", 993, MAILTLS_IMPLICIT );
        SetServer( p.smtp, "mail.gmx.net", 587, MAILTLS_STARTTLS );
        SetServer( p.pop3, "pop.gmx.net", 995, MAILTLS_IMPLICIT );
        p.passwordHelp = "GMX lets mail programs in only after you allow it: in the settings of your GMX mailbox turn on the access by POP3 and IMAP, then use your usual GMX password.";
        Add( p );
    }
    {
        static const char* const domains[] = { "web.de", GUCEF_NULL };
        CEMailProviderPreset p = MakePreset( "webde", "Web.de" );
        AddDomains( p, domains );
        SetServer( p.imap, "imap.web.de", 993, MAILTLS_IMPLICIT );
        SetServer( p.smtp, "smtp.web.de", 587, MAILTLS_STARTTLS );
        SetServer( p.pop3, "pop3.web.de", 995, MAILTLS_IMPLICIT );
        p.passwordHelp = "Web.de lets mail programs in only after you allow it: in the settings of your Web.de mailbox turn on the access by POP3 and IMAP, then use your usual Web.de password.";
        Add( p );
    }
    {
        static const char* const domains[] = { "fastmail.com", "fastmail.fm", GUCEF_NULL };
        CEMailProviderPreset p = MakePreset( "fastmail", "Fastmail" );
        AddDomains( p, domains );
        SetServer( p.imap, "imap.fastmail.com", 993, MAILTLS_IMPLICIT );
        SetServer( p.smtp, "smtp.fastmail.com", 465, MAILTLS_IMPLICIT );
        SetServer( p.pop3, "pop.fastmail.com", 995, MAILTLS_IMPLICIT );
        p.passwordPolicy = MAILPASSWORD_APP_PASSWORD;
        p.passwordHelp = "Fastmail needs an app password for mail programs, not your usual password. In Fastmail open the settings for privacy and security, make a new app password that may use mail (IMAP, POP and SMTP) and type that password below.";
        Add( p );
    }
    Add( MakeCustomPreset() );
}

/*-------------------------------------------------------------------------*/

bool
CEMailProviderPresets::Add( const CEMailProviderPreset& preset, CORE::CString* reason )
{GUCEF_TRACE;

    CORE::CString why;
    if ( !preset.IsValid( why ) )
    {
        if ( GUCEF_NULL != reason )
            *reason = why;
        return false;
    }
    for ( size_t i=0; i<m_presets.size(); ++i )
        if ( m_presets[ i ].id == preset.id )
        {
            m_presets[ i ] = preset;
            return true;
        }
    if ( m_presets.size() >= MaxEntries )
    {
        if ( GUCEF_NULL != reason )
            *reason = "The table of mail providers is full.";
        return false;
    }
    m_presets.push_back( preset );
    return true;
}

bool
CEMailProviderPresets::Remove( const CORE::CString& id )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_presets.size(); ++i )
        if ( m_presets[ i ].id == id )
        {
            m_presets.erase( m_presets.begin() + i );
            return true;
        }
    return false;
}

UInt32
CEMailProviderPresets::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_presets.size();
}

const CEMailProviderPreset*
CEMailProviderPresets::GetPreset( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_presets.size() ? &m_presets[ index ] : GUCEF_NULL;
}

const CEMailProviderPreset*
CEMailProviderPresets::FindById( const CORE::CString& id ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_presets.size(); ++i )
        if ( m_presets[ i ].id == id )
            return &m_presets[ i ];
    return GUCEF_NULL;
}

const CEMailProviderPreset&
CEMailProviderPresets::GetCustomPreset( void ) const
{GUCEF_TRACE;

    const CEMailProviderPreset* custom = FindById( CustomId );
    return GUCEF_NULL != custom ? *custom : m_fallbackCustom;
}

UInt32
CEMailProviderPresets::GetSkippedCount( void ) const
{GUCEF_TRACE;

    return m_skipped;
}

CEMailProviderPresets::ERecognition
CEMailProviderPresets::Recognize( const CORE::CString& address, const CEMailProviderPreset*& preset ) const
{GUCEF_TRACE;

    preset = &GetCustomPreset();
    const CORE::CString domain = CEMailSetupValidation::GetDomain( address );
    if ( domain.IsNULLOrEmpty() )
        return RECOGNIZE_INVALID_ADDRESS;
    for ( size_t i=0; i<m_presets.size(); ++i )
        if ( m_presets[ i ].MatchesDomain( domain ) )
        {
            preset = &m_presets[ i ];
            return RECOGNIZE_KNOWN;
        }
    return RECOGNIZE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

bool
CEMailProviderPresets::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_presets.size(); ++i )
    {
        CORE::CDataNode* node = config.AddChild( m_presets[ i ].id );
        if ( GUCEF_NULL == node || !m_presets[ i ].SaveConfig( *node ) )
            return false;
    }
    return true;
}

bool
CEMailProviderPresets::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_skipped = 0;
    if ( config.GetAttributeValueOrChildValueByName( "replaceDefaults" ).AsBool( false, true ) )
        m_presets.clear();

    CORE::CDataNode::const_iterator i = config.ConstBegin();
    while ( i != config.ConstEnd() )
    {
        const CORE::CDataNode& node = *(*i);
        ++i;

        // an entry has attributes or children of its own; the children of a list value ("domains") are not entries of the table
        CEMailProviderPreset preset;
        CORE::CString id = node.GetAttributeValueOrChildValueByName( "id" ).AsString( node.GetName(), true );
        if ( node.GetAttributeValueOrChildValueByName( "remove" ).AsBool( false, true ) )
        {
            Remove( id );
            continue;
        }
        const CEMailProviderPreset* existing = FindById( id );
        if ( GUCEF_NULL != existing )
            preset = *existing;
        CORE::CString reason;
        if ( !preset.LoadConfig( node ) || !Add( preset, &reason ) )
        {
            ++m_skipped;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MailProviderPresets: An entry of the table of mail providers is not usable and was left out" );
        }
    }
    return true;
}

const CORE::CString&
CEMailProviderPresets::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CEMailProviderPresets";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailProviderPresets::GetOAuthDisplayName( const CORE::CString& oauthProvider )
{GUCEF_TRACE;

    if ( "google" == oauthProvider )
        return "Google";
    if ( "microsoft" == oauthProvider )
        return "Microsoft";
    if ( oauthProvider.IsNULLOrEmpty() )
        return oauthProvider;
    const UInt32 length = oauthProvider.ByteSize() - 1;
    GUCEF::vector< char > chars( oauthProvider.C_String(), oauthProvider.C_String() + length );
    if ( chars[ 0 ] >= 'a' && chars[ 0 ] <= 'z' )
        chars[ 0 ] = (char) ( chars[ 0 ] - 'a' + 'A' );
    return CORE::CString( &chars[ 0 ], length );
}

CORE::CString
CEMailProviderPresets::DescribeSignIn( const CEMailProviderPreset& preset, bool oauthAvailable )
{GUCEF_TRACE;

    CORE::CString text;
    if ( preset.serverFieldsEditable && preset.imap.host.IsNULLOrEmpty() )
    {
        text = "Your mail provider is not known to the game, so fill in its servers on the page Servers and options: the incoming server (IMAP or POP3) and the outgoing server (SMTP), with SSL/TLS or STARTTLS. ";
        text += preset.passwordHelp;
        return text;
    }
    if ( MAILPASSWORD_NORMAL == preset.passwordPolicy )
        text = "Sign in with the user name and the password of your mailbox. ";
    text += preset.passwordHelp;
    if ( !preset.helpUrl.IsNULLOrEmpty() )
        text += " More about this: " + preset.helpUrl + ".";
    if ( preset.HasOAuth() )
    {
        const CORE::CString name = GetOAuthDisplayName( preset.oauthProvider );
        if ( oauthAvailable )
            text += " Or press Sign in with " + name + ": your browser opens, " + name + " asks you to allow the game to use your mail, and the game never sees your password.";
        else
        if ( MAILPASSWORD_APP_PASSWORD == preset.passwordPolicy )
            text += " Sign-in with " + name + " is not available in this build. Use an app password.";
        else
            text += " Sign-in with " + name + " is not available in this build, so a mailbox that refuses passwords cannot be used here. Use a mailbox of another provider, or one that takes a password.";
    }
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
