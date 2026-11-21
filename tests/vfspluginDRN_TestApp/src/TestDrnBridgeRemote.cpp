/*
 *  vfspluginDRN_TestApp: tests of the DRN VFS plugin
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRemoteArchiveConformance.h"
#include "TestDrnRemoteEnv.h"
#include "TestDrnBridgeRemote.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDrnBridgeRemoteTests( bool loopback, bool proxied )
{GUCEF_TRACE;

    if ( loopback )
        PerformRemoteArchiveCasesOn( &DRNBRIDGETEST::MakeLoopbackRemoteEnv, "DrnBridgeRemoteLoopback" );
    if ( proxied )
        PerformRemoteArchiveCasesOn( &DRNBRIDGETEST::MakeProxiedRemoteEnv, "DrnBridgeRemoteProxied" );
}

/*-------------------------------------------------------------------------*/
