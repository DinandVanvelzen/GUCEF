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
#include <Mq.h>

#include <string>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H */

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENT_H
#include "pubsubpluginMSMQ_CMsmqPubSubClient.h"
#define PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENT_H ? */

#include "pubsubpluginMSMQ_CMsmqPubSubClientTopic.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

std::wstring 
RetrieveWin32APIErrorMessage( DWORD dwErr )
{
    // from: https://docs.microsoft.com/en-us/windows/win32/seccrypto/retrieving-error-messages

    WCHAR   wszMsgBuff[512];  // Buffer for text.

    DWORD   dwChars;  // Number of chars returned.

    // Try to get the message from the system errors.
    dwChars = ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL,
                                dwErr,
                                0,
                                wszMsgBuff,
                                512,
                                NULL );

    if (0 == dwChars)
    {
        // The error code did not exist in the system errors.
        // Try Ntdsbmsg.dll for the error code.

        HINSTANCE hInstNtdsbmsg = NULL;

        // Load the library.
        hInstNtdsbmsg = ::LoadLibraryA( "Ntdsbmsg.dll" );
        if ( NULL == hInstNtdsbmsg )
        {
            // cannot load Ntdsbmsg.dll for refinement
            return std::wstring();
        }

        // Try getting message text from ntdsbmsg.
        dwChars = FormatMessageW( FORMAT_MESSAGE_FROM_HMODULE |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                 hInstNtdsbmsg,
                                 dwErr,
                                 0,
                                 wszMsgBuff,
                                 512,
                                 NULL );

        // Free the library.
        ::FreeLibrary( hInstNtdsbmsg );

    }

    return wszMsgBuff;
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopic::CMsmqPubSubClientTopic( CMsmqPubSubClient* client )
    : PUBSUB::CPubSubClientTopic( client->GetPulseGenerator() )
    , CORE::CTSharedObjCreator< CMsmqPubSubClientTopic, MT::CMutex >( this )
    , m_client( client )
    , m_msgSendMsg()
    , m_config()
    , m_queue()
    , m_msmqQueueFormatName()
    , m_syncReadTimer( GUCEF_NULL )
    , m_reconnectTimer( GUCEF_NULL )
    , m_lock()
    , m_receiveQueueHandle( GUCEF_NULL )
    , m_sendQueueHandle( GUCEF_NULL )
    , m_currentPublishActionId( 1 )
    , m_currentReceiveActionId( 1 )
    , m_publishSuccessActionIds()
    , m_publishSuccessActionEventData()
    , m_publishFailureActionIds()
    , m_publishFailureActionEventData()
    , m_guidStr()
    , m_msmqMessagesPublished( 0 )
    , m_msmqErrorsOnPublish( 0 )
    , m_msmqMessagesReceived( 0 )
    , m_msmqErrorsOnReceive( 0 )
    , m_msmqErrorsOnAck( 0 )
    , m_metrics()
    , m_msmqMsgSentToArriveLatencies()
    , m_msmqLastLookupId( 0 )
    , m_isHealthy( true )
    , m_subscriptionIsAtEndOfData( false )
{GUCEF_TRACE;
        
    m_publishSuccessActionEventData.LinkTo( &m_publishSuccessActionIds );
    m_publishFailureActionEventData.LinkTo( &m_publishFailureActionIds );

    m_syncReadTimer = GUCEF_NEW CORE::CTimer( client->GetPulseGenerator(), 25 );

    if ( GUCEF_NULL != m_client && m_client->GetConfig().desiredFeatures.supportsAutoReconnect )
    {
        m_reconnectTimer = GUCEF_NEW CORE::CTimer( client->GetPulseGenerator(), m_client->GetConfig().reconnectDelayInMs );
    }

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopic::~CMsmqPubSubClientTopic()
{GUCEF_TRACE;

    Shutdown();    
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    m_client = GUCEF_NULL;

    Disconnect();

    GUCEF_DELETE m_syncReadTimer;
    m_syncReadTimer = GUCEF_NULL;

    GUCEF_DELETE m_reconnectTimer;
    m_reconnectTimer = GUCEF_NULL;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopic::MsmqQueue::MsmqQueue( void )
    : queueName()
    , msmqQueueFormatName()
    , queueNameIsMsmqFormatName( false )
    , metricFriendlyQueueName()
    , isPrivateQueue( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopic::MsmqQueueProperties::MsmqQueueProperties( void )
    : queueLabel()
    , quota( 0 )
    , pathName()
    , pathNameDNS()
    , typeId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString 
CMsmqPubSubClientTopic::MsmqQueueProperties::ToString( void ) const
{GUCEF_TRACE;

    return "queueLabel=" + queueLabel + ", typeId=" + typeId + ", quota=" + CORE::ToString( quota ) + ", pathName=" + pathName + ", pathNameDNS=" + pathNameDNS;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CMsmqPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopic::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_syncReadTimer )
    {
        TEventCallback callback( this, &CMsmqPubSubClientTopic::OnSyncReadTimerCycle );
        SubscribeTo( m_syncReadTimer                ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    if ( GUCEF_NULL != m_reconnectTimer )
    {
        TEventCallback callback( this, &CMsmqPubSubClientTopic::OnReconnectTimerCycle );
        SubscribeTo( m_reconnectTimer               ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    if ( GUCEF_NULL != m_client )
    {
        TEventCallback callback( this, &CMsmqPubSubClientTopic::OnPulseCycle );
        SubscribeTo( m_client->GetConfig().pulseGenerator.GetPointerAlways() ,
                     CORE::CPulseGenerator::PulseEvent                       ,
                     callback                                                );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;
    
    return m_config.needPublishSupport;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;

    return m_config.needSubscribeSupport;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CMsmqPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.topicName;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    if ( 0 == publishActionId )
    {
        publishActionId = m_currentPublishActionId; 
        ++m_currentPublishActionId;
    }

    // Each Message Queuing message can have no more than 4 MB of data.
    // We cannot do anything with a DateTime already set on the message since MSMQ fully controls the timestamps

    // Do we want to send a body per config?
    MSGPROPIDToUInt32Map::iterator n = m_msgSendPropMap.find( PROPID_M_BODY );
    if ( n != m_msgSendPropMap.end() )
    {
        // Do we have a body to send?
        const CORE::CVariant& primaryPayload = msg.GetPrimaryPayload();
        
        bool variantConversionSuccess = false; 
        if ( primaryPayload.IsInitialized() )
            variantConversionSuccess = VariantToMsmqProperty( PROPID_M_BODY, primaryPayload, m_msgSendMsg.aMsgPropVar[ (*n).second ] );
        else
            variantConversionSuccess = VariantToMsmqProperty( PROPID_M_BODY, CORE::CVariant::Empty, m_msgSendMsg.aMsgPropVar[ (*n).second ] );

        if ( !variantConversionSuccess )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Publish: Cannot translate IPubSubMsg primary payload to a MSMQ BODY" ); 
            return false;
        }

        SetUInt32OnPropertyVariant( PROPID_M_BODY_SIZE, primaryPayload.ByteSize(), m_msgSendMsg );
    }
    
    const PUBSUB::CIPubSubMsg::TKeyValuePairs& kvPairs = msg.GetKeyValuePairs();
    //@TODO: Find a way to handle these driven by config

    const PUBSUB::CIPubSubMsg::TKeyValuePairs& metaDataKvPairs = msg.GetMetaDataKeyValuePairs();
    PUBSUB::CIPubSubMsg::TKeyValuePairs::const_iterator i = metaDataKvPairs.begin();
    while ( i != metaDataKvPairs.end() )
    {
        MSGPROPID propId = (MSGPROPID) (*i).first.AsUInt32();
        MSGPROPIDToUInt32Map::iterator n = m_msgSendPropMap.find( propId );
        if ( n != m_msgSendPropMap.end() )
        {
            if ( !VariantToMsmqProperty( propId, (*i).second, m_msgSendMsg.aMsgPropVar[ (*n).second ] ) ) 
            {   
                // Cannot map this property's value
                if ( !m_config.ignoreUnmappableMetaDataFieldOnPublish )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Publish: Cannot translate IPubSubMsg meta-data value to a configured MSMQ VARIANT. MSGPROPID=" + CORE::ToString( (CORE::UInt32) propId ) ); 
                    return false;
                }
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Publish: Cannot translate IPubSubMsg meta-data value to a configured MSMQ VARIANT. Ignoring it. MSGPROPID=" + CORE::ToString( (CORE::UInt32) propId ) ); 
            }
        }
        else
        {
            // Cannot map this property
            
            if ( !m_config.ignoreUnmappableMetaDataFieldOnPublish )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Publish: Cannot map IPubSubMsg meta-data key to a configured MSMQ field. MSGPROPID=" + CORE::ToString( (CORE::UInt32) propId ) ); 
                return false;
            }
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Publish: Cannot map IPubSubMsg meta-data key to a configured MSMQ field. Ignoring it. MSGPROPID=" + CORE::ToString( (CORE::UInt32) propId ) ); 
        }
        ++i;
    }
    
    bool success = false;
    
    if ( GUCEF_NULL == m_sendQueueHandle )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Publish: No send queue handle, will try to auto-subscribe" );
        if ( !Subscribe() )
        {
            GUCEF_ERROR_LOG(CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Publish: Cannot publish message, failed to subscribe" );
            return false;
        }        
    }
    
    if ( GUCEF_NULL != m_sendQueueHandle )
    {
        // Send message
        HRESULT sendMsgResult = ::MQSendMessage( m_sendQueueHandle, &m_msgSendMsg.msgprops, NULL );

        if ( !FAILED( sendMsgResult ) )
        {
            success = true;
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Publish: Successfully published msg to \"" + m_queue.queueName + "\" with publishActionId=" + CORE::ToString( publishActionId ) ); 

            if ( notify )
            {
                m_publishSuccessActionIds.push_back( publishActionId );
            }
            return true;
        }
        else
        {
            CORE::UInt32 errorCode =  HRESULT_CODE( sendMsgResult );
            std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Publish: Failed to publish to MSMQ queue " + m_queue.queueName + ". HRESULT=" + CORE::ToString( sendMsgResult ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 

            if ( notify )
            {
                m_publishFailureActionIds.push_back( publishActionId );
            }
        }
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::SetupToSubscribe( PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( LoadConfig( config ) )
    {
        InitQueueInfo( m_queue, config );
        PrepStorageForReadMsgs( m_config.maxMsmqMsgsToReadPerSyncCycle );    
        m_syncReadTimer->SetEnabled( true );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    config = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMsmqPubSubClientTopic::GenerateMetricsFriendlyTopicName( const CORE::CString& topicName )
{GUCEF_TRACE;

    // Let's avoid non-ASCII stumbling blocks and force the down to ASCII
    CORE::CAsciiString asciiMetricsFriendlyTopicName = topicName.ForceToAscii( '_' );
    
    // Replace special chars
    static const char disallowedChars[] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '=', '+', ',', '.', '<', '>', '/', '?', '`', '~', '\\', '|', '{', '}', '[', ']', ';', ':', '\'', '\"' };
    asciiMetricsFriendlyTopicName = asciiMetricsFriendlyTopicName.ReplaceChars( disallowedChars, sizeof(disallowedChars)/sizeof(char), '_' );

    // Back to the platform wide string convention format
    CORE::CString metricsFriendlyTopicName = CORE::ToString( asciiMetricsFriendlyTopicName );
    return metricsFriendlyTopicName;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    m_config = config;
    return InitQueueInfo( m_queue, config );
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::PrepStorageForReadMsgs( CORE::UInt32 msgCount )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:PrepStorageForReadMsgs: Allocating storage to read " + CORE::ToString( msgCount ) + " messages per batch" );
    
    bool failedToAllocate = false;
    do
    {
        try
        {
            // reset size, note this does not dealloc the underlying memory
            TPubSubMsgsRefVector& msgRefs = m_pubsubMsgsRefs;
            msgRefs.clear();

            if ( msgCount > msgRefs.capacity() )
            {
                // Grow the actual storage, this is allowed to become larger than msgCount to limit
                // the nr of dynamic allocations
                msgRefs.reserve( msgCount );
            }

            if ( msgCount > m_pubsubMsgs.size() )
            {
                // Grow the actual storage plus count. These slots are pre-reserved to limit the nr of dynamic allocations
                // This helps make the code more performant
                m_pubsubMsgs.resize( msgCount );        
                m_msmqReceiveMsgs.resize( msgCount );
                if ( !PrepMsmqMsgsStorage() )
                    failedToAllocate = true;    
            }
        }
        catch ( const std::bad_alloc& )
        {
            failedToAllocate = true;
        }

        if ( failedToAllocate )
        {
            // First release all the memory, we dont know where we failed
            // get back to a clean/consistent state
            m_pubsubMsgsRefs.resize( 0 );
            m_pubsubMsgs.resize( 0 );
            m_msmqReceiveMsgs.resize( 0 );

            // not really on receive/publish but we want to draw attention to this issue
            ++m_msmqErrorsOnReceive;
            ++m_msmqErrorsOnPublish;

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqPubSubClientTopic:PrepStorageForReadMsgs: Failed to allocate enough memory storage for all message in max batch size (" + CORE::ToString( msgCount ) + "), reducing batch size by half" );
                
            msgCount /= 2;
            if ( 1 >= msgCount )
                return false;
        }
    }
    while ( failedToAllocate );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_syncReadTimer->SetEnabled( false );

    if ( GUCEF_NULL != m_receiveQueueHandle )
    {
        ::MQCloseQueue( m_receiveQueueHandle );
        m_receiveQueueHandle = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_sendQueueHandle )
    {
        ::MQCloseQueue( m_sendQueueHandle );
        m_sendQueueHandle = GUCEF_NULL;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::BeginReconnectSequence( const CORE::CEvent* eventMsg )
{GUCEF_TRACE;
    
    if ( Disconnect() )
    {
        {
            MT::CScopeMutex lock( m_lock );
            
            if ( GUCEF_NULL == m_client )
                return false;

            // Make sure to notify of any messages received already
            // If we are using the peek behavior you would not lose messages via reconnect but you would lost more time if it happens at a semi-regular interval
            // If you are not using the peeks then you'd actually lose messages if we dont do this step since the new subscribe will reinitialize everything and thus wipe messages
            NotifyOfReceivedMsgs();
            
            if ( m_client->GetConfig().desiredFeatures.supportsAutoReconnect && GUCEF_NULL != m_reconnectTimer )
            {
                m_reconnectTimer->SetEnabled( true );
            }
        }

        if ( GUCEF_NULL != eventMsg )
        {
            NotifyObservers( *eventMsg );
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::MsmqPathNameToMsmqQueueFormatName( const std::wstring& pathName   ,
                                                           std::wstring& queueFormatName  )
{GUCEF_TRACE;

    DWORD maxFormatNameSize = 512;
    WCHAR queueFormatNameTmp[ 512 ]; 
    HRESULT formatConversionResult = ::MQPathNameToFormatName( pathName.c_str()    ,    
                                                               queueFormatNameTmp  ,    
                                                               &maxFormatNameSize  );
    if ( !FAILED( formatConversionResult ) )
    {
        queueFormatName = queueFormatNameTmp;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:MsmqPathNameToMsmqQueueFormatName: Converted pathName \"" + 
            CORE::ToString( pathName ) + "\" to \"" + CORE::ToString( queueFormatName ) + "\"" ) ; 
        return true;
    }
    else
    {
        CORE::UInt32 errorCode =  HRESULT_CODE( formatConversionResult );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:MsmqPathNameToMsmqQueueFormatName: Failed to convert path \"" + 
            CORE::ToString( pathName ) + "\" to MSMQ format name. HRESULT=" + CORE::ToString( formatConversionResult ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::MsmqPathNameToMsmqQueueFormatName( const CORE::CString& pathName   ,
                                                           CORE::CString& queueFormatName  )
{GUCEF_TRACE;

    std::wstring wPathName;
    if ( CORE::Utf8toUtf16( CORE::ToUtf8String( pathName ), wPathName ) )
    {
        std::wstring wQueueFormatName;
        if ( MsmqPathNameToMsmqQueueFormatName( wPathName, wQueueFormatName ) )
        {
            queueFormatName = CORE::ToString( wQueueFormatName );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::MsmqQueueGUIDToMsmqQueueFormatName( const GUID& queueGuid          ,
                                                            std::wstring& queueFormatName  )
{GUCEF_TRACE;

    DWORD maxFormatNameSize = 512;
    WCHAR queueFormatNameTmp[ 512 ]; 
    HRESULT formatConversionResult = ::MQInstanceToFormatName( const_cast< GUID* >( &queueGuid ) ,    
                                                               queueFormatNameTmp                ,    
                                                               &maxFormatNameSize                );
    if ( !FAILED( formatConversionResult ) )
    {
        queueFormatName = queueFormatNameTmp;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:MsmqQueueGUIDToMsmqQueueFormatName: Converted GUID structure to \"" + 
            CORE::ToString( queueFormatName ) + "\"" ) ; 
        return true;
    }
    else
    {
        CORE::UInt32 errorCode =  HRESULT_CODE( formatConversionResult );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:MsmqQueueGUIDToMsmqQueueFormatName: Failed to convert GUID to MSMQ format name. HRESULT=" + 
            CORE::ToString( formatConversionResult ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
        return false;
    }
    
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::MsmqQueueGUIDToMsmqQueueFormatName( const CORE::CString& queueGuid ,
                                                            std::wstring& queueFormatName  )
{GUCEF_TRACE;

    CORE::CAsciiString asciiGuidStr = queueGuid.ForceToAscii( 0 );
    if ( asciiGuidStr.DetermineLength() != queueGuid.Length() )
    {
        CORE::CAsciiString asciiGuidStrLogExample = queueGuid.ForceToAscii( '*' );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:MsmqQueueGUIDToMsmqQueueFormatName: Code point count difference while converting queueGuid \"" + 
            queueGuid + "\" to ASCII representation. Replacing non-ASCII with '*' yields: " + asciiGuidStrLogExample ) ; 
    }

    GUID queueGuidStruct;
    if ( StringToMsmqGUID( asciiGuidStr, queueGuidStruct ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:MsmqQueueGUIDToMsmqQueueFormatName: Converted queueGuid string \"" + 
            queueGuid + "\" to a GUID structure successfully" ); 

        return MsmqQueueGUIDToMsmqQueueFormatName( queueGuidStruct, queueFormatName );
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:MsmqQueueGUIDToMsmqQueueFormatName: Failed to convert queueGuid string \"" + 
        queueGuid + "\" to a GUID structure" ); 
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::MsmqQueueGUIDToMsmqQueueFormatName( const CORE::CString& queueGuid ,
                                                            CORE::CString& queueFormatName )
{GUCEF_TRACE;

    std::wstring wQueueFormatName;
    if ( MsmqQueueGUIDToMsmqQueueFormatName( queueGuid, wQueueFormatName ) )
    {
        queueFormatName = CORE::ToString( wQueueFormatName );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const std::wstring&
CMsmqPubSubClientTopic::GetMsmqQueueFormatName( MsmqQueue& q )
{
    if ( !q.msmqQueueFormatName.empty() )
        return q.msmqQueueFormatName;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:GetMsmqQueueFormatName: MSMQ queue 'format name' is not yet set, attempting to obtain it. Queue name =" + q.queueName ) ; 

    std::wstring wQueueName;
    CORE::Utf8toUtf16( q.queueName, wQueueName );

    // Should we try and take the string as given and not do anything fancy?
    if ( !q.queueNameIsMsmqFormatName )  
    {
        bool isSuspectedQueueGuid = ( 0 == q.queueName.HasChar( '{', true ) && 0 < q.queueName.HasChar( '}', true ) );
        if ( isSuspectedQueueGuid )
        {
            std::wstring wQueueFormatName;
            if ( MsmqQueueGUIDToMsmqQueueFormatName( q.queueName, wQueueFormatName ) )
            {
                q.msmqQueueFormatName = wQueueFormatName;
            }
        }
        else
        {
            std::wstring wQueueFormatName;
            if ( MsmqPathNameToMsmqQueueFormatName( wQueueName, wQueueFormatName ) )
            {
                q.msmqQueueFormatName = wQueueFormatName;
            }
        }
    }
    else
    {
        q.msmqQueueFormatName = wQueueName;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:GetMsmqQueueFormatName: Since the topic name is flagged as a format name we will use it as-is. format name aka queue aka Topic =" + q.queueName );
    }

    return q.msmqQueueFormatName; 
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL != m_receiveQueueHandle )
    {
        ::MQCloseQueue( m_receiveQueueHandle );
        m_receiveQueueHandle = GUCEF_NULL;
    }

    InitQueueInfo( m_queue, m_config );

    // MQOpenQueue only supports a format name
    // As such depending on how the setting is presented we need to translate it
    const std::wstring& queueFormatName = GetMsmqQueueFormatName( m_queue );
    if ( queueFormatName.empty() )
        return false;

    ///* MQOpenQueue() access flags */
    //#define MQ_RECEIVE_ACCESS   0x00000001L
    //#define MQ_SEND_ACCESS      0x00000002L
    //#define MQ_MOVE_ACCESS      0x00000004L
    //#define MQ_PEEK_ACCESS      0x00000020L
    //#define MQ_ADMIN_ACCESS     0x00000080L

    HRESULT openQueueResult = ::MQOpenQueue( queueFormatName.c_str() ,  // IN:  Queue format name
                                             MQ_RECEIVE_ACCESS       ,  // IN:  Want to receive from queue
                                             0                       ,  // IN:  Allow sharing
                                             &m_receiveQueueHandle   ); // OUT: Handle of open queue

    if ( FAILED( openQueueResult ) )
    {
        CORE::UInt32 errorCode =  HRESULT_CODE( openQueueResult );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Subscribe: Failed to 'subscribe' by opening a MSMQ queue for recieving. Topic Name: " + 
            m_config.topicName + ". HRESULT= " + CORE::ToString( openQueueResult ) + ". Code segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
        BeginReconnectSequence( &ConnectionErrorEvent );
        return false;
    }

    if ( !PrepStorageForReadMsgs( m_config.maxMsmqMsgsToReadPerSyncCycle ) )
    {        
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Subscribe: Failed to init memory storage for publishing. Topic Name: " + m_config.topicName );
        BeginReconnectSequence( &InternalErrorEvent );
        return false;
    }
    
    // Initialize getting all messages in queue
    // this needs the proper account access level. if this failed we will not try again in the metrics timer cycle
    m_metrics.msmqMsgsInQueue = GetCurrentNrOfMessagesInQueue( m_queue );

    // Enable the timer to start throttled sync reading 
    m_syncReadTimer->SetEnabled( true );
    
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:Subscribe: Successfully 'subscribed to topic' for MSMQ queue. Topic Name: " + m_config.topicName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::SubscribeStartingAtMsgId( const CORE::CVariant& msgIdBookmark )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    // Currently not supported.
    // Could theoretically be implemented using a cursor combined with PEEK only reads
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::SubscribeStartingAtMsgDateTime( const CORE::CDateTime& msgDtBookmark )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    // Currently not supported.
    // Could theoretically be implemented using a cursor combined with PEEK only reads
    // For regular queue consuming reads you could perhaps ignore messages with a send time older then msgDtBookmark
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark ) 
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )

    MT::CScopeMutex lock( m_lock );
    if ( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX == bookmark.GetBookmarkType() )
    {
        m_msmqLastLookupId = bookmark.GetBookmarkData().AsUInt64();
        return Subscribe();
    }
    return false;

    #else

    // Currently not supported.
    return false;
    
    #endif
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubBookmark 
CMsmqPubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )

    MT::CScopeMutex lock( m_lock );
    return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX, CORE::CVariant( m_msmqLastLookupId ) );

    #else

    // Not supported
    return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_APPLICABLE );
    
    #endif
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::DeriveBookmarkFromMsg( const PUBSUB::CIPubSubMsg& msg, PUBSUB::CPubSubBookmark& bookmark ) const
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )

    bookmark.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX );
    bookmark.SetBookmarkData( msg.GetMsgIndex() );
    return true;

    #else

    return false; 

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg )
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )

    PUBSUB::CPubSubBookmark bookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX, msg.GetMsgIndex() );
    return AcknowledgeReceiptImpl( bookmark, msg.GetReceiveActionId() );

    #else

    // Since we read messages in a destructive manner, no peeking, treat as FYI no-op
    return true; 

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;
    
    return AcknowledgeReceiptImpl( bookmark, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::AcknowledgeReceiptImpl( const PUBSUB::CPubSubBookmark& bookmark , 
                                                CORE::UInt64 receiveActionId             )
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )

    if ( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX != bookmark.GetBookmarkType() )
        return false;
    
    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
        return false;

    const CMsmqPubSubClientConfig& clientConfig = m_client->GetConfig();
    bool supportsAcksViaLookup = clientConfig.simulateReceiveAckFeatureViaLookupId && clientConfig.desiredFeatures.supportsSubscriberMsgReceivedAck; 
    if ( supportsAcksViaLookup )
    {
        CORE::UInt64 lookupId = bookmark.GetBookmarkData().AsUInt64();
        if ( 0 != lookupId )
        {
            HRESULT receiveHResult = ::MQReceiveMessageByLookupId( m_receiveQueueHandle      ,        // Handle to the queue 
                                                                   lookupId                  ,        // The lookup identifier  
                                                                   MQ_LOOKUP_RECEIVE_CURRENT ,        // Access mode: There is no delete per se, we 'receive' the message to delete it
                                                                   NULL                      ,        // Normally pointer to the MQMSGPROPS structure, we dont care since we care deleting thus NULL
                                                                   NULL                      ,        // No OVERLAPPED structure  
                                                                   NULL                      ,        // No callback function  
                                                                   NULL                      );       // Not in a transaction  

            CORE::UInt64 receiveResult = (CORE::UInt64) receiveHResult;
            switch ( receiveResult )
            {
                case (CORE::UInt64) MQ_OK:
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::AcknowledgeReceipt: Successfully removed message with msg index (LookupId) " + CORE::ToString( lookupId ) + 
                        ". receiveActionId=" + CORE::ToString( receiveActionId ) );
                    return true;
                }
                case (CORE::UInt64) MQ_ERROR_MESSAGE_NOT_FOUND:
                {
                    // Per docs: "The message referenced by the lookup identifier does not exist. Either the lookup identifier is incorrect or the message has been removed from the queue."
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::AcknowledgeReceipt: Message with msg index (LookupId) " + CORE::ToString( lookupId ) + 
                        " cannot be found, we will treat this as successfully removed. receiveActionId=" + CORE::ToString( receiveActionId ) );
                    return true;
                }

                case (CORE::UInt64) MQ_ERROR_QUEUE_DELETED:        // <- queue was deleted outside the app, just keep trying to 'reconnect'
                {
                    // If the queue is deleted the lookup id becomes invalid since its a property of that instance of the queue
                    m_msmqLastLookupId = 0;

                    // we intentiontionally dont break here because we want the code below as well
                    // break;
                }
                case (CORE::UInt64) MQ_ERROR_QUEUE_NOT_ACTIVE:     // <- caused by someone messing with the queue outside the app?
                case (CORE::UInt64) MQ_ERROR_QUEUE_NOT_FOUND:      // <- caused by someone messing with the queue outside the app?
                case (CORE::UInt64) MQ_ERROR_INVALID_HANDLE:
                case (CORE::UInt64) MQ_ERROR_OPERATION_CANCELLED:  // <- caused by handle becoming invalid as well? Per docs: "For example, the queue handle was closed by another thread while waiting for a message"
                case (CORE::UInt64) MQ_ERROR_INVALID_PARAMETER:    // <- caused by handle becoming invalid as well?
                {
                    ++m_msmqErrorsOnAck;                

                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::AcknowledgeReceipt: Queue Access issue removing message with msg index (LookupId) " + CORE::ToString( lookupId ) +
                        ". HRESULT= " + CORE::Base16Encode( &receiveHResult, sizeof(receiveHResult) ) + 
                        ". Win32 Error message: " + CORE::ToString( RetrieveWin32APIErrorMessage( HRESULT_CODE( receiveHResult ) ) ) );

                    lock.EarlyUnlock();
                    BeginReconnectSequence( &ConnectionErrorEvent );
                    return false;
                }

                default:
                {
                    ++m_msmqErrorsOnAck;

                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::AcknowledgeReceipt: Unhandled issue removing message with msg index (LookupId) " + CORE::ToString( lookupId ) +
                        ". receiveActionId=" + CORE::ToString( receiveActionId ) +
                        ". HRESULT= " + CORE::Base16Encode( &receiveHResult, sizeof(receiveHResult) ) + 
                        ". Win32 Error message: " + CORE::ToString( RetrieveWin32APIErrorMessage( HRESULT_CODE( receiveHResult ) ) ) );          

                    lock.EarlyUnlock();
                    BeginReconnectSequence( &InternalErrorEvent );
                    return false;
                }
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::AcknowledgeReceipt: Cannot remove message with invalid init msg index (LookupId) 0. receiveActionId=" + CORE::ToString( receiveActionId ) );
        }
    }

    return false;

    #else

    // Since we read messages in a destructive manner, no peeking, treat as FYI no-op
    return true;

    #endif 
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
           
    if ( m_config.needPublishSupport && GUCEF_NULL == m_sendQueueHandle )
        return false;

    if ( m_config.needSubscribeSupport && GUCEF_NULL == m_receiveQueueHandle )
        return false;

    return ( m_config.needPublishSupport && GUCEF_NULL != m_sendQueueHandle ) && ( m_config.needSubscribeSupport && GUCEF_NULL != m_receiveQueueHandle );
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;

    // Snapshot in time as such no lock needed.
    return GUCEF_NULL != m_receiveQueueHandle;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    bool newIsHealthyState = true;
    
    if ( m_config.maxMsmqErrorsOnAckToBeHealthy >= 0 )
    {
        // Current and last metrics cycle error count counts against the max
        if ( m_msmqErrorsOnAck + m_metrics.msmqErrorsOnAck > (CORE::UInt32) m_config.maxMsmqErrorsOnAckToBeHealthy )
            newIsHealthyState = false;
    }
    if ( newIsHealthyState && m_config.maxMsmqErrorsOnReceiveToBeHealthy >= 0 )
    {
        // Current and last metrics cycle error count counts against the max
        if ( m_msmqErrorsOnReceive + m_metrics.msmqErrorsOnReceive > (CORE::UInt32) m_config.maxMsmqErrorsOnReceiveToBeHealthy )
            newIsHealthyState = false;
    }
    
    // If we are having to 'reconnect' than we are not healthy
    if ( newIsHealthyState && GUCEF_NULL != m_reconnectTimer && m_reconnectTimer->GetEnabled() )
        newIsHealthyState = false;

    if ( newIsHealthyState != m_isHealthy )
    {
        m_isHealthy = newIsHealthyState;

        if ( m_isHealthy )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::IsHealthy: Topic " + m_config.topicName + " is now healthy" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::IsHealthy: Topic " + m_config.topicName + " is now unhealthy" );         
        }

        lock.EarlyUnlock();
        THealthStatusChangeEventData eData( newIsHealthyState ); 
        NotifyObservers( HealthStatusChangeEvent, &eData ); 
    }
    
    return newIsHealthyState;
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopic::UpdateIsHealthyStatus( bool newStatus )
{GUCEF_TRACE;

    // See if this is a change from the current persisted state
    if ( newStatus != m_isHealthy )
    {
        // Make sure we use the same consistent logic to determine health
        // the caller might only know about one aspect of health
        // The IsHealthy check itself will update the status and perform eventing if needed
        newStatus = IsHealthy();
    }    
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::InitializeConnectivity( bool reset )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_config.needPublishSupport )
    {
        if ( !reset && GUCEF_NULL != m_sendQueueHandle )
            return true;
        
        if ( GUCEF_NULL != m_sendQueueHandle )
        {
            ::MQCloseQueue( m_sendQueueHandle );
            m_sendQueueHandle = GUCEF_NULL;
        }

        // MQOpenQueue only supports a format name
        // As such depending on how the setting is presented we need to translate it
        const std::wstring& queueFormatName = GetMsmqQueueFormatName( m_queue );
        if ( queueFormatName.empty() )
            return false;

        // Opening the handle counts as our 'connectivity'
        HRESULT openQueueResult = ::MQOpenQueue( queueFormatName.c_str() ,  // IN:  Queue format name
                                                 MQ_SEND_ACCESS          ,  // IN:  Want to send to queue
                                                 0                       ,  // IN:  Allow sharing
                                                 &m_sendQueueHandle      ); // OUT: Handle of open queue

        if ( FAILED( openQueueResult ) )
        {
            CORE::UInt32 errorCode =  HRESULT_CODE( openQueueResult );
            std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( openQueueResult ) );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:InitializeConnectivity: Failed to init queue access for publishing. Topic Name: " + m_config.topicName +". errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
            
            lock.EarlyUnlock();
            BeginReconnectSequence( &ConnectionErrorEvent );
            return false;
        }
        else
        {
            if ( !PrepMsmqMsgsStorage() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:InitializeConnectivity: Failed to init memory storage for publishing. Topic Name: " + m_config.topicName );

                lock.EarlyUnlock();
                BeginReconnectSequence( &InternalErrorEvent );
                return false;
            }
        }

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:InitializeConnectivity: Successfully initialized for publishing. Topic Name: " + m_config.topicName );
    }

    return true;
}

/*-------------------------------------------------------------------------*/
   
// From: https://stackoverflow.com/questions/45149620/convert-ole-automation-date-oadate-double-to-struct-tm-without-using-variantti
// Slightly altered: used platform types to address ULL type concern

struct tm* SecondsSinceEpochToDateTime(struct tm* pTm, uint64_t SecondsSinceEpoch)
{
   uint64_t sec;
   unsigned int quadricentennials, centennials, quadrennials, annuals/*1-ennial?*/;
   unsigned int year, leap;
   unsigned int yday, hour, min;
   unsigned int month, mday, wday;
   static const unsigned int daysSinceJan1st[2][13]=
   {
      {0,31,59,90,120,151,181,212,243,273,304,334,365}, // 365 days, non-leap
      {0,31,60,91,121,152,182,213,244,274,305,335,366}  // 366 days, leap
  };
/*
   400 years:

   1st hundred, starting immediately after a leap year that's a multiple of 400:
   n n n l  \
   n n n l   } 24 times
   ...      /
   n n n l /
   n n n n

   2nd hundred:
   n n n l  \
   n n n l   } 24 times
   ...      /
   n n n l /
   n n n n

   3rd hundred:
   n n n l  \
   n n n l   } 24 times
   ...      /
   n n n l /
   n n n n

   4th hundred:
   n n n l  \
   n n n l   } 24 times
   ...      /
   n n n l /
   n n n L <- 97'th leap year every 400 years
*/

   // Re-bias from 1970 to 1601:
   // 1970 - 1601 = 369 = 3*100 + 17*4 + 1 years (incl. 89 leap days) =
   // (3*100*(365+24/100) + 17*4*(365+1/4) + 1*365)*24*3600 seconds
   sec = SecondsSinceEpoch + 11644473600ULL;

   wday = (CORE::UInt32)((sec / 86400 + 1) % 7); // day of week

   // Remove multiples of 400 years (incl. 97 leap days)
   quadricentennials = (CORE::UInt32)(sec / CORE::UInt64( 12622780800ULL ) ); // 400*365.2425*24*3600
   sec %= CORE::UInt64( 12622780800ULL );

   // Remove multiples of 100 years (incl. 24 leap days), can't be more than 3
   // (because multiples of 4*100=400 years (incl. leap days) have been removed)
   centennials = (CORE::UInt32)(sec / CORE::UInt64( 3155673600ULL ) ); // 100*(365+24/100)*24*3600
   if (centennials > 3)
   {
      centennials = 3;
   }
   sec -= centennials * CORE::UInt64( 3155673600 );

   // Remove multiples of 4 years (incl. 1 leap day), can't be more than 24
   // (because multiples of 25*4=100 years (incl. leap days) have been removed)
   quadrennials = (CORE::UInt32)(sec / 126230400); // 4*(365+1/4)*24*3600
   if (quadrennials > 24)
   {
      quadrennials = 24;
   }
   sec -= quadrennials * CORE::UInt64( 126230400ULL );

   // Remove multiples of years (incl. 0 leap days), can't be more than 3
   // (because multiples of 4 years (incl. leap days) have been removed)
   annuals = (CORE::UInt32)(sec / 31536000); // 365*24*3600
   if (annuals > 3)
   {
      annuals = 3;
   }
   sec -= annuals * CORE::UInt64( 31536000ULL );

   // Calculate the year and find out if it's leap
   year = 1601 + quadricentennials * 400 + centennials * 100 + quadrennials * 4 + annuals;
   leap = !(year % 4) && (year % 100 || !(year % 400));

   // Calculate the day of the year and the time
   yday = sec / 86400;
   sec %= 86400;
   hour = sec / 3600;
   sec %= 3600;
   min = sec / 60;
   sec %= 60;

   // Calculate the month
   for (mday = month = 1; month < 13; month++)
   {
      if (yday < daysSinceJan1st[leap][month])
      {
         mday += yday - daysSinceJan1st[leap][month - 1];
         break;
      }
   }

   // Fill in C's "struct tm"
   memset(pTm, 0, sizeof(*pTm));
   pTm->tm_sec = sec;          // [0,59]
   pTm->tm_min = min;          // [0,59]
   pTm->tm_hour = hour;        // [0,23]
   pTm->tm_mday = mday;        // [1,31]  (day of month)
   pTm->tm_mon = month - 1;    // [0,11]  (month)
   pTm->tm_year = year - 1900; // 70+     (year since 1900)
   pTm->tm_wday = wday;        // [0,6]   (day since Sunday AKA day of week)
   pTm->tm_yday = yday;        // [0,365] (day since January 1st AKA day of year)
   pTm->tm_isdst = -1;         // daylight saving time flag

   return pTm;
}

/*-------------------------------------------------------------------------*/
   
// From: https://stackoverflow.com/questions/45149620/convert-ole-automation-date-oadate-double-to-struct-tm-without-using-variantti
// Slightly altered, got rid of exception throw

struct tm* fromOADate(struct tm* p_Tm, double p_OADate)
{
   static const int64_t OA_UnixTimestamp = -2209161600; /* 30-Dec-1899 */

   if (!(   -109205 <= p_OADate               /* 01-Jan-1601 */
         &&            p_OADate <= 2958465))  /* 31-Dec-9999 */
   {
      return GUCEF_NULL;
   }

   int64_t OADatePassedDays = p_OADate;
   double  OADateDayTime    = p_OADate - OADatePassedDays;
   int64_t OADateSeconds    = OA_UnixTimestamp
                            + OADatePassedDays * 24LL * 3600LL
                            + OADateDayTime * 24.0 * 3600.0;

   return SecondsSinceEpochToDateTime(p_Tm, OADateSeconds);
}

/*-------------------------------------------------------------------------*/

CORE::Int32
CMsmqPubSubClientTopic::SizeOfVarType( VARTYPE vt )
{GUCEF_TRACE;

    switch ( vt )
    {
        case VT_VECTOR | VT_UI1:
        case VT_UI1:                    { return sizeof( UCHAR ); }
        
        case VT_VECTOR | VT_I2:
        case VT_I2:                     { return sizeof( SHORT ); }
        
        case VT_VECTOR | VT_UI2:
        case VT_UI2:                    { return sizeof( USHORT ); }

        case VT_VECTOR | VT_I4:
        case VT_I4:                     { return sizeof( LONG ); }

        case VT_UI4:                    { return sizeof( ULONG ); }
        case VT_UI8:                    { return sizeof( ULONGLONG ); }
        case VT_ERROR:                  { return sizeof( LONG ); }
                
        case VT_VECTOR | VT_BOOL:
        case VT_BOOL:                   { return sizeof( VARIANT_BOOL ); }

        case VT_VECTOR | VT_CLSID:
        case VT_CLSID:                  { return sizeof( CLSID ); }

        case VT_VECTOR | VT_BSTR:
        case VT_BSTR:                   { return sizeof( WCHAR ); }
        
        case VT_LPSTR:                  { return sizeof( CHAR ); }

        case VT_VECTOR | VT_LPWSTR:
        case VT_LPWSTR:                 { return sizeof( WCHAR ); }

        case VT_BLOB:                   { return sizeof( UCHAR ); }        
        case VT_DATE:                   { return sizeof( DATE ); }

        case VT_VECTOR | VT_VARIANT:    { return sizeof( VARIANT ); }
        
        case VT_EMPTY:                  { return 0; }

        default:
        {
            return -1;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::StringToMsmqGUID( const CORE::CAsciiString& guidString ,
                                          GUID& guid                           )
{GUCEF_TRACE;

    if ( !guidString.IsNULLOrEmpty() && guidString.IsFormattingValid() )
    {
        int scanResult = sscanf( guidString.C_String(), "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
                &guid.Data1, &guid.Data2, &guid.Data3, 
                &guid.Data4[0], &guid.Data4[1], &guid.Data4[2], &guid.Data4[3] ,
                &guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7] );

        return 11 == scanResult;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::MsmqGUIDToString( const GUID& guid               ,
                                          CORE::CAsciiString& guidString )
{GUCEF_TRACE;

    guidString.Reserve( 128 );
    int printResult = sprintf( guidString.C_String(), "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
                guid.Data1, guid.Data2, guid.Data3, 
                guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3] ,
                guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7] );
    guidString.DetermineLength();
                
    return printResult > 0;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::MsmqPropertyToVariantStatic( MQPROPVARIANT& msmqSourceVariant , 
                                                     CORE::CVariant& targetVariant    , 
                                                     bool linkIfPossible              , 
                                                     CORE::UInt32 lengthIfApplicable  ,
                                                     bool scanForNullTerminatorOnStrs ,
                                                     bool convertMsmqClsIdToString    ,
                                                     CORE::CAsciiString& guidStrCache )
{GUCEF_TRACE;

    switch ( msmqSourceVariant.vt )
    {
        case VT_I1:                     { targetVariant = (CORE::Int8) msmqSourceVariant.cVal; return true; }
        case VT_UI1:                    { targetVariant = (CORE::UInt8) msmqSourceVariant.bVal; return true; }
        case VT_I2:                     { targetVariant = (CORE::Int16) msmqSourceVariant.iVal; return true; }
        case VT_UI2:                    { targetVariant = (CORE::UInt16) msmqSourceVariant.uiVal; return true; }
        case VT_I4:                     { targetVariant = (CORE::Int32) msmqSourceVariant.lVal; return true; }
        case VT_UI4:                    { targetVariant = (CORE::UInt32) msmqSourceVariant.ulVal; return true; }
        case VT_I8:                     { targetVariant = (CORE::Int64) msmqSourceVariant.hVal.QuadPart; return true; }
        case VT_UI8:                    { targetVariant = (CORE::UInt64) msmqSourceVariant.uhVal.QuadPart; return true; }
        case VT_ERROR:                  { targetVariant = (CORE::Int32) msmqSourceVariant.scode; return true; }        
        case VT_BOOL:                   { targetVariant = ( VARIANT_FALSE != msmqSourceVariant.boolVal ); return true; } // Actually stored as a short
        
        // The CLSID (GUID) record type is a C structure. The adapter does not need a size field for this record because it is a fixed length field. 
        // It is critical that the data must be exactly sizeof(CLSID) bytes (16) long for proper I/O with MSMQ.
        case VT_CLSID:                  
        {
            if ( convertMsmqClsIdToString )
            {
                const GUID& guid = *msmqSourceVariant.puuid;
                if ( MsmqGUIDToString( guid, guidStrCache ) )
                {
                    targetVariant = guidStrCache;
                    return true;
                }
                return false;
            }
            else
            {
                return targetVariant.Set( msmqSourceVariant.puuid, sizeof(CLSID), GUCEF_DATATYPE_BINARY_BLOB, linkIfPossible );
            }
        }

        // Rememeber that like the COM concept the length is actually stored at (BSTR pointer address)-4  
        // Since in the COM world this type is often used to shuttle binary data around we will mark it as a binary and not GUCEF_DATATYPE_UTF16_STRING
        case VT_BSTR:                   { return targetVariant.Set( msmqSourceVariant.pwszVal, SysStringLen( msmqSourceVariant.bstrVal ), GUCEF_DATATYPE_BINARY_BLOB, linkIfPossible ); } 

        case VT_LPSTR:                  
        { 
            if ( scanForNullTerminatorOnStrs )
                return targetVariant.Set( msmqSourceVariant.pszVal, (UInt32) strlen( msmqSourceVariant.pszVal ), GUCEF_DATATYPE_UTF8_STRING, linkIfPossible );
            return targetVariant.Set( msmqSourceVariant.pszVal, lengthIfApplicable, GUCEF_DATATYPE_UTF8_STRING, linkIfPossible );
        }
        case VT_LPWSTR:                 
        { 
            if ( scanForNullTerminatorOnStrs )
                return targetVariant.Set( msmqSourceVariant.pwszVal, (UInt32) wcslen( msmqSourceVariant.pwszVal ), GUCEF_DATATYPE_UTF16_LE_STRING, linkIfPossible );    
            return targetVariant.Set( msmqSourceVariant.pwszVal, lengthIfApplicable, GUCEF_DATATYPE_UTF16_LE_STRING, linkIfPossible );
        }

        case VT_BLOB:                   { return targetVariant.Set( msmqSourceVariant.blob.pBlobData, msmqSourceVariant.blob.cbSize, GUCEF_DATATYPE_BINARY_BLOB, linkIfPossible ); }
        
        case VT_DATE:                   
        { 
            // Sadly date is stored as a OLE Automation DATE which is a pain to convert
            struct tm tmStruct;
            if ( GUCEF_NULL != fromOADate( &tmStruct, msmqSourceVariant.date ) )
            {
                targetVariant = CORE::CDateTime( &tmStruct, true ).ToIso8601DateTimeString( true, true );
                return true;
            } 
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:MsmqPropertyToVariant: Unable to convert VT_DATE with value " + CORE::ToString( msmqSourceVariant.date ) );
                return false;             
            }
        }
        
        // We dont have support for a special type for more complex data arrangements like vectors
        // until (if ever) we do, just note these as BINARY data
        case VT_VECTOR | VT_I1:
        case VT_VECTOR | VT_UI1:
        case VT_VECTOR | VT_I2:
        case VT_VECTOR | VT_UI2:
        case VT_VECTOR | VT_BOOL:
        case VT_VECTOR | VT_I4:
        case VT_VECTOR | VT_UI4:
        case VT_VECTOR | VT_I8:
        case VT_VECTOR | VT_UI8:
        case VT_VECTOR | VT_CLSID:
        case VT_VECTOR | VT_BSTR:
        case VT_VECTOR | VT_LPWSTR:
        case VT_VECTOR | VT_VARIANT:
        {
            // msmqSourceVariant.caub.cElems contains the nr of elements possible in the available buffer not the nr of bytes actually used for elements inside said buffer
            // Hence we need to rely on the "lengthIfApplicable" param which obtains the size from a different property
            return targetVariant.Set( msmqSourceVariant.caub.pElems, lengthIfApplicable, GUCEF_DATATYPE_BINARY_BLOB, linkIfPossible ); 
        }

        case VT_EMPTY:
        {
            targetVariant.Clear();
            return true;
        }

        default:
        {
            targetVariant.Clear();
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:MsmqPropertyToVariant: Unable to convert VARIANT of type " + CORE::ToString( msmqSourceVariant.vt ) );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::MsmqPropertyToVariant( MQPROPVARIANT& msmqSourceVariant , 
                                               CORE::CVariant& targetVariant    , 
                                               bool linkIfPossible              , 
                                               CORE::UInt32 lengthIfApplicable  )
{GUCEF_TRACE;
                                                     
    return MsmqPropertyToVariantStatic( msmqSourceVariant                 ,
                                        targetVariant                     ,
                                        linkIfPossible                    ,
                                        lengthIfApplicable                ,
                                        false                             , 
                                        m_config.convertMsmqClsIdToString ,
                                        m_guidStr                         );
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::VariantToMsmqProperty( MSGPROPID propId, const CORE::CVariant& sourceVariant, MQPROPVARIANT& msmqTargetVariant )
{GUCEF_TRACE;

    VARTYPE targetVarType = GetMsmqVariantTypeForMsmqProperty( propId );    
    msmqTargetVariant.vt = targetVarType;

    switch ( targetVarType )
    {
        case VT_I1:                     { msmqTargetVariant.cVal = (CHAR) sourceVariant.AsInt8(); return true; }
        case VT_UI1:                    { msmqTargetVariant.bVal = (UCHAR) sourceVariant.AsUInt8(); return true; }
        case VT_I2:                     { msmqTargetVariant.iVal = (SHORT) sourceVariant.AsInt16(); return true; }
        case VT_UI2:                    { msmqTargetVariant.uiVal = (USHORT) sourceVariant.AsUInt16(); return true; }
        case VT_I4:                     { msmqTargetVariant.lVal = (LONG) sourceVariant.AsInt32(); return true; }
        case VT_UI4:                    { msmqTargetVariant.ulVal = (ULONG) sourceVariant.AsUInt32(); return true; } 
        case VT_I8:                     { msmqTargetVariant.hVal.QuadPart = (LONGLONG) sourceVariant.AsInt64(); return true; }
        case VT_UI8:                    { msmqTargetVariant.uhVal.QuadPart = (ULONGLONG) sourceVariant.AsUInt64(); return true; }
        case VT_ERROR:                  { msmqTargetVariant.scode = (LONG) sourceVariant.AsInt32(); return true; }  
        case VT_BOOL:                   { msmqTargetVariant.boolVal = (VARIANT_BOOL) sourceVariant.AsBool() ? VARIANT_TRUE : VARIANT_FALSE; return true; } // Actually stored as a short
        
        // The CLSID (GUID) record type is a C structure. The adapter does not need a size field for this record because it is a fixed length field. 
        // It is critical that the data must be exactly sizeof(CLSID) bytes (16) long for proper I/O with MSMQ. No more no less or you get an error!
        case VT_CLSID:                  
        { 
            if ( sourceVariant.ByteSize() >= sizeof(CLSID) ) 
            { 
                msmqTargetVariant.puuid = (CLSID*) sourceVariant.AsVoidPtr(); 
                return true; 
            } 
            else
            { 
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:VariantToMsmqProperty: Data given for VT_CLSID field is of incorrect nr of bytes: " + CORE::ToString( sourceVariant.ByteSize() ) );
                return false; 
            } 
        }

        // Remember that like the COM concept the length is actually stored at (BSTR pointer address)-4  
        // Since in the COM world this type is often used to shuttle binary data around we will mark it as a binary and not GUCEF_DATATYPE_UTF16_STRING
        case VT_BSTR:                   
        { 
            // @TODO if needed, this would require a dynamic memory alloc
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:VariantToMsmqProperty: VT_BSTR is currently not supported" );
            return false; 
        } 

        case VT_LPSTR:                  { msmqTargetVariant.pszVal = (LPSTR) sourceVariant.AsCharPtr(); return true; }
        case VT_LPWSTR:                 { msmqTargetVariant.pwszVal = (LPWSTR) sourceVariant.AsVoidPtr(); return true; }
        case VT_BLOB:                   { msmqTargetVariant.blob.pBlobData = (BYTE*) sourceVariant.AsVoidPtr(); msmqTargetVariant.blob.cbSize = (ULONG) sourceVariant.ByteSize(); return true; }
        
        case VT_DATE:                  
        { 
            // Sadly date is stored as a OLE Automation DATE which is a pain to convert
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:VariantToMsmqProperty: VT_DATE is currently not supported" );
            return false;             
        }
        
        // We dont have support for a special type for more complex data arrangements like vectors
        // until (if ever) we do, just note these as BINARY data
        case VT_VECTOR | VT_I1:
        case VT_VECTOR | VT_UI1:
        case VT_VECTOR | VT_I2:
        case VT_VECTOR | VT_UI2:
        case VT_VECTOR | VT_BOOL:
        case VT_VECTOR | VT_I4:
        case VT_VECTOR | VT_UI4:
        case VT_VECTOR | VT_I8:
        case VT_VECTOR | VT_UI8:
        case VT_VECTOR | VT_CLSID:
        case VT_VECTOR | VT_BSTR:
        case VT_VECTOR | VT_LPWSTR:
        case VT_VECTOR | VT_VARIANT:
        {
            // msmqSourceVariant.caub.cElems contains the nr of elements possible in the available buffer not the nr of bytes actually used for elements inside said buffer
            // Hence we need to rely on the "lengthIfApplicable" param which obtains the size from a different property
            CORE::Int32 elementSize = SizeOfVarType( targetVarType );
            if ( elementSize > 0 )
            {
                msmqTargetVariant.caub.cElems = (ULONG) ( sourceVariant.ByteSize() / elementSize ); 
                msmqTargetVariant.caub.pElems = (UCHAR*) sourceVariant.AsVoidPtr();
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqPubSubClientTopic:VariantToMsmqProperty: Unable to determine size or vector element of type " + CORE::ToString( targetVarType ) );
                return false;
            }
        }
        
        case VT_EMPTY: return true;

        default:
        {
            msmqTargetVariant.vt = VT_NULL;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:VariantToMsmqProperty: Unable to map property " + CORE::ToString( (CORE::UInt32) propId ) );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

VARTYPE
CMsmqPubSubClientTopic::GetMsmqVariantTypeForMsmqProperty( PROPID propertyId )
{GUCEF_TRACE;
    
    switch ( propertyId )
    {
        case PROPID_M_HASH_ALG: { return VT_UI4; }
        case PROPID_M_ENCRYPTION_ALG: { return VT_UI4; }
        case PROPID_M_SENDER_CERT: { return VT_VECTOR | VT_UI1; } 
        case PROPID_M_SENDER_CERT_LEN: { return VT_UI4; } 
        case PROPID_M_SECURITY_CONTEXT: { return VT_UI4; } 
        case PROPID_M_XACT_STATUS_QUEUE: { return VT_LPWSTR; } 
        case PROPID_M_XACT_STATUS_QUEUE_LEN: { return VT_UI4; } 
        case PROPID_M_DEST_SYMM_KEY: { return VT_VECTOR | VT_UI1; }
        case PROPID_M_DEST_SYMM_KEY_LEN: { return VT_UI4; }
        case PROPID_M_SIGNATURE: { return VT_VECTOR | VT_UI1; }
        case PROPID_M_SIGNATURE_LEN: { return VT_UI4; }
        case PROPID_M_PROV_TYPE: { return VT_UI4; }
        case PROPID_M_PROV_NAME: { return VT_LPWSTR; }
        case PROPID_M_PROV_NAME_LEN: { return VT_UI4; }
        case PROPID_M_PRIV_LEVEL: { return VT_UI4; }
        case PROPID_M_SENDERID: { return VT_VECTOR | VT_UI1; } 
        case PROPID_M_SENDERID_LEN: { return VT_UI4; } 
        case PROPID_M_SENDERID_TYPE: { return VT_UI4; } 
        case PROPID_M_ACKNOWLEDGE: { return VT_UI1; }
        case PROPID_M_ADMIN_QUEUE: { return VT_LPWSTR; }                // Format name string of the administration queue.
        case PROPID_M_ADMIN_QUEUE_LEN: { return VT_UI4; }
        case PROPID_M_APPSPECIFIC: { return VT_UI4; }
        case PROPID_M_ARRIVEDTIME: { return VT_UI4; }
        case PROPID_M_AUTH_LEVEL: { return VT_UI4; }
        case PROPID_M_AUTHENTICATED: { return VT_UI1; }
        case PROPID_M_AUTHENTICATED_EX: { return VT_UI1; }
        case PROPID_M_BODY: { return VT_VECTOR | VT_UI1; }
        case PROPID_M_BODY_SIZE: { return VT_UI4; }
        case PROPID_M_BODY_TYPE: { return VT_UI4; }
        case PROPID_M_CLASS: { return VT_UI2; }
        case PROPID_M_CONNECTOR_TYPE: { return VT_CLSID; }              // Pointer to a connector application identifier.
        case PROPID_M_CORRELATIONID: { return VT_VECTOR | VT_UI1; }     // Application-defined 20-byte message identifier (default is 0).
        case PROPID_M_DELIVERY: { return VT_UI1; }                      // The PROPID_M_DELIVERY property specifies how Message Queuing delivers the message to the queue.
        case PROPID_M_DEST_QUEUE: { return VT_LPWSTR; }                 // Format name of the original destination queue.
        case PROPID_M_DEST_QUEUE_LEN: { return VT_UI4; }                // The length (in Unicode characters) of the supplied format name buffer OR length of the returned format name string for the destination queue
        case PROPID_M_EXTENSION: { return VT_UI1 | VT_VECTOR; }         // This property provides a place to put additional application-defined information that is associated with the message.
        case PROPID_M_EXTENSION_LEN: { return VT_UI4; }                 // This property specifies the length of the information in the PROPID_M_EXTENSION property.
        case PROPID_M_FIRST_IN_XACT: { return VT_UI1; }                 // This property indicates whether the message was the first message sent in a transaction.
        case PROPID_M_JOURNAL: { return VT_UI1; }                       // This property specifies the source journaling level for the message.
        case PROPID_M_LABEL: { return VT_LPWSTR; }                      // This property specifies a label of the message.
        case PROPID_M_LABEL_LEN: { return VT_UI4; }                     // This property identifies the length (in Unicode characters) of the message label buffer.
        case PROPID_M_LAST_IN_XACT: { return VT_UI1; }                  // This property indicates whether the message was the last message sent in a transaction.
        case PROPID_M_MSGID: { return VT_VECTOR | VT_UI1; }             // This property indicates the MSMQ-generated identifier of the message. MSMQ generates a 20-byte message identifier when the message is sent.
        case PROPID_M_PRIORITY: { return VT_UI1; }                      // This property specifies the priority of the message. A low number means low priority.
        case PROPID_M_RESP_QUEUE: { return VT_LPWSTR; }                 // This property specifies the queue where application-generated response messages are returned.
        case PROPID_M_RESP_QUEUE_LEN: { return VT_UI4; }                // This property indicates the length (in Unicode characters) of the response queue format name buffer.
        case PROPID_M_SENTTIME: { return VT_UI4; }                      // This property indicates the date and time that the message was sent by the source Queue Manager.
        case PROPID_M_SRC_MACHINE_ID: { return VT_CLSID; }              // This property specifies the computer where the message originated.
        case PROPID_M_TIME_TO_BE_RECEIVED: { return VT_UI4; }           // This property specifies the total time (in seconds) the message is allowed to live. This includes the time it spends getting to the destination queue plus the time spent waiting in the queue before it is retrieved by an application.
        case PROPID_M_TIME_TO_REACH_QUEUE: { return VT_UI4; }           // This property specifies a time limit (in seconds) for the message to reach the queue.
        case PROPID_M_TRACE: { return VT_UI1; }                         // This property specifies where report messages are sent when tracing a message.
        case PROPID_M_VERSION: { return VT_UI4; }                       // This property specifies the version of MSMQ used to send the message.
        case PROPID_M_XACTID: { return VT_VECTOR | VT_UI1; }            // This property indicates which transaction sent the message.
        case PROPID_Q_AUTHENTICATE: { return VT_UI1; }                  // Optional. This property specifies whether the queue accepts only authenticated messages.
        case PROPID_Q_CREATE_TIME: { return VT_I4; }                    // Optional read-only. This property indicates the time and date when the queue was created.
        case PROPID_Q_JOURNAL: { return VT_UI1; }                       // Optional. This property specifies the journaling level of the queue.
        case PROPID_Q_JOURNAL_QUOTA: { return VT_UI4; }                 // Optional. This property specifies the maximum size (in kilobytes) of the queue journal.
        case PROPID_Q_LABEL: { return VT_LPWSTR; }                      // Optional. This property specifies a description of the queue.
        case PROPID_Q_MODIFY_TIME: { return VT_I4; }                    // Optional. This property indicates the last time the properties of a queue were modified.
        case PROPID_Q_PATHNAME: { return VT_LPWSTR; }                   // Required (to create the queue). This property specifies the MSMQ path name of the queue. The maximum length of the name of the queue is 124 Unicode characters
        case PROPID_Q_PATHNAME_DNS: { return VT_LPWSTR; }
        case PROPID_Q_PRIV_LEVEL: { return VT_UI4; }                    // Optional. This property specifies the privacy level required by the queue.
        case PROPID_Q_QUOTA: { return VT_UI4; }                         // Optional. This property specifies the maximum size (in kilobytes) of the queue. The default is INFINITE.
        case PROPID_Q_TRANSACTION: { return VT_UI1; }                   // Optional. This property specifies whether the queue is a transactional queue or a nontransactional queue.
        case PROPID_QM_MACHINE_ID: { return VT_CLSID; }                 // This property identifies the computer. The PROPID_QM_MACHINE_ID property is set by MSMQ when the computer is defined.

        // For MSMQ 3.0 and above:
        #if(_WIN32_WINNT >= 0x0501)
        case PROPID_M_RESP_FORMAT_NAME: { return VT_LPWSTR; }
        case PROPID_M_RESP_FORMAT_NAME_LEN: { return VT_UI4; }
        case PROPID_M_DEST_FORMAT_NAME: { return VT_LPWSTR; }           // String that contains a public, private, direct, distribution list, or multiple-element format name.
        case PROPID_M_DEST_FORMAT_NAME_LEN: { return VT_UI4; }          // On input: The length of the format name buffer (in Unicode characters) allocated by the receiving application. | On return: The length (in Unicode characters) of the format name string (including the null-terminating character) returned in PROPID_M_DEST_FORMAT_NAME.
        case PROPID_M_LOOKUPID: { return VT_UI8; } 
        case PROPID_M_SOAP_ENVELOPE: { return VT_LPWSTR; }              // This property provides the SOAP envelope of an HTTP message and does not include binary attachments.
        case PROPID_M_SOAP_ENVELOPE_LEN: { return VT_UI4; }             // This property provides the size of the SOAP envelope of an HTTP message.
        case PROPID_M_COMPOUND_MESSAGE: { return VT_VECTOR | VT_UI1; }  // An array of bytes that represents the entire contents of an SRMP message, including both the SOAP envelope and the SOAP attachments (MIME binary attachments) associated with it.
        case PROPID_M_COMPOUND_MESSAGE_SIZE: { return VT_UI4; }         // The size (in bytes) of the entire SRMP message, including both the SOAP envelope and the SOAP attachments, provided by PROPID_M_COMPOUND_MESSAGE.
        case PROPID_M_SOAP_HEADER: { return VT_LPWSTR; }                // This property specifies additional application-generated header elements for inclusion in the SOAP envelope of an HTTP message.
        case PROPID_M_SOAP_BODY: { return VT_LPWSTR; }                  // This property specifies additional application-generated body elements for inclusion in the SOAP envelope of an HTTP message.
        case PROPID_M_DEADLETTER_QUEUE: { return VT_LPWSTR; }           // The path name of an application-specific DLQ.
        case PROPID_M_DEADLETTER_QUEUE_LEN: { return VT_UI4; }          // Input: The length of the application-specific dead letter queue (DLQ) name buffer (in Unicode characters) allocated by the receiving application. | Return: The length (in Unicode characters) of the application-specific DLQ name string (including the null-terminating character) returned by PROPID_M_DEADLETTER_QUEUE.
        case PROPID_M_ABORT_COUNT: { return VT_UI4; } 
        case PROPID_M_MOVE_COUNT: { return VT_UI4; } 
        case PROPID_M_LAST_MOVE_TIME: { return VT_UI4; }
        #endif

        default: 
        { 
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:GetMsmqVariantTypeForMsmqProperty: Unable to map property " + CORE::ToString( (CORE::UInt32) propertyId ) );
            return VT_EMPTY; 
        }
    }
}

/*-------------------------------------------------------------------------*/

PROPID
CMsmqPubSubClientTopic::GetPayloadPropertyForPayloadSizeProperty( PROPID payloadSizePropId )
{GUCEF_TRACE;

    switch ( payloadSizePropId )
    {
        case PROPID_M_SENDERID_LEN:          { return PROPID_M_SENDERID; }
        case PROPID_M_PROV_NAME_LEN:         { return PROPID_M_PROV_NAME; }
        case PROPID_M_SIGNATURE_LEN:         { return PROPID_M_SIGNATURE; }
        case PROPID_M_DEST_SYMM_KEY_LEN:     { return PROPID_M_DEST_SYMM_KEY; }
        case PROPID_M_XACT_STATUS_QUEUE_LEN: { return PROPID_M_XACT_STATUS_QUEUE; }
        case PROPID_M_SENDER_CERT_LEN:       { return PROPID_M_SENDER_CERT; }
        case PROPID_M_ADMIN_QUEUE_LEN:       { return PROPID_M_ADMIN_QUEUE; }
        case PROPID_M_BODY_SIZE:             { return PROPID_M_BODY; }                
        case PROPID_M_DEST_QUEUE_LEN:        { return PROPID_M_DEST_QUEUE; }
        case PROPID_M_EXTENSION_LEN:         { return PROPID_M_EXTENSION; }
        case PROPID_M_LABEL_LEN:             { return PROPID_M_LABEL; }
        case PROPID_M_RESP_QUEUE_LEN:        { return PROPID_M_RESP_QUEUE; }        

        // For MSMQ 3.0 and above:
        #if(_WIN32_WINNT >= 0x0501)
        case PROPID_M_RESP_FORMAT_NAME_LEN:  { return PROPID_M_RESP_FORMAT_NAME; }
        case PROPID_M_DEST_FORMAT_NAME_LEN:  { return PROPID_M_DEST_FORMAT_NAME; }
        case PROPID_M_SOAP_ENVELOPE_LEN:     { return PROPID_M_SOAP_ENVELOPE; }
        case PROPID_M_COMPOUND_MESSAGE_SIZE: { return PROPID_M_COMPOUND_MESSAGE; }
        case PROPID_M_DEADLETTER_QUEUE_LEN:  { return PROPID_M_DEADLETTER_QUEUE; }
        #endif
                
        default: 
        { 
            return PROPID_M_BASE; 
        }
    }
}   

/*-------------------------------------------------------------------------*/

PROPID
CMsmqPubSubClientTopic::GetPayloadSizePropertyForPayloadProperty( PROPID payloadPropId )
{GUCEF_TRACE;

    switch ( payloadPropId )
    {
        case PROPID_M_SENDERID:          { return PROPID_M_SENDERID_LEN; }
        case PROPID_M_PROV_NAME:         { return PROPID_M_PROV_NAME_LEN; }
        case PROPID_M_SIGNATURE:         { return PROPID_M_SIGNATURE_LEN; }
        case PROPID_M_DEST_SYMM_KEY:     { return PROPID_M_DEST_SYMM_KEY_LEN; }
        case PROPID_M_XACT_STATUS_QUEUE: { return PROPID_M_XACT_STATUS_QUEUE_LEN; }
        case PROPID_M_SENDER_CERT:       { return PROPID_M_SENDER_CERT_LEN; }
        case PROPID_M_ADMIN_QUEUE:       { return PROPID_M_ADMIN_QUEUE_LEN; }
        case PROPID_M_BODY:              { return PROPID_M_BODY_SIZE; }
        case PROPID_M_DEST_QUEUE:        { return PROPID_M_DEST_QUEUE_LEN; }
        case PROPID_M_EXTENSION:         { return PROPID_M_EXTENSION_LEN; }
        case PROPID_M_LABEL:             { return PROPID_M_LABEL_LEN; }
        case PROPID_M_RESP_QUEUE:        { return PROPID_M_RESP_QUEUE_LEN; }

        // For MSMQ 3.0 and above:
        #if(_WIN32_WINNT >= 0x0501)
        case PROPID_M_RESP_FORMAT_NAME:  { return PROPID_M_RESP_FORMAT_NAME_LEN; }
        case PROPID_M_DEST_FORMAT_NAME:  { return PROPID_M_DEST_FORMAT_NAME_LEN; }
        case PROPID_M_SOAP_ENVELOPE:     { return PROPID_M_SOAP_ENVELOPE_LEN; }
        case PROPID_M_COMPOUND_MESSAGE:  { return PROPID_M_COMPOUND_MESSAGE_SIZE; }
        case PROPID_M_DEADLETTER_QUEUE:  { return PROPID_M_DEADLETTER_QUEUE_LEN; }
        #endif
                
        default: 
        { 
            return PROPID_M_BASE; 
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::SetUInt32OnPropertyVariant( PROPID propertyId, CORE::UInt32 valueToSet, TMsmqMsg& msgData )
{GUCEF_TRACE;
    
    if ( PROPID_M_BASE == propertyId )
        return true;

    for ( DWORD i=0; i<msgData.msgprops.cProp; ++i )
    {
        if ( propertyId == msgData.msgprops.aPropID[ i ] )
        {
            msgData.msgprops.aPropVar[ i ].ulVal = (ULONG) valueToSet;
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::Int32
CMsmqPubSubClientTopic::GetIndexOfProperty( PROPID propertyId, TMsmqMsg& msgData )
{GUCEF_TRACE;
    
    for ( DWORD i=0; i<msgData.msgprops.cProp; ++i )
    {
        if ( propertyId == msgData.msgprops.aPropID[ i ] )
        {
            return (CORE::Int32) i;
        }
    }

    return -1;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::PrepMsmqVariantStorageForProperty( PROPID propertyId, MQPROPVARIANT& msmqVariant, TMsmqMsg& msgData, bool relinkBufferOnly )
{GUCEF_TRACE;
    
    try
    {
        if ( !relinkBufferOnly )
            memset( &msmqVariant, 0, sizeof(MQPROPVARIANT) );

        msmqVariant.vt = GetMsmqVariantTypeForMsmqProperty( propertyId );

        switch ( msmqVariant.vt )
        {
            case VT_LPWSTR:
            { 
                // Why 64: 
                // "Queue names longer than 124 Unicode characters are not supported. Using names longer than 64 Unicode characters for public queue names may cause a slight reduction in performance."
                // Queue names are the most common usage for VT_LPWSTR

                CORE::CDynamicBuffer& buffer = msgData.propBuffers[ propertyId ];
                if ( !relinkBufferOnly )
                    buffer.SetDataSize( 64*sizeof(WCHAR) ); 
                msmqVariant.pwszVal = buffer.AsTypePtr<WCHAR>();
                SetUInt32OnPropertyVariant( GetPayloadSizePropertyForPayloadProperty( propertyId ), buffer.GetBufferSize(), msgData );
                break;
            } 
            case VT_LPSTR:
            { 
                // Why 64: Since we picked 64 for unicode as the max, seems like a good default for this one as well
                CORE::CDynamicBuffer& buffer = msgData.propBuffers[ propertyId ];
                if ( !relinkBufferOnly )
                    buffer.SetDataSize( 64*sizeof(CHAR) ); 
                msmqVariant.pszVal = buffer.AsTypePtr<CHAR>();
                SetUInt32OnPropertyVariant( GetPayloadSizePropertyForPayloadProperty( propertyId ), buffer.GetBufferSize(), msgData );  
                break;
            }
            case VT_CLSID:
            { 
                CORE::CDynamicBuffer& buffer = msgData.propBuffers[ propertyId ];
                buffer.SetDataSize( sizeof(CLSID) ); 
                msmqVariant.puuid = buffer.AsTypePtr<CLSID>();  
                break;
            }
            case VT_VECTOR | VT_I1:
            case VT_VECTOR | VT_UI1:
            case VT_VECTOR | VT_I2:
            case VT_VECTOR | VT_UI2:
            case VT_VECTOR | VT_I4:
            case VT_VECTOR | VT_UI4:
            case VT_VECTOR | VT_I8:
            case VT_VECTOR | VT_UI8:
            {
                CORE::CDynamicBuffer& buffer = msgData.propBuffers[ propertyId ];
                if ( !relinkBufferOnly )
                {
                    switch ( propertyId )
                    {
                        // BODY is by far the largest field in allowing up to 4MB per msg so we handle that as a special case
                        // We use a config setting for this with a large default. This prevents what would likely be common resizes.
                        case PROPID_M_BODY: { buffer.SetDataSize( m_config.defaultMsmqBodyBufferSizeInBytes ); break; }

                        // Fields containing a MSGID MUST be exactly PROPID_M_MSGID_SIZE (20 bytes)
                        // MSDN notes: "The buffer must consist of exactly 20 unsigned characters. Specifying either a smaller or larger buffer will result in an error."
                        case PROPID_M_CORRELATIONID:
                        case PROPID_M_MSGID: { buffer.SetDataSize( PROPID_M_MSGID_SIZE ); break; }

                        default: { buffer.SetDataSize( m_config.defaultMsmqMiscBufferSizeInBytes ); break; }
                    }          
                }
                
                msmqVariant.caub.cElems = (ULONG) buffer.GetBufferSize();
                msmqVariant.caub.pElems = buffer.AsTypePtr<UCHAR>();  
                break;
            } 
            default:
                break;
        }
    }
    catch ( const std::bad_alloc& )
    {
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::PrepMsmqVariantStorageForProperty( PROPID propertyId, TMsmqMsg& msgData, bool relinkBufferOnly )
{GUCEF_TRACE;

    CORE::Int32 propIndex = GetIndexOfProperty( propertyId, msgData );
    if ( propIndex < 0 )
        return false;

    return PrepMsmqVariantStorageForProperty( propertyId, msgData.aMsgPropVar[ propIndex ], msgData, relinkBufferOnly );
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::OnMsmqMsgBufferTooSmall( TMsmqMsg& msgsData )
{GUCEF_TRACE;
    
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnMsmqMsgBufferTooSmall: The to-be-mapped MSMQ message reportedly needs larger buffer space to copy the data" );

    // If this happens we need cycle through the message properties and find any mismatch between
    // size/length of the property as relayed vs the buffer of the associated paired property
    CORE::UInt32 foundIssues = 0;
    for ( DWORD i=0; i<msgsData.msgprops.cProp; ++i )
    {     
        MSGPROPID propId = msgsData.msgprops.aPropID[ i ];
        
        // Check to see if this property has a related payload size or stands on its own
        PROPID relatedPayloadProperty = GetPayloadPropertyForPayloadSizeProperty( propId );
        if ( relatedPayloadProperty != PROPID_M_BASE )
        { 
            // We have a related payload property
            
            CORE::CDynamicBuffer& buffer = msgsData.propBuffers[ relatedPayloadProperty ];
            CORE::UInt32 currentSize = buffer.GetBufferSize();
            CORE::UInt32 requiredSize = msgsData.msgprops.aPropVar[ i ].ulVal; 

            if ( currentSize < requiredSize )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnMsmqMsgBufferTooSmall: Buffer size for payload property " + CORE::ToString( (CORE::UInt32) relatedPayloadProperty ) + " as specified by property " + 
                    CORE::ToString( (CORE::UInt32) propId ) + " is insufficient at " + CORE::ToString( currentSize ) + " bytes. It needs " + CORE::ToString( requiredSize ) + " bytes per the MSMQ response"  );

                // First reset the message
                if ( PrepMsmqMsgStorage( msgsData, m_config.msmqPropIdsToReceive, true ) )
                {
                    // We use a suggested size to limit the nr of times we resize vs just taking the required size directly
                    CORE::UInt32 suggestedSize = currentSize + (CORE::UInt32) ( ( currentSize / 100.0 ) * m_config.minPayloadFieldGrowthPercOnBufferTooSmall );
                    if ( suggestedSize < requiredSize )
                        suggestedSize = requiredSize;
                    
                    // Resize the buffer and be sure to update the pointers and meta-data for said buffer
                    buffer.SetDataSize( suggestedSize );
                    PrepMsmqVariantStorageForProperty( relatedPayloadProperty, msgsData, true );
                    SetUInt32OnPropertyVariant( propId, suggestedSize, msgsData );

                    ++foundIssues;
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnMsmqMsgBufferTooSmall: Failed to reset MSMQ message storage in prep for buffer resize"  );
                }
            }
        }
    }

    if ( 0 == foundIssues )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqPubSubClientTopic::OnMsmqMsgBufferTooSmall: Unable to find any issue with a size mismatch on buffers vs reported needs of the received message, unable to fix the issue" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::PrepMsmqPropIdToPropIndexMap( MSGPROPIDToUInt32Map& propIndexMapToBuild, const MSGPROPIDVector& msmqPropIdsToUse )
{GUCEF_TRACE;

    try
    {
        CORE::UInt32 propIndex = 0;
        MSGPROPIDVector::const_iterator i = msmqPropIdsToUse.begin();
        while ( i != msmqPropIdsToUse.end() )
        {
            propIndexMapToBuild[ (*i) ] = propIndex;
            ++i; ++propIndex;
        }
    }
    catch ( const std::bad_alloc& )
    {
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::PrepMsmqMsgStorage( TMsmqMsg& msg, MSGPROPIDVector& msmqPropIdsToUse, bool force )
{GUCEF_TRACE;

    // @TODO: Note that property storage could be made more efficient by pre-allocating a large buffer for all dynamic length /
    //        buffer using properties and mapping the individual storage into the larger buffer. This reduces memory fragmentation
    
    try
    {
        if ( force || msg.aMsgPropId.size() != msmqPropIdsToUse.size() )
        {       
            msg.aMsgPropId.resize( msmqPropIdsToUse.size() );
            msg.aMsgPropVar.resize( msmqPropIdsToUse.size() );
            msg.aStatus.resize( msmqPropIdsToUse.size() );

            msg.msgprops.cProp = (DWORD) msmqPropIdsToUse.size();   // Number of properties we want to receive or send per message
            msg.msgprops.aPropID = &msg.aMsgPropId[ 0 ];            // pointer to MSMQ PROPID array
            msg.msgprops.aPropVar = &msg.aMsgPropVar[ 0 ];          // pointer to MSMQ VARIANT struct array
            msg.msgprops.aStatus = &msg.aStatus[ 0 ];               // Error reports on a per property basis

            CORE::UInt32 propIndex = 0;
            MSGPROPIDVector::iterator p = msmqPropIdsToUse.begin();
            while ( p != msmqPropIdsToUse.end() )
            {
                msg.aMsgPropId[ propIndex ] = (*p);

                if ( !PrepMsmqVariantStorageForProperty( (*p), msg.msgprops.aPropVar[ propIndex ], msg, false ) )
                    return false;
                ++p; ++propIndex;
            }
        }
    }
    catch ( const std::bad_alloc& )
    {
        UpdateIsHealthyStatus( false );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::PrepMsmqMsgsStorage( void )
{GUCEF_TRACE;
    
    try 
    {
        if ( m_config.needSubscribeSupport )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:PrepMsmqMsgsStorage: Allocating storage for subscription related activity. maxMsmqMsgsToReadPerSyncCycle=" + CORE::ToString( m_config.maxMsmqMsgsToReadPerSyncCycle ) );
        
            if ( m_config.convertMsmqClsIdToString )
                m_guidStr.Reserve( 128 );
        
            m_pubsubMsgs.resize( m_config.maxMsmqMsgsToReadPerSyncCycle );
            m_msmqReceiveMsgs.resize( m_config.maxMsmqMsgsToReadPerSyncCycle );
            MsmqMsgVector::iterator i = m_msmqReceiveMsgs.begin();
            while ( i != m_msmqReceiveMsgs.end() )
            {
                TMsmqMsg& msg = (*i);
                if ( !PrepMsmqMsgStorage( msg, m_config.msmqPropIdsToReceive, false ) )
                    return false;
                ++i;
            }
        }

        if ( m_config.needPublishSupport )
        {        
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:PrepMsmqMsgsStorage: Allocating storage for publishing related activity" );
        
            if ( !PrepMsmqMsgStorage( m_msgSendMsg, m_config.msmqPropIdsToSend, false ) )
                return false;
            if ( !PrepMsmqPropIdToPropIndexMap( m_msgSendPropMap, m_config.msmqPropIdsToSend ) )
                return false;
        }
    }
    catch ( const std::bad_alloc& )
    {
        UpdateIsHealthyStatus( false );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopic::TMsmqHresultSeverityCode
CMsmqPubSubClientTopic::ExtractSeverityCode( HRESULT code )
{GUCEF_TRACE;

    CORE::UInt32 sevCode = ( 10 * GUCEF_GETBITX( code, 0 ) ) + GUCEF_GETBITX( code, 1 );
    return (TMsmqHresultSeverityCode) sevCode;
}

/*-------------------------------------------------------------------------*/

const CMsmqPubSubClientTopicConfig& 
CMsmqPubSubClientTopic::GetTopicConfig( void ) const
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

CORE::CVariant
CMsmqPubSubClientTopic::GetMsmqPropertyValue( const MQMSGPROPS& msg, MSGPROPID propertyId )
{GUCEF_TRACE;

    for ( CORE::UInt32 i=0; i<msg.cProp; ++i )
    {                
        MSGPROPID currentPropertyId = msg.aPropID[ i ]; 
        if ( currentPropertyId == propertyId )
        {
            CORE::CVariant ValueVar;
            if ( MsmqPropertyToVariant( msg.aPropVar[ i ], ValueVar, false ) )
            {
                return ValueVar;
            }
        }
    }
    return CORE::CVariant::Empty;
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopic::OnMsmqMsgReceived( const MQMSGPROPS& msg, CORE::UInt32 msgCycleIndex, bool linkIfPossible )
{GUCEF_TRACE;

    PUBSUB::CBasicPubSubMsg& translatedMsg = m_pubsubMsgs[ msgCycleIndex ];
    translatedMsg.Clear();
    translatedMsg.SetOriginClientTopic( CreateSharedPtr() );

    translatedMsg.SetReceiveActionId( m_currentReceiveActionId );
    ++m_currentReceiveActionId;

    CORE::UInt64 sentTimeUnixEpocInSecs = 0;
    CORE::UInt64 arriveTimeUnixEpocInSecs = 0;

    CORE::UInt32 bodyType = VT_EMPTY;
    CORE::UInt32 bodySize = 0;

    for ( CORE::UInt32 i=0; i<msg.cProp; ++i )
    {
        // Depending on what the property is we may perform special handling
        // Note that we rely here on config loading having enforced an ordering in the message
        // properties that ensures that things like length/size or type come before the actual payload

        CORE::CVariant keyVar( (CORE::UInt64) msg.aPropID[ i ] );
        CORE::CVariant ValueVar;
        
        MSGPROPID propertyId = msg.aPropID[ i ];        
        switch ( propertyId )
        {
            
            case PROPID_M_BODY_TYPE: 
            { 
                if ( MsmqPropertyToVariant( msg.aPropVar[ i ], ValueVar, linkIfPossible ) )
                {
                    bodyType = ValueVar.AsUInt32( bodyType ); 

                    // Retain as meta-data
                    if ( linkIfPossible )
                        translatedMsg.AddLinkedMetaDataKeyValuePair( keyVar, ValueVar );
                    else
                        translatedMsg.AddMetaDataKeyValuePair( keyVar, ValueVar );
                }
                break; 
            }
            case PROPID_M_BODY_SIZE: { MsmqPropertyToVariant( msg.aPropVar[ i ], ValueVar, linkIfPossible ); bodySize = ValueVar.AsUInt32( bodySize ); break; }
            case PROPID_M_BODY:
            {
                if ( MsmqPropertyToVariant( msg.aPropVar[ i ], ValueVar, linkIfPossible, bodySize ) )
                {                
                    if ( linkIfPossible )
                        translatedMsg.GetPrimaryPayload().LinkTo( ValueVar );
                    else
                        ValueVar.TransferOwnershipTo( translatedMsg.GetPrimaryPayload() );
                }
                else
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnMsmqMsgReceived: Unable to translate MSMQ PROPID_M_BODY. Translated message won't have a body" );
                    translatedMsg.GetPrimaryPayload().Clear();
                }
                break;
            }

            #if(_WIN32_WINNT >= 0x0501)
            case PROPID_M_LOOKUPID:
            {
                if ( MsmqPropertyToVariant( msg.aPropVar[ i ], ValueVar, linkIfPossible ) )
                {
                    translatedMsg.GetMsgIndex() = ValueVar.AsUInt64();

                    // IMPORTANT: For the peek cycle to work we need to obtain the lookup id and retain it as state so that
                    //            we can use it to iterate to the next message
                    UInt64 priorLastLookupId = m_msmqLastLookupId;
                    m_msmqLastLookupId = ValueVar.AsUInt64();

                    // Retain as meta-data
                    if ( linkIfPossible )
                        translatedMsg.AddLinkedMetaDataKeyValuePair( keyVar, ValueVar );
                    else
                        translatedMsg.AddMetaDataKeyValuePair( keyVar, ValueVar );

                    // Also retain the prior lookup id as meta-data for handling gaps in the lookup id sequence.
                    // Note that there is no MSMQ property ID for such a thing. As such as a somewhat safe hack
                    // we will use a negative value since prop ids are all unsigned ints hence no risk of conflict.
                    // 
                    // Even under normal conditions for simplistic scenarios it was found that the lookup id
                    // has a high incidence of gaps in the numbering. As such we need the 'prior' number as well from
                    // the consuming app's perspective as a 'gap fill' such that the lookup id can serve as a sequence
                    // number 
                    CORE::CVariant keyForPriorLookupId( keyVar.AsInt32() * -1 );
                    CORE::CVariant valueOfPriorLookupId( priorLastLookupId ); 
                    translatedMsg.AddMetaDataKeyValuePair( keyForPriorLookupId, valueOfPriorLookupId );
                }
                else
                {
                    // PROPID_M_LOOKUPID should always be available since its MSMQ generated in MSMQ 3.0 and above
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnMsmqMsgReceived: Unable to obtain MSMQ PROPID_M_LOOKUPID. Translated message won't have a message index value" );
                }
                break;
            }
            #endif

            case PROPID_M_SENTTIME: 
            {
                if ( MsmqPropertyToVariant( msg.aPropVar[ i ], ValueVar, linkIfPossible ) )
                {
                    // Time is Unix epoch based in second resolution
                    // It is stored as a UInt32 but we use a UInt64 because we need the headroom to convert to milliseconds
                    sentTimeUnixEpocInSecs = ValueVar.AsUInt64();
                    translatedMsg.GetMsgDateTime().FromUnixEpochBasedTicksInMillisecs( sentTimeUnixEpocInSecs * 1000 );

                    if ( m_config.retainMsmqSentTimeAsMetaData )
                    {
                        // Retain as meta-data as well
                        if ( linkIfPossible )
                            translatedMsg.AddLinkedMetaDataKeyValuePair( keyVar, ValueVar );
                        else
                            translatedMsg.AddMetaDataKeyValuePair( keyVar, ValueVar );
                    }
                }
                else
                {
                    // PROPID_M_SENTTIME should always be available since its MSMQ generated
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnMsmqMsgReceived: Unable to translate MSMQ PROPID_M_SENTTIME. Translated message won't have a DateTime timestamp originating from MSMQ, falling back to 'now' dt" );
                    translatedMsg.GetMsgDateTime() = CORE::CDateTime::NowUTCDateTime();
                }
                break;
            }
            case PROPID_M_ARRIVEDTIME: 
            {
                if ( MsmqPropertyToVariant( msg.aPropVar[ i ], ValueVar, linkIfPossible ) )
                {
                    // Time is Unix epoch based in second resolution
                    arriveTimeUnixEpocInSecs = ValueVar.AsUInt64();

                    // Retain as meta-data
                    if ( linkIfPossible )
                        translatedMsg.AddLinkedMetaDataKeyValuePair( keyVar, ValueVar );
                    else
                        translatedMsg.AddMetaDataKeyValuePair( keyVar, ValueVar );
                }
                else
                {
                    // PROPID_M_ARRIVEDTIME should always be available since its MSMQ generated
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnMsmqMsgReceived: Unable to translate MSMQ PROPID_M_ARRIVEDTIME" );
                }
                break;
            }
            
            default:
            {
                if ( !MsmqPropertyToVariant( msg.aPropVar[ i ], ValueVar, linkIfPossible ) )
                {
                    // Skip this property, continue best effort
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::OnMsmqMsgReceived: Unable to translate MSMQ meta-data propery value variant for property " + keyVar.AsString() +
                        " which is property " + CORE::ToString( i ) + " of " + CORE::ToString( (CORE::UInt32) msg.cProp ) );
                    
                    if ( propertyId == m_config.msmqMsgPropIdToMapToMsgIdOnReceive )
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::OnMsmqMsgReceived: Unable to translate MSMQ property type " + keyVar.AsString() + ". Translated message won't have an ID as this property is designated to be used as the MsgId" );
                    }
                    
                    continue;
                }                

                if ( propertyId == m_config.msmqMsgPropIdToMapToMsgIdOnReceive )
                {
                    if ( ValueVar.IsDynamicMemoryLinked() )
                        translatedMsg.GetMsgId().LinkTo( ValueVar );
                    else
                        ValueVar.TransferOwnershipTo( translatedMsg.GetMsgId() );
                }
                else
                {
                    // No special processing, treat as meta-data
                    if ( linkIfPossible )
                        translatedMsg.AddLinkedMetaDataKeyValuePair( keyVar, ValueVar );
                    else
                        translatedMsg.AddMetaDataKeyValuePair( keyVar, ValueVar );
                    break;
                }
            }
        }
    }
    
    // Collect MSMQ transit time metric if so configured
    // Its a little extra work but can help diagnose the root of latency issues
    if ( m_config.gatherMsmqTransitTimeOnReceiveMetric && 0 != sentTimeUnixEpocInSecs && 0 != arriveTimeUnixEpocInSecs && m_config.maxMsmqTransitTimeOnReceiveMetricDataPoints >= m_msmqMsgSentToArriveLatencies.size() )
    {
        // signed vs unsigned: Ignore errant deltas due to clock drift
        CORE::Int64 msmqTransitTimeInSecs = (CORE::Int64) ( arriveTimeUnixEpocInSecs - sentTimeUnixEpocInSecs );
        if ( msmqTransitTimeInSecs >= 0 )
        {
            m_msmqMsgSentToArriveLatencies.push_back( (CORE::UInt32) ( msmqTransitTimeInSecs * 1000 ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopic::NotifyOfReceivedMsgs( void )
{GUCEF_TRACE;

    if ( !m_pubsubMsgsRefs.empty() )
    {
        // Communicate all the messages received via an event notification
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:NotifyOfReceivedMsgs: Received and mapped a total of " + CORE::ToString( m_pubsubMsgsRefs.size() ) + 
            " messages. Poll cycle max =" + CORE::ToString( m_config.maxMsmqMsgsToReadPerSyncCycle ) );

        m_msmqMessagesReceived += m_pubsubMsgsRefs.size();
        
        if ( !NotifyObservers( MsgsRecievedEvent, &m_pubsubMsgsRefs ) ) return;
        m_pubsubMsgsRefs.clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopic::OnSyncReadTimerCycle( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;
        
    DWORD dwAction = MQ_ACTION_RECEIVE;                 // could support peeking: isPeekOnly ? MQ_ACTION_PEEK_CURRENT : MQ_ACTION_RECEIVE;
    DWORD dwTimeout = 0;                                // We dont wait for messages during sync reads, we get what we can this cycle and thats it
    ITransaction* pTransaction = MQ_NO_TRANSACTION;     // could support transactions: isTransactional ? MQ_SINGLE_MESSAGE : MQ_NO_TRANSACTION;

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
        return;
    
    // For MSMQ 3.0 and above:
    const CMsmqPubSubClientConfig& clientConfig = m_client->GetConfig();
    bool supportLookup = clientConfig.simulateReceiveAckFeatureViaLookupId && clientConfig.desiredFeatures.supportsSubscriberMsgReceivedAck; 
    bool endOfData = false;

    CORE::UInt32 msgsRead = 0; bool isRetry = false;
    for ( CORE::Int32 i=0; i<(CORE::Int32)m_config.maxMsmqMsgsToReadPerSyncCycle; ++i )
    {
        // For MSMQ 3.0 and above:
        #if ( _WIN32_WINNT >= 0x0501 )
        
        HRESULT receiveHResult = MQ_OK;
        if ( !supportLookup )
        {
            receiveHResult = ::MQReceiveMessage( m_receiveQueueHandle             ,  // Handle to the queue
                                                 dwTimeout                        ,  // Time out interval
                                                 dwAction                         ,  // Peek?  or Dequeue.  Receive action
                                                 &m_msmqReceiveMsgs[ i ].msgprops ,  // Pointer to the MQMSGPROPS structure
                                                 NULL                             ,  // No OVERLAPPED structure etc.
                                                 NULL                             ,  // No OVERLAPPED structure etc.
                                                 NULL                             ,  // NULL, we dont use a cursor
                                                 pTransaction                     ); // MQ_SINGLE_MESSAGE | MQ_MTS_TRANSACTION | MQ_XA_TRANSACTION   
        }
        else
        {
            receiveHResult = MQReceiveMessageByLookupId( m_receiveQueueHandle ,                 // Handle to the queue 
                                                         m_msmqLastLookupId   ,                 // The lookup identifier  
                                                         0 == m_msmqLastLookupId ? MQ_LOOKUP_PEEK_FIRST : ( isRetry ? MQ_LOOKUP_PEEK_CURRENT : MQ_LOOKUP_PEEK_NEXT ),   // Access mode  
                                                         &m_msmqReceiveMsgs[ i ].msgprops ,     // Pointer to the MQMSGPROPS structure
                                                         NULL                 ,                 // No OVERLAPPED structure  
                                                         NULL                 ,                 // No callback function  
                                                         pTransaction         );                // Not in a transaction  
        }
        
        // reset the retry flag (MQ_LOOKUP_PEEK_CURRENT)
        isRetry = false;

        #else

        HRESULT receiveHResult = ::MQReceiveMessage( m_receiveQueueHandle             ,  // Handle to the queue
                                                     dwTimeout                        ,  // Time out interval
                                                     dwAction                         ,  // Peek?  or Dequeue.  Receive action
                                                     &m_msmqReceiveMsgs[ i ].msgprops ,  // Pointer to the MQMSGPROPS structure
                                                     NULL                             ,  // No OVERLAPPED structure etc.
                                                     NULL                             ,  // No OVERLAPPED structure etc.
                                                     NULL                             ,  // NULL, we dont use a cursor
                                                     pTransaction                     ); // MQ_SINGLE_MESSAGE | MQ_MTS_TRANSACTION | MQ_XA_TRANSACTION  

        #endif

        CORE::UInt64 receiveResult = (CORE::UInt64) receiveHResult;
        switch ( receiveResult )
        {
            case (CORE::UInt64) MQ_OK:
            {
                // Earliest point we can claim we successfully received this message
                CORE::CDateTime msgReadFromMsmqDateTime = CORE::CDateTime::NowUTCDateTime();
                
                // We will consider the ability to receive and process messages as healthy
                UpdateIsHealthyStatus( true );
                
                // Successfully received a message
                // Since we are doing a sync cycle we can use linking
                OnMsmqMsgReceived( m_msmqReceiveMsgs[ i ].msgprops, i, true );

                // Stamp the time the message was received on the object properties
                if ( m_config.addMsgReadFromMsmqTimeAsMetaData )
                    m_pubsubMsgs[ i ].AddMetaDataKeyValuePair( m_config.metadataKeyForMsgReadFromMsmqTime, msgReadFromMsmqDateTime.ToUnixEpochBasedTicksInMillisecs() );
                
                ++msgsRead;
                m_pubsubMsgsRefs.push_back( TPubSubMsgRef( &m_pubsubMsgs[ i ] ) );
                 
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnSyncReadTimerCycle: Received and mapped MSMQ message " + CORE::ToString( i ) + " of the current cycle. receiveActionId=" + CORE::ToString( m_pubsubMsgs[ i ].GetReceiveActionId() ) );
                break;
            }
            case (CORE::UInt64) MQ_ERROR_PROPERTY:         // this means: "One or more message properties specified in m_msmqReceiveMsgs[ i ].msgprops resulted in an error."
            case (CORE::UInt64) MQ_INFORMATION_PROPERTY:
            {
                bool hasError = false;
                const MQMSGPROPS& msg = m_msmqReceiveMsgs[ i ].msgprops;
                for ( CORE::UInt32 s=0; s<msg.cProp; ++s )
                {
                    TMsmqHresultSeverityCode sevCode = ExtractSeverityCode( msg.aStatus[ s ] );
                    switch ( sevCode )
                    {
                        case TMsmqHresultSeverityCode::Error:
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "MsmqPubSubClientTopic::OnSyncReadTimerCycle: MSMQ message property with ID " + CORE::ToString( (CORE::UInt32) msg.aPropID[ s ] ) + " was flagged as having an error status" );
                            hasError = true;
                            break;
                        }
                        case TMsmqHresultSeverityCode::Warning:
                        {
                            GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "MsmqPubSubClientTopic::OnSyncReadTimerCycle: MSMQ message property with ID " + CORE::ToString( (CORE::UInt32) msg.aPropID[ s ] ) + " was flagged as having a warning status" );
                            break;
                        }
                        case TMsmqHresultSeverityCode::Informational:
                        {
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "MsmqPubSubClientTopic::OnSyncReadTimerCycle: MSMQ message property with ID " + CORE::ToString( (CORE::UInt32) msg.aPropID[ s ] ) + " was flagged as having an informational status" );
                            break;
                        }
                        case TMsmqHresultSeverityCode::Success:
                        default:
                            break;
                    } 
                }

                // We will consider the ability to receive and process messages as healthy
                UpdateIsHealthyStatus( !hasError );

                // Received a message but some warnings were triggered
                // Since we are doing a sync cycle we can use linking
                OnMsmqMsgReceived( m_msmqReceiveMsgs[ i ].msgprops, i, true );
                
                ++msgsRead;
                m_pubsubMsgsRefs.push_back( TPubSubMsgRef( &m_pubsubMsgs[ i ] ) );
                
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnSyncReadTimerCycle: Received and mapped MSMQ message " + CORE::ToString( i ) + " of the current cycle but with MSMQ property warnings upon retrieval" );
                break;
            }

            // Not an error in the way we use it to check for messages
            // This is how we find out there are no more messages when using MQReceiveMessage()
            case (CORE::UInt64) MQ_ERROR_IO_TIMEOUT: 
            {
                // No more messages are available, stop trying to read more
                i = m_config.maxMsmqMsgsToReadPerSyncCycle; 
                endOfData = true;
                break; 
            }

            // This is how we find out there are no more messages when using MQReceiveMessageByLookupId()
            case (CORE::UInt64) MQ_ERROR_MESSAGE_NOT_FOUND:
            {
                // No more messages are available, stop trying to read more
                i = m_config.maxMsmqMsgsToReadPerSyncCycle; 
                endOfData = true;
                break; 
            }

            case (CORE::UInt64) MQ_ERROR_BUFFER_OVERFLOW:
            case (CORE::UInt64) MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL:
            case (CORE::UInt64) MQ_ERROR_LABEL_BUFFER_TOO_SMALL:
            case (CORE::UInt64) MQ_ERROR_PROV_NAME_BUFFER_TOO_SMALL: 
            case (CORE::UInt64) MQ_ERROR_SENDER_CERT_BUFFER_TOO_SMALL:
            case (CORE::UInt64) MQ_ERROR_SENDERID_BUFFER_TOO_SMALL:
            case (CORE::UInt64) MQ_ERROR_SIGNATURE_BUFFER_TOO_SMALL:
            case (CORE::UInt64) MQ_ERROR_SYMM_KEY_BUFFER_TOO_SMALL:
            {
                ++m_msmqErrorsOnReceive;

                // Use the length/size given to resize buffers as needed
                // When this error happens the msg is not removed so we can resize and try again
                OnMsmqMsgBufferTooSmall( m_msmqReceiveMsgs[ i ] );
                
                // For MSMQ 3.0 and above:
                #if ( _WIN32_WINNT >= 0x0501 )

                if ( supportLookup )
                {
                    isRetry = true;
                    if ( 0 == m_msmqLastLookupId )
                    {
                        // IMPORTANT: For the peek cycle to work we need to obtain the lookup id and retain it as state so that
                        //            we can use it to again look at the same message.
                        CORE::CVariant propValue = GetMsmqPropertyValue( m_msmqReceiveMsgs[ i ].msgprops, PROPID_M_LOOKUPID );
                        if ( propValue.IsInitialized() )
                            m_msmqLastLookupId = propValue.AsUInt64();
                    }
                }

                #endif

                UpdateIsHealthyStatus( false );

                --i; // retry the same message slot, hopefully this time obtaining all the payloads properly
                break;
            }

            case (CORE::UInt64) MQ_ERROR_QUEUE_DELETED:        // <- queue was deleted outside the app, just keep trying to 'reconnect'
            {
                // If the queue is deleted the lookup id becomes invalid since its a property of that instance of the queue
                m_msmqLastLookupId = 0;

                // we intentiontionally dont break here because we want the code below as well
                // break;
            }
            case (CORE::UInt64) MQ_ERROR_QUEUE_NOT_ACTIVE:     // <- caused by someone messing with the queue outside the app?
            case (CORE::UInt64) MQ_ERROR_QUEUE_NOT_FOUND:      // <- caused by someone messing with the queue outside the app?
            case (CORE::UInt64) MQ_ERROR_INVALID_HANDLE:
            case (CORE::UInt64) MQ_ERROR_INVALID_PARAMETER:    // <- caused by handle becoming invalid as well?
            {
                ++m_msmqErrorsOnReceive;                
                i = m_config.maxMsmqMsgsToReadPerSyncCycle;

                CORE::UInt32 win32ErrorCode = HRESULT_CODE( receiveHResult );
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::OnSyncReadTimerCycle: Queue Access issue retrieving message " + CORE::ToString( i ) + 
                    " of the current cycle. HRESULT= 0x" + CORE::Base16Encode( &receiveResult, sizeof(receiveResult) ) + 
                    ". Win32 Error " + CORE::ToString( win32ErrorCode ) + " with message: " + CORE::ToString( RetrieveWin32APIErrorMessage( win32ErrorCode ) ) );

                lock.EarlyUnlock();

                UpdateIsHealthyStatus( false );
                BeginReconnectSequence( &ConnectionErrorEvent );
                break;
            }

            default:
            {
                ++m_msmqErrorsOnReceive;
                i = m_config.maxMsmqMsgsToReadPerSyncCycle;

                CORE::UInt32 win32ErrorCode = HRESULT_CODE( receiveHResult );
                if ( 11 == win32ErrorCode )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "MsmqPubSubClientTopic::OnSyncReadTimerCycle: Unhandled issue retrieving message " + CORE::ToString( i ) + 
                        " of the current cycle. HRESULT= 0x" + CORE::Base16Encode( &receiveResult, sizeof(receiveResult) ) + 
                        ". WIN32 ERROR CODE 11! Check system health! Win32 Error message: " + CORE::ToString( RetrieveWin32APIErrorMessage( win32ErrorCode ) ) );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic::OnSyncReadTimerCycle: Unhandled issue retrieving message " + CORE::ToString( i ) + 
                        " of the current cycle. HRESULT= 0x" + CORE::Base16Encode( &receiveResult, sizeof(receiveResult) ) + 
                        ". Win32 Error " + CORE::ToString( win32ErrorCode ) + " with message: " + CORE::ToString( RetrieveWin32APIErrorMessage( win32ErrorCode ) ) );                
                }

                // Since this is some unhandled error during a message read we will call it a connection error
                //  With unhandled errors we have no idea what the problem is and as such have no way of knowing if we can recover
                //  As a result we have to 'reset' our MSMQ access as a safety precaution. Its always better if its something that can be coded for explcitly in one of
                //  the above cases
                lock.EarlyUnlock();
                UpdateIsHealthyStatus( false );
                BeginReconnectSequence( &ConnectionErrorEvent );  
                break;
            }
        }
    }

    NotifyOfReceivedMsgs();
    m_subscriptionIsAtEndOfData = endOfData;

    if ( msgsRead == m_config.maxMsmqMsgsToReadPerSyncCycle )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:OnSyncReadTimerCycle: max msgs per cycle reached, asking for immediate pulse cycle" );
        m_syncReadTimer->RequestImmediateTrigger();
    }
    else
    if ( endOfData && clientConfig.desiredFeatures.supportsSubscriptionEndOfDataEvent )
    {
        NotifyObservers( SubscriptionEndOfDataEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopic::OnReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    bool totalSuccess = true;
    if ( m_config.needPublishSupport )
        totalSuccess = InitializeConnectivity( true ) && totalSuccess;
    if ( m_config.needSubscribeSupport )
        totalSuccess = Subscribe() && totalSuccess;

    m_reconnectTimer->SetEnabled( !totalSuccess );
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopic::OnPulseCycle( CORE::CNotifier* notifier    ,
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

CORE::UInt64
CMsmqPubSubClientTopic::GetMsmqMessagesPublishedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt64 msmqMessagesReceived = m_msmqMessagesReceived;
        m_msmqMessagesReceived = 0;
        return msmqMessagesReceived;
    }
    else
        return m_msmqMessagesReceived;
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CMsmqPubSubClientTopic::GetMsmqErrorsOnPublishCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt64 msmqErrorsOnReceive = m_msmqErrorsOnReceive;
        m_msmqErrorsOnReceive = 0;
        return msmqErrorsOnReceive;
    }
    else
        return m_msmqErrorsOnReceive;
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CMsmqPubSubClientTopic::GetMsmqMessagesReceivedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt64 msmqMessagesReceived = m_msmqMessagesReceived;
        m_msmqMessagesReceived = 0;
        return msmqMessagesReceived;
    }
    else
        return m_msmqMessagesReceived;
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CMsmqPubSubClientTopic::GetMsmqErrorsOnReceiveCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt64 msmqErrorsOnReceive = m_msmqErrorsOnReceive;
        m_msmqErrorsOnReceive = 0;
        return msmqErrorsOnReceive;
    }
    else
        return m_msmqErrorsOnReceive;
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CMsmqPubSubClientTopic::GetMsmqErrorsOnAckCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt64 msmqErrorsOnAck = m_msmqErrorsOnAck;
        m_msmqErrorsOnAck = 0;
        return msmqErrorsOnAck;
    }
    else
        return m_msmqErrorsOnAck;
}

/*-------------------------------------------------------------------------*/

CMsmqPubSubClientTopic::TopicMetrics::TopicMetrics( void )
    : msmqMsgsInQueue( 0 )
    , msmqMsgsInJournal( 0 )
    , msmqMsgBytesInQueue( 0 )
    , msmqMsgBytesInJournal( 0 )    
    , msmqMessagesPublished( 0 )
    , msmqErrorsOnPublish( 0 )
    , msmqErrorsOnAck( 0 )
    , msmqMessagesReceived( 0 )
    , msmqErrorsOnReceive( 0 )    
    , msmqGlobalMsgBytes( 0 )
    , msmqMsgSentToArriveLatencies()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CMsmqPubSubClientTopic::TopicMetrics& 
CMsmqPubSubClientTopic::GetMetrics( void ) const
{GUCEF_TRACE;

    return m_metrics;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CMsmqPubSubClientTopic::GetMetricFriendlyTopicName( void ) const
{GUCEF_TRACE;

    return m_queue.metricFriendlyQueueName;
}


/*-------------------------------------------------------------------------*/

CORE::Int64
CMsmqPubSubClientTopic::GetQueueMetric( MsmqQueue& q                           , 
                                        const CORE::CString& metricDescription ,
                                        UInt32 propId                          ,
                                        UInt32 propType                        )
{GUCEF_TRACE;

    // MQMgmtGetInfo only supports a format name
    // As such depending on how the setting is presented we need to translate it
    const std::wstring& queueFormatName = GetMsmqQueueFormatName( q );
    if ( queueFormatName.empty() )
        return -1;

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
    std::wstring queueInfoFormatName = L"QUEUE=" + queueFormatName;
    HRESULT queueInfoFetchResult = ::MQMgmtGetInfo( NULL, queueInfoFormatName.c_str(), &mgmtprops );
    if ( MQ_OK == queueInfoFetchResult )
    {
        switch ( propType )
        {
            case VT_UI4: return (CORE::Int64) aMgmtPropVar[ 0 ].ulVal;
            case VT_UI8: return (CORE::Int64) aMgmtPropVar[ 0 ].uhVal.QuadPart;

            default:
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:GetQueueMetric: PropType is unsupported. Change the code! Type=" + CORE::ToString( propType ) );
                return 0;
            }
        }
        
    }

    CORE::UInt32 errorCode =  HRESULT_CODE( queueInfoFetchResult );
    std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( queueInfoFetchResult ) );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:GetQueueMetric: Failed to obtain " + metricDescription + ". Queue Name: " + q.queueName +". errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
    return -1;
}

/*-------------------------------------------------------------------------*/

CORE::Int64
CMsmqPubSubClientTopic::GetCurrentNrOfMessagesInQueue( MsmqQueue& q ) 
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    return GetQueueMetric( q, "count of queued messages", PROPID_MGMT_QUEUE_MESSAGE_COUNT, VT_UI4 );
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::Int64
CMsmqPubSubClientTopic::GetCurrentByteCountOfMesssagesInQueue( MsmqQueue& q ) 
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    return GetQueueMetric( q, "byte count of messages", PROPID_MGMT_QUEUE_BYTES_IN_QUEUE, VT_UI4 );
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::Int64
CMsmqPubSubClientTopic::GetCurrentNrOfMessagesInQueueJournal( MsmqQueue& q ) 
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    return GetQueueMetric( q, "count of messages in journal", PROPID_MGMT_QUEUE_JOURNAL_MESSAGE_COUNT, VT_UI4 );
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::Int64
CMsmqPubSubClientTopic::GetCurrentByteCountOfMesssagesInQueueJournal( MsmqQueue& q ) 
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    return GetQueueMetric( q, "byte count of messages in journal", PROPID_MGMT_QUEUE_BYTES_IN_JOURNAL, VT_UI4 );
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::GetMsmqQueueProperty( const std::wstring& queueFormatName ,
                                              UInt32 propId                       ,
                                              UInt32 propType                     ,
                                              CORE::CVariant& qProperty           )
{GUCEF_TRACE;

    // Validate the input string.  
    if ( queueFormatName.empty() ) 
        return false;  

    // Define the maximum number of queue properties and a property counter.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId=0;  
  
    // Define a queue property structure.  
    MQQUEUEPROPS   QueueProps;  
    QUEUEPROPID    aQueuePropId[ NUMBEROFPROPERTIES ];  
    MQPROPVARIANT  aQueuePropVar[ NUMBEROFPROPERTIES ];  
    HRESULT        aQueuePropStatus[ NUMBEROFPROPERTIES ];  
    HRESULT queryResultCode = MQ_OK;  
  
    // Specify the property we want to read
    aQueuePropId[ cPropId ] = propId;                // Property ID  
    aQueuePropVar[ cPropId ].vt = propType;          // Type indicator  
    cPropId++;  
  
    // Initialize the MQQUEUEPROPS structure.  
    QueueProps.cProp = cPropId;  
    QueueProps.aPropID = aQueuePropId;  
    QueueProps.aPropVar = aQueuePropVar;                                     
    QueueProps.aStatus = aQueuePropStatus;  
 
    // Get the queue properties.  
    queryResultCode = ::MQGetQueueProperties( queueFormatName.c_str(), &QueueProps );  
    if ( FAILED( queryResultCode ) )  
    {  
        CORE::UInt32 errorCode =  HRESULT_CODE( queryResultCode );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:GetMsmqQueueProperty: Failed to obtain queue quota for queue format name \"" + 
            CORE::ToString( queueFormatName ) + "\". HRESULT=" + CORE::ToString( queryResultCode ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
        return false; 
    } 
    
    CORE::CAsciiString guidConversionTmp;
    bool success = MsmqPropertyToVariantStatic( aQueuePropVar[ 0 ], qProperty, false, 0, true, true, guidConversionTmp );
    if ( !success )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:GetMsmqQueueProperty: Failed to convert MSMQ property to variant for queue format name \"" +  CORE::ToString( queueFormatName ) + "\"" ) ; 
    }

    VARTYPE outputType = GetMsmqVariantTypeForMsmqProperty( propId );
    switch ( outputType )
    {
        case VT_LPSTR:  { MQFreeMemory( aQueuePropVar[ 0 ].pszVal ); break; }
        case VT_LPWSTR: { MQFreeMemory( aQueuePropVar[ 0 ].pwszVal ); break; }
        default: { }
    }
    
    return success;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::GetMsmqQueueQuota( const std::wstring& queueFormatName ,
                                           UInt64& queueQuota                  )

{GUCEF_TRACE;

    CORE::CVariant qProperty;
    bool success = GetMsmqQueueProperty( queueFormatName, PROPID_Q_QUOTA, VT_NULL, qProperty );
    if ( success )
    {
        queueQuota = qProperty.AsUInt64();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::GetMsmqQueueLabel( const std::wstring& queueFormatName  ,
                                           CORE::CString& queueLabel            )
{GUCEF_TRACE;

    CORE::CVariant qProperty;
    bool success = GetMsmqQueueProperty( queueFormatName, PROPID_Q_LABEL, VT_NULL, qProperty );
    if ( success )
    {
        queueLabel = qProperty.AsString();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::GetMsmqQueuePathName( const std::wstring& queueFormatName  ,
                                              CORE::CString& queuePathName         )
{GUCEF_TRACE;

    CORE::CVariant qProperty;
    bool success = GetMsmqQueueProperty( queueFormatName, PROPID_Q_PATHNAME, VT_NULL, qProperty );
    if ( success )
    {
        queuePathName = qProperty.AsString();
        return true;
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::GetMsmqQueuePathNameDNS( const std::wstring& queueFormatName  ,
                                                 CORE::CString& queuePathNameDNS      )
{GUCEF_TRACE;

    CORE::CVariant qProperty;
    bool success = GetMsmqQueueProperty( queueFormatName, PROPID_Q_PATHNAME_DNS, VT_NULL, qProperty );
    if ( success )
    {
        queuePathNameDNS = qProperty.AsString();
        return true;
    }
    return false; 
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::GetMsmqQueueType( const std::wstring& queueFormatName ,
                                          CORE::CString& queueTypeId          )
{GUCEF_TRACE;

    CORE::CVariant qProperty;
    bool success = false; /// GetMsmqQueueProperty( queueFormatName, PROPID_Q_TYPE, VT_CLSID, qProperty );
    if ( success )
    {
        queueTypeId = qProperty.AsString();
        return true;
    }
    return false; 
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::GetMsmqQueueProperties( const std::wstring& queueFormatName  ,
                                                MsmqQueueProperties& queueProperties )
{GUCEF_TRACE;

    // Validate the input string.  
    if ( queueFormatName.empty() ) 
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueueProperties: No queue format name was provided" ); 
        return false; 
    }

    // We query for each of the properties independently since we have no clue what type of queue we are 
    // actually dealing with. As such we best effort try to obtain properties, getting whatever we can get
    bool totalSuccess = true;
    totalSuccess = GetMsmqQueueLabel( queueFormatName, queueProperties.queueLabel ) && totalSuccess;
    //totalSuccess = GetMsmqQueueType( queueFormatName, queueProperties.typeId ) && totalSuccess;
    totalSuccess = GetMsmqQueueQuota( queueFormatName, queueProperties.quota ) && totalSuccess;
    totalSuccess = GetMsmqQueuePathName( queueFormatName, queueProperties.pathName ) && totalSuccess;
    totalSuccess = GetMsmqQueuePathNameDNS( queueFormatName, queueProperties.pathNameDNS ) && totalSuccess;

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CMsmqPubSubClientTopic::InitQueueInfo( MsmqQueue& q, const CMsmqPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    q.queueName = config.topicName;
    q.msmqQueueFormatName = GetMsmqQueueFormatName( q );
    q.metricFriendlyQueueName = GenerateMetricsFriendlyTopicName( q.queueName );    
    if ( !q.msmqQueueFormatName.empty() )
    {
        GetMsmqQueueProperties( q.msmqQueueFormatName, q.queueProperties );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqPubSubClientTopic:InitQueueInfo: Obtained properties of MSMQ queue with name \"" + q.queueName + 
            "\" as follows: " + q.queueProperties.ToString() );

        if ( !q.queueProperties.pathName.IsNULLOrEmpty() && "private" != q.queueProperties.pathName && "private" != q.queueProperties.queueLabel )
            q.metricFriendlyQueueName = GenerateMetricsFriendlyTopicName( q.queueProperties.pathName );
    
        if ( ( q.queueProperties.pathName.Lowercase().HasSubstr( "private" ) >= 0 ) ||
             ( q.queueProperties.queueLabel.Lowercase().HasSubstr( "private" ) >= 0 ) )
            q.isPrivateQueue = true;
        else
            q.isPrivateQueue = false;
    }

    return true; // best effort is fine
}

/*-------------------------------------------------------------------------*/

void
CMsmqPubSubClientTopic::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return;    
    
    // If m_msmqMsgsQueued is -1 dont bother trying again as we cannot solve config/permission issues here
    // in order to efficiently get this metric you need elevated access to the queue
    // We assume the same would apply to the other similar metrics since they have the same restriction
    if ( m_metrics.msmqMsgsInQueue >= 0 )
    {
        m_metrics.msmqMsgsInQueue = GetCurrentNrOfMessagesInQueue( m_queue );
        m_metrics.msmqMsgBytesInQueue = GetCurrentByteCountOfMesssagesInQueue( m_queue );
        m_metrics.msmqMsgsInJournal = GetCurrentNrOfMessagesInQueueJournal( m_queue );
        m_metrics.msmqMsgBytesInJournal = GetCurrentByteCountOfMesssagesInQueueJournal( m_queue );
    }

    const PUBSUB::CPubSubClientConfig& clientConfig = m_client->GetConfig();
    
    if ( clientConfig.desiredFeatures.supportsPublishing )
    {
        m_metrics.msmqMessagesPublished = GetMsmqMessagesPublishedCounter( true );
        m_metrics.msmqErrorsOnPublish = GetMsmqErrorsOnPublishCounter( true );
    }
    if ( clientConfig.desiredFeatures.supportsSubscribing )
    {
        m_metrics.msmqMessagesReceived = GetMsmqMessagesReceivedCounter( true );
        m_metrics.msmqErrorsOnReceive = GetMsmqErrorsOnReceiveCounter( true );
        m_metrics.msmqErrorsOnAck = GetMsmqErrorsOnAckCounter( true ); 
    }
    if ( m_config.gatherMsmqTransitTimeOnReceiveMetric )
    {
        m_metrics.msmqMsgSentToArriveLatencies = m_msmqMsgSentToArriveLatencies;
        m_msmqMsgSentToArriveLatencies.clear();
    }
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CMsmqPubSubClientTopic::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

 MT::TLockStatus
 CMsmqPubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CMsmqPubSubClientTopic::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

bool 
CMsmqPubSubClientTopic::IsSubscriptionAtEndOfData( void ) const
{GUCEF_TRACE;

    return m_subscriptionIsAtEndOfData;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CMsmqPubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::MSMQ::CMsmqPubSubClientTopic";
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
