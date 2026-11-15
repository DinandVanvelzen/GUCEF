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

#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#include "gucefASSETSTORE_CAssetVocabulary.h"
#define GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARY_H ? */
#include "gucefASSETSTORE_CAssetStoreItemNormalizer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreItemNormalizer::CAssetStoreItemNormalizer( void )
    : CIAssetMetadataNormalizer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreItemNormalizer::~CAssetStoreItemNormalizer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreItemNormalizer::GetName( void ) const
{GUCEF_TRACE;

    return "storeItem";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreItemNormalizer::GetVersion( void ) const
{GUCEF_TRACE;

    return "1";
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreItemNormalizer::MapFields( const CAssetStoreItem& item, CAssetPackRecord& pack )
{GUCEF_TRACE;

    pack.sourceItemId = item.itemId;
    pack.sourceVersion = item.version;
    pack.sourceUrl = item.url;
    pack.title = item.title.Trim( true ).Trim( false );
    pack.seller = item.seller.Trim( true ).Trim( false );
    pack.description = item.description;
    pack.sourceCategories = item.categories;
    pack.sourceTags = item.tags;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreItemNormalizer::MapCategories( const CORE::CString& sourceId, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack )
{GUCEF_TRACE;

    bool mapped = false;
    for ( size_t c=0; c<pack.sourceCategories.size(); ++c )
    {
        CAssetClassification classification;
        if ( vocabulary.MapStoreCategory( sourceId, pack.sourceCategories[ c ], classification ) )
        {
            pack.categories.insert( classification.categories.begin(), classification.categories.end() );
            pack.themes.insert( classification.themes.begin(), classification.themes.end() );
            if ( !classification.kind.IsNULLOrEmpty() )
                pack.kinds.insert( classification.kind );
            mapped = mapped || !classification.categories.empty();
            if ( classification.categories.empty() && classification.themes.empty() && classification.kind.IsNULLOrEmpty() )
                pack.unmappedCategories.insert( pack.sourceCategories[ c ] );
        }
        else
        {
            pack.unmappedCategories.insert( pack.sourceCategories[ c ] );
        }
    }
    return mapped;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreItemNormalizer::MapLicence( const CAssetStoreItem& item, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack )
{GUCEF_TRACE;

    pack.licence.name = item.licenceName;
    pack.licence.url = item.licenceUrl;
    pack.licence.text = item.licenceText;
    pack.licence.restrictions.insert( item.licenceRestrictions.begin(), item.licenceRestrictions.end() );
    vocabulary.ClassifyLicence( pack.licence );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreItemNormalizer::MakeKeywords( const CAssetVocabulary& vocabulary, CAssetPackRecord& pack )
{GUCEF_TRACE;

    const CAssetKeywordNormalizer& normalizer = vocabulary.GetKeywordNormalizer();
    normalizer.Normalize( pack.title, "title", TitleWeight, pack.keywords );
    for ( size_t t=0; t<pack.sourceTags.size(); ++t )
        normalizer.Normalize( pack.sourceTags[ t ], "tag", TagWeight, pack.keywords );
    for ( size_t c=0; c<pack.sourceCategories.size(); ++c )
        normalizer.Normalize( pack.sourceCategories[ c ], "category", CategoryWeight, pack.keywords );
    normalizer.Normalize( pack.seller, "seller", SellerWeight, pack.keywords );
    normalizer.Normalize( pack.description, "description", DescriptionWeight, pack.keywords );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreItemNormalizer::ApplyRules( const CAssetNormalizationInput& input, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack )
{GUCEF_TRACE;

    // the rules look at what the pack is called and what it describes, not at its description
    CORE::CString::StringSet keywords;
    for ( size_t k=0; k<pack.keywords.size(); ++k )
        if ( pack.keywords[ k ].field != "description" )
            keywords.insert( pack.keywords[ k ].keyword );
    CORE::CString::StringVector names;
    names.push_back( pack.title );
    if ( !input.packName.IsNULLOrEmpty() )
        names.push_back( input.packName );
    CAssetClassification classification;
    if ( !vocabulary.ApplyRules( keywords, names, pack.seller, classification ) )
        return;
    if ( pack.categories.empty() )
        pack.categories.insert( classification.categories.begin(), classification.categories.end() );
    pack.themes.insert( classification.themes.begin(), classification.themes.end() );
    pack.artStyles.insert( classification.artStyles.begin(), classification.artStyles.end() );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreItemNormalizer::Normalize( const CAssetNormalizationInput& input, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack )
{GUCEF_TRACE;

    MapFields( input.item, pack );
    MapCategories( input.sourceId, vocabulary, pack );
    MapLicence( input.item, vocabulary, pack );
    MakeKeywords( vocabulary, pack );
    ApplyRules( input, vocabulary, pack );
    pack.normalizer = GetName() + "/" + GetVersion();
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
