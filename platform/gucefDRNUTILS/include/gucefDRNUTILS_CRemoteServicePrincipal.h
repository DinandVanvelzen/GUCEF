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

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEPRINCIPAL_H
#define GUCEF_DRNUTILS_CREMOTESERVICEPRINCIPAL_H

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

/**
 *  Who makes a call on a remote service host (CRemoteServiceHost::GetCallPrincipal()): the device as the registry has it
 *  now (its roles and kind, never those of its token), the member link the call arrived over, and the client session
 *  the call's RPC session is bound to (empty before Session/Hello or Session/Resume, and for RPC sessions without one)
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteServicePrincipal
{
    public:

    CORE::CString deviceId;
    CORE::CString userId;
    CORE::CString deviceName;
    CORE::CString deviceKind;
    CORE::CString::StringSet roles;
    CORE::CString subject;                  /**< the RPC principal's subject ("drn:<deviceId>") */
    CORE::CString address;                  /**< host of the member link, without the port */
    UInt32 peerId;                          /**< the DRN member of the link the RPC session runs on now */
    CORE::CString sessionId;                /**< the client session, empty for none */
    CORE::CString channel;                  /**< "control" for the RPC session that opened the client session, "bulk" for an attached one */

    CRemoteServicePrincipal( void )
        : deviceId()
        , userId()
        , deviceName()
        , deviceKind()
        , roles()
        , subject()
        , address()
        , peerId( 0 )
        , sessionId()
        , channel()
    {}

    bool HasRole( const CORE::CString& role ) const
    {
        return roles.find( role ) != roles.end();
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESERVICEPRINCIPAL_H ? */
