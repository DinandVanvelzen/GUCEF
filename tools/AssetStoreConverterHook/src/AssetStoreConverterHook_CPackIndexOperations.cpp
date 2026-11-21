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

#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

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

#ifndef ASSETCONVERTERLIB_CINTERMEDIATELOCK_H
#include "AssetConverterLib_CIntermediateLock.h"
#define ASSETCONVERTERLIB_CINTERMEDIATELOCK_H
#endif /* ASSETCONVERTERLIB_CINTERMEDIATELOCK_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXBUILDER_H
#include "AssetConverterLib_CPackIndexBuilder.h"
#define ASSETCONVERTERLIB_CPACKINDEXBUILDER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXBUILDER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXWRITER_H
#include "AssetConverterLib_CPackIndexWriter.h"
#define ASSETCONVERTERLIB_CPACKINDEXWRITER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXWRITER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#include "AssetConverterLib_CPackIndexLocator.h"
#define ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKLISTING_H
#include "AssetConverterLib_CAssetPackListing.h"
#define ASSETCONVERTERLIB_CASSETPACKLISTING_H
#endif /* ASSETCONVERTERLIB_CASSETPACKLISTING_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXUPDATER_H
#include "AssetConverterLib_CPackIndexUpdater.h"
#define ASSETCONVERTERLIB_CPACKINDEXUPDATER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXUPDATER_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CLICENCERECORDBUILDER_H
#include "AssetStoreConverterHook_CLicenceRecordBuilder.h"
#define ASSETSTORECONVERTERHOOK_CLICENCERECORDBUILDER_H
#endif /* ASSETSTORECONVERTERHOOK_CLICENCERECORDBUILDER_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINECONVERTERHOOK_H
#include "AssetStoreConverterHook_CPipelineConverterHook.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINECONVERTERHOOK_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINECONVERTERHOOK_H ? */

#include "AssetStoreConverterHook_CPackIndexOperations.h"

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

/** Gives what the pipeline tells to the observer of the request */
class CIndexRequestObserver : public ASSETCONVERTER::CIAssetPipelineObserver
{
    public:

    explicit CIndexRequestObserver( ASSETSTORE::CIAssetConversionObserver* observer )
        : ASSETCONVERTER::CIAssetPipelineObserver()
        , m_observer( observer )
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

    private:

    ASSETSTORE::CIAssetConversionObserver* m_observer;
};

/** Makes the folder and the folders above it */
bool
MakeFolders( const CORE::CString& folder )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( vfs.DirExists( folder ) )
        return true;
    CORE::CString::StringVector missing;
    CORE::CString parent = folder;
    while ( !parent.IsNULLOrEmpty() && !vfs.DirExists( parent ) )
    {
        missing.push_back( parent );
        parent = ASSETCONVERTER::CVfsPaths::FolderOf( parent );
    }
    for ( size_t m=missing.size(); m>0; --m )
        if ( !vfs.CreateDir( missing[ m-1 ] ) && !vfs.DirExists( missing[ m-1 ] ) )
            return false;
    return true;
}

