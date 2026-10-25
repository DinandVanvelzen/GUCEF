/*
 *  authpluginSTEAM: gucefAUTH plugin for logging in with the Steam client
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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#include "authpluginSTEAM_CSteamRuntime.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace STEAM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

// The size of SteamErrMsg, the buffer SteamAPI_InitFlat writes its message in
static const UInt32 SteamErrorMessageSize = 1024;

// The most answers kept for the provider between two polls, the oldest go first
static const size_t MaxReadyResponses = 16;

// The size of the fixed part of GetTicketForWebApiResponse_t: HAuthTicket, EResult and the ticket size. To confirm
static const Int32 TicketResponseHeaderSize = 12;

// ESteamAPIInitResult k_ESteamAPIInitResult_OK
static const Int32 SteamInitResultOk = 0;

// The accessors of the interfaces carry the version of the interface, newest first. Which of them a runtime has is
// the version of the SDK it belongs to. To confirm against steam_api_flat.h of the SDK that is shipped
static const char* const SteamUserAccessors[] = { "SteamAPI_SteamUser_v023", "SteamAPI_SteamUser_v022", "SteamAPI_SteamUser_v021", "SteamAPI_SteamUser_v020", "SteamAPI_SteamUser_v019", GUCEF_NULL };
static const char* const SteamFriendsAccessors[] = { "SteamAPI_SteamFriends_v018", "SteamAPI_SteamFriends_v017", "SteamAPI_SteamFriends_v016", "SteamAPI_SteamFriends_v015", GUCEF_NULL };

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void*
ResolveFromModule( void* resolverContext, const char* symbolName )
{GUCEF_TRACE;

    CORE::TAnyPointer address = CORE::GetFunctionAddress( resolverContext, symbolName, 0 );
    return reinterpret_cast< void* >( address.funcPtr );
}

/*-------------------------------------------------------------------------*/

CSteamRuntime::CSteamRuntime( void )
    : m_module( GUCEF_NULL )
    , m_resolver( GUCEF_NULL )
    , m_resolverContext( GUCEF_NULL )
    , m_bound( false )
    , m_initialized( false )
    , m_ownsApi( true )
    , m_initFlat( GUCEF_NULL )
    , m_shutdown( GUCEF_NULL )
    , m_isSteamRunning( GUCEF_NULL )
    , m_getPipe( GUCEF_NULL )
    , m_manualDispatchInit( GUCEF_NULL )
    , m_manualDispatchRunFrame( GUCEF_NULL )
    , m_manualDispatchGetNextCallback( GUCEF_NULL )
    , m_manualDispatchFreeLastCallback( GUCEF_NULL )
    , m_getUser( GUCEF_NULL )
    , m_getFriends( GUCEF_NULL )
    , m_userGetSteamId( GUCEF_NULL )
    , m_userBLoggedOn( GUCEF_NULL )
    , m_userGetAuthTicketForWebApi( GUCEF_NULL )
    , m_userGetAuthSessionTicket( GUCEF_NULL )
    , m_userCancelAuthTicket( GUCEF_NULL )
    , m_userBeginAuthSession( GUCEF_NULL )
    , m_userEndAuthSession( GUCEF_NULL )
    , m_friendsGetPersonaName( GUCEF_NULL )
    , m_user( GUCEF_NULL )
    , m_friends( GUCEF_NULL )
    , m_pipe( 0 )
    , m_ready()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSteamRuntime::~CSteamRuntime()
{GUCEF_TRACE;

    Unload();
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CSteamRuntime::GetDefaultLibraryNames( void )
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #if defined( _WIN64 )
    names.push_back( "steam_api64.dll" );
    #else
    names.push_back( "steam_api.dll" );
    #endif
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE )
    names.push_back( "libsteam_api.dylib" );
    #else
    names.push_back( "libsteam_api.so" );
    #endif
    return names;
}

/*-------------------------------------------------------------------------*/

bool
CSteamRuntime::Load( const CORE::CString& libraryName )
{GUCEF_TRACE;

    if ( m_bound )
        return true;

    CORE::CString::StringVector names;
    if ( libraryName.IsNULLOrEmpty() )
        names = GetDefaultLibraryNames();
    else
        names.push_back( libraryName );

    for ( size_t i=0; i<names.size() && GUCEF_NULL == m_module; ++i )
        m_module = CORE::LoadModuleDynamicly( names[ i ] );

    if ( GUCEF_NULL == m_module )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamRuntime: The Steam runtime library " << names[ 0 ] << " cannot be loaded, Steam is not available" );
        return false;
    }

    if ( !Bind( &ResolveFromModule, m_module ) )
    {
        Unload();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void*
CSteamRuntime::Find( const char* name, bool required, bool& allFound ) const
{GUCEF_TRACE;

    void* address = m_resolver( m_resolverContext, name );
    if ( GUCEF_NULL == address && required )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamRuntime: The Steam runtime has no function " << name );
        allFound = false;
    }
    return address;
}

/*-------------------------------------------------------------------------*/

void*
CSteamRuntime::FindVersioned( const char* const* names, bool required, bool& allFound ) const
{GUCEF_TRACE;

    for ( size_t i=0; GUCEF_NULL != names[ i ]; ++i )
    {
        void* address = m_resolver( m_resolverContext, names[ i ] );
        if ( GUCEF_NULL != address )
            return address;
    }
    if ( required )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamRuntime: The Steam runtime has none of the versions of " << names[ 0 ] );
        allFound = false;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CSteamRuntime::Bind( TSteamSymbolResolver resolver, void* resolverContext )
{GUCEF_TRACE;

    if ( GUCEF_NULL == resolver )
        return false;

    m_resolver = resolver;
    m_resolverContext = resolverContext;

    // Function pointers are converted from the data pointer the symbol lookup gives: this is what the platform
    // loaders (dlsym, GetProcAddress) require
    bool allFound = true;
    m_initFlat = (TInitFlat) Find( "SteamAPI_InitFlat", true, allFound );
    m_shutdown = (TShutdown) Find( "SteamAPI_Shutdown", true, allFound );
    m_isSteamRunning = (TIsSteamRunning) Find( "SteamAPI_IsSteamRunning", false, allFound );
    m_getPipe = (TGetHSteamPipe) Find( "SteamAPI_GetHSteamPipe", true, allFound );
    m_manualDispatchInit = (TManualDispatchInit) Find( "SteamAPI_ManualDispatch_Init", true, allFound );
    m_manualDispatchRunFrame = (TManualDispatchRunFrame) Find( "SteamAPI_ManualDispatch_RunFrame", true, allFound );
    m_manualDispatchGetNextCallback = (TManualDispatchGetNextCallback) Find( "SteamAPI_ManualDispatch_GetNextCallback", true, allFound );
    m_manualDispatchFreeLastCallback = (TManualDispatchFreeLastCallback) Find( "SteamAPI_ManualDispatch_FreeLastCallback", true, allFound );
    m_getUser = (TGetInterface) FindVersioned( SteamUserAccessors, true, allFound );
    m_getFriends = (TGetInterface) FindVersioned( SteamFriendsAccessors, false, allFound );
    m_userGetSteamId = (TUserGetSteamId) Find( "SteamAPI_ISteamUser_GetSteamID", true, allFound );
    m_userBLoggedOn = (TUserBLoggedOn) Find( "SteamAPI_ISteamUser_BLoggedOn", false, allFound );
    m_userGetAuthTicketForWebApi = (TUserGetAuthTicketForWebApi) Find( "SteamAPI_ISteamUser_GetAuthTicketForWebApi", false, allFound );
    m_userGetAuthSessionTicket = (TUserGetAuthSessionTicket) Find( "SteamAPI_ISteamUser_GetAuthSessionTicket", false, allFound );
    m_userCancelAuthTicket = (TUserCancelAuthTicket) Find( "SteamAPI_ISteamUser_CancelAuthTicket", true, allFound );
    m_userBeginAuthSession = (TUserBeginAuthSession) Find( "SteamAPI_ISteamUser_BeginAuthSession", false, allFound );
    m_userEndAuthSession = (TUserEndAuthSession) Find( "SteamAPI_ISteamUser_EndAuthSession", false, allFound );
    m_friendsGetPersonaName = (TFriendsGetPersonaName) Find( "SteamAPI_ISteamFriends_GetPersonaName", false, allFound );

    if ( GUCEF_NULL == m_userGetAuthTicketForWebApi && GUCEF_NULL == m_userGetAuthSessionTicket )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamRuntime: The Steam runtime has no function to get an authentication ticket" );
        allFound = false;
    }

    m_bound = allFound;
    if ( !m_bound )
    {
        m_resolver = GUCEF_NULL;
        m_resolverContext = GUCEF_NULL;
    }
    return m_bound;
}

/*-------------------------------------------------------------------------*/

void
CSteamRuntime::SetOwnsApi( bool ownsApi )
{GUCEF_TRACE;

    m_ownsApi = ownsApi;
}

/*-------------------------------------------------------------------------*/

void
CSteamRuntime::Unload( void )
{GUCEF_TRACE;

    if ( m_ownsApi )
        ShimShutdown( this );
    m_initialized = false;
    m_bound = false;
    m_resolver = GUCEF_NULL;
    m_resolverContext = GUCEF_NULL;
    m_ready.clear();

    if ( GUCEF_NULL != m_module )
    {
        CORE::UnloadModuleDynamicly( m_module );
        m_module = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CSteamRuntime::IsBound( void ) const
{GUCEF_TRACE;

    return m_bound;
}

/*-------------------------------------------------------------------------*/

bool
CSteamRuntime::HasWebApiTicket( void ) const
{GUCEF_TRACE;

    return m_bound && GUCEF_NULL != m_userGetAuthTicketForWebApi;
}

/*-------------------------------------------------------------------------*/

bool
CSteamRuntime::FillShim( TSteamShim& shim )
{GUCEF_TRACE;

    if ( !m_bound )
        return false;

    shim = TSteamShim();
    shim.context = this;
    shim.init = &CSteamRuntime::ShimInit;
    shim.shutdown = &CSteamRuntime::ShimShutdown;
    shim.isInitialized = &CSteamRuntime::ShimIsInitialized;
    shim.isLoggedOn = &CSteamRuntime::ShimIsLoggedOn;
    shim.getSteamId = &CSteamRuntime::ShimGetSteamId;
    shim.getPersonaName = &CSteamRuntime::ShimGetPersonaName;
    shim.requestWebApiTicket = GUCEF_NULL != m_userGetAuthTicketForWebApi ? &CSteamRuntime::ShimRequestWebApiTicket : GUCEF_NULL;
    shim.requestSessionTicket = GUCEF_NULL != m_userGetAuthSessionTicket ? &CSteamRuntime::ShimRequestSessionTicket : GUCEF_NULL;
    shim.runFrame = &CSteamRuntime::ShimRunFrame;
    shim.pollTicketResponse = &CSteamRuntime::ShimPollTicketResponse;
    shim.cancelTicket = &CSteamRuntime::ShimCancelTicket;
    shim.beginAuthSession = GUCEF_NULL != m_userBeginAuthSession ? &CSteamRuntime::ShimBeginAuthSession : GUCEF_NULL;
    shim.endAuthSession = GUCEF_NULL != m_userEndAuthSession ? &CSteamRuntime::ShimEndAuthSession : GUCEF_NULL;
    return true;
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

bool
CSteamRuntime::ShimInit( void* context, char* errorText, UInt32 errorTextSize )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( runtime->m_initialized )
        return true;
    if ( !runtime->m_bound )
    {
        CopyText( "The Steam runtime library is not loaded", errorText, errorTextSize );
        return false;
    }
    if ( GUCEF_NULL != runtime->m_isSteamRunning && !runtime->m_isSteamRunning() )
    {
        CopyText( "The Steam client is not running", errorText, errorTextSize );
        return false;
    }

    char message[ SteamErrorMessageSize ];
    message[ 0 ] = '\0';
    Int32 initResult = runtime->m_initFlat( message );
    if ( SteamInitResultOk != initResult )
    {
        if ( '\0' == message[ 0 ] )
            CopyText( "The Steam API could not be initialized", errorText, errorTextSize );
        else
            CopyText( message, errorText, errorTextSize );
        return false;
    }

    // The plugin dispatches the callbacks itself, on a timer of the application
    runtime->m_manualDispatchInit();
    runtime->m_pipe = runtime->m_getPipe();
    runtime->m_user = runtime->m_getUser();
    runtime->m_friends = GUCEF_NULL != runtime->m_getFriends ? runtime->m_getFriends() : GUCEF_NULL;
    if ( GUCEF_NULL == runtime->m_user )
    {
        runtime->m_shutdown();
        CopyText( "The Steam API gave no user interface", errorText, errorTextSize );
        return false;
    }

    runtime->m_initialized = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSteamRuntime::ShimShutdown( void* context )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( runtime->m_initialized && GUCEF_NULL != runtime->m_shutdown )
        runtime->m_shutdown();
    runtime->m_initialized = false;
    runtime->m_user = GUCEF_NULL;
    runtime->m_friends = GUCEF_NULL;
    runtime->m_ready.clear();
}

/*-------------------------------------------------------------------------*/

bool
CSteamRuntime::ShimIsInitialized( void* context )
{GUCEF_TRACE;

    return static_cast< CSteamRuntime* >( context )->m_initialized;
}

/*-------------------------------------------------------------------------*/

bool
CSteamRuntime::ShimIsLoggedOn( void* context )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( !runtime->m_initialized )
        return false;
    // Without the function the check cannot be made and the ticket request is the test
    return GUCEF_NULL == runtime->m_userBLoggedOn ? true : runtime->m_userBLoggedOn( runtime->m_user );
}

/*-------------------------------------------------------------------------*/

UInt64
CSteamRuntime::ShimGetSteamId( void* context )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    return runtime->m_initialized ? runtime->m_userGetSteamId( runtime->m_user ) : 0;
}

/*-------------------------------------------------------------------------*/

bool
CSteamRuntime::ShimGetPersonaName( void* context, char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( !runtime->m_initialized || GUCEF_NULL == runtime->m_friends || GUCEF_NULL == runtime->m_friendsGetPersonaName )
        return false;
    const char* name = runtime->m_friendsGetPersonaName( runtime->m_friends );
    if ( GUCEF_NULL == name )
        return false;
    CopyText( name, buffer, bufferSize );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CSteamRuntime::ShimRequestWebApiTicket( void* context, const char* identity )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( !runtime->m_initialized || GUCEF_NULL == runtime->m_userGetAuthTicketForWebApi )
        return SteamInvalidTicketHandle;
    return runtime->m_userGetAuthTicketForWebApi( runtime->m_user, identity );
}

/*-------------------------------------------------------------------------*/

UInt32
CSteamRuntime::ShimRequestSessionTicket( void* context, UInt8* ticket, UInt32 ticketCapacity, UInt32* ticketSize )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( !runtime->m_initialized || GUCEF_NULL == runtime->m_userGetAuthSessionTicket )
        return SteamInvalidTicketHandle;
    // The networking identity is the last argument of newer SDKs, it is passed as NULL (none) in any case
    return runtime->m_userGetAuthSessionTicket( runtime->m_user, ticket, (Int32) ticketCapacity, ticketSize, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CSteamRuntime::DrainCallbacks( void )
{GUCEF_TRACE;

    TCallbackMsg message;
    memset( &message, 0, sizeof( message ) );
    while ( m_manualDispatchGetNextCallback( m_pipe, &message ) )
    {
        if ( TicketForWebApiCallbackId == message.callbackId && GUCEF_NULL != message.param && message.paramSize >= TicketResponseHeaderSize )
        {
            // Copied field by field: the callback structure is packed by the SDK and not necessarily aligned here
            SSteamTicketResponse response;
            Int32 ticketSize = 0;
            memcpy( &response.ticketHandle, message.param, sizeof( UInt32 ) );
            memcpy( &response.result, message.param + 4, sizeof( Int32 ) );
            memcpy( &ticketSize, message.param + 8, sizeof( Int32 ) );
            Int32 available = message.paramSize - TicketResponseHeaderSize;
            if ( ticketSize < 0 )
                ticketSize = 0;
            if ( ticketSize > available )
                ticketSize = available;
            if ( ticketSize > (Int32) SteamMaxTicketSize )
                ticketSize = (Int32) SteamMaxTicketSize;
            response.ticketSize = (UInt32) ticketSize;
            if ( ticketSize > 0 )
                memcpy( response.ticket, message.param + TicketResponseHeaderSize, (size_t) ticketSize );

            if ( m_ready.size() >= MaxReadyResponses )
                m_ready.erase( m_ready.begin() );
            m_ready.push_back( response );
        }
        m_manualDispatchFreeLastCallback( m_pipe );
        memset( &message, 0, sizeof( message ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CSteamRuntime::ShimRunFrame( void* context )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( !runtime->m_initialized )
        return;
    runtime->m_manualDispatchRunFrame( runtime->m_pipe );
    runtime->DrainCallbacks();
}

/*-------------------------------------------------------------------------*/

bool
CSteamRuntime::ShimPollTicketResponse( void* context, SSteamTicketResponse* response )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( runtime->m_ready.empty() || GUCEF_NULL == response )
        return false;
    *response = runtime->m_ready.front();
    runtime->m_ready.erase( runtime->m_ready.begin() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSteamRuntime::ShimCancelTicket( void* context, UInt32 ticketHandle )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( runtime->m_initialized && SteamInvalidTicketHandle != ticketHandle )
        runtime->m_userCancelAuthTicket( runtime->m_user, ticketHandle );
}

/*-------------------------------------------------------------------------*/

Int32
CSteamRuntime::ShimBeginAuthSession( void* context, const void* ticket, Int32 ticketSize, UInt64 steamId )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( !runtime->m_initialized || GUCEF_NULL == runtime->m_userBeginAuthSession )
        return -1;
    return runtime->m_userBeginAuthSession( runtime->m_user, ticket, ticketSize, steamId );
}

/*-------------------------------------------------------------------------*/

void
CSteamRuntime::ShimEndAuthSession( void* context, UInt64 steamId )
{GUCEF_TRACE;

    CSteamRuntime* runtime = static_cast< CSteamRuntime* >( context );
    if ( runtime->m_initialized && GUCEF_NULL != runtime->m_userEndAuthSession )
        runtime->m_userEndAuthSession( runtime->m_user, steamId );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
