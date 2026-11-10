/*
 *  pubsubpluginAWSSNS: Generic GUCEF COMCORE plugin for providing pubsub via AWS's SNS
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

#include <string.h>


#include <aws/core/Aws.h>
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <iostream>

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

#ifndef PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENT_H
#include "pubsubpluginAWSSNS_CAwsSnsPubSubClient.h"
#define PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENT_H ? */

#include "pubsubpluginAWSSNS_CAwsSnsPubSubClientTopic.h"


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace AWSSNS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define SNSCLIENT_MAX_PAYLOAD_SIZE                  ( 256 * 1024 ) // 256KB
#define SNSCLIENT_MAX_NR_OF_BATCH_ENTRIES           10             // Per the AWS API a batch can have up to 10 entries

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopic::CAwsSnsPubSubClientTopic( CAwsSnsPubSubClient* client )
    : PUBSUB::CPubSubClientTopic( client->GetPulseGenerator() )
    , CORE::CTSharedObjCreator< CAwsSnsPubSubClientTopic, MT::CMutex >( this )
    , m_client( client )
    , m_config()
    , m_lock()
    , m_topicArn()
    , m_snsMaximumMessageSize( SNSCLIENT_MAX_PAYLOAD_SIZE )
    , m_metrics()
    , m_publishBulkMsgRemapStorage()
    , m_currentPublishActionId( 1 )
    , m_publishSuccessActionIds()
    , m_publishSuccessActionEventData()
    , m_publishFailureActionIds()
    , m_publishFailureActionEventData()
{GUCEF_TRACE;

    m_publishSuccessActionEventData.LinkTo( &m_publishSuccessActionIds );
    m_publishFailureActionEventData.LinkTo( &m_publishFailureActionIds );

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopic::~CAwsSnsPubSubClientTopic()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CAwsSnsPubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_client = GUCEF_NULL;
    Disconnect();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CAwsSnsPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

void
CAwsSnsPubSubClientTopic::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
    {
        TEventCallback callback( this, &CAwsSnsPubSubClientTopic::OnPulseCycle );
        SubscribeTo( m_client->GetConfig().pulseGenerator.GetPointerAlways() ,
                     CORE::CPulseGenerator::PulseEvent                       ,
                     callback                                                );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;

    // SNS pushes messages to its subscribers, a client cannot pull messages from a topic
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAwsSnsPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.topicName;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::ApplySnsMessageAttributeNameContraints( CORE::CAsciiString& candidateName )
{GUCEF_TRACE;

    // From the AWS documentation:
    //          Name – The message attribute name can contain the following characters: A-Z, a-z, 0-9, underscore (_), hyphen (-), and period (.). The following restrictions apply:
    //              - Can be up to 256 characters long
    //              - Can't start with AWS. or Amazon. (or any casing variations)
    //              - Is case-sensitive
    //              - Must be unique among all attribute names for the message
    //              - Must not start or end with a period
    //              - Must not have periods in a sequence
    
    bool wasAdjusted = false;

    if ( !candidateName.IsNULLOrEmpty() )
    {
        // Take care of this requirement: 'Can't start with AWS. or Amazon.'
        CORE::CAsciiString lcTestString = candidateName.Lowercase();
        if ( 0 == lcTestString.HasSubstr( "aws" ) )
        {
            candidateName = candidateName.CutChars( 3, true );    
            wasAdjusted = true;
        }
        else
        if ( 0 == lcTestString.HasSubstr( "amazon" ) )
        {
            candidateName = candidateName.CutChars( 6, true );    
            wasAdjusted = true;
        }
        
        if ( !candidateName.IsNULLOrEmpty() )
        {
            // Take care of this requirement: 'Must not start or end with a period'
            if ( '.' == candidateName[ 0 ] )
            {
                candidateName[ 0 ] = '_';    
                wasAdjusted = true;
            }
            if ( '.' == candidateName[ candidateName.Length()-1 ] )
            {
                candidateName[ candidateName.Length()-1 ] = '_';
                wasAdjusted = true;
            }

            // Take care of this requirement: 'Must not have periods in a sequence'
            if ( candidateName.HasRepeatingChar( '.' ) )
            {
                candidateName = candidateName.CompactRepeatingChar( '.' );
                wasAdjusted = true;
            }

            // Take care of this requirement: 'Can be up to 256 characters long'
            if ( candidateName.Length() > 256 )
            {
                candidateName = candidateName.CutChars( candidateName.Length() - 256, false );
                wasAdjusted = true;
            }
        }
    }

    return wasAdjusted;
}

/*-------------------------------------------------------------------------*/

template< class T >
bool
CAwsSnsPubSubClientTopic::AddAttributesToSnsMsg( T& snsMsg                                          , 
                                                 const PUBSUB::CIPubSubMsg::TKeyValuePairs& kvPairs ,
                                                 bool addPrefix                                     ,
                                                 const CORE::CAsciiString& prefixToAdd              ,
                                                 CORE::UInt32& msgByteSize                          )
{GUCEF_TRACE;

    if ( !kvPairs.empty() )
    {        
        typedef Aws::Map< Aws::String, Aws::SNS::Model::MessageAttributeValue > TSnsMsgAttributeMap;

        TSnsMsgAttributeMap attribs;
        PUBSUB::CIPubSubMsg::TKeyValuePairs::const_iterator n = kvPairs.begin();
        while ( n != kvPairs.end() )
        {
            const PUBSUB::CIPubSubMsg::TKeyValuePair& kvPair = (*n);
            const CORE::CVariant& kvKey = kvPair.first;
            const CORE::CVariant& kvValue = kvPair.second;

            if ( kvKey.IsInitialized() && kvValue.IsInitialized() )
            {                    
                Aws::SNS::Model::MessageAttributeValue snsAttribValue;
                bool valueHasBeenSetOk = false;

                if ( kvValue.IsString() )
                {
                    CORE::CUtf8String valueStr = kvValue.AsUtf8String();
                    if ( !valueStr.IsNULLOrEmpty() )
                    {
                        snsAttribValue.SetDataType( "String" );
                        msgByteSize += 6;                 
                        snsAttribValue.SetStringValue( valueStr );
                        msgByteSize += (valueStr.ByteSize()-1);
                        valueHasBeenSetOk = true;
                    }
                }
                else
                if ( kvValue.IsNumber() )
                {
                    CORE::CUtf8String valueStr = kvValue.AsUtf8String();
                    if ( !valueStr.IsNULLOrEmpty() )
                    {
                        snsAttribValue.SetDataType( "Number" );
                        msgByteSize += 6;                 
                        snsAttribValue.SetStringValue( valueStr );
                        msgByteSize += (valueStr.ByteSize()-1);
                        valueHasBeenSetOk = true;
                    }
                }
                else // kvValue.IsBinary() ? catch-all
                {
                    if ( kvValue.ByteSize() > 0 )
                    {
                        snsAttribValue.SetDataType( "Binary" );
                        msgByteSize += 6;
                        snsAttribValue.SetBinaryValue( Aws::Utils::ByteBuffer( static_cast< const unsigned char* >( kvValue.AsVoidPtr() ), kvValue.ByteSize() ) );
                        msgByteSize += kvValue.ByteSize();
                        valueHasBeenSetOk = true;
                    }
                }
                                    
                if ( valueHasBeenSetOk )
                {
                    CORE::CAsciiString kvKeyName = kvKey.AsAsciiString();
                    if ( addPrefix )
                    {
                        kvKeyName = prefixToAdd + kvKeyName;
                    }
                    ApplySnsMessageAttributeNameContraints( kvKeyName );
                    if ( !kvKeyName.IsNULLOrEmpty() )
                    {
                        snsMsg.AddMessageAttributes( kvKeyName, snsAttribValue ); // if there are dupicate keys last one wins                    
                        msgByteSize += (kvKeyName.ByteSize()-1);
                    }
                }
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:AddAttributesToSnsMsg: Uninitialized key and/or value set on msg, ignoring" );
            }

            ++n;
        }    
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

template< class T >
bool
CAwsSnsPubSubClientTopic::TranslateToSnsMsgOfType( T& snsMsg                      , 
                                                   const PUBSUB::CIPubSubMsg* msg , 
                                                   CORE::UInt32& msgByteSize      )
{GUCEF_TRACE;

    if ( GUCEF_NULL == msg )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:TranslateToSnsMsgOfType: NULL Message passed" );
        return false;
    }

    // A message can include only XML, JSON, and unformatted text. The following Unicode characters are allowed:
    // #x9 | #xA | #xD | #x20 to #xD7FF | #xE000 to #xFFFD | #x10000 to #x10FFFF
    // The minimum size is one character. The maximum size is 256 KB, which includes the message attributes

    const CORE::CVariant& bodyPayload = msg->GetPrimaryPayload();
    if ( bodyPayload.IsInitialized() )
    {            
        // We request the payload as a string. Note that this auto converts
        // Binary is Base64 encoded. For SNS strings are Unicode with UTF-8 binary encoding
        CORE::CUtf8String bodyPayloadStr = bodyPayload.AsUtf8String();
            
        if ( bodyPayloadStr.ByteSize() >= 1 && bodyPayloadStr.ByteSize() <= m_snsMaximumMessageSize )
        {
            snsMsg.SetMessage( bodyPayloadStr );
            msgByteSize += (bodyPayloadStr.ByteSize()-1);
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:TranslateToSnsMsgOfType: Message body size as string has an invalid size. Must be between 1-256KB. Cannot translate" );
            return false;
        }
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:TranslateToSnsMsgOfType: Message does not have a body which SNS does not allow. Cannot publish" );
        return false;
    }

    bool totalSuccess = true;
    const PUBSUB::CIPubSubMsg::TKeyValuePairs& kvPairs = msg->GetKeyValuePairs();
    totalSuccess = AddAttributesToSnsMsg( snsMsg, kvPairs, m_config.addPrefixWhenSendingKvPairs, m_config.kvPairPrefixToAddOnSend, msgByteSize ) && totalSuccess;
    const PUBSUB::CIPubSubMsg::TKeyValuePairs& metaKvPairs = msg->GetMetaDataKeyValuePairs();
    totalSuccess = AddAttributesToSnsMsg( snsMsg, metaKvPairs, m_config.addPrefixWhenSendingMetaDataKvPairs, m_config.metaDatakvPairPrefixToAddOnSend, msgByteSize ) && totalSuccess;

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::TranslateToSnsBatchMsg( Aws::SNS::Model::PublishBatchRequestEntry& snsMsg ,
                                                  const PUBSUB::CIPubSubMsg* msg                    ,
                                                  CORE::UInt64 publishActionId                      ,
                                                  CORE::UInt32& approxMsgByteSize                   )
{GUCEF_TRACE;

    // Specific to batch messages: Its mandatory to set an ID on each entry in the batch.
    // This is done to be able to provide a response to the API call that has individual success/fail states per entry
    CORE::CString IdStr = CORE::ToString( publishActionId );
    snsMsg.SetId( IdStr );
    approxMsgByteSize += (IdStr.ByteSize()-1);

    // Now do the translation work which is generic and shared between batch and non-batch messages
    return TranslateToSnsMsgOfType< Aws::SNS::Model::PublishBatchRequestEntry >( snsMsg, msg, approxMsgByteSize );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::TranslateToSnsMsg( Aws::SNS::Model::PublishRequest& snsMsg ,
                                             const PUBSUB::CIPubSubMsg* msg          ,
                                             CORE::UInt32& approxMsgByteSize         )
{GUCEF_TRACE;

    approxMsgByteSize += (CORE::UInt32) m_topicArn.size();

    // Now do the translation work which is generic and shared between batch and non-batch messages
    return TranslateToSnsMsgOfType< Aws::SNS::Model::PublishRequest >( snsMsg, msg, approxMsgByteSize );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify )
{GUCEF_TRACE;

    bool success = false;

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
        return false;

    if ( 0 == publishActionId )
    {
        publishActionId = m_currentPublishActionId;
        ++m_currentPublishActionId;
    }

    try
    {
        Aws::SNS::Model::PublishRequest publishRequest;
        publishRequest.SetTopicArn( m_topicArn );

        CORE::UInt32 msgByteSize = 0;
        if ( TranslateToSnsMsg( publishRequest, &msg, msgByteSize ) )
        {
            if ( msgByteSize <= m_snsMaximumMessageSize )
            {
                Aws::SNS::Model::PublishOutcome publishOutcome = m_client->GetSnsClient().Publish( publishRequest );
                if ( publishOutcome.IsSuccess() )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:Publish: Success publishing message to \"" + CORE::ToString( m_topicArn ) + "\"" );
                    ++m_metrics.snsMessagesPublished;
                    success = true;
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:Publish: Error publishing message to \"" + CORE::ToString( m_topicArn ) + "\". Error Msg: " + publishOutcome.GetError().GetMessage() );
                    ++m_metrics.snsPublishErrors;
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:Publish: Message too large with size of " + CORE::ToString( msgByteSize ) + " to \"" + CORE::ToString( m_topicArn ) + "\"" );
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, CORE::CString( "AwsSnsPubSubClientTopic:Publish: experienced an exception: " ) + e.what() );
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "AwsSnsPubSubClientTopic:Publish: experienced an unknown exception, your application may be unstable" );
    }

    if ( notify )
    {
        if ( success )
            m_publishSuccessActionIds.push_back( publishActionId );
        else
            m_publishFailureActionIds.push_back( publishActionId );
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds, const PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector& msgs, bool notify )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_publishBulkMsgRemapStorage.clear();
    m_publishBulkMsgRemapStorage.reserve( msgs.size() );
    PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector::const_iterator i = msgs.begin();
    while ( i != msgs.end() )
    {
        m_publishBulkMsgRemapStorage.push_back( &(*i) );
        ++i;
    }
    return Publish( publishActionIds, m_publishBulkMsgRemapStorage, notify );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds, const PUBSUB::CIPubSubMsg::TIPubSubMsgSPtrVector& msgs, bool notify )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_publishBulkMsgRemapStorage.clear();
    m_publishBulkMsgRemapStorage.reserve( msgs.size() );
    PUBSUB::CIPubSubMsg::TIPubSubMsgSPtrVector::const_iterator i = msgs.begin();
    while ( i != msgs.end() )
    {
        m_publishBulkMsgRemapStorage.push_back( (*i).GetPointerAlways() );
        ++i;
    }
    return Publish( publishActionIds, m_publishBulkMsgRemapStorage, notify );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds                      ,
                                   const PUBSUB::CIPubSubMsg::TIPubSubMsgConstRawPtrVector& msgs ,
                                   bool notify                                                   )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
        return false;

    // Every message gets a publish action id, those that were given are kept, the others are generated
    size_t preExistingActionIds = publishActionIds.size();
    for ( size_t n=0; n<msgs.size(); ++n )
    {
        if ( n < preExistingActionIds )
        {
            if ( 0 == publishActionIds[ n ] )
                publishActionIds[ n ] = m_currentPublishActionId++;
        }
        else
        {
            publishActionIds.push_back( m_currentPublishActionId++ );
        }
    }

    bool totalSuccess = true;
    try
    {
        if ( !m_config.tryToUsePublishBatch )
        {
            for ( size_t n=0; n<msgs.size(); ++n )
            {
                CORE::UInt64 publishActionId = publishActionIds[ n ];
                totalSuccess = Publish( publishActionId, *msgs[ n ], notify ) && totalSuccess;
            }
            return totalSuccess;
        }

        size_t n = 0;
        while ( n < msgs.size() )
        {
            // Fill up a batch, within the limits of the API: a number of entries and a total size
            Aws::SNS::Model::PublishBatchRequest batchRequest;
            batchRequest.SetTopicArn( m_topicArn );

            typedef std::vector< CORE::UInt64 > TActionIdVector;
            TActionIdVector actionIdsInBatch;
            CORE::UInt32 batchByteSize = 0;

            while ( n < msgs.size() && actionIdsInBatch.size() < SNSCLIENT_MAX_NR_OF_BATCH_ENTRIES )
            {
                CORE::UInt64 publishActionId = publishActionIds[ n ];
                CORE::UInt32 entryByteSize = 0;
                Aws::SNS::Model::PublishBatchRequestEntry entry;
                if ( !TranslateToSnsBatchMsg( entry, msgs[ n ], publishActionId, entryByteSize ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:Publish: Failed to translate message to something SNS compatible. publishActionId=" + CORE::ToString( publishActionId ) );
                    m_publishFailureActionIds.push_back( publishActionId );
                    totalSuccess = false;
                    ++n;
                    continue;
                }

                if ( batchByteSize + entryByteSize > m_snsMaximumMessageSize )
                {
                    // The message does not fit in this batch. If it is the only one it is too large to ever be sent
                    if ( actionIdsInBatch.empty() )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:Publish: Message too large with size of " + CORE::ToString( entryByteSize ) + " to \"" + CORE::ToString( m_topicArn ) + "\"" );
                        m_publishFailureActionIds.push_back( publishActionId );
                        totalSuccess = false;
                        ++n;
                        continue;
                    }
                    break;
                }

                batchRequest.AddPublishBatchRequestEntries( entry );
                actionIdsInBatch.push_back( publishActionId );
                batchByteSize += entryByteSize;
                ++n;
            }

            if ( actionIdsInBatch.empty() )
                continue;

            Aws::SNS::Model::PublishBatchOutcome batchOutcome = m_client->GetSnsClient().PublishBatch( batchRequest );
            if ( batchOutcome.IsSuccess() )
            {
                const Aws::Vector< Aws::SNS::Model::BatchResultErrorEntry >& failedEntries = batchOutcome.GetResult().GetFailed();
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:Publish: Performed batch operation for messages to \"" + CORE::ToString( m_topicArn ) +
                    "\" with " + CORE::ToString( failedEntries.size() ) + " entries failed out of " + CORE::ToString( actionIdsInBatch.size() ) );

                Aws::Vector< Aws::SNS::Model::BatchResultErrorEntry >::const_iterator f = failedEntries.begin();
                while ( f != failedEntries.end() )
                {
                    CORE::UInt64 failedActionId = CORE::StringToUInt64( (*f).GetId(), GUCEF_UINT64MAX );
                    if ( GUCEF_UINT64MAX == failedActionId )
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:Publish: Likely issue interpreting the 'Id' of a failed batch entry error result: " + CORE::ToString( (*f).GetId() ) );
                    m_publishFailureActionIds.push_back( failedActionId );
                    ++m_metrics.snsPublishErrors;
                    totalSuccess = false;
                    ++f;
                }

                const Aws::Vector< Aws::SNS::Model::PublishBatchResultEntry >& successEntries = batchOutcome.GetResult().GetSuccessful();
                Aws::Vector< Aws::SNS::Model::PublishBatchResultEntry >::const_iterator s2 = successEntries.begin();
                while ( s2 != successEntries.end() )
                {
                    CORE::UInt64 successActionId = CORE::StringToUInt64( (*s2).GetId(), GUCEF_UINT64MAX );
                    if ( GUCEF_UINT64MAX == successActionId )
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:Publish: Likely issue interpreting the 'Id' of a batch entry success result: " + CORE::ToString( (*s2).GetId() ) );
                    m_publishSuccessActionIds.push_back( successActionId );
                    ++m_metrics.snsMessagesPublished;
                    ++s2;
                }
            }
            else
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:Publish: Error sending batch message to \"" + CORE::ToString( m_topicArn ) + "\". Error Msg: " + batchOutcome.GetError().GetMessage() );
                m_metrics.snsPublishErrors += (CORE::UInt32) actionIdsInBatch.size();
                for ( size_t a=0; a<actionIdsInBatch.size(); ++a )
                    m_publishFailureActionIds.push_back( actionIdsInBatch[ a ] );
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, CORE::CString( "AwsSnsPubSubClientTopic:Publish: experienced an exception: " ) + e.what() );
        totalSuccess = false;
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "AwsSnsPubSubClientTopic:Publish: experienced an unknown exception, your application may be unstable" );
        totalSuccess = false;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::AcknowledgeReceipt( const CORE::CVariant& receipt )
{GUCEF_TRACE;

    // Messages cannot be received from SNS so there is nothing to acknowledge
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CAwsSnsPubSubClientTopic::OnPulseCycle( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_publishSuccessActionIds.empty() )
    {
        if ( !NotifyObservers( MsgsPublishedEvent, &m_publishSuccessActionEventData ) ) return;
        m_publishSuccessActionIds.clear();
    }

    if ( !m_publishFailureActionIds.empty() )
    {
        if ( !NotifyObservers( MsgsPublishFailureEvent, &m_publishFailureActionEventData ) ) return;
        m_publishFailureActionIds.clear();
    }
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    config = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_config = config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_topicArn.clear();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;

    // Not supported
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    // Not supported
    return false;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubBookmark
CAwsSnsPubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;

    // Not supported
    return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_APPLICABLE );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;

    // SNS is accessed through a web API, the topic is usable once we know its ARN
    MT::CScopeMutex lock( m_lock );
    return !m_topicArn.empty();
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopic::InitializeConnectivity( bool reset )
{GUCEF_TRACE;

    if ( reset )
    {
        if ( !Disconnect() )
            return false;
    }

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL != m_client )
    {
        m_topicArn = m_client->GetSnsTopicArnForTopicName( m_config.topicName, m_config.createTopicIfMissing );
        if ( !m_topicArn.empty() )
        {
            CORE::CString::StringMap topicAttributes;
            if ( m_client->TryGetTopicAttributes( m_topicArn, topicAttributes ) )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:InitializeConnectivity: Retrieved attributes for topic \"" + CORE::ToString( m_topicArn ) +
                    "\" which are as follows: " + CORE::ToString( topicAttributes ) );
            }
            return true;
        }
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClientTopic:InitializeConnectivity: No topic ARN could be determined for \"" + m_config.topicName + "\" thus cannot use the SNS topic" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopic::TopicMetrics::TopicMetrics( void )
    : snsMessagesPublished( 0 )
    , snsPublishErrors( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAwsSnsPubSubClientTopic::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CAwsSnsPubSubClientTopic::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CAwsSnsPubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CAwsSnsPubSubClientTopic::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAwsSnsPubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::AWSSNS::CAwsSnsPubSubClientTopic";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSSNS */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
