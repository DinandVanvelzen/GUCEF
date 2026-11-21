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

#ifndef GUCEF_ASSETCOOKER_OGRE_CBONESUBSETSPLITTER_H
#define GUCEF_ASSETCOOKER_OGRE_CBONESUBSETSPLITTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETMESH_H
#include "AssetConverterLib_CAssetMesh.h"
#define ASSETCONVERTERLIB_CASSETMESH_H
#endif /* ASSETCONVERTERLIB_CASSETMESH_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_MACROS_H
#include "assetcookerOGRE_macros.h"
#define GUCEF_ASSETCOOKER_OGRE_MACROS_H
#endif /* GUCEF_ASSETCOOKER_OGRE_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Splits the skinned submeshes of a mesh by bone subsets, so that a rig of any size works: a submesh of Ogre
 *  addresses its bones with 8 bits (its blend index to bone index map), so it can use 256 different bones at most,
 *  whatever the number of bones of the skeleton is.
 *
 *  The triangles of a submesh that uses more bones than allowed are dealt into parts, each part using at most the
 *  allowed number of bones (a triangle goes to the part that holds its bones, or has room for them; the triangles are
 *  taken in the order of the bone they are mostly weighted to, so triangles of the same limb end up together). A part
 *  is a submesh of its own: the vertices of its triangles, with all their streams, weights and morph target offsets,
 *  and the material of the source. A vertex used by triangles of two parts is in both. The joints keep the index of
 *  the skin: the mesh cooker gives the bones of a vertex by that index, the map of a submesh is made by Ogre.
 *
 *  The influences of a vertex are expected to be reduced to what the output keeps (see CMeshCooker::ReduceInfluences())
 *  before the split, as the bones that count are those that remain. A submesh within the limit is left as it is.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CBoneSubsetSplitter
{
    public:

    /**
     *  Splits every submesh of the mesh that uses more than 'maxBones' bones; the others are copied. 'maxBones' is at
     *  least 12 (a triangle can use that many bones); a smaller value is taken as 12.
     *
     *  @param extraSubMeshes the number of submeshes the output has more than the input
     *  @return the number of submeshes that were split
     */
    static UInt32 Split( const ASSETCONVERTER::CAssetMesh& mesh, UInt32 maxBones, ASSETCONVERTER::CAssetMesh& split, UInt32& extraSubMeshes );

    /** Splits one submesh; a submesh within the limit comes out as the only part */
    static void SplitSubMesh( const ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 maxBones, ASSETCONVERTER::CAssetMesh::TSubMeshVector& parts );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_CBONESUBSETSPLITTER_H ? */
