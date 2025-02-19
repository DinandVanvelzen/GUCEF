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

#include <Objbase.h>
#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H */

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTCONFIG_H
#include "pubsubpluginMSMQ_CMsmqPubSubClientConfig.h"
#define PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTCONFIG_H
#endif /* PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTCONFIG_H ? */

#include "pubsubpluginMSMQ_CMsmqPubSubClient.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMsmqPubSubClient::TypeName = "MSMQ";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMsmqPubSubClient::CMsmqPubSubClient( const PUBSUB::CPubSubClientConfig& config )
    : PUBSUB::CPubSubClient( config.pulseGenerator )
    , m_config()
    , m_metricsTimer( GUCEF_NULL )
    , m_topicMap()
    , m_isHealthy( true )
    , m_lock()
{GUCEF_TRACE;

    if ( !LoadConfig( config ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqPubSubClient: Failed to load config at construction" );
    }

    if ( config.desiredFeatures.supportsMetrics )
    {
        m_metricsTimer = GUCEF_NEW CORE::CTimer( config.pulseGenerator, 1000 );
        m_metricsTimer->SetEnabled( config.desiredFeatures.supportsMetrics );
    }

    m_config.metricsPrefix += "msmq.";

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClient::~CMsmqPubSubClient()
{GUCEF_TRACE;
    
    Disconnect();

    MT::CScopeMutex lock( m_lock );

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->Shutdown();
        (*i).second.Unlink();
        ++i;
    }
    m_topicMap.clear();
    
    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    return SetPulseGenerator( newPulseGenerator, true );
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
                                      bool includeTopics                        )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    CORE::CTSGNotifier::SetPulseGenerator( newPulseGenerator );
    m_config.pulseGenerator = newPulseGenerator;
    
    if ( GUCEF_NULL != m_metricsTimer )
    {
        m_metricsTimer->SetPulseGenerator( newPulseGenerator );
    }

    if ( includeTopics )
    {
        m_config.topicPulseGenerator = m_config.pulseGenerator;

        TTopicMap::iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            (*i).second->SetPulseGenerator( newPulseGenerator );
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientConfig& 
CMsmqPubSubClient::GetConfig( void )
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    features.supportsBinaryPayloads = true;             // The MSMQ body property supports a binary payload
    features.supportsPerMsgIds = true;                  // MSMQ has the concept of a message ID which is unique and an additional non-unique label
    features.supportsPrimaryPayloadPerMsg = true;       // For MSMQ "BODY" is the primary payload which is also in of itself a key-value message propery
    features.supportsAbsentPrimaryPayloadPerMsg = true; // Its allowed to send tags without a BODY payload
    features.supportsKeyValueSetPerMsg = false;         // Arbitrary key-value app data is not natively supported by MSMQ
    features.supportsDuplicateKeysPerMsg = false;       // TODO: Since arbitrary key-value app data is not native and we simulate this we will do so in a manner that supports duplicate keys
    features.supportsMetaDataKeyValueSetPerMsg = true;  // This is native to MSMQ
    features.supportsMultiHostSharding = false;         // MSMQ is tied to the Windows O/S and queues are not auto shared across such O/S instances
    features.supportsPublishing = true;                 // We support being a MSQM queue publisher in this plugin
    features.supportsSubscribing = true;                // We support being a MSMQ queue subscriber in this plugin
    features.supportsMetrics = true;                    // This plugin has support for reporting its own set of metrics
    features.supportsAutoReconnect = true;              // Not applicable to local queues and for remote queues MSMQ supports the concept of "offline mode"        
    features.supportsAbsentMsgReceivedAck = false;      // Since MSMQ is a queue, by default you consume the message when you read it
    features.supportsAckUsingLastMsgInBatch = false;    // Even when using LookupID we have to operate per message. We dont track the batch ourselves    
    features.supportsBookmarkingConcept = true;         // Always getting the top msg in the queue could be thought of as "remembering your last read position" so as such we will claim MSMQ supports this    
    features.supportsAutoBookmarking = true;            // Always getting the top msg in the queue could be thought of as "remembering your last read position" so as such we will claim MSMQ supports this
    features.supportsMsgIdBasedBookmark = false;        // MSMQ does not support this concept. receiving messages removes them from the O/S queue    
    features.supportsMsgIndexBasedBookmark = false;     // MSMQ does not support this concept. receiving messages removes them from the O/S queue
    features.supportsMsgDateTimeBasedBookmark = false;  // MSMQ does not support this concept. receiving messages removes them from the O/S queue
    features.supportsDiscoveryOfAvailableTopics = true; // We support local private queue plus public queue discovery, provided account and AD permissions allow for access
    features.supportsGlobPatternTopicNames = false;
    features.supportsSubscriptionMsgArrivalDelayRequests = false;
    features.supportsSubscriptionEndOfDataEvent = true; // We support notifying that we did not find any additional messages to read from the queue
    
    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    
    bool supportLookup = m_config.simulateReceiveAckFeatureViaLookupId && m_config.desiredFeatures.supportsSubscriberMsgReceivedAck; 
    
    features.supportsAutoMsgReceivedAck = !supportLookup;            // When simulating receive acks we never auto ack
    features.supportsSubscriberMsgReceivedAck = supportLookup;       // The whole point is simulating the ability to ack that a message was handled
    features.supportsAckUsingBookmark = supportLookup;               // Bookmark or message, either way we use the LookupID which we count as a topic index
    features.supportsServerSideBookmarkPersistance = !supportLookup; // If we are using lookups the LookUp will need to be persisted externally from the app between runs
    
    features.supportsSubscribingUsingBookmark = supportLookup;    // we use the LookupID which we count as a topic index
    features.supportsTopicIndexBasedBookmark = supportLookup;     // we use the LookupID which we count as a topic index
    features.supportsDerivingBookmarkFromMsg = supportLookup;     // the message index property is the LookupID
    
    #else

    features.supportsServerSideBookmarkPersistance = true; // since MSMQ is a queue it remembers simply through consumption
    features.supportsAutoMsgReceivedAck = true;            // Since MSMQ is a queue, by default you consume the message when you read it we can consider this an ack
    features.supportsSubscriberMsgReceivedAck = false;     // MSMQ does not support this concept. receiving messages removes them from the O/S queue
    features.supportsAckUsingBookmark = false;             // MSMQ does not support this concept. receiving messages removes them from the O/S queue
    features.supportsSubscribingUsingBookmark = false;     // MSMQ does not support this concept. receiving messages removes them from the O/S queue
    features.supportsTopicIndexBasedBookmark = false;      // MSMQ does not support this concept. receiving messages removes them from the O/S queue
    
    #endif

    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr
CMsmqPubSubClient::CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                      CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    CMsmqPubSubClientTopicPtr topicAccess;
    {
        MT::CScopeMutex lock( m_lock );

        topicAccess = CMsmqPubSubClientTopic::CreateSharedObjWithParam( this );
        if ( topicAccess->LoadConfig( *topicConfig ) )
        {
            m_topicMap[ topicConfig->topicName ] = topicAccess;            
            RegisterTopicEventHandlers( topicAccess );
        }
        else
        {
            topicAccess->Shutdown();
            topicAccess.Unlink();
        }
    }

    if ( !topicAccess.IsNULL() )
    {
        TopicAccessCreatedEventData eData( topicConfig->topicName );
        NotifyObservers( TopicAccessCreatedEvent, &eData );
    }

    return topicAccess;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr 
CMsmqPubSubClient::GetTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
    {
        return (*i).second;
    }
    return PUBSUB::CPubSubClientTopicBasicPtr();
}

/*-------------------------------------------------------------------------*/

void 
CMsmqPubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess )
{GUCEF_TRACE;
        
    MT::CObjectScopeLock lock( this );

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        topicAccess.insert( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClient::DestroyTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
    {
        CMsmqPubSubClientTopicPtr topicAccess = (*i).second;
        m_topicMap.erase( i );

        TopicAccessDestroyedEventData eData( topicName );
        NotifyObservers( TopicAccessDestroyedEvent, &eData );
        
        topicAccess->Shutdown();
        topicAccess.Unlink();        
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CMsmqPubSubClient::GetDefaultTopicConfig( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.defaultTopicConfig;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr 
CMsmqPubSubClient::GetTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator i = m_config.topics.begin();
    while ( i != m_config.topics.end() )
    {
        if ( topicName == (*i)->topicName )
        {
            return (*i);
        }
        ++i;
    }
    return PUBSUB::CPubSubClientTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr 
CMsmqPubSubClient::GetOrCreateTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    PUBSUB::CPubSubClientTopicConfigPtr preExistingConfig = GetTopicConfig( topicName );
    if ( GUCEF_NULL != preExistingConfig )
        return preExistingConfig;
    
    if ( !m_config.defaultTopicConfig.IsNULL() )
    {
        PUBSUB::CPubSubClientTopicConfigPtr newTopicConfig( GUCEF_NEW PUBSUB::CPubSubClientTopicConfig( *m_config.defaultTopicConfig ) );
        newTopicConfig->topicName = topicName;
        m_config.topics.push_back( newTopicConfig );
        return newTopicConfig;
    }
    return PUBSUB::CPubSubClientTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClient::GetPrivateQueues( const std::wstring& computerName ,
                                     TWStringVector& queuePathNames   )
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
  
    // Define the required constants and variables.  
    const int NUMBEROFPROPERTIES = 1;                  // Number of properties  
    DWORD cPropId = 0;                                 // Property counter  
    DWORD cQ = 0;                                      // Queue counter  
    HRESULT hr = MQ_OK;                                // Return code  
  
    // Define an MQMGMTROPS structure.  
    MQMGMTPROPS mgmtprops;  
    MGMTPROPID aMgmtPropId[ NUMBEROFPROPERTIES ];  
    MQPROPVARIANT aMgmtPropVar[ NUMBEROFPROPERTIES ];  
  
    // Specify PROPID_MGMT_MSMQ_PRIVATEQ as a property to be retrieved.  
    aMgmtPropId[cPropId] = PROPID_MGMT_MSMQ_PRIVATEQ;  // Property ID  
    aMgmtPropVar[cPropId].vt = VT_NULL;                // Type indicator  
    cPropId++;  
  
    // Initialize the MQMGMTPROPS structure.  
    mgmtprops.cProp = cPropId;                         // Number of management properties  
    mgmtprops.aPropID = aMgmtPropId;                   // IDs of the management properties  
    mgmtprops.aPropVar = aMgmtPropVar;                 // Values of management properties  
    mgmtprops.aStatus  = NULL;                         // No storage for error codes  
  
    // Call MQMgmtGetInfo to retrieve the information.  
    hr = ::MQMgmtGetInfo( computerName.c_str() ,     // Name of the computer  
                          MO_MACHINE_TOKEN     ,     // Object name  
                          &mgmtprops           );    // Management properties structure  
  
    if ( FAILED( hr ) )  
    {  
        CORE::UInt32 errorCode =  HRESULT_CODE( hr );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( hr ) );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:GetPrivateQueues: Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
        return false;  
    }  
  
    // Display the path names of the private queues found.  
    if ( aMgmtPropVar[ 0 ].calpwstr.cElems > 0 )  
    {  
        for ( cQ=0; cQ < aMgmtPropVar[ 0 ].calpwstr.cElems; cQ++)  
        {  
            queuePathNames.push_back( aMgmtPropVar[ 0 ].calpwstr.pElems[ cQ ] );
        }  
    }  
  
    // Free the memory allocated to store the path names.  
    for ( cQ=0; cQ < aMgmtPropVar[ 0 ].calpwstr.cElems; cQ++ )  
    {  
        ::MQFreeMemory( aMgmtPropVar[ 0 ].calpwstr.pElems[ cQ ] );  
    }  
    ::MQFreeMemory( aMgmtPropVar[ 0 ].calpwstr.pElems );  
  
    return true;  

    #else

    // Not supported in older MSMQ
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClient::GetPrivateQueues( const CORE::CString& computerName           ,
                                     CORE::CString::StringVector& queuePathNames )
{GUCEF_TRACE;

    CORE::CUtf8String utf8ComputerName = CORE::ToUtf8String( computerName );
    std::wstring utf16ComputerName;
    if ( CORE::Utf8toUtf16( utf8ComputerName, utf16ComputerName ) )
    {
        TWStringVector wQueuePathNames;
        if ( GetPrivateQueues( utf16ComputerName ,
                               wQueuePathNames   ) )
        {
            TWStringVector::iterator i = wQueuePathNames.begin();
            while ( i != wQueuePathNames.end() )
            {
                std::string utf8QueuePathName;
                if ( CORE::Utf16toUtf8( (*i), utf8QueuePathName ) )
                {
                    queuePathNames.push_back( CORE::ToString( utf8QueuePathName ) );
                }
                ++i;
            }
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClient::GetLocalPrivateQueues( CORE::CString::StringVector& queuePathNames )
{GUCEF_TRACE;

    // Something of note:
    //      'The ComputerName and Host Name is limited to 15 characters by MSMQ. 
    //       If the Host Name contains more than 15 characters, MSMQ will truncate the name. 
    //       In this case, you must use the truncated Host Name.'    
    CORE::CString hostName = CORE::GetHostname();
    if ( hostName.Length() > 15 )
        hostName = hostName.CutChars( hostName.Length() - 15, false );

    return GetPrivateQueues( hostName, queuePathNames );
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::GetPublicQueues( CORE::CString::StringVector& queueIDs )
{GUCEF_TRACE;
  
    const int MAX_PROPERTIES = 13;   // 13 possible queue properties  
    bool totalSuccess = true;
    
    // Set a MQCOLUMNSET structure to specify the properties to be returned:           
    //      PROPID_Q_INSTANCE.  
    MQCOLUMNSET column;  
    PROPID aPropId[ 1 ];     // Nr of properties to retrieve  
    DWORD dwColumnCount = 0;  
  
    aPropId[dwColumnCount] = PROPID_Q_INSTANCE;  
    dwColumnCount++;  
  
    column.cCol = dwColumnCount;  
    column.aCol = aPropId;  
  
    // Call MQLocateBegin to start a Active Directory query.  
    // This won't work if you are not in an Active Directory domain
    HANDLE hEnum = NULL;  
    HRESULT hr = ::MQLocateBegin( NULL    ,   // Start search at the top  
                                  NULL    ,   // Search criteria  (NULL = return everything, no filters)
                                  &column ,   // Properties to return  
                                  NULL    ,   // No sort order  
                                  &hEnum  );  // Enumeration handle  
  
    if ( FAILED( hr ) )  
    {  
        if ( MQ_ERROR_UNSUPPORTED_OPERATION != hr )
        {
            CORE::UInt32 errorCode =  HRESULT_CODE( hr );
            std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( hr ) );
            totalSuccess = false;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:GetPublicQueues: MQLocateBegin Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
            return false; 
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:GetPublicQueues: MQLocateBegin is not a supported operation. Likely this machine is in a workgroup not an AD domain which is a requirement for using public queues" );
        }
        
        // This operation is not supported for Message Queuing installed in workgroup mode.
        // As such the list we obtained, nothing, actually is the complete list of available public queues since there are none
        return true;
    }  
  
    // Call MQLocateNext in a loop to examine the  
    // query results.  
    MQPROPVARIANT aPropVar[ MAX_PROPERTIES ];  
    DWORD cProps, i;  
  
    do  
    {  
        cProps = MAX_PROPERTIES;  
        hr = ::MQLocateNext( hEnum    ,   // Handle returned by MQLocateBegin  
                             &cProps  ,   // Size of aPropVar array  
                             aPropVar );  // An array of MQPROPVARIANT for results  
                        
  
        if ( FAILED( hr ) )  
        {  
            CORE::UInt32 errorCode =  HRESULT_CODE( hr );
            std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( hr ) );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:GetPublicQueues: MQLocateNext Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
            totalSuccess = false;
            break;  
        }  
  
        for ( i=0; i < cProps; i += dwColumnCount )    
        {  
            if ( VT_CLSID == aPropVar[ i ].vt )
            {
                CORE::CAsciiString queueGuid;
                if ( CMsmqPubSubClientTopic::MsmqGUIDToString( *aPropVar[ i ].puuid, queueGuid ) )
                {
                    queueIDs.push_back( queueGuid );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:GetPublicQueues: Found public queue with ID " + queueGuid );  
                }
            }
        }  
  
    } 
    while ( cProps > 0 );  
  
    // Call MQLocateEnd to end query.  
    hr = ::MQLocateEnd( hEnum );   // Handle returned by MQLocateBegin.  
    if ( FAILED( hr ) )  
    {  
        CORE::UInt32 errorCode =  HRESULT_CODE( hr );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( hr ) );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:GetPublicQueues: MQLocateEnd Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );  
        totalSuccess = false;
    } 

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClient::BeginTopicDiscovery( const CORE::CString::StringSet& globPatternFilters )
{GUCEF_TRACE;

    TopicDiscoveryEventData discoveredTopics;
    
    // For discovery in the MSMQ realm we always use the format name in order to be able to collapse sources into a singular uniform list

    // Let's first check the private local queues as that is most likely to succeed
    CORE::CString::StringVector localPrivateQueuePathNames;
    if ( GetLocalPrivateQueues( localPrivateQueuePathNames ) )
    {
        CORE::CString::StringVector::iterator i = localPrivateQueuePathNames.begin();
        while ( i != localPrivateQueuePathNames.end() )
        {
            CORE::CString queueFormatName;
            if ( CMsmqPubSubClientTopic::MsmqPathNameToMsmqQueueFormatName( (*i), queueFormatName ) )
            {
                if ( globPatternFilters.empty() || queueFormatName.WildcardEquals( globPatternFilters, '*', true, true ) )
                {
                    discoveredTopics.insert( queueFormatName );
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:BeginTopicDiscovery: Found private queue: " + queueFormatName );
                }
            }
            ++i;
        }
    }

    // Next try our luck at any public queues we have access to
    CORE::CString::StringVector publicQueueIDs;
    if ( GetPublicQueues( publicQueueIDs ) )
    {
        CORE::CString::StringVector::iterator i = publicQueueIDs.begin();
        while ( i != publicQueueIDs.end() )
        {
            CORE::CString queueFormatName;
            if ( CMsmqPubSubClientTopic::MsmqQueueGUIDToMsmqQueueFormatName( (*i), queueFormatName ) )
            {
                if ( globPatternFilters.empty() || queueFormatName.WildcardEquals( globPatternFilters, '*', true, true ) )
                {
                    discoveredTopics.insert( queueFormatName );
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:BeginTopicDiscovery: Found public queue: " + queueFormatName );
                }
            }
            ++i;
        }        
    }

    NotifyObservers( TopicDiscoveryEvent, &discoveredTopics );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClient::GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator i = m_config.topics.begin();
    while ( i != m_config.topics.end() )
    {
        topicNameList.insert( (*i)->topicName );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClient::GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        topicNameList.insert( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CMsmqPubSubClient::GetType( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    // Try to see if we can properly load the entire config before
    // applying it. If not stick with old config vs corrupt config
    CMsmqPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        MT::CScopeMutex lock( m_lock );
        m_config = parsedCfg;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg  )
{GUCEF_TRACE;

    // Try to see if we can properly load the entire config before
    // applying it. If not stick with old config vs corrupt config
    CMsmqPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        MT::CScopeMutex lock( m_lock );
        m_config = parsedCfg;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::Disconnect( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    bool totalSuccess = true;
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        totalSuccess = (*i).second->Disconnect() && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::Connect( bool reset )
{GUCEF_TRACE;

    if ( !reset && IsConnected() )
        return true;

    MT::CScopeMutex lock( m_lock );

    if ( !m_topicMap.empty() )
    {
        bool allConnected = true;
        TTopicMap::iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            allConnected = (*i).second->InitializeConnectivity( false ) && allConnected;
            ++i;
        }
        return allConnected;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_topicMap.empty() )
    {
        bool allConnected = true;
        TTopicMap::const_iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            allConnected = (*i).second->IsConnected() && allConnected;
            ++i;
        }
        return allConnected;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::IsHealthy( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    if ( !m_topicMap.empty() )
    {
        // Aggregate the health status of all topics
        bool allHealthy = true;
        TTopicMap::const_iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            allHealthy = (*i).second->IsHealthy() && allHealthy;
            ++i;
        }

        // Notify if there was a change in status
        if ( allHealthy != m_isHealthy )
        {
            m_isHealthy = allHealthy;        

            if ( m_isHealthy )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:IsHealthy: overall health is now Ok" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:IsHealthy: overall health status is now unhealthy" );         
            }

            lock.EarlyUnlock();
            THealthStatusChangeEventData eData( allHealthy ); 
            NotifyObservers( HealthStatusChangeEvent, &eData );         
        }

        return allHealthy;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClient::IsInitialized( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClient::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_metricsTimer )
    {
        TEventCallback callback( this, &CMsmqPubSubClient::OnMetricsTimerCycle );
        SubscribeTo( m_metricsTimer                 ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }
}

/*-------------------------------------------------------------------------*/

void 
CMsmqPubSubClient::RegisterTopicEventHandlers( CMsmqPubSubClientTopicPtr& topic )
{GUCEF_TRACE;

    if ( !topic.IsNULL() )
    {
        TEventCallback callback( this, &CMsmqPubSubClient::OnTopicHealthStatusChange );
        SubscribeTo( topic.GetPointerAlways()                        ,
                     CMsmqPubSubClientTopic::HealthStatusChangeEvent ,
                     callback                                        );
    }
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClient::OnTopicHealthStatusChange( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // (Re)determine the aggregate health status
    IsHealthy();
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CMsmqPubSubClient::GetMsmqComputerMetric( const CORE::CString& metricDescription ,
                                          UInt32 propId                          ,
                                          UInt32 propType                        )
{GUCEF_TRACE;

    // Define the required constants and variables.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId = 0;  

    // Define an MQMGMTROPS structure.  
    ::MQMGMTPROPS mgmtprops;  
    ::MGMTPROPID aMgmtPropId[ NUMBEROFPROPERTIES ];  
    ::MQPROPVARIANT aMgmtPropVar[ NUMBEROFPROPERTIES ];  

    aMgmtPropId[ cPropId ] = (PROPID) propId;              // Property identifier  
    aMgmtPropVar[ cPropId ].vt = (VARTYPE) propType;       // Type indicator  
    ++cPropId;

    // Initialize the MQMGMTPROPS structure.  
    mgmtprops.cProp = cPropId;   // number of management properties  
    mgmtprops.aPropID = aMgmtPropId;// management property IDs  
    mgmtprops.aPropVar = aMgmtPropVar;// management property values  
    mgmtprops.aStatus  = NULL;// no storage for error codes   

    // Now that we formulated the request
    // actually ask for the info
    HRESULT computerInfoFetchResult = ::MQMgmtGetInfo( NULL, L"MACHINE", &mgmtprops );
    if ( MQ_OK == computerInfoFetchResult )
    {
        switch ( propType )
        {
            case VT_UI4: return (CORE::Int64) aMgmtPropVar[ 0 ].ulVal;
            case VT_UI8: return (CORE::Int64) aMgmtPropVar[ 0 ].uhVal.QuadPart;

            default:
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:GetMsmqComputerMetric: PropType is unsupported. Change the code! Type=" + CORE::ToString( propType ) );
                return 0;
            }
        }
        
    }

    CORE::UInt32 errorCode =  HRESULT_CODE( computerInfoFetchResult );
    std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( computerInfoFetchResult ) );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClient:GetMsmqComputerMetric: Failed to obtain " + metricDescription + ". errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
    return -1;
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CMsmqPubSubClient::GetComputerGlobalTotalBytesOfAllMessagesOfAllQueues( void ) 
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    return (CORE::UInt64) GetMsmqComputerMetric( "global byte count of messages", PROPID_MGMT_MSMQ_BYTES_IN_ALL_QUEUES, VT_UI8 );
    #else
    return 0;
    #endif
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClient::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // Quickly grab a snapshot of metric values for all topics 
    // we don't combine this with metrics publishing as it adds to metrics timeframe drift across topics
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnMetricsTimerCycle( notifier  ,
                                          eventId   ,
                                          eventData );
        ++i;
    }

    // Now publish all the metrics to the metrics system
    i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        CMsmqPubSubClientTopicPtr topic = (*i).second;
        const CMsmqPubSubClientTopic::TopicMetrics& topicMetrics = topic->GetMetrics();
        const CORE::CString& topicName = topic->GetMetricFriendlyTopicName();
        const CMsmqPubSubClientTopicConfig& topicConfig = topic->GetTopicConfig();
        CORE::CString metricsPrefix = m_config.metricsPrefix + topicName;

        if ( topicMetrics.msmqMsgsInQueue >= 0 )
            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsInQueue", topicMetrics.msmqMsgsInQueue, 1.0f );
        if ( topicMetrics.msmqMsgsInJournal >= 0 )
            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsInJournal", topicMetrics.msmqMsgsInJournal, 1.0f );
        if ( topicMetrics.msmqMsgBytesInQueue >= 0 )
            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgBytesInQueue", topicMetrics.msmqMsgBytesInQueue, 1.0f );
        if ( topicMetrics.msmqMsgBytesInJournal >= 0 )
            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgBytesInJournal", topicMetrics.msmqMsgBytesInJournal, 1.0f );

        if ( m_config.desiredFeatures.supportsPublishing )
        {
            GUCEF_METRIC_COUNT( metricsPrefix + ".msmqMessagesPublished", topicMetrics.msmqMessagesPublished, 1.0f );
            GUCEF_METRIC_COUNT( metricsPrefix + ".msmqErrorsOnPublish", topicMetrics.msmqErrorsOnPublish, 1.0f );
        }
        if ( m_config.desiredFeatures.supportsSubscribing )
        {
            GUCEF_METRIC_COUNT( metricsPrefix + ".msmqMessagesReceived", topicMetrics.msmqMessagesReceived, 1.0f );
            GUCEF_METRIC_COUNT( metricsPrefix + ".msmqErrorsOnReceive", topicMetrics.msmqErrorsOnReceive, 1.0f );

            // For MSMQ 3.0 and above:
            #if ( _WIN32_WINNT >= 0x0501 )

            GUCEF_METRIC_COUNT( metricsPrefix + ".msmqErrorsOnReceiveAck", topicMetrics.msmqErrorsOnAck, 1.0f );

            #endif
        }

        if ( topicConfig.gatherMsmqTransitTimeOnReceiveMetric )
        {
            CMsmqPubSubClientTopic::UInt32Vector::const_iterator n = topicMetrics.msmqMsgSentToArriveLatencies.begin();
            while ( n != topicMetrics.msmqMsgSentToArriveLatencies.end() )
            {
                GUCEF_METRIC_TIMING( metricsPrefix + ".msmqMsgSentToArriveLatencyInMs", (*n), 1.0f );
                ++n;
            }
        }

        ++i;
    }

    if ( m_config.gatherMsmqMetricForTotalBytesAllQueues )
    {
        CORE::UInt64 msmqGlobalMsgBytes = GetComputerGlobalTotalBytesOfAllMessagesOfAllQueues();
        GUCEF_METRIC_GAUGE( m_config.metricsPrefix + "msmqGlobalMsgBytes", msmqGlobalMsgBytes, 1.0f );
    }
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CMsmqPubSubClient::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CMsmqPubSubClient::Unlock( void ) const 
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CMsmqPubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::MSMQ::CMsmqPubSubClient";
    return classTypeName;
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
