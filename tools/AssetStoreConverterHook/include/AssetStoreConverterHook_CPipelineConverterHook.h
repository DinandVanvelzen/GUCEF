/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINECONVERTERHOOK_H
#define ASSETSTORECONVERTERHOOK_CPIPELINECONVERTERHOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#include "gucefASSETSTORE_CIAssetConverterHook.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#include "AssetConverterLib_CAssetPackManifest.h"
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONRUNNER_H
#include "AssetStoreConverterHook_CPipelineOperationRunner.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONRUNNER_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONRUNNER_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H
#include "AssetStoreConverterHook_CConverterHookSettings.h"
#define ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H
#endif /* ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The conversion step of the primary asset store, on the pipeline of AssetConverterLib (section 7.4 and step L4 of
 *  gucefASSETSTORE_OnlineAssets_Design.md). It sits in a module of its own because gucefASSETSTORE must not depend on
 *  the tools, and AssetConverterLib has no business knowing the store: this module depends on both, and an application
 *  that has both gives the store an instance (CPrimaryAssetStore::SetConverterHook()).
 *
 *  ConvertPack() for a pack:
 *
 *  1. Makes the licence record of the converter from the licence of the pack in the catalogue (CLicenceRecordBuilder)
 *     and applies the rules of the converter to it (CLicenceRecord::AllowsConversion()). A pack without a licence
 *     record, or with ueOnly, referenceOnly or restrictedOrNonStandard without reviewed, is REFUSED with the reason.
 *  2. Builds the manifest of the pack: the original files of the request as the source, the importer "auto" (or the one
 *     of the settings), the exporter, the cook profile of the settings as the one profile and the cook target when there
 *     is one. The work folder is request.convertedVfsFolder, "converted/<profile>" of the pack: "licence/" (the record,
 *     which is also what the report names), "intermediate/" (the engine-neutral files, the cache, the sidecars), the
 *     cooked files in "<pack id>/", "report.json" and "report.txt", "pack.assetpack.json" (the manifest, so the run can be
 *     repeated by hand), "references.json" and "sizes.json".
 *  3. Runs the pipeline (CAssetPipeline::RunPack(), which blocks on its tasks, per asset one task of the pool of the
 *     settings) with the observer of the request: progress per asset, and a cancel request stops the run (CANCELLED).
 *  4. Maps the report: a report with errors is FAILED, what the run wrote is taken away unless the pack was converted
 *     before (an earlier conversion is not damaged by a failed new one), the errors go into the problems; a clean report
 *     is OK, and the assets that converted get convertedPath (the main file of what was made for them, see the settings)
 *     and convertedFormat, the counts, the report path and the versions of the converter parts that were used.
 *
 *  The recorded versions ("importer:gltf/1", "exporter:gltf/1", "cooker:ogre/2", "converter:AssetConverterLib/3", the
 *  default post-processors of the importers) are what IsConversionStale() compares with the parts that are loaded now.
 *
 *  Thread safe: settings are copied under a lock, one conversion at a time per call; conversions of different packs may
 *  run at the same time (the pool of the converter is shared).
 *
 *  The hook also carries out the operations of the converter pipeline for the tools of the asset store service
 *  (SupportsPipelineOperation(), RunPipelineOperation(): every name of CAssetPipelineOperations but render_sheet) on the
 *  converter packs below the roots of its operation settings (SetOperationSettings(), CPipelineOperationRunner). The
 *  thread pool and the threads of the hook settings are those of the runs of these operations too.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineConverterHook : public ASSETSTORE::CIAssetConverterHook
{
    public:

    CPipelineConverterHook( void );
    explicit CPipelineConverterHook( const CConverterHookSettings& settings );
    virtual ~CPipelineConverterHook() GUCEF_VIRTUAL_OVERRIDE;

    void SetSettings( const CConverterHookSettings& settings );
    CConverterHookSettings GetSettings( void ) const;

    /**
     *  Builds the manifest the pipeline gets for the request: exposed so that the manifest of a pack can be checked
     *  (and so that an application can write it next to the pack). 'problem' says why not when it returns false.
     */
    bool BuildManifest( const ASSETSTORE::CAssetConversionRequest& request, ASSETCONVERTER::CAssetPackManifest& manifest, CORE::CString& problem ) const;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetProfileName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanConvert( const ASSETSTORE::CAssetRecord& asset ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual ASSETSTORE::TAssetStoreStatus ConvertPack( const ASSETSTORE::CAssetConversionRequest& request ,
                                                       ASSETSTORE::TAssetRecordVector& assets             ,
                                                       ASSETSTORE::CAssetConversionOutcome& outcome       ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConversionStale( const ASSETSTORE::CAssetConversionRecord& recorded, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The roots and limits of the pipeline operations; without roots there are no packs to work on */
    void SetOperationSettings( const CPipelineOperationSettings& settings );

    CPipelineOperationSettings GetOperationSettings( void ) const;

    /** The runner of the pipeline operations (tests) */
    const CPipelineOperationRunner& GetOperationRunner( void ) const;

    virtual bool SupportsPipelineOperation( const CORE::CString& operation ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual ASSETSTORE::TAssetStoreStatus RunPipelineOperation( const ASSETSTORE::CAssetPipelineRequest& request    ,
                                                                ASSETSTORE::CIAssetConversionObserver* observer     ,
                                                                ASSETSTORE::CAssetServiceDocument& result           ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CPipelineConverterHook( const CPipelineConverterHook& src );              /**< not implemented */
    CPipelineConverterHook& operator=( const CPipelineConverterHook& src );   /**< not implemented */

    private:

    CConverterHookSettings m_settings;
    mutable MT::CMutex m_lock;
    CPipelineOperationRunner m_operations;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINECONVERTERHOOK_H ? */
