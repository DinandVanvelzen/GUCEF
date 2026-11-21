/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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


#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXINFO_H
#define GUCEF_ASSETSTORE_CASSETPACKINDEXINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKGROUP_H
#include "gucefASSETSTORE_CAssetPackGroup.h"
#define GUCEF_ASSETSTORE_CASSETPACKGROUP_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKGROUP_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the service says of the pack index of a pack (the operation GetPackIndex): the row of the table pack_indexes, the
 *  summaries of the groups (pack_groups) and, when the document is there to be read, its manifest as a data tree (identity,
 *  producer, profile, licence flags, counts and the list of shards, in the schema of the pack index design, without the
 *  resource records). The manifest is at most 256 KiB by the rules of the document, so the answer stays far below the
 *  message limit of the service.
 *
 *  'state' is the state of the copy in the catalogue: "loaded", "loading" (a load was interrupted: it is repaired on the
 *  next start), "stale" (the document is loaded, but the pack or the tools are not what the document was made for) or
 *  "failed". A pack without an index has no such row: GetPackIndex answers NOT_FOUND. 'indexKey' is the key a client caches
 *  the manifest under: documents are immutable per index key.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPackIndexInfo : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString StateLoaded;     /**< "loaded" */
    static const CORE::CString StateLoading;    /**< "loading" */
    static const CORE::CString StateStale;      /**< "stale" */
    static const CORE::CString StateFailed;     /**< "failed" */

    CORE::CString packId;
    CORE::CString indexKey;
    CORE::CString packKey;
    UInt32 schemaVersion;
    CORE::CString documentPath;                 /**< the VFS path of the manifest in the service */
    CORE::CString documentHash;
    CORE::CString::StringVector levels;         /**< structure, analysis, outputs, thumbnails */
    CORE::CString state;
    CORE::CString stateReason;
    bool stale;
    UInt64 resourceCount;
    UInt64 noiseCount;
    UInt32 shardCount;
    CORE::CString producer;
    Int64 producedMs;
    Int64 loadedMs;
    TAssetPackGroupVector groups;
    bool hasManifest;                           /**< the document could be read: 'manifest' holds it */
    CORE::CDataNode manifest;

    CAssetPackIndexInfo( void );
    virtual ~CAssetPackIndexInfo() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    /** Whether the licence of the manifest carries the restriction flag (false without a manifest) */
    bool HasLicenceRestriction( const CORE::CString& flag ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXINFO_H ? */
