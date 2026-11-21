/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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
#include "TestRemoteArchiveFixture.h"
#include "TestRemoteArchiveConformance.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static CRemoteEnv*
MakeMemoryPlaceEnv( const char* name )
{
    return GUCEF_NEW CRemoteTestEnv( name );
}

/*-------------------------------------------------------------------------*/

void
PerformRemoteArchiveConformanceTests( void )
{
    PerformRemoteArchiveCasesOn( &MakeMemoryPlaceEnv, "RemoteArchiveConformance" );

    // the cases of the archive conformance suite that the file system archive passes, on the same archive type
    CRemoteArchiveConformanceFixture fixture;
    PerformArchiveConformanceSuite( fixture );
}

/*-------------------------------------------------------------------------*/
