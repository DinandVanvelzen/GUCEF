/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#include "gucefAUTH_COAuth2EngineSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COAuth2EngineSettings::COAuth2EngineSettings( void )
    : CORE::CIConfigurable()
    , allowInsecureHttp( false )
    , launchBrowser( true )
    , clockSkewInSeconds( 120 )
    , httpConnectTimeoutInMs( 10000 )
    , httpTimeoutInMs( 20000 )
    , maxResponseBytes( 1048576 )
    , jwksMinTtlInSeconds( 60 )
    , jwksMaxTtlInSeconds( 86400 )
    , jwksDefaultTtlInSeconds( 3600 )
    , jwksMinRefreshIntervalInMs( 10000 )
    , discoveryTtlInSeconds( 3600 )
    , deviceIntervalUnitInMs( 1000 )
    , deviceDefaultIntervalInSeconds( 5 )
    , deviceSlowDownIncrementInSeconds( 5 )
    , openId2MaxAgeInSeconds( 300 )
    , maxActiveLogins( 16 )
    , caBundlePath()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COAuth2EngineSettings::COAuth2EngineSettings( const COAuth2EngineSettings& src )
    : CORE::CIConfigurable( src )
    , allowInsecureHttp( src.allowInsecureHttp )
    , launchBrowser( src.launchBrowser )
    , clockSkewInSeconds( src.clockSkewInSeconds )
    , httpConnectTimeoutInMs( src.httpConnectTimeoutInMs )
    , httpTimeoutInMs( src.httpTimeoutInMs )
    , maxResponseBytes( src.maxResponseBytes )
    , jwksMinTtlInSeconds( src.jwksMinTtlInSeconds )
    , jwksMaxTtlInSeconds( src.jwksMaxTtlInSeconds )
    , jwksDefaultTtlInSeconds( src.jwksDefaultTtlInSeconds )
    , jwksMinRefreshIntervalInMs( src.jwksMinRefreshIntervalInMs )
    , discoveryTtlInSeconds( src.discoveryTtlInSeconds )
    , deviceIntervalUnitInMs( src.deviceIntervalUnitInMs )
    , deviceDefaultIntervalInSeconds( src.deviceDefaultIntervalInSeconds )
    , deviceSlowDownIncrementInSeconds( src.deviceSlowDownIncrementInSeconds )
    , openId2MaxAgeInSeconds( src.openId2MaxAgeInSeconds )
    , maxActiveLogins( src.maxActiveLogins )
    , caBundlePath( src.caBundlePath )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COAuth2EngineSettings::~COAuth2EngineSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COAuth2EngineSettings&
