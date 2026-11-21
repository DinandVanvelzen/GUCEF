/*
 *  assetpluginUNREAL: Brings the materials of the slots of a mesh into the scene: resolves the instance chains and maps the parameters
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALIMPORTER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALIMPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H
#include "assetpluginUNREAL_CUnrealSkeletalMesh.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#include "assetpluginUNREAL_CUnrealObjectPathResolver.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  For each material slot of a mesh: finds the package of the material through the object path resolver, resolves the
 *  parent chain of the instance (CUnrealMaterialInstanceReader), maps the parameters to a material of the scene
 *  (CUnrealMaterialMapper) and reports what it left out. A material that two meshes use is added once. A material that is
 *  not in the pack (engine content) becomes a default material with the name of the slot, and the report says so.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealMaterialImporter
{
    public:

    /** @param slotMaterials receives per slot the index of the material in the scene, -1 for none */
    static void AddMaterialsOfMesh( const CUnrealPackage& package                 ,
                                    const CUnrealSkeletalMesh& mesh               ,
                                    const CUnrealObjectPathResolver& resolver     ,
                                    ASSETCONVERTER::CAssetScene& scene            ,
                                    ASSETCONVERTER::CConversionReport& report     ,
                                    GUCEF::vector< Int32 >& slotMaterials         );

    /** The same for the material slots of any mesh (static meshes) */
    static void AddMaterialsOfSlots( const CUnrealPackage& package                                  ,
                                     const CUnrealSkeletalMesh::TMaterialSlotVector& materialSlots  ,
                                     const CUnrealObjectPathResolver& resolver                      ,
                                     ASSETCONVERTER::CAssetScene& scene                             ,
                                     ASSETCONVERTER::CConversionReport& report                      ,
                                     GUCEF::vector< Int32 >& slotMaterials                          );

    private:

    CUnrealMaterialImporter( void );    /**< only static members */
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALIMPORTER_H ? */
