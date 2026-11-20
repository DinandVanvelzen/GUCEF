/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_SETTINGS_H
#define GUCEF_LABLINK_SETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_WebSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What both roles work with; see PrintUsage() of the main program for the options that set it */
struct SSettings
{
    GUCEF::CORE::CString stateDir;              /**< secret, identity, approvals */
    GUCEF::CORE::CString clusterName;           /**< cluster: the name it announces; default the host name */
    GUCEF::UInt16 labPort;
    GUCEF::UInt16 accessPort;
    CAddressRanges allowedRanges;               /**< peers outside them are not answered and not used */
    bool useMdns;
    bool useBroadcast;
    GUCEF::UInt16 broadcastPort;

    // cluster
    GUCEF::CORE::CString joinCommand;           /**< agent-node/join-command.sh */
    GUCEF::CORE::CString k3sBin;
    GUCEF::CORE::CString mcpDir;                /**< /var/lib/gucef-mcp: tls/ca.crt, lab-token, mcp-nodes; machine: ci/ */

    // machine
    GUCEF::CORE::CString machineName;           /**< the name this machine gives itself; default the host name */
    bool join;                                  /**< join the cluster as a node when this machine is not one */
    bool ephemeral;                             /**< join as an ephemeral node */
    GUCEF::CORE::CString joinScript;            /**< agent-node/join.sh */
    GUCEF::CORE::CString nodeMarkerFile;        /**< exists when this machine is a k3s node already: agent-node/join.sh writes it once the agent runs */
    GUCEF::CORE::CString claudeUser;            /**< the user whose Claude Code is set up; empty: none */
    GUCEF::CORE::CString claudeCommand;         /**< the claude program; empty: looked up for the user */
    GUCEF::CORE::CString claudeHome;            /**< where the CA file and the line of the profile go; empty: the home of the user */
    GUCEF::CORE::CString localMcpNode;          /**< the node name of this machine, whose MCP service is registered locally; empty: none */
    GUCEF::UInt32 refreshIntervalInSecs;        /**< how often a linked machine asks the cluster again */
    GUCEF::UInt32 retryIntervalInSecs;          /**< how often a machine that waits for approval asks again */

    // hub storage: where the cluster keeps what it learns of its nodes
    GUCEF::CORE::CString dbType;                /**< "vfstables" (tables as files in a directory), "postgresql" or "none": the nodes are kept in memory only */
    GUCEF::CORE::CString dbDirectory;           /**< vfstables: the directory, default "db" in the state directory */
    GUCEF::CORE::CString dbHost;                /**< postgresql: */
    GUCEF::UInt16 dbPort;                       /**< postgresql: 0 is the port of the server */
    GUCEF::CORE::CString dbName;
    GUCEF::CORE::CString dbUser;
    GUCEF::CORE::CString dbSslMode;
    GUCEF::CORE::CString dbSchema;
    GUCEF::CORE::CString dbPasswordFile;        /**< postgresql: the password is read from this file, never given on the command line or in the settings */
    GUCEF::UInt32 eventRetentionInDays;         /**< how long the history of the nodes is kept */
    GUCEF::UInt32 maxStoredEvents;              /**< and how many of its events at most */

    // node reports
    bool report;                                /**< machine: tell the hub about this node; a cluster that does not know the call is left alone */
    GUCEF::UInt32 reportIntervalInSecs;         /**< machine: how often; hub: the interval its nodes are told, and a third of the time after which a silent node is offline */

    // the website of the hub (cluster), off unless enabled
    SWebSettings web;

    // the CI configuration (gucefMcpCi): the hub hands it with a gateway token to approved machines
    GUCEF::CORE::CString ciRegistryFile;        /**< cluster: the CI registry (JSON); empty: ci.json in the state directory. No file: no CI */
    GUCEF::CORE::CString ciGatewayUrl;          /**< cluster: the URL of the CI gateway; empty: https://<cluster name>:8792 */
    GUCEF::CORE::CString ciGatewayTokensFile;   /**< cluster: the hashes of the gateway tokens, for the gateway; empty: ci/gateway-tokens in the state directory */
    GUCEF::CORE::CString ciGatewayUser;         /**< cluster: the user the gateway runs as, who gets that file (Linux); empty: the account of the hub */
    bool ci;                                    /**< machine: ask the cluster for the CI configuration and write it to <mcp dir>/ci */
    GUCEF::UInt32 offeredLabMinor;              /**< cluster: the minor of gucef.lab it offers; lower only to test machines against an older hub */

    SSettings( void )
        : stateDir( "/var/lib/gucef-lablink" )
        , clusterName()
        , labPort( DefaultLabPort )
        , accessPort( DefaultAccessPort )
        , allowedRanges()
        , useMdns( true )
        , useBroadcast( true )
        , broadcastPort( 47812 )
        , joinCommand()
        , k3sBin( "/usr/local/bin/k3s" )
        , mcpDir( "/var/lib/gucef-mcp" )
        , machineName()
        , join( false )
        , ephemeral( true )
        , joinScript()
        , nodeMarkerFile( "/etc/rancher/k3s/gucef-node-joined" )
        , claudeUser()
        , claudeCommand()
        , claudeHome()
        , localMcpNode()
        , refreshIntervalInSecs( 600 )
        , retryIntervalInSecs( 30 )
        , dbType( "vfstables" )
        , dbDirectory()
        , dbHost()
        , dbPort( 0 )
        , dbName( "gucef_lablink" )
        , dbUser()
        , dbSslMode()
        , dbSchema()
        , dbPasswordFile()
        , eventRetentionInDays( 30 )
        , maxStoredEvents( 20000 )
        , report( true )
        , reportIntervalInSecs( 20 )
        , web()
        , ciRegistryFile()
        , ciGatewayUrl()
        , ciGatewayTokensFile()
        , ciGatewayUser()
        , ci( true )
        , offeredLabMinor( LabContractMinor )
    {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_SETTINGS_H ? */
