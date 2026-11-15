/*
 *  assetpluginGODOT: the particle systems of Godot
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#include "assetpluginGODOT_CGodotVariant.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H ? */

#include "assetpluginGODOT_CGodotParticleReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "godot";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef ASSETCONVERTER::CAssetParticleCurve TCurve;
typedef ASSETCONVERTER::CAssetParticleGradient TGradient;
typedef ASSETCONVERTER::CAssetParticleSystem TSystem;

CORE::CString
SectionType( const CGodotDocument& document, const CORE::CDataNode& section )
{
    CORE::CString type = section.GetAttributeValue( "type" ).AsString();
    if ( type.IsNULLOrEmpty() )
        type = document.GetResourceType();
    return type;
}

Float32
GetNumberOf( const CORE::CDataNode* section, const CORE::CString& key, Float32 fallback )
{
    if ( GUCEF_NULL == section )
        return fallback;
    return (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( *section, key ), fallback );
}

/** The keys scaled by a factor (a curve times a constant of the range) */
TCurve::TKeyVector
ScaledKeys( const TCurve::TKeyVector& keys, Float32 factor )
{
    TCurve::TKeyVector result = keys;
    for ( size_t k=0; k<result.size(); ++k )
    {
        result[ k ].value *= factor;
        result[ k ].inTangent *= factor;
        result[ k ].outTangent *= factor;
    }
    return result;
}

/** A plain copy of the properties of a section, for the "source" of the system */
void
CopyProperties( CORE::CDataNode& target, const CORE::CDataNode& section )
{
    const CORE::CDataNode* properties = section.FindChild( "properties" );
    if ( GUCEF_NULL == properties )
        return;
    for ( const CORE::CDataNode* property = properties->GetFirstChild(); GUCEF_NULL != property; property = property->GetNext() )
        CGodotVariant::AddPlain( target, property->GetName(), *property );
}

Float32
MaterialNumber( const ASSETCONVERTER::CAssetMaterial& material, const char* key, Float32 fallback )
{
    const CORE::CVariant& value = material.source.properties.GetAttributeValueOrChildValueByName( key );
    return value.IsNULLOrEmpty() ? fallback : value.AsFloat32( fallback );
}

bool
MaterialBool( const ASSETCONVERTER::CAssetMaterial& material, const char* key, bool fallback )
{
    const CORE::CVariant& value = material.source.properties.GetAttributeValueOrChildValueByName( key );
    return value.IsNULLOrEmpty() ? fallback : value.AsBool( fallback );
}

