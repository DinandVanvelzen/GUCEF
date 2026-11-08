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

#ifndef GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#define GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

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
 *  The contracts built into gucefCOMCORE (gucefCOMCORE RPC design 3.6): gucef.rpc 1.0, which holds the reflection
 *  service (11), the meta-types of the canonical schema encoding (3.8) and the documentation message. Built in code,
 *  so it needs no JSON codec; created on first use, thread safe.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcBuiltInContracts
{
    public:

    static const char* RpcContractName;         /**< "gucef.rpc" */

    /** Type and service names of gucef.rpc */
    static const char* ContractSchemaType;      /**< "gucef.rpc.ContractSchema" */
    static const char* ContractDocsType;        /**< "gucef.rpc.ContractDocs" */
    static const char* ReflectionService;       /**< "gucef.rpc.Reflection" */

    /** gucef.rpc 1.0 */
    static CRpcContractPtr GetRpcContract( void );

    /** The authoring form of gucef.rpc 1.0 (the same as the contract's SaveToDataNode()) */
    static void GetRpcContractSource( CORE::CDataNode& source );

    private:

    CRpcBuiltInContracts( void );       /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H ? */
