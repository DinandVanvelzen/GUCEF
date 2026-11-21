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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#include "gucefASSETSTORE_CAssetNameHints.h"
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */
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
#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */
#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */
#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */
#ifndef ASSETSTORECONVERTERHOOK_CLICENCERECORDBUILDER_H
#include "AssetStoreConverterHook_CLicenceRecordBuilder.h"
#define ASSETSTORECONVERTERHOOK_CLICENCERECORDBUILDER_H
#endif /* ASSETSTORECONVERTERHOOK_CLICENCERECORDBUILDER_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPACKINDEXOPERATIONS_H
#include "AssetStoreConverterHook_CPackIndexOperations.h"
#define ASSETSTORECONVERTERHOOK_CPACKINDEXOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPACKINDEXOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CCOOKPHASELOCK_H
#include "AssetStoreConverterHook_CCookPhaseLock.h"
#define ASSETSTORECONVERTERHOOK_CCOOKPHASELOCK_H
#endif /* ASSETSTORECONVERTERHOOK_CCOOKPHASELOCK_H ? */

#include "AssetStoreConverterHook_CPipelineConverterHook.h"

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

const UInt32 MaxProblemsFromReport = 20;

/** Gives what the pipeline tells to the observer of the request, which is what the store gave */
class CRequestPipelineObserver : public ASSETCONVERTER::CIAssetPipelineObserver
{
    public:

    explicit CRequestPipelineObserver( ASSETSTORE::CIAssetConversionObserver* observer )
        : ASSETCONVERTER::CIAssetPipelineObserver()
        , m_observer( observer )
        , m_cookLock()
    {

    }

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NULL != m_observer && m_observer->IsCancelRequested();
    }

    virtual void OnAssetFinished( UInt32 done, UInt32 total, const CORE::CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_observer )
            m_observer->OnConversionProgress( done, total, assetPath );
    }

    virtual void OnPhase( ASSETCONVERTER::TPipelinePhase phase ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_cookLock.OnPhase( phase );
    }

    private:

    ASSETSTORE::CIAssetConversionObserver* m_observer;
    CCookPhaseLock m_cookLock;
};

ASSETCONVERTER::CAssetPluginRegistry&
Plugins( void )
{
    return ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry();
}

CORE::CString
AbsoluteVfsPath( const CORE::CString& vfsPath )
{
    return "/" + ASSETCONVERTER::CVfsPaths::Normalize( vfsPath );
}

/** The converted file of the asset: the first extension of the list among its cooked files, else among its intermediate files */
bool
PickMainFile( const ASSETCONVERTER::CConversionReportEntry& entry, const CConverterHookSettings& settings, const CORE::CString& convertedFolder, const CORE::CString& cookedFolder, CORE::CString& relativePath, bool& inCookedFolder )
{
    for ( UInt32 pass=0; pass<2; ++pass )
    {
        for ( size_t e=0; e<settings.mainExtensions.size(); ++e )
        {
            for ( size_t f=0; f<entry.files.size(); ++f )
            {
                const ASSETCONVERTER::CConversionReportFile& file = entry.files[ f ];
                if ( file.IsCooked() != ( 0 == pass ) )
                    continue;
                if ( ASSETCONVERTER::CVfsPaths::ExtensionOf( file.path ) != settings.mainExtensions[ e ].Lowercase() )
                    continue;
                inCookedFolder = false;
                if ( !cookedFolder.IsNULLOrEmpty() && file.IsCooked() && ASSETCONVERTER::CVfsPaths::MakeRelative( cookedFolder, file.path, relativePath ) )
                {
                    inCookedFolder = true;
                    return true;
                }
                if ( ASSETCONVERTER::CVfsPaths::MakeRelative( convertedFolder, file.path, relativePath ) )
                    return true;
            }
        }
    }
    return false;
}

