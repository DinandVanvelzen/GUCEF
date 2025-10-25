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

#ifndef GUCEF_COMCORE_CIPV4ADDRESS_H
#define GUCEF_COMCORE_CIPV4ADDRESS_H

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

/**
 *  Class representing an IPv4 address with a port number included.
 *  Note that this class uses network byte order as its default.
 */
class GUCEF_COMCORE_EXPORT_CPP CIPv4Address
{
    public:

    typedef GUCEF::vector< CIPv4Address, gucef_allocator< CIPv4Address > >                            TIPv4AddressVector;
    typedef GUCEF::set< CIPv4Address, std::less< CIPv4Address >, gucef_allocator< CIPv4Address > >    TIPv4AddressSet;
    
    static const CIPv4Address Any;
    static const CIPv4Address None;
    static const CIPv4Address Loopback;
    static const CIPv4Address Broadcast;
    
    CIPv4Address( void );
    
    CIPv4Address( const CIPv4Address& src );

    /**
     *  The values are expected to be in network byte order 
     */
    CIPv4Address( const UInt32 addressInNetworkByteOrder ,
                  const UInt16 portInNetworkByteOrder    );

    /**
     *  The port value is expected to be in host byte order
     */
    CIPv4Address( const CORE::CString& address     ,
                  const UInt16 portInHostByteOrder );
                    
    virtual ~CIPv4Address();
    
    void SetPort( const UInt16 portInNetworkByteOrder );
    
    /**
     *  Port number in network byte order
     */
    UInt16 GetPort( void ) const;
    
    void SetPortInHostByteOrder( const UInt16 port );

    /**
     *  Port number in host byte order
     */
    UInt16 GetPortInHostByteOrder( void ) const;
    
    bool SetAddress( const UInt32 address );

    bool SetAddress( const CString& address );

    bool SetAddressAndPort( const CString& addressAndPort );
    
    /**
     *  Address in network byte order
     */
    UInt32 GetAddress( void ) const;
    
    void SetAddressInHostByteOrder( const UInt32 address );
    
    /**
     *  Address in host byte order
     */
    UInt32 GetAddressInHostByteOrder( void ) const;
    
    /**
     *  Address as a string with the network to host
     *  conversion applied
     */
    CORE::CString AddressAsString( void ) const;
    
    /**
     *  Port as a string with the network to host
     *  conversion applied
     */
    CORE::CString PortAsString( void ) const;
    
    /**
     *  Address and port as a string with the network to host
     *  conversion applied
     */
    virtual CORE::CString AddressAndPortAsString( void ) const;
    
    CIPv4Address& operator=( const CIPv4Address& src );
    
    bool operator==( const CIPv4Address& other ) const;
    
    bool operator!=( const CIPv4Address& other ) const;
    
    /**
     *  This operator is only implemented to facilitate ordering
     *  in STL containers. The return value has no real meaning
     *  except that of a binary data compare.
     */
    bool operator<( const CIPv4Address& other ) const;

    char GetIPClass( void ) const;

    bool IsMulticast( void ) const;

    void Clear( void );

    static UInt16 HostByteOrderToNetworkByteOrder( UInt16 nrInHostByteOrder );
    static UInt32 HostByteOrderToNetworkByteOrder( UInt32 nrInHostByteOrder );

    /**
     *  Function to calculate subnet mask from CIDR notation
     */
    static UInt32 CidrToSubnetMask( Int32 cidr );

    private:
    
    UInt16 m_port;
    UInt32 m_address;
};

/*-------------------------------------------------------------------------*/

GUCEF_COMCORE_EXPORT_CPP void
IPv4AddressSetToIPv4AddressVector( const CIPv4Address::TIPv4AddressSet& sortedSet , 
                                   CIPv4Address::TIPv4AddressVector& vec          );

/*-------------------------------------------------------------------------*/

inline CString ToString( const CIPv4Address& ip ) { return ip.AddressAndPortAsString(); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIPV4ADDRESS_H ? */
