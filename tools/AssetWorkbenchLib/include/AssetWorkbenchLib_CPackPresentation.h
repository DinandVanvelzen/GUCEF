/*
 *  AssetWorkbenchLib: what a pack shows when it is opened: the kind of presentation, the assets, the view preset, camera and light hints, and why
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

#ifndef ASSETWORKBENCHLIB_CPACKPRESENTATION_H
#define ASSETWORKBENCHLIB_CPACKPRESENTATION_H

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
 *  What the workbench shows when a pack is opened, as data: the kind of presentation, the assets to load (limited and ordered, paths
 *  relative to the root of the pack), the view preset, the camera and light hints of a character, what the presentation needs and
 *  why this one was chosen. CPackPresentationResolver makes it from the profile and the listing of the pack; the shell carries it out.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CPackPresentation
{
    public:

    enum EKind
    {
        KIND_SUMMARY = 0        ,   /**< nothing to show in 3D: the summary of the pack only */
        KIND_PORTRAIT           ,   /**< the head and shoulders of a character, three quarter view, neutral studio light */
        KIND_EFFECT_FOUR_VIEW   ,   /**< a particle effect that plays, in the four view layout */
        KIND_SCENE_DEMO         ,   /**< the own scene of the pack (demo, sample, main scene, default map) */
        KIND_MODEL_GRID         ,   /**< the models in a grid, grouped by folder, with names */
        KIND_ANIMATION_PLAYER   ,   /**< a representative model that plays the clips of the pack */
        KIND_MATERIAL_PREVIEW   ,   /**< a material on a sphere and a plane with tiling */
        KIND_IMAGE_GRID         ,   /**< the pictures of the pack in a grid */
        KIND_AUDIO_PLAYER       ,   /**< the audio player (not built: the files are listed, with a note) */
        KIND_ENVIRONMENT_PREVIEW,   /**< an environment picture as background and light (not built: the picture is shown, with a note) */
        KIND_COUNT
    };

    /** The camera the character asks for: a portrait of the head and shoulders. Angles in degrees, lengths in the units of the asset */
    struct SCameraHint
    {
        bool valid;
        Float32 fovDegrees;
        Float32 yawDegrees;
        Float32 pitchDegrees;
        Float32 distance;
        Float32 target[ 3 ];
        Float32 eye[ 3 ];
        SCameraHint( void );
    };

    /** One of the three lights of the neutral studio set of a character */
    struct SLightHint
    {
        Float32 yawDegrees;
        Float32 pitchDegrees;
        Float32 intensity;
        SLightHint( void );
    };

    CPackPresentation( void );
    void Clear( void );

    /** The stable lower case name of a kind for the settings ("portrait", "effect", "scene", "grid", "animation", "material", "images", "audio", "environment", "summary") */
    static CORE::CString GetKindId( EKind kind );
    /**
     *  Where the geometry of this presentation comes from, for the inspector and the report: the cooked runtime files of Ogre, or
     *  a quick look through the Assimp codec of Ogre (the original files, or the intermediate glTF of a conversion in the cache
     *  folder), which is not the converted output of the pack. For a quick look it says what the cooked output is, and that Convert for Ogre makes it
     */
    CORE::CString GetSourceDescription( void ) const;

    /** What a person reads: "Portrait", "Effect in four views", "Pack scene", "Showcase grid" */
    static CORE::CString GetKindName( EKind kind );
    static bool TryParseKindId( const CORE::CString& id, EKind& kind );

    /**
     *  Takes the camera and the lights of a character from the conversion sidecar of its asset: the scene extras "gucefPresentation" of
     *  kind "character" with the default view "portrait" (schema gucefPresentation/1): the portrait view gives the target, the eye, the
     *  field of view, the angles and the distance, the lights the angles and intensities of the key, fill and rim light. False, nothing
     *  changed, when the sidecar holds no such hints
     */
    bool ReadCharacterHints( const CORE::CDataNode& sidecar );

    /** What was chosen */
    EKind kind;
    /** What the pack type or the user asked for first; differs from 'kind' when a requirement was missing */
    EKind wantedKind;
    CORE::CString packType;
    /** The VFS folder of the pack; the paths of 'assets' are below it */
    CORE::CString packRoot;
    /** The assets to load, in the order they are shown: at most the limit of the resolver */
    CORE::CString::StringVector assets;
    /** How many more assets of the pack fit the presentation than 'assets' holds ("load more") */
    UInt32 moreAssets;
    /** The converter plugin that has to turn the assets into something the views read ("unreal", "unity", "godot"); empty when the views read them as they are */
    CORE::CString converter;
    /** The view preset of the 3D view: "Four views", "Three quarter", "Frame", ... */
    CORE::CString viewPreset;
    /** The generated presentation instead of the one of the pack */
    bool generated;
    /** The user chose it (a remembered choice or the selector) */
    bool userChoice;
    SCameraHint camera;
    SLightHint keyLight;
    SLightHint fillLight;
    SLightHint rimLight;
    /** What this presentation needs ("the unreal converter plugin"), empty when nothing */
    CORE::CString requirement;
    /** Why this one: for the log and the inspector */
    CORE::CString why;
    /** Why the wanted one is not shown, when it is not */
    CORE::CString fallbackReason;
    /** The geometry is the current cooked Ogre output (gucef-ogre3d) of the pack, not a quick look */
    bool cooked;
    /** For a quick look: what the cooked output is ("cooked output: none (not converted yet), use Convert for Ogre", "cooked output: stale (...)"); empty when it was not asked */
    CORE::CString cookedNote;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CPACKPRESENTATION_H ? */
