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

#ifndef GUCEF_AUTH_CACCESSTOKENKEEPERSETTINGS_H
#define GUCEF_AUTH_CACCESSTOKENKEEPERSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of a CAccessTokenKeeper (all times in milliseconds unless the name says otherwise). They belong to the
 *  application, not to a person: LoadConfig() takes the node of the application's configuration that holds them.
 */
class GUCEF_AUTH_EXPORT_CPP CAccessTokenKeeperSettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 DefaultRefreshAheadMs = 120000;
    static const UInt32 DefaultExpirySkewMs = 30000;
    static const UInt32 DefaultRetryDelayMs = 30000;
    static const UInt32 DefaultMaxRetryDelayMs = 600000;
    static const UInt32 DefaultReauthWarnHours = 24;
    static const UInt32 DefaultCheckIntervalMs = 250;
    static const UInt32 DefaultPollIntervalMs = 25;
    static const UInt32 DefaultRefreshTimeoutMs = 60000;

    UInt32 refreshAheadMs;              /**< a renewal starts when the token expires within this time (capped at half the life of the token) */
    UInt32 expirySkewMs;                /**< a token that expires within this time is not handed out (capped at half the life of the token) */
    UInt32 retryDelayMs;                /**< after a renewal that failed for another reason than a refused refresh token; doubles with every further failure */
    UInt32 maxRetryDelayMs;             /**< the cap of that doubling */
    UInt32 refreshTokenLifetimeHours;   /**< the life of a refresh token from the interactive login; 0 takes CAuthProviderDescriptor::refreshTokenLifetimeInSeconds, and when that is 0 too the life is unknown and nothing is warned */
    UInt32 reauthWarnHours;             /**< ReauthDueSoonEvent is raised when less than this remains of the life of the refresh token */
    UInt32 checkIntervalMs;             /**< how often the keeper looks at the clock and at requests for a renewal when nothing runs: a renewal asked for from any thread starts within this time */
    UInt32 pollIntervalMs;              /**< how often a renewal in flight is looked at */
    UInt32 refreshTimeoutMs;            /**< a renewal that takes longer is cancelled and counts as a failure of the network */

    CAccessTokenKeeperSettings( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Values that are not there keep what they were; refreshAheadMs and expirySkewMs are capped at a day, the intervals have a floor of 1 */
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CACCESSTOKENKEEPERSETTINGS_H ? */
