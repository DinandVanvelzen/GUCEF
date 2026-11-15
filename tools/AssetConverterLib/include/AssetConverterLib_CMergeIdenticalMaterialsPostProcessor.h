/*
 *  AssetConverterLib: the built-in post-processor that merges materials that render the same
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

#ifndef ASSETCONVERTERLIB_CMERGEIDENTICALMATERIALSPOSTPROCESSOR_H
#define ASSETCONVERTERLIB_CMERGEIDENTICALMATERIALSPOSTPROCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#include "AssetConverterLib_CIAssetPostProcessor.h"
#define ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#endif /* ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H ? */


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
 *  "MergeIdenticalMaterials": a material with the same appearance as an earlier one (CAssetMaterial::HasSameAppearance:
 *  every resolved value, texture and extension, not the name or the source material) is merged into it: its submeshes
 *  use the earlier one and it goes. Every merge is reported, with both names.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CMergeIdenticalMaterialsPostProcessor : public CIAssetPostProcessor
{
    public:

    static const CORE::CString TypeName;    /**< "MergeIdenticalMaterials" */

    CMergeIdenticalMaterialsPostProcessor( void );

    virtual ~CMergeIdenticalMaterialsPostProcessor() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Process( CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CMERGEIDENTICALMATERIALSPOSTPROCESSOR_H ? */
