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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "AssetConverterLib_CPackGroupVocabulary.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPackGroupVocabulary::Scenes = "scenes";
const CORE::CString CPackGroupVocabulary::Characters = "characters";
const CORE::CString CPackGroupVocabulary::StaticModels = "static-models";
const CORE::CString CPackGroupVocabulary::Animations = "animations";
const CORE::CString CPackGroupVocabulary::VisualEffects = "visual-effects";
const CORE::CString CPackGroupVocabulary::Materials = "materials";
const CORE::CString CPackGroupVocabulary::Textures = "textures";
const CORE::CString CPackGroupVocabulary::Audio = "audio";
const CORE::CString CPackGroupVocabulary::Prefabs = "prefabs";
const CORE::CString CPackGroupVocabulary::Shaders = "shaders";
const CORE::CString CPackGroupVocabulary::Fonts = "fonts";
const CORE::CString CPackGroupVocabulary::Packages = "packages";
const CORE::CString CPackGroupVocabulary::Other = "other";

/*-------------------------------------------------------------------------*/

namespace {

struct SGroupEntry
{
    const char* id;
    const char* name;
};

const SGroupEntry GroupTable[] =
{
    { "scenes", "Scenes" }, { "characters", "Characters" }, { "static-models", "Static models" }, { "animations", "Animations" },
    { "visual-effects", "Visual effects" }, { "materials", "Materials" }, { "textures", "Textures and images" }, { "audio", "Audio" },
    { "prefabs", "Prefabs" }, { "shaders", "Shaders" }, { "fonts", "Fonts" }, { "packages", "Package assets" }, { "other", "Other" }
};

const UInt32 GroupTableSize = sizeof( GroupTable ) / sizeof( GroupTable[ 0 ] );

}

/*-------------------------------------------------------------------------*/

UInt32
CPackGroupVocabulary::GetGroupCount( void )
{GUCEF_TRACE;

    return GroupTableSize;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPackGroupVocabulary::GetGroupIdAt( UInt32 index )
{GUCEF_TRACE;

    static const CORE::CString ids[] = { Scenes, Characters, StaticModels, Animations, VisualEffects, Materials, Textures, Audio, Prefabs, Shaders, Fonts, Packages, Other };
    return index < GroupTableSize ? ids[ index ] : Other;
}

/*-------------------------------------------------------------------------*/

UInt32
CPackGroupVocabulary::GetGroupOrder( const CORE::CString& groupId )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<GroupTableSize; ++i )
        if ( groupId == GroupTable[ i ].id )
            return i;
    return GroupTableSize;
}

/*-------------------------------------------------------------------------*/

bool
CPackGroupVocabulary::IsKnownGroup( const CORE::CString& groupId )
{GUCEF_TRACE;

    return GroupTableSize != GetGroupOrder( groupId );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackGroupVocabulary::GetGroupName( const CORE::CString& groupId )
{GUCEF_TRACE;

    const UInt32 order = GetGroupOrder( groupId );
    return order < GroupTableSize ? CORE::CString( GroupTable[ order ].name ) : groupId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPackGroupVocabulary::GetGroupOfKind( EAssetKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case ASSETKIND_MODEL:       return StaticModels;
        case ASSETKIND_SKELETON:    return Characters;
        case ASSETKIND_SCENE:       return Scenes;
        case ASSETKIND_ANIMATION:   return Animations;
        case ASSETKIND_PARTICLE:    return VisualEffects;
        case ASSETKIND_MATERIAL:    return Materials;
        case ASSETKIND_TEXTURE:     return Textures;
        case ASSETKIND_IMAGE:       return Textures;
        case ASSETKIND_AUDIO:       return Audio;
        case ASSETKIND_PREFAB:      return Prefabs;
        case ASSETKIND_SHADER:      return Shaders;
        case ASSETKIND_FONT:        return Fonts;
        case ASSETKIND_DOCUMENT:    return Other;
        case ASSETKIND_DATA:        return Other;
        case ASSETKIND_PACKAGE:     return Packages;
        case ASSETKIND_OTHER:       return Other;
        default:                    return CORE::CString::Empty;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
