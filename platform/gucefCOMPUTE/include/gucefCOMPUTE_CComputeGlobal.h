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

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
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

class CModule;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to all global compute systems.
 *  The backend, kernel and settings registries are added here as they come into existence.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeGlobal
{
    public:

    static CComputeGlobal* Instance( void );

    /**
     *  Gives the module the opportunity to process pending completions on platforms that need the application to
     *  drive them. Nothing is pending until the backends exist.
     */
    void Update( void );

    private:
    friend class CModule;

    static void Deinstance( void );

    private:

    CComputeGlobal( void );

    ~CComputeGlobal();

    /** Creates the owned global systems in their dependency order, after the instance pointer is set */
    void Initialize( void );

    CComputeGlobal( const CComputeGlobal& src );            /**< not implemented */
    CComputeGlobal& operator=( const CComputeGlobal& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */
