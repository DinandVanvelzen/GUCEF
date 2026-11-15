/*
 *  AssetConverterLib: the conversion of one asset: its input and its results, the data of its task
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

#ifndef ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H
#define ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef ASSETCONVERTERLIB_CIASSETIMPORTER_H
#include "AssetConverterLib_CIAssetImporter.h"
#define ASSETCONVERTERLIB_CIASSETIMPORTER_H
#endif /* ASSETCONVERTERLIB_CIASSETIMPORTER_H ? */

#ifndef ASSETCONVERTERLIB_CIASSETEXPORTER_H
#include "AssetConverterLib_CIAssetExporter.h"
#define ASSETCONVERTERLIB_CIASSETEXPORTER_H
#endif /* ASSETCONVERTERLIB_CIASSETEXPORTER_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CIMPORTACTIVITYENTRY_H
#include "AssetConverterLib_CImportActivityEntry.h"
#define ASSETCONVERTERLIB_CIMPORTACTIVITYENTRY_H
#endif /* ASSETCONVERTERLIB_CIMPORTACTIVITYENTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#include "AssetConverterLib_CAssetCacheEntry.h"
#define ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#endif /* ASSETCONVERTERLIB_CASSETCACHEENTRY_H ? */


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

class CAssetPackManifest;
class CAssetPackContext;
class CTextureBaker;

/*-------------------------------------------------------------------------*/

/**
 *  The conversion of one asset by CAssetPipeline::ConvertAsset(): what it needs of the run (the manifest, the context
 *  of the pack, the texture baker, the importer and exporter, the versions of the cookers, the hash of the processing
 *  settings) and what it gives back (its own report, its activity entry, its cache entry, whether it was cached,
 *  converted and may be cooked). It is the task data of the asynchronous form; the objects it points to belong to
 *  the run and outlive the task.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetConversionJob : public CORE::CICloneable
{
    public:

    // input
    const CAssetPackManifest* manifest;
    CAssetPackContext* context;
    CTextureBaker* baker;
    TAssetImporterPtr importer;
    TAssetExporterPtr exporter;
    CAssetCacheEntry::TPluginVersionMap cookerVersions;    /**< "cooker:<target>" and version, for the cache key */
    CORE::CString processingHash;
    UInt32 maxBones;
    bool force;
    bool reimport;
    bool validateOnly;                  /**< import, normalise, post-process and validate only: no cache check, no export, nothing written */
    CORE::CString assetPath;

    // results
    CConversionReport report;
    CImportActivityEntry activity;
    CAssetCacheEntry cacheEntry;
    bool cached;
    bool converted;
    bool cookAllowed;
    bool cookPending;                   /**< cached, but the cooking of an earlier run did not happen: cook it in this run */

    CAssetConversionJob( void );

    CAssetConversionJob( const CAssetConversionJob& src );

    virtual ~CAssetConversionJob() GUCEF_VIRTUAL_OVERRIDE;

    CAssetConversionJob& operator=( const CAssetConversionJob& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Type check of task data without RTTI, see CICloneable::CloneableAs() */
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H ? */
