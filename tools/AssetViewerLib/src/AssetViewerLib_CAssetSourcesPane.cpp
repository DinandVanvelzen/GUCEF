/*
 *  AssetViewerLib: the asset sources of the viewer: online stores, the primary store and the transfers
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

#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#include "gucefASSETSTORE_CAssetStoreSafeNames.h"
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */


#ifndef GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#include "gucefASSETSTORE_CAssetIngestRequest.h"
#define GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H ? */

#ifndef ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#include "AssetViewerLib_CAssetTypeResolver.h"
#define ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#endif /* ASSETVIEWERLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETVIEWERLIB_CASSETSTOREINSPECTORDATA_H
#include "AssetViewerLib_CAssetStoreInspectorData.h"
#define ASSETVIEWERLIB_CASSETSTOREINSPECTORDATA_H
#endif /* ASSETVIEWERLIB_CASSETSTOREINSPECTORDATA_H ? */

#ifndef ASSETVIEWERLIB_CASSETSOURCESPANE_H
#include "AssetViewerLib_CAssetSourcesPane.h"
#define ASSETVIEWERLIB_CASSETSOURCESPANE_H
#endif /* ASSETVIEWERLIB_CASSETSOURCESPANE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAssetSourcesPane::AssetOpenRequestedEvent = "GUCEF::ASSETVIEWER::CAssetSourcesPane::AssetOpenRequestedEvent";
const CORE::CEvent CAssetSourcesPane::FolderOpenRequestedEvent = "GUCEF::ASSETVIEWER::CAssetSourcesPane::FolderOpenRequestedEvent";
const CORE::CEvent CAssetSourcesPane::SignInRequestedEvent = "GUCEF::ASSETVIEWER::CAssetSourcesPane::SignInRequestedEvent";
const CORE::CEvent CAssetSourcesPane::SignInSettingsRequestedEvent = "GUCEF::ASSETVIEWER::CAssetSourcesPane::SignInSettingsRequestedEvent";
const CORE::CEvent CAssetSourcesPane::SessionExpiredEvent = "GUCEF::ASSETVIEWER::CAssetSourcesPane::SessionExpiredEvent";
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
    , m_updatingWidgets( false )
    , m_initialized( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAssetSourcesPane::~CAssetSourcesPane()
{GUCEF_TRACE;

    Shutdown();
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
    MakeWidget( *m_context, "Combobox", "Location", actions, "0.5,2 0,32", "0.5,-2 0,28" );
    GUI::CWidget* import = MakeWidget( *m_context, "Button", "Import", actions, "0,0 0,64", "0.4,-2 0,28" );
    GUI::CWidget* add = MakeWidget( *m_context, "Button", "AddToPrimary", actions, "0.4,2 0,64", "0.6,-2 0,28" );
    MakeWidget( *m_context, "Editbox", "DiskPath", actions, "0,0 0,0", "1,0 0,28" );
    GUI::CWidget* addDisk = MakeWidget( *m_context, "Button", "AddDisk", actions, "0,0 0,64", "0.7,-2 0,28" );
    GUI::CWidget* convert = MakeWidget( *m_context, "Button", "Convert", actions, "0.7,2 0,64", "0.3,-2 0,28" );
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
    if ( GUCEF_NULL != m_service )
        m_service->Close();
    m_servicePtr.Unlink();
    m_service = GUCEF_NULL;
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
        actions->GetChildWidget( "DiskPath" )->SetBoolProperty( GUI::EWidgetProperty_Visible, primary );
        actions->GetChildWidget( "AddDisk" )->SetBoolProperty( GUI::EWidgetProperty_Visible, primary );
        actions->GetChildWidget( "Convert" )->SetBoolProperty( GUI::EWidgetProperty_Visible, primary );
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
    // the folder of the download is a mount of the viewer when it is not inside one
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
                CORE::TCloneableString data( CAssetMounter::NormalizePath( root + "/" + pack.rootPath ) );
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
    CORE::CDataNode data;
    ASSETSTORE::CAssetPackRecord pack;
    const bool havePack = ASSETSTORE::ASSETSTORE_STATUS_OK == m_service->GetPack( hit.packId, pack );
    if ( !hit.assetId.IsNULLOrEmpty() )
    {
        ASSETSTORE::CAssetRecord asset;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK == m_service->GetAsset( hit.assetId, asset ) )
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
CAssetSourcesPane::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    if ( GUCEF_NULL != m_searchModel )
        m_searchModel->Update();
    if ( GUCEF_NULL != m_transferModel )
        m_transferModel->Update();
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

    static const CORE::CString classTypeName = "GUCEF::ASSETVIEWER::CAssetSourcesPane";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
