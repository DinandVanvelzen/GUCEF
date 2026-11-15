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

#include "gucefDRNUTILS_CRemoteSessionContracts.h"

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

const char* CRemoteSessionContracts::ContractName = "gucef.drnutils.session";
const char* CRemoteSessionContracts::HelloPath = "gucef.drnutils.session.Session/Hello";
const char* CRemoteSessionContracts::ResumePath = "gucef.drnutils.session.Session/Resume";
const char* CRemoteSessionContracts::AttachPath = "gucef.drnutils.session.Session/Attach";
const char* CRemoteSessionContracts::EventsPath = "gucef.drnutils.session.Session/Events";
const char* CRemoteSessionContracts::ResyncPath = "gucef.drnutils.session.Session/Resync";
const char* CRemoteSessionContracts::GoodbyePath = "gucef.drnutils.session.Session/Goodbye";
const char* CRemoteSessionContracts::RenewTokenPath = "gucef.drnutils.session.Session/RenewToken";
const char* CRemoteSessionContracts::ServiceName = "Session";
const char* CRemoteSessionContracts::EventKindNotice = "NOTICE";
const char* CRemoteSessionContracts::EventKindServiceStopping = "SERVICE_STOPPING";

/*-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
CRemoteSessionContracts::GetContract( void )
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
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteSessionContracts: building the contract " << ContractName << " failed: " << error );
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
CRemoteSessionContracts::GetContractSource( CORE::CDataNode& result )
{GUCEF_TRACE;

    COMCORE::CRpcContractBuilder builder( "gucef.drnutils.session", 1, 0 );
    builder.SetDoc( "The client session layer of a remote service: a session of a paired device that outlives RPC sessions within its grace, its event stream with sequence numbers and gap recovery, additional channels attached to it, and device token renewal. All times are UTC milliseconds" );
    builder.AddType( "Empty" );
    builder.AddType( "HelloRequest" );
    builder.AddField( "HelloRequest", 1, "clientKind", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloRequest", 2, "clientVersion", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloRequest", 3, "capabilities", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddField( "HelloRequest", 4, "clientTimeUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "HelloResponse" );
    builder.AddField( "HelloResponse", 1, "sessionId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 2, "resumeSecret", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, GUCEF_RPC_FIELDFLAG_SENSITIVE );
    builder.AddField( "HelloResponse", 3, "deviceId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 4, "deviceKind", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 5, "roles", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddField( "HelloResponse", 6, "serviceName", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 7, "serviceVersion", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 8, "hostInstanceId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 9, "serverTimeUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 10, "resumeGraceSeconds", "uint32", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 11, "eventSeq", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 12, "limits", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 13, "capabilities", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddField( "HelloResponse", 14, "sessionEndpoints", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddField( "HelloResponse", 15, "tokenExpiresUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 16, "notices", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "HelloResponse", 17, "serviceInfo", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "ResumeRequest" );
    builder.AddField( "ResumeRequest", 1, "sessionId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ResumeRequest", 2, "resumeSecret", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, GUCEF_RPC_FIELDFLAG_SENSITIVE );
    builder.AddField( "ResumeRequest", 3, "lastEventSeq", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "ResumeResponse" );
    builder.AddField( "ResumeResponse", 1, "eventSeq", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ResumeResponse", 2, "eventsRetainedFrom", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ResumeResponse", 3, "hostInstanceId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ResumeResponse", 4, "serverTimeUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ResumeResponse", 5, "extensions", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "AttachRequest" );
    builder.AddField( "AttachRequest", 1, "sessionId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "AttachRequest", 2, "resumeSecret", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, GUCEF_RPC_FIELDFLAG_SENSITIVE );
    builder.AddField( "AttachRequest", 3, "channel", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "AttachResponse" );
    builder.AddField( "AttachResponse", 1, "channelIndex", "uint32", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "AttachResponse", 2, "limits", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "EventsRequest" );
    builder.AddField( "EventsRequest", 1, "fromEventSeq", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "Event" );
    builder.AddField( "Event", 1, "eventSeq", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "Event", 2, "kind", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "Event", 3, "atUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "Event", 4, "ids", "string", COMCORE::RPC_CARDINALITY_REPEATED, 0, 0 );
    builder.AddField( "Event", 5, "revision", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "Event", 6, "data", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "ResyncResponse" );
    builder.AddField( "ResyncResponse", 1, "eventSeq", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ResyncResponse", 2, "hostInstanceId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ResyncResponse", 3, "serverTimeUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ResyncResponse", 4, "notices", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ResyncResponse", 5, "extensions", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "GoodbyeRequest" );
    builder.AddField( "GoodbyeRequest", 1, "reason", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "RenewTokenResponse" );
    builder.AddField( "RenewTokenResponse", 1, "token", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, GUCEF_RPC_FIELDFLAG_SENSITIVE );
    builder.AddField( "RenewTokenResponse", 2, "expiresUtcMs", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddService( "Session", 1 );
    builder.AddMethod( "Session", 1, "Hello", COMCORE::RPC_METHODKIND_UNARY, "HelloRequest", "HelloResponse", 0, 15000 );
    builder.AddMethod( "Session", 2, "Resume", COMCORE::RPC_METHODKIND_UNARY, "ResumeRequest", "ResumeResponse", GUCEF_RPC_METHODFLAG_SENSITIVE, 15000 );
    builder.AddMethod( "Session", 3, "Attach", COMCORE::RPC_METHODKIND_UNARY, "AttachRequest", "AttachResponse", GUCEF_RPC_METHODFLAG_SENSITIVE, 15000 );
    builder.AddMethod( "Session", 4, "Events", COMCORE::RPC_METHODKIND_SERVER_STREAM, "EventsRequest", "Event", GUCEF_RPC_METHODFLAG_RESUMABLESTREAM, 0 );
    builder.SetMethodInitialCredit( "Session", "Events", 256 );
    builder.AddMethod( "Session", 5, "Resync", COMCORE::RPC_METHODKIND_UNARY, "Empty", "ResyncResponse", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 15000 );
    builder.AddMethod( "Session", 6, "Goodbye", COMCORE::RPC_METHODKIND_UNARY, "GoodbyeRequest", "Empty", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 10000 );
    builder.AddMethod( "Session", 7, "RenewToken", COMCORE::RPC_METHODKIND_UNARY, "Empty", "RenewTokenResponse", GUCEF_RPC_METHODFLAG_SENSITIVE, 15000 );

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