bool
MaterialColour( const ASSETCONVERTER::CAssetMaterial& material, const char* key, Float32 rgba[ 4 ] )
{
    CORE::CDataNode::TVariantVector values;
    if ( !material.source.properties.GetValuesOfChildByName( key, values ) || values.size() < 3 )
        return false;
    for ( UInt32 i=0; i<4; ++i )
        rgba[ i ] = i < values.size() ? values[ i ].AsFloat32( 1.0f ) : 1.0f;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGodotParticleReader::CGodotParticleReader( CIGodotResourceResolver& resolver, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report )
    : m_resolver( resolver )
    , m_scene( scene )
    , m_report( report )
    , m_document( GUCEF_NULL )
    , m_node( GUCEF_NULL )
    , m_processDocument( GUCEF_NULL )
    , m_process( GUCEF_NULL )
    , m_cpu( false )
    , m_name()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGodotParticleReader::~CGodotParticleReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGodotParticleReader::IsParticlesType( const CORE::CString& type )
{GUCEF_TRACE;

    return "GPUParticles3D" == type || "CPUParticles3D" == type;
}

/*-------------------------------------------------------------------------*/

void
CGodotParticleReader::Skipped( const CORE::CString& message )
{GUCEF_TRACE;

    m_report.AddSkipped( SourceName, m_name, message );
}

/*-------------------------------------------------------------------------*/

void
CGodotParticleReader::Approximated( const CORE::CString& message )
{GUCEF_TRACE;

    m_report.AddApproximation( SourceName, m_name, message );
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotParticleReader::Property( const CORE::CString& key ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_process )
        return GUCEF_NULL;
    CORE::CString name = key;
    if ( m_cpu )
    {
        // CPUParticles3D names the scale "scale_amount"
        if ( "scale_min" == key ) name = "scale_amount_min";
        else if ( "scale_max" == key ) name = "scale_amount_max";
        else if ( "scale_curve" == key ) name = "scale_amount_curve";
    }
    return CGodotVariant::FindProperty( *m_process, name );
}

/*-------------------------------------------------------------------------*/

Float32
CGodotParticleReader::Number( const CORE::CString& key, Float32 fallback ) const
{GUCEF_TRACE;

    return (Float32) CGodotVariant::GetNumber( Property( key ), fallback );
}

/*-------------------------------------------------------------------------*/

void
CGodotParticleReader::Vector3( const CORE::CString& key, const Float32 fallback[ 3 ], Float32 out[ 3 ] ) const
{GUCEF_TRACE;

    if ( !CGodotVariant::GetVector3( Property( key ), out ) )
    {
        out[ 0 ] = fallback[ 0 ];
        out[ 1 ] = fallback[ 1 ];
        out[ 2 ] = fallback[ 2 ];
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotParticleReader::ReadCurve( const CORE::CDataNode* value, TCurve::TKeyVector& keys )
{GUCEF_TRACE;

    keys.clear();
    const CGodotDocument* document = m_processDocument;
    const CORE::CDataNode* section = GUCEF_NULL;
    if ( GUCEF_NULL == value || GUCEF_NULL == document || !m_resolver.ResolveResourceSection( *document, value, document, section ) )
        return false;
    CORE::CString type = SectionType( *document, *section );
    if ( "CurveXYZTexture" == type )
    {
        Approximated( "a curve per axis (CurveXYZTexture) is read as its X curve" );
        const CORE::CDataNode* curve = CGodotVariant::FindProperty( *section, "curve_x" );
        if ( GUCEF_NULL == curve || !m_resolver.ResolveResourceSection( *document, curve, document, section ) )
            return false;
        type = SectionType( *document, *section );
    }
    if ( "CurveTexture" == type )
    {
        const CORE::CDataNode* curve = CGodotVariant::FindProperty( *section, "curve" );
        if ( GUCEF_NULL == curve || !m_resolver.ResolveResourceSection( *document, curve, document, section ) )
            return false;
        type = SectionType( *document, *section );
    }
    if ( "Curve" != type )
    {
        Skipped( "the curve resource of the type " + type + " is not read" );
        return false;
    }
    // _data: per point the position (x, y), the left and the right tangent and their modes
    CGodotVariant::TNumberVector numbers;
    if ( !CGodotVariant::GetNumbers( CGodotVariant::FindProperty( *section, "_data" ), numbers ) )
        return false;
    for ( size_t i=0; i+5<numbers.size(); i+=6 )
        keys.push_back( TCurve::SKey( (Float32) numbers[ i ], (Float32) numbers[ i+1 ], (Float32) numbers[ i+2 ], (Float32) numbers[ i+3 ] ) );
    return !keys.empty();
}

/*-------------------------------------------------------------------------*/

bool
CGodotParticleReader::ReadGradient( const CORE::CDataNode* value, TGradient::SGradient& gradient )
{GUCEF_TRACE;

    gradient = TGradient::SGradient();
    const CGodotDocument* document = m_processDocument;
    const CORE::CDataNode* section = GUCEF_NULL;
    if ( GUCEF_NULL == value || GUCEF_NULL == document || !m_resolver.ResolveResourceSection( *document, value, document, section ) )
        return false;
    CORE::CString type = SectionType( *document, *section );
    if ( "GradientTexture1D" == type || "GradientTexture2D" == type )
    {
        if ( "GradientTexture2D" == type )
            Approximated( "a two dimensional gradient texture is read as its gradient" );
        const CORE::CDataNode* inner = CGodotVariant::FindProperty( *section, "gradient" );
        if ( GUCEF_NULL == inner || !m_resolver.ResolveResourceSection( *document, inner, document, section ) )
            return false;
        type = SectionType( *document, *section );
    }
    if ( "Gradient" != type )
    {
        Skipped( "the gradient resource of the type " + type + " is not read" );
        return false;
    }
    // the defaults of Gradient: offsets 0 and 1, black to white
    CGodotVariant::TNumberVector offsets;
    CGodotVariant::TNumberVector colours;
    if ( !CGodotVariant::GetNumbers( CGodotVariant::FindProperty( *section, "offsets" ), offsets ) )
    {
        offsets.push_back( 0.0 );
        offsets.push_back( 1.0 );
    }
    if ( !CGodotVariant::GetNumbers( CGodotVariant::FindProperty( *section, "colors" ), colours ) )
    {
        const Float64 defaults[ 8 ] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
        colours.assign( defaults, defaults + 8 );
    }
    for ( size_t k=0; k<offsets.size() && 4*k+3<colours.size(); ++k )
    {
        const Float32 rgba[ 4 ] = { (Float32) colours[ 4*k ], (Float32) colours[ 4*k+1 ], (Float32) colours[ 4*k+2 ], (Float32) colours[ 4*k+3 ] };
        gradient.AddKey( (Float32) offsets[ k ], rgba );
    }
    const Int64 interpolation = CGodotVariant::GetInteger( CGodotVariant::FindProperty( *section, "interpolation_mode" ), 0 );
    gradient.stepped = 1 == interpolation;
    if ( 2 == interpolation )
        Approximated( "a cubic gradient is interpolated linearly" );
    return !gradient.colourKeys.empty();
}

/*-------------------------------------------------------------------------*/

TCurve
CGodotParticleReader::ReadRange( const CORE::CString& name, Float32 defaultMin, Float32 defaultMax )
{GUCEF_TRACE;

    const Float32 low = Number( name + "_min", defaultMin );
    const Float32 high = Number( name + "_max", defaultMax );
    TCurve::TKeyVector keys;
    if ( ReadCurve( Property( name + "_curve" ), keys ) )
    {
        if ( low == high )
            return TCurve::Curve( ScaledKeys( keys, low ) );
        return TCurve::RandomBetweenCurves( ScaledKeys( keys, low ), ScaledKeys( keys, high ) );
    }
    return TCurve::RandomBetween( low, high );
}

/*-------------------------------------------------------------------------*/

void
CGodotParticleReader::ReadEmission( TSystem& system )
{GUCEF_TRACE;

    const CORE::CDataNode& node = *m_node;
    const Float32 amount = GetNumberOf( &node, "amount", 8.0f );
    const Float32 lifetime = GetNumberOf( &node, "lifetime", 1.0f );
    const Float32 ratio = GetNumberOf( &node, "amount_ratio", 1.0f );
    system.maxParticles = (UInt32) ( amount > 1.0f ? amount : 1.0f );
    system.duration = lifetime;
    system.looping = !CGodotVariant::GetBool( CGodotVariant::FindProperty( node, "one_shot" ), false );
    system.playOnAwake = CGodotVariant::GetBool( CGodotVariant::FindProperty( node, "emitting" ), true );
    system.prewarm = GetNumberOf( &node, "preprocess", 0.0f );
    system.simulationSpeed = GetNumberOf( &node, "speed_scale", 1.0f );
    system.explosiveness = GetNumberOf( &node, "explosiveness", 0.0f );
    system.emissionRandomness = GetNumberOf( &node, "randomness", 0.0f );
    system.simulationSpace = CGodotVariant::GetBool( CGodotVariant::FindProperty( node, "local_coords" ), false ) ? ASSETCONVERTER::PARTICLESPACE_LOCAL : ASSETCONVERTER::PARTICLESPACE_WORLD;
    system.rateOverTime = TCurve( lifetime > 0.0f ? amount * ratio / lifetime : 0.0f );
    system.rateOverDistance = TCurve( 0.0f );

    const Float32 lifetimeRandomness = Number( "lifetime_randomness", 0.0f );
    system.startLifetime = TCurve::RandomBetween( lifetime * ( 1.0f - lifetimeRandomness ), lifetime );

    CGodotVariant::TNumberVector aabb;
    if ( CGodotVariant::GetNumbers( CGodotVariant::FindProperty( node, "visibility_aabb" ), aabb ) && aabb.size() >= 6 )
    {
        system.hasBounds = true;
        for ( UInt32 i=0; i<3; ++i )
        {
            system.boundsMin[ i ] = (Float32) aabb[ i ];
            system.boundsMax[ i ] = (Float32) ( aabb[ i ] + aabb[ 3 + i ] );
        }
    }

    switch ( CGodotVariant::GetInteger( CGodotVariant::FindProperty( node, "draw_order" ), 0 ) )
    {
        case 1: system.renderer.sortMode = ASSETCONVERTER::PARTICLESORT_OLDESTFIRST; break;
        case 2: system.renderer.sortMode = ASSETCONVERTER::PARTICLESORT_YOUNGESTFIRST; break;
        case 3: system.renderer.sortMode = ASSETCONVERTER::PARTICLESORT_DISTANCE; break;
        default: system.renderer.sortMode = ASSETCONVERTER::PARTICLESORT_NONE; break;
    }

    if ( CGodotVariant::GetBool( CGodotVariant::FindProperty( node, "trail_enabled" ), false ) )
    {
        system.trails.enabled = true;
        system.trails.lifetime = TCurve( GetNumberOf( &node, "trail_lifetime", 0.3f ) );
        system.trails.worldSpace = ASSETCONVERTER::PARTICLESPACE_WORLD == system.simulationSpace;
    }
    if ( 0 != CGodotVariant::GetInteger( CGodotVariant::FindProperty( node, "transform_align" ), 0 ) )
        Skipped( "the transform alignment of the particles (transform_align) has no counterpart in the model" );
    if ( GetNumberOf( &node, "draw_passes", 1.0f ) > 1.0f )
        Skipped( "only the first draw pass is carried" );
}

/*-------------------------------------------------------------------------*/

void
CGodotParticleReader::ReadShape( TSystem& system )
{GUCEF_TRACE;

    TSystem::SShape& shape = system.shape;
    const Float32 one[ 3 ] = { 1.0f, 1.0f, 1.0f };
    const Float32 zero[ 3 ] = { 0.0f, 0.0f, 0.0f };
    Vector3( "emission_shape_scale", one, shape.scale );
    Vector3( "emission_shape_offset", zero, shape.position );
    shape.radius = Number( "emission_sphere_radius", 1.0f );
    switch ( (Int32) Number( "emission_shape", 0.0f ) )
    {
        case 1: shape.type = ASSETCONVERTER::PARTICLESHAPE_SPHERE; shape.radiusThickness = 1.0f; break;
        case 2: shape.type = ASSETCONVERTER::PARTICLESHAPE_SPHERE; shape.radiusThickness = 0.0f; break;
        case 3:
        {
            Float32 extents[ 3 ];
            Vector3( "emission_box_extents", one, extents );
            shape.type = ASSETCONVERTER::PARTICLESHAPE_BOX;
            for ( UInt32 i=0; i<3; ++i )
                shape.boxSize[ i ] = 2.0f * extents[ i ];
            break;
        }
        case 4:
        case 5:
        {
            shape.type = ASSETCONVERTER::PARTICLESHAPE_POINTS;
            Approximated( "the emission points (a texture of points) are not read: the visibility box is the shape" );
            if ( system.hasBounds )
                for ( UInt32 i=0; i<3; ++i )
                    shape.boxSize[ i ] = system.boundsMax[ i ] - system.boundsMin[ i ];
            break;
        }
        case 6:
        {
            shape.type = ASSETCONVERTER::PARTICLESHAPE_RING;
            const Float32 zAxis[ 3 ] = { 0.0f, 0.0f, 1.0f };
            Float32 axis[ 3 ];
            Vector3( "emission_ring_axis", zAxis, axis );
            shape.radius = Number( "emission_ring_radius", 1.0f );
            shape.ringHeight = Number( "emission_ring_height", 1.0f );
            shape.ringInnerRadius = Number( "emission_ring_inner_radius", 0.0f );
            shape.ringRadius = 0.0f;
            const Float32 coneAngle = Number( "emission_ring_cone_angle", 90.0f );
            if ( fabs( coneAngle - 90.0f ) > 0.01f )
                Approximated( "the ring narrows as a cone (emission_ring_cone_angle " + CORE::ToString( coneAngle ) + "): it is a straight ring" );
            // the rotation that turns +Y onto the axis
            const Float32 length = (Float32) sqrt( axis[ 0 ] * axis[ 0 ] + axis[ 1 ] * axis[ 1 ] + axis[ 2 ] * axis[ 2 ] );
            if ( length > 0.0f )
            {
                Float32 a[ 3 ] = { axis[ 0 ] / length, axis[ 1 ] / length, axis[ 2 ] / length };
                // q = (Y x a, 1 + Y . a), normalised; the half turn about X for a = -Y
                Float32 q[ 4 ] = { a[ 2 ], 0.0f, -a[ 0 ], 1.0f + a[ 1 ] };
                if ( q[ 3 ] < 1.0e-6f )
                {
                    q[ 0 ] = 1.0f; q[ 1 ] = 0.0f; q[ 2 ] = 0.0f; q[ 3 ] = 0.0f;
                }
                const Float32 n = (Float32) sqrt( q[ 0 ] * q[ 0 ] + q[ 1 ] * q[ 1 ] + q[ 2 ] * q[ 2 ] + q[ 3 ] * q[ 3 ] );
                for ( UInt32 i=0; i<4; ++i )
                    shape.rotation[ i ] = q[ i ] / n;
            }
            break;
        }
        default: shape.type = ASSETCONVERTER::PARTICLESHAPE_POINT; break;
    }

    // Godot emits along 'direction' within 'spread' from any shape; a zero direction is +Z, as Godot's shader takes it
    const Float32 xAxis[ 3 ] = { 1.0f, 0.0f, 0.0f };
    Float32 direction[ 3 ];
    Vector3( "direction", xAxis, direction );
    const Float32 length = (Float32) sqrt( direction[ 0 ] * direction[ 0 ] + direction[ 1 ] * direction[ 1 ] + direction[ 2 ] * direction[ 2 ] );
    shape.directionMode = ASSETCONVERTER::PARTICLEDIRECTION_FIXED;
    if ( length > 0.0f )
    {
        for ( UInt32 i=0; i<3; ++i )
            shape.direction[ i ] = direction[ i ] / length;
    }
    else
    {
        shape.direction[ 0 ] = 0.0f; shape.direction[ 1 ] = 0.0f; shape.direction[ 2 ] = 1.0f;
    }
    shape.spreadAngle = Number( "spread", 45.0f );
    if ( Number( "flatness", 0.0f ) != 0.0f )
        Skipped( "the flatness of the spread has no counterpart in the model" );
}

/*-------------------------------------------------------------------------*/

void
CGodotParticleReader::ReadMotion( TSystem& system )
{GUCEF_TRACE;

    system.startSpeed = TCurve::RandomBetween( Number( "initial_velocity_min", 0.0f ), Number( "initial_velocity_max", 0.0f ) );
    const Float32 defaultGravity[ 3 ] = { 0.0f, -9.8f, 0.0f };
    Vector3( "gravity", defaultGravity, system.gravity );
    system.linearAcceleration = ReadRange( "linear_accel", 0.0f, 0.0f );
    system.radialAcceleration = ReadRange( "radial_accel", 0.0f, 0.0f );
    system.tangentialAcceleration = ReadRange( "tangential_accel", 0.0f, 0.0f );
    system.damping = ReadRange( "damping", 0.0f, 0.0f );
    system.angularVelocity = ReadRange( "angular_velocity", 0.0f, 0.0f );
    system.startRotation = TCurve::RandomBetween( Number( "angle_min", 0.0f ), Number( "angle_max", 0.0f ) );
    if ( GUCEF_NULL != Property( "angle_curve" ) )
        Approximated( "the angle curve over the lifetime is not carried: the start angle stays" );

    // turbulence: a coherent noise field; its strength in the model is the noise strength times the mean influence
    if ( CGodotVariant::GetBool( Property( "turbulence_enabled" ), false ) )
    {
        system.noise.enabled = true;
        const Float32 influence = 0.5f * ( Number( "turbulence_influence_min", 0.1f ) + Number( "turbulence_influence_max", 0.1f ) );
        system.noise.strength = TCurve( Number( "turbulence_noise_strength", 1.0f ) * influence );
        const Float32 scale = Number( "turbulence_noise_scale", 9.0f );
        system.noise.frequency = scale > 0.0f ? 1.0f / scale : 0.0f;
        const Float32 zero[ 3 ] = { 0.0f, 0.0f, 0.0f };
        Float32 speed[ 3 ];
        Vector3( "turbulence_noise_speed", zero, speed );
        system.noise.scrollSpeed = (Float32) sqrt( speed[ 0 ] * speed[ 0 ] + speed[ 1 ] * speed[ 1 ] + speed[ 2 ] * speed[ 2 ] );
        if ( Number( "turbulence_initial_displacement_max", 0.0f ) > 0.001f )
            Skipped( "the initial displacement of the turbulence has no counterpart in the model" );
    }

    const Int32 collision = (Int32) Number( "collision_mode", 0.0f );
    if ( 0 != collision )
    {
        system.collision.enabled = true;
        system.collision.world = true;
        system.collision.bounce = Number( "collision_bounce", 0.0f );
        system.collision.dampen = Number( "collision_friction", 0.0f );
        system.collision.lifetimeLoss = 2 == collision ? 1.0f : 0.0f;
    }

    const char* const unsupported[] = { "orbit_velocity", "radial_velocity", "directional_velocity", "scale_over_velocity", "hue_variation" };
    for ( UInt32 i=0; i<sizeof( unsupported ) / sizeof( unsupported[ 0 ] ); ++i )
    {
        const CORE::CString name = unsupported[ i ];
        if ( Number( name + "_min", 0.0f ) != 0.0f || Number( name + "_max", 0.0f ) != 0.0f )
            Skipped( "the property " + name + " has no counterpart in the model" );
    }
    if ( GUCEF_NULL != Property( "velocity_limit_curve" ) )
        Skipped( "the velocity limit curve has no counterpart in the model" );
    if ( Number( "inherit_velocity_ratio", 0.0f ) != 0.0f )
        Skipped( "the inherited velocity of the emitter has no counterpart in the model" );
    if ( CGodotVariant::GetBool( Property( "particle_flag_disable_z" ), false ) )
        Skipped( "the movement in the plane (particle_flag_disable_z) has no counterpart in the model" );
    if ( CGodotVariant::GetBool( Property( "particle_flag_align_y" ), false ) )
        system.renderer.alignment = ASSETCONVERTER::PARTICLEALIGNMENT_VELOCITY;
}

/*-------------------------------------------------------------------------*/

void
CGodotParticleReader::ReadAppearance( TSystem& system )
{GUCEF_TRACE;

    system.startSize = TCurve::RandomBetween( Number( "scale_min", 1.0f ), Number( "scale_max", 1.0f ) );
    TCurve::TKeyVector keys;
    if ( ReadCurve( Property( "scale_curve" ), keys ) )
    {
        system.sizeOverLifetimeEnabled = true;
        system.sizeOverLifetime = TCurve::Curve( keys );
    }

    Float32 colour[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
    CGodotVariant::GetColour( Property( "color" ), colour );
    TGradient::SGradient gradient;
    if ( ReadGradient( Property( "color_initial_ramp" ), gradient ) )
    {
        // a colour picked at random from the ramp, times the colour
        for ( size_t k=0; k<gradient.colourKeys.size(); ++k )
            for ( UInt32 i=0; i<3; ++i )
                gradient.colourKeys[ k ].colour[ i ] *= colour[ i ];
        for ( size_t k=0; k<gradient.alphaKeys.size(); ++k )
            gradient.alphaKeys[ k ].alpha *= colour[ 3 ];
        system.startColour = TGradient::Gradient( gradient );
        system.startColour.mode = ASSETCONVERTER::PARTICLECOLOUR_RANDOMFROMGRADIENT;
    }
    else
    {
        system.startColour = TGradient::Colour( colour );
    }
    if ( ReadGradient( Property( "color_ramp" ), gradient ) )
    {
        system.colourOverLifetimeEnabled = true;
        system.colourOverLifetime = TGradient::Gradient( gradient );
    }
    if ( ReadCurve( Property( "alpha_curve" ), keys ) )
    {
        system.alphaOverLifetimeEnabled = true;
        system.alphaOverLifetime = TCurve::Curve( keys );
    }
    if ( ReadCurve( Property( "emission_curve" ), keys ) )
    {
        system.emissionOverLifetimeEnabled = true;
        system.emissionOverLifetime = TCurve::Curve( keys );
    }
}

/*-------------------------------------------------------------------------*/

void
CGodotParticleReader::ReadMaterial( TSystem& system, Int32 material, Float32 animSpeed )
{GUCEF_TRACE;

    system.renderer.material = material;
    if ( material < 0 || (size_t) material >= m_scene.materials.size() )
        return;
    const ASSETCONVERTER::CAssetMaterial& source = m_scene.materials[ material ];

    // billboard_mode: 0 disabled, 1 enabled, 2 Y-billboard, 3 particle billboard (the only one that turns the quad
    // by the particle's angle)
    const Int32 billboard = (Int32) MaterialNumber( source, "billboard_mode", 0.0f );
    if ( ASSETCONVERTER::PARTICLERENDER_STRETCHED != system.renderer.mode && ASSETCONVERTER::PARTICLERENDER_MESH != system.renderer.mode )
    {
        switch ( billboard )
        {
            case 1:
                system.renderer.mode = ASSETCONVERTER::PARTICLERENDER_BILLBOARD;
                system.startRotation = TCurve( 0.0f );
                system.angularVelocity = TCurve( 0.0f );
                break;
            case 2: system.renderer.mode = ASSETCONVERTER::PARTICLERENDER_VERTICAL; break;
            case 3: system.renderer.mode = ASSETCONVERTER::PARTICLERENDER_BILLBOARD; break;
            default:
                if ( ASSETCONVERTER::PARTICLERENDER_HORIZONTAL == system.renderer.mode )
                    break;
                if ( CGodotVariant::GetBool( Property( "particle_flag_rotate_y" ), false ) )
                {
                    // quads that turn about Y by the particle's angle: seen from every side, as quads that turn about
                    // Y towards the camera
                    system.renderer.mode = ASSETCONVERTER::PARTICLERENDER_VERTICAL;
                    system.startRotation = TCurve( 0.0f );
                    system.angularVelocity = TCurve( 0.0f );
                    Approximated( "quads turning about Y by the particle's angle (particle_flag_rotate_y) are drawn as billboards that turn about Y towards the camera" );
                }
                else
                {
                    system.renderer.mode = ASSETCONVERTER::PARTICLERENDER_ALIGNED;
                }
                break;
        }
    }

    // transparency: 0 disabled, 1 alpha, 2 alpha scissor, 3 alpha hash, 4 depth pre-pass; blend_mode: 0 mix, 1 add,
    // 2 subtract, 3 multiply, 4 premultiplied alpha
    const Int32 transparency = (Int32) MaterialNumber( source, "transparency", 0.0f );
    switch ( (Int32) MaterialNumber( source, "blend_mode", 0.0f ) )
    {
        case 1: system.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_ADDITIVE; break;
        case 2: system.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_SUBTRACT; break;
        case 3: system.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_MULTIPLY; break;
        case 4: system.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_PREMULTIPLIED; break;
        default:
            system.renderer.blendMode = ( 1 == transparency || 4 == transparency ) ? ASSETCONVERTER::PARTICLEBLEND_ALPHA : ASSETCONVERTER::PARTICLEBLEND_OPAQUE;
            break;
    }
    if ( 2 == transparency || 3 == transparency )
        system.renderer.alphaCutoff = MaterialNumber( source, "alpha_scissor_threshold", 0.5f );

    Float32 tint[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
    if ( MaterialColour( source, "albedo_color", tint ) )
        for ( UInt32 i=0; i<4; ++i )
            system.renderer.tint[ i ] = tint[ i ];
    system.renderer.useParticleColour = MaterialBool( source, "vertex_color_use_as_albedo", false );
    system.renderer.lit = MaterialNumber( source, "shading_mode", 1.0f ) != 0.0f;
    system.renderer.softParticles = MaterialBool( source, "proximity_fade_enabled", false );

    const UInt32 framesX = (UInt32) MaterialNumber( source, "particles_anim_h_frames", 1.0f );
    const UInt32 framesY = (UInt32) MaterialNumber( source, "particles_anim_v_frames", 1.0f );
    if ( framesX * framesY > 1 )
    {
        ASSETCONVERTER::CAssetParticleSystem::STextureSheet& sheet = system.textureSheet;
        sheet.enabled = true;
        sheet.tilesX = framesX;
        sheet.tilesY = framesY;
        sheet.cycles = animSpeed;
        sheet.startFrame = TCurve::RandomBetween( Number( "anim_offset_min", 0.0f ), Number( "anim_offset_max", 0.0f ) );
        if ( animSpeed > 1.0f && !MaterialBool( source, "particles_anim_loop", false ) )
            Approximated( "the sprite sheet plays " + CORE::ToString( animSpeed ) + " times per lifetime without looping (Godot holds the last frame): it loops" );
    }
}

/*-------------------------------------------------------------------------*/

void
CGodotParticleReader::ReadDrawPass( TSystem& system )
{GUCEF_TRACE;

    const CORE::CDataNode* pass = CGodotVariant::FindProperty( *m_node, m_cpu ? "mesh" : "draw_pass_1" );
    const CGodotDocument* passDocument = m_document;
    const CORE::CDataNode* passSection = GUCEF_NULL;
    Int32 material = -1;
    const Float32 animSpeedMin = Number( "anim_speed_min", 0.0f );
    const Float32 animSpeedMax = Number( "anim_speed_max", 0.0f );
    if ( animSpeedMin != animSpeedMax )
        Approximated( "a random speed of the sprite sheet animation is its mean" );
    const Float32 animSpeed = 0.5f * ( animSpeedMin + animSpeedMax );

    if ( GUCEF_NULL != pass && m_resolver.ResolveResourceSection( *m_document, pass, passDocument, passSection ) )
    {
        const CORE::CString type = SectionType( *passDocument, *passSection );
        const CORE::CDataNode* materialValue = CGodotVariant::FindProperty( *passSection, "material" );
        if ( "QuadMesh" == type || "PlaneMesh" == type )
        {
            Float32 size[ 2 ] = { 1.0f, 1.0f };
            CGodotVariant::GetVector2( CGodotVariant::FindProperty( *passSection, "size" ), size );
            Float32 offset[ 3 ] = { 0.0f, 0.0f, 0.0f };
            CGodotVariant::GetVector3( CGodotVariant::FindProperty( *passSection, "center_offset" ), offset );
            system.renderer.quadSize[ 0 ] = size[ 0 ];
            system.renderer.quadSize[ 1 ] = size[ 1 ];
            system.renderer.pivot[ 0 ] = size[ 0 ] != 0.0f ? offset[ 0 ] / size[ 0 ] : 0.0f;
            system.renderer.pivot[ 1 ] = size[ 1 ] != 0.0f ? offset[ 1 ] / size[ 1 ] : 0.0f;
            system.renderer.pivot[ 2 ] = offset[ 2 ];
            // QuadMesh orientation: 0 face X, 1 face Y, 2 face Z (the default); a plane mesh faces Y
            const Int32 orientation = (Int32) CGodotVariant::GetInteger( CGodotVariant::FindProperty( *passSection, "orientation" ), "PlaneMesh" == type ? 1 : 2 );
            system.renderer.mode = 1 == orientation ? ASSETCONVERTER::PARTICLERENDER_HORIZONTAL : ASSETCONVERTER::PARTICLERENDER_BILLBOARD;
        }
        else
        if ( "RibbonTrailMesh" == type || "TubeTrailMesh" == type )
        {
            // a strip along Y: 'sections' of 'section_length', 'size' wide (RibbonTrailMesh) or 'radius' (tube),
            // times the width curve
            const Float32 sections = (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( *passSection, "sections" ), 5.0 );
            const Float32 sectionLength = (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( *passSection, "section_length" ), 0.2 );
            Float32 width = "RibbonTrailMesh" == type ? (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( *passSection, "size" ), 1.0 )
                                                      : 2.0f * (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( *passSection, "radius" ), 0.5 );
            const CGodotDocument* saved = m_processDocument;
            m_processDocument = passDocument;
            TCurve::TKeyVector keys;
            if ( ReadCurve( CGodotVariant::FindProperty( *passSection, "curve" ), keys ) )
                width *= TCurve::Curve( keys ).GetMaximum();
            m_processDocument = saved;
            system.renderer.quadSize[ 0 ] = width;
            system.renderer.quadSize[ 1 ] = sections * sectionLength;
            system.renderer.mode = ASSETCONVERTER::PARTICLERENDER_STRETCHED;
            system.renderer.alignment = ASSETCONVERTER::PARTICLEALIGNMENT_VELOCITY;
            system.renderer.lengthScale = 1.0f;
            if ( !system.trails.enabled )
                Approximated( "the " + type + " of the draw pass is drawn as a streak along the velocity" );
        }
        else
        {
            system.renderer.mode = ASSETCONVERTER::PARTICLERENDER_MESH;
            system.renderer.mesh = m_resolver.ResolveMeshSection( *passDocument, *passSection );
        }
        if ( GUCEF_NULL != materialValue )
            material = m_resolver.ResolveMaterialResource( *passDocument, materialValue );
    }
    else
    {
        system.renderer.mode = ASSETCONVERTER::PARTICLERENDER_NONE;
        Skipped( "the particles have no draw pass: nothing is drawn" );
    }

    // the material override of the node replaces the material of the mesh
    const CORE::CDataNode* materialOverride = CGodotVariant::FindProperty( *m_node, "material_override" );
    if ( GUCEF_NULL != materialOverride )
        material = m_resolver.ResolveMaterialResource( *m_document, materialOverride );
    ReadMaterial( system, material, animSpeed );
}

/*-------------------------------------------------------------------------*/

Int32
CGodotParticleReader::Read( const CGodotDocument& document                      ,
                            const CORE::CDataNode& section                      ,
                            const CORE::CString& type                           ,
                            const CORE::CString& nodeName                       ,
                            CORE::CString& subEmitterPath                       ,
                            ASSETCONVERTER::TParticleTrigger& subEmitterTrigger )
{GUCEF_TRACE;

    subEmitterPath.Clear();
    subEmitterTrigger = ASSETCONVERTER::PARTICLETRIGGER_BIRTH;
    if ( !IsParticlesType( type ) )
        return -1;

    m_document = &document;
    m_node = &section;
    m_cpu = "CPUParticles3D" == type;
    m_name = nodeName;
    m_processDocument = &document;
    m_process = m_cpu ? &section : GUCEF_NULL;

    TSystem system;
    system.name = nodeName;
    CopyProperties( *system.source.AddChild( "node", GUCEF_DATATYPE_OBJECT ), section );
    system.source.SetAttribute( "type", type );
    if ( !m_cpu )
    {
        const CORE::CDataNode* process = CGodotVariant::FindProperty( section, "process_material" );
        const CORE::CDataNode* processSection = GUCEF_NULL;
        const CGodotDocument* processDocument = &document;
        if ( GUCEF_NULL != process && m_resolver.ResolveResourceSection( document, process, processDocument, processSection ) )
        {
            if ( "ParticleProcessMaterial" == SectionType( *processDocument, *processSection ) )
            {
                m_process = processSection;
                m_processDocument = processDocument;
                CopyProperties( *system.source.AddChild( "processMaterial", GUCEF_DATATYPE_OBJECT ), *processSection );
            }
            else
            {
                Approximated( "the process material is a " + SectionType( *processDocument, *processSection ) + ": the defaults of a ParticleProcessMaterial are used" );
            }
        }
        else
        {
            Approximated( "the particles have no process material: the defaults of a ParticleProcessMaterial are used" );
        }
    }

    ReadEmission( system );
    ReadShape( system );
    ReadMotion( system );
    ReadAppearance( system );
    ReadDrawPass( system );

    // the sub-emitter is a node path; the mode is on the process material
    const CORE::CDataNode* subEmitter = CGodotVariant::FindProperty( section, "sub_emitter" );
    if ( GUCEF_NULL != subEmitter )
    {
        subEmitterPath = CGodotVariant::GetString( subEmitter );
        switch ( (Int32) Number( "sub_emitter_mode", 0.0f ) )
        {
            case 2: subEmitterTrigger = ASSETCONVERTER::PARTICLETRIGGER_DEATH; break;
            case 3: subEmitterTrigger = ASSETCONVERTER::PARTICLETRIGGER_COLLISION; break;
            case 4: subEmitterTrigger = ASSETCONVERTER::PARTICLETRIGGER_BIRTH; break;
            case 1: subEmitterTrigger = ASSETCONVERTER::PARTICLETRIGGER_BIRTH; break;
            default: subEmitterPath.Clear(); break;
        }
    }

    m_scene.particleSystems.push_back( system );
    m_document = GUCEF_NULL;
    m_node = GUCEF_NULL;
    m_process = GUCEF_NULL;
    m_processDocument = GUCEF_NULL;
    return (Int32) m_scene.particleSystems.size() - 1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
