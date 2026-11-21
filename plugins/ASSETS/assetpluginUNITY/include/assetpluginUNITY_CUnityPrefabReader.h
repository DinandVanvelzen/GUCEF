/*
 *  assetpluginUNITY: the reader of Unity prefabs and scenes
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYPREFABREADER_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYPREFABREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMATERIALREADER_H
#include "assetpluginUNITY_CUnityMaterialReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMATERIALREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMATERIALREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYPARTICLEREADER_H
#include "assetpluginUNITY_CUnityParticleReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYPARTICLEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYPARTICLEREADER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads a prefab (.prefab) or a scene (.unity) into the scene of the project, in Unity's coordinates:
 *
 *  - the GameObject and Transform documents become the node hierarchy (m_Father, m_Children; local position,
 *    rotation and scale), m_IsActive 0 is kept as the extra "active": false (the setting "skipInactive" leaves such
 *    objects out);
 *  - MeshFilter with MeshRenderer, and SkinnedMeshRenderer, give the mesh of a node: from a model file through its
 *    .meta ID table (CUnityModel), from a Mesh document of an .asset file (CUnityMeshDecoder) or a built-in primitive;
 *    the renderer's m_Materials the material per submesh (CUnityMaterialReader);
 *  - Box, Sphere, Capsule and Mesh colliders become colliders of the node (a mesh collider convex or trimesh by
 *    m_Convex), LODGroup a LOD group with the screen-relative heights, Light a punctual light; a Rigidbody is kept in
 *    the extras of the node; other components are reported;
 *  - prefab instances of both generations are expanded: the class 1001 document is a PrefabInstance (m_SourcePrefab,
 *    2018.3 and later) or a Prefab of the older generation (m_ParentPrefab, m_IsPrefabParent 0). The modifications
 *    (target, propertyPath, value, objectReference) are applied to the documents of the source prefab before it is
 *    read, removed components are left out, and the stripped placeholder documents of the file resolve to the
 *    objects of the instance. The Prefab document of an older prefab file itself (m_IsPrefabParent 1) is no instance.
 *  - the asset becomes a CAssetPrefab with its direct instances, whose overrides (the modifications in Unity's words,
 *    and "materials", the scene materials per submesh of a renderer on the instance root that the modifications
 *    change) go into the sidecar.
 *
 *  Unresolved references (built-ins, objects outside the package) are warnings with a default, never errors.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityPrefabReader
{
    public:

    CUnityPrefabReader( CUnityProject& project, CUnityMaterialReader& materials );

    /** Reads the prefab or scene into the scene of the project and records it as the prefab of the asset */
    bool ReadAsset( const CORE::CString& vfsPath );

    /** How often a source mesh was fetched and copied for a renderer: renderers that use the same mesh share it, so this is the number of different meshes */
    UInt32 GetSourceMeshCount( void ) const;

    /**
     *  Applies a property modification to the object of a document: "m_LocalPosition.x", "m_Name",
     *  "m_Materials.Array.data[1]", "m_Materials.Array.size". A non-null objectReference sets a reference.
     */
    static bool ApplyPropertyPath( CORE::CDataNode& object, const CORE::CString& propertyPath, const CORE::CString& value, const CORE::CDataNode* objectReference );

    private:

    typedef GUCEF::map< Int64, Int32 > TObjectNodeMap;
    typedef GUCEF::map< Int64, CORE::CDataNode > TObjectOverrideMap;
    typedef GUCEF::set< Int64 > TIdSet;

    struct SFileResult
    {
        TObjectNodeMap nodes;           /**< the node of every GameObject, Transform and component ID of the file */
        GUCEF::vector< UInt32 > roots;  /**< the nodes without a parent in the file */
    };

    bool ReadFile( const CORE::CString& vfsPath, const TObjectOverrideMap* overrides, const TIdSet* removed, UInt32 depth, SFileResult& result,
                   ASSETCONVERTER::CAssetPrefab* record );

    bool ExpandInstance( const CUnityYamlDocument& instance, const CORE::CDataNode& body, const CORE::CString& vfsPath, UInt32 depth, SFileResult& inner,
                         ASSETCONVERTER::CAssetPrefabInstance& record );

    /** An instance of a model file (m_SourcePrefab of a model): its hierarchy, with the modifications applied by object */
    bool ExpandModelInstance( const CORE::CString& modelPath, const CUnityYamlReader::TNodeList& modifications, const CUnityYamlReader::TNodeList& removedItems,
                              const CORE::CString& subject, SFileResult& inner, ASSETCONVERTER::CAssetPrefabInstance& record );

    static bool IsInstance( const CUnityYamlDocument& document, const CORE::CDataNode& body );

    /**
     *  A component the intermediate has no element for (cameras, audio sources, scripts, video players, ...) is kept with all its
     *  properties in the extras of its node, in the list "components", with the references to assets given as paths of the package;
     *  cameras and audio sources also get the members the cookers of the engines read, "camera" and "audioSource"
     */
    void RecordComponent( const CUnityYamlDocument& document, const CORE::CDataNode& body, UInt32 node );

    /** The settings of a scene (render, lightmap, navigation and occlusion settings): kept in the extras of the scene, not applied */
    void RecordSceneSettings( const CUnityYamlDocument& document, const CORE::CDataNode& body );

    /** Gives the references of a tree that resolve to an asset of the package the member "path" */
    void AnnotateReferences( CORE::CDataNode& tree );

    UInt32 NewNode( const CORE::CString& name );
    void Link( UInt32 child, Int32 parent );

    void ReadTransform( const CORE::CDataNode& body, ASSETCONVERTER::CAssetNode& node );

    /** The mesh of a reference in Unity's coordinates, with the skin of the model and the model it came from; false when it cannot be had */
    bool GetSourceMesh( const CUnityReference& reference, const CORE::CString& vfsPath, const CORE::CString& subject, const CORE::CString& userName,
                        ASSETCONVERTER::CAssetMesh& mesh, const ASSETCONVERTER::CAssetSkin*& skin, const CUnityModel*& model );

    /**
     *  The key of the source mesh of a reference when it is a definite one: the file and the mesh in it, so that renderers (instances of a
     *  prefab, for one) that use the same mesh share the mesh of the scene without the source mesh being copied again. False when the mesh
     *  is not known, or is found by its name only: the renderer takes the longer way, which reports what it finds.
     */
    bool TryGetMeshKey( const CUnityReference& reference, const CORE::CString& vfsPath, const CORE::CString& userName, CORE::CString& key,
                        const ASSETCONVERTER::CAssetSkin*& skin, const CUnityModel*& model );

    /** The mesh of the scene for a renderer: the source mesh with the materials of the renderer per submesh */
    Int32 GetRenderMesh( const CUnityReference& reference, const CORE::CString& vfsPath, const GUCEF::vector< Int32 >& materials, const CORE::CString& subject,
                         const CORE::CString& userName, const ASSETCONVERTER::CAssetSkin*& skin, const CUnityModel*& model );

    void ReadRenderer( const CORE::CDataNode& renderer, const CUnityReference& meshReference, UInt32 node, const CORE::CString& vfsPath, bool skinned,
                       const SFileResult& result );

    void ReadCollider( const CUnityYamlDocument& document, const CORE::CDataNode& body, UInt32 node, const CORE::CString& vfsPath );

    void ReadLight( const CORE::CDataNode& body, UInt32 node );

    void ReadLodGroup( const CORE::CDataNode& body, UInt32 node, const SFileResult& result );

    CUnityPrefabReader( const CUnityPrefabReader& src );            /**< not implemented */
    CUnityPrefabReader& operator=( const CUnityPrefabReader& src ); /**< not implemented */

    CUnityProject& m_project;
    CUnityMaterialReader& m_materials;
    CUnityParticleReader m_particles;
    CORE::CString::StringVector m_stack;
    TIdSet m_inactive;
    UInt32 m_sourceMeshCount;       /**< the source meshes fetched */
    UInt32 m_instanceCount;         /**< the prefab instances met, nested ones included */
    UInt32 m_unresolvedCount;       /**< of those, the ones whose prefab is not in the package or nests too deep: an empty node stands in */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYPREFABREADER_H ? */
