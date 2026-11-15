/*
 *  AssetConverterLib: a scalar value of a particle system that can vary randomly and over time
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

#include "AssetConverterLib_CAssetParticleCurve.h"

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

bool
IsFinite( Float32 value )
{
    return value == value && value < 1.0e29f && value > -1.0e29f;
}

/** The keys as one flat list of values: time, value, in tangent, out tangent per key (non-finite slopes as 1e30) */
void
WriteKeys( CORE::CDataNode& node, const CORE::CString& name, const CAssetParticleCurve::TKeyVector& keys )
{
    CORE::CDataNode::TVariantVector values;
    for ( size_t k=0; k<keys.size(); ++k )
    {
        values.push_back( CORE::CVariant( keys[ k ].time ) );
        values.push_back( CORE::CVariant( keys[ k ].value ) );
        values.push_back( CORE::CVariant( IsFinite( keys[ k ].inTangent ) ? keys[ k ].inTangent : 1.0e30f ) );
        values.push_back( CORE::CVariant( IsFinite( keys[ k ].outTangent ) ? keys[ k ].outTangent : 1.0e30f ) );
    }
    node.SetValuesOfChildByName( name, values );
}

void
ReadKeys( const CORE::CDataNode& node, const CORE::CString& name, CAssetParticleCurve::TKeyVector& keys )
{
    keys.clear();
    CORE::CDataNode::TVariantVector values;
    if ( !node.GetValuesOfChildByName( name, values ) )
        return;
    for ( size_t i=0; i+3<values.size(); i+=4 )
        keys.push_back( CAssetParticleCurve::SKey( values[ i ].AsFloat32(), values[ i+1 ].AsFloat32(), values[ i+2 ].AsFloat32(), values[ i+3 ].AsFloat32() ) );
}

bool
KeysAreFlat( const CAssetParticleCurve::TKeyVector& keys )
{
    for ( size_t k=0; k<keys.size(); ++k )
    {
        if ( keys[ k ].value != keys[ 0 ].value )
            return false;
        // a slope only bends the curve between keys
        if ( keys.size() > 1 && ( ( k > 0 && keys[ k ].inTangent != 0.0f && IsFinite( keys[ k ].inTangent ) ) ||
                                  ( k+1 < keys.size() && keys[ k ].outTangent != 0.0f && IsFinite( keys[ k ].outTangent ) ) ) )
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

const UInt32 CAssetParticleCurve::SampleCount = 65;

/*-------------------------------------------------------------------------*/

CAssetParticleCurve::SKey::SKey( void )
    : time( 0.0f )
    , value( 0.0f )
    , inTangent( 0.0f )
    , outTangent( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleCurve::SKey::SKey( Float32 keyTime, Float32 keyValue, Float32 keyInTangent, Float32 keyOutTangent )
    : time( keyTime )
    , value( keyValue )
    , inTangent( keyInTangent )
    , outTangent( keyOutTangent )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleCurve::CAssetParticleCurve( void )
    : mode( PARTICLEVALUE_CONSTANT )
    , minimum( 0.0f )
    , maximum( 0.0f )
    , multiplier( 1.0f )
    , curveMin()
    , curveMax()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleCurve::CAssetParticleCurve( Float32 constant )
    : mode( PARTICLEVALUE_CONSTANT )
    , minimum( constant )
    , maximum( constant )
    , multiplier( 1.0f )
    , curveMin()
    , curveMax()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetParticleCurve
CAssetParticleCurve::RandomBetween( Float32 low, Float32 high )
{GUCEF_TRACE;

    CAssetParticleCurve curve( low );
    curve.maximum = high;
    if ( low != high )
        curve.mode = PARTICLEVALUE_RANDOMBETWEENCONSTANTS;
    return curve;
}

/*-------------------------------------------------------------------------*/

CAssetParticleCurve
CAssetParticleCurve::Curve( const TKeyVector& keys, Float32 curveMultiplier )
{GUCEF_TRACE;

    CAssetParticleCurve curve;
    curve.mode = PARTICLEVALUE_CURVE;
    curve.curveMax = keys;
    curve.multiplier = curveMultiplier;
    return curve;
}

/*-------------------------------------------------------------------------*/

CAssetParticleCurve
CAssetParticleCurve::RandomBetweenCurves( const TKeyVector& low, const TKeyVector& high, Float32 curveMultiplier )
{GUCEF_TRACE;

    CAssetParticleCurve curve;
    curve.mode = PARTICLEVALUE_RANDOMBETWEENCURVES;
    curve.curveMin = low;
    curve.curveMax = high;
    curve.multiplier = curveMultiplier;
    return curve;
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleCurve::IsConstantOverTime( void ) const
{GUCEF_TRACE;

    switch ( mode )
    {
        case PARTICLEVALUE_CURVE: return KeysAreFlat( curveMax ) || 0.0f == multiplier;
        case PARTICLEVALUE_RANDOMBETWEENCURVES: return ( KeysAreFlat( curveMin ) && KeysAreFlat( curveMax ) ) || 0.0f == multiplier;
        default: return true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleCurve::IsConstant( void ) const
{GUCEF_TRACE;

    if ( !IsConstantOverTime() )
        return false;
    return GetMinimum() == GetMaximum();
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleCurve::IsZero( void ) const
{GUCEF_TRACE;

    return IsConstant() && 0.0f == GetMinimum();
}

/*-------------------------------------------------------------------------*/

Float32
CAssetParticleCurve::EvaluateKeys( const TKeyVector& keys, Float32 time )
{GUCEF_TRACE;

    if ( keys.empty() )
        return 0.0f;
    if ( time <= keys.front().time )
        return keys.front().value;
    if ( time >= keys.back().time )
        return keys.back().value;
    for ( size_t k=0; k+1<keys.size(); ++k )
    {
        const SKey& a = keys[ k ];
        const SKey& b = keys[ k+1 ];
        if ( time < a.time || time > b.time )
            continue;
        const Float32 dt = b.time - a.time;
        if ( dt <= 0.0f )
            return b.value;
        if ( !IsFinite( a.outTangent ) || !IsFinite( b.inTangent ) )
            return a.value;
        const Float32 s = ( time - a.time ) / dt;
        const Float32 s2 = s * s;
        const Float32 s3 = s2 * s;
        const Float32 h00 = 2.0f * s3 - 3.0f * s2 + 1.0f;
        const Float32 h10 = s3 - 2.0f * s2 + s;
        const Float32 h01 = -2.0f * s3 + 3.0f * s2;
        const Float32 h11 = s3 - s2;
        return h00 * a.value + h10 * dt * a.outTangent + h01 * b.value + h11 * dt * b.inTangent;
    }
    return keys.back().value;
}

/*-------------------------------------------------------------------------*/

Float32
CAssetParticleCurve::Evaluate( Float32 time, Float32 random ) const
{GUCEF_TRACE;

    switch ( mode )
    {
        case PARTICLEVALUE_CONSTANT: return minimum;
        case PARTICLEVALUE_RANDOMBETWEENCONSTANTS: return minimum + ( maximum - minimum ) * random;
        case PARTICLEVALUE_CURVE: return multiplier * EvaluateKeys( curveMax, time );
        case PARTICLEVALUE_RANDOMBETWEENCURVES:
        {
            const Float32 low = EvaluateKeys( curveMin, time );
            const Float32 high = EvaluateKeys( curveMax, time );
            return multiplier * ( low + ( high - low ) * random );
        }
        default: return 0.0f;
    }
}

/*-------------------------------------------------------------------------*/

Float32
CAssetParticleCurve::GetMinimum( void ) const
{GUCEF_TRACE;

    Float32 result = Evaluate( 0.0f, 0.0f );
    for ( UInt32 i=0; i<SampleCount; ++i )
    {
        const Float32 t = (Float32) i / (Float32) ( SampleCount - 1 );
        const Float32 a = Evaluate( t, 0.0f );
        const Float32 b = Evaluate( t, 1.0f );
        result = a < result ? a : result;
        result = b < result ? b : result;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

Float32
CAssetParticleCurve::GetMaximum( void ) const
{GUCEF_TRACE;

    Float32 result = Evaluate( 0.0f, 0.0f );
    for ( UInt32 i=0; i<SampleCount; ++i )
    {
        const Float32 t = (Float32) i / (Float32) ( SampleCount - 1 );
        const Float32 a = Evaluate( t, 0.0f );
        const Float32 b = Evaluate( t, 1.0f );
        result = a > result ? a : result;
        result = b > result ? b : result;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

Float32
CAssetParticleCurve::GetAverageAt( Float32 time ) const
{GUCEF_TRACE;

    // every mode is linear in the random factor
    return Evaluate( time, 0.5f );
}

/*-------------------------------------------------------------------------*/

Float32
CAssetParticleCurve::GetAverage( void ) const
{GUCEF_TRACE;

    if ( IsConstantOverTime() )
        return GetAverageAt( 0.0f );
    Float64 sum = 0.0;
    for ( UInt32 i=0; i<SampleCount; ++i )
        sum += GetAverageAt( (Float32) i / (Float32) ( SampleCount - 1 ) );
    return (Float32) ( sum / (Float64) SampleCount );
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleCurve::GetLinearFit( Float32& intercept, Float32& slope, Float32& maxError ) const
{GUCEF_TRACE;

    Float64 sumT = 0.0, sumV = 0.0, sumTT = 0.0, sumTV = 0.0;
    const Float64 n = (Float64) SampleCount;
    for ( UInt32 i=0; i<SampleCount; ++i )
    {
        const Float64 t = (Float64) i / (Float64) ( SampleCount - 1 );
        const Float64 v = GetAverageAt( (Float32) t );
        sumT += t; sumV += v; sumTT += t * t; sumTV += t * v;
    }
    const Float64 denominator = n * sumTT - sumT * sumT;
    const Float64 b = denominator != 0.0 ? ( n * sumTV - sumT * sumV ) / denominator : 0.0;
    const Float64 a = ( sumV - b * sumT ) / n;
    intercept = (Float32) a;
    slope = (Float32) b;
    maxError = 0.0f;
    for ( UInt32 i=0; i<SampleCount; ++i )
    {
        const Float32 t = (Float32) i / (Float32) ( SampleCount - 1 );
        const Float32 error = (Float32) fabs( GetAverageAt( t ) - ( a + b * t ) );
        maxError = error > maxError ? error : maxError;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetParticleCurve::Scale( Float32 factor )
{GUCEF_TRACE;

    minimum *= factor;
    maximum *= factor;
    multiplier *= factor;
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleCurve::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "mode", ToString( mode ) );
    switch ( mode )
    {
        case PARTICLEVALUE_CONSTANT:
            node.SetAttribute( "value", minimum );
            break;
        case PARTICLEVALUE_RANDOMBETWEENCONSTANTS:
            node.SetAttribute( "min", minimum );
            node.SetAttribute( "max", maximum );
            break;
        case PARTICLEVALUE_RANDOMBETWEENCURVES:
            WriteKeys( node, "curveMin", curveMin );
            node.SetAttribute( "multiplier", multiplier );
            WriteKeys( node, "curveMax", curveMax );
            break;
        default:
            node.SetAttribute( "multiplier", multiplier );
            WriteKeys( node, "curveMax", curveMax );
            break;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetParticleCurve::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetParticleCurve curve;
    if ( !FromString( node.GetAttributeValueOrChildValueByName( "mode" ).AsString(), curve.mode ) )
        return false;
    switch ( curve.mode )
    {
        case PARTICLEVALUE_CONSTANT:
            curve.minimum = curve.maximum = node.GetAttributeValueOrChildValueByName( "value" ).AsFloat32( 0.0f );
            break;
        case PARTICLEVALUE_RANDOMBETWEENCONSTANTS:
            curve.minimum = node.GetAttributeValueOrChildValueByName( "min" ).AsFloat32( 0.0f );
            curve.maximum = node.GetAttributeValueOrChildValueByName( "max" ).AsFloat32( 0.0f );
            break;
        default:
            curve.multiplier = node.GetAttributeValueOrChildValueByName( "multiplier" ).AsFloat32( 1.0f );
            ReadKeys( node, "curveMin", curve.curveMin );
            ReadKeys( node, "curveMax", curve.curveMax );
            break;
    }
    *this = curve;
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
