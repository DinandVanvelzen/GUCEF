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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADPLAN_H
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADPLAN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H
#include "gucefASSETSTORE_CAssetStoreTransfer.h"
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H ? */
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
 *  The concrete transfers for a download of one item and format, or of chosen files of it (section 4.1). A plan is
 *  made right before its transfers start, CIAssetStoreProvider::RenewPlan() makes it again when an address expired.
 *  The result data of CIAssetStoreProvider::PlanDownloadAsync().
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreDownloadPlan : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString providerId;
    CORE::CString itemId;
    CORE::CString formatId;
    CORE::CString::StringVector fileIds;        /**< the files asked for, empty for every file of the format */
    TAssetStoreTransferVector transfers;
    Int64 createdAtMs;                          /**< UTC milliseconds since the Unix epoch */

    CAssetStoreDownloadPlan( void );
    virtual ~CAssetStoreDownloadPlan() GUCEF_VIRTUAL_OVERRIDE;

    /** The transfer of the file, GUCEF_NULL when the plan has none */
    const CAssetStoreTransfer* FindTransfer( const CORE::CString& fileId ) const;

    UInt64 GetTotalSize( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADPLAN_H ? */
