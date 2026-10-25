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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "lobbyservicepluginEOS_CEosLobbyServiceSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosLobbyServiceSettings::CEosLobbyServiceSettings( void )
    : CORE::CIConfigurable()
    , bucketId()
    , permission()
    , presenceEnabled( false )
    , mirrorSettings()
    , spectatorSlots( 8 )
    , secondInMs( 1000 )
    , requestTimeoutInMs( 15000 )
    , joinTimeoutInMs( 15000 )
    , joinGraceInMs( 5000 )
    , departureGraceInMs( 2000 )
    , launchTimeoutInMs( 15000 )
    , tokenLifetimeInSecs( 3600 )
    , updatesPerSecond( 4 )
    , updateBurst( 8 )
    , memberRequestsPerSecond( 5 )
    , memberRequestBurst( 10 )
    , tickIntervalInMs( 20 )
    , browseMaxResults( 50 )
    , relayControl( PLUGINGLUE::EOS::EOSRELAY_ALLOW_RELAYS )
    , allowUnsignedLaunch( false )
    , limits()
    , rules()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosLobbyServiceSettings::CEosLobbyServiceSettings( const CEosLobbyServiceSettings& src )
    : CORE::CIConfigurable( src )
    , bucketId( src.bucketId )
    , permission( src.permission )
    , presenceEnabled( src.presenceEnabled )
    , mirrorSettings( src.mirrorSettings )
    , spectatorSlots( src.spectatorSlots )
    , secondInMs( src.secondInMs )
    , requestTimeoutInMs( src.requestTimeoutInMs )
    , joinTimeoutInMs( src.joinTimeoutInMs )
    , joinGraceInMs( src.joinGraceInMs )
    , departureGraceInMs( src.departureGraceInMs )
    , launchTimeoutInMs( src.launchTimeoutInMs )
    , tokenLifetimeInSecs( src.tokenLifetimeInSecs )
    , updatesPerSecond( src.updatesPerSecond )
    , updateBurst( src.updateBurst )
    , memberRequestsPerSecond( src.memberRequestsPerSecond )
    , memberRequestBurst( src.memberRequestBurst )
    , tickIntervalInMs( src.tickIntervalInMs )
    , browseMaxResults( src.browseMaxResults )
    , relayControl( src.relayControl )
    , allowUnsignedLaunch( src.allowUnsignedLaunch )
    , limits( src.limits )
    , rules( src.rules )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosLobbyServiceSettings::~CEosLobbyServiceSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosLobbyServiceSettings&
CEosLobbyServiceSettings::operator=( const CEosLobbyServiceSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        bucketId = src.bucketId;
        permission = src.permission;
        presenceEnabled = src.presenceEnabled;
        mirrorSettings = src.mirrorSettings;
        spectatorSlots = src.spectatorSlots;
        secondInMs = src.secondInMs;
        requestTimeoutInMs = src.requestTimeoutInMs;
        joinTimeoutInMs = src.joinTimeoutInMs;
        joinGraceInMs = src.joinGraceInMs;
        departureGraceInMs = src.departureGraceInMs;
        launchTimeoutInMs = src.launchTimeoutInMs;
        tokenLifetimeInSecs = src.tokenLifetimeInSecs;
        updatesPerSecond = src.updatesPerSecond;
        updateBurst = src.updateBurst;
        memberRequestsPerSecond = src.memberRequestsPerSecond;
        memberRequestBurst = src.memberRequestBurst;
        tickIntervalInMs = src.tickIntervalInMs;
        browseMaxResults = src.browseMaxResults;
        relayControl = src.relayControl;
        allowUnsignedLaunch = src.allowUnsignedLaunch;
        limits = src.limits;
        rules = src.rules;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosLobbyServiceSettings::GetBucketId( const CORE::CString& gameId ) const
{GUCEF_TRACE;

    if ( !bucketId.IsNULLOrEmpty() )
        return bucketId;

    // The id of the game as far as a bucket id takes it
    CORE::CString clean = "gucef";
    if ( !gameId.IsNULLOrEmpty() )
    {
        clean += '-';
        const char* chars = gameId.C_String();
        for ( UInt32 i=0; i<gameId.ByteSize()-1 && i<48; ++i )
        {
            char c = chars[ i ];
            bool plain = ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || '-' == c || '_' == c;
            clean += plain ? c : '_';
        }
    }
    return clean;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyServiceSettings::IsMirrored( const CORE::CString& key ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<mirrorSettings.size(); ++i )
        if ( mirrorSettings[ i ] == key )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyServiceSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "bucketId", bucketId );
    config.SetAttribute( "permission", permission );
    config.SetAttribute( "presenceEnabled", presenceEnabled );
    CORE::CString mirrors;
    for ( size_t i=0; i<mirrorSettings.size(); ++i )
        mirrors += ( 0 == i ? "" : "," ) + mirrorSettings[ i ];
    config.SetAttribute( "mirrorSettings", mirrors );
    config.SetAttribute( "spectatorSlots", spectatorSlots );
    config.SetAttribute( "secondInMs", secondInMs );
    config.SetAttribute( "requestTimeoutInMs", requestTimeoutInMs );
    config.SetAttribute( "joinTimeoutInMs", joinTimeoutInMs );
    config.SetAttribute( "joinGraceInMs", joinGraceInMs );
    config.SetAttribute( "departureGraceInMs", departureGraceInMs );
    config.SetAttribute( "launchTimeoutInMs", launchTimeoutInMs );
    config.SetAttribute( "tokenLifetimeInSecs", tokenLifetimeInSecs );
    config.SetAttribute( "updatesPerSecond", updatesPerSecond );
    config.SetAttribute( "updateBurst", updateBurst );
    config.SetAttribute( "memberRequestsPerSecond", memberRequestsPerSecond );
    config.SetAttribute( "memberRequestBurst", memberRequestBurst );
    config.SetAttribute( "tickIntervalInMs", tickIntervalInMs );
    config.SetAttribute( "browseMaxResults", browseMaxResults );
    config.SetAttribute( "relayControl", PLUGINGLUE::EOS::EOSRELAY_NO_RELAYS == relayControl ? "none" : ( PLUGINGLUE::EOS::EOSRELAY_FORCE_RELAYS == relayControl ? "force" : "allow" ) );
    config.SetAttribute( "allowUnsignedLaunch", allowUnsignedLaunch );
    CORE::CDataNode* limitsNode = config.AddChild( "Limits", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* rulesNode = config.AddChild( "Rules", GUCEF_DATATYPE_OBJECT );
    return GUCEF_NULL != limitsNode && GUCEF_NULL != rulesNode && limits.SaveConfig( *limitsNode ) && rules.SaveConfig( *rulesNode );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyServiceSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CEosLobbyServiceSettings loaded( *this );
    loaded.bucketId = config.GetAttributeValueOrChildValueByName( "bucketId" ).AsString( bucketId, true );
    loaded.permission = config.GetAttributeValueOrChildValueByName( "permission" ).AsString( permission, true ).Lowercase();
    if ( !loaded.permission.IsNULLOrEmpty() && "public" != loaded.permission && "presence" != loaded.permission && "invite" != loaded.permission )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyServiceSettings: The permission has to be public, presence or invite, the settings are not used" );
        return false;
    }
    loaded.presenceEnabled = config.GetAttributeValueOrChildValueByName( "presenceEnabled" ).AsBool( presenceEnabled, true );
    CORE::CString mirrors = config.GetAttributeValueOrChildValueByName( "mirrorSettings" ).AsString( CORE::CString::Empty, true );
    if ( !mirrors.IsNULLOrEmpty() )
    {
        loaded.mirrorSettings.clear();
        CORE::CString::StringVector keys = mirrors.ParseElements( ',', false );
        for ( size_t i=0; i<keys.size(); ++i )
        {
            CORE::CString key = keys[ i ].Trim( true ).Trim( false );
            if ( !key.IsNULLOrEmpty() )
                loaded.mirrorSettings.push_back( key );
        }
    }
    loaded.spectatorSlots = config.GetAttributeValueOrChildValueByName( "spectatorSlots" ).AsUInt32( spectatorSlots, true );
    loaded.secondInMs = config.GetAttributeValueOrChildValueByName( "secondInMs" ).AsUInt32( secondInMs, true );
    loaded.requestTimeoutInMs = config.GetAttributeValueOrChildValueByName( "requestTimeoutInMs" ).AsUInt32( requestTimeoutInMs, true );
    loaded.joinTimeoutInMs = config.GetAttributeValueOrChildValueByName( "joinTimeoutInMs" ).AsUInt32( joinTimeoutInMs, true );
    loaded.joinGraceInMs = config.GetAttributeValueOrChildValueByName( "joinGraceInMs" ).AsUInt32( joinGraceInMs, true );
    loaded.departureGraceInMs = config.GetAttributeValueOrChildValueByName( "departureGraceInMs" ).AsUInt32( departureGraceInMs, true );
    loaded.launchTimeoutInMs = config.GetAttributeValueOrChildValueByName( "launchTimeoutInMs" ).AsUInt32( launchTimeoutInMs, true );
    loaded.tokenLifetimeInSecs = config.GetAttributeValueOrChildValueByName( "tokenLifetimeInSecs" ).AsUInt32( tokenLifetimeInSecs, true );
    loaded.updatesPerSecond = config.GetAttributeValueOrChildValueByName( "updatesPerSecond" ).AsUInt32( updatesPerSecond, true );
    loaded.updateBurst = config.GetAttributeValueOrChildValueByName( "updateBurst" ).AsUInt32( updateBurst, true );
    loaded.memberRequestsPerSecond = config.GetAttributeValueOrChildValueByName( "memberRequestsPerSecond" ).AsUInt32( memberRequestsPerSecond, true );
    loaded.memberRequestBurst = config.GetAttributeValueOrChildValueByName( "memberRequestBurst" ).AsUInt32( memberRequestBurst, true );
    loaded.tickIntervalInMs = config.GetAttributeValueOrChildValueByName( "tickIntervalInMs" ).AsUInt32( tickIntervalInMs, true );
    loaded.browseMaxResults = config.GetAttributeValueOrChildValueByName( "browseMaxResults" ).AsUInt32( browseMaxResults, true );
    CORE::CString relay = config.GetAttributeValueOrChildValueByName( "relayControl" ).AsString( CORE::CString::Empty, true ).Lowercase();
    if ( "none" == relay )
        loaded.relayControl = PLUGINGLUE::EOS::EOSRELAY_NO_RELAYS;
    else
    if ( "force" == relay )
        loaded.relayControl = PLUGINGLUE::EOS::EOSRELAY_FORCE_RELAYS;
    else
    if ( "allow" == relay )
        loaded.relayControl = PLUGINGLUE::EOS::EOSRELAY_ALLOW_RELAYS;
    loaded.allowUnsignedLaunch = config.GetAttributeValueOrChildValueByName( "allowUnsignedLaunch" ).AsBool( allowUnsignedLaunch, true );

    const CORE::CDataNode* limitsNode = config.FindChild( "Limits" );
    if ( GUCEF_NULL != limitsNode )
    {
        if ( !loaded.limits.LoadConfig( *limitsNode ) )
            return false;
    }
    else
    {
        // The limits may also be given with the other settings
        if ( !loaded.limits.LoadConfig( config ) )
            return false;
    }

    const CORE::CDataNode* rulesNode = config.FindChild( "Rules" );
    if ( GUCEF_NULL != rulesNode && !loaded.rules.LoadConfig( *rulesNode ) )
        return false;

    if ( 0 == loaded.tickIntervalInMs || 0 == loaded.secondInMs || 0 == loaded.requestTimeoutInMs || 0 == loaded.browseMaxResults )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyServiceSettings: The intervals and the number of results have to be more than 0, the settings are not used" );
        return false;
    }
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosLobbyServiceSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::EOS::CEosLobbyServiceSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
