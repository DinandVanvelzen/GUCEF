/*
 *  assetcookerOGRE: writes Ogre particle scripts of the particle systems of the intermediate
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
#include <algorithm>

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "assetcookerOGRE_CParticleScriptWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "ogre";

/** A burst of Ogre's emitters: emission_rate particles at once with a negative duration */
static const Float32 OgreBurstDuration = -1.0f;

/** The difference of a colour channel that the reduction to six stages may leave without a report */
static const Float32 ColourTolerance = 2.0f / 255.0f;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef ASSETCONVERTER::CAssetParticleSystem TSystem;
typedef ASSETCONVERTER::CAssetParticleCurve TCurve;

Float32
Length( const Float32 v[ 3 ] )
{
    return (Float32) sqrt( v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ] );
}

Float32
Dot( const Float32 a[ 3 ], const Float32 b[ 3 ] )
{
    return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
}

void
Cross( const Float32 a[ 3 ], const Float32 b[ 3 ], Float32 out[ 3 ] )
{
    out[ 0 ] = a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
    out[ 1 ] = a[ 2 ] * b[ 0 ] - a[ 0 ] * b[ 2 ];
    out[ 2 ] = a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ];
}

void
Normalise( Float32 v[ 3 ] )
{
    const Float32 length = Length( v );
    if ( length > 0.0f )
        for ( UInt32 i=0; i<3; ++i )
            v[ i ] /= length;
}

/** v' = q v q* */
void
Rotate( const Float32 q[ 4 ], const Float32 v[ 3 ], Float32 out[ 3 ] )
{
    const Float32 u[ 3 ] = { q[ 0 ], q[ 1 ], q[ 2 ] };
    const Float32 w = q[ 3 ];
    Float32 uv[ 3 ], uuv[ 3 ];
    Cross( u, v, uv );
    Cross( u, uv, uuv );
    for ( UInt32 i=0; i<3; ++i )
        out[ i ] = v[ i ] + 2.0f * ( w * uv[ i ] + uuv[ i ] );
}

CORE::CString
Vector( const Float32 v[ 3 ] )
{
    return CParticleScriptWriter::FormatNumber( v[ 0 ] ) + " " + CParticleScriptWriter::FormatNumber( v[ 1 ] ) + " " + CParticleScriptWriter::FormatNumber( v[ 2 ] );
}

/** The sRGB value of a colour channel as linear, as the cooked materials decode their colour textures */
Float32
SrgbToLinear( Float32 value )
{
    if ( value <= 0.04045f )
        return value / 12.92f;
    return (Float32) pow( ( value + 0.055 ) / 1.055, 2.4 );
}

/** A colour for Ogre: clamped, the colour channels (authored in sRGB) linear, alpha as it is */
CORE::CString
Colour( const Float32 rgba[ 4 ] )
{
    Float32 c[ 4 ];
    for ( UInt32 i=0; i<4; ++i )
        c[ i ] = rgba[ i ] < 0.0f ? 0.0f : ( rgba[ i ] > 1.0f ? 1.0f : rgba[ i ] );
    for ( UInt32 i=0; i<3; ++i )
        c[ i ] = SrgbToLinear( c[ i ] );
    return CParticleScriptWriter::FormatNumber( c[ 0 ] ) + " " + CParticleScriptWriter::FormatNumber( c[ 1 ] ) + " " +
           CParticleScriptWriter::FormatNumber( c[ 2 ] ) + " " + CParticleScriptWriter::FormatNumber( c[ 3 ] );
}

CORE::CString
Quote( const CORE::CString& text )
{
    return "\"" + text + "\"";
}

/** The extent of a box of 'sizes' along the axes 'axes' (unit vectors, the frame of the shape), measured along 'u' */
Float32
Extent( const Float32 axes[ 3 ][ 3 ], const Float32 sizes[ 3 ], const Float32 u[ 3 ] )
{
    return (Float32) ( fabs( Dot( u, axes[ 0 ] ) ) * sizes[ 0 ] + fabs( Dot( u, axes[ 1 ] ) ) * sizes[ 1 ] + fabs( Dot( u, axes[ 2 ] ) ) * sizes[ 2 ] );
}

bool
IsAxisAligned( const Float32 axes[ 3 ][ 3 ], const Float32 frame[ 3 ][ 3 ] )
{
    for ( UInt32 a=0; a<3; ++a )
    {
        bool aligned = false;
        for ( UInt32 f=0; f<3; ++f )
            aligned = aligned || fabs( fabs( Dot( axes[ a ], frame[ f ] ) ) - 1.0f ) < 1.0e-3f;
        if ( !aligned )
            return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CParticleScriptWriter::CParticleScriptWriter( const ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report )
    : m_scene( scene )
    , m_report( report )
    , m_text()
    , m_subject()
    , m_count( 0 )
    , m_colourAffector( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CParticleScriptWriter::FormatNumber( Float32 value )
{GUCEF_TRACE;

    // whole numbers plainly (the round-trip form writes "2e+01"), others in the round-trip form
    const Float64 whole = floor( (Float64) value + 0.5 );
    if ( fabs( (Float64) value - whole ) < 1.0e-6 && fabs( whole ) < 1.0e9 )
        return CORE::Int64ToString( (Int64) whole );
    return ASSETCONVERTER::CCanonicalJson::FormatFloat32( value );
}

/*-------------------------------------------------------------------------*/

UInt32
CParticleScriptWriter::GetSystemCount( void ) const
{GUCEF_TRACE;

    return m_count;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CParticleScriptWriter::GetText( void ) const
{GUCEF_TRACE;

    return m_text;
}

/*-------------------------------------------------------------------------*/

void
CParticleScriptWriter::Line( UInt32 depth, const CORE::CString& text )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<depth; ++i )
        m_text += "    ";
    m_text += text;
    m_text += "\n";
}

/*-------------------------------------------------------------------------*/

void
CParticleScriptWriter::Approximated( const CORE::CString& message )
{GUCEF_TRACE;

    m_report.AddApproximation( SourceName, m_subject, message );
}

/*-------------------------------------------------------------------------*/

void
CParticleScriptWriter::Skipped( const CORE::CString& message )
{GUCEF_TRACE;

    m_report.AddSkipped( SourceName, m_subject, message );
}

/*-------------------------------------------------------------------------*/

void
CParticleScriptWriter::SampleColour( const TSystem& system, Float32 time, Float32 rgba[ 4 ] )
{GUCEF_TRACE;

    Float32 start[ 4 ];
    system.startColour.GetAverageAt( 0.0f, start );
    for ( UInt32 i=0; i<4; ++i )
        rgba[ i ] = start[ i ] * system.renderer.tint[ i ];
    if ( system.colourOverLifetimeEnabled )
    {
        Float32 colour[ 4 ];
        system.colourOverLifetime.GetAverageAt( time, colour );
        for ( UInt32 i=0; i<4; ++i )
            rgba[ i ] *= colour[ i ];
    }
    if ( system.alphaOverLifetimeEnabled )
        rgba[ 3 ] *= system.alphaOverLifetime.GetAverageAt( time );
    if ( system.emissionOverLifetimeEnabled )
    {
        const Float32 brightening = 1.0f + system.emissionOverLifetime.GetAverageAt( time );
        for ( UInt32 i=0; i<3; ++i )
            rgba[ i ] *= brightening;
    }
}

/*-------------------------------------------------------------------------*/

bool
CParticleScriptWriter::BuildColourStages( const TSystem& system, UInt32 maxStages, TColourStageVector& stages, Float32& maxError )
{GUCEF_TRACE;

    stages.clear();
    maxError = 0.0f;
    const bool overLifetime = ( system.colourOverLifetimeEnabled && !system.colourOverLifetime.IsConstantOverTime() ) ||
                              ( system.alphaOverLifetimeEnabled && !system.alphaOverLifetime.IsConstantOverTime() ) ||
                              ( system.emissionOverLifetimeEnabled && !system.emissionOverLifetime.IsConstantOverTime() );
    if ( !overLifetime )
        return false;

    // the candidates: the key times and an even grid; the colours clamped as Ogre stores them
    GUCEF::vector< Float32 > times;
    system.colourOverLifetime.GetKeyTimes( times );
    const UInt32 grid = 64;
    for ( UInt32 i=0; i<=grid; ++i )
        times.push_back( (Float32) i / (Float32) grid );
    std::sort( times.begin(), times.end() );
    times.erase( std::unique( times.begin(), times.end() ), times.end() );
    GUCEF::vector< SColourStage > samples( times.size() );
    for ( size_t s=0; s<times.size(); ++s )
    {
        samples[ s ].time = times[ s ];
        SampleColour( system, times[ s ], samples[ s ].colour );
        for ( UInt32 i=0; i<4; ++i )
            samples[ s ].colour[ i ] = samples[ s ].colour[ i ] < 0.0f ? 0.0f : ( samples[ s ].colour[ i ] > 1.0f ? 1.0f : samples[ s ].colour[ i ] );
    }

    // greedy: start with the ends, add the sample with the largest error until the stages are used
    GUCEF::vector< size_t > chosen;
    chosen.push_back( 0 );
    chosen.push_back( samples.size() - 1 );
    while ( true )
    {
        Float32 worst = 0.0f;
        size_t worstIndex = 0;
        for ( size_t s=0; s<samples.size(); ++s )
        {
            size_t a = 0;
            while ( a+1 < chosen.size() && samples[ chosen[ a+1 ] ].time <= samples[ s ].time )
                ++a;
            const size_t b = a+1 < chosen.size() ? a+1 : a;
            const Float32 t0 = samples[ chosen[ a ] ].time;
            const Float32 t1 = samples[ chosen[ b ] ].time;
            const Float32 f = t1 > t0 ? ( samples[ s ].time - t0 ) / ( t1 - t0 ) : 0.0f;
            for ( UInt32 i=0; i<4; ++i )
            {
                const Float32 interpolated = samples[ chosen[ a ] ].colour[ i ] + ( samples[ chosen[ b ] ].colour[ i ] - samples[ chosen[ a ] ].colour[ i ] ) * f;
                const Float32 error = (Float32) fabs( interpolated - samples[ s ].colour[ i ] );
                if ( error > worst )
                {
                    worst = error;
                    worstIndex = s;
                }
            }
        }
        maxError = worst;
        if ( worst <= 1.0f / 255.0f || chosen.size() >= maxStages )
            break;
        chosen.push_back( worstIndex );
        std::sort( chosen.begin(), chosen.end() );
    }
    for ( size_t c=0; c<chosen.size(); ++c )
        stages.push_back( samples[ chosen[ c ] ] );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CParticleScriptWriter::WriteRenderer( const TSystem& system, Float32 speed, Float32& width, Float32& height )
{GUCEF_TRACE;

    const TSystem::SRenderer& renderer = system.renderer;

    // the size: the quad times the mean start size times the intercept of the size curve's line
    const Float32 meanSize = system.startSize.GetAverage();
    Float32 factor = 1.0f;
    if ( system.sizeOverLifetimeEnabled )
    {
        Float32 intercept = 1.0f, slope = 0.0f, error = 0.0f;
        system.sizeOverLifetime.GetLinearFit( intercept, slope, error );
        const Float32 peak = system.sizeOverLifetime.GetMaximum();
        factor = intercept > 0.05f * peak ? intercept : 0.05f * peak;
    }
    width = renderer.quadSize[ 0 ] * meanSize * factor;
    height = renderer.quadSize[ 1 ] * meanSize * factor;

    CORE::CString type = "point";
    bool rotation = !system.startRotation.IsZero() || !system.angularVelocity.IsZero();
    switch ( renderer.mode )
    {
        case ASSETCONVERTER::PARTICLERENDER_STRETCHED:
        {
            type = "oriented_self";
            const Float32 speedPart = renderer.velocityScale * system.startSpeed.GetAverage() * speed;
            height = renderer.quadSize[ 1 ] * meanSize * factor * renderer.lengthScale + speedPart;
            if ( 0.0f != renderer.velocityScale )
                Approximated( "the stretch by the speed uses the mean start speed (Ogre's billboards do not stretch with the speed)" );
            rotation = false;
            break;
        }
        case ASSETCONVERTER::PARTICLERENDER_VERTICAL: type = "oriented_common"; break;
        case ASSETCONVERTER::PARTICLERENDER_HORIZONTAL: type = "perpendicular_common"; break;
        case ASSETCONVERTER::PARTICLERENDER_ALIGNED: type = "perpendicular_common"; break;
        case ASSETCONVERTER::PARTICLERENDER_MESH:
        {
            Approximated( "mesh particles are drawn as billboards (ParticleFX renders billboards only)" );
            if ( renderer.mesh >= 0 && (size_t) renderer.mesh < m_scene.meshes.size() )
            {
                // the size of the mesh
                Float32 low[ 3 ] = { 0.0f, 0.0f, 0.0f }, high[ 3 ] = { 0.0f, 0.0f, 0.0f };
                bool any = false;
                const ASSETCONVERTER::CAssetMesh& mesh = m_scene.meshes[ renderer.mesh ];
                for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
                    for ( size_t p=0; p+2<mesh.subMeshes[ s ].positions.size(); p+=3 )
                        for ( UInt32 i=0; i<3; ++i )
                        {
                            const Float32 v = mesh.subMeshes[ s ].positions[ p+i ];
                            low[ i ] = any && low[ i ] < v ? low[ i ] : v;
                            high[ i ] = any && high[ i ] > v ? high[ i ] : v;
                            any = any || 2 == i;
                        }
                if ( any )
                {
                    const Float32 x = high[ 0 ] - low[ 0 ], y = high[ 1 ] - low[ 1 ];
                    width = meanSize * factor * ( x > 0.0f ? x : 1.0f );
                    height = meanSize * factor * ( y > 0.0f ? y : 1.0f );
                }
            }
            break;
        }
        default:
            if ( ASSETCONVERTER::PARTICLEALIGNMENT_VELOCITY == renderer.alignment )
            {
                type = "oriented_self";
                rotation = false;
            }
            break;
    }

    Line( 1, "renderer billboard" );
    Line( 1, "billboard_type " + type );
    if ( ASSETCONVERTER::PARTICLERENDER_VERTICAL == renderer.mode )
        Line( 1, "common_direction 0 1 0" );
    if ( ASSETCONVERTER::PARTICLERENDER_HORIZONTAL == renderer.mode )
    {
        Line( 1, "common_direction 0 1 0" );
        Line( 1, "common_up_vector 0 0 1" );
    }
    if ( ASSETCONVERTER::PARTICLERENDER_ALIGNED == renderer.mode )
    {
        Line( 1, "common_direction 0 0 1" );
        Line( 1, "common_up_vector 0 1 0" );
    }
    if ( ASSETCONVERTER::PARTICLEALIGNMENT_FACING == renderer.alignment && "point" == type )
        Line( 1, "accurate_facing on" );
    if ( rotation )
    {
        if ( "point" == type )
            Line( 1, "billboard_rotation_type vertex" );
        else
            Approximated( "the rotation of the particles is not applied to " + type + " billboards" );
    }

    // the origin: the particle's position on the quad; the pivot is the offset of the quad's centre in its sizes
    const Float32 px = renderer.pivot[ 0 ];
    const Float32 py = renderer.pivot[ 1 ];
    if ( 0.0f != px || 0.0f != py )
    {
        const CORE::CString vertical = py >= 0.25f ? "bottom" : ( py <= -0.25f ? "top" : "center" );
        const CORE::CString horizontal = px >= 0.25f ? "left" : ( px <= -0.25f ? "right" : "center" );
        CORE::CString origin = "center";
        if ( "center" != vertical && "center" != horizontal )
            origin = vertical + "_" + horizontal;
        else if ( "center" != vertical )
            origin = vertical + "_center";
        else if ( "center" != horizontal )
            origin = "center_" + horizontal;
        if ( "center" != origin )
            Line( 1, "billboard_origin " + origin );
        const bool exact = ( 0.0f == px || 0.5f == (Float32) fabs( px ) ) && ( 0.0f == py || 0.5f == (Float32) fabs( py ) );
        if ( !exact )
            Approximated( "the pivot (" + FormatNumber( px ) + ", " + FormatNumber( py ) + ") of the quad is written as the billboard origin " + origin );
    }
    if ( 0.0f != renderer.pivot[ 2 ] )
        Approximated( "the pivot offset towards the camera is not carried" );

    if ( system.textureSheet.enabled && system.textureSheet.tilesX * system.textureSheet.tilesY > 1 )
        Line( 1, "texture_sheet_size " + CORE::UInt32ToString( system.textureSheet.tilesY ) + " " + CORE::UInt32ToString( system.textureSheet.tilesX ) );
}

/*-------------------------------------------------------------------------*/

void
CParticleScriptWriter::WriteEmitterBody( const TSystem& system, Float32 speed, bool withColour )
{GUCEF_TRACE;

    const TSystem::SShape& shape = system.shape;

    // the frame of the shape
    Float32 axes[ 3 ][ 3 ];
    const Float32 unit[ 3 ][ 3 ] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
    for ( UInt32 a=0; a<3; ++a )
        Rotate( shape.rotation, unit[ a ], axes[ a ] );
    Float32 axis[ 3 ] = { axes[ 1 ][ 0 ], axes[ 1 ][ 1 ], axes[ 1 ][ 2 ] };

    // the emission direction, with a constant velocity over the lifetime added to the start velocity
    Float32 direction[ 3 ] = { shape.direction[ 0 ], shape.direction[ 1 ], shape.direction[ 2 ] };
    bool outward = ASSETCONVERTER::PARTICLEDIRECTION_OUTWARD == shape.directionMode;
    Float32 angle = shape.spreadAngle;
    if ( outward )
    {
        for ( UInt32 i=0; i<3; ++i )
            direction[ i ] = axis[ i ];
        angle = ( ASSETCONVERTER::PARTICLESHAPE_CONE == shape.type ) ? shape.angle : 0.0f;
        if ( ASSETCONVERTER::PARTICLESHAPE_CONE == shape.type )
            Approximated( "the cone emits within its angle at random, not away from its apex" );
    }
    Float32 speedMin = system.startSpeed.GetMinimum() * speed;
    Float32 speedMax = system.startSpeed.GetMaximum() * speed;
    if ( system.velocity.enabled )
    {
        Float32 added[ 3 ];
        system.velocity.GetAverage( added );
        if ( !system.velocity.IsConstant() )
            Approximated( "a varying velocity over the lifetime is its mean, added to the start velocity" );
        if ( system.velocity.space != system.simulationSpace )
            Approximated( "the velocity over the lifetime in another space than the simulation is added in the simulation space" );
        const Float32 addedSpeed = Length( added ) * speed;
        if ( addedSpeed > 0.0f )
        {
            if ( speedMax <= 0.0f && !outward )
            {
                // without a start speed the added velocity is the motion: exact
                for ( UInt32 i=0; i<3; ++i )
                    direction[ i ] = added[ i ];
                speedMin = speedMax = addedSpeed;
            }
            else
            {
                const Float32 meanSpeed = 0.5f * ( speedMin + speedMax );
                Float32 sum[ 3 ];
                for ( UInt32 i=0; i<3; ++i )
                    sum[ i ] = direction[ i ] * meanSpeed + added[ i ] * speed;
                const Float32 total = Length( sum );
                if ( total > 0.0f )
                {
                    for ( UInt32 i=0; i<3; ++i )
                        direction[ i ] = sum[ i ];
                    const Float32 scale = meanSpeed > 0.0f ? total / meanSpeed : 1.0f;
                    speedMin = meanSpeed > 0.0f ? speedMin * scale : total;
                    speedMax = meanSpeed > 0.0f ? speedMax * scale : total;
                }
                outward = false;
                Approximated( "the velocity over the lifetime is added to the start velocity of the mean particle" );
            }
        }
    }
    if ( system.speedModifier.GetMinimum() != 1.0f || system.speedModifier.GetMaximum() != 1.0f )
        Approximated( "the speed modifier over the lifetime is not applied" );
    Normalise( direction );
    if ( Length( direction ) <= 0.0f )
    {
        direction[ 0 ] = 0.0f; direction[ 1 ] = 1.0f; direction[ 2 ] = 0.0f;
    }
    if ( shape.randomDirectionAmount > 0.0f )
    {
        angle = angle + ( 180.0f - angle ) * shape.randomDirectionAmount;
        Approximated( "the random direction amount widens the spread of the emission" );
    }
    angle = angle > 180.0f ? 180.0f : ( angle < 0.0f ? 0.0f : angle );

    // Ogre's area emitters span 'up x direction', 'up' and 'direction': up is the world axis least along the direction
    Float32 up[ 3 ] = { 0.0f, 1.0f, 0.0f };
    {
        Float32 best = 2.0f;
        for ( UInt32 a=0; a<3; ++a )
        {
            const Float32 along = (Float32) fabs( direction[ a ] );
            if ( along < best )
            {
                best = along;
                up[ 0 ] = up[ 1 ] = up[ 2 ] = 0.0f;
                up[ a ] = 1.0f;
            }
        }
        const Float32 d = Dot( up, direction );
        for ( UInt32 i=0; i<3; ++i )
            up[ i ] -= d * direction[ i ];
        Normalise( up );
    }
    Float32 left[ 3 ];
    Cross( up, direction, left );
    const Float32 frame[ 3 ][ 3 ] = { { left[ 0 ], left[ 1 ], left[ 2 ] }, { up[ 0 ], up[ 1 ], up[ 2 ] }, { direction[ 0 ], direction[ 1 ], direction[ 2 ] } };

    // the sizes of the shape in its own frame
    Float32 sizes[ 3 ] = { 0.0f, 0.0f, 0.0f };
    CORE::CString type = "Point";
    Float32 inner = 0.0f;
    switch ( shape.type )
    {
        case ASSETCONVERTER::PARTICLESHAPE_BOX:
        case ASSETCONVERTER::PARTICLESHAPE_EDGE:
        case ASSETCONVERTER::PARTICLESHAPE_MESH:
        case ASSETCONVERTER::PARTICLESHAPE_POINTS:
            type = "Box";
            for ( UInt32 i=0; i<3; ++i )
                sizes[ i ] = shape.boxSize[ i ] * shape.scale[ i ];
            if ( ASSETCONVERTER::PARTICLESHAPE_MESH == shape.type || ASSETCONVERTER::PARTICLESHAPE_POINTS == shape.type )
                Approximated( "the emission from a mesh or a set of points is the emission from its box" );
            break;
        case ASSETCONVERTER::PARTICLESHAPE_SPHERE:
        case ASSETCONVERTER::PARTICLESHAPE_HEMISPHERE:
            for ( UInt32 i=0; i<3; ++i )
                sizes[ i ] = 2.0f * shape.radius * shape.scale[ i ];
            if ( ASSETCONVERTER::PARTICLESHAPE_HEMISPHERE == shape.type )
                Approximated( "the hemisphere emits from the whole sphere" );
            inner = 1.0f - shape.radiusThickness;
            type = inner > 0.0f ? "HollowEllipsoid" : "Ellipsoid";
            break;
        case ASSETCONVERTER::PARTICLESHAPE_CONE:
        case ASSETCONVERTER::PARTICLESHAPE_CIRCLE:
            sizes[ 0 ] = 2.0f * shape.radius * shape.scale[ 0 ];
            sizes[ 2 ] = 2.0f * shape.radius * shape.scale[ 2 ];
            inner = 1.0f - shape.radiusThickness;
            type = inner > 0.0f ? "HollowEllipsoid" : "Ellipsoid";
            break;
        case ASSETCONVERTER::PARTICLESHAPE_RING:
        {
            // the outer and inner radius of a ring (Godot: radius and inner radius; Unity: the torus around the radius)
            Float32 outer = shape.radius;
            Float32 innerRadius = shape.ringInnerRadius;
            Float32 height = shape.ringHeight;
            if ( shape.ringRadius > 0.0f )
            {
                outer = shape.radius + shape.ringRadius;
                innerRadius = shape.radius - shape.ringRadius;
                height = 2.0f * shape.ringRadius;
            }
            sizes[ 0 ] = 2.0f * outer * shape.scale[ 0 ];
            sizes[ 1 ] = height * shape.scale[ 1 ];
            sizes[ 2 ] = 2.0f * outer * shape.scale[ 2 ];
            inner = outer > 0.0f && innerRadius > 0.0f ? innerRadius / outer : 0.0f;
            type = inner > 0.0f ? "HollowEllipsoid" : "Ellipsoid";
            Approximated( "the ring emits from a flat hollow ellipsoid (Ogre's ring emitter turns with the emission direction)" );
            break;
        }
        default:
            break;
    }
    if ( sizes[ 0 ] < 1.0e-4f && sizes[ 1 ] < 1.0e-4f && sizes[ 2 ] < 1.0e-4f )
        type = "Point";
    if ( shape.arc < 359.9f && ( ASSETCONVERTER::PARTICLESHAPE_CONE == shape.type || ASSETCONVERTER::PARTICLESHAPE_CIRCLE == shape.type || ASSETCONVERTER::PARTICLESHAPE_RING == shape.type ) )
        Approximated( "the arc of " + FormatNumber( shape.arc ) + " degrees of the shape is a full circle" );

    Line( 1, "emitter " + type );
    Line( 1, "{" );
    Line( 2, "direction " + Vector( direction ) );
    Line( 2, "up " + Vector( up ) );
    if ( "Point" != type )
    {
        if ( !IsAxisAligned( axes, frame ) && ( sizes[ 0 ] != sizes[ 1 ] || sizes[ 1 ] != sizes[ 2 ] ) )
            Approximated( "the shape is turned against the frame of the emitter: its sizes are its extents along that frame" );
        Line( 2, "width " + FormatNumber( Extent( axes, sizes, left ) ) );
        Line( 2, "height " + FormatNumber( Extent( axes, sizes, up ) ) );
        Line( 2, "depth " + FormatNumber( Extent( axes, sizes, direction ) ) );
        if ( "HollowEllipsoid" == type )
        {
            // Ogre asserts an inner size strictly between 0 and 1: a surface is 0.999 of the size
            inner = inner < 0.001f ? 0.001f : ( inner > 0.999f ? 0.999f : inner );
            Line( 2, "inner_width " + FormatNumber( inner ) );
            Line( 2, "inner_height " + FormatNumber( inner ) );
            Line( 2, "inner_depth " + FormatNumber( inner ) );
        }
    }
    Line( 2, "position " + Vector( shape.position ) );
    if ( outward && ASSETCONVERTER::PARTICLESHAPE_CONE != shape.type && "Point" != type )
        Line( 2, "direction_position_reference " + Vector( shape.position ) + " 1" );
    else if ( outward && "Point" == type && ASSETCONVERTER::PARTICLESHAPE_CONE != shape.type )
        angle = 180.0f;     // outward from a point: every direction
    Line( 2, "angle " + FormatNumber( angle ) );
    Line( 2, "velocity_min " + FormatNumber( speedMin < speedMax ? speedMin : speedMax ) );
    Line( 2, "velocity_max " + FormatNumber( speedMin < speedMax ? speedMax : speedMin ) );
    Line( 2, "time_to_live_min " + FormatNumber( system.startLifetime.GetMinimum() / speed ) );
    Line( 2, "time_to_live_max " + FormatNumber( system.startLifetime.GetMaximum() / speed ) );
    if ( withColour )
    {
        Float32 low[ 4 ], high[ 4 ];
        system.startColour.Evaluate( 0.0f, 0.0f, low );
        system.startColour.Evaluate( 0.0f, 1.0f, high );
        if ( ASSETCONVERTER::PARTICLECOLOUR_RANDOMFROMGRADIENT == system.startColour.mode )
        {
            system.startColour.gradientMax.Evaluate( 0.0f, low );
            system.startColour.gradientMax.Evaluate( 1.0f, high );
            Approximated( "a start colour picked from a gradient is random between the gradient's ends" );
        }
        for ( UInt32 i=0; i<4; ++i )
        {
            low[ i ] *= system.renderer.tint[ i ];
            high[ i ] *= system.renderer.tint[ i ];
        }
        Line( 2, "colour_range_start " + Colour( low ) );
        Line( 2, "colour_range_end " + Colour( high ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CParticleScriptWriter::WriteEmitters( const TSystem& system, Float32 speed )
{GUCEF_TRACE;

    const bool withColour = !m_colourAffector && system.renderer.useParticleColour;
    const Float32 cycle = system.duration > 0.0f ? system.duration : 5.0f;
    if ( !system.startColour.IsConstantOverTime() )
        Approximated( "a start colour that varies over the cycle of the system is taken at its start" );
    if ( !system.startLifetime.IsConstantOverTime() || !system.startSpeed.IsConstantOverTime() || !system.startSize.IsConstantOverTime() )
        Approximated( "start values that vary over the cycle of the system are taken over the whole cycle" );

    // the rate
    Float32 rate = system.rateOverTime.GetAverage() * speed;
    if ( !system.rateOverTime.IsConstant() )
        Approximated( "the emission rate that varies over the cycle is its mean" );
    const Float32 explosiveness = system.explosiveness;
    if ( rate > 0.0f && explosiveness < 0.999f )
    {
        WriteEmitterBody( system, speed, withColour );
        if ( explosiveness > 0.0f )
        {
            // the window (1 - e) of every cycle emits what the cycle emits
            Line( 2, "emission_rate " + FormatNumber( rate / ( 1.0f - explosiveness ) ) );
            Line( 2, "duration " + FormatNumber( ( 1.0f - explosiveness ) * cycle / speed ) );
            if ( system.looping )
                Line( 2, "repeat_delay " + FormatNumber( explosiveness * cycle / speed ) );
        }
        else
        {
            Line( 2, "emission_rate " + FormatNumber( rate ) );
            if ( !system.looping )
                Line( 2, "duration " + FormatNumber( cycle / speed ) );
        }
        Line( 1, "}" );
    }
    else
    if ( rate > 0.0f )
    {
        // everything at once
        WriteEmitterBody( system, speed, withColour );
        Line( 2, "emission_rate " + FormatNumber( rate * cycle / speed ) );
        Line( 2, "duration " + FormatNumber( OgreBurstDuration ) );
        if ( system.looping )
            Line( 2, "repeat_delay " + FormatNumber( cycle / speed ) );
        Line( 1, "}" );
    }
    if ( system.emissionRandomness > 0.0f )
        Approximated( "the randomness of the emission times is not carried" );

    // the bursts: Ogre emits emission_rate particles at once for a negative duration, again after the repeat delay
    for ( size_t b=0; b<system.bursts.size(); ++b )
    {
        const TSystem::SBurst& burst = system.bursts[ b ];
        const Float32 count = burst.count.GetAverage() * burst.probability;
        if ( count <= 0.0f )
            continue;
        if ( !burst.count.IsConstant() || burst.probability < 1.0f )
            Approximated( "the count of burst " + CORE::UInt32ToString( (UInt32) b ) + " is its mean" );
        if ( burst.time > 0.0f )
            Approximated( "burst " + CORE::UInt32ToString( (UInt32) b ) + " at " + FormatNumber( burst.time ) + " s fires at the start of the cycle" );
        WriteEmitterBody( system, speed, withColour );
        Line( 2, "emission_rate " + FormatNumber( count ) );
        Line( 2, "duration " + FormatNumber( OgreBurstDuration ) );
        Float32 repeat = 0.0f;
        if ( 0 == burst.cycles || burst.cycles > 1 )
        {
            repeat = burst.interval;
            if ( burst.cycles > 1 )
                Approximated( "burst " + CORE::UInt32ToString( (UInt32) b ) + " repeats for ever, not " + CORE::UInt32ToString( burst.cycles ) + " times" );
        }
        else
        if ( system.looping )
        {
            repeat = cycle;
        }
        if ( repeat > 0.0f )
            Line( 2, "repeat_delay " + FormatNumber( repeat / speed ) );
        Line( 1, "}" );
    }
    if ( !system.rateOverDistance.IsZero() )
        Skipped( "the emission over the distance the system moves has no counterpart in Ogre" );
    if ( !system.startDelay.IsZero() )
        Approximated( "the start delay is not in the script: it is recorded on the node (particleStartDelay)" );
}

/*-------------------------------------------------------------------------*/

void
CParticleScriptWriter::WriteAffectors( const TSystem& system, Float32 speed, Float32 width )
{GUCEF_TRACE;

    // gravity and a constant force: one linear force in the simulation space
    Float32 force[ 3 ] = { system.gravity[ 0 ], system.gravity[ 1 ], system.gravity[ 2 ] };
    if ( system.force.enabled )
    {
        Float32 added[ 3 ];
        system.force.GetAverage( added );
        for ( UInt32 i=0; i<3; ++i )
            force[ i ] += added[ i ];
        if ( !system.force.IsConstant() )
            Approximated( "a varying force over the lifetime is its mean" );
        if ( system.force.space != system.simulationSpace )
            Approximated( "the force in another space than the simulation is applied in the simulation space" );
    }
    if ( Length( force ) > 0.0f )
    {
        if ( ASSETCONVERTER::PARTICLESPACE_LOCAL == system.simulationSpace && Length( system.gravity ) > 0.0f )
            Approximated( "the gravity of a system that simulates in its own space turns with the node" );
        Line( 1, "affector LinearForce" );
        Line( 1, "{" );
        const Float32 scaled[ 3 ] = { force[ 0 ] * speed * speed, force[ 1 ] * speed * speed, force[ 2 ] * speed * speed };
        Line( 2, "force_vector " + Vector( scaled ) );
        Line( 2, "force_application add" );
        Line( 1, "}" );
    }

    // the colour over the lifetime
    if ( m_colourAffector )
    {
        TColourStageVector stages;
        Float32 error = 0.0f;
        BuildColourStages( system, 6, stages, error );
        if ( error > ColourTolerance )
            Approximated( "the colour over the lifetime is reduced to six stages (largest difference " + FormatNumber( error ) + ")" );
        Line( 1, "affector ColourInterpolator" );
        Line( 1, "{" );
        for ( UInt32 s=0; s<6; ++s )
        {
            const SColourStage& stage = stages[ s < stages.size() ? s : stages.size() - 1 ];
            Line( 2, "time" + CORE::UInt32ToString( s ) + " " + FormatNumber( s < stages.size() ? stage.time : 1.0f ) );
            Line( 2, "colour" + CORE::UInt32ToString( s ) + " " + Colour( stage.colour ) );
        }
        Line( 1, "}" );
    }

    // the size: a random start size as the scale range, the size curve as the slope of its line
    const Float32 minSize = system.startSize.GetMinimum();
    const Float32 maxSize = system.startSize.GetMaximum();
    const Float32 meanSize = system.startSize.GetAverage();
    Float32 rate = 0.0f;
    if ( system.sizeOverLifetimeEnabled )
    {
        Float32 intercept = 1.0f, slope = 0.0f, error = 0.0f;
        system.sizeOverLifetime.GetLinearFit( intercept, slope, error );
        const Float32 meanLifetime = system.startLifetime.GetAverage() / speed;
        const Float32 base = system.renderer.quadSize[ 0 ] * meanSize;
        rate = meanLifetime > 0.0f ? slope * base / meanLifetime : 0.0f;
        const Float32 peak = (Float32) fabs( system.sizeOverLifetime.GetMaximum() );
        if ( error > 0.05f * ( peak > 0.0f ? peak : 1.0f ) )
            Approximated( "the size over the lifetime is the line through its curve (Ogre's scaler is linear; largest difference " + FormatNumber( error ) + ")" );
        if ( 0.0f != rate && system.renderer.quadSize[ 0 ] != system.renderer.quadSize[ 1 ] )
            Approximated( "the scaler adds the same amount to the width and the height of the quad" );
        if ( system.sizeOverLifetimeSeparate )
            Approximated( "the size over the lifetime per axis is the size of X" );
    }
    if ( system.startSize3D )
        Approximated( "the 3D start size is the size of X" );
    const bool range = meanSize > 0.0f && ( minSize != maxSize );
    if ( range || 0.0f != rate )
    {
        Line( 1, "affector Scaler" );
        Line( 1, "{" );
        Line( 2, "rate " + FormatNumber( rate ) );
        if ( range )
            Line( 2, "scale_range " + FormatNumber( minSize / meanSize ) + " " + FormatNumber( maxSize / meanSize ) );
        Line( 1, "}" );
    }
    (void) width;

    // the rotation
    if ( !system.startRotation.IsZero() || !system.angularVelocity.IsZero() )
    {
        if ( !system.angularVelocity.IsConstantOverTime() )
            Approximated( "the angular velocity over the lifetime is its mean" );
        if ( system.startRotation3D )
            Approximated( "the 3D start rotation is the rotation about the view axis" );
        Line( 1, "affector Rotator" );
        Line( 1, "{" );
        Line( 2, "rotation_range_start " + FormatNumber( system.startRotation.GetMinimum() ) );
        Line( 2, "rotation_range_end " + FormatNumber( system.startRotation.GetMaximum() ) );
        Line( 2, "rotation_speed_range_start " + FormatNumber( system.angularVelocity.GetMinimum() * speed ) );
        Line( 2, "rotation_speed_range_end " + FormatNumber( system.angularVelocity.GetMaximum() * speed ) );
        Line( 1, "}" );
    }

    // noise: white noise on the direction
    if ( system.noise.enabled && !system.noise.strength.IsZero() )
    {
        Approximated( "the noise is Ogre's direction randomiser (white noise, not a coherent field)" );
        Line( 1, "affector DirectionRandomiser" );
        Line( 1, "{" );
        Line( 2, "randomness " + FormatNumber( system.noise.strength.GetAverage() * speed ) );
        Line( 2, "scope 1" );
        Line( 2, "keep_velocity true" );
        Line( 1, "}" );
    }

    // collision with planes
    if ( system.collision.enabled )
    {
        if ( system.collision.world )
            Skipped( "collision with the scene has no counterpart in ParticleFX" );
        for ( size_t p=0; p<system.collision.planes.size(); ++p )
        {
            Line( 1, "affector DeflectorPlane" );
            Line( 1, "{" );
            Line( 2, "plane_point " + Vector( system.collision.planes[ p ].point ) );
            Line( 2, "plane_normal " + Vector( system.collision.planes[ p ].normal ) );
            Line( 2, "bounce " + FormatNumber( system.collision.bounce ) );
            Line( 1, "}" );
        }
        if ( !system.collision.world && ( system.collision.dampen > 0.0f || system.collision.lifetimeLoss > 0.0f ) )
            Approximated( "the dampening and the lifetime loss of collisions are not carried" );
    }

    // the sprite sheet
    const TSystem::STextureSheet& sheet = system.textureSheet;
    if ( sheet.enabled && sheet.tilesX * sheet.tilesY > 1 )
    {
        const UInt32 frames = sheet.singleRow ? sheet.tilesX : sheet.tilesX * sheet.tilesY;
        UInt32 first = 0;
        if ( sheet.singleRow )
        {
            if ( sheet.rowIndex < 0 )
                Approximated( "a random row of the sprite sheet is the first row" );
            else
                first = (UInt32) sheet.rowIndex * sheet.tilesX;
        }
        const bool randomStart = !sheet.startFrame.IsConstant();
        if ( !randomStart && sheet.startFrame.GetAverage() > 0.0f )
            Approximated( "the start frame " + FormatNumber( sheet.startFrame.GetAverage() ) + " of the sprite sheet is the first frame" );
        Float32 intercept = 0.0f, slope = 1.0f, error = 0.0f;
        sheet.frameOverTime.GetLinearFit( intercept, slope, error );
        if ( error > 0.02f || fabs( intercept ) > 0.02f || fabs( slope - 1.0f ) > 0.02f )
            Approximated( "the frames over the lifetime are played evenly" );
        Float32 duration = 0.0f;
        bool animated = true;
        if ( sheet.fps > 0.0f )
        {
            duration = (Float32) frames / sheet.fps / speed;
        }
        else
        if ( sheet.cycles > 0.0f )
        {
            duration = ( 1.0f == sheet.cycles ) ? 0.0f : system.startLifetime.GetAverage() / speed / sheet.cycles;
            if ( 1.0f != sheet.cycles && !system.startLifetime.IsConstant() )
                Approximated( "several cycles of the sprite sheet per lifetime use the mean lifetime" );
        }
        else
        {
            animated = false;
        }
        if ( animated || randomStart )
        {
            Line( 1, "affector TextureAnimator" );
            Line( 1, "{" );
            Line( 2, "texcoord_start " + CORE::UInt32ToString( first ) );
            Line( 2, "texcoord_count " + CORE::UInt32ToString( frames ) );
            // a negative duration picks one frame at random and keeps it
            Line( 2, "duration " + FormatNumber( animated ? duration : -1.0f ) );
            if ( animated && randomStart )
                Line( 2, "random_offset true" );
            Line( 1, "}" );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CParticleScriptWriter::ReportUnsupported( const TSystem& system )
{GUCEF_TRACE;

    if ( !system.linearAcceleration.IsZero() )
        Skipped( "the linear acceleration along the velocity has no counterpart in ParticleFX" );
    if ( !system.radialAcceleration.IsZero() )
        Skipped( "the radial acceleration has no counterpart in ParticleFX" );
    if ( !system.tangentialAcceleration.IsZero() )
        Skipped( "the tangential acceleration has no counterpart in ParticleFX" );
    if ( !system.damping.IsZero() )
        Skipped( "the damping of the velocity has no counterpart in ParticleFX" );
    if ( system.limitSpeed )
        Skipped( "the velocity limit and its dampening have no counterpart in ParticleFX" );
    if ( !system.drag.IsZero() )
        Skipped( "the drag has no counterpart in ParticleFX" );
    if ( !system.subEmitters.empty() )
        Skipped( "sub-emitters have no counterpart in ParticleFX (its emitted emitters share the system and its material)" );
    if ( system.trails.enabled )
        Skipped( "particle trails have no counterpart in ParticleFX (its ribbon trails follow nodes)" );
    if ( system.renderer.softParticles )
        Skipped( "soft particles (the fade near surfaces) are not carried" );
    if ( system.renderer.lit )
        Approximated( "lit particles are drawn unlit (billboards have no normals for the lighting of the RTSS)" );
    if ( ASSETCONVERTER::PARTICLESORT_OLDESTFIRST == system.renderer.sortMode || ASSETCONVERTER::PARTICLESORT_YOUNGESTFIRST == system.renderer.sortMode )
        Approximated( "the draw order by age is not carried (Ogre sorts by distance only)" );
    if ( system.prewarm > 0.0f )
        Approximated( "the prewarm time is not in the script: it is recorded on the node (particlePrewarm)" );
    if ( system.emissionOverLifetimeEnabled && system.emissionOverLifetime.GetMaximum() > 0.0f )
        Approximated( "the brightening by the emission over the lifetime is clamped to the colour range of Ogre" );
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( system.renderer.tint[ i ] > 1.0f )
        {
            Approximated( "a tint above 1 (HDR) is clamped" );
            break;
        }
    }
    if ( !system.renderer.useParticleColour )
    {
        Float32 rgba[ 4 ];
        SampleColour( system, 0.5f, rgba );
        if ( rgba[ 0 ] != 1.0f || rgba[ 1 ] != 1.0f || rgba[ 2 ] != 1.0f || rgba[ 3 ] != 1.0f )
            Approximated( "the material does not use the particle colour, as in the source: the colours of the particles are not drawn" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CParticleScriptWriter::AddSystem( const CORE::CString& templateName, const CORE::CString& materialName, const TSystem& system )
{GUCEF_TRACE;

    m_subject = system.name.IsNULLOrEmpty() ? templateName : system.name;
    if ( ASSETCONVERTER::PARTICLERENDER_NONE == system.renderer.mode )
    {
        Skipped( "the particle system draws nothing: no template is written" );
        return false;
    }
    const Float32 speed = system.simulationSpeed > 0.0f ? system.simulationSpeed : 1.0f;
    TColourStageVector stages;
    Float32 error = 0.0f;
    m_colourAffector = system.renderer.useParticleColour && BuildColourStages( system, 6, stages, error );
    if ( m_colourAffector && !system.startColour.IsConstant() )
        Approximated( "a random start colour enters the colour over the lifetime as its mean" );

    if ( 0 != m_count )
        Line( 0, "" );
    Line( 0, "// " + m_subject );
    Line( 0, "particle_system " + Quote( templateName ) );
    Line( 0, "{" );
    Line( 1, "quota " + CORE::UInt32ToString( system.maxParticles > 0 ? system.maxParticles : 1 ) );
    Line( 1, "material " + Quote( materialName ) );
    const UInt32 sizeLine = (UInt32) m_text.Length();
    Line( 1, "cull_each false" );
    if ( ASSETCONVERTER::PARTICLESORT_DISTANCE == system.renderer.sortMode )
        Line( 1, "sorted true" );
    if ( ASSETCONVERTER::PARTICLESPACE_LOCAL == system.simulationSpace )
        Line( 1, "local_space true" );
    Float32 width = 1.0f, height = 1.0f;
    WriteRenderer( system, speed, width, height );

    // the size lines go before the renderer lines, at the place kept for them
    CORE::CString sizes = "    particle_width " + FormatNumber( width ) + "\n    particle_height " + FormatNumber( height ) + "\n";
    m_text = m_text.SubstrFromRange( 0, sizeLine ) + sizes + m_text.SubstrFromRange( sizeLine, m_text.Length() );

    Line( 0, "" );
    WriteEmitters( system, speed );
    WriteAffectors( system, speed, width );
    ReportUnsupported( system );
    Line( 0, "}" );
    ++m_count;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
