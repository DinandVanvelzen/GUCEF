/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefMULTIPLAY_CIIdentityProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIIdentityProvider::IdentityChangedEvent = "GUCEF::MULTIPLAY::CIIdentityProvider::IdentityChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIIdentityProvider::RegisterEvents( void )
{GUCEF_TRACE;

    IdentityChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIIdentityProvider::CIIdentityProvider( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , AUTH::IAccountProfile()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIIdentityProvider::~CIIdentityProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIIdentityProvider::Login( const CORE::CString& /* externalProviderName */, CORE::CFutureResult& /* future */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIIdentityProvider::AutoLogin( CORE::CFutureResult& /* future */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIIdentityProvider::GetDisplayName( void )
{GUCEF_TRACE;

    CPlayerIdentity identity;
    if ( !GetIdentity( identity ) )
        return CORE::CString::Empty;
    return identity.displayName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIIdentityProvider::GetProfileId( void )
{GUCEF_TRACE;

    CPlayerIdentity identity;
    if ( !GetIdentity( identity ) )
        return CORE::CString::Empty;
    return identity.userId;
}

/*-------------------------------------------------------------------------*/

CORE::CNotifier*
CIIdentityProvider::GetProfileNotifier( void )
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

const CORE::CEvent&
CIIdentityProvider::GetProfileChangedEvent( void ) const
{GUCEF_TRACE;

    return IdentityChangedEvent;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
