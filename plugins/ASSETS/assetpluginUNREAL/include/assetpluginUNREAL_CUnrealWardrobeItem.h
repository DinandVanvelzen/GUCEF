/*
 *  assetpluginUNREAL: a wardrobe item of a MetaHuman character: the outfit it puts on and its hidden face map
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALWARDROBEITEM_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALWARDROBEITEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A wardrobe item (class MetaHumanWardrobeItem): the asset it puts on the character (the principal asset, an outfit) and the
 *  settings of its editor pipeline (a subobject of the item). The pipeline of an outfit names the hidden face map of the body
 *  (UE 5.8: property BodyHiddenFaceMapTexture, a struct with the texture and optional settings MaxCullValue, MinKeepValue and
 *  MaxShrinkDistance; UE 5.6: property BodyHiddenFaceMap, the texture). Settings that the item does not store keep the defaults
 *  of CUnrealHiddenFaceMap.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealWardrobeItem
{
    public:

    CORE::CString name;                     /**< the object name of the item, for example WI_Outfit */
    CORE::CString packageName;
    CORE::CString principalAsset;           /**< the object path of what the item puts on */
    CORE::CString bodyHiddenFaceMap;        /**< the object path of the texture, empty when the item has none */
    bool hasMaxCullValue;
    Float32 maxCullValue;
    bool hasMinKeepValue;
    Float32 minKeepValue;
    bool hasMaxShrinkDistance;
    Float32 maxShrinkDistance;              /**< centimetres */

    CUnrealWardrobeItem( void );

    /** Reads the item of the export and the properties of the pipelines below it */
    bool Read( CUnrealPackage& package, UInt32 exportIndex );

    const CORE::CString& GetError( void ) const;

    private:

    CORE::CString m_error;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALWARDROBEITEM_H ? */
