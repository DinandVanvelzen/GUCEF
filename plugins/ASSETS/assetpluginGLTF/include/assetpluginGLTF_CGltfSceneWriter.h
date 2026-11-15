/*
 *  assetpluginGLTF: builds the glTF text and binary buffer of a scene of the converter model
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEWRITER_H
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFJSONWRITER_H
#include "assetpluginGLTF_CGltfJsonWriter.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFJSONWRITER_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFJSONWRITER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFBUFFERBUILDER_H
#include "assetpluginGLTF_CGltfBufferBuilder.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFBUFFERBUILDER_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFBUFFERBUILDER_H ? */

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
 *  Turns a scene of the converter model into the text of a glTF 2.0 file and the bytes of its one binary buffer.
 *
 *  - Indices of nodes, meshes, skins, materials and textures are those of the model; nodes for colliders follow the
 *    nodes of the scene. A submesh is a primitive, a texture an image with a sampler.
 *  - LOD groups use MSFT_lod: the first level holds the ids of the lower levels and the screen coverage of every
 *    level in its extras. The lower levels are not children of their parent, so readers that ignore the extension
 *    show the highest level only; the parent of such a level is in its extras.
 *  - A collider is a node of its own below the node it belongs to, with OMI_physics_body and a shape of
 *    OMI_physics_shape, so every reader of those sees compound shapes.
 *  - Lights use KHR_lights_punctual, texture tiling KHR_texture_transform, HDR emission KHR_materials_emissive_strength.
 *    The extension data of a material goes through unchanged.
 *  - The output is deterministic: no times, ordered sets, numbers in their shortest exact form.
 *
 *  Image URIs come from the baked file of a texture, relative to 'folder' (the VFS folder of the glTF file).
 */
class GUCEF_ASSETPLUGIN_GLTF_EXPORT_CPP CGltfSceneWriter
{
    public:

    CGltfSceneWriter( const ASSETCONVERTER::CAssetScene& scene     ,
                      const CORE::CString& folder                  ,
                      const CORE::CString& binaryUri               ,
                      ASSETCONVERTER::CConversionReport& report    );

    ~CGltfSceneWriter();

    /** Builds the text and the binary buffer */
    bool Write( void );

    const CORE::CDynamicBuffer& GetText( void ) const;

    const CORE::CDynamicBuffer& GetBinary( void ) const;

    /** The URIs of the images, as written */
    const CORE::CString::StringVector& GetImageUris( void ) const;

    /** The version of the writer: raised whenever its output for the same scene changes */
    static UInt32 GetVersion( void );

    private:

    CGltfSceneWriter( const CGltfSceneWriter& src );                /**< not implemented */
    CGltfSceneWriter& operator=( const CGltfSceneWriter& src );     /**< not implemented */

    struct SColliderEntry
    {
        UInt32 owner;
        UInt32 index;
        UInt32 node;
    };
    typedef GUCEF::vector< SColliderEntry > TColliderVector;
    typedef GUCEF::vector< UInt32 > TUInt32Vector;
    typedef GUCEF::set< UInt32 > TUInt32Set;

    void Prepare( void );
    void WriteAsset( void );
    void WriteScenes( void );
    void WriteNodes( void );
    void WriteNodeTransform( const ASSETCONVERTER::CAssetNode& node );
    void WriteMeshes( void );
    bool WritePrimitive( const ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 morphTargetCount );
    void WriteMorphTargets( const ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 morphTargetCount, UInt32 vertexCount );
    void WriteSkins( void );
    void WriteAnimations( void );
    void WriteMaterials( void );
    void WriteTextureInfo( const char* key, const ASSETCONVERTER::CAssetTextureReference& reference, bool isNormal, bool isOcclusion );
    void WriteTextures( void );
    void WriteExtensions( void );
    UInt32 GetSamplerIndex( const ASSETCONVERTER::CAssetTexture& texture );
    bool IsLowerLodLevel( UInt32 node ) const;

    private:

    const ASSETCONVERTER::CAssetScene& m_scene;
    CORE::CString m_folder;
    CORE::CString m_binaryUri;
    ASSETCONVERTER::CConversionReport& m_report;
    CGltfJsonWriter m_writer;
    CGltfBufferBuilder m_binary;
    CORE::CString::StringSet m_extensionsUsed;
    CORE::CString::StringVector m_imageUris;
    TUInt32Set m_lowerLodNodes;
    GUCEF::map< UInt32, UInt32 > m_lodGroupOfFirstLevel;
    GUCEF::map< UInt32, UInt32 > m_firstColliderNode;
    TColliderVector m_colliders;
    GUCEF::vector< UInt32 > m_samplerWraps;
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

#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEWRITER_H ? */
