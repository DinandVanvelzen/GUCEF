/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include <math.h>

#ifndef GUCEF_COMCORE_SOCKETUTILS_H
#include "socketutils.h"
#define GUCEF_COMCORE_SOCKETUTILS_H
#endif /* GUCEF_COMCORE_SOCKETUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "CHostAddress.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHostAddress::CHostAddress( void )
    : m_hostname()
    , m_aliases()
    , m_ipv4()
    , m_ipv6()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHostAddress::CHostAddress( const CHostAddress& src )
    : m_hostname( src.m_hostname )
    , m_aliases( src.m_aliases )
    , m_ipv4( src.m_ipv4 )
    , m_ipv6( src.m_ipv6 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHostAddress::CHostAddress( const CIPv4Address& ipAddress ,
                            const CORE::CString& hostname )
    : m_hostname( hostname )
    , m_aliases()
    , m_ipv4()
    , m_ipv6()
{GUCEF_TRACE;

    m_ipv4.push_back( ipAddress );
}

/*-------------------------------------------------------------------------*/

CHostAddress::CHostAddress( const CORE::CString& hostname ,
                            const UInt16 port             )
    : m_hostname( hostname )
    , m_aliases()
    , m_ipv4()
    , m_ipv6()
{GUCEF_TRACE;

    SetHostnameAndPort( hostname, port );
}

/*-------------------------------------------------------------------------*/

CHostAddress::CHostAddress( const CORE::CString& hostAndPort )
    : m_hostname()
    , m_aliases()
    , m_ipv4()
    , m_ipv6()
{GUCEF_TRACE;

    SetHostnameAndPort( hostAndPort );
}

/*-------------------------------------------------------------------------*/

CHostAddress::~CHostAddress()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::SetHostnameAndPort( const CORE::CString& hostAndPort )
{GUCEF_TRACE;

    UInt16 newPort = 0;
    UInt16 currentPort = GetPortInHostByteOrder();
    CORE::CString varResolvedHostAndPort = CORE::ResolveVars( hostAndPort );

    Clear();
    
    Int32 sepCharIndex = varResolvedHostAndPort.HasChar( ':', false );
    if ( sepCharIndex >= 0 )
    {
        m_hostname = varResolvedHostAndPort.SubstrToIndex( sepCharIndex, true );
        newPort = CORE::StringToUInt16( varResolvedHostAndPort.SubstrToIndex( sepCharIndex+1, false ), currentPort );
    }
    else
    {
        m_hostname = varResolvedHostAndPort;
        newPort = currentPort;
    }

    if ( !m_hostname.IsNULLOrEmpty() )
    {
        return CDnsResolver::Resolve( m_hostname ,
                                      newPort    ,
                                      m_aliases  ,
                                      m_ipv4     ,
                                      m_ipv6     );
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::SetHostnameAndPort( const CORE::CString& host ,
                                  UInt16 portInHostOrder    )
{GUCEF_TRACE;

    Clear();

    if ( !host.IsNULLOrEmpty() )
    {
        Int32 sepCharIndex = host.HasChar( ':', false );
        if ( sepCharIndex >= 0 )
        {
            // we only take the dns portion not the port since a port was explicitly distinctly provided
            // as such we'd consider that an override
            m_hostname = host.SubstrToIndex( sepCharIndex, true );
        }
        else
        {
            m_hostname = host;
        }

        return CDnsResolver::Resolve( m_hostname      ,
                                      portInHostOrder ,
                                      m_aliases       ,
                                      m_ipv4          ,
                                      m_ipv6          );
    }
    else
    {
        CIPv4Address ipv4;
        ipv4.SetAddress( INADDR_ANY );
        ipv4.SetPortInHostByteOrder( portInHostOrder );
        m_ipv4.push_back( ipv4 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::SetPortInHostByteOrder( UInt16 portInHostOrder )
{GUCEF_TRACE;

    CIPv4Address::TIPv4AddressVector::iterator i = m_ipv4.begin();
    while ( i != m_ipv4.end() )
    {
        CIPv4Address& ipv4 = (*i);
        ipv4.SetPortInHostByteOrder( portInHostOrder );
        ++i;
    }
    CIPv6Address::TIPv6AddressVector::iterator n = m_ipv6.begin();
    while ( n != m_ipv6.end() )
    {
        CIPv6Address& ipv6 = (*n);
        //ipv6.SetPortInHostByteOrder( portInHostOrder );
        ++n;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::SetHostname( const CORE::CString& hostName )
{GUCEF_TRACE;

    UInt16 currentPort = GetPortInHostByteOrder();

    Clear();

    m_hostname = hostName;
    return CDnsResolver::Resolve( m_hostname  ,
                                  currentPort ,
                                  m_aliases   ,
                                  m_ipv4      ,
                                  m_ipv6      );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CHostAddress::GetHostname( void ) const
{GUCEF_TRACE;

    return m_hostname;
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::Refresh( void )
{GUCEF_TRACE;

    UInt16 currentPort = GetPortInHostByteOrder();

    m_aliases.clear();
    m_ipv4.clear();
    m_ipv6.clear();

    return CDnsResolver::Resolve( m_hostname  ,
                                  currentPort ,
                                  m_aliases   ,
                                  m_ipv4      ,
                                  m_ipv6      );
}

/*-------------------------------------------------------------------------*/

UInt16
CHostAddress::GetPortInHostByteOrder( void ) const
{GUCEF_TRACE;

    if ( !m_ipv4.empty() )
    {
        return m_ipv4.front().GetPortInHostByteOrder();
    }
    if ( !m_ipv6.empty() )
    {
        // @TODO: IPv6 support
        //return m_ipv6.front().GetPortInHostByteOrder();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt16
CHostAddress::GetPortInNetworkByteOrder( void ) const
{GUCEF_TRACE;

    if ( !m_ipv4.empty() )
    {
        return m_ipv4.front().GetPort();
    }
    if ( !m_ipv6.empty() )
    {
        // @TODO: IPv6 support
        //return m_ipv6.front().GetPort();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHostAddress::PortAsString( void ) const
{GUCEF_TRACE;

    if ( !m_ipv4.empty() )
    {
        return m_ipv4.front().PortAsString();
    }
    if ( !m_ipv6.empty() )
    {
        //return m_ipv6.front().PortAsString();
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHostAddress::HostnameAndPortAsString( void ) const
{GUCEF_TRACE;

    return m_hostname + ':' + PortAsString();
}

/*-------------------------------------------------------------------------*/

void
CHostAddress::Clear( void )
{GUCEF_TRACE;

    m_hostname.Clear();
    m_aliases.clear();
    m_ipv4.clear();
    m_ipv6.clear();
}

/*-------------------------------------------------------------------------*/

CHostAddress&
CHostAddress::operator=( const CHostAddress& src )
{GUCEF_TRACE;

    Assign( src, true );
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CHostAddress::Assign( const CHostAddress& src, bool copyPort )
{
    if ( this != &src )
    {
        if ( copyPort )
        {
            m_hostname = src.m_hostname;
            m_aliases = src.m_aliases;
            m_ipv4 = src.m_ipv4;
            m_ipv6 = src.m_ipv6;
        }
        else
        {
            UInt16 currentPortInNetworkByteOrder = GetPortInNetworkByteOrder();
            Clear();

            m_hostname = src.m_hostname;
            m_aliases = src.m_aliases;

            CIPv4Address::TIPv4AddressVector::const_iterator i = src.m_ipv4.begin();
            while ( i != src.m_ipv4.end() )
            {
                const CIPv4Address& srcIpv4 = (*i);
                m_ipv4.push_back( CIPv4Address( srcIpv4.GetAddress(), currentPortInNetworkByteOrder ) );
                ++i;
            }
            CIPv6Address::TIPv6AddressVector::const_iterator n = src.m_ipv6.begin();
            while ( n != src.m_ipv6.end() )
            {
                // @TODO: IPv6 support
                const CIPv6Address& srcIpv6 = (*n);
                //m_ipv6.push_back( CIPv6Address( srcIpv6.GetAddress(), currentPortInNetworkByteOrder ) );
                ++n;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::Equals( const CHostAddress& other, bool compareEverything )
{GUCEF_TRACE;

    if ( m_hostname != other.m_hostname )
        return false;

    if ( compareEverything )
    {
        return m_aliases == other.m_aliases &&
               m_ipv4 == other.m_ipv4       &&
               m_ipv6 == other.m_ipv6;
    }

    return true;
}


/*-------------------------------------------------------------------------*/

CHostAddress&
CHostAddress::operator=( const CIPv4Address& src )
{GUCEF_TRACE;

    Clear();

    m_hostname = src.AddressAsString();
    m_ipv4.push_back( src );

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::operator==( const CHostAddress& other ) const
{GUCEF_TRACE;

    return m_hostname == other.m_hostname;
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::operator!=( const CHostAddress& other ) const
{GUCEF_TRACE;

    return m_hostname != other.m_hostname;
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::operator<( const CHostAddress& other ) const
{GUCEF_TRACE;

    return m_hostname < other.m_hostname;
}

/*-------------------------------------------------------------------------*/

CIPv4Address::TIPv4AddressVector&
CHostAddress::GetIPv4Addresses( void )
{GUCEF_TRACE;

    return m_ipv4;
}

/*-------------------------------------------------------------------------*/

const CIPv4Address::TIPv4AddressVector&
CHostAddress::GetIPv4Addresses( void ) const
{GUCEF_TRACE;

    return m_ipv4;
}

/*-------------------------------------------------------------------------*/

CString 
CHostAddress::GetIPv4AddressesAsString( char delimiter ) const
{GUCEF_TRACE;

    CString result;
    CIPv4Address::TIPv4AddressVector::const_iterator i = m_ipv4.begin();
    while ( i != m_ipv4.end() )
    {
        if ( !result.IsNULLOrEmpty() )
            result += delimiter; 
        result += (*i).AddressAndPortAsString();
        ++i;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

const CIPv4Address&
CHostAddress::GetRandomIPv4Address( void ) const
{GUCEF_TRACE;

    if ( !m_ipv4.empty() )
    {
        float fractionalIndex = ( static_cast< float >( std::rand() ) / RAND_MAX ) * static_cast< float >( m_ipv4.size() );
        UInt32 index = static_cast< UInt32 >( round( fractionalIndex ) );
        if ( index >= m_ipv4.size() )
            index = static_cast< UInt32 >( m_ipv4.size() - 1 );
        return m_ipv4[ index ];
    }
    return CIPv4Address::None;
}

/*-------------------------------------------------------------------------*/

const CIPv4Address&
CHostAddress::GetFirstIPv4Address( void ) const
{GUCEF_TRACE;

    if ( !m_ipv4.empty() )
    {
        return m_ipv4.front();
    }
    return CIPv4Address::None;
}

/*-------------------------------------------------------------------------*/

CIPv6Address::TIPv6AddressVector&
CHostAddress::GetIPv6Addresses( void )
{GUCEF_TRACE;

    return m_ipv6;
}

/*-------------------------------------------------------------------------*/

const CIPv6Address::TIPv6AddressVector&
CHostAddress::GetIPv6Addresses( void ) const
{GUCEF_TRACE;

    return m_ipv6;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHostAddress::GetFirstAddressAndPortAsString( void ) const
{GUCEF_TRACE;

    if ( !m_ipv4.empty() )
    {
        return m_ipv4.front().AddressAndPortAsString();
    }
    if ( !m_ipv6.empty() )
    {
        // @TODO: IPv6 support
        //return m_ipv6.front().AddressAndPortAsString();
    }
    return m_hostname;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHostAddress::GetFirstAddressAsString( void ) const
{GUCEF_TRACE;

    if ( !m_ipv4.empty() )
    {
        return m_ipv4.front().AddressAsString();
    }
    if ( !m_ipv6.empty() )
    {
        // @TODO: IPv6 support
        //return m_ipv6.front().AddressAsString();
    }
    return m_hostname;
}

/*-------------------------------------------------------------------------*/

bool
CHostAddress::IsIPv4Only( void ) const
{GUCEF_TRACE;

    bool hostnameIsIPv4 = false;
    if ( !m_hostname.IsNULLOrEmpty() )
    {
        hostnameIsIPv4 = ( 0 != CORE::Check_If_IPv4( m_hostname.C_String() ) );
    }    
    
    return hostnameIsIPv4 && !m_ipv4.empty() && m_ipv6.empty(); 
}

/*-------------------------------------------------------------------------*/

bool 
CHostAddress::IsDnsWithIPv4Only( void ) const
{GUCEF_TRACE;

    return !m_hostname.IsNULLOrEmpty() && !m_ipv4.empty() && m_ipv6.empty(); 
}

/*-------------------------------------------------------------------------*/

bool 
CHostAddress::HasDnsBasedHostname( void ) const
{GUCEF_TRACE;

    // @TODO: IPv6 support
    return !m_hostname.IsNULLOrEmpty() && ( 0 == CORE::Check_If_IPv4( m_hostname.C_String() ) ); 
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
