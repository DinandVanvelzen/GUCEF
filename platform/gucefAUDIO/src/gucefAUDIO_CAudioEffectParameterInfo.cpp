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

#include <math.h>

#ifndef GUCEF_AUDIO_CAUDIOEFFECTPARAMETERINFO_H
#include "gucefAUDIO_CAudioEffectParameterInfo.h"
#define GUCEF_AUDIO_CAUDIOEFFECTPARAMETERINFO_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTPARAMETERINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioEffectParameterInfo::CAudioEffectParameterInfo( void )
    : m_id( 0 )
    , m_name()
    , m_unit( AUDIOPARAMUNIT_NONE )
    , m_scale( AUDIOPARAMSCALE_LINEAR )
    , m_kind( AUDIOPARAMKIND_CONTINUOUS )
    , m_minimum( 0.0f )
    , m_maximum( 1.0f )
    , m_defaultValue( 0.0f )
    , m_enumLabels()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioEffectParameterInfo::CAudioEffectParameterInfo( UInt32 id                   ,
                                                      const CString& name         ,
                                                      TAudioParameterUnit unit    ,
                                                      TAudioParameterScale scale  ,
                                                      Float32 minimum             ,
                                                      Float32 maximum             ,
                                                      Float32 defaultValue        ,
                                                      TAudioParameterKind kind    )
    : m_id( id )
    , m_name( name )
    , m_unit( unit )
    , m_scale( scale )
    , m_kind( kind )
    , m_minimum( minimum <= maximum ? minimum : maximum )
    , m_maximum( minimum <= maximum ? maximum : minimum )
    , m_defaultValue( 0.0f )
    , m_enumLabels()
{GUCEF_TRACE;

    if ( AUDIOPARAMKIND_BOOLEAN == m_kind )
    {
        m_minimum = 0.0f;
        m_maximum = 1.0f;
    }
    m_defaultValue = Clamp( defaultValue );
}

/*-------------------------------------------------------------------------*/

CAudioEffectParameterInfo::CAudioEffectParameterInfo( UInt32 id                 ,
                                                      const CString& name       ,
                                                      const char* const* labels ,
                                                      UInt32 labelCount         ,
                                                      UInt32 defaultIndex       )
    : m_id( id )
    , m_name( name )
    , m_unit( AUDIOPARAMUNIT_NONE )
    , m_scale( AUDIOPARAMSCALE_LINEAR )
    , m_kind( AUDIOPARAMKIND_ENUMERATION )
    , m_minimum( 0.0f )
    , m_maximum( 0.0f )
    , m_defaultValue( 0.0f )
    , m_enumLabels()
{GUCEF_TRACE;

    for ( UInt32 i=0; GUCEF_NULL != labels && i<labelCount; ++i )
        m_enumLabels.push_back( CString( labels[ i ] ) );
    m_maximum = m_enumLabels.empty() ? 0.0f : (Float32) ( m_enumLabels.size() - 1 );
    m_defaultValue = Clamp( (Float32) defaultIndex );
}

/*-------------------------------------------------------------------------*/

CAudioEffectParameterInfo::CAudioEffectParameterInfo( const CAudioEffectParameterInfo& src )
    : m_id( src.m_id )
    , m_name( src.m_name )
    , m_unit( src.m_unit )
    , m_scale( src.m_scale )
    , m_kind( src.m_kind )
    , m_minimum( src.m_minimum )
    , m_maximum( src.m_maximum )
    , m_defaultValue( src.m_defaultValue )
    , m_enumLabels( src.m_enumLabels )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioEffectParameterInfo&
CAudioEffectParameterInfo::operator=( const CAudioEffectParameterInfo& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_id = src.m_id;
        m_name = src.m_name;
        m_unit = src.m_unit;
        m_scale = src.m_scale;
        m_kind = src.m_kind;
        m_minimum = src.m_minimum;
        m_maximum = src.m_maximum;
        m_defaultValue = src.m_defaultValue;
        m_enumLabels = src.m_enumLabels;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAudioEffectParameterInfo::~CAudioEffectParameterInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEffectParameterInfo::GetId( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioEffectParameterInfo::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

TAudioParameterUnit
CAudioEffectParameterInfo::GetUnit( void ) const
{GUCEF_TRACE;

    return m_unit;
}

/*-------------------------------------------------------------------------*/

TAudioParameterScale
CAudioEffectParameterInfo::GetScale( void ) const
{GUCEF_TRACE;

    return m_scale;
}

/*-------------------------------------------------------------------------*/

TAudioParameterKind
CAudioEffectParameterInfo::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEffectParameterInfo::GetMinimum( void ) const
{GUCEF_TRACE;

    return m_minimum;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEffectParameterInfo::GetMaximum( void ) const
{GUCEF_TRACE;

    return m_maximum;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEffectParameterInfo::GetDefaultValue( void ) const
{GUCEF_TRACE;

    return m_defaultValue;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioEffectParameterInfo::GetEnumLabelCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_enumLabels.size();
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioEffectParameterInfo::GetEnumLabel( UInt32 index ) const
{GUCEF_TRACE;

    if ( index < m_enumLabels.size() )
        return m_enumLabels[ index ];
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEffectParameterInfo::TryGetEnumValue( const CString& label, Float32& value ) const
{GUCEF_TRACE;

    const CString wanted = label.Lowercase();
    for ( size_t i=0; i<m_enumLabels.size(); ++i )
    {
        if ( m_enumLabels[ i ].Lowercase() == wanted )
        {
            value = (Float32) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioEffectParameterInfo::Clamp( Float32 value ) const
{GUCEF_TRACE;

    if ( value != value )
        return m_defaultValue;
    if ( value < m_minimum )
        value = m_minimum;
    if ( value > m_maximum )
        value = m_maximum;
    if ( AUDIOPARAMKIND_CONTINUOUS != m_kind )
        value = (Float32) floor( (Float64) value + 0.5 );
    return value;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
