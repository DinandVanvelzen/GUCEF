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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H
#define GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CIASSETMETADATANORMALIZER_H
#include "gucefASSETSTORE_CIAssetMetadataNormalizer.h"
#define GUCEF_ASSETSTORE_CIASSETMETADATANORMALIZER_H
#endif /* GUCEF_ASSETSTORE_CIASSETMETADATANORMALIZER_H ? */
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

/**
 *  The generic metadata normaliser of store items (section 7.2): title, seller, description, address and version
 *  as the store gives them; every store category through the category map of the store (and its parents), the ones
 *  without a mapping listed as unmapped; the licence name, address, text and the restriction flags the store states,
 *  classified by the licence map; keywords from the title, the tags, the store categories, the seller and the
 *  description; the category rules on the keywords and the names when no store category mapped, and their themes and
 *  art styles always. Store plugins derive from it to change a step.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreItemNormalizer : public CIAssetMetadataNormalizer
{
    public:

    CAssetStoreItemNormalizer( void );
    virtual ~CAssetStoreItemNormalizer() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TAssetStoreStatus Normalize( const CAssetNormalizationInput& input, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack ) GUCEF_VIRTUAL_OVERRIDE;

    /** The licence as MapLicence() gives it to a pack, so a normaliser of a store that overrides MapLicence() plans with its own rules */
    virtual void ClassifyItemLicence( const CAssetStoreItem& item, const CAssetVocabulary& vocabulary, CAssetLicence& licence ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** The fields of the item: title, seller, description, address, version, store categories and tags */
    virtual void MapFields( const CAssetStoreItem& item, CAssetPackRecord& pack );

    /** The store categories through the category map; true when one of them mapped onto a category */
    virtual bool MapCategories( const CORE::CString& sourceId, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack );

    /** The licence of the item, classified */
    virtual void MapLicence( const CAssetStoreItem& item, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack );

    /** The keywords of the fields of the pack */
    virtual void MakeKeywords( const CAssetVocabulary& vocabulary, CAssetPackRecord& pack );

    /** The category rules on the keywords and the names; the category only when the pack has none yet */
    virtual void ApplyRules( const CAssetNormalizationInput& input, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H ? */
