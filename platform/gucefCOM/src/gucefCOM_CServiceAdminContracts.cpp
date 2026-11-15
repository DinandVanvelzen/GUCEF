/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#include "gucefCOM_CServiceAdminContracts.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CServiceAdminContracts::ContractName = "gucef.service.admin";
const char* CServiceAdminContracts::ServiceName = "Admin";
const char* CServiceAdminContracts::RunMethodPath = "gucef.service.admin.Admin/Run";

/*-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
CServiceAdminContracts::GetContract( void )
{GUCEF_TRACE;

    static MT::CMutex contractLock;
    static COMCORE::CRpcContractPtr* contract = GUCEF_NULL;
    MT::CScopeMutex lock( contractLock );
    if ( GUCEF_NULL != contract )
        return *contract;

    COMCORE::CRpcContractBuilder builder( ContractName, 1, 0 );
    builder.SetDoc( "The loopback admin endpoint of a service: one command with its arguments, authenticated by the token file of the state directory" );
    builder.AddType( "AdminRequest", "A command of the admin client" );
    builder.AddField( "AdminRequest", 1, "token", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, GUCEF_RPC_FIELDFLAG_SENSITIVE );
    builder.AddField( "AdminRequest", 2, "command", "string" );
    builder.AddField( "AdminRequest", 3, "arguments", "map<string,string>" );
    builder.AddType( "AdminResponse", "The answer of the service" );
    builder.AddField( "AdminResponse", 1, "result", "datanode" );
    builder.AddField( "AdminResponse", 2, "message", "string" );
    builder.AddService( ServiceName, 1 );
    builder.AddMethod( ServiceName, 1, "Run", COMCORE::RPC_METHODKIND_UNARY, "AdminRequest", "AdminResponse", GUCEF_RPC_METHODFLAG_SENSITIVE );
    builder.SetMethodProtection( ServiceName, "Run", COMCORE::RPC_PROTECTION_CONFIDENTIAL, COMCORE::RPC_PROTECTION_CONFIDENTIAL );

    CORE::CString error;
    COMCORE::CRpcContractPtr built = builder.Build( error );
    if ( built.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "ServiceAdminContracts: " << ContractName << " 1.0 cannot be built: " << error );
        return built;
    }

    // known to every endpoint of the process, so that a client encodes with it
    COMCORE::CRpcContractRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetRpcContractRegistry();
    if ( registry.FindByHash( built->GetHash() ).IsNULL() )
        registry.Register( built, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_BUILTIN );

    contract = GUCEF_NEW COMCORE::CRpcContractPtr( built );
    return *contract;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
