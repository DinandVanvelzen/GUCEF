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

#ifndef GUCEF_COMCORE_CIPV6ADDRESS_H
#define GUCEF_COMCORE_CIPV6ADDRESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"      /* macros and build config for the COMCORE library */
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CIPv4Address;

/**
 *  Class representing an IPv6 address
 */
class GUCEF_COMCORE_EXPORT_CPP CIPv6Address
{
    public:

    typedef GUCEF::vector< CIPv6Address, gucef_allocator< CIPv6Address > > TIPv6AddressVector;


    // Default constructor initializes to :: (all zeroes)
    CIPv6Address( void );

    // Constructor from a string (e.g., "2001:0db8:85a3::8a2e:0370:7334")
    CIPv6Address( const CString& addr );

    // Constructor from an array of 16 bytes
    CIPv6Address( const UInt8 addr[16] );

    // Constructor from an IPv4 address
    CIPv6Address( UInt32 ipv4Addr );

    CIPv6Address( const CIPv4Address& ip );

    ~CIPv6Address();

    // Static function to create from an IPv4 address in string form
    static bool TryConvertIPv4StringToIPv6Address( const CString& ipv4Addr, CIPv6Address& ipv6 );

    // Static function to create from a CIDR notation
    static bool TryConvertCIDRStringToIPv6Address( const CString& cidr, Int32& prefixLength, CIPv6Address& ipv6 );

    // Convert to CIDR notation
    CString ToCIDRString( Int32 prefixLength ) const;

    // Get the address as a string
    CString ToString( void ) const;

    bool FromIPv6String( const CString& ipv6Str );

    void Clear( void );

    bool operator==( const CIPv6Address& other ) const;

    bool operator!=( const CIPv6Address& other ) const;

    bool operator<( const CIPv6Address& other ) const;

    // Comparison with an IPv4 address (in binary form)
    bool operator==( const unsigned char ipv4Addr[ 4 ] ) const;

    bool operator==( const CIPv4Address& other ) const;

    bool operator!=( const CIPv4Address& other ) const;

    // Check if the address is an IPv4-mapped IPv6 address (::ffff:x.x.x.x)
    bool IsIPv4Mapped( void ) const;

    // Convert to IPv4 address if it is IPv4-mapped
    bool TryConvertToIPv4String( CString& ipv4Str ) const;

    // Subnet utility function to check if two addresses are in the same subnet
    bool IsInSameSubnet( const CIPv6Address& other, Int32 prefixLength ) const;

    // Apply subnet mask to an address
    bool ApplyMask( Int32 prefixLength );

    bool IsMulticast( void ) const;

    bool IsLoopback( void ) const;

    bool IsUnspecified( void ) const;

    bool IsLinkLocal( void ) const;

    private:
    
    UInt8 m_address[ 16 ]; /**< The IPv6 address is stored as a 16-byte array. */
};

/*-------------------------------------------------------------------------*/

inline CString ToString( const CIPv6Address& addr ) { return addr.ToString(); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIPV6ADDRESS_H ? */
