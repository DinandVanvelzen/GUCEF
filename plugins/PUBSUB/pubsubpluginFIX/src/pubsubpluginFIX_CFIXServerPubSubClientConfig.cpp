/*
 *  pubsubpluginFIX: Generic GUCEF COMCORE plugin for providing pubsub via FIX protocol
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#include "pubsubpluginFIX_CFIXServerPubSubClientConfig.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace FIX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFIXServerPubSubClientConfig::CFIXServerPubSubClientConfig( void )
    : PUBSUB::CPubSubClientConfig()
    , senderCompId()
    , fixVersion( "FIX.4.2" )
    , listenPort( 9878 )
    , bindAddress()
    , heartbeatIntervalSecs( 30 )
    , logonTimeoutInMs( 10000 )
    , allowPublishing( false )
    , maxMsgSizeBytes( 65536 )
    , disableChecksumValidation( false )
    , maxConsecutiveChecksumFailures( 3 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientConfig::CFIXServerPubSubClientConfig( const PUBSUB::CPubSubClientConfig& genericConfig )
    : PUBSUB::CPubSubClientConfig( genericConfig )
    , senderCompId()
    , fixVersion( "FIX.4.2" )
    , listenPort( 9878 )
    , bindAddress()
    , heartbeatIntervalSecs( 30 )
    , logonTimeoutInMs( 10000 )
    , allowPublishing( false )
    , maxMsgSizeBytes( 65536 )
    , disableChecksumValidation( false )
    , maxConsecutiveChecksumFailures( 3 )
{GUCEF_TRACE;

    LoadCustomConfig( genericConfig.customConfig );
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientConfig::~CFIXServerPubSubClientConfig()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientConfig&
CFIXServerPubSubClientConfig::operator=( const PUBSUB::CPubSubClientConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientConfig::operator=( src );
        LoadCustomConfig( src.customConfig );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientConfig&
CFIXServerPubSubClientConfig::operator=( const CFIXServerPubSubClientConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientConfig::operator=( src );
        senderCompId                    = src.senderCompId;
        fixVersion                      = src.fixVersion;
        listenPort                      = src.listenPort;
        bindAddress                     = src.bindAddress;
        heartbeatIntervalSecs           = src.heartbeatIntervalSecs;
        logonTimeoutInMs                = src.logonTimeoutInMs;
        allowPublishing                 = src.allowPublishing;
        maxMsgSizeBytes                 = src.maxMsgSizeBytes;
        disableChecksumValidation       = src.disableChecksumValidation;
        maxConsecutiveChecksumFailures  = src.maxConsecutiveChecksumFailures;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientConfig::LoadCustomConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    senderCompId                   = config.GetAttributeValueOrChildValueByName( "senderCompId" ).AsAsciiString( senderCompId, true );
    fixVersion                     = config.GetAttributeValueOrChildValueByName( "fixVersion" ).AsAsciiString( fixVersion, true );
    listenPort                     = (CORE::UInt16) config.GetAttributeValueOrChildValueByName( "listenPort" ).AsUInt32( listenPort, true );
    bindAddress                    = config.GetAttributeValueOrChildValueByName( "bindAddress" ).AsAsciiString( bindAddress, true );
    heartbeatIntervalSecs          = config.GetAttributeValueOrChildValueByName( "heartbeatIntervalSecs" ).AsUInt32( heartbeatIntervalSecs, true );
    logonTimeoutInMs               = config.GetAttributeValueOrChildValueByName( "logonTimeoutInMs" ).AsUInt32( logonTimeoutInMs, true );
    allowPublishing                = config.GetAttributeValueOrChildValueByName( "allowPublishing" ).AsBool( allowPublishing, true );
    maxMsgSizeBytes                = config.GetAttributeValueOrChildValueByName( "maxMsgSizeBytes" ).AsUInt32( maxMsgSizeBytes, true );
    disableChecksumValidation      = config.GetAttributeValueOrChildValueByName( "disableChecksumValidation" ).AsBool( disableChecksumValidation, true );
    maxConsecutiveChecksumFailures = config.GetAttributeValueOrChildValueByName( "maxConsecutiveChecksumFailures" ).AsUInt32( maxConsecutiveChecksumFailures, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientConfig::SaveCustomConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "senderCompId",                   CORE::CString( senderCompId ) );
    config.SetAttribute( "fixVersion",                     CORE::CString( fixVersion ) );
    config.SetAttribute( "listenPort",                     (CORE::UInt32) listenPort );
    config.SetAttribute( "bindAddress",                    CORE::CString( bindAddress ) );
    config.SetAttribute( "heartbeatIntervalSecs",          heartbeatIntervalSecs );
    config.SetAttribute( "logonTimeoutInMs",               logonTimeoutInMs );
    config.SetAttribute( "allowPublishing",                allowPublishing );
    config.SetAttribute( "maxMsgSizeBytes",                maxMsgSizeBytes );
    config.SetAttribute( "disableChecksumValidation",      disableChecksumValidation );
    config.SetAttribute( "maxConsecutiveChecksumFailures", maxConsecutiveChecksumFailures );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientConfig::SerializeCustomConfigToGenericConfig( void )
{GUCEF_TRACE;

    return SaveCustomConfig( customConfig );
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientConfig::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( PUBSUB::CPubSubClientConfig::LoadConfig( config ) )
    {
        return LoadCustomConfig( customConfig );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientConfig::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    PUBSUB::CPubSubClientConfig cfgCopy( *this );
    if ( SaveCustomConfig( cfgCopy.customConfig ) )
    {
        return cfgCopy.SaveConfig( config );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientConfig::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg )
{GUCEF_TRACE;

    if ( &cfg != this )
    {
        PUBSUB::CPubSubClientConfig::operator=( cfg );
    }
    return LoadCustomConfig( cfg.customConfig );
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientConfig::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;

    cfg = *this;
    return SaveCustomConfig( cfg.customConfig );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
