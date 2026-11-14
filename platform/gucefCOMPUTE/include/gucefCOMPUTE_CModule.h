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

#ifndef GUCEF_COMPUTE_CMODULE_H
#define GUCEF_COMPUTE_CMODULE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"         /* often used gucefCOMPUTE macros */
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Entry points that the shared module initialisation calls when the library is loaded and unloaded.
 */
class GUCEF_COMPUTE_PRIVATE_CPP CModule
{
    public:

    /** Loading more than once is harmless: the work is done the first time */
    static bool Load( void );

    static bool Unload( void );

    private:

    CModule( void );
    CModule( const CModule& src );
    ~CModule();
    CModule& operator=( const CModule& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Probe for the tests: true once CModule::Load() has run and CModule::Unload() has not.
 *  Being exported and callable from another module is also the proof that the export specification of the module works.
 */
GUCEF_COMPUTE_PUBLIC_CPP bool
ComputeModuleLoadProbe( void );

/**
 *  Probe for the tests: true when the export specification of the module expands, inside the build of the module,
 *  to the one that exports symbols.
 */
GUCEF_COMPUTE_PUBLIC_CPP bool
ComputeModulePublicSpecProbe( void );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CMODULE_H ? */
