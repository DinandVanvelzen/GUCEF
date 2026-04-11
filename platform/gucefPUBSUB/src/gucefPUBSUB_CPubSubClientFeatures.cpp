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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefPUBSUB_CPubSubClientFeatures.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPubSubClientFeatures::CPubSubClientFeatures( void )
    : supportsPublishing( false )
    , supportsSubscribing( false )    
    , supportsSimultaneousPubAndSub( false )
    , supportsAutoReconnect( false )
    , supportsSubscriberRateThrottling( false )
    , supportsSubscriptionMsgArrivalDelayRequests( false )
    , supportsSubscriptionEndOfDataEvent( false )
    , supportsBinaryPayloads( false )
    , supportsMultiHostSharding( false )
    , supportsPerMsgIds( false )
    , supportsMsgIndex( false )
    , supportsPrimaryPayloadPerMsg( false )
    , supportsAbsentPrimaryPayloadPerMsg( false )
    , supportsKeyValueSetPerMsg( false )
    , supportsDuplicateKeysPerMsg( false )
    , supportsMetaDataKeyValueSetPerMsg( false )
    , supportsSubscriberMsgReceivedAck( false )
    , supportsAutoMsgReceivedAck( false )
    , supportsAbsentMsgReceivedAck( false )
    , supportsAckUsingLastMsgInBatch( false )
    , supportsAckUsingBookmark( false )
    , supportsBookmarkingConcept( false )
    , supportsSubscribingUsingBookmark( false )
    , supportsServerSideBookmarkPersistance( false )
    , supportsAutoBookmarking( false )
    , supportsMsgIdBasedBookmark( false )
    , supportsMsgIndexBasedBookmark( false )
    , supportsTopicIndexBasedBookmark( false )
    , supportsMsgDateTimeBasedBookmark( false )
    , supportsDerivingBookmarkFromMsg( false )
    , supportsDiscoveryOfAvailableTopics( false )
    , supportsGlobPatternTopicNames( false )
    , supportsPatternBasedAggregateTopic( false )
    , supportsMetrics( false )
    , supportsReplay( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientFeatures::~CPubSubClientFeatures()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool 
CPubSubClientFeatures::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool success = true;
    success = cfg.SetAttribute( "supportsPublishing", supportsPublishing ) && success;
    success = cfg.SetAttribute( "supportsSubscribing", supportsSubscribing ) && success;
    success = cfg.SetAttribute( "supportsSimultaneousPubAndSub", supportsSimultaneousPubAndSub ) && success;    
    success = cfg.SetAttribute( "supportsAutoReconnect", supportsAutoReconnect ) && success;
    success = cfg.SetAttribute( "supportsSubscriberRateThrottling", supportsSubscriberRateThrottling ) && success;    
    success = cfg.SetAttribute( "supportsSubscriptionMsgArrivalDelayRequests", supportsSubscriptionMsgArrivalDelayRequests ) && success;        
    success = cfg.SetAttribute( "supportsSubscriptionEndOfDataEvent", supportsSubscriptionEndOfDataEvent ) && success;            
    success = cfg.SetAttribute( "supportsBinaryPayloads", supportsBinaryPayloads ) && success;
    success = cfg.SetAttribute( "supportsMultiHostSharding", supportsMultiHostSharding ) && success;
    success = cfg.SetAttribute( "supportsPerMsgIds", supportsPerMsgIds ) && success;
    success = cfg.SetAttribute( "supportsMsgIndex", supportsMsgIndex ) && success;
    success = cfg.SetAttribute( "supportsPrimaryPayloadPerMsg", supportsPrimaryPayloadPerMsg ) && success;
    success = cfg.SetAttribute( "supportsAbsentPrimaryPayloadPerMsg", supportsAbsentPrimaryPayloadPerMsg ) && success;
    success = cfg.SetAttribute( "supportsKeyValueSetPerMsg", supportsKeyValueSetPerMsg ) && success;
    success = cfg.SetAttribute( "supportsDuplicateKeysPerMsg", supportsDuplicateKeysPerMsg ) && success;
    success = cfg.SetAttribute( "supportsMetaDataKeyValueSetPerMsg", supportsMetaDataKeyValueSetPerMsg ) && success;
    success = cfg.SetAttribute( "supportsSubscriberMsgReceivedAck", supportsSubscriberMsgReceivedAck ) && success;
    success = cfg.SetAttribute( "supportsAutoMsgReceivedAck", supportsAutoMsgReceivedAck ) && success;
    success = cfg.SetAttribute( "supportsAbsentMsgReceivedAck", supportsAbsentMsgReceivedAck ) && success;
    success = cfg.SetAttribute( "supportsAckUsingLastMsgInBatch", supportsAckUsingLastMsgInBatch ) && success;
    success = cfg.SetAttribute( "supportsAckUsingBookmark", supportsAckUsingBookmark ) && success;
    success = cfg.SetAttribute( "supportsBookmarkingConcept", supportsBookmarkingConcept ) && success;
    success = cfg.SetAttribute( "supportsSubscribingUsingBookmark", supportsSubscribingUsingBookmark ) && success;
    success = cfg.SetAttribute( "supportsServerSideBookmarkPersistance", supportsServerSideBookmarkPersistance ) && success;
    success = cfg.SetAttribute( "supportsAutoBookmarking", supportsAutoBookmarking ) && success;
    success = cfg.SetAttribute( "supportsMsgIdBasedBookmark", supportsMsgIdBasedBookmark ) && success;
    success = cfg.SetAttribute( "supportsMsgIndexBasedBookmark", supportsMsgIndexBasedBookmark ) && success;
    success = cfg.SetAttribute( "supportsTopicIndexBasedBookmark", supportsTopicIndexBasedBookmark ) && success;    
    success = cfg.SetAttribute( "supportsMsgDateTimeBasedBookmark", supportsMsgDateTimeBasedBookmark ) && success;
    success = cfg.SetAttribute( "supportsDerivingBookmarkFromMsg", supportsDerivingBookmarkFromMsg ) && success;    
    success = cfg.SetAttribute( "supportsDiscoveryOfAvailableTopics", supportsDiscoveryOfAvailableTopics ) && success;    
    success = cfg.SetAttribute( "supportsGlobPatternTopicNames", supportsGlobPatternTopicNames ) && success;    
    success = cfg.SetAttribute( "supportsPatternBasedAggregateTopic", supportsPatternBasedAggregateTopic ) && success;
    success = cfg.SetAttribute( "supportsMetrics", supportsMetrics ) && success;
    success = cfg.SetAttribute( "supportsReplay", supportsReplay ) && success;

    return success;
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubClientFeatures::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    supportsPublishing = cfg.GetAttributeValueOrChildValueByName( "supportsPublishing" ).AsBool( supportsPublishing, true );
    supportsSubscribing = cfg.GetAttributeValueOrChildValueByName( "supportsSubscribing" ).AsBool( supportsSubscribing, true );
    supportsSimultaneousPubAndSub = cfg.GetAttributeValueOrChildValueByName( "supportsSimultaneousPubAndSub" ).AsBool( supportsSimultaneousPubAndSub, true );
    supportsAutoReconnect = cfg.GetAttributeValueOrChildValueByName( "supportsAutoReconnect" ).AsBool( supportsAutoReconnect, true );
    supportsSubscriberRateThrottling = cfg.GetAttributeValueOrChildValueByName( "supportsSubscriberRateThrottling" ).AsBool( supportsSubscriberRateThrottling, true );
    supportsSubscriptionMsgArrivalDelayRequests = cfg.GetAttributeValueOrChildValueByName( "supportsSubscriptionEndOfDataEvent" ).AsBool( supportsSubscriptionEndOfDataEvent, true );
    supportsSubscriptionEndOfDataEvent = cfg.GetAttributeValueOrChildValueByName( "supportsSubscriptionMsgArrivalDelayRequests" ).AsBool( supportsSubscriptionMsgArrivalDelayRequests, true );
    supportsBinaryPayloads = cfg.GetAttributeValueOrChildValueByName( "supportsBinaryPayloads" ).AsBool( supportsBinaryPayloads, true );
    supportsMultiHostSharding = cfg.GetAttributeValueOrChildValueByName( "supportsMultiHostSharding" ).AsBool( supportsMultiHostSharding, true );
    supportsPerMsgIds = cfg.GetAttributeValueOrChildValueByName( "supportsPerMsgIds" ).AsBool( supportsPerMsgIds, true );
    supportsMsgIndex = cfg.GetAttributeValueOrChildValueByName( "supportsMsgIndex" ).AsBool( supportsMsgIndex, true );
    supportsPrimaryPayloadPerMsg = cfg.GetAttributeValueOrChildValueByName( "supportsPrimaryPayloadPerMsg" ).AsBool( supportsPrimaryPayloadPerMsg, true );    
    supportsAbsentPrimaryPayloadPerMsg = cfg.GetAttributeValueOrChildValueByName( "supportsAbsentPrimaryPayloadPerMsg" ).AsBool( supportsAbsentPrimaryPayloadPerMsg, true );    
    supportsKeyValueSetPerMsg = cfg.GetAttributeValueOrChildValueByName( "supportsKeyValueSetPerMsg" ).AsBool( supportsKeyValueSetPerMsg, true );
    supportsDuplicateKeysPerMsg = cfg.GetAttributeValueOrChildValueByName( "supportsDuplicateKeysPerMsg" ).AsBool( supportsDuplicateKeysPerMsg, true );
    supportsMetaDataKeyValueSetPerMsg = cfg.GetAttributeValueOrChildValueByName( "supportsMetaDataKeyValueSetPerMsg" ).AsBool( supportsMetaDataKeyValueSetPerMsg, true );
    supportsSubscriberMsgReceivedAck = cfg.GetAttributeValueOrChildValueByName( "supportsSubscriberMsgReceivedAck" ).AsBool( supportsSubscriberMsgReceivedAck, true );
    supportsAutoMsgReceivedAck = cfg.GetAttributeValueOrChildValueByName( "supportsAutoMsgReceivedAck" ).AsBool( supportsAutoMsgReceivedAck, true );
    supportsAbsentMsgReceivedAck = cfg.GetAttributeValueOrChildValueByName( "supportsAbsentMsgReceivedAck" ).AsBool( supportsAbsentMsgReceivedAck, true );
    supportsAckUsingLastMsgInBatch = cfg.GetAttributeValueOrChildValueByName( "supportsAckUsingLastMsgInBatch" ).AsBool( supportsAckUsingLastMsgInBatch, true );
    supportsAckUsingBookmark = cfg.GetAttributeValueOrChildValueByName( "supportsAckUsingBookmark" ).AsBool( supportsAckUsingBookmark, true );
    supportsBookmarkingConcept = cfg.GetAttributeValueOrChildValueByName( "supportsBookmarkingConcept" ).AsBool( supportsBookmarkingConcept, true );
    supportsSubscribingUsingBookmark = cfg.GetAttributeValueOrChildValueByName( "supportsSubscribingUsingBookmark" ).AsBool( supportsSubscribingUsingBookmark, true );
    supportsServerSideBookmarkPersistance = cfg.GetAttributeValueOrChildValueByName( "supportsServerSideBookmarkPersistance" ).AsBool( supportsServerSideBookmarkPersistance, true );
    supportsAutoBookmarking = cfg.GetAttributeValueOrChildValueByName( "supportsAutoBookmarking" ).AsBool( supportsAutoBookmarking, true );
    supportsMsgIdBasedBookmark = cfg.GetAttributeValueOrChildValueByName( "supportsMsgIdBasedBookmark" ).AsBool( supportsMsgIdBasedBookmark, true );
    supportsMsgIndexBasedBookmark = cfg.GetAttributeValueOrChildValueByName( "supportsMsgIndexBasedBookmark" ).AsBool( supportsMsgIndexBasedBookmark, true );
    supportsTopicIndexBasedBookmark = cfg.GetAttributeValueOrChildValueByName( "supportsTopicIndexBasedBookmark" ).AsBool( supportsTopicIndexBasedBookmark, true );
    supportsMsgDateTimeBasedBookmark = cfg.GetAttributeValueOrChildValueByName( "supportsMsgDateTimeBasedBookmark" ).AsBool( supportsMsgDateTimeBasedBookmark, true );
    supportsDerivingBookmarkFromMsg = cfg.GetAttributeValueOrChildValueByName( "supportsDerivingBookmarkFromMsg" ).AsBool( supportsDerivingBookmarkFromMsg, true );
    supportsDiscoveryOfAvailableTopics = cfg.GetAttributeValueOrChildValueByName( "supportsDiscoveryOfAvailableTopics" ).AsBool( supportsDiscoveryOfAvailableTopics, true );    
    supportsGlobPatternTopicNames = cfg.GetAttributeValueOrChildValueByName( "supportsGlobPatternTopicNames" ).AsBool( supportsGlobPatternTopicNames, true );    
    supportsPatternBasedAggregateTopic = cfg.GetAttributeValueOrChildValueByName( "supportsPatternBasedAggregateTopic" ).AsBool( supportsPatternBasedAggregateTopic, true );
    supportsMetrics = cfg.GetAttributeValueOrChildValueByName( "supportsMetrics" ).AsBool( supportsMetrics, true );
    supportsReplay = cfg.GetAttributeValueOrChildValueByName( "supportsReplay" ).AsBool( supportsReplay, true );

    return true;
}

/*-------------------------------------------------------------------------*/

const CString& 
CPubSubClientFeatures::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::PUBSUB::CPubSubClientFeatures";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
