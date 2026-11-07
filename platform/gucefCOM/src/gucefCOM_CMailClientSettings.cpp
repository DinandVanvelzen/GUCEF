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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCOM_CMailClientSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailClientSettings::CMailClientSettings( void )
    : CORE::CIConfigurable()
    , host()
    , port( 0 )
    , tlsMode( MAILTLS_STARTTLS )
    , tls()
    , userName()
    , password()
    , passwordIsToken( false )
    , authMechanisms()
    , allowInsecureAuth( false )
    , connectTimeoutInMs( GUCEF_COM_MAIL_DEFAULT_CONNECT_TIMEOUT_MS )
    , commandTimeoutInMs( GUCEF_COM_MAIL_DEFAULT_COMMAND_TIMEOUT_MS )
    , idleTimeoutInMs( 0 )
    , maxLineLength( GUCEF_COM_MAIL_DEFAULT_MAX_LINE_LENGTH )
    , maxLiteralSize( GUCEF_COM_MAIL_DEFAULT_MAX_LITERAL_SIZE )
    , maxResponseSize( GUCEF_COM_MAIL_DEFAULT_MAX_RESPONSE_SIZE )
    , maxResponseCount( GUCEF_COM_MAIL_DEFAULT_MAX_RESPONSE_COUNT )
    , maxMessageSize( GUCEF_COM_MAIL_DEFAULT_MAX_MESSAGE_SIZE )
    , saveSecrets( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailClientSettings::CMailClientSettings( const CMailClientSettings& src )
    : CORE::CIConfigurable( src )
    , host( src.host )
    , port( src.port )
    , tlsMode( src.tlsMode )
    , tls( src.tls )
    , userName( src.userName )
    , password( src.password )
    , passwordIsToken( src.passwordIsToken )
    , authMechanisms( src.authMechanisms )
    , allowInsecureAuth( src.allowInsecureAuth )
    , connectTimeoutInMs( src.connectTimeoutInMs )
    , commandTimeoutInMs( src.commandTimeoutInMs )
    , idleTimeoutInMs( src.idleTimeoutInMs )
    , maxLineLength( src.maxLineLength )
    , maxLiteralSize( src.maxLiteralSize )
    , maxResponseSize( src.maxResponseSize )
    , maxResponseCount( src.maxResponseCount )
    , maxMessageSize( src.maxMessageSize )
    , saveSecrets( src.saveSecrets )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailClientSettings::~CMailClientSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailClientSettings&
CMailClientSettings::operator=( const CMailClientSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        host = src.host;
        port = src.port;
        tlsMode = src.tlsMode;
        tls = src.tls;
        userName = src.userName;
        password = src.password;
        passwordIsToken = src.passwordIsToken;
        authMechanisms = src.authMechanisms;
        allowInsecureAuth = src.allowInsecureAuth;
        connectTimeoutInMs = src.connectTimeoutInMs;
        commandTimeoutInMs = src.commandTimeoutInMs;
        idleTimeoutInMs = src.idleTimeoutInMs;
        maxLineLength = src.maxLineLength;
        maxLiteralSize = src.maxLiteralSize;
        maxResponseSize = src.maxResponseSize;
        maxResponseCount = src.maxResponseCount;
        maxMessageSize = src.maxMessageSize;
        saveSecrets = src.saveSecrets;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt16
CMailClientSettings::GetEffectivePort( void ) const
{GUCEF_TRACE;

    return 0 != port ? port : GetDefaultPort( tlsMode );
}

/*-------------------------------------------------------------------------*/

void
CMailClientSettings::GetPreferredMechanisms( CORE::CString::StringVector& mechanisms ) const
{GUCEF_TRACE;

    mechanisms.clear();
    if ( !authMechanisms.empty() )
    {
        mechanisms = authMechanisms;
        return;
    }
    if ( passwordIsToken )
    {
        mechanisms.push_back( "XOAUTH2" );
        return;
    }
    mechanisms.push_back( "SCRAM-SHA-256" );
    mechanisms.push_back( "PLAIN" );
    mechanisms.push_back( "LOGIN" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailClientSettings::TlsModeToString( EMailTlsMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case MAILTLS_NONE: return "none";
        case MAILTLS_STARTTLS: return "starttls";
        case MAILTLS_STARTTLS_IF_OFFERED: return "starttls-if-offered";
        case MAILTLS_IMPLICIT: return "implicit";
    }
    return "starttls";
}

/*-------------------------------------------------------------------------*/

bool
CMailClientSettings::TlsModeFromString( const CORE::CString& text, EMailTlsMode& mode )
{GUCEF_TRACE;

    const CORE::CString lower = text.Lowercase().Trim( true ).Trim( false );
    if ( lower == "none" ) { mode = MAILTLS_NONE; return true; }
    if ( lower == "starttls" ) { mode = MAILTLS_STARTTLS; return true; }
    if ( lower == "starttls-if-offered" ) { mode = MAILTLS_STARTTLS_IF_OFFERED; return true; }
    if ( lower == "implicit" || lower == "tls" || lower == "ssl" ) { mode = MAILTLS_IMPLICIT; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailClientSettings::JoinList( const CORE::CString::StringVector& list )
{GUCEF_TRACE;

    CORE::CString joined;
    for ( size_t i=0; i<list.size(); ++i )
    {
        if ( i > 0 )
            joined += ',';
        joined += list[ i ];
    }
    return joined;
}

/*-------------------------------------------------------------------------*/

void
CMailClientSettings::ReadList( const CORE::CDataNode& config, const char* name, CORE::CString::StringVector& list )
{GUCEF_TRACE;

    const CORE::CVariant& value = config.GetAttributeValueOrChildValueByName( name );
    if ( value.IsNULLOrEmpty() )
        return;

    list.clear();
    CORE::CString::StringVector elements = value.AsString( CORE::CString::Empty, true ).ParseElements( ',', false );
    for ( size_t i=0; i<elements.size(); ++i )
    {
        CORE::CString element = elements[ i ].Trim( true ).Trim( false );
        if ( !element.IsNULLOrEmpty() )
            list.push_back( element );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailClientSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "host", host );
    config.SetAttribute( "port", port );
    config.SetAttribute( "tlsMode", TlsModeToString( tlsMode ) );
    config.SetAttribute( "userName", userName );
    if ( saveSecrets )
        config.SetAttribute( "password", password );
    config.SetAttribute( "passwordIsToken", passwordIsToken );
    config.SetAttribute( "authMechanisms", JoinList( authMechanisms ) );
    config.SetAttribute( "allowInsecureAuth", allowInsecureAuth );
    config.SetAttribute( "connectTimeoutInMs", connectTimeoutInMs );
    config.SetAttribute( "commandTimeoutInMs", commandTimeoutInMs );
    config.SetAttribute( "idleTimeoutInMs", idleTimeoutInMs );
    config.SetAttribute( "maxLineLength", maxLineLength );
    config.SetAttribute( "maxLiteralSize", maxLiteralSize );
    config.SetAttribute( "maxResponseSize", maxResponseSize );
    config.SetAttribute( "maxResponseCount", maxResponseCount );
    config.SetAttribute( "maxMessageSize", maxMessageSize );

    CORE::CDataNode* tlsNode = config.FindOrAddChild( "Tls" );
    return GUCEF_NULL != tlsNode && tls.SaveConfig( *tlsNode );
}

/*-------------------------------------------------------------------------*/

bool
CMailClientSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    host = config.GetAttributeValueOrChildValueByName( "host" ).AsString( host, true );
    port = config.GetAttributeValueOrChildValueByName( "port" ).AsUInt16( port, true );

    const CORE::CVariant& modeValue = config.GetAttributeValueOrChildValueByName( "tlsMode" );
    if ( !modeValue.IsNULLOrEmpty() )
    {
        EMailTlsMode mode = tlsMode;
        if ( !TlsModeFromString( modeValue.AsString( CORE::CString::Empty, true ), mode ) )
            return false;
        tlsMode = mode;
    }

    const CORE::CDataNode* tlsNode = config.FindChild( "Tls" );
    if ( GUCEF_NULL != tlsNode )
        tls.LoadConfig( *tlsNode );

    userName = config.GetAttributeValueOrChildValueByName( "userName" ).AsString( userName, true );
    password = config.GetAttributeValueOrChildValueByName( "password" ).AsString( password, true );
    passwordIsToken = config.GetAttributeValueOrChildValueByName( "passwordIsToken" ).AsBool( passwordIsToken, true );
    ReadList( config, "authMechanisms", authMechanisms );
    allowInsecureAuth = config.GetAttributeValueOrChildValueByName( "allowInsecureAuth" ).AsBool( allowInsecureAuth, true );
    connectTimeoutInMs = config.GetAttributeValueOrChildValueByName( "connectTimeoutInMs" ).AsUInt32( connectTimeoutInMs, true );
    commandTimeoutInMs = config.GetAttributeValueOrChildValueByName( "commandTimeoutInMs" ).AsUInt32( commandTimeoutInMs, true );
    idleTimeoutInMs = config.GetAttributeValueOrChildValueByName( "idleTimeoutInMs" ).AsUInt32( idleTimeoutInMs, true );
    maxLineLength = config.GetAttributeValueOrChildValueByName( "maxLineLength" ).AsUInt32( maxLineLength, true );
    maxLiteralSize = config.GetAttributeValueOrChildValueByName( "maxLiteralSize" ).AsUInt32( maxLiteralSize, true );
    maxResponseSize = config.GetAttributeValueOrChildValueByName( "maxResponseSize" ).AsUInt32( maxResponseSize, true );
    maxResponseCount = config.GetAttributeValueOrChildValueByName( "maxResponseCount" ).AsUInt32( maxResponseCount, true );
    maxMessageSize = config.GetAttributeValueOrChildValueByName( "maxMessageSize" ).AsUInt32( maxMessageSize, true );
    return !host.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
