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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_AUTH_CAUTHURL_H
#include "gucefAUTH_CAuthUrl.h"
#define GUCEF_AUTH_CAUTHURL_H
#endif /* GUCEF_AUTH_CAUTHURL_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#include "authpluginSTEAM_CSteamOpenId2Profile.h"

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

const CORE::CString CSteamOpenId2Profile::ProviderName = "steam";
const CORE::CString CSteamOpenId2Profile::AccountNamespace = "steam";
const CORE::CString CSteamOpenId2Profile::Endpoint = "https://steamcommunity.com/openid/login";
const UInt64 CSteamOpenId2Profile::HintLifeTimeInMs;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSteamOpenId2Profile::CSteamOpenId2Profile( void )
    : AUTH::IOpenId2ProviderProfile()
    , m_lock()
    , m_detector()
    , m_info()
    , m_infoTimeInMs( 0 )
    , m_infoValid( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSteamOpenId2Profile::~CSteamOpenId2Profile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

AUTH::CAuthProviderDescriptor
CSteamOpenId2Profile::MakeDescriptor( void )
{GUCEF_TRACE;

    AUTH::CAuthProviderDescriptor d;
    d.name = ProviderName;
    d.displayName = "Steam";
    d.accountProviderId = AccountNamespace;
    d.protocol = AUTH::AUTHPROTOCOL_OPENID2;
    d.idScope = AUTH::AUTHIDSCOPE_GLOBAL;
    d.capabilities = AUTH::AUTHCAP_BROWSER | AUTH::AUTHCAP_SERVER_VERIFIABLE;
    d.authorizationEndpoint = Endpoint;
    d.subjectClaim = "steamid";
    d.publicClient = true;
    d.usesPkce = false;
    d.formerNames.push_back( "steam-openid" );
    d.notes = "OpenID 2.0, not OAuth: no client id and no secret. realm and return_to are the loopback address of the app, which Steam shows to the player as the site that asks for the sign in; the parameter relayUrl (https, a static page on a domain of the developer) makes the realm and return_to that page instead, and the page sends the browser on to the loopback address. The positive assertion is checked by posting it back to the endpoint with openid.mode=check_authentication, "
              "and the steamid is the last path segment of the claimed_id https://steamcommunity.com/openid/id/<SteamID64>. Display name and avatar need the Steam Web API with a publisher key, so only a server has them. "
              "A running Steam client gives a login without the browser through the Steamworks SDK instead, see CSteamAuthProvider. This provider was called steam-openid before the two logins became one provider.";
    return d;
}

/*-------------------------------------------------------------------------*/

bool
CSteamOpenId2Profile::ExtractSteamId( const CORE::CString& endpoint, const CORE::CString& claimedId, CORE::CString& steamId )
{GUCEF_TRACE;

    steamId.Clear();
    CORE::CString origin = AUTH::CAuthUrl::GetOrigin( endpoint );
    if ( origin.IsNULLOrEmpty() )
        return false;
    CORE::CString prefix = origin + "/openid/id/";
    UInt32 prefixSize = AUTH::GetByteLength( prefix );
    UInt32 size = AUTH::GetByteLength( claimedId );
    if ( size <= prefixSize || size > prefixSize + 20 || 0 != memcmp( claimedId.C_String(), prefix.C_String(), prefixSize ) )
        return false;

    // Digits only, no leading zero, within 64 bits
    const char* digits = claimedId.C_String() + prefixSize;
    UInt32 count = size - prefixSize;
    if ( '0' == digits[ 0 ] )
        return false;
    UInt64 value = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( digits[ i ] < '0' || digits[ i ] > '9' )
            return false;
        UInt64 digit = (UInt64) ( digits[ i ] - '0' );
        if ( value > ( GUCEF_UINT64MAX - digit ) / 10 )
            return false;
        value = value * 10 + digit;
    }
    steamId = AUTH::MakeString( digits, count );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSteamOpenId2Profile::SetHostView( const ISteamHostView* view )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_detector.SetHostView( view );
    m_infoValid = false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSteamOpenId2Profile::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

bool
CSteamOpenId2Profile::ExtractSubject( const CORE::CString& endpoint ,
                                      const CORE::CString& claimedId ,
                                      const CORE::CString& identity  ,
                                      CORE::CString& subject         ) const
{GUCEF_TRACE;

    // Steam gives one identity: the identity of the assertion is the claimed id
    if ( identity != claimedId )
    {
        subject.Clear();
        return false;
    }
    return ExtractSteamId( endpoint, claimedId, subject );
}

/*-------------------------------------------------------------------------*/

void
CSteamOpenId2Profile::ForgetDetectedAccountHint( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_infoValid = false;
}

/*-------------------------------------------------------------------------*/

bool
CSteamOpenId2Profile::GetDetectedAccountHint( CORE::CString& displayName, bool& isRunning ) const
{GUCEF_TRACE;

    displayName.Clear();
    isRunning = false;

    MT::CScopeMutex lock( m_lock );
    const UInt64 now = AUTH::GetMonotonicTimeInMs();
    if ( !m_infoValid || now - m_infoTimeInMs >= HintLifeTimeInMs )
    {
        m_detector.Detect( m_info );
        m_infoTimeInMs = now;
        m_infoValid = true;
    }
    if ( !m_info.installed )
        return false;
    isRunning = m_info.running;
    if ( m_info.hasUser )
        displayName = m_info.user.personaName.IsNULLOrEmpty() ? m_info.user.accountName : m_info.user.personaName;
    return true;
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
