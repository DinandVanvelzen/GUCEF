/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CRemoteServiceConnectionSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteServiceConnectionSettings::CRemoteServiceConnectionSettings( void )
    : clientKind( "client" )
    , clientVersion()
    , capabilities()
    , wantedContracts()
    , bulkContracts()
    , bulkChannels( 0 )
    , hostOverride()
    , connectTimeoutMs( 20000 )
    , linkTimeoutMs( 15000 )
    , echoIntervalMs( 1000 )
    , resumeGraceMs( 600000 )
    , controlWindowBytes( 1024 * 1024 )
    , controlMaxMessageBytes( 4 * 1024 * 1024 )
    , controlSendQueueBytes( 4 * 1024 * 1024 )
    , bulkWindowBytes( 8 * 1024 * 1024 )
    , bulkMaxMessageBytes( 1024 * 1024 )
    , bulkSendQueueBytes( 16 * 1024 * 1024 )
    , autoReconnect( true )
    , reconnectFirstMaxMs( 250 )
    , reconnectBaseMs( 500 )
    , reconnectMaxMs( 30000 )
    , reconnectStableResetMs( 60000 )
    , wakeDetectMs( 5000 )
    , clockSkewNoticeMs( 2000 )
    , openEventStream( true )
    , tickIntervalMs( 10 )
    , lanDiscovery( true )
    , lanDiscoveryFinder()
{GUCEF_TRACE;

    lanDiscoveryFinder.serviceType = "_gucef-service._tcp";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
