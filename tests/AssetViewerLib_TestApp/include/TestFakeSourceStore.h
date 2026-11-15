/*
 *  AssetViewerLib_TestApp: tests of the AssetViewerLib module
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

#ifndef TESTFAKESOURCESTORE_H
#define TESTFAKESOURCESTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORETRANSFERSOURCE_H
#include "gucefASSETSTORE_CIAssetStoreTransferSource.h"
#define GUCEF_ASSETSTORE_CIASSETSTORETRANSFERSOURCE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORETRANSFERSOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace SOURCESTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CFakeSourceProvider;

/** Delivers the files of the fake store from memory into the ".part" file, in chunks, as far as the test lets it */
class CFakeTransferSource : public GUCEF::ASSETSTORE::CIAssetStoreTransferSource
{
    public:

    explicit CFakeTransferSource( CFakeSourceProvider& provider );

    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RunTransfer( const GUCEF::ASSETSTORE::CAssetStoreTransfer& transfer, GUCEF::ASSETSTORE::CAssetStoreTransferContext& context ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CFakeSourceProvider& m_provider;
};

/**
 *  A store of the tests that lives in memory: a library of items with formats and files, a sign in that the test turns on and off, and
 *  downloads that deliver their bytes in chunks of 'chunkSize' only as far as the test allows (AllowBytes()), so that a download can
 *  be seen halfway. The provider id is "fakesrc".
 */
class CFakeSourceProvider : public GUCEF::ASSETSTORE::CIAssetStoreProvider
{
    public:

    struct SFileSpec
    {
        GUCEF::CORE::CString name;
        GUCEF::UInt32 size;
        GUCEF::UInt32 seed;
    };

    struct SFormatSpec
    {
        GUCEF::CORE::CString formatId;
        GUCEF::CORE::CString name;
        GUCEF::CORE::CString group;
        GUCEF::vector< SFileSpec > files;
    };

    static const GUCEF::CORE::CString ProviderIdText;

    CFakeSourceProvider( void );
    virtual ~CFakeSourceProvider();

    void AddItem( const GUCEF::CORE::CString& itemId, const GUCEF::CORE::CString& title, const GUCEF::CORE::CString& seller, const GUCEF::CORE::CString& listingType,
                  const GUCEF::vector< SFormatSpec >& formats, const GUCEF::CORE::CString& licenceName = GUCEF::CORE::CString::Empty,
                  const GUCEF::CORE::CString& thumbnailUrl = GUCEF::CORE::CString::Empty );

    void SetSignedIn( bool signedIn );

    /** The downloads write up to this many bytes of a file and wait for more; the default lets everything through */
    void AllowBytes( GUCEF::UInt64 bytes );
    GUCEF::UInt64 GetAllowedBytes( void ) const;

    GUCEF::UInt32 GetChunkSize( void ) const;

    /** The bytes of a file of the store by its id */
    bool GetContent( const GUCEF::CORE::CString& fileId, GUCEF::CORE::CDynamicBuffer& content ) const;

    virtual const GUCEF::CORE::CString& GetProviderId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetAuthProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsSignedIn( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus QueryLibrary( const GUCEF::ASSETSTORE::CAssetStoreQuery& query, GUCEF::ASSETSTORE::CAssetStoreLibraryPage& page ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetItem( const GUCEF::CORE::CString& itemId, GUCEF::ASSETSTORE::CAssetStoreItem& item ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus PlanDownload( const GUCEF::CORE::CString& itemId, const GUCEF::CORE::CString& formatId,
                                                               const GUCEF::CORE::CString::StringVector& fileIds, GUCEF::ASSETSTORE::CAssetStoreDownloadPlan& plan ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::CIAssetStoreTransferSource* GetTransferSource( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::ASSETSTORE::TAssetStoreItemVector m_items;
    CFakeTransferSource m_source;
    bool m_signedIn;
    GUCEF::UInt64 m_allowedBytes;
    mutable GUCEF::MT::CMutex m_lock;
};

/** Makes fake stores as a store plugin would: the provider in a shared pointer */
typedef GUCEF::CORE::CTFactory< GUCEF::ASSETSTORE::CIAssetStoreProvider, CFakeSourceProvider, GUCEF::MT::CMutex > TFakeSourceFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace SOURCESTEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTFAKESOURCESTORE_H ? */
