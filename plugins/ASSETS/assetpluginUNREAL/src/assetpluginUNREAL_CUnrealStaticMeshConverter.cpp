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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHCONVERTER_H
#include "assetpluginUNREAL_CUnrealStaticMeshConverter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHCONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHCONVERTER_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "unreal";

/*-------------------------------------------------------------------------*/

MATH::CCoordinateSystem
CUnrealStaticMeshConverter::GetCoordinateSystem( void )
{GUCEF_TRACE;

    // The models of the packs in the owned corpus (the POLYGON Shops project) are the same models as the Godot and FBX forms of the pack, and
    // coincide with those after the swap of Y and Z: X to the right, Z up, the front of a model toward -Y. This is the form in which the
    // engine receives models that were modelled with a +Z front (the Unity and Godot habit), not the +X forward axis of the engine itself.
    return MATH::CCoordinateSystem( MATH::SIGNEDAXIS_POSITIVE_X, MATH::SIGNEDAXIS_POSITIVE_Z, MATH::SIGNEDAXIS_NEGATIVE_Y, 100.0 );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealStaticMeshConverter::TurnsTriangles( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealStaticMeshConverter::AddToScene( const CUnrealStaticMesh& mesh                  ,
                                        const GUCEF::vector< Int32 >& slotMaterials    ,
                                        ASSETCONVERTER::CAssetScene& scene             ,
                                        ASSETCONVERTER::CConversionReport& report      ,
                                        UInt32& rootNode                               )
{GUCEF_TRACE;

    if ( mesh.lods.empty() )
    {
        report.AddError( SourceName, mesh.name, "The static mesh has no levels of detail" );
        return false;
    }
    const UInt32 holder = scene.AddNode( mesh.name );
    rootNode = holder;
    ASSETCONVERTER::CAssetLodGroup group;
    group.name = mesh.name;
    bool anyScreenSize = false;
    for ( size_t l=0; l<mesh.lods.size(); ++l )
    {
        const CUnrealStaticMesh::SLod& lod = mesh.lods[ l ];
        ASSETCONVERTER::CAssetMesh assetMesh;
        assetMesh.name = mesh.lods.size() > 1 ? mesh.name + "_LOD" + CORE::UInt32ToString( (UInt32) l ) : mesh.name;
        for ( size_t s=0; s<lod.sections.size(); ++s )
        {
            const CUnrealStaticMesh::SSection& section = lod.sections[ s ];
            if ( section.indices.empty() )
                continue;
            ASSETCONVERTER::CAssetSubMesh subMesh;
            subMesh.materialIndex = section.slot >= 0 && (size_t) section.slot < slotMaterials.size() ? slotMaterials[ (size_t) section.slot ] : -1;
            subMesh.positions = section.positions;
            subMesh.normals = section.normals;
            subMesh.tangents = section.tangents;
            subMesh.colours = section.colours;
            for ( UInt32 u=0; u<lod.uvSetCount && u<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++u )
                subMesh.uvSets[ u ] = section.uvSets[ u ];
            subMesh.indices = section.indices;
            if ( TurnsTriangles() )
                subMesh.ReverseWinding();
            assetMesh.subMeshes.push_back( subMesh );
        }
        scene.meshes.push_back( assetMesh );
        const UInt32 meshNode = scene.AddNode( assetMesh.name, (Int32) holder );
        scene.nodes[ meshNode ].mesh = (Int32) scene.meshes.size() - 1;
        group.AddLevel( meshNode, lod.screenSize > 0.0f ? lod.screenSize : -1.0f );
        anyScreenSize = anyScreenSize || lod.screenSize > 0.0f;
    }
    if ( mesh.lods.size() > 1 )
    {
        scene.lodGroups.push_back( group );
        scene.nodes[ holder ].lodGroup = (Int32) scene.lodGroups.size() - 1;
        if ( !anyScreenSize )
            report.AddApproximation( SourceName, mesh.name, "the mesh states no screen sizes for its LODs: the levels are in the order of the LODs" );
    }
    report.AddInfo( SourceName, mesh.name, "Static mesh with " + CORE::UInt64ToString( mesh.lods.size() ) + " LODs, " + CORE::UInt32ToString( mesh.GetVertexCount( 0 ) ) + " vertices and " + CORE::UInt32ToString( mesh.GetTriangleCount( 0 ) ) + " triangles in the first LOD" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
