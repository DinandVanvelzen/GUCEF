/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTFAKESTOREPROVIDER_H
#define TESTFAKESTOREPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */
#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
#ifndef TESTFAKESTORESERVER_H
#include "TestFakeStoreServer.h"
#define TESTFAKESTORESERVER_H
#endif /* TESTFAKESTORESERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The store provider of the fake store, as a store plugin would make it: library, items and plans over HTTP from
 *  the CFakeStoreServer, the session token as a secret in the Authorization header of downloads, renewed through the
 *  token endpoint of the store.
 */
class CFakeStoreProvider : public GUCEF::ASSETSTORE::CIAssetStoreProvider
{
    public:

    /** The server all fake providers talk to; set by the tests */
    static void SetServer( CFakeStoreServer* server );

    CFakeStoreProvider( void );
    virtual ~CFakeStoreProvider();

    void SetProviderId( const GUCEF::CORE::CString& providerId );

    GUCEF::UInt32 GetPlanCount( void ) const;
    GUCEF::UInt32 GetRenewCount( void ) const;
    void SetToken( const GUCEF::CORE::CString& token );

    virtual const GUCEF::CORE::CString& GetProviderId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsSignedIn( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RenewSession( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus QueryLibrary( const GUCEF::ASSETSTORE::CAssetStoreQuery& query, GUCEF::ASSETSTORE::CAssetStoreLibraryPage& page ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetItem( const GUCEF::CORE::CString& itemId, GUCEF::ASSETSTORE::CAssetStoreItem& item ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus PlanDownload( const GUCEF::CORE::CString& itemId, const GUCEF::CORE::CString& formatId,
                                                               const GUCEF::CORE::CString::StringVector& fileIds, GUCEF::ASSETSTORE::CAssetStoreDownloadPlan& plan ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AuthorizeTransferRequest( const GUCEF::ASSETSTORE::CAssetStoreTransfer& transfer, GUCEF::WEB::CHttpRequestData& request ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::ASSETSTORE::TAssetStoreStatus Get( const GUCEF::CORE::CString& path, GUCEF::CORE::CDataNode& json ) const;

    private:

    GUCEF::CORE::CString m_providerId;
    GUCEF::AUTH::CSecretString m_token;
    GUCEF::UInt32 m_planCount;
    GUCEF::UInt32 m_renewCount;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

/** Makes fake store providers with an id, as the concrete factory of a store plugin */
class CFakeStoreProviderFactory : public GUCEF::CORE::CTFactory< GUCEF::ASSETSTORE::CIAssetStoreProvider, CFakeStoreProvider, GUCEF::MT::CMutex >
{
    public:

    typedef GUCEF::CORE::CTFactory< GUCEF::ASSETSTORE::CIAssetStoreProvider, CFakeStoreProvider, GUCEF::MT::CMutex > TBase;

    explicit CFakeStoreProviderFactory( const GUCEF::CORE::CString& providerId );
    virtual ~CFakeStoreProviderFactory();

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::CORE::CString m_providerId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTFAKESTOREPROVIDER_H ? */
