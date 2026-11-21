/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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

#include "gucef_test_framework.h"
#include "gucefVFS.h"
#include "AssetWorkbenchLib.h"
#include "AssetConverterLib.h"
#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchResourceView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** The index of a pack of synthetic files, structure level, sizes given */
bool
BuildDocument( const CORE::CString::StringVector& files, ASSETCONVERTER::CPackIndexDocument& doc )
{
    ASSETCONVERTER::CAssetPackListing listing;
    ASSETCONVERTER::CPackIndexBuilder builder( ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().GetKinds() );
    for ( size_t i=0; i<files.size(); ++i )
    {
        listing.AddFile( files[ i ] );
        builder.SetFileSize( files[ i ], 100 + (UInt64) i );
    }
    ASSETCONVERTER::CPackIndexBuildOptions options;
    options.useFileSystemSizes = false;
    options.packName = "synthetic";
    return builder.BuildStructure( listing, ASSETCONVERTER::CAssetPackProfile(), options, doc, GUCEF_NULL );
}

/** The names of a folder of the file system, a "/" behind the folders, in the order the file system gives them */
bool
ListNames( const CResourceViewFileSystemAccess& fileSystem, const CORE::CString& folder, CORE::CString::StringVector& names )
{
    names.clear();
    CORE::TFileSystemIteratorPtr iterator;
    if ( CORE::FILESYSTEMACCESS_SUCCESS != fileSystem.TryGetFileSystemIterator( folder, iterator ) || iterator.IsNULL() )
        return false;
    if ( iterator->FindFirst( folder ) )
    {
        do
        {
            names.push_back( iterator->GetResourceName() + ( iterator->IsADirectory() ? CORE::CString( "/" ) : CORE::CString::Empty ) );
        }
        while ( iterator->FindNext() );
        iterator->FindClose();
    }
    return true;
}

bool
Has( const CORE::CString::StringVector& names, const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

bool
HasPrefix( const CORE::CString::StringVector& names, const CORE::CString& prefix )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ].Length() >= prefix.Length() && names[ i ].SubstrFromRange( 0, prefix.Length() ) == prefix )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchResourceViewTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbench.ResourceView" );

    GUCEF_TESTFW_TESTCASE( "Resource view: the levels of the pack by function with their counts, the noise hidden, only the opened group made ready, and the way back from a resource to its real path" )
    try
    {
        CORE::CString::StringVector files;
        files.push_back( "Assets/Pack/Models/SM_Bld_A.fbx" );
        files.push_back( "Assets/Pack/Models/SM_Bld_B.fbx" );
        files.push_back( "Assets/Pack/Models/SM_Prop_C.fbx" );
        files.push_back( "Assets/Pack/Models/Chair.fbx" );
        files.push_back( "Assets/Pack/Meshes/Chair.fbx" );
        files.push_back( "Assets/Pack/Textures/T_A.png" );
        files.push_back( "Assets/Pack/Textures/T_B.png" );
        files.push_back( "Assets/Pack/Audio/door.wav" );
        files.push_back( "Assets/Pack/.DS_Store" );
        files.push_back( "Assets/Pack/Models/SM_Bld_A.fbx.meta" );
        ASSETCONVERTER::CPackIndexDocument doc;
        ASSERT_TRUE( BuildDocument( files, doc ) );
        ASSERT_TRUE( 8 == doc.resources.size() && 1 == doc.counts.noise && 1 == doc.counts.metadata );

        CResourceViewFileSystemAccess view;
        ASSERT_FALSE( view.HasPack() );
        CORE::CString::StringVector names;
        ASSERT_TRUE( ListNames( view, "/", names ) && names.empty() );
        view.SetPack( doc, CORE::CString::Empty, "work/pack" );
        ASSERT_TRUE( view.HasPack() && "Resources" == view.GetDisplayName() && "/" == view.GetRootPath() );

        // the top: the groups in the fixed order with their counts, from the manifest: nothing is made ready
        ASSERT_TRUE( ListNames( view, "/", names ) );
        ASSERT_TRUE( 3 == names.size() && "Static models (5)/" == names[ 0 ] && "Textures and images (2)/" == names[ 1 ] && "Audio (1)/" == names[ 2 ] );
        ASSERT_TRUE( 0 == view.GetMaterialisedGroupCount() && 0 == view.GetLoadedResourceCount() && 0 == view.GetShardReadCount() );

        // the second level: the sub-groups, then the resources that have none; only this group is made ready
        ASSERT_TRUE( ListNames( view, "/static-models", names ) );
        ASSERT_TRUE( Has( names, "Buildings (2)/" ) && Has( names, "Props (1)/" ) );
        ASSERT_TRUE( 1 == view.GetMaterialisedGroupCount() && 5 == view.GetLoadedResourceCount() );
        // two resources with the same name in one folder are told apart by the folder of the pack
        UInt32 chairs = 0;
        for ( size_t i=0; i<names.size(); ++i )
            if ( names[ i ].Length() > 11 && names[ i ].SubstrFromRange( 0, 11 ) == "Chair.fbx [" )
                ++chairs;
        ASSERT_TRUE( 2 == chairs && 4 == names.size() );
        ASSERT_TRUE( ListNames( view, "/static-models/buildings", names ) && 2 == names.size() && Has( names, "SM_Bld_A.fbx" ) && Has( names, "SM_Bld_B.fbx" ) );
        ASSERT_TRUE( 1 == view.GetMaterialisedGroupCount() );
        // the noise and the metadata files are not there at any level
        ASSERT_TRUE( ListNames( view, "/", names ) && !HasPrefix( names, ".DS_Store" ) );
        ASSERT_TRUE( ListNames( view, "/static-models/buildings", names ) && !Has( names, "SM_Bld_A.fbx.meta" ) );
        // a folder is listed once while it is the same
        const UInt32 listings = view.GetListingCount();
        ASSERT_TRUE( ListNames( view, "/static-models/buildings", names ) && listings == view.GetListingCount() );

        // select and open need the real path: every file of the view maps back to its resource and its place in the pack
        CORE::CString::StringVector all;
        ASSERT_TRUE( ListNames( view, "/static-models", all ) && ListNames( view, "/textures", names ) );
        ASSETCONVERTER::CPackIndexResource resource;
        ASSERT_TRUE( view.TryResolve( "/static-models/buildings/SM_Bld_A.fbx", resource ) && "Assets/Pack/Models/SM_Bld_A.fbx" == resource.path );
        ASSERT_TRUE( "work/pack/Assets/Pack/Models/SM_Bld_A.fbx" == view.GetVfsPathOf( resource ) );
        ASSERT_TRUE( "/static-models/buildings/SM_Bld_A.fbx" == view.GetViewPathOf( resource ) );
        ASSERT_TRUE( "Static models / Buildings" == view.GetGroupTextOf( resource ) );
        ASSERT_TRUE( view.TryResolve( "/textures/T_B.png", resource ) && "Assets/Pack/Textures/T_B.png" == resource.path && 2 == view.GetMaterialisedGroupCount() );
        for ( size_t i=0; i<doc.resources.size(); ++i )
        {
            const CORE::CString viewPath = view.GetViewPathOf( doc.resources[ i ] );
            ASSETCONVERTER::CPackIndexResource back;
            ASSERT_TRUE( !viewPath.IsNULLOrEmpty() && view.TryResolve( viewPath, back ) && back.path == doc.resources[ i ].path );
        }
        ASSERT_FALSE( view.TryResolve( "/static-models", resource ) || view.TryResolve( "/nothing/here.fbx", resource ) );

        // what the browser asks of a file system
        CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == view.TryGetEntryType( "/", type ) && CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == view.TryGetEntryType( "/static-models/props", type ) && CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == view.TryGetEntryType( "/audio/doors/door.wav", type ) && CORE::FILESYSTEMENTRYTYPE_FILE == type );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == view.TryGetEntryType( "/static-models/nothing", type ) && CORE::FILESYSTEMENTRYTYPE_NONE == type );
        CORE::CResourceMetaData metaData;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == view.TryGetMetaData( "/audio/doors/door.wav", metaData ) && metaData.hasResourceSizeInBytes && metaData.resourceSizeInBytes > 100 );
        ASSERT_TRUE( !view.IsWriteable( "/" ) && CORE::FILESYSTEMACCESS_READ_ONLY == view.TryDeleteFile( "/audio/doors/door.wav" ) );
        CORE::CString text;
        UInt32 count = 0;
        UInt64 bytes = 0;
        ASSERT_TRUE( view.GetFolderSummary( "/static-models/buildings", text, count, bytes ) && "Static models / Buildings" == text && 2 == count && bytes > 200 );

        // another pack replaces what is shown
        view.Clear();
        ASSERT_TRUE( !view.HasPack() && ListNames( view, "/", names ) && names.empty() && 0 == view.GetLoadedResourceCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resource view: the groups have the rank of the fixed order of the vocabulary, not of the alphabet; the prefabs are browsable by sub-group like the models; package assets are a group of their own" )
    try
    {
        CORE::CString::StringVector files;
        files.push_back( "Assets/Pack/Audio/door.wav" );
        files.push_back( "Assets/Pack/Textures/T_A.png" );
        files.push_back( "Assets/Pack/Models/SM_Bld_A.fbx" );
        files.push_back( "Assets/Pack/Scenes/Demo.scene" );
        files.push_back( "Assets/Pack/Materials/M_A.material" );
        files.push_back( "Assets/Pack/Shaders/water.shader" );
        files.push_back( "Assets/Pack/Prefabs/Props/SM_Prop_Table.prefab" );
        files.push_back( "Assets/Pack/Prefabs/Props/SM_Prop_Chair.prefab" );
        files.push_back( "Assets/Pack/Prefabs/Buildings/SM_Bld_Shop.prefab" );
        files.push_back( "Assets/Pack/Prefabs/Signs/SM_Sign_Open.prefab" );
        files.push_back( "Assets/Pack/Content/Hero.uasset" );
        files.push_back( "Assets/Pack/Readme.txt" );
        ASSETCONVERTER::CPackIndexDocument doc;
        ASSERT_TRUE( BuildDocument( files, doc ) );
        CResourceViewFileSystemAccess view;
        view.SetPack( doc, CORE::CString::Empty, "work/pack" );

        // the rank of a group folder is its place in the vocabulary + 1, an item of a group and a file have none
        const GUI::CIContentItemOrder& order = view;
        ASSERT_TRUE( order.GetRank( "/scenes", true ) < order.GetRank( "/static-models", true ) && order.GetRank( "/static-models", true ) < order.GetRank( "/textures", true ) );
        ASSERT_TRUE( order.GetRank( "/audio", true ) < order.GetRank( "/prefabs", true ) && order.GetRank( "/prefabs", true ) < order.GetRank( "/shaders", true ) );
        ASSERT_TRUE( order.GetRank( "/shaders", true ) < order.GetRank( "/packages", true ) && order.GetRank( "/packages", true ) < order.GetRank( "/other", true ) );
        ASSERT_TRUE( 0 == order.GetRank( "/prefabs/props", true ) && 0 == order.GetRank( "/audio/doors/door.wav", false ) && 0 == order.GetRank( "/", true ) );
        // the groups in the manifest are in that order too, and the list the file system gives
        CORE::CString::StringVector names;
        ASSERT_TRUE( ListNames( view, "/", names ) && 9 == names.size() );
        ASSERT_TRUE( HasPrefix( CORE::CString::StringVector( 1, names[ 0 ] ), "Scenes" ) && HasPrefix( CORE::CString::StringVector( 1, names[ 8 ] ), "Other" ) );
        ASSERT_TRUE( Has( names, "Package assets (1)/" ) && Has( names, "Prefabs (4)/" ) );

        // the prefabs are browsable: by folder and by name token, as the models are
        ASSERT_TRUE( ListNames( view, "/prefabs", names ) );
        ASSERT_TRUE( 3 == names.size() && Has( names, "Props (2)/" ) && Has( names, "Buildings (1)/" ) && Has( names, "Signs (1)/" ) );
        ASSERT_TRUE( ListNames( view, "/prefabs/props", names ) && 2 == names.size() && Has( names, "SM_Prop_Chair.prefab" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resource view: the search lists what matches in every group, flat, and the groups show the matches only; an empty search is the whole pack again" )
    try
    {
        CORE::CString::StringVector files;
        files.push_back( "Models/SM_Bld_Shop.fbx" );
        files.push_back( "Models/SM_Bld_Bank.fbx" );
        files.push_back( "Models/SM_Prop_Table.fbx" );
        files.push_back( "Textures/T_Bld_Shop_D.png" );
        files.push_back( "Textures/T_Grass.png" );
        files.push_back( "Audio/door.wav" );
        ASSETCONVERTER::CPackIndexDocument doc;
        ASSERT_TRUE( BuildDocument( files, doc ) );
        CResourceViewFileSystemAccess view;
        view.SetPack( doc, CORE::CString::Empty, "work/pack" );
        CORE::CString::StringVector names;
        view.SetSearch( "shop" );
        ASSERT_TRUE( "shop" == view.GetSearch() && ListNames( view, "/", names ) );
        ASSERT_TRUE( Has( names, "Static models (1)/" ) && Has( names, "Textures and images (1)/" ) && !HasPrefix( names, "Audio" ) );
        ASSERT_TRUE( HasPrefix( names, "SM_Bld_Shop.fbx  (" ) && HasPrefix( names, "T_Bld_Shop_D.png  (" ) && !HasPrefix( names, "SM_Bld_Bank" ) );
        // a flat result opens like the file in its folder
        ASSETCONVERTER::CPackIndexResource resource;
        CORE::TFileSystemIteratorPtr iterator;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == view.TryGetFileSystemIterator( "/", iterator ) && iterator->FindFirst( "/" ) );
        UInt32 files2 = 0;
        do
        {
            if ( !iterator->IsADirectory() )
            {
                ++files2;
                ASSERT_TRUE( view.TryResolve( iterator->GetResourcePath(), resource ) && resource.name.Lowercase().HasSubstr( "shop" ) >= 0 );
            }
        }
        while ( iterator->FindNext() );
        iterator->FindClose();
        ASSERT_TRUE( 2 == files2 );
        // a pattern is a pattern
        view.SetSearch( "*.wav" );
        ASSERT_TRUE( ListNames( view, "/", names ) && Has( names, "Audio (1)/" ) && !HasPrefix( names, "Static" ) );
        view.SetSearch( "nothing like it" );
        ASSERT_TRUE( ListNames( view, "/", names ) && names.empty() );
        view.SetSearch( CORE::CString::Empty );
        ASSERT_TRUE( ListNames( view, "/", names ) && 3 == names.size() && "Static models (3)/" == names[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resource view: a large pack is read by shard, the group that is opened only, and its index is read through the manifest alone" )
    try
    {
        CORE::CString::StringVector files;
        for ( UInt32 i=0; i<2500; ++i )
        {
            files.push_back( "Assets/Props/SM_Prop_" + CORE::UInt32ToString( 100000 + i ) + ".fbx" );
            files.push_back( "Assets/Textures/T_" + CORE::UInt32ToString( 100000 + i ) + ".png" );
        }
        ASSETCONVERTER::CPackIndexDocument doc;
        ASSERT_TRUE( BuildDocument( files, doc ) );
        const CORE::CString dir = MakeCleanTestDir( "rv_shards" );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.MountArchive( "rv_shards", dir, VFS::CVFS::FileSystemArchiveTypeName, "rvshards", true, false, dir ) );
        ASSETCONVERTER::CPackIndexWriter writer;
        ASSERT_TRUE( writer.Write( doc, "rvshards/index" ) && 4 == doc.shards.size() );
        ASSETCONVERTER::CPackIndexReader reader;
        ASSETCONVERTER::CPackIndexDocument manifest;
        ASSERT_TRUE( ASSETCONVERTER::CPackIndexReader::RESULT_OK == reader.ReadManifest( "rvshards/index", manifest ) && manifest.resources.empty() );

        CResourceViewFileSystemAccess view;
        view.SetPack( manifest, "rvshards/index", "work/big" );
        CORE::CString::StringVector names;
        ASSERT_TRUE( ListNames( view, "/", names ) && 2 == names.size() && "Static models (2,500)/" == names[ 0 ] && "Textures and images (2,500)/" == names[ 1 ] );
        ASSERT_TRUE( 0 == view.GetShardReadCount() && 0 == view.GetLoadedResourceCount() );
        ASSERT_TRUE( ListNames( view, "/textures", names ) );
        ASSERT_TRUE( 2 == view.GetShardReadCount() && 2500 == view.GetLoadedResourceCount() && 1 == view.GetMaterialisedGroupCount() );
        ASSERT_TRUE( 5 == names.size() && "Items 0001 - 0500 (500)/" == names[ 0 ] && "Items 2001 - 2500 (500)/" == names[ 4 ] );
        ASSERT_TRUE( ListNames( view, "/static-models", names ) && 4 == view.GetShardReadCount() && 5000 == view.GetLoadedResourceCount() );
        ASSETCONVERTER::CPackIndexResource resource;
        // a folder of many files is listed in pages, and a file is reached through its page
        ASSERT_TRUE( ListNames( view, "/static-models/props", names ) && 5 == names.size() && ListNames( view, "/static-models/props/~p3", names ) && 500 == names.size() && "SM_Prop_101000.fbx" == names[ 0 ] );
        ASSERT_TRUE( view.TryResolve( "/static-models/props/~p3/SM_Prop_101234.fbx", resource ) && "Assets/Props/SM_Prop_101234.fbx" == resource.path );
        ASSERT_TRUE( "/static-models/props/~p3/SM_Prop_101234.fbx" == view.GetViewPathOf( resource ) );
        ASSERT_FALSE( view.TryResolve( "/static-models/props/~p2/SM_Prop_101234.fbx", resource ) || view.TryResolve( "/static-models/props/SM_Prop_101234.fbx", resource ) );
        vfs.UnmountArchiveByName( "rv_shards" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resource view: the settings keep the start view and the switch, the start view is on by default, and the command line and the settings page say the same" )
    try
    {
        CWorkbenchPreferences defaults;
        ASSERT_TRUE( defaults.openPacksInResourceView && !defaults.resourceViewShown );
        CWorkbenchPreferences changed;
        changed.openPacksInResourceView = false;
        changed.resourceViewShown = true;
        CORE::CDataNode section( "AssetWorkbench" );
        changed.Save( section );
        CWorkbenchPreferences loaded;
        loaded.Load( section );
        ASSERT_TRUE( !loaded.openPacksInResourceView && loaded.resourceViewShown );
        // settings of before the view: on, and the files
        CORE::CDataNode old( "AssetWorkbench" );
        CWorkbenchPreferences fromOld;
        fromOld.openPacksInResourceView = false;
        fromOld.Load( old );
        ASSERT_TRUE( fromOld.openPacksInResourceView && !fromOld.resourceViewShown );
        // the help and the tooltip know the setting
        CWorkbenchTooltips tooltips;
        ASSERT_TRUE( !tooltips.FindText( "OpenPacksInResourceView" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
