/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifdef GUCEF_AUTH_HAS_EOS

// The headers of the Epic Online Services SDK, found through the include path the build was given
#include "eos_sdk.h"
#include "eos_p2p.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#endif /* GUCEF_AUTH_HAS_EOS ? */

#include "pluginglueEOS_CEosPlatformSdk.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace EOS {

#ifndef GUCEF_AUTH_HAS_EOS

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD WITHOUT THE SDK                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CEosPlatformSdk::IsCompiledIn( void )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSdk::FillPlatformShim( SEosPlatformShim& shim )
{GUCEF_TRACE;

    shim = SEosPlatformShim();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSdk::FillP2PShim( SEosP2PShim& shim )
{GUCEF_TRACE;

    shim = SEosP2PShim();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSdk::AdoptPlatform( void* platformHandle, bool tickedByApplication )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void*
CEosPlatformSdk::GetPlatformHandle( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

#else /* GUCEF_AUTH_HAS_EOS */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD WITH THE SDK. NEVER COMPILED OR RUN WHERE THIS WAS WRITTEN   //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The one platform of the process as the SDK has it */
struct SSdkState
{
    MT::CMutex lock;
    EOS_HPlatform platform;
    bool ownsPlatform;
    bool initializedSdk;
    bool tickedByApplication;
    EOS_ProductUserId localUser;
    EOS_NotificationId requestNotification;
    EOS_NotificationId establishedNotification;
    EOS_NotificationId interruptedNotification;
    EOS_NotificationId closedNotification;
    GUCEF::vector< SEosP2PEvent > events;

    SSdkState( void )
        : lock(), platform( GUCEF_NULL ), ownsPlatform( false ), initializedSdk( false ), tickedByApplication( false ), localUser( GUCEF_NULL )
        , requestNotification( EOS_INVALID_NOTIFICATIONID ), establishedNotification( EOS_INVALID_NOTIFICATIONID )
        , interruptedNotification( EOS_INVALID_NOTIFICATIONID ), closedNotification( EOS_INVALID_NOTIFICATIONID ), events()
    {}
};

static SSdkState&
State( void )
{GUCEF_TRACE;

    static SSdkState state;
    return state;
}

/*-------------------------------------------------------------------------*/

static void
CopyText( const char* text, char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL == buffer || 0 == bufferSize )
        return;
    size_t length = GUCEF_NULL != text ? strlen( text ) : 0;
    if ( length >= bufferSize )
        length = bufferSize - 1;
    if ( length > 0 )
        memcpy( buffer, text, length );
    buffer[ length ] = '\0';
}

/*-------------------------------------------------------------------------*/

static void
ProductUserIdToText( EOS_ProductUserId id, char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    buffer[ 0 ] = '\0';
    if ( GUCEF_NULL == id || !EOS_ProductUserId_IsValid( id ) )
        return;
    int32_t length = (int32_t) bufferSize;
    if ( EOS_Success != EOS_ProductUserId_ToString( id, buffer, &length ) )
        buffer[ 0 ] = '\0';
}

/*-------------------------------------------------------------------------*/

static EEosResult
TranslateResult( EOS_EResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case EOS_Success :                  return EOSRESULT_SUCCESS;
        case EOS_Canceled :                 return EOSRESULT_CANCELLED;
        case EOS_NotFound :                 return EOSRESULT_NOT_FOUND;
        case EOS_AccessDenied :
        case EOS_MissingPermissions :       return EOSRESULT_NO_PERMISSION;
        case EOS_InvalidParameters :        return EOSRESULT_INVALID_PARAMETERS;
        case EOS_InvalidUser :              return EOSRESULT_NOT_LOGGED_IN;
        case EOS_LimitExceeded :            return EOSRESULT_LIMIT_EXCEEDED;
        case EOS_NoConnection :
        case EOS_NetworkDisconnected :      return EOSRESULT_NO_CONNECTION;
        case EOS_TimedOut :                 return EOSRESULT_TIMEOUT;
        case EOS_AlreadyPending :
        case EOS_TooManyRequests :          return EOSRESULT_ALREADY_PENDING;
        default :                           return EOSRESULT_FAILED;
    }
}

/*-------------------------------------------------------------------------*/

static bool
SdkIsAvailable( void* context )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SdkCreatePlatform( void* context, const SEosPlatformConfig* config, char* errorText, UInt32 errorTextSize )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL != state.platform )
        return true;

    EOS_InitializeOptions initOptions;
    memset( &initOptions, 0, sizeof( initOptions ) );
    initOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
    initOptions.ProductName = config->productName;
    initOptions.ProductVersion = config->productVersion;
    EOS_EResult initResult = EOS_Initialize( &initOptions );
    if ( EOS_Success != initResult && EOS_AlreadyConfigured != initResult )
    {
        CopyText( "EOS_Initialize failed", errorText, errorTextSize );
        return false;
    }
    state.initializedSdk = EOS_Success == initResult;

    EOS_Platform_Options options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
    options.bIsServer = EOS_FALSE;
    options.ProductId = config->productId;
    options.SandboxId = config->sandboxId;
    options.DeploymentId = config->deploymentId;
    options.ClientCredentials.ClientId = config->clientId;
    options.ClientCredentials.ClientSecret = config->clientSecret;
    if ( GUCEF_NULL != config->encryptionKey && '\0' != config->encryptionKey[ 0 ] )
        options.EncryptionKey = config->encryptionKey;
    if ( GUCEF_NULL != config->cacheDirectory && '\0' != config->cacheDirectory[ 0 ] )
        options.CacheDirectory = config->cacheDirectory;
    // The overlay is not loaded into the application for the sake of a login
    options.Flags = EOS_PF_DISABLE_OVERLAY | EOS_PF_DISABLE_SOCIAL_OVERLAY;

    state.platform = EOS_Platform_Create( &options );
    state.ownsPlatform = true;
    state.tickedByApplication = false;
    if ( GUCEF_NULL == state.platform )
    {
        CopyText( "EOS_Platform_Create failed, check the product, sandbox, deployment and client ids", errorText, errorTextSize );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
RemoveNotifications( SSdkState& state )
{GUCEF_TRACE;

    if ( GUCEF_NULL == state.platform )
        return;
    EOS_HP2P p2p = EOS_Platform_GetP2PInterface( state.platform );
    if ( EOS_INVALID_NOTIFICATIONID != state.requestNotification )
        EOS_P2P_RemoveNotifyPeerConnectionRequest( p2p, state.requestNotification );
    if ( EOS_INVALID_NOTIFICATIONID != state.establishedNotification )
        EOS_P2P_RemoveNotifyPeerConnectionEstablished( p2p, state.establishedNotification );
    if ( EOS_INVALID_NOTIFICATIONID != state.interruptedNotification )
        EOS_P2P_RemoveNotifyPeerConnectionInterrupted( p2p, state.interruptedNotification );
    if ( EOS_INVALID_NOTIFICATIONID != state.closedNotification )
        EOS_P2P_RemoveNotifyPeerConnectionClosed( p2p, state.closedNotification );
    state.requestNotification = EOS_INVALID_NOTIFICATIONID;
    state.establishedNotification = EOS_INVALID_NOTIFICATIONID;
    state.interruptedNotification = EOS_INVALID_NOTIFICATIONID;
    state.closedNotification = EOS_INVALID_NOTIFICATIONID;
}

/*-------------------------------------------------------------------------*/

static void
SdkReleasePlatform( void* context )
{GUCEF_TRACE;

    SSdkState& state = State();
    RemoveNotifications( state );
    if ( state.ownsPlatform && GUCEF_NULL != state.platform )
    {
        EOS_Platform_Release( state.platform );
        if ( state.initializedSdk )
            EOS_Shutdown();
    }
    state.platform = GUCEF_NULL;
    state.ownsPlatform = false;
    state.initializedSdk = false;
    state.localUser = GUCEF_NULL;
    MT::CScopeMutex scopeLock( state.lock );
    state.events.clear();
}

/*-------------------------------------------------------------------------*/

static bool
SdkIsPlatformReady( void* context )
{GUCEF_TRACE;

    return GUCEF_NULL != State().platform;
}

/*-------------------------------------------------------------------------*/

static void
SdkTick( void* context )
{GUCEF_TRACE;

    SSdkState& state = State();
    // A platform of the application is ticked by the application when it says so
    if ( GUCEF_NULL != state.platform && !state.tickedByApplication )
        EOS_Platform_Tick( state.platform );
}

/*-------------------------------------------------------------------------*/

static void*
SdkGetPlatformHandle( void* context )
{GUCEF_TRACE;

    return State().platform;
}

/*-------------------------------------------------------------------------*/

static void
FillSocketId( EOS_P2P_SocketId& socketId, const char* socketName )
{GUCEF_TRACE;

    memset( &socketId, 0, sizeof( socketId ) );
    socketId.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
    strncpy( socketId.SocketName, socketName, sizeof( socketId.SocketName ) - 1 );
}

/*-------------------------------------------------------------------------*/

static void
PushP2PEvent( SSdkState& state, const SEosP2PEvent& event )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( state.lock );
    if ( state.events.size() < 1024 )
        state.events.push_back( event );
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnConnectionRequest( const EOS_P2P_OnIncomingConnectionRequestInfo* data )
{GUCEF_TRACE;

    SEosP2PEvent event;
    event.kind = EOSP2PEVENT_CONNECTION_REQUEST;
    ProductUserIdToText( data->LocalUserId, event.localUser, sizeof( event.localUser ) );
    ProductUserIdToText( data->RemoteUserId, event.remoteUser, sizeof( event.remoteUser ) );
    CopyText( data->SocketId->SocketName, event.socketName, sizeof( event.socketName ) );
    PushP2PEvent( State(), event );
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnConnectionEstablished( const EOS_P2P_OnPeerConnectionEstablishedInfo* data )
{GUCEF_TRACE;

    SEosP2PEvent event;
    event.kind = EOSP2PEVENT_CONNECTION_ESTABLISHED;
    ProductUserIdToText( data->LocalUserId, event.localUser, sizeof( event.localUser ) );
    ProductUserIdToText( data->RemoteUserId, event.remoteUser, sizeof( event.remoteUser ) );
    CopyText( data->SocketId->SocketName, event.socketName, sizeof( event.socketName ) );
    switch ( data->NetworkType )
    {
        case EOS_NCT_DirectConnection :     event.networkType = EOSNETWORK_DIRECT; break;
        case EOS_NCT_RelayedConnection :    event.networkType = EOSNETWORK_RELAYED; break;
        default :                           event.networkType = EOSNETWORK_NONE; break;
    }
    PushP2PEvent( State(), event );
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnConnectionInterrupted( const EOS_P2P_OnPeerConnectionInterruptedInfo* data )
{GUCEF_TRACE;

    SEosP2PEvent event;
    event.kind = EOSP2PEVENT_CONNECTION_INTERRUPTED;
    ProductUserIdToText( data->LocalUserId, event.localUser, sizeof( event.localUser ) );
    ProductUserIdToText( data->RemoteUserId, event.remoteUser, sizeof( event.remoteUser ) );
    CopyText( data->SocketId->SocketName, event.socketName, sizeof( event.socketName ) );
    PushP2PEvent( State(), event );
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnConnectionClosed( const EOS_P2P_OnRemoteConnectionClosedInfo* data )
{GUCEF_TRACE;

    SEosP2PEvent event;
    event.kind = EOSP2PEVENT_CONNECTION_CLOSED;
    ProductUserIdToText( data->LocalUserId, event.localUser, sizeof( event.localUser ) );
    ProductUserIdToText( data->RemoteUserId, event.remoteUser, sizeof( event.remoteUser ) );
    CopyText( data->SocketId->SocketName, event.socketName, sizeof( event.socketName ) );
    switch ( data->Reason )
    {
        case EOS_CCR_ClosedByLocalUser :    event.closeReason = EOSCLOSE_CLOSED_BY_LOCAL_USER; break;
        case EOS_CCR_ClosedByPeer :         event.closeReason = EOSCLOSE_CLOSED_BY_PEER; break;
        case EOS_CCR_TimedOut :             event.closeReason = EOSCLOSE_TIMED_OUT; break;
        case EOS_CCR_TooManyConnections :   event.closeReason = EOSCLOSE_TOO_MANY_CONNECTIONS; break;
        case EOS_CCR_InvalidMessage :       event.closeReason = EOSCLOSE_INVALID_MESSAGE; break;
        case EOS_CCR_InvalidData :          event.closeReason = EOSCLOSE_INVALID_DATA; break;
        case EOS_CCR_ConnectionFailed :     event.closeReason = EOSCLOSE_CONNECTION_FAILED; break;
        case EOS_CCR_ConnectionClosed :     event.closeReason = EOSCLOSE_CONNECTION_CLOSED; break;
        case EOS_CCR_NegotiationFailed :    event.closeReason = EOSCLOSE_NEGOTIATION_FAILED; break;
        case EOS_CCR_UnexpectedError :      event.closeReason = EOSCLOSE_UNEXPECTED_ERROR; break;
        default :                           event.closeReason = EOSCLOSE_UNKNOWN; break;
    }
    PushP2PEvent( State(), event );
}

/*-------------------------------------------------------------------------*/

static void
SdkSetLocalUser( void* context, const char* localUser )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == state.platform )
        return;

    RemoveNotifications( state );
    state.localUser = ( GUCEF_NULL != localUser && '\0' != localUser[ 0 ] ) ? EOS_ProductUserId_FromString( localUser ) : GUCEF_NULL;
    if ( GUCEF_NULL == state.localUser )
        return;

    EOS_HP2P p2p = EOS_Platform_GetP2PInterface( state.platform );

    // The notifications are for every socket of the local user (SocketId NULL)
    EOS_P2P_AddNotifyPeerConnectionRequestOptions requestOptions;
    memset( &requestOptions, 0, sizeof( requestOptions ) );
    requestOptions.ApiVersion = EOS_P2P_ADDNOTIFYPEERCONNECTIONREQUEST_API_LATEST;
    requestOptions.LocalUserId = state.localUser;
    requestOptions.SocketId = GUCEF_NULL;
    state.requestNotification = EOS_P2P_AddNotifyPeerConnectionRequest( p2p, &requestOptions, GUCEF_NULL, &OnConnectionRequest );

    EOS_P2P_AddNotifyPeerConnectionEstablishedOptions establishedOptions;
    memset( &establishedOptions, 0, sizeof( establishedOptions ) );
    establishedOptions.ApiVersion = EOS_P2P_ADDNOTIFYPEERCONNECTIONESTABLISHED_API_LATEST;
    establishedOptions.LocalUserId = state.localUser;
    establishedOptions.SocketId = GUCEF_NULL;
    state.establishedNotification = EOS_P2P_AddNotifyPeerConnectionEstablished( p2p, &establishedOptions, GUCEF_NULL, &OnConnectionEstablished );

    EOS_P2P_AddNotifyPeerConnectionInterruptedOptions interruptedOptions;
    memset( &interruptedOptions, 0, sizeof( interruptedOptions ) );
    interruptedOptions.ApiVersion = EOS_P2P_ADDNOTIFYPEERCONNECTIONINTERRUPTED_API_LATEST;
    interruptedOptions.LocalUserId = state.localUser;
    interruptedOptions.SocketId = GUCEF_NULL;
    state.interruptedNotification = EOS_P2P_AddNotifyPeerConnectionInterrupted( p2p, &interruptedOptions, GUCEF_NULL, &OnConnectionInterrupted );

    EOS_P2P_AddNotifyPeerConnectionClosedOptions closedOptions;
    memset( &closedOptions, 0, sizeof( closedOptions ) );
    closedOptions.ApiVersion = EOS_P2P_ADDNOTIFYPEERCONNECTIONCLOSED_API_LATEST;
    closedOptions.LocalUserId = state.localUser;
    closedOptions.SocketId = GUCEF_NULL;
    state.closedNotification = EOS_P2P_AddNotifyPeerConnectionClosed( p2p, &closedOptions, GUCEF_NULL, &OnConnectionClosed );
}

/*-------------------------------------------------------------------------*/

static EEosResult
SdkSendPacket( void* context, const char* localUser, const char* remoteUser, const char* socketName, UInt8 channel,
               const void* data, UInt32 size, EEosPacketReliability reliability, bool allowDelayedDelivery )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == state.platform )
        return EOSRESULT_FAILED;

    EOS_P2P_SocketId socketId;
    FillSocketId( socketId, socketName );

    EOS_P2P_SendPacketOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    options.RemoteUserId = EOS_ProductUserId_FromString( remoteUser );
    options.SocketId = &socketId;
    options.Channel = channel;
    options.DataLengthBytes = size;
    options.Data = data;
    options.bAllowDelayedDelivery = allowDelayedDelivery ? EOS_TRUE : EOS_FALSE;
    switch ( reliability )
    {
        case EOSRELIABILITY_RELIABLE_UNORDERED :    options.Reliability = EOS_PR_ReliableUnordered; break;
        case EOSRELIABILITY_RELIABLE_ORDERED :      options.Reliability = EOS_PR_ReliableOrdered; break;
        default :                                   options.Reliability = EOS_PR_UnreliableUnordered; break;
    }
    options.bDisableAutoAcceptConnection = EOS_FALSE;
    return TranslateResult( EOS_P2P_SendPacket( EOS_Platform_GetP2PInterface( state.platform ), &options ) );
}

/*-------------------------------------------------------------------------*/

static bool
SdkReceivePacket( void* context, const char* localUser, UInt32 bufferSize, void* buffer, UInt32* size,
                  char* remoteUser, char* socketName, UInt8* channel )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == state.platform )
        return false;
    EOS_HP2P p2p = EOS_Platform_GetP2PInterface( state.platform );
    EOS_ProductUserId local = EOS_ProductUserId_FromString( localUser );

    EOS_P2P_GetNextReceivedPacketSizeOptions sizeOptions;
    memset( &sizeOptions, 0, sizeof( sizeOptions ) );
    sizeOptions.ApiVersion = EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST;
    sizeOptions.LocalUserId = local;
    sizeOptions.RequestedChannel = GUCEF_NULL;
    uint32_t nextSize = 0;
    if ( EOS_Success != EOS_P2P_GetNextReceivedPacketSize( p2p, &sizeOptions, &nextSize ) )
        return false;

    EOS_P2P_ReceivePacketOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
    options.LocalUserId = local;
    options.MaxDataSizeBytes = bufferSize;
    options.RequestedChannel = GUCEF_NULL;

    EOS_ProductUserId peer = GUCEF_NULL;
    EOS_P2P_SocketId socketId;
    memset( &socketId, 0, sizeof( socketId ) );
    uint8_t receivedChannel = 0;
    uint32_t written = 0;
    if ( EOS_Success != EOS_P2P_ReceivePacket( p2p, &options, &peer, &socketId, &receivedChannel, buffer, &written ) )
        return false;

    *size = written;
    *channel = receivedChannel;
    ProductUserIdToText( peer, remoteUser, EosP2PUserIdBufferSize );
    CopyText( socketId.SocketName, socketName, EosP2PSocketBufferSize );
    return true;
}

/*-------------------------------------------------------------------------*/

static EEosResult
SdkAcceptConnection( void* context, const char* localUser, const char* remoteUser, const char* socketName )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == state.platform )
        return EOSRESULT_FAILED;
    EOS_P2P_SocketId socketId;
    FillSocketId( socketId, socketName );
    EOS_P2P_AcceptConnectionOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_P2P_ACCEPTCONNECTION_API_LATEST;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    options.RemoteUserId = EOS_ProductUserId_FromString( remoteUser );
    options.SocketId = &socketId;
    return TranslateResult( EOS_P2P_AcceptConnection( EOS_Platform_GetP2PInterface( state.platform ), &options ) );
}

/*-------------------------------------------------------------------------*/

static EEosResult
SdkCloseConnection( void* context, const char* localUser, const char* remoteUser, const char* socketName )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == state.platform )
        return EOSRESULT_FAILED;
    EOS_P2P_SocketId socketId;
    FillSocketId( socketId, socketName );
    EOS_P2P_CloseConnectionOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_P2P_CLOSECONNECTION_API_LATEST;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    options.RemoteUserId = EOS_ProductUserId_FromString( remoteUser );
    options.SocketId = &socketId;
    return TranslateResult( EOS_P2P_CloseConnection( EOS_Platform_GetP2PInterface( state.platform ), &options ) );
}

/*-------------------------------------------------------------------------*/

static EEosResult
SdkCloseConnections( void* context, const char* localUser, const char* socketName )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == state.platform )
        return EOSRESULT_FAILED;
    EOS_P2P_SocketId socketId;
    FillSocketId( socketId, socketName );
    EOS_P2P_CloseConnectionsOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_P2P_CLOSECONNECTIONS_API_LATEST;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    options.SocketId = &socketId;
    return TranslateResult( EOS_P2P_CloseConnections( EOS_Platform_GetP2PInterface( state.platform ), &options ) );
}

/*-------------------------------------------------------------------------*/

static bool
SdkPollP2PEvent( void* context, SEosP2PEvent* event )
{GUCEF_TRACE;

    SSdkState& state = State();
    MT::CScopeMutex scopeLock( state.lock );
    if ( state.events.empty() )
        return false;
    *event = state.events.front();
    state.events.erase( state.events.begin() );
    return true;
}

/*-------------------------------------------------------------------------*/

static EEosResult
SdkSetRelayControl( void* context, EEosRelayControl control )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == state.platform )
        return EOSRESULT_FAILED;
    EOS_P2P_SetRelayControlOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_P2P_SETRELAYCONTROL_API_LATEST;
    switch ( control )
    {
        case EOSRELAY_NO_RELAYS :       options.RelayControl = EOS_RC_NoRelays; break;
        case EOSRELAY_FORCE_RELAYS :    options.RelayControl = EOS_RC_ForceRelays; break;
        default :                       options.RelayControl = EOS_RC_AllowRelays; break;
    }
    return TranslateResult( EOS_P2P_SetRelayControl( EOS_Platform_GetP2PInterface( state.platform ), &options ) );
}

/*-------------------------------------------------------------------------*/

static EEosNatType
SdkGetNatType( void* context )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == state.platform )
        return EOSNAT_UNKNOWN;
    EOS_P2P_GetNATTypeOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_P2P_GETNATTYPE_API_LATEST;
    EOS_ENATType type = EOS_NAT_Unknown;
    if ( EOS_Success != EOS_P2P_GetNATType( EOS_Platform_GetP2PInterface( state.platform ), &options, &type ) )
        return EOSNAT_UNKNOWN;
    switch ( type )
    {
        case EOS_NAT_Open :     return EOSNAT_OPEN;
        case EOS_NAT_Moderate : return EOSNAT_MODERATE;
        case EOS_NAT_Strict :   return EOSNAT_STRICT;
        default :               return EOSNAT_UNKNOWN;
    }
}

/*-------------------------------------------------------------------------*/

static EEosResult
SdkSetPacketQueueSize( void* context, UInt64 incomingMaxBytes, UInt64 outgoingMaxBytes )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == state.platform )
        return EOSRESULT_FAILED;
    EOS_P2P_SetPacketQueueSizeOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_P2P_SETPACKETQUEUESIZE_API_LATEST;
    options.IncomingPacketQueueMaxSizeBytes = incomingMaxBytes;
    options.OutgoingPacketQueueMaxSizeBytes = outgoingMaxBytes;
    return TranslateResult( EOS_P2P_SetPacketQueueSize( EOS_Platform_GetP2PInterface( state.platform ), &options ) );
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSdk::IsCompiledIn( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSdk::FillPlatformShim( SEosPlatformShim& shim )
{GUCEF_TRACE;

    shim = SEosPlatformShim();
    shim.context = &State();
    shim.isSdkAvailable = &SdkIsAvailable;
    shim.createPlatform = &SdkCreatePlatform;
    shim.releasePlatform = &SdkReleasePlatform;
    shim.isPlatformReady = &SdkIsPlatformReady;
    shim.tick = &SdkTick;
    shim.getPlatformHandle = &SdkGetPlatformHandle;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSdk::FillP2PShim( SEosP2PShim& shim )
{GUCEF_TRACE;

    shim = SEosP2PShim();
    FillPlatformShim( shim.platform );
    shim.context = &State();
    shim.sendPacket = &SdkSendPacket;
    shim.receivePacket = &SdkReceivePacket;
    shim.acceptConnection = &SdkAcceptConnection;
    shim.closeConnection = &SdkCloseConnection;
    shim.closeConnections = &SdkCloseConnections;
    shim.pollEvent = &SdkPollP2PEvent;
    shim.setRelayControl = &SdkSetRelayControl;
    shim.getNatType = &SdkGetNatType;
    shim.setPacketQueueSize = &SdkSetPacketQueueSize;
    shim.setLocalUser = &SdkSetLocalUser;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSdk::AdoptPlatform( void* platformHandle, bool tickedByApplication )
{GUCEF_TRACE;

    SSdkState& state = State();
    if ( GUCEF_NULL == platformHandle || GUCEF_NULL != state.platform )
        return false;
    state.platform = static_cast< EOS_HPlatform >( platformHandle );
    state.ownsPlatform = false;
    state.tickedByApplication = tickedByApplication;
    return true;
}

/*-------------------------------------------------------------------------*/

void*
CEosPlatformSdk::GetPlatformHandle( void )
{GUCEF_TRACE;

    return State().platform;
}

#endif /* GUCEF_AUTH_HAS_EOS ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
