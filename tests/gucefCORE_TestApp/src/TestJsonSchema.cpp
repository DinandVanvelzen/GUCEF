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

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CJSONSCHEMAVALIDATOR_H
#include "gucefCORE_CJsonSchemaValidator.h"
#define GUCEF_CORE_CJSONSCHEMAVALIDATOR_H
#endif /* GUCEF_CORE_CJSONSCHEMAVALIDATOR_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

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
#include "TestJsonSchema.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#define CASE_COUNT( cases ) ( sizeof( cases ) / sizeof( cases[ 0 ] ) )

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One test of a schema, in the style of the official JSON Schema test suite.
 *  The texts use ' where JSON has ", so they stay readable; a real " is written as \"
 */
namespace {

struct SCase
{
    const char* description;
    const char* instance;
    bool valid;
};

struct SGroup
{
    const char* description;
    const char* schema;
    const SCase* cases;
    UInt32 caseCount;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The JSON text of a test text: ' becomes " */
static CString
JsonText( const char* text )
{GUCEF_TRACE;

    return CString( text ).ReplaceChar( '\'', '"' );
}

/*-------------------------------------------------------------------------*/

/**
 *  Parses any JSON value into a node: an object or an array is the node itself, a scalar is a node holding the value.
 *  The codec cannot parse a scalar at the top, so the value is parsed as a member of an object first
 */
static bool
ParseValue( const char* text, CDataNode& value )
{GUCEF_TRACE;

    CDataNode wrapper;
    if ( !ParseJsonTestText( "{\"v\":" + JsonText( text ) + "}", wrapper ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JsonSchema test: cannot parse " << text );
        return false;
    }
    value.Clear();
    const CDataNode::TKeyValuePair* attribute = wrapper.GetAttribute( "v" );
    if ( GUCEF_NULL != attribute )
    {
        value.SetValue( attribute->second );
        return true;
    }
    CDataNode::const_iterator i = wrapper.ConstBegin();
    if ( i != wrapper.ConstEnd() && (*i)->GetName() == "v" )
    {
        value = *(*i);
        return true;
    }
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JsonSchema test: no value in " << text );
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
CompileText( const char* schemaText, CJsonSchema& schema, CString& error, UInt32 maxDepth = 32, UInt32 maxSubschemas = 1024 )
{GUCEF_TRACE;

    CDataNode schemaNode;
    if ( !ParseValue( schemaText, schemaNode ) )
    {
        error = "unparsable";
        return false;
    }
    return schema.Compile( schemaNode, error, maxDepth, maxSubschemas );
}

/*-------------------------------------------------------------------------*/

static CString
ErrorsText( const CJsonSchemaValidator::TErrorVector& errors )
{GUCEF_TRACE;

    CString text;
    for ( UInt32 i=0; i<errors.size(); ++i )
        text += "[" + errors[ i ].instancePath + " " + errors[ i ].keyword + ": " + errors[ i ].message + "] ";
    return text;
}

/*-------------------------------------------------------------------------*/

/** Validates the instance text; logs what happened when it is not what was expected */
static bool
ValidatesAs( const CJsonSchema& schema, const char* instanceText, bool expected, const char* description )
{GUCEF_TRACE;

    CDataNode instance;
    if ( !ParseValue( instanceText, instance ) )
        return false;
    CJsonSchemaValidator::TErrorVector errors;
    bool valid = CJsonSchemaValidator::Validate( schema, instance, errors );
    if ( valid != expected )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JsonSchema test: \"" << description << "\": " << instanceText << " is " <<
                         ( valid ? "valid" : "invalid" ) << ", expected " << ( expected ? "valid" : "invalid" ) << " " << ErrorsText( errors ) );
        return false;
    }
    if ( !valid && errors.empty() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JsonSchema test: \"" << description << "\": invalid without an error" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
RunGroups( const SGroup* groups, UInt32 groupCount )
{GUCEF_TRACE;

    for ( UInt32 g=0; g<groupCount; ++g )
    {
        const SGroup& group = groups[ g ];
        CString testName = "JSON Schema: " + CString( group.description );
        GUCEF_TESTFW_TESTCASE( testName.C_String() )
            try
            {
                CJsonSchema schema;
                CString error;
                bool compiled = CompileText( group.schema, schema, error );
                if ( !compiled )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JsonSchema test: \"" << group.description << "\" does not compile: " << error );
                ASSERT_TRUE( compiled );
                if ( compiled )
                {
                    for ( UInt32 c=0; c<group.caseCount; ++c )
                    {
                        const SCase& testCase = group.cases[ c ];
                        ASSERT_TRUE( ValidatesAs( schema, testCase.instance, testCase.valid, testCase.description ) );
                    }
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;
    }
}

/*-------------------------------------------------------------------------*/

/** True when compiling fails with a message that holds the given text */
static bool
RefusedWith( const char* schemaText, const char* expectedText )
{GUCEF_TRACE;

    CJsonSchema schema;
    CString error;
    if ( CompileText( schemaText, schema, error ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JsonSchema test: " << schemaText << " compiled, it should have been refused" );
        return false;
    }
    if ( schema.IsCompiled() || error.HasSubstr( expectedText ) < 0 )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JsonSchema test: " << schemaText << " was refused with \"" << error << "\", expected \"" << expectedText << "\"" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
Compiles( const char* schemaText )
{GUCEF_TRACE;

    CJsonSchema schema;
    CString error;
    if ( CompileText( schemaText, schema, error ) && schema.IsCompiled() )
        return true;
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JsonSchema test: " << schemaText << " was refused: " << error );
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      KEYWORD CASES                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

static const SCase g_typeInteger[] = {
    { "an integer is an integer", "1", true },
    { "a negative integer is an integer", "-7", true },
    { "a float with zero fractional part is an integer", "1.0", true },
    { "a float is not an integer", "1.1", false },
    { "a string is not an integer", "'foo'", false },
    { "a string is still not an integer, even if it looks like one", "'1'", false },
    { "an object is not an integer", "{}", false },
    { "an array is not an integer", "[]", false },
    { "a boolean is not an integer", "true", false },
    { "null is not an integer", "null", false }
};
static const SCase g_typeNumber[] = {
    { "an integer is a number", "1", true },
    { "a float with zero fractional part is a number", "1.0", true },
    { "a float is a number", "1.1", true },
    { "a string is not a number", "'foo'", false },
    { "a string is still not a number, even if it looks like one", "'1'", false },
    { "an object is not a number", "{}", false },
    { "a boolean is not a number", "true", false },
    { "null is not a number", "null", false }
};
static const SCase g_typeString[] = {
    { "1 is not a string", "1", false },
    { "a float is not a string", "1.1", false },
    { "a string is a string", "'foo'", true },
    { "a string is still a string, even if it looks like a number", "'1'", true },
    { "an empty string is still a string", "''", true },
    { "an object is not a string", "{}", false },
    { "an array is not a string", "[]", false },
    { "a boolean is not a string", "true", false },
    { "null is not a string", "null", false }
};
static const SCase g_typeObject[] = {
    { "an integer is not an object", "1", false },
    { "a string is not an object", "'foo'", false },
    { "an object is an object", "{}", true },
    { "an object with members is an object", "{'a':1,'b':{}}", true },
    { "an array is not an object", "[]", false },
    { "null is not an object", "null", false }
};
static const SCase g_typeArray[] = {
    { "an integer is not an array", "1", false },
    { "an object is not an array", "{}", false },
    { "an array is an array", "[]", true },
    { "an array of values is an array", "[1,'a',null]", true },
    { "a boolean is not an array", "true", false }
};
static const SCase g_typeBoolean[] = {
    { "an integer is not a boolean", "1", false },
    { "zero is not a boolean", "0", false },
    { "an empty string is not a boolean", "''", false },
    { "true is a boolean", "true", true },
    { "false is a boolean", "false", true },
    { "null is not a boolean", "null", false }
};
static const SCase g_typeNull[] = {
    { "an integer is not null", "1", false },
    { "zero is not null", "0", false },
    { "an empty string is not null", "''", false },
    { "an object is not null", "{}", false },
    { "false is not null", "false", false },
    { "null is null", "null", true }
};
static const SCase g_typeMultiple[] = {
    { "an integer is valid", "1", true },
    { "a string is valid", "'foo'", true },
    { "a float is invalid", "1.1", false },
    { "an object is invalid", "{}", false },
    { "null is invalid", "null", false }
};
static const SCase g_typeArrayOrObject[] = {
    { "an array is valid", "[1,2,3]", true },
    { "an object is valid", "{'foo':123}", true },
    { "a number is invalid", "123", false },
    { "a string is invalid", "'foo'", false },
    { "null is invalid", "null", false }
};

static const SCase g_enumSimple[] = {
    { "one of the enum is valid", "1", true },
    { "something else is invalid", "4", false },
    { "the same number as a float is valid", "1.0", true }
};
static const SCase g_enumHeterogeneous[] = {
    { "one of the enum is valid", "[]", true },
    { "something else is invalid", "null", false },
    { "objects are deep compared", "{'foo':false}", false },
    { "a valid object is valid", "{'foo':12}", true },
    { "an extra member is invalid", "{'foo':12,'boo':42}", false }
};
static const SCase g_enumFalse[] = {
    { "false is valid", "false", true },
    { "integer zero is invalid", "0", false },
    { "float zero is invalid", "0.0", false }
};
static const SCase g_enumZero[] = {
    { "false is invalid", "false", false },
    { "integer zero is valid", "0", true },
    { "float zero is valid", "0.0", true }
};
static const SCase g_enumNested[] = {
    { "[false] is valid", "[false]", true },
    { "[0] is invalid", "[0]", false },
    { "[0.0] is invalid", "[0.0]", false }
};
static const SCase g_enumEscaped[] = {
    { "a member 1 is valid", "'foo\\nbar'", true },
    { "a member 2 is valid", "'foo\\rbar'", true },
    { "another string is invalid", "'abc'", false }
};

static const SCase g_constNumber[] = {
    { "the same value is valid", "2", true },
    { "another value is invalid", "5", false },
    { "another type is invalid", "'a'", false },
    { "the same value as a float is valid", "2.0", true }
};
static const SCase g_constObject[] = {
    { "the same object is valid", "{'foo':'bar','baz':'bax'}", true },
    { "the same object with another member order is valid", "{'baz':'bax','foo':'bar'}", true },
    { "another object is invalid", "{'foo':'bar'}", false },
    { "another type is invalid", "[1,2]", false }
};
static const SCase g_constArray[] = {
    { "the same array is valid", "[{'foo':'bar'}]", true },
    { "an array of another value is invalid", "[2]", false },
    { "an array with an extra item is invalid", "[{'foo':'bar'},'bar']", false },
    { "an object is invalid", "{'foo':'bar'}", false }
};
static const SCase g_constNull[] = {
    { "null is valid", "null", true },
    { "zero is invalid", "0", false },
    { "false is invalid", "false", false }
};
static const SCase g_constFalse[] = {
    { "false is valid", "false", true },
    { "integer zero is invalid", "0", false },
    { "float zero is invalid", "0.0", false }
};
static const SCase g_constFloat[] = {
    { "the same integer is valid", "-2", true },
    { "the positive integer is invalid", "2", false },
    { "the same float is valid", "-2.0", true },
    { "the positive float is invalid", "2.0", false },
    { "a close float is invalid", "-2.00001", false }
};
static const SCase g_constLargeInteger[] = {
    { "the integer is valid", "9007199254740992", true },
    { "the integer minus one is invalid", "9007199254740991", false },
    { "the same value as a float is valid", "9007199254740992.0", true }
};

static const SCase g_properties[] = {
    { "both present and valid is valid", "{'foo':1,'bar':'baz'}", true },
    { "one property invalid is invalid", "{'foo':1,'bar':{}}", false },
    { "both properties invalid is invalid", "{'foo':[],'bar':{}}", false },
    { "properties that are not there are valid", "{'quux':[]}", true },
    { "an array is valid", "[]", true },
    { "a number is valid", "12", true }
};
static const SCase g_propertiesBoolean[] = {
    { "no property present is valid", "{}", true },
    { "only the 'true' property present is valid", "{'foo':1}", true },
    { "only the 'false' property present is invalid", "{'bar':2}", false },
    { "both present is invalid", "{'foo':1,'bar':2}", false }
};
static const SCase g_propertiesEscaped[] = {
    { "the escaped names are matched exactly", "{'foo\\nbar':1,'foo\\\"bar':1,'foo/bar':1,'foo.bar':1,'foo~bar':1}", true },
    { "the escaped names are checked", "{'foo\\nbar':'1','foo\\\"bar':'1','foo/bar':'1','foo.bar':'1','foo~bar':'1'}", false },
    { "a name with a '/' is checked", "{'foo/bar':'1'}", false },
    { "a name with a '.' is checked", "{'foo.bar':'1'}", false }
};
static const SCase g_propertiesNull[] = {
    { "a null value is valid", "{'foo':null}", true },
    { "another value is invalid", "{'foo':0}", false }
};
static const SCase g_propertiesProto[] = {
    { "names that are also names of object machinery are just names", "{'__proto__':12,'toString':{'length':37},'constructor':{'length':37}}", false },
    { "with the right types they are valid", "{'__proto__':'a','toString':{'length':'x'},'constructor':{'length':'y'}}", true }
};
static const SCase g_required[] = {
    { "a present required property is valid", "{'foo':1}", true },
    { "a missing required property is invalid", "{'bar':1}", false },
    { "arrays are ignored", "[]", true },
    { "strings are ignored", "''", true },
    { "other non-objects are ignored", "12", true }
};
static const SCase g_requiredEmpty[] = {
    { "an object without properties is valid", "{}", true }
};
static const SCase g_requiredEscaped[] = {
    { "the names with escapes are present", "{'foo\\nbar':1,'foo\\\"bar':1,'foo\\\\bar':1}", true },
    { "the names with escapes are missing", "{'foo\\nbar':'1','foo\\\"bar':'1'}", false }
};
static const SCase g_requiredNested[] = {
    { "a required member of an object member is checked", "{'a':{'b':1}}", true },
    { "a missing required member of an object member is invalid", "{'a':{'c':1}}", false },
    { "a missing object member is valid", "{}", true }
};
static const SCase g_additionalFalse[] = {
    { "no additional properties is valid", "{'foo':1}", true },
    { "an additional property is invalid", "{'foo':1,'bar':2,'quux':'boom'}", false },
    { "arrays are ignored", "[1,2,3]", true },
    { "strings are ignored", "'foobarbaz'", true },
    { "an additional object member is invalid", "{'foo':1,'quux':{}}", false },
    { "an additional array member is invalid", "{'foo':1,'quux':[]}", false }
};
static const SCase g_additionalSchema[] = {
    { "no additional properties is valid", "{'foo':1}", true },
    { "a valid additional property is valid", "{'foo':1,'bar':2,'quux':true}", true },
    { "an invalid additional property is invalid", "{'foo':1,'bar':2,'quux':12}", false }
};
static const SCase g_additionalAlone[] = {
    { "a valid additional property is valid", "{'foo':true}", true },
    { "an invalid additional property is invalid", "{'foo':1}", false }
};
static const SCase g_additionalNull[] = {
    { "a null value is valid", "{'foo':null}", true }
};
static const SCase g_propertyNames[] = {
    { "all property names valid", "{'f':{},'foo':{}}", true },
    { "some property names invalid", "{'foo':{},'foobar':{}}", false },
    { "an object without properties is valid", "{}", true },
    { "arrays are ignored", "[1,2,3,4]", true },
    { "strings are ignored", "'foobar'", true },
    { "other non-objects are ignored", "12", true }
};
static const SCase g_propertyNamesFalse[] = {
    { "an object with any property is invalid", "{'foo':1}", false },
    { "an empty object is valid", "{}", true }
};
static const SCase g_propertyNamesConst[] = {
    { "the property name matching the const is valid", "{'foo':1}", true },
    { "another property name is invalid", "{'bar':1}", false },
    { "an empty object is valid", "{}", true }
};
static const SCase g_propertyNamesEnum[] = {
    { "a property name in the enum is valid", "{'foo':1}", true },
    { "both property names in the enum are valid", "{'foo':1,'bar':1}", true },
    { "a property name not in the enum is invalid", "{'baz':1}", false }
};
static const SCase g_minProperties[] = {
    { "longer is valid", "{'foo':1,'bar':2}", true },
    { "exact length is valid", "{'foo':1}", true },
    { "too short is invalid", "{}", false },
    { "arrays are ignored", "[]", true },
    { "strings are ignored", "''", true }
};
static const SCase g_maxProperties[] = {
    { "shorter is valid", "{'foo':1}", true },
    { "exact length is valid", "{'foo':1,'bar':2}", true },
    { "too long is invalid", "{'foo':1,'bar':2,'baz':3}", false },
    { "object members count", "{'foo':{},'bar':[],'baz':3}", false },
    { "arrays are ignored", "[1,2,3]", true }
};
static const SCase g_maxPropertiesZero[] = {
    { "no properties is valid", "{}", true },
    { "one property is invalid", "{'foo':1}", false }
};
static const SCase g_dependentRequired[] = {
    { "neither is valid", "{}", true },
    { "a non-dependant is valid", "{'foo':1}", true },
    { "with the dependency is valid", "{'foo':1,'bar':2}", true },
    { "without the dependency is invalid", "{'bar':2}", false },
    { "arrays are ignored", "['bar']", true }
};
static const SCase g_dependentRequiredEmpty[] = {
    { "an empty dependency list is always met", "{'bar':2}", true }
};
static const SCase g_dependentRequiredMultiple[] = {
    { "all dependencies present is valid", "{'foo':1,'bar':2,'quux':3}", true },
    { "one dependency missing is invalid", "{'foo':1,'quux':2}", false },
    { "both dependencies missing is invalid", "{'quux':1}", false }
};

static const SCase g_items[] = {
    { "valid items", "[1,2,3]", true },
    { "a wrong type of item", "[1,'x']", false },
    { "objects are ignored", "{'foo':'bar'}", true },
    { "an empty array is valid", "[]", true }
};
static const SCase g_itemsTrue[] = {
    { "any array is valid", "[1,'foo',true]", true },
    { "an empty array is valid", "[]", true }
};
static const SCase g_itemsFalse[] = {
    { "any non-empty array is invalid", "[1,'foo',true]", false },
    { "an empty array is valid", "[]", true }
};
static const SCase g_itemsNested[] = {
    { "valid nested array", "[[[[1]],[[2],[3]]],[[[4],[5],[6]]]]", true },
    { "nested array with an invalid type", "[[[['1']],[[2],[3]]],[[[4],[5],[6]]]]", false },
    { "not deep enough", "[[[1],[2],[3]],[[4],[5],[6]]]", false }
};
static const SCase g_itemsOfObjects[] = {
    { "valid items", "[{'a':1},{'a':2,'b':'x'}]", true },
    { "an item without the required member", "[{'a':1},{'b':'x'}]", false },
    { "an item with a wrong member type", "[{'a':'1'}]", false }
};
static const SCase g_itemsNull[] = {
    { "null items are valid", "[null]", true }
};
static const SCase g_prefixItems[] = {
    { "correct types", "[1,'foo']", true },
    { "wrong types", "['foo',1]", false },
    { "incomplete array of items", "[1]", true },
    { "array with additional items", "[1,'foo',true]", true },
    { "an empty array", "[]", true },
    { "a JavaScript pseudo-array is valid", "{'0':'invalid','1':'valid','length':2}", true }
};
static const SCase g_prefixItemsWithItemsFalse[] = {
    { "the prefix only is valid", "[1,'foo']", true },
    { "an incomplete prefix is valid", "[1]", true },
    { "an item beyond the prefix is invalid", "[1,'foo',2]", false }
};
static const SCase g_prefixItemsWithItems[] = {
    { "items beyond the prefix are checked", "['x',2,3]", true },
    { "an item beyond the prefix of the wrong type", "['x',2,'3']", false }
};
static const SCase g_minItems[] = {
    { "longer is valid", "[1,2]", true },
    { "exact length is valid", "[1]", true },
    { "too short is invalid", "[]", false },
    { "non-arrays are ignored", "''", true }
};
static const SCase g_maxItems[] = {
    { "shorter is valid", "[1]", true },
    { "exact length is valid", "[1,2]", true },
    { "too long is invalid", "[1,2,3]", false },
    { "non-arrays are ignored", "'foobar'", true }
};
static const SCase g_uniqueItems[] = {
    { "unique array of integers is valid", "[1,2]", true },
    { "non-unique array of integers is invalid", "[1,1]", false },
    { "non-unique array of more than two integers is invalid", "[1,2,1]", false },
    { "numbers are unique if mathematically unequal", "[1.0,1.00,1]", false },
    { "false is not equal to zero", "[0,false]", true },
    { "true is not equal to one", "[1,true]", true },
    { "unique array of strings is valid", "['foo','bar','baz']", true },
    { "non-unique array of strings is invalid", "['foo','bar','foo']", false },
    { "unique array of objects is valid", "[{'foo':'bar'},{'foo':'baz'}]", true },
    { "non-unique array of objects is invalid", "[{'foo':'bar'},{'foo':'bar'}]", false },
    { "property order of array of objects is ignored", "[{'foo':'bar','bar':'foo'},{'bar':'foo','foo':'bar'}]", false },
    { "unique array of nested objects is valid", "[{'foo':{'bar':{'baz':true}}},{'foo':{'bar':{'baz':false}}}]", true },
    { "non-unique array of nested objects is invalid", "[{'foo':{'bar':{'baz':true}}},{'foo':{'bar':{'baz':true}}}]", false },
    { "unique array of arrays is valid", "[['foo'],['bar']]", true },
    { "non-unique array of arrays is invalid", "[['foo'],['foo']]", false },
    { "[false] and [0] are unique", "[[false],[0]]", true },
    { "unique heterogeneous types are valid", "[{},[1],true,null,1,'{}']", true },
    { "non-unique heterogeneous types are invalid", "[{},[1],true,null,{},1]", false },
    { "{'a': false} and {'a': 0} are unique", "[{'a':false},{'a':0}]", true },
    { "a single item is unique", "[1]", true }
};
static const SCase g_uniqueItemsFalse[] = {
    { "non-unique items are valid", "[1,1]", true },
    { "unique items are valid", "[1,2]", true }
};

static const SCase g_minLength[] = {
    { "longer is valid", "'foo'", true },
    { "exact length is valid", "'fo'", true },
    { "too short is invalid", "'f'", false },
    { "non-strings are ignored", "1", true },
    { "one code point is not long enough, whatever its byte count", "'\\ud83d\\udca9'", false },
    { "two code points of several bytes each are long enough", "'\xc3\xa9\xc3\xa9'", true }
};
static const SCase g_maxLength[] = {
    { "shorter is valid", "'f'", true },
    { "exact length is valid", "'fo'", true },
    { "too long is invalid", "'foo'", false },
    { "non-strings are ignored", "100", true },
    { "two code points are short enough, whatever their byte count", "'\\ud83d\\udca9\\ud83d\\udca9'", true }
};
static const SCase g_minimum[] = {
    { "above the minimum is valid", "2.6", true },
    { "the boundary is valid", "1.1", true },
    { "below the minimum is invalid", "0.6", false },
    { "non-numbers are ignored", "'x'", true }
};
static const SCase g_minimumNegative[] = {
    { "negative above the minimum is valid", "-1", true },
    { "positive above the minimum is valid", "0", true },
    { "the boundary is valid", "-2", true },
    { "the boundary as a float is valid", "-2.0", true },
    { "a float below the minimum is invalid", "-2.0001", false },
    { "an integer below the minimum is invalid", "-3", false }
};
static const SCase g_minimumExact[] = {
    { "an integer one below a big minimum is invalid, exactly compared", "9007199254740992", false },
    { "the big minimum itself is valid", "9007199254740993", true }
};
static const SCase g_maximum[] = {
    { "below the maximum is valid", "2.6", true },
    { "the boundary is valid", "3.0", true },
    { "above the maximum is invalid", "3.5", false },
    { "non-numbers are ignored", "'x'", true }
};
static const SCase g_maximumInteger[] = {
    { "the boundary as a float is valid", "300.00", true },
    { "above the maximum is invalid", "300.5", false }
};
static const SCase g_exclusiveMaximum[] = {
    { "below the exclusive maximum is valid", "2.2", true },
    { "the boundary is invalid", "3.0", false },
    { "above the exclusive maximum is invalid", "3.5", false },
    { "non-numbers are ignored", "'x'", true }
};
static const SCase g_exclusiveMinimum[] = {
    { "above the exclusive minimum is valid", "1.2", true },
    { "the boundary is invalid", "1.1", false },
    { "below the exclusive minimum is invalid", "0.6", false },
    { "non-numbers are ignored", "'x'", true }
};
static const SCase g_multipleOfInteger[] = {
    { "a multiple is valid", "10", true },
    { "not a multiple is invalid", "7", false },
    { "non-numbers are ignored", "'foo'", true }
};
static const SCase g_multipleOfNumber[] = {
    { "zero is a multiple of anything", "0", true },
    { "4.5 is a multiple of 1.5", "4.5", true },
    { "35 is not a multiple of 1.5", "35", false }
};
static const SCase g_multipleOfSmall[] = {
    { "0.0075 is a multiple of 0.0001", "0.0075", true },
    { "0.00751 is not a multiple of 0.0001", "0.00751", false }
};
static const SCase g_multipleOfOverflow[] = {
    { "the division overflows a double and the value still is not a multiple", "1e308", false }
};
static const SCase g_multipleOfHalfOverflow[] = {
    { "all integers are multiples of 0.5, also when the division overflows", "1e308", true }
};
static const SCase g_multipleOfTinyDivisor[] = {
    { "a big integer is a multiple of a tiny number", "12391239123", true }
};

static const SCase g_allOf[] = {
    { "allOf", "{'foo':'baz','bar':2}", true },
    { "mismatch second", "{'foo':'baz'}", false },
    { "mismatch first", "{'bar':2}", false },
    { "wrong type", "{'foo':'baz','bar':'quux'}", false }
};
static const SCase g_allOfBase[] = {
    { "valid", "{'foo':'quux','bar':2,'baz':null}", true },
    { "mismatch the base schema", "{'foo':'quux','baz':null}", false },
    { "mismatch the first allOf", "{'bar':2,'baz':null}", false },
    { "mismatch the second allOf", "{'foo':'quux','bar':2}", false },
    { "mismatch both", "{'bar':2}", false }
};
static const SCase g_allOfSimple[] = {
    { "valid", "25", true },
    { "mismatch one", "35", false }
};
static const SCase g_allOfTrue[] = {
    { "any value is valid", "'foo'", true }
};
static const SCase g_allOfSomeFalse[] = {
    { "any value is invalid", "'foo'", false }
};
static const SCase g_anyOf[] = {
    { "the first is valid", "1", true },
    { "the second is valid", "2.5", true },
    { "both are valid", "3", true },
    { "neither is invalid", "1.5", false }
};
static const SCase g_anyOfBase[] = {
    { "a mismatch of the base schema", "3", false },
    { "one anyOf valid", "'foobar'", true },
    { "both anyOf invalid", "'foo'", false }
};
static const SCase g_anyOfBooleans[] = {
    { "valid with a true in anyOf", "'foo'", true }
};
static const SCase g_anyOfFalse[] = {
    { "invalid with only false in anyOf", "'foo'", false }
};
static const SCase g_anyOfComplex[] = {
    { "the first is valid", "{'bar':2}", true },
    { "the second is valid", "{'foo':'baz'}", true },
    { "both are valid", "{'foo':'baz','bar':2}", true },
    { "neither is invalid", "{'foo':2,'bar':'quux'}", false }
};
static const SCase g_oneOf[] = {
    { "the first is valid", "1", true },
    { "the second is valid", "2.5", true },
    { "both are invalid", "3", false },
    { "neither is invalid", "1.5", false }
};
static const SCase g_oneOfAllTrue[] = {
    { "three true schemas are invalid", "'foo'", false }
};
static const SCase g_oneOfOneTrue[] = {
    { "one true of three is valid", "'foo'", true }
};
static const SCase g_oneOfRequired[] = {
    { "both invalid is invalid", "{'bar':2}", false },
    { "the first valid is valid", "{'foo':1,'bar':2}", true },
    { "the second valid is valid", "{'foo':1,'baz':3}", true },
    { "both valid is invalid", "{'foo':1,'bar':2,'baz':3}", false }
};
static const SCase g_not[] = {
    { "allowed", "'foo'", true },
    { "disallowed", "1", false }
};
static const SCase g_notMultipleTypes[] = {
    { "valid", "'foo'", true },
    { "a mismatch", "1", false },
    { "another mismatch", "true", false }
};
static const SCase g_notObject[] = {
    { "a match", "1", true },
    { "another match", "{'foo':1}", true },
    { "a mismatch", "{'foo':'bar'}", false }
};
static const SCase g_notTrue[] = {
    { "any value is invalid", "'foo'", false }
};
static const SCase g_notFalse[] = {
    { "any value is valid", "'foo'", true }
};
static const SCase g_notNot[] = {
    { "a double negation is the schema itself: valid", "1", true },
    { "a double negation is the schema itself: invalid", "'1'", false }
};
static const SCase g_ifThenElse[] = {
    { "valid through then", "-1", true },
    { "invalid through then", "-100", false },
    { "valid through else", "4", true },
    { "invalid through else", "3", false }
};
static const SCase g_ifAlone[] = {
    { "a valid if alone is ignored", "0", true },
    { "an invalid if alone is ignored", "'hello'", true }
};
static const SCase g_thenWithoutIf[] = {
    { "then without if is ignored, also when it would fail", "3", true }
};
static const SCase g_elseWithoutIf[] = {
    { "else without if is ignored, also when it would fail", "3", true }
};
static const SCase g_ifThenOnly[] = {
    { "valid through then", "-1", true },
    { "invalid through then", "-100", false },
    { "valid when if fails", "3", true }
};

static const SCase g_refRoot[] = {
    { "a match", "{'foo':false}", true },
    { "a recursive match", "{'foo':{'foo':false}}", true },
    { "a mismatch", "{'bar':false}", false },
    { "a recursive mismatch", "{'foo':{'bar':false}}", false }
};
static const SCase g_refRelative[] = {
    { "a match", "{'bar':3}", true },
    { "a mismatch", "{'bar':true}", false }
};
static const SCase g_refEscaped[] = {
    { "slash invalid", "{'slash':'aoeu'}", false },
    { "tilde invalid", "{'tilde':'aoeu'}", false },
    { "percent invalid", "{'percent':'aoeu'}", false },
    { "slash valid", "{'slash':123}", true },
    { "tilde valid", "{'tilde':123}", true },
    { "percent valid", "{'percent':123}", true }
};
static const SCase g_refNested[] = {
    { "a nested ref valid", "5", true },
    { "a nested ref invalid", "'a'", false }
};
static const SCase g_refSiblings[] = {
    { "the ref valid, maxItems valid", "{'foo':[]}", true },
    { "the ref valid, maxItems invalid", "{'foo':[1,2,3]}", false },
    { "the ref invalid", "{'foo':'string'}", false }
};
static const SCase g_refToFalse[] = {
    { "a ref to false is invalid", "'foo'", false }
};
static const SCase g_refToTrue[] = {
    { "a ref to true is valid", "'foo'", true }
};
static const SCase g_refTree[] = {
    { "a valid tree", "{'value':1,'children':[{'value':2,'children':[]},{'value':3,'children':[{'value':4}]}]}", true },
    { "an invalid node deep down", "{'value':1,'children':[{'value':2},{'value':3,'children':[{'value':'four'}]}]}", false },
    { "a node without value deep down", "{'value':1,'children':[{'children':[]}]}", false }
};
static const SCase g_refPrefixItems[] = {
    { "valid", "[1,2]", true },
    { "the referred position checks", "[1,'a']", false }
};
static const SCase g_refWithRootId[] = {
    { "valid through the ref", "1", true },
    { "invalid through the ref", "'1'", false }
};
static const SCase g_refInDefsOnly[] = {
    { "the unreferenced definition does nothing", "'anything'", true }
};

static const SCase g_booleanTrue[] = {
    { "a number is valid", "1", true },
    { "a string is valid", "'foo'", true },
    { "an object is valid", "{'foo':'bar'}", true },
    { "null is valid", "null", true }
};
static const SCase g_booleanFalse[] = {
    { "a number is invalid", "1", false },
    { "an empty object is invalid", "{}", false },
    { "null is invalid", "null", false }
};
static const SCase g_emptySchema[] = {
    { "a number is valid", "1", true },
    { "an array is valid", "[1,{}]", true },
    { "null is valid", "null", true }
};
static const SCase g_annotations[] = {
    { "format is an annotation and not checked", "'not an e-mail address'", true },
    { "the type still is checked", "1", false }
};
static const SCase g_mcpToolInput[] = {
    { "a typical tool call is valid", "{'symbol':'VZ','days':30,'includeDividends':true}", true },
    { "only the required argument is valid", "{'symbol':'VZ'}", true },
    { "a missing required argument is invalid", "{'days':30}", false },
    { "an unknown argument is invalid", "{'symbol':'VZ','sneaky':1}", false },
    { "a number of days out of range is invalid", "{'symbol':'VZ','days':0}", false },
    { "a fractional number of days is invalid", "{'symbol':'VZ','days':1.5}", false },
    { "a whole float number of days is valid", "{'symbol':'VZ','days':2.0}", true },
    { "an empty symbol is invalid", "{'symbol':''}", false },
    { "a value of the enum is valid", "{'symbol':'VZ','period':'month'}", true },
    { "a value outside the enum is invalid", "{'symbol':'VZ','period':'decade'}", false }
};

static const SGroup g_groups[] = {
    { "type integer", "{'type':'integer'}", g_typeInteger, CASE_COUNT( g_typeInteger ) },
    { "type number", "{'type':'number'}", g_typeNumber, CASE_COUNT( g_typeNumber ) },
    { "type string", "{'type':'string'}", g_typeString, CASE_COUNT( g_typeString ) },
    { "type object", "{'type':'object'}", g_typeObject, CASE_COUNT( g_typeObject ) },
    { "type array", "{'type':'array'}", g_typeArray, CASE_COUNT( g_typeArray ) },
    { "type boolean", "{'type':'boolean'}", g_typeBoolean, CASE_COUNT( g_typeBoolean ) },
    { "type null", "{'type':'null'}", g_typeNull, CASE_COUNT( g_typeNull ) },
    { "multiple types", "{'type':['integer','string']}", g_typeMultiple, CASE_COUNT( g_typeMultiple ) },
    { "type array or object", "{'type':['array','object']}", g_typeArrayOrObject, CASE_COUNT( g_typeArrayOrObject ) },

    { "simple enum", "{'enum':[1,2,3]}", g_enumSimple, CASE_COUNT( g_enumSimple ) },
    { "heterogeneous enum", "{'enum':[6,'foo',[],true,{'foo':12}]}", g_enumHeterogeneous, CASE_COUNT( g_enumHeterogeneous ) },
    { "enum with false does not match 0", "{'enum':[false]}", g_enumFalse, CASE_COUNT( g_enumFalse ) },
    { "enum with 0 does not match false", "{'enum':[0]}", g_enumZero, CASE_COUNT( g_enumZero ) },
    { "nested enum", "{'enum':[[false]]}", g_enumNested, CASE_COUNT( g_enumNested ) },
    { "enum with escaped characters", "{'enum':['foo\\nbar','foo\\rbar']}", g_enumEscaped, CASE_COUNT( g_enumEscaped ) },

    { "const number", "{'const':2}", g_constNumber, CASE_COUNT( g_constNumber ) },
    { "const object", "{'const':{'foo':'bar','baz':'bax'}}", g_constObject, CASE_COUNT( g_constObject ) },
    { "const array", "{'const':[{'foo':'bar'}]}", g_constArray, CASE_COUNT( g_constArray ) },
    { "const null", "{'const':null}", g_constNull, CASE_COUNT( g_constNull ) },
    { "const false", "{'const':false}", g_constFalse, CASE_COUNT( g_constFalse ) },
    { "const float", "{'const':-2.0}", g_constFloat, CASE_COUNT( g_constFloat ) },
    { "const large integer", "{'const':9007199254740992}", g_constLargeInteger, CASE_COUNT( g_constLargeInteger ) },

    { "properties", "{'properties':{'foo':{'type':'integer'},'bar':{'type':'string'}}}", g_properties, CASE_COUNT( g_properties ) },
    { "properties with boolean schemas", "{'properties':{'foo':true,'bar':false}}", g_propertiesBoolean, CASE_COUNT( g_propertiesBoolean ) },
    { "properties with escaped and pointer-like names", "{'properties':{'foo\\nbar':{'type':'number'},'foo\\\"bar':{'type':'number'},'foo/bar':{'type':'number'},'foo.bar':{'type':'number'},'foo~bar':{'type':'number'}}}", g_propertiesEscaped, CASE_COUNT( g_propertiesEscaped ) },
    { "properties with a null valued instance", "{'properties':{'foo':{'type':'null'}}}", g_propertiesNull, CASE_COUNT( g_propertiesNull ) },
    { "properties named like object machinery", "{'properties':{'__proto__':{'type':'string'},'toString':{'properties':{'length':{'type':'string'}}},'constructor':{'properties':{'length':{'type':'string'}}}}}", g_propertiesProto, CASE_COUNT( g_propertiesProto ) },
    { "required", "{'properties':{'foo':{},'bar':{}},'required':['foo']}", g_required, CASE_COUNT( g_required ) },
    { "required empty", "{'required':[]}", g_requiredEmpty, CASE_COUNT( g_requiredEmpty ) },
    { "required with escaped names", "{'required':['foo\\nbar','foo\\\"bar','foo\\\\bar']}", g_requiredEscaped, CASE_COUNT( g_requiredEscaped ) },
    { "required in a property", "{'properties':{'a':{'required':['b']}}}", g_requiredNested, CASE_COUNT( g_requiredNested ) },
    { "additionalProperties false", "{'properties':{'foo':{},'bar':{}},'additionalProperties':false}", g_additionalFalse, CASE_COUNT( g_additionalFalse ) },
    { "additionalProperties with a schema", "{'properties':{'foo':{},'bar':{}},'additionalProperties':{'type':'boolean'}}", g_additionalSchema, CASE_COUNT( g_additionalSchema ) },
    { "additionalProperties alone", "{'additionalProperties':{'type':'boolean'}}", g_additionalAlone, CASE_COUNT( g_additionalAlone ) },
    { "additionalProperties with a null valued instance", "{'additionalProperties':{'type':'null'}}", g_additionalNull, CASE_COUNT( g_additionalNull ) },
    { "propertyNames", "{'propertyNames':{'maxLength':3}}", g_propertyNames, CASE_COUNT( g_propertyNames ) },
    { "propertyNames false", "{'propertyNames':false}", g_propertyNamesFalse, CASE_COUNT( g_propertyNamesFalse ) },
    { "propertyNames with const", "{'propertyNames':{'const':'foo'}}", g_propertyNamesConst, CASE_COUNT( g_propertyNamesConst ) },
    { "propertyNames with enum", "{'propertyNames':{'enum':['foo','bar']}}", g_propertyNamesEnum, CASE_COUNT( g_propertyNamesEnum ) },
    { "minProperties", "{'minProperties':1}", g_minProperties, CASE_COUNT( g_minProperties ) },
    { "minProperties as a whole float", "{'minProperties':1.0}", g_minProperties, CASE_COUNT( g_minProperties ) },
    { "maxProperties", "{'maxProperties':2}", g_maxProperties, CASE_COUNT( g_maxProperties ) },
    { "maxProperties 0", "{'maxProperties':0}", g_maxPropertiesZero, CASE_COUNT( g_maxPropertiesZero ) },
    { "dependentRequired", "{'dependentRequired':{'bar':['foo']}}", g_dependentRequired, CASE_COUNT( g_dependentRequired ) },
    { "dependentRequired empty", "{'dependentRequired':{'bar':[]}}", g_dependentRequiredEmpty, CASE_COUNT( g_dependentRequiredEmpty ) },
    { "dependentRequired multiple", "{'dependentRequired':{'quux':['foo','bar']}}", g_dependentRequiredMultiple, CASE_COUNT( g_dependentRequiredMultiple ) },

    { "items", "{'items':{'type':'integer'}}", g_items, CASE_COUNT( g_items ) },
    { "items true", "{'items':true}", g_itemsTrue, CASE_COUNT( g_itemsTrue ) },
    { "items false", "{'items':false}", g_itemsFalse, CASE_COUNT( g_itemsFalse ) },
    { "nested items", "{'type':'array','items':{'type':'array','items':{'type':'array','items':{'type':'array','items':{'type':'number'}}}}}", g_itemsNested, CASE_COUNT( g_itemsNested ) },
    { "items of objects", "{'items':{'type':'object','required':['a'],'properties':{'a':{'type':'integer'}}}}", g_itemsOfObjects, CASE_COUNT( g_itemsOfObjects ) },
    { "items with null instance elements", "{'items':{'type':'null'}}", g_itemsNull, CASE_COUNT( g_itemsNull ) },
    { "prefixItems", "{'prefixItems':[{'type':'integer'},{'type':'string'}]}", g_prefixItems, CASE_COUNT( g_prefixItems ) },
    { "prefixItems with items false", "{'prefixItems':[{},{}],'items':false}", g_prefixItemsWithItemsFalse, CASE_COUNT( g_prefixItemsWithItemsFalse ) },
    { "prefixItems with items", "{'prefixItems':[{'type':'string'}],'items':{'type':'integer'}}", g_prefixItemsWithItems, CASE_COUNT( g_prefixItemsWithItems ) },
    { "minItems", "{'minItems':1}", g_minItems, CASE_COUNT( g_minItems ) },
    { "minItems as a whole float", "{'minItems':1.0}", g_minItems, CASE_COUNT( g_minItems ) },
    { "maxItems", "{'maxItems':2}", g_maxItems, CASE_COUNT( g_maxItems ) },
    { "maxItems as a whole float", "{'maxItems':2.0}", g_maxItems, CASE_COUNT( g_maxItems ) },
    { "uniqueItems", "{'uniqueItems':true}", g_uniqueItems, CASE_COUNT( g_uniqueItems ) },
    { "uniqueItems false", "{'uniqueItems':false}", g_uniqueItemsFalse, CASE_COUNT( g_uniqueItemsFalse ) },

    { "minLength", "{'minLength':2}", g_minLength, CASE_COUNT( g_minLength ) },
    { "minLength as a whole float", "{'minLength':2.0}", g_minLength, CASE_COUNT( g_minLength ) },
    { "maxLength", "{'maxLength':2}", g_maxLength, CASE_COUNT( g_maxLength ) },
    { "minimum", "{'minimum':1.1}", g_minimum, CASE_COUNT( g_minimum ) },
    { "minimum negative", "{'minimum':-2}", g_minimumNegative, CASE_COUNT( g_minimumNegative ) },
    { "minimum of a big integer", "{'minimum':9007199254740993}", g_minimumExact, CASE_COUNT( g_minimumExact ) },
    { "maximum", "{'maximum':3.0}", g_maximum, CASE_COUNT( g_maximum ) },
    { "maximum integer", "{'maximum':300}", g_maximumInteger, CASE_COUNT( g_maximumInteger ) },
    { "exclusiveMaximum", "{'exclusiveMaximum':3.0}", g_exclusiveMaximum, CASE_COUNT( g_exclusiveMaximum ) },
    { "exclusiveMinimum", "{'exclusiveMinimum':1.1}", g_exclusiveMinimum, CASE_COUNT( g_exclusiveMinimum ) },
    { "multipleOf an integer", "{'multipleOf':2}", g_multipleOfInteger, CASE_COUNT( g_multipleOfInteger ) },
    { "multipleOf a number", "{'multipleOf':1.5}", g_multipleOfNumber, CASE_COUNT( g_multipleOfNumber ) },
    { "multipleOf a small number", "{'multipleOf':0.0001}", g_multipleOfSmall, CASE_COUNT( g_multipleOfSmall ) },
    { "multipleOf with a division beyond a double", "{'type':'integer','multipleOf':0.123456789}", g_multipleOfOverflow, CASE_COUNT( g_multipleOfOverflow ) },
    { "multipleOf 0.5 with a division beyond a double", "{'type':'integer','multipleOf':0.5}", g_multipleOfHalfOverflow, CASE_COUNT( g_multipleOfHalfOverflow ) },
    { "multipleOf a tiny number", "{'type':'integer','multipleOf':1e-8}", g_multipleOfTinyDivisor, CASE_COUNT( g_multipleOfTinyDivisor ) },

    { "allOf", "{'allOf':[{'properties':{'bar':{'type':'integer'}},'required':['bar']},{'properties':{'foo':{'type':'string'}},'required':['foo']}]}", g_allOf, CASE_COUNT( g_allOf ) },
    { "allOf with a base schema", "{'properties':{'bar':{'type':'integer'}},'required':['bar'],'allOf':[{'properties':{'foo':{'type':'string'}},'required':['foo']},{'properties':{'baz':{'type':'null'}},'required':['baz']}]}", g_allOfBase, CASE_COUNT( g_allOfBase ) },
    { "allOf simple types", "{'allOf':[{'maximum':30},{'minimum':20}]}", g_allOfSimple, CASE_COUNT( g_allOfSimple ) },
    { "allOf with all true", "{'allOf':[true,true]}", g_allOfTrue, CASE_COUNT( g_allOfTrue ) },
    { "allOf with some false", "{'allOf':[true,false]}", g_allOfSomeFalse, CASE_COUNT( g_allOfSomeFalse ) },
    { "anyOf", "{'anyOf':[{'type':'integer'},{'minimum':2}]}", g_anyOf, CASE_COUNT( g_anyOf ) },
    { "anyOf with a base schema", "{'type':'string','anyOf':[{'maxLength':2},{'minLength':4}]}", g_anyOfBase, CASE_COUNT( g_anyOfBase ) },
    { "anyOf with true and false", "{'anyOf':[true,false]}", g_anyOfBooleans, CASE_COUNT( g_anyOfBooleans ) },
    { "anyOf with only false", "{'anyOf':[false,false]}", g_anyOfFalse, CASE_COUNT( g_anyOfFalse ) },
    { "anyOf complex types", "{'anyOf':[{'properties':{'bar':{'type':'integer'}},'required':['bar']},{'properties':{'foo':{'type':'string'}},'required':['foo']}]}", g_anyOfComplex, CASE_COUNT( g_anyOfComplex ) },
    { "oneOf", "{'oneOf':[{'type':'integer'},{'minimum':2}]}", g_oneOf, CASE_COUNT( g_oneOf ) },
    { "oneOf with all true", "{'oneOf':[true,true,true]}", g_oneOfAllTrue, CASE_COUNT( g_oneOfAllTrue ) },
    { "oneOf with one true", "{'oneOf':[true,false,false]}", g_oneOfOneTrue, CASE_COUNT( g_oneOfOneTrue ) },
    { "oneOf with required", "{'type':'object','oneOf':[{'required':['foo','bar']},{'required':['foo','baz']}]}", g_oneOfRequired, CASE_COUNT( g_oneOfRequired ) },
    { "not", "{'not':{'type':'integer'}}", g_not, CASE_COUNT( g_not ) },
    { "not multiple types", "{'not':{'type':['integer','boolean']}}", g_notMultipleTypes, CASE_COUNT( g_notMultipleTypes ) },
    { "not more complex schema", "{'not':{'type':'object','properties':{'foo':{'type':'string'}}}}", g_notObject, CASE_COUNT( g_notObject ) },
    { "not true", "{'not':true}", g_notTrue, CASE_COUNT( g_notTrue ) },
    { "not false", "{'not':false}", g_notFalse, CASE_COUNT( g_notFalse ) },
    { "double negation", "{'not':{'not':{'type':'integer'}}}", g_notNot, CASE_COUNT( g_notNot ) },
    { "if then else", "{'if':{'exclusiveMaximum':0},'then':{'minimum':-10},'else':{'multipleOf':2}}", g_ifThenElse, CASE_COUNT( g_ifThenElse ) },
    { "if without then and else", "{'if':{'const':0}}", g_ifAlone, CASE_COUNT( g_ifAlone ) },
    { "then without if", "{'then':{'const':0}}", g_thenWithoutIf, CASE_COUNT( g_thenWithoutIf ) },
    { "else without if", "{'else':{'const':0}}", g_elseWithoutIf, CASE_COUNT( g_elseWithoutIf ) },
    { "if then without else", "{'if':{'exclusiveMaximum':0},'then':{'minimum':-10}}", g_ifThenOnly, CASE_COUNT( g_ifThenOnly ) },

    { "$ref to the root", "{'properties':{'foo':{'$ref':'#'}},'additionalProperties':false}", g_refRoot, CASE_COUNT( g_refRoot ) },
    { "$ref with a relative pointer", "{'properties':{'foo':{'type':'integer'},'bar':{'$ref':'#/properties/foo'}}}", g_refRelative, CASE_COUNT( g_refRelative ) },
    { "$ref with escaped pointers", "{'$defs':{'tilde~field':{'type':'integer'},'slash/field':{'type':'integer'},'percent%field':{'type':'integer'}},'properties':{'tilde':{'$ref':'#/$defs/tilde~0field'},'slash':{'$ref':'#/$defs/slash~1field'},'percent':{'$ref':'#/$defs/percent%25field'}}}", g_refEscaped, CASE_COUNT( g_refEscaped ) },
    { "nested $refs", "{'$defs':{'a':{'type':'integer'},'b':{'$ref':'#/$defs/a'},'c':{'$ref':'#/$defs/b'}},'$ref':'#/$defs/c'}", g_refNested, CASE_COUNT( g_refNested ) },
    { "$ref applies next to sibling keywords", "{'$defs':{'reffed':{'type':'array'}},'properties':{'foo':{'$ref':'#/$defs/reffed','maxItems':2}}}", g_refSiblings, CASE_COUNT( g_refSiblings ) },
    { "$ref to the false schema", "{'$ref':'#/$defs/bool','$defs':{'bool':false}}", g_refToFalse, CASE_COUNT( g_refToFalse ) },
    { "$ref to the true schema", "{'$ref':'#/$defs/bool','$defs':{'bool':true}}", g_refToTrue, CASE_COUNT( g_refToTrue ) },
    { "a recursive tree through $ref", "{'$defs':{'node':{'type':'object','properties':{'value':{'type':'number'},'children':{'type':'array','items':{'$ref':'#/$defs/node'}}},'required':['value']}},'$ref':'#/$defs/node'}", g_refTree, CASE_COUNT( g_refTree ) },
    { "$ref to a prefixItems position", "{'prefixItems':[{'type':'integer'},{'$ref':'#/prefixItems/0'}]}", g_refPrefixItems, CASE_COUNT( g_refPrefixItems ) },
    { "$ref next to $id and $schema at the root", "{'$schema':'https://json-schema.org/draft/2020-12/schema','$id':'https://example.com/root.json','$defs':{'A':{'type':'integer'}},'$ref':'#/$defs/A'}", g_refWithRootId, CASE_COUNT( g_refWithRootId ) },
    { "definitions without a $ref", "{'$defs':{'unused':{'type':'integer'}}}", g_refInDefsOnly, CASE_COUNT( g_refInDefsOnly ) },

    { "the true schema", "true", g_booleanTrue, CASE_COUNT( g_booleanTrue ) },
    { "the false schema", "false", g_booleanFalse, CASE_COUNT( g_booleanFalse ) },
    { "the empty schema", "{}", g_emptySchema, CASE_COUNT( g_emptySchema ) },
    { "annotations and extensions are kept and not checked", "{'type':'string','title':'t','description':'d','default':'x','examples':['a'],'format':'email','deprecated':true,'readOnly':false,'writeOnly':false,'$comment':'c','x-mcp-header':'X-Foo','contentEncoding':'base64','contentMediaType':'text/plain','contentSchema':{'type':'object'}}", g_annotations, CASE_COUNT( g_annotations ) },
    { "an MCP tool input schema", "{'type':'object','properties':{'symbol':{'type':'string','minLength':1,'description':'ticker'},'days':{'type':'integer','minimum':1,'maximum':3650},'includeDividends':{'type':'boolean','default':false},'period':{'enum':['day','week','month']}},'required':['symbol'],'additionalProperties':false}", g_mcpToolInput, CASE_COUNT( g_mcpToolInput ) }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformRefusalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON Schema: the 2020-12 dialect is accepted, other dialects are refused" )
        try
        {
            ASSERT_TRUE( Compiles( "{'$schema':'https://json-schema.org/draft/2020-12/schema','type':'string'}" ) );
            ASSERT_TRUE( Compiles( "{'$schema':'https://json-schema.org/draft/2020-12/schema#'}" ) );
            ASSERT_TRUE( RefusedWith( "{'$schema':'http://json-schema.org/draft-07/schema#','type':'string'}", "dialect not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'$schema':'https://json-schema.org/draft/2019-09/schema'}", "dialect not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'properties':{'a':{'$schema':'http://json-schema.org/draft-04/schema#'}}}", "dialect not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'$schema':5}", "must be a string" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema: references that are not local, or point nowhere, are refused" )
        try
        {
            ASSERT_TRUE( RefusedWith( "{'$ref':'https://json-schema.org/draft/2020-12/schema'}", "not local" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':'http://localhost:1234/node.json'}", "never dereferenced" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':'other.json#/$defs/a'}", "not local" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':''}", "not local" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':'#foo','$defs':{'a':{}}}", "anchor" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':'#/$defs/missing','$defs':{'a':{}}}", "does not point at a schema" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':'#/properties','properties':{'a':{}}}", "does not point at a schema" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':'#/default','default':{}}", "does not point at a schema" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':'#/%zz'}", "not a valid JSON pointer" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':'#/a~2b'}", "not a valid JSON pointer" ) );
            ASSERT_TRUE( RefusedWith( "{'$ref':5}", "must be a string" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema: unknown and unsupported keywords are refused" )
        try
        {
            ASSERT_TRUE( RefusedWith( "{'typo':'string'}", "unknown keyword \"typo\"" ) );
            ASSERT_TRUE( RefusedWith( "{'properties':{'a':{'tpye':'string'}}}", "#/properties/a/tpye: unknown keyword" ) );
            ASSERT_TRUE( RefusedWith( "{'definitions':{}}", "unknown keyword" ) );
            ASSERT_TRUE( RefusedWith( "{'dependencies':{}}", "unknown keyword" ) );
            ASSERT_TRUE( RefusedWith( "{'additionalItems':false}", "unknown keyword" ) );
            ASSERT_TRUE( RefusedWith( "{'pattern':'^a'}", "regular expression" ) );
            ASSERT_TRUE( RefusedWith( "{'patternProperties':{'^a':{}}}", "regular expression" ) );
            ASSERT_TRUE( RefusedWith( "{'contains':{'type':'integer'}}", "not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'minContains':1}", "not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'dependentSchemas':{'a':{}}}", "not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'unevaluatedProperties':false}", "not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'unevaluatedItems':false}", "not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'$anchor':'a'}", "not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'$dynamicRef':'#a'}", "not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'$vocabulary':{}}", "not supported" ) );
            ASSERT_TRUE( RefusedWith( "{'properties':{'a':{'$id':'https://example.com/a'}}}", "$id is only supported at the root" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema: keyword values of the wrong kind are refused" )
        try
        {
            ASSERT_TRUE( RefusedWith( "'string'", "must be an object or a boolean" ) );
            ASSERT_TRUE( RefusedWith( "{'properties':{'a':5}}", "#/properties/a: a schema must be an object or a boolean" ) );
            ASSERT_TRUE( RefusedWith( "{'items':[{'type':'integer'}]}", "prefixItems" ) );
            ASSERT_TRUE( RefusedWith( "{'type':'strin'}", "unknown type \"strin\"" ) );
            ASSERT_TRUE( RefusedWith( "{'type':[]}", "non-empty array of type names" ) );
            ASSERT_TRUE( RefusedWith( "{'type':['string','string']}", "twice" ) );
            ASSERT_TRUE( RefusedWith( "{'type':5}", "type name" ) );
            ASSERT_TRUE( RefusedWith( "{'minLength':-1}", "non-negative integer" ) );
            ASSERT_TRUE( RefusedWith( "{'minLength':1.5}", "non-negative integer" ) );
            ASSERT_TRUE( RefusedWith( "{'maxItems':'2'}", "non-negative integer" ) );
            ASSERT_TRUE( RefusedWith( "{'multipleOf':0}", "greater than 0" ) );
            ASSERT_TRUE( RefusedWith( "{'multipleOf':-2}", "greater than 0" ) );
            ASSERT_TRUE( RefusedWith( "{'minimum':'1'}", "must be a number" ) );
            ASSERT_TRUE( RefusedWith( "{'exclusiveMinimum':true}", "must be a number" ) );
            ASSERT_TRUE( RefusedWith( "{'required':'foo'}", "array of strings" ) );
            ASSERT_TRUE( RefusedWith( "{'required':['foo',1]}", "array of strings" ) );
            ASSERT_TRUE( RefusedWith( "{'required':['foo','foo']}", "twice" ) );
            ASSERT_TRUE( RefusedWith( "{'enum':5}", "must be an array" ) );
            ASSERT_TRUE( RefusedWith( "{'allOf':[]}", "non-empty array of schemas" ) );
            ASSERT_TRUE( RefusedWith( "{'anyOf':{}}", "non-empty array of schemas" ) );
            ASSERT_TRUE( RefusedWith( "{'oneOf':[{},5]}", "#/oneOf/1: a schema must be an object or a boolean" ) );
            ASSERT_TRUE( RefusedWith( "{'uniqueItems':1}", "must be a boolean" ) );
            ASSERT_TRUE( RefusedWith( "{'dependentRequired':{'a':'b'}}", "arrays of strings" ) );
            ASSERT_TRUE( RefusedWith( "{'dependentRequired':{'a':['b','b']}}", "twice" ) );
            ASSERT_TRUE( RefusedWith( "{'properties':[]}", "members are schemas" ) );
            ASSERT_TRUE( RefusedWith( "{'$defs':{'a':'x'}}", "#/$defs/a: a schema must be an object or a boolean" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema: a reference cycle that never reaches into the instance is refused" )
        try
        {
            ASSERT_TRUE( RefusedWith( "{'$ref':'#'}", "cycle of references" ) );
            ASSERT_TRUE( RefusedWith( "{'$defs':{'a':{'$ref':'#/$defs/b'},'b':{'allOf':[{'$ref':'#/$defs/a'}]}},'$ref':'#/$defs/a'}", "cycle of references" ) );
            ASSERT_TRUE( RefusedWith( "{'not':{'$ref':'#'}}", "cycle of references" ) );
            ASSERT_TRUE( RefusedWith( "{'anyOf':[{'type':'string'},{'$ref':'#'}]}", "cycle of references" ) );
            ASSERT_TRUE( RefusedWith( "{'if':{'$ref':'#'}}", "cycle of references" ) );
            // recursion through a keyword that reaches into the instance is fine
            ASSERT_TRUE( Compiles( "{'items':{'$ref':'#'}}" ) );
            ASSERT_TRUE( Compiles( "{'properties':{'next':{'$ref':'#'}},'additionalProperties':{'$ref':'#'}}" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/** Nests 'levels' properties schemas: {'properties':{'a':{'properties':{'a':{}}}}} for 2 levels */
static CString
NestedPropertiesSchema( UInt32 levels )
{GUCEF_TRACE;

    CString text;
    for ( UInt32 i=0; i<levels; ++i )
        text += "{'properties':{'a':";
    text += "{}";
    for ( UInt32 i=0; i<levels; ++i )
        text += "}}";
    return text;
}

/*-------------------------------------------------------------------------*/

/** A schema whose subschemas are 'count' true schemas in one allOf */
static CString
WideAllOfSchema( UInt32 count )
{GUCEF_TRACE;

    CString text = "{'allOf':[";
    for ( UInt32 i=0; i<count; ++i )
        text += ( 0 == i ) ? "true" : ",true";
    text += "]}";
    return text;
}

/*-------------------------------------------------------------------------*/

/** An array node nested 'levels' deep */
static void
BuildNestedArrays( CDataNode& root, UInt32 levels )
{GUCEF_TRACE;

    root.Clear();
    root.SetNodeType( GUCEF_DATATYPE_ARRAY );
    CDataNode* node = &root;
    for ( UInt32 i=1; i<levels; ++i )
        node = node->AddChild( CString::Empty, GUCEF_DATATYPE_ARRAY );
}

/*-------------------------------------------------------------------------*/

static bool
HasErrorWith( const CJsonSchemaValidator::TErrorVector& errors, const char* text )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<errors.size(); ++i )
    {
        if ( errors[ i ].message.HasSubstr( text ) >= 0 )
            return true;
    }
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "JsonSchema test: no error with \"" << text << "\" in " << ErrorsText( errors ) );
    return false;
}

/*-------------------------------------------------------------------------*/

static void
PerformLimitTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON Schema limits: nesting deeper than maxDepth is refused" )
        try
        {
            // the root is level 1, every properties level adds one
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( NestedPropertiesSchema( 31 ).C_String(), schema, error ) );
            ASSERT_FALSE( CompileText( NestedPropertiesSchema( 32 ).C_String(), schema, error ) );
            ASSERT_TRUE( error.HasSubstr( "nested deeper than 32 levels" ) >= 0 );
            ASSERT_FALSE( schema.IsCompiled() );
            ASSERT_TRUE( CompileText( NestedPropertiesSchema( 40 ).C_String(), schema, error, 64 ) );
            ASSERT_FALSE( CompileText( NestedPropertiesSchema( 4 ).C_String(), schema, error, 4 ) );

            // the values of const and enum count as well
            CString deepConst = "{'const':";
            for ( UInt32 i=0; i<40; ++i )
                deepConst += "[";
            for ( UInt32 i=0; i<40; ++i )
                deepConst += "]";
            deepConst += "}";
            ASSERT_TRUE( RefusedWith( deepConst.C_String(), "nested deeper than 32 levels" ) );
            ASSERT_TRUE( RefusedWith( ( "{'enum':[1," + deepConst.CutChars( 9, true ).CutChars( 1, false ) + "]}" ).C_String(), "nested deeper than 32 levels" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema limits: more than maxSubschemas subschemas is refused" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( WideAllOfSchema( 1023 ).C_String(), schema, error ) );
            ASSERT_TRUE( 1024 == schema.GetSubschemaCount() );
            ASSERT_FALSE( CompileText( WideAllOfSchema( 1024 ).C_String(), schema, error ) );
            ASSERT_TRUE( error.HasSubstr( "more than 1024 subschemas" ) >= 0 );
            ASSERT_TRUE( 0 == schema.GetSubschemaCount() );
            ASSERT_TRUE( CompileText( WideAllOfSchema( 2000 ).C_String(), schema, error, 32, 4096 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema limits: the step budget ends an expensive validation" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'uniqueItems':true}", schema, error ) );

            // 1000 different items need 499500 comparisons
            CDataNode instance( GUCEF_DATATYPE_ARRAY );
            for ( Int32 i=0; i<1000; ++i )
                instance.AddValueAsChild( CVariant( i ) );

            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            ASSERT_TRUE( 1 == errors.size() );
            ASSERT_TRUE( HasErrorWith( errors, "schema too expensive" ) );
            ASSERT_TRUE( errors.size() > 0 && errors[ 0 ].keyword.IsNULLOrEmpty() );

            ASSERT_TRUE( CJsonSchemaValidator::Validate( schema, instance, errors, 16, 1000000 ) );
            ASSERT_TRUE( errors.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema limits: an exponential anyOf schema is stopped by the budget, quickly" )
        try
        {
            // every level tries both branches: 2^24 evaluations without a budget
            CString text = "{'$defs':{";
            for ( UInt32 i=0; i<24; ++i )
            {
                text += "'l" + UInt32ToString( i ) + "':{'anyOf':[{'$ref':'#/$defs/l" + UInt32ToString( i+1 ) +
                        "'},{'$ref':'#/$defs/l" + UInt32ToString( i+1 ) + "'}]},";
            }
            text += "'l24':false},'$ref':'#/$defs/l0'}";

            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( text.C_String(), schema, error ) );

            CDataNode instance;
            instance.SetValue( CVariant( (Int32) 1 ) );
            CJsonSchemaValidator::TErrorVector errors;
            UInt64 start = CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            UInt64 duration = CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() - start;
            ASSERT_TRUE( HasErrorWith( errors, "schema too expensive" ) );
            ASSERT_TRUE( duration < 5000 );
            GUCEF_LOG( LOGLEVEL_NORMAL, "JsonSchema test: the exponential schema stopped after " << duration << " ms" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema limits: a deeply nested instance is stopped by the evaluation depth" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'type':'array','items':{'$ref':'#'}}", schema, error ) );

            CDataNode shallow;
            BuildNestedArrays( shallow, 50 );
            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_TRUE( CJsonSchemaValidator::Validate( schema, shallow, errors ) );

            CDataNode deep;
            BuildNestedArrays( deep, 2000 );
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, deep, errors, 16, 10000000 ) );
            ASSERT_TRUE( HasErrorWith( errors, "nests deeper than 512 levels" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema limits: equality of deep values is bounded too" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'uniqueItems':true}", schema, error ) );

            CDataNode instance( GUCEF_DATATYPE_ARRAY );
            CDataNode first;
            BuildNestedArrays( first, 1000 );
            instance.AddChild( first );
            instance.AddChild( first );
            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors, 16, 10000000 ) );
            ASSERT_TRUE( HasErrorWith( errors, "schema too expensive" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema limits: maxErrors bounds the errors collected" )
        try
        {
            CString schemaText = "{'properties':{";
            CString instanceText = "{";
            for ( UInt32 i=0; i<30; ++i )
            {
                CString name = "'p" + UInt32ToString( i ) + "'";
                schemaText += ( 0 == i ? "" : "," ) + name + ":{'type':'integer'}";
                instanceText += ( 0 == i ? "" : "," ) + name + ":'x'";
            }
            schemaText += "}}";
            instanceText += "}";

            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( schemaText.C_String(), schema, error ) );
            CDataNode instance;
            ASSERT_TRUE( ParseValue( instanceText.C_String(), instance ) );

            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            ASSERT_TRUE( 16 == errors.size() );
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors, 3 ) );
            ASSERT_TRUE( 3 == errors.size() );
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors, 100 ) );
            ASSERT_TRUE( 30 == errors.size() );

            // no errors asked for: only the answer
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors, 0 ) );
            ASSERT_TRUE( errors.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema limits: uniqueItems on two objects of 50000 object members ends within a second" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'properties':{'tags':{'type':'array','uniqueItems':true}}}", schema, error ) );

            // the members are objects, so they are child nodes; the second object has them in the reverse order
            CDataNode instance( GUCEF_DATATYPE_OBJECT );
            CDataNode* tags = instance.AddChild( "tags", GUCEF_DATATYPE_ARRAY );
            CDataNode* first = tags->AddChild( CString::Empty, GUCEF_DATATYPE_OBJECT );
            CDataNode* second = tags->AddChild( CString::Empty, GUCEF_DATATYPE_OBJECT );
            const UInt32 memberCount = 50000;
            for ( UInt32 i=0; i<memberCount; ++i )
            {
                first->AddChild( "m" + UInt32ToString( i ), GUCEF_DATATYPE_OBJECT );
                second->AddChild( "m" + UInt32ToString( memberCount - 1 - i ), GUCEF_DATATYPE_OBJECT );
            }

            CJsonSchemaValidator::TErrorVector errors;
            UInt64 start = CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
            bool valid = CJsonSchemaValidator::Validate( schema, instance, errors );
            UInt64 duration = CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() - start;
            GUCEF_LOG( LOGLEVEL_NORMAL, "JsonSchema test: uniqueItems on two objects of " << memberCount << " members took " << duration << " ms: " << ErrorsText( errors ) );
            ASSERT_FALSE( valid );
            ASSERT_TRUE( HasErrorWith( errors, "are equal, but the items must be unique" ) || HasErrorWith( errors, "schema too expensive" ) );
            ASSERT_TRUE( duration < 1000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema limits: properties, required and dependentRequired on an object of 100000 object members end within a second" )
        try
        {
            // 300 names in each keyword, none of them present: every look-up misses
            CString properties = "{";
            CString required = "[";
            CString dependent = "{";
            for ( UInt32 i=0; i<300; ++i )
            {
                CString name = "'p" + UInt32ToString( i ) + "'";
                properties += ( 0 == i ? "" : "," ) + name + ":{'type':'integer'}";
                required += ( 0 == i ? "" : "," ) + name;
                dependent += ( 0 == i ? "" : "," ) + CString( "'q" ) + UInt32ToString( i ) + "':['r']";
            }
            CString schemaText = "{'properties':" + properties + "},'dependentRequired':" + dependent + "},'not':{'required':" + required + "]}}";

            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( schemaText.C_String(), schema, error ) );

            CDataNode instance( GUCEF_DATATYPE_OBJECT );
            for ( UInt32 i=0; i<100000; ++i )
                instance.AddChild( "m" + UInt32ToString( i ), GUCEF_DATATYPE_OBJECT );

            CJsonSchemaValidator::TErrorVector errors;
            UInt64 start = CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
            bool valid = CJsonSchemaValidator::Validate( schema, instance, errors );
            UInt64 duration = CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() - start;
            GUCEF_LOG( LOGLEVEL_NORMAL, "JsonSchema test: 900 member look-ups in an object of 100000 members took " << duration << " ms: " << ErrorsText( errors ) );
            ASSERT_TRUE( valid || HasErrorWith( errors, "schema too expensive" ) );
            ASSERT_TRUE( duration < 1000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema: a member name that occurs twice in an object is refused, whatever its value" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'properties':{'a':{'additionalProperties':false}}}", schema, error ) );

            // { "a": {}, "a": { "x": 1 } } as a parser that keeps both members gives it
            CDataNode instance( GUCEF_DATATYPE_OBJECT );
            instance.AddChild( "a", GUCEF_DATATYPE_OBJECT );
            instance.AddChild( "a", GUCEF_DATATYPE_OBJECT )->SetAttribute( CString( "x" ), (Int32) 1 );
            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            ASSERT_TRUE( HasErrorWith( errors, "occurs more than once" ) );

            // a scalar member and an object member of the same name
            CDataNode mixed( GUCEF_DATATYPE_OBJECT );
            mixed.SetAttribute( CString( "a" ), (Int32) 1 );
            mixed.AddChild( "a", GUCEF_DATATYPE_OBJECT );
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, mixed, errors ) );
            ASSERT_TRUE( HasErrorWith( errors, "occurs more than once" ) );

            // also where no keyword looks at the members, and in equality: two objects with a repeated name are no set of members
            CJsonSchema anything;
            ASSERT_TRUE( CompileText( "{'type':'object'}", anything, error ) );
            ASSERT_FALSE( CJsonSchemaValidator::Validate( anything, instance, errors ) );

            CDataNode single( GUCEF_DATATYPE_OBJECT );
            single.AddChild( "a", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( CJsonSchemaValidator::Validate( schema, single, errors ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema numbers: integers beyond 2^53 are compared exactly with whole numbers written with an exponent" )
        try
        {
            // 1e16 is a floating point number for the codec, 10000000000000001 an integer: a double cannot tell them apart
            CJsonSchema maximum;
            CString error;
            ASSERT_TRUE( CompileText( "{'maximum':1e16}", maximum, error ) );
            ASSERT_TRUE( ValidatesAs( maximum, "10000000000000000", true, "1e16 is at most 1e16" ) );
            ASSERT_TRUE( ValidatesAs( maximum, "10000000000000001", false, "1e16 + 1 is more than 1e16" ) );

            CJsonSchema minimum;
            ASSERT_TRUE( CompileText( "{'minimum':1e16}", minimum, error ) );
            ASSERT_TRUE( ValidatesAs( minimum, "9999999999999999", false, "1e16 - 1 is less than 1e16" ) );

            CJsonSchema exclusive;
            ASSERT_TRUE( CompileText( "{'exclusiveMaximum':9007199254740993,'exclusiveMinimum':-9.007199254740992e15}", exclusive, error ) );
            ASSERT_TRUE( ValidatesAs( exclusive, "9007199254740992", true, "2^53 is below 2^53 + 1" ) );
            ASSERT_TRUE( ValidatesAs( exclusive, "9007199254740993", false, "2^53 + 1 is not below itself" ) );
            ASSERT_TRUE( ValidatesAs( exclusive, "-9007199254740991", true, "-(2^53) + 1 is above -(2^53)" ) );
            ASSERT_TRUE( ValidatesAs( exclusive, "-9007199254740992", false, "-(2^53) is not above itself" ) );

            CJsonSchema constant;
            ASSERT_TRUE( CompileText( "{'const':1e16}", constant, error ) );
            ASSERT_TRUE( ValidatesAs( constant, "10000000000000000", true, "1e16 equals 10000000000000000" ) );
            ASSERT_TRUE( ValidatesAs( constant, "10000000000000001", false, "1e16 is not 10000000000000001" ) );

            CJsonSchema multiple;
            ASSERT_TRUE( CompileText( "{'multipleOf':1e1}", multiple, error ) );
            ASSERT_TRUE( ValidatesAs( multiple, "10000000000000000", true, "1e16 is a multiple of 10" ) );
            ASSERT_TRUE( ValidatesAs( multiple, "10000000000000001", false, "1e16 + 1 is no multiple of 10" ) );

            // a fraction still compares as before
            CJsonSchema fraction;
            ASSERT_TRUE( CompileText( "{'maximum':2.5}", fraction, error ) );
            ASSERT_TRUE( ValidatesAs( fraction, "2", true, "2 is at most 2.5" ) );
            ASSERT_TRUE( ValidatesAs( fraction, "3", false, "3 is more than 2.5" ) );
            ASSERT_TRUE( ValidatesAs( fraction, "10000000000000001", false, "a big integer is more than 2.5" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformErrorReportTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON Schema errors: the instance path points at the value that fails" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'type':'object','properties':{'a':{'type':'array','items':{'type':'object','properties':{'b':{'type':'integer'}}}}}}", schema, error ) );
            CDataNode instance;
            ASSERT_TRUE( ParseValue( "{'a':[{'b':1},{'b':'x'}]}", instance ) );
            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            ASSERT_TRUE( 1 == errors.size() );
            if ( 1 == errors.size() )
            {
                ASSERT_TRUE( "/a/1/b" == errors[ 0 ].instancePath );
                ASSERT_TRUE( "type" == errors[ 0 ].keyword );
                ASSERT_TRUE( "must be of type integer, but is string" == errors[ 0 ].message );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema errors: member names in paths are escaped as JSON pointers" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'properties':{'a/b':{'type':'integer'},'c~d':{'type':'integer'}}}", schema, error ) );
            CDataNode instance;
            ASSERT_TRUE( ParseValue( "{'a/b':'x','c~d':'y'}", instance ) );
            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            ASSERT_TRUE( 2 == errors.size() );
            if ( 2 == errors.size() )
            {
                ASSERT_TRUE( "/a~1b" == errors[ 0 ].instancePath || "/a~1b" == errors[ 1 ].instancePath );
                ASSERT_TRUE( "/c~0d" == errors[ 0 ].instancePath || "/c~0d" == errors[ 1 ].instancePath );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema errors: the messages say what to fix" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'type':'object','properties':{'name':{'type':'string','minLength':3},'size':{'enum':['S','M','L']},"
                                      "'count':{'type':'integer','minimum':1,'maximum':10,'multipleOf':2},'tags':{'type':'array','maxItems':2,'uniqueItems':true},"
                                      "'kind':{'const':'book'},'any':{'anyOf':[{'type':'string'},{'type':'integer'}]},'one':{'oneOf':[{'minimum':0},{'maximum':10}]},"
                                      "'neg':{'not':{'type':'null'}}},'required':['name','id'],'dependentRequired':{'count':['size']},'additionalProperties':false}", schema, error ) );
            CDataNode instance;
            ASSERT_TRUE( ParseValue( "{'name':'ab','size':'XL','count':11,'tags':['a','a','b'],'kind':'film','any':true,'one':5,'neg':null,'extra':1}", instance ) );
            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors, 100 ) );
            GUCEF_LOG( LOGLEVEL_NORMAL, "JsonSchema test: the errors: " << ErrorsText( errors ) );
            ASSERT_TRUE( HasErrorWith( errors, "must be at least 3 characters long, but is 2" ) );
            ASSERT_TRUE( HasErrorWith( errors, "must be one of the allowed values: \"S\", \"M\", \"L\"" ) );
            ASSERT_TRUE( HasErrorWith( errors, "must be <= 10" ) );
            ASSERT_TRUE( HasErrorWith( errors, "must be a multiple of 2" ) );
            ASSERT_TRUE( HasErrorWith( errors, "must have at most 2 items, but has 3" ) );
            ASSERT_TRUE( HasErrorWith( errors, "the items 0 and 1 are equal" ) );
            ASSERT_TRUE( HasErrorWith( errors, "must be the constant value \"book\"" ) );
            ASSERT_TRUE( HasErrorWith( errors, "must match at least one of the 2 schemas of anyOf" ) );
            ASSERT_TRUE( HasErrorWith( errors, "must match exactly one of the 2 schemas of oneOf, but matches both 0 and 1" ) );
            ASSERT_TRUE( HasErrorWith( errors, "must not match the schema of not" ) );
            ASSERT_TRUE( HasErrorWith( errors, "the required property \"id\" is missing" ) );
            ASSERT_TRUE( HasErrorWith( errors, "the property \"extra\" is not allowed" ) );

            // the error for a missing property is at the object, the one for a wrong value at the value
            bool requiredAtRoot = false;
            bool sizeAtValue = false;
            for ( UInt32 i=0; i<errors.size(); ++i )
            {
                if ( "required" == errors[ i ].keyword && errors[ i ].instancePath.IsNULLOrEmpty() )
                    requiredAtRoot = true;
                if ( "enum" == errors[ i ].keyword && "/size" == errors[ i ].instancePath )
                    sizeAtValue = true;
            }
            ASSERT_TRUE( requiredAtRoot );
            ASSERT_TRUE( sizeAtValue );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema errors: dependentRequired, minProperties, items false and the false schema" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'dependentRequired':{'count':['size']},'minProperties':3,'properties':{'t':{'prefixItems':[{}],'items':false},'f':false}}", schema, error ) );
            CDataNode instance;
            ASSERT_TRUE( ParseValue( "{'count':1,'t':[1,2,3],'f':0}", instance ) );
            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            GUCEF_LOG( LOGLEVEL_NORMAL, "JsonSchema test: the errors: " << ErrorsText( errors ) );
            ASSERT_TRUE( HasErrorWith( errors, "the property \"size\" is required when the property \"count\" is present" ) );
            ASSERT_TRUE( HasErrorWith( errors, "must have at most 1 items, but has 3" ) );
            ASSERT_TRUE( HasErrorWith( errors, "no value is allowed here" ) );

            ASSERT_TRUE( CompileText( "{'minProperties':3}", schema, error ) );
            ASSERT_TRUE( ParseValue( "{'a':1}", instance ) );
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            ASSERT_TRUE( HasErrorWith( errors, "must have at least 3 properties, but has 1" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema errors: a schema that is not compiled validates nothing" )
        try
        {
            CJsonSchema schema;
            CDataNode instance;
            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            ASSERT_TRUE( HasErrorWith( errors, "not compiled" ) );

            CString error;
            ASSERT_FALSE( CompileText( "{'typo':1}", schema, error ) );
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

namespace {

/** Validates the same schema many times on a thread of its own */
class CValidationThread : public MT::CActiveObject
{
    public:

    CValidationThread( const CJsonSchema& schema, const CDataNode& good, const CDataNode& bad )
        : MT::CActiveObject()
        , m_schema( schema )
        , m_good( good )
        , m_bad( bad )
        , m_wrongAnswers( 0 )
    {
    }

    virtual ~CValidationThread() GUCEF_VIRTUAL_OVERRIDE
    {
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return this;
    }

    UInt32 GetWrongAnswers( void ) const
    {
        return m_wrongAnswers;
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        CJsonSchemaValidator::TErrorVector errors;
        for ( UInt32 i=0; i<300; ++i )
        {
            if ( !CJsonSchemaValidator::Validate( m_schema, m_good, errors ) || !errors.empty() )
                ++m_wrongAnswers;
            if ( CJsonSchemaValidator::Validate( m_schema, m_bad, errors ) || errors.empty() )
                ++m_wrongAnswers;
        }
        return true;
    }

    private:

    const CJsonSchema& m_schema;
    const CDataNode& m_good;
    const CDataNode& m_bad;
    UInt32 m_wrongAnswers;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

static void
PerformUsageTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON Schema use: a schema and an instance built in code, without the json codec" )
        try
        {
            CDataNode schemaNode;
            schemaNode.SetAttribute( "type", "object" );
            schemaNode.SetAttribute( CString( "additionalProperties" ), false );
            CDataNode* properties = schemaNode.AddChild( "properties" );
            CDataNode* count = properties->AddChild( "count" );
            count->SetAttribute( "type", "integer" );
            count->SetAttribute( CString( "minimum" ), (Int32) 0 );
            CDataNode* ratio = properties->AddChild( "ratio" );
            ratio->SetAttribute( "type", "number" );
            ratio->SetAttribute( CString( "exclusiveMaximum" ), (Int32) 1 );
            properties->AddChild( "flag" )->SetAttribute( "type", "boolean" );
            properties->AddChild( "big" )->SetAttribute( "type", "integer" );
            CDataNode* list = properties->AddChild( "list" );
            list->SetAttribute( "type", "array" );
            list->AddChild( "items" )->SetAttribute( "type", "string" );
            CDataNode* required = schemaNode.AddChild( "required", GUCEF_DATATYPE_ARRAY );
            required->AddValueAsChild( CString( "count" ) );

            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( schema.Compile( schemaNode, error ) );

            CDataNode instance;
            instance.SetAttribute( CString( "count" ), (UInt32) 3 );
            instance.SetAttribute( CString( "ratio" ), (Float32) 0.5f );
            instance.SetAttribute( CString( "flag" ), true );
            instance.SetAttribute( CString( "big" ), (UInt64) 18446744073709551615ULL );
            CDataNode* items = instance.AddChild( "list", GUCEF_DATATYPE_ARRAY );
            items->AddValueAsChild( CString( "a" ) );
            items->AddValueAsChild( CString( "b" ) );
            CJsonSchemaValidator::TErrorVector errors;
            ASSERT_TRUE( CJsonSchemaValidator::Validate( schema, instance, errors ) );

            instance.SetAttribute( CString( "count" ), (Int64) -1 );
            items->AddValueAsChild( CVariant( (Int32) 5 ) );
            ASSERT_FALSE( CJsonSchemaValidator::Validate( schema, instance, errors ) );
            ASSERT_TRUE( 2 == errors.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

}

/*-------------------------------------------------------------------------*/

static void
PerformCopyAndThreadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "JSON Schema use: a copy compiles its own copy of the schema" )
        try
        {
            CJsonSchema* original = GUCEF_NEW CJsonSchema();
            CString error;
            ASSERT_TRUE( CompileText( "{'$defs':{'n':{'type':'integer'}},'properties':{'a':{'$ref':'#/$defs/n'}}}", *original, error ) );
            CJsonSchema copy( *original );
            CJsonSchema assigned;
            assigned = *original;
            UInt32 subschemaCount = original->GetSubschemaCount();
            GUCEF_DELETE original;

            ASSERT_TRUE( copy.IsCompiled() && assigned.IsCompiled() );
            ASSERT_TRUE( subschemaCount == copy.GetSubschemaCount() && subschemaCount == assigned.GetSubschemaCount() );
            ASSERT_TRUE( ValidatesAs( copy, "{'a':1}", true, "copy valid" ) );
            ASSERT_TRUE( ValidatesAs( copy, "{'a':'x'}", false, "copy invalid" ) );
            ASSERT_TRUE( ValidatesAs( assigned, "{'a':'x'}", false, "assigned invalid" ) );

            CJsonSchema empty;
            assigned = empty;
            ASSERT_FALSE( assigned.IsCompiled() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON Schema use: a compiled schema validates on several threads at once" )
        try
        {
            CJsonSchema schema;
            CString error;
            ASSERT_TRUE( CompileText( "{'type':'object','properties':{'a':{'type':'array','items':{'enum':[1,2,{'x':[true]}]},'uniqueItems':true}},'required':['a']}", schema, error ) );
            CDataNode good, bad;
            ASSERT_TRUE( ParseValue( "{'a':[1,2,{'x':[true]}]}", good ) );
            ASSERT_TRUE( ParseValue( "{'a':[1,1,{'x':[false]}]}", bad ) );

            const UInt32 threadCount = 4;
            CValidationThread* threads[ threadCount ];
            for ( UInt32 i=0; i<threadCount; ++i )
            {
                threads[ i ] = GUCEF_NEW CValidationThread( schema, good, bad );
                ASSERT_TRUE( threads[ i ]->Activate( GUCEF_NULL, 0, 0 ) );
            }
            UInt32 wrongAnswers = 0;
            for ( UInt32 i=0; i<threadCount; ++i )
            {
                UInt32 waited = 0;
                while ( !threads[ i ]->HasThreadEnded() && waited < 60000 )
                {
                    MT::ThreadDelay( 10 );
                    waited += 10;
                }
                ASSERT_TRUE( threads[ i ]->HasThreadEnded() );
                threads[ i ]->Deactivate( false, true );
                wrongAnswers += threads[ i ]->GetWrongAnswers();
                GUCEF_DELETE threads[ i ];
            }
            ASSERT_TRUE( 0 == wrongAnswers );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

void
PerformJsonSchemaTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING JSON Schema TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "JsonSchema" );

    PerformUsageTests();

    if ( !JsonTestCodecAvailable() )
    {
        GUCEF_TESTFW_TESTCASE( "JSON Schema: the json codec is available for the keyword tests" )
            ASSERT_TRUE( JsonTestCodecAvailable() );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }

    RunGroups( g_groups, CASE_COUNT( g_groups ) );
    PerformCopyAndThreadTests();
    PerformRefusalTests();
    PerformLimitTests();
    PerformErrorReportTests();
}

/*-------------------------------------------------------------------------*/
