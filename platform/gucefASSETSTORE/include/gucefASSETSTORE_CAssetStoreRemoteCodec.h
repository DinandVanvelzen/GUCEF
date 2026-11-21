/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What both ends of the remote asset store service (CAssetStoreServiceHost, CRemoteAssetStoreService) agree on besides
 *  the contracts (CAssetStoreRemoteContracts): the record type each operation answers with, the answer of a call
 *  (status name, message, the result record in its DataNode form, the event sequence) and how a failure of the RPC layer
 *  itself becomes a TAssetStoreStatus, so a caller sees the statuses of the in-process implementation in both modes.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreRemoteCodec
{
    public:

    /** A new, empty result record of the operation (owned by the caller); GUCEF_NULL for an operation that is not known */
    static CORE::CIDataNodeSerializable* NewResult( const CORE::CString& operation );

    /** The result of CIAssetStoreService::Dispatch() as the record of the operation (checked by type name); GUCEF_NULL for none or another type */
    static const CORE::CIDataNodeSerializable* AsRecord( const CORE::CString& operation, const CORE::CICloneable* result );

    /** Fills a CallAnswer message: the status name, the message, the record in its DataNode form (when there is one) */
    static void FillAnswer( COMCORE::CRpcMessage& answer, TAssetStoreStatus status, const CORE::CString& message,
                            const CORE::CIDataNodeSerializable* record, UInt64 eventSeq, bool replayed );

    /**
     *  Reads a CallAnswer: the status, the message and a new result record of the operation (owned by the caller, filled
     *  from the answer; for a failure without a record one that carries the status and the message where it can)
     */
    static TAssetStoreStatus ReadAnswer( const CORE::CString& operation, const COMCORE::CRpcMessage& answer,
                                         CORE::CIDataNodeSerializable*& result, CORE::CString& message );

    /** The status of the service for a failure of the RPC layer (the service was not reached, refused the call, ...) */
    static TAssetStoreStatus StatusOfRpcFailure( const COMCORE::CRpcStatus& status );

    /** Puts the status and the message into a result record that has members for them; false when it has none */
    static bool SetFailure( CORE::CICloneable* result, TAssetStoreStatus status, const CORE::CString& message );

    private:

    CAssetStoreRemoteCodec( void );     /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H ? */
