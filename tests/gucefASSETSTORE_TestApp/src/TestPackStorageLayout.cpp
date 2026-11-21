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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */

#include "TestAssetStoreSupport.h"

#include "TestPackStorageLayout.h"

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt32
BytesOf( const CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

/** Whether the bytes are well formed UTF-8: a cut in the middle of a character is not */
bool
IsWellFormedUtf8( const CString& text )
{
    const unsigned char* bytes = (const unsigned char*) text.C_String();
    const UInt32 length = BytesOf( text );
    UInt32 i = 0;
    while ( i < length )
    {
        const unsigned char c = bytes[ i ];
        UInt32 follow = 0;
        if ( c < 0x80 )
            follow = 0;
        else
        if ( 0xC0 == ( c & 0xE0 ) )
            follow = 1;
        else
        if ( 0xE0 == ( c & 0xF0 ) )
            follow = 2;
        else
        if ( 0xF0 == ( c & 0xF8 ) )
            follow = 3;
        else
            return false;
        if ( follow > 0 && i + follow >= length )
            return false;
        for ( UInt32 f=1; f<=follow; ++f )
            if ( 0x80 != ( bytes[ i + f ] & 0xC0 ) )
                return false;
        i += follow + 1;
    }
    return true;
}

bool
HasNoUnsafeCharacters( const CString& name )
{
    const char* chars = name.C_String();
    const UInt32 length = BytesOf( name );
    for ( UInt32 i=0; i<length; ++i )
    {
        const unsigned char c = (unsigned char) chars[ i ];
        if ( c < 0x20 || 0x7F == c || '/' == c || '\\' == c || ':' == c || '*' == c || '?' == c || '"' == c || '<' == c || '>' == c || '|' == c )
            return false;
    }
    return length > 0 && '.' != chars[ 0 ] && ' ' != chars[ 0 ] && '.' != chars[ length-1 ] && ' ' != chars[ length-1 ];
}

struct SNameCase
{
    const char* source;
    const char* expected;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPackStorageLayoutTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "PackStorageLayout" );

    GUCEF_TESTFW_TESTCASE( "Pack folder names: the source name with its extension stays, unsafe parts change, case is kept" )
        try
        {
            static const SNameCase cases[] =
            {
                { "POLYGON_Shops_Unity_2022_3_v1_6_6.unitypackage", "POLYGON_Shops_Unity_2022_3_v1_6_6.unitypackage" },
                { "MyFooBar.zip", "MyFooBar.zip" },
                { "MyFooBar.ZIP", "MyFooBar.ZIP" },
                { "scarletjohansson.mhpkg", "scarletjohansson.mhpkg" },
                { "archive.tar.gz", "archive.tar.gz" },
                { "Medieval Weapons", "Medieval Weapons" },
                { "dir/sub/MyFooBar.zip", "MyFooBar.zip" },
                { "dir\\sub\\MyFooBar.zip", "MyFooBar.zip" },
                { "folder/", "folder" },
                { "bad:name*here?.zip", "bad_name_here_.zip" },
                { "quote\"lt<gt>pipe|.pak", "quote_lt_gt_pipe_.pak" },
                { "tab\there.zip", "tab_here.zip" },
                { "  ..hidden pack.zip  ", "hidden pack.zip" },
                { "double   blank.zip", "double blank.zip" },
                { "name...", "name" },
                { "file.", "file" },
                { ".zip", "zip" },
                { "-== Pack X ==-", "-== Pack X ==-" },
                { "", "pack" },
                { "/", "pack" },
                { "...", "pack" }
            };
            for ( size_t c=0; c<sizeof(cases)/sizeof(cases[0]); ++c )
            {
                const CString name = CPackStorageLayout::MakePackFolderName( cases[ c ].source );
                ASSERT_TRUE( name == cases[ c ].expected );
                ASSERT_TRUE( HasNoUnsafeCharacters( name ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Pack folder names: the device names of Windows are not used, with or without an extension, any case" )
        try
        {
            static const SNameCase cases[] =
            {
                { "CON", "_CON" }, { "con", "_con" }, { "PRN", "_PRN" }, { "AUX", "_AUX" }, { "NUL", "_NUL" },
                { "nul.txt", "_nul.txt" }, { "CON.zip", "_CON.zip" }, { "COM1.zip", "_COM1.zip" }, { "LPT9.unitypackage", "_LPT9.unitypackage" },
                { "COM0", "_COM0" }, { "Aux.tar.gz", "_Aux.tar.gz" },
                { "COM10.zip", "COM10.zip" }, { "CONSOLE.zip", "CONSOLE.zip" }, { "CONTROL", "CONTROL" }, { "AUX1", "AUX1" }
            };
            for ( size_t c=0; c<sizeof(cases)/sizeof(cases[0]); ++c )
                ASSERT_TRUE( CPackStorageLayout::MakePackFolderName( cases[ c ].source ) == cases[ c ].expected );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Pack folder names: a long name is cut to the limit and keeps its extension, also with multi byte characters" )
        try
        {
            CString longStem;
            for ( UInt32 i=0; i<300; ++i )
                longStem += 'a';
            CString name = CPackStorageLayout::MakePackFolderName( longStem + ".unitypackage" );
            ASSERT_TRUE( BytesOf( name ) == CPackStorageLayout::MaxFolderNameBytes );
            ASSERT_TRUE( name.HasSubstr( ".unitypackage", false ) == (Int32) ( BytesOf( name ) - 13 ) );
            ASSERT_TRUE( 'a' == name.C_String()[ 0 ] );

            name = CPackStorageLayout::MakePackFolderName( longStem );
            ASSERT_TRUE( BytesOf( name ) == CPackStorageLayout::MaxFolderNameBytes );
            CString exact;
            for ( UInt32 i=0; i<120; ++i )
                exact += 'b';
            ASSERT_TRUE( CPackStorageLayout::MakePackFolderName( exact ) == exact );
            ASSERT_TRUE( BytesOf( CPackStorageLayout::MakePackFolderName( exact + "x" ) ) == CPackStorageLayout::MaxFolderNameBytes );

            // two byte and three byte characters: the cut never lands inside one
            CString multi;
            for ( UInt32 i=0; i<100; ++i )
                multi += "\xC3\xA4\xE6\x97\xA5";
            name = CPackStorageLayout::MakePackFolderName( multi + ".zip" );
            ASSERT_TRUE( BytesOf( name ) <= CPackStorageLayout::MaxFolderNameBytes && IsWellFormedUtf8( name ) );
            ASSERT_TRUE( BytesOf( name ) > 4 && CString( name.C_String() + BytesOf( name ) - 4 ) == ".zip" );

            // the characters themselves stay
            ASSERT_TRUE( CPackStorageLayout::MakePackFolderName( "St\xC3\xA4" "dte_Pack_\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E.zip" ) == "St\xC3\xA4" "dte_Pack_\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E.zip" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Collisions: the first pack keeps the clean name, the next gets the short id, the same input gives the same answer" )
        try
        {
            const CString idA = "pk19c3a4b5d6e7f0a1b2c3d4";
            const CString idB = "pk19c3a4b5ffff0000aaaa5555";
            ASSERT_TRUE( "19c3a4b5" == CPackStorageLayout::MakeShortId( idA ) && "19c3a4b5" == CPackStorageLayout::MakeShortId( idB ) );
            ASSERT_TRUE( "abc" == CPackStorageLayout::MakeShortId( "abc" ) );

            CString::StringSet taken;
            ASSERT_TRUE( "MyFoo.zip" == CPackStorageLayout::ChoosePackFolderName( "MyFoo.zip", idA, taken ) );
            taken.insert( "myfoo.zip" );
            const CString second = CPackStorageLayout::ChoosePackFolderName( "MyFoo.zip", idA, taken );
            ASSERT_TRUE( "MyFoo.zip [19c3a4b5]" == second );
            ASSERT_TRUE( second == CPackStorageLayout::ChoosePackFolderName( "MyFoo.zip", idA, taken ) );

            // the shares are not case sensitive everywhere
            ASSERT_TRUE( "MYFOO.ZIP [19c3a4b5]" == CPackStorageLayout::ChoosePackFolderName( "MYFOO.ZIP", idA, taken ) );
            ASSERT_TRUE( "Other.zip" == CPackStorageLayout::ChoosePackFolderName( "Other.zip", idA, taken ) );

            // the short id is taken as well: the whole id
            taken.insert( second.Lowercase() );
            ASSERT_TRUE( "MyFoo.zip [" + idB + "]" == CPackStorageLayout::ChoosePackFolderName( "MyFoo.zip", idB, taken ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Layout 2: the three areas hold the folder of the pack, logical paths resolve below their area" )
        try
        {
            const CPackStorageLayout layout( CPackStorageLayout::MakeRootPath( "My Pack.zip" ) );
            ASSERT_TRUE( layout.IsAreaLayout() && CPackStorageLayout::LayoutAreas == layout.GetVersion() && "My Pack.zip" == layout.GetPackFolder() );
            ASSERT_TRUE( "layout2:My Pack.zip" == layout.GetStoredRootPath() );
            const CString root = "assetstore/main";
            ASSERT_TRUE( layout.Resolve( root, "original/Meshes/a.fbx" ) == "assetstore/main/original/My Pack.zip/Meshes/a.fbx" );
            ASSERT_TRUE( layout.Resolve( root, "intermediate/report.json" ) == "assetstore/main/intermediate/My Pack.zip/report.json" );
            ASSERT_TRUE( layout.Resolve( root, "gucef-ogre3d/desktop/a.mesh" ) == "assetstore/main/gucef-ogre3d/My Pack.zip/desktop/a.mesh" );
            ASSERT_TRUE( layout.Resolve( "/assetstore/main/", "/original/a.fbx" ) == "assetstore/main/original/My Pack.zip/a.fbx" );
            ASSERT_TRUE( layout.Resolve( root, "original" ) == "assetstore/main/original/My Pack.zip" );
            ASSERT_TRUE( layout.GetOriginalFolder( root ) == "assetstore/main/original/My Pack.zip" );
            ASSERT_TRUE( layout.GetIntermediateFolder( root, "desktop" ) == "assetstore/main/intermediate/My Pack.zip" );
            ASSERT_TRUE( layout.GetCookedFolder( root, "desktop" ) == "assetstore/main/gucef-ogre3d/My Pack.zip" );
            ASSERT_TRUE( "intermediate" == layout.GetIntermediatePrefix( "desktop" ) && "gucef-ogre3d" == layout.GetCookedPrefix( "desktop" ) && "original" == layout.GetOriginalPrefix() );
            ASSERT_TRUE( layout.GetPackIndexPath( root, "desktop" ) == "assetstore/main/intermediate/My Pack.zip/pack-index.json" );
            ASSERT_TRUE( layout.GetBrowseFolder( root ) == "assetstore/main/original/My Pack.zip" );

            // what the earlier layout kept below converted/ lives in the intermediate area once moved
            ASSERT_TRUE( layout.Resolve( root, "converted/desktop/a.gltf" ) == "assetstore/main/intermediate/My Pack.zip/converted/desktop/a.gltf" );

            CString::StringVector folders;
            layout.GetOwnedFolders( root, folders );
            ASSERT_TRUE( 3 == folders.size() && folders[ 0 ] == "assetstore/main/original/My Pack.zip" && folders[ 1 ] == "assetstore/main/intermediate/My Pack.zip" &&
                         folders[ 2 ] == "assetstore/main/gucef-ogre3d/My Pack.zip" );
            ASSERT_TRUE( CPackStorageLayout::IsAreaName( "original" ) && CPackStorageLayout::IsAreaName( "intermediate" ) && CPackStorageLayout::IsAreaName( "gucef-ogre3d" ) &&
                         !CPackStorageLayout::IsAreaName( "staging" ) && !CPackStorageLayout::IsAreaName( "converted" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Layout 1: a root path of the earlier layout resolves below its folder, in place packs are not owned" )
        try
        {
            const CPackStorageLayout layout( "packs/disk/Forge Works/Medieval Weapons [pk1]" );
            ASSERT_TRUE( !layout.IsAreaLayout() && CPackStorageLayout::LayoutFolders == layout.GetVersion() && layout.GetPackFolder().IsNULLOrEmpty() );
            ASSERT_TRUE( "packs/disk/Forge Works/Medieval Weapons [pk1]" == layout.GetStoredRootPath() );
            const CString root = "assetstore/main";
            ASSERT_TRUE( layout.Resolve( root, "original/a.fbx" ) == "assetstore/main/packs/disk/Forge Works/Medieval Weapons [pk1]/original/a.fbx" );
            ASSERT_TRUE( layout.Resolve( root, "converted/test/a.gltf" ) == "assetstore/main/packs/disk/Forge Works/Medieval Weapons [pk1]/converted/test/a.gltf" );
            ASSERT_TRUE( layout.GetOriginalFolder( root ) == "assetstore/main/packs/disk/Forge Works/Medieval Weapons [pk1]/original" );
            ASSERT_TRUE( layout.GetIntermediateFolder( root, "test" ) == "assetstore/main/packs/disk/Forge Works/Medieval Weapons [pk1]/converted/test" );
            ASSERT_TRUE( layout.GetCookedFolder( root, "test" ) == layout.GetIntermediateFolder( root, "test" ) );
            ASSERT_TRUE( "converted/test" == layout.GetIntermediatePrefix( "test" ) && "converted/test" == layout.GetCookedPrefix( "test" ) );
            ASSERT_TRUE( layout.GetBrowseFolder( root ) == "assetstore/main/packs/disk/Forge Works/Medieval Weapons [pk1]" );

            CString::StringVector folders;
            layout.GetOwnedFolders( root, folders );
            ASSERT_TRUE( 1 == folders.size() && folders[ 0 ] == "assetstore/main/packs/disk/Forge Works/Medieval Weapons [pk1]" );

            // a pack referenced in place is a folder of the owner
            const CPackStorageLayout inPlace( "Rocks" );
            ASSERT_TRUE( !inPlace.IsAreaLayout() && inPlace.Resolve( root, "Rocks.fbx" ) == "assetstore/main/Rocks/Rocks.fbx" );
            inPlace.GetOwnedFolders( root, folders );
            ASSERT_TRUE( folders.empty() );

            // the empty root path and a bare marker are the location itself
            const CPackStorageLayout empty( CString::Empty );
            ASSERT_TRUE( !empty.IsAreaLayout() && empty.Resolve( root, "a.fbx" ) == "assetstore/main/a.fbx" );
            ASSERT_TRUE( !CPackStorageLayout( "layout2:" ).IsAreaLayout() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}
