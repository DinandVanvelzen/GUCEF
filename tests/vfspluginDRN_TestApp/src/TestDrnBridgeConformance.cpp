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

#include "TestArchiveConformance.h"
#include "TestDrnBridgeFixture.h"
#include "TestDrnBridgeConformance.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDrnBridgeConformanceTests( bool withCache, bool withoutCache, bool withPumpedClient )
{GUCEF_TRACE;

    if ( withCache )
    {
        // a long time to live for both: a change that the cache is not told about would not show, the fixture tells it as a change notification would
        DRNBRIDGETEST::CBridgeMountOptions options;
        options.route = DRNBRIDGETEST::MOUNTROUTE_URI;
        options.metadataTtlMs = 60000;
        options.negativeTtlMs = 60000;
        options.invalidateOnChange = true;
        options.shareKnowsDigests = false;
        DRNBRIDGETEST::CDrnBridgeConformanceFixture fixture( "DRN bridge: drn:// source, metadata cache on", options );
        PerformArchiveConformanceSuite( fixture );
    }
    if ( withoutCache )
    {
        // no cache: every call asks, nothing is told to the archive; the share knows the digests of its files
        DRNBRIDGETEST::CBridgeMountOptions options;
        options.route = DRNBRIDGETEST::MOUNTROUTE_REGISTERED;
        options.metadataTtlMs = 0;
        options.negativeTtlMs = 0;
        options.invalidateOnChange = false;
        options.shareKnowsDigests = true;
        DRNBRIDGETEST::CDrnBridgeConformanceFixture fixture( "DRN bridge: registered connection, no metadata cache, share with digests", options );
        PerformArchiveConformanceSuite( fixture );
    }
    if ( withPumpedClient )
    {
        // the client node has no thread of its own: the thread of the suite pumps it while the calls of the archive wait for the share
        DRNBRIDGETEST::CBridgeMountOptions options;
        options.route = DRNBRIDGETEST::MOUNTROUTE_URI;
        options.metadataTtlMs = 60000;
        options.negativeTtlMs = 60000;
        options.invalidateOnChange = true;
        options.shareKnowsDigests = false;
        options.clientOwnThread = false;
        DRNBRIDGETEST::CDrnBridgeConformanceFixture fixture( "DRN bridge: drn:// source, metadata cache on, client node pumped by the calling thread", options );
        PerformArchiveConformanceSuite( fixture );
    }
}

/*-------------------------------------------------------------------------*/
