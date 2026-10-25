/*
 *  gucefAUTH_PluginsTestApp: Application for testing the gucefAUTH plugins
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKESTEAM_H
#define GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKESTEAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H
#include "authpluginSTEAM_SteamShim.h"
#define GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A Steam of the test, behind the function table of the provider: the Steam client may or may not run, tickets are
 *  created and answered on the frames the test lets pass, and everything that is asked is counted.
 */
class CFakeSteam
{
    public:

    bool initOk;
    GUCEF::CORE::CString initError;
    bool loggedOn;
    GUCEF::UInt64 steamId;
    GUCEF::CORE::CString persona;
    bool hasWebApi;
    bool hasSession;
    bool hasPeer;
    bool answerTickets;                 /**< a ticket is answered on the frames after the request */
    GUCEF::Int32 answerResult;          /**< EResult of the answer */
    GUCEF::UInt32 answerAfterFrames;
    bool sessionTicketFails;
    GUCEF::Int32 beginResult;

    bool initialized;
    GUCEF::UInt32 initCalls;
    GUCEF::UInt32 shutdownCalls;
    GUCEF::UInt32 frames;
    GUCEF::UInt32 nextHandle;
    GUCEF::vector< GUCEF::CORE::CString > identities;
    GUCEF::vector< GUCEF::UInt32 > cancelled;
    GUCEF::vector< GUCEF::UInt64 > ended;
    GUCEF::vector< GUCEF::UInt64 > begun;
    GUCEF::vector< GUCEF::UInt8 > lastBeginTicket;

    CFakeSteam( void )
        : initOk( true ), initError(), loggedOn( true ), steamId( 76561198000000001ULL ), persona( "Fake Persona" ), hasWebApi( true ), hasSession( true )
        , hasPeer( true ), answerTickets( true ), answerResult( GUCEF::AUTHPLUGIN::STEAM::SteamResultOk ), answerAfterFrames( 1 ), sessionTicketFails( false )
        , beginResult( GUCEF::AUTHPLUGIN::STEAM::SteamBeginAuthSessionOk ), initialized( false ), initCalls( 0 ), shutdownCalls( 0 ), frames( 0 ), nextHandle( 100 )
        , identities(), cancelled(), ended(), begun(), lastBeginTicket(), m_pending(), m_ready()
    {}

    /** The bytes of the ticket of a handle, and the hex they come out as */
    static GUCEF::UInt32 TicketBytes( GUCEF::UInt32 handle, GUCEF::UInt8* out )
    {
        const GUCEF::UInt8 bytes[ 8 ] = { 0x14, 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x01, (GUCEF::UInt8) handle };
        memcpy( out, bytes, sizeof( bytes ) );
        return sizeof( bytes );
    }

    static GUCEF::CORE::CString TicketHex( GUCEF::UInt32 handle )
    {
        GUCEF::UInt8 bytes[ 8 ];
        GUCEF::UInt32 size = TicketBytes( handle, bytes );
        return GUCEF::AUTH::CAuthCrypto::ToHex( bytes, size );
    }

    bool WasCancelled( GUCEF::UInt32 handle ) const
    {
        for ( size_t i=0; i<cancelled.size(); ++i )
            if ( cancelled[ i ] == handle )
                return true;
        return false;
    }

    GUCEF::AUTHPLUGIN::STEAM::TSteamShim MakeShim( void )
    {
        using namespace GUCEF::AUTHPLUGIN::STEAM;
        TSteamShim shim;
        shim.context = this;
        shim.init = &CFakeSteam::Init;
        shim.shutdown = &CFakeSteam::Shutdown;
        shim.isInitialized = &CFakeSteam::IsInitialized;
        shim.isLoggedOn = &CFakeSteam::IsLoggedOn;
        shim.getSteamId = &CFakeSteam::GetSteamId;
        shim.getPersonaName = &CFakeSteam::GetPersonaName;
        shim.requestWebApiTicket = hasWebApi ? &CFakeSteam::RequestWebApiTicket : GUCEF_NULL;
        shim.requestSessionTicket = hasSession ? &CFakeSteam::RequestSessionTicket : GUCEF_NULL;
        shim.runFrame = &CFakeSteam::RunFrame;
        shim.pollTicketResponse = &CFakeSteam::PollTicketResponse;
        shim.cancelTicket = &CFakeSteam::CancelTicket;
        shim.beginAuthSession = hasPeer ? &CFakeSteam::BeginAuthSession : GUCEF_NULL;
        shim.endAuthSession = hasPeer ? &CFakeSteam::EndAuthSession : GUCEF_NULL;
        return shim;
    }

