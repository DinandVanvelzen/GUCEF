/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CSERVICEADMINCONTRACTS_H
#define GUCEF_COM_CSERVICEADMINCONTRACTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The contract gucef.service.admin 1.0 of the loopback admin endpoint of a service (CServiceAdminEndpoint), built in
 *  code with COMCORE::CRpcContractBuilder:
 *      type AdminRequest  { 1 token bytes (sensitive), 2 command string, 3 arguments map<string,string> }
 *      type AdminResponse { 1 result datanode, 2 message string }
 *      service Admin (1): Run (1) unary AdminRequest -> AdminResponse, sensitive, CONFIDENTIAL
 *  Failures travel as RPC status: UNAUTHENTICATED (no or a wrong token), NOT_FOUND (an unknown command),
 *  INVALID_ARGUMENT, UNIMPLEMENTED (a command of the service's design it does not have yet), FAILED_PRECONDITION
 *  (understood, did not succeed), each with the handler's message.
 */
class GUCEF_COM_PUBLIC_CPP CServiceAdminContracts
{
    public:

    static const char* ContractName;    /**< "gucef.service.admin" */
    static const char* ServiceName;     /**< "Admin" */
    static const char* RunMethodPath;   /**< "gucef.service.admin.Admin/Run" */

    /** gucef.service.admin 1.0, built once; NULL when building failed (logged) */
    static COMCORE::CRpcContractPtr GetContract( void );

    private:

    CServiceAdminContracts( void );     /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSERVICEADMINCONTRACTS_H ? */
