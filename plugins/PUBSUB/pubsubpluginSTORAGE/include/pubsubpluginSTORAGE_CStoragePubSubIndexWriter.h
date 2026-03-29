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

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXWRITER_H
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgContainerBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H
#include "pubsubpluginSTORAGE_CStoragePubSubIndexDef.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_MACROS_H
#include "pubsubpluginSTORAGE_macros.h"
#define PUBSUBPLUGIN_STORAGE_MACROS_H
#endif /* PUBSUBPLUGIN_STORAGE_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes and incrementally updates a .sidx sidecar index file for a single
 *  topic root directory and a single index definition.
 *
 *  Usage:
 *    - Construct once per (topic-root, indexDef) pair.
 *    - Call AppendContainer() immediately after a container file is stored to VFS.
 *    - The writer lazy-initialises on the first AppendContainer() call: if a
 *      pre-existing .sidx is present it loads its state, otherwise it creates a
 *      fresh file.
 *    - Call RebuildIndex() to rescan all container files in the directory and
 *      rewrite the .sidx from scratch.
 */
class PUBSUBPLUGIN_STORAGE_PLUGIN_PRIVATE_CPP CStoragePubSubIndexWriter
{
    public:

    CStoragePubSubIndexWriter( const CStoragePubSubIndexDef& def ,
                               const CORE::CString&          dirPath );

    ~CStoragePubSubIndexWriter( void );

    /**
     *  Called immediately after StoreNextReceivedPubSubBuffer() succeeds.
     *  @param serializerOptions  Options that match those used when the buffer was serialized.
     *  @param buffer             The in-memory container buffer (still valid at call time).
     *  @param msgOffsets         bufferMetaData->msgOffsetIndex for this buffer.
     *  @param containerFilename  The VFS filename only (not full path), e.g. "2024-01-01_...pubsubmsgs".
     */
    bool AppendContainer( const PUBSUB::CPubSubMsgBinarySerializerOptions&                    serializerOptions ,
                          const CORE::CDynamicBuffer&                                         buffer            ,
                          const PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex& msgOffsets        ,
                          const CORE::CString&                                                containerFilename );

    /**
     *  Full rebuild: scans all container files in the topic root directory,
     *  rewrites the .sidx from scratch.
     */
    bool RebuildIndex( const PUBSUB::CPubSubMsgBinarySerializerOptions& serializerOptions );

    private:

    struct FileRegEntry
    {
        CORE::UInt32  fileId;
        CORE::UInt64  firstKey;
        CORE::UInt64  lastKey;
        CORE::UInt32  entryCount;
        CORE::CString filename;

        FileRegEntry( void );
    };

    typedef GUCEF::vector< FileRegEntry, gucef_allocator< FileRegEntry > > TFileRegEntryVector;
    typedef GUCEF::vector< CORE::UInt64, gucef_allocator< CORE::UInt64 > > TUInt64Vector;
    typedef GUCEF::vector< CORE::UInt32, gucef_allocator< CORE::UInt32 > > TUInt32Vector;

    /**
     *  Lazy initialization: loads state from an existing .sidx, or marks as
     *  ready-to-create-new if no file exists yet.
     */
    bool Initialize( void );

    bool ExtractKeyFromMsg( const PUBSUB::CBasicPubSubMsg& msg ,
                            CORE::UInt64&                  keyOut ) const;

    bool WriteNewFile( const TUInt64Vector& keys       ,
                       const TUInt32Vector& msgIndexes ,
                       const FileRegEntry&  regEntry   );

    bool AppendToFile( const TUInt64Vector& keys       ,
                       const TUInt32Vector& msgIndexes ,
                       const FileRegEntry&  regEntry   );

    CORE::UInt32 ComputeHeaderSize( void ) const;

    static void WriteHeader( CORE::CDynamicBuffer&      buf    ,
                             CORE::UInt32&              offset ,
                             const CStoragePubSubIndexDef& def );

    static void WriteIndexEntries( CORE::CDynamicBuffer& buf        ,
                                   CORE::UInt32&         offset     ,
                                   const TUInt64Vector&  keys       ,
                                   const TUInt32Vector&  msgIndexes ,
                                   CORE::UInt32          fileId     );

    void WriteFileRegistry( CORE::CDynamicBuffer&      buf    ,
                            CORE::UInt32&              offset ) const;

    static void WriteFooter( CORE::CDynamicBuffer& buf                ,
                             CORE::UInt32&         offset             ,
                             CORE::UInt32          indexEntryCount    ,
                             CORE::UInt32          indexEntriesOffset ,
                             CORE::UInt32          fileRegistryOffset ,
                             CORE::UInt32          fileRegistryCount  );

    CStoragePubSubIndexDef  m_def;
    CORE::CString           m_dirPath;        /**< VFS directory path (topic root) */
    CORE::CString           m_indexFilePath;  /**< full VFS path to the .sidx file */
    TFileRegEntryVector     m_fileRegistry;   /**< in-memory registry of all indexed container files */
    CORE::UInt32            m_nextFileId;
    CORE::UInt32            m_totalEntryCount;
    bool                    m_initialized;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STORAGE */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXWRITER_H ? */
