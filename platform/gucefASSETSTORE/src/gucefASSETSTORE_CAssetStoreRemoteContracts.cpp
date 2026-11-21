/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */

#include "gucefASSETSTORE_CAssetStoreRemoteContracts.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CAssetStoreRemoteContracts::ControlContractName = "gucef.assetstore";
const char* CAssetStoreRemoteContracts::FilesContractName = "gucef.assetstore.files";
const UInt32 CAssetStoreRemoteContracts::Major;
const UInt32 CAssetStoreRemoteContracts::BlockSize;
const char* CAssetStoreRemoteContracts::QueryPath = "gucef.assetstore.Service/Query";
const char* CAssetStoreRemoteContracts::CommandPath = "gucef.assetstore.Service/Command";
const char* CAssetStoreRemoteContracts::SecretPath = "gucef.assetstore.Service/Secret";
const char* CAssetStoreRemoteContracts::FilesQueryPath = "gucef.assetstore.files.Files/Query";
const char* CAssetStoreRemoteContracts::FilesCommandPath = "gucef.assetstore.files.Files/Command";
const char* CAssetStoreRemoteContracts::ReadBlockPath = "gucef.assetstore.files.Files/ReadBlock";
const char* CAssetStoreRemoteContracts::WriteBlockPath = "gucef.assetstore.files.Files/WriteBlock";
const char* CAssetStoreRemoteContracts::EventCatalogueChanged = "CATALOGUE_CHANGED";
const char* CAssetStoreRemoteContracts::EventJobChanged = "JOB_CHANGED";
const char* CAssetStoreRemoteContracts::EventJobRemoved = "JOB_REMOVED";
const char* CAssetStoreRemoteContracts::EventLocationState = "LOCATION_STATE";
const char* CAssetStoreRemoteContracts::EventStoreState = "STORE_STATE";
const char* CAssetStoreRemoteContracts::EventStoresChanged = "STORES_CHANGED";
const char* CAssetStoreRemoteContracts::EventSignInPrompt = "SIGNIN_PROMPT";
const char* CAssetStoreRemoteContracts::EventSignInPromptClosed = "SIGNIN_PROMPT_CLOSED";
const char* CAssetStoreRemoteContracts::EventThumbnailUpdated = "THUMBNAIL_UPDATED";
const char* CAssetStoreRemoteContracts::EventServiceNotice = "ASSETSTORE_NOTICE";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Builds the source and registers the contract once; another thread that was first wins */
COMCORE::CRpcContractPtr
RegisterContract( const char* name, const CORE::CDataNode& source )
{GUCEF_TRACE;

    COMCORE::CRpcContractRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetRpcContractRegistry();
    COMCORE::CRpcContractPtr contract = registry.FindLatest( name, CAssetStoreRemoteContracts::Major );
    if ( !contract.IsNULL() )
        return contract;
    CORE::CString error;
    contract = COMCORE::CRpcContract::LoadFromDataNode( source, error );
    if ( contract.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetStoreRemoteContracts: building the contract " << name << " failed: " << error );
        return contract;
    }
    if ( !registry.Register( contract, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_BUILTIN ) )
    {
        COMCORE::CRpcContractPtr registered = registry.FindLatest( name, CAssetStoreRemoteContracts::Major );
        if ( !registered.IsNULL() )
            return registered;
    }
    return contract;
}

/** The messages of the generic calls, the same in both contracts */
void
AddCallTypes( COMCORE::CRpcContractBuilder& builder )
{GUCEF_TRACE;

    builder.AddType( "CallRequest", "One call of the asset store service: the operation of CAssetServiceOperations, the command id of a mutation and the arguments by parameter name" );
    builder.AddField( "CallRequest", 1, "operation", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CallRequest", 2, "commandId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CallRequest", 3, "args", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "CallAnswer", "The answer of a call: the status name, a message for people, the result record and the event sequence after the call" );
    builder.AddField( "CallAnswer", 1, "status", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CallAnswer", 2, "message", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CallAnswer", 3, "result", "datanode", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CallAnswer", 4, "eventSeq", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "CallAnswer", 5, "replayed", "bool", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAssetStoreRemoteContracts::GetControlContractSource( CORE::CDataNode& source )
{GUCEF_TRACE;

    COMCORE::CRpcContractBuilder builder( ControlContractName, Major, 0 );
    builder.SetDoc( "The asset store as a remote service: every operation of the service interface as a generic call, its result as the DataNode form of the record" );
    AddCallTypes( builder );
    builder.AddType( "SecretRequest", "A call that carries a secret: the answer of a sign in prompt, the code of the keyring" );
    builder.AddField( "SecretRequest", 1, "operation", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "SecretRequest", 2, "commandId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "SecretRequest", 3, "target", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "SecretRequest", 4, "secret", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, GUCEF_RPC_FIELDFLAG_SENSITIVE );
    builder.AddService( "Service", 1 );
    builder.AddMethod( "Service", 1, "Query", COMCORE::RPC_METHODKIND_UNARY, "CallRequest", "CallAnswer", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 60000 );
    builder.AddMethod( "Service", 2, "Command", COMCORE::RPC_METHODKIND_UNARY, "CallRequest", "CallAnswer", 0, 600000 );
    builder.AddMethod( "Service", 3, "Secret", COMCORE::RPC_METHODKIND_UNARY, "SecretRequest", "CallAnswer", GUCEF_RPC_METHODFLAG_SENSITIVE, 60000 );
    source = builder.GetDataNode();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreRemoteContracts::GetFilesContractSource( CORE::CDataNode& source )
{GUCEF_TRACE;

    COMCORE::CRpcContractBuilder builder( FilesContractName, Major, 0 );
    builder.SetDoc( "The files of the asset store service: stats, listings and uploads as generic calls, blocks of files and uploads as bytes" );
    AddCallTypes( builder );
    builder.AddType( "ReadBlockRequest", "At most one block (262144 bytes) of a file of the service from an offset" );
    builder.AddField( "ReadBlockRequest", 1, "path", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ReadBlockRequest", 2, "offset", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "ReadBlockRequest", 3, "length", "uint32", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "Block", "The bytes read, with the size of the file" );
    builder.AddField( "Block", 1, "status", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "Block", 2, "message", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "Block", 3, "fileSize", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "Block", 4, "endOfFile", "bool", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "Block", 5, "data", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "Block", 6, "offset", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddType( "WriteBlockRequest", "Bytes of a file of an upload at an offset" );
    builder.AddField( "WriteBlockRequest", 1, "uploadId", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "WriteBlockRequest", 2, "path", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "WriteBlockRequest", 3, "offset", "uint64", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddField( "WriteBlockRequest", 4, "data", "bytes", COMCORE::RPC_CARDINALITY_SINGLE, 0, 0 );
    builder.AddService( "Files", 1 );
    builder.AddMethod( "Files", 1, "Query", COMCORE::RPC_METHODKIND_UNARY, "CallRequest", "CallAnswer", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 60000 );
    builder.AddMethod( "Files", 2, "Command", COMCORE::RPC_METHODKIND_UNARY, "CallRequest", "CallAnswer", 0, 600000 );
    builder.AddMethod( "Files", 3, "ReadBlock", COMCORE::RPC_METHODKIND_UNARY, "ReadBlockRequest", "Block", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 60000 );
    builder.AddMethod( "Files", 4, "WriteBlock", COMCORE::RPC_METHODKIND_UNARY, "WriteBlockRequest", "CallAnswer", 0, 60000 );
    source = builder.GetDataNode();
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
CAssetStoreRemoteContracts::GetControlContract( void )
{GUCEF_TRACE;

    CORE::CDataNode source;
    GetControlContractSource( source );
    return RegisterContract( ControlContractName, source );
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
CAssetStoreRemoteContracts::GetFilesContract( void )
{GUCEF_TRACE;

    CORE::CDataNode source;
    GetFilesContractSource( source );
    return RegisterContract( FilesContractName, source );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreRemoteContracts::IsFilesOperation( const CORE::CString& operation )
{GUCEF_TRACE;

    return CAssetServiceOperations::StatFiles == operation ||
           CAssetServiceOperations::ListDirectory == operation ||
           CAssetServiceOperations::ReadFileBlock == operation ||
           CAssetServiceOperations::BeginUpload == operation ||
           CAssetServiceOperations::BeginUploadFiles == operation ||
           CAssetServiceOperations::WriteUploadBlock == operation ||
           CAssetServiceOperations::QueryUpload == operation ||
           CAssetServiceOperations::CommitUpload == operation ||
           CAssetServiceOperations::AbortUpload == operation;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
