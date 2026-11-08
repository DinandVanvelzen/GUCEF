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

#ifndef GUCEF_COMCORE_CRPCREQUESTSTATUS_H
#define GUCEF_COMCORE_CRPCREQUESTSTATUS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

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
 *  The answer to QueryStatus() / CancelRequest() for accepted work (gucefCOMCORE RPC design 6.7): the STATUS of the
 *  provider and, when RESULT was asked again and the work is done, the result. Result data of their futures.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcRequestStatus : public CORE::CICloneable
{
    public:

    CRpcRequestId requestId;
    TRpcRequestState state;
    UInt32 progressPermille;        /**< 0 - 1000, 1001 = unknown */
    UInt32 etaMs;
    CORE::CString note;
    CRpcStatus status;              /**< of the exchange: OK when STATUS arrived */
    bool hasResult;
    CRpcCallResult result;          /**< the RESULT sent again (resendResultWhenDone): its status */
    CORE::CDynamicBuffer resultPayload;     /**< and its response message, encoded: decode with the method's response type */

    CRpcRequestStatus( void );
    CRpcRequestStatus( const CRpcRequestStatus& src );
    virtual ~CRpcRequestStatus() GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_COMCORE_CRPCREQUESTSTATUS_H ? */
