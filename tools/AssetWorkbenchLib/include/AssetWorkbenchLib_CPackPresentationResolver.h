/*
 *  AssetWorkbenchLib: chooses what a pack shows when it is opened, from the profile of its recogniser and the kinds of its files
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

#ifndef ASSETWORKBENCHLIB_CPACKPRESENTATIONRESOLVER_H
#define ASSETWORKBENCHLIB_CPACKPRESENTATIONRESOLVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETPACKLISTING_H
#include "AssetConverterLib_CAssetPackListing.h"
#define ASSETCONVERTERLIB_CASSETPACKLISTING_H
#endif /* ASSETCONVERTERLIB_CASSETPACKLISTING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#include "AssetConverterLib_CAssetPackProfile.h"
#define ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPROFILE_H ? */

#ifndef ASSETWORKBENCHLIB_CPACKPRESENTATION_H
#include "AssetWorkbenchLib_CPackPresentation.h"
#define ASSETWORKBENCHLIB_CPACKPRESENTATION_H
#endif /* ASSETWORKBENCHLIB_CPACKPRESENTATION_H ? */

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
 *  Chooses the presentation of a pack. The order of the choice:
 *    1. a character package (MetaHuman): the portrait, when the converter of its engine is there
 *    2. a pack of effects: the effect in the four view layout, when the four views, the converter and the cooker are there
 *    3. the own scene of the pack (the best showcase candidate of the recogniser: the main scene of a Godot project, the demo or sample
 *       scene of a Unity pack), when the importer of its engine reads it and nothing said it failed before
 *    4. a generated presentation by the dominant content: animations, models in a grid, materials, pictures, environments, audio
 *    5. the summary
 *  A requirement that is missing sends the choice on to the next, and says so in 'fallbackReason'. A presentation that the user chose
 *  comes first, the same way. Nothing here loads a file: the resolver reads the names in the listing and the profile.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CPackPresentationResolver
{
    public:

    /** What the workbench can do, as far as it matters for the choice */
    struct ASSETWORKBENCHLIB_PUBLIC_CPP SCapabilities
    {
        bool fourViews;             /**< the four view layout exists */
        bool unityConverter;        /**< the importer of the Unity plugin */
        bool godotConverter;
        bool unrealConverter;
        bool ogreCooker;            /**< the cooker that makes particle scripts and scenes of an intermediate */
        bool audioPlayer;           /**< not built */
        SCapabilities( void );
    };

    struct ASSETWORKBENCHLIB_PUBLIC_CPP SOptions
    {
        UInt32 maxAssets;           /**< how many models or pictures are loaded: 48 */
        CORE::CString requestedId;  /**< the kind the user chose (CPackPresentation::GetKindId()); empty for the default */
        CPackPresentation::EKind failedKind;    /**< a kind that was tried and failed while it was loaded (a conversion that did not work) */
        CORE::CString failedReason;     /**< why: the kind is then not chosen again and the reason is in the fallback text; empty when nothing failed */
        UInt32 firstAsset;          /**< the first of the assets of the presentation that is loaded: "load more" goes on from there */
        SCapabilities capabilities;
        SOptions( void );
    };

    /** What the files of the pack are, by the kinds of the registry; the lists are in the order of the listing */
    struct SInventory
    {
        CORE::CString::StringVector models;         /**< models the views read as they are */
        CORE::CString::StringVector effects;        /**< particle scripts, and prefabs and scenes that are effects by their names */
        CORE::CString::StringVector animations;
        CORE::CString::StringVector materials;      /**< material scripts */
        CORE::CString::StringVector images;         /**< pictures and textures, without the environments */
        CORE::CString::StringVector environments;   /**< hdr and exr */
        CORE::CString::StringVector audio;
        UInt32 prefabCount;
        UInt32 sceneCount;
        UInt32 fileCount;
        SInventory( void );
    };

    /** Asks the plugin registry which importers and cookers are there; the four views and the missing audio player are set here too */
    static void DetectCapabilities( SCapabilities& capabilities );

    /** Counts what the pack holds; noise of the profile and metadata files are left out */
    static void MakeInventory( const ASSETCONVERTER::CAssetPackListing& listing, const ASSETCONVERTER::CAssetPackProfile& profile, SInventory& inventory );

    /** The presentation for the pack; false when there is nothing to show (the summary is then in 'presentation'). 'vfsRoot' is the folder of the pack in the VFS */
    static bool Resolve( const ASSETCONVERTER::CAssetPackListing& listing, const ASSETCONVERTER::CAssetPackProfile& profile, const CORE::CString& vfsRoot, const SOptions& options, CPackPresentation& presentation );

    /** The kinds the selector offers for this pack: those with their requirements met and something to show, the default first */
    static void GetOffered( const ASSETCONVERTER::CAssetPackListing& listing, const ASSETCONVERTER::CAssetPackProfile& profile, const CORE::CString& vfsRoot, const SOptions& options, GUCEF::vector< CPackPresentation::EKind >& kinds );

    /** The key of a pack in the settings: its place on the disk, without the characters the settings use */
    static CORE::CString MakePackKey( const CORE::CString& source );

    /** Whether the name of the path says it is an effect ("FX_Fire", "particles", "vfx", "effects") */
    static bool IsEffectPath( const CORE::CString& path );

    private:

    static bool Make( CPackPresentation::EKind kind, const ASSETCONVERTER::CAssetPackProfile& profile, const SInventory& inventory, const CORE::CString& vfsRoot, const SOptions& options, CPackPresentation& presentation, CORE::CString& problem );
    static CPackPresentation::EKind GetDefaultKind( const ASSETCONVERTER::CAssetPackProfile& profile, const SInventory& inventory );
    static CORE::CString GetSceneConverter( const CORE::CString& scenePath );
    static void Limit( const CORE::CString::StringVector& source, const SOptions& options, CPackPresentation& presentation );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CPACKPRESENTATIONRESOLVER_H ? */
