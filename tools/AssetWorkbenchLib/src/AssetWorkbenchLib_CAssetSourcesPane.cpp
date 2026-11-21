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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CLABEL_H
#include "gucefGUI_CLabel.h"
#define GUCEF_GUI_CLABEL_H
#endif /* GUCEF_GUI_CLABEL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#include "gucefASSETSTORE_CAssetStoreSafeNames.h"
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */


#ifndef GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#include "gucefASSETSTORE_CAssetIngestRequest.h"
#define GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#include "AssetWorkbenchLib_CAssetTypeResolver.h"
#include "AssetWorkbenchLib_CCookedOutputResolver.h"
#define ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#endif /* ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSTOREINSPECTORDATA_H
#include "AssetWorkbenchLib_CAssetStoreInspectorData.h"
#define ASSETWORKBENCHLIB_CASSETSTOREINSPECTORDATA_H
#endif /* ASSETWORKBENCHLIB_CASSETSTOREINSPECTORDATA_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSOURCESPANE_H
#ifndef GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H
#include "gucefASSETSTORE_CRemoteFileTransfer.h"
#define GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H
#endif /* GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H ? */

#include <algorithm>

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H ? */

#include "AssetWorkbenchLib_CAssetSourcesPane.h"
#define ASSETWORKBENCHLIB_CASSETSOURCESPANE_H
#endif /* ASSETWORKBENCHLIB_CASSETSOURCESPANE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAssetSourcesPane::AssetOpenRequestedEvent = "GUCEF::ASSETWORKBENCH::CAssetSourcesPane::AssetOpenRequestedEvent";
const CORE::CEvent CAssetSourcesPane::FolderOpenRequestedEvent = "GUCEF::ASSETWORKBENCH::CAssetSourcesPane::FolderOpenRequestedEvent";
const CORE::CEvent CAssetSourcesPane::SignInRequestedEvent = "GUCEF::ASSETWORKBENCH::CAssetSourcesPane::SignInRequestedEvent";
const CORE::CEvent CAssetSourcesPane::SignInSettingsRequestedEvent = "GUCEF::ASSETWORKBENCH::CAssetSourcesPane::SignInSettingsRequestedEvent";
const CORE::CEvent CAssetSourcesPane::SessionExpiredEvent = "GUCEF::ASSETWORKBENCH::CAssetSourcesPane::SessionExpiredEvent";
const CORE::CEvent CAssetSourcesPane::ServiceChangedEvent = "GUCEF::ASSETWORKBENCH::CAssetSourcesPane::ServiceChangedEvent";
const CORE::CEvent CAssetSourcesPane::RemoteContentChangedEvent = "GUCEF::ASSETWORKBENCH::CAssetSourcesPane::RemoteContentChangedEvent";
const CORE::CString CAssetSourcesPane::PrimaryStoreId = "primary";

