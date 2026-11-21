/*
 *  AssetWorkbenchLib: the asset sources of the workbench: online stores, the primary store and the transfers
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
#ifndef ASSETWORKBENCHLIB_CASSETSTOREINSPECTORDATA_H
#define ASSETWORKBENCHLIB_CASSETSTOREINSPECTORDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETRECORD_H
#include "gucefASSETSTORE_CAssetRecord.h"
#define GUCEF_ASSETSTORE_CASSETRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRECORD_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLICENCE_H
#include "gucefASSETSTORE_CAssetLicence.h"
#define GUCEF_ASSETSTORE_CASSETLICENCE_H
#endif /* GUCEF_ASSETSTORE_CASSETLICENCE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/


/**
 *  What the property inspector shows of a pack, an asset or an item of a store: trees of CORE::CDataNode with attributes in
 *  the plain words of the person, all read only. The licence is shown with its restriction flags as statements ("Machine
 *  learning: not allowed"), the formats and files of a store item with their sizes.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetStoreInspectorData
{
    public:

    /** The pack record: where it came from, what it is in the vocabulary, its sizes, its state and the formats it holds; the licence is a child "Licence" */
    static void FillPack( const ASSETSTORE::CAssetPackRecord& pack, CORE::CDataNode& node );

    /** An asset with its typed properties (model, texture, animation and audio, as far as they are known) and its files */
    static void FillAsset( const ASSETSTORE::CAssetRecord& asset, CORE::CDataNode& node );

    /** The licence as a child "Licence" of the node: class, name, address and the restrictions in words */
    static void FillLicence( const ASSETSTORE::CAssetLicence& licence, CORE::CDataNode& node );

    /** An item of an online store: title, seller, the licence with the restrictions it states, the formats (and the files of the format with the id) */
    static void FillStoreItem( const ASSETSTORE::CAssetStoreItem& item, const CORE::CString& formatId, CORE::CDataNode& node );

    /** The words of a restriction flag of a licence: "Machine learning", "not allowed" */
    static void DescribeRestriction( const CORE::CString& flag, CORE::CString& subject, CORE::CString& statement );

    /** The words for the restrictions of a licence in one text, "none" without any */
    static CORE::CString DescribeRestrictions( const CORE::CString::StringSet& flags );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETSTOREINSPECTORDATA_H ? */
