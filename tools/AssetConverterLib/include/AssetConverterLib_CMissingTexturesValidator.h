/*
 *  AssetConverterLib: the built-in validator of missing textures
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

#ifndef ASSETCONVERTERLIB_CMISSINGTEXTURESVALIDATOR_H
#define ASSETCONVERTERLIB_CMISSINGTEXTURESVALIDATOR_H

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
 *  "MissingTextures": textures whose sources could not be read or baked, and material slots that refer to a texture
 *  that is not there. Severity default: error.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CMissingTexturesValidator : public CIAssetValidator
{
    public:

    static const CORE::CString TypeName;    /**< "MissingTextures" */

    CMissingTexturesValidator( void );

    virtual ~CMissingTexturesValidator() GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* ASSETCONVERTERLIB_CMISSINGTEXTURESVALIDATOR_H ? */
