/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDE                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"                /* Needed for dir itteration */
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define DVCPPSTRINGUTILS_H
#endif /* DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef CMFILEACCESS_H
#include "CMFileAccess.h"
#define CMFILEACCESS_H
#endif /* CMFILEACCESS_H ? */

#ifndef CFILEACCESS_H
#include "CFileAccess.h"
#define CFILEACCESS_H
#endif /* CFILEACCESS_H ? */

#ifndef CDATANODE_H
#include "CDataNode.h"          /* node for data storage */
#define CDATANODE_H
#endif /* CDATANODE_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_VFS_CASYNCVFSOPERATION_H
#include "gucefVFS_CAsyncVfsOperation.h"
#define GUCEF_VFS_CASYNCVFSOPERATION_H
#endif /* GUCEF_VFS_CASYNCVFSOPERATION_H ? */

#include "gucefVFS_CVFS.h"           /* definition of the file implemented here */

#ifdef ACTIVATE_MEMORY_MANAGER
  #ifndef GUCEF_NEW_ON_H
  #include "gucef_new_on.h"   /* Use the GUCEF memory manager instead of the standard manager ? */
  #define GUCEF_NEW_ON_H
  #endif /* GUCEF_NEW_ON_H ? */
#endif /* ACTIVATE_MEMORY_MANAGER ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CVFS::AsyncVfsOperationCompletedEvent = "GUCEF::VFS::CVFS::AsyncVfsOperationCompletedEvent";
const CORE::CEvent CVFS::ArchiveMountedEvent = "GUCEF::VFS::CVFS::ArchiveMountedEvent";
const CORE::CEvent CVFS::ArchiveUnmountedEvent = "GUCEF::VFS::CVFS::ArchiveUnmountedEvent";
const CORE::CEvent CVFS::DelayedArchiveMountingCompletedEvent = "GUCEF::VFS::CVFS::DelayedArchiveMountingCompletedEvent";
const CORE::CEvent CVFS::VfsInitializationCompletedEvent = "GUCEF::VFS::CVFS::VfsInitializationCompletedEvent";

const CORE::CString CVFS::FileSystemArchiveTypeName = "FileSystem";
const CORE::CString CVFS::DefaultASyncOpsThreadPoolName = "VFS";

#define GUCEF_VFS_RESERVED_STRING_DEFAULT_VOLUME_MOUNT_ROOT     "/system/"
#define GUCEF_VFS_RESERVED_STRING_VOLUMES_BY_ID                 "volumes_by_id/"
#define GUCEF_VFS_RESERVED_STRING_VOLUMES_BY_PATH               "volumes_by_path/"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CVFS::RegisterEvents( void )
{GUCEF_TRACE;

    AsyncVfsOperationCompletedEvent.Initialize();
    ArchiveMountedEvent.Initialize();
    ArchiveUnmountedEvent.Initialize();
    DelayedArchiveMountingCompletedEvent.Initialize();
    VfsInitializationCompletedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CVFS::CVFS( void )
    : CORE::CTSGNotifier()
    , CORE::CGloballyConfigurable( false )
    , CORE::CIDirectoryWatcher()
    , m_mountList()
    , m_maxmemloadsize( 1024 )
    , m_asyncOpsThreadpool( DefaultASyncOpsThreadPoolName )
    , m_asyncOpsMinWorkerThreads( 0 )
    , m_abstractArchiveFactory()
    , m_fileSystemArchiveFactory()
    , m_delayMountedArchiveSettings()
    , m_delayedArchiveMountingIsComplete( false )
    , m_vfsRootForVolumes( GUCEF_VFS_RESERVED_STRING_DEFAULT_VOLUME_MOUNT_ROOT )
    , m_vfsVolumesAreWriteableIfPossible( true )
    , m_autoMountFsVolumes( true )
    , m_rwdataLock( true )
{GUCEF_TRACE;

    RegisterEvents();
    RegisterEventHandlers();
    RegisterArchiveFactory( FileSystemArchiveTypeName, m_fileSystemArchiveFactory );
}

/*-------------------------------------------------------------------------*/

CVFS::~CVFS()
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwdataLock );

    UnmountAllArchives();
    UnregisterAllArchiveFactories();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CVFS::RegisterEventHandlers( void )
{GUCEF_TRACE;

    // Success or fail we use the config load finish event messages to trigger delayed mounting
    TEventCallback callback( this, &CVFS::OnGlobalConfigLoadFinished );
    SubscribeTo( &CORE::CCoreGlobal::Instance()->GetConfigStore()   ,
                 CORE::CConfigStore::GlobalConfigLoadCompletedEvent ,
                 callback                                           );
    TEventCallback callback2( this, &CVFS::OnGlobalConfigLoadFinished );
    SubscribeTo( &CORE::CCoreGlobal::Instance()->GetConfigStore() ,
                 CORE::CConfigStore::GlobalConfigLoadFailedEvent  ,
                 callback2                                        );
    TEventCallback callback3( this, &CVFS::OnSomeShutdownEvent );
    SubscribeTo( &CORE::CCoreGlobal::Instance()->GetApplication()  ,
                 CORE::CGUCEFApplication::AppShutdownCompleteEvent ,
                 callback3                                         );
    TEventCallback callback4( this, &CVFS::OnSomeShutdownEvent );
    SubscribeTo( &CORE::CCoreGlobal::Instance()->GetPluginControl()       ,
                 CORE::CPluginControl::UnregisterOfAllPluginsStartedEvent ,
                 callback4                                                );
}

/*-------------------------------------------------------------------------*/

void
CVFS::UnmountAllArchives( void )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwdataLock );

    while ( !m_mountList.empty() )
    {
        TMountEntryPtr mountEntry = (*m_mountList.begin());
        TArchiveUnmountedEventData eData( mountEntry->mountPath );
        if ( mountEntry->archive->UnloadArchive() )
        {
            m_archivePtrToMountEntryLookup.erase( mountEntry->archive.GetPointerAlways() );
            m_mountList.erase( m_mountList.begin() );
            NotifyObservers( ArchiveUnmountedEvent, &eData );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CVFS::UnregisterAllArchiveFactories( void )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwdataLock );
    m_abstractArchiveFactory.UnregisterAllConcreteFactories();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVFS::StoreAsFileAsync( const CORE::CString& filepath       ,
                        const CORE::CDynamicBuffer& data    ,
                        const CORE::UInt64 offset           ,
                        const bool overwrite                ,
                        CORE::CICloneable* requestorData    ,
                        const CORE::CString& asyncRequestId )
{GUCEF_TRACE;

    try
    {
        CStoreAsFileTaskData operationData;
        operationData.operationType = ASYNCVFSOPERATIONTYPE_STOREDATAASFILE;
        operationData.asyncRequestId = asyncRequestId;
        operationData.filepath = filepath;
        operationData.data.LinkTo( data );
        operationData.offset = offset;
        operationData.overwrite = overwrite;
        operationData.SetRequestorData( requestorData );

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_asyncOpsThreadpool );
        return threadPool->QueueTask( CAsyncVfsOperation::TaskType, &operationData, GUCEF_NULL, &AsObserver() );
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:StoreAsFileAsync: Failed to queue task due to timeout. asyncRequestId=" +
            asyncRequestId );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:StoreAsFileAsync: Failed to queue task due to exception. asyncRequestId=" +
            asyncRequestId + " what=" + CORE::ToString( e.what() ) );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVFS::MountArchiveAsync( const CArchiveSettings& settings    ,
                         CORE::CICloneable* requestorData    ,
                         const CORE::CString& asyncRequestId )
{GUCEF_TRACE;

    try
    {
        CMountArchiveTaskData operationData;
        operationData.operationType = ASYNCVFSOPERATIONTYPE_MOUNTARCHIVE;
        operationData.asyncRequestId = asyncRequestId;
        operationData.settings = settings;
        operationData.SetRequestorData( requestorData );

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_asyncOpsThreadpool );
        return threadPool->QueueTask( CAsyncVfsOperation::TaskType, &operationData, GUCEF_NULL, &AsObserver() );
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MountArchiveAsync: Failed to queue task due to timeout. asyncRequestId=" +
            asyncRequestId );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:MountArchiveAsync: Failed to queue task due to exception. asyncRequestId=" +
            asyncRequestId + " what=" + CORE::ToString( e.what() ) );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

void
CVFS::OnArchiveDirectoryWatcherEvent( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventid  ,
                                      CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    VFS::CArchive* archive = static_cast< VFS::CArchive* >( notifier );

    if ( IsDirectoryWatcherDirEvent( eventid ) )
    {
        MT::CScopeReaderLock lock( m_rwdataLock );

        TArchivePtrToMountEntryMap::iterator i = m_archivePtrToMountEntryLookup.find( archive );
        if ( i != m_archivePtrToMountEntryLookup.end() )
        {
            TMountEntryPtr mountEntry = (*i).second;

            if ( eventid == CORE::CDirectoryWatcherEvents::DirRenamedEvent )
            {
                CORE::CDirectoryWatcherEvents::TDirRenamedEventData* dirRenameInfo = static_cast< CORE::CDirectoryWatcherEvents::TDirRenamedEventData* >( eventdata );
                if ( GUCEF_NULL != dirRenameInfo )
                {
                    CORE::CDirectoryWatcherEvents::TDirRenamedEventData vfsAdjusted;
                    vfsAdjusted.GetData().newDirName = mountEntry->mountPath + '/' + dirRenameInfo->GetData().newDirName;
                    vfsAdjusted.GetData().oldDirName = mountEntry->mountPath + '/' + dirRenameInfo->GetData().oldDirName;
                    vfsAdjusted.GetData().newDirName = ConformVfsDirPath( vfsAdjusted.GetData().newDirName );
                    vfsAdjusted.GetData().oldDirName = ConformVfsDirPath( vfsAdjusted.GetData().oldDirName );
                    NotifyObservers( eventid, &vfsAdjusted );
                }
            }
            else
            {
                CORE::TCloneableString* dirPath = static_cast< CORE::TCloneableString* >( eventdata );
                if ( GUCEF_NULL != dirPath )
                {
                    CORE::TCloneableString vfsPath( mountEntry->mountPath + '/' + *dirPath );
                    vfsPath = ConformVfsDirPath( vfsPath );
                    NotifyObservers( eventid, &vfsPath );
                }
            }
        }
    }
    else
    if ( IsDirectoryWatcherFileEvent( eventid ) )
    {
        MT::CScopeReaderLock lock( m_rwdataLock );

        TArchivePtrToMountEntryMap::iterator i = m_archivePtrToMountEntryLookup.find( archive );
        if ( i != m_archivePtrToMountEntryLookup.end() )
        {
            TMountEntryPtr mountEntry = (*i).second;

            if ( eventid == CORE::CDirectoryWatcherEvents::FileRenamedEvent )
            {
                CORE::CDirectoryWatcherEvents::TFileRenamedEventData* fileRenameInfo = static_cast< CORE::CDirectoryWatcherEvents::TFileRenamedEventData* >( eventdata );
                if ( GUCEF_NULL != fileRenameInfo )
                {
                    CORE::CDirectoryWatcherEvents::TFileRenamedEventData vfsAdjusted;
                    vfsAdjusted.GetData().newFilename = mountEntry->mountPath + '/' + fileRenameInfo->GetData().newFilename;
                    vfsAdjusted.GetData().oldFilename = mountEntry->mountPath + '/' + fileRenameInfo->GetData().oldFilename;
                    vfsAdjusted.GetData().newFilename = ConformVfsFilePath( vfsAdjusted.GetData().newFilename );
                    vfsAdjusted.GetData().oldFilename = ConformVfsFilePath( vfsAdjusted.GetData().oldFilename );
                    NotifyObservers( eventid, &vfsAdjusted );
                }
            }
            else
            {
                CORE::TCloneableString* filePath = static_cast< CORE::TCloneableString* >( eventdata );
                if ( GUCEF_NULL != filePath )
                {
                    CORE::TCloneableString vfsPath( mountEntry->mountPath + '/' + *filePath );
                    vfsPath = ConformVfsFilePath( vfsPath );
                    NotifyObservers( eventid, &vfsPath );
                }
            }
        }
    }
    else
    {
        // adminstrative event
        NotifyObservers( eventid );
    }
}

/*-------------------------------------------------------------------------*/

void
CVFS::OnPumpedNotify( CORE::CNotifier* notifier    ,
                      const CORE::CEvent& eventid  ,
                      CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CAsyncVfsOperation::AsyncVfsOperationCompletedEvent == eventid )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "VFS:OnPumpedNotify: Async operation completed, passing on event notification" );
        NotifyObservers( AsyncVfsOperationCompletedEvent, eventdata );
    }

    if ( IsDirectoryWatcherEvent( eventid ) )
    {
        OnArchiveDirectoryWatcherEvent( notifier  ,
                                        eventid   ,
                                        eventdata );
    }
}

/*-------------------------------------------------------------------------*/

bool
CVFS::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_delayedArchiveMountingIsComplete;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    bool isConnectedOverall = true;
    TMountVector::const_iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr mountEntry = (*i);
        if ( !mountEntry->archive->IsConnected() )
        {
            isConnectedOverall = false;
            break;
        }
        ++i;
    }

    return isConnectedOverall;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::IsHealthy( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    bool isHealthyOverall = true;
    TMountVector::const_iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr mountEntry = (*i);
        if ( !mountEntry->archive->IsHealthy() )
        {
            isHealthyOverall = false;
            break;
        }
        ++i;
    }

    return isHealthyOverall;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::IsDelayedArchiveMountingCompleted( void ) const
{GUCEF_TRACE;

    return m_delayedArchiveMountingIsComplete;
}

/*-------------------------------------------------------------------------*/

void
CVFS::MountAllDelayMountedArchives( void )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwdataLock );

    while ( !m_delayMountedArchiveSettings.empty() )
    {
        TArchiveSettingsVector::iterator i = m_delayMountedArchiveSettings.begin();
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CVFS:MountAllDelayMountedArchives: Commencing delayed mounting of archive with name: " + (*i).GetArchiveName() );
        if ( MountArchive( (*i) ) )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CVFS:MountAllDelayMountedArchives: Succeeded in delayed mounting of archive with name: " + (*i).GetArchiveName() );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CVFS:MountAllDelayMountedArchives: Failed delayed mounting of archive with name: " + (*i).GetArchiveName() );
        }
        m_delayMountedArchiveSettings.erase( i );
    }
    m_delayedArchiveMountingIsComplete = true;

    lock.EarlyUnlock();

    NotifyObservers( DelayedArchiveMountingCompletedEvent );
}

/*-------------------------------------------------------------------------*/

void
CVFS::OnGlobalConfigLoadFinished( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventid  ,
                                  CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    MountAllDelayMountedArchives();

    NotifyObservers( VfsInitializationCompletedEvent );
}

/*-------------------------------------------------------------------------*/

void
CVFS::OnSomeShutdownEvent( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventid  ,
                           CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwdataLock );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS: Triggering cleanup due to shutdown event" );
    UnmountAllArchives();
    UnregisterAllArchiveFactories();
}

/*-------------------------------------------------------------------------*/

bool
CVFS::DeleteFile( const CString& filePath, bool okIfItDoesNotExist )
{GUCEF_TRACE;

    // @TODO: This functionality could be improved by taking into account outstanding references to the resource
    //        Not all archive types might be able to handle denying a delete themselves during ongoing I/O especially with remote storage

    CString path = ConformVfsFilePath( filePath );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Search for a file match and attempt to delete if a match exists
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        TArchivePtr archive = mountLink->mountEntry->archive;
        if ( archive->FileExists( mountLink->remainder ) )
        {
            return archive->DeleteFile( mountLink->remainder );
        }
        ++i;
    }

    // No such file found
    return okIfItDoesNotExist;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::MoveFile( const CString& oldFilePath ,
                const CString& newFilePath ,
                const bool overwrite       )
{GUCEF_TRACE;

    CString oldPath = ConformVfsFilePath( oldFilePath );
    CString newPath = ConformVfsFilePath( newFilePath );

    if ( oldPath == newPath )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MoveFile: old and new path are the same. No action taken. Path= " + oldPath );
        return true;
    }

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get lists of all eligable mounts
    TConstMountLinkVector oldPathMountLinks;
    GetEligableMounts( oldPath, false, oldPathMountLinks );
    TConstMountLinkVector newPathMountLinks;
    GetEligableMounts( newPath, true, newPathMountLinks );

    TConstMountLinkVector::iterator i = oldPathMountLinks.begin();
    while ( i != oldPathMountLinks.end() )
    {
        TConstMountLinkPtr& oldMountLink = (*i);
        TConstMountLinkVector::iterator n = newPathMountLinks.begin();
        while ( n != newPathMountLinks.end() )
        {
            TConstMountLinkPtr& newMountLink = (*n);
            if ( oldMountLink->mountEntry == newMountLink->mountEntry )
            {
                // Found a match where both old and new path are available via the same archive
                // this takes priority over any cross-archive logical 'moves'
                TArchivePtr archive = newMountLink->mountEntry->archive;
                if ( archive->MoveFile( oldMountLink->remainder ,
                                        newMountLink->remainder ,
                                        overwrite               ) )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MoveFile: File moved from \"" +
                        oldFilePath + "\" to \"" + newFilePath + "\" with overwrite=" + CORE::ToString( overwrite ) +
                        " using archive \"" + archive->GetArchiveName() + "\" of type " + archive->GetType() );
                    return true;
                }
                else
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MoveFile: Failed to move file from \"" +
                        oldFilePath + "\" to \"" + newFilePath + "\" with overwrite=" + CORE::ToString( overwrite ) +
                        " using archive \"" + archive->GetArchiveName() + "\" of type " + archive->GetType() );
                }
            }
            ++n;
        }
        ++i;
    }

    // Since we could not find an archive that can hold both the old and new location we will have to
    // perform a logical move which is a copy followed by a delete

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MoveFile: Unable to resolve using same-archive MoveFile operation, falling back to Copy/Delete" );

    if ( CopyFile( oldFilePath ,
                   newFilePath ,
                   overwrite   ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MoveFile: File copied from \"" + oldFilePath + "\" to \"" + newFilePath + "\" with overwrite=" + CORE::ToString( overwrite ) );
        if ( DeleteFile( oldFilePath, false ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MoveFile: Copied file at old location \"" + oldFilePath + "\" is deleted" );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:MoveFile: Failed to delete copied file at old location \"" + oldFilePath + "\". File may now exist in both locations!" );
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVFS::MoveFileAsync( const CORE::CString& oldFilePath    ,
                     const CORE::CString& newFilePath    ,
                     const bool overwrite                ,
                     const CORE::CString& asyncRequestId )
{GUCEF_TRACE;

    try
    {
        CMoveFileTaskData operationData;
        operationData.operationType = ASYNCVFSOPERATIONTYPE_MOVEFILE;
        operationData.asyncRequestId = asyncRequestId;
        operationData.originalFilepath = oldFilePath;
        operationData.newFilepath = newFilePath;
        operationData.overwrite = overwrite;

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_asyncOpsThreadpool );
        return threadPool->QueueTask( CAsyncVfsOperation::TaskType, &operationData, GUCEF_NULL, &AsObserver() );
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MoveFileAsync: Failed to queue task due to timeout. asyncRequestId=" +
            asyncRequestId );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:MoveFileAsync: Failed to queue task due to exception. asyncRequestId=" +
            asyncRequestId + " what=" + CORE::ToString( e.what() ) );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::CopyFileContent( const CORE::CString& originalFilepath ,
                       const CORE::CString& copyFilepath     ,
                       const bool overwrite                  )
{GUCEF_TRACE;

    if ( originalFilepath == copyFilepath )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFileContent: original and target path cannot be the same: " + originalFilepath );
        return false;
    }

    TBasicVfsResourcePtr originalFile = GetFile( originalFilepath, "rb", overwrite );
    if ( originalFile.IsNULL() || originalFile->GetAccess().IsNULL() || !originalFile->GetAccess()->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFileContent: Cannot obtain original file: " + originalFilepath );
        return false;
    }
    UInt64 origSize = originalFile->GetAccess()->GetSize();

    TBasicVfsResourcePtr targetFile = GetFile( copyFilepath, "wb", overwrite );
    if ( targetFile.IsNULL() || targetFile->GetAccess().IsNULL() || !targetFile->GetAccess()->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFileContent: Cannot obtain access to output file: " + copyFilepath );
        return false;
    }

    // Perform the actual byte copy synchronously and check if the expected size matches the actual size
    UInt64 bytesWritten = targetFile->GetAccess()->Write( *originalFile->GetAccess() );
    if ( origSize != bytesWritten )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFileContent: Failed to fully copy file \"" + originalFilepath +
            "\" to \"" + copyFilepath + "\". Bytes written (" + CORE::ToString( bytesWritten ) +
            ") does not match expected size (" + CORE::ToString( origSize ) );

        targetFile.Unlink();

        // We should not leave a corrupt file behind
        if ( !DeleteFile( copyFilepath, false ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:CopyFileContent: Failed to delete partially copied file at \"" + copyFilepath + "\". Corrupt file may now exist in target location, please check!" );
        }
        return false;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFileContent: Successfully copied file \"" + originalFilepath +
            "\" to \"" + copyFilepath + "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVFS::CopyFileContentAsync( const CORE::CString& originalFilepath ,
                            const CORE::CString& copyFilepath     ,
                            const bool overwrite                  ,
                            const CORE::CString& asyncRequestId   )
{GUCEF_TRACE;

    try
    {
        CCopyFileContentTaskData operationData;
        operationData.operationType = ASYNCVFSOPERATIONTYPE_COPYFILECONTENT;
        operationData.asyncRequestId = asyncRequestId;
        operationData.originalFilepath = originalFilepath;
        operationData.copyFilepath = copyFilepath;
        operationData.overwrite = overwrite;

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_asyncOpsThreadpool );
        return threadPool->QueueTask( CAsyncVfsOperation::TaskType, &operationData, GUCEF_NULL, &AsObserver() );
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFileContentAsync: Failed to queue task due to timeout. asyncRequestId=" +
            asyncRequestId );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:CopyFileContentAsync: Failed to queue task due to exception. asyncRequestId=" +
            asyncRequestId + " what=" + CORE::ToString( e.what() ) );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::CopyFile( const CORE::CString& originalFilepath ,
                const CORE::CString& copyFilepath     ,
                const bool overwrite                  )
{GUCEF_TRACE;

    if ( originalFilepath == copyFilepath )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFile: original and target path cannot be the same: " + originalFilepath );
        return false;
    }

    CORE::CResourceMetaData metaData;
    if ( !GetFileMetaData( originalFilepath ,
                           metaData         ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFile: Cannot obtain meta-data on original file: " + originalFilepath );
        return false;
    }

    TBasicVfsResourcePtr originalFile = GetFile( originalFilepath, "rb", overwrite );
    if ( originalFile.IsNULL() || originalFile->GetAccess().IsNULL() || !originalFile->GetAccess()->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFile: Cannot obtain original file: " + originalFilepath );
        return false;
    }

    // Applying meta-data when obtaining the target file is ideal since it avoids the possibility of race conditions
    // where the file is created without the meta-data and then the meta-data is applied afterwards
    // However not all backend implementations support this so we fall back to applying the meta-data afterwards
    TBasicVfsResourcePtr targetFile = GetFileAs( copyFilepath, metaData, "wb", overwrite );
    if ( targetFile.IsNULL() || targetFile->GetAccess().IsNULL() || !targetFile->GetAccess()->IsValid() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFile: Cannot obtain access to output file using source metadata as a constraint: " + copyFilepath );

        // Try again without the meta-data constraint
        targetFile = GetFile( copyFilepath, "wb", overwrite );
        if ( targetFile.IsNULL() || targetFile->GetAccess().IsNULL() || !targetFile->GetAccess()->IsValid() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFile: Cannot obtain access to output file: " + copyFilepath );
            return false;
        }
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFile: Obtained access to output file without using source metadata as a constraint: " + copyFilepath );
    }

    // Perform the actual byte copy synchronously and check if the expected size matches the actual size
    UInt64 bytesWritten = targetFile->GetAccess()->Write( *originalFile->GetAccess() );
    if ( metaData.hasResourceSizeInBytes )
    {
        if ( bytesWritten != metaData.resourceSizeInBytes )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFile: Failed to fully copy file \"" + originalFilepath +
                "\" to \"" + copyFilepath + "\". Bytes written (" + CORE::ToString( bytesWritten ) +
                ") does not match expected size (" + CORE::ToString( metaData.resourceSizeInBytes ) + ")" );

            targetFile.Unlink();

            // We should not leave a corrupt file behind
            if ( !DeleteFile( copyFilepath, false ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:CopyFile: Failed to delete partially copied file at \"" + copyFilepath + "\". Corrupt file may now exist in target location, please check!" );
            }
            return false;
        }
    }

    // Again set the meta-data, this time after the file has been written
    // We need to do this regardless to fix the 'lastModified' time if possible
    // hopefully the other flags were already set during the file creation
    if ( !SetFileMetaData( copyFilepath, metaData ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFile: Could not apply meta-data to copied file \"" + copyFilepath +
            "\" using info from \"" + originalFilepath + "\". File content is copied but meta-data will differ" );
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFile: Successfully copied file \"" + originalFilepath +
            "\" to \"" + copyFilepath + "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVFS::CopyFileAsync( const CORE::CString& originalFilepath ,
                     const CORE::CString& copyFilepath     ,
                     const bool overwrite                  ,
                     const CORE::CString& asyncRequestId   )
{GUCEF_TRACE;

    try
    {
        CCopyFileTaskData operationData;
        operationData.operationType = ASYNCVFSOPERATIONTYPE_COPYFILE;
        operationData.asyncRequestId = asyncRequestId;
        operationData.originalFilepath = originalFilepath;
        operationData.copyFilepath = copyFilepath;
        operationData.overwrite = overwrite;

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_asyncOpsThreadpool );
        return threadPool->QueueTask( CAsyncVfsOperation::TaskType, &operationData, GUCEF_NULL, &AsObserver() );
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:CopyFileAsync: Failed to queue task due to timeout. asyncRequestId=" +
            asyncRequestId );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:CopyFileAsync: Failed to queue task due to exception. asyncRequestId=" +
            asyncRequestId + " what=" + CORE::ToString( e.what() ) );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::EncodeFile( const CORE::CString& originalFilepath ,
                  const CORE::CString& encodedFilepath  ,
                  const bool overwrite                  ,
                  const CORE::CString& codecFamily      ,
                  const CORE::CString& encodeCodec      )
{GUCEF_TRACE;

    if ( originalFilepath == encodedFilepath )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeFile: in-place replacement not suppported at present" );
        return false;
    }

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TICodecPtr codec = codecRegistry.TryGetCodec( codecFamily, encodeCodec );
    if ( codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeFile: Cannot encode file due to inability to obtain codec " +
            encodeCodec + " for codec family " + codecFamily );
        return false;
    }

    TBasicVfsResourcePtr originalFile = GetFile( originalFilepath, "rb", overwrite );
    if ( originalFile.IsNULL() || originalFile->GetAccess().IsNULL() || !originalFile->GetAccess()->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeFile: Cannot obtain original file: " + originalFilepath );
        return false;
    }

    TBasicVfsResourcePtr targetFile = GetFile( encodedFilepath, "wb", overwrite );
    if ( targetFile.IsNULL() || targetFile->GetAccess().IsNULL() || !targetFile->GetAccess()->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeFile: Cannot obtain access to output file: " + encodedFilepath );
        return false;
    }

    if ( !codec->Encode( *originalFile->GetAccess(), *targetFile->GetAccess() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeFile: Failed to encode file \"" + originalFilepath +
            "\" using codec \"" + encodeCodec + "\" from codec family \"" + codecFamily + "\"" );
        return false;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeFile: Successfully encoded file \"" + originalFilepath +
            "\" using codec \"" + encodeCodec + "\" from codec family \"" + codecFamily + "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVFS::EncodeFileAsync( const CORE::CString& originalFilepath ,
                       const CORE::CString& encodedFilepath  ,
                       const bool overwrite                  ,
                       const CORE::CString& codecFamily      ,
                       const CORE::CString& encodeCodec      ,
                       const CORE::CString& asyncRequestId   )
{GUCEF_TRACE;

    try
    {
        CEncodeFileTaskData operationData;
        operationData.operationType = ASYNCVFSOPERATIONTYPE_ENCODEFILE;
        operationData.asyncRequestId = asyncRequestId;
        operationData.originalFilepath = originalFilepath;
        operationData.encodedFilepath = encodedFilepath;
        operationData.overwrite = overwrite;
        operationData.codecFamily = codecFamily;
        operationData.encodeCodec = encodeCodec;

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_asyncOpsThreadpool );
        return threadPool->QueueTask( CAsyncVfsOperation::TaskType, &operationData, GUCEF_NULL, &AsObserver() );
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeFileAsync: Failed to queue task due to timeout. asyncRequestId=" +
            asyncRequestId );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:EncodeFileAsync: Failed to queue task due to exception. asyncRequestId=" +
            asyncRequestId + " what=" + CORE::ToString( e.what() ) );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::DecodeAsFile( CORE::CDynamicBuffer& decodedOutput  ,
                    const CORE::UInt64 bufferOffset      ,
                    const CORE::CString& encodedFilePath ,
                    const CORE::CString& codecFamily     ,
                    const CORE::CString& decodeCodec     )
{GUCEF_TRACE;

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TICodecPtr codec = codecRegistry.TryGetCodec( codecFamily, decodeCodec );
    if ( codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeAsFile: Cannot decode file due to inability to obtain codec " +
            decodeCodec + " for codec family " + codecFamily );
        return false;
    }

    TBasicVfsResourcePtr sourceFile = GetFile( encodedFilePath, "rb", false );
    if ( sourceFile.IsNULL() || sourceFile->GetAccess().IsNULL() || !sourceFile->GetAccess()->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeAsFile: Cannot obtain access to output file: " + encodedFilePath );
        return false;
    }

    CORE::CDynamicBufferAccess bufferAccess( decodedOutput );
    bufferAccess.Setpos( (UInt32) bufferOffset );

    if ( !codec->Decode( *sourceFile->GetAccess(), bufferAccess  ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeAsFile: Failed to decode buffer using codec \"" +
            decodeCodec + "\" from codec family \"" + codecFamily + "\"" );
        return false;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeAsFile: Successfully decoded buffer using codec \"" +
        decodeCodec + "\" from codec family \"" + codecFamily + "\" loaded from \"" + encodedFilePath + "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::EncodeAsFile( const CORE::CDynamicBuffer& data     ,
                    const CORE::UInt64 bufferOffset      ,
                    const CORE::CString& encodedFilepath ,
                    const bool overwrite                 ,
                    const CORE::CString& codecFamily     ,
                    const CORE::CString& encodeCodec     )
{GUCEF_TRACE;

    if ( data.GetDataSize() < bufferOffset )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeAsFile: Invalid buffer offset given which is larger than the available data" );
        return false;
    }

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TICodecPtr codec = codecRegistry.TryGetCodec( codecFamily, encodeCodec );
    if ( codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeAsFile: Cannot encode file due to inability to obtain codec " +
            encodeCodec + " for codec family " + codecFamily );
        return false;
    }

    TBasicVfsResourcePtr targetFile = GetFile( encodedFilepath, "wb", overwrite );
    if ( targetFile.IsNULL() || targetFile->GetAccess().IsNULL() || !targetFile->GetAccess()->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeAsFile: Cannot obtain access to output file: " + encodedFilepath );
        return false;
    }

    CORE::CDynamicBufferAccess bufferAccess( data );
    bufferAccess.Setpos( (UInt32) bufferOffset );

    if ( !codec->Encode( bufferAccess, *targetFile->GetAccess() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeAsFile: Failed to encode buffer using codec \"" +
            encodeCodec + "\" from codec family \"" + codecFamily + "\"" );
        return false;
    }

    targetFile->GetAccess()->Flush();

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeFile: Successfully encoded buffer using codec \"" +
        encodeCodec + "\" from codec family \"" + codecFamily + "\"" );

    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVFS::EncodeAsFileAsync( const CORE::CDynamicBuffer& data     ,
                         const CORE::UInt64 bufferOffset      ,
                         const CORE::CString& encodedFilepath ,
                         const bool overwrite                 ,
                         const CORE::CString& codecFamily     ,
                         const CORE::CString& encodeCodec     ,
                         const CORE::CString& asyncRequestId  )
{GUCEF_TRACE;

    try
    {
        CEncodeBufferAsFileTaskData operationData;
        operationData.operationType = ASYNCVFSOPERATIONTYPE_ENCODEDATAASFILE;
        operationData.asyncRequestId = asyncRequestId;
        operationData.data = data;
        operationData.bufferOffset = bufferOffset;
        operationData.encodedFilepath = encodedFilepath;
        operationData.overwrite = overwrite;
        operationData.codecFamily = codecFamily;
        operationData.encodeCodec = encodeCodec;

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_asyncOpsThreadpool );
        return threadPool->QueueTask( CAsyncVfsOperation::TaskType, &operationData, GUCEF_NULL, &AsObserver() );
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:EncodeAsFileAsync: Failed to queue task due to timeout. asyncRequestId=" +
            asyncRequestId );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:EncodeAsFileAsync: Failed to queue task due to exception. asyncRequestId=" +
            asyncRequestId + " what=" + CORE::ToString( e.what() ) );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::EncodeAsFile( CORE::CIOAccess& externalData        ,
                    const CORE::CString& encodedFilepath ,
                    const bool overwrite                 ,
                    const CORE::CString& codecFamily     ,
                    const CORE::CString& encodeCodec     )
{GUCEF_TRACE;

    // For now we just route to a memory buffer based implementation
    // @TODO: improve

    CORE::CDynamicBuffer buffer;
    externalData.Read( buffer, 1 );

    return EncodeAsFile( buffer, 0, encodedFilepath, overwrite, codecFamily, encodeCodec );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVFS::EncodeAsFileAsync( CORE::CIOAccess& externalData        ,
                         const CORE::CString& encodedFilepath ,
                         const bool overwrite                 ,
                         const CORE::CString& codecFamily     ,
                         const CORE::CString& encodeCodec     ,
                         const CORE::CString& asyncRequestId  )
{GUCEF_TRACE;

    // For now we just route to a memory buffer based implementation
    // @TODO: improve

    CORE::CDynamicBuffer buffer;
    externalData.Read( buffer, 1 );

    return EncodeAsFileAsync( buffer, 0, encodedFilepath, overwrite, codecFamily, encodeCodec, asyncRequestId );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::DecodeFile( const CORE::CString& originalFilepath ,
                  const CORE::CString& decodedFilepath  ,
                  const bool overwrite                  ,
                  const CORE::CString& codecFamily      ,
                  const CORE::CString& decodeCodec      )
{GUCEF_TRACE;

    if ( originalFilepath == decodedFilepath )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeFile: in-place replacement not suppported at present" );
        return false;
    }

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TICodecPtr codec = codecRegistry.TryGetCodec( codecFamily, decodeCodec );
    if ( codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeFile: Cannot encode file due to inability to obtain codec " +
            decodeCodec + " for codec family " + codecFamily );
        return false;
    }

    TBasicVfsResourcePtr originalFile = GetFile( originalFilepath, "rb", overwrite );
    if ( originalFile.IsNULL() || originalFile->GetAccess().IsNULL() || !originalFile->GetAccess()->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeFile: Cannot obtain original file: " + originalFilepath );
        return false;
    }

    TBasicVfsResourcePtr targetFile = GetFile( decodedFilepath, "wb", overwrite );
    if ( targetFile.IsNULL() || targetFile->GetAccess().IsNULL() || !targetFile->GetAccess()->IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeFile: Cannot obtain access to output file: " + decodedFilepath );
        return false;
    }

    if ( !codec->Encode( *originalFile->GetAccess(), *targetFile->GetAccess() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeFile: Failed to encode file \"" + originalFilepath +
            "\" using codec \"" + decodeCodec + "\" from codec family \"" + codecFamily + "\"" );
        return false;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeFile: Successfully decoded file \"" + originalFilepath +
            "\" using codec \"" + decodeCodec + "\" from codec family \"" + codecFamily + "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVFS::DecodeFileAsync( const CORE::CString& originalFilepath ,
                       const CORE::CString& decodedFilepath  ,
                       const bool overwrite                  ,
                       const CORE::CString& codecFamily      ,
                       const CORE::CString& decodeCodec      ,
                       const CORE::CString& asyncRequestId   )
{GUCEF_TRACE;

    try
    {
        CDecodeFileTaskData operationData;
        operationData.operationType = ASYNCVFSOPERATIONTYPE_DECODEFILE;
        operationData.asyncRequestId = asyncRequestId;
        operationData.originalFilepath = originalFilepath;
        operationData.decodedFilepath = decodedFilepath;
        operationData.overwrite = overwrite;
        operationData.codecFamily = codecFamily;
        operationData.decodeCodec = decodeCodec;

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_asyncOpsThreadpool );
        return threadPool->QueueTask( CAsyncVfsOperation::TaskType, &operationData, GUCEF_NULL, &AsObserver() );
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VFS:DecodeFileAsync: Failed to queue task due to timeout. asyncRequestId=" +
            asyncRequestId );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "VFS:DecodeFileAsync: Failed to queue task due to exception. asyncRequestId=" +
            asyncRequestId + " what=" + CORE::ToString( e.what() ) );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::StoreAsFile( const CORE::CString& file        ,
                   const CORE::CDynamicBuffer& data ,
                   const CORE::UInt64 offset        ,
                   const bool overwrite             )
{GUCEF_TRACE;

    CString filepath = ConformVfsFilePath( file );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( filepath, true, mountLinks );

    // Try to store using the available archives
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        TArchivePtr archive = mountLink->mountEntry->archive;

        if ( archive->StoreAsFile( mountLink->remainder  ,
                                   data                 ,
                                   offset               ,
                                   overwrite            ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Vfs: Stored " + CORE::UInt32ToString( data.GetDataSize() )  +
                    " bytes as file content at offset " + CORE::UInt64ToString( offset ) + " using archive mounted at: " + mountLink->remainder );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vfs: Failed to store " + CORE::UInt32ToString( data.GetDataSize() )  +
                    " bytes as file content at offset " + CORE::UInt64ToString( offset ) + " using archive mounted at: " + mountLink->remainder );
        }

        ++i;
    }

    if ( mountLinks.empty() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Vfs: Unable to store " + CORE::UInt32ToString( data.GetDataSize() )  +
                " bytes as file content at offset " + CORE::UInt64ToString( offset ) + ". No eligible archives mounted" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

TBasicVfsResourcePtr
CVFS::GetFile( const CORE::CString& file          ,
               const char* mode /* = "rb" */      ,
               const bool overwrite /* = false */ )
{GUCEF_TRACE;

    CString filepath = ConformVfsFilePath( file );
    bool fileMustExist = *mode == 'r';

    if ( filepath.IsNULLOrEmpty() || GUCEF_NULL == mode )
        return TBasicVfsResourcePtr();

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( filepath, overwrite, mountLinks );

    // Find the file in the available archives
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);

        bool tryToGetFile = true;
        if ( fileMustExist )
        {
            if ( !mountLink->mountEntry->archive->FileExists( mountLink->remainder ) )
            {
                tryToGetFile = false;
            }
        }

        if ( tryToGetFile )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Vfs: Found requested file using mount link remainder: " + mountLink->remainder );
            TArchivePtr archive = mountLink->mountEntry->archive;
            TBasicVfsResourcePtr filePtr = archive->GetFile( mountLink->remainder ,
                                                             mode                ,
                                                             m_maxmemloadsize     ,
                                                             overwrite           );

            if ( !filePtr )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Vfs: Requested file pre-conditions passed but could not be loaded from mounted archive using mount link remainder: " + mountLink->remainder );
            }
            return filePtr;
        }

        ++i;
    }

    // Unable to load file
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Vfs: Unable to locate a mount which can provide the file: " + file );
    return TBasicVfsResourcePtr();
}

/*-------------------------------------------------------------------------*/

TBasicVfsResourcePtr
CVFS::GetFileAs( const CORE::CString& file               ,
                 const CORE::CResourceMetaData& metaData ,
                 const char* mode                        ,
                 const bool overwrite                    )
{GUCEF_TRACE;

    CString filepath = ConformVfsFilePath( file );

    if ( filepath.IsNULLOrEmpty() || GUCEF_NULL == mode )
        return TBasicVfsResourcePtr();

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( filepath, overwrite, mountLinks );

    // Find the file in the available archives
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Vfs: Found requested file using mount link remainder: " + mountLink->remainder );
        TArchivePtr archive = mountLink->mountEntry->archive;
        TBasicVfsResourcePtr filePtr = archive->GetFileAs( mountLink->remainder ,
                                                           metaData             ,
                                                           mode                 ,
                                                           m_maxmemloadsize     ,
                                                           overwrite            );

        if ( filePtr )
            return filePtr;

        ++i;
    }

    // Unable to load file
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Vfs: Unable to locate a mount which can provide the file using the provided meta-data constraints: " + file );
    return TBasicVfsResourcePtr();
}

/*-------------------------------------------------------------------------*/

bool
CVFS::LoadFile( CORE::CDynamicBuffer& destinationBuffer ,
                const CORE::CString& filePath           ,
                const char* mode                        )
{GUCEF_TRACE;

    // This operation logically only supports read modes
    if ( 0 != strcmp( "rb", mode ) && 0 != strcmp( "r", mode ) )
        return false;

    // First load the file as a VFS reference as usual
    TBasicVfsResourcePtr fileReference = GetFile( filePath, mode, false );
    if ( fileReference.IsNULL() || fileReference->GetAccess().IsNULL() )
        return false;

    // load the data from whatever abstracted vfs medium into memory
    destinationBuffer.Append( *fileReference->GetAccess() );

    // When we leave this scope the VFS reference is released
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::LoadFile( CORE::CDataNode& destination    ,
                const CORE::CString& filePath   ,
                const CORE::CString& codecToUse )
{GUCEF_TRACE;


    CORE::CString actualCodecToUse = codecToUse;
    if ( actualCodecToUse.IsNULLOrEmpty() )
        actualCodecToUse = CORE::ExtractFileExtention( filePath );
    if ( actualCodecToUse.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Vfs:LoadFile: No codec type name or file extension provided for file: " + filePath );
        return false;
    }

    // First load the file as a VFS reference as usual
    TBasicVfsResourcePtr fileReference = GetFile( filePath, "rb", false );
    if ( fileReference.IsNULL() || fileReference->GetAccess().IsNULL() )
        return false;

    // Now obtain the codec
    CORE::CDStoreCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( codecRegistry.TryLookup( actualCodecToUse, codec, false ) && !codec.IsNULL() )
    {
        // Now pass the I/O access to the codec
        CORE::IOAccessPtr accessPtr = fileReference->GetAccess();
        if ( codec->BuildDataTree( &destination, accessPtr.GetPointerAlways() ) )
        {
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Vfs:LoadFile: Could not obtain a DataNode codec for type \"" + actualCodecToUse + "\"" );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Vfs:LoadFile: Could not obtain a DataNode codec for type \"" + actualCodecToUse + "\"" );
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::StoreAsFile( const CORE::CString& filepath   ,
                   const CORE::CDataNode& data     ,
                   const CORE::CString& codecToUse ,
                   const bool overwrite            )
{GUCEF_TRACE;

    // input sanity check. we have have a codec type name or file extension
    CORE::CString actualCodecToUse = codecToUse;
    if ( actualCodecToUse.IsNULLOrEmpty() )
        actualCodecToUse = CORE::ExtractFileExtention( filepath );
    if ( actualCodecToUse.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Vfs:StoreAsFile: No codec type name or file extension provided for file: " + filepath );
        return false;
    }

    // First asccess the file location as a VFS reference as usual to get IO access
    TBasicVfsResourcePtr fileReference = GetFile( filepath, "wb", overwrite );
    if ( fileReference.IsNULL() || fileReference->GetAccess().IsNULL() )
        return false;

    // Now obtain the codec
    CORE::CDStoreCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( codecRegistry.TryLookup( actualCodecToUse, codec, false ) && !codec.IsNULL() )
    {
        // Now pass the I/O access to the codec
        CORE::IOAccessPtr accessPtr = fileReference->GetAccess();
        if ( codec->StoreDataTree( &data, accessPtr.GetPointerAlways() ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Vfs:StoreAsFile: Successfully stored DataNode document using codec for type \"" + actualCodecToUse + "\" to : " + filepath );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Vfs:StoreAsFile: Could not serialize using a DataNode codec for type \"" + actualCodecToUse + "\"" );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Vfs:StoreAsFile: Could not obtain a DataNode codec for type \"" + actualCodecToUse + "\"" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::MountArchive( const CString& archiveName  ,
                    const CString& archivePath  ,
                    const bool writeableRequest )
{GUCEF_TRACE;

    // Mount the archive using the archive path
    return MountArchive( archiveName      ,
                         archivePath      ,
                         archivePath      ,
                         writeableRequest );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::MountArchive( const CString& archiveName  ,
                    const CString& archivePath  ,
                    const CString& mountPoint   ,
                    const bool writeableRequest )
{GUCEF_TRACE;

    // Use the archive file extension as the archive type
    CString archiveType( archivePath.SubstrToChar( '.', false ) );
    return MountArchive( archiveName      ,
                         archivePath      ,
                         archiveType      ,
                         mountPoint       ,
                         writeableRequest ,
                         false            );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::MountArchive( const CString& archiveName              ,
                    const CString& archivePath              ,
                    const CString& archiveType              ,
                    const CString& mountPoint               ,
                    const bool writeableRequest             ,
                    const bool autoMountSubArchives         ,
                    const CString& actualNonVfsPathOverride )
{GUCEF_TRACE;

    CArchiveSettings settings;
    settings.SetActualArchivePath( actualNonVfsPathOverride );
    settings.SetArchiveName( archiveName );
    settings.SetArchivePath( archivePath );
    settings.SetArchiveType( archiveType );
    settings.SetAutoMountSubArchives( autoMountSubArchives );
    settings.SetMountPath( mountPoint );
    settings.SetWriteableRequested( writeableRequest );
    return MountArchive( settings );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::DelayMountArchive( const CArchiveSettings& settings )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_delayMountedArchiveSettings.push_back( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

VFS::CString
CVFS::ConformVfsDirPath( const VFS::CString& originalDirPath ) const
{GUCEF_TRACE;

    // Regardless of how the settings are presented we want to ensure a consistent vfs path scheme
    // As such we enforce having a start '/' and a trailing '/' for dir paths

    if ( originalDirPath.IsNULLOrEmpty() )
        return "/";

    VFS::CString resultStr = originalDirPath.ReplaceChar( '\\', '/' );
    resultStr = resultStr.CompactRepeatingChar( '/' );

    if ( resultStr[ 0 ] != '/' )
        resultStr = '/' + resultStr;

    if ( resultStr[ resultStr.Length()-1 ] != '/' )
        resultStr += '/';

    return resultStr;
}

/*-------------------------------------------------------------------------*/

VFS::CString
CVFS::ConformVfsFilePath( const VFS::CString& originalFilePath ) const
{GUCEF_TRACE;

    // Regardless of how the settings are presented we want to ensure a consistent vfs path scheme
    // As such we enforce having a start '/' and NO trailing '/' for file paths

    if ( originalFilePath.IsNULLOrEmpty() )
        return CString::Empty;

    VFS::CString resultStr = originalFilePath.ReplaceChar( '\\', '/' );
    resultStr = resultStr.CompactRepeatingChar( '/' );

    if ( resultStr[ 0 ] != '/' )
        resultStr = '/' + resultStr;

    if ( resultStr[ resultStr.Length()-1 ] == '/' )
        resultStr = resultStr.CutChars( 1, false, 0 );

    return resultStr;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::MountArchive( const CArchiveSettings& settings )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwdataLock );

    // Either determine the actual non-vfs path based on previously loaded/mounted archives or use the given explicit path if any
    CString actualArchivePath = CORE::ResolveVars( settings.GetActualArchivePath() );
    if ( !actualArchivePath.IsNULLOrEmpty() || GetActualFilePath( settings.GetArchivePath(), actualArchivePath ) )
    {
        // Found a compatible type,.. create an archive for the type
        TArchivePtr archive = m_abstractArchiveFactory.Create( settings.GetArchiveType() );
        if ( !archive.IsNULL() )
        {
            // Try to load from the resource
            CArchiveSettings updatedSettings( settings );
            if ( actualArchivePath != settings.GetActualArchivePath() )
                updatedSettings.SetActualArchivePath( actualArchivePath );

            // Regardless of how the settings are presented we want to ensure a consistent mounting scheme
            // As such we enforce having a start '/' and a trailing '/' for mount paths, with the start '/' signaling 'root'
            updatedSettings.SetMountPath( ConformVfsDirPath( updatedSettings.GetMountPath() ) );

            if ( archive->LoadArchive( updatedSettings ) )
            {
                // Successfully loaded/linked the archive
                // We will add it to our mount list
                TMountEntryPtr archiveEntry = MountEntry::CreateSharedObj();
                archiveEntry->abspath = actualArchivePath;
                archiveEntry->archiveType = settings.GetArchiveType();
                archiveEntry->path = actualArchivePath;
                archiveEntry->writeable = updatedSettings.GetWriteableRequested();
                archiveEntry->archive = archive;
                archiveEntry->mountPath = updatedSettings.GetMountPath();

                m_mountList.push_back( archiveEntry );
                m_archivePtrToMountEntryLookup[ archive.GetPointerAlways() ] = archiveEntry;

                SubscribeTo( archive.GetPointerAlways() );

                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MountArchive: Mounted archive of type \"" +
                    settings.GetArchiveType() + "\" at path: " + updatedSettings.GetMountPath() );

                TArchiveMountedEventData eData( archiveEntry->mountPath );
                NotifyObservers( ArchiveMountedEvent, &eData );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MountArchive: Failed to load archive of type \"" +
                    settings.GetArchiveType() + "\" which was to be mounted at path: " + updatedSettings.GetMountPath() );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MountArchive: No factory available to handle archive of type: " + settings.GetArchiveType() );
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::MountArchive( const CString& archiveName           ,
                    TBasicVfsResourcePtr archiveResource ,
                    const CString& archiveType           ,
                    const CString& mountPoint            ,
                    const bool writeableRequest          )
{GUCEF_TRACE;

    if ( archiveResource.IsNULL() )
        return false;

    MT::CScopeWriterLock lock( m_rwdataLock );

    // create an archive for the type
    TArchivePtr archive = m_abstractArchiveFactory.Create( archiveType );
    if ( !archive.IsNULL() )
    {
        // Try to load from the resource
        if ( archive->LoadArchive( archiveName      ,
                                   archiveResource  ,
                                   writeableRequest ) )
        {
            // Successfully loaded/linked the archive
            // We will add it to our mount list
            TMountEntryPtr archiveEntry = MountEntry::CreateSharedObj();
            archiveEntry->path = archiveResource->GetFilename();
            archiveEntry->abspath = archiveResource->GetFilePath();
            archiveEntry->writeable = writeableRequest;
            archiveEntry->archive = archive;

            // Regardless of how the settings are presented we want to ensure a conistent mounting scheme
            // As such we enforce having a start '/' and no trailing '/' for mount paths, with the start '/' signaling 'root'
            archiveEntry->mountPath = ConformVfsDirPath( mountPoint );

            m_mountList.push_back( archiveEntry );
            m_archivePtrToMountEntryLookup[ archive.GetPointerAlways() ] = archiveEntry;

            SubscribeTo( archive.GetPointerAlways() );

            TArchiveMountedEventData eData( archiveEntry->mountPath );
            NotifyObservers( ArchiveMountedEvent, &eData );
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::IsMountedArchive( const CString& location ) const
{GUCEF_TRACE;

    // Keep in mind that mounted archives count as directories
    CString testLocation( ConformVfsDirPath( location ) );

    MT::CScopeReaderLock lock( m_rwdataLock );

    TMountVector::const_iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr archiveEntry = (*i);
        if ( archiveEntry->mountPath == testLocation )
        {
            return true;
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

void
CVFS::GetMountedArchiveList( const CString& location ,
                             const CString& filter   ,
                             const bool recursive    ,
                             TStringSet& outputList  ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    TMountVector::const_iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr archiveEntry = (*i);

        if ( archiveEntry->mountPath.Length() > 0 )
        {
            // Check if the location is a root for the mount path
            Int32 subSection = archiveEntry->mountPath.HasSubstr( location, true );
            if ( 0 == subSection )
            {
                // We now know the archive is mounted in either the root of "location"
                // or a sub directory
                if ( !recursive )
                {
                    // Check if we have multiple subdirs beyond the "location" to get to
                    // the archive. If so then we cannot add this archive because recursive
                    // searching is not allowed.
                    CString remainder = archiveEntry->mountPath.CutChars( subSection, true );
                    if ( remainder.Length() > 0 )
                    {
                        // make sure we are using a single directory seperator scheme
                        remainder = remainder.ReplaceChar( '\\', '/' );

                        // Dont count a starting directory seperator
                        if ( remainder[ 0 ] == '/' )
                        {
                            remainder = remainder.CutChars( 1, true );
                        }

                        Int32 dirDelimter = remainder.HasChar( '/', true );
                        if ( dirDelimter > -1 )
                        {
                            // We found a directory seperator so now me must check if it happens
                            // to be the last character
                            if ( remainder.Length() != dirDelimter+1 )
                            {
                                // The directory seperator was not the last character so we have multiple
                                // sub-dirs which is not allowed, we cannot add this item
                                ++i;
                                continue;
                            }
                        }
                    }
                }

                // Now we must validate the filename with the given filter
                CString filename = GUCEF::CORE::ExtractFilename( archiveEntry->mountPath );
                if ( FilterValidation( filename ,
                                       filter   ) )
                {
                    // Everything checks out, we have found a mounted archive that meets
                    // the criterea so we add it to the list
                    outputList.insert( archiveEntry->mountPath );
                }
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CVFS::GetActualFilePath( const CString& file ,
                         CString& path       ) const
{GUCEF_TRACE;

    CString filePath = ConformVfsFilePath( file );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( filePath, false, mountLinks );

    // Find the file in the available archives
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        if ( mountLink->mountEntry->archive->FileExists( mountLink->remainder ) )
        {
            path = mountLink->mountEntry->abspath;
            CORE::AppendToPath( path, mountLink->remainder );
            return true;
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::GetVfsPathForAbsolutePath( const CString& absolutePath ,
                                 CString& relativePath       ) const
{GUCEF_TRACE;

    // Make sure there are no more variables in the given absolute path
    CString realAbsPath = CORE::RelativePath( absolutePath );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Try to match a mount entry
    TMountVector::const_iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr mountEntry = (*i);
        Int32 subStrIndex = realAbsPath.HasSubstr( mountEntry->abspath, true );
        if ( subStrIndex == 0 )
        {
            // This mount entry has at least a partially matching path
            CString remainder = realAbsPath.CutChars( mountEntry->abspath.Length(), true );

            // Now we will try to locate a resource given that path so we can validate
            // that this is not a coincidence
            if ( mountEntry->archive->FileExists( remainder ) )
            {
                // We found a file given this remainer
                // Now we construct the full relative path including mount path
                relativePath = mountEntry->mountPath;
                CORE::AppendToPath( relativePath, remainder );
                return true;
            }
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::DirExists( const CString& dirPath ) const
{GUCEF_TRACE;

    CString path = ConformVfsDirPath( dirPath );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Search for a file and then get the hash
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        if ( mountLink->mountEntry->archive->DirExists( mountLink->remainder ) )
        {
            return true;
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::FileExists( const CString& file ) const
{GUCEF_TRACE;

    CString path = ConformVfsFilePath( file );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Search for a file and then get the hash
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        if ( mountLink->mountEntry->archive->FileExists( mountLink->remainder ) )
        {
            return true;
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::IsDirPathSubOfDirPath( const CORE::CString& dirPath        ,
                             const CORE::CString& dirPathToCheck ) const
{GUCEF_TRACE;

    CString conformedDirPath = ConformVfsDirPath( dirPath );
    CString conformedTestDirPath = ConformVfsDirPath( dirPathToCheck );
    return 0 == conformedTestDirPath.HasSubstr( conformedDirPath, true );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::IsFilePathSubOfDirPath( const CORE::CString& dirPath         ,
                              const CORE::CString& filePathToCheck ) const
{GUCEF_TRACE;

    CString conformedDirPath = ConformVfsDirPath( dirPath );
    CString conformedFilePath = ConformVfsFilePath( filePathToCheck );
    return 0 == conformedFilePath.HasSubstr( conformedDirPath, true );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::UnmountArchiveByName( const CString& archiveName )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwdataLock );

    TMountVector::iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        TMountEntryPtr mountEntry = (*i);
        if ( mountEntry->archive->GetArchiveName() == archiveName )
        {
            if ( mountEntry->archive->UnloadArchive() )
            {
                TArchiveUnmountedEventData eData( mountEntry->mountPath );
                m_archivePtrToMountEntryLookup.erase( mountEntry->archive.GetPointerAlways() );
                m_mountList.erase( i );

                lock.EarlyUnlock();

                NotifyObservers( ArchiveUnmountedEvent, &eData );
                return true;
            }
            return false;
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::AddRoot( const CORE::CString& rootpath                                ,
               const CString& archiveName                                   ,
               const bool writeable                                         ,
               const bool autoMountArchives /* = false */                   ,
               const CString& mountPoint /* = "" */                         ,
               const CString& archiveType /* = FileSystemArchiveTypeName */ )
{GUCEF_TRACE;

    CArchiveSettings settings;
    settings.SetArchiveName( archiveName );
    settings.SetActualArchivePath( rootpath );
    settings.SetWriteableRequested( writeable );
    settings.SetAutoMountSubArchives( autoMountArchives );
    settings.SetMountPath( mountPoint );
    settings.SetArchiveType( archiveType );

    return MountArchive( settings );
}

/*-------------------------------------------------------------------------*/

void
CVFS::GetListOfSupportedArchiveTypes( TAbstractArchiveFactory::TKeySet& outList ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );
    m_abstractArchiveFactory.ObtainKeySet( outList );
}

/*-------------------------------------------------------------------------*/

void
CVFS::SetMemloadSize( UInt32 bytesize )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwdataLock );

    m_maxmemloadsize = bytesize;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS maximum memory load size set to " + CORE::UInt32ToString( bytesize ) + " Bytes" );
}

/*-------------------------------------------------------------------------*/

UInt32
CVFS::GetMemloadSize( void ) const
{GUCEF_TRACE;

    return m_maxmemloadsize;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    CORE::CDataNode* n = tree.Structure( "GUCEF%VFS%CVFS" ,
                                         '%'              );

    if ( GUCEF_NULL != n )
    {
        n->SetAttribute( "maxmemload", m_maxmemloadsize );

        n->DelSubTree();
        UInt32 count = (UInt32) m_mountList.size();
        CORE::CDataNode pathnode( "vfsroot" );
        for ( UInt32 i=0; i<count; ++i )
        {
            pathnode.SetAttribute( "path", m_mountList[ i ]->path );
            if ( m_mountList[ i ]->archive->IsWriteable() )
            {
                pathnode.SetAttribute( "writeable", "true" );
            }
            else
            {
                pathnode.SetAttribute( "writeable", "false" );
            }
            n->AddChild( pathnode );
        }

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS configuration successfully saved" );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::LoadVfsSystemConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_maxmemloadsize = cfg.GetAttributeValueOrChildValueByName( "maxmemload" ).AsUInt32( m_maxmemloadsize, true );
    m_asyncOpsThreadpool = cfg.GetAttributeValueOrChildValueByName( "asyncOpsThreadpool" ).AsString( m_asyncOpsThreadpool, true );
    m_asyncOpsMinWorkerThreads = cfg.GetAttributeValueOrChildValueByName( "asyncOpsMinWorkerThreads" ).AsUInt32( m_asyncOpsMinWorkerThreads, true );
    m_vfsRootForVolumes = cfg.GetAttributeValueOrChildValueByName( "vfsRootForVolumes" ).AsString( m_vfsRootForVolumes, true );
    m_vfsVolumesAreWriteableIfPossible = cfg.GetAttributeValueOrChildValueByName( "vfsVolumesAreWriteableIfPossible" ).AsBool( m_vfsVolumesAreWriteableIfPossible, true );
    m_autoMountFsVolumes = cfg.GetAttributeValueOrChildValueByName( "m_autoMountFsVolumes" ).AsBool( m_autoMountFsVolumes, true );

    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_asyncOpsThreadpool );
    threadPool->SetDesiredMinNrOfWorkerThreads( m_asyncOpsMinWorkerThreads );

    if ( m_autoMountFsVolumes )
        MountFileSystemRoots( m_vfsRootForVolumes );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::LoadConfig( const CORE::CDataNode& tree )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwdataLock );

    bool globalConfigLoadInProgress = IsGlobalConfigLoadInProgress();
    const CORE::CDataNode* n = tree.Search( "GUCEF%VFS%CVFS" ,
                                            '%'              ,
                                            false            );

    bool triedLoadingSystemConfig = false;
    if ( GUCEF_NULL != n )
    {
        LoadVfsSystemConfig( *n );
        triedLoadingSystemConfig = true;

        CORE::CDataNode::TConstDataNodeSet rootNodeList = n->FindChildrenOfType( "VfsRoot" );
        CORE::CDataNode::TConstDataNodeSet::iterator i = rootNodeList.begin();
        while( i != rootNodeList.end() )
        {
            const CORE::CDataNode* legacySettingsNode = (*i);
            CArchiveSettings settings;
            settings.LoadConfig( *legacySettingsNode );
            if ( !globalConfigLoadInProgress )
                MountArchive( settings );
            else
                DelayMountArchive( settings );
            ++i;
        }

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS legacy configuration load sequence completed" );
    }

    n = tree.Find( "VFS" );
    if ( GUCEF_NULL != n )
    {
        LoadVfsSystemConfig( *n );
        triedLoadingSystemConfig = true;

        CORE::CDataNode::TConstDataNodeSet rootNodeList = n->FindChildrenOfType( "ArchiveSettings", false, false );
        CORE::CDataNode::TConstDataNodeSet::iterator i = rootNodeList.begin();
        while( i != rootNodeList.end() )
        {
            const CORE::CDataNode* settingsNode = (*i);
            CArchiveSettings settings;
            if ( settings.LoadConfig( *settingsNode ) )
            {
                if ( !globalConfigLoadInProgress )
                    MountArchive( settings );
                else
                    DelayMountArchive( settings );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS archive settings entry failed to load" );
            }
            ++i;
        }
        const CORE::CDataNode* listRoot = n->FindChild( "archives", false );
        if ( listRoot != GUCEF_NULL )
        {
            CORE::CDataNode::const_iterator node = listRoot->ConstBegin();
            while( node != n->ConstEnd() )
            {
                const CORE::CDataNode* settingsNode = (*node);

                CArchiveSettings settings;
                if ( settings.LoadConfig( *settingsNode ) )
                {
                    if ( !globalConfigLoadInProgress )
                        MountArchive( settings );
                    else
                        DelayMountArchive( settings );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS archive settings entry failed to load" );
                }
                ++node;
            }
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS archive configuration load sequence completed" );
    }

    if ( !triedLoadingSystemConfig )
    {
        // Even if we have no system config we still want to trigger relative init functions using defaults
        CORE::CDataNode emptyDummy;
        LoadVfsSystemConfig( emptyDummy );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::AddMountAliases( const CString& mountPath                   ,
                       const CString::StringSet& mountPathAliases )
{GUCEF_TRACE;

    bool addedAliases = false;
    CString path = ConformVfsDirPath( mountPath );

    MT::CScopeReaderLock readerLock( m_rwdataLock );

    TMountVector::const_iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        TMountEntryPtr mountEntry = (*i);
        if ( mountEntry->mountPath == path )
        {
            MT::CScopeWriterLock writerLock( readerLock );

            CString::StringSet& aliases = mountEntry->mountPathAliases;
            CString::StringSet::const_iterator n = mountPathAliases.begin();
            while ( n != mountPathAliases.end() )
            {
                const CString& alias = (*n);
                aliases.insert( alias );
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS:AddMountAliases: Path alias \"" + alias + "\" added for existing mount path \"" + mountPath + "\"" );
                ++n;
            }
            addedAliases = true;
        }
        ++i;
    }

    return addedAliases;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::AddMountAlias( const CString& mountPath      ,
                     const CString& mountPathAlias )
{GUCEF_TRACE;

    CString::StringSet mountPathAliases;
    mountPathAliases.insert( mountPathAlias );
    return AddMountAliases( mountPath, mountPathAliases );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::MountFileSystemRoots( const CORE::CString& overrideMountRoot ,
                            bool vfsVolumesAreWriteableIfPossible  )
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwdataLock );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MountFileSystemRoots: Mounting filesystem roots into VFS" );

    CORE::CString mountRoot = ConformVfsDirPath( overrideMountRoot.IsNULLOrEmpty() ? m_vfsRootForVolumes : overrideMountRoot );
    if ( m_vfsRootForVolumes != mountRoot )
    {
        MT::CScopeWriterLock writerLock( readerLock );
        m_vfsRootForVolumes = mountRoot;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MountFileSystemRoots: Volume mount root set to: " + mountRoot );

    // First get the list of volume IDs
    CString::StringSet volumeIds;
    if ( !CORE::GetAllFileSystemStorageVolumes( volumeIds ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VFS:MountFileSystemRoots: Failed to get storage volume ids" );
        return false;
    }

    // Mount all volumes via their volume id
    bool totalSuccess = true;
    CORE::CString volumeIdMountRoot = mountRoot + GUCEF_VFS_RESERVED_STRING_VOLUMES_BY_ID;
    CORE::CString::StringSet::iterator n = volumeIds.begin();
    while ( n != volumeIds.end() )
    {
        const CORE::CString& volumeId = (*n);
        CORE::CString volumeIdMount = volumeIdMountRoot + volumeId + "/";

        CORE::CString fsVolumePath;
        CORE::GetVolumePathForVolumeId( volumeId, fsVolumePath );

        CORE::CStorageVolumeInfoOSData volumeInfoOSData;
        CORE::CStorageVolumeInformation fsVolumeInfo;
        volumeInfoOSData.SetVolumeId( volumeId );
        CORE::GetFileSystemStorageVolumeInformation( volumeInfoOSData, fsVolumeInfo );

        bool volumeIsWriteable = m_vfsVolumesAreWriteableIfPossible;
        if ( fsVolumeInfo.hasIsReadOnly && m_vfsVolumesAreWriteableIfPossible )
            volumeIsWriteable = !fsVolumeInfo.isReadOnly;

        bool addRootSuccess = AddRoot( fsVolumePath, volumeId, volumeIsWriteable, false, volumeIdMount, FileSystemArchiveTypeName );
        if ( addRootSuccess )
        {
            // Now attempt to obtain all filesystem path names for the given volume id
            // These are the more human friendly path names that are normally used for accessing files
            // Path names returned represent root paths which are garantueed to be backed by the given volume
            // Note that this is optional and may not be supported on all platforms
            CString::StringSet fsPathNames;
            if ( CORE::GetAllFileSystemPathNamesForVolume( volumeId, fsPathNames ) && !fsPathNames.empty() )
            {
                CORE::CString volumePathMountRoot = mountRoot + GUCEF_VFS_RESERVED_STRING_VOLUMES_BY_PATH;

                CString::StringSet aliasPaths;
                CString::StringSet::iterator m = fsPathNames.begin();
                while ( m != fsPathNames.end() )
                {
                    const CString& fsPathName = (*m);
                    if ( !fsPathName.IsNULLOrEmpty() )
                    {
                        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

                        // On MS Windows we want to perform special processing for drive letter paths
                        // we want to unify the syntax with Linux style paths
                        //   Example C:\ -> /C/
                        if ( 3 == fsPathName.Length() && ( fsPathName[ 1 ] == ':' && fsPathName[ 2 ] == '\\' ) )
                        {
                            CORE::CString volumePath = ConformVfsDirPath( volumePathMountRoot + ConformVfsDirPath( CString( fsPathName[ 0 ] ) ) );
                            aliasPaths.insert( volumePath );
                        }
                        else
                        {
                            CORE::CString volumePath = ConformVfsDirPath( volumePathMountRoot + ConformVfsDirPath( fsPathName ) );
                            aliasPaths.insert( volumePath );
                        }

                        #else

                        CORE::CString volumePath = ConformVfsDirPath( volumePathMountRoot + ConformVfsDirPath( fsPathName ) );
                        aliasPaths.insert( volumePath );

                        #endif
                    }
                    ++m;
                }

                if ( !aliasPaths.empty() )
                    totalSuccess = AddMountAliases( volumeIdMount, aliasPaths ) && totalSuccess;
            }
        }
        totalSuccess = addRootSuccess && totalSuccess;
        ++n;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVFS::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::VFS::CVFS";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::FilterValidation( const CORE::CString& filename ,
                        const CORE::CString& filter   )
{GUCEF_TRACE;

    if ( !filter.IsNULLOrEmpty() )
    {
        // perform a 'glob pattern' match which is just wildcard character based segment matching

        CVFS::TStringVector segs = filter.ParseElements( '*', false );
        VFS::Int32 lastSeg = 0;
        CVFS::TStringVector::iterator i = segs.begin();
        while ( i != segs.end() )
        {
            lastSeg = filename.HasSubstr( (*i), lastSeg, true );
            if ( 0 > lastSeg )
                return false;
            ++i;
        }
        return true;
    }

    // no filter passed means no filtering should be applied
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::FilterValidation( const CORE::CString& filename           ,
                        const CORE::CString::StringSet& filters )
{GUCEF_TRACE;

    if ( !filters.empty() )
    {
        CORE::CString::StringSet::const_iterator i = filters.begin();
        while ( i != filters.end() )
        {
            // we just need to match 1 of the filters passed
            if ( FilterValidation( filename, (*i) ) )
                return true;

            ++i;
        }
        return false;
    }

    // no filters passed means no filtering should be applied
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVFS::GetEligableMounts( const CString& location                ,
                         bool mustBeWritable                    ,
                         TConstMountLinkVector& mountLinkVector ) const
{GUCEF_TRACE;

    TMountVector::const_iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr mountEntry = (*i);

        // Exclude non-writeable archives right away for 'mustBeWritable' scenarios
        if ( !mustBeWritable || mustBeWritable == mountEntry->writeable )
        {
            if ( mountEntry->mountPath.Length() > 0 )
            {
                // Check to see if the mount path is the starting sub string of the location
                if ( location.HasSubstr( mountEntry->mountPath, true ) == 0 )
                {
                    // We need to make sure we dont accidentally match a partial directory name with a similarly named path
                    // for example matching on "/example/foo" when the location is "/example/foo2/"
                    // It needs to be the full directory name to count as a match
                    if ( location.Length() == mountEntry->mountPath.Length() ||
                       ( location[ mountEntry->mountPath.Length()-1 ] == '/' ) )
                    {
                        TConstMountLinkPtr mountLink = ConstMountLink::CreateSharedObjWithParam( mountEntry );
                        mountLink->remainder = location.CutChars( mountEntry->mountPath.Length(), true );

                        // make sure the remainder does not have a path seperator prefix
                        if ( mountLink->remainder.Length() > 0 )
                        {
                            if ( ( mountLink->remainder[ 0 ] == '/' ) ||
                                 ( mountLink->remainder[ 0 ] == '\\' ) )
                            {
                                // remove the dir seperator prefix
                                mountLink->remainder = mountLink->remainder.CutChars( 1, true );
                            }
                        }

                        mountLinkVector.push_back( mountLink );

                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS: Found eligable mount link using archive of type \"" +
                            mountEntry->archive->GetType() + "\" with remainder \"" + mountLink->remainder + "\". mustBeWritable=" + CORE::BoolToString( mustBeWritable ) );
                    }
                }
                else
                if ( !mountEntry->mountPathAliases.empty() )
                {
                    CORE::CString::StringSet::iterator m = mountEntry->mountPathAliases.begin();
                    while ( m != mountEntry->mountPathAliases.end() )
                    {
                        const CORE::CString mountPathAlias = (*m);

                        // Check to see if the mount path is the starting sub string of the location
                        if ( location.HasSubstr( mountPathAlias, true ) == 0 )
                        {
                            // We need to make sure we dont accidentally match a partial directory name with a similarly named path
                            // for example matching on "/example/foo" when the location is "/example/foo2/"
                            // It needs to be the full directory name to count as a match
                            if ( location.Length() == mountPathAlias.Length() ||
                               ( location[ mountPathAlias.Length()-1 ] == '/' ) )
                            {
                                TConstMountLinkPtr mountLink = ConstMountLink::CreateSharedObjWithParam( mountEntry );
                                mountLink->remainder = location.CutChars( mountPathAlias.Length(), true );

                                // make sure the remainder does not have a path seperator prefix
                                if ( mountLink->remainder.Length() > 0 )
                                {
                                    if ( ( mountLink->remainder[ 0 ] == '/' ) ||
                                         ( mountLink->remainder[ 0 ] == '\\' ) )
                                    {
                                        // remove the dir seperator prefix
                                        mountLink->remainder = mountLink->remainder.CutChars( 1, true );
                                    }
                                }

                                mountLinkVector.push_back( mountLink );

                                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS: Found eligable mount link using archive of type \"" +
                                    mountEntry->archive->GetType() + "\" with remainder \"" + mountLink->remainder + "\". mustBeWritable=" + CORE::BoolToString( mustBeWritable ) );
                            }
                        }
                        ++m;
                    }
                }
            }
            else
            {
                TConstMountLinkPtr mountLink = ConstMountLink::CreateSharedObjWithParam( mountEntry );
                mountLink->remainder = location;
                mountLinkVector.push_back( mountLink );

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS: Found eligable mount link using archive of type \"" +
                    mountEntry->archive->GetType() + "\" with remainder \"" + location + "\". mustBeWritable=" + CORE::BoolToString( mustBeWritable ) );
            }
        }
        ++i;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFS: Found a total of " + CORE::ToString( mountLinkVector.size() ) + " eligable mounts for location \"" + location + "\"" );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::GetFileList( TStringVector& outputList      ,
                   const CString& location        ,
                   bool recursive                 ,
                   bool includePathInFilename     ,
                   const CString& nameFilter      ,
                   UInt32 maxListEntries          ) const
{GUCEF_TRACE;

    CORE::CString::StringSet filters;
    if ( !nameFilter.IsNULLOrEmpty() )
        filters.insert( nameFilter );

    return GetFileList( outputList            ,
                        location              ,
                        recursive             ,
                        includePathInFilename ,
                        filters               ,
                        maxListEntries        );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::GetFileList( TStringVector& outputList             ,
                   const CString& location               ,
                   bool recursive                        ,
                   bool includePathInFilename            ,
                   const CString::StringSet& nameFilters ,
                   UInt32 maxListEntries                 ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    CString path = ConformVfsDirPath( location );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Get a list from each mount
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        mountLink->mountEntry->archive->GetFileList( outputList                       ,
                                                     mountLink->mountEntry->mountPath ,
                                                     mountLink->remainder             ,
                                                     recursive                        ,
                                                     includePathInFilename            ,
                                                     nameFilters                      ,
                                                     maxListEntries                   );
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::GetDirList( TStringVector& outputList  ,
                  const CString& location    ,
                  bool recursive             ,
                  bool includePathInFilename ,
                  const CString& nameFilter  ,
                  UInt32 maxListEntries      ) const
{GUCEF_TRACE;

    CORE::CString::StringSet filters;
    filters.insert( nameFilter );

    return GetDirList( outputList            ,
                       location              ,
                       recursive             ,
                       includePathInFilename ,
                       filters               ,
                       maxListEntries        );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::GetDirList( TStringVector& outputList             ,
                  const CString& location               ,
                  bool recursive                        ,
                  bool includePathInFilename            ,
                  const CString::StringSet& nameFilters ,
                  UInt32 maxListEntries                 ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    CString path = ConformVfsDirPath( location );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Get a list from each mount
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        mountLink->mountEntry->archive->GetDirList( outputList                       ,
                                                    mountLink->mountEntry->mountPath ,
                                                    mountLink->remainder             ,
                                                    recursive                        ,
                                                    includePathInFilename            ,
                                                    nameFilters                      ,
                                                    maxListEntries                   );
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CVFS::GetFileList( CORE::CDataNode& outputDataTree        ,
                   const CORE::CString& location          ,
                   bool recursive  /* = false */          ,
                   const CORE::CString& filter /* = "" */ ,
                   const bool addHash /* = false */       ,
                   UInt32 maxListEntries                  ) const
{GUCEF_TRACE;

    // First we get a list of files and their path
    TStringVector outputList;
    GetFileList( outputList      ,
                 location        ,
                 recursive       ,
                 true            ,
                 filter          ,
                 maxListEntries  );

    // First we clear the tree
    outputDataTree.Clear();

    // Now we build our data tree using the list we obtained
    TStringVector::iterator i = outputList.begin();
    while ( i != outputList.end() )
    {
        // First we build the path structure in the data tree
        CString path = (*i).SubstrToChar( '/', false );
        CORE::CDataNode* pathRootNode = outputDataTree.Structure( "DIR", "Name", path, '/' );

        // Extract the filename from the path
        CORE::CString filename( (*i).C_String() + path.Length() ,
                                (*i).Length() - path.Length()   );

        // Create the file node and set all the attributes
        CORE::CDataNode fileNode( "FILE" );
        fileNode.SetAttribute( "Name", filename );
        fileNode.SetAttribute( "Size", CORE::ToString( GetFileSize( (*i) ) ) );

        if ( addHash )
        {
            // Obtain the hash for the file, this can be a lengthy operation
            fileNode.SetAttribute( "Hash", GetFileHash( (*i) ) );
        }

        // Create a copy child node thus storing the file properties in the tree
        pathRootNode->AddChild( fileNode );

        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CVFS::SetFileMetaData( const CString& filename           ,
                       CORE::CResourceMetaData& metaData )
{GUCEF_TRACE;

    CString path = ConformVfsFilePath( filename );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Search for a file and then get the meta data
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        TArchivePtr archive = mountLink->mountEntry->archive;
        if ( archive->SetFileMetaData( mountLink->remainder, metaData ) )
        {
            return true;
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::GetFileMetaData( const CString& filename           ,
                       CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    CString path = ConformVfsFilePath( filename );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Search for a file and then get the meta data
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        if ( mountLink->mountEntry->archive->GetFileMetaData( mountLink->remainder, metaData ) )
        {
            if ( metaData.resourceExists )
                return true;
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CVFS::GetFileModificationTime( const CString& filename ) const
{GUCEF_TRACE;

    CString path = ConformVfsFilePath( filename );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Search for a file and then get the hash
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        if ( mountLink->mountEntry->archive->FileExists( mountLink->remainder ) )
        {
            CORE::CDateTime modificationTime = mountLink->mountEntry->archive->GetFileModificationTime( mountLink->remainder );
            return modificationTime;
        }
        ++i;
    }

    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVFS::GetFileHash( const CORE::CString& file ) const
{GUCEF_TRACE;

    CString path = ConformVfsFilePath( file );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Search for a file and then get the hash
    CString hash;
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        if ( mountLink->mountEntry->archive->FileExists( mountLink->remainder ) )
        {
            CString hash = mountLink->mountEntry->archive->GetFileHash( mountLink->remainder );
            return hash;
        }
        ++i;
    }

    return CString();
}

/*-------------------------------------------------------------------------*/

UInt64
CVFS::GetFileSize( const CORE::CString& file ) const
{GUCEF_TRACE;

    CString path = ConformVfsFilePath( file );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Search for a file and then get its size
    CString hash;
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        if ( mountLink->mountEntry->archive->FileExists( mountLink->remainder ) )
        {
            UInt64 fileSize = mountLink->mountEntry->archive->GetFileSize( mountLink->remainder );
            return fileSize;
        }
        ++i;
    }

    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::TryResolveSpecialDir( CORE::TSpecialDirs dir                 ,
                            CString& resolvedPath                  ,
                            const CORE::CString& hintNamespacePath ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    if ( hintNamespacePath.IsNULLOrEmpty() )
    {
        // try to resolve through the archives which can represent different filesystems
        // first successful resolution will be returned
        TMountVector::const_iterator i = m_mountList.begin();
        while ( i != m_mountList.end() )
        {
            const TMountEntryPtr mountEntry = (*i);
            if ( !mountEntry->archive.IsNULL() )
                if ( mountEntry->archive->TryResolveSpecialDir( dir, resolvedPath ) )
                    return true;
            ++i;
        }
    }
    else
    {
        CString path = ConformVfsDirPath( hintNamespacePath );

        // Get a list of all eligable mounts based on the namespace hint
        TConstMountLinkVector mountLinks;
        GetEligableMounts( path, false, mountLinks );

        // Search for a file and then get its size
        TConstMountLinkVector::iterator i = mountLinks.begin();
        while ( i != mountLinks.end() )
        {
            TConstMountLinkPtr& mountLink = (*i);
            if ( mountLink->mountEntry->archive->TryResolveSpecialDir( dir, resolvedPath ) )
                return true;
            ++i;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::TryGetVfsPathForFileSystemPath( const CORE::CString& fsPath ,
                                      CString& resolvedPath       )
{GUCEF_TRACE;

    // Use the global VFS default for root auto mounting
    return TryGetVfsPathForFileSystemPath( fsPath, resolvedPath, m_autoMountFsVolumes );
}

/*-------------------------------------------------------------------------*/

bool
CVFS::TryGetVfsPathForFileSystemPath( const CORE::CString& fsPath ,
                                      CString& resolvedPath       ,
                                      bool autoMountRoots         )
{GUCEF_TRACE;

    if ( fsPath.IsNULLOrEmpty() )
        return false;

    MT::CScopeReaderLock lock( m_rwdataLock );

    // First try regular mount matching
    // This should be able to match against all explicitly mounted archives
    // We will try first against native filesystem style archives as those are the best fit for this operation
    TMountVector::const_iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr& mountEntry = (*i);

        if ( mountEntry->archiveType == CVFS::FileSystemArchiveTypeName )
        {
            const CString& absPath = mountEntry->abspath;

            // Check to see if the archive absolute path is the starting sub string of the location
            if ( fsPath.HasSubstr( absPath, true ) == 0 )
            {
                // We need to make sure we dont accidentally match a partial directory name with a similarly named path
                // for example matching on "/example/foo" when the location is "/example/foo2/"
                // It needs to be the full directory name to count as a match
                if ( absPath.Length() == fsPath.Length() ||
                    ( fsPath[ absPath.Length()-1 ] == '/' ) )
                {
                    resolvedPath = ConformVfsDirPath( mountEntry->mountPath + fsPath.CutChars( absPath.Length(), true ) );
                    return true;
                }
            }
        }
        ++i;
    }

    // Second try again but against non-native filesystem style archives
    i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr& mountEntry = (*i);

        if ( mountEntry->archiveType != CVFS::FileSystemArchiveTypeName )
        {
            const CString& absPath = mountEntry->abspath;

            // Check to see if the archive absolute path is the starting sub string of the location
            if ( fsPath.HasSubstr( absPath, true ) == 0 )
            {
                // We need to make sure we dont accidentally match a partial directory name with a similarly named path
                // for example matching on "/example/foo" when the location is "/example/foo2/"
                // It needs to be the full directory name to count as a match
                if ( absPath.Length() == fsPath.Length() ||
                    ( fsPath[ absPath.Length()-1 ] == '/' ) )
                {
                    resolvedPath = ConformVfsDirPath( mountEntry->mountPath + fsPath.CutChars( absPath.Length(), true ) );
                    return true;
                }
            }
        }
        ++i;
    }

    // If we get here we will perform special handling for filesystem paths
    // While we may not have a direct mount for the filesystem path we can still try to resolve it
    // by using the filesystem (volume) root paths for which we have mounting facilities as well (if enabled) with a known naming convention

    CORE::CString volumePath;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // On MS Windows we want to perform special processing for drive letter paths
    // we want to map based on drive letter if we can since that will be the most natural way to access files
    // on this operating system
    //   Example C:\ -> /C/
    if ( 3 < fsPath.Length() && ( fsPath[ 1 ] == ':' && fsPath[ 2 ] == '\\' ) )
    {
        // this is an absolute path so we just need to map the drive letter to the VFS volume path
        volumePath = ConformVfsDirPath( m_vfsRootForVolumes + GUCEF_VFS_RESERVED_STRING_VOLUMES_BY_PATH + CString( fsPath[ 0 ] ) );
        volumePath = ConformVfsDirPath( volumePath + fsPath.CutChars( 3 ) );
    }
    else
    {
        // The given path is not an absolute path so we need to make it one before we can map the drive letter to the VFS volume path
        CORE::CString absPath = CORE::CombinePath( CORE::CurrentWorkingDir(), fsPath );
        volumePath = ConformVfsDirPath( m_vfsRootForVolumes + GUCEF_VFS_RESERVED_STRING_VOLUMES_BY_PATH + CString( absPath[ 0 ] ) );
        volumePath = ConformVfsDirPath( volumePath + absPath.CutChars( 3 ) );
    }

    #else

    // Assume Linux convention of / prefix for absolute paths rooted at the root of the native filesystem
    if ( 1 < fsPath.Length() && ( fsPath[ 0 ] == '/' ) )
    {
        volumePath = ConformVfsDirPath( m_vfsRootForVolumes + GUCEF_VFS_RESERVED_STRING_VOLUMES_BY_PATH + fsPath );
    }
    else
    {
        // The given path is not an absolute path so we need to make it one
        volumePath = CORE::CombinePath( CORE::CurrentWorkingDir(), fsPath );
        volumePath = ConformVfsDirPath( m_vfsRootForVolumes + GUCEF_VFS_RESERVED_STRING_VOLUMES_BY_PATH + volumePath );
    }

    #endif

    // Now try again for the third time but with our adjusted volume routed path
    // This more more of a confirmation step since we already constructed what the VFS path 'would' be
    // We now want to verify that a mount exists for this VFS actually path, otherwise it doesnt count as a valid resolution
    i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr& mountEntry = (*i);

        if ( mountEntry->archiveType == CVFS::FileSystemArchiveTypeName )
        {
            CString shapedAbsPath = ConformVfsDirPath( mountEntry->abspath );

            // Check to see if the archive absolute path is the starting sub string of the location
            if ( volumePath.HasSubstr( shapedAbsPath, true ) == 0 )
            {
                // We need to make sure we dont accidentally match a partial directory name with a similarly named path
                // for example matching on "/example/foo" when the location is "/example/foo2/"
                // It needs to be the full directory name to count as a match
                if ( shapedAbsPath.Length() == volumePath.Length() ||
                    ( volumePath[ shapedAbsPath.Length()-1 ] == '/' ) )
                {
                    resolvedPath = volumePath;
                    return true;
                }
            }

            const CString::StringSet& aliases = mountEntry->mountPathAliases;
            CString::StringSet::iterator n = aliases.begin();
            while ( n != aliases.end() )
            {
                const CString& alias = (*n);
                if ( volumePath.HasSubstr( alias, true ) == 0 )
                {
                    if ( alias.Length() == volumePath.Length() ||
                        ( volumePath[ alias.Length()-1 ] == '/' ) )
                    {
                        resolvedPath = volumePath;
                        return true;
                    }
                }
                ++n;
            }
        }
        ++i;
    }

    if ( autoMountRoots )
    {
        // Its possible we did not have a mount YET for the given filesystem path because we are taking an on-demand approach vs everything at bootstrap
        // We can try to mount the filesystem path now as a lazy-load and then try again
        // @TODO
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::AddDirToWatch( const CString& dirToWatch       ,
                     const CDirWatchOptions& options ,
                     bool tryToCreatePathIfNotExists )
{GUCEF_TRACE;

    CString path = ConformVfsFilePath( dirToWatch );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Add the path as monitored for all relevant mounted archives
    bool totalSuccess = true;
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        TArchivePtr archiveToWatch = mountLink->mountEntry->archive;

        if ( !archiveToWatch->AddDirToWatch( mountLink->remainder, options, tryToCreatePathIfNotExists ) )
            totalSuccess = false;
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::RemoveDirToWatch( const CString& dirToWatch )
{GUCEF_TRACE;

    CString path = ConformVfsFilePath( dirToWatch );

    MT::CScopeReaderLock lock( m_rwdataLock );

    // Get a list of all eligable mounts
    TConstMountLinkVector mountLinks;
    GetEligableMounts( path, false, mountLinks );

    // Remove the path as monitored for all relevant mounted archives
    bool totalSuccess = true;
    TConstMountLinkVector::iterator i = mountLinks.begin();
    while ( i != mountLinks.end() )
    {
        TConstMountLinkPtr& mountLink = (*i);
        TArchivePtr archiveToWatch = mountLink->mountEntry->archive;

        if ( !archiveToWatch->RemoveDirToWatch( mountLink->remainder ) )
            totalSuccess = false;
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::RemoveAllWatches( void )
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    bool totalSuccess = true;
    TMountVector::iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        TMountEntryPtr mountEntry = (*i);
        if ( !mountEntry->archive->RemoveAllWatches() )
            totalSuccess = false;
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CVFS::GetAllWatchedDirs( CString::StringSet& dirs ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    bool totalSuccess = true;
    TMountVector::const_iterator i = m_mountList.begin();
    while ( i != m_mountList.end() )
    {
        const TMountEntryPtr mountEntry = (*i);

        CString::StringSet archiveDirs;
        if ( mountEntry->archive->GetAllWatchedDirs( archiveDirs ) )
        {
            CString::StringSet::const_iterator n = archiveDirs.begin();
            while ( n != archiveDirs.end() )
            {
                const CString& archiveDir = (*n);
                CString vfsPath = ConformVfsDirPath( CORE::CombinePath( mountEntry->mountPath, archiveDir ) );
                dirs.insert( vfsPath );
                ++n;
            }
        }
        else
            totalSuccess = false;
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CVFS::RegisterArchiveFactory( const CString& archiveType      ,
                              TArchiveFactory& archiveFactory )
{GUCEF_TRACE;

    m_abstractArchiveFactory.RegisterConcreteFactory( archiveType     ,
                                                      &archiveFactory );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Registered VFS archive factory for type \"" + archiveType + "\"" );
}

/*-------------------------------------------------------------------------*/

void
CVFS::UnregisterArchiveFactory( const CString& archiveType )
{GUCEF_TRACE;

    m_abstractArchiveFactory.UnregisterConcreteFactory( archiveType );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Unregistered VFS archive factory for type \"" + archiveType + "\"" );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CVFS::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwdataLock.WriterStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CVFS::Unlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwdataLock.WriterStop() );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CVFS::ReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwdataLock.ReaderStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CVFS::ReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwdataLock.ReaderStop() );
}

/*-------------------------------------------------------------------------*/

CVFS::MountEntry::MountEntry( void )
    : CORE::CTSharedObjCreator< MountEntry, MT::CNoLock >( this )
    , abspath()
    , path()
    , writeable( false )
    , archive()
    , archiveType()
    , mountPath()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVFS::ConstMountLink::ConstMountLink( const TMountEntryPtr& srcMountEntry )
    : CORE::CTSharedObjCreator< ConstMountLink, MT::CNoLock >( this )
    , mountEntry( srcMountEntry )
    , remainder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
