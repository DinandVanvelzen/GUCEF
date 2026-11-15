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

#ifndef GUCEF_ASSETCOOKER_OGRE_CSKELETONCOOKER_H
#define GUCEF_ASSETCOOKER_OGRE_CSKELETONCOOKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreSkeleton.h"

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_CSCENEMATH_H
#include "assetcookerOGRE_CSceneMath.h"
#define GUCEF_ASSETCOOKER_OGRE_CSCENEMATH_H
#endif /* GUCEF_ASSETCOOKER_OGRE_CSCENEMATH_H ? */

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
 *  Builds the Ogre skeleton of a skin of the intermediate and writes it as .skeleton (SkeletonSerializer, the newest
 *  version of 14.6.3, little endian).
 *
 *  - Bones: one per joint of the skin, its handle the index of the joint in the skin, which is the index the joints of
 *    the vertices use; the parent of a bone is the nearest ancestor node that is a joint of the same skin.
 *  - Skeleton space is the space of the scene: the world bind transform of a joint is the inverse of its inverse bind
 *    matrix (the world transform of its node at rest without inverse bind matrices), and the binding pose of a bone
 *    is that transform relative to the one of its parent bone, so a skinned vertex is in scene space as in glTF.
 *  - Clips: every clip of the scene that animates at least one joint of the skin becomes an animation of the length of
 *    the clip. A joint that the clip animates gets a track with a key per key time of its channels; Ogre applies keys
 *    on top of the binding pose, so a key holds the offset from the binding pose (translation added, rotation and scale
 *    multiplied). A joint whose rest transform differs from its binding pose and that the clip does not animate gets a
 *    track with one key at its rest transform, as glTF leaves such joints at rest. Nodes between a joint and its parent
 *    joint, and above the root joints, contribute their rest transforms.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CSkeletonCooker
{
    public:

    CSkeletonCooker( const ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report );

    ~CSkeletonCooker();

    /** Builds the skeleton of the skin under the resource name, in the resource group of the cooker */
    bool Build( UInt32 skinIndex, const CORE::CString& resourceName );

    const Ogre::SkeletonPtr& GetSkeleton( void ) const;

    bool Save( const CORE::CString& vfsPath ) const;

    /** Takes the skeleton out of the skeleton manager */
    void Destroy( void );

    private:

    CSkeletonCooker( const CSkeletonCooker& src );              /**< not implemented */
    CSkeletonCooker& operator=( const CSkeletonCooker& src );   /**< not implemented */

    private:

    const ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    Ogre::SkeletonPtr m_skeleton;
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

#endif /* GUCEF_ASSETCOOKER_OGRE_CSKELETONCOOKER_H ? */
