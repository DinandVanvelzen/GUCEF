/*
 *  AssetConverterLib: a node of the hierarchy of the intermediate model
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

#ifndef ASSETCONVERTERLIB_CASSETNODE_H
#define ASSETCONVERTERLIB_CASSETNODE_H

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

#ifndef ASSETCONVERTERLIB_CASSETCOLLIDER_H
#include "AssetConverterLib_CAssetCollider.h"
#define ASSETCONVERTERLIB_CASSETCOLLIDER_H
#endif /* ASSETCONVERTERLIB_CASSETCOLLIDER_H ? */


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
 *  A node of the hierarchy: its name, its transform relative to the parent, its children, and what it holds by index
 *  into the lists of the scene (-1 for nothing): a mesh, the skin of that mesh, a LOD group, a light, a particle system. It also holds
 *  its colliders, the asset path of the prefab it instances, if any, and extras of the source.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetNode
{
    public:

    typedef GUCEF::vector< UInt32 > TUInt32Vector;
    typedef GUCEF::vector< CAssetCollider > TColliderVector;

    CORE::CString name;
    MATH::TFloat32Transform3D transform;
    Int32 parent;
    TUInt32Vector children;
    Int32 mesh;
    Int32 skin;
    Int32 lodGroup;
    Int32 light;
    Int32 particleSystem;
    TColliderVector colliders;
    CORE::CString prefabInstance;
    CORE::CDataNode extras;

    CAssetNode( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETNODE_H ? */
