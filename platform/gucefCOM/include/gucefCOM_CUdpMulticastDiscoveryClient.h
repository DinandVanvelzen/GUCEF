/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CUDPMULTICASTDISCOVERYCLIENT_H
#define GUCEF_COM_CUDPMULTICASTDISCOVERYCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H
#include "gucefCOM_CUdpDiscoveryClientBase.h"
#define GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H
#endif /* GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Discovery with UDP multicast: to group 239.255.47.80 on port 47800, joined on every network interface, with a
 *  multicast TTL of 1 so it stays on the subnet. A higher TTL crosses routers that forward multicast.
 *
 *  Settings (LoadConfig): see CUdpDiscoveryClientBase, plus group, multicastTtl and allowLoopback (whether
 *  instances on the same host hear each other, default true)
 */
class GUCEF_COM_PUBLIC_CPP CUdpMulticastDiscoveryClient : public CUdpDiscoveryClientBase
{
    public:

    static const CORE::CString TypeName;            /**< "multicast" */
    static const CORE::CString DefaultGroup;        /**< 239.255.47.80 */

    CUdpMulticastDiscoveryClient( void );

    explicit CUdpMulticastDiscoveryClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CUdpMulticastDiscoveryClient();

    bool SetGroup( const CORE::CString& group );

    const COMCORE::CIPv4Address& GetGroup( void ) const;

    void SetMulticastTtl( Int32 ttl );

    Int32 GetMulticastTtl( void ) const;

    void SetAllowLoopback( bool allowLoopback );

    /** The number of interfaces the group was joined on when the socket opened */
    UInt32 GetJoinedInterfaceCount( void ) const;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OpenSocket( COMCORE::CUDPSocket& socket, UInt16 port ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDestinations( TIPv4AddressVector& destinations ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnIdle( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CUdpMulticastDiscoveryClient( const CUdpMulticastDiscoveryClient& src );
    CUdpMulticastDiscoveryClient& operator=( const CUdpMulticastDiscoveryClient& src );

    private:

    COMCORE::CIPv4Address m_group;
    Int32 m_multicastTtl;
    bool m_allowLoopback;
    UInt32 m_joinedInterfaceCount;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIDiscoveryClient, CUdpMulticastDiscoveryClient, MT::CMutex > TUdpMulticastDiscoveryClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CUDPMULTICASTDISCOVERYCLIENT_H ? */
