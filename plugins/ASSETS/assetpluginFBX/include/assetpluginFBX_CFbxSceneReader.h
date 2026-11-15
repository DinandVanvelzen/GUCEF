/*
 *  assetpluginFBX: reads an FBX file into a scene of the converter model
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

#ifndef GUCEF_ASSETPLUGIN_FBX_CFBXSCENEREADER_H
#define GUCEF_ASSETPLUGIN_FBX_CFBXSCENEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_FBX_CFBXDOCUMENT_H
#include "assetpluginFBX_CFbxDocument.h"
#define GUCEF_ASSETPLUGIN_FBX_CFBXDOCUMENT_H
#endif /* GUCEF_ASSETPLUGIN_FBX_CFBXDOCUMENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace FBX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads an FBX file into the model: the node hierarchy, meshes with skins and blend shapes
 *  (CFbxMeshReader), materials with their textures (CFbxMaterialReader), animation takes as clips
 *  (CFbxAnimationReader), lights and LOD groups.
 *
 *  The scene is left in the coordinate system of the file: its axes and its unit (the length of one unit of the
 *  file in metres) become the coordinate system of the scene, which the normalisation of the pipeline converts. The
 *  facts of the file that have no place in the model (version, creator, frame rate, the warnings of ufbx) go to the
 *  extras of the scene and so to the sidecar.
 *
 *  A light points along the forward axis of the scene in the model: where the light of the file points elsewhere
 *  (usually down), the light sits on a child node with the rotation that turns it that way.
 */
class GUCEF_ASSETPLUGIN_FBX_EXPORT_CPP CFbxSceneReader
{
    public:

    CFbxSceneReader( const CORE::CString& vfsPath                   ,
                     ASSETCONVERTER::CAssetPackContext& context     ,
                     ASSETCONVERTER::CAssetScene& scene             ,
                     ASSETCONVERTER::CConversionReport& report      );

    ~CFbxSceneReader();

    bool Read( void );

    private:

    CFbxSceneReader( const CFbxSceneReader& src );              /**< not implemented */
    CFbxSceneReader& operator=( const CFbxSceneReader& src );   /**< not implemented */

    void ReadFacts( const ufbx_scene& fbx );
    void ReadCoordinateSystem( const ufbx_scene& fbx );
    void ReadNode( const ufbx_scene& fbx, const ufbx_node& node, Int32 parent );
    void ReadMeshes( const ufbx_scene& fbx );
    void ReadLights( const ufbx_scene& fbx );
    void ReadLodGroups( const ufbx_scene& fbx );

    private:

    CORE::CString m_vfsPath;
    ASSETCONVERTER::CAssetPackContext& m_context;
    ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    GUCEF::vector< Int32 > m_nodeMap;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FBX */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_FBX_CFBXSCENEREADER_H ? */
