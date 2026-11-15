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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#include "gucefASSETSTORE_CAssetToolUris.h"
#define GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLURIS_H ? */

#include "gucefASSETSTORE_CAssetStoreReadTool.h"

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

const UInt32 RelationsPerAsset = 50;
const UInt32 IngestHistoryEntries = 10;

void
ReadRange( const CORE::CDataNode& arguments, const char* name, Int64& minimum, Int64& maximum )
{GUCEF_TRACE;

    const CORE::CDataNode* range = arguments.FindChild( name );
    if ( GUCEF_NULL == range )
        return;
    const CORE::CVariant& low = range->GetAttributeValueOrChildValueByName( "min" );
    const CORE::CVariant& high = range->GetAttributeValueOrChildValueByName( "max" );
    if ( !low.IsNULLOrEmpty() )
        minimum = (Int64) low.AsFloat64( -1.0, true );
    if ( !high.IsNULLOrEmpty() )
        maximum = (Int64) high.AsFloat64( -1.0, true );
}

void
ReadTriState( const CORE::CDataNode& arguments, const char* name, Int64& value )
{GUCEF_TRACE;

    const CORE::CVariant& flag = arguments.GetAttributeValueOrChildValueByName( name );
    if ( !flag.IsNULLOrEmpty() )
        value = flag.AsBool( false, true ) ? 1 : 0;
}

/** "p<page index>" */
bool
ParsePageCursor( const CORE::CString& cursor, Int64& pageIndex )
{GUCEF_TRACE;

    pageIndex = 0;
    if ( cursor.IsNULLOrEmpty() )
        return true;
    if ( cursor.Length() < 2 || 'p' != cursor[ 0 ] )
        return false;
    for ( UInt32 i=1; i<cursor.Length(); ++i )
        if ( cursor[ i ] < '0' || cursor[ i ] > '9' )
            return false;
    pageIndex = (Int64) CORE::StringToUInt32( cursor.CutChars( 1, true ) );
    return true;
}

void
SetStringList( CORE::CDataNode& object, const char* name, const CORE::CString::StringSet& values )
{GUCEF_TRACE;

    CORE::CString::StringVector list( values.begin(), values.end() );
    CORE::CDataNode* node = object.AddChild( name, GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<list.size(); ++i )
        node->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_STRING )->SetValue( CORE::CVariant( list[ i ] ) );
}

void
SetStringList( CORE::CDataNode& object, const char* name, const CORE::CString::StringVector& values )
{GUCEF_TRACE;

    CORE::CDataNode* node = object.AddChild( name, GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<values.size(); ++i )
        node->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_STRING )->SetValue( CORE::CVariant( values[ i ] ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreReadTool::CAssetStoreReadTool( CAssetToolContext& context, const CORE::CString& name )
    : CAssetToolBase( context, name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreReadTool::~CAssetStoreReadTool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStoreReadTool::BuildQuery( const CORE::CDataNode& arguments, CAssetSearchQuery& query )
{GUCEF_TRACE;

    query.Clear();
    query.text = GetStringArgument( arguments, "text" );
    GetStringSetArgument( arguments, "kinds", query.kinds );
    GetStringSetArgument( arguments, "categories", query.categories );
    GetStringSetArgument( arguments, "themes", query.themes );
    GetStringSetArgument( arguments, "artStyles", query.artStyles );
    GetStringSetArgument( arguments, "sources", query.sources );
    GetStringSetArgument( arguments, "sellers", query.sellers );
    GetStringSetArgument( arguments, "licenceClasses", query.licenceClasses );
    GetStringSetArgument( arguments, "requireLicenceRestrictions", query.requireLicenceRestrictions );
    GetStringSetArgument( arguments, "excludeLicenceRestrictions", query.excludeLicenceRestrictions );
    GetStringSetArgument( arguments, "formats", query.formats );
    GetStringSetArgument( arguments, "pbrRoles", query.pbrRoles );
    GetStringSetArgument( arguments, "storePackIds", query.packIds );
    ReadRange( arguments, "triangles", query.minTriangles, query.maxTriangles );
    ReadRange( arguments, "vertices", query.minVertices, query.maxVertices );
    ReadRange( arguments, "assetBytes", query.minAssetBytes, query.maxAssetBytes );
    ReadRange( arguments, "packBytes", query.minPackBytes, query.maxPackBytes );
    ReadRange( arguments, "textureResolution", query.minTextureResolution, query.maxTextureResolution );
    const CORE::CDataNode* seconds = arguments.FindChild( "animationSeconds" );
    if ( GUCEF_NULL != seconds )
    {
        const CORE::CVariant& low = seconds->GetAttributeValueOrChildValueByName( "min" );
        const CORE::CVariant& high = seconds->GetAttributeValueOrChildValueByName( "max" );
        if ( !low.IsNULLOrEmpty() )
            query.minAnimationSeconds = low.AsFloat64( -1.0, true );
        if ( !high.IsNULLOrEmpty() )
            query.maxAnimationSeconds = high.AsFloat64( -1.0, true );
    }
    ReadTriState( arguments, "rigged", query.rigged );
    ReadTriState( arguments, "animated", query.animated );
    ReadTriState( arguments, "hasLods", query.hasLods );
    ReadTriState( arguments, "hasCollision", query.hasCollision );
    query.unclassified = GetBoolArgument( arguments, "unclassified", false );
    const CORE::CString resultKind = GetStringArgument( arguments, "resultKind" );
    if ( !resultKind.IsNULLOrEmpty() )
        query.resultKind = resultKind;
    const CORE::CString sort = GetStringArgument( arguments, "sort" );
    if ( !sort.IsNULLOrEmpty() )
        query.sort = sort;
    query.withFacets = GetBoolArgument( arguments, "withFacets", true );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreReadTool::AddLicenceSummary( const CAssetLicence& licence, const CORE::CString& storePackId, CORE::CDataNode& object )
{GUCEF_TRACE;

    CORE::CDataNode* node = object.AddChild( "licence", GUCEF_DATATYPE_OBJECT );
    node->SetAttribute( "licenceId", licence.licenceId );
    node->SetAttribute( "licenceClass", licence.licenceClass );
    node->SetAttribute( "name", licence.name );
    SetTextOrNull( *node, "url", CORE::CRedactor::ForUrl().RedactUrl( licence.url ) );
    SetStringList( *node, "restrictions", licence.restrictions );
    CORE::CString reason;
    const bool allowed = m_context.GetGate().IsStorePackAllowed( storePackId, reason );
    node->SetAttribute( "contentExposure", allowed ? "allowed" : "withheld" );
    SetTextOrNull( *node, "exposureReason", allowed ? CORE::CString::Empty : reason );
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreReadTool::Search( const CORE::CDataNode& arguments, COM::CMcpToolResult& result, bool unclassified )
{GUCEF_TRACE;

    Int64 pageIndex = 0;
    if ( !ParsePageCursor( GetStringArgument( arguments, "cursor" ), pageIndex ) )
    {
        m_context.SetError( result, "invalid-arguments", "The cursor is not one this tool gave", false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    const UInt32 limit = m_context.GetPageLimit( GetUInt32Argument( arguments, "limit", 0 ) );
    CAssetSearchResultPage page;
    TAssetStoreStatus status = ASSETSTORE_STATUS_OK;
    const CORE::CString savedSearchId = GetStringArgument( arguments, "savedSearchId" );
    if ( !unclassified && !savedSearchId.IsNULLOrEmpty() )
    {
        status = m_context.GetService().RunSavedSearch( savedSearchId, (UInt32) pageIndex, page );
    }
    else
    {
        CAssetSearchQuery query;
        if ( unclassified )
        {
            query.unclassified = true;
            query.resultKind = "pack" == GetStringArgument( arguments, "level" ) ? "packs" : "assets";
            const CORE::CString kind = GetStringArgument( arguments, "kind" );
            if ( !kind.IsNULLOrEmpty() )
                query.kinds.insert( kind );
            const CORE::CString storePackId = GetStringArgument( arguments, "storePackId" );
            if ( !storePackId.IsNULLOrEmpty() )
                query.packIds.insert( storePackId );
            query.withFacets = false;
        }
        else
        {
            BuildQuery( arguments, query );
        }
        query.pageIndex = pageIndex;
        query.pageSize = (Int64) limit;
        status = m_context.GetService().Search( query, page );
    }
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, CORE::CString::Empty, GUCEF_NULL );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringMap exposures;
    for ( size_t i=0; i<page.hits.size(); ++i )
    {
        const CAssetSearchHit& hit = page.hits[ i ];
        CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        SetTextOrNull( *item, "storeAssetId", hit.assetId );
        item->SetAttribute( "storePackId", hit.packId );
        item->SetAttribute( "name", hit.name );
        item->SetAttribute( "kind", hit.kind );
        SetTextOrNull( *item, "categoryId", hit.categoryId );
        item->SetAttribute( "packTitle", hit.packTitle );
        item->SetAttribute( "store", hit.sourceId );
        item->SetAttribute( "seller", hit.seller );
        item->SetAttribute( "licenceClass", hit.licenceClass );
        item->SetAttribute( "available", hit.available );
        if ( exposures.end() == exposures.find( hit.packId ) )
        {
            CORE::CString reason;
            exposures[ hit.packId ] = m_context.GetGate().IsStorePackAllowed( hit.packId, reason ) ? "allowed" : "withheld";
        }
        item->SetAttribute( "contentExposure", exposures[ hit.packId ] );
        if ( unclassified )
        {
            // what a classification goes by: keywords and the categories of the store (untrusted)
            if ( !hit.assetId.IsNULLOrEmpty() )
            {
                CAssetRecord asset;
                if ( ASSETSTORE_STATUS_OK == m_context.GetService().GetAsset( hit.assetId, asset ) )
                {
                    CORE::CDataNode* keywords = item->AddChild( "keywords", GUCEF_DATATYPE_ARRAY );
                    for ( size_t k=0; k<asset.keywords.size() && k<32; ++k )
                        keywords->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_STRING )->SetValue( CORE::CVariant( asset.keywords[ k ].keyword ) );
                }
            }
            CAssetPackRecord pack;
            if ( ASSETSTORE_STATUS_OK == m_context.GetService().GetPack( hit.packId, pack ) )
            {
                CORE::CDataNode* categories = item->AddChild( "sourceCategories", GUCEF_DATATYPE_ARRAY );
                for ( size_t c=0; c<pack.sourceCategories.size() && c<16; ++c )
                    categories->AddChild( MakeUntrusted( "store-category", pack.sourceCategories[ c ] ) );
                item->SetAttribute( "store", pack.sourceId );
            }
        }
    }
    document.SetAttribute( "total", page.total );
    const UInt64 shown = ( (UInt64) pageIndex + 1 ) * (UInt64) limit;
    if ( shown < page.total && !page.hits.empty() )
        document.SetAttribute( "nextCursor", "p" + CORE::ToString( (UInt32) pageIndex + 1 ) );
    else
        SetNull( document, "nextCursor" );
    if ( !unclassified )
    {
        CORE::CDataNode* facets = document.AddChild( "facets", GUCEF_DATATYPE_OBJECT );
        CAssetSearchResultPage::TFacetMap::const_iterator f = page.facets.begin();
        while ( f != page.facets.end() )
        {
            CORE::CDataNode* facet = facets->AddChild( (*f).first, GUCEF_DATATYPE_OBJECT );
            CAssetSearchResultPage::TValueCountMap::const_iterator v = (*f).second.begin();
            while ( v != (*f).second.end() )
            {
                facet->SetAttribute( (*v).first, (*v).second );
                ++v;
            }
            ++f;
        }
    }
    SetStructured( result, document );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreReadTool::Asset( const CORE::CDataNode& arguments, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    const CORE::CString assetId = GetStringArgument( arguments, "storeAssetId" );
    CAssetRecord asset;
    const TAssetStoreStatus status = m_context.GetService().GetAsset( assetId, asset );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, "There is no asset " + assetId + " in the store", GUCEF_NULL );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    asset.Serialize( document, settings );
    document.SetAttribute( "storeAssetId", asset.assetId );
    document.SetAttribute( "storePackId", asset.packId );

    CAssetRelationPage relations;
    if ( ASSETSTORE_STATUS_OK == m_context.GetService().GetRelations( assetId, CORE::CString::Empty, RelationsPerAsset, relations ) )
    {
        CORE::CDataNode* list = document.AddChild( "relations", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<relations.entries.size(); ++i )
        {
            CORE::CDataNode* relation = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            relation->SetAttribute( "from", relations.entries[ i ].fromAssetId );
            relation->SetAttribute( "kind", relations.entries[ i ].kind );
            relation->SetAttribute( "to", relations.entries[ i ].toAssetId );
        }
        document.SetAttribute( "relationsTotal", relations.total );
    }
    CAssetPackRecord pack;
    if ( ASSETSTORE_STATUS_OK == m_context.GetService().GetPack( asset.packId, pack ) )
    {
        CORE::CDataNode* summary = document.AddChild( "pack", GUCEF_DATATYPE_OBJECT );
        summary->SetAttribute( "storePackId", pack.packId );
        summary->SetAttribute( "title", pack.title );
        summary->SetAttribute( "seller", pack.seller );
        summary->SetAttribute( "store", pack.sourceId );
        summary->SetAttribute( "locationId", pack.locationId );
        AddLicenceSummary( pack.licence, pack.packId, document );
        CAssetLocationPage locations;
        bool online = false;
        if ( ASSETSTORE_STATUS_OK == m_context.GetService().GetLocations( locations ) )
            for ( size_t l=0; l<locations.entries.size(); ++l )
                if ( locations.entries[ l ].location.locationId == pack.locationId )
                    online = locations.entries[ l ].online;
        document.SetAttribute( "available", online );
    }
    document.SetAttribute( "renderHint", "render_sheet with { \"storeAssetId\": \"" + asset.assetId + "\" } where the licence allows content" );
    SetStructured( result, document );
    result.AddContent( COM::CMcpContent::ResourceLink( CAssetToolUris::ForStoreRecord( "assets", asset.assetId ), "asset record", "application/json" ) );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreReadTool::Pack( const CORE::CDataNode& arguments, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    const CORE::CString packId = GetStringArgument( arguments, "storePackId" );
    CAssetPackRecord pack;
    const TAssetStoreStatus status = m_context.GetService().GetPack( packId, pack );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, "There is no pack " + packId + " in the store", GUCEF_NULL );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    pack.Serialize( document, settings );
    document.SetAttribute( "storePackId", pack.packId );
    document.DelAttribute( "description" );
    document.AddChild( MakeUntrusted( "store-description", pack.description ) )->SetName( "description" );
    document.DelAttribute( "sourceUrl" );
    SetTextOrNull( document, "sourceUrl", CORE::CRedactor::ForUrl().RedactUrl( pack.sourceUrl ) );
    while ( document.DelChild( "sourceCategories" ) ) {}
    CORE::CDataNode* untrustedCategories = document.AddChild( "sourceCategories", GUCEF_DATATYPE_ARRAY );
    for ( size_t c=0; c<pack.sourceCategories.size() && c<32; ++c )
        untrustedCategories->AddChild( MakeUntrusted( "store-category", pack.sourceCategories[ c ] ) );
    while ( document.DelChild( "licence" ) ) {}
    AddLicenceSummary( pack.licence, pack.packId, document );

    // the assets per kind, from the first page of its assets
    CAssetRecordPage assets;
    if ( ASSETSTORE_STATUS_OK == m_context.GetService().GetAssets( packId, CORE::CString::Empty, CIAssetStoreService::MaxPageEntries, assets ) )
    {
        CORE::CDataNode* kinds = document.AddChild( "assetsPerKind", GUCEF_DATATYPE_OBJECT );
        CORE::CString::StringMap counted;
        GUCEF::map< CORE::CString, UInt32 > counts;
        for ( size_t a=0; a<assets.entries.size(); ++a )
            ++counts[ assets.entries[ a ].kind ];
        GUCEF::map< CORE::CString, UInt32 >::const_iterator k = counts.begin();
        while ( k != counts.end() )
        {
            kinds->SetAttribute( (*k).first.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : (*k).first, (*k).second );
            ++k;
        }
        document.SetAttribute( "assetsCounted", (UInt32) assets.entries.size() );
        document.SetAttribute( "assetsTotal", assets.total );
    }
    CAssetIngestHistoryPage history;
    if ( ASSETSTORE_STATUS_OK == m_context.GetService().GetIngestHistory( packId, CORE::CString::Empty, IngestHistoryEntries, history ) )
    {
        CORE::CDataNode* list = document.AddChild( "ingestHistory", GUCEF_DATATYPE_ARRAY );
        for ( size_t h=0; h<history.entries.size(); ++h )
        {
            CORE::CDataNode* entry = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            entry->SetAttribute( "ingestId", history.entries[ h ].ingestId );
            entry->SetAttribute( "outcome", history.entries[ h ].outcome );
            entry->SetAttribute( "step", history.entries[ h ].step );
            entry->SetAttribute( "policy", history.entries[ h ].policy );
        }
    }
    SetStructured( result, document );
    result.AddContent( COM::CMcpContent::ResourceLink( CAssetToolUris::ForStoreRecord( "packs", pack.packId ), "pack record", "application/json" ) );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreReadTool::Licence( const CORE::CDataNode& arguments, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    CORE::CString id = GetStringArgument( arguments, "licenceId" );
    const CORE::CString packId = GetStringArgument( arguments, "storePackId" );
    if ( id.IsNULLOrEmpty() )
        id = packId;
    if ( id.IsNULLOrEmpty() )
    {
        m_context.SetError( result, "invalid-arguments", "Give licenceId or storePackId", false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    CAssetLicence licence;
    const TAssetStoreStatus status = m_context.GetService().GetLicence( id, licence );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, "There is no licence " + id + " in the store", GUCEF_NULL );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    document.SetAttribute( "licenceId", licence.licenceId );
    document.SetAttribute( "licenceClass", licence.licenceClass );
    document.SetAttribute( "name", licence.name );
    SetTextOrNull( document, "url", CORE::CRedactor::ForUrl().RedactUrl( licence.url ) );
    SetStringList( document, "restrictions", licence.restrictions );
    document.AddChild( MakeUntrusted( "licence-text", licence.text ) )->SetName( "text" );
    SetStructured( result, document );
    if ( !licence.licenceId.IsNULLOrEmpty() )
        result.AddContent( COM::CMcpContent::ResourceLink( CAssetToolUris::ForStoreLicenceText( licence.licenceId ), "licence text", "text/plain" ) );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreReadTool::Locations( COM::CMcpToolResult& result )
{GUCEF_TRACE;

    CAssetLocationPage locations;
    const TAssetStoreStatus status = m_context.GetService().GetLocations( locations );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, CORE::CString::Empty, GUCEF_NULL );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<locations.entries.size(); ++i )
    {
        const CAssetLocationInfo& info = locations.entries[ i ];
        CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "locationId", info.location.locationId );
        item->SetAttribute( "name", info.location.name );
        item->SetAttribute( "online", info.online );
        item->SetAttribute( "writeable", info.location.writeable );
        item->SetAttribute( "root", CORE::CRedactor::ForUrl().RedactUrl( info.location.vfsRoot ) );
    }
    document.SetAttribute( "total", (UInt32) locations.entries.size() );
    SetNull( document, "nextCursor" );
    SetStructured( result, document );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreReadTool::Vocabulary( const CORE::CDataNode& arguments, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    CAssetVocabularyInfo vocabulary;
    const TAssetStoreStatus status = m_context.GetService().GetVocabulary( vocabulary );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, CORE::CString::Empty, GUCEF_NULL );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    const CORE::CString axis = GetStringArgument( arguments, "axis" );
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    if ( axis.IsNULLOrEmpty() || "kind" == axis )
        SetStringList( document, "kinds", vocabulary.kinds );
    if ( axis.IsNULLOrEmpty() || "category" == axis )
        SetStringList( document, "categories", vocabulary.categories );
    if ( axis.IsNULLOrEmpty() || "theme" == axis )
        SetStringList( document, "themes", vocabulary.themes );
    if ( axis.IsNULLOrEmpty() || "style" == axis )
        SetStringList( document, "styles", vocabulary.styles );
    if ( axis.IsNULLOrEmpty() )
    {
        SetStringList( document, "licenceClasses", vocabulary.licenceClasses );
        SetStringList( document, "restrictionFlags", vocabulary.restrictionFlags );
    }
    SetStructured( result, document );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreReadTool::SavedSearches( const CORE::CDataNode& arguments, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    CAssetSavedSearchPage page;
    const TAssetStoreStatus status = m_context.GetService().GetSavedSearches( GetStringArgument( arguments, "cursor" ),
                                                                             m_context.GetPageLimit( GetUInt32Argument( arguments, "limit", 0 ) ), page );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, CORE::CString::Empty, GUCEF_NULL );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<page.entries.size(); ++i )
    {
        CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "searchId", page.entries[ i ].searchId );
        item->SetAttribute( "name", page.entries[ i ].name );
        item->SetAttribute( "owner", page.entries[ i ].precanned ? "seed" : "user" );
        CORE::CDataNode* query = item->AddChild( "query", GUCEF_DATATYPE_OBJECT );
        page.entries[ i ].query.Serialize( *query, settings );
    }
    document.SetAttribute( "total", page.total );
    SetTextOrNull( document, "nextCursor", page.nextPageToken );
    SetStructured( result, document );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreReadTool::Compute( COM::CMcpRequestContext& context      ,
                              const CORE::CDataNode& arguments      ,
                              COM::CMcpToolResult& result           ,
                              COM::CMcpInputRequired& inputRequired ,
                              SCallState& state                     )
{GUCEF_TRACE;

    if ( "store_search" == m_name )
        return Search( arguments, result, false );
    if ( "store_unclassified" == m_name )
        return Search( arguments, result, true );
    if ( "store_asset" == m_name )
        return Asset( arguments, result );
    if ( "store_pack" == m_name )
        return Pack( arguments, result );
    if ( "store_licence" == m_name )
        return Licence( arguments, result );
    if ( "store_locations" == m_name )
        return Locations( result );
    if ( "store_vocabulary" == m_name )
        return Vocabulary( arguments, result );
    if ( "store_saved_searches" == m_name )
        return SavedSearches( arguments, result );
    m_context.SetError( result, "unsupported", "unsupported: the tool " + m_name + " is not a store read tool", false );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
