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

#ifndef GUCEF_ASSETCOOKER_OGRE_CCOLLISIONWRITER_H
#define GUCEF_ASSETCOOKER_OGRE_CCOLLISIONWRITER_H

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
 *  The colliders of an asset as <asset>.collision.json, for a later physics plugin, written with the deterministic
 *  JSON of the converter:
 *
 *      { "asset": "<asset path>", "scene": "<the DotScene of the asset>",
 *        "colliders": [ { "node": "<node name in that scene>", "nodePath": "<names of the nodes from the root, '/'>",
 *                         "name", "shape": "box|sphere|capsule|cylinder|convex|trimesh", "trigger",
 *                         "translation": [x, y, z], "rotation": [x, y, z, w], "scale": [x, y, z],
 *                         "halfExtents": [x, y, z] | "radius" | "radius" and "height",
 *                         "mesh": { "vertices": [x, y, z, ...], "indices": [a, b, c, ...] } }, ... ] }
 *
 *  Shapes are in the space of their node, placed by their own transform; the mesh of a convex or trimesh shape is in the
 *  space of the shape, its submeshes joined.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CCollisionWriter
{
    public:

    typedef GUCEF::map< UInt32, CORE::CString > TNameMap;

    /** The JSON tree of the colliders of the scene; false when it has none */
    static bool Build( const ASSETCONVERTER::CAssetScene& scene      ,
                       const CORE::CString& assetPath                ,
                       const CORE::CString& sceneName                ,
                       const TNameMap& nodeNames                     ,
                       CORE::CDataNode& document                     );

    /** The number of colliders of the scene */
    static UInt32 CountColliders( const ASSETCONVERTER::CAssetScene& scene );

    private:

    CCollisionWriter( void );
    ~CCollisionWriter();
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

#endif /* GUCEF_ASSETCOOKER_OGRE_CCOLLISIONWRITER_H ? */
