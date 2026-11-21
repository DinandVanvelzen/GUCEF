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

#ifndef GUCEF_LABLINK_HUBVIEW_H
#define GUCEF_LABLINK_HUBVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefLabLink_NodeReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the hub knows of whether a node is there */
extern const char* HubNodeOnline;
extern const char* HubNodeOffline;
extern const char* HubNodeUnknown;          /**< no report since the hub started */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A node as the hub shows it */
struct SHubNode
{
    SNodeReport report;                         /**< the last report the hub accepted */
    bool verified;                              /**< reported with the token of its approval */
    GUCEF::CORE::CString address;
    GUCEF::CORE::CString status;                /**< HubNode... */
    GUCEF::UInt64 firstSeenInUnixSecs;          /**< the clock of the hub */
    GUCEF::UInt64 lastSeenInUnixSecs;
    GUCEF::UInt32 duplicateInstanceRefusals;    /**< reports of a second instance of the machine id that were refused */

    SHubNode( void )
        : report(), verified( false ), address(), status( HubNodeUnknown ), firstSeenInUnixSecs( 0 ), lastSeenInUnixSecs( 0 )
        , duplicateInstanceRefusals( 0 )
    {}
};

/** A service a node offers that clients can use: what a workload of the node publishes */
struct SHubEndpoint
{
    GUCEF::CORE::CString kind;                  /**< "llm", "image", "tts", "stt", "mcp", "generic" */
    GUCEF::CORE::CString machineId;
    GUCEF::CORE::CString node;                  /**< the name of the node */
    GUCEF::CORE::CString url;
    GUCEF::CORE::CString model;
    GUCEF::CORE::CString workloadId;
    GUCEF::UInt32 idleSlots;

    SHubEndpoint( void ) : kind(), machineId(), node(), url(), model(), workloadId(), idleSlots( 0 ) {}
};

/**
 *  What the website reads of the hub. The website uses the hub through this view only, so that it can run in the
 *  process of the hub (CRegistryHubView) or, later, in a process of its own that asks the hub over a link. Read only
 *  for now; changes of workloads and policies are added with the workloads. Every call can fail, as a call over a
 *  link can: false means the hub could not be asked, not that there is nothing.
 */
class ILabHubView
{
    public:

    ILabHubView( void );

    virtual ~ILabHubView();

    /** The name the hub announces */
    virtual GUCEF::CORE::CString GetHubName( void ) const = 0;

    /** The wall clock of the hub, in unix seconds: the times of the nodes are on it */
    virtual GUCEF::UInt64 GetHubTimeInUnixSecs( void ) const = 0;

    /** Every node the hub knows */
    virtual bool GetNodes( GUCEF::vector< SHubNode >& nodes ) const = 0;

    /** @param found set to whether the hub knows the node */
    virtual bool GetNode( const GUCEF::CORE::CString& machineId, SHubNode& node, bool& found ) const = 0;

    /** The services the nodes that are online offer */
    virtual bool GetEndpoints( GUCEF::vector< SHubEndpoint >& endpoints ) const = 0;

    private:

    ILabHubView( const ILabHubView& src );
    ILabHubView& operator=( const ILabHubView& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_HUBVIEW_H ? */
