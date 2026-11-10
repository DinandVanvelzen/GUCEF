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

#ifndef GUCEF_AUTH_CACCESSTOKENKEEPEREVENTDATA_H
#define GUCEF_AUTH_CACCESSTOKENKEEPEREVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

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

/** What CAccessTokenKeeper::GetAccessToken() has to say */
enum EAccessTokenStatus
{
    TOKEN_READY = 0,        /**< the token is there and lives longer than the skew */
    TOKEN_PENDING,          /**< a renewal is running or was asked for: wait on the future of RequestRefresh() */
    TOKEN_FAILED,           /**< the last renewal failed and the retry delay has not passed */
    NO_LOGIN                /**< no account is set, there is no token and no refresh token, or the keeper was shut down */
};

/*-------------------------------------------------------------------------*/

typedef enum EAccessTokenStatus TAccessTokenStatus;

/*-------------------------------------------------------------------------*/

/** Why CAccessTokenKeeper::ReauthRequiredEvent was raised */
enum EReauthReason
{
    REAUTH_NONE = 0,
    REAUTH_REFRESH_TOKEN_EXPIRED,       /**< the refresh token is past the lifetime the provider declares, counted from the last interactive login */
    REAUTH_REFRESH_TOKEN_REJECTED,      /**< the provider refused the refresh token (AUTHFAIL_INVALID_GRANT): it is removed from the store */
    REAUTH_NO_REFRESH_TOKEN             /**< a renewal is needed and the store has no refresh token for the account */
};

/*-------------------------------------------------------------------------*/

typedef enum EReauthReason TReauthReason;

/*-------------------------------------------------------------------------*/

/** Event data of the events of CAccessTokenKeeper; no token is ever part of it */
class GUCEF_AUTH_EXPORT_CPP CAccessTokenKeeperEventData : public CORE::CICloneable
{
    public:

    CORE::CString providerName;
    CAccountRef accountRef;
    UInt64 accessTokenExpiresAtMs;      /**< TokenRefreshedEvent: the expiry of the new access token, ms since 1970 UTC, 0 when it is not known */
    TAuthFailureReason failureReason;   /**< TokenRefreshFailedEvent: why the renewal failed */
    UInt64 retryAtMs;                   /**< TokenRefreshFailedEvent: when the next renewal may be tried, 0 when not before a new login */
    TReauthReason reauthReason;         /**< ReauthRequiredEvent */
    UInt64 refreshTokenExpiresAtMs;     /**< ReauthDueSoonEvent, ReauthRequiredEvent: when the refresh token expires by the declared lifetime, 0 when it is not known */
    UInt64 remainingMs;                 /**< ReauthDueSoonEvent: how long the refresh token still lives */

    CAccessTokenKeeperEventData( void );

    CAccessTokenKeeperEventData( const CAccessTokenKeeperEventData& src );

    virtual ~CAccessTokenKeeperEventData();

    CAccessTokenKeeperEventData& operator=( const CAccessTokenKeeperEventData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CACCESSTOKENKEEPEREVENTDATA_H ? */
