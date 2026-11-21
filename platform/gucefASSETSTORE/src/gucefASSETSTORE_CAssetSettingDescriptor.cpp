/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#include "gucefASSETSTORE_CAssetSettingDescriptor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetSettingDescriptor::TypeBool = "bool";
const CORE::CString CAssetSettingDescriptor::TypeUInt = "uint";
const CORE::CString CAssetSettingDescriptor::TypeInt = "int";
const CORE::CString CAssetSettingDescriptor::TypeFloat = "float";
const CORE::CString CAssetSettingDescriptor::TypeString = "string";
const CORE::CString CAssetSettingDescriptor::TypeEnum = "enum";
const CORE::CString CAssetSettingDescriptor::ApplyLive = "live";
const CORE::CString CAssetSettingDescriptor::ApplyRestart = "restart";
const CORE::CString CAssetSettingDescriptor::SensitivityPublic = "public";
const CORE::CString CAssetSettingDescriptor::SensitivityOwnerOnlyRead = "owner-only-read";

/*-------------------------------------------------------------------------*/

namespace {

/** The text as a whole number from 0: digits only, without overflow */
bool
ParseUInt64( const CORE::CString& text, UInt64& value )
{GUCEF_TRACE;

    const UInt32 length = text.Length();
    if ( 0 == length || length > 20 )
        return false;
    const char* chars = text.C_String();
    UInt64 number = 0;
    for ( UInt32 c=0; c<length; ++c )
    {
        if ( chars[ c ] < '0' || chars[ c ] > '9' )
            return false;
        const UInt64 digit = (UInt64) ( chars[ c ] - '0' );
        if ( number > ( GUCEF_UINT64MAX - digit ) / 10 )
            return false;
        number = number * 10 + digit;
    }
    value = number;
    return true;
}

/** The text as a whole number with an optional sign, without overflow */
bool
ParseInt64( const CORE::CString& text, Int64& value )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return false;
    const bool negative = '-' == text[ 0 ];
    const CORE::CString digits = ( negative || '+' == text[ 0 ] ) ? text.CutChars( 1, true ) : text;
    UInt64 magnitude = 0;
    if ( !ParseUInt64( digits, magnitude ) )
        return false;
    if ( negative )
    {
        if ( magnitude > (UInt64) GUCEF_INT64MAX + 1 )
            return false;
        value = magnitude == (UInt64) GUCEF_INT64MAX + 1 ? GUCEF_INT64MIN : -(Int64) magnitude;
        return true;
    }
    if ( magnitude > (UInt64) GUCEF_INT64MAX )
        return false;
    value = (Int64) magnitude;
    return true;
}

/** The text as a number: an optional sign, digits with at most one '.', an optional exponent; nothing else */
bool
ParseFloat64( const CORE::CString& text, Float64& value )
{GUCEF_TRACE;

    const UInt32 length = text.Length();
    if ( 0 == length || length > 64 )
        return false;
    const char* chars = text.C_String();
    UInt32 c = 0;
    if ( '-' == chars[ c ] || '+' == chars[ c ] )
        ++c;
    UInt32 digits = 0;
    bool point = false;
    for ( ; c<length; ++c )
    {
        if ( chars[ c ] >= '0' && chars[ c ] <= '9' )
            ++digits;
        else
        if ( '.' == chars[ c ] && !point )
            point = true;
        else
            break;
    }
    if ( 0 == digits )
        return false;
    if ( c < length && ( 'e' == chars[ c ] || 'E' == chars[ c ] ) )
    {
        ++c;
        if ( c < length && ( '-' == chars[ c ] || '+' == chars[ c ] ) )
            ++c;
        UInt32 exponentDigits = 0;
        for ( ; c<length && chars[ c ] >= '0' && chars[ c ] <= '9'; ++c )
            ++exponentDigits;
        if ( 0 == exponentDigits || exponentDigits > 3 )
            return false;
    }
    if ( c != length )
        return false;
    value = CORE::StringToDouble( text, 0.0 );
    // a value beyond the range of a double is no number: compare with itself to leave out NaN, and the largest finite value
    return value == value && value <= 1.7976931348623157e308 && value >= -1.7976931348623157e308;
}

}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor::CAssetSettingDescriptor( void )
    : CORE::CIDataNodeSerializable()
    , key()
    , group()
    , type( TypeString )
    , minimum()
    , maximum()
    , allowedValues()
    , maxLength( 0 )
    , defaultValue()
    , description()
    , unit()
    , applyMode( ApplyLive )
    , sensitivity( SensitivityPublic )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor::~CAssetSettingDescriptor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor
CAssetSettingDescriptor::MakeBool( const CORE::CString& key, const CORE::CString& group, bool defaultValue, const CORE::CString& description )
{GUCEF_TRACE;

    CAssetSettingDescriptor descriptor;
    descriptor.key = key;
    descriptor.group = group;
    descriptor.type = TypeBool;
    descriptor.defaultValue = defaultValue ? "true" : "false";
    descriptor.description = description;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor
CAssetSettingDescriptor::MakeUInt( const CORE::CString& key, const CORE::CString& group, UInt64 lowest, UInt64 highest, UInt64 defaultValue,
                               const CORE::CString& unit, const CORE::CString& description )
{GUCEF_TRACE;

    CAssetSettingDescriptor descriptor;
    descriptor.key = key;
    descriptor.group = group;
    descriptor.type = TypeUInt;
    descriptor.minimum = CORE::ToString( lowest );
    descriptor.maximum = CORE::ToString( highest );
    descriptor.defaultValue = CORE::ToString( defaultValue );
    descriptor.unit = unit;
    descriptor.description = description;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor
CAssetSettingDescriptor::MakeInt( const CORE::CString& key, const CORE::CString& group, Int64 lowest, Int64 highest, Int64 defaultValue,
                              const CORE::CString& unit, const CORE::CString& description )
{GUCEF_TRACE;

    CAssetSettingDescriptor descriptor;
    descriptor.key = key;
    descriptor.group = group;
    descriptor.type = TypeInt;
    descriptor.minimum = CORE::ToString( lowest );
    descriptor.maximum = CORE::ToString( highest );
    descriptor.defaultValue = CORE::ToString( defaultValue );
    descriptor.unit = unit;
    descriptor.description = description;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor
CAssetSettingDescriptor::MakeFloat( const CORE::CString& key, const CORE::CString& group, Float64 lowest, Float64 highest, Float64 defaultValue,
                                const CORE::CString& unit, const CORE::CString& description )
{GUCEF_TRACE;

    CAssetSettingDescriptor descriptor;
    descriptor.key = key;
    descriptor.group = group;
    descriptor.type = TypeFloat;
    descriptor.minimum = CORE::ToString( lowest );
    descriptor.maximum = CORE::ToString( highest );
    descriptor.defaultValue = CORE::ToString( defaultValue );
    descriptor.unit = unit;
    descriptor.description = description;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor
CAssetSettingDescriptor::MakeString( const CORE::CString& key, const CORE::CString& group, UInt32 maxLength, const CORE::CString& defaultValue,
                                 const CORE::CString& description )
{GUCEF_TRACE;

    CAssetSettingDescriptor descriptor;
    descriptor.key = key;
    descriptor.group = group;
    descriptor.type = TypeString;
    descriptor.maxLength = maxLength;
    descriptor.defaultValue = defaultValue;
    descriptor.description = description;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor
CAssetSettingDescriptor::MakeEnum( const CORE::CString& key, const CORE::CString& group, const CORE::CString::StringVector& allowedValues,
                               const CORE::CString& defaultValue, const CORE::CString& description )
{GUCEF_TRACE;

    CAssetSettingDescriptor descriptor;
    descriptor.key = key;
    descriptor.group = group;
    descriptor.type = TypeEnum;
    descriptor.allowedValues = allowedValues;
    descriptor.defaultValue = defaultValue;
    descriptor.description = description;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor
CAssetSettingDescriptor::AppliedAtRestart( void ) const
{GUCEF_TRACE;

    CAssetSettingDescriptor descriptor( *this );
    descriptor.applyMode = ApplyRestart;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CAssetSettingDescriptor
CAssetSettingDescriptor::OwnerOnlyRead( void ) const
{GUCEF_TRACE;

    CAssetSettingDescriptor descriptor( *this );
    descriptor.sensitivity = SensitivityOwnerOnlyRead;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingDescriptor::IsPublic( void ) const
{GUCEF_TRACE;

    return SensitivityPublic == sensitivity;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingDescriptor::AppliesLive( void ) const
{GUCEF_TRACE;

    return ApplyLive == applyMode;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingDescriptor::IsValidKey( const CORE::CString& key )
{GUCEF_TRACE;

    const UInt32 length = key.Length();
    if ( 0 == length || length > 128 )
        return false;
    const char* chars = key.C_String();
    char previous = '.';
    for ( UInt32 c=0; c<length; ++c )
    {
        const char ch = chars[ c ];
        const bool allowed = ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) || ( ch >= '0' && ch <= '9' ) || '.' == ch || '_' == ch || '-' == ch;
        if ( !allowed || ( '.' == ch && '.' == previous ) )
            return false;
        previous = ch;
    }
    return '.' != previous;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingDescriptor::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !IsValidKey( key ) )
    {
        reason = "\"" + key + "\" is not a key: letters, digits, '.', '_' and '-' with no empty part";
        return false;
    }
    if ( TypeBool != type && TypeUInt != type && TypeInt != type && TypeFloat != type && TypeString != type && TypeEnum != type )
    {
        reason = key + ": the type \"" + type + "\" is not known";
        return false;
    }
    if ( ApplyLive != applyMode && ApplyRestart != applyMode )
    {
        reason = key + ": the apply mode \"" + applyMode + "\" is not known";
        return false;
    }
    if ( SensitivityPublic != sensitivity && SensitivityOwnerOnlyRead != sensitivity )
    {
        reason = key + ": the sensitivity \"" + sensitivity + "\" is not known";
        return false;
    }
    if ( TypeEnum == type && allowedValues.empty() )
    {
        reason = key + ": an enum needs its allowed values";
        return false;
    }
    CORE::CString normalized;
    CORE::CString problem;
    if ( !Normalize( defaultValue, normalized, problem ) )
    {
        reason = key + ": the default is not a value of the setting: " + problem;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingDescriptor::Normalize( const CORE::CString& text, CORE::CString& normalized, CORE::CString& problem ) const
{GUCEF_TRACE;

    normalized.Clear();
    problem.Clear();
    const CORE::CString value = text.Trim( true ).Trim( false );
    if ( TypeBool == type )
    {
        const CORE::CString lower = value.Lowercase();
        if ( "true" == lower || "1" == lower )
            normalized = "true";
        else
        if ( "false" == lower || "0" == lower )
            normalized = "false";
        else
        {
            problem = "\"" + value + "\" is not true or false";
            return false;
        }
        return true;
    }
    if ( TypeUInt == type )
    {
        UInt64 number = 0;
        UInt64 lowest = 0;
        UInt64 highest = GUCEF_UINT64MAX;
        if ( !ParseUInt64( value, number ) ||
             ( !minimum.IsNULLOrEmpty() && ParseUInt64( minimum, lowest ) && number < lowest ) ||
             ( !maximum.IsNULLOrEmpty() && ParseUInt64( maximum, highest ) && number > highest ) )
        {
            problem = "\"" + value + "\" is not a whole number from " + GetRangeText();
            return false;
        }
        normalized = CORE::ToString( number );
        return true;
    }
    if ( TypeInt == type )
    {
        Int64 number = 0;
        Int64 lowest = GUCEF_INT64MIN;
        Int64 highest = GUCEF_INT64MAX;
        if ( !ParseInt64( value, number ) ||
             ( !minimum.IsNULLOrEmpty() && ParseInt64( minimum, lowest ) && number < lowest ) ||
             ( !maximum.IsNULLOrEmpty() && ParseInt64( maximum, highest ) && number > highest ) )
        {
            problem = "\"" + value + "\" is not a whole number from " + GetRangeText();
            return false;
        }
        normalized = CORE::ToString( number );
        return true;
    }
    if ( TypeFloat == type )
    {
        Float64 number = 0;
        Float64 lowest = 0;
        Float64 highest = 0;
        if ( !ParseFloat64( value, number ) ||
             ( !minimum.IsNULLOrEmpty() && ParseFloat64( minimum, lowest ) && number < lowest ) ||
             ( !maximum.IsNULLOrEmpty() && ParseFloat64( maximum, highest ) && number > highest ) )
        {
            problem = "\"" + value + "\" is not a number from " + GetRangeText();
            return false;
        }
        normalized = CORE::ToString( number );
        return true;
    }
    if ( TypeEnum == type )
    {
        for ( size_t i=0; i<allowedValues.size(); ++i )
        {
            if ( allowedValues[ i ].Equals( value, false ) )
            {
                normalized = allowedValues[ i ];
                return true;
            }
        }
        problem = "\"" + value + "\" is not " + GetRangeText();
        return false;
    }
    // a text: as given, without control characters and within its length
    const UInt32 length = text.Length();
    if ( maxLength > 0 && length > maxLength )
    {
        problem = "the text has " + CORE::ToString( length ) + " characters, at most " + CORE::ToString( maxLength ) + " are allowed";
        return false;
    }
    const char* chars = text.C_String();
    for ( UInt32 c=0; c<length; ++c )
    {
        if ( (unsigned char) chars[ c ] < 32 || 127 == (unsigned char) chars[ c ] )
        {
            problem = "the text has a control character";
            return false;
        }
    }
    normalized = text;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSettingDescriptor::GetRangeText( void ) const
{GUCEF_TRACE;

    if ( TypeBool == type )
        return "true or false";
    if ( TypeEnum == type )
    {
        CORE::CString list;
        for ( size_t i=0; i<allowedValues.size(); ++i )
            list += ( 0 == i ? CORE::CString::Empty : CORE::CString( ", " ) ) + allowedValues[ i ];
        return "one of " + list;
    }
    if ( TypeString == type )
        return maxLength > 0 ? "a text of at most " + CORE::ToString( maxLength ) + " characters" : CORE::CString( "a text" );
    const CORE::CString lowest = minimum.IsNULLOrEmpty() ? CORE::CString( "any" ) : minimum;
    const CORE::CString highest = maximum.IsNULLOrEmpty() ? CORE::CString( "any" ) : maximum;
    return lowest + " to " + highest;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingDescriptor::Serialize( CORE::CDataNode& domRootNode                        ,
                                    const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "key", key );
    success = domRootNode.SetAttribute( "group", group ) && success;
    success = domRootNode.SetAttribute( "type", type ) && success;
    success = domRootNode.SetAttribute( "minimum", minimum ) && success;
    success = domRootNode.SetAttribute( "maximum", maximum ) && success;
    success = domRootNode.SetValuesOfChildByName( "allowedValues", allowedValues ) && success;
    success = domRootNode.SetAttribute( "maxLength", maxLength ) && success;
    success = domRootNode.SetAttribute( "defaultValue", defaultValue ) && success;
    success = domRootNode.SetAttribute( "description", description ) && success;
    success = domRootNode.SetAttribute( "unit", unit ) && success;
    success = domRootNode.SetAttribute( "applyMode", applyMode ) && success;
    success = domRootNode.SetAttribute( "sensitivity", sensitivity ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingDescriptor::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                      const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetSettingDescriptor();
    key = domRootNode.GetAttributeValueOrChildValueByName( "key" ).AsString( key, true );
    group = domRootNode.GetAttributeValueOrChildValueByName( "group" ).AsString( group, true );
    type = domRootNode.GetAttributeValueOrChildValueByName( "type" ).AsString( type, true );
    minimum = domRootNode.GetAttributeValueOrChildValueByName( "minimum" ).AsString( minimum, true );
    maximum = domRootNode.GetAttributeValueOrChildValueByName( "maximum" ).AsString( maximum, true );
    domRootNode.GetValuesOfChildByName( "allowedValues", allowedValues );
    maxLength = domRootNode.GetAttributeValueOrChildValueByName( "maxLength" ).AsUInt32( maxLength, true );
    defaultValue = domRootNode.GetAttributeValueOrChildValueByName( "defaultValue" ).AsString( defaultValue, true );
    description = domRootNode.GetAttributeValueOrChildValueByName( "description" ).AsString( description, true );
    unit = domRootNode.GetAttributeValueOrChildValueByName( "unit" ).AsString( unit, true );
    applyMode = domRootNode.GetAttributeValueOrChildValueByName( "applyMode" ).AsString( applyMode, true );
    sensitivity = domRootNode.GetAttributeValueOrChildValueByName( "sensitivity" ).AsString( sensitivity, true );
    return !key.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSettingDescriptor::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSettingDescriptor( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingDescriptor::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingDescriptor::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetSettingDescriptor";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSettingDescriptor::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSettingDescriptor";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
