/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CCOOKSETTINGS_H
#include "AssetConverterLib_CCookSettings.h"
#define ASSETCONVERTERLIB_CCOOKSETTINGS_H
#endif /* ASSETCONVERTERLIB_CCOOKSETTINGS_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "assetcookerOGRE_CTextureCooker.h"
#include "assetcookerOGRE_CAssetCook.h"
#include "assetcookerOGRE_COgreCooker.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COgreCooker::TargetName = "ogre";
const UInt32 COgreCooker::Version = 2;

static const char* const IntermediateImporter = "gltf";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreCooker::COgreCooker( void )
    : ASSETCONVERTER::CIAssetCooker()
    , m_environment()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreCooker::~COgreCooker()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

CORE::CString COgreCooker::GetTargetName( void ) const { return TargetName; }
UInt32 COgreCooker::GetVersion( void ) const { return Version; }
UInt32 COgreCooker::GetMaxBonesPerSubMesh( void ) const { return 256; }

/*-------------------------------------------------------------------------*/

void
COgreCooker::Release( void )
{GUCEF_TRACE;

    m_environment.Release();
}

/*-------------------------------------------------------------------------*/

bool
COgreCooker::Cook( const CORE::CString& intermediateDir                 ,
                   const CORE::CString& outDir                          ,
                   const ASSETCONVERTER::CCookSettings& settings        ,
                   ASSETCONVERTER::CConversionReport& report            )
{GUCEF_TRACE;

    CORE::CString problem;
    if ( !m_environment.Prepare( problem ) )
    {
        report.SelectAsset( CORE::CString::Empty );
        report.AddError( TargetName, settings.profile.name, problem );
        return false;
    }

    ASSETCONVERTER::TAssetImporterPtr importer;
    if ( !ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( IntermediateImporter, importer ) )
    {
        report.SelectAsset( CORE::CString::Empty );
        report.AddError( TargetName, settings.profile.name, "the glTF importer that reads the intermediate is not registered (is assetpluginGLTF loaded?)" );
        return false;
    }

    // the intermediate as a pack of its own, which is what the importer reads from
    ASSETCONVERTER::CAssetPackManifest manifest;
    manifest.id = settings.packId;
    manifest.source = "/" + ASSETCONVERTER::CVfsPaths::Normalize( intermediateDir );
    manifest.intermediate = manifest.source;
    ASSETCONVERTER::CAssetPackContext context;
    if ( !context.Open( manifest ) )
    {
        report.SelectAsset( CORE::CString::Empty );
        report.AddError( TargetName, intermediateDir, "the intermediate folder cannot be read" );
        return false;
    }

    VFS::CVfsGlobal::Instance()->GetVfs().CreateDir( outDir );
    CTextureCooker textures( outDir, settings.packId, intermediateDir, settings.sourceRoot, settings.profile );
    UInt32 cooked = 0;
    for ( size_t a=0; a<settings.assets.size(); ++a )
    {
        const ASSETCONVERTER::CCookSettings::SAsset& asset = settings.assets[ a ];
        report.SelectAsset( asset.assetPath );

        CORE::CString gltfPath;
        for ( size_t f=0; f<asset.files.size() && gltfPath.IsNULLOrEmpty(); ++f )
        {
            const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( asset.files[ f ] );
            if ( "gltf" == extension || "glb" == extension )
                gltfPath = asset.files[ f ];
        }
        if ( gltfPath.IsNULLOrEmpty() )
        {
            report.AddError( TargetName, asset.assetPath, "the asset has no glTF in its intermediate" );
            continue;
        }

        // the intermediate back into the model; what the reader finds is only of interest when it is an error
        ASSETCONVERTER::CAssetScene scene;
        ASSETCONVERTER::CConversionReport readReport;
        readReport.SelectAsset( asset.assetPath );
        const bool read = importer->Import( gltfPath, context, scene, readReport );
        const ASSETCONVERTER::CConversionReportEntry* readEntry = readReport.FindEntry( asset.assetPath );
        if ( GUCEF_NULL != readEntry )
            for ( size_t i=0; i<readEntry->items.size(); ++i )
                if ( ASSETCONVERTER::REPORTITEM_ERROR == readEntry->items[ i ].kind )
                    report.AddError( readEntry->items[ i ].source, readEntry->items[ i ].subject, readEntry->items[ i ].message );
        if ( !read )
        {
            report.AddError( TargetName, gltfPath, "the intermediate cannot be read back" );
            continue;
        }

        CORE::CDataNode sidecar;
        if ( !asset.sidecarFile.IsNULLOrEmpty() && VFS::CVfsGlobal::Instance()->GetVfs().FileExists( asset.sidecarFile ) )
            ASSETCONVERTER::CCanonicalJson::Load( asset.sidecarFile, sidecar );

        CAssetCook cook( settings, asset, outDir, textures, report );
        if ( cook.Cook( scene, sidecar ) )
            ++cooked;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "assetcookerOGRE: Profile " << settings.profile.name << " of pack " << settings.packId << ": " << cooked << " of " <<
               (UInt32) settings.assets.size() << " assets cooked, " << textures.GetWrittenCount() << " textures written" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
