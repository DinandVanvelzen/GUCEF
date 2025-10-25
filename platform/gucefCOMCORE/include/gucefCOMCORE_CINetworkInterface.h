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

#ifndef GUCEF_COMCORE_CINETWORKINTERFACE_H
#define GUCEF_COMCORE_CINETWORKINTERFACE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_COMCORE_CICOMMUNICATIONINTERFACE_H
#include "gucefCOMCORE_CICommunicationInterface.h"
#define GUCEF_COMCORE_CICOMMUNICATIONINTERFACE_H
#endif /* GUCEF_COMCORE_CICOMMUNICATIONINTERFACE_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_COMCORE_CIPV4INFO_H
#include "gucefCOMCORE_CIPv4Info.h"
#define GUCEF_COMCORE_CIPV4INFO_H
#endif /* GUCEF_COMCORE_CIPV4INFO_H ? */

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

class GUCEF_COMCORE_EXPORT_CPP CNetworkInterfaceMetrics
{
    public:
    
    UInt64 transmitLinkSpeedBitsPerSec;
    bool hasTransmitLinkSpeedBitsPerSec;
    UInt64 receiveLinkSpeedBitsPerSec;
    bool hasReceiveLinkSpeedBitsPerSec;
    UInt64 inboundOctets;
    bool hasInboundOctets;
    UInt64 inboundUnicastPackets;
    bool hasInboundUnicastPackets;
    UInt64 inboundNonUnicastPackets;
    bool hasInboundNonUnicastPackets;
    UInt64 inboundDiscardedPackets;
    bool hasInboundDiscardedPackets;
    UInt64 inboundErroredPackets;
    bool hasInboundErroredPackets;
    UInt64 inboundUnknownProtocolPackets;
    bool hasInboundUnknownProtocolPackets;
    UInt64 inboundUnicastOctets;
    bool hasInboundUnicastOctets;
    UInt64 inboundMulticastOctets;
    bool hasInboundMulticastOctets;
    UInt64 inboundBroadcastOctets;
    bool hasInboundBroadcastOctets;
    UInt64 outboundOctets;
    bool hasOutboundOctets;
    UInt64 outboundUnicastPackets;
    bool hasOutboundUnicastPackets;
    UInt64 outboundNonUnicastPackets;
    bool hasOutboundNonUnicastPackets;
    UInt64 outboundDiscardedPackets;
    bool hasOutboundDiscardedPackets;
    UInt64 outboundErroredPackets;
    bool hasOutboundErroredPackets;
    UInt64 outboundUnknownProtocolPackets;
    bool hasOutboundUnknownProtocolPackets;
    UInt64 outboundUnicastOctets;
    bool hasOutboundUnicastOctets;
    UInt64 outboundMulticastOctets;
    bool hasOutboundMulticastOctets;
    UInt64 outboundBroadcastOctets;
    bool hasOutboundBroadcastOctets;

    CNetworkInterfaceMetrics( void );
};

/*-------------------------------------------------------------------------*/

class GUCEF_COMCORE_EXPORT_CPP CINetworkInterface : public virtual CICommunicationInterface ,
                                                    public virtual MT::CILockable
{
    public:

    typedef CHostAddress::THostAddressVector    THostAddressVector;
    typedef CIPv4Address::TIPv4AddressVector    TIPv4AddressVector;
    typedef CIPv4Info::TIPInfoVector            TIPv4InfoVector;
    typedef CORE::CTBasicSharedPtr< CINetworkInterface, MT::CMutex >                          CINetworkInterfacePtr;
    typedef GUCEF::vector< CINetworkInterfacePtr, gucef_allocator< CINetworkInterfacePtr > >    TINetworkInterfacePtrVector;
    
    virtual ~CINetworkInterface();

    virtual CString GetAdapterName( void ) const = 0;

    virtual CString GetAdapterDescription( void ) const = 0;

    virtual UInt32 GetNrOfDnsAddresses( void ) const = 0;
    
    virtual bool GetDnsAddresses( THostAddressVector& dnsAddresses ) = 0;

    virtual UInt32 GetNrOfIPAddresses( void ) const = 0;

    virtual bool GetIPInfo( TIPv4InfoVector& ipInfo, bool includeUninitialized = false ) const = 0;
		
    virtual bool IsDhcpUsedForIPv4( void ) const = 0;

    virtual bool IsDhcpUsedForIPv6( void ) const = 0;

    virtual CORE::CDateTime GetDhcpLeaseObtainedTime( void ) const = 0;

    virtual CORE::CDateTime GetDhcpLeaseExpirationTime( void ) const = 0;
    
    virtual bool ReleaseAddress( void ) = 0;

    virtual bool RenewAddress( void ) = 0;

    virtual bool IsWinsUsed( void ) const = 0;

    virtual CIPv4Address GetPrimaryWinsServer( void ) const = 0;

    virtual CIPv4Address GetSecondaryWinsServer( void ) const = 0;

    virtual UInt32 GetOsAdapterIndex( void ) const = 0;

    virtual bool GetMetrics( CNetworkInterfaceMetrics& metrics ) const = 0;

    protected:

    CINetworkInterface( void );
    CINetworkInterface( const CINetworkInterface& src );
    CINetworkInterface& operator=( const CINetworkInterface& src );
};

/*-------------------------------------------------------------------------*/

typedef CINetworkInterface::CINetworkInterfacePtr     CINetworkInterfacePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CINETWORKINTERFACE_H ? */
