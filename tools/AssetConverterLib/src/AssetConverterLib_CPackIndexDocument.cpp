/*
 *  AssetConverterLib: the normalized pack index document (gucef.packIndex, version 1) as a typed model
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#ifndef ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H
#include "AssetConverterLib_CPackGroupVocabulary.h"
#define ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H
#endif /* ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "AssetConverterLib_CPackIndexDocument.h"

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

const CORE::CString CPackIndexDocument::FormatId = "gucef.packIndex";
const CORE::CString CPackIndexDocument::StateBuilding = "building";
const CORE::CString CPackIndexDocument::StatePartial = "partial";
const CORE::CString CPackIndexDocument::StateComplete = "complete";
const CORE::CString CPackIndexDocument::StateCompleteWithFailures = "completeWithFailures";
const CORE::CString CPackIndexDocument::StateFailed = "failed";
const CORE::CString CPackIndexDocument::LevelStructure = "structure";
const CORE::CString CPackIndexDocument::LevelAnalysis = "analysis";
const CORE::CString CPackIndexDocument::LevelOutputs = "outputs";
const CORE::CString CPackIndexDocument::LevelThumbnails = "thumbnails";

/*-------------------------------------------------------------------------*/

CPackIndexDocument::CPackIndexDocument( void )
    : version( SchemaVersion )
    , levels()
    , state( StateBuilding )
    , stateReasons()
    , identity()
    , producer()
    , profile()
    , licence()
    , counts()
    , groups()
    , shards()
    , resources()
    , documentHash()
    , producedUtc()
    , producedDurationMs( 0 )
    , producedHost()
    , producedJobId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPackIndexDocument::Clear( void )
{GUCEF_TRACE;

    *this = CPackIndexDocument();
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexDocument::IsComplete( void ) const
{GUCEF_TRACE;

    return StateComplete == state || StateCompleteWithFailures == state;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexDocument::HasLevel( const CORE::CString& level ) const
{GUCEF_TRACE;

    return std::find( levels.begin(), levels.end(), level ) != levels.end();
}

/*-------------------------------------------------------------------------*/

void
CPackIndexDocument::AddLevel( const CORE::CString& level )
{GUCEF_TRACE;

    if ( !HasLevel( level ) )
        levels.push_back( level );
}

/*-------------------------------------------------------------------------*/

namespace {

UInt32
LevelOrder( const CORE::CString& level )
{
    if ( CPackIndexDocument::LevelStructure == level ) return 0;
    if ( CPackIndexDocument::LevelAnalysis == level ) return 1;
    if ( CPackIndexDocument::LevelOutputs == level ) return 2;
    if ( CPackIndexDocument::LevelThumbnails == level ) return 3;
    return 4;
}

bool
LevelLess( const CORE::CString& a, const CORE::CString& b )
{
    const UInt32 oa = LevelOrder( a );
    const UInt32 ob = LevelOrder( b );
    return oa != ob ? oa < ob : a < b;
}

bool
GroupLess( const CPackIndexGroup& a, const CPackIndexGroup& b )
{
    const UInt32 oa = CPackGroupVocabulary::GetGroupOrder( a.id );
    const UInt32 ob = CPackGroupVocabulary::GetGroupOrder( b.id );
    return oa != ob ? oa < ob : a.id < b.id;
}

bool
SubGroupLess( const CPackIndexSubGroup& a, const CPackIndexSubGroup& b )
{
    return a.id < b.id;
}

}

/*-------------------------------------------------------------------------*/

void
CPackIndexDocument::Normalise( void )
{GUCEF_TRACE;

    std::sort( resources.begin(), resources.end() );
    std::sort( levels.begin(), levels.end(), LevelLess );
    levels.erase( std::unique( levels.begin(), levels.end() ), levels.end() );
    std::sort( groups.begin(), groups.end(), GroupLess );
    for ( size_t g=0; g<groups.size(); ++g )
        std::sort( groups[ g ].subGroups.begin(), groups[ g ].subGroups.end(), SubGroupLess );
    std::sort( shards.begin(), shards.end() );
    std::sort( licence.restrictions.begin(), licence.restrictions.end() );
    licence.restrictions.erase( std::unique( licence.restrictions.begin(), licence.restrictions.end() ), licence.restrictions.end() );
    std::sort( stateReasons.begin(), stateReasons.end() );
}

/*-------------------------------------------------------------------------*/

namespace {

CORE::CString
LastSegment( const CORE::CString& subGroupId )
{
    const Int32 slash = subGroupId.HasChar( '/', false );
    return slash >= 0 ? subGroupId.SubstrFromRange( (UInt32) slash+1, subGroupId.Length() ) : subGroupId;
}

/** "polygon-shops" as "Polygon shops": the text the views show for the last segment of a sub-group id */
CORE::CString
DisplayOfSegment( const CORE::CString& segment )
{
    CORE::CString text = segment.ReplaceChar( '-', ' ' ).ReplaceChar( '_', ' ' );
    if ( text.Length() > 0 )
        text = text.SubstrFromRange( 0, 1 ).Uppercase() + text.SubstrFromRange( 1, text.Length() );
    return text;
}

}

void
CPackIndexDocument::UpdateSummaries( void )
{GUCEF_TRACE;

    typedef GUCEF::map< CORE::CString, CPackIndexSubGroup > TSubMap;
    typedef GUCEF::map< CORE::CString, CPackIndexGroup > TGroupMap;
    typedef GUCEF::map< CORE::CString, TSubMap > TGroupSubMap;
    TGroupMap groupMap;
    TGroupSubMap subMaps;

    // the presentation hints survive a recount
    GUCEF::map< CORE::CString, CORE::CString > presentations;
    for ( size_t g=0; g<groups.size(); ++g )
        if ( !groups[ g ].presentation.IsNULLOrEmpty() )
            presentations[ groups[ g ].id ] = groups[ g ].presentation;

    const UInt32 noise = counts.noise;
    const UInt32 metadata = counts.metadata;
    counts = CPackIndexCounts();
    counts.noise = noise;
    counts.metadata = metadata;
    counts.resources = (UInt32) resources.size();
    for ( size_t i=0; i<resources.size(); ++i )
    {
        const CPackIndexResource& resource = resources[ i ];
        counts.bytes += resource.size;
        ++counts.byGroup[ resource.group ];
        ++counts.byKind[ resource.kind ];
        ++counts.byStatus[ resource.status.state ];
        CPackIndexGroup& group = groupMap[ resource.group ];
        group.id = resource.group;
        ++group.count;
        group.bytes += resource.size;
        if ( !resource.subGroup.IsNULLOrEmpty() )
        {
            CPackIndexSubGroup& sub = subMaps[ resource.group ][ resource.subGroup ];
            sub.id = resource.subGroup;
            ++sub.count;
            sub.bytes += resource.size;
        }
    }
    groups.clear();
    TGroupMap::iterator g = groupMap.begin();
    while ( g != groupMap.end() )
    {
        CPackIndexGroup& group = (*g).second;
        group.name = CPackGroupVocabulary::GetGroupName( group.id );
        GUCEF::map< CORE::CString, CORE::CString >::const_iterator p = presentations.find( group.id );
        if ( p != presentations.end() )
            group.presentation = (*p).second;
        TSubMap& subs = subMaps[ group.id ];
        TSubMap::iterator s = subs.begin();
        while ( s != subs.end() )
        {
            (*s).second.name = DisplayOfSegment( LastSegment( (*s).second.id ) );
            group.subGroups.push_back( (*s).second );
            ++s;
        }
        group.subGroupsTruncated = group.subGroups.size() > CPackIndexGroup::MaxSubGroupsInManifest;
        groups.push_back( group );
        ++g;
    }
    std::sort( groups.begin(), groups.end(), GroupLess );
}

/*-------------------------------------------------------------------------*/

const CPackIndexResource*
CPackIndexDocument::FindResourceByPath( const CORE::CString& path ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<resources.size(); ++i )
        if ( resources[ i ].path == path )
            return &resources[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CPackIndexResource*
CPackIndexDocument::FindResourceById( const CORE::CString& resourceId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<resources.size(); ++i )
        if ( resources[ i ].id == resourceId )
            return &resources[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CPackIndexGroup*
CPackIndexDocument::FindGroup( const CORE::CString& groupId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<groups.size(); ++i )
        if ( groups[ i ].id == groupId )
            return &groups[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexDocument::SerializeManifest( CORE::CDataNode& node, bool withResources, bool withProducedAndHash ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "format", FormatId );
    node.SetAttribute( "version", version );
    node.SetValuesOfChildByName( "levels", levels );
    CPackIndexSerial::SetTextAlways( node, "state", state );
    CPackIndexSerial::SetList( node, "stateReasons", stateReasons );
    identity.Serialize( *node.AddChild( "identity", GUCEF_DATATYPE_OBJECT ) );
    producer.Serialize( *node.AddChild( "producer", GUCEF_DATATYPE_OBJECT ) );
    profile.Serialize( *node.AddChild( "profile", GUCEF_DATATYPE_OBJECT ) );
    if ( !licence.IsEmpty() )
        licence.Serialize( *node.AddChild( "licence", GUCEF_DATATYPE_OBJECT ) );
    counts.Serialize( *node.AddChild( "counts", GUCEF_DATATYPE_OBJECT ) );
    CORE::CDataNode* groupList = node.AddChild( "groups", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<groups.size(); ++i )
        groups[ i ].Serialize( *groupList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    if ( !shards.empty() )
    {
        CORE::CDataNode* shardList = node.AddChild( "shards", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<shards.size(); ++i )
            shards[ i ].Serialize( *shardList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }
    if ( withResources )
    {
        CORE::CDataNode* list = node.AddChild( "resources", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<resources.size(); ++i )
            resources[ i ].Serialize( *list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }
    if ( withProducedAndHash )
    {
        CPackIndexSerial::SetText( node, "documentHash", documentHash );
        CORE::CDataNode* produced = node.AddChild( "produced", GUCEF_DATATYPE_OBJECT );
        CPackIndexSerial::SetText( *produced, "utc", producedUtc );
        produced->SetAttribute( "durationMs", producedDurationMs );
        CPackIndexSerial::SetText( *produced, "host", producedHost );
        CPackIndexSerial::SetText( *produced, "jobId", producedJobId );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexDocument::DeserializeManifest( const CORE::CDataNode& node )
{GUCEF_TRACE;

    Clear();
    version = (UInt32) CPackIndexSerial::GetCount( node, "version" );
    CPackIndexSerial::GetList( node, "levels", levels );
    state = CPackIndexSerial::GetText( node, "state" );
    CPackIndexSerial::GetList( node, "stateReasons", stateReasons );
    const CORE::CDataNode* child = node.FindChild( "identity" );
    if ( GUCEF_NULL == child || !identity.Deserialize( *child ) )
        return false;
    child = node.FindChild( "producer" );
    if ( GUCEF_NULL != child )
        producer.Deserialize( *child );
    child = node.FindChild( "profile" );
    if ( GUCEF_NULL != child )
        profile.Deserialize( *child );
    child = node.FindChild( "licence" );
    if ( GUCEF_NULL != child )
        licence.Deserialize( *child );
    child = node.FindChild( "counts" );
    if ( GUCEF_NULL != child )
        counts.Deserialize( *child );
    child = node.FindChild( "groups" );
    if ( GUCEF_NULL != child )
    {
        CORE::CDataNode::const_iterator i = child->ConstBegin();
        while ( i != child->ConstEnd() )
        {
            CPackIndexGroup group;
            if ( group.Deserialize( *(*i) ) )
                groups.push_back( group );
            ++i;
        }
    }
    child = node.FindChild( "shards" );
    if ( GUCEF_NULL != child )
    {
        CORE::CDataNode::const_iterator i = child->ConstBegin();
        while ( i != child->ConstEnd() )
        {
            CPackIndexShardInfo shard;
            if ( shard.Deserialize( *(*i) ) )
                shards.push_back( shard );
            ++i;
        }
    }
    child = node.FindChild( "resources" );
    if ( GUCEF_NULL != child )
    {
        resources.reserve( child->GetNrOfDirectChildNodes() );
        CORE::CDataNode::const_iterator i = child->ConstBegin();
        while ( i != child->ConstEnd() )
        {
            CPackIndexResource resource;
            if ( resource.Deserialize( *(*i) ) )
                resources.push_back( resource );
            ++i;
        }
    }
    documentHash = CPackIndexSerial::GetText( node, "documentHash" );
    child = node.FindChild( "produced" );
    if ( GUCEF_NULL != child )
    {
        producedUtc = CPackIndexSerial::GetText( *child, "utc" );
        producedDurationMs = CPackIndexSerial::GetCount( *child, "durationMs" );
        producedHost = CPackIndexSerial::GetText( *child, "host" );
        producedJobId = CPackIndexSerial::GetText( *child, "jobId" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexDocument::ComputeDocumentHash( void ) const
{GUCEF_TRACE;

    CORE::CDataNode node;
    SerializeManifest( node, shards.empty(), false );
    return CCanonicalJson::Hash( node );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
