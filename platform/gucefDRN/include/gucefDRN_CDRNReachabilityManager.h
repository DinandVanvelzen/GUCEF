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

#ifndef GUCEF_DRN_CDRNREACHABILITYMANAGER_H
#define GUCEF_DRN_CDRNREACHABILITYMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COMCORE_CIPORTMAPPER_H
#include "gucefCOMCORE_CIPortMapper.h"
#define GUCEF_COMCORE_CIPORTMAPPER_H
#endif /* GUCEF_COMCORE_CIPORTMAPPER_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNELLISTENER_H
#include "gucefDRN_CIDRNChannelListener.h"
#define GUCEF_DRN_CIDRNCHANNELLISTENER_H
#endif /* GUCEF_DRN_CIDRNCHANNELLISTENER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

namespace COM {
class CStunClient;
}

namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNNode;

/**
 *  What the reachability manager asks for, section "DRNReachabilitySettings" of a config
 */
class GUCEF_DRN_EXPORT_CPP CDRNReachabilitySettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    /** The configuration of one port mapper of the COMCORE port mapper factory */
    class GUCEF_DRN_EXPORT_CPP CPortMapperConfig
    {
        public:
        CORE::CString name;             /**< the factory name: "natpmp" (built into COM), "upnp" (plugin) */
        CORE::CDataNode config;         /**< passed to the mapper's LoadConfig, for example the gateway */
    };

    typedef GUCEF::vector< CPortMapperConfig > TPortMapperConfigVector;
    typedef GUCEF::vector< CORE::CString > TStringVector;

    TPortMapperConfigVector portMappers;
    TStringVector stunServers;          /**< host:port */
    CDRNEndpointList relays;            /**< relay services to bind at, as udp://host:port or tcp://host:port */
    CORE::CString relayRealm;
    UInt32 mappingLeaseInSecs;          /**< default 3600 */
    UInt32 timeoutMs;                   /**< after this the questions without an answer count as failed, default 5000 */

    CDRNReachabilitySettings( void );
    CDRNReachabilitySettings( const CDRNReachabilitySettings& src );
    virtual ~CDRNReachabilitySettings();
    CDRNReachabilitySettings& operator=( const CDRNReachabilitySettings& src );

    void AddPortMapper( const CORE::CString& name, const CORE::CDataNode& config );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Finds out how others can reach a node and builds its endpoint list from it:
 *  - the endpoints the node listens on (LAN);
 *  - the public endpoints the configured COMCORE port mappers ("natpmp", "upnp") map the UDP and TCP listeners to;
 *  - the reflexive address of every UDP listener as COM's STUN client learns it;
 *  - the relay endpoints the node binds under at the configured relays, as the last resort.
 *  A connector tries endpoints in that order. The list becomes the node's advertised endpoints, which its sessions
 *  tell the other members (unless CDRNSessionSettings::advertisedEndpoints says otherwise); Stop() gives the node its
 *  listen endpoints back. Works on the node's pulse, ReachabilityChangedEvent when the list changed.
 *  The manager goes before its node does.
 */
class GUCEF_DRN_EXPORT_CPP CDRNReachabilityManager : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent ReachabilityChangedEvent;    /**< the endpoint list changed */
    static const CORE::CEvent ReachabilityCompleteEvent;   /**< every question has been answered or timed out */

    static void RegisterEvents( void );

    explicit CDRNReachabilityManager( CDRNNode& node );

    virtual ~CDRNReachabilityManager();

    /** Starts asking; the node should be listening */
    bool Start( const CDRNReachabilitySettings& settings );

    void Stop( void );

    bool IsComplete( void ) const;

    /** LAN, mapped, reflexive, relay; without doubles */
    CDRNEndpointList GetEndpoints( void ) const;

    CDRNEndpointList GetMappedEndpoints( void ) const;
    CDRNEndpointList GetReflexiveEndpoints( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CInternalObserver : public CORE::CObserver
    {
        public:
        CDRNReachabilityManager* m_manager;
        CInternalObserver( void ) : CORE::CObserver(), m_manager( GUCEF_NULL ) {}
        virtual ~CInternalObserver() { SignalUpcomingObserverDestruction(); }
        protected:
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    };
    friend class CInternalObserver;

    class CQuestion
    {
        public:
        CORE::CFutureResult future;
        bool isStun;
        CORE::CString channelType;      /**< of the listener asked about */
        CORE::CString path;             /**< of the listener asked about (web socket) */
        bool answered;

        CQuestion( void );
    };

    typedef GUCEF::vector< CQuestion > TQuestionVector;
    typedef GUCEF::vector< COMCORE::CIPortMapperPtr > TMapperVector;
    typedef GUCEF::vector< COM::CStunClient* > TStunClientVector;
    typedef GUCEF::vector< CIDRNChannelListenerPtr > TListenerVector;

    CDRNReachabilityManager( const CDRNReachabilityManager& src );
    CDRNReachabilityManager& operator=( const CDRNReachabilityManager& src );

    void OnPulse( void );
    void Emit( const CORE::CEvent& eventid );

    CDRNNode* m_node;
    CInternalObserver m_observer;
    CDRNReachabilitySettings m_settings;
    TMapperVector m_mappers;
    TStunClientVector m_stunClients;
    TListenerVector m_stunListeners;    /**< keep the sockets of the STUN clients */
    TQuestionVector m_questions;
    CDRNEndpointList m_mapped;
    CDRNEndpointList m_reflexive;
    UInt64 m_deadline;
    bool m_started;
    bool m_complete;
    CORE::CString m_lastList;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNREACHABILITYMANAGER_H ? */
