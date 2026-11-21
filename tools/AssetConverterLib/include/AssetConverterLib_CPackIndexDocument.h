/*
 *  AssetConverterLib: the normalized pack index document (gucef.packIndex, version 1) as a typed model
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKINDEXIDENTITY_H
#include "AssetConverterLib_CPackIndexIdentity.h"
#define ASSETCONVERTERLIB_CPACKINDEXIDENTITY_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXIDENTITY_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXPRODUCER_H
#include "AssetConverterLib_CPackIndexProducer.h"
#define ASSETCONVERTERLIB_CPACKINDEXPRODUCER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXPRODUCER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXPROFILE_H
#include "AssetConverterLib_CPackIndexProfile.h"
#define ASSETCONVERTERLIB_CPACKINDEXPROFILE_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXPROFILE_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXLICENCE_H
#include "AssetConverterLib_CPackIndexLicence.h"
#define ASSETCONVERTERLIB_CPACKINDEXLICENCE_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXLICENCE_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXCOUNTS_H
#include "AssetConverterLib_CPackIndexCounts.h"
#define ASSETCONVERTERLIB_CPACKINDEXCOUNTS_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXCOUNTS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXGROUP_H
#include "AssetConverterLib_CPackIndexGroup.h"
#define ASSETCONVERTERLIB_CPACKINDEXGROUP_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXGROUP_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXSHARDINFO_H
#include "AssetConverterLib_CPackIndexShardInfo.h"
#define ASSETCONVERTERLIB_CPACKINDEXSHARDINFO_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSHARDINFO_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXRESOURCE_H
#include "AssetConverterLib_CPackIndexResource.h"
#define ASSETCONVERTERLIB_CPACKINDEXRESOURCE_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXRESOURCE_H ? */

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
 *  The pack index of one pack: the manifest (identity, producer, profile, licence, counts, group summaries, shard list) and the
 *  resource records. In memory the records are always in 'resources'; whether they are written inline or in shards is the choice
 *  of CPackIndexWriter (up to MaxInlineResources inline). The members follow the schema of the pack index design, section 2.4:
 *  the format id and version, the levels that are complete, the state of the build and the reasons for a state other than
 *  complete. The documentHash is the hash of the canonical bytes of the manifest without the produced block and without the
 *  hash itself, so a rebuild that gives the same result gives the same hash whoever ran it and when.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexDocument
{
    public:

    static const CORE::CString FormatId;            /**< "gucef.packIndex" */
    static const UInt32 SchemaVersion = 1;
    static const UInt32 IndexerVersion = 2;         /**< raised whenever the output for the same input changes; 2: refs from the sidecar relations */
    static const UInt32 MaxInlineResources = 2000;  /**< above this the resources go to shards */
    static const UInt32 MaxResources = 200000;      /**< a pack with more is refused */

    static const CORE::CString StateBuilding;
    static const CORE::CString StatePartial;
    static const CORE::CString StateComplete;
    static const CORE::CString StateCompleteWithFailures;
    static const CORE::CString StateFailed;

    static const CORE::CString LevelStructure;
    static const CORE::CString LevelAnalysis;
    static const CORE::CString LevelOutputs;
    static const CORE::CString LevelThumbnails;

    UInt32 version;
    CORE::CString::StringVector levels;
    CORE::CString state;
    CORE::CString::StringVector stateReasons;
    CPackIndexIdentity identity;
    CPackIndexProducer producer;
    CPackIndexProfile profile;
    CPackIndexLicence licence;
    CPackIndexCounts counts;
    CPackIndexGroup::TGroupVector groups;
    CPackIndexShardInfo::TShardVector shards;
    CPackIndexResource::TResourceVector resources;
    CORE::CString documentHash;

    // the produced block: volatile, never part of the hash
    CORE::CString producedUtc;
    UInt64 producedDurationMs;
    CORE::CString producedHost;
    CORE::CString producedJobId;

    CPackIndexDocument( void );

    void Clear( void );

    /** Whether the state says the build finished (complete, or complete with failures) */
    bool IsComplete( void ) const;
    bool HasLevel( const CORE::CString& level ) const;
    void AddLevel( const CORE::CString& level );

    /** Sorts the records, the group summaries and the levels into the canonical order */
    void Normalise( void );

    /** Recounts the counts and the group summaries from the records: the manifest never says anything the records do not */
    void UpdateSummaries( void );

    const CPackIndexResource* FindResourceByPath( const CORE::CString& path ) const;
    const CPackIndexResource* FindResourceById( const CORE::CString& resourceId ) const;
    const CPackIndexGroup* FindGroup( const CORE::CString& groupId ) const;

    /** The manifest as a data tree. The resources are written inline when asked; 'withProduced' adds the produced block and the hash */
    bool SerializeManifest( CORE::CDataNode& node, bool withResources, bool withProducedAndHash ) const;

    /** Reads the manifest members; an inline resource list is read too. Members this build does not know are ignored */
    bool DeserializeManifest( const CORE::CDataNode& node );

    /** The hash of the canonical manifest (with the records inline when the document has no shards) without produced and hash */
    CORE::CString ComputeDocumentHash( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

