/*
 *  AssetConverterLib: the sidecar of an asset of the intermediate: what glTF cannot hold
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

#ifndef ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#define ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORTENTRY_H
#include "AssetConverterLib_CConversionReportEntry.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORTENTRY_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORTENTRY_H ? */


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

class CAssetPackContext;

/*-------------------------------------------------------------------------*/

/**
 *  The sidecar <asset>.conversion.json next to the glTF of an asset: everything the intermediate keeps that glTF
 *  cannot hold, written deterministically (CCanonicalJson, no times):
 *
 *  - "provenance": pack, asset path, source format and version, source id, engine, importer and converter versions;
 *  - "licenceRecord": the identifier of the licence record of the pack;
 *  - "settings": the effective settings, and "settingsLayers": the layers that made them;
 *  - "normalisation": what the normalisation did;
 *  - "materials": the resolved values with the source material (shader and every original property);
 *  - "textures": sources (paths in the pack), operation chains and baked files (relative to the intermediate);
 *  - "lights", "colliders" (per node), "lodGroups", "prefabs" with the overrides of nested instances, "extras";
 *  - "particles" (only when the asset has particle systems): every CAssetParticleSystem with "node" and "nodeIndex",
 *    the node it is attached to (the glTF node has "gucefParticleSystem" with the index into this list in its extras);
 *  - "lossySteps": every approximation and skipped item of the report of the asset;
 *  - "summary": counts, bounds, bones and clips (CAssetSceneSummary), for the fidelity tests;
 *  - "dependencies": the files the import read besides the source (paths in the pack).
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CConversionSidecar
{
    public:

    /** <intermediate>/<asset path without its extension>.conversion.json */
    static CORE::CString GetPath( const CORE::CString& intermediateDir, const CORE::CString& assetPath );

    static void Build( const CAssetScene& scene                     ,
                       const CConversionReportEntry& entry          ,
                       const CORE::CString::StringVector& layers    ,
                       const CORE::CString& licenceRecordId         ,
                       const CAssetPackContext& context             ,
                       const CORE::CString& intermediateDir         ,
                       CORE::CDataNode& sidecar                     );

    /**
     *  Reads the "particles" list of a sidecar into 'systems' (in the order of the list, so the indices are those of
     *  the glTF node extras). 'nodes' receives the node index of each system (-1 when it has none). False when an entry
     *  cannot be read; a sidecar without the list gives an empty result.
     */
    static bool ReadParticleSystems( const CORE::CDataNode& sidecar                            ,
                                     GUCEF::vector< CAssetParticleSystem >& systems             ,
                                     GUCEF::vector< Int32 >& nodes                              );

    private:

    CConversionSidecar( void );
    ~CConversionSidecar();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H ? */
