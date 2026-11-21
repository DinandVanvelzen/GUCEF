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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

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
 *  The RPC contracts of the asset store as a remote service (gucefASSETSTORE_RemoteService_Design.md 6, the generic call
 *  surface of step R2): every operation of CIAssetStoreService travels as its CAssetServiceCall (the operation name of
 *  CAssetServiceOperations, the command id and the arguments as a DataNode) and is answered with the status name of
 *  AssetStoreStatusToString(), a message and the result record in its DataNode form, so a new operation of the
 *  interface needs no new contract. Only the hot path is typed: blocks of files and of uploads travel as bytes.
 *
 *  gucef.assetstore 1.0, service Service (the control channel):
 *      Query    (U, idempotent)   reads
 *      Command  (U)               mutations; the command id makes a repeat answer the first outcome
 *      Secret   (U, sensitive)    the operations that carry a secret: AnswerPrompt, ImportWebSession, UnlockKeyring (field secret is sensitive)
 *  gucef.assetstore.files 1.0, service Files (the bulk channel, a DRN link of its own; the control channel binds it too):
 *      Query    (U, idempotent)   StatFiles, ListDirectory, QueryUpload
 *      Command  (U)               BeginUpload, BeginUploadFiles, CommitUpload, AbortUpload
 *      ReadBlock  (U, idempotent) at most BlockSize bytes of a file, binary
 *      WriteBlock (U)             bytes of a file of an upload, binary
 *  Every answer carries eventSeq, the next sequence of the caller's session event ring after the call: a client delivers
 *  the session events before that sequence before it hands out the answer, so events a call caused come before its
 *  result as they do in process.
 *
 *  Authoring sources: platform/gucefASSETSTORE/rpc/gucef.assetstore.json and gucef.assetstore.files.json; the contracts
 *  are built in code (no JSON codec needed at run time).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreRemoteContracts
{
    public:

    static const char* ControlContractName;     /**< "gucef.assetstore" */
    static const char* FilesContractName;       /**< "gucef.assetstore.files" */
    static const UInt32 Major = 1;
    static const UInt32 BlockSize = 262144;     /**< bytes of a block of a file, 256 KiB */

    static const char* QueryPath;               /**< "gucef.assetstore.Service/Query" */
    static const char* CommandPath;             /**< "gucef.assetstore.Service/Command" */
    static const char* SecretPath;              /**< "gucef.assetstore.Service/Secret" */
    static const char* FilesQueryPath;          /**< "gucef.assetstore.files.Files/Query" */
    static const char* FilesCommandPath;        /**< "gucef.assetstore.files.Files/Command" */
    static const char* ReadBlockPath;           /**< "gucef.assetstore.files.Files/ReadBlock" */
    static const char* WriteBlockPath;          /**< "gucef.assetstore.files.Files/WriteBlock" */

    // the field numbers of the messages
    static const UInt32 CallOperationField = 1;
    static const UInt32 CallCommandIdField = 2;
    static const UInt32 CallArgsField = 3;
    static const UInt32 AnswerStatusField = 1;
    static const UInt32 AnswerMessageField = 2;
    static const UInt32 AnswerResultField = 3;
    static const UInt32 AnswerEventSeqField = 4;
    static const UInt32 AnswerReplayedField = 5;
    static const UInt32 SecretOperationField = 1;
    static const UInt32 SecretCommandIdField = 2;
    static const UInt32 SecretTargetField = 3;
    static const UInt32 SecretValueField = 4;
    static const UInt32 ReadPathField = 1;
    static const UInt32 ReadOffsetField = 2;
    static const UInt32 ReadLengthField = 3;
    static const UInt32 BlockStatusField = 1;
    static const UInt32 BlockMessageField = 2;
    static const UInt32 BlockFileSizeField = 3;
    static const UInt32 BlockEndOfFileField = 4;
    static const UInt32 BlockDataField = 5;
    static const UInt32 BlockOffsetField = 6;
    static const UInt32 WriteUploadIdField = 1;
    static const UInt32 WritePathField = 2;
    static const UInt32 WriteOffsetField = 3;
    static const UInt32 WriteDataField = 4;

    // the kinds of the session events of the asset store (the data is the record of the event in its DataNode form)
    static const char* EventCatalogueChanged;   /**< "CATALOGUE_CHANGED": CAssetCatalogueChange; revision */
    static const char* EventJobChanged;         /**< "JOB_CHANGED": CAssetJobEntry */
    static const char* EventJobRemoved;         /**< "JOB_REMOVED": CAssetJobEntry */
    static const char* EventLocationState;      /**< "LOCATION_STATE": CAssetLocationInfo */
    static const char* EventStoreState;         /**< "STORE_STATE": CAssetStoreInfo */
    static const char* EventStoresChanged;      /**< "STORES_CHANGED": no data */
    static const char* EventSignInPrompt;       /**< "SIGNIN_PROMPT": CAssetSignInPrompt */
    static const char* EventSignInPromptClosed; /**< "SIGNIN_PROMPT_CLOSED": CAssetSignInPrompt */
    static const char* EventThumbnailUpdated;   /**< "THUMBNAIL_UPDATED": CAssetThumbnail without pixels */
    static const char* EventServiceNotice;      /**< "ASSETSTORE_NOTICE": CAssetServiceNotice */

    /** The contract of the control channel, built and registered once */
    static COMCORE::CRpcContractPtr GetControlContract( void );

    /** The contract of the files (the bulk channel), built and registered once */
    static COMCORE::CRpcContractPtr GetFilesContract( void );

    /** The documents the contracts are built from, in the form of their JSON authoring sources */
    static void GetControlContractSource( CORE::CDataNode& source );
    static void GetFilesContractSource( CORE::CDataNode& source );

    /** Whether the operation belongs to the files contract (the bulk channel) */
    static bool IsFilesOperation( const CORE::CString& operation );

    private:

    CAssetStoreRemoteContracts( void );     /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H ? */
