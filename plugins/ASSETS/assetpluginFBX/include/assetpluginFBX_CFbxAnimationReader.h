/*
 *  assetpluginFBX: the animation stacks of an FBX scene as clips of the converter model
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

#ifndef GUCEF_ASSETPLUGIN_FBX_CFBXANIMATIONREADER_H
#define GUCEF_ASSETPLUGIN_FBX_CFBXANIMATIONREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_FBX_MACROS_H
#include "assetpluginFBX_macros.h"
#define GUCEF_ASSETPLUGIN_FBX_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_FBX_MACROS_H ? */

#include "ufbx.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace FBX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Every animation stack (take) of the file becomes a clip. The animation of the file is baked by ufbx into linear
 *  keys of the translation, rotation and scale of every node (the Euler angles, pivots and curve tangents of FBX
 *  do not exist in the model), so the clips are marked as sampled. Keys are in seconds from the start of the take.
 *  Nodes whose transform does not change in the take get no channel. The animated blend channels of a mesh become
 *  one weights channel of its node, sampled at the times of the keys of those channels.
 */
class GUCEF_ASSETPLUGIN_FBX_EXPORT_CPP CFbxAnimationReader
{
    public:

    typedef GUCEF::vector< Int32 > TIndexVector;

    CFbxAnimationReader( const ufbx_scene& fbx                      ,
                         const TIndexVector& nodeMap                ,
                         ASSETCONVERTER::CAssetScene& scene         ,
                         ASSETCONVERTER::CConversionReport& report  );

    ~CFbxAnimationReader();

    /** Adds the clips of the file to the scene */
    void ReadAnimations( void );

    private:

    CFbxAnimationReader( const CFbxAnimationReader& src );              /**< not implemented */
    CFbxAnimationReader& operator=( const CFbxAnimationReader& src );   /**< not implemented */

    void ReadStack( const ufbx_anim_stack& stack );
    void ReadWeights( const ufbx_anim_stack& stack, const ufbx_baked_anim& baked, ASSETCONVERTER::CAssetAnimation& animation );

    private:

    const ufbx_scene& m_fbx;
    const TIndexVector& m_nodeMap;
    ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FBX */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_FBX_CFBXANIMATIONREADER_H ? */
