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

#ifndef GUCEF_CORE_CJSONSCHEMAVALIDATOR_H
#define GUCEF_CORE_CJSONSCHEMAVALIDATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJSONSCHEMA_H
#include "gucefCORE_CJsonSchema.h"
#define GUCEF_CORE_CJSONSCHEMA_H
#endif /* GUCEF_CORE_CJSONSCHEMA_H ? */

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

/**
 *  Validates a JSON value held in a CDataNode tree (see CJsonSchema for the conventions) against a compiled schema.
 *
 *  Validation never throws and is bounded: it stops after maxErrors errors or maxSteps keyword evaluations, and refuses
 *  to nest deeper than MaxEvaluationDepth. A validation that runs out of steps or nesting fails with one more error,
 *  with an empty keyword and a message that starts with "schema too expensive".
 */
class GUCEF_CORE_PUBLIC_CPP CJsonSchemaValidator
{
    public:

    /** One reason why the instance does not match */
    struct SError
    {
        CString instancePath;   /**< JSON pointer of the value that fails, "" for the instance itself, "/a/0/b" below it */
        CString keyword;        /**< the keyword that failed; "false" for the false schema, empty when the budget ran out */
        CString message;        /**< what is wrong, phrased so that a person or a language model can fix the value */
    };
    typedef GUCEF::vector< SError > TErrorVector;

    static const UInt32 DefaultMaxErrors = 16;
    static const UInt32 DefaultMaxSteps = 100000;

    /** How deep evaluation may nest: subschemas applied to values inside values, references included */
    static const UInt32 MaxEvaluationDepth = 512;

    /**
     *  Validates an instance; stops after maxErrors errors or maxSteps keyword evaluations (the time budget).
     *  With maxErrors 0 no errors are collected and the first failure ends the validation.
     *
     *  @param errors   cleared, then filled with the reasons when the instance does not match
     *  @return true when the instance matches the schema
     */
    static bool Validate( const CJsonSchema& schema                 ,
                          const CDataNode& instance                 ,
                          TErrorVector& errors                      ,
                          UInt32 maxErrors = DefaultMaxErrors       ,
                          UInt32 maxSteps = DefaultMaxSteps         );

    private:

    struct SRun;
    typedef CJsonSchema::SValueRef SValueRef;
    typedef CJsonSchema::SSubschema SSubschema;

    static bool ValidateSubschema( SRun& run, UInt32 index, const SValueRef& value );

    static bool ValidateNumber( SRun& run, const SSubschema& subschema, const SValueRef& value );

    static bool ValidateString( SRun& run, const SSubschema& subschema, const SValueRef& value );

    static bool ValidateObject( SRun& run, const SSubschema& subschema, const SValueRef& value );

    static bool ValidateMember( SRun& run, const SSubschema& subschema, const CString& name, const SValueRef& member );

    static bool ValidateArray( SRun& run, const SSubschema& subschema, const SValueRef& value );

    static bool ValidateComposition( SRun& run, const SSubschema& subschema, const SValueRef& value );

    static bool ValidateSilently( SRun& run, UInt32 index, const SValueRef& value );

    static bool JsonEquals( SRun& run, const SValueRef& a, const SValueRef& b, UInt32 depth );

    /** A member of an object by name: an attribute by its map, an object or array member through the index of the run */
    static bool FindObjectMember( SRun& run, const CDataNode& object, const CString& name, SValueRef& member );

    /** A member name that occurs more than once in the object, GUCEF_NULL when every name is unique */
    static const CString* FindRepeatedMemberName( SRun& run, const CDataNode& object );

    static bool Step( SRun& run );

    static void StopForBudget( SRun& run, const CString& message );

    static void AddError( SRun& run, const char* keyword, const CString& message );

    static CString Describe( const SValueRef& value );

    static const char* TypeNameOf( const SValueRef& value );

    CJsonSchemaValidator( void );                                           /**< not implemented: static members only */
    CJsonSchemaValidator( const CJsonSchemaValidator& src );                /**< not implemented */
    CJsonSchemaValidator& operator=( const CJsonSchemaValidator& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJSONSCHEMAVALIDATOR_H ? */