Int64
NowMs( void )
{
    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPackIndexOperations::Build( const CPipelineConverterHook& hook, const ASSETSTORE::CAssetPackIndexRequest& request, ASSETSTORE::CAssetPackIndexOutcome& outcome )
{GUCEF_TRACE;

    outcome = ASSETSTORE::CAssetPackIndexOutcome();
    outcome.packId = request.pack.packId;
    if ( request.derive )
        return Derive( request, outcome );

    const CConverterHookSettings settings = hook.GetSettings();
    const Int64 startedMs = NowMs();
    const CORE::CString intermediate = ASSETCONVERTER::CVfsPaths::Normalize( request.intermediateVfsFolder );
    if ( intermediate.IsNULLOrEmpty() || request.sourceVfsPath.IsNULLOrEmpty() )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT;
        outcome.message = "the request names no source or no intermediate folder";
        return outcome.status;
    }
    const CORE::CString licenceFolder = ASSETCONVERTER::CVfsPaths::Join( intermediate, "licence" );
    if ( !MakeFolders( licenceFolder ) )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_IO_ERROR;
        outcome.message = "the folder " + licenceFolder + " cannot be made";
        return outcome.status;
    }

    // the manifest of the pack for the pipeline, and its licence record: the flags of the licence go into the document
    ASSETSTORE::CAssetConversionRequest conversion;
    conversion.pack = request.pack;
    conversion.sourceVfsPath = request.sourceVfsPath;
    conversion.sourcePrefix = request.sourcePrefix;
    conversion.convertedVfsFolder = intermediate;
    conversion.cookedVfsFolder = request.cookedVfsFolder;
    ASSETCONVERTER::CAssetPackManifest manifest;
    CORE::CString problem;
    if ( !hook.BuildManifest( conversion, manifest, problem ) )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT;
        outcome.message = problem;
        return outcome.status;
    }
    ASSETCONVERTER::CLicenceRecord licence;
    CORE::CString licenceProblem;
    if ( CLicenceRecordBuilder::Build( request.pack, settings, licence, licenceProblem ) )
    {
        CORE::CDataNode licenceNode( "licence", GUCEF_DATATYPE_OBJECT );
        licence.Serialize( licenceNode );
        ASSETCONVERTER::CCanonicalJson::Store( manifest.GetLicenceRecordPath(), licenceNode );
    }

    ASSETCONVERTER::CAssetPipelineOptions options;
    options.indexOnly = true;
    options.analyse = request.analyse;
    options.reindex = request.reindex;
    options.reanalyse = request.reanalyse;
    options.threads = settings.threads;
    options.threadPoolName = settings.threadPoolName;
    options.indexFolder = intermediate;
    CIndexRequestObserver observer( request.observer );
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
        report.AddError( "AssetStoreConverterHook", request.pack.packId, "an exception ended the indexing" );
        clean = false;
    }
    const ASSETCONVERTER::CPackIndexRunResult& result = pipeline.GetLastIndexResult();
    outcome.documentPath = ASSETCONVERTER::CPackIndexLocator::GetManifestPath( result.indexFolder.IsNULLOrEmpty() ? intermediate : result.indexFolder );
    outcome.reused = result.reused;
    outcome.built = result.written;
    outcome.locked = result.locked;
    outcome.durationMs = (UInt32) ( NowMs() - startedMs );
    if ( report.IsCancelled() || result.cancelled )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_CANCELLED;
        outcome.message = "the indexing was cancelled";
        return outcome.status;
    }
    if ( !clean )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_FAILED;
        outcome.message = result.message.IsNULLOrEmpty() ? CORE::CString( "the pack index could not be built" ) : result.message;
        return outcome.status;
    }
    outcome.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
    outcome.message = result.ToSummaryText();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreConverterHook: Pack " << request.pack.packId << " index: " << outcome.message );
    return outcome.status;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPackIndexOperations::Derive( const ASSETSTORE::CAssetPackIndexRequest& request, ASSETSTORE::CAssetPackIndexOutcome& outcome )
{GUCEF_TRACE;

    outcome.packId = request.pack.packId;
    outcome.derived = true;
    const Int64 startedMs = NowMs();
    const CORE::CString indexFolder = ASSETCONVERTER::CVfsPaths::Normalize( request.intermediateVfsFolder );
    if ( indexFolder.IsNULLOrEmpty() )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT;
        outcome.message = "the request names no intermediate folder";
        return outcome.status;
    }
    if ( !MakeFolders( indexFolder ) )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_IO_ERROR;
        outcome.message = "the folder " + indexFolder + " cannot be made";
        return outcome.status;
    }

    // the listing is made from the paths and the sizes of the assets: no file of the pack is read
    ASSETCONVERTER::CAssetPackListing listing;
    listing.SetPackName( request.pack.packId );
    ASSETCONVERTER::CPackIndexBuilder builder( ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().GetKinds() );
    GUCEF::map< CORE::CString, size_t > assetOfPath;
    for ( size_t a=0; a<request.assets.size(); ++a )
    {
        const ASSETSTORE::CAssetRecord& asset = request.assets[ a ];
        CORE::CString path = asset.originalPath;
        if ( !request.sourcePrefix.IsNULLOrEmpty() && 0 == path.HasSubstr( request.sourcePrefix, true ) )
            path = path.CutChars( request.sourcePrefix.Length(), true );
        if ( path.IsNULLOrEmpty() || assetOfPath.end() != assetOfPath.find( path ) )
            continue;
        assetOfPath[ path ] = a;
        listing.AddFile( path );
        builder.SetFileSize( path, (UInt64) asset.sizeBytes );
    }
    ASSETCONVERTER::CAssetPackProfile profile;
    ASSETCONVERTER::CPackIndexBuildOptions options;
    options.packName = request.pack.packId;
    options.packTitle = request.pack.title;
    options.storePackId = request.pack.packId;
    options.tool = "AssetConverterLib";
    options.toolVersion = CORE::UInt32ToString( ASSETCONVERTER::CAssetConverterGlobal::GetConverterVersion() );
    options.outputRoot = "pack";
    options.useFileSystemSizes = false;
    for ( CORE::CString::StringSet::const_iterator r = request.pack.licence.restrictions.begin(); r != request.pack.licence.restrictions.end(); ++r )
        options.licence.restrictions.push_back( *r );
    options.licence.recordId = request.pack.licence.licenceId;
    ASSETCONVERTER::CPackIndexDocument document;
    if ( !builder.BuildStructure( listing, profile, options, document, GUCEF_NULL ) )
    {
        outcome.status = builder.GetError() == "cancelled" ? ASSETSTORE::ASSETSTORE_STATUS_CANCELLED : ASSETSTORE::ASSETSTORE_STATUS_FAILED;
        outcome.message = "the structure of the pack index could not be built: " + builder.GetError();
        return outcome.status;
    }

    // what the catalogue knows goes into the records
    for ( size_t r=0; r<document.resources.size(); ++r )
    {
        ASSETCONVERTER::CPackIndexResource& resource = document.resources[ r ];
        GUCEF::map< CORE::CString, size_t >::const_iterator found = assetOfPath.find( resource.path );
        if ( found == assetOfPath.end() )
            continue;
        const ASSETSTORE::CAssetRecord& asset = request.assets[ (*found).second ];
        resource.digest = asset.digest;
        if ( asset.model.isSet )
        {
            resource.SetStat( "triangles", (UInt64) asset.model.triangles );
            resource.SetStat( "vertices", (UInt64) asset.model.vertices );
            resource.SetStat( "subMeshes", (UInt64) asset.model.subMeshes );
            resource.SetStat( "bones", (UInt64) asset.model.bones );
            resource.SetStat( "lods", (UInt64) asset.model.lods );
            resource.SetStat( "animations", (UInt64) asset.model.animations );
        }
        if ( asset.texture.isSet )
        {
            resource.SetStat( "width", (UInt64) asset.texture.width );
            resource.SetStat( "height", (UInt64) asset.texture.height );
            if ( asset.texture.channels > 0 )
                resource.SetStat( "channels", (UInt64) asset.texture.channels );
            if ( asset.texture.bitDepth > 0 )
                resource.SetStat( "bitDepth", (UInt64) asset.texture.bitDepth );
        }
        if ( asset.animation.isSet )
        {
            resource.stats[ "lengthS" ] = CORE::CVariant( asset.animation.lengthSeconds );
            resource.stats[ "frameRate" ] = CORE::CVariant( asset.animation.frameRate );
        }
        if ( asset.audio.isSet )
        {
            resource.stats[ "lengthS" ] = CORE::CVariant( asset.audio.lengthSeconds );
            if ( asset.audio.channels > 0 )
                resource.SetStat( "channels", (UInt64) asset.audio.channels );
            if ( asset.audio.sampleRate > 0 )
                resource.SetStat( "sampleRate", (UInt64) asset.audio.sampleRate );
        }
        const bool converted = !asset.convertedPath.IsNULLOrEmpty();
        if ( converted )
        {
            resource.outputs.intermediate.path = asset.convertedPath;
            resource.status.state = ASSETCONVERTER::CPackIndexStatus::Converted;
            resource.status.SetStep( ASSETCONVERTER::CPackIndexStatus::StepConvert, ASSETCONVERTER::CPackIndexStepStatus( ASSETCONVERTER::CPackIndexStepStatus::Fresh ) );
        }
        else
        if ( asset.analysed )
            resource.status.state = ASSETCONVERTER::CPackIndexStatus::Analysed;
        if ( asset.analysed )
            resource.status.SetStep( ASSETCONVERTER::CPackIndexStatus::StepAnalyse, ASSETCONVERTER::CPackIndexStepStatus( ASSETCONVERTER::CPackIndexStepStatus::Fresh ) );
    }
    // the references: the relations of the catalogue, replaced per resource by the normalized relations of its sidecar where the
    // intermediate folder has one (pack index design 2.12); no file of the pack is read
    GUCEF::map< CORE::CString, CORE::CString > pathOfAsset;
    for ( GUCEF::map< CORE::CString, size_t >::const_iterator p = assetOfPath.begin(); p != assetOfPath.end(); ++p )
        pathOfAsset[ request.assets[ (*p).second ].assetId ] = (*p).first;
    GUCEF::map< CORE::CString, size_t > resourceOfPath;
    for ( size_t r=0; r<document.resources.size(); ++r )
        resourceOfPath[ document.resources[ r ].path ] = r;
    for ( size_t r=0; r<request.relations.size(); ++r )
    {
        const ASSETSTORE::CAssetRelationRecord& relation = request.relations[ r ];
        GUCEF::map< CORE::CString, CORE::CString >::const_iterator from = pathOfAsset.find( relation.fromAssetId );
        GUCEF::map< CORE::CString, CORE::CString >::const_iterator to = pathOfAsset.find( relation.toAssetId );
        if ( from == pathOfAsset.end() || to == pathOfAsset.end() || relation.kind.IsNULLOrEmpty() )
            continue;
        GUCEF::map< CORE::CString, size_t >::const_iterator resource = resourceOfPath.find( (*from).second );
        if ( resource == resourceOfPath.end() )
            continue;
        ASSETCONVERTER::CPackIndexReference ref;
        ref.kind = relation.kind;
        ref.path = (*to).second;
        ref.to = ASSETCONVERTER::CPackIndexResource::MakeId( request.pack.packId, ref.path );
        document.resources[ (*resource).second ].refs.push_back( ref );
    }
    UInt32 fromSidecars = 0;
    for ( size_t r=0; r<document.resources.size(); ++r )
    {
        if ( ASSETCONVERTER::CPackIndexUpdater::ReadSidecarRelations( indexFolder, request.pack.packId, document.resources[ r ] ) )
            ++fromSidecars;
        std::sort( document.resources[ r ].refs.begin(), document.resources[ r ].refs.end() );
    }
    if ( fromSidecars > 0 )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreConverterHook: Pack " << request.pack.packId << ": the relations of " << fromSidecars << " resource(s) come from their sidecars" );

    document.producer.derivedFromCatalogue = true;
    document.producer.outputRoot = "pack";
    document.AddLevel( ASSETCONVERTER::CPackIndexDocument::LevelStructure );
    document.AddLevel( ASSETCONVERTER::CPackIndexDocument::LevelAnalysis );
    document.state = ASSETCONVERTER::CPackIndexDocument::StateComplete;
    document.UpdateSummaries();

    // the document says what the pack is as the catalogue has it
    document.identity.fileCount = (UInt64) request.pack.fileCount;
    document.identity.totalBytes = (UInt64) request.pack.sizeBytes;
    document.producedUtc = CORE::CDateTime::NowUTCDateTime().ToIso8601DateTimeString( true, true );
    document.producedDurationMs = (UInt64) ( NowMs() - startedMs );

    ASSETCONVERTER::CIntermediateLock lock( ASSETCONVERTER::CPackIndexLocator::GetShardFolder( indexFolder ) );
    if ( !lock.IsHeld() )
    {
        outcome.locked = true;
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_UNAVAILABLE;
        outcome.message = "the pack index is being written by " + lock.GetHolder();
        return outcome.status;
    }
    ASSETCONVERTER::CPackIndexWriter writer;
    if ( !writer.Write( document, indexFolder ) )
    {
        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_IO_ERROR;
        outcome.message = "the pack index could not be written: " + writer.GetError();
        return outcome.status;
    }
    outcome.built = true;
    outcome.documentPath = ASSETCONVERTER::CPackIndexLocator::GetManifestPath( indexFolder );
    outcome.indexKey = document.producer.indexKey;
    outcome.documentHash = document.documentHash;
    outcome.resourceCount = (UInt32) document.resources.size();
    outcome.durationMs = (UInt32) ( NowMs() - startedMs );
    outcome.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
    outcome.message = "derived from the catalogue: " + CORE::ToString( outcome.resourceCount ) + " resource(s)";
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreConverterHook: Pack " << request.pack.packId << " index " << outcome.message );
    return outcome.status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
