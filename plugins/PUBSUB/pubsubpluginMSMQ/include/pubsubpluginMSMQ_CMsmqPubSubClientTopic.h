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

#ifndef PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPIC_H
#define PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPIC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#include "gucefPUBSUB_CPubSubClientTopic.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

#ifndef PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPICCONFIG_H
#include "pubsubpluginMSMQ_CMsmqPubSubClientTopicConfig.h"
#define PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPICCONFIG_H
#endif /* PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPICCONFIG_H ? */

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
 *  MS Windows MSMQ implementation of the conceptual pub-sub "topic"
 *  MSMQ feature support is implemented on a as-needed basis and as such this backend may not
 *  be feature complete for every usecase
 *  MSMQ generally is not very friendly towards the implementation of a config driven backend as generally the approach is to use
 *  hardcoded message property lists. We deal with that by using a default common set with the ability to specify other properties
 *  via property ID nr via the backend custom config
 *
 *  Note that a lot of effort was put into the code to ensure that we can use pre-allocated structures, reusing memory
 *  This greatly adds to the performance of the code as we are merely setting memory addresses and not doing allocations on a per message basis
 *  As a tradeoff this makes the code in the backend appear more complex due to the memory mapping aspect.
 * 
 *  General MSMQ notes not specific to this implementation:
 * 
 *      All message properties can be read. However, only those properties specified in the pMessagePropsparameter are returned to the calling application; other properties are simply discarded when the message is read. 
 *      For example, you may choose to retrieve the size of the message without retrieving the message body itself.
 *
 *      The hCursorparameter contains the handle to a cursor created by MQCreateCursor. Using a cursor is optional and is only necessary when you want to read messages that are not at the front of the queue. 
 *      When using a cursor, you must peek at the first message in the queue by setting dwActionto MQ_ACTION_PEEK_CURRENT before you make calls with dwActionset to MQ_ACTION_PEEK_NEXT.
 *
 *      To retrieve the message body, you must specify PROPID_M_BODY in pMessageProps. The VT field of the corresponding element in the aPropVararray should be set to VT_UI1 | VT_VECTOR, allowing MSMQ to use the buffer specified in the CAUI1structure to store the message. 
 *      If the supplied buffer is too small to contain the entire message body, MQReceiveMessagefails and MQ_ERROR_BUFFER_OVERFLOW is returned. 
 *      The buffer is filled to capacity, and the full message remains in the queue. When this happens, the other properties specified by pMessagePropsare still read.
 *
 *      To retrieve the size of the message, specify PROPID_M_BODY_SIZE in pMessageProps. MSMQ sets PROPID_M_BODY_SIZE to the size of the message body, even if MQReceiveMessage fails because the message body exceeded the buffer allocated by PROPID_M_BODY. 
 *      When retrieving the message body size, the CAUI1structure associated with the PROPID_M_BODY property does not indicate the size. The cElemsfield of the CAUI1structure merely indicates the maximum message body, which could be copied into the pElemsbuffer. 
 *      The cElemsfield is never modified by MSMQ.
 *      Not all properties require the application to specify the property type in the VT field of the aPropVararray. For these properties, the corresponding VT field in the aPropVararray can be set to VT_NULL.
 *      When reading messages in a queue, the function's time-out timer ( dwTimeout) can be set to 0, a specific amount of time, or INFINITE. A message can be retrieved if it is available at that period of time.
 *      Messages cannot be read from a queue that resides on a computer without a direct connection. Opening a queue with receive or peek access requires a direct connection to the computer where the queue resides.
 *
 *      Synchronously Reading Messages:
 *      To synchronously read messages, set fnReceiveCallbackand lpOverlappedto NULL. When this is done, the calling thread is blocked until a suitable message is available or a time-out occurs.
 *      Using a zero timeout allows for a non-blocking check for messages
 *
 *      Asynchronously Reading Messages:
 *      When asynchronously reading messages, MQReceiveMessagereturns a SUCCESS value if a suitable message is found. Otherwise, the function returns immediately with the return value MQ_INFORMATION_OPERATION_PENDING. 
 *      This return value indicates that the operation is pending and will be completed as soon as a suitable message can be found. Asynchronous receive is based on standard Microsoft Win32 mechanisms.
 *      Applications can use a callback function, a Windows Event mechanism, or a completion port to read messages asynchronously. When using a callback function, you can register only 64 callbacks at the same time.
 *      Completion ports cannot be used by dependent client applications. On MSMQ independent clients and servers, queue handles are implemented as file handles and so, can be associated with completion ports. 
 *      However, on dependent clients, queue handles are not file handles and as a result, cannot be used with completion ports.
 *      The output parameters to an asynchronous call to MQReceiveMessageshould be kept intact until the operation is complete (that is, you cannot free them or reuse them). Use automatic variables with caution.
 *
 *      Messages in Administration Queues
 *      When reading acknowledgment messages in an administration queue, you can see if the original message failed to reach its destination or was not retrieved from the queue by looking at the class property ( PROPID_M_CLASS) of the acknowledgment message. 
 *      The class property will contain a positive or negative acknowledgment, depending on the acknowledgment level specified by the original message.
 *      If the class property is positive, the original message body is not included in the acknowledgment message. If the class property is negative, the message body is included as the message body of the acknowledgment message.
 *
 *      Responding to Messages
 *      The receiving application can determine if the sending application expects a response by retrieving the PROPID_M_RESP_QUEUE and PROPID_M_RESP_QUEUE_LEN properties when reading a message. 
 *      PROPID_M_RESP_QUEUE contains the format name of a response queue if a response is requested.
 *      When opening a response queue to send several response messages, the receiving application can cache the queue handle returned by MQOpenQueue, thus eliminating the need to call MQOpenQueueseveral times for the same response queue. 
 */
class PUBSUBPLUGIN_MSMQ_PLUGIN_PRIVATE_CPP CMsmqPubSubClientTopic : public PUBSUB::CPubSubClientTopic ,
                                                                    public CORE::CTSharedObjCreator< CMsmqPubSubClientTopic, MT::CMutex >
{
    public:

    typedef GUCEF::vector< CORE::UInt32 > UInt32Vector;

    CMsmqPubSubClientTopic( CMsmqPubSubClient* client );

    virtual ~CMsmqPubSubClientTopic() GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClient* GetClient( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool InitializeConnectivity( bool reset ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    bool BeginReconnectSequence( const CORE::CEvent* eventMsg );

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribed( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscriptionAtEndOfData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsPublishingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Subscribe( void ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool SubscribeStartingAtMsgId( const CORE::CVariant& msgIdBookmark );

    virtual bool SubscribeStartingAtMsgDateTime( const CORE::CDateTime& msgDtBookmark );

    virtual bool SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubBookmark GetCurrentBookmark( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTopicName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetMetricFriendlyTopicName( void ) const;

    virtual bool Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;
    bool AcknowledgeReceiptImpl( const PUBSUB::CPubSubBookmark& bookmark, CORE::UInt64 receiveActionId );

    virtual bool DeriveBookmarkFromMsg( const PUBSUB::CIPubSubMsg& msg, PUBSUB::CPubSubBookmark& bookmark ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    CORE::UInt64 GetMsmqErrorsOnReceiveCounter( bool resetCounter );

    CORE::UInt64 GetMsmqMessagesReceivedCounter( bool resetCounter );

    CORE::UInt64 GetMsmqMessagesPublishedCounter( bool resetCounter );

    CORE::UInt64 GetMsmqErrorsOnPublishCounter( bool resetCounter );

    CORE::UInt64 GetMsmqErrorsOnAckCounter( bool resetCounter );
    
    class TopicMetrics
    {
        public:

        TopicMetrics( void );

        CORE::Int64 msmqMsgsInQueue;
        CORE::Int64 msmqMsgsInJournal;
        CORE::Int64 msmqMsgBytesInQueue;
        CORE::Int64 msmqMsgBytesInJournal;

        CORE::UInt64 msmqMessagesPublished;
        CORE::UInt64 msmqErrorsOnPublish;
        CORE::UInt64 msmqErrorsOnAck;

        CORE::UInt64 msmqMessagesReceived;
        CORE::UInt64 msmqErrorsOnReceive;

        CORE::UInt64 msmqGlobalMsgBytes;
        UInt32Vector msmqMsgSentToArriveLatencies;
    };

    const TopicMetrics& GetMetrics( void ) const;

    const CMsmqPubSubClientTopicConfig& GetTopicConfig( void ) const;

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );
    
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static bool MsmqPathNameToMsmqQueueFormatName( const std::wstring& pathName   ,
                                                   std::wstring& queueFormatName  );

    static bool MsmqPathNameToMsmqQueueFormatName( const CORE::CString& pathName   ,
                                                   CORE::CString& queueFormatName  );

    static bool MsmqQueueGUIDToMsmqQueueFormatName( const GUID& queueGuid ,
                                                    std::wstring& queueFormatName  );

    static bool MsmqQueueGUIDToMsmqQueueFormatName( const CORE::CString& queueGuid ,
                                                    std::wstring& queueFormatName  );

    static bool MsmqQueueGUIDToMsmqQueueFormatName( const CORE::CString& queueGuid ,
                                                    CORE::CString& queueFormatName );

    static bool MsmqGUIDToString( const GUID& guid               ,
                                  CORE::CAsciiString& guidString );

    static bool StringToMsmqGUID( const CORE::CAsciiString& guidString ,
                                  GUID& guid                           );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void Shutdown( void );
    
    protected:

    virtual MT::TLockStatus Lock( CORE::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMsmqPubSubClientTopic > TEventCallback;

    // Types to implement/hook-up topic interface
    typedef GUCEF::vector< PUBSUB::CBasicPubSubMsg > TPubSubMsgsVector;
    
    // Types used as caches to prevent ongoing memory allocations
    typedef GUCEF::vector< MQPROPVARIANT >    MQPROPVARIANTVector;
    typedef GUCEF::vector< MSGPROPID >        MSGPROPIDVector;
    typedef GUCEF::vector< PROPVARIANT >      PROPVARIANTVector;
    typedef GUCEF::vector< HRESULT >          HRESULTVector;    
    typedef GUCEF::map< MSGPROPID, CORE::CDynamicBuffer > MSGPROPIDToBufferMap;

    typedef CMsmqPubSubClientTopicConfig::MSGPROPIDMapVector    MSGPROPIDMapVector;

    struct SMsmqMsg
    {
        MQMSGPROPS msgprops;
        MSGPROPIDVector aMsgPropId;
        PROPVARIANTVector aMsgPropVar;
        HRESULTVector aStatus;
        MSGPROPIDToBufferMap propBuffers;
    };
    typedef struct SMsmqMsg TMsmqMsg;
    typedef GUCEF::vector< TMsmqMsg >         MsmqMsgVector;
    typedef GUCEF::map< MSGPROPID, CORE::UInt32 > MSGPROPIDToUInt32Map;

    private:

    enum EMsmqHresultSeverityCode : char
    {
        Success       = 0,  // bits 00
        Informational = 1,  // bits 01
        Warning       = 10, // bits 10
        Error         = 11, // bits 11
    };
    typedef enum EMsmqHresultSeverityCode TMsmqHresultSeverityCode;

    void RegisterEventHandlers( void );

    bool PrepStorageForReadMsgs( CORE::UInt32 msgCount );
    bool PrepMsmqMsgsStorage( void );
    bool PrepMsmqMsgStorage( TMsmqMsg& msg, MSGPROPIDVector& msmqPropIdsToUse, bool force );
    bool PrepMsmqVariantStorageForProperty( PROPID propertyId, MQPROPVARIANT& msmqVariant, TMsmqMsg& msgData, bool relinkBufferOnly );
    bool PrepMsmqVariantStorageForProperty( PROPID propertyId, TMsmqMsg& msgData, bool relinkBufferOnly );
    bool PrepMsmqPropIdToPropIndexMap( MSGPROPIDToUInt32Map& propIndexMapToBuild, const MSGPROPIDVector& msmqPropIdsToUse );

    bool SetUInt32OnPropertyVariant( PROPID propertyId, CORE::UInt32 valueToSet, TMsmqMsg& msgData );

    static VARTYPE GetMsmqVariantTypeForMsmqProperty( PROPID propertyId );
    static PROPID GetPayloadPropertyForPayloadSizeProperty( PROPID payloadSizePropId );
    static PROPID GetPayloadSizePropertyForPayloadProperty( PROPID payloadPropId );
    static TMsmqHresultSeverityCode ExtractSeverityCode( HRESULT code );
    CORE::CVariant GetMsmqPropertyValue( const MQMSGPROPS& msg, MSGPROPID propertyId );


    bool SetupToSubscribe( PUBSUB::CPubSubClientTopicConfig& config );

    static CORE::CString GenerateMetricsFriendlyTopicName( const CORE::CString& topicName );

    void NotifyOfReceivedMsgs( void );

    void UpdateIsHealthyStatus( bool newStatus );
    
    void
    OnSyncReadTimerCycle( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void
    OnReconnectTimerCycle( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );
    
    void
    OnPulseCycle( CORE::CNotifier* notifier    ,
                  const CORE::CEvent& eventId  ,
                  CORE::CICloneable* eventData );

    void OnMsmqMsgReceived( const MQMSGPROPS& msg, CORE::UInt32 msgCycleIndex, bool linkIfPossible );

    bool OnMsmqMsgBufferTooSmall( TMsmqMsg& msgsData );

    static bool MsmqPropertyToVariantStatic( MQPROPVARIANT& msmqSourceVariant , 
                                             CORE::CVariant& targetVariant    , 
                                             bool linkIfPossible              , 
                                             CORE::UInt32 lengthIfApplicable  ,
                                             bool scanForNullTerminatorOnStrs ,
                                             bool convertMsmqClsIdToString    ,
                                             CORE::CAsciiString& guidStrCache );

    bool MsmqPropertyToVariant( MQPROPVARIANT& msmqSourceVariant     , 
                                CORE::CVariant& targetVariant        , 
                                bool linkIfPossible                  , 
                                CORE::UInt32 lengthIfApplicable = 0  );
    
    static bool VariantToMsmqProperty( MSGPROPID propId, const CORE::CVariant& sourceVariant, MQPROPVARIANT& msmqTargetVariant );
    static CORE::Int32 SizeOfVarType( VARTYPE vt );
    static CORE::Int32 GetIndexOfProperty( PROPID propertyId, TMsmqMsg& msgData );
   
    class MsmqQueueProperties
    {
        public:
        
        CORE::CString queueLabel;
        UInt64 quota;
        CORE::CString pathName;
        CORE::CString pathNameDNS;
        CORE::CString typeId;

        CORE::CString ToString( void ) const;

        MsmqQueueProperties( void );
    };

    class MsmqQueue
    {
        public:
        
        CORE::CString queueName;
        std::wstring msmqQueueFormatName;
        bool queueNameIsMsmqFormatName;
        CORE::CString metricFriendlyQueueName;
        MsmqQueueProperties queueProperties;
        bool isPrivateQueue;

        MsmqQueue( void );
    };
    typedef GUCEF::vector< MsmqQueue >    MsmqQueueVector;
    
    static const std::wstring& GetMsmqQueueFormatName( MsmqQueue& q );
    
    static CORE::Int64 GetQueueMetric( MsmqQueue& q                           , 
                                       const CORE::CString& metricDescription ,
                                       CORE::UInt32 propId                    ,
                                       CORE::UInt32 propType                  );
    
    static CORE::Int64 GetCurrentNrOfMessagesInQueue( MsmqQueue& q );

    static CORE::Int64 GetCurrentByteCountOfMesssagesInQueue( MsmqQueue& q );

    static CORE::Int64 GetCurrentNrOfMessagesInQueueJournal( MsmqQueue& q );

    static CORE::Int64 GetCurrentByteCountOfMesssagesInQueueJournal( MsmqQueue& q );

    static bool GetMsmqQueueProperty( const std::wstring& queueFormatName ,
                                      CORE::UInt32 propId                 ,
                                      CORE::UInt32 propType               ,
                                      CORE::CVariant& qProperty           );
        
    static bool GetMsmqQueueLabel( const std::wstring& queueFormatName  ,
                                   CORE::CString& queueLabel            );

    static bool GetMsmqQueuePathName( const std::wstring& queueFormatName  ,
                                      CORE::CString& queuePathName         );

    static bool GetMsmqQueuePathNameDNS( const std::wstring& queueFormatName  ,
                                         CORE::CString& queuePathNameDNS      );

    static bool GetMsmqQueueQuota( const std::wstring& queueFormatName ,
                                   UInt64& queueQuota                  );

    static bool GetMsmqQueueType( const std::wstring& queueFormatName ,
                                  CORE::CString& queueTypeId          );

    static bool GetMsmqQueueProperties( const std::wstring& queueFormatName  ,
                                        MsmqQueueProperties& queueProperties );
    
    static bool InitQueueInfo( MsmqQueue& q, const CMsmqPubSubClientTopicConfig& config );

    private:

    CMsmqPubSubClient* m_client;
    TPubSubMsgsVector m_pubsubMsgs;
    TMsgsRecievedEventData m_pubsubMsgsRefs;
    MsmqMsgVector m_msmqReceiveMsgs;
    TMsmqMsg m_msgSendMsg;
    MSGPROPIDToUInt32Map m_msgSendPropMap;
    CMsmqPubSubClientTopicConfig m_config;
    MsmqQueue m_queue;
    mutable std::wstring m_msmqQueueFormatName;
    CORE::CTimer* m_syncReadTimer;    
    CORE::CTimer* m_reconnectTimer;
    MT::CMutex m_lock;
    QUEUEHANDLE m_receiveQueueHandle;
    QUEUEHANDLE m_sendQueueHandle;
    CORE::UInt64 m_currentPublishActionId;
    CORE::UInt64 m_currentReceiveActionId;
    TPublishActionIdVector m_publishSuccessActionIds;
    TMsgsPublishedEventData m_publishSuccessActionEventData;
    TPublishActionIdVector m_publishFailureActionIds;
    TMsgsPublishFailureEventData m_publishFailureActionEventData;
    CORE::CAsciiString m_guidStr;
    CORE::UInt64 m_msmqMessagesPublished;
    CORE::UInt64 m_msmqErrorsOnPublish;
    CORE::UInt64 m_msmqMessagesReceived;
    CORE::UInt64 m_msmqErrorsOnReceive;
    CORE::UInt64 m_msmqErrorsOnAck;
    TopicMetrics m_metrics;
    UInt32Vector m_msmqMsgSentToArriveLatencies;
    CORE::UInt64 m_msmqLastLookupId;
    mutable bool m_isHealthy;
    bool m_subscriptionIsAtEndOfData;
};

/*-------------------------------------------------------------------------*/

std::wstring RetrieveWin32APIErrorMessage( DWORD dwErr );

typedef CMsmqPubSubClientTopic::TSharedPtrType  CMsmqPubSubClientTopicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MSMQ */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPIC_H ? */
