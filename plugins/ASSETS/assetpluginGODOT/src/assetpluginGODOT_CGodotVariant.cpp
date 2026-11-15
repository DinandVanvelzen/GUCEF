/*
 *  assetpluginGODOT: the values of the Godot text formats
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
#include <string.h>

#include "assetpluginGODOT_CGodotVariant.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CGodotVariant::GetType( const CORE::CDataNode* value )
{GUCEF_TRACE;

    if ( GUCEF_NULL == value )
        return CORE::CString::Empty;
    return value->GetAttributeValue( "t" ).AsString();
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::IsType( const CORE::CDataNode* value, const CORE::CString& type )
{GUCEF_TRACE;

    return GUCEF_NULL != value && type == GetType( value );
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::IsScalar( const CORE::CDataNode* value )
{GUCEF_TRACE;

    const CORE::CString type = GetType( value );
    return "int" == type || "float" == type || "bool" == type || "String" == type || "StringName" == type || "NodePath" == type;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotVariant::GetString( const CORE::CDataNode* value, const CORE::CString& fallback )
{GUCEF_TRACE;

    const CORE::CString type = GetType( value );
    if ( "String" == type || "StringName" == type || "NodePath" == type || "ExtResource" == type || "SubResource" == type || "Identifier" == type )
        return value->GetValue().AsString();
    return fallback;
}

/*-------------------------------------------------------------------------*/

Float64
CGodotVariant::GetNumber( const CORE::CDataNode* value, Float64 fallback )
{GUCEF_TRACE;

    const CORE::CString type = GetType( value );
    if ( "float" == type )
        return value->GetValue().AsFloat64();
    if ( "int" == type )
        return (Float64) value->GetValue().AsInt64();
    if ( "bool" == type )
        return value->GetValue().AsBool() ? 1.0 : 0.0;
    return fallback;
}

/*-------------------------------------------------------------------------*/

Int64
CGodotVariant::GetInteger( const CORE::CDataNode* value, Int64 fallback )
{GUCEF_TRACE;

    const CORE::CString type = GetType( value );
    if ( "int" == type )
        return value->GetValue().AsInt64();
    if ( "float" == type )
        return (Int64) value->GetValue().AsFloat64();
    if ( "bool" == type )
        return value->GetValue().AsBool() ? 1 : 0;
    return fallback;
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::GetBool( const CORE::CDataNode* value, bool fallback )
{GUCEF_TRACE;

    const CORE::CString type = GetType( value );
    if ( "bool" == type )
        return value->GetValue().AsBool();
    if ( "int" == type )
        return 0 != value->GetValue().AsInt64();
    if ( "float" == type )
        return 0.0 != value->GetValue().AsFloat64();
    return fallback;
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::GetNumbers( const CORE::CDataNode* value, TNumberVector& numbers )
{GUCEF_TRACE;

    numbers.clear();
    if ( GUCEF_NULL == value )
        return false;
    const CORE::CString type = GetType( value );

    if ( type.StartsWith( "Packed" ) )
    {
        const CORE::CString element = value->GetAttributeValue( "elem" ).AsString();
        const UInt32 count = value->GetAttributeValue( "count" ).AsUInt32();
        const UInt32 components = value->GetAttributeValue( "components" ).AsUInt32();
        if ( element.IsNULLOrEmpty() )
            return false;
        const UInt32 total = count * ( 0 == components ? 1 : components );
        if ( 0 == total )
            return true;
        const void* data = value->GetValue().AsVoidPtr();
        UInt32 elementSize = 4;
        if ( "u8" == element ) elementSize = 1;
        else if ( "i64" == element || "f64" == element ) elementSize = 8;
        if ( GUCEF_NULL == data || value->GetValue().ByteSize( false ) < total * elementSize )
            return false;
        numbers.reserve( total );
        for ( UInt32 i=0; i<total; ++i )
        {
            if ( "u8" == element ) numbers.push_back( static_cast< const UInt8* >( data )[ i ] );
            else if ( "i32" == element ) numbers.push_back( static_cast< const Int32* >( data )[ i ] );
            else if ( "i64" == element ) numbers.push_back( (Float64) static_cast< const Int64* >( data )[ i ] );
            else if ( "f64" == element ) numbers.push_back( static_cast< const Float64* >( data )[ i ] );
            else numbers.push_back( static_cast< const Float32* >( data )[ i ] );
        }
        return true;
    }

    if ( "int" == type || "float" == type || "bool" == type )
    {
        numbers.push_back( GetNumber( value, 0.0 ) );
        return true;
    }

    // call syntax and arrays: the numbers of the children
    if ( "String" == type || "StringName" == type || "NodePath" == type || "ExtResource" == type || "SubResource" == type || "null" == type || "Dictionary" == type || "Object" == type )
        return false;
    for ( const CORE::CDataNode* child = value->GetFirstChild(); GUCEF_NULL != child; child = child->GetNext() )
    {
        TNumberVector nested;
        if ( !GetNumbers( child, nested ) )
        {
            numbers.clear();
            return false;
        }
        numbers.insert( numbers.end(), nested.begin(), nested.end() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::GetBytes( const CORE::CDataNode* value, TByteVector& bytes )
{GUCEF_TRACE;

    bytes.clear();
    if ( !IsType( value, "PackedByteArray" ) )
        return false;
    const UInt32 count = value->GetAttributeValue( "count" ).AsUInt32();
    if ( 0 == count )
        return true;
    const void* data = value->GetValue().AsVoidPtr();
    if ( GUCEF_NULL == data || value->GetValue().ByteSize( false ) < count )
        return false;
    bytes.assign( static_cast< const UInt8* >( data ), static_cast< const UInt8* >( data ) + count );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::GetStrings( const CORE::CDataNode* value, CORE::CString::StringVector& strings )
{GUCEF_TRACE;

    strings.clear();
    const CORE::CString type = GetType( value );
    if ( "PackedStringArray" != type && "Array" != type )
        return false;
    for ( const CORE::CDataNode* child = value->GetFirstChild(); GUCEF_NULL != child; child = child->GetNext() )
        strings.push_back( GetString( child ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::GetReference( const CORE::CDataNode* value, CORE::CString& kind, CORE::CString& id )
{GUCEF_TRACE;

    const CORE::CString type = GetType( value );
    if ( "ExtResource" != type && "SubResource" != type )
        return false;
    kind = type;
    id = value->GetValue().AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotVariant::FindProperty( const CORE::CDataNode& section, const CORE::CString& key )
{GUCEF_TRACE;

    const CORE::CDataNode* properties = section.FindChild( "properties" );
    if ( GUCEF_NULL == properties )
        return GUCEF_NULL;
    return properties->FindChild( key );
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotVariant::FindMember( const CORE::CDataNode* container, const CORE::CString& key )
{GUCEF_TRACE;

    if ( GUCEF_NULL == container )
        return GUCEF_NULL;
    return container->FindChild( key );
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::GetVector3( const CORE::CDataNode* value, Float32 out[ 3 ] )
{GUCEF_TRACE;

    TNumberVector numbers;
    if ( !GetNumbers( value, numbers ) || numbers.size() < 3 )
        return false;
    for ( UInt32 i=0; i<3; ++i )
        out[ i ] = (Float32) numbers[ i ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::GetVector2( const CORE::CDataNode* value, Float32 out[ 2 ] )
{GUCEF_TRACE;

    TNumberVector numbers;
    if ( !GetNumbers( value, numbers ) || numbers.size() < 2 )
        return false;
    out[ 0 ] = (Float32) numbers[ 0 ];
    out[ 1 ] = (Float32) numbers[ 1 ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::GetColour( const CORE::CDataNode* value, Float32 out[ 4 ] )
{GUCEF_TRACE;

    TNumberVector numbers;
    if ( !GetNumbers( value, numbers ) || numbers.size() < 3 )
        return false;
    for ( UInt32 i=0; i<3; ++i )
        out[ i ] = (Float32) numbers[ i ];
    out[ 3 ] = numbers.size() >= 4 ? (Float32) numbers[ 3 ] : 1.0f;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotVariant::GetTransform( const CORE::CDataNode* value, MATH::TFloat32Transform3D& transform )
{GUCEF_TRACE;

    TNumberVector numbers;
    if ( !GetNumbers( value, numbers ) )
        return false;
    const CORE::CString type = GetType( value );
    Float32 rows[ 16 ] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
    if ( ( "Transform3D" == type && numbers.size() >= 12 ) || ( "Basis" == type && numbers.size() >= 9 ) )
    {
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 c=0; c<3; ++c )
                rows[ r * 4 + c ] = (Float32) numbers[ r * 3 + c ];
        if ( "Transform3D" == type )
        {
            rows[ 3 ] = (Float32) numbers[ 9 ];
            rows[ 7 ] = (Float32) numbers[ 10 ];
            rows[ 11 ] = (Float32) numbers[ 11 ];
        }
        const MATH::TFloat32Matrix4D matrix = MATH::TFloat32Matrix4D::FromRowMajor( rows );
        MATH::TFloat32Transform3D result;
        if ( !MATH::TFloat32Transform3D::FromMatrix( matrix, result ) )
            return false;
        transform = result;
        return true;
    }
    if ( "Quaternion" == type && numbers.size() >= 4 )
    {
        MATH::TFloat32Transform3D result;
        result.rotation.x = (Float32) numbers[ 0 ];
        result.rotation.y = (Float32) numbers[ 1 ];
        result.rotation.z = (Float32) numbers[ 2 ];
        result.rotation.w = (Float32) numbers[ 3 ];
        transform = result;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

Float32
CGodotVariant::SrgbToLinear( Float32 value )
{GUCEF_TRACE;

    if ( value <= 0.0f )
        return 0.0f;
    if ( value <= 0.04045f )
        return value / 12.92f;
    return (Float32) pow( ( value + 0.055 ) / 1.055, 2.4 );
}

/*-------------------------------------------------------------------------*/

static void
AddPlainElement( CORE::CDataNode& array, const CORE::CDataNode& value, UInt32 maxElements )
{GUCEF_TRACE;

    if ( CGodotVariant::IsScalar( &value ) )
    {
        array.AddValueAsChild( value.GetValue() );
        return;
    }
    CORE::CDataNode* element = array.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    CGodotVariant::AddPlain( *element, "value", value, maxElements );
}

/*-------------------------------------------------------------------------*/

void
CGodotVariant::AddPlain( CORE::CDataNode& object, const CORE::CString& key, const CORE::CDataNode& value, UInt32 maxElements )
{GUCEF_TRACE;

    const CORE::CString type = GetType( &value );
    if ( IsScalar( &value ) )
    {
        object.SetAttribute( key, value.GetValue() );
        return;
    }
    if ( "ExtResource" == type || "SubResource" == type )
    {
        object.SetAttribute( key, type + "(" + value.GetValue().AsString() + ")" );
        return;
    }
    if ( "null" == type || type.IsNULLOrEmpty() )
        return;
    if ( "Dictionary" == type || "Object" == type )
    {
        CORE::CDataNode* child = object.AddChild( key, GUCEF_DATATYPE_OBJECT );
        if ( "Object" == type )
            child->SetAttribute( "class", value.GetAttributeValue( "class" ).AsString() );
        for ( const CORE::CDataNode* member = value.GetFirstChild(); GUCEF_NULL != member; member = member->GetNext() )
            AddPlain( *child, member->GetName(), *member, maxElements );
        return;
    }
    if ( "PackedStringArray" == type )
    {
        CORE::CDataNode* child = object.AddChild( key, GUCEF_DATATYPE_ARRAY );
        for ( const CORE::CDataNode* member = value.GetFirstChild(); GUCEF_NULL != member; member = member->GetNext() )
            child->AddValueAsChild( member->GetValue() );
        return;
    }

    TNumberVector numbers;
    if ( GetNumbers( &value, numbers ) )
    {
        if ( numbers.size() <= maxElements )
        {
            CORE::CDataNode::TVariantVector values;
            for ( size_t i=0; i<numbers.size(); ++i )
                values.push_back( CORE::CVariant( numbers[ i ] ) );
            object.SetValuesOfChildByName( key, values );
        }
        else
        {
            CORE::CDataNode* child = object.AddChild( key, GUCEF_DATATYPE_OBJECT );
            child->SetAttribute( "type", type );
            child->SetAttribute( "numbers", (UInt32) numbers.size() );
        }
        return;
    }

    // an array of other values
    CORE::CDataNode* child = object.AddChild( key, GUCEF_DATATYPE_ARRAY );
    UInt32 count = 0;
    for ( const CORE::CDataNode* member = value.GetFirstChild(); GUCEF_NULL != member; member = member->GetNext() )
    {
        if ( ++count > maxElements )
            break;
        AddPlainElement( *child, *member, maxElements );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
