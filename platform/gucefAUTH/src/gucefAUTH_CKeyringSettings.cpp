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

#include "gucefAUTH_CKeyringSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CKeyringSettings::DefaultIdleLockInSeconds;
const UInt32 CKeyringSettings::DefaultFreeAttempts;
const UInt32 CKeyringSettings::DefaultRetryDelayInMs;
const UInt32 CKeyringSettings::DefaultMaxRetryDelayInMs;
const UInt32 CKeyringSettings::DefaultCheckIntervalInMs;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKeyringSettings::CKeyringSettings( void )
    : CORE::CIConfigurable()
    , idleLockInSeconds( DefaultIdleLockInSeconds )
    , freeAttempts( DefaultFreeAttempts )
    , retryDelayInMs( DefaultRetryDelayInMs )
    , maxRetryDelayInMs( DefaultMaxRetryDelayInMs )
    , checkIntervalInMs( DefaultCheckIntervalInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CKeyringSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool ok = config.SetAttribute( "idleLockInSeconds", idleLockInSeconds );
    ok = config.SetAttribute( "freeAttempts", freeAttempts ) && ok;
    ok = config.SetAttribute( "retryDelayInMs", retryDelayInMs ) && ok;
    ok = config.SetAttribute( "maxRetryDelayInMs", maxRetryDelayInMs ) && ok;
    ok = config.SetAttribute( "checkIntervalInMs", checkIntervalInMs ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CKeyringSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    idleLockInSeconds = config.GetAttributeValueOrChildValueByName( "idleLockInSeconds" ).AsUInt32( idleLockInSeconds, true );
    freeAttempts = config.GetAttributeValueOrChildValueByName( "freeAttempts" ).AsUInt32( freeAttempts, true );
    retryDelayInMs = config.GetAttributeValueOrChildValueByName( "retryDelayInMs" ).AsUInt32( retryDelayInMs, true );
    maxRetryDelayInMs = config.GetAttributeValueOrChildValueByName( "maxRetryDelayInMs" ).AsUInt32( maxRetryDelayInMs, true );
    checkIntervalInMs = config.GetAttributeValueOrChildValueByName( "checkIntervalInMs" ).AsUInt32( checkIntervalInMs, true );
    if ( maxRetryDelayInMs < retryDelayInMs )
        maxRetryDelayInMs = retryDelayInMs;
    if ( 0 == checkIntervalInMs )
        checkIntervalInMs = 1;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CKeyringSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CKeyringSettings";
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
