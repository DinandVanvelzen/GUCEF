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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

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

#ifndef ASSETCONVERTERLIB_CASSETPIPELINE_H
#include "AssetConverterLib_CAssetPipeline.h"
#define ASSETCONVERTERLIB_CASSETPIPELINE_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#include "AssetConverterLib_CAssetPipelineOptions.h"
#define ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPLAN_H
#include "AssetConverterLib_CAssetPackPlan.h"
#define ASSETCONVERTERLIB_CASSETPACKPLAN_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPLAN_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#include "AssetConverterLib_CAssetCacheEntry.h"
#define ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#endif /* ASSETCONVERTERLIB_CASSETCACHEENTRY_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#include "AssetConverterLib_CConversionSidecar.h"
#define ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H ? */

#ifndef ASSETCONVERTERLIB_CREFERENCEGRAPH_H
#include "AssetConverterLib_CReferenceGraph.h"
#define ASSETCONVERTERLIB_CREFERENCEGRAPH_H
#endif /* ASSETCONVERTERLIB_CREFERENCEGRAPH_H ? */

#ifndef ASSETCONVERTERLIB_CSIZEREPORT_H
#include "AssetConverterLib_CSizeReport.h"
#define ASSETCONVERTERLIB_CSIZEREPORT_H
#endif /* ASSETCONVERTERLIB_CSIZEREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetStoreConverterHook_CPipelineInspectionOperations.h"

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

/** A row of pack_assets */
struct SAssetRow
{
    CORE::CString assetPath;
    CORE::CString importer;
    CORE::CString state;
    UInt32 errors;
    UInt32 warnings;
    UInt32 approximations;
    UInt32 durationMs;
};

/** A row of pack_cook_outputs */
struct SCookRow
{
    CORE::CString file;
    CORE::CString asset;
    UInt64 own;
    UInt64 inclusive;
    CORE::CString::StringVector references;
};

ASSETCONVERTER::CAssetPluginRegistry&
Plugins( void )
{GUCEF_TRACE;

    return ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry();
}

bool
ContainsNoCase( const CORE::CString& text, const CORE::CString& part )
{GUCEF_TRACE;

    return part.IsNULLOrEmpty() || text.Lowercase().HasSubstr( part.Lowercase(), true ) >= 0;
}

/** Whether the asset path matches the glob (or is it), as the asset filter of the pipeline matches */
bool
MatchesGlob( const CORE::CString& glob, const CORE::CString& assetPath )
{GUCEF_TRACE;

    if ( glob.IsNULLOrEmpty() )
        return true;
    ASSETCONVERTER::CAssetPipelineOptions options;
    options.assetGlobs.push_back( glob );
    return options.IsInAssetFilter( assetPath );
}

/** The sidecar of an asset; false when there is none */
bool
LoadSidecar( const CConverterPack& pack, const CORE::CString& assetPath, CORE::CDataNode& sidecar )
{GUCEF_TRACE;

    const CORE::CString path = ASSETCONVERTER::CConversionSidecar::GetPath( pack.manifest.GetIntermediatePath(), assetPath );
    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( path ) && ASSETCONVERTER::CCanonicalJson::Load( path, sidecar );
}

UInt32
SummaryValue( const CORE::CDataNode& sidecar, const char* name )
{GUCEF_TRACE;

    const CORE::CDataNode* summary = sidecar.FindChild( "summary" );
    if ( GUCEF_NULL == summary )
        return 0;
    return summary->GetAttributeValueOrChildValueByName( name ).AsUInt32( 0, true );
}

/** The last activity entry of the asset, searching the runs from the last one back */
bool
FindLastActivity( const ASSETCONVERTER::CImportActivityLog& log, const CORE::CString& assetPath, ASSETCONVERTER::CImportActivityEntry& entry, CORE::CString& runTime )
{GUCEF_TRACE;

    const ASSETCONVERTER::CImportActivityLog::TRunVector& runs = log.GetRuns();
    for ( size_t r=runs.size(); r>0; --r )
    {
        const ASSETCONVERTER::CImportActivityLog::SRun& run = runs[ r-1 ];
        for ( size_t e=0; e<run.entries.size(); ++e )
        {
            if ( run.entries[ e ].assetPath == assetPath )
            {
                entry = run.entries[ e ];
                runTime = run.time;
                return true;
            }
        }
    }
    return false;
}

/** The plan of one asset of the pack, as a run would treat it now */
bool
PlanAsset( const CConverterPack& pack, const CORE::CString& assetPath, ASSETCONVERTER::CAssetPlanEntry& entry, ASSETCONVERTER::CAssetPackPlan& plan )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetPipelineOptions options;
    options.assetGlobs.push_back( assetPath );
    ASSETCONVERTER::CAssetPipeline pipeline;
    if ( !pipeline.PlanPack( pack.manifest, options, plan ) )
        return false;
    const ASSETCONVERTER::CAssetPlanEntry* found = plan.FindEntry( assetPath );
    if ( GUCEF_NULL == found )
        return false;
    entry = *found;
    return true;
}

/** "part" of an explanation for the reason of the cache comparison */
CORE::CString
PartOfReason( ASSETCONVERTER::TActivityReason reason )
{GUCEF_TRACE;

    switch ( reason )
    {
        case ASSETCONVERTER::ACTIVITYREASON_NEW:                return "new";
        case ASSETCONVERTER::ACTIVITYREASON_SOURCE_CHANGED:     return "source";
        case ASSETCONVERTER::ACTIVITYREASON_SETTINGS_CHANGED:   return "settings";
        case ASSETCONVERTER::ACTIVITYREASON_VERSION_CHANGED:    return "plugin-version";
        case ASSETCONVERTER::ACTIVITYREASON_DEPENDENCY_CHANGED: return "dependency";
        case ASSETCONVERTER::ACTIVITYREASON_OUTPUT_MISSING:     return "output-missing";
        case ASSETCONVERTER::ACTIVITYREASON_FORCED:             return "forced";
        case ASSETCONVERTER::ACTIVITYREASON_LICENCE:            return "licence";
        case ASSETCONVERTER::ACTIVITYREASON_COOK_PENDING:       return "cook-pending";
        default:                                                return CORE::CString::Empty;
    }
}

CORE::CString
EscapePointerToken( const CORE::CString& token )
{GUCEF_TRACE;

    return token.ReplaceSubstr( "~", "~0" ).ReplaceSubstr( "/", "~1" );
}

void
Difference( const CORE::CString& pointer, const CORE::CString& before, const CORE::CString& after, CORE::CDataNode& list )
{GUCEF_TRACE;

    CORE::CDataNode* item = list.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    item->SetAttribute( "pointer", pointer.IsNULLOrEmpty() ? CORE::CString( "/" ) : pointer );
    CPipelineOperationCall::SetTextOrNull( *item, "before", before );
    CPipelineOperationCall::SetTextOrNull( *item, "after", after );
}

void
CompareTrees( const CORE::CString& pointer, const CORE::CDataNode& before, const CORE::CDataNode& after, CORE::CDataNode& list, UInt32& budget )
{GUCEF_TRACE;

    if ( 0 == budget )
        return;
    if ( GUCEF_DATATYPE_ARRAY == before.GetNodeType() || GUCEF_DATATYPE_ARRAY == after.GetNodeType() )
    {
        // a list is replaced as a whole by a later layer: it differs as a whole
        const CORE::CString a = ASSETCONVERTER::CCanonicalJson::ToJson( before );
        const CORE::CString b = ASSETCONVERTER::CCanonicalJson::ToJson( after );
        if ( a != b )
        {
            Difference( pointer, a, b, list );
            --budget;
        }
        return;
    }
    // the values of both, by name
    CORE::CString::StringSet names;
    for ( CORE::CDataNode::TAttributeMap::const_iterator a=before.AttributeBegin(); a!=before.AttributeEnd(); ++a )
        names.insert( (*a).first );
    for ( CORE::CDataNode::TAttributeMap::const_iterator a=after.AttributeBegin(); a!=after.AttributeEnd(); ++a )
        names.insert( (*a).first );
    CORE::CDataNode::const_iterator c = before.ConstBegin();
    for ( ; c != before.ConstEnd(); ++c )
        if ( !(*c)->GetName().IsNULLOrEmpty() )
            names.insert( (*c)->GetName() );
    for ( c = after.ConstBegin(); c != after.ConstEnd(); ++c )
        if ( !(*c)->GetName().IsNULLOrEmpty() )
            names.insert( (*c)->GetName() );
    for ( CORE::CString::StringSet::const_iterator n=names.begin(); n!=names.end() && budget>0; ++n )
    {
        const CORE::CString childPointer = pointer + "/" + EscapePointerToken( (*n) );
        const CORE::CDataNode::TKeyValuePair* beforeValue = before.GetAttribute( (*n) );
        const CORE::CDataNode::TKeyValuePair* afterValue = after.GetAttribute( (*n) );
        const CORE::CDataNode* beforeChild = GUCEF_NULL == beforeValue ? before.FindChild( (*n) ) : GUCEF_NULL;
        const CORE::CDataNode* afterChild = GUCEF_NULL == afterValue ? after.FindChild( (*n) ) : GUCEF_NULL;
        if ( GUCEF_NULL != beforeChild && GUCEF_NULL != afterChild )
        {
            CompareTrees( childPointer, *beforeChild, *afterChild, list, budget );
            continue;
        }
        const CORE::CString a = CPipelineInspectionOperations::DescribeValue( beforeChild, GUCEF_NULL != beforeValue ? &(*beforeValue).second : GUCEF_NULL );
        const CORE::CString b = CPipelineInspectionOperations::DescribeValue( afterChild, GUCEF_NULL != afterValue ? &(*afterValue).second : GUCEF_NULL );
        const bool hadBefore = GUCEF_NULL != beforeChild || GUCEF_NULL != beforeValue;
        const bool hasAfter = GUCEF_NULL != afterChild || GUCEF_NULL != afterValue;
        if ( a != b || hadBefore != hasAfter )
        {
            Difference( childPointer, hadBefore ? a : CORE::CString::Empty, hasAfter ? b : CORE::CString::Empty, list );
            --budget;
        }
    }
}

/** The state of an asset of the report as the tools name it */
CORE::CString
StateOf( const ASSETCONVERTER::CConversionReportEntry& entry )
{GUCEF_TRACE;

    return ASSETCONVERTER::ToString( entry.GetStatus() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPipelineInspectionOperations::AddLink( CORE::CDataNode& node, const CORE::CString& uri, const CORE::CString& name, const CORE::CString& mimeType )
{GUCEF_TRACE;

    CORE::CDataNode* links = node.FindChild( "links" );
    if ( GUCEF_NULL == links )
        links = node.AddChild( "links", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* link = links->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    link->SetAttribute( "uri", uri );
    link->SetAttribute( "name", name );
    link->SetAttribute( "mimeType", mimeType );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPipelineInspectionOperations::DescribeValue( const CORE::CDataNode* node, const CORE::CVariant* value )
{GUCEF_TRACE;

    if ( GUCEF_NULL != value )
        return value->AsString( CORE::CString::Empty, true );
    if ( GUCEF_NULL != node )
    {
        if ( node->HasValue() && !node->HasChildren() && 0 == node->GetAttCount() )
            return node->GetValue().AsString( CORE::CString::Empty, true );
        return ASSETCONVERTER::CCanonicalJson::ToJson( *node );
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CPipelineInspectionOperations::AddSettingsDifferences( const CORE::CDataNode& before, const CORE::CDataNode& after, CORE::CDataNode& list )
{GUCEF_TRACE;

    UInt32 budget = 200;
    CompareTrees( CORE::CString::Empty, before, after, list, budget );
}

/*-------------------------------------------------------------------------*/

void
CPipelineInspectionOperations::AddCounts( const ASSETCONVERTER::CConversionReport& report, CORE::CDataNode& counts )
{GUCEF_TRACE;

    counts.SetNodeType( GUCEF_DATATYPE_OBJECT );
    counts.SetAttribute( "converted", report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_CONVERTED ) );
    counts.SetAttribute( "cached", report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_CACHED ) );
    counts.SetAttribute( "approximated", report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_APPROXIMATED ) );
    counts.SetAttribute( "failed", report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_FAILED ) );
    counts.SetAttribute( "refused", report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_REFUSED ) );
    counts.SetAttribute( "skipped", report.GetAssetCount( ASSETCONVERTER::ASSETSTATUS_SKIPPED ) );
}

/*-------------------------------------------------------------------------*/

void
CPipelineInspectionOperations::AddLastRun( const CConverterPack& pack, CORE::CDataNode& node )
{GUCEF_TRACE;

    ASSETCONVERTER::CConversionReport report;
    const bool hasReport = pack.LoadReport( report );
    ASSETCONVERTER::CImportActivityLog log;
    const bool hasLog = pack.LoadActivity( log ) && !log.GetRuns().empty();
    if ( !hasReport && !hasLog )
    {
        node.SetAttribute( "lastRun", CORE::CVariant() );
        return;
    }
    CORE::CDataNode* lastRun = node.AddChild( "lastRun", GUCEF_DATATYPE_OBJECT );
    CPipelineOperationCall::SetTextOrNull( *lastRun, "endedUtc", hasLog ? log.GetRuns().back().time : CORE::CString::Empty );
    AddCounts( report, *lastRun->AddChild( "counts", GUCEF_DATATYPE_OBJECT ) );
}

/*-------------------------------------------------------------------------*/

void
CPipelineInspectionOperations::AddLicence( const CPipelineOperationCall& call, const CConverterPack& pack, bool details, CORE::CDataNode& node )
{GUCEF_TRACE;

    pack.licence.ToDataNode( *node.AddChild( "licence", GUCEF_DATATYPE_OBJECT ), details, call.exposePurchaseDetails, call.maxUntrustedTextChars );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineInspectionOperations::PackList( CPipelineOperationCall& call )
{GUCEF_TRACE;

    const CORE::CString text = call.GetArgument( "text" );
    const CORE::CString rootId = call.GetArgument( "root" );
    UInt32 first = 0;
    UInt32 count = 0;
    if ( !call.GetPage( 0, first, count ) )
        return call.result.status;

    TConverterPackVector packs;
    call.GetPacks( packs );
    GUCEF::vector< const CConverterPack* > selected;
    for ( size_t i=0; i<packs.size(); ++i )
    {
        if ( !rootId.IsNULLOrEmpty() && 0 != packs[ i ].manifestRooted.HasSubstr( rootId + ":", true ) )
            continue;
        if ( !ContainsNoCase( packs[ i ].packId, text ) && !ContainsNoCase( packs[ i ].manifest.title, text ) && !ContainsNoCase( packs[ i ].manifest.store, text ) )
            continue;
        selected.push_back( &packs[ i ] );
    }

    CORE::CDataNode& document = call.Document();
    CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    UInt32 shown = 0;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( UInt32 i=first; i<(UInt32) selected.size() && shown<count; ++i, ++shown )
    {
        const CConverterPack& pack = *selected[ i ];
        CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "packId", pack.packId );
        item->SetAttribute( "title", pack.manifest.title );
        item->SetAttribute( "store", pack.manifest.store );
        item->SetAttribute( "manifest", pack.manifestRooted );
        item->SetAttribute( "ambiguous", pack.ambiguous );
        item->SetAttribute( "runnable", pack.runnable );
        CPipelineOperationCall::SetTextOrNull( *item, "runnableReason", pack.runnableReason );
        AddLicence( call, pack, false, *item );
        AddLastRun( pack, *item );
        item->SetAttribute( "intermediateExists", pack.HasIntermediate() );
        CORE::CDataNode* profiles = item->AddChild( "outputProfiles", GUCEF_DATATYPE_ARRAY );
        for ( size_t p=0; p<pack.manifest.cookProfiles.size(); ++p )
        {
            const CORE::CString folder = pack.FindProfileFolder( pack.manifest.cookProfiles[ p ].name );
            if ( !folder.IsNULLOrEmpty() && vfs.DirExists( folder ) )
                profiles->AddValueAsChild( pack.manifest.cookProfiles[ p ].name );
        }
    }
    call.FinishPage( first, shown, (UInt32) selected.size() );
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineInspectionOperations::PackGet( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, false ) )
        return call.result.status;

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "packId", pack.packId );
    document.SetAttribute( "manifestPath", pack.manifestRooted );
    document.SetAttribute( "runnable", pack.runnable );
    CPipelineOperationCall::SetTextOrNull( document, "runnableReason", pack.runnableReason );

    // the effective manifest, its folders as rooted paths
    CORE::CDataNode* manifestNode = document.AddChild( "manifest", GUCEF_DATATYPE_OBJECT );
    pack.manifest.Serialize( *manifestNode );
    manifestNode->SetName( "manifest" );
    call.SetRooted( *manifestNode, "source", pack.manifest.GetSourcePath() );
    call.SetRooted( *manifestNode, "intermediate", pack.manifest.GetIntermediatePath() );
    if ( pack.manifest.output.IsNULLOrEmpty() )
        manifestNode->SetAttribute( "output", CORE::CVariant() );
    else
        call.SetRooted( *manifestNode, "output", pack.manifest.GetOutputPath() );
    if ( pack.manifest.overrides.IsNULLOrEmpty() )
        manifestNode->SetAttribute( "overrides", CORE::CVariant() );
    else
        call.SetRooted( *manifestNode, "overrides", pack.manifest.GetOverridesPath() );
    call.SetRooted( *manifestNode, "licenceFolder", ASSETCONVERTER::CVfsPaths::FolderOf( pack.manifest.GetLicenceRecordPath() ) );

    AddLicence( call, pack, true, document );
    CORE::CDataNode* profiles = document.AddChild( "cookProfiles", GUCEF_DATATYPE_ARRAY );
    for ( size_t p=0; p<pack.manifest.cookProfiles.size(); ++p )
        pack.manifest.cookProfiles[ p ].Serialize( *profiles->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    AddLastRun( pack, document );

    AddLink( document, ASSETSTORE::CAssetToolUris::ForPack( pack.packId, "manifest" ), "manifest", "application/json" );
    AddLink( document, ASSETSTORE::CAssetToolUris::ForPack( pack.packId, "licence" ), "licence record", "application/json" );
    AddLink( document, ASSETSTORE::CAssetToolUris::ForPack( pack.packId, "report" ), "report", "application/json" );
    AddLink( document, ASSETSTORE::CAssetToolUris::ForPack( pack.packId, "report-summary" ), "report summary", "text/plain" );
    AddLink( document, ASSETSTORE::CAssetToolUris::ForPack( pack.packId, "activity" ), "activity log", "application/json" );
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineInspectionOperations::PackAssets( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, false ) )
        return call.result.status;
    const CORE::CString state = call.GetArgument( "state" );
    const CORE::CString glob = call.GetArgument( "glob" );
    const CORE::CString importer = call.GetArgument( "importer" );
    const CORE::CString kind = call.GetArgument( "kind" ).Lowercase();
    if ( !state.IsNULLOrEmpty() && "not-run" != state && "converted" != state && "cached" != state && "approximated" != state &&
         "failed" != state && "refused" != state && "skipped" != state )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "the state " + state + " is not one of converted, cached, approximated, failed, refused, skipped, not-run" );
    UInt32 first = 0;
    UInt32 count = 0;
    if ( !call.GetPage( 0, first, count ) )
        return call.result.status;

    GUCEF::vector< SAssetRow > rows;
    ASSETCONVERTER::CConversionReport report;
    pack.LoadReport( report );
    const ASSETCONVERTER::CConversionReport::TEntryVector& entries = report.GetEntries();
    if ( "not-run" == state )
    {
        // the assets a run would take that the last report does not have
        ASSETCONVERTER::CAssetPipelineOptions options;
        ASSETCONVERTER::CAssetPipeline pipeline;
        ASSETCONVERTER::CAssetPackPlan plan;
        pipeline.PlanPack( pack.manifest, options, plan );
        for ( size_t i=0; i<plan.entries.size(); ++i )
        {
            if ( GUCEF_NULL != report.FindEntry( plan.entries[ i ].assetPath ) )
                continue;
            SAssetRow row;
            row.assetPath = plan.entries[ i ].assetPath;
            row.importer = plan.entries[ i ].importer;
            row.state = "not-run";
            row.errors = row.warnings = row.approximations = row.durationMs = 0;
            rows.push_back( row );
        }
    }
    else
    {
        for ( size_t i=0; i<entries.size(); ++i )
        {
            if ( entries[ i ].assetPath.IsNULLOrEmpty() )
                continue;
            SAssetRow row;
            row.assetPath = entries[ i ].assetPath;
            row.importer = entries[ i ].importer;
            row.state = StateOf( entries[ i ] );
            row.errors = entries[ i ].GetItemCount( ASSETCONVERTER::REPORTITEM_ERROR );
            row.warnings = entries[ i ].GetItemCount( ASSETCONVERTER::REPORTITEM_WARNING );
            row.approximations = entries[ i ].GetItemCount( ASSETCONVERTER::REPORTITEM_APPROXIMATION );
            row.durationMs = entries[ i ].durationInMs;
            rows.push_back( row );
        }
    }

    GUCEF::vector< const SAssetRow* > selected;
    for ( size_t i=0; i<rows.size(); ++i )
    {
        if ( !state.IsNULLOrEmpty() && rows[ i ].state != state )
            continue;
        if ( !MatchesGlob( glob, rows[ i ].assetPath ) )
            continue;
        if ( !importer.IsNULLOrEmpty() && rows[ i ].importer != importer )
            continue;
        if ( !kind.IsNULLOrEmpty() && ASSETCONVERTER::CVfsPaths::ExtensionOf( rows[ i ].assetPath ).Lowercase() != kind )
            continue;
        selected.push_back( &rows[ i ] );
    }

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "packId", pack.packId );
    CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    UInt32 shown = 0;
    for ( UInt32 i=first; i<(UInt32) selected.size() && shown<count; ++i, ++shown )
    {
        const SAssetRow& row = *selected[ i ];
        CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "assetPath", row.assetPath );
        item->SetAttribute( "importer", row.importer );
        item->SetAttribute( "state", row.state );
        item->SetAttribute( "errors", row.errors );
        item->SetAttribute( "warnings", row.warnings );
        item->SetAttribute( "approximations", row.approximations );
        CORE::CDataNode sidecar;
        if ( "not-run" != row.state && LoadSidecar( pack, row.assetPath, sidecar ) )
        {
            item->SetAttribute( "triangles", SummaryValue( sidecar, "triangles" ) );
            item->SetAttribute( "vertices", SummaryValue( sidecar, "vertices" ) );
            item->SetAttribute( "materials", SummaryValue( sidecar, "materials" ) );
        }
        else
        {
            item->SetAttribute( "triangles", CORE::CVariant() );
            item->SetAttribute( "vertices", CORE::CVariant() );
            item->SetAttribute( "materials", CORE::CVariant() );
        }
        item->SetAttribute( "durationMs", row.durationMs );
    }
    call.FinishPage( first, shown, (UInt32) selected.size() );
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineInspectionOperations::AssetReport( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, false ) || !call.CheckAssetPath( call.request.assetPath ) )
        return call.result.status;
    const CORE::CString& assetPath = call.request.assetPath;
    CORE::CString::StringVector sections;
    call.GetListArgument( "sections", sections );
    CORE::CString::StringSet wanted( sections.begin(), sections.end() );
    const bool all = wanted.empty();

    ASSETCONVERTER::CConversionReport report;
    pack.LoadReport( report );
    const ASSETCONVERTER::CConversionReportEntry* entry = report.FindEntry( assetPath );
    CORE::CDataNode sidecar;
    const bool hasSidecar = LoadSidecar( pack, assetPath, sidecar );
    ASSETCONVERTER::CImportActivityLog log;
    pack.LoadActivity( log );
    ASSETCONVERTER::CImportActivityEntry activity;
    CORE::CString runTime;
    const bool hasActivity = FindLastActivity( log, assetPath, activity, runTime );
    if ( GUCEF_NULL == entry && !hasSidecar && !hasActivity )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the pack " + pack.packId + " has no report, sidecar or activity of the asset " + assetPath + "; pack_assets lists its assets" );

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "packId", pack.packId );
    document.SetAttribute( "assetPath", assetPath );
    if ( all || wanted.end() != wanted.find( "report" ) )
    {
        if ( GUCEF_NULL == entry )
            document.SetAttribute( "report", CORE::CVariant() );
        else
        {
            CORE::CDataNode* node = document.AddChild( "report", GUCEF_DATATYPE_OBJECT );
            node->SetAttribute( "importer", entry->importer );
            node->SetAttribute( "state", StateOf( *entry ) );
            node->SetAttribute( "durationMs", entry->durationInMs );
            CORE::CDataNode* items = node->AddChild( "items", GUCEF_DATATYPE_ARRAY );
            for ( size_t i=0; i<entry->items.size(); ++i )
            {
                CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                item->SetAttribute( "kind", ASSETCONVERTER::ToString( entry->items[ i ].kind ) );
                item->SetAttribute( "source", entry->items[ i ].source );
                item->SetAttribute( "subject", entry->items[ i ].subject );
                item->AddChild( call.Untrusted( "message", "importer-message", entry->items[ i ].message ) );
            }
            CORE::CDataNode* files = node->AddChild( "files", GUCEF_DATATYPE_ARRAY );
            for ( size_t f=0; f<entry->files.size(); ++f )
            {
                CORE::CDataNode* file = files->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                call.SetRooted( *file, "path", entry->files[ f ].path );
                CPipelineOperationCall::SetTextOrNull( *file, "profile", entry->files[ f ].profile );
            }
        }
    }
    if ( all || wanted.end() != wanted.find( "sidecar" ) )
    {
        if ( !hasSidecar )
            document.SetAttribute( "sidecar", CORE::CVariant() );
        else
        {
            CORE::CDataNode* node = document.AddChild( "sidecar", GUCEF_DATATYPE_OBJECT );
            const char* members[] = { "summary", "settings", "settingsLayers", "lossySteps" };
            for ( UInt32 m=0; m<4; ++m )
            {
                const CORE::CDataNode* child = sidecar.FindChild( members[ m ] );
                if ( GUCEF_NULL == child )
                {
                    node->SetAttribute( members[ m ], CORE::CVariant() );
                    continue;
                }
                CORE::CDataNode* copy = node->AddChild( *child );
                copy->SetName( members[ m ] );
                if ( 3 == m )
                {
                    // the steps quote names from the pack: their messages are untrusted text
                    CORE::CDataNode::iterator s = copy->Begin();
                    for ( ; s != copy->End(); ++s )
                    {
                        const CORE::CString message = (*s)->GetAttributeValueOrChildValueByName( "message" ).AsString( CORE::CString::Empty, true );
                        (*s)->DelAttribute( "message" );
                        (*s)->DelChild( "message" );
                        (*s)->AddChild( call.Untrusted( "message", "importer-message", message ) );
                    }
                }
            }
        }
    }
    if ( all || wanted.end() != wanted.find( "activity" ) )
    {
        if ( !hasActivity )
            document.SetAttribute( "activity", CORE::CVariant() );
        else
        {
            CORE::CDataNode* node = document.AddChild( "activity", GUCEF_DATATYPE_OBJECT );
            node->SetAttribute( "action", ASSETCONVERTER::ToString( activity.action ) );
            node->SetAttribute( "reason", ASSETCONVERTER::ToString( activity.reason ) );
            node->SetAttribute( "detail", activity.detail );
            node->SetAttribute( "durationMs", activity.durationInMs );
            node->SetAttribute( "runTime", runTime );
        }
    }
    if ( all || wanted.end() != wanted.find( "cache" ) )
    {
        ASSETCONVERTER::CAssetCacheEntry cache;
        const CORE::CString cachePath = ASSETCONVERTER::CAssetCacheEntry::GetEntryPath( pack.manifest.GetIntermediatePath(), assetPath );
        if ( !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( cachePath ) || !cache.Load( cachePath ) )
            document.SetAttribute( "cache", CORE::CVariant() );
        else
        {
            CORE::CDataNode* node = document.AddChild( "cache", GUCEF_DATATYPE_OBJECT );
            node->SetAttribute( "key", cache.key );
            node->SetAttribute( "sourceHash", cache.sourceHash );
            node->SetAttribute( "settingsHash", cache.settingsHash );
            node->SetAttribute( "cookPending", cache.cookPending );
            CORE::CDataNode* plugins = node->AddChild( "plugins", GUCEF_DATATYPE_OBJECT );
            for ( ASSETCONVERTER::CAssetCacheEntry::TPluginVersionMap::const_iterator p=cache.plugins.begin(); p!=cache.plugins.end(); ++p )
                plugins->SetAttribute( (*p).first, (*p).second );
            CORE::CDataNode* dependencies = node->AddChild( "dependencies", GUCEF_DATATYPE_OBJECT );
            for ( ASSETCONVERTER::CAssetCacheEntry::THashMap::const_iterator d=cache.dependencies.begin(); d!=cache.dependencies.end(); ++d )
                dependencies->SetAttribute( (*d).first, (*d).second );
        }
    }
    AddLink( document, ASSETSTORE::CAssetToolUris::ForAsset( pack.packId, assetPath, "report" ), "asset report", "application/json" );
    AddLink( document, ASSETSTORE::CAssetToolUris::ForAsset( pack.packId, assetPath, "sidecar" ), "sidecar", "application/json" );
    AddLink( document, ASSETSTORE::CAssetToolUris::ForAsset( pack.packId, assetPath, "override" ), "override", "application/json" );
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineInspectionOperations::PackActivity( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, false ) )
        return call.result.status;
    const UInt32 runIndex = call.request.arguments.GetAttributeValueOrChildValueByName( "runIndex" ).AsUInt32( 0, true );
    const CORE::CString action = call.GetArgument( "action" );
    const CORE::CString reason = call.GetArgument( "reason" );
    const CORE::CString glob = call.GetArgument( "glob" );
    UInt32 first = 0;
    UInt32 count = 0;
    if ( !call.GetPage( 0, first, count ) )
        return call.result.status;

    ASSETCONVERTER::CImportActivityLog log;
    pack.LoadActivity( log );
    const ASSETCONVERTER::CImportActivityLog::TRunVector& runs = log.GetRuns();
    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "packId", pack.packId );
    document.SetAttribute( "runIndex", runIndex );
    document.SetAttribute( "runCount", (UInt32) runs.size() );
    CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    if ( runIndex >= (UInt32) runs.size() )
    {
        document.SetAttribute( "runTime", CORE::CVariant() );
        call.FinishPage( 0, 0, 0 );
        return call.Succeed();
    }
    const ASSETCONVERTER::CImportActivityLog::SRun& run = runs[ runs.size() - 1 - runIndex ];
    document.SetAttribute( "runTime", run.time );
    GUCEF::vector< const ASSETCONVERTER::CImportActivityEntry* > selected;
    for ( size_t e=0; e<run.entries.size(); ++e )
    {
        const ASSETCONVERTER::CImportActivityEntry& entry = run.entries[ e ];
        if ( !action.IsNULLOrEmpty() && ASSETCONVERTER::ToString( entry.action ) != action )
            continue;
        if ( !reason.IsNULLOrEmpty() && ASSETCONVERTER::ToString( entry.reason ) != reason )
            continue;
        if ( !MatchesGlob( glob, entry.assetPath ) )
            continue;
        selected.push_back( &entry );
    }
    UInt32 shown = 0;
    for ( UInt32 i=first; i<(UInt32) selected.size() && shown<count; ++i, ++shown )
    {
        CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "assetPath", selected[ i ]->assetPath );
        item->SetAttribute( "action", ASSETCONVERTER::ToString( selected[ i ]->action ) );
        item->SetAttribute( "reason", ASSETCONVERTER::ToString( selected[ i ]->reason ) );
        item->SetAttribute( "detail", selected[ i ]->detail );
        item->SetAttribute( "durationMs", selected[ i ]->durationInMs );
    }
    call.FinishPage( first, shown, (UInt32) selected.size() );
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineInspectionOperations::ExplainReimport( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, false ) || !call.CheckAssetPath( call.request.assetPath ) )
        return call.result.status;
    const CORE::CString& assetPath = call.request.assetPath;

    ASSETCONVERTER::CAssetPlanEntry entry;
    ASSETCONVERTER::CAssetPackPlan plan;
    if ( !PlanAsset( pack, assetPath, entry, plan ) )
    {
        if ( !plan.conversionAllowed && !plan.licenceReason.IsNULLOrEmpty() )
            return call.Fail( CPipelineOperationCall::ErrorLicenceRefused, "the pack is refused by its licence record: " + pack.licence.reason );
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the asset " + assetPath + " is not one the pack " + pack.packId + " converts (no importer takes it, or the manifest leaves it out)" );
    }

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "packId", pack.packId );
    document.SetAttribute( "assetPath", assetPath );
    ASSETCONVERTER::CImportActivityLog log;
    pack.LoadActivity( log );
    ASSETCONVERTER::CImportActivityEntry activity;
    CORE::CString runTime;
    if ( FindLastActivity( log, assetPath, activity, runTime ) )
    {
        CORE::CDataNode* lastRun = document.AddChild( "lastRun", GUCEF_DATATYPE_OBJECT );
        lastRun->SetAttribute( "action", ASSETCONVERTER::ToString( activity.action ) );
        lastRun->SetAttribute( "reason", ASSETCONVERTER::ToString( activity.reason ) );
        lastRun->SetAttribute( "detail", activity.detail );
        lastRun->SetAttribute( "runTime", runTime );
    }
    else
        document.SetAttribute( "lastRun", CORE::CVariant() );

    CORE::CDataNode* now = document.AddChild( "now", GUCEF_DATATYPE_OBJECT );
    now->SetAttribute( "wouldReimport", ASSETCONVERTER::PLANACTION_IMPORT == entry.action );
    now->SetAttribute( "action", ASSETCONVERTER::ToString( entry.action ) );
    CORE::CDataNode* reasons = now->AddChild( "reasons", GUCEF_DATATYPE_ARRAY );
    const CORE::CString part = PartOfReason( entry.reason );
    if ( !part.IsNULLOrEmpty() && ASSETCONVERTER::PLANACTION_CACHED != entry.action )
    {
        CORE::CDataNode* item = reasons->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "part", part );
        item->SetAttribute( "detail", entry.detail.IsNULLOrEmpty() ? ASSETCONVERTER::ToString( entry.reason ) : entry.detail );
    }

    // what the settings of the last conversion (the sidecar) and the settings of now differ in
    CORE::CDataNode* differences = document.AddChild( "settingsDifferences", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode sidecar;
    if ( LoadSidecar( pack, assetPath, sidecar ) )
    {
        const CORE::CDataNode* before = sidecar.FindChild( "settings" );
        if ( GUCEF_NULL != before )
            AddSettingsDifferences( *before, entry.settings, *differences );
    }
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineInspectionOperations::CookOutputs( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, false ) )
        return call.result.status;
    CORE::CString profile = call.GetArgument( "profile" );
    if ( profile.IsNULLOrEmpty() && !pack.manifest.cookProfiles.empty() )
        profile = pack.manifest.cookProfiles[ 0 ].name;
    const CORE::CString sort = call.GetArgument( "sort" );
    if ( !sort.IsNULLOrEmpty() && "inclusive-size" != sort && "name" != sort )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "sort is inclusive-size or name" );
    const CORE::CString& assetPath = call.request.assetPath;
    UInt32 first = 0;
    UInt32 count = 0;
    if ( !call.GetPage( 0, first, count ) )
        return call.result.status;

    const CORE::CString folder = pack.FindProfileFolder( profile );
    ASSETCONVERTER::CReferenceGraph graph;
    ASSETCONVERTER::CSizeReport sizes;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString referencesPath = ASSETCONVERTER::CVfsPaths::Join( folder, "references.json" );
    const CORE::CString sizesPath = ASSETCONVERTER::CVfsPaths::Join( folder, "sizes.json" );
    if ( folder.IsNULLOrEmpty() || !vfs.FileExists( referencesPath ) || !graph.Load( referencesPath ) )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the pack " + pack.packId + " has no cooked files of the profile " + profile );
    if ( vfs.FileExists( sizesPath ) )
        sizes.Load( sizesPath );

    GUCEF::vector< SCookRow > rows;
    const ASSETCONVERTER::CReferenceGraph::TFileMap& files = graph.GetFiles();
    const ASSETCONVERTER::CSizeReport::TFileSizeMap& fileSizes = sizes.GetFileSizes();
    for ( ASSETCONVERTER::CReferenceGraph::TFileMap::const_iterator f=files.begin(); f!=files.end(); ++f )
    {
        if ( !assetPath.IsNULLOrEmpty() && (*f).second.asset != assetPath )
            continue;
        SCookRow row;
        row.file = (*f).first;
        row.asset = (*f).second.asset;
        row.references = (*f).second.references;
        ASSETCONVERTER::CSizeReport::TFileSizeMap::const_iterator own = fileSizes.find( row.file );
        row.own = own == fileSizes.end() ? 0 : (*own).second;
        CORE::CString::StringSet closure;
        graph.GetClosure( row.file, closure );
        row.inclusive = 0;
        for ( CORE::CString::StringSet::const_iterator c=closure.begin(); c!=closure.end(); ++c )
        {
            ASSETCONVERTER::CSizeReport::TFileSizeMap::const_iterator s = fileSizes.find( (*c) );
            if ( s != fileSizes.end() )
                row.inclusive += (*s).second;
        }
        rows.push_back( row );
    }
    if ( "inclusive-size" == sort )
    {
        // largest first; a stable insertion order keeps equal sizes by name
        for ( size_t i=1; i<rows.size(); ++i )
            for ( size_t j=i; j>0 && rows[ j-1 ].inclusive < rows[ j ].inclusive; --j )
                std::swap( rows[ j-1 ], rows[ j ] );
    }

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "packId", pack.packId );
    document.SetAttribute( "profile", profile );
    CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    UInt32 shown = 0;
    for ( UInt32 i=first; i<(UInt32) rows.size() && shown<count; ++i, ++shown )
    {
        CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "file", rows[ i ].file );
        item->SetAttribute( "asset", rows[ i ].asset );
        item->SetAttribute( "ownBytes", rows[ i ].own );
        item->SetAttribute( "inclusiveBytes", rows[ i ].inclusive );
        item->SetValuesOfChildByName( "references", rows[ i ].references );
    }
    call.FinishPage( first, shown, (UInt32) rows.size() );
    AddLink( document, ASSETSTORE::CAssetToolUris::ForCookProfile( pack.packId, profile, "references" ), "references", "application/json" );
    AddLink( document, ASSETSTORE::CAssetToolUris::ForCookProfile( pack.packId, profile, "sizes" ), "sizes", "application/json" );
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineInspectionOperations::PipelinePlugins( CPipelineOperationCall& call )
{GUCEF_TRACE;

    const CORE::CString kind = call.GetArgument( "kind" );
    if ( !kind.IsNULLOrEmpty() && "importers" != kind && "exporters" != kind && "cookers" != kind && "postProcessors" != kind && "validators" != kind )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "kind is importers, exporters, cookers, postProcessors or validators" );
    ASSETCONVERTER::CAssetPluginRegistry& plugins = Plugins();
    CORE::CDataNode& document = call.Document();

    if ( kind.IsNULLOrEmpty() || "importers" == kind )
    {
        CORE::CDataNode* list = document.AddChild( "importers", GUCEF_DATATYPE_ARRAY );
        GUCEF::vector< ASSETCONVERTER::TAssetImporterPtr > importers;
        plugins.GetImporters( importers );
        for ( size_t i=0; i<importers.size(); ++i )
        {
            CORE::CDataNode* item = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "name", importers[ i ]->GetName() );
            item->SetAttribute( "version", importers[ i ]->GetVersion() );
            CORE::CDataNode defaults( "defaultSettings", GUCEF_DATATYPE_OBJECT );
            importers[ i ]->GetDefaultSettings( defaults );
            defaults.SetName( "defaultSettings" );
            defaults.SetNodeType( GUCEF_DATATYPE_OBJECT );
            item->AddChild( defaults );
            CORE::CString::StringVector processors;
            importers[ i ]->GetDefaultPostProcessors( processors );
            item->SetValuesOfChildByName( "defaultPostProcessors", processors );
        }
    }
    if ( kind.IsNULLOrEmpty() || "exporters" == kind )
    {
        CORE::CDataNode* list = document.AddChild( "exporters", GUCEF_DATATYPE_ARRAY );
        CORE::CString::StringVector names;
        plugins.GetExporters().GetRegisteredObjNames( names );
        for ( size_t i=0; i<names.size(); ++i )
        {
            ASSETCONVERTER::TAssetExporterPtr exporter;
            if ( !plugins.TryGetExporter( names[ i ], exporter ) )
                continue;
            CORE::CDataNode* item = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "name", exporter->GetName() );
            item->SetAttribute( "version", exporter->GetVersion() );
        }
    }
    if ( kind.IsNULLOrEmpty() || "cookers" == kind )
    {
        CORE::CDataNode* list = document.AddChild( "cookers", GUCEF_DATATYPE_ARRAY );
        CORE::CString::StringVector names;
        plugins.GetCookers().GetRegisteredObjNames( names );
        for ( size_t i=0; i<names.size(); ++i )
        {
            ASSETCONVERTER::TAssetCookerPtr cooker;
            if ( !plugins.TryGetCooker( names[ i ], cooker ) )
                continue;
            CORE::CDataNode* item = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "name", cooker->GetTargetName() );
            item->SetAttribute( "version", cooker->GetVersion() );
            item->SetAttribute( "maxBonesPerSubMesh", cooker->GetMaxBonesPerSubMesh() );
        }
    }
    if ( kind.IsNULLOrEmpty() || "postProcessors" == kind )
    {
        CORE::CDataNode* list = document.AddChild( "postProcessors", GUCEF_DATATYPE_ARRAY );
        CORE::CString::StringVector names;
        plugins.GetPostProcessors().GetRegisteredObjNames( names );
        for ( size_t i=0; i<names.size(); ++i )
        {
            ASSETCONVERTER::TAssetPostProcessorPtr processor;
            if ( !plugins.TryGetPostProcessor( names[ i ], processor ) )
                continue;
            CORE::CDataNode* item = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "name", processor->GetName() );
            item->SetAttribute( "version", processor->GetVersion() );
        }
    }
    if ( kind.IsNULLOrEmpty() || "validators" == kind )
    {
        CORE::CDataNode* list = document.AddChild( "validators", GUCEF_DATATYPE_ARRAY );
        CORE::CString::StringVector names;
        plugins.GetValidators().GetRegisteredObjNames( names );
        for ( size_t i=0; i<names.size(); ++i )
        {
            ASSETCONVERTER::TAssetValidatorPtr validator;
            if ( !plugins.TryGetValidator( names[ i ], validator ) )
                continue;
            CORE::CDataNode* item = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "name", validator->GetName() );
            item->SetAttribute( "version", validator->GetVersion() );
        }
    }
    return call.Succeed();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
