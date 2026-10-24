/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_DRN_CDRNSESSIONSETTINGS_H
#define GUCEF_DRN_CDRNSESSIONSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRN_CDRNSECURITYPOLICY_H
#include "gucefDRN_CDRNSecurityPolicy.h"
#define GUCEF_DRN_CDRNSECURITYPOLICY_H
#endif /* GUCEF_DRN_CDRNSECURITYPOLICY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Settings of a CDRNSession, section "DRNSessionSettings" of a config.
 *  The topology is decided by the host; a joining member takes it from the host.
 */
class GUCEF_DRN_EXPORT_CPP CDRNSessionSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    CORE::CString sessionName;
    TDRNTopology topology;                  /**< CLIENT_SERVER (default) or PEER_TO_PEER */
    UInt32 maxMembers;                      /**< 0 = no limit */
    CDRNEndpointList advertisedEndpoints;   /**< where other members can reach this one, empty = the node's advertised endpoints */
    UInt32 joinTimeoutMs;                   /**< default 10000 */
    UInt32 maxBytesPerTickPerLink;          /**< what the grids and streams may send over one link in one tick, 0 = no limit */
    bool publishLatency;                    /**< the sequencer publishes the round trip time to every member in the grid "drn.latency" */
    UInt32 latencyPublishIntervalMs;        /**< default 1000 */
    CDRNSecurityPolicy securityPolicy;      /**< the game's policy for the session: relay only, encryption, who may join over what */

    CDRNSessionSettings( void );
    CDRNSessionSettings( const CDRNSessionSettings& src );
    virtual ~CDRNSessionSettings();
    CDRNSessionSettings& operator=( const CDRNSessionSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CORE::CString TopologyToString( TDRNTopology topology );
    static TDRNTopology StringToTopology( const CORE::CString& topology, TDRNTopology defaultTopology );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNSESSIONSETTINGS_H ? */
