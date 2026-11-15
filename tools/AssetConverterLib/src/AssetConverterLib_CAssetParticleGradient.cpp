/*
 *  AssetConverterLib: a colour of a particle system that can vary randomly and over time
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

#include <algorithm>

#include "AssetConverterLib_CAssetParticleGradient.h"

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
WriteColour( CORE::CDataNode& node, const CORE::CString& name, const Float32 rgba[ 4 ] )
{
    CORE::CDataNode::TVariantVector values;
    for ( UInt32 i=0; i<4; ++i )
        values.push_back( CORE::CVariant( rgba[ i ] ) );
    node.SetValuesOfChildByName( name, values );
}

bool
ReadColour( const CORE::CDataNode& node, const CORE::CString& name, Float32 rgba[ 4 ] )
{
    CORE::CDataNode::TVariantVector values;
    if ( !node.GetValuesOfChildByName( name, values ) || values.size() < 4 )
        return false;
    for ( UInt32 i=0; i<4; ++i )
        rgba[ i ] = values[ i ].AsFloat32( 1.0f );
    return true;
}

/** colour keys as time, r, g, b; alpha keys as time, a */
void
WriteGradient( CORE::CDataNode& parent, const CORE::CString& name, const CAssetParticleGradient::SGradient& gradient )
{
    CORE::CDataNode* node = parent.AddChild( name, GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode::TVariantVector colours;
    for ( size_t k=0; k<gradient.colourKeys.size(); ++k )
    {
        colours.push_back( CORE::CVariant( gradient.colourKeys[ k ].time ) );
        for ( UInt32 i=0; i<3; ++i )
            colours.push_back( CORE::CVariant( gradient.colourKeys[ k ].colour[ i ] ) );
    }
    node->SetValuesOfChildByName( "colourKeys", colours );
    CORE::CDataNode::TVariantVector alphas;
    for ( size_t k=0; k<gradient.alphaKeys.size(); ++k )
    {
        alphas.push_back( CORE::CVariant( gradient.alphaKeys[ k ].time ) );
        alphas.push_back( CORE::CVariant( gradient.alphaKeys[ k ].alpha ) );
    }
    node->SetValuesOfChildByName( "alphaKeys", alphas );
    node->SetAttribute( "stepped", gradient.stepped );
}

void
ReadGradient( const CORE::CDataNode& parent, const CORE::CString& name, CAssetParticleGradient::SGradient& gradient )
{
    gradient = CAssetParticleGradient::SGradient();
    const CORE::CDataNode* node = parent.FindChild( name );
    if ( GUCEF_NULL == node )
        return;
    CORE::CDataNode::TVariantVector values;
    node->GetValuesOfChildByName( "colourKeys", values );
    for ( size_t i=0; i+3<values.size(); i+=4 )
        gradient.colourKeys.push_back( CAssetParticleGradient::SColourKey( values[ i ].AsFloat32(), values[ i+1 ].AsFloat32(), values[ i+2 ].AsFloat32(), values[ i+3 ].AsFloat32() ) );
    values.clear();
    node->GetValuesOfChildByName( "alphaKeys", values );
    for ( size_t i=0; i+1<values.size(); i+=2 )
        gradient.alphaKeys.push_back( CAssetParticleGradient::SAlphaKey( values[ i ].AsFloat32(), values[ i+1 ].AsFloat32() ) );
    gradient.stepped = node->GetAttributeValueOrChildValueByName( "stepped" ).AsBool( false );
}

template < class TKey >
void
FindSegment( const GUCEF::vector< TKey >& keys, Float32 time, bool stepped, size_t& first, size_t& second, Float32& factor )
{
    first = second = 0;
    factor = 0.0f;
    if ( keys.empty() || time <= keys.front().time )
        return;
    if ( time >= keys.back().time )
    {
        first = second = keys.size() - 1;
        return;
    }
    for ( size_t k=0; k+1<keys.size(); ++k )
    {
        if ( time >= keys[ k ].time && time < keys[ k+1 ].time )
        {
            first = k;
            second = stepped ? k : k+1;
            const Float32 dt = keys[ k+1 ].time - keys[ k ].time;
            factor = dt > 0.0f ? ( time - keys[ k ].time ) / dt : 0.0f;
            return;
        }
    }
}

bool
SameColour( const Float32 a[ 4 ], const Float32 b[ 4 ] )
{
    return a[ 0 ] == b[ 0 ] && a[ 1 ] == b[ 1 ] && a[ 2 ] == b[ 2 ] && a[ 3 ] == b[ 3 ];
}

bool
IsFlat( const CAssetParticleGradient::SGradient& gradient )
{
    for ( size_t k=1; k<gradient.colourKeys.size(); ++k )
        for ( UInt32 i=0; i<3; ++i )
            if ( gradient.colourKeys[ k ].colour[ i ] != gradient.colourKeys[ 0 ].colour[ i ] )
                return false;
    for ( size_t k=1; k<gradient.alphaKeys.size(); ++k )
        if ( gradient.alphaKeys[ k ].alpha != gradient.alphaKeys[ 0 ].alpha )
            return false;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetParticleGradient::SColourKey::SColourKey( void )
    : time( 0.0f )
{GUCEF_TRACE;

    colour[ 0 ] = colour[ 1 ] = colour[ 2 ] = 1.0f;
}

/*-------------------------------------------------------------------------*/

CAssetParticleGradient::SColourKey::SColourKey( Float32 keyTime, Float32 red, Float32 green, Float32 blue )
    : time( keyTime )
{GUCEF_TRACE;

    colour[ 0 ] = red;
    colour[ 1 ] = green;
    colour[ 2 ] = blue;
}

/*-------------------------------------------------------------------------*/

CAssetParticleGradient::SAlphaKey::SAlphaKey( void )
    : time( 0.0f )
    , alpha( 1.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleGradient::SAlphaKey::SAlphaKey( Float32 keyTime, Float32 keyAlpha )
    : time( keyTime )
    , alpha( keyAlpha )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleGradient::SGradient::SGradient( void )
    : colourKeys()
    , alphaKeys()
    , stepped( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetParticleGradient::SGradient::AddKey( Float32 time, const Float32 rgba[ 4 ] )
{GUCEF_TRACE;

    colourKeys.push_back( SColourKey( time, rgba[ 0 ], rgba[ 1 ], rgba[ 2 ] ) );
    alphaKeys.push_back( SAlphaKey( time, rgba[ 3 ] ) );
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleGradient::SGradient::Evaluate( Float32 time, Float32 rgba[ 4 ] ) const
{GUCEF_TRACE;

    rgba[ 0 ] = rgba[ 1 ] = rgba[ 2 ] = rgba[ 3 ] = 1.0f;
    size_t a = 0, b = 0;
    Float32 f = 0.0f;
    if ( !colourKeys.empty() )
    {
        FindSegment( colourKeys, time, stepped, a, b, f );
        for ( UInt32 i=0; i<3; ++i )
            rgba[ i ] = colourKeys[ a ].colour[ i ] + ( colourKeys[ b ].colour[ i ] - colourKeys[ a ].colour[ i ] ) * f;
    }
    if ( !alphaKeys.empty() )
    {
        FindSegment( alphaKeys, time, stepped, a, b, f );
        rgba[ 3 ] = alphaKeys[ a ].alpha + ( alphaKeys[ b ].alpha - alphaKeys[ a ].alpha ) * f;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleGradient::SGradient::GetKeyTimes( GUCEF::vector< Float32 >& times ) const
{GUCEF_TRACE;

    for ( size_t k=0; k<colourKeys.size(); ++k )
        times.push_back( colourKeys[ k ].time );
    for ( size_t k=0; k<alphaKeys.size(); ++k )
        times.push_back( alphaKeys[ k ].time );
    std::sort( times.begin(), times.end() );
    times.erase( std::unique( times.begin(), times.end() ), times.end() );
}

/*-------------------------------------------------------------------------*/

CAssetParticleGradient::CAssetParticleGradient( void )
    : mode( PARTICLECOLOUR_COLOUR )
    , gradientMin()
    , gradientMax()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<4; ++i )
        colourMin[ i ] = colourMax[ i ] = 1.0f;
}

/*-------------------------------------------------------------------------*/

CAssetParticleGradient
CAssetParticleGradient::Colour( const Float32 rgba[ 4 ] )
{GUCEF_TRACE;

    CAssetParticleGradient result;
    for ( UInt32 i=0; i<4; ++i )
        result.colourMin[ i ] = result.colourMax[ i ] = rgba[ i ];
    return result;
}

/*-------------------------------------------------------------------------*/

CAssetParticleGradient
CAssetParticleGradient::RandomBetweenColours( const Float32 low[ 4 ], const Float32 high[ 4 ] )
{GUCEF_TRACE;

    CAssetParticleGradient result;
    result.mode = SameColour( low, high ) ? PARTICLECOLOUR_COLOUR : PARTICLECOLOUR_RANDOMBETWEENCOLOURS;
    for ( UInt32 i=0; i<4; ++i )
    {
        result.colourMin[ i ] = low[ i ];
        result.colourMax[ i ] = high[ i ];
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CAssetParticleGradient
CAssetParticleGradient::Gradient( const SGradient& gradient )
{GUCEF_TRACE;

    CAssetParticleGradient result;
    result.mode = PARTICLECOLOUR_GRADIENT;
    result.gradientMax = gradient;
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleGradient::IsConstantOverTime( void ) const
{GUCEF_TRACE;

    switch ( mode )
    {
        case PARTICLECOLOUR_GRADIENT: return IsFlat( gradientMax );
        case PARTICLECOLOUR_RANDOMBETWEENGRADIENTS: return IsFlat( gradientMin ) && IsFlat( gradientMax );
        default: return true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleGradient::IsConstant( void ) const
{GUCEF_TRACE;

    switch ( mode )
    {
        case PARTICLECOLOUR_COLOUR: return true;
        case PARTICLECOLOUR_RANDOMBETWEENCOLOURS: return SameColour( colourMin, colourMax );
        case PARTICLECOLOUR_GRADIENT: return IsFlat( gradientMax );
        case PARTICLECOLOUR_RANDOMFROMGRADIENT: return IsFlat( gradientMax );
        default:
        {
            if ( !IsFlat( gradientMin ) || !IsFlat( gradientMax ) )
                return false;
            Float32 a[ 4 ], b[ 4 ];
            gradientMin.Evaluate( 0.0f, a );
            gradientMax.Evaluate( 0.0f, b );
            return SameColour( a, b );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleGradient::IsWhite( void ) const
{GUCEF_TRACE;

    if ( !IsConstant() )
        return false;
    Float32 rgba[ 4 ];
    Evaluate( 0.0f, 0.0f, rgba );
    return 1.0f == rgba[ 0 ] && 1.0f == rgba[ 1 ] && 1.0f == rgba[ 2 ] && 1.0f == rgba[ 3 ];
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleGradient::Evaluate( Float32 time, Float32 random, Float32 rgba[ 4 ] ) const
{GUCEF_TRACE;

    switch ( mode )
    {
        case PARTICLECOLOUR_COLOUR:
        {
            for ( UInt32 i=0; i<4; ++i )
                rgba[ i ] = colourMin[ i ];
            return;
        }
        case PARTICLECOLOUR_RANDOMBETWEENCOLOURS:
        {
            for ( UInt32 i=0; i<4; ++i )
                rgba[ i ] = colourMin[ i ] + ( colourMax[ i ] - colourMin[ i ] ) * random;
            return;
        }
        case PARTICLECOLOUR_GRADIENT:
        {
            gradientMax.Evaluate( time, rgba );
            return;
        }
        case PARTICLECOLOUR_RANDOMFROMGRADIENT:
        {
            gradientMax.Evaluate( random, rgba );
            return;
        }
        default:
        {
            Float32 low[ 4 ], high[ 4 ];
            gradientMin.Evaluate( time, low );
            gradientMax.Evaluate( time, high );
            for ( UInt32 i=0; i<4; ++i )
                rgba[ i ] = low[ i ] + ( high[ i ] - low[ i ] ) * random;
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleGradient::GetAverageAt( Float32 time, Float32 rgba[ 4 ] ) const
{GUCEF_TRACE;

    if ( PARTICLECOLOUR_RANDOMFROMGRADIENT != mode )
    {
        Evaluate( time, 0.5f, rgba );
        return;
    }
    const UInt32 samples = 65;
    Float32 sum[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
    for ( UInt32 s=0; s<samples; ++s )
    {
        Float32 value[ 4 ];
        gradientMax.Evaluate( (Float32) s / (Float32) ( samples - 1 ), value );
        for ( UInt32 i=0; i<4; ++i )
            sum[ i ] += value[ i ];
    }
    for ( UInt32 i=0; i<4; ++i )
        rgba[ i ] = sum[ i ] / (Float32) samples;
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleGradient::GetKeyTimes( GUCEF::vector< Float32 >& times ) const
{GUCEF_TRACE;

    times.clear();
    times.push_back( 0.0f );
    times.push_back( 1.0f );
    if ( PARTICLECOLOUR_GRADIENT == mode || PARTICLECOLOUR_RANDOMBETWEENGRADIENTS == mode )
    {
        gradientMax.GetKeyTimes( times );
        if ( PARTICLECOLOUR_RANDOMBETWEENGRADIENTS == mode )
            gradientMin.GetKeyTimes( times );
    }
    std::sort( times.begin(), times.end() );
    times.erase( std::unique( times.begin(), times.end() ), times.end() );
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleGradient::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "mode", ToString( mode ) );
    switch ( mode )
    {
        case PARTICLECOLOUR_COLOUR:
            WriteColour( node, "colour", colourMin );
            break;
        case PARTICLECOLOUR_RANDOMBETWEENCOLOURS:
            WriteColour( node, "colourMin", colourMin );
            WriteColour( node, "colourMax", colourMax );
            break;
        case PARTICLECOLOUR_RANDOMBETWEENGRADIENTS:
            WriteGradient( node, "gradientMin", gradientMin );
            WriteGradient( node, "gradientMax", gradientMax );
            break;
        default:
            WriteGradient( node, "gradientMax", gradientMax );
            break;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleGradient::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetParticleGradient result;
    if ( !FromString( node.GetAttributeValueOrChildValueByName( "mode" ).AsString(), result.mode ) )
        return false;
    if ( ReadColour( node, "colour", result.colourMin ) )
        ReadColour( node, "colour", result.colourMax );
    ReadColour( node, "colourMin", result.colourMin );
    ReadColour( node, "colourMax", result.colourMax );
    ReadGradient( node, "gradientMin", result.gradientMin );
    ReadGradient( node, "gradientMax", result.gradientMax );
    *this = result;
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
