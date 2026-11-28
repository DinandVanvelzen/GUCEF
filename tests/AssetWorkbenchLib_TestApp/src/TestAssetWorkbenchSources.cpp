/*
 *  AssetWorkbenchLib_TestApp: tests of the AssetWorkbenchLib module
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

#include <stdlib.h>

#include <Ogre.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#include "AssetWorkbenchLib.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"
#include "TestFakeSourceStore.h"
#include "TestFakeConverterHook.h"
#include "TestAssetWorkbenchSources.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Counts the events of a notifier and keeps the text of the last one */
class CTextRecorder : public CORE::CObserver
{
    public:

    UInt32 count;
    CORE::CString last;

    CTextRecorder( CORE::CNotifier* notifier, const CORE::CEvent& id ) : CORE::CObserver(), count( 0 ), last(), m_id( id )
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CTextRecorder() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id != m_id )
            return;
        ++count;
        const CORE::TCloneableString* text = static_cast< const CORE::TCloneableString* >( eventData );
        if ( GUCEF_NULL != text )
            last = text->GetData();
    }

    private:

    CORE::CEvent m_id;
};

/** The workbench with its asset sources: a primary store on the table database of the VFS in a folder of the test, and the fake online store */
class CSourcesFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CAssetMounter mounter;
    CWorkbenchShell* shell;
    SOURCESTEST::TFakeSourceFactory factory;
    SOURCESTEST::CFakeSourceProvider* fake;
    CORE::CString dir;
    CORE::CString root;
    CAssetSourcesSettings settings;
    bool ready;
    bool sourcesOpened;

    CSourcesFixture( const CORE::CString& name, bool signedIn = true )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), mounter(), shell( GUCEF_NULL ), factory(), fake( GUCEF_NULL ), dir(), root(), settings(), ready( false ), sourcesOpened( false )
    {
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        const CORE::CString storeDir = dir + "/store";
        CORE::CreateDirs( storeDir );
        root = "avstore/" + name;
        settings.primaryStore.locations[ 0 ].vfsRoot = root;
        settings.primaryStore.locations[ 0 ].fileSystemPath = storeDir;
        settings.primaryStore.database.database = root + "/catalogue";
        settings.primaryStore.analysisConcurrency = 2;

        ASSETSTORE::TAssetStoreProviderPtr provider = factory.Create();
        fake = static_cast< SOURCESTEST::CFakeSourceProvider* >( provider.GetPointerAlways() );
        fake->SetSignedIn( signedIn );
        FillFakeStore();
        ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().AddProvider( provider );

        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        ready = shell->Initialize( *context, GUCEF_NULL, MakeCleanTestDir( name + "_cache" ) );
        sourcesOpened = ready && shell->InitializeSources( settings );
    }

    ~CSourcesFixture()
    {
        if ( GUCEF_NULL != shell )
        {
            shell->Shutdown();
            GUCEF_DELETE shell;
        }
        ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( SOURCESTEST::CFakeSourceProvider::ProviderIdText );
        contextPtr.Unlink();
        mounter.UnmountAll();
    }

    void FillFakeStore( void )
    {
        typedef SOURCESTEST::CFakeSourceProvider TFake;
        GUCEF::vector< TFake::SFormatSpec > weapon;
        TFake::SFormatSpec fbx;
        fbx.formatId = "fbx";
        fbx.name = "FBX";
        fbx.group = "3d-exchange";
        TFake::SFileSpec file;
        file.name = "weapon.fbx";
        file.size = 40000;
        file.seed = 1;
        fbx.files.push_back( file );
        TFake::SFormatSpec unreal;
        unreal.formatId = "unreal-5-4";
        unreal.name = "Unreal Engine 5.4";
        unreal.group = "engine";
        file.name = "weapon.uasset";
        file.size = 20000;
        unreal.files.push_back( file );
        TFake::SFormatSpec gltf;
        gltf.formatId = "gltf";
        gltf.name = "glTF";
        gltf.group = "3d-exchange";
        file.name = "weapon.gltf";
        file.size = 81920;
        gltf.files.push_back( file );
        file.name = "weapon_d.png";
        file.size = 4096;
        gltf.files.push_back( file );
        TFake::SFormatSpec obj;
        obj.formatId = "obj";
        obj.name = "OBJ";
        obj.group = "3d-exchange";
        file.name = "weapon.obj";
        file.size = 30000;
        obj.files.push_back( file );
        weapon.push_back( fbx );
        weapon.push_back( unreal );
        weapon.push_back( gltf );
        weapon.push_back( obj );
        fake->AddItem( "weapon-1", "Iron Sword", "Forge Works", "3d-model", weapon, "Store EULA" );

        GUCEF::vector< TFake::SFormatSpec > small;
        TFake::SFormatSpec tiny;
        tiny.formatId = "gltf";
        tiny.name = "glTF";
        tiny.group = "3d-exchange";
        file.name = "barrel.gltf";
        file.size = 20000;
        tiny.files.push_back( file );
        small.push_back( tiny );
        fake->AddItem( "prop-1", "Wooden Barrel", "Forge Works", "3d-model", small );
        fake->AddItem( "mat-1", "Rock Surface", "Stone Age", "material", small );
    }

    /** A pack on the disk that the primary store can ingest */
    CORE::CString WritePack( const CORE::CString& name )
    {
        const CORE::CString pack = dir + "/packs/" + name;
        CORE::CreateDirs( pack + "/Textures" );
        WriteTestFile( pack + "/pack.json", "{ \"title\": \"" + name + "\", \"author\": \"Forge Works\", \"tags\": [ \"sword\", \"medieval\" ], "
                                            "\"licence\": { \"name\": \"CC BY 4.0\", \"restrictions\": [ \"noMachineLearning\" ] } }" );
        WriteTestFile( pack + "/sword_model.gltf", BuildTestGltf() );
        WriteTestPng( pack + "/Textures/sword_d.png", 16, 8 );
        return pack;
    }

    /** Frames of the window with the pulses of the application: the shell has its turn, the events of the stores arrive */
    IMAGE::TPixelMapPtr Frames( UInt32 count )
    {
        IMAGE::TPixelMapPtr pixels;
        for ( UInt32 i=0; i<count; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            shell->Update( 5 );
            window->RenderFrame( &pixels );
            MT::PrecisionDelay( 2 );
        }
        return pixels;
    }

    template < typename TCondition >
    bool WaitFor( TCondition condition, UInt32 maxFrames = 600 )
    {
        for ( UInt32 i=0; i<maxFrames; ++i )
        {
            if ( condition() )
                return true;
            Frames( 1 );
            MT::PrecisionDelay( 5 );
        }
        return condition();
    }

    CAssetSourcesPane& Pane( void ) { return *shell->GetSources(); }

    /** The entry of the transfer queue with the id, GUCEF_NULL when there is none */
    bool FindTransfer( const CORE::CString& idPart, GUI::CITransferQueueModel::TEntry& entry )
    {
        GUI::CITransferQueueModel::TEntryVector entries;
        Pane().GetTransferModel()->GetEntries( entries );
        for ( size_t i=0; i<entries.size(); ++i )
        {
            if ( entries[ i ].id.HasSubstr( idPart ) >= 0 || entries[ i ].name == idPart )
            {
                entry = entries[ i ];
                return true;
            }
        }
        return false;
    }

    UInt32 CountIngests( GUI::CITransferQueueModel::ETransferState state )
    {
        GUI::CITransferQueueModel::TEntryVector entries;
        Pane().GetTransferModel()->GetEntries( entries );
        UInt32 count = 0;
        for ( size_t i=0; i<entries.size(); ++i )
            if ( entries[ i ].id.HasSubstr( CAssetStoreTransferModel::IngestIdPrefix ) == 0 && entries[ i ].state == state )
                ++count;
        return count;
    }

    /** Ingests a pack from the disk and waits for the search to find it */
    bool IngestPack( const CORE::CString& name )
    {
        const UInt32 before = CountIngests( GUI::CITransferQueueModel::TRANSFERSTATE_DONE );
        if ( !Pane().AddDiskPack( WritePack( name ) ) )
            return false;
        return WaitFor( [&]() { return CountIngests( GUI::CITransferQueueModel::TRANSFERSTATE_DONE ) > before; } );
    }

    bool Search( const CORE::CString& text )
    {
        GUI::CAssetSearchForm* form = Pane().GetSearchForm();
        form->SetText( text );
        return WaitFor( [&]() { return !form->IsSearching(); } );
    }
};

CORE::CString
Attribute( const CORE::CDataNode& data, const CORE::CString& childName, const CORE::CString& attribute )
{
    const CORE::CDataNode* child = childName.IsNULLOrEmpty() ? &data : data.FindChild( childName );
    return GUCEF_NULL != child ? child->GetAttributeValue( attribute ).AsString( CORE::CString::Empty, true ) : CORE::CString::Empty;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestSourcesSettings( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: the settings keep the folder, the preference and the stores in the section AssetSources, rank the formats and name the folder of the downloads" )
    try
    {
        CAssetSourcesSettings settings;
        ASSERT_TRUE( settings.mountOnCompletion && 5 == settings.formatPreference.size() && "gltf" == settings.formatPreference[ 0 ] && "usd" == settings.formatPreference[ 4 ] );
        ASSERT_TRUE( "local-assetstore/local/downloads" == settings.GetEffectiveDownloadRoot() );
        settings.downloadRoot = "other/place";
        ASSERT_TRUE( "other/place" == settings.GetEffectiveDownloadRoot() );
        settings.downloadRoot.Clear();
        settings.mountOnCompletion = false;
        settings.formatPreference.clear();
        settings.formatPreference.push_back( "obj" );
        settings.formatPreference.push_back( "FBX" );
        settings.downloads.maxConcurrentJobs = 7;
        settings.primaryStore.defaultPolicy.keep = ASSETSTORE::ASSETKEEP_ORIGINAL;

        CORE::CDataNode config;
        ASSERT_TRUE( settings.Save( config ) );
        CAssetSourcesSettings loaded;
        ASSERT_TRUE( loaded.Load( config ) );
        ASSERT_TRUE( !loaded.mountOnCompletion && 2 == loaded.formatPreference.size() && "obj" == loaded.formatPreference[ 0 ] && "FBX" == loaded.formatPreference[ 1 ] );
        ASSERT_TRUE( 7 == loaded.downloads.maxConcurrentJobs && ASSETSTORE::ASSETKEEP_ORIGINAL == loaded.primaryStore.defaultPolicy.keep );
        CAssetSourcesSettings untouched;
        ASSERT_TRUE( untouched.Load( CORE::CDataNode() ) && 5 == untouched.formatPreference.size() );

        // the formats of an item: the preference first, the engine export last, what the list does not name in between
        ASSETSTORE::TAssetStoreFormatVector formats;
        const char* const names[][ 2 ] = { { "unreal-5-4", "Unreal Engine 5.4" }, { "blend", "Blender" }, { "fbx", "FBX" }, { "obj", "OBJ" }, { "gltf", "glTF" } };
        for ( UInt32 i=0; i<5; ++i )
        {
            ASSETSTORE::CAssetStoreFormat format;
            format.formatId = names[ i ][ 0 ];
            format.name = names[ i ][ 1 ];
            formats.push_back( format );
        }
        CAssetSourcesSettings standard;
        GUCEF::vector< UInt32 > order;
        standard.RankFormats( formats, order );
        ASSERT_TRUE( 5 == order.size() && 4 == order[ 0 ] && 2 == order[ 1 ] && 3 == order[ 2 ] && 1 == order[ 3 ] && 0 == order[ 4 ] );
        ASSERT_TRUE( 4 == standard.ChooseFormat( formats ) && -1 == standard.ChooseFormat( ASSETSTORE::TAssetStoreFormatVector() ) );
        ASSETSTORE::TAssetStoreFormatVector only;
        only.push_back( formats[ 0 ] );
        only.push_back( formats[ 1 ] );
        ASSERT_TRUE( 1 == standard.ChooseFormat( only ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesQueryMapping( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: the query of the search form is the query of the store and back, and what the form does not edit rides along" )
    try
    {
        GUI::CSearchQuery query;
        query.text = "sword";
        query.AddChoice( "kind", "static-model" );
        query.AddChoice( "format", "fbx" );
        query.AddChoice( "flags", "rigged" );
        query.AddChoice( "allows", "machineLearning" );
        GUI::CSearchQuery::TRange megabytes;
        megabytes.hasMaximum = true;
        megabytes.maximum = 2.0;
        query.SetRange( "megabytes", megabytes );
        GUI::CSearchQuery::TRange triangles;
        triangles.hasMinimum = true;
        triangles.minimum = 1000;
        query.SetRange( "triangles", triangles );
        query.sort = "size";
        query.pageIndex = 3;
        query.pageSize = 20;

        ASSETSTORE::CAssetSearchQuery store;
        CPrimaryStoreSearchModel::ToStoreQuery( query, store );
        ASSERT_TRUE( "sword" == store.text && store.kinds.end() != store.kinds.find( "static-model" ) && store.formats.end() != store.formats.find( "fbx" ) );
        ASSERT_TRUE( 1 == store.rigged && -1 == store.animated && -1 == store.hasLods );
        ASSERT_TRUE( store.excludeLicenceRestrictions.end() != store.excludeLicenceRestrictions.find( "noMachineLearning" ) );
        ASSERT_TRUE( 2 * 1048576 == store.maxAssetBytes && -1 == store.minAssetBytes && 1000 == store.minTriangles && -1 == store.maxTriangles );
        ASSERT_TRUE( "size" == store.sort && 3 == store.pageIndex && 20 == store.pageSize && store.withFacets );

        // a saved search of the store keeps what the form has no widget for
        store.themes.insert( "fantasy" );
        store.requireLicenceRestrictions.insert( "attributionRequired" );
        GUI::CSearchQuery generic;
        CPrimaryStoreSearchModel::FromStoreQuery( store, generic );
        ASSERT_TRUE( generic.HasChoice( "kind", "static-model" ) && generic.HasChoice( "flags", "rigged" ) && generic.HasChoice( "allows", "machineLearning" ) );
        ASSERT_TRUE( generic.GetRange( "megabytes" ).hasMaximum && 2.0 == generic.GetRange( "megabytes" ).maximum );
        ASSETSTORE::CAssetSearchQuery again;
        CPrimaryStoreSearchModel::ToStoreQuery( generic, again );
        ASSERT_TRUE( again.themes.end() != again.themes.find( "fantasy" ) && again.requireLicenceRestrictions.end() != again.requireLicenceRestrictions.find( "attributionRequired" ) );
        ASSERT_TRUE( again.kinds == store.kinds && again.maxAssetBytes == store.maxAssetBytes && "sword" == again.text );
        // the form's choices win over the extra: unchecking a kind leaves it out
        generic.RemoveChoice( "kind", "static-model" );
        ASSETSTORE::CAssetSearchQuery cleared;
        CPrimaryStoreSearchModel::ToStoreQuery( generic, cleared );
        ASSERT_TRUE( cleared.kinds.empty() && cleared.themes.end() != cleared.themes.find( "fantasy" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesTab( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: the left tab Asset sources lists the primary store first and then the online stores with their sign in state, and sign in is an extension point" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSourcesFixture fixture( "src_tab", false );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( GUCEF_NULL != shell.GetSources() && GUCEF_NULL != shell.GetTransfers() );
        shell.ShowLeftTab( 2 );
        ASSERT_TRUE( 2 == shell.GetLeftTab() );
        CAssetSourcesPane& pane = fixture.Pane();
        ASSERT_TRUE( 2 == pane.GetStoreCount() );
        ASSERT_TRUE( "primary" == pane.GetStoreId( 0 ) && "Primary store" == pane.GetStoreLine( 0 ) );
        ASSERT_TRUE( "fakesrc" == pane.GetStoreId( 1 ) && "Fake Store - sign in required" == pane.GetStoreLine( 1 ) );
        ASSERT_TRUE( "primary" == pane.GetSelectedStoreId() && !pane.GetControl( "SignIn" )->GetBoolProperty( "Visible" ) );
        ASSERT_TRUE( pane.GetControl( "AddDisk" )->GetBoolProperty( "Visible" ) && !pane.GetControl( "Import" )->GetBoolProperty( "Visible" ) );
        GUI::CListbox* list = GUI::CWidget::As< GUI::CListbox >( pane.GetControl( "Stores" ) );
        ASSERT_TRUE( 2 == list->GetItems().size() && "Fake Store - sign in required" == list->GetItems()[ 1 ] );

        // choosing the online store shows what it needs
        list->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "1" );
        fixture.Frames( 3 );
        ASSERT_TRUE( "fakesrc" == pane.GetSelectedStoreId() && "Sign in required" == pane.GetStoreStatusText() );
        ASSERT_TRUE( pane.GetControl( "SignIn" )->GetBoolProperty( "Visible" ) && pane.GetControl( "Import" )->GetBoolProperty( "Visible" ) && !pane.GetControl( "Import" )->GetBoolProperty( "Enabled" ) );
        ASSERT_TRUE( !pane.GetControl( "AddDisk" )->GetBoolProperty( "Visible" ) );
        CTextRecorder signIn( &pane, CAssetSourcesPane::SignInRequestedEvent );
        fixture.context->FlushChanges();
        const GUI::CRect rect = pane.GetControl( "SignIn" )->GetRect();
        fixture.context->InjectMouseMove( rect.left + rect.width / 2, rect.top + rect.height / 2 );
        fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 1 == signIn.count && "fakesrc" == signIn.last );

        // when the session starts the page follows
        fixture.fake->SetSignedIn( true );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake Store - signed in" == pane.GetStoreLine( 1 ) && !pane.GetControl( "SignIn" )->GetBoolProperty( "Visible" ); } ) );
        ASSERT_TRUE( "Sign in required" != pane.GetStoreStatusText() );
        ASSERT_FALSE( pane.SelectStore( "nothing" ) );
        ASSERT_TRUE( pane.SelectStore( "primary" ) && "primary" == pane.GetSelectedStoreId() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesPrimaryStore( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: a pack from the disk is ingested, found with the search form, shown in the inspector with its licence and opened in the views" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSourcesFixture fixture( "src_primary", true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        ASSERT_TRUE( pane.GetService()->IsCatalogueAvailable() && GUCEF_NULL != pane.GetSearchForm() );

        // nothing in the catalogue yet: the search says so
        GUI::CAssetSearchForm* form = pane.GetSearchForm();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching(); } ) );
        ASSERT_TRUE( "No results" == form->GetStatusText() );
        ASSERT_TRUE( form->GetWidget( "Saved" )->GetBoolProperty( "Visible" ) && form->GetSavedSearchNames().size() >= 20 );

        // a pack that is not there, and one that is
        ASSERT_FALSE( pane.AddDiskPack( fixture.dir + "/packs/Nothing" ) );
        ASSERT_TRUE( fixture.IngestPack( "Sword Pack" ) );
        ASSERT_TRUE( 1 == pane.GetStartedIngestCount() );
        GUI::CITransferQueueModel::TEntry entry;
        ASSERT_TRUE( fixture.FindTransfer( "Sword Pack", entry ) && "Ingest" == entry.kind && GUI::CITransferQueueModel::TRANSFERSTATE_DONE == entry.state );
        fixture.shell->GetTransfers()->Refresh();
        ASSERT_TRUE( 1 == fixture.shell->GetTransfers()->GetEntryCount() && !fixture.shell->GetTransfers()->IsEmptyStateShown() );

        ASSERT_TRUE( fixture.Search( "sword" ) );
        ASSERT_TRUE( form->GetShownRowCount() >= 2 && form->GetPage().total >= 2 );
        CORE::CString modelKey, textureKey;
        for ( UInt32 i=0; i<form->GetShownRowCount(); ++i )
        {
            if ( "sword_model" == form->GetShownRow( i ).title )
                modelKey = form->GetShownRow( i ).key;
            if ( "sword_d" == form->GetShownRow( i ).title )
                textureKey = form->GetShownRow( i ).key;
        }
        ASSERT_TRUE( !modelKey.IsNULLOrEmpty() && !textureKey.IsNULLOrEmpty() );
        ASSERT_TRUE( form->GetShownRow( 0 ).values.find( "pack" ) != form->GetShownRow( 0 ).values.end() && "Sword Pack" == form->GetShownRow( 0 ).values.find( "pack" )->second );

        // the facets count the kinds of the result
        ASSERT_TRUE( form->GetChoiceText( "kind", "static-model" ).HasSubstr( "(1)" ) >= 0 && form->GetChoiceText( "kind", "texture" ).HasSubstr( "(1)" ) >= 0 );

        // selecting a result: the inspector shows the asset, its pack and the licence with its restrictions
        ASSERT_TRUE( form->SelectResult( modelKey ) );
        const CORE::CDataNode& data = fixture.shell->GetInspector()->GetData();
        ASSERT_TRUE( "sword_model" == Attribute( data, "", "Name" ) && "gltf" == Attribute( data, "", "Format" ) );
        ASSERT_TRUE( "Sword Pack" == Attribute( data, "Pack", "Title" ) && "Forge Works" == Attribute( data, "Pack", "Seller" ) && "disk" == Attribute( data, "Pack", "Source" ) );
        const CORE::CDataNode* pack = data.FindChild( "Pack" );
        ASSERT_TRUE( GUCEF_NULL != pack && GUCEF_NULL != pack->FindChild( "Licence" ) );
        ASSERT_TRUE( "CC BY 4.0" == Attribute( *pack, "Licence", "Name" ) && "not allowed" == Attribute( *pack, "Licence", "Machine learning" ) && "required" == Attribute( *pack, "Licence", "Attribution" ) );
        ASSERT_TRUE( "allowed" == Attribute( *pack, "Licence", "Commercial use" ) );

        // opening it shows it in the views: the path comes from the catalogue, the location is a mount of the workbench
        CTextRecorder opened( &pane, CAssetSourcesPane::AssetOpenRequestedEvent );
        ASSERT_TRUE( form->OpenResult( modelKey ) );
        ASSERT_TRUE( 1 == opened.count && opened.last.HasSubstr( "sword_model.gltf" ) >= 0 );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ASSETKIND_MODEL == fixture.shell->GetShownKind(); } ) );
        ASSERT_TRUE( CAssetMounter::NormalizePath( opened.last ) == fixture.shell->GetShownPath() );
        // a picture of the pack opens in the texture view
        ASSERT_TRUE( form->SelectResult( textureKey ) && form->OpenResult( textureKey ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ASSETKIND_IMAGE == fixture.shell->GetShownKind(); } ) );

        // the same pack again is not ingested twice
        const UInt64 total = form->GetPage().total;
        ASSERT_TRUE( fixture.IngestPack( "Sword Pack" ) && fixture.Search( "sword" ) && total == form->GetPage().total );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "sources_primary_" + CORE::CString( GetTestRenderer() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesFilters( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: the filters of the search form narrow the catalogue (kind, format, size, licence) and the saved searches of the store are chosen, kept and deleted" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSourcesFixture fixture( "src_filters", true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened && fixture.IngestPack( "Sword Pack" ) );
        CAssetSourcesPane& pane = fixture.Pane();
        GUI::CAssetSearchForm* form = pane.GetSearchForm();
        form->Search();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );

        // the filter panel comes from the store: the vocabulary, the formats, the flags, what the licence allows and the ranges
        ASSERT_TRUE( GUCEF_NULL != form->GetChoiceWidget( "kind", "texture" ) && GUCEF_NULL != form->GetChoiceWidget( "format", "fbx" ) );
        ASSERT_TRUE( GUCEF_NULL != form->GetChoiceWidget( "flags", "rigged" ) && GUCEF_NULL != form->GetChoiceWidget( "allows", "machineLearning" ) && GUCEF_NULL != form->GetChoiceWidget( "source", "disk" ) );
        ASSERT_TRUE( GUCEF_NULL != form->GetChoiceWidget( "source", "fakesrc" ) && GUCEF_NULL != form->GetChoiceWidget( "licence", "attribution" ) );
        ASSERT_TRUE( GUCEF_NULL != form->GetRangeMinimumWidget( "megabytes" ) && GUCEF_NULL != form->GetRangeMinimumWidget( "triangles" ) && GUCEF_NULL != form->GetRangeMaximumWidget( "texture" ) );

        ASSERT_TRUE( form->SetChoice( "kind", "texture", true ) && fixture.WaitFor( [&]() { return !form->IsSearching(); } ) );
        ASSERT_TRUE( 1 == form->GetPage().total && "sword_d" == form->GetShownRow( 0 ).title );
        ASSERT_TRUE( form->SetChoice( "kind", "texture", false ) && fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );
        ASSERT_TRUE( form->SetChoice( "format", "png", true ) && fixture.WaitFor( [&]() { return !form->IsSearching(); } ) );
        form->ClearFilters();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );

        // a size range of the file in MB: nothing is as small as 1 byte
        GUI::CSearchQuery::TRange range;
        range.hasMaximum = true;
        range.maximum = 0.000001;
        ASSERT_TRUE( form->SetRange( "megabytes", range ) && fixture.WaitFor( [&]() { return !form->IsSearching() && 0 == form->GetPage().total; } ) );
        range.maximum = 5.0;
        ASSERT_TRUE( form->SetRange( "megabytes", range ) && fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );

        // the licence of the pack does not allow machine learning: asking for packs that do leaves it out
        form->ClearFilters();
        ASSERT_TRUE( form->SetChoice( "allows", "machineLearning", true ) && fixture.WaitFor( [&]() { return !form->IsSearching() && 0 == form->GetPage().total; } ) );
        form->ClearFilters();
        ASSERT_TRUE( form->SetChoice( "allows", "commercial", true ) && fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );
        form->ClearFilters();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );

        // the suggestions are the keywords of the index
        CORE::CString::StringVector suggestions;
        pane.GetSearchModel()->GetSuggestions( "swo", 5, suggestions );
        ASSERT_TRUE( !suggestions.empty() && "sword" == suggestions[ 0 ] );

        // the saved searches of the store: the precanned ones and the ones of the user
        const UInt32 precanned = (UInt32) form->GetSavedSearchNames().size();
        ASSERT_TRUE( precanned >= 20 );
        const CORE::CString firstName = form->GetSavedSearchNames()[ 0 ];
        GUI::TSavedSearchVector saved;
        pane.GetSearchModel()->GetSavedSearches( saved );
        ASSERT_TRUE( !saved.empty() && saved[ 0 ].builtIn && firstName == saved[ 0 ].name );
        ASSERT_TRUE( form->ChooseSavedSearch( saved[ 0 ].id ) && saved[ 0 ].id == form->GetChosenSavedSearchId() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching(); } ) );
        ASSERT_FALSE( form->DeleteChosenSavedSearch() );
        form->ClearFilters();
        ASSERT_TRUE( form->SetChoice( "kind", "texture", true ) && fixture.WaitFor( [&]() { return !form->IsSearching(); } ) );
        ASSERT_TRUE( form->SaveCurrentSearch( "My textures" ) && precanned + 1 == form->GetSavedSearchNames().size() );
        form->ClearFilters();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );
        GUI::TSavedSearchVector after;
        pane.GetSearchModel()->GetSavedSearches( after );
        CORE::CString mineId;
        for ( size_t i=0; i<after.size(); ++i )
            if ( "My textures" == after[ i ].name )
                mineId = after[ i ].id;
        ASSERT_TRUE( !mineId.IsNULLOrEmpty() && form->ChooseSavedSearch( mineId ) && fixture.WaitFor( [&]() { return !form->IsSearching() && 1 == form->GetPage().total; } ) );
        ASSERT_TRUE( form->DeleteChosenSavedSearch() && precanned == form->GetSavedSearchNames().size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesOnlineStore( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: the library of an online store is browsed in the content browser and an item shows its formats, files and licence in the inspector" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSourcesFixture fixture( "src_online", true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        ASSERT_TRUE( pane.SelectStore( "fakesrc" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return GUCEF_NULL != pane.GetLibrary( "fakesrc" ) && !pane.GetLibrary( "fakesrc" )->IsLoading() && 3 == pane.GetLibrary( "fakesrc" )->GetItemCount(); } ) );
        ASSERT_TRUE( "3 items" == pane.GetStoreStatusText() );
        GUI::CContentBrowserForm* browser = pane.GetOnlineBrowser( "fakesrc" );
        ASSERT_TRUE( GUCEF_NULL != browser && GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS == browser->GetViewMode() );
        fixture.Frames( 3 );
        const CORE::CString::StringVector folders = browser->GetListedNames();
        ASSERT_TRUE( 2 == folders.size() && "3d-model/" == folders[ 0 ] && "material/" == folders[ 1 ] );

        // selecting an item: the formats in the order of the preference, the details from the store, the licence
        ASSERT_TRUE( pane.SelectItem( "fakesrc", "weapon-1" ) );
        ASSERT_TRUE( "weapon-1" == pane.GetSelectedItemId() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 4 == pane.GetFormatChoices().size(); } ) );
        const CORE::CString::StringVector choices = pane.GetFormatChoices();
        ASSERT_TRUE( "gltf" == choices[ 0 ] && "fbx" == choices[ 1 ] && "obj" == choices[ 2 ] && "unreal-5-4" == choices[ 3 ] );
        ASSERT_TRUE( "gltf" == pane.GetSelectedFormatId() );
        const CORE::CDataNode& data = fixture.shell->GetInspector()->GetData();
        ASSERT_TRUE( "Iron Sword" == Attribute( data, "", "Title" ) && "Forge Works" == Attribute( data, "", "Seller" ) );
        ASSERT_TRUE( "Store EULA" == Attribute( data, "Licence", "Name" ) && "not allowed" == Attribute( data, "Licence", "Machine learning" ) );
        ASSERT_TRUE( GUCEF_NULL != data.FindChild( "Formats" ) && Attribute( data, "Formats", "glTF" ).HasSubstr( "2 files" ) >= 0 && Attribute( data, "Formats", "OBJ" ).HasSubstr( "30000 bytes" ) >= 0 );
        ASSERT_TRUE( "glTF" == Attribute( data, "Files", "Format" ) && Attribute( data, "Files", "weapon.gltf" ).HasSubstr( "81920" ) >= 0 && Attribute( data, "Files", "weapon_d.png" ).HasSubstr( "4096" ) >= 0 );

        // another format shows its files
        ASSERT_TRUE( pane.SelectFormat( 1 ) && "fbx" == pane.GetSelectedFormatId() );
        const CORE::CDataNode& fbx = fixture.shell->GetInspector()->GetData();
        ASSERT_TRUE( "FBX" == Attribute( fbx, "Files", "Format" ) && fbx.FindChild( "Files" )->GetAttribute( "weapon.fbx" ) != GUCEF_NULL );
        ASSERT_FALSE( pane.SelectFormat( 9 ) );
        ASSERT_FALSE( pane.SelectItem( "fakesrc", "nothing" ) );
        ASSERT_TRUE( pane.GetControl( "Import" )->GetBoolProperty( "Enabled" ) && pane.GetControl( "AddToPrimary" )->GetBoolProperty( "Enabled" ) );

        fixture.shell->ShowLeftTab( 2 );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 4 );
        DumpPicture( pixels, "sources_online_" + CORE::CString( GetTestRenderer() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesImport( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: Import queues a download that shows its progress in the transfer queue and lands in the download folder, with pause, resume, cancel, retry and remove" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSourcesFixture fixture( "src_import", true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        GUI::CTransferQueueForm* transfers = fixture.shell->GetTransfers();
        ASSERT_TRUE( transfers->IsEmptyStateShown() );
        ASSERT_TRUE( pane.SelectStore( "fakesrc" ) && fixture.WaitFor( [&]() { return 3 == pane.GetLibrary( "fakesrc" )->GetItemCount(); } ) );
        ASSERT_TRUE( pane.SelectItem( "fakesrc", "weapon-1" ) && fixture.WaitFor( [&]() { return 4 == pane.GetFormatChoices().size(); } ) );

        // the store hands out the first two chunks only
        fixture.fake->AllowBytes( 2 * fixture.fake->GetChunkSize() );
        CORE::CString jobId;
        ASSERT_TRUE( pane.ImportSelected( jobId ) && !jobId.IsNULLOrEmpty() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { ASSETSTORE::CAssetJobEntry job; return ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetJob( jobId, job ) && job.doneBytes >= 2 * 16384; } ) );
        transfers->Refresh();
        GUI::CITransferQueueModel::TEntry entry;
        ASSERT_TRUE( fixture.FindTransfer( jobId, entry ) );
        ASSERT_TRUE( "Iron Sword" == entry.name && "Download" == entry.kind && GUI::CITransferQueueModel::TRANSFERSTATE_RUNNING == entry.state );
        ASSERT_TRUE( 81920 + 4096 == entry.totalBytes && entry.doneBytes >= 32768 && entry.doneBytes < entry.totalBytes );
        ASSERT_TRUE( entry.GetProgress() > 0.3f && entry.GetProgress() < 0.5f );
        ASSERT_TRUE( entry.detail.HasSubstr( "fakesrc gltf" ) >= 0 && entry.folder.HasSubstr( "downloads/fakesrc/" ) >= 0 && entry.folder.HasSubstr( fixture.root ) == 0 );
        ASSERT_TRUE( 1 == transfers->GetEntryCount() && transfers->IsActionShown( jobId, GUI::CITransferQueueModel::TRANSFERACTION_PAUSE ) );

        // pause keeps the bytes, resume goes on
        ASSERT_TRUE( transfers->RequestAction( jobId, GUI::CITransferQueueModel::TRANSFERACTION_PAUSE ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { ASSETSTORE::CAssetJobEntry job; return ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetJob( jobId, job ) && ASSETSTORE::ASSETJOB_STATE_PAUSED == job.state; } ) );
        transfers->Refresh();
        ASSERT_TRUE( transfers->IsActionShown( jobId, GUI::CITransferQueueModel::TRANSFERACTION_RESUME ) && !transfers->IsActionShown( jobId, GUI::CITransferQueueModel::TRANSFERACTION_PAUSE ) );
        ASSERT_TRUE( transfers->RequestAction( jobId, GUI::CITransferQueueModel::TRANSFERACTION_RESUME ) );
        fixture.fake->AllowBytes( (UInt64) 1 << 40 );
        ASSERT_TRUE( fixture.WaitFor( [&]() { ASSETSTORE::CAssetJobEntry job; return ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetJob( jobId, job ) && ASSETSTORE::ASSETJOB_STATE_DONE == job.state; } ) );
        transfers->Refresh();
        ASSERT_TRUE( fixture.FindTransfer( jobId, entry ) && GUI::CITransferQueueModel::TRANSFERSTATE_DONE == entry.state && 1.0f == entry.GetProgress() );

        // the files are in the download folder, in a location that is a mount of the workbench
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        const CORE::CString folder = CAssetMounter::NormalizePath( entry.folder );
        ASSERT_TRUE( vfs.FileExists( folder + "/weapon.gltf" ) && vfs.FileExists( folder + "/weapon_d.png" ) && !vfs.FileExists( folder + "/weapon.gltf.part" ) );
        CAssetMounter::SMount mount;
        CORE::CString relative;
        ASSERT_TRUE( fixture.mounter.FindMount( folder, mount, relative ) );
        ASSERT_TRUE( vfs.GetFileSize( folder + "/weapon.gltf" ) == 81920 );

        // open folder is a request to show it in the content browser
        CTextRecorder folderRequest( &pane, CAssetSourcesPane::FolderOpenRequestedEvent );
        ASSERT_TRUE( transfers->RequestAction( jobId, GUI::CITransferQueueModel::TRANSFERACTION_OPEN_FOLDER ) );
        ASSERT_TRUE( 1 == folderRequest.count && folderRequest.last == folder && 0 == fixture.shell->GetLeftTab() );

        // a download that is cancelled and tried again; removing takes it out of the list
        ASSERT_TRUE( transfers->RequestAction( jobId, GUI::CITransferQueueModel::TRANSFERACTION_REMOVE ) );
        transfers->Refresh();
        ASSERT_TRUE( 0 == transfers->GetEntryCount() && transfers->IsEmptyStateShown() );
        fixture.fake->AllowBytes( 0 );
        ASSERT_TRUE( pane.SelectItem( "fakesrc", "weapon-1" ) && pane.SelectFormat( 1 ) );
        transfers->Refresh();
        CORE::CString second;
        ASSERT_TRUE( pane.ImportSelected( second ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { ASSETSTORE::CAssetJobEntry job; return ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetJob( second, job ) && ASSETSTORE::ASSETJOB_STATE_RUNNING == job.state; } ) );
        transfers->Refresh();
        ASSERT_TRUE( transfers->RequestAction( second, GUI::CITransferQueueModel::TRANSFERACTION_CANCEL ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { ASSETSTORE::CAssetJobEntry job; return ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetJob( second, job ) && ASSETSTORE::ASSETJOB_STATE_CANCELLED == job.state; } ) );
        transfers->Refresh();
        ASSERT_TRUE( transfers->IsActionShown( second, GUI::CITransferQueueModel::TRANSFERACTION_RETRY ) && transfers->IsActionShown( second, GUI::CITransferQueueModel::TRANSFERACTION_REMOVE ) );
        fixture.fake->AllowBytes( (UInt64) 1 << 40 );
        ASSERT_TRUE( transfers->RequestAction( second, GUI::CITransferQueueModel::TRANSFERACTION_RETRY ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { ASSETSTORE::CAssetJobEntry job; return ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetJob( second, job ) && ASSETSTORE::ASSETJOB_STATE_DONE == job.state; } ) );
        const CORE::CString second_folder = CAssetMounter::NormalizePath( pane.GetImportFolder( "fakesrc", "weapon-1", "Iron Sword" ) );
        ASSERT_TRUE( vfs.FileExists( second_folder + "/weapon.fbx" ) );

        fixture.shell->ShowLeftTab( 2 );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 4 );
        DumpPicture( pixels, "sources_transfers_" + CORE::CString( GetTestRenderer() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesAddToPrimary( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: Add to primary store downloads for the store and ingests the pack when it is done, with the keep and location of the page" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSourcesFixture fixture( "src_add", true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        ASSERT_TRUE( ASSETSTORE::ASSETKEEP_BOTH == pane.GetKeepPolicy() );
        ASSERT_TRUE( pane.SelectStore( "fakesrc" ) && fixture.WaitFor( [&]() { return 3 == pane.GetLibrary( "fakesrc" )->GetItemCount(); } ) );
        ASSERT_TRUE( pane.SelectItem( "fakesrc", "prop-1" ) && fixture.WaitFor( [&]() { return 1 == pane.GetFormatChoices().size(); } ) );

        // the keep choice of the combo box of the page
        GUI::CCombobox* keep = GUI::CWidget::As< GUI::CCombobox >( pane.GetControl( "Keep" ) );
        keep->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "0" );
        ASSERT_TRUE( ASSETSTORE::ASSETKEEP_ORIGINAL == pane.GetKeepPolicy() );
        CORE::CString jobId;
        ASSERT_TRUE( pane.AddSelectedToPrimaryStore( jobId ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 1 == pane.GetStartedIngestCount() && 1 == fixture.CountIngests( GUI::CITransferQueueModel::TRANSFERSTATE_DONE ); } ) );
        GUI::CITransferQueueModel::TEntry entry;
        ASSERT_TRUE( fixture.FindTransfer( jobId, entry ) && GUI::CITransferQueueModel::TRANSFERSTATE_DONE == entry.state && entry.detail.HasSubstr( "for the primary store" ) >= 0 );

        // the pack is in the catalogue as a pack of that store, with the original kept
        ASSETSTORE::CAssetSearchQuery query;
        query.sources.insert( "fakesrc" );
        query.resultKind = "packs";
        query.pageSize = 10;
        ASSETSTORE::CAssetSearchResultPage page;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->Search( query, page ) && 1 == page.hits.size() );
        ASSETSTORE::CAssetPackRecord pack;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetPack( page.hits[ 0 ].packId, pack ) );
        ASSERT_TRUE( "fakesrc" == pack.sourceId && "prop-1" == pack.sourceItemId && "Wooden Barrel" == pack.title && ASSETSTORE::ASSETKEEP_ORIGINAL == pack.keep );

        // and found with the search form of the page
        ASSERT_TRUE( pane.SelectStore( "primary" ) && fixture.Search( "barrel" ) );
        ASSERT_TRUE( pane.GetSearchForm()->GetPage().total >= 1 );
        ASSERT_TRUE( pane.GetSearchForm()->SetChoice( "source", "fakesrc", true ) && fixture.WaitFor( [&]() { return !pane.GetSearchForm()->IsSearching(); } ) );
        ASSERT_TRUE( pane.GetSearchForm()->GetPage().total >= 1 );
        // the transfer is kept in the list until it is removed
        GUI::CTransferQueueForm* transfers = fixture.shell->GetTransfers();
        transfers->Refresh();
        ASSERT_TRUE( 2 == transfers->GetEntryCount() );
        GUI::CITransferQueueModel::TEntry ingest;
        ASSERT_TRUE( fixture.FindTransfer( CAssetStoreTransferModel::IngestIdPrefix, ingest ) && transfers->IsActionShown( ingest.id, GUI::CITransferQueueModel::TRANSFERACTION_REMOVE ) );
        ASSERT_FALSE( transfers->IsActionShown( ingest.id, GUI::CITransferQueueModel::TRANSFERACTION_PAUSE ) );
        ASSERT_TRUE( transfers->RequestAction( ingest.id, GUI::CITransferQueueModel::TRANSFERACTION_REMOVE ) && 0 == pane.GetTransferModel()->GetIngestCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesThumbnails( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: the thumbnails of the library of a store come from its listing pictures through the asset store service, and answer at once for what has none" )
    try
    {
        CORE::CString::StringVector ids;
        SOURCESTEST::TFakeSourceFactory factory;
        ASSETSTORE::TAssetStoreProviderPtr provider = factory.Create();
        SOURCESTEST::CFakeSourceProvider* fake = static_cast< SOURCESTEST::CFakeSourceProvider* >( provider.GetPointerAlways() );
        GUCEF::vector< SOURCESTEST::CFakeSourceProvider::SFormatSpec > none;
        fake->AddItem( "pic-1", "With picture", "Seller", "3d-model", none, CORE::CString::Empty, "http://127.0.0.1:1/missing.png" );
        fake->AddItem( "pic-2", "Without picture", "Seller", "3d-model", none );
        ASSETSTORE::CAssetStoreLibraryFileSystemAccess library( provider );
        // the pictures come through the asset store service; the one of this process keeps them in its image cache
        ASSETSTORE::CAssetStoreServiceConfig config;
        config.primaryStore.database.backendName = "none";
        config.loadDownloadQueue = false;
        config.startDownloads = false;
        ASSETSTORE::CLocalAssetStoreService service( config );
        service.Open();
        ASSERT_TRUE( service.IsOpen() && GUCEF_NULL != service.GetImageCache() );
        library.Refresh();
        for ( UInt32 i=0; i<300 && library.GetItemCount() < 2; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            MT::PrecisionDelay( 5 );
        }
        ASSERT_TRUE( 2 == library.GetItemCount() );
        const CORE::CString withPicture = library.GetPathOfItem( "pic-1" );
        const CORE::CString without = library.GetPathOfItem( "pic-2" );
        CAssetStoreThumbnailProvider thumbnails( service, library );
        CThumbnailRecorder recorder( &thumbnails, GUI::CIThumbnailProvider::ThumbnailReadyEvent );

        // an item without a picture, a folder and a path that is not there: answered at once, without pixels
        thumbnails.RequestThumbnail( without, 64 );
        ASSERT_TRUE( 1 == recorder.count && !recorder.lastHadPixels && recorder.lastPath == without && 64 == recorder.lastSize );
        thumbnails.RequestThumbnail( "/3d-model", 64 );
        thumbnails.RequestThumbnail( "/nothing/at/all", 64 );
        ASSERT_TRUE( 3 == recorder.count && 0 == thumbnails.GetWaitingCount() );

        // a picture that cannot be fetched is answered without pixels when the cache gives up; the request is asked once for any number of askers
        thumbnails.RequestThumbnail( withPicture, 64 );
        thumbnails.RequestThumbnail( withPicture, 64 );
        ASSERT_TRUE( 2 == thumbnails.GetWaitingCount() );
        for ( UInt32 i=0; i<600 && recorder.count < 5; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            MT::PrecisionDelay( 5 );
        }
        ASSERT_TRUE( 5 == recorder.count && !recorder.lastHadPixels && recorder.lastPath == withPicture && 0 == thumbnails.GetWaitingCount() );
        ASSERT_TRUE( 1 == service.GetImageCache()->GetNetworkRequestCount() );

        // a request that is cancelled is not answered
        thumbnails.RequestThumbnail( withPicture, 32 );
        thumbnails.CancelThumbnail( withPicture, 32 );
        ASSERT_TRUE( 0 == thumbnails.GetWaitingCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesConvert( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: Convert on a selected result converts its pack with the converter of the store, shows the progress in the transfer queue and the converted file opens in the views" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSourcesFixture fixture( "src_convert", true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        GUI::CAssetSearchForm* form = pane.GetSearchForm();

        // the converter of the settings is on the store; nothing is selected, so there is nothing to convert
        ASSERT_TRUE( GUCEF_NULL != pane.GetConverterHook() && GUCEF_NULL != pane.GetControl( "Convert" ) );
        ASSERT_FALSE( pane.ConvertSelectedPack() && 0 == pane.GetStartedConversionCount() );
        ASSERT_FALSE( pane.GetControl( "Convert" )->GetBoolProperty( "Enabled" ) );

        // the workbench tests load no importer: the converter has nothing it can convert and the pack waits for conversion
        ASSERT_TRUE( fixture.IngestPack( "Sword Pack" ) && fixture.Search( "sword" ) );
        CORE::CString modelKey, textureKey;
        for ( UInt32 i=0; i<form->GetShownRowCount(); ++i )
        {
            if ( "sword_model" == form->GetShownRow( i ).title )
                modelKey = form->GetShownRow( i ).key;
            if ( "sword_d" == form->GetShownRow( i ).title )
                textureKey = form->GetShownRow( i ).key;
        }
        ASSERT_TRUE( !modelKey.IsNULLOrEmpty() && !textureKey.IsNULLOrEmpty() );
        ASSERT_TRUE( form->SelectResult( modelKey ) && !pane.GetSelectedPackId().IsNULLOrEmpty() );
        const CORE::CString packId = pane.GetSelectedPackId();
        ASSETSTORE::CAssetPackRecord pack;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetPack( packId, pack ) && ASSETSTORE::ASSETPACK_STATE_CONVERSION_PENDING == pack.state );
        ASSERT_TRUE( "conversionPending" == Attribute( fixture.shell->GetInspector()->GetData(), "Pack", "State" ) );
        ASSERT_TRUE( pane.GetControl( "Convert" )->GetBoolProperty( "Enabled" ) && pane.GetControl( "Convert" )->GetBoolProperty( "Visible" ) );

        // the converter that stands in for the pipeline, and Convert: a task of the store, a transfer of the queue
        SOURCESTEST::CFakeConverterHook fake;
        pane.SetConverterHook( &fake );
        ASSERT_TRUE( pane.ConvertSelectedPack() && 1 == pane.GetStartedConversionCount() );
        GUI::CITransferQueueModel::TEntry entry;
        ASSERT_TRUE( fixture.WaitFor( [&]() { return fixture.FindTransfer( CAssetStoreTransferModel::ConversionIdPrefix, entry ) && GUI::CITransferQueueModel::TRANSFERSTATE_DONE == entry.state; } ) );
        ASSERT_TRUE( "Conversion" == entry.kind && "Sword Pack" == entry.name && entry.GetProgress() >= 1.0f && 1 == fake.GetCallCount() );
        ASSERT_TRUE( pane.GetTransferModel()->IsActionAvailable( entry, GUI::CITransferQueueModel::TRANSFERACTION_REMOVE ) );
        ASSERT_FALSE( pane.GetTransferModel()->IsActionAvailable( entry, GUI::CITransferQueueModel::TRANSFERACTION_CANCEL ) );

        // the pack in the catalogue, and the inspector that shows it again
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetPack( packId, pack ) && ASSETSTORE::ASSETPACK_STATE_INGESTED == pack.state );
        ASSERT_TRUE( pack.formatsConverted.end() != pack.formatsConverted.find( "gltf" ) && "fake" == pack.conversion.profile );
        const CORE::CDataNode& data = fixture.shell->GetInspector()->GetData();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "ingested" == Attribute( fixture.shell->GetInspector()->GetData(), "Pack", "State" ); } ) );
        ASSERT_TRUE( "converted" == Attribute( *data.FindChild( "Pack" ), "Conversion", "Outcome" ) && "gltf" == Attribute( data, "", "Converted format" ) );
        ASSERT_TRUE( Attribute( data, "", "Converted path" ) == "intermediate/sword_model.gltf" );

        // the converted file opens in the views, from the catalogue; a picture has none
        CTextRecorder opened( &pane, CAssetSourcesPane::AssetOpenRequestedEvent );
        ASSERT_TRUE( pane.OpenResultConverted( modelKey ) && 1 == opened.count && opened.last.HasSubstr( "/intermediate/" + ASSETSTORE::CPackStorageLayout( pack.rootPath ).GetPackFolder() + "/sword_model.gltf" ) >= 0 );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ASSETKIND_MODEL == fixture.shell->GetShownKind(); } ) );
        ASSERT_TRUE( CAssetMounter::NormalizePath( opened.last ) == fixture.shell->GetShownPath() );
        ASSERT_FALSE( pane.OpenResultConverted( textureKey ) );

        // a pack the converter refuses: ingested as original, the page says so, and Convert shows the refusal in the queue
        const CORE::CString second = fixture.dir + "/packs/Axe Pack";
        CORE::CreateDirs( second + "/Textures" );
        WriteTestFile( second + "/pack.json", "{ \"title\": \"Axe Pack\", \"author\": \"Forge Works\", \"licence\": { \"name\": \"CC BY 4.0\" } }" );
        WriteTestFile( second + "/axe_model.gltf", BuildTestGltf() + "\n" );
        WriteTestPng( second + "/Textures/axe_d.png", 24, 8 );
        fake.SetRefuse( true );
        const UInt32 ingested = fixture.CountIngests( GUI::CITransferQueueModel::TRANSFERSTATE_DONE );
        ASSERT_TRUE( pane.AddDiskPack( second ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return fixture.CountIngests( GUI::CITransferQueueModel::TRANSFERSTATE_DONE ) > ingested; } ) );
        ASSERT_TRUE( fixture.Search( "axe" ) );
        CORE::CString axeKey;
        for ( UInt32 i=0; i<form->GetShownRowCount(); ++i )
            if ( "axe_model" == form->GetShownRow( i ).title )
                axeKey = form->GetShownRow( i ).key;
        ASSERT_TRUE( !axeKey.IsNULLOrEmpty() && form->SelectResult( axeKey ) );
        const CORE::CDataNode& axeData = fixture.shell->GetInspector()->GetData();
        ASSERT_TRUE( "conversionRefused" == Attribute( axeData, "Pack", "State" ) && Attribute( axeData, "Pack", "Reason" ) == "conversion refused: the licence record does not allow it" );
        ASSERT_TRUE( "refused" == Attribute( *axeData.FindChild( "Pack" ), "Conversion", "Outcome" ) );
        ASSERT_TRUE( pane.ConvertSelectedPack() && 2 == pane.GetStartedConversionCount() );
        ASSERT_TRUE( fixture.WaitFor( [&]() {
            GUI::CITransferQueueModel::TEntryVector entries;
            pane.GetTransferModel()->GetEntries( entries );
            for ( size_t i=0; i<entries.size(); ++i )
            {
                if ( "Axe Pack" == entries[ i ].name && entries[ i ].id.HasSubstr( CAssetStoreTransferModel::ConversionIdPrefix ) == 0 && entries[ i ].IsEnded() )
                {
                    entry = entries[ i ];
                    return true;
                }
            }
            return false; } ) );
        ASSERT_TRUE( GUI::CITransferQueueModel::TRANSFERSTATE_DONE == entry.state && entry.message.HasSubstr( "Refused: the licence record does not allow it" ) == 0 );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "sources_convert_" + CORE::CString( GetTestRenderer() ) );

        // removing the finished transfers
        ASSERT_TRUE( pane.GetTransferModel()->PerformAction( entry.id, GUI::CITransferQueueModel::TRANSFERACTION_REMOVE ) );
        ASSERT_TRUE( 1 == pane.GetTransferModel()->GetConversionCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  The findings (warnings and errors, summed over the matrix) the shell is still allowed per left tab and rule; recorded by the
 *  first run of the layout lint and lowered as the layout steps fix the shell, never raised. A scope that is not listed is allowed none
 */
static const char* const ShellLintBudget =
    "# scope rule count: the warnings and errors after G1 (Noto Sans 16 px, 15 combinations), the most of the runs on the headless\n"
    "# surface, X11 and Vulkan plus one: the Sources tab varies by a few between runs (the state colour of a transfer row\n"
    "# and the rows of the search change with the timing of the store's events)\n"
    "WorkbenchShell.Sources L01 66\n"
    "WorkbenchShell.Sources L02 28\n"
    "WorkbenchShell.Sources L03 6\n"
    "WorkbenchShell.Sources L04 168\n"
    "WorkbenchShell.Sources L05 12\n"
    "WorkbenchShell.Sources L07 9\n"
    "WorkbenchShell.Sources L08 8\n"
    "WorkbenchShell.Sources L09 8\n"
    "WorkbenchShell.Sources L10 68\n"
    "WorkbenchShell.Sources L12 15\n"
    "WorkbenchShell.Sources L15 14\n"
    "WorkbenchShell.Content L01 25\n"
    "WorkbenchShell.Content L02 26\n"
    "WorkbenchShell.Content L03 4\n"
    "WorkbenchShell.Content L04 84\n"
    "WorkbenchShell.Content L05 32\n"
    "WorkbenchShell.Content L08 2\n"
    "WorkbenchShell.Content L09 5\n"
    "WorkbenchShell.Content L10 26\n"
    "WorkbenchShell.Content L12 2\n"
    "WorkbenchShell.Content L15 13\n"
    "WorkbenchShell.Scene L01 22\n"
    "WorkbenchShell.Scene L02 25\n"
    "WorkbenchShell.Scene L03 4\n"
    "WorkbenchShell.Scene L04 16\n"
    "WorkbenchShell.Scene L05 32\n"
    "WorkbenchShell.Scene L08 2\n"
    "WorkbenchShell.Scene L09 5\n"
    "WorkbenchShell.Scene L10 22\n"
    "WorkbenchShell.Scene L12 2\n"
    "WorkbenchShell.Scene L15 13\n"
    ;

/** The sweep of the workbench: the frames of the application let the shell catch up, the text scale registers the font of the workbench again */
class CShellLintSweep : public GUI::CLayoutLintSweep
{
    public:

    explicit CShellLintSweep( CSourcesFixture& fixture ) : GUI::CLayoutLintSweep(), m_fixture( fixture ) {}

    protected:

    virtual bool ApplyTextScale( GUI::CGUIContext& context, Float32 scale ) GUCEF_VIRTUAL_OVERRIDE
    {
        return context.RegisterFont( "WorkbenchFont", GUI::CGuiGlobal::DefaultUiFontPath, (UInt32) ( 16.0f * scale + 0.5f ) );
    }

    virtual void Settle( GUI::CGUIContext& context ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_fixture.Frames( 3 );
    }

    private:

    CSourcesFixture& m_fixture;
};

/*-------------------------------------------------------------------------*/

static void
TestSourcesLayoutLint( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: the layout lint over the shell with the Sources, Content and Scene tabs, from 640 x 480 to ultrawide and pseudo-localised, stays within its budget" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSourcesFixture fixture( "src_lint", true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        if ( !fixture.ready || !fixture.sourcesOpened )
            return;

        // what the screen of the defects showed: a pack in the primary store, a search with results, the font of the workbench (16 px)
        ASSERT_TRUE( fixture.IngestPack( "Sword Pack" ) && fixture.Search( "sword" ) );
        const bool workbenchFont = fixture.context->RegisterFont( "WorkbenchFont", GUI::CGuiGlobal::DefaultUiFontPath, 16 );
        ASSERT_TRUE( workbenchFont );

        // the Content tab on the folder of the test (its packs and store), so that what it lists is the same on every machine
        ASSERT_TRUE( fixture.shell->OpenLocation( fixture.dir, true ) );

        GUI::CLayoutLintBudget budget;
        CORE::CString problems;
        ASSERT_TRUE( budget.LoadFromText( ShellLintBudget, problems ) );
        const Int32 tabs[] = { 2, 0, 1 };
        const char* const scopes[] = { "WorkbenchShell.Sources", "WorkbenchShell.Content", "WorkbenchShell.Scene" };
        for ( UInt32 t=0; t<3; ++t )
        {
            fixture.shell->ShowLeftTab( tabs[ t ] );
            fixture.Frames( 3 );
            GUI::CLayoutLintMatrix matrix;
            matrix.SetTextScaleSupported( true );
            CShellLintSweep sweep( fixture );
            sweep.Run( *fixture.context, matrix, GUI::CLayoutLintOptions() );
            for ( UInt32 c=0; c<matrix.GetCombinationCount(); ++c )
            {
                const GUI::CLayoutLintMatrix::TCombination& combination = matrix.GetCombination( c );
                if ( 1600 == combination.width && 900 == combination.height && !combination.pseudoLocalized )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LayoutLint " << scopes[ t ] << " at " << matrix.DescribeCombination( c ) << ": " << GUI::CLayoutLint::Report( sweep.GetFindings( c ), 80 ) );
            }

            CORE::CString over, improvements;
            const bool within = budget.Check( scopes[ t ], sweep.GetRuleCounts(), over, improvements );
            const CORE::CString summary = CORE::CString( "== " ) + scopes[ t ] + " (Noto Sans 16 px, " + GetTestRenderer() + "): " +
                                          CORE::UInt32ToString( sweep.GetCount( GUI::LAYOUTLINTSEVERITY_ERROR ) ) + " errors, " +
                                          CORE::UInt32ToString( sweep.GetCount( GUI::LAYOUTLINTSEVERITY_WARNING ) ) + " warnings, " +
                                          CORE::UInt32ToString( sweep.GetCount( GUI::LAYOUTLINTSEVERITY_INFO ) ) + " info over " +
                                          CORE::UInt32ToString( matrix.GetCombinationCount() ) + " combinations\n" + sweep.GetSummary() +
                                          GUI::CLayoutLintBudget::FormatCounts( scopes[ t ], sweep.GetRuleCounts() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LayoutLint baseline " << summary );
            const char* reportPath = getenv( "GUCEF_TEST_LAYOUT_LINT_REPORT" );
            if ( GUCEF_NULL != reportPath && '\0' != *reportPath )
            {
                CORE::CFileAccess file( reportPath, "ab" );
                if ( file.IsValid() )
                    file.Write( summary.C_String(), 1, summary.Length() );
            }
            if ( !improvements.IsNULLOrEmpty() )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LayoutLint: " << improvements );
            if ( !within )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LayoutLint: over the budget:\n" << over );
            ASSERT_TRUE( within || !workbenchFont );        // the budget is for the font of the workbench
        }
        fixture.shell->ShowLeftTab( 2 );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "sources_lint_" + CORE::CString( GetTestRenderer() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** The layout lint of a form of the shell at three sizes: no warning and no error */
static bool
LintSyncForm( CSourcesFixture& fixture, GUI::CFormEx* form )
{
    const Int32 widths[] = { 0, 1000, 1600 };
    const Int32 heights[] = { 0, 700, 900 };
    bool clean = true;
    for ( UInt32 size=0; size<3; ++size )
    {
        fixture.context->SetLogicalResolution( widths[ size ], heights[ size ] );
        fixture.Frames( 3 );
        GUI::CLayoutLintOptions options;
        options.root = GUCEF_NULL != form ? form->GetRootWidget() : GUCEF_NULL;
        GUI::TLayoutLintFindingVector findings;
        GUI::CLayoutLint lint;
        lint.Run( *fixture.context, options, findings );
        const UInt32 problems = GUI::CLayoutLint::CountAtLeast( findings, GUI::LAYOUTLINTSEVERITY_WARNING );
        if ( 0 != problems )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Library sync lint at " << widths[ size ] << "x" << heights[ size ] << ": " << GUI::CLayoutLint::Report( findings, 20 ) );
            clean = false;
        }
    }
    fixture.context->SetLogicalResolution( 0, 0 );
    return clean;
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesLibrarySync( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: Download selected is one library sync of the selected items; the library sync panel plans the rest with its counts, syncs it at once with its progress and notice, and passes the layout lint" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSourcesFixture fixture( "src_sync", true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        if ( !fixture.ready || !fixture.sourcesOpened )
            return;
        CAssetSourcesPane& pane = fixture.Pane();
        ASSETSTORE::CIAssetStoreService& service = *pane.GetService();

        // the profile of the store kept by the service: no reserve of free space on the small folder of the test, no index job
        ASSETSTORE::CAssetLibrarySyncProfile profile;
        profile.reservePercent = 0;
        profile.reserveBytes = 0;
        profile.indexAfter = false;
        ASSETSTORE::CAssetServiceResult set;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == service.SetSyncProfile( CORE::CString::Empty, "fakesrc", profile, set ) );

        // two items selected in the browser of the store: one sync of those two, every format of each
        ASSERT_TRUE( pane.SelectStore( "fakesrc" ) && fixture.WaitFor( [&]() { return 3 == pane.GetLibrary( "fakesrc" )->GetItemCount(); } ) );
        CORE::CString::StringVector selection;
        selection.push_back( "weapon-1" );
        selection.push_back( "prop-1" );
        ASSERT_TRUE( pane.SelectItems( "fakesrc", selection ) && 2 == pane.GetSelectedItemIds().size() );
        CORE::CString jobId;
        ASSERT_TRUE( pane.AddSelectedItemsToPrimaryStore( jobId ) && !jobId.IsNULLOrEmpty() );
        ASSETSTORE::CAssetJobEntry job;
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ASSETSTORE::ASSETSTORE_STATUS_OK == service.GetJob( jobId, job ) && job.IsEnded(); }, 4000 ) );
        ASSERT_TRUE( ASSETSTORE::ASSETJOB_STATE_DONE == job.state && "sync" == job.subKind && 5 == job.itemsTotal && 5 == job.itemsDone );

        // the panel: a row for the store, the plan of what is left (the third item), then the sync of it at once
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( shell.ShowLibrarySync() );
        CLibrarySyncForm* form = shell.GetLibrarySyncForm();
        CLibrarySyncController* controller = shell.GetLibrarySyncController();
        ASSERT_TRUE( GUCEF_NULL != form && GUCEF_NULL != controller && form->IsOpen() );
        if ( GUCEF_NULL == form || GUCEF_NULL == controller )
            return;
        ASSERT_TRUE( form->SelectStore( "fakesrc" ) && "fakesrc" == form->GetSelectedStoreId() );
        UInt32 row = 0;
        for ( UInt32 i=0; i<controller->GetStoreCount(); ++i )
            if ( "fakesrc" == controller->GetStore( i )->storeId )
                row = i;
        ASSERT_TRUE( "signed in" == form->GetCellText( row, CLibrarySyncForm::COLUMN_SIGNIN ) && form->IsSyncEnabled() );
        ASSERT_TRUE( form->PlanNow() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { const CLibrarySyncController::SStoreState* s = controller->FindStore( "fakesrc" ); return GUCEF_NULL != s && s->planned; }, 2000 ) );
        fixture.Frames( 3 );
        form->Update( 300 );
        ASSERT_TRUE( "1" == form->GetCellText( row, CLibrarySyncForm::COLUMN_MISSING ) && "2" == form->GetCellText( row, CLibrarySyncForm::COLUMN_CURRENT ) );
        ASSERT_TRUE( "0" == form->GetCellText( row, CLibrarySyncForm::COLUMN_STALE ) && !form->GetCellText( row, CLibrarySyncForm::COLUMN_DOWNLOAD ).IsNULLOrEmpty() );
        ASSERT_TRUE( !form->GetCellText( row, CLibrarySyncForm::COLUMN_FREEDISK ).IsNULLOrEmpty() && form->GetProgressText().HasSubstr( "1 missing" ) > 0 );
        DumpPicture( fixture.Frames( 3 ), "library_sync_plan" );

        ASSERT_TRUE( form->SyncNow() );
        bool sawRunning = false;
        for ( UInt32 i=0; i<2000; ++i )
        {
            fixture.Frames( 1 );
            form->Update( 300 );
            const CLibrarySyncController::SStoreState* state = controller->FindStore( "fakesrc" );
            if ( GUCEF_NULL != state && state->running && !state->runJobId.IsNULLOrEmpty() )
                sawRunning = true;
            if ( GUCEF_NULL != state && !state->running && !state->lastResult.IsNULLOrEmpty() )
                break;
        }
        fixture.Frames( 3 );
        form->Update( 300 );
        const CLibrarySyncController::SStoreState* state = controller->FindStore( "fakesrc" );
        ASSERT_TRUE( sawRunning && GUCEF_NULL != state && !state->running && 0 == state->lastResult.HasSubstr( "done" ) );
        ASSERT_TRUE( form->GetCellText( row, CLibrarySyncForm::COLUMN_LASTRESULT ).HasSubstr( "ingested" ) > 0 && form->GetNoticeText().HasSubstr( "Library sync of" ) == 0 );
        ASSERT_TRUE( 1 == controller->GetNotifications().size() && 100.0f == form->GetProgressPercent() );
        DumpPicture( fixture.Frames( 3 ), "library_sync_done" );

        ASSERT_TRUE( LintSyncForm( fixture, form ) );
        form->Close();
        ASSERT_FALSE( form->IsOpen() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchSourcesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.Sources" );

    TestSourcesSettings();
    TestSourcesQueryMapping();
    TestSourcesTab();
    TestSourcesPrimaryStore();
    TestSourcesFilters();
    TestSourcesOnlineStore();
    TestSourcesImport();
    TestSourcesAddToPrimary();
    TestSourcesLibrarySync();
    TestSourcesThumbnails();
    TestSourcesConvert();
    TestSourcesLayoutLint();
}

/*-------------------------------------------------------------------------*/
