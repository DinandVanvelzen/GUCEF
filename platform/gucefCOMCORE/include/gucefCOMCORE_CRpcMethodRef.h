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

#ifndef GUCEF_COMCORE_CRPCMETHODREF_H
#define GUCEF_COMCORE_CRPCMETHODREF_H

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
 *  A method resolved once against a bound contract (gucefCOMCORE RPC design 10.3): CRpcEndpoint::ResolveMethod()
 *  fills it and every call reuses it, so a call does no name lookup. Valid for the session that resolved it; after
 *  RESUME_GAP / SESSION_UNKNOWN the endpoint rebinds and a ref resolves again by the same path.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcMethodRef
{
    public:

    CRpcContractPtr contract;                       /**< the contract this side encodes with */
    const CRpcMethodDescriptor* method;
    UInt32 bindIndex;
    UInt32 serviceNumber;
    UInt32 methodNumber;
    UInt32 bindGeneration;                          /**< which binding round of the endpoint */
    CORE::CString path;

    CRpcMethodRef( void );

    bool IsValid( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCMETHODREF_H ? */
