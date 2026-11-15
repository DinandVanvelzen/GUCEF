/*
 *  assetpluginGLTF: the exporter that writes the intermediate as glTF with a binary buffer
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFEXPORTER_H
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFEXPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETEXPORTER_H
#include "AssetConverterLib_CIAssetExporter.h"
#define ASSETCONVERTERLIB_CIASSETEXPORTER_H
#endif /* ASSETCONVERTERLIB_CIASSETEXPORTER_H ? */

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
 *  The exporter "gltf": writes <name>.gltf and <name>.bin into the folder it gets, deterministically (CGltfSceneWriter),
 *  and then reads both back through the VFS and runs cgltf_validate on them: a file that does not validate is an error
 *  of the export. Reports the two files, the .gltf with the files it references (the .bin and the baked textures).
 */
class GUCEF_ASSETPLUGIN_GLTF_EXPORT_CPP CGltfExporter : public ASSETCONVERTER::CIAssetExporter
{
    public:

    CGltfExporter( void );

    virtual ~CGltfExporter();

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Export( const ASSETCONVERTER::CAssetScene& scene, const CORE::CString& vfsDir, ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;

    /** Runs cgltf_validate on a glTF or GLB file of the VFS, buffers included; 'problem' says what failed */
    static bool ValidateFile( const CORE::CString& vfsPath, CORE::CString& problem );
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

#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFEXPORTER_H ? */
