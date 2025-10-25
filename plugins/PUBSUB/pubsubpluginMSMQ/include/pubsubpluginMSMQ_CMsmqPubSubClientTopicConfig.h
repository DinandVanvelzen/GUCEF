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

#ifndef PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPICCONFIG_H
#define PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPICCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <transact.h>
#include <Mq.h>

#include <vector>

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#include "gucefPUBSUB_CPubSubClientTopicConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef PUBSUBPLUGIN_MSMQ_MACROS_H
#include "pubsubpluginMSMQ_macros.h"
#define PUBSUBPLUGIN_MSMQ_MACROS_H
#endif /* PUBSUBPLUGIN_MSMQ_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMsmqPubSubClient;

/**
 *  Standard pub-sub client config with some custom specifcs added for this
 *  specific pub-sub backend
 */
class PUBSUBPLUGIN_MSMQ_PLUGIN_PRIVATE_CPP CMsmqPubSubClientTopicConfig : public PUBSUB::CPubSubClientTopicConfig
{
    public:

    typedef GUCEF::vector< MSGPROPID >                  MSGPROPIDVector;
    typedef GUCEF::map< MSGPROPID, MSGPROPIDVector >    MSGPROPIDMapVector;

    CMsmqPubSubClientTopicConfig( void );
    
    CMsmqPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig );

    virtual ~CMsmqPubSubClientTopicConfig() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CMsmqPubSubClientTopicConfig& operator=( const PUBSUB::CPubSubClientTopicConfig& src );

    CMsmqPubSubClientTopicConfig& operator=( const CMsmqPubSubClientTopicConfig& src );

    bool LoadCustomConfig( const CORE::CDataNode& config );

    bool PopulateDefaultReceivePropIds( void );
    bool SupplementPropIdsWithPairedPropId( MSGPROPIDVector& propIds );
    bool PopulateDefaultSendPropIds( void );
    void PopulateDefaultPairedPropIds( void );
    CORE::CString PropIdsToCsvString( const MSGPROPIDVector& propIds );
    bool CsvStringToPropIds( const CORE::CString& csvStr, MSGPROPIDVector& propIds );
    void ErasePropId( PROPID propId, MSGPROPIDVector& propIds );
    bool IsPairedPropId( PROPID id ) const;
    static void AddPropId( PROPID id, MSGPROPIDVector& propIds );
    static bool HasPropId( PROPID id, const MSGPROPIDVector& propIds );

    MSGPROPIDVector msmqPropIdsToReceive;
    MSGPROPIDVector msmqPropIdsToSend;
    MSGPROPIDMapVector msmqPairedPropIds;
    CORE::UInt32 maxMsmqMsgsToReadPerSyncCycle;
    bool topicNameIsMsmqFormatName;
    bool convertMsmqMsgIdToString;
    bool convertMsmqClsIdToString;
    bool ignoreUnmappableMetaDataFieldOnPublish;
    bool gatherMsmqTransitTimeOnReceiveMetric;
    CORE::UInt32 maxMsmqTransitTimeOnReceiveMetricDataPoints;
    CORE::UInt64 msmqMsgPropIdToMapToMsgIdOnReceive;
    CORE::UInt32 defaultMsmqBodyBufferSizeInBytes;
    CORE::UInt32 defaultMsmqMiscBufferSizeInBytes;
    CORE::UInt16 minPayloadFieldGrowthPercOnBufferTooSmall;
    bool retainMsmqSentTimeAsMetaData;
    bool addMsgReadFromMsmqTimeAsMetaData;
    CORE::CVariant metadataKeyForMsgReadFromMsmqTime;
    CORE::Int32 maxMsmqErrorsOnAckToBeHealthy;
    CORE::Int32 maxMsmqErrorsOnReceiveToBeHealthy;
};

/*--------------------------------------------------------------------------*/

/*

PROPID_M_ABORT_COUNT	            (Introduced in MSMQ 4.0.) Indicates the number of times that transactional processing has been aborted on a message since it was placed in its current queue.
PROPID_M_ACKNOWLEDGE	            Specifies the type of acknowledgment messages returned to the sending application.
PROPID_M_ADMIN_QUEUE	            Specifies one administration queue used for returned acknowledgment messages.
PROPID_M_ADMIN_QUEUE_LEN	        Specifies the length of the administration queue buffer specified in PROPID_M_ADMIN_QUEUE.
PROPID_M_APPSPECIFIC	            Specifies application-generated information.
PROPID_M_ARRIVEDTIME	            Indicates when the message arrived at the queue.
PROPID_M_AUTH_LEVEL	                Specifies the authentication level of the message.
PROPID_M_AUTHENTICATED	            Indicates whether the message was authenticated at the request of the sending application.
PROPID_M_AUTHENTICATED_EX	        (Introduced in MSMQ 2.0.) Indicates whether the message was authenticated at the request of the sending application and what type of digital signature was used.
PROPID_M_BODY	                    Contains the body of the message.
PROPID_M_BODY_SIZE	                Indicates the actual size of the message body.
PROPID_M_BODY_TYPE	                Indicates the type of body that the message contains.
PROPID_M_CLASS	                    Indicates the type of message.
PROPID_M_COMPOUND_MESSAGE	        (Introduced in MSMQ 3.0.) Provides the entire contents of an SRMP message, including the SOAP envelope and the SOAP attachments associated with it.
PROPID_M_COMPOUND_MESSAGE_SIZE	    (Introduced in MSMQ 3.0.) Provides the size of the entire contents of an SRMP message, including the SOAP envelope and the SOAP attachments associated with it.
PROPID_M_CONNECTOR_TYPE	            Specifies a connector application identifier. This property is set when some properties typically set by Message Queuing are set by the sending application.
PROPID_M_CORRELATIONID	            Specifies the application-generated correlation identifier of the message.
PROPID_M_DEADLETTER_QUEUE	        Specifies the path name of an application-specific dead letter queue (DLQ).
PROPID_M_DEADLETTER_QUEUE_LEN	    Indicates the length (in Unicode characters) of the PROPID_M_DEADLETTER_QUEUE property.
PROPID_M_DELIVERY	                Specifies how the message is delivered.
PROPID_M_DEST_FORMAT_NAME	        (Introduced in MSMQ 3.0.) Indicates the destination queue or queues to which the message was sent.
PROPID_M_DEST_FORMAT_NAME_LEN	    (Introduced in MSMQ 3.0.) Indicates the length (in Unicode characters) of the format name buffer allocated by the receiving application and the length of the format name string returned in PROPID_M_DEST_FORMAT_NAME.
PROPID_M_DEST_QUEUE	                Identifies the destination queue of the message.
PROPID_M_DEST_QUEUE_LEN	            Indicates the length of the format name buffer for the destination queue.
PROPID_M_DEST_SYMM_KEY	            Specifies the symmetric key used to encrypt an application-encrypted message or a message sent to a foreign queue.
PROPID_M_DEST_SYMM_KEY_LEN	        Specifies the length of the symmetric key used to encrypt messages sent to a foreign queue.
PROPID_M_ENCRYPTION_ALG	            Specifies the encryption algorithm used when encrypting private and authenticated messages.
PROPID_M_EXTENSION	                Provides a place to put additional information associated with the message.
PROPID_M_EXTENSION_LEN	            Specifies the length of the extension information property.
PROPID_M_FIRST_IN_XACT	            (Introduced in MSMQ 2.0.) Indicates whether the message was the first message in a transaction.
PROPID_M_HASH_ALG	                Identifies the hashing algorithm that Message Queuing uses when authenticating the message.
PROPID_M_JOURNAL	                Specifies whether the message should be kept in the computer journal, sent to a dead-letter queue, or neither.
PROPID_M_LABEL	                    Specifies a description of the message.
PROPID_M_LABEL_LEN	                Identifies the length of the message label buffer.
PROPID_M_LAST_IN_XACT	            (Introduced in MSMQ 2.0.) Indicates whether the message was the last message in a transaction.
PROPID_M_LOOKUPID	                (Introduced in MSMQ 3.0.) Indicates the lookup identifier of the message.
PROPID_M_MOVE_COUNT	                (Introduced in MSMQ 4.0.) Indicates the number of times that transactional processing has been aborted on a message during its lifetime.
PROPID_M_MSGID	                    Identifies the identifier of the message (GUID).
PROPID_M_PRIORITY	                Specifies the priority level of the message.
PROPID_M_PRIV_LEVEL	                Specifies the privacy level of the message to be sent.
PROPID_M_PROV_NAME	                Specifies the name of the cryptographic provider used to generate the digital signature attached to the message.
PROPID_M_PROV_NAME_LEN	            Specifies the length of the name of the cryptographic provider used for validating the digital signature attached to the message.
PROPID_M_PROV_TYPE	                Specifies the type of cryptographic provider used to generate the digital signature attached to the message.
PROPID_M_RESP_FORMAT_NAME	        (Introduced in MSMQ 3.0, supersedes PROPID_M_RESP_QUEUE.) Specifies the response queues used for returning application-generated response messages.
PROPID_M_RESP_FORMAT_NAME_LEN	    (Introduced in MSMQ 3.0.) Indicates the length (in Unicode characters) of the format name buffer allocated by the receiving application and the length of format name string returned in PROPID_M_RESP_FORMAT_NAME.
PROPID_M_RESP_QUEUE	                (Superseded by PROPID_M_RESP_FORMAT_NAME.) Specifies the response queue used for returning response messages.
PROPID_M_RESP_QUEUE_LEN	            Specifies the length of the format name of the response queue.
PROPID_M_SECURITY_CONTEXT	        Specifies a handle to a security context structure.
PROPID_M_SENDER_CERT	            Specifies the external certificate used to authenticate the message.
PROPID_M_SENDER_CERT_LEN	        Specifies the length of the external certificate buffer.
PROPID_M_SENDERID	                Identifies who sent the message.
PROPID_M_SENDERID_LEN	            Indicates the length of the sender identifier.
PROPID_M_SENDERID_TYPE	            Specifies the type of sender identifier.
PROPID_M_SENTTIME	                Indicates the date and time that the message was sent by the source queue manager.
PROPID_M_SIGNATURE	                Specifies the MSMQ 1.0 signature or an application-generated signature that is attached to the message.
PROPID_M_SIGNATURE_LEN	            Specifies the length of the MSMQ 1.0 signature or application-generated signature that is attached to the message.
PROPID_M_SOAP_BODY	                (Introduced in MSMQ 3.0.) Specifies application-generated SOAP body elements for inclusion in the SOAP envelope of an SRMP message.
PROPID_M_SOAP_ENVELOPE	            (Introduced in MSMQ 3.0.) Provides the SOAP envelope of an SRMP message.
PROPID_M_SOAP_ENVELOPE_LEN	        (Introduced in MSMQ 3.0.) Provides the size of the SOAP envelope of an SRMP message.
PROPID_M_SOAP_HEADER	            (Introduced in MSMQ 3.0.) Specifies additional application-generated header elements for inclusion in the SOAP envelope of an SRMP message.
PROPID_M_SRC_MACHINE_ID	            Specifies the computer where the message originated.
PROPID_M_TIME_TO_BE_RECEIVED	    Specifies the total time the message is allowed to live.
PROPID_M_TIME_TO_REACH_QUEUE	    Specifies a time limit for the message to reach the queue.
PROPID_M_TRACE	                    Specifies whether Message Queuing sends report messages indicating the route of the message.
PROPID_M_VERSION	                (Introduced in MSMQ 3.0.) Specifies the wire format used to send the message.
PROPID_M_XACTID	                    (Introduced in MSMQ 2.0.) Indicates the transaction identifier for the message.
PROPID_M_XACT_STATUS_QUEUE	        Identifies the transaction status queue on the source computer.
PROPID_M_XACT_STATUS_QUEUE_LEN	    Indicates the length of the format name buffer of the transaction status queue.

*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MSMQ */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPICCONFIG_H ? */
