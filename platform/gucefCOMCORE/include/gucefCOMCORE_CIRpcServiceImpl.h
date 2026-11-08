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

#ifndef GUCEF_COMCORE_CIRPCSERVICEIMPL_H
#define GUCEF_COMCORE_CIRPCSERVICEIMPL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_RPCTYPES_H
#include "gucefCOMCORE_RpcTypes.h"
#define GUCEF_COMCORE_RPCTYPES_H
#endif /* GUCEF_COMCORE_RPCTYPES_H ? */

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

class CRpcServerCall;
class CRpcMethodDescriptor;

/*-------------------------------------------------------------------------*/

/** The implementation of a service (gucefCOMCORE RPC design 10.6): gets the calls of every method of the service */
class GUCEF_COMCORE_EXPORT_CPP CIRpcServiceImpl
{
    public:

    virtual ~CIRpcServiceImpl();

    virtual TRpcHandlerResult OnCall( CRpcServerCall& call ) = 0;

    /** An execution mode for one method that overrides the one the service was registered with; false = none */
    virtual bool GetMethodExecutionMode( const CRpcMethodDescriptor& method, TRpcExecutionMode& mode ) const;

    /** False when the method has no handler: the endpoint answers UNIMPLEMENTED without calling OnCall() */
    virtual bool HasHandler( const CRpcMethodDescriptor& method ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIRPCSERVICEIMPL_H ? */
