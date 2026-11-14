/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CTDISTANCEATTENUATION_H
#define GUCEF_MATH_CTDISTANCEATTENUATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_ETYPES_H
#include "gucefMATH_ETypes.h"
#define GUCEF_MATH_ETYPES_H
#endif /* GUCEF_MATH_ETYPES_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTPIECEWISELINEARCURVE_H
#include "gucefMATH_CTPiecewiseLinearCurve.h"
#define GUCEF_MATH_CTPIECEWISELINEARCURVE_H
#endif /* GUCEF_MATH_CTPIECEWISELINEARCURVE_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#include "gucefMATH_SpatialAudioFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Distance attenuation of a sound source with the models of OpenAL 1.1 and a curve model.
 *
 *  With the distance d and the reference distance dMin, the maximum distance dMax and the rolloff R, and with
 *  d' = clamp( d, dMin, dMax ) for the clamped models and the curve (the other models use d, not below 0):
 *
 *      none                  1
 *      inverse               dMin / ( dMin + R ( d' - dMin ) )
 *      linear                clamp( 1 - R ( d' - dMin ) / ( dMax - dMin ), 0, 1 )
 *      exponent              ( d' / dMin ) ^ ( -R )
 *      curve                 10 ^ ( curve( ( d' - dMin ) / ( dMax - dMin ) ) / 20 ), the curve is a piecewise
 *                            linear function of up to 16 points in decibels over the normalised distance
 *
 *  A degenerate configuration never produces NaN or an infinite gain: a denominator that is not positive, or a
 *  reference distance that is not positive, gives the gain 1; dMax <= dMin makes the linear and curve models
 *  step from 1 at d <= dMin to the end value beyond it.
 *
 *  The distance low pass cutoff follows the same distance: EvaluateLogLerpCutoff() interpolates the cutoff
 *  logarithmically from the near cutoff at dMin to the far cutoff at dMax.
 *
 *  A value type: no allocation. T is a floating point kind. Evaluate() and EvaluateLogLerpCutoff() are realtime safe.
 *  Serialize() writes, and Deserialize() reads, the settings as a data node: the attributes 'model' (text:
 *  none, inverse, inverseClamped, linear, linearClamped, exponent, exponentClamped, curve), 'minDistance',
 *  'maxDistance', 'rolloff' and a child node 'Curve' with one child 'Point' ( attributes 'x' and 'y' ) per point.
 */
template< typename T >
class CTDistanceAttenuation
{
    public:

    typedef T value_type;
    typedef CTPiecewiseLinearCurve< T, 16 > TCurve;

    /** Inverse clamped model, dMin 1, dMax 100, rolloff 1: the default of OpenAL */
    CTDistanceAttenuation( void )
        : m_model( DISTANCEMODEL_INVERSE_CLAMPED ) ,
          m_minDistance( 1 )                       ,
          m_maxDistance( 100 )                     ,
          m_rolloff( 1 )                           ,
          m_curve()
    {
    }

    CTDistanceAttenuation( TDistanceModel model, const T& minDistance, const T& maxDistance, const T& rolloff )
        : m_model( model )              ,
          m_minDistance( minDistance )  ,
          m_maxDistance( maxDistance )  ,
          m_rolloff( rolloff )          ,
          m_curve()
    {
    }

    TDistanceModel GetModel( void ) const { return m_model; }
    T GetMinDistance( void ) const { return m_minDistance; }
    T GetMaxDistance( void ) const { return m_maxDistance; }
    T GetRolloff( void ) const { return m_rolloff; }

    void SetModel( TDistanceModel model ) { m_model = model; }
    void SetMinDistance( const T& minDistance ) { m_minDistance = minDistance; }
    void SetMaxDistance( const T& maxDistance ) { m_maxDistance = maxDistance; }
    void SetRolloff( const T& rolloff ) { m_rolloff = rolloff; }

    /** The curve of the curve model: gain in decibels over the normalised distance in [0, 1] */
    TCurve& GetCurve( void ) { return m_curve; }
    const TCurve& GetCurve( void ) const { return m_curve; }

    /**
     *  Gain for a distance, from 0 up to 1e30: the unclamped models grow without bound toward the source, which
     *  is limited so that the result is always finite. A negative distance is taken as 0.
     */
    T Evaluate( const T& distance ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        const T gain = EvaluateUnlimited( distance );
        return gain < T( 1e30 ) ? gain : T( 1e30 );
    }

    private:

    T EvaluateUnlimited( const T& distance ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        T d = distance > T( 0 ) ? distance : T( 0 );
        const T dMin = m_minDistance;
        const T dMax = m_maxDistance;

        switch ( m_model )
        {
            case DISTANCEMODEL_INVERSE_CLAMPED:
            case DISTANCEMODEL_LINEAR_CLAMPED:
            case DISTANCEMODEL_EXPONENT_CLAMPED:
            case DISTANCEMODEL_CURVE:
            {
                d = Clamp( d, dMin, dMax < dMin ? dMin : dMax );
                break;
            }
            default:
                break;
        }

        switch ( m_model )
        {
            case DISTANCEMODEL_INVERSE:
            case DISTANCEMODEL_INVERSE_CLAMPED:
            {
                const T denominator = dMin + m_rolloff * ( d - dMin );
                if ( !( dMin > T( 0 ) ) || !( denominator > T( 0 ) ) )
                    return T( 1 );
                return dMin / denominator;
            }
            case DISTANCEMODEL_LINEAR:
            case DISTANCEMODEL_LINEAR_CLAMPED:
            {
                const T span = dMax - dMin;
                if ( !( span > T( 0 ) ) )
                    return d <= dMin ? T( 1 ) : T( 0 );
                return Clamp01( T( 1 ) - m_rolloff * ( d - dMin ) / span );
            }
            case DISTANCEMODEL_EXPONENT:
            case DISTANCEMODEL_EXPONENT_CLAMPED:
            {
                if ( !( dMin > T( 0 ) ) || !( d > T( 0 ) ) )
                    return T( 1 );
                return std::pow( d / dMin, -m_rolloff );
            }
            case DISTANCEMODEL_CURVE:
            {
                const T span = dMax - dMin;
                const T normalised = span > T( 0 ) ? ( d - dMin ) / span : ( d <= dMin ? T( 0 ) : T( 1 ) );
                return DecibelsToGain( m_curve.Evaluate( normalised ) );
            }
            case DISTANCEMODEL_NONE:
            default:
                return T( 1 );
        }
    }

    public:

    /**
     *  Cutoff frequency of the distance low pass: nearCutoff at dMin and closer, farCutoff at dMax and beyond,
     *  logarithmic in between ( exp( lerp( ln near, ln far, t ) ) ). A far cutoff that is not positive means the
     *  low pass is off and gives the near cutoff.
     */
    T EvaluateLogLerpCutoff( const T& distance, const T& nearCutoff, const T& farCutoff ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( farCutoff > T( 0 ) ) || !( nearCutoff > T( 0 ) ) )
            return nearCutoff;
        const T span = m_maxDistance - m_minDistance;
        const T t = span > T( 0 ) ? Clamp01( ( distance - m_minDistance ) / span ) : ( distance > m_minDistance ? T( 1 ) : T( 0 ) );
        return LogLerp( nearCutoff, farCutoff, t );
    }

    /** The name of a model as written by Serialize(), "none" for a value that is no model */
    static const char* GetModelName( TDistanceModel model )
    {
        switch ( model )
        {
            case DISTANCEMODEL_INVERSE:             return "inverse";
            case DISTANCEMODEL_INVERSE_CLAMPED:     return "inverseClamped";
            case DISTANCEMODEL_LINEAR:              return "linear";
            case DISTANCEMODEL_LINEAR_CLAMPED:      return "linearClamped";
            case DISTANCEMODEL_EXPONENT:            return "exponent";
            case DISTANCEMODEL_EXPONENT_CLAMPED:    return "exponentClamped";
            case DISTANCEMODEL_CURVE:               return "curve";
            default:                                return "none";
        }
    }

    /** The model of a name written by Serialize(); false, leaving the model alone, for an unknown name */
    static bool ParseModelName( const CORE::CString& name, TDistanceModel& model )
    {
        for ( Int32 i = (Int32) DISTANCEMODEL_NONE; i <= (Int32) DISTANCEMODEL_CURVE; ++i )
        {
            if ( name == GetModelName( (TDistanceModel) i ) )
            {
                model = (TDistanceModel) i;
                return true;
            }
        }
        return false;
    }

    /** Writes the settings into the node, replacing a 'Curve' child that is there already */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        while ( node.DelChild( "Curve" ) )
            ;

        if ( !node.SetAttribute( "model", CORE::CVariant( CORE::CString( GetModelName( m_model ) ) ) ) ||
             !node.SetAttribute( "minDistance", CORE::CVariant( m_minDistance ) )                      ||
             !node.SetAttribute( "maxDistance", CORE::CVariant( m_maxDistance ) )                      ||
             !node.SetAttribute( "rolloff", CORE::CVariant( m_rolloff ) )                              )
        {
            return false;
        }

        CORE::CDataNode* curveNode = node.AddChild( "Curve" );
        if ( GUCEF_NULL == curveNode )
            return false;
        for ( UInt32 i = 0; i < m_curve.GetPointCount(); ++i )
        {
            CORE::CDataNode* pointNode = curveNode->AddChild( "Point" );
            if ( GUCEF_NULL == pointNode ||
                 !pointNode->SetAttribute( "x", CORE::CVariant( m_curve.GetPointX( i ) ) ) ||
                 !pointNode->SetAttribute( "y", CORE::CVariant( m_curve.GetPointY( i ) ) ) )
            {
                return false;
            }
        }
        return true;
    }

    /**
     *  Reads the settings from the node, which is the node written by Serialize(). All four attributes are
     *  required; the 'Curve' child is optional. On any failure the object is left unchanged and false returned.
     */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        const CORE::CDataNode::TKeyValuePair* modelAttribute = node.GetAttribute( "model" );
        const CORE::CDataNode::TKeyValuePair* minAttribute = node.GetAttribute( "minDistance" );
        const CORE::CDataNode::TKeyValuePair* maxAttribute = node.GetAttribute( "maxDistance" );
        const CORE::CDataNode::TKeyValuePair* rolloffAttribute = node.GetAttribute( "rolloff" );
        if ( GUCEF_NULL == modelAttribute || GUCEF_NULL == minAttribute || GUCEF_NULL == maxAttribute || GUCEF_NULL == rolloffAttribute )
            return false;

        CTDistanceAttenuation loaded;
        if ( !ParseModelName( modelAttribute->second.AsString(), loaded.m_model ) )
            return false;
        loaded.m_minDistance = (T) minAttribute->second.AsFloat64();
        loaded.m_maxDistance = (T) maxAttribute->second.AsFloat64();
        loaded.m_rolloff = (T) rolloffAttribute->second.AsFloat64();

        const CORE::CDataNode* curveNode = node.FindChild( "Curve" );
        if ( GUCEF_NULL != curveNode )
        {
            for ( CORE::CDataNode::const_iterator i = curveNode->ConstBegin(); i != curveNode->ConstEnd(); ++i )
            {
                const CORE::CDataNode* pointNode = (*i);
                if ( pointNode->GetName() != "Point" )
                    continue;
                const CORE::CDataNode::TKeyValuePair* x = pointNode->GetAttribute( "x" );
                const CORE::CDataNode::TKeyValuePair* y = pointNode->GetAttribute( "y" );
                if ( GUCEF_NULL == x || GUCEF_NULL == y )
                    return false;
                if ( !loaded.m_curve.AddPoint( (T) x->second.AsFloat64(), (T) y->second.AsFloat64() ) )
                    return false;
            }
        }

        *this = loaded;
        return true;
    }

    bool operator==( const CTDistanceAttenuation& other ) const
    {
        return m_model == other.m_model && m_minDistance == other.m_minDistance && m_maxDistance == other.m_maxDistance &&
               m_rolloff == other.m_rolloff && m_curve == other.m_curve;
    }

    bool operator!=( const CTDistanceAttenuation& other ) const
    {
        return !( *this == other );
    }

    private:

    TDistanceModel m_model;
    T m_minDistance;
    T m_maxDistance;
    T m_rolloff;
    TCurve m_curve;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTDISTANCEATTENUATION_H ? */
