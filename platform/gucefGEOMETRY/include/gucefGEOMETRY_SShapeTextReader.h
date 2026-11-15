/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#define GUCEF_GEOMETRY_SSHAPETEXTREADER_H

/*
 *  The text and DataNode helpers shared by the shape classes (sections 9.1 and 9.2 of the design).
 *
 *  Text: every shape writes "Name(field value, field value, ...)" where a value is a point, vector or direction
 *  "(x, y, z)", a number, or a nested shape; SShapeTextReader parses that form. Reading is locale independent, accepts
 *  ASCII white space between tokens and never modifies a shape: the shapes parse into temporaries and assign on success.
 *
 *  DataNode: a shape writes its members as attributes (numbers) and child nodes (points, vectors, nested shapes) named
 *  after the members; SerializeShapeChild / DeserializeShapeChild and SerializeShapeScalar / DeserializeShapeScalar
 *  implement that. Missing members are failures.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_MATH_COMPONENTFUNCTIONS_H
#include "gucefMATH_ComponentFunctions.h"
#define GUCEF_MATH_COMPONENTFUNCTIONS_H
#endif /* GUCEF_MATH_COMPONENTFUNCTIONS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A cursor over the text form of a shape. All Try* members skip leading ASCII white space, return false when the
 *  text does not match and then leave the cursor where it was (TryScalar and TryValue may have skipped white space).
 */
struct SShapeTextReader
{
    const char* data;
    UInt32 length;
    UInt32 position;

    explicit SShapeTextReader( const CORE::CString& text )
        : data( text.C_String() )
        , length( text.Length() )
        , position( 0 )
    {
        if ( GUCEF_NULL == data )
            length = 0;
    }

    void SkipWhitespace( void )
    {
        while ( position < length && MATH::IsComponentTextWhitespace( data[ position ] ) )
            ++position;
    }

    /** True when only white space is left */
    bool AtEnd( void )
    {
        SkipWhitespace();
        return position == length;
    }

    /** Matches the characters of 'literal' exactly (a name such as "Sphere" or "centre") */
    bool TryLiteral( const char* literal )
    {
        UInt32 start = position;
        SkipWhitespace();
        UInt32 i = 0;
        while ( '\0' != literal[ i ] )
        {
            if ( position + i >= length || data[ position + i ] != literal[ i ] )
            {
                position = start;
                return false;
            }
            ++i;
        }
        position += i;
        return true;
    }

    bool TryChar( char c )
    {
        UInt32 start = position;
        SkipWhitespace();
        if ( position < length && c == data[ position ] )
        {
            ++position;
            return true;
        }
        position = start;
        return false;
    }

    /** The next non-white-space character is 'c' (not consumed) */
    bool PeekChar( char c )
    {
        SkipWhitespace();
        return position < length && c == data[ position ];
    }

    /** Reads "( ... )" with nested parentheses balanced, the parentheses included */
    bool TryBalancedGroup( CORE::CString& group )
    {
        UInt32 start = position;
        SkipWhitespace();
        if ( position >= length || '(' != data[ position ] )
        {
            position = start;
            return false;
        }
        UInt32 depth = 0;
        UInt32 first = position;
        while ( position < length )
        {
            if ( '(' == data[ position ] )
                ++depth;
            else if ( ')' == data[ position ] )
            {
                --depth;
                if ( 0 == depth )
                {
                    ++position;
                    group = CORE::CString( data + first, position - first );
                    return true;
                }
            }
            ++position;
        }
        position = start;
        return false;
    }

    /** A point, vector or direction (or any type with FromString) in the form of its ToString */
    template < typename TValue >
    bool TryValue( TValue& value )
    {
        UInt32 start = position;
        CORE::CString group;
        TValue parsed = value;
        if ( TryBalancedGroup( group ) && parsed.FromString( group ) )
        {
            value = parsed;
            return true;
        }
        position = start;
        return false;
    }

    /** A nested shape in the form of its ToString: a name of letters and digits followed by a balanced group */
    template < typename TShape >
    bool TryShape( TShape& shape )
    {
        UInt32 start = position;
        SkipWhitespace();
        UInt32 first = position;
        while ( position < length && ( ( data[ position ] >= 'A' && data[ position ] <= 'Z' ) || ( data[ position ] >= 'a' && data[ position ] <= 'z' ) || ( data[ position ] >= '0' && data[ position ] <= '9' ) ) )
            ++position;
        CORE::CString group;
        TShape parsed = shape;
        if ( position > first && TryBalancedGroup( group ) )
        {
            CORE::CString text( data + first, position - first );
            if ( parsed.FromString( text ) )
            {
                shape = parsed;
                return true;
            }
        }
        position = start;
        return false;
    }

    /** "( p, p, ... )": a possibly empty list of points in the form of their ToString */
    template < typename TPoint >
    bool TryPointList( GUCEF::vector< TPoint >& points )
    {
        UInt32 start = position;
        GUCEF::vector< TPoint > parsed;
        if ( TryChar( '(' ) )
        {
            if ( TryChar( ')' ) )
            {
                points.swap( parsed );
                return true;
            }
            for ( ;; )
            {
                TPoint point;
                if ( !TryValue( point ) )
                    break;
                parsed.push_back( point );
                if ( TryChar( ',' ) )
                    continue;
                if ( TryChar( ')' ) )
                {
                    points.swap( parsed );
                    return true;
                }
                break;
            }
        }
        position = start;
        return false;
    }

    /** A number of kind T up to the next ',' or ')' */
    template < typename T >
    bool TryScalar( T& value )
    {
        UInt32 start = position;
        SkipWhitespace();
        UInt32 first = position;
        while ( position < length && ',' != data[ position ] && ')' != data[ position ] && '(' != data[ position ] )
            ++position;
        UInt32 last = position;
        while ( last > first && MATH::IsComponentTextWhitespace( data[ last - 1 ] ) )
            --last;
        T parsed;
        if ( last > first && MATH::CTNumericTraits< T >::FromString( CORE::CString( data + first, last - first ), parsed ) )
        {
            value = parsed;
            return true;
        }
        position = start;
        return false;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Writes 'value' as the child node 'name' of 'node' */
template < typename TValue >
bool
SerializeShapeChild( CORE::CDataNode& node, const char* name, const TValue& value, const CORE::CDataNodeSerializableSettings& settings )
{
    CORE::CDataNode* child = node.AddChild( CORE::CString( name ), GUCEF_DATATYPE_OBJECT );
    return GUCEF_NULL != child && value.Serialize( *child, settings );
}

/*-------------------------------------------------------------------------*/

/** Reads the child node 'name' of 'node' into 'value'; false (value unchanged) when the child is missing or invalid */
template < typename TValue >
bool
DeserializeShapeChild( const CORE::CDataNode& node, const char* name, TValue& value, const CORE::CDataNodeSerializableSettings& settings )
{
    const CORE::CDataNode* child = node.FindChild( CORE::CString( name ) );
    if ( GUCEF_NULL == child )
        return false;
    TValue parsed = value;
    if ( !parsed.Deserialize( *child, settings ) )
        return false;
    value = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Writes the number 'value' as the attribute 'name' of 'node' */
template < typename T >
bool
SerializeShapeScalar( CORE::CDataNode& node, const char* name, const T& value )
{
    const char* const names[ 1 ] = { name };
    const T components[ 1 ] = { value };
    return MATH::SerializeComponents( node, names, components );
}

/*-------------------------------------------------------------------------*/

/** Reads the attribute 'name' of 'node'; false (value unchanged) when it is missing or invalid */
template < typename T >
bool
DeserializeShapeScalar( const CORE::CDataNode& node, const char* name, T& value )
{
    const char* const names[ 1 ] = { name };
    T components[ 1 ];
    if ( !MATH::DeserializeComponents( node, names, components ) )
        return false;
    value = components[ 0 ];
    return true;
}

/*-------------------------------------------------------------------------*/

/** "name(p, p, ...)" with the ToString of each point */
template < typename TPoint >
CORE::CString
ShapePointsToString( const char* name, const GUCEF::vector< TPoint >& points )
{
    CORE::CString text( name );
    text += '(';
    for ( UInt32 i=0; i<(UInt32) points.size(); ++i )
    {
        if ( i > 0 )
            text += ", ";
        text += points[ i ].ToString();
    }
    text += ')';
    return text;
}

/*-------------------------------------------------------------------------*/

/** Writes the child node 'name' of 'node' with one child node 'point' per point */
template < typename TPoint >
bool
SerializeShapePoints( CORE::CDataNode& node, const char* name, const GUCEF::vector< TPoint >& points, const CORE::CDataNodeSerializableSettings& settings )
{
    CORE::CDataNode* list = node.AddChild( CORE::CString( name ), GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == list )
        return false;
    for ( UInt32 i=0; i<(UInt32) points.size(); ++i )
    {
        CORE::CDataNode* child = list->AddChild( CORE::CString( "point" ), GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL == child || !points[ i ].Serialize( *child, settings ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Reads the child node 'name' of 'node' and the points in its child nodes; false (points unchanged) on any error */
template < typename TPoint >
bool
DeserializeShapePoints( const CORE::CDataNode& node, const char* name, GUCEF::vector< TPoint >& points, const CORE::CDataNodeSerializableSettings& settings )
{
    const CORE::CDataNode* list = node.FindChild( CORE::CString( name ) );
    if ( GUCEF_NULL == list )
        return false;
    GUCEF::vector< TPoint > parsed;
    CORE::CDataNode::const_iterator i = list->ConstBegin();
    while ( i != list->ConstEnd() )
    {
        TPoint point;
        if ( !point.Deserialize( **i, settings ) )
            return false;
        parsed.push_back( point );
        ++i;
    }
    points.swap( parsed );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SSHAPETEXTREADER_H ? */
