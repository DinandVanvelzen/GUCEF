/*
 *  AssetConverterLib: the built-in post-processor that makes tangents for normal mapped submeshes
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

#ifndef ASSETCONVERTERLIB_CGENERATETANGENTSPOSTPROCESSOR_H
#define ASSETCONVERTERLIB_CGENERATETANGENTSPOSTPROCESSOR_H

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

class CAssetSubMesh;

/*-------------------------------------------------------------------------*/

/**
 *  "GenerateTangents": makes the tangents of submeshes whose material uses a normal map, when the source has none,
 *  from the UV set of the normal map: per triangle the direction of increasing U, accumulated per vertex, made
 *  orthogonal to the normal, and the handedness of the bitangent in w. This is not MikkTSpace, which glTF readers use
 *  for meshes without tangents, so it is reported as an approximation. Submeshes without normals or without that UV
 *  set are left without tangents, which is reported too.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CGenerateTangentsPostProcessor : public CIAssetPostProcessor
{
    public:

    static const CORE::CString TypeName;    /**< "GenerateTangents" */

    CGenerateTangentsPostProcessor( void );

    virtual ~CGenerateTangentsPostProcessor() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Process( CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the tangents of the submesh from the UV set; false without normals or that UV set */
    static bool GenerateTangents( CAssetSubMesh& subMesh, UInt32 uvSet );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CGENERATETANGENTSPOSTPROCESSOR_H ? */
