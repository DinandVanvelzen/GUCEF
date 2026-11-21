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

#ifndef GUCEF_ASSETSTORE_CASSETFORMATPOLICY_H
#define GUCEF_ASSETSTORE_CASSETFORMATPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */

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
 *  The order in which the formats of an item are downloaded by the library sync (library sync design, section 4.3, with
 *  the decision of the owner that every format of an item is downloaded): the formats in the preference list first, in the
 *  order of the list, then the formats outside of it in the order of the listing, the exports of game engines last. With
 *  onlyPreferred the formats outside the list are left out. A format matches an entry of the list by its id or its name,
 *  in any letter case. The default list is that of the workbench: gltf, glb, fbx, obj, usd.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetFormatPolicy : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString::StringVector preference;     /**< the formats in the order they are wanted */
    bool onlyPreferred;                         /**< leave out the formats that are not in the list */

    CAssetFormatPolicy( void );
    virtual ~CAssetFormatPolicy() GUCEF_VIRTUAL_OVERRIDE;

    /** gltf, glb, fbx, obj, usd */
    static CORE::CString::StringVector GetDefaultPreference( void );

    /** Whether a format is an export of a game engine (Unreal Engine and the like), which is wanted last */
    static bool IsEngineExport( const CAssetStoreFormat& format );

    /** The ids of the formats of the item in the order they are downloaded; with onlyPreferred only those of the list */
    void RankFormats( const CAssetStoreItem& item, CORE::CString::StringVector& formatIds ) const;

    /** Whether the format is in the preference list */
    bool IsPreferred( const CAssetStoreFormat& format ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    UInt32 RankOf( const CAssetStoreFormat& format ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETFORMATPOLICY_H ? */
