/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define CDYNAMICBUFFER_H
#endif /* CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#include "gucefASSETSTORE_CAssetToolUris.h"
#define GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLURIS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#include "AssetConverterLib_CConversionSidecar.h"
#define ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H ? */

#ifndef ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H
#include "AssetConverterLib_CImportSettingsBuilder.h"
#define ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H
#endif /* ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetStoreConverterHook_CPipelineResourceOperations.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const JsonMime = "application/json";

bool
ReadFileText( const CORE::CString& vfsPath, CORE::CString& text, UInt64& size )
{GUCEF_TRACE;

    text.Clear();
    size = 0;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( vfsPath.IsNULLOrEmpty() || !vfs.FileExists( vfsPath ) )
        return false;
    size = vfs.GetFileSize( vfsPath );
    CORE::CDynamicBuffer buffer;
    if ( !vfs.LoadFile( buffer, vfsPath, "rb" ) )
        return false;
    size = buffer.GetDataSize();
    if ( buffer.GetDataSize() > 0 )
        text.Set( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
    return true;
}

ASSETSTORE::TAssetStoreStatus
Answer( CPipelineOperationCall& call, const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CString& text )
{GUCEF_TRACE;

    UInt32 limit = call.request.arguments.GetAttributeValueOrChildValueByName( "maxResourceBytes" ).AsUInt32( call.settings.maxResourceBytes, true );
    if ( 0 == limit || limit > call.settings.maxResourceBytes )
        limit = call.settings.maxResourceBytes;
    if ( text.Length() > limit )
        return call.Fail( CPipelineOperationCall::ErrorTooLarge, "the resource is larger than " + CORE::ToString( limit ) + " bytes: use the paged tools (pack_assets, pack_activity, job_result) instead" );
    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "uri", uri );
    document.SetAttribute( "mimeType", mimeType );
    document.SetAttribute( "text", text );
    return call.Succeed();
}

ASSETSTORE::TAssetStoreStatus
AnswerFile( CPipelineOperationCall& call, const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CString& vfsPath, const CORE::CString& what )
{GUCEF_TRACE;

    CORE::CString text;
    UInt64 size = 0;
    if ( !ReadFileText( vfsPath, text, size ) )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no " + what + " yet" );
    return Answer( call, uri, mimeType, text );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CPipelineResourceOperations::GetManifestReference( void )
{GUCEF_TRACE;

    return
    "# Converter pack manifest (<pack>.assetpack.json)\n\n"
    "A JSON object that tells the converter pipeline what a pack is, where its files are and how to convert them. "
    "Folders are VFS paths, relative to the folder of the manifest or absolute (starting with '/'); through the tools they "
    "must lie in roots of the matching kind.\n\n"
    "| member | meaning |\n|---|---|\n"
    "| `id` | the pack id; unique among the manifests |\n"
    "| `title`, `store` | names for people |\n"
    "| `licenceRecord` | the id of the licence record `<id>.licence.json`; required for a conversion; never changed through the tools |\n"
    "| `licenceFolder` | the folder of the licence record (a licences root); default the folder of the manifest |\n"
    "| `source` | the folder or archive of the purchased files (a source or staging root) |\n"
    "| `importer` | the importer, or `auto` (the first importer that takes a file) |\n"
    "| `include`, `exclude` | globs on pack relative paths: the files of the pack |\n"
    "| `assets` | globs that select the assets to convert among the files |\n"
    "| `rules` | pack wide settings: `lod`, `colliders`, `shaderPropertyMaps` (per shader: property name to slot, or `ignore`), material overrides |\n"
    "| `nameHints` | the name hints preset of the post-processor |\n"
    "| `presets` | settings per asset glob, applied in order |\n"
    "| `overrides` | the folder of the per asset overrides `<assetPath>.import.json` (an overrides root) |\n"
    "| `postProcessors` | per source format (or `*`) a list of `{ name, settings }` |\n"
    "| `validators` | a list of `{ name, settings }` |\n"
    "| `exporter` | the exporter of the intermediate (default `gltf`) |\n"
    "| `intermediate` | the folder of the engine neutral intermediate, the cache, the sidecars and the report (an intermediate root) |\n"
    "| `output` | the folder of the cooked files (an output root) |\n"
    "| `cookTargets` | the cookers to run (`ogre`, ...) |\n"
    "| `cookProfiles` | a list of `{ name, ... }` output settings per profile |\n\n"
    "Settings of an asset are layered: the importer defaults, `rules`, the `presets` that match, then the override file. "
    "A later layer wins per key; objects merge member by member; a list replaces a list.\n";
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineResourceOperations::ReadResource( CPipelineOperationCall& call, const CPipelinePlanStore& plans )
{GUCEF_TRACE;

    const CORE::CString uri = call.GetArgument( "uri" );
    CORE::CString::StringVector segments;
    if ( !ASSETSTORE::CAssetToolUris::Split( uri, segments ) || segments.empty() )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no resource " + uri );

    if ( "docs" == segments[ 0 ] )
    {
        if ( 2 == segments.size() && "manifest-reference" == segments[ 1 ] )
            return Answer( call, uri, "text/markdown", GetManifestReference() );
        if ( 3 == segments.size() && "import-settings" == segments[ 1 ] )
        {
            ASSETCONVERTER::TAssetImporterPtr importer;
            if ( !ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( segments[ 2 ], importer ) )
                return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no importer " + segments[ 2 ] );
            CORE::CDataNode defaults( "settings", GUCEF_DATATYPE_OBJECT );
            importer->GetDefaultSettings( defaults );
            defaults.SetName( CORE::CString::Empty );
            defaults.SetNodeType( GUCEF_DATATYPE_OBJECT );
            return Answer( call, uri, JsonMime, ASSETCONVERTER::CCanonicalJson::ToJson( defaults ) );
        }
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no resource " + uri );
    }
    if ( "packs" != segments[ 0 ] || segments.size() < 3 )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no resource " + uri );

    CConverterPack pack;
    if ( !call.FindPack( segments[ 1 ], pack ) )
        return call.result.status;
    const CORE::CString member = segments[ 2 ];
    if ( 3 == segments.size() && "manifest" == member )
        return AnswerFile( call, uri, JsonMime, pack.manifestVfsPath, "manifest" );
    if ( !pack.runnable )
        return call.Fail( CPipelineOperationCall::ErrorPathRefused, "the files of the pack " + pack.packId + " are not served through the tools: " + pack.runnableReason );

    if ( 3 == segments.size() )
    {
        if ( "licence" == member )
        {
            if ( !pack.licence.recordFound )
                return call.Fail( CPipelineOperationCall::ErrorNotFound, "the pack has no licence record that can be read" );
            CORE::CDataNode record( "licence", GUCEF_DATATYPE_OBJECT );
            pack.licence.record.Serialize( record );
            if ( !call.exposePurchaseDetails )
            {
                record.DelAttribute( "purchaser" );
                record.DelAttribute( "orderReference" );
            }
            record.SetName( CORE::CString::Empty );
            return Answer( call, uri, JsonMime, ASSETCONVERTER::CCanonicalJson::ToJson( record ) );
        }
        if ( "report" == member )
            return AnswerFile( call, uri, JsonMime, pack.GetReportPath(), "report" );
        if ( "report-summary" == member )
            return AnswerFile( call, uri, "text/plain", ASSETCONVERTER::CVfsPaths::WithoutExtension( pack.GetReportPath() ) + ".txt", "report summary" );
        if ( "activity" == member )
        {
            ASSETCONVERTER::CImportActivityLog log;
            if ( !pack.LoadActivity( log ) )
                return call.Fail( CPipelineOperationCall::ErrorNotFound, "the pack has no activity log yet" );
            ASSETCONVERTER::CImportActivityLog recent;
            const ASSETCONVERTER::CImportActivityLog::TRunVector& runs = log.GetRuns();
            for ( size_t r = runs.size() > 5 ? runs.size() - 5 : 0; r<runs.size(); ++r )
                recent.AddRun( runs[ r ] );
            CORE::CDataNode node( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            recent.Serialize( node );
            node.SetName( CORE::CString::Empty );
            return Answer( call, uri, JsonMime, ASSETCONVERTER::CCanonicalJson::ToJson( node ) );
        }
        if ( "plan" == member )
        {
            CORE::CDataNode plan;
            if ( !plans.Get( pack.packId, plan ) )
                return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no plan of the pack: call convert_plan first" );
            plan.SetName( CORE::CString::Empty );
            return Answer( call, uri, JsonMime, ASSETCONVERTER::CCanonicalJson::ToJson( plan ) );
        }
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no resource " + uri );
    }
    if ( "cook" == member && 5 == segments.size() )
    {
        const CORE::CString folder = pack.FindProfileFolder( segments[ 3 ] );
        if ( folder.IsNULLOrEmpty() )
            return call.Fail( CPipelineOperationCall::ErrorNotFound, "the pack has no cooked files of the profile " + segments[ 3 ] );
        if ( "references" == segments[ 4 ] )
            return AnswerFile( call, uri, JsonMime, ASSETCONVERTER::CVfsPaths::Join( folder, "references.json" ), "reference report" );
        if ( "sizes" == segments[ 4 ] )
            return AnswerFile( call, uri, JsonMime, ASSETCONVERTER::CVfsPaths::Join( folder, "sizes.json" ), "size report" );
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no resource " + uri );
    }
    if ( "assets" == member && segments.size() >= 5 )
    {
        CORE::CString assetPath;
        for ( size_t s=3; s+1<segments.size(); ++s )
            assetPath += ( assetPath.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( "/" ) ) + segments[ s ];
        if ( !call.CheckAssetPath( assetPath ) )
            return call.result.status;
        const CORE::CString which = segments.back();
        if ( "report" == which )
        {
            ASSETCONVERTER::CConversionReport report;
            const ASSETCONVERTER::CConversionReportEntry* entry = pack.LoadReport( report ) ? report.FindEntry( assetPath ) : GUCEF_NULL;
            if ( GUCEF_NULL == entry )
                return call.Fail( CPipelineOperationCall::ErrorNotFound, "the report has no asset " + assetPath );
            CORE::CDataNode node( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            entry->Serialize( node );
            node.SetName( CORE::CString::Empty );
            return Answer( call, uri, JsonMime, ASSETCONVERTER::CCanonicalJson::ToJson( node ) );
        }
        if ( "sidecar" == which )
            return AnswerFile( call, uri, JsonMime, ASSETCONVERTER::CConversionSidecar::GetPath( pack.manifest.GetIntermediatePath(), assetPath ), "sidecar of the asset" );
        if ( "override" == which )
        {
            const CORE::CString path = ASSETCONVERTER::CImportSettingsBuilder::GetOverridePath( pack.manifest, assetPath );
            CORE::CString text;
            UInt64 size = 0;
            if ( path.IsNULLOrEmpty() || !ReadFileText( path, text, size ) )
                return Answer( call, uri, JsonMime, "{ \"exists\": false }" );
            return Answer( call, uri, JsonMime, text );
        }
    }
    return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no resource " + uri );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
