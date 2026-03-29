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

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  Describes a single sidecar index to build alongside container files.
 *  One definition produces one .sidx file per topic root directory.
 *
 *  Config node (child of the topic config node):
 *    <index keySource="mk" keyName="fix_seq_num" indexType="su64" samplingInterval="1" />
 *
 *  keySource:  "mk" = metadata key-value pair, "k" = message key-value pair
 *  indexType:  "su64" = sorted uint64, "si64" = sorted int64, "sstr" = sorted string
 *  samplingInterval: 1 = every message, N = every N-th message
 */
class PUBSUBPLUGIN_STORAGE_PLUGIN_PRIVATE_CPP CStoragePubSubIndexDef
{
    public:

    enum EKeySource
    {
        KEY_SOURCE_META_DATA = 0,
        KEY_SOURCE_KV_PAIR   = 1
    };

    enum EIndexType
    {
        INDEX_TYPE_SORTED_UINT64 = 1,
        INDEX_TYPE_SORTED_INT64  = 2,
        INDEX_TYPE_SORTED_STRING = 3
    };

    EKeySource    keySource;         /**< where to look for the key: metadata or KV pairs */
    CORE::CString keyName;           /**< key name to index on, e.g. "fix_seq_num" */
    EIndexType    indexType;         /**< sort/key type for the index entries */
    CORE::UInt32  samplingInterval;  /**< 1=every msg, N=every N-th msg */

    CORE::CString GetIndexFilename( void ) const;  /**< e.g. "idx.mk.fix_seq_num.su64.sidx" */
    CORE::CString GetTypeCode( void ) const;       /**< e.g. "su64" */
    CORE::UInt16  GetEntrySize( void ) const;      /**< bytes per index entry: 16 for su64/si64 */

    bool LoadConfig( const CORE::CDataNode& config );
    bool SaveConfig( CORE::CDataNode& config ) const;

    CStoragePubSubIndexDef( void );
    CStoragePubSubIndexDef( const CStoragePubSubIndexDef& src );
    CStoragePubSubIndexDef& operator=( const CStoragePubSubIndexDef& src );
    bool operator==( const CStoragePubSubIndexDef& other ) const;
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

#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H ? */
