/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#include <string.h>
#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CBINARYWIREWRITER_H
#include "gucefCORE_CBinaryWireWriter.h"
#define GUCEF_CORE_CBINARYWIREWRITER_H
#endif /* GUCEF_CORE_CBINARYWIREWRITER_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#include "gucefCOMCORE_CRpcBuiltInContracts.h"
#define GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#endif /* GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H ? */

#include "gucefCOMCORE_CRpcContract.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Field numbers of the gucef.rpc meta-types (design 3.8) */
enum
{
    CS_NAME = 1, CS_MAJOR = 2, CS_MINOR = 3, CS_ENUMS = 4, CS_TYPES = 5, CS_SERVICES = 6,
    ES_NAME = 1, ES_VALUES = 2, ES_RESERVEDNUMBERS = 3, ES_RESERVEDNAMES = 4,
    EV_NUMBER = 1, EV_NAME = 2, EV_SINCE = 3,
    TS_NAME = 1, TS_FIELDS = 2, TS_RESERVEDNUMBERS = 3, TS_RESERVEDNAMES = 4,
    FS_NUMBER = 1, FS_NAME = 2, FS_TYPE = 3, FS_CARDINALITY = 4, FS_TYPEREF = 5, FS_MAPKEYTYPE = 6, FS_DECIMALSCALEPLUSONE = 7, FS_FLAGS = 8, FS_SINCE = 9,
    SS_NUMBER = 1, SS_NAME = 2, SS_METHODS = 3, SS_RESERVEDNUMBERS = 4, SS_SINCE = 5,
    MS_NUMBER = 1, MS_NAME = 2, MS_KIND = 3, MS_REQUESTTYPE = 4, MS_RESPONSETYPE = 5, MS_FLAGS = 6, MS_DEADLINE = 7, MS_PROTECTION = 8,
    MS_REQUESTPROTECTION = 9, MS_INITIALCREDIT = 10, MS_SINCE = 11,
    CD_ENTRIES = 1, DE_PATH = 1, DE_TEXT = 2
};

const char g_schemaMagic[ 4 ] = { 'G', 'U', 'C', 'R' };

bool
Fail( CORE::CString& error, const CORE::CString& path, const CORE::CString& what )
{
    error = path.IsNULLOrEmpty() ? what : path + ": " + what;
    return false;
}

CORE::CString
Path( const char* list, UInt32 index )
{
    return CORE::CString( list ) + "[" + CORE::ToString( index ) + "]";
}

bool
IsAsciiRange( const CORE::CString& name, UInt32 maxLength )
{
    CORE::CUtf8String utf8 = CORE::ToUtf8String( name );
    UInt32 size = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    if ( 0 == size || size > maxLength )
        return false;
    const char* c = utf8.C_String();
    for ( UInt32 i=0; i<size; ++i )
        if ( (unsigned char) c[ i ] > 127 )
            return false;
    return true;
}

bool
IsLower( char c ) { return c >= 'a' && c <= 'z'; }
bool
IsUpper( char c ) { return c >= 'A' && c <= 'Z'; }
bool
IsDigit( char c ) { return c >= '0' && c <= '9'; }

/** Prefix test with the members every string class has */
bool
BeginsWith( const CORE::CString& text, const CORE::CString& prefix )
{
    return text.Length() >= prefix.Length() && text.SubstrToIndex( prefix.Length(), true ) == prefix;
}

/** Byte order compare of names, for the canonical sort */
bool
NameLess( const CORE::CString& a, const CORE::CString& b )
{
    CORE::CUtf8String sa = CORE::ToUtf8String( a );
    CORE::CUtf8String sb = CORE::ToUtf8String( b );
    return strcmp( sa.C_String(), sb.C_String() ) < 0;
}

/** Reads an unsigned integer attribute; false when present and not a non-negative integer */
bool
ReadUInt( const CORE::CDataNode& node, const char* name, UInt32& value, UInt32 defaultValue, bool& present )
{
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    present = GUCEF_NULL != attribute;
    if ( !present )
    {
        value = defaultValue;
        return true;
    }
    const CORE::CVariant& v = attribute->second;
    if ( !v.IsInteger() && !v.IsString() )
        return false;
    Int64 n = v.AsInt64( -1 );
    if ( n < 0 || n > (Int64) 0xFFFFFFFFLL )
        return false;
    value = (UInt32) n;
    return true;
}

bool
ReadBoolAttribute( const CORE::CDataNode& node, const char* name )
{
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    return GUCEF_NULL != attribute && attribute->second.AsBool();
}

CORE::CString
ReadString( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsString();
}

bool
ParseScalarType( const CORE::CString& text, TRpcFieldType& type, Int32& decimalScale )
{
    decimalScale = -1;
    if ( BeginsWith( text, "decimal(" ) && text.HasChar( ')' ) == (Int32) text.Length() - 1 )
    {
        CORE::CString digits = text.SubstrToIndex( 8, false ).CutChars( 1, false );
        if ( digits.IsNULLOrEmpty() || digits.Length() > 2 )
            return false;
        Int32 scale = CORE::StringToInt32( digits, -1 );
        if ( scale < 0 || scale > (Int32) CORE::CDecimal::MaxScale )
            return false;
        type = RPC_FIELDTYPE_DECIMAL;
        decimalScale = scale;
        return true;
    }
    type = RpcFieldTypeFromString( text );
    return RPC_FIELDTYPE_INVALID != type && RPC_FIELDTYPE_MESSAGE != type && RPC_FIELDTYPE_ENUM != type;
}

void
ReadReserved( const CORE::CDataNode& node, GUCEF::set< Int32 >* signedNumbers, GUCEF::set< UInt32 >* numbers, CORE::CString::StringSet* names )
{
    const CORE::CDataNode* reserved = node.FindChild( "reserved" );
    if ( GUCEF_NULL == reserved )
        return;
    const CORE::CDataNode* list = reserved->FindChild( "numbers" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        while ( i != list->ConstEnd() )
        {
            if ( GUCEF_NULL != signedNumbers )
                signedNumbers->insert( (*i)->GetValue().AsInt32() );
            if ( GUCEF_NULL != numbers )
                numbers->insert( (*i)->GetValue().AsUInt32() );
            ++i;
        }
    }
    list = reserved->FindChild( "names" );
    if ( GUCEF_NULL != list && GUCEF_NULL != names )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        while ( i != list->ConstEnd() )
        {
            names->insert( (*i)->GetValue().AsString() );
            ++i;
        }
    }
}

template < class TNumberSet >
void
WriteReserved( CORE::CDataNode& node, const TNumberSet& numbers, const CORE::CString::StringSet* names )
{
    if ( numbers.empty() && ( GUCEF_NULL == names || names->empty() ) )
        return;
    CORE::CDataNode* reserved = node.AddChild( "reserved", GUCEF_DATATYPE_OBJECT );
    if ( !numbers.empty() )
    {
        CORE::CDataNode* list = reserved->AddChild( "numbers", GUCEF_DATATYPE_ARRAY );
        typename TNumberSet::const_iterator i = numbers.begin();
        while ( i != numbers.end() )
        {
            list->AddValueAsChild( CORE::CVariant( (Int64) (*i) ) );
            ++i;
        }
    }
    if ( GUCEF_NULL != names && !names->empty() )
    {
        CORE::CDataNode* list = reserved->AddChild( "names", GUCEF_DATATYPE_ARRAY );
        CORE::CString::StringSet::const_iterator i = names->begin();
        while ( i != names->end() )
        {
            list->AddValueAsChild( CORE::CVariant( *i ) );
            ++i;
        }
    }
}

struct SEnumValueLess
{
    bool operator()( const CRpcEnumDescriptor::SValue& a, const CRpcEnumDescriptor::SValue& b ) const { return a.number < b.number; }
};

struct SFieldLess
{
    bool operator()( const CRpcFieldDescriptor* a, const CRpcFieldDescriptor* b ) const { return a->GetNumber() < b->GetNumber(); }
};

struct SMethodLess
{
    bool operator()( const CRpcMethodDescriptor* a, const CRpcMethodDescriptor* b ) const { return a->GetNumber() < b->GetNumber(); }
};

struct SEnumNameLess
{
    bool operator()( const CRpcEnumDescriptor* a, const CRpcEnumDescriptor* b ) const { return NameLess( a->GetFullName(), b->GetFullName() ); }
};

struct STypeNameLess
{
    bool operator()( const CRpcTypeDescriptor* a, const CRpcTypeDescriptor* b ) const { return NameLess( a->GetFullName(), b->GetFullName() ); }
};

struct SServiceNumberLess
{
    bool operator()( const CRpcServiceDescriptor* a, const CRpcServiceDescriptor* b ) const { return a->GetNumber() < b->GetNumber(); }
};

}

/*-------------------------------------------------------------------------*/

CRpcContract::CRpcContract( void )
    : CORE::CTSharedObjCreator< CRpcContract, MT::CMutex >( this )
    , m_name()
    , m_major( 0 )
    , m_minor( 0 )
    , m_doc()
    , m_enums()
    , m_types()
    , m_services()
    , m_schema()
    , m_docs()
    , m_hash()
    , m_isBuiltIn( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcContract::~CRpcContract()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CRpcContract::Clear( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_enums.size(); ++i )
        GUCEF_DELETE m_enums[ i ];
    m_enums.clear();
    for ( UInt32 i=0; i<m_types.size(); ++i )
        GUCEF_DELETE m_types[ i ];
    m_types.clear();
    for ( UInt32 i=0; i<m_services.size(); ++i )
        GUCEF_DELETE m_services[ i ];
    m_services.clear();
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::IsValidContractName( const CORE::CString& name )
{GUCEF_TRACE;

    if ( !IsAsciiRange( name, MaxNameLength ) )
        return false;
    CORE::CAsciiString ascii = CORE::ToAsciiString( name );
    const char* c = ascii.C_String();
    bool segmentStart = true;
    while ( '\0' != *c )
    {
        if ( segmentStart )
        {
            if ( !IsLower( *c ) )
                return false;
            segmentStart = false;
        }
        else if ( '.' == *c )
            segmentStart = true;
        else if ( !IsLower( *c ) && !IsDigit( *c ) && '_' != *c )
            return false;
        ++c;
    }
    return !segmentStart;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::IsValidTypeName( const CORE::CString& name )
{GUCEF_TRACE;

    if ( !IsAsciiRange( name, MaxNameLength ) )
        return false;
    CORE::CAsciiString ascii = CORE::ToAsciiString( name );
    const char* c = ascii.C_String();
    if ( !IsUpper( *c ) )
        return false;
    for ( ++c; '\0' != *c; ++c )
        if ( !IsUpper( *c ) && !IsLower( *c ) && !IsDigit( *c ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::IsValidFieldName( const CORE::CString& name )
{GUCEF_TRACE;

    if ( !IsAsciiRange( name, MaxNameLength ) )
        return false;
    CORE::CAsciiString ascii = CORE::ToAsciiString( name );
    const char* c = ascii.C_String();
    if ( !IsLower( *c ) )
        return false;
    for ( ++c; '\0' != *c; ++c )
        if ( !IsUpper( *c ) && !IsLower( *c ) && !IsDigit( *c ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::IsValidEnumValueName( const CORE::CString& name )
{GUCEF_TRACE;

    if ( !IsAsciiRange( name, MaxNameLength ) )
        return false;
    CORE::CAsciiString ascii = CORE::ToAsciiString( name );
    const char* c = ascii.C_String();
    if ( !IsUpper( *c ) )
        return false;
    for ( ++c; '\0' != *c; ++c )
        if ( !IsUpper( *c ) && !IsDigit( *c ) && '_' != *c )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcContract::ResolveTypeName( const CORE::CString& ref ) const
{GUCEF_TRACE;

    if ( ref.HasChar( '.' ) >= 0 )
        return ref;
    return m_name + "." + ref;
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcContract::LoadFromDataNode( const CORE::CDataNode& source, CORE::CString& error )
{GUCEF_TRACE;

    CRpcContract* contract = GUCEF_NEW CRpcContract();
    CRpcContractPtr ptr = contract->CreateSharedPtr();
    if ( !contract->LoadFromDataNodeImpl( source, error ) || !contract->Finalize( error ) )
        return CRpcContractPtr();
    CRpcContractPtr meta = CRpcBuiltInContracts::GetRpcContract();
    if ( meta.IsNULL() || !contract->BuildSchema( *meta, error ) )
        return CRpcContractPtr();
    contract->BuildDocs();
    return ptr;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::LoadFromDataNodeImpl( const CORE::CDataNode& source, CORE::CString& error )
{GUCEF_TRACE;

    Clear();
    m_name = ReadString( source, "contract" );
    if ( !IsValidContractName( m_name ) )
        return Fail( error, "contract", "invalid contract name \"" + m_name + "\"" );

    CORE::CString version = ReadString( source, "version" );
    Int32 dot = version.HasChar( '.' );
    if ( dot <= 0 )
        return Fail( error, "version", "expected \"major.minor\", got \"" + version + "\"" );
    Int32 major = CORE::StringToInt32( version.SubstrToIndex( (UInt32) dot, true ), -1 );
    Int32 minor = CORE::StringToInt32( version.SubstrToIndex( (UInt32) dot + 1, false ), -1 );
    if ( major < 0 || minor < 0 )
        return Fail( error, "version", "expected \"major.minor\", got \"" + version + "\"" );
    m_major = (UInt32) major;
    m_minor = (UInt32) minor;
    m_doc = ReadString( source, "doc" );

    // enums
    const CORE::CDataNode* enums = source.FindChild( "enums" );
    if ( GUCEF_NULL != enums )
    {
        UInt32 enumIndex = 0;
        CORE::CDataNode::const_iterator e = enums->ConstBegin();
        for ( ; e != enums->ConstEnd(); ++e, ++enumIndex )
        {
            const CORE::CDataNode& node = *(*e);
            CORE::CString path = Path( "enums", enumIndex );
            CRpcEnumDescriptor* descriptor = GUCEF_NEW CRpcEnumDescriptor();
            m_enums.push_back( descriptor );
            descriptor->m_name = ReadString( node, "name" );
            descriptor->m_doc = ReadString( node, "doc" );
            const CORE::CDataNode* values = node.FindChild( "values" );
            if ( GUCEF_NULL != values )
            {
                UInt32 valueIndex = 0;
                CORE::CDataNode::const_iterator v = values->ConstBegin();
                for ( ; v != values->ConstEnd(); ++v, ++valueIndex )
                {
                    const CORE::CDataNode& valueNode = *(*v);
                    CRpcEnumDescriptor::SValue value;
                    value.name = ReadString( valueNode, "name" );
                    value.doc = ReadString( valueNode, "doc" );
                    const CORE::CDataNode::TKeyValuePair* number = valueNode.GetAttribute( "number" );
                    if ( GUCEF_NULL == number || !number->second.IsInteger() )
                        return Fail( error, path + "." + Path( "values", valueIndex ) + ".number", "missing or not an integer" );
                    Int64 n = number->second.AsInt64();
                    if ( n < GUCEF_INT32MIN || n > GUCEF_INT32MAX )
                        return Fail( error, path + "." + Path( "values", valueIndex ) + ".number", "outside the Int32 range" );
                    value.number = (Int32) n;
                    bool present = false;
                    if ( !ReadUInt( valueNode, "since", value.since, 0, present ) )
                        return Fail( error, path + "." + Path( "values", valueIndex ) + ".since", "not a non-negative integer" );
                    descriptor->m_values.push_back( value );
                }
            }
            ReadReserved( node, &descriptor->m_reservedNumbers, GUCEF_NULL, &descriptor->m_reservedNames );
        }
    }

    // types
    const CORE::CDataNode* types = source.FindChild( "types" );
    if ( GUCEF_NULL != types )
    {
        UInt32 typeIndex = 0;
        CORE::CDataNode::const_iterator t = types->ConstBegin();
        for ( ; t != types->ConstEnd(); ++t, ++typeIndex )
        {
            const CORE::CDataNode& node = *(*t);
            CORE::CString path = Path( "types", typeIndex );
            CRpcTypeDescriptor* descriptor = GUCEF_NEW CRpcTypeDescriptor();
            m_types.push_back( descriptor );
            descriptor->m_name = ReadString( node, "name" );
            descriptor->m_doc = ReadString( node, "doc" );
            const CORE::CDataNode* fields = node.FindChild( "fields" );
            if ( GUCEF_NULL != fields )
            {
                UInt32 fieldIndex = 0;
                CORE::CDataNode::const_iterator f = fields->ConstBegin();
                for ( ; f != fields->ConstEnd(); ++f, ++fieldIndex )
                {
                    const CORE::CDataNode& fieldNode = *(*f);
                    CORE::CString fieldPath = path + "." + Path( "fields", fieldIndex );
                    CRpcFieldDescriptor* field = GUCEF_NEW CRpcFieldDescriptor();
                    descriptor->m_fields.push_back( field );
                    field->m_name = ReadString( fieldNode, "name" );
                    field->m_doc = ReadString( fieldNode, "doc" );
                    bool present = false;
                    if ( !ReadUInt( fieldNode, "number", field->m_number, 0, present ) || !present )
                        return Fail( error, fieldPath + ".number", "missing or not a positive integer" );
                    if ( !ReadUInt( fieldNode, "since", field->m_since, 0, present ) )
                        return Fail( error, fieldPath + ".since", "not a non-negative integer" );
                    if ( ReadBoolAttribute( fieldNode, "required" ) )
                        field->m_flags |= GUCEF_RPC_FIELDFLAG_REQUIRED;
                    if ( ReadBoolAttribute( fieldNode, "sensitive" ) )
                        field->m_flags |= GUCEF_RPC_FIELDFLAG_SENSITIVE;
                    if ( ReadBoolAttribute( fieldNode, "deprecated" ) )
                        field->m_flags |= GUCEF_RPC_FIELDFLAG_DEPRECATED;

                    CORE::CString cardinality = ReadString( fieldNode, "cardinality" );
                    if ( cardinality.IsNULLOrEmpty() || "single" == cardinality )
                        field->m_cardinality = RPC_CARDINALITY_SINGLE;
                    else if ( "optional" == cardinality )
                        field->m_cardinality = RPC_CARDINALITY_OPTIONAL;
                    else if ( "repeated" == cardinality )
                        field->m_cardinality = RPC_CARDINALITY_REPEATED;
                    else
                        return Fail( error, fieldPath + ".cardinality", "unknown cardinality \"" + cardinality + "\"" );

                    CORE::CString typeText = ReadString( fieldNode, "type" ).RemoveChar( ' ' );
                    CORE::CString valueText = typeText;
                    if ( BeginsWith( typeText, "map<" ) )
                    {
                        if ( RPC_CARDINALITY_REPEATED == field->m_cardinality )
                            return Fail( error, fieldPath + ".cardinality", "a map cannot be repeated" );
                        Int32 comma = typeText.HasChar( ',' );
                        if ( comma < 0 || typeText.HasChar( '>', false ) != (Int32) typeText.Length() - 1 )
                            return Fail( error, fieldPath + ".type", "expected map<key,value>" );
                        CORE::CString keyText = typeText.SubstrToIndex( (UInt32) comma, true ).CutChars( 4, true );
                        valueText = typeText.SubstrToIndex( (UInt32) comma + 1, false ).CutChars( 1, false );
                        Int32 keyScale = -1;
                        TRpcFieldType keyType = RPC_FIELDTYPE_INVALID;
                        if ( !ParseScalarType( keyText, keyType, keyScale ) )
                            return Fail( error, fieldPath + ".type", "invalid map key type \"" + keyText + "\"" );
                        field->m_cardinality = RPC_CARDINALITY_MAP;
                        field->m_mapKeyType = keyType;
                        if ( BeginsWith( valueText, "map<" ) )
                            return Fail( error, fieldPath + ".type", "a map of maps is not allowed" );
                    }
                    Int32 scale = -1;
                    TRpcFieldType fieldType = RPC_FIELDTYPE_INVALID;
                    if ( ParseScalarType( valueText, fieldType, scale ) )
                    {
                        field->m_type = fieldType;
                        field->m_decimalScale = scale;
                    }
                    else if ( valueText.IsNULLOrEmpty() )
                        return Fail( error, fieldPath + ".type", "missing" );
                    else
                    {
                        // a message or enum reference, resolved when the contract is finalized
                        field->m_type = RPC_FIELDTYPE_INVALID;
                        field->m_typeRef = valueText;
                    }
                }
            }
            ReadReserved( node, GUCEF_NULL, &descriptor->m_reservedNumbers, &descriptor->m_reservedNames );
        }
    }

    // services
    const CORE::CDataNode* services = source.FindChild( "services" );
    if ( GUCEF_NULL != services )
    {
        UInt32 serviceIndex = 0;
        CORE::CDataNode::const_iterator s = services->ConstBegin();
        for ( ; s != services->ConstEnd(); ++s, ++serviceIndex )
        {
            const CORE::CDataNode& node = *(*s);
            CORE::CString path = Path( "services", serviceIndex );
            CRpcServiceDescriptor* descriptor = GUCEF_NEW CRpcServiceDescriptor();
            m_services.push_back( descriptor );
            descriptor->m_name = ReadString( node, "name" );
            descriptor->m_doc = ReadString( node, "doc" );
            bool present = false;
            if ( !ReadUInt( node, "number", descriptor->m_number, 0, present ) || !present )
                return Fail( error, path + ".number", "missing or not a positive integer" );
            if ( !ReadUInt( node, "since", descriptor->m_since, 0, present ) )
                return Fail( error, path + ".since", "not a non-negative integer" );
            const CORE::CDataNode* methods = node.FindChild( "methods" );
            if ( GUCEF_NULL != methods )
            {
                UInt32 methodIndex = 0;
                CORE::CDataNode::const_iterator m = methods->ConstBegin();
                for ( ; m != methods->ConstEnd(); ++m, ++methodIndex )
                {
                    const CORE::CDataNode& methodNode = *(*m);
                    CORE::CString methodPath = path + "." + Path( "methods", methodIndex );
                    CRpcMethodDescriptor* method = GUCEF_NEW CRpcMethodDescriptor();
                    descriptor->m_methods.push_back( method );
                    method->m_name = ReadString( methodNode, "name" );
                    method->m_doc = ReadString( methodNode, "doc" );
                    if ( !ReadUInt( methodNode, "number", method->m_number, 0, present ) || !present )
                        return Fail( error, methodPath + ".number", "missing or not a positive integer" );
                    if ( !ReadUInt( methodNode, "since", method->m_since, 0, present ) )
                        return Fail( error, methodPath + ".since", "not a non-negative integer" );
                    if ( !ReadUInt( methodNode, "deadlineMs", method->m_defaultDeadlineMs, 0, present ) )
                        return Fail( error, methodPath + ".deadlineMs", "not a non-negative integer" );
                    bool creditPresent = false;
                    if ( !ReadUInt( methodNode, "initialCredit", method->m_initialCredit, 0, creditPresent ) )
                        return Fail( error, methodPath + ".initialCredit", "not a non-negative integer" );
                    CORE::CString kind = ReadString( methodNode, "kind" );
                    if ( kind.IsNULLOrEmpty() )
                        kind = "unary";
                    if ( !RpcMethodKindFromString( kind, method->m_kind ) )
                        return Fail( error, methodPath + ".kind", "unknown kind \"" + kind + "\"" );
                    if ( creditPresent && RPC_METHODKIND_UNARY == method->m_kind )
                        return Fail( error, methodPath + ".initialCredit", "only streams take an initial credit" );
                    method->m_requestTypeName = ReadString( methodNode, "request" );
                    method->m_responseTypeName = ReadString( methodNode, "response" );

                    CORE::CString protection = ReadString( methodNode, "protection" );
                    if ( "public" == protection )
                        method->m_protection = RPC_PROTECTION_PUBLIC;
                    else if ( !protection.IsNULLOrEmpty() && "confidential" != protection )
                        return Fail( error, methodPath + ".protection", "expected \"confidential\" or \"public\"" );
                    CORE::CString requestProtection = ReadString( methodNode, "requestProtection" );
                    if ( "public" == requestProtection )
                        method->m_requestProtection = RPC_PROTECTION_PUBLIC;
                    else if ( !requestProtection.IsNULLOrEmpty() && "confidential" != requestProtection )
                        return Fail( error, methodPath + ".requestProtection", "expected \"confidential\" or \"public\"" );

                    const CORE::CDataNode* flags = methodNode.FindChild( "flags" );
                    if ( GUCEF_NULL != flags )
                    {
                        CORE::CDataNode::const_iterator fl = flags->ConstBegin();
                        for ( ; fl != flags->ConstEnd(); ++fl )
                        {
                            CORE::CString flag = (*fl)->GetValue().AsString();
                            if ( "idempotent" == flag ) method->m_flags |= GUCEF_RPC_METHODFLAG_IDEMPOTENT;
                            else if ( "mayAnswerAsync" == flag ) method->m_flags |= GUCEF_RPC_METHODFLAG_MAYANSWERASYNC;
                            else if ( "resumableStream" == flag ) method->m_flags |= GUCEF_RPC_METHODFLAG_RESUMABLESTREAM;
                            else if ( "sensitive" == flag ) method->m_flags |= GUCEF_RPC_METHODFLAG_SENSITIVE;
                            else
                                return Fail( error, methodPath + ".flags", "unknown flag \"" + flag + "\"" );
                        }
                    }
                }
            }
            ReadReserved( node, GUCEF_NULL, &descriptor->m_reservedNumbers, GUCEF_NULL );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::Finalize( CORE::CString& error )
{GUCEF_TRACE;

    if ( !IsValidContractName( m_name ) )
        return Fail( error, "contract", "invalid contract name \"" + m_name + "\"" );
    if ( m_types.size() > MaxTypes )
        return Fail( error, "types", "more than " + CORE::ToString( MaxTypes ) + " types" );
    if ( m_services.size() > MaxServices )
        return Fail( error, "services", "more than " + CORE::ToString( MaxServices ) + " services" );

    // names of enums and types share one scope
    CORE::CString::StringSet typeNames;

    for ( UInt32 e=0; e<m_enums.size(); ++e )
    {
        CRpcEnumDescriptor& descriptor = *m_enums[ e ];
        CORE::CString path = Path( "enums", e );
        if ( !IsValidTypeName( descriptor.m_name ) )
            return Fail( error, path + ".name", "invalid enum name \"" + descriptor.m_name + "\"" );
        descriptor.m_fullName = m_name + "." + descriptor.m_name;
        if ( !typeNames.insert( descriptor.m_name ).second )
            return Fail( error, path + ".name", "duplicate name \"" + descriptor.m_name + "\"" );

        GUCEF::set< Int32 > numbers;
        CORE::CString::StringSet names;
        for ( UInt32 v=0; v<descriptor.m_values.size(); ++v )
        {
            const CRpcEnumDescriptor::SValue& value = descriptor.m_values[ v ];
            CORE::CString valuePath = path + "." + Path( "values", v );
            if ( !IsValidEnumValueName( value.name ) )
                return Fail( error, valuePath + ".name", "invalid enum value name \"" + value.name + "\"" );
            if ( !numbers.insert( value.number ).second )
                return Fail( error, valuePath + ".number", "duplicate number " + CORE::ToString( value.number ) );
            if ( !names.insert( value.name ).second )
                return Fail( error, valuePath + ".name", "duplicate name \"" + value.name + "\"" );
            if ( descriptor.m_reservedNumbers.find( value.number ) != descriptor.m_reservedNumbers.end() )
                return Fail( error, valuePath + ".number", "number " + CORE::ToString( value.number ) + " is reserved" );
            if ( descriptor.m_reservedNames.find( value.name ) != descriptor.m_reservedNames.end() )
                return Fail( error, valuePath + ".name", "name \"" + value.name + "\" is reserved" );
            if ( value.since > m_minor )
                return Fail( error, valuePath + ".since", "since " + CORE::ToString( value.since ) + " is above the contract minor " + CORE::ToString( m_minor ) );
        }
        std::stable_sort( descriptor.m_values.begin(), descriptor.m_values.end(), SEnumValueLess() );
    }

    for ( UInt32 t=0; t<m_types.size(); ++t )
    {
        CRpcTypeDescriptor& descriptor = *m_types[ t ];
        CORE::CString path = Path( "types", t );
        if ( !IsValidTypeName( descriptor.m_name ) )
            return Fail( error, path + ".name", "invalid type name \"" + descriptor.m_name + "\"" );
        descriptor.m_fullName = m_name + "." + descriptor.m_name;
        if ( !typeNames.insert( descriptor.m_name ).second )
            return Fail( error, path + ".name", "duplicate name \"" + descriptor.m_name + "\"" );
        if ( descriptor.m_fields.size() > MaxFieldsPerType )
            return Fail( error, path + ".fields", "more than " + CORE::ToString( MaxFieldsPerType ) + " fields" );
    }

    for ( UInt32 t=0; t<m_types.size(); ++t )
    {
        CRpcTypeDescriptor& descriptor = *m_types[ t ];
        CORE::CString path = Path( "types", t );
        GUCEF::set< UInt32 > numbers;
        CORE::CString::StringSet names;
        for ( UInt32 f=0; f<descriptor.m_fields.size(); ++f )
        {
            CRpcFieldDescriptor& field = *descriptor.m_fields[ f ];
            CORE::CString fieldPath = path + "." + Path( "fields", f );
            if ( !IsValidFieldName( field.m_name ) )
                return Fail( error, fieldPath + ".name", "invalid field name \"" + field.m_name + "\"" );
            if ( field.m_number < 1 || field.m_number > GUCEF_RPC_MAX_FIELD_NUMBER )
                return Fail( error, fieldPath + ".number", "field numbers are 1 to 536870911" );
            if ( !numbers.insert( field.m_number ).second )
                return Fail( error, fieldPath + ".number", "duplicate number " + CORE::ToString( field.m_number ) );
            if ( !names.insert( field.m_name ).second )
                return Fail( error, fieldPath + ".name", "duplicate name \"" + field.m_name + "\"" );
            if ( descriptor.m_reservedNumbers.find( field.m_number ) != descriptor.m_reservedNumbers.end() )
                return Fail( error, fieldPath + ".number", "number " + CORE::ToString( field.m_number ) + " is reserved" );
            if ( descriptor.m_reservedNames.find( field.m_name ) != descriptor.m_reservedNames.end() )
                return Fail( error, fieldPath + ".name", "name \"" + field.m_name + "\" is reserved" );
            if ( field.m_since > m_minor )
                return Fail( error, fieldPath + ".since", "since " + CORE::ToString( field.m_since ) + " is above the contract minor " + CORE::ToString( m_minor ) );

            // resolve references
            field.m_messageType = GUCEF_NULL;
            field.m_enumType = GUCEF_NULL;
            if ( !field.m_typeRef.IsNULLOrEmpty() )
            {
                CORE::CString fullName = ResolveTypeName( field.m_typeRef );
                field.m_typeRef = fullName;
                const CRpcTypeDescriptor* messageType = FindType( fullName );
                const CRpcEnumDescriptor* enumType = FindEnum( fullName );
                if ( GUCEF_NULL != messageType && ( RPC_FIELDTYPE_INVALID == field.m_type || RPC_FIELDTYPE_MESSAGE == field.m_type ) )
                {
                    field.m_type = RPC_FIELDTYPE_MESSAGE;
                    field.m_messageType = messageType;
                }
                else if ( GUCEF_NULL != enumType && ( RPC_FIELDTYPE_INVALID == field.m_type || RPC_FIELDTYPE_ENUM == field.m_type ) )
                {
                    field.m_type = RPC_FIELDTYPE_ENUM;
                    field.m_enumType = enumType;
                }
                else if ( !BeginsWith( fullName, m_name + "." ) )
                    return Fail( error, fieldPath + ".type", "references to other contracts are not supported: \"" + fullName + "\"" );
                else
                    return Fail( error, fieldPath + ".type", "unknown type \"" + field.m_typeRef + "\"" );
            }
            else if ( RPC_FIELDTYPE_INVALID == field.m_type || RPC_FIELDTYPE_MESSAGE == field.m_type || RPC_FIELDTYPE_ENUM == field.m_type ||
                      field.m_type > RPC_FIELDTYPE_DATANODE )
                return Fail( error, fieldPath + ".type", "missing or invalid type" );

            if ( RPC_FIELDTYPE_DECIMAL != field.m_type && field.m_decimalScale >= 0 )
                return Fail( error, fieldPath + ".type", "a scale is for decimals only" );
            if ( field.m_decimalScale > (Int32) CORE::CDecimal::MaxScale )
                return Fail( error, fieldPath + ".type", "decimal scale above 18" );
            if ( RPC_CARDINALITY_MAP == field.m_cardinality )
            {
                switch ( field.m_mapKeyType )
                {
                    case RPC_FIELDTYPE_BOOL: case RPC_FIELDTYPE_STRING:
                    case RPC_FIELDTYPE_INT8: case RPC_FIELDTYPE_INT16: case RPC_FIELDTYPE_INT32: case RPC_FIELDTYPE_INT64:
                    case RPC_FIELDTYPE_UINT8: case RPC_FIELDTYPE_UINT16: case RPC_FIELDTYPE_UINT32: case RPC_FIELDTYPE_UINT64:
                        break;
                    default:
                        return Fail( error, fieldPath + ".type", "map keys are integers, bool or string" );
                }
            }
            else if ( RPC_FIELDTYPE_INVALID != field.m_mapKeyType )
                return Fail( error, fieldPath + ".type", "a key type without a map" );
            if ( field.IsRequired() && !field.IsSingular() )
                return Fail( error, fieldPath + ".required", "only single fields can be required" );

            field.m_wireType = RpcWireTypeOfFieldType( field.m_type, field.m_decimalScale >= 0 );
        }

        std::stable_sort( descriptor.m_fields.begin(), descriptor.m_fields.end(), SFieldLess() );
        for ( UInt32 f=0; f<descriptor.m_fields.size(); ++f )
            descriptor.m_fields[ f ]->m_slotIndex = f;
        descriptor.BuildLookups();
    }

    GUCEF::set< UInt32 > serviceNumbers;
    CORE::CString::StringSet serviceNames;
    for ( UInt32 s=0; s<m_services.size(); ++s )
    {
        CRpcServiceDescriptor& service = *m_services[ s ];
        CORE::CString path = Path( "services", s );
        if ( !IsValidTypeName( service.m_name ) )
            return Fail( error, path + ".name", "invalid service name \"" + service.m_name + "\"" );
        if ( service.m_number < 1 || service.m_number > 65535 )
            return Fail( error, path + ".number", "service numbers are 1 to 65535" );
        if ( !serviceNumbers.insert( service.m_number ).second )
            return Fail( error, path + ".number", "duplicate number " + CORE::ToString( service.m_number ) );
        if ( !serviceNames.insert( service.m_name ).second )
            return Fail( error, path + ".name", "duplicate name \"" + service.m_name + "\"" );
        if ( service.m_since > m_minor )
            return Fail( error, path + ".since", "since " + CORE::ToString( service.m_since ) + " is above the contract minor " + CORE::ToString( m_minor ) );
        if ( service.m_methods.size() > MaxMethodsPerService )
            return Fail( error, path + ".methods", "more than " + CORE::ToString( MaxMethodsPerService ) + " methods" );
        service.m_fullName = m_name + "." + service.m_name;

        GUCEF::set< UInt32 > methodNumbers;
        CORE::CString::StringSet methodNames;
        for ( UInt32 m=0; m<service.m_methods.size(); ++m )
        {
            CRpcMethodDescriptor& method = *service.m_methods[ m ];
            CORE::CString methodPath = path + "." + Path( "methods", m );
            if ( !IsValidTypeName( method.m_name ) )
                return Fail( error, methodPath + ".name", "invalid method name \"" + method.m_name + "\"" );
            if ( method.m_number < 1 || method.m_number > 65535 )
                return Fail( error, methodPath + ".number", "method numbers are 1 to 65535" );
            if ( !methodNumbers.insert( method.m_number ).second )
                return Fail( error, methodPath + ".number", "duplicate number " + CORE::ToString( method.m_number ) );
            if ( !methodNames.insert( method.m_name ).second )
                return Fail( error, methodPath + ".name", "duplicate name \"" + method.m_name + "\"" );
            if ( service.m_reservedNumbers.find( method.m_number ) != service.m_reservedNumbers.end() )
                return Fail( error, methodPath + ".number", "number " + CORE::ToString( method.m_number ) + " is reserved" );
            if ( method.m_since > m_minor )
                return Fail( error, methodPath + ".since", "since " + CORE::ToString( method.m_since ) + " is above the contract minor " + CORE::ToString( m_minor ) );
            if ( method.MayAnswerAsync() && RPC_METHODKIND_UNARY != method.m_kind )
                return Fail( error, methodPath + ".flags", "mayAnswerAsync is for unary methods only" );
            if ( method.m_initialCredit > 0 && RPC_METHODKIND_UNARY == method.m_kind )
                return Fail( error, methodPath + ".initialCredit", "only streams take an initial credit" );

            method.m_requestTypeName = ResolveTypeName( method.m_requestTypeName );
            method.m_responseTypeName = ResolveTypeName( method.m_responseTypeName );
            method.m_requestType = FindType( method.m_requestTypeName );
            method.m_responseType = FindType( method.m_responseTypeName );
            if ( GUCEF_NULL == method.m_requestType )
                return Fail( error, methodPath + ".request", "unknown message type \"" + method.m_requestTypeName + "\"" );
            if ( GUCEF_NULL == method.m_responseType )
                return Fail( error, methodPath + ".response", "unknown message type \"" + method.m_responseTypeName + "\"" );
            method.m_path = service.m_fullName + "/" + method.m_name;
            method.m_service = &service;
        }
        std::stable_sort( service.m_methods.begin(), service.m_methods.end(), SMethodLess() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::BuildSchema( const CRpcContract& meta, CORE::CString& error )
{GUCEF_TRACE;

    CRpcContractPtr metaPtr = meta.CreateSharedPtr();
    const CRpcTypeDescriptor* schemaType = meta.FindType( CRpcBuiltInContracts::ContractSchemaType );
    if ( GUCEF_NULL == schemaType )
        return Fail( error, CORE::CString::Empty, "the gucef.rpc meta-types are not available" );

    CRpcMessage schema( metaPtr, schemaType );
    schema.SetString( CS_NAME, m_name );
    if ( 0 != m_major )
        schema.SetUInt32( CS_MAJOR, m_major );
    if ( 0 != m_minor )
        schema.SetUInt32( CS_MINOR, m_minor );

    TEnumVector enums( m_enums );
    std::stable_sort( enums.begin(), enums.end(), SEnumNameLess() );
    for ( UInt32 e=0; e<enums.size(); ++e )
    {
        const CRpcEnumDescriptor& descriptor = *enums[ e ];
        CRpcMessage* es = schema.AddMessage( CS_ENUMS );
        es->SetString( ES_NAME, descriptor.m_fullName );
        for ( UInt32 v=0; v<descriptor.m_values.size(); ++v )
        {
            const CRpcEnumDescriptor::SValue& value = descriptor.m_values[ v ];
            CRpcMessage* ev = es->AddMessage( ES_VALUES );
            if ( 0 != value.number )
                ev->SetInt32( EV_NUMBER, value.number );
            ev->SetString( EV_NAME, value.name );
            if ( 0 != value.since )
                ev->SetUInt32( EV_SINCE, value.since );
        }
        CRpcEnumDescriptor::TInt32Set::const_iterator n = descriptor.m_reservedNumbers.begin();
        for ( ; n != descriptor.m_reservedNumbers.end(); ++n )
            es->Add( ES_RESERVEDNUMBERS, CORE::CVariant( (Int32) *n ) );
        CORE::CString::StringSet::const_iterator r = descriptor.m_reservedNames.begin();
        for ( ; r != descriptor.m_reservedNames.end(); ++r )
            es->Add( ES_RESERVEDNAMES, CORE::CVariant( *r ) );
    }

    TTypeVector types( m_types );
    std::stable_sort( types.begin(), types.end(), STypeNameLess() );
    for ( UInt32 t=0; t<types.size(); ++t )
    {
        const CRpcTypeDescriptor& descriptor = *types[ t ];
        CRpcMessage* ts = schema.AddMessage( CS_TYPES );
        ts->SetString( TS_NAME, descriptor.m_fullName );
        for ( UInt32 f=0; f<descriptor.m_fields.size(); ++f )
        {
            const CRpcFieldDescriptor& field = *descriptor.m_fields[ f ];
            CRpcMessage* fs = ts->AddMessage( TS_FIELDS );
            fs->SetUInt32( FS_NUMBER, field.m_number );
            fs->SetString( FS_NAME, field.m_name );
            fs->SetUInt32( FS_TYPE, (UInt32) field.m_type );
            if ( RPC_CARDINALITY_SINGLE != field.m_cardinality )
                fs->SetUInt32( FS_CARDINALITY, (UInt32) field.m_cardinality );
            if ( !field.m_typeRef.IsNULLOrEmpty() )
                fs->SetString( FS_TYPEREF, field.m_typeRef );
            if ( RPC_FIELDTYPE_INVALID != field.m_mapKeyType )
                fs->SetUInt32( FS_MAPKEYTYPE, (UInt32) field.m_mapKeyType );
            if ( field.m_decimalScale >= 0 )
                fs->SetUInt32( FS_DECIMALSCALEPLUSONE, (UInt32) field.m_decimalScale + 1 );
            if ( 0 != field.m_flags )
                fs->SetUInt32( FS_FLAGS, field.m_flags );
            if ( 0 != field.m_since )
                fs->SetUInt32( FS_SINCE, field.m_since );
        }
        CRpcTypeDescriptor::TUInt32Set::const_iterator n = descriptor.m_reservedNumbers.begin();
        for ( ; n != descriptor.m_reservedNumbers.end(); ++n )
            ts->Add( TS_RESERVEDNUMBERS, CORE::CVariant( (UInt32) *n ) );
        CORE::CString::StringSet::const_iterator r = descriptor.m_reservedNames.begin();
        for ( ; r != descriptor.m_reservedNames.end(); ++r )
            ts->Add( TS_RESERVEDNAMES, CORE::CVariant( *r ) );
    }

    TServiceVector services( m_services );
    std::stable_sort( services.begin(), services.end(), SServiceNumberLess() );
    for ( UInt32 s=0; s<services.size(); ++s )
    {
        const CRpcServiceDescriptor& service = *services[ s ];
        CRpcMessage* ss = schema.AddMessage( CS_SERVICES );
        ss->SetUInt32( SS_NUMBER, service.m_number );
        ss->SetString( SS_NAME, service.m_name );
        for ( UInt32 m=0; m<service.m_methods.size(); ++m )
        {
            const CRpcMethodDescriptor& method = *service.m_methods[ m ];
            CRpcMessage* ms = ss->AddMessage( SS_METHODS );
            ms->SetUInt32( MS_NUMBER, method.m_number );
            ms->SetString( MS_NAME, method.m_name );
            if ( RPC_METHODKIND_UNARY != method.m_kind )
                ms->SetUInt32( MS_KIND, (UInt32) method.m_kind );
            ms->SetString( MS_REQUESTTYPE, method.m_requestTypeName );
            ms->SetString( MS_RESPONSETYPE, method.m_responseTypeName );
            if ( 0 != method.m_flags )
                ms->SetUInt32( MS_FLAGS, method.m_flags );
            if ( 0 != method.m_defaultDeadlineMs )
                ms->SetUInt32( MS_DEADLINE, method.m_defaultDeadlineMs );
            if ( RPC_PROTECTION_CONFIDENTIAL != method.m_protection )
                ms->SetUInt32( MS_PROTECTION, (UInt32) method.m_protection );
            if ( RPC_PROTECTION_CONFIDENTIAL != method.m_requestProtection )
                ms->SetUInt32( MS_REQUESTPROTECTION, (UInt32) method.m_requestProtection );
            if ( 0 != method.m_initialCredit )
                ms->SetUInt32( MS_INITIALCREDIT, method.m_initialCredit );
            if ( 0 != method.m_since )
                ms->SetUInt32( MS_SINCE, method.m_since );
        }
        CRpcServiceDescriptor::TUInt32Set::const_iterator n = service.m_reservedNumbers.begin();
        for ( ; n != service.m_reservedNumbers.end(); ++n )
            ss->Add( SS_RESERVEDNUMBERS, CORE::CVariant( (UInt32) *n ) );
        if ( 0 != service.m_since )
            ss->SetUInt32( SS_SINCE, service.m_since );
    }

    m_schema.Clear( true );
    CORE::CBinaryWireWriter writer( m_schema );
    writer.WriteBytes( g_schemaMagic, 4 );
    writer.WriteUInt8( SchemaEncodingVersion );
    UInt32 written = 0;
    if ( !schema.Encode( m_schema, writer.GetOffset(), &written ) )
        return Fail( error, CORE::CString::Empty, "the schema cannot be encoded" );
    if ( m_schema.GetDataSize() > MaxSchemaBytes )
        return Fail( error, CORE::CString::Empty, "the schema is larger than 1 MB" );
    m_hash = CRpcContractHash::Compute( m_schema.GetConstBufferPtr(), m_schema.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcContract::BuildDocs( void )
{GUCEF_TRACE;

    m_docs.Clear( true );
    CRpcContractPtr meta = CRpcBuiltInContracts::GetRpcContract();
    if ( meta.IsNULL() )
        return;
    const CRpcTypeDescriptor* docsType = meta->FindType( CRpcBuiltInContracts::ContractDocsType );
    if ( GUCEF_NULL == docsType )
        return;

    // path -> text, sorted by path so the docs message is deterministic too
    CORE::CString::StringMap entries;
    if ( !m_doc.IsNULLOrEmpty() )
        entries[ CORE::CString::Empty ] = m_doc;
    for ( UInt32 e=0; e<m_enums.size(); ++e )
    {
        const CRpcEnumDescriptor& d = *m_enums[ e ];
        if ( !d.m_doc.IsNULLOrEmpty() )
            entries[ d.m_name ] = d.m_doc;
        for ( UInt32 v=0; v<d.m_values.size(); ++v )
            if ( !d.m_values[ v ].doc.IsNULLOrEmpty() )
                entries[ d.m_name + "." + d.m_values[ v ].name ] = d.m_values[ v ].doc;
    }
    for ( UInt32 t=0; t<m_types.size(); ++t )
    {
        const CRpcTypeDescriptor& d = *m_types[ t ];
        if ( !d.m_doc.IsNULLOrEmpty() )
            entries[ d.m_name ] = d.m_doc;
        for ( UInt32 f=0; f<d.m_fields.size(); ++f )
            if ( !d.m_fields[ f ]->m_doc.IsNULLOrEmpty() )
                entries[ d.m_name + "." + d.m_fields[ f ]->m_name ] = d.m_fields[ f ]->m_doc;
    }
    for ( UInt32 s=0; s<m_services.size(); ++s )
    {
        const CRpcServiceDescriptor& d = *m_services[ s ];
        if ( !d.m_doc.IsNULLOrEmpty() )
            entries[ d.m_name ] = d.m_doc;
        for ( UInt32 m=0; m<d.m_methods.size(); ++m )
            if ( !d.m_methods[ m ]->m_doc.IsNULLOrEmpty() )
                entries[ d.m_name + "/" + d.m_methods[ m ]->m_name ] = d.m_methods[ m ]->m_doc;
    }
    if ( entries.empty() )
        return;

    CRpcMessage docs( meta, docsType );
    CORE::CString::StringMap::const_iterator i = entries.begin();
    for ( ; i != entries.end(); ++i )
    {
        CRpcMessage* entry = docs.AddMessage( CD_ENTRIES );
        if ( !(*i).first.IsNULLOrEmpty() )
            entry->SetString( DE_PATH, (*i).first );
        entry->SetString( DE_TEXT, (*i).second );
    }
    docs.Encode( m_docs );
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::LoadDocsMessage( const CRpcMessage& docs )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<docs.GetCount( CD_ENTRIES ); ++i )
    {
        const CRpcMessage* entry = docs.GetMessageAt( CD_ENTRIES, i );
        CORE::CString path = entry->GetString( DE_PATH );
        CORE::CString text = entry->GetString( DE_TEXT );
        if ( path.IsNULLOrEmpty() )
        {
            m_doc = text;
            continue;
        }
        Int32 slash = path.HasChar( '/' );
        if ( slash > 0 )
        {
            CRpcServiceDescriptor* service = const_cast< CRpcServiceDescriptor* >( FindService( path.SubstrToIndex( (UInt32) slash, true ) ) );
            if ( GUCEF_NULL != service )
            {
                CRpcMethodDescriptor* method = const_cast< CRpcMethodDescriptor* >( service->FindMethodByName( path.SubstrToIndex( (UInt32) slash + 1, false ) ) );
                if ( GUCEF_NULL != method )
                    method->m_doc = text;
            }
            continue;
        }
        Int32 dot = path.HasChar( '.' );
        CORE::CString owner = dot > 0 ? path.SubstrToIndex( (UInt32) dot, true ) : path;
        CORE::CString member = dot > 0 ? path.SubstrToIndex( (UInt32) dot + 1, false ) : CORE::CString::Empty;
        for ( UInt32 e=0; e<m_enums.size(); ++e )
        {
            if ( m_enums[ e ]->m_name != owner )
                continue;
            if ( member.IsNULLOrEmpty() )
                m_enums[ e ]->m_doc = text;
            for ( UInt32 v=0; v<m_enums[ e ]->m_values.size(); ++v )
                if ( m_enums[ e ]->m_values[ v ].name == member )
                    m_enums[ e ]->m_values[ v ].doc = text;
        }
        for ( UInt32 t=0; t<m_types.size(); ++t )
        {
            if ( m_types[ t ]->m_name != owner )
                continue;
            if ( member.IsNULLOrEmpty() )
                m_types[ t ]->m_doc = text;
            for ( UInt32 f=0; f<m_types[ t ]->m_fields.size(); ++f )
                if ( m_types[ t ]->m_fields[ f ]->m_name == member )
                    m_types[ t ]->m_fields[ f ]->m_doc = text;
        }
        for ( UInt32 s=0; s<m_services.size(); ++s )
            if ( m_services[ s ]->m_name == owner && member.IsNULLOrEmpty() )
                m_services[ s ]->m_doc = text;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::LoadFromSchemaMessage( const CRpcMessage& schema, CORE::CString& error )
{GUCEF_TRACE;

    Clear();
    m_name = schema.GetString( CS_NAME );
    m_major = schema.GetUInt32( CS_MAJOR );
    m_minor = schema.GetUInt32( CS_MINOR );
    if ( !IsValidContractName( m_name ) )
        return Fail( error, "name", "invalid contract name \"" + m_name + "\"" );
    const CORE::CString prefix = m_name + ".";

    if ( schema.GetCount( CS_TYPES ) > MaxTypes )
        return Fail( error, "types", "more than " + CORE::ToString( MaxTypes ) + " types" );
    if ( schema.GetCount( CS_SERVICES ) > MaxServices )
        return Fail( error, "services", "more than " + CORE::ToString( MaxServices ) + " services" );

    for ( UInt32 e=0; e<schema.GetCount( CS_ENUMS ); ++e )
    {
        const CRpcMessage* es = schema.GetMessageAt( CS_ENUMS, e );
        CRpcEnumDescriptor* descriptor = GUCEF_NEW CRpcEnumDescriptor();
        m_enums.push_back( descriptor );
        CORE::CString fullName = es->GetString( ES_NAME );
        if ( !BeginsWith( fullName, prefix ) )
            return Fail( error, Path( "enums", e ) + ".name", "\"" + fullName + "\" is not in the contract" );
        descriptor->m_name = fullName.CutChars( prefix.Length(), true );
        for ( UInt32 v=0; v<es->GetCount( ES_VALUES ); ++v )
        {
            const CRpcMessage* ev = es->GetMessageAt( ES_VALUES, v );
            CRpcEnumDescriptor::SValue value;
            value.number = ev->GetInt32( EV_NUMBER );
            value.name = ev->GetString( EV_NAME );
            value.since = ev->GetUInt32( EV_SINCE );
            descriptor->m_values.push_back( value );
        }
        for ( UInt32 n=0; n<es->GetCount( ES_RESERVEDNUMBERS ); ++n )
        {
            CORE::CVariant v;
            es->GetAt( ES_RESERVEDNUMBERS, n, v );
            descriptor->m_reservedNumbers.insert( v.AsInt32() );
        }
        for ( UInt32 n=0; n<es->GetCount( ES_RESERVEDNAMES ); ++n )
        {
            CORE::CVariant v;
            es->GetAt( ES_RESERVEDNAMES, n, v );
            descriptor->m_reservedNames.insert( v.AsString() );
        }
    }

    for ( UInt32 t=0; t<schema.GetCount( CS_TYPES ); ++t )
    {
        const CRpcMessage* ts = schema.GetMessageAt( CS_TYPES, t );
        CRpcTypeDescriptor* descriptor = GUCEF_NEW CRpcTypeDescriptor();
        m_types.push_back( descriptor );
        CORE::CString fullName = ts->GetString( TS_NAME );
        if ( !BeginsWith( fullName, prefix ) )
            return Fail( error, Path( "types", t ) + ".name", "\"" + fullName + "\" is not in the contract" );
        descriptor->m_name = fullName.CutChars( prefix.Length(), true );
        if ( ts->GetCount( TS_FIELDS ) > MaxFieldsPerType )
            return Fail( error, Path( "types", t ) + ".fields", "more than " + CORE::ToString( MaxFieldsPerType ) + " fields" );
        for ( UInt32 f=0; f<ts->GetCount( TS_FIELDS ); ++f )
        {
            const CRpcMessage* fs = ts->GetMessageAt( TS_FIELDS, f );
            CRpcFieldDescriptor* field = GUCEF_NEW CRpcFieldDescriptor();
            descriptor->m_fields.push_back( field );
            field->m_number = fs->GetUInt32( FS_NUMBER );
            field->m_name = fs->GetString( FS_NAME );
            UInt32 type = fs->GetUInt32( FS_TYPE );
            UInt32 cardinality = fs->GetUInt32( FS_CARDINALITY );
            UInt32 mapKeyType = fs->GetUInt32( FS_MAPKEYTYPE );
            if ( type > RPC_FIELDTYPE_DATANODE || cardinality > RPC_CARDINALITY_MAP || mapKeyType > RPC_FIELDTYPE_DATANODE )
                return Fail( error, Path( "types", t ) + "." + Path( "fields", f ), "unknown type or cardinality" );
            field->m_type = (TRpcFieldType) type;
            field->m_cardinality = (TRpcCardinality) cardinality;
            field->m_mapKeyType = (TRpcFieldType) mapKeyType;
            field->m_typeRef = fs->GetString( FS_TYPEREF );
            UInt32 scalePlusOne = fs->GetUInt32( FS_DECIMALSCALEPLUSONE );
            if ( scalePlusOne > (UInt32) CORE::CDecimal::MaxScale + 1 )
                return Fail( error, Path( "types", t ) + "." + Path( "fields", f ), "decimal scale above 18" );
            field->m_decimalScale = (Int32) scalePlusOne - 1;
            field->m_flags = fs->GetUInt32( FS_FLAGS );
            field->m_since = fs->GetUInt32( FS_SINCE );
        }
        for ( UInt32 n=0; n<ts->GetCount( TS_RESERVEDNUMBERS ); ++n )
        {
            CORE::CVariant v;
            ts->GetAt( TS_RESERVEDNUMBERS, n, v );
            descriptor->m_reservedNumbers.insert( v.AsUInt32() );
        }
        for ( UInt32 n=0; n<ts->GetCount( TS_RESERVEDNAMES ); ++n )
        {
            CORE::CVariant v;
            ts->GetAt( TS_RESERVEDNAMES, n, v );
            descriptor->m_reservedNames.insert( v.AsString() );
        }
    }

    for ( UInt32 s=0; s<schema.GetCount( CS_SERVICES ); ++s )
    {
        const CRpcMessage* ss = schema.GetMessageAt( CS_SERVICES, s );
        CRpcServiceDescriptor* service = GUCEF_NEW CRpcServiceDescriptor();
        m_services.push_back( service );
        service->m_number = ss->GetUInt32( SS_NUMBER );
        service->m_name = ss->GetString( SS_NAME );
        service->m_since = ss->GetUInt32( SS_SINCE );
        if ( ss->GetCount( SS_METHODS ) > MaxMethodsPerService )
            return Fail( error, Path( "services", s ) + ".methods", "more than " + CORE::ToString( MaxMethodsPerService ) + " methods" );
        for ( UInt32 m=0; m<ss->GetCount( SS_METHODS ); ++m )
        {
            const CRpcMessage* ms = ss->GetMessageAt( SS_METHODS, m );
            CRpcMethodDescriptor* method = GUCEF_NEW CRpcMethodDescriptor();
            service->m_methods.push_back( method );
            method->m_number = ms->GetUInt32( MS_NUMBER );
            method->m_name = ms->GetString( MS_NAME );
            UInt32 kind = ms->GetUInt32( MS_KIND );
            UInt32 protection = ms->GetUInt32( MS_PROTECTION );
            UInt32 requestProtection = ms->GetUInt32( MS_REQUESTPROTECTION );
            if ( kind > RPC_METHODKIND_BIDI_STREAM || protection > RPC_PROTECTION_PUBLIC || requestProtection > RPC_PROTECTION_PUBLIC )
                return Fail( error, Path( "services", s ) + "." + Path( "methods", m ), "unknown kind or protection class" );
            method->m_kind = (TRpcMethodKind) kind;
            method->m_requestTypeName = ms->GetString( MS_REQUESTTYPE );
            method->m_responseTypeName = ms->GetString( MS_RESPONSETYPE );
            method->m_flags = ms->GetUInt32( MS_FLAGS );
            method->m_defaultDeadlineMs = ms->GetUInt32( MS_DEADLINE );
            method->m_protection = (TRpcProtectionClass) protection;
            method->m_requestProtection = (TRpcProtectionClass) requestProtection;
            method->m_initialCredit = ms->GetUInt32( MS_INITIALCREDIT );
            method->m_since = ms->GetUInt32( MS_SINCE );
        }
        for ( UInt32 n=0; n<ss->GetCount( SS_RESERVEDNUMBERS ); ++n )
        {
            CORE::CVariant v;
            ss->GetAt( SS_RESERVEDNUMBERS, n, v );
            service->m_reservedNumbers.insert( v.AsUInt32() );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcContract::FromSchema( const void* schema, UInt32 size, CORE::CString& error, const void* docs, UInt32 docsSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL == schema || size < 5 )
    {
        error = "schema too short";
        return CRpcContractPtr();
    }
    if ( size > MaxSchemaBytes )
    {
        error = "schema larger than 1 MB";
        return CRpcContractPtr();
    }
    const UInt8* bytes = static_cast< const UInt8* >( schema );
    if ( 0 != memcmp( bytes, g_schemaMagic, 4 ) )
    {
        error = "not a GUCEF RPC contract schema";
        return CRpcContractPtr();
    }
    if ( SchemaEncodingVersion != bytes[ 4 ] )
    {
        error = "unsupported schema encoding version " + CORE::ToString( (UInt32) bytes[ 4 ] );
        return CRpcContractPtr();
    }

    CRpcContractPtr meta = CRpcBuiltInContracts::GetRpcContract();
    if ( meta.IsNULL() )
    {
        error = "the gucef.rpc meta-types are not available";
        return CRpcContractPtr();
    }
    CRpcMessage message( meta, meta->FindType( CRpcBuiltInContracts::ContractSchemaType ) );
    CRpcStatus status;
    if ( !message.Decode( bytes + 5, size - 5, status ) )
    {
        error = "schema not decodable: " + status.ToString();
        return CRpcContractPtr();
    }

    CRpcContract* contract = GUCEF_NEW CRpcContract();
    CRpcContractPtr ptr = contract->CreateSharedPtr();
    if ( !contract->LoadFromSchemaMessage( message, error ) || !contract->Finalize( error ) )
        return CRpcContractPtr();

    // the identity is the bytes as received, so a schema with fields of a newer schema encoding keeps its hash
    contract->m_schema.CopyFrom( size, schema );
    contract->m_hash = CRpcContractHash::Compute( schema, size );

    if ( GUCEF_NULL != docs && docsSize > 0 )
    {
        CRpcMessage docsMessage( meta, meta->FindType( CRpcBuiltInContracts::ContractDocsType ) );
        if ( docsMessage.Decode( docs, docsSize, status ) )
        {
            contract->LoadDocsMessage( docsMessage );
            contract->m_docs.CopyFrom( docsSize, docs );
        }
    }
    return ptr;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::SaveToDataNode( CORE::CDataNode& target ) const
{GUCEF_TRACE;

    target.Clear();
    target.SetNodeType( GUCEF_DATATYPE_OBJECT );
    target.SetAttribute( "contract", m_name );
    target.SetAttribute( "version", CORE::ToString( m_major ) + "." + CORE::ToString( m_minor ) );
    if ( !m_doc.IsNULLOrEmpty() )
        target.SetAttribute( "doc", m_doc );

    if ( !m_enums.empty() )
    {
        CORE::CDataNode* enums = target.AddChild( "enums", GUCEF_DATATYPE_ARRAY );
        for ( UInt32 e=0; e<m_enums.size(); ++e )
        {
            const CRpcEnumDescriptor& d = *m_enums[ e ];
            CORE::CDataNode* node = enums->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            node->SetAttribute( "name", d.m_name );
            if ( !d.m_doc.IsNULLOrEmpty() )
                node->SetAttribute( "doc", d.m_doc );
            CORE::CDataNode* values = node->AddChild( "values", GUCEF_DATATYPE_ARRAY );
            for ( UInt32 v=0; v<d.m_values.size(); ++v )
            {
                CORE::CDataNode* value = values->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                value->SetAttribute( "name", d.m_values[ v ].name );
                value->SetAttribute( "number", (Int64) d.m_values[ v ].number );
                if ( 0 != d.m_values[ v ].since )
                    value->SetAttribute( "since", (Int64) d.m_values[ v ].since );
                if ( !d.m_values[ v ].doc.IsNULLOrEmpty() )
                    value->SetAttribute( "doc", d.m_values[ v ].doc );
            }
            WriteReserved( *node, d.m_reservedNumbers, &d.m_reservedNames );
        }
    }

    if ( !m_types.empty() )
    {
        CORE::CDataNode* types = target.AddChild( "types", GUCEF_DATATYPE_ARRAY );
        for ( UInt32 t=0; t<m_types.size(); ++t )
        {
            const CRpcTypeDescriptor& d = *m_types[ t ];
            CORE::CDataNode* node = types->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            node->SetAttribute( "name", d.m_name );
            if ( !d.m_doc.IsNULLOrEmpty() )
                node->SetAttribute( "doc", d.m_doc );
            CORE::CDataNode* fields = node->AddChild( "fields", GUCEF_DATATYPE_ARRAY );
            for ( UInt32 f=0; f<d.m_fields.size(); ++f )
            {
                const CRpcFieldDescriptor& field = *d.m_fields[ f ];
                CORE::CDataNode* fieldNode = fields->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                fieldNode->SetAttribute( "number", (Int64) field.m_number );
                fieldNode->SetAttribute( "name", field.m_name );

                CORE::CString valueType;
                if ( !field.m_typeRef.IsNULLOrEmpty() )
                    valueType = BeginsWith( field.m_typeRef, m_name + "." ) ? field.m_typeRef.CutChars( m_name.Length() + 1, true ) : field.m_typeRef;
                else if ( field.m_decimalScale >= 0 )
                    valueType = "decimal(" + CORE::ToString( field.m_decimalScale ) + ")";
                else
                    valueType = RpcFieldTypeToString( field.m_type );
                if ( field.IsMap() )
                    fieldNode->SetAttribute( "type", "map<" + CORE::CString( RpcFieldTypeToString( field.m_mapKeyType ) ) + "," + valueType + ">" );
                else
                    fieldNode->SetAttribute( "type", valueType );
                if ( RPC_CARDINALITY_OPTIONAL == field.m_cardinality )
                    fieldNode->SetAttribute( "cardinality", "optional" );
                else if ( RPC_CARDINALITY_REPEATED == field.m_cardinality )
                    fieldNode->SetAttribute( "cardinality", "repeated" );
                if ( field.IsRequired() )
                    fieldNode->SetAttribute( "required", true );
                if ( field.IsSensitive() )
                    fieldNode->SetAttribute( "sensitive", true );
                if ( 0 != ( field.m_flags & GUCEF_RPC_FIELDFLAG_DEPRECATED ) )
                    fieldNode->SetAttribute( "deprecated", true );
                if ( 0 != field.m_since )
                    fieldNode->SetAttribute( "since", (Int64) field.m_since );
                if ( !field.m_doc.IsNULLOrEmpty() )
                    fieldNode->SetAttribute( "doc", field.m_doc );
            }
            WriteReserved( *node, d.m_reservedNumbers, &d.m_reservedNames );
        }
    }

    if ( !m_services.empty() )
    {
        CORE::CDataNode* services = target.AddChild( "services", GUCEF_DATATYPE_ARRAY );
        for ( UInt32 s=0; s<m_services.size(); ++s )
        {
            const CRpcServiceDescriptor& d = *m_services[ s ];
            CORE::CDataNode* node = services->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            node->SetAttribute( "name", d.m_name );
            node->SetAttribute( "number", (Int64) d.m_number );
            if ( 0 != d.m_since )
                node->SetAttribute( "since", (Int64) d.m_since );
            if ( !d.m_doc.IsNULLOrEmpty() )
                node->SetAttribute( "doc", d.m_doc );
            CORE::CDataNode* methods = node->AddChild( "methods", GUCEF_DATATYPE_ARRAY );
            for ( UInt32 m=0; m<d.m_methods.size(); ++m )
            {
                const CRpcMethodDescriptor& method = *d.m_methods[ m ];
                CORE::CDataNode* methodNode = methods->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                methodNode->SetAttribute( "number", (Int64) method.m_number );
                methodNode->SetAttribute( "name", method.m_name );
                methodNode->SetAttribute( "kind", RpcMethodKindToString( method.m_kind ) );
                methodNode->SetAttribute( "request", method.m_requestTypeName.CutChars( m_name.Length() + 1, true ) );
                methodNode->SetAttribute( "response", method.m_responseTypeName.CutChars( m_name.Length() + 1, true ) );
                if ( 0 != method.m_flags )
                {
                    CORE::CDataNode* flags = methodNode->AddChild( "flags", GUCEF_DATATYPE_ARRAY );
                    if ( method.IsIdempotent() ) flags->AddValueAsChild( CORE::CVariant( "idempotent" ) );
                    if ( method.MayAnswerAsync() ) flags->AddValueAsChild( CORE::CVariant( "mayAnswerAsync" ) );
                    if ( method.IsResumableStream() ) flags->AddValueAsChild( CORE::CVariant( "resumableStream" ) );
                    if ( method.IsSensitive() ) flags->AddValueAsChild( CORE::CVariant( "sensitive" ) );
                }
                if ( 0 != method.m_defaultDeadlineMs )
                    methodNode->SetAttribute( "deadlineMs", (Int64) method.m_defaultDeadlineMs );
                if ( RPC_PROTECTION_PUBLIC == method.m_protection )
                    methodNode->SetAttribute( "protection", "public" );
                if ( RPC_PROTECTION_PUBLIC == method.m_requestProtection )
                    methodNode->SetAttribute( "requestProtection", "public" );
                if ( 0 != method.m_initialCredit )
                    methodNode->SetAttribute( "initialCredit", (Int64) method.m_initialCredit );
                if ( 0 != method.m_since )
                    methodNode->SetAttribute( "since", (Int64) method.m_since );
                if ( !method.m_doc.IsNULLOrEmpty() )
                    methodNode->SetAttribute( "doc", method.m_doc );
            }
            GUCEF::set< UInt32 > reserved( d.m_reservedNumbers );
            WriteReserved( *node, reserved, GUCEF_NULL );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcContract::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcContract::GetMajor( void ) const
{GUCEF_TRACE;

    return m_major;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcContract::GetMinor( void ) const
{GUCEF_TRACE;

    return m_minor;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcContract::GetDoc( void ) const
{GUCEF_TRACE;

    return m_doc;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcContract::GetDisplayName( void ) const
{GUCEF_TRACE;

    return m_name + " " + CORE::ToString( m_major ) + "." + CORE::ToString( m_minor );
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CRpcContract::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

const CRpcContractHash&
CRpcContract::GetHash( void ) const
{GUCEF_TRACE;

    return m_hash;
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CRpcContract::GetDocs( void ) const
{GUCEF_TRACE;

    return m_docs;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContract::HasDocs( void ) const
{GUCEF_TRACE;

    return m_docs.GetDataSize() > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcContract::GetEnumCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_enums.size();
}

/*-------------------------------------------------------------------------*/

const CRpcEnumDescriptor&
CRpcContract::GetEnumAt( UInt32 index ) const
{GUCEF_TRACE;

    return *m_enums[ index ];
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcContract::GetTypeCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_types.size();
}

/*-------------------------------------------------------------------------*/

const CRpcTypeDescriptor&
CRpcContract::GetTypeAt( UInt32 index ) const
{GUCEF_TRACE;

    return *m_types[ index ];
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcContract::GetServiceCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_services.size();
}

/*-------------------------------------------------------------------------*/

const CRpcServiceDescriptor&
CRpcContract::GetServiceAt( UInt32 index ) const
{GUCEF_TRACE;

    return *m_services[ index ];
}

/*-------------------------------------------------------------------------*/

const CRpcTypeDescriptor*
CRpcContract::FindType( const CORE::CString& name ) const
{GUCEF_TRACE;

    CORE::CString fullName = ResolveTypeName( name );
    for ( UInt32 i=0; i<m_types.size(); ++i )
    {
        const CRpcTypeDescriptor* t = m_types[ i ];
        if ( t->m_fullName == fullName || ( t->m_fullName.IsNULLOrEmpty() && m_name + "." + t->m_name == fullName ) )
            return t;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcEnumDescriptor*
CRpcContract::FindEnum( const CORE::CString& name ) const
{GUCEF_TRACE;

    CORE::CString fullName = ResolveTypeName( name );
    for ( UInt32 i=0; i<m_enums.size(); ++i )
    {
        const CRpcEnumDescriptor* e = m_enums[ i ];
        if ( e->m_fullName == fullName || ( e->m_fullName.IsNULLOrEmpty() && m_name + "." + e->m_name == fullName ) )
            return e;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcServiceDescriptor*
CRpcContract::FindService( const CORE::CString& name ) const
{GUCEF_TRACE;

    CORE::CString fullName = ResolveTypeName( name );
    for ( UInt32 i=0; i<m_services.size(); ++i )
        if ( m_name + "." + m_services[ i ]->m_name == fullName )
            return m_services[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcServiceDescriptor*
CRpcContract::FindServiceByNumber( UInt32 number ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_services.size(); ++i )
        if ( m_services[ i ]->m_number == number )
            return m_services[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcMethodDescriptor*
CRpcContract::FindMethod( const CORE::CString& methodPath ) const
{GUCEF_TRACE;

    Int32 slash = methodPath.HasChar( '/' );
    if ( slash <= 0 )
        return GUCEF_NULL;
    const CRpcServiceDescriptor* service = FindService( methodPath.SubstrToIndex( (UInt32) slash, true ) );
    if ( GUCEF_NULL == service )
        return GUCEF_NULL;
    return service->FindMethodByName( methodPath.SubstrToIndex( (UInt32) slash + 1, false ) );
}

/*-------------------------------------------------------------------------*/

CRpcMessage
CRpcContract::NewMessage( const CORE::CString& typeName ) const
{GUCEF_TRACE;

    const CRpcTypeDescriptor* type = FindType( typeName );
    if ( GUCEF_NULL == type )
        return CRpcMessage();
    return CRpcMessage( CreateSharedPtr(), type );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
