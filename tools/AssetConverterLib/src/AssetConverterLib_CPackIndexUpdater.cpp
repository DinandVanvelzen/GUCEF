/*
 *  AssetConverterLib: reads, checks, builds, analyses and writes the pack index of a pack: the work done once
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXBUILDER_H
#include "AssetConverterLib_CPackIndexBuilder.h"
#define ASSETCONVERTERLIB_CPACKINDEXBUILDER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXBUILDER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXANALYSER_H
#include "AssetConverterLib_CPackIndexAnalyser.h"
#define ASSETCONVERTERLIB_CPACKINDEXANALYSER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXANALYSER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXREADER_H
#include "AssetConverterLib_CPackIndexReader.h"
#define ASSETCONVERTERLIB_CPACKINDEXREADER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXREADER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXWRITER_H
#include "AssetConverterLib_CPackIndexWriter.h"
#define ASSETCONVERTERLIB_CPACKINDEXWRITER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXWRITER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#include "AssetConverterLib_CPackIndexLocator.h"
#define ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H ? */

#ifndef ASSETCONVERTERLIB_CPACKWORKGATE_H
#include "AssetConverterLib_CPackWorkGate.h"
#define ASSETCONVERTERLIB_CPACKWORKGATE_H
#endif /* ASSETCONVERTERLIB_CPACKWORKGATE_H ? */

#ifndef ASSETCONVERTERLIB_CPACKGROUPRULES_H
#include "AssetConverterLib_CPackGroupRules.h"
#define ASSETCONVERTERLIB_CPACKGROUPRULES_H
#endif /* ASSETCONVERTERLIB_CPACKGROUPRULES_H ? */

#ifndef ASSETCONVERTERLIB_CINTERMEDIATELOCK_H
#include "AssetConverterLib_CIntermediateLock.h"
#define ASSETCONVERTERLIB_CINTERMEDIATELOCK_H
#endif /* ASSETCONVERTERLIB_CINTERMEDIATELOCK_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#include "AssetConverterLib_CAssetCacheEntry.h"
#define ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#endif /* ASSETCONVERTERLIB_CASSETCACHEENTRY_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#include "AssetConverterLib_CConversionSidecar.h"
#define ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CPackIndexUpdater.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackIndexUpdater::CPackIndexUpdater( const CAssetKindRegistry& kinds )
    : m_kinds( kinds )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackGateDecision
CPackIndexUpdater::PlanIndex( const CAssetPackListing& listing, const CAssetPackProfile& profile, const CPackIndexBuildOptions& buildOptions,
                              const CPackIndexUpdateOptions& updateOptions, const CORE::CString& givenIndexFolder )
{GUCEF_TRACE;

    CPackIndexBuilder builder( m_kinds );
    CPackIndexDocument current;
    if ( !builder.PrepareInputs( listing, profile, buildOptions, current ) )
        return CPackGateDecision( CPackGateDecision::OUTCOME_REFUSED, ACTIVITYREASON_CANCELLED );
    const CORE::CString indexFolder = givenIndexFolder.IsNULLOrEmpty() ? CPackIndexLocator::GetUserCacheFolder( updateOptions.userCacheRoot, current.identity.packKey ) : givenIndexFolder;
    CPackIndexReader reader;
    CPackIndexDocument stored;
    const CPackIndexReader::EResult read = reader.ReadManifest( indexFolder, stored );
    return CPackWorkGate::DecideIndex( read, stored, current, updateOptions.redo );
}

/*-------------------------------------------------------------------------*/

namespace {

UInt32
ElapsedMs( UInt64 startTicks )
{
    return (UInt32) MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );
}

}

bool
CPackIndexUpdater::EnsureIndex( const CAssetPackListing& listing, const CAssetPackProfile& profile, const CPackIndexBuildOptions& buildOptions,
                                const CPackIndexUpdateOptions& updateOptions, const CORE::CString& givenIndexFolder, CPackIndexDocument& document, CPackIndexRunResult& result )
{GUCEF_TRACE;

    result.Clear();
    document.Clear();
    const UInt64 startTicks = MT::PrecisionTickCount();
    const CPackRedoRequest& redo = updateOptions.redo;
    const CORE::CString packRoot = updateOptions.packVfsRoot.IsNULLOrEmpty() ? listing.GetVfsRoot() : updateOptions.packVfsRoot;

    CPackIndexBuilder builder( m_kinds );
    CPackIndexDocument current;
    if ( !builder.PrepareInputs( listing, profile, buildOptions, current ) )
    {
        result.cancelled = true;
        result.message = builder.GetError();
        return false;
    }

    CPackIndexReader reader;
    CPackIndexDocument stored;
    const CORE::CString indexFolder = givenIndexFolder.IsNULLOrEmpty() ? CPackIndexLocator::GetUserCacheFolder( updateOptions.userCacheRoot, current.identity.packKey ) : givenIndexFolder;
    result.indexFolder = indexFolder;
    CPackIndexReader::EResult read = reader.Read( indexFolder, stored );
    CPackGateDecision decision = CPackWorkGate::DecideIndex( read, stored, current, redo );
    result.indexDecision = decision;
    if ( CPackGateDecision::OUTCOME_REFUSED == decision.outcome )
    {
        result.refused = true;
        result.message = reader.GetMessage();
        return false;
    }
    if ( CPackIndexReader::RESULT_CORRUPT == read )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PackIndexUpdater: " << reader.GetMessage() << ": it is renamed and built again (" << ASSETCONVERTER::ToString( ACTIVITYREASON_DOCUMENT_CORRUPT ) << ")" );
        reader.MarkBad( indexFolder );
    }

    const bool wantAnalysis = CPackIndexUpdateOptions::LEVEL_ANALYSIS == updateOptions.level;
    bool structureNeeded = decision.NeedsWork();
    bool analysisNeeded = wantAnalysis && ( structureNeeded || !stored.HasLevel( CPackIndexDocument::LevelAnalysis ) || redo.reanalyse || redo.retryFailed );
    const bool outputsWanted = !updateOptions.intermediateDir.IsNULLOrEmpty();
    if ( !structureNeeded && !analysisNeeded && !outputsWanted )
    {
        document = stored;
        result.reused = true;
        result.totalMs = ElapsedMs( startTicks );
        return true;
    }

    // the writers of an index exclude each other
    CIntermediateLock lock( CPackIndexLocator::GetShardFolder( indexFolder ) );
    if ( !lock.IsHeld() )
    {
        result.locked = true;
        result.holder = lock.GetHolder();
        result.message = "the index is being written by " + lock.GetHolder();
        if ( CPackIndexReader::RESULT_OK == read )
        {
            document = stored;
            return true;
        }
        return false;
    }

    // another writer may have finished while this one waited for the lock
    if ( structureNeeded && !redo.reindex )
    {
        stored.Clear();
        read = reader.Read( indexFolder, stored );
        const CPackGateDecision redecided = CPackWorkGate::DecideIndex( read, stored, current, redo );
        structureNeeded = redecided.NeedsWork();
        if ( !structureNeeded )
        {
            decision = redecided;
            result.indexDecision = decision;
        }
        analysisNeeded = wantAnalysis && ( structureNeeded || !stored.HasLevel( CPackIndexDocument::LevelAnalysis ) || redo.reanalyse || redo.retryFailed );
    }
    const bool haveStored = CPackIndexReader::RESULT_OK == read;

    if ( structureNeeded )
    {
        const UInt64 structureTicks = MT::PrecisionTickCount();
        if ( !builder.BuildStructure( listing, profile, buildOptions, document, haveStored ? &stored : GUCEF_NULL ) )
        {
            result.cancelled = true;
            result.message = builder.GetError();
            document.Clear();
            return false;
        }
        result.structureBuilt = true;
        result.resourcesGrouped = builder.GetGroupedCount();
        result.recordsKept = builder.GetKeptCount();
        result.structureMs = ElapsedMs( structureTicks );
    }
    else
    {
        document = stored;
    }

    bool changed = structureNeeded;
    if ( analysisNeeded || document.HasLevel( CPackIndexDocument::LevelAnalysis ) )
    {
        if ( analysisNeeded )
        {
            CPackIndexAnalyser analyser;
            bool analysisChanged = false;
            if ( !analyser.Analyse( document, packRoot, redo, buildOptions.observer, analysisChanged ) )
            {
                result.cancelled = true;
                result.message = "cancelled";
                // what was done so far is kept in memory only: the stored document stays as it was
                document = haveStored ? stored : CPackIndexDocument();
                return haveStored;
            }
            result.resourcesAnalysed = analyser.GetAnalysedCount();
            result.analysisReused = analyser.GetReusedCount();
            result.analysisFailed = analyser.GetFailedCount();
            changed = changed || analysisChanged;
        }
    }
    if ( outputsWanted )
    {
        result.outputsRefreshed = RefreshOutputs( document, updateOptions );
        if ( result.outputsRefreshed > 0 && !document.HasLevel( CPackIndexDocument::LevelOutputs ) )
            document.AddLevel( CPackIndexDocument::LevelOutputs );
        changed = true;
    }

    RegroupRefined( document );
    document.UpdateSummaries();
    document.Normalise();
    if ( haveStored && !structureNeeded && document.ComputeDocumentHash() == stored.documentHash )
        changed = false;
    if ( !haveStored )
        changed = true;

    if ( !changed )
    {
        document = stored;
        result.reused = true;
        result.totalMs = ElapsedMs( startTicks );
        return true;
    }

    document.producedUtc = CORE::CDateTime::NowUTCDateTime().ToIso8601DateTimeString( true, true );
    document.producedHost = CORE::GetHostname();
    document.producedDurationMs = ElapsedMs( startTicks );
    if ( GUCEF_NULL != buildOptions.observer )
        buildOptions.observer->OnProgress( "write", 0, 1 );
    CPackIndexWriter writer;
    if ( !writer.Write( document, indexFolder ) )
    {
        result.message = writer.GetError();
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PackIndexUpdater: " << writer.GetError() );
        // the previous complete index stays in place
        document = haveStored ? stored : CPackIndexDocument();
        return haveStored;
    }
    result.written = true;
    result.shardsWritten = writer.GetShardsWritten();
    result.shardsReused = writer.GetShardsReused();
    result.totalMs = ElapsedMs( startTicks );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexUpdater::RegroupRefined( CPackIndexDocument& document )
{GUCEF_TRACE;

    CPackGroupRules rules( m_kinds );
    rules.SetProfile( document.profile );
    for ( size_t i=0; i<document.resources.size(); ++i )
    {
        CPackIndexResource& resource = document.resources[ i ];
        if ( "model" != resource.kind )
            continue;
        // a record that was refined before is placed again from its kind: the statistic may have changed
        if ( resource.groupRule.WildcardEquals( "refine:*", '*', true ) )
        {
            const CORE::CString subGroup = resource.subGroup;
            const CORE::CString format = resource.format;
            CPackIndexResource placed = resource;
            rules.Assign( placed );
            resource.group = placed.group;
            resource.groupRule = placed.groupRule;
            resource.subGroup = subGroup.IsNULLOrEmpty() ? placed.subGroup : subGroup;
        }
        rules.Refine( resource );
    }
}

/*-------------------------------------------------------------------------*/

namespace {

CORE::CString
RelativeTo( const CORE::CString& folder, const CORE::CString& path )
{
    CORE::CString relative;
    if ( CVfsPaths::MakeRelative( folder, CVfsPaths::Normalize( path ), relative ) )
        return relative;
    return path;
}

void
ReadSummary( const CORE::CDataNode& sidecar, CPackIndexResource& resource )
{
    const CORE::CDataNode* summary = sidecar.FindChild( "summary" );
    if ( GUCEF_NULL == summary )
        return;
    static const char* const names[][ 2 ] = { { "triangles", "triangles" }, { "vertices", "vertices" }, { "subMeshes", "subMeshes" }, { "bones", "bones" },
                                              { "materials", "materials" }, { "lodLevels", "lods" }, { "particleSystems", "systems" } };
    for ( size_t n=0; n<sizeof( names ) / sizeof( names[ 0 ] ); ++n )
        if ( CPackIndexSerial::HasMember( *summary, names[ n ][ 0 ] ) )
            resource.SetStat( names[ n ][ 1 ], CPackIndexSerial::GetCount( *summary, names[ n ][ 0 ] ) );
    const CORE::CDataNode* clips = summary->FindChild( "clips" );
    if ( GUCEF_NULL != clips && clips->HasChildren() )
        resource.SetStat( "animations", clips->GetNrOfDirectChildNodes() );
    const CORE::CDataNode* bounds = summary->FindChild( "bounds" );
    if ( GUCEF_NULL != bounds )
    {
        CORE::CDataNode::TVariantVector minValues;
        CORE::CDataNode::TVariantVector maxValues;
        bounds->GetValuesOfChildByName( "min", minValues );
        bounds->GetValuesOfChildByName( "max", maxValues );
        if ( 3 == minValues.size() && 3 == maxValues.size() )
        {
            resource.hasBounds = true;
            for ( UInt32 i=0; i<3; ++i )
            {
                resource.boundsMin[ i ] = minValues[ i ].AsFloat32( 0.0f, true );
                resource.boundsMax[ i ] = maxValues[ i ].AsFloat32( 0.0f, true );
            }
        }
    }
}

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexUpdater::ReadRelations( const CORE::CDataNode& sidecar, const CORE::CString& idBase, CPackIndexResource& resource )
{GUCEF_TRACE;

    if ( GUCEF_NULL == sidecar.FindChild( "relations" ) )
        return false;
    CAssetReference::TReferenceVector relations;
    CConversionSidecar::ReadRelations( sidecar, relations );
    resource.refs.clear();
    resource.unresolvedRefs.clear();
    CORE::CString::StringSet unresolved;
    for ( size_t r=0; r<relations.size(); ++r )
    {
        const CAssetReference& relation = relations[ r ];
        if ( !relation.IsResolved() )
        {
            if ( !relation.via.IsNULLOrEmpty() && unresolved.insert( relation.via ).second )
                resource.unresolvedRefs.push_back( relation.via );
            continue;
        }
        CPackIndexReference ref;
        ref.kind = relation.kind;
        ref.path = relation.target;
        ref.to = CPackIndexResource::MakeId( idBase, relation.target );
        ref.via = relation.via;
        ref.optional = relation.optional;
        resource.refs.push_back( ref );
    }
    std::sort( resource.refs.begin(), resource.refs.end() );
    std::sort( resource.unresolvedRefs.begin(), resource.unresolvedRefs.end() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexUpdater::ReadSidecarRelations( const CORE::CString& intermediateDir, const CORE::CString& idBase, CPackIndexResource& resource )
{GUCEF_TRACE;

    const CORE::CString sidecarPath = CConversionSidecar::GetPath( intermediateDir, resource.path );
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( sidecarPath ) )
        return false;
    CORE::CDataNode sidecar;
    if ( !CCanonicalJson::Load( sidecarPath, sidecar ) )
        return false;
    return ReadRelations( sidecar, idBase, resource );
}

/*-------------------------------------------------------------------------*/

UInt32
CPackIndexUpdater::RefreshOutputs( CPackIndexDocument& document, const CPackIndexUpdateOptions& updateOptions )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString& intermediate = updateOptions.intermediateDir;
    document.producer.outputRoot = "intermediate";
    UInt32 refreshed = 0;
    // the id base of the document, as the builder makes the ids of the resources
    const CORE::CString idBase = !document.identity.storePackId.IsNULLOrEmpty() ? document.identity.storePackId : document.identity.name;
    for ( size_t i=0; i<document.resources.size(); ++i )
    {
        CPackIndexResource& resource = document.resources[ i ];
        const CConversionReportEntry* entry = GUCEF_NULL != updateOptions.report ? updateOptions.report->FindEntry( resource.path ) : GUCEF_NULL;
        if ( updateOptions.licenceRefused )
        {
            CPackIndexStepStatus refused( CPackIndexStepStatus::Refused );
            refused.reasons.push_back( ASSETCONVERTER::ToString( ACTIVITYREASON_LICENCE ) );
            resource.status.SetStep( CPackIndexStatus::StepImport, refused );
            continue;
        }
        if ( GUCEF_NULL != entry && ASSETSTATUS_FAILED == entry->GetStatus() )
        {
            CPackIndexStepStatus failed( CPackIndexStepStatus::Failed );
            failed.reasons.push_back( "the conversion failed: see the report" );
            resource.status.SetStep( CPackIndexStatus::StepImport, failed );
            resource.status.state = CPackIndexStatus::Failed;
            continue;
        }
        const CORE::CString entryPath = CAssetCacheEntry::GetEntryPath( intermediate, resource.path );
        if ( !vfs.FileExists( entryPath ) )
            continue;
        CAssetCacheEntry cache;
        if ( !cache.Load( entryPath ) )
            continue;

        CPackIndexOutputs outputs;
        outputs.cacheKey = cache.key;
        bool cooked = false;
        for ( size_t f=0; f<cache.files.size(); ++f )
        {
            const CConversionReportFile& file = cache.files[ f ];
            CPackIndexOutputFile out;
            out.path = RelativeTo( intermediate, file.path );
            out.size = vfs.GetFileSize( file.path );
            if ( file.IsCooked() )
            {
                out.profile = file.profile;
                out.kind = CAssetKindRegistry::GetExtensionOfPath( file.path ).Lowercase();
                outputs.cooked.push_back( out );
                cooked = true;
            }
            else
            if ( outputs.intermediate.IsEmpty() )
                outputs.intermediate = out;
        }
        const CORE::CString sidecarPath = CConversionSidecar::GetPath( intermediate, resource.path );
        if ( vfs.FileExists( sidecarPath ) )
        {
            outputs.sidecar.path = RelativeTo( intermediate, sidecarPath );
            CORE::CDataNode sidecar;
            if ( CCanonicalJson::Load( sidecarPath, sidecar ) )
            {
                ReadSummary( sidecar, resource );
                ReadRelations( sidecar, idBase, resource );
            }
        }
        resource.outputs = outputs;

        CPackIndexStepStatus import( CPackIndexStepStatus::Fresh );
        import.key = cache.key;
        resource.status.SetStep( CPackIndexStatus::StepImport, import );
        CPackIndexStepStatus cook( cache.cookPending ? CPackIndexStepStatus::Stale : ( cooked ? CPackIndexStepStatus::Fresh : CPackIndexStepStatus::Missing ) );
        if ( cache.cookPending )
            cook.reasons.push_back( ASSETCONVERTER::ToString( ACTIVITYREASON_COOK_PENDING ) );
        resource.status.SetStep( CPackIndexStatus::StepCook, cook );
        resource.status.state = CPackIndexStatus::Converted;
        ++refreshed;
    }
    return refreshed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
