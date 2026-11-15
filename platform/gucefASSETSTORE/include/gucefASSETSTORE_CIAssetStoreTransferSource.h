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

#ifndef GUCEF_ASSETSTORE_CIASSETSTORETRANSFERSOURCE_H
#define GUCEF_ASSETSTORE_CIASSETSTORETRANSFERSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H
#include "gucefASSETSTORE_CAssetStoreTransfer.h"
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H
#include "gucefASSETSTORE_CAssetStoreTransferContext.h"
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H ? */
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
 *  The transfers of a store that does not deliver plain files, for example a manifest of chunks that are put
 *  together (section 4.1). The download manager still runs the job: it gives the context, retries, checks the
 *  digest and the size and renames the ".part" file; the source only fills the ".part" file.
 *
 *  RunTransfer() runs on the thread of the job. It writes the ".part" file of the context through the VFS, starting at
 *  the resume offset of the context (bytes that are there already), reports the bytes the file has with
 *  ReportFileBytes(), stops soon after IsStopRequested() turns true (returning ASSETSTORE_STATUS_CANCELLED) and
 *  returns OK when the file is complete. The statuses NETWORK_ERROR, SERVER_ERROR and RATE_LIMITED (with
 *  SetRetryAfterInMs() when the store said how long to wait) are retried, URL_EXPIRED renews the plan once and
 *  SESSION_EXPIRED renews the session once; any other status fails the job.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetStoreTransferSource
{
    public:

    CIAssetStoreTransferSource( void );
    virtual ~CIAssetStoreTransferSource();

    virtual TAssetStoreStatus RunTransfer( const CAssetStoreTransfer& transfer, CAssetStoreTransferContext& context ) = 0;

    private:

    CIAssetStoreTransferSource( const CIAssetStoreTransferSource& src );              /**< not implemented */
    CIAssetStoreTransferSource& operator=( const CIAssetStoreTransferSource& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETSTORETRANSFERSOURCE_H ? */
