/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
 *
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CBACKOFFPOLICY_H
#include "gucefCORE_CBackoffPolicy.h"
#define GUCEF_CORE_CBACKOFFPOLICY_H
#endif /* GUCEF_CORE_CBACKOFFPOLICY_H ? */

#ifndef GUCEF_CORE_CDATATABLEPARTITIONSPEC_H
#include "gucefCORE_CDataTablePartitionSpec.h"
#define GUCEF_CORE_CDATATABLEPARTITIONSPEC_H
#endif /* GUCEF_CORE_CDATATABLEPARTITIONSPEC_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "dbpluginVFSTABLES_CVfsTablesStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CVfsTablesStore::TablesDirName = "_tables";
const CORE::CString CVfsTablesStore::TemporaryFileMarker = ".tmp-";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The directory part and the name of a VFS path */
void
SplitPath( const CORE::CString& path, CORE::CString& dir, CORE::CString& name )
{
    const Int32 slash = path.HasChar( '/', false );
    if ( slash < 0 )
    {
        dir.Clear();
        name = path;
        return;
    }
    dir = path.SubstrToIndex( (UInt32) slash, true );
    name = path.CutChars( (UInt32) slash + 1, true );
}

/** Whether the text after the marker is "<writerId>-<digits>" for the writer id */
bool
IsTemporaryOf( const CORE::CString& name, const CORE::CString& escapedWriterId )
{
    const CORE::CString infix = CVfsTablesStore::TemporaryFileMarker + escapedWriterId + "-";
    const Int32 at = name.HasSubstr( infix, false );
    if ( at <= 0 )
        return false;
    const CORE::CString counter = name.CutChars( (UInt32) at + infix.Length(), true );
    if ( counter.IsNULLOrEmpty() )
        return false;
    CORE::CUtf8String digits = CORE::ToUtf8String( counter );
    for ( UInt32 i=0; i+1<digits.ByteSize(); ++i )
    {
        const char c = digits.C_String()[ i ];
        if ( c < '0' || c > '9' )
            return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsTablesStore::CVfsTablesStore( const CORE::CString& root              ,
                                  bool readOnly                          ,
                                  const CVfsTablesSettings& settings     ,
                                  UInt64 bufferPoolMaxBytes              ,
                                  const CORE::ThreadPoolPtr& threadPool  )
    : m_root( root.ReplaceChar( '\\', '/' ) )
    , m_readOnly( readOnly )
    , m_settings( settings )
    , m_escapedWriterId( CORE::CDataTablePartitionSpec::EscapeSegmentValue( settings.writerId ) )
    , m_bufferPool( bufferPoolMaxBytes )
    , m_threadPool( threadPool )
    , m_temporaryCounter( 0 )
    , m_replaceLock()
    , m_generation( 0 )
    , m_listingLock()
    , m_listings()
    , m_tickResolution( MT::PrecisionTimerResolution() )
    , m_footerLock()
    , m_footers()
    , m_footerUse()
    , m_footerBytes( 0 )
    , m_footerUseCounter( 0 )
    , m_writeLocksLock()
    , m_writeLocks()
    , m_statsLock()
    , m_stats()
{GUCEF_TRACE;

    while ( m_root.Length() > 1 && m_root.HasChar( '/', false ) == (Int32) m_root.Length() - 1 )
        m_root = m_root.CutChars( 1, false );
    if ( 0 == m_tickResolution )
        m_tickResolution = 1000;
}

/*-------------------------------------------------------------------------*/

CVfsTablesStore::~CVfsTablesStore()
{GUCEF_TRACE;

    ClearCaches();
    for ( TWriteLockMap::iterator i = m_writeLocks.begin(); i != m_writeLocks.end(); ++i )
        GUCEF_DELETE (*i).second;
    m_writeLocks.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsTablesStore::GetRoot( void ) const
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

bool
CVfsTablesStore::IsReadOnly( void ) const
{GUCEF_TRACE;

    return m_readOnly;
}

/*-------------------------------------------------------------------------*/

const CVfsTablesSettings&
CVfsTablesStore::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableBufferPool&
CVfsTablesStore::GetBufferPool( void )
{GUCEF_TRACE;

    return m_bufferPool;
}

/*-------------------------------------------------------------------------*/

CORE::ThreadPoolPtr
CVfsTablesStore::GetThreadPool( void ) const
{GUCEF_TRACE;

    return m_threadPool;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsTablesStore::MakePath( const CORE::CString& relativePath ) const
{GUCEF_TRACE;

    if ( m_root.IsNULLOrEmpty() )
        return relativePath;
    if ( relativePath.IsNULLOrEmpty() )
        return m_root;
    return m_root + "/" + relativePath;
}

/*-------------------------------------------------------------------------*/

bool
CVfsTablesStore::IsListingExpired( const SListing& listing, UInt64 nowTick ) const
{GUCEF_TRACE;

    if ( 0 == m_settings.listingCacheTtlMs )
        return false;
    const UInt64 ageTicks = nowTick >= listing.listedAtTick ? nowTick - listing.listedAtTick : 0;
    return ageTicks * 1000 >= (UInt64) m_settings.listingCacheTtlMs * m_tickResolution;
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesStore::ListDirectory( const CORE::CString& dirPath, bool directories, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    names.clear();
    const CORE::CString key = dirPath + ( directories ? "|d" : "|f" );
    {
        MT::CScopeMutex lock( m_listingLock );
        TListingMap::iterator i = m_listings.find( key );
        if ( i != m_listings.end() )
        {
            if ( !IsListingExpired( (*i).second, MT::PrecisionTickCount() ) )
            {
                names.reserve( (*i).second.names.size() );
                for ( CORE::CString::StringSet::const_iterator n = (*i).second.names.begin(); n != (*i).second.names.end(); ++n )
                    names.push_back( (*n) );
                lock.EarlyUnlock();
                MT::CScopeMutex statsLock( m_statsLock );
                ++m_stats.listingCacheHits;
                return;
            }
            m_listings.erase( i );
        }
    }

    // a listing is only kept when no file changed while it was made, else it could miss a file put in place meanwhile
    UInt64 generationBefore = 0;
    {
        MT::CScopeMutex lock( m_replaceLock );
        generationBefore = m_generation;
    }
    const UInt64 listedAt = MT::PrecisionTickCount();

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TStringVector list;
    if ( directories )
        vfs.GetDirList( list, dirPath, false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
    else
        vfs.GetFileList( list, dirPath, false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );

    SListing listing;
    listing.listedAtTick = listedAt;
    for ( VFS::CVFS::TStringVector::const_iterator i = list.begin(); i != list.end(); ++i )
    {
        CORE::CString name = (*i);
        while ( !name.IsNULLOrEmpty() && name.HasChar( '/', false ) == (Int32) name.Length() - 1 )
            name = name.CutChars( 1, false );
        if ( !name.IsNULLOrEmpty() )
            listing.names.insert( name );
    }
    names.reserve( listing.names.size() );
    for ( CORE::CString::StringSet::const_iterator n = listing.names.begin(); n != listing.names.end(); ++n )
        names.push_back( (*n) );

    {
        MT::CScopeMutex replaceLock( m_replaceLock );
        if ( generationBefore == m_generation )
        {
            MT::CScopeMutex lock( m_listingLock );
            m_listings[ key ] = listing;
        }
    }
    MT::CScopeMutex statsLock( m_statsLock );
    ++m_stats.directoryListings;
}

/*-------------------------------------------------------------------------*/

bool
CVfsTablesStore::FileExists( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString dir;
    CORE::CString name;
    SplitPath( path, dir, name );
    {
        MT::CScopeMutex lock( m_listingLock );
        TListingMap::const_iterator i = m_listings.find( dir + "|f" );
        if ( i != m_listings.end() && !IsListingExpired( (*i).second, MT::PrecisionTickCount() ) )
            return (*i).second.names.find( name ) != (*i).second.names.end();
    }
    {
        MT::CScopeMutex statsLock( m_statsLock );
        ++m_stats.fileProbes;
    }
    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( path );
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesStore::PutFooterInCache( const CORE::CString& path, const CORE::CITableFileMetaDataPtr& metaData, UInt64 size )
{GUCEF_TRACE;

    const UInt64 bytes = metaData->GetMemoryFootprint() + path.ByteSize() + 64;
    if ( bytes > m_settings.footerCacheBytes )
        return;

    UInt64 evictions = 0;
    {
        MT::CScopeMutex lock( m_footerLock );
        TFooterMap::iterator existing = m_footers.find( path );
        if ( existing != m_footers.end() )
        {
            m_footerBytes -= (*existing).second.bytes;
            m_footerUse.erase( (*existing).second.lastUse );
            m_footers.erase( existing );
        }
        while ( m_footerBytes + bytes > m_settings.footerCacheBytes && !m_footerUse.empty() )
        {
            // the least recently used goes first
            TFooterUseMap::iterator oldest = m_footerUse.begin();
            TFooterMap::iterator victim = m_footers.find( (*oldest).second );
            if ( victim != m_footers.end() )
            {
                m_footerBytes -= (*victim).second.bytes;
                m_footers.erase( victim );
            }
            m_footerUse.erase( oldest );
            ++evictions;
        }
        SFooterEntry& entry = m_footers[ path ];
        entry.metaData = metaData;
        entry.size = size;
        entry.bytes = bytes;
        entry.lastUse = ++m_footerUseCounter;
        m_footerUse[ entry.lastUse ] = path;
        m_footerBytes += bytes;
    }
    if ( evictions > 0 )
    {
        MT::CScopeMutex statsLock( m_statsLock );
        m_stats.footerCacheEvictions += evictions;
    }
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesStore::OpenSharedFile( const CORE::CString& path                              ,
                                 const TTableCodecPtr& codec                            ,
                                 const CORE::CValueList& codecOptions                   ,
                                 TDataTableSharedFilePtr& file                          ,
                                 bool& metaDataFromCache                                ,
                                 CORE::CString& error                                   ,
                                 UInt64 requiredGeneration                              ,
                                 const CORE::CITableFileMetaDataPtr& requiredMetaData   )
{GUCEF_TRACE;

    file.Unlink();
    metaDataFromCache = false;
    if ( codec.IsNULL() )
    {
        error = "No table codec to read \"" + path + "\"";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }

    TDataTableSharedFilePtr opened( GUCEF_NEW CDataTableSharedFile() );
    opened->path = path;
    // the replace generation before and after the open: when it did not change, no file was replaced or deleted by
    // this process in between, so the file opened is the one at the path and a footer in the cache describes it
    {
        MT::CScopeMutex lock( m_replaceLock );
        opened->generation = m_generation;
    }
    if ( GUCEF_UINT64MAX != requiredGeneration && requiredGeneration != opened->generation )
    {
        error = "File \"" + path + "\" may have been replaced since the scan planned it";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }

    // one lookup of the mounts per open: the existence is only asked when the open fails
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    opened->resource = vfs.GetFile( path, "rb", false );
    if ( opened->resource.IsNULL() || opened->resource->GetAccess().IsNULL() )
    {
        if ( !vfs.FileExists( path ) )
        {
            error = "File \"" + path + "\" does not exist";
            return CORE::DATATABLE_STATUS_NOT_FOUND;
        }
        error = "File \"" + path + "\" cannot be opened for reading";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    opened->access = opened->resource->GetAccess();

    // the size of the file that is open (GetSize() of an IO access is that of its stream, not of a path a replace moved on)
    opened->size = opened->access->GetSize();
    if ( 0 != opened->access->Setpos( 0 ) )
    {
        error = "File \"" + path + "\" cannot be positioned";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }

    CORE::CITableFileMetaDataPtr cached;
    {
        MT::CScopeMutex lock( m_replaceLock );
        if ( opened->generation != m_generation )
        {
            if ( GUCEF_UINT64MAX != requiredGeneration )
            {
                error = "File \"" + path + "\" may have been replaced since the scan planned it";
                return CORE::DATATABLE_STATUS_IO_ERROR;
            }
            // replaced meanwhile: which version was opened is not known, its footer is read from it and not cached
            opened->generation = GUCEF_UINT64MAX;
        }
        else
        if ( requiredMetaData.IsNULL() )
        {
            MT::CScopeMutex footerLock( m_footerLock );
            TFooterMap::iterator i = m_footers.find( path );
            if ( i != m_footers.end() && (*i).second.size == opened->size )
                cached = (*i).second.metaData;
        }
    }

    if ( !requiredMetaData.IsNULL() && requiredMetaData->CanVerifySource() )
    {
        // opened again for a scan: another process may have replaced the file, the open file must be the planned version
        CSharedFileReadAccess input( opened );
        if ( !requiredMetaData->VerifySource( input ) )
        {
            error = "File \"" + path + "\" was replaced since the scan planned it";
            return CORE::DATATABLE_STATUS_IO_ERROR;
        }
        opened->metaData = requiredMetaData;
        MT::CScopeMutex statsLock( m_statsLock );
        ++m_stats.filesOpened;
        file = opened;
        return CORE::DATATABLE_STATUS_OK;
    }

    if ( !cached.IsNULL() )
    {
        // the cache entry is keyed on the path and the size only: the footer bytes of the open file decide, so a file
        // replaced by another process, by one of the same size within the resolution of any time stamp, is read anew
        CSharedFileReadAccess input( opened );
        if ( cached->CanVerifySource() && cached->VerifySource( input ) )
        {
            opened->metaData = cached;
            metaDataFromCache = true;
            MT::CScopeMutex footerLock( m_footerLock );
            TFooterMap::iterator i = m_footers.find( path );
            if ( i != m_footers.end() && (*i).second.metaData.GetPointer() == cached.GetPointer() )
            {
                m_footerUse.erase( (*i).second.lastUse );
                (*i).second.lastUse = ++m_footerUseCounter;
                m_footerUse[ (*i).second.lastUse ] = path;
            }
        }
        else
        {
            MT::CScopeMutex statsLock( m_statsLock );
            ++m_stats.footerCacheStale;
        }
    }

    if ( !metaDataFromCache )
    {
        CSharedFileReadAccess input( opened );
        CORE::CITableFileMetaDataPtr metaData;
        const CORE::TDataTableStatus status = codec->ReadMetaData( input, codecOptions, metaData, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            error = "Reading the metadata of \"" + path + "\" failed: " + error;
            return status;
        }
        if ( metaData.IsNULL() || metaData->GetSchema().IsNULL() )
        {
            error = "The codec gave no schema for \"" + path + "\"";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        opened->metaData = metaData;

        // cached only while it still describes the file at the path (no replace happened since it was opened) and only
        // when it can be verified against the file at a later open
        MT::CScopeMutex lock( m_replaceLock );
        if ( opened->generation == m_generation && m_settings.footerCacheBytes > 0 && metaData->CanVerifySource() )
            PutFooterInCache( path, metaData, opened->size );
    }

    {
        MT::CScopeMutex statsLock( m_statsLock );
        ++m_stats.filesOpened;
        if ( metaDataFromCache )
            ++m_stats.footerCacheHits;
        else
            ++m_stats.footerReads;
    }
    file = opened;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsTablesStore::MakeTemporaryPath( const CORE::CString& finalPath )
{GUCEF_TRACE;

    const UInt64 counter = ++m_temporaryCounter;
    return finalPath + TemporaryFileMarker + m_escapedWriterId + "-" + CORE::ToString( counter );
}

/*-------------------------------------------------------------------------*/

bool
CVfsTablesStore::FinishTemporaryFile( CORE::IOAccessPtr& access )
{GUCEF_TRACE;

    if ( access.IsNULL() )
        return false;
    bool synced = true;
    access->Flush();
    if ( m_settings.syncWrites )
    {
        // durable when the input is a file of the file system; CIOAccess has no sync of its own (open question 8 of the design)
        #ifdef GUCEF_RTTI_IS_SUPPORTED
        CORE::CFileAccess* fileAccess = dynamic_cast< CORE::CFileAccess* >( access.GetPointerAlways() );
        if ( GUCEF_NULL != fileAccess )
            synced = fileAccess->Sync();
        #endif
    }
    access->Close();
    access.Unlink();
    return synced;
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesStore::OnFileChangedLocked( const CORE::CString& path, bool deleted )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_footerLock );
        TFooterMap::iterator i = m_footers.find( path );
        if ( i != m_footers.end() )
        {
            m_footerBytes -= (*i).second.bytes;
            m_footerUse.erase( (*i).second.lastUse );
            m_footers.erase( i );
        }
    }

    CORE::CString dir;
    CORE::CString name;
    SplitPath( path, dir, name );
    MT::CScopeMutex lock( m_listingLock );
    TListingMap::iterator files = m_listings.find( dir + "|f" );
    if ( deleted )
    {
        if ( files != m_listings.end() )
            (*files).second.names.erase( name );
        return;
    }
    if ( files != m_listings.end() )
        (*files).second.names.insert( name );

    // the directories on the way may be new: cached listings of their parents get them
    CORE::CString child = dir;
    while ( !child.IsNULLOrEmpty() && child.Length() > m_root.Length() )
    {
        CORE::CString parent;
        CORE::CString childName;
        SplitPath( child, parent, childName );
        TListingMap::iterator dirs = m_listings.find( parent + "|d" );
        if ( dirs != m_listings.end() )
            (*dirs).second.names.insert( childName );
        child = parent;
    }
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesStore::ReplaceFile( const CORE::CString& temporaryPath, const CORE::CString& finalPath, CORE::CString& error )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CBackoffPolicy backoff( 10, 2.0, 500, 0.0 );
    const UInt64 started = MT::PrecisionTickCount();
    while ( true )
    {
        bool replaced = false;
        {
            MT::CScopeMutex replaceLock( m_replaceLock );
            replaced = vfs.MoveFileAtomically( temporaryPath, finalPath, true );
            if ( replaced )
            {
                ++m_generation;
                OnFileChangedLocked( finalPath, false );
            }
        }
        if ( replaced )
        {
            MT::CScopeMutex statsLock( m_statsLock );
            ++m_stats.filesReplaced;
            return CORE::DATATABLE_STATUS_OK;
        }

        const UInt64 elapsedMs = (UInt64) MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started );
        if ( elapsedMs >= m_settings.replaceRetryTimeoutMs )
        {
            error = "File \"" + temporaryPath + "\" could not replace \"" + finalPath + "\"";
            return CORE::DATATABLE_STATUS_IO_ERROR;
        }
        {
            MT::CScopeMutex statsLock( m_statsLock );
            ++m_stats.replaceRetries;
        }
        UInt64 delay = backoff.NextDelayInMs();
        const UInt64 left = m_settings.replaceRetryTimeoutMs - elapsedMs;
        if ( delay > left )
            delay = left;
        MT::ThreadDelay( (UInt32) ( delay > 0 ? delay : 1 ) );
    }
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesStore::DeleteFile( const CORE::CString& path, CORE::CString& error )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    MT::CScopeMutex replaceLock( m_replaceLock );
    if ( !vfs.FileExists( path ) )
    {
        OnFileChangedLocked( path, true );
        return CORE::DATATABLE_STATUS_OK;
    }
    if ( !vfs.DeleteFile( path, true ) || vfs.FileExists( path ) )
    {
        error = "File \"" + path + "\" could not be deleted";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    ++m_generation;
    OnFileChangedLocked( path, true );
    replaceLock.EarlyUnlock();

    MT::CScopeMutex statsLock( m_statsLock );
    ++m_stats.filesDeleted;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesStore::RemoveTemporaryFile( const CORE::CString& temporaryPath )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( vfs.FileExists( temporaryPath ) && vfs.DeleteFile( temporaryPath, true ) )
    {
        MT::CScopeMutex statsLock( m_statsLock );
        ++m_stats.temporaryFilesRemoved;
    }
}

/*-------------------------------------------------------------------------*/

bool
CVfsTablesStore::IsTemporaryFileName( const CORE::CString& name )
{GUCEF_TRACE;

    return name.HasSubstr( TemporaryFileMarker, true ) > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsTablesStore::RemoveLeftoverTemporaryFiles( void )
{GUCEF_TRACE;

    if ( m_readOnly )
        return 0;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TStringVector files;
    vfs.GetFileList( files, m_root, true, true, TemporaryFileMarker + m_escapedWriterId + "-", GUCEF_UINT32MAX );
    UInt32 removed = 0;
    for ( VFS::CVFS::TStringVector::const_iterator i = files.begin(); i != files.end(); ++i )
    {
        CORE::CString dir;
        CORE::CString name;
        SplitPath( (*i), dir, name );
        if ( !IsTemporaryOf( name, m_escapedWriterId ) )
            continue;
        if ( vfs.DeleteFile( (*i), true ) )
        {
            ++removed;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "VfsTables: removed the temporary file \"" << (*i) << "\" a write of this writer left behind" );
        }
    }
    if ( removed > 0 )
    {
        MT::CScopeMutex statsLock( m_statsLock );
        m_stats.temporaryFilesRemoved += removed;
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

MT::CMutex&
CVfsTablesStore::GetTableWriteLock( const CORE::CString& tableName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writeLocksLock );
    TWriteLockMap::iterator i = m_writeLocks.find( tableName );
    if ( i != m_writeLocks.end() )
        return *(*i).second;
    MT::CMutex* tableLock = GUCEF_NEW MT::CMutex();
    m_writeLocks[ tableName ] = tableLock;
    return *tableLock;
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsTablesStore::GetGeneration( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_replaceLock );
    return m_generation;
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesStore::ClearCaches( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_listingLock );
        m_listings.clear();
    }
    MT::CScopeMutex lock( m_footerLock );
    m_footers.clear();
    m_footerUse.clear();
    m_footerBytes = 0;
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesStore::GetStats( CVfsTablesStats& stats ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_statsLock );
    stats = m_stats;
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesStore::ResetStats( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_statsLock );
    m_stats.Clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
