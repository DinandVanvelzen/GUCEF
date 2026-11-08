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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H
#include "gucefAUDIO_CAudioEffectFactory.h"
#define GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H ? */

#include "gucefAUDIO_CAudioEffect.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CAudioEffect::ClassTypeName = "GUCEF::AUDIO::CAudioEffect";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const TypeAttributeName = "type";
const char* const OptionsNodeName = "Options";
const char* const ParametersNodeName = "Parameters";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioEffect::CAudioEffect( const CAudioEffectParameterInfo* parameterInfos, UInt32 parameterCount )
    : CORE::CIConfigurable()
    , m_parameterInfos( parameterInfos )
    , m_parameterCount( GUCEF_NULL != parameterInfos ? parameterCount : 0 )
    , m_parameters( GUCEF_NULL != parameterInfos ? parameterCount : 0 )
    , m_format()
    , m_maxBlockFrames( 0 )
    , m_prepared( false )
    , m_processedFrames( 0 )
    , m_factory( GUCEF_NULL )
{GUCEF_TRACE;

    // every parameter starts at its default, all published together
    if ( m_parameterCount > 0 )
    {
        GUCEF::vector< UInt32 > ids( m_parameterCount, 0 );
        GUCEF::vector< Float32 > values( m_parameterCount, 0.0f );
        for ( UInt32 i=0; i<m_parameterCount; ++i )
        {
            ids[ i ] = i;
            values[ i ] = m_parameterInfos[ i ].GetDefaultValue();
        }
        m_parameters.Set( &ids[ 0 ], &values[ 0 ], m_parameterCount );
        m_parameters.SynchronizeSnapshot();
    }
}

/*-------------------------------------------------------------------------*/

CAudioEffect::~CAudioEffect()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_factory )
        m_factory->OnInstanceDestroyed();
    m_factory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioEffect::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEffect::GetParameterCount( void ) const
{GUCEF_TRACE;

    return m_parameterCount;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CAudioEffect::GetParameterInfo( UInt32 parameterId ) const
{GUCEF_TRACE;

    return parameterId < m_parameterCount ? &m_parameterInfos[ parameterId ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffect::TryGetParameterId( const CString& name, UInt32& parameterId ) const
{GUCEF_TRACE;

    const CString wanted = name.Lowercase();
    for ( UInt32 i=0; i<m_parameterCount; ++i )
    {
        if ( m_parameterInfos[ i ].GetName().Lowercase() == wanted )
        {
            parameterId = i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAudioEffect::SetParameter( UInt32 parameterId, Float32 value )
{GUCEF_TRACE;

    if ( parameterId < m_parameterCount )
        m_parameters.Set( parameterId, m_parameterInfos[ parameterId ].Clamp( value ) );
}

/*-------------------------------------------------------------------------*/

void
CAudioEffect::SetParameters( const UInt32* parameterIds, const Float32* values, UInt32 count )
{GUCEF_TRACE;

    if ( GUCEF_NULL == parameterIds || GUCEF_NULL == values || 0 == count )
        return;

    // clamped first, so the block publishes the final values in one go
    Float32 clampedOnStack[ 32 ];
    GUCEF::vector< Float32 > clampedOnHeap;
    Float32* clamped = clampedOnStack;
    if ( count > 32 )
    {
        clampedOnHeap.assign( count, 0.0f );
        clamped = &clampedOnHeap[ 0 ];
    }
    for ( UInt32 i=0; i<count; ++i )
        clamped[ i ] = parameterIds[ i ] < m_parameterCount ? m_parameterInfos[ parameterIds[ i ] ].Clamp( values[ i ] ) : values[ i ];
    m_parameters.Set( parameterIds, clamped, count );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEffect::GetParameter( UInt32 parameterId ) const
{GUCEF_TRACE;

    return m_parameters.Get( parameterId );
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffect::Prepare( const CAudioFormat& format, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_prepared = false;
    if ( !format.IsValid() || 0 == maxBlockFrames )
        return false;

    // the format is always float, whatever the caller describes
    CAudioFormat floatFormat( format );
    floatFormat.SetSampleType( SAMPLETYPE_FLOAT32 );

    m_parameters.SynchronizeSnapshot();
    if ( !OnPrepare( floatFormat, maxBlockFrames ) )
        return false;

    m_format = floatFormat;
    m_maxBlockFrames = maxBlockFrames;
    m_processedFrames = 0;
    OnReset();
    m_prepared = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffect::IsPrepared( void ) const
{GUCEF_TRACE;

    return m_prepared;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffect::IsPreparedFor( const CAudioFormat& format, UInt32 maxBlockFrames ) const
{GUCEF_TRACE;

    if ( !m_prepared || maxBlockFrames > m_maxBlockFrames )
        return false;
    return format.GetSampleRate() == m_format.GetSampleRate() &&
           format.GetChannelCount() == m_format.GetChannelCount() &&
           format.GetChannelLayout() == m_format.GetChannelLayout();
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioEffect::GetFormat( void ) const
{GUCEF_TRACE;

    return m_format;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEffect::GetMaxBlockFrames( void ) const
{GUCEF_TRACE;

    return m_maxBlockFrames;
}

/*-------------------------------------------------------------------------*/

void
CAudioEffect::Reset( void )
{
    if ( !m_prepared )
        return;
    m_processedFrames = 0;
    OnReset();
}

/*-------------------------------------------------------------------------*/

void
CAudioEffect::Process( Float32* inOut, UInt32 frames )
{
    if ( !m_prepared || frames > m_maxBlockFrames || 0 == frames || GUCEF_NULL == inOut )
        return;
    OnProcess( inOut, frames );
    m_processedFrames += frames;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffect::HaveParametersChanged( void )
{
    return m_parameters.Refresh();
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEffect::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEffect::GetTailInFrames( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

CAudioEffectFactory*
CAudioEffect::GetFactory( void ) const
{GUCEF_TRACE;

    return m_factory;
}

/*-------------------------------------------------------------------------*/

void
CAudioEffect::GetConstructionOptions( CORE::CValueList& ) const
{GUCEF_TRACE;

    // most effects are fully described by their parameters
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffect::ResolveReferences( CAudioEngine& )
{GUCEF_TRACE;

    // most effects refer to nothing outside themselves
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffect::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( TypeAttributeName, GetEffectTypeName() );

    config.DelChild( OptionsNodeName );
    CORE::CValueList options;
    GetConstructionOptions( options );
    if ( options.GetKeyCount() > 0 )
    {
        CORE::CDataNode* optionsNode = config.FindOrAddChild( OptionsNodeName );
        if ( GUCEF_NULL == optionsNode )
            return false;
        for ( UInt32 i=0; i<options.GetKeyCount(); ++i )
        {
            const CString key = options.GetKey( i ).AsString();
            optionsNode->SetAttribute( key, options.GetValueAlways( key, "" ) );
        }
    }

    CORE::CDataNode* parameters = config.FindOrAddChild( ParametersNodeName );
    if ( GUCEF_NULL == parameters )
        return false;

    for ( UInt32 i=0; i<m_parameterCount; ++i )
    {
        const CAudioEffectParameterInfo& info = m_parameterInfos[ i ];
        const Float32 value = m_parameters.Get( i );
        if ( AUDIOPARAMKIND_ENUMERATION == info.GetKind() )
            parameters->SetAttribute( info.GetName(), info.GetEnumLabel( (UInt32) value ) );
        else
        if ( AUDIOPARAMKIND_BOOLEAN == info.GetKind() )
            parameters->SetAttribute( info.GetName(), value != 0.0f );
        else
        if ( AUDIOPARAMKIND_INTEGER == info.GetKind() )
            parameters->SetAttribute( info.GetName(), (Int32) value );
        else
            parameters->SetAttribute( info.GetName(), value );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffect::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return ApplyConfig( config, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffect::ApplyConfig( const CORE::CDataNode& config, const CAudioEffect* baseline )
{GUCEF_TRACE;

    const CString typeName = config.GetAttributeValueOrChildValueByName( TypeAttributeName ).AsString();
    if ( !typeName.IsNULLOrEmpty() && typeName.Lowercase() != GetEffectTypeName().Lowercase() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEffect: configuration of type \"" << typeName << "\" given to an effect of type \"" << GetEffectTypeName() << "\"" );
        return false;
    }

    const CORE::CDataNode* parameters = config.FindChild( ParametersNodeName );
    if ( GUCEF_NULL == parameters && GUCEF_NULL == baseline )
        return true;                        // nothing to set: everything keeps its value

    // names nobody knows are worth a line in the log: usually a typing error in the configuration
    if ( GUCEF_NULL != parameters )
    {
        for ( CORE::CDataNode::TAttributeMap::const_iterator a=parameters->AttributeBegin(); a!=parameters->AttributeEnd(); ++a )
        {
            UInt32 unused = 0;
            if ( !TryGetParameterId( (*a).first, unused ) )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioEffect: effect \"" << GetEffectTypeName() << "\" has no parameter \"" << (*a).first << "\", it is ignored" );
        }
    }

    GUCEF::vector< UInt32 > ids;
    GUCEF::vector< Float32 > values;
    for ( UInt32 i=0; i<m_parameterCount; ++i )
    {
        const CAudioEffectParameterInfo& info = m_parameterInfos[ i ];
        const CORE::CVariant& value = GUCEF_NULL != parameters ? parameters->GetAttributeValueOrChildValueByName( info.GetName() ) : CORE::CVariant::Empty;

        Float32 number = 0.0f;
        if ( !value.IsInitialized() || value.IsNULL() )
        {
            if ( GUCEF_NULL == baseline || baseline->m_parameterCount != m_parameterCount )
                continue;
            number = baseline->m_parameters.Get( i );
        }
        else
        if ( AUDIOPARAMKIND_ENUMERATION == info.GetKind() && value.IsString() )
        {
            if ( !info.TryGetEnumValue( value.AsString(), number ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioEffect: \"" << value.AsString() << "\" is not a value of parameter \"" << info.GetName() << "\" of effect \"" << GetEffectTypeName() << "\", it is ignored" );
                continue;
            }
        }
        else
        if ( AUDIOPARAMKIND_BOOLEAN == info.GetKind() && ( value.IsBoolean() || value.IsString() ) )
            number = value.AsBool( false ) ? 1.0f : 0.0f;
        else
            number = value.AsFloat32( info.GetDefaultValue() );

        number = info.Clamp( number );
        if ( number == m_parameters.Get( i ) )
            continue;                       // already so: no publication for nothing

        ids.push_back( i );
        values.push_back( number );
    }
    if ( !ids.empty() )
        m_parameters.Set( &ids[ 0 ], &values[ 0 ], (UInt32) ids.size() );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
