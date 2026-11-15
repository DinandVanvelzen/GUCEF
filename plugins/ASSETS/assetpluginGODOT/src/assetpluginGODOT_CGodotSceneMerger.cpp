/*
 *  assetpluginGODOT: adds the scene of a model file to a scene
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

#include "assetpluginGODOT_CGodotSceneMerger.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static Int32
Move( Int32 index, UInt32 base )
{
    return index >= 0 ? index + (Int32) base : index;
}

/*-------------------------------------------------------------------------*/

UInt32
CGodotSceneMerger::Append( ASSETCONVERTER::CAssetScene& target, const ASSETCONVERTER::CAssetScene& source, Int32 parent, SBases& bases, TUInt32Vector& newRoots )
{GUCEF_TRACE;

    newRoots.clear();

    // what is shared by the instances of a file
    if ( !bases.added )
    {
        bases.added = true;
        bases.meshBase = (UInt32) target.meshes.size();
        bases.materialBase = (UInt32) target.materials.size();
        bases.textureBase = (UInt32) target.textures.size();
        bases.lightBase = (UInt32) target.lights.size();

        for ( size_t t=0; t<source.textures.size(); ++t )
            target.textures.push_back( source.textures[ t ] );
        for ( size_t m=0; m<source.materials.size(); ++m )
        {
            target.materials.push_back( source.materials[ m ] );
            GUCEF::vector< ASSETCONVERTER::CAssetTextureReference* > references;
            target.materials.back().GetTextureReferences( references );
            for ( size_t r=0; r<references.size(); ++r )
                if ( references[ r ]->IsUsed() )
                    references[ r ]->textureIndex += (Int32) bases.textureBase;
        }
        for ( size_t m=0; m<source.meshes.size(); ++m )
        {
            target.meshes.push_back( source.meshes[ m ] );
            for ( size_t s=0; s<target.meshes.back().subMeshes.size(); ++s )
                target.meshes.back().subMeshes[ s ].materialIndex = Move( target.meshes.back().subMeshes[ s ].materialIndex, bases.materialBase );
        }
        for ( size_t l=0; l<source.lights.size(); ++l )
            target.lights.push_back( source.lights[ l ] );
        for ( size_t d=0; d<source.dependencies.size(); ++d )
            target.AddDependency( source.dependencies[ d ] );
    }

    // the nodes
    const UInt32 nodeBase = (UInt32) target.nodes.size();
    const UInt32 skinBase = (UInt32) target.skins.size();
    const UInt32 lodBase = (UInt32) target.lodGroups.size();
    for ( size_t n=0; n<source.nodes.size(); ++n )
    {
        ASSETCONVERTER::CAssetNode node = source.nodes[ n ];
        node.parent = node.parent >= 0 ? Move( node.parent, nodeBase ) : parent;
        for ( size_t c=0; c<node.children.size(); ++c )
            node.children[ c ] += nodeBase;
        node.mesh = Move( node.mesh, bases.meshBase );
        node.skin = Move( node.skin, skinBase );
        node.lodGroup = Move( node.lodGroup, lodBase );
        node.light = Move( node.light, bases.lightBase );
        node.particleSystem = -1;     // a model file has no particle systems: an index of its extras names nothing here
        for ( size_t c=0; c<node.colliders.size(); ++c )
            node.colliders[ c ].meshIndex = Move( node.colliders[ c ].meshIndex, bases.meshBase );
        target.nodes.push_back( node );
    }
    for ( size_t r=0; r<source.rootNodes.size(); ++r )
    {
        const UInt32 root = source.rootNodes[ r ] + nodeBase;
        newRoots.push_back( root );
        if ( parent >= 0 && (size_t) parent < target.nodes.size() )
            target.nodes[ parent ].children.push_back( root );
        else
            target.rootNodes.push_back( root );
    }

    for ( size_t s=0; s<source.skins.size(); ++s )
    {
        ASSETCONVERTER::CAssetSkin skin = source.skins[ s ];
        for ( size_t j=0; j<skin.joints.size(); ++j )
            skin.joints[ j ] += nodeBase;
        skin.skeletonRoot = Move( skin.skeletonRoot, nodeBase );
        target.skins.push_back( skin );
    }
    for ( size_t g=0; g<source.lodGroups.size(); ++g )
    {
        ASSETCONVERTER::CAssetLodGroup group = source.lodGroups[ g ];
        for ( size_t l=0; l<group.levels.size(); ++l )
            group.levels[ l ].node += nodeBase;
        target.lodGroups.push_back( group );
    }
    for ( size_t a=0; a<source.animations.size(); ++a )
    {
        ASSETCONVERTER::CAssetAnimation animation = source.animations[ a ];
        for ( size_t c=0; c<animation.channels.size(); ++c )
            animation.channels[ c ].targetNode += nodeBase;
        target.animations.push_back( animation );
    }
    return nodeBase;
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
