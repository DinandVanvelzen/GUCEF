/*
 *  assetpluginGLTF: reads a glTF or GLB file into a scene of the converter model
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEREADER_H
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEREADER_H

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

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFDOCUMENT_H
#include "assetpluginGLTF_CGltfDocument.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFDOCUMENT_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFDOCUMENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads a glTF 2.0 file into the model. cgltf parses the file and decodes the accessors, the JSON of the file is
 *  read as a data tree for what cgltf does not keep: the data of the extensions and the extras.
 *
 *  - Meshes keep every vertex stream, all joint and weight sets (more than four influences), the morph targets
 *    and their names and default weights. Triangle strips and fans become triangles; points and lines are skipped
 *    and reported.
 *  - Images in files become the sources of textures. Images inside the file (GLB buffer views, data URIs) are
 *    written next to the intermediate ("embedded/<asset>/") and used as sources from there.
 *  - The extensions of the intermediate come back as they were written (CGltfSceneWriter): MSFT_lod as LOD groups,
 *    OMI_physics_shape and OMI_physics_body as colliders, KHR_lights_punctual as lights, KHR_texture_transform,
 *    KHR_materials_emissive_strength and KHR_materials_unlit in the materials. Other extension data of a material
 *    is kept in its extensions node. KHR_materials_pbrSpecularGlossiness is approximated as metal-roughness.
 *  - The scene is in the coordinate system of glTF, which is that of the intermediate.
 */
class GUCEF_ASSETPLUGIN_GLTF_EXPORT_CPP CGltfSceneReader
{
    public:

    CGltfSceneReader( const CORE::CString& vfsPath                      ,
                      ASSETCONVERTER::CAssetPackContext& context        ,
                      ASSETCONVERTER::CAssetScene& scene                ,
                      ASSETCONVERTER::CConversionReport& report         );

    ~CGltfSceneReader();

    bool Read( void );

    private:

    CGltfSceneReader( const CGltfSceneReader& src );                /**< not implemented */
    CGltfSceneReader& operator=( const CGltfSceneReader& src );     /**< not implemented */

    typedef GUCEF::vector< Float32 > TFloatVector;
    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    void ReadAsset( void );
    void ReadTextures( void );
    CORE::CString ExtractEmbeddedImage( const cgltf_image& image, UInt32 index );
    void ReadMaterials( void );
    void ReadTextureReference( const cgltf_texture_view& view, ASSETCONVERTER::CAssetTextureReference& reference, ASSETCONVERTER::TTextureUsage usage );
    void ReadMeshes( void );
    bool ReadPrimitive( const cgltf_mesh& mesh, UInt32 meshIndex, UInt32 primitiveIndex, ASSETCONVERTER::CAssetSubMesh& subMesh );
    void ReadMorphTargets( const cgltf_mesh& mesh, const cgltf_primitive& primitive, ASSETCONVERTER::CAssetSubMesh& subMesh );
    void ReadLights( void );
    void ReadNodes( void );
    void ReadSkins( void );
    void ReadAnimations( void );
    void ReadLodGroups( void );
    void ReadColliders( void );
    void ReadDependencies( void );
    bool ReadFloats( const cgltf_accessor* accessor, UInt32 components, TFloatVector& values ) const;

    private:

    CORE::CString m_vfsPath;
    ASSETCONVERTER::CAssetPackContext& m_context;
    ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    CGltfDocument m_document;
    CORE::CDataNode m_json;
    bool m_hasJson;
    GUCEF::vector< bool > m_textureUsageFixed;
    CORE::CString::StringSet m_embedded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEREADER_H ? */
