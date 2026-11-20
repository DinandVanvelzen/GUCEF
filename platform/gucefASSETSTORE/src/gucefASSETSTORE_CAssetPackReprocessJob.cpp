/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include <algorithm>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKREGENERATION_H
#include "gucefASSETSTORE_CAssetPackRegeneration.h"
#define GUCEF_ASSETSTORE_CASSETPACKREGENERATION_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKREGENERATION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H
#include "gucefASSETSTORE_CAssetPackGenerations.h"
#define GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONREPORTSUMMARY_H
#include "gucefASSETSTORE_CAssetConversionReportSummary.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONREPORTSUMMARY_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONREPORTSUMMARY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H
#include "gucefASSETSTORE_CAssetLibrarySyncAdmission.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETITEMREVISION_H
#include "gucefASSETSTORE_CAssetItemRevision.h"
#define GUCEF_ASSETSTORE_CASSETITEMREVISION_H
#endif /* GUCEF_ASSETSTORE_CASSETITEMREVISION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#include "gucefASSETSTORE_CAssetIngestRequest.h"
#define GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTRESULT_H
#include "gucefASSETSTORE_CAssetIngestResult.h"
#define GUCEF_ASSETSTORE_CASSETINGESTRESULT_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTRESULT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H
#include "gucefASSETSTORE_CAssetPackSourceRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H
#include "gucefASSETSTORE_CAssetSyncMarkStore.h"
#define GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H
#endif /* GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXOPTIONS_H
#include "gucefASSETSTORE_CAssetPackIndexOptions.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXOPTIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXOPTIONS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H
#include "gucefASSETSTORE_CAssetPackIndexOutcome.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H ? */
#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */

#include "gucefASSETSTORE_CAssetPackReprocessJob.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

