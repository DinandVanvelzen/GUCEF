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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOMCORE_CRpcContractBuilder.h"

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

CRpcContractBuilder::CRpcContractBuilder( const CORE::CString& contractName, UInt32 major, UInt32 minor )
    : m_root( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

    m_root.SetAttribute( "contract", contractName );
    m_root.SetAttribute( "version", CORE::ToString( major ) + "." + CORE::ToString( minor ) );
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder::~CRpcContractBuilder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::SetDoc( const CORE::CString& doc )
{GUCEF_TRACE;

    m_root.SetAttribute( "doc", doc );
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode*
CRpcContractBuilder::FindNamed( const CORE::CString& listName, const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CDataNode* list = m_root.FindChild( listName );
    if ( GUCEF_NULL == list )
        return GUCEF_NULL;
    CORE::CDataNode::iterator i = list->Begin();
    while ( i != list->End() )
    {
        if ( (*i)->GetAttributeValue( "name" ).AsString() == name )
            return (*i);
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::AddEnum( const CORE::CString& enumName, const CORE::CString& doc )
{GUCEF_TRACE;

    CORE::CDataNode* list = m_root.FindOrAddChild( "enums", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* node = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    node->SetAttribute( "name", enumName );
    if ( !doc.IsNULLOrEmpty() )
        node->SetAttribute( "doc", doc );
    node->AddChild( "values", GUCEF_DATATYPE_ARRAY );
    return *this;
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::AddEnumValue( const CORE::CString& enumName, Int32 number, const CORE::CString& valueName, UInt32 since )
{GUCEF_TRACE;

    CORE::CDataNode* node = FindNamed( "enums", enumName );
    if ( GUCEF_NULL == node )
    {
        AddEnum( enumName );
        node = FindNamed( "enums", enumName );
    }
    CORE::CDataNode* value = node->FindOrAddChild( "values", GUCEF_DATATYPE_ARRAY )->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    value->SetAttribute( "name", valueName );
    value->SetAttribute( "number", (Int64) number );
    if ( 0 != since )
        value->SetAttribute( "since", (Int64) since );
    return *this;
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::AddType( const CORE::CString& typeName, const CORE::CString& doc )
{GUCEF_TRACE;

    CORE::CDataNode* list = m_root.FindOrAddChild( "types", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* node = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    node->SetAttribute( "name", typeName );
    if ( !doc.IsNULLOrEmpty() )
        node->SetAttribute( "doc", doc );
    node->AddChild( "fields", GUCEF_DATATYPE_ARRAY );
    return *this;
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::AddField( const CORE::CString& typeName, UInt32 number, const CORE::CString& fieldName,
                               const CORE::CString& typeString, TRpcCardinality cardinality, UInt32 since, UInt32 fieldFlags )
{GUCEF_TRACE;

    CORE::CDataNode* node = FindNamed( "types", typeName );
    if ( GUCEF_NULL == node )
    {
        AddType( typeName );
        node = FindNamed( "types", typeName );
    }
    CORE::CDataNode* field = node->FindOrAddChild( "fields", GUCEF_DATATYPE_ARRAY )->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    field->SetAttribute( "number", (Int64) number );
    field->SetAttribute( "name", fieldName );
    field->SetAttribute( "type", typeString );
    if ( RPC_CARDINALITY_OPTIONAL == cardinality )
        field->SetAttribute( "cardinality", "optional" );
    else if ( RPC_CARDINALITY_REPEATED == cardinality )
        field->SetAttribute( "cardinality", "repeated" );
    if ( 0 != since )
        field->SetAttribute( "since", (Int64) since );
    if ( 0 != ( fieldFlags & GUCEF_RPC_FIELDFLAG_REQUIRED ) )
        field->SetAttribute( "required", true );
    if ( 0 != ( fieldFlags & GUCEF_RPC_FIELDFLAG_SENSITIVE ) )
        field->SetAttribute( "sensitive", true );
    if ( 0 != ( fieldFlags & GUCEF_RPC_FIELDFLAG_DEPRECATED ) )
        field->SetAttribute( "deprecated", true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::AddReservedFieldNumber( const CORE::CString& typeName, UInt32 number )
{GUCEF_TRACE;

    CORE::CDataNode* node = FindNamed( "types", typeName );
    if ( GUCEF_NULL != node )
        node->FindOrAddChild( "reserved", GUCEF_DATATYPE_OBJECT )->FindOrAddChild( "numbers", GUCEF_DATATYPE_ARRAY )->AddValueAsChild( CORE::CVariant( (Int64) number ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::AddService( const CORE::CString& serviceName, UInt32 number, UInt32 since )
{GUCEF_TRACE;

    CORE::CDataNode* list = m_root.FindOrAddChild( "services", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* node = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    node->SetAttribute( "name", serviceName );
    node->SetAttribute( "number", (Int64) number );
    if ( 0 != since )
        node->SetAttribute( "since", (Int64) since );
    node->AddChild( "methods", GUCEF_DATATYPE_ARRAY );
    return *this;
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::AddMethod( const CORE::CString& serviceName, UInt32 number, const CORE::CString& methodName,
                                TRpcMethodKind kind, const CORE::CString& requestType, const CORE::CString& responseType,
                                UInt32 methodFlags, UInt32 deadlineMs, UInt32 since )
{GUCEF_TRACE;

    CORE::CDataNode* node = FindNamed( "services", serviceName );
    if ( GUCEF_NULL == node )
        return *this;
    CORE::CDataNode* method = node->FindOrAddChild( "methods", GUCEF_DATATYPE_ARRAY )->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    method->SetAttribute( "number", (Int64) number );
    method->SetAttribute( "name", methodName );
    method->SetAttribute( "kind", RpcMethodKindToString( kind ) );
    method->SetAttribute( "request", requestType );
    method->SetAttribute( "response", responseType );
    if ( 0 != methodFlags )
    {
        CORE::CDataNode* flags = method->AddChild( "flags", GUCEF_DATATYPE_ARRAY );
        if ( 0 != ( methodFlags & GUCEF_RPC_METHODFLAG_IDEMPOTENT ) ) flags->AddValueAsChild( CORE::CVariant( "idempotent" ) );
        if ( 0 != ( methodFlags & GUCEF_RPC_METHODFLAG_MAYANSWERASYNC ) ) flags->AddValueAsChild( CORE::CVariant( "mayAnswerAsync" ) );
        if ( 0 != ( methodFlags & GUCEF_RPC_METHODFLAG_RESUMABLESTREAM ) ) flags->AddValueAsChild( CORE::CVariant( "resumableStream" ) );
        if ( 0 != ( methodFlags & GUCEF_RPC_METHODFLAG_SENSITIVE ) ) flags->AddValueAsChild( CORE::CVariant( "sensitive" ) );
    }
    if ( 0 != deadlineMs )
        method->SetAttribute( "deadlineMs", (Int64) deadlineMs );
    if ( 0 != since )
        method->SetAttribute( "since", (Int64) since );
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode*
CRpcContractBuilder::FindMethod( const CORE::CString& serviceName, const CORE::CString& methodName )
{GUCEF_TRACE;

    CORE::CDataNode* service = FindNamed( "services", serviceName );
    if ( GUCEF_NULL == service )
        return GUCEF_NULL;
    CORE::CDataNode* methods = service->FindChild( "methods" );
    if ( GUCEF_NULL == methods )
        return GUCEF_NULL;
    CORE::CDataNode::iterator i = methods->Begin();
    while ( i != methods->End() )
    {
        if ( (*i)->GetAttributeValue( "name" ).AsString() == methodName )
            return (*i);
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::SetMethodProtection( const CORE::CString& serviceName, const CORE::CString& methodName,
                                          TRpcProtectionClass protection, TRpcProtectionClass requestProtection )
{GUCEF_TRACE;

    CORE::CDataNode* method = FindMethod( serviceName, methodName );
    if ( GUCEF_NULL != method )
    {
        method->SetAttribute( "protection", RPC_PROTECTION_PUBLIC == protection ? "public" : "confidential" );
        method->SetAttribute( "requestProtection", RPC_PROTECTION_PUBLIC == requestProtection ? "public" : "confidential" );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CRpcContractBuilder&
CRpcContractBuilder::SetMethodInitialCredit( const CORE::CString& serviceName, const CORE::CString& methodName, UInt32 credit )
{GUCEF_TRACE;

    CORE::CDataNode* method = FindMethod( serviceName, methodName );
    if ( GUCEF_NULL != method )
        method->SetAttribute( "initialCredit", (Int64) credit );
    return *this;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CRpcContractBuilder::GetDataNode( void ) const
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcContractBuilder::Build( CORE::CString& error ) const
{GUCEF_TRACE;

    return CRpcContract::LoadFromDataNode( m_root, error );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
