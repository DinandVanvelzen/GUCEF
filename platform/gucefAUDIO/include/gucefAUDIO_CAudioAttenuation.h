/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CAUDIOATTENUATION_H
#define GUCEF_AUDIO_CAUDIOATTENUATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_MATH_CTDISTANCEATTENUATION_H
#include "gucefMATH_CTDistanceAttenuation.h"
#define GUCEF_MATH_CTDISTANCEATTENUATION_H
#endif /* GUCEF_MATH_CTDISTANCEATTENUATION_H ? */

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How the loudness of a spatial sound follows its distance to the listener: the model and its parameters, and the
 *  configuration of them. The formulas are those of gucefMATH (MATH::CTDistanceAttenuation, the clamped models of
 *  OpenAL 1.1 and a curve), evaluated here on Float32; this class is the audio facing layer with the parameter
 *  limits and the configuration. A value type, evaluation is realtime safe.
 *
 *  The audio thread does not use this object itself but the plain evaluator it holds (GetEvaluator()), which is what the
 *  published spatial snapshots carry: no virtual functions, copied by value.
 *
 *  The configuration is the section "Attenuation": attributes model, minDistance, maxDistance, rolloff and a child "Curve"
 *  with one child "Point" per point (attributes distance, in 0 .. 1 over the span from minDistance to maxDistance, and
 *  gainDb). The model names are none, inverse, linear, exponential and curve, which are the clamped models of the design
 *  (the distance is limited to minDistance .. maxDistance), and inverseUnclamped, linearUnclamped and exponentialUnclamped for
 *  the models that are not limited. The names of gucefMATH (inverseClamped, linearClamped, exponent, exponentClamped, ...) are read as well.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioAttenuation : public CORE::CIConfigurable
{
    public:

    typedef MATH::CTDistanceAttenuation< Float32 > TEvaluator;
    typedef MATH::TDistanceModel TDistanceModel;

    static const CString ConfigSectionName;     /**< "Attenuation" */
    static const UInt32 MaxCurvePoints = 16;    /**< points of the curve of the curve model */

    /** The default of OpenAL: inverse model, clamped, minimum distance 1, maximum distance 100, rolloff 1 */
    CAudioAttenuation( void );

    CAudioAttenuation( TDistanceModel model, Float32 minDistance, Float32 maxDistance, Float32 rolloff );

    CAudioAttenuation( const CAudioAttenuation& src );

    CAudioAttenuation& operator=( const CAudioAttenuation& src );

    virtual ~CAudioAttenuation() GUCEF_VIRTUAL_OVERRIDE;

    bool operator==( const CAudioAttenuation& other ) const;
    bool operator!=( const CAudioAttenuation& other ) const;

    TDistanceModel GetModel( void ) const;
    void SetModel( TDistanceModel model );

    /** The reference distance: up to it the gain is 1 (clamped models). Never negative */
    Float32 GetMinDistance( void ) const;
    void SetMinDistance( Float32 distance );

    /** The distance from which the gain does not fall any further (clamped models). Never negative */
    Float32 GetMaxDistance( void ) const;
    void SetMaxDistance( Float32 distance );

    /** How fast the gain falls: the rolloff of the models. Never negative */
    Float32 GetRolloff( void ) const;
    void SetRolloff( Float32 rolloff );

    /** Removes all points of the curve of the curve model */
    void ClearCurve( void );

    /** Adds a point (or replaces the gain of the point at that distance): the gain in dB over the distance normalised to 0 .. 1. False when the curve is full or a value is not a number */
    bool AddCurvePoint( Float32 normalisedDistance, Float32 gainInDecibels );

    UInt32 GetCurvePointCount( void ) const;
    Float32 GetCurvePointDistance( UInt32 index ) const;
    Float32 GetCurvePointGainInDecibels( UInt32 index ) const;

    /** The gain, 0 .. 1e30, for a distance; a negative distance is taken as 0. Realtime safe */
    Float32 Evaluate( Float32 distance ) const GUCEF_AUDIO_REALTIME_CONTEXT;

    /** The cutoff of the distance low pass in Hz: nearCutoff up to the minimum distance, farCutoff from the maximum distance, logarithmic in between; farCutoff 0 or less means off and gives nearCutoff. Realtime safe */
    Float32 EvaluateCutoff( Float32 distance, Float32 nearCutoff, Float32 farCutoff ) const GUCEF_AUDIO_REALTIME_CONTEXT;

    /** The plain evaluator, for the published snapshots */
    const TEvaluator& GetEvaluator( void ) const;

    /** The name of a model as it is written to the configuration */
    static const char* ModelToString( TDistanceModel model );

    /** The reverse of ModelToString() without regard for case, and the names of gucefMATH. False, and 'model' as it was, for text that is not a model */
    static bool StringToModel( const CString& text, TDistanceModel& model );

    /** Writes the section "Attenuation" (replaced when it is there) */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Reads the section from the tree, or the tree itself when it is that section. False when there is no such section.
     *  What the section does not have keeps its value, names that are not valid are logged and ignored. A child "Curve" replaces the curve
     */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    TEvaluator m_evaluator;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOATTENUATION_H ? */
