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

#include <algorithm>

#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#include "gucefASSETSTORE_CAssetVocabulary.h"
#define GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARY_H ? */
#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */
#include "gucefASSETSTORE_CAssetSearchEngine.h"

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

struct SScoredHit
{
    CAssetSearchHit hit;
    Int64 sortValue;
};

struct SByRelevance
{
    bool operator()( const SScoredHit& a, const SScoredHit& b ) const
    {
        if ( a.hit.score != b.hit.score )
            return a.hit.score > b.hit.score;
        if ( a.hit.name != b.hit.name )
            return a.hit.name < b.hit.name;
        return a.hit.assetId + a.hit.packId < b.hit.assetId + b.hit.packId;
    }
};

struct SByName
{
    bool operator()( const SScoredHit& a, const SScoredHit& b ) const
    {
        if ( a.hit.name != b.hit.name )
            return a.hit.name < b.hit.name;
        return a.hit.assetId + a.hit.packId < b.hit.assetId + b.hit.packId;
    }
};

struct SByValue
{
    bool operator()( const SScoredHit& a, const SScoredHit& b ) const
    {
        if ( a.sortValue != b.sortValue )
            return a.sortValue > b.sortValue;
        return SByName()( a, b );
    }
};

CORE::CVariant::VariantVector
ToVariants( const CORE::CString::StringSet& values )
{
    CORE::CVariant::VariantVector variants;
    CORE::CString::StringSet::const_iterator i = values.begin();
    while ( i != values.end() )
    {
        variants.push_back( CORE::CVariant( (*i) ) );
        ++i;
    }
    return variants;
}

void
AddRange( CORE::CDataTableQuery& query, const CORE::CString& column, Int64 low, Int64 high, bool& any )
{
    if ( low < 0 && high < 0 )
        return;
    query.WhereRange( column, low >= 0 ? CORE::CVariant( low ) : CORE::CVariant::Empty, true, high >= 0 ? CORE::CVariant( high ) : CORE::CVariant::Empty, true );
    any = true;
}

void
AddRange( CORE::CDataTableQuery& query, const CORE::CString& column, Float64 low, Float64 high, bool& any )
{
    if ( low < 0.0 && high < 0.0 )
        return;
    query.WhereRange( column, low >= 0.0 ? CORE::CVariant( low ) : CORE::CVariant::Empty, true, high >= 0.0 ? CORE::CVariant( high ) : CORE::CVariant::Empty, true );
    any = true;
}

void
AddFlag( CORE::CDataTableQuery& query, const CORE::CString& column, Int64 flag, bool& any )
{
    if ( flag < 0 )
        return;
    query.WhereEquals( column, CORE::CVariant( 0 != flag ) );
    any = true;
}

CORE::CString
JoinVfsPath( const CORE::CString& a, const CORE::CString& b )
{
    if ( a.IsNULLOrEmpty() )
        return b;
    if ( b.IsNULLOrEmpty() )
        return a;
    return a + "/" + b;
}

