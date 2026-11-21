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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#include "gucefAUTH_CFileCredentialStore.h"
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H
#include "gucefDRNUTILS_CDevicePairingClient.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#include "gucefDRNUTILS_CRemoteSessionContracts.h"
#define GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#include "gucefDRNUTILS_CRemoteServiceHost.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEHOST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#include "gucefASSETSTORE_CAssetStoreRemoteContracts.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#include "gucefASSETSTORE_CAssetServiceNotice.h"
#define GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H
#include "gucefASSETSTORE_CAssetStoreRemoteCodec.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H ? */

#include "gucefASSETSTORE_CRemoteAssetStoreService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A message of a type of a contract, bound without an endpoint (the call may wait for a reconnect) */
COMCORE::CRpcMessage
NewMessage( const COMCORE::CRpcContractPtr& contract, const char* typeName )
{GUCEF_TRACE;

    if ( contract.IsNULL() )
        return COMCORE::CRpcMessage();
    return COMCORE::CRpcMessage( contract, contract->FindType( typeName ) );
}

/** Reads a record of an event's data */
template < class T >
bool
ReadEventData( const COMCORE::CRpcMessage& event, T& record )
{
    CORE::CDataNode node;
    if ( !event.GetDataNode( 6, node ) )
        return false;
    CORE::CDataNodeSerializableSettings settings;
    return record.Deserialize( node, settings );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteAssetStoreService::SPendingCall::SPendingCall( void )
    : kind( KIND_CALL )
    , operation()
    , rpcFutures()
    , source()
    , decoded( false )
    , status( ASSETSTORE_STATUS_FAILED )
    , message()
    , result( GUCEF_NULL )
    , eventSeq( 0 )
    , decodedAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetStoreService::CRemoteAssetStoreService( const CAssetStoreServiceConfig& config )
    : CIAssetStoreService( config.pulseGenerator )
    , m_config( config )
    , m_settings()
    , m_settingsValid( true )
    , m_store( GUCEF_NULL )
    , m_ownStore( GUCEF_NULL )
    , m_connection( GUCEF_NULL )
    , m_timer( GetPulseGenerator(), 10 )
    , m_pending()
    , m_providers()
    , m_stores()
    , m_statusMessage()
    , m_open( false )
    , m_catalogueAvailable( false )
    , m_lock()
{GUCEF_TRACE;

    if ( config.implementationSettings.GetAttCount() > 0 || config.implementationSettings.GetNrOfChildNodes() > 0 )
        m_settingsValid = m_settings.LoadConfig( config.implementationSettings );
    if ( !m_settingsValid )
        m_statusMessage = "the settings of the remote asset store service are refused (see the log)";
}

/*-------------------------------------------------------------------------*/

CRemoteAssetStoreService::~CRemoteAssetStoreService()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

const CRemoteAssetStoreServiceSettings&
CRemoteAssetStoreService::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CRemoteServiceConnection*
CRemoteAssetStoreService::GetConnection( void ) const
{GUCEF_TRACE;

    return m_connection;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteAssetStoreService::GetPendingCallCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_pending.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetStoreService::GetType( void ) const
{GUCEF_TRACE;

    return CAssetStoreServiceConfig::RemoteImplementationId;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreService::IsInProcess( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteAssetStoreService::NowMs( void )
{GUCEF_TRACE;

    return DRNUTILS::CRemoteServiceHost::NowMs();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreService::PairDevice( const CORE::CString& pairingEndpoint, const AUTH::CSecretString& code, const CORE::CString& serviceName,
                                      const CORE::CString& drnSessionName, const CORE::CString& deviceName, const CORE::CString& clientKind,
                                      AUTH::ICredentialStore& store, const CORE::CString& slot, CORE::CString& error )
{GUCEF_TRACE;

    CAssetStoreRemoteContracts::GetControlContract();
    CAssetStoreRemoteContracts::GetFilesContract();
    // the pairing client pumps the global pulse generator while it waits when this thread drives it (a host in this process answers on it)
    DRNUTILS::CDevicePairingClient client( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
    DRNUTILS::CDevicePairingClient::SRequest request;
    request.pairingEndpoint = pairingEndpoint;
    request.code = code.Reveal();
    request.serviceName = serviceName;
    request.drnSessionName = drnSessionName;
    request.clientKind = clientKind;
    request.deviceName = deviceName;
    request.platform = "gucef";
    request.appVersion = "0.1.0";
    DRNUTILS::CDeviceCredentials credentials;
    const bool paired = client.Pair( request, credentials, error );
    request.code.Clear();
    if ( !paired )
        return false;
    const bool saved = credentials.SaveToStore( store, slot, error );
    credentials.Wipe();
    if ( saved )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteAssetStoreService: this device is paired with " << serviceName << " (credentials kept as \"" << slot << "\")" );
    return saved;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::Open( void )
{GUCEF_TRACE;

    if ( m_open )
        return IsCatalogueAvailable() ? ASSETSTORE_STATUS_OK : ASSETSTORE_STATUS_UNAVAILABLE;
    if ( !m_settingsValid )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    if ( CAssetStoreRemoteContracts::GetControlContract().IsNULL() || CAssetStoreRemoteContracts::GetFilesContract().IsNULL() )
    {
        m_statusMessage = "the contracts of the remote asset store service cannot be built";
        return ASSETSTORE_STATUS_FAILED;
    }

    // the credentials of this device: the store of the configuration or a file store of its own
    m_store = m_config.credentialStore;
    if ( !m_settings.credentialsDirectory.IsNULLOrEmpty() )
    {
        AUTH::CFileCredentialStore* fileStore = GUCEF_NEW AUTH::CFileCredentialStore( m_settings.credentialsDirectory );
        if ( !m_settings.credentialsKeyFile.IsNULLOrEmpty() )
            fileStore->SetKeyFile( m_settings.credentialsKeyFile );
        m_ownStore = fileStore;
        m_store = fileStore;
    }
    if ( GUCEF_NULL == m_store )
    {
        m_statusMessage = "no credential store holds the credentials of this device (credentialsDirectory)";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    DRNUTILS::CDeviceCredentials credentials;
    CORE::CString error;
    if ( !credentials.LoadFromStore( *m_store, m_settings.credentialsSlot, error ) || !credentials.IsComplete() )
    {
        m_statusMessage = "this device is not paired with an asset store service (no credentials \"" + m_settings.credentialsSlot + "\"): pair it first";
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteAssetStoreService: " << m_statusMessage );
        credentials.Wipe();
        return ASSETSTORE_STATUS_UNAVAILABLE;
    }

    DRNUTILS::CRemoteServiceConnectionSettings connectionSettings;
    m_settings.GetConnectionSettings( connectionSettings );
    m_connection = GUCEF_NEW DRNUTILS::CRemoteServiceConnection( GetPulseGenerator(), connectionSettings );
    m_connection->SetCredentialStore( m_store, m_settings.credentialsSlot );
    TEventCallback onEvent( this, &CRemoteAssetStoreService::OnSessionEvent );
    SubscribeTo( m_connection, DRNUTILS::CRemoteServiceConnection::SessionEventEvent, onEvent );
    TEventCallback onState( this, &CRemoteAssetStoreService::OnStateChanged );
    SubscribeTo( m_connection, DRNUTILS::CRemoteServiceConnection::StateChangedEvent, onState );
    TEventCallback onStopping( this, &CRemoteAssetStoreService::OnServiceStopping );
    SubscribeTo( m_connection, DRNUTILS::CRemoteServiceConnection::ServiceStoppingEvent, onStopping );
    TEventCallback onResync( this, &CRemoteAssetStoreService::OnResync );
    SubscribeTo( m_connection, DRNUTILS::CRemoteServiceConnection::ResyncEvent, onResync );
    SubscribeTo( m_connection, DRNUTILS::CRemoteServiceConnection::SessionReplacedEvent, onResync );
    TEventCallback onTimer( this, &CRemoteAssetStoreService::OnTimer );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, onTimer );
    m_timer.SetEnabled( true );
    m_open = true;

    const bool connected = m_connection->Connect( credentials, error );
    credentials.Wipe();
    if ( !connected )
    {
        m_statusMessage = "the asset store service cannot be reached: " + error;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteAssetStoreService: " << m_statusMessage );
        return ASSETSTORE_STATUS_UNAVAILABLE;
    }
    m_statusMessage.Clear();
    CAssetServiceInfo info;
    const TAssetStoreStatus status = GetServiceInfo( info );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_statusMessage = "the asset store service did not describe itself: " + CORE::CString( AssetStoreStatusToString( status ) );
        return status;
    }
    if ( !m_catalogueAvailable )
        m_statusMessage = "the catalogue of the asset store service is not available";
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteAssetStoreService: connected as device " << m_connection->GetDeviceId() << ", " << info.stores.size()
               << " online stores, " << info.locations.size() << " storage locations" );
    return m_catalogueAvailable ? ASSETSTORE_STATUS_OK : ASSETSTORE_STATUS_UNAVAILABLE;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::Close( void )
{GUCEF_TRACE;

    if ( !m_open && GUCEF_NULL == m_connection && GUCEF_NULL == m_ownStore )
        return;
    m_open = false;
    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();

    // the calls that wait end now
    TPendingVector pending;
    {
        MT::CScopeMutex lock( m_lock );
        pending.swap( m_pending );
    }
    for ( size_t i=0; i<pending.size(); ++i )
    {
        for ( size_t f=0; f<pending[ i ]->rpcFutures.size(); ++f )
            pending[ i ]->rpcFutures[ f ].RequestCancellation();
        if ( !pending[ i ]->decoded )
        {
            GUCEF_DELETE pending[ i ]->result;
            pending[ i ]->result = CAssetStoreRemoteCodec::NewResult( pending[ i ]->operation );
            pending[ i ]->status = ASSETSTORE_STATUS_UNAVAILABLE;
            pending[ i ]->message = "the remote asset store service was closed";
            SetFailureOf( pending[ i ]->result, pending[ i ]->status, pending[ i ]->message );
        }
        Complete( *pending[ i ] );
        GUCEF_DELETE pending[ i ];
    }
    if ( GUCEF_NULL != m_connection )
    {
        m_connection->Close( true, 2000 );
        GUCEF_DELETE m_connection;
        m_connection = GUCEF_NULL;
    }
    DetachProviders();
    m_store = GUCEF_NULL;
    GUCEF_DELETE m_ownStore;
    m_ownStore = GUCEF_NULL;
    m_catalogueAvailable = false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreService::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreService::IsCatalogueAvailable( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_open && m_catalogueAvailable && GUCEF_NULL != m_connection && DRNUTILS::REMOTECONNECTION_FAILED != m_connection->GetState();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetStoreService::GetStatusMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_open )
        return m_statusMessage.IsNULLOrEmpty() ? CORE::CString( "The remote asset store service is not open" ) : m_statusMessage;
    return m_statusMessage;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreService::IsPumpingThread( void ) const
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulse = GetPulseGenerator();
    if ( pulse.IsNULL() )
        return false;
    const TThreadId driver = pulse->GetPulseDriverThreadId();
    return 0 == driver || driver == MT::GetCurrentThreadID();
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteAssetStoreService::GetFilesChannel( void ) const
{GUCEF_TRACE;

    // the bulk link when it is attached, the control link (which binds the files contract too) until then
    if ( GUCEF_NULL != m_connection && m_settings.bulkChannels > 0 && m_connection->IsBulkChannelReady( 1 ) )
        return 1;
    return 0;
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcCallOptions
CRemoteAssetStoreService::MakeOptions( bool idempotent ) const
{GUCEF_TRACE;

    COMCORE::CRpcCallOptions options;
    options.waitForReady = true;
    options.retryIdempotentOnNewSession = idempotent;
    return options;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteAssetStoreService::Refuse( const CORE::CString& operation, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    SPendingCall pending;
    pending.operation = operation;
    pending.decoded = true;
    pending.status = status;
    pending.message = message;
    pending.result = CAssetStoreRemoteCodec::NewResult( operation );
    SetFailureOf( pending.result, status, message );
    CORE::CFutureResult future = pending.source.GetFuture();
    Complete( pending );
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteAssetStoreService::Track( SPendingCall* pending )
{GUCEF_TRACE;

    CORE::CFutureResult future = pending->source.GetFuture();
    MT::CScopeMutex lock( m_lock );
    m_pending.push_back( pending );
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteAssetStoreService::ExecuteAsync( const CAssetServiceCall& call )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == m_connection )
        return Refuse( call.operation, ASSETSTORE_STATUS_UNAVAILABLE, "the remote asset store service is not open" );

    const bool files = CAssetStoreRemoteContracts::IsFilesOperation( call.operation );
    const bool mutation = CAssetServiceOperations::IsMutation( call.operation );
    const char* path = files ? ( mutation ? CAssetStoreRemoteContracts::FilesCommandPath : CAssetStoreRemoteContracts::FilesQueryPath )
                             : ( mutation ? CAssetStoreRemoteContracts::CommandPath : CAssetStoreRemoteContracts::QueryPath );
    COMCORE::CRpcMessage request = NewMessage( files ? CAssetStoreRemoteContracts::GetFilesContract() : CAssetStoreRemoteContracts::GetControlContract(), "CallRequest" );
    if ( !request.IsBound() )
        return Refuse( call.operation, ASSETSTORE_STATUS_FAILED, "the contracts of the remote asset store service cannot be built" );
    request.SetString( CAssetStoreRemoteContracts::CallOperationField, call.operation );
    request.SetString( CAssetStoreRemoteContracts::CallCommandIdField, call.commandId );
    request.SetDataNode( CAssetStoreRemoteContracts::CallArgsField, call.args );

    SPendingCall* pending = GUCEF_NEW SPendingCall();
    pending->kind = SPendingCall::KIND_CALL;
    pending->operation = call.operation;
    pending->rpcFutures.push_back( m_connection->CallAsync( path, request, MakeOptions( !mutation ), files ? GetFilesChannel() : 0 ) );
    return Track( pending );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteAssetStoreService::WriteUploadBlockAsync( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == m_connection )
        return Refuse( CAssetServiceOperations::WriteUploadBlock, ASSETSTORE_STATUS_UNAVAILABLE, "the remote asset store service is not open" );
    COMCORE::CRpcContractPtr contract = CAssetStoreRemoteContracts::GetFilesContract();
    SPendingCall* pending = GUCEF_NEW SPendingCall();
    pending->kind = SPendingCall::KIND_WRITE;
    pending->operation = CAssetServiceOperations::WriteUploadBlock;
    const UInt32 channel = GetFilesChannel();
    const COMCORE::CRpcCallOptions options = MakeOptions( false );
    const UInt32 size = data.GetDataSize();
    UInt32 done = 0;
    do
    {
        const UInt32 blockLength = size - done > CAssetStoreRemoteContracts::BlockSize ? CAssetStoreRemoteContracts::BlockSize : size - done;
        COMCORE::CRpcMessage request = NewMessage( contract, "WriteBlockRequest" );
        request.SetString( CAssetStoreRemoteContracts::WriteUploadIdField, uploadId );
        request.SetString( CAssetStoreRemoteContracts::WritePathField, relativePath );
        request.SetUInt64( CAssetStoreRemoteContracts::WriteOffsetField, offset + done );
        request.SetBytes( CAssetStoreRemoteContracts::WriteDataField, static_cast< const UInt8* >( data.GetConstBufferPtr() ) + done, blockLength );
        pending->rpcFutures.push_back( m_connection->CallAsync( CAssetStoreRemoteContracts::WriteBlockPath, request, options, channel ) );
        done += blockLength;
    }
    while ( done < size );
    return Track( pending );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteAssetStoreService::AnswerPromptAsync( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == m_connection )
        return Refuse( CAssetServiceOperations::AnswerPrompt, ASSETSTORE_STATUS_UNAVAILABLE, "the remote asset store service is not open" );
    COMCORE::CRpcMessage request = NewMessage( CAssetStoreRemoteContracts::GetControlContract(), "SecretRequest" );
    request.SetString( CAssetStoreRemoteContracts::SecretOperationField, CAssetServiceOperations::AnswerPrompt );
    request.SetString( CAssetStoreRemoteContracts::SecretCommandIdField, commandId );
    request.SetString( CAssetStoreRemoteContracts::SecretTargetField, promptId );
    request.SetString( CAssetStoreRemoteContracts::SecretValueField, answer.Reveal() );
    SPendingCall* pending = GUCEF_NEW SPendingCall();
    pending->operation = CAssetServiceOperations::AnswerPrompt;
    pending->rpcFutures.push_back( m_connection->CallAsync( CAssetStoreRemoteContracts::SecretPath, request, MakeOptions( false ), 0 ) );
    request.Clear( CAssetStoreRemoteContracts::SecretValueField );
    return Track( pending );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteAssetStoreService::ImportWebSessionAsync( const CORE::CString& commandId, const CORE::CString& storeId, const AUTH::CSecretString& session )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == m_connection )
        return Refuse( CAssetServiceOperations::ImportWebSession, ASSETSTORE_STATUS_UNAVAILABLE, "the remote asset store service is not open" );
    // the session travels in the sensitive field of the secret method only (encrypted link, never logged)
    COMCORE::CRpcMessage request = NewMessage( CAssetStoreRemoteContracts::GetControlContract(), "SecretRequest" );
    request.SetString( CAssetStoreRemoteContracts::SecretOperationField, CAssetServiceOperations::ImportWebSession );
    request.SetString( CAssetStoreRemoteContracts::SecretCommandIdField, commandId );
    request.SetString( CAssetStoreRemoteContracts::SecretTargetField, storeId );
    request.SetString( CAssetStoreRemoteContracts::SecretValueField, session.Reveal() );
    SPendingCall* pending = GUCEF_NEW SPendingCall();
    pending->operation = CAssetServiceOperations::ImportWebSession;
    pending->rpcFutures.push_back( m_connection->CallAsync( CAssetStoreRemoteContracts::SecretPath, request, MakeOptions( false ), 0 ) );
    request.Clear( CAssetStoreRemoteContracts::SecretValueField );
    return Track( pending );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteAssetStoreService::UnlockKeyringAsync( const CORE::CString& commandId, const AUTH::CSecretString& code )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == m_connection )
        return Refuse( CAssetServiceOperations::UnlockKeyring, ASSETSTORE_STATUS_UNAVAILABLE, "the remote asset store service is not open" );
    COMCORE::CRpcMessage request = NewMessage( CAssetStoreRemoteContracts::GetControlContract(), "SecretRequest" );
    request.SetString( CAssetStoreRemoteContracts::SecretOperationField, CAssetServiceOperations::UnlockKeyring );
    request.SetString( CAssetStoreRemoteContracts::SecretCommandIdField, commandId );
    request.SetString( CAssetStoreRemoteContracts::SecretValueField, code.Reveal() );
    SPendingCall* pending = GUCEF_NEW SPendingCall();
    pending->operation = CAssetServiceOperations::UnlockKeyring;
    pending->rpcFutures.push_back( m_connection->CallAsync( CAssetStoreRemoteContracts::SecretPath, request, MakeOptions( false ), 0 ) );
    request.Clear( CAssetStoreRemoteContracts::SecretValueField );
    return Track( pending );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::Decode( SPendingCall& pending )
{GUCEF_TRACE;

    pending.decoded = true;
    pending.decodedAtMs = NowMs();
    GUCEF_DELETE pending.result;
    pending.result = GUCEF_NULL;
    if ( SPendingCall::KIND_WRITE == pending.kind )
    {
        CAssetServiceResult* outcome = GUCEF_NEW CAssetServiceResult();
        pending.result = outcome;
        pending.status = ASSETSTORE_STATUS_OK;
        for ( size_t i=0; i<pending.rpcFutures.size(); ++i )
        {
            COMCORE::CRpcCallResult rpc;
            if ( !COMCORE::CRpcEndpoint::TryGetCallResult( pending.rpcFutures[ i ], rpc ) || !rpc.status.IsOk() )
            {
                pending.status = rpc.status.IsOk() ? ASSETSTORE_STATUS_NETWORK_ERROR : CAssetStoreRemoteCodec::StatusOfRpcFailure( rpc.status );
                pending.message = rpc.status.IsOk() ? CORE::CString( "a block of the upload was not answered" ) : rpc.status.message;
                break;
            }
            CORE::CIDataNodeSerializable* answer = GUCEF_NULL;
            CORE::CString message;
            const TAssetStoreStatus status = CAssetStoreRemoteCodec::ReadAnswer( pending.operation, rpc.response, answer, message );
            CAssetServiceResult* blockOutcome = CORE::CICloneable::CloneableAs< CAssetServiceResult >( answer );
            if ( GUCEF_NULL != blockOutcome )
                *outcome = *blockOutcome;
            GUCEF_DELETE answer;
            if ( ASSETSTORE_STATUS_OK != status )
            {
                pending.status = status;
                pending.message = message;
                break;
            }
        }
        outcome->status = pending.status;
        if ( ASSETSTORE_STATUS_OK != pending.status )
            outcome->message = pending.message;
        return;
    }

    COMCORE::CRpcCallResult rpc;
    if ( pending.rpcFutures.empty() || !COMCORE::CRpcEndpoint::TryGetCallResult( pending.rpcFutures[ 0 ], rpc ) )
    {
        pending.status = ASSETSTORE_STATUS_UNAVAILABLE;
        pending.message = "the asset store service did not answer (no connection)";
        pending.result = CAssetStoreRemoteCodec::NewResult( pending.operation );
        SetFailureOf( pending.result, pending.status, pending.message );
        return;
    }
    if ( !rpc.status.IsOk() )
    {
        pending.status = CAssetStoreRemoteCodec::StatusOfRpcFailure( rpc.status );
        pending.message = rpc.status.message.IsNULLOrEmpty() ? rpc.status.ToString() : rpc.status.message;
        pending.result = CAssetStoreRemoteCodec::NewResult( pending.operation );
        SetFailureOf( pending.result, pending.status, pending.message );
        return;
    }
    pending.status = CAssetStoreRemoteCodec::ReadAnswer( pending.operation, rpc.response, pending.result, pending.message );
    pending.eventSeq = rpc.response.GetUInt64( CAssetStoreRemoteContracts::AnswerEventSeqField );

    // what the client keeps of the answers
    if ( CAssetServiceOperations::GetServiceInfo == pending.operation && GUCEF_NULL != pending.result )
    {
        CAssetServiceInfo* info = static_cast< CAssetServiceInfo* >( pending.result );
        info->implementationId = CAssetStoreServiceConfig::RemoteImplementationId;
        info->inProcess = false;
        if ( ASSETSTORE_STATUS_OK == pending.status )
        {
            m_catalogueAvailable = info->HasCapability( "catalogue" );
            UpdateStores( info->stores );
        }
    }
    else
    if ( CAssetServiceOperations::ListStores == pending.operation && GUCEF_NULL != pending.result && ASSETSTORE_STATUS_OK == pending.status )
    {
        UpdateStores( static_cast< CAssetStoreInfoPage* >( pending.result )->entries );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::Complete( SPendingCall& pending )
{GUCEF_TRACE;

    CORE::CICloneable* result = pending.result;
    pending.result = GUCEF_NULL;
    if ( IsOutcomeStatus( pending.operation, pending.status ) && GUCEF_NULL != result )
    {
        // a DUPLICATE of an ingest is an outcome: the status is in the record
        pending.source.Complete( result, true );
        return;
    }
    if ( ASSETSTORE_STATUS_CANCELLED == pending.status )
    {
        GUCEF_DELETE result;
        pending.source.Cancel();
        return;
    }
    // a failure ends with "status: message", the form of the in-process implementation
    GUCEF_DELETE result;
    CORE::CString info = AssetStoreStatusToString( pending.status );
    if ( !pending.message.IsNULLOrEmpty() )
        info += ": " + pending.message;
    pending.source.Fail( CORE::TASKSTATUS_EXCEPTION_DURING_RUN, info );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::ProcessPending( void )
{GUCEF_TRACE;

    TPendingVector done;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_pending.empty() )
            return;
        const UInt64 now = NowMs();
        const UInt64 lastEventSeq = GUCEF_NULL == m_connection ? 0 : m_connection->GetLastEventSeq();
        const bool connected = GUCEF_NULL != m_connection && DRNUTILS::REMOTECONNECTION_CONNECTED == m_connection->GetState();
        for ( size_t i=0; i<m_pending.size(); )
        {
            SPendingCall& pending = *m_pending[ i ];
            if ( !pending.decoded )
            {
                bool ready = true;
                for ( size_t f=0; f<pending.rpcFutures.size() && ready; ++f )
                    ready = pending.rpcFutures[ f ].IsReady() || !pending.rpcFutures[ f ].HasAFuture();
                if ( !ready )
                {
                    ++i;
                    continue;
                }
                Decode( pending );
            }
            // the events before the answer first (at most eventWaitMs, and not while the events cannot arrive)
            const bool eventsIn = 0 == pending.eventSeq || lastEventSeq + 1 >= pending.eventSeq;
            if ( !eventsIn && connected && now - pending.decodedAtMs < m_settings.eventWaitMs )
            {
                ++i;
                continue;
            }
            done.push_back( m_pending[ i ] );
            m_pending.erase( m_pending.begin() + i );
        }
    }
    for ( size_t i=0; i<done.size(); ++i )
    {
        Complete( *done[ i ] );
        GUCEF_DELETE done[ i ];
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::WaitForFuture( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    const bool pump = IsPumpingThread();
    CORE::PulseGeneratorPtr pulse = GetPulseGenerator();
    while ( !future.IsReady() && future.HasAFuture() )
    {
        if ( pump && !pulse.IsNULL() )
            pulse->ForceDirectPulse();
        ProcessPending();
        if ( !future.IsReady() )
            MT::PrecisionDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetStatusOfAnswer( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    const TAssetStoreStatus status = GetStatusOfFuture( future );
    if ( ASSETSTORE_STATUS_OK != status || !future.IsReady() )
        return status;
    // an outcome that is not OK (DUPLICATE of an ingest) is in the record
    CORE::CTaskPtr task = future.GetResult( 0 );
    const CAssetIngestResult* ingest = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CAssetIngestResult >( task->GetTaskData() );
    if ( GUCEF_NULL != ingest && ASSETSTORE_STATUS_DUPLICATE == ingest->status )
        return ingest->status;
    // a change of the settings carries its status in the record
    const CAssetSettingsChangeResult* settingsChange = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CAssetSettingsChangeResult >( task->GetTaskData() );
    return GUCEF_NULL != settingsChange ? settingsChange->status : status;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::SetFailureOf( CORE::CICloneable* result, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    CAssetStoreRemoteCodec::SetFailure( result, status, message );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::UpdateStores( const TAssetStoreInfoVector& stores )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_stores = stores;
    for ( size_t i=0; i<stores.size(); ++i )
    {
        TProviderMap::iterator p = m_providers.find( stores[ i ].storeId );
        if ( p != m_providers.end() )
        {
            CRemoteAssetStoreProvider* provider = static_cast< CRemoteAssetStoreProvider* >( (*p).second.GetPointerAlways() );
            provider->Update( stores[ i ].displayName, stores[ i ].signedIn );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::DetachProviders( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TProviderMap::iterator p = m_providers.begin(); p != m_providers.end(); ++p )
        static_cast< CRemoteAssetStoreProvider* >( (*p).second.GetPointerAlways() )->Detach();
    m_providers.clear();
    m_stores.clear();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreService::GetStoreProvider( const CORE::CString& storeId, TAssetStoreProviderPtr& provider )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TProviderMap::const_iterator p = m_providers.find( storeId );
    if ( p != m_providers.end() )
    {
        provider = (*p).second;
        return true;
    }
    for ( size_t i=0; i<m_stores.size(); ++i )
    {
        if ( m_stores[ i ].storeId == storeId )
        {
            TRemoteProviderPtr remote( GUCEF_NEW CRemoteAssetStoreProvider( this, storeId, m_stores[ i ].displayName, m_stores[ i ].signedIn ) );
            provider = remote;
            m_providers[ storeId ] = provider;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreService::TryGetListingImage( const CORE::CString& url, UInt32 size, CAssetThumbnail& image )
{GUCEF_TRACE;

    // listing pictures through the service are not built in this version (GetListingImage of the design, R7)
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreService::RequestListingImage( const CORE::CString& url, UInt32 size )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::CancelListingImage( const CORE::CString& url, UInt32 size )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ProcessPending();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::OnSessionEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const COMCORE::CRpcMessage& event = static_cast< DRNUTILS::TRemoteMessageEventData* >( eventData )->GetData();
    const CORE::CString kind = event.GetString( 2 );
    if ( CAssetStoreRemoteContracts::EventCatalogueChanged == kind )
    {
        CAssetCatalogueChange change;
        if ( ReadEventData( event, change ) )
            NotifyObservers( CatalogueChangedEvent, &change );
    }
    else
    if ( CAssetStoreRemoteContracts::EventJobChanged == kind || CAssetStoreRemoteContracts::EventJobRemoved == kind )
    {
        CAssetJobEntry job;
        if ( ReadEventData( event, job ) )
            NotifyObservers( CAssetStoreRemoteContracts::EventJobChanged == kind ? JobChangedEvent : JobRemovedEvent, &job );
    }
    else
    if ( CAssetStoreRemoteContracts::EventLocationState == kind )
    {
        CAssetLocationInfo location;
        if ( ReadEventData( event, location ) )
            NotifyObservers( LocationStateChangedEvent, &location );
    }
    else
    if ( CAssetStoreRemoteContracts::EventStoreState == kind )
    {
        CAssetStoreInfo store;
        if ( ReadEventData( event, store ) )
        {
            {
                MT::CScopeMutex lock( m_lock );
                for ( size_t i=0; i<m_stores.size(); ++i )
                    if ( m_stores[ i ].storeId == store.storeId )
                        m_stores[ i ] = store;
                TProviderMap::iterator p = m_providers.find( store.storeId );
                if ( p != m_providers.end() )
                    static_cast< CRemoteAssetStoreProvider* >( (*p).second.GetPointerAlways() )->Update( store.displayName, store.signedIn );
            }
            NotifyObservers( StoreStateChangedEvent, &store );
        }
    }
    else
    if ( CAssetStoreRemoteContracts::EventStoresChanged == kind )
    {
        // the list is read again in the background; its answer updates the stores of the providers
        ListStoresAsync();
        NotifyObservers( StoresChangedEvent );
    }
    else
    if ( CAssetStoreRemoteContracts::EventSignInPrompt == kind || CAssetStoreRemoteContracts::EventSignInPromptClosed == kind )
    {
        CAssetSignInPrompt prompt;
        if ( ReadEventData( event, prompt ) )
            NotifyObservers( CAssetStoreRemoteContracts::EventSignInPrompt == kind ? SignInPromptEvent : SignInPromptClosedEvent, &prompt );
    }
    else
    if ( CAssetStoreRemoteContracts::EventThumbnailUpdated == kind )
    {
        CAssetThumbnail thumbnail;
        if ( ReadEventData( event, thumbnail ) )
            NotifyObservers( ThumbnailUpdatedEvent, &thumbnail );
    }
    else
    if ( CAssetStoreRemoteContracts::EventServiceNotice == kind )
    {
        CAssetServiceNotice notice;
        if ( ReadEventData( event, notice ) )
            NotifyObservers( NoticeEvent, &notice );
    }
    else
    if ( CAssetStoreRemoteContracts::EventSettingsChanged == kind )
    {
        CAssetSettingsChangeResult change;
        if ( ReadEventData( event, change ) )
            NotifyObservers( SettingsChangedEvent, &change );
    }
    else
    if ( DRNUTILS::CRemoteSessionContracts::EventKindNotice == kind )
    {
        // the notices of the session layer; SERVICE_STOPPING comes as ServiceStoppingEvent of the connection (OnServiceStopping())
        CORE::CDataNode data;
        event.GetDataNode( 6, data );
        CAssetServiceNotice notice;
        notice.level = data.GetAttributeValueOrChildValueByName( "level" ).AsString( "info", true );
        notice.code = data.GetAttributeValueOrChildValueByName( "code" ).AsString();
        notice.text = data.GetAttributeValueOrChildValueByName( "text" ).AsString();
        NotifyObservers( NoticeEvent, &notice );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::OnServiceStopping( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // once per stop, whether the event stream brought SERVICE_STOPPING or a Hello or Resume was refused with DRAINING
    if ( GUCEF_NULL == eventData )
        return;
    const CORE::CDataNode& data = static_cast< DRNUTILS::TRemoteServiceStoppingEventData* >( eventData )->GetData();
    CAssetServiceNotice notice;
    notice.level = "warning";
    notice.code = DRNUTILS::CRemoteSessionContracts::EventKindServiceStopping;
    notice.text = "the service is stopping (" + data.GetAttributeValueOrChildValueByName( "reason" ).AsString() + "), expected back in " +
                  data.GetAttributeValueOrChildValueByName( "expectedBackSeconds" ).AsString() + " s";
    NotifyObservers( NoticeEvent, &notice );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::OnStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData || GUCEF_NULL == m_connection )
        return;
    const DRNUTILS::TRemoteConnectionState state = static_cast< DRNUTILS::TRemoteConnectionStateEventData* >( eventData )->GetData();
    CAssetServiceNotice notice;
    notice.code = DRNUTILS::RemoteConnectionStateToString( state );
    notice.level = DRNUTILS::REMOTECONNECTION_FAILED == state ? "error" : ( DRNUTILS::REMOTECONNECTION_CONNECTED == state ? "info" : "warning" );
    {
        MT::CScopeMutex lock( m_lock );
        if ( DRNUTILS::REMOTECONNECTION_CONNECTED == state )
            m_statusMessage.Clear();
        else
        if ( DRNUTILS::REMOTECONNECTION_FAILED == state )
            m_statusMessage = "the connection to the asset store service failed: " + m_connection->GetLastError();
        else
        if ( DRNUTILS::REMOTECONNECTION_RECONNECTING == state )
            m_statusMessage = "reconnecting to the asset store service";
        notice.text = m_statusMessage;
    }
    NotifyObservers( ConnectionStateChangedEvent, &notice );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreService::OnResync( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // events may have been missed (a gap of the stream, a new session): what is cached is to be read again
    CAssetCatalogueChange change;
    change.kind = ASSETCHANGE_RESYNC;
    NotifyObservers( CatalogueChangedEvent, &change );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetServiceInfo( CAssetServiceInfo& result )
{GUCEF_TRACE;

    return Await( GetServiceInfoAsync(  ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetCatalogueChanges( UInt64 sinceRevision, UInt32 maxEntries, CAssetCatalogueChangePage& result )
{GUCEF_TRACE;

    return Await( GetCatalogueChangesAsync( sinceRevision, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::Search( const CAssetSearchQuery& query, CAssetSearchResultPage& result )
{GUCEF_TRACE;

    return Await( SearchAsync( query ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::Suggest( const CORE::CString& prefix, UInt32 maxCount, CAssetServiceStringList& result )
{GUCEF_TRACE;

    return Await( SuggestAsync( prefix, maxCount ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetVocabulary( CAssetVocabularyInfo& result )
{GUCEF_TRACE;

    return Await( GetVocabularyAsync(  ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetPack( const CORE::CString& packId, CAssetPackRecord& result )
{GUCEF_TRACE;

    return Await( GetPackAsync( packId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetAssets( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRecordPage& result )
{GUCEF_TRACE;

    return Await( GetAssetsAsync( packId, pageToken, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetAsset( const CORE::CString& assetId, CAssetRecord& result )
{GUCEF_TRACE;

    return Await( GetAssetAsync( assetId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetRelations( const CORE::CString& assetId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRelationPage& result )
{GUCEF_TRACE;

    return Await( GetRelationsAsync( assetId, pageToken, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetIngestHistory( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetIngestHistoryPage& result )
{GUCEF_TRACE;

    return Await( GetIngestHistoryAsync( packId, pageToken, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetSavedSearches( const CORE::CString& pageToken, UInt32 maxEntries, CAssetSavedSearchPage& result )
{GUCEF_TRACE;

    return Await( GetSavedSearchesAsync( pageToken, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SaveSearch( const CORE::CString& commandId, const CAssetSavedSearch& search, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SaveSearchAsync( commandId, search ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::DeleteSavedSearch( const CORE::CString& commandId, const CORE::CString& searchId, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( DeleteSavedSearchAsync( commandId, searchId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RunSavedSearch( const CORE::CString& searchId, UInt32 pageIndex, CAssetSearchResultPage& result )
{GUCEF_TRACE;

    return Await( RunSavedSearchAsync( searchId, pageIndex ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetAssetFileSet( const CORE::CString& assetId, CAssetFileSet& result )
{GUCEF_TRACE;

    return Await( GetAssetFileSetAsync( assetId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetLicence( const CORE::CString& licenceOrPackId, CAssetLicence& result )
{GUCEF_TRACE;

    return Await( GetLicenceAsync( licenceOrPackId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetPackIndex( const CORE::CString& packId, bool forLearning, CAssetPackIndexInfo& result )
{GUCEF_TRACE;

    return Await( GetPackIndexAsync( packId, forLearning ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetResourceGroups( const CORE::CString& packId, bool forLearning, CAssetPackGroupPage& result )
{GUCEF_TRACE;

    return Await( GetResourceGroupsAsync( packId, forLearning ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup, const CORE::CString& pageToken, UInt32 maxEntries, bool forLearning, CAssetPackResourcePage& result )
{GUCEF_TRACE;

    return Await( GetResourcesAsync( packId, groupId, subGroup, pageToken, maxEntries, forLearning ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::UpdatePackMetadata( const CORE::CString& commandId, const CAssetPackRecord& pack, UInt64 expectedRevision, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( UpdatePackMetadataAsync( commandId, pack, expectedRevision ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SetClassification( const CORE::CString& commandId, const CORE::CString& objectId, const CAssetClassification& classification, const CORE::CString& source, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SetClassificationAsync( commandId, objectId, classification, source ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SetCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, const CORE::CString& categoryId, const CORE::CString& theme, const CORE::CString& kind, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SetCategoryMappingAsync( commandId, sourceId, storeCategory, categoryId, theme, kind ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RemoveCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( RemoveCategoryMappingAsync( commandId, sourceId, storeCategory ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SetSynonym( const CORE::CString& commandId, const CORE::CString& word, const CORE::CString& replacement, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SetSynonymAsync( commandId, word, replacement ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SetCategoryRule( const CORE::CString& commandId, const CAssetCategoryRule& rule, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SetCategoryRuleAsync( commandId, rule ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RemoveCategoryRule( const CORE::CString& commandId, const CORE::CString& ruleId, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( RemoveCategoryRuleAsync( commandId, ruleId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::CountRuleMatches( const CAssetCategoryRule& rule, CAssetServiceDocument& result )
{GUCEF_TRACE;

    return Await( CountRuleMatchesAsync( rule ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::Reclassify( const CORE::CString& commandId, const CORE::CString::StringVector& packIds, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( ReclassifyAsync( commandId, packIds ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RemovePack( const CORE::CString& commandId, const CORE::CString& packId, bool deleteFiles, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( RemovePackAsync( commandId, packId, deleteFiles ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetThumbnails( const CORE::CString::StringVector& assetIds, UInt32 size, CAssetThumbnailPage& result )
{GUCEF_TRACE;

    return Await( GetThumbnailsAsync( assetIds, size ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::PutThumbnail( const CORE::CString& commandId, const CAssetThumbnail& thumbnail, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( PutThumbnailAsync( commandId, thumbnail ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetMissingThumbnails( const CORE::CString& rendererId, const CORE::CString& rendererVersion, UInt32 size, UInt32 maxEntries, CAssetThumbnailPage& result )
{GUCEF_TRACE;

    return Await( GetMissingThumbnailsAsync( rendererId, rendererVersion, size, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::BeginUpload( const CORE::CString& commandId, const CORE::CString& name, UInt64 totalBytes, UInt32 fileCount, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( BeginUploadAsync( commandId, name, totalBytes, fileCount ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::QueryUpload( const CORE::CString& uploadId, CAssetFileStatPage& result )
{GUCEF_TRACE;

    return Await( QueryUploadAsync( uploadId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::CommitUpload( const CORE::CString& commandId, const CORE::CString& uploadId, const CAssetIngestRequest& ingest, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( CommitUploadAsync( commandId, uploadId, ingest ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::AbortUpload( const CORE::CString& commandId, const CORE::CString& uploadId, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( AbortUploadAsync( commandId, uploadId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetLocations( CAssetLocationPage& result )
{GUCEF_TRACE;

    return Await( GetLocationsAsync(  ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RefreshLocationStates( CAssetLocationPage& result )
{GUCEF_TRACE;

    return Await( RefreshLocationStatesAsync(  ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RelocateLocation( const CORE::CString& commandId, const CORE::CString& locationId, const CORE::CString& newVfsRoot, const CORE::CString& newFileSystemPath, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( RelocateLocationAsync( commandId, locationId, newVfsRoot, newFileSystemPath ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::VerifyLocation( const CORE::CString& commandId, const CORE::CString& locationId, CAssetVerificationReport& result )
{GUCEF_TRACE;

    return Await( VerifyLocationAsync( commandId, locationId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::Ingest( const CORE::CString& commandId, const CAssetIngestRequest& request, CAssetIngestResult& result )
{GUCEF_TRACE;

    return Await( IngestAsync( commandId, request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::ResumeIngest( const CORE::CString& commandId, const CORE::CString& ingestId, CAssetIngestResult& result )
{GUCEF_TRACE;

    return Await( ResumeIngestAsync( commandId, ingestId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetPendingIngests( CAssetServiceStringList& result )
{GUCEF_TRACE;

    return Await( GetPendingIngestsAsync(  ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::ListImportRoots( CAssetServiceStringList& result )
{GUCEF_TRACE;

    return Await( ListImportRootsAsync(  ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SetSourcePolicy( const CORE::CString& commandId, const CAssetSourcePolicy& policy, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SetSourcePolicyAsync( commandId, policy ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::ListJobs( const CORE::CString& pageToken, UInt32 maxEntries, CAssetJobPage& result )
{GUCEF_TRACE;

    return Await( ListJobsAsync( pageToken, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetJob( const CORE::CString& jobId, CAssetJobEntry& result )
{GUCEF_TRACE;

    return Await( GetJobAsync( jobId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::ControlJob( const CORE::CString& commandId, const CORE::CString& jobId, TAssetJobAction action, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( ControlJobAsync( commandId, jobId, action ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetJobResult( const CORE::CString& jobId, const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result )
{GUCEF_TRACE;

    return Await( GetJobResultAsync( jobId, section, pageToken, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::ListStores( CAssetStoreInfoPage& result )
{GUCEF_TRACE;

    return Await( ListStoresAsync(  ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::QueryLibrary( const CORE::CString& storeId, const CAssetStoreQuery& query, CAssetStoreLibraryPage& result )
{GUCEF_TRACE;

    return Await( QueryLibraryAsync( storeId, query ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetStoreItem( const CORE::CString& storeId, const CORE::CString& itemId, CAssetStoreItem& result )
{GUCEF_TRACE;

    return Await( GetStoreItemAsync( storeId, itemId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::EnqueueDownload( const CORE::CString& commandId, const CAssetDownloadRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( EnqueueDownloadAsync( commandId, request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::BeginSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( BeginSignInAsync( commandId, storeId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::CancelSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( CancelSignInAsync( commandId, storeId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SignOut( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SignOutAsync( commandId, storeId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetSignInPrompts( CAssetSignInPromptPage& result )
{GUCEF_TRACE;

    return Await( GetSignInPromptsAsync(  ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RequestConversion( const CORE::CString& commandId, const CAssetConversionJobRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( RequestConversionAsync( commandId, request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RequestExport( const CORE::CString& commandId, const CORE::CString::StringVector& assetIds, const CORE::CString& profile, const CORE::CString& format, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( RequestExportAsync( commandId, assetIds, profile, format ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RunPipelineQuery( const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{GUCEF_TRACE;

    return Await( RunPipelineQueryAsync( request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::ApplyPipelineChange( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{GUCEF_TRACE;

    return Await( ApplyPipelineChangeAsync( commandId, request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::StartPipelineJob( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( StartPipelineJobAsync( commandId, request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RenderSheet( const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{GUCEF_TRACE;

    return Await( RenderSheetAsync( request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::RequestLocalCopy( const CORE::CString& commandId, const CAssetLocalCopyRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( RequestLocalCopyAsync( commandId, request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SetOfflinePin( const CORE::CString& commandId, const CORE::CString& packId, bool pinned, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SetOfflinePinAsync( commandId, packId, pinned ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::LockKeyring( const CORE::CString& commandId, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( LockKeyringAsync( commandId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetAuditRecords( const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result )
{GUCEF_TRACE;

    return Await( GetAuditRecordsAsync( pageToken, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::ReindexFromStorage( const CORE::CString& commandId, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( ReindexFromStorageAsync( commandId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetResourceStatus( CAssetServiceDocument& result )
{GUCEF_TRACE;

    return Await( GetResourceStatusAsync(  ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetSyncMarks( const CORE::CString& storeId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetSyncMarkPage& result )
{GUCEF_TRACE;

    return Await( GetSyncMarksAsync( storeId, pageToken, maxEntries ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SetSyncMarks( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetSyncMarkPage& marks, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SetSyncMarksAsync( commandId, storeId, marks ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::GetSyncProfile( const CORE::CString& storeId, CAssetLibrarySyncProfile& result )
{GUCEF_TRACE;

    return Await( GetSyncProfileAsync( storeId ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SetSyncProfile( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetLibrarySyncProfile& profile, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( SetSyncProfileAsync( commandId, storeId, profile ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::DescribeSettings( const CAssetSettingsRequest& request, CAssetSettingsDescription& result )
{GUCEF_TRACE;

    return Await( DescribeSettingsAsync( request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::SetSettings( const CORE::CString& commandId, const CAssetSettingsRequest& request, CAssetSettingsChangeResult& result )
{GUCEF_TRACE;

    return Await( SetSettingsAsync( commandId, request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::ResetSettings( const CORE::CString& commandId, const CAssetSettingsRequest& request, CAssetSettingsChangeResult& result )
{GUCEF_TRACE;

    return Await( ResetSettingsAsync( commandId, request ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::BeginUploadFiles( const CORE::CString& commandId, const CORE::CString& name, const CAssetFileStatPage& files, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( BeginUploadFilesAsync( commandId, name, files ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::AnswerPrompt( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( AnswerPromptAsync( commandId, promptId, answer ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::WriteUploadBlock( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( WriteUploadBlockAsync( uploadId, relativePath, offset, data ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::ImportWebSession( const CORE::CString& commandId, const CORE::CString& storeId, const AUTH::CSecretString& session, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( ImportWebSessionAsync( commandId, storeId, session ), result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreService::UnlockKeyring( const CORE::CString& commandId, const AUTH::CSecretString& code, CAssetServiceResult& result )
{GUCEF_TRACE;

    return Await( UnlockKeyringAsync( commandId, code ), result );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteAssetStoreService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CRemoteAssetStoreService";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
