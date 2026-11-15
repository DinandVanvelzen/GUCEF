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

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#define GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

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
 *  The contract gucef.drnutils.session 1.0 of the remote service kit (gucefASSETSTORE_RemoteNetworking_Design.md 4.2), built in
 *  code with COMCORE::CRpcContractBuilder. The authoring source is platform/gucefDRNUTILS/rpc/gucef.drnutils.session.json;
 *  a test checks that the canonical hash of the built contract equals that of the JSON file. The Event type has the field
 *  numbers of CRemoteSessionEventRing (1 eventSeq, 2 kind, 3 atUtcMs)
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteSessionContracts
{
    public:

    static const char* ContractName;    /**< "gucef.drnutils.session" */
    static const UInt32 Major = 1;
    static const char* HelloPath;           /**< "gucef.drnutils.session.Session/Hello" */
    static const char* ResumePath;          /**< ".../Resume" */
    static const char* AttachPath;          /**< ".../Attach" */
    static const char* EventsPath;          /**< ".../Events" */
    static const char* ResyncPath;          /**< ".../Resync" */
    static const char* GoodbyePath;         /**< ".../Goodbye" */
    static const char* RenewTokenPath;      /**< ".../RenewToken" */
    static const char* ServiceName;         /**< "Session" */
    static const char* EventKindNotice;           /**< "NOTICE": data { level, code, text } */
    static const char* EventKindServiceStopping;  /**< "SERVICE_STOPPING": data { reason, expectedBackSeconds, retryAfterMs } */

    /** gucef.drnutils.session 1.0, built once and registered in the contract registry of gucefCOMCORE; NULL when building failed (logged) */
    static COMCORE::CRpcContractPtr GetContract( void );

    /** The authoring form of the contract */
    static void GetContractSource( CORE::CDataNode& source );

    private:

    CRemoteSessionContracts( void );       /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H ? */
