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

#ifndef GUCEF_ASSETSTORE_CIASSETMETADATANORMALIZER_H
#define GUCEF_ASSETSTORE_CIASSETMETADATANORMALIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETNORMALIZATIONINPUT_H
#include "gucefASSETSTORE_CAssetNormalizationInput.h"
#define GUCEF_ASSETSTORE_CASSETNORMALIZATIONINPUT_H
#endif /* GUCEF_ASSETSTORE_CASSETNORMALIZATIONINPUT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */
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
class CAssetStoreItem;

/**
 *  Maps the metadata of a pack onto the pack record of the primary store (section 7.2): its categories onto the
 *  category tree through the mapping table (unmapped categories are kept and listed), its licence onto a licence
 *  class with restriction flags, its title, tags, categories and description onto keywords, and themes and art
 *  styles. A store plugin gives its own through CIAssetStoreProvider::GetMetadataNormalizer() where the generic
 *  CAssetStoreItemNormalizer does not fit; packs from disk use CAssetDiskPackNormalizer.
 *
 *  Normalize() runs on the thread of the ingest and fills the fields of the pack it knows; the pack id, the source,
 *  the location and the state are set by the pipeline.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetMetadataNormalizer
{
    public:

    /** The keyword weights of the fields, for the ranking of searches (section 9.4) */
    static const Int64 TitleWeight = 10;
    static const Int64 NameWeight = 6;
    static const Int64 TagWeight = 5;
    static const Int64 CategoryWeight = 4;
    static const Int64 SellerWeight = 3;
    static const Int64 DescriptionWeight = 1;
    static const Int64 DerivedWeight = 1;

    CIAssetMetadataNormalizer( void );
    virtual ~CIAssetMetadataNormalizer();

    virtual CORE::CString GetName( void ) const = 0;

    virtual CORE::CString GetVersion( void ) const = 0;

    virtual TAssetStoreStatus Normalize( const CAssetNormalizationInput& input, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack ) = 0;

    /**
     *  The licence of a listed item classified as the ingest of its pack would classify it, without a pack: the library sync
     *  plans with it (the licence filter, "needs-licence-review"). This default gives the name, address, text and restrictions
     *  of the item to the vocabulary; a normaliser with licence rules of its store gives the class those rules give
     */
    virtual void ClassifyItemLicence( const CAssetStoreItem& item, const CAssetVocabulary& vocabulary, CAssetLicence& licence );

    private:

    CIAssetMetadataNormalizer( const CIAssetMetadataNormalizer& src );              /**< not implemented */
    CIAssetMetadataNormalizer& operator=( const CIAssetMetadataNormalizer& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETMETADATANORMALIZER_H ? */
