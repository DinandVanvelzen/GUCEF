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
    PerformStructureTests();
    PerformEscapeTests();
    PerformParserBoundsTests();
}

/*-------------------------------------------------------------------------*/
