/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMULTIPLAY_CLobbySettingsSchema.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* TypeNames[] = { "bool", "int", "float", "enum", "string" };
const char* PolicyNames[] = { "boss", "vote", "any", "owner" };

/** Reads a number, false when there is anything else in the text */
bool
ParseNumber( const CORE::CString& text, bool integer, Float64& number )
{GUCEF_TRACE;

    const char* str = text.C_String();
    if ( text.IsNULLOrEmpty() )
        return false;
    char* end = GUCEF_NULL;
    if ( integer )
        number = (Float64) strtol( str, &end, 10 );
    else
        number = strtod( str, &end );
    return GUCEF_NULL != end && '\0' == *end && end != str;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLobbySettingDef::CLobbySettingDef( void )
    : key()
    , label()
    , type( TYPE_STRING )
    , defaultValue()
    , minValue( 0 )
    , maxValue( 0 )
    , step( 0 )
    , hasRange( false )
    , options()
    , changePolicy( CHANGEPOLICY_BOSS )
    , uiHint()
    , group()
    , unit()
    , enabledByKey()
    , enabledByValue()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const char*
CLobbySettingDef::TypeToString( TType type )
{GUCEF_TRACE;

    return TypeNames[ type ];
}

/*-------------------------------------------------------------------------*/

const char*
CLobbySettingDef::ChangePolicyToString( TChangePolicy policy )
{GUCEF_TRACE;

    return PolicyNames[ policy ];
}

CLobbySettingDef::TType
CLobbySettingDef::StringToType( const CORE::CString& type )
{GUCEF_TRACE;

    CORE::CString lower = type.Lowercase();
    for ( int i=0; i<5; ++i )
        if ( lower == TypeNames[ i ] )
            return (TType) i;
    return TYPE_STRING;
}

CLobbySettingDef::TChangePolicy
CLobbySettingDef::StringToChangePolicy( const CORE::CString& policy )
{GUCEF_TRACE;

    CORE::CString lower = policy.Lowercase();
    for ( int i=0; i<4; ++i )
        if ( lower == PolicyNames[ i ] )
            return (TChangePolicy) i;
    return CHANGEPOLICY_BOSS;
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingDef::IsValidValue( const CORE::CString& value, CORE::CString& reason ) const
{GUCEF_TRACE;

    switch ( type )
    {
        case TYPE_BOOL:
        {
            CORE::CString lower = value.Lowercase();
            if ( "true" == lower || "false" == lower || "1" == lower || "0" == lower )
                return true;
            reason = "\"" + value + "\" is not a yes or no for " + key;
            return false;
        }
        case TYPE_INT:
        case TYPE_FLOAT:
        {
            Float64 number = 0;
            if ( !ParseNumber( value, TYPE_INT == type, number ) )
            {
                reason = "\"" + value + "\" is not a number for " + key;
                return false;
            }
            if ( hasRange && ( number < minValue || number > maxValue ) )
            {
                reason = value + " is outside the range of " + key;
                return false;
            }
            if ( step > 0 )
            {
                Float64 steps = ( number - minValue ) / step;
                if ( fabs( steps - floor( steps + 0.5 ) ) > 1e-6 )
                {
                    reason = value + " is not a step of " + key;
                    return false;
                }
            }
            return true;
        }
        case TYPE_ENUM:
        {
            for ( size_t i=0; i<options.size(); ++i )
                if ( options[ i ] == value )
                    return true;
            reason = "\"" + value + "\" is not one of the options of " + key;
            return false;
        }
        default:
            return true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingDef::IsSameValue( const CORE::CString& a, const CORE::CString& b ) const
{GUCEF_TRACE;

    switch ( type )
    {
        case TYPE_BOOL:
            return CORE::StringToBool( a, false ) == CORE::StringToBool( b, false );
        case TYPE_INT:
        case TYPE_FLOAT:
            return fabs( CORE::StringToDouble( a, 0.0 ) - CORE::StringToDouble( b, 0.0 ) ) < 1e-9;
        default:
            return a == b;
    }
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingDef::HasEnabledBy( void ) const
{GUCEF_TRACE;

    return !enabledByKey.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingDef::IsEnabledByValue( const CLobbySettingDef& controller, const CORE::CString& controllerValue ) const
{GUCEF_TRACE;

    if ( !HasEnabledBy() )
        return true;
    return controller.IsSameValue( controllerValue, enabledByValue );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingDef::Save( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "key", key );
    node.SetAttribute( "label", label );
    node.SetAttribute( "type", CORE::CString( TypeToString( type ) ) );
    node.SetAttribute( "default", defaultValue );
    if ( hasRange )
    {
        node.SetAttribute( "min", CORE::DoubleToString( minValue ) );
        node.SetAttribute( "max", CORE::DoubleToString( maxValue ) );
    }
    if ( step > 0 )
        node.SetAttribute( "step", CORE::DoubleToString( step ) );
    node.SetAttribute( "changePolicy", CORE::CString( ChangePolicyToString( changePolicy ) ) );
    node.SetAttribute( "uiHint", uiHint );
    node.SetAttribute( "group", group );
    node.SetAttribute( "unit", unit );
    node.SetAttribute( "enabledByKey", enabledByKey );
    node.SetAttribute( "enabledByValue", enabledByValue );
    node.SetValuesOfChildByName( "options", options );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingDef::Load( const CORE::CDataNode& node )
{GUCEF_TRACE;

    key = node.GetAttributeValueOrChildValueByName( "key" ).AsString( CORE::CString::Empty, true );
    label = node.GetAttributeValueOrChildValueByName( "label" ).AsString( CORE::CString::Empty, true );
    type = StringToType( node.GetAttributeValueOrChildValueByName( "type" ).AsString( CORE::CString::Empty, true ) );
    defaultValue = node.GetAttributeValueOrChildValueByName( "default" ).AsString( CORE::CString::Empty, true );
    CORE::CString minStr = node.GetAttributeValueOrChildValueByName( "min" ).AsString( CORE::CString::Empty, true );
    CORE::CString maxStr = node.GetAttributeValueOrChildValueByName( "max" ).AsString( CORE::CString::Empty, true );
    hasRange = !minStr.IsNULLOrEmpty() && !maxStr.IsNULLOrEmpty();
    minValue = CORE::StringToDouble( minStr, 0 );
    maxValue = CORE::StringToDouble( maxStr, 0 );
    step = CORE::StringToDouble( node.GetAttributeValueOrChildValueByName( "step" ).AsString( CORE::CString::Empty, true ), 0 );
    changePolicy = StringToChangePolicy( node.GetAttributeValueOrChildValueByName( "changePolicy" ).AsString( CORE::CString::Empty, true ) );
    uiHint = node.GetAttributeValueOrChildValueByName( "uiHint" ).AsString( CORE::CString::Empty, true );
    group = node.GetAttributeValueOrChildValueByName( "group" ).AsString( CORE::CString::Empty, true );
    unit = node.GetAttributeValueOrChildValueByName( "unit" ).AsString( CORE::CString::Empty, true );
    enabledByKey = node.GetAttributeValueOrChildValueByName( "enabledByKey" ).AsString( CORE::CString::Empty, true );
    enabledByValue = node.GetAttributeValueOrChildValueByName( "enabledByValue" ).AsString( CORE::CString::Empty, true );
    node.GetValuesOfChildByName( "options", options );
}

/*-------------------------------------------------------------------------*/

CLobbySettingsSchema::CLobbySettingsSchema( void )
    : CORE::CIConfigurable()
    , settings()
    , factions()
    , colours()
    , uniqueColours( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbySettingsSchema::CLobbySettingsSchema( const CLobbySettingsSchema& src )
    : CORE::CIConfigurable( src )
    , settings( src.settings )
    , factions( src.factions )
    , colours( src.colours )
    , uniqueColours( src.uniqueColours )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbySettingsSchema::~CLobbySettingsSchema()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbySettingsSchema&
CLobbySettingsSchema::operator=( const CLobbySettingsSchema& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        settings = src.settings;
        factions = src.factions;
        colours = src.colours;
        uniqueColours = src.uniqueColours;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CLobbySettingDef*
CLobbySettingsSchema::FindSetting( const CORE::CString& key ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<settings.size(); ++i )
        if ( settings[ i ].key == key )
            return &settings[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsSchema::AddSetting( const CLobbySettingDef& def )
{GUCEF_TRACE;

    for ( size_t i=0; i<settings.size(); ++i )
    {
        if ( settings[ i ].key == def.key )
        {
            settings[ i ] = def;
            return;
        }
    }
    settings.push_back( def );
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsSchema::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    CORE::CDataNode* settingsNode = config.AddChild( "settings", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<settings.size(); ++i )
        settings[ i ].Save( *settingsNode->AddChild( "setting", GUCEF_DATATYPE_OBJECT ) );
    config.SetValuesOfChildByName( "factions", factions );
    config.SetValuesOfChildByName( "colours", colours );
    config.SetAttribute( "uniqueColours", uniqueColours );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsSchema::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    settings.clear();
    const CORE::CDataNode* settingsNode = config.FindChild( "settings" );
    if ( GUCEF_NULL != settingsNode )
    {
        CORE::CDataNode::const_iterator i = settingsNode->ConstBegin();
        while ( i != settingsNode->ConstEnd() )
        {
            CLobbySettingDef def;
            def.Load( *(*i) );
            if ( !def.key.IsNULLOrEmpty() )
                settings.push_back( def );
            ++i;
        }
    }
    config.GetValuesOfChildByName( "factions", factions );
    config.GetValuesOfChildByName( "colours", colours );
    uniqueColours = config.GetAttributeValueOrChildValueByName( "uniqueColours" ).AsBool( false, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbySettingsSchema::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CLobbySettingsSchema";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
