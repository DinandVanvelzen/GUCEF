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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

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

#ifndef ASSETCONVERTERLIB_CASSETPIPELINE_H
#include "AssetConverterLib_CAssetPipeline.h"
#define ASSETCONVERTERLIB_CASSETPIPELINE_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#include "AssetConverterLib_CAssetPipelineOptions.h"
#define ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H ? */

#ifndef ASSETCONVERTERLIB_CIASSETPIPELINEOBSERVER_H
#include "AssetConverterLib_CIAssetPipelineObserver.h"
#define ASSETCONVERTERLIB_CIASSETPIPELINEOBSERVER_H
#endif /* ASSETCONVERTERLIB_CIASSETPIPELINEOBSERVER_H ? */

#ifndef ASSETCONVERTERLIB_CINTERMEDIATELOCK_H
#include "AssetConverterLib_CIntermediateLock.h"
#define ASSETCONVERTERLIB_CINTERMEDIATELOCK_H
#endif /* ASSETCONVERTERLIB_CINTERMEDIATELOCK_H ? */

#ifndef ASSETCONVERTERLIB_CPLUGINSTACKENTRY_H
#include "AssetConverterLib_CPluginStackEntry.h"
#define ASSETCONVERTERLIB_CPLUGINSTACKENTRY_H
#endif /* ASSETCONVERTERLIB_CPLUGINSTACKENTRY_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineInspectionOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineLicenceOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CCOOKPHASELOCK_H
#include "AssetStoreConverterHook_CCookPhaseLock.h"
#define ASSETSTORECONVERTERHOOK_CCOOKPHASELOCK_H
#endif /* ASSETSTORECONVERTERHOOK_CCOOKPHASELOCK_H ? */

#include "AssetStoreConverterHook_CPipelineRunOperations.h"

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

const UInt32 MaxExamples = 5;
const UInt32 MaxSlowest = 5;
const UInt32 MaxApproximationGroups = 20;
const UInt32 MaxFailureGroups = 50;

/** Gives the progress of the pipeline to the observer of the call and its cancel request to the pipeline */
class CCallPipelineObserver : public ASSETCONVERTER::CIAssetPipelineObserver
{
    public:

    explicit CCallPipelineObserver( ASSETSTORE::CIAssetConversionObserver* observer )
        : ASSETCONVERTER::CIAssetPipelineObserver()
        , m_observer( observer )
        , m_cookLock()
    {GUCEF_TRACE;
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

/** A group of findings of the same source and pattern */
struct SGroup
{
    CORE::CString source;
    CORE::CString pattern;
    UInt32 count;
    CORE::CString::StringVector examples;
};

typedef GUCEF::vector< SGroup > TGroupVector;

void
AddToGroups( TGroupVector& groups, const CORE::CString& source, const CORE::CString& pattern, const CORE::CString& example )
{GUCEF_TRACE;

    for ( size_t g=0; g<groups.size(); ++g )
    {
        if ( groups[ g ].source == source && groups[ g ].pattern == pattern )
        {
            ++groups[ g ].count;
            if ( groups[ g ].examples.size() < MaxExamples && groups[ g ].examples.end() == std::find( groups[ g ].examples.begin(), groups[ g ].examples.end(), example ) )
                groups[ g ].examples.push_back( example );
            return;
        }
    }
    SGroup group;
    group.source = source;
    group.pattern = pattern;
    group.count = 1;
    group.examples.push_back( example );
    groups.push_back( group );
}

bool
LargerGroup( const SGroup& a, const SGroup& b )
{GUCEF_TRACE;

    return a.count > b.count;
}

void
WriteGroups( TGroupVector groups, UInt32 maxGroups, const char* sourceName, CORE::CDataNode& list )
{GUCEF_TRACE;

    std::stable_sort( groups.begin(), groups.end(), LargerGroup );
    for ( size_t g=0; g<groups.size() && g<maxGroups; ++g )
    {
        CORE::CDataNode* item = list.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( sourceName, groups[ g ].source );
        item->SetAttribute( "pattern", groups[ g ].pattern );
        item->SetAttribute( "count", groups[ g ].count );
        item->SetValuesOfChildByName( "examples", groups[ g ].examples );
    }
}

struct SAssetTime
{
    CORE::CString assetPath;
    UInt32 durationMs;
};

bool
SlowerAsset( const SAssetTime& a, const SAssetTime& b )
{GUCEF_TRACE;

    return a.durationMs > b.durationMs;
}

/** Whether the token looks like a path or a file name */
bool
IsPathLike( const CORE::CString& token )
{GUCEF_TRACE;

    if ( token.HasChar( '/' ) >= 0 || token.HasChar( '\\' ) >= 0 )
        return true;
    const Int32 dot = token.HasChar( '.', false );
    if ( dot > 0 && (UInt32) dot + 1 < token.Length() )
    {
        const char next = token[ (UInt32) dot + 1 ];
        return ( next >= 'a' && next <= 'z' ) || ( next >= 'A' && next <= 'Z' );
    }
    return false;
}

/** The document of a run, see the class description */
ASSETSTORE::TAssetStoreStatus
FinishRun( CPipelineOperationCall& call, const CORE::CString& packId, ASSETCONVERTER::TPipelineMode mode,
           const ASSETCONVERTER::CConversionReport& report, bool ran, UInt64 durationMs, const CORE::CString& reportVfsPath )
{GUCEF_TRACE;

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "operation", call.request.operation );
    document.SetAttribute( "packId", packId );
    document.SetAttribute( "mode", ASSETCONVERTER::ToString( mode ) );
    CPipelineInspectionOperations::AddCounts( report, *document.AddChild( "counts", GUCEF_DATATYPE_OBJECT ) );
    document.SetAttribute( "durationMs", durationMs );
    document.SetAttribute( "cancelled", report.IsCancelled() );
    document.SetAttribute( "refused", report.IsRefused() );
    CPipelineOperationCall::SetTextOrNull( document, "refusalReason", report.GetRefusalReason() );
    document.SetAttribute( "locked", report.IsLocked() );
    CPipelineOperationCall::SetTextOrNull( document, "lockHolder", report.GetLockHolder() );
    document.SetAttribute( "clean", ran && !report.HasFailures() && !report.IsCancelled() && !report.IsRefused() && !report.IsLocked() );

    TGroupVector failureGroups;
    TGroupVector approximationGroups;
    TGroupVector findingGroups;
    GUCEF::vector< SAssetTime > times;
    CORE::CDataNode* failures = document.AddChild( "failures", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* approximations = document.AddChild( "approximations", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* assets = document.AddChild( "assets", GUCEF_DATATYPE_ARRAY );
    const ASSETCONVERTER::CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
    {
        const ASSETCONVERTER::CConversionReportEntry& entry = entries[ e ];
        const CORE::CString subject = entry.assetPath.IsNULLOrEmpty() ? CORE::CString( "pack" ) : entry.assetPath;
        if ( !entry.assetPath.IsNULLOrEmpty() )
        {
            CORE::CDataNode* asset = assets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            asset->SetAttribute( "assetPath", entry.assetPath );
            asset->SetAttribute( "state", ASSETCONVERTER::ToString( entry.GetStatus() ) );
            asset->SetAttribute( "durationMs", entry.durationInMs );
            SAssetTime time;
            time.assetPath = entry.assetPath;
            time.durationMs = entry.durationInMs;
            times.push_back( time );
        }
        for ( size_t i=0; i<entry.items.size(); ++i )
        {
            const ASSETCONVERTER::CConversionReportItem& item = entry.items[ i ];
            const CORE::CString pattern = CPipelineRunOperations::MakePattern( item.message );
            if ( ASSETCONVERTER::REPORTITEM_ERROR == item.kind || ASSETCONVERTER::REPORTITEM_APPROXIMATION == item.kind )
            {
                const bool error = ASSETCONVERTER::REPORTITEM_ERROR == item.kind;
                CORE::CDataNode* finding = ( error ? failures : approximations )->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                finding->SetAttribute( "assetPath", subject );
                finding->SetAttribute( "source", item.source );
                finding->AddChild( call.Untrusted( "message", "importer-message", item.message ) );
                AddToGroups( error ? failureGroups : approximationGroups, item.source, pattern, subject );
            }
            if ( ASSETCONVERTER::PIPELINEMODE_VALIDATE_ONLY == mode && ASSETCONVERTER::REPORTITEM_APPROXIMATION != item.kind && ASSETCONVERTER::REPORTITEM_SKIPPED != item.kind )
                AddToGroups( findingGroups, item.source + "|" + ASSETCONVERTER::ToString( item.kind ), pattern, subject );
        }
    }
    WriteGroups( failureGroups, MaxFailureGroups, "source", *document.AddChild( "failureGroups", GUCEF_DATATYPE_ARRAY ) );
    WriteGroups( approximationGroups, MaxApproximationGroups, "source", *document.AddChild( "approximationGroups", GUCEF_DATATYPE_ARRAY ) );
    if ( ASSETCONVERTER::PIPELINEMODE_VALIDATE_ONLY == mode )
    {
        // per validator (the source of the finding) and severity
        CORE::CDataNode* findings = document.AddChild( "validatorFindings", GUCEF_DATATYPE_ARRAY );
        std::stable_sort( findingGroups.begin(), findingGroups.end(), LargerGroup );
        for ( size_t g=0; g<findingGroups.size() && g<MaxFailureGroups; ++g )
        {
            const Int32 bar = findingGroups[ g ].source.HasChar( '|', false );
            CORE::CDataNode* item = findings->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "validator", bar > 0 ? findingGroups[ g ].source.SubstrToIndex( (UInt32) bar, true ) : findingGroups[ g ].source );
            item->SetAttribute( "severity", bar > 0 ? findingGroups[ g ].source.CutChars( (UInt32) bar + 1, true ) : CORE::CString::Empty );
            item->SetAttribute( "pattern", findingGroups[ g ].pattern );
            item->SetAttribute( "count", findingGroups[ g ].count );
            item->SetValuesOfChildByName( "examples", findingGroups[ g ].examples );
        }
    }
    std::stable_sort( times.begin(), times.end(), SlowerAsset );
    CORE::CDataNode* slowest = document.AddChild( "slowest", GUCEF_DATATYPE_ARRAY );
    for ( size_t t=0; t<times.size() && t<MaxSlowest; ++t )
    {
        CORE::CDataNode* item = slowest->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "assetPath", times[ t ].assetPath );
        item->SetAttribute( "durationMs", times[ t ].durationMs );
    }
    if ( !reportVfsPath.IsNULLOrEmpty() && !packId.IsNULLOrEmpty() && VFS::CVfsGlobal::Instance()->GetVfs().FileExists( reportVfsPath ) )
    {
        CPipelineInspectionOperations::AddLink( document, ASSETSTORE::CAssetToolUris::ForPack( packId, "report" ), "report", "application/json" );
        CPipelineInspectionOperations::AddLink( document, ASSETSTORE::CAssetToolUris::ForPack( packId, "report-summary" ), "report summary", "text/plain" );
        CPipelineInspectionOperations::AddLink( document, ASSETSTORE::CAssetToolUris::ForPack( packId, "activity" ), "activity log", "application/json" );
    }

    if ( report.IsRefused() )
        return call.Fail( CPipelineOperationCall::ErrorLicenceRefused, "the pipeline refused the pack: " + report.GetRefusalReason() );
    if ( report.IsLocked() )
        return call.Fail( CPipelineOperationCall::ErrorBusy, "the intermediate of the pack is in use by " + report.GetLockHolder() );
    if ( report.IsCancelled() )
    {
        call.result.status = ASSETSTORE::ASSETSTORE_STATUS_CANCELLED;
        call.result.message = "the run was cancelled; the assets that were done are kept in the cache";
        return call.result.status;
    }
    if ( !ran )
    {
        CORE::CString first;
        for ( size_t e=0; e<entries.size() && first.IsNULLOrEmpty(); ++e )
            for ( size_t i=0; i<entries[ e ].items.size() && first.IsNULLOrEmpty(); ++i )
                if ( ASSETCONVERTER::REPORTITEM_ERROR == entries[ e ].items[ i ].kind && entries[ e ].assetPath.IsNULLOrEmpty() )
                    first = entries[ e ].items[ i ].message;
        if ( !first.IsNULLOrEmpty() || report.GetEntries().empty() )
            return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "the run could not start" + ( first.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + first ) );
    }
    const UInt32 failed = report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_FAILED );
    return call.Succeed( failed > 0 ? CORE::ToString( failed ) + " asset(s) failed, see failureGroups" : CORE::CString( "the run is done" ) );
}

/** The checks before a run that refuse it: the licence and the lock of the intermediate */
bool
CheckBeforeRun( CPipelineOperationCall& call, const ASSETCONVERTER::CAssetPackManifest& manifest, const CConverterLicenceDecision& licence, bool checkLock )
{GUCEF_TRACE;

    if ( !licence.conversionAllowed )
    {
        call.Fail( CPipelineOperationCall::ErrorLicenceRefused, "the licence record does not allow the conversion: " + licence.reason );
        return false;
    }
    CORE::CString holder;
    if ( checkLock && ASSETCONVERTER::CIntermediateLock::GetCurrentHolder( manifest.GetIntermediatePath(), holder ) && !holder.IsNULLOrEmpty() )
    {
        call.Fail( CPipelineOperationCall::ErrorBusy, "the intermediate of the pack is in use by " + holder );
        return false;
    }
    return true;
}

/** The cook profiles of the manifest that the request names; all when it names none */
bool
FilterProfiles( CPipelineOperationCall& call, ASSETCONVERTER::CAssetPackManifest& manifest )
{GUCEF_TRACE;

    if ( call.request.profiles.empty() )
        return true;
    ASSETCONVERTER::CAssetPackManifest::TProfileVector kept;
    for ( size_t p=0; p<call.request.profiles.size(); ++p )
    {
        bool found = false;
        for ( size_t m=0; m<manifest.cookProfiles.size() && !found; ++m )
        {
            if ( manifest.cookProfiles[ m ].name == call.request.profiles[ p ] )
            {
                kept.push_back( manifest.cookProfiles[ m ] );
                found = true;
            }
        }
        if ( !found )
        {
            call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "the pack has no cook profile " + call.request.profiles[ p ] );
            return false;
        }
    }
    manifest.cookProfiles = kept;
    return true;
}

ASSETSTORE::TAssetStoreStatus
RunManifest( CPipelineOperationCall& call, const CORE::CString& packId, const ASSETCONVERTER::CAssetPackManifest& manifest,
             ASSETCONVERTER::CAssetPipelineOptions& options )
{GUCEF_TRACE;

    CCallPipelineObserver observer( call.observer );
    options.observer = &observer;
    ASSETCONVERTER::CAssetPipeline pipeline;
    ASSETCONVERTER::CConversionReport report;
    const UInt64 start = MT::PrecisionTickCount();
    bool ran = false;
    try
    {
        pipeline.RunPack( manifest, options, report );
        ran = true;
    }
    catch ( ... )
    {
        report.AddError( "AssetStoreConverterHook", packId, "an exception ended the run" );
    }
    const UInt64 durationMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();
    const CORE::CString reportPath = ASSETCONVERTER::PIPELINEMODE_VALIDATE_ONLY == options.mode ? CORE::CString::Empty : ASSETCONVERTER::CVfsPaths::Join( manifest.GetIntermediatePath(), "report.json" );
    const ASSETSTORE::TAssetStoreStatus status = FinishRun( call, packId, options.mode, report, ran, durationMs, reportPath );
    // where the run wrote, so that the jobs list can open the result
    if ( ASSETCONVERTER::PIPELINEMODE_VALIDATE_ONLY != options.mode )
        call.Document().SetAttribute( "intermediateFolder", manifest.GetIntermediatePath() );
    return status;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CPipelineRunOperations::MakePattern( const CORE::CString& message )
{GUCEF_TRACE;

    CORE::CString pattern;
    UInt32 i = 0;
    const UInt32 length = message.Length();
    while ( i < length )
    {
        const char c = message[ i ];
        if ( '"' == c || '\'' == c )
        {
            // a quoted name
            UInt32 end = i + 1;
            while ( end < length && message[ end ] != c )
                ++end;
            pattern += "<name>";
            i = end < length ? end + 1 : end;
            continue;
        }
        if ( ' ' == c || '\t' == c || ',' == c || ';' == c || ':' == c || '(' == c || ')' == c || '[' == c || ']' == c )
        {
            pattern += c;
            ++i;
            continue;
        }
        UInt32 end = i;
        while ( end < length && ' ' != message[ end ] && '\t' != message[ end ] && ',' != message[ end ] && ';' != message[ end ] &&
                ':' != message[ end ] && '(' != message[ end ] && ')' != message[ end ] && '[' != message[ end ] && ']' != message[ end ] )
            ++end;
        const CORE::CString token = message.SubstrFromRange( i, end );
        bool digits = !token.IsNULLOrEmpty();
        bool hasDigit = false;
        for ( UInt32 t=0; t<token.Length(); ++t )
        {
            const char d = token[ t ];
            if ( d >= '0' && d <= '9' )
                hasDigit = true;
            else
            if ( '.' != d && '-' != d && 'x' != d )
                digits = false;
        }
        if ( digits && hasDigit )
            pattern += "<n>";
        else
        if ( IsPathLike( token ) )
            pattern += "<path>";
        else
        if ( hasDigit )
            pattern += "<name>";
        else
            pattern += token;
        i = end;
    }
    return pattern;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineRunOperations::ConvertPack( CPipelineOperationCall& call, const CORE::CString& threadPoolName, UInt32 threads )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, true ) )
        return call.result.status;
    if ( !CheckBeforeRun( call, pack.manifest, pack.licence, true ) )
        return call.result.status;
    ASSETCONVERTER::CAssetPackManifest manifest = pack.manifest;
    if ( !FilterProfiles( call, manifest ) )
        return call.result.status;
    ASSETCONVERTER::CAssetPipelineOptions options;
    options.assetGlobs = call.request.assets;
    options.cookTargets = call.request.cookTargets;
    options.force = call.request.force;
    options.reimport = call.request.reimport;
    options.threadPoolName = threadPoolName;
    call.ApplyRedoArguments( options );
    options.threads = threads;
    options.mode = ASSETCONVERTER::PIPELINEMODE_FULL;
    return RunManifest( call, pack.packId, manifest, options );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineRunOperations::CookPack( CPipelineOperationCall& call, const CORE::CString& threadPoolName, UInt32 threads )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, true ) )
        return call.result.status;
    if ( !pack.HasIntermediate() )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the pack " + pack.packId + " has no intermediate to cook from: convert it first" );
    if ( !CheckBeforeRun( call, pack.manifest, pack.licence, true ) )
        return call.result.status;
    ASSETCONVERTER::CAssetPackManifest manifest = pack.manifest;
    if ( !FilterProfiles( call, manifest ) )
        return call.result.status;
    ASSETCONVERTER::CAssetPipelineOptions options;
    options.assetGlobs = call.request.assets;
    options.cookTargets = call.request.cookTargets;
    options.force = call.request.force;
    options.threadPoolName = threadPoolName;
    call.ApplyRedoArguments( options );
    options.threads = threads;
    options.mode = ASSETCONVERTER::PIPELINEMODE_COOK_ONLY;
    return RunManifest( call, pack.packId, manifest, options );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineRunOperations::ValidatePack( CPipelineOperationCall& call, const CORE::CString& threadPoolName, UInt32 threads )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, true ) )
        return call.result.status;
    if ( !CheckBeforeRun( call, pack.manifest, pack.licence, false ) )
        return call.result.status;
    ASSETCONVERTER::CAssetPackManifest manifest = pack.manifest;
    const CORE::CDataNode* validators = call.GetNodeArgument( "validators" );
    if ( GUCEF_NULL != validators )
    {
        ASSETCONVERTER::CPluginStackEntry::TEntryVector entries;
        if ( !ASSETCONVERTER::CPluginStackEntry::DeserializeList( *validators, entries ) )
            return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "validators is a list of { name, settings }" );
        manifest.hasValidators = true;
        manifest.validators = entries;
    }
    ASSETCONVERTER::CAssetPipelineOptions options;
    options.assetGlobs = call.request.assets;
    options.threadPoolName = threadPoolName;
    options.threads = threads;
    options.mode = ASSETCONVERTER::PIPELINEMODE_VALIDATE_ONLY;
    return RunManifest( call, pack.packId, manifest, options );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineRunOperations::ConvertFile( CPipelineOperationCall& call, const CORE::CString& threadPoolName, UInt32 threads )
{GUCEF_TRACE;

    CORE::CString sourceVfs;
    const ASSETSTORE::CAssetToolRoot* sourceRoot = GUCEF_NULL;
    CORE::CString sourceRooted;
    if ( !call.ResolveRootedPath( call.request.sourcePath, "source|staging", false, sourceVfs, sourceRoot, sourceRooted ) )
        return call.result.status;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.FileExists( sourceVfs ) )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is no file " + sourceRooted );

    // the licence record the request names, in a licences root
    const CORE::CString recordId = call.GetArgument( "licenceRecordId" );
    if ( recordId.IsNULLOrEmpty() )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "convert_file needs the licenceRecordId of an existing licence record" );
    CORE::CString problem;
    if ( !ASSETSTORE::CAssetRootedPath::IsSafeRelativePath( recordId, false, true, problem ) || recordId.HasChar( '/' ) >= 0 )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "the licenceRecordId is no plain id" );
    CORE::CString recordFolder;
    for ( size_t r=0; r<call.settings.roots.size() && recordFolder.IsNULLOrEmpty(); ++r )
    {
        if ( ASSETSTORE::CAssetToolRoot::KindLicences != call.settings.roots[ r ].kind )
            continue;
        if ( vfs.FileExists( ASSETCONVERTER::CVfsPaths::Join( call.settings.roots[ r ].vfsPath, recordId + ".licence.json" ) ) )
            recordFolder = call.settings.roots[ r ].vfsPath;
    }
    if ( recordFolder.IsNULLOrEmpty() )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "there is no licence record " + recordId + " in the licences roots" );
    CConverterLicenceDecision licence;
    licence.EvaluateRecord( recordId, ASSETCONVERTER::CVfsPaths::Join( recordFolder, recordId + ".licence.json" ) );

    // the scratch pack below the first work root that can be written
    const ASSETSTORE::CAssetToolRoot* work = GUCEF_NULL;
    for ( size_t r=0; r<call.settings.roots.size() && GUCEF_NULL == work; ++r )
        if ( ASSETSTORE::CAssetToolRoot::KindWork == call.settings.roots[ r ].kind && call.settings.roots[ r ].AllowsWrites() )
            work = &call.settings.roots[ r ];
    if ( GUCEF_NULL == work )
        return call.Fail( CPipelineOperationCall::ErrorBackendUnavailable, "there is no work root that can be written for the scratch pack" );
    CORE::CString jobKey = CPipelineLicenceOperations::MakeId( call.GetArgument( "jobKey" ) );
    if ( call.GetArgument( "jobKey" ).IsNULLOrEmpty() )
    {
        UInt8 bytes[ 8 ];
        CORE::CRandom::GetRandomBytes( bytes, 8 );
        jobKey.Clear();
        static const char* hex = "0123456789abcdef";
        for ( UInt32 b=0; b<8; ++b )
        {
            jobKey += hex[ bytes[ b ] >> 4 ];
            jobKey += hex[ bytes[ b ] & 15 ];
        }
    }
    const CORE::CString folder = ASSETCONVERTER::CVfsPaths::Join( ASSETCONVERTER::CVfsPaths::Join( work->vfsPath, "adhoc" ), jobKey );

    ASSETCONVERTER::CAssetPackManifest manifest;
    manifest.id = "adhoc-" + jobKey;
    manifest.title = ASSETCONVERTER::CVfsPaths::NameOf( sourceVfs );
    manifest.licenceRecord = recordId;
    manifest.licenceFolder = "/" + ASSETCONVERTER::CVfsPaths::Normalize( recordFolder );
    CORE::CString archiveType;
    if ( vfs.TryGetArchiveTypeForFile( sourceVfs, archiveType ) && !archiveType.IsNULLOrEmpty() )
    {
        manifest.source = "/" + ASSETCONVERTER::CVfsPaths::Normalize( sourceVfs );
    }
    else
    {
        manifest.source = "/" + ASSETCONVERTER::CVfsPaths::FolderOf( sourceVfs );
        manifest.assets.push_back( ASSETCONVERTER::CVfsPaths::NameOf( sourceVfs ) );
    }
    const CORE::CString importer = call.GetArgument( "importer" );
    manifest.importer = importer.IsNULLOrEmpty() ? ASSETCONVERTER::CAssetPackManifest::AutoImporter : importer;
    manifest.intermediate = "/" + ASSETCONVERTER::CVfsPaths::Join( folder, "intermediate" );
    manifest.output = "/" + ASSETCONVERTER::CVfsPaths::Join( folder, "output" );
    manifest.cookTargets = call.request.cookTargets;
    manifest.cookProfiles.push_back( ASSETCONVERTER::CCookProfile( "desktop" ) );
    manifest.manifestFolder = folder;
    if ( !CheckBeforeRun( call, manifest, licence, true ) )
        return call.result.status;

    // the manifest next to the scratch pack, so that the run can be repeated by hand
    CORE::CDataNode manifestNode( "assetpack", GUCEF_DATATYPE_OBJECT );
    manifest.Serialize( manifestNode );
    ASSETCONVERTER::CCanonicalJson::Store( ASSETCONVERTER::CVfsPaths::Join( folder, "pack.assetpack.json" ), manifestNode );

    ASSETCONVERTER::CAssetPipelineOptions options;
    options.cookTargets = call.request.cookTargets;
    options.force = call.request.force;
    options.threadPoolName = threadPoolName;
    options.threads = threads;
    options.mode = ASSETCONVERTER::PIPELINEMODE_FULL;
    const ASSETSTORE::TAssetStoreStatus status = RunManifest( call, CORE::CString::Empty, manifest, options );
    call.Document().SetAttribute( "packId", CORE::CVariant() );
    call.SetRooted( call.Document(), "scratchPack", folder );
    call.Document().SetAttribute( "source", sourceRooted );
    return status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
