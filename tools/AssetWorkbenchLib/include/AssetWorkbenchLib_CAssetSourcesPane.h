/*
 *  AssetWorkbenchLib: the asset sources of the workbench: online stores, the primary store and the transfers
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
#ifndef ASSETWORKBENCHLIB_CASSETSOURCESPANE_H
#define ASSETWORKBENCHLIB_CASSETSOURCESPANE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CCONTENTBROWSERFORM_H
#include "gucefGUI_CContentBrowserForm.h"
#define GUCEF_GUI_CCONTENTBROWSERFORM_H
#endif /* GUCEF_GUI_CCONTENTBROWSERFORM_H ? */

#ifndef GUCEF_GUI_CASSETSEARCHFORM_H
#include "gucefGUI_CAssetSearchForm.h"
#define GUCEF_GUI_CASSETSEARCHFORM_H
#endif /* GUCEF_GUI_CASSETSEARCHFORM_H ? */

#ifndef GUCEF_GUI_CTRANSFERQUEUEFORM_H
#include "gucefGUI_CTransferQueueForm.h"
#define GUCEF_GUI_CTRANSFERQUEUEFORM_H
#endif /* GUCEF_GUI_CTRANSFERQUEUEFORM_H ? */

#ifndef GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#include "gucefGUI_CPropertyInspectorForm.h"
#define GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#endif /* GUCEF_GUI_CPROPERTYINSPECTORFORM_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */


#ifndef GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H
#include "gucefASSETSTORE_CAssetStoreLibraryFileSystemAccess.h"
#define GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINECONVERTERHOOK_H
#include "AssetStoreConverterHook_CPipelineConverterHook.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINECONVERTERHOOK_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINECONVERTERHOOK_H ? */
#ifndef GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#include "gucefASSETSTORE_CRemoteAssetFileSystem.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H ? */

#ifndef ASSETWORKBENCHLIB_CREMOTEFETCHJOBS_H
#include "AssetWorkbenchLib_CRemoteFetchJobs.h"
#define ASSETWORKBENCHLIB_CREMOTEFETCHJOBS_H
#endif /* ASSETWORKBENCHLIB_CREMOTEFETCHJOBS_H ? */

#ifndef ASSETWORKBENCHLIB_CREMOTETHUMBNAILSOURCE_H
#include "AssetWorkbenchLib_CRemoteThumbnailSource.h"
#define ASSETWORKBENCHLIB_CREMOTETHUMBNAILSOURCE_H
#endif /* ASSETWORKBENCHLIB_CREMOTETHUMBNAILSOURCE_H ? */

#ifndef ASSETWORKBENCHLIB_CREMOTECOOKEDDISCOVERY_H
#include "AssetWorkbenchLib_CRemoteCookedDiscovery.h"
#define ASSETWORKBENCHLIB_CREMOTECOOKEDDISCOVERY_H
#endif /* ASSETWORKBENCHLIB_CREMOTECOOKEDDISCOVERY_H ? */

#ifndef ASSETWORKBENCHLIB_CREMOTEREFERENCEDDISCOVERY_H
#include "AssetWorkbenchLib_CRemoteReferencedDiscovery.h"
#define ASSETWORKBENCHLIB_CREMOTEREFERENCEDDISCOVERY_H
#endif /* ASSETWORKBENCHLIB_CREMOTEREFERENCEDDISCOVERY_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETMOUNTER_H
#include "AssetWorkbenchLib_CAssetMounter.h"
#define ASSETWORKBENCHLIB_CASSETMOUNTER_H
#endif /* ASSETWORKBENCHLIB_CASSETMOUNTER_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSOURCESSETTINGS_H
#include "AssetWorkbenchLib_CAssetSourcesSettings.h"
#define ASSETWORKBENCHLIB_CASSETSOURCESSETTINGS_H
#endif /* ASSETWORKBENCHLIB_CASSETSOURCESSETTINGS_H ? */

#ifndef ASSETWORKBENCHLIB_CPRIMARYSTORESEARCHMODEL_H
#include "AssetWorkbenchLib_CPrimaryStoreSearchModel.h"
#define ASSETWORKBENCHLIB_CPRIMARYSTORESEARCHMODEL_H
#endif /* ASSETWORKBENCHLIB_CPRIMARYSTORESEARCHMODEL_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H
#include "AssetWorkbenchLib_CAssetStoreTransferModel.h"
#define ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H
#endif /* ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSTORETHUMBNAILPROVIDER_H
#include "AssetWorkbenchLib_CAssetStoreThumbnailProvider.h"
#define ASSETWORKBENCHLIB_CASSETSTORETHUMBNAILPROVIDER_H
#endif /* ASSETWORKBENCHLIB_CASSETSTORETHUMBNAILPROVIDER_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/


/**
 *  The "Asset sources" workflow of the workbench, the page of a tab of the left side: the primary store first, then the online stores
 *  that are registered, each with its sign in state.
 *
 *  - The primary store: the search form (CAssetSearchForm on CPrimaryStoreSearchModel). A result is selected to see it in the
 *    inspector (the asset, its pack, the licence with its restrictions) and opened to show it in the views: the VFS path comes from the
 *    catalogue, and the storage locations of the store are adopted as mounts of the workbench (CAssetMounter::Adopt()). "Add
 *    folder or archive" ingests a pack from the disk: a path of the VFS or of the disk (mounted first), with the keep choice and
 *    the storage location of the page.
 *  - An online store: its library is browsed in a content browser through ASSETSTORE::CAssetStoreLibraryFileSystemAccess, with the listing pictures
 *    as thumbnails. Selecting an item asks the store for its details and shows the formats, files and licence in the inspector;
 *    the format list offers the formats in the order of the preference of the settings. "Import" queues a download into the
 *    download folder (and mounts what finished when the settings say so), "Add to primary store" queues a download for the
 *    primary store and ingests it when it is done with the keep and location choices of the page (a source with the automatic
 *    policy is ingested by the store itself). A store that needs a sign in says so in its line ("signed in", "sign in required",
 *    "session ended") and has two buttons: "Sign in" notifies SignInRequestedEvent and "Sign in settings" SignInSettingsRequestedEvent
 *    with the id of the store, for the application to route (CWorkbenchShell gives them to CWorkbenchSignIn). When an action of the
 *    library finds the session over (the status "session expired"), the page raises SessionExpiredEvent once, and when the sign in is
 *    done it reads the library again, or asks for the details of the selected item again, once; a second refusal is shown as the
 *    error of the store and does not start another sign in.
 *  - The downloads, the ingests and the conversions are the transfers of a CTransferQueueForm that the application gives with
 *    Initialize(); "open folder" notifies FolderOpenRequestedEvent.
 *  - Conversion: the store gets the converter hook of the settings (the converter pipeline with the cook profile of the store), so
 *    that a pack whose keep policy asks for converted content is converted when it is ingested. A pack that is "conversion pending"
 *    (no converter then, or a conversion that ended with errors) or "conversion refused" is converted later with "Convert" on the
 *    selected result of the primary store (the pack of the result): a pool task whose progress is a transfer of the queue.
 *    Opening a result opens its original; an asset whose original is gone (keep: converted), or that has none, opens its
 *    converted file, and OpenResultConverted() opens the converted file of an asset that has one.
 *
 *  Every store operation goes through the asset store service (ASSETSTORE::CIAssetStoreService): Open() makes the implementation that
 *  the settings name ("AssetSources/service/mode": "local", the default, is the store, the download manager and the online stores
 *  in this process, ASSETSTORE::CLocalAssetStoreService, with the converter hook of the settings; "remote" works only in a build that
 *  registers a remote implementation, the page says "remote service not available in this build" otherwise) and opens it;
 *  Initialize() makes the forms in a host widget. Update() has to be called between two frames (the answers of the searches, of
 *  the item details and of the transfers are taken there). With a service in this process the selection, the inspector and the
 *  opening of a result ask the service synchronously, as the store answers at once.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetSourcesPane : public CORE::CObservingNotifier
{
    public:

    typedef CORE::CTEventHandlerFunctor< CAssetSourcesPane > TEventCallback;

    static const CORE::CEvent AssetOpenRequestedEvent;      /**< TCloneableString: the VFS path of an asset to show in the views */
    static const CORE::CEvent FolderOpenRequestedEvent;     /**< TCloneableString: the VFS path of a folder to show in the content browser */
    static const CORE::CEvent SignInRequestedEvent;         /**< TCloneableString: the id of the store the person wants to sign in to */
    static const CORE::CEvent SignInSettingsRequestedEvent; /**< TCloneableString: the id of the store whose sign in settings the person wants to see ("Sign in settings") */
    static const CORE::CEvent SessionExpiredEvent;          /**< TCloneableString: the id of the store whose session ended, found out by an action of the library; raised once per sign in, the page asks for a sign in again and retries the action once */
    static const CORE::CEvent ServiceChangedEvent;          /**< no data: the page works on another asset store service now (UseService() or UseLocalService()): what holds the service of the page has to take the new one */
    static const CORE::CEvent RemoteContentChangedEvent;    /**< no data: files or lists of folders of a remote store arrived that the views may show now (the content browser reads its folder again) */

    static void RegisterEvents( void );

    static const CORE::CString PrimaryStoreId;              /**< "primary" */

    explicit CAssetSourcesPane( CAssetMounter& mounter );

    virtual ~CAssetSourcesPane() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Makes and opens the asset store service of the settings (the store, the download manager); false when the service cannot be made or
     *  its catalogue cannot be opened (the page works without it and says why)
     */
    bool Open( const CAssetSourcesSettings& settings );

    /**
     *  Makes the page in the widget. 'inspector' shows what is selected, 'transfers' the downloads and the ingests, 'resultThumbnails' (may
     *  be NULL) makes the pictures of the results of the primary store. None of them is owned
     */
    bool Initialize( GUI::CGUIContext& context, GUI::CWidget* host, GUI::CPropertyInspectorForm* inspector, GUI::CTransferQueueForm* transfers, GUI::CIThumbnailProvider* resultThumbnails );

    void Shutdown( void );

    bool IsInitialized( void ) const;

    void Update( UInt32 deltaInMs );

    /*
     *  The stores
     */

    /** The line of the store list, for tests */
    UInt32 GetStoreCount( void ) const;
    CORE::CString GetStoreId( UInt32 index ) const;
    CORE::CString GetStoreLine( UInt32 index ) const;

    /** Makes the store the one that is shown; false for an id that is not in the list */
    bool SelectStore( const CORE::CString& storeId );

    const CORE::CString& GetSelectedStoreId( void ) const;

    /** Reads the registered stores again (after a store plugin was loaded) */
    void RefreshStores( void );

    /** The stores the owner switched off in the settings: they are not listed (the primary store cannot be switched off) */
    void SetDisabledStores( const CORE::CString::StringVector& storeIds );
    const CORE::CString::StringVector& GetDisabledStores( void ) const;

    /** The number of online stores in the list, the primary store not counted */
    UInt32 GetOnlineStoreCount( void ) const;

    /** The state of a service that is not in this process (its connection), or that this build cannot make; empty for the local service */
    CORE::CString GetServiceStateText( void ) const;

    /**
     *  What the line above the stores says when there is something to tell: that no online store is installed and what to do about it, or the
     *  connection state of a service that is not in this process. Empty when there is nothing
     */
    CORE::CString GetOnlineHintText( void ) const;

    /**
     *  Makes the page work on another asset store service, for example the proxy of a remote one (the store of the page in this process stays open
     *  and is what UseLocalService() gives back): the search, the transfers, the details, the online stores and the actions of the page go to it.
     *  'label' says which store that is ("worker.lan:47400 (gucef.assetstore)"), it is shown in the line above the stores. 'info' (may be NULL) is what
     *  the service said of itself, so that the page needs no call that waits to fill its store list. False without a service
     */
    bool UseService( const ASSETSTORE::TAssetStoreServicePtr& service, const CORE::CString& label, const ASSETSTORE::CAssetServiceInfo* info = GUCEF_NULL );

    /** Goes back to the asset store service the page opened itself (the store of this process) */
    bool UseLocalService( void );

    /** Whether the service the page works on is not in this process */
    bool IsUsingRemoteService( void ) const;

    /** The label given with UseService(); empty for the store of this process */
    const CORE::CString& GetServiceLabel( void ) const;

    CORE::CString GetStoreStatusText( void ) const;

    /*
     *  The files of a remote store (remote service design 5.4 and 5.10): while the page works on a service that is not in this process, the
     *  storage locations of the service are mounted at their own VFS roots (ASSETSTORE::CRemoteAssetFileSystem) and adopted by the mounter, so the VFS
     *  paths of the catalogue open unchanged. Opening a result fetches the file set of its asset into the block cache of this computer first (a
     *  transfer on a pool thread, the "Fetching" state of the page, an operation of the activity indicator and a job of the jobs list), then
     *  the views open it. "Download to folder" and "Keep offline" copy or pin the files of a pack or an asset in the same way.
     */

    /** The activity service and the registry of jobs of the workbench (not owned): the fetches are operations of the first and jobs of the second */
    void SetFeedback( GUI::CActivityService* activity, CWorkbenchJobRegistry* registry );

    /** The folder of the block cache and the pictures of the remote files; empty (the default): the folder "remote" in the cache folder of the user */
    void SetRemoteCacheFolder( const CORE::CString& folder );

    /** The prefix of the VFS paths of the mounts of a remote store; empty (the default): the paths of the catalogue open unchanged. For a process that has the service in it too (tests) */
    void SetRemoteMountPrefix( const CORE::CString& prefix );

    ASSETSTORE::CRemoteAssetFileSystem* GetRemoteFiles( void ) const;
    CRemoteFetchJobs* GetFetchJobs( void ) const;
    CRemoteThumbnailSource* GetRemoteThumbnails( void ) const;

    /** Downloads the files of a pack, of an asset with its file set, or of the selected result to a folder of the disk (digest checked, continued after a cut); false with 'message' when that is not possible */
    bool DownloadPack( const CORE::CString& packId, const CORE::CString& folder, CORE::CString& message );
    bool DownloadAsset( const CORE::CString& assetId, const CORE::CString& folder, CORE::CString& message );
    bool DownloadSelected( const CORE::CString& folder, CORE::CString& message );

    /** Keeps the files of a pack (or of the selected result) in the cache of this computer, so they open without the service, or lets them go again */
    bool KeepPackOffline( const CORE::CString& packId, bool keep, CORE::CString& message );

    /** As KeepPackOffline() for one asset: its file set, with the companion files of its relations (found by name for an asset without relations) */
    bool KeepAssetOffline( const CORE::CString& assetId, bool keep, CORE::CString& message );

    /** The selected asset with its companion files, or the selected pack */
    bool KeepSelectedOffline( bool keep, CORE::CString& message );

    /**
     *  Fetches what a path of a remote store needs again (the main file and the cooked output that was written meanwhile, with the lists of the folders of
     *  its pack) and then asks the views to open it (AssetOpenRequestedEvent): for the views that show it again after a conversion; false for a path that is not remote
     */
    bool ReopenRemotePath( const CORE::CString& clientPath );

    /** What the page is fetching in words ("Fetching Crate.fbx"), empty when it fetches nothing for an open */
    CORE::CString GetRemoteOpenText( void ) const;

    /** Whether an open of a result of a remote store is under way */
    bool IsRemoteOpenBusy( void ) const;

    /** How many opens of a result of a remote store were completed (the views were asked to open), for tests */
    UInt32 GetRemoteOpenCount( void ) const;

    /*
     *  Primary store
     */

    bool OpenResult( const CORE::CString& key );

    /** Opens the converted file of the asset of the result in the views; false when the asset was not converted or its location is not reachable */
    bool OpenResultConverted( const CORE::CString& key );

    /** The id of the pack of the result that is selected, empty for none */
    const CORE::CString& GetSelectedPackId( void ) const;

    /** Converts the pack of the selected result of the primary store (the "Convert" action): a pool task, a transfer of the queue; false when there is none, or no converter */
    bool ConvertSelectedPack( void );

    /** Converts the pack with the id; false when the store, the converter or the transfer queue is not there */
    bool ConvertPack( const CORE::CString& packId );

    /** Converts every pack that is "conversion pending", one after the other, as one pool task */
    bool ConvertPendingPacks( void );

    /** The number of conversions this page started, for tests */
    UInt32 GetStartedConversionCount( void ) const;

    /**
     *  The converter hook of the store: the one of the settings, or the one given with SetConverterHook(). Not owned by the caller;
     *  GUCEF_NULL when conversion is switched off in the settings
     */
    ASSETSTORE::CIAssetConverterHook* GetConverterHook( void ) const;

    /** Gives the store another converter hook (not owned, must outlive the page); GUCEF_NULL takes the converter away. For applications with their own converter, and for tests */
    void SetConverterHook( ASSETSTORE::CIAssetConverterHook* hook );

    /** Ingests a pack from the disk: a VFS path (folder or archive) or a path of the disk, which is mounted for it; false when it cannot be found */
    bool AddDiskPack( const CORE::CString& path );

    /** The keep policy and the storage location that the add actions use */
    void SetKeepPolicy( ASSETSTORE::TAssetKeepPolicy keep );
    ASSETSTORE::TAssetKeepPolicy GetKeepPolicy( void ) const;
    void SetLocationId( const CORE::CString& locationId );
    const CORE::CString& GetLocationId( void ) const;

    /*
     *  Online stores
     */

    /** The item of the online store that the browser has selected, empty for none */
    const CORE::CString& GetSelectedItemId( void ) const;

    /** Selects the item with the id in the browser of the online store; false when it is not in the library listing */
    bool SelectItem( const CORE::CString& storeId, const CORE::CString& itemId );

    /** The formats of the selected item, best first (the order of the format list) */
    CORE::CString::StringVector GetFormatChoices( void ) const;
    bool SelectFormat( UInt32 index );
    CORE::CString GetSelectedFormatId( void ) const;

    /** Queues a download of the selected item and format into the download folder; the id of the job in 'jobId' */
    bool ImportSelected( CORE::CString& jobId );

    /** Queues a download of the selected item for the primary store, to be ingested when it is done */
    bool AddSelectedToPrimaryStore( CORE::CString& jobId );

    /** Selects the items with the ids in the browser of the online store (those in the folder of the first one); false when none is in the listing */
    bool SelectItems( const CORE::CString& storeId, const CORE::CString::StringVector& itemIds );

    /** The items selected in the browser of the online store that is shown (several when several are selected), in the order of the selection */
    CORE::CString::StringVector GetSelectedItemIds( void ) const;

    /**
     *  "Download selected": the items selected in the browser of the online store go into the primary store through one library
     *  sync of those items alone (the pipeline job "sync-library" with the itemIds of its spec, the profile of the store), so they get
     *  the same plan as a sync of the library: items that are current already are left out, every format is downloaded, the
     *  admission rules apply. 'jobId' is the id of the job (empty while a service that is not in process has not answered). The
     *  button Add to the primary store does this when more than one item is selected
     */
    bool AddSelectedItemsToPrimaryStore( CORE::CString& jobId );

    /** The VFS folder a download of the selected item goes into */
    CORE::CString GetImportFolder( const CORE::CString& providerId, const CORE::CString& itemId, const CORE::CString& title ) const;

    /*
     *  The parts
     */

    /** The asset store service of the page; GUCEF_NULL when none could be made */
    ASSETSTORE::CIAssetStoreService* GetService( void ) const;

    /** The service as the in-process implementation, GUCEF_NULL for another implementation */
    ASSETSTORE::CLocalAssetStoreService* GetLocalService( void ) const;

    CAssetStoreTransferModel* GetTransferModel( void ) const;
    CPrimaryStoreSearchModel* GetSearchModel( void ) const;
    GUI::CAssetSearchForm* GetSearchForm( void ) const;
    GUI::CContentBrowserForm* GetOnlineBrowser( const CORE::CString& storeId ) const;
    ASSETSTORE::CAssetStoreLibraryFileSystemAccess* GetLibrary( const CORE::CString& storeId ) const;
    const CAssetSourcesSettings& GetSettings( void ) const;

    /** A control of the page by its name ("Stores", "SourceStatus", "SignIn", "SignInSettings", "Format", "Keep", "Location", "Import", "AddToPrimary", "DiskPath", "AddDisk", "Convert"); NULL for another name */
    GUI::CWidget* GetControl( const CORE::CString& name ) const;

    /** Imports that were mounted when they finished, and ingests that the page started for finished downloads, for tests */
    UInt32 GetMountedDownloadCount( void ) const;
    UInt32 GetStartedIngestCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SOnlineStore
    {
        CORE::CString providerId;
        ASSETSTORE::TAssetStoreProviderPtr provider;   /**< the handle of the service for the browsing of the library */
        ASSETSTORE::CAssetStoreLibraryFileSystemAccess* library;
        CORE::TFileSystemAccessPtr access;
        GUI::CContentBrowserForm* browser;
        CAssetStoreThumbnailProvider* thumbnails;
        GUI::CWidget* host;
        CORE::CString message;
        bool signInAsked;               /**< SessionExpiredEvent was raised since the person (or a sign in) last signed in: the retry is used */
        bool sessionEnded;              /**< the store told that the session is over */
        CORE::CString retryItemId;      /**< the details of this item are asked again after the sign in */
    };
    typedef GUCEF::vector< SOnlineStore* > TOnlineVector;


    void OnStoresSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResultOpened( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResultSelected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBrowserSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLibraryChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLibraryFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStoreStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStoresChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFormatValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeepValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLocationValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnImportActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAddActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAddDiskActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSignInActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSignInSettingsActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool AskSignInAgain( SOnlineStore* store, const CORE::CString& reason );
    void OnJobChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConvertActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnServiceConnectionStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool SwitchService( const ASSETSTORE::TAssetStoreServicePtr& service, const CORE::CString& label, const ASSETSTORE::CAssetServiceInfo* info );
    void SubscribeToService( void );
    void ShowRecords( bool havePack, const ASSETSTORE::CAssetPackRecord& pack, bool hasAssetId, bool haveAsset, const ASSETSTORE::CAssetRecord& asset );
    void CheckDetails( void );
    bool OpenAssetFile( const CORE::CString& key, bool converted );
    void ShowResult( const CORE::CString& key );
    void OnOpenFolderRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( void );
    void FillStoreList( void );
    void ShowSelectedStore( void );
    void ShowStatus( void );
    void ShowOnlineHint( void );
    void ShowButtons( void );
    void ShowItem( const ASSETSTORE::CAssetStoreItem& item );
    void ShowFormatChoices( void );
    void ShowItemInspector( void );
    void ClearInspector( void );
    void ShowKeepAndLocations( void );
    SOnlineStore* FindOnline( const CORE::CString& storeId ) const;
    SOnlineStore* MakeOnline( const CORE::CString& storeId );
    SOnlineStore* FindOnlineOfNotifier( CORE::CNotifier* notifier ) const;
    SOnlineStore* GetSelectedOnline( void ) const;
    bool QueueDownload( bool forPrimaryStore, CORE::CString& jobId );
    void MountFinishedDownload( const ASSETSTORE::CAssetJobEntry& job );
    void SessionChanged( const ASSETSTORE::CAssetStoreInfo& info );
    bool GetStoreInfo( const CORE::CString& storeId, ASSETSTORE::CAssetStoreInfo& info ) const;
    void ReadStoreInfos( void );
    bool IsCatalogueAvailable( void ) const;
    bool HasConverter( void ) const;
    CORE::CString GetLocationRoot( const CORE::CString& locationId ) const;
    void AdoptLocations( void );
    void StartRemoteFiles( void );
    void StopRemoteFiles( void );
    bool OpenRemoteResult( const CORE::CString& key, bool converted );
    void UpdateRemote( void );
    void FinishRemoteOpen( const CORE::CString& problem );
    void BeginRemoteOpenFetch( const ASSETSTORE::CAssetFileSet& files );
    bool StartTransferJob( ASSETSTORE::TRemoteFileTransferPtr transfer, const CORE::CString& kind, const CORE::CString& label, CORE::CString& message );
    void OnFetchJobEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRemoteFilesArrived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDownloadActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeepOfflineActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    CORE::CString GetRemoteCacheBase( void ) const;
    CORE::CString GetPackFolderOfClientPath( const CORE::CString& clientPath ) const;
    void AddPackListings( ASSETSTORE::TRemoteFileTransferPtr& transfer, const CORE::CString& clientPackFolder ) const;
    bool GetResultTarget( const CORE::CString& key, CORE::CString& packId, CORE::CString& assetId ) const;
    void AskItemDetails( const CORE::CString& providerId, const CORE::CString& itemId );
    CORE::CString GetThumbnailPath( const CORE::CString& vfsPath ) const;

    CAssetSourcesPane( const CAssetSourcesPane& src );
    CAssetSourcesPane& operator=( const CAssetSourcesPane& src );

    private:

    CAssetMounter& m_mounter;
    CAssetSourcesSettings m_settings;
    GUI::CGUIContext* m_context;
    GUI::CWidget* m_host;
    GUI::CPropertyInspectorForm* m_inspector;
    GUI::CTransferQueueForm* m_transfers;
    GUI::CIThumbnailProvider* m_resultThumbnails;
    ASSETSTORE::TAssetStoreServicePtr m_servicePtr;
    ASSETSTORE::CIAssetStoreService* m_service;
    ASSETSTORE::TAssetStoreServicePtr m_localPtr;       /**< the service this page opened: in this process, kept open while a remote one is in use */
    CORE::CString m_serviceLabel;                       /**< of a service that is not the local one */
    bool m_storeInfosGiven;                             /**< m_storeInfos came with UseService(): the next reading of the store list is not needed */
    CORE::CFutureResult m_detailPack;                   /**< the details of the selected result of a service that is not in this process */
    CORE::CFutureResult m_detailAsset;
    bool m_detailWaiting;
    bool m_detailHasAsset;
    ASSETSTORE::TAssetStoreInfoVector m_storeInfos;
    ASSETSTORECONVERTER::CPipelineConverterHook* m_ownHook;
    ASSETSTORE::CIAssetConverterHook* m_hook;
    CAssetStoreTransferModel* m_transferModel;
    CPrimaryStoreSearchModel* m_searchModel;
    GUI::CAssetSearchForm* m_searchForm;
    TOnlineVector m_online;
    CORE::CString::StringSet m_addJobs;          /**< the downloads this page queued for the primary store */
    CORE::CString::StringSet m_endedDownloads;   /**< the downloads whose end the page handled */
    CORE::CString::StringVector m_storeIds;
    CORE::CString::StringVector m_disabledStores;
    CORE::CString m_selectedStore;
    CORE::CString m_selectedItemId;
    CORE::CString m_selectedProviderId;
    ASSETSTORE::CAssetStoreItem m_item;
    bool m_haveItem;
    GUCEF::vector< UInt32 > m_formatOrder;
    Int32 m_formatIndex;
    CORE::CFutureResult m_itemFuture;
    CORE::CString m_itemFutureId;
    ASSETSTORE::TAssetKeepPolicy m_keep;
    CORE::CString m_locationId;
    CORE::CString::StringVector m_locationIds;
    CORE::CString m_primaryMessage;
    UInt32 m_mountedDownloads;
    UInt32 m_startedIngests;
    UInt32 m_startedConversions;
    CORE::CString m_selectedPackId;
    CORE::CString m_selectedResultKey;
    UInt32 m_diskMounts;
    bool m_hasConverter;                         /**< of a service that is not the local one: what its info said */

    struct SRemoteOpen
    {
        bool active;                             /**< an open of a result of a remote store is under way */
        CORE::CString key;
        CORE::CString assetId;
        CORE::CString name;
        bool converted;
        CORE::CFutureResult future;              /**< the file set of the asset */
        UInt32 jobId;                            /**< the transfer of its files, 0 before it starts */
        CORE::CString openPath;
        CORE::CString folderPath;                /**< a pack: the folder to show when its list is known */
        UInt32 retries;
        bool pack;                               /**< a pack: its folder is shown, not its asset */
        UInt64 finishedTicks;                    /**< when the files of the last open were all in: a miss of the views soon after is fetched */
        CORE::CString packFolder;                /**< the folder of the pack that the resource group is made of when the files are in (the lists of its folders are known then) */
        CORE::CString::StringVector folders;     /**< the folders of the files of the pack */
        SRemoteOpen( void );
    };

    ASSETSTORE::CRemoteAssetFileSystem* m_remoteFiles;
    CRemoteFetchJobs* m_fetchJobs;
    CRemoteThumbnailSource* m_remoteThumbs;
    GUI::CActivityService* m_activity;
    CWorkbenchJobRegistry* m_registry;
    CORE::CString m_remoteCacheFolder;
    CORE::CString m_remotePrefix;
    CORE::CString::StringVector m_remoteRoots;   /**< the locations of the remote store that the mounter adopted */
    SRemoteOpen m_open;
    CORE::CString m_remoteOpenText;
    GUCEF::map< CORE::CString, CORE::CString > m_remoteRootOfLocation;     /**< the VFS root of each location of the remote store, by its id */
    CORE::CString::StringSet m_keptPacks;                                    /**< the packs that were kept offline from this page */
    UInt32 m_remoteOpens;
    UInt32 m_remoteChangedDelay;
    bool m_updatingWidgets;
    bool m_initialized;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETSOURCESPANE_H ? */
