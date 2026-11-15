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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefAUDIO_CAudioAttenuation.h"

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** 'value' from 0 up; anything that is not a number gives 'fallback' */
Float32
NotNegative( Float32 value, Float32 fallback )
{
    if ( !( value == value ) )
        return fallback;
    return value < 0.0f ? 0.0f : value;
}

struct SModelName
{
    const char* name;
    MATH::TDistanceModel model;
};

/** The names of the configuration, the first of a model is the one that is written */
const SModelName ModelNames[] =
{
    { "none", MATH::DISTANCEMODEL_NONE },
    { "inverse", MATH::DISTANCEMODEL_INVERSE_CLAMPED },
    { "linear", MATH::DISTANCEMODEL_LINEAR_CLAMPED },
    { "exponential", MATH::DISTANCEMODEL_EXPONENT_CLAMPED },
    { "curve", MATH::DISTANCEMODEL_CURVE },
    { "inverseUnclamped", MATH::DISTANCEMODEL_INVERSE },
    { "linearUnclamped", MATH::DISTANCEMODEL_LINEAR },
    { "exponentialUnclamped", MATH::DISTANCEMODEL_EXPONENT },
    { "inverseClamped", MATH::DISTANCEMODEL_INVERSE_CLAMPED },
    { "linearClamped", MATH::DISTANCEMODEL_LINEAR_CLAMPED },
    { "exponent", MATH::DISTANCEMODEL_EXPONENT },
    { "exponentClamped", MATH::DISTANCEMODEL_EXPONENT_CLAMPED }
};

const UInt32 ModelNameCount = sizeof( ModelNames ) / sizeof( ModelNames[ 0 ] );

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CAudioAttenuation::ConfigSectionName = "Attenuation";

/*-------------------------------------------------------------------------*/

CAudioAttenuation::CAudioAttenuation( void )
    : CORE::CIConfigurable()
    , m_evaluator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioAttenuation::CAudioAttenuation( TDistanceModel model, Float32 minDistance, Float32 maxDistance, Float32 rolloff )
    : CORE::CIConfigurable()
    , m_evaluator()
{GUCEF_TRACE;

    SetModel( model );
    SetMinDistance( minDistance );
    SetMaxDistance( maxDistance );
    SetRolloff( rolloff );
}

/*-------------------------------------------------------------------------*/

CAudioAttenuation::CAudioAttenuation( const CAudioAttenuation& src )
    : CORE::CIConfigurable( src )
    , m_evaluator( src.m_evaluator )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioAttenuation&
CAudioAttenuation::operator=( const CAudioAttenuation& src )
{GUCEF_TRACE;

    if ( this != &src )
        m_evaluator = src.m_evaluator;
    return *this;
}

/*-------------------------------------------------------------------------*/

CAudioAttenuation::~CAudioAttenuation()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAudioAttenuation::operator==( const CAudioAttenuation& other ) const
{GUCEF_TRACE;

    return m_evaluator == other.m_evaluator;
}

/*-------------------------------------------------------------------------*/

bool
CAudioAttenuation::operator!=( const CAudioAttenuation& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CAudioAttenuation::TDistanceModel
CAudioAttenuation::GetModel( void ) const
{GUCEF_TRACE;

    return m_evaluator.GetModel();
}

/*-------------------------------------------------------------------------*/

void
CAudioAttenuation::SetModel( TDistanceModel model )
{GUCEF_TRACE;

    if ( (Int32) model >= (Int32) MATH::DISTANCEMODEL_NONE && (Int32) model <= (Int32) MATH::DISTANCEMODEL_CURVE )
        m_evaluator.SetModel( model );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioAttenuation::GetMinDistance( void ) const
{GUCEF_TRACE;

    return m_evaluator.GetMinDistance();
}

/*-------------------------------------------------------------------------*/

void
CAudioAttenuation::SetMinDistance( Float32 distance )
{GUCEF_TRACE;

    m_evaluator.SetMinDistance( NotNegative( distance, m_evaluator.GetMinDistance() ) );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioAttenuation::GetMaxDistance( void ) const
{GUCEF_TRACE;

    return m_evaluator.GetMaxDistance();
}

/*-------------------------------------------------------------------------*/

void
CAudioAttenuation::SetMaxDistance( Float32 distance )
{GUCEF_TRACE;

    m_evaluator.SetMaxDistance( NotNegative( distance, m_evaluator.GetMaxDistance() ) );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioAttenuation::GetRolloff( void ) const
{GUCEF_TRACE;

    return m_evaluator.GetRolloff();
}

/*-------------------------------------------------------------------------*/

void
CAudioAttenuation::SetRolloff( Float32 rolloff )
{GUCEF_TRACE;

    m_evaluator.SetRolloff( NotNegative( rolloff, m_evaluator.GetRolloff() ) );
}

/*-------------------------------------------------------------------------*/

void
CAudioAttenuation::ClearCurve( void )
{GUCEF_TRACE;

    m_evaluator.GetCurve().Clear();
}

/*-------------------------------------------------------------------------*/

bool
CAudioAttenuation::AddCurvePoint( Float32 normalisedDistance, Float32 gainInDecibels )
{GUCEF_TRACE;

    return m_evaluator.GetCurve().AddPoint( normalisedDistance, gainInDecibels );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioAttenuation::GetCurvePointCount( void ) const
{GUCEF_TRACE;

    return m_evaluator.GetCurve().GetPointCount();
}

/*-------------------------------------------------------------------------*/

Float32
CAudioAttenuation::GetCurvePointDistance( UInt32 index ) const
{GUCEF_TRACE;

    return m_evaluator.GetCurve().GetPointX( index );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioAttenuation::GetCurvePointGainInDecibels( UInt32 index ) const
{GUCEF_TRACE;

    return m_evaluator.GetCurve().GetPointY( index );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioAttenuation::Evaluate( Float32 distance ) const GUCEF_AUDIO_REALTIME_CONTEXT
{
    return m_evaluator.Evaluate( distance );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioAttenuation::EvaluateCutoff( Float32 distance, Float32 nearCutoff, Float32 farCutoff ) const GUCEF_AUDIO_REALTIME_CONTEXT
{
    return m_evaluator.EvaluateLogLerpCutoff( distance, nearCutoff, farCutoff );
}

/*-------------------------------------------------------------------------*/

const CAudioAttenuation::TEvaluator&
CAudioAttenuation::GetEvaluator( void ) const
{GUCEF_TRACE;

    return m_evaluator;
}

/*-------------------------------------------------------------------------*/

const char*
CAudioAttenuation::ModelToString( TDistanceModel model )
{GUCEF_TRACE;

    for ( UInt32 i = 0; i < ModelNameCount; ++i )
    {
        if ( ModelNames[ i ].model == model )
            return ModelNames[ i ].name;
    }
    return "none";
}

/*-------------------------------------------------------------------------*/

bool
CAudioAttenuation::StringToModel( const CString& text, TDistanceModel& model )
{GUCEF_TRACE;

    const CString name = text.Lowercase();
    for ( UInt32 i = 0; i < ModelNameCount; ++i )
    {
        if ( name == CString( ModelNames[ i ].name ).Lowercase() )
        {
            model = ModelNames[ i ].model;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAudioAttenuation::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "model", CString( ModelToString( m_evaluator.GetModel() ) ) );
    section->SetAttribute( "minDistance", m_evaluator.GetMinDistance() );
    section->SetAttribute( "maxDistance", m_evaluator.GetMaxDistance() );
    section->SetAttribute( "rolloff", m_evaluator.GetRolloff() );

    // the curve is only written when it has points
    while ( section->DelChild( "Curve" ) )
        ;
    if ( m_evaluator.GetCurve().GetPointCount() > 0 )
    {
        CORE::CDataNode* curve = section->AddChild( "Curve" );
        if ( GUCEF_NULL == curve )
            return false;
        for ( UInt32 i = 0; i < m_evaluator.GetCurve().GetPointCount(); ++i )
        {
            CORE::CDataNode* point = curve->AddChild( "Point" );
            if ( GUCEF_NULL == point )
                return false;
            point->SetAttribute( "distance", m_evaluator.GetCurve().GetPointX( i ) );
            point->SetAttribute( "gainDb", m_evaluator.GetCurve().GetPointY( i ) );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioAttenuation::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    const CString modelName = section->GetAttributeValueOrChildValueByName( "model" ).AsString( CString::Empty, true );
    if ( !modelName.IsNULLOrEmpty() )
    {
        TDistanceModel model = MATH::DISTANCEMODEL_NONE;
        if ( StringToModel( modelName, model ) )
            SetModel( model );
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioAttenuation: Ignoring the unknown distance model \"" << modelName << "\"" );
        }
    }
    SetMinDistance( section->GetAttributeValueOrChildValueByName( "minDistance" ).AsFloat32( GetMinDistance(), true ) );
    SetMaxDistance( section->GetAttributeValueOrChildValueByName( "maxDistance" ).AsFloat32( GetMaxDistance(), true ) );
    SetRolloff( section->GetAttributeValueOrChildValueByName( "rolloff" ).AsFloat32( GetRolloff(), true ) );

    const CORE::CDataNode* curve = section->FindChild( "Curve" );
    if ( GUCEF_NULL != curve )
    {
        ClearCurve();
        for ( CORE::CDataNode::const_iterator i = curve->ConstBegin(); i != curve->ConstEnd(); ++i )
        {
            const CORE::CDataNode* point = (*i);
            if ( point->GetName() != "Point" )
                continue;

            const CORE::CVariant distance = point->GetAttributeValueOrChildValueByName( "distance" );
            const CORE::CVariant gain = point->GetAttributeValueOrChildValueByName( "gainDb" );
            if ( !distance.IsInitialized() || !gain.IsInitialized() ||
                 !AddCurvePoint( distance.AsFloat32( 0.0f, true ), gain.AsFloat32( 0.0f, true ) ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioAttenuation: Ignoring a curve point that is not valid or does not fit" );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioAttenuation::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CAudioAttenuation";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
