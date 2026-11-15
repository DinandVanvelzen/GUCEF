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

#ifndef GUCEF_ASSETSTORE_CASSETSEARCHRESULTPAGE_H
#define GUCEF_ASSETSTORE_CASSETSEARCHRESULTPAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSEARCHHIT_H
#include "gucefASSETSTORE_CAssetSearchHit.h"
#define GUCEF_ASSETSTORE_CASSETSEARCHHIT_H
#endif /* GUCEF_ASSETSTORE_CASSETSEARCHHIT_H ? */
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

typedef GUCEF::vector< CAssetSearchHit > TAssetSearchHitVector;

/**
 *  A page of the results of a search (section 9.4): the hits of the page, the number of results over all pages and,
 *  when the query asked for them, the facets of the whole result: counts per "kind", "category" (top level), "source"
 *  and "licenceClass". The result data of CPrimaryAssetStore::SearchAsync().
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSearchResultPage : public CORE::CIDataNodeSerializable
{
    public:

    typedef GUCEF::map< CORE::CString, UInt64 > TValueCountMap;
    typedef GUCEF::map< CORE::CString, TValueCountMap > TFacetMap;

    TAssetSearchHitVector hits;
    UInt64 total;
    UInt64 pageIndex;
    UInt64 pageSize;
    UInt64 revision;                             /**< the catalogue revision the page was read at (CIAssetStoreService), 0 when not known */
    TFacetMap facets;

    CAssetSearchResultPage( void );
    virtual ~CAssetSearchResultPage() GUCEF_VIRTUAL_OVERRIDE;

    void Clear( void );

    /** The count of a value of a facet, 0 when it has none */
    UInt64 GetFacetCount( const CORE::CString& facet, const CORE::CString& value ) const;

    /** Whether the page has a hit of the asset (or of the pack, for pack results) */
    bool HasHit( const CORE::CString& assetOrPackId ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSEARCHRESULTPAGE_H ? */
