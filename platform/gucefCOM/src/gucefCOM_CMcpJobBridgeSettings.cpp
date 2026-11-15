/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#include "gucefCOM_CMcpJobBridgeSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMcpJobBridgeSettings::SectionName = "McpJobBridge";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpJobBridgeSettings::CMcpJobBridgeSettings( void )
    : defaultWaitMs( 20000 )
    , maxWaitMs( 60000 )
    , pollIntervalMs( 100 )
    , resultTtlMs( 604800000 )
    , requestKeyTtlMs( 86400000 )
    , maxRequestKeysPerPrincipal( 256 )
    , maxRequestKeysTotal( 4096 )
    , maxActivePerPrincipal( 8 )
    , maxJobs( 10000 )
    , operatorScope( "jobs:all" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridgeSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetNodeType( GUCEF_DATATYPE_OBJECT );
    config.SetAttribute( "defaultWaitMs", defaultWaitMs );
    config.SetAttribute( "maxWaitMs", maxWaitMs );
    config.SetAttribute( "pollIntervalMs", pollIntervalMs );
    config.SetAttribute( "resultTtlMs", resultTtlMs );
    config.SetAttribute( "requestKeyTtlMs", requestKeyTtlMs );
    config.SetAttribute( "maxRequestKeysPerPrincipal", maxRequestKeysPerPrincipal );
    config.SetAttribute( "maxRequestKeysTotal", maxRequestKeysTotal );
    config.SetAttribute( "maxActivePerPrincipal", maxActivePerPrincipal );
    config.SetAttribute( "maxJobs", maxJobs );
    config.SetAttribute( "operatorScope", operatorScope );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridgeSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CMcpJobBridgeSettings loaded( *this );
    loaded.defaultWaitMs = config.GetAttributeValueOrChildValueByName( "defaultWaitMs" ).AsUInt32( defaultWaitMs, true );
    loaded.maxWaitMs = config.GetAttributeValueOrChildValueByName( "maxWaitMs" ).AsUInt32( maxWaitMs, true );
    loaded.pollIntervalMs = config.GetAttributeValueOrChildValueByName( "pollIntervalMs" ).AsUInt32( pollIntervalMs, true );
    loaded.resultTtlMs = config.GetAttributeValueOrChildValueByName( "resultTtlMs" ).AsUInt64( resultTtlMs, true );
    loaded.requestKeyTtlMs = config.GetAttributeValueOrChildValueByName( "requestKeyTtlMs" ).AsUInt64( requestKeyTtlMs, true );
    loaded.maxRequestKeysPerPrincipal = config.GetAttributeValueOrChildValueByName( "maxRequestKeysPerPrincipal" ).AsUInt32( maxRequestKeysPerPrincipal, true );
    loaded.maxRequestKeysTotal = config.GetAttributeValueOrChildValueByName( "maxRequestKeysTotal" ).AsUInt32( maxRequestKeysTotal, true );
    loaded.maxActivePerPrincipal = config.GetAttributeValueOrChildValueByName( "maxActivePerPrincipal" ).AsUInt32( maxActivePerPrincipal, true );
    loaded.maxJobs = config.GetAttributeValueOrChildValueByName( "maxJobs" ).AsUInt32( maxJobs, true );
    loaded.operatorScope = config.GetAttributeValueOrChildValueByName( "operatorScope" ).AsString( operatorScope, true );
    if ( 0 == loaded.pollIntervalMs || loaded.pollIntervalMs > 10000 || loaded.defaultWaitMs > loaded.maxWaitMs ||
         0 == loaded.maxActivePerPrincipal || 0 == loaded.maxJobs || 0 == loaded.resultTtlMs )
        return false;
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
