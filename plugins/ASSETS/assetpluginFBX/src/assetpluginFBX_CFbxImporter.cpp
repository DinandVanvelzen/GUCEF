/*
 *  assetpluginFBX: the importer of FBX files
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

#ifndef GUCEF_ASSETPLUGIN_FBX_CFBXSCENEREADER_H
#include "assetpluginFBX_CFbxSceneReader.h"
#define GUCEF_ASSETPLUGIN_FBX_CFBXSCENEREADER_H
#endif /* GUCEF_ASSETPLUGIN_FBX_CFBXSCENEREADER_H ? */

#include "assetpluginFBX_CFbxImporter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace FBX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFbxImporter::CFbxImporter( void )
    : ASSETCONVERTER::CIAssetImporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFbxImporter::~CFbxImporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CFbxImporter::GetName( void ) const
{GUCEF_TRACE;

    return "fbx";
}

/*-------------------------------------------------------------------------*/

UInt32
CFbxImporter::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CFbxImporter::CanImport( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& /* context */ ) const
{GUCEF_TRACE;

    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( vfsPath );
    return "fbx" == extension;
}

/*-------------------------------------------------------------------------*/

bool
CFbxImporter::Import( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    out.provenance.importerName = GetName();
    out.provenance.importerVersion = GetVersion();
    CFbxSceneReader reader( vfsPath, context, out, report );
    return reader.Read();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FBX */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
