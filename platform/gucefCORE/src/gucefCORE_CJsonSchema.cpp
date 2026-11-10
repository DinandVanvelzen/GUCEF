/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CJsonSchema.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CJsonSchema::Dialect2020_12 = "https://json-schema.org/draft/2020-12/schema";

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** State of one compilation */
struct CJsonSchema::SCompileContext
{
    UInt32 maxDepth;
    UInt32 maxSubschemas;
    TStringIndexMap pointers;           /**< escaped JSON pointer of every subschema -> its index */
    TStringVector pointerOfIndex;       /**< the JSON pointer of every subschema, by index */
    CString error;

    SCompileContext( UInt32 depthLimit, UInt32 subschemaLimit )
        : maxDepth( depthLimit )
        , maxSubschemas( subschemaLimit )
        , pointers()
        , pointerOfIndex()
        , error()
    {
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Keywords that are kept and not checked */
static const char* g_annotationKeywords[] =
{
    "title", "description", "default", "examples", "format", "deprecated", "readOnly", "writeOnly",
    "contentEncoding", "contentMediaType", "contentSchema", "$comment"
};
static const UInt32 g_annotationKeywordCount = sizeof( g_annotationKeywords ) / sizeof( g_annotationKeywords[ 0 ] );

/** Keywords of JSON Schema 2020-12 that are known and refused, since they cannot be checked */
static const char* g_unsupportedKeywords[] =
{
    "contains", "minContains", "maxContains", "dependentSchemas", "unevaluatedProperties", "unevaluatedItems",
    "$anchor", "$dynamicRef", "$dynamicAnchor", "$vocabulary"
};
static const UInt32 g_unsupportedKeywordCount = sizeof( g_unsupportedKeywords ) / sizeof( g_unsupportedKeywords[ 0 ] );

/*-------------------------------------------------------------------------*/

static bool
IsOneOf( const CString& keyword, const char** list, UInt32 count )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<count; ++i )
    {
        if ( keyword == list[ i ] )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
BeginsWith( const CString& text, const char* prefix )
{GUCEF_TRACE;

    size_t prefixSize = strlen( prefix );
    return text.ByteSize() > prefixSize && 0 == strncmp( text.C_String(), prefix, prefixSize );
}

/*-------------------------------------------------------------------------*/

static CString
Quoted( const CString& text )
{GUCEF_TRACE;

    return "\"" + text + "\"";
}

/*-------------------------------------------------------------------------*/

static CString
Located( const CString& pointer, const CString& message )
{GUCEF_TRACE;

    return "#" + pointer + ": " + message;
}

/*-------------------------------------------------------------------------*/

static Int32
HexDigitValue( char c )
{GUCEF_TRACE;

    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

/** Decodes the %XX escapes of a URI fragment. False when an escape is malformed */
static bool
PercentDecode( const CString& text, CString& decoded )
{GUCEF_TRACE;

    const char* data = text.C_String();
    UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    GUCEF::vector< char > bytes;
    bytes.reserve( size + 1 );
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '%' == data[ i ] )
        {
            if ( i + 2 >= size )
                return false;
            Int32 high = HexDigitValue( data[ i+1 ] );
            Int32 low = HexDigitValue( data[ i+2 ] );
            if ( high < 0 || low < 0 )
                return false;
            bytes.push_back( (char) ( high * 16 + low ) );
            i += 2;
        }
        else
        {
            bytes.push_back( data[ i ] );
        }
    }
    bytes.push_back( '\0' );
    decoded = CString( &bytes[ 0 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

/** True when every '~' of the JSON pointer starts a valid escape: "~0" or "~1" */
static bool
HasValidPointerEscapes( const CString& pointer )
{GUCEF_TRACE;

    const char* data = pointer.C_String();
    UInt32 size = pointer.ByteSize() > 0 ? pointer.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '~' == data[ i ] )
        {
            if ( i + 1 >= size || ( '0' != data[ i+1 ] && '1' != data[ i+1 ] ) )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CJsonSchema::SValueRef::SValueRef( void )
    : node( GUCEF_NULL )
    , scalar( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJsonSchema::SValueRef::SValueRef( const CDataNode* valueNode, const CVariant* scalarValue )
    : node( valueNode )
    , scalar( scalarValue )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJsonSchema::SSubschema::SSubschema( void )
    : isBooleanSchema( false )
    , booleanValue( true )
    , typeMask( 0 )
    , hasConst( false )
    , constValue()
    , hasEnum( false )
    , enumValues()
    , ref( NoSubschema )
    , refText()
    , hasMinimum( false )
    , hasMaximum( false )
    , hasExclusiveMinimum( false )
    , hasExclusiveMaximum( false )
    , hasMultipleOf( false )
    , minimum()
    , maximum()
    , exclusiveMinimum()
    , exclusiveMaximum()
    , multipleOf()
    , hasMinLength( false )
    , hasMaxLength( false )
    , minLength( 0 )
    , maxLength( 0 )
    , properties()
    , required()
    , additionalProperties( NoSubschema )
    , propertyNames( NoSubschema )
    , hasMinProperties( false )
    , hasMaxProperties( false )
    , minProperties( 0 )
    , maxProperties( 0 )
    , dependentRequired()
    , items( NoSubschema )
    , prefixItems()
    , hasMinItems( false )
    , hasMaxItems( false )
    , minItems( 0 )
    , maxItems( 0 )
    , uniqueItems( false )
    , allOf()
    , anyOf()
    , oneOf()
    , notSchema( NoSubschema )
    , ifSchema( NoSubschema )
    , thenSchema( NoSubschema )
    , elseSchema( NoSubschema )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJsonSchema::CJsonSchema( void )
    : m_source()
    , m_subschemas()
    , m_compiled( false )
    , m_maxDepth( DefaultMaxDepth )
    , m_maxSubschemas( DefaultMaxSubschemas )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJsonSchema::CJsonSchema( const CJsonSchema& src )
    : m_source()
    , m_subschemas()
    , m_compiled( false )
    , m_maxDepth( src.m_maxDepth )
    , m_maxSubschemas( src.m_maxSubschemas )
{GUCEF_TRACE;

    // The compiled form points into the source, so the copy compiles its own copy of the source
    if ( src.m_compiled )
    {
        m_source = src.m_source;
        CString error;
        CompileFromSource( error );
    }
}

/*-------------------------------------------------------------------------*/

CJsonSchema::~CJsonSchema()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJsonSchema&
CJsonSchema::operator=( const CJsonSchema& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        Clear();
        m_maxDepth = src.m_maxDepth;
        m_maxSubschemas = src.m_maxSubschemas;
        if ( src.m_compiled )
        {
            m_source = src.m_source;
            CString error;
            CompileFromSource( error );
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::IsCompiled( void ) const
{GUCEF_TRACE;

    return m_compiled;
}

/*-------------------------------------------------------------------------*/

void
CJsonSchema::Clear( void )
{GUCEF_TRACE;

    m_compiled = false;
    m_subschemas.clear();
    m_source.Clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CJsonSchema::GetSubschemaCount( void ) const
{GUCEF_TRACE;

    return m_compiled ? (UInt32) m_subschemas.size() : 0;
}

/*-------------------------------------------------------------------------*/

const CDataNode&
CJsonSchema::GetSchema( void ) const
{GUCEF_TRACE;

    return m_source;
}

/*-------------------------------------------------------------------------*/

CJsonSchema::EJsonKind
CJsonSchema::KindOfScalar( const CVariant& value )
{GUCEF_TRACE;

    if ( !value.IsInitialized() || value.IsNIL() || value.IsNULL() )
        return JSONKIND_NULL;
    if ( value.IsBoolean() )
        return JSONKIND_BOOLEAN;
    if ( value.IsInteger() )
        return JSONKIND_INTEGER;
    if ( value.IsNumber() )
        return JSONKIND_NUMBER;
    if ( value.IsString() )
        return JSONKIND_STRING;
    return JSONKIND_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

CJsonSchema::EJsonKind
CJsonSchema::KindOf( const SValueRef& value )
{GUCEF_TRACE;

    if ( GUCEF_NULL != value.scalar )
        return KindOfScalar( *value.scalar );
    if ( GUCEF_NULL == value.node )
        return JSONKIND_UNSUPPORTED;

    const CDataNode& node = *value.node;
    Int32 nodeType = node.GetNodeType();
    if ( GUCEF_DATATYPE_ARRAY == nodeType || GUCEF_DATATYPE_SET == nodeType )
        return JSONKIND_ARRAY;

    bool hasMembers = node.GetAttCount() > 0 || node.HasChildren();
    if ( !hasMembers && node.HasValue() )
        return KindOfScalar( node.GetValue() );     // a node that only holds a value is that value
    if ( hasMembers || GUCEF_DATATYPE_OBJECT == nodeType || GUCEF_DATATYPE_UNKNOWN == nodeType )
        return JSONKIND_OBJECT;
    if ( GUCEF_DATATYPE_NIL == nodeType || GUCEF_DATATYPE_NULL == nodeType )
        return JSONKIND_NULL;
    return JSONKIND_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

CJsonSchema::SValueRef
CJsonSchema::ChildAsValue( const CDataNode& child )
{GUCEF_TRACE;

    Int32 nodeType = child.GetNodeType();
    if ( GUCEF_DATATYPE_ARRAY != nodeType && GUCEF_DATATYPE_SET != nodeType &&
         0 == child.GetAttCount() && !child.HasChildren() && child.HasValue() )
    {
        return SValueRef( GUCEF_NULL, &child.GetValue() );
    }
    return SValueRef( &child, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::FindMember( const CDataNode& object, const CString& name, SValueRef& member )
{GUCEF_TRACE;

    const CDataNode::TKeyValuePair* attribute = object.GetAttribute( name );
    if ( GUCEF_NULL != attribute )
    {
        member = SValueRef( GUCEF_NULL, &attribute->second );
        return true;
    }
    CDataNode::const_iterator i = object.ConstBegin();
    while ( i != object.ConstEnd() )
    {
        if ( (*i)->GetName() == name )
        {
            member = ChildAsValue( *(*i) );
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CJsonSchema::CountMembers( const CDataNode& object )
{GUCEF_TRACE;

    return object.GetAttCount() + object.GetNrOfDirectChildNodes();
}

/*-------------------------------------------------------------------------*/

CString
CJsonSchema::EscapePointerToken( const CString& token )
{GUCEF_TRACE;

    if ( token.HasChar( '~' ) < 0 && token.HasChar( '/' ) < 0 )
        return token;
    return token.ReplaceSubstr( "~", "~0" ).ReplaceSubstr( "/", "~1" );
}

/*-------------------------------------------------------------------------*/

/** The nesting depth of a JSON value: 0 for a scalar, 1 for an object or array of scalars. Stops counting above 'limit' */
static UInt32
ValueDepth( const CDataNode& node, UInt32 limit )
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_ARRAY != node.GetNodeType() && GUCEF_DATATYPE_SET != node.GetNodeType() &&
         0 == node.GetAttCount() && !node.HasChildren() && node.HasValue() )
        return 0;
    if ( 0 == limit )
        return 1;
    UInt32 deepest = 0;
    CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        UInt32 childDepth = ValueDepth( *(*i), limit-1 );
        if ( childDepth > deepest )
            deepest = childDepth;
        if ( deepest >= limit )
            break;
        ++i;
    }
    return 1 + deepest;
}

/*-------------------------------------------------------------------------*/

/** A non-negative integer keyword value: an integer, or a number with a zero fraction */
static bool
AsNonNegativeInteger( const CVariant* value, UInt64& result )
{GUCEF_TRACE;

    if ( GUCEF_NULL == value )
        return false;
    if ( value->IsInteger() )
    {
        if ( value->IsUnsignedInteger() )
        {
            result = value->AsUInt64();
            return true;
        }
        Int64 signedValue = value->AsInt64();
        if ( signedValue < 0 )
            return false;
        result = (UInt64) signedValue;
        return true;
    }
    if ( value->IsFloat() )
    {
        Float64 number = value->AsFloat64();
        if ( number != number || number < 0.0 || number > 9007199254740992.0 || floor( number ) != number )
            return false;
        result = (UInt64) number;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::Compile( const CDataNode& schema ,
                      CString& error          ,
                      UInt32 maxDepth         ,
                      UInt32 maxSubschemas    )
{GUCEF_TRACE;

    try
    {
        Clear();
        m_maxDepth = maxDepth;
        m_maxSubschemas = maxSubschemas;
        m_source = schema;
        return CompileFromSource( error );
    }
    catch ( ... )
    {
        Clear();
        error = "the schema could not be compiled: out of memory or an internal error";
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::CompileFromSource( CString& error )
{GUCEF_TRACE;

    m_compiled = false;
    m_subschemas.clear();

    SCompileContext context( m_maxDepth, m_maxSubschemas );
    UInt32 rootIndex = NoSubschema;
    if ( !CompileSubschema( context, ChildAsValue( m_source ), CString::Empty, 1, rootIndex ) ||
         !ResolveReferences( context )                                                         ||
         !CheckReferenceCycles( context )                                                      )
    {
        error = context.error;
        m_subschemas.clear();
        m_source.Clear();
        return false;
    }
    m_compiled = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::CompileSubschema( SCompileContext& context ,
                               const SValueRef& value   ,
                               const CString& pointer   ,
                               UInt32 depth             ,
                               UInt32& index            )
{GUCEF_TRACE;

    if ( depth > context.maxDepth )
    {
        context.error = Located( pointer, "the schema is nested deeper than " + UInt32ToString( context.maxDepth ) + " levels" );
        return false;
    }
    if ( m_subschemas.size() >= context.maxSubschemas )
    {
        context.error = Located( pointer, "the schema has more than " + UInt32ToString( context.maxSubschemas ) + " subschemas" );
        return false;
    }

    index = (UInt32) m_subschemas.size();
    m_subschemas.push_back( SSubschema() );
    context.pointers[ pointer ] = index;
    context.pointerOfIndex.push_back( pointer );

    EJsonKind kind = KindOf( value );
    if ( JSONKIND_BOOLEAN == kind )
    {
        m_subschemas[ index ].isBooleanSchema = true;
        m_subschemas[ index ].booleanValue = ( GUCEF_NULL != value.scalar ? value.scalar : &value.node->GetValue() )->AsBool();
        return true;
    }
    if ( JSONKIND_OBJECT != kind )
    {
        context.error = Located( pointer, "a schema must be an object or a boolean" );
        return false;
    }

    const CDataNode& schemaNode = *value.node;
    CDataNode::TAttributeMap::const_iterator a = schemaNode.AttributeBegin();
    while ( a != schemaNode.AttributeEnd() )
    {
        if ( !CompileKeyword( context, index, (*a).first, SValueRef( GUCEF_NULL, &(*a).second ), pointer, depth ) )
            return false;
        ++a;
    }

    GUCEF::set< CString > childKeywords;
    CDataNode::const_iterator c = schemaNode.ConstBegin();
    while ( c != schemaNode.ConstEnd() )
    {
        const CString& keyword = (*c)->GetName();
        if ( GUCEF_NULL != schemaNode.GetAttribute( keyword ) || !childKeywords.insert( keyword ).second )
        {
            context.error = Located( pointer, "the keyword " + Quoted( keyword ) + " appears more than once" );
            return false;
        }
        if ( !CompileKeyword( context, index, keyword, ChildAsValue( *(*c) ), pointer, depth ) )
            return false;
        ++c;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::CompileSchemaArray( SCompileContext& context ,
                                 const SValueRef& value   ,
                                 const CString& pointer   ,
                                 UInt32 depth             ,
                                 TIndexVector& indexes    )
{GUCEF_TRACE;

    if ( JSONKIND_ARRAY != KindOf( value ) || !value.node->HasChildren() )
    {
        context.error = Located( pointer, "must be a non-empty array of schemas" );
        return false;
    }
    UInt32 position = 0;
    CDataNode::const_iterator i = value.node->ConstBegin();
    while ( i != value.node->ConstEnd() )
    {
        UInt32 subschema = NoSubschema;
        if ( !CompileSubschema( context, ChildAsValue( *(*i) ), pointer + "/" + UInt32ToString( position ), depth+1, subschema ) )
            return false;
        indexes.push_back( subschema );
        ++position;
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::CompileSchemaMap( SCompileContext& context ,
                               const SValueRef& value   ,
                               const CString& pointer   ,
                               UInt32 depth             ,
                               TStringIndexMap* indexes )
{GUCEF_TRACE;

    if ( JSONKIND_OBJECT != KindOf( value ) )
    {
        context.error = Located( pointer, "must be an object whose members are schemas" );
        return false;
    }
    const CDataNode& mapNode = *value.node;
    CDataNode::TAttributeMap::const_iterator a = mapNode.AttributeBegin();
    while ( a != mapNode.AttributeEnd() )
    {
        UInt32 subschema = NoSubschema;
        if ( !CompileSubschema( context, SValueRef( GUCEF_NULL, &(*a).second ), pointer + "/" + EscapePointerToken( (*a).first ), depth+1, subschema ) )
            return false;
        if ( GUCEF_NULL != indexes )
            (*indexes)[ (*a).first ] = subschema;
        ++a;
    }
    CDataNode::const_iterator c = mapNode.ConstBegin();
    while ( c != mapNode.ConstEnd() )
    {
        const CString& name = (*c)->GetName();
        if ( GUCEF_NULL != mapNode.GetAttribute( name ) || ( GUCEF_NULL != indexes && indexes->find( name ) != indexes->end() ) )
        {
            context.error = Located( pointer, "the member " + Quoted( name ) + " appears more than once" );
            return false;
        }
        UInt32 subschema = NoSubschema;
        if ( !CompileSubschema( context, ChildAsValue( *(*c) ), pointer + "/" + EscapePointerToken( name ), depth+1, subschema ) )
            return false;
        if ( GUCEF_NULL != indexes )
            (*indexes)[ name ] = subschema;
        ++c;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Reads an array of unique strings */
static bool
ReadUniqueStrings( const CDataNode* arrayNode, GUCEF::vector< CString >& strings, CString& problem )
{GUCEF_TRACE;

    GUCEF::set< CString > seen;
    CDataNode::const_iterator i = arrayNode->ConstBegin();
    while ( i != arrayNode->ConstEnd() )
    {
        const CDataNode& element = *(*i);
        if ( element.GetAttCount() > 0 || element.HasChildren() || !element.HasValue() || !element.GetValue().IsString() )
        {
            problem = "must be an array of strings";
            return false;
        }
        CString text = element.GetValue().AsString();
        if ( !seen.insert( text ).second )
        {
            problem = "must not name " + Quoted( text ) + " twice";
            return false;
        }
        strings.push_back( text );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::CompileKeyword( SCompileContext& context ,
                             UInt32 index             ,
                             const CString& keyword   ,
                             const SValueRef& value   ,
                             const CString& pointer   ,
                             UInt32 depth             )
{GUCEF_TRACE;

    const CString keywordPointer = pointer + "/" + EscapePointerToken( keyword );
    EJsonKind kind = KindOf( value );

    // Annotations and extensions
    if ( IsOneOf( keyword, g_annotationKeywords, g_annotationKeywordCount ) || BeginsWith( keyword, "x-" ) )
        return true;

    if ( keyword == "$schema" )
    {
        if ( JSONKIND_STRING != kind )
        {
            context.error = Located( keywordPointer, "must be a string" );
            return false;
        }
        CString dialect = value.scalar->AsString();
        if ( dialect != Dialect2020_12 && dialect != Dialect2020_12 + "#" )
        {
            context.error = Located( keywordPointer, "dialect not supported: " + Quoted( dialect ) + ", only " + Dialect2020_12 + " is" );
            return false;
        }
        return true;
    }
    if ( keyword == "$id" )
    {
        if ( !pointer.IsNULLOrEmpty() )
        {
            context.error = Located( keywordPointer, "$id is only supported at the root: below it, it changes what local references resolve to" );
            return false;
        }
        if ( JSONKIND_STRING != kind )
        {
            context.error = Located( keywordPointer, "must be a string" );
            return false;
        }
        return true;
    }
    if ( keyword == "$ref" )
    {
        if ( JSONKIND_STRING != kind )
        {
            context.error = Located( keywordPointer, "must be a string" );
            return false;
        }
        CString reference = value.scalar->AsString();
        if ( !BeginsWith( reference, "#" ) )
        {
            context.error = Located( keywordPointer, "the reference " + Quoted( reference ) + " is not local: only references within the schema (\"#...\") are supported, other documents and network references are never dereferenced" );
            return false;
        }
        if ( reference.Length() > 1 && !BeginsWith( reference, "#/" ) )
        {
            context.error = Located( keywordPointer, "the reference " + Quoted( reference ) + " names an anchor: only JSON pointer references (\"#/...\") are supported" );
            return false;
        }
        m_subschemas[ index ].refText = reference;
        return true;
    }
    if ( keyword == "$defs" )
        return CompileSchemaMap( context, value, keywordPointer, depth, GUCEF_NULL );

    if ( keyword == "type" )
    {
        GUCEF::vector< CString > typeNames;
        if ( JSONKIND_STRING == kind )
        {
            typeNames.push_back( value.scalar->AsString() );
        }
        else
        if ( JSONKIND_ARRAY == kind && value.node->HasChildren() )
        {
            CString problem;
            if ( !ReadUniqueStrings( value.node, typeNames, problem ) )
            {
                context.error = Located( keywordPointer, problem );
                return false;
            }
        }
        else
        {
            context.error = Located( keywordPointer, "must be a type name or a non-empty array of type names" );
            return false;
        }

        UInt32 mask = 0;
        for ( UInt32 i=0; i<typeNames.size(); ++i )
        {
            const CString& typeName = typeNames[ i ];
            if ( typeName == "null" )           mask |= TYPEBIT_NULL;
            else if ( typeName == "boolean" )   mask |= TYPEBIT_BOOLEAN;
            else if ( typeName == "object" )    mask |= TYPEBIT_OBJECT;
            else if ( typeName == "array" )     mask |= TYPEBIT_ARRAY;
            else if ( typeName == "number" )    mask |= TYPEBIT_NUMBER;
            else if ( typeName == "integer" )   mask |= TYPEBIT_INTEGER;
            else if ( typeName == "string" )    mask |= TYPEBIT_STRING;
            else
            {
                context.error = Located( keywordPointer, "unknown type " + Quoted( typeName ) );
                return false;
            }
        }
        m_subschemas[ index ].typeMask = mask;
        return true;
    }
    if ( keyword == "enum" )
    {
        if ( JSONKIND_ARRAY != kind )
        {
            context.error = Located( keywordPointer, "must be an array" );
            return false;
        }
        TValueRefVector values;
        CDataNode::const_iterator i = value.node->ConstBegin();
        while ( i != value.node->ConstEnd() )
        {
            if ( depth + ValueDepth( *(*i), context.maxDepth ) > context.maxDepth )
            {
                context.error = Located( keywordPointer, "a value is nested deeper than " + UInt32ToString( context.maxDepth ) + " levels" );
                return false;
            }
            values.push_back( ChildAsValue( *(*i) ) );
            ++i;
        }
        m_subschemas[ index ].hasEnum = true;
        m_subschemas[ index ].enumValues = values;
        return true;
    }
    if ( keyword == "const" )
    {
        if ( GUCEF_NULL != value.node && depth + ValueDepth( *value.node, context.maxDepth ) > context.maxDepth )
        {
            context.error = Located( keywordPointer, "the value is nested deeper than " + UInt32ToString( context.maxDepth ) + " levels" );
            return false;
        }
        if ( JSONKIND_UNSUPPORTED == kind )
        {
            context.error = Located( keywordPointer, "the value has a type JSON does not have" );
            return false;
        }
        m_subschemas[ index ].hasConst = true;
        m_subschemas[ index ].constValue = value;
        return true;
    }

    // Numbers
    if ( keyword == "minimum" || keyword == "maximum" || keyword == "exclusiveMinimum" || keyword == "exclusiveMaximum" || keyword == "multipleOf" )
    {
        if ( ( JSONKIND_INTEGER != kind && JSONKIND_NUMBER != kind ) || GUCEF_NULL == value.scalar )
        {
            context.error = Located( keywordPointer, "must be a number" );
            return false;
        }
        Float64 number = value.scalar->AsFloat64();
        if ( number != number || number - number != 0.0 )
        {
            context.error = Located( keywordPointer, "must be a finite number" );
            return false;
        }
        SSubschema& subschema = m_subschemas[ index ];
        if ( keyword == "minimum" )                 { subschema.hasMinimum = true; subschema.minimum = *value.scalar; }
        else if ( keyword == "maximum" )            { subschema.hasMaximum = true; subschema.maximum = *value.scalar; }
        else if ( keyword == "exclusiveMinimum" )   { subschema.hasExclusiveMinimum = true; subschema.exclusiveMinimum = *value.scalar; }
        else if ( keyword == "exclusiveMaximum" )   { subschema.hasExclusiveMaximum = true; subschema.exclusiveMaximum = *value.scalar; }
        else
        {
            if ( number <= 0.0 )
            {
                context.error = Located( keywordPointer, "must be greater than 0" );
                return false;
            }
            subschema.hasMultipleOf = true;
            subschema.multipleOf = *value.scalar;
        }
        return true;
    }

    // Counts
    if ( keyword == "minLength" || keyword == "maxLength" || keyword == "minItems" || keyword == "maxItems" ||
         keyword == "minProperties" || keyword == "maxProperties" )
    {
        UInt64 count = 0;
        if ( !AsNonNegativeInteger( value.scalar, count ) )
        {
            context.error = Located( keywordPointer, "must be a non-negative integer" );
            return false;
        }
        SSubschema& subschema = m_subschemas[ index ];
        if ( keyword == "minLength" )           { subschema.hasMinLength = true; subschema.minLength = count; }
        else if ( keyword == "maxLength" )      { subschema.hasMaxLength = true; subschema.maxLength = count; }
        else if ( keyword == "minItems" )       { subschema.hasMinItems = true; subschema.minItems = count; }
        else if ( keyword == "maxItems" )       { subschema.hasMaxItems = true; subschema.maxItems = count; }
        else if ( keyword == "minProperties" )  { subschema.hasMinProperties = true; subschema.minProperties = count; }
        else                                    { subschema.hasMaxProperties = true; subschema.maxProperties = count; }
        return true;
    }

    // Objects
    if ( keyword == "properties" )
    {
        TStringIndexMap properties;
        if ( !CompileSchemaMap( context, value, keywordPointer, depth, &properties ) )
            return false;
        m_subschemas[ index ].properties = properties;
        return true;
    }
    if ( keyword == "required" )
    {
        GUCEF::vector< CString > names;
        CString problem;
        if ( JSONKIND_ARRAY != kind )
            problem = "must be an array of strings";
        if ( !problem.IsNULLOrEmpty() || !ReadUniqueStrings( value.node, names, problem ) )
        {
            context.error = Located( keywordPointer, problem );
            return false;
        }
        m_subschemas[ index ].required = names;
        return true;
    }
    if ( keyword == "dependentRequired" )
    {
        if ( JSONKIND_OBJECT != kind || value.node->GetAttCount() > 0 )
        {
            context.error = Located( keywordPointer, "must be an object whose members are arrays of strings" );
            return false;
        }
        TStringListMap dependencies;
        CDataNode::const_iterator i = value.node->ConstBegin();
        while ( i != value.node->ConstEnd() )
        {
            const CString& name = (*i)->GetName();
            GUCEF::vector< CString > names;
            CString problem;
            if ( JSONKIND_ARRAY != KindOf( ChildAsValue( *(*i) ) ) )
                problem = "must be an array of strings";
            if ( !problem.IsNULLOrEmpty() || !ReadUniqueStrings( *i, names, problem ) )
            {
                context.error = Located( keywordPointer + "/" + EscapePointerToken( name ), problem );
                return false;
            }
            if ( dependencies.find( name ) != dependencies.end() )
            {
                context.error = Located( keywordPointer, "the member " + Quoted( name ) + " appears more than once" );
                return false;
            }
            dependencies[ name ] = names;
            ++i;
        }
        m_subschemas[ index ].dependentRequired = dependencies;
        return true;
    }

    // Arrays
    if ( keyword == "uniqueItems" )
    {
        if ( JSONKIND_BOOLEAN != kind || GUCEF_NULL == value.scalar )
        {
            context.error = Located( keywordPointer, "must be a boolean" );
            return false;
        }
        m_subschemas[ index ].uniqueItems = value.scalar->AsBool();
        return true;
    }
    if ( keyword == "items" && JSONKIND_ARRAY == kind )
    {
        context.error = Located( keywordPointer, "must be a schema: an array of schemas is the form of earlier dialects, 2020-12 uses prefixItems for it" );
        return false;
    }
    if ( keyword == "prefixItems" || keyword == "allOf" || keyword == "anyOf" || keyword == "oneOf" )
    {
        TIndexVector indexes;
        if ( !CompileSchemaArray( context, value, keywordPointer, depth, indexes ) )
            return false;
        SSubschema& subschema = m_subschemas[ index ];
        if ( keyword == "prefixItems" )     subschema.prefixItems = indexes;
        else if ( keyword == "allOf" )      subschema.allOf = indexes;
        else if ( keyword == "anyOf" )      subschema.anyOf = indexes;
        else                                subschema.oneOf = indexes;
        return true;
    }

    // Keywords with one subschema
    if ( keyword == "items" || keyword == "additionalProperties" || keyword == "propertyNames" ||
         keyword == "not" || keyword == "if" || keyword == "then" || keyword == "else" )
    {
        UInt32 subschemaIndex = NoSubschema;
        if ( !CompileSubschema( context, value, keywordPointer, depth+1, subschemaIndex ) )
            return false;
        SSubschema& subschema = m_subschemas[ index ];
        if ( keyword == "items" )                       subschema.items = subschemaIndex;
        else if ( keyword == "additionalProperties" )   subschema.additionalProperties = subschemaIndex;
        else if ( keyword == "propertyNames" )          subschema.propertyNames = subschemaIndex;
        else if ( keyword == "not" )                    subschema.notSchema = subschemaIndex;
        else if ( keyword == "if" )                     subschema.ifSchema = subschemaIndex;
        else if ( keyword == "then" )                   subschema.thenSchema = subschemaIndex;
        else                                            subschema.elseSchema = subschemaIndex;
        return true;
    }

    if ( keyword == "pattern" || keyword == "patternProperties" )
    {
        context.error = Located( keywordPointer, "the keyword " + Quoted( keyword ) + " is not supported: it needs a regular expression engine, which gucefCORE does not have" );
        return false;
    }
    if ( IsOneOf( keyword, g_unsupportedKeywords, g_unsupportedKeywordCount ) )
    {
        context.error = Located( keywordPointer, "the keyword " + Quoted( keyword ) + " is not supported" );
        return false;
    }
    context.error = Located( keywordPointer, "unknown keyword " + Quoted( keyword ) );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::ResolveReferences( SCompileContext& context )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_subschemas.size(); ++i )
    {
        SSubschema& subschema = m_subschemas[ i ];
        if ( subschema.refText.IsNULLOrEmpty() )
            continue;

        const CString location = context.pointerOfIndex[ i ] + "/$ref";
        CString targetPointer;
        if ( !PercentDecode( subschema.refText.CutChars( 1, true ), targetPointer ) || !HasValidPointerEscapes( targetPointer ) )
        {
            context.error = Located( location, "the reference " + Quoted( subschema.refText ) + " is not a valid JSON pointer" );
            return false;
        }
        TStringIndexMap::const_iterator target = context.pointers.find( targetPointer );
        if ( target == context.pointers.end() )
        {
            context.error = Located( location, "the reference " + Quoted( subschema.refText ) + " does not point at a schema in this document" );
            return false;
        }
        subschema.ref = (*target).second;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CJsonSchema::GetInPlaceApplicators( UInt32 index, TIndexVector& targets ) const
{GUCEF_TRACE;

    // The keywords that apply a subschema to the same instance location: a cycle of them never ends
    const SSubschema& subschema = m_subschemas[ index ];
    targets.clear();
    if ( NoSubschema != subschema.ref )
        targets.push_back( subschema.ref );
    targets.insert( targets.end(), subschema.allOf.begin(), subschema.allOf.end() );
    targets.insert( targets.end(), subschema.anyOf.begin(), subschema.anyOf.end() );
    targets.insert( targets.end(), subschema.oneOf.begin(), subschema.oneOf.end() );
    if ( NoSubschema != subschema.notSchema )
        targets.push_back( subschema.notSchema );
    if ( NoSubschema != subschema.ifSchema )
    {
        targets.push_back( subschema.ifSchema );
        if ( NoSubschema != subschema.thenSchema )
            targets.push_back( subschema.thenSchema );
        if ( NoSubschema != subschema.elseSchema )
            targets.push_back( subschema.elseSchema );
    }
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchema::CheckReferenceCycles( SCompileContext& context )
{GUCEF_TRACE;

    // Depth first search without recursion, so a long chain cannot exhaust the stack
    // state: 0 not visited, 1 on the current path, 2 done
    const UInt32 count = (UInt32) m_subschemas.size();
    GUCEF::vector< UInt8 > state( count, 0 );
    GUCEF::vector< TIndexVector > targets( count );
    for ( UInt32 i=0; i<count; ++i )
        GetInPlaceApplicators( i, targets[ i ] );

    GUCEF::vector< std::pair< UInt32, UInt32 > > stack;        // subschema, next target to look at
    for ( UInt32 start=0; start<count; ++start )
    {
        if ( 0 != state[ start ] )
            continue;
        stack.push_back( std::pair< UInt32, UInt32 >( start, 0 ) );
        state[ start ] = 1;
        while ( !stack.empty() )
        {
            std::pair< UInt32, UInt32 >& top = stack.back();
            const TIndexVector& next = targets[ top.first ];
            if ( top.second >= next.size() )
            {
                state[ top.first ] = 2;
                stack.pop_back();
                continue;
            }
            UInt32 target = next[ top.second ];
            ++top.second;
            if ( 1 == state[ target ] )
            {
                context.error = Located( context.pointerOfIndex[ target ], "the schema applies itself to the same value through a cycle of references, which never ends" );
                return false;
            }
            if ( 0 == state[ target ] )
            {
                state[ target ] = 1;
                stack.push_back( std::pair< UInt32, UInt32 >( target, 0 ) );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
