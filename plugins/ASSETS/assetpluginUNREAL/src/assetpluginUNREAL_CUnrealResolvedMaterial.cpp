/*
 *  assetpluginUNREAL: The parameters of a material after its instance chain is resolved
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H
#include "assetpluginUNREAL_CUnrealResolvedMaterial.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealResolvedMaterial::CUnrealResolvedMaterial( void )
    : name()
    , masterPath()
    , chain()
    , problems()
    , scalars()
    , colours()
    , textures()
    , switches()
    , blendMode()
    , shadingModel()
    , twoSided( false )
    , opacityMaskClipValue( 0.3333 )
    , hasTwoSided( false )
    , hasClipValue( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealResolvedMaterial::HasScalar( const CORE::CString& parameter ) const
{GUCEF_TRACE;

    return scalars.find( parameter ) != scalars.end();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealResolvedMaterial::HasColour( const CORE::CString& parameter ) const
{GUCEF_TRACE;

    return colours.find( parameter ) != colours.end();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealResolvedMaterial::HasTexture( const CORE::CString& parameter ) const
{GUCEF_TRACE;

    return textures.find( parameter ) != textures.end();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealResolvedMaterial::HasSwitch( const CORE::CString& parameter ) const
{GUCEF_TRACE;

    return switches.find( parameter ) != switches.end();
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
