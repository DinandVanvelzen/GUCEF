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

#ifndef GUCEF_PUBSUB_CPUBSUBFLOWROUTECONFIG_H
#define GUCEF_PUBSUB_CPUBSUBFLOWROUTECONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  Class which hold the configuration representing a pubsub route between named topics of known pubsub 'sides'
 * 
 *  Using the topic names for known sides it logically refers to said topics to spell out a mapping between them that the sides themselves are oblivious of
 *  The flow router uses this information to correctly route data between sides and their associated client's topics for a given channel
 */
class GUCEF_PUBSUB_EXPORT_CPP CPubSubFlowRouteTopicConfig : public CORE::CIConfigurable ,
                                                            public CORE::CTSharedObjCreator< CPubSubFlowRouteTopicConfig, MT::CMutex >
{
    public:

    typedef CORE::CTSharedPtr< CPubSubFlowRouteTopicConfig, MT::CMutex >                                       CPubSubFlowRouteTopicConfigPtr;
    typedef GUCEF::vector< CPubSubFlowRouteTopicConfigPtr, gucef_allocator< CPubSubFlowRouteTopicConfigPtr > >   PubSubFlowRouteTopicConfigPtrVector;

    CORE::CString fromSideTopicName;        /**< primary 'from' side topic for this route */
    CORE::CString toSideTopicName;          /**< primary 'to' side topic for this route */
    CORE::CString failoverSideTopicName;    /**< if the primary flow fails traffic would be rerouted here as a equivelant */ 
    CORE::CString spilloverSideTopicName;   /**< if the primary and failover is unhealthy or is a slow consumer the spill over acts as buffer for the route publishing/subscribing to said side */
    CORE::CString deadLetterSideTopicName;  /**< unable-to-publish messages on configured channels with no remaining remedies go here */

    CPubSubFlowRouteTopicConfig( void );

    CPubSubFlowRouteTopicConfig( const CPubSubFlowRouteTopicConfig& src );

    virtual ~CPubSubFlowRouteTopicConfig();

    CPubSubFlowRouteTopicConfig& operator=( const CPubSubFlowRouteTopicConfig& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

};

/*-------------------------------------------------------------------------*/

/**
 *  Class which hold the configuration representing a pubsub route between pubsub 'sides'
 * 
 *  Using the ids of sides it logically refers to said sides to spell out a mapping between them that the sides themselves are oblivious of
 *  The flow router uses this information to correctly route data between sides for a given channel
 */
class GUCEF_PUBSUB_EXPORT_CPP CPubSubFlowRouteConfig : public CORE::CIConfigurable ,
                                                       public CORE::CTSharedObjCreator< CPubSubFlowRouteConfig, MT::CMutex >
{
    public:
                                                                            
    typedef CORE::CTSharedPtr< CPubSubFlowRouteConfig, MT::CMutex >                                 CPubSubFlowRouteConfigPtr;
    typedef GUCEF::vector< CPubSubFlowRouteConfigPtr, gucef_allocator< CPubSubFlowRouteConfigPtr > >  PubSubFlowRouteConfigPtrVector;                                                
    typedef CPubSubFlowRouteTopicConfig::CPubSubFlowRouteTopicConfigPtr                             CPubSubFlowRouteTopicConfigPtr;
    typedef CPubSubFlowRouteTopicConfig::PubSubFlowRouteTopicConfigPtrVector                        PubSubFlowRouteTopicConfigPtrVector;

    CORE::CString fromSideId;                              /**< primary 'from' side for this route */
    CORE::CString toSideId;                                /**< primary 'to' side for this route */
    CORE::CString failoverSideId;                          /**< if the primary flow fails traffic would be rerouted here as a equivelant */ 
    CORE::CString spilloverBufferSideId;                   /**< if the primary and failover is unhealthy or is a slow consumer the spill over acts as buffer for the route publishing/subscribing to said side */
    CORE::CString deadLetterSideId;                        /**< unable-to-publish messages on configured channels with no remaining remedies go here */
    PubSubFlowRouteTopicConfigPtrVector topicAssociations; /**< Beyond the sides there is the mapping of which topics are associated with what other topic on the other sides if any, this holds that information */
    bool toSideTopicsAutoMatchFromSide;                    /**< Whether 'to' side topics should be auto matched naming wise 1:1 with the 'from' side automatically, barring an explicity association override */
    bool failoverSideTopicsAutoMatchFromSide;              /**< Whether 'failover' side topics should be auto matched naming wise 1:1 with the 'from' side automatically, barring an explicity association override */
    bool spilloverSideTopicsAutoMatchFromSide;             /**< Whether 'spillover' side topics should be auto matched naming wise 1:1 with the 'from' side automatically, barring an explicity association override */
    bool deadLetterSideTopicsAutoMatchFromSide;            /**< Whether 'deadletter' side topics should be auto matched naming wise 1:1 with the 'from' side automatically, barring an explicity association override */
    bool preferFromTopicThreadForDestination;              /**< if available the 'from' topic pulseGenerator will also be used for any created destination topics */
    bool egressAllDiscoveredSpilloverTopicsOnStart;        /**< if true all discovered spillover topics will be egressed otherwise only configured spillover topics will be egressed on startup */
    bool autoAssociateTopicsAnyToAnyAcrossSides;           /**< if true any topic on any side will be associated with any topic on any other side if no explicit association is found */

    CPubSubFlowRouteConfig( void );

    CPubSubFlowRouteConfig( const CPubSubFlowRouteConfig& src );

    virtual ~CPubSubFlowRouteConfig();

    CPubSubFlowRouteConfig& operator=( const CPubSubFlowRouteConfig& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool IsAnyAutoTopicMatchingNeeded( void ) const;

    CPubSubFlowRouteTopicConfigPtr FindTopicAssociation( const CORE::CString& fromTopicName ,
                                                         bool performPatternMatch           );

    CPubSubFlowRouteTopicConfigPtr FindOrCreateTopicAssociation( const CORE::CString& fromTopicName ,
                                                                 bool performPatternMatch           );

};

/*-------------------------------------------------------------------------*/

typedef CPubSubFlowRouteTopicConfig::CPubSubFlowRouteTopicConfigPtr     CPubSubFlowRouteTopicConfigPtr;
typedef CPubSubFlowRouteConfig::CPubSubFlowRouteConfigPtr               CPubSubFlowRouteConfigPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBFLOWROUTECONFIG_H ? */

