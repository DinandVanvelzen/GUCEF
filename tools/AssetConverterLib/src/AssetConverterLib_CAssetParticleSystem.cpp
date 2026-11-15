/*
 *  AssetConverterLib: an engine-neutral particle system of the intermediate model
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

#include "AssetConverterLib_CAssetParticleSystem.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
SetVector( CORE::CDataNode& node, const CORE::CString& name, const Float32* values, UInt32 count )
{
    CORE::CDataNode::TVariantVector list;
    for ( UInt32 i=0; i<count; ++i )
        list.push_back( CORE::CVariant( values[ i ] ) );
    node.SetValuesOfChildByName( name, list );
}

void
GetVector( const CORE::CDataNode& node, const CORE::CString& name, Float32* values, UInt32 count )
{
    CORE::CDataNode::TVariantVector list;
    if ( !node.GetValuesOfChildByName( name, list ) )
        return;
    for ( UInt32 i=0; i<count && i<list.size(); ++i )
        values[ i ] = list[ i ].AsFloat32( values[ i ] );
}

void
SetCurve( CORE::CDataNode& node, const CORE::CString& name, const CAssetParticleCurve& curve )
{
    curve.Serialize( *node.AddChild( name, GUCEF_DATATYPE_OBJECT ) );
}

void
GetCurve( const CORE::CDataNode& node, const CORE::CString& name, CAssetParticleCurve& curve )
{
    const CORE::CDataNode* child = node.FindChild( name );
    if ( GUCEF_NULL != child )
        curve.Deserialize( *child );
}

void
SetGradient( CORE::CDataNode& node, const CORE::CString& name, const CAssetParticleGradient& gradient )
{
    gradient.Serialize( *node.AddChild( name, GUCEF_DATATYPE_OBJECT ) );
}

void
GetGradient( const CORE::CDataNode& node, const CORE::CString& name, CAssetParticleGradient& gradient )
{
    const CORE::CDataNode* child = node.FindChild( name );
    if ( GUCEF_NULL != child )
        gradient.Deserialize( *child );
}

void
SetVectorCurves( CORE::CDataNode& parent, const CORE::CString& name, const CAssetParticleSystem::SVectorCurves& curves )
{
    CORE::CDataNode* node = parent.AddChild( name, GUCEF_DATATYPE_OBJECT );
    node->SetAttribute( "enabled", curves.enabled );
    node->SetAttribute( "space", ToString( curves.space ) );
    SetCurve( *node, "x", curves.x );
    SetCurve( *node, "y", curves.y );
    SetCurve( *node, "z", curves.z );
}

void
GetVectorCurves( const CORE::CDataNode& parent, const CORE::CString& name, CAssetParticleSystem::SVectorCurves& curves )
{
    const CORE::CDataNode* node = parent.FindChild( name );
    if ( GUCEF_NULL == node )
        return;
    curves.enabled = node->GetAttributeValueOrChildValueByName( "enabled" ).AsBool( false );
    FromString( node->GetAttributeValueOrChildValueByName( "space" ).AsString(), curves.space );
    GetCurve( *node, "x", curves.x );
    GetCurve( *node, "y", curves.y );
    GetCurve( *node, "z", curves.z );
}

Float32
GetFloat( const CORE::CDataNode& node, const char* name, Float32 fallback )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsFloat32( fallback );
}

bool
GetBool( const CORE::CDataNode& node, const char* name, bool fallback )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsBool( fallback );
}

/** v' = factor * P v */
void
Apply( const Float32 p[ 9 ], Float32* v, Float32 factor )
{
    const Float32 x = v[ 0 ], y = v[ 1 ], z = v[ 2 ];
    for ( UInt32 r=0; r<3; ++r )
        v[ r ] = factor * ( p[ r ] * x + p[ 3 + r ] * y + p[ 6 + r ] * z );
}

/** The curves of the axes moved by the signed permutation, scaled */
void
ApplyCurves( const Float32 p[ 9 ], CAssetParticleSystem::SVectorCurves& curves, Float32 factor )
{
    CAssetParticleCurve source[ 3 ] = { curves.x, curves.y, curves.z };
    CAssetParticleCurve* target[ 3 ] = { &curves.x, &curves.y, &curves.z };
    for ( UInt32 r=0; r<3; ++r )
    {
        for ( UInt32 c=0; c<3; ++c )
        {
            const Float32 sign = p[ 3 * c + r ];
            if ( 0.0f == sign )
                continue;
            *target[ r ] = source[ c ];
            target[ r ]->Scale( sign * factor );
        }
    }
}

/** a * b for quaternions x, y, z, w */
void
Multiply( const Float32 a[ 4 ], const Float32 b[ 4 ], Float32 out[ 4 ] )
{
    out[ 0 ] = a[ 3 ] * b[ 0 ] + a[ 0 ] * b[ 3 ] + a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
    out[ 1 ] = a[ 3 ] * b[ 1 ] - a[ 0 ] * b[ 2 ] + a[ 1 ] * b[ 3 ] + a[ 2 ] * b[ 0 ];
    out[ 2 ] = a[ 3 ] * b[ 2 ] + a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ] + a[ 2 ] * b[ 3 ];
    out[ 3 ] = a[ 3 ] * b[ 3 ] - a[ 0 ] * b[ 0 ] - a[ 1 ] * b[ 1 ] - a[ 2 ] * b[ 2 ];
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetParticleSystem::SBurst::SBurst( void )
    : time( 0.0f )
    , count( 0.0f )
    , cycles( 1 )
    , interval( 0.01f )
    , probability( 1.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::SShape::SShape( void )
    : type( PARTICLESHAPE_POINT )
    , radius( 1.0f )
    , radiusThickness( 1.0f )
    , angle( 25.0f )
    , arc( 360.0f )
    , length( 5.0f )
    , ringRadius( 0.2f )
    , ringHeight( 0.0f )
    , ringInnerRadius( 0.0f )
    , mesh( -1 )
    , directionMode( PARTICLEDIRECTION_FIXED )
    , spreadAngle( 0.0f )
    , randomDirectionAmount( 0.0f )
{GUCEF_TRACE;

    position[ 0 ] = position[ 1 ] = position[ 2 ] = 0.0f;
    rotation[ 0 ] = rotation[ 1 ] = rotation[ 2 ] = 0.0f;
    rotation[ 3 ] = 1.0f;
    scale[ 0 ] = scale[ 1 ] = scale[ 2 ] = 1.0f;
    boxSize[ 0 ] = boxSize[ 1 ] = boxSize[ 2 ] = 1.0f;
    direction[ 0 ] = 0.0f; direction[ 1 ] = 1.0f; direction[ 2 ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleSystem::SShape::GetAxis( Float32 axis[ 3 ] ) const
{GUCEF_TRACE;

    // v' = q v q* for v = (0, 1, 0)
    const Float32 x = rotation[ 0 ], y = rotation[ 1 ], z = rotation[ 2 ], w = rotation[ 3 ];
    axis[ 0 ] = 2.0f * ( x * y - w * z );
    axis[ 1 ] = 1.0f - 2.0f * ( x * x + z * z );
    axis[ 2 ] = 2.0f * ( y * z + w * x );
}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::SVectorCurves::SVectorCurves( void )
    : enabled( false )
    , x()
    , y()
    , z()
    , space( PARTICLESPACE_LOCAL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleSystem::SVectorCurves::IsConstant( void ) const
{GUCEF_TRACE;

    return !enabled || ( x.IsConstant() && y.IsConstant() && z.IsConstant() );
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleSystem::SVectorCurves::GetAverage( Float32 out[ 3 ] ) const
{GUCEF_TRACE;

    out[ 0 ] = enabled ? x.GetAverage() : 0.0f;
    out[ 1 ] = enabled ? y.GetAverage() : 0.0f;
    out[ 2 ] = enabled ? z.GetAverage() : 0.0f;
}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::SNoise::SNoise( void )
    : enabled( false )
    , strength( 1.0f )
    , frequency( 0.5f )
    , scrollSpeed( 0.0f )
    , octaves( 1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::SPlane::SPlane( void )
{GUCEF_TRACE;

    point[ 0 ] = point[ 1 ] = point[ 2 ] = 0.0f;
    normal[ 0 ] = 0.0f; normal[ 1 ] = 1.0f; normal[ 2 ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::SCollision::SCollision( void )
    : enabled( false )
    , world( false )
    , planes()
    , bounce( 1.0f )
    , dampen( 0.0f )
    , lifetimeLoss( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::STextureSheet::STextureSheet( void )
    : enabled( false )
    , tilesX( 1 )
    , tilesY( 1 )
    , singleRow( false )
    , rowIndex( 0 )
    , frameOverTime()
    , startFrame( 0.0f )
    , cycles( 1.0f )
    , fps( 0.0f )
{GUCEF_TRACE;

    CAssetParticleCurve::TKeyVector keys;
    keys.push_back( CAssetParticleCurve::SKey( 0.0f, 0.0f, 1.0f, 1.0f ) );
    keys.push_back( CAssetParticleCurve::SKey( 1.0f, 1.0f, 1.0f, 1.0f ) );
    frameOverTime = CAssetParticleCurve::Curve( keys );
}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::SRenderer::SRenderer( void )
    : mode( PARTICLERENDER_BILLBOARD )
    , alignment( PARTICLEALIGNMENT_VIEW )
    , material( -1 )
    , trailMaterial( -1 )
    , mesh( -1 )
    , lengthScale( 1.0f )
    , velocityScale( 0.0f )
    , blendMode( PARTICLEBLEND_ALPHA )
    , alphaCutoff( 0.0f )
    , useParticleColour( true )
    , lit( false )
    , softParticles( false )
    , sortMode( PARTICLESORT_NONE )
{GUCEF_TRACE;

    quadSize[ 0 ] = quadSize[ 1 ] = 1.0f;
    pivot[ 0 ] = pivot[ 1 ] = pivot[ 2 ] = 0.0f;
    tint[ 0 ] = tint[ 1 ] = tint[ 2 ] = tint[ 3 ] = 1.0f;
}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::SSubEmitter::SSubEmitter( void )
    : system( -1 )
    , trigger( PARTICLETRIGGER_BIRTH )
    , probability( 1.0f )
    , inherit( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::STrails::STrails( void )
    : enabled( false )
    , lifetime( 1.0f )
    , widthOverTrail( 1.0f )
    , colourOverTrail()
    , minVertexDistance( 0.2f )
    , worldSpace( false )
    , inheritColour( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleSystem::CAssetParticleSystem( void )
    : name()
    , duration( 5.0f )
    , looping( true )
    , playOnAwake( true )
    , prewarm( 0.0f )
    , startDelay( 0.0f )
    , simulationSpeed( 1.0f )
    , simulationSpace( PARTICLESPACE_LOCAL )
    , maxParticles( 1000 )
    , hasBounds( false )
    , rateOverTime( 10.0f )
    , rateOverDistance( 0.0f )
    , bursts()
    , explosiveness( 0.0f )
    , emissionRandomness( 0.0f )
    , startLifetime( 5.0f )
    , startSpeed( 5.0f )
    , startSize( 1.0f )
    , startSize3D( false )
    , startSizeY( 1.0f )
    , startSizeZ( 1.0f )
    , startRotation( 0.0f )
    , startRotation3D( false )
    , startColour()
    , shape()
    , velocity()
    , speedModifier( 1.0f )
    , force()
    , linearAcceleration( 0.0f )
    , radialAcceleration( 0.0f )
    , tangentialAcceleration( 0.0f )
    , damping( 0.0f )
    , limitSpeed( false )
    , speedLimit( 1.0f )
    , speedLimitDampen( 0.0f )
    , drag( 0.0f )
    , sizeOverLifetimeEnabled( false )
    , sizeOverLifetime( 1.0f )
    , sizeOverLifetimeSeparate( false )
    , sizeOverLifetimeY( 1.0f )
    , sizeOverLifetimeZ( 1.0f )
    , angularVelocity( 0.0f )
    , colourOverLifetimeEnabled( false )
    , colourOverLifetime()
    , alphaOverLifetimeEnabled( false )
    , alphaOverLifetime( 1.0f )
    , emissionOverLifetimeEnabled( false )
    , emissionOverLifetime( 0.0f )
    , noise()
    , collision()
    , textureSheet()
    , renderer()
    , subEmitters()
    , trails()
    , source( "source", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

    boundsMin[ 0 ] = boundsMin[ 1 ] = boundsMin[ 2 ] = 0.0f;
    boundsMax[ 0 ] = boundsMax[ 1 ] = boundsMax[ 2 ] = 0.0f;
    gravity[ 0 ] = gravity[ 1 ] = gravity[ 2 ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleSystem::ConvertSpace( const Float32 p[ 9 ], Float32 scale, Float32 determinant )
{GUCEF_TRACE;

    Apply( p, shape.position, scale );
    Apply( p, shape.direction, 1.0f );
    Apply( p, gravity, scale );
    if ( hasBounds )
    {
        Apply( p, boundsMin, scale );
        Apply( p, boundsMax, scale );
        for ( UInt32 i=0; i<3; ++i )
        {
            if ( boundsMin[ i ] > boundsMax[ i ] )
            {
                const Float32 swap = boundsMin[ i ];
                boundsMin[ i ] = boundsMax[ i ];
                boundsMax[ i ] = swap;
            }
        }
    }
    for ( size_t i=0; i<collision.planes.size(); ++i )
    {
        Apply( p, collision.planes[ i ].point, scale );
        Apply( p, collision.planes[ i ].normal, 1.0f );
    }
    ApplyCurves( p, velocity, scale );
    ApplyCurves( p, force, scale );

    // the rotation of the shape as a rotation of the target: P R P^T is (det(P) P v, w); then the canonical axis, which
    // the conversion moves to P (0, 1, 0), is turned back onto +Y
    Apply( p, shape.rotation, determinant );
    const Float32 axis[ 3 ] = { p[ 3 ], p[ 4 ], p[ 5 ] };
    const Float32 h = 0.70710678f;
    Float32 back[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
    if ( axis[ 1 ] < -0.5f )      { back[ 0 ] = 1.0f; back[ 3 ] = 0.0f; }
    else if ( axis[ 0 ] > 0.5f )  { back[ 2 ] = -h; back[ 3 ] = h; }
    else if ( axis[ 0 ] < -0.5f ) { back[ 2 ] = h; back[ 3 ] = h; }
    else if ( axis[ 2 ] > 0.5f )  { back[ 0 ] = h; back[ 3 ] = h; }
    else if ( axis[ 2 ] < -0.5f ) { back[ 0 ] = -h; back[ 3 ] = h; }
    Float32 turned[ 4 ];
    Multiply( shape.rotation, back, turned );
    for ( UInt32 i=0; i<4; ++i )
        shape.rotation[ i ] = turned[ i ];

    // distances, speeds and accelerations follow the unit scale; the box is in the frame of the shape
    shape.radius *= scale;
    shape.length *= scale;
    shape.ringRadius *= scale;
    shape.ringHeight *= scale;
    shape.ringInnerRadius *= scale;
    for ( UInt32 i=0; i<3; ++i )
        shape.boxSize[ i ] *= scale;
    startSize.Scale( scale );
    startSizeY.Scale( scale );
    startSizeZ.Scale( scale );
    startSpeed.Scale( scale );
    linearAcceleration.Scale( scale );
    radialAcceleration.Scale( scale );
    tangentialAcceleration.Scale( scale );
    damping.Scale( scale );
    speedLimit.Scale( scale );
    noise.strength.Scale( scale );
    if ( scale != 0.0f )
        rateOverDistance.Scale( 1.0f / scale );
    trails.minVertexDistance *= scale;
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleSystem::IsSubEmitter( const GUCEF::vector< CAssetParticleSystem >& systems, UInt32 index )
{GUCEF_TRACE;

    for ( size_t s=0; s<systems.size(); ++s )
        for ( size_t e=0; e<systems[ s ].subEmitters.size(); ++e )
            if ( systems[ s ].subEmitters[ e ].system == (Int32) index )
                return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleSystem::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "name", name );
    node.SetAttribute( "duration", duration );
    node.SetAttribute( "looping", looping );
    node.SetAttribute( "playOnAwake", playOnAwake );
    node.SetAttribute( "prewarm", prewarm );
    SetCurve( node, "startDelay", startDelay );
    node.SetAttribute( "simulationSpeed", simulationSpeed );
    node.SetAttribute( "simulationSpace", ToString( simulationSpace ) );
    node.SetAttribute( "maxParticles", maxParticles );
    if ( hasBounds )
    {
        SetVector( node, "boundsMin", boundsMin, 3 );
        SetVector( node, "boundsMax", boundsMax, 3 );
    }

    CORE::CDataNode* emission = node.AddChild( "emission", GUCEF_DATATYPE_OBJECT );
    SetCurve( *emission, "rateOverTime", rateOverTime );
    SetCurve( *emission, "rateOverDistance", rateOverDistance );
    emission->SetAttribute( "explosiveness", explosiveness );
    emission->SetAttribute( "randomness", emissionRandomness );
    CORE::CDataNode* burstList = emission->AddChild( "bursts", GUCEF_DATATYPE_ARRAY );
    for ( size_t b=0; b<bursts.size(); ++b )
    {
        CORE::CDataNode* burst = burstList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        burst->SetAttribute( "time", bursts[ b ].time );
        SetCurve( *burst, "count", bursts[ b ].count );
        burst->SetAttribute( "cycles", bursts[ b ].cycles );
        burst->SetAttribute( "interval", bursts[ b ].interval );
        burst->SetAttribute( "probability", bursts[ b ].probability );
    }

    CORE::CDataNode* start = node.AddChild( "start", GUCEF_DATATYPE_OBJECT );
    SetCurve( *start, "lifetime", startLifetime );
    SetCurve( *start, "speed", startSpeed );
    SetCurve( *start, "size", startSize );
    start->SetAttribute( "size3D", startSize3D );
    if ( startSize3D )
    {
        SetCurve( *start, "sizeY", startSizeY );
        SetCurve( *start, "sizeZ", startSizeZ );
    }
    SetCurve( *start, "rotation", startRotation );
    start->SetAttribute( "rotation3D", startRotation3D );
    SetGradient( *start, "colour", startColour );

    CORE::CDataNode* shapeNode = node.AddChild( "shape", GUCEF_DATATYPE_OBJECT );
    shapeNode->SetAttribute( "type", ToString( shape.type ) );
    SetVector( *shapeNode, "position", shape.position, 3 );
    SetVector( *shapeNode, "rotation", shape.rotation, 4 );
    SetVector( *shapeNode, "scale", shape.scale, 3 );
    shapeNode->SetAttribute( "radius", shape.radius );
    shapeNode->SetAttribute( "radiusThickness", shape.radiusThickness );
    shapeNode->SetAttribute( "angle", shape.angle );
    shapeNode->SetAttribute( "arc", shape.arc );
    shapeNode->SetAttribute( "length", shape.length );
    SetVector( *shapeNode, "boxSize", shape.boxSize, 3 );
    shapeNode->SetAttribute( "ringRadius", shape.ringRadius );
    shapeNode->SetAttribute( "ringHeight", shape.ringHeight );
    shapeNode->SetAttribute( "ringInnerRadius", shape.ringInnerRadius );
    shapeNode->SetAttribute( "mesh", shape.mesh );
    shapeNode->SetAttribute( "directionMode", ToString( shape.directionMode ) );
    SetVector( *shapeNode, "direction", shape.direction, 3 );
    shapeNode->SetAttribute( "spreadAngle", shape.spreadAngle );
    shapeNode->SetAttribute( "randomDirectionAmount", shape.randomDirectionAmount );

    CORE::CDataNode* life = node.AddChild( "overLifetime", GUCEF_DATATYPE_OBJECT );
    SetVectorCurves( *life, "velocity", velocity );
    SetCurve( *life, "speedModifier", speedModifier );
    SetVectorCurves( *life, "force", force );
    SetVector( *life, "gravity", gravity, 3 );
    SetCurve( *life, "linearAcceleration", linearAcceleration );
    SetCurve( *life, "radialAcceleration", radialAcceleration );
    SetCurve( *life, "tangentialAcceleration", tangentialAcceleration );
    SetCurve( *life, "damping", damping );
    life->SetAttribute( "limitSpeed", limitSpeed );
    SetCurve( *life, "speedLimit", speedLimit );
    life->SetAttribute( "speedLimitDampen", speedLimitDampen );
    SetCurve( *life, "drag", drag );
    life->SetAttribute( "sizeEnabled", sizeOverLifetimeEnabled );
    SetCurve( *life, "size", sizeOverLifetime );
    life->SetAttribute( "sizeSeparate", sizeOverLifetimeSeparate );
    if ( sizeOverLifetimeSeparate )
    {
        SetCurve( *life, "sizeY", sizeOverLifetimeY );
        SetCurve( *life, "sizeZ", sizeOverLifetimeZ );
    }
    SetCurve( *life, "angularVelocity", angularVelocity );
    life->SetAttribute( "colourEnabled", colourOverLifetimeEnabled );
    SetGradient( *life, "colour", colourOverLifetime );
    life->SetAttribute( "alphaEnabled", alphaOverLifetimeEnabled );
    SetCurve( *life, "alpha", alphaOverLifetime );
    life->SetAttribute( "emissionEnabled", emissionOverLifetimeEnabled );
    SetCurve( *life, "emission", emissionOverLifetime );

    CORE::CDataNode* noiseNode = node.AddChild( "noise", GUCEF_DATATYPE_OBJECT );
    noiseNode->SetAttribute( "enabled", noise.enabled );
    SetCurve( *noiseNode, "strength", noise.strength );
    noiseNode->SetAttribute( "frequency", noise.frequency );
    noiseNode->SetAttribute( "scrollSpeed", noise.scrollSpeed );
    noiseNode->SetAttribute( "octaves", noise.octaves );

    CORE::CDataNode* collisionNode = node.AddChild( "collision", GUCEF_DATATYPE_OBJECT );
    collisionNode->SetAttribute( "enabled", collision.enabled );
    collisionNode->SetAttribute( "world", collision.world );
    collisionNode->SetAttribute( "bounce", collision.bounce );
    collisionNode->SetAttribute( "dampen", collision.dampen );
    collisionNode->SetAttribute( "lifetimeLoss", collision.lifetimeLoss );
    CORE::CDataNode* planes = collisionNode->AddChild( "planes", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<collision.planes.size(); ++i )
    {
        CORE::CDataNode* plane = planes->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        SetVector( *plane, "point", collision.planes[ i ].point, 3 );
        SetVector( *plane, "normal", collision.planes[ i ].normal, 3 );
    }

    CORE::CDataNode* sheet = node.AddChild( "textureSheet", GUCEF_DATATYPE_OBJECT );
    sheet->SetAttribute( "enabled", textureSheet.enabled );
    sheet->SetAttribute( "tilesX", textureSheet.tilesX );
    sheet->SetAttribute( "tilesY", textureSheet.tilesY );
    sheet->SetAttribute( "singleRow", textureSheet.singleRow );
    sheet->SetAttribute( "rowIndex", textureSheet.rowIndex );
    SetCurve( *sheet, "frameOverTime", textureSheet.frameOverTime );
    SetCurve( *sheet, "startFrame", textureSheet.startFrame );
    sheet->SetAttribute( "cycles", textureSheet.cycles );
    sheet->SetAttribute( "fps", textureSheet.fps );

    CORE::CDataNode* renderNode = node.AddChild( "renderer", GUCEF_DATATYPE_OBJECT );
    renderNode->SetAttribute( "mode", ToString( renderer.mode ) );
    renderNode->SetAttribute( "alignment", ToString( renderer.alignment ) );
    renderNode->SetAttribute( "material", renderer.material );
    renderNode->SetAttribute( "trailMaterial", renderer.trailMaterial );
    renderNode->SetAttribute( "mesh", renderer.mesh );
    SetVector( *renderNode, "quadSize", renderer.quadSize, 2 );
    SetVector( *renderNode, "pivot", renderer.pivot, 3 );
    renderNode->SetAttribute( "lengthScale", renderer.lengthScale );
    renderNode->SetAttribute( "velocityScale", renderer.velocityScale );
    renderNode->SetAttribute( "blendMode", ToString( renderer.blendMode ) );
    renderNode->SetAttribute( "alphaCutoff", renderer.alphaCutoff );
    SetVector( *renderNode, "tint", renderer.tint, 4 );
    renderNode->SetAttribute( "useParticleColour", renderer.useParticleColour );
    renderNode->SetAttribute( "lit", renderer.lit );
    renderNode->SetAttribute( "softParticles", renderer.softParticles );
    renderNode->SetAttribute( "sortMode", ToString( renderer.sortMode ) );

    CORE::CDataNode* subs = node.AddChild( "subEmitters", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<subEmitters.size(); ++i )
    {
        CORE::CDataNode* sub = subs->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        sub->SetAttribute( "system", subEmitters[ i ].system );
        sub->SetAttribute( "trigger", ToString( subEmitters[ i ].trigger ) );
        sub->SetAttribute( "probability", subEmitters[ i ].probability );
        sub->SetAttribute( "inherit", subEmitters[ i ].inherit );
    }

    CORE::CDataNode* trailNode = node.AddChild( "trails", GUCEF_DATATYPE_OBJECT );
    trailNode->SetAttribute( "enabled", trails.enabled );
    SetCurve( *trailNode, "lifetime", trails.lifetime );
    SetCurve( *trailNode, "widthOverTrail", trails.widthOverTrail );
    SetGradient( *trailNode, "colourOverTrail", trails.colourOverTrail );
    trailNode->SetAttribute( "minVertexDistance", trails.minVertexDistance );
    trailNode->SetAttribute( "worldSpace", trails.worldSpace );
    trailNode->SetAttribute( "inheritColour", trails.inheritColour );

    CORE::CDataNode* sourceNode = node.AddChild( source );
    sourceNode->SetName( "source" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleSystem::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetParticleSystem s;
    s.name = node.GetAttributeValueOrChildValueByName( "name" ).AsString();
    s.duration = GetFloat( node, "duration", s.duration );
    s.looping = GetBool( node, "looping", s.looping );
    s.playOnAwake = GetBool( node, "playOnAwake", s.playOnAwake );
    s.prewarm = GetFloat( node, "prewarm", s.prewarm );
    GetCurve( node, "startDelay", s.startDelay );
    s.simulationSpeed = GetFloat( node, "simulationSpeed", s.simulationSpeed );
    FromString( node.GetAttributeValueOrChildValueByName( "simulationSpace" ).AsString(), s.simulationSpace );
    s.maxParticles = node.GetAttributeValueOrChildValueByName( "maxParticles" ).AsUInt32( s.maxParticles );
    s.hasBounds = GUCEF_NULL != node.FindChild( "boundsMin" );
    GetVector( node, "boundsMin", s.boundsMin, 3 );
    GetVector( node, "boundsMax", s.boundsMax, 3 );

    const CORE::CDataNode* emission = node.FindChild( "emission" );
    if ( GUCEF_NULL != emission )
    {
        GetCurve( *emission, "rateOverTime", s.rateOverTime );
        GetCurve( *emission, "rateOverDistance", s.rateOverDistance );
        s.explosiveness = GetFloat( *emission, "explosiveness", s.explosiveness );
        s.emissionRandomness = GetFloat( *emission, "randomness", s.emissionRandomness );
        const CORE::CDataNode* burstList = emission->FindChild( "bursts" );
        if ( GUCEF_NULL != burstList )
        {
            for ( const CORE::CDataNode* b = burstList->GetFirstChild(); GUCEF_NULL != b; b = b->GetNext() )
            {
                SBurst burst;
                burst.time = GetFloat( *b, "time", burst.time );
                GetCurve( *b, "count", burst.count );
                burst.cycles = b->GetAttributeValueOrChildValueByName( "cycles" ).AsUInt32( burst.cycles );
                burst.interval = GetFloat( *b, "interval", burst.interval );
                burst.probability = GetFloat( *b, "probability", burst.probability );
                s.bursts.push_back( burst );
            }
        }
    }

    const CORE::CDataNode* start = node.FindChild( "start" );
    if ( GUCEF_NULL != start )
    {
        GetCurve( *start, "lifetime", s.startLifetime );
        GetCurve( *start, "speed", s.startSpeed );
        GetCurve( *start, "size", s.startSize );
        s.startSize3D = GetBool( *start, "size3D", false );
        GetCurve( *start, "sizeY", s.startSizeY );
        GetCurve( *start, "sizeZ", s.startSizeZ );
        GetCurve( *start, "rotation", s.startRotation );
        s.startRotation3D = GetBool( *start, "rotation3D", false );
        GetGradient( *start, "colour", s.startColour );
    }

    const CORE::CDataNode* shapeNode = node.FindChild( "shape" );
    if ( GUCEF_NULL != shapeNode )
    {
        FromString( shapeNode->GetAttributeValueOrChildValueByName( "type" ).AsString(), s.shape.type );
        GetVector( *shapeNode, "position", s.shape.position, 3 );
        GetVector( *shapeNode, "rotation", s.shape.rotation, 4 );
        GetVector( *shapeNode, "scale", s.shape.scale, 3 );
        s.shape.radius = GetFloat( *shapeNode, "radius", s.shape.radius );
        s.shape.radiusThickness = GetFloat( *shapeNode, "radiusThickness", s.shape.radiusThickness );
        s.shape.angle = GetFloat( *shapeNode, "angle", s.shape.angle );
        s.shape.arc = GetFloat( *shapeNode, "arc", s.shape.arc );
        s.shape.length = GetFloat( *shapeNode, "length", s.shape.length );
        GetVector( *shapeNode, "boxSize", s.shape.boxSize, 3 );
        s.shape.ringRadius = GetFloat( *shapeNode, "ringRadius", s.shape.ringRadius );
        s.shape.ringHeight = GetFloat( *shapeNode, "ringHeight", s.shape.ringHeight );
        s.shape.ringInnerRadius = GetFloat( *shapeNode, "ringInnerRadius", s.shape.ringInnerRadius );
        s.shape.mesh = shapeNode->GetAttributeValueOrChildValueByName( "mesh" ).AsInt32( -1 );
        FromString( shapeNode->GetAttributeValueOrChildValueByName( "directionMode" ).AsString(), s.shape.directionMode );
        GetVector( *shapeNode, "direction", s.shape.direction, 3 );
        s.shape.spreadAngle = GetFloat( *shapeNode, "spreadAngle", s.shape.spreadAngle );
        s.shape.randomDirectionAmount = GetFloat( *shapeNode, "randomDirectionAmount", s.shape.randomDirectionAmount );
    }

    const CORE::CDataNode* life = node.FindChild( "overLifetime" );
    if ( GUCEF_NULL != life )
    {
        GetVectorCurves( *life, "velocity", s.velocity );
        GetCurve( *life, "speedModifier", s.speedModifier );
        GetVectorCurves( *life, "force", s.force );
        GetVector( *life, "gravity", s.gravity, 3 );
        GetCurve( *life, "linearAcceleration", s.linearAcceleration );
        GetCurve( *life, "radialAcceleration", s.radialAcceleration );
        GetCurve( *life, "tangentialAcceleration", s.tangentialAcceleration );
        GetCurve( *life, "damping", s.damping );
        s.limitSpeed = GetBool( *life, "limitSpeed", false );
        GetCurve( *life, "speedLimit", s.speedLimit );
        s.speedLimitDampen = GetFloat( *life, "speedLimitDampen", 0.0f );
        GetCurve( *life, "drag", s.drag );
        s.sizeOverLifetimeEnabled = GetBool( *life, "sizeEnabled", false );
        GetCurve( *life, "size", s.sizeOverLifetime );
        s.sizeOverLifetimeSeparate = GetBool( *life, "sizeSeparate", false );
        GetCurve( *life, "sizeY", s.sizeOverLifetimeY );
        GetCurve( *life, "sizeZ", s.sizeOverLifetimeZ );
        GetCurve( *life, "angularVelocity", s.angularVelocity );
        s.colourOverLifetimeEnabled = GetBool( *life, "colourEnabled", false );
        GetGradient( *life, "colour", s.colourOverLifetime );
        s.alphaOverLifetimeEnabled = GetBool( *life, "alphaEnabled", false );
        GetCurve( *life, "alpha", s.alphaOverLifetime );
        s.emissionOverLifetimeEnabled = GetBool( *life, "emissionEnabled", false );
        GetCurve( *life, "emission", s.emissionOverLifetime );
    }

    const CORE::CDataNode* noiseNode = node.FindChild( "noise" );
    if ( GUCEF_NULL != noiseNode )
    {
        s.noise.enabled = GetBool( *noiseNode, "enabled", false );
        GetCurve( *noiseNode, "strength", s.noise.strength );
        s.noise.frequency = GetFloat( *noiseNode, "frequency", s.noise.frequency );
        s.noise.scrollSpeed = GetFloat( *noiseNode, "scrollSpeed", s.noise.scrollSpeed );
        s.noise.octaves = noiseNode->GetAttributeValueOrChildValueByName( "octaves" ).AsUInt32( 1 );
    }

    const CORE::CDataNode* collisionNode = node.FindChild( "collision" );
    if ( GUCEF_NULL != collisionNode )
    {
        s.collision.enabled = GetBool( *collisionNode, "enabled", false );
        s.collision.world = GetBool( *collisionNode, "world", false );
        s.collision.bounce = GetFloat( *collisionNode, "bounce", s.collision.bounce );
        s.collision.dampen = GetFloat( *collisionNode, "dampen", s.collision.dampen );
        s.collision.lifetimeLoss = GetFloat( *collisionNode, "lifetimeLoss", s.collision.lifetimeLoss );
        const CORE::CDataNode* planes = collisionNode->FindChild( "planes" );
        if ( GUCEF_NULL != planes )
        {
            for ( const CORE::CDataNode* p = planes->GetFirstChild(); GUCEF_NULL != p; p = p->GetNext() )
            {
                SPlane plane;
                GetVector( *p, "point", plane.point, 3 );
                GetVector( *p, "normal", plane.normal, 3 );
                s.collision.planes.push_back( plane );
            }
        }
    }

    const CORE::CDataNode* sheet = node.FindChild( "textureSheet" );
    if ( GUCEF_NULL != sheet )
    {
        s.textureSheet.enabled = GetBool( *sheet, "enabled", false );
        s.textureSheet.tilesX = sheet->GetAttributeValueOrChildValueByName( "tilesX" ).AsUInt32( 1 );
        s.textureSheet.tilesY = sheet->GetAttributeValueOrChildValueByName( "tilesY" ).AsUInt32( 1 );
        s.textureSheet.singleRow = GetBool( *sheet, "singleRow", false );
        s.textureSheet.rowIndex = sheet->GetAttributeValueOrChildValueByName( "rowIndex" ).AsInt32( 0 );
        GetCurve( *sheet, "frameOverTime", s.textureSheet.frameOverTime );
        GetCurve( *sheet, "startFrame", s.textureSheet.startFrame );
        s.textureSheet.cycles = GetFloat( *sheet, "cycles", 1.0f );
        s.textureSheet.fps = GetFloat( *sheet, "fps", 0.0f );
    }

    const CORE::CDataNode* renderNode = node.FindChild( "renderer" );
    if ( GUCEF_NULL != renderNode )
    {
        FromString( renderNode->GetAttributeValueOrChildValueByName( "mode" ).AsString(), s.renderer.mode );
        FromString( renderNode->GetAttributeValueOrChildValueByName( "alignment" ).AsString(), s.renderer.alignment );
        s.renderer.material = renderNode->GetAttributeValueOrChildValueByName( "material" ).AsInt32( -1 );
        s.renderer.trailMaterial = renderNode->GetAttributeValueOrChildValueByName( "trailMaterial" ).AsInt32( -1 );
        s.renderer.mesh = renderNode->GetAttributeValueOrChildValueByName( "mesh" ).AsInt32( -1 );
        GetVector( *renderNode, "quadSize", s.renderer.quadSize, 2 );
        GetVector( *renderNode, "pivot", s.renderer.pivot, 3 );
        s.renderer.lengthScale = GetFloat( *renderNode, "lengthScale", s.renderer.lengthScale );
        s.renderer.velocityScale = GetFloat( *renderNode, "velocityScale", s.renderer.velocityScale );
        FromString( renderNode->GetAttributeValueOrChildValueByName( "blendMode" ).AsString(), s.renderer.blendMode );
        s.renderer.alphaCutoff = GetFloat( *renderNode, "alphaCutoff", 0.0f );
        GetVector( *renderNode, "tint", s.renderer.tint, 4 );
        s.renderer.useParticleColour = GetBool( *renderNode, "useParticleColour", true );
        s.renderer.lit = GetBool( *renderNode, "lit", false );
        s.renderer.softParticles = GetBool( *renderNode, "softParticles", false );
        FromString( renderNode->GetAttributeValueOrChildValueByName( "sortMode" ).AsString(), s.renderer.sortMode );
    }

    const CORE::CDataNode* subs = node.FindChild( "subEmitters" );
    if ( GUCEF_NULL != subs )
    {
        for ( const CORE::CDataNode* e = subs->GetFirstChild(); GUCEF_NULL != e; e = e->GetNext() )
        {
            SSubEmitter sub;
            sub.system = e->GetAttributeValueOrChildValueByName( "system" ).AsInt32( -1 );
            FromString( e->GetAttributeValueOrChildValueByName( "trigger" ).AsString(), sub.trigger );
            sub.probability = GetFloat( *e, "probability", 1.0f );
            sub.inherit = e->GetAttributeValueOrChildValueByName( "inherit" ).AsUInt32( 0 );
            s.subEmitters.push_back( sub );
        }
    }

    const CORE::CDataNode* trailNode = node.FindChild( "trails" );
    if ( GUCEF_NULL != trailNode )
    {
        s.trails.enabled = GetBool( *trailNode, "enabled", false );
        GetCurve( *trailNode, "lifetime", s.trails.lifetime );
        GetCurve( *trailNode, "widthOverTrail", s.trails.widthOverTrail );
        GetGradient( *trailNode, "colourOverTrail", s.trails.colourOverTrail );
        s.trails.minVertexDistance = GetFloat( *trailNode, "minVertexDistance", s.trails.minVertexDistance );
        s.trails.worldSpace = GetBool( *trailNode, "worldSpace", false );
        s.trails.inheritColour = GetBool( *trailNode, "inheritColour", true );
    }

    const CORE::CDataNode* sourceNode = node.FindChild( "source" );
    if ( GUCEF_NULL != sourceNode )
    {
        s.source = *sourceNode;
        s.source.SetName( "source" );
    }
    *this = s;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
