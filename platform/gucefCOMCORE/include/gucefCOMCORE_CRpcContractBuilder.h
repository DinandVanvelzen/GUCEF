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

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Builds a contract in code (gucefCOMCORE RPC design 3.4, 3.6), for built-in contracts that should not need the
 *  JSON codec. It assembles the JSON authoring form (CDataNode) and builds it with CRpcContract::LoadFromDataNode(),
 *  so a built contract passes the same validation as an authored one.
 *
 *      CRpcContractBuilder builder( "gucef.mcp", 1, 0 );
 *      builder.AddType( "McpMessage" );
 *      builder.AddField( "McpMessage", 1, "json", "string" );
 *      builder.AddService( "Session", 1 );
 *      builder.AddMethod( "Session", 1, "Exchange", RPC_METHODKIND_BIDI_STREAM, "McpMessage", "McpMessage" );
 *      CRpcContractPtr contract = builder.Build( error );
 *
 *  Type strings are those of the authoring format: "uint64", "decimal(4)", "map<string,Row>", a type or enum name.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcContractBuilder
{
    public:

    CRpcContractBuilder( const CORE::CString& contractName, UInt32 major, UInt32 minor );
    ~CRpcContractBuilder();

    CRpcContractBuilder& SetDoc( const CORE::CString& doc );

    CRpcContractBuilder& AddEnum( const CORE::CString& enumName, const CORE::CString& doc = CORE::CString::Empty );
    CRpcContractBuilder& AddEnumValue( const CORE::CString& enumName, Int32 number, const CORE::CString& valueName, UInt32 since = 0 );

    CRpcContractBuilder& AddType( const CORE::CString& typeName, const CORE::CString& doc = CORE::CString::Empty );
    CRpcContractBuilder& AddField( const CORE::CString& typeName, UInt32 number, const CORE::CString& fieldName,
                                   const CORE::CString& typeString, TRpcCardinality cardinality = RPC_CARDINALITY_SINGLE,
                                   UInt32 since = 0, UInt32 fieldFlags = 0 );
    CRpcContractBuilder& AddReservedFieldNumber( const CORE::CString& typeName, UInt32 number );

    CRpcContractBuilder& AddService( const CORE::CString& serviceName, UInt32 number, UInt32 since = 0 );
    CRpcContractBuilder& AddMethod( const CORE::CString& serviceName, UInt32 number, const CORE::CString& methodName,
                                    TRpcMethodKind kind, const CORE::CString& requestType, const CORE::CString& responseType,
                                    UInt32 methodFlags = 0, UInt32 deadlineMs = 0, UInt32 since = 0 );
    CRpcContractBuilder& SetMethodProtection( const CORE::CString& serviceName, const CORE::CString& methodName,
                                              TRpcProtectionClass protection, TRpcProtectionClass requestProtection );
    CRpcContractBuilder& SetMethodInitialCredit( const CORE::CString& serviceName, const CORE::CString& methodName, UInt32 credit );

    /** The authoring form assembled so far */
    const CORE::CDataNode& GetDataNode( void ) const;

    /** Builds and validates the contract; NULL with the error otherwise */
    CRpcContractPtr Build( CORE::CString& error ) const;

    private:

    CORE::CDataNode* FindNamed( const CORE::CString& listName, const CORE::CString& name );
    CORE::CDataNode* FindMethod( const CORE::CString& serviceName, const CORE::CString& methodName );

    CRpcContractBuilder( const CRpcContractBuilder& src );                /**< not implemented */
    CRpcContractBuilder& operator=( const CRpcContractBuilder& src );     /**< not implemented */

    CORE::CDataNode m_root;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */
