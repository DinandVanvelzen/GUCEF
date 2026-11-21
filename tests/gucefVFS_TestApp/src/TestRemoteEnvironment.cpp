/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "TestRemoteEnvironment.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestEventRecorder::SRecord::SRecord( void )
    : eventName()
    , path()
    , revision()
    , oldRevision()
    , firstByte( 0 )
    , byteCount( 0 )
    , fileComplete( false )
    , result( CORE::FILESYSTEMACCESS_SUCCESS )
    , message()
    , jobKind( VFS::FETCHJOB_STAT )
    , mountPath()
    , connected( false )
    , thread( 0 )
    , sequence( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTestEventRecorder::CTestEventRecorder( void )
    : CORE::CObservingNotifier()
    , m_lock()
    , m_records()
    , m_sequence( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTestEventRecorder::~CTestEventRecorder()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CTestEventRecorder::Listen( CORE::CNotifier* notifier )
{GUCEF_TRACE;

    SubscribeTo( notifier );
}

/*-------------------------------------------------------------------------*/

void
CTestEventRecorder::OnNotify( CORE::CNotifier* /* notifier */ ,
                              const CORE::CEvent& eventid     ,
                              CORE::CICloneable* eventdata    )
{GUCEF_TRACE;

    if ( !VFS::CRemoteArchiveEvents::IsRemoteArchiveEvent( eventid ) )
        return;

    SRecord record;
    record.eventName = eventid.GetName();
    record.thread = MT::GetCurrentThreadID();

    if ( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent == eventid )
    {
        // the recorder listens to the VFS, which passes the state on with the mount in it
        VFS::CRemoteArchiveEvents::TConnectionStateChangedVfsEventData* vfsState = static_cast< VFS::CRemoteArchiveEvents::TConnectionStateChangedVfsEventData* >( eventdata );
        if ( GUCEF_NULL != vfsState )
        {
            record.mountPath = vfsState->GetData().mountPath;
            record.connected = vfsState->GetData().connected;
        }
    }
    else
    {
        VFS::CVfsArrivalInfo* info = static_cast< VFS::CVfsArrivalInfo* >( eventdata );
        if ( GUCEF_NULL != info )
        {
            record.path = info->path;
            record.revision = info->revision;
            record.oldRevision = info->oldRevision;
            record.firstByte = info->firstByte;
            record.byteCount = info->byteCount;
            record.fileComplete = info->fileComplete;
            record.result = info->result;
            record.message = info->message;
            record.jobKind = info->jobKind;
        }
    }

    MT::CScopeMutex lock( m_lock );
    record.sequence = ++m_sequence;
    m_records.push_back( record );
}

/*-------------------------------------------------------------------------*/

UInt32
CTestEventRecorder::Count( const CORE::CEvent& eventId ) const
{GUCEF_TRACE;

    return (UInt32) Find( eventId ).size();
}

/*-------------------------------------------------------------------------*/

UInt32
CTestEventRecorder::Count( const CORE::CEvent& eventId, const CORE::CString& path ) const
{GUCEF_TRACE;

    return (UInt32) Find( eventId, path ).size();
}

/*-------------------------------------------------------------------------*/

CTestEventRecorder::TRecordVector
CTestEventRecorder::Find( const CORE::CEvent& eventId, const CORE::CString& path ) const
{GUCEF_TRACE;

    TRecordVector found;
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_records.size(); ++i )
    {
        if ( m_records[ i ].eventName == eventId.GetName() && ( path.IsNULLOrEmpty() || m_records[ i ].path == path ) )
            found.push_back( m_records[ i ] );
    }
    return found;
}

/*-------------------------------------------------------------------------*/

CTestEventRecorder::TRecordVector
CTestEventRecorder::GetAll( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_records;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestEventRecorder::GetTotal( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_records.size();
}

/*-------------------------------------------------------------------------*/

void
CTestEventRecorder::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_records.clear();
}

/*-------------------------------------------------------------------------*/

SRemoteEnvOptions::SRemoteEnvOptions( void )
    : metadataTtlMs( 3000 )
    , blockBytes( 65536 )
    , cacheMaxBytes( 0 )
    , useCache( true )
    , maxRunning( 4 )
    , maxQueued( 64 )
    , readAheadAfterBlocks( 2 )
    , readAheadBlocks( 8 )
    , maxBlocksPerFetch( 16 )
    , offlineListings( true )
    , verifyCompleteFiles( true )
    , refetchPinned( true )
    , maxMetadataEntries( 4096 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const char* const CRemoteEnv::ReadmeFile = "docs/readme.txt";
const char* const CRemoteEnv::NotesFile = "docs/notes/a.bin";
const char* const CRemoteEnv::BigFile = "data/big.bin";
const char* const CRemoteEnv::MidFile = "data/mid.bin";
const char* const CRemoteEnv::EmptyFile = "empty.bin";
const char* const CRemoteEnv::TopFile = "top.txt";
const char* const CRemoteEnv::DeepFile = "data/sub/deep/x.bin";
const char* const CRemoteEnv::MountPath = "/remote/";
const char* const CRemoteEnv::ArchiveName = "remotetest";

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteEnv::GetPlaceText( void ) const
{GUCEF_TRACE;

    return "testshare of testmember";
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CRemoteEnv::GetCutFetchResult( void ) const
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteEnv::HasHandleStatistics( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

VFS::CVFS&
CRemoteEnv::Vfs( void )
{GUCEF_TRACE;

    return VFS::CVfsGlobal::Instance()->GetVfs();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteEnv::VfsPath( const CORE::CString& relative ) const
{GUCEF_TRACE;

    return CORE::CString( MountPath ) + relative;
}

/*-------------------------------------------------------------------------*/

void
CRemoteEnv::Pump( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteEnv::WaitForEvents( const CORE::CEvent& eventId, const CORE::CString& relativePath, UInt32 atLeast, UInt32 timeoutMs )
{GUCEF_TRACE;

    const CORE::CString vfsPath = relativePath.IsNULLOrEmpty() ? CORE::CString::Empty : VfsPath( relativePath );
    for ( UInt32 waited=0; waited<=timeoutMs; ++waited )
    {
        Pump();
        if ( Events().Count( eventId, vfsPath ) >= atLeast )
            return true;
        MT::ThreadDelay( 1 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteEnv::WaitUntilIdle( UInt32 timeoutMs )
{GUCEF_TRACE;

    VFS::CVfsFetchScheduler& scheduler = Scheduler();
    for ( UInt32 waited=0; waited<=timeoutMs; ++waited )
    {
        Pump();
        if ( 0 == scheduler.GetRunningCount() && 0 == scheduler.GetQueuedCount() && scheduler.WaitUntilIdle( 0 ) )
        {
            Pump();
            return true;
        }
        MT::ThreadDelay( 1 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteEnv::ReadAll( const CORE::CString& relativePath, CORE::CDynamicBuffer& content, VFS::CVfsCallResult* readResult )
{GUCEF_TRACE;

    content.Clear( false );
    VFS::TBasicVfsResourcePtr resource = Vfs().GetFile( VfsPath( relativePath ) );
    if ( resource.IsNULL() )
    {
        if ( GUCEF_NULL != readResult )
            Vfs().GetLastResult( *readResult );
        return false;
    }

    CORE::IOAccessPtr access = resource->GetAccess();
    const UInt64 size = access->GetSize();
    content.SetDataSize( (UInt32) size );
    const UInt32 got = access->Read( content.GetBufferPtr(), 1, (UInt32) size );
    content.SetDataSize( got );
    if ( GUCEF_NULL != readResult )
    {
        VFS::CVfsCachedIOAccess* cached = static_cast< VFS::CVfsCachedIOAccess* >( access.GetPointerAlways() );
        *readResult = cached->GetLastResult();
    }
    return got == size;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteEnv::Matches( const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& content, UInt32 count )
{GUCEF_TRACE;

    return count <= content.GetDataSize() && Backend().CheckExpected( relativePath, offset, content.GetConstBufferPtr(), count );
}

/*-------------------------------------------------------------------------*/
