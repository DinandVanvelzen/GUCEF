/*
 *  assetpluginUNREAL: Static meshes into the intermediate scene
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHCONVERTER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHCONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H
#include "assetpluginUNREAL_CUnrealStaticMesh.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H ? */

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

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
 *  Puts a static mesh into the scene: a node for the mesh with one child node per level of detail (a LOD group when there are
 *  several), a mesh per level with one sub mesh per section and the material of its slot. The source system is declared in the
 *  scene as the engine states it (see GetCoordinateSystem()); the normaliser converts it, with the triangle order of the source
 *  turned as CWindingOfSource() says.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealStaticMeshConverter
{
    public:

    /**
     *  The axes of a static mesh in the source system: X right, Z up, the front of the model toward -Y (left handed), centimetres. The
     *  engine itself has X forward; the owned packs face -Y as the cross-engine check of the tests shows: the Shops models of the Unreal,
     *  Godot and FBX forms of the pack have the same bounds and orientation after this conversion.
     */
    static MATH::CCoordinateSystem GetCoordinateSystem( void );

    /**
     *  True when the triangles of the source (the order of the corners of the mesh description) are clockwise in the source system
     *  as the engine draws them, and have to be turned to the counter-clockwise order the intermediate stores.
     */
    static bool TurnsTriangles( void );

    static bool AddToScene( const CUnrealStaticMesh& mesh                  ,
                            const GUCEF::vector< Int32 >& slotMaterials    ,
                            ASSETCONVERTER::CAssetScene& scene             ,
                            ASSETCONVERTER::CConversionReport& report      ,
                            UInt32& rootNode                               );

    private:
    CUnrealStaticMeshConverter( void ); /**< only static members */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHCONVERTER_H ? */
