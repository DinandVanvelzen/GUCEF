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

#ifndef GUCEF_ASSETSTORE_CASSETSHAREPROVIDER_H
#define GUCEF_ASSETSTORE_CASSETSHAREPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_DRN_CDRNFILESYSTEMACCESSRESOURCEPROVIDER_H
#include "gucefDRN_CDRNFileSystemAccessResourceProvider.h"
#define GUCEF_DRN_CDRNFILESYSTEMACCESSRESOURCEPROVIDER_H
#endif /* GUCEF_DRN_CDRNFILESYSTEMACCESSRESOURCEPROVIDER_H ? */

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

class CAssetShareDecision;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A share of a tree of the VFS of the asset store service (the root of the primary store) through the DRN resource facility: names, sizes,
 *  times and listings from the VFS, the bytes from its URI accessor (CDRNFileSystemAccessResourceProvider), and the SHA-256 of a file from
 *  the catalogue when the catalogue has the file with the size it has on disk (CIAssetStoreService::GetCatalogueDigestOfPath()), so a
 *  client of the share knows the digest of a file without the file being read or hashed. The files that the catalogue does not hold
 *  (staging, thumbnails, the files of a pack that no asset names) are described without a digest.
 *
 *  Describing a file also has the restrictions of its pack kept by the service (CAssetShareDecision::Warm()) when a decision is given, so the
 *  read that follows a STAT or a listing finds them. The calls run on the threads of the pool of the resource transfers, where a query of
 *  the catalogue is allowed.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetShareProvider : public DRN::CDRNFileSystemAccessResourceProvider
{
    public:

    /**
     *  @param vfsRoot  the VFS path of the root of the share ("assetstore/main"), without a leading or trailing '/'
     *  @param service  the service whose catalogue knows the digests; not owned beyond the pointer
     *  @param decision the decision that keeps the restrictions of the packs; may be NULL; must outlive the provider
     */
    CAssetShareProvider( const CORE::CString& vfsRoot, const TAssetStoreServicePtr& service, CAssetShareDecision* decision );

    virtual ~CAssetShareProvider() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetVfsRoot( void ) const;

    /** Digests answered from the catalogue since the provider was made */
    UInt64 GetCatalogueDigestCount( void ) const;

    protected:

    virtual bool GetKnownSha256( const CORE::CString& name, const DRN::CDRNNamespaceEntry& entry, UInt8* digest ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetShareProvider( const CAssetShareProvider& src );                  /**< not implemented */
    CAssetShareProvider& operator=( const CAssetShareProvider& src );       /**< not implemented */

    CORE::CString m_vfsRoot;
    TAssetStoreServicePtr m_service;
    CAssetShareDecision* m_decision;
    mutable MT::CTAtomic< UInt64 > m_digests;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSHAREPROVIDER_H ? */
