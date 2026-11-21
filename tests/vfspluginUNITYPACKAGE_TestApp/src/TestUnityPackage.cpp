/*
 *  vfspluginUNITYPACKAGE_TestApp: tests of the Unity package VFS plugin
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

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestUnityPackage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/** a source of a repeating byte pattern of a given size which takes no memory or disk space */
class CPatternSource : public CORE::CIOAccess
{
    public:

    CPatternSource( UInt64 size ) : CORE::CIOAccess(), m_size( size ), m_pos( 0 ) {}

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE { m_pos = 0; }
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt64 wanted = (UInt64) esize * elements;
        UInt64 left = m_size - m_pos;
        UInt64 count = wanted < left ? wanted : left;
        UInt8* out = static_cast< UInt8* >( dest );
        for ( UInt64 i=0; i<count; ++i )
            out[ i ] = PatternByte( m_pos + i );
        m_pos += count;
        return esize > 0 ? (UInt32) ( count / esize ) : 0;
    }

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_pos; }
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE { return -1; }
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE { char c = 0; Read( &c, 1, 1 ); return c; }
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_pos >= m_size; }
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_size; }
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CPatternSource( m_size ); }

    static UInt8 PatternByte( UInt64 position ) { return (UInt8) ( position % 251 ); }

    private:

    UInt64 m_size;
    UInt64 m_pos;
};

/*-------------------------------------------------------------------------*/

static CORE::CDynamicBuffer
MakeContent( const CORE::CString& text )
{
    CORE::CDynamicBuffer content;
    if ( text.Length() > 0 )
        content.CopyFrom( text.Length(), text.C_String() );
    return content;
}

/*-------------------------------------------------------------------------*/

/** binary content of a given size with a recognizable seed so contents of different files differ */
static CORE::CDynamicBuffer
MakeBinaryContent( UInt32 size, UInt8 seed )
{
    CORE::CDynamicBuffer content;
    for ( UInt32 i=0; i<size; ++i )
    {
        UInt8 value = (UInt8) ( seed + i * 7 + ( i >> 8 ) );
        content.Append( &value, 1 );
    }
    return content;
}

/*-------------------------------------------------------------------------*/

static bool
AddEntry( VFS::CIArchiveWriter& writer, const CORE::CString& name, const CORE::CDynamicBuffer& content )
{
    VFS::CArchiveWriterEntrySettings settings;
    settings.modificationTime = CORE::CDateTime( 2021, 6, 15, 12, 34, 56, 0, 0 );
    CORE::CDynamicBufferAccess access( content );
    return writer.AddFile( name, access, settings );
}

/*-------------------------------------------------------------------------*/

static const char* GuidHero = "0123456789abcdef0123456789abcdef";
static const char* GuidSkin = "11111111111111111111111111111111";
static const char* GuidModels = "22222222222222222222222222222222";
static const char* GuidPrefab = "33333333333333333333333333333333";
static const char* GuidEmptyFile = "44444444444444444444444444444444";

/*-------------------------------------------------------------------------*/

static CORE::CDynamicBuffer HeroAsset = MakeBinaryContent( 300, 3 );
static CORE::CDynamicBuffer HeroMeta = MakeContent( "fileFormatVersion: 2\nguid: 0123456789abcdef0123456789abcdef\n" );
static CORE::CDynamicBuffer HeroPreview = MakeBinaryContent( 64, 99 );
static CORE::CDynamicBuffer SkinAsset = MakeBinaryContent( 100 * 1024, 17 );        // over the size that is kept in memory
static CORE::CDynamicBuffer SkinMeta = MakeContent( "fileFormatVersion: 2\nguid: 11111111111111111111111111111111\n" );
static CORE::CDynamicBuffer ModelsMeta = MakeContent( "fileFormatVersion: 2\nfolderAsset: yes\n" );
static CORE::CDynamicBuffer PrefabAsset = MakeContent( "%YAML 1.1\n--- !u!1 &100000\nGameObject:\n" );
static CORE::CDynamicBuffer PrefabMeta = MakeContent( "fileFormatVersion: 2\nguid: 33333333333333333333333333333333\n" );
static CORE::CDynamicBuffer IconContent = MakeBinaryContent( 32, 5 );

/*-------------------------------------------------------------------------*/

/** writes a package like the Unity editor does, with the entries in a variety of orders */
static bool
WriteTestPackage( VFS::CVFS& vfs, const CORE::CString& path )
{
    CORE::CFileAccess output( path, "wb" );
    if ( !output.IsValid() )
        return false;
    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "tgz" );
    if ( writer.IsNULL() || !writer->Open( output ) )
        return false;

    bool ok = true;

    // A file with all parts, the pathname has a trailing newline
    ok = ok && writer->AddDirectory( GuidHero, CORE::CDateTime( 2021, 6, 15, 12, 34, 56, 0, 0 ) );
    ok = ok && AddEntry( *writer, CORE::CString( GuidHero ) + "/asset", HeroAsset );
    ok = ok && AddEntry( *writer, CORE::CString( GuidHero ) + "/asset.meta", HeroMeta );
    ok = ok && AddEntry( *writer, CORE::CString( GuidHero ) + "/pathname", MakeContent( "Assets/Models/Hero.fbx\n" ) );
    ok = ok && AddEntry( *writer, CORE::CString( GuidHero ) + "/preview.png", HeroPreview );

    // A larger file in a nested path, pathname without a line end, written last
    ok = ok && AddEntry( *writer, CORE::CString( GuidSkin ) + "/asset", SkinAsset );
    ok = ok && AddEntry( *writer, CORE::CString( GuidSkin ) + "/asset.meta", SkinMeta );
    ok = ok && AddEntry( *writer, CORE::CString( GuidSkin ) + "/pathname", MakeContent( "Assets/Models/Textures/Skin.png" ) );

    // A folder: no asset
    ok = ok && AddEntry( *writer, CORE::CString( GuidModels ) + "/asset.meta", ModelsMeta );
    ok = ok && AddEntry( *writer, CORE::CString( GuidModels ) + "/pathname", MakeContent( "Assets/Models" ) );

    // A deeply nested file, the pathname has a Windows line end and a second line
    ok = ok && AddEntry( *writer, CORE::CString( GuidPrefab ) + "/pathname", MakeContent( "Assets/Prefabs/Deep/Nested/Thing.prefab\r\nsecond line\r\n" ) );
    ok = ok && AddEntry( *writer, CORE::CString( GuidPrefab ) + "/asset", PrefabAsset );
    ok = ok && AddEntry( *writer, CORE::CString( GuidPrefab ) + "/asset.meta", PrefabMeta );

    // An empty file
    ok = ok && AddEntry( *writer, CORE::CString( GuidEmptyFile ) + "/pathname", MakeContent( "Assets/Empty.txt\n" ) );
    ok = ok && AddEntry( *writer, CORE::CString( GuidEmptyFile ) + "/asset", CORE::CDynamicBuffer() );

    // What a store download adds at the root, and a stray entry in an asset folder
    ok = ok && AddEntry( *writer, ".icon.png", IconContent );
    ok = ok && AddEntry( *writer, CORE::CString( GuidHero ) + "/unknown.bin", IconContent );

    return writer->Close() && ok;
}

/*-------------------------------------------------------------------------*/

/** a package of many assets that are all over the size that is kept in memory, so that they are served from the spill file */
static const UInt32 BigAssetCount = 24;
static const UInt32 BigAssetSize = 200 * 1024;

static CORE::CString
BigAssetGuid( UInt32 index )
{
    CORE::CString number = CORE::UInt32ToString( 1000 + index );
    return "9999999999999999999999999999" + number;
}

static bool
WriteBigPackage( VFS::CVFS& vfs, const CORE::CString& path )
{
    CORE::CFileAccess output( path, "wb" );
    if ( !output.IsValid() )
        return false;
    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "tgz" );
    if ( writer.IsNULL() || !writer->Open( output ) )
        return false;
    bool ok = true;
    for ( UInt32 i=0; i<BigAssetCount; ++i )
    {
        const CORE::CString guid = BigAssetGuid( i );
        ok = ok && AddEntry( *writer, guid + "/pathname", MakeContent( "Assets/Big/Asset" + CORE::UInt32ToString( i ) + ".bin" ) );
        ok = ok && AddEntry( *writer, guid + "/asset", MakeBinaryContent( BigAssetSize + i, (UInt8) i ) );
        ok = ok && AddEntry( *writer, guid + "/asset.meta", MakeContent( "fileFormatVersion: 2\nguid: " + guid + "\n" ) );
    }
    return writer->Close() && ok;
}

/*-------------------------------------------------------------------------*/

static bool
ReadAll( VFS::CVFS& vfs, const CORE::CString& path, CORE::CDynamicBuffer& content )
{
    VFS::TBasicVfsResourcePtr file = vfs.GetFile( path, "rb", false );
    if ( file.IsNULL() )
        return false;
    CORE::IOAccessPtr access = file->GetAccess();
    if ( access.IsNULL() )
        return false;
    content.Clear();
    return content.Append( *access ) == access->GetSize();
}

/*-------------------------------------------------------------------------*/

static bool
ReadEquals( VFS::CVFS& vfs, const CORE::CString& path, const CORE::CDynamicBuffer& expected )
{
    CORE::CDynamicBuffer content;
    return ReadAll( vfs, path, content ) && content == expected;
}

/*-------------------------------------------------------------------------*/

static bool
HasName( const VFS::CVFS::TStringVector& list, const CORE::CString& name )
{
    for ( VFS::CVFS::TStringVector::const_iterator i = list.begin(); i != list.end(); ++i )
        if ( (*i) == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
LoadPlugin( const CORE::CString& pluginName )
{
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
GetExtension( const CORE::CString& filename )
{
    Int32 dot = filename.HasChar( '.', false );
    if ( dot < 0 )
        return "(none)";
    return filename.CutChars( (UInt32) dot, true ).Lowercase();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnityPackageTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Unity package plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "UnityPackage" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    bool packagePluginLoaded = LoadPlugin( "vfspluginUNITYPACKAGE" );
    bool archivePluginLoaded = LoadPlugin( "vfspluginLIBARCHIVE" );

    CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "vfspluginUNITYPACKAGE_TestApp" );
    CORE::CreateDirs( dir );
    vfs.AddRoot( dir, "UnityPackageTempDir", false, false, "upkgtemp" );

    CORE::CString packagePath = CORE::CombinePath( dir, "basic.unitypackage" );

    GUCEF_TESTFW_TESTCASE( "Test: The plugins load and register the unitypackage archive type" )
        try
        {
            ASSERT_TRUE( packagePluginLoaded );
            ASSERT_TRUE( archivePluginLoaded );

            VFS::CVFS::TAbstractArchiveFactory::TKeySet readerTypes;
            vfs.GetListOfSupportedArchiveTypes( readerTypes );
            ASSERT_TRUE( readerTypes.find( "unitypackage" ) != readerTypes.end() );
            ASSERT_TRUE( WriteTestPackage( vfs, packagePath ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A package mounts as the project tree it contains" )
        try
        {
            ASSERT_TRUE( vfs.MountArchive( "upkg", "upkgtemp/basic.unitypackage", "unitypackage", "upkg", false, false ) );

            // existence, sizes and directories
            ASSERT_TRUE( vfs.FileExists( "upkg/Assets/Models/Hero.fbx" ) );
            ASSERT_TRUE( vfs.FileExists( "upkg/Assets/Models/Hero.fbx.meta" ) );
            ASSERT_TRUE( vfs.FileExists( "upkg/Assets/Models/Hero.fbx.preview.png" ) );
            ASSERT_TRUE( vfs.FileExists( "upkg/Assets/Models/Textures/Skin.png" ) );
            ASSERT_TRUE( vfs.FileExists( "upkg/Assets/Models/Textures/Skin.png.meta" ) );
            ASSERT_TRUE( vfs.FileExists( "upkg/Assets/Models.meta" ) );
            ASSERT_TRUE( vfs.FileExists( "upkg/Assets/Prefabs/Deep/Nested/Thing.prefab" ) );
            ASSERT_TRUE( vfs.FileExists( "upkg/Assets/Prefabs/Deep/Nested/Thing.prefab.meta" ) );
            ASSERT_TRUE( vfs.FileExists( "upkg/Assets/Empty.txt" ) );
            ASSERT_FALSE( vfs.FileExists( "upkg/Assets/Models/Hero.fbx.preview.png.meta" ) );
            ASSERT_FALSE( vfs.FileExists( "upkg/Assets/Models/Missing.fbx" ) );
            ASSERT_FALSE( vfs.FileExists( "upkg/Assets/Models" ) );                 // a folder is not a file
            ASSERT_FALSE( vfs.FileExists( "upkg/.icon.png" ) );                     // ignored
            ASSERT_FALSE( vfs.FileExists( CORE::CString( "upkg/" ) + GuidHero + "/asset" ) );  // the package layout is not shown
            ASSERT_FALSE( vfs.FileExists( CORE::CString( "upkg/" ) + GuidHero + "/unknown.bin" ) );

            ASSERT_TRUE( vfs.DirExists( "upkg/Assets" ) );
            ASSERT_TRUE( vfs.DirExists( "upkg/Assets/Models" ) );                   // from a GUID without asset
            ASSERT_TRUE( vfs.DirExists( "upkg/Assets/Models/Textures" ) );          // implied by a pathname
            ASSERT_TRUE( vfs.DirExists( "upkg/Assets/Prefabs/Deep/Nested" ) );
            ASSERT_FALSE( vfs.DirExists( "upkg/Assets/Nothing" ) );
            ASSERT_FALSE( vfs.DirExists( CORE::CString( "upkg/" ) + GuidHero ) );

            ASSERT_TRUE( HeroAsset.GetDataSize() == vfs.GetFileSize( "upkg/Assets/Models/Hero.fbx" ) );
            ASSERT_TRUE( HeroMeta.GetDataSize() == vfs.GetFileSize( "upkg/Assets/Models/Hero.fbx.meta" ) );
            ASSERT_TRUE( HeroPreview.GetDataSize() == vfs.GetFileSize( "upkg/Assets/Models/Hero.fbx.preview.png" ) );
            ASSERT_TRUE( SkinAsset.GetDataSize() == vfs.GetFileSize( "upkg/Assets/Models/Textures/Skin.png" ) );
            ASSERT_TRUE( 0 == vfs.GetFileSize( "upkg/Assets/Empty.txt" ) );

            // listings
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, "upkg/Assets/Models", false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 3 == files.size() );
            ASSERT_TRUE( HasName( files, "Hero.fbx" ) && HasName( files, "Hero.fbx.meta" ) && HasName( files, "Hero.fbx.preview.png" ) );

            files.clear();
            vfs.GetFileList( files, "upkg/Assets", false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 2 == files.size() );       // Empty.txt and Models.meta
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Content of assets, meta files and previews reads back identically" )
        try
        {
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models/Hero.fbx", HeroAsset ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models/Hero.fbx.meta", HeroMeta ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models/Hero.fbx.preview.png", HeroPreview ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models/Textures/Skin.png", SkinAsset ) );     // loaded when opened
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models/Textures/Skin.png.meta", SkinMeta ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models.meta", ModelsMeta ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Prefabs/Deep/Nested/Thing.prefab", PrefabAsset ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Prefabs/Deep/Nested/Thing.prefab.meta", PrefabMeta ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Empty.txt", CORE::CDynamicBuffer() ) );

            // reading again and in another order gives the same
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models/Textures/Skin.png", SkinAsset ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models/Hero.fbx", HeroAsset ) );

            // random access in an entry
            VFS::TBasicVfsResourcePtr skin = vfs.GetFile( "upkg/Assets/Models/Textures/Skin.png", "rb", false );
            ASSERT_FALSE( skin.IsNULL() );
            if ( !skin.IsNULL() )
            {
                CORE::IOAccessPtr access = skin->GetAccess();
                UInt8 chunk[ 4 ];
                ASSERT_TRUE( 0 == access->Seek( 90000, SEEK_SET ) && 4 == access->Read( chunk, 1, 4 ) );
                ASSERT_TRUE( 0 == memcmp( chunk, SkinAsset.GetConstBufferPtr( 90000 ), 4 ) );
                ASSERT_TRUE( 0 == access->Seek( 10, SEEK_SET ) && 4 == access->Read( chunk, 1, 4 ) );
                ASSERT_TRUE( 0 == memcmp( chunk, SkinAsset.GetConstBufferPtr( 10 ), 4 ) );
            }

            // the hash is that of the content
            ASSERT_FALSE( vfs.GetFileHash( "upkg/Assets/Models/Hero.fbx" ).IsNULLOrEmpty() );
            ASSERT_TRUE( vfs.GetFileHash( "upkg/Assets/Models/Hero.fbx" ) != vfs.GetFileHash( "upkg/Assets/Models/Hero.fbx.meta" ) );

            // a missing file does not open
            ASSERT_TRUE( vfs.GetFile( "upkg/Assets/Models/Missing.fbx", "rb", false ).IsNULL() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Directory and recursive listings of the project tree" )
        try
        {
            VFS::CVFS::TStringVector dirs;
            vfs.GetDirList( dirs, "upkg/Assets", false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 2 == dirs.size() );
            ASSERT_TRUE( HasName( dirs, "Models/" ) && HasName( dirs, "Prefabs/" ) );

            dirs.clear();
            vfs.GetDirList( dirs, "upkg", true, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( HasName( dirs, "Assets/Models/Textures/" ) );
            ASSERT_TRUE( HasName( dirs, "Assets/Prefabs/Deep/Nested/" ) );

            // 3 for the hero, 2 for the skin, 1 for the models folder, 2 for the prefab, 1 empty file
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, "upkg", true, true, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 9 == files.size() );
            ASSERT_TRUE( HasName( files, "/upkg/Assets/Models/Hero.fbx" ) );

            files.clear();
            vfs.GetFileList( files, "upkg", true, false, CORE::CString( "*.meta" ) );
            ASSERT_TRUE( 4 == files.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Writing to a mounted package is refused" )
        try
        {
            ASSERT_TRUE( vfs.GetFile( "upkg/Assets/Models/Hero.fbx", "wb", true ).IsNULL() );
            ASSERT_TRUE( vfs.GetFile( "upkg/Assets/Models/Hero.fbx", "r+b", false ).IsNULL() );
            ASSERT_TRUE( vfs.GetFile( "upkg/Assets/New.txt", "wb", true ).IsNULL() );

            CORE::CDynamicBuffer data = MakeContent( "new data" );
            ASSERT_FALSE( vfs.StoreAsFile( "upkg/Assets/New.txt", data, 0, true ) );
            ASSERT_FALSE( vfs.StoreAsFile( "upkg/Assets/Models/Hero.fbx", data, 0, true ) );
            ASSERT_FALSE( vfs.DeleteFile( "upkg/Assets/Models/Hero.fbx", false ) );
            ASSERT_FALSE( vfs.MoveFile( "upkg/Assets/Models/Hero.fbx", "upkg/Assets/Models/Hero2.fbx", true ) );
            ASSERT_FALSE( vfs.FileExists( "upkg/Assets/New.txt" ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models/Hero.fbx", HeroAsset ) );

            // and the package cannot be mounted for writing
            ASSERT_FALSE( vfs.MountArchive( "upkgwrite", "upkgtemp/basic.unitypackage", "unitypackage", "upkgwrite", true, false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Files that are not packages do not mount" )
        try
        {
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "notapackage.unitypackage" ), "this is not a gzip compressed tar" ) );
            ASSERT_FALSE( vfs.MountArchive( "upkgbad", "upkgtemp/notapackage.unitypackage", "unitypackage", "upkgbad", false, false ) );
            ASSERT_FALSE( vfs.MountArchive( "upkgmissing", "upkgtemp/doesnotexist.unitypackage", "unitypackage", "upkgmissing", false, false ) );

            // a valid tar without anything of the Unity package layout
            CORE::CString plainPath = CORE::CombinePath( dir, "plain.unitypackage" );
            {
                CORE::CFileAccess output( plainPath, "wb" );
                VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "tgz" );
                ASSERT_FALSE( writer.IsNULL() );
                if ( !writer.IsNULL() )
                {
                    ASSERT_TRUE( writer->Open( output ) );
                    ASSERT_TRUE( AddEntry( *writer, "readme.txt", MakeContent( "hello" ) ) );
                    ASSERT_TRUE( writer->Close() );
                }
            }
            ASSERT_FALSE( vfs.MountArchive( "upkgplain", "upkgtemp/plain.unitypackage", "unitypackage", "upkgplain", false, false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A package inside a mounted zip archive mounts and reads through both levels" )
        try
        {
            // the zip holds the package and another file
            CORE::CString zipPath = CORE::CombinePath( dir, "outer.zip" );
            {
                CORE::CFileAccess output( zipPath, "wb" );
                VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
                ASSERT_FALSE( writer.IsNULL() );
                if ( !writer.IsNULL() )
                {
                    ASSERT_TRUE( writer->Open( output ) );
                    ASSERT_TRUE( AddEntry( *writer, "readme.txt", MakeContent( "outer" ) ) );
                    VFS::CArchiveWriterEntrySettings settings;
                    ASSERT_TRUE( writer->AddFileFromPath( "packs/basic.unitypackage", packagePath, settings ) );
                    ASSERT_TRUE( writer->Close() );
                }
            }

            ASSERT_TRUE( vfs.MountArchive( "upkgouter", "upkgtemp/outer.zip", "zip", "upkgouter", false, false ) );
            ASSERT_TRUE( vfs.FileExists( "upkgouter/packs/basic.unitypackage" ) );

            VFS::TBasicVfsResourcePtr packageResource = vfs.GetFile( "upkgouter/packs/basic.unitypackage", "rb", false );
            ASSERT_FALSE( packageResource.IsNULL() );
            ASSERT_TRUE( vfs.MountArchive( "upkginner", packageResource, "unitypackage", "upkginner", false ) );
            packageResource = VFS::TBasicVfsResourcePtr();

            ASSERT_TRUE( vfs.FileExists( "upkginner/Assets/Models/Hero.fbx" ) );
            ASSERT_TRUE( vfs.DirExists( "upkginner/Assets/Models/Textures" ) );
            ASSERT_TRUE( HeroAsset.GetDataSize() == vfs.GetFileSize( "upkginner/Assets/Models/Hero.fbx" ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkginner/Assets/Models/Hero.fbx", HeroAsset ) );                 // kept in memory
            ASSERT_TRUE( ReadEquals( vfs, "upkginner/Assets/Models/Hero.fbx.meta", HeroMeta ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkginner/Assets/Models/Textures/Skin.png", SkinAsset ) );        // read through the zip
            ASSERT_TRUE( ReadEquals( vfs, "upkginner/Assets/Prefabs/Deep/Nested/Thing.prefab", PrefabAsset ) );

            // the same through the plain mount of the package
            ASSERT_TRUE( ReadEquals( vfs, "upkg/Assets/Models/Textures/Skin.png", SkinAsset ) );

            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, "upkginner", true, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 9 == files.size() );

            // writes remain refused
            ASSERT_TRUE( vfs.GetFile( "upkginner/Assets/Models/Hero.fbx", "wb", true ).IsNULL() );
            CORE::CDynamicBuffer data = MakeContent( "new data" );
            ASSERT_FALSE( vfs.StoreAsFile( "upkginner/Assets/New.txt", data, 0, true ) );

            ASSERT_TRUE( vfs.UnmountArchiveByName( "upkginner" ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "upkgouter" ) );

            // a package cannot be mounted for writing from a resource either
            VFS::TBasicVfsResourcePtr localResource = vfs.GetFile( "upkgtemp/basic.unitypackage", "rb", false );
            ASSERT_FALSE( localResource.IsNULL() );
            ASSERT_FALSE( vfs.MountArchive( "upkgwriteres", localResource, "unitypackage", "upkgwriteres", true ) );
            ASSERT_TRUE( vfs.MountArchive( "upkgres", localResource, "unitypackage", "upkgres", false ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkgres/Assets/Models/Textures/Skin.png", SkinAsset ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "upkgres" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An entry too large for memory is streamed with seeking" )
        try
        {
            // larger than what is loaded into memory when opened, the pattern compresses to next to nothing
            const UInt64 entrySize = 256ULL * 1024 * 1024 + 4096;
            CORE::CString largePath = CORE::CombinePath( dir, "large.unitypackage" );
            {
                CORE::CFileAccess output( largePath, "wb" );
                VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "tgz" );
                ASSERT_FALSE( writer.IsNULL() );
                if ( !writer.IsNULL() )
                {
                    ASSERT_TRUE( writer->Open( output ) );
                    ASSERT_TRUE( AddEntry( *writer, CORE::CString( GuidHero ) + "/pathname", MakeContent( "Assets/Big.bin\n" ) ) );
                    CPatternSource source( entrySize );
                    VFS::CArchiveWriterEntrySettings settings;
                    ASSERT_TRUE( writer->AddFile( CORE::CString( GuidHero ) + "/asset", source, settings ) );
                    ASSERT_TRUE( AddEntry( *writer, CORE::CString( GuidHero ) + "/asset.meta", HeroMeta ) );
                    ASSERT_TRUE( writer->Close() );
                }
            }

            ASSERT_TRUE( vfs.MountArchive( "upkglarge", "upkgtemp/large.unitypackage", "unitypackage", "upkglarge", false, false ) );
            ASSERT_TRUE( entrySize == vfs.GetFileSize( "upkglarge/Assets/Big.bin" ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkglarge/Assets/Big.bin.meta", HeroMeta ) );

            VFS::TBasicVfsResourcePtr big = vfs.GetFile( "upkglarge/Assets/Big.bin", "rb", false );
            ASSERT_FALSE( big.IsNULL() );
            if ( !big.IsNULL() )
            {
                CORE::IOAccessPtr access = big->GetAccess();
                ASSERT_TRUE( entrySize == access->GetSize() );

                UInt8 chunk[ 16 ];
                ASSERT_TRUE( 16 == access->Read( chunk, 1, 16 ) );
                bool matches = true;
                for ( UInt32 i=0; i<16; ++i )
                    matches = matches && chunk[ i ] == CPatternSource::PatternByte( i );
                ASSERT_TRUE( matches );

                // forward, which skips data
                const UInt64 farPosition = 200ULL * 1000 * 1000;
                ASSERT_TRUE( 0 == access->Seek( (Int64) farPosition, SEEK_SET ) );
                ASSERT_TRUE( farPosition == access->Tell() );
                ASSERT_TRUE( 16 == access->Read( chunk, 1, 16 ) );
                matches = true;
                for ( UInt32 i=0; i<16; ++i )
                    matches = matches && chunk[ i ] == CPatternSource::PatternByte( farPosition + i );
                ASSERT_TRUE( matches );

                // backward, which starts over
                ASSERT_TRUE( 0 == access->Seek( 1000, SEEK_SET ) );
                ASSERT_TRUE( 16 == access->Read( chunk, 1, 16 ) );
                matches = true;
                for ( UInt32 i=0; i<16; ++i )
                    matches = matches && chunk[ i ] == CPatternSource::PatternByte( 1000 + i );
                ASSERT_TRUE( matches );

                // from the end, and out of range
                ASSERT_TRUE( 0 == access->Seek( -16, SEEK_END ) );
                ASSERT_TRUE( 16 == access->Read( chunk, 1, 16 ) );
                ASSERT_TRUE( access->Eof() );
                ASSERT_TRUE( 0 == access->Read( chunk, 1, 16 ) );
                ASSERT_FALSE( 0 == access->Seek( 1, SEEK_END ) );
            }
            big = VFS::TBasicVfsResourcePtr();

            ASSERT_TRUE( vfs.UnmountArchiveByName( "upkglarge" ) );
            CORE::DeleteFile( largePath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Unmounting a package that has been used" )
        try
        {
            ASSERT_TRUE( vfs.UnmountArchiveByName( "upkg" ) );
            ASSERT_FALSE( vfs.FileExists( "upkg/Assets/Models/Hero.fbx" ) );
            ASSERT_FALSE( vfs.DirExists( "upkg/Assets" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A real package given by GUCEF_TEST_UNITYPACKAGE mounts, lists and reads" )
        try
        {
            const char* realPath = CORE::GUCEFGetEnv( "GUCEF_TEST_UNITYPACKAGE" );
            if ( GUCEF_NULL == realPath || '\0' == realPath[ 0 ] )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackage test: GUCEF_TEST_UNITYPACKAGE is not set, skipping the test with a real package" );
            }
            else
            {
                // Mounted directly from where it is, read only, nothing is copied
                CORE::CString realFile = realPath;
                CORE::CString realDir = CORE::StripFilename( realFile );
                CORE::CString realName = CORE::ExtractFilename( realFile );
                ASSERT_TRUE( vfs.AddRoot( realDir, "UnityPackageRealDir", false, false, "upkgrealdir" ) );
                ASSERT_TRUE( vfs.MountArchive( "upkgreal", "upkgrealdir/" + realName, "unitypackage", "upkgreal", false, false ) );

                VFS::CVFS::TStringVector files;
                vfs.GetFileList( files, "upkgreal", true, true, CORE::CString::EmptyStringSet, 1000000 );
                VFS::CVFS::TStringVector dirs;
                vfs.GetDirList( dirs, "upkgreal", true, false, CORE::CString::EmptyStringSet, 1000000 );

                GUCEF::map< CORE::CString, UInt32 > countsByExtension;
                CORE::CString firstFbx;
                CORE::CString firstMeta;
                for ( VFS::CVFS::TStringVector::const_iterator i = files.begin(); i != files.end(); ++i )
                {
                    CORE::CString extension = GetExtension( CORE::ExtractFilename( (*i) ) );
                    ++countsByExtension[ extension ];
                    if ( firstFbx.IsNULLOrEmpty() && extension == ".fbx" )
                        firstFbx = (*i);
                    if ( firstMeta.IsNULLOrEmpty() && extension == ".meta" )
                        firstMeta = (*i);
                }

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackage real package: " << (UInt32) files.size() << " files in " << (UInt32) dirs.size() << " directories" );
                for ( GUCEF::map< CORE::CString, UInt32 >::const_iterator i = countsByExtension.begin(); i != countsByExtension.end(); ++i )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackage real package: extension " << (*i).first << " count " << (*i).second );

                ASSERT_TRUE( files.size() > 0 );
                ASSERT_TRUE( dirs.size() > 0 );

                if ( !firstMeta.IsNULLOrEmpty() )
                {
                    CORE::CDynamicBuffer content;
                    ASSERT_TRUE( ReadAll( vfs, firstMeta, content ) );
                    ASSERT_TRUE( content.GetDataSize() == vfs.GetFileSize( firstMeta ) );
                    ASSERT_TRUE( content.GetDataSize() > 16 && 0 == memcmp( content.GetConstBufferPtr(), "fileFormatVersion", 17 ) );
                }
                if ( !firstFbx.IsNULLOrEmpty() )
                {
                    CORE::CDynamicBuffer content;
                    ASSERT_TRUE( ReadAll( vfs, firstFbx, content ) );
                    ASSERT_TRUE( content.GetDataSize() == vfs.GetFileSize( firstFbx ) );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackage real package: read " << firstFbx << " with " << content.GetDataSize() << " bytes" );
                }

                ASSERT_TRUE( vfs.UnmountArchiveByName( "upkgreal" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Entries over the in-memory limit are served without a pass over the package" )
        try
        {
            CORE::CString bigPath = CORE::CombinePath( dir, "big.unitypackage" );
            ASSERT_TRUE( WriteBigPackage( vfs, bigPath ) );
            ASSERT_TRUE( vfs.MountArchive( "upkgbig", "upkgtemp/big.unitypackage", "unitypackage", "upkgbig", false, false ) );

            // none needs a pass over the package, whatever the order of the reads: the metadata says how an entry is served
            for ( UInt32 i=0; i<BigAssetCount; ++i )
            {
                CORE::CResourceMetaData metaData;
                const CORE::CString file = "upkgbig/Assets/Big/Asset" + CORE::UInt32ToString( i ) + ".bin";
                ASSERT_TRUE( vfs.GetFileMetaData( file, metaData ) );
                ASSERT_TRUE( metaData.hasIsOffline );
                ASSERT_FALSE( metaData.isOffline );
                ASSERT_TRUE( BigAssetSize + i == metaData.resourceSizeInBytes );
                ASSERT_TRUE( vfs.GetFileMetaData( file + ".meta", metaData ) );
                ASSERT_FALSE( metaData.isOffline );
            }

            // the content is what was written, read in reverse and again in the order of the package
            for ( UInt32 i=BigAssetCount; i>0; --i )
                ASSERT_TRUE( ReadEquals( vfs, "upkgbig/Assets/Big/Asset" + CORE::UInt32ToString( i - 1 ) + ".bin", MakeBinaryContent( BigAssetSize + i - 1, (UInt8) ( i - 1 ) ) ) );
            for ( UInt32 i=0; i<BigAssetCount; ++i )
                ASSERT_TRUE( ReadEquals( vfs, "upkgbig/Assets/Big/Asset" + CORE::UInt32ToString( i ) + ".bin", MakeBinaryContent( BigAssetSize + i, (UInt8) i ) ) );

            // random access in an entry that is served from the spill file; the handles are released before the unmount
            {
            VFS::TBasicVfsResourcePtr entry = vfs.GetFile( "upkgbig/Assets/Big/Asset7.bin", "rb", false );
            ASSERT_FALSE( entry.IsNULL() );
            if ( !entry.IsNULL() )
            {
                const CORE::CDynamicBuffer expected = MakeBinaryContent( BigAssetSize + 7, 7 );
                CORE::IOAccessPtr access = entry->GetAccess();
                UInt8 chunk[ 8 ];
                ASSERT_TRUE( 0 == access->Seek( 150000, SEEK_SET ) && 8 == access->Read( chunk, 1, 8 ) );
                ASSERT_TRUE( 0 == memcmp( chunk, expected.GetConstBufferPtr( 150000 ), 8 ) );
                ASSERT_TRUE( 0 == access->Seek( 3, SEEK_SET ) && 8 == access->Read( chunk, 1, 8 ) );
                ASSERT_TRUE( 0 == memcmp( chunk, expected.GetConstBufferPtr( 3 ), 8 ) );
            }

            // several readers at once
            VFS::TBasicVfsResourcePtr first = vfs.GetFile( "upkgbig/Assets/Big/Asset1.bin", "rb", false );
            VFS::TBasicVfsResourcePtr second = vfs.GetFile( "upkgbig/Assets/Big/Asset2.bin", "rb", false );
            ASSERT_FALSE( first.IsNULL() );
            ASSERT_FALSE( second.IsNULL() );
            if ( !first.IsNULL() && !second.IsNULL() )
            {
                UInt8 a[ 4 ];
                UInt8 b[ 4 ];
                ASSERT_TRUE( 4 == first->GetAccess()->Read( a, 1, 4 ) && 4 == second->GetAccess()->Read( b, 1, 4 ) );
                ASSERT_TRUE( 0 == memcmp( a, MakeBinaryContent( 8, 1 ).GetConstBufferPtr(), 4 ) );
                ASSERT_TRUE( 0 == memcmp( b, MakeBinaryContent( 8, 2 ).GetConstBufferPtr(), 4 ) );
            }
            }

            ASSERT_TRUE( vfs.UnmountArchiveByName( "upkgbig" ) );
            ASSERT_FALSE( vfs.FileExists( "upkgbig/Assets/Big/Asset7.bin" ) );

            // mounting again after the unmount works the same: the spill file of the first mount is gone, the new one is its own
            ASSERT_TRUE( vfs.MountArchive( "upkgbig", "upkgtemp/big.unitypackage", "unitypackage", "upkgbig", false, false ) );
            ASSERT_TRUE( ReadEquals( vfs, "upkgbig/Assets/Big/Asset23.bin", MakeBinaryContent( BigAssetSize + 23, 23 ) ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "upkgbig" ) );
            CORE::DeleteFile( bigPath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::DeleteFile( packagePath );
    CORE::DeleteFile( CORE::CombinePath( dir, "outer.zip" ) );
    CORE::DeleteFile( CORE::CombinePath( dir, "notapackage.unitypackage" ) );
    CORE::DeleteFile( CORE::CombinePath( dir, "plain.unitypackage" ) );
}

/*-------------------------------------------------------------------------*/
