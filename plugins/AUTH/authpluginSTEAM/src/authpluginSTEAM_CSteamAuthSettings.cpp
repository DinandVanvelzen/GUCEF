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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "authpluginSTEAM_CSteamAuthSettings.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads a text setting: the node of the provider itself first, then the attributes of its Parameters child.
 *  The value that is there already is the default.
 */
static CORE::CString
ReadText( const CORE::CDataNode& config, const CORE::CDataNode* parameters, const char* name, const CORE::CString& current, bool resolveVariables )
{GUCEF_TRACE;

    CORE::CString value = current;
    if ( GUCEF_NULL != parameters )
        value = parameters->GetAttributeValueOrChildValueByName( name ).AsString( value, resolveVariables );
    value = config.GetAttributeValueOrChildValueByName( name ).AsString( value, resolveVariables );
    return value;
}

/*-------------------------------------------------------------------------*/

static UInt32
ReadUInt32( const CORE::CDataNode& config, const CORE::CDataNode* parameters, const char* name, UInt32 current )
{GUCEF_TRACE;

    UInt32 value = current;
    if ( GUCEF_NULL != parameters )
        value = parameters->GetAttributeValueOrChildValueByName( name ).AsUInt32( value, true );
    value = config.GetAttributeValueOrChildValueByName( name ).AsUInt32( value, true );
    return value;
}

/*-------------------------------------------------------------------------*/

static bool
ReadBool( const CORE::CDataNode& config, const CORE::CDataNode* parameters, const char* name, bool current )
{GUCEF_TRACE;

    bool value = current;
    if ( GUCEF_NULL != parameters )
        value = parameters->GetAttributeValueOrChildValueByName( name ).AsBool( value, true );
    value = config.GetAttributeValueOrChildValueByName( name ).AsBool( value, true );
    return value;
}

/*-------------------------------------------------------------------------*/

static bool
IsOneOf( const CORE::CString& value, const char* first, const char* second, const char* third )
{GUCEF_TRACE;

    return value == first || value == second || ( GUCEF_NULL != third && value == third );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSteamAuthSettings::CSteamAuthSettings( void )
    : CORE::CIConfigurable()
    , runtimeLibrary()
    , ticketIdentity()
    , ticketMode( "auto" )
    , callbackMode( "manual" )
    , ownsSteamApi( true )
    , ticketTimeoutInMs( DefaultTicketTimeoutInMs )
    , callbackIntervalInMs( DefaultCallbackIntervalInMs )
    , retryIntervalInMs( DefaultRetryIntervalInMs )
    , appId()
    , webApiKey()
    , webApiKeyPlacement( "query" )
    , webApiBaseUrl( "https://partner.steam-api.com" )
    , webApiTimeoutInMs( DefaultWebApiTimeoutInMs )
    , allowInsecureHttp( false )
    , verifyMode( "webapi" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSteamAuthSettings::CSteamAuthSettings( const CSteamAuthSettings& src )
    : CORE::CIConfigurable( src )
    , runtimeLibrary( src.runtimeLibrary )
    , ticketIdentity( src.ticketIdentity )
    , ticketMode( src.ticketMode )
    , callbackMode( src.callbackMode )
    , ownsSteamApi( src.ownsSteamApi )
    , ticketTimeoutInMs( src.ticketTimeoutInMs )
    , callbackIntervalInMs( src.callbackIntervalInMs )
    , retryIntervalInMs( src.retryIntervalInMs )
    , appId( src.appId )
    , webApiKey( src.webApiKey )
    , webApiKeyPlacement( src.webApiKeyPlacement )
    , webApiBaseUrl( src.webApiBaseUrl )
    , webApiTimeoutInMs( src.webApiTimeoutInMs )
    , allowInsecureHttp( src.allowInsecureHttp )
    , verifyMode( src.verifyMode )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSteamAuthSettings::~CSteamAuthSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSteamAuthSettings&
CSteamAuthSettings::operator=( const CSteamAuthSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        runtimeLibrary = src.runtimeLibrary;
        ticketIdentity = src.ticketIdentity;
        ticketMode = src.ticketMode;
        callbackMode = src.callbackMode;
        ownsSteamApi = src.ownsSteamApi;
        ticketTimeoutInMs = src.ticketTimeoutInMs;
        callbackIntervalInMs = src.callbackIntervalInMs;
        retryIntervalInMs = src.retryIntervalInMs;
        appId = src.appId;
        webApiKey = src.webApiKey;
        webApiKeyPlacement = src.webApiKeyPlacement;
        webApiBaseUrl = src.webApiBaseUrl;
        webApiTimeoutInMs = src.webApiTimeoutInMs;
        allowInsecureHttp = src.allowInsecureHttp;
        verifyMode = src.verifyMode;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthSettings::operator==( const CSteamAuthSettings& other ) const
{GUCEF_TRACE;

    return runtimeLibrary == other.runtimeLibrary && ticketIdentity == other.ticketIdentity && ticketMode == other.ticketMode &&
           callbackMode == other.callbackMode && ownsSteamApi == other.ownsSteamApi && ticketTimeoutInMs == other.ticketTimeoutInMs &&
           callbackIntervalInMs == other.callbackIntervalInMs && retryIntervalInMs == other.retryIntervalInMs && appId == other.appId &&
           webApiKey == other.webApiKey && webApiKeyPlacement == other.webApiKeyPlacement && webApiBaseUrl == other.webApiBaseUrl && webApiTimeoutInMs == other.webApiTimeoutInMs &&
           allowInsecureHttp == other.allowInsecureHttp && verifyMode == other.verifyMode;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthSettings::IsWellFormed( void ) const
{GUCEF_TRACE;

    return IsOneOf( ticketMode, "auto", "webapi", "session" ) &&
           IsOneOf( callbackMode, "manual", "external", GUCEF_NULL ) &&
           IsOneOf( verifyMode, "webapi", "peer", GUCEF_NULL ) &&
           IsOneOf( webApiKeyPlacement, "query", "header", GUCEF_NULL ) &&
           callbackIntervalInMs > 0;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    CORE::CDataNode* parameters = config.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == parameters )
        return false;

    parameters->SetAttribute( "runtimeLibrary", runtimeLibrary );
    parameters->SetAttribute( "ticketIdentity", ticketIdentity );
    parameters->SetAttribute( "ticketMode", ticketMode );
    parameters->SetAttribute( "callbackMode", callbackMode );
    parameters->SetAttribute( "ownsSteamApi", ownsSteamApi );
    parameters->SetAttribute( "ticketTimeoutInMs", ticketTimeoutInMs );
    parameters->SetAttribute( "callbackIntervalInMs", callbackIntervalInMs );
    parameters->SetAttribute( "retryIntervalInMs", retryIntervalInMs );
    parameters->SetAttribute( "appId", appId );
    if ( !webApiKey.IsEmpty() )
        parameters->SetAttribute( "webApiKey", webApiKey.Reveal() );
    parameters->SetAttribute( "webApiKeyPlacement", webApiKeyPlacement );
    parameters->SetAttribute( "webApiBaseUrl", webApiBaseUrl );
    parameters->SetAttribute( "webApiTimeoutInMs", webApiTimeoutInMs );
    parameters->SetAttribute( "allowInsecureHttp", allowInsecureHttp );
    parameters->SetAttribute( "verifyMode", verifyMode );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* parameters = config.FindChild( "Parameters" );

    CSteamAuthSettings loaded( *this );
    loaded.runtimeLibrary = ReadText( config, parameters, "runtimeLibrary", runtimeLibrary, true );
    loaded.ticketIdentity = ReadText( config, parameters, "ticketIdentity", ticketIdentity, false );
    loaded.ticketMode = ReadText( config, parameters, "ticketMode", ticketMode, false );
    loaded.callbackMode = ReadText( config, parameters, "callbackMode", callbackMode, false );
    loaded.ownsSteamApi = ReadBool( config, parameters, "ownsSteamApi", ownsSteamApi );
    loaded.ticketTimeoutInMs = ReadUInt32( config, parameters, "ticketTimeoutInMs", ticketTimeoutInMs );
    loaded.callbackIntervalInMs = ReadUInt32( config, parameters, "callbackIntervalInMs", callbackIntervalInMs );
    loaded.retryIntervalInMs = ReadUInt32( config, parameters, "retryIntervalInMs", retryIntervalInMs );
    loaded.appId = ReadText( config, parameters, "appId", appId, false );
    CORE::CString key = ReadText( config, parameters, "webApiKey", CORE::CString::Empty, false );
    if ( !key.IsNULLOrEmpty() )
        loaded.webApiKey.Set( key );
    loaded.webApiKeyPlacement = ReadText( config, parameters, "webApiKeyPlacement", webApiKeyPlacement, false );
    loaded.webApiBaseUrl = ReadText( config, parameters, "webApiBaseUrl", webApiBaseUrl, false );
    loaded.webApiTimeoutInMs = ReadUInt32( config, parameters, "webApiTimeoutInMs", webApiTimeoutInMs );
    loaded.allowInsecureHttp = ReadBool( config, parameters, "allowInsecureHttp", allowInsecureHttp );
    loaded.verifyMode = ReadText( config, parameters, "verifyMode", verifyMode, false );

    // A value that this plugin does not know is refused as a whole, so that a typo does not silently change the behaviour
    if ( !loaded.IsWellFormed() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthSettings: The settings have a mode that is not known (ticketMode " << loaded.ticketMode <<
                           ", callbackMode " << loaded.callbackMode << ", verifyMode " << loaded.verifyMode << ", webApiKeyPlacement " << loaded.webApiKeyPlacement << "), they are not used" );
        return false;
    }

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSteamAuthSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::STEAM::CSteamAuthSettings";
    return classTypeName;
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
