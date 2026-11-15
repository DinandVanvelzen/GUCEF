/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCRELEASECOURIERREGISTRY_H
#define GUCEF_COMCORE_CRPCRELEASECOURIERREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COMCORE_CRPCRELEASECOURIER_H
#include "gucefCOMCORE_CRpcReleaseCourier.h"
#define GUCEF_COMCORE_CRPCRELEASECOURIER_H
#endif /* GUCEF_COMCORE_CRPCRELEASECOURIER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The release couriers of gucefCOMCORE, one per pulse generator (CComCoreGlobal::GetRpcReleaseCourierRegistry()).
 *  An endpoint gets the courier of its notification pulse generator when it is made, on the thread that makes it, so
 *  that the courier is linked to that pulse generator there and never later on another thread (gucefCOMCORE RPC design
 *  10.9). A courier that no endpoint uses any more and that holds nothing goes when another courier is asked for, or
 *  with the registry.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcReleaseCourierRegistry
{
    public:

    CRpcReleaseCourierRegistry( void );

    ~CRpcReleaseCourierRegistry();

    /** The courier of the pulse generator, made when there is none; NULL for a NULL pulse generator */
    CRpcReleaseCourierPtr GetCourier( const CORE::PulseGeneratorPtr& pulseGenerator );

    /** The couriers kept */
    UInt32 GetCourierCount( void ) const;

    private:

    CRpcReleaseCourierRegistry( const CRpcReleaseCourierRegistry& src );                /**< not implemented */
    CRpcReleaseCourierRegistry& operator=( const CRpcReleaseCourierRegistry& src );     /**< not implemented */

    typedef GUCEF::map< const CORE::CPulseGenerator*, CRpcReleaseCourierPtr > TCourierMap;

    mutable MT::CMutex m_lock;
    TCourierMap m_couriers;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCRELEASECOURIERREGISTRY_H ? */
