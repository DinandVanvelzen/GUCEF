/*
 *  pubsubpluginSTORAGE: Generic GUCEF plugin for PUBSUB module functionality
 *                       providing a storage interface
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

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTCONFIG_H
#include "pubsubpluginSTORAGE_CStoragePubSubClientConfig.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTCONFIG_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTCONFIG_H ? */

#include "pubsubpluginSTORAGE_CStoragePubSubClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace STORAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CStoragePubSubClient::TypeName = "STORAGE";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStoragePubSubClient::CStoragePubSubClient( const PUBSUB::CPubSubClientConfig& config )
    : PUBSUB::CPubSubClient( config.pulseGenerator )
    , m_config()
    , m_journal()
    , m_metricsTimer( GUCEF_NULL )
    , m_topicMap()
    , m_pubsubBookmarkPersistence()
    , m_threadPool()
    , m_isHealthy( true )
    , m_needToTrackAcks( true )
    , m_lock()
{GUCEF_TRACE;

    if ( !LoadConfig( config ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StoragePubSubClient: Failed to load config at construction" );
    }

    ConfigureJournal( m_config );

    if ( config.desiredFeatures.supportsMetrics )
    {
        m_metricsTimer = GUCEF_NEW CORE::CTimer( config.pulseGenerator, 1000 );
        m_metricsTimer->SetEnabled( config.desiredFeatures.supportsMetrics );
    }

    m_threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "StoragePubSubClient(" + CORE::ToString( this ) + ")", GetPulseGenerator(), true );
    m_config.metricsPrefix += "storage.";

    RegisterEventHandlers();

    if ( !m_journal.IsNULL() )
        m_journal->AddClientCreatedJournalEntry();
}

/*-------------------------------------------------------------------------*/

CStoragePubSubClient::~CStoragePubSubClient()
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

    m_pubsubBookmarkPersistence.Unlink();

    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    if ( !m_journal.IsNULL() )
    {
        m_journal->AddClientDestroyedJournalEntry();
        m_journal.Unlink();
    }

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    return SetPulseGenerator( newPulseGenerator, true );
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
                                         bool includeTopics                        )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    CORE::CTSGNotifier::SetPulseGenerator( newPulseGenerator );
    m_config.pulseGenerator = newPulseGenerator;
    
    if ( GUCEF_NULL != m_metricsTimer )
    {
        m_metricsTimer->SetPulseGenerator( newPulseGenerator );
    }
    if ( !m_threadPool.IsNULL() )
    {
        m_threadPool->SetPulseGenerator( newPulseGenerator );
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

CStoragePubSubClientConfig&
CStoragePubSubClient::GetConfig( void )
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

CORE::ThreadPoolPtr
CStoragePubSubClient::GetThreadPool( void )
{GUCEF_TRACE;

    return m_threadPool;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const
{GUCEF_TRACE;

    features.supportsBinaryPayloads = true;             // The storage backend with the natively compatible pub-sub serializers can always support everything
    features.supportsPerMsgIds = true;                  // The storage backend with the natively compatible pub-sub serializers can always support everything
    features.supportsPrimaryPayloadPerMsg = true;       // The storage backend with the natively compatible pub-sub serializers can always support everything
    features.supportsAbsentPrimaryPayloadPerMsg = true; // The storage backend with the natively compatible pub-sub serializers can always support everything
    features.supportsKeyValueSetPerMsg = true;          // The storage backend with the natively compatible pub-sub serializers can always support everything
    features.supportsDuplicateKeysPerMsg = true;        // The storage backend with the natively compatible pub-sub serializers can always support everything
    features.supportsMetaDataKeyValueSetPerMsg = true;  // The storage backend with the natively compatible pub-sub serializers can always support everything
    features.supportsMultiHostSharding = false;         // Its possible the VFS storage is multi-host but we'd have no way of knowing. Afaik its 1 file system
    features.supportsPublishing = true;                 // We support writing to VFS storage files
    features.supportsSubscribing = true;                // We support reading from VFS storage files
    features.supportsMetrics = true;                    // This plugin has support for reporting its own set of metrics
    features.supportsAutoReconnect = true;              // To the extent it even applies in this case, sure we 'reconnect' to storage
    features.supportsDiscoveryOfAvailableTopics = true; // The VFS dirs available per config will be regarded as available 'topics'
    features.supportsGlobPatternTopicNames = true;      // We support generating topics from VFS directory name matching using glob patterns
    features.supportsSubscriptionMsgArrivalDelayRequests = false;
    features.supportsSubscriptionEndOfDataEvent = true; // we support sending these at the end of every request fullfillment or when we run out of requests to fullfill
    features.supportsReplay = true;                     // We support replaying stored messages starting at a given key value via RequestReplay() and the sidecar index

    features.supportsBookmarkingConcept = true;         // We can create a reference to the storage location plus offset
    features.supportsAutoBookmarking = false;           // Currently we do 'forget' where we are if the app crashes
    features.supportsMsgIdBasedBookmark = false;        // In this context we have no idea what the message ID is, as such we cannot use it as a bookmark since we cannot garantee anything
    features.supportsMsgIndexBasedBookmark = false;     // In this context we have no idea what the message Index is, as such we cannot use it as a bookmark since we cannot garantee anything
    features.supportsTopicIndexBasedBookmark = true;    // We can create a reference to the storage location plus offset
    features.supportsMsgDateTimeBasedBookmark = false;      // In this context we have no idea what the message datetime is, as such we cannot use it as a bookmark since we cannot garantee anything
    features.supportsServerSideBookmarkPersistance = false; // Currently we do 'forget' where we are if the app crashes. @TODO? Maybe we can add storage backend opinionated bookmark storage
    features.supportsSubscribingUsingBookmark = true;       // We can create a reference to the storage location plus offset and then use that to resume the reading from that location

    // since the storage backend is more of a transcribing passthrough it doesnt know what on the message means what.
    // As such it can only support this runtime via the receiveActionId
    features.supportsDerivingBookmarkFromMsg = m_config.desiredFeatures.supportsSubscribing && m_config.desiredFeatures.supportsDerivingBookmarkFromMsg;

    features.supportsAbsentMsgReceivedAck = m_config.desiredFeatures.supportsAbsentMsgReceivedAck;
    features.supportsAckUsingLastMsgInBatch = m_config.desiredFeatures.supportsAckUsingLastMsgInBatch;
    features.supportsAutoMsgReceivedAck = m_config.desiredFeatures.supportsAutoMsgReceivedAck;
    features.supportsSubscriberMsgReceivedAck = m_config.desiredFeatures.supportsSubscribing;
    features.supportsAckUsingBookmark = features.supportsDerivingBookmarkFromMsg;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::DetermineIfTrackingAcksIsNeeded( void ) const
{GUCEF_TRACE;

    PUBSUB::CPubSubClientFeatures features;
    GetSupportedFeatures( features );

    // Whether we need to track successfull message handoff (garanteed handling) depends both on whether we want that extra reliability per the config
    // (optional since nothing is free and this likely degrades performance a bit) but also whether the backend even supports it.
    // If the backend doesnt support it all we will be able to do between the sides is fire-and-forget

    bool doWeWantIt = ( m_config.desiredFeatures.supportsSubscribing &&                         // <- does it apply in this context ?
                        ( m_config.desiredFeatures.supportsSubscriberMsgReceivedAck ||          // <- do we want it?
                          m_config.desiredFeatures.supportsSubscribingUsingBookmark  )
                      );

    bool isItSupported = features.supportsSubscriberMsgReceivedAck ||
                         ( features.supportsBookmarkingConcept && features.supportsSubscribingUsingBookmark );

    bool canWeNotWantIt = features.supportsAbsentMsgReceivedAck &&          // <- Is it even an option to not do it regardless of desired features
                          ( !features.supportsBookmarkingConcept ||         // <- if we need to perform client-side bookmarking then its not really an option to forgo acks if you want a reliable handoff and thus bookmark progression
                             features.supportsBookmarkingConcept && features.supportsSubscribingUsingBookmark && features.supportsServerSideBookmarkPersistance );

    bool acksNeeded =  ( doWeWantIt && isItSupported ) ||
                       ( !canWeNotWantIt && isItSupported );

    return acksNeeded;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::IsTrackingAcksNeeded( void ) const
{GUCEF_TRACE;

    return m_needToTrackAcks;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::GetMultiTopicAccess( const CORE::CString& topicName    ,
                                           PubSubClientTopicSet& topicAccess )
{GUCEF_TRACE;

    // Check to see if this logical/conceptual 'topic' name represents multiple pattern matched vfs paths
    if ( m_config.desiredFeatures.supportsGlobPatternTopicNames &&
         topicName.HasChar( '*' ) > -1                           )
    {
        // We create the actual topic objects from the wildcard glob pattern topic which is used
        // as a template. As such we need to match the pattern again to find the various topics that could have been spawned
        bool matchesFound = false;
        TTopicMap::iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            if ( (*i).first.WildcardEquals( topicName, '*', true ) )
            {
                topicAccess.insert( (*i).second );
                matchesFound = true;
            }
            ++i;
        }
        return matchesFound;
    }
    else
    {
        TTopicMap::iterator i = m_topicMap.find( topicName );
        if ( i != m_topicMap.end() )
        {
            topicAccess.insert( (*i).second );
            return true;
        }
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::GetMultiTopicAccess( const CORE::CString::StringSet& topicNames ,
                                           PubSubClientTopicSet& topicAccess          )
{GUCEF_TRACE;

    bool totalSuccess = true;
    CORE::CString::StringSet::const_iterator i = topicNames.begin();
    while ( i != topicNames.end() )
    {
        totalSuccess = GetMultiTopicAccess( (*i), topicAccess ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr
CStoragePubSubClient::CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                         CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    CStoragePubSubClientTopicPtr topicAccess;
    {
        MT::CScopeMutex lock( m_lock );

        if ( pulseGenerator.IsNULL() )
            pulseGenerator = m_config.topicPulseGenerator; // none given, use default for topics
        if ( pulseGenerator.IsNULL() )
            pulseGenerator = GetPulseGenerator();          // none present, use client level pulse generator

        TTopicMap::iterator i = m_topicMap.find( topicConfig->topicName );
        if ( i == m_topicMap.end() )
        {
            topicAccess = ( GUCEF_NEW CStoragePubSubClientTopic( this, pulseGenerator ) )->CreateSharedPtr();
            if ( topicAccess->LoadConfig( *topicConfig ) )
            {
                m_topicMap[ topicConfig->topicName ] = topicAccess;

                ConfigureJournal( topicAccess, topicConfig );
                PUBSUB::CIPubSubJournalBasicPtr journal = topicAccess->GetJournal();
                if ( !journal.IsNULL() && topicConfig->journalConfig.useJournal )
                    journal->AddTopicCreatedJournalEntry();

                RegisterTopicEventHandlers( topicAccess );

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):CreateTopicAccess: Created topic access for topic \"" +
                        topicConfig->topicName + "\" with implementation " + CORE::ToString( topicAccess.GetPointerAlways() ) );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):CreateTopicAccess: Failed to load config while creating topic access for topic \"" +
                        topicConfig->topicName + "\" with implementation " + CORE::ToString( topicAccess.GetPointerAlways() ) );

                topicAccess->Shutdown();
                topicAccess.Unlink();
            }
        }   
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):CreateTopicAccess: Topic access for topic \"" + topicConfig->topicName + "\" already exists" );
            return (*i).second;
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

bool
CStoragePubSubClient::AutoCreateMultiTopicAccess( const TTopicConfigPtrToStringSetMap& topicsToCreate ,
                                                  PubSubClientTopicSet& topicAccess                   ,
                                                  CORE::PulseGeneratorPtr pulseGenerator              )
{GUCEF_TRACE;

    CORE::UInt32 newTopicAccessCount = 0;
    bool totalSuccess = true;
    {
        MT::CObjectScopeLock lock( this );

        TTopicConfigPtrToStringSetMap::const_iterator m = topicsToCreate.begin();
        while ( m != topicsToCreate.end() )
        {
            PUBSUB::CPubSubClientTopicConfigPtr templateTopicConfig( ((*m).first) );
            if ( !templateTopicConfig.IsNULL() ) 
            {
                const CORE::CString::StringSet& topicNameList = (*m).second;

                CORE::CString::StringSet::const_iterator i = topicNameList.begin();
                while ( i != topicNameList.end() )
                {
                    const CORE::CString& topicName = (*i);
                    TTopicMap::iterator t = m_topicMap.find( topicName );
                    if ( t == m_topicMap.end() )
                    {
                        CStoragePubSubClientTopicConfigPtr topicConfig = CStoragePubSubClientTopicConfig::CreateSharedObj();
                        topicConfig->LoadConfig( *templateTopicConfig.GetPointerAlways() ); 
                        topicConfig->topicName = topicName;
                        topicConfig->vfsStorageRootPath = m_config.vfsStorageRootPath + GUCEF_VFS_DIR_SEP_CHAR + topicName + GUCEF_VFS_DIR_SEP_CHAR;

                        CStoragePubSubClientTopicPtr tAccess;
                        {
                            MT::CObjectScopeLock lock( this );

                            tAccess = ( GUCEF_NEW CStoragePubSubClientTopic( this, pulseGenerator ) )->CreateSharedPtr();
                            if ( tAccess->LoadConfig( *topicConfig ) )
                            {
                                m_topicMap[ topicConfig->topicName ] = tAccess;                            
                                topicAccess.insert( tAccess );
                                m_config.topics.push_back( topicConfig );
                                ++newTopicAccessCount;

                                ConfigureJournal( tAccess, topicConfig );
                                PUBSUB::CIPubSubJournalBasicPtr journal = tAccess->GetJournal();
                                if ( !journal.IsNULL() && topicConfig->journalConfig.useJournal )
                                    journal->AddTopicCreatedJournalEntry();

                                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):AutoCreateMultiTopicAccess: Auto created topic \"" +
                                        topicConfig->topicName + "\" based on template config \"" + templateTopicConfig->topicName + "\" with implementation " + CORE::ToString( tAccess.GetPointerAlways() ) );
                        
                                RegisterTopicEventHandlers( tAccess );
                            }
                            else
                            {
                                tAccess.Unlink();
                                totalSuccess = false;

                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):AutoCreateMultiTopicAccess: Failed to load config for topic \"" +
                                        topicConfig->topicName + "\" based on template config \"" + templateTopicConfig->topicName + "\" with implementation " + CORE::ToString( tAccess.GetPointerAlways() ) );
                            }
                        }
                    }
                    else
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):AutoCreateMultiTopicAccess: Topic access for topic \"" + topicName + "\" already exists" );
                    }
                    ++i;
                }
            }
            ++m;
        }
    }

    if ( newTopicAccessCount > 0 )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Auto created " +
            CORE::ToString( newTopicAccessCount ) + " topics based on template configs" );

        TopicsAccessAutoCreatedEventData eData( topicAccess );
        NotifyObservers( TopicsAccessAutoCreatedEvent, &eData );
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::AutoCreateMultiTopicAccess( CStoragePubSubClientTopicConfigPtr templateTopicConfig ,
                                                  const CORE::CString::StringSet& topicNameList          ,
                                                  PubSubClientTopicSet& topicAccess                      ,
                                                  CORE::PulseGeneratorPtr pulseGenerator                 )
{GUCEF_TRACE;

    TTopicConfigPtrToStringSetMap topicToCreate;
    topicToCreate[ templateTopicConfig ] = topicNameList;
    return AutoCreateMultiTopicAccess( topicToCreate, topicAccess, pulseGenerator );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::CreateMultiTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                              PubSubClientTopicSet& topicAccess               ,
                                              CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    if ( m_config.desiredFeatures.supportsGlobPatternTopicNames &&
         topicConfig->topicName.HasChar( '*' ) > -1               )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:CreateMultiTopicAccess: Will look for dirs from which to generate topics using vfs root: " + m_config.vfsStorageRootPath );
        
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        
        VFS::CVFS::TStringVector dirList;
        if ( vfs.GetDirList( dirList                               , 
                             m_config.vfsStorageRootPath           , 
                             m_config.dirTopicDiscoveryIsRecursive , 
                             m_config.includeDirParentInTopicName  , 
                             topicConfig->topicName                ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:CreateMultiTopicAccess: Found " + CORE::ToString( dirList.size() ) + " dirs matching topic filter: " + topicConfig->topicName );

            // When used as topic names we dont want a trailing '/'
            VFS::CVFS::TStringVector::iterator i = dirList.begin();
            while ( i != dirList.end() )
            {
                CORE::CString& topicName = (*i);
                if ( !topicName.IsNULLOrEmpty() && topicName[ topicName.Length()-1 ] == GUCEF_VFS_DIR_SEP_CHAR )
                {
                    (*i) = topicName.CutChars( 1, false );
                }
                ++i;
            }

            CORE::CString::StringSet topicNames( CORE::ToStringSet( dirList ) );            
            if ( !topicNames.empty() )
            {                
                return AutoCreateMultiTopicAccess( topicConfig, topicNames, topicAccess, pulseGenerator );
            }
            return true; // Since its pattern based potential creation at a later time also counts as success
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:CreateMultiTopicAccess: Failed to get dir listing for vfs root: " + m_config.vfsStorageRootPath );
            return false;
        }
    }
    else
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:CreateMultiTopicAccess: Interpreting topic name litterally: " + topicConfig->topicName );
        
        PUBSUB::CPubSubClientTopicBasicPtr tAccess = CreateTopicAccess( topicConfig, pulseGenerator );
        if ( !tAccess.IsNULL() )
        {
            topicAccess.insert( tAccess );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::AutoDestroyTopicAccess( const CORE::CString::StringSet& topicNames )
{GUCEF_TRACE;

    PubSubClientTopicSet topicAccess;
    {
        MT::CScopeMutex lock( m_lock );

        CORE::CString::StringSet::const_iterator t = topicNames.begin();
        while ( t != topicNames.end() )
        {
            const CORE::CString& topicName = (*t);
            TTopicMap::iterator i = m_topicMap.find( topicName );
            if ( i != m_topicMap.end() )
            {
                CStoragePubSubClientTopicPtr tAccess = (*i).second;
                topicAccess.insert( tAccess );
            }
            ++t;
        }
    }

    if ( !topicAccess.empty() )
    {
        TopicsAccessAutoDestroyedEventData eData( topicAccess );
        NotifyObservers( TopicsAccessAutoDestroyedEvent, &eData );

        {
            // Now that everyone has been duely warned we can proceed with the actual destruction
            MT::CObjectScopeLock lock( this );

            CORE::UInt32 destroyedTopicAccessCount = 0;
            PubSubClientTopicSet::iterator t = topicAccess.begin();
            while ( t != topicAccess.end() )
            {
                PUBSUB::CPubSubClientTopicBasicPtr tAccess = (*t);

                CORE::CString topicName = tAccess->GetTopicName();
                m_topicMap.erase( topicName );                
                {
                    CStoragePubSubClientTopicBasicPtr topicAccess = tAccess.StaticCast< CStoragePubSubClientTopic >();
                    topicAccess->Shutdown();
                    topicAccess.Unlink();
                }
                tAccess.Unlink();

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):AutoDestroyTopicAccess: destroyed topic access for topic \"" + topicName + "\"" );

                ++destroyedTopicAccessCount;
                ++t;
            }

            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):AutoDestroyTopicAccess: destroyed topic access for " +
                CORE::ToString( destroyedTopicAccessCount ) + "topics" );
        }
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr
CStoragePubSubClient::GetTopicAccess( const CORE::CString& topicName )
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
CStoragePubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        topicAccess.insert( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::DestroyTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
    {
        CStoragePubSubClientTopicPtr topicAccess = (*i).second;
        m_topicMap.erase( i );

        lock.EarlyUnlock();
        TopicAccessDestroyedEventData eData( topicName );
        NotifyObservers( TopicAccessDestroyedEvent, &eData );

        topicAccess->Shutdown();
        topicAccess.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CStoragePubSubClient::GetDefaultTopicConfig( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.defaultTopicConfig;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CStoragePubSubClient::GetTopicConfig( const CORE::CString& topicName )
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
CStoragePubSubClient::GetOrCreateTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientTopicConfigPtr preExistingConfig = GetTopicConfig( topicName );
    if ( !preExistingConfig.IsNULL() )
        return preExistingConfig;

    CStoragePubSubClientTopicConfigPtr newTopicConfig = CStoragePubSubClientTopicConfig::CreateSharedObj();
    if ( !newTopicConfig.IsNULL() && 
         !m_config.defaultTopicConfig.IsNULL() && 
         newTopicConfig->LoadConfig( *m_config.defaultTopicConfig ) )
    {
        newTopicConfig->topicName = topicName;
        m_config.topics.push_back( newTopicConfig );
        return newTopicConfig;
    }
    return PUBSUB::CPubSubClientTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::OnVfsDirCreated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CDirectoryWatcherEvents::TDirCreatedEventData* dirCreatedEventData = static_cast< CORE::CDirectoryWatcherEvents::TDirCreatedEventData* >( eventData );
    if ( GUCEF_NULL != dirCreatedEventData )
    {
        const CORE::CString& newDirPath = (*dirCreatedEventData);
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        
        // Check to see if this dir is a sub dir of the storage root, it could be an irrelevant dir
        if ( vfs.IsDirPathSubOfDirPath( m_config.vfsStorageRootPath, newDirPath ) )
        {
            // Do we take action for new dirs that are created in the storage root ?
            if ( m_config.desiredFeatures.supportsDiscoveryOfAvailableTopics )
            {
                PubSubClientTopicSet topicAccess;
                TTopicConfigPtrToStringSetMap topicToCreate;
                topicToCreate[ m_config.defaultTopicConfig ].insert( newDirPath );
                AutoCreateMultiTopicAccess( topicToCreate, topicAccess, m_config.topicPulseGenerator );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::OnVfsDirDeleted( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CDirectoryWatcherEvents::TDirDeletedEventData* dirDeletedEventData = static_cast< CORE::CDirectoryWatcherEvents::TDirDeletedEventData* >( eventData );
    if ( GUCEF_NULL != dirDeletedEventData )
    {
        const CORE::CString& deletedDirPath = (*dirDeletedEventData);
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        
        // Check to see if this dir is a sub dir of the storage root, it could be an irrelevant dir
        if ( vfs.IsDirPathSubOfDirPath( m_config.vfsStorageRootPath, deletedDirPath ) )
        {
            // Do we take action for new dirs that are created in the storage root ?
            if ( m_config.destroyTopicWhenDirIsDeleted )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):OnVfsDirDeleted: auto destroying topic access for vfs path " +
                    CORE::ToString( deletedDirPath ) );

                DestroyTopicAccess( deletedDirPath );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::SubscribeToVfsTopicPathEvents( void )
{GUCEF_TRACE;

    bool success = true;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    CORE::CIDirectoryWatcher::CDirWatchOptions dirWatchOptions( false );
    dirWatchOptions.watchForDirCreation = m_config.desiredFeatures.supportsDiscoveryOfAvailableTopics;
    dirWatchOptions.watchForDirDeletion = m_config.destroyTopicWhenDirIsDeleted;
    dirWatchOptions.watchForDirRenames  = true;
    dirWatchOptions.watchSubTree        = m_config.dirTopicDiscoveryIsRecursive;

    if ( vfs.AddDirToWatch( m_config.vfsStorageRootPath, dirWatchOptions ) )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:SubscribeToVfsTopicPathEvents: Now watching vfs storage root: " + m_config.vfsStorageRootPath );
    }
    else
    {
        success = false;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:SubscribeToVfsTopicPathEvents: Failed to add watch to vfs storage root: " + m_config.vfsStorageRootPath );
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::BeginTopicDiscovery( const CORE::CString::StringSet& globPatternFilters )
{GUCEF_TRACE;

    if ( !m_config.desiredFeatures.supportsDiscoveryOfAvailableTopics )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:BeginTopicDiscovery: Topic discovery is disabled" );
        return false;
    }

    SubscribeToVfsTopicPathEvents();
    
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    VFS::CVFS::TStringVector dirList;
    if ( vfs.GetDirList( dirList                               , 
                         m_config.vfsStorageRootPath           , 
                         m_config.dirTopicDiscoveryIsRecursive , 
                         m_config.includeDirParentInTopicName  , 
                         globPatternFilters                    ) )
    {
        if ( !dirList.empty() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:BeginTopicDiscovery: Found " + CORE::ToString( dirList.size() ) + " dirs" );

            // When used as topic names we dont want a trailing '/'
            VFS::CVFS::TStringVector::iterator i = dirList.begin();
            while ( i != dirList.end() )
            {
                CORE::CString& topicName = (*i);
                if ( !topicName.IsNULLOrEmpty() && topicName[ topicName.Length()-1 ] == GUCEF_VFS_DIR_SEP_CHAR )
                {
                    (*i) = topicName.CutChars( 1, false );
                }
                ++i;
            }

            TopicDiscoveryEventData topicNames( CORE::ToStringSet( dirList ) );            
            NotifyObservers( TopicDiscoveryEvent, &topicNames );
            return true;
        }
        return true;
    }
    
    return false;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList )
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
CStoragePubSubClient::GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList )
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
CStoragePubSubClient::GetType( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

PUBSUB::TIPubSubBookmarkPersistenceBasicPtr
CStoragePubSubClient::GetBookmarkPersistence( void ) const
{GUCEF_TRACE;

    return m_pubsubBookmarkPersistence;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::ConfigureBookmarkPersistance( void )
{GUCEF_TRACE;

    m_pubsubBookmarkPersistence.Unlink();

    // Create and configure the pub-sub bookmark persistence
    m_pubsubBookmarkPersistence = PUBSUB::CPubSubGlobal::Instance()->GetPubSubBookmarkPersistenceFactory().Create( m_config.pubsubBookmarkPersistenceConfig.bookmarkPersistenceType, m_config.pubsubBookmarkPersistenceConfig );

    if ( m_pubsubBookmarkPersistence.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StoragePubSubClient(" + CORE::PointerToString( this ) +
            "):ConfigureBookmarkPersistance: Failed to create bookmark persistance access of type \"" + m_config.pubsubBookmarkPersistenceConfig.bookmarkPersistenceType + "\". Cannot proceed" );
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    // Try to see if we can properly load the entire config before
    // applying it. If not stick with old config vs corrupt config
    CStoragePubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        MT::CScopeMutex lock( m_lock );

        m_config = parsedCfg;
        return ConfigureBookmarkPersistance();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg  )
{GUCEF_TRACE;

    // Try to see if we can properly load the entire config before
    // applying it. If not stick with old config vs corrupt config
    CStoragePubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        MT::CScopeMutex lock( m_lock );

        m_config = parsedCfg;
        m_needToTrackAcks = DetermineIfTrackingAcksIsNeeded();
        return ConfigureBookmarkPersistance();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::Disconnect( void )
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
CStoragePubSubClient::Connect( bool reset )
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
CStoragePubSubClient::IsConnected( void ) const
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
CStoragePubSubClient::IsHealthy( void ) const
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
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:IsHealthy: overall health is now Ok" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient:IsHealthy: overall health status is now unhealthy" );
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
CStoragePubSubClient::IsInitialized( void ) const
{GUCEF_TRACE;

    // the storage backend fully depends on the VFS without which we cannot do anything
    return VFS::CVfsGlobal::Instance()->GetVfs().IsInitialized();
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_metricsTimer )
    {
        TEventCallback callback( this, &CStoragePubSubClient::OnMetricsTimerCycle );
        SubscribeTo( m_metricsTimer                 ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    
    TEventCallback callback( this, &CStoragePubSubClient::OnVfsDirCreated );
    SubscribeTo( &vfs, VFS::CVFS::DirCreatedEvent, callback );
    
    TEventCallback callback2( this, &CStoragePubSubClient::OnVfsDirDeleted );
    SubscribeTo( &vfs, VFS::CVFS::DirDeletedEvent, callback2 );
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::RegisterTopicEventHandlers( CStoragePubSubClientTopicPtr& topic )
{GUCEF_TRACE;

    if ( !topic.IsNULL() )
    {
        TEventCallback callback( this, &CStoragePubSubClient::OnTopicHealthStatusChange );
        SubscribeTo( topic.GetPointerAlways()                           ,
                     CStoragePubSubClientTopic::HealthStatusChangeEvent ,
                     callback                                           );
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::OnTopicHealthStatusChange( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventId  ,
                                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // (Re)determine the aggregate health status
    IsHealthy();
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClient::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
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
        CStoragePubSubClientTopicPtr topic = (*i).second;
        const CStoragePubSubClientTopic::TopicMetrics& topicMetrics = topic->GetMetrics();
        const CORE::CString& topicName = topic->GetMetricFriendlyTopicName();
        const CStoragePubSubClientTopicConfig& topicConfig = topic->GetTopicConfig();
        CORE::CString metricsPrefix = m_config.metricsPrefix + topicName;

        GUCEF_METRIC_GAUGE( metricsPrefix + ".queuedReadyToReadBuffers", topicMetrics.queuedReadyToReadBuffers, 1.0f );
        GUCEF_METRIC_GAUGE( metricsPrefix + ".smallestBufferSizeInBytes", topicMetrics.smallestBufferSizeInBytes, 1.0f );
        GUCEF_METRIC_GAUGE( metricsPrefix + ".largestBufferSizeInBytes", topicMetrics.largestBufferSizeInBytes, 1.0f );

        if ( topicConfig.needSubscribeSupport )
        {
            GUCEF_METRIC_COUNT( metricsPrefix + ".storageCorruptionDetections", topicMetrics.storageCorruptionDetections, 1.0f );
            GUCEF_METRIC_COUNT( metricsPrefix + ".msgsLoadedFromStorage", topicMetrics.msgsLoadedFromStorage, 1.0f );
            GUCEF_METRIC_COUNT( metricsPrefix + ".msgsNotifiedAsReceived", topicMetrics.msgsNotifiedAsReceived, 1.0f );
            GUCEF_METRIC_COUNT( metricsPrefix + ".storageDeserializationFailures", topicMetrics.storageDeserializationFailures, 1.0f );
        }
        else
        if ( topicConfig.needPublishSupport )
        {
            GUCEF_METRIC_COUNT( metricsPrefix + ".msgsWrittenToStorage", topicMetrics.msgsWrittenToStorage, 1.0f );
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CStoragePubSubClient::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CStoragePubSubClient::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStoragePubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::STORAGE::CStoragePubSubClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClient::SetJournal( PUBSUB::CIPubSubJournalBasicPtr journal )
{GUCEF_TRACE;

    m_journal = journal;
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CIPubSubJournalBasicPtr 
CStoragePubSubClient::GetJournal( void ) const
{GUCEF_TRACE;

    return m_journal;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STORAGE */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
