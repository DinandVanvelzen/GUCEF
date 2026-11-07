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


#include <string.h>

#include "gucefCOM_CMailProviderPreset.h"

#ifndef GUCEF_COM_CMAILCLIENTSETTINGS_H
#include "gucefCOM_CMailClientSettings.h"
#define GUCEF_COM_CMAILCLIENTSETTINGS_H
#endif /* GUCEF_COM_CMAILCLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CMAILSETUPVALIDATION_H
#include "gucefCOM_CMailSetupValidation.h"
#define GUCEF_COM_CMAILSETUPVALIDATION_H
#endif /* GUCEF_COM_CMAILSETUPVALIDATION_H ? */

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

const UInt32 MaxIdLength = 32;
const UInt32 MaxTextLength = 1200;
const UInt32 MaxUrlLength = 200;
const UInt32 MaxScopes = 16;
const UInt32 MaxScopeLength = 200;

bool
IsIdText( const CORE::CString& id, bool allowHyphen )
{
    const UInt32 size = id.ByteSize();
    if ( size < 2 || size - 1 > MaxIdLength )
        return false;
    const char* p = id.C_String();
    for ( UInt32 i=0; i+1<size; ++i )
    {
        const char c = p[ i ];
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || ( allowHyphen && '-' == c && i > 0 ) ) )
            return false;
    }
    return true;
}

bool
IsServerValid( const CMailServerPreset& server )
{
    return !server.IsSet() || CMailSetupValidation::IsValidHostName( server.host );
}

/** Reads one server of an entry: the three attributes <prefix>Host, <prefix>Port and <prefix>Security. False for a value that is not usable */
bool
ReadServer( const CORE::CDataNode& node, const char* prefix, CMailServerPreset& server )
{
    const CORE::CString hostName = CORE::CString( prefix ) + "Host";
    const CORE::CString portName = CORE::CString( prefix ) + "Port";
    const CORE::CString securityName = CORE::CString( prefix ) + "Security";
    server.host = node.GetAttributeValueOrChildValueByName( hostName ).AsString( server.host, true );
    const UInt32 port = node.GetAttributeValueOrChildValueByName( portName ).AsUInt32( server.port, true );
    if ( port > 65535 )
        return false;
    server.port = (UInt16) port;
    const CORE::CVariant& security = node.GetAttributeValueOrChildValueByName( securityName );
    if ( !security.IsNULLOrEmpty() )
    {
        CORE::CString text = security.AsString( CORE::CString::Empty, true ).Lowercase();
        if ( "implicit" == text || "tls" == text )
            server.security = MAILTLS_IMPLICIT;
        else
        if ( "starttls" == text )
            server.security = MAILTLS_STARTTLS;
        else
        if ( "starttls-if-offered" == text )
            server.security = MAILTLS_STARTTLS_IF_OFFERED;
        else
        if ( "none" == text )
            server.security = MAILTLS_NONE;
        else
            return false;
    }
    return true;
}

