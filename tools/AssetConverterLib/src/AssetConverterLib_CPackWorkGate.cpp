/*
 *  AssetConverterLib: the single place that decides whether a stored pack index result may be used
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#include "AssetConverterLib_CPackWorkGate.h"

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

CORE::CString
CPackWorkGate::MakeAnalyseKey( const CORE::CString& analyserId, UInt32 analyserVersion, const CORE::CString& settingsHash, const CORE::CString& digest, UInt64 size )
{GUCEF_TRACE;

    CORE::CString::StringVector parts;
    parts.push_back( analyserId );
    parts.push_back( CORE::UInt32ToString( analyserVersion ) );
    parts.push_back( settingsHash );
    parts.push_back( digest.IsNULLOrEmpty() ? "size:" + CORE::UInt64ToString( size ) : "sha256:" + digest );
    return CPackIndexSerial::HashParts( parts );
}

/*-------------------------------------------------------------------------*/

CPackGateDecision
CPackWorkGate::DecideRecognise( const CPackIndexDocument* stored, const CPackIndexDocument& current, const CPackRedoRequest& request )
{GUCEF_TRACE;

    if ( GUCEF_NULL == stored )
        return CPackGateDecision( CPackGateDecision::OUTCOME_MISSING, ACTIVITYREASON_NEW );
    if ( request.reindex )
        return CPackGateDecision( CPackGateDecision::OUTCOME_FORCED, ACTIVITYREASON_FORCED, "re-index" );
    if ( stored->profile.recognitionKey != current.profile.recognitionKey )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_SOURCE_CHANGED, "the files or the recognisers changed" );
    return CPackGateDecision( CPackGateDecision::OUTCOME_FRESH, ACTIVITYREASON_UNCHANGED );
}

/*-------------------------------------------------------------------------*/

CPackGateDecision
CPackWorkGate::DecideIndex( CPackIndexReader::EResult storedResult, const CPackIndexDocument& stored, const CPackIndexDocument& current, const CPackRedoRequest& request )
{GUCEF_TRACE;

    switch ( storedResult )
    {
        case CPackIndexReader::RESULT_OK:
            break;
        case CPackIndexReader::RESULT_MISSING:
            return CPackGateDecision( request.reindex ? CPackGateDecision::OUTCOME_FORCED : CPackGateDecision::OUTCOME_MISSING, request.reindex ? ACTIVITYREASON_FORCED : ACTIVITYREASON_NEW );
        case CPackIndexReader::RESULT_CORRUPT:
            return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_DOCUMENT_CORRUPT, "the stored index failed its parse or hash check" );
        case CPackIndexReader::RESULT_NOT_A_PACK_INDEX:
            return CPackGateDecision( CPackGateDecision::OUTCOME_REFUSED, ACTIVITYREASON_SCHEMA_CHANGED, "the stored file is not a pack index" );
        case CPackIndexReader::RESULT_VERSION_TOO_NEW:
            return CPackGateDecision( CPackGateDecision::OUTCOME_REFUSED, ACTIVITYREASON_SCHEMA_CHANGED, "the stored index is of a newer version than this build reads" );
    }
    if ( request.reindex )
        return CPackGateDecision( CPackGateDecision::OUTCOME_FORCED, ACTIVITYREASON_FORCED, "re-index" );
    if ( !stored.IsComplete() )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_NEW, "the stored index is " + stored.state );
    if ( stored.version != CPackIndexDocument::SchemaVersion )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_SCHEMA_CHANGED, "version " + CORE::UInt32ToString( stored.version ) + " to " + CORE::UInt32ToString( CPackIndexDocument::SchemaVersion ) );

    // the pack: the fingerprint first (paths and sizes), the content only when it is asked for
    const CPackIndexIdentity& a = stored.identity;
    const CPackIndexIdentity& b = current.identity;
    if ( a.fileCount != b.fileCount )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_SOURCE_CHANGED, "the file count changed from " + CORE::UInt64ToString( a.fileCount ) + " to " + CORE::UInt64ToString( b.fileCount ) );
    if ( a.totalBytes != b.totalBytes )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_SOURCE_CHANGED, "the size of the pack changed from " + CORE::UInt64ToString( a.totalBytes ) + " to " + CORE::UInt64ToString( b.totalBytes ) );
    if ( a.treeHash != b.treeHash )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_SOURCE_CHANGED, "the paths or sizes of the files changed" );
    if ( request.verifyContent && ( b.contentHash.IsNULLOrEmpty() || a.contentHash != b.contentHash ) )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_SOURCE_CHANGED, "the content of the files changed" );

    // the settings and the tools
    const CPackIndexProducer& pa = stored.producer;
    const CPackIndexProducer& pb = current.producer;
    if ( pa.settingsHash != pb.settingsHash )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_SETTINGS_CHANGED, "the index settings changed" );
    CPackIndexProducer::TPluginVersionMap::const_iterator p = pb.plugins.begin();
    while ( p != pb.plugins.end() )
    {
        CPackIndexProducer::TPluginVersionMap::const_iterator found = pa.plugins.find( (*p).first );
        if ( found == pa.plugins.end() )
            return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_VERSION_CHANGED, (*p).first + " is new" );
        if ( (*found).second != (*p).second )
            return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_VERSION_CHANGED, (*p).first + " changed from version " + CORE::UInt32ToString( (*found).second ) + " to " + CORE::UInt32ToString( (*p).second ) );
        ++p;
    }
    p = pa.plugins.begin();
    while ( p != pa.plugins.end() )
    {
        if ( pb.plugins.find( (*p).first ) == pb.plugins.end() )
            return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_VERSION_CHANGED, (*p).first + " is gone" );
        ++p;
    }
    if ( pa.kindRegistryHash != pb.kindRegistryHash )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_VERSION_CHANGED, "the kind registry changed" );
    if ( pa.groupRulesVersion != pb.groupRulesVersion || pa.groupRulesHash != pb.groupRulesHash )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_VERSION_CHANGED, "the group rules changed" );
    if ( pa.schemaWriter != pb.schemaWriter )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_VERSION_CHANGED, "the indexer changed from version " + CORE::UInt32ToString( pa.schemaWriter ) + " to " + CORE::UInt32ToString( pb.schemaWriter ) );
    if ( pa.indexKey != pb.indexKey )
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_VERSION_CHANGED, "the index key differs" );
    return CPackGateDecision( CPackGateDecision::OUTCOME_FRESH, ACTIVITYREASON_UNCHANGED );
}

/*-------------------------------------------------------------------------*/

CPackGateDecision
CPackWorkGate::DecideAnalyse( const CPackIndexResource* stored, const CORE::CString& currentKey, UInt64 currentSize, const CPackRedoRequest& request )
{GUCEF_TRACE;

    if ( request.reanalyse )
        return CPackGateDecision( CPackGateDecision::OUTCOME_FORCED, ACTIVITYREASON_FORCED, "re-analyse" );
    const CPackIndexStepStatus* step = GUCEF_NULL != stored ? stored->status.FindStep( CPackIndexStatus::StepAnalyse ) : GUCEF_NULL;
    if ( GUCEF_NULL == step )
        return CPackGateDecision( CPackGateDecision::OUTCOME_MISSING, ACTIVITYREASON_NEW );
    if ( step->key != currentKey )
    {
        if ( stored->size != currentSize )
            return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_SOURCE_CHANGED, "the size of the file changed" );
        return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_VERSION_CHANGED, "the analyser or its settings changed" );
    }
    if ( CPackIndexStepStatus::Failed == step->state && request.retryFailed )
        return CPackGateDecision( CPackGateDecision::OUTCOME_FORCED, ACTIVITYREASON_FORCED, "retry of a failed analysis" );
    return CPackGateDecision( CPackGateDecision::OUTCOME_FRESH, ACTIVITYREASON_UNCHANGED, CPackIndexStepStatus::Failed == step->state ? CORE::CString( "failed before: not tried again" ) : CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

CPackGateDecision
CPackWorkGate::DecideConvert( const CAssetCacheEntry* stored, const CAssetCacheEntry& current, bool licenceRefused, const CORE::CString& licenceReason, const CPackRedoRequest& request )
{GUCEF_TRACE;

    if ( licenceRefused )
        return CPackGateDecision( CPackGateDecision::OUTCOME_REFUSED, ACTIVITYREASON_LICENCE, licenceReason );
    if ( request.reimport )
        return CPackGateDecision( CPackGateDecision::OUTCOME_FORCED, ACTIVITYREASON_FORCED, "re-import" );
    if ( GUCEF_NULL == stored )
        return CPackGateDecision( CPackGateDecision::OUTCOME_MISSING, ACTIVITYREASON_NEW );
    CORE::CString detail;
    const TActivityReason reason = stored->CompareWith( current, detail );
    if ( ACTIVITYREASON_UNCHANGED == reason )
        return CPackGateDecision( CPackGateDecision::OUTCOME_FRESH, reason, detail );
    return CPackGateDecision( ACTIVITYREASON_NEW == reason ? CPackGateDecision::OUTCOME_MISSING : CPackGateDecision::OUTCOME_STALE, reason, detail );
}

/*-------------------------------------------------------------------------*/

CPackGateDecision
CPackWorkGate::DecideThumbnail( const CPackIndexResource* stored, const CPackIndexThumbnail& wanted, const CPackRedoRequest& request )
{GUCEF_TRACE;

    if ( request.rethumbnail )
        return CPackGateDecision( CPackGateDecision::OUTCOME_FORCED, ACTIVITYREASON_FORCED, "re-thumbnail" );
    if ( GUCEF_NULL == stored || stored->thumbnails.empty() )
        return CPackGateDecision( CPackGateDecision::OUTCOME_MISSING, ACTIVITYREASON_NEW );
    for ( size_t i=0; i<stored->thumbnails.size(); ++i )
    {
        const CPackIndexThumbnail& t = stored->thumbnails[ i ];
        if ( t.size == wanted.size && t.rendererId == wanted.rendererId )
        {
            if ( t.digest != wanted.digest )
                return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_SOURCE_CHANGED, "the file the thumbnail shows changed" );
            if ( t.rendererVersion != wanted.rendererVersion )
                return CPackGateDecision( CPackGateDecision::OUTCOME_STALE, ACTIVITYREASON_VERSION_CHANGED, "the renderer changed from version " + CORE::UInt32ToString( t.rendererVersion ) + " to " + CORE::UInt32ToString( wanted.rendererVersion ) );
            return CPackGateDecision( CPackGateDecision::OUTCOME_FRESH, ACTIVITYREASON_UNCHANGED );
        }
    }
    return CPackGateDecision( CPackGateDecision::OUTCOME_MISSING, ACTIVITYREASON_NEW );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
