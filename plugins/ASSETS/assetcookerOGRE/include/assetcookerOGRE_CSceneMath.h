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

#ifndef GUCEF_ASSETCOOKER_OGRE_CSCENEMATH_H
#define GUCEF_ASSETCOOKER_OGRE_CSCENEMATH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreMatrix4.h"
#include "OgreQuaternion.h"
#include "OgreVector.h"

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
 *  The transforms of the intermediate in Ogre's math: node transforms and matrices of the model as Ogre affine
 *  matrices, the transform of a node relative to the scene, and the split of an affine matrix into position,
 *  rotation and scale. The intermediate and Ogre share their conventions (right handed, +Y up, column vectors), so
 *  the values carry over as they are.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CSceneMath
{
    public:

    struct STrs
    {
        Ogre::Vector3 position;
        Ogre::Quaternion rotation;
        Ogre::Vector3 scale;

        STrs( void );
    };

    static Ogre::Affine3 ToAffine( const MATH::TFloat32Transform3D& transform );

    static Ogre::Affine3 ToAffine( const MATH::TFloat32Matrix4D& matrix );

    static Ogre::Affine3 ToAffine( const STrs& trs );

    static STrs ToTrs( const MATH::TFloat32Transform3D& transform );

    /** Position, rotation and scale of an affine matrix; shear is lost */
    static STrs Decompose( const Ogre::Affine3& matrix );

    /** The transform of the node relative to the scene: those of its parents and its own */
    static Ogre::Affine3 GetWorld( const ASSETCONVERTER::CAssetScene& scene, Int32 node );

    /** Whether two transforms are the same within a small tolerance */
    static bool IsNear( const STrs& a, const STrs& b );

    static bool IsIdentity( const STrs& trs );

    private:

    CSceneMath( void );
    ~CSceneMath();
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

#endif /* GUCEF_ASSETCOOKER_OGRE_CSCENEMATH_H ? */
