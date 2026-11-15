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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "AssetConverterLib_CAssetAnimation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetAnimation::CAssetAnimation( void )
    : name()
    , channels()
    , sampled( false )
    , firstFrame( 0.0f )
    , lastFrame( 0.0f )
    , frameRate( 30.0f )
    , loop( false )
    , sourceTake()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Float32
CAssetAnimation::GetDuration( void ) const
{GUCEF_TRACE;

    Float32 duration = 0.0f;
    for ( size_t i=0; i<channels.size(); ++i )
        if ( !channels[ i ].times.empty() && channels[ i ].times.back() > duration )
            duration = channels[ i ].times.back();
    return duration;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
