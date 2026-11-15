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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#include "gucefASSETSTORE_CAssetStoreDownloadJob.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H ? */
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
 *  The event data of the events of CAssetStoreDownloadManager: the job as it was when the event was raised, and for
 *  progress the rate and the time left (both 0 when not known yet).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreDownloadJobEventData : public CORE::CICloneable
{
    public:

    CAssetStoreDownloadJob job;
    UInt64 bytesPerSecond;
    UInt64 secondsLeft;

    CAssetStoreDownloadJobEventData( void );
    explicit CAssetStoreDownloadJobEventData( const CAssetStoreDownloadJob& jobState );
    virtual ~CAssetStoreDownloadJobEventData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H ? */
