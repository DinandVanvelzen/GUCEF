/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_VFS_CVFSREMOTEPATH_H
#include "gucefVFS_CVfsRemotePath.h"
#define GUCEF_VFS_CVFSREMOTEPATH_H
#endif /* GUCEF_VFS_CVFSREMOTEPATH_H ? */

#include "gucefVFS_CVfsRemoteArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Ends the handles of the files of a remote archive. It is not the archive: a file can be open (and read on another thread) when
 *  the archive is unloaded and gone, and its handle must not need the archive to end.
 */
class CVfsRemoteHandleDestroyer : public CORE::CTDynamicDestructorBase< CVFSHandle >
{
    public:

    virtual void DestroyObject( CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        GUCEF_DELETE objectToBeDestroyed;
    }
};

static CVfsRemoteHandleDestroyer&
HandleDestroyer( void )
{GUCEF_TRACE;

    static CVfsRemoteHandleDestroyer destroyer;
    return destroyer;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsRemoteArchive::CVfsRemoteArchive( void )
    : CArchive()
    , m_core()
    , m_unloadedCore()
    , m_archiveName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsRemoteArchive::~CVfsRemoteArchive()
{GUCEF_TRACE;

    UnloadArchive();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsRemoteArchive::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString typeName = "GUCEF::VFS::CVfsRemoteArchive";
    return typeName;
}

/*-------------------------------------------------------------------------*/

CVfsRemoteArchiveCorePtr
CVfsRemoteArchive::GetCore( void ) const
{GUCEF_TRACE;

    return m_core;
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchive::AttachCore( const CVfsRemoteArchiveCorePtr& core, const CORE::CString& archiveName )
{GUCEF_TRACE;

    UnloadArchive();
    m_core = core;
    m_archiveName = archiveName;
    if ( !m_core.IsNULL() )
        SubscribeTo( m_core.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchive::OnNotify( CORE::CNotifier* notifier   ,
                             const CORE::CEvent& eventid ,
                             CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // the events of the core are the events of the archive, which the VFS passes on
    if ( CRemoteArchiveEvents::IsRemoteArchiveEvent( eventid ) )
        NotifyObservers( eventid, eventdata );
    else
        CArchive::OnNotify( notifier, eventid, eventdata );
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::UnloadArchive( void )
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    if ( !core.IsNULL() )
    {
        // no event is raised after this: the jobs are dropped and waited for first, then the subscription ends
        core->Shutdown();
        UnsubscribeFrom( core.GetPointerAlways() );

        // a call that made its way to the core before the unload (an asynchronous one runs without the lock of the VFS) can still be
        // using it: it is shut down, which makes every call on it fail, and it goes with the archive, which such a call holds
        m_unloadedCore = core;
    }
    m_core.Unlink();
    return true;
}

/*-------------------------------------------------------------------------*/

TBasicVfsResourcePtr
CVfsRemoteArchive::GetFile( const CString& file     ,
                            const char* mode        ,
                            const UInt32 memLoadSize ,
                            const bool overwrite     )
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    if ( core.IsNULL() || GUCEF_NULL == mode || 'r' != *mode || overwrite )
        return TBasicVfsResourcePtr();

    const CORE::CString path = CVfsRemotePath::Normalize( file );
    CORE::IOAccessPtr access = core->OpenFile( path );
    if ( access.IsNULL() )
        return TBasicVfsResourcePtr();
    return TVfsResourcePtr( GUCEF_NEW CVFSHandle( access, CVfsRemotePath::LeafOf( path ), path ), &HandleDestroyer() );
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::StoreAsFile( const CORE::CString& /* filepath */    ,
                                const CORE::CDynamicBuffer& /* data */ ,
                                const CORE::UInt64 /* offset */        ,
                                const bool /* overwrite */             )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchive::GetFileList( TStringVector& outputList             ,
                                const CString& mountLocation          ,
                                const CString& archiveLocation        ,
                                bool recursive                        ,
                                bool includePathInFilename            ,
                                const CString::StringSet& nameFilters ,
                                UInt32 maxListEntries                 ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    if ( !core.IsNULL() )
        core->GetFileList( outputList, mountLocation, archiveLocation, recursive, includePathInFilename, nameFilters, maxListEntries );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchive::GetDirList( TStringVector& outputList             ,
                               const CString& mountLocation          ,
                               const CString& archiveLocation        ,
                               bool recursive                        ,
                               bool includeParentDirInName           ,
                               const CString::StringSet& nameFilters ,
                               UInt32 maxListEntries                 ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    if ( !core.IsNULL() )
        core->GetDirList( outputList, mountLocation, archiveLocation, recursive, includeParentDirInName, nameFilters, maxListEntries );
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::DeleteFile( const CString& /* filePath */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::MoveFile( const CString& /* oldFilePath */ ,
                             const CString& /* newFilePath */ ,
                             const bool /* overwrite */       )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::FileExists( const CString& filePath ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    return !core.IsNULL() && core->FileExists( filePath );
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::DirExists( const CString& dirPath ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    return !core.IsNULL() && core->DirExists( dirPath );
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsRemoteArchive::GetFileSize( const CString& filePath ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    return core.IsNULL() ? 0 : core->GetFileSize( filePath );
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::GetFileMetaData( const CString& filePath           ,
                                    CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    return !core.IsNULL() && core->GetFileMetaData( filePath, metaData );
}

/*-------------------------------------------------------------------------*/

CString
CVfsRemoteArchive::GetFileHash( const CString& file ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    return core.IsNULL() ? CString() : core->GetFileHash( file );
}

/*-------------------------------------------------------------------------*/

CVfsDigest
CVfsRemoteArchive::GetFileDigest( const CString& file ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    return core.IsNULL() ? CVfsDigest() : core->GetFileDigest( file );
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CVfsRemoteArchive::GetFileModificationTime( const CString& filePath ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    return core.IsNULL() ? CORE::CDateTime() : core->GetFileModificationTime( filePath );
}

/*-------------------------------------------------------------------------*/

const CString&
CVfsRemoteArchive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::IsHealthy( void ) const
{GUCEF_TRACE;

    return !m_core.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchive::IsConnected( void ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    return !core.IsNULL() && core->IsLinkUp();
}

/*-------------------------------------------------------------------------*/

CIRemoteArchive*
CVfsRemoteArchive::GetRemoteArchiveInterface( void ) const
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core = m_core;
    return core.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchive::DestroyObject( CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    GUCEF_DELETE objectToBeDestroyed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
