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

#include "pubsubpluginSTORAGE_CStoragePubSubIndexReader.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char   SIDX_MAGIC[ 8 ]   = { 'G', 'U', 'C', 'E', 'F', 'I', 'D', 'X' };
static const UInt8  SIDX_FORMAT_VER    = 1;
static const UInt32 SIDX_FOOTER_MAGIC  = 0x47494458u;
static const UInt32 SIDX_FOOTER_SIZE   = 20;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStoragePubSubIndexReader::CStoragePubSubIndexReader( const CStoragePubSubIndexDef& def     ,
                                                      const CORE::CString&          dirPath )
    : m_def( def )
    , m_indexFilePath( CORE::CombinePath( dirPath, def.GetIndexFilename() ) )
    , m_dirPath( dirPath )
    , m_fileRegistry()
    , m_entries()
    , m_loaded( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CStoragePubSubIndexReader::~CStoragePubSubIndexReader( void )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexReader::IsLoaded( void ) const
{GUCEF_TRACE;

    return m_loaded;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexReader::LoadIndex( void )
{GUCEF_TRACE;

    m_loaded = false;
    m_entries.clear();
    m_fileRegistry.clear();

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    if ( !vfs.FileExists( m_indexFilePath ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:LoadIndex: Index file not found: " + m_indexFilePath );
        return false;
    }

    CORE::CDynamicBuffer buf;
    if ( !vfs.LoadFile( buf, m_indexFilePath, "rb" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:LoadIndex: Failed to load index file: " + m_indexFilePath );
        return false;
    }

    CORE::UInt32 fileSize = buf.GetDataSize();
    if ( fileSize < SIDX_FOOTER_SIZE )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:LoadIndex: Index file too small: " + m_indexFilePath );
        return false;
    }

    /* Validate header magic */
    char magic[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    buf.CopyTo( 0, 8, magic );
    if ( 0 != ::memcmp( magic, SIDX_MAGIC, 8 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:LoadIndex: Invalid magic in: " + m_indexFilePath );
        return false;
    }

    /* Validate footer magic */
    CORE::UInt32 footerMagic = 0;
    buf.CopyTo( fileSize - 4, 4, &footerMagic );
    if ( footerMagic != SIDX_FOOTER_MAGIC )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:LoadIndex: Invalid footer magic in: " + m_indexFilePath );
        return false;
    }

    /* Read footer fields */
    CORE::UInt32 footerOffset = fileSize - SIDX_FOOTER_SIZE;

    CORE::UInt32 indexEntryCount    = 0;
    CORE::UInt32 indexEntriesOffset = 0;
    CORE::UInt32 fileRegistryOffset = 0;
    CORE::UInt32 fileRegistryCount  = 0;

    buf.CopyTo( footerOffset +  0, 4, &indexEntryCount    );
    buf.CopyTo( footerOffset +  4, 4, &indexEntriesOffset );
    buf.CopyTo( footerOffset +  8, 4, &fileRegistryOffset );
    buf.CopyTo( footerOffset + 12, 4, &fileRegistryCount  );

    /* Read index type from header and verify it matches the definition */
    CORE::UInt32 fileIndexTypeId = 0;
    buf.CopyTo( 9, 4, &fileIndexTypeId );  /* offset 9 = after magic(8) + version(1) */
    if ( fileIndexTypeId != static_cast< CORE::UInt32 >( m_def.indexType ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:LoadIndex: Index type mismatch in " + m_indexFilePath );
        /* Continue anyway - may still be usable */
    }

    /* Read all index entries */
    CORE::UInt16 entrySize = m_def.GetEntrySize();
    if ( entrySize > 0 && indexEntryCount > 0 )
    {
        m_entries.reserve( indexEntryCount );
        CORE::UInt32 pos = indexEntriesOffset;
        for ( CORE::UInt32 i=0; i<indexEntryCount; ++i )
        {
            if ( pos + entrySize > fileSize )
                break;

            IndexEntry entry;
            buf.CopyTo( pos + 0, 8, &entry.keyValue  );
            buf.CopyTo( pos + 8, 4, &entry.fileId    );
            buf.CopyTo( pos + 12, 4, &entry.msgIndex );
            m_entries.push_back( entry );
            pos += entrySize;
        }
    }

    /* Read file registry */
    if ( fileRegistryCount > 0 && fileRegistryOffset + 4 <= fileSize )
    {
        m_fileRegistry.reserve( fileRegistryCount );
        CORE::UInt32 regPos = fileRegistryOffset + 4;  /* skip count field */
        for ( CORE::UInt32 i=0; i<fileRegistryCount; ++i )
        {
            if ( regPos + 26 > fileSize )
                break;

            FileRegEntry entry;
            buf.CopyTo( regPos +  0, 4, &entry.fileId     ); regPos += 4;
            buf.CopyTo( regPos +  0, 8, &entry.firstKey   ); regPos += 8;
            buf.CopyTo( regPos +  0, 8, &entry.lastKey    ); regPos += 8;
            buf.CopyTo( regPos +  0, 4, &entry.entryCount ); regPos += 4;

            CORE::UInt16 fnLen = 0;
            buf.CopyTo( regPos, 2, &fnLen ); regPos += 2;
            if ( fnLen > 0 && regPos + fnLen <= fileSize )
            {
                CORE::CDynamicBuffer fnBuf( fnLen + 1, false );
                buf.CopyTo( regPos, fnLen, fnBuf.GetBufferPtr() );
                static_cast< char* >( fnBuf.GetBufferPtr() )[ fnLen ] = '\0';
                entry.filename = static_cast< const char* >( fnBuf.GetConstBufferPtr() );
                regPos += fnLen;
            }

            m_fileRegistry.push_back( entry );
        }
    }

    m_loaded = true;
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:LoadIndex: Loaded " +
        CORE::ToString( (CORE::UInt32) m_entries.size() ) + " entries from " + m_indexFilePath );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexReader::FindStartBookmark( CORE::UInt64                                      startKeyValue ,
                                               CStoragePubSubClientTopic::CStorageBookmarkInfo& bookmark      ) const
{GUCEF_TRACE;

    if ( !m_loaded )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:FindStartBookmark: Index not loaded" );
        return false;
    }

    if ( m_entries.empty() || m_fileRegistry.empty() )
        return false;

    /* Step 1: binary search the file registry for the first file where lastKey >= startKeyValue */
    CORE::UInt32 fileRegIdx  = static_cast< CORE::UInt32 >( m_fileRegistry.size() );
    CORE::UInt32 lo          = 0;
    CORE::UInt32 hi          = static_cast< CORE::UInt32 >( m_fileRegistry.size() );
    while ( lo < hi )
    {
        CORE::UInt32 mid = lo + ( hi - lo ) / 2;
        if ( m_fileRegistry[ mid ].lastKey < startKeyValue )
            lo = mid + 1;
        else
        {
            fileRegIdx = mid;
            hi         = mid;
        }
    }

    if ( fileRegIdx >= static_cast< CORE::UInt32 >( m_fileRegistry.size() ) )
        return false;  /* all keys are below startKeyValue */

    const FileRegEntry& fileReg = m_fileRegistry[ fileRegIdx ];

    /* Step 2: binary search m_entries for the first entry where keyValue >= startKeyValue
               that also belongs to this file (fileId match) */

    /* Find the range of entries for this file by scanning (entries are in file-order) */
    CORE::UInt32 entryCount = static_cast< CORE::UInt32 >( m_entries.size() );
    CORE::UInt32 rangeStart = entryCount;
    CORE::UInt32 rangeEnd   = entryCount;

    for ( CORE::UInt32 i=0; i<entryCount; ++i )
    {
        if ( m_entries[ i ].fileId == fileReg.fileId )
        {
            if ( rangeStart == entryCount )
                rangeStart = i;
            rangeEnd = i + 1;
        }
        else if ( rangeStart != entryCount )
        {
            break;  /* entries are grouped by file in insertion order */
        }
    }

    if ( rangeStart == entryCount )
        return false;

    /* Binary search within the file's entry range for first keyValue >= startKeyValue */
    lo = rangeStart;
    hi = rangeEnd;
    CORE::UInt32 foundIdx = rangeEnd;
    while ( lo < hi )
    {
        CORE::UInt32 mid = lo + ( hi - lo ) / 2;
        if ( m_entries[ mid ].keyValue < startKeyValue )
            lo = mid + 1;
        else
        {
            foundIdx = mid;
            hi       = mid;
        }
    }

    if ( foundIdx >= rangeEnd )
        return false;

    const IndexEntry& entry = m_entries[ foundIdx ];

    bookmark = CStoragePubSubClientTopic::CStorageBookmarkInfo(
        CORE::CombinePath( m_dirPath, fileReg.filename ) );
    bookmark.msgIndex     = entry.msgIndex;
    bookmark.offsetInFile = 0;   /* caller's read path fills this from the container footer */
    bookmark.doneWithFile = 0;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:FindStartBookmark: Found key=" +
        CORE::ToString( entry.keyValue ) + " in " + fileReg.filename + " at msgIndex=" +
        CORE::ToString( entry.msgIndex ) );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexReader::GetOrderedFilePathsFromFile( const CORE::CString& startFilePath ,
                                                         TStringVector&        outPaths      ) const
{GUCEF_TRACE;

    if ( !m_loaded )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:GetOrderedFilePathsFromFile: Index not loaded" );
        return false;
    }

    // Find the registry entry whose full path matches startFilePath
    CORE::UInt32 regCount = static_cast< CORE::UInt32 >( m_fileRegistry.size() );
    CORE::UInt32 startIdx = regCount;
    for ( CORE::UInt32 i=0; i<regCount; ++i )
    {
        CORE::CString fullPath = CORE::CombinePath( m_dirPath, m_fileRegistry[ i ].filename );
        if ( fullPath == startFilePath )
        {
            startIdx = i;
            break;
        }
    }

    if ( startIdx >= regCount )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexReader:GetOrderedFilePathsFromFile: "
            "startFilePath not found in file registry: " + startFilePath );
        return false;
    }

    outPaths.clear();
    outPaths.reserve( regCount - startIdx );
    for ( CORE::UInt32 i=startIdx; i<regCount; ++i )
    {
        outPaths.push_back( CORE::CombinePath( m_dirPath, m_fileRegistry[ i ].filename ) );
    }
    return true;
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
