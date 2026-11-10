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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

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

#include "pubsubpluginSTORAGE_CStoragePubSubIndexWriter.h"

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

/* Binary .sidx format constants */
static const char   SIDX_MAGIC[ 8 ]   = { 'G', 'U', 'C', 'E', 'F', 'I', 'D', 'X' };
static const UInt8  SIDX_FORMAT_VER    = 1;
static const UInt32 SIDX_FOOTER_MAGIC  = 0x47494458u;  /* "GIDX" */
static const UInt32 SIDX_FOOTER_SIZE   = 20;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION: FileRegEntry                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

CStoragePubSubIndexWriter::FileRegEntry::FileRegEntry( void )
    : fileId( 0 )
    , firstKey( 0 )
    , lastKey( 0 )
    , entryCount( 0 )
    , filename()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION: CStoragePubSubIndexWriter                         //
//                                                                         //
//-------------------------------------------------------------------------*/

CStoragePubSubIndexWriter::CStoragePubSubIndexWriter( const CStoragePubSubIndexDef& def      ,
                                                      const CORE::CString&          dirPath  )
    : m_def( def )
    , m_dirPath( dirPath )
    , m_indexFilePath( CORE::CombinePath( dirPath, def.GetIndexFilename() ) )
    , m_fileRegistry()
    , m_nextFileId( 1 )
    , m_totalEntryCount( 0 )
    , m_initialized( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CStoragePubSubIndexWriter::~CStoragePubSubIndexWriter( void )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CStoragePubSubIndexWriter::ComputeHeaderSize( void ) const
{GUCEF_TRACE;

    /* 8(magic) + 1(version) + 4(typeId) + 4(flags) + 4(interval) + 2(nameLen) + nameBytes */
    return 8u + 1u + 4u + 4u + 4u + 2u + (CORE::UInt32) m_def.keyName.Length();
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubIndexWriter::WriteHeader( CORE::CDynamicBuffer&      buf    ,
                                        CORE::UInt32&              offset ,
                                        const CStoragePubSubIndexDef& def )
{GUCEF_TRACE;

    /* magic (8 bytes) */
    buf.CopyFrom( offset, 8, SIDX_MAGIC );
    offset += 8;

    /* format_version (UInt8) */
    UInt8 fmtVer = SIDX_FORMAT_VER;
    buf.CopyFrom( offset, 1, &fmtVer );
    offset += 1;

    /* index_type_id (UInt32) */
    CORE::UInt32 typeId = static_cast< CORE::UInt32 >( def.indexType );
    buf.CopyFrom( offset, 4, &typeId );
    offset += 4;

    /* flags (UInt32, reserved = 0) */
    CORE::UInt32 flags = 0;
    buf.CopyFrom( offset, 4, &flags );
    offset += 4;

    /* sampling_interval (UInt32) */
    CORE::UInt32 interval = def.samplingInterval;
    buf.CopyFrom( offset, 4, &interval );
    offset += 4;

    /* key_name_len (UInt16) */
    CORE::UInt16 nameLen = static_cast< CORE::UInt16 >( def.keyName.Length() );
    buf.CopyFrom( offset, 2, &nameLen );
    offset += 2;

    /* key_name (ASCII, no null) */
    if ( nameLen > 0 )
    {
        buf.CopyFrom( offset, nameLen, def.keyName.C_String() );
        offset += nameLen;
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubIndexWriter::WriteIndexEntries( CORE::CDynamicBuffer& buf        ,
                                              CORE::UInt32&         offset     ,
                                              const TUInt64Vector&  keys       ,
                                              const TUInt32Vector&  msgIndexes ,
                                              CORE::UInt32          fileId     )
{GUCEF_TRACE;

    CORE::UInt32 count = static_cast< CORE::UInt32 >( keys.size() );
    for ( CORE::UInt32 i=0; i<count; ++i )
    {
        buf.CopyFrom( offset, 8, &keys[ i ] );
        offset += 8;
        buf.CopyFrom( offset, 4, &fileId );
        offset += 4;
        buf.CopyFrom( offset, 4, &msgIndexes[ i ] );
        offset += 4;
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubIndexWriter::WriteFileRegistry( CORE::CDynamicBuffer& buf    ,
                                              CORE::UInt32&         offset ) const
{GUCEF_TRACE;

    CORE::UInt32 count = static_cast< CORE::UInt32 >( m_fileRegistry.size() );
    buf.CopyFrom( offset, 4, &count );
    offset += 4;

    for ( CORE::UInt32 i=0; i<count; ++i )
    {
        const FileRegEntry& entry = m_fileRegistry[ i ];

        buf.CopyFrom( offset, 4, &entry.fileId );
        offset += 4;

        buf.CopyFrom( offset, 8, &entry.firstKey );
        offset += 8;

        buf.CopyFrom( offset, 8, &entry.lastKey );
        offset += 8;

        buf.CopyFrom( offset, 4, &entry.entryCount );
        offset += 4;

        CORE::UInt16 fnLen = static_cast< CORE::UInt16 >( entry.filename.Length() );
        buf.CopyFrom( offset, 2, &fnLen );
        offset += 2;

        if ( fnLen > 0 )
        {
            buf.CopyFrom( offset, fnLen, entry.filename.C_String() );
            offset += fnLen;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubIndexWriter::WriteFooter( CORE::CDynamicBuffer& buf                ,
                                        CORE::UInt32&         offset             ,
                                        CORE::UInt32          indexEntryCount    ,
                                        CORE::UInt32          indexEntriesOffset ,
                                        CORE::UInt32          fileRegistryOffset ,
                                        CORE::UInt32          fileRegistryCount  )
{GUCEF_TRACE;

    buf.CopyFrom( offset, 4, &indexEntryCount    ); offset += 4;
    buf.CopyFrom( offset, 4, &indexEntriesOffset ); offset += 4;
    buf.CopyFrom( offset, 4, &fileRegistryOffset ); offset += 4;
    buf.CopyFrom( offset, 4, &fileRegistryCount  ); offset += 4;

    CORE::UInt32 magic = SIDX_FOOTER_MAGIC;
    buf.CopyFrom( offset, 4, &magic );
    offset += 4;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexWriter::ExtractKeyFromMsg( const PUBSUB::CBasicPubSubMsg& msg ,
                                              CORE::UInt64&                  keyOut ) const
{GUCEF_TRACE;

    const PUBSUB::CIPubSubMsg::TKeyValuePairs* kvPairs = GUCEF_NULL;

    if ( m_def.keySource == CStoragePubSubIndexDef::KEY_SOURCE_META_DATA )
        kvPairs = &msg.GetMetaDataKeyValuePairs();
    else
        kvPairs = &msg.GetKeyValuePairs();

    CORE::UInt32 pairCount = static_cast< CORE::UInt32 >( kvPairs->size() );
    for ( CORE::UInt32 i=0; i<pairCount; ++i )
    {
        const PUBSUB::CIPubSubMsg::TKeyValuePair& pair = (*kvPairs)[ i ];
        if ( pair.first.AsString() == m_def.keyName )
        {
            keyOut = pair.second.AsUInt64( 0, false );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexWriter::Initialize( void )
{GUCEF_TRACE;

    if ( m_initialized )
        return true;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    if ( !vfs.FileExists( m_indexFilePath ) )
    {
        /* No pre-existing file - start fresh */
        m_fileRegistry.clear();
        m_nextFileId       = 1;
        m_totalEntryCount  = 0;
        m_initialized      = true;
        return true;
    }

    /* Load existing .sidx to restore in-memory state */
    CORE::CDynamicBuffer existing;
    if ( !vfs.LoadFile( existing, m_indexFilePath, "rb" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:Initialize: Failed to load existing index file: " + m_indexFilePath );
        return false;
    }

    CORE::UInt32 fileSize = existing.GetDataSize();
    if ( fileSize < SIDX_FOOTER_SIZE )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:Initialize: Existing index file is too small, will recreate: " + m_indexFilePath );
        m_fileRegistry.clear();
        m_nextFileId      = 1;
        m_totalEntryCount = 0;
        m_initialized     = true;
        return true;
    }

    /* Read footer from last SIDX_FOOTER_SIZE bytes */
    CORE::UInt32 footerOffset = fileSize - SIDX_FOOTER_SIZE;

    CORE::UInt32 footerMagic = 0;
    existing.CopyTo( footerOffset + 16, 4, &footerMagic );
    if ( footerMagic != SIDX_FOOTER_MAGIC )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:Initialize: Footer magic mismatch in " + m_indexFilePath + ", will recreate" );
        m_fileRegistry.clear();
        m_nextFileId      = 1;
        m_totalEntryCount = 0;
        m_initialized     = true;
        return true;
    }

    CORE::UInt32 indexEntryCount    = 0;
    CORE::UInt32 indexEntriesOffset = 0;
    CORE::UInt32 fileRegistryOffset = 0;
    CORE::UInt32 fileRegistryCount  = 0;

    existing.CopyTo( footerOffset +  0, 4, &indexEntryCount    );
    existing.CopyTo( footerOffset +  4, 4, &indexEntriesOffset );
    existing.CopyTo( footerOffset +  8, 4, &fileRegistryOffset );
    existing.CopyTo( footerOffset + 12, 4, &fileRegistryCount  );

    m_totalEntryCount = indexEntryCount;

    /* Parse file registry */
    m_fileRegistry.clear();
    m_nextFileId = 1;

    if ( fileRegistryCount > 0 && fileRegistryOffset < fileSize )
    {
        CORE::UInt32 regPos = fileRegistryOffset + 4;  /* skip the count field we already have */
        for ( CORE::UInt32 i=0; i<fileRegistryCount; ++i )
        {
            if ( regPos + 26 > fileSize )
                break;

            FileRegEntry entry;
            existing.CopyTo( regPos +  0, 4, &entry.fileId    ); regPos += 4;
            existing.CopyTo( regPos +  0, 8, &entry.firstKey  ); regPos += 8;
            existing.CopyTo( regPos +  0, 8, &entry.lastKey   ); regPos += 8;
            existing.CopyTo( regPos +  0, 4, &entry.entryCount ); regPos += 4;

            CORE::UInt16 fnLen = 0;
            existing.CopyTo( regPos, 2, &fnLen ); regPos += 2;
            if ( fnLen > 0 && regPos + fnLen <= fileSize )
            {
                CORE::CDynamicBuffer fnBuf( fnLen + 1, false );
                existing.CopyTo( regPos, fnLen, fnBuf.GetBufferPtr() );
                static_cast< char* >( fnBuf.GetBufferPtr() )[ fnLen ] = '\0';
                entry.filename = static_cast< const char* >( fnBuf.GetConstBufferPtr() );
                regPos += fnLen;
            }

            if ( entry.fileId >= m_nextFileId )
                m_nextFileId = entry.fileId + 1;

            m_fileRegistry.push_back( entry );
        }
    }

    /* Parse index entries so AppendToFile can rebuild the file without re-reading VFS */
    m_allKeys.clear();
    m_allFileIds.clear();
    m_allMsgIndexes.clear();
    if ( indexEntryCount > 0 && indexEntriesOffset < fileSize )
    {
        CORE::UInt32 entrySize = m_def.GetEntrySize();
        CORE::UInt32 entryPos  = indexEntriesOffset;
        for ( CORE::UInt32 i=0; i<indexEntryCount; ++i )
        {
            if ( entryPos + entrySize > fileSize )
                break;
            CORE::UInt64 key    = 0;
            CORE::UInt32 fileId = 0;
            CORE::UInt32 msgIdx = 0;
            existing.CopyTo( entryPos, 8, &key    ); entryPos += 8;
            existing.CopyTo( entryPos, 4, &fileId ); entryPos += 4;
            existing.CopyTo( entryPos, 4, &msgIdx ); entryPos += 4;
            m_allKeys.push_back( key );
            m_allFileIds.push_back( fileId );
            m_allMsgIndexes.push_back( msgIdx );
        }
    }

    m_initialized = true;
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:Initialize: Loaded existing index with " +
        CORE::ToString( m_totalEntryCount ) + " entries and " +
        CORE::ToString( (CORE::UInt32) m_fileRegistry.size() ) + " files from: " + m_indexFilePath );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexWriter::WriteNewFile( const TUInt64Vector& keys       ,
                                         const TUInt32Vector& msgIndexes ,
                                         const FileRegEntry&  regEntry   )
{GUCEF_TRACE;

    CORE::UInt32 headerSize       = ComputeHeaderSize();
    CORE::UInt32 entrySize        = m_def.GetEntrySize();
    CORE::UInt32 newEntryCount    = static_cast< CORE::UInt32 >( keys.size() );
    CORE::UInt32 entriesSize      = newEntryCount * entrySize;
    CORE::UInt32 regOffset        = headerSize + entriesSize;
    /* registry: 4(count) + (4+8+8+4+2+fnLen) per entry */
    CORE::UInt32 regEntrySize     = 26u + static_cast< CORE::UInt32 >( regEntry.filename.Length() );
    CORE::UInt32 regSize          = 4u + regEntrySize;
    CORE::UInt32 totalSize        = regOffset + regSize + SIDX_FOOTER_SIZE;

    CORE::CDynamicBuffer buf( totalSize, false );
    buf.SetDataSize( totalSize );

    CORE::UInt32 offset = 0;
    WriteHeader( buf, offset, m_def );
    WriteIndexEntries( buf, offset, keys, msgIndexes, regEntry.fileId );

    for ( CORE::UInt32 i=0; i<newEntryCount; ++i )
    {
        m_allKeys.push_back( keys[ i ] );
        m_allFileIds.push_back( regEntry.fileId );
        m_allMsgIndexes.push_back( msgIndexes[ i ] );
    }
    m_fileRegistry.push_back( regEntry );
    m_totalEntryCount += newEntryCount;

    CORE::UInt32 fileRegOffset = offset;
    WriteFileRegistry( buf, offset );

    CORE::UInt32 fileRegCount = static_cast< CORE::UInt32 >( m_fileRegistry.size() );
    WriteFooter( buf, offset, m_totalEntryCount, headerSize, fileRegOffset, fileRegCount );
    buf.SetDataSize( offset );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.StoreAsFile( m_indexFilePath, buf, 0, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:WriteNewFile: Failed to write index file: " + m_indexFilePath );
        for ( CORE::UInt32 i=0; i<newEntryCount; ++i )
        {
            m_allKeys.pop_back();
            m_allFileIds.pop_back();
            m_allMsgIndexes.pop_back();
        }
        m_fileRegistry.pop_back();
        m_totalEntryCount -= newEntryCount;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexWriter::AppendToFile( const TUInt64Vector& keys       ,
                                          const TUInt32Vector& msgIndexes ,
                                          const FileRegEntry&  regEntry   )
{GUCEF_TRACE;

    /* Build the complete .sidx from in-memory state so we never depend on
       reading back the previously written VFS file.  The VFS may cache the
       first version of the file and return it stale on all subsequent reads,
       which would corrupt the index entries section.  All data we need is
       already in m_allKeys / m_allFileIds / m_allMsgIndexes / m_fileRegistry. */

    CORE::UInt32 headerSize    = ComputeHeaderSize();
    CORE::UInt32 entrySize     = m_def.GetEntrySize();
    CORE::UInt32 newEntryCount = static_cast< CORE::UInt32 >( keys.size() );
    CORE::UInt32 totalEntries  = m_totalEntryCount + newEntryCount;

    /* registry size: existing entries + the new one */
    CORE::UInt32 newFileRegCount = static_cast< CORE::UInt32 >( m_fileRegistry.size() ) + 1u;
    CORE::UInt32 regSize = 4u;  /* count field */
    for ( CORE::UInt32 i=0; i<(CORE::UInt32)m_fileRegistry.size(); ++i )
        regSize += 26u + static_cast< CORE::UInt32 >( m_fileRegistry[ i ].filename.Length() );
    regSize += 26u + static_cast< CORE::UInt32 >( regEntry.filename.Length() );

    CORE::UInt32 fileRegistryOffset = headerSize + totalEntries * entrySize;
    CORE::UInt32 totalSize          = fileRegistryOffset + regSize + SIDX_FOOTER_SIZE;

    CORE::CDynamicBuffer outBuf( totalSize, false );
    outBuf.SetDataSize( totalSize );

    CORE::UInt32 offset = 0;
    WriteHeader( outBuf, offset, m_def );

    /* Write all previously accumulated index entries from memory */
    CORE::UInt32 existingCount = static_cast< CORE::UInt32 >( m_allKeys.size() );
    for ( CORE::UInt32 i=0; i<existingCount; ++i )
    {
        outBuf.CopyFrom( offset, 8, &m_allKeys[ i ] );       offset += 8;
        outBuf.CopyFrom( offset, 4, &m_allFileIds[ i ] );    offset += 4;
        outBuf.CopyFrom( offset, 4, &m_allMsgIndexes[ i ] ); offset += 4;
    }

    /* Append the new entries for this container */
    WriteIndexEntries( outBuf, offset, keys, msgIndexes, regEntry.fileId );

    /* Update in-memory state */
    for ( CORE::UInt32 i=0; i<newEntryCount; ++i )
    {
        m_allKeys.push_back( keys[ i ] );
        m_allFileIds.push_back( regEntry.fileId );
        m_allMsgIndexes.push_back( msgIndexes[ i ] );
    }
    m_fileRegistry.push_back( regEntry );
    m_totalEntryCount += newEntryCount;

    CORE::UInt32 newFileRegOffset = offset;
    WriteFileRegistry( outBuf, offset );
    WriteFooter( outBuf, offset, m_totalEntryCount, headerSize, newFileRegOffset, newFileRegCount );
    outBuf.SetDataSize( offset );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.StoreAsFile( m_indexFilePath, outBuf, 0, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:AppendToFile: Failed to write index file: " + m_indexFilePath );
        for ( CORE::UInt32 i=0; i<newEntryCount; ++i )
        {
            m_allKeys.pop_back();
            m_allFileIds.pop_back();
            m_allMsgIndexes.pop_back();
        }
        m_fileRegistry.pop_back();
        m_totalEntryCount -= newEntryCount;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexWriter::AppendContainer( const PUBSUB::CPubSubMsgBinarySerializerOptions&                    serializerOptions ,
                                             const CORE::CDynamicBuffer&                                         buffer            ,
                                             const PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex& msgOffsets        ,
                                             const CORE::CString&                                                containerFilename )
{GUCEF_TRACE;

    if ( !Initialize() )
        return false;

    if ( msgOffsets.empty() )
        return true;   /* nothing to index */

    TUInt64Vector keys;
    TUInt32Vector msgIndexes;

    CORE::UInt32 msgCount = static_cast< CORE::UInt32 >( msgOffsets.size() );
    CORE::UInt32 interval = m_def.samplingInterval;

    for ( CORE::UInt32 i=0; i<msgCount; i += interval )
    {
        PUBSUB::CBasicPubSubMsg msg;
        CORE::UInt32 bytesRead = 0;
        if ( !PUBSUB::CPubSubMsgBinarySerializer::Deserialize( serializerOptions, true, msg, msgOffsets[ i ], buffer, bytesRead ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:AppendContainer: Failed to deserialize msg at offset " +
                CORE::ToString( msgOffsets[ i ] ) + " in container " + containerFilename );
            continue;
        }

        CORE::UInt64 keyValue = 0;
        if ( !ExtractKeyFromMsg( msg, keyValue ) )
            continue;

        keys.push_back( keyValue );
        msgIndexes.push_back( i );
    }

    if ( keys.empty() )
        return true;

    FileRegEntry reg;
    reg.fileId     = m_nextFileId++;
    reg.firstKey   = keys.front();
    reg.lastKey    = keys.back();
    reg.entryCount = static_cast< CORE::UInt32 >( keys.size() );
    reg.filename   = containerFilename;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    bool success = false;
    if ( !vfs.FileExists( m_indexFilePath ) )
        success = WriteNewFile( keys, msgIndexes, reg );
    else
        success = AppendToFile( keys, msgIndexes, reg );

    if ( success )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:AppendContainer: Indexed " +
            CORE::ToString( (CORE::UInt32) keys.size() ) + " entries from " + containerFilename +
            " into " + m_indexFilePath );
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexWriter::RebuildIndex( const PUBSUB::CPubSubMsgBinarySerializerOptions& serializerOptions )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    /* Reset in-memory state */
    m_fileRegistry.clear();
    m_allKeys.clear();
    m_allFileIds.clear();
    m_allMsgIndexes.clear();
    m_nextFileId      = 1;
    m_totalEntryCount = 0;
    m_initialized     = true;

    /* Remove existing index so AppendContainer will create a new one */
    if ( vfs.FileExists( m_indexFilePath ) )
        vfs.DeleteFile( m_indexFilePath, true );

    /* Enumerate container files in the directory */
    VFS::CVFS::TStringVector files;
    if ( !vfs.GetFileList( files, m_dirPath, false, false, CORE::CString::Empty ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:RebuildIndex: Failed to list files in: " + m_dirPath );
        return false;
    }

    bool success = true;
    CORE::UInt32 fileCount = static_cast< CORE::UInt32 >( files.size() );
    for ( CORE::UInt32 i=0; i<fileCount; ++i )
    {
        const CORE::CString& filename = files[ i ];

        /* Skip index files themselves */
        if ( filename.HasSubstr( ".sidx", true ) )
            continue;

        CORE::CString fullPath = CORE::CombinePath( m_dirPath, filename );

        CORE::CDynamicBuffer containerBuf;
        if ( !vfs.LoadFile( containerBuf, fullPath, "rb" ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:RebuildIndex: Failed to load container: " + fullPath );
            success = false;
            continue;
        }

        bool isCorrupted = false;
        PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex msgOffsets;
        PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector msgs;
        if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild( msgs, true, msgOffsets, containerBuf, isCorrupted, true ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:RebuildIndex: Failed to deserialize container: " + fullPath );
            success = false;
            continue;
        }

        if ( !AppendContainer( serializerOptions, containerBuf, msgOffsets, filename ) )
        {
            success = false;
        }
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CStoragePubSubIndexWriter:RebuildIndex: Rebuilt index with " +
        CORE::ToString( m_totalEntryCount ) + " entries from " +
        CORE::ToString( (CORE::UInt32) m_fileRegistry.size() ) + " files in: " + m_dirPath );

    return success;
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
