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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
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
#ifndef GUCEF_COMCORE_CDATABASETRANSACTION_H
#include "gucefCOMCORE_CDatabaseTransaction.h"
#define GUCEF_COMCORE_CDATABASETRANSACTION_H
#endif /* GUCEF_COMCORE_CDATABASETRANSACTION_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#include "gucefASSETSTORE_CAssetCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUE_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#include "gucefASSETSTORE_CIAssetConverterHook.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */

#include "gucefASSETSTORE_CAssetPackConversion.h"

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

inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

Int64
NowUtcMs( void )
{
    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

CORE::CString
NormalizeVfsPath( const CORE::CString& path )
{
    CORE::CString normalized = path.ReplaceChar( '\\', '/' );
    while ( ByteLengthOf( normalized ) > 0 && '/' == normalized.C_String()[ 0 ] )
        normalized = normalized.CutChars( 1, true );
    while ( ByteLengthOf( normalized ) > 0 && '/' == normalized.C_String()[ ByteLengthOf( normalized )-1 ] )
        normalized = normalized.CutChars( 1, false );
    return normalized;
}

CORE::CString
JoinVfs( const CORE::CString& a, const CORE::CString& b )
{
    const CORE::CString left = NormalizeVfsPath( a );
    const CORE::CString right = NormalizeVfsPath( b );
    if ( left.IsNULLOrEmpty() )
        return right;
    if ( right.IsNULLOrEmpty() )
        return left;
    return left + "/" + right;
}

CORE::CString
HexOf( UInt64 value, UInt32 digits )
{
    static const char hexDigits[] = "0123456789abcdef";
    CORE::CString text;
    for ( Int32 d=(Int32) digits-1; d>=0; --d )
        text += hexDigits[ ( value >> ( d * 4 ) ) & 0xF ];
    return text;
}

/** The observer of a conversion of a pack in the store: the cancel request of the job's future, the progress as events of the store */
class CStoredPackObserver : public CIAssetConversionObserver
{
    public:

    CStoredPackObserver( CPrimaryAssetStore& store, const CORE::CString& jobId, const CORE::CString& packId, const CORE::CFutureResultSource* cancel )
        : CIAssetConversionObserver()
        , m_store( store )
        , m_jobId( jobId )
        , m_packId( packId )
        , m_cancel( cancel )
    {

    }

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NULL != m_cancel && m_cancel->IsCancellationRequested();
    }

    virtual void OnConversionProgress( UInt32 done, UInt32 total, const CORE::CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionProgressEvent, m_jobId, m_packId, false, done, total, assetPath, ASSETSTORE_STATUS_OK,
                                       "Converted " + CORE::ToString( done ) + " of " + CORE::ToString( total ) );
    }

    private:

    CPrimaryAssetStore& m_store;
    CORE::CString m_jobId;
    CORE::CString m_packId;
    const CORE::CFutureResultSource* m_cancel;
};


TAssetStoreStatus
FailOutcome( CAssetConversionOutcome& outcome, TAssetStoreStatus status, const CORE::CString& message )
{
    outcome.status = status;
    outcome.message = message;
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackConversion: Pack " << outcome.packId << ": " << message );
    return status;
}

/** The rows of the catalogue that a conversion changes */
struct SConversionRows
{
    CAssetCatalogue::TRowVector assets;
    CAssetCatalogue::TRowVector pack;
    CAssetCatalogue::TRowVector facets;
    CAssetCatalogue::TRowVector conversion;
    CAssetCatalogue::TRowVector history;
    CORE::CDataTableQuery facetQuery;
    CORE::CString::StringSet removedFileAssetIds;
};

TAssetStoreStatus
WriteConversionRows( CAssetCatalogue& catalogue, const SConversionRows& rows, CORE::CString& message )
{
    TAssetStoreStatus status = ASSETSTORE_STATUS_OK;
    if ( !rows.assets.empty() )
        status = catalogue.Write( "assets", rows.assets );
    for ( CORE::CString::StringSet::const_iterator r=rows.removedFileAssetIds.begin(); ASSETSTORE_STATUS_OK == status && r!=rows.removedFileAssetIds.end(); ++r )
        status = catalogue.DeleteWhereEquals( "asset_files", "asset_id", CORE::CVariant( (*r) ) );
    if ( ASSETSTORE_STATUS_OK == status )
        status = catalogue.Delete( "facets", rows.facetQuery );
    if ( ASSETSTORE_STATUS_OK == status )
        status = catalogue.Write( "facets", rows.facets );
    if ( ASSETSTORE_STATUS_OK == status )
        status = catalogue.Write( "pack_conversions", rows.conversion );
    if ( ASSETSTORE_STATUS_OK == status )
        status = catalogue.Write( "ingest_history", rows.history );
    if ( ASSETSTORE_STATUS_OK == status )
        status = catalogue.Write( "packs", rows.pack );
    if ( ASSETSTORE_STATUS_OK != status )
        message = "writing the result of the conversion failed: " + catalogue.GetLastErrorMessage();
    return status;
}
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPackConversion::CAssetPackConversion( CPrimaryAssetStore& store, const CORE::CFutureResultSource* cancel )
    : m_jobId()
    , m_historyOrigin( "conversion" )
    , m_historyPolicy()
    , m_observer( GUCEF_NULL )
    , m_notifyEvents( true )
    , m_removeOriginals( true )
    , m_store( store )
    , m_cancel( cancel )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPackConversion::~CAssetPackConversion()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPrimaryAssetStore&
CAssetPackConversion::GetStore( void ) const
{GUCEF_TRACE;

    return m_store;
}

/*-------------------------------------------------------------------------*/

const CORE::CFutureResultSource*
CAssetPackConversion::GetCancelSource( void ) const
{GUCEF_TRACE;

    return m_cancel;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackConversion::OnPrepare( const CAssetPackRecord& pack, const TAssetRecordVector& assets, const CORE::CString& locationRoot,
                                 CAssetConversionRequest& request, CORE::CString& message )
{GUCEF_TRACE;

    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackConversion::OnConverted( const CAssetConversionRequest& request, TAssetStoreStatus& hookStatus, CAssetConversionOutcome& hookOutcome,
                                   const TAssetRecordVector& assets, TAssetRecordVector& converted, CORE::CString& message )
{GUCEF_TRACE;

    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackConversion::OnCommit( const CAssetConversionRequest& request, TAssetStoreStatus hookStatus, const CAssetPackRecord& before,
                                const TAssetRecordVector& beforeAssets, CORE::CString& message )
{GUCEF_TRACE;

    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackConversion::OnCommitted( bool persisted )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackConversion::RestoreState( const CAssetPackRecord& pack, const TAssetRecordVector& assets, const CAssetConversionOutcome& outcome,
                                    Int64 startedAtMs, CORE::CString& message )
{GUCEF_TRACE;

    const CORE::CString::StringSet noRemovedOriginals;
    return Persist( pack, assets, true, noRemovedOriginals, outcome, startedAtMs, message );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackConversion::IsCancelRequested( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_cancel && m_cancel->IsCancellationRequested();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackConversion::MakeJobId( void )
{GUCEF_TRACE;

    return "cv" + HexOf( (UInt64) NowUtcMs(), 11 ) + HexOf( CORE::CRandom::GetRandomUInt32(), 8 );
}

/*-------------------------------------------------------------------------*/

void
CAssetPackConversion::CollectConvertedFormats( const TAssetRecordVector& assets, CORE::CString::StringSet& formats )
{GUCEF_TRACE;

    formats.clear();
    for ( size_t a=0; a<assets.size(); ++a )
        if ( !assets[ a ].convertedFormat.IsNULLOrEmpty() && !assets[ a ].convertedPath.IsNULLOrEmpty() )
            formats.insert( assets[ a ].convertedFormat );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackConversion::ApplyOutcome( TAssetStoreStatus hookStatus, const CAssetConversionOutcome& hookOutcome ,
                                    const TAssetRecordVector& converted, CAssetPackRecord& pack              ,
                                    TAssetRecordVector& assets, CORE::CString::StringVector& problems         ,
                                    CORE::CString& message                                                    )
{GUCEF_TRACE;

    problems.insert( problems.end(), hookOutcome.problems.begin(), hookOutcome.problems.end() );
    CAssetConversionRecord record = hookOutcome.record;
    record.convertedAtMs = NowUtcMs();

    switch ( hookStatus )
    {
        case ASSETSTORE_STATUS_OK:
        {
            assets = converted;
            CollectConvertedFormats( assets, pack.formatsConverted );
            record.outcome = CAssetConversionRecord::OutcomeConverted;
            record.reason.Clear();
            pack.conversion = record;
            pack.stateReason.Clear();
            if ( ASSETPACK_STATE_CONVERSION_PENDING == pack.state || ASSETPACK_STATE_CONVERSION_REFUSED == pack.state )
                pack.state = ASSETPACK_STATE_INGESTED;
            return ASSETSTORE_STATUS_OK;
        }
        case ASSETSTORE_STATUS_REFUSED:
        {
            const CORE::CString reason = "conversion refused: " + hookOutcome.message;
            problems.push_back( reason );
            record.outcome = CAssetConversionRecord::OutcomeRefused;
            record.reason = hookOutcome.message;
            pack.conversion = record;
            pack.state = ASSETPACK_STATE_CONVERSION_REFUSED;
            pack.stateReason = reason;
            return ASSETSTORE_STATUS_OK;
        }
        case ASSETSTORE_STATUS_FAILED:
        {
            const CORE::CString reason = "conversion failed: " + hookOutcome.message;
            problems.push_back( reason + "; the pack is kept as original" );
            record.outcome = CAssetConversionRecord::OutcomeFailed;
            record.reason = hookOutcome.message;
            pack.conversion = record;
            pack.state = ASSETPACK_STATE_CONVERSION_PENDING;
            pack.stateReason = reason;
            return ASSETSTORE_STATUS_OK;
        }
        case ASSETSTORE_STATUS_CANCELLED:
        {
            message = hookOutcome.message.IsNULLOrEmpty() ? CORE::CString( "the conversion was cancelled" ) : hookOutcome.message;
            return ASSETSTORE_STATUS_CANCELLED;
        }
        default:
        {
            message = hookOutcome.message.IsNULLOrEmpty() ? CORE::CString( "the conversion could not run" ) : hookOutcome.message;
            return hookStatus;
        }
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackConversion::Persist( const CAssetPackRecord& pack, const TAssetRecordVector& assets, bool assetsChanged   ,
                               const CORE::CString::StringSet& removedOriginals, const CAssetConversionOutcome& outcome ,
                               Int64 startedAtMs, CORE::CString& message                                              )
{GUCEF_TRACE;

    CAssetCatalogue& catalogue = m_store.GetCatalogue();

    // the rows to change
    CAssetCatalogue::TRowVector assetRows;
    if ( assetsChanged )
    {
        CORE::CDataTableQuery query;
        query.WhereEquals( "pack_id", CORE::CVariant( pack.packId ) );
        TAssetStoreStatus status = catalogue.Select( "assets", query, assetRows );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            message = "the assets of the pack cannot be read: " + catalogue.GetLastErrorMessage();
            return status;
        }
        GUCEF::map< CORE::CString, size_t > byId;
        for ( size_t a=0; a<assets.size(); ++a )
            byId[ assets[ a ].assetId ] = a;
        for ( size_t r=0; r<assetRows.size(); ++r )
        {
            GUCEF::map< CORE::CString, size_t >::const_iterator i = byId.find( CAssetCatalogue::GetText( assetRows[ r ], "asset_id" ) );
            if ( i == byId.end() )
                continue;
            assetRows[ r ][ "converted_path" ] = CORE::CVariant( assets[ (*i).second ].convertedPath );
            assetRows[ r ][ "converted_format" ] = CORE::CVariant( assets[ (*i).second ].convertedFormat );
        }
    }

    CAssetCatalogue::TRowVector packRows;
    {
        CORE::CDataTableQuery query;
        query.WhereEquals( "pack_id", CORE::CVariant( pack.packId ) );
        const TAssetStoreStatus status = catalogue.Select( "packs", query, packRows );
        if ( ASSETSTORE_STATUS_OK != status || packRows.empty() )
        {
            message = "the pack row cannot be read: " + catalogue.GetLastErrorMessage();
            return ASSETSTORE_STATUS_OK != status ? status : ASSETSTORE_STATUS_NOT_FOUND;
        }
    }
    CAssetCatalogue::TRow& packRow = packRows[ 0 ];
    packRow[ "formats_converted" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.formatsConverted ) );
    packRow[ "state" ] = CORE::CVariant( CORE::CString( AssetPackStateToString( pack.state ) ) );
    packRow[ "state_reason" ] = CORE::CVariant( pack.stateReason );

    // the format facets: the original formats and the converted ones
    CORE::CString::StringSet formats = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( packRow, "formats_original" ) );
    formats.insert( pack.formatsConverted.begin(), pack.formatsConverted.end() );
    CAssetCatalogue::TRowVector facetRows;
    for ( CORE::CString::StringSet::const_iterator f=formats.begin(); f!=formats.end(); ++f )
    {
        CAssetCatalogue::TRow facet;
        facet[ "object_id" ] = CORE::CVariant( pack.packId );
        facet[ "facet" ] = CORE::CVariant( CORE::CString( "format" ) );
        facet[ "value" ] = CORE::CVariant( (*f) );
        facet[ "object_type" ] = CORE::CVariant( CORE::CString( "pack" ) );
        facet[ "pack_id" ] = CORE::CVariant( pack.packId );
        facetRows.push_back( facet );
    }
    CORE::CDataTableQuery facetQuery;
    facetQuery.WhereEquals( "object_id", CORE::CVariant( pack.packId ) );
    facetQuery.WhereEquals( "facet", CORE::CVariant( CORE::CString( "format" ) ) );

    CAssetCatalogue::TRowVector conversionRows( 1 );
    CAssetCatalogue::MakeConversionRow( pack.packId, pack.conversion, conversionRows[ 0 ] );

    CAssetCatalogue::TRowVector historyRows( 1 );
    {
        CAssetCatalogue::TRow& row = historyRows[ 0 ];
        const CORE::CString outcomeName = ASSETSTORE_STATUS_OK == outcome.status ? CORE::CString( "converted" ) :
                                            ( ASSETSTORE_STATUS_REFUSED == outcome.status ? CORE::CString( "conversionRefused" ) : CORE::CString( "conversionFailed" ) );
        row[ "entry_id" ] = CORE::CVariant( "ih" + HexOf( (UInt64) NowUtcMs(), 11 ) + HexOf( CORE::CRandom::GetRandomUInt32(), 8 ) );
        row[ "ingest_id" ] = CORE::CVariant( outcome.jobId );
        row[ "pack_id" ] = CORE::CVariant( pack.packId );
        row[ "source_id" ] = CORE::CVariant( pack.sourceId );
        row[ "origin" ] = CORE::CVariant( m_historyOrigin.IsNULLOrEmpty() ? CORE::CString( "conversion" ) : m_historyOrigin );
        row[ "policy" ] = CORE::CVariant( "keep=" + CORE::CString( AssetKeepPolicyToString( pack.keep ) ) + ";profile=" + outcome.record.profile +
                                          ( m_historyPolicy.IsNULLOrEmpty() ? CORE::CString::Empty : ";" + m_historyPolicy ) );
        row[ "started_ms" ] = CORE::CVariant( startedAtMs );
        row[ "duration_ms" ] = CORE::CVariant( NowUtcMs() - startedAtMs );
        row[ "outcome" ] = CORE::CVariant( outcomeName );
        row[ "step" ] = CORE::CVariant( CORE::CString( AssetIngestStepToString( ASSETINGEST_STEP_CONVERT ) ) );
        row[ "problems" ] = CORE::CVariant( CAssetCatalogue::JoinLines( outcome.problems ) );
    }

    // the files rows of the removed originals go with their assets
    SConversionRows rows;
    rows.assets = assetRows;
    rows.pack = packRows;
    rows.facets = facetRows;
    rows.conversion = conversionRows;
    rows.history = historyRows;
    rows.facetQuery = facetQuery;
    for ( CORE::CString::StringSet::const_iterator r=removedOriginals.begin(); r!=removedOriginals.end(); ++r )
        rows.removedFileAssetIds.insert( (*r) );

    // one index at a time: a client has one transaction
    MT::CScopeMutex indexLock( m_store.GetIndexLock() );
    if ( catalogue.HasTransactions() )
    {
        COMCORE::CDatabaseTransaction transaction( catalogue.GetClient() );
        if ( !transaction.IsActive() )
        {
            message = "the transaction of the conversion cannot begin";
            return ASSETSTORE_STATUS_DATABASE_ERROR;
        }
        const TAssetStoreStatus status = WriteConversionRows( catalogue, rows, message );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
        if ( CORE::DATATABLE_STATUS_OK != transaction.Commit() )
        {
            message = "the transaction of the conversion was not committed";
            return ASSETSTORE_STATUS_DATABASE_ERROR;
        }
        return ASSETSTORE_STATUS_OK;
    }
    return WriteConversionRows( catalogue, rows, message );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackConversion::Convert( const CORE::CString& packId, CAssetConversionOutcome& outcome )
{GUCEF_TRACE;

    outcome = CAssetConversionOutcome();
    outcome.packId = packId;
    outcome.jobId = m_jobId.IsNULLOrEmpty() ? MakeJobId() : m_jobId;
    const Int64 startedAtMs = NowUtcMs();

    CIAssetConverterHook* hook = m_store.GetConverterHook();
    if ( !m_store.IsOpen() )
        return FailOutcome( outcome, ASSETSTORE_STATUS_INVALID_ARGUMENT, "the primary store is not open" );
    if ( GUCEF_NULL == hook )
        return FailOutcome( outcome, ASSETSTORE_STATUS_UNSUPPORTED, "no converter is attached to the primary store" );

    CAssetPackRecord pack;
    TAssetStoreStatus status = m_store.GetPack( packId, pack );
    if ( ASSETSTORE_STATUS_OK != status )
        return FailOutcome( outcome, status, "there is no pack " + packId + " in the catalogue" );
    outcome.state = pack.state;
    if ( !AssetPackStateIsIngested( pack.state ) )
        return FailOutcome( outcome, ASSETSTORE_STATUS_INVALID_ARGUMENT, "the pack is not ingested (its state is " + CORE::CString( AssetPackStateToString( pack.state ) ) + ")" );
    if ( !m_store.IsLocationOnline( pack.locationId ) )
        return FailOutcome( outcome, ASSETSTORE_STATUS_UNAVAILABLE, "the storage location " + pack.locationId + " that holds the pack is not reachable" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CPackStorageLayout layout( pack.rootPath );
    const CORE::CString locationRoot = m_store.GetLocationRoot( pack.locationId );
    const CORE::CString originalFolder = layout.GetOriginalFolder( locationRoot );
    if ( !vfs.DirExists( originalFolder ) )
        return FailOutcome( outcome, ASSETSTORE_STATUS_UNSUPPORTED, "the pack is referenced in place: its converted files would have to go into the folder of the pack, which the store does not write to" );

    TAssetRecordVector assets;
    status = m_store.GetAssets( packId, assets );
    if ( ASSETSTORE_STATUS_OK != status )
        return FailOutcome( outcome, status, "the assets of the pack cannot be read: " + m_store.GetLastErrorMessage() );
    bool convertible = false;
    bool originalsGone = false;
    for ( size_t a=0; a<assets.size(); ++a )
    {
        if ( !hook->CanConvert( assets[ a ] ) )
            continue;
        convertible = true;
        if ( assets[ a ].files.empty() && !assets[ a ].convertedPath.IsNULLOrEmpty() )
            originalsGone = true;
    }
    if ( !convertible )
        return FailOutcome( outcome, ASSETSTORE_STATUS_UNSUPPORTED, "there is nothing in the pack that the converter can convert" );
    if ( originalsGone )
        return FailOutcome( outcome, ASSETSTORE_STATUS_UNSUPPORTED, "the originals of the pack were removed (keep: converted): it cannot be converted again" );

    CAssetConversionRequest request;
    request.pack = pack;
    request.sourceVfsPath = originalFolder;
    request.sourcePrefix = layout.GetOriginalPrefix() + "/";
    const CORE::CString profile = hook->GetProfileName();
    request.convertedVfsFolder = layout.GetIntermediateFolder( locationRoot, profile );
    request.convertedPathPrefix = layout.GetIntermediatePrefix( profile );
    if ( layout.IsAreaLayout() )
    {
        request.cookedVfsFolder = layout.GetCookedFolder( locationRoot, profile );
        request.cookedPathPrefix = layout.GetCookedPrefix( profile );
    }
    CStoredPackObserver observer( m_store, outcome.jobId, packId, m_cancel );
    request.observer = GUCEF_NULL != m_observer ? m_observer : &observer;
    request.reconvert = false;
    {
        CORE::CString prepareMessage;
        const TAssetStoreStatus prepared = OnPrepare( pack, assets, locationRoot, request, prepareMessage );
        if ( ASSETSTORE_STATUS_OK != prepared )
            return FailOutcome( outcome, prepared, prepareMessage );
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackConversion: Converting pack " << packId << " \"" << pack.title << "\" with " << hook->GetName() << " for the profile " << profile
                      << ( request.assetGlobs.empty() ? CORE::CString::Empty : ", " + CORE::ToString( (UInt32) request.assetGlobs.size() ) + " asset filter(s)" )
                      << ( request.cookOnly ? ", cook only" : ( request.noCook ? ", no cook" : "" ) ) );
    if ( m_notifyEvents )
        m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionProgressEvent, outcome.jobId, packId, false, 0, (UInt32) assets.size(), CORE::CString::Empty, ASSETSTORE_STATUS_OK, "Starting" );

    TAssetRecordVector converted = assets;
    CAssetConversionOutcome hookOutcome;
    hookOutcome.packId = packId;
    hookOutcome.jobId = outcome.jobId;
    TAssetStoreStatus hookStatus = ASSETSTORE_STATUS_FAILED;
    try
    {
        hookStatus = hook->ConvertPack( request, converted, hookOutcome );
    }
    catch ( ... )
    {
        hookStatus = ASSETSTORE_STATUS_FAILED;
        hookOutcome.message = "an exception ended the conversion";
    }

    // a run of a part of the pack leaves the converted paths of the other assets as they were
    if ( !request.assetGlobs.empty() )
    {
        for ( size_t a=0; a<converted.size() && a<assets.size(); ++a )
        {
            const CORE::CString relative = request.GetSourceRelativePath( assets[ a ] );
            if ( relative.IsNULLOrEmpty() || !request.IsInScope( relative ) )
            {
                converted[ a ].convertedPath = assets[ a ].convertedPath;
                converted[ a ].convertedFormat = assets[ a ].convertedFormat;
            }
        }
    }
    {
        CORE::CString convertedMessage;
        const TAssetStoreStatus seen = OnConverted( request, hookStatus, hookOutcome, assets, converted, convertedMessage );
        if ( ASSETSTORE_STATUS_OK != seen )
        {
            if ( m_notifyEvents )
                m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionFailedEvent, outcome.jobId, packId, false, 0, 0, CORE::CString::Empty, seen, convertedMessage );
            return FailOutcome( outcome, seen, convertedMessage );
        }
    }

    // what the outcome does to the pack; an earlier conversion stays when this one is refused or fails
    const bool hadConverted = CAssetConversionRecord::OutcomeConverted == pack.conversion.outcome && !pack.formatsConverted.empty();
    CAssetPackRecord updated( pack );
    TAssetRecordVector updatedAssets = assets;
    CORE::CString message;
    const TAssetStoreStatus applied = ApplyOutcome( hookStatus, hookOutcome, converted, updated, updatedAssets, outcome.problems, message );
    outcome.record = updated.conversion;
    if ( ASSETSTORE_STATUS_OK != applied )
    {
        outcome.status = applied;
        outcome.message = message;
        if ( m_notifyEvents )
            m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionFailedEvent, outcome.jobId, packId, false, 0, 0, CORE::CString::Empty, applied, message );
        OnCommitted( false );
        return applied;
    }
    if ( hadConverted && ASSETSTORE_STATUS_OK != hookStatus )
    {
        updated = pack;
        updatedAssets = assets;
    }
    outcome.status = hookStatus;
    outcome.state = updated.state;
    outcome.message = hookOutcome.message;
    outcome.record = updated.conversion;

    // the originals of what converted go when the keep policy is "converted", after the catalogue says so
    CORE::CString::StringSet removedAssetIds;
    CORE::CString::StringVector removedPaths;
    if ( ASSETSTORE_STATUS_OK == hookStatus && ASSETKEEP_CONVERTED == pack.keep && m_removeOriginals )
    {
        for ( size_t a=0; a<updatedAssets.size(); ++a )
        {
            if ( updatedAssets[ a ].convertedPath.IsNULLOrEmpty() || updatedAssets[ a ].files.empty() )
                continue;
            removedAssetIds.insert( updatedAssets[ a ].assetId );
            for ( size_t f=0; f<updatedAssets[ a ].files.size(); ++f )
                removedPaths.push_back( layout.Resolve( locationRoot, updatedAssets[ a ].files[ f ].path ) );
            updatedAssets[ a ].files.clear();
        }
    }

    {
        CORE::CString commitMessage;
        status = OnCommit( request, hookStatus, pack, assets, commitMessage );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            outcome.status = status;
            outcome.message = commitMessage;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackConversion: Pack " << packId << ": " << commitMessage );
            if ( m_notifyEvents )
                m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionFailedEvent, outcome.jobId, packId, false, 0, 0, CORE::CString::Empty, status, commitMessage );
            OnCommitted( false );
            return status;
        }
    }
    CORE::CString persistMessage;
    status = Persist( updated, updatedAssets, ASSETSTORE_STATUS_OK == hookStatus, removedAssetIds, outcome, startedAtMs, persistMessage );
    OnCommitted( ASSETSTORE_STATUS_OK == status );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        outcome.status = status;
        outcome.message = persistMessage;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackConversion: Pack " << packId << ": " << persistMessage );
        if ( m_notifyEvents )
            m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionFailedEvent, outcome.jobId, packId, false, 0, 0, CORE::CString::Empty, status, persistMessage );
        return status;
    }
    for ( size_t p=0; p<removedPaths.size(); ++p )
        vfs.DeleteFile( removedPaths[ p ], true );

    if ( !m_notifyEvents )
    {
        // the caller reports the end itself
    }
    else
    if ( ASSETSTORE_STATUS_OK == hookStatus || ASSETSTORE_STATUS_REFUSED == hookStatus )
    {
        m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionFinishedEvent, outcome.jobId, packId, false, (UInt32) assets.size(), (UInt32) assets.size(),
                                       CORE::CString::Empty, hookStatus, hookOutcome.message );
    }
    else
    {
        m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionFailedEvent, outcome.jobId, packId, false, 0, 0, CORE::CString::Empty, hookStatus, hookOutcome.message );
    }
    return outcome.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackConversion::ConvertPending( CAssetConversionOutcome& outcome )
{GUCEF_TRACE;

    outcome = CAssetConversionOutcome();
    outcome.jobId = MakeJobId();
    CORE::CString::StringVector packIds;
    m_store.GetPacksPendingConversion( packIds );
    for ( size_t p=0; p<packIds.size(); ++p )
    {
        if ( IsCancelRequested() )
        {
            outcome.status = ASSETSTORE_STATUS_CANCELLED;
            outcome.message = "the conversion of the pending packs was cancelled";
            return outcome.status;
        }
        CAssetConversionOutcome single;
        const TAssetStoreStatus status = Convert( packIds[ p ], single );
        if ( ASSETSTORE_STATUS_OK == status )
            ++outcome.packsConverted;
        else
        if ( ASSETSTORE_STATUS_REFUSED == status )
            ++outcome.packsRefused;
        else
        if ( ASSETSTORE_STATUS_CANCELLED == status )
        {
            outcome.status = ASSETSTORE_STATUS_CANCELLED;
            outcome.message = single.message;
            return outcome.status;
        }
        else
        if ( ASSETSTORE_STATUS_UNSUPPORTED != status )
            ++outcome.packsFailed;
        outcome.problems.insert( outcome.problems.end(), single.problems.begin(), single.problems.end() );
    }
    outcome.status = ASSETSTORE_STATUS_OK;
    outcome.message = CORE::ToString( outcome.packsConverted ) + " converted, " + CORE::ToString( outcome.packsRefused ) + " refused, " + CORE::ToString( outcome.packsFailed ) + " failed";
    return outcome.status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
