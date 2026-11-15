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

#ifndef GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#define GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
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
 *  A search of the primary store (section 9.4): keywords and typed filters, combined. The text is normalised as
 *  keywords are; every word must match (an EQUALS on the keyword index, a prefix RANGE for the last word when
 *  prefixLastWord is set). Filters on sets match any of their values; the categories include their sub categories.
 *  Bounds of -1 are not set; flags of -1 do not filter, 0 asks for false and 1 for true.
 *
 *  Typed property filters work per family: the model filters (triangles, vertices, rigged, animated, LODs,
 *  collision), the texture filter (resolution: the larger side), the animation filter (length) and the PBR roles (of
 *  the textures of a pack). An asset passes when it passes the filters of its own family; with filters of several
 *  families the asset sets of the families are joined, so "triangles <= 2000 and textures <= 1024" gives the light
 *  models and the small textures.
 *
 *  The licence restriction filters work on the restriction flags of the licences of the packs: required flags must
 *  all be there, excluded flags must all be absent ("licence allows machine learning use: no" requires
 *  noMachineLearning; leaving such packs out excludes it). resultKind "assets" gives assets, "packs" gives packs.
 *  Sorts: "relevance" (the default; without text the name), "name", "size", "triangles", "newest".
 *  Serialized as a DataNode object with the members by their names: the form of the saved searches.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSearchQuery : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString text;
    bool prefixLastWord;
    CORE::CString::StringSet kinds;
    CORE::CString::StringSet categories;         /**< with their sub categories */
    CORE::CString::StringSet themes;
    CORE::CString::StringSet artStyles;
    CORE::CString::StringSet sources;            /**< store ids, "disk" */
    CORE::CString::StringSet sellers;
    CORE::CString::StringSet licenceClasses;
    CORE::CString::StringSet requireLicenceRestrictions;
    CORE::CString::StringSet excludeLicenceRestrictions;
    CORE::CString::StringSet formats;            /**< any format held, of the asset or its pack */
    CORE::CString::StringSet packFormatsOnly;    /**< packs whose formats are all among these */
    Int64 minAssetBytes;
    Int64 maxAssetBytes;
    Int64 minPackBytes;
    Int64 maxPackBytes;
    Int64 minTriangles;
    Int64 maxTriangles;
    Int64 minVertices;
    Int64 maxVertices;
    Int64 minTextureResolution;
    Int64 maxTextureResolution;
    Float64 minAnimationSeconds;
    Float64 maxAnimationSeconds;
    Int64 rigged;
    Int64 animated;
    Int64 hasLods;
    Int64 hasCollision;
    CORE::CString::StringSet pbrRoles;           /**< texture roles a pack has */
    CORE::CString::StringSet packStates;
    bool unclassified;                           /**< packs and assets without a category */
    CORE::CString::StringSet packIds;            /**< only these packs */
    CORE::CString resultKind;
    CORE::CString sort;
    Int64 pageIndex;
    Int64 pageSize;
    bool withFacets;

    CAssetSearchQuery( void );
    virtual ~CAssetSearchQuery() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H ? */
