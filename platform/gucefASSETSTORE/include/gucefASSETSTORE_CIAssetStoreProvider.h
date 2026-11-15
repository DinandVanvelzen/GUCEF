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

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */
#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */
#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */
#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */
#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */
#ifndef GUCEF_WEB_CHTTPREQUESTDATA_H
#include "gucefWEB_CHttpRequestData.h"
#define GUCEF_WEB_CHTTPREQUESTDATA_H
#endif /* GUCEF_WEB_CHTTPREQUESTDATA_H ? */

#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREQUERY_H
#include "gucefASSETSTORE_CAssetStoreQuery.h"
#define GUCEF_ASSETSTORE_CASSETSTOREQUERY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREQUERY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#include "gucefASSETSTORE_CAssetStoreLibraryPage.h"
#define GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADPLAN_H
#include "gucefASSETSTORE_CAssetStoreDownloadPlan.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADPLAN_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADPLAN_H ? */
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

class CIAssetMetadataNormalizer;
class CIAssetStoreTransferSource;
class CAssetStoreProviderOperationTaskConsumer;
class CWebSessionSignIn;

/**
 *  A store of assets (section 4.1 of the design): its identity, the sign in it needs, its library, the details of an
 *  item and the plan of a download. Implemented by a store plugin, which registers a concrete factory under the id of
 *  the store in CAssetStoreGlobal::GetProviderFactory() when it loads; CAssetStoreRegistry makes the providers. The
 *  primary store is a provider as well, so the library of every store is browsed the same way.
 *
 *  Every library operation has a sync form, which runs on the calling thread and never pumps a pulse generator, and an
 *  async form that runs the sync form as a task on a thread pool (the pool of SetThreadPoolName(), "AssetStore" by
 *  default) and returns a CFutureResult. The result data comes with the completed future: CAssetStoreLibraryPage,
 *  CAssetStoreItem or CAssetStoreDownloadPlan. A failure ends the future in an error state with "statusName: message"
 *  as its extra info; GetStatusOfFuture() gives the status back.
 *
 *  A provider whose async operations may still run when it is destroyed calls WaitForAsyncOperations() at the start of
 *  its destructor: the operations call into the derived class.
 *
 *  Sign in goes through gucefAUTH: GetAuthProviderName() names the provider of gucefAUTH that the store needs, the
 *  tokens are kept as AUTH::CSecretString by the plugin and are never logged.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetStoreProvider : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent SessionStateChangedEvent;     /**< TCloneableString: the provider id; signed in or out */

    static const CORE::CString DefaultThreadPoolName;       /**< "AssetStore" */

    static void RegisterEvents( void );

    CIAssetStoreProvider( void );

    virtual ~CIAssetStoreProvider() GUCEF_VIRTUAL_OVERRIDE;

    /** The id of the store, lower case: "primary", or the id a store plugin registers its factory under */
    virtual const CORE::CString& GetProviderId( void ) const = 0;

    /** The name of the store for people */
    virtual CORE::CString GetDisplayName( void ) const = 0;

    /** A generic icon name for the store ("store", "archive"), empty by default */
    virtual CORE::CString GetIconName( void ) const;

    /** The home page of the store, empty by default */
    virtual CORE::CString GetHomePageUrl( void ) const;

    /** The gucefAUTH provider the store signs in with, empty when the store needs no sign in (the default) */
    virtual CORE::CString GetAuthProviderName( void ) const;

    /**
     *  How the store signs in (section 12.1 of the design). By default ASSETSTORE_SIGNIN_AUTH_PROVIDER when
     *  GetAuthProviderName() names a provider, else ASSETSTORE_SIGNIN_NONE. A store that signs in on its web site
     *  answers ASSETSTORE_SIGNIN_WEB_SESSION and gives its sign in with GetWebSessionSignIn()
     *  (CWebSessionAssetStoreProvider does both)
     */
    virtual TAssetStoreSignInKind GetSignInKind( void ) const;

    /**
     *  The web session sign in of a store whose kind is ASSETSTORE_SIGNIN_WEB_SESSION: the application starts it
     *  (CWebSessionSignIn::SignInAsync()) when the owner asks to sign in and shows its events. GUCEF_NULL by default
     */
    virtual CWebSessionSignIn* GetWebSessionSignIn( void );

    virtual bool IsSignedIn( void ) const = 0;

    /** Forgets the session of the store; true by default */
    virtual bool SignOut( void );

    /**
     *  Renews the session after the store refused it (HTTP 401), for example with the refresh token of the account.
     *  The download manager calls it once per refusal before it gives up with ASSETSTORE_STATUS_SESSION_EXPIRED.
     *  False by default: the session cannot be renewed without the person
     */
    virtual bool RenewSession( void );

    /** A page of the library of the signed in account */
    virtual TAssetStoreStatus QueryLibrary( const CAssetStoreQuery& query, CAssetStoreLibraryPage& page ) = 0;

    /** The details of an item, with its formats and files */
    virtual TAssetStoreStatus GetItem( const CORE::CString& itemId, CAssetStoreItem& item ) = 0;

    /**
     *  The concrete transfers of a download of the format of an item: of every file of the format, or of the files
     *  whose ids are given. Made right before the transfers start, since the addresses may be short lived
     */
    virtual TAssetStoreStatus PlanDownload( const CORE::CString& itemId              ,
                                            const CORE::CString& formatId            ,
                                            const CORE::CString::StringVector& fileIds,
                                            CAssetStoreDownloadPlan& plan            ) = 0;

    /** Makes the plan again after an address expired; by default PlanDownload() with the item, format and files of the plan */
    virtual TAssetStoreStatus RenewPlan( CAssetStoreDownloadPlan& plan );

    /** The normaliser of the metadata of the items of the store (section 7.2); GUCEF_NULL by default: the generic one is used */
    virtual CIAssetMetadataNormalizer* GetMetadataNormalizer( void );

    /** For stores that do not deliver plain files; GUCEF_NULL by default: transfers are plain HTTP(S) downloads */
    virtual CIAssetStoreTransferSource* GetTransferSource( void );

    /**
     *  Adds what the store needs to a download request of a transfer, for example the header with the token of the
     *  session. Called for every attempt, so a renewed session is used. True by default, without a change
     */
    virtual bool AuthorizeTransferRequest( const CAssetStoreTransfer& transfer, WEB::CHttpRequestData& request );

    /** Whether a response to a download request means that its address expired; by default HTTP 403 and 410 */
    virtual bool IsExpiredUrlResponse( const CAssetStoreTransfer& transfer, const WEB::CHttpResponseData& response );

    CORE::CFutureResult QueryLibraryAsync( const CAssetStoreQuery& query );

    CORE::CFutureResult GetItemAsync( const CORE::CString& itemId );

    CORE::CFutureResult PlanDownloadAsync( const CORE::CString& itemId               ,
                                           const CORE::CString& formatId             ,
                                           const CORE::CString::StringVector& fileIds );

    /** The thread pool of the async operations */
    void SetThreadPoolName( const CORE::CString& threadPoolName );
    CORE::CString GetThreadPoolName( void ) const;

    /** Waits until no async operation of the provider runs any more; false when the timeout passed first */
    bool WaitForAsyncOperations( UInt32 timeoutInMs = GUCEF_MT_LONG_LOCK_TIMEOUT );

    UInt32 GetPendingAsyncOperationCount( void ) const;

    /** The status of an async operation that ended: OK when it finished, otherwise the status of its extra info */
    static TAssetStoreStatus GetStatusOfFuture( const CORE::CFutureResult& future );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Raises SessionStateChangedEvent, from any thread */
    void NotifySessionStateChanged( void );

    private:

    friend class CAssetStoreProviderOperationTaskConsumer;

    CORE::CFutureResult QueueOperation( CORE::CICloneable* operationData );
    void OperationEnded( void );

    CIAssetStoreProvider( const CIAssetStoreProvider& src );              /**< not implemented */
    CIAssetStoreProvider& operator=( const CIAssetStoreProvider& src );   /**< not implemented */

    private:

    MT::CTAtomic< UInt32 > m_pendingOperations;
    CORE::CString m_threadPoolName;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIAssetStoreProvider, MT::CMutex >  CAssetStoreProviderFactory;
typedef CAssetStoreProviderFactory::TProductPtr                                      TAssetStoreProviderPtr;

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
