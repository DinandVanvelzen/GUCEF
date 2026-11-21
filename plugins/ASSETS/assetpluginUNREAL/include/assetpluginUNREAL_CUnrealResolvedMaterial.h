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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a material instance means after its parents are followed: the value of every scalar, vector, texture and static
 *  switch parameter (the instance overrides its parent instance, which overrides the master material, whose expressions
 *  hold the defaults) and the base properties (blend mode, shading model, two sided, mask clip value) of the chain.
 *  The names are the names of the parameters as authored.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealResolvedMaterial
{
    public:

    struct SColour
    {
        Float64 r;
        Float64 g;
        Float64 b;
        Float64 a;
    };
    typedef GUCEF::map< CORE::CString, Float64 > TScalarMap;
    typedef GUCEF::map< CORE::CString, SColour > TColourMap;
    typedef GUCEF::map< CORE::CString, CORE::CString > TTextureMap;     /**< parameter name to object path */
    typedef GUCEF::map< CORE::CString, bool > TSwitchMap;

    CUnrealResolvedMaterial( void );

    CORE::CString name;                         /**< object name of the material the chain started with */
    CORE::CString masterPath;                   /**< object path of the master material, empty when the chain ends before it */
    CORE::CString::StringVector chain;          /**< package names, the instance first */
    CORE::CString::StringVector problems;       /**< what was not found or not understood */
    TScalarMap scalars;
    TColourMap colours;
    TTextureMap textures;
    TSwitchMap switches;
    CORE::CString blendMode;                    /**< BLEND_Opaque, BLEND_Masked, ..., empty when not stated: opaque */
    CORE::CString shadingModel;                 /**< MSM_DefaultLit, ..., empty when not stated */
    bool twoSided;
    Float64 opacityMaskClipValue;
    bool hasTwoSided;
    bool hasClipValue;

    bool HasScalar( const CORE::CString& parameter ) const;
    bool HasColour( const CORE::CString& parameter ) const;
    bool HasTexture( const CORE::CString& parameter ) const;
    bool HasSwitch( const CORE::CString& parameter ) const;
};

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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H ? */
