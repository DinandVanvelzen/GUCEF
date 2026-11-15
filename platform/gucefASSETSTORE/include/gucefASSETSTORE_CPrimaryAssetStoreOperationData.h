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

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTOREOPERATIONDATA_H
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTOREOPERATIONDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#include "gucefASSETSTORE_CAssetIngestRequest.h"
#define GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#include "gucefASSETSTORE_CAssetSearchQuery.h"
#define GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#endif /* GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H ? */
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

class CPrimaryAssetStore;

/** The task data of an async operation of a CPrimaryAssetStore */
class GUCEF_ASSETSTORE_PUBLIC_CPP CPrimaryAssetStoreOperationData : public CORE::CICloneable
{
    public:

    enum EOperation
    {
        OPERATION_INGEST = 0    ,
        OPERATION_RESUME_INGEST ,
        OPERATION_SEARCH        ,
        OPERATION_VERIFY        ,
        OPERATION_CONVERT_PACK  ,
        OPERATION_CONVERT_PENDING
    };

    CPrimaryAssetStore* store;
    EOperation operation;
    CAssetIngestRequest request;
    CORE::CString ingestId;
    CAssetSearchQuery query;
    CORE::CString locationId;
    CORE::CString packId;
    CORE::CFutureResultSource source;

    CPrimaryAssetStoreOperationData( void );
    virtual ~CPrimaryAssetStoreOperationData() GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTOREOPERATIONDATA_H ? */
