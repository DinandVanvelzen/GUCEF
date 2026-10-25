/*
 *  lobbyservicepluginEOS: gucefMULTIPLAY lobby service plugin for lobbies on Epic Online Services
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
#include "eos_lobby.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#endif /* GUCEF_AUTH_HAS_EOS ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H
#include "pluginglueEOS_CEosPlatformSdk.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H ? */

#include "lobbyservicepluginEOS_CEosLobbySdk.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace EOS {

using namespace PLUGINGLUE::EOS;

#ifndef GUCEF_AUTH_HAS_EOS

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD WITHOUT THE SDK                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CEosLobbySdk::IsCompiledIn( void )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbySdk::FillShim( SEosLobbyShim& shim )
{GUCEF_TRACE;

    shim = SEosLobbyShim();
    return false;
}

#else /* GUCEF_AUTH_HAS_EOS */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD WITH THE SDK. NEVER COMPILED OR RUN WHERE THIS WAS WRITTEN   //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SLobbySdkState
{
    MT::CMutex lock;
    GUCEF::vector< SEosLobbyEvent > events;
    bool notificationsAdded;
    EOS_NotificationId updateNotification;
    EOS_NotificationId memberUpdateNotification;
    EOS_NotificationId memberStatusNotification;
    EOS_NotificationId inviteNotification;

    SLobbySdkState( void )
        : lock(), events(), notificationsAdded( false ), updateNotification( EOS_INVALID_NOTIFICATIONID )
        , memberUpdateNotification( EOS_INVALID_NOTIFICATIONID ), memberStatusNotification( EOS_INVALID_NOTIFICATIONID )
        , inviteNotification( EOS_INVALID_NOTIFICATIONID )
    {}
};

static SLobbySdkState&
State( void )
{GUCEF_TRACE;

    static SLobbySdkState state;
    return state;
}

/** The data that travels with a request through the SDK to its callback */
struct SLobbyRequest
{
    UInt32 requestId;
    EOS_HLobbySearch search;
};

/*-------------------------------------------------------------------------*/

static EOS_HLobby
Lobby( void )
{GUCEF_TRACE;

    EOS_HPlatform platform = static_cast< EOS_HPlatform >( CEosPlatformSdk::GetPlatformHandle() );
    return GUCEF_NULL == platform ? GUCEF_NULL : EOS_Platform_GetLobbyInterface( platform );
}

/*-------------------------------------------------------------------------*/

static void
Push( const SEosLobbyEvent& event )
{GUCEF_TRACE;

    SLobbySdkState& state = State();
    MT::CScopeMutex scopeLock( state.lock );
    if ( state.events.size() < 1024 )
        state.events.push_back( event );
}

/*-------------------------------------------------------------------------*/

static EEosResult
Translate( EOS_EResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case EOS_Success :                  return EOSRESULT_SUCCESS;
        case EOS_Canceled :                 return EOSRESULT_CANCELLED;
        case EOS_NotFound :                 return EOSRESULT_NOT_FOUND;
        case EOS_AccessDenied :
        case EOS_MissingPermissions :
        case EOS_Lobby_NotOwner :
        case EOS_Lobby_NoPermission :       return EOSRESULT_NO_PERMISSION;           // names to confirm
        case EOS_InvalidParameters :        return EOSRESULT_INVALID_PARAMETERS;
        case EOS_InvalidUser :              return EOSRESULT_NOT_LOGGED_IN;
        case EOS_Lobby_TooManyPlayers :     return EOSRESULT_LOBBY_FULL;              // name to confirm
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

static CORE::CString
UserToText( EOS_ProductUserId id )
{GUCEF_TRACE;

    char buffer[ EosP2PUserIdBufferSize ];
    buffer[ 0 ] = '\0';
    if ( GUCEF_NULL == id || !EOS_ProductUserId_IsValid( id ) )
        return CORE::CString::Empty;
    int32_t length = (int32_t) sizeof( buffer );
    if ( EOS_Success != EOS_ProductUserId_ToString( id, buffer, &length ) )
        return CORE::CString::Empty;
    return CORE::CString( buffer );
}

/*-------------------------------------------------------------------------*/

static SLobbyRequest*
MakeRequest( UInt32 requestId )
{GUCEF_TRACE;

    SLobbyRequest* request = GUCEF_NEW SLobbyRequest();
    request->requestId = requestId;
    request->search = GUCEF_NULL;
    return request;
}

/*-------------------------------------------------------------------------*/

static void
PushEnd( SLobbyRequest* request, EEosLobbyEventKind kind, EOS_EResult result, const char* lobbyId )
{GUCEF_TRACE;

    SEosLobbyEvent event;
    event.requestId = request->requestId;
    event.kind = kind;
    event.result = Translate( result );
    event.sdkResultCode = (Int32) result;
    if ( GUCEF_NULL != lobbyId )
        event.lobbyId = lobbyId;
    Push( event );
    GUCEF_DELETE request;
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnCreateLobby( const EOS_Lobby_CreateLobbyCallbackInfo* data )
{GUCEF_TRACE;

    PushEnd( static_cast< SLobbyRequest* >( data->ClientData ), EOSLOBBYEVENT_CREATE, data->ResultCode, data->LobbyId );
}

static void EOS_CALL
OnUpdateLobby( const EOS_Lobby_UpdateLobbyCallbackInfo* data )
{GUCEF_TRACE;

    PushEnd( static_cast< SLobbyRequest* >( data->ClientData ), EOSLOBBYEVENT_UPDATE, data->ResultCode, data->LobbyId );
}

static void EOS_CALL
OnJoinLobby( const EOS_Lobby_JoinLobbyByIdCallbackInfo* data )
{GUCEF_TRACE;

    PushEnd( static_cast< SLobbyRequest* >( data->ClientData ), EOSLOBBYEVENT_JOIN, data->ResultCode, data->LobbyId );
}

static void EOS_CALL
OnLeaveLobby( const EOS_Lobby_LeaveLobbyCallbackInfo* data )
{GUCEF_TRACE;

    PushEnd( static_cast< SLobbyRequest* >( data->ClientData ), EOSLOBBYEVENT_LEAVE, data->ResultCode, data->LobbyId );
}

static void EOS_CALL
OnDestroyLobby( const EOS_Lobby_DestroyLobbyCallbackInfo* data )
{GUCEF_TRACE;

    PushEnd( static_cast< SLobbyRequest* >( data->ClientData ), EOSLOBBYEVENT_DESTROY, data->ResultCode, data->LobbyId );
}

static void EOS_CALL
OnKickMember( const EOS_Lobby_KickMemberCallbackInfo* data )
{GUCEF_TRACE;

    PushEnd( static_cast< SLobbyRequest* >( data->ClientData ), EOSLOBBYEVENT_KICK, data->ResultCode, data->LobbyId );
}

static void EOS_CALL
OnPromoteMember( const EOS_Lobby_PromoteMemberCallbackInfo* data )
{GUCEF_TRACE;

    PushEnd( static_cast< SLobbyRequest* >( data->ClientData ), EOSLOBBYEVENT_PROMOTE, data->ResultCode, data->LobbyId );
}

static void EOS_CALL
OnSendInvite( const EOS_Lobby_SendInviteCallbackInfo* data )
{GUCEF_TRACE;

    PushEnd( static_cast< SLobbyRequest* >( data->ClientData ), EOSLOBBYEVENT_SEND_INVITE, data->ResultCode, data->LobbyId );
}

/*-------------------------------------------------------------------------*/

static void
ReadAttribute( const EOS_Lobby_Attribute* source, SEosAttribute& target )
{GUCEF_TRACE;

    target.key = source->Data->Key;
    target.visibility = EOS_LAT_PRIVATE == source->Visibility ? EOSVISIBILITY_PRIVATE : EOSVISIBILITY_PUBLIC;
    if ( EOS_AT_STRING == source->Data->ValueType )
    {
        target.type = EOSATTRIBUTE_STRING;
        target.stringValue = GUCEF_NULL != source->Data->Value.AsUtf8 ? source->Data->Value.AsUtf8 : "";
    }
    else
    if ( EOS_AT_INT64 == source->Data->ValueType )
    {
        target.type = EOSATTRIBUTE_INT64;
        target.intValue = source->Data->Value.AsInt64;
    }
    else
    {
        // Booleans and doubles are not what this writes: they are read as a number and a text that no code of the plugin looks for
        target.type = EOSATTRIBUTE_INT64;
        target.intValue = EOS_AT_BOOLEAN == source->Data->ValueType ? ( source->Data->Value.AsBool ? 1 : 0 ) : 0;
    }
}

/*-------------------------------------------------------------------------*/

static bool
ReadDetails( EOS_HLobbyDetails handle, SEosLobbyDetails& details )
{GUCEF_TRACE;

    EOS_LobbyDetails_CopyInfoOptions infoOptions;
    memset( &infoOptions, 0, sizeof( infoOptions ) );
    infoOptions.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;
    EOS_LobbyDetails_Info* info = GUCEF_NULL;
    if ( EOS_Success != EOS_LobbyDetails_CopyInfo( handle, &infoOptions, &info ) || GUCEF_NULL == info )
        return false;
    details.lobbyId = info->LobbyId;
    details.ownerPuid = UserToText( info->LobbyOwnerUserId );
    details.bucketId = GUCEF_NULL != info->BucketId ? info->BucketId : "";
    details.maxMembers = info->MaxMembers;
    details.availableSlots = info->AvailableSlots;
    details.allowInvites = EOS_TRUE == info->bAllowInvites;
    switch ( info->PermissionLevel )
    {
        case EOS_LPL_JOINVIAPRESENCE :  details.permission = EOSLOBBYPERM_JOIN_VIA_PRESENCE; break;
        case EOS_LPL_INVITEONLY :       details.permission = EOSLOBBYPERM_INVITE_ONLY; break;
        default :                       details.permission = EOSLOBBYPERM_PUBLIC_ADVERTISED; break;
    }
    EOS_LobbyDetails_Info_Release( info );

    EOS_LobbyDetails_GetAttributeCountOptions countOptions;
    memset( &countOptions, 0, sizeof( countOptions ) );
    countOptions.ApiVersion = EOS_LOBBYDETAILS_GETATTRIBUTECOUNT_API_LATEST;
    uint32_t attributeCount = EOS_LobbyDetails_GetAttributeCount( handle, &countOptions );
    for ( uint32_t i=0; i<attributeCount; ++i )
    {
        EOS_LobbyDetails_CopyAttributeByIndexOptions copyOptions;
        memset( &copyOptions, 0, sizeof( copyOptions ) );
        copyOptions.ApiVersion = EOS_LOBBYDETAILS_COPYATTRIBUTEBYINDEX_API_LATEST;
        copyOptions.AttrIndex = i;
        EOS_Lobby_Attribute* attribute = GUCEF_NULL;
        if ( EOS_Success == EOS_LobbyDetails_CopyAttributeByIndex( handle, &copyOptions, &attribute ) && GUCEF_NULL != attribute )
        {
            SEosAttribute read;
            ReadAttribute( attribute, read );
            details.attributes.push_back( read );
            EOS_Lobby_Attribute_Release( attribute );
        }
    }

    EOS_LobbyDetails_GetMemberCountOptions memberCountOptions;
    memset( &memberCountOptions, 0, sizeof( memberCountOptions ) );
    memberCountOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST;
    uint32_t memberCount = EOS_LobbyDetails_GetMemberCount( handle, &memberCountOptions );
    for ( uint32_t m=0; m<memberCount; ++m )
    {
        EOS_LobbyDetails_GetMemberByIndexOptions memberOptions;
        memset( &memberOptions, 0, sizeof( memberOptions ) );
        memberOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST;
        memberOptions.MemberIndex = m;
        EOS_ProductUserId memberId = EOS_LobbyDetails_GetMemberByIndex( handle, &memberOptions );

        SEosLobbyMember member;
        member.puid = UserToText( memberId );

        EOS_LobbyDetails_GetMemberAttributeCountOptions memberAttributeCountOptions;
        memset( &memberAttributeCountOptions, 0, sizeof( memberAttributeCountOptions ) );
        memberAttributeCountOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERATTRIBUTECOUNT_API_LATEST;
        memberAttributeCountOptions.TargetUserId = memberId;
        uint32_t memberAttributeCount = EOS_LobbyDetails_GetMemberAttributeCount( handle, &memberAttributeCountOptions );
        for ( uint32_t a=0; a<memberAttributeCount; ++a )
        {
            EOS_LobbyDetails_CopyMemberAttributeByIndexOptions memberCopyOptions;
            memset( &memberCopyOptions, 0, sizeof( memberCopyOptions ) );
            memberCopyOptions.ApiVersion = EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYINDEX_API_LATEST;
            memberCopyOptions.TargetUserId = memberId;
            memberCopyOptions.AttrIndex = a;
            EOS_Lobby_Attribute* attribute = GUCEF_NULL;
            if ( EOS_Success == EOS_LobbyDetails_CopyMemberAttributeByIndex( handle, &memberCopyOptions, &attribute ) && GUCEF_NULL != attribute )
            {
                SEosAttribute read;
                ReadAttribute( attribute, read );
                member.attributes.push_back( read );
                EOS_Lobby_Attribute_Release( attribute );
            }
        }
        details.members.push_back( member );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnSearchFind( const EOS_LobbySearch_FindCallbackInfo* data )
{GUCEF_TRACE;

    SLobbyRequest* request = static_cast< SLobbyRequest* >( data->ClientData );
    SEosLobbyEvent event;
    event.requestId = request->requestId;
    event.kind = EOSLOBBYEVENT_SEARCH;
    event.result = Translate( data->ResultCode );
    event.sdkResultCode = (Int32) data->ResultCode;

    if ( EOS_Success == data->ResultCode && GUCEF_NULL != request->search )
    {
        EOS_LobbySearch_GetSearchResultCountOptions countOptions;
        memset( &countOptions, 0, sizeof( countOptions ) );
        countOptions.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
        uint32_t count = EOS_LobbySearch_GetSearchResultCount( request->search, &countOptions );
        for ( uint32_t i=0; i<count; ++i )
        {
            EOS_LobbySearch_CopySearchResultByIndexOptions copyOptions;
            memset( &copyOptions, 0, sizeof( copyOptions ) );
            copyOptions.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
            copyOptions.LobbyIndex = i;
            EOS_HLobbyDetails handle = GUCEF_NULL;
            if ( EOS_Success == EOS_LobbySearch_CopySearchResultByIndex( request->search, &copyOptions, &handle ) && GUCEF_NULL != handle )
            {
                SEosLobbyDetails details;
                if ( ReadDetails( handle, details ) )
                    event.found.push_back( details );
                EOS_LobbyDetails_Release( handle );
            }
        }
    }
    if ( GUCEF_NULL != request->search )
        EOS_LobbySearch_Release( request->search );
    Push( event );
    GUCEF_DELETE request;
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnLobbyUpdateReceived( const EOS_Lobby_LobbyUpdateReceivedCallbackInfo* data )
{GUCEF_TRACE;

    SEosLobbyEvent event;
    event.kind = EOSLOBBYEVENT_LOBBY_UPDATED;
    event.result = EOSRESULT_SUCCESS;
    event.lobbyId = data->LobbyId;
    Push( event );
}

static void EOS_CALL
OnLobbyMemberUpdateReceived( const EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo* data )
{GUCEF_TRACE;

    SEosLobbyEvent event;
    event.kind = EOSLOBBYEVENT_MEMBER_UPDATED;
    event.result = EOSRESULT_SUCCESS;
    event.lobbyId = data->LobbyId;
    event.userId = UserToText( data->TargetUserId );
    Push( event );
}

static void EOS_CALL
OnLobbyMemberStatusReceived( const EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo* data )
{GUCEF_TRACE;

    SEosLobbyEvent event;
    event.kind = EOSLOBBYEVENT_MEMBER_STATUS;
    event.result = EOSRESULT_SUCCESS;
    event.lobbyId = data->LobbyId;
    event.userId = UserToText( data->TargetUserId );
    switch ( data->CurrentStatus )
    {
        case EOS_LMS_JOINED :       event.status = EOSMEMBERSTATUS_JOINED; break;
        case EOS_LMS_LEFT :         event.status = EOSMEMBERSTATUS_LEFT; break;
        case EOS_LMS_DISCONNECTED : event.status = EOSMEMBERSTATUS_DISCONNECTED; break;
        case EOS_LMS_KICKED :       event.status = EOSMEMBERSTATUS_KICKED; break;
        case EOS_LMS_PROMOTED :     event.status = EOSMEMBERSTATUS_PROMOTED; break;
        default :                   event.status = EOSMEMBERSTATUS_CLOSED; break;
    }
    Push( event );
}

static void EOS_CALL
OnLobbyInviteReceived( const EOS_Lobby_LobbyInviteReceivedCallbackInfo* data )
{GUCEF_TRACE;

    // The invitation names the lobby through a handle
    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return;
    EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLEBYINVITEID_API_LATEST;
    options.InviteId = data->InviteId;
    EOS_HLobbyDetails handle = GUCEF_NULL;
    if ( EOS_Success != EOS_Lobby_CopyLobbyDetailsHandleByInviteId( lobby, &options, &handle ) || GUCEF_NULL == handle )
        return;
    SEosLobbyDetails details;
    if ( ReadDetails( handle, details ) )
    {
        SEosLobbyEvent event;
        event.kind = EOSLOBBYEVENT_INVITE_RECEIVED;
        event.result = EOSRESULT_SUCCESS;
        event.lobbyId = details.lobbyId;
        event.senderId = UserToText( data->TargetUserId );
        Push( event );
    }
    EOS_LobbyDetails_Release( handle );
}

/*-------------------------------------------------------------------------*/

static void
EnsureNotifications( void )
{GUCEF_TRACE;

    SLobbySdkState& state = State();
    EOS_HLobby lobby = Lobby();
    if ( state.notificationsAdded || GUCEF_NULL == lobby )
        return;

    EOS_Lobby_AddNotifyLobbyUpdateReceivedOptions updateOptions;
    memset( &updateOptions, 0, sizeof( updateOptions ) );
    updateOptions.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYUPDATERECEIVED_API_LATEST;
    state.updateNotification = EOS_Lobby_AddNotifyLobbyUpdateReceived( lobby, &updateOptions, GUCEF_NULL, &OnLobbyUpdateReceived );

    EOS_Lobby_AddNotifyLobbyMemberUpdateReceivedOptions memberUpdateOptions;
    memset( &memberUpdateOptions, 0, sizeof( memberUpdateOptions ) );
    memberUpdateOptions.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYMEMBERUPDATERECEIVED_API_LATEST;
    state.memberUpdateNotification = EOS_Lobby_AddNotifyLobbyMemberUpdateReceived( lobby, &memberUpdateOptions, GUCEF_NULL, &OnLobbyMemberUpdateReceived );

    EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions memberStatusOptions;
    memset( &memberStatusOptions, 0, sizeof( memberStatusOptions ) );
    memberStatusOptions.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYMEMBERSTATUSRECEIVED_API_LATEST;
    state.memberStatusNotification = EOS_Lobby_AddNotifyLobbyMemberStatusReceived( lobby, &memberStatusOptions, GUCEF_NULL, &OnLobbyMemberStatusReceived );

    EOS_Lobby_AddNotifyLobbyInviteReceivedOptions inviteOptions;
    memset( &inviteOptions, 0, sizeof( inviteOptions ) );
    inviteOptions.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYINVITERECEIVED_API_LATEST;
    state.inviteNotification = EOS_Lobby_AddNotifyLobbyInviteReceived( lobby, &inviteOptions, GUCEF_NULL, &OnLobbyInviteReceived );
    state.notificationsAdded = true;
}

/*-------------------------------------------------------------------------*/

static void
FillAttributeData( const SEosAttribute& source, EOS_Lobby_AttributeData& data )
{GUCEF_TRACE;

    memset( &data, 0, sizeof( data ) );
    data.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
    data.Key = source.key.C_String();
    if ( EOSATTRIBUTE_INT64 == source.type )
    {
        data.ValueType = EOS_AT_INT64;
        data.Value.AsInt64 = source.intValue;
    }
    else
    {
        data.ValueType = EOS_AT_STRING;
        data.Value.AsUtf8 = source.stringValue.C_String();
    }
}

/*-------------------------------------------------------------------------*/

static bool
SdkCreateLobby( void* context, UInt32 requestId, const SEosLobbyCreateOptions* options )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;
    EnsureNotifications();

    EOS_Lobby_CreateLobbyOptions create;
    memset( &create, 0, sizeof( create ) );
    create.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
    create.LocalUserId = EOS_ProductUserId_FromString( options->localUser.C_String() );
    create.MaxLobbyMembers = options->maxMembers;
    switch ( options->permission )
    {
        case EOSLOBBYPERM_JOIN_VIA_PRESENCE :   create.PermissionLevel = EOS_LPL_JOINVIAPRESENCE; break;
        case EOSLOBBYPERM_INVITE_ONLY :         create.PermissionLevel = EOS_LPL_INVITEONLY; break;
        default :                               create.PermissionLevel = EOS_LPL_PUBLICADVERTISED; break;
    }
    create.bPresenceEnabled = options->presenceEnabled ? EOS_TRUE : EOS_FALSE;
    create.bAllowInvites = options->allowInvites ? EOS_TRUE : EOS_FALSE;
    create.BucketId = options->bucketId.C_String();
    create.bDisableHostMigration = options->disableHostMigration ? EOS_TRUE : EOS_FALSE;
    create.bEnableRTCRoom = EOS_FALSE;
    create.bEnableJoinById = options->enableJoinById ? EOS_TRUE : EOS_FALSE;                          // to confirm
    create.bRejoinAfterKickRequiresInvite = options->rejoinAfterKickRequiresInvite ? EOS_TRUE : EOS_FALSE;   // to confirm
    EOS_Lobby_CreateLobby( lobby, &create, MakeRequest( requestId ), &OnCreateLobby );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SdkUpdateLobby( void* context, UInt32 requestId, const SEosLobbyUpdateOptions* options )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;

    EOS_Lobby_UpdateLobbyModificationOptions modificationOptions;
    memset( &modificationOptions, 0, sizeof( modificationOptions ) );
    modificationOptions.ApiVersion = EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST;
    modificationOptions.LobbyId = options->lobbyId.C_String();
    modificationOptions.LocalUserId = EOS_ProductUserId_FromString( options->localUser.C_String() );
    EOS_HLobbyModification modification = GUCEF_NULL;
    if ( EOS_Success != EOS_Lobby_UpdateLobbyModification( lobby, &modificationOptions, &modification ) || GUCEF_NULL == modification )
        return false;

    bool ok = true;
    if ( options->setPermission )
    {
        EOS_LobbyModification_SetPermissionLevelOptions permission;
        memset( &permission, 0, sizeof( permission ) );
        permission.ApiVersion = EOS_LOBBYMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
        switch ( options->permission )
        {
            case EOSLOBBYPERM_JOIN_VIA_PRESENCE :   permission.PermissionLevel = EOS_LPL_JOINVIAPRESENCE; break;
            case EOSLOBBYPERM_INVITE_ONLY :         permission.PermissionLevel = EOS_LPL_INVITEONLY; break;
            default :                               permission.PermissionLevel = EOS_LPL_PUBLICADVERTISED; break;
        }
        ok = ok && EOS_Success == EOS_LobbyModification_SetPermissionLevel( modification, &permission );
    }
    if ( options->setMaxMembers )
    {
        EOS_LobbyModification_SetMaxMembersOptions maxMembers;
        memset( &maxMembers, 0, sizeof( maxMembers ) );
        maxMembers.ApiVersion = EOS_LOBBYMODIFICATION_SETMAXMEMBERS_API_LATEST;
        maxMembers.MaxMembers = options->maxMembers;
        ok = ok && EOS_Success == EOS_LobbyModification_SetMaxMembers( modification, &maxMembers );
    }
    if ( options->setAllowInvites )
    {
        EOS_LobbyModification_SetInvitesAllowedOptions invites;
        memset( &invites, 0, sizeof( invites ) );
        invites.ApiVersion = EOS_LOBBYMODIFICATION_SETINVITESALLOWED_API_LATEST;
        invites.bInvitesAllowed = options->allowInvites ? EOS_TRUE : EOS_FALSE;
        ok = ok && EOS_Success == EOS_LobbyModification_SetInvitesAllowed( modification, &invites );
    }
    for ( size_t i=0; ok && i<options->setAttributes.size(); ++i )
    {
        EOS_Lobby_AttributeData data;
        FillAttributeData( options->setAttributes[ i ], data );
        EOS_LobbyModification_AddAttributeOptions add;
        memset( &add, 0, sizeof( add ) );
        add.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;
        add.Attribute = &data;
        add.Visibility = EOSVISIBILITY_PRIVATE == options->setAttributes[ i ].visibility ? EOS_LAT_PRIVATE : EOS_LAT_PUBLIC;
        ok = EOS_Success == EOS_LobbyModification_AddAttribute( modification, &add );
    }
    for ( size_t i=0; ok && i<options->removeAttributes.size(); ++i )
    {
        EOS_LobbyModification_RemoveAttributeOptions remove;
        memset( &remove, 0, sizeof( remove ) );
        remove.ApiVersion = EOS_LOBBYMODIFICATION_REMOVEATTRIBUTE_API_LATEST;
        remove.Key = options->removeAttributes[ i ].C_String();
        ok = EOS_Success == EOS_LobbyModification_RemoveAttribute( modification, &remove );
    }
    for ( size_t i=0; ok && i<options->setMemberAttributes.size(); ++i )
    {
        EOS_Lobby_AttributeData data;
        FillAttributeData( options->setMemberAttributes[ i ], data );
        EOS_LobbyModification_AddMemberAttributeOptions add;
        memset( &add, 0, sizeof( add ) );
        add.ApiVersion = EOS_LOBBYMODIFICATION_ADDMEMBERATTRIBUTE_API_LATEST;
        add.Attribute = &data;
        add.Visibility = EOSVISIBILITY_PRIVATE == options->setMemberAttributes[ i ].visibility ? EOS_LAT_PRIVATE : EOS_LAT_PUBLIC;
        ok = EOS_Success == EOS_LobbyModification_AddMemberAttribute( modification, &add );
    }
    for ( size_t i=0; ok && i<options->removeMemberAttributes.size(); ++i )
    {
        EOS_LobbyModification_RemoveMemberAttributeOptions remove;
        memset( &remove, 0, sizeof( remove ) );
        remove.ApiVersion = EOS_LOBBYMODIFICATION_REMOVEMEMBERATTRIBUTE_API_LATEST;
        remove.Key = options->removeMemberAttributes[ i ].C_String();
        ok = EOS_Success == EOS_LobbyModification_RemoveMemberAttribute( modification, &remove );
    }

    if ( ok )
    {
        EOS_Lobby_UpdateLobbyOptions update;
        memset( &update, 0, sizeof( update ) );
        update.ApiVersion = EOS_LOBBY_UPDATELOBBY_API_LATEST;
        update.LobbyModificationHandle = modification;
        EOS_Lobby_UpdateLobby( lobby, &update, MakeRequest( requestId ), &OnUpdateLobby );
    }
    EOS_LobbyModification_Release( modification );
    return ok;
}

/*-------------------------------------------------------------------------*/

static bool
SdkJoinLobbyById( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, bool presenceEnabled )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;
    EnsureNotifications();
    EOS_Lobby_JoinLobbyByIdOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_LOBBY_JOINLOBBYBYID_API_LATEST;
    options.LobbyId = lobbyId;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    options.bPresenceEnabled = presenceEnabled ? EOS_TRUE : EOS_FALSE;
    EOS_Lobby_JoinLobbyById( lobby, &options, MakeRequest( requestId ), &OnJoinLobby );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SdkLeaveLobby( void* context, UInt32 requestId, const char* lobbyId, const char* localUser )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;
    EOS_Lobby_LeaveLobbyOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
    options.LobbyId = lobbyId;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    EOS_Lobby_LeaveLobby( lobby, &options, MakeRequest( requestId ), &OnLeaveLobby );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SdkDestroyLobby( void* context, UInt32 requestId, const char* lobbyId, const char* localUser )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;
    EOS_Lobby_DestroyLobbyOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_LOBBY_DESTROYLOBBY_API_LATEST;
    options.LobbyId = lobbyId;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    EOS_Lobby_DestroyLobby( lobby, &options, MakeRequest( requestId ), &OnDestroyLobby );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SdkKickMember( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;
    EOS_Lobby_KickMemberOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_LOBBY_KICKMEMBER_API_LATEST;
    options.LobbyId = lobbyId;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    options.TargetUserId = EOS_ProductUserId_FromString( targetUser );
    EOS_Lobby_KickMember( lobby, &options, MakeRequest( requestId ), &OnKickMember );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SdkPromoteMember( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;
    EOS_Lobby_PromoteMemberOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_LOBBY_PROMOTEMEMBER_API_LATEST;
    options.LobbyId = lobbyId;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    options.TargetUserId = EOS_ProductUserId_FromString( targetUser );
    EOS_Lobby_PromoteMember( lobby, &options, MakeRequest( requestId ), &OnPromoteMember );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SdkSendInvite( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;
    EOS_Lobby_SendInviteOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_LOBBY_SENDINVITE_API_LATEST;
    options.LobbyId = lobbyId;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    options.TargetUserId = EOS_ProductUserId_FromString( targetUser );
    EOS_Lobby_SendInvite( lobby, &options, MakeRequest( requestId ), &OnSendInvite );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SdkSearchLobbies( void* context, UInt32 requestId, const SEosLobbySearchOptions* options )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;
    EnsureNotifications();

    EOS_Lobby_CreateLobbySearchOptions createOptions;
    memset( &createOptions, 0, sizeof( createOptions ) );
    createOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
    createOptions.MaxResults = options->maxResults;
    EOS_HLobbySearch search = GUCEF_NULL;
    if ( EOS_Success != EOS_Lobby_CreateLobbySearch( lobby, &createOptions, &search ) || GUCEF_NULL == search )
        return false;

    bool ok = true;
    if ( !options->lobbyId.IsNULLOrEmpty() )
    {
        EOS_LobbySearch_SetLobbyIdOptions idOptions;
        memset( &idOptions, 0, sizeof( idOptions ) );
        idOptions.ApiVersion = EOS_LOBBYSEARCH_SETLOBBYID_API_LATEST;
        idOptions.LobbyId = options->lobbyId.C_String();
        ok = ok && EOS_Success == EOS_LobbySearch_SetLobbyId( search, &idOptions );
    }
    if ( !options->targetUser.IsNULLOrEmpty() )
    {
        EOS_LobbySearch_SetTargetUserIdOptions userOptions;
        memset( &userOptions, 0, sizeof( userOptions ) );
        userOptions.ApiVersion = EOS_LOBBYSEARCH_SETTARGETUSERID_API_LATEST;
        userOptions.TargetUserId = EOS_ProductUserId_FromString( options->targetUser.C_String() );
        ok = ok && EOS_Success == EOS_LobbySearch_SetTargetUserId( search, &userOptions );
    }

    // The bucket and the free places are search parameters of their own name (to confirm: EOS_LOBBY_SEARCH_BUCKET_ID, EOS_LOBBY_SEARCH_MINSLOTSAVAILABLE)
    if ( !options->bucketId.IsNULLOrEmpty() )
    {
        EOS_Lobby_AttributeData bucket;
        memset( &bucket, 0, sizeof( bucket ) );
        bucket.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
        bucket.Key = EOS_LOBBY_SEARCH_BUCKET_ID;
        bucket.ValueType = EOS_AT_STRING;
        bucket.Value.AsUtf8 = options->bucketId.C_String();
        EOS_LobbySearch_SetParameterOptions parameter;
        memset( &parameter, 0, sizeof( parameter ) );
        parameter.ApiVersion = EOS_LOBBYSEARCH_SETPARAMETER_API_LATEST;
        parameter.Parameter = &bucket;
        parameter.ComparisonOp = EOS_CO_EQUAL;
        ok = ok && EOS_Success == EOS_LobbySearch_SetParameter( search, &parameter );
    }
    if ( options->minSlotsAvailable >= 0 )
    {
        EOS_Lobby_AttributeData slots;
        memset( &slots, 0, sizeof( slots ) );
        slots.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
        slots.Key = EOS_LOBBY_SEARCH_MINSLOTSAVAILABLE;
        slots.ValueType = EOS_AT_INT64;
        slots.Value.AsInt64 = options->minSlotsAvailable;
        EOS_LobbySearch_SetParameterOptions parameter;
        memset( &parameter, 0, sizeof( parameter ) );
        parameter.ApiVersion = EOS_LOBBYSEARCH_SETPARAMETER_API_LATEST;
        parameter.Parameter = &slots;
        parameter.ComparisonOp = EOS_CO_EQUAL;
        ok = ok && EOS_Success == EOS_LobbySearch_SetParameter( search, &parameter );
    }
    for ( size_t i=0; ok && i<options->parameters.size(); ++i )
    {
        const SEosLobbySearchParameter& source = options->parameters[ i ];
        EOS_Lobby_AttributeData data;
        memset( &data, 0, sizeof( data ) );
        data.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
        data.Key = source.key.C_String();
        if ( EOSATTRIBUTE_INT64 == source.type )
        {
            data.ValueType = EOS_AT_INT64;
            data.Value.AsInt64 = source.intValue;
        }
        else
        {
            data.ValueType = EOS_AT_STRING;
            data.Value.AsUtf8 = source.stringValue.C_String();
        }
        EOS_LobbySearch_SetParameterOptions parameter;
        memset( &parameter, 0, sizeof( parameter ) );
        parameter.ApiVersion = EOS_LOBBYSEARCH_SETPARAMETER_API_LATEST;
        parameter.Parameter = &data;
        switch ( source.comparison )
        {
            case EOSCOMPARE_NOT_EQUAL :             parameter.ComparisonOp = EOS_CO_NOTEQUAL; break;
            case EOSCOMPARE_GREATER_THAN :          parameter.ComparisonOp = EOS_CO_GREATERTHAN; break;
            case EOSCOMPARE_GREATER_THAN_OR_EQUAL : parameter.ComparisonOp = EOS_CO_GREATERTHANOREQUAL; break;
            case EOSCOMPARE_LESS_THAN :             parameter.ComparisonOp = EOS_CO_LESSTHAN; break;
            case EOSCOMPARE_LESS_THAN_OR_EQUAL :    parameter.ComparisonOp = EOS_CO_LESSTHANOREQUAL; break;
            default :                               parameter.ComparisonOp = EOS_CO_EQUAL; break;
        }
        ok = EOS_Success == EOS_LobbySearch_SetParameter( search, &parameter );
    }

    if ( !ok )
    {
        EOS_LobbySearch_Release( search );
        return false;
    }
    SLobbyRequest* request = MakeRequest( requestId );
    request->search = search;
    EOS_LobbySearch_FindOptions find;
    memset( &find, 0, sizeof( find ) );
    find.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
    find.LocalUserId = EOS_ProductUserId_FromString( options->localUser.C_String() );
    EOS_LobbySearch_Find( search, &find, request, &OnSearchFind );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SdkCopyLobbyDetails( void* context, const char* lobbyId, const char* localUser, SEosLobbyDetails* details )
{GUCEF_TRACE;

    EOS_HLobby lobby = Lobby();
    if ( GUCEF_NULL == lobby )
        return false;
    EOS_Lobby_CopyLobbyDetailsHandleOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLE_API_LATEST;
    options.LobbyId = lobbyId;
    options.LocalUserId = EOS_ProductUserId_FromString( localUser );
    EOS_HLobbyDetails handle = GUCEF_NULL;
    if ( EOS_Success != EOS_Lobby_CopyLobbyDetailsHandle( lobby, &options, &handle ) || GUCEF_NULL == handle )
        return false;
    bool ok = ReadDetails( handle, *details );
    EOS_LobbyDetails_Release( handle );
    return ok;
}

/*-------------------------------------------------------------------------*/

static bool
SdkPollEvent( void* context, SEosLobbyEvent* event )
{GUCEF_TRACE;

    SLobbySdkState& state = State();
    MT::CScopeMutex scopeLock( state.lock );
    if ( state.events.empty() )
        return false;
    *event = state.events.front();
    state.events.erase( state.events.begin() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbySdk::IsCompiledIn( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbySdk::FillShim( SEosLobbyShim& shim )
{GUCEF_TRACE;

    shim = SEosLobbyShim();
    CEosPlatformSdk::FillPlatformShim( shim.platform );
    shim.context = &State();
    shim.createLobby = &SdkCreateLobby;
    shim.updateLobby = &SdkUpdateLobby;
    shim.joinLobbyById = &SdkJoinLobbyById;
    shim.leaveLobby = &SdkLeaveLobby;
    shim.destroyLobby = &SdkDestroyLobby;
    shim.kickMember = &SdkKickMember;
    shim.promoteMember = &SdkPromoteMember;
    shim.sendInvite = &SdkSendInvite;
    shim.searchLobbies = &SdkSearchLobbies;
    shim.copyLobbyDetails = &SdkCopyLobbyDetails;
    shim.pollEvent = &SdkPollEvent;
    return true;
}

#endif /* GUCEF_AUTH_HAS_EOS ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
