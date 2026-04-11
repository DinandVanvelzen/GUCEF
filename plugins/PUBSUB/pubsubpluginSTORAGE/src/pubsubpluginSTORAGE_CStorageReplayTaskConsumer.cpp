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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgContainerBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#include "gucefPUBSUB_CPubSubClientSide.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H
#include "pubsubpluginSTORAGE_CStoragePubSubClientTopic.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXREADER_H
#include "pubsubpluginSTORAGE_CStoragePubSubIndexReader.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXREADER_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXREADER_H ? */

#include "pubsubpluginSTORAGE_CStorageReplayTaskData.h"
#include "pubsubpluginSTORAGE_CStorageReplayTaskConsumer.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStorageReplayTaskConsumer::CStorageReplayTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CStorageReplayTaskConsumer::~CStorageReplayTaskConsumer()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStorageReplayTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskTypeName();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStorageReplayTaskConsumer::TaskTypeName( void )
{GUCEF_TRACE;

    static const CORE::CString typeName = "StorageReplay";
    return typeName;
}

/*-------------------------------------------------------------------------*/

bool
CStorageReplayTaskConsumer::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    CStorageReplayTaskData* taskData = static_cast< CStorageReplayTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == taskData )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: Task has no data" );
        return false;
    }

    if ( GUCEF_NULL == taskData->requestingSide )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: requestingSide is null" );
        return false;
    }

    taskData->containerFilePaths.clear();
    taskData->currentFileIndex = 0;
    taskData->startMsgIndex    = 0;

    const PUBSUB::CPubSubBookmark& startBm = taskData->startBookmark;

    if ( startBm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_INDEX_KEY_VALUE )
    {
        /* Parse keyNameWithPrefix: "mk:<keyName>" or "k:<keyName>" */
        const CORE::CString& keyFieldWithPrefix = startBm.GetBookmarkKeyField();
        CStoragePubSubIndexDef::EKeySource keySource = CStoragePubSubIndexDef::KEY_SOURCE_META_DATA;
        CORE::CString keyName;

        if ( keyFieldWithPrefix.HasSubstr( "mk:", true ) )
        {
            keySource = CStoragePubSubIndexDef::KEY_SOURCE_META_DATA;
            keyName   = keyFieldWithPrefix.CutChars( 3, true, 0 );
        }
        else if ( keyFieldWithPrefix.HasSubstr( "k:", true ) )
        {
            keySource = CStoragePubSubIndexDef::KEY_SOURCE_KV_PAIR;
            keyName   = keyFieldWithPrefix.CutChars( 2, true, 0 );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: Invalid key field prefix (expected 'mk:' or 'k:'): " + keyFieldWithPrefix );
            return false;
        }

        /* Find a matching index definition */
        const CStoragePubSubIndexDef* matchedDef = GUCEF_NULL;
        CORE::UInt32 defCount = static_cast< CORE::UInt32 >( taskData->indexDefinitions.size() );
        for ( CORE::UInt32 i=0; i<defCount; ++i )
        {
            const CStoragePubSubIndexDef& def = taskData->indexDefinitions[ i ];
            if ( def.keySource == keySource && def.keyName == keyName )
            {
                matchedDef = &def;
                break;
            }
        }

        if ( GUCEF_NULL == matchedDef )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: No index definition found for key: " + keyFieldWithPrefix );
            return false;
        }

        CStoragePubSubIndexReader reader( *matchedDef, taskData->vfsRootPath );
        if ( !reader.LoadIndex() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: Failed to load index for key: " + keyFieldWithPrefix );
            return false;
        }

        CORE::UInt64 startKeyValue = startBm.GetBookmarkData().AsUInt64( 0 );

        CStoragePubSubClientTopic::CStorageBookmarkInfo bookmark;
        if ( !reader.FindStartBookmark( startKeyValue, bookmark ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: No index entry found for startKeyValue=" +
                CORE::ToString( startKeyValue ) + " in index for key: " + keyFieldWithPrefix );
            return false;
        }

        taskData->startMsgIndex = bookmark.msgIndex;

        if ( !reader.GetOrderedFilePathsFromFile( bookmark.vfsFilePath, taskData->containerFilePaths ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: Failed to get ordered file paths from: " + bookmark.vfsFilePath );
            return false;
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: Replay requestId=" +
            CORE::ToString( taskData->replayRequestId ) + " key=" + keyFieldWithPrefix +
            " startValue=" + CORE::ToString( startKeyValue ) + " -> file=" + bookmark.vfsFilePath +
            " msgIndex=" + CORE::ToString( bookmark.msgIndex ) + " totalFiles=" +
            CORE::ToString( static_cast< CORE::UInt32 >( taskData->containerFilePaths.size() ) ) );
    }
    else if ( startBm.GetBookmarkType() == PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_INITIALIZED )
    {
        /* Replay from the beginning.
         * If we have index definitions use the first one to obtain the ordered file registry;
         * otherwise fall back to a VFS directory listing. */

        if ( !taskData->indexDefinitions.empty() )
        {
            CStoragePubSubIndexReader reader( taskData->indexDefinitions[ 0 ], taskData->vfsRootPath );
            if ( reader.LoadIndex() )
            {
                /* Get all files from the very first entry in the registry */
                CStoragePubSubClientTopic::CStorageBookmarkInfo firstBookmark;
                if ( reader.FindStartBookmark( 0, firstBookmark ) )
                {
                    if ( reader.GetOrderedFilePathsFromFile( firstBookmark.vfsFilePath, taskData->containerFilePaths ) )
                    {
                        taskData->startMsgIndex = 0;
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: Replay requestId=" +
                            CORE::ToString( taskData->replayRequestId ) + " from beginning via index; totalFiles=" +
                            CORE::ToString( static_cast< CORE::UInt32 >( taskData->containerFilePaths.size() ) ) );
                    }
                }
            }
        }

        if ( taskData->containerFilePaths.empty() )
        {
            /* Fall back: list all files in vfsRootPath sorted by name */
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            CORE::CString fileFilter = taskData->containerFileFilter.IsNULLOrEmpty() ? CORE::CString( "*" ) : taskData->containerFileFilter;
            VFS::CVFS::TStringVector listing;
            if ( !vfs.GetFileList( listing, taskData->vfsRootPath, false, true, fileFilter, true ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: VFS file listing failed for: " + taskData->vfsRootPath );
                return false;
            }

            taskData->containerFilePaths.reserve( listing.size() );
            VFS::CVFS::TStringVector::const_iterator it = listing.begin();
            while ( it != listing.end() )
            {
                taskData->containerFilePaths.push_back( *it );
                ++it;
            }
            taskData->startMsgIndex = 0;

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: Replay requestId=" +
                CORE::ToString( taskData->replayRequestId ) + " from beginning via VFS listing; totalFiles=" +
                CORE::ToString( static_cast< CORE::UInt32 >( taskData->containerFilePaths.size() ) ) );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: Unsupported start bookmark type" );
        return false;
    }

    if ( taskData->containerFilePaths.empty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskStart: No container files to replay for requestId=" +
            CORE::ToString( taskData->replayRequestId ) );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStorageReplayTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    CStorageReplayTaskData* taskData = static_cast< CStorageReplayTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == taskData )
        return true;

    CORE::UInt32 totalFiles = static_cast< CORE::UInt32 >( taskData->containerFilePaths.size() );

    if ( taskData->currentFileIndex >= totalFiles )
        return true;  /* all files processed, task is done */

    const CORE::CString& containerPath = taskData->containerFilePaths[ taskData->currentFileIndex ];

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskCycle: requestId=" +
        CORE::ToString( taskData->replayRequestId ) + " processing file " +
        CORE::ToString( taskData->currentFileIndex + 1 ) + "/" + CORE::ToString( totalFiles ) +
        " : " + containerPath );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDynamicBuffer containerBuffer;
    if ( !vfs.LoadFile( containerBuffer, containerPath, "rb" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskCycle: Failed to load container: " + containerPath +
            " for requestId=" + CORE::ToString( taskData->replayRequestId ) );
        ++taskData->currentFileIndex;
        return taskData->currentFileIndex >= totalFiles;
    }

    PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector msgs;
    bool isCorrupted = false;
    if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild(
            msgs, true, containerBuffer, isCorrupted, taskData->bestEffortDeserializeIsAllowed ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskCycle: Deserialization failed for: " + containerPath +
            " requestId=" + CORE::ToString( taskData->replayRequestId ) + " isCorrupted=" + CORE::BoolToString( isCorrupted ) );
        ++taskData->currentFileIndex;
        return taskData->currentFileIndex >= totalFiles;
    }

    CORE::UInt32 msgCount = static_cast< CORE::UInt32 >( msgs.size() );
    CORE::UInt32 startMsg = ( taskData->currentFileIndex == 0 ) ? taskData->startMsgIndex : 0;

    if ( startMsg < msgCount )
    {
        PUBSUB::CPubSubClientTopic::TPubSubMsgsRefVector refs;
        refs.reserve( msgCount - startMsg );
        for ( CORE::UInt32 i=startMsg; i<msgCount; ++i )
        {
            refs.push_back( static_cast< PUBSUB::CIPubSubMsg* >( &msgs[ i ] ) );
        }

        if ( !taskData->requestingSide->OnReplayMsgsReceived( taskData->replayRequestId, taskData->requestingTopic, refs ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskCycle: OnReplayMsgsReceived failed for requestId=" +
                CORE::ToString( taskData->replayRequestId ) + " file=" + containerPath );
        }
    }

    ++taskData->currentFileIndex;
    return taskData->currentFileIndex >= totalFiles;
}

/*-------------------------------------------------------------------------*/

void
CStorageReplayTaskConsumer::OnTaskEnding( CORE::CTaskPtr task ,
                                          bool willBeForced   )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

void
CStorageReplayTaskConsumer::OnTaskEnded( CORE::CTaskPtr task ,
                                         bool wasForced      )
{GUCEF_TRACE;

    if ( wasForced )
        return;

    CStorageReplayTaskData* taskData = static_cast< CStorageReplayTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == taskData || GUCEF_NULL == taskData->requestingSide )
        return;

    taskData->requestingSide->OnReplayComplete( taskData->replayRequestId, taskData->requestingTopic );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CStorageReplayTaskConsumer:OnTaskEnded: Replay complete for requestId=" +
        CORE::ToString( taskData->replayRequestId ) );
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
