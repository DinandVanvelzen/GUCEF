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

#include "gucefAUTH_CAccessTokenKeeperEventData.h"

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

CAccessTokenKeeperEventData::CAccessTokenKeeperEventData( void )
    : CORE::CICloneable()
    , providerName()
    , accountRef()
    , accessTokenExpiresAtMs( 0 )
    , failureReason( AUTHFAIL_UNKNOWN )
    , retryAtMs( 0 )
    , reauthReason( REAUTH_NONE )
    , refreshTokenExpiresAtMs( 0 )
    , remainingMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeperEventData::CAccessTokenKeeperEventData( const CAccessTokenKeeperEventData& src )
    : CORE::CICloneable( src )
    , providerName( src.providerName )
    , accountRef( src.accountRef )
    , accessTokenExpiresAtMs( src.accessTokenExpiresAtMs )
    , failureReason( src.failureReason )
    , retryAtMs( src.retryAtMs )
    , reauthReason( src.reauthReason )
    , refreshTokenExpiresAtMs( src.refreshTokenExpiresAtMs )
    , remainingMs( src.remainingMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeperEventData::~CAccessTokenKeeperEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeperEventData&
CAccessTokenKeeperEventData::operator=( const CAccessTokenKeeperEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        providerName = src.providerName;
        accountRef = src.accountRef;
        accessTokenExpiresAtMs = src.accessTokenExpiresAtMs;
        failureReason = src.failureReason;
        retryAtMs = src.retryAtMs;
        reauthReason = src.reauthReason;
        refreshTokenExpiresAtMs = src.refreshTokenExpiresAtMs;
        remainingMs = src.remainingMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAccessTokenKeeperEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAccessTokenKeeperEventData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
