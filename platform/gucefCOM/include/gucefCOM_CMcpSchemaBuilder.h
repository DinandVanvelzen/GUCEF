/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPSCHEMABUILDER_H
#define GUCEF_COM_CMCPSCHEMABUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Builds the JSON Schemas (dialect 2020-12) of tool inputs and outputs (CMcpToolDescriptor::inputSchema and
 *  outputSchema) as CDataNode trees in the conventions of the json codec and CORE::CJsonSchema, so that tools do not
 *  write schema text by hand:
 *
 *  - objects refuse undeclared members by default (additionalProperties false): a model that invents an argument
 *    gets an invalid argument error instead of being silently ignored;
 *  - strings, lists and numbers carry their bounds, so that the server's argument validation enforces the limits a
 *    tool relies on;
 *  - an untrusted text object ({ "untrusted": true, "source", "text", "truncated" }) and the x-gucef-untrusted marker
 *    for members whose text comes from outside the server, so that clients and reviewers can tell data from
 *    instructions;
 *  - every schema built here compiles in CORE::CJsonSchema (Compile() checks a finished schema).
 *
 *  A bound of 0 means "no bound" where the description of the function says so.
 */
class GUCEF_COM_PUBLIC_CPP CMcpSchemaBuilder
{
    public:

    static const CORE::CString UntrustedMarker;         /**< "x-gucef-untrusted" */

    /** { "type": "object", "additionalProperties": additionalProperties } with an optional description */
    static CORE::CDataNode Object( const CORE::CString& description = CORE::CString::Empty, bool additionalProperties = false );

    /**
     *  Adds (or replaces) a member of an object schema under "properties", and lists it in "required" when it is
     *  @return false when the schema is no object node or the name is empty
     */
    static bool AddProperty( CORE::CDataNode& objectSchema            ,
                             const CORE::CString& name                ,
                             const CORE::CDataNode& propertySchema    ,
                             bool required = false                    );

    /** { "type": "string" } with minLength (0: none) and maxLength (0: none) */
    static CORE::CDataNode String( UInt32 minLength = 0, UInt32 maxLength = 0, const CORE::CString& description = CORE::CString::Empty );

    /** { "type": "integer", "minimum", "maximum" } */
    static CORE::CDataNode Integer( Int64 minimum, Int64 maximum, const CORE::CString& description = CORE::CString::Empty );

    /** { "type": "integer" } without bounds */
    static CORE::CDataNode UnboundedInteger( const CORE::CString& description = CORE::CString::Empty );

    /** { "type": "number", "minimum", "maximum" } */
    static CORE::CDataNode Number( Float64 minimum, Float64 maximum, const CORE::CString& description = CORE::CString::Empty );

    /** { "type": "number" } without bounds */
    static CORE::CDataNode UnboundedNumber( const CORE::CString& description = CORE::CString::Empty );

    /** { "type": "boolean" }, with a default when hasDefault */
    static CORE::CDataNode Boolean( const CORE::CString& description = CORE::CString::Empty, bool hasDefault = false, bool defaultValue = false );

    /** { "type": "string", "enum": [ values ] }: the model cannot invent a value */
    static CORE::CDataNode Enum( const CORE::CString::StringVector& values, const CORE::CString& description = CORE::CString::Empty );

    /** { "const": value } */
    static CORE::CDataNode Const( const CORE::CVariant& value, const CORE::CString& description = CORE::CString::Empty );

    /** { "type": "null" } */
    static CORE::CDataNode Null( void );

    /** { "type": "array", "items": items, "minItems" (0: none), "maxItems" (0: none) } */
    static CORE::CDataNode Array( const CORE::CDataNode& items, UInt32 minItems = 0, UInt32 maxItems = 0, const CORE::CString& description = CORE::CString::Empty );

    /** A list of strings with caps on the number of elements (0: none) and on the length of each (0: none) */
    static CORE::CDataNode StringList( UInt32 maxItems, UInt32 maxLength, const CORE::CString& description = CORE::CString::Empty );

    /** { "anyOf": [ schema, { "type": "null" } ] }: the value or null */
    static CORE::CDataNode Nullable( const CORE::CDataNode& schema );

    /** { "oneOf": [ ... ] } of two to four alternatives; more through OneOfList() */
    static CORE::CDataNode OneOf( const CORE::CDataNode& first, const CORE::CDataNode& second );
    static CORE::CDataNode OneOfList( const GUCEF::vector< CORE::CDataNode >& alternatives );

    /** { "anyOf": [ ... ] } */
    static CORE::CDataNode AnyOfList( const GUCEF::vector< CORE::CDataNode >& alternatives );

    /** { "$ref": pointer }, a local reference such as "#/$defs/range" */
    static CORE::CDataNode Ref( const CORE::CString& pointer );

    /** Adds a schema under "$defs" of a root schema, for Ref( "#/$defs/<name>" ) */
    static bool AddDefinition( CORE::CDataNode& rootSchema, const CORE::CString& name, const CORE::CDataNode& schema );

    /**
     *  The untrusted text object: { "untrusted": true, "source": <one of sources, or any string when empty>,
     *  "text": <at most maxChars, 0: no cap>, "truncated": boolean }, all required, no other members, marked with
     *  x-gucef-untrusted: true
     */
    static CORE::CDataNode UntrustedText( UInt32 maxChars, const CORE::CString::StringVector& sources = CORE::CString::StringVector(), const CORE::CString& description = CORE::CString::Empty );

    /** Fills an untrusted text object value as the schema of UntrustedText() describes it */
    static CORE::CDataNode MakeUntrustedTextValue( const CORE::CString& source, const CORE::CString& text, UInt32 maxChars );

    /** Marks a schema as one whose text comes from outside the server (x-gucef-untrusted: true) */
    static void MarkUntrusted( CORE::CDataNode& schema );

    static void SetDescription( CORE::CDataNode& schema, const CORE::CString& description );

    static void SetDefault( CORE::CDataNode& schema, const CORE::CVariant& value );

    /** Whether the schema compiles in CORE::CJsonSchema; when not, why */
    static bool Compile( const CORE::CDataNode& schema, CORE::CString& error );

    private:

    CMcpSchemaBuilder( void );                                         /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPSCHEMABUILDER_H ? */
