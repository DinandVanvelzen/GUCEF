/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CWEBSIGNINBROWSERSETTINGS_H
#include "gucefASSETSTORE_CWebSignInBrowserSettings.h"
#define GUCEF_ASSETSTORE_CWEBSIGNINBROWSERSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CWEBSIGNINBROWSERSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CWebSignInBrowserSettings::DefaultStartTimeoutInMs = 30000;
const UInt32 CWebSignInBrowserSettings::DefaultCommandTimeoutInMs = 15000;

/*-------------------------------------------------------------------------*/

CWebSignInBrowserSettings::CWebSignInBrowserSettings( void )
    : executablePath()
    , profileDirectory()
    , debuggingPort( 0 )
    , headless( false )
    , keepOpen( false )
    , extraArguments()
    , startTimeoutInMs( DefaultStartTimeoutInMs )
    , commandTimeoutInMs( DefaultCommandTimeoutInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebSignInBrowserSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "executablePath", executablePath );
    config.SetAttribute( "profileDirectory", profileDirectory );
    config.SetAttribute( "debuggingPort", (UInt32) debuggingPort );
    config.SetAttribute( "headless", headless );
    config.SetAttribute( "keepOpen", keepOpen );
    config.SetAttribute( "startTimeoutMs", startTimeoutInMs );
    config.SetAttribute( "commandTimeoutMs", commandTimeoutInMs );
    if ( !extraArguments.empty() )
        config.SetValuesOfChildByName( "extraArguments", extraArguments );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSignInBrowserSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    executablePath = config.GetAttributeValueOrChildValueByName( "executablePath" ).AsString( executablePath );
    profileDirectory = config.GetAttributeValueOrChildValueByName( "profileDirectory" ).AsString( profileDirectory );
    const UInt32 port = config.GetAttributeValueOrChildValueByName( "debuggingPort" ).AsUInt32( debuggingPort );
    debuggingPort = port <= 65535 ? (UInt16) port : debuggingPort;
    headless = config.GetAttributeValueOrChildValueByName( "headless" ).AsBool( headless );
    keepOpen = config.GetAttributeValueOrChildValueByName( "keepOpen" ).AsBool( keepOpen );
    startTimeoutInMs = config.GetAttributeValueOrChildValueByName( "startTimeoutMs" ).AsUInt32( startTimeoutInMs );
    commandTimeoutInMs = config.GetAttributeValueOrChildValueByName( "commandTimeoutMs" ).AsUInt32( commandTimeoutInMs );
    CORE::CString::StringVector arguments;
    if ( config.GetValuesOfChildByName( "extraArguments", arguments ) )
        extraArguments = arguments;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSignInBrowserSettings::IsArgumentAllowed( const CORE::CString& argument )
{GUCEF_TRACE;

    const CORE::CString lower = argument.Lowercase();
    return !lower.StartsWith( "--remote-debugging" ) && !lower.StartsWith( "--remote-allow-origins" ) && !lower.StartsWith( "--user-data-dir" ) &&
           !lower.StartsWith( "-remote-debugging" ) && !lower.StartsWith( "-user-data-dir" ) && !lower.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
