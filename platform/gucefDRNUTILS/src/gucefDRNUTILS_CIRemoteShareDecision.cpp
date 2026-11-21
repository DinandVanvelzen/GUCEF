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

#include "gucefDRNUTILS_CIRemoteShareDecision.h"

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

CIRemoteShareDecision::CIRemoteShareDecision( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIRemoteShareDecision::~CIRemoteShareDecision()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIRemoteShareDecision::MayRead( const CDeviceRecord& /* device */, const CORE::CString& /* share */, const CORE::CString& /* name */, CORE::CString& /* refusal */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIRemoteShareDecision::CheckRead( const CDeviceRecord& device, const CORE::CString& share, const CORE::CString& name, CORE::CString& refusal, bool& retryLater )
{GUCEF_TRACE;

    retryLater = false;
    return MayRead( device, share, name, refusal );
}

/*-------------------------------------------------------------------------*/

bool
CIRemoteShareDecision::MayBrowse( const CDeviceRecord& /* device */, const CORE::CString& /* share */, const CORE::CString& /* name */ )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIRemoteShareDecision::MayListEntry( const CDeviceRecord& /* device */, const CORE::CString& /* share */, const CORE::CString& /* name */, bool /* isDirectory */ )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
