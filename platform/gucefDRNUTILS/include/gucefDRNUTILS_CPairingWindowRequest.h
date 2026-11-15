/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CPAIRINGWINDOWREQUEST_H
#define GUCEF_DRNUTILS_CPAIRINGWINDOWREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a pairing window grants the device that pairs through it (CDevicePairingHost::Open()) */
class GUCEF_DRNUTILS_PUBLIC_CPP CPairingWindowRequest
{
    public:

    CORE::CString deviceKind;               /**< service defined ("viewer", "worker", "relay", "cli") */
    CORE::CString::StringSet roles;
    UInt32 seconds;                         /**< 0: the host's default; at most the host's maximum */
    UInt32 count;                           /**< pairings allowed through the window, default 1 */
    bool hasRequireConfirmation;            /**< false: the host's rule decides (confirmation for privileged roles) */
    bool requireConfirmation;

    CPairingWindowRequest( void )
        : deviceKind()
        , roles()
        , seconds( 0 )
        , count( 1 )
        , hasRequireConfirmation( false )
        , requireConfirmation( false )
    {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CPAIRINGWINDOWREQUEST_H ? */
