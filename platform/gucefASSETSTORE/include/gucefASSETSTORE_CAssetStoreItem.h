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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREFORMAT_H
#include "gucefASSETSTORE_CAssetStoreFormat.h"
#define GUCEF_ASSETSTORE_CASSETSTOREFORMAT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREFORMAT_H ? */
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
 *  An item of a store as the store describes it (section 4.2): what a library listing and the item details give.
 *  Categories and tags are kept as the store has them; the primary store maps them during the ingest (section 7.2).
 *  Attributes that only the store knows go into 'storeAttributes'.
 *
 *  Serialized as a DataNode object with the members by their names; "formats" is an array of format objects,
 *  "categories", "tags", "imageUrls" and "licenceRestrictions" are arrays of text, "storeAttributes" an object.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreItem : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString providerId;                   /**< the store the item comes from */
    CORE::CString itemId;
    CORE::CString version;                      /**< the version of the item when the store has one */
    CORE::CString title;
    CORE::CString seller;
    CORE::CString listingType;                  /**< as the store names it: "3d-model", "material", "audio", ... */
    CORE::CString url;                          /**< the page of the item */
    CORE::CString thumbnailUrl;
    CORE::CString::StringVector imageUrls;
    CORE::CString description;
    CORE::CString licenceName;
    CORE::CString licenceUrl;
    CORE::CString licenceText;                  /**< when the store gives it */
    CORE::CString::StringSet licenceRestrictions; /**< restriction flags the store states, for example "noMachineLearning" */
    CORE::CString::StringVector categories;     /**< as the store has them, a path per entry: "3D/Weapons & Combat/Melee" */
    CORE::CString::StringVector tags;
    TAssetStoreFormatVector formats;
    CORE::CDataNode storeAttributes;

    CAssetStoreItem( void );
    virtual ~CAssetStoreItem() GUCEF_VIRTUAL_OVERRIDE;

    /** The format with the id, GUCEF_NULL when there is none */
    const CAssetStoreFormat* FindFormat( const CORE::CString& formatId ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef GUCEF::vector< CAssetStoreItem > TAssetStoreItemVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */
