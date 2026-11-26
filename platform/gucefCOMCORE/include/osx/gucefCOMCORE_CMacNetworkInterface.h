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

#ifndef GUCEF_COMCORE_CMACNETWORKINTERFACE_H
#define GUCEF_COMCORE_CMACNETWORKINTERFACE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CINETWORKINTERFACE_H
#include "gucefCOMCORE_CINetworkInterface.h"
#define GUCEF_COMCORE_CINETWORKINTERFACE_H
#endif /* GUCEF_COMCORE_CINETWORKINTERFACE_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

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
 *  The network interface of macOS: the interfaces and their addresses come from getifaddrs(), the counters from the
 *  64 bit interface statistics of the routing socket interface (sysctl), the default gateways from the routing table.
 */
class GUCEF_COMCORE_EXPORT_CPP CMacNetworkInterface : public CINetworkInterface
{
    public:

    typedef CORE::CTSharedPtr< CMacNetworkInterface, MT::CMutex >  CMacNetworkInterfacePtr;

    static bool EnumNetworkAdapters( TINetworkInterfacePtrVector& interfaces );

    /**
     *  The metrics of all the network interfaces of the machine together, loopback excluded:
     *  what the system keeps per interface, summed up.
     */
    static bool GetGlobalMetrics( CNetworkInterfaceMetrics& metrics );

    /**
     *  The IPv4 default gateway of the machine: where the routing table sends what has no route of its own.
     *  The address that is given has no port.
     */
    static bool GetDefaultGateway( CIPv4Address& gateway );

    virtual ~CMacNetworkInterface() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetCommunicationInterfaceType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetAdapterName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetAdapterDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetNrOfDnsAddresses( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetDnsAddresses( THostAddressVector& dnsAddresses ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetNrOfIPAddresses( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetIPInfo( TIPv4InfoVector& ipInfo, bool includeUninitialized = false ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsDhcpUsedForIPv4( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsDhcpUsedForIPv6( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CDateTime GetDhcpLeaseObtainedTime( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CDateTime GetDhcpLeaseExpirationTime( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ReleaseAddress( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RenewAddress( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWinsUsed( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CIPv4Address GetPrimaryWinsServer( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CIPv4Address GetSecondaryWinsServer( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetOsAdapterIndex( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetMetrics( CNetworkInterfaceMetrics& metrics ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CMacNetworkInterface( void );

    CMacNetworkInterface( const CMacNetworkInterface& src );            /**< not implemented, doesnt make sense */
    CMacNetworkInterface& operator=( const CMacNetworkInterface& src ); /**< not implemented, doesnt make sense */

    private:

    void FillExtraInfoFromConfig( void );

    private:

    CString m_name;
    CString m_desc;
    CString::StringSet m_macAddrs;
    CIPv4Address m_priWins;
    CIPv4Address m_secWins;
    CIPv4Address m_defGateway;
    CIPv4Address m_dhcpAddr;
    CIPv4Info m_curIpAddr;  // this is also in the ip address list but this is the address currently active.
    UInt32 m_nicIndex;      // machine index of the adapter.
    UInt32 m_adapterType;
    bool m_dhcpUsedforIpv4;
    bool m_dhcpUsedforIpv6;
    bool m_winsUsed;
    THostAddressVector m_dnsAddresses;
    CIPv4Info::TIPv4InfoSet m_ipv4Addresses;
    TIPv4AddressVector m_gatewayList;
    time_t m_leaseObtained;
    time_t m_leaseExpires;
    MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------*/

typedef CMacNetworkInterface::CMacNetworkInterfacePtr  CMacNetworkInterfacePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CMACNETWORKINTERFACE_H ? */
