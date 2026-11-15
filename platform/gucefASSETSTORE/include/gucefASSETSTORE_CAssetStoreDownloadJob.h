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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJOBFILE_H
#include "gucefASSETSTORE_CAssetStoreJobFile.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJOBFILE_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJOBFILE_H ? */
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
 *  A job of the download manager (section 4.4): one item and format of a store, or chosen files of it, into a
 *  destination (a VFS folder). What the queue file holds and what the events of the manager carry.
 *
 *  A job meant for the primary store ('forIngest') goes into its staging area; the primary store ingests it when the
 *  job is done and the policy of the store is automatic.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreDownloadJob : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString jobId;
    UInt64 sequence;                        /**< the order of the queue */
    CORE::CString providerId;
    CORE::CString itemId;
    CORE::CString itemTitle;
    CORE::CString formatId;
    CORE::CString::StringVector fileIds;    /**< the files asked for, empty for every file of the format */
    CORE::CString destination;              /**< a VFS folder */
    bool forIngest;
    TAssetDownloadJobState state;
    TAssetStoreStatus status;               /**< why the job failed, OK otherwise */
    CORE::CString message;                  /**< for people, never holds a secret */
    TAssetStoreJobFileVector files;
    UInt32 attempts;                        /**< transfer attempts that failed and were retried */
    Int64 createdAtMs;                      /**< UTC milliseconds since the Unix epoch */
    Int64 updatedAtMs;

    CAssetStoreDownloadJob( void );
    virtual ~CAssetStoreDownloadJob() GUCEF_VIRTUAL_OVERRIDE;

    UInt64 GetTotalBytes( void ) const;
    UInt64 GetBytesDone( void ) const;

    /** The VFS path of a file of the job */
    CORE::CString GetFilePath( const CAssetStoreJobFile& file ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef GUCEF::vector< CAssetStoreDownloadJob > TAssetStoreDownloadJobVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H ? */
