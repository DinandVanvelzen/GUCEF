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

#include <string.h>

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

#ifndef GUCEF_COMCORE_DVSOCKET_H
#include "dvwinsock.h"
#define GUCEF_COMCORE_DVSOCKET_H
#endif /* GUCEF_COMCORE_DVSOCKET_H ? */

#include "gucefCOMCORE_CIPv4Address.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #undef FD_SETSIZE
  #define FD_SETSIZE 1      /* should set the size of the FD set struct to 1 for VC */
  #include <winsock2.h>
  #include <Ws2tcpip.h>
  #include <Wspiapi.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    #include <unistd.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

#else

    #error Unsupported platform

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CIPv4Address CIPv4Address::Any( INADDR_ANY, 0 );
const CIPv4Address CIPv4Address::None( INADDR_NONE, 0 );
const CIPv4Address CIPv4Address::Loopback( INADDR_LOOPBACK, 0 );
const CIPv4Address CIPv4Address::Broadcast( INADDR_BROADCAST, 0 );

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CIPv4Address::CIPv4Address( void )
    : m_address( 0 ) ,
      m_port( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIPv4Address::CIPv4Address( const CIPv4Address& src )
    : m_address( src.m_address ) ,
      m_port( src.m_port )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIPv4Address::CIPv4Address( const UInt32 addressInNetworkByteOrder ,
                            const UInt16 portInNetworkByteOrder    )
    : m_address( addressInNetworkByteOrder ) ,
      m_port( portInNetworkByteOrder )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIPv4Address::CIPv4Address( const CORE::CString& address     ,
                            const UInt16 portInHostByteOrder )
    : m_address( 0 )          ,
      m_port( htons( portInHostByteOrder ) )
{GUCEF_TRACE;

    SetAddress( address );
}

/*-------------------------------------------------------------------------*/

CIPv4Address::~CIPv4Address()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt16
CIPv4Address::HostByteOrderToNetworkByteOrder( UInt16 nrInHostByteOrder )
{GUCEF_TRACE;

    return ::htons( nrInHostByteOrder );
}

/*-------------------------------------------------------------------------*/

UInt32 
CIPv4Address::HostByteOrderToNetworkByteOrder( UInt32 nrInHostByteOrder )
{GUCEF_TRACE;

    return ::htonl( nrInHostByteOrder );
}

/*-------------------------------------------------------------------------*/

void
CIPv4Address::SetPort( const UInt16 portInNetworkByteOrder )
{GUCEF_TRACE;

    m_port = portInNetworkByteOrder;
}

/*-------------------------------------------------------------------------*/

UInt16
CIPv4Address::GetPort( void ) const
{GUCEF_TRACE;

    return m_port;
}

/*-------------------------------------------------------------------------*/

void
CIPv4Address::SetPortInHostByteOrder( const UInt16 port )
{GUCEF_TRACE;

    m_port = htons( port );
}

/*-------------------------------------------------------------------------*/

UInt16
CIPv4Address::GetPortInHostByteOrder( void ) const
{GUCEF_TRACE;

    return ntohs( m_port );
}

/*-------------------------------------------------------------------------*/

void
CIPv4Address::SetAddressInHostByteOrder( const UInt32 address )
{GUCEF_TRACE;

    m_address = htonl( address );
}

/*-------------------------------------------------------------------------*/

bool
CIPv4Address::SetAddress( const UInt32 address )
{GUCEF_TRACE;

    m_address = address;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIPv4Address::SetAddress( const CString& address )
{GUCEF_TRACE;

    if ( address.IsNULLOrEmpty() ) 
        return false;
    
    if ( CORE::Check_If_IPv4( address.C_String() ) )
    {
        UInt32 newAddress = inet_addr( address.C_String() );
        if ( newAddress == INADDR_NONE ) 
            return false;

        if ( m_address != newAddress )
        {
            m_address = newAddress;
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv4Address::SetAddressAndPort( const CString& addressAndPort )
{GUCEF_TRACE;

    if ( addressAndPort.IsNULLOrEmpty() ) 
        return false;

    Int32 colonOffset = addressAndPort.HasChar( ':', true );
    if ( colonOffset > 0 )
    {
        UInt16 portInHostByteOrder = StringToUInt16( addressAndPort.SubstrToIndex( (UInt32) colonOffset, false ), 0 );  
        SetPortInHostByteOrder( portInHostByteOrder );
        return SetAddress( addressAndPort.SubstrToIndex( (UInt32) colonOffset, true ) );
    }
    else
    {
        return SetAddress( addressAndPort );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CIPv4Address::GetAddress( void ) const
{GUCEF_TRACE;

    return m_address;
}

/*-------------------------------------------------------------------------*/

UInt32
CIPv4Address::GetAddressInHostByteOrder( void ) const
{GUCEF_TRACE;

    return ntohl( m_address );
}

/*-------------------------------------------------------------------------*/

CString
CIPv4Address::AddressAsString( void ) const
{GUCEF_TRACE;

    UInt32 address = ntohl( m_address );
    UInt8* addressArray = (UInt8*) &address;

    CString addressStr =
      CORE::UInt8ToString( addressArray[ 3 ] ) + '.' +
      CORE::UInt8ToString( addressArray[ 2 ] ) + '.' +
      CORE::UInt8ToString( addressArray[ 1 ] ) + '.' +
      CORE::UInt8ToString( addressArray[ 0 ] );

    return addressStr;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIPv4Address::PortAsString( void ) const
{GUCEF_TRACE;

    UInt32 port = ntohs( m_port );
    return CORE::UInt32ToString( port );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIPv4Address::AddressAndPortAsString( void ) const
{GUCEF_TRACE;

    return AddressAsString() + ':' + PortAsString();
}

/*-------------------------------------------------------------------------*/

char 
CIPv4Address::GetIPClass( void ) const
{GUCEF_TRACE;

    if ( m_address & 0xE ) // First octet starts with 1110
        return 'D';
    if ( m_address & 0x6 ) // First octet starts with 110
        return 'C';
    if ( m_address & 0x2 ) // First octet starts with 10
        return 'B';
    if ( !( m_address & 0x1 ) ) // First octet starts with 0
        return 'A';
    return 'E';
}

/*-------------------------------------------------------------------------*/

bool 
CIPv4Address::IsMulticast( void ) const
{GUCEF_TRACE;

    // Multicast is always a class D address
    if ( m_address & 0xE ) // First octet starts with 1110
        return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CIPv4Address::Clear( void )
{GUCEF_TRACE;

    m_address = 0;
    m_port = 0;
}

/*-------------------------------------------------------------------------*/

CIPv4Address&
CIPv4Address::operator=( const CIPv4Address& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_address = src.m_address;
        m_port = src.m_port;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CIPv4Address::operator==( const CIPv4Address& other ) const
{GUCEF_TRACE;

    return ( m_address == other.m_address ) &&
           ( m_port == other.m_port );
}

/*-------------------------------------------------------------------------*/

bool
CIPv4Address::operator!=( const CIPv4Address& other ) const
{GUCEF_TRACE;

    return ( m_address != other.m_address ) ||
           ( m_port != other.m_port );
}

/*-------------------------------------------------------------------------*/

bool
CIPv4Address::operator<( const CIPv4Address& other ) const
{GUCEF_TRACE;

    // Implemented in this manner because it appears to result in the best
    // runtime performance
    char buff[ 6 ];
    *static_cast< UInt32* >( (void*)buff ) = m_address;
    *static_cast< UInt16* >( (void*)(buff+4) ) = m_port;

    char otherBuff[ 6 ];
    *static_cast< UInt32* >( (void*)otherBuff ) = other.m_address;
    *static_cast< UInt16* >( (void*)(otherBuff+4) ) = other.m_port;

    return memcmp( buff, otherBuff, 6 ) < 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CIPv4Address::CidrToSubnetMask( Int32 cidr )
{GUCEF_TRACE;

    return (cidr == 0) ? 0 : (~0U << (32 - cidr));
}

/*-------------------------------------------------------------------------*/

void
IPv4AddressSetToIPv4AddressVector( const CIPv4Address::TIPv4AddressSet& sortedSet , 
                                   CIPv4Address::TIPv4AddressVector& vec          )
{GUCEF_TRACE;

    vec.reserve( sortedSet.size() );
    CIPv4Address::TIPv4AddressSet::const_iterator i = sortedSet.begin();
    while ( i != sortedSet.end() )
    {
        vec.push_back( (*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
