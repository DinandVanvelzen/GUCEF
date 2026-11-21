/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_CORE_IFILESYSTEMITERATOR_H
#include "gucefCORE_IFileSystemIterator.h"
#define GUCEF_CORE_IFILESYSTEMITERATOR_H
#endif /* GUCEF_CORE_IFILESYSTEMITERATOR_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#include "gucefDRN_CDRNFileSystemAccessResourceProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The listing of one directory of the file system access, entry by entry, as the iterator gives them */
class CDRNFileSystemAccessEnumerator : public CIDRNNamespaceEnumerator
{
    public:

    CDRNFileSystemAccessEnumerator( const CDRNFileSystemAccessResourceProvider& provider, const CORE::TFileSystemIteratorPtr& iterator,
                                    const CORE::CString& dirPath, const CORE::CString& dirName )
        : CIDRNNamespaceEnumerator()
        , m_provider( provider )
        , m_iterator( iterator )
        , m_dirPath( dirPath )
        , m_dirName( dirName )
        , m_started( false )
        , m_have( false )
    {GUCEF_TRACE;
    }

    virtual ~CDRNFileSystemAccessEnumerator() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_iterator.IsNULL() && m_have )
            m_iterator->FindClose();
    }

    virtual TDRNResourceStatus ReadEntries( UInt32 maxEntries, GUCEF::vector< CDRNNamespaceEntry >& entries, bool& atEnd ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        atEnd = false;
        if ( !m_started )
        {
            m_started = true;
            m_have = m_iterator->FindFirst( m_dirPath );
        }
        UInt32 added = 0;
        while ( m_have && added < maxEntries )
        {
            Produce( entries, added );
            m_have = m_iterator->FindNext();
            if ( !m_have )
                m_iterator->FindClose();
        }
        atEnd = !m_have;
        return DRN_RESSTATUS_OK;
    }

    private:

    /** The entry the iterator stands on, when it is one that is listed */
    void Produce( GUCEF::vector< CDRNNamespaceEntry >& entries, UInt32& added )
    {GUCEF_TRACE;

        bool isDirectory = m_iterator->IsADirectory();
        bool isFile = m_iterator->IsAFile();
        const bool isSymlink = m_iterator->IsSymlink();
        if ( isSymlink && !m_provider.m_settings.followSymlinks )
        {
            m_provider.CountSkipped( true );
            return;
        }
        CORE::CString name = m_iterator->GetResourceName();
        if ( name.IsNULLOrEmpty() )
            return;
        CORE::CString path = m_dirPath.IsNULLOrEmpty() ? name : m_dirPath + "/" + name;
        CORE::CResourceMetaData metaData;
        bool haveMetaData = false;
        if ( isSymlink )
        {
            // followed: what it points at, described the way a STAT of it would
            CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
            if ( CORE::FILESYSTEMACCESS_SUCCESS != m_provider.m_fileSystem->TryGetEntryType( path, type ) || CORE::FILESYSTEMENTRYTYPE_NONE == type )
                return;
            isDirectory = CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type;
            isFile = CORE::FILESYSTEMENTRYTYPE_FILE == type;
        }
        else
        {
            haveMetaData = m_iterator->TryReadMetaData( metaData );
        }
        if ( !isDirectory && !isFile )
            return;         // devices, sockets and the like are not part of a share
        if ( !haveMetaData || ( isFile && !metaData.hasResourceSizeInBytes ) )
        {
            // an iterator that holds names only (the VFS's): the same call a STAT makes
            metaData = CORE::CResourceMetaData();
            if ( CORE::FILESYSTEMACCESS_SUCCESS != m_provider.m_fileSystem->TryGetMetaData( path, metaData ) && isFile )
                return;     // unreadable now: not listed
        }
        if ( !m_provider.m_settings.showHiddenEntries && metaData.hasIsHidden && metaData.isHidden )
        {
            m_provider.CountSkipped( false );
            return;
        }
        if ( isFile && !metaData.hasResourceSizeInBytes )
            return;
        entries.push_back( CDRNNamespaceEntry() );
        CDRNNamespaceEntry& entry = entries.back();
        CORE::CString relative = m_dirName.IsNULLOrEmpty() ? name : m_dirName + "/" + name;
        m_provider.Describe( relative, isDirectory, metaData, entry );
        entry.name = name;
        ++added;
    }

    const CDRNFileSystemAccessResourceProvider& m_provider;
    CORE::TFileSystemIteratorPtr m_iterator;
    CORE::CString m_dirPath;
    CORE::CString m_dirName;
    bool m_started;
    bool m_have;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNFileSystemAccessResourceProvider::CDRNFileSystemAccessResourceProvider( const CORE::TFileSystemAccessPtr& fileSystem  ,
                                                                            const CORE::CString& rootPath                ,
                                                                            const CORE::CUri& bytesBase                  ,
                                                                            const CDRNNamespaceShareSettings& settings   )
    : CIDRNResourceProvider()
    , m_fileSystem( fileSystem )
    , m_rootPath( rootPath )
    , m_bytes( bytesBase )
    , m_settings( settings )
    , m_lock()
    , m_skippedSymlinks( 0 )
    , m_skippedHidden( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNFileSystemAccessResourceProvider::~CDRNFileSystemAccessResourceProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNFileSystemAccessResourceProvider::GetRootPath( void ) const
{GUCEF_TRACE;

    return m_rootPath;
}

/*-------------------------------------------------------------------------*/

const CDRNNamespaceShareSettings&
CDRNFileSystemAccessResourceProvider::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNFileSystemAccessResourceProvider::GetDescription( void ) const
{GUCEF_TRACE;

    return "filesystemaccess:" + m_rootPath + " (" + m_fileSystem->GetDisplayName() + ")";
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNFileSystemAccessResourceProvider::GetNamespaceCapabilities( void ) const
{GUCEF_TRACE;

    return DRN_NS_CAP_READ | DRN_NS_CAP_LIST | DRN_NS_CAP_STAT;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNFileSystemAccessResourceProvider::GetNamespaceFlags( void ) const
{GUCEF_TRACE;

    return DRN_NS_SHAREFLAG_READ_ONLY | ( m_settings.weakRevisions ? DRN_NS_SHAREFLAG_WEAK_REVISION : 0 );
}

/*-------------------------------------------------------------------------*/

void
CDRNFileSystemAccessResourceProvider::GetNamespaceLimits( UInt32& maxStatPaths, UInt32& maxPageEntries ) const
{GUCEF_TRACE;

    maxStatPaths = m_settings.maxStatPaths;
    maxPageEntries = m_settings.maxPageEntries;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNFileSystemAccessResourceProvider::GetSkippedSymlinkCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_skippedSymlinks;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNFileSystemAccessResourceProvider::GetSkippedHiddenCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_skippedHidden;
}

/*-------------------------------------------------------------------------*/

void
CDRNFileSystemAccessResourceProvider::CountSkipped( bool symlink ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( symlink )
        ++m_skippedSymlinks;
    else
        ++m_skippedHidden;
}

/*-------------------------------------------------------------------------*/

bool
CDRNFileSystemAccessResourceProvider::GetKnownSha256( const CORE::CString& /* name */, const CDRNNamespaceEntry& /* entry */, UInt8* /* digest */ ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNFileSystemAccessResourceProvider::PathOf( const CORE::CString& name ) const
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return m_rootPath;
    if ( m_rootPath.IsNULLOrEmpty() )
        return name;
    if ( m_rootPath.EndsWith( "/" ) || m_rootPath.EndsWith( "\\" ) )
        return m_rootPath + name;
    return m_rootPath + "/" + name;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileSystemAccessResourceProvider::StatusOf( CORE::TFileSystemAccessResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case CORE::FILESYSTEMACCESS_SUCCESS:            return DRN_RESSTATUS_OK;
        case CORE::FILESYSTEMACCESS_NOT_FOUND:
        case CORE::FILESYSTEMACCESS_PARENT_NOT_FOUND:   return DRN_RESSTATUS_NOT_FOUND;
        case CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY:    return DRN_RESSTATUS_NOT_A_DIRECTORY;
        case CORE::FILESYSTEMACCESS_INVALID_PATH:
        case CORE::FILESYSTEMACCESS_INVALID_NAME:
        case CORE::FILESYSTEMACCESS_TOO_LONG:           return DRN_RESSTATUS_INVALID_NAME;
        default:                                        return DRN_RESSTATUS_IO_ERROR;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNFileSystemAccessResourceProvider::Describe( const CORE::CString& name, bool isDirectory, const CORE::CResourceMetaData& metaData, CDRNNamespaceEntry& entry ) const
{GUCEF_TRACE;

    entry.Clear();
    entry.name = name;
    entry.kind = isDirectory ? DRN_NS_KIND_DIRECTORY : DRN_NS_KIND_FILE;
    entry.size = isDirectory || !metaData.hasResourceSizeInBytes ? 0 : metaData.resourceSizeInBytes;
    entry.modifiedUtcMs = metaData.hasModifiedDateTime ? metaData.modifiedDateTime.ToUnixEpochBasedTicksInMillisecs() : 0;
    if ( !metaData.hasModifiedDateTime )
        entry.flags |= DRN_NS_ENTRYFLAG_WEAK_REVISION;
    if ( metaData.hasIsReadOnly && metaData.isReadOnly )
        entry.flags |= DRN_NS_ENTRYFLAG_READ_ONLY;
    if ( metaData.hasIsHidden && metaData.isHidden )
        entry.flags |= DRN_NS_ENTRYFLAG_HIDDEN;

    // the revision of a file is the one Resolve() gives, which OPEN checks; a directory's is its change token
    if ( isDirectory )
        entry.revision = 0 != entry.modifiedUtcMs ? CDRNResourceInfo::MakeRevision( 0, entry.modifiedUtcMs ) : CORE::CString::Empty;
    else
        entry.revision = CDRNResourceInfo::MakeRevision( entry.size, entry.modifiedUtcMs );

    entry.hasSha256 = false;
    if ( !isDirectory )
        entry.hasSha256 = GetKnownSha256( name, entry, entry.sha256 );
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileSystemAccessResourceProvider::Inspect( const CORE::CString& name, CDRNNamespaceEntry& entry ) const
{GUCEF_TRACE;

    entry.Clear();
    CORE::CString path = PathOf( name );
    CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
    TDRNResourceStatus status = StatusOf( m_fileSystem->TryGetEntryType( path, type ) );
    if ( DRN_RESSTATUS_OK != status )
        return status;
    if ( CORE::FILESYSTEMENTRYTYPE_NONE == type )
        return DRN_RESSTATUS_NOT_FOUND;
    CORE::CResourceMetaData metaData;
    const bool isDirectory = CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type;
    status = StatusOf( m_fileSystem->TryGetMetaData( path, metaData ) );
    if ( DRN_RESSTATUS_OK != status )
    {
        // a file system that keeps no metadata of its directories (the VFS) still has them
        if ( !isDirectory || DRN_RESSTATUS_NOT_FOUND == status )
            return status;
        metaData = CORE::CResourceMetaData();
    }
    if ( !isDirectory && !metaData.hasResourceSizeInBytes )
        return DRN_RESSTATUS_IO_ERROR;
    if ( !m_settings.showHiddenEntries && !name.IsNULLOrEmpty() && metaData.hasIsHidden && metaData.isHidden )
        return DRN_RESSTATUS_NOT_FOUND;
    Describe( name, isDirectory, metaData, entry );
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileSystemAccessResourceProvider::Stat( const CORE::CString& name, CDRNNamespaceEntry& entry )
{GUCEF_TRACE;

    if ( !name.IsNULLOrEmpty() && !CDRNResourceName::IsValidResourceName( name ) )
        return DRN_RESSTATUS_INVALID_NAME;
    return Inspect( name, entry );
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileSystemAccessResourceProvider::OpenEnumerator( const CORE::CString& dirName, CIDRNNamespaceEnumeratorPtr& enumerator )
{GUCEF_TRACE;

    if ( !dirName.IsNULLOrEmpty() && !CDRNResourceName::IsValidResourceName( dirName ) )
        return DRN_RESSTATUS_INVALID_NAME;
    CORE::CString path = PathOf( dirName );
    CORE::TFileSystemIteratorPtr iterator;
    TDRNResourceStatus status = StatusOf( m_fileSystem->TryGetFileSystemIterator( path, iterator ) );
    if ( DRN_RESSTATUS_OK != status )
        return status;
    if ( iterator.IsNULL() )
        return DRN_RESSTATUS_IO_ERROR;
    enumerator = CIDRNNamespaceEnumeratorPtr( GUCEF_NEW CDRNFileSystemAccessEnumerator( *this, iterator, path, dirName ) );
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileSystemAccessResourceProvider::Resolve( const CORE::CString& name, CDRNResourceInfo& info )
{GUCEF_TRACE;

    info.Clear();
    if ( !CDRNResourceName::IsValidResourceName( name ) )
        return DRN_RESSTATUS_INVALID_NAME;
    CDRNNamespaceEntry entry;
    TDRNResourceStatus status = Inspect( name, entry );
    if ( DRN_RESSTATUS_OK != status )
        return status;
    if ( !entry.IsFile() )
        return DRN_RESSTATUS_NOT_FOUND;
    info.name = name;
    info.size = entry.size;
    info.modifiedUtcMs = entry.modifiedUtcMs;
    info.revision = entry.revision;
    info.hasSha256 = entry.hasSha256;
    if ( entry.hasSha256 )
        memcpy( info.sha256, entry.sha256, CDRNResourceInfo::DigestSize );
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileSystemAccessResourceProvider::Read( const CORE::CString& name            ,
                                            const CDRNResourceInfo& resolved     ,
                                            UInt64 offset                        ,
                                            UInt32 size                          ,
                                            void* destination                    ,
                                            UInt32& bytesRead                    )
{GUCEF_TRACE;

    bytesRead = 0;
    if ( !CDRNResourceName::IsValidResourceName( name ) )
        return DRN_RESSTATUS_INVALID_NAME;
    CDRNNamespaceEntry entry;
    if ( DRN_RESSTATUS_OK != Inspect( name, entry ) || !entry.IsFile() )
        return DRN_RESSTATUS_CHANGED;
    if ( entry.size != resolved.size || entry.revision != resolved.revision )
        return DRN_RESSTATUS_CHANGED;
    return CDRNUriResourceProvider::ReadFromUri( m_bytes.GetUriOf( name ), entry.size, offset, size, destination, bytesRead );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
