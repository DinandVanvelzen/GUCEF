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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOMCORE_CTlsServerSettings.h"

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
ServerVersionToString( ETlsVersion version )
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
ServerVersionFromString( const CORE::CString& text, ETlsVersion defaultVersion )
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
JoinServerList( const CTlsServerSettings::TStringVector& list )
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

CTlsServerSettings::CTlsServerSettings( void )
    : CORE::CIConfigurable()
    , m_certificateChainPath()
    , m_privateKeyPath()
    , m_minVersion( TLSVERSION_1_2 )
    , m_maxVersion( TLSVERSION_ANY )
    , m_alpnProtocols()
    , m_handshakeTimeoutInMs( DefaultHandshakeTimeoutInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsServerSettings::CTlsServerSettings( const CTlsServerSettings& src )
    : CORE::CIConfigurable( src )
    , m_certificateChainPath( src.m_certificateChainPath )
    , m_privateKeyPath( src.m_privateKeyPath )
    , m_minVersion( src.m_minVersion )
    , m_maxVersion( src.m_maxVersion )
    , m_alpnProtocols( src.m_alpnProtocols )
    , m_handshakeTimeoutInMs( src.m_handshakeTimeoutInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsServerSettings::~CTlsServerSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsServerSettings&
CTlsServerSettings::operator=( const CTlsServerSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_certificateChainPath = src.m_certificateChainPath;
        m_privateKeyPath = src.m_privateKeyPath;
        m_minVersion = src.m_minVersion;
        m_maxVersion = src.m_maxVersion;
        m_alpnProtocols = src.m_alpnProtocols;
        m_handshakeTimeoutInMs = src.m_handshakeTimeoutInMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CTlsServerSettings::IsEnabled( void ) const
{GUCEF_TRACE;

    return !m_certificateChainPath.IsNULLOrEmpty() && !m_privateKeyPath.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
CTlsServerSettings::SetCertificateChainPath( const CORE::CString& path )
{GUCEF_TRACE;

    m_certificateChainPath = path;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTlsServerSettings::GetCertificateChainPath( void ) const
{GUCEF_TRACE;

    return m_certificateChainPath;
}

/*-------------------------------------------------------------------------*/

void
CTlsServerSettings::SetPrivateKeyPath( const CORE::CString& path )
{GUCEF_TRACE;

    m_privateKeyPath = path;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTlsServerSettings::GetPrivateKeyPath( void ) const
{GUCEF_TRACE;

    return m_privateKeyPath;
}

/*-------------------------------------------------------------------------*/

void
CTlsServerSettings::SetMinVersion( ETlsVersion version )
{GUCEF_TRACE;

    // The floor: nothing below TLS 1.2 exists as a value, and anything else than 1.3 is 1.2
    m_minVersion = TLSVERSION_1_3 == version ? TLSVERSION_1_3 : TLSVERSION_1_2;
    if ( TLSVERSION_ANY != m_maxVersion && m_maxVersion < m_minVersion )
        m_maxVersion = m_minVersion;
}

/*-------------------------------------------------------------------------*/

ETlsVersion
CTlsServerSettings::GetMinVersion( void ) const
{GUCEF_TRACE;

    return m_minVersion;
}

/*-------------------------------------------------------------------------*/

void
CTlsServerSettings::SetMaxVersion( ETlsVersion version )
{GUCEF_TRACE;

    if ( TLSVERSION_ANY == version )
        m_maxVersion = TLSVERSION_ANY;
    else
        m_maxVersion = version < m_minVersion ? m_minVersion : version;
}

/*-------------------------------------------------------------------------*/

ETlsVersion
CTlsServerSettings::GetMaxVersion( void ) const
{GUCEF_TRACE;

    return m_maxVersion;
}

/*-------------------------------------------------------------------------*/

void
CTlsServerSettings::SetAlpnProtocols( const TStringVector& protocols )
{GUCEF_TRACE;

    m_alpnProtocols.clear();
    for ( size_t i=0; i<protocols.size(); ++i )
        AddAlpnProtocol( protocols[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CTlsServerSettings::AddAlpnProtocol( const CORE::CString& protocol )
{GUCEF_TRACE;

    if ( !protocol.IsNULLOrEmpty() && protocol.Length() <= 255 )
        m_alpnProtocols.push_back( protocol );
}

/*-------------------------------------------------------------------------*/

const CTlsServerSettings::TStringVector&
CTlsServerSettings::GetAlpnProtocols( void ) const
{GUCEF_TRACE;

    return m_alpnProtocols;
}

/*-------------------------------------------------------------------------*/

void
CTlsServerSettings::SetHandshakeTimeoutInMs( UInt32 timeoutInMs )
{GUCEF_TRACE;

    m_handshakeTimeoutInMs = timeoutInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CTlsServerSettings::GetHandshakeTimeoutInMs( void ) const
{GUCEF_TRACE;

    return m_handshakeTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTlsServerSettings::GetContextKey( void ) const
{GUCEF_TRACE;

    return CORE::CString( "server|" ) + ServerVersionToString( m_minVersion ) + "|" + ServerVersionToString( m_maxVersion ) + "|" +
           m_certificateChainPath + "|" + m_privateKeyPath + "|" + JoinServerList( m_alpnProtocols );
}

/*-------------------------------------------------------------------------*/

bool
CTlsServerSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "certificateChainPath", m_certificateChainPath );
    config.SetAttribute( "privateKeyPath", m_privateKeyPath );
    config.SetAttribute( "minVersion", CORE::CString( ServerVersionToString( m_minVersion ) ) );
    config.SetAttribute( "maxVersion", CORE::CString( ServerVersionToString( m_maxVersion ) ) );
    config.SetAttribute( "alpn", JoinServerList( m_alpnProtocols ) );
    config.SetAttribute( "handshakeTimeoutInMs", m_handshakeTimeoutInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTlsServerSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_certificateChainPath = config.GetAttributeValueOrChildValueByName( "certificateChainPath" ).AsString( m_certificateChainPath, true );
    m_privateKeyPath = config.GetAttributeValueOrChildValueByName( "privateKeyPath" ).AsString( m_privateKeyPath, true );
    SetMinVersion( ServerVersionFromString( config.GetAttributeValueOrChildValueByName( "minVersion" ).AsString( CORE::CString::Empty, true ), m_minVersion ) );
    SetMaxVersion( ServerVersionFromString( config.GetAttributeValueOrChildValueByName( "maxVersion" ).AsString( CORE::CString::Empty, true ), m_maxVersion ) );
    m_handshakeTimeoutInMs = config.GetAttributeValueOrChildValueByName( "handshakeTimeoutInMs" ).AsUInt32( m_handshakeTimeoutInMs, true );

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
CTlsServerSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CTlsServerSettings";
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
