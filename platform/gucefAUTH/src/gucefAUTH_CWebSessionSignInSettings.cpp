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

#ifndef GUCEF_AUTH_CWEBSESSIONSIGNINSETTINGS_H
#include "gucefAUTH_CWebSessionSignInSettings.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNINSETTINGS_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNINSETTINGS_H ? */

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

const UInt32 CWebSessionSignInSettings::DefaultSignInTimeoutInMs = 900000;
const UInt32 CWebSessionSignInSettings::DefaultHandOverTimeoutInMs = 600000;
const UInt32 CWebSessionSignInSettings::DefaultPollIntervalInMs = 250;
const CORE::CString CWebSessionSignInSettings::DefaultApplicationName = "GUCEF";

/*-------------------------------------------------------------------------*/

CWebSessionSignInSettings::CWebSessionSignInSettings( void )
    : storeId()
    , displayName()
    , script()
    , browser()
    , signInTimeoutInMs( DefaultSignInTimeoutInMs )
    , handOverTimeoutInMs( DefaultHandOverTimeoutInMs )
    , pollIntervalInMs( DefaultPollIntervalInMs )
    , allowUnattendedRenewal( true )
    , saveSession( true )
    , applicationName( DefaultApplicationName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebSessionSignInSettings::GetDisplayName( void ) const
{GUCEF_TRACE;

    if ( !displayName.IsNULLOrEmpty() )
        return displayName;
    if ( !script.name.IsNULLOrEmpty() )
        return script.name;
    return storeId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebSessionSignInSettings::GetProfileDirectory( void ) const
{GUCEF_TRACE;

    if ( !browser.profileDirectory.IsNULLOrEmpty() )
        return browser.profileDirectory;
    return "$APPDATADIR$/" + ( applicationName.IsNULLOrEmpty() ? DefaultApplicationName : applicationName ) + "/WebSignIn/" + storeId;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignInSettings::IsValidStoreId( const CORE::CString& id )
{GUCEF_TRACE;

    const UInt32 byteCount = id.IsNULLOrEmpty() ? 0 : id.ByteSize() - 1;
    if ( 0 == byteCount || byteCount > 64 )
        return false;
    const char* c = id.C_String();
    for ( UInt32 i=0; i<byteCount; ++i )
    {
        const char ch = c[ i ];
        if ( !( ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) || ( ch >= '0' && ch <= '9' ) || '_' == ch || '-' == ch ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignInSettings::Validate( CORE::CString::StringVector& problems ) const
{GUCEF_TRACE;

    const size_t before = problems.size();
    if ( !IsValidStoreId( storeId ) )
        problems.push_back( "the store id must be letters, digits, '_' or '-', at most 64" );
    if ( 0 == signInTimeoutInMs || 0 == handOverTimeoutInMs || 0 == pollIntervalInMs )
        problems.push_back( "the time limits and the poll interval must be more than 0" );
    script.Validate( problems );
    return before == problems.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
