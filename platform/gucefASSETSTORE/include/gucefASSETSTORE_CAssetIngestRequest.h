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

#ifndef GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#define GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */
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
 *  What to ingest into the primary store (section 8): a download in the staging area of a location, or a folder or
 *  archive on disk (any VFS path). For a pack of a store the item describes it; when only its id is given the ingest
 *  asks the store for the details. Empty overrides take the policy of the source: 'keep' ("original", "converted",
 *  "both"), 'locationId' and 'duplicatePolicy' ("skip", "keep"). 'copyContent' tells whether a pack from disk is copied
 *  into the location or referenced in place; -1 takes the setting copyDiskPacks.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetIngestRequest : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString sourceId;                 /**< the store id, or "disk" */
    CORE::CString itemId;
    CAssetStoreItem item;
    CORE::CString contentPath;              /**< the VFS folder or archive with the files */
    Int32 copyContent;
    CORE::CString keep;
    CORE::CString locationId;
    CORE::CString duplicatePolicy;
    CORE::CString downloadJobId;

    CAssetIngestRequest( void );
    virtual ~CAssetIngestRequest() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H ? */
