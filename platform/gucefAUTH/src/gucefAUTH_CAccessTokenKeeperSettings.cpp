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

#include "gucefAUTH_CAccessTokenKeeperSettings.h"

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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

const UInt32 CAccessTokenKeeperSettings::DefaultRefreshAheadMs;
const UInt32 CAccessTokenKeeperSettings::DefaultExpirySkewMs;
const UInt32 CAccessTokenKeeperSettings::DefaultRetryDelayMs;
const UInt32 CAccessTokenKeeperSettings::DefaultMaxRetryDelayMs;
const UInt32 CAccessTokenKeeperSettings::DefaultReauthWarnHours;
const UInt32 CAccessTokenKeeperSettings::DefaultCheckIntervalMs;
const UInt32 CAccessTokenKeeperSettings::DefaultPollIntervalMs;
const UInt32 CAccessTokenKeeperSettings::DefaultRefreshTimeoutMs;

/*-------------------------------------------------------------------------*/

CAccessTokenKeeperSettings::CAccessTokenKeeperSettings( void )
    : CORE::CIConfigurable()
    , refreshAheadMs( DefaultRefreshAheadMs )
    , expirySkewMs( DefaultExpirySkewMs )
    , retryDelayMs( DefaultRetryDelayMs )
    , maxRetryDelayMs( DefaultMaxRetryDelayMs )
    , refreshTokenLifetimeHours( 0 )
    , reauthWarnHours( DefaultReauthWarnHours )
    , checkIntervalMs( DefaultCheckIntervalMs )
    , pollIntervalMs( DefaultPollIntervalMs )
    , refreshTimeoutMs( DefaultRefreshTimeoutMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeperSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool ok = config.SetAttribute( "refreshAheadMs", refreshAheadMs );
    ok = config.SetAttribute( "expirySkewMs", expirySkewMs ) && ok;
    ok = config.SetAttribute( "retryDelayMs", retryDelayMs ) && ok;
    ok = config.SetAttribute( "maxRetryDelayMs", maxRetryDelayMs ) && ok;
    ok = config.SetAttribute( "refreshTokenLifetimeHours", refreshTokenLifetimeHours ) && ok;
    ok = config.SetAttribute( "reauthWarnHours", reauthWarnHours ) && ok;
    ok = config.SetAttribute( "checkIntervalMs", checkIntervalMs ) && ok;
    ok = config.SetAttribute( "pollIntervalMs", pollIntervalMs ) && ok;
    ok = config.SetAttribute( "refreshTimeoutMs", refreshTimeoutMs ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeperSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    static const UInt32 OneDayMs = 24 * 3600 * 1000;

    refreshAheadMs = config.GetAttributeValueOrChildValueByName( "refreshAheadMs" ).AsUInt32( refreshAheadMs, true );
    expirySkewMs = config.GetAttributeValueOrChildValueByName( "expirySkewMs" ).AsUInt32( expirySkewMs, true );
    retryDelayMs = config.GetAttributeValueOrChildValueByName( "retryDelayMs" ).AsUInt32( retryDelayMs, true );
    maxRetryDelayMs = config.GetAttributeValueOrChildValueByName( "maxRetryDelayMs" ).AsUInt32( maxRetryDelayMs, true );
    refreshTokenLifetimeHours = config.GetAttributeValueOrChildValueByName( "refreshTokenLifetimeHours" ).AsUInt32( refreshTokenLifetimeHours, true );
    reauthWarnHours = config.GetAttributeValueOrChildValueByName( "reauthWarnHours" ).AsUInt32( reauthWarnHours, true );
    checkIntervalMs = config.GetAttributeValueOrChildValueByName( "checkIntervalMs" ).AsUInt32( checkIntervalMs, true );
    pollIntervalMs = config.GetAttributeValueOrChildValueByName( "pollIntervalMs" ).AsUInt32( pollIntervalMs, true );
    refreshTimeoutMs = config.GetAttributeValueOrChildValueByName( "refreshTimeoutMs" ).AsUInt32( refreshTimeoutMs, true );

    if ( refreshAheadMs > OneDayMs )
        refreshAheadMs = OneDayMs;
    if ( expirySkewMs > OneDayMs )
        expirySkewMs = OneDayMs;
    if ( maxRetryDelayMs < retryDelayMs )
        maxRetryDelayMs = retryDelayMs;
    if ( 0 == checkIntervalMs )
        checkIntervalMs = 1;
    if ( 0 == pollIntervalMs )
        pollIntervalMs = 1;
    if ( 0 == refreshTimeoutMs )
        refreshTimeoutMs = 1;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAccessTokenKeeperSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAccessTokenKeeperSettings";
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