    private:

    struct SPendingTicket
    {
        GUCEF::UInt32 handle;
        GUCEF::UInt32 framesLeft;
    };

    static CFakeSteam* Self( void* context ) { return static_cast< CFakeSteam* >( context ); }

    static bool Init( void* context, char* errorText, GUCEF::UInt32 errorTextSize )
    {
        CFakeSteam* self = Self( context );
        ++self->initCalls;
        if ( !self->initOk )
        {
            const char* text = self->initError.IsNULLOrEmpty() ? "The Steam client is not running" : self->initError.C_String();
            strncpy( errorText, text, errorTextSize - 1 );
            errorText[ errorTextSize - 1 ] = '\0';
            return false;
        }
        self->initialized = true;
        return true;
    }

    static void Shutdown( void* context ) { CFakeSteam* self = Self( context ); ++self->shutdownCalls; self->initialized = false; }
    static bool IsInitialized( void* context ) { return Self( context )->initialized; }
    static bool IsLoggedOn( void* context ) { return Self( context )->loggedOn; }
    static GUCEF::UInt64 GetSteamId( void* context ) { return Self( context )->steamId; }

    static bool GetPersonaName( void* context, char* buffer, GUCEF::UInt32 bufferSize )
    {
        CFakeSteam* self = Self( context );
        strncpy( buffer, self->persona.C_String(), bufferSize - 1 );
        buffer[ bufferSize - 1 ] = '\0';
        return true;
    }

    static GUCEF::UInt32 RequestWebApiTicket( void* context, const char* identity )
    {
        CFakeSteam* self = Self( context );
        self->identities.push_back( identity );
        SPendingTicket pending;
        pending.handle = self->nextHandle++;
        pending.framesLeft = self->answerAfterFrames;
        self->m_pending.push_back( pending );
        return pending.handle;
    }

    static GUCEF::UInt32 RequestSessionTicket( void* context, GUCEF::UInt8* ticket, GUCEF::UInt32 ticketCapacity, GUCEF::UInt32* ticketSize )
    {
        CFakeSteam* self = Self( context );
        if ( self->sessionTicketFails )
            return GUCEF::AUTHPLUGIN::STEAM::SteamInvalidTicketHandle;
        GUCEF::UInt32 handle = self->nextHandle++;
        *ticketSize = TicketBytes( handle, ticket );
        return handle;
    }

    static void RunFrame( void* context )
    {
        CFakeSteam* self = Self( context );
        ++self->frames;
        if ( !self->answerTickets )
            return;
        for ( size_t i=0; i<self->m_pending.size(); )
        {
            if ( self->m_pending[ i ].framesLeft > 0 )
                --self->m_pending[ i ].framesLeft;
            if ( 0 == self->m_pending[ i ].framesLeft )
            {
                GUCEF::AUTHPLUGIN::STEAM::SSteamTicketResponse response;
                response.ticketHandle = self->m_pending[ i ].handle;
                response.result = self->answerResult;
                if ( GUCEF::AUTHPLUGIN::STEAM::SteamResultOk == self->answerResult )
                    response.ticketSize = TicketBytes( response.ticketHandle, response.ticket );
                self->m_ready.push_back( response );
                self->m_pending.erase( self->m_pending.begin() + i );
            }
            else
            {
                ++i;
            }
        }
    }

    static bool PollTicketResponse( void* context, GUCEF::AUTHPLUGIN::STEAM::SSteamTicketResponse* response )
    {
        CFakeSteam* self = Self( context );
        if ( self->m_ready.empty() )
            return false;
        *response = self->m_ready.front();
        self->m_ready.erase( self->m_ready.begin() );
        return true;
    }

    static void CancelTicket( void* context, GUCEF::UInt32 handle ) { Self( context )->cancelled.push_back( handle ); }

    static GUCEF::Int32 BeginAuthSession( void* context, const void* ticket, GUCEF::Int32 ticketSize, GUCEF::UInt64 id )
    {
        CFakeSteam* self = Self( context );
        self->begun.push_back( id );
        const GUCEF::UInt8* bytes = static_cast< const GUCEF::UInt8* >( ticket );
        self->lastBeginTicket.assign( bytes, bytes + ticketSize );
        return self->beginResult;
    }

    static void EndAuthSession( void* context, GUCEF::UInt64 id ) { Self( context )->ended.push_back( id ); }

    private:

    GUCEF::vector< SPendingTicket > m_pending;
    GUCEF::vector< GUCEF::AUTHPLUGIN::STEAM::SSteamTicketResponse > m_ready;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKESTEAM_H ? */
