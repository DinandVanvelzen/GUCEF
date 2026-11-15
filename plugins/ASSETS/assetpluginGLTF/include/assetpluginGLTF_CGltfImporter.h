/*
 *  assetpluginGLTF: the importer of glTF and GLB files
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFIMPORTER_H
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFIMPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETIMPORTER_H
#include "AssetConverterLib_CIAssetImporter.h"
#define ASSETCONVERTERLIB_CIASSETIMPORTER_H
#endif /* ASSETCONVERTERLIB_CIASSETIMPORTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_MACROS_H
#include "assetpluginGLTF_macros.h"
#define GUCEF_ASSETPLUGIN_GLTF_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The importer "gltf": reads .gltf and .glb files (glTF 2.0) through the VFS into the model (CGltfSceneReader). It
 *  keeps no state between calls, so the tasks of the pipeline can use it at the same time.
 */
class GUCEF_ASSETPLUGIN_GLTF_EXPORT_CPP CGltfImporter : public ASSETCONVERTER::CIAssetImporter
{
    public:

    CGltfImporter( void );

    virtual ~CGltfImporter();

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanImport( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Import( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFIMPORTER_H ? */
