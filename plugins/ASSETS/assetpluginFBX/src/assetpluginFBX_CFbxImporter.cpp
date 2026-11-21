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

#include "AssetConverterLib_CAssetPackListing.h"
#include "AssetConverterLib_CAssetPackProfile.h"

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

    // 2: records the normalized relations: texture files (usesTexture), texture files that are not found kept unresolved
    return 2;
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

/*-------------------------------------------------------------------------*/

void
CFbxImporter::GetFileKinds( ASSETCONVERTER::TAssetFileKindVector& kinds ) const
{GUCEF_TRACE;

    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "fbx", ASSETCONVERTER::ASSETKIND_MODEL ) );
}

/*-------------------------------------------------------------------------*/

bool
CFbxImporter::RecognisePack( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile ) const
{GUCEF_TRACE;

    // a folder with FBX files and nothing that says it belongs to an engine: a pack of models
    CORE::CString::StringVector keywords;
    keywords.push_back( "scene" );
    keywords.push_back( "demo" );
    keywords.push_back( "sample" );
    CORE::CString::StringVector models;
    if ( 0 == listing.FindFilesRanked( "*.fbx", keywords, models, 16 ) )
        return false;
    profile.Clear();
    profile.packType = "model-pack";
    profile.packTypeName = "Model pack";
    profile.confidence = 30;
    profile.showcaseCandidates = models;
    listing.GetSubFolders( CORE::CString::Empty, profile.groupingHints );
    return true;
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
