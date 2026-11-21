/*
 *  AssetConverterLib: reads, checks, builds, analyses and writes the pack index of a pack: the work done once
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXUPDATER_H
#define ASSETCONVERTERLIB_CPACKINDEXUPDATER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETPACKLISTING_H
#include "AssetConverterLib_CAssetPackListing.h"
#define ASSETCONVERTERLIB_CASSETPACKLISTING_H
#endif /* ASSETCONVERTERLIB_CASSETPACKLISTING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#include "AssetConverterLib_CAssetPackProfile.h"
#define ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPROFILE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETKINDREGISTRY_H
#include "AssetConverterLib_CAssetKindRegistry.h"
#define ASSETCONVERTERLIB_CASSETKINDREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETKINDREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXBUILDOPTIONS_H
#include "AssetConverterLib_CPackIndexBuildOptions.h"
#define ASSETCONVERTERLIB_CPACKINDEXBUILDOPTIONS_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXBUILDOPTIONS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXUPDATEOPTIONS_H
#include "AssetConverterLib_CPackIndexUpdateOptions.h"
#define ASSETCONVERTERLIB_CPACKINDEXUPDATEOPTIONS_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXUPDATEOPTIONS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H
#include "AssetConverterLib_CPackIndexRunResult.h"
#define ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H ? */

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
 *  The whole of "work once" for the pack index (design 4): the converter pipeline, the command line and the workbench all call
 *  EnsureIndex() and nothing else.
 *
 *    1. the inputs of now are computed (the listing and the sizes: the fingerprint), the stored document is read;
 *    2. the work gate decides: fresh (nothing is built), stale, missing, forced or refused;
 *    3. when work is needed the writers' lock is taken (CIntermediateLock on pack-index.d: a second writer is refused with the
 *       holder named, and returns the stored document), the stored document is read again under the lock, and the structure
 *       level is built, keeping the records of the files that did not change;
 *    4. the analysis (headers of the files), per record under the gate, and the copy of the outputs of the conversion cache and
 *       the sidecars;
 *    5. the document is written only when it differs from the stored one, shards first, the manifest last.
 *
 *  A corrupt stored document is renamed to pack-index.json.bad once and rebuilt. A document of a newer version is never touched.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexUpdater
{
    public:

    explicit CPackIndexUpdater( const CAssetKindRegistry& kinds );

    /** False when nothing usable could be produced (cancelled, refused, locked without a stored document); the result says which */
    bool EnsureIndex( const CAssetPackListing& listing, const CAssetPackProfile& profile, const CPackIndexBuildOptions& buildOptions,
                      const CPackIndexUpdateOptions& updateOptions, const CORE::CString& indexFolder, CPackIndexDocument& document, CPackIndexRunResult& result );

    /** What EnsureIndex() would do, without doing it: the gate's decision for the index (what --dry-run prints) */
    CPackGateDecision PlanIndex( const CAssetPackListing& listing, const CAssetPackProfile& profile, const CPackIndexBuildOptions& buildOptions,
                                 const CPackIndexUpdateOptions& updateOptions, const CORE::CString& indexFolder );

    /** Copies what the conversion cache entries and the sidecars say into the records; returns the number of records that got outputs */
    UInt32 RefreshOutputs( CPackIndexDocument& document, const CPackIndexUpdateOptions& updateOptions );

    /**
     *  The references of a resource from the normalized relations of a sidecar (pack index design 2.12): per resolved relation kind,
     *  path, via, optional and the resource id of the path with 'idBase'; per unresolved one its via in unresolvedRefs. A sidecar
     *  without the member "relations" (written before converter version 2) leaves the references of the resource as they are; false then
     */
    static bool ReadRelations( const CORE::CDataNode& sidecar, const CORE::CString& idBase, CPackIndexResource& resource );

    /** ReadRelations() from the sidecar of the resource in the intermediate folder; false when there is none or it has no relations */
    static bool ReadSidecarRelations( const CORE::CString& intermediateDir, const CORE::CString& idBase, CPackIndexResource& resource );

    private:

    CPackIndexUpdater( const CPackIndexUpdater& src );              /**< not implemented */
    CPackIndexUpdater& operator=( const CPackIndexUpdater& src );   /**< not implemented */

    void RegroupRefined( CPackIndexDocument& document );

    const CAssetKindRegistry& m_kinds;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXUPDATER_H ? */

