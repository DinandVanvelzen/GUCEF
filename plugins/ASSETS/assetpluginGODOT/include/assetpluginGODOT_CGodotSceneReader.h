/*
 *  assetpluginGODOT: the scenes of Godot
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEREADER_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEREADER_H

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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#include "assetpluginGODOT_CGodotDocument.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTMATERIALREADER_H
#include "assetpluginGODOT_CGodotMaterialReader.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTMATERIALREADER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTMATERIALREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTIMPORTSETTINGS_H
#include "assetpluginGODOT_CGodotImportSettings.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTIMPORTSETTINGS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTIMPORTSETTINGS_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEMERGER_H
#include "assetpluginGODOT_CGodotSceneMerger.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEMERGER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEMERGER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTPARTICLEREADER_H
#include "assetpluginGODOT_CGodotParticleReader.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTPARTICLEREADER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTPARTICLEREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#include "assetpluginGODOT_macros.h"
#define GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads a scene file (.tscn) of a Godot 4 project, or a mesh resource (.tres), into the model:
 *
 *  - the [node] sections become nodes with the transform of their "transform" property (or "position", "rotation",
 *    "quaternion" and "scale"); a node path is the name of the node below its parent, "." the root;
 *  - MeshInstance3D: its "mesh" is an ArrayMesh (CGodotMeshReader), a primitive mesh (CGodotPrimitiveMeshes) or a
 *    mesh resource file; "surface_material_override/N" and "material_override" replace the materials of the
 *    surfaces (a copy of the mesh per combination of materials); materials are read by CGodotMaterialReader;
 *  - an "instance" of a scene file (.tscn) is read in place below the node and recorded as a prefab instance with the
 *    properties of the instancing scene as overrides; the sections of the instancing scene that name a node of the
 *    instanced scene without a type or an instance ("[node name=... parent=...]") change that node and are recorded as
 *    overrides as well; an instance of a glTF, GLB or FBX file is imported through the importer of that format and
 *    added below the node (the settings of its .import file applied: root scale, animation import and loops,
 *    skipped nodes, external materials); a .blend source is reported and the node stays empty;
 *  - CollisionShape3D with a shape resource becomes a collider of the nearest body above it (StaticBody3D,
 *    AnimatableBody3D, RigidBody3D, VehicleBody3D, CharacterBody3D, Area3D, which makes triggers), with the kind and
 *    properties of the body in the extras "godotBody" of its node; the shape node, which has nothing else, is left out;
 *  - OmniLight3D, SpotLight3D and DirectionalLight3D become lights;
 *  - GPUParticles3D and CPUParticles3D become particle systems of their nodes (CGodotParticleReader); a sub-emitter
 *    (a node path) is resolved once all nodes are read;
 *  - other node types keep their node (with the type in the extras) and are reported when Godot nodes of that type
 *    have no counterpart (cameras, audio, navigation, ...).
 *
 *  A reader serves one asset: it keeps the documents, the models and the materials it read until it is destroyed.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotSceneReader : private CIGodotResourceResolver
{
    public:

    CGodotSceneReader( const CORE::CString& assetPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report );

    ~CGodotSceneReader();

    /** Reads the file; false with an error in the report when it cannot be read */
    bool Read( void );

    private:

    CGodotSceneReader( const CGodotSceneReader& src );            /**< not implemented */
    CGodotSceneReader& operator=( const CGodotSceneReader& src ); /**< not implemented */

    struct SModel
    {
        ASSETCONVERTER::CAssetScene* scene;
        CGodotSceneMerger::SBases bases;
        SModel( void ) : scene( GUCEF_NULL ), bases() {}
    };

    struct SInstance
    {
        UInt32 node;
        CORE::CString key;
        CORE::CString sourcePrefab;
        CORE::CDataNode overrides;
        SInstance( void ) : node( 0 ), key(), sourcePrefab(), overrides( "overrides", GUCEF_DATATYPE_OBJECT ) {}
    };

    struct SShape
    {
        const CGodotDocument* document;
        const CORE::CDataNode* value;
        bool disabled;
        SShape( void ) : document( GUCEF_NULL ), value( GUCEF_NULL ), disabled( false ) {}
    };

    typedef GUCEF::map< CORE::CString, CGodotDocument* > TDocumentMap;
    typedef GUCEF::map< CORE::CString, SModel > TModelMap;
    typedef GUCEF::map< CORE::CString, UInt32 > TPathMap;
    typedef GUCEF::map< CORE::CString, Int32 > TIndexMap;
    typedef GUCEF::map< UInt32, SShape > TShapeMap;
    typedef GUCEF::vector< SInstance* > TInstanceVector;
    typedef GUCEF::vector< Int32 > TInt32Vector;
    typedef GUCEF::set< Int32 > TInt32Set;

    CGodotDocument* LoadDocument( const CORE::CString& assetPath );
    void SetNodeType( UInt32 node, const CORE::CString& type );
    CORE::CString GetNodeType( UInt32 node ) const;
    static CORE::CString JoinKey( const CORE::CString& parent, const CORE::CString& name );

    bool ReadScene( const CGodotDocument& document, Int32 existingRoot, const CORE::CString& rootKey, UInt32 depth );
    bool ReadMeshResource( const CGodotDocument& document );
    void InstanceNode( const CGodotDocument& document, const CORE::CDataNode& section, UInt32 node, const CORE::CString& key, UInt32 depth );
    void InstanceModel( const CORE::CString& assetPath, const CORE::CString& importerName, UInt32 node, const CORE::CString& key );
    SModel* GetModel( const CORE::CString& assetPath, const CORE::CString& importerName );
    void ApplyImportSettings( ASSETCONVERTER::CAssetScene& model, const CGodotImportSettings& settings, const CORE::CString& assetPath );
    void RegisterModelNodes( UInt32 node, const CORE::CString& key );
    void ReplaceExternalMaterials( const ASSETCONVERTER::CAssetScene& model, const CGodotImportSettings& settings, UInt32 materialBase );

    void ApplyProperties( const CGodotDocument& document, const CORE::CDataNode& section, UInt32 node, const CORE::CString& type, const CORE::CString& key, bool isOverride );
    void ApplyTransform( UInt32 node, const CORE::CDataNode& section );
    void ApplyMesh( const CGodotDocument& document, const CORE::CDataNode& section, UInt32 node );
    void ApplyLight( UInt32 node, const CORE::CString& type, const CORE::CDataNode& section );
    void ApplyBody( const CGodotDocument& document, UInt32 node, const CORE::CString& type, const CORE::CDataNode& section );
    void RecordOverrides( const CGodotDocument& document, const CORE::CDataNode& section, const CORE::CString& key, bool isOverride );

    Int32 ResolveMesh( const CGodotDocument& document, const CORE::CDataNode* value );
    Int32 MeshFromSection( const CGodotDocument& document, const CORE::CDataNode& section, const CORE::CString& key );
    Int32 ResolveMaterial( const CGodotDocument& document, const CORE::CDataNode* value );
    bool ResolveResource( const CGodotDocument& document, const CORE::CDataNode* value, const CGodotDocument*& resourceDocument, const CORE::CDataNode*& section, CORE::CString& key );

    void BuildColliders( void );
    void FinishMeshes( void );
    void FinishPrefab( void );
    void ResolveSubEmitters( void );

    virtual bool ResolveResourceSection( const CGodotDocument& document, const CORE::CDataNode* value, const CGodotDocument*& resourceDocument, const CORE::CDataNode*& section ) GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 ResolveMaterialResource( const CGodotDocument& document, const CORE::CDataNode* value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 ResolveMeshSection( const CGodotDocument& document, const CORE::CDataNode& section ) GUCEF_VIRTUAL_OVERRIDE;

    struct SSubEmitterLink
    {
        UInt32 system;
        CORE::CString nodeKey;
        CORE::CString path;
        ASSETCONVERTER::TParticleTrigger trigger;
    };
    typedef GUCEF::vector< SSubEmitterLink > TSubEmitterLinkVector;

    private:

    CORE::CString m_assetPath;
    ASSETCONVERTER::CAssetPackContext& m_context;
    ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    CGodotMaterialReader m_materials;
    TDocumentMap m_documents;
    TModelMap m_models;
    TPathMap m_paths;
    TIndexMap m_meshes;
    TInt32Set m_ownMeshes;
    TIndexMap m_meshVariants;
    TShapeMap m_shapes;
    TInstanceVector m_instances;
    CORE::CString::StringVector m_nodeTypes;
    CORE::CString::StringSet m_active;
    ASSETCONVERTER::CAssetScene::TUInt32Set m_carriers;
    bool m_scriptReported;
    CGodotParticleReader m_particles;
    TSubEmitterLinkVector m_subEmitterLinks;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEREADER_H ? */
