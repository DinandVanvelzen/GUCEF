/*
 *  AssetConverterLib: writes a pack index document in the canonical form, inline or sharded
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXWRITER_H
#define ASSETCONVERTERLIB_CPACKINDEXWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The canonical writer of the pack index. The document is put in the canonical order (Normalise()), written through
 *  CCanonicalJson (sorted keys, round-trip numbers, LF, one trailing newline), so the same document written twice is
 *  byte identical, and so is the same document built from the same records in another order.
 *
 *  Up to CPackIndexDocument::MaxInlineResources records go into the manifest. Above that the records go to shards in
 *  pack-index.d, cut at group boundaries first and at ShardSize records within a group (design 2.8); a shard file is named
 *  shard-NNNN-<first 12 hex of its hash>.json, so a new shard never overwrites a file an older manifest points at and an
 *  unchanged shard is not written again. The manifest is written last, through a temporary file and an atomic rename where
 *  the VFS allows one, and the shard files of the previous manifest that the new one does not name are removed after it,
 *  so a reader sees either the old index or the new one. The lock of the writers is the caller's business
 *  (CIntermediateLock on the shard folder, see CPackIndexUpdater).
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexWriter
{
    public:

    CPackIndexWriter( void );

    /** The records per shard; MaxInlineResources by default */
    void SetShardSize( UInt32 recordsPerShard );

    /** Writes the document to the index folder (a VFS path); on success 'document' has its documentHash, its shard list and its canonical order */
    bool Write( CPackIndexDocument& document, const CORE::CString& indexFolder );

    /** What the last Write() said when it failed */
    const CORE::CString& GetError( void ) const;

    UInt32 GetShardsWritten( void ) const;
    UInt32 GetShardsReused( void ) const;

    private:

    bool StoreNode( const CORE::CDataNode& node, const CORE::CString& vfsPath );
    void RemoveObsoleteShards( const CORE::CString& indexFolder, const CPackIndexShardInfo::TShardVector& keep );

    UInt32 m_shardSize;
    UInt32 m_shardsWritten;
    UInt32 m_shardsReused;
    CORE::CString m_error;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXWRITER_H ? */

