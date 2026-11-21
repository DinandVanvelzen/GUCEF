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

#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLICENCE_H
#include "gucefASSETSTORE_CAssetLicence.h"
#define GUCEF_ASSETSTORE_CASSETLICENCE_H
#endif /* GUCEF_ASSETSTORE_CASSETLICENCE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETRECORD_H
#include "gucefASSETSTORE_CAssetRecord.h"
#define GUCEF_ASSETSTORE_CASSETRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H
#include "gucefASSETSTORE_CAssetConversionRecord.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H ? */
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
 *  The normalised record of a pack (section 7.1): where it came from, what it is in the vocabulary of the primary
 *  store (categories, themes, art styles, keywords), its licence with its restriction flags, the formats it holds,
 *  where it is stored, its sizes and the state of its ingest. The categories and tags as the source had them are kept;
 *  source categories that the mapping does not know are listed in 'unmappedCategories' for the owner to map.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPackRecord : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString packId;
    CORE::CString sourceId;                         /**< the store id, or "disk" */
    CORE::CString sourceItemId;
    CORE::CString sourceVersion;
    CORE::CString sourceUrl;
    CORE::CString title;
    CORE::CString seller;
    CORE::CString description;                      /**< plain text */
    CORE::CString::StringVector sourceCategories;
    CORE::CString::StringVector sourceTags;
    CORE::CString::StringSet categories;            /**< category ids of the vocabulary */
    CORE::CString::StringSet unmappedCategories;    /**< source categories without a mapping */
    CORE::CString::StringSet kinds;                 /**< the kinds of its assets */
    CORE::CString::StringSet themes;
    CORE::CString::StringSet artStyles;
    CORE::CString::StringSet pbrRoles;              /**< the texture roles its textures have */
    TAssetKeywordVector keywords;                   /**< from the title, the tags, the categories and the description */
    CAssetLicence licence;
    CORE::CString::StringSet formatsOriginal;       /**< format ids */
    CORE::CString::StringSet formatsConverted;
    TAssetKeepPolicy keep;
    CORE::CString locationId;
    CORE::CString rootPath;                         /**< layout 2: "layout2:<pack folder>"; layout 1: a folder relative to the root of the location; see CPackStorageLayout */
    Int64 sizeBytes;
    Int64 fileCount;
    Int64 assetCount;
    Int64 ingestedAtMs;                             /**< UTC milliseconds since the Unix epoch */
    CORE::CString normalizer;                       /**< "name/version" */
    CORE::CString::StringSet analyzers;             /**< "name/version" */
    CAssetConversionRecord conversion;              /**< what the last conversion did (empty when the pack was never converted) */
    TAssetPackState state;
    CORE::CString stateReason;

    CAssetPackRecord( void );
    virtual ~CAssetPackRecord() GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */
