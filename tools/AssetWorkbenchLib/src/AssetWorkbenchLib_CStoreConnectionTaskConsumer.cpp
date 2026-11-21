/*
 *  AssetWorkbenchLib: the pool task that pairs with, or opens, a remote asset store service
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

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H
#include "gucefDRNUTILS_CDevicePairingClient.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H
#include "gucefASSETSTORE_CRemoteAssetStoreService.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#include "gucefASSETSTORE_CAssetStoreRemoteContracts.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H ? */

#include "AssetWorkbenchLib_CStoreConnectionTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

const CORE::CString CStoreConnectionTaskConsumer::TaskType = "AssetWorkbenchStoreConnection";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStoreConnectionTaskConsumer::CStoreConnectionTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStoreConnectionTaskConsumer::~CStoreConnectionTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CStoreConnectionTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStoreConnectionTaskConsumer::DescribePairingFailure( Int32 failure, const CORE::CString& detail )
{GUCEF_TRACE;

    switch ( failure )
    {
        case DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_BAD_REQUEST:
            return "The pairing code or the address is not usable. Type the code as the server shows it and check the address.";
        case DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_UNREACHABLE:
            return "No pairing is open at that address. Open pairing on the server first, and check the address and that the service runs.";
        case DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_WRONG_CODE:
            return "The pairing code was not accepted. Check the code; after three wrong codes this computer is refused for 15 minutes, so open a new pairing on the server.";
        case DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_SERVER_PROOF:
            return "The service did not prove that it knows the pairing code, so it is not trusted and nothing was kept. Check that the address is the service you meant.";
        case DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_REFUSED:
            return "The service refused the pairing (the pairing window is closed or used up). Open a new pairing on the server.";
        case DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_DECLINED:
            return "The owner of the service declined the pairing.";
        case DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_TIMEOUT:
            return "The service did not answer in time. Check the address and the network and try again.";
        case DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_STORE:
            return "The pairing worked, but this computer could not keep the credentials (the credential store of the workbench is not usable). " + detail;
        default:
            return detail.IsNULLOrEmpty() ? CORE::CString( "The pairing did not work." ) : CORE::CString( "The pairing did not work: " ) + detail;
    }
}

/*-------------------------------------------------------------------------*/

bool
CStoreConnectionTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CStoreConnectionTaskData* data = static_cast< CStoreConnectionTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return true;
    try
    {
        if ( CStoreConnectionTaskData::KIND_PAIR == data->kind )
            Pair( *data );
        else
            Connect( *data );
    }
    catch ( const std::exception& e )
    {
        data->succeeded = false;
        data->problem = CORE::CString( "The connection step failed: " ) + e.what();
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "StoreConnectionTaskConsumer: " << data->problem );
    }
    catch ( ... )
    {
        data->succeeded = false;
        data->problem = "The connection step failed";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StoreConnectionTaskConsumer: " << data->problem );
    }
    data->code.Clear();
    data->done = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CStoreConnectionTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStoreConnectionTaskConsumer::Pair( CStoreConnectionTaskData& data )
{GUCEF_TRACE;

    // the contracts of the service are known to the process before the pairing link binds them
    ASSETSTORE::CAssetStoreRemoteContracts::GetControlContract();
    ASSETSTORE::CAssetStoreRemoteContracts::GetFilesContract();

    // a pulse generator of its own: the pairing never drives the one of the interface
    DRNUTILS::CDevicePairingClient client;
    DRNUTILS::CDevicePairingClient::SRequest request;
    request.pairingEndpoint = data.pairingEndpoint;
    request.code = data.code.Reveal();
    request.serviceName = data.serviceName;
    request.drnSessionName = data.sessionName;
    request.clientKind = data.clientKind;
    request.deviceName = data.deviceName;
    request.platform = "gucef";
    request.appVersion = "0.1.0";
    request.connectTimeoutMs = data.timeoutInMs;
    request.lanDiscovery = data.lanDiscovery;
    if ( !data.serviceType.IsNULLOrEmpty() )
        request.lanDiscoveryFinder.serviceType = data.serviceType;
    DRNUTILS::CDeviceCredentials credentials;
    CORE::CString error;
    data.succeeded = client.Pair( request, credentials, error, data.store, data.credentialsSlot );
    request.code.Clear();
    credentials.Wipe();
    data.fingerprint = client.GetFingerprint();
    data.failure = (Int32) client.GetLastFailure();
    if ( data.succeeded )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: this computer is paired with the service (key " << data.fingerprint << ")" );
        return;
    }
    data.problem = DescribePairingFailure( data.failure, error );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: the pairing did not work (" << data.failure << "): " << error );
}

/*-------------------------------------------------------------------------*/

void
CStoreConnectionTaskConsumer::Connect( CStoreConnectionTaskData& data )
{GUCEF_TRACE;

    ASSETSTORE::CIAssetStoreService* service = data.service.GetPointerAlways();
    if ( GUCEF_NULL == service )
    {
        data.problem = "There is no service to open";
        return;
    }
    // Open() waits for the connection: the thread that drives the pulse generator of the service goes on meanwhile
    const ASSETSTORE::TAssetStoreStatus status = service->Open();
    const CORE::CString message = service->GetStatusMessage();
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status && !service->IsCatalogueAvailable() )
    {
        data.succeeded = false;
        data.problem = message.IsNULLOrEmpty() ? CORE::CString( "The store service did not open" ) : message;
        data.transient = message.HasSubstr( "cannot be reached" ) >= 0;
        return;
    }
    service->GetServiceInfo( data.info );
    ASSETSTORE::CRemoteAssetStoreService* remote = static_cast< ASSETSTORE::CRemoteAssetStoreService* >( service );
    DRNUTILS::CRemoteServiceConnection* connection = remote->GetConnection();
    if ( GUCEF_NULL != connection )
    {
        const COMCORE::CRpcMessage hello = connection->GetHelloResponse();
        data.serverName = hello.GetString( 6 );
        data.serverVersion = hello.GetString( 7 );
    }
    data.succeeded = true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
