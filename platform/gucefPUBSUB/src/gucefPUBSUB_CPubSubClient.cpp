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

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBJOURNALFACTORY_H
#include "gucefPUBSUB_CPubSubJournalFactory.h"
#define GUCEF_PUBSUB_CPUBSUBJOURNALFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBJOURNALFACTORY_H ? */

#include "gucefPUBSUB_CPubSubClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CPubSubClient::TopicAccessCreatedEvent = "GUCEF::PUBSUB::CPubSubClient::TopicAccessCreatedEvent";
const CORE::CEvent CPubSubClient::TopicAccessDestroyedEvent = "GUCEF::PUBSUB::CPubSubClient::TopicAccessDestroyedEvent";
const CORE::CEvent CPubSubClient::TopicsAccessAutoCreatedEvent = "GUCEF::PUBSUB::CPubSubClient::TopicsAccessAutoCreatedEvent";
const CORE::CEvent CPubSubClient::TopicsAccessAutoDestroyedEvent = "GUCEF::PUBSUB::CPubSubClient::TopicsAccessAutoDestroyedEvent";
const CORE::CEvent CPubSubClient::TopicDiscoveryEvent = "GUCEF::PUBSUB::CPubSubClient::TopicDiscoveryEvent";
const CORE::CEvent CPubSubClient::TopicDiscoveryEndedEvent = "GUCEF::PUBSUB::CPubSubClient::TopicDiscoveryEndedEvent";
const CORE::CEvent CPubSubClient::HealthStatusChangeEvent = "GUCEF::PUBSUB::CPubSubClient::HealthStatusChangeEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPubSubClient::CPubSubClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CTSGNotifier( pulseGenerator, true, false )
    , CORE::CIConfigurable()
    , m_opaqueUserData( GUCEF_NULL )
    , m_parentSide( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPubSubClient::CPubSubClient( void )
    : CORE::CTSGNotifier( CORE::PulseGeneratorPtr(), true, false )
    , CORE::CIConfigurable()
    , m_opaqueUserData( GUCEF_NULL )
    , m_parentSide( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPubSubClient::CPubSubClient( const CPubSubClient& src )
    : CORE::CTSGNotifier( src )
    , CORE::CIConfigurable( src )
    , m_opaqueUserData( src.m_opaqueUserData )
    , m_parentSide( src.m_parentSide )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClient::~CPubSubClient()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CPubSubClient::RegisterEvents( void )
{GUCEF_TRACE;

    TopicAccessCreatedEvent.Initialize();
    TopicAccessDestroyedEvent.Initialize();
    TopicsAccessAutoCreatedEvent.Initialize();
    TopicsAccessAutoDestroyedEvent.Initialize();
    TopicDiscoveryEvent.Initialize();
    TopicDiscoveryEndedEvent.Initialize();
    HealthStatusChangeEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPubSubClient&
CPubSubClient::operator=( const CPubSubClient& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( *this );
        CORE::CTSGNotifier::operator=( *this );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CString&
CPubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::PUBSUB::CPubSubClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::SetDefaultTopicPulseGenerator( CORE::PulseGeneratorPtr defaultPulseGenerator )
{GUCEF_TRACE;

    // default implementation is a no-op
    // The backend can implement this if relevant
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CPubSubClient::GetDefaultTopicPulseGenerator( void ) const
{GUCEF_TRACE;

    return CORE::PulseGeneratorPtr();
}

/*-------------------------------------------------------------------------*/

void
CPubSubClient::SetTopicPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    SetDefaultTopicPulseGenerator( newPulseGenerator );
    
    PubSubClientTopicSet allTopicAccess;
    GetAllCreatedTopicAccess( allTopicAccess );

    PubSubClientTopicSet::iterator i = allTopicAccess.begin();
    while ( i != allTopicAccess.end() )
    {
        CPubSubClientTopicBasicPtr topic = (*i);
        if ( !topic.IsNULL() )
            topic->SetPulseGenerator( newPulseGenerator );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
                                  bool includeTopics                        )
{GUCEF_TRACE;

    CORE::CTSGNotifier::SetPulseGenerator( newPulseGenerator );

    if ( includeTopics )
    {
        SetTopicPulseGenerator( newPulseGenerator );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    // note that this is overloading the version from a base class
    SetPulseGenerator( newPulseGenerator, true );
}

/*-------------------------------------------------------------------------*/

CPubSubClientTopicBasicPtr
CPubSubClient::GetOrCreateTopicAccess( const CString& topicName               ,
                                       CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    CPubSubClientTopicBasicPtr topicAccess = GetTopicAccess( topicName );
    if ( topicAccess.IsNULL() )
    {
        topicAccess = CreateTopicAccess( topicName, pulseGenerator );
    }
    return topicAccess;
}

/*-------------------------------------------------------------------------*/

CPubSubClientTopicBasicPtr
CPubSubClient::CreateTopicAccess( const CString& topicName               ,
                                  CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    CPubSubClientTopicConfigPtr topicConfig = GetOrCreateTopicConfig( topicName );
    if ( !topicConfig.IsNULL() )
    {
        CPubSubClientTopicBasicPtr topicAccess = CreateTopicAccess( topicConfig, pulseGenerator );
        return topicAccess;
    }
    return CPubSubClientTopicPtr();
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::GetMultiTopicAccess( CPubSubClientTopicConfigPtr topicConfig ,
                                    PubSubClientTopicSet& topicAccess       )
{GUCEF_TRACE;

    if ( topicConfig.IsNULL() )
        return false;
    
    return GetMultiTopicAccess( topicConfig->topicName, topicAccess );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::GetMultiTopicAccess( const CString& topicName          ,
                                    PubSubClientTopicSet& topicAccess )
{GUCEF_TRACE;

    // The default implementation here assumes no 1:N pattern matching access is supported
    // As such it redirects to the basic GetTopicAccess()
    // Backends should override this if they support pattern matching access

    CPubSubClientTopicBasicPtr tAccess = GetTopicAccess( topicName );
    if ( !tAccess.IsNULL() )
    {
        topicAccess.insert( tAccess );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::GetMultiTopicAccess( const CString::StringSet& topicNames ,
                                    PubSubClientTopicSet& topicAccess    )
{GUCEF_TRACE;

    bool totalSuccess = true;
    CString::StringSet::const_iterator i = topicNames.begin();
    while ( i != topicNames.end() )
    {
        totalSuccess = GetMultiTopicAccess( (*i), topicAccess ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::GetOrCreateMultiTopicAccess( CPubSubClientTopicConfigPtr topicConfig ,
                                            PubSubClientTopicSet& topicAccess       ,
                                            CORE::PulseGeneratorPtr pulseGenerator  )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    // In the multi-topic scenario we'd expect the topic name to be a pattern to match
    // As such the config applies to everything matching the pattern
    // This also means there is no way of knowing if we obtained the 'correct' nr of topics here, just something vs nothing.
    if ( !GetMultiTopicAccess( topicConfig, topicAccess ) || topicAccess.empty()  )
    {
        return CreateMultiTopicAccess( topicConfig, topicAccess, pulseGenerator );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::GetOrCreateMultiTopicAccess( const CString& topicName               ,
                                            PubSubClientTopicSet& topicAccess      ,
                                            CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    // In the multi-topic scenario we'd expect the topic name to be a pattern to match
    // As such the config applies to everything matching the pattern
    // This also means there is no way of knowing if we obtained the 'correct' nr of topics here, just something vs nothing.
    if ( !GetMultiTopicAccess( topicName, topicAccess ) || topicAccess.empty()  )
    {
        return CreateMultiTopicAccess( topicName, topicAccess );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::CreateMultiTopicAccess( CPubSubClientTopicConfigPtr topicConfig ,
                                       PubSubClientTopicSet& topicAccess       ,
                                       CORE::PulseGeneratorPtr pulseGenerator  )
{GUCEF_TRACE;

    // The default implementation here assumes no 1:N pattern matching access is supported
    // As such it redirects to the basic CreateTopicAccess()
    // Backends should override this if they support pattern matching access

    CPubSubClientTopicBasicPtr tAccess = CreateTopicAccess( topicConfig, pulseGenerator );
    if ( !tAccess.IsNULL() )
    {
        topicAccess.insert( tAccess );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::CreateMultiTopicAccess( const CString& topicName               ,
                                       PubSubClientTopicSet& topicAccess      ,
                                       CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    // In the multi-topic scenario we'd expect the topic name to be a pattern to match
    // As such the config applies to everything matching the pattern
    CPubSubClientTopicConfigPtr topicConfig = GetTopicConfig( topicName );
    if ( topicConfig.IsNULL() )
    {
        CPubSubClientTopicConfigPtr defaultTopicConfig = GetDefaultTopicConfig();
        if ( !defaultTopicConfig.IsNULL() )
        {
            topicConfig = CPubSubClientTopicConfigPtr( static_cast< CPubSubClientTopicConfig* >( defaultTopicConfig->Clone() ) );
            if ( !topicConfig.IsNULL() )
            {
                // overwrite the template's topic name, which could be anything, with the topic name given
                topicConfig->topicName = topicName;
            }
        }
    }
    if ( !topicConfig.IsNULL() )
    {
        return CreateMultiTopicAccess( topicConfig, topicAccess, pulseGenerator );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::BeginTopicDiscovery( const CORE::CString::StringSet& globPatternFilters )
{GUCEF_TRACE;

    // By default not supported
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClient::SetOpaqueUserData( void* opaqueUserData )
{GUCEF_TRACE;

   MT::CObjectScopeLock lock( this );
   m_opaqueUserData = opaqueUserData;
}

/*-------------------------------------------------------------------------*/

void*
CPubSubClient::GetOpaqueUserData( void ) const
{GUCEF_TRACE;

    return m_opaqueUserData;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClient::SetParentSide( CPubSubClientSide* parentSide )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_parentSide = parentSide;
}

/*-------------------------------------------------------------------------*/

CPubSubClientSide*
CPubSubClient::GetParentSide( void ) const
{GUCEF_TRACE;

    return m_parentSide;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess ) const
{GUCEF_TRACE;

    // Since this is just a const correctness helper function we can abuse the non-const version
    // for efficiency the backend should really implement its own version if supported.
    CPubSubClient* thisObj = const_cast< CPubSubClient* >( this );

    PubSubClientTopicSet topics;
    thisObj->GetAllCreatedTopicAccess( topics );

    PubSubClientTopicSet::iterator i = topics.begin();
    while ( i != topics.end() )
    {
        topicAccess.insert( (*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::AreAllSubscriptionsAtEndOfData( void ) const
{GUCEF_TRACE;

    // First get a list of all available topics from the backend
    // for efficiency the backend should really implement its own version if supported as we need to pull a list copy
    // the backend likely already has said list
    PubSubClientTopicSet topics;
    GetAllCreatedTopicAccess( topics );

    if ( !topics.empty() )
    {
        bool allAtEnd = true;

        PubSubClientTopicSet::iterator i = topics.begin();
        while ( i != topics.end() )
        {
            if ( !(*i)->IsSubscriptionAtEndOfData() )
            {
                allAtEnd = false;
                break;
            }
            ++i;
        }

        return allAtEnd;
    }

    // if there are no topics you also cannot have any data
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    CPubSubClientConfig parsedGenericCfg;
    if ( parsedGenericCfg.SaveConfig( cfg ) )
    {
        return SaveConfig( parsedGenericCfg );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    CPubSubClientConfig parsedGenericCfg;
    if ( parsedGenericCfg.LoadConfig( cfg ) )
    {
        return LoadConfig( parsedGenericCfg );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::SetJournal( CIPubSubJournalBasicPtr journal )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CIPubSubJournalBasicPtr 
CPubSubClient::GetJournal( void ) const
{GUCEF_TRACE;

    return CIPubSubJournalBasicPtr();
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubClient::ConfigureJournal( CPubSubClientConfig& clientConfig )
{GUCEF_TRACE;
    
    if ( clientConfig.journalConfig.useJournal )
    {
        PUBSUB::CPubSubJournalConfig& journalConfig = clientConfig.journalConfig;
        
        // resolve variables/macros in the journal resource if any exist
        journalConfig.journalResource.ReplaceSubstr( "{clientType}", GetType() );
        journalConfig.journalResource.ReplaceSubstr( "{clientInstance}", CORE::ToString( this ) );
        
        PUBSUB::CIPubSubJournalBasicPtr journal = PUBSUB::CPubSubGlobal::Instance()->GetPubSubJournalFactory().Create( journalConfig.journalType, journalConfig );
        if ( !journal.IsNULL() )
        {        
            return SetJournal( journal );
        }
        else
        {
            return false;
        }
    } 
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubClient::ConfigureJournal( CPubSubClientConfigPtr clientConfig )
{GUCEF_TRACE;

    if ( !clientConfig.IsNULL() )
    {
        return ConfigureJournal( *clientConfig.GetPointerAlways() );
    } 
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClient::ConfigureJournal( CPubSubClientTopicBasicPtr topic        ,
                                 CPubSubClientTopicConfigPtr topicConfig )
{GUCEF_TRACE;

    if ( !topicConfig.IsNULL() && !topic.IsNULL() && topicConfig->journalConfig.useJournal )
    {
        PUBSUB::CPubSubJournalConfig& journalConfig = topicConfig->journalConfig;
        
        // resolve variables/macros in the journal resource if any exist        
        journalConfig.journalResource.ReplaceSubstr( "{clientType}", GetType() );
        journalConfig.journalResource.ReplaceSubstr( "{clientInstance}", CORE::ToString( this ) );
        journalConfig.journalResource.ReplaceSubstr( "{topicName}", topicConfig->topicName );
        journalConfig.journalResource.ReplaceSubstr( "{topicInstance}", CORE::ToString( topic.GetPointerAlways() ) );
        
        PUBSUB::CIPubSubJournalBasicPtr journal = PUBSUB::CPubSubGlobal::Instance()->GetPubSubJournalFactory().Create( journalConfig.journalType, journalConfig );
        if ( !journal.IsNULL() )
        {        
            return topic->SetJournal( journal );
        }
        else
        {
            return false;
        }
    } 
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubClient::IsConnectedAndSubscribedAsNeeded( void ) const
{GUCEF_TRACE;

    if ( IsConnected() )
    {
        PubSubClientTopicSet topics;
        GetAllCreatedTopicAccess( topics );

        if ( !topics.empty() )
        {
            PubSubClientTopicSet::iterator i = topics.begin();
            while ( i != topics.end() )
            {
                const CPubSubClientTopicBasicPtr& topic = (*i);
                if ( topic->IsSubscribingSupported() && !topic->IsSubscribed() )
                {
                    // Found a topic which supports subscribing but it isnt currently subscribed
                    return false;
                }
                ++i;
            }
        }

        return true;
    } 
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
