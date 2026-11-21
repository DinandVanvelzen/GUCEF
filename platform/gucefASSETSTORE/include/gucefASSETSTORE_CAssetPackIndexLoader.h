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


#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H
#define GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#include "gucefASSETSTORE_CAssetCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXINFO_H
#include "gucefASSETSTORE_CAssetPackIndexInfo.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXINFO_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXINFO_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H
#include "gucefASSETSTORE_CAssetPackIndexOutcome.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKRESOURCE_H
#include "gucefASSETSTORE_CAssetPackResource.h"
#define GUCEF_ASSETSTORE_CASSETPACKRESOURCE_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRESOURCE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H
#include "gucefASSETSTORE_CAssetKeywordNormalizer.h"
#define GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H
#endif /* GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H ? */

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

typedef GUCEF::vector< CAssetPackResource > TAssetPackResourceVector;

/**
 *  The loader of the pack index (section 3.4 of the pack index design): it reads the document of a pack
 *  ("pack-index.json" with its shards, as the converter writes it) and writes its derived copy into the catalogue, the tables
 *  pack_indexes, pack_groups and pack_resources of schema version 4, and what the document adds to tables that exist: the
 *  references into relations, the names of resources that are no assets into keywords, the statistics of assets that have
 *  none into the typed property tables. The document is the source of truth: the rows are replaced from it, the loader never
 *  writes into a document, and the catalogue can be rebuilt from the documents of a location.
 *
 *  The store does not depend on the converter, so the document is read as a generic data tree against the schema of the
 *  design (section 2.4), not through the typed model of AssetConverterLib: the schema is the contract. A document of another
 *  format, of a newer major version, with a shard whose hash does not match, or without identity is refused with the reason
 *  and nothing is written. Members this build does not know are ignored.
 *
 *  Load() is idempotent: a document whose hash is the one the catalogue holds (and whose rows are complete) is not loaded
 *  again unless forced. The rows are replaced in one transaction where the backend has them; without, the row of
 *  pack_indexes says "loading" until the others are written, and a pack found in "loading" is loaded again when the store opens.
 *  The state is "stale" when the document lists fewer files or bytes than the pack record has now (it may list more: the store
 *  leaves the descriptor of a pack and hidden files out of its counts): the rows are loaded all the same, so the search keeps
 *  working while a refresh is made. What else makes a document stale (another version of a tool, changed settings) only the
 *  gate of the converter knows: IndexPack() with the option verify asks it, MarkStale() records an answer.
 *
 *  Resource ids are those of the store: "as" and the first 20 hexadecimal digits of the SHA-256 of the pack id, a line feed
 *  and the path in the pack (MakeResourceId()), which for a resource that is an asset is its asset id. The ids of the
 *  document, made with another base, are mapped to them, also in the references.
 *
 *  The caller serialises writers of one catalogue (CPrimaryAssetStore::GetIndexLock()). Reading is always allowed.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPackIndexLoader
{
    public:

    static const CORE::CString FormatId;                    /**< "gucef.packIndex" */
    static const UInt32 MaxSchemaVersion = 1;               /**< the highest version of the document this build reads */
    static const UInt32 MaxResources = 200000;              /**< a document with more records is refused, as the document design says */

    CAssetPackIndexLoader( CAssetCatalogue& catalogue, const CAssetKeywordNormalizer* normalizer = GUCEF_NULL );

    /** The id the store gives the resource at the path of the pack: the asset id when the resource is an asset */
    static CORE::CString MakeResourceId( const CORE::CString& packId, const CORE::CString& path );

    /** Whether the catalogue has the tables of the pack index (schema version 4) */
    bool IsAvailable( void ) const;

    /**
     *  Loads the document at the VFS path of the manifest as the index of the pack, which has to be in the catalogue. 'force'
     *  loads it also when the catalogue holds this document. The outcome says how it went: loaded or unchanged, with the
     *  document hash, the index key and the number of resources. OK is also the status of an unchanged document
     */
    TAssetStoreStatus Load( const CORE::CString& packId, const CORE::CString& manifestVfsPath, bool force, CAssetPackIndexOutcome& outcome );

    /** Takes the rows of the pack index of a pack out (the document stays) */
    TAssetStoreStatus Remove( const CORE::CString& packId );

    /** Marks the loaded index of the pack stale with the reason; NOT_FOUND when it has none */
    TAssetStoreStatus MarkStale( const CORE::CString& packId, const CORE::CString& reason );

    /** The row of the pack index of the pack with its groups; with 'withManifest' also the manifest read from the document (without the records); NOT_FOUND when the pack has no index */
    TAssetStoreStatus GetInfo( const CORE::CString& packId, bool withManifest, CAssetPackIndexInfo& info ) const;

    /** Reads the manifest of the document at the VFS path into the info (without the records): hasManifest, manifest; the other members are left as they are */
    TAssetStoreStatus ReadManifest( const CORE::CString& manifestVfsPath, CAssetPackIndexInfo& info ) const;

    /** The group rows of the pack: the function groups in the fixed order of the vocabulary, each followed by its sub-groups by name */
    TAssetStoreStatus GetGroups( const CORE::CString& packId, TAssetPackGroupVector& groups ) const;

    /** The resources of the pack in the canonical order (group in the fixed order, sub-group, path); with a group id only the resources of the group, with a sub-group only those of it */
    TAssetStoreStatus GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup, TAssetPackResourceVector& resources ) const;

    /** One resource, by its id */
    TAssetStoreStatus GetResource( const CORE::CString& resourceId, CAssetPackResource& resource ) const;

    /** The ids of the packs that have a pack index row in this state */
    TAssetStoreStatus GetPacksInState( const CORE::CString& state, CORE::CString::StringVector& packIds ) const;

    /** The ids of the packs of the catalogue that have no pack index row */
    TAssetStoreStatus GetPacksWithoutIndex( CORE::CString::StringVector& packIds ) const;

    /** The position of a function group in the fixed order of the vocabulary; groups that are not in it come after, by name */
    static UInt32 GetGroupOrder( const CORE::CString& groupId );

    /** The message of the last failure */
    const CORE::CString& GetLastErrorMessage( void ) const;

    private:

    struct SDocument;

    TAssetStoreStatus ReadDocument( const CORE::CString& manifestVfsPath, SDocument& document, CORE::CString& message ) const;
    TAssetStoreStatus WriteRows( const CORE::CString& packId, const CORE::CString& manifestVfsPath, const SDocument& document, const CORE::CString& state, const CORE::CString& reason, Int64 loadedMs, CORE::CString& message );
    TAssetStoreStatus Replace( const CORE::CString& packId, const CORE::CString& manifestVfsPath, const SDocument& document, const CORE::CString& state, const CORE::CString& reason, Int64 loadedMs, CORE::CString& message );
    TAssetStoreStatus Fail( TAssetStoreStatus status, const CORE::CString& message );

    CAssetPackIndexLoader( const CAssetPackIndexLoader& src );              /**< not implemented */
    CAssetPackIndexLoader& operator=( const CAssetPackIndexLoader& src );   /**< not implemented */

    private:

    CAssetCatalogue& m_catalogue;
    const CAssetKeywordNormalizer* m_normalizer;
    CORE::CString m_lastError;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H ? */
