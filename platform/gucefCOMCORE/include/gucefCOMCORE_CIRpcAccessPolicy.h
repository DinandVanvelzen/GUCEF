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

#ifndef GUCEF_COMCORE_CIRPCACCESSPOLICY_H
#define GUCEF_COMCORE_CIRPCACCESSPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#include "gucefCOMCORE_CRpcPrincipal.h"
#define GUCEF_COMCORE_CRPCPRINCIPAL_H
#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COMCORE_CRPCMETADATA_H
#include "gucefCOMCORE_CRpcMetadata.h"
#define GUCEF_COMCORE_CRPCMETADATA_H
#endif /* GUCEF_COMCORE_CRPCMETADATA_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

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
 *  Decides what a principal may do (gucefCOMCORE RPC design 10.7, 14.2): bind a contract (and see it through the
 *  handshake offers, CONTRACT and reflection), call a method, query or cancel accepted work of a request id.
 *  Called on the protocol thread; implementations are thread safe.
 */
class GUCEF_COMCORE_EXPORT_CPP CIRpcAccessPolicy
{
    public:

    virtual ~CIRpcAccessPolicy();

    virtual bool MayBindContract( const CRpcPrincipal& principal, const CRpcContract& contract ) const = 0;

    virtual TRpcAccessDecision AuthorizeCall( const CRpcPrincipal& principal, const CRpcContract& contract,
                                              const CRpcServiceDescriptor& service, const CRpcMethodDescriptor& method,
                                              const CRpcMetadata& metadata, CORE::CString& reason ) const = 0;

    /** DURABLE ids of another subject: false keeps them invisible (STATUS UNKNOWN) */
    virtual bool MayQueryRequest( const CRpcPrincipal& principal, const CORE::CString& ownerSubject, const CRpcRequestId& id ) const = 0;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIRpcAccessPolicy, MT::CMutex > CIRpcAccessPolicyPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIRPCACCESSPOLICY_H ? */
