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

#ifndef GUCEF_COMCORE_CRPCREFLECTIONSERVICE_H
#define GUCEF_COMCORE_CRPCREFLECTIONSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CIRPCSERVICEIMPL_H
#include "gucefCOMCORE_CIRpcServiceImpl.h"
#define GUCEF_COMCORE_CIRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CIRPCSERVICEIMPL_H ? */

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
 *  The reflection service gucef.rpc.Reflection (gucefCOMCORE RPC design 11), bound implicitly at index 0 in every
 *  session and served by the endpoint when its settings enable it: ListContracts lists the contracts this side
 *  exports, GetContract returns the canonical schema bytes (the encoding CONTRACT uses). Every contract it reveals
 *  passes the access policy's MayBindContract().
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcReflectionService : public CIRpcServiceImpl
{
    public:

    CRpcReflectionService( void );
    virtual ~CRpcReflectionService() GUCEF_VIRTUAL_OVERRIDE;

    virtual TRpcHandlerResult OnCall( CRpcServerCall& call ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCREFLECTIONSERVICE_H ? */
