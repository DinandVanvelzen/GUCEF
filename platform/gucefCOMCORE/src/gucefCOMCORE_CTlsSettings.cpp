/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOMCORE_CTlsSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char*
VersionToString( ETlsVersion version )
{GUCEF_TRACE;

    switch ( version )
    {
        case TLSVERSION_1_2: return "1.2";
        case TLSVERSION_1_3: return "1.3";
        default: return "any";
    }
}

/*-------------------------------------------------------------------------*/

ETlsVersion
VersionFromString( const CORE::CString& text, ETlsVersion defaultVersion )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return defaultVersion;
    if ( text == "1.3" || text == "TLSv1.3" )
        return TLSVERSION_1_3;
    if ( text == "any" )
        return TLSVERSION_ANY;

    // Anything else, including 1.0, 1.1 and what we do not know, is the floor
    return TLSVERSION_1_2;
}

/*-------------------------------------------------------------------------*/

CORE::CString
JoinList( const CTlsSettings::TStringVector& list )
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

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTlsSettings::CTlsSettings( void )
    : CORE::CIConfigurable()
    , m_minVersion( TLSVERSION_1_2 )
    , m_maxVersion( TLSVERSION_ANY )
    , m_verify( true )
    , m_useSystemTrust( true )
    , m_caBundlePath()
    , m_caDir()
    , m_caBundleData()
    , m_serverName()
    , m_alpnProtocols()
    , m_handshakeTimeoutInMs( DefaultHandshakeTimeoutInMs )
    , m_allowResumption( true )
    , m_sessionCacheSize( DefaultSessionCacheSize )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsSettings::CTlsSettings( const CTlsSettings& src )
    : CORE::CIConfigurable( src )
    , m_minVersion( src.m_minVersion )
    , m_maxVersion( src.m_maxVersion )
    , m_verify( src.m_verify )
    , m_useSystemTrust( src.m_useSystemTrust )
    , m_caBundlePath( src.m_caBundlePath )
    , m_caDir( src.m_caDir )
    , m_caBundleData( src.m_caBundleData )
    , m_serverName( src.m_serverName )
    , m_alpnProtocols( src.m_alpnProtocols )
    , m_handshakeTimeoutInMs( src.m_handshakeTimeoutInMs )
    , m_allowResumption( src.m_allowResumption )
    , m_sessionCacheSize( src.m_sessionCacheSize )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsSettings::~CTlsSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsSettings&
CTlsSettings::operator=( const CTlsSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_minVersion = src.m_minVersion;
        m_maxVersion = src.m_maxVersion;
        m_verify = src.m_verify;
        m_useSystemTrust = src.m_useSystemTrust;
        m_caBundlePath = src.m_caBundlePath;
        m_caDir = src.m_caDir;
        m_caBundleData = src.m_caBundleData;
        m_serverName = src.m_serverName;
        m_alpnProtocols = src.m_alpnProtocols;
        m_handshakeTimeoutInMs = src.m_handshakeTimeoutInMs;
        m_allowResumption = src.m_allowResumption;
        m_sessionCacheSize = src.m_sessionCacheSize;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetMinVersion( ETlsVersion version )
{GUCEF_TRACE;

    // The floor: nothing below TLS 1.2 exists as a value, and anything else than 1.3 is 1.2
    m_minVersion = TLSVERSION_1_3 == version ? TLSVERSION_1_3 : TLSVERSION_1_2;
    if ( TLSVERSION_ANY != m_maxVersion && m_maxVersion < m_minVersion )
        m_maxVersion = m_minVersion;
}

/*-------------------------------------------------------------------------*/

ETlsVersion
CTlsSettings::GetMinVersion( void ) const
{GUCEF_TRACE;

    return m_minVersion;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetMaxVersion( ETlsVersion version )
{GUCEF_TRACE;

    if ( TLSVERSION_ANY == version )
        m_maxVersion = TLSVERSION_ANY;
    else
        m_maxVersion = version < m_minVersion ? m_minVersion : version;
}

/*-------------------------------------------------------------------------*/

ETlsVersion
CTlsSettings::GetMaxVersion( void ) const
{GUCEF_TRACE;

    return m_maxVersion;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::DisableVerificationInsecurely( void )
{GUCEF_TRACE;

    m_verify = false;
}

/*-------------------------------------------------------------------------*/

bool
CTlsSettings::IsVerificationEnabled( void ) const
{GUCEF_TRACE;

    return m_verify;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetUseSystemTrust( bool useSystemTrust )
{GUCEF_TRACE;

    m_useSystemTrust = useSystemTrust;
}

/*-------------------------------------------------------------------------*/

bool
CTlsSettings::GetUseSystemTrust( void ) const
{GUCEF_TRACE;

    return m_useSystemTrust;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetCaBundlePath( const CORE::CString& path )
{GUCEF_TRACE;

    m_caBundlePath = path;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTlsSettings::GetCaBundlePath( void ) const
{GUCEF_TRACE;

    return m_caBundlePath;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetCaDir( const CORE::CString& path )
{GUCEF_TRACE;

    m_caDir = path;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTlsSettings::GetCaDir( void ) const
{GUCEF_TRACE;

    return m_caDir;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetCaBundleData( const CORE::CString& pem )
{GUCEF_TRACE;

    m_caBundleData = pem;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTlsSettings::GetCaBundleData( void ) const
{GUCEF_TRACE;

    return m_caBundleData;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetServerName( const CORE::CString& serverName )
{GUCEF_TRACE;

    m_serverName = serverName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTlsSettings::GetServerName( void ) const
{GUCEF_TRACE;

    return m_serverName;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetAlpnProtocols( const TStringVector& protocols )
{GUCEF_TRACE;

    m_alpnProtocols = protocols;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::AddAlpnProtocol( const CORE::CString& protocol )
{GUCEF_TRACE;

    if ( !protocol.IsNULLOrEmpty() )
        m_alpnProtocols.push_back( protocol );
}

/*-------------------------------------------------------------------------*/

const CTlsSettings::TStringVector&
CTlsSettings::GetAlpnProtocols( void ) const
{GUCEF_TRACE;

    return m_alpnProtocols;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetHandshakeTimeoutInMs( UInt32 timeoutInMs )
{GUCEF_TRACE;

    m_handshakeTimeoutInMs = timeoutInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CTlsSettings::GetHandshakeTimeoutInMs( void ) const
{GUCEF_TRACE;

    return m_handshakeTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetAllowResumption( bool allowResumption )
{GUCEF_TRACE;

    m_allowResumption = allowResumption;
}

/*-------------------------------------------------------------------------*/

bool
CTlsSettings::GetAllowResumption( void ) const
{GUCEF_TRACE;

    return m_allowResumption;
}

/*-------------------------------------------------------------------------*/

void
CTlsSettings::SetSessionCacheSize( UInt32 size )
{GUCEF_TRACE;

    m_sessionCacheSize = size;
}

/*-------------------------------------------------------------------------*/

UInt32
CTlsSettings::GetSessionCacheSize( void ) const
{GUCEF_TRACE;

    return m_sessionCacheSize;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTlsSettings::GetContextKey( void ) const
{GUCEF_TRACE;

    CORE::CString key = CORE::CString( VersionToString( m_minVersion ) ) + "|" + VersionToString( m_maxVersion ) + "|" + ( m_verify ? "1|" : "0|" ) +
                        ( m_useSystemTrust ? "1|" : "0|" ) + m_caBundlePath + "|" + m_caDir + "|" + m_caBundleData + "|" + JoinList( m_alpnProtocols ) + "|" +
                        ( m_allowResumption ? "1|" : "0|" ) + CORE::ToString( m_sessionCacheSize );
    return key;
}

/*-------------------------------------------------------------------------*/

bool
CTlsSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "minVersion", CORE::CString( VersionToString( m_minVersion ) ) );
    config.SetAttribute( "maxVersion", CORE::CString( VersionToString( m_maxVersion ) ) );
    config.SetAttribute( "verifyMode", CORE::CString( m_verify ? "full" : "none" ) );
    config.SetAttribute( "useSystemTrust", m_useSystemTrust );
    config.SetAttribute( "caBundlePath", m_caBundlePath );
    config.SetAttribute( "caDir", m_caDir );
    config.SetAttribute( "serverName", m_serverName );
    config.SetAttribute( "alpn", JoinList( m_alpnProtocols ) );
    config.SetAttribute( "handshakeTimeoutInMs", m_handshakeTimeoutInMs );
    config.SetAttribute( "allowResumption", m_allowResumption );
    config.SetAttribute( "sessionCacheSize", m_sessionCacheSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTlsSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    SetMinVersion( VersionFromString( config.GetAttributeValueOrChildValueByName( "minVersion" ).AsString( CORE::CString::Empty, true ), m_minVersion ) );
    SetMaxVersion( VersionFromString( config.GetAttributeValueOrChildValueByName( "maxVersion" ).AsString( CORE::CString::Empty, true ), m_maxVersion ) );

    // A configuration cannot turn verification off, only code that says so explicitly can
    CORE::CString verifyMode = config.GetAttributeValueOrChildValueByName( "verifyMode" ).AsString( CORE::CString::Empty, true );
    if ( !verifyMode.IsNULLOrEmpty() && verifyMode != "full" )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TlsSettings: The verifyMode \"" << verifyMode << "\" of the configuration is ignored, verification stays on. Only DisableVerificationInsecurely() turns it off" );

    m_useSystemTrust = config.GetAttributeValueOrChildValueByName( "useSystemTrust" ).AsBool( m_useSystemTrust, true );
    m_caBundlePath = config.GetAttributeValueOrChildValueByName( "caBundlePath" ).AsString( m_caBundlePath, true );
    m_caDir = config.GetAttributeValueOrChildValueByName( "caDir" ).AsString( m_caDir, true );
    m_serverName = config.GetAttributeValueOrChildValueByName( "serverName" ).AsString( m_serverName, true );
    m_handshakeTimeoutInMs = config.GetAttributeValueOrChildValueByName( "handshakeTimeoutInMs" ).AsUInt32( m_handshakeTimeoutInMs, true );
    m_allowResumption = config.GetAttributeValueOrChildValueByName( "allowResumption" ).AsBool( m_allowResumption, true );
    m_sessionCacheSize = config.GetAttributeValueOrChildValueByName( "sessionCacheSize" ).AsUInt32( m_sessionCacheSize, true );

    const CORE::CVariant& alpn = config.GetAttributeValueOrChildValueByName( "alpn" );
    if ( !alpn.IsNULLOrEmpty() )
    {
        m_alpnProtocols.clear();
        TStringVector elements = alpn.AsString( CORE::CString::Empty, true ).ParseElements( ',', false );
        for ( size_t i=0; i<elements.size(); ++i )
            AddAlpnProtocol( elements[ i ].Trim( true ).Trim( false ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTlsSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CTlsSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
