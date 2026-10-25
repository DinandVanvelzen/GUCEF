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

#include "gucefAUTH_CAuthUnavailableProvider.h"

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

CAuthUnavailableProvider::CAuthUnavailableProvider( void )
    : providerName()
    , reason( AUTHUNAVAILABLE_NONE )
    , message()
    , configKey()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthUnavailableProvider::CAuthUnavailableProvider( const CORE::CString& provider       ,
                                                    TAuthUnavailableReason unavailable  ,
                                                    const CORE::CString& line           ,
                                                    const CORE::CString& key            )
    : providerName( provider )
    , reason( unavailable )
    , message( line )
    , configKey( key )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAuthUnavailableProvider::Clear( void )
{GUCEF_TRACE;

    providerName.Clear();
    reason = AUTHUNAVAILABLE_NONE;
    message.Clear();
    configKey.Clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthUnavailableProvider::GetTooltipText( void ) const
{GUCEF_TRACE;

    if ( configKey.IsNULLOrEmpty() )
        return message;
    return message + ". Fill in " + configKey + " in the configuration";
}

/*-------------------------------------------------------------------------*/

const char*
CAuthUnavailableProvider::GetReasonName( TAuthUnavailableReason reason )
{GUCEF_TRACE;

    switch ( reason )
    {
        case AUTHUNAVAILABLE_NO_CLIENT_ID:              return "no-client-id";
        case AUTHUNAVAILABLE_MISSING_SETTING:           return "missing-setting";
        case AUTHUNAVAILABLE_MISSING_PLUGIN:            return "missing-plugin";
        case AUTHUNAVAILABLE_PLATFORM_NOT_SUPPORTED:    return "platform-not-supported";
        case AUTHUNAVAILABLE_NOT_AVAILABLE:             return "not-available";
        default:                                        return "none";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
