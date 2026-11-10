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

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#define GUCEF_COMCORE_CRPCCALLRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

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
 *  The outcome of an asynchronous call: the result data of the future CRpcEndpoint::CallAsync() returns
 *  (gucefCOMCORE RPC design 10.3, 10.9). Every outcome, local errors included, completes the future with one.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcCallResult : public CORE::CICloneable
{
    public:

    CRpcStatus status;
    CRpcMessage response;           /**< bound to the response type when the call answered OK */
    CRpcRequestId requestId;        /**< the id of ACCEPTED, when there was one */
    bool wasAccepted;               /**< the provider answered ACCEPTED, the result came later */
    bool isDurable;

    CRpcCallResult( void );
    CRpcCallResult( const CRpcCallResult& src );
    virtual ~CRpcCallResult() GUCEF_VIRTUAL_OVERRIDE;
    CRpcCallResult& operator=( const CRpcCallResult& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */
