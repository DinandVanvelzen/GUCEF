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

#ifndef GUCEF_CORE_CJSONSCHEMA_H
#define GUCEF_CORE_CJSONSCHEMA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CJsonSchemaValidator;

/*-------------------------------------------------------------------------*/

/**
 *  A JSON Schema (dialect 2020-12) compiled for validation with CJsonSchemaValidator.
 *
 *  The schema is a CDataNode tree in the JSON conventions of the json dstore codec: a JSON object is a node whose
 *  scalar members are attributes and whose object and array members are child nodes, a JSON array is a node of type
 *  GUCEF_DATATYPE_ARRAY whose scalar elements are child nodes holding a value, and null is the nil type.
 *
 *  Supported keywords:
 *      - types and values: type, enum, const
 *      - objects: properties, required, additionalProperties, propertyNames, minProperties, maxProperties, dependentRequired
 *      - arrays: items, prefixItems, minItems, maxItems, uniqueItems
 *      - strings: minLength, maxLength (in code points)
 *      - numbers: minimum, maximum, exclusiveMinimum, exclusiveMaximum, multipleOf
 *      - composition: allOf, anyOf, oneOf, not, if, then, else
 *      - references: $defs and $ref with a local reference: "#" or "#" followed by a JSON pointer
 *  Kept and not checked (annotations): title, description, default, examples, format, deprecated, readOnly, writeOnly,
 *  contentEncoding, contentMediaType, contentSchema, $comment, $schema, $id (at the root only) and extensions whose name
 *  starts with "x-".
 *
 *  Compilation refuses what it cannot fully check, so a compiled schema never validates less than it says:
 *      - a $schema other than 2020-12 ("dialect not supported");
 *      - a $ref that is not local: network references are never dereferenced;
 *      - a $ref that does not point at a schema of the same document, or a plain-name fragment ($anchor);
 *      - $id below the root, since it would change what local references resolve to;
 *      - keywords that are unknown, or known and not supported (pattern and patternProperties need a regular
 *        expression engine, which gucefCORE does not have; contains, dependentSchemas, unevaluated*, $anchor, $dynamic*);
 *      - keyword values of the wrong JSON type;
 *      - nesting deeper than maxDepth: subschemas and the values of enum and const;
 *      - more than maxSubschemas subschemas;
 *      - a cycle of references that never descends into the instance, which could only recurse forever.
 *
 *  A compiled schema is read only. Validating with it from several threads at once is safe.
 */
class GUCEF_CORE_PUBLIC_CPP CJsonSchema
{
    public:

    /** The dialect a $schema keyword may name: "https://json-schema.org/draft/2020-12/schema" */
    static const CString Dialect2020_12;

    static const UInt32 DefaultMaxDepth = 32;
    static const UInt32 DefaultMaxSubschemas = 1024;

    CJsonSchema( void );

    /** Copies the source schema and compiles the copy with the same limits */
    CJsonSchema( const CJsonSchema& src );

    ~CJsonSchema();

    CJsonSchema& operator=( const CJsonSchema& src );

    /**
     *  Compiles a schema. A copy of the schema is kept, the original may change or go away afterwards.
     *  Fails on: a $schema other than 2020-12, a $ref that is not local ("#..."), unknown or unsupported keywords,
     *  depth over maxDepth, more than maxSubschemas subschemas; see the class description.
     *
     *  @param error  on failure: what is wrong and where, as "#/json/pointer/into/the/schema: reason"
     *  @return true when the schema compiled; on failure the object is left without a compiled schema
     */
    bool Compile( const CDataNode& schema                       ,
                  CString& error                                ,
                  UInt32 maxDepth = DefaultMaxDepth             ,
                  UInt32 maxSubschemas = DefaultMaxSubschemas   );

    bool IsCompiled( void ) const;

    /** Forgets the compiled schema */
    void Clear( void );

    /** The number of subschemas the compiled schema consists of, the root and boolean schemas included */
    UInt32 GetSubschemaCount( void ) const;

    /** The copy of the schema that was compiled */
    const CDataNode& GetSchema( void ) const;

    private:
    friend class CJsonSchemaValidator;

    /** Index value meaning "no subschema" */
    static const UInt32 NoSubschema = 0xFFFFFFFF;

    /** The JSON kinds of a value. A number with a zero fraction is a NUMBER, the type "integer" still accepts it */
    enum EJsonKind
    {
        JSONKIND_NULL = 0,
        JSONKIND_BOOLEAN,
        JSONKIND_INTEGER,
        JSONKIND_NUMBER,
        JSONKIND_STRING,
        JSONKIND_OBJECT,
        JSONKIND_ARRAY,
        JSONKIND_UNSUPPORTED        /**< a value of a type JSON does not have, binary data for example */
    };

    /** The bits of a "type" keyword */
    enum ETypeBit
    {
        TYPEBIT_NULL    = 0x01,
        TYPEBIT_BOOLEAN = 0x02,
        TYPEBIT_OBJECT  = 0x04,
        TYPEBIT_ARRAY   = 0x08,
        TYPEBIT_NUMBER  = 0x10,
        TYPEBIT_INTEGER = 0x20,
        TYPEBIT_STRING  = 0x40
    };

    /**
     *  A JSON value inside a CDataNode tree: an object or an array is a node, a scalar is a variant (an attribute
     *  value or the value of a node). Exactly one of the two is set
     */
    struct SValueRef
    {
        const CDataNode* node;
        const CVariant* scalar;

        SValueRef( void );
        SValueRef( const CDataNode* valueNode, const CVariant* scalarValue );
    };

    typedef GUCEF::vector< SValueRef >          TValueRefVector;
    typedef GUCEF::vector< UInt32 >             TIndexVector;
    typedef GUCEF::vector< CString >            TStringVector;
    typedef GUCEF::map< CString, UInt32 >       TStringIndexMap;
    typedef GUCEF::map< CString, TStringVector > TStringListMap;

    /** One compiled subschema. Indexes refer to other entries of m_subschemas */
    struct SSubschema
    {
        bool isBooleanSchema;
        bool booleanValue;

        UInt32 typeMask;                    /**< 0 when there is no "type" keyword */
        bool hasConst;
        SValueRef constValue;
        bool hasEnum;
        TValueRefVector enumValues;

        UInt32 ref;                         /**< the subschema "$ref" resolves to */
        CString refText;

        bool hasMinimum;
        bool hasMaximum;
        bool hasExclusiveMinimum;
        bool hasExclusiveMaximum;
        bool hasMultipleOf;
        CVariant minimum;
        CVariant maximum;
        CVariant exclusiveMinimum;
        CVariant exclusiveMaximum;
        CVariant multipleOf;

        bool hasMinLength;
        bool hasMaxLength;
        UInt64 minLength;
        UInt64 maxLength;

        TStringIndexMap properties;
        TStringVector required;
        UInt32 additionalProperties;
        UInt32 propertyNames;
        bool hasMinProperties;
        bool hasMaxProperties;
        UInt64 minProperties;
        UInt64 maxProperties;
        TStringListMap dependentRequired;

        UInt32 items;
        TIndexVector prefixItems;
        bool hasMinItems;
        bool hasMaxItems;
        UInt64 minItems;
        UInt64 maxItems;
        bool uniqueItems;

        TIndexVector allOf;
        TIndexVector anyOf;
        TIndexVector oneOf;
        UInt32 notSchema;
        UInt32 ifSchema;
        UInt32 thenSchema;
        UInt32 elseSchema;

        SSubschema( void );
    };

    typedef GUCEF::vector< SSubschema > TSubschemaVector;

    struct SCompileContext;

    /** The JSON kind of a value */
    static EJsonKind KindOf( const SValueRef& value );

    /** The JSON kind of a scalar */
    static EJsonKind KindOfScalar( const CVariant& value );

    /** A child node of an object or an array as the JSON value it holds */
    static SValueRef ChildAsValue( const CDataNode& child );

    /** Looks up the member with the given name of an object node */
    static bool FindMember( const CDataNode& object, const CString& name, SValueRef& member );

    /** The number of members of an object node */
    static UInt32 CountMembers( const CDataNode& object );

    /** Escapes a member name or index for use as a JSON pointer token: '~' becomes "~0" and '/' becomes "~1" */
    static CString EscapePointerToken( const CString& token );

    bool CompileSubschema( SCompileContext& context      ,
                           const SValueRef& value        ,
                           const CString& pointer        ,
                           UInt32 depth                  ,
                           UInt32& index                 );

    bool CompileKeyword( SCompileContext& context      ,
                         UInt32 index                  ,
                         const CString& keyword        ,
                         const SValueRef& value        ,
                         const CString& pointer        ,
                         UInt32 depth                  );

    bool CompileSchemaArray( SCompileContext& context      ,
                             const SValueRef& value        ,
                             const CString& pointer        ,
                             UInt32 depth                  ,
                             TIndexVector& indexes         );

    bool CompileSchemaMap( SCompileContext& context      ,
                           const SValueRef& value        ,
                           const CString& pointer        ,
                           UInt32 depth                  ,
                           TStringIndexMap* indexes      );

    bool ResolveReferences( SCompileContext& context );

    bool CheckReferenceCycles( SCompileContext& context );

    void GetInPlaceApplicators( UInt32 index, TIndexVector& targets ) const;

    bool CompileFromSource( CString& error );

    private:

    CDataNode m_source;
    TSubschemaVector m_subschemas;
    bool m_compiled;
    UInt32 m_maxDepth;
    UInt32 m_maxSubschemas;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJSONSCHEMA_H ? */
