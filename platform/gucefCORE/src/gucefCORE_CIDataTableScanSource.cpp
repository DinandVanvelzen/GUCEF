/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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


#include "gucefCORE_CIDataTableScanSource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableScanFile::CDataTableScanFile( void )
    : path()
    , fileOrdinal( 0 )
    , partitionKeys()
    , fileSize( GUCEF_UINT64MAX )
    , modificationTime()
    , metaData()
    , sharedFileLock( GUCEF_NULL )
    , backendData( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDataTableScanFile::Clear( void )
{GUCEF_TRACE;

    path.Clear();
    fileOrdinal = 0;
    partitionKeys.Clear();
    fileSize = GUCEF_UINT64MAX;
    modificationTime = CDateTime();
    metaData.Unlink();
    sharedFileLock = GUCEF_NULL;
    backendData = 0;
}

/*-------------------------------------------------------------------------*/

CIDataTableScanSource::CIDataTableScanSource( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDataTableScanSource::~CIDataTableScanSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIDataTableScanSource::AddSourceStats( CDataTableScanStats& stats ) const
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
