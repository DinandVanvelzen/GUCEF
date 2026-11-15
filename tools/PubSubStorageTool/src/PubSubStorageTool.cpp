/*
 *  PubSubStorageTool: Tool to manipulate archives
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "PubSubStorageTool.h"

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTimeBoundContainer::CTimeBoundContainer( void )
    : CORE::CTSharedObjCreator< CTimeBoundContainer, MT::CNoLock >( this )
    , minClampedDt()
    , maxClampedDt()
    , creationDt()
    , tempContainerPath() 
    , resource()
    , index()
    , earliestMsgDt( CORE::CDateTime::FutureMax )
    , newestMsgDt( CORE::CDateTime::PastMax )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimeBoundContainer::~CTimeBoundContainer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSubStorageTool::PubSubStorageTool( void )
    : CORE::CObservingNotifier()
    , m_workStartTrigger()
    , m_console( GUCEF_NULL )
    , m_params()
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

PubSubStorageTool::~PubSubStorageTool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
PubSubStorageTool::OnWorkStartTrigger( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    PerformWork();
}

/*-------------------------------------------------------------------------*/

void 
PubSubStorageTool::RegisterEventHandlers( void )
{GUCEF_TRACE;

    // Get access to the systems this application depends on
    CORE::CGUCEFApplication& app = CORE::CCoreGlobal::Instance()->GetApplication();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    // Set our trigger criterea for starting work
    // depending on the application needs you want different systems to be ready and initialized before starting work
    m_workStartTrigger.SubscribeAndAddEventToTriggerCriterea( &app, CORE::CGUCEFApplication::FirstCycleEvent );
    m_workStartTrigger.SubscribeAndAddEventToTriggerCriterea( &vfs, VFS::CVFS::VfsInitializationCompletedEvent );
    
    // Now hook up the actual trigger for starting work
    TEventCallback callback( this, &PubSubStorageTool::OnWorkStartTrigger );
    SubscribeTo( &m_workStartTrigger                                 ,
                 CORE::CEventAggregateTrigger::AggregateTriggerEvent ,
                 callback                                            );
}

/*-------------------------------------------------------------------------*/

void 
PubSubStorageTool::SetConsoleAccess( CORE::CPlatformNativeConsoleLogger& console )
{GUCEF_TRACE;

    m_console = &console;
}

/*-------------------------------------------------------------------------*/

void 
PubSubStorageTool::SetParams( const CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    m_params = keyValueList;
}

/*-------------------------------------------------------------------------*/

