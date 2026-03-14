/*
 *  pubsubpluginAWSSQS: Generic GUCEF COMCORE plugin for providing pubsub via AWS's SQS
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

// Windoze has a macro for SendMessage which messes up our API calls
#undef SendMessage

#include <aws/core/Aws.h>
#include <aws/sqs/SQSClient.h>
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

#ifndef PUBSUBPLUGIN_AWSSQS_CAWSSQSPUBSUBCLIENT_H
#include "pubsubpluginAWSSQS_CAwsSqsPubSubClient.h"
#define PUBSUBPLUGIN_AWSSQS_CAWSSQSPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_AWSSQS_CAWSSQSPUBSUBCLIENT_H ? */

#include "pubsubpluginAWSSQS_CAwsSqsPubSubClientTopic.h"

// Windoze has a macro for SendMessage which messes up our API calls
#undef SendMessage

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace AWSSQS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define SQSCLIENT_MAX_PAYLOAD_SIZE                  ( 256 * 1024 ) // 256KB

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAwsSqsPubSubClientTopic::CAwsSqsPubSubClientTopic( CAwsSqsPubSubClient* client )
    : PUBSUB::CPubSubClientTopic( client->GetPulseGenerator() )
    , CORE::CTSharedObjCreator< CAwsSqsPubSubClientTopic, MT::CMutex >( this )
    , m_client( client )
    , m_redisShardHost()
    , m_config()
    , m_lock()
    , m_queueUrl()
    , m_sqsMaximumMessageSize( SQSCLIENT_MAX_PAYLOAD_SIZE )
    , m_publishBulkMsgRemapStorage()
    , m_currentPublishActionId( 1 )
    , m_currentReceiveActionId( 1 )
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

CAwsSqsPubSubClientTopic::~CAwsSqsPubSubClientTopic()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    m_client = GUCEF_NULL;
    Disconnect();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CAwsSqsPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClientTopic::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
    {
        TEventCallback callback( this, &CAwsSqsPubSubClientTopic::OnPulseCycle );
        SubscribeTo( m_client->GetConfig().pulseGenerator.GetPointerAlways() ,
                     CORE::CPulseGenerator::PulseEvent                       ,
                     callback                                                );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;
    
    return m_config.needPublishSupport;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CAwsSqsPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.topicName;
}

/*-------------------------------------------------------------------------*/

Aws::String
CAwsSqsPubSubClientTopic::GetSqsQueueUrlForQueueName( const CORE::CString& queueName )
{GUCEF_TRACE;

    try
    {
        Aws::SQS::Model::GetQueueUrlRequest gqu_req;
        gqu_req.SetQueueName( queueName );

        Aws::SQS::Model::GetQueueUrlOutcome gqu_out = m_client->GetSqsClient().GetQueueUrl( gqu_req );
        if ( gqu_out.IsSuccess() ) 
        {
            const Aws::String& queueUrl = gqu_out.GetResult().GetQueueUrl();

            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:GetSqsQueueUrlForQueueName: Resolved queue name \"" + queueName + "\" to URL: " + CORE::ToString( queueUrl ) );
            return queueUrl;
        } 
        else 
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:GetSqsQueueUrlForQueueName: Error getting URL for queue name \"" + queueName + "\":" + CORE::ToString( gqu_out.GetError().GetMessage() ) );
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, CORE::CString( "AwsSqsPubSubClientTopic:GetSqsQueueUrlForQueueName: experienced an exception: " ) + e.what() );
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "AwsSqsPubSubClientTopic:GetSqsQueueUrlForQueueName: experienced an unknown exception, your application may be unstable" );
    }

    return Aws::String();
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClientTopic::Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify )
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
        Aws::SQS::Model::SendMessageRequest sm_req;
        sm_req.SetQueueUrl( m_queueUrl );
               
        CORE::UInt32 msgByteSize = 0;                     
        if ( TranslateToSqsMsg( sm_req, &msg, msgByteSize ) )
        {
            if ( msgByteSize <= m_sqsMaximumMessageSize )
            {
                Aws::SQS::Model::SendMessageOutcome sm_out = m_client->GetSqsClient().SendMessage( sm_req );
                if ( sm_out.IsSuccess() )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Success sending message to \"" + m_queueUrl + "\"" );
                    success = true;
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Error sending message to \"" + m_queueUrl + "\". Error Msg: " + sm_out.GetError().GetMessage() );
                } 
            }
            else
            {
                GUCEF_ERROR_LOG(CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Message too large with size of " + CORE::ToString( msgByteSize ) + " to \"" + m_queueUrl + "\"");
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, CORE::CString( "AwsSqsPubSubClientTopic:Publish: experienced an exception: " ) + e.what() );
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "AwsSqsPubSubClientTopic:Publish: experienced an unknown exception, your application may be unstable" );
    }
    
    if ( notify )
    {
        if ( success )
        {           
            m_publishSuccessActionIds.push_back( publishActionId );
        }
        else
        {
            m_publishFailureActionIds.push_back( publishActionId );
        }
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds, const PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector& msgs, bool notify )
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
CAwsSqsPubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds, const PUBSUB::CIPubSubMsg::TIPubSubMsgSPtrVector& msgs, bool notify )
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
CAwsSqsPubSubClientTopic::AcknowledgeReceipt( const CORE::CVariant& receipt )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
        return false;
    
    bool totalSuccess = true;
    try
    {            
        if ( receipt.IsString() && !receipt.IsNULLOrEmpty() )
        {
            const char* receiptHandle = receipt.AsCharPtr();
            if ( GUCEF_NULL != receiptHandle )
            {        
                Aws::SQS::Model::DeleteMessageRequest delete_req;
                delete_req.SetQueueUrl( m_queueUrl );
                delete_req.SetReceiptHandle( receiptHandle );

                Aws::SQS::Model::DeleteMessageOutcome delete_out = m_client->GetSqsClient().DeleteMessage( delete_req );
                if ( delete_out.IsSuccess() )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:AcknowledgeReceipt: Successfully deleted message with receipt handle " + CORE::ToString( receiptHandle ) );
                    return true;
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:AcknowledgeReceipt: provided variant does not have valid content" );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:AcknowledgeReceipt: provided variant is not a string" );
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, CORE::CString( "AwsSqsPubSubClientTopic:AcknowledgeReceipt: experienced an exception: " ) + e.what() );
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "AwsSqsPubSubClientTopic:AcknowledgeReceipt: experienced an unknown exception, your application may be unstable" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg )
{GUCEF_TRACE;

    const CORE::CVariant& msgIndex = msg.GetMsgIndex();
    return AcknowledgeReceipt( msgIndex );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    if ( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX == bookmark.GetBookmarkType() )
    {
        const CORE::CVariant& msgIndex = bookmark.GetBookmarkData();
        return AcknowledgeReceipt( msgIndex );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:AcknowledgeReceipt: provided bookmark type is invalid" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::ApplySqsMessageAttributeNameContraints( CORE::CAsciiString& candidateName )
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
CAwsSqsPubSubClientTopic::AddAttributesToSqsMsg( T& sqsMsg                                          , 
                                                 const PUBSUB::CIPubSubMsg::TKeyValuePairs& kvPairs ,
                                                 bool addPrefix                                     ,
                                                 const CORE::CAsciiString& prefixToAdd              ,
                                                 CORE::UInt32& msgByteSize                          )
{GUCEF_TRACE;

    if ( !kvPairs.empty() )
    {        
        typedef Aws::Map< Aws::String, Aws::SQS::Model::MessageAttributeValue > TSqsMsgAttributeMap;

        TSqsMsgAttributeMap attribs;
        PUBSUB::CIPubSubMsg::TKeyValuePairs::const_iterator n = kvPairs.begin();
        while ( n != kvPairs.end() )
        {
            const PUBSUB::CIPubSubMsg::TKeyValuePair& kvPair = (*n);
            const CORE::CVariant& kvKey = kvPair.first;
            const CORE::CVariant& kvValue = kvPair.second;

            if ( kvKey.IsInitialized() && kvValue.IsInitialized() )
            {                    
                Aws::SQS::Model::MessageAttributeValue sqsAttribValue;
                bool valueHasBeenSetOk = false;

                if ( kvValue.IsString() )
                {
                    CORE::CUtf8String valueStr = kvValue.AsUtf8String();
                    if ( !valueStr.IsNULLOrEmpty() )
                    {
                        sqsAttribValue.SetDataType( "String" );
                        msgByteSize += 6;                 
                        sqsAttribValue.SetStringValue( valueStr );
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
                        sqsAttribValue.SetDataType( "StringValue" );
                        msgByteSize += 11;                 
                        sqsAttribValue.SetStringValue( valueStr );
                        msgByteSize += (valueStr.ByteSize()-1);
                        valueHasBeenSetOk = true;
                    }
                }
                else // kvValue.IsBinary() ? catch-all
                {
                    if ( kvValue.ByteSize() > 0 )
                    {
                        sqsAttribValue.SetDataType( "Binary" );
                        msgByteSize += 6;
                        sqsAttribValue.SetBinaryValue( Aws::Utils::ByteBuffer( static_cast< const unsigned char* >( kvValue.AsVoidPtr() ), kvValue.ByteSize() ) );
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
                    ApplySqsMessageAttributeNameContraints( kvKeyName );
                    if ( !kvKeyName.IsNULLOrEmpty() )
                    {
                        sqsMsg.AddMessageAttributes( kvKeyName, sqsAttribValue ); // if there are dupicate keys last one wins                    
                        msgByteSize += (kvKeyName.ByteSize()-1);
                    }
                }
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Uninitialized key and/or value set on msg, ignoring" );
            }

            ++n;
        }    
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template< class T >
bool
CAwsSqsPubSubClientTopic::TranslateToSqsMsgOfType( T& sqsMsg                      , 
                                                   const PUBSUB::CIPubSubMsg* msg , 
                                                   CORE::UInt32& msgByteSize      )
{GUCEF_TRACE;

    if ( GUCEF_NULL == msg )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:TranslateToSqsMsgOfType: NULL Message passed" );
        return false;
    }

    // A message can include only XML, JSON, and unformatted text. The following Unicode characters are allowed:
    // #x9 | #xA | #xD | #x20 to #xD7FF | #xE000 to #xFFFD | #x10000 to #x10FFFF
    // The minimum size is one character. The maximum size is 256 KB (or smaller based on queue settings)

    const CORE::CVariant& bodyPayload = msg->GetPrimaryPayload();
    if ( bodyPayload.IsInitialized() )
    {            
        // We request the payload as a string. Note that this auto converts
        // Binary is Base64 encoded. For SQS strings are Unicode with UTF-8 binary encoding
        CORE::CUtf8String bodyPayloadStr = bodyPayload.AsUtf8String();
            
        if ( bodyPayloadStr.ByteSize() >= 1 && bodyPayloadStr.ByteSize() <= m_sqsMaximumMessageSize )
        {
            sqsMsg.SetMessageBody( bodyPayloadStr );
            msgByteSize += (bodyPayloadStr.ByteSize()-1);
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:TranslateToSqsMsgOfType: Message body size as string has an invalid size. Must be between 1-256KB. Cannot translate" );
            return false;
        }
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:TranslateToSqsMsgOfType: Message does not have a body which SQS does not allow. Cannot publish" );
        return false;
    }

    bool totalSuccess = true;
    const PUBSUB::CIPubSubMsg::TKeyValuePairs& kvPairs = msg->GetKeyValuePairs();
    totalSuccess = AddAttributesToSqsMsg( sqsMsg, kvPairs, m_config.addPrefixWhenSendingKvPairs, m_config.kvPairPrefixToAddOnSend, msgByteSize ) && totalSuccess;
    const PUBSUB::CIPubSubMsg::TKeyValuePairs& metaKvPairs = msg->GetMetaDataKeyValuePairs();
    totalSuccess = AddAttributesToSqsMsg( sqsMsg, metaKvPairs, m_config.addPrefixWhenSendingMetaDataKvPairs, m_config.metaDatakvPairPrefixToAddOnSend, msgByteSize ) && totalSuccess;

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::TranslateToSqsBatchMsg( Aws::SQS::Model::SendMessageBatchRequestEntry& sqsMsg , 
                                                  const PUBSUB::CIPubSubMsg* msg                        , 
                                                  CORE::UInt64 publishActionId                          ,
                                                  CORE::UInt32& approxMsgByteSize                       )
{GUCEF_TRACE;

    /*
    
        The JSON for the request will become as follows:
        
        {
           "Entries": [ 
              { 
                 "DelaySeconds": number,
                 "Id": "string",
                 "MessageAttributes": { 
                    "string" : { 
                       "BinaryListValues": [ blob ],
                       "BinaryValue": blob,
                       "DataType": "string",
                       "StringListValues": [ "string" ],
                       "StringValue": "string"
                    }
                 },
                 "MessageBody": "string",
                 "MessageDeduplicationId": "string",
                 "MessageGroupId": "string",
                 "MessageSystemAttributes": { 
                    "string" : { 
                       "BinaryListValues": [ blob ],
                       "BinaryValue": blob,
                       "DataType": "string",
                       "StringListValues": [ "string" ],
                       "StringValue": "string"
                    }
                 }
              }
           ],
           "QueueUrl": "string"
        }
    
        
        Hence based on the above we will put a floor message size at 742 bytes purely due to JSON formatting on a per batch entry basis
    
    */
    approxMsgByteSize += 742;
   

    // Specific to batch messages: Its mandatory to set an ID on each entry in the batch.
    //                             This is done to be able to provide a response to the API call that has individual success/fail states per entry

    CORE::CString IdStr = CORE::ToString( publishActionId ); 
    sqsMsg.SetId( IdStr );
    approxMsgByteSize += (IdStr.ByteSize()-1);
    
    // Now do the translation work which is generic and shared between batch and non-batch messages
    
    return TranslateToSqsMsgOfType< Aws::SQS::Model::SendMessageBatchRequestEntry >( sqsMsg, msg, approxMsgByteSize );
}

/*-------------------------------------------------------------------------*/
    
bool 
CAwsSqsPubSubClientTopic::TranslateToSqsMsg( Aws::SQS::Model::SendMessageRequest& sqsMsg ,
                                             const PUBSUB::CIPubSubMsg* msg              , 
                                             CORE::UInt32& approxMsgByteSize             )
{GUCEF_TRACE;

    /*
    
        The JSON for the request will become as follows:

        {
           "DelaySeconds": number,
           "MessageAttributes": { 
              "string" : { 
                 "BinaryListValues": [ blob ],
                 "BinaryValue": blob,
                 "DataType": "string",
                 "StringListValues": [ "string" ],
                 "StringValue": "string"
              }
           },
           "MessageBody": "string",
           "MessageDeduplicationId": "string",
           "MessageGroupId": "string",
           "MessageSystemAttributes": { 
              "string" : { 
                 "BinaryListValues": [ blob ],
                 "BinaryValue": blob,
                 "DataType": "string",
                 "StringListValues": [ "string" ],
                 "StringValue": "string"
              }
           },
           "QueueUrl": "string"
        }


        Hence based on the above we will put a floor message size at 574 bytes purely due to JSON formatting

    */
    approxMsgByteSize += 574;
    approxMsgByteSize += (CORE::UInt32) m_queueUrl.size();

    // Now do the translation work which is generic and shared between batch and non-batch messages
    
    return TranslateToSqsMsgOfType< Aws::SQS::Model::SendMessageRequest >( sqsMsg, msg, approxMsgByteSize );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds                      , 
                                   const PUBSUB::CIPubSubMsg::TIPubSubMsgConstRawPtrVector& msgs ,
                                   bool notify                                                   )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
        return false;
    
    bool totalSuccess = true;
    try
    {            
        if ( m_config.tryToUseSendMessageBatch )
        {       
            size_t preExistingActionIds = publishActionIds.size(); 
            size_t n=0;
            CORE::UInt32 sqsMsgByteSize = 0;
            PUBSUB::CIPubSubMsg::TIPubSubMsgConstRawPtrVector::const_iterator i = msgs.begin();
            while ( i != msgs.end() )
            {        
                size_t batchStartN = n;

                Aws::SQS::Model::SendMessageBatchRequest sm_req;
                sm_req.SetQueueUrl( m_queueUrl );

                while ( i != msgs.end() )
                {                 
                    const PUBSUB::CIPubSubMsg* msg = (*i);            
                
                    CORE::UInt64 publishActionId = 0;
                    if ( preExistingActionIds > n )
                    {
                        publishActionId = publishActionIds[ n ];
                        if ( 0 == publishActionId )
                        {
                            publishActionId = m_currentPublishActionId; 
                            ++m_currentPublishActionId;
                            publishActionIds[ n ] = publishActionId;
                        }
                    }
                    else
                    {
                        publishActionId = m_currentPublishActionId; 
                        ++m_currentPublishActionId;
                        publishActionIds.push_back( publishActionId );
                    }

                    CORE::UInt32 sqsSingleMsgByteSize = 0;
                    Aws::SQS::Model::SendMessageBatchRequestEntry sqsMsg;                        
                    if ( TranslateToSqsBatchMsg( sqsMsg, msg, publishActionId, sqsSingleMsgByteSize ) )
                    {
                        if ( ( sqsMsgByteSize + sqsSingleMsgByteSize <= m_sqsMaximumMessageSize ) &&  // max batch message size is still just 256K same as using non-batch so this is mostly usefuly for small messages
                             ( sm_req.GetEntries().size() < 10 )                                   )  // max nr of entries per the AWS API is 10 per batch operation
                        {
                            sm_req.AddEntries( sqsMsg );
                            sqsMsgByteSize += sqsSingleMsgByteSize;
                        }
                        else
                        {
                            if ( !sm_req.GetEntries().empty() )
                            {
                                Aws::SQS::Model::SendMessageBatchOutcome sm_out = m_client->GetSqsClient().SendMessageBatch( sm_req );
                                if ( sm_out.IsSuccess() )
                                {
                                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Success performing batch operation for messages to \"" + CORE::ToString( m_queueUrl ) + 
                                        "\" with " + CORE::ToString( sm_out.GetResult().GetFailed().size() ) + " entries failed out of " + CORE::ToString( sm_req.GetEntries().size() ) );

                                    const Aws::Vector< Aws::SQS::Model::BatchResultErrorEntry >& failedEntries = sm_out.GetResult().GetFailed();
                                    Aws::Vector< Aws::SQS::Model::BatchResultErrorEntry >::const_iterator b = failedEntries.begin();
                                    while ( b != failedEntries.end() )
                                    {
                                        const Aws::SQS::Model::BatchResultErrorEntry& failureEntry = (*b);
                                        CORE::UInt64 publishActionIdOfFailedEntry = CORE::StringToUInt64( failureEntry.GetId(), GUCEF_UINT64MAX );
                                        if ( publishActionIdOfFailedEntry == GUCEF_UINT64MAX )
                                        {
                                            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Likely issue interpreting the 'Id' of a failed batch entry error result: " + CORE::ToString( failureEntry.GetId() ) );
                                        }
                                        m_publishFailureActionIds.push_back( publishActionIdOfFailedEntry );
                                        ++b;
                                    }

                                    const Aws::Vector<Aws::SQS::Model::SendMessageBatchResultEntry>& successEntries = sm_out.GetResult().GetSuccessful();
                                    Aws::Vector< Aws::SQS::Model::SendMessageBatchResultEntry >::const_iterator b2 = successEntries.begin();
                                    while ( b2 != successEntries.end() )
                                    {
                                        const Aws::SQS::Model::SendMessageBatchResultEntry& successEntry = (*b2);
                                        CORE::UInt64 publishActionIdOfSuccessEntry = CORE::StringToUInt64( successEntry.GetId(), GUCEF_UINT64MAX );
                                        if ( publishActionIdOfSuccessEntry == GUCEF_UINT64MAX )
                                        {
                                            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Likely issue interpreting the 'Id' of a batch entry success result: " + CORE::ToString( successEntry.GetId() ) );
                                        }
                                        m_publishSuccessActionIds.push_back( publishActionIdOfSuccessEntry );
                                        ++b2;
                                    }
                                }
                                else
                                {
                                    totalSuccess = false;
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Error sending batch message to \"" + m_queueUrl + "\". Error Msg: " + sm_out.GetError().GetMessage() );

                                    if ( notify && !msgs.empty() )
                                        for ( size_t i=batchStartN; i<n; ++i )    
                                            m_publishFailureActionIds.push_back( publishActionIds[ preExistingActionIds + i ] );
                                }                                
                            }
                            else
                            {
                                // We could not even add a single message to the batch message
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Failed to send message using SQS batch API. publishActionId=" + CORE::ToString( publishActionId ) );

                                // the non-batch API has slightly less message byte overhead so we will try again using that API instead.
                                // the regular publish will add the action ids
                                Publish( publishActionId, *msg, false );
                                break;
                            }

                            // we need to start a new SQS batch message to stay within its constraints  
                            sqsMsgByteSize = 0;                            

                            // Undo the action id change since we will go for another pass
                            --m_currentPublishActionId;
                            publishActionIds.pop_back();
                            break;
                        }
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:Publish: Failed to translate message something SQS compatible. publishActionId=" + CORE::ToString( publishActionIds.back() ) );
                        m_publishFailureActionIds.push_back( publishActionIds.back() );
                    }

                    ++i; 
                    ++n;
                }
            }
        }
        else
        {
            Aws::SQS::Model::SendMessageRequest sm_req;
            sm_req.SetQueueUrl( m_queueUrl );
       
            size_t preExistingActionIds = publishActionIds.size(); size_t n=0;
            PUBSUB::CIPubSubMsg::TIPubSubMsgConstRawPtrVector::const_iterator i = msgs.begin();
            while ( i != msgs.end() )
            {        
                CORE::UInt64 publishActionId = 0;
                if ( preExistingActionIds > n )
                {
                    publishActionId = publishActionIds[ n ];
                    if ( 0 == publishActionId )
                    {
                        publishActionId = m_currentPublishActionId; 
                        ++m_currentPublishActionId;
                        publishActionIds[ n ] = publishActionId;
                    }
                }
                else
                {
                    publishActionId = m_currentPublishActionId; 
                    ++m_currentPublishActionId;
                    publishActionIds.push_back( publishActionId );
                }

                bool success = Publish( publishActionId, *(*i), false );

                totalSuccess = success && totalSuccess;                
                ++i; ++n;
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, CORE::CString( "AwsSqsPubSubClientTopic:GetSqsQueueUrlForQueueName: experienced an exception: " ) + e.what() );
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "AwsSqsPubSubClientTopic:GetSqsQueueUrlForQueueName: experienced an unknown exception, your application may be unstable" );
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClientTopic::IsQueueEmpty( void ) 
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
        m_client->IsQueueEmpty( m_queueUrl );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClientTopic::OnPulseCycle( CORE::CNotifier* notifier    ,
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
CAwsSqsPubSubClientTopic::SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    config = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClientTopic::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    m_config = config;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClientTopic::PrepStorageForReadMsgs( CORE::UInt32 msgCount )
{GUCEF_TRACE;

    // reset size, note this does not dealloc the underlying memory
    m_pubsubMsgs.clear();
    TPubSubMsgsRefVector& msgRefs = m_pubsubMsgsRefs;
    msgRefs.clear();

    if ( msgCount > m_pubsubMsgs.capacity() )
    {
        // Grow the actual storage, this is allowed to become larger than msgCount to limit
        // the nr of dynamic allocations
        m_pubsubMsgs.reserve( msgCount );        
        msgRefs.reserve( msgCount );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );


    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark ) 
{GUCEF_TRACE;

    // Not supported
    return false;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubBookmark 
CAwsSqsPubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;

    // Not supported
    return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_APPLICABLE );
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClientTopic::InitializeConnectivity( bool reset )
{GUCEF_TRACE;

    if ( reset )
    {
        if ( !Disconnect() )
            return false;
    }
    
    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL != m_client )
    {
        if ( m_config.topicNameIsQueueName )
            m_queueUrl = GetSqsQueueUrlForQueueName( m_config.topicName );
        else
            m_queueUrl = m_config.topicName;

        if ( !m_queueUrl.empty() )
        {
            CORE::CString::StringMap queueAttributes;
            if ( m_client->TryGetQueueAttributes( m_queueUrl, queueAttributes ) )
            {            
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:InitializeConnectivity: Retrieved attributes for queue \"" + CORE::ToString( m_queueUrl ) + 
                    "\" which are as follows: " + CORE::ToString( queueAttributes ) );
                
                CORE::CString::StringMap::iterator i = queueAttributes.find( "MaximumMessageSize" );
                if ( i != queueAttributes.end() )
                {
                    m_sqsMaximumMessageSize = CORE::StringToUInt32( (*i).second, m_sqsMaximumMessageSize );
                }
                return true;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:InitializeConnectivity: No queue URL could be determined thus cannot use SQS queue" );
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

CAwsSqsPubSubClientTopic::TopicMetrics::TopicMetrics( void )
    : sqsMessagesTransmitted( 0 )
    , sqsMessagesReceived( 0 )
    , sqsMessagesInQueue( 0 )
    , sqsMessagesFiltered( 0 )
    , sqsErrorReplies( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClientTopic::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CAwsSqsPubSubClientTopic::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

 MT::TLockStatus
 CAwsSqsPubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CAwsSqsPubSubClientTopic::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CAwsSqsPubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::AWSSQS::CAwsSqsPubSubClientTopic";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSSQS */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
