/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNEndpoint.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNEndpoint::ChannelTypeTcp = "tcp";
const CORE::CString CDRNEndpoint::ChannelTypeUdp = "udp";
const CORE::CString CDRNEndpoint::ChannelTypeWebSocket = "websocket";
const CORE::CString CDRNEndpoint::ChannelTypeRelay = "relay";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNEndpoint::CDRNEndpoint( void )
    : m_channelType()
    , m_address()
    , m_params()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNEndpoint::CDRNEndpoint( const CORE::CString& channelType, const CORE::CString& address )
    : m_channelType( channelType )
    , m_address( address )
    , m_params()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNEndpoint::CDRNEndpoint( const CORE::CString& uri )
    : m_channelType()
    , m_address()
    , m_params()
{GUCEF_TRACE;

    FromString( uri );
}

/*-------------------------------------------------------------------------*/

CDRNEndpoint::CDRNEndpoint( const CDRNEndpoint& src )
    : m_channelType( src.m_channelType )
    , m_address( src.m_address )
    , m_params( src.m_params )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNEndpoint::~CDRNEndpoint()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNEndpoint&
CDRNEndpoint::operator=( const CDRNEndpoint& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_channelType = src.m_channelType;
        m_address = src.m_address;
        m_params = src.m_params;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpoint::operator==( const CDRNEndpoint& other ) const
{GUCEF_TRACE;

    return ToString() == other.ToString();
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpoint::operator!=( const CDRNEndpoint& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpoint::operator<( const CDRNEndpoint& other ) const
{GUCEF_TRACE;

    return ToString() < other.ToString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNEndpoint::SchemeForChannelType( const CORE::CString& channelType )
{GUCEF_TRACE;

    if ( channelType == ChannelTypeWebSocket )
        return "ws";
    return channelType;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNEndpoint::ChannelTypeForScheme( const CORE::CString& scheme )
{GUCEF_TRACE;

    CORE::CString lowerScheme = scheme.Lowercase();
    if ( lowerScheme == "ws" )
        return ChannelTypeWebSocket;
    return lowerScheme;
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpoint::FromString( const CORE::CString& uri )
{GUCEF_TRACE;

    Clear();

    Int32 schemeEnd = uri.HasSubstr( "://", true );
    if ( schemeEnd <= 0 )
        return false;

    m_channelType = ChannelTypeForScheme( uri.SubstrToIndex( (UInt32) schemeEnd, true ) );
    CORE::CString rest = uri.SubstrFromRange( (UInt32) schemeEnd + 3, uri.Length() );

    Int32 paramStart = rest.HasChar( '?', true );
    if ( paramStart >= 0 )
    {
        CORE::CString paramStr = rest.SubstrFromRange( (UInt32) paramStart + 1, rest.Length() );
        m_address = rest.SubstrToIndex( (UInt32) paramStart, true );

        CORE::CString::StringVector pairs = paramStr.ParseElements( '&', false );
        CORE::CString::StringVector::iterator i = pairs.begin();
        while ( i != pairs.end() )
        {
            const CORE::CString& pair = (*i);
            Int32 eq = pair.HasChar( '=', true );
            if ( eq > 0 )
                m_params.Set( pair.SubstrToIndex( (UInt32) eq, true ), pair.SubstrFromRange( (UInt32) eq + 1, pair.Length() ) );
            else
            if ( !pair.IsNULLOrEmpty() )
                m_params.Set( pair, CORE::CVariant( CORE::CString::Empty ) );
            ++i;
        }
    }
    else
    {
        m_address = rest;
    }

    return IsValid();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNEndpoint::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString uri = SchemeForChannelType( m_channelType ) + "://" + m_address;
    UInt32 paramCount = m_params.GetKeyCount();
    for ( UInt32 i=0; i<paramCount; ++i )
    {
        uri += ( 0 == i ) ? "?" : "&";
        uri += m_params.GetKey( i ).AsString() + "=" + m_params.GetValue( i ).AsString();
    }
    return uri;
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpoint::IsValid( void ) const
{GUCEF_TRACE;

    return !m_channelType.IsNULLOrEmpty() && !m_address.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
CDRNEndpoint::Clear( void )
{GUCEF_TRACE;

    m_channelType.Clear();
    m_address.Clear();
    m_params.Clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNEndpoint::SetChannelType( const CORE::CString& channelType )
{GUCEF_TRACE;

    m_channelType = channelType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNEndpoint::GetChannelType( void ) const
{GUCEF_TRACE;

    return m_channelType;
}

/*-------------------------------------------------------------------------*/

void
CDRNEndpoint::SetAddress( const CORE::CString& address )
{GUCEF_TRACE;

    m_address = address;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNEndpoint::GetAddress( void ) const
{GUCEF_TRACE;

    return m_address;
}

/*-------------------------------------------------------------------------*/

CORE::CValueList&
CDRNEndpoint::GetParams( void )
{GUCEF_TRACE;

    return m_params;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
CDRNEndpoint::GetParams( void ) const
{GUCEF_TRACE;

    return m_params;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNEndpoint::GetHost( void ) const
{GUCEF_TRACE;

    CORE::CString hostAndPort = m_address;
    Int32 slash = hostAndPort.HasChar( '/', true );
    if ( slash >= 0 )
        hostAndPort = hostAndPort.SubstrToIndex( (UInt32) slash, true );
    Int32 colon = hostAndPort.HasChar( ':', false );
    if ( colon >= 0 )
        return hostAndPort.SubstrToIndex( (UInt32) colon, true );
    return hostAndPort;
}

/*-------------------------------------------------------------------------*/

UInt16
CDRNEndpoint::GetPort( void ) const
{GUCEF_TRACE;

    CORE::CString hostAndPort = m_address;
    Int32 slash = hostAndPort.HasChar( '/', true );
    if ( slash >= 0 )
        hostAndPort = hostAndPort.SubstrToIndex( (UInt32) slash, true );
    Int32 colon = hostAndPort.HasChar( ':', false );
    if ( colon < 0 )
        return 0;
    return CORE::StringToUInt16( hostAndPort.SubstrFromRange( (UInt32) colon + 1, hostAndPort.Length() ), 0 );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNEndpoint::GetPath( void ) const
{GUCEF_TRACE;

    Int32 slash = m_address.HasChar( '/', true );
    if ( slash < 0 )
        return CORE::CString::Empty;
    return m_address.SubstrFromRange( (UInt32) slash, m_address.Length() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
