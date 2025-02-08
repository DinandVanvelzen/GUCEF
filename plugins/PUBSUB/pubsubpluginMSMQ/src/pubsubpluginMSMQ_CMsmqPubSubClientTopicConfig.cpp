/*
 *  pubsubpluginMSMQ: Generic GUCEF COMCORE plugin for providing pubsub via Microsoft MSMQ
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "pubsubpluginMSMQ_CMsmqPubSubClientTopicConfig.h"

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace MSMQ {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

// BODY is by far the largest in allowing up to 4MB per msg so we handle that as a special case
// This prevents what would likely be common resizes. Half the max allowed seems like a decent default
#define DEFAULT_MSMQ_BODY_BUFFER_SIZE_IN_BYTES       (1024 * 1024 * 2)

#define DEFAULT_MSMQ_MISC_BUFFER_SIZE_IN_BYTES       (128)

// If you have a very busy queue you do not want to spend all your time sending detailed
// per message MSMQ latency metrics. As such we provide a max nr of samples to take per 
// measuring cycle as dictated by the metrics interval.
#define DEFAULT_MAX_MSMQ_TRANSIT_TIME_ON_RECEIVE_DATA_POINTS            50

#define DEFAULT_MINIMAL_BUFFER_GROWTH_ON_PAYLOAD_BUFFER_TOO_SMALL       50

// MSMQ uses ULONG as the type for the PROPID_M_MSGID property
// Looking at the docs there are only a few properties relative to the range of ULONG
// that are reserved by MSMQ. We will use the range 10000 and up for custom properties as needed (defaults)
// This is a bit of a hack but it should be safe enough. At least the type will match the other ids and as such be a more natural fit downstream
#define DEFAULT_METADATA_KEY_OFFSET_FOR_CUSTOM_FAKE_MSMQ_PROPIDS        10000

// The default fake MSQM prop id used as a metadata key for the timestamp of when the message was read from MSMQ
#define DEFAULT_METADATA_KEY_FOR_MSG_READ_FROM_MSMQ_TIME                ( (CORE::UInt64) (DEFAULT_METADATA_KEY_OFFSET_FOR_CUSTOM_FAKE_MSMQ_PROPIDS + 1) )

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMsmqPubSubClientTopicConfig::CMsmqPubSubClientTopicConfig( void )
    : PUBSUB::CPubSubClientTopicConfig()
    , msmqPropIdsToReceive()
    , msmqPropIdsToSend()
    , msmqPairedPropIds()
    , maxMsmqMsgsToReadPerSyncCycle( 100 )
    , topicNameIsMsmqFormatName( false )
    , convertMsmqMsgIdToString( false )
    , convertMsmqClsIdToString( false )
    , ignoreUnmappableMetaDataFieldOnPublish( false )
    , gatherMsmqTransitTimeOnReceiveMetric( false )
    , maxMsmqTransitTimeOnReceiveMetricDataPoints( DEFAULT_MAX_MSMQ_TRANSIT_TIME_ON_RECEIVE_DATA_POINTS )
    , msmqMsgPropIdToMapToMsgIdOnReceive( PROPID_M_MSGID )
    , defaultMsmqBodyBufferSizeInBytes( DEFAULT_MSMQ_BODY_BUFFER_SIZE_IN_BYTES )
    , defaultMsmqMiscBufferSizeInBytes( DEFAULT_MSMQ_MISC_BUFFER_SIZE_IN_BYTES )
    , minPayloadFieldGrowthPercOnBufferTooSmall( DEFAULT_MINIMAL_BUFFER_GROWTH_ON_PAYLOAD_BUFFER_TOO_SMALL )
    , retainMsmqSentTimeAsMetaData( true )
    , addMsgReadFromMsmqTimeAsMetaData( true )
    , metadataKeyForMsgReadFromMsmqTime( DEFAULT_METADATA_KEY_FOR_MSG_READ_FROM_MSMQ_TIME )
    , maxMsmqErrorsOnAckToBeHealthy( 0 )
    , maxMsmqErrorsOnReceiveToBeHealthy( 0 )
{GUCEF_TRACE;
    
    PopulateDefaultReceivePropIds();
    PopulateDefaultSendPropIds();
    PopulateDefaultPairedPropIds();
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopicConfig::CMsmqPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig )
    : PUBSUB::CPubSubClientTopicConfig( genericConfig )
    , msmqPropIdsToReceive()
    , msmqPropIdsToSend()
    , msmqPairedPropIds()
    , maxMsmqMsgsToReadPerSyncCycle( 100 )
    , topicNameIsMsmqFormatName( false )
    , convertMsmqMsgIdToString( false )
    , convertMsmqClsIdToString( false )
    , ignoreUnmappableMetaDataFieldOnPublish( false )
    , gatherMsmqTransitTimeOnReceiveMetric( false )
    , maxMsmqTransitTimeOnReceiveMetricDataPoints( DEFAULT_MAX_MSMQ_TRANSIT_TIME_ON_RECEIVE_DATA_POINTS )
    , msmqMsgPropIdToMapToMsgIdOnReceive( PROPID_M_MSGID )
    , defaultMsmqBodyBufferSizeInBytes( DEFAULT_MSMQ_BODY_BUFFER_SIZE_IN_BYTES )
    , defaultMsmqMiscBufferSizeInBytes( DEFAULT_MSMQ_MISC_BUFFER_SIZE_IN_BYTES )
    , minPayloadFieldGrowthPercOnBufferTooSmall( DEFAULT_MINIMAL_BUFFER_GROWTH_ON_PAYLOAD_BUFFER_TOO_SMALL )
    , retainMsmqSentTimeAsMetaData( true )
    , addMsgReadFromMsmqTimeAsMetaData( true )
    , metadataKeyForMsgReadFromMsmqTime( DEFAULT_METADATA_KEY_FOR_MSG_READ_FROM_MSMQ_TIME )
    , maxMsmqErrorsOnAckToBeHealthy( 0 )
    , maxMsmqErrorsOnReceiveToBeHealthy( 0 )
{GUCEF_TRACE;
    
    PopulateDefaultReceivePropIds();
    PopulateDefaultSendPropIds();
    PopulateDefaultPairedPropIds();
    LoadCustomConfig( genericConfig.customConfig );  
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopicConfig::~CMsmqPubSubClientTopicConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopicConfig::LoadCustomConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;
    
    bool totalSuccess = true;

    maxMsmqMsgsToReadPerSyncCycle = config.GetAttributeValueOrChildValueByName( "maxMsmqMsgsToReadPerSyncCycle" ).AsUInt32( maxMsmqMsgsToReadPerSyncCycle, true );
    if ( 0 == maxMsmqMsgsToReadPerSyncCycle )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqPubSubClientTopicConfig:LoadCustomConfig: maxMsmqMsgsToReadPerSyncCycle=0, this is invalid. Setting to minimum of 1" );
        maxMsmqMsgsToReadPerSyncCycle = 1;
    }

    defaultMsmqBodyBufferSizeInBytes = config.GetAttributeValueOrChildValueByName( "defaultMsmqBodyBufferSizeInBytes" ).AsUInt32( defaultMsmqBodyBufferSizeInBytes, true );
    defaultMsmqMiscBufferSizeInBytes = config.GetAttributeValueOrChildValueByName( "defaultMsmqMiscBufferSizeInBytes" ).AsUInt32( defaultMsmqMiscBufferSizeInBytes, true );
    topicNameIsMsmqFormatName = config.GetAttributeValueOrChildValueByName( "topicNameIsMsmqFormatName" ).AsBool( topicNameIsMsmqFormatName, true );
    convertMsmqMsgIdToString = config.GetAttributeValueOrChildValueByName( "convertMsmqMsgIdToString" ).AsBool( convertMsmqMsgIdToString, true );
    convertMsmqClsIdToString = config.GetAttributeValueOrChildValueByName( "convertMsmqClsIdToString" ).AsBool( convertMsmqClsIdToString, true );
    ignoreUnmappableMetaDataFieldOnPublish = config.GetAttributeValueOrChildValueByName( "ignoreUnmappableMetaDataFieldOnPublish" ).AsBool( ignoreUnmappableMetaDataFieldOnPublish, true );
    gatherMsmqTransitTimeOnReceiveMetric = config.GetAttributeValueOrChildValueByName( "gatherMsmqTransitTimeOnReceiveMetric" ).AsBool( gatherMsmqTransitTimeOnReceiveMetric, true ); 
    maxMsmqTransitTimeOnReceiveMetricDataPoints = config.GetAttributeValueOrChildValueByName( "maxMsmqTransitTimeOnReceiveMetricDataPoints" ).AsUInt32( maxMsmqTransitTimeOnReceiveMetricDataPoints, true ); 
    msmqMsgPropIdToMapToMsgIdOnReceive = config.GetAttributeValueOrChildValueByName( "msmqMsgPropIdToMapToMsgIdOnReceive" ).AsUInt64( msmqMsgPropIdToMapToMsgIdOnReceive, true );
    minPayloadFieldGrowthPercOnBufferTooSmall = config.GetAttributeValueOrChildValueByName( "minPayloadFieldGrowthPercOnBufferTooSmall" ).AsUInt16( minPayloadFieldGrowthPercOnBufferTooSmall, true );
    retainMsmqSentTimeAsMetaData = config.GetAttributeValueOrChildValueByName( "retainMsmqSentTimeAsMetaData" ).AsBool( retainMsmqSentTimeAsMetaData, true );
    addMsgReadFromMsmqTimeAsMetaData = config.GetAttributeValueOrChildValueByName( "addMsgReadFromMsmqTimeAsMetaData" ).AsBool( addMsgReadFromMsmqTimeAsMetaData, true );
    metadataKeyForMsgReadFromMsmqTime = config.GetAttributeValueOrChildValueByName( "metadataKeyForMsgReadFromMsmqTime", metadataKeyForMsgReadFromMsmqTime );
    maxMsmqErrorsOnAckToBeHealthy = config.GetAttributeValueOrChildValueByName( "maxMsmqErrorsOnAckToBeHealthy" ).AsInt32( maxMsmqErrorsOnAckToBeHealthy, true );
    maxMsmqErrorsOnReceiveToBeHealthy = config.GetAttributeValueOrChildValueByName( "maxMsmqErrorsOnReceiveToBeHealthy" ).AsInt32( maxMsmqErrorsOnReceiveToBeHealthy, true );
    
    CORE::CString csvPropIds = config.GetAttributeValueOrChildValueByName( "msmqPropIdsToReceive" ).AsString( PropIdsToCsvString( msmqPropIdsToReceive ), true );
    msmqPropIdsToReceive.clear();
    if ( !CsvStringToPropIds( csvPropIds, msmqPropIdsToReceive ) )
    {
        totalSuccess = false;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopicConfig: Unable to obtain MSMQ Property IDs from msmqPropIdsToReceive: " + csvPropIds );
    }

    csvPropIds = config.GetAttributeValueOrChildValueByName( "msmqPropIdsToSend" ).AsString( PropIdsToCsvString( msmqPropIdsToSend ), true );
    msmqPropIdsToSend.clear();
    if ( !CsvStringToPropIds( csvPropIds, msmqPropIdsToSend ) )
    {
        totalSuccess = false;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopicConfig: Unable to obtain MSMQ Property IDs from msmqPropIdsToSend: " + csvPropIds );
    }

    SupplementPropIdsWithPairedPropId( msmqPropIdsToReceive );
    SupplementPropIdsWithPairedPropId( msmqPropIdsToSend );

    if ( gatherMsmqTransitTimeOnReceiveMetric )
    {
        AddPropId( PROPID_M_SENTTIME, msmqPropIdsToReceive );
        AddPropId( PROPID_M_ARRIVEDTIME, msmqPropIdsToReceive );
    }

    // For MSMQ 3.0 and above:
    #if(_WIN32_WINNT >= 0x0501)
    AddPropId( PROPID_M_LOOKUPID, msmqPropIdsToReceive );
    #endif

    return true;
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopicConfig& 
CMsmqPubSubClientTopicConfig::operator=( const PUBSUB::CPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );

        msmqPropIdsToReceive.clear();
        msmqPropIdsToSend.clear();
        PopulateDefaultReceivePropIds();
        PopulateDefaultSendPropIds();

        LoadCustomConfig( src.customConfig );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopicConfig& 
CMsmqPubSubClientTopicConfig::operator=( const CMsmqPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
        msmqPairedPropIds = src.msmqPairedPropIds;
        msmqPropIdsToReceive = src.msmqPropIdsToReceive;
        msmqPropIdsToSend = src.msmqPropIdsToSend;
        maxMsmqMsgsToReadPerSyncCycle = src.maxMsmqMsgsToReadPerSyncCycle;
        topicNameIsMsmqFormatName = src.topicNameIsMsmqFormatName;
        convertMsmqMsgIdToString = src.convertMsmqMsgIdToString;
        convertMsmqClsIdToString = src.convertMsmqClsIdToString;
        ignoreUnmappableMetaDataFieldOnPublish = src.ignoreUnmappableMetaDataFieldOnPublish;
        gatherMsmqTransitTimeOnReceiveMetric = src.gatherMsmqTransitTimeOnReceiveMetric;
        maxMsmqTransitTimeOnReceiveMetricDataPoints = src.maxMsmqTransitTimeOnReceiveMetricDataPoints;
        msmqMsgPropIdToMapToMsgIdOnReceive = src.msmqMsgPropIdToMapToMsgIdOnReceive;
        defaultMsmqBodyBufferSizeInBytes = src.defaultMsmqBodyBufferSizeInBytes;
        defaultMsmqMiscBufferSizeInBytes = src.defaultMsmqMiscBufferSizeInBytes;
        minPayloadFieldGrowthPercOnBufferTooSmall = src.minPayloadFieldGrowthPercOnBufferTooSmall;
        retainMsmqSentTimeAsMetaData = src.retainMsmqSentTimeAsMetaData;
        addMsgReadFromMsmqTimeAsMetaData = src.addMsgReadFromMsmqTimeAsMetaData;
        metadataKeyForMsgReadFromMsmqTime = src.metadataKeyForMsgReadFromMsmqTime;
        maxMsmqErrorsOnAckToBeHealthy = src.maxMsmqErrorsOnAckToBeHealthy;
        maxMsmqErrorsOnReceiveToBeHealthy = src.maxMsmqErrorsOnReceiveToBeHealthy;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMsmqPubSubClientTopicConfig::PropIdsToCsvString( const MSGPROPIDVector& propIds )
{GUCEF_TRACE;

    CORE::CString csvStr;
    MSGPROPIDVector::const_iterator i = propIds.begin();
    while ( i != propIds.end() )
    {
        if ( !csvStr.IsNULLOrEmpty() )
        {
            csvStr += ',' + CORE::ToString( CORE::UInt64( (*i) ) );
        }
        else
        {
            csvStr = CORE::ToString( CORE::UInt64( (*i) ) );
        }
        ++i;
    }
    return csvStr;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopicConfig::CsvStringToPropIds( const CORE::CString& csvStr, MSGPROPIDVector& propIds )
{GUCEF_TRACE;

    CORE::CString::StringVector elements = csvStr.ParseElements( ',', false );
    
    bool totalSuccess = true;
    propIds.reserve( elements.size() );
    CORE::CString::StringVector::iterator i = elements.begin();
    while ( i != elements.end() )
    {
        CORE::UInt64 propId = CORE::StringToUInt64( (*i), GUCEF_MT_UINT64MAX );
        if ( propId != GUCEF_MT_UINT64MAX )
            propIds.push_back( (MSGPROPID) propId );
        else
            totalSuccess = false;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopicConfig::PopulateDefaultReceivePropIds( void )
{GUCEF_TRACE;

    // Maps to our Primary payload variant
    msmqPropIdsToReceive.push_back( PROPID_M_BODY );
    
    // Maps to our PubSubMsg MsgId, 20 unicode chars 
    msmqPropIdsToReceive.push_back( PROPID_M_MSGID );

    // Maps to our PubSubMsg MsgIndex
    #if(_WIN32_WINNT >= 0x0501)
    msmqPropIdsToReceive.push_back( PROPID_M_LOOKUPID );
    #endif

    // Maps to our PubSubMsg DateTime, uses Unix Epoch based UInt32 in seconds
    msmqPropIdsToReceive.push_back( PROPID_M_SENTTIME );

    // Maps to the topic name which is a MSMQ queue address
    msmqPropIdsToReceive.push_back( PROPID_M_DEST_QUEUE );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopicConfig::PopulateDefaultSendPropIds( void )
{GUCEF_TRACE;

    // Maps to our Primary payload variant
    msmqPropIdsToSend.push_back( PROPID_M_BODY ); 

    // Maps to our PubSubMsg MsgId, 20 unicode chars 
    // Note only available after send completes and is assigned by MSMQ itself
    msmqPropIdsToSend.push_back( PROPID_M_MSGID );

    // Maps to our PubSubMsg DateTime, uses Unix Epoch based UInt32 in seconds
    msmqPropIdsToSend.push_back( PROPID_M_SENTTIME );

    // Maps to the topic name which is a MSMQ queue address
    msmqPropIdsToSend.push_back( PROPID_M_DEST_QUEUE );

    return true;

}
/*--------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopicConfig::PopulateDefaultPairedPropIds( void )
{GUCEF_TRACE;

    msmqPairedPropIds[ PROPID_M_ADMIN_QUEUE ].push_back( PROPID_M_ADMIN_QUEUE_LEN );
    msmqPairedPropIds[ PROPID_M_BODY ].push_back( PROPID_M_BODY_SIZE );
    msmqPairedPropIds[ PROPID_M_BODY ].push_back( PROPID_M_BODY_TYPE );
    msmqPairedPropIds[ PROPID_M_DEST_QUEUE ].push_back( PROPID_M_DEST_QUEUE_LEN );
    msmqPairedPropIds[ PROPID_M_DEST_SYMM_KEY ].push_back( PROPID_M_DEST_SYMM_KEY_LEN );    
    msmqPairedPropIds[ PROPID_M_DEADLETTER_QUEUE ].push_back( PROPID_M_DEADLETTER_QUEUE_LEN );
    msmqPairedPropIds[ PROPID_M_EXTENSION ].push_back( PROPID_M_EXTENSION_LEN );
    msmqPairedPropIds[ PROPID_M_LABEL ].push_back( PROPID_M_LABEL_LEN );
    msmqPairedPropIds[ PROPID_M_PROV_NAME ].push_back( PROPID_M_PROV_NAME_LEN );
    msmqPairedPropIds[ PROPID_M_RESP_FORMAT_NAME ].push_back( PROPID_M_RESP_FORMAT_NAME_LEN );
    msmqPairedPropIds[ PROPID_M_RESP_QUEUE ].push_back( PROPID_M_RESP_QUEUE_LEN );
    msmqPairedPropIds[ PROPID_M_SENDER_CERT ].push_back( PROPID_M_SENDER_CERT_LEN );
    msmqPairedPropIds[ PROPID_M_SENDERID ].push_back( PROPID_M_SENDERID_LEN );
    msmqPairedPropIds[ PROPID_M_SENDERID ].push_back( PROPID_M_SENDERID_TYPE );
    msmqPairedPropIds[ PROPID_M_SIGNATURE ].push_back( PROPID_M_SIGNATURE_LEN );
    msmqPairedPropIds[ PROPID_M_XACT_STATUS_QUEUE ].push_back( PROPID_M_XACT_STATUS_QUEUE_LEN );

    // Introduced in MSMQ 3.0
    #if ( _WIN32_WINNT >= 0x0501 )
    msmqPairedPropIds[ PROPID_M_COMPOUND_MESSAGE ].push_back( PROPID_M_COMPOUND_MESSAGE_SIZE );
    msmqPairedPropIds[ PROPID_M_DEST_FORMAT_NAME ].push_back( PROPID_M_DEST_FORMAT_NAME_LEN );
    msmqPairedPropIds[ PROPID_M_SOAP_ENVELOPE ].push_back( PROPID_M_SOAP_ENVELOPE_LEN );    
    #endif
}

/*--------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopicConfig::ErasePropId( PROPID propId, MSGPROPIDVector& propIds )
{GUCEF_TRACE;

    MSGPROPIDVector::iterator i = propIds.begin();
    while ( i != propIds.end() )
    {
        if ( (*i) == propId )
        {
            propIds.erase( i );
            i = propIds.begin();
        }
        ++i;
    }
}

/*--------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopicConfig::HasPropId( PROPID id, const MSGPROPIDVector& propIds )
{GUCEF_TRACE;

    MSGPROPIDVector::const_iterator i = propIds.begin();
    while ( i != propIds.end() )
    {
        if ( id == (*i) )
            return true;
        ++i;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopicConfig::AddPropId( PROPID id, MSGPROPIDVector& propIds )
{GUCEF_TRACE;

    if ( !HasPropId( id, propIds ) )
        propIds.push_back( id );
}

/*--------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopicConfig::IsPairedPropId( PROPID id ) const
{GUCEF_TRACE;

    MSGPROPIDMapVector::const_iterator n = msmqPairedPropIds.begin();
    while ( n != msmqPairedPropIds.end() )
    {
        const MSGPROPIDVector& pairedPropIds = (*n).second;
        MSGPROPIDVector::const_iterator i = pairedPropIds.begin();
        while ( i != pairedPropIds.end() )
        {
            if ( id == (*i) )
                return true;
            ++i;
        }
        ++n;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopicConfig::SupplementPropIdsWithPairedPropId( MSGPROPIDVector& propIds )
{GUCEF_TRACE;
    
    // To make the config less error prone we will auto-add properties
    // which are MSMQ mandated paired properties
    // We only need to do this at config load so we use the vector to store the values for runtime performance
    // even if it makes this a bit less efficient

    MSGPROPIDVector newPropIds;

    MSGPROPIDVector::iterator i = propIds.begin();
    while ( i != propIds.end() )
    {
        // is the property one that needs associated paired properties which are meta-data about the main property?
        MSGPROPIDMapVector::iterator n = msmqPairedPropIds.find( (*i) );
        if ( n != msmqPairedPropIds.end() )
        {                                  
            // For speed of processing later we should add the paired properties in order
            // first in the flattended list. For example read the length of the string before the
            // string itself to allow proper pre-allocation when needed
            MSGPROPIDVector& pairedProps = (*n).second;
            MSGPROPIDVector::iterator m = pairedProps.begin();
            while ( m != pairedProps.end() ) 
            {
                ErasePropId( (*m), newPropIds );
                newPropIds.push_back( (*m) );
                ++m;
            }
        }
        if ( !IsPairedPropId( (*i) ) )
        {
            newPropIds.push_back( (*i) );
        }
        ++i;
    }

    propIds = newPropIds;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CMsmqPubSubClientTopicConfig::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMsmqPubSubClientTopicConfig( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MSMQ */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