CORE::CString
TopLevelOf( const CORE::CString& categoryId )
{
    return categoryId.SubstrToChar( '/', true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetSearchEngine::CAssetSearchEngine( const CAssetCatalogue& catalogue, const CAssetVocabulary& vocabulary )
    : m_catalogue( catalogue )
    , m_vocabulary( vocabulary )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSearchEngine::PrefixUpperBound( const CORE::CString& prefix )
{GUCEF_TRACE;

    return prefix + "\xF4\x8F\xBF\xBF";
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchEngine::Intersect( TIdSet& target, const TIdSet& other, bool& hasTarget )
{GUCEF_TRACE;

    if ( !hasTarget )
    {
        target = other;
        hasTarget = true;
        return;
    }
    TIdSet result;
    TIdSet::const_iterator i = target.begin();
    while ( i != target.end() )
    {
        if ( other.end() != other.find( (*i) ) )
            result.insert( (*i) );
        ++i;
    }
    target = result;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSearchEngine::SelectIds( const CORE::CString& table, const CORE::CDataTableQuery& query, const CORE::CString& idColumn, TIdSet& ids ) const
{GUCEF_TRACE;

    ids.clear();
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( table, query, rows );
    for ( size_t r=0; r<rows.size(); ++r )
        ids.insert( CAssetCatalogue::GetText( rows[ r ], idColumn ) );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSearchEngine::SelectFacetObjects( const CORE::CString& facet, const CORE::CString::StringSet& values, bool all, TIdSet& objects ) const
{GUCEF_TRACE;

    objects.clear();
    if ( !all )
    {
        CORE::CDataTableQuery query;
        query.WhereEquals( "facet", CORE::CVariant( facet ) );
        query.WhereIn( "value", ToVariants( values ) );
        return SelectIds( "facets", query, "object_id", objects );
    }
    bool hasObjects = false;
    CORE::CString::StringSet::const_iterator i = values.begin();
    while ( i != values.end() )
    {
        CORE::CDataTableQuery query;
        query.WhereEquals( "facet", CORE::CVariant( facet ) );
        query.WhereEquals( "value", CORE::CVariant( (*i) ) );
        TIdSet found;
        const TAssetStoreStatus status = SelectIds( "facets", query, "object_id", found );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
        Intersect( objects, found, hasObjects );
        ++i;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSearchEngine::FindKeyword( const CORE::CString& word, bool prefix, TWeightMap& assetWeights, TWeightMap& packWeights, CORE::CString::StringMap& assetPacks ) const
{GUCEF_TRACE;

    CORE::CDataTableQuery query;
    if ( prefix )
        query.WhereRange( "keyword", CORE::CVariant( word ), true, CORE::CVariant( PrefixUpperBound( word ) ), false );
    else
        query.WhereEquals( "keyword", CORE::CVariant( word ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "keywords", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t r=0; r<rows.size(); ++r )
    {
        const CORE::CString objectId = CAssetCatalogue::GetText( rows[ r ], "object_id" );
        const Int64 weight = CAssetCatalogue::GetInt( rows[ r ], "weight", 1 );
        TWeightMap& weights = CAssetCatalogue::GetText( rows[ r ], "object_type" ) == "pack" ? packWeights : assetWeights;
        TWeightMap::iterator w = weights.find( objectId );
        if ( w == weights.end() || (*w).second < weight )
            weights[ objectId ] = weight;
        if ( &weights == &assetWeights )
            assetPacks[ objectId ] = CAssetCatalogue::GetText( rows[ r ], "pack_id" );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSearchEngine::Search( const CAssetSearchQuery& query, const TLocationStateMap& locations, CAssetSearchResultPage& page ) const
{GUCEF_TRACE;

    page.Clear();
    page.pageIndex = query.pageIndex > 0 ? (UInt64) query.pageIndex : 0;
    page.pageSize = query.pageSize > 0 ? (UInt64) query.pageSize : 50;
    const bool packsMode = query.resultKind.Equals( "packs", false );
    TAssetStoreStatus status = ASSETSTORE_STATUS_OK;

    // the keywords
    CORE::CString::StringVector words;
    m_vocabulary.GetKeywordNormalizer().NormalizeQuery( query.text, words );
    GUCEF::vector< TWeightMap > assetWeights( words.size() );
    GUCEF::vector< TWeightMap > packWeights( words.size() );
    CORE::CString::StringMap assetPacks;
    for ( size_t w=0; w<words.size(); ++w )
    {
        status = FindKeyword( words[ w ], query.prefixLastWord && w+1 == words.size(), assetWeights[ w ], packWeights[ w ], assetPacks );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
    }

    // the filters on assets
    TIdSet assetFilter;
    bool hasAssetFilter = false;
    {
        CORE::CDataTableQuery assetQuery;
        bool any = false;
        if ( !query.kinds.empty() )
        {
            assetQuery.WhereIn( "kind", ToVariants( query.kinds ) );
            any = true;
        }
        AddRange( assetQuery, "size_bytes", query.minAssetBytes, query.maxAssetBytes, any );
        if ( query.unclassified && !packsMode )
        {
            assetQuery.WhereEquals( "category_id", CORE::CVariant( CORE::CString::Empty ) );
            any = true;
        }
        if ( any )
        {
            TIdSet ids;
            if ( ASSETSTORE_STATUS_OK != ( status = SelectIds( "assets", assetQuery, "asset_id", ids ) ) )
                return status;
            Intersect( assetFilter, ids, hasAssetFilter );
        }
    }
    if ( !query.categories.empty() )
    {
        TIdSet ids;
        CORE::CString::StringSet::const_iterator c = query.categories.begin();
        while ( c != query.categories.end() )
        {
            CORE::CDataTableQuery exact;
            exact.WhereEquals( "category_id", CORE::CVariant( (*c) ) );
            CORE::CDataTableQuery below;
            below.WhereRange( "category_id", CORE::CVariant( (*c) + "/" ), true, CORE::CVariant( PrefixUpperBound( (*c) + "/" ) ), false );
            TIdSet found;
            if ( ASSETSTORE_STATUS_OK != ( status = SelectIds( "assets", exact, "asset_id", found ) ) )
                return status;
            ids.insert( found.begin(), found.end() );
            if ( ASSETSTORE_STATUS_OK != ( status = SelectIds( "assets", below, "asset_id", found ) ) )
                return status;
            ids.insert( found.begin(), found.end() );
            ++c;
        }
        Intersect( assetFilter, ids, hasAssetFilter );
    }

    // the typed property families: within a family all filters, the families joined
    {
        TIdSet familyAssets;
        bool anyFamily = false;

        CORE::CDataTableQuery modelQuery;
        bool anyModel = false;
        AddRange( modelQuery, "triangles", query.minTriangles, query.maxTriangles, anyModel );
        AddRange( modelQuery, "vertices", query.minVertices, query.maxVertices, anyModel );
        AddFlag( modelQuery, "rigged", query.rigged, anyModel );
        AddFlag( modelQuery, "animated", query.animated, anyModel );
        AddFlag( modelQuery, "has_collision", query.hasCollision, anyModel );
        if ( query.hasLods >= 0 )
        {
            if ( 0 != query.hasLods )
                modelQuery.Where( CORE::CDataTablePredicate( "lods", CORE::DATATABLE_PREDICATE_GREATER_OR_EQUAL, CORE::CVariant( (Int64) 2 ) ) );
            else
                modelQuery.Where( CORE::CDataTablePredicate( "lods", CORE::DATATABLE_PREDICATE_LESS_OR_EQUAL, CORE::CVariant( (Int64) 1 ) ) );
            anyModel = true;
        }
        if ( anyModel )
        {
            TIdSet ids;
            if ( ASSETSTORE_STATUS_OK != ( status = SelectIds( "asset_model_props", modelQuery, "asset_id", ids ) ) )
                return status;
            familyAssets.insert( ids.begin(), ids.end() );
            anyFamily = true;
        }

        CORE::CDataTableQuery textureQuery;
        bool anyTexture = false;
        AddRange( textureQuery, "resolution", query.minTextureResolution, query.maxTextureResolution, anyTexture );
        if ( anyTexture )
        {
            TIdSet ids;
            if ( ASSETSTORE_STATUS_OK != ( status = SelectIds( "asset_texture_props", textureQuery, "asset_id", ids ) ) )
                return status;
            familyAssets.insert( ids.begin(), ids.end() );
            anyFamily = true;
        }

        CORE::CDataTableQuery animationQuery;
        bool anyAnimation = false;
        AddRange( animationQuery, "length_s", query.minAnimationSeconds, query.maxAnimationSeconds, anyAnimation );
        if ( anyAnimation )
        {
            TIdSet ids;
            if ( ASSETSTORE_STATUS_OK != ( status = SelectIds( "asset_animation_props", animationQuery, "asset_id", ids ) ) )
                return status;
            familyAssets.insert( ids.begin(), ids.end() );
            anyFamily = true;
        }
        if ( anyFamily )
            Intersect( assetFilter, familyAssets, hasAssetFilter );
    }

    // the groups of the pack index: the assets that are resources of them
    if ( !query.groups.empty() || !query.subGroups.empty() )
    {
        TIdSet ids;
        if ( m_catalogue.GetSchemaVersion() >= 4 )
        {
            CORE::CDataTableQuery groupQuery;
            groupQuery.Select( "asset_id" );
            if ( !query.groups.empty() )
                groupQuery.WhereIn( "group_id", ToVariants( query.groups ) );
            if ( !query.subGroups.empty() )
                groupQuery.WhereIn( "sub_group", ToVariants( query.subGroups ) );
            if ( !query.packIds.empty() )
                groupQuery.WhereIn( "pack_id", ToVariants( query.packIds ) );
            if ( ASSETSTORE_STATUS_OK != ( status = SelectIds( "pack_resources", groupQuery, "asset_id", ids ) ) )
                return status;
            ids.erase( CORE::CString::Empty );
        }
        Intersect( assetFilter, ids, hasAssetFilter );
    }

    // the filters on packs
    TIdSet packFilter;
    bool hasPackFilter = false;
    {
        CORE::CDataTableQuery packQuery;
        bool any = false;
        if ( !query.sources.empty() )
        {
            CORE::CString::StringSet sources;
            CORE::CString::StringSet::const_iterator s = query.sources.begin();
            while ( s != query.sources.end() )
                sources.insert( (*s++).Lowercase() );
            packQuery.WhereIn( "source_id", ToVariants( sources ) );
            any = true;
        }
        if ( !query.sellers.empty() )
        {
            packQuery.WhereIn( "seller", ToVariants( query.sellers ) );
            any = true;
        }
        if ( !query.licenceClasses.empty() )
        {
            packQuery.WhereIn( "licence_class", ToVariants( query.licenceClasses ) );
            any = true;
        }
        if ( !query.packStates.empty() )
        {
            packQuery.WhereIn( "state", ToVariants( query.packStates ) );
            any = true;
        }
        if ( !query.sourceItemIds.empty() )
        {
            packQuery.WhereIn( "source_item_id", ToVariants( query.sourceItemIds ) );
            any = true;
        }
        AddRange( packQuery, "size_bytes", query.minPackBytes, query.maxPackBytes, any );
        if ( query.unclassified && packsMode )
        {
            packQuery.WhereEquals( "categories", CORE::CVariant( CORE::CString::Empty ) );
            any = true;
        }
        if ( any )
        {
            TIdSet ids;
            if ( ASSETSTORE_STATUS_OK != ( status = SelectIds( "packs", packQuery, "pack_id", ids ) ) )
                return status;
            Intersect( packFilter, ids, hasPackFilter );
        }
    }
    if ( !query.packIds.empty() )
        Intersect( packFilter, query.packIds, hasPackFilter );
    if ( !query.requireLicenceRestrictions.empty() )
    {
        TIdSet ids;
        if ( ASSETSTORE_STATUS_OK != ( status = SelectFacetObjects( "licenceRestriction", query.requireLicenceRestrictions, true, ids ) ) )
            return status;
        Intersect( packFilter, ids, hasPackFilter );
    }
    if ( !query.pbrRoles.empty() )
    {
        TIdSet ids;
        if ( ASSETSTORE_STATUS_OK != ( status = SelectFacetObjects( "pbrRole", query.pbrRoles, true, ids ) ) )
            return status;
        Intersect( packFilter, ids, hasPackFilter );
    }
    if ( !query.packFormatsOnly.empty() )
    {
        CORE::CDataTableQuery formatQuery;
        formatQuery.WhereEquals( "facet", CORE::CVariant( CORE::CString( "format" ) ) );
        CAssetCatalogue::TRowVector rows;
        if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.Select( "facets", formatQuery, rows ) ) )
            return status;
        TIdSet inside;
        TIdSet outside;
        for ( size_t r=0; r<rows.size(); ++r )
        {
            const CORE::CString packId = CAssetCatalogue::GetText( rows[ r ], "object_id" );
            if ( query.packFormatsOnly.end() != query.packFormatsOnly.find( CAssetCatalogue::GetText( rows[ r ], "value" ) ) )
                inside.insert( packId );
            else
                outside.insert( packId );
        }
        TIdSet only;
        TIdSet::const_iterator p = inside.begin();
        while ( p != inside.end() )
        {
            if ( outside.end() == outside.find( (*p) ) )
                only.insert( (*p) );
            ++p;
        }
        Intersect( packFilter, only, hasPackFilter );
    }

    // filters that hold for an asset or for its pack
    GUCEF::vector< TIdSet > eitherFilters;
    if ( !query.themes.empty() )
    {
        eitherFilters.push_back( TIdSet() );
        if ( ASSETSTORE_STATUS_OK != ( status = SelectFacetObjects( "theme", query.themes, false, eitherFilters.back() ) ) )
            return status;
    }
    if ( !query.artStyles.empty() )
    {
        eitherFilters.push_back( TIdSet() );
        if ( ASSETSTORE_STATUS_OK != ( status = SelectFacetObjects( "artStyle", query.artStyles, false, eitherFilters.back() ) ) )
            return status;
    }
    if ( !query.formats.empty() )
    {
        TIdSet ids;
        if ( ASSETSTORE_STATUS_OK != ( status = SelectFacetObjects( "format", query.formats, false, ids ) ) )
            return status;
        CORE::CDataTableQuery formatQuery;
        formatQuery.WhereIn( "source_format", ToVariants( query.formats ) );
        TIdSet assets;
        if ( ASSETSTORE_STATUS_OK != ( status = SelectIds( "assets", formatQuery, "asset_id", assets ) ) )
            return status;
        ids.insert( assets.begin(), assets.end() );
        eitherFilters.push_back( ids );
    }
    TIdSet excludedPacks;
    if ( !query.excludeLicenceRestrictions.empty() )
    {
        if ( ASSETSTORE_STATUS_OK != ( status = SelectFacetObjects( "licenceRestriction", query.excludeLicenceRestrictions, false, excludedPacks ) ) )
            return status;
    }

    // the asset rows to work with
    TRowMap assets;
    CAssetCatalogue::TRowVector rows;
    if ( !words.empty() )
    {
        TIdSet hitAssets;
        TIdSet hitPacks;
        for ( size_t w=0; w<words.size(); ++w )
        {
            for ( TWeightMap::const_iterator a=assetWeights[ w ].begin(); a!=assetWeights[ w ].end(); ++a )
                hitAssets.insert( (*a).first );
            for ( TWeightMap::const_iterator p=packWeights[ w ].begin(); p!=packWeights[ w ].end(); ++p )
                hitPacks.insert( (*p).first );
        }
        if ( hasPackFilter )
        {
            TIdSet keep;
            for ( TIdSet::const_iterator p=hitPacks.begin(); p!=hitPacks.end(); ++p )
                if ( packFilter.end() != packFilter.find( (*p) ) )
                    keep.insert( (*p) );
            hitPacks = keep;
        }
        if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.SelectIn( "assets", "asset_id", hitAssets, rows ) ) )
            return status;
        CAssetCatalogue::TRowVector packAssets;
        if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.SelectIn( "assets", "pack_id", hitPacks, packAssets ) ) )
            return status;
        rows.insert( rows.end(), packAssets.begin(), packAssets.end() );
    }
    else
    if ( hasAssetFilter )
    {
        if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.SelectIn( "assets", "asset_id", assetFilter, rows ) ) )
            return status;
    }
    else
    if ( hasPackFilter )
    {
        if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.SelectIn( "assets", "pack_id", packFilter, rows ) ) )
            return status;
    }
    else
    {
        if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.SelectAll( "assets", rows ) ) )
            return status;
    }
    for ( size_t r=0; r<rows.size(); ++r )
        assets[ CAssetCatalogue::GetText( rows[ r ], "asset_id" ) ] = rows[ r ];

    // every filter on every asset, and the score of the keywords
    GUCEF::map< CORE::CString, Int64 > scores;
    TIdSet packIds;
    TRowMap::iterator a = assets.begin();
    while ( a != assets.end() )
    {
        const CORE::CString& assetId = (*a).first;
        const CORE::CString packId = CAssetCatalogue::GetText( (*a).second, "pack_id" );
        bool keep = true;
        if ( hasAssetFilter && assetFilter.end() == assetFilter.find( assetId ) )
            keep = false;
        if ( keep && hasPackFilter && packFilter.end() == packFilter.find( packId ) )
            keep = false;
        if ( keep && excludedPacks.end() != excludedPacks.find( packId ) )
            keep = false;
        for ( size_t e=0; keep && e<eitherFilters.size(); ++e )
            keep = eitherFilters[ e ].end() != eitherFilters[ e ].find( assetId ) || eitherFilters[ e ].end() != eitherFilters[ e ].find( packId );
        Int64 score = 0;
        for ( size_t w=0; keep && w<words.size(); ++w )
        {
            Int64 best = 0;
            TWeightMap::const_iterator aw = assetWeights[ w ].find( assetId );
            if ( aw != assetWeights[ w ].end() )
                best = (*aw).second;
            TWeightMap::const_iterator pw = packWeights[ w ].find( packId );
            if ( pw != packWeights[ w ].end() && (*pw).second > best )
                best = (*pw).second;
            keep = best > 0;
            score += best;
        }
        if ( !keep )
        {
            assets.erase( a++ );
            continue;
        }
        scores[ assetId ] = score;
        packIds.insert( packId );
        ++a;
    }

    // the packs of the assets
    TRowMap packs;
    if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.SelectIn( "packs", "pack_id", packIds, rows ) ) )
        return status;
    for ( size_t r=0; r<rows.size(); ++r )
    {
        const CORE::CString state = CAssetCatalogue::GetText( rows[ r ], "state" );
        const bool allowed = query.packStates.empty() ? AssetPackStateIsIngested( state ) : query.packStates.end() != query.packStates.find( state );
        if ( allowed )
            packs[ CAssetCatalogue::GetText( rows[ r ], "pack_id" ) ] = rows[ r ];
    }

    // the hits with their facets
    GUCEF::vector< SScoredHit > hits;
    GUCEF::map< CORE::CString, size_t > packHitIndex;
    const bool withFacets = query.withFacets;
    for ( a=assets.begin(); a!=assets.end(); ++a )
    {
        const CAssetCatalogue::TRow& asset = (*a).second;
        TRowMap::const_iterator p = packs.find( CAssetCatalogue::GetText( asset, "pack_id" ) );
        if ( p == packs.end() )
            continue;
        const CAssetCatalogue::TRow& pack = (*p).second;
        const CORE::CString locationId = CAssetCatalogue::GetText( pack, "location_id" );
        TLocationStateMap::const_iterator location = locations.find( locationId );
        const CORE::CString locationRoot = location == locations.end() ? CORE::CString::Empty : (*location).second.vfsRoot;
        const bool online = location != locations.end() && (*location).second.online;
        const CPackStorageLayout layout( CAssetCatalogue::GetText( pack, "root_path" ) );

        if ( packsMode )
        {
            GUCEF::map< CORE::CString, size_t >::iterator existing = packHitIndex.find( (*p).first );
            if ( existing != packHitIndex.end() )
            {
                SScoredHit& packHit = hits[ (*existing).second ];
                if ( scores[ (*a).first ] > packHit.hit.score )
                    packHit.hit.score = scores[ (*a).first ];
                continue;
            }
            SScoredHit scored;
            scored.hit.packId = (*p).first;
            scored.hit.name = CAssetCatalogue::GetText( pack, "title" );
            scored.hit.packTitle = scored.hit.name;
            scored.hit.sizeBytes = CAssetCatalogue::GetInt( pack, "size_bytes" );
            scored.hit.ingestedAtMs = CAssetCatalogue::GetInt( pack, "ingested_ms" );
            const CORE::CString::StringSet categories = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( pack, "categories" ) );
            scored.hit.categoryId = categories.empty() ? CORE::CString::Empty : *categories.begin();
            scored.hit.vfsPath = layout.GetBrowseFolder( locationRoot );
            scored.hit.score = scores[ (*a).first ];
            scored.hit.sourceId = CAssetCatalogue::GetText( pack, "source_id" );
            scored.hit.seller = CAssetCatalogue::GetText( pack, "seller" );
            scored.hit.licenceClass = CAssetCatalogue::GetText( pack, "licence_class" );
            scored.hit.locationId = locationId;
            scored.hit.available = online;
            scored.sortValue = 0;
            packHitIndex[ (*p).first ] = hits.size();
            hits.push_back( scored );
            if ( withFacets )
            {
                const CORE::CString::StringSet kinds = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( pack, "kinds" ) );
                for ( CORE::CString::StringSet::const_iterator k=kinds.begin(); k!=kinds.end(); ++k )
                    ++page.facets[ "kind" ][ (*k) ];
                CORE::CString::StringSet tops;
                for ( CORE::CString::StringSet::const_iterator c=categories.begin(); c!=categories.end(); ++c )
                    tops.insert( TopLevelOf( (*c) ) );
                for ( CORE::CString::StringSet::const_iterator t=tops.begin(); t!=tops.end(); ++t )
                    ++page.facets[ "category" ][ (*t) ];
                ++page.facets[ "source" ][ scored.hit.sourceId ];
                ++page.facets[ "licenceClass" ][ scored.hit.licenceClass ];
            }
            continue;
        }

        SScoredHit scored;
        scored.hit.assetId = (*a).first;
        scored.hit.packId = (*p).first;
        scored.hit.name = CAssetCatalogue::GetText( asset, "name" );
        scored.hit.kind = CAssetCatalogue::GetText( asset, "kind" );
        scored.hit.categoryId = CAssetCatalogue::GetText( asset, "category_id" );
        scored.hit.sourceId = CAssetCatalogue::GetText( pack, "source_id" );
        scored.hit.seller = CAssetCatalogue::GetText( pack, "seller" );
        scored.hit.packTitle = CAssetCatalogue::GetText( pack, "title" );
        scored.hit.licenceClass = CAssetCatalogue::GetText( pack, "licence_class" );
        scored.hit.locationId = locationId;
        scored.hit.vfsPath = layout.Resolve( locationRoot, CAssetCatalogue::GetText( asset, "original_path" ) );
        scored.hit.available = online;
        scored.hit.score = scores[ (*a).first ];
        scored.hit.sizeBytes = CAssetCatalogue::GetInt( asset, "size_bytes" );
        scored.hit.ingestedAtMs = CAssetCatalogue::GetInt( pack, "ingested_ms" );
        scored.sortValue = 0;
        hits.push_back( scored );
        if ( withFacets )
        {
            ++page.facets[ "kind" ][ scored.hit.kind ];
            ++page.facets[ "category" ][ TopLevelOf( scored.hit.categoryId ) ];
            ++page.facets[ "source" ][ scored.hit.sourceId ];
            ++page.facets[ "licenceClass" ][ scored.hit.licenceClass ];
        }
    }

    // the order
    const CORE::CString sort = query.sort.Lowercase();
    if ( sort == "size" || sort == "newest" || sort == "triangles" )
    {
        GUCEF::map< CORE::CString, Int64 > triangles;
        if ( sort == "triangles" && !packsMode )
        {
            TIdSet ids;
            for ( size_t h=0; h<hits.size(); ++h )
                ids.insert( hits[ h ].hit.assetId );
            CAssetCatalogue::TRowVector modelRows;
            if ( ASSETSTORE_STATUS_OK == m_catalogue.SelectIn( "asset_model_props", "asset_id", ids, modelRows ) )
                for ( size_t r=0; r<modelRows.size(); ++r )
                    triangles[ CAssetCatalogue::GetText( modelRows[ r ], "asset_id" ) ] = CAssetCatalogue::GetInt( modelRows[ r ], "triangles" );
        }
        for ( size_t h=0; h<hits.size(); ++h )
        {
            if ( sort == "newest" )
                hits[ h ].sortValue = hits[ h ].hit.ingestedAtMs;
            else if ( sort == "triangles" && !packsMode )
                hits[ h ].sortValue = triangles[ hits[ h ].hit.assetId ];
            else
                hits[ h ].sortValue = hits[ h ].hit.sizeBytes;
        }
        std::sort( hits.begin(), hits.end(), SByValue() );
    }
    else
    if ( sort == "name" || words.empty() )
        std::sort( hits.begin(), hits.end(), SByName() );
    else
        std::sort( hits.begin(), hits.end(), SByRelevance() );

    // the page
    page.total = hits.size();
    const UInt64 first = page.pageIndex * page.pageSize;
    for ( UInt64 h=first; h<hits.size() && h<first+page.pageSize; ++h )
        page.hits.push_back( hits[ (size_t) h ].hit );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSearchEngine::Suggest( const CORE::CString& prefix, UInt32 maxCount, CORE::CString::StringVector& keywords ) const
{GUCEF_TRACE;

    keywords.clear();
    const CORE::CString folded = CAssetKeywordNormalizer::Fold( prefix );
    if ( folded.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_OK;
    CORE::CDataTableQuery query;
    query.WhereRange( "keyword", CORE::CVariant( folded ), true, CORE::CVariant( PrefixUpperBound( folded ) ), false );
    query.Select( "keyword" );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "keywords", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    GUCEF::map< CORE::CString, Int64 > counts;
    for ( size_t r=0; r<rows.size(); ++r )
        ++counts[ CAssetCatalogue::GetText( rows[ r ], "keyword" ) ];
    GUCEF::vector< std::pair< Int64, CORE::CString > > ordered;
    for ( GUCEF::map< CORE::CString, Int64 >::const_iterator c=counts.begin(); c!=counts.end(); ++c )
        ordered.push_back( std::pair< Int64, CORE::CString >( -(*c).second, (*c).first ) );
    std::sort( ordered.begin(), ordered.end() );
    for ( size_t o=0; o<ordered.size() && o<maxCount; ++o )
        keywords.push_back( ordered[ o ].second );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
