/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#define GUCEF_MATH_CMATHGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CModule;
class CMathBatchDispatcher;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to all global math systems.
 *  Every module-wide object of this module is owned here, so creation and initialisation order is explicit.
 */
class GUCEF_MATH_PUBLIC_CPP CMathGlobal
{
    public:

    /**
     *  The instance, created on first use. Thread-safe: when several threads make the first call together, one creates
     *  and initialises the instance and the others wait until it is complete. After that a call is one acquire load
     *  (lock-free, realtime-safe).
     */
    static CMathGlobal* Instance( void );

    /**
     *  The batch dispatch table that the batch functions (gucefMATH_BatchFunctions.h) use, created with the providers of
     *  this module registered (scalar, and the best SIMD set for the CPU). Bridge plugins register their providers
     *  here. Realtime-safe once Instance() has been called.
     */
    CMathBatchDispatcher& GetBatchDispatcher( void ) GUCEF_MATH_REALTIME_CONTEXT;


    private:
    friend class CModule;

    static void Deinstance( void );

    /** The slow path of Instance(): one thread creates, the others wait for the publication */
    static CMathGlobal* CreateInstance( void );

    private:

    CMathGlobal( void );

    ~CMathGlobal();

    /** Creates the owned global systems in their dependency order, after the instance pointer is set */
    void Initialize( void );

    CMathGlobal( const CMathGlobal& src );            /**< not implemented */
    CMathGlobal& operator=( const CMathGlobal& src ); /**< not implemented */

    private:

    CMathBatchDispatcher* m_batchDispatcher;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */
