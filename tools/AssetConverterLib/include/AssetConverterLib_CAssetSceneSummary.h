/*
 *  AssetConverterLib: the summary of an asset for the sidecar and the fidelity tests
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

#ifndef ASSETCONVERTERLIB_CASSETSCENESUMMARY_H
#define ASSETCONVERTERLIB_CASSETSCENESUMMARY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */


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
 *  The summary of an asset that the sidecar holds and the fidelity tests compare: the counts of everything, the bounds
 *  of the geometry as the nodes place it (bind pose), the bones and the length of every clip.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetSceneSummary
{
    public:

    struct SClip
    {
        CORE::CString name;
        Float32 duration;
        bool loop;
    };
    typedef GUCEF::vector< SClip > TClipVector;

    UInt32 nodeCount;
    UInt32 meshCount;
    UInt32 subMeshCount;
    UInt32 vertexCount;
    UInt32 triangleCount;
    UInt32 materialCount;
    UInt32 textureCount;
    UInt32 skinCount;
    UInt32 boneCount;               /**< the joints of all skins, each node counted once */
    UInt32 morphTargetCount;
    UInt32 colliderCount;
    UInt32 lodGroupCount;
    UInt32 lodLevelCount;
    UInt32 lightCount;
    UInt32 prefabCount;
    UInt32 particleSystemCount;     /**< written only when there are particle systems */
    bool hasBounds;
    Float32 boundsMin[ 3 ];
    Float32 boundsMax[ 3 ];
    TClipVector clips;

    CAssetSceneSummary( void );

    void Compute( const CAssetScene& scene );

    bool Serialize( CORE::CDataNode& node ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETSCENESUMMARY_H ? */
