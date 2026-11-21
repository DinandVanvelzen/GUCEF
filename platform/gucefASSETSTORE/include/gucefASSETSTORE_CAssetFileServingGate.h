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

#ifndef GUCEF_ASSETSTORE_CASSETFILESERVINGGATE_H
#define GUCEF_ASSETSTORE_CASSETFILESERVINGGATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

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
 *  The licence gate of the content that a host of the asset store service gives to a device (remote service design 7.2 and 7.3): the
 *  blocks of the files and the stored thumbnails.
 *
 *  - The role "mcp" is machine operation, not machine learning: it sees the metadata of every pack (searches, details, file sets, the
 *    names and sizes of a folder), and the raw content of a pack stays withheld from it (MCP design D6): no block of a file and no
 *    thumbnail.
 *  - A pack with the licence flag noRedistribution is for the devices of the owner only (the role "owner").
 *  - A caller that exports raw data into a learning pipeline (a learning role of the host) never gets the content of a pack with
 *    noMachineLearning (answered as if it is not there).
 *
 *  A device with the owner role that is no learning role passes without a look at the catalogue. The pictures that a gated caller was
 *  allowed to see with GetThumbnails() are remembered per device, so reading their files afterwards is allowed (the file of a thumbnail
 *  is in no pack folder).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetFileServingGate
{
    public:

    struct GUCEF_ASSETSTORE_PUBLIC_CPP SCaller
    {
        CORE::CString deviceId;
        bool owner;                     /**< has the role "owner" */
        bool mcp;                       /**< has the role "mcp" */
        bool learning;                  /**< has a learning role of the host */
        SCaller( void );
        bool NeedsLook( void ) const;   /**< false for an owner that is no learning role and no machine caller */
    };

    enum EDecision
    {
        ALLOWED = 0,
        WITHHELD_FROM_MCP,
        RESTRICTED_NO_REDISTRIBUTION,
        RESTRICTED_NO_MACHINE_LEARNING
    };

    explicit CAssetFileServingGate( CIAssetStoreService* service );

    void SetService( CIAssetStoreService* service );

    /** The decision for the blocks of a file (a path of the service); 'reason' says it in plain words when it is not ALLOWED */
    EDecision CheckFile( const SCaller& caller, const CORE::CString& servicePath, CORE::CString& reason ) const;

    /** The decision for the content of a pack, for the thumbnails of its assets */
    EDecision CheckPack( const SCaller& caller, const CORE::CString& packId, CORE::CString& reason ) const;

    /** The decision for the thumbnail of an id of GetThumbnails() ("pack:<id>" or an asset id) */
    EDecision CheckThumbnailId( const SCaller& caller, const CORE::CString& id, CORE::CString& reason ) const;

    /** Remembers that a device may read the picture file of a thumbnail it was given; bounded */
    void GrantThumbnail( const CORE::CString& deviceId, const CORE::CString& vfsPath );

    private:

    CAssetFileServingGate( const CAssetFileServingGate& src );
    CAssetFileServingGate& operator=( const CAssetFileServingGate& src );

    EDecision Decide( const SCaller& caller, const CORE::CString::StringSet& flags, CORE::CString& reason ) const;
    static bool IsThumbnailPath( const CORE::CString& path );

    private:

    CIAssetStoreService* m_service;
    mutable MT::CMutex m_lock;
    GUCEF::map< CORE::CString, CORE::CString::StringSet > m_granted;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETFILESERVINGGATE_H ? */
