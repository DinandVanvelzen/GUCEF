/*
 *  assetpluginUNITY: the particle systems of Unity
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

#include "assetpluginUNITY_CUnityParticleReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef ASSETCONVERTER::CAssetParticleCurve TCurve;
typedef ASSETCONVERTER::CAssetParticleGradient TGradient;
typedef ASSETCONVERTER::CAssetParticleSystem TSystem;

const Float32 RadiansToDegrees = 57.2957795f;

/** Unity's default gravity (Physics.gravity); a package does not hold the project's setting */
const Float32 UnityGravity = 9.81f;

void
ReadKeys( const CORE::CDataNode* curve, Float32 scale, TCurve::TKeyVector& keys, bool& weighted )
{
    keys.clear();
    if ( GUCEF_NULL == curve )
        return;
    CUnityYamlReader::TNodeList items;
    CUnityYamlReader::GetItems( curve->FindChild( "m_Curve" ), items );
    for ( size_t i=0; i<items.size(); ++i )
    {
        const CORE::CDataNode& item = *items[ i ];
        TCurve::SKey key;
        key.time = CUnityYamlReader::GetFloat( item, "time", 0.0f );
        key.value = scale * CUnityYamlReader::GetFloat( item, "value", 0.0f );
        key.inTangent = scale * CUnityYamlReader::GetFloat( item, "inSlope", 0.0f );
        key.outTangent = scale * CUnityYamlReader::GetFloat( item, "outSlope", 0.0f );
        if ( 0 != CUnityYamlReader::GetInt32( item, "weightedMode", 0 ) )
            weighted = true;
        keys.push_back( key );
    }
}

/** The quaternion product a * b, (x, y, z, w) */
void
Multiply( const Float32 a[ 4 ], const Float32 b[ 4 ], Float32 out[ 4 ] )
{
    out[ 0 ] = a[ 3 ] * b[ 0 ] + a[ 0 ] * b[ 3 ] + a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
    out[ 1 ] = a[ 3 ] * b[ 1 ] - a[ 0 ] * b[ 2 ] + a[ 1 ] * b[ 3 ] + a[ 2 ] * b[ 0 ];
    out[ 2 ] = a[ 3 ] * b[ 2 ] + a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ] + a[ 2 ] * b[ 3 ];
    out[ 3 ] = a[ 3 ] * b[ 3 ] - a[ 0 ] * b[ 0 ] - a[ 1 ] * b[ 1 ] - a[ 2 ] * b[ 2 ];
}

/** A value of the shape module that is a mapping with "value" in newer files and a plain number in older ones */
Float32
ShapeValue( const CORE::CDataNode& shape, const char* key, Float32 fallback )
{
    const CORE::CDataNode* node = shape.FindChild( key );
    if ( GUCEF_NULL != node && GUCEF_DATATYPE_OBJECT == node->GetNodeType() )
        return CUnityYamlReader::GetFloat( *node, "value", fallback );
    return CUnityYamlReader::GetFloat( shape, key, fallback );
}

bool
Enabled( const CORE::CDataNode& system, const char* module, const CORE::CDataNode*& node )
{
    node = system.FindChild( module );
    return GUCEF_NULL != node && CUnityYamlReader::GetBool( *node, "enabled", false );
}

/** The Unity blend factor enum (UnityEngine.Rendering.BlendMode) of a material float; -1 when it is not there */
Int32
BlendFactor( const CORE::CDataNode* floats, const char* key )
{
    if ( GUCEF_NULL == floats )
        return -1;
    const CORE::CVariant& value = floats->GetAttributeValue( key );
    return value.IsNULLOrEmpty() ? -1 : (Int32) value.AsFloat32( -1.0f );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityParticleReader::CUnityParticleReader( CUnityProject& project, CUnityMaterialReader& materials )
    : m_project( project )
    , m_materials( materials )
    , m_name()
    , m_weighted( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnityParticleReader::~CUnityParticleReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TCurve
CUnityParticleReader::ReadCurve( const CORE::CDataNode* minMaxCurve, Float32 scale, bool& weighted )
{GUCEF_TRACE;

    if ( GUCEF_NULL == minMaxCurve )
        return TCurve( 0.0f );
    const Int32 state = CUnityYamlReader::GetInt32( *minMaxCurve, "minMaxState", 0 );
    const Float32 scalar = scale * CUnityYamlReader::GetFloat( *minMaxCurve, "scalar", 0.0f );
    TCurve::TKeyVector maxKeys;
    TCurve::TKeyVector minKeys;
    switch ( state )
    {
        case 1:
            ReadKeys( minMaxCurve->FindChild( "maxCurve" ), 1.0f, maxKeys, weighted );
            return TCurve::Curve( maxKeys, scalar );
        case 2:
            ReadKeys( minMaxCurve->FindChild( "minCurve" ), 1.0f, minKeys, weighted );
            ReadKeys( minMaxCurve->FindChild( "maxCurve" ), 1.0f, maxKeys, weighted );
            return TCurve::RandomBetweenCurves( minKeys, maxKeys, scalar );
        case 3:
        {
            // older files have no minScalar: the lower constant is the value of the lower curve times the scalar
            Float32 low = 0.0f;
            if ( CUnityYamlReader::HasScalar( *minMaxCurve, "minScalar" ) )
            {
                low = scale * CUnityYamlReader::GetFloat( *minMaxCurve, "minScalar", 0.0f );
            }
            else
            {
                ReadKeys( minMaxCurve->FindChild( "minCurve" ), 1.0f, minKeys, weighted );
                low = scalar * TCurve::EvaluateKeys( minKeys, 0.0f );
            }
            return TCurve::RandomBetween( low, scalar );
        }
        default:
            return TCurve( scalar );
    }
}

/*-------------------------------------------------------------------------*/

TGradient::SGradient
CUnityParticleReader::ReadGradientKeys( const CORE::CDataNode* gradient )
{GUCEF_TRACE;

    TGradient::SGradient result;
    if ( GUCEF_NULL == gradient )
        return result;
    const Int32 colourKeys = CUnityYamlReader::GetInt32( *gradient, "m_NumColorKeys", 2 );
    const Int32 alphaKeys = CUnityYamlReader::GetInt32( *gradient, "m_NumAlphaKeys", 2 );
    for ( Int32 k=0; k<8; ++k )
    {
        Float32 rgba[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
        CUnityYamlReader::GetComponents( *gradient, "key" + CORE::Int32ToString( k ), "rgba", rgba );
        if ( k < colourKeys )
            result.colourKeys.push_back( TGradient::SColourKey( CUnityYamlReader::GetFloat( *gradient, "ctime" + CORE::Int32ToString( k ), 0.0f ) / 65535.0f, rgba[ 0 ], rgba[ 1 ], rgba[ 2 ] ) );
        if ( k < alphaKeys )
            result.alphaKeys.push_back( TGradient::SAlphaKey( CUnityYamlReader::GetFloat( *gradient, "atime" + CORE::Int32ToString( k ), 0.0f ) / 65535.0f, rgba[ 3 ] ) );
    }
    result.stepped = 1 == CUnityYamlReader::GetInt32( *gradient, "m_Mode", 0 );
    return result;
}

/*-------------------------------------------------------------------------*/

TGradient
CUnityParticleReader::ReadGradient( const CORE::CDataNode* minMaxGradient )
{GUCEF_TRACE;

    TGradient result;
    if ( GUCEF_NULL == minMaxGradient )
        return result;
    Float32 low[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
    Float32 high[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
    CUnityYamlReader::GetComponents( *minMaxGradient, "minColor", "rgba", low );
    CUnityYamlReader::GetComponents( *minMaxGradient, "maxColor", "rgba", high );
    switch ( CUnityYamlReader::GetInt32( *minMaxGradient, "minMaxState", 0 ) )
    {
        case 1:
            return TGradient::Gradient( ReadGradientKeys( minMaxGradient->FindChild( "maxGradient" ) ) );
        case 2:
            return TGradient::RandomBetweenColours( low, high );
        case 3:
            result = TGradient::Gradient( ReadGradientKeys( minMaxGradient->FindChild( "maxGradient" ) ) );
            result.mode = ASSETCONVERTER::PARTICLECOLOUR_RANDOMBETWEENGRADIENTS;
            result.gradientMin = ReadGradientKeys( minMaxGradient->FindChild( "minGradient" ) );
            return result;
        case 4:
            result = TGradient::Gradient( ReadGradientKeys( minMaxGradient->FindChild( "maxGradient" ) ) );
            result.mode = ASSETCONVERTER::PARTICLECOLOUR_RANDOMFROMGRADIENT;
            return result;
        default:
            return TGradient::Colour( high );
    }
}

/*-------------------------------------------------------------------------*/

void
CUnityParticleReader::EulerToQuaternion( const Float32 degrees[ 3 ], Float32 q[ 4 ] )
{GUCEF_TRACE;

    const Float32 toHalfRadians = 0.5f * 0.0174532925f;
    const Float32 x[ 4 ] = { sinf( degrees[ 0 ] * toHalfRadians ), 0.0f, 0.0f, cosf( degrees[ 0 ] * toHalfRadians ) };
    const Float32 y[ 4 ] = { 0.0f, sinf( degrees[ 1 ] * toHalfRadians ), 0.0f, cosf( degrees[ 1 ] * toHalfRadians ) };
    const Float32 z[ 4 ] = { 0.0f, 0.0f, sinf( degrees[ 2 ] * toHalfRadians ), cosf( degrees[ 2 ] * toHalfRadians ) };
    Float32 yx[ 4 ];
    Multiply( y, x, yx );
    Multiply( yx, z, q );
}

/*-------------------------------------------------------------------------*/

TCurve
CUnityParticleReader::Curve( const CORE::CDataNode& module, const CORE::CString& key, Float32 scale )
{GUCEF_TRACE;

    return ReadCurve( module.FindChild( key ), scale, m_weighted );
}

/*-------------------------------------------------------------------------*/

TGradient
CUnityParticleReader::Gradient( const CORE::CDataNode& module, const CORE::CString& key )
{GUCEF_TRACE;

    return ReadGradient( module.FindChild( key ) );
}

/*-------------------------------------------------------------------------*/

void
CUnityParticleReader::ReadMain( const CORE::CDataNode& system, TSystem& out )
{GUCEF_TRACE;

    out.duration = CUnityYamlReader::GetFloat( system, "lengthInSec", 5.0f );
    out.looping = CUnityYamlReader::GetBool( system, "looping", true );
    out.playOnAwake = CUnityYamlReader::GetBool( system, "playOnAwake", true );
    out.prewarm = CUnityYamlReader::GetBool( system, "prewarm", false ) ? out.duration : 0.0f;
    out.startDelay = Curve( system, "startDelay" );
    out.simulationSpeed = CUnityYamlReader::GetFloat( system, "simulationSpeed", 1.0f );

    // moveWithTransform holds ParticleSystemSimulationSpace: 0 local, 1 world, 2 custom (inferred from the corpus, see
    // design section 5.7.3)
    const Int32 space = CUnityYamlReader::GetInt32( system, "moveWithTransform", 0 );
    out.simulationSpace = 0 == space ? ASSETCONVERTER::PARTICLESPACE_LOCAL : ASSETCONVERTER::PARTICLESPACE_WORLD;
    if ( 2 == space )
        m_project.Approximate( m_name, "the custom simulation space is the world" );
    if ( 0 != CUnityYamlReader::GetInt32( system, "scalingMode", 1 ) && 1 != CUnityYamlReader::GetInt32( system, "scalingMode", 1 ) )
        m_project.Approximate( m_name, "the scaling mode 'shape' (the transform scales only the shape) is not carried" );

    const CORE::CDataNode* initial = system.FindChild( "InitialModule" );
    if ( GUCEF_NULL == initial )
        return;
    out.maxParticles = (UInt32) CUnityYamlReader::GetInt32( *initial, "maxNumParticles", 1000 );
    out.startLifetime = Curve( *initial, "startLifetime" );
    out.startSpeed = Curve( *initial, "startSpeed" );
    out.startSize = Curve( *initial, "startSize" );
    out.startSize3D = CUnityYamlReader::GetBool( *initial, "size3D", false );
    if ( out.startSize3D )
    {
        out.startSizeY = Curve( *initial, "startSizeY" );
        out.startSizeZ = Curve( *initial, "startSizeZ" );
    }
    out.startRotation = Curve( *initial, "startRotation", RadiansToDegrees );
    out.startRotation3D = CUnityYamlReader::GetBool( *initial, "rotation3D", false );
    out.startColour = Gradient( *initial, "startColor" );
    if ( CUnityYamlReader::GetFloat( *initial, "randomizeRotationDirection", 0.0f ) > 0.0f && !out.startRotation.IsZero() )
        m_project.Approximate( m_name, "the random direction of the rotation (randomizeRotationDirection) is not carried" );

    const TCurve gravity = Curve( *initial, "gravityModifier" );
    if ( !gravity.IsConstant() )
        m_project.Approximate( m_name, "a varying gravity modifier is its mean" );
    out.gravity[ 0 ] = 0.0f;
    out.gravity[ 1 ] = -UnityGravity * gravity.GetAverage();
    out.gravity[ 2 ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CUnityParticleReader::ReadShape( const CORE::CDataNode& shapeModule, TSystem& out )
{GUCEF_TRACE;

    TSystem::SShape& shape = out.shape;

    // the transform of the shape: Unity's shapes have their axis along +Z, the canonical shape along +Y
    Float32 position[ 3 ] = { 0.0f, 0.0f, 0.0f };
    Float32 euler[ 3 ] = { 0.0f, 0.0f, 0.0f };
    Float32 scale[ 3 ] = { 1.0f, 1.0f, 1.0f };
    CUnityYamlReader::GetComponents( shapeModule, "m_Position", "xyz", position );
    CUnityYamlReader::GetComponents( shapeModule, "m_Rotation", "xyz", euler );
    const bool hasScale = CUnityYamlReader::GetComponents( shapeModule, "m_Scale", "xyz", scale );
    if ( !hasScale )
    {
        // before serialised version 5: the box sizes, no transform
        scale[ 0 ] = CUnityYamlReader::GetFloat( shapeModule, "boxX", 1.0f );
        scale[ 1 ] = CUnityYamlReader::GetFloat( shapeModule, "boxY", 1.0f );
        scale[ 2 ] = CUnityYamlReader::GetFloat( shapeModule, "boxZ", 1.0f );
    }
    Float32 rotation[ 4 ];
    EulerToQuaternion( euler, rotation );
    const Float32 h = 0.70710678f;
    const Float32 yToZ[ 4 ] = { h, 0.0f, 0.0f, h };

    const bool enabled = CUnityYamlReader::GetBool( shapeModule, "enabled", true );
    const Int32 type = CUnityYamlReader::GetInt32( shapeModule, "type", 4 );
    bool unityAxis = true;
    shape.radius = ShapeValue( shapeModule, "radius", 1.0f );
    shape.radiusThickness = CUnityYamlReader::GetFloat( shapeModule, "radiusThickness", 1.0f );
    shape.angle = CUnityYamlReader::GetFloat( shapeModule, "angle", 25.0f );
    shape.length = CUnityYamlReader::GetFloat( shapeModule, "length", 5.0f );
    shape.arc = ShapeValue( shapeModule, "arc", 360.0f );
    shape.directionMode = ASSETCONVERTER::PARTICLEDIRECTION_OUTWARD;
    shape.spreadAngle = 0.0f;
    if ( !enabled )
    {
        // without the shape module Unity emits from the origin along +Z (unverified by data)
        shape.type = ASSETCONVERTER::PARTICLESHAPE_POINT;
        shape.directionMode = ASSETCONVERTER::PARTICLEDIRECTION_FIXED;
        euler[ 0 ] = euler[ 1 ] = euler[ 2 ] = 0.0f;
        EulerToQuaternion( euler, rotation );
    }
    else
    {
        switch ( type )
        {
            case 0: shape.type = ASSETCONVERTER::PARTICLESHAPE_SPHERE; break;
            case 1: shape.type = ASSETCONVERTER::PARTICLESHAPE_SPHERE; shape.radiusThickness = 0.0f; break;
            case 2: shape.type = ASSETCONVERTER::PARTICLESHAPE_HEMISPHERE; break;
            case 3: shape.type = ASSETCONVERTER::PARTICLESHAPE_HEMISPHERE; shape.radiusThickness = 0.0f; break;
            case 4: shape.type = ASSETCONVERTER::PARTICLESHAPE_CONE; break;
            case 7: shape.type = ASSETCONVERTER::PARTICLESHAPE_CONE; shape.radiusThickness = 0.0f; break;
            case 8:
            case 9:
                shape.type = ASSETCONVERTER::PARTICLESHAPE_CONE;
                if ( 9 == type )
                    shape.radiusThickness = 0.0f;
                m_project.Approximate( m_name, "a cone that emits from its volume emits from its base" );
                break;
            case 5:
            case 15:
            case 16:
                shape.type = ASSETCONVERTER::PARTICLESHAPE_BOX;
                shape.directionMode = ASSETCONVERTER::PARTICLEDIRECTION_FIXED;
                if ( 5 != type )
                    m_project.Approximate( m_name, "a box that emits from its shell or edges emits from its volume" );
                break;
            case 10: shape.type = ASSETCONVERTER::PARTICLESHAPE_CIRCLE; break;
            case 11: shape.type = ASSETCONVERTER::PARTICLESHAPE_CIRCLE; shape.radiusThickness = 0.0f; break;
            case 12:
                // a line along X that emits along +Y (unverified by data)
                shape.type = ASSETCONVERTER::PARTICLESHAPE_EDGE;
                shape.directionMode = ASSETCONVERTER::PARTICLEDIRECTION_FIXED;
                unityAxis = false;
                break;
            case 17:
                shape.type = ASSETCONVERTER::PARTICLESHAPE_RING;
                shape.ringRadius = ShapeValue( shapeModule, "donutRadius", 0.2f );
                break;
            case 18:
                shape.type = ASSETCONVERTER::PARTICLESHAPE_BOX;
                shape.directionMode = ASSETCONVERTER::PARTICLEDIRECTION_FIXED;
                scale[ 2 ] = 0.0f;
                break;
            default:
                shape.type = ASSETCONVERTER::PARTICLESHAPE_MESH;
                shape.directionMode = ASSETCONVERTER::PARTICLEDIRECTION_FIXED;
                m_project.Approximate( m_name, "the emission shape (type " + CORE::Int32ToString( type ) + ": a mesh, a renderer or a sprite) is not read: a box of the shape's scale emits" );
                break;
        }
    }

    for ( UInt32 i=0; i<3; ++i )
        shape.position[ i ] = position[ i ];
    if ( unityAxis )
    {
        Multiply( rotation, yToZ, shape.rotation );
        // the canonical frame: X, Y (Unity's Z) and Z (Unity's Y)
        shape.scale[ 0 ] = scale[ 0 ]; shape.scale[ 1 ] = scale[ 2 ]; shape.scale[ 2 ] = scale[ 1 ];
    }
    else
    {
        for ( UInt32 i=0; i<4; ++i )
            shape.rotation[ i ] = rotation[ i ];
        for ( UInt32 i=0; i<3; ++i )
            shape.scale[ i ] = scale[ i ];
    }
    if ( ASSETCONVERTER::PARTICLESHAPE_BOX == shape.type || ASSETCONVERTER::PARTICLESHAPE_MESH == shape.type )
    {
        // a box is the unit cube scaled by the shape's scale
        for ( UInt32 i=0; i<3; ++i )
        {
            shape.boxSize[ i ] = shape.scale[ i ];
            shape.scale[ i ] = 1.0f;
        }
    }
    if ( ASSETCONVERTER::PARTICLESHAPE_EDGE == shape.type )
    {
        shape.boxSize[ 0 ] = 2.0f * shape.radius;
        shape.boxSize[ 1 ] = shape.boxSize[ 2 ] = 0.0f;
    }
    shape.GetAxis( shape.direction );

    const Float32 randomDirection = CUnityYamlReader::GetFloat( shapeModule, "randomDirectionAmount", 0.0f );
    const Float32 sphericalDirection = CUnityYamlReader::GetFloat( shapeModule, "sphericalDirectionAmount", 0.0f );
    shape.randomDirectionAmount = randomDirection > sphericalDirection ? randomDirection : sphericalDirection;
    if ( CUnityYamlReader::GetFloat( shapeModule, "randomPositionAmount", 0.0f ) > 0.0f )
        m_project.Skip( m_name, "the random position offset of the shape (randomPositionAmount) has no counterpart in the model" );
    if ( CUnityYamlReader::GetBool( shapeModule, "alignToDirection", false ) )
        m_project.Skip( m_name, "aligning the particles to the emission direction (alignToDirection) has no counterpart in the model" );
}

/*-------------------------------------------------------------------------*/

void
CUnityParticleReader::ReadEmission( const CORE::CDataNode& emission, TSystem& out )
{GUCEF_TRACE;

    if ( !CUnityYamlReader::GetBool( emission, "enabled", true ) )
    {
        out.rateOverTime = TCurve( 0.0f );
        return;
    }
    out.rateOverTime = Curve( emission, "rateOverTime" );
    out.rateOverDistance = Curve( emission, "rateOverDistance" );
    CUnityYamlReader::TNodeList items;
    CUnityYamlReader::GetItems( emission.FindChild( "m_Bursts" ), items );
    const Int32 count = CUnityYamlReader::GetInt32( emission, "m_BurstCount", (Int32) items.size() );
    for ( size_t i=0; i<items.size() && (Int32) i<count; ++i )
    {
        const CORE::CDataNode& item = *items[ i ];
        TSystem::SBurst burst;
        burst.time = CUnityYamlReader::GetFloat( item, "time", 0.0f );
        if ( GUCEF_NULL != item.FindChild( "countCurve" ) )
            burst.count = Curve( item, "countCurve" );
        else
            burst.count = TCurve::RandomBetween( CUnityYamlReader::GetFloat( item, "minCount", 0.0f ), CUnityYamlReader::GetFloat( item, "maxCount", 0.0f ) );
        burst.cycles = (UInt32) CUnityYamlReader::GetInt32( item, "cycleCount", 1 );
        burst.interval = CUnityYamlReader::GetFloat( item, "repeatInterval", 0.01f );
        burst.probability = CUnityYamlReader::GetFloat( item, "probability", 1.0f );
        out.bursts.push_back( burst );
    }
}

/*-------------------------------------------------------------------------*/

void
CUnityParticleReader::ReadModules( const CORE::CDataNode& system, TSystem& out, TSubEmitterReferenceVector& subEmitters )
{GUCEF_TRACE;

    const CORE::CDataNode* module = GUCEF_NULL;
    if ( Enabled( system, "SizeModule", module ) )
    {
        out.sizeOverLifetimeEnabled = true;
        out.sizeOverLifetime = Curve( *module, "curve" );
        out.sizeOverLifetimeSeparate = CUnityYamlReader::GetBool( *module, "separateAxes", false );
        if ( out.sizeOverLifetimeSeparate )
        {
            // with separate axes "curve" is X
            out.sizeOverLifetimeY = Curve( *module, "y" );
            out.sizeOverLifetimeZ = Curve( *module, "z" );
        }
    }
    if ( Enabled( system, "RotationModule", module ) )
    {
        out.angularVelocity = Curve( *module, "curve", RadiansToDegrees );
        if ( CUnityYamlReader::GetBool( *module, "separateAxes", false ) )
            m_project.Approximate( m_name, "the rotation over the lifetime about separate axes is its rotation about the view axis" );
    }
    if ( Enabled( system, "ColorModule", module ) )
    {
        out.colourOverLifetimeEnabled = true;
        out.colourOverLifetime = Gradient( *module, "gradient" );
    }
    if ( Enabled( system, "VelocityModule", module ) )
    {
        out.velocity.enabled = true;
        out.velocity.x = Curve( *module, "x" );
        out.velocity.y = Curve( *module, "y" );
        out.velocity.z = Curve( *module, "z" );
        out.velocity.space = CUnityYamlReader::GetBool( *module, "inWorldSpace", false ) ? ASSETCONVERTER::PARTICLESPACE_WORLD : ASSETCONVERTER::PARTICLESPACE_LOCAL;
        if ( GUCEF_NULL != module->FindChild( "speedModifier" ) )
            out.speedModifier = Curve( *module, "speedModifier" );
        const char* const unsupported[] = { "orbitalX", "orbitalY", "orbitalZ", "orbitalOffsetX", "orbitalOffsetY", "orbitalOffsetZ", "radial" };
        for ( UInt32 i=0; i<sizeof( unsupported ) / sizeof( unsupported[ 0 ] ); ++i )
        {
            if ( GUCEF_NULL != module->FindChild( unsupported[ i ] ) && !Curve( *module, unsupported[ i ] ).IsZero() )
                m_project.Skip( m_name, CORE::CString( "the velocity over the lifetime " ) + unsupported[ i ] + " has no counterpart in the model" );
        }
    }
    if ( Enabled( system, "ForceModule", module ) )
    {
        out.force.enabled = true;
        out.force.x = Curve( *module, "x" );
        out.force.y = Curve( *module, "y" );
        out.force.z = Curve( *module, "z" );
        out.force.space = CUnityYamlReader::GetBool( *module, "inWorldSpace", false ) ? ASSETCONVERTER::PARTICLESPACE_WORLD : ASSETCONVERTER::PARTICLESPACE_LOCAL;
        if ( CUnityYamlReader::GetBool( *module, "randomizePerFrame", false ) )
            m_project.Approximate( m_name, "a force chosen anew every frame (randomizePerFrame) is chosen once per particle" );
    }
    if ( Enabled( system, "ClampVelocityModule", module ) )
    {
        out.limitSpeed = true;
        out.speedLimit = Curve( *module, "magnitude" );
        out.speedLimitDampen = CUnityYamlReader::GetFloat( *module, "dampen", 0.0f );
        if ( GUCEF_NULL != module->FindChild( "drag" ) )
            out.drag = Curve( *module, "drag" );
        if ( CUnityYamlReader::GetBool( *module, "separateAxis", false ) )
            m_project.Approximate( m_name, "the velocity limit per axis is the limit of the speed" );
    }
    if ( Enabled( system, "NoiseModule", module ) )
    {
        out.noise.enabled = true;
        out.noise.strength = Curve( *module, "strength" );
        out.noise.frequency = CUnityYamlReader::GetFloat( *module, "frequency", 0.5f );
        out.noise.scrollSpeed = Curve( *module, "scrollSpeed" ).GetAverage();
        out.noise.octaves = (UInt32) CUnityYamlReader::GetInt32( *module, "octaves", 1 );
        if ( CUnityYamlReader::GetBool( *module, "separateAxes", false ) )
            m_project.Approximate( m_name, "the noise strength per axis is the strength of X" );
    }
    if ( Enabled( system, "CollisionModule", module ) )
    {
        out.collision.enabled = true;
        out.collision.world = 1 == CUnityYamlReader::GetInt32( *module, "type", 0 );
        out.collision.bounce = Curve( *module, "m_Bounce" ).GetAverage();
        out.collision.dampen = Curve( *module, "m_Dampen" ).GetAverage();
        out.collision.lifetimeLoss = Curve( *module, "m_EnergyLossOnCollision" ).GetAverage();
        if ( !out.collision.world )
            m_project.Skip( m_name, "the collision planes (transforms of the scene) are not resolved" );
    }
    if ( Enabled( system, "UVModule", module ) )
    {
        TSystem::STextureSheet& sheet = out.textureSheet;
        if ( 0 != CUnityYamlReader::GetInt32( *module, "mode", 0 ) )
        {
            m_project.Skip( m_name, "the sprite list of the texture sheet animation is not read" );
        }
        else
        {
            sheet.enabled = true;
            sheet.tilesX = (UInt32) CUnityYamlReader::GetInt32( *module, "tilesX", 1 );
            sheet.tilesY = (UInt32) CUnityYamlReader::GetInt32( *module, "tilesY", 1 );
            sheet.singleRow = 1 == CUnityYamlReader::GetInt32( *module, "animationType", 0 );
            const bool randomRow = CUnityYamlReader::HasScalar( *module, "rowMode" ) ? 1 == CUnityYamlReader::GetInt32( *module, "rowMode", 0 )
                                                                                      : CUnityYamlReader::GetBool( *module, "randomRow", false );
            sheet.rowIndex = randomRow ? -1 : CUnityYamlReader::GetInt32( *module, "rowIndex", 0 );
            sheet.frameOverTime = Curve( *module, "frameOverTime" );
            sheet.startFrame = Curve( *module, "startFrame" );
            sheet.cycles = CUnityYamlReader::GetFloat( *module, "cycles", 1.0f );
            const Int32 timeMode = CUnityYamlReader::GetInt32( *module, "timeMode", 0 );
            sheet.fps = 2 == timeMode ? CUnityYamlReader::GetFloat( *module, "fps", 30.0f ) : 0.0f;
            if ( 1 == timeMode )
                m_project.Approximate( m_name, "a texture sheet animated by speed is animated over the lifetime" );
        }
    }
    if ( Enabled( system, "SubModule", module ) )
    {
        CUnityYamlReader::TNodeList items;
        CUnityYamlReader::GetItems( module->FindChild( "subEmitters" ), items );
        for ( size_t i=0; i<items.size(); ++i )
        {
            const CUnityReference emitter = CUnityReference::Get( *items[ i ], "emitter" );
            if ( emitter.IsNull() )
                continue;
            SSubEmitterReference reference;
            reference.emitter = emitter.fileId;
            switch ( CUnityYamlReader::GetInt32( *items[ i ], "type", 0 ) )
            {
                case 1: reference.trigger = ASSETCONVERTER::PARTICLETRIGGER_COLLISION; break;
                case 2: reference.trigger = ASSETCONVERTER::PARTICLETRIGGER_DEATH; break;
                case 3: reference.trigger = ASSETCONVERTER::PARTICLETRIGGER_TRIGGER; break;
                case 4: reference.trigger = ASSETCONVERTER::PARTICLETRIGGER_MANUAL; break;
                default: reference.trigger = ASSETCONVERTER::PARTICLETRIGGER_BIRTH; break;
            }
            reference.probability = CUnityYamlReader::GetFloat( *items[ i ], "emitProbability", 1.0f );
            reference.inherit = (UInt32) CUnityYamlReader::GetInt32( *items[ i ], "properties", 0 );
            subEmitters.push_back( reference );
        }
    }
    if ( Enabled( system, "TrailModule", module ) )
    {
        out.trails.enabled = true;
        out.trails.lifetime = Curve( *module, "lifetime" );
        out.trails.widthOverTrail = Curve( *module, "widthOverTrail" );
        out.trails.colourOverTrail = Gradient( *module, "colorOverTrail" );
        out.trails.minVertexDistance = CUnityYamlReader::GetFloat( *module, "minVertexDistance", 0.2f );
        out.trails.worldSpace = CUnityYamlReader::GetBool( *module, "worldSpace", false );
        out.trails.inheritColour = CUnityYamlReader::GetBool( *module, "inheritParticleColor", true );
        if ( 0 != CUnityYamlReader::GetInt32( *module, "mode", 0 ) )
            m_project.Approximate( m_name, "ribbon trails (connecting the particles) are particle trails" );
    }

    const char* const reported[] = { "InheritVelocityModule", "LifetimeByEmitterSpeedModule", "ExternalForcesModule", "SizeBySpeedModule",
                                     "RotationBySpeedModule", "ColorBySpeedModule", "TriggerModule", "LightsModule", "CustomDataModule" };
    for ( UInt32 i=0; i<sizeof( reported ) / sizeof( reported[ 0 ] ); ++i )
    {
        if ( Enabled( system, reported[ i ], module ) )
            m_project.Skip( m_name, CORE::CString( "the module " ) + reported[ i ] + " has no counterpart in the model" );
    }
}

/*-------------------------------------------------------------------------*/

void
CUnityParticleReader::ReadMaterial( TSystem& out )
{GUCEF_TRACE;

    const ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    const Int32 material = out.renderer.material;
    if ( material < 0 || (size_t) material >= scene.materials.size() )
        return;
    const ASSETCONVERTER::CSourceMaterial& source = scene.materials[ material ].source;
    const CORE::CDataNode* floats = source.properties.FindChild( "floats" );
    const CORE::CDataNode* colours = source.properties.FindChild( "colors" );
    const CORE::CString kind = source.properties.GetAttributeValue( "shaderKind" ).AsString();

    // the explicit blend factors (UnityEngine.Rendering.BlendMode: 0 zero, 1 one, 2 dst colour, 3 src colour, 5 src
    // alpha, 10 one minus src alpha) say the most; then the URP surface and blend modes
    const Int32 sourceFactor = BlendFactor( floats, "_SrcBlend" );
    const Int32 destinationFactor = BlendFactor( floats, "_DstBlend" );
    const Int32 surface = BlendFactor( floats, "_Surface" );
    const Int32 blend = BlendFactor( floats, "_Blend" );
    bool known = true;
    if ( 5 == sourceFactor && 10 == destinationFactor )
        out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_ALPHA;
    else if ( 1 == sourceFactor && 10 == destinationFactor )
        out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_PREMULTIPLIED;
    else if ( ( 5 == sourceFactor || 1 == sourceFactor ) && 1 == destinationFactor )
        out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_ADDITIVE;
    else if ( ( 2 == sourceFactor && 0 == destinationFactor ) || ( 0 == sourceFactor && 3 == destinationFactor ) )
        out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_MULTIPLY;
    else if ( 1 == surface )
    {
        switch ( blend )
        {
            case 1: out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_PREMULTIPLIED; break;
            case 2: out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_ADDITIVE; break;
            case 3: out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_MULTIPLY; break;
            default: out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_ALPHA; break;
        }
    }
    else if ( 0 == surface )
        out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_OPAQUE;
    else
        known = false;
    if ( "builtinParticle" == kind && ( !known || ASSETCONVERTER::PARTICLEBLEND_OPAQUE == out.renderer.blendMode ) )
    {
        out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_ALPHA;
        m_project.Approximate( m_name, "the built-in particle shader " + source.properties.GetAttributeValue( "shaderReference" ).AsString() + " is not named in the package: alpha blending is assumed" );
    }
    else
    if ( !known )
    {
        out.renderer.blendMode = ASSETCONVERTER::PARTICLEBLEND_ALPHA;
        m_project.Approximate( m_name, "the material does not say how it blends: alpha blending is assumed" );
    }

    if ( GUCEF_NULL != floats && BlendFactor( floats, "_AlphaClip" ) == 1 )
    {
        Float32 cutoff = floats->GetAttributeValue( "_Alpha_Clip_Threshold" ).AsFloat32( -1.0f );
        if ( cutoff < 0.0f )
            cutoff = floats->GetAttributeValue( "_Cutoff" ).AsFloat32( 0.0f );
        out.renderer.alphaCutoff = cutoff > 0.0f && cutoff < 1.0f ? cutoff : 0.0f;
    }

    // the tint: _BaseColor (URP and the shader graphs), else _TintColor of the legacy particle shaders, which double
    // it (unverified), else _Color
    const char* const names[] = { "_BaseColor", "_TintColor", "_Color" };
    for ( UInt32 n=0; n<3; ++n )
    {
        const CORE::CDataNode* colour = GUCEF_NULL != colours ? colours->FindChild( names[ n ] ) : GUCEF_NULL;
        if ( GUCEF_NULL == colour || ( 1 == n && "builtinParticle" != kind ) )
            continue;
        const Float32 factor = 1 == n ? 2.0f : 1.0f;
        out.renderer.tint[ 0 ] = factor * colour->GetAttributeValue( "r" ).AsFloat32( 1.0f );
        out.renderer.tint[ 1 ] = factor * colour->GetAttributeValue( "g" ).AsFloat32( 1.0f );
        out.renderer.tint[ 2 ] = factor * colour->GetAttributeValue( "b" ).AsFloat32( 1.0f );
        out.renderer.tint[ 3 ] = factor * colour->GetAttributeValue( "a" ).AsFloat32( 1.0f );
        break;
    }
    out.renderer.lit = "standard" == kind || "standardSpecular" == kind || "urpLit" == kind || "hdrpLit" == kind || "legacyDiffuse" == kind;
}

/*-------------------------------------------------------------------------*/

void
CUnityParticleReader::ReadRenderer( const CORE::CDataNode& renderer, TSystem& out, CUnityReference& meshReference )
{GUCEF_TRACE;

    if ( !CUnityYamlReader::GetBool( renderer, "m_Enabled", true ) )
    {
        out.renderer.mode = ASSETCONVERTER::PARTICLERENDER_NONE;
        m_project.Skip( m_name, "the renderer of the particles is disabled: nothing is drawn" );
        return;
    }
    switch ( CUnityYamlReader::GetInt32( renderer, "m_RenderMode", 0 ) )
    {
        case 1: out.renderer.mode = ASSETCONVERTER::PARTICLERENDER_STRETCHED; out.renderer.alignment = ASSETCONVERTER::PARTICLEALIGNMENT_VELOCITY; break;
        case 2: out.renderer.mode = ASSETCONVERTER::PARTICLERENDER_HORIZONTAL; break;
        case 3: out.renderer.mode = ASSETCONVERTER::PARTICLERENDER_VERTICAL; break;
        case 4: out.renderer.mode = ASSETCONVERTER::PARTICLERENDER_MESH; meshReference = CUnityReference::Get( renderer, "m_Mesh" ); break;
        case 5: out.renderer.mode = ASSETCONVERTER::PARTICLERENDER_NONE; break;
        default: out.renderer.mode = ASSETCONVERTER::PARTICLERENDER_BILLBOARD; break;
    }
    if ( ASSETCONVERTER::PARTICLERENDER_BILLBOARD == out.renderer.mode )
    {
        // m_RenderAlignment: 0 view, 1 world, 2 local, 3 facing, 4 velocity
        switch ( CUnityYamlReader::GetInt32( renderer, "m_RenderAlignment", 0 ) )
        {
            case 1:
            case 2: out.renderer.mode = ASSETCONVERTER::PARTICLERENDER_ALIGNED; break;
            case 3: out.renderer.alignment = ASSETCONVERTER::PARTICLEALIGNMENT_FACING; break;
            case 4: out.renderer.alignment = ASSETCONVERTER::PARTICLEALIGNMENT_VELOCITY; break;
            default: break;
        }
    }
    switch ( CUnityYamlReader::GetInt32( renderer, "m_SortMode", 0 ) )
    {
        case 1: out.renderer.sortMode = ASSETCONVERTER::PARTICLESORT_DISTANCE; break;
        case 2: out.renderer.sortMode = ASSETCONVERTER::PARTICLESORT_OLDESTFIRST; break;
        case 3: out.renderer.sortMode = ASSETCONVERTER::PARTICLESORT_YOUNGESTFIRST; break;
        default: out.renderer.sortMode = ASSETCONVERTER::PARTICLESORT_NONE; break;
    }
    CUnityYamlReader::GetComponents( renderer, "m_Pivot", "xyz", out.renderer.pivot );
    out.renderer.lengthScale = CUnityYamlReader::GetFloat( renderer, "m_LengthScale", 2.0f );
    out.renderer.velocityScale = CUnityYamlReader::GetFloat( renderer, "m_VelocityScale", 0.0f );
    if ( CUnityYamlReader::GetFloat( renderer, "m_CameraVelocityScale", 0.0f ) != 0.0f )
        m_project.Skip( m_name, "the stretching by the camera's velocity has no counterpart in the model" );
    Float32 flip[ 3 ] = { 0.0f, 0.0f, 0.0f };
    CUnityYamlReader::GetComponents( renderer, "m_Flip", "xyz", flip );
    if ( flip[ 0 ] > 0.0f || flip[ 1 ] > 0.0f )
        m_project.Skip( m_name, "the random flip of the particles has no counterpart in the model" );

    // the materials: the particles' and the trails'
    CUnityYamlReader::TNodeList items;
    CUnityYamlReader::GetItems( renderer.FindChild( "m_Materials" ), items );
    for ( size_t i=0; i<items.size() && i<2; ++i )
    {
        CUnityReference reference;
        reference.Read( items[ i ] );
        if ( reference.IsNull() )
            continue;
        const Int32 material = m_materials.GetMaterial( reference, m_name );
        if ( 0 == i )
            out.renderer.material = material;
        else
        if ( out.trails.enabled )
            out.renderer.trailMaterial = material;
    }
    ReadMaterial( out );
}

/*-------------------------------------------------------------------------*/

Int32
CUnityParticleReader::Read( const CORE::CDataNode& system              ,
                            const CORE::CDataNode* renderer            ,
                            const CORE::CString& name                  ,
                            CUnityReference& meshReference             ,
                            TSubEmitterReferenceVector& subEmitters    )
{GUCEF_TRACE;

    m_name = name;
    m_weighted = false;
    meshReference = CUnityReference();
    subEmitters.clear();

    ASSETCONVERTER::CAssetParticleSystem out;
    out.name = name;
    ReadMain( system, out );
    const CORE::CDataNode* shape = system.FindChild( "ShapeModule" );
    if ( GUCEF_NULL != shape )
        ReadShape( *shape, out );
    const CORE::CDataNode* emission = system.FindChild( "EmissionModule" );
    if ( GUCEF_NULL != emission )
        ReadEmission( *emission, out );
    ReadModules( system, out, subEmitters );
    if ( GUCEF_NULL != renderer )
    {
        ReadRenderer( *renderer, out, meshReference );
    }
    else
    {
        out.renderer.mode = ASSETCONVERTER::PARTICLERENDER_NONE;
        m_project.Skip( m_name, "the particle system has no renderer: nothing is drawn" );
    }
    if ( m_weighted )
        m_project.Approximate( m_name, "weighted tangents of the curves are taken as plain tangents" );

    // the source: the names of the enabled modules
    CORE::CString::StringVector modules;
    for ( const CORE::CDataNode* child = system.GetFirstChild(); GUCEF_NULL != child; child = child->GetNext() )
        if ( child->GetName().EndsWith( "Module" ) && CUnityYamlReader::GetBool( *child, "enabled", false ) )
            modules.push_back( child->GetName() );
    out.source.SetValuesOfChildByName( "enabledModules", modules );
    out.source.SetAttribute( "serializedVersion", CUnityYamlReader::GetInt32( system, "serializedVersion", 0 ) );

    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    scene.particleSystems.push_back( out );
    return (Int32) scene.particleSystems.size() - 1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
