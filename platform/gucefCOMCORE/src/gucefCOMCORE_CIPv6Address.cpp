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

#ifndef GUCEF_COMCORE_CIPV4ADDRESS_H
#include "gucefCOMCORE_CIPv4Address.h"
#define GUCEF_COMCORE_CIPV4ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV4ADDRESS_H ? */

#include "gucefCOMCORE_CIPv6Address.h"

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

/*-------------------------------------------------------------------------*/

// Default constructor initializes to :: (all zeroes)
CIPv6Address::CIPv6Address( void )
    : m_address()
{GUCEF_TRACE;

    memset( m_address, 0, sizeof( m_address ) );
}

/*-------------------------------------------------------------------------*/

// Constructor from a string (e.g., "2001:0db8:85a3::8a2e:0370:7334")
CIPv6Address::CIPv6Address( const CString& addr ) 
    : m_address()
{GUCEF_TRACE;

    memset( m_address, 0, sizeof( m_address ) );
    if ( ::inet_pton(AF_INET6, addr.C_String(), m_address) != 1 ) {
        throw std::invalid_argument("Invalid IPv6 m_address format");
    }
}

/*-------------------------------------------------------------------------*/

// Constructor from an array of 16 bytes
CIPv6Address::CIPv6Address( const unsigned char addr[16] ) 
    : m_address()
{GUCEF_TRACE;

    memcpy( m_address, addr, 16 );
}

/*-------------------------------------------------------------------------*/

// Constructor from an IPv4 m_address
CIPv6Address::CIPv6Address( UInt32 ipv4Addr ) 
    : m_address()
{GUCEF_TRACE;

    memset( m_address, 0, 10);             // First 10 bytes are zero
    m_address[10] = 0xff;                  // Next 2 bytes are 0xff
    m_address[11] = 0xff;
    memcpy(m_address + 12, &ipv4Addr, 4);   // Last 4 bytes are the IPv4 m_address
}

/*-------------------------------------------------------------------------*/

// Constructor from an IPv4 object
CIPv6Address::CIPv6Address( const CIPv4Address& ip ) 
    : m_address()
{GUCEF_TRACE;

    memset( m_address, 0, 10);             // First 10 bytes are zero
    m_address[10] = 0xff;                  // Next 2 bytes are 0xff
    m_address[11] = 0xff;
    UInt32 ipAddr = ip.GetAddress();
    memcpy( m_address + 12, &ipAddr, 4 );   // Last 4 bytes are the IPv4 m_address
}

/*-------------------------------------------------------------------------*/

CIPv6Address::~CIPv6Address() 
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

// Static function to create from an IPv4 m_address in string form
bool
CIPv6Address::TryConvertIPv4StringToIPv6Address( const CString& ipv4Addr, CIPv6Address& ipv6 ) 
{GUCEF_TRACE;

    if ( ipv4Addr.IsNULLOrEmpty() )
        return false;
    
    UInt8 ipv4Binary[4];
    if ( ::inet_pton( AF_INET, ipv4Addr.C_String(), ipv4Binary) != 1 ) 
    {
        // Invalid IPv4 m_address format
        return false;
    }
    ipv6 = CIPv6Address( ipv4Binary );
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::TryConvertCIDRStringToIPv6Address( const CString& cidr, Int32& prefixLength, CIPv6Address& ipv6 ) 
{GUCEF_TRACE;

    Int32 pos = cidr.HasChar( '/', false );
    if ( pos < 0 ) 
    {
        // Invalid CIDR notation
        return false;
    }

    CString addr = cidr.SubstrToIndex( pos, true );
    CString prefixLengthStr = cidr.SubstrToIndex( pos+1, false );

    prefixLength = CORE::StringToInt32( prefixLengthStr, 0 );
    if ( prefixLength < 0 || prefixLength > 128 ) 
    {
        // Invalid prefix length
        return false;
    }

    ipv6 = addr;
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CIPv6Address::ToCIDRString( Int32 prefixLength ) const 
{GUCEF_TRACE;

    if ( prefixLength < 0 || prefixLength > 128 ) 
    {
        // Invalid prefix length
        return CString::Empty;
    }
    return ToString() + "/" + CORE::ToString( prefixLength );
}

/*-------------------------------------------------------------------------*/

// Get the m_address as a string
CString
CIPv6Address::ToString( void ) const 
{GUCEF_TRACE;

    char str[ INET6_ADDRSTRLEN ];
    if ( ::inet_ntop( AF_INET6, m_address, str, INET6_ADDRSTRLEN) == GUCEF_NULL ) 
    {
        // Failed to convert IPv6 m_address to string
        return CString::Empty;
    }
    return CString( str, INET6_ADDRSTRLEN );
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::operator==( const CIPv6Address& other ) const 
{GUCEF_TRACE;

    return ::memcmp( m_address, other.m_address, 16 ) == 0;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::operator!=( const CIPv6Address& other ) const 
{GUCEF_TRACE;

    return !(*this == other);
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::operator<( const CIPv6Address& other ) const 
{GUCEF_TRACE;

    return ::memcmp( m_address, other.m_address, 16 ) < 0;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::operator==( const unsigned char ipv4Addr[ 4 ] ) const 
{GUCEF_TRACE;

    if ( !IsIPv4Mapped() ) 
        return false;

    return ::memcmp( m_address + 12, ipv4Addr, 4 ) == 0;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::operator==( const CIPv4Address& other ) const 
{GUCEF_TRACE;

    if ( !IsIPv4Mapped() ) 
        return false;

    UInt32 ipv4Addr = other.GetAddress();
    return ::memcmp( m_address + 12, &ipv4Addr, 4 ) == 0;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::operator!=( const CIPv4Address& other ) const 
{GUCEF_TRACE;

    return !( (*this) == other );
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::IsIPv4Mapped( void ) const 
{GUCEF_TRACE;

    return ( ::memcmp( m_address, "\0\0\0\0\0\0\0\0\0\0\xff\xff", 12 ) == 0 );
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::FromIPv6String( const CString& ipv6Str ) 
{GUCEF_TRACE;

    memset( m_address, 0, sizeof( m_address ) );
    if ( ::inet_pton( AF_INET6, ipv6Str.C_String(), m_address ) == 1 ) 
        return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void 
CIPv6Address::Clear( void ) 
{GUCEF_TRACE;

    memset( m_address, 0, sizeof( m_address ) );
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::TryConvertToIPv4String( CString& ipv4Str ) const 
{GUCEF_TRACE;

    if ( !IsIPv4Mapped() ) 
    {
        // Not an IPv4-mapped IPv6 m_address
        return false;
    }
    char str[ INET_ADDRSTRLEN ];
    if ( ::inet_ntop( AF_INET, m_address + 12, str, INET_ADDRSTRLEN ) == GUCEF_NULL ) 
    {
        // Failed to convert IPv4-mapped IPv6 m_address to IPv4 string
        return false;
    }
    
    ipv4Str.Set( str, INET_ADDRSTRLEN );
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::IsInSameSubnet( const CIPv6Address& other, Int32 prefixLength ) const 
{GUCEF_TRACE;

    if ( prefixLength < 0 || prefixLength > 128 ) 
    {
        // Invalid prefix length
        return false;
    }
    Int32 fullBytes = prefixLength / 8;
    Int32 remainingBits = prefixLength % 8;

    // Compare the full bytes
    if ( ::memcmp( m_address, other.m_address, fullBytes ) != 0 ) 
    {
        return false;
    }

    // Compare the remaining bits
    if ( remainingBits > 0 ) 
    {
        UInt8 mask = (0xFF << (8 - remainingBits));
        return ( m_address[fullBytes] & mask ) == ( other.m_address[fullBytes] & mask );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIPv6Address::ApplyMask( Int32 prefixLength ) 
{GUCEF_TRACE;

    if ( prefixLength < 0 || prefixLength > 128 ) 
    {
        // Invalid prefix length
        return false;
    }

    Int32 fullBytes = prefixLength / 8;
    Int32 remainingBits = prefixLength % 8;

    // Zero out the bits beyond the prefix length
    if ( fullBytes < 16 ) 
    {
        ::memset( m_address + fullBytes, 0, 16 - fullBytes );
    }

    if ( remainingBits > 0 ) 
    {
        UInt8 mask = (0xFF << (8 - remainingBits));
        m_address[ fullBytes ] &= mask;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::IsMulticast( void ) const 
{GUCEF_TRACE;

    return (m_address[0] == 0xff); // Multicast addresses start with "ff"
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::IsLoopback( void ) const 
{GUCEF_TRACE;

    static const unsigned char loopback[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    return memcmp(m_address, loopback, 16) == 0;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::IsUnspecified( void ) const 
{GUCEF_TRACE;

    static const unsigned char unspecified[16] = {0};
    return memcmp(m_address, unspecified, 16) == 0;
}

/*-------------------------------------------------------------------------*/

bool 
CIPv6Address::IsLinkLocal( void ) const 
{GUCEF_TRACE;

    return (m_address[0] == 0xfe && (m_address[1] & 0xc0) == 0x80); // Link-local addresses start with "fe80::"
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
