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

#include "gucefAUTH_IOpenId2ProviderProfile.h"

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

IOpenId2ProviderProfile::IOpenId2ProviderProfile( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

IOpenId2ProviderProfile::~IOpenId2ProviderProfile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
IOpenId2ProviderProfile::IsRelayAllowed( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
IOpenId2ProviderProfile::GetDisplayName( const COpenId2Assertion& assertion, CORE::CString& displayName ) const
{GUCEF_TRACE;

    displayName.Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
IOpenId2ProviderProfile::GetDetectedAccountHint( CORE::CString& displayName, bool& isRunning ) const
{GUCEF_TRACE;

    displayName.Clear();
    isRunning = false;
    return false;
}

/*-------------------------------------------------------------------------*/

void
IOpenId2ProviderProfile::ForgetDetectedAccountHint( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
