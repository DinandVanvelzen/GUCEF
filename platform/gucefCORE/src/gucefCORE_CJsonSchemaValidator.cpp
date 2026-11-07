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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CJsonSchemaValidator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** One step of the path from the instance to the value being validated: a member name or an array index */
struct SPathSegment
{
    const CString* name;
    UInt32 index;
};
typedef GUCEF::vector< SPathSegment > TPath;

/*-------------------------------------------------------------------------*/

/** Adds a step to the path for as long as it exists */
class CPathScope
{
    public:

    CPathScope( TPath& path, const CString* name )
        : m_path( path )
    {
        SPathSegment segment;
        segment.name = name;
        segment.index = 0;
        m_path.push_back( segment );
    }

    CPathScope( TPath& path, UInt32 index )
        : m_path( path )
    {
        SPathSegment segment;
        segment.name = GUCEF_NULL;
        segment.index = index;
        m_path.push_back( segment );
    }

    ~CPathScope()
    {
        m_path.pop_back();
    }

    private:

    CPathScope( const CPathScope& src );
    CPathScope& operator=( const CPathScope& src );

    TPath& m_path;
};

/*-------------------------------------------------------------------------*/

/** Raises a counter for as long as it exists */
class CCounterScope
{
    public:

    explicit CCounterScope( UInt32& counter )
        : m_counter( counter )
    {
        ++m_counter;
    }

    ~CCounterScope()
    {
        --m_counter;
    }

    private:

    CCounterScope( const CCounterScope& src );
    CCounterScope& operator=( const CCounterScope& src );

    UInt32& m_counter;
};

/*-------------------------------------------------------------------------*/

/** A JSON number: an integer exactly as sign and magnitude, or a floating point number */
struct SNumber
{
    bool isInteger;
    bool negative;
    UInt64 magnitude;
    Float64 real;
};

/*-------------------------------------------------------------------------*/

/**
 *  The child members of an object sorted by name, so that a look-up costs O(log n) and not O(n): an object with many
 *  members would otherwise make every look-up, and the equality of two such objects, quadratic
 */
struct SMemberIndex
{
    GUCEF::vector< const CDataNode* > children;     /**< sorted by name; equal names keep their document order */
    const CString* repeatedName;                    /**< a name that occurs more than once, among the children or also as an attribute */

    SMemberIndex( void ) : children(), repeatedName( GUCEF_NULL ) {}
};

/** Orders child nodes by name, for the member index */
struct SChildNameLess
{
    bool operator()( const CDataNode* a, const CDataNode* b ) const
    {
        return a->GetName() < b->GetName();
    }
};

/** Below this many child members a look-up scans them: an index would cost more than it saves */
static const UInt32 MinChildrenForIndex = 8;

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

/** State of one validation */
struct CJsonSchemaValidator::SRun
{
    /**
     *  The member index of every object that needed one, made once per object and validation: what it costs is in
     *  proportion to the size of the instance, whatever the schema asks, so the step budget does not have to count it
     */
    typedef GUCEF::map< const CDataNode*, SMemberIndex > TMemberIndexMap;

    const CJsonSchema& schema;
    TErrorVector& errors;
    UInt32 maxErrors;
    UInt32 maxSteps;
    UInt32 steps;
    UInt32 depth;
    UInt32 silent;              /**< > 0 while a result is only looked at, not reported: anyOf, oneOf, not, if */
    bool full;                  /**< maxErrors errors were collected */
    bool outOfBudget;           /**< the steps or the nesting ran out */
    CString budgetMessage;
    CString budgetPath;
    TPath path;
    TMemberIndexMap memberIndexes;

    SRun( const CJsonSchema& compiledSchema, TErrorVector& errorList, UInt32 errorLimit, UInt32 stepLimit )
        : schema( compiledSchema )
        , errors( errorList )
        , maxErrors( errorLimit )
        , maxSteps( stepLimit )
        , steps( 0 )
        , depth( 0 )
        , silent( 0 )
        , full( false )
        , outOfBudget( false )
        , budgetMessage()
        , budgetPath()
        , path()
        , memberIndexes()
    {
    }

    bool Stopped( void ) const
    {
        return full || outOfBudget;
    }

    private:

    SRun( const SRun& src );
    SRun& operator=( const SRun& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CString
Quoted( const CString& text )
{GUCEF_TRACE;

    return "\"" + text + "\"";
}

/*-------------------------------------------------------------------------*/

static CString
PathText( const TPath& path )
{GUCEF_TRACE;

    CString text;
    for ( UInt32 i=0; i<path.size(); ++i )
    {
        text += "/";
        if ( GUCEF_NULL != path[ i ].name )
        {
            const CString& name = *path[ i ].name;
            if ( name.HasChar( '~' ) < 0 && name.HasChar( '/' ) < 0 )
                text += name;
            else
                text += name.ReplaceSubstr( "~", "~0" ).ReplaceSubstr( "/", "~1" );
        }
        else
        {
            text += UInt32ToString( path[ i ].index );
        }
    }
    return text;
}

/*-------------------------------------------------------------------------*/

static void
ToNumber( const CVariant& value, SNumber& number )
{GUCEF_TRACE;

    if ( value.IsInteger() )
    {
        number.isInteger = true;
        if ( value.IsUnsignedInteger() )
        {
            number.negative = false;
            number.magnitude = value.AsUInt64();
        }
        else
        {
            Int64 signedValue = value.AsInt64();
            number.negative = signedValue < 0;
            number.magnitude = signedValue < 0 ? ( (UInt64) ( -( signedValue + 1 ) ) ) + 1 : (UInt64) signedValue;
        }
        number.real = number.negative ? -( (Float64) number.magnitude ) : (Float64) number.magnitude;
    }
    else
    {
        // a whole number written with a fraction or an exponent (2.0, 1e16) is an integer too, compared exactly: as a
        // double, 1e16 and 10000000000000001 would be equal. Beyond 2^64 it stays a double
        number.real = value.AsFloat64();
        const bool finite = number.real == number.real && number.real - number.real == 0.0;
        if ( finite && floor( number.real ) == number.real && fabs( number.real ) < 18446744073709551616.0 )
        {
            number.isInteger = true;
            number.magnitude = (UInt64) fabs( number.real );
            number.negative = number.real < 0.0 && 0 != number.magnitude;
        }
        else
        {
            number.isInteger = false;
            number.negative = false;
            number.magnitude = 0;
        }
    }
}

/*-------------------------------------------------------------------------*/

/** -1, 0 or 1 as a is smaller, equal or bigger than b; 2 when they cannot be ordered (not a number) */
static Int32
CompareNumbers( const SNumber& a, const SNumber& b )
{GUCEF_TRACE;

    if ( a.isInteger && b.isInteger )
    {
        if ( a.negative != b.negative )
            return a.negative ? -1 : 1;
        if ( a.magnitude == b.magnitude )
            return 0;
        bool smaller = a.magnitude < b.magnitude;
        if ( a.negative )
            smaller = !smaller;
        return smaller ? -1 : 1;
    }
    if ( a.real < b.real )
        return -1;
    if ( a.real > b.real )
        return 1;
    if ( a.real == b.real )
        return 0;
    return 2;
}

/*-------------------------------------------------------------------------*/

static bool
IsFinite( Float64 value )
{GUCEF_TRACE;

    return value == value && value - value == 0.0;
}

/*-------------------------------------------------------------------------*/

static bool
IsMultipleOf( const SNumber& value, const SNumber& divisor )
{GUCEF_TRACE;

    if ( value.isInteger && divisor.isInteger )
        return 0 != divisor.magnitude && 0 == value.magnitude % divisor.magnitude;

    // Decimal fractions have no exact binary form: a tiny relative distance to a whole quotient counts as a multiple
    Float64 quotient = value.real / divisor.real;
    if ( IsFinite( quotient ) )
        return fabs( quotient - floor( quotient + 0.5 ) ) <= 1e-9;

    // The quotient does not fit a double, the remainder still tells
    Float64 remainder = fmod( fabs( value.real ), fabs( divisor.real ) );
    Float64 tolerance = fabs( divisor.real ) * 1e-9;
    return remainder <= tolerance || fabs( divisor.real ) - remainder <= tolerance;
}

/*-------------------------------------------------------------------------*/

/** A number as JSON text, with as few digits as give back the same value */
static CString
FormatNumber( const CVariant& value )
{GUCEF_TRACE;

    if ( value.IsInteger() )
        return value.IsUnsignedInteger() ? UInt64ToString( value.AsUInt64() ) : Int64ToString( value.AsInt64() );

    // a decimal point whatever the locale of the C library is
    CString text = Float64ToRoundTripString( value.AsFloat64() );
    return text.IsNULLOrEmpty() ? CString( "null" ) : text;
}

/*-------------------------------------------------------------------------*/

/** The UTF-8 bytes of a string, without a copy where the variant holds them already */
static void
GetUtf8Bytes( const CVariant& value, CUtf8String& storage, const char*& data, UInt32& size )
{GUCEF_TRACE;

    UInt8 typeId = value.GetTypeId();
    if ( GUCEF_DATATYPE_UTF8_STRING == typeId || GUCEF_DATATYPE_ASCII_STRING == typeId ||
         GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING == typeId || GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING == typeId )
    {
        data = value.AsCharPtr( "" );
        size = value.ByteSize( false );
        if ( GUCEF_NULL == data )
        {
            data = "";
            size = 0;
        }
        return;
    }
    storage = value.AsUtf8String();
    data = storage.C_String();
    size = storage.ByteSize() > 0 ? storage.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

static UInt64
CodePointCount( const CVariant& value )
{GUCEF_TRACE;

    CUtf8String storage;
    const char* data = GUCEF_NULL;
    UInt32 size = 0;
    GetUtf8Bytes( value, storage, data, size );

    UInt64 count = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        // every byte that is not a continuation byte starts a code point
        if ( 0x80 != ( ( (UInt8) data[ i ] ) & 0xC0 ) )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

static bool
SameStrings( const CVariant& a, const CVariant& b )
{GUCEF_TRACE;

    CUtf8String storageA, storageB;
    const char* dataA = GUCEF_NULL;
    const char* dataB = GUCEF_NULL;
    UInt32 sizeA = 0, sizeB = 0;
    GetUtf8Bytes( a, storageA, dataA, sizeA );
    GetUtf8Bytes( b, storageB, dataB, sizeB );
    return sizeA == sizeB && ( 0 == sizeA || 0 == memcmp( dataA, dataB, sizeA ) );
}

/*-------------------------------------------------------------------------*/

static CString
TypeMaskText( UInt32 mask )
{GUCEF_TRACE;

    static const char* names[] = { "null", "boolean", "object", "array", "number", "integer", "string" };
    CString text;
    for ( UInt32 bit=0; bit<7; ++bit )
    {
        if ( 0 != ( mask & ( 1u << bit ) ) )
        {
            if ( !text.IsNULLOrEmpty() )
                text += " or ";
            text += names[ bit ];
        }
    }
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
CJsonSchemaValidator::TypeNameOf( const SValueRef& value )
{GUCEF_TRACE;

    switch ( CJsonSchema::KindOf( value ) )
    {
        case CJsonSchema::JSONKIND_NULL:        return "null";
        case CJsonSchema::JSONKIND_BOOLEAN:     return "boolean";
        case CJsonSchema::JSONKIND_INTEGER:     return "integer";
        case CJsonSchema::JSONKIND_NUMBER:      return "number";
        case CJsonSchema::JSONKIND_STRING:      return "string";
        case CJsonSchema::JSONKIND_OBJECT:      return "object";
        case CJsonSchema::JSONKIND_ARRAY:       return "array";
        default:                                return "a value of a type JSON does not have";
    }
}

/*-------------------------------------------------------------------------*/

CString
CJsonSchemaValidator::Describe( const SValueRef& value )
{GUCEF_TRACE;

    switch ( CJsonSchema::KindOf( value ) )
    {
        case CJsonSchema::JSONKIND_NULL:        return "null";
        case CJsonSchema::JSONKIND_BOOLEAN:     return value.scalar->AsBool() ? "true" : "false";
        case CJsonSchema::JSONKIND_INTEGER:
        case CJsonSchema::JSONKIND_NUMBER:      return FormatNumber( *value.scalar );
        case CJsonSchema::JSONKIND_STRING:
        {
            CString text = value.scalar->AsString();
            if ( text.Length() > 64 )
                text = text.SubstrFromRange( 0, 64 ) + "...";
            return Quoted( text );
        }
        case CJsonSchema::JSONKIND_OBJECT:      return "{...}";
        case CJsonSchema::JSONKIND_ARRAY:       return "[...]";
        default:                                return "?";
    }
}

/*-------------------------------------------------------------------------*/

void
CJsonSchemaValidator::StopForBudget( SRun& run, const CString& message )
{GUCEF_TRACE;

    if ( !run.outOfBudget )
    {
        run.outOfBudget = true;
        run.budgetMessage = message;
        run.budgetPath = PathText( run.path );
    }
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::Step( SRun& run )
{GUCEF_TRACE;

    ++run.steps;
    if ( run.steps > run.maxSteps )
    {
        StopForBudget( run, "schema too expensive: the validation needs more than " + UInt32ToString( run.maxSteps ) + " steps" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CJsonSchemaValidator::AddError( SRun& run, const char* keyword, const CString& message )
{GUCEF_TRACE;

    if ( run.silent > 0 || run.full )
        return;

    SError error;
    error.instancePath = PathText( run.path );
    error.keyword = keyword;
    error.message = message;
    run.errors.push_back( error );
    if ( run.errors.size() >= run.maxErrors )
        run.full = true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::JsonEquals( SRun& run, const SValueRef& a, const SValueRef& b, UInt32 depth )
{GUCEF_TRACE;

    if ( !Step( run ) )
        return false;
    if ( depth >= MaxEvaluationDepth )
    {
        StopForBudget( run, "schema too expensive: comparing values nests deeper than " + UInt32ToString( MaxEvaluationDepth ) + " levels" );
        return false;
    }

    CJsonSchema::EJsonKind kindA = CJsonSchema::KindOf( a );
    CJsonSchema::EJsonKind kindB = CJsonSchema::KindOf( b );
    bool numberA = CJsonSchema::JSONKIND_INTEGER == kindA || CJsonSchema::JSONKIND_NUMBER == kindA;
    bool numberB = CJsonSchema::JSONKIND_INTEGER == kindB || CJsonSchema::JSONKIND_NUMBER == kindB;
    if ( numberA || numberB )
    {
        if ( !numberA || !numberB )
            return false;
        SNumber x, y;
        ToNumber( *a.scalar, x );
        ToNumber( *b.scalar, y );
        return 0 == CompareNumbers( x, y );
    }
    if ( kindA != kindB )
        return false;

    switch ( kindA )
    {
        case CJsonSchema::JSONKIND_NULL:
            return true;
        case CJsonSchema::JSONKIND_BOOLEAN:
            return a.scalar->AsBool() == b.scalar->AsBool();
        case CJsonSchema::JSONKIND_STRING:
            return SameStrings( *a.scalar, *b.scalar );
        case CJsonSchema::JSONKIND_ARRAY:
        {
            if ( a.node->GetNrOfDirectChildNodes() != b.node->GetNrOfDirectChildNodes() )
                return false;
            CDataNode::const_iterator i = a.node->ConstBegin();
            CDataNode::const_iterator n = b.node->ConstBegin();
            while ( i != a.node->ConstEnd() && n != b.node->ConstEnd() )
            {
                if ( !JsonEquals( run, CJsonSchema::ChildAsValue( *(*i) ), CJsonSchema::ChildAsValue( *(*n) ), depth+1 ) )
                    return false;
                ++i;
                ++n;
            }
            return true;
        }
        case CJsonSchema::JSONKIND_OBJECT:
        {
            // with unique names on both sides, the same count and every member of a found in b, the member sets are the same.
            // An object with a repeated name is no set of members: it equals nothing
            if ( CJsonSchema::CountMembers( *a.node ) != CJsonSchema::CountMembers( *b.node ) )
                return false;
            if ( GUCEF_NULL != FindRepeatedMemberName( run, *a.node ) || GUCEF_NULL != FindRepeatedMemberName( run, *b.node ) )
                return false;
            CDataNode::TAttributeMap::const_iterator att = a.node->AttributeBegin();
            while ( att != a.node->AttributeEnd() )
            {
                SValueRef other;
                if ( !FindObjectMember( run, *b.node, (*att).first, other ) ||
                     !JsonEquals( run, SValueRef( GUCEF_NULL, &(*att).second ), other, depth+1 ) )
                    return false;
                ++att;
            }
            CDataNode::const_iterator c = a.node->ConstBegin();
            while ( c != a.node->ConstEnd() )
            {
                SValueRef other;
                if ( !FindObjectMember( run, *b.node, (*c)->GetName(), other ) ||
                     !JsonEquals( run, CJsonSchema::ChildAsValue( *(*c) ), other, depth+1 ) )
                    return false;
                ++c;
            }
            return true;
        }
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::FindObjectMember( SRun& run, const CDataNode& object, const CString& name, SValueRef& member )
{GUCEF_TRACE;

    const CDataNode::TKeyValuePair* attribute = object.GetAttribute( name );
    if ( GUCEF_NULL != attribute )
    {
        member = SValueRef( GUCEF_NULL, &attribute->second );
        return true;
    }
    if ( object.GetNrOfDirectChildNodes() < MinChildrenForIndex )
        return CJsonSchema::FindMember( object, name, member );

    // the first child of the name, in a binary search of the sorted children
    FindRepeatedMemberName( run, object );
    const GUCEF::vector< const CDataNode* >& children = run.memberIndexes[ &object ].children;
    size_t low = 0;
    size_t high = children.size();
    while ( low < high )
    {
        const size_t middle = low + ( high - low ) / 2;
        if ( children[ middle ]->GetName() < name )
            low = middle + 1;
        else
            high = middle;
    }
    if ( low < children.size() && children[ low ]->GetName() == name )
    {
        member = CJsonSchema::ChildAsValue( *children[ low ] );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString*
CJsonSchemaValidator::FindRepeatedMemberName( SRun& run, const CDataNode& object )
{GUCEF_TRACE;

    const UInt32 childCount = object.GetNrOfDirectChildNodes();
    if ( 0 == childCount )
        return GUCEF_NULL;       // attributes are a map: their names are unique

    if ( childCount < MinChildrenForIndex )
    {
        for ( CDataNode::const_iterator c = object.ConstBegin(); c != object.ConstEnd(); ++c )
        {
            const CString& name = (*c)->GetName();
            if ( GUCEF_NULL != object.GetAttribute( name ) )
                return &name;
            CDataNode::const_iterator other = c;
            for ( ++other; other != object.ConstEnd(); ++other )
                if ( (*other)->GetName() == name )
                    return &name;
        }
        return GUCEF_NULL;
    }

    SRun::TMemberIndexMap::iterator found = run.memberIndexes.find( &object );
    if ( found != run.memberIndexes.end() )
        return (*found).second.repeatedName;

    SMemberIndex& index = run.memberIndexes[ &object ];
    index.repeatedName = GUCEF_NULL;
    index.children.reserve( childCount );
    for ( CDataNode::const_iterator c = object.ConstBegin(); c != object.ConstEnd(); ++c )
        index.children.push_back( *c );
    std::stable_sort( index.children.begin(), index.children.end(), SChildNameLess() );
    for ( size_t i=0; i<index.children.size() && GUCEF_NULL == index.repeatedName; ++i )
    {
        const CString& name = index.children[ i ]->GetName();
        if ( ( i+1 < index.children.size() && index.children[ i+1 ]->GetName() == name ) || GUCEF_NULL != object.GetAttribute( name ) )
            index.repeatedName = &name;
    }
    return index.repeatedName;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::ValidateSilently( SRun& run, UInt32 index, const SValueRef& value )
{GUCEF_TRACE;

    CCounterScope silentScope( run.silent );
    return ValidateSubschema( run, index, value );
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::ValidateNumber( SRun& run, const SSubschema& subschema, const SValueRef& value )
{GUCEF_TRACE;

    bool valid = true;
    SNumber number;
    ToNumber( *value.scalar, number );

    if ( subschema.hasMinimum )
    {
        if ( !Step( run ) )
            return false;
        SNumber limit;
        ToNumber( subschema.minimum, limit );
        Int32 order = CompareNumbers( number, limit );
        if ( order < 0 || 2 == order )
        {
            AddError( run, "minimum", "must be >= " + FormatNumber( subschema.minimum ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }
    if ( subschema.hasMaximum )
    {
        if ( !Step( run ) )
            return false;
        SNumber limit;
        ToNumber( subschema.maximum, limit );
        Int32 order = CompareNumbers( number, limit );
        if ( order > 0 )
        {
            AddError( run, "maximum", "must be <= " + FormatNumber( subschema.maximum ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }
    if ( subschema.hasExclusiveMinimum )
    {
        if ( !Step( run ) )
            return false;
        SNumber limit;
        ToNumber( subschema.exclusiveMinimum, limit );
        Int32 order = CompareNumbers( number, limit );
        if ( order <= 0 || 2 == order )
        {
            AddError( run, "exclusiveMinimum", "must be > " + FormatNumber( subschema.exclusiveMinimum ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }
    if ( subschema.hasExclusiveMaximum )
    {
        if ( !Step( run ) )
            return false;
        SNumber limit;
        ToNumber( subschema.exclusiveMaximum, limit );
        Int32 order = CompareNumbers( number, limit );
        if ( order >= 0 )
        {
            AddError( run, "exclusiveMaximum", "must be < " + FormatNumber( subschema.exclusiveMaximum ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }
    if ( subschema.hasMultipleOf )
    {
        if ( !Step( run ) )
            return false;
        SNumber divisor;
        ToNumber( subschema.multipleOf, divisor );
        if ( !IsMultipleOf( number, divisor ) )
        {
            AddError( run, "multipleOf", "must be a multiple of " + FormatNumber( subschema.multipleOf ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }
    return valid;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::ValidateString( SRun& run, const SSubschema& subschema, const SValueRef& value )
{GUCEF_TRACE;

    if ( !subschema.hasMinLength && !subschema.hasMaxLength )
        return true;
    if ( !Step( run ) )
        return false;

    bool valid = true;
    UInt64 length = CodePointCount( *value.scalar );
    if ( subschema.hasMinLength && length < subschema.minLength )
    {
        AddError( run, "minLength", "must be at least " + UInt64ToString( subschema.minLength ) + " characters long, but is " + UInt64ToString( length ) );
        valid = false;
        if ( run.silent > 0 || run.Stopped() )
            return false;
    }
    if ( subschema.hasMaxLength && length > subschema.maxLength )
    {
        AddError( run, "maxLength", "must be at most " + UInt64ToString( subschema.maxLength ) + " characters long, but is " + UInt64ToString( length ) );
        valid = false;
    }
    return valid;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::ValidateMember( SRun& run, const SSubschema& subschema, const CString& name, const SValueRef& member )
{GUCEF_TRACE;

    bool valid = true;
    if ( CJsonSchema::NoSubschema != subschema.propertyNames )
    {
        CVariant nameValue( name );
        bool nameValid = ValidateSilently( run, subschema.propertyNames, SValueRef( GUCEF_NULL, &nameValue ) );
        if ( run.outOfBudget )
            return false;
        if ( !nameValid )
        {
            AddError( run, "propertyNames", "the property name " + Quoted( name ) + " is not allowed by propertyNames" );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }
    if ( CJsonSchema::NoSubschema != subschema.additionalProperties &&
         subschema.properties.find( name ) == subschema.properties.end() )
    {
        const SSubschema& additional = run.schema.m_subschemas[ subschema.additionalProperties ];
        if ( additional.isBooleanSchema && !additional.booleanValue )
        {
            if ( !Step( run ) )
                return false;
            AddError( run, "additionalProperties", "the property " + Quoted( name ) + " is not allowed" );
            return false;
        }
        CPathScope pathScope( run.path, &name );
        if ( !ValidateSubschema( run, subschema.additionalProperties, member ) )
            valid = false;
    }
    return valid;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::ValidateObject( SRun& run, const SSubschema& subschema, const SValueRef& value )
{GUCEF_TRACE;

    bool valid = true;
    const CDataNode& object = *value.node;

    // a repeated member name makes the object ambiguous: which of the two would the keywords judge, and which would
    // the receiver use? Refused, so that no member escapes validation
    const CString* repeated = FindRepeatedMemberName( run, object );
    if ( GUCEF_NULL != repeated )
    {
        if ( !Step( run ) )
            return false;
        AddError( run, "", "the property " + Quoted( *repeated ) + " occurs more than once" );
        return false;
    }

    CJsonSchema::TStringIndexMap::const_iterator p = subschema.properties.begin();
    while ( p != subschema.properties.end() )
    {
        SValueRef member;
        if ( FindObjectMember( run, object, (*p).first, member ) )
        {
            CPathScope pathScope( run.path, &(*p).first );
            if ( !ValidateSubschema( run, (*p).second, member ) )
            {
                valid = false;
                if ( run.silent > 0 || run.Stopped() )
                    return false;
            }
        }
        ++p;
    }

    for ( UInt32 i=0; i<subschema.required.size(); ++i )
    {
        if ( !Step( run ) )
            return false;
        SValueRef member;
        if ( !FindObjectMember( run, object, subschema.required[ i ], member ) )
        {
            AddError( run, "required", "the required property " + Quoted( subschema.required[ i ] ) + " is missing" );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    CJsonSchema::TStringListMap::const_iterator d = subschema.dependentRequired.begin();
    while ( d != subschema.dependentRequired.end() )
    {
        if ( !Step( run ) )
            return false;
        SValueRef member;
        if ( FindObjectMember( run, object, (*d).first, member ) )
        {
            const CJsonSchema::TStringVector& names = (*d).second;
            for ( UInt32 i=0; i<names.size(); ++i )
            {
                if ( !FindObjectMember( run, object, names[ i ], member ) )
                {
                    AddError( run, "dependentRequired", "the property " + Quoted( names[ i ] ) + " is required when the property " + Quoted( (*d).first ) + " is present" );
                    valid = false;
                    if ( run.silent > 0 || run.Stopped() )
                        return false;
                }
            }
        }
        ++d;
    }

    if ( subschema.hasMinProperties || subschema.hasMaxProperties )
    {
        if ( !Step( run ) )
            return false;
        UInt64 count = CJsonSchema::CountMembers( object );
        if ( subschema.hasMinProperties && count < subschema.minProperties )
        {
            AddError( run, "minProperties", "must have at least " + UInt64ToString( subschema.minProperties ) + " properties, but has " + UInt64ToString( count ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
        if ( subschema.hasMaxProperties && count > subschema.maxProperties )
        {
            AddError( run, "maxProperties", "must have at most " + UInt64ToString( subschema.maxProperties ) + " properties, but has " + UInt64ToString( count ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    if ( CJsonSchema::NoSubschema != subschema.additionalProperties || CJsonSchema::NoSubschema != subschema.propertyNames )
    {
        CDataNode::TAttributeMap::const_iterator a = object.AttributeBegin();
        while ( a != object.AttributeEnd() )
        {
            if ( !ValidateMember( run, subschema, (*a).first, SValueRef( GUCEF_NULL, &(*a).second ) ) )
            {
                valid = false;
                if ( run.silent > 0 || run.Stopped() )
                    return false;
            }
            ++a;
        }
        CDataNode::const_iterator c = object.ConstBegin();
        while ( c != object.ConstEnd() )
        {
            if ( !ValidateMember( run, subschema, (*c)->GetName(), CJsonSchema::ChildAsValue( *(*c) ) ) )
            {
                valid = false;
                if ( run.silent > 0 || run.Stopped() )
                    return false;
            }
            ++c;
        }
    }
    return valid;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::ValidateArray( SRun& run, const SSubschema& subschema, const SValueRef& value )
{GUCEF_TRACE;

    bool valid = true;
    const CDataNode& array = *value.node;
    const UInt32 count = array.GetNrOfDirectChildNodes();

    if ( !subschema.prefixItems.empty() || CJsonSchema::NoSubschema != subschema.items )
    {
        UInt32 position = 0;
        CDataNode::const_iterator i = array.ConstBegin();
        while ( i != array.ConstEnd() )
        {
            UInt32 itemSchema = CJsonSchema::NoSubschema;
            if ( position < subschema.prefixItems.size() )
                itemSchema = subschema.prefixItems[ position ];
            else
                itemSchema = subschema.items;
            if ( CJsonSchema::NoSubschema == itemSchema )
                break;

            const SSubschema& itemSubschema = run.schema.m_subschemas[ itemSchema ];
            if ( itemSubschema.isBooleanSchema && !itemSubschema.booleanValue && position >= subschema.prefixItems.size() )
            {
                // "items": false after prefixItems is a maximum length, which says more as one error than one per item
                if ( !Step( run ) )
                    return false;
                AddError( run, "items", "must have at most " + UInt32ToString( (UInt32) subschema.prefixItems.size() ) + " items, but has " + UInt32ToString( count ) );
                valid = false;
                if ( run.silent > 0 || run.Stopped() )
                    return false;
                break;
            }

            CPathScope pathScope( run.path, position );
            if ( !ValidateSubschema( run, itemSchema, CJsonSchema::ChildAsValue( *(*i) ) ) )
            {
                valid = false;
                if ( run.silent > 0 || run.Stopped() )
                    return false;
            }
            ++position;
            ++i;
        }
    }

    if ( subschema.hasMinItems || subschema.hasMaxItems )
    {
        if ( !Step( run ) )
            return false;
        if ( subschema.hasMinItems && count < subschema.minItems )
        {
            AddError( run, "minItems", "must have at least " + UInt64ToString( subschema.minItems ) + " items, but has " + UInt32ToString( count ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
        if ( subschema.hasMaxItems && count > subschema.maxItems )
        {
            AddError( run, "maxItems", "must have at most " + UInt64ToString( subschema.maxItems ) + " items, but has " + UInt32ToString( count ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    if ( subschema.uniqueItems && count > 1 )
    {
        GUCEF::vector< SValueRef > elements;
        elements.reserve( count );
        CDataNode::const_iterator i = array.ConstBegin();
        while ( i != array.ConstEnd() )
        {
            elements.push_back( CJsonSchema::ChildAsValue( *(*i) ) );
            ++i;
        }
        for ( UInt32 first=0; first+1<elements.size(); ++first )
        {
            for ( UInt32 second=first+1; second<elements.size(); ++second )
            {
                bool same = JsonEquals( run, elements[ first ], elements[ second ], 0 );
                if ( run.outOfBudget )
                    return false;
                if ( same )
                {
                    AddError( run, "uniqueItems", "the items " + UInt32ToString( first ) + " and " + UInt32ToString( second ) + " are equal, but the items must be unique" );
                    return false;
                }
            }
        }
    }
    return valid;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::ValidateComposition( SRun& run, const SSubschema& subschema, const SValueRef& value )
{GUCEF_TRACE;

    bool valid = true;

    for ( UInt32 i=0; i<subschema.allOf.size(); ++i )
    {
        if ( !ValidateSubschema( run, subschema.allOf[ i ], value ) )
        {
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    if ( !subschema.anyOf.empty() )
    {
        bool matched = false;
        for ( UInt32 i=0; i<subschema.anyOf.size() && !matched; ++i )
        {
            matched = ValidateSilently( run, subschema.anyOf[ i ], value );
            if ( run.outOfBudget )
                return false;
        }
        if ( !matched )
        {
            AddError( run, "anyOf", "must match at least one of the " + UInt32ToString( (UInt32) subschema.anyOf.size() ) + " schemas of anyOf, but matches none" );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    if ( !subschema.oneOf.empty() )
    {
        UInt32 matches = 0;
        UInt32 firstMatch = 0;
        UInt32 secondMatch = 0;
        for ( UInt32 i=0; i<subschema.oneOf.size() && matches < 2; ++i )
        {
            bool matched = ValidateSilently( run, subschema.oneOf[ i ], value );
            if ( run.outOfBudget )
                return false;
            if ( matched )
            {
                if ( 0 == matches )
                    firstMatch = i;
                else
                    secondMatch = i;
                ++matches;
            }
        }
        if ( 1 != matches )
        {
            CString count = UInt32ToString( (UInt32) subschema.oneOf.size() );
            if ( 0 == matches )
                AddError( run, "oneOf", "must match exactly one of the " + count + " schemas of oneOf, but matches none" );
            else
                AddError( run, "oneOf", "must match exactly one of the " + count + " schemas of oneOf, but matches both " +
                                        UInt32ToString( firstMatch ) + " and " + UInt32ToString( secondMatch ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    if ( CJsonSchema::NoSubschema != subschema.notSchema )
    {
        bool matched = ValidateSilently( run, subschema.notSchema, value );
        if ( run.outOfBudget )
            return false;
        if ( matched )
        {
            AddError( run, "not", "must not match the schema of not" );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    if ( CJsonSchema::NoSubschema != subschema.ifSchema )
    {
        bool condition = ValidateSilently( run, subschema.ifSchema, value );
        if ( run.outOfBudget )
            return false;
        UInt32 consequence = condition ? subschema.thenSchema : subschema.elseSchema;
        if ( CJsonSchema::NoSubschema != consequence && !ValidateSubschema( run, consequence, value ) )
            valid = false;
    }
    return valid;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::ValidateSubschema( SRun& run, UInt32 index, const SValueRef& value )
{GUCEF_TRACE;

    if ( run.Stopped() )
        return false;
    if ( run.depth >= MaxEvaluationDepth )
    {
        StopForBudget( run, "schema too expensive: the validation nests deeper than " + UInt32ToString( MaxEvaluationDepth ) + " levels" );
        return false;
    }
    if ( !Step( run ) )
        return false;
    CCounterScope depthScope( run.depth );

    const SSubschema& subschema = run.schema.m_subschemas[ index ];
    if ( subschema.isBooleanSchema )
    {
        if ( subschema.booleanValue )
            return true;
        AddError( run, "false", "no value is allowed here" );
        return false;
    }

    bool valid = true;
    CJsonSchema::EJsonKind kind = CJsonSchema::KindOf( value );

    if ( CJsonSchema::NoSubschema != subschema.ref )
    {
        if ( !ValidateSubschema( run, subschema.ref, value ) )
        {
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    if ( 0 != subschema.typeMask )
    {
        bool matches = false;
        switch ( kind )
        {
            case CJsonSchema::JSONKIND_NULL:        matches = 0 != ( subschema.typeMask & CJsonSchema::TYPEBIT_NULL ); break;
            case CJsonSchema::JSONKIND_BOOLEAN:     matches = 0 != ( subschema.typeMask & CJsonSchema::TYPEBIT_BOOLEAN ); break;
            case CJsonSchema::JSONKIND_INTEGER:     matches = 0 != ( subschema.typeMask & ( CJsonSchema::TYPEBIT_INTEGER | CJsonSchema::TYPEBIT_NUMBER ) ); break;
            case CJsonSchema::JSONKIND_STRING:      matches = 0 != ( subschema.typeMask & CJsonSchema::TYPEBIT_STRING ); break;
            case CJsonSchema::JSONKIND_OBJECT:      matches = 0 != ( subschema.typeMask & CJsonSchema::TYPEBIT_OBJECT ); break;
            case CJsonSchema::JSONKIND_ARRAY:       matches = 0 != ( subschema.typeMask & CJsonSchema::TYPEBIT_ARRAY ); break;
            case CJsonSchema::JSONKIND_NUMBER:
            {
                matches = 0 != ( subschema.typeMask & CJsonSchema::TYPEBIT_NUMBER );
                if ( !matches && 0 != ( subschema.typeMask & CJsonSchema::TYPEBIT_INTEGER ) )
                {
                    // 2020-12: a number with a zero fractional part is an integer
                    Float64 number = value.scalar->AsFloat64();
                    matches = IsFinite( number ) && floor( number ) == number;
                }
                break;
            }
            default:                                matches = false; break;
        }
        if ( !matches )
        {
            AddError( run, "type", "must be of type " + TypeMaskText( subschema.typeMask ) + ", but is " + TypeNameOf( value ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    if ( subschema.hasConst )
    {
        bool same = JsonEquals( run, value, subschema.constValue, 0 );
        if ( run.outOfBudget )
            return false;
        if ( !same )
        {
            AddError( run, "const", "must be the constant value " + Describe( subschema.constValue ) );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    if ( subschema.hasEnum )
    {
        bool found = false;
        for ( UInt32 i=0; i<subschema.enumValues.size() && !found; ++i )
        {
            found = JsonEquals( run, value, subschema.enumValues[ i ], 0 );
            if ( run.outOfBudget )
                return false;
        }
        if ( !found )
        {
            CString allowed;
            UInt32 shown = 0;
            for ( UInt32 i=0; i<subschema.enumValues.size() && shown<10; ++i, ++shown )
            {
                if ( shown > 0 )
                    allowed += ", ";
                allowed += Describe( subschema.enumValues[ i ] );
            }
            if ( subschema.enumValues.size() > shown )
                allowed += ", ...";
            if ( subschema.enumValues.empty() )
                AddError( run, "enum", "no value is allowed: the enum is empty" );
            else
                AddError( run, "enum", "must be one of the allowed values: " + allowed );
            valid = false;
            if ( run.silent > 0 || run.Stopped() )
                return false;
        }
    }

    bool kindValid = true;
    switch ( kind )
    {
        case CJsonSchema::JSONKIND_INTEGER:
        case CJsonSchema::JSONKIND_NUMBER:  kindValid = ValidateNumber( run, subschema, value ); break;
        case CJsonSchema::JSONKIND_STRING:  kindValid = ValidateString( run, subschema, value ); break;
        case CJsonSchema::JSONKIND_OBJECT:  kindValid = ValidateObject( run, subschema, value ); break;
        case CJsonSchema::JSONKIND_ARRAY:   kindValid = ValidateArray( run, subschema, value ); break;
        default:                            break;
    }
    if ( !kindValid )
    {
        valid = false;
        if ( run.silent > 0 || run.Stopped() )
            return false;
    }

    if ( !ValidateComposition( run, subschema, value ) )
        valid = false;
    return valid && !run.outOfBudget;
}

/*-------------------------------------------------------------------------*/

bool
CJsonSchemaValidator::Validate( const CJsonSchema& schema ,
                                const CDataNode& instance ,
                                TErrorVector& errors      ,
                                UInt32 maxErrors          ,
                                UInt32 maxSteps           )
{GUCEF_TRACE;

    errors.clear();
    try
    {
        if ( !schema.IsCompiled() )
        {
            if ( maxErrors > 0 )
            {
                SError error;
                error.message = "the schema is not compiled";
                errors.push_back( error );
            }
            return false;
        }

        SRun run( schema, errors, maxErrors, maxSteps );
        if ( 0 == maxErrors )
            run.silent = 1;

        bool valid = ValidateSubschema( run, 0, CJsonSchema::ChildAsValue( instance ) );
        if ( run.outOfBudget )
        {
            valid = false;
            if ( maxErrors > 0 )
            {
                if ( errors.size() >= maxErrors )
                    errors.pop_back();
                SError error;
                error.instancePath = run.budgetPath;
                error.message = run.budgetMessage;
                errors.push_back( error );
            }
        }
        return valid;
    }
    catch ( ... )
    {
        errors.clear();
        if ( maxErrors > 0 )
        {
            SError error;
            error.message = "the validation failed: out of memory or an internal error";
            errors.push_back( error );
        }
        return false;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
