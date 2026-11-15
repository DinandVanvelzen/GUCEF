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

#ifndef GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H
#define GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#include "gucefCOMCORE_CRpcPrincipal.h"
#define GUCEF_COMCORE_CRPCPRINCIPAL_H
#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */

#ifndef GUCEF_DRNUTILS_DRNUTILSTYPES_H
#include "gucefDRNUTILS_DrnUtilsTypes.h"
#define GUCEF_DRNUTILS_DRNUTILSTYPES_H
#endif /* GUCEF_DRNUTILS_DRNUTILSTYPES_H ? */

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
 *  Module-private: what both hosts share on their control paths: clocks, the secure transport test, role sets,
 *  command ids and status codes.
 */
class CDrnUtilsHostSupport
{
    public:

    /** Principal attribute under which a host passes the RPC session (hex) of a call to its audit record */
    static const char* SessionIdAttribute;

    static const UInt32 MaxCommandIdBytes = 128;

    static UInt64 GetMonotonicMs( void );
    static Int64 GetUtcMs( void );

    /**
     *  Whether the principal came over a transport that protects every frame: "loopback" and "local" (in process),
     *  or a transport that says so in the principal's attributes ("<transport>.protected" true, as the drn transport
     *  does for an encrypted link)
     */
    static bool IsSecure( const COMCORE::CRpcPrincipal& principal );

    /** Whether one of the roles is in the set; an empty set answers emptySetAllows */
    static bool HasAnyRole( const CORE::CString::StringSet& principalRoles, const CORE::CString::StringSet& roles, bool emptySetAllows );

    /** INVALID_ARGUMENT for an empty id (unless a dry run) or one above MaxCommandIdBytes */
    static TDrnUtilsCommandStatus CheckCommandId( const CORE::CString& commandId, bool dryRun, CORE::CString& problem );

    /** The command id of a request, or the metadata "idempotency-key" when the field is empty; false when both differ */
    static bool GetCommandId( const COMCORE::CRpcServerCall& call, const CORE::CString& fieldValue, CORE::CString& commandId, CORE::CString& problem );

    /** The RPC session of the call as hex, empty when the call has none */
    static CORE::CString GetSessionKey( const COMCORE::CRpcServerCall& call );

    static COMCORE::TRpcStatusCode ToRpcStatusCode( TDrnUtilsCommandStatus status );
    static TDrnUtilsCommandStatus FromRpcStatusCode( COMCORE::TRpcStatusCode code );

    static CORE::CString JoinMessages( const CORE::CString::StringVector& messages );

    private:
    CDrnUtilsHostSupport( void );   /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H ? */
