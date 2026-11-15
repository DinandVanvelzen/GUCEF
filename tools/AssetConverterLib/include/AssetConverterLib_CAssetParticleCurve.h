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

#ifndef ASSETCONVERTERLIB_CASSETPARTICLECURVE_H
#define ASSETCONVERTERLIB_CASSETPARTICLECURVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A scalar of a particle system (a lifetime, a speed, a size, a rate, ...): a constant, random between two constants,
 *  a curve over a normalised time (0..1: the lifetime of a particle or the cycle of the system), or random between two
 *  curves. The curve keys are cubic Hermite with the slopes of the value per unit of time on both sides of a key, the
 *  form of Unity's AnimationCurve and Godot's Curve; a slope that is not finite makes the segment a step. The value of
 *  a curve is its keys times 'multiplier'; outside the keys the first and the last value hold. A value type.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetParticleCurve
{
    public:

    struct ASSETCONVERTERLIB_PUBLIC_CPP SKey
    {
        Float32 time;
        Float32 value;
        Float32 inTangent;
        Float32 outTangent;

        SKey( void );
        SKey( Float32 keyTime, Float32 keyValue, Float32 keyInTangent = 0.0f, Float32 keyOutTangent = 0.0f );
    };
    typedef GUCEF::vector< SKey > TKeyVector;

    TParticleValueMode mode;
    Float32 minimum;            /**< the constant, or the lower constant */
    Float32 maximum;            /**< the upper constant (equal to minimum for a constant) */
    Float32 multiplier;         /**< of the curves */
    TKeyVector curveMin;        /**< the lower curve (random between curves) */
    TKeyVector curveMax;        /**< the curve, or the upper curve */

    /** The constant 0 */
    CAssetParticleCurve( void );

    explicit CAssetParticleCurve( Float32 constant );

    static CAssetParticleCurve RandomBetween( Float32 low, Float32 high );

    static CAssetParticleCurve Curve( const TKeyVector& keys, Float32 curveMultiplier = 1.0f );

    static CAssetParticleCurve RandomBetweenCurves( const TKeyVector& low, const TKeyVector& high, Float32 curveMultiplier = 1.0f );

    /** Whether the value is the same at every time (a constant, two constants, or curves without variation) */
    bool IsConstantOverTime( void ) const;

    /** Whether the value is the same at every time and for every particle */
    bool IsConstant( void ) const;

    /** Whether the value is 0 at every time and for every particle */
    bool IsZero( void ) const;

    /** The value at the normalised time; 'random' (0..1) chooses between the lower and the upper constant or curve */
    Float32 Evaluate( Float32 time, Float32 random ) const;

    /** The lowest and the highest value over time and particles (curves sampled) */
    Float32 GetMinimum( void ) const;
    Float32 GetMaximum( void ) const;

    /** The mean over time and particles */
    Float32 GetAverage( void ) const;

    /** The mean over particles at the time */
    Float32 GetAverageAt( Float32 time ) const;

    /**
     *  The least-squares line through the mean value over time: value(t) ~ intercept + slope * t for t in 0..1, and the
     *  largest distance of the mean value from that line
     */
    void GetLinearFit( Float32& intercept, Float32& slope, Float32& maxError ) const;

    /** Multiplies every value (constants or the multiplier of the curves) */
    void Scale( Float32 factor );

    bool Serialize( CORE::CDataNode& node ) const;

    bool Deserialize( const CORE::CDataNode& node );

    /** The value of keys at a time, without a multiplier; 0 without keys */
    static Float32 EvaluateKeys( const TKeyVector& keys, Float32 time );

    /** The number of samples over time used by the minimum, the maximum, the mean and the linear fit */
    static const UInt32 SampleCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPARTICLECURVE_H ? */
