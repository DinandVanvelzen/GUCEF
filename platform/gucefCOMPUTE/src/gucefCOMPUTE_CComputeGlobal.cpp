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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "gucefCOMPUTE_CComputeGlobal.h"

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

static CComputeGlobal* g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

CComputeGlobal*
CComputeGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        // Ensure the Core systems are initialized first since this module depends on Core
        CORE::CCoreGlobal::Instance();

        g_instance = GUCEF_NEW CComputeGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CComputeGlobal::Deinstance( void )
{GUCEF_TRACE;

    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CComputeGlobal::CComputeGlobal( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeGlobal::~CComputeGlobal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CComputeGlobal::Initialize( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CComputeGlobal::Update( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
