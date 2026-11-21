/*
 *  assetpluginUNREAL: The presentation hints of a character: the framing of a 3D portrait, the lights, the full body view
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H
#include "assetpluginUNREAL_CUnrealPresentationHints.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "unreal";
static const Float32 Pi = 3.14159265358979f;
static const Float32 DegreesToRadians = Pi / 180.0f;

const char* const CUnrealPresentationHints::ExtrasKey = "gucefPresentation";
const char* const CUnrealPresentationHints::CharacterKey = "gucefCharacter";
const Float32 CUnrealPresentationHints::PortraitFovDegrees = 30.0f;
const Float32 CUnrealPresentationHints::PortraitYawDegrees = 30.0f;
const Float32 CUnrealPresentationHints::PortraitPitchDegrees = 3.0f;

/*-------------------------------------------------------------------------*/

CUnrealPresentationHints::SVector::SVector( void )
{GUCEF_TRACE;

    v[ 0 ] = v[ 1 ] = v[ 2 ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

CUnrealPresentationHints::SVector::SVector( Float32 x, Float32 y, Float32 z )
{GUCEF_TRACE;

    v[ 0 ] = x;
    v[ 1 ] = y;
    v[ 2 ] = z;
}

/*-------------------------------------------------------------------------*/

CUnrealPresentationHints::SHints::SHints( void )
    : up( 0.0f, 1.0f, 0.0f )
    , facing( 0.0f, 0.0f, 1.0f )
    , side( 1.0f, 0.0f, 0.0f )
    , boundsMin()
    , boundsMax()
    , headMin()
    , headMax()
    , headJoint()
    , hasHeadJoint( false )
    , headEstimated( true )
    , portrait()
    , fullBody()
    , turntableCentre()
    , turntableRadius( 0.0f )
    , key()
    , fill()
    , rim()
{GUCEF_TRACE;

    memset( &portrait, 0, sizeof( portrait ) );
    memset( &fullBody, 0, sizeof( fullBody ) );
    memset( &key, 0, sizeof( key ) );
    memset( &fill, 0, sizeof( fill ) );
    memset( &rim, 0, sizeof( rim ) );
}

/*-------------------------------------------------------------------------*/

/** The direction of a yaw about up from the facing direction toward the side, raised by the pitch */
static CUnrealPresentationHints::SVector
Direction( const CUnrealPresentationHints::SHints& hints, Float32 yawDegrees, Float32 pitchDegrees )
{GUCEF_TRACE;

    const Float32 yaw = yawDegrees * DegreesToRadians;
    const Float32 pitch = pitchDegrees * DegreesToRadians;
    CUnrealPresentationHints::SVector result;
    for ( UInt32 c=0; c<3; ++c )
        result.v[ c ] = cosf( pitch ) * ( cosf( yaw ) * hints.facing.v[ c ] + sinf( yaw ) * hints.side.v[ c ] ) + sinf( pitch ) * hints.up.v[ c ];
    return result;
}

/*-------------------------------------------------------------------------*/

static void
SetView( const CUnrealPresentationHints::SHints& hints, const CUnrealPresentationHints::SVector& target, Float32 frameHeight, Float32 marginFactor, Float32 yawDegrees, Float32 pitchDegrees, CUnrealPresentationHints::SView& view )
{GUCEF_TRACE;

    view.target = target;
    view.fovDegrees = CUnrealPresentationHints::PortraitFovDegrees;
    view.yawDegrees = yawDegrees;
    view.pitchDegrees = pitchDegrees;
    view.frameHeight = frameHeight;
    view.distance = marginFactor * ( frameHeight * 0.5f ) / tanf( 0.5f * view.fovDegrees * DegreesToRadians );
    const CUnrealPresentationHints::SVector direction = Direction( hints, yawDegrees, pitchDegrees );
    for ( UInt32 c=0; c<3; ++c )
        view.position.v[ c ] = target.v[ c ] + view.distance * direction.v[ c ];
}

/*-------------------------------------------------------------------------*/

static void
SetLight( const CUnrealPresentationHints::SHints& hints, Float32 yawDegrees, Float32 pitchDegrees, Float32 intensity, CUnrealPresentationHints::SLight& light )
{GUCEF_TRACE;

    light.yawDegrees = yawDegrees;
    light.pitchDegrees = pitchDegrees;
    light.intensity = intensity;
    light.towardLight = Direction( hints, yawDegrees, pitchDegrees );
}

/*-------------------------------------------------------------------------*/

void
CUnrealPresentationHints::Compute( const SVector& boundsMin, const SVector& boundsMax, const SVector& headMin, const SVector& headMax, const SVector& facing,
                                   bool hasLightRotation, Float32 lightRotation, SHints& hints )
{GUCEF_TRACE;

    hints.up = SVector( 0.0f, 1.0f, 0.0f );
    hints.boundsMin = boundsMin;
    hints.boundsMax = boundsMax;
    hints.headMin = headMin;
    hints.headMax = headMax;

    // the facing direction is horizontal
    Float32 fx = facing.v[ 0 ];
    Float32 fz = facing.v[ 2 ];
    const Float32 length = sqrtf( fx * fx + fz * fz );
    if ( length < 1e-6f )
    {
        fx = 0.0f;
        fz = 1.0f;
    }
    else
    {
        fx /= length;
        fz /= length;
    }
    hints.facing = SVector( fx, 0.0f, fz );
    hints.side = SVector( fz, 0.0f, -fx );          // cross( up, facing )

    // the portrait: crown and a little air above it down to the shoulders, which is twice the height of the head
    const Float32 headHeight = headMax.v[ 1 ] - headMin.v[ 1 ];
    const Float32 headTop = headMax.v[ 1 ];
    const SVector headCentre( 0.5f * ( headMin.v[ 0 ] + headMax.v[ 0 ] ), 0.0f, 0.5f * ( headMin.v[ 2 ] + headMax.v[ 2 ] ) );
    const SVector portraitTarget( headCentre.v[ 0 ], headTop - 0.8f * headHeight, headCentre.v[ 2 ] );
    SetView( hints, portraitTarget, 2.0f * headHeight, 1.0f, PortraitYawDegrees, PortraitPitchDegrees, hints.portrait );

    // the full body: the whole bounds with a margin
    const Float32 bodyHeight = boundsMax.v[ 1 ] - boundsMin.v[ 1 ];
    const SVector bodyCentre( 0.5f * ( boundsMin.v[ 0 ] + boundsMax.v[ 0 ] ), 0.5f * ( boundsMin.v[ 1 ] + boundsMax.v[ 1 ] ), 0.5f * ( boundsMin.v[ 2 ] + boundsMax.v[ 2 ] ) );
    SetView( hints, bodyCentre, 1.1f * bodyHeight, 1.05f, PortraitYawDegrees, 0.0f, hints.fullBody );

    hints.turntableCentre = SVector( bodyCentre.v[ 0 ], boundsMin.v[ 1 ], bodyCentre.v[ 2 ] );
    hints.turntableRadius = hints.fullBody.distance;

    // the lights: a key from the side of the camera and above (45 degrees without a light rotation), a weaker fill from the other side and a rim from behind
    // The light rotation of the engine turns the other way than the yaw here (the engine is left handed): its sign is flipped.
    // Assumption, to be checked against the preview of the MetaHuman editor: the sample value -47.5 then puts the key on the side of the camera.
    const Float32 keyYaw = hasLightRotation ? -lightRotation : 45.0f;
    SetLight( hints, keyYaw, 35.0f, 1.0f, hints.key );
    SetLight( hints, -keyYaw, 10.0f, 0.35f, hints.fill );
    SetLight( hints, 180.0f + 0.5f * keyYaw, 30.0f, 0.5f, hints.rim );
}

/*-------------------------------------------------------------------------*/

namespace {

/** v' = scale * P v with the signed permutation of the conversion between two coordinate systems */
struct SConversion
{
    Int32 p[ 9 ];       /**< column-major */
    Float32 scale;

    SConversion( const MATH::CCoordinateSystem& source, const MATH::CCoordinateSystem& target )
    {
        source.GetConversionPermutation( target, p );
        scale = (Float32) source.GetConversionScale( target );
    }

    CUnrealPresentationHints::SVector Apply( const Float32* in, Float32 factor ) const
    {
        CUnrealPresentationHints::SVector out;
        for ( UInt32 r=0; r<3; ++r )
            out.v[ r ] = factor * (Float32) ( p[ r ] * in[ 0 ] + p[ 3 + r ] * in[ 1 ] + p[ 6 + r ] * in[ 2 ] );
        return out;
    }
};

/** A box that grows with points */
struct SBox
{
    Float32 low[ 3 ];
    Float32 high[ 3 ];
    UInt32 count;

    SBox( void ) : count( 0 ) { low[ 0 ] = low[ 1 ] = low[ 2 ] = 0.0f; high[ 0 ] = high[ 1 ] = high[ 2 ] = 0.0f; }

    void Add( const Float32* point )
    {
        if ( 0 == count )
        {
            for ( UInt32 c=0; c<3; ++c ) { low[ c ] = high[ c ] = point[ c ]; }
        }
        else
        {
            for ( UInt32 c=0; c<3; ++c )
            {
                if ( point[ c ] < low[ c ] ) low[ c ] = point[ c ];
                if ( point[ c ] > high[ c ] ) high[ c ] = point[ c ];
            }
        }
        ++count;
    }
};

/** The corners of a box of the source converted to the target: the box of the converted corners */
void
ConvertBox( const SConversion& conversion, const SBox& box, CUnrealPresentationHints::SVector& low, CUnrealPresentationHints::SVector& high )
{
    const CUnrealPresentationHints::SVector a = conversion.Apply( box.low, conversion.scale );
    const CUnrealPresentationHints::SVector b = conversion.Apply( box.high, conversion.scale );
    for ( UInt32 c=0; c<3; ++c )
    {
        low.v[ c ] = a.v[ c ] < b.v[ c ] ? a.v[ c ] : b.v[ c ];
        high.v[ c ] = a.v[ c ] < b.v[ c ] ? b.v[ c ] : a.v[ c ];
    }
}

}

/*-------------------------------------------------------------------------*/

bool
CUnrealPresentationHints::Build( const ASSETCONVERTER::CAssetScene& scene                          ,
                                 const CUnrealCharacterAssembler::SResult& assembly                ,
                                 const CUnrealMetaHumanCharacter* character                        ,
                                 const CORE::CString& characterName                                ,
                                 ASSETCONVERTER::CConversionReport& report                         ,
                                 SHints& hints                                                     )
{GUCEF_TRACE;

    hints = SHints();
    if ( assembly.skinIndex >= scene.skins.size() || assembly.bodyMeshFirst >= scene.meshes.size() )
        return false;
    const ASSETCONVERTER::CAssetSkin& skin = scene.skins[ assembly.skinIndex ];

    // the head joint: by name
    Int32 headJoint = -1;
    for ( size_t j=0; j<skin.joints.size() && headJoint < 0; ++j )
        if ( scene.nodes[ skin.joints[ j ] ].name == "head" )
            headJoint = (Int32) j;
    GUCEF::vector< bool > belowHead( skin.joints.size(), false );
    if ( headJoint >= 0 )
        for ( size_t j=0; j<skin.joints.size(); ++j )
            belowHead[ j ] = scene.IsInSubtree( skin.joints[ j ], skin.joints[ (size_t) headJoint ] );

    // the vertices of the first LOD of the body and the garments: the bounds, and for the body the head
    GUCEF::vector< UInt32 > lodMeshes;
    lodMeshes.push_back( assembly.bodyMeshFirst );
    for ( size_t g=0; g<assembly.garmentMeshFirst.size(); ++g )
        if ( g < assembly.garmentHolderNodes.size() && 0 != assembly.garmentHolderNodes[ g ] && assembly.garmentMeshFirst[ g ] < scene.meshes.size() )
            lodMeshes.push_back( assembly.garmentMeshFirst[ g ] );
    SBox bounds;
    SBox head;
    for ( size_t m=0; m<lodMeshes.size(); ++m )
    {
        const ASSETCONVERTER::CAssetMesh& mesh = scene.meshes[ lodMeshes[ m ] ];
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        {
            const ASSETCONVERTER::CAssetSubMesh& sub = mesh.subMeshes[ s ];
            const size_t vertices = sub.positions.size() / 3;
            const UInt32 influences = sub.influencesPerVertex;
            for ( size_t v=0; v<vertices; ++v )
            {
                bounds.Add( &sub.positions[ v * 3 ] );
                if ( 0 != m || headJoint < 0 || 0 == influences || sub.joints.size() < ( v + 1 ) * influences )
                    continue;
                UInt32 strongest = 0;
                for ( UInt32 i=1; i<influences; ++i )
                    if ( sub.weights[ v * influences + i ] > sub.weights[ v * influences + strongest ] )
                        strongest = i;
                const UInt32 joint = sub.joints[ v * influences + strongest ];
                if ( sub.weights[ v * influences + strongest ] > 0.0f && joint < belowHead.size() && belowHead[ joint ] )
                    head.Add( &sub.positions[ v * 3 ] );
            }
        }
    }
    if ( 0 == bounds.count )
        return false;

    const MATH::CCoordinateSystem target = MATH::CCoordinateSystem::GucefDefault();
    const SConversion conversion( scene.coordinateSystem, target );
    SVector boundsMin;
    SVector boundsMax;
    ConvertBox( conversion, bounds, boundsMin, boundsMax );

    // the head: its vertices, or the top of the bounds when the skeleton or the weights do not give them
    static const UInt32 MinimumHeadVertices = 100;
    SVector headMin;
    SVector headMax;
    bool estimated = false;
    if ( head.count >= MinimumHeadVertices )
        ConvertBox( conversion, head, headMin, headMax );
    else
    {
        estimated = true;
        headMin = boundsMin;
        headMax = boundsMax;
        headMin.v[ 1 ] = boundsMax.v[ 1 ] - 0.15f * ( boundsMax.v[ 1 ] - boundsMin.v[ 1 ] );
        report.AddApproximation( SourceName, characterName, "the head bounds are the top 15 percent of the character: " + CORE::CString( headJoint < 0 ? "the skeleton has no joint named head" : "too few vertices are weighted to the head joint" ) );
    }

    // the direction the character faces: the forward axis of the source, which the mesh converter states for a skeletal mesh
    const MATH::CTVector3D< Float32 > forward = scene.coordinateSystem.GetForward< Float32 >();
    const Float32 forwardInSource[ 3 ] = { forward.x, forward.y, forward.z };
    const SVector facing = conversion.Apply( forwardInSource, 1.0f );

    Compute( boundsMin, boundsMax, headMin, headMax, facing, GUCEF_NULL != character && character->hasLightRotation, GUCEF_NULL != character ? character->lightRotation : 0.0f, hints );
    hints.headEstimated = estimated;
    if ( headJoint >= 0 )
    {
        const MATH::TFloat32Matrix4D world = scene.GetWorldMatrix( skin.joints[ (size_t) headJoint ] );
        const Float32 position[ 3 ] = { world.GetTranslation().x, world.GetTranslation().y, world.GetTranslation().z };
        hints.headJoint = conversion.Apply( position, conversion.scale );
        hints.hasHeadJoint = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
AddVector( CORE::CDataNode& parent, const char* name, const CUnrealPresentationHints::SVector& vector )
{GUCEF_TRACE;

    CORE::CDataNode* node = parent.AddChild( name );
    node->SetAttribute( "x", vector.v[ 0 ] );
    node->SetAttribute( "y", vector.v[ 1 ] );
    node->SetAttribute( "z", vector.v[ 2 ] );
}

/*-------------------------------------------------------------------------*/

static void
AddView( CORE::CDataNode& parent, const char* name, const char* frame, const CUnrealPresentationHints::SView& view )
{GUCEF_TRACE;

    CORE::CDataNode* node = parent.AddChild( name );
    node->SetAttribute( "frame", frame );
    node->SetAttribute( "fovDegrees", view.fovDegrees );
    node->SetAttribute( "yawDegrees", view.yawDegrees );
    node->SetAttribute( "pitchDegrees", view.pitchDegrees );
    node->SetAttribute( "distance", view.distance );
    node->SetAttribute( "frameHeight", view.frameHeight );
    AddVector( *node, "target", view.target );
    AddVector( *node, "position", view.position );
}

/*-------------------------------------------------------------------------*/

static void
AddLight( CORE::CDataNode& parent, const char* name, const CUnrealPresentationHints::SLight& light )
{GUCEF_TRACE;

    CORE::CDataNode* node = parent.AddChild( name );
    node->SetAttribute( "yawDegrees", light.yawDegrees );
    node->SetAttribute( "pitchDegrees", light.pitchDegrees );
    node->SetAttribute( "intensity", light.intensity );
    AddVector( *node, "towardLight", light.towardLight );
}

/*-------------------------------------------------------------------------*/

void
CUnrealPresentationHints::ToDataNode( const SHints& hints, const CUnrealMetaHumanCharacter* character, const CORE::CString& characterName, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetAttribute( "schema", "gucefPresentation/1" );
    node.SetAttribute( "kind", "character" );
    node.SetAttribute( "defaultView", "portrait" );
    node.SetAttribute( "name", characterName );
    node.SetAttribute( "unit", "m" );
    AddVector( node, "up", hints.up );
    AddVector( node, "facing", hints.facing );
    AddVector( node, "side", hints.side );

    CORE::CDataNode* bounds = node.AddChild( "bounds" );
    bounds->SetAttribute( "height", hints.boundsMax.v[ 1 ] - hints.boundsMin.v[ 1 ] );
    AddVector( *bounds, "min", hints.boundsMin );
    AddVector( *bounds, "max", hints.boundsMax );

    CORE::CDataNode* head = node.AddChild( "head" );
    head->SetAttribute( "joint", "head" );
    head->SetAttribute( "hasJoint", hints.hasHeadJoint );
    head->SetAttribute( "estimated", hints.headEstimated );
    head->SetAttribute( "height", hints.headMax.v[ 1 ] - hints.headMin.v[ 1 ] );
    if ( hints.hasHeadJoint )
        AddVector( *head, "jointPosition", hints.headJoint );
    AddVector( *head, "min", hints.headMin );
    AddVector( *head, "max", hints.headMax );

    CORE::CDataNode* views = node.AddChild( "views" );
    AddView( *views, "portrait", "headAndShoulders", hints.portrait );
    AddView( *views, "fullBody", "fullBody", hints.fullBody );
    CORE::CDataNode* turntable = views->AddChild( "turntable" );
    turntable->SetAttribute( "radius", hints.turntableRadius );
    turntable->SetAttribute( "degreesPerSecond", 20.0f );
    AddVector( *turntable, "axis", hints.up );
    AddVector( *turntable, "centre", hints.turntableCentre );

    CORE::CDataNode* lights = node.AddChild( "lights" );
    lights->SetAttribute( "preset", "neutralStudio" );
    AddLight( *lights, "key", hints.key );
    AddLight( *lights, "fill", hints.fill );
    AddLight( *lights, "rim", hints.rim );

    // what of the character asset was used, as the asset states it
    CORE::CDataNode* source = node.AddChild( "source" );
    if ( GUCEF_NULL != character )
    {
        source->SetAttribute( "cameraFrame", character->cameraFrame );
        source->SetAttribute( "characterEnvironment", character->characterEnvironment );
        source->SetAttribute( "hasLightRotation", character->hasLightRotation );
        if ( character->hasLightRotation )
            source->SetAttribute( "lightRotation", character->lightRotation );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
