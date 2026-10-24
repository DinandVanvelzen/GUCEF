/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*
 *      This is the main include header for the gucefDRN library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with RAD networking functionality
 *      The facilities in this library are designed to make it easy to setup
 *      a networking infrastructure without having to worry about the details
 *      of low-level networking.
 */

#ifndef GUCEF_DRN_H
#define GUCEF_DRN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration      
 */
#ifndef GUCEF_DRN_CONFIG_H
#include "gucefDRN_config.h"
#define GUCEF_DRN_CONFIG_H
#endif /* GUCEF_DRN_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_DRN_ETYPES_H
#include "gucefDRN_ETypes.h"
#define GUCEF_DRN_ETYPES_H
#endif /* GUCEF_DRN_ETYPES_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINT_H
#include "gucefDRN_CDRNEndpoint.h"
#define GUCEF_DRN_CDRNENDPOINT_H
#endif /* GUCEF_DRN_CDRNENDPOINT_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNEL_H
#include "gucefDRN_CIDRNChannel.h"
#define GUCEF_DRN_CIDRNCHANNEL_H
#endif /* GUCEF_DRN_CIDRNCHANNEL_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNELLISTENER_H
#include "gucefDRN_CIDRNChannelListener.h"
#define GUCEF_DRN_CIDRNCHANNELLISTENER_H
#endif /* GUCEF_DRN_CIDRNCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNSTREAMFRAMER_H
#include "gucefDRN_CDRNStreamFramer.h"
#define GUCEF_DRN_CDRNSTREAMFRAMER_H
#endif /* GUCEF_DRN_CDRNSTREAMFRAMER_H ? */

#ifndef GUCEF_DRN_CDRNTCPCHANNEL_H
#include "gucefDRN_CDRNTcpChannel.h"
#define GUCEF_DRN_CDRNTCPCHANNEL_H
#endif /* GUCEF_DRN_CDRNTCPCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNTCPCHANNELLISTENER_H
#include "gucefDRN_CDRNTcpChannelListener.h"
#define GUCEF_DRN_CDRNTCPCHANNELLISTENER_H
#endif /* GUCEF_DRN_CDRNTCPCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNUDPCHANNEL_H
#include "gucefDRN_CDRNUdpChannel.h"
#define GUCEF_DRN_CDRNUDPCHANNEL_H
#endif /* GUCEF_DRN_CDRNUDPCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNUDPCHANNELLISTENER_H
#include "gucefDRN_CDRNUdpChannelListener.h"
#define GUCEF_DRN_CDRNUDPCHANNELLISTENER_H
#endif /* GUCEF_DRN_CDRNUDPCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H
#include "gucefDRN_CDRNWebSocketChannel.h"
#define GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H
#endif /* GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNWEBSOCKETCHANNELLISTENER_H
#include "gucefDRN_CDRNWebSocketChannelListener.h"
#define GUCEF_DRN_CDRNWEBSOCKETCHANNELLISTENER_H
#endif /* GUCEF_DRN_CDRNWEBSOCKETCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNPACKETCODEC_H
#include "gucefDRN_CDRNPacketCodec.h"
#define GUCEF_DRN_CDRNPACKETCODEC_H
#endif /* GUCEF_DRN_CDRNPACKETCODEC_H ? */

#ifndef GUCEF_DRN_CDRNLINKRELIABILITY_H
#include "gucefDRN_CDRNLinkReliability.h"
#define GUCEF_DRN_CDRNLINKRELIABILITY_H
#endif /* GUCEF_DRN_CDRNLINKRELIABILITY_H ? */

#ifndef GUCEF_DRN_CDRNLATENCYINFO_H
#include "gucefDRN_CDRNLatencyInfo.h"
#define GUCEF_DRN_CDRNLATENCYINFO_H
#endif /* GUCEF_DRN_CDRNLATENCYINFO_H ? */

#ifndef GUCEF_DRN_CIDRNLATENCYMETER_H
#include "gucefDRN_CIDRNLatencyMeter.h"
#define GUCEF_DRN_CIDRNLATENCYMETER_H
#endif /* GUCEF_DRN_CIDRNLATENCYMETER_H ? */

#ifndef GUCEF_DRN_CDRNLATENCYMETER_H
#include "gucefDRN_CDRNLatencyMeter.h"
#define GUCEF_DRN_CDRNLATENCYMETER_H
#endif /* GUCEF_DRN_CDRNLATENCYMETER_H ? */

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNANONYMOUSAUTHENTICATOR_H
#include "gucefDRN_CDRNAnonymousAuthenticator.h"
#define GUCEF_DRN_CDRNANONYMOUSAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNANONYMOUSAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNSHAREDSECRETAUTHENTICATOR_H
#include "gucefDRN_CDRNSharedSecretAuthenticator.h"
#define GUCEF_DRN_CDRNSHAREDSECRETAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNSHAREDSECRETAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNSECURITYPOLICY_H
#include "gucefDRN_CDRNSecurityPolicy.h"
#define GUCEF_DRN_CDRNSECURITYPOLICY_H
#endif /* GUCEF_DRN_CDRNSECURITYPOLICY_H ? */

#ifndef GUCEF_DRN_CIDRNBANLIST_H
#include "gucefDRN_CIDRNBanList.h"
#define GUCEF_DRN_CIDRNBANLIST_H
#endif /* GUCEF_DRN_CIDRNBANLIST_H ? */

