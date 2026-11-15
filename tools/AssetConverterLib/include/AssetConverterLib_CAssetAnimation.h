/*
 *  AssetConverterLib: an animation clip of the intermediate model
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

#ifndef ASSETCONVERTERLIB_CASSETANIMATION_H
#define ASSETCONVERTERLIB_CASSETANIMATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETANIMATIONCHANNEL_H
#include "AssetConverterLib_CAssetAnimationChannel.h"
#define ASSETCONVERTERLIB_CASSETANIMATIONCHANNEL_H
#endif /* ASSETCONVERTERLIB_CASSETANIMATIONCHANNEL_H ? */


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
 *  An animation clip: its channels, whether they were sampled at the frame rate (the tangents of the source could not
 *  be kept) or are the keys of the source, the frame range and rate of the source, the loop flag and the name of the
 *  take of the source it came from (FBX takes, Unity clip splits).
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetAnimation
{
    public:

    typedef GUCEF::vector< CAssetAnimationChannel > TChannelVector;

    CORE::CString name;
    TChannelVector channels;
    bool sampled;
    Float32 firstFrame;
    Float32 lastFrame;
    Float32 frameRate;
    bool loop;
    CORE::CString sourceTake;

    CAssetAnimation( void );

    /** The time of the last key of every channel, in seconds */
    Float32 GetDuration( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETANIMATION_H ? */
