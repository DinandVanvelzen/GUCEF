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

#include <stdlib.h>
#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefAUDIO_CAudioBusLayout.h"

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

const CString CAudioBusLayout::ConfigSectionName = "AudioBusLayout";
const CString CAudioBusLayout::MasterBusName = "master";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Float32 DecibelTolerance = 0.001f;
const Float32 RelativeTolerance = 1.0e-6f;

const char* const OptionsNodeName = "Options";
const char* const ParametersNodeName = "Parameters";

bool
SameNumber( Float32 a, Float32 b, Float32 absoluteTolerance, Float32 relativeTolerance )
{
    const Float32 difference = fabsf( a - b );
    if ( difference <= absoluteTolerance )
        return true;
    const Float32 largest = fabsf( a ) > fabsf( b ) ? fabsf( a ) : fabsf( b );
    return difference <= largest * relativeTolerance;
}

/** True when the text is a number and nothing else */
bool
ParseNumber( const CString& text, Float64& value )
{
    if ( text.IsNULLOrEmpty() )
        return false;
    const char* begin = text.C_String();
    char* end = GUCEF_NULL;
    value = strtod( begin, &end );
    return GUCEF_NULL != end && end != begin && '\0' == *end;
}

/** Two values are the same when their text is, or when both are numbers that are the same. "true" and "1" are not the same: the config writes what the parameter is */
bool
SameValue( const CORE::CVariant& a, const CORE::CVariant& b )
{
    const CString textA = a.AsString();
    const CString textB = b.AsString();
    if ( textA == textB )
        return true;
    Float64 numberA = 0.0, numberB = 0.0;
    if ( ParseNumber( textA, numberA ) && ParseNumber( textB, numberB ) )
        return SameNumber( (Float32) numberA, (Float32) numberB, 0.0f, RelativeTolerance );
    return false;
}

/** The attributes of two nodes: the same names with the same values */
bool
SameAttributes( const CORE::CDataNode* a, const CORE::CDataNode* b )
{
    const UInt32 countA = GUCEF_NULL == a ? 0 : a->GetAttCount();
    const UInt32 countB = GUCEF_NULL == b ? 0 : b->GetAttCount();
    if ( countA != countB )
        return false;
    if ( 0 == countA )
        return true;
    for ( CORE::CDataNode::TAttributeMap::const_iterator i=a->AttributeBegin(); i!=a->AttributeEnd(); ++i )
    {
        const CORE::CDataNode::TKeyValuePair* other = b->GetAttribute( (*i).first );
        if ( GUCEF_NULL == other || !SameValue( (*i).second, other->second ) )
            return false;
    }
    return true;
}

bool
SameEffect( const CAudioBusLayout::SEffect& a, const CAudioBusLayout::SEffect& b )
{
    if ( a.slot != b.slot || a.typeName.Lowercase() != b.typeName.Lowercase() || a.enabled != b.enabled )
        return false;
    if ( !SameNumber( a.wet, b.wet, 0.0f, RelativeTolerance ) || !SameNumber( a.dry, b.dry, 0.0f, RelativeTolerance ) )
        return false;
    return SameAttributes( a.config.FindChild( OptionsNodeName ), b.config.FindChild( OptionsNodeName ) ) &&
           SameAttributes( a.config.FindChild( ParametersNodeName ), b.config.FindChild( ParametersNodeName ) );
}

bool
SameSend( const CAudioBusLayout::SSend& a, const CAudioBusLayout::SSend& b )
{
    return a.target == b.target && a.preFader == b.preFader && SameNumber( a.levelInDecibels, b.levelInDecibels, DecibelTolerance, 0.0f );
}

bool
SameBus( const CAudioBusLayout::SBus& a, const CAudioBusLayout::SBus& b )
{
    // a child of the master bus is named by its parent or not named at all
    const CString parentA = a.parent == CAudioBusLayout::MasterBusName ? CString::Empty : a.parent;
    const CString parentB = b.parent == CAudioBusLayout::MasterBusName ? CString::Empty : b.parent;
    if ( a.name != b.name || parentA != parentB || a.muted != b.muted )
        return false;
    if ( !SameNumber( a.volumeInDecibels, b.volumeInDecibels, DecibelTolerance, 0.0f ) )
        return false;
    if ( a.effects.size() != b.effects.size() || a.sends.size() != b.sends.size() )
        return false;

    // effects by slot
    for ( size_t i=0; i<a.effects.size(); ++i )
    {
        bool found = false;
        for ( size_t j=0; !found && j<b.effects.size(); ++j )
            if ( a.effects[ i ].slot == b.effects[ j ].slot )
                found = SameEffect( a.effects[ i ], b.effects[ j ] );
        if ( !found )
            return false;
    }

    // sends as a set: every send of 'a' pairs with a send of 'b' that no other send has paired with
    GUCEF::vector< bool > used( b.sends.size(), false );
    for ( size_t i=0; i<a.sends.size(); ++i )
    {
        bool found = false;
        for ( size_t j=0; !found && j<b.sends.size(); ++j )
        {
            if ( !used[ j ] && SameSend( a.sends[ i ], b.sends[ j ] ) )
            {
                used[ j ] = true;
                found = true;
            }
        }
        if ( !found )
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

CAudioBusLayout::SSend::SSend( void )
    : target()
    , levelInDecibels( 0.0f )
    , preFader( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SSend::SSend( const CString& target_, Float32 levelInDecibels_, bool preFader_ )
    : target( target_ )
    , levelInDecibels( levelInDecibels_ )
    , preFader( preFader_ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SEffect::SEffect( void )
    : slot( 0 )
    , typeName()
    , enabled( true )
    , wet( 1.0f )
    , dry( 0.0f )
    , config()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SEffect::SEffect( UInt32 slot_, const CString& typeName_ )
    : slot( slot_ )
    , typeName( typeName_ )
    , enabled( true )
    , wet( 1.0f )
    , dry( 0.0f )
    , config()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SEffect::SEffect( const SEffect& src )
    : slot( src.slot )
    , typeName( src.typeName )
    , enabled( src.enabled )
    , wet( src.wet )
    , dry( src.dry )
    , config( src.config )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SEffect&
CAudioBusLayout::SEffect::operator=( const SEffect& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        slot = src.slot;
        typeName = src.typeName;
        enabled = src.enabled;
        wet = src.wet;
        dry = src.dry;
        config = src.config;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CAudioBusLayout::SEffect::SetParameter( const CString& name, const CORE::CVariant& value )
{GUCEF_TRACE;

    CORE::CDataNode* parameters = config.FindOrAddChild( ParametersNodeName );
    if ( GUCEF_NULL != parameters )
        parameters->SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

void
CAudioBusLayout::SEffect::SetOption( const CString& name, const CORE::CVariant& value )
{GUCEF_TRACE;

    CORE::CDataNode* options = config.FindOrAddChild( OptionsNodeName );
    if ( GUCEF_NULL != options )
        options->SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SBus::SBus( void )
    : name()
    , parent()
    , volumeInDecibels( 0.0f )
    , muted( false )
    , sends()
    , effects()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SBus::SBus( const CString& name_, const CString& parent_, Float32 volumeInDecibels_, bool muted_ )
    : name( name_ )
    , parent( parent_ )
    , volumeInDecibels( volumeInDecibels_ )
    , muted( muted_ )
    , sends()
    , effects()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SSend&
CAudioBusLayout::SBus::AddSend( const CString& target, Float32 levelInDecibels, bool preFader )
{GUCEF_TRACE;

    sends.push_back( SSend( target, levelInDecibels, preFader ) );
    return sends.back();
}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SEffect&
CAudioBusLayout::SBus::AddEffect( UInt32 slot, const CString& typeName )
{GUCEF_TRACE;

    effects.push_back( SEffect( slot, typeName ) );
    return effects.back();
}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::CAudioBusLayout( void )
    : CORE::CIDataNodeSerializable()
    , m_buses()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::CAudioBusLayout( const CAudioBusLayout& src )
    : CORE::CIDataNodeSerializable( src )
    , m_buses( src.m_buses )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::~CAudioBusLayout()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBusLayout&
CAudioBusLayout::operator=( const CAudioBusLayout& src )
{GUCEF_TRACE;

    if ( this != &src )
        m_buses = src.m_buses;
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CAudioBusLayout::Clear( void )
{GUCEF_TRACE;

    m_buses.clear();
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusLayout::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_buses.empty();
}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SBus&
CAudioBusLayout::AddBus( const CString& name, const CString& parent, Float32 volumeInDecibels, bool muted )
{GUCEF_TRACE;

    m_buses.push_back( SBus( name, parent, volumeInDecibels, muted ) );
    return m_buses.back();
}

/*-------------------------------------------------------------------------*/

const CAudioBusLayout::SBus*
CAudioBusLayout::FindBus( const CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_buses.size(); ++i )
        if ( m_buses[ i ].name == name )
            return &m_buses[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::SBus*
CAudioBusLayout::FindBus( const CString& name )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_buses.size(); ++i )
        if ( m_buses[ i ].name == name )
            return &m_buses[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CAudioBusLayout::TBusVector&
CAudioBusLayout::GetBuses( void ) const
{GUCEF_TRACE;

    return m_buses;
}

/*-------------------------------------------------------------------------*/

CAudioBusLayout::TBusVector&
CAudioBusLayout::GetBuses( void )
{GUCEF_TRACE;

    return m_buses;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusLayout::operator==( const CAudioBusLayout& other ) const
{GUCEF_TRACE;

    if ( m_buses.size() != other.m_buses.size() )
        return false;
    for ( size_t i=0; i<m_buses.size(); ++i )
    {
        const SBus* match = other.FindBus( m_buses[ i ].name );
        if ( GUCEF_NULL == match || !SameBus( m_buses[ i ], *match ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusLayout::operator!=( const CAudioBusLayout& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusLayout::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    // a section that is there already is replaced
    while ( domRootNode.DelChild( "Bus" ) )
        ;

    for ( size_t b=0; b<m_buses.size(); ++b )
    {
        const SBus& bus = m_buses[ b ];
        CORE::CDataNode* busNode = domRootNode.AddChild( "Bus" );
        if ( GUCEF_NULL == busNode )
            return false;

        busNode->SetAttribute( "name", bus.name );
        if ( !bus.parent.IsNULLOrEmpty() )
            busNode->SetAttribute( "parent", bus.parent );
        busNode->SetAttribute( "volumeDb", bus.volumeInDecibels );
        busNode->SetAttribute( "muted", bus.muted );

        for ( size_t s=0; s<bus.sends.size(); ++s )
        {
            CORE::CDataNode* sendNode = busNode->AddChild( "Send" );
            if ( GUCEF_NULL == sendNode )
                return false;
            sendNode->SetAttribute( "target", bus.sends[ s ].target );
            sendNode->SetAttribute( "levelDb", bus.sends[ s ].levelInDecibels );
            sendNode->SetAttribute( "preFader", bus.sends[ s ].preFader );
        }

        for ( size_t e=0; e<bus.effects.size(); ++e )
        {
            const SEffect& effect = bus.effects[ e ];
            CORE::CDataNode* effectNode = busNode->AddChild( "Effect" );
            if ( GUCEF_NULL == effectNode )
                return false;
            effectNode->SetAttribute( "type", effect.typeName );
            effectNode->SetAttribute( "slot", effect.slot );
            effectNode->SetAttribute( "enabled", effect.enabled );
            effectNode->SetAttribute( "wet", effect.wet );
            effectNode->SetAttribute( "dry", effect.dry );

            const char* const names[ 2 ] = { OptionsNodeName, ParametersNodeName };
            for ( int n=0; n<2; ++n )
            {
                const CORE::CDataNode* source = effect.config.FindChild( names[ n ] );
                if ( GUCEF_NULL == source )
                    continue;
                CORE::CDataNode* child = effectNode->AddChild( names[ n ] );
                if ( GUCEF_NULL == child )
                    return false;
                for ( CORE::CDataNode::TAttributeMap::const_iterator a=source->AttributeBegin(); a!=source->AttributeEnd(); ++a )
                    child->SetAttribute( (*a).first, (*a).second );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBusLayout::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    m_buses.clear();

    // the section itself, or the section as a child of what we got (the root of a settings file), or a bare list of buses
    const CORE::CDataNode* section = &domRootNode;
    if ( domRootNode.GetName() != ConfigSectionName )
    {
        const CORE::CDataNode* child = domRootNode.FindChild( ConfigSectionName );
        if ( GUCEF_NULL != child )
            section = child;
    }

    bool success = true;
    bool foundAny = false;
    for ( CORE::CDataNode::const_iterator i=section->ConstBegin(); i!=section->ConstEnd(); ++i )
    {
        const CORE::CDataNode* busNode = (*i);
        if ( busNode->GetName() != "Bus" )
            continue;
        foundAny = true;

        const CString name = busNode->GetAttributeValueOrChildValueByName( "name" ).AsString( CString::Empty, true );
        if ( name.IsNULLOrEmpty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioBusLayout: Ignoring a bus entry without a name" );
            success = false;
            continue;
        }

        SBus& bus = AddBus( name );
        bus.parent = busNode->GetAttributeValueOrChildValueByName( "parent" ).AsString( CString::Empty, true );
        bus.volumeInDecibels = busNode->GetAttributeValueOrChildValueByName( "volumeDb" ).AsFloat32( 0.0f, true );
        bus.muted = busNode->GetAttributeValueOrChildValueByName( "muted" ).AsBool( false, true );

        for ( CORE::CDataNode::const_iterator c=busNode->ConstBegin(); c!=busNode->ConstEnd(); ++c )
        {
            const CORE::CDataNode* node = (*c);
            if ( node->GetName() == "Send" )
            {
                SSend send;
                send.target = node->GetAttributeValueOrChildValueByName( "target" ).AsString( CString::Empty, true );
                if ( send.target.IsNULLOrEmpty() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioBusLayout: Ignoring a send of bus \"" << name << "\" without a target" );
                    success = false;
                    continue;
                }
                send.levelInDecibels = node->GetAttributeValueOrChildValueByName( "levelDb" ).AsFloat32( 0.0f, true );
                send.preFader = node->GetAttributeValueOrChildValueByName( "preFader" ).AsBool( false, true );
                bus.sends.push_back( send );
            }
            else
            if ( node->GetName() == "Effect" )
            {
                SEffect effect;
                effect.typeName = node->GetAttributeValueOrChildValueByName( "type" ).AsString( CString::Empty, true );
                if ( effect.typeName.IsNULLOrEmpty() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioBusLayout: Ignoring an effect of bus \"" << name << "\" without a type" );
                    success = false;
                    continue;
                }
                effect.slot = node->GetAttributeValueOrChildValueByName( "slot" ).AsUInt32( 0, true );
                effect.enabled = node->GetAttributeValueOrChildValueByName( "enabled" ).AsBool( true, true );
                effect.wet = node->GetAttributeValueOrChildValueByName( "wet" ).AsFloat32( 1.0f, true );
                effect.dry = node->GetAttributeValueOrChildValueByName( "dry" ).AsFloat32( 0.0f, true );

                // the configuration of the effect is its type, its options and its parameters
                effect.config.SetAttribute( "type", effect.typeName );
                const char* const names[ 2 ] = { OptionsNodeName, ParametersNodeName };
                for ( int n=0; n<2; ++n )
                {
                    const CORE::CDataNode* source = node->FindChild( names[ n ] );
                    if ( GUCEF_NULL == source )
                        continue;
                    CORE::CDataNode* child = effect.config.AddChild( names[ n ] );
                    if ( GUCEF_NULL == child )
                        continue;
                    for ( CORE::CDataNode::TAttributeMap::const_iterator a=source->AttributeBegin(); a!=source->AttributeEnd(); ++a )
                        child->SetAttribute( (*a).first, (*a).second );
                }
                bus.effects.push_back( effect );
            }
        }
    }
    return success && ( foundAny || section->GetName() == ConfigSectionName );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAudioBusLayout::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAudioBusLayout( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioBusLayout::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CAudioBusLayout";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CAudioBusLayout::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAudioBusLayout::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::AUDIO::CAudioBusLayout";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
