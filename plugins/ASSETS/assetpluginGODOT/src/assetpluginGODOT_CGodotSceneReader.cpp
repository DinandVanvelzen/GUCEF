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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <stdlib.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETNORMALISER_H
#include "AssetConverterLib_CAssetNormaliser.h"
#define ASSETCONVERTERLIB_CASSETNORMALISER_H
#endif /* ASSETCONVERTERLIB_CASSETNORMALISER_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#include "assetpluginGODOT_CGodotVariant.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H
#include "assetpluginGODOT_CGodotPaths.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTMESHREADER_H
#include "assetpluginGODOT_CGodotMeshReader.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTMESHREADER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTMESHREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTPRIMITIVEMESHES_H
#include "assetpluginGODOT_CGodotPrimitiveMeshes.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTPRIMITIVEMESHES_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTPRIMITIVEMESHES_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTSHAPEREADER_H
#include "assetpluginGODOT_CGodotShapeReader.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTSHAPEREADER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTSHAPEREADER_H ? */

#include "assetpluginGODOT_CGodotSceneReader.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "godot";
static const UInt32 MaxInstanceDepth = 16;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsBodyType( const CORE::CString& type )
{
    return "StaticBody3D" == type || "AnimatableBody3D" == type || "RigidBody3D" == type || "VehicleBody3D" == type || "CharacterBody3D" == type || "Area3D" == type;
}

/*-------------------------------------------------------------------------*/

static bool
IsLightType( const CORE::CString& type )
{
    return "OmniLight3D" == type || "SpotLight3D" == type || "DirectionalLight3D" == type;
}

/*-------------------------------------------------------------------------*/

/** The 3D node types that have no counterpart in the model: the node stays, its content does not */
static bool
IsUnsupportedType( const CORE::CString& type )
{
    static const char* const types[] = { "Camera3D", "AudioStreamPlayer3D", "AudioListener3D", "NavigationRegion3D",
                                         "NavigationLink3D", "NavigationObstacle3D", "CSGBox3D", "CSGSphere3D", "CSGCylinder3D", "CSGMesh3D", "CSGPolygon3D",
                                         "CSGCombiner3D", "CSGTorus3D", "Decal", "GridMap", "Label3D", "Sprite3D", "AnimatedSprite3D", "VoxelGI", "LightmapGI",
                                         "ReflectionProbe", "FogVolume", "OccluderInstance3D", "WorldEnvironment", "MultiMeshInstance3D", "SpringArm3D",
                                         "RayCast3D", "ShapeCast3D", "VisibleOnScreenNotifier3D", "XRCamera3D", "XROrigin3D", "GPUParticlesCollision3D", "CollisionPolygon3D",
                                         "Skeleton3D", "BoneAttachment3D", "PhysicalBone3D", "AnimationPlayer", "AnimationTree", "SubViewport", "VideoStreamPlayer",
                                         "GPUParticlesAttractor3D", "Path3D", "PathFollow3D" };
    for ( UInt32 t=0; t<sizeof( types ) / sizeof( types[ 0 ] ); ++t )
        if ( type == types[ t ] )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

static void
MultiplyQuaternions( const Float32 a[ 4 ], const Float32 b[ 4 ], Float32 out[ 4 ] )
{
    out[ 0 ] = a[ 3 ] * b[ 0 ] + a[ 0 ] * b[ 3 ] + a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
    out[ 1 ] = a[ 3 ] * b[ 1 ] - a[ 0 ] * b[ 2 ] + a[ 1 ] * b[ 3 ] + a[ 2 ] * b[ 0 ];
    out[ 2 ] = a[ 3 ] * b[ 2 ] + a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ] + a[ 2 ] * b[ 3 ];
    out[ 3 ] = a[ 3 ] * b[ 3 ] - a[ 0 ] * b[ 0 ] - a[ 1 ] * b[ 1 ] - a[ 2 ] * b[ 2 ];
}

/*-------------------------------------------------------------------------*/

/** The rotation of Godot's Euler angles (radians, order YXZ: the Y rotation is applied last) */
static void
EulerToQuaternion( const Float32 euler[ 3 ], Float32 out[ 4 ] )
{
    const Float32 qx[ 4 ] = { (Float32) sin( euler[ 0 ] * 0.5 ), 0.0f, 0.0f, (Float32) cos( euler[ 0 ] * 0.5 ) };
    const Float32 qy[ 4 ] = { 0.0f, (Float32) sin( euler[ 1 ] * 0.5 ), 0.0f, (Float32) cos( euler[ 1 ] * 0.5 ) };
    const Float32 qz[ 4 ] = { 0.0f, 0.0f, (Float32) sin( euler[ 2 ] * 0.5 ), (Float32) cos( euler[ 2 ] * 0.5 ) };
    Float32 yx[ 4 ];
    MultiplyQuaternions( qy, qx, yx );
    MultiplyQuaternions( yx, qz, out );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Stem( const CORE::CString& path )
{
    return ASSETCONVERTER::CVfsPaths::StemOf( path );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGodotSceneReader::CGodotSceneReader( const CORE::CString& assetPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report )
    : m_assetPath( assetPath )
    , m_context( context )
    , m_scene( scene )
    , m_report( report )
    , m_materials( context, scene, report )
    , m_documents()
    , m_models()
    , m_paths()
    , m_meshes()
    , m_ownMeshes()
    , m_meshVariants()
    , m_shapes()
    , m_instances()
    , m_nodeTypes()
    , m_active()
    , m_carriers()
    , m_scriptReported( false )
    , m_particles( *this, scene, report )
    , m_subEmitterLinks()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGodotSceneReader::~CGodotSceneReader()
{GUCEF_TRACE;

    for ( TDocumentMap::iterator d = m_documents.begin(); d != m_documents.end(); ++d )
        delete (*d).second;
    for ( TModelMap::iterator m = m_models.begin(); m != m_models.end(); ++m )
        delete (*m).second.scene;
    for ( size_t i=0; i<m_instances.size(); ++i )
        delete m_instances[ i ];
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotSceneReader::JoinKey( const CORE::CString& parent, const CORE::CString& name )
{GUCEF_TRACE;

    if ( "." == parent || parent.IsNULLOrEmpty() )
        return name;
    return parent + "/" + name;
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::SetNodeType( UInt32 node, const CORE::CString& type )
{GUCEF_TRACE;

    while ( m_nodeTypes.size() <= node )
        m_nodeTypes.push_back( CORE::CString::Empty );
    m_nodeTypes[ node ] = type;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotSceneReader::GetNodeType( UInt32 node ) const
{GUCEF_TRACE;

    return node < m_nodeTypes.size() ? m_nodeTypes[ node ] : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CGodotDocument*
CGodotSceneReader::LoadDocument( const CORE::CString& assetPath )
{GUCEF_TRACE;

    TDocumentMap::const_iterator known = m_documents.find( assetPath );
    if ( known != m_documents.end() )
        return (*known).second;

    CGodotDocument* document = GUCEF_NEW CGodotDocument();
    if ( !document->Load( m_context, assetPath ) )
    {
        // the file of the asset itself is the asset; another file that cannot be read leaves out what it holds
        if ( assetPath == m_assetPath )
            m_report.AddError( SourceName, assetPath, document->GetProblem() );
        else
            m_report.AddWarning( SourceName, assetPath, document->GetProblem() + ": what the file holds is left out" );
        delete document;
        document = GUCEF_NULL;
    }
    else
    if ( assetPath != m_assetPath )
        m_scene.AddDependency( m_context.ToVfsPath( assetPath ) );
    m_documents[ assetPath ] = document;
    return document;
}

/*-------------------------------------------------------------------------*/

bool
CGodotSceneReader::Read( void )
{GUCEF_TRACE;

    m_scene.coordinateSystem = MATH::CCoordinateSystem::GucefDefault();
    m_scene.provenance.sourceEngine = "godot";

    CGodotDocument* document = LoadDocument( m_assetPath );
    if ( GUCEF_NULL == document )
        return false;
    m_scene.provenance.sourceFormatVersion = "format " + CORE::Int32ToString( document->GetFormat() );
    const CORE::CString uid = document->GetUid();
    if ( !uid.IsNULLOrEmpty() )
        m_scene.provenance.sourceId = uid;

    const CORE::CString kind = document->GetKind();
    if ( "gd_scene" == kind )
    {
        if ( 3 != document->GetFormat() )
        {
            m_report.AddError( SourceName, m_assetPath, "the scene has the file format " + CORE::Int32ToString( document->GetFormat() ) + ": only the format of Godot 4 (3) is read" );
            return false;
        }
        if ( document->GetNodeSections().empty() )
        {
            m_report.AddError( SourceName, m_assetPath, "the scene has no nodes" );
            return false;
        }
        m_active.insert( m_assetPath );
        ReadScene( *document, -1, ".", 0 );
    }
    else
    if ( "gd_resource" == kind )
    {
        if ( !ReadMeshResource( *document ) )
            return false;
    }
    else
    {
        m_report.AddError( SourceName, m_assetPath, "the file is neither a scene nor a resource of Godot 4" );
        return false;
    }

    BuildColliders();
    ResolveSubEmitters();
    FinishMeshes();
    FinishPrefab();
    if ( !m_carriers.empty() )
        m_scene.RemoveNodes( m_carriers );

    // what the meshes of the files had for themselves and the nodes replaced is not part of the scene
    m_scene.RemoveUnusedMeshes();
    m_scene.RemoveUnusedMaterials();
    m_scene.RemoveUnusedTextures();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotSceneReader::ReadMeshResource( const CGodotDocument& document )
{GUCEF_TRACE;

    const CORE::CDataNode* main = document.GetMainResource();
    const CORE::CString type = document.GetResourceType();
    if ( GUCEF_NULL == main || ( "ArrayMesh" != type && !CGodotPrimitiveMeshes::IsPrimitiveType( type ) ) )
    {
        m_report.AddSkipped( SourceName, m_assetPath, "the resource of the type " + type + " is not a mesh this importer reads" );
        return false;
    }
    const UInt32 node = m_scene.AddNode( Stem( m_assetPath ) );
    SetNodeType( node, "MeshInstance3D" );
    m_paths[ "." ] = node;
    const Int32 mesh = MeshFromSection( document, *main, m_assetPath + "#resource" );
    if ( mesh < 0 )
        return false;
    m_scene.nodes[ node ].mesh = mesh;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotSceneReader::ReadScene( const CGodotDocument& document, Int32 existingRoot, const CORE::CString& rootKey, UInt32 depth )
{GUCEF_TRACE;

    const CGodotDocument::TSectionVector& sections = document.GetNodeSections();
    for ( size_t s=0; s<sections.size(); ++s )
    {
        const CORE::CDataNode& section = *sections[ s ];
        const CORE::CString name = section.GetAttributeValue( "name" ).AsString();
        const CORE::CString type = section.GetAttributeValue( "type" ).AsString();
        const CORE::CString parent = section.GetAttributeValue( "parent" ).AsString();
        const bool hasParent = section.IsAttribute( "parent" );
        const bool hasInstance = section.IsAttribute( "instance" );

        UInt32 node = 0;
        CORE::CString key;
        bool isOverride = false;
        if ( !hasParent )
        {
            key = rootKey;
            if ( existingRoot >= 0 )
                node = (UInt32) existingRoot;
            else
            {
                node = m_scene.AddNode( name );
                m_paths[ key ] = node;
            }
        }
        else
        {
            const CORE::CString parentKey = "." == parent ? rootKey : JoinKey( rootKey, parent );
            key = JoinKey( parentKey, name );
            if ( !type.IsNULLOrEmpty() || hasInstance )
            {
                TPathMap::const_iterator foundParent = m_paths.find( parentKey );
                if ( foundParent == m_paths.end() )
                {
                    m_report.AddWarning( SourceName, name, "the parent " + parent + " of the node is not in the scene: the node goes to the root" );
                    foundParent = m_paths.find( rootKey );
                }
                if ( foundParent == m_paths.end() )
                    continue;
                node = m_scene.AddNode( name, (Int32) (*foundParent).second );
                m_paths[ key ] = node;
            }
            else
            {
                TPathMap::const_iterator found = m_paths.find( key );
                if ( found == m_paths.end() )
                {
                    m_report.AddWarning( SourceName, name, "the section changes the node " + key + ", which the scene does not have" );
                    continue;
                }
                node = (*found).second;
                isOverride = true;
            }
        }

        if ( !type.IsNULLOrEmpty() )
            SetNodeType( node, type );
        const CORE::CString effectiveType = type.IsNULLOrEmpty() ? GetNodeType( node ) : type;
        if ( hasInstance )
            InstanceNode( document, section, node, key, depth );
        ApplyProperties( document, section, node, effectiveType, key, isOverride );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::InstanceNode( const CGodotDocument& document, const CORE::CDataNode& section, UInt32 node, const CORE::CString& key, UInt32 depth )
{GUCEF_TRACE;

    const CORE::CString id = section.GetAttributeValue( "instance" ).AsString();
    CORE::CString type;
    CORE::CString path;
    CORE::CString uid;
    const CORE::CString nodeName = m_scene.nodes[ node ].name;
    if ( !document.GetExtResourceInfo( id, type, path, uid ) )
    {
        m_report.AddWarning( SourceName, nodeName, "the instanced scene " + id + " is not declared in the file" );
        return;
    }
    CORE::CString assetPath;
    if ( !CGodotPaths::Resolve( m_context, document.GetAssetPath(), path, uid, assetPath ) )
    {
        m_report.AddWarning( SourceName, nodeName, "the instanced scene " + path + " is not in the pack: the node stays empty" );
        return;
    }
    m_scene.nodes[ node ].extras.SetAttribute( "instanceOf", assetPath );
    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( assetPath );

    if ( "tscn" == extension )
    {
        if ( depth >= MaxInstanceDepth || m_active.find( assetPath ) != m_active.end() )
        {
            m_report.AddError( SourceName, nodeName, "the scene " + assetPath + " instances itself, directly or through other scenes" );
            return;
        }
        CGodotDocument* nested = LoadDocument( assetPath );
        if ( GUCEF_NULL == nested )
            return;
        SInstance* instance = GUCEF_NEW SInstance();
        instance->node = node;
        instance->key = key;
        instance->sourcePrefab = assetPath;
        m_instances.push_back( instance );
        m_scene.nodes[ node ].prefabInstance = assetPath;
        m_active.insert( assetPath );
        ReadScene( *nested, (Int32) node, key, depth + 1 );
        m_active.erase( assetPath );
        return;
    }
    if ( "glb" == extension || "gltf" == extension )
    {
        InstanceModel( assetPath, "gltf", node, key );
        return;
    }
    if ( "fbx" == extension )
    {
        InstanceModel( assetPath, "fbx", node, key );
        return;
    }
    if ( "blend" == extension )
    {
        m_report.AddSkipped( SourceName, assetPath, "a Blender source needs Blender to be read: the node " + nodeName + " stays empty" );
        return;
    }
    m_report.AddSkipped( SourceName, assetPath, "the file type of the instanced scene is not read: the node " + nodeName + " stays empty" );
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::ApplyImportSettings( ASSETCONVERTER::CAssetScene& model, const CGodotImportSettings& settings, const CORE::CString& assetPath )
{GUCEF_TRACE;

    if ( !settings.IsPresent() )
        return;

    // the nodes the settings skip, by their path below the root
    ASSETCONVERTER::CAssetScene::TUInt32Set skipped;
    for ( size_t n=0; n<model.nodes.size(); ++n )
    {
        CORE::CString path = model.nodes[ n ].name;
        for ( Int32 p = model.nodes[ n ].parent; p >= 0; p = model.nodes[ p ].parent )
            path = model.nodes[ p ].name + "/" + path;
        if ( settings.IsNodeSkipped( path ) )
            skipped.insert( (UInt32) n );
    }
    if ( !skipped.empty() )
    {
        model.RemoveNodes( skipped );
        m_report.AddInfo( SourceName, assetPath, CORE::UInt32ToString( (UInt32) skipped.size() ) + " node(s) are skipped by the import settings" );
    }

    if ( !settings.GetImportAnimations() && !model.animations.empty() )
    {
        model.animations.clear();
        m_report.AddInfo( SourceName, assetPath, "the import settings turn the animations off: they are not imported" );
    }
    for ( size_t a=0; a<model.animations.size(); ++a )
    {
        bool loop = false;
        if ( settings.GetAnimationLoop( model.animations[ a ].name, loop ) )
            model.animations[ a ].loop = loop;
    }

    const Float64 scale = settings.GetRootScale();
    if ( scale != 1.0 && scale > 0.0 )
    {
        for ( size_t r=0; r<model.rootNodes.size(); ++r )
        {
            MATH::TFloat32Transform3D& transform = model.nodes[ model.rootNodes[ r ] ].transform;
            transform.position = transform.position * (Float32) scale;
            transform.scale = transform.scale * (Float32) scale;
        }
        m_report.AddInfo( SourceName, assetPath, "the import settings scale the scene by " + CORE::DoubleToString( scale ) );
    }

    CORE::CDataNode* record = m_scene.extras.FindOrAddChild( "godotImport", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* file = record->AddChild( assetPath, GUCEF_DATATYPE_OBJECT );
    settings.AddPlain( *file );
}

/*-------------------------------------------------------------------------*/

CGodotSceneReader::SModel*
CGodotSceneReader::GetModel( const CORE::CString& assetPath, const CORE::CString& importerName )
{GUCEF_TRACE;

    TModelMap::iterator known = m_models.find( assetPath );
    if ( known != m_models.end() )
        return GUCEF_NULL != (*known).second.scene ? &(*known).second : GUCEF_NULL;

    SModel& entry = m_models[ assetPath ];
    ASSETCONVERTER::TAssetImporterPtr importer;
    if ( !ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( importerName, importer ) )
    {
        m_report.AddError( SourceName, assetPath, "the importer \"" + importerName + "\" for this file is not loaded" );
        return GUCEF_NULL;
    }

    ASSETCONVERTER::CAssetScene* model = GUCEF_NEW ASSETCONVERTER::CAssetScene();
    model->name = Stem( assetPath );
    model->settings = m_scene.settings;
    model->provenance.packId = m_scene.provenance.packId;
    model->provenance.assetPath = assetPath;
    model->provenance.sourceId = assetPath;
    model->provenance.sourceFormat = ASSETCONVERTER::CVfsPaths::ExtensionOf( assetPath );
    model->provenance.sourceEngine = "godot";
    const CORE::CString vfsPath = m_context.ToVfsPath( assetPath );
    if ( !importer->Import( vfsPath, m_context, *model, m_report ) || !ASSETCONVERTER::CAssetNormaliser::Normalise( *model, m_report ) )
    {
        m_report.AddError( SourceName, assetPath, "the model file cannot be imported" );
        delete model;
        return GUCEF_NULL;
    }
    m_scene.AddDependency( vfsPath );
    for ( size_t d=0; d<model->dependencies.size(); ++d )
        m_scene.AddDependency( model->dependencies[ d ] );

    CGodotImportSettings settings;
    if ( settings.Load( m_context, assetPath ) )
    {
        m_scene.AddDependency( m_context.ToVfsPath( settings.GetSettingsPath() ) );
        ApplyImportSettings( *model, settings, assetPath );
    }
    entry.scene = model;
    return &entry;
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::RegisterModelNodes( UInt32 node, const CORE::CString& key )
{GUCEF_TRACE;

    SetNodeType( node, m_scene.nodes[ node ].mesh >= 0 ? "MeshInstance3D" : "Node3D" );
    const GUCEF::vector< UInt32 > children = m_scene.nodes[ node ].children;
    for ( size_t c=0; c<children.size(); ++c )
    {
        const CORE::CString childKey = JoinKey( key, m_scene.nodes[ children[ c ] ].name );
        if ( m_paths.find( childKey ) == m_paths.end() )
            m_paths[ childKey ] = children[ c ];
        RegisterModelNodes( children[ c ], childKey );
    }
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::ReplaceExternalMaterials( const ASSETCONVERTER::CAssetScene& model, const CGodotImportSettings& settings, UInt32 materialBase )
{GUCEF_TRACE;

    for ( size_t m=0; m<model.materials.size(); ++m )
    {
        CORE::CString path;
        CORE::CString uid;
        if ( !settings.GetExternalMaterial( model.materials[ m ].name, path, uid ) )
            continue;
        CORE::CString assetPath;
        if ( !CGodotPaths::Resolve( m_context, settings.GetSettingsPath(), path, uid, assetPath ) )
        {
            m_report.AddWarning( SourceName, model.materials[ m ].name, "the external material " + path + " of the import settings is not in the pack: the material of the file stays" );
            continue;
        }
        const CGodotDocument* document = LoadDocument( assetPath );
        if ( GUCEF_NULL == document || GUCEF_NULL == document->GetMainResource() )
            continue;
        const Int32 replacement = m_materials.ReadMaterial( *document, *document->GetMainResource() );
        if ( replacement >= 0 )
            m_scene.ReplaceMaterial( materialBase + (UInt32) m, (UInt32) replacement );
    }
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::InstanceModel( const CORE::CString& assetPath, const CORE::CString& importerName, UInt32 node, const CORE::CString& key )
{GUCEF_TRACE;

    SModel* model = GetModel( assetPath, importerName );
    if ( GUCEF_NULL == model )
        return;
    const bool firstUse = !model->bases.added;
    CGodotSceneMerger::TUInt32Vector roots;
    const UInt32 first = CGodotSceneMerger::Append( m_scene, *model->scene, (Int32) node, model->bases, roots );
    (void) first;
    if ( firstUse )
    {
        CGodotImportSettings settings;
        if ( settings.Load( m_context, assetPath ) )
            ReplaceExternalMaterials( *model->scene, settings, model->bases.materialBase );
    }
    for ( size_t r=0; r<roots.size(); ++r )
    {
        const CORE::CString rootKey = JoinKey( key, m_scene.nodes[ roots[ r ] ].name );
        if ( m_paths.find( rootKey ) == m_paths.end() )
            m_paths[ rootKey ] = roots[ r ];
        RegisterModelNodes( roots[ r ], rootKey );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotSceneReader::ResolveResource( const CGodotDocument& document, const CORE::CDataNode* value, const CGodotDocument*& resourceDocument, const CORE::CDataNode*& section, CORE::CString& key )
{GUCEF_TRACE;

    CORE::CString kind;
    CORE::CString id;
    if ( !CGodotVariant::GetReference( value, kind, id ) )
        return false;
    if ( "SubResource" == kind )
    {
        section = document.FindSubResource( id );
        resourceDocument = &document;
        key = document.GetAssetPath() + "#" + id;
        return GUCEF_NULL != section;
    }
    CORE::CString type;
    CORE::CString path;
    CORE::CString uid;
    CORE::CString assetPath;
    if ( !document.GetExtResourceInfo( id, type, path, uid ) || !CGodotPaths::Resolve( m_context, document.GetAssetPath(), path, uid, assetPath ) )
    {
        m_report.AddWarning( SourceName, document.GetAssetPath(), "the resource " + path + " is not in the pack" );
        return false;
    }
    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( assetPath );
    if ( "tres" != extension && "res" != extension && "mesh" != extension )
    {
        m_report.AddSkipped( SourceName, assetPath, "a resource of this file type is not read" );
        return false;
    }
    const CGodotDocument* external = LoadDocument( assetPath );
    if ( GUCEF_NULL == external || GUCEF_NULL == external->GetMainResource() )
        return false;
    resourceDocument = external;
    section = external->GetMainResource();
    key = assetPath + "#resource";
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CGodotSceneReader::ResolveMaterial( const CGodotDocument& document, const CORE::CDataNode* value )
{GUCEF_TRACE;

    if ( GUCEF_NULL == value || CGodotVariant::IsType( value, "null" ) )
        return -1;
    const CGodotDocument* resourceDocument = GUCEF_NULL;
    const CORE::CDataNode* section = GUCEF_NULL;
    CORE::CString key;
    if ( !ResolveResource( document, value, resourceDocument, section, key ) )
        return -1;
    const Int32 index = m_materials.ReadMaterial( *resourceDocument, *section );
    if ( index < 0 )
        m_report.AddSkipped( SourceName, key, "the resource is not a material this importer reads" );
    return index;
}

/*-------------------------------------------------------------------------*/

Int32
CGodotSceneReader::MeshFromSection( const CGodotDocument& document, const CORE::CDataNode& section, const CORE::CString& key )
{GUCEF_TRACE;

    TIndexMap::const_iterator known = m_meshes.find( key );
    if ( known != m_meshes.end() )
        return (*known).second;

    CORE::CString type = section.GetAttributeValue( "type" ).AsString();
    if ( type.IsNULLOrEmpty() )
        type = document.GetResourceType();
    CORE::CString name = CGodotVariant::GetString( CGodotVariant::FindProperty( section, "resource_name" ) );
    if ( name.IsNULLOrEmpty() )
    {
        const Int32 hash = key.HasChar( '#' );
        name = hash >= 0 ? key.SubstrFromRange( (UInt32) hash + 1, key.Length() ) : key;
    }

    ASSETCONVERTER::CAssetMesh mesh;
    mesh.name = name;
    if ( "ArrayMesh" == type )
    {
        const CORE::CDataNode* surfaces = CGodotVariant::FindProperty( section, "_surfaces" );
        CGodotMeshReader::TNodeVector materials;
        if ( GUCEF_NULL == surfaces || !CGodotMeshReader::ReadSurfaces( *surfaces, mesh, materials, m_report, name ) )
        {
            m_report.AddWarning( SourceName, name, "the mesh has no surfaces" );
            m_meshes[ key ] = -1;
            return -1;
        }
        for ( size_t s=0; s<mesh.subMeshes.size() && s<materials.size(); ++s )
            if ( GUCEF_NULL != materials[ s ] )
                mesh.subMeshes[ s ].materialIndex = ResolveMaterial( document, materials[ s ] );
    }
    else
    if ( CGodotPrimitiveMeshes::IsPrimitiveType( type ) )
    {
        CGodotPrimitiveMeshes::Build( type, section, mesh );
        if ( !mesh.subMeshes.empty() )
            mesh.subMeshes[ 0 ].materialIndex = ResolveMaterial( document, CGodotVariant::FindProperty( section, "material" ) );
    }
    else
    {
        m_report.AddSkipped( SourceName, name, "the mesh type " + type + " is not read" );
        m_meshes[ key ] = -1;
        return -1;
    }
    m_scene.meshes.push_back( mesh );
    const Int32 index = (Int32) m_scene.meshes.size() - 1;
    m_meshes[ key ] = index;
    m_ownMeshes.insert( index );
    return index;
}

/*-------------------------------------------------------------------------*/

Int32
CGodotSceneReader::ResolveMesh( const CGodotDocument& document, const CORE::CDataNode* value )
{GUCEF_TRACE;

    if ( GUCEF_NULL == value || CGodotVariant::IsType( value, "null" ) )
        return -1;
    const CGodotDocument* resourceDocument = GUCEF_NULL;
    const CORE::CDataNode* section = GUCEF_NULL;
    CORE::CString key;
    if ( !ResolveResource( document, value, resourceDocument, section, key ) )
        return -1;
    return MeshFromSection( *resourceDocument, *section, key );
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::ApplyTransform( UInt32 node, const CORE::CDataNode& section )
{GUCEF_TRACE;

    MATH::TFloat32Transform3D transform = m_scene.nodes[ node ].transform;
    const CORE::CDataNode* whole = CGodotVariant::FindProperty( section, "transform" );
    if ( GUCEF_NULL != whole )
    {
        MATH::TFloat32Transform3D parsed;
        if ( CGodotVariant::GetTransform( whole, parsed ) )
            transform = parsed;
        else
            m_report.AddApproximation( SourceName, m_scene.nodes[ node ].name, "the transform has a shear that translation, rotation and scale cannot hold: it is not applied" );
    }
    else
    {
        Float32 value[ 3 ];
        if ( CGodotVariant::GetVector3( CGodotVariant::FindProperty( section, "position" ), value ) )
            transform.position = MATH::TFloat32Transform3D::TPositionVector( value[ 0 ], value[ 1 ], value[ 2 ] );
        MATH::TFloat32Transform3D basis;
        if ( CGodotVariant::GetTransform( CGodotVariant::FindProperty( section, "basis" ), basis ) )
        {
            transform.rotation = basis.rotation;
            transform.scale = basis.scale;
        }
        CGodotVariant::TNumberVector numbers;
        if ( CGodotVariant::GetNumbers( CGodotVariant::FindProperty( section, "quaternion" ), numbers ) && numbers.size() >= 4 )
        {
            transform.rotation.x = (Float32) numbers[ 0 ];
            transform.rotation.y = (Float32) numbers[ 1 ];
            transform.rotation.z = (Float32) numbers[ 2 ];
            transform.rotation.w = (Float32) numbers[ 3 ];
        }
        else
        if ( CGodotVariant::GetVector3( CGodotVariant::FindProperty( section, "rotation" ), value ) )
        {
            Float32 q[ 4 ];
            EulerToQuaternion( value, q );
            transform.rotation.x = q[ 0 ];
            transform.rotation.y = q[ 1 ];
            transform.rotation.z = q[ 2 ];
            transform.rotation.w = q[ 3 ];
        }
        if ( CGodotVariant::GetVector3( CGodotVariant::FindProperty( section, "scale" ), value ) )
            transform.scale = MATH::TFloat32Transform3D::TVector( value[ 0 ], value[ 1 ], value[ 2 ] );
    }
    m_scene.nodes[ node ].transform = transform;
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::ApplyMesh( const CGodotDocument& document, const CORE::CDataNode& section, UInt32 node )
{GUCEF_TRACE;

    const CORE::CDataNode* meshValue = CGodotVariant::FindProperty( section, "mesh" );
    if ( GUCEF_NULL != meshValue )
    {
        if ( CGodotVariant::IsType( meshValue, "null" ) )
            m_scene.nodes[ node ].mesh = -1;
        else
        {
            const Int32 mesh = ResolveMesh( document, meshValue );
            if ( mesh >= 0 )
                m_scene.nodes[ node ].mesh = mesh;
        }
    }

    const Int32 current = m_scene.nodes[ node ].mesh;
    if ( current < 0 )
        return;

    // the materials the node sets for the surfaces of the mesh
    TInt32Vector materials;
    for ( size_t s=0; s<m_scene.meshes[ current ].subMeshes.size(); ++s )
        materials.push_back( m_scene.meshes[ current ].subMeshes[ s ].materialIndex );
    const TInt32Vector before = materials;

    const CORE::CDataNode* all = CGodotVariant::FindProperty( section, "material_override" );
    if ( GUCEF_NULL != all && !CGodotVariant::IsType( all, "null" ) )
    {
        const Int32 material = ResolveMaterial( document, all );
        if ( material >= 0 )
            for ( size_t s=0; s<materials.size(); ++s )
                materials[ s ] = material;
    }
    const CORE::CDataNode* properties = section.FindChild( "properties" );
    if ( GUCEF_NULL != properties )
    {
        for ( const CORE::CDataNode* property = properties->GetFirstChild(); GUCEF_NULL != property; property = property->GetNext() )
        {
            const CORE::CString& propertyName = property->GetName();
            if ( !propertyName.StartsWith( "surface_material_override/" ) )
                continue;
            const Int32 surface = atoi( propertyName.C_String() + 26 );
            if ( surface < 0 || (size_t) surface >= materials.size() )
            {
                m_report.AddWarning( SourceName, m_scene.nodes[ node ].name, "the material override " + propertyName + " is for a surface the mesh does not have" );
                continue;
            }
            if ( CGodotVariant::IsType( property, "null" ) )
                continue;
            const Int32 material = ResolveMaterial( document, property );
            if ( material >= 0 )
                materials[ surface ] = material;
        }
    }
    if ( materials == before )
        return;

    // a copy of the mesh for this combination of materials
    CORE::CString variantKey = CORE::Int32ToString( current ) + ":";
    for ( size_t s=0; s<materials.size(); ++s )
        variantKey += CORE::Int32ToString( materials[ s ] ) + ",";
    TIndexMap::const_iterator known = m_meshVariants.find( variantKey );
    if ( known != m_meshVariants.end() )
    {
        m_scene.nodes[ node ].mesh = (*known).second;
        return;
    }
    ASSETCONVERTER::CAssetMesh copy = m_scene.meshes[ current ];
    for ( size_t s=0; s<materials.size(); ++s )
        copy.subMeshes[ s ].materialIndex = materials[ s ];
    m_scene.meshes.push_back( copy );
    const Int32 index = (Int32) m_scene.meshes.size() - 1;
    if ( m_ownMeshes.find( current ) != m_ownMeshes.end() )
        m_ownMeshes.insert( index );
    m_meshVariants[ variantKey ] = index;
    m_scene.nodes[ node ].mesh = index;
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::ApplyLight( UInt32 node, const CORE::CString& type, const CORE::CDataNode& section )
{GUCEF_TRACE;

    const bool spot = "SpotLight3D" == type;
    ASSETCONVERTER::CAssetLight light;
    light.name = m_scene.nodes[ node ].name;
    light.type = "DirectionalLight3D" == type ? ASSETCONVERTER::LIGHTTYPE_DIRECTIONAL : ( spot ? ASSETCONVERTER::LIGHTTYPE_SPOT : ASSETCONVERTER::LIGHTTYPE_POINT );
    Float32 colour[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
    CGodotVariant::GetColour( CGodotVariant::FindProperty( section, "light_color" ), colour );
    for ( UInt32 c=0; c<3; ++c )
        light.colour[ c ] = CGodotVariant::SrgbToLinear( colour[ c ] );
    light.intensity = (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "light_energy" ), 1.0 );
    if ( ASSETCONVERTER::LIGHTTYPE_POINT == light.type )
        light.range = (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "omni_range" ), 5.0 );
    else
    if ( spot )
    {
        light.range = (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "spot_range" ), 5.0 );
        light.innerConeAngle = 0.0f;
        light.outerConeAngle = (Float32) ( CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "spot_angle" ), 45.0 ) * 3.14159265358979323846 / 180.0 );
    }
    m_report.AddApproximation( SourceName, light.name, "light_energy is used as the intensity of the glTF light without a unit conversion" );

    ASSETCONVERTER::CAssetNode& target = m_scene.nodes[ node ];
    if ( target.light >= 0 && (size_t) target.light < m_scene.lights.size() )
        m_scene.lights[ target.light ] = light;
    else
    {
        m_scene.lights.push_back( light );
        m_scene.nodes[ node ].light = (Int32) m_scene.lights.size() - 1;
    }
    CORE::CDataNode* record = m_scene.nodes[ node ].extras.FindOrAddChild( "godotLight", GUCEF_DATATYPE_OBJECT );
    record->SetAttribute( "shadowEnabled", CGodotVariant::GetBool( CGodotVariant::FindProperty( section, "shadow_enabled" ), false ) );
    record->SetAttribute( "indirectEnergy", (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "light_indirect_energy" ), 1.0 ) );
    record->SetAttribute( "specular", (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "light_specular" ), 0.5 ) );
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::ApplyBody( const CGodotDocument& document, UInt32 node, const CORE::CString& type, const CORE::CDataNode& section )
{GUCEF_TRACE;

    const char* motion = "static";
    if ( "RigidBody3D" == type || "VehicleBody3D" == type )
        motion = "dynamic";
    else
    if ( "CharacterBody3D" == type || "AnimatableBody3D" == type )
        motion = "kinematic";
    else
    if ( "Area3D" == type )
        motion = "trigger";

    CORE::CDataNode* record = m_scene.nodes[ node ].extras.FindOrAddChild( "godotBody", GUCEF_DATATYPE_OBJECT );
    record->SetAttribute( "type", type );
    record->SetAttribute( "motion", motion );
    record->SetAttribute( "layer", (Int32) CGodotVariant::GetInteger( CGodotVariant::FindProperty( section, "collision_layer" ), 1 ) );
    record->SetAttribute( "mask", (Int32) CGodotVariant::GetInteger( CGodotVariant::FindProperty( section, "collision_mask" ), 1 ) );
    if ( "dynamic" == CORE::CString( motion ) )
    {
        record->SetAttribute( "mass", (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "mass" ), 1.0 ) );
        record->SetAttribute( "gravityScale", (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "gravity_scale" ), 1.0 ) );
        record->SetAttribute( "linearDamp", (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "linear_damp" ), 0.0 ) );
        record->SetAttribute( "angularDamp", (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "angular_damp" ), 0.0 ) );
    }
    const CORE::CDataNode* material = CGodotVariant::FindProperty( section, "physics_material_override" );
    CORE::CString kind;
    CORE::CString id;
    if ( CGodotVariant::GetReference( material, kind, id ) && "SubResource" == kind )
    {
        const CORE::CDataNode* physics = document.FindSubResource( id );
        if ( GUCEF_NULL != physics )
        {
            record->SetAttribute( "friction", (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( *physics, "friction" ), 1.0 ) );
            record->SetAttribute( "bounce", (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( *physics, "bounce" ), 0.0 ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::RecordOverrides( const CGodotDocument& document, const CORE::CDataNode& section, const CORE::CString& key, bool isOverride )
{GUCEF_TRACE;

    (void) document;
    SInstance* owner = GUCEF_NULL;
    CORE::CString relative;
    if ( section.IsAttribute( "instance" ) )
    {
        for ( size_t i=0; i<m_instances.size(); ++i )
            if ( m_instances[ i ]->key == key )
                owner = m_instances[ i ];
    }
    else
    if ( isOverride )
    {
        for ( size_t i=0; i<m_instances.size(); ++i )
        {
            const CORE::CString& candidate = m_instances[ i ]->key;
            const bool inside = key == candidate || ( "." == candidate ) || key.StartsWith( candidate + "/" );
            if ( inside && ( GUCEF_NULL == owner || candidate.Length() > owner->key.Length() ) )
                owner = m_instances[ i ];
        }
        if ( GUCEF_NULL != owner && key != owner->key )
            relative = "." == owner->key ? key : key.SubstrFromRange( owner->key.Length() + 1, key.Length() );
    }
    if ( GUCEF_NULL == owner )
        return;

    const CORE::CDataNode* properties = section.FindChild( "properties" );
    if ( GUCEF_NULL == properties || !properties->HasChildren() )
        return;
    CORE::CDataNode* target = &owner->overrides;
    if ( !relative.IsNULLOrEmpty() )
    {
        CORE::CDataNode* nodes = owner->overrides.FindOrAddChild( "nodes", GUCEF_DATATYPE_OBJECT );
        target = nodes->FindOrAddChild( relative, GUCEF_DATATYPE_OBJECT );
    }
    for ( const CORE::CDataNode* property = properties->GetFirstChild(); GUCEF_NULL != property; property = property->GetNext() )
        CGodotVariant::AddPlain( *target, property->GetName(), *property );
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::ApplyProperties( const CGodotDocument& document, const CORE::CDataNode& section, UInt32 node, const CORE::CString& type, const CORE::CString& key, bool isOverride )
{GUCEF_TRACE;

    if ( !type.IsNULLOrEmpty() )
        m_scene.nodes[ node ].extras.SetAttribute( "godotType", type );
    if ( !isOverride && IsUnsupportedType( type ) )
        m_report.AddSkipped( SourceName, m_scene.nodes[ node ].name, "the node type " + type + " has no counterpart in the model: the node is kept without its content" );

    ApplyTransform( node, section );
    const CORE::CDataNode* visible = CGodotVariant::FindProperty( section, "visible" );
    if ( GUCEF_NULL != visible )
        m_scene.nodes[ node ].extras.SetAttribute( "visible", CGodotVariant::GetBool( visible, true ) );

    ApplyMesh( document, section, node );

    if ( "CollisionShape3D" == type )
    {
        SShape& shape = m_shapes[ node ];
        const CORE::CDataNode* value = CGodotVariant::FindProperty( section, "shape" );
        if ( GUCEF_NULL != value )
        {
            shape.document = &document;
            shape.value = value;
        }
        const CORE::CDataNode* disabled = CGodotVariant::FindProperty( section, "disabled" );
        if ( GUCEF_NULL != disabled )
            shape.disabled = CGodotVariant::GetBool( disabled, false );
    }
    if ( IsBodyType( type ) )
        ApplyBody( document, node, type, section );
    if ( IsLightType( type ) )
        ApplyLight( node, type, section );
    if ( CGodotParticleReader::IsParticlesType( type ) )
    {
        if ( !isOverride )
        {
            CORE::CString subEmitterPath;
            ASSETCONVERTER::TParticleTrigger trigger = ASSETCONVERTER::PARTICLETRIGGER_BIRTH;
            const Int32 system = m_particles.Read( document, section, type, m_scene.nodes[ node ].name, subEmitterPath, trigger );
            m_scene.nodes[ node ].particleSystem = system;
            if ( system >= 0 && !subEmitterPath.IsNULLOrEmpty() )
            {
                SSubEmitterLink link;
                link.system = (UInt32) system;
                link.nodeKey = key;
                link.path = subEmitterPath;
                link.trigger = trigger;
                m_subEmitterLinks.push_back( link );
            }
        }
    }
    if ( isOverride && m_scene.nodes[ node ].particleSystem >= 0 )
    {
        // an instancing scene that changes more than the place of a particle node: those changes are not applied
        const CORE::CDataNode* properties = section.FindChild( "properties" );
        for ( const CORE::CDataNode* property = GUCEF_NULL != properties ? properties->GetFirstChild() : GUCEF_NULL; GUCEF_NULL != property; property = property->GetNext() )
        {
            const CORE::CString& name = property->GetName();
            if ( "transform" != name && "position" != name && "rotation" != name && "quaternion" != name && "scale" != name && "visible" != name )
            {
                m_report.AddApproximation( SourceName, m_scene.nodes[ node ].name, "the instancing scene changes the property " + name + " of the particle system: not applied" );
                break;
            }
        }
    }

    if ( !m_scriptReported && GUCEF_NULL != CGodotVariant::FindProperty( section, "script" ) )
    {
        m_scriptReported = true;
        m_report.AddInfo( SourceName, m_assetPath, "scripts of the nodes are not imported" );
    }
    RecordOverrides( document, section, key, isOverride );
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::BuildColliders( void )
{GUCEF_TRACE;

    for ( TShapeMap::const_iterator s = m_shapes.begin(); s != m_shapes.end(); ++s )
    {
        const UInt32 shapeNode = (*s).first;
        const SShape& shape = (*s).second;
        if ( shape.disabled || GUCEF_NULL == shape.value || GUCEF_NULL == shape.document )
            continue;
        const CORE::CString name = m_scene.nodes[ shapeNode ].name;

        const CGodotDocument* resourceDocument = GUCEF_NULL;
        const CORE::CDataNode* section = GUCEF_NULL;
        CORE::CString key;
        if ( !ResolveResource( *shape.document, shape.value, resourceDocument, section, key ) )
            continue;
        CORE::CString type = section->GetAttributeValue( "type" ).AsString();
        if ( type.IsNULLOrEmpty() )
            type = resourceDocument->GetResourceType();

        // the nearest body above the shape
        Int32 body = -1;
        for ( Int32 p = m_scene.nodes[ shapeNode ].parent; p >= 0; p = m_scene.nodes[ p ].parent )
        {
            if ( IsBodyType( GetNodeType( (UInt32) p ) ) )
            {
                body = p;
                break;
            }
        }
        MATH::TFloat32Transform3D relative;
        bool trigger = false;
        if ( body >= 0 )
        {
            trigger = "Area3D" == GetNodeType( (UInt32) body );
            const MATH::TFloat32Matrix4D combined = m_scene.GetWorldMatrix( (UInt32) body ).InverseAffine() * m_scene.GetWorldMatrix( shapeNode );
            if ( !MATH::TFloat32Transform3D::FromMatrix( combined, relative ) )
            {
                m_report.AddApproximation( SourceName, name, "the position of the shape in its body has a shear: the shape is placed at the origin of the body" );
                relative = MATH::TFloat32Transform3D();
            }
        }

        ASSETCONVERTER::CAssetCollider collider;
        if ( CGodotShapeReader::Read( type, *section, relative, trigger, name, m_scene, collider, m_report ) )
        {
            m_scene.nodes[ body >= 0 ? (UInt32) body : shapeNode ].colliders.push_back( collider );
            if ( body >= 0 && m_scene.nodes[ shapeNode ].children.empty() && m_scene.nodes[ shapeNode ].mesh < 0 && m_scene.nodes[ shapeNode ].light < 0 )
                m_carriers.insert( shapeNode );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::FinishMeshes( void )
{GUCEF_TRACE;

    // Godot uses the colours of the vertices only when the material says so
    for ( TInt32Set::const_iterator m = m_ownMeshes.begin(); m != m_ownMeshes.end(); ++m )
    {
        ASSETCONVERTER::CAssetMesh& mesh = m_scene.meshes[ *m ];
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        {
            ASSETCONVERTER::CAssetSubMesh& subMesh = mesh.subMeshes[ s ];
            if ( subMesh.colours.empty() )
                continue;
            const Int32 material = subMesh.materialIndex;
            const bool used = material >= 0 && (size_t) material < m_scene.materials.size() &&
                              m_scene.materials[ material ].source.properties.GetAttributeValue( "vertex_color_use_as_albedo" ).AsBool( false );
            if ( !used )
                subMesh.colours.clear();
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotSceneReader::ResolveResourceSection( const CGodotDocument& document, const CORE::CDataNode* value, const CGodotDocument*& resourceDocument, const CORE::CDataNode*& section )
{GUCEF_TRACE;

    CORE::CString key;
    return ResolveResource( document, value, resourceDocument, section, key );
}

/*-------------------------------------------------------------------------*/

Int32
CGodotSceneReader::ResolveMaterialResource( const CGodotDocument& document, const CORE::CDataNode* value )
{GUCEF_TRACE;

    return ResolveMaterial( document, value );
}

/*-------------------------------------------------------------------------*/

Int32
CGodotSceneReader::ResolveMeshSection( const CGodotDocument& document, const CORE::CDataNode& section )
{GUCEF_TRACE;

    CORE::CString id = section.GetAttributeValue( "id" ).AsString();
    if ( id.IsNULLOrEmpty() )
        id = "resource";
    return MeshFromSection( document, section, document.GetAssetPath() + "#" + id );
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::ResolveSubEmitters( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_subEmitterLinks.size(); ++i )
    {
        const SSubEmitterLink& link = m_subEmitterLinks[ i ];

        // the node path is relative to the particle node: "..", "." and names
        CORE::CString::StringVector segments;
        if ( "." != link.nodeKey )
            segments = link.nodeKey.ParseElements( '/', false );
        const CORE::CString::StringVector steps = link.path.ParseElements( '/', false );
        for ( size_t s=0; s<steps.size(); ++s )
        {
            if ( ".." == steps[ s ] )
            {
                if ( !segments.empty() )
                    segments.pop_back();
            }
            else
            if ( "." != steps[ s ] )
                segments.push_back( steps[ s ] );
        }
        CORE::CString key = ".";
        for ( size_t s=0; s<segments.size(); ++s )
            key = JoinKey( key, segments[ s ] );

        TPathMap::const_iterator found = m_paths.find( key );
        const Int32 target = found != m_paths.end() ? m_scene.nodes[ (*found).second ].particleSystem : -1;
        ASSETCONVERTER::CAssetParticleSystem& system = m_scene.particleSystems[ link.system ];
        if ( target < 0 )
        {
            m_report.AddWarning( SourceName, system.name, "the sub-emitter " + link.path + " is not a particle node of the scene" );
            continue;
        }
        ASSETCONVERTER::CAssetParticleSystem::SSubEmitter sub;
        sub.system = target;
        sub.trigger = link.trigger;
        system.subEmitters.push_back( sub );
    }
}

/*-------------------------------------------------------------------------*/

void
CGodotSceneReader::FinishPrefab( void )
{GUCEF_TRACE;

    TPathMap::const_iterator root = m_paths.find( "." );
    if ( root == m_paths.end() )
        return;
    ASSETCONVERTER::CAssetPrefab prefab;
    prefab.name = m_scene.name.IsNULLOrEmpty() ? Stem( m_assetPath ) : m_scene.name;
    prefab.rootNode = (*root).second;
    for ( size_t i=0; i<m_instances.size(); ++i )
    {
        ASSETCONVERTER::CAssetPrefabInstance instance;
        instance.node = m_instances[ i ]->node;
        instance.sourcePrefab = m_instances[ i ]->sourcePrefab;
        instance.overrides = m_instances[ i ]->overrides;
        prefab.instances.push_back( instance );
    }
    m_scene.prefabs.push_back( prefab );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
