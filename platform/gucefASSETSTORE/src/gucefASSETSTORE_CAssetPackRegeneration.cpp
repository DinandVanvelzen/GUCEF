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

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
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
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTPIPELINE_H
#include "gucefASSETSTORE_CAssetIngestPipeline.h"
#define GUCEF_ASSETSTORE_CASSETINGESTPIPELINE_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTPIPELINE_H ? */
#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */

#include "gucefASSETSTORE_CAssetPackRegeneration.h"

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
Join( const CORE::CString& a, const CORE::CString& b )
{GUCEF_TRACE;

    return CPackStorageLayout::Join( a, b );
}

CORE::CString
SafeName( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString safe;
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        const char c = text[ i ];
        safe += ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c || '_' == c ) ? c : '_';
    }
    return safe;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPackRegeneration::CAssetPackRegeneration( CPrimaryAssetStore& store, const CORE::CFutureResultSource* cancel, CIAssetConversionObserver* observer )
    : CAssetPackConversion( store, cancel )
    , m_mode( MODE_INTERMEDIATE )
    , m_globs()
    , m_cook( true )
    , m_targets()
    , m_acceptErrors( false )
    , m_made()
    , m_generations( GUCEF_NULL )
    , m_stagingRoot()
    , m_areas()
    , m_currentReport()
    , m_stagedReport()
    , m_pack()
    , m_before()
    , m_after()
    , m_ran( false )
    , m_replaced( false )
    , m_committed( false )
    , m_report( "regeneration", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

    m_observer = observer;
    m_notifyEvents = false;
    m_removeOriginals = false;
    m_historyOrigin = "regeneration";
}

/*-------------------------------------------------------------------------*/

CAssetPackRegeneration::~CAssetPackRegeneration()
{GUCEF_TRACE;

    GUCEF_DELETE m_generations;
    m_generations = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackRegeneration::SetJob( const CORE::CString& jobId, const CORE::CString& origin, const CORE::CString& audit )
{GUCEF_TRACE;

    m_jobId = jobId;
    if ( !origin.IsNULLOrEmpty() )
        m_historyOrigin = origin;
    m_historyPolicy = audit;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CAssetPackRegeneration::GetReport( void ) const
{GUCEF_TRACE;

    return m_report;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackRegeneration::Reset( void )
{GUCEF_TRACE;

    GUCEF_DELETE m_generations;
    m_generations = GUCEF_NULL;
    m_stagingRoot.Clear();
    m_areas.clear();
    m_currentReport.Clear();
    m_stagedReport.Clear();
    m_before.Clear();
    m_after.Clear();
    m_ran = false;
    m_replaced = false;
    m_committed = false;
    m_report.Clear();
    m_report.SetName( "regeneration" );
    m_report.SetNodeType( GUCEF_DATATYPE_OBJECT );
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetPackRegeneration::GetFolderBytes( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( vfsFolder.IsNULLOrEmpty() || !vfs.DirExists( vfsFolder ) )
        return 0;
    CORE::CString::StringVector files;
    CAssetIngestPipeline::ListFiles( vfsFolder, files );
    UInt64 bytes = 0;
    for ( size_t f=0; f<files.size(); ++f )
        bytes += vfs.GetFileSize( Join( vfsFolder, files[ f ] ) );
    return bytes;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackRegeneration::Check( CPrimaryAssetStore& store, const CORE::CString& packId, EMode mode, CORE::CString& problem,
                               UInt64& intermediateBytes, UInt64& cookedBytes, bool& hasPrevious )
{GUCEF_TRACE;

    intermediateBytes = 0;
    cookedBytes = 0;
    hasPrevious = false;
    CAssetPackRecord pack;
    if ( ASSETSTORE_STATUS_OK != store.GetPack( packId, pack ) )
    {
        problem = "there is no pack " + packId + " in the catalogue";
        return ASSETSTORE_STATUS_NOT_FOUND;
    }
    const CPackStorageLayout layout( pack.rootPath );
    const CORE::CString root = store.GetLocationRoot( pack.locationId );
    CAssetPackGenerations generations( layout, root );
    hasPrevious = generations.HasPrevious();
    if ( !layout.IsAreaLayout() )
    {
        problem = "the pack is stored in the earlier layout (originals and converted files in one folder): its outputs have no generations; convert it again instead";
        return ASSETSTORE_STATUS_UNSUPPORTED;
    }
    if ( GUCEF_NULL == store.GetConverterHook() )
    {
        problem = "no converter is attached to the primary store";
        return ASSETSTORE_STATUS_UNSUPPORTED;
    }
    if ( !store.IsLocationOnline( pack.locationId ) )
    {
        problem = "the storage location " + pack.locationId + " that holds the pack is not reachable";
        return ASSETSTORE_STATUS_UNAVAILABLE;
    }
    intermediateBytes = GetFolderBytes( generations.GetCurrentFolder( CPackStorageLayout::IntermediateArea ) );
    cookedBytes = GetFolderBytes( generations.GetCurrentFolder( CPackStorageLayout::CookedArea ) );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( layout.GetOriginalFolder( root ) ) )
    {
        problem = "the originals of the pack are not in the store (a pack referenced in place, or one whose originals were removed)";
        return ASSETSTORE_STATUS_UNSUPPORTED;
    }
    if ( MODE_TARGET == mode && !vfs.DirExists( generations.GetCurrentFolder( CPackStorageLayout::IntermediateArea ) ) )
    {
        problem = "the pack has no intermediate resources to cook: regenerate the intermediate first";
        return ASSETSTORE_STATUS_NOT_FOUND;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackRegeneration::Regenerate( const CORE::CString& packId, EMode mode, const CORE::CString::StringVector& assetGlobs, bool cook,
                                    const CORE::CString::StringVector& cookTargets, bool acceptErrors, const CORE::CDataNode& made,
                                    CAssetConversionOutcome& outcome )
{GUCEF_TRACE;

    Reset();
    m_mode = mode;
    m_globs = assetGlobs;
    m_cook = MODE_TARGET == mode || cook;
    m_targets = cookTargets;
    m_acceptErrors = acceptErrors;
    m_made = made;
    m_report.SetAttribute( "mode", MODE_TARGET == mode ? "target" : "intermediate" );
    TAssetStoreStatus status = Convert( packId, outcome );
    FinishReport();
    DiscardStaging();
    if ( ASSETSTORE_STATUS_OK == status && !m_replaced )
        status = ASSETSTORE_STATUS_FAILED;
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackRegeneration::OnPrepare( const CAssetPackRecord& pack, const TAssetRecordVector& assets, const CORE::CString& locationRoot,
                                   CAssetConversionRequest& request, CORE::CString& message )
{GUCEF_TRACE;

    m_pack = pack;
    const CPackStorageLayout layout( pack.rootPath );
    if ( !layout.IsAreaLayout() )
    {
        message = "the pack is stored in the earlier layout (originals and converted files in one folder): its outputs have no generations; convert it again instead";
        return ASSETSTORE_STATUS_UNSUPPORTED;
    }
    GUCEF_DELETE m_generations;
    m_generations = GUCEF_NEW CAssetPackGenerations( layout, locationRoot );
    CORE::CString note;
    if ( m_generations->Recover( note ) )
        m_report.SetAttribute( "recovered", note );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString intermediate = m_generations->GetCurrentFolder( CPackStorageLayout::IntermediateArea );
    const CORE::CString cooked = m_generations->GetCurrentFolder( CPackStorageLayout::CookedArea );
    if ( MODE_TARGET == m_mode && !vfs.DirExists( intermediate ) )
    {
        message = "the pack has no intermediate resources to cook: regenerate the intermediate first";
        return ASSETSTORE_STATUS_NOT_FOUND;
    }

    // the staging folder of this run, in the location of the pack so the swap is a rename
    const CORE::CString job = SafeName( m_jobId.IsNULLOrEmpty() ? CORE::ToString( (UInt64) NowUtcMs() ) : m_jobId );
    m_stagingRoot = Join( Join( locationRoot, CPackStorageLayout::StagingArea ), "regen-" + SafeName( pack.packId ) + "-" + job );
    if ( vfs.DirExists( m_stagingRoot ) )
        vfs.DeleteDir( m_stagingRoot, true );
    if ( !vfs.CreateDir( m_stagingRoot ) )
    {
        message = "the staging folder " + m_stagingRoot + " cannot be made";
        return ASSETSTORE_STATUS_IO_ERROR;
    }
    const CORE::CString stagedIntermediate = Join( m_stagingRoot, CPackStorageLayout::IntermediateArea );
    const CORE::CString stagedCooked = Join( m_stagingRoot, CPackStorageLayout::CookedArea );
    m_areas.clear();
    if ( MODE_INTERMEDIATE == m_mode )
        m_areas.push_back( CPackStorageLayout::IntermediateArea );
    if ( m_cook )
        m_areas.push_back( CPackStorageLayout::CookedArea );

    // a run of a part of the pack starts from a copy of the outputs, the other assets keep theirs
    if ( !m_globs.empty() )
    {
        if ( MODE_INTERMEDIATE == m_mode && vfs.DirExists( intermediate ) && !CAssetIngestPipeline::CopyTree( intermediate, stagedIntermediate ) )
        {
            message = "the intermediate resources of the pack cannot be copied to " + stagedIntermediate;
            return ASSETSTORE_STATUS_IO_ERROR;
        }
        if ( m_cook && vfs.DirExists( cooked ) && !CAssetIngestPipeline::CopyTree( cooked, stagedCooked ) )
        {
            message = "the cooked resources of the pack cannot be copied to " + stagedCooked;
            return ASSETSTORE_STATUS_IO_ERROR;
        }
    }

    request.reconvert = MODE_INTERMEDIATE == m_mode;
    request.assetGlobs = m_globs;
    request.cookTargets = m_targets;
    request.keepPartialResult = m_acceptErrors;
    request.cookedVfsFolder = stagedCooked;
    request.cookedPathPrefix = layout.GetCookedPrefix( CORE::CString::Empty );
    if ( MODE_TARGET == m_mode )
    {
        // the intermediate of the pack is read, the cooked files and the report of the cook go to the staging folder
        request.convertedVfsFolder = intermediate;
        request.cookOnly = true;
        m_currentReport = Join( cooked, "cook-report.json" );
        m_stagedReport = Join( stagedCooked, "cook-report.json" );
        request.reportVfsPath = m_stagedReport;
        if ( !vfs.DirExists( stagedCooked ) )
            vfs.CreateDir( stagedCooked );
    }
    else
    {
        request.convertedVfsFolder = stagedIntermediate;
        request.noCook = !m_cook;
        m_currentReport = Join( intermediate, "report.json" );
        m_stagedReport = Join( stagedIntermediate, "report.json" );
    }
    if ( MODE_TARGET == m_mode && !vfs.FileExists( m_currentReport ) )
        m_before.Load( Join( intermediate, "report.json" ) );
    else
        m_before.Load( m_currentReport );
    m_report.SetAttribute( "staging", m_stagingRoot );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackRegeneration::OnConverted( const CAssetConversionRequest& request, TAssetStoreStatus& hookStatus, CAssetConversionOutcome& hookOutcome,
                                     const TAssetRecordVector& assets, TAssetRecordVector& converted, CORE::CString& message )
{GUCEF_TRACE;

    const bool completed = ASSETSTORE_STATUS_OK == hookStatus || ASSETSTORE_STATUS_FAILED == hookStatus;
    m_ran = completed;
    if ( !completed )
        return ASSETSTORE_STATUS_OK;

    // the report of a run of a part of the pack goes into that of the pack
    if ( !m_globs.empty() && VFS::CVfsGlobal::Instance()->GetVfs().FileExists( m_stagedReport ) )
        CAssetConversionReportSummary::MergeReportFiles( m_currentReport, m_stagedReport, m_stagedReport );
    m_after.Load( m_stagedReport );

    if ( ASSETSTORE_STATUS_FAILED == hookStatus && m_acceptErrors )
    {
        // the caller takes the outputs of a run with errors: the assets that failed have none
        hookOutcome.problems.push_back( "the outputs were put in place although the run had errors (acceptErrors): " + hookOutcome.message );
        hookStatus = ASSETSTORE_STATUS_OK;
    }
    if ( m_after.found && ( !m_globs.empty() || MODE_INTERMEDIATE == m_mode ) && !m_after.assets.empty() )
    {
        // the counts of the conversion record describe the whole pack
        hookOutcome.record.convertedCount = m_after.GetCount( "converted" );
        hookOutcome.record.approximatedCount = m_after.GetCount( "approximated" );
        hookOutcome.record.cachedCount = m_after.GetCount( "cached" );
        hookOutcome.record.skippedCount = m_after.GetCount( "skipped" );
        hookOutcome.record.failedCount = m_after.GetCount( "failed" ) + m_after.GetCount( "refused" );
    }
    if ( MODE_TARGET == m_mode && m_pack.conversion.IsSet() )
    {
        // a cook keeps what the conversion of the intermediate counted
        hookOutcome.record.convertedCount = m_pack.conversion.convertedCount;
        hookOutcome.record.approximatedCount = m_pack.conversion.approximatedCount;
        hookOutcome.record.cachedCount = m_pack.conversion.cachedCount;
        hookOutcome.record.skippedCount = m_pack.conversion.skippedCount;
        hookOutcome.record.failedCount = m_pack.conversion.failedCount;
        hookOutcome.record.reportPath = m_pack.conversion.reportPath;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackRegeneration::OnCommit( const CAssetConversionRequest& request, TAssetStoreStatus hookStatus, const CAssetPackRecord& before,
                                  const TAssetRecordVector& beforeAssets, CORE::CString& message )
{GUCEF_TRACE;

    if ( ASSETSTORE_STATUS_OK != hookStatus || GUCEF_NULL == m_generations )
    {
        // nothing is put in place: the outputs of the pack stay as they were
        DiscardStaging();
        return ASSETSTORE_STATUS_OK;
    }
    CORE::CDataNode generation;
    CAssetPackGenerations::MakeGeneration( before, beforeAssets, &m_made, generation );
    CORE::CString problem;
    if ( !m_generations->Replace( m_stagingRoot, m_areas, generation, problem ) )
    {
        message = "the regenerated outputs could not be put in place: " + problem;
        DiscardStaging();
        return ASSETSTORE_STATUS_IO_ERROR;
    }
    m_replaced = true;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackRegeneration::OnCommitted( bool persisted )
{GUCEF_TRACE;

    if ( !m_replaced || GUCEF_NULL == m_generations )
        return;
    if ( persisted )
    {
        m_generations->Commit();
        m_committed = true;
        return;
    }
    // the catalogue did not take it: the files go back
    CORE::CString problem;
    m_generations->Undo( problem );
    m_replaced = false;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackRegeneration::DiscardStaging( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !m_stagingRoot.IsNULLOrEmpty() && vfs.DirExists( m_stagingRoot ) )
        vfs.DeleteDir( m_stagingRoot, true );
}

/*-------------------------------------------------------------------------*/

void
CAssetPackRegeneration::FinishReport( void )
{GUCEF_TRACE;

    m_before.SerializeCounts( *m_report.AddChild( "before", GUCEF_DATATYPE_OBJECT ) );
    m_after.SerializeCounts( *m_report.AddChild( "after", GUCEF_DATATYPE_OBJECT ) );
    UInt32 improved = 0;
    UInt32 regressed = 0;
    if ( m_after.found )
        CAssetConversionReportSummary::Compare( m_before, m_after, MaxListedChanges, *m_report.AddChild( "changes", GUCEF_DATATYPE_OBJECT ), improved, regressed );
    m_report.SetAttribute( "ran", m_ran );
    m_report.SetAttribute( "replaced", m_replaced && m_committed );
    m_report.SetValuesOfChildByName( "areas", m_areas );
    m_report.SetAttribute( "reportPath", m_replaced && m_committed ? m_currentReport : m_before.reportPath );
    m_report.SetAttribute( "previousGeneration", GUCEF_NULL != m_generations && m_generations->HasPrevious() ? m_generations->GetPreviousFolder() : CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackRegeneration::Rollback( const CORE::CString& packId, const CORE::CDataNode& made, CAssetConversionOutcome& outcome )
{GUCEF_TRACE;

    Reset();
    m_report.SetAttribute( "mode", "rollback" );
    outcome = CAssetConversionOutcome();
    outcome.packId = packId;
    outcome.jobId = m_jobId.IsNULLOrEmpty() ? MakeJobId() : m_jobId;
    const Int64 startedAtMs = NowUtcMs();
    CPrimaryAssetStore& store = GetStore();
    CAssetPackRecord pack;
    TAssetRecordVector assets;
    if ( ASSETSTORE_STATUS_OK != store.GetPack( packId, pack ) )
    {
        outcome.status = ASSETSTORE_STATUS_NOT_FOUND;
        outcome.message = "there is no pack " + packId + " in the catalogue";
        return outcome.status;
    }
    if ( ASSETSTORE_STATUS_OK != store.GetAssets( packId, assets ) )
    {
        outcome.status = ASSETSTORE_STATUS_DATABASE_ERROR;
        outcome.message = "the assets of the pack cannot be read";
        return outcome.status;
    }
    if ( !store.IsLocationOnline( pack.locationId ) )
    {
        outcome.status = ASSETSTORE_STATUS_UNAVAILABLE;
        outcome.message = "the storage location " + pack.locationId + " that holds the pack is not reachable";
        return outcome.status;
    }
    const CPackStorageLayout layout( pack.rootPath );
    const CORE::CString root = store.GetLocationRoot( pack.locationId );
    GUCEF_DELETE m_generations;
    m_generations = GUCEF_NEW CAssetPackGenerations( layout, root );
    CORE::CString note;
    if ( m_generations->Recover( note ) )
        m_report.SetAttribute( "recovered", note );
    if ( !m_generations->HasPrevious() )
    {
        outcome.status = ASSETSTORE_STATUS_NOT_FOUND;
        outcome.message = "there is no previous generation of the outputs of pack " + packId + " to roll back to";
        FinishReport();
        return outcome.status;
    }
    m_currentReport = Join( m_generations->GetCurrentFolder( CPackStorageLayout::IntermediateArea ), "report.json" );
    m_before.Load( m_currentReport );

    CORE::CDataNode current;
    CAssetPackGenerations::MakeGeneration( pack, assets, &made, current );
    CORE::CDataNode restored;
    CORE::CString problem;
    if ( !m_generations->Rollback( current, restored, m_areas, problem ) )
    {
        outcome.status = ASSETSTORE_STATUS_IO_ERROR;
        outcome.message = problem;
        FinishReport();
        return outcome.status;
    }
    m_replaced = true;
    m_ran = true;
    CAssetPackRecord rolledBack( pack );
    TAssetRecordVector rolledBackAssets( assets );
    CAssetPackGenerations::ApplyGeneration( restored, rolledBack, rolledBackAssets );
    outcome.status = ASSETSTORE_STATUS_OK;
    outcome.state = rolledBack.state;
    outcome.record = rolledBack.conversion;
    outcome.message = "the outputs of the previous generation (" + CORE::CString( restored.FindChild( "made" ) != GUCEF_NULL ?
        restored.FindChild( "made" )->GetAttributeValueOrChildValueByName( "operation" ).AsString( CORE::CString( "unknown" ), true ) : CORE::CString( "unknown" ) ) + ") are current again";
    CORE::CString message;
    const TAssetStoreStatus written = RestoreState( rolledBack, rolledBackAssets, outcome, startedAtMs, message );
    OnCommitted( ASSETSTORE_STATUS_OK == written );
    if ( ASSETSTORE_STATUS_OK != written )
    {
        outcome.status = written;
        outcome.message = message;
    }
    m_after.Load( m_currentReport );
    FinishReport();
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
