/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_PUBSUB_CPUBSUBFLOWROUTER_H
#define GUCEF_PUBSUB_CPUBSUBFLOWROUTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#include "gucefPUBSUB_CPubSubClientSide.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBFLOWROUTERCONFIG_H
#include "gucefPUBSUB_CPubSubFlowRouterConfig.h"
#define GUCEF_PUBSUB_CPUBSUBFLOWROUTERCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBFLOWROUTERCONFIG_H ? */

#ifndef GUCEF_PUBSUB_PUBSUBROUTETYPES_H
#include "gucefPUBSUB_PubSubRouteTypes.h"
#define GUCEF_PUBSUB_PUBSUBROUTETYPES_H
#endif /* GUCEF_PUBSUB_PUBSUBROUTETYPES_H ? */

#ifndef GUCEF_PUBSUB_MACROS_H
#include "gucefPUBSUB_macros.h"
#define GUCEF_PUBSUB_MACROS_H
#endif /* GUCEF_PUBSUB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class which aims to provide a config driven mapping between pubsub sides
 *  The mapping entails various information flows and behaviour between said sides
 */
class GUCEF_PUBSUB_EXPORT_CPP CPubSubFlowRouter : public CORE::CTSGNotifier
{
    public:

    CPubSubFlowRouter( void );

    virtual ~CPubSubFlowRouter();

    bool SaveConfig( CPubSubFlowRouterConfig& cfg ) const;

    bool LoadConfig( const CPubSubFlowRouterConfig& src );

    void ClearRoutes( void );
    
    bool BuildRoutes( const CPubSubFlowRouterConfig& config ,
                      TPubSubClientSidePtrVector& sides     ,
                      bool attemptConnect                   );

    bool IsTrackingInFlightPublishedMsgsForAcksNeeded( CPubSubClientSide* sideWeAskFor ) const;

    bool ShouldSideBeConnected( CPubSubClientSide* sideWeAskFor ) const;

    bool PublishMsgs( CPubSubClientSide* fromSide                          ,
                      const CPubSubClientTopic::TPubSubMsgsRefVector& msgs ,
                      RouteType routeType = RouteType::Primary             );
    
    bool AcknowledgeReceiptForSide( CPubSubClientSide* msgReceiverSide ,
                                    CIPubSubMsg::TNoLockSharedPtr& msg );
    
    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    
    static CPubSubClientSidePtr GetSideWithId( const TPubSubClientSidePtrVector& sides ,
                                               const CORE::CString& sideId             );
    
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus ReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const;

    virtual MT::TLockStatus ReadOnlyUnlock( void ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< CPubSubFlowRouter >                                TEventCallback;
    typedef std::vector< CPubSubClientSidePtr, gucef_allocator< CPubSubClientSidePtr > >    TPubSubClientSideVector;

    class CSpilloverInfo;

    class CRouteTopicLinks
    {
        public:
        
        CPubSubClientTopic* fromTopic;
        CPubSubClientTopic* activeTopic;
        RouteType activeTopicType;

        CPubSubClientTopic* toTopic;
        CPubSubClientTopic* failoverTopic;
        CPubSubClientTopic* spilloverTopic;
        CPubSubClientTopic* deadletterTopic;

        CRouteTopicLinks( void );
        CRouteTopicLinks( const CRouteTopicLinks& src );
    };
    typedef std::pair< const CPubSubClientTopic*, CRouteTopicLinks* >        TTopicRawPtrAndRouteTopicLinksPtrPair;
    typedef std::pair< const CORE::CString, CRouteTopicLinks >               TStringAndRouteTopicLinksPair;
    typedef std::map< CPubSubClientTopic*, CRouteTopicLinks*, std::less< CPubSubClientTopic* >, gucef_allocator< TTopicRawPtrAndRouteTopicLinksPtrPair > >  TTopicRawPtrToRouteTopicLinksRawPtrMap;
    typedef std::map< CORE::CString, CRouteTopicLinks, std::less< CORE::CString >, gucef_allocator< TStringAndRouteTopicLinksPair > >                       TStringToRouteTopicLinksMap;
    
    class CRouteInfo
    {
        public:
        CPubSubClientSide* fromSide;
        CPubSubClientSide* activeSide;
        RouteType activeSideType;
        
        CPubSubClientSide* toSide;
        bool toSideIsHealthy;
        CORE::CDateTime toSideLastHealthStatusChange;        

        CPubSubClientSide* failoverSide;
        bool failoverSideIsHealthy;
        CORE::CDateTime failoverSideLastHealthStatusChange;

        CPubSubClientSide* spilloverBufferSide;
        bool spilloverBufferSideIsHealthy;        
        CORE::CDateTime spilloverBufferSideLastHealthStatusChange;
        bool flowingIntoSpillover;
        bool isSpilloverEgressRoute;

        CPubSubClientSide* deadLetterSide;
        bool deadLetterSideIsHealthy;
        CORE::CDateTime deadLetterSideLastHealthStatusChange;

        CORE::CTimer routeSwitchingTimer;
        CSpilloverInfo* spilloverInfo;
        CPubSubFlowRouteConfigPtr routeConfig;

        TTopicRawPtrToRouteTopicLinksRawPtrMap    fromSideTopicLinks;
        TStringToRouteTopicLinksMap               fromSideTopicNameLinks;

        bool IsSpilloverEgressActive( void ) const;
        bool IsSpilloverEgressOngoing( void ) const;
        bool IsSpilloverIngressOngoing( void ) const;
        bool DidMsgsFlowIntoSpillover( void ) const;
        bool IsSpilloverInActiveUse( void ) const;
        void SwitchAllTopicLinksActiveTopic( RouteType activeSide );
        void SwitchTopicLinksActiveTopic( RouteType activeSide, CRouteTopicLinks& topicLinks );
        bool MatchTopicRouteConfig( const CPubSubFlowRouteTopicConfigPtr topicRouteConfig , 
                                    CORE::PulseGeneratorPtr fromPulseGenerator            , 
                                    CORE::PulseGeneratorPtr destPulseGenerator            );
        bool MatchTopicRouteConfig( CPubSubClientTopic* topicAccess            , 
                                    CORE::PulseGeneratorPtr destPulseGenerator );
        bool MatchTopicRouteConfig( const CPubSubFlowRouteTopicConfigPtr topicRouteConfig );
        bool MatchAllTopicRouteConfigs( void );

        CRouteInfo( void );
        CRouteInfo( const CRouteInfo& src );
    };
    typedef std::vector< CRouteInfo, gucef_allocator< CRouteInfo > >        TRouteInfoVector;
    typedef std::vector< CRouteInfo*, gucef_allocator< CRouteInfo* > >      TRouteInfoPtrVector;
    typedef std::set< CRouteInfo*, std::less< CRouteInfo* >, gucef_allocator< CRouteInfo* > >                       TRouteInfoPtrSet;
    typedef std::set< CPubSubClientSide*, std::less< CPubSubClientSide* >, gucef_allocator< CPubSubClientSide* > >  TPubSubClientSidePtrSet;

    class CSpilloverInfo
    {
        public:
        CORE::UInt64 spilloverIngressMsgCount;
        CORE::UInt64 spilloverEgressMsgCount;
        mutable bool endOfDataEventOccured;
        bool endOfDataEventSupported;
        bool supportsDiscoveryOfAvailableTopics;
        bool msgsFlowedIntoSpillover;
        CRouteInfo* route;

        CSpilloverInfo( void );

        bool IsEgressOngoing( void ) const;
        bool IsEgressActive( void ) const;
        bool IsIngressOngoing( void ) const;
        bool DidMsgsFlowIntoSpillover( void ) const;
        bool IsInActiveUse( void ) const;
    };

    typedef std::pair< const CPubSubClientSide*, TRouteInfoVector >         TPubSubClientRawPtrAndRouteInfoVectorPair; 
    typedef std::pair< const CPubSubClientSide*, TRouteInfoPtrVector >      TPubSubClientRawPtrAndRouteInfoPtrVectorPair;
    typedef std::pair< const CPubSubClientSide*, TRouteInfoPtrSet >         TPubSubClientRawPtrAndRouteInfoPtrSetPair;
    typedef std::pair< const CPubSubClientSide*, CSpilloverInfo >           TPubSubClientRawPtrAndSpilloverInfoPair;
    typedef std::pair< const CPubSubClientSide*, CSpilloverInfo* >          TPubSubClientRawPtrAndSpilloverInfoRawPtrPair;
    typedef std::pair< const CORE::CString, CORE::CString >                 TStringPair;
    typedef std::map< CPubSubClientSide*, TRouteInfoVector, std::less< CPubSubClientSide* >, gucef_allocator< TPubSubClientRawPtrAndRouteInfoVectorPair > >         TSidePtrToRouteInfoVectorMap;
    typedef std::map< CPubSubClientSide*, TRouteInfoPtrVector, std::less< CPubSubClientSide* >, gucef_allocator< TPubSubClientRawPtrAndRouteInfoPtrVectorPair > >   TSidePtrToRouteInfoPtrVectorMap;
    typedef std::map< CPubSubClientSide*, TRouteInfoPtrSet, std::less< CPubSubClientSide* >, gucef_allocator< TPubSubClientRawPtrAndRouteInfoPtrSetPair > >         TSidePtrToRouteInfoPtrSetMap;
    typedef std::map< CPubSubClientSide*, CSpilloverInfo, std::less< CPubSubClientSide* >, gucef_allocator< TPubSubClientRawPtrAndSpilloverInfoPair > >             TSidePtrToSpilloverInfoMap;
    typedef std::map< CPubSubClientSide*, CSpilloverInfo*, std::less< CPubSubClientSide* >, gucef_allocator< TPubSubClientRawPtrAndSpilloverInfoRawPtrPair >  >     TSidePtrToSpilloverInfoPtrMap;
    typedef std::map< CORE::CString, CORE::CString, std::less< CORE::CString >, gucef_allocator< TStringPair > >  TStringMap;

    bool NormalizeConfig( const CPubSubFlowRouterConfig& originalConfig ,
                          TPubSubClientSidePtrVector& sides             ,
                          CPubSubFlowRouterConfig& normalizedConfig     );

    bool IsTrackingInFlightPublishedMsgsForAcksNeededForSide( const CPubSubClientSide* side ) const;
    
    void
    OnSidePubSubClientInstantiation( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData );

    void
    OnSidePubSubClientTopicCreation( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData );

    void
    OnSidePubSubClientTopicsAutoCreation( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData );    
    
    void
    OnSidePubSubClientTopicEndOfData( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData );

    void
    OnSidePubSubClientTopicEndOfData( CPubSubClientTopic* topic );
    
    void
    OnSideHealthStatusChange( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData );
    
    void RegisterSideEventHandlers( CPubSubClientSide* side );

    void RegisterSidePubSubClientEventHandlers( CPubSubClientPtr& client );

    void RegisterSidePubSubClientTopicEventHandlers( CPubSubClientTopicBasicPtr topicAccess );

    void RegisterSidePubSubClientTopicEventHandlers( CPubSubClientPtr& client );

    void RegisterRouteEventHandlers( CRouteInfo& routeInfo );
    
    void UpdatePulseGeneratorUsage( void );

    void DetermineActiveRoutes( bool attemptConnect );

    void DetermineActiveRoute( CRouteInfo& routeInfo, bool attemptConnect );

    void OnRouteSwitchTimerCycle( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData );

    void 
    UpdateRoutesBasedOnSideHealthStatus( CPubSubClientSide* side ,
                                         bool isHealthy          );
    
    void CreateNewFromTopicAssociationAsNeeded( CPubSubClientTopicBasicPtr topicAccess, bool attemptConnect );
    
    bool ConfigureSpillover( CPubSubClientSide* spilloverSide, bool flowIntoSpillover, bool attemptConnect );

    bool ConnectRoutesForFromSide( CPubSubClientSide* fromSide );

    bool ConnectSide( CPubSubClientSide* sideToConnect );

    void DetermineFirstActiveRoute( CRouteInfo& routeInfo, bool attemptConnect );

    void DetermineActiveRouteDestination( CRouteInfo& routeInfo, bool attemptConnect );

    CRouteTopicLinks* GetTargetTopicLinks( CPubSubClientTopic* fromTopic ,
                                           CRouteInfo& routeInfo         ,
                                           bool& errorOccured            );
    
    private:
    
    CPubSubFlowRouterConfig m_config;
    CPubSubFlowRouterConfig m_normalizedConfig;
    TSidePtrToRouteInfoVectorMap m_routeMap;
    TSidePtrToRouteInfoPtrVectorMap m_usedInRouteMap;
    TSidePtrToSpilloverInfoMap m_spilloverInfoMap;
    TSidePtrToSpilloverInfoPtrMap m_spilloverInfoForTargetsMap;
    bool m_busyDeterminingActiveRoutes;
    MT::CReadWriteLock m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBFLOWROUTER_H ? */

