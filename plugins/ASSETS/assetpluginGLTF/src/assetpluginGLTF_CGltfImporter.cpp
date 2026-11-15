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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEREADER_H
#include "assetpluginGLTF_CGltfSceneReader.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEREADER_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEREADER_H ? */

#include "assetpluginGLTF_CGltfImporter.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGltfImporter::CGltfImporter( void )
    : ASSETCONVERTER::CIAssetImporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGltfImporter::~CGltfImporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CGltfImporter::GetName( void ) const
{GUCEF_TRACE;

    return "gltf";
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfImporter::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CGltfImporter::CanImport( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& /* context */ ) const
{GUCEF_TRACE;

    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( vfsPath );
    return "gltf" == extension || "glb" == extension;
}

/*-------------------------------------------------------------------------*/

bool
CGltfImporter::Import( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    out.provenance.importerName = GetName();
    out.provenance.importerVersion = GetVersion();
    CGltfSceneReader reader( vfsPath, context, out, report );
    return reader.Read();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
