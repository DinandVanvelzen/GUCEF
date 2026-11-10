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

#ifndef GUCEF_COM_CUDPBROADCASTDISCOVERYCLIENT_H
#define GUCEF_COM_CUDPBROADCASTDISCOVERYCLIENT_H

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
 *  Discovery with UDP broadcasts: to 255.255.255.255 and to the directed broadcast address of every network
 *  interface (address | ~mask), by default on port 47800. IPv4 only and limited to the subnet.
 *
 *  Settings (LoadConfig): see CUdpDiscoveryClientBase, plus useLimitedBroadcast (send to 255.255.255.255,
 *  default true) and useDirectedBroadcasts (send to the broadcast address of every interface, default true)
 */
class GUCEF_COM_PUBLIC_CPP CUdpBroadcastDiscoveryClient : public CUdpDiscoveryClientBase
{
    public:

    static const CORE::CString TypeName;    /**< "broadcast" */

    CUdpBroadcastDiscoveryClient( void );

    explicit CUdpBroadcastDiscoveryClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CUdpBroadcastDiscoveryClient();

    void SetUseLimitedBroadcast( bool useLimitedBroadcast );

    void SetUseDirectedBroadcasts( bool useDirectedBroadcasts );

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OpenSocket( COMCORE::CUDPSocket& socket, UInt16 port ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDestinations( TIPv4AddressVector& destinations ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CUdpBroadcastDiscoveryClient( const CUdpBroadcastDiscoveryClient& src );
    CUdpBroadcastDiscoveryClient& operator=( const CUdpBroadcastDiscoveryClient& src );

    private:

    bool m_useLimitedBroadcast;
    bool m_useDirectedBroadcasts;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIDiscoveryClient, CUdpBroadcastDiscoveryClient, MT::CMutex > TUdpBroadcastDiscoveryClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CUDPBROADCASTDISCOVERYCLIENT_H ? */
