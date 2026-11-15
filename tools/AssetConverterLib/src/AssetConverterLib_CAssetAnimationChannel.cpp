/*
 *  AssetConverterLib: a channel of an animation: the keys of one property of one node
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

#include "AssetConverterLib_CAssetAnimationChannel.h"

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

CAssetAnimationChannel::CAssetAnimationChannel( void )
    : targetNode( 0 )
    , path( ANIMATIONPATH_TRANSLATION )
    , interpolation( ANIMATIONINTERPOLATION_LINEAR )
    , times()
    , values()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CAssetAnimationChannel::GetComponentCount( void ) const
{GUCEF_TRACE;

    switch ( path )
    {
        case ANIMATIONPATH_TRANSLATION:
        case ANIMATIONPATH_SCALE:
            return 3;
        case ANIMATIONPATH_ROTATION:
            return 4;
        default:
        {
            if ( times.empty() )
                return 0;
            const UInt32 perKey = (UInt32) ( values.size() / times.size() );
            return ANIMATIONINTERPOLATION_CUBICSPLINE == interpolation ? perKey / 3 : perKey;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
