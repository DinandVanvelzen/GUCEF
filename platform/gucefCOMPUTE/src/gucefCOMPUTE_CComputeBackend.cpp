/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#include "gucefCOMPUTE_CComputeBackend.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CComputeBackend::CComputeBackend( void )
    : m_available( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeBackend::~CComputeBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CComputeBackend::GetDescription( void ) const
{GUCEF_TRACE;

    return GetName();
}

/*-------------------------------------------------------------------------*/

Int32
CComputeBackend::GetPriority( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

void
CComputeBackend::Update( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CComputeBackend::IsAvailable( void ) const
{GUCEF_TRACE;

    return 0 != m_available.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CComputeBackend::SetAvailable( bool available )
{GUCEF_TRACE;

    m_available.Store( available ? 1 : 0, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