namespace {

const Int32 StoreListHeight = 72;
const Int32 StatusTop = 78;
const Int32 ActionsHeight = 104;
const Int32 ContentTop = 104;
const char* const KeepNames[] = { "Keep original", "Keep converted", "Keep both" };

GUI::CWidget*
MakeWidget( GUI::CGUIContext& context, const char* type, const char* name, GUI::CWidget* parent, const CORE::CString& position, const CORE::CString& size )
{
    GUI::CWidget* widget = context.CreateWidget( type, name, parent );
    if ( GUCEF_NULL == widget )
        return GUCEF_NULL;
    widget->SetProperty( GUI::EWidgetProperty_Position, position );
    widget->SetProperty( GUI::EWidgetProperty_Size, size );
    return widget;
}

CORE::CString
Int( Int32 value )
{
    return CORE::Int32ToString( value );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


void
CAssetSourcesPane::RegisterEvents( void )
{GUCEF_TRACE;

    AssetOpenRequestedEvent.Initialize();
    FolderOpenRequestedEvent.Initialize();
    SignInRequestedEvent.Initialize();
    SignInSettingsRequestedEvent.Initialize();
    SessionExpiredEvent.Initialize();
    ServiceChangedEvent.Initialize();
    RemoteContentChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetSourcesPane::CAssetSourcesPane( CAssetMounter& mounter )
    : CORE::CObservingNotifier()
    , m_mounter( mounter )
    , m_settings()
    , m_context( GUCEF_NULL )
    , m_host( GUCEF_NULL )
    , m_inspector( GUCEF_NULL )
    , m_transfers( GUCEF_NULL )
    , m_resultThumbnails( GUCEF_NULL )
    , m_servicePtr()
    , m_service( GUCEF_NULL )
    , m_localPtr()
    , m_serviceLabel()
    , m_storeInfosGiven( false )
    , m_detailPack( CORE::TASKSTATUS_UNDEFINED )
    , m_detailAsset( CORE::TASKSTATUS_UNDEFINED )
    , m_detailWaiting( false )
    , m_detailHasAsset( false )
    , m_storeInfos()
    , m_ownHook( GUCEF_NULL )
    , m_hook( GUCEF_NULL )
    , m_transferModel( GUCEF_NULL )
    , m_searchModel( GUCEF_NULL )
    , m_searchForm( GUCEF_NULL )
    , m_online()
    , m_addJobs()
    , m_endedDownloads()
    , m_storeIds()
    , m_selectedStore()
    , m_selectedItemId()
    , m_selectedProviderId()
    , m_item()
    , m_haveItem( false )
    , m_formatOrder()
    , m_formatIndex( -1 )
    , m_itemFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_itemFutureId()
    , m_keep( ASSETSTORE::ASSETKEEP_BOTH )
    , m_locationId()
    , m_locationIds()
    , m_primaryMessage()
    , m_mountedDownloads( 0 )
    , m_startedIngests( 0 )
    , m_startedConversions( 0 )
    , m_selectedPackId()
    , m_diskMounts( 0 )
    , m_hasConverter( false )
    , m_remoteFiles( GUCEF_NULL )
    , m_fetchJobs( GUCEF_NULL )
    , m_remoteThumbs( GUCEF_NEW CRemoteThumbnailSource() )
    , m_activity( GUCEF_NULL )
    , m_registry( GUCEF_NULL )
    , m_remoteCacheFolder()
    , m_remotePrefix()
    , m_remoteRoots()
    , m_open()
    , m_remoteOpenText()
    , m_remoteRootOfLocation()
    , m_keptPacks()
    , m_remoteOpens( 0 )
    , m_remoteChangedDelay( 0 )
    , m_updatingWidgets( false )
    , m_initialized( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAssetSourcesPane::~CAssetSourcesPane()
{GUCEF_TRACE;

    Shutdown();
    GUCEF_DELETE m_fetchJobs;
    m_fetchJobs = GUCEF_NULL;
    GUCEF_DELETE m_remoteThumbs;
    m_remoteThumbs = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::Open( const CAssetSourcesSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    m_keep = settings.primaryStore.defaultPolicy.keep;
    m_locationId = settings.primaryStore.defaultLocationId;

    m_locationIds.clear();
    for ( size_t i=0; i<settings.primaryStore.locations.size(); ++i )
        m_locationIds.push_back( settings.primaryStore.locations[ i ].locationId );

    // the service the settings name: the store of this process by default
    ASSETSTORE::CAssetStoreServiceConfig config;
    config.implementationId = settings.serviceMode.IsNULLOrEmpty() ? ASSETSTORE::CAssetStoreServiceConfig::LocalImplementationId : settings.serviceMode;
    const bool local = ASSETSTORE::CAssetStoreServiceConfig::LocalImplementationId == config.implementationId;
    config.primaryStore = settings.primaryStore;
    config.downloads = settings.downloads;
    config.implementationSettings.Copy( settings.serviceSettings );
    config.implementationSettings.SetName( "implementationSettings" );
    if ( local && settings.conversionEnabled && GUCEF_NULL == m_hook )
    {
        // the converter pipeline with the cook profile of the store: the conversion step of the ingest and the Convert action
        m_ownHook = GUCEF_NEW ASSETSTORECONVERTER::CPipelineConverterHook( settings.conversion );
        m_hook = m_ownHook;
    }
    config.converterHook = local ? m_hook : GUCEF_NULL;
    CORE::CString error;
    m_servicePtr = ASSETSTORE::CAssetStoreGlobal::Instance()->CreateService( config, error );
    m_service = m_servicePtr.GetPointerAlways();
    m_localPtr = m_servicePtr;
    if ( GUCEF_NULL == m_service )
    {
        m_primaryMessage = "The asset store is not available: " + error;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetSourcesPane: " << m_primaryMessage );
        return false;
    }
    m_service->Open();
    const bool opened = m_service->IsCatalogueAvailable();
    if ( !opened )
    {
        m_primaryMessage = m_service->GetStatusMessage();
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetSourcesPane: " << m_primaryMessage );
    }
    if ( !local )
    {
        ASSETSTORE::CAssetServiceInfo info;
        m_hasConverter = ASSETSTORE::ASSETSTORE_STATUS_OK == m_service->GetServiceInfo( info ) && !info.converterName.IsNULLOrEmpty();
    }

    TEventCallback jobCallback( this, &CAssetSourcesPane::OnJobChanged );
    SubscribeTo( m_service, ASSETSTORE::CIAssetStoreService::JobChangedEvent, jobCallback );
    TEventCallback storeCallback( this, &CAssetSourcesPane::OnStoreStateChanged );
    SubscribeTo( m_service, ASSETSTORE::CIAssetStoreService::StoreStateChangedEvent, storeCallback );
    TEventCallback storesCallback( this, &CAssetSourcesPane::OnStoresChanged );
    SubscribeTo( m_service, ASSETSTORE::CIAssetStoreService::StoresChangedEvent, storesCallback );

    m_transferModel = GUCEF_NEW CAssetStoreTransferModel( *m_service );
    m_searchModel = GUCEF_NEW CPrimaryStoreSearchModel( *m_service );
    m_searchModel->SetRemoteThumbnails( m_remoteThumbs );
    if ( opened )
        AdoptLocations();
    return opened;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::AdoptLocations( void )
{GUCEF_TRACE;

    ASSETSTORE::CAssetLocationPage locations;
    if ( GUCEF_NULL == m_service || ASSETSTORE::ASSETSTORE_STATUS_OK != m_service->GetLocations( locations ) )
        return;
    for ( size_t i=0; i<locations.entries.size(); ++i )
        if ( locations.entries[ i ].online )
            m_mounter.Adopt( locations.entries[ i ].location.vfsRoot );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::BuildLayout( void )
{GUCEF_TRACE;

    GUI::CWidget* stores = MakeWidget( *m_context, "Listbox", "Stores", m_host, "0,4 0,4", "1,-8 0," + Int( StoreListHeight ) );
    GUI::CWidget* status = MakeWidget( *m_context, "Label", "SourceStatus", m_host, "0,6 0," + Int( StatusTop ), "1,-248 0,22" );
    GUI::CWidget* onlineHint = MakeWidget( *m_context, "Label", "OnlineHint", m_host, "0,12 0," + Int( 4 + 24 ), "1,-24 0,40" );
    if ( GUCEF_NULL != onlineHint )
    {
        onlineHint->SetBoolProperty( GUI::EWidgetProperty_InputTransparent, true );
        onlineHint->SetBoolProperty( GUI::EWidgetProperty_WordWrap, true );
        onlineHint->SetBoolProperty( GUI::EWidgetProperty_Visible, false );
        onlineHint->SetProperty( GUI::EWidgetProperty_TextColour, "176,182,196,255" );
    }
    GUI::CWidget* signIn = MakeWidget( *m_context, "Button", "SignIn", m_host, "1,-84 0," + Int( StatusTop - 2 ), "0,80 0,26" );
    GUI::CWidget* signInSettings = MakeWidget( *m_context, "Button", "SignInSettings", m_host, "1,-244 0," + Int( StatusTop - 2 ), "0,156 0,26" );
    if ( GUCEF_NULL == stores || GUCEF_NULL == status || GUCEF_NULL == signIn || GUCEF_NULL == signInSettings )
        return;
    signIn->SetProperty( GUI::EWidgetProperty_Text, "Sign in" );
    signIn->SetBoolProperty( GUI::EWidgetProperty_Visible, false );
    signInSettings->SetProperty( GUI::EWidgetProperty_Text, "Sign in settings" );
    signInSettings->SetProperty( GUI::EWidgetProperty_Tooltip, "Sign in and out, saved credentials and the keyring code of this store" );
    signInSettings->SetBoolProperty( GUI::EWidgetProperty_Visible, false );
    status->SetBoolProperty( GUI::EWidgetProperty_InputTransparent, true );

    // the places of the stores below, over each other: the one of the selected store is shown
    const CORE::CString contentSize = "1,-8 1,-" + Int( ContentTop + ActionsHeight + 4 );
    GUI::CWidget* primary = MakeWidget( *m_context, "Panel", "PrimaryHost", m_host, "0,4 0," + Int( ContentTop ), contentSize );
    if ( GUCEF_NULL != primary )
        primary->SetProperty( GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );

    GUI::CWidget* actions = MakeWidget( *m_context, "Panel", "Actions", m_host, "0,4 1,-" + Int( ActionsHeight ), "1,-8 0," + Int( ActionsHeight - 4 ) );
    if ( GUCEF_NULL == actions )
        return;
    actions->SetProperty( GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );
    MakeWidget( *m_context, "Combobox", "Format", actions, "0,0 0,0", "1,0 0,28" );
    MakeWidget( *m_context, "Combobox", "Keep", actions, "0,0 0,32", "0.5,-2 0,28" );
    MakeWidget( *m_context, "Combobox", "Location", actions, "0.5,2 0,32", "0.5,-3 0,28" );
    GUI::CWidget* import = MakeWidget( *m_context, "Button", "Import", actions, "0,0 0,64", "0.4,-2 0,28" );
    GUI::CWidget* add = MakeWidget( *m_context, "Button", "AddToPrimary", actions, "0.4,2 0,64", "0.6,-2 0,28" );
    MakeWidget( *m_context, "Editbox", "DiskPath", actions, "0,0 0,0", "1,0 0,28" );
    GUI::CWidget* addDisk = MakeWidget( *m_context, "Button", "AddDisk", actions, "0,0 0,64", "0.7,-2 0,28" );
    GUI::CWidget* convert = MakeWidget( *m_context, "Button", "Convert", actions, "0.7,2 0,64", "0.3,-2 0,28" );
    // the actions of a remote store take the place of "Add folder or archive" (a pack of this computer cannot be added to a remote store): the folder to download to is the box above
    GUI::CWidget* download = MakeWidget( *m_context, "Button", "Download", actions, "0,0 0,64", "0.35,-2 0,28" );
    GUI::CWidget* keepOffline = MakeWidget( *m_context, "Button", "KeepOffline", actions, "0.35,2 0,64", "0.35,-4 0,28" );
    if ( GUCEF_NULL != download )
    {
        download->SetProperty( GUI::EWidgetProperty_Text, "Download to folder" );
        download->SetProperty( GUI::EWidgetProperty_Tooltip, "Copies the files of the selected result (the pack, or the asset with its files) to the folder typed above: checked against their digests, continued when the link breaks" );
        download->SetBoolProperty( GUI::EWidgetProperty_Visible, false );
    }
    if ( GUCEF_NULL != keepOffline )
    {
        keepOffline->SetProperty( GUI::EWidgetProperty_Text, "Keep offline" );
        keepOffline->SetProperty( GUI::EWidgetProperty_Tooltip, "Keeps the files of the selected result in the cache of this computer, so they open without the service; press again to let them go" );
        keepOffline->SetBoolProperty( GUI::EWidgetProperty_Visible, false );
    }
    if ( GUCEF_NULL != convert )
    {
        convert->SetProperty( GUI::EWidgetProperty_Text, "Convert" );
        convert->SetProperty( GUI::EWidgetProperty_Tooltip, "Converts the pack of the selected result with the converter of the store: for packs that are conversion pending or refused, or again" );
    }
    if ( GUCEF_NULL != import )
        import->SetProperty( GUI::EWidgetProperty_Text, "Import" );
    if ( GUCEF_NULL != add )
        add->SetProperty( GUI::EWidgetProperty_Text, "Add to primary store" );
    if ( GUCEF_NULL != addDisk )
        addDisk->SetProperty( GUI::EWidgetProperty_Text, "Add folder or archive to primary store" );
    GUI::CWidget* path = actions->GetChildWidget( "DiskPath" );
    if ( GUCEF_NULL != path )
        path->SetProperty( GUI::EWidgetProperty_Tooltip, "A folder or archive of the VFS or of the disk" );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::Initialize( GUI::CGUIContext& context, GUI::CWidget* host, GUI::CPropertyInspectorForm* inspector, GUI::CTransferQueueForm* transfers, GUI::CIThumbnailProvider* resultThumbnails )
{GUCEF_TRACE;

    if ( m_initialized )
        return true;
    if ( GUCEF_NULL == host )
        return false;
    m_context = &context;
    m_host = host;
    m_inspector = inspector;
    m_transfers = transfers;
    m_resultThumbnails = resultThumbnails;
    BuildLayout();
    GUI::CWidget* primary = m_host->GetChildWidget( "PrimaryHost" );
    if ( GUCEF_NULL == primary || GUCEF_NULL == m_host->GetChildWidget( "Actions" ) )
        return false;

    m_searchForm = GUCEF_NEW GUI::CAssetSearchForm( m_context );
    m_searchForm->SetFilterPlacement( GUI::CAssetSearchForm::FILTERPLACEMENT_TOP );
    if ( !m_searchForm->Build( primary ) )
        return false;
    if ( GUCEF_NULL != m_resultThumbnails )
        m_searchForm->SetThumbnailProvider( m_resultThumbnails );
    TEventCallback openedCallback( this, &CAssetSourcesPane::OnResultOpened );
    SubscribeTo( m_searchForm, GUI::CAssetSearchForm::ResultOpenedEvent, openedCallback );
    TEventCallback selectedCallback( this, &CAssetSourcesPane::OnResultSelected );
    SubscribeTo( m_searchForm, GUI::CAssetSearchForm::ResultSelectedEvent, selectedCallback );

    GUI::CWidget* widget = m_host->GetChildWidget( "Stores" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnStoresSelectionChanged );
        SubscribeTo( widget, GUI::CWidget::ValueChangedEvent, callback );
    }
    GUI::CWidget* actions = m_host->GetChildWidget( "Actions" );
    const char* const buttons[] = { "Import", "AddToPrimary", "AddDisk" };
    for ( UInt32 i=0; i<3; ++i )
    {
        widget = actions->GetChildWidget( buttons[ i ] );
        if ( GUCEF_NULL == widget )
            continue;
        TEventCallback callback( this, 0 == i ? &CAssetSourcesPane::OnImportActivated : 1 == i ? &CAssetSourcesPane::OnAddActivated : &CAssetSourcesPane::OnAddDiskActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = actions->GetChildWidget( "Convert" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnConvertActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = actions->GetChildWidget( "Download" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnDownloadActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = actions->GetChildWidget( "KeepOffline" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnKeepOfflineActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = m_host->GetChildWidget( "SignIn" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnSignInActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = m_host->GetChildWidget( "SignInSettings" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnSignInSettingsActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = actions->GetChildWidget( "Format" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnFormatValueChanged );
        SubscribeTo( widget, GUI::CWidget::ValueChangedEvent, callback );
    }
    widget = actions->GetChildWidget( "Keep" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnKeepValueChanged );
        SubscribeTo( widget, GUI::CWidget::ValueChangedEvent, callback );
    }
    widget = actions->GetChildWidget( "Location" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnLocationValueChanged );
        SubscribeTo( widget, GUI::CWidget::ValueChangedEvent, callback );
    }
    if ( GUCEF_NULL != m_transfers )
    {
        TEventCallback callback( this, &CAssetSourcesPane::OnOpenFolderRequested );
        SubscribeTo( m_transfers, GUI::CTransferQueueForm::OpenFolderRequestedEvent, callback );
        m_transfers->SetModel( m_transferModel );
    }
    if ( GUCEF_NULL != m_searchModel )
        m_searchForm->SetModel( m_searchModel );

    m_initialized = true;
    ShowKeepAndLocations();
    RefreshStores();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::Shutdown( void )
{GUCEF_TRACE;

    StopRemoteFiles();
    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_transfers )
        m_transfers->SetModel( GUCEF_NULL );
    GUCEF_DELETE m_searchForm;
    m_searchForm = GUCEF_NULL;
    for ( size_t i=0; i<m_online.size(); ++i )
    {
        GUCEF_DELETE m_online[ i ]->browser;
        GUCEF_DELETE m_online[ i ]->thumbnails;
        m_online[ i ]->access.Unlink();
        GUCEF_DELETE m_online[ i ];
    }
    m_online.clear();
    GUCEF_DELETE m_transferModel;
    m_transferModel = GUCEF_NULL;
    GUCEF_DELETE m_searchModel;
    m_searchModel = GUCEF_NULL;
    // a remote service in use is not the page's: it is let go of, the service the page opened is the one that is closed
    if ( !m_localPtr.IsNULL() && m_localPtr.GetPointerAlways() != m_service )
    {
        m_servicePtr = m_localPtr;
        m_service = m_localPtr.GetPointerAlways();
    }
    if ( GUCEF_NULL != m_service )
        m_service->Close();
    m_servicePtr.Unlink();
    m_service = GUCEF_NULL;
    m_localPtr.Unlink();
    m_serviceLabel.Clear();
    m_detailWaiting = false;
    m_detailPack = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    m_detailAsset = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    m_storeInfos.clear();
    m_addJobs.clear();
    m_endedDownloads.clear();
    GUCEF_DELETE m_ownHook;
    m_ownHook = GUCEF_NULL;
    m_hook = GUCEF_NULL;
    m_selectedPackId.Clear();
    m_haveItem = false;
    m_initialized = false;
    m_context = GUCEF_NULL;
    m_host = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::RefreshStores( void )
{GUCEF_TRACE;

    m_storeIds.clear();
    m_storeIds.push_back( PrimaryStoreId );
    // the online stores of the service, with their sign in state: the service tells when it changes
    ReadStoreInfos();
    CORE::CString::StringVector online;
    for ( size_t i=0; i<m_storeInfos.size(); ++i )
    {
        if ( m_storeInfos[ i ].storeId == PrimaryStoreId )
            continue;
        if ( std::find( m_disabledStores.begin(), m_disabledStores.end(), m_storeInfos[ i ].storeId ) != m_disabledStores.end() )
            continue;
        m_storeIds.push_back( m_storeInfos[ i ].storeId );
        online.push_back( m_storeInfos[ i ].storeId );
    }
    if ( GUCEF_NULL != m_searchModel )
    {
        CORE::CString::StringVector sources;
        sources.push_back( "disk" );
        for ( size_t i=0; i<online.size(); ++i )
            sources.push_back( online[ i ] );
        m_searchModel->SetSourceIds( sources );
    }
    if ( m_selectedStore.IsNULLOrEmpty() )
        m_selectedStore = PrimaryStoreId;
    FillStoreList();
    ShowSelectedStore();
    ShowOnlineHint();
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::SetDisabledStores( const CORE::CString::StringVector& storeIds )
{GUCEF_TRACE;

    if ( storeIds == m_disabledStores )
        return;
    m_disabledStores = storeIds;
    // a store that goes while it is selected leaves the primary store selected
    if ( std::find( m_disabledStores.begin(), m_disabledStores.end(), m_selectedStore ) != m_disabledStores.end() )
        m_selectedStore = PrimaryStoreId;
    if ( m_initialized )
        RefreshStores();
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CAssetSourcesPane::GetDisabledStores( void ) const
{GUCEF_TRACE;

    return m_disabledStores;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSourcesPane::GetOnlineStoreCount( void ) const
{GUCEF_TRACE;

    return m_storeIds.empty() ? 0 : (UInt32) m_storeIds.size() - 1;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetServiceStateText( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service && !m_service->IsInProcess() )
    {
        const CORE::CString message = m_service->GetStatusMessage();
        if ( !m_serviceLabel.IsNULLOrEmpty() )
            return "Remote store " + m_serviceLabel + ( message.IsNULLOrEmpty() ? CORE::CString( ": connected" ) : ": " + message );
        return "Remote asset service (" + m_service->GetType() + "): " + ( m_service->IsOpen() ? CORE::CString( "connected" ) : CORE::CString( "not connected" ) ) +
               ( message.IsNULLOrEmpty() ? CORE::CString::Empty : ", " + message );
    }
    // a service of another kind than the local one that this build cannot make: the online stores are behind it
    if ( GUCEF_NULL == m_service && m_initialized && m_settings.serviceMode != CAssetSourcesSettings::LocalServiceMode )
        return "The asset store service \"" + m_settings.serviceMode + "\" is not available in this build or cannot be reached: see the report";
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetOnlineHintText( void ) const
{GUCEF_TRACE;

    const CORE::CString serviceState = GetServiceStateText();
    if ( !serviceState.IsNULLOrEmpty() )
        return serviceState;
    if ( 0 == GetOnlineStoreCount() )
        return m_disabledStores.empty() ? CORE::CString( "No online store is installed: see Settings, Sources" ) : CORE::CString( "The online stores are switched off: see Settings, Sources" );
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowOnlineHint( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_host )
        return;
    GUI::CWidget* hint = m_host->GetChildWidget( "OnlineHint" );
    if ( GUCEF_NULL == hint )
        return;
    const CORE::CString text = GetOnlineHintText();
    hint->SetProperty( GUI::EWidgetProperty_Text, text );
    hint->SetBoolProperty( GUI::EWidgetProperty_Visible, !text.IsNULLOrEmpty() );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSourcesPane::GetStoreCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_storeIds.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetStoreId( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_storeIds.size() ? m_storeIds[ index ] : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetStoreLine( UInt32 index ) const
{GUCEF_TRACE;

    if ( index >= (UInt32) m_storeIds.size() )
        return CORE::CString::Empty;
    if ( PrimaryStoreId == m_storeIds[ index ] )
        return IsCatalogueAvailable() ? CORE::CString( "Primary store" ) : CORE::CString( "Primary store - not available" );
    ASSETSTORE::CAssetStoreInfo info;
    if ( !GetStoreInfo( m_storeIds[ index ], info ) )
        return m_storeIds[ index ];
    const CORE::CString name = info.displayName;
    if ( ASSETSTORE::ASSETSTORE_SIGNIN_NONE == info.signInKind )
        return name;
    if ( info.signedIn )
        return name + " - signed in";
    SOnlineStore* store = FindOnline( m_storeIds[ index ] );
    return name + ( GUCEF_NULL != store && store->sessionEnded ? " - session ended" : " - sign in required" );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::FillStoreList( void )
{GUCEF_TRACE;

    GUI::CListbox* list = GUI::CWidget::As< GUI::CListbox >( GUCEF_NULL != m_host ? m_host->GetChildWidget( "Stores" ) : GUCEF_NULL );
    if ( GUCEF_NULL == list )
        return;
    GUI::CListbox::TStringVector lines;
    Int32 selected = 0;
    for ( UInt32 i=0; i<(UInt32) m_storeIds.size(); ++i )
    {
        lines.push_back( GetStoreLine( i ) );
        if ( m_storeIds[ i ] == m_selectedStore )
            selected = (Int32) i;
    }
    m_updatingWidgets = true;
    list->SetItems( lines );
    list->SetSelectedIndex( selected );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::SelectStore( const CORE::CString& storeId )
{GUCEF_TRACE;

    bool known = false;
    for ( size_t i=0; i<m_storeIds.size(); ++i )
        known = known || m_storeIds[ i ] == storeId;
    if ( !known )
        return false;
    m_selectedStore = storeId;
    m_haveItem = false;
    m_selectedItemId.Clear();
    FillStoreList();
    ShowSelectedStore();
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSourcesPane::GetSelectedStoreId( void ) const
{GUCEF_TRACE;

    return m_selectedStore;
}

/*-------------------------------------------------------------------------*/

CAssetSourcesPane::SOnlineStore*
CAssetSourcesPane::FindOnline( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_online.size(); ++i )
        if ( m_online[ i ]->providerId == storeId )
            return m_online[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAssetSourcesPane::SOnlineStore*
CAssetSourcesPane::FindOnlineOfNotifier( CORE::CNotifier* notifier ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_online.size(); ++i )
        if ( m_online[ i ]->library == notifier || m_online[ i ]->browser == notifier )
            return m_online[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAssetSourcesPane::SOnlineStore*
CAssetSourcesPane::GetSelectedOnline( void ) const
{GUCEF_TRACE;

    return FindOnline( m_selectedStore );
}

/*-------------------------------------------------------------------------*/

CAssetSourcesPane::SOnlineStore*
CAssetSourcesPane::MakeOnline( const CORE::CString& storeId )
{GUCEF_TRACE;

    SOnlineStore* existing = FindOnline( storeId );
    if ( GUCEF_NULL != existing )
        return existing;
    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( GUCEF_NULL == m_service || !m_service->GetStoreProvider( storeId, provider ) || provider.IsNULL() || GUCEF_NULL == m_context || GUCEF_NULL == m_host )
        return GUCEF_NULL;
    const CORE::CString contentSize = "1,-8 1,-" + Int( ContentTop + ActionsHeight + 4 );
    GUI::CWidget* hostWidget = MakeWidget( *m_context, "Panel", ( "OnlineHost_" + storeId ).C_String(), m_host, "0,4 0," + Int( ContentTop ), contentSize );
    if ( GUCEF_NULL == hostWidget )
        return GUCEF_NULL;
    hostWidget->SetProperty( GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );

    SOnlineStore* store = GUCEF_NEW SOnlineStore();
    store->providerId = storeId;
    store->signInAsked = false;
    store->sessionEnded = false;
    store->provider = provider;
    store->library = GUCEF_NEW ASSETSTORE::CAssetStoreLibraryFileSystemAccess( provider );
    store->access = CORE::TFileSystemAccessPtr( store->library );
    store->host = hostWidget;
    store->thumbnails = GUCEF_NEW CAssetStoreThumbnailProvider( *m_service, *store->library );
    store->browser = GUCEF_NEW GUI::CContentBrowserForm( m_context );
    store->browser->Build( hostWidget );
    store->browser->AddRoot( provider->GetDisplayName(), store->access );
    store->browser->SetThumbnailProvider( store->thumbnails );
    store->browser->SetViewMode( GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS );
    m_online.push_back( store );

    TEventCallback selectionCallback( this, &CAssetSourcesPane::OnBrowserSelectionChanged );
    SubscribeTo( store->browser, GUI::CContentBrowserForm::SelectionChangedEvent, selectionCallback );
    TEventCallback changedCallback( this, &CAssetSourcesPane::OnLibraryChanged );
    SubscribeTo( store->library, ASSETSTORE::CAssetStoreLibraryFileSystemAccess::LibraryChangedEvent, changedCallback );
    TEventCallback failedCallback( this, &CAssetSourcesPane::OnLibraryFailed );
    SubscribeTo( store->library, ASSETSTORE::CAssetStoreLibraryFileSystemAccess::RefreshFailedEvent, failedCallback );
    store->library->Refresh();
    return store;
}

/*-------------------------------------------------------------------------*/

GUI::CContentBrowserForm*
CAssetSourcesPane::GetOnlineBrowser( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    SOnlineStore* store = FindOnline( storeId );
    return GUCEF_NULL != store ? store->browser : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::CAssetStoreLibraryFileSystemAccess*
CAssetSourcesPane::GetLibrary( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    SOnlineStore* store = FindOnline( storeId );
    return GUCEF_NULL != store ? store->library : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowSelectedStore( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_host )
        return;
    const bool primary = PrimaryStoreId == m_selectedStore;
    GUI::CWidget* primaryHost = m_host->GetChildWidget( "PrimaryHost" );
    if ( GUCEF_NULL != primaryHost )
        primaryHost->SetBoolProperty( GUI::EWidgetProperty_Visible, primary );
    if ( !primary )
        MakeOnline( m_selectedStore );
    for ( size_t i=0; i<m_online.size(); ++i )
        m_online[ i ]->host->SetBoolProperty( GUI::EWidgetProperty_Visible, m_online[ i ]->providerId == m_selectedStore );
    GUI::CWidget* actions = m_host->GetChildWidget( "Actions" );
    if ( GUCEF_NULL != actions )
    {
        actions->GetChildWidget( "Format" )->SetBoolProperty( GUI::EWidgetProperty_Visible, !primary );
        actions->GetChildWidget( "Import" )->SetBoolProperty( GUI::EWidgetProperty_Visible, !primary );
        actions->GetChildWidget( "AddToPrimary" )->SetBoolProperty( GUI::EWidgetProperty_Visible, !primary );
        const bool remote = IsUsingRemoteService();
        actions->GetChildWidget( "DiskPath" )->SetBoolProperty( GUI::EWidgetProperty_Visible, primary );
        actions->GetChildWidget( "AddDisk" )->SetBoolProperty( GUI::EWidgetProperty_Visible, primary && !remote );
        actions->GetChildWidget( "Convert" )->SetBoolProperty( GUI::EWidgetProperty_Visible, primary );
        GUI::CWidget* download = actions->GetChildWidget( "Download" );
        if ( GUCEF_NULL != download )
            download->SetBoolProperty( GUI::EWidgetProperty_Visible, primary && remote );
        GUI::CWidget* keepOffline = actions->GetChildWidget( "KeepOffline" );
        if ( GUCEF_NULL != keepOffline )
            keepOffline->SetBoolProperty( GUI::EWidgetProperty_Visible, primary && remote );
    }
    ShowFormatChoices();
    ShowStatus();
    ShowButtons();
    if ( primary )
        ClearInspector();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetStoreStatusText( void ) const
{GUCEF_TRACE;

    if ( PrimaryStoreId == m_selectedStore )
    {
        if ( !m_remoteOpenText.IsNULLOrEmpty() )
            return m_remoteOpenText;
        if ( !IsCatalogueAvailable() )
            return m_primaryMessage.IsNULLOrEmpty() ? CORE::CString( "The primary store is not available" ) : m_primaryMessage;
        return m_searchForm ? m_searchForm->GetStatusText() : CORE::CString::Empty;
    }
    SOnlineStore* store = FindOnline( m_selectedStore );
    if ( GUCEF_NULL == store )
        return CORE::CString::Empty;
    ASSETSTORE::CAssetStoreInfo info;
    GetStoreInfo( store->providerId, info );
    if ( ASSETSTORE::ASSETSTORE_SIGNIN_NONE != info.signInKind && !info.signedIn )
    {
        if ( !store->sessionEnded )
            return "Sign in required";
        // what the page knows of the end of the session, else the invitation
        return store->message.IsNULLOrEmpty() ? CORE::CString( "The session ended: sign in again" ) : store->message;
    }
    if ( !store->message.IsNULLOrEmpty() )
        return store->message;
    if ( store->library->IsLoading() )
        return "Reading the library...";
    return CORE::UInt32ToString( store->library->GetItemCount() ) + " items";
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowStatus( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_host )
        return;
    GUI::CWidget* status = m_host->GetChildWidget( "SourceStatus" );
    if ( GUCEF_NULL != status )
        status->SetProperty( GUI::EWidgetProperty_Text, GetStoreStatusText() );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowButtons( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_host )
        return;
    GUI::CWidget* signIn = m_host->GetChildWidget( "SignIn" );
    GUI::CWidget* signInSettings = m_host->GetChildWidget( "SignInSettings" );
    SOnlineStore* store = GetSelectedOnline();
    ASSETSTORE::CAssetStoreInfo info;
    const bool hasSignIn = GUCEF_NULL != store && GetStoreInfo( store->providerId, info ) && ASSETSTORE::ASSETSTORE_SIGNIN_NONE != info.signInKind;
    const bool needsSignIn = hasSignIn && !info.signedIn;
    if ( GUCEF_NULL != signIn )
        signIn->SetBoolProperty( GUI::EWidgetProperty_Visible, needsSignIn );
    if ( GUCEF_NULL != signInSettings )
        signInSettings->SetBoolProperty( GUI::EWidgetProperty_Visible, hasSignIn );
    GUI::CWidget* actions = m_host->GetChildWidget( "Actions" );
    if ( GUCEF_NULL == actions )
        return;
    const bool item = GUCEF_NULL != store && m_haveItem && !needsSignIn && m_formatIndex >= 0;
    actions->GetChildWidget( "Import" )->SetBoolProperty( GUI::EWidgetProperty_Enabled, item );
    actions->GetChildWidget( "AddToPrimary" )->SetBoolProperty( GUI::EWidgetProperty_Enabled, item && IsCatalogueAvailable() );
    actions->GetChildWidget( "AddDisk" )->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsCatalogueAvailable() );
    actions->GetChildWidget( "Convert" )->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsCatalogueAvailable() && HasConverter() && !m_selectedPackId.IsNULLOrEmpty() );
    GUI::CWidget* download = actions->GetChildWidget( "Download" );
    if ( GUCEF_NULL != download )
        download->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsCatalogueAvailable() && !m_selectedPackId.IsNULLOrEmpty() && GUCEF_NULL != m_remoteFiles );
    GUI::CWidget* keepOffline = actions->GetChildWidget( "KeepOffline" );
    if ( GUCEF_NULL != keepOffline )
        keepOffline->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsCatalogueAvailable() && !m_selectedPackId.IsNULLOrEmpty() && GUCEF_NULL != m_remoteFiles );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowKeepAndLocations( void )
{GUCEF_TRACE;

    GUI::CWidget* actions = GUCEF_NULL != m_host ? m_host->GetChildWidget( "Actions" ) : GUCEF_NULL;
    if ( GUCEF_NULL == actions )
        return;
    GUI::CCombobox* keep = GUI::CWidget::As< GUI::CCombobox >( actions->GetChildWidget( "Keep" ) );
    GUI::CCombobox* location = GUI::CWidget::As< GUI::CCombobox >( actions->GetChildWidget( "Location" ) );
    m_updatingWidgets = true;
    if ( GUCEF_NULL != keep )
    {
        GUI::CCombobox::TStringVector items;
        for ( UInt32 i=0; i<3; ++i )
            items.push_back( KeepNames[ i ] );
        keep->SetListItems( items );
        keep->SetSelectedIndex( (Int32) m_keep );
    }
    if ( GUCEF_NULL != location )
    {
        GUI::CCombobox::TStringVector items;
        Int32 selected = 0;
        for ( size_t i=0; i<m_settings.primaryStore.locations.size(); ++i )
        {
            const ASSETSTORE::CAssetStorageLocation& entry = m_settings.primaryStore.locations[ i ];
            items.push_back( entry.name.IsNULLOrEmpty() ? entry.locationId : entry.name );
            if ( entry.locationId == m_locationId )
                selected = (Int32) i;
        }
        location->SetListItems( items );
        if ( !items.empty() )
            location->SetSelectedIndex( selected );
    }
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::SetKeepPolicy( ASSETSTORE::TAssetKeepPolicy keep )
{GUCEF_TRACE;

    m_keep = keep;
    ShowKeepAndLocations();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetKeepPolicy
CAssetSourcesPane::GetKeepPolicy( void ) const
{GUCEF_TRACE;

    return m_keep;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::SetLocationId( const CORE::CString& locationId )
{GUCEF_TRACE;

    m_locationId = locationId;
    ShowKeepAndLocations();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSourcesPane::GetLocationId( void ) const
{GUCEF_TRACE;

    return m_locationId;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ClearInspector( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_inspector )
        m_inspector->SetDataCopy( CORE::CDataNode() );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowFormatChoices( void )
{GUCEF_TRACE;

    GUI::CWidget* actions = GUCEF_NULL != m_host ? m_host->GetChildWidget( "Actions" ) : GUCEF_NULL;
    GUI::CCombobox* format = GUCEF_NULL != actions ? GUI::CWidget::As< GUI::CCombobox >( actions->GetChildWidget( "Format" ) ) : GUCEF_NULL;
    m_formatOrder.clear();
    m_formatIndex = -1;
    GUI::CCombobox::TStringVector items;
    if ( m_haveItem )
    {
        m_settings.RankFormats( m_item.formats, m_formatOrder );
        for ( size_t i=0; i<m_formatOrder.size(); ++i )
        {
            const ASSETSTORE::CAssetStoreFormat& entry = m_item.formats[ m_formatOrder[ i ] ];
            items.push_back( entry.name.IsNULLOrEmpty() ? entry.formatId : entry.name );
        }
        if ( !items.empty() )
            m_formatIndex = 0;
    }
    if ( GUCEF_NULL != format )
    {
        m_updatingWidgets = true;
        format->SetListItems( items );
        if ( !items.empty() )
            format->SetSelectedIndex( 0 );
        m_updatingWidgets = false;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CAssetSourcesPane::GetFormatChoices( void ) const
{GUCEF_TRACE;

    CORE::CString::StringVector choices;
    if ( m_haveItem )
        for ( size_t i=0; i<m_formatOrder.size(); ++i )
            choices.push_back( m_item.formats[ m_formatOrder[ i ] ].formatId );
    return choices;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::SelectFormat( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_formatOrder.size() )
        return false;
    m_formatIndex = (Int32) index;
    GUI::CWidget* actions = GUCEF_NULL != m_host ? m_host->GetChildWidget( "Actions" ) : GUCEF_NULL;
    GUI::CCombobox* format = GUCEF_NULL != actions ? GUI::CWidget::As< GUI::CCombobox >( actions->GetChildWidget( "Format" ) ) : GUCEF_NULL;
    if ( GUCEF_NULL != format && format->GetSelectedIndex() != (Int32) index )
    {
        m_updatingWidgets = true;
        format->SetSelectedIndex( (Int32) index );
        m_updatingWidgets = false;
    }
    ShowItemInspector();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetSelectedFormatId( void ) const
{GUCEF_TRACE;

    if ( !m_haveItem || m_formatIndex < 0 || m_formatIndex >= (Int32) m_formatOrder.size() )
        return CORE::CString::Empty;
    return m_item.formats[ m_formatOrder[ m_formatIndex ] ].formatId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSourcesPane::GetSelectedItemId( void ) const
{GUCEF_TRACE;

    return m_selectedItemId;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowItemInspector( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_inspector || !m_haveItem )
        return;
    CORE::CDataNode data;
    CAssetStoreInspectorData::FillStoreItem( m_item, GetSelectedFormatId(), data );
    m_inspector->ClearDescriptors();
    m_inspector->SetDataCopy( data );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowItem( const ASSETSTORE::CAssetStoreItem& item )
{GUCEF_TRACE;

    const bool sameItem = m_haveItem && m_item.itemId == item.itemId;
    const CORE::CString keepFormat = sameItem ? GetSelectedFormatId() : CORE::CString::Empty;
    m_item = item;
    m_haveItem = true;
    ShowFormatChoices();
    if ( !keepFormat.IsNULLOrEmpty() )
    {
        for ( size_t i=0; i<m_formatOrder.size(); ++i )
        {
            if ( m_item.formats[ m_formatOrder[ i ] ].formatId == keepFormat )
            {
                SelectFormat( (UInt32) i );
                break;
            }
        }
    }
    ShowItemInspector();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::AskItemDetails( const CORE::CString& providerId, const CORE::CString& itemId )
{GUCEF_TRACE;

    SOnlineStore* store = FindOnline( providerId );
    if ( GUCEF_NULL == store )
        return;
    m_itemFutureId = itemId;
    m_itemFuture = m_service->GetStoreItemAsync( providerId, itemId );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnBrowserSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SOnlineStore* store = FindOnlineOfNotifier( notifier );
    if ( GUCEF_NULL == store )
        return;
    const CORE::CString::StringVector paths = store->browser->GetSelectedPaths();
    if ( 1 != paths.size() )
        return;
    const CORE::CString itemId = store->library->GetItemIdOfPath( paths[ 0 ] );
    if ( itemId.IsNULLOrEmpty() )
        return;
    m_selectedProviderId = store->providerId;
    m_selectedItemId = itemId;
    ASSETSTORE::CAssetStoreItem item;
    if ( store->library->TryGetItem( paths[ 0 ], item ) )
        ShowItem( item );
    AskItemDetails( store->providerId, itemId );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::SelectItem( const CORE::CString& storeId, const CORE::CString& itemId )
{GUCEF_TRACE;

    SOnlineStore* store = FindOnline( storeId );
    if ( GUCEF_NULL == store )
        return false;
    const CORE::CString path = store->library->GetPathOfItem( itemId );
    if ( path.IsNULLOrEmpty() )
        return false;
    // the folder of the item, then the item
    const Int32 slash = path.HasChar( '/', false );
    store->browser->NavigateTo( store->browser->GetCurrentRootName(), slash > 0 ? path.SubstrToIndex( (UInt32) slash, true ) : CORE::CString( "/" ) );
    CORE::CString::StringVector names;
    names.push_back( slash >= 0 ? path.CutChars( (UInt32) slash + 1, true ) : path );
    return store->browser->SelectNames( names );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnLibraryChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SOnlineStore* store = FindOnlineOfNotifier( notifier );
    if ( GUCEF_NULL == store )
        return;
    store->message.Clear();
    store->signInAsked = false;
    store->browser->Refresh();
    ShowStatus();
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnLibraryFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SOnlineStore* store = FindOnlineOfNotifier( notifier );
    if ( GUCEF_NULL == store )
        return;
    const CORE::TCloneableString* message = static_cast< const CORE::TCloneableString* >( eventData );
    store->message = GUCEF_NULL != message ? message->GetData() : CORE::CString( "The library cannot be read" );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: The library of " << store->providerId << " cannot be read: " << store->message );
    const ASSETSTORE::TAssetStoreStatus status = store->library->GetLastStatus();
    if ( ASSETSTORE::ASSETSTORE_STATUS_SESSION_EXPIRED == status )
    {
        // the session is over (not "never signed in": that is what the line of the store says): signing in again is asked for once,
        // and the reading of the library is tried again when it is done
        store->sessionEnded = true;
        if ( AskSignInAgain( store, store->message ) )
            store->message.Clear();
        else
            store->message = "The session ended and signing in again did not help";
    }
    FillStoreList();
    ShowStatus();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::AskSignInAgain( SOnlineStore* store, const CORE::CString& reason )
{GUCEF_TRACE;

    ASSETSTORE::CAssetStoreInfo info;
    if ( GUCEF_NULL == store || !GetStoreInfo( store->providerId, info ) || ASSETSTORE::ASSETSTORE_SIGNIN_NONE == info.signInKind || store->signInAsked )
        return false;
    store->signInAsked = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: The session of " << store->providerId << " is over: sign in again is asked for" );
    CORE::TCloneableString data( store->providerId );
    NotifyObservers( SessionExpiredEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnStoreStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetStoreInfo* info = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetStoreInfo >( eventData );
    if ( GUCEF_NULL == info )
        return;
    bool known = false;
    for ( size_t i=0; i<m_storeInfos.size(); ++i )
    {
        if ( m_storeInfos[ i ].storeId == info->storeId )
        {
            m_storeInfos[ i ] = *info;
            known = true;
        }
    }
    if ( !known )
        m_storeInfos.push_back( *info );
    SessionChanged( *info );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::SessionChanged( const ASSETSTORE::CAssetStoreInfo& info )
{GUCEF_TRACE;

    if ( info.sessionEnded )
    {
        // a store that signs in on its web site told that the session ended, as apart from a sign out
        SOnlineStore* store = FindOnline( info.storeId );
        if ( GUCEF_NULL == store )
            return;
        store->sessionEnded = true;
        FillStoreList();
        ShowStatus();
        ShowButtons();
        return;
    }

    // a sign in that worked ends what is known of the end of the session and gives the actions that failed their one more try
    SOnlineStore* changed = FindOnline( info.storeId );
    if ( GUCEF_NULL != changed && info.signedIn )
    {
        changed->sessionEnded = false;
        if ( !changed->retryItemId.IsNULLOrEmpty() )
        {
            const CORE::CString itemId = changed->retryItemId;
            changed->retryItemId.Clear();
            if ( itemId == m_selectedItemId )
                AskItemDetails( changed->providerId, itemId );
        }
    }
    FillStoreList();
    ShowStatus();
    ShowButtons();
    SOnlineStore* store = GetSelectedOnline();
    ASSETSTORE::CAssetStoreInfo selected;
    if ( GUCEF_NULL != changed && info.signedIn )
        changed->library->Refresh();
    else
    if ( GUCEF_NULL != store && GetStoreInfo( store->providerId, selected ) && selected.signedIn )
        store->library->Refresh();
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnStoresChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // what the page shows of the stores is read again with RefreshStores(), as when a store plugin was loaded
    ReadStoreInfos();
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ReadStoreInfos( void )
{GUCEF_TRACE;

    if ( m_storeInfosGiven )
    {
        // what the service said of itself when the page took it
        m_storeInfosGiven = false;
        return;
    }
    m_storeInfos.clear();
    ASSETSTORE::CAssetStoreInfoPage stores;
    if ( GUCEF_NULL != m_service && ASSETSTORE::ASSETSTORE_STATUS_OK == m_service->ListStores( stores ) )
        m_storeInfos = stores.entries;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::GetStoreInfo( const CORE::CString& storeId, ASSETSTORE::CAssetStoreInfo& info ) const
{GUCEF_TRACE;

    const ASSETSTORE::CAssetStoreInfo* cached = GUCEF_NULL;
    for ( size_t i=0; i<m_storeInfos.size(); ++i )
        if ( m_storeInfos[ i ].storeId == storeId )
            cached = &m_storeInfos[ i ];
    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( GUCEF_NULL != m_service && m_service->IsInProcess() && m_service->GetStoreProvider( storeId, provider ) && !provider.IsNULL() )
    {
        // the store of this process answers at once: its state as it is now
        info.storeId = storeId;
        info.displayName = provider->GetDisplayName();
        info.iconName = provider->GetIconName();
        info.signInKind = provider->GetSignInKind();
        info.signedIn = provider->IsSignedIn();
        info.sessionEnded = GUCEF_NULL != cached && cached->sessionEnded;
        return true;
    }
    if ( GUCEF_NULL == cached )
        return false;
    info = *cached;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::IsCatalogueAvailable( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_service && m_service->IsCatalogueAvailable();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::HasConverter( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != GetLocalService() ? GUCEF_NULL != m_hook : m_hasConverter;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetLocationRoot( const CORE::CString& locationId ) const
{GUCEF_TRACE;

    ASSETSTORE::CAssetLocationPage locations;
    if ( GUCEF_NULL == m_service || ASSETSTORE::ASSETSTORE_STATUS_OK != m_service->GetLocations( locations ) )
        return CORE::CString::Empty;
    for ( size_t i=0; i<locations.entries.size(); ++i )
        if ( locations.entries[ i ].location.locationId == locationId )
            return locations.entries[ i ].location.vfsRoot;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnStoresSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    GUI::CListbox* list = GUI::CWidget::As< GUI::CListbox >( m_host->GetChildWidget( "Stores" ) );
    if ( GUCEF_NULL == list )
        return;
    const Int32 index = list->GetSelectedIndex();
    if ( index >= 0 && index < (Int32) m_storeIds.size() && m_storeIds[ index ] != m_selectedStore )
        SelectStore( m_storeIds[ index ] );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnFormatValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    GUI::CWidget* actions = m_host->GetChildWidget( "Actions" );
    GUI::CCombobox* format = GUI::CWidget::As< GUI::CCombobox >( actions->GetChildWidget( "Format" ) );
    if ( GUCEF_NULL != format && format->GetSelectedIndex() >= 0 )
        SelectFormat( (UInt32) format->GetSelectedIndex() );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnKeepValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    GUI::CCombobox* keep = GUI::CWidget::As< GUI::CCombobox >( m_host->GetChildWidget( "Actions" )->GetChildWidget( "Keep" ) );
    if ( GUCEF_NULL != keep && keep->GetSelectedIndex() >= 0 && keep->GetSelectedIndex() < 3 )
        m_keep = static_cast< ASSETSTORE::TAssetKeepPolicy >( keep->GetSelectedIndex() );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnLocationValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    GUI::CCombobox* location = GUI::CWidget::As< GUI::CCombobox >( m_host->GetChildWidget( "Actions" )->GetChildWidget( "Location" ) );
    if ( GUCEF_NULL != location && location->GetSelectedIndex() >= 0 && location->GetSelectedIndex() < (Int32) m_locationIds.size() )
        m_locationId = m_locationIds[ location->GetSelectedIndex() ];
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetImportFolder( const CORE::CString& providerId, const CORE::CString& itemId, const CORE::CString& title ) const
{GUCEF_TRACE;

    const CORE::CString name = ASSETSTORE::CAssetStoreSafeNames::MakeSafePart( title.IsNULLOrEmpty() ? itemId : title + " [" + itemId + "]" );
    return m_settings.GetEffectiveDownloadRoot() + "/" + ASSETSTORE::CAssetStoreSafeNames::MakeSafePart( providerId ) + "/" + name;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::QueueDownload( bool forPrimaryStore, CORE::CString& jobId )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_service || !m_haveItem || m_selectedProviderId.IsNULLOrEmpty() )
        return false;
    const CORE::CString formatId = GetSelectedFormatId();
    if ( formatId.IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: " << m_item.title << " has no format to download" );
        return false;
    }
    ASSETSTORE::CAssetDownloadRequest request;
    request.providerId = m_selectedProviderId;
    request.itemId = m_item.itemId;
    request.itemTitle = m_item.title;
    request.formatId = formatId;
    if ( forPrimaryStore )
    {
        if ( !IsCatalogueAvailable() )
            return false;
        // the service ingests it when it is done, with the keep policy and the location of the page
        request.forIngest = true;
        request.keep = ASSETSTORE::AssetKeepPolicyToString( m_keep );
        request.locationId = m_locationId;
    }
    else
    {
        request.destination = GetImportFolder( m_selectedProviderId, m_item.itemId, m_item.title );
    }
    ASSETSTORE::CAssetServiceResult result;
    const ASSETSTORE::TAssetStoreStatus status = m_service->EnqueueDownload( CORE::CString::Empty, request, result );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetSourcesPane: The download of " << m_item.title << " cannot be queued: " << ASSETSTORE::AssetStoreStatusToString( status ) );
        return false;
    }
    jobId = result.id;
    if ( forPrimaryStore )
        m_addJobs.insert( jobId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::ImportSelected( CORE::CString& jobId )
{GUCEF_TRACE;

    return QueueDownload( false, jobId );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::AddSelectedToPrimaryStore( CORE::CString& jobId )
{GUCEF_TRACE;

    return QueueDownload( true, jobId );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::SelectItems( const CORE::CString& storeId, const CORE::CString::StringVector& itemIds )
{GUCEF_TRACE;

    SOnlineStore* store = FindOnline( storeId );
    if ( GUCEF_NULL == store || itemIds.empty() )
        return false;
    // the folder of the first item; the items of that folder are selected together
    CORE::CString folder;
    CORE::CString::StringVector names;
    for ( size_t i=0; i<itemIds.size(); ++i )
    {
        const CORE::CString path = store->library->GetPathOfItem( itemIds[ i ] );
        if ( path.IsNULLOrEmpty() )
            continue;
        const Int32 slash = path.HasChar( '/', false );
        const CORE::CString itemFolder = slash > 0 ? path.SubstrToIndex( (UInt32) slash, true ) : CORE::CString( "/" );
        if ( names.empty() )
            folder = itemFolder;
        else
        if ( itemFolder != folder )
            continue;
        names.push_back( slash >= 0 ? path.CutChars( (UInt32) slash + 1, true ) : path );
    }
    if ( names.empty() )
        return false;
    store->browser->NavigateTo( store->browser->GetCurrentRootName(), folder );
    return store->browser->SelectNames( names );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CAssetSourcesPane::GetSelectedItemIds( void ) const
{GUCEF_TRACE;

    CORE::CString::StringVector itemIds;
    SOnlineStore* store = GetSelectedOnline();
    if ( GUCEF_NULL == store || GUCEF_NULL == store->browser || GUCEF_NULL == store->library )
        return itemIds;
    const CORE::CString::StringVector paths = store->browser->GetSelectedPaths();
    for ( size_t p=0; p<paths.size(); ++p )
    {
        const CORE::CString itemId = store->library->GetItemIdOfPath( paths[ p ] );
        if ( !itemId.IsNULLOrEmpty() )
            itemIds.push_back( itemId );
    }
    return itemIds;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::AddSelectedItemsToPrimaryStore( CORE::CString& jobId )
{GUCEF_TRACE;

    jobId.Clear();
    SOnlineStore* store = GetSelectedOnline();
    const CORE::CString::StringVector itemIds = GetSelectedItemIds();
    if ( GUCEF_NULL == m_service || GUCEF_NULL == store || itemIds.empty() || !IsCatalogueAvailable() )
        return false;
    ASSETSTORE::CAssetLibrarySyncSpec spec;
    spec.storeId = store->providerId;
    spec.mode = ASSETSTORE::CAssetLibrarySyncSpec::ModeRun;
    spec.useStoreProfile = true;
    spec.itemIds = itemIds;
    ASSETSTORE::CAssetPipelineRequest request;
    spec.ToRequest( request );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: Download of " << itemIds.size() << " selected item(s) of store " << store->providerId << " into the primary store" );
    if ( !m_service->IsInProcess() )
        return m_service->StartPipelineJobAsync( CORE::CString::Empty, request ).HasAFuture();
    ASSETSTORE::CAssetServiceResult result;
    const ASSETSTORE::TAssetStoreStatus status = m_service->StartPipelineJob( CORE::CString::Empty, request, result );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetSourcesPane: The download of the selected items cannot be started: " << ASSETSTORE::AssetStoreStatusToString( status ) << ": " << result.message );
        return false;
    }
    jobId = result.id;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnImportActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CString jobId;
    ImportSelected( jobId );
}

void
CAssetSourcesPane::OnAddActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CString jobId;
    if ( GetSelectedItemIds().size() > 1 )
        AddSelectedItemsToPrimaryStore( jobId );
    else
        AddSelectedToPrimaryStore( jobId );
}

void
CAssetSourcesPane::OnAddDiskActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CWidget* path = m_host->GetChildWidget( "Actions" )->GetChildWidget( "DiskPath" );
    if ( GUCEF_NULL != path )
        AddDiskPack( path->GetProperty( GUI::EWidgetProperty_Text ).Trim( true ).Trim( false ) );
}

void
CAssetSourcesPane::OnSignInActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SOnlineStore* store = GetSelectedOnline();
    if ( GUCEF_NULL == store )
        return;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: Sign in to " << store->providerId << " is asked for" );
    // the person asks: the actions get their retry again after this sign in
    store->signInAsked = false;
    CORE::TCloneableString data( store->providerId );
    NotifyObservers( SignInRequestedEvent, &data );
}

void
CAssetSourcesPane::OnSignInSettingsActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SOnlineStore* store = GetSelectedOnline();
    if ( GUCEF_NULL == store )
        return;
    CORE::TCloneableString data( store->providerId );
    NotifyObservers( SignInSettingsRequestedEvent, &data );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::AddDiskPack( const CORE::CString& pathText )
{GUCEF_TRACE;

    if ( IsUsingRemoteService() )
    {
        m_primaryMessage = "A pack from this computer cannot be added to the remote store yet: the upload is not built. Copy the pack to the server and ingest it there.";
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: " << m_primaryMessage );
        return false;
    }
    if ( !IsCatalogueAvailable() || pathText.IsNULLOrEmpty() )
        return false;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CString contentPath = CAssetMounter::NormalizePath( pathText );
    if ( !vfs.DirExists( contentPath ) && !vfs.FileExists( contentPath ) )
    {
        // a path of the disk: it is mounted to be read through the VFS
        const CORE::CString native = CORE::RelativePath( pathText );
        if ( !CORE::DirExists( native ) && !CORE::FileExists( native ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetSourcesPane: " << pathText << " is no folder or archive of the VFS or of the disk" );
            return false;
        }
        CORE::CString leaf = native;
        while ( leaf.Length() > 0 && ( '/' == leaf[ leaf.Length()-1 ] || '\\' == leaf[ leaf.Length()-1 ] ) )
            leaf = leaf.CutChars( 1, false );
        const Int32 slash = leaf.HasChar( '/', false ) > leaf.HasChar( '\\', false ) ? leaf.HasChar( '/', false ) : leaf.HasChar( '\\', false );
        if ( slash >= 0 )
            leaf = leaf.CutChars( (UInt32) slash + 1, true );
        contentPath = "disk/" + CORE::UInt32ToString( ++m_diskMounts ) + "_" + ASSETSTORE::CAssetStoreSafeNames::MakeSafePart( leaf );
        if ( !m_mounter.Mount( contentPath, native ) )
            return false;
    }
    ASSETSTORE::CAssetIngestRequest request;
    request.sourceId = ASSETSTORE::CPrimaryAssetStoreSettings::DiskSourceId;
    request.contentPath = contentPath;
    request.keep = ASSETSTORE::AssetKeepPolicyToString( m_keep );
    request.locationId = m_locationId;
    m_service->IngestAsync( CORE::CString::Empty, request );
    ++m_startedIngests;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: Ingest of " << contentPath << " started" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::MountFinishedDownload( const ASSETSTORE::CAssetJobEntry& job )
{GUCEF_TRACE;

    // the folder is part of a location that is mounted; archives are mounted to be browsed
    const CORE::CString folder = CAssetMounter::NormalizePath( job.folder );
    const CORE::CString prefix = CORE::CString( job.folder ).ReplaceChar( '\\', '/' );
    for ( size_t i=0; i<job.files.size(); ++i )
    {
        const CORE::CString& file = job.files[ i ];
        if ( CAssetMounter::GetArchiveType( file ).IsNULLOrEmpty() )
            continue;
        // the name of the file in the download: its path below the folder of the job
        CORE::CString name = file;
        if ( 0 == file.HasSubstr( prefix ) && file.Length() > prefix.Length() )
            name = file.CutChars( prefix.Length() + ( '/' == file[ prefix.Length() ] ? 1 : 0 ), true );
        const CORE::CString mountPoint = folder + "/" + name + ".mount";
        if ( m_mounter.Mount( mountPoint, CAssetMounter::NormalizePath( file ) ) )
            ++m_mountedDownloads;
    }
    // the folder of the download is a mount of the workbench when it is not inside one
    CAssetMounter::SMount mount;
    CORE::CString relative;
    if ( !m_mounter.FindMount( job.folder, mount, relative ) && m_mounter.Adopt( job.folder ) )
        ++m_mountedDownloads;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnJobChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetJobEntry* data = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetJobEntry >( eventData );
    if ( GUCEF_NULL == data )
        return;
    const ASSETSTORE::CAssetJobEntry& job = *data;
    if ( "conversion" == job.kind )
    {
        // the pack that is shown changed (its state, its converted files): the inspector shows it again
        if ( job.IsEnded() && !m_selectedPackId.IsNULLOrEmpty() && job.packId == m_selectedPackId )
            ShowResult( m_selectedResultKey );
        return;
    }
    if ( "download" != job.kind )
        return;
    // a download for the primary store that this page queued: the service started its ingest
    if ( job.ingestRequested && m_addJobs.find( job.jobId ) != m_addJobs.end() )
    {
        m_addJobs.erase( job.jobId );
        ++m_startedIngests;
    }
    if ( ASSETSTORE::ASSETJOB_STATE_DONE != job.state || m_endedDownloads.find( job.jobId ) != m_endedDownloads.end() )
        return;
    m_endedDownloads.insert( job.jobId );
    if ( !job.forIngest && m_settings.mountOnCompletion )
        MountFinishedDownload( job );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnOpenFolderRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* id = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == id || GUCEF_NULL == m_service )
        return;
    ASSETSTORE::CAssetJobEntry job;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK == m_service->GetJob( id->GetData(), job ) && "download" == job.kind )
    {
        CORE::TCloneableString data( CAssetMounter::NormalizePath( job.folder ) );
        NotifyObservers( FolderOpenRequestedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::OpenResult( const CORE::CString& key )
{GUCEF_TRACE;

    if ( IsUsingRemoteService() )
        return OpenRemoteResult( key, false );
    ASSETSTORE::CAssetSearchHit hit;
    if ( GUCEF_NULL == m_searchModel || !m_searchModel->FindHit( key, hit ) )
        return false;
    if ( hit.assetId.IsNULLOrEmpty() )
    {
        // a pack: its folder is shown
        ASSETSTORE::CAssetPackRecord pack;
        if ( GUCEF_NULL != m_service && ASSETSTORE::ASSETSTORE_STATUS_OK == m_service->GetPack( hit.packId, pack ) )
        {
            const CORE::CString root = GetLocationRoot( pack.locationId );
            if ( !root.IsNULLOrEmpty() )
            {
                CORE::TCloneableString data( CAssetMounter::NormalizePath( ASSETSTORE::CPackStorageLayout( pack.rootPath ).GetBrowseFolder( root ) ) );
                NotifyObservers( FolderOpenRequestedEvent, &data );
                return true;
            }
        }
        return false;
    }
    return OpenAssetFile( key, false );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::OpenResultConverted( const CORE::CString& key )
{GUCEF_TRACE;

    if ( IsUsingRemoteService() )
        return OpenRemoteResult( key, true );
    return OpenAssetFile( key, true );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::OpenAssetFile( const CORE::CString& key, bool converted )
{GUCEF_TRACE;

    ASSETSTORE::CAssetSearchHit hit;
    if ( GUCEF_NULL == m_service || GUCEF_NULL == m_searchModel || !m_searchModel->FindHit( key, hit ) || hit.assetId.IsNULLOrEmpty() )
        return false;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    ASSETSTORE::CAssetFileSet files;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service->GetAssetFileSet( hit.assetId, files ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetSourcesPane: " << hit.name << " cannot be opened: the asset is not in the catalogue" );
        return false;
    }
    CORE::CString vfsPath, message;
    ASSETSTORE::TAssetStoreStatus status = ASSETSTORE::ASSETSTORE_STATUS_NOT_FOUND;
    if ( !converted )
    {
        status = files.mainStatus;
        vfsPath = files.mainVfsPath;
        message = files.mainMessage;
        if ( ASSETSTORE::ASSETSTORE_STATUS_UNAVAILABLE != status && !( ASSETSTORE::ASSETSTORE_STATUS_OK == status && vfs.FileExists( vfsPath ) ) )
        {
            // the original is gone (keep: converted): the converted file is the asset
            message.Clear();
            converted = true;
        }
    }
    if ( converted )
    {
        status = files.convertedStatus;
        vfsPath = files.convertedVfsPath;
        message = files.convertedMessage;
    }
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetSourcesPane: " << hit.name << " cannot be opened: " << message );
        return false;
    }
    CORE::TCloneableString data( CAssetMounter::NormalizePath( vfsPath ) );
    NotifyObservers( AssetOpenRequestedEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnResultOpened( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* key = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != key )
        OpenResult( key->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnResultSelected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* key = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == key || GUCEF_NULL == m_inspector || GUCEF_NULL == m_service )
        return;
    ShowResult( key->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowResult( const CORE::CString& key )
{GUCEF_TRACE;

    m_selectedResultKey = key;
    m_selectedPackId.Clear();
    m_detailWaiting = false;
    ASSETSTORE::CAssetSearchHit hit;
    if ( key.IsNULLOrEmpty() || GUCEF_NULL == m_searchModel || !m_searchModel->FindHit( key, hit ) )
    {
        ClearInspector();
        ShowButtons();
        return;
    }
    m_selectedPackId = hit.packId;
    ShowButtons();
    if ( GUCEF_NULL == m_inspector || GUCEF_NULL == m_service )
        return;
    if ( !m_service->IsInProcess() )
    {
        // a service on the network: the answers come later, the interface does not wait for them (CheckDetails())
        m_detailPack = m_service->GetPackAsync( hit.packId );
        m_detailHasAsset = !hit.assetId.IsNULLOrEmpty();
        m_detailAsset = m_detailHasAsset ? m_service->GetAssetAsync( hit.assetId ) : CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        m_detailWaiting = true;
        return;
    }
    ASSETSTORE::CAssetPackRecord pack;
    const bool havePack = ASSETSTORE::ASSETSTORE_STATUS_OK == m_service->GetPack( hit.packId, pack );
    ASSETSTORE::CAssetRecord asset;
    const bool haveAsset = !hit.assetId.IsNULLOrEmpty() && ASSETSTORE::ASSETSTORE_STATUS_OK == m_service->GetAsset( hit.assetId, asset );
    ShowRecords( havePack, pack, !hit.assetId.IsNULLOrEmpty(), haveAsset, asset );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::ShowRecords( bool havePack, const ASSETSTORE::CAssetPackRecord& pack, bool hasAssetId, bool haveAsset, const ASSETSTORE::CAssetRecord& asset )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_inspector )
        return;
    CORE::CDataNode data;
    if ( hasAssetId )
    {
        if ( haveAsset )
            CAssetStoreInspectorData::FillAsset( asset, data );
        if ( data.GetName().IsNULLOrEmpty() )
            data.SetName( "Asset" );
        if ( havePack )
        {
            CORE::CDataNode packNode;
            CAssetStoreInspectorData::FillPack( pack, packNode );
            data.AddChild( "Pack" )->Copy( packNode );
            data.FindChild( "Pack" )->SetName( "Pack" );
        }
    }
    else
    if ( havePack )
        CAssetStoreInspectorData::FillPack( pack, data );
    m_inspector->ClearDescriptors();
    m_inspector->SetDataCopy( data );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::CheckDetails( void )
{GUCEF_TRACE;

    if ( !m_detailWaiting || !m_detailPack.IsReady() || ( m_detailHasAsset && !m_detailAsset.IsReady() ) )
        return;
    m_detailWaiting = false;
    CORE::CTaskPtr packTask = m_detailPack.GetResult( 0 );
    CORE::CTaskPtr assetTask = m_detailHasAsset ? m_detailAsset.GetResult( 0 ) : CORE::CTaskPtr();
    m_detailPack = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    m_detailAsset = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    const ASSETSTORE::CAssetPackRecord* pack = packTask.IsNULL() || CORE::TASKSTATUS_FINISHED != packTask->GetTaskStatus() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetPackRecord >( packTask->GetTaskData() );
    const ASSETSTORE::CAssetRecord* asset = assetTask.IsNULL() || CORE::TASKSTATUS_FINISHED != assetTask->GetTaskStatus() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetRecord >( assetTask->GetTaskData() );
    ShowRecords( GUCEF_NULL != pack, GUCEF_NULL != pack ? *pack : ASSETSTORE::CAssetPackRecord(), m_detailHasAsset, GUCEF_NULL != asset, GUCEF_NULL != asset ? *asset : ASSETSTORE::CAssetRecord() );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    if ( GUCEF_NULL != m_searchModel )
        m_searchModel->Update();
    if ( GUCEF_NULL != m_transferModel )
        m_transferModel->Update();
    UpdateRemote();
    CheckDetails();
    // a future that ended in failure has no future either: the outstanding request is what m_itemFutureId says
    if ( !m_itemFutureId.IsNULLOrEmpty() && m_itemFuture.IsReady() )
    {
        CORE::CTaskPtr task = m_itemFuture.GetResult( 0 );
        const ASSETSTORE::CAssetStoreItem* item = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetStoreItem >( task->GetTaskData() );
        // only the item that is still selected is shown
        if ( GUCEF_NULL != item && CORE::TASKSTATUS_FINISHED == task->GetTaskStatus() && item->itemId == m_selectedItemId )
        {
            ShowItem( *item );
            SOnlineStore* shown = FindOnline( m_selectedProviderId );
            if ( GUCEF_NULL != shown )
                shown->signInAsked = false;
        }
        else
        if ( !task.IsNULL() && CORE::TASKSTATUS_FINISHED != task->GetTaskStatus() && m_itemFutureId == m_selectedItemId )
        {
            // a failure is "statusName: message"; the end of the session is signed in again once and the details are asked again then
            const CORE::CString info = task->GetTaskStatusExtraInfo();
            const CORE::CString statusName = info.SubstrToChar( ':', true );
            SOnlineStore* failed = FindOnline( m_selectedProviderId );
            if ( GUCEF_NULL != failed )
            {
                const ASSETSTORE::TAssetStoreStatus status = ASSETSTORE::AssetStoreStatusFromString( statusName );
                if ( ASSETSTORE::ASSETSTORE_STATUS_SESSION_EXPIRED == status )
                {
                    failed->sessionEnded = true;
                    if ( AskSignInAgain( failed, info ) )
                        failed->retryItemId = m_itemFutureId;
                    else
                        failed->message = "The session ended and signing in again did not help";
                    FillStoreList();
                    ShowButtons();
                }
            }
        }
        m_itemFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        m_itemFutureId.Clear();
    }
    ShowStatus();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::IsUsingRemoteService( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_service && !m_service->IsInProcess();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSourcesPane::GetServiceLabel( void ) const
{GUCEF_TRACE;

    return m_serviceLabel;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::UseService( const ASSETSTORE::TAssetStoreServicePtr& service, const CORE::CString& label, const ASSETSTORE::CAssetServiceInfo* info )
{GUCEF_TRACE;

    return SwitchService( service, label, info );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::UseLocalService( void )
{GUCEF_TRACE;

    if ( m_localPtr.IsNULL() )
        return false;
    return SwitchService( m_localPtr, CORE::CString::Empty, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::SubscribeToService( void )
{GUCEF_TRACE;

    TEventCallback jobCallback( this, &CAssetSourcesPane::OnJobChanged );
    SubscribeTo( m_service, ASSETSTORE::CIAssetStoreService::JobChangedEvent, jobCallback );
    TEventCallback storeCallback( this, &CAssetSourcesPane::OnStoreStateChanged );
    SubscribeTo( m_service, ASSETSTORE::CIAssetStoreService::StoreStateChangedEvent, storeCallback );
    TEventCallback storesCallback( this, &CAssetSourcesPane::OnStoresChanged );
    SubscribeTo( m_service, ASSETSTORE::CIAssetStoreService::StoresChangedEvent, storesCallback );
    TEventCallback stateCallback( this, &CAssetSourcesPane::OnServiceConnectionStateChanged );
    SubscribeTo( m_service, ASSETSTORE::CIAssetStoreService::ConnectionStateChangedEvent, stateCallback );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnServiceConnectionStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ShowOnlineHint();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::SwitchService( const ASSETSTORE::TAssetStoreServicePtr& service, const CORE::CString& label, const ASSETSTORE::CAssetServiceInfo* info )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreServicePtr held( service );
    ASSETSTORE::CIAssetStoreService* next = held.GetPointerAlways();
    if ( GUCEF_NULL == next )
        return false;
    if ( next == m_service )
    {
        m_serviceLabel = label;
        ShowOnlineHint();
        return true;
    }

    // what works on the service that goes lets go of it: the models of the search and of the transfers, the browsers of the online stores
    if ( GUCEF_NULL != m_searchForm )
        m_searchForm->SetModel( GUCEF_NULL );
    if ( GUCEF_NULL != m_transfers )
        m_transfers->SetModel( GUCEF_NULL );
    for ( size_t i=0; i<m_online.size(); ++i )
    {
        GUCEF_DELETE m_online[ i ]->browser;
        GUCEF_DELETE m_online[ i ]->thumbnails;
        m_online[ i ]->access.Unlink();
        GUCEF_DELETE m_online[ i ];
    }
    m_online.clear();
    GUCEF_DELETE m_transferModel;
    m_transferModel = GUCEF_NULL;
    GUCEF_DELETE m_searchModel;
    m_searchModel = GUCEF_NULL;
    StopRemoteFiles();
    if ( GUCEF_NULL != m_service )
        UnsubscribeFrom( m_service );

    m_servicePtr = held;
    m_service = next;
    m_serviceLabel = label;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: the asset store is now " << ( label.IsNULLOrEmpty() ? CORE::CString( "the store of this computer" ) : label ) );
    SubscribeToService();
    m_transferModel = GUCEF_NEW CAssetStoreTransferModel( *m_service );
    m_searchModel = GUCEF_NEW CPrimaryStoreSearchModel( *m_service );
    m_searchModel->SetRemoteThumbnails( m_remoteThumbs );

    // the files of a service that is not in this process are mounted here, the ones of the service that went are not
    if ( m_service->IsInProcess() )
        StopRemoteFiles();
    else
        StartRemoteFiles();

    // what the page knew of the other service does not apply
    m_storeInfos.clear();
    m_storeInfosGiven = false;
    if ( !m_service->IsInProcess() )
    {
        if ( GUCEF_NULL != info )
        {
            m_storeInfos = info->stores;
            m_storeInfosGiven = true;
        }
        m_hasConverter = GUCEF_NULL != info && !info->converterName.IsNULLOrEmpty();
    }
    m_primaryMessage = m_service->IsCatalogueAvailable() ? CORE::CString::Empty : m_service->GetStatusMessage();
    m_selectedPackId.Clear();
    m_selectedResultKey.Clear();
    m_selectedStore = PrimaryStoreId;
    m_selectedItemId.Clear();
    m_selectedProviderId.Clear();
    m_haveItem = false;
    m_itemFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    m_itemFutureId.Clear();
    m_detailWaiting = false;
    m_addJobs.clear();
    m_endedDownloads.clear();
    if ( m_service->IsInProcess() )
        AdoptLocations();

    if ( m_initialized )
    {
        if ( GUCEF_NULL != m_transfers )
            m_transfers->SetModel( m_transferModel );
        if ( GUCEF_NULL != m_searchForm )
        {
            m_searchForm->SetModel( m_searchModel );
            m_searchForm->SetText( CORE::CString::Empty );
        }
        ClearInspector();
        ShowKeepAndLocations();
        RefreshStores();
        ShowButtons();
    }
    NotifyObservers( ServiceChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE FILES OF A REMOTE STORE                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetSourcesPane::SRemoteOpen::SRemoteOpen( void )
    : active( false ), key(), assetId(), name(), converted( false ), future( CORE::TASKSTATUS_UNDEFINED ), jobId( 0 ), openPath(), folderPath(), retries( 0 ), pack( false ), finishedTicks( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::SetFeedback( GUI::CActivityService* activity, CWorkbenchJobRegistry* registry )
{GUCEF_TRACE;

    m_activity = activity;
    m_registry = registry;
    if ( GUCEF_NULL != m_fetchJobs )
    {
        UnsubscribeFrom( m_fetchJobs );
        GUCEF_DELETE m_fetchJobs;
    }
    m_fetchJobs = GUCEF_NEW CRemoteFetchJobs( m_activity, m_registry );
    TEventCallback callback( this, &CAssetSourcesPane::OnFetchJobEnded );
    SubscribeTo( m_fetchJobs, CRemoteFetchJobs::JobEndedEvent, callback );
    m_fetchJobs->SetFileSystem( m_remoteFiles );
}

/*-------------------------------------------------------------------------*/

void CAssetSourcesPane::SetRemoteCacheFolder( const CORE::CString& folder ) { m_remoteCacheFolder = folder; }
void CAssetSourcesPane::SetRemoteMountPrefix( const CORE::CString& prefix ) { m_remotePrefix = prefix; }
ASSETSTORE::CRemoteAssetFileSystem* CAssetSourcesPane::GetRemoteFiles( void ) const { return m_remoteFiles; }
CRemoteFetchJobs* CAssetSourcesPane::GetFetchJobs( void ) const { return m_fetchJobs; }
CRemoteThumbnailSource* CAssetSourcesPane::GetRemoteThumbnails( void ) const { return m_remoteThumbs; }
CORE::CString CAssetSourcesPane::GetRemoteOpenText( void ) const { return m_remoteOpenText; }
bool CAssetSourcesPane::IsRemoteOpenBusy( void ) const { return m_open.active; }
UInt32 CAssetSourcesPane::GetRemoteOpenCount( void ) const { return m_remoteOpens; }

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetRemoteCacheBase( void ) const
{GUCEF_TRACE;

    if ( !m_remoteCacheFolder.IsNULLOrEmpty() )
        return m_remoteCacheFolder;
    if ( !m_settings.remoteFiles.cacheFolder.IsNULLOrEmpty() )
        return m_settings.remoteFiles.cacheFolder;
    return CORE::RelativePath( "$CACHEDIR$/AssetWorkbench/remote" );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::StartRemoteFiles( void )
{GUCEF_TRACE;

    StopRemoteFiles();
    if ( GUCEF_NULL == m_service || m_service->IsInProcess() )
        return;
    ASSETSTORE::CRemoteFilesSettings files( m_settings.remoteFiles );
    const CORE::CString base = GetRemoteCacheBase();
    files.cacheFolder = CORE::CombinePath( base, "blocks" );
    m_remoteFiles = GUCEF_NEW ASSETSTORE::CRemoteAssetFileSystem( *m_service, files );
    if ( !m_remoteFiles->Open() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetSourcesPane: The files of the remote store cannot be opened" );
        GUCEF_DELETE m_remoteFiles;
        m_remoteFiles = GUCEF_NULL;
        return;
    }
    // the thread of the interface never waits for the service: what is not in the cache is fetched in the background
    m_remoteFiles->SetNoWaitThread( MT::GetCurrentThreadID() );
    TEventCallback arrivedCallback( this, &CAssetSourcesPane::OnRemoteFilesArrived );
    SubscribeTo( m_remoteFiles, ASSETSTORE::CRemoteAssetFileSystem::FileArrivedEvent, arrivedCallback );
    SubscribeTo( m_remoteFiles, ASSETSTORE::CRemoteAssetFileSystem::ListingArrivedEvent, arrivedCallback );
    const UInt32 mounted = m_remoteFiles->MountLocations( m_remotePrefix );
    m_remoteRootOfLocation.clear();
    {
        ASSETSTORE::CAssetLocationPage locations;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK == m_service->GetLocations( locations ) )
            for ( size_t i=0; i<locations.entries.size(); ++i )
                m_remoteRootOfLocation[ locations.entries[ i ].location.locationId ] = locations.entries[ i ].location.vfsRoot;
    }
    CORE::CString::StringVector roots;
    m_remoteFiles->GetMounts( roots );
    for ( size_t i=0; i<roots.size(); ++i )
    {
        const CORE::CString clientRoot = m_remoteFiles->ToClientPath( roots[ i ] );
        m_mounter.AdoptRemote( clientRoot );
        m_remoteRoots.push_back( clientRoot );
    }
    {
        // only the owner devices put pictures on the service: the others do not try, and so do not log a refusal for each render
        ASSETSTORE::CAssetServiceInfo info;
        m_remoteThumbs->SetMayUpload( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service->GetServiceInfo( info ) || info.roles.count( "owner" ) > 0 );
    }
    m_remoteThumbs->SetService( m_service, m_remoteFiles );
    m_remoteThumbs->SetCacheFolder( CORE::CombinePath( base, "thumbnails" ) );
    if ( GUCEF_NULL != m_fetchJobs )
        m_fetchJobs->SetFileSystem( m_remoteFiles );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: The files of the remote store are mounted (" << mounted << " locations), the block cache is in " << files.cacheFolder );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::StopRemoteFiles( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_fetchJobs )
        m_fetchJobs->SetFileSystem( GUCEF_NULL );
    m_open = SRemoteOpen();
    m_remoteOpenText.Clear();
    if ( GUCEF_NULL != m_remoteThumbs )
        m_remoteThumbs->SetService( GUCEF_NULL, GUCEF_NULL );
    for ( size_t i=0; i<m_remoteRoots.size(); ++i )
        m_mounter.Forget( m_remoteRoots[ i ] );
    m_remoteRoots.clear();
    m_remoteRootOfLocation.clear();
    m_keptPacks.clear();
    if ( GUCEF_NULL != m_remoteFiles )
    {
        UnsubscribeFrom( m_remoteFiles );
        m_remoteFiles->Close();
        GUCEF_DELETE m_remoteFiles;
        m_remoteFiles = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnRemoteFilesArrived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a list that a folder of the content browser or a pack that is opened asked for is in: the views read it again (not at every list: a few frames later)
    if ( m_open.active && m_open.pack && ASSETSTORE::CRemoteAssetFileSystem::ListingArrivedEvent == eventId )
    {
        const CORE::TCloneableString* folder = static_cast< const CORE::TCloneableString* >( eventData );
        if ( GUCEF_NULL != folder && GUCEF_NULL != m_remoteFiles && m_remoteFiles->ToClientPath( folder->GetData() ) == m_open.folderPath )
        {
            const CORE::CString path = m_open.folderPath;
            m_open = SRemoteOpen();
            m_remoteOpenText.Clear();
            ++m_remoteOpens;
            CORE::TCloneableString data( CAssetMounter::NormalizePath( path ) );
            NotifyObservers( FolderOpenRequestedEvent, &data );
            return;
        }
    }
    m_remoteChangedDelay = 3;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::GetResultTarget( const CORE::CString& key, CORE::CString& packId, CORE::CString& assetId ) const
{GUCEF_TRACE;

    ASSETSTORE::CAssetSearchHit hit;
    if ( GUCEF_NULL == m_searchModel || !m_searchModel->FindHit( key, hit ) )
        return false;
    packId = hit.packId;
    assetId = hit.assetId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::OpenRemoteResult( const CORE::CString& key, bool converted )
{GUCEF_TRACE;

    ASSETSTORE::CAssetSearchHit hit;
    if ( GUCEF_NULL == m_remoteFiles || GUCEF_NULL == m_service || GUCEF_NULL == m_searchModel || !m_searchModel->FindHit( key, hit ) )
        return false;
    if ( m_open.active )
        FinishRemoteOpen( "Another result was opened" );
    m_open = SRemoteOpen();
    m_open.active = true;
    m_open.key = key;
    m_open.name = hit.name;
    m_open.converted = converted;
    if ( hit.assetId.IsNULLOrEmpty() )
    {
        // a pack: its folder is shown when the service has given its list
        m_open.pack = true;
        m_open.future = m_service->GetPackAsync( hit.packId );
        m_remoteOpenText = "Fetching the folder of " + hit.name + "...";
    }
    else
    {
        m_open.assetId = hit.assetId;
        m_open.future = m_service->GetAssetFileSetAsync( hit.assetId );
        m_remoteOpenText = "Fetching " + hit.name + "...";
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: " << m_remoteOpenText );
    ShowStatus();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::FinishRemoteOpen( const CORE::CString& problem )
{GUCEF_TRACE;

    if ( !problem.IsNULLOrEmpty() )
    {
        m_remoteOpenText = problem;
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: " << problem );
    }
    else
        m_remoteOpenText.Clear();
    if ( 0 != m_open.jobId && GUCEF_NULL != m_fetchJobs )
        m_fetchJobs->Cancel( m_open.jobId );
    m_open = SRemoteOpen();
    ShowStatus();
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::UpdateRemote( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_remoteFiles )
        return;
    m_remoteFiles->Update();
    if ( GUCEF_NULL != m_fetchJobs )
        m_fetchJobs->Update();
    m_remoteThumbs->Update();
    if ( m_remoteChangedDelay > 0 && 0 == --m_remoteChangedDelay )
        NotifyObservers( RemoteContentChangedEvent );

    // a file that the views asked for in the cache and did not find: fetched, then the views open the asset again (twice at most)
    CORE::CString::StringSet missed;
    m_remoteFiles->TakeMissedPaths( missed );
    if ( !missed.empty() && !m_open.active && !m_open.openPath.IsNULLOrEmpty() && m_open.retries < 2 && 0 != m_open.finishedTicks &&
         ( ( MT::PrecisionTickCount() - m_open.finishedTicks ) * 1000 ) / MT::PrecisionTimerResolution() < 10000 && GUCEF_NULL != m_fetchJobs )
    {
        ASSETSTORE::TRemoteFileTransferPtr transfer( GUCEF_NEW ASSETSTORE::CRemoteFileTransfer( ASSETSTORE::CRemoteFileTransfer::MODE_PREFETCH, m_open.name ) );
        UInt32 count = 0;
        for ( CORE::CString::StringSet::const_iterator i = missed.begin(); i != missed.end(); ++i )
        {
            ASSETSTORE::CRemoteAssetFileSystem::SFileInfo info;
            // known by the list of its folder, or asked for in the background: the next frame
            if ( m_remoteFiles->GetFileInfo( *i, info ) && !info.isDirectory )
            {
                transfer->AddFile( *i, *i, info.size, info.digest );
                ++count;
            }
        }
        if ( count > 0 )
        {
            const UInt32 retries = m_open.retries + 1;
            const CORE::CString path = m_open.openPath;
            const CORE::CString name = m_open.name;
            m_open = SRemoteOpen();
            m_open.active = true;
            m_open.retries = retries;
            m_open.openPath = path;
            m_open.name = name;
            m_open.jobId = m_fetchJobs->Start( transfer, "open", "Fetching the rest of " + name, path );
            m_remoteOpenText = "Fetching the rest of " + name + "...";
            if ( 0 == m_open.jobId )
                FinishRemoteOpen( "The files cannot be fetched" );
        }
    }

    if ( !m_open.active || 0 != m_open.jobId || !m_open.future.HasAFuture() || !m_open.future.IsReady() )
        return;

    if ( m_open.pack )
    {
        ASSETSTORE::CAssetPackRecord pack;
        if ( !ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_open.future, pack ) || ASSETSTORE::ASSETSTORE_STATUS_OK != ASSETSTORE::CIAssetStoreService::GetStatusOfFuture( m_open.future ) )
        {
            FinishRemoteOpen( "The pack " + m_open.name + " cannot be read from the store service: " + ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( m_open.future ) );
            return;
        }
        const CORE::CString root = m_remoteRootOfLocation.find( pack.locationId ) != m_remoteRootOfLocation.end() ? m_remoteRootOfLocation[ pack.locationId ] : CORE::CString::Empty;
        if ( root.IsNULLOrEmpty() )
        {
            FinishRemoteOpen( "The storage location of the pack " + m_open.name + " is not offered by the store service" );
            return;
        }
        const CORE::CString folder = ASSETSTORE::CPackStorageLayout( pack.rootPath ).GetBrowseFolder( root );
        m_open.folderPath = m_remoteFiles->ToClientPath( folder );
        m_open.future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        if ( m_remoteFiles->IsFolderKnown( folder ) )
        {
            const CORE::CString path = m_open.folderPath;
            m_open = SRemoteOpen();
            m_remoteOpenText.Clear();
            ++m_remoteOpens;
            CORE::TCloneableString data( CAssetMounter::NormalizePath( path ) );
            NotifyObservers( FolderOpenRequestedEvent, &data );
            return;
        }
        // the list is asked for; OnRemoteFilesArrived() shows the folder when it is in
        m_remoteFiles->RequestFolder( folder );
        return;
    }

    // an asset: its file set is known, its files are fetched into the cache, then the views open it
    ASSETSTORE::CAssetFileSet files;
    if ( !ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_open.future, files ) || ASSETSTORE::ASSETSTORE_STATUS_OK != ASSETSTORE::CIAssetStoreService::GetStatusOfFuture( m_open.future ) )
    {
        FinishRemoteOpen( m_open.name + " cannot be opened: the asset is not in the catalogue of the store service" );
        return;
    }
    m_open.future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    BeginRemoteOpenFetch( files );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::BeginRemoteOpenFetch( const ASSETSTORE::CAssetFileSet& files )
{GUCEF_TRACE;

    CORE::CString servicePath = files.mainVfsPath;
    ASSETSTORE::TAssetStoreStatus status = files.mainStatus;
    CORE::CString message = files.mainMessage;
    bool converted = m_open.converted;
    if ( !converted && ASSETSTORE::ASSETSTORE_STATUS_OK != status && ASSETSTORE::ASSETSTORE_STATUS_UNAVAILABLE != status )
    {
        // the original is gone (keep: converted): the converted file is the asset
        converted = true;
        message.Clear();
    }
    if ( converted )
    {
        servicePath = files.convertedVfsPath;
        status = files.convertedStatus;
        message = files.convertedMessage;
    }
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status || servicePath.IsNULLOrEmpty() )
    {
        FinishRemoteOpen( m_open.name + " cannot be opened: " + ( message.IsNULLOrEmpty() ? CORE::CString( "its files are not there" ) : message ) );
        return;
    }
    m_remoteFiles->NoteFiles( files.files );
    const CORE::CString clientPath = CAssetMounter::NormalizePath( m_remoteFiles->ToClientPath( servicePath ) );
    m_open.openPath = clientPath;

    ASSETSTORE::TRemoteFileTransferPtr transfer( GUCEF_NEW ASSETSTORE::CRemoteFileTransfer( ASSETSTORE::CRemoteFileTransfer::MODE_PREFETCH, m_open.name ) );
    UInt64 total = 0;
    GUCEF::set< CORE::CString > folders;
    CORE::CString packFolder;
    for ( size_t f=0; f<files.files.size(); ++f )
    {
        const ASSETSTORE::CAssetFileRecord& file = files.files[ f ];
        transfer->AddFile( file.path, file.path, file.sizeBytes > 0 ? (UInt64) file.sizeBytes : 0, file.digest );
        total += file.sizeBytes > 0 ? (UInt64) file.sizeBytes : 0;
        const CORE::CString client = CAssetMounter::NormalizePath( m_remoteFiles->ToClientPath( file.path ) );
        const Int32 slash = client.HasChar( '/', false );
        if ( slash > 0 )
            folders.insert( client.SubstrFromRange( 0, (UInt32) slash ) );
        if ( packFolder.IsNULLOrEmpty() )
            packFolder = GetPackFolderOfClientPath( client );
    }
    if ( total > m_remoteFiles->GetSettings().maxPrefetchBytes )
        transfer->SetFetchLimit( m_remoteFiles->GetSettings().openWithoutPrefetchBytes );
    if ( ASSETKIND_MODEL == CAssetTypeResolver::GetKind( clientPath ) && !converted )
    {
        transfer->SetDiscovery( GUCEF_NEW CRemoteCookedDiscovery( clientPath, CORE::CString::Empty ), true );
        // the companion files come from the relations of the asset (they are in the file set); the search by name is the fallback for an
        // asset without relations
        if ( !packFolder.IsNULLOrEmpty() && 0 == files.relationCount )
            transfer->AddDiscovery( GUCEF_NEW CRemoteReferencedDiscovery( clientPath, packFolder ), true );
    }

    // the lists of the areas of the pack: what the Convert for Ogre of the shell and the cooked output of the views ask of the VFS without a wait
    if ( !packFolder.IsNULLOrEmpty() )
        AddPackListings( transfer, packFolder );

    // the resource group of the pack is made when the files are in (Ogre reads the lists of the folders when a group is made): the folders are kept for then
    m_open.packFolder = packFolder;
    for ( GUCEF::set< CORE::CString >::const_iterator i = folders.begin(); i != folders.end(); ++i )
        m_open.folders.push_back( *i );

    const CORE::CString label = "Fetching " + m_open.name;
    m_remoteOpenText = label + "...";
    m_open.jobId = GUCEF_NULL != m_fetchJobs ? m_fetchJobs->Start( transfer, "open", label, clientPath, total > 0 ? 1500 : 0 ) : 0;
    if ( 0 == m_open.jobId )
        FinishRemoteOpen( m_open.name + " cannot be fetched: the transfer did not start" );
    ShowStatus();
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::AddPackListings( ASSETSTORE::TRemoteFileTransferPtr& transfer, const CORE::CString& clientPackFolder ) const
{GUCEF_TRACE;

    // "<location>/original/<pack>": the three areas of the location and the folders of the pack in the areas that the converter writes
    const CORE::CString original = ASSETSTORE::CPackStorageLayout::OriginalArea;
    const Int32 at = clientPackFolder.HasSubstr( "/" + original + "/", false );
    if ( at < 0 || GUCEF_NULL == m_remoteFiles )
        return;
    const CORE::CString root = clientPackFolder.SubstrFromRange( 0, (UInt32) at );
    const CORE::CString pack = clientPackFolder.CutChars( (UInt32) at + original.Length() + 2, true );
    const CORE::CString areas[ 3 ] = { ASSETSTORE::CPackStorageLayout::OriginalArea, ASSETSTORE::CPackStorageLayout::IntermediateArea, ASSETSTORE::CPackStorageLayout::CookedArea };
    CORE::CString servicePath;
    for ( UInt32 a=0; a<3; ++a )
    {
        if ( m_remoteFiles->ToServicePath( root + "/" + areas[ a ], servicePath ) )
            transfer->AddListing( servicePath );
        // the folders of the pack with what is below them: the resource groups of Ogre list what is in a place when they are made
        if ( m_remoteFiles->ToServicePath( root + "/" + areas[ a ] + "/" + pack, servicePath ) )
            transfer->AddListing( servicePath, true );
    }
    if ( m_remoteFiles->ToServicePath( root + "/" + ASSETSTORE::CPackStorageLayout::IntermediateArea + "/" + pack + "/.cache", servicePath ) )
        transfer->AddListing( servicePath );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::ReopenRemotePath( const CORE::CString& clientPath )
{GUCEF_TRACE;

    CORE::CString servicePath;
    if ( GUCEF_NULL == m_remoteFiles || GUCEF_NULL == m_fetchJobs || !m_remoteFiles->ToServicePath( clientPath, servicePath ) )
        return false;
    // what is known of the folders is old (a conversion wrote files): it is read again
    m_remoteFiles->ForgetMetadata();
    const CORE::CString path = CAssetMounter::NormalizePath( clientPath );
    const Int32 slash = path.HasChar( '/', false );
    const CORE::CString name = slash >= 0 ? path.CutChars( (UInt32) slash + 1, true ) : path;
    ASSETSTORE::TRemoteFileTransferPtr transfer( GUCEF_NEW ASSETSTORE::CRemoteFileTransfer( ASSETSTORE::CRemoteFileTransfer::MODE_PREFETCH, name ) );
    transfer->AddFile( servicePath, servicePath, 0, CORE::CString::Empty );
    const CORE::CString packFolder = GetPackFolderOfClientPath( path );
    if ( !packFolder.IsNULLOrEmpty() )
        AddPackListings( transfer, packFolder );
    if ( ASSETKIND_MODEL == CAssetTypeResolver::GetKind( path ) )
    {
        transfer->SetDiscovery( GUCEF_NEW CRemoteCookedDiscovery( path, CORE::CString::Empty ), true );
        if ( !packFolder.IsNULLOrEmpty() )
            transfer->AddDiscovery( GUCEF_NEW CRemoteReferencedDiscovery( path, packFolder ), true );
    }
    if ( m_open.active )
        FinishRemoteOpen( "Another result was opened" );
    m_open = SRemoteOpen();
    m_open.active = true;
    m_open.name = name;
    m_open.openPath = path;
    m_remoteOpenText = "Fetching " + name + "...";
    m_open.jobId = m_fetchJobs->Start( transfer, "open", "Fetching " + name, path, 0 );
    if ( 0 == m_open.jobId )
    {
        FinishRemoteOpen( name + " cannot be fetched: the transfer did not start" );
        return false;
    }
    ShowStatus();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesPane::GetPackFolderOfClientPath( const CORE::CString& clientPath ) const
{GUCEF_TRACE;

    // "<location>/<area>/<pack folder>/...": the folder of the pack in its area (layout version 2)
    for ( size_t r=0; r<m_remoteRoots.size(); ++r )
    {
        const CORE::CString root = CAssetMounter::NormalizePath( m_remoteRoots[ r ] );
        if ( clientPath.Length() <= root.Length() + 1 || 0 != clientPath.HasSubstr( root + "/", true ) )
            continue;
        const CORE::CString rest = clientPath.CutChars( root.Length() + 1, true );
        const Int32 first = rest.HasChar( '/', true );
        if ( first <= 0 || !ASSETSTORE::CPackStorageLayout::IsAreaName( rest.SubstrFromRange( 0, (UInt32) first ) ) )
            continue;
        const CORE::CString afterArea = rest.CutChars( (UInt32) first + 1, true );
        const Int32 second = afterArea.HasChar( '/', true );
        if ( second <= 0 )
            continue;
        return root + "/" + rest.SubstrFromRange( 0, (UInt32) first ) + "/" + afterArea.SubstrFromRange( 0, (UInt32) second );
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnFetchJobEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CRemoteFetchJobs::TEndedEventData* data = static_cast< const CRemoteFetchJobs::TEndedEventData* >( eventData );
    if ( GUCEF_NULL == data )
        return;
    const CRemoteFetchJobs::SEnded& ended = data->GetData();
    if ( "open" != ended.kind )
    {
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: " << ended.name << ": " << ended.message );
        return;
    }
    if ( !m_open.active || m_open.jobId != ended.id )
        return;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != ended.status )
    {
        FinishRemoteOpen( ended.cancelled ? "Fetching " + m_open.name + " was cancelled" : m_open.name + " cannot be opened: " + ended.message );
        return;
    }
    const CORE::CString path = m_open.openPath;
    const UInt32 retries = m_open.retries;
    const CORE::CString name = m_open.name;
    if ( !m_open.packFolder.IsNULLOrEmpty() )
        m_mounter.AdoptRemotePack( m_open.packFolder, m_open.folders );
    m_open = SRemoteOpen();
    m_open.openPath = path;
    m_open.retries = retries;
    m_open.name = name;
    m_open.finishedTicks = MT::PrecisionTickCount();
    m_remoteOpenText.Clear();
    ++m_remoteOpens;

    // the folders of the cooked files of a model get their own resource group (the lists of the folders are known to the cache by now: no wait)
    if ( ASSETKIND_MODEL == CAssetTypeResolver::GetKind( path ) )
    {
        CCookedOutputResolver::SOptions options;
        CCookedOutputResolver::SCookedOutput cooked;
        CCookedOutputResolver::LocateInStore( path, CORE::CString::Empty, options, cooked );
        if ( cooked.IsLocated() && !cooked.cookedFolder.IsNULLOrEmpty() )
        {
            GUCEF::set< CORE::CString > cookedFolders;
            for ( size_t i=0; i<cooked.files.size(); ++i )
            {
                const Int32 slash = cooked.files[ i ].HasChar( '/', false );
                if ( slash > 0 )
                    cookedFolders.insert( cooked.files[ i ].SubstrFromRange( 0, (UInt32) slash ) );
            }
            CORE::CString::StringVector folderList;
            for ( GUCEF::set< CORE::CString >::const_iterator i = cookedFolders.begin(); i != cookedFolders.end(); ++i )
                folderList.push_back( *i );
            m_mounter.AdoptRemotePack( cooked.cookedFolder, folderList );
        }
    }
    ShowStatus();
    CORE::TCloneableString open( path );
    NotifyObservers( AssetOpenRequestedEvent, &open );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::StartTransferJob( ASSETSTORE::TRemoteFileTransferPtr transfer, const CORE::CString& kind, const CORE::CString& label, CORE::CString& message )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_remoteFiles || GUCEF_NULL == m_fetchJobs )
    {
        message = "The files of the store service are not available on this computer";
        return false;
    }
    if ( 0 == m_fetchJobs->Start( transfer, kind, label, CORE::CString::Empty, 0 ) )
    {
        message = "The transfer did not start";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::DownloadPack( const CORE::CString& packId, const CORE::CString& folder, CORE::CString& message )
{GUCEF_TRACE;

    if ( packId.IsNULLOrEmpty() || folder.IsNULLOrEmpty() )
    {
        message = "Type the folder to download to, and select a result";
        return false;
    }
    if ( GUCEF_NULL == m_service )
    {
        message = "There is no store service";
        return false;
    }
    ASSETSTORE::TRemoteFileTransferPtr transfer( GUCEF_NEW ASSETSTORE::CRemoteFileTransfer( ASSETSTORE::CRemoteFileTransfer::MODE_DOWNLOAD, packId ) );
    transfer->SetDestination( CORE::RelativePath( folder ) );
    transfer->RequestPack( *m_service, packId );
    return StartTransferJob( transfer, "download", "Downloading " + packId + " to " + folder, message );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::DownloadAsset( const CORE::CString& assetId, const CORE::CString& folder, CORE::CString& message )
{GUCEF_TRACE;

    if ( assetId.IsNULLOrEmpty() || folder.IsNULLOrEmpty() )
    {
        message = "Type the folder to download to, and select a result";
        return false;
    }
    if ( GUCEF_NULL == m_service )
    {
        message = "There is no store service";
        return false;
    }
    ASSETSTORE::TRemoteFileTransferPtr transfer( GUCEF_NEW ASSETSTORE::CRemoteFileTransfer( ASSETSTORE::CRemoteFileTransfer::MODE_DOWNLOAD, assetId ) );
    transfer->SetDestination( CORE::RelativePath( folder ) );
    transfer->RequestAsset( *m_service, assetId );
    // the companion files are in the file set by the relations of the asset; for an asset without relations they are looked for by name
    transfer->AddFallbackDiscovery( GUCEF_NEW CRemoteReferencedDiscovery(), true );
    return StartTransferJob( transfer, "download", "Downloading " + assetId + " to " + folder, message );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::DownloadSelected( const CORE::CString& folder, CORE::CString& message )
{GUCEF_TRACE;

    CORE::CString packId;
    CORE::CString assetId;
    if ( m_selectedResultKey.IsNULLOrEmpty() || !GetResultTarget( m_selectedResultKey, packId, assetId ) )
    {
        message = "Select a result first";
        return false;
    }
    // an asset with its file set, a pack with everything in its folder
    return assetId.IsNULLOrEmpty() ? DownloadPack( packId, folder, message ) : DownloadAsset( assetId, folder, message );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::KeepPackOffline( const CORE::CString& packId, bool keep, CORE::CString& message )
{GUCEF_TRACE;

    if ( packId.IsNULLOrEmpty() || GUCEF_NULL == m_service )
    {
        message = "Select a result first";
        return false;
    }
    ASSETSTORE::TRemoteFileTransferPtr transfer( GUCEF_NEW ASSETSTORE::CRemoteFileTransfer( keep ? ASSETSTORE::CRemoteFileTransfer::MODE_KEEP_OFFLINE : ASSETSTORE::CRemoteFileTransfer::MODE_RELEASE_OFFLINE, packId ) );
    transfer->RequestPack( *m_service, packId );
    return StartTransferJob( transfer, "offline", ( keep ? "Keeping offline " : "Releasing " ) + packId, message );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::KeepSelectedOffline( bool keep, CORE::CString& message )
{GUCEF_TRACE;

    CORE::CString packId;
    CORE::CString assetId;
    if ( m_selectedResultKey.IsNULLOrEmpty() || !GetResultTarget( m_selectedResultKey, packId, assetId ) )
    {
        message = "Select a result first";
        return false;
    }
    // an asset with its companion files, a pack with everything in its folder
    return assetId.IsNULLOrEmpty() ? KeepPackOffline( packId, keep, message ) : KeepAssetOffline( assetId, keep, message );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::KeepAssetOffline( const CORE::CString& assetId, bool keep, CORE::CString& message )
{GUCEF_TRACE;

    if ( assetId.IsNULLOrEmpty() || GUCEF_NULL == m_service )
    {
        message = "Select a result first";
        return false;
    }
    ASSETSTORE::TRemoteFileTransferPtr transfer( GUCEF_NEW ASSETSTORE::CRemoteFileTransfer( keep ? ASSETSTORE::CRemoteFileTransfer::MODE_KEEP_OFFLINE : ASSETSTORE::CRemoteFileTransfer::MODE_RELEASE_OFFLINE, assetId ) );
    transfer->RequestAsset( *m_service, assetId );
    if ( keep )
        transfer->AddFallbackDiscovery( GUCEF_NEW CRemoteReferencedDiscovery(), true );
    return StartTransferJob( transfer, "offline", ( keep ? "Keeping offline " : "Releasing " ) + assetId, message );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnDownloadActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CWidget* path = m_host->GetChildWidget( "Actions" )->GetChildWidget( "DiskPath" );
    CORE::CString message;
    if ( !DownloadSelected( GUCEF_NULL != path ? path->GetProperty( GUI::EWidgetProperty_Text ).Trim( true ).Trim( false ) : CORE::CString::Empty, message ) )
    {
        m_remoteOpenText = message;
        ShowStatus();
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnKeepOfflineActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // pressed again for a pack that is kept: it is let go of (the files stay in the cache until the limit needs the room)
    CORE::CString message;
    CORE::CString packId;
    CORE::CString assetId;
    bool keep = true;
    if ( !m_selectedResultKey.IsNULLOrEmpty() && GetResultTarget( m_selectedResultKey, packId, assetId ) && m_keptPacks.find( packId ) != m_keptPacks.end() )
        keep = false;
    if ( KeepSelectedOffline( keep, message ) )
    {
        if ( keep )
            m_keptPacks.insert( packId );
        else
            m_keptPacks.erase( packId );
    }
    else
    {
        m_remoteOpenText = message;
        ShowStatus();
    }
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
CAssetSourcesPane::GetControl( const CORE::CString& name ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_host )
        return GUCEF_NULL;
    GUI::CWidget* widget = m_host->GetChildWidget( name );
    if ( GUCEF_NULL != widget )
        return widget;
    GUI::CWidget* actions = m_host->GetChildWidget( "Actions" );
    return GUCEF_NULL != actions ? actions->GetChildWidget( name ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::CIAssetStoreService* CAssetSourcesPane::GetService( void ) const { return m_service; }
CAssetStoreTransferModel* CAssetSourcesPane::GetTransferModel( void ) const { return m_transferModel; }
CPrimaryStoreSearchModel* CAssetSourcesPane::GetSearchModel( void ) const { return m_searchModel; }
GUI::CAssetSearchForm* CAssetSourcesPane::GetSearchForm( void ) const { return m_searchForm; }
const CAssetSourcesSettings& CAssetSourcesPane::GetSettings( void ) const { return m_settings; }
UInt32 CAssetSourcesPane::GetMountedDownloadCount( void ) const { return m_mountedDownloads; }
UInt32 CAssetSourcesPane::GetStartedIngestCount( void ) const { return m_startedIngests; }
UInt32 CAssetSourcesPane::GetStartedConversionCount( void ) const { return m_startedConversions; }
const CORE::CString& CAssetSourcesPane::GetSelectedPackId( void ) const { return m_selectedPackId; }
ASSETSTORE::CIAssetConverterHook* CAssetSourcesPane::GetConverterHook( void ) const { return m_hook; }

/*-------------------------------------------------------------------------*/

ASSETSTORE::CLocalAssetStoreService*
CAssetSourcesPane::GetLocalService( void ) const
{GUCEF_TRACE;

    // the implementation is known by its registered id: no run time type information is needed
    if ( GUCEF_NULL == m_service || ASSETSTORE::CAssetStoreServiceConfig::LocalImplementationId != m_service->GetType() )
        return GUCEF_NULL;
    return static_cast< ASSETSTORE::CLocalAssetStoreService* >( m_service );
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::SetConverterHook( ASSETSTORE::CIAssetConverterHook* hook )
{GUCEF_TRACE;

    // the hook of the settings is replaced: it goes with the page, the one that was given stays with its owner
    ASSETSTORE::CLocalAssetStoreService* local = GetLocalService();
    if ( GUCEF_NULL != local )
        local->SetConverterHook( hook );
    if ( hook != m_ownHook )
    {
        GUCEF_DELETE m_ownHook;
        m_ownHook = GUCEF_NULL;
    }
    m_hook = hook;
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::ConvertPack( const CORE::CString& packId )
{GUCEF_TRACE;

    if ( !IsCatalogueAvailable() || !HasConverter() || GUCEF_NULL == m_transferModel || packId.IsNULLOrEmpty() )
        return false;
    if ( !m_transferModel->StartConversion( packId ) )
    {
        m_primaryMessage = "The conversion could not be started";
        ShowStatus();
        return false;
    }
    ++m_startedConversions;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSourcesPane: Converting the pack " << packId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::ConvertSelectedPack( void )
{GUCEF_TRACE;

    return ConvertPack( m_selectedPackId );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesPane::ConvertPendingPacks( void )
{GUCEF_TRACE;

    if ( !IsCatalogueAvailable() || !HasConverter() || GUCEF_NULL == m_transferModel )
        return false;
    if ( !m_transferModel->StartPendingConversion() )
        return false;
    ++m_startedConversions;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesPane::OnConvertActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ConvertSelectedPack();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSourcesPane::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CAssetSourcesPane";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
