/*
 *  AssetConverterLib: the function groups of the pack index and which asset kinds fill them
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

#ifndef ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H
#define ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The stable ids, display names and the fixed order of the function groups of F17 (the first level of the resource view).
 *  The ids are part of the document format (schema version 1): they never change meaning.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackGroupVocabulary
{
    public:

    static const CORE::CString Scenes;
    static const CORE::CString Characters;
    static const CORE::CString StaticModels;
    static const CORE::CString Animations;
    static const CORE::CString VisualEffects;
    static const CORE::CString Materials;
    static const CORE::CString Textures;
    static const CORE::CString Audio;
    static const CORE::CString Prefabs;
    static const CORE::CString Shaders;
    static const CORE::CString Fonts;
    static const CORE::CString Packages;
    static const CORE::CString Other;

    /** The number of groups */
    static UInt32 GetGroupCount( void );

    /** The id of the group at the position of the fixed order */
    static const CORE::CString& GetGroupIdAt( UInt32 index );

    /** The position of the group in the fixed order; the count (behind all) for an id that is not known */
    static UInt32 GetGroupOrder( const CORE::CString& groupId );

    static bool IsKnownGroup( const CORE::CString& groupId );

    /** The text the views show for the group */
    static CORE::CString GetGroupName( const CORE::CString& groupId );

    /** The default group of an asset kind (stage K); empty for the kinds that are no resources (folders, metadata) */
    static const CORE::CString& GetGroupOfKind( EAssetKind kind );

    private:

    CPackGroupVocabulary( void );
    ~CPackGroupVocabulary();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H ? */

