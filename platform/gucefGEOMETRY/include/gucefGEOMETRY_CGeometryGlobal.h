/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#define GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {
class CMathBatchDispatcher;
}
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CModule;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to all global geometry systems.
 *  Every module-wide object of this module is owned here, so creation and initialisation order is explicit.
 */
class GUCEF_GEOMETRY_PUBLIC_CPP CGeometryGlobal
{
    public:

    /**
     *  The instance, created on first use after the global classes of gucefCORE and gucefMATH. Thread-safe: when
     *  several threads make the first call together, one creates and initialises the instance and the others wait until
     *  it is complete. After that a call is one acquire load (lock-free, realtime-safe).
     */
    static CGeometryGlobal* Instance( void );

    /**
     *  True when the batch kernels of this module (BatchFunctions.h) were registered with the dispatcher of gucefMATH by
     *  Initialize(); they are registered under this object as owner and removed again when it is destroyed.
     */
    bool AreBatchKernelsRegistered( void ) const;

    private:
    friend class CModule;

    static void Deinstance( void );

    /** The slow path of Instance(): one thread creates, the others wait for the publication */
    static CGeometryGlobal* CreateInstance( void );

    private:

    CGeometryGlobal( void );

    ~CGeometryGlobal();

    /** Creates the owned global systems in their dependency order, after the instance pointer is set */
    void Initialize( void );

    CGeometryGlobal( const CGeometryGlobal& src );            /**< not implemented */
    CGeometryGlobal& operator=( const CGeometryGlobal& src ); /**< not implemented */

    private:

    MATH::CMathBatchDispatcher* m_batchDispatcher;      /**< the dispatcher the batch kernels are registered with, NULL when not registered */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H ? */
