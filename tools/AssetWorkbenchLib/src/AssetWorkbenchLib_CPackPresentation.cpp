/*
 *  AssetWorkbenchLib: what a pack shows when it is opened
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

#ifndef ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H
#include "AssetWorkbenchLib_CCookedOutputResolver.h"
#define ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H
#endif /* ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H ? */

#include "AssetWorkbenchLib_CPackPresentation.h"

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

CPackPresentation::SCameraHint::SCameraHint( void )
    : valid( false )
    , fovDegrees( 30.0f )
    , yawDegrees( 30.0f )
    , pitchDegrees( 3.0f )
    , distance( 1.0f )
{GUCEF_TRACE;

    for ( int i=0; i<3; ++i )
    {
        target[ i ] = 0.0f;
        eye[ i ] = 0.0f;
    }
}

/*-------------------------------------------------------------------------*/

CPackPresentation::SLightHint::SLightHint( void )
    : yawDegrees( 0.0f )
    , pitchDegrees( 0.0f )
    , intensity( 1.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackPresentation::CPackPresentation( void )
    : kind( KIND_SUMMARY )
    , wantedKind( KIND_SUMMARY )
    , packType()
    , packRoot()
    , assets()
    , moreAssets( 0 )
    , converter()
    , viewPreset()
    , generated( false )
    , userChoice( false )
    , camera()
    , keyLight()
    , fillLight()
    , rimLight()
    , requirement()
    , why()
    , fallbackReason()
    , cooked( false )
    , cookedNote()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPackPresentation::Clear( void )
{GUCEF_TRACE;

    *this = CPackPresentation();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackPresentation::GetKindId( EKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case KIND_PORTRAIT:             return "portrait";
        case KIND_EFFECT_FOUR_VIEW:     return "effect";
        case KIND_SCENE_DEMO:           return "scene";
        case KIND_MODEL_GRID:           return "grid";
        case KIND_ANIMATION_PLAYER:     return "animation";
        case KIND_MATERIAL_PREVIEW:     return "material";
        case KIND_IMAGE_GRID:           return "images";
        case KIND_AUDIO_PLAYER:         return "audio";
        case KIND_ENVIRONMENT_PREVIEW:  return "environment";
        default:                        return "summary";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackPresentation::GetSourceDescription( void ) const
{GUCEF_TRACE;

    if ( cooked )
        return CCookedOutputResolver::GetCookedLabel();
    if ( KIND_EFFECT_FOUR_VIEW == kind )
        return "cooked for Ogre in the cache folder by the " + converter + " converter and the Ogre cooker";
    const CORE::CString note = cookedNote.IsNULLOrEmpty() ? CORE::CString::Empty : "; " + cookedNote;
    if ( !converter.IsNULLOrEmpty() )
        return "quick look: converted to glTF in the cache folder by the " + converter + " converter and loaded through the Assimp codec, not the cooked gucef-ogre3d output" + note;
    return "quick look: the original files loaded through the Assimp codec, not converted" + note;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackPresentation::GetKindName( EKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case KIND_PORTRAIT:             return "Portrait";
        case KIND_EFFECT_FOUR_VIEW:     return "Effect in four views";
        case KIND_SCENE_DEMO:           return "Pack scene";
        case KIND_MODEL_GRID:           return "Showcase grid";
        case KIND_ANIMATION_PLAYER:     return "Animation player";
        case KIND_MATERIAL_PREVIEW:     return "Material preview";
        case KIND_IMAGE_GRID:           return "Image grid";
        case KIND_AUDIO_PLAYER:         return "Audio player";
        case KIND_ENVIRONMENT_PREVIEW:  return "Environment preview";
        default:                        return "Summary";
    }
}

/*-------------------------------------------------------------------------*/

bool
CPackPresentation::TryParseKindId( const CORE::CString& id, EKind& kind )
{GUCEF_TRACE;

    for ( Int32 i=0; i<(Int32) KIND_COUNT; ++i )
    {
        if ( GetKindId( (EKind) i ) == id )
        {
            kind = (EKind) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

namespace {

Float32
ReadFloat( const CORE::CDataNode& node, const CORE::CString& name, Float32 defaultValue )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsFloat32( defaultValue, true );
}

bool
ReadVector( const CORE::CDataNode* node, Float32 vector[ 3 ] )
{
    if ( GUCEF_NULL == node )
        return false;
    vector[ 0 ] = ReadFloat( *node, "x", 0.0f );
    vector[ 1 ] = ReadFloat( *node, "y", 0.0f );
    vector[ 2 ] = ReadFloat( *node, "z", 0.0f );
    return true;
}

void
ReadLight( const CORE::CDataNode* lights, const char* name, CPackPresentation::SLightHint& light )
{
    const CORE::CDataNode* node = GUCEF_NULL == lights ? GUCEF_NULL : lights->FindChild( name );
    if ( GUCEF_NULL == node )
        return;
    light.yawDegrees = ReadFloat( *node, "yawDegrees", light.yawDegrees );
    light.pitchDegrees = ReadFloat( *node, "pitchDegrees", light.pitchDegrees );
    light.intensity = ReadFloat( *node, "intensity", light.intensity );
}

}

/*-------------------------------------------------------------------------*/

bool
CPackPresentation::ReadCharacterHints( const CORE::CDataNode& sidecar )
{GUCEF_TRACE;

    const CORE::CDataNode* extras = sidecar.FindChild( "extras" );
    const CORE::CDataNode* hints = GUCEF_NULL == extras ? GUCEF_NULL : extras->FindChild( "gucefPresentation" );
    if ( GUCEF_NULL == hints || "character" != hints->GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true ) )
        return false;
    const CORE::CDataNode* views = hints->FindChild( "views" );
    const CORE::CDataNode* portrait = GUCEF_NULL == views ? GUCEF_NULL : views->FindChild( "portrait" );
    if ( GUCEF_NULL == portrait )
        return false;
    SCameraHint hint;
    hint.fovDegrees = ReadFloat( *portrait, "fovDegrees", 30.0f );
    hint.yawDegrees = ReadFloat( *portrait, "yawDegrees", 30.0f );
    hint.pitchDegrees = ReadFloat( *portrait, "pitchDegrees", 3.0f );
    hint.distance = ReadFloat( *portrait, "distance", 1.0f );
    if ( !ReadVector( portrait->FindChild( "target" ), hint.target ) || !ReadVector( portrait->FindChild( "position" ), hint.eye ) )
        return false;
    hint.valid = hint.distance > 1e-5f;
    camera = hint;
    const CORE::CDataNode* lights = hints->FindChild( "lights" );
    ReadLight( lights, "key", keyLight );
    ReadLight( lights, "fill", fillLight );
    ReadLight( lights, "rim", rimLight );
    return camera.valid;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
