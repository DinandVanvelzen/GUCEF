/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CDeviceContracts.h"

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
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CDeviceContracts::ContractName = "gucef.drnutils.devices";
const char* CDeviceContracts::PairingServiceName = "Pairing";
const char* CDeviceContracts::DevicesServiceName = "Devices";
const char* CDeviceContracts::BeginPath = "gucef.drnutils.devices.Pairing/Begin";
const char* CDeviceContracts::CompletePath = "gucef.drnutils.devices.Pairing/Complete";
const char* CDeviceContracts::ListPath = "gucef.drnutils.devices.Devices/List";
const char* CDeviceContracts::RevokePath = "gucef.drnutils.devices.Devices/Revoke";
const char* CDeviceContracts::SetRolesPath = "gucef.drnutils.devices.Devices/SetRoles";
const char* CDeviceContracts::RenamePath = "gucef.drnutils.devices.Devices/Rename";
const char* CDeviceContracts::OpenPairingWindowPath = "gucef.drnutils.devices.Devices/OpenPairingWindow";
const char* CDeviceContracts::ClosePairingWindowPath = "gucef.drnutils.devices.Devices/ClosePairingWindow";
const char* CDeviceContracts::ListPendingPairingsPath = "gucef.drnutils.devices.Devices/ListPendingPairings";
const char* CDeviceContracts::ConfirmPairingPath = "gucef.drnutils.devices.Devices/ConfirmPairing";
const char* CDeviceContracts::ListSessionsPath = "gucef.drnutils.devices.Devices/ListSessions";
const char* CDeviceContracts::CloseSessionPath = "gucef.drnutils.devices.Devices/CloseSession";

/*-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
CDeviceContracts::GetContract( void )
{GUCEF_TRACE;

    COMCORE::CRpcContractRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetRpcContractRegistry();
    COMCORE::CRpcContractPtr contract = registry.FindLatest( ContractName, Major );
    if ( !contract.IsNULL() )
        return contract;

    CORE::CDataNode source;
    GetContractSource( source );
    CORE::CString error;
    contract = COMCORE::CRpcContract::LoadFromDataNode( source, error );
    if ( contract.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CDeviceContracts: building the contract " << ContractName << " failed: " << error );
        return contract;
    }
    if ( !registry.Register( contract, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_BUILTIN ) )
    {
        // another thread registered it first: use that one
        COMCORE::CRpcContractPtr registered = registry.FindLatest( ContractName, Major );
        if ( !registered.IsNULL() )
            return registered;
    }
    return contract;
}

/*-------------------------------------------------------------------------*/

void
CDeviceContracts::GetContractSource( CORE::CDataNode& result )
{GUCEF_TRACE;

    COMCORE::CRpcContractBuilder builder( "gucef.drnutils.devices", 1, 0 );
    builder.SetDoc( "Pairing of devices with a remote service (on the pairing listener only, authenticated by the one-time code) and the administration of the paired devices and their client sessions (for admin devices). All times are UTC milliseconds" );
    builder.AddType( "Empty" );
    builder.AddType( "BeginRequest" );
    builder.AddField( "BeginRequest", 1, "clientKind", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "BeginResponse" );
    builder.AddField( "BeginResponse", 1, "serviceId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "BeginResponse", 2, "serverStaticKey", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "BeginResponse", 3, "pairingNonce", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "BeginResponse", 4, "serverProof", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "BeginResponse", 5, "windowExpiresUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "CompleteRequest" );
    builder.AddField( "CompleteRequest", 1, "deviceName", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteRequest", 2, "platform", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteRequest", 3, "appVersion", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteRequest", 4, "devicePublicKey", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteRequest", 5, "keyProof", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "CompleteResponse" );
    builder.AddField( "CompleteResponse", 1, "deviceId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteResponse", 2, "token", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, GUCEF_RPC_FIELDFLAG_SENSITIVE );
    builder.AddField( "CompleteResponse", 3, "tokenExpiresUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteResponse", 4, "roles", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddField( "CompleteResponse", 5, "deviceKind", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteResponse", 6, "sessionEndpoints", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddField( "CompleteResponse", 7, "serverStaticKey", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteResponse", 8, "issuerPublicKey", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteResponse", 9, "securityPolicy", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteResponse", 10, "serviceName", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteResponse", 11, "drnSessionName", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CompleteResponse", 12, "serviceId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "DeviceList" );
    builder.AddField( "DeviceList", 1, "devices", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "DeviceCommand" );
    builder.AddField( "DeviceCommand", 1, "commandId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "DeviceCommand", 2, "deviceId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "DeviceCommand", 3, "reason", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "DeviceCommand", 4, "roles", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddField( "DeviceCommand", 5, "name", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "DeviceCommand", 6, "fingerprint", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "DeviceCommand", 7, "approve", "bool", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "CommandOutcome" );
    builder.AddField( "CommandOutcome", 1, "applied", "bool", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CommandOutcome", 2, "replayed", "bool", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CommandOutcome", 3, "detail", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "OpenWindowRequest" );
    builder.AddField( "OpenWindowRequest", 1, "deviceKind", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "OpenWindowRequest", 2, "roles", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddField( "OpenWindowRequest", 3, "seconds", "uint32", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "OpenWindowRequest", 4, "count", "uint32", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "OpenWindowRequest", 5, "requireConfirmation", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "OpenWindowResponse" );
    builder.AddField( "OpenWindowResponse", 1, "windowId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "OpenWindowResponse", 2, "code", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, GUCEF_RPC_FIELDFLAG_SENSITIVE );
    builder.AddField( "OpenWindowResponse", 3, "expiresUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "OpenWindowResponse", 4, "pairingEndpoints", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddType( "CloseWindowRequest" );
    builder.AddField( "CloseWindowRequest", 1, "reason", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "SessionList" );
    builder.AddField( "SessionList", 1, "sessions", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "CloseSessionRequest" );
    builder.AddField( "CloseSessionRequest", 1, "commandId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CloseSessionRequest", 2, "sessionId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddService( "Pairing", 1 );
    builder.AddMethod( "Pairing", 1, "Begin", COMCORE::RPC_METHODKIND_UNARY, "BeginRequest", "BeginResponse", 0, 15000 );
    builder.AddMethod( "Pairing", 2, "Complete", COMCORE::RPC_METHODKIND_UNARY, "CompleteRequest", "CompleteResponse", GUCEF_RPC_METHODFLAG_MAYANSWERASYNC | GUCEF_RPC_METHODFLAG_SENSITIVE, 300000 );
    builder.AddService( "Devices", 2 );
    builder.AddMethod( "Devices", 1, "List", COMCORE::RPC_METHODKIND_UNARY, "Empty", "DeviceList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 15000 );
    builder.AddMethod( "Devices", 2, "Revoke", COMCORE::RPC_METHODKIND_UNARY, "DeviceCommand", "CommandOutcome", 0, 30000 );
    builder.AddMethod( "Devices", 3, "SetRoles", COMCORE::RPC_METHODKIND_UNARY, "DeviceCommand", "CommandOutcome", 0, 30000 );
    builder.AddMethod( "Devices", 4, "Rename", COMCORE::RPC_METHODKIND_UNARY, "DeviceCommand", "CommandOutcome", 0, 30000 );
    builder.AddMethod( "Devices", 5, "OpenPairingWindow", COMCORE::RPC_METHODKIND_UNARY, "OpenWindowRequest", "OpenWindowResponse", GUCEF_RPC_METHODFLAG_SENSITIVE, 30000 );
    builder.AddMethod( "Devices", 6, "ClosePairingWindow", COMCORE::RPC_METHODKIND_UNARY, "CloseWindowRequest", "Empty", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 15000 );
    builder.AddMethod( "Devices", 7, "ListPendingPairings", COMCORE::RPC_METHODKIND_UNARY, "Empty", "DeviceList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 15000 );
    builder.AddMethod( "Devices", 8, "ConfirmPairing", COMCORE::RPC_METHODKIND_UNARY, "DeviceCommand", "CommandOutcome", 0, 30000 );
    builder.AddMethod( "Devices", 9, "ListSessions", COMCORE::RPC_METHODKIND_UNARY, "Empty", "SessionList", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 15000 );
    builder.AddMethod( "Devices", 10, "CloseSession", COMCORE::RPC_METHODKIND_UNARY, "CloseSessionRequest", "CommandOutcome", 0, 30000 );

    CORE::CString error;
    COMCORE::CRpcContractPtr built = builder.Build( error );
    if ( built.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Contract source of the remote service kit is not valid: " << error );
        result = builder.GetDataNode();
        return;
    }
    result = builder.GetDataNode();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
