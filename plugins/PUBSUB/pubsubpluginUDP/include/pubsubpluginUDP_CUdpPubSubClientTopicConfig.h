/*
 *  pubsubpluginUDP: Generic GUCEF COMCORE plugin for providing pubsub approximation via UDP
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

#ifndef PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPICCONFIG_H
#define PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPICCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#include "gucefPUBSUB_CPubSubClientTopicConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef PUBSUBPLUGIN_UDP_MACROS_H
#include "pubsubpluginUDP_macros.h"
#define PUBSUBPLUGIN_UDP_MACROS_H
#endif /* PUBSUBPLUGIN_UDP_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace UDP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CUdpPubSubClient;

/**
 *  Standard pub-sub client config with some custom specifcs added for this
 *  specific pub-sub backend
 */
class PUBSUBPLUGIN_UDP_PLUGIN_PRIVATE_CPP CUdpPubSubClientTopicConfig : public PUBSUB::CPubSubClientTopicConfig ,
                                                                        public CORE::CTSharedObjCreator< CUdpPubSubClientTopicConfig, MT::CMutex >
{
    public:

    typedef GUCEF::vector< COMCORE::CHostAddress > HostAddressVector;

    CUdpPubSubClientTopicConfig( void );
    
    CUdpPubSubClientTopicConfig( const CUdpPubSubClientTopicConfig& src );

    CUdpPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig );

    virtual ~CUdpPubSubClientTopicConfig() GUCEF_VIRTUAL_OVERRIDE;

    CUdpPubSubClientTopicConfig& operator=( const PUBSUB::CPubSubClientTopicConfig& src );

    CUdpPubSubClientTopicConfig& operator=( const CUdpPubSubClientTopicConfig& src );

    bool LoadCustomConfig( const CORE::CDataNode& config );

    bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    COMCORE::CHostAddress udpInterface;
    HostAddressVector udpMulticastToJoin;
    bool wantsTestPackage;
    CORE::UInt32 ticketRefillOnBusyCycle;
    CORE::UInt32 nrOfUdpReceiveBuffersPerSocket;
    CORE::UInt32 udpSocketOsReceiveBufferSize;
    CORE::UInt32 udpSocketUpdateCyclesPerPulse;
    bool addUdpSourceAddressAsMetaData;
    bool translateUdpSourceAddressToString;
    CORE::CString udpSourceAddressAsMetaDataKeyName;
    CORE::UInt16 maxUdpPacketPayloadSizeInBytes;
    bool addTimestampToReceivedPackages;
};

/*-------------------------------------------------------------------------*/

typedef CUdpPubSubClientTopicConfig::TSharedPtrType     CUdpPubSubClientTopicConfigPtr;  

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UDP */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPICCONFIG_H ? */
