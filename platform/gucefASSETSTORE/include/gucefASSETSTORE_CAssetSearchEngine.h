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

#ifndef GUCEF_ASSETSTORE_CASSETSEARCHENGINE_H
#define GUCEF_ASSETSTORE_CASSETSEARCHENGINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#include "gucefASSETSTORE_CAssetCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#include "gucefASSETSTORE_CAssetSearchQuery.h"
#define GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#endif /* GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSEARCHRESULTPAGE_H
#include "gucefASSETSTORE_CAssetSearchResultPage.h"
#define GUCEF_ASSETSTORE_CASSETSEARCHRESULTPAGE_H
#endif /* GUCEF_ASSETSTORE_CASSETSEARCHRESULTPAGE_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAssetVocabulary;

/**
 *  The search of the primary store (section 9.4), portable to every database backend:
 *    - keywords: the text is normalised like the keywords; per word an EQUALS on keywords.keyword, or a prefix RANGE
 *      for the last word when the query asks for it; a keyword of a pack counts for every asset of the pack. The assets
 *      that match every word remain; their score is the sum over the words of the highest weight of a matching field
 *    - typed filters: predicates on the tables assets, packs, facets and the typed property tables, pushed to the
 *      backend; the asset ids and pack ids they give are intersected with the keyword result in memory
 *    - facets: counts per kind, top level category, source and licence class of the whole result
 *    - paging after sorting
 *  Packs that are staged, being indexed or failed are left out unless the query names pack states. Native full text
 *  search of a backend can replace the keyword part behind the same interface later.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSearchEngine
{
    public:

    struct SLocationState
    {
        CORE::CString vfsRoot;
        bool online;
    };
    typedef GUCEF::map< CORE::CString, SLocationState > TLocationStateMap;

    CAssetSearchEngine( const CAssetCatalogue& catalogue, const CAssetVocabulary& vocabulary );

    TAssetStoreStatus Search( const CAssetSearchQuery& query, const TLocationStateMap& locations, CAssetSearchResultPage& page ) const;

    /** Keywords that start with the prefix (normalised), the most used first, at most 'maxCount' */
    TAssetStoreStatus Suggest( const CORE::CString& prefix, UInt32 maxCount, CORE::CString::StringVector& keywords ) const;

    /** The upper bound of a prefix RANGE on text: the prefix followed by the highest code point */
    static CORE::CString PrefixUpperBound( const CORE::CString& prefix );

    private:

    typedef CORE::CString::StringSet TIdSet;
    typedef GUCEF::map< CORE::CString, Int64 > TWeightMap;
    typedef GUCEF::map< CORE::CString, CAssetCatalogue::TRow > TRowMap;

    TAssetStoreStatus FindKeyword( const CORE::CString& word, bool prefix, TWeightMap& assetWeights, TWeightMap& packWeights, CORE::CString::StringMap& assetPacks ) const;
    TAssetStoreStatus SelectIds( const CORE::CString& table, const CORE::CDataTableQuery& query, const CORE::CString& idColumn, TIdSet& ids ) const;
    TAssetStoreStatus SelectFacetObjects( const CORE::CString& facet, const CORE::CString::StringSet& values, bool all, TIdSet& objects ) const;

    static void Intersect( TIdSet& target, const TIdSet& other, bool& hasTarget );

    CAssetSearchEngine( const CAssetSearchEngine& src );              /**< not implemented */
    CAssetSearchEngine& operator=( const CAssetSearchEngine& src );   /**< not implemented */

    private:

    const CAssetCatalogue& m_catalogue;
    const CAssetVocabulary& m_vocabulary;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSEARCHENGINE_H ? */
