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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENESUMMARY_H
#include "AssetConverterLib_CAssetSceneSummary.h"
#define ASSETCONVERTERLIB_CASSETSCENESUMMARY_H
#endif /* ASSETCONVERTERLIB_CASSETSCENESUMMARY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CConversionSidecar.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A VFS path as a path in the pack when it is in the source, else as it is */
CORE::CString
InPack( const CAssetPackContext& context, const CORE::CString& vfsPath )
{
    CORE::CString assetPath;
    if ( context.ToAssetPath( vfsPath, assetPath ) )
        return assetPath;
    return CVfsPaths::Normalize( vfsPath );
}

CORE::CString
NodeName( const CAssetScene& scene, Int32 node )
{
    if ( node < 0 || (size_t) node >= scene.nodes.size() )
        return CORE::CString::Empty;
    return scene.nodes[ node ].name;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CConversionSidecar::GetPath( const CORE::CString& intermediateDir, const CORE::CString& assetPath )
{GUCEF_TRACE;

    return CVfsPaths::Join( intermediateDir, CVfsPaths::WithoutExtension( assetPath ) + ".conversion.json" );
}

/*-------------------------------------------------------------------------*/

void
CConversionSidecar::Build( const CAssetScene& scene                     ,
                           const CConversionReportEntry& entry          ,
                           const CORE::CString::StringVector& layers    ,
                           const CORE::CString& licenceRecordId         ,
                           const CAssetPackContext& context             ,
                           const CORE::CString& intermediateDir         ,
                           CORE::CDataNode& sidecar                     )
{GUCEF_TRACE;

    sidecar = CORE::CDataNode( "sidecar", GUCEF_DATATYPE_OBJECT );
    scene.provenance.Serialize( *sidecar.AddChild( "provenance", GUCEF_DATATYPE_OBJECT ) );
    sidecar.SetAttribute( "licenceRecord", licenceRecordId );

    CORE::CDataNode* settings = sidecar.AddChild( scene.settings );
    settings->SetName( "settings" );
    sidecar.SetValuesOfChildByName( "settingsLayers", layers );
    CORE::CDataNode* normalisation = sidecar.AddChild( scene.normalisation );
    normalisation->SetName( "normalisation" );

    CORE::CDataNode* materials = sidecar.AddChild( "materials", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<scene.materials.size(); ++i )
        scene.materials[ i ].Serialize( *materials->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );

    CORE::CDataNode* textures = sidecar.AddChild( "textures", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<scene.textures.size(); ++i )
    {
        CAssetTexture texture = scene.textures[ i ];
        for ( size_t s=0; s<texture.sources.size(); ++s )
            texture.sources[ s ] = InPack( context, texture.sources[ s ] );
        CORE::CString baked;
        if ( !texture.bakedPath.IsNULLOrEmpty() && CVfsPaths::MakeRelative( intermediateDir, texture.bakedPath, baked ) )
            texture.bakedPath = baked;
        texture.Serialize( *textures->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }

    CORE::CDataNode* lights = sidecar.AddChild( "lights", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<scene.lights.size(); ++i )
        scene.lights[ i ].Serialize( *lights->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );

    CORE::CDataNode* colliders = sidecar.AddChild( "colliders", GUCEF_DATATYPE_ARRAY );
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        for ( size_t c=0; c<scene.nodes[ n ].colliders.size(); ++c )
        {
            CORE::CDataNode* collider = colliders->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            collider->SetAttribute( "node", scene.nodes[ n ].name );
            scene.nodes[ n ].colliders[ c ].Serialize( *collider );
        }
    }

    CORE::CDataNode* lodGroups = sidecar.AddChild( "lodGroups", GUCEF_DATATYPE_ARRAY );
    for ( size_t g=0; g<scene.lodGroups.size(); ++g )
    {
        CORE::CDataNode* group = lodGroups->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        group->SetAttribute( "name", scene.lodGroups[ g ].name );
        CORE::CDataNode* levels = group->AddChild( "levels", GUCEF_DATATYPE_ARRAY );
        for ( size_t l=0; l<scene.lodGroups[ g ].levels.size(); ++l )
        {
            CORE::CDataNode* level = levels->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            level->SetAttribute( "node", NodeName( scene, (Int32) scene.lodGroups[ g ].levels[ l ].node ) );
            level->SetAttribute( "screenRelativeHeight", scene.lodGroups[ g ].levels[ l ].screenRelativeHeight );
        }
    }

    CORE::CDataNode* prefabs = sidecar.AddChild( "prefabs", GUCEF_DATATYPE_ARRAY );
    for ( size_t p=0; p<scene.prefabs.size(); ++p )
    {
        CORE::CDataNode* prefab = prefabs->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        prefab->SetAttribute( "name", scene.prefabs[ p ].name );
        prefab->SetAttribute( "root", NodeName( scene, (Int32) scene.prefabs[ p ].rootNode ) );
        CORE::CDataNode* instances = prefab->AddChild( "instances", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<scene.prefabs[ p ].instances.size(); ++i )
        {
            const CAssetPrefabInstance& instance = scene.prefabs[ p ].instances[ i ];
            CORE::CDataNode* node = instances->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            node->SetAttribute( "node", NodeName( scene, (Int32) instance.node ) );
            node->SetAttribute( "sourcePrefab", instance.sourcePrefab );
            CORE::CDataNode* overrides = node->AddChild( instance.overrides );
            overrides->SetName( "overrides" );
        }
    }

    if ( !scene.particleSystems.empty() )
    {
        // the node each system is attached to: the first node that names it
        GUCEF::vector< Int32 > attachedTo( scene.particleSystems.size(), -1 );
        for ( size_t n=0; n<scene.nodes.size(); ++n )
        {
            const Int32 system = scene.nodes[ n ].particleSystem;
            if ( system >= 0 && (size_t) system < attachedTo.size() && attachedTo[ system ] < 0 )
                attachedTo[ system ] = (Int32) n;
        }
        CORE::CDataNode* particles = sidecar.AddChild( "particles", GUCEF_DATATYPE_ARRAY );
        for ( size_t p=0; p<scene.particleSystems.size(); ++p )
        {
            CORE::CDataNode* entry = particles->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            entry->SetAttribute( "node", NodeName( scene, attachedTo[ p ] ) );
            entry->SetAttribute( "nodeIndex", attachedTo[ p ] );
            const Int32 material = scene.particleSystems[ p ].renderer.material;
            if ( material >= 0 && (size_t) material < scene.materials.size() )
                entry->SetAttribute( "materialName", scene.materials[ material ].name );
            scene.particleSystems[ p ].Serialize( *entry );
        }
    }

    CORE::CDataNode* extras = sidecar.AddChild( scene.extras );
    extras->SetName( "extras" );

    CORE::CDataNode* lossy = sidecar.AddChild( "lossySteps", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<entry.items.size(); ++i )
    {
        if ( REPORTITEM_APPROXIMATION == entry.items[ i ].kind || REPORTITEM_SKIPPED == entry.items[ i ].kind )
            entry.items[ i ].Serialize( *lossy->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }

    CAssetSceneSummary summary;
    summary.Compute( scene );
    summary.Serialize( *sidecar.AddChild( "summary", GUCEF_DATATYPE_OBJECT ) );

    CORE::CString::StringVector dependencies;
    for ( size_t i=0; i<scene.dependencies.size(); ++i )
        dependencies.push_back( InPack( context, scene.dependencies[ i ] ) );
    sidecar.SetValuesOfChildByName( "dependencies", dependencies );
}

/*-------------------------------------------------------------------------*/

bool
CConversionSidecar::ReadParticleSystems( const CORE::CDataNode& sidecar                ,
                                         GUCEF::vector< CAssetParticleSystem >& systems ,
                                         GUCEF::vector< Int32 >& nodes                  )
{GUCEF_TRACE;

    systems.clear();
    nodes.clear();
    const CORE::CDataNode* list = sidecar.FindChild( "particles" );
    if ( GUCEF_NULL == list )
        return true;
    bool ok = true;
    for ( const CORE::CDataNode* entry = list->GetFirstChild(); GUCEF_NULL != entry; entry = entry->GetNext() )
    {
        CAssetParticleSystem system;
        if ( !system.Deserialize( *entry ) )
            ok = false;
        systems.push_back( system );
        nodes.push_back( entry->GetAttributeValueOrChildValueByName( "nodeIndex" ).AsInt32( -1 ) );
    }
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
