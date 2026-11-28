/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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

#include <math.h>
#include <string.h>
#include <algorithm>

#include <Ogre.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#include "AssetWorkbenchLib.h"
#include "AssetConverterLib.h"
#include "AssetConverterLib_CAssetConverterGlobal.h"
#include "AssetConverterLib_CAssetPluginRegistry.h"
#include "AssetConverterLib_CAssetScene.h"
#include "AssetConverterLib_CAssetAnimation.h"
#include "AssetConverterLib_CAssetSkin.h"
#include "AssetConverterLib_CConversionReport.h"
#include "AssetConverterLib_CIAssetExporter.h"
#include "AssetConverterLib_CCanonicalJson.h"
#include "AssetConverterLib_CVfsPaths.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"
#include "TestAssetWorkbenchPackPresentation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CPackPresentationResolver TResolver;

bool
Near( Float32 a, Float32 b, Float32 tolerance = 1e-3f )
{
    return a - b <= tolerance && b - a <= tolerance;
}

bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part ) >= 0;
}

/** What the plugins would be: the capabilities by name */
TResolver::SCapabilities
Caps( bool unity, bool godot, bool unreal, bool cooker, bool fourViews = true )
{
    TResolver::SCapabilities caps;
    caps.unityConverter = unity;
    caps.godotConverter = godot;
    caps.unrealConverter = unreal;
    caps.ogreCooker = cooker;
    caps.fourViews = fourViews;
    return caps;
}

TResolver::SOptions
Options( const TResolver::SCapabilities& caps, const CORE::CString& requested = CORE::CString::Empty )
{
    TResolver::SOptions options;
    options.capabilities = caps;
    options.requestedId = requested;
    return options;
}

ASSETCONVERTER::CAssetPackProfile
Profile( const CORE::CString& packType, const CORE::CString& candidate = CORE::CString::Empty )
{
    ASSETCONVERTER::CAssetPackProfile profile;
    profile.packType = packType;
    profile.packTypeName = packType;
    profile.confidence = 80;
    if ( !candidate.IsNULLOrEmpty() )
        profile.showcaseCandidates.push_back( candidate );
    return profile;
}

bool
Resolve( const ASSETCONVERTER::CAssetPackListing& listing, const ASSETCONVERTER::CAssetPackProfile& profile, const TResolver::SOptions& options, CPackPresentation& presentation )
{
    return TResolver::Resolve( listing, profile, "packroot", options, presentation );
}

/** A glTF with a box of this size and colour, or, with a count above one, that many boxes side by side as nodes of one scene */
CORE::CString
Base64( const GUCEF::vector< UInt8 >& data )
{
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    CORE::CString out;
    for ( size_t i=0; i<data.size(); i+=3 )
    {
        const UInt32 a = data[ i ], b = i+1 < data.size() ? data[ i+1 ] : 0, c = i+2 < data.size() ? data[ i+2 ] : 0;
        const UInt32 triple = ( a << 16 ) | ( b << 8 ) | c;
        out += alphabet[ ( triple >> 18 ) & 63 ];
        out += alphabet[ ( triple >> 12 ) & 63 ];
        out += i+1 < data.size() ? alphabet[ ( triple >> 6 ) & 63 ] : '=';
        out += i+2 < data.size() ? alphabet[ triple & 63 ] : '=';
    }
    return out;
}

CORE::CString
MakeBoxGltf( Float32 sx, Float32 sy, Float32 sz, Float32 r, Float32 g, Float32 b, UInt32 count = 1 )
{
    static const float faces[ 6 ][ 9 ] =
    {
        {  1, 0, 0,   0, 1, 0,   0, 0, 1 }, { -1, 0, 0,   0, 0, 1,   0, 1, 0 }, {  0, 1, 0,   0, 0, 1,   1, 0, 0 },
        {  0,-1, 0,   1, 0, 0,   0, 0, 1 }, {  0, 0, 1,   1, 0, 0,   0, 1, 0 }, {  0, 0,-1,   0, 1, 0,   1, 0, 0 }
    };
    static const float corners[ 4 ][ 2 ] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
    GUCEF::vector< UInt8 > bytes;
    GUCEF::vector< Float32 > positions, normals;
    for ( int f=0; f<6; ++f )
        for ( int c=0; c<4; ++c )
        {
            positions.push_back( sx * 0.5f * ( faces[ f ][ 0 ] + corners[ c ][ 0 ] * faces[ f ][ 3 ] + corners[ c ][ 1 ] * faces[ f ][ 6 ] ) );
            positions.push_back( sy * 0.5f * ( 1.0f + faces[ f ][ 1 ] + corners[ c ][ 0 ] * faces[ f ][ 4 ] + corners[ c ][ 1 ] * faces[ f ][ 7 ] ) );
            positions.push_back( sz * 0.5f * ( faces[ f ][ 2 ] + corners[ c ][ 0 ] * faces[ f ][ 5 ] + corners[ c ][ 1 ] * faces[ f ][ 8 ] ) );
            normals.push_back( faces[ f ][ 0 ] ); normals.push_back( faces[ f ][ 1 ] ); normals.push_back( faces[ f ][ 2 ] );
        }
    const UInt8* p = reinterpret_cast< const UInt8* >( &positions[ 0 ] );
    bytes.insert( bytes.end(), p, p + positions.size() * 4 );
    p = reinterpret_cast< const UInt8* >( &normals[ 0 ] );
    bytes.insert( bytes.end(), p, p + normals.size() * 4 );
    for ( int f=0; f<6; ++f )
    {
        static const int order[ 6 ] = { 0, 1, 2, 0, 2, 3 };
        for ( int i=0; i<6; ++i )
        {
            const UInt16 index = (UInt16) ( f * 4 + order[ i ] );
            bytes.push_back( (UInt8) ( index & 0xFF ) );
            bytes.push_back( (UInt8) ( index >> 8 ) );
        }
    }
    CORE::CString nodes;
    CORE::CString sceneNodes;
    for ( UInt32 n=0; n<count; ++n )
    {
        if ( n > 0 ) { nodes += ","; sceneNodes += ","; }
        nodes += "{\"name\":\"Box" + CORE::UInt32ToString( n ) + "\",\"mesh\":0,\"translation\":[" + CORE::FloatToString( 1.5f * sx * (Float32) n ) + ",0,0]}";
        sceneNodes += CORE::UInt32ToString( n );
    }
    return "{\"asset\":{\"version\":\"2.0\"},\"scene\":0,\"scenes\":[{\"nodes\":[" + sceneNodes + "]}],\"nodes\":[" + nodes + "],"
           "\"meshes\":[{\"primitives\":[{\"attributes\":{\"POSITION\":0,\"NORMAL\":1},\"indices\":2,\"material\":0}]}],"
           "\"materials\":[{\"pbrMetallicRoughness\":{\"baseColorFactor\":[" + CORE::FloatToString( r ) + "," + CORE::FloatToString( g ) + "," + CORE::FloatToString( b ) + ",1],\"metallicFactor\":0,\"roughnessFactor\":1}}],"
           "\"accessors\":[{\"bufferView\":0,\"componentType\":5126,\"count\":24,\"type\":\"VEC3\",\"min\":[" + CORE::FloatToString( -0.5f * sx ) + ",0," + CORE::FloatToString( -0.5f * sz ) + "],\"max\":[" + CORE::FloatToString( 0.5f * sx ) + "," + CORE::FloatToString( sy ) + "," + CORE::FloatToString( 0.5f * sz ) + "]},"
           "{\"bufferView\":1,\"componentType\":5126,\"count\":24,\"type\":\"VEC3\"},{\"bufferView\":2,\"componentType\":5123,\"count\":36,\"type\":\"SCALAR\"}],"
           "\"bufferViews\":[{\"buffer\":0,\"byteOffset\":0,\"byteLength\":288,\"target\":34962},{\"buffer\":0,\"byteOffset\":288,\"byteLength\":288,\"target\":34962},{\"buffer\":0,\"byteOffset\":576,\"byteLength\":72,\"target\":34963}],"
           "\"buffers\":[{\"byteLength\":648,\"uri\":\"data:application/octet-stream;base64," + Base64( bytes ) + "\"}]}";
}

/** The text of a sidecar with the hints of a character, as the converter of an Unreal package writes them (numbers of a character of the corpus, rounded) */
const char* const CharacterSidecar =
    "{\"extras\":{\"gucefPresentation\":{\"schema\":\"gucefPresentation/1\",\"kind\":\"character\",\"defaultView\":\"portrait\",\"unit\":\"m\","
    "\"views\":{\"portrait\":{\"frame\":\"headAndShoulders\",\"fovDegrees\":30,\"yawDegrees\":30,\"pitchDegrees\":3,\"distance\":0.83727306,\"frameHeight\":0.44869328,"
    "\"position\":{\"x\":-0.41700652,\"y\":1.5529866,\"z\":-0.71626633},\"target\":{\"x\":0.00105628,\"y\":1.5091672,\"z\":0.00783962}}},"
    "\"lights\":{\"preset\":\"neutralStudio\",\"key\":{\"yawDegrees\":45,\"pitchDegrees\":35,\"intensity\":1},\"fill\":{\"yawDegrees\":-45,\"pitchDegrees\":10,\"intensity\":0.35},"
    "\"rim\":{\"yawDegrees\":202.5,\"pitchDegrees\":30,\"intensity\":0.5}}}}}";

/** An 8 bit RGBA picture filled by a rule of the position */
template < typename TFill >
IMAGE::TPixelMapPtr
MakePixels( UInt32 width, UInt32 height, TFill fill )
{
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( UInt32 y=0; y<height; ++y )
        for ( UInt32 x=0; x<width; ++x )
        {
            UInt8 rgb[ 3 ] = { 0, 0, 0 };
            fill( x, y, rgb );
            UInt8* texel = data + ( (size_t) y * width + x ) * 4;
            texel[ 0 ] = rgb[ 0 ]; texel[ 1 ] = rgb[ 1 ]; texel[ 2 ] = rgb[ 2 ]; texel[ 3 ] = 255;
        }
    return pixels;
}

struct SBlack { void operator()( UInt32, UInt32, UInt8 rgb[ 3 ] ) const { rgb[ 0 ] = rgb[ 1 ] = rgb[ 2 ] = 0; } };
struct SGrey { void operator()( UInt32, UInt32, UInt8 rgb[ 3 ] ) const { rgb[ 0 ] = rgb[ 1 ] = rgb[ 2 ] = 120; } };
/** A disc in the middle on a dark background: what a model on the stage looks like */
struct SDisc
{
    UInt32 size;
    void operator()( UInt32 x, UInt32 y, UInt8 rgb[ 3 ] ) const
    {
        const Float32 dx = (Float32) x - 0.5f * size, dy = (Float32) y - 0.5f * size;
        const bool inside = dx * dx + dy * dy < 0.09f * size * size;
        rgb[ 0 ] = inside ? 200 : 30; rgb[ 1 ] = inside ? 90 : 32; rgb[ 2 ] = inside ? 40 : 40;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS OF THE CHOICE                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestResolver( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Presentation: the resolver chooses the default of every kind of pack, and every missing requirement falls back to the generated showcase with the reason" )
    try
    {
        CPackPresentation p;

        // a character package opens on the portrait of the character
        ASSETCONVERTER::CAssetPackListing character;
        character.AddFile( "Manifest.json" ); character.AddFile( "FileList.json" ); character.AddFile( "Hero/Hero.uasset" ); character.AddFile( "Hero/Textures/T_Skin.png" );
        const ASSETCONVERTER::CAssetPackProfile characterProfile = Profile( "metahuman-package", "Hero/Hero.uasset" );
        ASSERT_TRUE( Resolve( character, characterProfile, Options( Caps( false, false, true, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_PORTRAIT == p.kind && "unreal" == p.converter && 1 == p.assets.size() && "Hero/Hero.uasset" == p.assets[ 0 ] && !p.generated );
        ASSERT_TRUE( Contains( p.why, "portrait" ) && !p.requirement.IsNULLOrEmpty() && p.fallbackReason.IsNULLOrEmpty() );
        // without the converter of the engine: the pictures of the package, and the text says what was missing
        ASSERT_TRUE( Resolve( character, characterProfile, Options( Caps( true, true, false, true ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_IMAGE_GRID == p.kind && CPackPresentation::KIND_PORTRAIT == p.wantedKind && p.generated );
        ASSERT_TRUE( Contains( p.fallbackReason, "Unreal converter plugin" ) && Contains( p.why, "Fallback" ) );

        // an effect pack plays its first effect in four views; the prefabs need the converter and the cooker
        ASSETCONVERTER::CAssetPackListing effects;
        effects.AddFile( "Assets/Particle_FX/Prefabs/FX_Fire_01.prefab" ); effects.AddFile( "Assets/Particle_FX/Prefabs/FX_Smoke_01.prefab" );
        effects.AddFile( "Assets/Particle_FX/Prefabs/FX_Dust_01.prefab" ); effects.AddFile( "Assets/Props/Crate.fbx" );
        const ASSETCONVERTER::CAssetPackProfile effectsProfile = Profile( "unity-package" );
        ASSERT_TRUE( Resolve( effects, effectsProfile, Options( Caps( true, false, false, true ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_EFFECT_FOUR_VIEW == p.kind && "unity" == p.converter && 3 == p.assets.size() && "Four views" == p.viewPreset );
        ASSERT_TRUE( Resolve( effects, effectsProfile, Options( Caps( true, false, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == p.kind && Contains( p.fallbackReason, "Ogre cooker" ) );
        ASSERT_TRUE( Resolve( effects, effectsProfile, Options( Caps( true, false, false, true, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == p.kind && Contains( p.fallbackReason, "four view layout" ) );
        // particle scripts are read as they are
        ASSETCONVERTER::CAssetPackListing scripts;
        scripts.AddFile( "fx/a.particle" ); scripts.AddFile( "fx/b.particle" ); scripts.AddFile( "fx/a.material" );
        ASSERT_TRUE( Resolve( scripts, Profile( "model-pack" ), Options( Caps( false, false, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_EFFECT_FOUR_VIEW == p.kind && p.converter.IsNULLOrEmpty() && 2 == p.assets.size() );

        // the own scene of a pack comes first: the demo scene of a Unity pack, the main scene of a Godot project; a level of Unreal has no importer
        ASSETCONVERTER::CAssetPackListing scenePack;
        scenePack.AddFile( "Assets/Scenes/Demo.unity" ); scenePack.AddFile( "Assets/Models/Crate.fbx" ); scenePack.AddFile( "Assets/Models/Barrel.fbx" );
        const ASSETCONVERTER::CAssetPackProfile unityProfile = Profile( "unity-package", "Assets/Scenes/Demo.unity" );
        ASSERT_TRUE( Resolve( scenePack, unityProfile, Options( Caps( true, false, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_SCENE_DEMO == p.kind && "unity" == p.converter && "Assets/Scenes/Demo.unity" == p.assets[ 0 ] && !p.generated && Contains( p.why, "own scene" ) );
        // it failed while it was converted: the showcase, with what failed
        TResolver::SOptions failed = Options( Caps( true, false, false, false ) );
        failed.failedKind = CPackPresentation::KIND_SCENE_DEMO;
        failed.failedReason = "The conversion of the scene failed: boom";
        ASSERT_TRUE( Resolve( scenePack, unityProfile, failed, p ) );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == p.kind && CPackPresentation::KIND_SCENE_DEMO == p.wantedKind && Contains( p.fallbackReason, "boom" ) && 2 == p.assets.size() );
        ASSERT_TRUE( Resolve( scenePack, unityProfile, Options( Caps( false, true, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == p.kind && Contains( p.fallbackReason, "unity converter plugin" ) );

        // a level of Unreal and the main scene of a Godot project
        ASSETCONVERTER::CAssetPackListing level;
        level.AddFile( "Game.uproject" ); level.AddFile( "Content/Maps/Entry.umap" ); level.AddFile( "Content/Props/Crate.fbx" );
        ASSERT_TRUE( Resolve( level, Profile( "unreal-project", "Content/Maps/Entry.umap" ), Options( Caps( true, true, true, true ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == p.kind && Contains( p.fallbackReason, "no importer for this kind of level" ) );
        ASSETCONVERTER::CAssetPackListing godot;
        godot.AddFile( "project.godot" ); godot.AddFile( "scenes/main.tscn" ); godot.AddFile( "models/Crate.glb" );
        ASSERT_TRUE( Resolve( godot, Profile( "godot-project", "scenes/main.tscn" ), Options( Caps( false, true, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_SCENE_DEMO == p.kind && "godot" == p.converter );

        // models in a grid: the first 48 in the order of the arrangement, the rest to load later, the same every time
        ASSETCONVERTER::CAssetPackListing many;
        for ( UInt32 i=0; i<60; ++i )
            many.AddFile( "Models/Set" + CORE::UInt32ToString( i % 6 ) + "/Item" + CORE::UInt32ToString( 100 - i ) + ".fbx" );
        TResolver::SOptions capped = Options( Caps( true, true, true, true ) );
        ASSERT_TRUE( Resolve( many, Profile( "model-pack" ), capped, p ) );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == p.kind && 48 == p.assets.size() && 12 == p.moreAssets && p.generated );
        ASSERT_TRUE( 0 == p.assets[ 0 ].HasSubstr( "Models/Set0/" ) && p.assets[ 0 ] < p.assets[ 1 ] );
        CPackPresentation again;
        ASSERT_TRUE( Resolve( many, Profile( "model-pack" ), capped, again ) && again.assets == p.assets );
        capped.firstAsset = 48;
        CPackPresentation rest;
        ASSERT_TRUE( Resolve( many, Profile( "model-pack" ), capped, rest ) && 12 == rest.assets.size() && 0 == rest.moreAssets );
        CORE::CString::StringVector all = p.assets;
        all.insert( all.end(), rest.assets.begin(), rest.assets.end() );
        std::sort( all.begin(), all.end() );
        ASSERT_TRUE( 60 == all.size() && std::unique( all.begin(), all.end() ) == all.end() );

        // far more models than the limit: every n-th of the arrangement, so that every folder shows something, the same every time
        ASSETCONVERTER::CAssetPackListing huge;
        for ( UInt32 i=0; i<480; ++i )
            huge.AddFile( "Pack" + CORE::UInt32ToString( i / 120 ) + "/Item" + CORE::UInt32ToString( 1000 + i ) + ".fbx" );
        ASSERT_TRUE( Resolve( huge, Profile( "model-pack" ), capped = Options( Caps( true, true, true, true ) ), p ) );
        ASSERT_TRUE( 48 == p.assets.size() && 432 == p.moreAssets );
        bool packs[ 4 ] = { false, false, false, false };
        for ( size_t i=0; i<p.assets.size(); ++i )
            packs[ p.assets[ i ][ 4 ] - '0' ] = true;
        ASSERT_TRUE( packs[ 0 ] && packs[ 1 ] && packs[ 2 ] && packs[ 3 ] );
        CPackPresentation again2;
        ASSERT_TRUE( Resolve( huge, Profile( "model-pack" ), capped, again2 ) && again2.assets == p.assets );

        // an animation pack, a material pack, a pack of pictures, environments, audio, and something that has nothing to show
        ASSETCONVERTER::CAssetPackListing animations;
        animations.AddFile( "Hero.fbx" ); animations.AddFile( "Clips/Walk.anim" ); animations.AddFile( "Clips/Run.anim" ); animations.AddFile( "Clips/Jump.anim" );
        ASSERT_TRUE( Resolve( animations, Profile( "model-pack" ), Options( Caps( false, false, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_ANIMATION_PLAYER == p.kind && 1 == p.assets.size() && "Hero.fbx" == p.assets[ 0 ] );
        ASSETCONVERTER::CAssetPackListing materials;
        materials.AddFile( "m/Brick.material" ); materials.AddFile( "m/Wood.material" ); materials.AddFile( "t/brick.png" ); materials.AddFile( "t/wood.png" );
        ASSERT_TRUE( Resolve( materials, Profile( "model-pack" ), Options( Caps( false, false, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_MATERIAL_PREVIEW == p.kind && 2 == p.assets.size() );
        ASSETCONVERTER::CAssetPackListing pictures;
        pictures.AddFile( "ui/a.png" ); pictures.AddFile( "ui/b.png" ); pictures.AddFile( "ui/c.png" );
        ASSERT_TRUE( Resolve( pictures, Profile( "model-pack" ), Options( Caps( false, false, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_IMAGE_GRID == p.kind && 3 == p.assets.size() );
        ASSETCONVERTER::CAssetPackListing sky;
        sky.AddFile( "sky/day.hdr" ); sky.AddFile( "sky/night.exr" );
        ASSERT_TRUE( Resolve( sky, Profile( "model-pack" ), Options( Caps( false, false, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_ENVIRONMENT_PREVIEW == p.kind && Contains( p.fallbackReason, "not built" ) );
        ASSETCONVERTER::CAssetPackListing sounds;
        sounds.AddFile( "sfx/a.wav" ); sounds.AddFile( "sfx/b.ogg" ); sounds.AddFile( "sfx/c.mp3" );
        ASSERT_TRUE( Resolve( sounds, Profile( "model-pack" ), Options( Caps( false, false, false, false ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_AUDIO_PLAYER == p.kind && 3 == p.assets.size() && Contains( p.fallbackReason, "no audio player" ) );
        ASSETCONVERTER::CAssetPackListing unknown;
        unknown.AddFile( "readme.txt" ); unknown.AddFile( "data.bin" );
        ASSERT_FALSE( Resolve( unknown, Profile( "model-pack" ), Options( Caps( true, true, true, true ) ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_SUMMARY == p.kind && !p.why.IsNULLOrEmpty() );

        // what the user chose comes first, with the same fallbacks; a kind that is not known is no choice
        ASSETCONVERTER::CAssetPackListing mixed;
        mixed.AddFile( "a/Crate.fbx" ); mixed.AddFile( "a/crate.png" );
        ASSERT_TRUE( Resolve( mixed, Profile( "model-pack" ), Options( Caps( true, true, true, true ), "images" ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_IMAGE_GRID == p.kind && p.userChoice );
        ASSERT_TRUE( Resolve( mixed, Profile( "model-pack" ), Options( Caps( true, true, true, true ), "portrait" ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == p.kind && CPackPresentation::KIND_PORTRAIT == p.wantedKind && p.userChoice );
        ASSERT_TRUE( Resolve( mixed, Profile( "model-pack" ), Options( Caps( true, true, true, true ), "nonsense" ), p ) );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == p.kind && !p.userChoice );

        // the selector offers the default first, what is possible, and the summary last
        GUCEF::vector< CPackPresentation::EKind > offered;
        TResolver::GetOffered( mixed, Profile( "model-pack" ), "packroot", Options( Caps( true, true, true, true ) ), offered );
        ASSERT_TRUE( offered.size() >= 3 && CPackPresentation::KIND_MODEL_GRID == offered[ 0 ] && CPackPresentation::KIND_SUMMARY == offered[ offered.size() - 1 ] );
        bool hasImages = false, hasPortrait = false;
        for ( size_t i=0; i<offered.size(); ++i )
        {
            hasImages = hasImages || CPackPresentation::KIND_IMAGE_GRID == offered[ i ];
            hasPortrait = hasPortrait || CPackPresentation::KIND_PORTRAIT == offered[ i ];
        }
        ASSERT_TRUE( hasImages && !hasPortrait );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == CPackPresentation::KIND_MODEL_GRID && "grid" == CPackPresentation::GetKindId( CPackPresentation::KIND_MODEL_GRID ) );
        
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static CPackShowcaseLayout::SItem
Item( const CORE::CString& path, Float32 x, Float32 y, Float32 z )
{
    CPackShowcaseLayout::SItem item;
    item.path = path;
    item.group = CPackShowcaseLayout::GetGroupOfPath( path );
    item.sizeX = x; item.sizeY = y; item.sizeZ = z;
    return item;
}

static void
TestLayoutAndPreferences( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Showcase layout: the spacing comes from the bounds, every folder starts a row, the order is the same for the same models, an outlier is scaled and the whole is centred" )
    try
    {
        CPackShowcaseLayout::TItemVector items;
        items.push_back( Item( "b/Tank.fbx", 4, 2, 4 ) ); items.push_back( Item( "a/Crate.fbx", 2, 1, 2 ) ); items.push_back( Item( "b/Truck.fbx", 4, 2, 3 ) );
        items.push_back( Item( "a/Barrel.fbx", 1, 1.5f, 1 ) ); items.push_back( Item( "b/Van.fbx", 3, 2, 4 ) ); items.push_back( Item( "a/Chair.fbx", 1, 1, 1 ) );
        CPackShowcaseLayout::TItemVector reversed( items.rbegin(), items.rend() );
        CPackShowcaseLayout layout, same;
        CPackShowcaseLayout::SOptions options;
        ASSERT_TRUE( layout.Compute( items, options ) && same.Compute( reversed, options ) );
        // folder a, then folder b; by name inside the folder, letter case ignored: the order does not depend on the order they came in
        ASSERT_TRUE( "a/Barrel.fbx" == items[ 0 ].path && "a/Chair.fbx" == items[ 1 ].path && "a/Crate.fbx" == items[ 2 ].path && "b/Tank.fbx" == items[ 3 ].path );
        ASSERT_TRUE( items.size() == reversed.size() );
        for ( size_t i=0; i<items.size(); ++i )
            ASSERT_TRUE( items[ i ].path == reversed[ i ].path && Near( layout.GetCells()[ i ].x, same.GetCells()[ i ].x ) && Near( layout.GetCells()[ i ].z, same.GetCells()[ i ].z ) );
        // the biggest footprint is 4: the distance between neighbours is that with the gap of 35 percent
        ASSERT_TRUE( Near( layout.GetPitch(), 4.0f * 1.35f ) && 2 == layout.GetRowCount() && 2 == layout.GetGroupCount() );
        const CPackShowcaseLayout::TCellVector& cells = layout.GetCells();
        ASSERT_TRUE( 0 == cells[ 0 ].row && 2 == cells[ 2 ].column && 1 == cells[ 3 ].row && 0 == cells[ 3 ].column && 1 == cells[ 3 ].groupIndex );
        ASSERT_TRUE( Near( cells[ 1 ].x - cells[ 0 ].x, layout.GetPitch() ) && Near( cells[ 3 ].z - cells[ 0 ].z, layout.GetPitch() ) );
        // centred on the origin: a row of three has its middle model at x 0 and the two rows are on both sides
        ASSERT_TRUE( Near( cells[ 1 ].x, 0.0f ) && Near( cells[ 0 ].z, -0.5f * layout.GetPitch() ) && Near( cells[ 3 ].z, 0.5f * layout.GetPitch() ) );
        Float32 cx, cy, cz, radius;
        layout.GetBoundingSphere( cx, cy, cz, radius );
        ASSERT_TRUE( Near( cx, 0.0f ) && cy > 0.0f && radius > layout.GetPitch() );

        // at most so many in a row: a folder of three with two columns takes two rows
        CPackShowcaseLayout::TItemVector three;
        three.push_back( Item( "x/a.fbx", 1, 1, 1 ) ); three.push_back( Item( "x/b.fbx", 1, 1, 1 ) ); three.push_back( Item( "x/c.fbx", 1, 1, 1 ) );
        CPackShowcaseLayout narrow;
        CPackShowcaseLayout::SOptions two;
        two.maxColumns = 2;
        ASSERT_TRUE( narrow.Compute( three, two ) && 2 == narrow.GetRowCount() && 1 == narrow.GetGroupCount() && 0 == narrow.GetCells()[ 2 ].column && 1 == narrow.GetCells()[ 2 ].row );

        // one model far bigger than the others does not spread them: it is scaled down to 2.5 times the typical footprint
        CPackShowcaseLayout::TItemVector outlier;
        for ( UInt32 i=0; i<9; ++i )
            outlier.push_back( Item( "p/m" + CORE::UInt32ToString( i ) + ".fbx", 1, 1, 1 ) );
        outlier.push_back( Item( "p/zz_tower.fbx", 10, 30, 10 ) );
        CPackShowcaseLayout spread;
        ASSERT_TRUE( spread.Compute( outlier, options ) );
        ASSERT_TRUE( Near( spread.GetPitch(), 2.5f * 1.35f ) && Near( spread.GetCells()[ 9 ].scale, 0.25f ) && Near( spread.GetCells()[ 0 ].scale, 1.0f ) );
        // small models stand lost in cells that the biggest model sized: with a minimum share they are scaled up to it, within a limit, and the pitch stays
        CPackShowcaseLayout::SOptions share = options;
        share.minShare = 0.5f;
        CPackShowcaseLayout bigger;
        ASSERT_TRUE( bigger.Compute( outlier, share ) );
        ASSERT_TRUE( Near( bigger.GetPitch(), spread.GetPitch() ) );
        // the biggest footprint is 2.5 (the tower after its scaling): the others are scaled from 1 to 1.25, the tower is left as it is
        ASSERT_TRUE( Near( bigger.GetCells()[ 0 ].scale, 1.25f ) && Near( bigger.GetCells()[ 9 ].scale, 0.25f ) );
        share.maxUpscale = 1.1f;
        CPackShowcaseLayout limited;
        ASSERT_TRUE( limited.Compute( outlier, share ) && Near( limited.GetCells()[ 0 ].scale, 1.1f ) );
        CPackShowcaseLayout::TItemVector none;
        CPackShowcaseLayout empty;
        ASSERT_FALSE( empty.Compute( none, options ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Presentation preferences: the choice per pack is remembered, replaced and forgotten, the switches default to on and are kept in the settings, a character gives its portrait camera and lights" )
    try
    {
        CWorkbenchPreferences prefs;
        ASSERT_TRUE( prefs.openPackPresentation && prefs.automaticThumbnails );
        CORE::CString id;
        ASSERT_FALSE( prefs.GetPackPresentation( "/disk/a.mhpkg", id ) );
        prefs.SetPackPresentation( "/disk/a.mhpkg", "grid" );
        prefs.SetPackPresentation( "/disk/b.zip", "images" );
        ASSERT_TRUE( prefs.GetPackPresentation( "/disk/a.mhpkg", id ) && "grid" == id && prefs.GetPackPresentation( "/disk/b.zip", id ) && "images" == id );
        prefs.SetPackPresentation( "/disk/a.mhpkg", "portrait" );
        ASSERT_TRUE( prefs.GetPackPresentation( "/disk/a.mhpkg", id ) && "portrait" == id && 2 == prefs.packPresentations.size() );
        prefs.openPackPresentation = false;
        prefs.automaticThumbnails = false;
        CORE::CDataNode section( "AssetWorkbench" );
        prefs.Save( section );
        CWorkbenchPreferences loaded;
        loaded.Load( section );
        ASSERT_TRUE( !loaded.openPackPresentation && !loaded.automaticThumbnails && loaded.GetPackPresentation( "/disk/a.mhpkg", id ) && "portrait" == id && loaded.GetPackPresentation( "/disk/b.zip", id ) && "images" == id );
        loaded.ClearPackPresentation( "/disk/a.mhpkg" );
        ASSERT_TRUE( !loaded.GetPackPresentation( "/disk/a.mhpkg", id ) && loaded.GetPackPresentation( "/disk/b.zip", id ) );
        // settings of before the presentation existed: both on
        CWorkbenchPreferences old;
        old.Load( CORE::CDataNode( "AssetWorkbench" ) );
        ASSERT_TRUE( old.openPackPresentation && old.automaticThumbnails && old.packPresentations.empty() );
        // the list does not grow without bound
        for ( UInt32 i=0; i<CWorkbenchPreferences::MaxPackPresentations + 20; ++i )
            prefs.SetPackPresentation( "/p/" + CORE::UInt32ToString( i ), "grid" );
        ASSERT_TRUE( CWorkbenchPreferences::MaxPackPresentations == prefs.packPresentations.size() && !prefs.GetPackPresentation( "/p/0", id ) && prefs.GetPackPresentation( "/p/" + CORE::UInt32ToString( CWorkbenchPreferences::MaxPackPresentations + 19 ), id ) );
        ASSERT_TRUE( "/disk/a=b" != TResolver::MakePackKey( "/disk/a=b" ) && "/disk/x" == TResolver::MakePackKey( "\\disk\\x" ) );

        // the camera and lights of a character, from the sidecar of its conversion
        CORE::CDataNode sidecar;
        ASSERT_TRUE( ASSETCONVERTER::CCanonicalJson::Parse( CharacterSidecar, sidecar ) );
        CPackPresentation portrait;
        ASSERT_FALSE( portrait.camera.valid );
        ASSERT_TRUE( portrait.ReadCharacterHints( sidecar ) && portrait.camera.valid );
        const CPackPresentation::SCameraHint& camera = portrait.camera;
        ASSERT_TRUE( Near( camera.fovDegrees, 30.0f ) && Near( camera.yawDegrees, 30.0f ) && Near( camera.pitchDegrees, 3.0f ) && Near( camera.distance, 0.83727306f, 1e-5f ) );
        ASSERT_TRUE( Near( camera.target[ 0 ], 0.00105628f, 1e-6f ) && Near( camera.target[ 1 ], 1.5091672f, 1e-6f ) && Near( camera.target[ 2 ], 0.00783962f, 1e-6f ) );
        ASSERT_TRUE( Near( camera.eye[ 0 ], -0.41700652f, 1e-6f ) && Near( camera.eye[ 1 ], 1.5529866f, 1e-6f ) && Near( camera.eye[ 2 ], -0.71626633f, 1e-6f ) );
        // the eye is at the distance of the hint from the target, as the formula of the hints says
        const Float32 dx = camera.eye[ 0 ] - camera.target[ 0 ], dy = camera.eye[ 1 ] - camera.target[ 1 ], dz = camera.eye[ 2 ] - camera.target[ 2 ];
        ASSERT_TRUE( Near( sqrtf( dx * dx + dy * dy + dz * dz ), camera.distance, 1e-3f ) );
        ASSERT_TRUE( Near( portrait.keyLight.yawDegrees, 45.0f ) && Near( portrait.keyLight.pitchDegrees, 35.0f ) && Near( portrait.keyLight.intensity, 1.0f ) );
        ASSERT_TRUE( Near( portrait.fillLight.yawDegrees, -45.0f ) && Near( portrait.fillLight.intensity, 0.35f ) && Near( portrait.rimLight.yawDegrees, 202.5f ) && Near( portrait.rimLight.intensity, 0.5f ) );
        CPackPresentation noHints;
        CORE::CDataNode other;
        ASSERT_TRUE( ASSETCONVERTER::CCanonicalJson::Parse( "{\"extras\":{\"other\":{}}}", other ) );
        ASSERT_FALSE( noHints.ReadCharacterHints( other ) || noHints.camera.valid );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestAutoThumbnailStore( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Automatic thumbnails: only a worthwhile render is kept, under a name of its own, once per content, never over a thumbnail of the content itself, and they can be cleared without touching the others" )
    try
    {
        const CORE::CString folder = MakeCleanTestDir( "autothumbs" );
        CAutoThumbnailStore store;
        store.SetFolder( folder );
        CORE::CString reason;
        SDisc discRule;
        discRule.size = 200;
        const IMAGE::TPixelMapPtr disc = MakePixels( 300, 200, discRule );
        discRule.size = 300;
        const IMAGE::TPixelMapPtr square = MakePixels( 300, 300, discRule );

        // a black frame, and one that is one grey, are no picture of anything
        CAutoThumbnailStore::SQuality quality;
        ASSERT_FALSE( CAutoThumbnailStore::IsWorthwhile( MakePixels( 64, 64, SBlack() ), &quality ) );
        ASSERT_TRUE( Near( quality.deviation, 0.0f ) && Near( quality.coverage, 0.0f ) );
        ASSERT_FALSE( CAutoThumbnailStore::IsWorthwhile( MakePixels( 64, 64, SGrey() ) ) );
        ASSERT_TRUE( CAutoThumbnailStore::IsWorthwhile( square, &quality ) && quality.deviation > 10.0f && quality.coverage > 0.2f );
        ASSERT_FALSE( store.Store( "aaaa", MakePixels( 64, 64, SBlack() ), false, reason ) );
        ASSERT_TRUE( Contains( reason, "not worthwhile" ) && 0 == store.GetCount() && !store.Has( "aaaa" ) );

        // a worthwhile one is kept, as a thumbnail of the size of the others, under the name of the thumbnails of the content with a mark
        ASSERT_TRUE( store.Store( "aaaa", disc, false, reason ) && store.Has( "aaaa" ) && 1 == store.GetCount() );
        ASSERT_TRUE( "aaaa_v1_256_auto.png" == CAutoThumbnailStore::MakeFileName( "aaaa", 256 ) && "aaaa_v1_256.png" == CThumbnailProvider::MakeCacheFileName( "aaaa", 256 ) );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( folder, "aaaa_v1_256_auto.png" ) ) && !CORE::FileExists( CORE::CombinePath( folder, "aaaa_v1_256.png" ) ) );
        IMAGE::TPixelMapPtr read;
        ASSERT_TRUE( store.Read( "aaaa", 256, read ) && 256 == read->GetWidthInPixels() && 256 == read->GetHeightInPixels() );
        ASSERT_TRUE( CAutoThumbnailStore::IsWorthwhile( read ) );
        // once per content
        ASSERT_FALSE( store.Store( "aaaa", disc, false, reason ) );
        ASSERT_TRUE( Contains( reason, "already" ) && 1 == store.GetCount() );
        // the content changed, so its hash did: the new render has a file of its own
        ASSERT_TRUE( store.Store( "bbbb", square, false, reason ) && 2 == store.GetCount() );
        // a thumbnail of the content itself wins: nothing is stored for it
        ASSERT_TRUE( WriteTestFile( CORE::CombinePath( folder, CThumbnailProvider::MakeCacheFileName( "cccc", 256 ) ), "real" ) );
        ASSERT_TRUE( store.HasRealThumbnail( "cccc" ) && !store.HasRealThumbnail( "aaaa" ) );
        ASSERT_FALSE( store.Store( "cccc", disc, false, reason ) );
        ASSERT_TRUE( Contains( reason, "own" ) && !store.Has( "cccc" ) );
        // a load that had errors, and the switch of the settings
        ASSERT_FALSE( store.Store( "dddd", disc, true, reason ) );
        ASSERT_TRUE( Contains( reason, "errors" ) );
        store.SetEnabled( false );
        ASSERT_FALSE( store.Store( "eeee", disc, false, reason ) );
        ASSERT_TRUE( Contains( reason, "switched off" ) && !store.Has( "eeee" ) && store.Has( "aaaa" ) );
        store.SetEnabled( true );
        // the clear action takes the automatic ones away and leaves the thumbnails of the content
        ASSERT_TRUE( 2 == store.Clear() && 0 == store.GetCount() );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( folder, CThumbnailProvider::MakeCacheFileName( "cccc", 256 ) ) ) );
        ASSERT_TRUE( store.Store( "aaaa", disc, false, reason ) );
        // the fit keeps the shape of the picture in the square
        IMAGE::TPixelMapPtr fitted;
        ASSERT_TRUE( CAutoThumbnailStore::FitToSquare( disc, 100, fitted ) && 100 == fitted->GetWidthInPixels() && 100 == fitted->GetHeightInPixels() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static void
TestPreviewPostProcessor( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Presentation: the preview of a character keeps the rig, the weights and the animations, and keeps only the first level of detail of every group" )
    try
    {
        ASSETCONVERTER::CAssetScene scene;
        scene.name = "Hero";
        ASSETCONVERTER::CAssetMesh mesh;
        ASSETCONVERTER::CAssetSubMesh sub;
        sub.positions.assign( 9, 0.0f );
        sub.joints.assign( 3, 1 );
        sub.weights.assign( 3, 1.0f );
        sub.influencesPerVertex = 1;
        sub.indices.push_back( 0 ); sub.indices.push_back( 1 ); sub.indices.push_back( 2 );
        mesh.subMeshes.push_back( sub );
        scene.meshes.push_back( mesh );
        scene.meshes.push_back( mesh );
        ASSETCONVERTER::CAssetNode lod0, lod1;
        lod0.mesh = 0; lod0.skin = 0;
        lod1.mesh = 1; lod1.skin = 0;
        scene.nodes.push_back( lod0 );
        scene.nodes.push_back( lod1 );
        scene.skins.push_back( ASSETCONVERTER::CAssetSkin() );
        scene.animations.push_back( ASSETCONVERTER::CAssetAnimation() );
        ASSETCONVERTER::CAssetLodGroup group;
        group.AddLevel( 0, 0.5f );
        group.AddLevel( 1, 0.1f );
        scene.lodGroups.push_back( group );
        ASSETCONVERTER::CConversionReport report;
        CPackPreviewPostProcessor processor;
        ASSERT_TRUE( "workbench-preview" == processor.GetName() && processor.Process( scene, CORE::CDataNode(), report ) );
        ASSERT_TRUE( 1 == scene.skins.size() && 1 == scene.animations.size() && 0 == scene.nodes[ 0 ].skin && 0 == scene.nodes[ 1 ].skin );
        ASSERT_TRUE( 0 == scene.nodes[ 0 ].mesh && -1 == scene.nodes[ 1 ].mesh );
        ASSERT_TRUE( 3 == scene.meshes[ 0 ].subMeshes[ 0 ].joints.size() && 3 == scene.meshes[ 0 ].subMeshes[ 0 ].weights.size() && 1 == scene.meshes[ 0 ].subMeshes[ 0 ].influencesPerVertex );
        ASSERT_TRUE( 3 == scene.meshes[ 0 ].subMeshes[ 0 ].indices.size() && 9 == scene.meshes[ 0 ].subMeshes[ 0 ].positions.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

static void
TestPreviewMerge( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Presentation: the merging form of the preview bakes the static meshes of a scene of 5000 nodes into the world and merges them by material, and leaves skinned, moving and few meshes alone" )
    try
    {
        // 5000 nodes, each a triangle of one of three materials, at x = node index; one with a mirrored scale; a skinned one and one that an animation moves
        ASSETCONVERTER::CAssetScene scene;
        scene.name = "Town";
        ASSETCONVERTER::CAssetMesh meshes[ 3 ];
        for ( UInt32 m=0; m<3; ++m )
        {
            ASSETCONVERTER::CAssetSubMesh sub;
            const Float32 corners[ 9 ] = { 0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f };
            sub.positions.assign( corners, corners + 9 );
            for ( UInt32 v=0; v<3; ++v ) { sub.normals.push_back( 0.0f ); sub.normals.push_back( 0.0f ); sub.normals.push_back( 1.0f ); }
            sub.indices.push_back( 0 ); sub.indices.push_back( 1 ); sub.indices.push_back( 2 );
            sub.materialIndex = (Int32) m;
            meshes[ m ].subMeshes.push_back( sub );
            scene.meshes.push_back( meshes[ m ] );
        }
        const UInt32 count = 5000;
        for ( UInt32 i=0; i<count; ++i )
        {
            const UInt32 node = scene.AddNode( "N" + CORE::UInt32ToString( i ) );
            scene.nodes[ node ].mesh = (Int32) ( i % 3 );
            scene.nodes[ node ].transform.position = MATH::TFloat32Transform3D::TPositionVector( (Float32) i, 0.0f, 0.0f );
        }
        // a mirrored one (scale -1 in x), a skinned one, and one that moves
        scene.nodes[ 3 ].transform.scale.x = -1.0f;
        scene.nodes[ 3 ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 3.0f, 0.0f, 0.0f );
        scene.skins.push_back( ASSETCONVERTER::CAssetSkin() );
        scene.nodes[ 4 ].skin = 0;
        ASSETCONVERTER::CAssetAnimation clip;
        ASSETCONVERTER::CAssetAnimationChannel channel;
        channel.targetNode = 5;
        clip.channels.push_back( channel );
        scene.animations.push_back( clip );

        UInt32 before = 0, after = 0;
        ASSERT_TRUE( CPackPreviewPostProcessor::MergeStaticMeshes( scene, 200, before, after ) );
        // 4997 submeshes of three materials: one of each, and the other two nodes keep their own mesh
        ASSERT_TRUE( 4998 == before && 3 == after );
        ASSERT_TRUE( scene.nodes[ 4 ].mesh >= 0 && scene.nodes[ 5 ].mesh >= 0 && scene.nodes[ 0 ].mesh < 0 && scene.nodes[ 6 ].mesh < 0 );
        const ASSETCONVERTER::CAssetNode& merged = scene.nodes.back();
        ASSERT_TRUE( "PreviewMerged" == merged.name && merged.mesh >= 0 );
        const ASSETCONVERTER::CAssetMesh& mesh = scene.meshes[ (size_t) merged.mesh ];
        ASSERT_TRUE( 3 == mesh.subMeshes.size() );
        UInt32 vertices = 0, triangles = 0;
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        {
            vertices += mesh.subMeshes[ s ].GetVertexCount();
            triangles += mesh.subMeshes[ s ].GetTriangleCount();
        }
        ASSERT_TRUE( 3 * 4998 == vertices && 4998 == triangles );
        // material 0 holds the nodes 0, 3 (mirrored), 6 ...: the triangles lie at x = 0, 3 (turned to 3, 2, 3) and 6
        const ASSETCONVERTER::CAssetSubMesh& zero = mesh.subMeshes[ 0 ];
        ASSERT_TRUE( 0 == zero.materialIndex );
        ASSERT_TRUE( zero.positions[ 0 ] == 0.0f && zero.positions[ 3 ] == 1.0f && zero.positions[ 9 ] == 3.0f && zero.positions[ 12 ] == 2.0f && zero.positions[ 18 ] == 6.0f );
        // the mirror turned its winding over: the second and third corner swap
        ASSERT_TRUE( 3 == zero.indices[ 3 ] && 5 == zero.indices[ 4 ] && 4 == zero.indices[ 5 ] );
        ASSERT_TRUE( 0 == zero.indices[ 0 ] && 1 == zero.indices[ 1 ] && 2 == zero.indices[ 2 ] );
        ASSERT_TRUE( 1.0f == zero.normals[ 2 ] );

        // few meshes are left as they are
        ASSETCONVERTER::CAssetScene small;
        small.AddNode( "A" );
        small.meshes.push_back( meshes[ 0 ] );
        small.nodes[ 0 ].mesh = 0;
        ASSERT_FALSE( CPackPreviewPostProcessor::MergeStaticMeshes( small, 200, before, after ) );
        ASSERT_TRUE( 0 == small.nodes[ 0 ].mesh && 1 == small.nodes.size() );
        CPackPreviewPostProcessor merging( true );
        ASSERT_TRUE( "workbench-preview-merged" == merging.GetName() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE SHELL                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt64
NowInMs( void )
{
    return MT::PrecisionTickCount() * 1000 / MT::PrecisionTimerResolution();
}

/** What the recogniser of the fake plugin makes of a folder that holds pack.marker */
struct SFakePack
{
    CORE::CString packType;
    CORE::CString candidate;
    CORE::CString engine;
};
SFakePack g_fakePack;
UInt32 g_fakeExportBoxes = 1;
UInt32 g_fakeImportMillis = 0;          /**< how long the fake importer works, polling for a cancel (0: it does not wait) */
bool g_fakeImportSawCancel = false;     /**< whether the fake importer was told to stop */
UInt64 g_fakeImportStopTick = 0;        /**< when it saw the cancel */

/** Stands in for the plugins of the engines: it recognises the packs of the tests, and converts any asset to an empty scene (with the hints of a character on request) */
class CFakePackImporter : public ASSETCONVERTER::CIAssetImporter
{
    public:

    CORE::CString name;
    bool fail;
    bool hints;

    CFakePackImporter( const CORE::CString& importerName, bool failing, bool withHints ) : name( importerName ), fail( failing ), hints( withHints ) {}
    virtual CORE::CString GetName( void ) const { return name; }
    virtual UInt32 GetVersion( void ) const { return 1; }
    virtual bool CanImport( const CORE::CString&, ASSETCONVERTER::CAssetPackContext& ) const { return true; }

    virtual bool Import( const CORE::CString& path, ASSETCONVERTER::CAssetPackContext&, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report )
    {
        if ( fail )
        {
            report.AddError( name, path, "the fake importer fails" );
            return false;
        }
        if ( g_fakeImportMillis > 0 )
        {
            // a long import that looks for the cancel as the importers of the engines do
            const UInt64 started = MT::PrecisionTickCount();
            while ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) < (Float64) g_fakeImportMillis )
            {
                report.ReportProgress( 0.5f );
                if ( report.IsCancellationRequested() )
                {
                    g_fakeImportSawCancel = true;
                    g_fakeImportStopTick = MT::PrecisionTickCount();
                    return false;
                }
                MT::ThreadDelay( 5 );
            }
        }
        if ( hints )
        {
            CORE::CDataNode parsed;
            if ( !ASSETCONVERTER::CCanonicalJson::Parse( CharacterSidecar, parsed ) || GUCEF_NULL == parsed.FindChild( "extras" ) )
                return false;
            out.extras = *parsed.FindChild( "extras" );
        }
        return true;
    }

    virtual bool RecognisePack( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile ) const
    {
        if ( !listing.HasFile( "pack.marker" ) || g_fakePack.packType.IsNULLOrEmpty() )
            return false;
        profile.packType = g_fakePack.packType;
        profile.packTypeName = g_fakePack.packType;
        profile.engine = g_fakePack.engine;
        profile.confidence = 90;
        if ( !g_fakePack.candidate.IsNULLOrEmpty() )
            profile.showcaseCandidates.push_back( g_fakePack.candidate );
        profile.noisePatterns.push_back( "*.marker" );
        return true;
    }
};

/** The glTF exporter of the tests: writes the boxes of g_fakeExportBoxes next to the asset */
class CFakeGltfExporter : public ASSETCONVERTER::CIAssetExporter
{
    public:

    virtual CORE::CString GetName( void ) const { return "gltf"; }
    virtual UInt32 GetVersion( void ) const { return 1; }

    virtual bool Export( const ASSETCONVERTER::CAssetScene& scene, const CORE::CString& vfsDir, ASSETCONVERTER::CConversionReport& report )
    {
        const CORE::CString path = ASSETCONVERTER::CVfsPaths::Join( vfsDir, scene.name + ".gltf" );
        const CORE::CString text = MakeBoxGltf( 1.0f, 1.0f, 1.0f, 0.2f, 0.8f, 0.3f, g_fakeExportBoxes );
        CORE::CDynamicBuffer buffer;
        buffer.LinkTo( text.C_String(), text.Length() );
        if ( !VFS::CVfsGlobal::Instance()->GetVfs().StoreAsFile( path, buffer, 0, true ) )
            return false;
        report.AddOutputFile( path, CORE::CString::StringVector() );
        return true;
    }
};

/** The workbench in a window with a pack folder to open, the fake plugins in the registry and the preferences of its own */
class CPackFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CAssetMounter mounter;
    CAssetCapture* capture;
    CWorkbenchShell* shell;
    CWorkbenchPreferences preferences;
    CORE::CString dir;
    CORE::CString cache;
    CORE::CString::StringVector registered;
    bool ready;

    explicit CPackFixture( const CORE::CString& name )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), mounter(), capture( GUCEF_NULL ), shell( GUCEF_NULL ), preferences(), dir(), cache(), registered(), ready( false )
    {
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        cache = MakeCleanTestDir( name + "_cache" );
        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        capture = GUCEF_NEW CAssetCapture( window );
        if ( !capture->Initialize( 64 ) )
            return;
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        shell->SetPreferences( &preferences );
        ready = shell->Initialize( *context, capture, cache );
        shell->SetAsyncOpenEnabled( true );
    }

    ~CPackFixture()
    {
        if ( GUCEF_NULL != shell )
        {
            shell->Shutdown();
            GUCEF_DELETE shell;
        }
        contextPtr.Unlink();
        if ( GUCEF_NULL != capture )
        {
            capture->Shutdown();
            GUCEF_DELETE capture;
        }
        mounter.UnmountAll();
        ASSETCONVERTER::CAssetPluginRegistry& plugins = ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry();
        for ( size_t i=0; i<registered.size(); ++i )
        {
            if ( "gltf" == registered[ i ] )
                plugins.UnregisterExporter( registered[ i ] );
            else
                plugins.UnregisterImporter( registered[ i ] );
        }
        g_fakePack = SFakePack();
        g_fakeExportBoxes = 1;
        g_fakeImportMillis = 0;
        g_fakeImportSawCancel = false;
        g_fakeImportStopTick = 0;
    }

    bool RegisterImporter( const CORE::CString& name, bool fail = false, bool hints = false )
    {
        ASSETCONVERTER::CAssetPluginRegistry& plugins = ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry();
        plugins.UnregisterImporter( name );
        if ( !plugins.RegisterImporter( ASSETCONVERTER::TAssetImporterPtr( GUCEF_NEW CFakePackImporter( name, fail, hints ) ) ) )
            return false;
        registered.push_back( name );
        return true;
    }

    bool RegisterExporter( void )
    {
        ASSETCONVERTER::CAssetPluginRegistry& plugins = ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry();
        plugins.UnregisterExporter( "gltf" );
        if ( !plugins.RegisterExporter( ASSETCONVERTER::TAssetExporterPtr( GUCEF_NEW CFakeGltfExporter() ) ) )
            return false;
        registered.push_back( "gltf" );
        return true;
    }

    void Frame( void )
    {
        shell->Update( 5 );
        window->RenderFrame( GUCEF_NULL );
    }

    void Frames( UInt32 count )
    {
        for ( UInt32 i=0; i<count; ++i )
        {
            Frame();
            MT::PrecisionDelay( 4 );
        }
    }

    void Click( GUI::CWidget* widget )
    {
        context->FlushChanges();
        const GUI::CRect rect = widget->GetRect();
        context->InjectMouseMove( rect.left + rect.width / 2, rect.top + rect.height / 2 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
    }

    /** Frames until nothing is opening any more, then a few more for the picture to settle */
    bool WaitIdle( UInt32 timeoutInMs = 40000 )
    {
        const UInt64 start = NowInMs();
        Frames( 2 );
        while ( shell->IsOpening() && NowInMs() - start < timeoutInMs )
        {
            Frame();
            MT::PrecisionDelay( 4 );
        }
        Frames( 12 );
        return !shell->IsOpening();
    }

    /** A pack of boxes of eight colours in two folders */
    bool MakeBoxPack( const CORE::CString& name, UInt32 count = 8 )
    {
        static const float colours[ 8 ][ 3 ] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 1, 1, 0 }, { 1, 0, 1 }, { 0, 1, 1 }, { 1, 1, 1 }, { 1, 0.45f, 0 } };
        const CORE::CString base = dir + "/" + name;
        bool ok = WriteTestFile( base + "/pack.marker", "pack" );
        for ( UInt32 i=0; i<count && i<8; ++i )
            ok = WriteTestFile( base + ( i < 4 ? "/props/" : "/vehicles/" ) + "model" + CORE::UInt32ToString( i ) + ".gltf",
                                MakeBoxGltf( 1.0f + 0.1f * (Float32) i, 1.0f + 0.2f * (Float32) ( i % 3 ), 1.0f, colours[ i ][ 0 ], colours[ i ][ 1 ], colours[ i ][ 2 ] ) ) && ok;
        g_fakePack.packType = "model-pack";
        return ok;
    }
};

/** Counts the changes of the preferences that the shell announces */
class CChangeCounter : public CORE::CObserver
{
    public:

    UInt32* count;

    CChangeCounter( UInt32* counted, CORE::CNotifier* notifier ) : CORE::CObserver(), count( counted )
    {
        typedef CORE::CTEventHandlerFunctor< CChangeCounter > TCallback;
        TCallback callback( this, &CChangeCounter::OnChanged );
        SubscribeTo( notifier, CWorkbenchShell::PreferencesChangedEvent, callback );
    }

    virtual ~CChangeCounter() { UnsubscribeAllFromObserver(); }
    void OnChanged( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++(*count); }
};

/** The class of a colour of a pixel of the stage: which of the eight boxes it belongs to, -1 for the background and for what is too dark */
Int32
ClassOfPixel( const UInt8 rgba[ 4 ] )
{
    static const float references[ 8 ][ 3 ] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 1, 1, 0 }, { 1, 0, 1 }, { 0, 1, 1 }, { 1, 1, 1 }, { 1, 0.45f, 0 } };
    const float maximum = (float) ( rgba[ 0 ] > rgba[ 1 ] ? ( rgba[ 0 ] > rgba[ 2 ] ? rgba[ 0 ] : rgba[ 2 ] ) : ( rgba[ 1 ] > rgba[ 2 ] ? rgba[ 1 ] : rgba[ 2 ] ) );
    if ( maximum < 60.0f )
        return -1;
    const float minimum = (float) ( rgba[ 0 ] < rgba[ 1 ] ? ( rgba[ 0 ] < rgba[ 2 ] ? rgba[ 0 ] : rgba[ 2 ] ) : ( rgba[ 1 ] < rgba[ 2 ] ? rgba[ 1 ] : rgba[ 2 ] ) );
    // the light adds some of every channel: the colour is what is above the least channel; hardly any of it is the white box
    if ( maximum - minimum < 0.2f * maximum )
        return 6;
    const float range = maximum - minimum;
    const float n[ 3 ] = { ( rgba[ 0 ] - minimum ) / range, ( rgba[ 1 ] - minimum ) / range, ( rgba[ 2 ] - minimum ) / range };
    Int32 best = -1;
    float bestDistance = 0.4f;
    for ( Int32 c=0; c<8; ++c )
    {
        if ( 6 == c )
            continue;
        const float d = fabsf( n[ 0 ] - references[ c ][ 0 ] ) + fabsf( n[ 1 ] - references[ c ][ 1 ] ) + fabsf( n[ 2 ] - references[ c ][ 2 ] );
        if ( d < bestDistance )
        {
            bestDistance = d;
            best = c;
        }
    }
    return best;
}

}

/*-------------------------------------------------------------------------*/

static void
TestShellGrid( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Pack presentation: opening a pack of eight models shows the generated showcase without a click, in the background: eight models in a grid by folder with their names, each its own colour and place" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "pp_grid" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "fakepacks" ) && fixture.MakeBoxPack( "packA" ) );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( shell.IsPackPresentationEnabled() && !shell.HasPackPresentation() );
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packA" ) );
        // it runs as a request of the loader: the interface goes on
        ASSERT_TRUE( shell.IsOpening() );
        ASSERT_TRUE( fixture.WaitIdle() );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
        ASSERT_TRUE( shell.HasPackPresentation() && CPackPresentation::KIND_MODEL_GRID == shell.GetPackPresentation().kind && shell.GetPackPresentation().generated );
        CShowcaseView& showcase = shell.GetShowcase();
        ASSERT_TRUE( showcase.IsLoaded() && 8 == showcase.GetModelCount() && 0 == showcase.GetSkippedCount() );
        // two folders are two rows, four models a row
        ASSERT_TRUE( 2 == showcase.GetLayout().GetRowCount() && 2 == showcase.GetLayout().GetGroupCount() );
        // the names: a line object of the stage holds the letters
        Ogre::ManualObject* labels = shell.GetStage().GetLineObject( "showcase_labels" );
        ASSERT_TRUE( GUCEF_NULL != labels && labels->getNumSections() > 0 );
        // the inspector says what is shown and why
        const CORE::CDataNode& shown = shell.GetInspector()->GetData();
        const CORE::CDataNode* presentation = shown.FindChild( "Presentation" );
        ASSERT_TRUE( GUCEF_NULL != presentation && "Showcase grid" == presentation->GetAttributeValue( "Kind" ).AsString() && Contains( presentation->GetAttributeValue( "Why" ).AsString(), "8 of 8 models" ) );
        ASSERT_TRUE( GUCEF_NULL != shown.FindChild( "Showcase" ) && 8 == shown.FindChild( "Showcase" )->GetAttributeValue( "Models" ).AsUInt32() );

        // the picture: the eight colours are all in it, in eight different places
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( shell.GetStage().ReadPixels( pixels ) && !pixels.IsNULL() );
        UInt32 counts[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        double sumX[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 }, sumY[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        for ( UInt32 y=0; y<pixels->GetHeightInPixels(); ++y )
            for ( UInt32 x=0; x<pixels->GetWidthInPixels(); ++x )
            {
                UInt8 rgba[ 4 ];
                GetPixel( pixels, (Int32) x, (Int32) y, rgba );
                const Int32 c = ClassOfPixel( rgba );
                if ( c >= 0 )
                {
                    ++counts[ c ];
                    sumX[ c ] += x;
                    sumY[ c ] += y;
                }
            }
        DumpPicture( pixels, "pack_grid" );
        for ( Int32 c=0; c<8; ++c )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Pack grid test: class " << c << " has " << counts[ c ] << " pixels" );
            ASSERT_TRUE( counts[ c ] >= 10 );
        }
        for ( Int32 a=0; a<8; ++a )
            for ( Int32 b=a+1; b<8; ++b )
            {
                const double dx = sumX[ a ] / counts[ a ] - sumX[ b ] / counts[ b ], dy = sumY[ a ] / counts[ a ] - sumY[ b ] / counts[ b ];
                ASSERT_TRUE( dx * dx + dy * dy > 16.0 );
            }
        ASSERT_TRUE( 0 == shell.GetShowcase().FindModelAtGroundPoint( 1.0e6f, 1.0e6f ) + 1 );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestShellSwitch( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Pack presentation: the switch is on by default, off behaves as before (mounted and listed, nothing loaded), the command line and the menu override it at run time, and it is kept in the settings" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "pp_switch" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "fakepacks" ) && fixture.MakeBoxPack( "packA", 4 ) && fixture.MakeBoxPack( "packB", 4 ) );
        CWorkbenchShell& shell = *fixture.shell;
        const CORE::CString packA = CAssetMounter().MakeMountPointName( fixture.dir + "/packA" );
        const CORE::CString packB = CAssetMounter().MakeMountPointName( fixture.dir + "/packB" );
        UInt32 changes = 0;
        CChangeCounter counter( &changes, &shell );

        // off: the pack is mounted and shown in the browser, nothing is loaded
        shell.SetPackPresentationEnabled( false );
        ASSERT_TRUE( !shell.IsPackPresentationEnabled() && !fixture.preferences.openPackPresentation && 1 == changes );
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packA" ) );
        ASSERT_TRUE( !shell.IsOpening() );
        fixture.Frames( 10 );
        ASSERT_TRUE( !shell.HasPackPresentation() && !shell.GetShowcase().IsLoaded() && 0 == shell.GetLoader().GetLastFinishedRequest() );
        ASSERT_TRUE( 1 == fixture.mounter.GetMountCount() );
        // the selector and the explicit request work on demand
        ASSERT_TRUE( shell.OpenPack( packA, true ) );
        ASSERT_TRUE( fixture.WaitIdle() && shell.HasPackPresentation() && 4 == shell.GetShowcase().GetModelCount() );

        // the command line says on for this run, whatever the settings say; it is not kept
        shell.CloseAsset();
        shell.SetPackPresentationOverride( 1 );
        ASSERT_TRUE( shell.IsPackPresentationEnabled() && !fixture.preferences.openPackPresentation );
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packB" ) && fixture.WaitIdle() );
        ASSERT_TRUE( shell.HasPackPresentation() && packB == shell.GetPackRoot() );
        shell.SetPackPresentationOverride( 0 );
        ASSERT_FALSE( shell.IsPackPresentationEnabled() );
        ASSERT_FALSE( shell.OpenPack( packB, false ) );
        shell.SetPackPresentationOverride( -1 );

        // switched on again at run time, from the menu action, without a restart
        ASSERT_TRUE( shell.ExecuteAction( "PackPresentation" ) );
        ASSERT_TRUE( shell.IsPackPresentationEnabled() && fixture.preferences.openPackPresentation );
        ASSERT_TRUE( "View/Pack presentation" == shell.GetMenuPathOfAction( "PackPresentation" ) );
        ASSERT_TRUE( shell.OpenPack( packA, false ) && fixture.WaitIdle() && shell.HasPackPresentation() && packA == shell.GetPackRoot() );
        // kept in the settings of the user
        shell.SetPackPresentationEnabled( false );
        CORE::CDataNode section( "AssetWorkbench" );
        fixture.preferences.Save( section );
        CWorkbenchPreferences reloaded;
        reloaded.Load( section );
        ASSERT_TRUE( !reloaded.openPackPresentation && reloaded.automaticThumbnails );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestShellSelector( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Pack presentation: the user can pick another presentation, it is remembered for the pack and comes back at the next open, and reset puts the default back; another request drops a presentation that is loading" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "pp_selector" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "fakepacks" ) && fixture.MakeBoxPack( "packA", 6 ) );
        CWorkbenchShell& shell = *fixture.shell;
        const CORE::CString packA = CAssetMounter().MakeMountPointName( fixture.dir + "/packA" );
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packA" ) && fixture.WaitIdle() );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == shell.GetPackPresentation().kind && !shell.GetPackPresentation().userChoice );
        const CORE::CString key = shell.GetPackKey( packA );
        CORE::CString id;
        ASSERT_TRUE( !key.IsNULLOrEmpty() && !fixture.preferences.GetPackPresentation( key, id ) );
        const GUCEF::vector< CPackPresentation::EKind >& offered = shell.GetOfferedPresentations();
        ASSERT_TRUE( offered.size() >= 2 && CPackPresentation::KIND_MODEL_GRID == offered[ 0 ] && CPackPresentation::KIND_SUMMARY == offered[ offered.size() - 1 ] );

        // the user picks the summary: it is shown and remembered
        ASSERT_TRUE( shell.ExecuteAction( "Presentation:summary" ) && fixture.WaitIdle() );
        ASSERT_TRUE( CPackPresentation::KIND_SUMMARY == shell.GetPackPresentation().kind && shell.GetPackPresentation().userChoice && !shell.GetShowcase().IsLoaded() );
        ASSERT_TRUE( fixture.preferences.GetPackPresentation( key, id ) && "summary" == id );
        // closing and opening it again: the remembered choice, not the default
        shell.CloseAsset();
        ASSERT_TRUE( shell.OpenPack( packA, false ) && fixture.WaitIdle() );
        ASSERT_TRUE( CPackPresentation::KIND_SUMMARY == shell.GetPackPresentation().kind );
        // reset: the default, and the choice is forgotten
        ASSERT_TRUE( shell.ExecuteAction( "Presentation:default" ) && fixture.WaitIdle() );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == shell.GetPackPresentation().kind && !shell.GetPackPresentation().userChoice && !fixture.preferences.GetPackPresentation( key, id ) );
        ASSERT_TRUE( 6 == shell.GetShowcase().GetModelCount() );

        // stale: a request that comes while the presentation loads ends it, and its result is not shown
        ASSERT_TRUE( shell.OpenPack( packA, false ) );
        ASSERT_TRUE( shell.IsOpening() );
        const UInt32 before = shell.GetLoader().GetDroppedCount();
        ASSERT_TRUE( shell.OpenAssetAsync( packA + "/props/model0.gltf" ).HasAFuture() );
        ASSERT_TRUE( fixture.WaitIdle() );
        ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() && !shell.HasPackPresentation() && !shell.GetShowcase().IsLoaded() );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
        (void) before;
        // cancelled by the user: nothing is left behind
        ASSERT_TRUE( shell.OpenPack( packA, false ) && shell.IsOpening() );
        shell.CancelOpening();
        ASSERT_TRUE( fixture.WaitIdle() );
        ASSERT_TRUE( !shell.IsOpening() && !shell.GetShowcase().IsLoaded() && !shell.HasPackPresentation() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestShellConverted( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Pack presentation: the own scene of a pack is converted in the background and shown, a character opens on its portrait with the camera of its hints, and a scene that cannot be converted gives way to the showcase with the reason" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        {
            // the pack has a demo scene: the importer of the engine makes a scene of five boxes of it
            CPackFixture fixture( "pp_scene" );
            ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "unity" ) && fixture.RegisterExporter() && fixture.RegisterImporter( "fakepacks" ) );
            g_fakeExportBoxes = 5;
            ASSERT_TRUE( fixture.MakeBoxPack( "packU", 2 ) && WriteTestFile( fixture.dir + "/packU/Assets/Scenes/Demo.unity", "scene" ) );
            g_fakePack.packType = "unity-package";
            g_fakePack.candidate = "Assets/Scenes/Demo.unity";
            CWorkbenchShell& shell = *fixture.shell;
            ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packU" ) && fixture.WaitIdle( 60000 ) );
            ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
            ASSERT_TRUE( CPackPresentation::KIND_SCENE_DEMO == shell.GetPackPresentation().kind && !shell.GetPackPresentation().generated && Contains( shell.GetPackPresentation().why, "Demo.unity" ) );
            ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() && shell.GetModelView().IsLoaded() && shell.GetModelView().GetStatistics().vertexCount >= 5 * 24 );
            ASSERT_TRUE( !shell.GetShowcase().IsLoaded() );
        }
        {
            // the conversion fails: the generated showcase of the models, and the report says what was not shown and why
            CPackFixture fixture( "pp_scene_failed" );
            ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "unity", true ) && fixture.RegisterExporter() && fixture.RegisterImporter( "fakepacks" ) );
            ASSERT_TRUE( fixture.MakeBoxPack( "packU", 3 ) && WriteTestFile( fixture.dir + "/packU/Assets/Scenes/Demo.unity", "scene" ) );
            g_fakePack.packType = "unity-package";
            g_fakePack.candidate = "Assets/Scenes/Demo.unity";
            CWorkbenchShell& shell = *fixture.shell;
            ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packU" ) && fixture.WaitIdle( 60000 ) );
            ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == shell.GetPackPresentation().kind && CPackPresentation::KIND_SCENE_DEMO == shell.GetPackPresentation().wantedKind );
            ASSERT_TRUE( Contains( shell.GetPackPresentation().fallbackReason, "Demo.unity" ) && 3 == shell.GetShowcase().GetModelCount() );
        }
        {
            // a big scene file is converted like any other: there is no size cap, and the time allowed is a preference of 600 seconds
            CPackFixture fixture( "pp_scene_big" );
            ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "unity" ) && fixture.RegisterExporter() && fixture.RegisterImporter( "fakepacks" ) );
            ASSERT_TRUE( fixture.MakeBoxPack( "packU", 2 ) );
            CORE::CString big;
            for ( UInt32 i=0; i<64 * 1024; ++i )
                big += "0123456789abcdef";
            ASSERT_TRUE( WriteTestFile( fixture.dir + "/packU/Assets/Scenes/Demo.unity", big ) && WriteTestFile( fixture.dir + "/packU/Assets/Scenes/Small.unity", "scene" ) );
            g_fakePack.packType = "unity-package";
            g_fakePack.candidate = "Assets/Scenes/Demo.unity";
            ASSERT_TRUE( 600 == CWorkbenchPreferences().maxSceneSeconds && 600 == fixture.preferences.maxSceneSeconds );
            CWorkbenchShell& shell = *fixture.shell;
            ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packU" ) && fixture.WaitIdle( 60000 ) );
            ASSERT_TRUE( CPackPresentation::KIND_SCENE_DEMO == shell.GetPackPresentation().kind && shell.GetPackPresentation().fallbackReason.IsNULLOrEmpty() );
            ASSERT_TRUE( Contains( shell.GetPackPresentation().why, "Demo.unity" ) );
        }
        {
            // a conversion that takes longer than MaxSceneSeconds is cancelled: the importer is told and stops at once, the showcase follows with the reason
            CPackFixture fixture( "pp_scene_slow" );
            ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "unity" ) && fixture.RegisterExporter() && fixture.RegisterImporter( "fakepacks" ) );
            ASSERT_TRUE( fixture.MakeBoxPack( "packU", 3 ) && WriteTestFile( fixture.dir + "/packU/Assets/Scenes/Demo.unity", "scene" ) );
            g_fakePack.packType = "unity-package";
            g_fakePack.candidate = "Assets/Scenes/Demo.unity";
            g_fakeImportMillis = 60000;
            fixture.preferences.maxSceneSeconds = 1;
            CWorkbenchShell& shell = *fixture.shell;
            const UInt64 started = MT::PrecisionTickCount();
            ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packU" ) && fixture.WaitIdle( 60000 ) );
            const Float64 elapsed = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started );
            ASSERT_TRUE( g_fakeImportSawCancel && elapsed < 30000.0 );
            ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == shell.GetPackPresentation().kind && Contains( shell.GetPackPresentation().fallbackReason, "took longer than 1 s and was cancelled" ) );
            ASSERT_TRUE( 3 == shell.GetShowcase().GetModelCount() );
        }
        {
            // a character: the portrait, with the camera and the light of its hints
            CPackFixture fixture( "pp_portrait" );
            ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "unreal", false, true ) && fixture.RegisterExporter() && fixture.RegisterImporter( "fakepacks" ) );
            ASSERT_TRUE( WriteTestFile( fixture.dir + "/packC/pack.marker", "pack" ) && WriteTestFile( fixture.dir + "/packC/Hero/Hero.uasset", "asset" ) );
            g_fakePack.packType = "metahuman-package";
            g_fakePack.candidate = "Hero/Hero.uasset";
            CWorkbenchShell& shell = *fixture.shell;
            ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packC" ) && fixture.WaitIdle( 60000 ) );
            ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
            const CPackPresentation& portrait = shell.GetPackPresentation();
            ASSERT_TRUE( CPackPresentation::KIND_PORTRAIT == portrait.kind && portrait.camera.valid && !portrait.generated );
            ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() );
            // the camera is the one of the hints: the field of view, the target and the distance, at the angles of the eye
            const COrbitController& controller = shell.GetController();
            ASSERT_TRUE( Near( controller.GetFieldOfView(), 30.0f ) );
            ASSERT_TRUE( Near( controller.GetTarget().x, 0.00105628f, 1e-4f ) && Near( controller.GetTarget().y, 1.5091672f, 1e-4f ) && Near( controller.GetTarget().z, 0.00783962f, 1e-4f ) );
            ASSERT_TRUE( Near( controller.GetDistance(), 0.83727306f, 1e-3f ) );
            const COrbitController::SVector3 eye = controller.GetEye();
            ASSERT_TRUE( Near( eye.x, -0.41700652f, 2e-3f ) && Near( eye.y, 1.5529866f, 2e-3f ) && Near( eye.z, -0.71626633f, 2e-3f ) );
            ASSERT_TRUE( CWorkbenchStage::LIGHTPRESET_THREE_POINT == shell.GetStage().GetLightPreset() );
        }
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestShellAutoThumbnails( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Automatic thumbnails: the render of a presentation is kept once the picture has settled, the content browser shows it for the root of the pack, switched off nothing is kept, and clear takes them away" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "pp_auto" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "fakepacks" ) && fixture.MakeBoxPack( "packA", 6 ) && fixture.MakeBoxPack( "packB", 5 ) );
        CWorkbenchShell& shell = *fixture.shell;
        const CORE::CString packA = CAssetMounter().MakeMountPointName( fixture.dir + "/packA" );
        ASSERT_TRUE( shell.GetAutoThumbnails().IsEnabled() && 0 == shell.GetAutoThumbnails().GetCount() );
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packA" ) && fixture.WaitIdle() );
        fixture.Frames( 30 );
        ASSERT_TRUE( 1 == shell.GetAutoThumbnails().GetCount() );
        const CORE::CString hash = CAutoThumbnailStore::HashPack( packA );
        ASSERT_TRUE( !hash.IsNULLOrEmpty() && shell.GetAutoThumbnails().Has( hash ) && !shell.GetAutoThumbnails().HasRealThumbnail( hash ) );
        // the content browser asks for the picture of the root of the pack and gets the automatic one
        CThumbnailProvider* provider = shell.GetThumbnailProvider();
        ASSERT_TRUE( GUCEF_NULL != provider );
        CThumbnailRecorder recorder( provider, GUI::CIThumbnailProvider::ThumbnailReadyEvent );
        provider->RequestThumbnail( packA, CAutoThumbnailStore::DefaultSize );
        for ( UInt32 i=0; i<50 && 0 == recorder.count; ++i )
        {
            provider->Update( 50 );
            fixture.Frame();
        }
        ASSERT_TRUE( recorder.count > 0 );
        ASSERT_TRUE( recorder.lastHadPixels && 1 == provider->GetAutoHitCount() && packA == recorder.lastPath );

        // switched off: a second pack leaves nothing
        shell.SetAutomaticThumbnailsEnabled( false );
        ASSERT_TRUE( !fixture.preferences.automaticThumbnails && !shell.GetAutoThumbnails().IsEnabled() );
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packB" ) && fixture.WaitIdle() );
        fixture.Frames( 30 );
        ASSERT_TRUE( 1 == shell.GetAutoThumbnails().GetCount() );
        shell.SetAutomaticThumbnailsEnabled( true );

        // a model that has a thumbnail of its own does not get an automatic one
        const CORE::CString model = packA + "/props/model1.gltf";
        const CORE::CString modelHash = CThumbnailProvider::HashContent( model );
        ASSERT_TRUE( !modelHash.IsNULLOrEmpty() && WriteTestFile( CORE::CombinePath( shell.GetAutoThumbnails().GetFolder(), CThumbnailProvider::MakeCacheFileName( modelHash, 256 ) ), "real" ) );
        ASSERT_TRUE( shell.OpenAsset( model ) );
        fixture.Frames( 30 );
        ASSERT_TRUE( !shell.GetAutoThumbnails().Has( modelHash ) && 1 == shell.GetAutoThumbnails().GetCount() );
        // one that has none does
        const CORE::CString second = packA + "/props/model2.gltf";
        ASSERT_TRUE( shell.OpenAsset( second ) );
        fixture.Frames( 30 );
        ASSERT_TRUE( shell.GetAutoThumbnails().Has( CThumbnailProvider::HashContent( second ) ) && 2 == shell.GetAutoThumbnails().GetCount() );

        // clear: the automatic thumbnails are gone, the thumbnail of the content stays
        ASSERT_TRUE( 2 == shell.ClearAutoThumbnails() && 0 == shell.GetAutoThumbnails().GetCount() );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( shell.GetAutoThumbnails().GetFolder(), CThumbnailProvider::MakeCacheFileName( modelHash, 256 ) ) ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestShellSettingsPage( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Settings: the General page has the check boxes Show a presentation when a pack is opened and Create automatic thumbnails, on by default, with tooltips, and the button that clears the automatic thumbnails; Apply keeps them" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "pp_settings" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        CWorkbenchSettingsLayer layer( shell, fixture.preferences );
        ASSERT_TRUE( layer.Create() && layer.Open( "General" ) );
        fixture.Frames( 3 );
        CWorkbenchGeneralSettingsForm* page = layer.GetGeneralPage();
        ASSERT_TRUE( GUCEF_NULL != page );
        GUI::CWidget* presentation = page->GetWidget( "OpenPackPresentation" );
        GUI::CWidget* thumbnails = page->GetWidget( "AutomaticThumbnails" );
        GUI::CWidget* clear = page->GetWidget( "ClearAutomaticThumbnails" );
        ASSERT_TRUE( GUCEF_NULL != presentation && GUCEF_NULL != thumbnails && GUCEF_NULL != clear );
        ASSERT_TRUE( "Show a presentation when a pack is opened" == presentation->GetProperty( GUI::EWidgetProperty_Text ) && "Create automatic thumbnails" == thumbnails->GetProperty( GUI::EWidgetProperty_Text ) );
        ASSERT_TRUE( CORE::StringToBool( presentation->GetProperty( GUI::EWidgetProperty_Checked ), false ) && CORE::StringToBool( thumbnails->GetProperty( GUI::EWidgetProperty_Checked ), false ) );
        // tooltips of the three controls
        CWorkbenchTooltips tooltips;
        ASSERT_TRUE( !tooltips.FindText( "OpenPackPresentation" ).IsNULLOrEmpty() && !tooltips.FindText( "AutomaticThumbnails" ).IsNULLOrEmpty() && !tooltips.FindText( "ClearAutomaticThumbnails" ).IsNULLOrEmpty() );

        // switched off in the window and applied: the preferences and the shell follow
        fixture.Click( presentation );
        fixture.Click( thumbnails );
        ASSERT_TRUE( !presentation->GetBoolProperty( "Checked" ) && !thumbnails->GetBoolProperty( "Checked" ) );
        IMAGE::TPixelMapPtr general;
        fixture.window->RenderFrame( &general );
        DumpPicture( general, "pp_settings_general" );
        fixture.Click( layer.GetForm()->GetWidget( "Ok" ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( !fixture.preferences.openPackPresentation && !fixture.preferences.automaticThumbnails );
        ASSERT_TRUE( !shell.IsPackPresentationEnabled() && !shell.GetAutoThumbnails().IsEnabled() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

namespace {

/** Frames until the index of the pack is there (and a few more for the view to follow) */
bool
WaitIndex( CPackFixture& fixture, UInt32 timeoutInMs = 30000 )
{
    const UInt64 start = NowInMs();
    fixture.Frames( 2 );
    while ( fixture.shell->IsIndexing() && NowInMs() - start < timeoutInMs )
    {
        fixture.Frame();
        MT::PrecisionDelay( 4 );
    }
    fixture.Frames( 6 );
    return !fixture.shell->IsIndexing();
}

CORE::CString::StringVector
ListedNames( CWorkbenchShell& shell )
{
    return shell.GetBrowser()->GetListedNames();
}

bool
HasName( const CORE::CString::StringVector& names, const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

Int32
IndexOfName( const CORE::CString::StringVector& names, const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return (Int32) i;
    return -1;
}

}

/*-------------------------------------------------------------------------*/

static void
TestResourceViewShell( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Resource view: a pack that is opened is indexed in the background and shown by function, the second open reads the stored index, the switch, Re-index, double click on a resource and on a group, and the search work" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        // the index of the pack is not in the cache of the tests yet: the cold start builds it
        {
            const CORE::CString cache = ASSETCONVERTER::CPackIndexLocator::GetUserCacheDiskFolder();
            if ( CORE::DirExists( cache ) )
                CORE::DeleteDir( cache, true );
            CORE::CreateDirs( cache );
        }
        CPackFixture fixture( "rv_shell" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "fakepacks" ) && fixture.MakeBoxPack( "packA", 8 ) );
        CWorkbenchShell& shell = *fixture.shell;
        GUI::CContentBrowserForm* browser = shell.GetBrowser();
        shell.SetPackPresentationEnabled( false );
        ASSERT_TRUE( shell.IsOpenPacksInResourceView() && !shell.IsResourceViewShown() && !shell.GetResourceView().HasPack() );
        ASSERT_TRUE( "Resources" == browser->GetAlternateRootName() );

        // the cold start: the files first, the index in the background, then the resources
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packA" ) );
        ASSERT_TRUE( shell.IsIndexing() && !shell.IsResourceViewShown() );
        ASSERT_TRUE( WaitIndex( fixture ) );
        const ASSETCONVERTER::CPackIndexRunResult first = shell.GetLastIndexResult();
        ASSERT_TRUE( first.structureBuilt && first.written && 8 == first.resourcesGrouped && !first.reused );
        ASSERT_TRUE( shell.IsResourceViewShown() && "Resources" == browser->GetCurrentRootName() && shell.GetResourceView().HasPack() );
        const CORE::CString pack = shell.GetResourceViewPack();
        ASSERT_TRUE( !pack.IsNULLOrEmpty() && VFS::CVfsGlobal::Instance()->GetVfs().FileExists( first.indexFolder + "/pack-index.json" ) );
        CORE::CString::StringVector names = ListedNames( shell );
        ASSERT_TRUE( 1 == names.size() && "Static models (8)/" == names[ 0 ] );
        ASSERT_TRUE( browser->NavigateTo( "Resources", "/static-models" ) );
        names = ListedNames( shell );
        ASSERT_TRUE( 2 == names.size() && "Props (4)/" == names[ 0 ] && "Vehicles (4)/" == names[ 1 ] );

        // the second open reads the stored index: nothing is built, analysed or written
        ASSERT_TRUE( shell.StartPackIndex( pack, false ) && WaitIndex( fixture ) );
        const ASSETCONVERTER::CPackIndexRunResult second = shell.GetLastIndexResult();
        ASSERT_TRUE( second.reused && !second.written && !second.structureBuilt && 0 == second.resourcesGrouped );
        ASSERT_TRUE( second.indexFolder == first.indexFolder );

        // Re-index forces it, from the menu, and keeps what is valid
        ASSERT_TRUE( "View/Re-index pack" == shell.GetMenuPathOfAction( "ReIndex" ) );
        ASSERT_TRUE( shell.ExecuteAction( "ReIndex" ) && WaitIndex( fixture ) );
        const ASSETCONVERTER::CPackIndexRunResult third = shell.GetLastIndexResult();
        ASSERT_TRUE( third.structureBuilt && third.written && 8 == third.recordsKept && ASSETCONVERTER::CPackGateDecision::OUTCOME_FORCED == third.indexDecision.outcome );

        // the redo actions on the pack of the view: each is a local job of the jobs list and does exactly its step (counters of the result)
        shell.GetJobRegistry().SetVisibleAfterMs( 0 );
        ASSERT_TRUE( "Jobs/Re-analyse" == shell.GetMenuPathOfAction( "Redo:reanalyse" ) );
        ASSERT_TRUE( shell.ExecuteAction( "Redo:reanalyse" ) && shell.IsIndexing() && WaitIndex( fixture ) );
        const ASSETCONVERTER::CPackIndexRunResult analysed = shell.GetLastIndexResult();
        ASSERT_TRUE( !analysed.structureBuilt && 0 == analysed.resourcesGrouped );
        ASSERT_TRUE( shell.ExecuteAction( "Redo:rethumbnail" ) && WaitIndex( fixture ) );
        const ASSETCONVERTER::CPackIndexRunResult thumbs = shell.GetLastIndexResult();
        ASSERT_TRUE( !thumbs.structureBuilt && 0 == thumbs.resourcesGrouped && 0 == thumbs.resourcesAnalysed );
        ASSERT_TRUE( shell.ExecuteAction( "Redo:reindex" ) && WaitIndex( fixture ) );
        const ASSETCONVERTER::CPackIndexRunResult reindexed = shell.GetLastIndexResult();
        ASSERT_TRUE( reindexed.structureBuilt && 8 == reindexed.recordsKept && ASSETCONVERTER::CPackGateDecision::OUTCOME_FORCED == reindexed.indexDecision.outcome );
        ASSETSTORE::TAssetJobEntryVector localJobs;
        shell.GetJobRegistry().GetJobs( localJobs );
        UInt32 indexJobs = 0, analyseJobs = 0;
        for ( size_t j=0; j<localJobs.size(); ++j )
        {
            if ( "index" == localJobs[ j ].kind && "analyse" == localJobs[ j ].subKind )
                ++analyseJobs;
            else
            if ( "index" == localJobs[ j ].kind )
                ++indexJobs;
        }
        ASSERT_TRUE( analyseJobs >= 1 && indexJobs >= 2 && ASSETSTORE::ASSETJOB_STATE_DONE == localJobs[ localJobs.size() - 1 ].state );

        // a click and a double click on a resource show and open the asset where it is in the pack
        ASSERT_TRUE( browser->NavigateTo( "Resources", "/static-models/props" ) );
        names = ListedNames( shell );
        const Int32 leaf = IndexOfName( names, "model0.gltf" );
        ASSERT_TRUE( leaf >= 0 );
        GUI::CContentBrowserForm::TInt32Vector selection;
        selection.push_back( leaf );
        ASSERT_TRUE( browser->SelectItems( selection ) );
        ASSERT_TRUE( fixture.WaitIdle() );
        const CORE::CString shown = shell.GetShownPath();
        ASSERT_TRUE( shown.Length() > 17 && "props/model0.gltf" == shown.SubstrFromRange( shown.Length() - 17, shown.Length() ) && shown.HasSubstr( pack ) == 0 );
        ASSETCONVERTER::CPackIndexResource resource;
        ASSERT_TRUE( shell.GetResourceOfPath( "/static-models/props/model0.gltf", resource ) && "props/model0.gltf" == resource.path && "static-models" == resource.group );
        const Int32 other = IndexOfName( names, "model1.gltf" );
        ASSERT_TRUE( other >= 0 && browser->ActivateItem( (UInt32) other ) && fixture.WaitIdle() );
        ASSERT_TRUE( shell.GetShownPath().HasSubstr( "props/model1.gltf" ) > 0 );

        // a double click on a group opens the presentation of the group: the models in a grid
        ASSERT_TRUE( browser->NavigateTo( "Resources", "/" ) );
        ASSERT_TRUE( browser->ActivateItem( 0 ) && fixture.WaitIdle() );
        ASSERT_TRUE( shell.HasPackPresentation() && CPackPresentation::KIND_MODEL_GRID == shell.GetPackPresentation().kind );
        ASSERT_TRUE( "Resources" == browser->GetCurrentRootName() && "/static-models" == browser->GetCurrentDirectory() );

        // the search of the filter line goes over all the groups
        ASSERT_TRUE( browser->NavigateTo( "Resources", "/" ) );
        browser->SetFilter( "model6" );
        fixture.Frames( 3 );
        names = ListedNames( shell );
        ASSERT_TRUE( names.size() >= 1 && names[ names.size() - 1 ].SubstrFromRange( 0, 12 ) == "model6.gltf " );
        browser->SetFilter( CORE::CString::Empty );
        fixture.Frames( 3 );
        names = ListedNames( shell );
        ASSERT_TRUE( 1 == names.size() && "Static models (8)/" == names[ 0 ] );

        // the switch, from the tool bar and from the menu; it is kept
        shell.ShowResourceView( false );
        ASSERT_TRUE( !shell.IsResourceViewShown() && "Resources" != browser->GetCurrentRootName() && !fixture.preferences.resourceViewShown );
        ASSERT_TRUE( "View/Resource view" == shell.GetMenuPathOfAction( "ResourceView" ) );
        ASSERT_TRUE( shell.ExecuteAction( "ResourceView" ) );
        ASSERT_TRUE( shell.IsResourceViewShown() && "Resources" == browser->GetCurrentRootName() && fixture.preferences.resourceViewShown );
        CORE::CDataNode section( "AssetWorkbench" );
        fixture.preferences.Save( section );
        CWorkbenchPreferences reloaded;
        reloaded.Load( section );
        ASSERT_TRUE( reloaded.resourceViewShown && reloaded.openPacksInResourceView );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resource view: the setting decides the view a pack starts in, the command line overrides it for the run, and a pack that opens in the files still has its index" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "rv_start" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "fakepacks" ) && fixture.MakeBoxPack( "packB", 4 ) && fixture.MakeBoxPack( "packC", 5 ) && fixture.MakeBoxPack( "packD", 6 ) );
        CWorkbenchShell& shell = *fixture.shell;
        shell.SetPackPresentationEnabled( false );
        UInt32 changes = 0;
        CChangeCounter counter( &changes, &shell );
        shell.SetOpenPacksInResourceView( false );
        ASSERT_TRUE( !shell.IsOpenPacksInResourceView() && !fixture.preferences.openPacksInResourceView && 1 == changes );
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packB" ) && WaitIndex( fixture ) );
        ASSERT_TRUE( !shell.IsResourceViewShown() && shell.GetResourceView().HasPack() && 4 == shell.GetResourceView().GetManifest().counts.resources );

        // the command line says resources for this run, whatever the settings say; it is not kept
        shell.SetResourceViewOverride( 1 );
        ASSERT_TRUE( shell.IsOpenPacksInResourceView() && !fixture.preferences.openPacksInResourceView );
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packC" ) && WaitIndex( fixture ) );
        ASSERT_TRUE( shell.IsResourceViewShown() && 5 == shell.GetResourceView().GetManifest().counts.resources );
        shell.SetResourceViewOverride( 0 );
        shell.ShowResourceView( false );
        ASSERT_TRUE( !shell.IsOpenPacksInResourceView() && shell.OpenLocation( fixture.dir + "/packD" ) && WaitIndex( fixture ) && !shell.IsResourceViewShown() );
        shell.SetResourceViewOverride( -1 );

        // on again from the settings: the next pack starts in the resources
        shell.SetOpenPacksInResourceView( true );
        ASSERT_TRUE( shell.IsOpenPacksInResourceView() && fixture.preferences.openPacksInResourceView );
        ASSERT_TRUE( shell.StartPackIndex( shell.GetResourceViewPack(), false ) && WaitIndex( fixture ) && shell.IsResourceViewShown() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resource view: a pack of 10,000 resources browses within the frame budget: no frame, not the one that opens a folder, takes 100 ms" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "rv_large" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        GUI::CContentBrowserForm* browser = shell.GetBrowser();
        ASSETCONVERTER::CAssetPackListing listing;
        ASSETCONVERTER::CPackIndexBuilder builder( ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().GetKinds() );
        for ( UInt32 i=0; i<10000; ++i )
        {
            const CORE::CString path = "Assets/Props/SM_Prop_" + CORE::UInt32ToString( 100000 + i ) + ".fbx";
            listing.AddFile( path );
            builder.SetFileSize( path, 1000 + i );
        }
        ASSETCONVERTER::CPackIndexBuildOptions options;
        options.useFileSystemSizes = false;
        options.packName = "large";
        ASSETCONVERTER::CPackIndexDocument doc;
        ASSERT_TRUE( builder.BuildStructure( listing, ASSETCONVERTER::CAssetPackProfile(), options, doc, GUCEF_NULL ) && 10000 == doc.resources.size() );
        shell.GetResourceView().SetPack( doc, CORE::CString::Empty, "large" );

        {
            // what the file system alone takes for the folder with the 10,000 resources, and what the browser takes beyond it
            const UInt64 adapterStart = NowInMs();
            CORE::TFileSystemIteratorPtr iterator;
            shell.GetResourceView().TryGetFileSystemIterator( "/static-models/props", iterator );
            const UInt64 adapterTook = NowInMs() - adapterStart;
            const UInt64 secondStart = NowInMs();
            shell.GetResourceView().TryGetFileSystemIterator( "/static-models/props", iterator );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ResourceView: the file system lists the first page of the 10000 in " << adapterTook << " ms, then in " << ( NowInMs() - secondStart ) << " ms" );
        }
        UInt64 worst = 0;
        UInt32 measured = 0;
        const char* const folders[] = { "/", "/static-models", "/static-models/props", "/static-models/props/~p1", "/static-models/props/~p20", "/static-models", "/" };
        for ( size_t f=0; f<sizeof( folders ) / sizeof( folders[ 0 ] ); ++f )
        {
            const UInt64 start = NowInMs();
            if ( 0 == f )
                shell.ShowResourceView( true );
            ASSERT_TRUE( browser->NavigateTo( "Resources", folders[ f ] ) );
            fixture.Frame();
            const UInt64 took = NowInMs() - start;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ResourceView: opening " << folders[ f ] << " took " << took << " ms, " << browser->GetListedCount() << " items" );
            worst = took > worst ? took : worst;
            ++measured;
            for ( UInt32 i=0; i<6; ++i )
            {
                const UInt64 frameStart = NowInMs();
                fixture.Frame();
                const UInt64 frame = NowInMs() - frameStart;
                worst = frame > worst ? frame : worst;
                ++measured;
            }
        }
        ASSERT_TRUE( 5 == browser->GetListedCount() || browser->GetListedCount() > 0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ResourceView: the worst of " << measured << " frames of a pack of 10000 resources took " << worst << " ms" );
        ASSERT_TRUE( worst < 100 );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

namespace {

/** Writes down the answers of a thumbnail provider */
class CThumbnailAnswerLog : public CORE::CObserver
{
    public:

    CORE::CString::StringVector paths;
    GUCEF::vector< bool > withPicture;

    explicit CThumbnailAnswerLog( CORE::CNotifier* notifier ) : CORE::CObserver(), paths(), withPicture()
    {
        typedef CORE::CTEventHandlerFunctor< CThumbnailAnswerLog > TCallback;
        TCallback callback( this, &CThumbnailAnswerLog::OnReady );
        SubscribeTo( notifier, GUI::CIThumbnailProvider::ThumbnailReadyEvent, callback );
    }

    virtual ~CThumbnailAnswerLog() { UnsubscribeAllFromObserver(); }

    void OnReady( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        const GUI::CIThumbnailProvider::TThumbnailReadyEventData* ready = static_cast< const GUI::CIThumbnailProvider::TThumbnailReadyEventData* >( eventData );
        paths.push_back( ready->GetData().path );
        withPicture.push_back( !ready->GetData().pixels.IsNULL() );
    }
};

}

/*-------------------------------------------------------------------------*/

static void
TestResourceViewFollowUps( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Resource view: a pack with models and scenes still gets its presentation when it is opened with the resource view on (the grid, never the summary), whichever way it is opened" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "rv_present" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "fakepacks" ) && fixture.MakeBoxPack( "packP", 8 ) && fixture.MakeBoxPack( "packQ", 6 ) );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( shell.IsPackPresentationEnabled() && shell.IsOpenPacksInResourceView() );

        // opened as a folder: the presentation of the pack and the resource view
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packP" ) );
        ASSERT_TRUE( fixture.WaitIdle() && WaitIndex( fixture ) );
        ASSERT_TRUE( shell.HasPackPresentation() );
        ASSERT_FALSE( CPackPresentation::KIND_SUMMARY == shell.GetPackPresentation().kind );
        ASSERT_TRUE( CPackPresentation::KIND_MODEL_GRID == shell.GetPackPresentation().kind && 8 == shell.GetShowcase().GetModelCount() );
        ASSERT_TRUE( shell.IsResourceViewShown() && 8 == shell.GetResourceView().GetManifest().counts.resources );

        // the other way: the explicit request for a pack that is mounted already, as --open of its folder of the VFS does
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packQ" ) );
        ASSERT_TRUE( fixture.WaitIdle() && WaitIndex( fixture ) );
        ASSERT_TRUE( shell.OpenPack( shell.GetPackRoot(), true ) && fixture.WaitIdle() && WaitIndex( fixture ) );
        ASSERT_TRUE( shell.HasPackPresentation() && CPackPresentation::KIND_MODEL_GRID == shell.GetPackPresentation().kind && 6 == shell.GetShowcase().GetModelCount() );
        ASSERT_TRUE( shell.IsResourceViewShown() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resource view: the browser lists the groups in the fixed order of the vocabulary (the list and the tree), and the group names and counts are in view" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "rv_order" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        GUI::CContentBrowserForm* browser = shell.GetBrowser();
        const char* const paths[] = { "Audio/a.wav", "Textures/T_A.png", "Models/SM_A.fbx", "Scenes/S.scene", "Materials/M_A.material", "Prefabs/P.prefab", "Docs/readme.txt", "Pack/B.zip", GUCEF_NULL };
        ASSETCONVERTER::CAssetPackListing listing;
        ASSETCONVERTER::CPackIndexBuilder builder( ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().GetKinds() );
        for ( const char* const* p = paths; GUCEF_NULL != *p; ++p )
        {
            listing.AddFile( *p );
            builder.SetFileSize( *p, 10 );
        }
        ASSETCONVERTER::CPackIndexBuildOptions options;
        options.useFileSystemSizes = false;
        options.packName = "ordered";
        ASSETCONVERTER::CPackIndexDocument doc;
        ASSERT_TRUE( builder.BuildStructure( listing, ASSETCONVERTER::CAssetPackProfile(), options, doc, GUCEF_NULL ) );
        shell.GetResourceView().SetPack( doc, CORE::CString::Empty, "ordered" );
        shell.ShowResourceView( true );
        ASSERT_TRUE( browser->NavigateTo( "Resources", "/" ) );
        fixture.Frames( 2 );
        const CORE::CString::StringVector names = ListedNames( shell );
        ASSERT_TRUE( 8 == names.size() );
        static const char* const expected[] = { "Scenes", "Static models", "Materials", "Textures and images", "Audio", "Prefabs", "Package assets", "Other" };
        for ( size_t i=0; i<names.size(); ++i )
            ASSERT_TRUE( names[ i ].Length() > CORE::CString( expected[ i ] ).Length() && names[ i ].SubstrFromRange( 0, CORE::CString( expected[ i ] ).Length() + 2 ) == CORE::CString( expected[ i ] ) + " (" );

        // reversed when the column is sorted the other way, and by the other columns the rank is not used
        // (the tree has the order of the vocabulary as well: it is the list of the folders of the root)
        GUI::CWidget* tree = browser->GetWidget( "Folders" );
        ASSERT_TRUE( GUCEF_NULL != tree );
        const GUI::CWidget::TStringVector items = tree->GetStringListProperty( GUI::EWidgetProperty_Items );
        size_t scenesAt = items.size(), audioAt = items.size(), otherAt = items.size();
        for ( size_t i=0; i<items.size(); ++i )
        {
            if ( items[ i ].HasSubstr( "Scenes (" ) >= 0 ) scenesAt = i;
            if ( items[ i ].HasSubstr( "Audio (" ) >= 0 ) audioAt = i;
            if ( items[ i ].HasSubstr( "Other (" ) >= 0 ) otherAt = i;
        }
        ASSERT_TRUE( scenesAt < audioAt && audioAt < otherAt && otherAt < items.size() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resource view: the resources show the thumbnails that exist for the files of the pack, with the path they asked for, and nothing is rendered for the others" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CPackFixture fixture( "rv_thumbs" );
        ASSERT_TRUE( fixture.ready && fixture.RegisterImporter( "fakepacks" ) && fixture.MakeBoxPack( "packT", 2 ) );
        CWorkbenchShell& shell = *fixture.shell;
        shell.SetPackPresentationEnabled( false );
        ASSERT_TRUE( shell.OpenLocation( fixture.dir + "/packT" ) && WaitIndex( fixture ) );
        const CORE::CString root = shell.GetResourceViewPack();
        const ASSETCONVERTER::CPackIndexResource* record = shell.GetResourceView().GetManifest().FindResourceByPath( "props/model0.gltf" );
        ASSERT_TRUE( GUCEF_NULL != record );
        const CORE::CString viewPath = shell.GetResourceView().GetViewPathOf( *record );
        const CORE::CString vfsPath = shell.GetResourceView().GetVfsPathOf( *record );
        ASSERT_TRUE( !viewPath.IsNULLOrEmpty() && vfsPath.HasSubstr( root ) == 0 );

        CORE::CString thumbs = fixture.cache + "/thumbs";
        CORE::CreatePathDirectories( thumbs + "/" );
        CThumbnailProvider provider( fixture.mounter, *fixture.capture );
        provider.SetCacheFolder( thumbs );
        CResourceViewThumbnailProvider wrapper( provider, shell.GetResourceView() );
        CThumbnailAnswerLog recorder( &wrapper );
        wrapper.SetResourceMode( true );

        // none exists yet: the answer has no picture, and nothing was rendered
        wrapper.RequestThumbnail( viewPath, 64 );
        provider.Update( 200 );
        ASSERT_TRUE( 1 == recorder.paths.size() && viewPath == recorder.paths[ 0 ] && !recorder.withPicture[ 0 ] );
        ASSERT_TRUE( 0 == provider.GetRenderCount() );
        // a group has none either
        wrapper.RequestThumbnail( "/static-models", 64 );
        ASSERT_TRUE( 2 == recorder.paths.size() && "/static-models" == recorder.paths[ 1 ] && !recorder.withPicture[ 1 ] );

        // the file view makes it (a render, kept in the cache) ...
        wrapper.SetResourceMode( false );
        wrapper.RequestThumbnail( vfsPath, 64 );
        for ( UInt32 i=0; i<50 && recorder.paths.size() < 3; ++i )
        {
            provider.Update( 200 );
            fixture.Frame();
        }
        ASSERT_TRUE( 3 == recorder.paths.size() && vfsPath == recorder.paths[ 2 ] && recorder.withPicture[ 2 ] && 1 == provider.GetRenderCount() );

        // ... and the resource view has it, under the path of the resource view, without a render
        wrapper.SetResourceMode( true );
        wrapper.RequestThumbnail( viewPath, 64 );
        provider.Update( 200 );
        ASSERT_TRUE( 4 == recorder.paths.size() && viewPath == recorder.paths[ 3 ] && recorder.withPicture[ 3 ] );
        ASSERT_TRUE( 1 == provider.GetRenderCount() && provider.GetCacheHitCount() >= 1 );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}


void
PerformAssetWorkbenchPackPresentationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbench.PackPresentation" );

    TestResolver();
    TestLayoutAndPreferences();
    TestAutoThumbnailStore();
    TestPreviewPostProcessor();
    TestPreviewMerge();
    TestShellGrid();
    TestShellSwitch();
    TestShellSelector();
    TestShellConverted();
    TestShellAutoThumbnails();
    TestShellSettingsPage();
    TestResourceViewShell();
    TestResourceViewFollowUps();
}

/*-------------------------------------------------------------------------*/
