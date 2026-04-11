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

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENT_H
#include "pubsubpluginSTORAGE_CStoragePubSubClient.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENT_H ? */

#include "pubsubpluginSTORAGE_CStoragePubSubClientTopic.h"

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXREADER_H
#include "pubsubpluginSTORAGE_CStoragePubSubIndexReader.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXREADER_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXREADER_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKDATA_H
#include "pubsubpluginSTORAGE_CStorageReplayTaskData.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKDATA_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKDATA_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKCONSUMER_H
#include "pubsubpluginSTORAGE_CStorageReplayTaskConsumer.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKCONSUMER_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKCONSUMER_H ? */

#undef MoveFile

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

#define GUCEF_DEFAULT_DEFAULT_NR_OF_SWAP_BUFFERS                    2
#define GUCEF_DEFAULT_VFS_CONTAINER_FILE_EXTENSION                  ".pubsubmsgs"
#define GUCEF_DEFAULT_NOACK_RETRANSMIT_CHECK_CYCLETIME_IN_MS        30000

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::CContainerRangeInfo::CContainerRangeInfo( const CORE::CString& vfsPath )
    : CStorageBookmarkInfo( vfsPath )
    , CORE::CTSharedObjCreator< CContainerRangeInfo, MT::CNoLock >( this )
    , hasEndDelimiter( false )
    , lastMsgIndex( 0 )
    , lastOffsetInFile( 0 )
    , minActionId( 0 )
    , maxActionId( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::CContainerRangeInfo::CContainerRangeInfo( const CContainerRangeInfo& src )
    : CStorageBookmarkInfo( src )
    , CORE::CTSharedObjCreator< CContainerRangeInfo, MT::CNoLock >( this )
    , hasEndDelimiter( src.hasEndDelimiter )
    , lastMsgIndex( src.lastMsgIndex )
    , lastOffsetInFile( src.lastOffsetInFile )
    , minActionId( src.minActionId )
    , maxActionId( src.maxActionId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::CContainerRangeInfo::CContainerRangeInfo( const CStorageBookmarkInfo& src )
    : CStorageBookmarkInfo( src )
    , CORE::CTSharedObjCreator< CContainerRangeInfo, MT::CNoLock >( this )
    , hasEndDelimiter( false )
    , lastMsgIndex( 0 )
    , lastOffsetInFile( 0 )
    , minActionId( 0 )
    , maxActionId( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::CContainerRangeInfo::operator<( const CContainerRangeInfo& other ) const
{GUCEF_TRACE;

    if ( vfsFilePath == other.vfsFilePath )
    {
        return msgIndex < other.msgIndex;
    }
    else
    {
        return vfsFilePath < other.vfsFilePath;
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::CContainerRangeInfo::operator==( const CContainerRangeInfo& other ) const
{GUCEF_TRACE;

    return CStorageBookmarkInfo::operator==( other ) &&
           ( hasEndDelimiter && other.hasEndDelimiter ) &&
           ( lastMsgIndex == other.lastMsgIndex ) &&
           ( lastOffsetInFile == other.lastOffsetInFile );
}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::CContainerRangeInfo&
CStoragePubSubClientTopic::CContainerRangeInfo::operator=( const CContainerRangeInfo& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CStorageBookmarkInfo::operator=( src );
        hasEndDelimiter = src.hasEndDelimiter;
        lastMsgIndex = src.lastMsgIndex;
        lastOffsetInFile = src.lastOffsetInFile;
        minActionId = src.minActionId;
        maxActionId = src.maxActionId;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::CContainerRangeInfo&
CStoragePubSubClientTopic::CContainerRangeInfo::operator=( const CStorageBookmarkInfo& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CStorageBookmarkInfo::operator=( src );
        hasEndDelimiter = false;
        lastMsgIndex = 0;
        lastOffsetInFile = 0;
        minActionId = 0;
        maxActionId = 0;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::StorageToPubSubRequest::StorageToPubSubRequest( void )
    : startDt()
    , endDt()
    , vfsPubSubMsgContainersToPush()
    , vfsPubSubMsgContainersPushed()
    , vfsPubSubMsgContainersTransmitted()
    , okIfZeroContainersAreFound( true )
    , isPersistentRequest( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::StorageToPubSubRequest::StorageToPubSubRequest( const CORE::CDateTime& startDt         ,
                                                                           const CORE::CDateTime& endDt           ,
                                                                           bool okIfZeroContainersAreFoundDefault ,
                                                                           bool isPersistentRequestDefault        )
    : startDt( startDt )
    , endDt( endDt )
    , vfsPubSubMsgContainersToPush()
    , vfsPubSubMsgContainersPushed()
    , vfsPubSubMsgContainersTransmitted()
    , okIfZeroContainersAreFound( okIfZeroContainersAreFoundDefault )
    , isPersistentRequest( isPersistentRequestDefault )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::StorageToPubSubRequest::StorageToPubSubRequest( const StorageToPubSubRequest& src )
    : startDt( src.startDt )
    , endDt( src.endDt )
    , vfsPubSubMsgContainersToPush( src.vfsPubSubMsgContainersToPush )
    , vfsPubSubMsgContainersPushed( src.vfsPubSubMsgContainersPushed )
    , vfsPubSubMsgContainersTransmitted( src.vfsPubSubMsgContainersTransmitted )
    , okIfZeroContainersAreFound( src.okIfZeroContainersAreFound )
    , isPersistentRequest( src.isPersistentRequest )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStoragePubSubClientTopic::StorageToPubSubRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "StorageToPubSubRequest";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStoragePubSubClientTopic::GenerateDefaultVfsStorageContainerFileExt( void )
{GUCEF_TRACE;

    return ".v" + CORE::ToString( PUBSUB::CPubSubMsgContainerBinarySerializer::CurrentFormatVersion ) + GUCEF_DEFAULT_VFS_CONTAINER_FILE_EXTENSION;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStoragePubSubClientTopic::GenerateVfsStorageContainerFileExt( void )
{GUCEF_TRACE;

    return ".v" + CORE::ToString( PUBSUB::CPubSubMsgContainerBinarySerializer::CurrentFormatVersion ) + '.' + m_config.vfsFileExtention;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::StorageToPubSubRequest::SaveConfig( CORE::CDataNode & tree ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::StorageToPubSubRequest::LoadConfig( const CORE::CDataNode & treeroot )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::StorageBufferMetaData::StorageBufferMetaData( void )
    : msgOffsetIndex()
    , msgs()
    , pubsubMsgsRefs()
    , msgsInFlight( 0 )
    , lastMsgTransmittedIndex( 0 )
    , msgAcks()
    , ackdMsgCount( 0 )
    , lastAckdMsgIndex( 0 )
    , lastAckdMsgTime( CORE::CDateTime::PastMax )
    , actionIds()
    , linkedRequest( GUCEF_NULL )
    , linkedRequestEntry()
    , isReleased( true )
    , isBeingWritten( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::StorageBufferMetaData::Clear( void )
{GUCEF_TRACE;

    msgOffsetIndex.clear();
    msgs.clear();
    pubsubMsgsRefs.clear();
    msgsInFlight = 0;
    lastMsgTransmittedIndex = 0;
    msgAcks.clear();
    ackdMsgCount = 0;
    lastAckdMsgIndex = 0;
    lastAckdMsgTime = CORE::CDateTime::PastMax;
    actionIds.clear();
    linkedRequest = GUCEF_NULL;
    linkedRequestEntry.Unlink();
    isReleased = true;
    isBeingWritten = false;
}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::CStoragePubSubClientTopic( CStoragePubSubClient* client           ,
                                                      CORE::PulseGeneratorPtr pulseGenerator )
    : PUBSUB::CPubSubClientTopic( pulseGenerator )
    , CORE::CTSharedObjCreator< CStoragePubSubClientTopic, MT::CMutex >( this )
    , m_client( client )
    , m_pubsubMsgs()
    , m_config()
    , m_syncVfsOpsTimer( GUCEF_NULL )
    , m_reconnectTimer( GUCEF_NULL )
    , m_noAckRetransmitTimer( GUCEF_NULL )
    , m_bufferContentTimeWindowCheckTimer( GUCEF_NULL )
    , m_lock()
    , m_currentPublishActionId( 1 )
    , m_currentReceiveActionId( 1 )
    , m_publishSuccessActionIds()
    , m_publishSuccessActionEventData()
    , m_publishFailureActionIds()
    , m_publishFailureActionEventData()
    , m_metrics()
    , m_metricFriendlyTopicName()
    , m_isHealthy( true )
    , m_needToTrackAcks( true )
    , m_subscriptionIsAtEndOfData( false )
    , m_vfsInitIsComplete( false )
    , m_currentReadBuffer( GUCEF_NULL )
    , m_currentWriteBuffer( GUCEF_NULL )
    , m_currentBookmarkInfo()
    , m_currentBookmark()
    , m_pubsubBookmarkPersistence()
    , m_vfsFilePostfix( GenerateDefaultVfsStorageContainerFileExt() )
    , m_vfsRootPath()
    , m_vfsStorageRootPathForFullyAckdContainers()
    , m_lastPersistedMsgId()
    , m_lastPersistedMsgDt()
    , m_encodeSizeRatio( -1 )
    , m_stage0StorageToPubSubRequests()
    , m_stage1StorageToPubSubRequests()
    , m_stage2StorageToPubSubRequests()
    , m_buffers( GUCEF_DEFAULT_DEFAULT_NR_OF_SWAP_BUFFERS )
    , m_lastWriteBlockCompletion()
    , m_storageBufferMetaData()
    , m_vfsOpsThread()
    , m_msgsLoadedFromStorage( 0 )
    , m_msgBytesLoadedFromStorage( 0 )
    , m_msgsNotifiedAsReceived( 0 )
    , m_msgsWrittenToStorage( 0 )
    , m_msgBytesWrittenToStorage( 0 )
    , m_storageCorruptionDetections( 0 )
    , m_storageDeserializationFailures( 0 )
{GUCEF_TRACE;

    GUCEF_ASSERT( GUCEF_NULL != client );    
    
    // Check if an override pulse generator was provided
    if ( pulseGenerator.IsNULL() && GUCEF_NULL != client )
    {
        pulseGenerator = client->GetConfig().pulseGenerator;
        SetPulseGenerator( pulseGenerator );
    }
    
    m_publishSuccessActionEventData.LinkTo( &m_publishSuccessActionIds );
    m_publishFailureActionEventData.LinkTo( &m_publishFailureActionIds );

    m_syncVfsOpsTimer = GUCEF_NEW CORE::CTimer( pulseGenerator, 25 );

    if ( m_client->GetConfig().desiredFeatures.supportsAutoReconnect )
    {
        m_reconnectTimer = GUCEF_NEW CORE::CTimer( pulseGenerator, m_client->GetConfig().reconnectDelayInMs );
    }

    if ( m_client->GetConfig().desiredFeatures.supportsPublishing )
    {
        m_bufferContentTimeWindowCheckTimer = GUCEF_NEW CORE::CTimer( pulseGenerator, 1000 );
    }

    m_pubsubBookmarkPersistence = m_client->GetBookmarkPersistence();
    m_needToTrackAcks = m_client->IsTrackingAcksNeeded();

    if ( m_needToTrackAcks )
    {
        m_noAckRetransmitTimer = GUCEF_NEW CORE::CTimer( pulseGenerator, GUCEF_DEFAULT_NOACK_RETRANSMIT_CHECK_CYCLETIME_IN_MS );
    }

    RegisterEventHandlers( pulseGenerator );
}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::~CStoragePubSubClientTopic()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_client = GUCEF_NULL;

    Disconnect();

    GUCEF_DELETE m_syncVfsOpsTimer;
    m_syncVfsOpsTimer = GUCEF_NULL;

    m_vfsOpsThread.Unlink();

    GUCEF_DELETE m_reconnectTimer;
    m_reconnectTimer = GUCEF_NULL;

    GUCEF_DELETE m_noAckRetransmitTimer;
    m_noAckRetransmitTimer = GUCEF_NULL;

    GUCEF_DELETE m_bufferContentTimeWindowCheckTimer;
    m_bufferContentTimeWindowCheckTimer = GUCEF_NULL;

    if ( !m_journal.IsNULL() )
    {
        m_journal->AddTopicDestroyedJournalEntry();
        m_journal.Unlink();
    }

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CStoragePubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

void 
CStoragePubSubClientTopic::RegisterPulseGeneratorEventHandlers( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    UnsubscribeFrom( GetPulseGenerator().GetPointerAlways() );
    TEventCallback callback( this, &CStoragePubSubClientTopic::OnPulseCycle );
    SubscribeTo( pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent ,
                 callback                         );
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::RegisterEventHandlers( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_reconnectTimer )
    {
        TEventCallback callback( this, &CStoragePubSubClientTopic::OnReconnectTimerCycle );
        SubscribeTo( m_reconnectTimer               ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    if ( GUCEF_NULL != m_syncVfsOpsTimer )
    {
        TEventCallback callback( this, &CStoragePubSubClientTopic::OnSyncVfsOpsTimerCycle );
        SubscribeTo( m_syncVfsOpsTimer              ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    if ( GUCEF_NULL != m_bufferContentTimeWindowCheckTimer )
    {
        TEventCallback callback( this, &CStoragePubSubClientTopic::OnBufferContentTimeWindowCheckCycle );
        SubscribeTo( m_bufferContentTimeWindowCheckTimer ,
                     CORE::CTimer::TimerUpdateEvent      ,
                     callback                            );
    }

    if ( GUCEF_NULL != m_noAckRetransmitTimer )
    {
        TEventCallback callback( this, &CStoragePubSubClientTopic::OnNoAckRetransmitCheckCycle );
        SubscribeTo( m_noAckRetransmitTimer         ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    TEventCallback callback( this, &CStoragePubSubClientTopic::OnVfsInitializationCompleted );
    SubscribeTo( &vfs                                       ,
                 VFS::CVFS::VfsInitializationCompletedEvent ,
                 callback                                   );
    m_vfsInitIsComplete = VFS::CVfsGlobal::Instance()->GetVfs().IsInitialized();

    TEventCallback callback2( this, &CStoragePubSubClientTopic::OnVfsArchiveMounted );
    SubscribeTo( &vfs                           ,
                 VFS::CVFS::ArchiveMountedEvent ,
                 callback2                      );
    
    RegisterPulseGeneratorEventHandlers( pulseGenerator );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;

    return m_config.needPublishSupport;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;

    return m_config.needSubscribeSupport;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStoragePubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.topicName;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::Publish( CORE::UInt64& publishActionId    ,
                                    const PUBSUB::CIPubSubMsg& msg  ,
                                    bool notify                      )
{GUCEF_TRACE;

    TIPubSubMsgConstRawPtrVector msgs;
    msgs.push_back( &msg );

    TPublishActionIdVector publishActionIds;
    bool result = Publish( publishActionIds, msgs, notify );

    if ( !publishActionIds.empty() )
    {
        publishActionId = publishActionIds[ 0 ];
    }

    return result;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds                   ,
                                    const PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector& msgs ,
                                    bool notify                                                )
{GUCEF_TRACE;

    CORE::UInt64 msgCount = msgs.size();
    TIPubSubMsgConstRawPtrVector msgsPtrs( msgCount );

    for ( CORE::UInt64 i=0; i<msgCount; ++i )
    {                                                                                               
        msgsPtrs[ i ] = &msgs[ i ];
    }

    return PublishViaMsgPtrs< const PUBSUB::CIPubSubMsg* >( publishActionIds, msgsPtrs, notify );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds ,
                                    const TIPubSubMsgConstRawPtrVector& msgs ,
                                    bool notify                              )
{GUCEF_TRACE;

    return PublishViaMsgPtrs< const PUBSUB::CIPubSubMsg* >( publishActionIds, msgs, notify );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds ,
                                    const TIPubSubMsgRawPtrVector& msgs      ,
                                    bool notify                              )
{GUCEF_TRACE;

    return PublishViaMsgPtrs< PUBSUB::CIPubSubMsg* >( publishActionIds, msgs, notify );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds ,
                                    const TIPubSubMsgSPtrVector& msgs        ,
                                    bool notify                              )
{GUCEF_TRACE;

    return PublishViaMsgPtrs< PUBSUB::CIPubSubMsg::TNoLockSharedPtr >( publishActionIds, msgs, notify );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::Publish( TPublishActionIdVector& publishActionIds ,
                                    const TPubSubMsgsRefVector& msgs         ,
                                    bool notify                              )
{GUCEF_TRACE;

    return PublishViaMsgPtrs< TPubSubMsgRef >( publishActionIds, msgs, notify );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CStoragePubSubClientTopic::PublishViaMsgPtrs( TPublishActionIdVector& publishActionIds           ,
                                              const std::vector< T, gucef_allocator< T > >& msgs ,
                                              bool notify                                        )
{GUCEF_TRACE;

    NotifyPublishResults();
    
    if ( !m_config.needPublishSupport )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
            "):PublishViaMsgPtrs: Attempt to publish messages on a topic that is not configured for publishing" );
        return false;
    }

    try
    {
        MT::CScopeMutex lock( m_lock );

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:PublishViaMsgPtrs: Given " + CORE::ToString( msgs.size() ) + " msgs" );

        TPublishActionIdVector::iterator publishActionIdIttr = publishActionIds.begin();
        typename std::vector< T, gucef_allocator< T > >::const_iterator i = msgs.begin();
        bool work = true;
        while ( work )
        {
            bool firstBlock = m_lastWriteBlockCompletion == CORE::CDateTime::Empty;
            if ( firstBlock )
            {
                m_lastWriteBlockCompletion = CORE::CDateTime::NowUTCDateTime();
            }

            StorageBufferMetaData* bufferMetaData = GUCEF_NULL;
            if ( GUCEF_NULL == m_currentWriteBuffer )
            {
                m_currentWriteBuffer = m_buffers.GetNextWriterBuffer( CORE::CDateTime::NowUTCDateTime(), m_config.performVfsOpsASync, GUCEF_MT_MEGA_LONG_LOCK_TIMEOUT );
                if ( GUCEF_NULL == m_currentWriteBuffer )
                {
                    if ( m_config.performVfsOpsASync )
                    {
                        // We should not get here with both the blocking & long timeout
                        // the async buffer reads should be allowing us to proceed in a reasonable timeframe
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
                            "):PublishViaMsgPtrs: Failed to obtain new message serialization buffer. There are " + CORE::ToString( m_buffers.GetNrOfBuffers() ) +
                            " buffers with " + CORE::ToString( m_buffers.GetBuffersQueuedToRead() ) + " waiting to write to storage" );
                        return false;
                    }
                    else
                    {
                        // Try to persist next ready to read buffer as part of the call chain, consequence of not doing async
                        if ( StoreNextReceivedPubSubBuffer( true ) )
                        {
                            // Try again to get a write buffer, this should typically work based on the prior operation
                            m_currentWriteBuffer = m_buffers.GetNextWriterBuffer( CORE::CDateTime::NowUTCDateTime(), m_config.performVfsOpsASync, GUCEF_MT_MEGA_LONG_LOCK_TIMEOUT );
                        }
                        if ( GUCEF_NULL == m_currentWriteBuffer )
                        {
                            lock.EarlyUnlock();
                            NotifyObservers( LocalPublishQueueFullEvent );
                            return false;
                        }
                    }
                }
                m_currentWriteBuffer->SetDataSize( 0 );

                CORE::UInt32 newBytesWritten = 0;
                if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::SerializeHeader( m_config.pubsubBinarySerializerOptions, 0, *m_currentWriteBuffer, newBytesWritten ) )
                {
                    // We carry on best effort but this is really bad
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
                        "):PublishViaMsgPtrs: Failed to write container header at start of new pub-sub msg container with associated start datetime of " + 
                        m_buffers.GetCurrenWriterBufferAssociatedDt().ToIso8601DateTimeString( true, true ) );
                }

                // We are (re)using a new buffer, as such make sure we reset the meta-data that may be pre-existing
                bufferMetaData = &( m_storageBufferMetaData[ m_currentWriteBuffer ] );
                bufferMetaData->msgOffsetIndex.clear();
                bufferMetaData->actionIds.clear();

                // Start the timer on content for this buffer
                if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_bufferContentTimeWindowCheckTimer )
                {
                    m_bufferContentTimeWindowCheckTimer->SetEnabled( true );
                }
            }
            else
            {
                // fetch the meta-data for this buffer
                bufferMetaData = &( m_storageBufferMetaData[ m_currentWriteBuffer ] );
            }

            CORE::UInt32 bufferOffset = m_currentWriteBuffer->GetDataSize();

            while ( i != msgs.end() )
            {
                // see if we have a pre-existing action id (retry)
                CORE::UInt64 publishActionId = 0;
                if ( publishActionIdIttr != publishActionIds.end() )
                    publishActionId = (*publishActionIdIttr);

                // Assign a new action id if we dont yet have one
                if ( 0 == publishActionId )
                {
                    publishActionId = m_currentPublishActionId;
                    ++m_currentPublishActionId;
                }

                // Persist the action id generated on the output param
                if ( publishActionIdIttr == publishActionIds.end() )
                {
                    publishActionIds.push_back( publishActionId );
                    publishActionIdIttr = publishActionIds.end();
                }
                else
                    (*publishActionIdIttr) = publishActionId;

                // Serialize the message
                CORE::UInt32 ticks = CORE::GUCEFGetTickCount();
                CORE::UInt32 msgBytesWritten = 0;
                const T& entry = (*i);
                const PUBSUB::CIPubSubMsg* msg = static_cast< const PUBSUB::CIPubSubMsg* >( &(*(entry)) );
                if ( GUCEF_NULL != msg )
                {
                    CORE::CDateTime dt = entry->GetMsgDateTime();  
                    
                    UInt64 msSinceUnixEpochInUtc = entry->GetMsgDateTime().ToUnixEpochBasedTicksInMillisecs();
                    
                    if ( PUBSUB::CPubSubMsgBinarySerializer::Serialize( m_config.pubsubBinarySerializerOptions, *msg, bufferOffset, *m_currentWriteBuffer, msgBytesWritten ) && 0 != msgBytesWritten )
                    {
                        #ifdef GUCEF_DEBUG_MODE
                        // Extra sanity check
                        if ( !bufferMetaData->msgOffsetIndex.empty() )
                        {
                            if ( bufferOffset < bufferMetaData->msgOffsetIndex[ bufferMetaData->msgOffsetIndex.size()-1 ] )
                            {
                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
                                    "):PublishViaMsgPtrs: Non-Ascending msg offset detected. publishActionId=" + CORE::ToString( publishActionId ) );
                            }
                        }
                        #endif

                        // add to our meta-data tracking
                        bufferMetaData->msgOffsetIndex.push_back( bufferOffset );
                        bufferMetaData->actionIds.push_back( publishActionId );

                        ticks = CORE::GUCEFGetTickCount() - ticks;
                        bufferOffset += msgBytesWritten;

                        GUCEF_DEBUG_LOG_EVERYTHING( "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
                            "):PublishViaMsgPtrs: Serialized a message with serialized size " + CORE::ToString( msgBytesWritten ) +
                            ". This took " + CORE::ToString( ticks ) + "ms. publishActionId=" + CORE::ToString( publishActionId ) );

                        // Check to see if we have gathered enough data or enough time has passed to consider the current container complete
                        bool maxContainerSizeReached = m_currentWriteBuffer->GetDataSize() >= m_config.desiredMinimalSerializedBlockSize;
                        bool timeLimitReached        = ( !firstBlock && m_lastWriteBlockCompletion.GetTimeDifferenceInMillisecondsToNow() >= m_config.desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs );
                        bool maxInFlightReached      = ( m_maxTotalMsgsInFlight > 0 && bufferMetaData->actionIds.size() >= (size_t) m_maxTotalMsgsInFlight );

                        if ( maxContainerSizeReached ||  // <- container byte size limit criterea
                             timeLimitReached        ||  // <- container max timespan limit criterea
                             maxInFlightReached       )  // <- respect max in-flight limit and dont block
                        {
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL,  "StoragePubSubClientTopic(" + CORE::ToString( this ) +
                                "):PublishViaMsgPtrs: Finalizing write buffer. maxContainerSizeReached=" +
                                CORE::ToString( maxContainerSizeReached ) + ", timeLimitReached=" + 
                                CORE::ToString( timeLimitReached ) + ", maxInFlightReached=" + CORE::ToString( maxInFlightReached ) + 
                                ", action IDs in block = " + CORE::ToString( bufferMetaData->actionIds.size() ) );

                            // The current container is now considered to have enough content.
                            // Let's wrap things up...
                            FinalizeWriteBuffer( bufferMetaData, bufferOffset, false );
                            ++i; break;
                        }
                    }
                    else
                    {
                        // End of the road for this message
                        m_publishFailureActionIds.push_back( publishActionId );

                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
                            "):PublishViaMsgPtrs: Failed to serialize a message. publishActionId=" + CORE::ToString( publishActionId ) );
                    }                    
                }
                else
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::ToString( this ) +
                        "):PublishViaMsgPtrs: Null message passed. publishActionId=" + CORE::ToString( publishActionId ) );
                }

                ++i;
            }

            if ( i == msgs.end() )
                work = false;
        }
        return true;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) + "):PublishViaMsgPtrs: exception: " + CORE::CString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::FinalizeWriteBuffer( StorageBufferMetaData* bufferMetaData, CORE::UInt32 bufferOffset, bool onlyIfBufferHasContent )
{GUCEF_TRACE;

    if ( onlyIfBufferHasContent && bufferMetaData->msgOffsetIndex.empty() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
            "):FinalizeWriteBuffer: Current buffer does not yet have any content, skipping finalization" );
        return false;
    }
    
    CORE::UInt32 newBytesWritten = 0;
    if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::SerializeFooter( bufferMetaData->msgOffsetIndex, bufferOffset, *m_currentWriteBuffer, newBytesWritten ) )
    {
        // We carry on best effort but this is really bad
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
            "):FinalizeWriteBuffer: Failed to write container footer at end of current pub-sub msg container" );
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
        "):FinalizeWriteBuffer: Completed a serialized msg data block of size " + CORE::ToString( bufferOffset ) );

    // prevent further write action on this buffer and cause a new write buffer to be requested
    m_currentWriteBuffer = GUCEF_NULL;

    // Now that the async threaded work is done signal that we are finished writing
    // this releases the write buffer for reading as a read buffer
    m_buffers.SignalEndOfWriting();
    bufferMetaData->isBeingWritten = false;

    m_lastWriteBlockCompletion = CORE::CDateTime::NowUTCDateTime();
    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_bufferContentTimeWindowCheckTimer )
        m_bufferContentTimeWindowCheckTimer->SetEnabled( false );

    if ( !m_config.performVfsOpsASync )
    {
        // Try to persist next ready to read buffer as part of the call chain, consequence of not doing async
        StoreNextReceivedPubSubBuffer( true );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::FinalizeWriteBuffer( bool onlyIfBufferHasContent )
{GUCEF_TRACE;

    CORE::UInt32 bufferOffset = m_currentWriteBuffer->GetDataSize();
    StorageBufferMetaData* bufferMetaData = &( m_storageBufferMetaData[ m_currentWriteBuffer ] );
    return FinalizeWriteBuffer( bufferMetaData, bufferOffset, onlyIfBufferHasContent );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SetupToSubscribe( PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( LoadConfig( config ) )
    {

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    config = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStoragePubSubClientTopic::GenerateMetricsFriendlyTopicName( const CORE::CString& topicName )
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

CORE::CString
CStoragePubSubClientTopic::ResolveVfsRootPath( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
    {
        CORE::CString vfsRootPath = m_config.vfsStorageRootPath.ReplaceSubstr( "{pubsubIdPrefix}", m_client->GetConfig().pubsubIdPrefix );
        vfsRootPath = vfsRootPath.ReplaceSubstr( "{topicName}", m_config.topicName );
        vfsRootPath = vfsRootPath.ReplaceSubstr( "{metricsFriendlyTopicName}", m_metricFriendlyTopicName );
        return vfsRootPath;
    }
    return  CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStoragePubSubClientTopic::ResolveVfsStorageRootPathForFullyAckdContainers( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
    {
        CORE::CString vfsRootPath = m_config.vfsStorageRootPathForFullyAckdContainers.ReplaceSubstr( "{pubsubIdPrefix}", m_client->GetConfig().pubsubIdPrefix );
        vfsRootPath = vfsRootPath.ReplaceSubstr( "{topicName}", m_config.topicName );
        vfsRootPath = vfsRootPath.ReplaceSubstr( "{metricsFriendlyTopicName}", m_metricFriendlyTopicName );
        return vfsRootPath;
    }
    return  CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    return LoadConfigType< PUBSUB::CPubSubClientTopicConfig >( config );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::LoadConfig( const CStoragePubSubClientTopicConfig& config )
{GUCEF_TRACE;
    
    return LoadConfigType< CStoragePubSubClientTopicConfig >( config );
}

/*-------------------------------------------------------------------------*/

template <class C>
bool 
CStoragePubSubClientTopic::LoadConfigType( const C& config )
{GUCEF_TRACE;

    // Sanity check the config
    if ( config.needPublishSupport && config.needSubscribeSupport )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::ToString( this ) + "):LoadConfig: This backend does not support publishing and subscribing on the same topic. Both are currently selected" );
        return false;
    }
    if ( !config.needPublishSupport && !config.needSubscribeSupport )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::ToString( this ) + "):LoadConfig: Neither publishing nor subscribing is selected for the topic, this is invalid" );
        return false;
    }

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
        return false;

    m_config = config;
    m_metricFriendlyTopicName = GenerateMetricsFriendlyTopicName( m_config.topicName );
    m_vfsFilePostfix = GenerateVfsStorageContainerFileExt();
    m_vfsRootPath = ResolveVfsRootPath();
    m_vfsStorageRootPathForFullyAckdContainers = ResolveVfsStorageRootPathForFullyAckdContainers();     

    if ( m_config.useTopicLevelMaxTotalMsgsInFlight )
        m_maxTotalMsgsInFlight = m_config.maxTotalMsgsInFlight;
    else
        m_maxTotalMsgsInFlight = m_client->GetConfig().maxTotalMsgsInFlight;

    if ( GUCEF_NULL != m_noAckRetransmitTimer )
        m_noAckRetransmitTimer->SetInterval( m_config.nonAckdMsgCheckIntervalInMs );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    return StopVfsOps();
}

/*-------------------------------------------------------------------------*/

void 
CStoragePubSubClientTopic::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_syncVfsOpsTimer )
    {
        m_syncVfsOpsTimer->SetPulseGenerator( newPulseGenerator );
    }
    if ( GUCEF_NULL != m_reconnectTimer )
    {
        m_reconnectTimer->SetPulseGenerator( newPulseGenerator );
    }
    if ( GUCEF_NULL != m_noAckRetransmitTimer )
    {
        m_noAckRetransmitTimer->SetPulseGenerator( newPulseGenerator );
    }
    if ( GUCEF_NULL != m_bufferContentTimeWindowCheckTimer )
    {
        m_bufferContentTimeWindowCheckTimer->SetPulseGenerator( newPulseGenerator );
    }
    
    PUBSUB::CPubSubClientTopic::SetPulseGenerator( newPulseGenerator );

    RegisterPulseGeneratorEventHandlers( newPulseGenerator );
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr 
CStoragePubSubClientTopic::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return PUBSUB::CPubSubClientTopic::GetPulseGenerator();
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::BeginVfsOps( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
        return false;

    if ( !m_config.performVfsOpsASync )
    {
        // If we do not perform VFS Ops Async then we use less system resources but we do run the risk
        // of filling up our write queues etc and causing performance issues in the pipeline
        // This is a performance tradeoff specific to your use-case
        if ( GUCEF_NULL != m_syncVfsOpsTimer )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::ToString( this ) + "):BeginVfsOps: Will use timer for sync VFS ops" );
            return m_syncVfsOpsTimer->SetEnabled( true );
        }
        return false;
    }
    else
    {
        // Do we want a dedicated thread per topic or will a more general threadpool with intermixed work suffice
        // This is a performance tradeoff specific to your use-case
        //      Do you have extra threading capacity available on the node?
        //      Is it important we read/write from/to VFS as fast as possible?
        //      Etc.
        if ( m_config.performVfsOpsInDedicatedThread )
        {
            if ( m_vfsOpsThread.IsNULL() )
            {
                m_vfsOpsThread = CStoragePubSubClientTopicVfsTaskPtr( GUCEF_NEW CStoragePubSubClientTopicVfsTask( this ) );
            }
            if ( !m_vfsOpsThread.IsNULL() )
            {
                CORE::ThreadPoolPtr threadPool = m_client->GetThreadPool();
                if ( threadPool.IsNULL() )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StoragePubSubClientTopic(" + CORE::ToString( this ) + "):BeginVfsOps: Failed to obtain dedicated thread pool for dedicated VFS thread for async operations. Falling back to global thread pool" );
                    threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();
                    if ( threadPool.IsNULL() )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) + "):BeginVfsOps: Failed to obtain global thread pool for dedicated VFS thread for async operations" );
                        return false;
                    }
                }
                CORE::CFutureResult result = threadPool->StartTaskWithConsumer( m_vfsOpsThread ); 
                if ( result.HasAFuture() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::ToString( this ) + "):BeginVfsOps: Started dedicated per-topic thread for async VFS ops" );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StoragePubSubClientTopic(" + CORE::ToString( this ) + "):BeginVfsOps: Failed to start dedicated VFS thread for async operations" );
                    return false;
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StoragePubSubClientTopic(" + CORE::ToString( this ) + "):BeginVfsOps: Failed to create dedicated VFS task (thread) for async operations" );
                return false;
            }
        }
        else
        {
            // Seems we want async but not go as far as spinning up a dedicated thread per topic to handle VFS operations
            // we will rely on defining 'work' tasks as work arrives
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::StopVfsOps( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    bool success = true;

    if ( GUCEF_NULL != m_syncVfsOpsTimer )
    {
        // we intentionally dont use the return value of setting the timer because we just need to store the state, the pulse generator may be missing
        m_syncVfsOpsTimer->SetEnabled( false );

        m_buffers.SignalEndOfWriting();
    }

    if ( !m_vfsOpsThread.IsNULL() )
        success = m_vfsOpsThread->RequestTaskToStop( false ) && success;

    return success;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::AddVfsFileToSubscription( const CORE::CString& vfsFilePath )
{GUCEF_TRACE;

    CORE::CDateTime containerFileFirstMsgDt;
    CORE::CDateTime containerFileLastMsgDt;
    CORE::CDateTime containerFileCaptureDt;
    if ( GetTimestampsFromContainerFilename( vfsFilePath, containerFileFirstMsgDt, containerFileLastMsgDt, containerFileCaptureDt ) )
    {
        // Now that we have the relevant timestamps we check these against the ranges of the persistent requests
        // this is on a first come first serve basis if we find a match

        StorageToPubSubRequestDeque::iterator i = m_stage0StorageToPubSubRequests.begin();
        while ( i != m_stage0StorageToPubSubRequests.end() )
        {
            // Check to see if the file is in range
            const StorageToPubSubRequest& persistentRequest = (*i);                        
            bool firstMsgIsInRange = containerFileFirstMsgDt.IsWithinRange( persistentRequest.startDt, persistentRequest.endDt );
            bool lastMsgIsInRange = containerFileLastMsgDt.IsWithinRange( persistentRequest.startDt, persistentRequest.endDt );

            if ( firstMsgIsInRange || lastMsgIsInRange )
            {
                StorageToPubSubRequest newAutoRequest( persistentRequest );
                newAutoRequest.isPersistentRequest = false;
                newAutoRequest.startDt = containerFileFirstMsgDt;
                newAutoRequest.endDt = containerFileLastMsgDt;
                newAutoRequest.okIfZeroContainersAreFound = false;

                TCContainerRangeInfoPtr bookmarkInfo = CContainerRangeInfo::CreateSharedObj();
                bookmarkInfo->vfsFilePath = vfsFilePath;
                newAutoRequest.vfsPubSubMsgContainersToPush.insert( bookmarkInfo );

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AddVfsFileToSubscription: auto-added file " + vfsFilePath +
                    " as request based on persistent storage request" );

                m_subscriptionIsAtEndOfData = false;
                m_stage2StorageToPubSubRequests.push_back( newAutoRequest );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "StoragePubSubClientTopic:AddVfsFileToSubscription: Ignoring new file for persistent storage request with time range from=" + 
                        CORE::ToString( persistentRequest.startDt ) + " to=" + CORE::ToString( persistentRequest.endDt ) + 
                        ". File represents time range: from=" + 
                        CORE::ToString( containerFileFirstMsgDt ) + " to=" + CORE::ToString( containerFileLastMsgDt ) );
            }
            ++i;
        }
    }
    else
    {
        // could be some other file that is not a container file
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AddVfsFileToSubscription: Ignoring new file because of inability to extract timestamps from supposed container file name: " + vfsFilePath );
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnVfsFileCreated( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventid  ,
                                             CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    CORE::CDirectoryWatcherEvents::TFileCreatedEventData* fileCreatedEventData = static_cast< CORE::CDirectoryWatcherEvents::TFileCreatedEventData* >( eventdata );
    if ( GUCEF_NULL != fileCreatedEventData )
    {
        CORE::CString vfsNewFilePath = fileCreatedEventData->GetData();

        // Since VFS events are global lets make sure this relates to a path we actually care about
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        if ( vfs.IsFilePathSubOfDirPath( m_vfsRootPath, vfsNewFilePath ) )
        {
            CORE::CString newFileExt = CORE::ExtractFileExtention( vfsNewFilePath );
            if ( newFileExt == m_config.vfsFileExtention )
            {            
                // We have a new file that matches our topic's file extension
                // We should add it to our subscription list
                AddVfsFileToSubscription( vfsNewFilePath );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:OnVfsFileCreated: Ignoring new file because its file extension doesnt match the configured one: " + vfsNewFilePath );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnVfsArchiveMounted( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventid  ,
                                                CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // new archives can be dynamically mounted
    // as such we need to be on the lookout for such events and if we have any
    // persistent storage requests set up a new dir watch for new files IF
    // the archive's mount location matches our vfs root

    VFS::CVFS::TArchiveMountedEventData* archiveMountedEventData = static_cast< VFS::CVFS::TArchiveMountedEventData* >( eventdata );
    if ( GUCEF_NULL != archiveMountedEventData )
    {
        MT::CScopeMutex lock( m_lock );
        
        if ( !m_stage0StorageToPubSubRequests.empty() )
        {
            if ( m_vfsRootPath.HasSubstr( *archiveMountedEventData, true ) )
            {
                // redo our subscription since the relevant archive landscape changed
                SubscribeToVfsTopicPathEvents();
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnVfsFileRenamed( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CDirectoryWatcherEvents::TFileRenamedEventData* fileRenamedEventData = static_cast< CORE::CDirectoryWatcherEvents::TFileRenamedEventData* >( eventData );
    if ( GUCEF_NULL != fileRenamedEventData )
    {
        const CORE::CString& oldFilename = fileRenamedEventData->GetData().oldFilename;
        const CORE::CString& newFilename = fileRenamedEventData->GetData().newFilename;

        // Since VFS events are global lets make sure this relates to a path we actually care about
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        if ( vfs.IsFilePathSubOfDirPath( m_vfsRootPath, newFilename ) )
        {
            CORE::CString oldFileExt = CORE::ExtractFileExtention( oldFilename );
            CORE::CString newFileExt = CORE::ExtractFileExtention( newFilename );

            MT::CScopeMutex lock( m_lock );
        
            if ( oldFileExt != newFileExt )
            { 
                // the file extension was changed. this could change the eligibility of the file for our topic
                if ( newFileExt == m_config.vfsFileExtention )
                {
                    // This file has now become eligible for our topic
                    AddVfsFileToSubscription( newFilename );
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnVfsDirRenamed( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // if the underlying vfs storage dir is renamed we should update our config accordingly
    CORE::CDirectoryWatcherEvents::TDirRenamedEventData* dirRenamedEventData = static_cast< CORE::CDirectoryWatcherEvents::TDirRenamedEventData* >( eventData );
    if ( GUCEF_NULL != dirRenamedEventData )
    {
        const CORE::CString& oldDirPath = dirRenamedEventData->GetData().oldDirName;
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        
        MT::CScopeMutex lock( m_lock );
        
        // Check to see if this renamed dir is the storage root, it could be an irrelevant dir
        if ( m_vfsRootPath == oldDirPath )
        {
            // Do we take action for new dirs that are created in the storage root ?
            if ( m_config.topicFollowsDirRenames )
            {
                const CORE::CString& newDirPath = dirRenamedEventData->GetData().newDirName;
                
                m_vfsRootPath = newDirPath;
                
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):OnVfsDirRenamed: Topic is auto following dir rename from \"" +
                    oldDirPath + " to \"" + newDirPath + "\"");

                // @TODO: Currently the topic name itself is not renamed, this is a future feature
                // we would need to add topic rename support
            }
            else
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClient(" + CORE::ToString( this ) + "):OnVfsDirRenamed: Underlying dir was renamed and per config the topic will not follow. It will be functionally 'disconnected'" );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SubscribeToVfsTopicPathEvents( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    CORE::CIDirectoryWatcher::CDirWatchOptions dirWatchOptions( false );
    dirWatchOptions.watchForFileCreation = true;
    dirWatchOptions.watchForFileRenames = true;
    dirWatchOptions.watchForDirRenames = m_config.topicFollowsDirRenames;

    TEventCallback callback( this, &CStoragePubSubClientTopic::OnVfsFileCreated );
    SubscribeTo( &vfs, VFS::CVFS::FileCreatedEvent, callback );

    TEventCallback callback2( this, &CStoragePubSubClientTopic::OnVfsDirRenamed );
    SubscribeTo( &vfs, VFS::CVFS::DirRenamedEvent, callback2 );

    TEventCallback callback3( this, &CStoragePubSubClientTopic::OnVfsFileRenamed );
    SubscribeTo( &vfs, VFS::CVFS::FileRenamedEvent, callback3 );

    //SubscribeTo( &vfs, VFS::CVFS::FileModifiedEvent );
    //SubscribeTo( &vfs, VFS::CVFS::FileRenamedEvent );
    //SubscribeTo( &vfs, VFS::CVFS::FileDeletedEvent );

    return vfs.AddDirToWatch( m_vfsRootPath, dirWatchOptions );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::AddStorageToPubSubRequest( const StorageToPubSubRequest& request )
{GUCEF_TRACE;

    if ( m_config.needPublishSupport || !m_config.needSubscribeSupport )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
            "):PublishViaMsgPtrs: Attempt to source messages from storage on a topic that is not configured for subscriptions" );
        return false;
    }

    MT::CScopeMutex lock( m_lock );

    m_stage1StorageToPubSubRequests.push_back( request );

    if ( request.isPersistentRequest )
    {
        bool hadPersistentRequests = !m_stage0StorageToPubSubRequests.empty();

        // persistent requests get 'refills' from new files being dropped in monitored
        // request root paths
        m_stage0StorageToPubSubRequests.push_back( request );

        if ( !hadPersistentRequests )
        {
            SubscribeToVfsTopicPathEvents();
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_config.needSubscribeSupport && m_config.autoPushAfterStartupIfStorageToPubSub )
    {
        if ( !AddStorageToPubSubRequest( StorageToPubSubRequest( m_config.oldestStoragePubSubMsgFileToLoad, m_config.youngestStoragePubSubMsgFileToLoad, true, m_config.autoPushAfterStartupIsPersistent ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:Subscribe: Failed to 'subscribe to topic' for Topic Name \"" + m_config.topicName + "\" because we could not auto add a publish request as configured");
            return false;
        }
    }

    bool success = BeginVfsOps();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:Subscribe: 'subscribe to topic' for Topic Name: " + m_config.topicName );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SubscribeStartingAtMsgId( const CORE::CVariant& msgIdBookmark )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
        return false;

    if ( m_client->GetConfig().desiredFeatures.supportsMsgIdBasedBookmark )
    {
        // Scan storage from earliest container onward for the given message id
        // this is a very expensive operation

        // @TODO Currently not supported.
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SubscribeStartingAtMsgDateTime( const CORE::CDateTime& msgDtBookmark )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( m_config.needSubscribeSupport )
    {
        if ( !AddStorageToPubSubRequest( StorageToPubSubRequest( msgDtBookmark, m_config.youngestStoragePubSubMsgFileToLoad, true, true ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtMsgDateTime: Failed to add publish request. DT=" + msgDtBookmark.ToIso8601DateTimeString( true, true ) + ", Topic Name: " + m_config.topicName );
            return false;
        }

        bool success = BeginVfsOps();
        if ( success )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtMsgDateTime: DT=" + msgDtBookmark.ToIso8601DateTimeString( true, true ) + ", Topic Name: " + m_config.topicName );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtMsgDateTime: Failed to start VFS Ops. DT=" + msgDtBookmark.ToIso8601DateTimeString( true, true ) + ", Topic Name: " + m_config.topicName );
        }
        return success;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtMsgDateTime: Attemping to subscribe to a topic that is not configured to support subscriptions, Topic Name: " + m_config.topicName );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SubscribeStartingAtKeyValue( const CORE::CString& keyNameWithPrefix ,
                                                         CORE::UInt64         startKeyValue     )
{GUCEF_TRACE;

    /* Parse keyNameWithPrefix: "mk:<keyName>" or "k:<keyName>" */
    CStoragePubSubIndexDef::EKeySource keySource = CStoragePubSubIndexDef::KEY_SOURCE_META_DATA;
    CORE::CString keyName;

    if ( keyNameWithPrefix.HasSubstr( "mk:", true ) )
    {
        keySource = CStoragePubSubIndexDef::KEY_SOURCE_META_DATA;
        keyName   = keyNameWithPrefix.CutChars( 3, true, 0 );
    }
    else if ( keyNameWithPrefix.HasSubstr( "k:", true ) )
    {
        keySource = CStoragePubSubIndexDef::KEY_SOURCE_KV_PAIR;
        keyName   = keyNameWithPrefix.CutChars( 2, true, 0 );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtKeyValue: Invalid keyNameWithPrefix (expected 'mk:' or 'k:' prefix): " + keyNameWithPrefix );
        return false;
    }

    /* Find a matching index definition */
    const CStoragePubSubIndexDef* matchedDef = GUCEF_NULL;
    CORE::UInt32 defCount = static_cast< CORE::UInt32 >( m_config.indexDefinitions.size() );
    for ( CORE::UInt32 i=0; i<defCount; ++i )
    {
        const CStoragePubSubIndexDef& def = m_config.indexDefinitions[ i ];
        if ( def.keySource == keySource && def.keyName == keyName )
        {
            matchedDef = &def;
            break;
        }
    }

    if ( GUCEF_NULL == matchedDef )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtKeyValue: No index definition found for key: " + keyNameWithPrefix + ", Topic: " + m_config.topicName );
        return false;
    }

    CStoragePubSubIndexReader reader( *matchedDef, m_vfsRootPath );
    if ( !reader.LoadIndex() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtKeyValue: Failed to load index for key: " + keyNameWithPrefix + ", Topic: " + m_config.topicName );
        return false;
    }

    CStorageBookmarkInfo bookmark;
    if ( !reader.FindStartBookmark( startKeyValue, bookmark ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtKeyValue: No entry found for startKeyValue=" +
            CORE::ToString( startKeyValue ) + " in index for key: " + keyNameWithPrefix + ", Topic: " + m_config.topicName );
        return false;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtKeyValue: Resolved key=" +
        keyNameWithPrefix + " startValue=" + CORE::ToString( startKeyValue ) + " to bookmark file=" +
        bookmark.vfsFilePath + " msgIndex=" + CORE::ToString( bookmark.msgIndex ) + ", Topic: " + m_config.topicName );

    return SubscribeStartingAtBookmarkInfo( bookmark );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SubscribeStartingAtBookmarkInfo( const CStorageBookmarkInfo& bookmarkInfo )
{GUCEF_TRACE;

    // first just get the timestamps from the file
    CORE::CDateTime firstMsgDt;
    CORE::CDateTime lastMsgDt;
    CORE::CDateTime containerDt;
    if ( GetTimestampsFromContainerFilename( bookmarkInfo.vfsFilePath, firstMsgDt, lastMsgDt, containerDt ) )
    {
        // if we are done with the file no need to get fancy
        if ( 0 != bookmarkInfo.doneWithFile )
        {
            return SubscribeStartingAtMsgDateTime( lastMsgDt );
        }
        else
        {
            // Define a partial container request
            TCContainerRangeInfoReference bookmarkInfoRef( CContainerRangeInfo::CreateSharedObj() );
            bookmarkInfoRef = CContainerRangeInfo( bookmarkInfo );

            StorageToPubSubRequest requestThatMatchesBookmark;
            requestThatMatchesBookmark.okIfZeroContainersAreFound = false;
            requestThatMatchesBookmark.isPersistentRequest = true;
            requestThatMatchesBookmark.startDt = firstMsgDt;
            requestThatMatchesBookmark.vfsPubSubMsgContainersToPush.insert( bookmarkInfoRef );

            MT::CScopeMutex lock( m_lock );
            requestThatMatchesBookmark.endDt = m_config.youngestStoragePubSubMsgFileToLoad;

            if ( m_config.needSubscribeSupport )
            {
                if ( !AddStorageToPubSubRequest( requestThatMatchesBookmark ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtBookmarkInfo: Failed to add publish request. DT=" + firstMsgDt.ToIso8601DateTimeString( true, true ) + ", Topic Name: " + m_config.topicName );
                    return false;
                }

                bool success = BeginVfsOps();
                if ( success )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtBookmarkInfo: DT=" + firstMsgDt.ToIso8601DateTimeString( true, true ) + ", Topic Name: " + m_config.topicName );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtBookmarkInfo: Failed to start VFS Ops. DT=" + firstMsgDt.ToIso8601DateTimeString( true, true ) + ", Topic Name: " + m_config.topicName );
                }
                return success;
            }

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtBookmarkInfo: Attemping to subscribe to a topic that is not configured to support subscriptions, Topic Name: " + m_config.topicName );
            return false;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    switch ( bookmark.GetBookmarkType() )
    {
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_DATETIME:
        {
            CORE::CDateTime msgDtBookmark = bookmark.GetBookmarkData().AsDateTime();
            if ( CORE::CDateTime::Empty != msgDtBookmark )
                return SubscribeStartingAtMsgDateTime( msgDtBookmark );
            return false;
        }
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID:
        {
            return SubscribeStartingAtMsgId( bookmark.GetBookmarkData() );
        }
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX:
        {
            CStorageBookmarkInfo bookmarkInfo;
            if ( SyncBookmarkToBookmarkInfo( bookmark, bookmarkInfo ) )
                return SubscribeStartingAtBookmarkInfo( bookmarkInfo );
            return false;
        }

        default:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:SubscribeStartingAtBookmark: Unsupported bookmark type given. Type=" + bookmark.GetBookmarkTypeName() );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SyncBookmarkInfoToBookmark( const CStorageBookmarkInfo& info  ,
                                                       PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer( 1+1+4+4+128, true );
    CORE::CDynamicBufferAccess bufferAccess( buffer );

    bool totalSuccess = true;
    totalSuccess = bufferAccess.WriteValue( info.bookmarkFormatVersion ) && totalSuccess;
    totalSuccess = bufferAccess.WriteValue( info.doneWithFile ) && totalSuccess;
    totalSuccess = bufferAccess.WriteValue( info.msgIndex ) && totalSuccess;
    totalSuccess = bufferAccess.WriteValue( info.offsetInFile ) && totalSuccess;
    totalSuccess = bufferAccess.WriteByteSizePrefixedString( info.vfsFilePath ) && totalSuccess;

    totalSuccess = bookmark.GetBookmarkData().TransferOwnershipFrom( buffer ) && totalSuccess;

    if ( totalSuccess )
        bookmark.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX );
    else
        bookmark.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_INITIALIZED );

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SyncBookmarkToBookmarkInfo( const PUBSUB::CPubSubBookmark& bookmark ,
                                                       CStorageBookmarkInfo& info              )
{GUCEF_TRACE;

    const CORE::CVariant& data = bookmark.GetBookmarkData();
    if ( data.IsBlob() )
    {
        CORE::CDynamicBuffer buffer;
        buffer.LinkTo( data );
        CORE::CDynamicBufferAccess bufferAccess( buffer );

        bool totalSuccess = true;
        totalSuccess = bufferAccess.ReadValue( info.bookmarkFormatVersion ) && totalSuccess;
        totalSuccess = bufferAccess.ReadValue( info.doneWithFile ) && totalSuccess;
        totalSuccess = bufferAccess.ReadValue( info.msgIndex ) && totalSuccess;
        totalSuccess = bufferAccess.ReadValue( info.offsetInFile ) && totalSuccess;
        totalSuccess = bufferAccess.ReadByteSizePrefixedString( info.vfsFilePath ) && totalSuccess;
        return totalSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubBookmark
CStoragePubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;

    // provides a bookmark for the latest read batch of data

    MT::CScopeMutex lock( m_lock );

    if ( SyncBookmarkInfoToBookmark( m_currentBookmarkInfo, m_currentBookmark ) )
        return m_currentBookmark;

    return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_AVAILABLE );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::GetStorageBufferMetaDataPtrForReceiveActionId( CORE::UInt64 receiveActionId     ,
                                                                          StorageBufferMetaData** metaData )
{GUCEF_TRACE;

    TStorageBufferMetaDataMap::iterator i = m_storageBufferMetaData.begin();
    while ( i != m_storageBufferMetaData.end() )
    {
        StorageBufferMetaData& metaDataEntry = (*i).second;

        // Logically since we control 'receiveActionId' right here in the backend
        // we know that we always just increment in a batch for all messages in a container
        // as such we can just check the first and last as a range to find the right container

        if ( !metaDataEntry.actionIds.empty() )
        {
            CORE::UInt64 containerReceiveActionIdMin = *metaDataEntry.actionIds.begin();
            CORE::UInt64 containerReceiveActionIdMax = *metaDataEntry.actionIds.rbegin();

            if ( receiveActionId >= containerReceiveActionIdMin &&
                 receiveActionId <= containerReceiveActionIdMax  )
            {
                // We found the container this 'receiveActionId' is linked with

                *metaData = &metaDataEntry;
                return true;
            }
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::GetStorageBufferMetaDataPtrForVfsContainerPath( const CORE::CString& vfsContainerPath ,
                                                                           bool onlyCurrentlyLoaded              ,
                                                                           StorageBufferMetaData** metaData      )
{GUCEF_TRACE;

    if ( GUCEF_NULL == metaData )
        return false;

    *metaData = GUCEF_NULL;

    TStorageBufferMetaDataMap::iterator i = m_storageBufferMetaData.begin();
    while ( i != m_storageBufferMetaData.end() )
    {
        StorageBufferMetaData& metaDataEntry = (*i).second;

        if ( onlyCurrentlyLoaded )
        {
            if ( !metaDataEntry.linkedRequestEntry.IsNULL() )
            {
                if ( metaDataEntry.linkedRequestEntry->vfsFilePath == vfsContainerPath )
                {
                    *metaData = &metaDataEntry;
                    return true;
                }
            }
        }
        else
        {
            if ( GUCEF_NULL != metaDataEntry.linkedRequest )
            {
                StorageToPubSubRequest* request = metaDataEntry.linkedRequest;

                TCContainerRangeInfoReferenceSet::iterator n = request->vfsPubSubMsgContainersTransmitted.begin();
                while ( n != request->vfsPubSubMsgContainersTransmitted.end() )
                {
                    if ( (*n)->vfsFilePath == vfsContainerPath )
                    {
                        *metaData = &metaDataEntry;
                        return true;
                    }
                    ++n;
                }

                n = request->vfsPubSubMsgContainersPushed.begin();
                while ( n != request->vfsPubSubMsgContainersPushed.end() )
                {
                    if ( (*n)->vfsFilePath == vfsContainerPath )
                    {
                        *metaData = &metaDataEntry;
                        return true;
                    }
                    ++n;
                }

                n = request->vfsPubSubMsgContainersToPush.begin();
                while ( n != request->vfsPubSubMsgContainersToPush.end() )
                {
                    if ( (*n)->vfsFilePath == vfsContainerPath )
                    {
                        *metaData = &metaDataEntry;
                        return true;
                    }
                    ++n;
                }
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::GetBookmarkForReceiveActionId( CORE::UInt64 receiveActionId      ,
                                                          PUBSUB::CPubSubBookmark& bookmark ) const
{GUCEF_TRACE;

    CStorageBookmarkInfo bmInfo;
    StorageBufferMetaData* metaData = GUCEF_NULL;

    // since we wont use 'metaData' we can const_cast as its logically const
    if ( const_cast< CStoragePubSubClientTopic* >( this )->GetBookmarkInfoForReceiveActionId( receiveActionId, &metaData, bmInfo ) )
    {
        return SyncBookmarkInfoToBookmark( bmInfo, bookmark );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::GetBookmarkInfoForReceiveActionId( CORE::UInt64 receiveActionId       ,
                                                              StorageBufferMetaData** metaData   ,
                                                              CStorageBookmarkInfo& bookmarkInfo )
{GUCEF_TRACE;

    StorageBufferMetaData* mData = GUCEF_NULL;
    if ( GetStorageBufferMetaDataPtrForReceiveActionId( receiveActionId, &mData ) )
    {
        if ( GUCEF_NULL != metaData )
            *metaData = mData;

        CORE::UInt64 containerReceiveActionIdMin = *mData->actionIds.cbegin();
        CORE::UInt64 containerReceiveActionIdMax = *mData->actionIds.crbegin();

        // Now we need to find the actual entry
        // for this we again use our knowledge of the 'receiveActionId' to try and forgo
        // having to seach and compare all IDs. Since they should be ordered
        // we should be able to use the min as an offset

        bool msgOffsetFound = false;

        // most of the time an exact offset will be an exact match
        // we should only have gaps in case of errors
        CORE::UInt64 actionIdSearchOffset = receiveActionId - containerReceiveActionIdMin;
        if ( actionIdSearchOffset+1 <= mData->actionIds.size() &&
             actionIdSearchOffset+1 <= mData->msgOffsetIndex.size() &&
             receiveActionId == mData->actionIds[ actionIdSearchOffset ] )
        {
            bookmarkInfo.msgIndex = (CORE::UInt32) actionIdSearchOffset;
            bookmarkInfo.offsetInFile = mData->msgOffsetIndex[ actionIdSearchOffset ];
            msgOffsetFound = true;
        }
        else
        {
            // we will have to search for the matching action id a bit
            // its still likely to be around the offset we calculated so lets start there
            // and then fan out

            CORE::UInt64 maxSearch = (CORE::UInt64) SMALLEST( mData->actionIds.size(), mData->msgOffsetIndex.size() );
            if ( actionIdSearchOffset >= maxSearch )
            {
                // Something is wrong, we are out of range
                // Switch to searching from the middle of the available range
                actionIdSearchOffset = maxSearch / 2;

                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:GetBookmarkInfoForReceiveActionId: min (" +
                        CORE::ToString( containerReceiveActionIdMin ) + ") and max (" +
                        CORE::ToString( containerReceiveActionIdMax ) + ") receiveActionId envelop given Id " +
                        CORE::ToString( receiveActionId ) + " but exceeds tracking storage size of " +
                        CORE::ToString( maxSearch ) );
            }

            for ( CORE::UInt64 n=1; n<maxSearch; ++n )
            {
                CORE::Int64 downIndex = ( (CORE::Int64) actionIdSearchOffset ) - n;
                CORE::UInt64 upIndex = actionIdSearchOffset + n;

                if ( downIndex >= 0 )
                {
                    if ( receiveActionId == mData->actionIds[ downIndex ] )
                    {
                        bookmarkInfo.msgIndex = (CORE::UInt32) downIndex;
                        bookmarkInfo.offsetInFile = mData->msgOffsetIndex[ downIndex ];
                        msgOffsetFound = true;
                    }
                }
                if ( upIndex < maxSearch )
                {
                    if ( receiveActionId == mData->actionIds[ upIndex ] )
                    {
                        bookmarkInfo.msgIndex = (CORE::UInt32) upIndex;
                        bookmarkInfo.offsetInFile = mData->msgOffsetIndex[ upIndex ];
                        msgOffsetFound = true;
                    }
                }

                if ( downIndex < 0 && upIndex >= maxSearch )
                    break;
            }
        }

        if ( msgOffsetFound )
        {
            bookmarkInfo.bookmarkFormatVersion = 1;
            bookmarkInfo.doneWithFile = containerReceiveActionIdMax == receiveActionId;
            bookmarkInfo.vfsFilePath = mData->linkedRequestEntry->vfsFilePath;
            return true;
        }

        // No other container is going to have this
        return false;
    }
    else
    {
        if ( GUCEF_NULL != metaData )
            *metaData = GUCEF_NULL;

        // the active buffers no longer reference this data
        // check the completed containers
        TCContainerRangeInfoReferenceSet::reverse_iterator c = m_completedContainers.rbegin();
        while ( c != m_completedContainers.rend() )
        {
            if ( (*c)->minActionId <= receiveActionId &&
                 (*c)->maxActionId >= receiveActionId )
            {
                // Found it
                // We have already completed this container, report as such
                bookmarkInfo = *(*c);
                return true;
            }
            ++c;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::DeriveBookmarkFromMsg( const PUBSUB::CIPubSubMsg& msg    ,
                                                  PUBSUB::CPubSubBookmark& bookmark ) const
{GUCEF_TRACE;

    // since the storage backend is more of a transcribing passthrough it doesnt know what on the message means what.
    // As such it cannot blindly support this using message attributes. The only exception to that is the receiveActionId
    // which is under this backend's runtime control

    if ( m_config.needSubscribeSupport && m_needToTrackAcks )
    {
        CORE::UInt64 receiveActionId = msg.GetReceiveActionId();
        if ( receiveActionId > 0 )
        {
            return GetBookmarkForReceiveActionId( receiveActionId, bookmark );
        }
    }

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:DeriveBookmarkFromMsg: this feature is only supported for messages sourced from this backend in 'subscriber' mode at runtime and needs a valid 'receiveActionId'" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg )
{GUCEF_TRACE;

    // since the storage backend is more of a transcribing passthrough it doesnt know what on the message means what.
    // As such it cannot blindly support this using message attributes. The only exception to that is the receiveActionId
    // which is under this backend's runtime control

    if ( m_config.needSubscribeSupport && m_needToTrackAcks )
    {
        CORE::UInt64 receiveActionId = msg.GetReceiveActionId();
        if ( receiveActionId > 0 )
        {
            MT::CScopeMutex lock( m_lock );

            CStorageBookmarkInfo bmInfo;
            StorageBufferMetaData* metaData = GUCEF_NULL;
            if ( GetBookmarkInfoForReceiveActionId( receiveActionId, &metaData, bmInfo ) )
            {
                return AcknowledgeReceiptImpl( bmInfo, metaData );
            }
            return false;
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: this feature is only supported for messages sourced from this backend at runtime as it needs the 'receiveActionId'" );
            return false;
        }
    }

    // since the storage backend is more of a transcribing passthrough it doesnt know what on the message means what. As such it cannot support this
    // treat as advisory fyi
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    if ( m_config.needSubscribeSupport && m_needToTrackAcks )
    {
        CStorageBookmarkInfo bm;
        if ( SyncBookmarkToBookmarkInfo( bookmark, bm ) )
        {
            MT::CScopeMutex lock( m_lock );

            StorageBufferMetaData* metaData = GUCEF_NULL;
            GetStorageBufferMetaDataPtrForVfsContainerPath( bm.vfsFilePath, true, &metaData );

            return AcknowledgeReceiptImpl( bm, metaData );
        }

        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: Unable to interpret generic bookmark" );
        return false;
    }

    // treat as advisory fyi
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::WasContainerMoved( const CORE::CString& vfsFilePath ) const
{GUCEF_TRACE;

    CORE::CString containerVfsFilename = vfsFilePath.SubstrToChar( '/', false, true );
    containerVfsFilename = m_vfsStorageRootPathForFullyAckdContainers + '/' + containerVfsFilename;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    return vfs.FileExists( containerVfsFilename );
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::AcknowledgeReceiptImpl( const CStorageBookmarkInfo& bookmark ,
                                                   StorageBufferMetaData* metaData      )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return false;

    if ( GUCEF_NULL != metaData )
    {
        CStoragePubSubClientConfig& clientConfig = m_client->GetConfig();
        PUBSUB::CPubSubClientFeatures& desiredFeatures = clientConfig.desiredFeatures;

        if ( metaData->msgAcks[ bookmark.msgIndex ] )
        {
            // this message has already been ack'd before so we already took whatever action we were going to take
            // treat as fyi no-op
            return true;
        }

        // This message had not been previously ack'd yet
        ++metaData->ackdMsgCount;
        metaData->msgAcks[ bookmark.msgIndex ] = true;
        if ( metaData->msgsInFlight > 0 )
            --metaData->msgsInFlight;

        if ( desiredFeatures.supportsAckUsingLastMsgInBatch )
        {
            // treat as a batch ack for this message and everything before it from the same file
            // we know we publish messages in the order they are stored in the file

            for ( CORE::UInt32 i=metaData->lastAckdMsgIndex; i<bookmark.msgIndex; ++i )
            {
                if ( !metaData->msgAcks[ i ] )
                {
                    metaData->msgAcks[ i ] = true;
                    ++metaData->ackdMsgCount;
                    if ( metaData->msgsInFlight > 0 )
                        --metaData->msgsInFlight;
                }
            }
            metaData->lastAckdMsgIndex = bookmark.msgIndex;
        }
        else
        {
            // We are not allowed to consider anything ack'd unless done so explicitly
            // However we can still recieve acks out of order so we need to check if potentially
            // gaps are getting filled allowing us to advance the 'lastAckdMsgIndex' by more than
            // just 1 message

            for ( CORE::UInt32 i=metaData->lastAckdMsgIndex; i<bookmark.msgIndex; ++i )
            {
                if ( metaData->msgAcks[ i ] )
                {
                    metaData->lastAckdMsgIndex = i;
                }
                else
                {
                    break;
                }
            }
        }

        metaData->lastAckdMsgTime = CORE::CDateTime::NowUTCDateTime();

        if ( metaData->ackdMsgCount >= metaData->msgAcks.size() )
        {
            // All messages from this container have been acknowledged
            if ( m_config.moveContainersWithFullyAckdContent )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: Moving container since its content is fully ack'd: " + metaData->linkedRequestEntry->vfsFilePath );

                CORE::CString containerFilename = metaData->linkedRequestEntry->vfsFilePath.SubstrToChar( '/', false, true );
                containerFilename = m_vfsStorageRootPathForFullyAckdContainers + '/' + containerFilename;

                VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
                if ( !vfs.MoveFile( metaData->linkedRequestEntry->vfsFilePath, containerFilename, true ) )
                {
                    // This might cause problems later like additional retransmission of data
                    // plus the age old problem perhaps of running out of disk space eventually (assuming non-auto-scaling)
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: Failed to move container file from \"" +
                            metaData->linkedRequestEntry->vfsFilePath + "\" to \"" + containerFilename + "\"" );
                }
            }
            else // <- move overrides delete as the safer choice
            if ( m_config.deleteContainersWithFullyAckdContent )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: Deleting container since its content is fully ack'd: " + metaData->linkedRequestEntry->vfsFilePath );

                VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
                if ( !vfs.DeleteFile( metaData->linkedRequestEntry->vfsFilePath, true ) )
                {
                    // This might cause problems later like additional retransmission of data
                    // plus the age old problem perhaps of running out of disk space eventually (assuming non-auto-scaling)
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: Failed to delete container file" );
                }
            }

            ProgressRequest( metaData, true, true );
        }
    }
    else
    {
        // We no longer have associated buffer meta data
        // This may be a bookmark ack that is a duplicate or arrived late

        if ( 0 != bookmark.doneWithFile && 0 != bookmark.offsetInFile )
        {
            // All messages from this container have been acknowledged
            if ( m_config.moveContainersWithFullyAckdContent )
            {
                if ( 0 == bookmark.vfsFilePath.HasSubstr( m_vfsRootPath, true ) )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: Moving container since its content is fully ack'd: " + bookmark.vfsFilePath );

                    CORE::CString containerFilename = bookmark.vfsFilePath.SubstrToChar( '/', false, true );
                    containerFilename = m_vfsStorageRootPathForFullyAckdContainers + '/' + containerFilename;

                    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
                    if ( !vfs.MoveFile( bookmark.vfsFilePath, containerFilename, true ) )
                    {
                        // This might cause problems later like additional retransmission of data
                        // plus the age old problem perhaps of running out of disk space eventually (assuming non-auto-scaling)
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: Failed to move container file from \"" +
                                bookmark.vfsFilePath + "\" to \"" + containerFilename + "\"" );
                    }
                }
                else // <- move overrides delete as the safer choice
                if ( m_config.deleteContainersWithFullyAckdContent )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: Deleting container since its content is fully ack'd: " + bookmark.vfsFilePath );

                    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
                    if ( !vfs.DeleteFile( bookmark.vfsFilePath, true ) )
                    {
                        // This might cause problems later like additional retransmission of data
                        // plus the age old problem perhaps of running out of disk space eventually (assuming non-auto-scaling)
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:AcknowledgeReceipt: Failed to delete container file" );
                    }
                }

                //ProgressRequest( metaData, true, true );
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnNoAckRetransmitCheckCycle( CORE::CNotifier* notifier    ,
                                                        const CORE::CEvent& eventId  ,
                                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    TStorageBufferMetaDataMap::iterator i = m_storageBufferMetaData.begin();
    while ( i != m_storageBufferMetaData.end() )
    {
        StorageBufferMetaData& metaDataEntry = (*i).second;

        // Check to see if acks apply to this buffer still
        if ( !metaDataEntry.isBeingWritten && !metaDataEntry.isReleased )
        {
            // Check if this buffer has messages which have not been ack'd yet
            if ( metaDataEntry.ackdMsgCount < metaDataEntry.actionIds.size() )
            {
                // Check to see if the max permissable amount of time to ack all the messages in the batch has expired
                if ( metaDataEntry.lastAckdMsgTime.GetTimeDifferenceInMillisecondsToNow() > m_config.maxTimeToWaitForAllMsgBatchAcksInMs )
                {
                    // Lets try to deliniate which messages to resend
                    // if we find an non-ack'd message then everything after that point will be sent again
                    // this is to ensure ordering in the retransmission similar to how Kafka and the like resend
                    // since the last successfully persisted bookmark

                    if ( metaDataEntry.lastAckdMsgIndex < metaDataEntry.lastMsgTransmittedIndex )
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:OnNoAckRetransmitCheckCycle: Not all (" + CORE::ToString( metaDataEntry.actionIds.size() ) +
                                ") messages have been ack'd (" + CORE::ToString( metaDataEntry.ackdMsgCount ) + ") in time for messages sourced from container " + metaDataEntry.linkedRequestEntry->vfsFilePath +
                                " . Resetting last transmitted msg index to " + CORE::ToString( metaDataEntry.lastAckdMsgIndex ) + " from " + CORE::ToString( metaDataEntry.lastMsgTransmittedIndex )  );

                        metaDataEntry.lastMsgTransmittedIndex = metaDataEntry.lastAckdMsgIndex;
                    }
                    else
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:OnNoAckRetransmitCheckCycle: Not all (" + CORE::ToString( metaDataEntry.actionIds.size() ) +
                                ") messages have been ack'd (" + CORE::ToString( metaDataEntry.ackdMsgCount ) + ") in time for messages sourced from container " + metaDataEntry.linkedRequestEntry->vfsFilePath +
                                " . Last transmitted msg index " + CORE::ToString( metaDataEntry.lastAckdMsgIndex ) + " is less than last ack index " + CORE::ToString( metaDataEntry.lastMsgTransmittedIndex )  );
                    }

                    // Reset the clock before transmitting to provide a new time window for the newly retransmitted messages to be ack'd
                    metaDataEntry.lastAckdMsgTime = CORE::CDateTime::NowUTCDateTime();

                    // Begin actuall retransmission
                    TransmitNextPubSubMsgBatch( (*i).first, &metaDataEntry );
                }
            }
        }

        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    return vfs.IsHealthy() && vfs.IsConnected();
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    if ( vfs.IsHealthy() && vfs.IsConnected() )
    {
        if ( !m_config.performVfsOpsASync )
        {
            if ( GUCEF_NULL != m_syncVfsOpsTimer && m_syncVfsOpsTimer->GetEnabled() )
                return true;
        }
        else
        {
            if ( m_config.performVfsOpsInDedicatedThread )
            {
                if ( !m_vfsOpsThread.IsNULL() && m_vfsOpsThread->IsActive() )
                    return true;
            }
            else
            {
                // we rely on defining 'work' tasks as work arrives
                return m_config.needSubscribeSupport;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    bool newIsHealthyState = VFS::CVfsGlobal::Instance()->GetVfs().IsHealthy();

    if ( m_config.maxStorageCorruptionDetectionsToBeHealthy >= 0 )
    {
        // Current and last metrics cycle error count counts against the max
        if ( m_storageCorruptionDetections + m_metrics.storageCorruptionDetections > (CORE::UInt32) m_config.maxStorageCorruptionDetectionsToBeHealthy )
            newIsHealthyState = false;
    }

    if ( newIsHealthyState && m_config.maxStorageDeserializationFailuresToBeHealthy >= 0 )
    {
        // Current and last metrics cycle error count counts against the max
        if ( m_storageDeserializationFailures + m_metrics.storageDeserializationFailures > (CORE::UInt32) m_config.maxStorageDeserializationFailuresToBeHealthy )
            newIsHealthyState = false;
    }

    if ( newIsHealthyState && GUCEF_NULL != m_reconnectTimer && m_reconnectTimer->GetEnabled() )
    {
        // running a reconnect timer cannot be good
        newIsHealthyState = false;
    }

    if ( newIsHealthyState != m_isHealthy )
    {
        m_isHealthy = newIsHealthyState;

        if ( m_isHealthy )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:IsHealthy: Topic " + m_config.topicName + " is now healthy" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:IsHealthy: Topic " + m_config.topicName + " is now unhealthy" );
        }

        lock.EarlyUnlock();
        THealthStatusChangeEventData eData( newIsHealthyState );
        NotifyObservers( HealthStatusChangeEvent, &eData );
    }

    return newIsHealthyState;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::UpdateIsHealthyStatus( bool newStatus )
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
CStoragePubSubClientTopic::InitializeConnectivity( bool reset )
{GUCEF_TRACE;

    m_buffers.SetNrOfBuffers( m_config.desiredNrOfBuffers );
    m_buffers.SetMinimalBufferSize( m_config.desiredMinimalSerializedBlockSize );

    if ( m_config.performVfsOpsInDedicatedThread )
    {

    }

    return true;
}

/*-------------------------------------------------------------------------*/

const CStoragePubSubClientTopicConfig&
CStoragePubSubClientTopic::GetTopicConfig( void ) const
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnReconnectTimerCycle( CORE::CNotifier* notifier    ,
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
CStoragePubSubClientTopic::OnEndOfASyncVfsWork( void )
{GUCEF_TRACE;

    if ( m_config.needPublishSupport )
    {
        // Finalize the write buffer content, there wont be any additional content at this time
        FinalizeWriteBuffer( false );

        // Persist next ready to read buffer
        StoreNextReceivedPubSubBuffer( true );
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::PerformASyncVfsWork( void )
{GUCEF_TRACE;

    if ( m_config.needPublishSupport )
    {
        StoreNextReceivedPubSubBuffer( true );
    }
    else
    {
        LocateFilesForStorageToPubSubRequest();
        ProcessNextPubSubRequestRelatedFile();
        TransmitNextPubSubMsgBuffer();
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnSyncVfsOpsTimerCycle( CORE::CNotifier* notifier    ,
                                                   const CORE::CEvent& eventId  ,
                                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_vfsInitIsComplete )
    {
        if ( m_config.needPublishSupport )
        {
            StoreNextReceivedPubSubBuffer( true );
        }
        else
        {
            LocateFilesForStorageToPubSubRequest();
            ProcessNextPubSubRequestRelatedFile();
            TransmitNextPubSubMsgBuffer();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::NotifyPublishResults( void )
{GUCEF_TRACE;

    if ( !m_publishSuccessActionIds.empty() )
    {
        UInt64 firstId = *m_publishSuccessActionEventData.GetData().begin();
        UInt64 lastId = *m_publishSuccessActionEventData.GetData().rbegin();
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "StoragePubSubClientTopic:NotifyPublishResults: Notifying MsgsPublishedEvent for " + 
            CORE::ToString( m_publishSuccessActionIds.size() ) + " messages. First Id=" + CORE::ToString( firstId ) + " Last Id=" + CORE::ToString( lastId ) );

        if ( !NotifyObservers( MsgsPublishedEvent, &m_publishSuccessActionEventData ) ) return;
        m_publishSuccessActionIds.clear();
    }
    if ( !m_publishFailureActionIds.empty() )
    {
        UInt64 firstId = *m_publishSuccessActionEventData.GetData().begin();
        UInt64 lastId = *m_publishSuccessActionEventData.GetData().rbegin();
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "StoragePubSubClientTopic:NotifyPublishResults: Notifying MsgsPublishFailureEvent for " + 
            CORE::ToString( m_publishFailureActionIds.size() ) + " messages. First Id=" + CORE::ToString( firstId ) + " Last Id=" + CORE::ToString( lastId ) );

        if ( !NotifyObservers( MsgsPublishFailureEvent, &m_publishFailureActionEventData ) ) return;
        m_publishFailureActionIds.clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnPulseCycle( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyPublishResults();
}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopic::TopicMetrics::TopicMetrics( void )
    : queuedReadyToReadBuffers( 0 )
    , smallestBufferSizeInBytes( 0 )
    , largestBufferSizeInBytes( 0 )
    , msgsLoadedFromStorage( 0 )
    , msgBytesLoadedFromStorage( 0 )
    , msgsNotifiedAsReceived( 0 )
    , msgsWrittenToStorage( 0 )
    , msgBytesWrittenToStorage( 0 )
    , storageCorruptionDetections( 0 )
    , storageDeserializationFailures( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CStoragePubSubClientTopic::TopicMetrics&
CStoragePubSubClientTopic::GetMetrics( void ) const
{GUCEF_TRACE;

    return m_metrics;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStoragePubSubClientTopic::GetMetricFriendlyTopicName( void ) const
{GUCEF_TRACE;

    return m_metricFriendlyTopicName;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnBufferContentTimeWindowCheckCycle( CORE::CNotifier* notifier    ,
                                                                const CORE::CEvent& eventId  ,
                                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_currentWriteBuffer && m_config.needPublishSupport )
    {
        if ( m_lastWriteBlockCompletion.GetTimeDifferenceInMillisecondsToNow() >= m_config.desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:OnBufferContentTimeWindowCheckCycle: Finalizing write buffer if it has data due to waiting for data longer then " + CORE::ToString( m_config.desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs ) );
            FinalizeWriteBuffer( true );
            NotifyPublishResults();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return;

    const PUBSUB::CPubSubClientConfig& clientConfig = m_client->GetConfig();

    CORE::UInt32 smallest = 0;
    CORE::UInt32 largest = 0;
    m_buffers.GetBufferSizeRange( smallest, largest );

    m_metrics.queuedReadyToReadBuffers = m_buffers.GetBuffersQueuedToRead();
    m_metrics.smallestBufferSizeInBytes = smallest;
    m_metrics.largestBufferSizeInBytes = largest;

    if ( clientConfig.desiredFeatures.supportsPublishing )
    {
        m_metrics.msgsWrittenToStorage = GetMsgsWrittenToStorageCounter( true );
        m_metrics.msgBytesWrittenToStorage = GetMsgBytesWrittenToStorageCounter( true );
    }
    if ( clientConfig.desiredFeatures.supportsSubscribing )
    {
        m_metrics.msgsLoadedFromStorage = GetMsgsLoadedFromStorageCounter( true );
        m_metrics.msgBytesLoadedFromStorage = GetMsgBytesLoadedFromStorageCounter( true );
        m_metrics.msgsNotifiedAsReceived = GetMsgsNotifiedAsReceivedCounter( true );
        m_metrics.storageDeserializationFailures = GetStorageDeserializationFailuresCounter( true );
        m_metrics.storageCorruptionDetections = GetStorageCorruptionDetectionCounter( true );
    }
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CStoragePubSubClientTopic::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

 MT::TLockStatus
 CStoragePubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CStoragePubSubClientTopic::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

CORE::CDynamicBufferSwap&
CStoragePubSubClientTopic::GetSerializedMsgBuffers( void )
{GUCEF_TRACE;

    return m_buffers;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::LoadStorageFile( const CORE::CString& vfsPath       ,
                                            CORE::CDynamicBuffer& targetBuffer ,
                                            bool& failureIsOk                  )
{GUCEF_TRACE;

    failureIsOk = false;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    if ( m_config.moveContainersWithFullyAckdContent )
    {
        if ( WasContainerMoved( vfsPath ) )
        {
            failureIsOk = true;
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:LoadStorageFile: pubsub msg container was already moved due to full ack: " + vfsPath );
            return false;
        }
    }
    else
    if ( m_config.deleteContainersWithFullyAckdContent )
    {
        if ( !vfs.FileExists( vfsPath ) )
        {
            failureIsOk = true;
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:LoadStorageFile: pubsub msg container seems to no longer exist. Delete after full ack is active. file: " + vfsPath );
            return false;
        }
    }

    if ( !m_config.decodeCodecFamily.IsNULLOrEmpty() && !m_config.decodeCodecName.IsNULLOrEmpty() )
    {
        CORE::Float32 encodeRatio = m_encodeSizeRatio < 0 ? m_config.defaultCodecDecodeGrowthRatioExpectation : m_encodeSizeRatio;
        CORE::UInt32 estimatedApproxDecodedSize = (CORE::UInt32) ( vfs.GetFileSize( vfsPath ) * encodeRatio );
        targetBuffer.SetBufferSize( estimatedApproxDecodedSize, false );

        if ( !vfs.DecodeAsFile( targetBuffer, 0, vfsPath, m_config.decodeCodecFamily, m_config.decodeCodecName ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:LoadStorageFile: Cannot decode and load persisted file. CodeFamily:" + m_config.decodeCodecFamily +
                " CodecName: " + m_config.decodeCodecName + ". VFS File: " + vfsPath );
            return false;
        }

        if ( targetBuffer.GetDataSize() > 0 )
            m_encodeSizeRatio = (CORE::Float32) ( targetBuffer.GetDataSize() / vfs.GetFileSize( vfsPath ) );

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:LoadStorageFile: Decoded and loaded persisted file. CodeFamily:" + m_config.decodeCodecFamily +
            " CodecName: " + m_config.decodeCodecName + ". VFS File: \"" + vfsPath + "\". encode ratio: " + CORE::ToString( m_encodeSizeRatio ) );
    }
    else
    {
        // Not using any encoding, load the file as-is
        if ( !vfs.LoadFile( targetBuffer, vfsPath, "rb" ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:LoadStorageFile: Cannot load last persisted file. VFS File: " + vfsPath );
            return false;
        }

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:LoadStorageFile: Loaded VFS File: " + vfsPath );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::GetTimestampsFromContainerFilename( const CORE::CString& fullPath     ,
                                                               CORE::CDateTime& firstMsgDt       ,
                                                               CORE::CDateTime& lastMsgDt        ,
                                                               CORE::CDateTime& containerStartDt ) const
{GUCEF_TRACE;

    // first strip the extra stuff from the full path to get the string form timestamps
    CORE::CString segment = CORE::ExtractFilename( fullPath );
    segment = segment.CutChars( m_vfsFilePostfix.Length(), false, 0 );
    CORE::CString firstMsgDtStr = segment.SubstrToChar( '_', true, false );
    CORE::CString lastMsgDtStr = segment.SubstrToChar( '_', firstMsgDtStr.Length()+1, true, false );
    CORE::CString containerStartDtStr = segment.SubstrToChar( '_', false, false  );

    // Try to parse what is left as a valid ISO 8601 DateTime
    if ( firstMsgDt.FromIso8601DateTimeString( firstMsgDtStr ) &&
         lastMsgDt.FromIso8601DateTimeString( lastMsgDtStr ) &&
         containerStartDt.FromIso8601DateTimeString( containerStartDtStr ) )
        return true;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:GetTimestampsFromContainerFilename: Failed to parse timestamps from filename " + segment );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::GetPathsToPubSubStorageFiles( const CORE::CDateTime& startDt           ,
                                                         const CORE::CDateTime& endDt             ,
                                                         TCContainerRangeInfoReferenceSet& files  ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    CORE::CString fileFilter = '*' + m_vfsFilePostfix;
    VFS::CVFS::TStringVector index;
    if ( vfs.GetFileList( index, m_vfsRootPath, false, true, fileFilter, true ) )
    {
        VFS::CVFS::TStringVector::iterator i = index.begin();
        while ( i != index.end() )
        {
            CORE::CDateTime containerFileFirstMsgDt;
            CORE::CDateTime containerFileLastMsgDt;
            CORE::CDateTime containerFileCaptureDt;
            if ( GetTimestampsFromContainerFilename( (*i), containerFileFirstMsgDt, containerFileLastMsgDt, containerFileCaptureDt ) )
            {
                // Check the container first messgage dt against the our time range
                // It is assumed here that the containers have messages chronologically ordered
                if ( containerFileFirstMsgDt.IsWithinRange( startDt, endDt ) || containerFileLastMsgDt.IsWithinRange( startDt, endDt ) )
                {
                    TCContainerRangeInfoPtr bookmarkInfo = CContainerRangeInfo::CreateSharedObj();
                    bookmarkInfo->vfsFilePath = (*i);
                    files.insert( bookmarkInfo );
                }
            }
            else
            {
                totalSuccess = false;
            }
            ++i;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:GetPathsToPubSubStorageFiles: VFS failure obtaining container list" );
        totalSuccess = false;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::AddPublishActionIdsToNotify( const TPublishActionIdVector& publishActionIds, bool success )
{GUCEF_TRACE;

    size_t idCount = publishActionIds.size();
    if ( success )
    {
        for ( size_t i=0; i<idCount; ++i )
        {
            if ( !m_journal.IsNULL() )
                m_journal->AddMessageSentJournalEntry( publishActionIds[ i ] );

            m_publishSuccessActionIds.push_back( publishActionIds[ i ] );
        }
    }
    else
    {
        for ( size_t i=0; i<idCount; ++i )
        {
            if ( !m_journal.IsNULL() )
                m_journal->AddMessageSendFailureJournalEntry( publishActionIds[ i ] );

            m_publishFailureActionIds.push_back( publishActionIds[ i ] );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::StoreNextReceivedPubSubBuffer( bool onlyStoreIfBufferHasContent )
{GUCEF_TRACE;

    if ( !m_vfsInitIsComplete )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "StoragePubSubClientTopic:StoreNextReceivedPubSubBuffer: Deferring until VFS init has been completed" );
        return false;
    }

    CORE::CDynamicBufferSwap& buffers = GetSerializedMsgBuffers();

    CORE::CDateTime msgBatchDt;
    if ( GUCEF_NULL == m_currentReadBuffer )
        m_currentReadBuffer = buffers.GetNextReaderBuffer( msgBatchDt, false, 25 );
    else
        msgBatchDt = buffers.GetCurrenReaderBufferAssociatedDt();

    if ( GUCEF_NULL != m_currentReadBuffer )
    {
        StorageBufferMetaData* bufferMetaData = &( m_storageBufferMetaData[ m_currentReadBuffer ] );

        // Check to see if we even want to store this buffer
        // headers notwithstanding, it may not make sense to store this buffer if the container file has no content
        if ( onlyStoreIfBufferHasContent && bufferMetaData->msgOffsetIndex.empty() )
        {
            buffers.SignalEndOfReading();
            m_currentReadBuffer = GUCEF_NULL;

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:StoreNextReceivedPubSubBuffer: Skipping storing buffer without meaningful content" );
            return true;
        }
        
        if ( msgBatchDt == CORE::CDateTime::Empty )
            msgBatchDt = CORE::CDateTime::NowUTCDateTime();

        // Get the timestamp of the last message in the buffer.
        bool isCorrupted = false;
        bool isMsgDtSupported = false;
        CORE::CDateTime firstMsgDt;
        CORE::CDateTime lastMsgDt;
        if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeFirstAndLastMsgDateTime( firstMsgDt, lastMsgDt, *m_currentReadBuffer, isMsgDtSupported, isCorrupted ) )
        {
            if ( !isMsgDtSupported )
            {
                firstMsgDt = msgBatchDt;
                lastMsgDt = CORE::CDateTime::NowUTCDateTime();
            }
        }

        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

        if ( m_config.encodeCodecFamily.IsNULLOrEmpty() || m_config.encodeCodecName.IsNULLOrEmpty() )
        {
            // Check if the file does not already exists which is highly unlikely working with buffers and a millisecond batch callout
            CORE::CString vfsFilename;
            CORE::CString vfsStoragePath;
            UInt32 loopIndex = 0;
            do
            {
                msgBatchDt.SetMilliseconds( msgBatchDt.GetMilliseconds() + loopIndex );
                vfsFilename = firstMsgDt.ToIso8601DateTimeString( false, true ) + '_' + lastMsgDt.ToIso8601DateTimeString( false, true )  + '_' + msgBatchDt.ToIso8601DateTimeString( false, true ) + m_vfsFilePostfix;
                vfsStoragePath = CORE::CombinePath( m_vfsRootPath, vfsFilename );
                ++loopIndex;
            }
            while ( vfs.FileExists( vfsStoragePath ) );

            if ( vfs.StoreAsFile( vfsStoragePath, *m_currentReadBuffer, 0, true ) )
            {
                m_msgsWrittenToStorage += (UInt32) bufferMetaData->msgOffsetIndex.size();
                m_msgBytesWrittenToStorage += m_currentReadBuffer->GetDataSize();
                AddPublishActionIdsToNotify( bufferMetaData->actionIds, true );

                if ( !m_config.indexDefinitions.empty() )
                {
                    CORE::UInt32 idxDefCount = static_cast< CORE::UInt32 >( m_config.indexDefinitions.size() );
                    for ( CORE::UInt32 idxDef=0; idxDef<idxDefCount; ++idxDef )
                    {
                        CStoragePubSubIndexDef& def = m_config.indexDefinitions[ idxDef ];
                        CORE::CString idxFilename = def.GetIndexFilename();
                        TIndexWriterMap::iterator writerIt = m_indexWriters.find( idxFilename );
                        if ( writerIt == m_indexWriters.end() )
                        {
                            m_indexWriters.insert( std::make_pair( idxFilename, CStoragePubSubIndexWriter( def, m_vfsRootPath ) ) );
                            writerIt = m_indexWriters.find( idxFilename );
                        }
                        writerIt->second.AppendContainer( m_config.pubsubBinarySerializerOptions,
                                                          *m_currentReadBuffer                  ,
                                                          bufferMetaData->msgOffsetIndex         ,
                                                          vfsFilename                            );
                    }
                }

                buffers.SignalEndOfReading();
                m_currentReadBuffer = GUCEF_NULL;

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:StoreNextReceivedPubSubBuffer: Successfully stored pub-sub mesage block at: " + vfsStoragePath );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StoragePubSubClientTopic:StoreNextReceivedPubSubBuffer: StoreAsFile() Failed" );
                AddPublishActionIdsToNotify( bufferMetaData->actionIds, false );
            }
        }
        else
        {
            // Check if the file does not already exists which is highly unlikely working with buffers and a millisecond batch callout
            CORE::CString vfsFilename;
            CORE::CString vfsStoragePath;
            CORE::CString vfsEncodedStoragePath;
            UInt32 loopIndex = 0;
            do
            {
                msgBatchDt.SetMilliseconds( msgBatchDt.GetMilliseconds() + loopIndex );
                vfsFilename = firstMsgDt.ToIso8601DateTimeString( false, true ) + '_' + lastMsgDt.ToIso8601DateTimeString( false, true )  + '_' + msgBatchDt.ToIso8601DateTimeString( false, true ) + m_vfsFilePostfix;
                vfsStoragePath = CORE::CombinePath( m_vfsRootPath, vfsFilename );
                vfsEncodedStoragePath = vfsStoragePath + '.' + m_config.encodeCodecName;
                ++loopIndex;
            }
            while ( vfs.FileExists( vfsEncodedStoragePath ) );

            if ( vfs.EncodeAsFile( *m_currentReadBuffer, 0, vfsStoragePath, true, m_config.encodeCodecFamily, m_config.encodeCodecName ) )
            {
                m_msgsWrittenToStorage += (UInt32) bufferMetaData->msgOffsetIndex.size();
                m_msgBytesWrittenToStorage += m_currentReadBuffer->GetDataSize();
                m_encodeSizeRatio = (CORE::Float32) ( m_currentReadBuffer->GetDataSize() / ( 1.0f * vfs.GetFileSize( vfsStoragePath ) ) );
                AddPublishActionIdsToNotify( bufferMetaData->actionIds, true );

                if ( !m_config.indexDefinitions.empty() )
                {
                    CORE::UInt32 idxDefCount = static_cast< CORE::UInt32 >( m_config.indexDefinitions.size() );
                    for ( CORE::UInt32 idxDef=0; idxDef<idxDefCount; ++idxDef )
                    {
                        CStoragePubSubIndexDef& def = m_config.indexDefinitions[ idxDef ];
                        CORE::CString idxFilename = def.GetIndexFilename();
                        TIndexWriterMap::iterator writerIt = m_indexWriters.find( idxFilename );
                        if ( writerIt == m_indexWriters.end() )
                        {
                            m_indexWriters.insert( std::make_pair( idxFilename, CStoragePubSubIndexWriter( def, m_vfsRootPath ) ) );
                            writerIt = m_indexWriters.find( idxFilename );
                        }
                        writerIt->second.AppendContainer( m_config.pubsubBinarySerializerOptions,
                                                          *m_currentReadBuffer                  ,
                                                          bufferMetaData->msgOffsetIndex         ,
                                                          vfsFilename                            );
                    }
                }

                buffers.SignalEndOfReading();
                m_currentReadBuffer = GUCEF_NULL;

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:StoreNextReceivedPubSubBuffer: Successfully encoded and stored pub-sub mesage block at: \"" + vfsStoragePath + "\" with a encoded size ratio of " + CORE::ToString( m_encodeSizeRatio ) );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StoragePubSubClientTopic:StoreNextReceivedPubSubBuffer: EncodeAsFile() Failed" );
                AddPublishActionIdsToNotify( bufferMetaData->actionIds, false );
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnUnableToFullFillStorageToPubSubRequest( const StorageToPubSubRequest& failedRequest )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::LocateFilesForStorageToPubSubRequest( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return false;

    if ( !m_vfsInitIsComplete )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "StoragePubSubClientTopic:LocateFilesForStorageToPubSubRequest: Deferring until VFS init has been completed" );
        return false;
    }

    bool totalSuccess = true;
    while ( !m_stage1StorageToPubSubRequests.empty() )
    {
        StorageToPubSubRequest& queuedRequest = (*m_stage1StorageToPubSubRequests.begin());

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:LocateFilesForStorageToPubSubRequest: Request for messages in range " +
            CORE::ToString( queuedRequest.startDt ) + " to " + CORE::ToString( queuedRequest.endDt ) );

        if ( GetPathsToPubSubStorageFiles( queuedRequest.startDt                      ,
                                           queuedRequest.endDt                        ,
                                           queuedRequest.vfsPubSubMsgContainersToPush ) && !queuedRequest.vfsPubSubMsgContainersToPush.empty() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:LocateFilesForStorageToPubSubRequest: Available data in the request range spans " +
                CORE::ToString( queuedRequest.vfsPubSubMsgContainersToPush.size() ) + " containers" );

            m_stage2StorageToPubSubRequests.push_front( queuedRequest );
            m_stage1StorageToPubSubRequests.pop_back();
        }
        else
        {
            if ( !queuedRequest.okIfZeroContainersAreFound )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:LocateFilesForStorageToPubSubRequest: Did not obtain any storage paths for time range " +
                    queuedRequest.startDt.ToIso8601DateTimeString( true, true ) + " to " + queuedRequest.endDt.ToIso8601DateTimeString( true, true ) );

                OnUnableToFullFillStorageToPubSubRequest( queuedRequest );
                totalSuccess = false;
            }

            // Are end-of-data (EOD) event messages desired?
            // A request may never find anything so we have to check here for EOD as well instead of just when having processed a set of files
            if ( m_client->GetConfig().desiredFeatures.supportsSubscriptionEndOfDataEvent )
            {
                if ( m_config.treatEveryFullfilledRequestAsEODEvent )
                {
                    m_subscriptionIsAtEndOfData = true;
                    NotifyObservers( SubscriptionEndOfDataEvent );
                }
                else
                {
                    // Only send end of data event if all requests have been fullfilled
                    if ( m_stage1StorageToPubSubRequests.size() <= 1 && m_stage2StorageToPubSubRequests.empty() )
                    {
                        m_subscriptionIsAtEndOfData = true;
                        NotifyObservers( SubscriptionEndOfDataEvent );
                    }
                }
            }

            m_stage1StorageToPubSubRequests.pop_back();
        }
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::ProcessNextPubSubRequestRelatedFile( void )
{GUCEF_TRACE;

    bool endOfData = false;
    bool totalSuccess = true;
    StorageToPubSubRequestDeque::iterator i = m_stage2StorageToPubSubRequests.begin();
    if ( i != m_stage2StorageToPubSubRequests.end() )
    {
        StorageToPubSubRequest& queuedRequest = (*i);

        size_t containersToProcess = queuedRequest.vfsPubSubMsgContainersPushed.size() + queuedRequest.vfsPubSubMsgContainersToPush.size();
        size_t containersProcessed = 0;
        TCContainerRangeInfoReferenceSet::iterator n = queuedRequest.vfsPubSubMsgContainersToPush.begin();
        while ( n != queuedRequest.vfsPubSubMsgContainersToPush.end() )
        {
            bool needContainerSubsetOnly = false;
            bool containerStartIsInRange = true;
            bool containerEndIsInRange = true;

            CORE::CDateTime containerFileFirstMsgDt;
            CORE::CDateTime containerFileLastMsgDt;
            CORE::CDateTime containerCaptureDt;
            if ( GetTimestampsFromContainerFilename( (*n)->vfsFilePath, containerFileFirstMsgDt, containerFileLastMsgDt, containerCaptureDt ) )
            {
                containerEndIsInRange = containerFileLastMsgDt.IsWithinRange( queuedRequest.startDt, queuedRequest.endDt );

                if ( 0 == (*n)->offsetInFile )
                {
                    containerStartIsInRange = containerFileFirstMsgDt.IsWithinRange( queuedRequest.startDt, queuedRequest.endDt );
                    needContainerSubsetOnly = !( containerStartIsInRange && containerEndIsInRange );
                }
                else
                {
                    containerStartIsInRange = false;
                    needContainerSubsetOnly = true;
                }

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Parsed file path container start and end DateTimes. Start=" +
                    CORE::ToString( containerFileFirstMsgDt ) + ", End=" + CORE::ToString( containerFileLastMsgDt ) + ". containerStartIsInRange=" + CORE::ToString( containerStartIsInRange ) +
                    ", containerEndIsInRange=" + CORE::ToString( containerEndIsInRange ) + ", needContainerSubsetOnly=" + CORE::ToString( needContainerSubsetOnly ) );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Failed to parse start and/or end DateTime from file at path \"" +
                    (*n)->vfsFilePath + "\". The file will be disregarded as invalid and skipped" );
                queuedRequest.vfsPubSubMsgContainersToPush.erase( (*n) );
                n = queuedRequest.vfsPubSubMsgContainersToPush.begin();
                totalSuccess = false;
                continue;
            }

            if ( GUCEF_NULL == m_currentWriteBuffer )
                m_currentWriteBuffer = GetSerializedMsgBuffers().GetNextWriterBuffer( false, 0 );

            if ( GUCEF_NULL != m_currentWriteBuffer )
            {
                StorageBufferMetaData* bufferMetaData = &( m_storageBufferMetaData[ m_currentWriteBuffer ] );
                bufferMetaData->Clear();
                bufferMetaData->linkedRequest = &queuedRequest;
                bufferMetaData->linkedRequestEntry = (*n);
                m_currentWriteBuffer->SetDataSize( 0 );

                // Check to see if we need to seek for the timestamp, or can we use bookmark info?
                if ( needContainerSubsetOnly && 0 == (*n)->offsetInFile )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Loading the container and seeking through a subset of its data to serve (part of) the request: " + bufferMetaData->linkedRequestEntry->vfsFilePath );
                    bool failureToLoadIsOk = false;
                    if ( LoadStorageFile( (*n)->vfsFilePath, *m_currentWriteBuffer, failureToLoadIsOk ) )
                    {
                        // Since we loaded the entire container we need to now efficiently make sure only the subset gets processed
                        // The way we can do that is by editing the footer in the buffer to logically eliminate entries we do not need
                        // This will make it appear as if only the needed entries are in the container to the reader when reading the footer

                        CORE::UInt32 startIndexOffset = 0;
                        CORE::UInt32 endIndexOffset = 0;

                        bool isCorrupted = false;
                        CORE::UInt32 bytesRead = 0;
                        PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector msgs;
                        if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild( msgs, true, bufferMetaData->msgOffsetIndex, *m_currentWriteBuffer, isCorrupted, m_config.bestEffortDeserializeIsAllowed ) )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Failed obtain messages from pubsub msg container at path \"" +
                                (*n)->vfsFilePath + "\". The file will be disregarded as invalid and skipped" );
                            queuedRequest.vfsPubSubMsgContainersToPush.erase( (*n) );
                            n = queuedRequest.vfsPubSubMsgContainersToPush.begin();
                            totalSuccess = false;
                            continue;
                        }

                        // Check to see if a rebuild was carried out
                        if ( isCorrupted )
                        {
                            // Make sure we log this because messages could be missing
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Successfully rebuild index for corrupted pubsub msg container at path \"" +
                                (*n)->vfsFilePath + "\". Found " + CORE::ToString( bufferMetaData->msgOffsetIndex.size() ) + " recoverable entries in the file" );
                        }

                        // Check to see how many we need to skip from the start
                        if ( !containerStartIsInRange )
                        {
                            PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector::iterator m = msgs.begin();
                            while ( m != msgs.end() )
                            {
                                if ( (*m).GetMsgDateTime() >= queuedRequest.startDt )
                                    break;
                                ++m; ++startIndexOffset;
                            }

                            (*n)->msgIndex = startIndexOffset;
                            (*n)->offsetInFile = bufferMetaData->msgOffsetIndex[ startIndexOffset ];
                        }
                        if ( !containerEndIsInRange )
                        {
                            PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector::reverse_iterator m = msgs.rbegin();
                            while ( m != msgs.rend() )
                            {
                                if ( (*m).GetMsgDateTime() <= queuedRequest.endDt )
                                    break;
                                ++m; ++endIndexOffset;
                            }

                            (*n)->hasEndDelimiter = true;
                            (*n)->lastMsgIndex = endIndexOffset;
                            (*n)->lastOffsetInFile = bufferMetaData->msgOffsetIndex[ endIndexOffset ];
                        }
                    }
                    else
                    {
                        if ( !failureToLoadIsOk )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Failed load pubsub msg container at path \"" +
                                (*n)->vfsFilePath + "\". The file will be disregarded as invalid and skipped" );
                        }
                        else
                        {
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: pubsub msg container not available to load at path \"" +
                                (*n)->vfsFilePath + "\". The file reference will be disregarded" );

                            (*n)->doneWithFile = 1;
                            m_completedContainers.insert( (*n) );
                        }

                        queuedRequest.vfsPubSubMsgContainersToPush.erase( (*n) );
                        n = queuedRequest.vfsPubSubMsgContainersToPush.begin();
                        totalSuccess = false;
                        continue;
                    }
                }
                else
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Loading the entire container as-is to serve (part of) the request: " + bufferMetaData->linkedRequestEntry->vfsFilePath );

                    bool failureToLoadIsOk = false;
                    if ( LoadStorageFile( (*n)->vfsFilePath, *m_currentWriteBuffer, failureToLoadIsOk ) )
                    {
                        queuedRequest.vfsPubSubMsgContainersPushed.insert( (*n) );
                        queuedRequest.vfsPubSubMsgContainersToPush.erase( (*n) );
                        n = queuedRequest.vfsPubSubMsgContainersToPush.begin();

                        // Since we loaded the entire container and we dont need a subset we are done
                        ++containersProcessed;
                        m_currentWriteBuffer = GUCEF_NULL;
                        m_buffers.SignalEndOfWriting();
                        bufferMetaData->isBeingWritten = false;
                        continue;
                    }
                    else
                    {
                        if ( !failureToLoadIsOk )
                        if ( !failureToLoadIsOk )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Failed load pubsub msg container at path \"" +
                                (*n)->vfsFilePath + "\". The file will be disregarded as invalid and skipped" );
                        }
                        else
                        {
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: pubsub msg container not available to load at path \"" +
                                (*n)->vfsFilePath + "\". The file reference will be disregarded" );

                            (*n)->doneWithFile = 1;
                            m_completedContainers.insert( (*n) );
                        }

                        queuedRequest.vfsPubSubMsgContainersToPush.erase( (*n) );
                        n = queuedRequest.vfsPubSubMsgContainersToPush.begin();
                        totalSuccess = false;
                        continue;
                    }
                }
            }
            else
            {
                // No write buffer available, we need to wait before processing more requests
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: No write buffer available at this time for loading next data set" );
                return totalSuccess;
            }
            ++n;
        }

        if ( containersProcessed > 0 && containersToProcess > 0 )
        {
            // Since we worked the request provide an update
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Loaded files for storage request. Start=" +
                CORE::ToString( queuedRequest.startDt ) + ", End=" + CORE::ToString( queuedRequest.endDt ) +
                " Proccessed " + CORE::ToString( containersProcessed ) + "/" + CORE::ToString( containersToProcess ) + " files successfully" );
        }

        if ( queuedRequest.vfsPubSubMsgContainersPushed.empty()      &&
             queuedRequest.vfsPubSubMsgContainersToPush.empty()      &&
             queuedRequest.vfsPubSubMsgContainersTransmitted.empty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProcessNextPubSubRequestRelatedFile: Storage request. Start=" +
                CORE::ToString( queuedRequest.startDt ) + ", End=" + CORE::ToString( queuedRequest.endDt ) +
                " has no associated containers, considering the request complete" );

            ProgressRequest( &queuedRequest, true );

            m_stage2StorageToPubSubRequests.erase( i );
        }
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnStoredPubSubMsgTransmissionFailure( const CORE::CDateTime& firstMsgDt )
{GUCEF_TRACE;

    // @TODO
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CStoragePubSubClientTopic::GetMsgsWrittenToStorageCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 msgsWrittenToStorage = m_msgsWrittenToStorage;
        m_msgsWrittenToStorage = 0;
        return msgsWrittenToStorage;
    }
    else
        return m_msgsWrittenToStorage;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CStoragePubSubClientTopic::GetMsgBytesWrittenToStorageCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 msgBytesWrittenToStorage = m_msgBytesWrittenToStorage;
        m_msgBytesWrittenToStorage = 0;
        return msgBytesWrittenToStorage;
    }
    else
        return m_msgBytesWrittenToStorage;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CStoragePubSubClientTopic::GetMsgsLoadedFromStorageCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 msgsLoadedFromStorage = m_msgsLoadedFromStorage;
        m_msgsLoadedFromStorage = 0;
        return msgsLoadedFromStorage;
    }
    else
        return m_msgsLoadedFromStorage;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CStoragePubSubClientTopic::GetMsgsNotifiedAsReceivedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 msgsNotifiedAsReceived = m_msgsNotifiedAsReceived;
        m_msgsNotifiedAsReceived = 0;
        return msgsNotifiedAsReceived;
    }
    else
        return m_msgsNotifiedAsReceived;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CStoragePubSubClientTopic::GetMsgBytesLoadedFromStorageCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 msgBytesLoadedFromStorage = m_msgBytesLoadedFromStorage;
        m_msgBytesLoadedFromStorage = 0;
        return msgBytesLoadedFromStorage;
    }
    else
        return m_msgBytesLoadedFromStorage;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CStoragePubSubClientTopic::GetStorageCorruptionDetectionCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 storageCorruptionDetections = m_storageCorruptionDetections;
        m_storageCorruptionDetections = 0;
        return storageCorruptionDetections;
    }
    else
        return m_storageCorruptionDetections;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CStoragePubSubClientTopic::GetStorageDeserializationFailuresCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 storageDeserializationFailures = m_storageDeserializationFailures;
        m_storageDeserializationFailures = 0;
        return storageDeserializationFailures;
    }
    else
        return m_storageDeserializationFailures;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::ProgressRequest( StorageToPubSubRequest* request ,
                                            bool isAcked                    )
{GUCEF_TRACE;

    if ( GUCEF_NULL == request || GUCEF_NULL == m_client )
        return;

    // Check to see if there is no additional work related to the overall request
    // if not we can perform some additional progression & cleanup
    if ( request->vfsPubSubMsgContainersToPush.empty() &&
         request->vfsPubSubMsgContainersPushed.empty()  )
    {
        // All containers for this request have had their content transmitted
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:ProgressRequest: Completed storage request. Start=" +
            CORE::ToString( request->startDt ) + ", End=" + CORE::ToString( request->endDt ) +
            ". Transmitted content of " + CORE::ToString( request->vfsPubSubMsgContainersTransmitted.size() ) + " containers. isAcked=" + CORE::ToString( isAcked ) );

        // Are end-of-data (EOD) event messages desired?
        if ( m_client->GetConfig().desiredFeatures.supportsSubscriptionEndOfDataEvent )
        {
            if ( m_config.treatEveryFullfilledRequestAsEODEvent )
            {
                m_subscriptionIsAtEndOfData = true;
                NotifyObservers( SubscriptionEndOfDataEvent );
            }
            else
            {
                // Only send end of data event if all requests have been fullfilled
                if ( m_stage1StorageToPubSubRequests.empty() && m_stage2StorageToPubSubRequests.size() <= 1 )
                {
                    m_subscriptionIsAtEndOfData = true;
                    NotifyObservers( SubscriptionEndOfDataEvent );
                }
            }
        }

        if ( m_needToTrackAcks && isAcked )
        {
            // Retain the container references a little longer for bookmarking purposes
            TCContainerRangeInfoReferenceSet::iterator c = request->vfsPubSubMsgContainersTransmitted.begin();
            while ( c != request->vfsPubSubMsgContainersTransmitted.end() )
            {
                (*c)->doneWithFile = 1;
                m_completedContainers.insert( (*c) );
                ++c;
            }

            // let's not keep an unlimited amount of these old references as we'd run out of memory over time
            if ( m_completedContainers.size() > m_config.maxCompletedContainerRefsToRetain )
            {
                UInt32 referencesToPrune = (UInt32) m_completedContainers.size() - m_config.maxCompletedContainerRefsToRetain;
                for ( UInt32 pruned=0; pruned < referencesToPrune; ++pruned )
                {
                    m_completedContainers.erase( m_completedContainers.begin() );
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::ProgressRequest( StorageBufferMetaData* bufferMetaData ,
                                            bool isTransmitted                    ,
                                            bool isAcked                          )
{GUCEF_TRACE;

    if ( GUCEF_NULL == bufferMetaData || GUCEF_NULL == m_client )
        return;

    StorageToPubSubRequest* request = bufferMetaData->linkedRequest;
    if ( GUCEF_NULL != request )
    {
        if ( isTransmitted )
        {
            bool firstTransmission = request->vfsPubSubMsgContainersPushed.end() != request->vfsPubSubMsgContainersPushed.find( bufferMetaData->linkedRequestEntry );
            if ( firstTransmission )
            {
                request->vfsPubSubMsgContainersTransmitted.insert( bufferMetaData->linkedRequestEntry );
                request->vfsPubSubMsgContainersPushed.erase( bufferMetaData->linkedRequestEntry );
            }

            // We are done with the transmission-as-message portion of the container request handling
            // check to see if we need to wait any longer for ack handling
            if ( !m_needToTrackAcks || isAcked )
            {
                // Since we are done with this specific container we can update
                // the 'bookmark' portion of the request entry to match what we have completed
                // This makes sure that if the info is retrieved and stored as a bookmark that it will
                // reference the correct position
                if ( bufferMetaData->linkedRequestEntry->hasEndDelimiter )
                {
                    bufferMetaData->linkedRequestEntry->msgIndex = bufferMetaData->linkedRequestEntry->lastMsgIndex;
                    bufferMetaData->linkedRequestEntry->offsetInFile = bufferMetaData->linkedRequestEntry->lastOffsetInFile;
                }
                else
                {
                    bufferMetaData->linkedRequestEntry->msgIndex = (UInt32) bufferMetaData->msgOffsetIndex.size()-1;
                    bufferMetaData->linkedRequestEntry->offsetInFile = bufferMetaData->msgOffsetIndex[ bufferMetaData->linkedRequestEntry->msgIndex ];
                }

                ProgressRequest( request, isAcked );

                // Check to see if there is no additional work related to the overall request
                // if not we can perform some additional progression & cleanup
                if ( request->vfsPubSubMsgContainersToPush.empty() &&
                     request->vfsPubSubMsgContainersPushed.empty()  )
                {
                    // get rid of the request itself since its completed
                    StorageToPubSubRequestDeque::iterator i = m_stage2StorageToPubSubRequests.begin();
                    while ( i != m_stage2StorageToPubSubRequests.end() )
                    {
                        if ( request == &(*i) )
                        {
                            bufferMetaData->linkedRequest = GUCEF_NULL;
                            m_stage2StorageToPubSubRequests.erase( i );

                            bufferMetaData->Clear();
                            break;
                        }
                        ++i;
                    }
                }

                // @TODO: multiple in-flight read buffers for speed
                m_buffers.SignalEndOfReading();
                bufferMetaData->isReleased = true;
                m_currentReadBuffer = GUCEF_NULL;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::TransmitNextPubSubMsgBatch( CORE::CDynamicBuffer* readBuffer      ,
                                                       StorageBufferMetaData* bufferMetaData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == readBuffer )
        return false;

    if ( GUCEF_NULL == bufferMetaData )
    {
        // fetch the meta-data for this buffer
        bufferMetaData = &( m_storageBufferMetaData[ readBuffer ] );
    }

    // Check to see what that means for the current batch
    // the max allowed vs whatever if already in flight gives us the allowed additional nr of messages to transmit
    CORE::UInt32 msgsMaxLeftToTransmit = 0;
    if ( m_maxTotalMsgsInFlight > 0 )
    {
        msgsMaxLeftToTransmit = (CORE::UInt32) ( m_maxTotalMsgsInFlight - bufferMetaData->msgsInFlight );
        CORE::Int64 availableMsgs = bufferMetaData->msgs.size() - bufferMetaData->lastMsgTransmittedIndex;
        if ( availableMsgs < msgsMaxLeftToTransmit )
            msgsMaxLeftToTransmit = (CORE::UInt32) availableMsgs;
    }
    else
    {
        msgsMaxLeftToTransmit = (CORE::UInt32) bufferMetaData->msgs.size() - bufferMetaData->lastMsgTransmittedIndex;
    }

    // If there is no capacity for extra messages in flight then come back later
    if ( msgsMaxLeftToTransmit == 0 )
        return true;

    bufferMetaData->pubsubMsgsRefs.clear();
    bufferMetaData->pubsubMsgsRefs.reserve( msgsMaxLeftToTransmit );

    for ( UInt32 i=0; i<msgsMaxLeftToTransmit; ++i )
    {
        UInt32 msgIndex = bufferMetaData->lastMsgTransmittedIndex + i;
        bufferMetaData->pubsubMsgsRefs.push_back( TPubSubMsgRef( &bufferMetaData->msgs[ msgIndex ] ) );
    }
    bufferMetaData->lastMsgTransmittedIndex += msgsMaxLeftToTransmit;

    // We use the 'msgsInFlight' counter to control whether we just notify in batches of max 'msgsInFlight' messages
    // or whether we actually wait for acks to come back. If we don't increment it we fire-and-forget cut batches
    if ( m_needToTrackAcks )
        bufferMetaData->msgsInFlight += msgsMaxLeftToTransmit;

    if ( !NotifyObservers( MsgsRecievedEvent, &bufferMetaData->pubsubMsgsRefs ) )
        return true;

    // increment metric
    m_msgsNotifiedAsReceived += static_cast< CORE::UInt32 >( bufferMetaData->pubsubMsgsRefs.size() );

    if ( bufferMetaData->lastMsgTransmittedIndex >= (bufferMetaData->msgs.size()-1) )
    {
        ProgressRequest( bufferMetaData, true, false );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::TransmitNextPubSubMsgBuffer( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_currentReadBuffer )
        return TransmitNextPubSubMsgBatch( m_currentReadBuffer, GUCEF_NULL );

    CORE::CDateTime firstMsgDt;
    m_currentReadBuffer = m_buffers.GetNextReaderBuffer( firstMsgDt, false, 0 );
    if ( GUCEF_NULL == m_currentReadBuffer )
        return true; // nothing to do

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
        "):TransmitNextPubSubMsgBuffer: New buffer with data is available of " + CORE::ToString( m_currentReadBuffer->GetDataSize() ) + " bytes" );

    // fetch the meta-data for this buffer
    StorageBufferMetaData* bufferMetaData = &( m_storageBufferMetaData[ m_currentReadBuffer ] );

    // We now link logical message objects to the data in the buffer
    bool isCorrupted = false;
    CORE::UInt32 bytesRead = 0;
    bufferMetaData->msgsInFlight = 0;
    bufferMetaData->lastMsgTransmittedIndex = 0;
    bufferMetaData->msgs.clear();
    bufferMetaData->msgOffsetIndex.clear();              // @TODO: Only read subset needed with params
    if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild( bufferMetaData->msgs, true, bufferMetaData->msgOffsetIndex, *m_currentReadBuffer, isCorrupted, m_config.bestEffortDeserializeIsAllowed ) )
    {
        // update metrics
        if ( isCorrupted )
            ++m_storageCorruptionDetections;
        ++m_storageDeserializationFailures;

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
            "):TransmitNextPubSubMsgBuffer: Failed to deserialize messages from container even with rebuild. According to the footer (if any existed) the container had " +
            CORE::ToString( bufferMetaData->msgOffsetIndex.size() ) + " entries. isCorrupted=" + CORE::BoolToString( isCorrupted ) );

        OnStoredPubSubMsgTransmissionFailure( firstMsgDt );

        m_buffers.SignalEndOfReading();
        bufferMetaData->Clear();
        m_currentReadBuffer = GUCEF_NULL;
        return false;
    }
    if ( isCorrupted )
    {
        // update metrics
        ++m_storageDeserializationFailures;
        ++m_storageCorruptionDetections;

        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic(" + CORE::PointerToString( this ) +
            "):TransmitNextPubSubMsgBuffer: Deserialize messages from container but corruption was detected. According to the index the container has " +
            CORE::ToString( bufferMetaData->msgOffsetIndex.size() ) + " entries" );
    }

    // update metrics
    m_msgsLoadedFromStorage += static_cast< CORE::UInt32 >( bufferMetaData->msgs.size() );
    m_msgBytesLoadedFromStorage += m_currentReadBuffer->GetDataSize();

    PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector::iterator seriesEnd;
    PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector::iterator i = bufferMetaData->msgs.begin();
    if ( i != bufferMetaData->msgs.end() )
    {
        UInt32 nrOfMessages = 0;
        i += bufferMetaData->linkedRequestEntry->msgIndex;

        if ( bufferMetaData->linkedRequestEntry->hasEndDelimiter )
        {
            seriesEnd = bufferMetaData->msgs.begin();
            seriesEnd += bufferMetaData->linkedRequestEntry->lastMsgIndex;
            nrOfMessages = bufferMetaData->linkedRequestEntry->lastMsgIndex - bufferMetaData->linkedRequestEntry->msgIndex;
        }
        else
        {
            seriesEnd = bufferMetaData->msgs.end();
            nrOfMessages = (UInt32) ( (bufferMetaData->msgs.size()-1) - bufferMetaData->linkedRequestEntry->msgIndex );
        }

        bufferMetaData->actionIds.clear();
        bufferMetaData->actionIds.reserve( nrOfMessages );
        bufferMetaData->pubsubMsgsRefs.clear();
        bufferMetaData->pubsubMsgsRefs.reserve( nrOfMessages );
        bufferMetaData->msgAcks.clear();
        if ( m_needToTrackAcks )
        {            
            bufferMetaData->msgAcks.reserve( nrOfMessages );
        }
        bufferMetaData->lastMsgTransmittedIndex = 0;

        bufferMetaData->linkedRequestEntry->minActionId = m_currentReceiveActionId;
        while ( i != seriesEnd )
        {
            PUBSUB::CBasicPubSubMsg& msg = (*i);

            msg.SetOriginClientTopic( CreateSharedPtr() );

            msg.SetReceiveActionId( m_currentReceiveActionId );
            bufferMetaData->actionIds.push_back( m_currentReceiveActionId );
            ++m_currentReceiveActionId;

            if ( m_needToTrackAcks )
                bufferMetaData->msgAcks.push_back( false );

            ++i;
        }
        bufferMetaData->linkedRequestEntry->maxActionId = m_currentReceiveActionId-1;

        // Now that we have staged the messages, transmit the first batch
        return TransmitNextPubSubMsgBatch( m_currentReadBuffer, GUCEF_NULL );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::IsSubscriptionAtEndOfData( void ) const
{GUCEF_TRACE;

    return m_subscriptionIsAtEndOfData;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopic::OnVfsInitializationCompleted( CORE::CNotifier* notifier    ,
                                                         const CORE::CEvent& eventId  ,
                                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_vfsInitIsComplete = true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStoragePubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::STORAGE::CStoragePubSubClientTopic";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopic::SetJournal( PUBSUB::CIPubSubJournalBasicPtr journal )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_journal = journal;
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CIPubSubJournalBasicPtr
CStoragePubSubClientTopic::GetJournal( void ) const
{GUCEF_TRACE;

    return m_journal;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CStoragePubSubClientTopic::RequestReplay( const PUBSUB::CPubSubBookmark& startBookmark   ,
                                          const PUBSUB::CPubSubBookmark& endBookmark     ,
                                          CORE::UInt64                   replayRequestId ,
                                          PUBSUB::CPubSubClientSide*     requestingSide  ,
                                          PUBSUB::CPubSubClientTopic*    requestingTopic )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StoragePubSubClientTopic:RequestReplay: Starting replay for replayRequestId=" +
        CORE::ToString( replayRequestId ) + " from " + startBookmark.ToString() + " to " + endBookmark.ToString() +
        ", Topic: " + m_config.topicName );

    if ( GUCEF_NULL == requestingSide )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StoragePubSubClientTopic:RequestReplay: requestingSide is null, Topic: " + m_config.topicName );
        return CORE::CFutureResult( CORE::TASKSTATUS_TASKDATA_INVALID );
    }

    CStorageReplayTaskData* taskData = GUCEF_NEW CStorageReplayTaskData();
    taskData->startBookmark                  = startBookmark;
    taskData->endBookmark                    = endBookmark;
    taskData->replayRequestId                = replayRequestId;
    taskData->requestingSide                 = requestingSide;
    taskData->requestingTopic                = requestingTopic;
    taskData->vfsRootPath                    = m_vfsRootPath;
    taskData->containerFileFilter            = '*' + GenerateVfsStorageContainerFileExt();
    taskData->bestEffortDeserializeIsAllowed = m_config.bestEffortDeserializeIsAllowed;
    taskData->indexDefinitions               = m_config.indexDefinitions;

    CORE::CString poolName = m_config.replayThreadPoolName.IsNULLOrEmpty()
        ? CORE::CTaskManager::DefaultThreadPoolName
        : m_config.replayThreadPoolName;

    CORE::CFutureResult future = CORE::CCoreGlobal::Instance()->GetTaskManager().QueueTask(
        poolName                                     ,
        CStorageReplayTaskConsumer::TaskTypeName()   ,
        taskData                                     ,
        GUCEF_NULL                                   ,
        true  /* assumeOwnershipOfTaskData */        );

    if ( future.HasNoFuture() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StoragePubSubClientTopic:RequestReplay: Failed to queue replay task for replayRequestId=" +
            CORE::ToString( replayRequestId ) + ", pool=" + poolName + ", Topic: " + m_config.topicName );
    }
    return future;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STORAGE */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