COAuth2EngineSettings::operator=( const COAuth2EngineSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        allowInsecureHttp = src.allowInsecureHttp;
        launchBrowser = src.launchBrowser;
        clockSkewInSeconds = src.clockSkewInSeconds;
        httpConnectTimeoutInMs = src.httpConnectTimeoutInMs;
        httpTimeoutInMs = src.httpTimeoutInMs;
        maxResponseBytes = src.maxResponseBytes;
        jwksMinTtlInSeconds = src.jwksMinTtlInSeconds;
        jwksMaxTtlInSeconds = src.jwksMaxTtlInSeconds;
        jwksDefaultTtlInSeconds = src.jwksDefaultTtlInSeconds;
        jwksMinRefreshIntervalInMs = src.jwksMinRefreshIntervalInMs;
        discoveryTtlInSeconds = src.discoveryTtlInSeconds;
        deviceIntervalUnitInMs = src.deviceIntervalUnitInMs;
        deviceDefaultIntervalInSeconds = src.deviceDefaultIntervalInSeconds;
        deviceSlowDownIncrementInSeconds = src.deviceSlowDownIncrementInSeconds;
        openId2MaxAgeInSeconds = src.openId2MaxAgeInSeconds;
        maxActiveLogins = src.maxActiveLogins;
        caBundlePath = src.caBundlePath;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2EngineSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool ok = true;
    ok = config.SetAttribute( "allowInsecureHttp", allowInsecureHttp ) && ok;
    ok = config.SetAttribute( "launchBrowser", launchBrowser ) && ok;
    ok = config.SetAttribute( "clockSkewInSeconds", clockSkewInSeconds ) && ok;
    ok = config.SetAttribute( "httpConnectTimeoutInMs", httpConnectTimeoutInMs ) && ok;
    ok = config.SetAttribute( "httpTimeoutInMs", httpTimeoutInMs ) && ok;
    ok = config.SetAttribute( "maxResponseBytes", maxResponseBytes ) && ok;
    ok = config.SetAttribute( "jwksMinTtlInSeconds", jwksMinTtlInSeconds ) && ok;
    ok = config.SetAttribute( "jwksMaxTtlInSeconds", jwksMaxTtlInSeconds ) && ok;
    ok = config.SetAttribute( "jwksDefaultTtlInSeconds", jwksDefaultTtlInSeconds ) && ok;
    ok = config.SetAttribute( "jwksMinRefreshIntervalInMs", jwksMinRefreshIntervalInMs ) && ok;
    ok = config.SetAttribute( "discoveryTtlInSeconds", discoveryTtlInSeconds ) && ok;
    ok = config.SetAttribute( "deviceIntervalUnitInMs", deviceIntervalUnitInMs ) && ok;
    ok = config.SetAttribute( "deviceDefaultIntervalInSeconds", deviceDefaultIntervalInSeconds ) && ok;
    ok = config.SetAttribute( "deviceSlowDownIncrementInSeconds", deviceSlowDownIncrementInSeconds ) && ok;
    ok = config.SetAttribute( "openId2MaxAgeInSeconds", openId2MaxAgeInSeconds ) && ok;
    ok = config.SetAttribute( "maxActiveLogins", maxActiveLogins ) && ok;
    ok = config.SetAttribute( "caBundlePath", caBundlePath ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2EngineSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &config;
    if ( config.GetName() != "Engine" )
    {
        const CORE::CDataNode* auth = &config;
        if ( config.GetName() != "Auth" )
        {
            const CORE::CDataNode* authChild = config.FindChild( "Auth" );
            if ( GUCEF_NULL != authChild )
                auth = authChild;
        }
        const CORE::CDataNode* engine = auth->FindChild( "Engine" );
        if ( GUCEF_NULL != engine )
            section = engine;
    }

    allowInsecureHttp = section->GetAttributeValueOrChildValueByName( "allowInsecureHttp" ).AsBool( allowInsecureHttp, true );
    launchBrowser = section->GetAttributeValueOrChildValueByName( "launchBrowser" ).AsBool( launchBrowser, true );
    clockSkewInSeconds = section->GetAttributeValueOrChildValueByName( "clockSkewInSeconds" ).AsUInt32( clockSkewInSeconds, true );
    httpConnectTimeoutInMs = section->GetAttributeValueOrChildValueByName( "httpConnectTimeoutInMs" ).AsUInt32( httpConnectTimeoutInMs, true );
    httpTimeoutInMs = section->GetAttributeValueOrChildValueByName( "httpTimeoutInMs" ).AsUInt32( httpTimeoutInMs, true );
    maxResponseBytes = section->GetAttributeValueOrChildValueByName( "maxResponseBytes" ).AsUInt32( maxResponseBytes, true );
    jwksMinTtlInSeconds = section->GetAttributeValueOrChildValueByName( "jwksMinTtlInSeconds" ).AsUInt32( jwksMinTtlInSeconds, true );
    jwksMaxTtlInSeconds = section->GetAttributeValueOrChildValueByName( "jwksMaxTtlInSeconds" ).AsUInt32( jwksMaxTtlInSeconds, true );
    jwksDefaultTtlInSeconds = section->GetAttributeValueOrChildValueByName( "jwksDefaultTtlInSeconds" ).AsUInt32( jwksDefaultTtlInSeconds, true );
    jwksMinRefreshIntervalInMs = section->GetAttributeValueOrChildValueByName( "jwksMinRefreshIntervalInMs" ).AsUInt32( jwksMinRefreshIntervalInMs, true );
    discoveryTtlInSeconds = section->GetAttributeValueOrChildValueByName( "discoveryTtlInSeconds" ).AsUInt32( discoveryTtlInSeconds, true );
    deviceIntervalUnitInMs = section->GetAttributeValueOrChildValueByName( "deviceIntervalUnitInMs" ).AsUInt32( deviceIntervalUnitInMs, true );
    deviceDefaultIntervalInSeconds = section->GetAttributeValueOrChildValueByName( "deviceDefaultIntervalInSeconds" ).AsUInt32( deviceDefaultIntervalInSeconds, true );
    deviceSlowDownIncrementInSeconds = section->GetAttributeValueOrChildValueByName( "deviceSlowDownIncrementInSeconds" ).AsUInt32( deviceSlowDownIncrementInSeconds, true );
    openId2MaxAgeInSeconds = section->GetAttributeValueOrChildValueByName( "openId2MaxAgeInSeconds" ).AsUInt32( openId2MaxAgeInSeconds, true );
    maxActiveLogins = section->GetAttributeValueOrChildValueByName( "maxActiveLogins" ).AsUInt32( maxActiveLogins, true );
    caBundlePath = section->GetAttributeValueOrChildValueByName( "caBundlePath" ).AsString( caBundlePath, true );
    if ( clockSkewInSeconds > 600 )
        clockSkewInSeconds = 600;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COAuth2EngineSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::COAuth2EngineSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
