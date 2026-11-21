/*
 *  AssetConverterLib: builds the structure level of a pack index from a pack listing
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXBUILDER_H
#define ASSETCONVERTERLIB_CPACKINDEXBUILDER_H

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
 *  The structure level of the pack index (design 2.7): a file listing, the sizes of the files, the kind registry and the rules,
 *  and no file contents. The recogniser has run already (the caller gives its profile), so the listing is read once.
 *
 *  Two steps, so that the work gate can decide before the grouping is paid for:
 *
 *  - PrepareInputs() reads the sizes and computes what the index key depends on: the identity with the fingerprint, the
 *    producer with the versions and the key. The result is a document without resources: what the index of this pack would
 *    say about itself if built now.
 *  - BuildStructure() assigns kind, group and sub-group to every file, attaches the metadata files to their owners, counts
 *    the noise and writes the document: all but the writing to the disk (CPackIndexWriter). A previous document of the same
 *    pack is used to keep the analysis records, outputs and thumbnails of the files whose size did not change.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexBuilder
{
    public:

    explicit CPackIndexBuilder( const CAssetKindRegistry& kinds );

    /** Gives the size of a file of a listing that has no file system behind it (a synthetic listing); wins over the VFS */
    void SetFileSize( const CORE::CString& path, UInt64 size );

    /** The identity and producer of the pack as it is now; no resources */
    bool PrepareInputs( const CAssetPackListing& listing, const CAssetPackProfile& profile, const CPackIndexBuildOptions& options, CPackIndexDocument& inputs );

    /** The structure level; 'previous' (optional) lends the records of files that did not change. False when cancelled or the pack is over the limit (GetError() says why) */
    bool BuildStructure( const CAssetPackListing& listing, const CAssetPackProfile& profile, const CPackIndexBuildOptions& options, CPackIndexDocument& document, const CPackIndexDocument* previous );

    const CORE::CString& GetError( void ) const;

    /** How many resources the last BuildStructure() grouped (a counter for the tests: the work that was done) */
    UInt32 GetGroupedCount( void ) const;

    /** How many records the last BuildStructure() took over from the previous document */
    UInt32 GetKeptCount( void ) const;

    /** The hash of the extension table of the kind registry: its content version */
    static CORE::CString GetKindRegistryHash( const CAssetKindRegistry& kinds );

    private:

    typedef GUCEF::map< CORE::CString, UInt64 > TSizeMap;

    CPackIndexBuilder( const CPackIndexBuilder& src );              /**< not implemented */
    CPackIndexBuilder& operator=( const CPackIndexBuilder& src );   /**< not implemented */

    const CAssetKindRegistry& m_kinds;
    TSizeMap m_givenSizes;
    TSizeMap m_sizes;
    CPackIndexDocument m_inputs;
    CORE::CString m_preparedFor;
    bool m_prepared;
    UInt32 m_groupedCount;
    UInt32 m_keptCount;
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

#endif /* ASSETCONVERTERLIB_CPACKINDEXBUILDER_H ? */

