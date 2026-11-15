/*
 *  AssetConverterLib: the built-in validator of the bones per submesh
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

#ifndef ASSETCONVERTERLIB_CBONELIMITVALIDATOR_H
#define ASSETCONVERTERLIB_CBONELIMITVALIDATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETVALIDATOR_H
#include "AssetConverterLib_CIAssetValidator.h"
#define ASSETCONVERTERLIB_CIASSETVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CIASSETVALIDATOR_H ? */


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
 *  "BoneLimit": the bones a skinned submesh uses (joints with a weight above 0) against "maxBones", the limit of the
 *  hardware skinning of the cook target. Without the setting the pipeline gives the lowest limit of the cookers of the
 *  run (CIAssetCooker::GetMaxBonesPerSubMesh()); 0 checks nothing. Severity default: error.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CBoneLimitValidator : public CIAssetValidator
{
    public:

    static const CORE::CString TypeName;    /**< "BoneLimit" */

    CBoneLimitValidator( void );

    virtual ~CBoneLimitValidator() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CBONELIMITVALIDATOR_H ? */
