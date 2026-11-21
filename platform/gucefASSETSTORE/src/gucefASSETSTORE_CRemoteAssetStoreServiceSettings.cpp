/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#include "gucefASSETSTORE_CAssetStoreRemoteContracts.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H ? */

#include "gucefASSETSTORE_CRemoteAssetStoreServiceSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CRemoteAssetStoreServiceSettings::DefaultServiceType = "_gucef-assetstore._tcp";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const KnownKeys[] = { "endpoint", "credentialsDirectory", "credentialsKeyFile", "credentialsSlot", "clientKind", "clientVersion",
                                  "bulkChannels", "connectTimeoutMs", "eventWaitMs", "linkTimeoutMs", "echoIntervalMs", "reconnectBaseMs",
                                  "reconnectMaxMs", "autoReconnect", "lanDiscovery", "lanServiceType", "lanFindTimeoutMs",
                                  "mode" /* the viewer's choice of the implementation, "local" or "remote" */ };

bool
IsKnownKey( const CORE::CString& key )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<sizeof( KnownKeys ) / sizeof( KnownKeys[ 0 ] ); ++i )
        if ( key == KnownKeys[ i ] )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteAssetStoreServiceSettings::CRemoteAssetStoreServiceSettings( void )
    : CORE::CIConfigurable()
    , endpoint()
    , credentialsDirectory()
    , credentialsKeyFile()
    , credentialsSlot( "assetstore" )
    , clientKind( "viewer" )
    , clientVersion( "0.1.0" )
    , bulkChannels( 1 )
    , connectTimeoutMs( 20000 )
    , eventWaitMs( 2000 )
    , linkTimeoutMs( 15000 )
    , echoIntervalMs( 1000 )
    , reconnectBaseMs( 500 )
    , reconnectMaxMs( 30000 )
    , autoReconnect( true )
    , lanDiscovery( true )
    , lanServiceType( DefaultServiceType )
    , lanFindTimeoutMs( 3000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetStoreServiceSettings::GetHostAndPort( void ) const
{GUCEF_TRACE;

    CORE::CString hostAndPort = endpoint.Trim( true );
    const Int32 scheme = hostAndPort.HasSubstr( "://" );
    if ( scheme >= 0 )
        hostAndPort = hostAndPort.CutChars( (UInt32) scheme + 3, true );
    while ( !hostAndPort.IsNULLOrEmpty() && '/' == hostAndPort[ hostAndPort.Length() - 1 ] )
        hostAndPort = hostAndPort.CutChars( 1, false );
    return hostAndPort;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreServiceSettings::GetConnectionSettings( DRNUTILS::CRemoteServiceConnectionSettings& settings ) const
{GUCEF_TRACE;

    settings.clientKind = clientKind;
    settings.clientVersion = clientVersion;
    settings.wantedContracts.clear();
    settings.wantedContracts.push_back( CAssetStoreRemoteContracts::ControlContractName );
    settings.wantedContracts.push_back( CAssetStoreRemoteContracts::FilesContractName );
    settings.bulkContracts.clear();
    settings.bulkContracts.push_back( CAssetStoreRemoteContracts::FilesContractName );
    settings.bulkChannels = bulkChannels > 2 ? 2 : bulkChannels;
    settings.hostOverride = GetHostAndPort();
    settings.connectTimeoutMs = connectTimeoutMs;
    settings.linkTimeoutMs = linkTimeoutMs;
    settings.echoIntervalMs = echoIntervalMs;
    settings.reconnectBaseMs = reconnectBaseMs;
    settings.reconnectMaxMs = reconnectMaxMs;
    settings.autoReconnect = autoReconnect;
    settings.lanDiscovery = lanDiscovery;
    settings.lanDiscoveryFinder.serviceType = lanServiceType;
    settings.lanDiscoveryFinder.findTimeoutMs = lanFindTimeoutMs;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreServiceSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool success = true;
    success = config.SetAttribute( "endpoint", endpoint ) && success;
    success = config.SetAttribute( "credentialsDirectory", credentialsDirectory ) && success;
    success = config.SetAttribute( "credentialsKeyFile", credentialsKeyFile ) && success;
    success = config.SetAttribute( "credentialsSlot", credentialsSlot ) && success;
    success = config.SetAttribute( "clientKind", clientKind ) && success;
    success = config.SetAttribute( "clientVersion", clientVersion ) && success;
    success = config.SetAttribute( "bulkChannels", bulkChannels ) && success;
    success = config.SetAttribute( "connectTimeoutMs", connectTimeoutMs ) && success;
    success = config.SetAttribute( "eventWaitMs", eventWaitMs ) && success;
    success = config.SetAttribute( "linkTimeoutMs", linkTimeoutMs ) && success;
    success = config.SetAttribute( "echoIntervalMs", echoIntervalMs ) && success;
    success = config.SetAttribute( "reconnectBaseMs", reconnectBaseMs ) && success;
    success = config.SetAttribute( "reconnectMaxMs", reconnectMaxMs ) && success;
    success = config.SetAttribute( "autoReconnect", autoReconnect ) && success;
    success = config.SetAttribute( "lanDiscovery", lanDiscovery ) && success;
    success = config.SetAttribute( "lanServiceType", lanServiceType ) && success;
    success = config.SetAttribute( "lanFindTimeoutMs", lanFindTimeoutMs ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreServiceSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    for ( CORE::CDataNode::TAttributeMap::const_iterator a = config.AttributeBegin(); a != config.AttributeEnd(); ++a )
    {
        if ( !IsKnownKey( (*a).first ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteAssetStoreServiceSettings: the setting \"" << (*a).first << "\" is not known" );
            return false;
        }
    }
    for ( CORE::CDataNode::const_iterator c = config.ConstBegin(); c != config.ConstEnd(); ++c )
    {
        if ( !IsKnownKey( (*c)->GetName() ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteAssetStoreServiceSettings: the setting \"" << (*c)->GetName() << "\" is not known" );
            return false;
        }
    }
    endpoint = config.GetAttributeValueOrChildValueByName( "endpoint" ).AsString( endpoint, true );
    credentialsDirectory = config.GetAttributeValueOrChildValueByName( "credentialsDirectory" ).AsString( credentialsDirectory, true );
    credentialsKeyFile = config.GetAttributeValueOrChildValueByName( "credentialsKeyFile" ).AsString( credentialsKeyFile, true );
    credentialsSlot = config.GetAttributeValueOrChildValueByName( "credentialsSlot" ).AsString( credentialsSlot, true );
    clientKind = config.GetAttributeValueOrChildValueByName( "clientKind" ).AsString( clientKind, true );
    clientVersion = config.GetAttributeValueOrChildValueByName( "clientVersion" ).AsString( clientVersion, true );
    bulkChannels = config.GetAttributeValueOrChildValueByName( "bulkChannels" ).AsUInt32( bulkChannels, true );
    connectTimeoutMs = config.GetAttributeValueOrChildValueByName( "connectTimeoutMs" ).AsUInt32( connectTimeoutMs, true );
    eventWaitMs = config.GetAttributeValueOrChildValueByName( "eventWaitMs" ).AsUInt32( eventWaitMs, true );
    linkTimeoutMs = config.GetAttributeValueOrChildValueByName( "linkTimeoutMs" ).AsUInt32( linkTimeoutMs, true );
    echoIntervalMs = config.GetAttributeValueOrChildValueByName( "echoIntervalMs" ).AsUInt32( echoIntervalMs, true );
    reconnectBaseMs = config.GetAttributeValueOrChildValueByName( "reconnectBaseMs" ).AsUInt32( reconnectBaseMs, true );
    reconnectMaxMs = config.GetAttributeValueOrChildValueByName( "reconnectMaxMs" ).AsUInt32( reconnectMaxMs, true );
    autoReconnect = config.GetAttributeValueOrChildValueByName( "autoReconnect" ).AsBool( autoReconnect, true );
    lanDiscovery = config.GetAttributeValueOrChildValueByName( "lanDiscovery" ).AsBool( lanDiscovery, true );
    lanServiceType = config.GetAttributeValueOrChildValueByName( "lanServiceType" ).AsString( lanServiceType, true );
    lanFindTimeoutMs = config.GetAttributeValueOrChildValueByName( "lanFindTimeoutMs" ).AsUInt32( lanFindTimeoutMs, true );
    if ( bulkChannels > 2 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteAssetStoreServiceSettings: bulkChannels is 0, 1 or 2" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteAssetStoreServiceSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CRemoteAssetStoreServiceSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