/** What a run that failed leaves in the intermediate folder goes, but the licence record, the manifest and the report */
void
RemoveWorkOfFailedRun( const CORE::CString& folder )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TStringVector names;
    vfs.GetDirList( names, folder, false, false, CORE::CString::Empty );
    for ( size_t n=0; n<names.size(); ++n )
    {
        const CORE::CString name = ASSETCONVERTER::CVfsPaths::NameOf( names[ n ] );
        if ( "licence" != name )
            vfs.DeleteDir( ASSETCONVERTER::CVfsPaths::Join( folder, name ), true );
    }
    names.clear();
    vfs.GetFileList( names, folder, false, false, CORE::CString::Empty, GUCEF_UINT32MAX );
    for ( size_t n=0; n<names.size(); ++n )
    {
        const CORE::CString name = ASSETCONVERTER::CVfsPaths::NameOf( names[ n ] );
        if ( "pack.assetpack.json" != name && "report.json" != name && "report.txt" != name )
            vfs.DeleteFile( ASSETCONVERTER::CVfsPaths::Join( folder, name ), true );
    }
}

/** The versions of the parts of the converter that made the conversion, "key/version" */
void
CollectConverters( const ASSETCONVERTER::CConversionReport& report, const CConverterHookSettings& settings, CORE::CString::StringSet& converters )
{
    CORE::CString::StringSet keys;
    keys.insert( "converter:AssetConverterLib" );
    keys.insert( "exporter:" + settings.exporter );
    if ( !settings.cookTarget.IsNULLOrEmpty() )
        keys.insert( "cooker:" + settings.cookTarget );
    const ASSETCONVERTER::CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
    {
        if ( entries[ e ].importer.IsNULLOrEmpty() )
            continue;
        keys.insert( "importer:" + entries[ e ].importer );
        ASSETCONVERTER::TAssetImporterPtr importer;
        if ( Plugins().TryGetImporter( entries[ e ].importer, importer ) )
        {
            CORE::CString::StringVector processors;
            importer->GetDefaultPostProcessors( processors );
            for ( size_t p=0; p<processors.size(); ++p )
                keys.insert( "postProcessor:" + processors[ p ] );
        }
    }
    for ( CORE::CString::StringSet::const_iterator k=keys.begin(); k!=keys.end(); ++k )
    {
        UInt32 version = 0;
        if ( ASSETCONVERTER::CAssetPipeline::GetCurrentPluginVersion( (*k), version ) )
            converters.insert( ASSETSTORE::CAssetConversionRecord::MakeConverterEntry( (*k), version ) );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPipelineConverterHook::CPipelineConverterHook( void )
    : ASSETSTORE::CIAssetConverterHook()
    , m_settings()
    , m_lock()
    , m_operations()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPipelineConverterHook::CPipelineConverterHook( const CConverterHookSettings& settings )
    : ASSETSTORE::CIAssetConverterHook()
    , m_settings( settings )
    , m_lock()
    , m_operations()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPipelineConverterHook::~CPipelineConverterHook()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPipelineConverterHook::SetSettings( const CConverterHookSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

CConverterHookSettings
CPipelineConverterHook::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPipelineConverterHook::GetName( void ) const
{GUCEF_TRACE;

    return "AssetConverterLib pipeline";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPipelineConverterHook::GetProfileName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings.cookProfile.name;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineConverterHook::CanConvert( const ASSETSTORE::CAssetRecord& asset ) const
{GUCEF_TRACE;

    const CConverterHookSettings settings = GetSettings();
    if ( settings.convertibleKinds.end() == settings.convertibleKinds.find( asset.kind ) )
    {
        // a package of an engine (an Unreal .uasset or .umap: the store has no kind of its own for it) that an importer declares
        // as a package: that importer reads the package into the intermediate
        if ( asset.originalPath.IsNULLOrEmpty() || ASSETCONVERTER::ASSETKIND_PACKAGE != Plugins().GetKinds().GetKindOfPath( asset.originalPath ) )
            return false;
    }

    // an importer that is not loaded cannot convert anything
    GUCEF::vector< ASSETCONVERTER::TAssetImporterPtr > importers;
    Plugins().GetImporters( importers );
    return !importers.empty();
}

/*-------------------------------------------------------------------------*/

bool
CPipelineConverterHook::BuildManifest( const ASSETSTORE::CAssetConversionRequest& request, ASSETCONVERTER::CAssetPackManifest& manifest, CORE::CString& problem ) const
{GUCEF_TRACE;

    const CConverterHookSettings settings = GetSettings();
    const CORE::CString convertedFolder = ASSETCONVERTER::CVfsPaths::Normalize( request.convertedVfsFolder );
    if ( convertedFolder.IsNULLOrEmpty() || request.sourceVfsPath.IsNULLOrEmpty() )
    {
        problem = "the request names no source or no folder for the converted files";
        return false;
    }
    manifest = ASSETCONVERTER::CAssetPackManifest();
    manifest.id = request.pack.packId;
    manifest.title = request.pack.title;
    manifest.store = request.pack.sourceId;
    manifest.licenceRecord = CLicenceRecordBuilder::GetRecordId( request.pack );
    manifest.licenceFolder = AbsoluteVfsPath( ASSETCONVERTER::CVfsPaths::Join( convertedFolder, "licence" ) );
    manifest.source = AbsoluteVfsPath( request.sourceVfsPath );
    manifest.importer = settings.importer.IsNULLOrEmpty() ? ASSETCONVERTER::CAssetPackManifest::AutoImporter : settings.importer;
    manifest.exporter = settings.exporter;
    const bool cookedApart = !request.cookedVfsFolder.IsNULLOrEmpty();
    if ( cookedApart )
    {
        // the intermediate files are the folder of the conversion, the cooked files of each profile a folder of the cooked area
        manifest.intermediate = AbsoluteVfsPath( convertedFolder );
        manifest.output = AbsoluteVfsPath( request.cookedVfsFolder );
    }
    else
    {
        manifest.intermediate = AbsoluteVfsPath( ASSETCONVERTER::CVfsPaths::Join( convertedFolder, "intermediate" ) );

        // the cooked files of the profile go into the folder of the conversion itself: <out>/<profile> is that folder
        const CORE::CString parent = ASSETCONVERTER::CVfsPaths::FolderOf( convertedFolder );
        const bool profileFolder = ASSETCONVERTER::CVfsPaths::NameOf( convertedFolder ) == settings.cookProfile.name;
        manifest.output = AbsoluteVfsPath( profileFolder && !parent.IsNULLOrEmpty() ? parent : convertedFolder );
    }
    manifest.manifestFolder = convertedFolder;
    if ( !settings.cookTarget.IsNULLOrEmpty() )
        manifest.cookTargets.push_back( settings.cookTarget );
    manifest.cookProfiles.push_back( settings.cookProfile );
    return true;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineConverterHook::ConvertPack( const ASSETSTORE::CAssetConversionRequest& request ,
                                     ASSETSTORE::TAssetRecordVector& assets             ,
                                     ASSETSTORE::CAssetConversionOutcome& outcome       )
{GUCEF_TRACE;

    const CConverterHookSettings settings = GetSettings();
    outcome.packId = request.pack.packId;
    outcome.record = ASSETSTORE::CAssetConversionRecord();
    outcome.record.profile = settings.cookProfile.name;
    const CORE::CString convertedFolder = ASSETCONVERTER::CVfsPaths::Normalize( request.convertedVfsFolder );

    // the licence decides first: nothing is written for a pack that is refused
    ASSETCONVERTER::CLicenceRecord licence;
    CORE::CString reason;
    if ( !CLicenceRecordBuilder::Build( request.pack, settings, licence, reason ) || !licence.AllowsConversion( reason ) )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_REFUSED;
        outcome.message = reason;
        outcome.record.reason = reason;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreConverterHook: Pack " << request.pack.packId << " is not converted: " << reason );
        return outcome.status;
    }

    // the folders, the licence record and the manifest
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString licenceFolder = ASSETCONVERTER::CVfsPaths::Join( convertedFolder, "licence" );
    CORE::CString::StringVector folders;
    folders.push_back( convertedFolder );
    folders.push_back( licenceFolder );
    for ( size_t f=0; f<folders.size(); ++f )
    {
        CORE::CString parent = ASSETCONVERTER::CVfsPaths::FolderOf( folders[ f ] );
        CORE::CString::StringVector missing;
        while ( !parent.IsNULLOrEmpty() && !vfs.DirExists( parent ) )
        {
            missing.push_back( parent );
            parent = ASSETCONVERTER::CVfsPaths::FolderOf( parent );
        }
        for ( size_t m=missing.size(); m>0; --m )
            vfs.CreateDir( missing[ m-1 ] );
        if ( !vfs.DirExists( folders[ f ] ) && !vfs.CreateDir( folders[ f ] ) )
        {
            outcome.status = ASSETSTORE::ASSETSTORE_STATUS_IO_ERROR;
            outcome.message = "the folder " + folders[ f ] + " cannot be made";
            return outcome.status;
        }
    }
    CORE::CDataNode licenceNode( "licence", GUCEF_DATATYPE_OBJECT );
    licence.Serialize( licenceNode );
    ASSETCONVERTER::CAssetPackManifest manifest;
    CORE::CString problem;
    if ( !BuildManifest( request, manifest, problem ) )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT;
        outcome.message = problem;
        return outcome.status;
    }
    CORE::CDataNode manifestNode( "assetpack", GUCEF_DATATYPE_OBJECT );
    manifest.Serialize( manifestNode );
    // a cook only run reads the intermediate of the pack and writes the cooked files elsewhere: the manifest and the licence
    // record of the intermediate stay as the conversion of the intermediate wrote them
    if ( !request.cookOnly &&
         ( !ASSETCONVERTER::CCanonicalJson::Store( manifest.GetLicenceRecordPath(), licenceNode ) ||
           !ASSETCONVERTER::CCanonicalJson::Store( ASSETCONVERTER::CVfsPaths::Join( convertedFolder, "pack.assetpack.json" ), manifestNode ) ) )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_IO_ERROR;
        outcome.message = "the licence record and the manifest cannot be written in " + convertedFolder;
        return outcome.status;
    }

    // the run
    ASSETCONVERTER::CAssetPipelineOptions options;
    options.reimport = request.reconvert && !request.cookOnly;
    // a reprocessing job of the service gives its share of the threads and a pool of its own; other runs use the settings
    options.threads = request.maxThreads > 0 ? request.maxThreads : settings.threads;
    options.threadPoolName = request.threadPoolName.IsNULLOrEmpty() ? settings.threadPoolName : request.threadPoolName;
    options.reportPath = request.reportVfsPath.IsNULLOrEmpty() ? ASSETCONVERTER::CVfsPaths::Join( convertedFolder, "report.json" ) : request.reportVfsPath;
    options.assetGlobs = request.assetGlobs;
    options.noCook = request.noCook && !request.cookOnly;
    if ( request.cookOnly )
    {
        options.mode = ASSETCONVERTER::PIPELINEMODE_COOK_ONLY;
        options.noIndex = true;
    }
    CRequestPipelineObserver observer( request.observer );
    options.observer = &observer;
    ASSETCONVERTER::CAssetPipeline pipeline;
    ASSETCONVERTER::CConversionReport report;
    bool clean = false;
    try
    {
        clean = pipeline.RunPack( manifest, options, report );
    }
    catch ( ... )
    {
        report.AddError( "AssetStoreConverterHook", request.pack.packId, "an exception ended the conversion" );
        clean = false;
    }

    // the counts and the versions, whatever the outcome
    outcome.record.reportPath = request.convertedPathPrefix + "/report.json";
    outcome.record.convertedCount = report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_CONVERTED );
    outcome.record.approximatedCount = report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_APPROXIMATED );
    outcome.record.cachedCount = report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_CACHED );
    outcome.record.skippedCount = report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_SKIPPED );
    outcome.record.failedCount = report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_FAILED );
    CollectConverters( report, settings, outcome.record.converters );

    if ( report.IsCancelled() )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_CANCELLED;
        outcome.message = "the conversion was cancelled";
        return outcome.status;
    }
    if ( report.IsRefused() )
    {
        // the pipeline's own check agrees with ours, unless the record changed in between
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_REFUSED;
        outcome.message = report.GetRefusalReason();
        outcome.record.reason = outcome.message;
        return outcome.status;
    }
    bool failedRun = false;
    if ( !clean )
    {
        // the errors of the report, a few of them, then what the run wrote goes unless it would take an earlier conversion with it
        failedRun = true;
        UInt32 errors = 0;
        const ASSETCONVERTER::CConversionReport::TEntryVector& entries = report.GetEntries();
        for ( size_t e=0; e<entries.size(); ++e )
        {
            for ( size_t i=0; i<entries[ e ].items.size(); ++i )
            {
                if ( ASSETCONVERTER::REPORTITEM_ERROR != entries[ e ].items[ i ].kind )
                    continue;
                if ( errors < MaxProblemsFromReport )
                    outcome.problems.push_back( ( entries[ e ].assetPath.IsNULLOrEmpty() ? CORE::CString( "pack" ) : entries[ e ].assetPath ) + ": " + entries[ e ].items[ i ].message );
                ++errors;
            }
        }
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_FAILED;
        outcome.message = CORE::ToString( errors ) + " error(s) in the conversion report" + ( outcome.problems.empty() ? CORE::CString::Empty : ", the first: " + outcome.problems[ 0 ] );
        if ( request.keepPartialResult )
        {
            // the caller takes what converted: the assets are mapped below, nothing is taken away
        }
        else
        if ( ASSETSTORE::CAssetConversionRecord::OutcomeConverted != request.pack.conversion.outcome )
        {
            if ( request.cookedVfsFolder.IsNULLOrEmpty() )
            {
                vfs.DeleteDir( ASSETCONVERTER::CVfsPaths::Join( convertedFolder, "intermediate" ), true );
                vfs.DeleteDir( ASSETCONVERTER::CVfsPaths::Join( convertedFolder, request.pack.packId ), true );
                vfs.DeleteFile( ASSETCONVERTER::CVfsPaths::Join( convertedFolder, "references.json" ), true );
                vfs.DeleteFile( ASSETCONVERTER::CVfsPaths::Join( convertedFolder, "sizes.json" ), true );
            }
            else
            {
                // the intermediate folder holds the licence record, the manifest and the report as well: they stay
                RemoveWorkOfFailedRun( convertedFolder );
                const CORE::CString profileFolder = ASSETCONVERTER::CVfsPaths::Join( ASSETCONVERTER::CVfsPaths::Normalize( request.cookedVfsFolder ), settings.cookProfile.name );
                vfs.DeleteDir( ASSETCONVERTER::CVfsPaths::Join( profileFolder, request.pack.packId ), true );
                vfs.DeleteFile( ASSETCONVERTER::CVfsPaths::Join( profileFolder, "references.json" ), true );
                vfs.DeleteFile( ASSETCONVERTER::CVfsPaths::Join( profileFolder, "sizes.json" ), true );
            }
        }
        if ( !request.keepPartialResult )
            return outcome.status;
    }

    const CORE::CString cookedFolder = ASSETCONVERTER::CVfsPaths::Normalize( request.cookedVfsFolder );
    // the assets that converted: their main file and its format
    UInt32 mapped = 0;
    for ( size_t a=0; a<assets.size(); ++a )
    {
        assets[ a ].convertedPath.Clear();
        assets[ a ].convertedFormat.Clear();
        const CORE::CString sourcePath = request.GetSourceRelativePath( assets[ a ] );
        if ( sourcePath.IsNULLOrEmpty() )
            continue;
        const ASSETCONVERTER::CConversionReportEntry* entry = report.FindEntry( sourcePath );
        if ( GUCEF_NULL == entry )
            continue;
        const ASSETCONVERTER::TAssetStatus status = entry->GetStatus();
        if ( ASSETCONVERTER::ASSETSTATUS_CONVERTED != status && ASSETCONVERTER::ASSETSTATUS_APPROXIMATED != status && ASSETCONVERTER::ASSETSTATUS_CACHED != status )
            continue;
        CORE::CString relative;
        bool inCookedFolder = false;
        if ( !PickMainFile( *entry, settings, convertedFolder, cookedFolder, relative, inCookedFolder ) )
            continue;
        assets[ a ].convertedPath = ( inCookedFolder ? request.cookedPathPrefix : request.convertedPathPrefix ) + "/" + relative;
        assets[ a ].convertedFormat = ASSETSTORE::CAssetNameHints::FormatFromExtension( ASSETCONVERTER::CVfsPaths::ExtensionOf( relative ) );
        ++mapped;
    }
    if ( outcome.record.approximatedCount > 0 )
        outcome.problems.push_back( CORE::ToString( outcome.record.approximatedCount ) + " asset(s) converted with approximations, see the conversion report" );
    if ( outcome.record.skippedCount > 0 )
        outcome.problems.push_back( CORE::ToString( outcome.record.skippedCount ) + " asset(s) skipped by the converter, see the conversion report" );
    if ( failedRun )
    {
        // a run with errors whose result the caller keeps: the outcome stays FAILED with the assets that converted mapped
        outcome.problems.push_back( CORE::ToString( mapped ) + " asset(s) converted in the run with errors" );
        return outcome.status;
    }
    outcome.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
    outcome.message = CORE::ToString( mapped ) + " asset(s) converted";
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreConverterHook: Pack " << request.pack.packId << ": " << outcome.message );
    return outcome.status;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineConverterHook::IsConversionStale( const ASSETSTORE::CAssetConversionRecord& recorded, CORE::CString& reason ) const
{GUCEF_TRACE;

    const CConverterHookSettings settings = GetSettings();
    bool hasCooker = false;
    for ( CORE::CString::StringSet::const_iterator e=recorded.converters.begin(); e!=recorded.converters.end(); ++e )
    {
        CORE::CString key;
        UInt32 recordedVersion = 0;
        if ( !ASSETSTORE::CAssetConversionRecord::ParseConverterEntry( (*e), key, recordedVersion ) )
            continue;
        if ( 0 == key.HasSubstr( "cooker:", true ) )
            hasCooker = true;
        UInt32 currentVersion = 0;
        if ( !ASSETCONVERTER::CAssetPipeline::GetCurrentPluginVersion( key, currentVersion ) )
        {
            reason = key + " that made the conversion (version " + CORE::ToString( recordedVersion ) + ") is not loaded";
            return true;
        }
        if ( currentVersion != recordedVersion )
        {
            reason = key + " is at version " + CORE::ToString( currentVersion ) + ", the conversion was made with version " + CORE::ToString( recordedVersion );
            return true;
        }
    }
    if ( !settings.cookTarget.IsNULLOrEmpty() && !hasCooker )
    {
        reason = "the cook target " + settings.cookTarget + " was added since the conversion";
        return true;
    }
    if ( settings.cookTarget.IsNULLOrEmpty() && hasCooker )
    {
        reason = "the conversion has cooked files, the settings have no cook target now";
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPipelineConverterHook::SetOperationSettings( const CPipelineOperationSettings& settings )
{GUCEF_TRACE;

    m_operations.SetSettings( settings );
}

/*-------------------------------------------------------------------------*/

CPipelineOperationSettings
CPipelineConverterHook::GetOperationSettings( void ) const
{GUCEF_TRACE;

    return m_operations.GetSettings();
}

/*-------------------------------------------------------------------------*/

const CPipelineOperationRunner&
CPipelineConverterHook::GetOperationRunner( void ) const
{GUCEF_TRACE;

    return m_operations;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineConverterHook::SupportsPipelineOperation( const CORE::CString& operation ) const
{GUCEF_TRACE;

    return m_operations.IsSupported( operation );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineConverterHook::RunPipelineOperation( const ASSETSTORE::CAssetPipelineRequest& request    ,
                                              ASSETSTORE::CIAssetConversionObserver* observer     ,
                                              ASSETSTORE::CAssetServiceDocument& result           )
{GUCEF_TRACE;

    const CConverterHookSettings settings = GetSettings();
    return m_operations.Run( request, observer, result, settings.threadPoolName, settings.threads );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineConverterHook::BuildPackIndex( const ASSETSTORE::CAssetPackIndexRequest& request ,
                                        ASSETSTORE::CAssetPackIndexOutcome& outcome       )
{GUCEF_TRACE;

    return CPackIndexOperations::Build( *this, request, outcome );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
