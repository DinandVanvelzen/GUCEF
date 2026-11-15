/*
 *  AssetConverterLib: a collider of the intermediate model, as an OMI physics shape
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

#ifndef ASSETCONVERTERLIB_CASSETCOLLIDER_H
#define ASSETCONVERTERLIB_CASSETCOLLIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MATH_TRANSFORMS_H
#include "gucefMATH_transforms.h"
#define GUCEF_MATH_TRANSFORMS_H
#endif /* GUCEF_MATH_TRANSFORMS_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */


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
 *  A collider of a node, as OMI_physics_shape: the shape and its parameters in the frame of 'transform', which is
 *  relative to the node and has no scale. A box has half extents, a sphere a radius, a capsule and a cylinder a radius
 *  and a height along their local Y (the height of a capsule includes its caps). Convex and trimesh shapes use the
 *  mesh 'meshIndex' of the scene, whose vertices are in the frame of the node (their transform stays identity). A
 *  trigger has no physical response. A value type.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetCollider
{
    public:

    CORE::CString name;
    TColliderShape shape;
    Float32 halfExtents[ 3 ];
    Float32 radius;
    Float32 height;
    MATH::TFloat32Transform3D transform;
    bool trigger;
    Int32 meshIndex;

    CAssetCollider( void );

    bool IsMeshShape( void ) const;

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

#endif /* ASSETCONVERTERLIB_CASSETCOLLIDER_H ? */
