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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#include "gucefCOMCORE_CRpcBuiltInContracts.h"

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

const char* CRpcBuiltInContracts::RpcContractName = "gucef.rpc";
const char* CRpcBuiltInContracts::ContractSchemaType = "gucef.rpc.ContractSchema";
const char* CRpcBuiltInContracts::ContractDocsType = "gucef.rpc.ContractDocs";
const char* CRpcBuiltInContracts::ReflectionService = "gucef.rpc.Reflection";

/*-------------------------------------------------------------------------*/

namespace {

MT::CMutex g_builtInLock;
CRpcContractPtr* g_rpcContract = GUCEF_NULL;

/** Releases the contract at the end of the process */
class CBuiltInContractsCleanup
{
    public:
    ~CBuiltInContractsCleanup()
    {
        GUCEF_DELETE g_rpcContract;
        g_rpcContract = GUCEF_NULL;
    }
};
CBuiltInContractsCleanup g_cleanup;

}

/*-------------------------------------------------------------------------*/

void
CRpcBuiltInContracts::GetRpcContractSource( CORE::CDataNode& source )
{GUCEF_TRACE;

    CRpcContractBuilder b( RpcContractName, 1, 0 );
    b.SetDoc( "GUCEF RPC built-in contract: reflection and the meta-types of the canonical contract schema" );

    // meta-types of the canonical schema encoding (design 3.8)
    b.AddType( "ContractSchema" );
    b.AddField( "ContractSchema", 1, "name", "string" );
    b.AddField( "ContractSchema", 2, "major", "uint32" );
    b.AddField( "ContractSchema", 3, "minor", "uint32" );
    b.AddField( "ContractSchema", 4, "enums", "EnumSchema", RPC_CARDINALITY_REPEATED );
    b.AddField( "ContractSchema", 5, "types", "TypeSchema", RPC_CARDINALITY_REPEATED );
    b.AddField( "ContractSchema", 6, "services", "ServiceSchema", RPC_CARDINALITY_REPEATED );

    b.AddType( "EnumSchema" );
    b.AddField( "EnumSchema", 1, "name", "string" );
    b.AddField( "EnumSchema", 2, "values", "EnumValueSchema", RPC_CARDINALITY_REPEATED );
    b.AddField( "EnumSchema", 3, "reservedNumbers", "int32", RPC_CARDINALITY_REPEATED );
    b.AddField( "EnumSchema", 4, "reservedNames", "string", RPC_CARDINALITY_REPEATED );

    b.AddType( "EnumValueSchema" );
    b.AddField( "EnumValueSchema", 1, "number", "int32" );
    b.AddField( "EnumValueSchema", 2, "name", "string" );
    b.AddField( "EnumValueSchema", 3, "since", "uint32" );

    b.AddType( "TypeSchema" );
    b.AddField( "TypeSchema", 1, "name", "string" );
    b.AddField( "TypeSchema", 2, "fields", "FieldSchema", RPC_CARDINALITY_REPEATED );
    b.AddField( "TypeSchema", 3, "reservedNumbers", "uint32", RPC_CARDINALITY_REPEATED );
    b.AddField( "TypeSchema", 4, "reservedNames", "string", RPC_CARDINALITY_REPEATED );

    b.AddType( "FieldSchema" );
    b.AddField( "FieldSchema", 1, "number", "uint32" );
    b.AddField( "FieldSchema", 2, "name", "string" );
    b.AddField( "FieldSchema", 3, "type", "uint32" );
    b.AddField( "FieldSchema", 4, "cardinality", "uint32" );
    b.AddField( "FieldSchema", 5, "typeRef", "string" );
    b.AddField( "FieldSchema", 6, "mapKeyType", "uint32" );
    b.AddField( "FieldSchema", 7, "decimalScalePlusOne", "uint32" );
    b.AddField( "FieldSchema", 8, "flags", "uint32" );
    b.AddField( "FieldSchema", 9, "since", "uint32" );

    b.AddType( "ServiceSchema" );
    b.AddField( "ServiceSchema", 1, "number", "uint32" );
    b.AddField( "ServiceSchema", 2, "name", "string" );
    b.AddField( "ServiceSchema", 3, "methods", "MethodSchema", RPC_CARDINALITY_REPEATED );
    b.AddField( "ServiceSchema", 4, "reservedNumbers", "uint32", RPC_CARDINALITY_REPEATED );
    b.AddField( "ServiceSchema", 5, "since", "uint32" );

    b.AddType( "MethodSchema" );
    b.AddField( "MethodSchema", 1, "number", "uint32" );
    b.AddField( "MethodSchema", 2, "name", "string" );
    b.AddField( "MethodSchema", 3, "kind", "uint32" );
    b.AddField( "MethodSchema", 4, "requestType", "string" );
    b.AddField( "MethodSchema", 5, "responseType", "string" );
    b.AddField( "MethodSchema", 6, "flags", "uint32" );
    b.AddField( "MethodSchema", 7, "defaultDeadlineMs", "uint32" );
    b.AddField( "MethodSchema", 8, "protection", "uint32" );
    b.AddField( "MethodSchema", 9, "requestProtection", "uint32" );
    b.AddField( "MethodSchema", 10, "initialCredit", "uint32" );
    b.AddField( "MethodSchema", 11, "since", "uint32" );

    b.AddType( "ContractDocs" );
    b.AddField( "ContractDocs", 1, "entries", "DocEntry", RPC_CARDINALITY_REPEATED );
    b.AddType( "DocEntry" );
    b.AddField( "DocEntry", 1, "path", "string" );
    b.AddField( "DocEntry", 2, "text", "string" );

    // reflection (design 11)
    b.AddType( "ListContractsRequest" );
    b.AddType( "ContractInfo" );
    b.AddField( "ContractInfo", 1, "name", "string" );
    b.AddField( "ContractInfo", 2, "major", "uint32" );
    b.AddField( "ContractInfo", 3, "minor", "uint32" );
    b.AddField( "ContractInfo", 4, "hash", "bytes" );
    b.AddField( "ContractInfo", 5, "services", "string", RPC_CARDINALITY_REPEATED );
    b.AddType( "ListContractsResponse" );
    b.AddField( "ListContractsResponse", 1, "contracts", "ContractInfo", RPC_CARDINALITY_REPEATED );
    b.AddType( "GetContractRequest" );
    b.AddField( "GetContractRequest", 1, "hash", "bytes" );
    b.AddField( "GetContractRequest", 2, "name", "string" );
    b.AddField( "GetContractRequest", 3, "major", "uint32" );
    b.AddField( "GetContractRequest", 4, "minor", "uint32" );
    b.AddField( "GetContractRequest", 5, "includeDocs", "bool" );
    b.AddType( "GetContractResponse" );
    b.AddField( "GetContractResponse", 1, "schema", "bytes" );
    b.AddField( "GetContractResponse", 2, "docs", "bytes" );

    b.AddService( "Reflection", 1 );
    b.AddMethod( "Reflection", 1, "ListContracts", RPC_METHODKIND_UNARY, "ListContractsRequest", "ListContractsResponse", GUCEF_RPC_METHODFLAG_IDEMPOTENT );
    b.AddMethod( "Reflection", 2, "GetContract", RPC_METHODKIND_UNARY, "GetContractRequest", "GetContractResponse", GUCEF_RPC_METHODFLAG_IDEMPOTENT );

    source.Copy( b.GetDataNode() );
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcBuiltInContracts::GetRpcContract( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_builtInLock );
    if ( GUCEF_NULL != g_rpcContract )
        return *g_rpcContract;

    // Built without the schema first: its own schema is encoded with its own meta-types
    CORE::CDataNode source;
    GetRpcContractSource( source );
    CRpcContract* contract = GUCEF_NEW CRpcContract();
    CRpcContractPtr ptr = contract->CreateSharedPtr();
    CORE::CString error;
    if ( !contract->LoadFromDataNodeImpl( source, error ) || !contract->Finalize( error ) || !contract->BuildSchema( *contract, error ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "RpcBuiltInContracts: gucef.rpc 1.0 cannot be built: " << error );
        return CRpcContractPtr();
    }
    contract->m_isBuiltIn = true;
    g_rpcContract = GUCEF_NEW CRpcContractPtr( ptr );

    // the docs are a gucef.rpc message too: the lock is recursive, the lookup inside finds the contract set above
    contract->BuildDocs();
    return ptr;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
