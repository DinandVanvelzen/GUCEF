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

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXREADER_H
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H
#include "pubsubpluginSTORAGE_CStoragePubSubIndexDef.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H
#include "pubsubpluginSTORAGE_CStoragePubSubClientTopic.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H ? */

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
 *  Loads a .sidx sidecar index file into memory and provides O(log N)
 *  lookups that translate a logical key value into a
 *  CStoragePubSubClientTopic::CStorageBookmarkInfo.
 *
 *  Typical usage:
 *    CStoragePubSubIndexReader reader( def, topicRootDirPath );
 *    if ( reader.LoadIndex() )
 *    {
 *        CStoragePubSubClientTopic::CStorageBookmarkInfo bm;
 *        if ( reader.FindStartBookmark( 1000, bm ) )
 *            topic.SubscribeStartingAtBookmarkInfo( bm );
 *    }
 */
class PUBSUBPLUGIN_STORAGE_PLUGIN_PRIVATE_CPP CStoragePubSubIndexReader
{
    public:

    CStoragePubSubIndexReader( const CStoragePubSubIndexDef& def     ,
                               const CORE::CString&          dirPath );

    ~CStoragePubSubIndexReader( void );

    /**
     *  Loads the .sidx file from VFS into memory.
     *  Must be called before FindStartBookmark().
     *  Safe to call multiple times (reloads each time).
     */
    bool LoadIndex( void );

    /**
     *  Returns the bookmark for the first index entry whose key >= startKeyValue.
     *  The offsetInFile field is set to 0; the existing read path derives the
     *  actual byte offset from the container's TMsgOffsetIndex when loading.
     *
     *  @param startKeyValue  The lower-bound key to search for.
     *  @param bookmark       Output bookmark pointing to the matching container + msg.
     *  @return true if a matching entry was found.
     */
    bool FindStartBookmark( CORE::UInt64                                       startKeyValue ,
                            CStoragePubSubClientTopic::CStorageBookmarkInfo&   bookmark      ) const;

    bool IsLoaded( void ) const;

    /**
     *  Returns the full VFS paths (in registry order) for all container files starting from
     *  (and including) the file whose path matches startFilePath.
     *  Useful for building the ordered file list for a replay task.
     *  Returns false if startFilePath is not found in the registry.
     */
    typedef GUCEF::vector< CORE::CString, gucef_allocator< CORE::CString > > TStringVector;
    bool GetOrderedFilePathsFromFile( const CORE::CString& startFilePath ,
                                      TStringVector&        outPaths      ) const;

    private:

    struct IndexEntry
    {
        CORE::UInt64 keyValue;
        CORE::UInt32 fileId;
        CORE::UInt32 msgIndex;
    };

    struct FileRegEntry
    {
        CORE::UInt32  fileId;
        CORE::UInt64  firstKey;
        CORE::UInt64  lastKey;
        CORE::UInt32  entryCount;
        CORE::CString filename;
    };

    typedef GUCEF::vector< IndexEntry,   gucef_allocator< IndexEntry   > > TIndexEntryVector;
    typedef GUCEF::vector< FileRegEntry, gucef_allocator< FileRegEntry > > TFileRegEntryVector;

    CStoragePubSubIndexDef  m_def;
    CORE::CString           m_indexFilePath;
    CORE::CString           m_dirPath;
    TFileRegEntryVector     m_fileRegistry;
    TIndexEntryVector       m_entries;
    bool                    m_loaded;
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

#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXREADER_H ? */
