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

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#define GUCEF_COMCORE_CHOSTADDRESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CDNSRESOLVER_H
#include "gucefCOMCORE_CDnsResolver.h"      
#define GUCEF_COMCORE_CDNSRESOLVER_H
#endif /* GUCEF_COMCORE_CDNSRESOLVER_H ? */

#ifndef GUCEF_COMCORE_CIPV4ADDRESS_H
#include "gucefCOMCORE_CIPv4Address.h"      
#define GUCEF_COMCORE_CIPV4ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV4ADDRESS_H ? */

#ifndef GUCEF_COMCORE_CIPV6ADDRESS_H
#include "gucefCOMCORE_CIPv6Address.h"      
#define GUCEF_COMCORE_CIPV6ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV6ADDRESS_H ? */

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
 *  Class representing an internet address with host name in string form
 *  plus the IPv4 or IPv6 addresses with a port number included.
 *
 *  Note that this class uses network byte order as its internal storage format.
 *  The aim here is to pay setup cost in favor of ongoing cost performance wise
 *
 *  This class is not threadsafe and is intended to act as thread private information
 *  for multi-threaded management of DNS resolution check out the DnsCache class instead
 */
class GUCEF_COMCORE_EXPORT_CPP CHostAddress
{
    public:

    typedef GUCEF::vector< CHostAddress, gucef_allocator< CHostAddress > >    THostAddressVector;

    CHostAddress( void );

    CHostAddress( const CHostAddress& src );

    /**
     *  The port value is expected to be in host byte order
     */
    CHostAddress( const CORE::CString& hostname ,
                  const UInt16 port             );

    /**
     *  The host (DNS) and port together in a single string
     *  takes the forms:
     *      <DNS>:<Port>
     *      <IP>:<Port>
     */
    CHostAddress( const CORE::CString& hostAndPort );

    /**
     *  Special case constructor in case you already have a dns plus a related IPv4
     *  In such a case you likely dont want to take the performance hit of resolving the DNS yet again
     *  this allows you to init a CHostAddress using those bits.
     */
    CHostAddress( const CIPv4Address& ipAddress ,
                  const CORE::CString& hostname );

    virtual ~CHostAddress();

    bool SetHostname( const CORE::CString& hostName );

    /**
     *  The host (DNS) and port together in a single string
     *  takes the forms:
     *      <DNS>:<Port>
     *      <IP>:<Port>
     */
    bool SetHostnameAndPort( const CORE::CString& hostAndPort );

    bool SetHostnameAndPort( const CORE::CString& host, UInt16 portInHostOrder );

    bool SetPortInHostByteOrder( UInt16 portInHostOrder );

    const CORE::CString& GetHostname( void ) const;

    CIPv4Address::TIPv4AddressVector& GetIPv4Addresses( void );

    const CIPv4Address::TIPv4AddressVector& GetIPv4Addresses( void ) const;

    CString GetIPv4AddressesAsString( char delimiter = ',' ) const;

    const CIPv4Address& GetRandomIPv4Address( void ) const;

    const CIPv4Address& GetFirstIPv4Address( void ) const;

    CIPv6Address::TIPv6AddressVector& GetIPv6Addresses( void );

    const CIPv6Address::TIPv6AddressVector& GetIPv6Addresses( void ) const;

    CORE::CString GetFirstAddressAndPortAsString( void ) const;

    CORE::CString GetFirstAddressAsString( void ) const;

    /**
     *  Refreshes the contained data using DNS resolution
     */
    bool Refresh( void );

    /**
     *  Address and port as a string with the network to host
     *  conversion applied
     */
    CORE::CString HostnameAndPortAsString( void ) const;

    CORE::CString PortAsString( void ) const;

    UInt16 GetPortInHostByteOrder( void ) const;

    UInt16 GetPortInNetworkByteOrder( void ) const;

    CHostAddress& operator=( const CHostAddress& src );

    CHostAddress& operator=( const CIPv4Address& src );

    bool operator==( const CHostAddress& other ) const;

    bool operator!=( const CHostAddress& other ) const;

    /**
     *  This operator is only implemented to facilitate ordering
     *  in STL containers. The return value has no real meaning
     *  except that of a binary data compare.
     */
    bool operator<( const CHostAddress& other ) const;

    void Clear( void );

    void Assign( const CHostAddress& src, bool copyPort );

    bool Equals( const CHostAddress& other, bool compareEverything );

    bool IsIPv4Only( void ) const;

    bool IsDnsWithIPv4Only( void ) const;

    bool HasDnsBasedHostname( void ) const;

    private:

    CORE::CString m_hostname;
    CORE::CString::StringVector m_aliases;
    CIPv4Address::TIPv4AddressVector m_ipv4;
    CIPv6Address::TIPv6AddressVector m_ipv6;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */
