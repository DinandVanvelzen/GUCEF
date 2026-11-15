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

#ifndef GUCEF_ASSETCOOKER_OGRE_CNODEANIMATIONSAMPLER_H
#define GUCEF_ASSETCOOKER_OGRE_CNODEANIMATIONSAMPLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  The local transform of one node in one clip of the intermediate: the translation, rotation and scale channels
 *  that target the node, sampled at any time. A path without a channel keeps the rest value of the node. Linear keys
 *  interpolate (rotations along the shortest arc), step keys hold, and cubic spline keys are read at their values
 *  (their tangents are not used, which HasNonLinearKeys() tells the caller to report).
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CNodeAnimationSampler
{
    public:

    typedef GUCEF::vector< Float32 > TFloatVector;

    CNodeAnimationSampler( const ASSETCONVERTER::CAssetScene& scene, const ASSETCONVERTER::CAssetAnimation& animation, UInt32 node );

    /** Whether a translation, rotation or scale channel of the clip targets the node */
    bool IsAnimated( void ) const;

    /** Whether a channel of the node has step or cubic spline keys */
    bool HasNonLinearKeys( void ) const;

    /** The key times of the channels of the node together, sorted, each once */
    void GetKeyTimes( TFloatVector& times ) const;

    CSceneMath::STrs Sample( Float32 time ) const;

    const CSceneMath::STrs& GetRest( void ) const;

    /**
     *  The values of a key of a channel: the value itself for linear and step keys, the middle of the three values
     *  of a cubic spline key. 'components' values are read into 'out'
     */
    static void ReadKey( const ASSETCONVERTER::CAssetAnimationChannel& channel, UInt32 key, UInt32 components, Float32* out );

    private:

    void SamplePath( UInt32 path, Float32 time, Float32* out ) const;

    private:

    const ASSETCONVERTER::CAssetAnimationChannel* m_channels[ 3 ];
    CSceneMath::STrs m_rest;
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

#endif /* GUCEF_ASSETCOOKER_OGRE_CNODEANIMATIONSAMPLER_H ? */
