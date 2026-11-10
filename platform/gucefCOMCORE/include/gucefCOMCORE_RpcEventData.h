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

#ifndef GUCEF_COMCORE_RPCEVENTDATA_H
#define GUCEF_COMCORE_RPCEVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTHASH_H
#include "gucefCOMCORE_CRpcContractHash.h"
#define GUCEF_COMCORE_CRPCCONTRACTHASH_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTHASH_H ? */

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

/*
 *  Event data of the RPC endpoint, streams and server calls (gucefCOMCORE RPC design 10.3, 10.4). Small value
 *  classes, kept together.
 */

/** CRpcEndpoint::AcceptedEvent: the provider answered ACCEPTED */
class GUCEF_COMCORE_EXPORT_CPP CRpcAcceptedEventData : public CORE::CICloneable
{
    public:

    UInt64 callRef;
    CORE::CString methodPath;
    CRpcRequestId requestId;
    bool isDurable;
    UInt32 retryAfterMs;
    UInt32 etaMs;

    CRpcAcceptedEventData( void );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** CRpcEndpoint::ContractBoundEvent: a contract this side calls was bound (or refused) */
class GUCEF_COMCORE_EXPORT_CPP CRpcBindingEventData : public CORE::CICloneable
{
    public:

    CORE::CString contractName;
    UInt32 major;
    UInt32 localMinor;
    UInt32 providerMinor;
    UInt32 bindIndex;
    CRpcContractHash providerHash;
    CRpcStatus status;              /**< OK when bound */

    CRpcBindingEventData( void );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** CRpcEndpoint::SessionGapEvent: the resume could not cover what was lost (8.4); a new session follows */
class GUCEF_COMCORE_EXPORT_CPP CRpcSessionGapEventData : public CORE::CICloneable
{
    public:

    UInt64 gapToInitiatorFrom;
    UInt64 gapToInitiatorTo;
    UInt64 gapToAcceptorFrom;
    UInt64 gapToAcceptorTo;
    bool sessionUnknown;            /**< the acceptor no longer knew the session (SESSION_UNKNOWN) */

    CRpcSessionGapEventData( void );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** CRpcEndpoint::GapDetectedEvent (telemetry, 7.4): frames from..to were missing on arrival */
class GUCEF_COMCORE_EXPORT_CPP CRpcGapEventData : public CORE::CICloneable
{
    public:

    UInt64 from;
    UInt64 to;

    CRpcGapEventData( UInt64 gapFrom, UInt64 gapTo );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** MessageEvent of streams and server calls: one stream message */
class GUCEF_COMCORE_EXPORT_CPP CRpcStreamMessageEventData : public CORE::CICloneable
{
    public:

    CRpcMessage message;
    UInt64 streamIndex;             /**< 1, 2, ... per call and direction */

    CRpcStreamMessageEventData( void );
    CRpcStreamMessageEventData( const CRpcMessage& msg, UInt64 index );
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

#endif /* GUCEF_COMCORE_RPCEVENTDATA_H ? */
