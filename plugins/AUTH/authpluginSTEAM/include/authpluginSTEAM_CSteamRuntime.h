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

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMRUNTIME_H
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMRUNTIME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H
#include "authpluginSTEAM_SteamShim.h"
#define GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Finds a symbol of the runtime library by name, NULL when it is not there */
typedef void* ( *TSteamSymbolResolver )( void* resolverContext, const char* symbolName );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The flat C interface of the Steam API (steam_api_flat.h of the Steamworks SDK) as far as the plugin uses it,
 *  bound by name from the Steam runtime library at run time. The prototypes are declared in this class from the
 *  published flat API, so building the plugin needs neither the SDK nor its headers and nothing of the SDK is
 *  copied here.
 *
 *  UNVERIFIED AGAINST THE REAL SDK. This machine has no Steamworks SDK and no Steam client, so the names, the
 *  prototypes and the layout of the callback structures below are written from the SDK documentation and have only
 *  been exercised against a fake runtime in the tests. Marked "to confirm" are the points that have to be checked
 *  against the steam_api_flat.h and steam_api.json of the SDK version that is shipped:
 *    - the version suffixes of the interface accessors (SteamAPI_SteamUser_v0NN, SteamAPI_SteamFriends_v0NN): the
 *      newest known suffixes are tried first, older ones after it
 *    - the callback id of GetTicketForWebApiResponse_t (k_iSteamUserCallbacks + 68) and its layout
 *    - the signature of SteamAPI_ISteamUser_GetAuthSessionTicket (4 or 5 arguments; 5 are always passed, which is
 *      harmless for a callee that takes 4 on the calling conventions Steam is built for)
 *    - SteamAPI_InitFlat taking a pointer to the 1024 byte SteamErrMsg
 *    - SteamAPI_ManualDispatch_*: the plugin takes over the dispatching of callbacks of the process; an
 *      application that dispatches callbacks itself has to use the "external" callback mode of the settings
 *
 *  The same table of prototypes is bound whether the symbols come from the runtime library (Load()) or from the
 *  fake of a test (Bind()), so what the tests run is the code that runs against Steam.
 */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSteamRuntime
{
    public:

    /** k_iSteamUserCallbacks (100) + 68: GetTicketForWebApiResponse_t. To confirm */
    static const Int32 TicketForWebApiCallbackId = 168;

    /** The layout of CallbackMsg_t of the manual dispatch interface */
    struct SCallbackMsg
    {
        Int32 userHandle;           /**< HSteamUser */
        Int32 callbackId;           /**< k_iCallback of the callback structure */
        UInt8* param;               /**< the callback structure, valid until FreeLastCallback */
        Int32 paramSize;
    };
    typedef struct SCallbackMsg TCallbackMsg;

    CSteamRuntime( void );

    ~CSteamRuntime();

    /**
     *  Loads the runtime library (steam_api64.dll, steam_api.dll, libsteam_api.so, libsteam_api.dylib by platform or
     *  the given name or path) and binds the functions
     */
    bool Load( const CORE::CString& libraryName = CORE::CString::Empty );

    /** Binds the functions from any source of symbols */
    bool Bind( TSteamSymbolResolver resolver, void* resolverContext );

    /** Whether Unload() and the destructor call SteamAPI_Shutdown when the API was initialized through this runtime (default true) */
    void SetOwnsApi( bool ownsApi );

    /** Releases the library; the functions are unbound */
    void Unload( void );

    /** The functions the plugin cannot do without are bound */
    bool IsBound( void ) const;

    /** Whether GetAuthTicketForWebApi is there (SDK 1.57 and later) */
    bool HasWebApiTicket( void ) const;

    /** Fills the table of the provider with functions that run on this runtime. False when the runtime is not bound */
    bool FillShim( TSteamShim& shim );

    /** The library names that Load() tries for the platform */
    static CORE::CString::StringVector GetDefaultLibraryNames( void );

    private:

    CSteamRuntime( const CSteamRuntime& src );
    CSteamRuntime& operator=( const CSteamRuntime& src );

    // The prototypes of the flat API
    typedef Int32 ( *TInitFlat )( char* errorMessage );
    typedef void ( *TShutdown )( void );
    typedef bool ( *TIsSteamRunning )( void );
    typedef Int32 ( *TGetHSteamPipe )( void );
    typedef void ( *TManualDispatchInit )( void );
    typedef void ( *TManualDispatchRunFrame )( Int32 pipe );
    typedef bool ( *TManualDispatchGetNextCallback )( Int32 pipe, TCallbackMsg* message );
    typedef void ( *TManualDispatchFreeLastCallback )( Int32 pipe );
    typedef void* ( *TGetInterface )( void );
    typedef UInt64 ( *TUserGetSteamId )( void* user );
    typedef bool ( *TUserBLoggedOn )( void* user );
    typedef UInt32 ( *TUserGetAuthTicketForWebApi )( void* user, const char* identity );
    typedef UInt32 ( *TUserGetAuthSessionTicket )( void* user, void* ticket, Int32 ticketCapacity, UInt32* ticketSize, const void* networkingIdentity );
    typedef void ( *TUserCancelAuthTicket )( void* user, UInt32 ticketHandle );
    typedef Int32 ( *TUserBeginAuthSession )( void* user, const void* ticket, Int32 ticketSize, UInt64 steamId );
    typedef void ( *TUserEndAuthSession )( void* user, UInt64 steamId );
    typedef const char* ( *TFriendsGetPersonaName )( void* friends );

    static bool ShimInit( void* context, char* errorText, UInt32 errorTextSize );
    static void ShimShutdown( void* context );
    static bool ShimIsInitialized( void* context );
    static bool ShimIsLoggedOn( void* context );
    static UInt64 ShimGetSteamId( void* context );
    static bool ShimGetPersonaName( void* context, char* buffer, UInt32 bufferSize );
    static UInt32 ShimRequestWebApiTicket( void* context, const char* identity );
    static UInt32 ShimRequestSessionTicket( void* context, UInt8* ticket, UInt32 ticketCapacity, UInt32* ticketSize );
    static void ShimRunFrame( void* context );
    static bool ShimPollTicketResponse( void* context, SSteamTicketResponse* response );
    static void ShimCancelTicket( void* context, UInt32 ticketHandle );
    static Int32 ShimBeginAuthSession( void* context, const void* ticket, Int32 ticketSize, UInt64 steamId );
    static void ShimEndAuthSession( void* context, UInt64 steamId );

    void* Find( const char* name, bool required, bool& allFound ) const;
    void* FindVersioned( const char* const* names, bool required, bool& allFound ) const;
    void DrainCallbacks( void );

    private:

    void* m_module;
    TSteamSymbolResolver m_resolver;
    void* m_resolverContext;
    bool m_bound;
    bool m_initialized;
    bool m_ownsApi;

    TInitFlat m_initFlat;
    TShutdown m_shutdown;
    TIsSteamRunning m_isSteamRunning;
    TGetHSteamPipe m_getPipe;
    TManualDispatchInit m_manualDispatchInit;
    TManualDispatchRunFrame m_manualDispatchRunFrame;
    TManualDispatchGetNextCallback m_manualDispatchGetNextCallback;
    TManualDispatchFreeLastCallback m_manualDispatchFreeLastCallback;
    TGetInterface m_getUser;
    TGetInterface m_getFriends;
    TUserGetSteamId m_userGetSteamId;
    TUserBLoggedOn m_userBLoggedOn;
    TUserGetAuthTicketForWebApi m_userGetAuthTicketForWebApi;
    TUserGetAuthSessionTicket m_userGetAuthSessionTicket;
    TUserCancelAuthTicket m_userCancelAuthTicket;
    TUserBeginAuthSession m_userBeginAuthSession;
    TUserEndAuthSession m_userEndAuthSession;
    TFriendsGetPersonaName m_friendsGetPersonaName;

    void* m_user;
    void* m_friends;
    Int32 m_pipe;
    GUCEF::vector< SSteamTicketResponse > m_ready;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMRUNTIME_H ? */
