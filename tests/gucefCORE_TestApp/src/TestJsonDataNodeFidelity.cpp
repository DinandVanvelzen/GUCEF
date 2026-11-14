/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestJsonDataNodeFidelity.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CDStoreCodecRegistry::TDStoreCodecPtr TCodecPtr;

/*-------------------------------------------------------------------------*/

static bool
GetJsonCodec( TCodecPtr& codec )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
JsonTestCodecAvailable( void )
{GUCEF_TRACE;

    TCodecPtr codec;
    if ( GetJsonCodec( codec ) )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginJSONPARSER_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/dstorepluginJSONPARSER";
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginJSONPARSER";
    CORE::CString secondPluginPath = "$MODULEDIR$/dstorepluginJSONPARSER_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
    return GetJsonCodec( codec );
}

/*-------------------------------------------------------------------------*/

bool
ParseJsonTestText( const CORE::CString& text, CORE::CDataNode& tree )
{GUCEF_TRACE;

    TCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
        return false;
    tree.Clear();
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( text.ByteSize() - 1, text.C_String() );
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &tree, &access );
}

/*-------------------------------------------------------------------------*/

bool
WriteJsonTestText( const CORE::CDataNode& tree, CORE::CString& text )
{GUCEF_TRACE;

    TCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
        return false;
    CORE::CDynamicBuffer buffer;
    {
        CORE::CDynamicBufferAccess access( &buffer, false );
        if ( !codec->StoreDataTree( &tree, &access ) )
            return false;
    }
    UInt32 size = buffer.GetDataSize();
    const char* data = static_cast< const char* >( buffer.GetConstBufferPtr() );
    while ( size > 0 && '\0' == data[ size-1 ] )
        --size;
    text = CORE::CString( data, size );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The type id of the attribute, or 0xFF when there is none */
static UInt8
AttributeType( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    return GUCEF_NULL != attribute ? attribute->second.GetTypeId() : (UInt8) 0xFF;
}

/*-------------------------------------------------------------------------*/

static const CORE::CDataNode*
DirectChild( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    CORE::CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        if ( (*i)->GetName() == name )
            return *i;
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

static bool
SameScalar( const CORE::CVariant& a, const CORE::CVariant& b )
{GUCEF_TRACE;

    return a.GetTypeId() == b.GetTypeId() && a.AsString() == b.AsString();
}

/*-------------------------------------------------------------------------*/

/**
 *  Compares two trees completely: names, node types, values, attributes, and the children in order.
 *  CDataNode::operator==() only looks at the name and the attributes
 */
static bool
DeepEqual( const CORE::CDataNode& a, const CORE::CDataNode& b )
{GUCEF_TRACE;

    if ( a.GetName() != b.GetName() || a.GetNodeType() != b.GetNodeType() || a.HasValue() != b.HasValue() ||
         a.GetAttCount() != b.GetAttCount() || a.GetNrOfDirectChildNodes() != b.GetNrOfDirectChildNodes() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonDataNodeFidelity: nodes differ: \"" << a.GetName() << "\" type " << a.GetNodeType() <<
                         " atts " << a.GetAttCount() << " children " << a.GetNrOfDirectChildNodes() << " vs \"" << b.GetName() << "\" type " <<
                         b.GetNodeType() << " atts " << b.GetAttCount() << " children " << b.GetNrOfDirectChildNodes() );
        return false;
    }
    if ( a.HasValue() && !SameScalar( a.GetValue(), b.GetValue() ) )
        return false;
    CORE::CDataNode::TAttributeMap::const_iterator att = a.AttributeBegin();
    while ( att != a.AttributeEnd() )
    {
        const CORE::CDataNode::TKeyValuePair* other = b.GetAttribute( (*att).first );
        if ( GUCEF_NULL == other || !SameScalar( (*att).second, other->second ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonDataNodeFidelity: attribute \"" << (*att).first << "\" differs" );
            return false;
        }
        ++att;
    }
    CORE::CDataNode::const_iterator i = a.ConstBegin();
    CORE::CDataNode::const_iterator n = b.ConstBegin();
    while ( i != a.ConstEnd() && n != b.ConstEnd() )
    {
        if ( !DeepEqual( *(*i), *(*n) ) )
            return false;
        ++i;
        ++n;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ParseLogged( const char* text, CORE::CDataNode& tree )
{GUCEF_TRACE;

    if ( ParseJsonTestText( text, tree ) )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonDataNodeFidelity: cannot parse: " << text );
    return false;
}

/*-------------------------------------------------------------------------*/

/** Writes the tree and parses the text again */
static bool
RoundTrip( const CORE::CDataNode& tree, CORE::CDataNode& again, CORE::CString& text )
{GUCEF_TRACE;

    if ( !WriteJsonTestText( tree, text ) )
        return false;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "JsonDataNodeFidelity: written: " << text );
    return ParseJsonTestText( text, again );
}

/*-------------------------------------------------------------------------*/

static bool
HasByte( const CORE::CString& text, char byte )
{GUCEF_TRACE;

    return GUCEF_NULL != memchr( text.C_String(), byte, text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
}

/*-------------------------------------------------------------------------*/

static void
PerformScalarTypeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON scalars: the variant types the json codec gives" )
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( "{ \"i\": 5, \"neg\": -7, \"max\": 9223372036854775807, \"f\": 2.5, \"two\": 2.0, \"e\": 1e3,"
                                      "  \"big\": 12345678901234567890, \"t\": true, \"fl\": false, \"n\": null, \"s\": \"x\" }", tree ) );

            // every scalar member of an object is an attribute
            ASSERT_TRUE( 11 == tree.GetAttCount() );
            ASSERT_FALSE( tree.HasChildren() );
            ASSERT_TRUE( GUCEF_DATATYPE_OBJECT == tree.GetNodeType() );

            ASSERT_TRUE( GUCEF_DATATYPE_INT64 == AttributeType( tree, "i" ) );
            ASSERT_TRUE( 5 == tree.GetAttributeValue( "i" ).AsInt64() );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64 == AttributeType( tree, "neg" ) );
            ASSERT_TRUE( -7 == tree.GetAttributeValue( "neg" ).AsInt64() );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64 == AttributeType( tree, "max" ) );
            ASSERT_TRUE( GUCEF_INT64MAX == tree.GetAttributeValue( "max" ).AsInt64() );

            // a fraction, a number written with a fraction and an exponent are doubles, also when they are whole
            ASSERT_TRUE( GUCEF_DATATYPE_FLOAT64 == AttributeType( tree, "f" ) );
            ASSERT_TRUE( 2.5 == tree.GetAttributeValue( "f" ).AsFloat64() );
            ASSERT_TRUE( GUCEF_DATATYPE_FLOAT64 == AttributeType( tree, "two" ) );
            ASSERT_TRUE( 2.0 == tree.GetAttributeValue( "two" ).AsFloat64() );
            ASSERT_TRUE( GUCEF_DATATYPE_FLOAT64 == AttributeType( tree, "e" ) );
            ASSERT_TRUE( 1000.0 == tree.GetAttributeValue( "e" ).AsFloat64() );

            // An integer beyond Int64 becomes a double, with the precision of a double: 12345678901234567890. (Before the
            // local patch of json-parser that resets its fraction flag per number, a number with a fraction earlier in
            // the document, "f" here, made it ten times too small; see dependencies/json-parser/LOCAL_PATCHES.md)
            ASSERT_TRUE( GUCEF_DATATYPE_FLOAT64 == AttributeType( tree, "big" ) );
            ASSERT_TRUE( tree.GetAttributeValue( "big" ).AsFloat64() > 1.2345678901e19 );
            ASSERT_TRUE( tree.GetAttributeValue( "big" ).AsFloat64() < 1.2345678902e19 );

            ASSERT_TRUE( GUCEF_DATATYPE_BOOLEAN_INT32 == AttributeType( tree, "t" ) );
            ASSERT_TRUE( tree.GetAttributeValue( "t" ).AsBool() );
            ASSERT_TRUE( GUCEF_DATATYPE_BOOLEAN_INT32 == AttributeType( tree, "fl" ) );
            ASSERT_FALSE( tree.GetAttributeValue( "fl" ).AsBool( true ) );

            ASSERT_TRUE( GUCEF_DATATYPE_NIL == AttributeType( tree, "n" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_UTF8_STRING == AttributeType( tree, "s" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON scalars: array elements are child nodes holding the value" )
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( "{ \"a\": [ 1, 2.5, true, null, \"s\" ] }", tree ) );
            const CORE::CDataNode* a = DirectChild( tree, "a" );
            ASSERT_TRUE( GUCEF_NULL != a );
            if ( GUCEF_NULL != a )
            {
                ASSERT_TRUE( GUCEF_DATATYPE_ARRAY == a->GetNodeType() );
                ASSERT_TRUE( 5 == a->GetNrOfDirectChildNodes() );
                ASSERT_TRUE( 0 == a->GetAttCount() );
                static const UInt8 types[] = { GUCEF_DATATYPE_INT64, GUCEF_DATATYPE_FLOAT64, GUCEF_DATATYPE_BOOLEAN_INT32, GUCEF_DATATYPE_NIL, GUCEF_DATATYPE_UTF8_STRING };
                UInt32 index = 0;
                CORE::CDataNode::const_iterator i = a->ConstBegin();
                while ( i != a->ConstEnd() && index < 5 )
                {
                    const CORE::CDataNode& element = *(*i);
                    ASSERT_TRUE( element.HasValue() );
                    ASSERT_TRUE( types[ index ] == element.GetValue().GetTypeId() );
                    ASSERT_TRUE( 0 == element.GetAttCount() );
                    ASSERT_FALSE( element.HasChildren() );
                    ASSERT_TRUE( element.GetName().IsNULLOrEmpty() );
                    ++index;
                    ++i;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON scalars: the types survive a round trip" )
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( "{ \"i\": 5, \"f\": 2.5, \"t\": true, \"fl\": false, \"n\": null, \"s\": \"x\", \"a\": [ 1, 2.5, false, null ] }", tree ) );
            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64 == AttributeType( again, "i" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_FLOAT64 == AttributeType( again, "f" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_BOOLEAN_INT32 == AttributeType( again, "t" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_BOOLEAN_INT32 == AttributeType( again, "fl" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_NIL == AttributeType( again, "n" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_UTF8_STRING == AttributeType( again, "s" ) );
            const CORE::CDataNode* a = DirectChild( again, "a" );
            ASSERT_TRUE( GUCEF_NULL != a && GUCEF_DATATYPE_ARRAY == a->GetNodeType() && 4 == a->GetNrOfDirectChildNodes() );
            ASSERT_TRUE( DeepEqual( tree, again ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformStructureTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON structure: an empty object and an empty array stay apart, also in a round trip" )
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( "{ \"o\": {}, \"a\": [], \"k\": 1 }", tree ) );
            const CORE::CDataNode* o = DirectChild( tree, "o" );
            const CORE::CDataNode* a = DirectChild( tree, "a" );
            ASSERT_TRUE( GUCEF_NULL != o && GUCEF_DATATYPE_OBJECT == o->GetNodeType() && 0 == o->GetAttCount() && !o->HasChildren() && !o->HasValue() );
            ASSERT_TRUE( GUCEF_NULL != a && GUCEF_DATATYPE_ARRAY == a->GetNodeType() && !a->HasChildren() );

            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            o = DirectChild( again, "o" );
            a = DirectChild( again, "a" );
            ASSERT_TRUE( GUCEF_NULL != o && GUCEF_DATATYPE_OBJECT == o->GetNodeType() && 0 == o->GetAttCount() && !o->HasChildren() );
            ASSERT_TRUE( GUCEF_NULL != a && GUCEF_DATATYPE_ARRAY == a->GetNodeType() && !a->HasChildren() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON structure: member names with '/' and '.' are kept as they are" )
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( "{ \"io.modelcontextprotocol/protocolVersion\": \"2026-07-28\","
                                      "  \"io.modelcontextprotocol/clientCapabilities\": { \"elicitation/form\": true, \"a.b\": {} },"
                                      "  \"x/y.z\": [ 1 ] }", tree ) );
            ASSERT_TRUE( GUCEF_DATATYPE_UTF8_STRING == AttributeType( tree, "io.modelcontextprotocol/protocolVersion" ) );
            const CORE::CDataNode* capabilities = DirectChild( tree, "io.modelcontextprotocol/clientCapabilities" );
            ASSERT_TRUE( GUCEF_NULL != capabilities );
            if ( GUCEF_NULL != capabilities )
            {
                ASSERT_TRUE( GUCEF_DATATYPE_BOOLEAN_INT32 == AttributeType( *capabilities, "elicitation/form" ) );
                ASSERT_TRUE( GUCEF_NULL != DirectChild( *capabilities, "a.b" ) );
            }
            ASSERT_TRUE( GUCEF_NULL != DirectChild( tree, "x/y.z" ) );

            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            ASSERT_TRUE( DeepEqual( tree, again ) );
            ASSERT_TRUE( "2026-07-28" == again.GetAttributeValue( "io.modelcontextprotocol/protocolVersion" ).AsString() );
            capabilities = DirectChild( again, "io.modelcontextprotocol/clientCapabilities" );
            ASSERT_TRUE( GUCEF_NULL != capabilities && GUCEF_NULL != capabilities->GetAttribute( "elicitation/form" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON structure: arrays of objects survive a round trip" )
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( "{ \"list\": [ { \"a\": 1 }, { \"a\": 2, \"b\": [ true, null, { \"c\": \"d\" } ] }, [], {} ] }", tree ) );
            const CORE::CDataNode* list = DirectChild( tree, "list" );
            ASSERT_TRUE( GUCEF_NULL != list && 4 == list->GetNrOfDirectChildNodes() );

            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            ASSERT_TRUE( DeepEqual( tree, again ) );
            list = DirectChild( again, "list" );
            ASSERT_TRUE( GUCEF_NULL != list && 4 == list->GetNrOfDirectChildNodes() );
            if ( GUCEF_NULL != list && 4 == list->GetNrOfDirectChildNodes() )
            {
                const CORE::CDataNode* first = list->GetChildAtIndex( 0 );
                const CORE::CDataNode* second = list->GetChildAtIndex( 1 );
                const CORE::CDataNode* third = list->GetChildAtIndex( 2 );
                const CORE::CDataNode* fourth = list->GetChildAtIndex( 3 );
                // an object with a single member stays an object
                ASSERT_TRUE( GUCEF_DATATYPE_OBJECT == first->GetNodeType() && 1 == first->GetAttCount() && 1 == first->GetAttributeValue( "a" ).AsInt32() );
                ASSERT_TRUE( GUCEF_DATATYPE_OBJECT == second->GetNodeType() && 2 == second->GetAttributeValue( "a" ).AsInt32() );
                const CORE::CDataNode* b = DirectChild( *second, "b" );
                ASSERT_TRUE( GUCEF_NULL != b && GUCEF_DATATYPE_ARRAY == b->GetNodeType() && 3 == b->GetNrOfDirectChildNodes() );
                ASSERT_TRUE( GUCEF_DATATYPE_ARRAY == third->GetNodeType() && !third->HasChildren() );
                ASSERT_TRUE( GUCEF_DATATYPE_OBJECT == fourth->GetNodeType() && !fourth->HasChildren() && 0 == fourth->GetAttCount() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON structure: an object with one scalar member stays an object in a round trip" )
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( "{ \"params\": { \"url\": \"http://example.org/\" }, \"empty\": { \"inner\": {} } }", tree ) );
            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            const CORE::CDataNode* params = DirectChild( again, "params" );
            ASSERT_TRUE( GUCEF_NULL != params && GUCEF_DATATYPE_OBJECT == params->GetNodeType() &&
                         "http://example.org/" == params->GetAttributeValue( "url" ).AsString() );
            const CORE::CDataNode* empty = DirectChild( again, "empty" );
            ASSERT_TRUE( GUCEF_NULL != empty && GUCEF_NULL != DirectChild( *empty, "inner" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON structure: deep nesting of arrays and objects is read and written" )
        try
        {
            const UInt32 depth = 100;
            CORE::CString text;
            for ( UInt32 i=0; i<depth; ++i )
                text += ( 0 == i % 2 ) ? "{\"n\":[" : "{\"m\":";
            text += "1";
            for ( UInt32 i=depth; i>0; --i )
                text += ( 0 == ( i-1 ) % 2 ) ? "]}" : "}";

            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( text.C_String(), tree ) );
            UInt32 levels = 0;
            const CORE::CDataNode* node = &tree;
            while ( GUCEF_NULL != node && node->HasChildren() )
            {
                node = node->GetFirstChild();
                ++levels;
            }
            // every object level but the innermost has one child; arrays add a level of their own
            ASSERT_TRUE( levels >= depth );

            CORE::CDataNode again;
            CORE::CString written;
            ASSERT_TRUE( RoundTrip( tree, again, written ) );
            ASSERT_TRUE( DeepEqual( tree, again ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON structure: a document that is a bare scalar gives an empty tree" )
        try
        {
            // the codec only builds nodes for objects and arrays: a scalar at the top has nothing to go into
            CORE::CDataNode tree;
            tree.SetAttribute( "stale", "yes" );
            ParseJsonTestText( "5", tree );
            ASSERT_TRUE( 0 == tree.GetAttCount() );
            ASSERT_FALSE( tree.HasChildren() );
            ASSERT_FALSE( tree.HasValue() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/** Parses exactly the bytes given, from a buffer of that size: a read past them is a read past the buffer (ASan) */
static bool
ParseExactBytes( const char* bytes, UInt32 length, CORE::CDataNode& tree )
{GUCEF_TRACE;

    TCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
        return false;
    tree.Clear();
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( length, bytes );
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &tree, &access );
}

/*-------------------------------------------------------------------------*/

static void
PerformParserBoundsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON parser: every truncation of true, false and null is refused without reading past the text" )
        try
        {
            const char* literals[] = { "true", "false", "null" };
            const char* contexts[] = { "[", "{\"a\":", "[1,", "{\"a\":[" };
            const char* closers[] = { "]", "}", "]", "]}" };
            for ( UInt32 l=0; l<3; ++l )
            {
                const UInt32 literalLength = (UInt32) strlen( literals[ l ] );
                for ( UInt32 c=0; c<4; ++c )
                {
                    const UInt32 contextLength = (UInt32) strlen( contexts[ c ] );
                    for ( UInt32 cut=1; cut<literalLength; ++cut )
                    {
                        char text[ 32 ];
                        memcpy( text, contexts[ c ], contextLength );
                        memcpy( text + contextLength, literals[ l ], cut );
                        CORE::CDataNode tree;
                        ASSERT_FALSE( ParseExactBytes( text, contextLength + cut, tree ) );
                    }

                    // the whole literal, closed, reads
                    const CORE::CString whole = CORE::CString( contexts[ c ] ) + literals[ l ] + closers[ c ];
                    CORE::CDataNode tree;
                    ASSERT_TRUE( ParseExactBytes( whole.C_String(), whole.Length(), tree ) );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON parser: an integer too large for Int64 after a number with a fraction keeps its magnitude" )
        try
        {
            const char* texts[] = { "[99999999999999999999]", "[1.5, 99999999999999999999]", "[1.5e3, 2.25, 123456789012345678901]" };
            const double expected[] = { 1e20, 1e20, 123456789012345678901.0 };
            for ( UInt32 t=0; t<3; ++t )
            {
                CORE::CDataNode tree;
                ASSERT_TRUE( ParseExactBytes( texts[ t ], (UInt32) strlen( texts[ t ] ), tree ) );
                const CORE::CDataNode* last = GUCEF_NULL;
                CORE::CDataNode::const_iterator i = tree.ConstBegin();
                while ( i != tree.ConstEnd() )
                {
                    last = (*i);
                    ++i;
                }
                ASSERT_TRUE( GUCEF_NULL != last );
                if ( GUCEF_NULL != last )
                {
                    const double value = last->GetValue().AsFloat64();
                    ASSERT_TRUE( value > expected[ t ] * 0.999999 && value < expected[ t ] * 1.000001 );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformEscapeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON escapes: line ends and tabs are escaped, no raw line end is written" )
        try
        {
            CORE::CDataNode tree;
            CORE::CString nasty = "line1\nline2\r\n\ttab \"quote\" back\\slash \x01 end";
            tree.SetAttribute( "text", nasty );
            CORE::CDataNode* list = tree.AddChild( "list", GUCEF_DATATYPE_ARRAY );
            list->AddValueAsChild( CORE::CString( "a\nb" ) );

            CORE::CString text;
            ASSERT_TRUE( WriteJsonTestText( tree, text ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "JsonDataNodeFidelity: escapes written as: " << text );
            ASSERT_FALSE( HasByte( text, '\n' ) );
            ASSERT_FALSE( HasByte( text, '\r' ) );
            ASSERT_FALSE( HasByte( text, '\t' ) );
            // json-builder escapes the named control characters only: U+0001 goes out as a raw byte, which is not valid
            // JSON. JSON that leaves a process goes through a writer that escapes every control character
            ASSERT_TRUE( HasByte( text, '\x01' ) );

            CORE::CDataNode again;
            ASSERT_TRUE( ParseJsonTestText( text, again ) );
            ASSERT_TRUE( nasty == again.GetAttributeValue( "text" ).AsString() );
            const CORE::CDataNode* againList = DirectChild( again, "list" );
            ASSERT_TRUE( GUCEF_NULL != againList && 1 == againList->GetNrOfDirectChildNodes() &&
                         "a\nb" == againList->GetFirstChild()->GetValue().AsString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON escapes: \\u escapes and UTF-8 text are read as UTF-8" )
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( "{ \"e\": \"caf\\u00e9\", \"raw\": \"caf\xc3\xa9\", \"pile\": \"\\ud83d\\udca9\" }", tree ) );
            ASSERT_TRUE( tree.GetAttributeValue( "e" ).AsString() == tree.GetAttributeValue( "raw" ).AsString() );
            ASSERT_TRUE( 4 == tree.GetAttributeValue( "pile" ).ByteSize( false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}


/*-------------------------------------------------------------------------*/

/** The text without spaces, so that a search does not depend on the whitespace the serializer puts around ':' and ',' */
static CORE::CString
WithoutSpaces( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString result;
    const char* data = text.C_String();
    const UInt32 length = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<length; ++i )
        if ( ' ' != data[ i ] )
            result += CORE::CString( data + i, 1 );
    return result;
}

/*-------------------------------------------------------------------------*/

static bool
TextHas( const CORE::CString& text, const char* needle )
{GUCEF_TRACE;

    return GUCEF_NULL != strstr( WithoutSpaces( text ).C_String(), needle );
}

/*-------------------------------------------------------------------------*/

/** A member as written, up to the end of its value: followed by ',' or, as the last member, by '}' (members are written in key order) */
static bool
HasMember( const CORE::CString& text, const char* member )
{GUCEF_TRACE;

    return TextHas( text, ( CORE::CString( member ) + "," ).C_String() ) || TextHas( text, ( CORE::CString( member ) + "}" ).C_String() );
}

/*-------------------------------------------------------------------------*/

static bool
SameBits( Float64 a, Float64 b )
{GUCEF_TRACE;

    return 0 == memcmp( &a, &b, sizeof( Float64 ) );
}

/*-------------------------------------------------------------------------*/

/** xorshift64: a fixed seed gives the same values on every platform */
static UInt64
NextRandom( UInt64& state )
{GUCEF_TRACE;

    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

/*-------------------------------------------------------------------------*/

static Float64
DoubleFromBits( UInt64 bits )
{GUCEF_TRACE;

    Float64 value;
    memcpy( &value, &bits, sizeof value );
    return value;
}

/*-------------------------------------------------------------------------*/

static bool
IsFiniteDouble( Float64 value )
{GUCEF_TRACE;

    return value == value && value - value == 0.0;
}

/*-------------------------------------------------------------------------*/

static void
PerformLosslessNumberTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON numbers: 10000 seeded doubles are written and read back bit-identical" )
        try
        {
            static const Float64 specials[] = { 5e-324, 2.2250738585072014e-308, 1.7976931348623157e308, -1.7976931348623157e308, 0.0, -0.0,
                                                0.1, 1.0 / 3.0, 1e22, -1e22, 1.0, -1.0, 0.30000000000000004, 123456789.123456789, 4.35, 1e-7,
                                                0.1234567, 1234567.0, 9007199254740993.0, 1e21, 1e23, 2.5e-5 };
            const UInt32 specialCount = sizeof( specials ) / sizeof( Float64 );
            const UInt32 count = 10000;
            GUCEF::vector< Float64 > values;
            UInt64 state = 0x9E3779B97F4A7C15ULL;
            for ( UInt32 i=0; i<specialCount; ++i )
                values.push_back( specials[ i ] );
            while ( values.size() < count )
            {
                const UInt64 bits = NextRandom( state );
                Float64 value = 0.0;
                switch ( values.size() % 4 )
                {
                    case 0:  value = DoubleFromBits( bits ); break;                                                            // any bit pattern
                    case 1:  value = DoubleFromBits( bits & 0x800FFFFFFFFFFFFFULL ); break;                                    // subnormals and zero
                    case 2:  value = (Float64) (Int64) ( NextRandom( state ) % 2000001 ) / 1000.0 - 1000.0; break;           // short decimals
                    default: value = (Float64) (Int64) ( bits % 1000003 ) / (Float64) (Int64) ( 1 + NextRandom( state ) % 99991 ); break;  // ratios
                }
                if ( IsFiniteDouble( value ) )
                    values.push_back( value );
            }

            CORE::CDataNode tree;
            CORE::CDataNode* list = tree.AddChild( "list", GUCEF_DATATYPE_ARRAY );
            for ( UInt32 i=0; i<count; ++i )
                list->AddValueAsChild( CORE::CVariant( (Float64) values[ i ] ) );

            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            const CORE::CDataNode* readList = DirectChild( again, "list" );
            ASSERT_TRUE( GUCEF_NULL != readList && count == readList->GetNrOfDirectChildNodes() );
            if ( GUCEF_NULL != readList && count == readList->GetNrOfDirectChildNodes() )
            {
                UInt32 mismatches = 0;
                for ( UInt32 i=0; i<count; ++i )
                {
                    const CORE::CVariant& v = readList->GetChildAtIndex( i )->GetValue();
                    // a double without a fraction or exponent that is whole is written with a ".0" and so stays a double
                    if ( GUCEF_DATATYPE_FLOAT64 != v.GetTypeId() || !SameBits( values[ i ], v.AsFloat64() ) )
                    {
                        if ( mismatches < 5 )
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonLossless: value " << i << " type " << (int) v.GetTypeId() << " was not read back bit-identical" );
                        ++mismatches;
                    }
                }
                ASSERT_TRUE( 0 == mismatches );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON numbers: Float32 values are stored as the double they widen to and read back exactly" )
        try
        {
            GUCEF::vector< Float32 > values;
            values.push_back( 0.1f );
            values.push_back( 3.4028235e38f );
            values.push_back( 1.17549435e-38f );
            values.push_back( 1e-45f );
            values.push_back( -0.0f );
            values.push_back( 16777217.0f );
            UInt64 state = 0xD1B54A32D192ED03ULL;
            while ( values.size() < 2000 )
            {
                const UInt32 bits = (UInt32) ( NextRandom( state ) >> 16 );
                Float32 value;
                memcpy( &value, &bits, sizeof value );
                if ( IsFiniteDouble( (Float64) value ) )
                    values.push_back( value );
            }
            CORE::CDataNode tree;
            CORE::CDataNode* list = tree.AddChild( "list", GUCEF_DATATYPE_ARRAY );
            for ( UInt32 i=0; i<values.size(); ++i )
                list->AddValueAsChild( CORE::CVariant( (Float32) values[ i ] ) );
            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            const CORE::CDataNode* readList = DirectChild( again, "list" );
            ASSERT_TRUE( GUCEF_NULL != readList && values.size() == readList->GetNrOfDirectChildNodes() );
            if ( GUCEF_NULL != readList && values.size() == readList->GetNrOfDirectChildNodes() )
            {
                UInt32 mismatches = 0;
                for ( UInt32 i=0; i<values.size(); ++i )
                {
                    const CORE::CVariant& v = readList->GetChildAtIndex( i )->GetValue();
                    const Float32 narrowed = (Float32) v.AsFloat64();
                    if ( GUCEF_DATATYPE_FLOAT64 != v.GetTypeId() || !SameBits( (Float64) values[ i ], v.AsFloat64() ) || 0 != memcmp( &narrowed, &values[ i ], sizeof narrowed ) )
                        ++mismatches;
                }
                ASSERT_TRUE( 0 == mismatches );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON numbers: doubles are written as the shortest text that reads back, whole ones keep a .0" )
        try
        {
            CORE::CDataNode tree;
            tree.SetAttribute( "a", CORE::CVariant( (Float64) 0.1 ) );
            tree.SetAttribute( "b", CORE::CVariant( (Float64) 1.0 ) );
            tree.SetAttribute( "c", CORE::CVariant( (Float64) 0.1234567 ) );
            tree.SetAttribute( "d", CORE::CVariant( (Float64) 1234567.0 ) );
            tree.SetAttribute( "e", CORE::CVariant( (Float64) -2.5 ) );
            CORE::CString text;
            ASSERT_TRUE( WriteJsonTestText( tree, text ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "JsonLossless: written: " << text );
            ASSERT_TRUE( HasMember( text, "\"a\":0.1" ) );
            ASSERT_TRUE( HasMember( text, "\"b\":1.0" ) );
            ASSERT_TRUE( HasMember( text, "\"c\":0.1234567" ) );
            ASSERT_TRUE( HasMember( text, "\"d\":1234567.0" ) );
            ASSERT_TRUE( HasMember( text, "\"e\":-2.5" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON numbers: a double written by another producer reads as the nearest double" )
        try
        {
            CORE::CDataNode tree;
            ASSERT_TRUE( ParseLogged( "{ \"a\": 0.30000000000000004, \"b\": 5e-324, \"c\": 1.7976931348623157e308, \"d\": -0.0, \"e\": 4.35, \"f\": 1E-7, \"g\": 123456789012345678.5 }", tree ) );
            ASSERT_TRUE( SameBits( 0.30000000000000004, tree.GetAttributeValue( "a" ).AsFloat64() ) );
            ASSERT_TRUE( SameBits( 5e-324, tree.GetAttributeValue( "b" ).AsFloat64() ) );
            ASSERT_TRUE( SameBits( 1.7976931348623157e308, tree.GetAttributeValue( "c" ).AsFloat64() ) );
            ASSERT_TRUE( SameBits( -0.0, tree.GetAttributeValue( "d" ).AsFloat64() ) );
            ASSERT_TRUE( SameBits( 4.35, tree.GetAttributeValue( "e" ).AsFloat64() ) );
            ASSERT_TRUE( SameBits( 1E-7, tree.GetAttributeValue( "f" ).AsFloat64() ) );
            ASSERT_TRUE( SameBits( 123456789012345678.5, tree.GetAttributeValue( "g" ).AsFloat64() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON numbers: 64 bit integers inside +-(2^53-1) stay numbers, outside they travel as strings" )
        try
        {
            CORE::CDataNode tree;
            tree.SetAttribute( "i32", (Int32) -5 );
            tree.SetAttribute( "safe", (Int64) 9007199254740991LL );
            tree.SetAttribute( "safeNeg", (Int64) -9007199254740991LL );
            tree.SetAttribute( "usafe", (UInt64) 9007199254740991ULL );
            tree.SetAttribute( "big", (Int64) 9007199254740993LL );
            tree.SetAttribute( "bigNeg", (Int64) -9007199254740993LL );
            tree.SetAttribute( "pow53", (Int64) 9007199254740992LL );
            tree.SetAttribute( "i64max", (Int64) GUCEF_INT64MAX );
            tree.SetAttribute( "i64min", (Int64) ( -GUCEF_INT64MAX - 1 ) );
            tree.SetAttribute( "ubig", (UInt64) 18446744073709551615ULL );
            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            ASSERT_TRUE( HasMember( text, "\"i32\":-5" ) );
            ASSERT_TRUE( HasMember( text, "\"safe\":9007199254740991" ) );
            ASSERT_TRUE( HasMember( text, "\"safeNeg\":-9007199254740991" ) );
            ASSERT_TRUE( HasMember( text, "\"usafe\":9007199254740991" ) );
            ASSERT_TRUE( HasMember( text, "\"big\":\"9007199254740993\"" ) );
            ASSERT_TRUE( HasMember( text, "\"bigNeg\":\"-9007199254740993\"" ) );
            ASSERT_TRUE( HasMember( text, "\"pow53\":\"9007199254740992\"" ) );
            ASSERT_TRUE( HasMember( text, "\"i64max\":\"9223372036854775807\"" ) );
            ASSERT_TRUE( HasMember( text, "\"i64min\":\"-9223372036854775808\"" ) );
            ASSERT_TRUE( HasMember( text, "\"ubig\":\"18446744073709551615\"" ) );

            ASSERT_TRUE( GUCEF_DATATYPE_INT64 == AttributeType( again, "safe" ) );
            ASSERT_TRUE( 9007199254740991LL == again.GetAttributeValue( "safe" ).AsInt64() );
            ASSERT_TRUE( -9007199254740991LL == again.GetAttributeValue( "safeNeg" ).AsInt64() );
            ASSERT_TRUE( 9007199254740991LL == again.GetAttributeValue( "usafe" ).AsInt64() );
            // the reader leaves the strings as strings, the conversions of the variant read them exactly
            ASSERT_TRUE( GUCEF_DATATYPE_UTF8_STRING == AttributeType( again, "big" ) );
            ASSERT_TRUE( 9007199254740993LL == again.GetAttributeValue( "big" ).AsInt64() );
            ASSERT_TRUE( -9007199254740993LL == again.GetAttributeValue( "bigNeg" ).AsInt64() );
            ASSERT_TRUE( 9007199254740992LL == again.GetAttributeValue( "pow53" ).AsInt64() );
            ASSERT_TRUE( GUCEF_INT64MAX == again.GetAttributeValue( "i64max" ).AsInt64() );
            ASSERT_TRUE( ( -GUCEF_INT64MAX - 1 ) == again.GetAttributeValue( "i64min" ).AsInt64( 0 ) );
            ASSERT_TRUE( 18446744073709551615ULL == again.GetAttributeValue( "ubig" ).AsUInt64() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON numbers: decimals are written as strings and read back exactly, a number reads as well" )
        try
        {
            const CORE::CDecimal original = CORE::CDecimal::Parse( "12345678901234.56789" );
            CORE::CDataNode tree;
            tree.SetAttribute( "dec", CORE::CVariant( original ) );
            tree.SetAttribute( "neg", CORE::CVariant( CORE::CDecimal::Parse( "-0.000000000000000001" ) ) );
            tree.SetAttribute( "whole", CORE::CVariant( CORE::CDecimal::Parse( "7" ) ) );
            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            ASSERT_TRUE( HasMember( text, "\"dec\":\"12345678901234.56789\"" ) );
            ASSERT_TRUE( HasMember( text, "\"neg\":\"-0.000000000000000001\"" ) );
            ASSERT_TRUE( HasMember( text, "\"whole\":\"7\"" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_UTF8_STRING == AttributeType( again, "dec" ) );
            CORE::CDecimal read;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == again.GetAttributeValue( "dec" ).GetDecimal( read ) );
            ASSERT_TRUE( original == read && original.ToString() == read.ToString() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == again.GetAttributeValue( "neg" ).GetDecimal( read ) );
            ASSERT_TRUE( CORE::CDecimal::Parse( "-0.000000000000000001" ).ToString() == read.ToString() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == again.GetAttributeValue( "whole" ).GetDecimal( read ) );
            ASSERT_TRUE( CORE::CDecimal::Parse( "7" ).ToString() == read.ToString() );

            // the number form reads too: a document from another producer
            CORE::CDataNode other;
            ASSERT_TRUE( ParseLogged( "{ \"dec\": 12.5 }", other ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == other.GetAttributeValue( "dec" ).GetDecimal( read ) );
            ASSERT_TRUE( CORE::CDecimal::Parse( "12.5" ) == read );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON numbers: fractions are written as strings and read back exactly" )
        try
        {
            CORE::CDataNode tree;
            tree.SetAttribute( "f32", CORE::CVariant( CORE::TInt32Fraction( -1, 3 ) ) );
            tree.SetAttribute( "fu32", CORE::CVariant( CORE::TUInt32Fraction( 4000000000U, 3 ) ) );
            tree.SetAttribute( "f64", CORE::CVariant( CORE::TInt64Fraction( 9007199254740993LL, 7 ) ) );
            tree.SetAttribute( "fu64", CORE::CVariant( CORE::TUInt64Fraction( 18446744073709551615ULL, 7 ) ) );
            tree.SetAttribute( "f6432", CORE::CVariant( CORE::TInt64Int32Fraction( -9007199254740993LL, 5 ) ) );
            tree.SetAttribute( "fu6432", CORE::CVariant( CORE::TUInt64UInt32Fraction( 9007199254740993ULL, 5 ) ) );
            CORE::CDataNode again;
            CORE::CString text;
            ASSERT_TRUE( RoundTrip( tree, again, text ) );
            ASSERT_TRUE( HasMember( text, "\"f32\":\"-1/3\"" ) );
            ASSERT_TRUE( HasMember( text, "\"fu32\":\"4000000000/3\"" ) );
            ASSERT_TRUE( HasMember( text, "\"f64\":\"9007199254740993/7\"" ) );
            ASSERT_TRUE( HasMember( text, "\"fu64\":\"18446744073709551615/7\"" ) );
            ASSERT_TRUE( HasMember( text, "\"f6432\":\"-9007199254740993/5\"" ) );
            ASSERT_TRUE( HasMember( text, "\"fu6432\":\"9007199254740993/5\"" ) );

            const CORE::TInt32Fraction a = again.GetAttributeValue( "f32" ).AsInt32Fraction();
            ASSERT_TRUE( -1 == a.GetNumerator() && 3 == a.GetDenominator() );
            const CORE::TUInt32Fraction b = again.GetAttributeValue( "fu32" ).AsUInt32Fraction();
            ASSERT_TRUE( 4000000000U == b.GetNumerator() && 3 == b.GetDenominator() );
            const CORE::TInt64Fraction c = again.GetAttributeValue( "f64" ).AsInt64Fraction();
            ASSERT_TRUE( 9007199254740993LL == c.GetNumerator() && 7 == c.GetDenominator() );
            const CORE::TUInt64Fraction d = again.GetAttributeValue( "fu64" ).AsUInt64Fraction();
            ASSERT_TRUE( 18446744073709551615ULL == d.GetNumerator() && 7 == d.GetDenominator() );
            const CORE::TInt64Int32Fraction e = again.GetAttributeValue( "f6432" ).AsInt64Int32Fraction();
            ASSERT_TRUE( -9007199254740993LL == e.GetNumerator() && 5 == e.GetDenominator() );
            const CORE::TUInt64UInt32Fraction f = again.GetAttributeValue( "fu6432" ).AsUInt64UInt32Fraction();
            ASSERT_TRUE( 9007199254740993ULL == f.GetNumerator() && 5 == f.GetDenominator() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
    GUCEF_TESTFW_TESTCASE( "JSON timestamps: nanosecond, second and millisecond timestamps are written as their tick count text and read back exactly" )
        try
        {
            const UInt8 ids[ 3 ] = { GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH };
            // beyond 2^53 for the nanosecond case, before the epoch, and an ordinary one
            const Int64 values[ 3 ] = { 1700000000123456789LL, -86400123LL, 1700000000123LL };
            const char* texts[ 3 ] = { "1700000000123456789", "-86400123", "1700000000123" };

            for ( int i = 0; i < 3; ++i )
            {
                for ( int swapped = 0; swapped < 2; ++swapped )
                {
                    CORE::CVariant original( &values[ i ], (UInt32) sizeof( Int64 ), ids[ i ] );
                    if ( 0 != swapped )
                        original.ToBigEndian();

                    CORE::CDataNode tree;
                    tree.SetAttribute( "t", original );
                    CORE::CDataNode again;
                    CORE::CString text;
                    ASSERT_TRUE( RoundTrip( tree, again, text ) );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "JsonTimestamp: type id " << (int) original.GetTypeId() << " written: " << text );
                    ASSERT_TRUE( HasMember( text, ( CORE::CString( "\"t\":\"" ) + texts[ i ] + "\"" ).C_String() ) );

                    // the reader leaves the text as a string, the conversions of the variant read it exactly and keep the type id
                    ASSERT_TRUE( GUCEF_DATATYPE_UTF8_STRING == AttributeType( again, "t" ) );
                    const CORE::CVariant readBack = again.GetAttributeValue( "t" );
                    ASSERT_TRUE( values[ i ] == readBack.AsInt64() );
                    CORE::CVariant typed( original.GetTypeId(), readBack.AsString(), CORE::CVariant() );
                    ASSERT_TRUE( typed.GetTypeId() == original.GetTypeId() );
                    ASSERT_TRUE( values[ i ] == typed.AsInt64() );
                    ASSERT_TRUE( typed.AsString() == original.AsString() );
                    if ( values[ i ] >= 0 )
                        ASSERT_TRUE( typed.AsTimestamp() == original.AsTimestamp() );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformJsonDataNodeFidelityTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING JSON DataNode fidelity TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "JsonDataNodeFidelity" );

    GUCEF_TESTFW_TESTCASE( "JSON fidelity: the json codec is available" )
        ASSERT_TRUE( JsonTestCodecAvailable() );
    GUCEF_TESTFW_TESTCASE_END;

    if ( !JsonTestCodecAvailable() )
        return;

    PerformScalarTypeTests();
    PerformLosslessNumberTests();
    PerformStructureTests();
    PerformEscapeTests();
    PerformParserBoundsTests();
}

/*-------------------------------------------------------------------------*/
