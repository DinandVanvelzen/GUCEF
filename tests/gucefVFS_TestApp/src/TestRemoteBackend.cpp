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

#include "TestArchiveConformanceTree.h"

#include "TestRemoteBackend.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The source of a file of the test place at a revision */
class CTestBlockSource : public VFS::CIVfsBlockSource
{
    public:

    CTestBlockSource( CTestRemoteBackend* place, const CORE::CString& path, const CORE::CString& revision )
        : VFS::CIVfsBlockSource()
        , m_place( place )
        , m_path( path )
        , m_revision( revision )
    {GUCEF_TRACE;

    }

    virtual VFS::CVfsCallResult FetchRange( UInt64 offset, UInt32 length, void* buffer, UInt32& bytesRead ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_place->FetchFor( m_path, m_revision, offset, length, buffer, bytesRead );
    }

    private:

    CTestRemoteBackend* m_place;
    CORE::CString m_path;
    CORE::CString m_revision;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

CTestRemoteBackend::CTestRemoteBackend( void )
    : VFS::CIVfsRemoteBackend()
    , m_lock()
    , m_files()
    , m_dirs()
    , m_linkUp( true )
    , m_delayMs( 0 )
    , m_cutAfterBytes( 0 )
    , m_cutArmed( false )
    , m_corrupt( false )
    , m_hookAfterFetch( 0 )
    , m_hookPath()
    , m_listener()
    , m_bytesServed( 0 )
    , m_lastCallThread( 0 )
    , m_callsInDelay( 0 )
{GUCEF_TRACE;

    for ( int i=0; i<4; ++i )
    {
        m_failCount[ i ] = 0;
        m_failResult[ i ] = CORE::FILESYSTEMACCESS_FAILED;
        m_counts[ i ] = 0;
    }
}

/*-------------------------------------------------------------------------*/

CTestRemoteBackend::~CTestRemoteBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CTestRemoteBackend::ContentIdOf( const SFile& file )
{GUCEF_TRACE;

    return file.contentId + file.version * 977;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::FillFileBytes( const SFile& file, UInt64 offset, void* dest, UInt32 count )
{GUCEF_TRACE;

    if ( file.explicitBytes )
    {
        if ( count > 0 )
            memcpy( dest, &file.bytes[ (size_t) offset ], count );
        return;
    }
    CArchiveConformanceTree::FillContent( ContentIdOf( file ), offset, dest, count );
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::PutFile( const CORE::CString& path, const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TFileMap::iterator i = m_files.find( path );
    const bool existed = i != m_files.end();
    SFile& file = m_files[ path ];
    if ( existed )
        ++file.version;
    file.explicitBytes = true;
    file.size = content.GetDataSize();
    file.bytes.assign( static_cast< const UInt8* >( content.GetConstBufferPtr() ), static_cast< const UInt8* >( content.GetConstBufferPtr() ) + content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestRemoteBackend::RevisionOf( const SFile& file ) const
{GUCEF_TRACE;

    return CORE::ToString( file.size ) + "-" + CORE::ToString( file.version );
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::AddFile( const CORE::CString& path, UInt32 contentId, UInt64 size )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    SFile& file = m_files[ path ];
    file.contentId = contentId;
    file.size = size;
    file.version = 0;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::AddDirectory( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_dirs.insert( path );
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_files.clear();
    m_dirs.clear();
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::RemoveFile( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_files.erase( path );
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::ChangeFile( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TFileMap::iterator i = m_files.find( path );
    if ( i != m_files.end() )
        ++i->second.version;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::ChangeFileSize( const CORE::CString& path, UInt64 newSize )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TFileMap::iterator i = m_files.find( path );
    if ( i != m_files.end() )
    {
        ++i->second.version;
        i->second.size = newSize;
    }
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::ProvideDigest( const CORE::CString& path, VFS::CVfsDigest::EAlgorithm algorithm )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TFileMap::iterator i = m_files.find( path );
    if ( i != m_files.end() )
    {
        i->second.provideDigest = true;
        i->second.wrongDigest = false;
        i->second.digestAlgorithm = algorithm;
    }
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::ProvideWrongDigest( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TFileMap::iterator i = m_files.find( path );
    if ( i != m_files.end() )
    {
        i->second.provideDigest = true;
        i->second.wrongDigest = true;
    }
}

/*-------------------------------------------------------------------------*/

VFS::CVfsDigest
CTestRemoteBackend::ComputeDigest( const CORE::CString& path, VFS::CVfsDigest::EAlgorithm algorithm ) const
{GUCEF_TRACE;

    SFile file;
    {
        MT::CScopeMutex lock( m_lock );
        TFileMap::const_iterator i = m_files.find( path );
        if ( i == m_files.end() )
            return VFS::CVfsDigest();
        file = i->second;
    }
    return DigestOfFile( file, algorithm );
}

/*-------------------------------------------------------------------------*/

VFS::CVfsDigest
CTestRemoteBackend::DigestOfFile( const SFile& file, VFS::CVfsDigest::EAlgorithm algorithm )
{GUCEF_TRACE;

    VFS::CVfsDigestHasher hasher( algorithm );
    CORE::CDynamicBuffer chunk( 65536, true );
    UInt64 position = 0;
    while ( position < file.size )
    {
        const UInt32 count = (UInt32) ( file.size - position < 65536 ? file.size - position : 65536 );
        FillFileBytes( file, position, chunk.GetBufferPtr(), count );
        hasher.Update( chunk.GetConstBufferPtr(), count );
        position += count;
    }
    return hasher.Final();
}

/*-------------------------------------------------------------------------*/

bool
CTestRemoteBackend::FillExpected( const CORE::CString& path, UInt64 offset, void* dest, UInt32 count ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TFileMap::const_iterator i = m_files.find( path );
    if ( i == m_files.end() || offset + count > i->second.size )
        return false;
    FillFileBytes( i->second, offset, dest, count );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestRemoteBackend::CheckExpected( const CORE::CString& path, UInt64 offset, const void* data, UInt32 count ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TFileMap::const_iterator i = m_files.find( path );
    if ( i == m_files.end() || offset + count > i->second.size )
        return false;
    GUCEF::vector< UInt8 > expected( count > 0 ? count : 1 );
    FillFileBytes( i->second, offset, &expected[ 0 ], count );
    return 0 == memcmp( &expected[ 0 ], data, count );
}

/*-------------------------------------------------------------------------*/

UInt64
CTestRemoteBackend::GetSize( const CORE::CString& path ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TFileMap::const_iterator i = m_files.find( path );
    return i == m_files.end() ? 0 : i->second.size;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestRemoteBackend::GetRevision( const CORE::CString& path ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TFileMap::const_iterator i = m_files.find( path );
    return i == m_files.end() ? CORE::CString() : RevisionOf( i->second );
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::SetLinkUp( bool linkUp )
{GUCEF_TRACE;

    VFS::CVfsRemoteArchiveCorePtr listener;
    {
        MT::CScopeMutex lock( m_lock );
        m_linkUp = linkUp;
        listener = m_listener.GetSharedPtr();
    }
    if ( !listener.IsNULL() )
        listener->NotifyLinkState( linkUp );
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::SetDelayMs( UInt32 delayMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_delayMs = delayMs;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::FailNext( EOperation operation, UInt32 count, CORE::TFileSystemAccessResult result )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_failCount[ operation ] = count;
    m_failResult[ operation ] = result;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::CutNextFetchAfter( UInt64 bytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_cutAfterBytes = bytes;
    m_cutArmed = true;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::SetCorruptContent( bool corrupt )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_corrupt = corrupt;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::SetLinkListener( const CORE::CTWeakPtr< VFS::CVfsRemoteArchiveCore, MT::CMutex >& listener )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_listener = listener;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::SetOnFetchHook( UInt32 afterFetchNumber, const CORE::CString& pathToChange )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_hookAfterFetch = afterFetchNumber;
    m_hookPath = pathToChange;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestRemoteBackend::GetStatCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_counts[ OP_STAT ];
}

/*-------------------------------------------------------------------------*/

UInt32
CTestRemoteBackend::GetListCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_counts[ OP_LIST ];
}

/*-------------------------------------------------------------------------*/

UInt32
CTestRemoteBackend::GetOpenCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_counts[ OP_OPEN ];
}

/*-------------------------------------------------------------------------*/

UInt32
CTestRemoteBackend::GetFetchCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_counts[ OP_FETCH ];
}

/*-------------------------------------------------------------------------*/

UInt64
CTestRemoteBackend::GetBytesServed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_bytesServed;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::ResetCounters( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( int i=0; i<4; ++i )
        m_counts[ i ] = 0;
    m_bytesServed = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestRemoteBackend::GetCallsInDelay( void ) const
{GUCEF_TRACE;

    return m_callsInDelay.Load();
}

/*-------------------------------------------------------------------------*/

bool
CTestRemoteBackend::IsCoreAlive( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_listener.GetSharedPtr().IsNULL();
}

/*-------------------------------------------------------------------------*/

TThreadId
CTestRemoteBackend::GetLastCallThreadId( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastCallThread;
}

/*-------------------------------------------------------------------------*/

bool
CTestRemoteBackend::IsLinkUp( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_linkUp;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestRemoteBackend::GetPlaceDescription( void ) const
{GUCEF_TRACE;

    return "the share testshare of testmember";
}

/*-------------------------------------------------------------------------*/

VFS::CVfsCallResult
CTestRemoteBackend::BeginCall( EOperation operation, bool& linkWasUp )
{GUCEF_TRACE;

    UInt32 delayMs = 0;
    VFS::CVfsCallResult injected;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_counts[ operation ];
        m_lastCallThread = MT::GetCurrentThreadID();
        delayMs = m_delayMs;
        linkWasUp = m_linkUp;
        if ( m_failCount[ operation ] > 0 )
        {
            --m_failCount[ operation ];
            injected = VFS::CVfsCallResult( m_failResult[ operation ], "injected failure" );
        }
    }
    if ( delayMs > 0 )
    {
        m_callsInDelay.FetchAdd( 1 );
        MT::ThreadDelay( delayMs );
        m_callsInDelay.FetchSub( 1 );
    }
    if ( !injected.IsSuccess() )
        return injected;
    if ( !linkWasUp || !IsLinkUp() )
        return VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, "the link is cut" );
    return VFS::CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteBackend::EntryOf( const CORE::CString& path, const SFile& file, VFS::CVfsRemoteEntry& entry ) const
{GUCEF_TRACE;

    entry = VFS::CVfsRemoteEntry();
    entry.name = VFS::CVfsRemotePath::LeafOf( path );
    entry.exists = true;
    entry.isDirectory = false;
    entry.size = file.size;
    entry.revision = RevisionOf( file );
    entry.modifiedUtcMs = 1700000000000ULL + file.version * 1000;
    if ( file.provideDigest )
    {
        if ( file.wrongDigest )
        {
            entry.digest = VFS::CVfsDigest( VFS::CVfsDigest::ALGORITHM_SHA256, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" );
        }
        else
        {
            // the digest of the content, made when asked: the files of the tests are small. The caller does not hold the lock
            entry.digest = DigestOfFile( file, file.digestAlgorithm );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CTestRemoteBackend::DirectoryExistsLocked( const CORE::CString& dirPath ) const
{GUCEF_TRACE;

    if ( dirPath.IsNULLOrEmpty() )
        return true;
    if ( m_dirs.find( dirPath ) != m_dirs.end() )
        return true;
    for ( TFileMap::const_iterator i = m_files.begin(); i != m_files.end(); ++i )
    {
        if ( VFS::CVfsRemotePath::IsSameOrBelow( dirPath, i->first ) && i->first != dirPath )
            return true;
    }
    for ( TDirSet::const_iterator d = m_dirs.begin(); d != m_dirs.end(); ++d )
    {
        if ( VFS::CVfsRemotePath::IsSameOrBelow( dirPath, *d ) && *d != dirPath )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

VFS::CVfsCallResult
CTestRemoteBackend::Stat( const CORE::CString& path, VFS::CVfsRemoteEntry& entry )
{GUCEF_TRACE;

    bool linkWasUp = false;
    const VFS::CVfsCallResult began = BeginCall( OP_STAT, linkWasUp );
    if ( !began.IsSuccess() )
        return began;

    SFile file;
    bool isFile = false;
    bool isDir = false;
    {
        MT::CScopeMutex lock( m_lock );
        TFileMap::const_iterator i = m_files.find( path );
        if ( i != m_files.end() )
        {
            isFile = true;
            file = i->second;
        }
        else
        {
            isDir = DirectoryExistsLocked( path );
        }
    }

    if ( isFile )
    {
        EntryOf( path, file, entry );
        return VFS::CVfsCallResult();
    }
    entry = VFS::CVfsRemoteEntry();
    entry.name = VFS::CVfsRemotePath::LeafOf( path );
    entry.exists = isDir;
    entry.isDirectory = isDir;
    return VFS::CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

VFS::CVfsCallResult
CTestRemoteBackend::List( const CORE::CString& dirPath, VFS::CVfsRemoteEntry::TVector& entries, bool& dirExists )
{GUCEF_TRACE;

    bool linkWasUp = false;
    const VFS::CVfsCallResult began = BeginCall( OP_LIST, linkWasUp );
    if ( !began.IsSuccess() )
        return began;

    entries.clear();
    CORE::CString::StringSet dirNames;
    GUCEF::vector< CORE::CString > filePaths;
    GUCEF::vector< SFile > files;
    {
    MT::CScopeMutex lock( m_lock );
    dirExists = DirectoryExistsLocked( dirPath );
    if ( !dirExists )
        return VFS::CVfsCallResult();

    for ( TFileMap::const_iterator i = m_files.begin(); i != m_files.end(); ++i )
    {
        if ( !VFS::CVfsRemotePath::IsSameOrBelow( dirPath, i->first ) || i->first == dirPath )
            continue;
        const CORE::CString relative = dirPath.IsNULLOrEmpty() ? i->first : CORE::CString( i->first.C_String() + VFS::CVfsRemotePath::GetByteCount( dirPath ) + 1, VFS::CVfsRemotePath::GetByteCount( i->first ) - VFS::CVfsRemotePath::GetByteCount( dirPath ) - 1 );
        const CORE::CString parent = VFS::CVfsRemotePath::ParentOf( relative );
        if ( parent.IsNULLOrEmpty() )
        {
            filePaths.push_back( i->first );
            files.push_back( i->second );
        }
        else
        {
            const CORE::CString::StringVector parts = relative.ParseElements( '/', false );
            if ( !parts.empty() )
                dirNames.insert( parts[ 0 ] );
        }
    }
    for ( TDirSet::const_iterator d = m_dirs.begin(); d != m_dirs.end(); ++d )
    {
        if ( !VFS::CVfsRemotePath::IsSameOrBelow( dirPath, *d ) || *d == dirPath )
            continue;
        const CORE::CString relative = dirPath.IsNULLOrEmpty() ? *d : CORE::CString( d->C_String() + VFS::CVfsRemotePath::GetByteCount( dirPath ) + 1, VFS::CVfsRemotePath::GetByteCount( *d ) - VFS::CVfsRemotePath::GetByteCount( dirPath ) - 1 );
        const CORE::CString::StringVector parts = relative.ParseElements( '/', false );
        if ( !parts.empty() )
            dirNames.insert( parts[ 0 ] );
    }
    }

    // the digests are made without the lock
    for ( size_t f=0; f<files.size(); ++f )
    {
        VFS::CVfsRemoteEntry fileEntry;
        EntryOf( filePaths[ f ], files[ f ], fileEntry );
        entries.push_back( fileEntry );
    }
    for ( CORE::CString::StringSet::const_iterator n = dirNames.begin(); n != dirNames.end(); ++n )
    {
        VFS::CVfsRemoteEntry entry;
        entry.name = (*n);
        entry.exists = true;
        entry.isDirectory = true;
        entries.push_back( entry );
    }
    return VFS::CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

VFS::CIVfsBlockSourcePtr
CTestRemoteBackend::OpenBlockSource( const CORE::CString& path, const CORE::CString& revision, VFS::CVfsCallResult& result )
{GUCEF_TRACE;

    bool linkWasUp = false;
    result = BeginCall( OP_OPEN, linkWasUp );
    if ( !result.IsSuccess() )
        return VFS::CIVfsBlockSourcePtr();

    {
        MT::CScopeMutex lock( m_lock );
        TFileMap::const_iterator i = m_files.find( path );
        if ( i == m_files.end() )
        {
            result = VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_FOUND, "no such file" );
            return VFS::CIVfsBlockSourcePtr();
        }
        if ( RevisionOf( i->second ) != revision )
        {
            result = VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED, "the file has another revision" );
            return VFS::CIVfsBlockSourcePtr();
        }
    }
    return VFS::CIVfsBlockSourcePtr( GUCEF_NEW CTestBlockSource( this, path, revision ) );
}

/*-------------------------------------------------------------------------*/

VFS::CVfsCallResult
CTestRemoteBackend::FetchFor( const CORE::CString& path, const CORE::CString& revision, UInt64 offset, UInt32 length, void* buffer, UInt32& bytesRead )
{GUCEF_TRACE;

    bytesRead = 0;
    bool linkWasUp = false;
    const VFS::CVfsCallResult began = BeginCall( OP_FETCH, linkWasUp );
    if ( !began.IsSuccess() )
        return began;

    UInt32 deliver = length;
    bool cut = false;
    CORE::CString hookPath;
    {
        MT::CScopeMutex lock( m_lock );
        TFileMap::const_iterator i = m_files.find( path );
        if ( i == m_files.end() )
            return VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED, "the file is gone" );
        if ( RevisionOf( i->second ) != revision )
            return VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED, "the file has another revision" );
        if ( offset >= i->second.size )
            return VFS::CVfsCallResult();
        if ( offset + deliver > i->second.size )
            deliver = (UInt32) ( i->second.size - offset );

        if ( m_cutArmed )
        {
            m_cutArmed = false;
            if ( m_cutAfterBytes < deliver )
            {
                deliver = (UInt32) m_cutAfterBytes;
                cut = true;
            }
        }
        FillFileBytes( i->second, offset, buffer, deliver );
        if ( m_corrupt && deliver > 0 )
            static_cast< UInt8* >( buffer )[ 0 ] = (UInt8) ~static_cast< UInt8* >( buffer )[ 0 ];
        m_bytesServed += deliver;
        if ( m_hookAfterFetch > 0 && m_counts[ OP_FETCH ] == m_hookAfterFetch )
            hookPath = m_hookPath;
    }
    bytesRead = deliver;

    if ( !hookPath.IsNULLOrEmpty() )
        ChangeFile( hookPath );

    if ( cut )
        return VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, "the link dropped in the middle of the run" );
    return VFS::CVfsCallResult();
}

/*-------------------------------------------------------------------------*/
