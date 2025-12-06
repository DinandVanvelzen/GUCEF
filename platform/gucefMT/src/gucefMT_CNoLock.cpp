/*
 *  gucefMT: GUCEF module providing multithreading solutions
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefMT_CNoLock.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNoLock::CNoLock( void )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CNoLock::CNoLock( CNoLock&& src ) GUCEF_NOEXCEPT
{GUCEF_TRACE;

}

#endif

/*--------------------------------------------------------------------------*/

CNoLock::~CNoLock()
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

TLockStatus
CNoLock::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return LOCKSTATUS_NOT_APPLICABLE;
}

/*--------------------------------------------------------------------------*/

TLockStatus
CNoLock::Unlock( void ) const
{GUCEF_TRACE;

    return LOCKSTATUS_NOT_APPLICABLE;
}

/*--------------------------------------------------------------------------*/

const CILockable* 
CNoLock::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