void
WriteServer( CORE::CDataNode& node, const char* prefix, const CMailServerPreset& server )
{
    if ( !server.IsSet() )
        return;
    node.SetAttribute( CORE::CString( prefix ) + "Host", server.host );
    node.SetAttribute( CORE::CString( prefix ) + "Port", server.port );
    node.SetAttribute( CORE::CString( prefix ) + "Security", CMailClientSettings::TlsModeToString( server.security ) );
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailServerPreset::CMailServerPreset( void )
    : host()
    , port( 0 )
    , security( MAILTLS_IMPLICIT )
{GUCEF_TRACE;

}

bool
CMailServerPreset::IsSet( void ) const
{GUCEF_TRACE;

    return !host.IsNULLOrEmpty();
}

UInt16
CMailServerPreset::GetEffectivePort( EMailProtocol protocol ) const
{GUCEF_TRACE;

    if ( 0 != port )
        return port;
    const bool implicit = MAILTLS_IMPLICIT == security;
    switch ( protocol )
    {
        case MAILPROTOCOL_IMAP: return implicit ? 993 : 143;
        case MAILPROTOCOL_POP3: return implicit ? 995 : 110;
        default: return implicit ? 465 : ( MAILTLS_NONE == security ? 25 : 587 );
    }
}

/*-------------------------------------------------------------------------*/

CMailProviderPreset::CMailProviderPreset( void )
    : id()
    , displayName()
    , domains()
    , imap()
    , smtp()
    , pop3()
    , passwordPolicy( MAILPASSWORD_NORMAL )
    , oauthProvider()
    , oauthScopes()
    , serverFieldsEditable( false )
    , passwordHelp()
    , helpUrl()
{GUCEF_TRACE;

}

bool
CMailProviderPreset::HasOAuth( void ) const
{GUCEF_TRACE;

    return !oauthProvider.IsNULLOrEmpty();
}

bool
CMailProviderPreset::MatchesDomain( const CORE::CString& domain ) const
{GUCEF_TRACE;

    const UInt32 size = domain.ByteSize();
    if ( size < 2 )
        return false;
    const UInt32 length = size - 1;
    for ( size_t i=0; i<domains.size(); ++i )
    {
        const UInt32 own = domains[ i ].ByteSize() - 1;
        if ( 0 == own || own > length )
            continue;
        const char* tail = domain.C_String() + ( length - own );
        if ( 0 != memcmp( tail, domains[ i ].C_String(), own ) )
            continue;
        if ( own == length || '.' == domain.C_String()[ length - own - 1 ] )
            return true;
    }
    return false;
}

bool
CMailProviderPreset::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !IsIdText( id, false ) )
    {
        reason = "The id of a provider is 1 to 32 lower case letters and digits.";
        return false;
    }
    if ( displayName.IsNULLOrEmpty() || !CMailSetupValidation::IsPlainText( displayName, 100 ) )
    {
        reason = "The name of a provider is 1 to 100 characters without line breaks.";
        return false;
    }
    if ( domains.size() > 256 )
    {
        reason = "A provider has at most 256 domains.";
        return false;
    }
    for ( size_t i=0; i<domains.size(); ++i )
    {
        CORE::CString local, domain;
        if ( !CMailSetupValidation::SplitAddress( CORE::CString( "x@" ) + domains[ i ], local, domain ) || domain != domains[ i ] )
        {
            reason = "A domain of a provider is not a lower case domain name.";
            return false;
        }
    }
    if ( !IsServerValid( imap ) || !IsServerValid( smtp ) || !IsServerValid( pop3 ) )
    {
        reason = "The name of a server of a provider is not a host name.";
        return false;
    }
    if ( !serverFieldsEditable && ( !smtp.IsSet() || ( !imap.IsSet() && !pop3.IsSet() ) ) )
    {
        reason = "A provider needs an outgoing server and an incoming server, unless the person fills them in.";
        return false;
    }
    if ( !CMailSetupValidation::IsPlainText( passwordHelp, MaxTextLength ) )
    {
        reason = "The help text of a provider is too long or has control characters.";
        return false;
    }
    if ( !helpUrl.IsNULLOrEmpty() && ( !CMailSetupValidation::IsPlainText( helpUrl, MaxUrlLength ) || 0 != strncmp( helpUrl.C_String(), "https://", 8 ) || helpUrl.HasSubstr( " " ) >= 0 ) )
    {
        reason = "The help address of a provider is an https address without spaces.";
        return false;
    }
    if ( !oauthProvider.IsNULLOrEmpty() && !IsIdText( oauthProvider, true ) )
    {
        reason = "The OAuth provider of a provider is a short name.";
        return false;
    }
    if ( oauthScopes.size() > MaxScopes )
    {
        reason = "A provider has at most 16 scopes.";
        return false;
    }
    for ( size_t i=0; i<oauthScopes.size(); ++i )
        if ( oauthScopes[ i ].IsNULLOrEmpty() || !CMailSetupValidation::IsPlainText( oauthScopes[ i ], MaxScopeLength ) || oauthScopes[ i ].HasSubstr( " " ) >= 0 )
        {
            reason = "A scope of a provider is a word without spaces.";
            return false;
        }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailProviderPreset::PasswordPolicyToString( EMailPasswordPolicy policy )
{GUCEF_TRACE;

    switch ( policy )
    {
        case MAILPASSWORD_APP_PASSWORD: return "app";
        case MAILPASSWORD_MAY_BE_REFUSED: return "mayBeRefused";
        default: return "normal";
    }
}