bool
PubSubStorageTool::AddLocalPathAsVfsRoot( const CORE::CString& path, CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CString fsPath = CORE::RelativePath( path );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    if ( !fsPath.IsNULLOrEmpty() )
    {
        if ( vfs.AddRoot( fsPath, "root", true, false, "localFileSystem" ) )
        {
            vfsPath = "localFileSystem";
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool: Added local path as VFS root: " + fsPath );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool: Failed to add local path as VFS root: " + fsPath );            
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool: Local FileSystem path is empty, cannot add as VFS root" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
PubSubStorageTool::DetermineLargestFileSize( const CORE::CString::StringVector& files ,
                                             UInt64& fileSize                         ) const
{GUCEF_TRACE;

    UInt64 maxFileSize = 0;
    bool totalSuccess = true;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( CORE::CString::StringVector::const_iterator it = files.begin(); it != files.end(); ++it )
    {
        const CORE::CString& filePath = (*it);
        UInt64 size = vfs.GetFileSize( filePath );
        if ( size > maxFileSize )
            maxFileSize = size;
    }

    fileSize = maxFileSize;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:DetermineLargestFileSize: Discovered that the largest file size across " + 
        CORE::ToString( files.size() ) + " files is " + CORE::ToString( maxFileSize ) );

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
PubSubStorageTool::DetermineStorageOptions( const CORE::CString::StringVector& files                     ,   
                                            PUBSUB::CPubSubMsgBinarySerializerOptions& atLeastOneOptions ,
                                            PUBSUB::CPubSubMsgBinarySerializerOptions& unanimousOptions  ,
                                            bool& sourceFilesAreHomogenous                               ) const
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:DetermineStorageOptions: Discovering common options across " + CORE::ToString( files.size() ) + " files" );
    
    // Find the common denominator for the options
    // If just one file supports a given option we will enable it
    // If no files support a given option we will disable it

    atLeastOneOptions.Clear();
    unanimousOptions.Clear();
    sourceFilesAreHomogenous = true;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    bool totalSuccess = true;

    UInt64 msgIdIncluded = 0;
    UInt64 msgIndexIncluded = 0;
    UInt64 msgDateTimeIncluded = 0;
    UInt64 msgDateTimeAsMsSinceUnixEpochInUtc = 0;
    UInt64 msgPrimaryPayloadIncluded = 0;
    UInt64 msgKeyValuePairsIncluded = 0;
    UInt64 msgMetaDataKeyValuePairsIncluded = 0;
    UInt64 topicNameIncluded = 0;

    for ( CORE::CString::StringVector::const_iterator it = files.begin(); it != files.end(); ++it )
    {
        const CORE::CString& filePath = (*it);
        VFS::TBasicVfsResourcePtr file = vfs.GetFile( filePath, "rb", false );
        if ( file.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:DetermineStorageOptions: Failed to open file " + filePath );
            totalSuccess = false;
            continue;
        }

        CORE::IOAccessPtr access = file->GetAccess();
        if ( access.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:DetermineStorageOptions: Failed to open file access: " + filePath );
            totalSuccess = false;
            continue;
        }

        UInt32 bytesRead = 0;
        PUBSUB::CPubSubMsgBinarySerializerOptions singleFileOptions;
        if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeHeader( singleFileOptions, *access, bytesRead ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:DetermineStorageOptions: Failed to deserialize PubSubMsgContainer header from file " + filePath );
            totalSuccess = false;
            continue;
        }

        if ( singleFileOptions.msgDateTimeAsMsSinceUnixEpochInUtc ) 
            msgDateTimeAsMsSinceUnixEpochInUtc++;
        if ( singleFileOptions.msgDateTimeIncluded ) 
            msgDateTimeIncluded++;
        if ( singleFileOptions.msgIdIncluded ) 
            msgIdIncluded++;
        if ( singleFileOptions.msgIndexIncluded ) 
            msgIndexIncluded++;
        if ( singleFileOptions.msgKeyValuePairsIncluded ) 
            msgKeyValuePairsIncluded++;
        if ( singleFileOptions.msgMetaDataKeyValuePairsIncluded ) 
            msgMetaDataKeyValuePairsIncluded++;
        if ( singleFileOptions.msgPrimaryPayloadIncluded ) 
            msgPrimaryPayloadIncluded++;
        if ( singleFileOptions.topicNameIncluded ) 
            topicNameIncluded++;
    }

    // If just one file supports a given option we will enable the option 
    // since any target file would have to support it to hold the superset of messages
    if ( msgIdIncluded > 0 ) 
        atLeastOneOptions.msgIdIncluded = true;
    if ( msgIndexIncluded > 0 ) 
        atLeastOneOptions.msgIndexIncluded = true;
    if ( msgDateTimeIncluded > 0 ) 
        atLeastOneOptions.msgDateTimeIncluded = true;
    if ( msgDateTimeAsMsSinceUnixEpochInUtc > 0 ) 
        atLeastOneOptions.msgDateTimeAsMsSinceUnixEpochInUtc = true;
    if ( msgPrimaryPayloadIncluded > 0 ) 
        atLeastOneOptions.msgPrimaryPayloadIncluded = true;
    if ( msgKeyValuePairsIncluded > 0 ) 
        atLeastOneOptions.msgKeyValuePairsIncluded = true;
    if ( msgMetaDataKeyValuePairsIncluded > 0 ) 
        atLeastOneOptions.msgMetaDataKeyValuePairsIncluded = true;
    if ( topicNameIncluded > 0 ) 
        atLeastOneOptions.topicNameIncluded = true;
    
    // Determine which options are unanimous across all files
    // This doesnt mean the files are homogenous, just that the options flagged as unanimous are
    unanimousOptions.msgIdIncluded = atLeastOneOptions.msgIdIncluded ? ( msgIdIncluded == files.size() ) : false;
    unanimousOptions.msgIndexIncluded = atLeastOneOptions.msgIndexIncluded ? ( msgIndexIncluded == files.size() ) : false;
    unanimousOptions.msgDateTimeIncluded = atLeastOneOptions.msgDateTimeIncluded ? ( msgDateTimeIncluded == files.size() ) : false;
    unanimousOptions.msgDateTimeAsMsSinceUnixEpochInUtc = atLeastOneOptions.msgDateTimeAsMsSinceUnixEpochInUtc ? ( msgDateTimeAsMsSinceUnixEpochInUtc == files.size() ) : false;
    unanimousOptions.msgPrimaryPayloadIncluded = atLeastOneOptions.msgPrimaryPayloadIncluded ? ( msgPrimaryPayloadIncluded == files.size() ) : false;
    unanimousOptions.msgKeyValuePairsIncluded = atLeastOneOptions.msgKeyValuePairsIncluded ? ( msgKeyValuePairsIncluded == files.size() ) : false;
    unanimousOptions.msgMetaDataKeyValuePairsIncluded = atLeastOneOptions.msgMetaDataKeyValuePairsIncluded ? ( msgMetaDataKeyValuePairsIncluded == files.size() ) : false;
    unanimousOptions.topicNameIncluded = atLeastOneOptions.topicNameIncluded ? ( topicNameIncluded == files.size() ) : false;
        
    sourceFilesAreHomogenous = ( 0 == msgIdIncluded || msgIdIncluded == files.size() ) &&
                               ( 0 == msgIndexIncluded || msgIndexIncluded == files.size() ) &&        
                               ( 0 == msgDateTimeIncluded || msgDateTimeIncluded == files.size() ) &&
                               ( 0 == msgDateTimeAsMsSinceUnixEpochInUtc || msgDateTimeAsMsSinceUnixEpochInUtc == files.size() ) &&
                               ( 0 == msgPrimaryPayloadIncluded || msgPrimaryPayloadIncluded == files.size() ) &&
                               ( 0 == msgKeyValuePairsIncluded || msgKeyValuePairsIncluded == files.size() ) &&
                               ( 0 == msgMetaDataKeyValuePairsIncluded || msgMetaDataKeyValuePairsIncluded == files.size() ) &&
                               ( 0 == topicNameIncluded || topicNameIncluded == files.size() );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:DetermineStorageOptions: Discovered common option mask across " + 
        CORE::ToString( files.size() ) + " files " + CORE::ToString( atLeastOneOptions.ToOptionsBitMask() ) + " and the file set homogeneity is " + CORE::ToString( sourceFilesAreHomogenous ) );

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
PubSubStorageTool::CreateTargetContainerAtTimeBoundary( const CORE::CString& targetRootPath                                , 
                                                        CORE::CDateTime::TDateTimeComponent timeBoundary                   ,
                                                        const CORE::CDateTime& datetime                                    , 
                                                        const PUBSUB::CPubSubMsgBinarySerializerOptions& atLeastOneOptions ,
                                                        CTimeBoundContainerPtr& container                                  )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Will attempt to create container at root \"" + targetRootPath + "\" with time boundary datetime " + 
        CORE::ToString( datetime ) );

    if ( timeBoundary == CORE::CDateTime::DATETIMECOMPONENT_UNKNOWN )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Unknown time boundary: " + timeBoundary );
        return false;
    }

    container = CTimeBoundContainer::CreateSharedObj();
    if ( container.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Failed to allocate container obj memory" );
        return false;
    }
    
    if ( !datetime.ClampToBoundary( timeBoundary, BOUNDARYTYPE_MINIMUM, container->minClampedDt ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Cannot clamp dt to minimum boundary" );
        return false;
    }
    if ( !datetime.ClampToBoundary( timeBoundary, BOUNDARYTYPE_MAXIMUM, container->maxClampedDt ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Cannot clamp dt to maximum boundary" );
        return false;
    }

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Clamped time boundary datetime to " + CORE::ToString( datetime ) );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    CORE::CString tempDir;
    if ( vfs.TryResolveSpecialDir( CORE::SPECIALDIR_TEMP_DIR, tempDir ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: VFS resolved temp dir to: " + tempDir );
    }
    else
    {
        tempDir = targetRootPath;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: VFS failed to resolve temp dir. Will use target dir as the temp dir: " + tempDir );
    }
    
    container->tempContainerPath = tempDir + "/" + container->minClampedDt.ToIso8601DateTimeString( false, true ) + "_" + container->maxClampedDt.ToIso8601DateTimeString( false, true ) + ".tmp";

    container->resource = vfs.GetFile( container->tempContainerPath, "wb", true );
    if ( container->resource.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Failed to create temp container file at " + container->tempContainerPath );
        return false;
    }
    CORE::IOAccessPtr access = container->resource->GetAccess();
    if ( access.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Failed to gain IO access to temp container file at " + container->tempContainerPath );
        return false;
    }

    // Since its a new file we should write the initial header
    UInt32 bytesWritten = 0;
    if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::SerializeHeader( atLeastOneOptions, *access, bytesWritten ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Failed to write header to temp container file at " + container->tempContainerPath );
        return false;
    }

    container->creationDt = CORE::CDateTime::NowUTCDateTime();
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:CreateTargetContainerAtTimeBoundary: Successfully prepared target temporary container at: " + container->tempContainerPath );
    
    return true;
}

/*-------------------------------------------------------------------------*/

bool
PubSubStorageTool::GetOrCreateTargetContainerAtTimeBoundary( TDateTimeToTimeBoundContainerMap& allContainers                    ,
                                                             const CORE::CString& targetRootPath                                , 
                                                             CORE::CDateTime::TDateTimeComponent timeBoundary                   ,
                                                             const CORE::CDateTime& datetime                                    , 
                                                             const PUBSUB::CPubSubMsgBinarySerializerOptions& atLeastOneOptions ,
                                                             CTimeBoundContainerPtr& container                                  )
{GUCEF_TRACE;

    CORE::CDateTime minClampedDt;
    if ( !datetime.ClampToBoundary( timeBoundary, BOUNDARYTYPE_MINIMUM, minClampedDt ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:GetOrCreateTargetContainerAtTimeBoundary: Cannot clamp dt to minimum boundary" );
        return false;
    }

    TDateTimeToTimeBoundContainerMap::iterator i = allContainers.find( minClampedDt );
    if ( i != allContainers.end() )
    {
        // Container exists
        container = i->second;
        return true;
    }

    // Container does not exist, create it
    if ( CreateTargetContainerAtTimeBoundary( targetRootPath    , 
                                              timeBoundary      ,
                                              datetime          , 
                                              atLeastOneOptions ,
                                              container         ) )
    {
        allContainers[ minClampedDt ] = container;
        return true;
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
PubSubStorageTool::PerformReSplit( void )
{GUCEF_TRACE;

    CORE::CString namePattern = m_params.GetValueAlways( "namePattern", "*.pubsubcapture" );    
    CORE::CString fsPath = m_params.GetValueAlways( "path" );
    CORE::CString vfsPath = m_params.GetValueAlways( "vfsPath" );

    if ( !fsPath.IsNULLOrEmpty() )
    {
        // If we have a relative path lets make it available to the VFS so that 
        // we can use the same VFS based code for everything
        if ( !AddLocalPathAsVfsRoot( fsPath, vfsPath ) )
        {
            return false;
        }
    }

    if ( fsPath.IsNULLOrEmpty() && vfsPath.IsNULLOrEmpty() )
    {
        // Use the current working directory as a fallback
        fsPath = CORE::ResolveVars( "$CURWORKDIR$" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: No input path specified, will try to use the current working directory" );

        // If we have a relative path lets make it available to the VFS so that 
        // we can use the same VFS based code for everything
        if ( !AddLocalPathAsVfsRoot( fsPath, vfsPath ) )
        {
            return false;
        }
    }

    if ( fsPath.IsNULLOrEmpty() && vfsPath.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: No input path specified and failed to use the current working directory" );
        return false;
    }
    
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    CORE::CString::StringVector files;
    if ( !vfs.GetFileList( files, vfsPath, false, true, namePattern, GUCEF_UINT32MAX ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to get file list from VFS" );
        return false;    
    }

    if ( files.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: No files found in VFS" );
        return false;
    }
    CORE::CString fileExtension = CORE::ExtractFileExtention( files[0] );

    bool sourceFilesAreHomogenous = true;
    PUBSUB::CPubSubMsgBinarySerializerOptions atLeastOneOptions;
    PUBSUB::CPubSubMsgBinarySerializerOptions unanimousOptions;
    if ( !DetermineStorageOptions( files, atLeastOneOptions, unanimousOptions, sourceFilesAreHomogenous ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to determine common container storage options from source files" );
        return false;
    }

    CORE::CString resplitType = m_params.GetValueAlways( "resplitType", "time" );
    if ( "time" == resplitType )
    {
        if ( !unanimousOptions.msgDateTimeIncluded )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Messages in all containers MUST have a DateTime to use this functionality, these files dont" );
            return false;
        }        

        UInt64 largestFileSize = 0;
        if ( !DetermineLargestFileSize( files, largestFileSize ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to determine largest file size from source files" );
            return false;
        }

        CORE::CString timeBoundaryStr = m_params.GetValueAlways( "timeBoundary", "hour" );
        bool bestEffortIsOk = m_params.GetValueAlways( "bestEffortIsOk" ).AsBool( true, true );
        CORE::CDateTime::TDateTimeComponent timeBoundaryValue = CORE::CDateTime::ToDateTimeComponentType( timeBoundaryStr );
        CORE::CString tempRootPath = m_params.GetValueAlways( "tempRootPath" ).AsString( "$TEMP$", true );
        CORE::CString targetRootPath = m_params.GetValueAlways( "targetRootPath" ).AsString( vfsPath, true );
        bool deleteSourceAfterSuccess = m_params.GetValueAlways( "deleteSourceAfterSuccess" ).AsBool( false, true );

        TDateTimeToTimeBoundContainerMap allTargetContainers;
        CORE::CDynamicBuffer containerBuffer( (UInt32)largestFileSize, true );
        PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector msgs;        
        PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex srcMsgIndex;
        TDateTimeMsgRefMap boundaryMsgRefs;

        for ( CORE::CString::StringVector::iterator it = files.begin(); it != files.end(); ++it )
        {
            containerBuffer.Clear( true );
            msgs.clear();
            boundaryMsgRefs.clear();
            srcMsgIndex.clear();
            
            const CORE::CString& filePath = (*it);

            if ( !vfs.LoadFile( containerBuffer, filePath ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to load container content: " + filePath );
                return false;                
            }

            UInt32 bytesRead = 0;
            if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeFooter( srcMsgIndex, containerBuffer, bytesRead, true ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to deserialize container footer: " + filePath );
                return false;                
            }

            bool isCorrupted = false;
            if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::Deserialize( msgs, true, srcMsgIndex, containerBuffer, isCorrupted, bestEffortIsOk ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to deserialize container content: " + filePath );
                return false;                
            }

            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:PerformReSplit: Source file \"" + filePath + "\" has " + 
                CORE::ToString( srcMsgIndex.size() ) + " entries. Will attempt to sort them now" );

            // First sort all the messages based on the time boundary to reduce lookup time for target containers
            for ( PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector::iterator msgIt = msgs.begin(); msgIt != msgs.end(); ++msgIt )
            {
                PUBSUB::CBasicPubSubMsg& msg = (*msgIt);
                const CORE::CDateTime& msgDt = msg.GetMsgDateTime();

                CORE::CDateTime minClampedDt;
                if ( !msgDt.ClampToBoundary( timeBoundaryValue, BOUNDARYTYPE_MINIMUM, minClampedDt ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Cannot clamp dt to minimum boundary" );
                    return false;
                }

                boundaryMsgRefs[ minClampedDt ].push_back( &msg );
            }   

            // Now we can start creating the target containers as needed and writing the messages in the desired container
            for ( TDateTimeMsgRefMap::iterator boundaryIt = boundaryMsgRefs.begin(); boundaryIt != boundaryMsgRefs.end(); ++boundaryIt )
            {
                const CORE::CDateTime& boundaryDt = boundaryIt->first;
                PUBSUB::CPubSubClientTopic::TPubSubMsgsRefVector& boundaryMsgs = boundaryIt->second;

                CTimeBoundContainerPtr targetContainer;                
                if ( !GetOrCreateTargetContainerAtTimeBoundary( allTargetContainers, tempRootPath, timeBoundaryValue, boundaryDt, atLeastOneOptions, targetContainer ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to get or create target container for message with DateTime " + boundaryDt.ToIso8601DateTimeString( true, true ) );
                    return false;
                }
                CORE::IOAccessPtr targetAccess = targetContainer->resource->GetAccess();
                if ( targetAccess.IsNULL() )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to get IO access to target container for message with DateTime " + boundaryDt.ToIso8601DateTimeString( true, true ) );
                    return false;
                }
                PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex& targetIndex = targetContainer->index;

                // Note that we defer writing a footer since we are combining collections from multiple files here
                // footer writing comes after all files are successfully written to the target files
                UInt32 bytesWritten = 0;
                for ( PUBSUB::CPubSubClientTopic::TPubSubMsgsRefVector::const_iterator msgIt = boundaryMsgs.begin(); msgIt != boundaryMsgs.end(); ++msgIt )
                {
                    const PUBSUB::CIPubSubMsg* msg = (*msgIt);
                    if ( GUCEF_NULL != msg )
                    {
                        UInt64 offset = targetAccess->Tell();
                        if ( !PUBSUB::CPubSubMsgBinarySerializer::Serialize( atLeastOneOptions, *msg, *targetAccess, bytesWritten ) )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to serialize message" );
                            return false;
                        }
                        targetIndex.push_back( (UInt32) offset );

                        CORE::CDateTime msgDt = msg->GetMsgDateTime();
                        if ( msgDt < targetContainer->earliestMsgDt )
                            targetContainer->earliestMsgDt = msgDt;
                        if ( msgDt > targetContainer->newestMsgDt )
                            targetContainer->newestMsgDt = msgDt;
                    }
                }
            }

            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:PerformReSplit: Finished writing content from source file \"" + filePath + "\" with " + 
                CORE::ToString( srcMsgIndex.size() ) + " entries into the temporary resplit target files" );
         }

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:PerformReSplit: Finished writing data from all " + CORE::ToString( files.size() ) +
            " source files to the temporary resplit target files" );

         // Now that we have all the messages written to new containers, combining data from all the files, we can write the footers
         // Also note that the current output files are still just temporary files with temporary names
         bool allSuccess = true;
         TDateTimeToTimeBoundContainerMap::iterator m = allTargetContainers.begin();
         while ( m != allTargetContainers.end() )
         {
            CTimeBoundContainerPtr targetContainer = m->second;
            if ( targetContainer.IsNULL() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Target container is NULL" );
                return false;
            }
            CORE::IOAccessPtr targetAccess = targetContainer->resource->GetAccess();
            if ( targetAccess.IsNULL() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to get IO access to target container for message with DateTime " + targetContainer->minClampedDt.ToIso8601DateTimeString( true, true ) );
                return false;
            }
            PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex& targetIndex = targetContainer->index;
    
            UInt32 bytesWritten = 0;
            if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::SerializeFooter( targetIndex, *targetAccess, bytesWritten ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to write footer to target container for message with DateTime " + targetContainer->minClampedDt.ToIso8601DateTimeString( true, true ) );
                return false;
            }
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:PerformReSplit: Finished writing footer with " + CORE::ToString( targetIndex.size() ) +
                " entries into the temporary resplit target files" );

            // Now that we have added the footer the container has been completed
            // We can now rename the file to its final name. To do so we need to close the file first to ensure everything is written
            targetContainer->resource->GetAccess()->Close();
            targetContainer->resource.Unlink();
                
            CORE::CString finalPath = targetRootPath + "/" + 
                targetContainer->earliestMsgDt.ToIso8601DateTimeString( false, true ) + "_" + 
                targetContainer->newestMsgDt.ToIso8601DateTimeString( false, true ) + "_" + 
                targetContainer->creationDt.ToIso8601DateTimeString( false, true ) + 
                ".v" + CORE::ToString( PUBSUB::CPubSubMsgContainerBinarySerializer::CurrentFormatVersion ) +
                "." + fileExtension;
    
            if ( vfs.MoveFile( targetContainer->tempContainerPath, finalPath, false ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:PerformReSplit: Successfully moved temp container file + \"" + targetContainer->tempContainerPath + 
                    "\" to final path \"" + finalPath + "\"" );
            }
            else
            {
                allSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to move temp container file + \"" + targetContainer->tempContainerPath + 
                    "\" to final path \"" + finalPath + "\"");
            }     
                
            ++m;
         }

        if ( allSuccess && deleteSourceAfterSuccess )
        {
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:PerformReSplit: Per config we will now delete the source files since we successfully performed the resplit operation" );            
            
            for ( CORE::CString::StringVector::iterator it = files.begin(); it != files.end(); ++it )
            {
                const CORE::CString& filePath = (*it);
                if ( vfs.DeleteFile( filePath, false ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:PerformReSplit: Deleted source file: " + filePath );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformReSplit: Failed to delete source file: " + filePath );
                }
            }
        }

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubStorageTool:PerformReSplit: Completed the the resplit operation for " + CORE::ToString( files.size() ) + 
            " source files. Full success = " + CORE::ToString( allSuccess ) );
        return allSuccess;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
PubSubStorageTool::PerformWork( void )
{GUCEF_TRACE;
       
    try
    {
        CORE::CString taskName = m_params.GetValueAlways( "task" );
        if ( "resplit" == taskName )
        {
            bool result = PerformReSplit();
            CORE::CCoreGlobal::Instance()->GetApplication().Stop( true );
            return result;
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "PubSubStorageTool:PerformWork: Exception caught: " + CORE::CString( e.what() ) );
        CORE::CCoreGlobal::Instance()->GetApplication().Stop( true );
        return false;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "No known task specified, need param 'task' to reference a known task" );
    CORE::CCoreGlobal::Instance()->GetApplication().Stop( true );
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
