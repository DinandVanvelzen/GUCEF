/*
 *  drnchannelpluginEOSP2P: DRN channel plugin that moves DRN packets over the Epic Online Services P2P interface
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H
#include "pluginglueEOS_CEosP2PSocket.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H ? */

#include "drnchannelpluginEOSP2P_CEosP2PEndpointInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNPLUGIN {
namespace EOSP2P {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CEosP2PEndpointInfo::ChannelType = "eos-p2p";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosP2PEndpointInfo::CEosP2PEndpointInfo( void )
    : remoteUser()
    , localUser()
    , socketName()
    , reliability( PLUGINGLUE::EOS::EOSRELIABILITY_UNRELIABLE_UNORDERED )
    , maxPacketSize( PLUGINGLUE::EOS::EosP2PMaxPacketSize )
    , connectTimeoutInMs( DefaultConnectTimeoutInMs )
    , tickIntervalInMs( DefaultTickIntervalInMs )
    , hasRelayControl( false )
    , relayControl( PLUGINGLUE::EOS::EOSRELAY_ALLOW_RELAYS )
    , acceptAny( false )
    , allowedRemotes()
    , maxConnections( DefaultMaxConnections )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CEosP2PEndpointInfo::IsValidUserId( const CORE::CString& text )
{GUCEF_TRACE;

    UInt32 size = text.ByteSize();
    if ( size < 2 || size > PLUGINGLUE::EOS::EosP2PUserIdBufferSize )
        return false;
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<size-1; ++i )
    {
        char c = chars[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpoint
CEosP2PEndpointInfo::MakeEndpoint( const CORE::CString& productUserId, const CORE::CString& socketName )
{GUCEF_TRACE;

    return DRN::CDRNEndpoint( ChannelType, productUserId + "/" + socketName );
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PEndpointInfo::Parse( const DRN::CDRNEndpoint& endpoint, bool forListening, CORE::CString& reason )
{GUCEF_TRACE;

    if ( endpoint.GetChannelType() != ChannelType )
    {
        reason = "The endpoint is not an eos-p2p endpoint";
        return false;
    }

    CORE::CString user = endpoint.GetHost();
    CORE::CString path = endpoint.GetPath();
    if ( path.Length() < 2 )
    {
        reason = "The endpoint names no socket";
        return false;
    }
    socketName = path.SubstrFromRange( 1, path.Length() );
    if ( !PLUGINGLUE::EOS::CEosP2PSocket::IsValidSocketName( socketName ) )
    {
        reason = "The socket name has to be 1 to 32 letters, digits, '-' or '_'";
        return false;
    }

    const CORE::CValueList& params = endpoint.GetParams();
    localUser = params.GetValueAlways( "local" ).AsString( CORE::CString::Empty, true );
    if ( !localUser.IsNULLOrEmpty() && !IsValidUserId( localUser ) )
    {
        reason = "The local Product User Id is not valid";
        return false;
    }

    if ( forListening )
    {
        if ( "*" != user && !user.IsNULLOrEmpty() )
        {
            if ( !IsValidUserId( user ) )
            {
                reason = "The Product User Id to listen as is not valid";
                return false;
            }
            localUser = user;
        }
    }
    else
    {
        if ( !IsValidUserId( user ) )
        {
            reason = "The Product User Id of the peer is not valid";
            return false;
        }
        remoteUser = user;
        if ( !localUser.IsNULLOrEmpty() && localUser == remoteUser )
        {
            reason = "A user cannot connect to itself";
            return false;
        }
    }

    CORE::CString mode = params.GetValueAlways( "reliability" ).AsString( "unreliable", true ).Lowercase();
    if ( "reliable" == mode )
        reliability = PLUGINGLUE::EOS::EOSRELIABILITY_RELIABLE_UNORDERED;
    else
    if ( "ordered" == mode )
        reliability = PLUGINGLUE::EOS::EOSRELIABILITY_RELIABLE_ORDERED;
    else
    if ( "unreliable" == mode )
        reliability = PLUGINGLUE::EOS::EOSRELIABILITY_UNRELIABLE_UNORDERED;
    else
    {
        reason = "The reliability has to be unreliable, reliable or ordered";
        return false;
    }

    maxPacketSize = params.GetValueAlways( "maxPacketSize" ).AsUInt32( PLUGINGLUE::EOS::EosP2PMaxPacketSize, true );
    if ( maxPacketSize < 64 || maxPacketSize > PLUGINGLUE::EOS::EosP2PMaxPacketSize )
    {
        reason = "The packet size has to be 64 to 1170 bytes";
        return false;
    }
    connectTimeoutInMs = params.GetValueAlways( "connectTimeoutMs" ).AsUInt32( DefaultConnectTimeoutInMs, true );
    tickIntervalInMs = params.GetValueAlways( "tickIntervalInMs" ).AsUInt32( DefaultTickIntervalInMs, true );
    if ( 0 == tickIntervalInMs )
        tickIntervalInMs = DefaultTickIntervalInMs;

    CORE::CString relay = params.GetValueAlways( "relay" ).AsString( CORE::CString::Empty, true ).Lowercase();
    if ( !relay.IsNULLOrEmpty() )
    {
        hasRelayControl = true;
        if ( "allow" == relay )
            relayControl = PLUGINGLUE::EOS::EOSRELAY_ALLOW_RELAYS;
        else
        if ( "none" == relay )
            relayControl = PLUGINGLUE::EOS::EOSRELAY_NO_RELAYS;
        else
        if ( "force" == relay )
            relayControl = PLUGINGLUE::EOS::EOSRELAY_FORCE_RELAYS;
        else
        {
            reason = "The relay has to be allow, none or force";
            return false;
        }
    }

    if ( forListening )
    {
        acceptAny = params.GetValueAlways( "acceptAny" ).AsBool( false, true );
        maxConnections = params.GetValueAlways( "maxConnections" ).AsUInt32( DefaultMaxConnections, true );
        CORE::CString allow = params.GetValueAlways( "allow" ).AsString( CORE::CString::Empty, true );
        if ( !allow.IsNULLOrEmpty() )
        {
            CORE::CString::StringVector users = allow.ParseElements( ',', false );
            for ( size_t i=0; i<users.size(); ++i )
            {
                CORE::CString trimmed = users[ i ].Trim( true ).Trim( false );
                if ( trimmed.IsNULLOrEmpty() )
                    continue;
                if ( !IsValidUserId( trimmed ) )
                {
                    reason = "A Product User Id in the allow list is not valid";
                    return false;
                }
                allowedRemotes.insert( trimmed );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOSP2P */
}; /* namespace DRNPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