bool
CMailProviderPreset::PasswordPolicyFromString( const CORE::CString& text, EMailPasswordPolicy& policy )
{GUCEF_TRACE;

    if ( "normal" == text )
        policy = MAILPASSWORD_NORMAL;
    else
    if ( "app" == text )
        policy = MAILPASSWORD_APP_PASSWORD;
    else
    if ( "mayBeRefused" == text )
        policy = MAILPASSWORD_MAY_BE_REFUSED;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailProviderPreset::SaveConfig( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "id", id );
    node.SetAttribute( "displayName", displayName );
    node.SetValuesOfChildByName( "domains", domains );
    WriteServer( node, "imap", imap );
    WriteServer( node, "smtp", smtp );
    WriteServer( node, "pop3", pop3 );
    node.SetAttribute( "passwordPolicy", PasswordPolicyToString( passwordPolicy ) );
    node.SetAttribute( "oauthProvider", oauthProvider );
    if ( !oauthScopes.empty() )
        node.SetValuesOfChildByName( "oauthScopes", oauthScopes );
    node.SetAttribute( "serverFieldsEditable", serverFieldsEditable );
    node.SetAttribute( "passwordHelp", passwordHelp );
    node.SetAttribute( "helpUrl", helpUrl );
    return true;
}

bool
CMailProviderPreset::LoadConfig( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CMailProviderPreset read = *this;
    read.id = node.GetAttributeValueOrChildValueByName( "id" ).AsString( read.id.IsNULLOrEmpty() ? node.GetName() : read.id, true );
    read.displayName = node.GetAttributeValueOrChildValueByName( "displayName" ).AsString( read.displayName, true );
    CORE::CString::StringVector list;
    if ( node.GetStringListByName( "domains", list ) )
    {
        read.domains.clear();
        for ( size_t i=0; i<list.size(); ++i )
            read.domains.push_back( list[ i ].Lowercase() );
    }
    if ( !ReadServer( node, "imap", read.imap ) || !ReadServer( node, "smtp", read.smtp ) || !ReadServer( node, "pop3", read.pop3 ) )
        return false;
    const CORE::CVariant& policy = node.GetAttributeValueOrChildValueByName( "passwordPolicy" );
    if ( !policy.IsNULLOrEmpty() && !PasswordPolicyFromString( policy.AsString( CORE::CString::Empty, true ), read.passwordPolicy ) )
        return false;
    read.oauthProvider = node.GetAttributeValueOrChildValueByName( "oauthProvider" ).AsString( read.oauthProvider, true );
    if ( node.GetStringListByName( "oauthScopes", list ) )
        read.oauthScopes = list;
    read.serverFieldsEditable = node.GetAttributeValueOrChildValueByName( "serverFieldsEditable" ).AsBool( read.serverFieldsEditable, true );
    read.passwordHelp = node.GetAttributeValueOrChildValueByName( "passwordHelp" ).AsString( read.passwordHelp, true );
    read.helpUrl = node.GetAttributeValueOrChildValueByName( "helpUrl" ).AsString( read.helpUrl, true );
    if ( read.displayName.IsNULLOrEmpty() )
        read.displayName = read.id;
    CORE::CString reason;
    if ( !read.IsValid( reason ) )
        return false;
    *this = read;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
