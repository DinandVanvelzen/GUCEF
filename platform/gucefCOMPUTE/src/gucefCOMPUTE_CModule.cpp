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

#include <string.h>

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#include "gucefCOMPUTE_CModule.h"

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

#define GUCEF_COMPUTE_STRINGIFY2( x ) #x
#define GUCEF_COMPUTE_STRINGIFY( x ) GUCEF_COMPUTE_STRINGIFY2( x )

/*-------------------------------------------------------------------------*/

static bool g_moduleLoaded = false;

/*-------------------------------------------------------------------------*/

bool
CModule::Load( void )
{
    g_moduleLoaded = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CModule::Unload( void )
{
    g_moduleLoaded = false;
    CComputeGlobal::Deinstance();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
ComputeModuleLoadProbe( void )
{
    return g_moduleLoaded;
}

/*-------------------------------------------------------------------------*/

bool
ComputeModulePublicSpecProbe( void )
{
    #ifdef GUCEF_COMPUTE_BUILD_MODULE
    return 0 == strcmp( GUCEF_COMPUTE_STRINGIFY( GUCEF_COMPUTE_PUBLIC_CPP ), GUCEF_COMPUTE_STRINGIFY( GUCEF_EXPORT ) );
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
