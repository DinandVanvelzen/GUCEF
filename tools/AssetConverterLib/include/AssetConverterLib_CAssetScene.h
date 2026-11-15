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

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#define ASSETCONVERTERLIB_CASSETSCENE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPROVENANCE_H
#include "AssetConverterLib_CAssetProvenance.h"
#define ASSETCONVERTERLIB_CASSETPROVENANCE_H
#endif /* ASSETCONVERTERLIB_CASSETPROVENANCE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETNODE_H
#include "AssetConverterLib_CAssetNode.h"
#define ASSETCONVERTERLIB_CASSETNODE_H
#endif /* ASSETCONVERTERLIB_CASSETNODE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETMESH_H
#include "AssetConverterLib_CAssetMesh.h"
#define ASSETCONVERTERLIB_CASSETMESH_H
#endif /* ASSETCONVERTERLIB_CASSETMESH_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSKIN_H
#include "AssetConverterLib_CAssetSkin.h"
#define ASSETCONVERTERLIB_CASSETSKIN_H
#endif /* ASSETCONVERTERLIB_CASSETSKIN_H ? */

#ifndef ASSETCONVERTERLIB_CASSETANIMATION_H
#include "AssetConverterLib_CAssetAnimation.h"
#define ASSETCONVERTERLIB_CASSETANIMATION_H
#endif /* ASSETCONVERTERLIB_CASSETANIMATION_H ? */

#ifndef ASSETCONVERTERLIB_CASSETMATERIAL_H
#include "AssetConverterLib_CAssetMaterial.h"
#define ASSETCONVERTERLIB_CASSETMATERIAL_H
#endif /* ASSETCONVERTERLIB_CASSETMATERIAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETTEXTURE_H
#include "AssetConverterLib_CAssetTexture.h"
#define ASSETCONVERTERLIB_CASSETTEXTURE_H
#endif /* ASSETCONVERTERLIB_CASSETTEXTURE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETLIGHT_H
#include "AssetConverterLib_CAssetLight.h"
#define ASSETCONVERTERLIB_CASSETLIGHT_H
#endif /* ASSETCONVERTERLIB_CASSETLIGHT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETLODGROUP_H
#include "AssetConverterLib_CAssetLodGroup.h"
#define ASSETCONVERTERLIB_CASSETLODGROUP_H
#endif /* ASSETCONVERTERLIB_CASSETLODGROUP_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPREFAB_H
#include "AssetConverterLib_CAssetPrefab.h"
#define ASSETCONVERTERLIB_CASSETPREFAB_H
#endif /* ASSETCONVERTERLIB_CASSETPREFAB_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPARTICLESYSTEM_H
#include "AssetConverterLib_CAssetParticleSystem.h"
#define ASSETCONVERTERLIB_CASSETPARTICLESYSTEM_H
#endif /* ASSETCONVERTERLIB_CASSETPARTICLESYSTEM_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One asset in the engine-neutral model: what an importer reads from a source, what the normalisation and the
 *  post-processors work on and what an exporter writes as the intermediate (glTF plus the sidecar).
 *
 *  Everything refers to everything else by index into the lists here. The importer states the conventions of the
 *  source: the coordinate system (axes and units) and the normal map convention; the normalisation (CAssetNormaliser)
 *  brings them to those of glTF and records what it did in 'normalisation'. 'settings' are the effective settings of
 *  the asset (importer defaults, pack rules, presets and the per-asset override) which the pipeline sets before the
 *  import; 'dependencies' are the VFS paths of the files the import read besides the source (textures, .meta files,
 *  referenced materials, nested prefabs), which the importer adds and which become part of the cache key.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetScene
{
    public:

    typedef GUCEF::vector< UInt32 > TUInt32Vector;
    typedef GUCEF::set< UInt32 > TUInt32Set;
    typedef GUCEF::vector< CAssetNode > TNodeVector;
    typedef GUCEF::vector< CAssetMesh > TMeshVector;
    typedef GUCEF::vector< CAssetSkin > TSkinVector;
    typedef GUCEF::vector< CAssetAnimation > TAnimationVector;
    typedef GUCEF::vector< CAssetMaterial > TMaterialVector;
    typedef GUCEF::vector< CAssetTexture > TTextureVector;
    typedef GUCEF::vector< CAssetLight > TLightVector;
    typedef GUCEF::vector< CAssetLodGroup > TLodGroupVector;
    typedef GUCEF::vector< CAssetPrefab > TPrefabVector;
    typedef GUCEF::vector< CAssetParticleSystem > TParticleSystemVector;

    CORE::CString name;
    CAssetProvenance provenance;
    MATH::CCoordinateSystem coordinateSystem;       /**< of the source until the normalisation, then GUCEF's (glTF's) */
    TNormalMapConvention normalMapConvention;
    TNodeVector nodes;
    TUInt32Vector rootNodes;
    TMeshVector meshes;
    TSkinVector skins;
    TAnimationVector animations;
    TMaterialVector materials;
    TTextureVector textures;
    TLightVector lights;
    TLodGroupVector lodGroups;
    TPrefabVector prefabs;
    TParticleSystemVector particleSystems;     /**< attached to nodes by CAssetNode::particleSystem */
    CORE::CString::StringVector dependencies;
    CORE::CDataNode settings;
    CORE::CDataNode extras;
    CORE::CDataNode normalisation;

    CAssetScene( void );

    /** Adds a node under the parent (a root node for -1); its index */
    UInt32 AddNode( const CORE::CString& nodeName, Int32 parentNode = -1 );

    /** The index of the first node with the name, -1 when there is none */
    Int32 FindNode( const CORE::CString& nodeName ) const;

    /** The transform of the node relative to the scene: the transforms of its parents and its own */
    MATH::TFloat32Matrix4D GetWorldMatrix( UInt32 node ) const;

    /** Adds a dependency of the import (a VFS path), once */
    void AddDependency( const CORE::CString& vfsPath );

    bool IsJoint( UInt32 node ) const;
    bool IsAnimationTarget( UInt32 node ) const;
    bool IsLodLevel( UInt32 node ) const;
    bool IsPrefabNode( UInt32 node ) const;

    /** Whether 'node' is 'ancestor' or below it */
    bool IsInSubtree( UInt32 node, UInt32 ancestor ) const;

    /**
     *  Removes the nodes and everything below them. Every reference to a node is updated: parents, children, the root
     *  nodes, the joints and roots of skins (a removed joint leaves its skin), animation channels (the channels of a
     *  removed node go), LOD levels and prefabs (those of a removed node go). The number of nodes removed.
     */
    UInt32 RemoveNodes( const TUInt32Set& nodesToRemove );

    /** Removes the meshes; nodes, colliders and particle systems that used them use none. The number removed */
    UInt32 RemoveMeshes( const TUInt32Set& meshesToRemove );

    /** Removes meshes that no node and no collider uses; the number removed */
    UInt32 RemoveUnusedMeshes( void );

    /** Removes the materials; submeshes and particle systems that used them use none. The number removed */
    UInt32 RemoveMaterials( const TUInt32Set& materialsToRemove );

    /** Removes materials that no submesh and no particle system uses; the number removed */
    UInt32 RemoveUnusedMaterials( void );

    /** Whether a node, a collider or a particle system uses the mesh */
    bool IsMeshUsed( UInt32 mesh ) const;

    /** Removes textures that no material uses; the number removed */
    UInt32 RemoveUnusedTextures( void );

    /** Changes the material of every submesh and particle system that uses 'from' to 'to' */
    void ReplaceMaterial( UInt32 from, UInt32 to );

    UInt32 GetTriangleCount( void ) const;
    UInt32 GetVertexCount( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */
