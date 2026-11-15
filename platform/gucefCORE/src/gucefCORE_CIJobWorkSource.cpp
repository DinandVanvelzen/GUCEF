/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CIJobWorkSource.h"

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

CIJobWorkSource::CIJobWorkSource( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIJobWorkSource::~CIJobWorkSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIJobWorkSource::OnRunNow( const CString& laneName, Int64 nowUtcMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIJobWorkSource::SaveState( CDataNode& state ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIJobWorkSource::LoadState( const CDataNode& state, CString& error )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CIJobWorkSource::GetCustomActions( TJobCustomActionVector& actions ) const
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TJobControlStatus
CIJobWorkSource::ApplyCustomAction( const SJobControlCommand& command, CDataNode& detail, CString& message )
{GUCEF_TRACE;

    message = "the work source declares no custom actions";
    return JOBCONTROLSTATUS_UNIMPLEMENTED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
