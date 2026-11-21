/*
 *  AssetConverterLib: an asset in the engine-neutral intermediate model
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

#include "AssetConverterLib_CAssetScene.h"

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

/** The new index of every old index, -1 for a removed one */
typedef GUCEF::vector< Int32 > TIndexMap;

Int32
Remap( const TIndexMap& map, Int32 index )
{
    if ( index < 0 || (size_t) index >= map.size() )
        return -1;
    return map[ index ];
}

void
RemapList( const TIndexMap& map, GUCEF::vector< UInt32 >& list )
{
    GUCEF::vector< UInt32 > result;
    result.reserve( list.size() );
    for ( size_t i=0; i<list.size(); ++i )
    {
        const Int32 index = Remap( map, (Int32) list[ i ] );
        if ( index >= 0 )
            result.push_back( (UInt32) index );
    }
    list = result;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetScene::CAssetScene( void )
    : name()
    , provenance()
    , coordinateSystem( MATH::CCoordinateSystem::GucefDefault() )
    , normalMapConvention( NORMALMAPCONVENTION_OPENGL )
    , nodes()
    , rootNodes()
    , meshes()
    , skins()
    , animations()
    , materials()
    , textures()
    , lights()
    , lodGroups()
    , prefabs()
    , particleSystems()
    , hiddenFaceSets()
    , dependencies()
    , references()
    , settings( "settings", GUCEF_DATATYPE_OBJECT )
    , extras( "extras", GUCEF_DATATYPE_OBJECT )
    , normalisation( "normalisation", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::AddNode( const CORE::CString& nodeName, Int32 parentNode )
{GUCEF_TRACE;

    const UInt32 index = (UInt32) nodes.size();
    CAssetNode node;
    node.name = nodeName;
    if ( parentNode >= 0 && (size_t) parentNode < nodes.size() )
    {
        node.parent = parentNode;
        nodes.push_back( node );
        nodes[ parentNode ].children.push_back( index );
    }
    else
    {
        nodes.push_back( node );
        rootNodes.push_back( index );
    }
    return index;
}

/*-------------------------------------------------------------------------*/

Int32
CAssetScene::FindNode( const CORE::CString& nodeName ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<nodes.size(); ++i )
        if ( nodes[ i ].name == nodeName )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

MATH::TFloat32Matrix4D
CAssetScene::GetWorldMatrix( UInt32 node ) const
{GUCEF_TRACE;

    MATH::TFloat32Matrix4D world = MATH::TFloat32Matrix4D::Identity();
    Int32 current = (Int32) node;
    UInt32 guard = 0;
    while ( current >= 0 && (size_t) current < nodes.size() && guard <= nodes.size() )
    {
        world = nodes[ current ].transform.ToMatrix() * world;
        current = nodes[ current ].parent;
        ++guard;
    }
    return world;
}

/*-------------------------------------------------------------------------*/

void
CAssetScene::AddDependency( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    for ( size_t i=0; i<dependencies.size(); ++i )
        if ( dependencies[ i ] == vfsPath )
            return;
    dependencies.push_back( vfsPath );
}

/*-------------------------------------------------------------------------*/

void
CAssetScene::AddReference( const CORE::CString& kind, const CORE::CString& vfsPath, const CORE::CString& via, bool optional )
{GUCEF_TRACE;

    if ( vfsPath.IsNULLOrEmpty() )
    {
        AddUnresolvedReference( kind, via, optional );
        return;
    }
    const CAssetReference reference( kind, vfsPath, via, optional );
    for ( size_t i=0; i<references.size(); ++i )
        if ( references[ i ] == reference )
            return;
    references.push_back( reference );
}

/*-------------------------------------------------------------------------*/

void
CAssetScene::AddUnresolvedReference( const CORE::CString& kind, const CORE::CString& via, bool optional )
{GUCEF_TRACE;

    if ( via.IsNULLOrEmpty() )
        return;
    const CAssetReference reference( kind, CORE::CString::Empty, via, optional );
    for ( size_t i=0; i<references.size(); ++i )
        if ( references[ i ] == reference )
            return;
    references.push_back( reference );
}

/*-------------------------------------------------------------------------*/

void
CAssetScene::MergeDependenciesOf( const CAssetScene& nested )
{GUCEF_TRACE;

    for ( size_t d=0; d<nested.dependencies.size(); ++d )
        AddDependency( nested.dependencies[ d ] );
    for ( size_t r=0; r<nested.references.size(); ++r )
    {
        const CAssetReference& reference = nested.references[ r ];
        if ( reference.IsResolved() )
            AddReference( reference.kind, reference.target, reference.via, reference.optional );
        else
            AddUnresolvedReference( reference.kind, reference.via, reference.optional );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetScene::IsJoint( UInt32 node ) const
{GUCEF_TRACE;

    for ( size_t s=0; s<skins.size(); ++s )
    {
        if ( skins[ s ].skeletonRoot == (Int32) node )
            return true;
        for ( size_t j=0; j<skins[ s ].joints.size(); ++j )
            if ( skins[ s ].joints[ j ] == node )
                return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetScene::IsAnimationTarget( UInt32 node ) const
{GUCEF_TRACE;

    for ( size_t a=0; a<animations.size(); ++a )
        for ( size_t c=0; c<animations[ a ].channels.size(); ++c )
            if ( animations[ a ].channels[ c ].targetNode == node )
                return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetScene::IsLodLevel( UInt32 node ) const
{GUCEF_TRACE;

    for ( size_t g=0; g<lodGroups.size(); ++g )
        for ( size_t l=0; l<lodGroups[ g ].levels.size(); ++l )
            if ( lodGroups[ g ].levels[ l ].node == node )
                return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetScene::IsPrefabNode( UInt32 node ) const
{GUCEF_TRACE;

    for ( size_t p=0; p<prefabs.size(); ++p )
    {
        if ( prefabs[ p ].rootNode == node )
            return true;
        for ( size_t i=0; i<prefabs[ p ].instances.size(); ++i )
            if ( prefabs[ p ].instances[ i ].node == node )
                return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetScene::IsInSubtree( UInt32 node, UInt32 ancestor ) const
{GUCEF_TRACE;

    Int32 current = (Int32) node;
    UInt32 guard = 0;
    while ( current >= 0 && (size_t) current < nodes.size() && guard <= nodes.size() )
    {
        if ( (UInt32) current == ancestor )
            return true;
        current = nodes[ current ].parent;
        ++guard;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::RemoveNodes( const TUInt32Set& nodesToRemove )
{GUCEF_TRACE;

    if ( nodesToRemove.empty() )
        return 0;

    // the nodes to remove with everything below them
    GUCEF::vector< bool > removed( nodes.size(), false );
    for ( size_t n=0; n<nodes.size(); ++n )
    {
        TUInt32Set::const_iterator i = nodesToRemove.begin();
        while ( i != nodesToRemove.end() )
        {
            if ( IsInSubtree( (UInt32) n, *i ) )
            {
                removed[ n ] = true;
                break;
            }
            ++i;
        }
    }

    TIndexMap map( nodes.size(), -1 );
    TNodeVector kept;
    for ( size_t n=0; n<nodes.size(); ++n )
    {
        if ( removed[ n ] )
            continue;
        map[ n ] = (Int32) kept.size();
        kept.push_back( nodes[ n ] );
    }
    const UInt32 removedCount = (UInt32) ( nodes.size() - kept.size() );
    if ( 0 == removedCount )
        return 0;

    for ( size_t n=0; n<kept.size(); ++n )
    {
        kept[ n ].parent = Remap( map, kept[ n ].parent );
        RemapList( map, kept[ n ].children );
    }
    nodes = kept;
    RemapList( map, rootNodes );

    for ( size_t s=0; s<skins.size(); ++s )
    {
        // the joints keep their order; a removed joint leaves the skin together with its bind matrix
        CAssetSkin& skin = skins[ s ];
        CAssetSkin::TUInt32Vector joints;
        CAssetSkin::TMatrixVector matrices;
        for ( size_t j=0; j<skin.joints.size(); ++j )
        {
            const Int32 index = Remap( map, (Int32) skin.joints[ j ] );
            if ( index < 0 )
                continue;
            joints.push_back( (UInt32) index );
            if ( j < skin.inverseBindMatrices.size() )
                matrices.push_back( skin.inverseBindMatrices[ j ] );
        }
        skin.joints = joints;
        skin.inverseBindMatrices = matrices;
        skin.skeletonRoot = Remap( map, skin.skeletonRoot );
    }

    for ( size_t a=0; a<animations.size(); ++a )
    {
        CAssetAnimation::TChannelVector channels;
        for ( size_t c=0; c<animations[ a ].channels.size(); ++c )
        {
            const Int32 index = Remap( map, (Int32) animations[ a ].channels[ c ].targetNode );
            if ( index < 0 )
                continue;
            channels.push_back( animations[ a ].channels[ c ] );
            channels.back().targetNode = (UInt32) index;
        }
        animations[ a ].channels = channels;
    }

    for ( size_t g=0; g<lodGroups.size(); ++g )
    {
        CAssetLodGroup::TLevelVector levels;
        for ( size_t l=0; l<lodGroups[ g ].levels.size(); ++l )
        {
            const Int32 index = Remap( map, (Int32) lodGroups[ g ].levels[ l ].node );
            if ( index < 0 )
                continue;
            levels.push_back( lodGroups[ g ].levels[ l ] );
            levels.back().node = (UInt32) index;
        }
        lodGroups[ g ].levels = levels;
    }

    TPrefabVector prefabsKept;
    for ( size_t p=0; p<prefabs.size(); ++p )
    {
        const Int32 root = Remap( map, (Int32) prefabs[ p ].rootNode );
        if ( root < 0 )
            continue;
        CAssetPrefab prefab = prefabs[ p ];
        prefab.rootNode = (UInt32) root;
        prefab.instances.clear();
        for ( size_t i=0; i<prefabs[ p ].instances.size(); ++i )
        {
            const Int32 node = Remap( map, (Int32) prefabs[ p ].instances[ i ].node );
            if ( node < 0 )
                continue;
            prefab.instances.push_back( prefabs[ p ].instances[ i ] );
            prefab.instances.back().node = (UInt32) node;
        }
        prefabsKept.push_back( prefab );
    }
    prefabs = prefabsKept;
    return removedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::RemoveMeshes( const TUInt32Set& meshesToRemove )
{GUCEF_TRACE;

    TIndexMap map( meshes.size(), -1 );
    TMeshVector kept;
    for ( size_t m=0; m<meshes.size(); ++m )
    {
        if ( meshesToRemove.find( (UInt32) m ) != meshesToRemove.end() )
            continue;
        map[ m ] = (Int32) kept.size();
        kept.push_back( meshes[ m ] );
    }
    const UInt32 removedCount = (UInt32) ( meshes.size() - kept.size() );
    if ( 0 == removedCount )
        return 0;
    meshes = kept;
    for ( size_t n=0; n<nodes.size(); ++n )
    {
        nodes[ n ].mesh = Remap( map, nodes[ n ].mesh );
        for ( size_t c=0; c<nodes[ n ].colliders.size(); ++c )
            nodes[ n ].colliders[ c ].meshIndex = Remap( map, nodes[ n ].colliders[ c ].meshIndex );
    }
    for ( size_t p=0; p<particleSystems.size(); ++p )
    {
        particleSystems[ p ].renderer.mesh = Remap( map, particleSystems[ p ].renderer.mesh );
        particleSystems[ p ].shape.mesh = Remap( map, particleSystems[ p ].shape.mesh );
    }
    return removedCount;
}

/*-------------------------------------------------------------------------*/

bool
CAssetScene::IsMeshUsed( UInt32 mesh ) const
{GUCEF_TRACE;

    for ( size_t n=0; n<nodes.size(); ++n )
    {
        if ( nodes[ n ].mesh == (Int32) mesh )
            return true;
        for ( size_t c=0; c<nodes[ n ].colliders.size(); ++c )
            if ( nodes[ n ].colliders[ c ].meshIndex == (Int32) mesh )
                return true;
    }
    for ( size_t p=0; p<particleSystems.size(); ++p )
        if ( particleSystems[ p ].renderer.mesh == (Int32) mesh || particleSystems[ p ].shape.mesh == (Int32) mesh )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::RemoveUnusedMeshes( void )
{GUCEF_TRACE;

    TUInt32Set unused;
    for ( size_t m=0; m<meshes.size(); ++m )
        if ( !IsMeshUsed( (UInt32) m ) )
            unused.insert( (UInt32) m );
    return RemoveMeshes( unused );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::RemoveMaterials( const TUInt32Set& materialsToRemove )
{GUCEF_TRACE;

    TIndexMap map( materials.size(), -1 );
    TMaterialVector kept;
    for ( size_t i=0; i<materials.size(); ++i )
    {
        if ( materialsToRemove.find( (UInt32) i ) != materialsToRemove.end() )
            continue;
        map[ i ] = (Int32) kept.size();
        kept.push_back( materials[ i ] );
    }
    const UInt32 removedCount = (UInt32) ( materials.size() - kept.size() );
    if ( 0 == removedCount )
        return 0;
    materials = kept;
    for ( size_t m=0; m<meshes.size(); ++m )
        for ( size_t s=0; s<meshes[ m ].subMeshes.size(); ++s )
            meshes[ m ].subMeshes[ s ].materialIndex = Remap( map, meshes[ m ].subMeshes[ s ].materialIndex );
    for ( size_t p=0; p<particleSystems.size(); ++p )
    {
        particleSystems[ p ].renderer.material = Remap( map, particleSystems[ p ].renderer.material );
        particleSystems[ p ].renderer.trailMaterial = Remap( map, particleSystems[ p ].renderer.trailMaterial );
    }
    return removedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::RemoveUnusedMaterials( void )
{GUCEF_TRACE;

    GUCEF::vector< bool > used( materials.size(), false );
    for ( size_t m=0; m<meshes.size(); ++m )
        for ( size_t s=0; s<meshes[ m ].subMeshes.size(); ++s )
        {
            const Int32 material = meshes[ m ].subMeshes[ s ].materialIndex;
            if ( material >= 0 && (size_t) material < materials.size() )
                used[ material ] = true;
        }
    for ( size_t p=0; p<particleSystems.size(); ++p )
    {
        const Int32 refs[ 2 ] = { particleSystems[ p ].renderer.material, particleSystems[ p ].renderer.trailMaterial };
        for ( UInt32 r=0; r<2; ++r )
            if ( refs[ r ] >= 0 && (size_t) refs[ r ] < materials.size() )
                used[ refs[ r ] ] = true;
    }
    TUInt32Set unused;
    for ( size_t i=0; i<materials.size(); ++i )
        if ( !used[ i ] )
            unused.insert( (UInt32) i );
    return RemoveMaterials( unused );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::RemoveUnusedTextures( void )
{GUCEF_TRACE;

    GUCEF::vector< bool > used( textures.size(), false );
    for ( size_t m=0; m<materials.size(); ++m )
    {
        GUCEF::vector< const CAssetTextureReference* > references;
        materials[ m ].GetTextureReferences( references );
        for ( size_t r=0; r<references.size(); ++r )
            if ( references[ r ]->IsUsed() && (size_t) references[ r ]->textureIndex < textures.size() )
                used[ references[ r ]->textureIndex ] = true;
    }
    TIndexMap map( textures.size(), -1 );
    TTextureVector kept;
    for ( size_t i=0; i<textures.size(); ++i )
    {
        if ( !used[ i ] )
            continue;
        map[ i ] = (Int32) kept.size();
        kept.push_back( textures[ i ] );
    }
    const UInt32 removedCount = (UInt32) ( textures.size() - kept.size() );
    if ( 0 == removedCount )
        return 0;
    textures = kept;
    for ( size_t m=0; m<materials.size(); ++m )
    {
        GUCEF::vector< CAssetTextureReference* > references;
        materials[ m ].GetTextureReferences( references );
        for ( size_t r=0; r<references.size(); ++r )
            if ( references[ r ]->IsUsed() )
                references[ r ]->textureIndex = Remap( map, references[ r ]->textureIndex );
    }
    return removedCount;
}

/*-------------------------------------------------------------------------*/

void
CAssetScene::ReplaceMaterial( UInt32 from, UInt32 to )
{GUCEF_TRACE;

    for ( size_t m=0; m<meshes.size(); ++m )
        for ( size_t s=0; s<meshes[ m ].subMeshes.size(); ++s )
            if ( meshes[ m ].subMeshes[ s ].materialIndex == (Int32) from )
                meshes[ m ].subMeshes[ s ].materialIndex = (Int32) to;
    for ( size_t p=0; p<particleSystems.size(); ++p )
    {
        if ( particleSystems[ p ].renderer.material == (Int32) from )
            particleSystems[ p ].renderer.material = (Int32) to;
        if ( particleSystems[ p ].renderer.trailMaterial == (Int32) from )
            particleSystems[ p ].renderer.trailMaterial = (Int32) to;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::GetTriangleCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t m=0; m<meshes.size(); ++m )
        count += meshes[ m ].GetTriangleCount();
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::GetVertexCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t m=0; m<meshes.size(); ++m )
        count += meshes[ m ].GetVertexCount();
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetScene::GetActiveHiddenFaceMask( void ) const
{GUCEF_TRACE;

    return CAssetHiddenFaceSet::GetActiveMask( hiddenFaceSets );
}

/*-------------------------------------------------------------------------*/

bool
CAssetScene::IsHiddenByActiveSet( const CAssetSubMesh& subMesh ) const
{GUCEF_TRACE;

    return 0 != ( subMesh.hiddenBy & GetActiveHiddenFaceMask() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