Int64
NowUtcMs( void )
{GUCEF_TRACE;

    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

CORE::CString
HexOf( UInt64 value, UInt32 digits )
{GUCEF_TRACE;

    static const char hexDigits[] = "0123456789abcdef";
    CORE::CString text;
    for ( Int32 d=(Int32) digits-1; d>=0; --d )
        text += hexDigits[ ( value >> ( d * 4 ) ) & 0xF ];
    return text;
}

bool
IsSessionStatus( TAssetStoreStatus status )
{GUCEF_TRACE;

    return ASSETSTORE_STATUS_NOT_SIGNED_IN == status || ASSETSTORE_STATUS_SESSION_EXPIRED == status;
}

CORE::CString
DescribeBytes( UInt64 bytes )
{GUCEF_TRACE;

    if ( bytes >= 1024ULL * 1024 * 1024 )
        return CORE::ToString( (UInt64) ( bytes / ( 1024ULL * 1024 * 1024 ) ) ) + "." + CORE::ToString( (UInt64) ( ( bytes % ( 1024ULL * 1024 * 1024 ) ) * 10 / ( 1024ULL * 1024 * 1024 ) ) ) + " GiB";
    if ( bytes >= 1024ULL * 1024 )
        return CORE::ToString( (UInt64) ( bytes / ( 1024ULL * 1024 ) ) ) + " MiB";
    return CORE::ToString( bytes ) + " bytes";
}

CORE::CString
CountsText( const CORE::CDataNode* counts )
{GUCEF_TRACE;

    if ( GUCEF_NULL == counts || !counts->GetAttributeValueOrChildValueByName( "found" ).AsBool( false, true ) )
        return "no report";
    CORE::CString text;
    static const char* const names[] = { "converted", "approximated", "skipped", "failed", "cached", "refused" };
    for ( UInt32 i=0; i<6; ++i )
    {
        const UInt32 count = counts->GetAttributeValueOrChildValueByName( names[ i ] ).AsUInt32( 0, true );
        if ( count > 0 )
            text += ( text.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( ", " ) ) + CORE::ToString( count ) + " " + names[ i ];
    }
    return text.IsNULLOrEmpty() ? CORE::CString( "no assets" ) : text;
}

/** The summary of a regeneration report in words */
CORE::CString
DescribeRegeneration( const char* what, const CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CString text = CORE::CString( what ) + ": ";
    const CORE::CString status = node.GetAttributeValueOrChildValueByName( "status" ).AsString( CORE::CString::Empty, true );
    if ( "OK" != status && !status.IsNULLOrEmpty() )
        text += status + " (" + node.GetAttributeValueOrChildValueByName( "message" ).AsString( CORE::CString::Empty, true ) + "); ";
    text += "before " + CountsText( node.FindChild( "before" ) ) + "; after " + CountsText( node.FindChild( "after" ) );
    const CORE::CDataNode* changes = node.FindChild( "changes" );
    if ( GUCEF_NULL != changes )
        text += "; " + changes->GetAttributeValueOrChildValueByName( "improvedCount" ).AsString( "0", true ) + " improved, " +
                changes->GetAttributeValueOrChildValueByName( "regressedCount" ).AsString( "0", true ) + " regressed, " +
                changes->GetAttributeValueOrChildValueByName( "stillFailingCount" ).AsString( "0", true ) + " still failing";
    text += node.GetAttributeValueOrChildValueByName( "replaced" ).AsBool( false, true ) ? CORE::CString( "; put in place" ) : CORE::CString( "; nothing put in place" );
    return text;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPackReprocessJob::CAssetPackReprocessJob( CLocalAssetStoreService& service, const CORE::CString& jobId, const CAssetReprocessRequest& request,
                                                CIAssetConversionObserver* observer )
    : m_service( service )
    , m_jobId( jobId )
    , m_request( request )
    , m_observer( observer )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPackReprocessJob::~CAssetPackReprocessJob()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetPackReprocessJob::IsCancelled( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_observer && m_observer->IsCancelRequested();
}

/*-------------------------------------------------------------------------*/

void
CAssetPackReprocessJob::Progress( UInt32 done, UInt32 total, const CORE::CString& text )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_observer )
        m_observer->OnConversionProgress( done, total, text );
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode
CAssetPackReprocessJob::MakeMade( const CORE::CString& step ) const
{GUCEF_TRACE;

    CORE::CDataNode made( "made", GUCEF_DATATYPE_OBJECT );
    made.SetAttribute( "operation", m_request.operation );
    made.SetAttribute( "step", step );
    made.SetAttribute( "jobId", m_jobId );
    made.SetAttribute( "caller", m_request.caller );
    made.SetAttribute( "machine", m_request.machine );
    made.SetAttribute( "atMs", NowUtcMs() );
    made.SetAttribute( "arguments", m_request.DescribeForAudit() );
    return made;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackReprocessJob::WriteHistory( const CORE::CString& packId, const CORE::CString& step, const CORE::CString& outcome, const CORE::CString& problems, Int64 startedAtMs )
{GUCEF_TRACE;

    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    if ( GUCEF_NULL == store )
        return;
    CAssetPackRecord pack;
    store->GetPack( packId, pack );
    CAssetCatalogue::TRowVector rows( 1 );
    CAssetCatalogue::TRow& row = rows[ 0 ];
    row[ "entry_id" ] = CORE::CVariant( "ih" + HexOf( (UInt64) NowUtcMs(), 11 ) + HexOf( CORE::CRandom::GetRandomUInt32(), 8 ) );
    row[ "ingest_id" ] = CORE::CVariant( m_jobId );
    row[ "pack_id" ] = CORE::CVariant( packId );
    row[ "source_id" ] = CORE::CVariant( pack.sourceId );
    row[ "origin" ] = CORE::CVariant( "reprocess:" + step );
    row[ "policy" ] = CORE::CVariant( m_request.DescribeForAudit() );
    row[ "started_ms" ] = CORE::CVariant( startedAtMs );
    row[ "duration_ms" ] = CORE::CVariant( NowUtcMs() - startedAtMs );
    row[ "outcome" ] = CORE::CVariant( outcome );
    row[ "step" ] = CORE::CVariant( step );
    row[ "problems" ] = CORE::CVariant( problems );
    MT::CScopeMutex indexLock( store->GetIndexLock() );
    if ( ASSETSTORE_STATUS_OK != store->GetCatalogue().Write( "ingest_history", rows ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackReprocessJob: " << m_jobId << " could not write its audit entry of pack " << packId );
}

/*-------------------------------------------------------------------------*/

void
CAssetPackReprocessJob::ReloadIndex( const CORE::CString& packId )
{GUCEF_TRACE;

    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    if ( GUCEF_NULL == store )
        return;
    // the document the regenerated intermediate holds is loaded; nothing is built here (the job index-pack does that)
    CAssetPackIndexOptions options;
    options.allowBuild = false;
    options.notify = true;
    CAssetPackIndexOutcome outcome;
    store->IndexPack( packId, options, outcome );
    m_service.RecordChange( ASSETCHANGE_PACK_UPDATED, packId );
    m_service.RecordChange( ASSETCHANGE_PACK_INDEX_UPDATED, packId );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackReprocessJob::ResolveScope( const CORE::CString& packId, CORE::CString::StringVector& globs, CORE::CDataNode& scope, CORE::CString& message )
{GUCEF_TRACE;

    globs.clear();
    scope.SetAttribute( "partial", m_request.IsPartial() );
    if ( !m_request.IsPartial() )
        return ASSETSTORE_STATUS_OK;
    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    CAssetPackRecord pack;
    if ( GUCEF_NULL == store || ASSETSTORE_STATUS_OK != store->GetPack( packId, pack ) )
    {
        message = "there is no pack " + packId + " in the catalogue";
        return ASSETSTORE_STATUS_NOT_FOUND;
    }
    const CPackStorageLayout layout( pack.rootPath );
    const CORE::CString prefix = layout.GetOriginalPrefix() + "/";

    // the paths and globs named, and the assets named by id
    CORE::CString::StringVector named = m_request.assets;
    for ( size_t i=0; i<m_request.storeAssetIds.size(); ++i )
    {
        CAssetRecord asset;
        if ( ASSETSTORE_STATUS_OK != m_service.GetAsset( m_request.storeAssetIds[ i ], asset ) || asset.packId != packId )
        {
            message = "the asset " + m_request.storeAssetIds[ i ] + " is not an asset of pack " + packId;
            return ASSETSTORE_STATUS_NOT_FOUND;
        }
        CORE::CString path = asset.originalPath;
        if ( 0 == path.HasSubstr( prefix, true ) )
            path = path.CutChars( prefix.Length(), true );
        named.push_back( path );
    }

    // the filters: the status in the last report, the kind in the catalogue
    bool filtered = false;
    CORE::CString::StringVector candidates;
    if ( !m_request.onlyStatuses.empty() )
    {
        filtered = true;
        CAssetConversionReportSummary report;
        const CORE::CString reportPath = CPackStorageLayout::Join( layout.GetIntermediateFolder( store->GetLocationRoot( pack.locationId ), CORE::CString::Empty ), "report.json" );
        report.Load( reportPath );
        report.GetAssetsWithStatus( m_request.onlyStatuses, candidates );
        scope.SetAttribute( "report", reportPath );
    }
    if ( !m_request.kinds.empty() )
    {
        TAssetRecordVector assets;
        store->GetAssets( packId, assets );
        CORE::CString::StringVector ofKinds;
        for ( size_t a=0; a<assets.size(); ++a )
        {
            if ( m_request.kinds.end() == m_request.kinds.find( assets[ a ].kind.Lowercase() ) )
                continue;
            CORE::CString path = assets[ a ].originalPath;
            if ( 0 == path.HasSubstr( prefix, true ) )
                path = path.CutChars( prefix.Length(), true );
            if ( !filtered || candidates.end() != std::find( candidates.begin(), candidates.end(), path ) )
                ofKinds.push_back( path );
        }
        candidates = ofKinds;
        filtered = true;
    }
    if ( !filtered )
        globs = named;
    else
    {
        for ( size_t c=0; c<candidates.size(); ++c )
            if ( named.empty() || CAssetConversionRequest::IsInScope( named, candidates[ c ] ) )
                globs.push_back( candidates[ c ] );
    }
    scope.SetAttribute( "assets", (UInt32) globs.size() );
    CORE::CString::StringVector sample( globs.begin(), globs.size() > 20 ? globs.begin() + 20 : globs.end() );
    scope.SetValuesOfChildByName( "sample", sample );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackReprocessJob::IsStoreSyncRunning( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_service.m_lock );
    for ( size_t i=0; i<m_service.m_pipelineJobs.size(); ++i )
    {
        const CLocalAssetStoreService::SPipelineJob& job = m_service.m_pipelineJobs[ i ];
        if ( CAssetPipelineOperations::SyncLibrary == job.request.operation && storeId.Equals( job.request.storeId, false ) && !AssetJobStateIsFinal( job.state ) &&
             !CAssetLibrarySyncSpec::FromRequest( job.request ).IsPlanOnly() )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackReprocessJob::FindStoreSource( const CAssetPackRecord& pack, CORE::CString& storeId, CORE::CString& itemId, CORE::CString& formatId,
                                         CORE::CString& outcome, CORE::CString& message )
{GUCEF_TRACE;

    storeId = pack.sourceId.Lowercase();
    itemId = pack.sourceItemId;
    formatId.Clear();
    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    if ( GUCEF_NULL != store && store->GetSyncStore().IsAvailable() )
    {
        CAssetPackSourceRecord facts;
        if ( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetPackSource( pack.packId, facts ) )
        {
            if ( !facts.IsCurrent() )
            {
                outcome = "superseded";
                message = "the pack is superseded by pack " + facts.supersededBy + ": reprocess that one";
                return ASSETSTORE_STATUS_INVALID_ARGUMENT;
            }
            storeId = facts.sourceId;
            itemId = facts.sourceItemId;
            formatId = facts.sourceFormatId;
        }
    }
    if ( storeId.IsNULLOrEmpty() || CPrimaryAssetStoreSettings::DiskSourceId == storeId || itemId.IsNULLOrEmpty() )
    {
        outcome = "no-store-source";
        message = "no store source: cannot re-download (the pack came from an upload or a local folder, not from a store)";
        return ASSETSTORE_STATUS_UNSUPPORTED;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetPackReprocessJob::ReadFreeBytes( const CAssetPackRecord& pack, bool& known ) const
{GUCEF_TRACE;

    known = false;
    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    if ( GUCEF_NULL == store )
        return 0;
    const CPrimaryAssetStoreSettings settings = store->GetSettings();
    for ( size_t l=0; l<settings.locations.size(); ++l )
    {
        if ( settings.locations[ l ].locationId != pack.locationId || settings.locations[ l ].fileSystemPath.IsNULLOrEmpty() )
            continue;
        CORE::CStorageVolumeInformation info;
        if ( CORE::GetFileSystemStorageVolumeInformationByDirPath( info, CORE::RelativePath( settings.locations[ l ].fileSystemPath ) ) && info.hasFreeBytesAvailableToCaller )
        {
            known = true;
            return info.freeBytesAvailableToCaller;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackReprocessJob::Redownload( const CAssetPackRecord& pack, CORE::CDataNode& node, CORE::CString& newPackId, bool& converted )
{GUCEF_TRACE;

    newPackId.Clear();
    converted = false;
    const Int64 startedAtMs = NowUtcMs();
    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    CAssetStoreDownloadManager* manager = m_service.GetDownloadManager();
    CORE::CString storeId;
    CORE::CString itemId;
    CORE::CString formatId;
    CORE::CString outcome;
    CORE::CString message;
    TAssetStoreStatus status = FindStoreSource( pack, storeId, itemId, formatId, outcome, message );
    TAssetStoreProviderPtr provider;
    CAssetStoreItem item;
    UInt64 bytes = 0;
    CORE::CString downloadJobId;
    CORE::CString destination;
    CAssetIngestResult ingested;
    node.SetAttribute( "storeId", storeId );
    node.SetAttribute( "itemId", itemId );
    while ( ASSETSTORE_STATUS_OK == status )
    {
        if ( GUCEF_NULL == store || GUCEF_NULL == manager )
        {
            status = ASSETSTORE_STATUS_UNAVAILABLE;
            outcome = "unavailable";
            message = "the store or the downloads of the service are not open";
            break;
        }
        if ( !m_service.GetStoreProvider( storeId, provider ) || provider.IsNULL() )
        {
            status = ASSETSTORE_STATUS_NOT_FOUND;
            outcome = "store-unavailable";
            message = "the store " + storeId + " that the pack came from is not available on this service";
            break;
        }
        if ( !provider->IsSignedIn() )
        {
            status = ASSETSTORE_STATUS_NOT_SIGNED_IN;
            outcome = "sign-in-needed";
            message = "the store " + storeId + " needs a sign in by the user (through the viewer or the sign in of the service) before its originals can be downloaded again; nothing was downloaded";
            break;
        }
        if ( IsStoreSyncRunning( storeId ) )
        {
            status = ASSETSTORE_STATUS_ALREADY_EXISTS;
            outcome = "busy";
            message = "a library sync of the store " + storeId + " runs: download again after it ended";
            break;
        }
        status = provider->GetItem( itemId, item );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            outcome = IsSessionStatus( status ) ? CORE::CString( "sign-in-needed" ) : CORE::CString( "item-unavailable" );
            message = IsSessionStatus( status ) ? "the store " + storeId + " refused its session: the user has to sign in again; nothing was downloaded"
                                                : "the store " + storeId + " does not give the item " + itemId + " (" + AssetStoreStatusToString( status ) + ")";
            if ( IsSessionStatus( status ) )
                status = ASSETSTORE_STATUS_NOT_SIGNED_IN;
            break;
        }
        if ( !item.downloadable )
        {
            status = ASSETSTORE_STATUS_UNSUPPORTED;
            outcome = "unavailable";
            message = "the store lists the item " + itemId + " but the account cannot download it now";
            break;
        }
        const CAssetStoreFormat* format = GUCEF_NULL;
        for ( size_t f=0; f<item.formats.size() && GUCEF_NULL == format; ++f )
            if ( formatId.IsNULLOrEmpty() || item.formats[ f ].formatId == formatId )
                format = &item.formats[ f ];
        if ( GUCEF_NULL == format )
        {
            status = ASSETSTORE_STATUS_NOT_FOUND;
            outcome = "format-unavailable";
            message = "the store no longer has the format " + formatId + " of the item " + itemId;
            break;
        }
        formatId = format->formatId;
        for ( size_t i=0; i<format->files.size(); ++i )
            bytes += format->files[ i ].size;
        node.SetAttribute( "formatId", formatId );
        node.SetAttribute( "estimatedBytes", bytes );

        // the size guard of the store plugin: a caller lets an item through it with allowLarge
        const UInt64 maxBytes = provider->GetMaxDownloadBytes();
        if ( maxBytes > 0 && bytes > maxBytes && !m_request.allowLarge )
        {
            status = ASSETSTORE_STATUS_REFUSED;
            outcome = "too-large";
            message = "the download is " + DescribeBytes( bytes ) + ", more than the size guard of the store plugin (" + DescribeBytes( maxBytes ) + "): ask again with allowLarge";
            break;
        }

        // the caps of the sync profile of the store and its space reserve, as for a manual download of the sync
        const CAssetLibrarySyncProfile profile = m_service.GetStoredSyncProfile( storeId );
        CAssetLibrarySyncAdmission admission;
        admission.Configure( profile, manager->GetSettings().maxConcurrentJobsPerProvider );
        admission.SetManual( true );
        admission.SetLedger( m_service.GetSyncLedger( storeId ) );
        const CPrimaryAssetStoreSettings storeSettings = store->GetSettings();
        for ( size_t l=0; l<storeSettings.locations.size(); ++l )
            if ( storeSettings.locations[ l ].locationId == pack.locationId && !storeSettings.locations[ l ].fileSystemPath.IsNULLOrEmpty() )
                admission.SetVolumePath( CORE::RelativePath( storeSettings.locations[ l ].fileSystemPath ) );
        CORE::CString reason;
        bool waiting = false;
        bool refused = false;
        if ( !admission.Admit( bytes, CAssetLibrarySyncAdmission::SLoad(), NowUtcMs(), reason, waiting, refused ) )
        {
            status = refused ? ASSETSTORE_STATUS_IO_ERROR : ASSETSTORE_STATUS_RATE_LIMITED;
            outcome = "refused-by-limits";
            message = "the download is not admitted by the limits of the store " + storeId + ": " + reason;
            break;
        }
        if ( !m_service.m_reprocessGuard.AdmitRedownload( m_request.machine, NowUtcMs(), reason ) )
        {
            status = ASSETSTORE_STATUS_RATE_LIMITED;
            outcome = "rate-limited";
            message = reason;
            break;
        }
        admission.NoteQueued( NowUtcMs(), bytes );
        m_service.NoteSyncLedger( storeId, admission.GetLedger() );
        if ( m_request.allowLarge )
            provider->AllowLargeDownload( itemId );

        // the download, to a staging folder of the location of the pack
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        destination = CPackStorageLayout::Join( CPackStorageLayout::Join( store->GetLocationRoot( pack.locationId ), CPackStorageLayout::StagingArea ),
                                                "redownload-" + pack.packId + "-" + HexOf( (UInt64) NowUtcMs(), 11 ) );
        if ( !vfs.CreateDir( destination ) )
        {
            status = ASSETSTORE_STATUS_IO_ERROR;
            outcome = "download-failed";
            message = "the staging folder " + destination + " cannot be made";
            break;
        }
        status = manager->Enqueue( storeId, itemId, formatId, CORE::CString::StringVector(), destination, false, downloadJobId, item.title );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            outcome = "download-failed";
            message = "the download cannot be queued";
            break;
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackReprocessJob: " << m_jobId << " downloads the original of pack " << pack.packId << " again: item " << itemId << " (" << formatId << ") of store " << storeId << " as " << downloadJobId );
        node.SetAttribute( "downloadJobId", downloadJobId );
        const Int64 deadline = NowUtcMs() + (Int64) m_service.m_reprocessGuard.GetSettings().downloadTimeoutMinutes * 60000;
        CAssetStoreDownloadJob job;
        while ( true )
        {
            if ( IsCancelled() || NowUtcMs() > deadline )
            {
                const bool cancelled = IsCancelled();
                manager->Cancel( downloadJobId );
                status = cancelled ? ASSETSTORE_STATUS_CANCELLED : ASSETSTORE_STATUS_TIMED_OUT;
                outcome = cancelled ? CORE::CString( "cancelled" ) : CORE::CString( "timed-out" );
                message = cancelled ? CORE::CString( "the job was cancelled during the download" ) : CORE::CString( "the download took longer than downloadTimeoutMinutes" );
                break;
            }
            if ( !manager->GetJob( downloadJobId, job ) )
            {
                status = ASSETSTORE_STATUS_FAILED;
                outcome = "download-failed";
                message = "the download " + downloadJobId + " left the queue";
                break;
            }
            if ( ASSETDOWNLOAD_STATE_DONE == job.state )
                break;
            if ( ASSETDOWNLOAD_STATE_FAILED == job.state || ASSETDOWNLOAD_STATE_CANCELLED == job.state )
            {
                status = ASSETDOWNLOAD_STATE_CANCELLED == job.state ? ASSETSTORE_STATUS_CANCELLED : ( ASSETSTORE_STATUS_OK == job.status ? ASSETSTORE_STATUS_FAILED : job.status );
                outcome = IsSessionStatus( status ) ? CORE::CString( "sign-in-needed" ) : CORE::CString( "download-failed" );
                message = IsSessionStatus( status ) ? "the store " + storeId + " refused its session during the download: the user has to sign in again" : "the download failed: " + job.message;
                if ( IsSessionStatus( status ) )
                    status = ASSETSTORE_STATUS_NOT_SIGNED_IN;
                break;
            }
            const UInt64 total = job.GetTotalBytes();
            Progress( (UInt32) ( job.GetBytesDone() / ( 1024 * 1024 ) ), (UInt32) ( total / ( 1024 * 1024 ) ), "downloading the original again (MiB)" );
            MT::ThreadDelay( 50 );
        }
        manager->Remove( downloadJobId );
        if ( ASSETSTORE_STATUS_OK != status )
            break;
        node.SetAttribute( "downloadedBytes", job.GetBytesDone() );

        // the ingest decides: the same files are the pack itself, other files a new pack that supersedes it
        Progress( 0, 0, "verifying the download against the stored original" );
        CAssetIngestRequest ingest;
        ingest.sourceId = storeId;
        ingest.itemId = itemId;
        ingest.item = item;
        ingest.contentPath = destination;
        ingest.keep = m_request.thenConvert ? CORE::CString( AssetKeepPolicyToString( pack.keep ) ) : CORE::CString( AssetKeepPolicyToString( ASSETKEEP_ORIGINAL ) );
        ingest.locationId = pack.locationId;
        ingest.duplicatePolicy = AssetDuplicatePolicyToString( ASSETDUPLICATE_SKIP );
        ingest.downloadJobId = downloadJobId;
        ingest.formatId = formatId;
        ingest.expectedRevision = CAssetItemRevision::GetEffectiveRevision( item );
        ingest.verifyPackId = pack.packId;
        ingest.repairOriginals = m_request.repairOriginals;
        const TAssetStoreStatus ingestStatus = store->Ingest( ingest, ingested );
        if ( ASSETSTORE_STATUS_DUPLICATE == ingestStatus && ingested.duplicateOfPackId == pack.packId )
        {
            status = ASSETSTORE_STATUS_OK;
            outcome = "verified-identical";
            message = "the original in the store is the same as the stored one: nothing changed" +
                      ( ingested.repairedFiles.empty() ? CORE::CString::Empty : "; " + CORE::ToString( (UInt32) ingested.repairedFiles.size() ) + " missing or damaged original file(s) were put back" );
            node.SetValuesOfChildByName( "repairedFiles", ingested.repairedFiles );
            break;
        }
        if ( ASSETSTORE_STATUS_OK != ingestStatus )
        {
            status = ingestStatus;
            outcome = "ingest-failed";
            message = "the download could not be ingested: " + ingested.message;
            break;
        }
        newPackId = ingested.packId;
        outcome = "new-pack";
        message = "the original in the store differs from the stored one: it is the new pack " + newPackId + "; pack " + pack.packId + " is kept, superseded by it";
        // the old pack is superseded by the new one, also when it had no store facts yet
        CAssetPackSourceRecord facts;
        CAssetSyncMarkStore& syncStore = store->GetSyncStore();
        if ( syncStore.IsAvailable() )
        {
            if ( ASSETSTORE_STATUS_OK != syncStore.GetPackSource( pack.packId, facts ) )
            {
                facts = CAssetPackSourceRecord();
                facts.packId = pack.packId;
                facts.sourceId = storeId;
                facts.sourceItemId = itemId;
                facts.sourceFormatId = formatId;
                facts.revision = "unk:";
                facts.syncedMs = pack.ingestedAtMs;
            }
            if ( facts.IsCurrent() )
            {
                facts.supersededBy = newPackId;
                facts.supersededMs = NowUtcMs();
                syncStore.WritePackSource( facts );
            }
        }
        CAssetPackRecord newPack;
        if ( ASSETSTORE_STATUS_OK == store->GetPack( newPackId, newPack ) )
            converted = CAssetConversionRecord::OutcomeConverted == newPack.conversion.outcome;
        m_service.RecordChange( ASSETCHANGE_PACK_UPDATED, pack.packId );
        break;
    }
    if ( !destination.IsNULLOrEmpty() && VFS::CVfsGlobal::Instance()->GetVfs().DirExists( destination ) )
        VFS::CVfsGlobal::Instance()->GetVfs().DeleteDir( destination, true );
    node.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) );
    node.SetAttribute( "outcome", outcome );
    node.SetAttribute( "message", message );
    if ( !newPackId.IsNULLOrEmpty() )
    {
        node.SetAttribute( "newPackId", newPackId );
        node.SetAttribute( "supersededPackId", pack.packId );
    }
    if ( !ingested.problems.empty() )
        node.SetValuesOfChildByName( "problems", ingested.problems );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackReprocessJob: " << m_jobId << " redownload of pack " << pack.packId << ": " << outcome << ": " << message );
    WriteHistory( pack.packId, CAssetReprocessRequest::StepRedownload, outcome, message, startedAtMs );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackReprocessJob::Regenerate( const CORE::CString& packId, bool target, const CORE::CString::StringVector& globs, bool cook, CORE::CDataNode& node )
{GUCEF_TRACE;

    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    if ( GUCEF_NULL == store )
        return ASSETSTORE_STATUS_UNAVAILABLE;
    const CORE::CString step = target ? CAssetReprocessRequest::StepTarget : CAssetReprocessRequest::StepIntermediate;
    Progress( 0, 0, target ? "regenerating the target resources" : "regenerating the intermediate resources" );
    CAssetPackRegeneration regeneration( *store, GUCEF_NULL, m_observer );
    regeneration.SetJob( m_jobId, "reprocess:" + step, m_request.DescribeForAudit() );
    CAssetConversionOutcome outcome;
    const TAssetStoreStatus status = regeneration.Regenerate( packId, target ? CAssetPackRegeneration::MODE_TARGET : CAssetPackRegeneration::MODE_INTERMEDIATE,
                                                              globs, cook, m_request.targets, m_request.acceptErrors, MakeMade( step ), outcome );
    node.Copy( regeneration.GetReport() );
    node.SetName( step );
    node.SetAttribute( "packId", packId );
    node.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) );
    node.SetAttribute( "message", outcome.message );
    if ( !outcome.problems.empty() )
        node.SetValuesOfChildByName( "problems", outcome.problems );
    if ( node.GetAttributeValueOrChildValueByName( "replaced" ).AsBool( false, true ) )
        ReloadIndex( packId );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackReprocessJob: " << m_jobId << " " << DescribeRegeneration( step.C_String(), node ) );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackReprocessJob::RollbackOutputs( const CORE::CString& packId, CORE::CDataNode& node )
{GUCEF_TRACE;

    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    if ( GUCEF_NULL == store )
        return ASSETSTORE_STATUS_UNAVAILABLE;
    CAssetPackRegeneration regeneration( *store, GUCEF_NULL, m_observer );
    regeneration.SetJob( m_jobId, "reprocess:" + CAssetReprocessRequest::StepRollback, m_request.DescribeForAudit() );
    CAssetConversionOutcome outcome;
    const TAssetStoreStatus status = regeneration.Rollback( packId, MakeMade( CAssetReprocessRequest::StepRollback ), outcome );
    node.Copy( regeneration.GetReport() );
    node.SetName( CAssetReprocessRequest::StepRollback );
    node.SetAttribute( "packId", packId );
    node.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) );
    node.SetAttribute( "message", outcome.message );
    if ( ASSETSTORE_STATUS_OK == status )
        ReloadIndex( packId );
    else
        WriteHistory( packId, CAssetReprocessRequest::StepRollback, "failed", outcome.message, NowUtcMs() );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackReprocessJob::Run( CAssetServiceDocument& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CDataNode& document = result.document;
    document.SetName( "result" );
    document.SetNodeType( GUCEF_DATATYPE_OBJECT );
    document.SetAttribute( "operation", m_request.operation );
    document.SetAttribute( "caller", m_request.caller );
    CORE::CString::StringVector steps;
    m_request.GetEffectiveSteps( steps );
    document.SetValuesOfChildByName( "steps", steps );

    CORE::CString problem;
    if ( !m_request.IsValid( problem ) )
    {
        result.message = problem;
        document.SetAttribute( "summary", problem );
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    CORE::CString packId = m_request.packId;
    if ( packId.IsNULLOrEmpty() && GUCEF_NULL != store && !m_request.storeAssetIds.empty() )
    {
        CAssetRecord asset;
        if ( ASSETSTORE_STATUS_OK == m_service.GetAsset( m_request.storeAssetIds[ 0 ], asset ) )
            packId = asset.packId;
    }
    CAssetPackRecord pack;
    if ( GUCEF_NULL == store || ASSETSTORE_STATUS_OK != store->GetPack( packId, pack ) )
    {
        result.message = "there is no pack " + packId + " in the catalogue";
        document.SetAttribute( "summary", result.message );
        return ASSETSTORE_STATUS_NOT_FOUND;
    }
    document.SetAttribute( "packId", packId );
    document.SetAttribute( "packTitle", pack.title );

    CORE::CString::StringVector globs;
    CORE::CDataNode* scope = document.AddChild( "scope", GUCEF_DATATYPE_OBJECT );
    TAssetStoreStatus status = ResolveScope( packId, globs, *scope, problem );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        result.message = problem;
        document.SetAttribute( "summary", problem );
        return status;
    }
    CORE::CString::StringVector summary;
    if ( m_request.IsPartial() && globs.empty() && !m_request.HasStep( CAssetReprocessRequest::StepRollback ) )
    {
        result.message = "no asset of pack " + packId + " matches the scope: nothing to do";
        document.SetAttribute( "summary", result.message );
        return ASSETSTORE_STATUS_OK;
    }

    CORE::CString targetPackId = packId;
    bool regenerated = false;
    const CORE::CDataNode* lastRegeneration = GUCEF_NULL;
    for ( size_t s=0; s<steps.size() && ASSETSTORE_STATUS_OK == status; ++s )
    {
        if ( IsCancelled() )
        {
            status = ASSETSTORE_STATUS_CANCELLED;
            summary.push_back( "cancelled before the step " + steps[ s ] );
            break;
        }
        const CORE::CString& step = steps[ s ];
        if ( CAssetReprocessRequest::StepRedownload == step )
        {
            CORE::CDataNode* node = document.AddChild( "redownload", GUCEF_DATATYPE_OBJECT );
            CORE::CString newPackId;
            bool converted = false;
            status = Redownload( pack, *node, newPackId, converted );
            summary.push_back( "redownload: " + node->GetAttributeValueOrChildValueByName( "outcome" ).AsString( CORE::CString::Empty, true ) + " (" +
                               node->GetAttributeValueOrChildValueByName( "message" ).AsString( CORE::CString::Empty, true ) + ")" );
            if ( ASSETSTORE_STATUS_OK != status )
                break;
            if ( !newPackId.IsNULLOrEmpty() )
            {
                targetPackId = newPackId;
                document.SetAttribute( "newPackId", newPackId );
                if ( !m_request.thenConvert )
                    break;
                if ( converted )
                {
                    // its ingest converted it with the keep policy of the pack
                    summary.push_back( "the new pack " + newPackId + " was converted by its ingest" );
                    regenerated = true;
                    ReloadIndex( newPackId );
                    continue;
                }
                if ( steps.end() == std::find( steps.begin() + s + 1, steps.end(), CAssetReprocessRequest::StepIntermediate ) )
                {
                    CORE::CDataNode* regen = document.AddChild( CAssetReprocessRequest::StepIntermediate, GUCEF_DATATYPE_OBJECT );
                    status = Regenerate( targetPackId, false, globs, true, *regen );
                    summary.push_back( DescribeRegeneration( "the new pack", *regen ) );
                    lastRegeneration = regen;
                    regenerated = true;
                }
            }
            continue;
        }
        if ( CAssetReprocessRequest::StepIntermediate == step )
        {
            if ( regenerated && targetPackId != packId )
                continue;
            // the cook of the target step goes with this run when it is asked
            const bool cookNow = m_request.cook || m_request.HasStep( CAssetReprocessRequest::StepTarget );
            CORE::CDataNode* node = document.AddChild( CAssetReprocessRequest::StepIntermediate, GUCEF_DATATYPE_OBJECT );
            status = Regenerate( targetPackId, false, globs, cookNow, *node );
            summary.push_back( DescribeRegeneration( "intermediate", *node ) );
            lastRegeneration = node;
            regenerated = regenerated || cookNow;
            continue;
        }
        if ( CAssetReprocessRequest::StepTarget == step )
        {
            if ( regenerated )
            {
                summary.push_back( "target: cooked in the run of the intermediate" );
                continue;
            }
            CORE::CDataNode* node = document.AddChild( CAssetReprocessRequest::StepTarget, GUCEF_DATATYPE_OBJECT );
            status = Regenerate( targetPackId, true, globs, true, *node );
            summary.push_back( DescribeRegeneration( "target", *node ) );
            lastRegeneration = node;
            continue;
        }
        if ( CAssetReprocessRequest::StepRollback == step )
        {
            CORE::CDataNode* node = document.AddChild( CAssetReprocessRequest::StepRollback, GUCEF_DATATYPE_OBJECT );
            status = RollbackOutputs( targetPackId, *node );
            summary.push_back( ASSETSTORE_STATUS_OK == status ? "rollback: the previous generation of the outputs is current again (" + CountsText( node->FindChild( "after" ) ) + ")"
                                                              : "rollback: " + node->GetAttributeValueOrChildValueByName( "message" ).AsString( CORE::CString::Empty, true ) );
            lastRegeneration = node;
        }
    }

    // the counts of the last regeneration at the top, for a reader that looks at one place
    if ( GUCEF_NULL != lastRegeneration )
    {
        const char* const copied[] = { "before", "after", "changes" };
        for ( UInt32 c=0; c<3; ++c )
        {
            const CORE::CDataNode* child = lastRegeneration->FindChild( copied[ c ] );
            if ( GUCEF_NULL != child )
                document.AddChild( *child );
        }
        const CORE::CDataNode* after = lastRegeneration->FindChild( "after" );
        CORE::CDataNode* counts = document.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
        counts->SetAttribute( "failed", GUCEF_NULL != after ? after->GetAttributeValueOrChildValueByName( "failed" ).AsUInt32( 0, true ) : (UInt32) 0 );
        document.SetAttribute( "reportPath", lastRegeneration->GetAttributeValueOrChildValueByName( "reportPath" ).AsString( CORE::CString::Empty, true ) );
    }
    CAssetPackGenerations generations( CPackStorageLayout( pack.rootPath ), store->GetLocationRoot( pack.locationId ) );
    if ( targetPackId == packId )
    {
        document.SetAttribute( "rollbackAvailable", generations.HasPrevious() );
        document.SetAttribute( "previousGeneration", generations.HasPrevious() ? generations.GetPreviousFolder() : CORE::CString::Empty );
    }
    CORE::CString text = m_request.operation + " of pack " + packId + ( pack.title.IsNULLOrEmpty() ? CORE::CString::Empty : " \"" + pack.title + "\"" ) + ": ";
    for ( size_t i=0; i<summary.size(); ++i )
        text += ( 0 == i ? CORE::CString::Empty : CORE::CString( "; " ) ) + summary[ i ];
    if ( document.GetAttributeValueOrChildValueByName( "rollbackAvailable" ).AsBool( false, true ) && CAssetPipelineOperations::ReprocessRollback != m_request.operation )
        text += "; the previous generation is kept (reprocess_rollback restores it)";
    document.SetAttribute( "summary", text );
    document.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) );
    result.message = text;
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackReprocessJob::Plan( CAssetServiceDocument& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CDataNode& document = result.document;
    document.SetName( "plan" );
    document.SetNodeType( GUCEF_DATATYPE_OBJECT );
    document.SetAttribute( "operation", m_request.operation );
    CORE::CString::StringVector steps;
    m_request.GetEffectiveSteps( steps );
    document.SetValuesOfChildByName( "steps", steps );
    CORE::CString problem;
    if ( !m_request.IsValid( problem ) )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = problem;
        return result.status;
    }
    CPrimaryAssetStore* store = m_service.GetPrimaryStore();
    CAssetPackRecord pack;
    if ( GUCEF_NULL == store || ASSETSTORE_STATUS_OK != store->GetPack( m_request.packId, pack ) )
    {
        result.status = ASSETSTORE_STATUS_NOT_FOUND;
        result.message = "there is no pack " + m_request.packId + " in the catalogue";
        return result.status;
    }
    document.SetAttribute( "packId", pack.packId );
    document.SetAttribute( "packTitle", pack.title );
    CORE::CString::StringVector refusals;
    CORE::CString::StringVector globs;
    CORE::CDataNode* scope = document.AddChild( "scope", GUCEF_DATATYPE_OBJECT );
    if ( ASSETSTORE_STATUS_OK != ResolveScope( pack.packId, globs, *scope, problem ) )
        refusals.push_back( problem );
    else
    if ( m_request.IsPartial() && globs.empty() && !m_request.HasStep( CAssetReprocessRequest::StepRollback ) )
        refusals.push_back( "no asset of the pack matches the scope: nothing would be done" );

    // the limits: the queue and the hourly limits of a machine
    CORE::CString reason;
    UInt32 queued = 0;
    {
        MT::CScopeMutex lock( m_service.m_lock );
        for ( size_t i=0; i<m_service.m_pipelineJobs.size(); ++i )
            if ( CAssetPipelineOperations::IsReprocess( m_service.m_pipelineJobs[ i ].request.operation ) && !AssetJobStateIsFinal( m_service.m_pipelineJobs[ i ].state ) )
            {
                ++queued;
                if ( CAssetReprocessRequest::FromRequest( m_service.m_pipelineJobs[ i ].request ).packId == pack.packId )
                    refusals.push_back( "busy: a reprocessing job of the pack runs: " + CLocalAssetStoreService::PipelineJobPrefix + m_service.m_pipelineJobs[ i ].key );
            }
    }
    if ( !m_service.m_reprocessGuard.WouldAdmit( pack.packId, m_request.machine, !m_request.machine, queued, NowUtcMs(), reason ) )
        refusals.push_back( reason );
    document.SetAttribute( "queuedJobs", queued );

    UInt64 intermediateBytes = 0;
    UInt64 cookedBytes = 0;
    bool hasPrevious = false;
    if ( m_request.HasStep( CAssetReprocessRequest::StepIntermediate ) || m_request.HasStep( CAssetReprocessRequest::StepTarget ) )
    {
        const bool targetOnly = !m_request.HasStep( CAssetReprocessRequest::StepIntermediate );
        if ( ASSETSTORE_STATUS_OK != CAssetPackRegeneration::Check( *store, pack.packId, targetOnly ? CAssetPackRegeneration::MODE_TARGET : CAssetPackRegeneration::MODE_INTERMEDIATE,
                                                                    problem, intermediateBytes, cookedBytes, hasPrevious ) )
            refusals.push_back( problem );
        CORE::CDataNode* outputs = document.AddChild( "outputs", GUCEF_DATATYPE_OBJECT );
        outputs->SetAttribute( "intermediateBytes", intermediateBytes );
        outputs->SetAttribute( "cookedBytes", cookedBytes );
        // a run writes the new outputs beside the current ones; the replaced generation is kept until the next one
        outputs->SetAttribute( "estimatedStagingBytes", ( targetOnly ? (UInt64) 0 : intermediateBytes ) + cookedBytes );
        bool known = false;
        const UInt64 freeBytes = ReadFreeBytes( pack, known );
        if ( known )
        {
            outputs->SetAttribute( "freeBytes", freeBytes );
            if ( freeBytes < ( targetOnly ? (UInt64) 0 : intermediateBytes ) + cookedBytes )
                refusals.push_back( "the location has " + DescribeBytes( freeBytes ) + " free, less than the outputs being made again need" );
        }
    }
    else
    {
        CAssetPackRegeneration::Check( *store, pack.packId, CAssetPackRegeneration::MODE_INTERMEDIATE, problem, intermediateBytes, cookedBytes, hasPrevious );
    }
    document.SetAttribute( "rollbackAvailable", hasPrevious );
    if ( m_request.HasStep( CAssetReprocessRequest::StepRollback ) && !hasPrevious )
        refusals.push_back( "there is no previous generation of the outputs of the pack to roll back to" );

    if ( m_request.HasStep( CAssetReprocessRequest::StepRedownload ) )
    {
        CORE::CDataNode* node = document.AddChild( "redownload", GUCEF_DATATYPE_OBJECT );
        CORE::CString storeId;
        CORE::CString itemId;
        CORE::CString formatId;
        CORE::CString outcome;
        if ( ASSETSTORE_STATUS_OK != FindStoreSource( pack, storeId, itemId, formatId, outcome, problem ) )
        {
            refusals.push_back( problem );
            node->SetAttribute( "outcome", outcome );
        }
        else
        {
            node->SetAttribute( "storeId", storeId );
            node->SetAttribute( "itemId", itemId );
            node->SetAttribute( "formatId", formatId );
            TAssetStoreProviderPtr provider;
            const bool hasProvider = m_service.GetStoreProvider( storeId, provider ) && !provider.IsNULL();
            const bool signedIn = hasProvider && provider->IsSignedIn();
            node->SetAttribute( "storeAvailable", hasProvider );
            node->SetAttribute( "signInNeeded", hasProvider && !signedIn );
            if ( !hasProvider )
                refusals.push_back( "the store " + storeId + " is not available on this service" );
            else
            if ( !signedIn )
                refusals.push_back( "the store " + storeId + " needs a sign in by the user first" );
            else
            {
                // the size of the download as the store gives it (one call to the store)
                CAssetStoreItem item;
                if ( ASSETSTORE_STATUS_OK == provider->GetItem( itemId, item ) )
                {
                    UInt64 bytes = 0;
                    for ( size_t f=0; f<item.formats.size(); ++f )
                        if ( formatId.IsNULLOrEmpty() || item.formats[ f ].formatId == formatId )
                        {
                            for ( size_t i=0; i<item.formats[ f ].files.size(); ++i )
                                bytes += item.formats[ f ].files[ i ].size;
                            break;
                        }
                    node->SetAttribute( "estimatedBytes", bytes );
                    const UInt64 maxBytes = provider->GetMaxDownloadBytes();
                    if ( maxBytes > 0 && bytes > maxBytes && !m_request.allowLarge )
                        refusals.push_back( "the download (" + DescribeBytes( bytes ) + ") is larger than the size guard of the store plugin: allowLarge is needed" );
                    CAssetLibrarySyncAdmission admission;
                    CAssetStoreDownloadManager* manager = m_service.GetDownloadManager();
                    admission.Configure( m_service.GetStoredSyncProfile( storeId ), GUCEF_NULL != manager ? manager->GetSettings().maxConcurrentJobsPerProvider : 4 );
                    admission.SetManual( true );
                    admission.SetLedger( m_service.GetSyncLedger( storeId ) );
                    bool waiting = false;
                    bool refused = false;
                    if ( !admission.Admit( bytes, CAssetLibrarySyncAdmission::SLoad(), NowUtcMs(), reason, waiting, refused ) )
                        refusals.push_back( "the limits of the store refuse the download now: " + reason );
                }
                else
                    refusals.push_back( "the store does not give the item " + itemId + " now" );
            }
            if ( IsStoreSyncRunning( storeId ) )
                refusals.push_back( "busy: a library sync of the store " + storeId + " runs" );
        }
    }
    document.SetValuesOfChildByName( "refusals", refusals );
    document.SetAttribute( "wouldRun", refusals.empty() );
    CORE::CString text = m_request.operation + " of pack " + pack.packId + ": " + ( refusals.empty() ? CORE::CString( "would run" ) : "would not run: " + refusals[ 0 ] );
    document.SetAttribute( "summary", text );
    result.status = ASSETSTORE_STATUS_OK;
    result.message = text;
    return result.status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
