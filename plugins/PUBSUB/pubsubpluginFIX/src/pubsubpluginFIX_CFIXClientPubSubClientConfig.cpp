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

#include "pubsubpluginFIX_CFIXClientPubSubClientConfig.h"

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

CFIXClientPubSubClientConfig::CFIXClientPubSubClientConfig( void )
    : PUBSUB::CPubSubClientConfig()
    , senderCompId()
    , targetCompId()
    , fixVersion( "FIX.4.2" )
    , heartbeatIntervalSecs( 30 )
    , resetSeqNumOnLogon( false )
    , logonTimeoutInMs( 10000 )
    , useSsl( false )
    , allowPublishing( false )
    , maxMsgSizeBytes( 65536 )
    , disableChecksumValidation( false )
    , maxConsecutiveChecksumFailures( 3 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientConfig::CFIXClientPubSubClientConfig( const PUBSUB::CPubSubClientConfig& genericConfig )
    : PUBSUB::CPubSubClientConfig( genericConfig )
    , senderCompId()
    , targetCompId()
    , fixVersion( "FIX.4.2" )
    , heartbeatIntervalSecs( 30 )
    , resetSeqNumOnLogon( false )
    , logonTimeoutInMs( 10000 )
    , useSsl( false )
    , allowPublishing( false )
    , maxMsgSizeBytes( 65536 )
    , disableChecksumValidation( false )
    , maxConsecutiveChecksumFailures( 3 )
{GUCEF_TRACE;

    LoadCustomConfig( genericConfig.customConfig );
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientConfig::~CFIXClientPubSubClientConfig()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientConfig&
CFIXClientPubSubClientConfig::operator=( const PUBSUB::CPubSubClientConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientConfig::operator=( src );
        LoadCustomConfig( src.customConfig );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientConfig&
CFIXClientPubSubClientConfig::operator=( const CFIXClientPubSubClientConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientConfig::operator=( src );
        senderCompId                    = src.senderCompId;
        targetCompId                    = src.targetCompId;
        fixVersion                      = src.fixVersion;
        heartbeatIntervalSecs           = src.heartbeatIntervalSecs;
        resetSeqNumOnLogon              = src.resetSeqNumOnLogon;
        logonTimeoutInMs                = src.logonTimeoutInMs;
        useSsl                          = src.useSsl;
        allowPublishing                 = src.allowPublishing;
        maxMsgSizeBytes                 = src.maxMsgSizeBytes;
        disableChecksumValidation       = src.disableChecksumValidation;
        maxConsecutiveChecksumFailures  = src.maxConsecutiveChecksumFailures;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClientConfig::LoadCustomConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    senderCompId                   = config.GetAttributeValueOrChildValueByName( "senderCompId" ).AsAsciiString( senderCompId, true );
    targetCompId                   = config.GetAttributeValueOrChildValueByName( "targetCompId" ).AsAsciiString( targetCompId, true );
    fixVersion                     = config.GetAttributeValueOrChildValueByName( "fixVersion" ).AsAsciiString( fixVersion, true );
    heartbeatIntervalSecs          = config.GetAttributeValueOrChildValueByName( "heartbeatIntervalSecs" ).AsUInt32( heartbeatIntervalSecs, true );
    resetSeqNumOnLogon             = config.GetAttributeValueOrChildValueByName( "resetSeqNumOnLogon" ).AsBool( resetSeqNumOnLogon, true );
    logonTimeoutInMs               = config.GetAttributeValueOrChildValueByName( "logonTimeoutInMs" ).AsUInt32( logonTimeoutInMs, true );
    useSsl                         = config.GetAttributeValueOrChildValueByName( "useSsl" ).AsBool( useSsl, true );
    allowPublishing                = config.GetAttributeValueOrChildValueByName( "allowPublishing" ).AsBool( allowPublishing, true );
    maxMsgSizeBytes                = config.GetAttributeValueOrChildValueByName( "maxMsgSizeBytes" ).AsUInt32( maxMsgSizeBytes, true );
    disableChecksumValidation      = config.GetAttributeValueOrChildValueByName( "disableChecksumValidation" ).AsBool( disableChecksumValidation, true );
    maxConsecutiveChecksumFailures = config.GetAttributeValueOrChildValueByName( "maxConsecutiveChecksumFailures" ).AsUInt32( maxConsecutiveChecksumFailures, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClientConfig::SaveCustomConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "senderCompId",                   CORE::CString( senderCompId ) );
    config.SetAttribute( "targetCompId",                   CORE::CString( targetCompId ) );
    config.SetAttribute( "fixVersion",                     CORE::CString( fixVersion ) );
    config.SetAttribute( "heartbeatIntervalSecs",          heartbeatIntervalSecs );
    config.SetAttribute( "resetSeqNumOnLogon",             resetSeqNumOnLogon );
    config.SetAttribute( "logonTimeoutInMs",               logonTimeoutInMs );
    config.SetAttribute( "useSsl",                         useSsl );
    config.SetAttribute( "allowPublishing",                allowPublishing );
    config.SetAttribute( "maxMsgSizeBytes",                maxMsgSizeBytes );
    config.SetAttribute( "disableChecksumValidation",      disableChecksumValidation );
    config.SetAttribute( "maxConsecutiveChecksumFailures", maxConsecutiveChecksumFailures );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClientConfig::SerializeCustomConfigToGenericConfig( void )
{GUCEF_TRACE;

    return SaveCustomConfig( customConfig );
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClientConfig::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( PUBSUB::CPubSubClientConfig::LoadConfig( config ) )
    {
        return LoadCustomConfig( customConfig );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClientConfig::SaveConfig( CORE::CDataNode& config ) const
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
CFIXClientPubSubClientConfig::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg )
{GUCEF_TRACE;

    if ( &cfg != this )
    {
        PUBSUB::CPubSubClientConfig::operator=( cfg );
    }
    return LoadCustomConfig( cfg.customConfig );
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClientConfig::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
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
