/*
 *  AssetWorkbenchLib: turns one asset of a pack into files the views read
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINE_H
#include "AssetConverterLib_CAssetPipeline.h"
#define ASSETCONVERTERLIB_CASSETPIPELINE_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H
#include "AssetConverterLib_CAssetConversionJob.h"
#define ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H ? */

#ifndef ASSETCONVERTERLIB_CTEXTUREBAKER_H
#include "AssetConverterLib_CTextureBaker.h"
#define ASSETCONVERTERLIB_CTEXTUREBAKER_H
#endif /* ASSETCONVERTERLIB_CTEXTUREBAKER_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#include "AssetConverterLib_CConversionSidecar.h"
#define ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CCOOKSETTINGS_H
#include "AssetConverterLib_CCookSettings.h"
#define ASSETCONVERTERLIB_CCOOKSETTINGS_H
#endif /* ASSETCONVERTERLIB_CCOOKSETTINGS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetWorkbenchLib_CPackAssetConverter.h"
#include "AssetWorkbenchLib_CPackPreviewPostProcessor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackAssetConverter::SRequest::SRequest( void )
    : converter(), sourceFolder(), sourceDisk(), assetPath(), cacheFolder(), id(), cook( false ), force( false ), mergeStatic( false ), staticPreview( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackAssetConverter::SResult::SResult( void )
    : ok( false ), problem(), mainFile(), sidecarFile(), files(), sidecar(), warningCount( 0 ), errorCount( 0 ), summary()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString CPackAssetConverter::GetCacheMountPoint( const SRequest& request ) { return "wbpw_" + request.id; }
CORE::CString CPackAssetConverter::GetSourceMountPoint( const SRequest& request ) { return "wbps_" + request.id; }

/*-------------------------------------------------------------------------*/

bool
CPackAssetConverter::Mount( const SRequest& request )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString cache = CORE::RelativePath( request.cacheFolder );
    if ( !CORE::DirExists( cache ) )
        CORE::CreateDirs( cache );
    if ( vfs.IsMountedArchive( GetCacheMountPoint( request ) ) )
        return true;
    if ( !vfs.MountArchive( "AssetWorkbench/pack/" + request.id + "/w", cache, VFS::CVFS::FileSystemArchiveTypeName, GetCacheMountPoint( request ), true, false, cache ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CPackAssetConverter: The cache folder " << cache << " cannot be mounted" );
        return false;
    }
    if ( !request.sourceDisk.IsNULLOrEmpty() )
    {
        const CORE::CString folder = CORE::StripFilename( CORE::RelativePath( request.sourceDisk ) );
        if ( !vfs.MountArchive( "AssetWorkbench/pack/" + request.id + "/s", folder, VFS::CVFS::FileSystemArchiveTypeName, GetSourceMountPoint( request ), false, false, folder ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CPackAssetConverter: The folder " << folder << " of the source cannot be mounted" );
            vfs.UnmountArchiveByName( "AssetWorkbench/pack/" + request.id + "/w" );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPackAssetConverter::Unmount( const SRequest& request )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !request.sourceDisk.IsNULLOrEmpty() )
        vfs.UnmountArchiveByName( "AssetWorkbench/pack/" + request.id + "/s" );
    vfs.UnmountArchiveByName( "AssetWorkbench/pack/" + request.id + "/w" );
}

/*-------------------------------------------------------------------------*/

namespace {

/** What the importers of a long conversion look at: the load control of the job tells when the user cancelled or something else was opened */
bool
IsControlCancelled( void* context )
{
    return GUCEF_NULL != context && static_cast< const CAssetLoadControl* >( context )->IsCancelled();
}

void
ReportControlProgress( void* context, Float32 fraction )
{
    if ( GUCEF_NULL != context )
        static_cast< CAssetLoadControl* >( context )->SetProgress( fraction );
}

}

/*-------------------------------------------------------------------------*/

bool
CPackAssetConverter::Convert( const SRequest& request, SResult& result, const CAssetLoadControl* control )
{GUCEF_TRACE;

    result = SResult();
    ASSETCONVERTER::CAssetConverterGlobal* global = ASSETCONVERTER::CAssetConverterGlobal::Instance();
    if ( GUCEF_NULL == global )
    {
        result.problem = "The asset converter is not available";
        return false;
    }
    ASSETCONVERTER::CAssetPluginRegistry& registry = global->GetPluginRegistry();
    ASSETCONVERTER::TAssetImporterPtr importer;
    ASSETCONVERTER::TAssetExporterPtr exporter;
    if ( !registry.TryGetImporter( request.converter, importer ) )
    {
        result.problem = "The converter plugin " + request.converter + " is not loaded";
        return false;
    }
    if ( !registry.TryGetExporter( "gltf", exporter ) )
    {
        result.problem = "The glTF exporter plugin is not loaded";
        return false;
    }

    // the cooker is wanted before the import: its version is part of the key of the cache entry
    ASSETCONVERTER::TAssetCookerPtr cooker;
    if ( request.cook && !registry.TryGetCooker( "ogre", cooker ) )
    {
        result.problem = "The Ogre cooker plugin is not loaded";
        return false;
    }

    const CORE::CString cacheRoot = "/" + GetCacheMountPoint( request );
    ASSETCONVERTER::CAssetPackManifest manifest;
    manifest.id = "wbpack_" + request.id;
    manifest.title = manifest.id;
    manifest.importer = request.converter;
    manifest.exporter = "gltf";
    manifest.intermediate = cacheRoot + "/intermediate";
    manifest.output = cacheRoot + "/out";
    if ( request.sourceDisk.IsNULLOrEmpty() )
    {
        manifest.source = request.sourceFolder.IsNULLOrEmpty() || '/' == request.sourceFolder[ 0 ] ? request.sourceFolder : "/" + request.sourceFolder;
    }
    else
    {
        manifest.source = "/" + GetSourceMountPoint( request );
        manifest.include.push_back( request.assetPath );
    }

    // the portrait of a character: the default post-processors of the importer, then the one that drops the rig
    if ( request.staticPreview || request.mergeStatic )
    {
        const CORE::CString& processorName = request.mergeStatic ? CPackPreviewPostProcessor::MergedProcessorName : CPackPreviewPostProcessor::ProcessorName;
        ASSETCONVERTER::TAssetPostProcessorPtr preview;
        if ( !registry.TryGetPostProcessor( processorName, preview ) )
            registry.RegisterPostProcessor( ASSETCONVERTER::TAssetPostProcessorPtr( GUCEF_NEW CPackPreviewPostProcessor( request.mergeStatic ) ) );
        CORE::CString::StringVector names;
        importer->GetDefaultPostProcessors( names );
        names.push_back( processorName );
        ASSETCONVERTER::CPluginStackEntry::TEntryVector stack;
        for ( size_t i=0; i<names.size(); ++i )
            stack.push_back( ASSETCONVERTER::CPluginStackEntry( names[ i ] ) );
        manifest.postProcessors[ ASSETCONVERTER::CAssetPackManifest::AnySourceFormat ] = stack;
    }

    ASSETCONVERTER::CAssetPackContext context;
    if ( !context.Open( manifest ) )
    {
        result.problem = "The source of the pack cannot be opened";
        return false;
    }
    if ( !context.HasFile( request.assetPath ) )
    {
        result.problem = "The asset " + request.assetPath + " is not in the pack";
        return false;
    }
    if ( !importer->IndexPack( context ) )
    {
        result.problem = "The pack cannot be indexed by the " + request.converter + " importer";
        return false;
    }
    if ( GUCEF_NULL != control && control->IsCancelled() )
    {
        result.problem = "Cancelled";
        return false;
    }

    ASSETCONVERTER::CTextureBaker baker( ASSETCONVERTER::CVfsPaths::Join( manifest.GetIntermediatePath(), "textures" ) );
    ASSETCONVERTER::CAssetConversionJob job;
    job.manifest = &manifest;
    job.context = &context;
    job.baker = &baker;
    job.importer = importer;
    job.exporter = exporter;
    job.assetPath = request.assetPath;
    job.reimport = request.force;
    if ( request.cook )
        job.cookerVersions[ "cooker:" + cooker->GetTargetName() ] = cooker->GetVersion();
    if ( GUCEF_NULL != control )
    {
        // the importer stops within a moment of a cancel and reports how far it is to the activity of the job
        job.report.SetCancellationCheck( &IsControlCancelled, const_cast< CAssetLoadControl* >( control ) );
        job.report.SetProgressSink( &ReportControlProgress, const_cast< CAssetLoadControl* >( control ) );
    }
    const bool converted = ASSETCONVERTER::CAssetPipeline::ConvertAsset( job );
    job.report.SetCancellationCheck( GUCEF_NULL, GUCEF_NULL );
    job.report.SetProgressSink( GUCEF_NULL, GUCEF_NULL );
    if ( !converted && job.report.IsCancelled() )
    {
        result.problem = "Cancelled";
        return false;
    }

    const ASSETCONVERTER::CConversionReportEntry* entry = GUCEF_NULL;
    const ASSETCONVERTER::CConversionReport::TEntryVector& entries = job.report.GetEntries();
    for ( size_t i=0; i<entries.size(); ++i )
        if ( entries[ i ].assetPath == request.assetPath )
            entry = &entries[ i ];
    if ( GUCEF_NULL != entry )
    {
        result.warningCount = entry->GetItemCount( ASSETCONVERTER::REPORTITEM_WARNING );
        result.errorCount = entry->GetItemCount( ASSETCONVERTER::REPORTITEM_ERROR );
        for ( size_t f=0; f<entry->files.size(); ++f )
        {
            result.files.push_back( entry->files[ f ].path );
            const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( entry->files[ f ].path ).Lowercase();
            if ( result.mainFile.IsNULLOrEmpty() && ( "gltf" == extension || "glb" == extension ) )
                result.mainFile = entry->files[ f ].path;
        }
    }
    if ( !converted )
    {
        result.problem = "The conversion of " + request.assetPath + " failed";
        if ( GUCEF_NULL != entry )
            for ( size_t i=0; i<entry->items.size(); ++i )
                if ( ASSETCONVERTER::REPORTITEM_ERROR == entry->items[ i ].kind )
                {
                    result.problem += ": " + entry->items[ i ].message;
                    break;
                }
        return false;
    }
    // the next open of the asset is a cache hit: the work is done once
    // (with a cook to come the entry says so, as the pipeline does: an interrupted cook leaves the intermediate for the next request to cook)
    if ( job.converted )
        ASSETCONVERTER::CAssetPipeline::StoreCacheEntry( job, request.cook );
    result.sidecarFile = ASSETCONVERTER::CConversionSidecar::GetPath( manifest.GetIntermediatePath(), request.assetPath );
    ASSETCONVERTER::CCanonicalJson::Load( result.sidecarFile, result.sidecar );

    // effects: the cooker turns the intermediate into the scripts and the scene that Ogre reads
    bool reusedCook = false;
    if ( request.cook && job.cached && !job.cookPending && !request.force )
    {
        // the cache entry is current and lists cooked files: they are used when they are all there (the work is done once)
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CORE::CString::StringVector found;
        bool allThere = true;
        for ( size_t f=0; f<job.cacheEntry.files.size(); ++f )
        {
            if ( !job.cacheEntry.files[ f ].IsCooked() )
                continue;
            found.push_back( job.cacheEntry.files[ f ].path );
            allThere = allThere && vfs.FileExists( job.cacheEntry.files[ f ].path );
        }
        if ( allThere && !found.empty() )
        {
            for ( size_t f=0; f<found.size(); ++f )
            {
                result.files.push_back( found[ f ] );
                if ( "scene" == ASSETCONVERTER::CVfsPaths::ExtensionOf( found[ f ] ).Lowercase() && "scene" != ASSETCONVERTER::CVfsPaths::ExtensionOf( result.mainFile ).Lowercase() )
                    result.mainFile = found[ f ];
            }
            reusedCook = !result.mainFile.IsNULLOrEmpty() && "scene" == ASSETCONVERTER::CVfsPaths::ExtensionOf( result.mainFile ).Lowercase();
        }
    }
    if ( request.cook && !reusedCook )
    {
        ASSETCONVERTER::CCookSettings settings;
        settings.target = "ogre";
        settings.profile = ASSETCONVERTER::CCookProfile( "desktop" );
        settings.packId = manifest.id;
        settings.sourceRoot = context.GetSourceRoot();
        settings.lodPolicy = manifest.GetLodPolicy();
        settings.colliderPolicy = manifest.GetColliderPolicy();
        settings.force = true;
        manifest.GetPackSettings( settings.settings );
        ASSETCONVERTER::CCookSettings::SAsset asset;
        asset.assetPath = request.assetPath;
        asset.sidecarFile = result.sidecarFile;
        for ( size_t f=0; f<result.files.size(); ++f )
            asset.files.push_back( result.files[ f ] );
        settings.assets.push_back( asset );
        const CORE::CString outDir = ASSETCONVERTER::CVfsPaths::Join( manifest.GetOutputPath(), "desktop" );
        if ( !cooker->Cook( manifest.GetIntermediatePath(), outDir, settings, job.report ) )
        {
            result.problem = "The cooking of " + request.assetPath + " failed";
            return false;
        }
        result.mainFile.Clear();
        const ASSETCONVERTER::CConversionReport::TEntryVector& cooked = job.report.GetEntries();
        for ( size_t i=0; i<cooked.size(); ++i )
        {
            if ( cooked[ i ].assetPath != request.assetPath )
                continue;
            for ( size_t f=0; f<cooked[ i ].files.size(); ++f )
            {
                if ( !cooked[ i ].files[ f ].IsCooked() )
                    continue;
                result.files.push_back( cooked[ i ].files[ f ].path );
                if ( result.mainFile.IsNULLOrEmpty() && "scene" == ASSETCONVERTER::CVfsPaths::ExtensionOf( cooked[ i ].files[ f ].path ).Lowercase() )
                    result.mainFile = cooked[ i ].files[ f ].path;
            }
        }
        if ( result.mainFile.IsNULLOrEmpty() )
        {
            result.problem = "The cooker wrote no scene for " + request.assetPath;
            return false;
        }
        // the entry now lists the cooked files, with the version of the cooker in its key: the next request finds them
        const bool wasConverted = job.converted;
        job.converted = true;
        ASSETCONVERTER::CAssetPipeline::StoreCacheEntry( job, false );
        job.converted = wasConverted;
    }
    if ( result.mainFile.IsNULLOrEmpty() )
    {
        result.problem = "The conversion of " + request.assetPath + " wrote no glTF file";
        return false;
    }
    result.summary = request.converter + " converter: " + request.assetPath + ", " + CORE::UInt32ToString( result.warningCount ) + " warnings" + ( job.cached ? ", from the cache" : "" );
    result.ok = true;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
