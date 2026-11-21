/*
 *  AssetWorkbenchLib: chooses what a pack shows when it is opened
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

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#include "AssetWorkbenchLib_CAssetTypeResolver.h"
#define ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#endif /* ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETWORKBENCHLIB_CPACKSHOWCASELAYOUT_H
#include "AssetWorkbenchLib_CPackShowcaseLayout.h"
#define ASSETWORKBENCHLIB_CPACKSHOWCASELAYOUT_H
#endif /* ASSETWORKBENCHLIB_CPACKSHOWCASELAYOUT_H ? */

#include "AssetWorkbenchLib_CPackPresentationResolver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackPresentationResolver::SCapabilities::SCapabilities( void )
    : fourViews( true ), unityConverter( false ), godotConverter( false ), unrealConverter( false ), ogreCooker( false ), audioPlayer( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackPresentationResolver::SOptions::SOptions( void )
    : maxAssets( 48 ), requestedId(), failedKind( CPackPresentation::KIND_SUMMARY ), failedReason(), firstAsset( 0 ), capabilities()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackPresentationResolver::SInventory::SInventory( void )
    : models(), effects(), animations(), materials(), images(), environments(), audio(), prefabCount( 0 ), sceneCount( 0 ), fileCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPackPresentationResolver::DetectCapabilities( SCapabilities& capabilities )
{GUCEF_TRACE;

    capabilities = SCapabilities();
    ASSETCONVERTER::CAssetConverterGlobal* global = ASSETCONVERTER::CAssetConverterGlobal::Instance();
    if ( GUCEF_NULL == global )
        return;
    const ASSETCONVERTER::CAssetPluginRegistry& registry = global->GetPluginRegistry();
    ASSETCONVERTER::TAssetImporterPtr importer;
    capabilities.unityConverter = registry.TryGetImporter( "unity", importer );
    capabilities.godotConverter = registry.TryGetImporter( "godot", importer );
    capabilities.unrealConverter = registry.TryGetImporter( "unreal", importer );
    ASSETCONVERTER::TAssetCookerPtr cooker;
    capabilities.ogreCooker = registry.TryGetCooker( "ogre", cooker );
}

/*-------------------------------------------------------------------------*/

bool
CPackPresentationResolver::IsEffectPath( const CORE::CString& path )
{GUCEF_TRACE;

    // by the names of the folders and of the file: "FX_Fire_01", "FX", "Particle_FX", "VFX", "Effects"
    const CORE::CString::StringVector parts = path.Lowercase().ParseElements( '/', false );
    for ( size_t i=0; i<parts.size(); ++i )
    {
        const CORE::CString& part = parts[ i ];
        if ( 0 == part.HasSubstr( "fx_" ) || "fx" == part || "effects" == part || part.HasSubstr( "particle" ) >= 0 || part.HasSubstr( "vfx" ) >= 0 )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationResolver::MakeInventory( const ASSETCONVERTER::CAssetPackListing& listing, const ASSETCONVERTER::CAssetPackProfile& profile, SInventory& inventory )
{GUCEF_TRACE;

    inventory = SInventory();
    const CORE::CString::StringVector& files = listing.GetFiles();
    for ( size_t i=0; i<files.size(); ++i )
    {
        const CORE::CString& path = files[ i ];
        if ( profile.IsNoise( path ) )
            continue;
        const EAssetKind kind = CAssetTypeResolver::GetKind( path );
        if ( ASSETKIND_METADATA == kind )
            continue;
        ++inventory.fileCount;
        const CORE::CString extension = CAssetTypeResolver::GetExtension( path ).Lowercase();
        // the prefabs and scenes that are effects by their names: the effects of an engine pack
        if ( ( "prefab" == extension || "tscn" == extension ) && IsEffectPath( path ) )
        {
            inventory.effects.push_back( path );
            continue;
        }
        switch ( kind )
        {
            case ASSETKIND_MODEL:       { if ( CAssetTypeResolver::HasView( path ) ) inventory.models.push_back( path ); break; }
            case ASSETKIND_PARTICLE:    { inventory.effects.push_back( path ); break; }
            case ASSETKIND_PREFAB:      { ++inventory.prefabCount; break; }
            case ASSETKIND_SCENE:       { ++inventory.sceneCount; break; }
            case ASSETKIND_ANIMATION:   { inventory.animations.push_back( path ); break; }
            case ASSETKIND_MATERIAL:    { if ( "material" == extension ) inventory.materials.push_back( path ); break; }
            case ASSETKIND_IMAGE:
            case ASSETKIND_TEXTURE:     { if ( "hdr" == extension || "exr" == extension ) inventory.environments.push_back( path ); else inventory.images.push_back( path ); break; }
            case ASSETKIND_AUDIO:       { inventory.audio.push_back( path ); break; }
            default:                    break;
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackPresentationResolver::GetSceneConverter( const CORE::CString& scenePath )
{GUCEF_TRACE;

    const CORE::CString extension = CAssetTypeResolver::GetExtension( scenePath ).Lowercase();
    if ( "unity" == extension )
        return "unity";
    if ( "tscn" == extension )
        return "godot";
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CPackPresentation::EKind
CPackPresentationResolver::GetDefaultKind( const ASSETCONVERTER::CAssetPackProfile& profile, const SInventory& inventory )
{GUCEF_TRACE;

    if ( "metahuman-package" == profile.packType )
        return CPackPresentation::KIND_PORTRAIT;
    if ( inventory.effects.size() >= 2 && inventory.effects.size() >= inventory.models.size() )
        return CPackPresentation::KIND_EFFECT_FOUR_VIEW;
    // the own scene of the pack first: the recogniser lists the best candidate first
    if ( !profile.showcaseCandidates.empty() )
    {
        const CORE::CString extension = CAssetTypeResolver::GetExtension( profile.showcaseCandidates[ 0 ] ).Lowercase();
        if ( "unity" == extension || "tscn" == extension || "umap" == extension )
            return CPackPresentation::KIND_SCENE_DEMO;
    }
    if ( inventory.animations.size() > inventory.models.size() && !inventory.models.empty() )
        return CPackPresentation::KIND_ANIMATION_PLAYER;
    if ( !inventory.models.empty() )
        return CPackPresentation::KIND_MODEL_GRID;
    // the dominant of the rest
    const size_t materials = inventory.materials.size(), images = inventory.images.size(), environments = inventory.environments.size(), audio = inventory.audio.size();
    if ( materials > 0 && materials * 2 >= images )
        return CPackPresentation::KIND_MATERIAL_PREVIEW;
    if ( environments > 0 && environments >= images && environments >= audio )
        return CPackPresentation::KIND_ENVIRONMENT_PREVIEW;
    if ( images > 0 && images >= audio )
        return CPackPresentation::KIND_IMAGE_GRID;
    if ( audio > 0 )
        return CPackPresentation::KIND_AUDIO_PLAYER;
    return CPackPresentation::KIND_SUMMARY;
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationResolver::Limit( const CORE::CString::StringVector& source, const SOptions& options, CPackPresentation& presentation )
{GUCEF_TRACE;

    // the order of the arrangement: by folder, then by name; the same pack always gives the same assets
    CPackShowcaseLayout::TItemVector items;
    for ( size_t i=0; i<source.size(); ++i )
    {
        CPackShowcaseLayout::SItem item;
        item.path = source[ i ];
        item.group = CPackShowcaseLayout::GetGroupOfPath( source[ i ] );
        items.push_back( item );
    }
    CPackShowcaseLayout::SortItems( items );
    presentation.assets.clear();
    // a pack with far more files than the limit is sampled: every n-th of the arrangement, so that all of its folders show something
    // and not only the first ones (a Unity package holds the packs it depends on, which sort first)
    if ( 0 == options.firstAsset && options.maxAssets > 0 && items.size() >= 2 * (size_t) options.maxAssets )
    {
        const size_t stride = items.size() / options.maxAssets;
        for ( size_t i=0; i<options.maxAssets && i*stride<items.size(); ++i )
            presentation.assets.push_back( items[ i * stride ].path );
        presentation.moreAssets = (UInt32) ( items.size() - presentation.assets.size() );
        return;
    }
    const size_t first = options.firstAsset < items.size() ? options.firstAsset : items.size();
    const size_t last = first + options.maxAssets < items.size() ? first + options.maxAssets : items.size();
    for ( size_t i=first; i<last; ++i )
        presentation.assets.push_back( items[ i ].path );
    presentation.moreAssets = (UInt32) ( items.size() - last );
}

/*-------------------------------------------------------------------------*/

bool
CPackPresentationResolver::Make( CPackPresentation::EKind kind, const ASSETCONVERTER::CAssetPackProfile& profile, const SInventory& inventory, const CORE::CString& vfsRoot, const SOptions& options, CPackPresentation& presentation, CORE::CString& problem )
{GUCEF_TRACE;

    const SCapabilities& caps = options.capabilities;
    presentation.Clear();
    presentation.kind = kind;
    presentation.packType = profile.packType;
    presentation.packRoot = vfsRoot;
    problem.Clear();
    if ( !options.failedReason.IsNULLOrEmpty() && options.failedKind == kind && CPackPresentation::KIND_SUMMARY != kind )
    {
        problem = options.failedReason;
        return false;
    }
    switch ( kind )
    {
        case CPackPresentation::KIND_PORTRAIT:
        {
            presentation.requirement = "the Unreal converter plugin and the root asset of the character";
            if ( profile.showcaseCandidates.empty() )
            {
                problem = "The package has no root asset of a character";
                return false;
            }
            if ( !caps.unrealConverter )
            {
                problem = "The Unreal converter plugin is not loaded: the character cannot be read";
                return false;
            }
            presentation.assets.push_back( profile.showcaseCandidates[ 0 ] );
            presentation.converter = "unreal";
            presentation.viewPreset = "Frame";
            presentation.why = "A character package opens on the portrait of the character: head and shoulders, three quarter view, neutral studio light";
            return true;
        }
        case CPackPresentation::KIND_EFFECT_FOUR_VIEW:
        {
            presentation.requirement = "the four view layout; for engine prefabs the converter of the engine and the Ogre cooker";
            if ( inventory.effects.empty() )
            {
                problem = "The pack has no effects";
                return false;
            }
            if ( !caps.fourViews )
            {
                problem = "The four view layout is not available";
                return false;
            }
            Limit( inventory.effects, options, presentation );
            const CORE::CString extension = CAssetTypeResolver::GetExtension( presentation.assets[ 0 ] ).Lowercase();
            if ( "prefab" == extension || "tscn" == extension )
            {
                presentation.converter = "prefab" == extension ? "unity" : "godot";
                if ( ( "unity" == presentation.converter && !caps.unityConverter ) || ( "godot" == presentation.converter && !caps.godotConverter ) || !caps.ogreCooker )
                {
                    problem = "The effects are " + CORE::CString( "prefab" == extension ? "Unity prefabs" : "Godot scenes" ) + " and need the converter plugin of the engine and the Ogre cooker";
                    return false;
                }
            }
            presentation.viewPreset = "Four views";
            presentation.why = "An effect pack opens on the first effect, playing, in the four view layout";
            return true;
        }
        case CPackPresentation::KIND_SCENE_DEMO:
        {
            presentation.requirement = "the importer of the engine that reads the scene of the pack";
            if ( profile.showcaseCandidates.empty() )
            {
                problem = "The pack names no scene of its own";
                return false;
            }
            const CORE::CString scene = profile.showcaseCandidates[ 0 ];
            const CORE::CString converter = GetSceneConverter( scene );
            if ( converter.IsNULLOrEmpty() )
            {
                problem = "The scene " + scene + " cannot be read: there is no importer for this kind of level";
                return false;
            }
            if ( ( "unity" == converter && !caps.unityConverter ) || ( "godot" == converter && !caps.godotConverter ) )
            {
                problem = "The scene " + scene + " needs the " + converter + " converter plugin, which is not loaded";
                return false;
            }
            presentation.assets.push_back( scene );
            presentation.converter = converter;
            presentation.viewPreset = "Frame";
            presentation.why = "The own scene of the pack, " + scene + ", read by the " + converter + " importer";
            return true;
        }
        case CPackPresentation::KIND_ANIMATION_PLAYER:
        {
            presentation.requirement = "a model and animation clips";
            if ( inventory.models.empty() || inventory.animations.empty() )
            {
                problem = "The pack has no model with animation clips";
                return false;
            }
            Limit( inventory.models, options, presentation );
            presentation.assets.resize( 1 );
            presentation.generated = true;
            presentation.viewPreset = "Three quarter";
            presentation.why = "Generated: the first model of an animation pack plays its clips in sequence";
            return true;
        }
        case CPackPresentation::KIND_MODEL_GRID:
        {
            presentation.requirement = "models that the 3D view reads";
            if ( inventory.models.empty() )
            {
                problem = "The pack has no models the 3D view reads";
                return false;
            }
            Limit( inventory.models, options, presentation );
            presentation.generated = true;
            presentation.viewPreset = "Three quarter";
            presentation.why = "Generated showcase: " + CORE::UInt64ToString( (UInt64) presentation.assets.size() ) + " of " + CORE::UInt64ToString( (UInt64) inventory.models.size() ) + " models in a grid, grouped by folder";
            return true;
        }
        case CPackPresentation::KIND_MATERIAL_PREVIEW:
        {
            presentation.requirement = "material scripts";
            if ( inventory.materials.empty() )
            {
                problem = "The pack has no material scripts";
                return false;
            }
            Limit( inventory.materials, options, presentation );
            presentation.generated = true;
            presentation.viewPreset = "Frame";
            presentation.why = "Generated: the first material on a sphere, with the pictures of the pack in the list";
            return true;
        }
        case CPackPresentation::KIND_IMAGE_GRID:
        {
            presentation.requirement = "pictures";
            if ( inventory.images.empty() )
            {
                problem = "The pack has no pictures";
                return false;
            }
            Limit( inventory.images, options, presentation );
            presentation.generated = true;
            presentation.why = "Generated: the pictures of the pack, " + CORE::UInt64ToString( (UInt64) inventory.images.size() ) + " of them, in the content browser";
            return true;
        }
        case CPackPresentation::KIND_ENVIRONMENT_PREVIEW:
        {
            presentation.requirement = "an environment picture";
            if ( inventory.environments.empty() )
            {
                problem = "The pack has no environment pictures";
                return false;
            }
            Limit( inventory.environments, options, presentation );
            presentation.generated = true;
            presentation.fallbackReason = "The environment as background and light is not built: the picture is shown";
            presentation.why = "Generated: the first environment picture of the pack";
            return true;
        }
        case CPackPresentation::KIND_AUDIO_PLAYER:
        {
            presentation.requirement = "the audio player (not built)";
            if ( inventory.audio.empty() )
            {
                problem = "The pack has no audio files";
                return false;
            }
            Limit( inventory.audio, options, presentation );
            presentation.generated = true;
            presentation.fallbackReason = "There is no audio player yet: the files are listed";
            presentation.why = "Generated: an audio pack lists its files until the audio player exists";
            return true;
        }
        default:
        {
            presentation.kind = CPackPresentation::KIND_SUMMARY;
            presentation.generated = true;
            presentation.why = "Nothing in the pack has a 3D or picture presentation: the summary is shown";
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CPackPresentationResolver::Resolve( const ASSETCONVERTER::CAssetPackListing& listing, const ASSETCONVERTER::CAssetPackProfile& profile, const CORE::CString& vfsRoot, const SOptions& options, CPackPresentation& presentation )
{GUCEF_TRACE;

    SInventory inventory;
    MakeInventory( listing, profile, inventory );

    // the first wanted kind: the choice of the user, else the default of the pack
    CPackPresentation::EKind wanted = GetDefaultKind( profile, inventory );
    bool userChoice = false;
    CPackPresentation::EKind requested = CPackPresentation::KIND_SUMMARY;
    if ( !options.requestedId.IsNULLOrEmpty() && CPackPresentation::TryParseKindId( options.requestedId, requested ) )
    {
        wanted = requested;
        userChoice = true;
    }

    // the chain: what was wanted, then the generated presentations by content, the summary last
    GUCEF::vector< CPackPresentation::EKind > chain;
    chain.push_back( wanted );
    chain.push_back( CPackPresentation::KIND_SCENE_DEMO );
    chain.push_back( CPackPresentation::KIND_MODEL_GRID );
    chain.push_back( CPackPresentation::KIND_MATERIAL_PREVIEW );
    chain.push_back( CPackPresentation::KIND_IMAGE_GRID );
    chain.push_back( CPackPresentation::KIND_ENVIRONMENT_PREVIEW );
    chain.push_back( CPackPresentation::KIND_AUDIO_PLAYER );
    chain.push_back( CPackPresentation::KIND_SUMMARY );

    CORE::CString reasons;
    for ( size_t i=0; i<chain.size(); ++i )
    {
        // after the wanted kind only the generated ones follow, the scene of the pack is not a fallback of the portrait of a character
        if ( i > 0 && CPackPresentation::KIND_SCENE_DEMO == chain[ i ] )
            continue;
        CORE::CString problem;
        CPackPresentation made;
        if ( Make( chain[ i ], profile, inventory, vfsRoot, options, made, problem ) )
        {
            presentation = made;
            presentation.wantedKind = wanted;
            presentation.userChoice = userChoice;
            if ( i > 0 )
            {
                presentation.fallbackReason = reasons;
                presentation.why = "Fallback to " + CPackPresentation::GetKindName( presentation.kind ) + ": " + reasons + ". " + made.why;
            }
            return CPackPresentation::KIND_SUMMARY != presentation.kind;
        }
        if ( !reasons.IsNULLOrEmpty() )
            reasons += "; ";
        reasons += CPackPresentation::GetKindName( chain[ i ] ) + " is not possible: " + problem;
    }
    presentation.Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationResolver::GetOffered( const ASSETCONVERTER::CAssetPackListing& listing, const ASSETCONVERTER::CAssetPackProfile& profile, const CORE::CString& vfsRoot, const SOptions& options, GUCEF::vector< CPackPresentation::EKind >& kinds )
{GUCEF_TRACE;

    kinds.clear();
    SInventory inventory;
    MakeInventory( listing, profile, inventory );
    const CPackPresentation::EKind defaultKind = GetDefaultKind( profile, inventory );
    GUCEF::vector< CPackPresentation::EKind > order;
    order.push_back( defaultKind );
    for ( Int32 k=0; k<(Int32) CPackPresentation::KIND_COUNT; ++k )
        if ( (CPackPresentation::EKind) k != defaultKind && CPackPresentation::KIND_SUMMARY != (CPackPresentation::EKind) k )
            order.push_back( (CPackPresentation::EKind) k );
    for ( size_t i=0; i<order.size(); ++i )
    {
        CPackPresentation made;
        CORE::CString problem;
        if ( CPackPresentation::KIND_SUMMARY != order[ i ] && Make( order[ i ], profile, inventory, vfsRoot, options, made, problem ) )
            kinds.push_back( order[ i ] );
    }
    kinds.push_back( CPackPresentation::KIND_SUMMARY );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackPresentationResolver::MakePackKey( const CORE::CString& source )
{GUCEF_TRACE;

    CORE::CString key = source.ReplaceChar( '\\', '/' ).ReplaceChar( '=', '_' ).ReplaceChar( '\n', '_' ).ReplaceChar( '\r', '_' );
    return key;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
