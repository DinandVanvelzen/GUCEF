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

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#include "gucefCOMCORE_CRpcReflectionService.h"

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

CRpcReflectionService::CRpcReflectionService( void )
    : CIRpcServiceImpl()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcReflectionService::~CRpcReflectionService()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TRpcHandlerResult
CRpcReflectionService::OnCall( CRpcServerCall& call )
{GUCEF_TRACE;

    CRpcEndpointLink::CScope scope( call.m_link );
    CRpcEndpoint* endpoint = scope.GetEndpoint();
    if ( GUCEF_NULL == endpoint )
        return RPC_HANDLER_DONE;

    // what this side exports, and gucef.rpc itself, as far as the principal may see it
    TRpcContractPtrVector contracts;
    CIRpcAccessPolicyPtr policy;
    {
        MT::CScopeMutex lock( endpoint->m_lock );
        endpoint->m_services->GetExportedContracts( contracts );
        if ( !endpoint->m_rpcContract.IsNULL() )
            contracts.push_back( endpoint->m_rpcContract );
        policy = endpoint->m_accessPolicy;
    }
    TRpcContractPtrVector visible;
    for ( UInt32 i=0; i<contracts.size(); ++i )
        if ( policy.IsNULL() || policy->MayBindContract( call.GetPrincipal(), *contracts[ i ] ) )
            visible.push_back( contracts[ i ] );

    CRpcMessage response = call.NewResponse();
    const CORE::CString& method = call.GetMethod().GetName();
    if ( "ListContracts" == method )
    {
        for ( UInt32 i=0; i<visible.size(); ++i )
        {
            const CRpcContract& contract = *visible[ i ];
            CRpcMessage* info = response.AddMessage( 1 );
            info->SetString( 1, contract.GetName() );
            info->SetUInt32( 2, contract.GetMajor() );
            info->SetUInt32( 3, contract.GetMinor() );
            info->SetBytes( 4, contract.GetHash().GetBytes(), CRpcContractHash::Size );
            for ( UInt32 s=0; s<contract.GetServiceCount(); ++s )
                info->Add( 5, CORE::CVariant( contract.GetServiceAt( s ).GetFullName() ) );
        }
        call.Reply( response );
        return RPC_HANDLER_DONE;
    }
    if ( "GetContract" == method )
    {
        const CRpcMessage& request = call.GetRequest();
        CORE::CDynamicBuffer hashBytes;
        request.GetBytes( 1, hashBytes );
        CORE::CString name = request.GetString( 2 );
        UInt32 major = request.GetUInt32( 3 );
        UInt32 minor = request.GetUInt32( 4 );
        bool includeDocs = request.GetBool( 5 );

        CRpcContractPtr found;
        for ( UInt32 i=0; i<visible.size() && found.IsNULL(); ++i )
        {
            const CRpcContract& contract = *visible[ i ];
            if ( CRpcContractHash::Size == hashBytes.GetDataSize() )
            {
                if ( contract.GetHash() == CRpcContractHash( static_cast< const UInt8* >( hashBytes.GetConstBufferPtr() ) ) )
                    found = visible[ i ];
            }
            else if ( contract.GetName() == name && contract.GetMajor() == major && ( 0 == minor || contract.GetMinor() == minor ) )
                found = visible[ i ];
        }
        if ( found.IsNULL() )
        {
            call.Fail( CRpcStatus( RPC_STATUS_NOT_FOUND, "no such contract" ) );
            return RPC_HANDLER_DONE;
        }
        response.SetBytes( 1, found->GetSchema() );
        if ( includeDocs && found->HasDocs() )
            response.SetBytes( 2, found->GetDocs() );
        call.Reply( response );
        return RPC_HANDLER_DONE;
    }
    call.Fail( CRpcStatus( RPC_STATUS_UNIMPLEMENTED, "unknown reflection method" ) );
    return RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
