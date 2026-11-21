/*
 *  assetpluginUNREAL: Maps the resolved parameters of a material to a metal-roughness material of the scene by a table of rules
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H
#include "assetpluginUNREAL_CUnrealResolvedMaterial.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETMATERIAL_H
#include "AssetConverterLib_CAssetMaterial.h"
#define ASSETCONVERTERLIB_CASSETMATERIAL_H
#endif /* ASSETCONVERTERLIB_CASSETMATERIAL_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

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
 *  The rule table that turns the parameters of a material into the values of a glTF material. Names of parameters are
 *  matched without case, spaces, underscores and dashes ("BaseColor_Tint" is "base color tint"). The rules cover the base
 *  colour (tint, brightness and saturation), roughness (also from a minimum and a maximum), metallic with the static
 *  switch that selects it from a packed texture, normal and occlusion strength, specular, emission, opacity, and the base
 *  properties blend mode, mask clip value and two sidedness. Texture parameters are recorded with the slot they belong to
 *  (CUnrealTextureConverter::AttachToMaterials() reads the textures and fills the slots). Everything that no rule consumed goes to the source material of the result, and one
 *  report entry names it.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealMaterialMapper
{
    public:

    static void Map( const CUnrealResolvedMaterial& resolved, const CORE::CString& name, ASSETCONVERTER::CAssetMaterial& out, ASSETCONVERTER::CConversionReport& report );

    /** The name without case, spaces, underscores and dashes */
    static CORE::CString Normalise( const CORE::CString& name );

    private:

    CUnrealMaterialMapper( void );  /**< only static members */
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H ? */
