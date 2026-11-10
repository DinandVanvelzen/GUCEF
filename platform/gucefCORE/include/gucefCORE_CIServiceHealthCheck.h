/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CISERVICEHEALTHCHECK_H
#define GUCEF_CORE_CISERVICEHEALTHCHECK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The outcome of a health check, ordered from good to bad */
enum EServiceHealthState
{
    HEALTH_PASS = 0 ,   /**< healthy */
    HEALTH_WARN     ,   /**< degraded but serving: counts as passing */
    HEALTH_FAIL         /**< not healthy */
};
typedef enum EServiceHealthState TServiceHealthState;

/** "pass", "warn" or "fail", the words of the health endpoint bodies */
GUCEF_CORE_PUBLIC_CPP const CString&
ServiceHealthStateToString( TServiceHealthState state );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One readiness check of a service, registered with a CServiceHealthRegistry: a dependency that has to be there
 *  for the service to take work (a database, a lock, a peer). The registry calls Check() on the thread that drives
 *  its pulse generator, at its check interval, and caches the outcome: the health endpoints never call it.
 *  Check() must be cheap and must not block (no network round trip, no waiting for a lock that is held across I/O):
 *  a check that blocks holds up the pulse thread, which then shows as a failed liveness. A check that throws is
 *  reported as failing.
 */
class GUCEF_CORE_PUBLIC_CPP CIServiceHealthCheck
{
    public:

    CIServiceHealthCheck( void );

    virtual ~CIServiceHealthCheck();

    /** A short, stable name for the endpoint body, such as "database" */
    virtual const CString& GetName( void ) const = 0;

    /**
     *  @param detail   a short human readable explanation, mostly for WARN and FAIL. It is shown by the health
     *                  endpoints after redaction: no secrets, no configuration dumps
     */
    virtual TServiceHealthState Check( CString& detail ) = 0;

    private:

    CIServiceHealthCheck( const CIServiceHealthCheck& src );            /**< not implemented */
    CIServiceHealthCheck& operator=( const CIServiceHealthCheck& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CISERVICEHEALTHCHECK_H ? */
