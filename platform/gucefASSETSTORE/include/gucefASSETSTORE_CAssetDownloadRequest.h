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

#ifndef GUCEF_ASSETSTORE_CASSETDOWNLOADREQUEST_H
#define GUCEF_ASSETSTORE_CASSETDOWNLOADREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
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
 *  A download from an online store that the asset store service is asked for: the format of an item, the files, and either
 *  a destination folder or the primary store, with the keep policy and the storage location of the ingest that follows.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetDownloadRequest : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString providerId;
    CORE::CString itemId;
    CORE::CString itemTitle;
    CORE::CString formatId;
    CORE::CString::StringVector fileIds;    /**< empty: every file of the format */
    CORE::CString destination;              /**< VFS folder; not used with forIngest */
    bool forIngest;                         /**< into the staging area of the primary store, ingested when it is done */
    CORE::CString keep;                     /**< keep policy name of the ingest; empty: the policy of the source */
    CORE::CString locationId;               /**< empty: the default location */
    CORE::CString expectedRevision;         /**< the tagged effective revision of the item the library sync planned with (CAssetItemRevision); empty for a manual download */
    CORE::CString syncJobId;                /**< the library sync job that queued the download, empty for a manual download */
    UInt64 expectedBytes;                   /**< the size the download is expected to have, 0 when not known: chooses whether the scratch staging area has room for it */

    CAssetDownloadRequest( void );
    virtual ~CAssetDownloadRequest() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

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

#endif /* GUCEF_ASSETSTORE_CASSETDOWNLOADREQUEST_H ? */
