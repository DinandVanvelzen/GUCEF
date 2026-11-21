/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "assetcookerOGRE_CMaterialScriptWriter.h"
#include "assetcookerOGRE_CSkeletonCooker.h"
#include "assetcookerOGRE_CMeshCooker.h"
#include "assetcookerOGRE_CLodCooker.h"
#include "assetcookerOGRE_CDotSceneWriter.h"
#include "assetcookerOGRE_CCollisionWriter.h"
#include "assetcookerOGRE_CParticleScriptWriter.h"
#include "AssetConverterLib_CConversionSidecar.h"
#include "assetcookerOGRE_CAssetCook.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "ogre";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetCook::CAssetCook( const ASSETCONVERTER::CCookSettings& settings       ,
                        const ASSETCONVERTER::CCookSettings::SAsset& asset  ,
                        const CORE::CString& profileDir                     ,
                        CTextureCooker& textures                            ,
                        ASSETCONVERTER::CConversionReport& report           )
    : m_settings( settings )
    , m_asset( asset )
    , m_profileDir( ASSETCONVERTER::CVfsPaths::Normalize( profileDir ) )
    , m_textures( textures )
    , m_report( report )
    , m_names( settings.packId, asset.assetPath )
    , m_scene( GUCEF_NULL )
    , m_materialNames()
    , m_defaultMaterialName()
    , m_materialScript()
    , m_skeletons()
    , m_skeletonNames()
    , m_meshes()
    , m_ok( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetCook::~CAssetCook()
{GUCEF_TRACE;

    for ( size_t i=0; i<m_skeletons.size(); ++i )
        GUCEF_DELETE m_skeletons[ i ];
    m_skeletons.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetCook::ToVfsPath( const CORE::CString& name ) const
{GUCEF_TRACE;

    return ASSETCONVERTER::CVfsPaths::Join( m_profileDir, name );
}

/*-------------------------------------------------------------------------*/

void
CAssetCook::AddFile( const CORE::CString& name, const CORE::CString::StringVector& referenceNames )
{GUCEF_TRACE;

    CORE::CString::StringVector references;
    for ( size_t i=0; i<referenceNames.size(); ++i )
        if ( !referenceNames[ i ].IsNULLOrEmpty() )
            references.push_back( ToVfsPath( referenceNames[ i ] ) );
    m_report.AddOutputFile( ToVfsPath( name ), references, m_settings.profile.name );
}

/*-------------------------------------------------------------------------*/

bool
CAssetCook::WriteText( const CORE::CString& name, const CORE::CString& text )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString path = ToVfsPath( name );
    vfs.CreateDir( ASSETCONVERTER::CVfsPaths::FolderOf( path ) );
    CORE::CDynamicBuffer bytes;
    bytes.CopyFrom( text.Length(), text.C_String() );
    if ( !vfs.StoreAsFile( path, bytes, 0, true ) )
    {
        m_report.AddError( SourceName, path, "the file cannot be written" );
        m_ok = false;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetCook::CookMesh( UInt32 mesh, Int32 skin, const TUInt32Vector& manualLevelNodes, const GUCEF::vector< Float32 >& manualLevelHeights )
{GUCEF_TRACE;

    const ASSETCONVERTER::CAssetScene& scene = *m_scene;
    for ( size_t i=0; i<m_meshes.size(); ++i )
        if ( m_meshes[ i ].mesh == mesh && m_meshes[ i ].skin == skin )
            return m_meshes[ i ].name;

    const CORE::CString name = m_names.MakeFileName( scene.meshes[ mesh ].name, "mesh", "mesh" + CORE::UInt32ToString( mesh ) );
    SCookedMesh cooked;
    cooked.mesh = mesh;
    cooked.skin = skin;
    cooked.name = name;
    const size_t cookedIndex = m_meshes.size();
    m_meshes.push_back( cooked );

    // the manual levels: the meshes of the other levels, cooked first
    CLodCooker::TLevelVector levels;
    CORE::CString::StringVector references;
    for ( size_t l=0; l<manualLevelNodes.size(); ++l )
    {
        const Int32 levelMesh = scene.nodes[ manualLevelNodes[ l ] ].mesh;
        if ( levelMesh < 0 || (UInt32) levelMesh == mesh )
        {
            m_report.AddInfo( SourceName, scene.nodes[ manualLevelNodes[ l ] ].name, "the LOD level shows no other mesh than the first level: left out" );
            continue;
        }
        CLodCooker::SLevel level;
        level.screenRelativeHeight = manualLevelHeights[ l ];
        level.manualMeshName = CookMesh( (UInt32) levelMesh, skin, TUInt32Vector(), GUCEF::vector< Float32 >() );
        if ( level.manualMeshName.IsNULLOrEmpty() )
            continue;
        levels.push_back( level );
        references.push_back( level.manualMeshName );
    }

    // the nodes that show the mesh, whose weight channels animate its poses
    TUInt32Vector users;
    for ( size_t n=0; n<scene.nodes.size(); ++n )
        if ( scene.nodes[ n ].mesh == (Int32) mesh )
            users.push_back( (UInt32) n );

    Ogre::SkeletonPtr skeleton;
    if ( skin >= 0 && (size_t) skin < m_skeletons.size() && GUCEF_NULL != m_skeletons[ skin ] )
        skeleton = m_skeletons[ skin ]->GetSkeleton();

    CMeshCooker cooker( scene, m_report );
    if ( !cooker.Build( mesh, name, m_materialNames, m_defaultMaterialName, skeleton ? skin : -1, skeleton, users ) )
    {
        m_ok = false;
        m_meshes[ cookedIndex ].name.Clear();
        return CORE::CString::Empty;
    }

    UInt32 lodLevels = 0;
    if ( !levels.empty() )
        lodLevels = CLodCooker::ApplyManualLevels( cooker.GetMesh(), levels, m_settings.profile.lodBias, m_report );
    else
    if ( m_settings.lodPolicy.Equals( "generate", false ) && manualLevelNodes.empty() )
        lodLevels = CLodCooker::GenerateLevels( cooker.GetMesh(), m_settings.lodSettings, m_settings.profile.lodBias, m_report );
    if ( lodLevels > 0 )
        m_report.AddInfo( SourceName, scene.meshes[ mesh ].name, CORE::UInt32ToString( lodLevels ) + ( levels.empty() ? " generated" : " manual" ) + " level(s) of detail" );

    if ( !cooker.Save( ToVfsPath( name ) ) )
    {
        m_ok = false;
        m_meshes[ cookedIndex ].name.Clear();
        return CORE::CString::Empty;
    }
    references.push_back( m_materialScript );
    if ( skeleton && skin >= 0 && (size_t) skin < m_skeletonNames.size() )
        references.push_back( m_skeletonNames[ skin ] );
    AddFile( name, references );
    return name;
}

/*-------------------------------------------------------------------------*/

void
CAssetCook::ReadPrefabs( const CORE::CDataNode& sidecar, GUCEF::vector< ASSETCONVERTER::CAssetPrefab >& prefabs ) const
{GUCEF_TRACE;

    const ASSETCONVERTER::CAssetScene& scene = *m_scene;
    const CORE::CDataNode* list = sidecar.FindChild( "prefabs" );
    if ( GUCEF_NULL == list )
        return;
    CORE::CDataNode::const_iterator i = list->ConstBegin();
    while ( i != list->ConstEnd() )
    {
        const CORE::CDataNode& entry = *(*i);
        ++i;
        const Int32 root = scene.FindNode( entry.GetAttributeValueOrChildValueByName( "root" ).AsString() );
        if ( root < 0 )
        {
            m_report.AddWarning( SourceName, entry.GetAttributeValueOrChildValueByName( "name" ).AsString(), "the root node of the prefab is not in the intermediate: no DotScene for it" );
            continue;
        }
        ASSETCONVERTER::CAssetPrefab prefab;
        prefab.name = entry.GetAttributeValueOrChildValueByName( "name" ).AsString();
        prefab.rootNode = (UInt32) root;
        const CORE::CDataNode* instances = entry.FindChild( "instances" );
        if ( GUCEF_NULL != instances )
        {
            CORE::CDataNode::const_iterator k = instances->ConstBegin();
            while ( k != instances->ConstEnd() )
            {
                const Int32 node = scene.FindNode( (*k)->GetAttributeValueOrChildValueByName( "node" ).AsString() );
                if ( node >= 0 )
                {
                    ASSETCONVERTER::CAssetPrefabInstance instance;
                    instance.node = (UInt32) node;
                    instance.sourcePrefab = (*k)->GetAttributeValueOrChildValueByName( "sourcePrefab" ).AsString();
                    const CORE::CDataNode* overrides = (*k)->FindChild( "overrides" );
                    if ( GUCEF_NULL != overrides )
                        instance.overrides = *overrides;
                    prefab.instances.push_back( instance );
                }
                ++k;
            }
        }
        prefabs.push_back( prefab );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetCook::Cook( const ASSETCONVERTER::CAssetScene& scene, const CORE::CDataNode& sidecar )
{GUCEF_TRACE;

    m_scene = &scene;
    m_ok = true;
    const CORE::CString stem = m_names.GetAssetStem();

    // the levels of detail: per first level the nodes of the other levels; those nodes are not shown on their own
    GUCEF::map< UInt32, TUInt32Vector > levelNodes;
    GUCEF::map< UInt32, GUCEF::vector< Float32 > > levelHeights;
    GUCEF::set< UInt32 > secondaryLevels;
    const bool noLod = m_settings.lodPolicy.Equals( "none", false );
    for ( size_t g=0; g<scene.lodGroups.size(); ++g )
    {
        const ASSETCONVERTER::CAssetLodGroup& group = scene.lodGroups[ g ];
        if ( group.levels.size() < 2 || group.levels[ 0 ].node >= scene.nodes.size() )
            continue;
        for ( size_t l=1; l<group.levels.size(); ++l )
        {
            if ( group.levels[ l ].node >= scene.nodes.size() )
                continue;
            secondaryLevels.insert( group.levels[ l ].node );
            if ( !noLod )
            {
                levelNodes[ group.levels[ 0 ].node ].push_back( group.levels[ l ].node );
                levelHeights[ group.levels[ 0 ].node ].push_back( group.levels[ l ].screenRelativeHeight );
            }
        }
        if ( noLod )
            m_report.AddSkipped( SourceName, group.name, "LOD policy none: only the first level of the group is cooked" );
    }

    // the materials, all in one script
    bool needsDefault = false;
    for ( size_t m=0; m<scene.meshes.size(); ++m )
        for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
            needsDefault = needsDefault || scene.meshes[ m ].subMeshes[ s ].materialIndex < 0 || (size_t) scene.meshes[ m ].subMeshes[ s ].materialIndex >= scene.materials.size();
    for ( size_t m=0; m<scene.materials.size(); ++m )
        m_materialNames.push_back( m_names.MakeMaterialName( scene.materials[ m ].name, "material" + CORE::UInt32ToString( (UInt32) m ) ) );
    if ( needsDefault )
        m_defaultMaterialName = m_names.MakeMaterialName( "Default", "Default" );

    // the outfits that are worn: the intermediate says which, the pack setting "hiddenFaceSets" (a list of names, or "none") chooses
    // others. The triangles of the body that a set hides keep their place in the mesh with a material variant per material and sets
    ASSETCONVERTER::CAssetHiddenFaceSet::THiddenFaceSetVector hiddenFaceSets( scene.hiddenFaceSets );
    CORE::CString::StringVector chosenSets;
    if ( !hiddenFaceSets.empty() && m_settings.settings.GetStringListByName( "hiddenFaceSets", chosenSets ) )
    {
        CORE::CString::StringVector unknown;
        ASSETCONVERTER::CAssetHiddenFaceSet::Activate( hiddenFaceSets, chosenSets, unknown );
        for ( size_t u=0; u<unknown.size(); ++u )
            m_report.AddWarning( SourceName, stem, "the setting hiddenFaceSets names " + unknown[ u ] + ", which is not a hidden face set of the asset" );
    }
    const UInt32 activeSets = ASSETCONVERTER::CAssetHiddenFaceSet::GetActiveMask( hiddenFaceSets );
    TMaterialVariantVector hiddenVariants;
    for ( size_t m=0; m<scene.meshes.size(); ++m )
        for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
        {
            const ASSETCONVERTER::CAssetSubMesh& subMesh = scene.meshes[ m ].subMeshes[ s ];
            if ( 0 == subMesh.hiddenBy )
                continue;
            const Int32 material = subMesh.materialIndex >= 0 && (size_t) subMesh.materialIndex < scene.materials.size() ? subMesh.materialIndex : -1;
            bool known = false;
            for ( size_t v=0; v<hiddenVariants.size() && !known; ++v )
                known = hiddenVariants[ v ].first == material && hiddenVariants[ v ].second == subMesh.hiddenBy;
            if ( !known )
                hiddenVariants.push_back( TMaterialVariant( material, subMesh.hiddenBy ) );
        }
    if ( !hiddenFaceSets.empty() )
        m_report.AddInfo( SourceName, stem, "hidden face sets: " + ASSETCONVERTER::CAssetHiddenFaceSet::NamesOf( hiddenFaceSets, 0xFFFFFFFF, ", " ) + "; worn (their faces of the body are not drawn): " +
                          ( 0 == activeSets ? CORE::CString( "none" ) : ASSETCONVERTER::CAssetHiddenFaceSet::NamesOf( hiddenFaceSets, activeSets, ", " ) ) + "; " +
                          CORE::UInt32ToString( (UInt32) hiddenVariants.size() ) + " material variant(s) for the hidden triangles" );

    // the particle systems of the sidecar (the glTF nodes name them by index): a template and a material each
    GUCEF::vector< ASSETCONVERTER::CAssetParticleSystem > particleSystems;
    GUCEF::vector< Int32 > particleNodes;
    if ( !ASSETCONVERTER::CConversionSidecar::ReadParticleSystems( sidecar, particleSystems, particleNodes ) )
        m_report.AddWarning( SourceName, stem, "a particle system of the sidecar cannot be read" );
    CORE::CString::StringVector particleTemplates;
    CORE::CString::StringVector particleMaterials;
    for ( size_t p=0; p<particleSystems.size(); ++p )
    {
        const CORE::CString name = particleSystems[ p ].name.IsNULLOrEmpty() ? "particles" + CORE::UInt32ToString( (UInt32) p ) : particleSystems[ p ].name;
        particleTemplates.push_back( m_names.MakeMaterialName( name + ".particles", "particles" + CORE::UInt32ToString( (UInt32) p ) ) );
        particleMaterials.push_back( m_names.MakeMaterialName( name + ".particles/material", "particles" + CORE::UInt32ToString( (UInt32) p ) + "/material" ) );
    }

    if ( !scene.materials.empty() || needsDefault || !particleSystems.empty() )
    {
        m_materialScript = m_names.MakeFileName( stem, "material", "asset" );
        CMaterialScriptWriter script( scene, m_textures, m_settings.profile, m_report );
        for ( size_t m=0; m<scene.materials.size(); ++m )
            script.AddMaterial( m_materialNames[ m ], scene.materials[ m ] );
        if ( needsDefault )
        {
            ASSETCONVERTER::CAssetMaterial fallback;
            fallback.name = "Default";
            script.AddMaterial( m_defaultMaterialName, fallback );
        }
        for ( size_t v=0; v<hiddenVariants.size(); ++v )
        {
            const Int32 material = hiddenVariants[ v ].first;
            const UInt32 hiddenBy = hiddenVariants[ v ].second;
            const CORE::CString baseName = material >= 0 ? m_materialNames[ (size_t) material ] : m_defaultMaterialName;
            const CORE::CString variantName = CMeshCooker::GetHiddenFaceMaterialName( baseName, scene, hiddenBy );
            if ( 0 != ( hiddenBy & activeSets ) )
            {
                script.AddHiddenMaterial( variantName, "the triangles of " + baseName + " that the outfit " + ASSETCONVERTER::CAssetHiddenFaceSet::NamesOf( hiddenFaceSets, hiddenBy & activeSets, " and " ) +
                                          " hides: not drawn while it is worn (give the submeshes the material " + baseName + " to show them)" );
            }
            else
            {
                ASSETCONVERTER::CAssetMaterial shown;
                if ( material >= 0 )
                    shown = scene.materials[ (size_t) material ];
                script.AddMaterial( variantName, shown );
            }
        }
        for ( size_t p=0; p<particleSystems.size(); ++p )
            if ( ASSETCONVERTER::PARTICLERENDER_NONE != particleSystems[ p ].renderer.mode )
                script.AddParticleMaterial( particleMaterials[ p ], particleSystems[ p ] );
        CORE::CString::StringVector textureNames;
        for ( size_t r=0; r<script.GetReferences().size(); ++r )
        {
            CORE::CString relative;
            if ( ASSETCONVERTER::CVfsPaths::MakeRelative( m_profileDir, script.GetReferences()[ r ], relative ) )
            {
                textureNames.push_back( relative );
                AddFile( relative, CORE::CString::StringVector() );
            }
        }
        if ( WriteText( m_materialScript, script.GetText() ) )
            AddFile( m_materialScript, textureNames );
    }

    // a skeleton per skin
    for ( size_t s=0; s<scene.skins.size(); ++s )
    {
        CSkeletonCooker* cooker = GUCEF_NEW CSkeletonCooker( scene, m_report );
        const CORE::CString skinName = scene.skins[ s ].name.IsNULLOrEmpty() ? stem : scene.skins[ s ].name;
        const CORE::CString name = m_names.MakeFileName( skinName, "skeleton", "skeleton" );
        if ( cooker->Build( (UInt32) s, name ) && cooker->Save( ToVfsPath( name ) ) )
        {
            AddFile( name, CORE::CString::StringVector() );
            m_skeletons.push_back( cooker );
            m_skeletonNames.push_back( name );
        }
        else
        {
            m_ok = false;
            GUCEF_DELETE cooker;
            m_skeletons.push_back( GUCEF_NULL );
            m_skeletonNames.push_back( CORE::CString::Empty );
        }
    }

    // the meshes the nodes show
    CDotSceneWriter dotScene( scene, m_report );
    dotScene.SetMaterialNames( m_materialNames );
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        const ASSETCONVERTER::CAssetNode& node = scene.nodes[ n ];
        if ( secondaryLevels.find( (UInt32) n ) != secondaryLevels.end() )
        {
            dotScene.Exclude( (UInt32) n );
            continue;
        }
        if ( node.mesh < 0 || (size_t) node.mesh >= scene.meshes.size() )
            continue;
        bool skinnedMesh = false;
        for ( size_t s=0; s<scene.meshes[ node.mesh ].subMeshes.size(); ++s )
            skinnedMesh = skinnedMesh || scene.meshes[ node.mesh ].subMeshes[ s ].IsSkinned();
        const Int32 skin = ( skinnedMesh && node.skin >= 0 && (size_t) node.skin < scene.skins.size() ) ? node.skin : -1;
        const CORE::CString meshName = CookMesh( (UInt32) node.mesh, skin, levelNodes[ (UInt32) n ], levelHeights[ (UInt32) n ] );
        if ( !meshName.IsNULLOrEmpty() )
        {
            dotScene.SetEntity( (UInt32) n, meshName, skin >= 0 );
            // the outfits that hide submeshes of the mesh, and those of them that are worn: for applications that switch outfits
            UInt32 meshSets = 0;
            for ( size_t s=0; s<scene.meshes[ node.mesh ].subMeshes.size(); ++s )
                meshSets |= scene.meshes[ node.mesh ].subMeshes[ s ].hiddenBy;
            if ( 0 != meshSets )
            {
                dotScene.SetEntityProperty( (UInt32) n, "gucefHiddenFaceSets", ASSETCONVERTER::CAssetHiddenFaceSet::NamesOf( hiddenFaceSets, meshSets, "+" ) );
                dotScene.SetEntityProperty( (UInt32) n, "gucefActiveHiddenFaceSets", ASSETCONVERTER::CAssetHiddenFaceSet::NamesOf( hiddenFaceSets, meshSets & activeSets, "+" ) );
            }
        }
    }

    // the particle script: every system as a template; the systems that only a sub-emitter starts are not attached
    UInt32 particleCount = 0;
    if ( !particleSystems.empty() )
    {
        CParticleScriptWriter particles( scene, m_report );
        GUCEF::vector< bool > written( particleSystems.size(), false );
        for ( size_t p=0; p<particleSystems.size(); ++p )
            written[ p ] = particles.AddSystem( particleTemplates[ p ], particleMaterials[ p ], particleSystems[ p ] );
        particleCount = particles.GetSystemCount();
        if ( particleCount > 0 )
        {
            const CORE::CString particleName = m_names.MakeFileName( stem, "particle", "asset" );
            CORE::CString::StringVector references;
            if ( !m_materialScript.IsNULLOrEmpty() )
                references.push_back( m_materialScript );
            if ( WriteText( particleName, particles.GetText() ) )
                AddFile( particleName, references );
        }
        for ( size_t n=0; n<scene.nodes.size(); ++n )
        {
            const Int32 system = scene.nodes[ n ].particleSystem;
            if ( system < 0 || (size_t) system >= particleSystems.size() || !written[ system ] )
                continue;
            if ( ASSETCONVERTER::CAssetParticleSystem::IsSubEmitter( particleSystems, (UInt32) system ) )
            {
                m_report.AddSkipped( SourceName, particleSystems[ system ].name, "the system is a sub-emitter: its template is written but it is not attached to its node" );
                continue;
            }
            const ASSETCONVERTER::CAssetParticleSystem& source = particleSystems[ system ];
            dotScene.SetParticleSystem( (UInt32) n, particleTemplates[ system ], source.prewarm / ( source.simulationSpeed > 0.0f ? source.simulationSpeed : 1.0f ),
                                        source.startDelay.GetAverage() / ( source.simulationSpeed > 0.0f ? source.simulationSpeed : 1.0f ), source.playOnAwake );
        }
    }

    // the scene of the asset and of its prefabs
    GUCEF::vector< ASSETCONVERTER::CAssetPrefab > prefabs;
    ReadPrefabs( sidecar, prefabs );
    const ASSETCONVERTER::CAssetPrefab* wholeAsset = GUCEF_NULL;
    if ( 1 == prefabs.size() && 1 == scene.rootNodes.size() && prefabs[ 0 ].rootNode == scene.rootNodes[ 0 ] )
        wholeAsset = &prefabs[ 0 ];
    const CORE::CString sceneName = m_names.MakeFileName( stem, "scene", "asset" );
    const CORE::CString sceneText = dotScene.Write( scene.rootNodes, wholeAsset, -1 );
    const CDotSceneWriter::TNameMap nodeNames = dotScene.GetNodeNames();
    if ( WriteText( sceneName, sceneText ) )
        AddFile( sceneName, dotScene.GetReferences() );
    for ( size_t p=0; p<prefabs.size(); ++p )
    {
        if ( &prefabs[ p ] == wholeAsset )
            continue;
        TUInt32Vector roots;
        roots.push_back( prefabs[ p ].rootNode );
        const CORE::CString prefabName = m_names.MakeFileName( prefabs[ p ].name, "scene", "prefab" + CORE::UInt32ToString( (UInt32) p ) );
        const CORE::CString text = dotScene.Write( roots, &prefabs[ p ], (Int32) prefabs[ p ].rootNode );
        if ( WriteText( prefabName, text ) )
            AddFile( prefabName, dotScene.GetReferences() );
    }

    // the colliders
    const UInt32 colliders = CCollisionWriter::CountColliders( scene );
    if ( colliders > 0 )
    {
        if ( m_settings.colliderPolicy.Equals( "none", false ) )
        {
            m_report.AddSkipped( SourceName, stem, "collider policy none: " + CORE::UInt32ToString( colliders ) + " collider(s) not written" );
        }
        else
        {
            CORE::CDataNode document;
            CCollisionWriter::Build( scene, m_asset.assetPath, sceneName, nodeNames, document );
            const CORE::CString collisionName = m_names.MakeFileName( stem + ".collision", "json", "asset.collision" );
            if ( ASSETCONVERTER::CCanonicalJson::Store( ToVfsPath( collisionName ), document ) )
            {
                CORE::CString::StringVector references;
                references.push_back( sceneName );
                AddFile( collisionName, references );
            }
            else
            {
                m_report.AddError( SourceName, collisionName, "the colliders cannot be written" );
                m_ok = false;
            }
        }
    }

    UInt32 meshCount = 0;
    for ( size_t i=0; i<m_meshes.size(); ++i )
        if ( !m_meshes[ i ].name.IsNULLOrEmpty() )
            ++meshCount;
    m_report.AddInfo( SourceName, stem, "cooked for Ogre 1.x: " + CORE::UInt32ToString( meshCount ) + " mesh(es), " + CORE::UInt32ToString( (UInt32) scene.skins.size() ) + " skeleton(s), " +
                      CORE::UInt32ToString( (UInt32) scene.materials.size() ) + " material(s), " + CORE::UInt32ToString( (UInt32) prefabs.size() ) + " prefab(s), " + CORE::UInt32ToString( colliders ) + " collider(s), " +
                      CORE::UInt32ToString( particleCount ) + " particle system(s)" );
    m_scene = GUCEF_NULL;
    return m_ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
