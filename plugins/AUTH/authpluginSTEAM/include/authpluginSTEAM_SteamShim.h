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

#ifndef GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H
#define GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#include "authpluginSTEAM_macros.h"
#define GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_MACROS_H ? */

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

/** The longest ticket the Steam API gives: the size of the ticket array of GetTicketForWebApiResponse_t */
static const UInt32 SteamMaxTicketSize = 2560;

/** EResult k_EResultOK of the Steam API */
static const Int32 SteamResultOk = 1;

/** The ticket handle that means "no ticket" (k_HAuthTicketInvalid) */
static const UInt32 SteamInvalidTicketHandle = 0;

/** EBeginAuthSessionResult k_EBeginAuthSessionResultOK */
static const Int32 SteamBeginAuthSessionOk = 0;

/*-------------------------------------------------------------------------*/

/** The answer to a request for a ticket for the web api: GetTicketForWebApiResponse_t */
struct SSteamTicketResponse
{
    UInt32 ticketHandle;            /**< the handle the request returned */
    Int32 result;                   /**< EResult, SteamResultOk when the ticket is there */
    UInt32 ticketSize;              /**< bytes used in ticket */
    UInt8 ticket[ SteamMaxTicketSize ];

    SSteamTicketResponse( void ) : ticketHandle( 0 ), result( 0 ), ticketSize( 0 ) { ticket[ 0 ] = 0; }
};
typedef struct SSteamTicketResponse TSteamTicketResponse;

/*-------------------------------------------------------------------------*/

/**
 *  The function table the provider uses to reach Steam. The Steam client and its runtime library are not part of this
 *  repository and are not needed to build it: the real table is filled by CSteamRuntime from the runtime library found
 *  at run time (see FillSteamShimFromRuntime()), the tests fill it with a fake Steam.
 *
 *  Every function takes the context pointer of the table as its first argument. A function that a table does not have
 *  is NULL; the provider checks what it uses. None of the functions blocks.
 */
struct SSteamShim
{
    void* context;

    /** Initializes the Steam API (SteamAPI_InitFlat). False when the Steam client is not running or the API cannot start; errorText then says why. Repeated calls are fine */
    bool ( *init )( void* context, char* errorText, UInt32 errorTextSize );

    /** SteamAPI_Shutdown */
    void ( *shutdown )( void* context );

    bool ( *isInitialized )( void* context );

    /** ISteamUser::BLoggedOn: the Steam client is connected to the Steam servers */
    bool ( *isLoggedOn )( void* context );

    /** ISteamUser::GetSteamID, the SteamID64 of the user of the client */
    UInt64 ( *getSteamId )( void* context );

    /** ISteamFriends::GetPersonaName copied as UTF-8 into the buffer */
    bool ( *getPersonaName )( void* context, char* buffer, UInt32 bufferSize );

    /** ISteamUser::GetAuthTicketForWebApi. SteamInvalidTicketHandle on failure. The ticket comes later through pollTicketResponse() */
    UInt32 ( *requestWebApiTicket )( void* context, const char* identity );

    /** ISteamUser::GetAuthSessionTicket: the ticket is there when it returns. SteamInvalidTicketHandle on failure */
    UInt32 ( *requestSessionTicket )( void* context, UInt8* ticket, UInt32 ticketCapacity, UInt32* ticketSize );

    /** Lets Steam deliver the callbacks that are due (SteamAPI_ManualDispatch_RunFrame) */
    void ( *runFrame )( void* context );

    /** Takes the next GetTicketForWebApiResponse_t that was delivered. False when there is none */
    bool ( *pollTicketResponse )( void* context, SSteamTicketResponse* response );

    /** ISteamUser::CancelAuthTicket */
    void ( *cancelTicket )( void* context, UInt32 ticketHandle );

    /** ISteamUser::BeginAuthSession, EBeginAuthSessionResult; SteamBeginAuthSessionOk when the ticket is valid for that user */
    Int32 ( *beginAuthSession )( void* context, const void* ticket, Int32 ticketSize, UInt64 steamId );

    /** ISteamUser::EndAuthSession */
    void ( *endAuthSession )( void* context, UInt64 steamId );

    SSteamShim( void )
        : context( GUCEF_NULL ), init( GUCEF_NULL ), shutdown( GUCEF_NULL ), isInitialized( GUCEF_NULL ), isLoggedOn( GUCEF_NULL )
        , getSteamId( GUCEF_NULL ), getPersonaName( GUCEF_NULL ), requestWebApiTicket( GUCEF_NULL ), requestSessionTicket( GUCEF_NULL )
        , runFrame( GUCEF_NULL ), pollTicketResponse( GUCEF_NULL ), cancelTicket( GUCEF_NULL ), beginAuthSession( GUCEF_NULL ), endAuthSession( GUCEF_NULL )
    {}
};
typedef struct SSteamShim TSteamShim;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H ? */
