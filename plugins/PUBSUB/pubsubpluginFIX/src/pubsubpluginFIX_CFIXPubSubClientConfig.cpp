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

#include "pubsubpluginFIX_CFIXPubSubClientConfig.h"

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

CFIXPubSubClientConfig::CFIXPubSubClientConfig( void )
    : PUBSUB::CPubSubClientConfig()
    , senderCompId()
    , targetCompId()
    , fixVersion( "FIX.4.2" )
    , heartbeatIntervalSecs( 30 )
    , resetSeqNumOnLogon( false )
    , logonTimeoutInMs( 10000 )
    , useSsl( false )
    , allowPublishing( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientConfig::CFIXPubSubClientConfig( const PUBSUB::CPubSubClientConfig& genericConfig )
    : PUBSUB::CPubSubClientConfig( genericConfig )
    , senderCompId()
    , targetCompId()
    , fixVersion( "FIX.4.2" )
    , heartbeatIntervalSecs( 30 )
    , resetSeqNumOnLogon( false )
    , logonTimeoutInMs( 10000 )
    , useSsl( false )
    , allowPublishing( false )
{GUCEF_TRACE;

    LoadCustomConfig( genericConfig.customConfig );
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientConfig::~CFIXPubSubClientConfig()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientConfig&
CFIXPubSubClientConfig::operator=( const PUBSUB::CPubSubClientConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientConfig::operator=( src );
        LoadCustomConfig( src.customConfig );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientConfig&
CFIXPubSubClientConfig::operator=( const CFIXPubSubClientConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientConfig::operator=( src );
        senderCompId         = src.senderCompId;
        targetCompId         = src.targetCompId;
        fixVersion           = src.fixVersion;
        heartbeatIntervalSecs = src.heartbeatIntervalSecs;
        resetSeqNumOnLogon   = src.resetSeqNumOnLogon;
        logonTimeoutInMs     = src.logonTimeoutInMs;
        useSsl               = src.useSsl;
        allowPublishing      = src.allowPublishing;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientConfig::LoadCustomConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    senderCompId         = config.GetAttributeValueOrChildValueByName( "senderCompId" ).AsString( senderCompId, true );
    targetCompId         = config.GetAttributeValueOrChildValueByName( "targetCompId" ).AsString( targetCompId, true );
    fixVersion           = config.GetAttributeValueOrChildValueByName( "fixVersion" ).AsString( fixVersion, true );
    heartbeatIntervalSecs = config.GetAttributeValueOrChildValueByName( "heartbeatIntervalSecs" ).AsUInt32( heartbeatIntervalSecs, true );
    resetSeqNumOnLogon   = config.GetAttributeValueOrChildValueByName( "resetSeqNumOnLogon" ).AsBool( resetSeqNumOnLogon, true );
    logonTimeoutInMs     = config.GetAttributeValueOrChildValueByName( "logonTimeoutInMs" ).AsUInt32( logonTimeoutInMs, true );
    useSsl               = config.GetAttributeValueOrChildValueByName( "useSsl" ).AsBool( useSsl, true );
    allowPublishing      = config.GetAttributeValueOrChildValueByName( "allowPublishing" ).AsBool( allowPublishing, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientConfig::SaveCustomConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "senderCompId",          senderCompId );
    config.SetAttribute( "targetCompId",          targetCompId );
    config.SetAttribute( "fixVersion",            fixVersion );
    config.SetAttribute( "heartbeatIntervalSecs", heartbeatIntervalSecs );
    config.SetAttribute( "resetSeqNumOnLogon",    resetSeqNumOnLogon );
    config.SetAttribute( "logonTimeoutInMs",      logonTimeoutInMs );
    config.SetAttribute( "useSsl",                useSsl );
    config.SetAttribute( "allowPublishing",       allowPublishing );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientConfig::SerializeCustomConfigToGenericConfig( void )
{GUCEF_TRACE;

    return SaveCustomConfig( customConfig );
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientConfig::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( PUBSUB::CPubSubClientConfig::LoadConfig( config ) )
    {
        return LoadCustomConfig( customConfig );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientConfig::SaveConfig( CORE::CDataNode& config ) const
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
CFIXPubSubClientConfig::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg )
{GUCEF_TRACE;

    if ( &cfg != this )
    {
        PUBSUB::CPubSubClientConfig::operator=( cfg );
    }
    return LoadCustomConfig( cfg.customConfig );
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientConfig::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
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