#ifndef GUCEF_DRN_CDRNBANLIST_H
#include "gucefDRN_CDRNBanList.h"
#define GUCEF_DRN_CDRNBANLIST_H
#endif /* GUCEF_DRN_CDRNBANLIST_H ? */

#ifndef GUCEF_DRN_CDRNLINKMONITOR_H
#include "gucefDRN_CDRNLinkMonitor.h"
#define GUCEF_DRN_CDRNLINKMONITOR_H
#endif /* GUCEF_DRN_CDRNLINKMONITOR_H ? */

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#include "gucefDRN_CIDRNAccessPolicy.h"
#define GUCEF_DRN_CIDRNACCESSPOLICY_H
#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRN_CDRNPACKETPROTECTION_H
#include "gucefDRN_CDRNPacketProtection.h"
#define GUCEF_DRN_CDRNPACKETPROTECTION_H
#endif /* GUCEF_DRN_CDRNPACKETPROTECTION_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CDRNCONNECTOR_H
#include "gucefDRN_CDRNConnector.h"
#define GUCEF_DRN_CDRNCONNECTOR_H
#endif /* GUCEF_DRN_CDRNCONNECTOR_H ? */

#ifndef GUCEF_DRN_CDRNNODESETTINGS_H
#include "gucefDRN_CDRNNodeSettings.h"
#define GUCEF_DRN_CDRNNODESETTINGS_H
#endif /* GUCEF_DRN_CDRNNODESETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#include "gucefDRN_CIDRNSessionComponent.h"
#define GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CIDRNSESSIONCOMPONENT_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONSETTINGS_H
#include "gucefDRN_CDRNSessionSettings.h"
#define GUCEF_DRN_CDRNSESSIONSETTINGS_H
#endif /* GUCEF_DRN_CDRNSESSIONSETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONRECORDER_H
#include "gucefDRN_CDRNSessionRecorder.h"
#define GUCEF_DRN_CDRNSESSIONRECORDER_H
#endif /* GUCEF_DRN_CDRNSESSIONRECORDER_H ? */

#ifndef GUCEF_DRN_CDRNVARIANTCODEC_H
#include "gucefDRN_CDRNVariantCodec.h"
#define GUCEF_DRN_CDRNVARIANTCODEC_H
#endif /* GUCEF_DRN_CDRNVARIANTCODEC_H ? */

#ifndef GUCEF_DRN_CDRNVOTEPOLICY_H
#include "gucefDRN_CDRNVotePolicy.h"
#define GUCEF_DRN_CDRNVOTEPOLICY_H
#endif /* GUCEF_DRN_CDRNVOTEPOLICY_H ? */

#ifndef GUCEF_DRN_CIDRNWRITEVALIDATOR_H
#include "gucefDRN_CIDRNWriteValidator.h"
#define GUCEF_DRN_CIDRNWRITEVALIDATOR_H
#endif /* GUCEF_DRN_CIDRNWRITEVALIDATOR_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRIDSETTINGS_H
#include "gucefDRN_CDRNDataGridSettings.h"
#define GUCEF_DRN_CDRNDATAGRIDSETTINGS_H
#endif /* GUCEF_DRN_CDRNDATAGRIDSETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNDATASTREAMSETTINGS_H
#include "gucefDRN_CDRNDataStreamSettings.h"
#define GUCEF_DRN_CDRNDATASTREAMSETTINGS_H
#endif /* GUCEF_DRN_CDRNDATASTREAMSETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNDATAITEM_H
#include "gucefDRN_CDRNDataItem.h"
#define GUCEF_DRN_CDRNDATAITEM_H
#endif /* GUCEF_DRN_CDRNDATAITEM_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CDRNDATASTREAM_H
#include "gucefDRN_CDRNDataStream.h"
#define GUCEF_DRN_CDRNDATASTREAM_H
#endif /* GUCEF_DRN_CDRNDATASTREAM_H ? */

#ifndef GUCEF_DRN_CDRNREPLICATOR_H
#include "gucefDRN_CDRNReplicator.h"
#define GUCEF_DRN_CDRNREPLICATOR_H
#endif /* GUCEF_DRN_CDRNREPLICATOR_H ? */

#ifndef GUCEF_DRN_CDRNBULKTRANSFER_H
#include "gucefDRN_CDRNBulkTransfer.h"
#define GUCEF_DRN_CDRNBULKTRANSFER_H
#endif /* GUCEF_DRN_CDRNBULKTRANSFER_H ? */

#ifndef GUCEF_DRN_CDRNRELAYSERVICE_H
#include "gucefDRN_CDRNRelayService.h"
#define GUCEF_DRN_CDRNRELAYSERVICE_H
#endif /* GUCEF_DRN_CDRNRELAYSERVICE_H ? */

#ifndef GUCEF_DRN_CDRNRELAYCHANNEL_H
#include "gucefDRN_CDRNRelayChannel.h"
#define GUCEF_DRN_CDRNRELAYCHANNEL_H
#endif /* GUCEF_DRN_CDRNRELAYCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNREACHABILITYMANAGER_H
#include "gucefDRN_CDRNReachabilityManager.h"
#define GUCEF_DRN_CDRNREACHABILITYMANAGER_H
#endif /* GUCEF_DRN_CDRNREACHABILITYMANAGER_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#endif /* __cplusplus ? */
/*--------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 21-04-2007 :
       - Made the initial version of the library header.

-----------------------------------------------------------------------------*/
