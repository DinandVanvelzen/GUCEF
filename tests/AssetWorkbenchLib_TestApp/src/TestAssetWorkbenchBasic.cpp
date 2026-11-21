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

#include <math.h>
#include <string.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_VFS_CIARCHIVEWRITER_H
#include "gucefVFS_CIArchiveWriter.h"
#define GUCEF_VFS_CIARCHIVEWRITER_H
#endif /* GUCEF_VFS_CIARCHIVEWRITER_H ? */

#include "AssetWorkbenchLib.h"
#include "AssetWorkbenchLib_CBlockDecoder.h"
#include "AssetWorkbenchLib_CSceneTreeFileSystemAccess.h"
#include "AssetWorkbenchLib_CReportLogger.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"
#include "TestAssetWorkbenchBasic.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

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

namespace {

bool
Near( Float32 a, Float32 b, Float32 tolerance = 1e-3f )
{
    return fabsf( a - b ) <= tolerance;
}

Float32
Distance( const COrbitController::SVector3& a, const COrbitController::SVector3& b )
{
    const Float32 dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return sqrtf( dx*dx + dy*dy + dz*dz );
}

bool
AddEntry( VFS::CIArchiveWriter& writer, const CORE::CString& name, const CORE::CString& text )
{
    CORE::CDynamicBuffer content;
    content.CopyFrom( text.Length(), text.C_String() );
    CORE::CDynamicBufferAccess access( content );
    VFS::CArchiveWriterEntrySettings settings;
    return writer.AddFile( name, access, settings );
}

bool
HasName( const CORE::CFileSystemEntryListIterator::TEntryVector& entries, const CORE::CString& name, bool directory )
{
    for ( size_t i=0; i<entries.size(); ++i )
        if ( entries[ i ].name == name && entries[ i ].isDirectory == directory )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------*/

static void
TestResolver( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Resolver: the kind of a file comes from its extension, in any case, and the type names are those of the browser" )
    try
    {
        ASSERT_TRUE( ASSETKIND_MODEL == CAssetTypeResolver::GetKind( "/a/b/House.fbx" ) );
        ASSERT_TRUE( ASSETKIND_MODEL == CAssetTypeResolver::GetKind( "x.MESH" ) );
        ASSERT_TRUE( ASSETKIND_MODEL == CAssetTypeResolver::GetKind( "x.gltf" ) && ASSETKIND_MODEL == CAssetTypeResolver::GetKind( "x.glb" ) && ASSETKIND_MODEL == CAssetTypeResolver::GetKind( "x.obj" ) );
        // a source picture is an Image, GPU-ready data a Texture
        ASSERT_TRUE( ASSETKIND_IMAGE == CAssetTypeResolver::GetKind( "tex.PNG" ) && ASSETKIND_IMAGE == CAssetTypeResolver::GetKind( "t.jpg" ) && ASSETKIND_IMAGE == CAssetTypeResolver::GetKind( "t.hdr" ) && ASSETKIND_IMAGE == CAssetTypeResolver::GetKind( "t.exr" ) );
        ASSERT_TRUE( ASSETKIND_TEXTURE == CAssetTypeResolver::GetKind( "t.dds" ) && ASSETKIND_TEXTURE == CAssetTypeResolver::GetKind( "t.ktx" ) && ASSETKIND_TEXTURE == CAssetTypeResolver::GetKind( "t.KTX2" ) && ASSETKIND_TEXTURE == CAssetTypeResolver::GetKind( "t.astc" ) && ASSETKIND_TEXTURE == CAssetTypeResolver::GetKind( "t.basis" ) );
        ASSERT_TRUE( ASSETKIND_AUDIO == CAssetTypeResolver::GetKind( "s.ogg" ) && ASSETKIND_FONT == CAssetTypeResolver::GetKind( "f.ttf" ) && ASSETKIND_DOCUMENT == CAssetTypeResolver::GetKind( "readme.txt" ) && ASSETKIND_PACKAGE == CAssetTypeResolver::GetKind( "p.unitypackage" ) );
        ASSERT_TRUE( ASSETKIND_METADATA == CAssetTypeResolver::GetKind( "a.png.meta" ) && CAssetTypeResolver::IsMetadata( "a.fbx.import" ) && !CAssetTypeResolver::IsMetadata( "a.fbx" ) );
        ASSERT_TRUE( ASSETKIND_MATERIAL == CAssetTypeResolver::GetKind( "m.material" ) );
        ASSERT_TRUE( ASSETKIND_SCENE == CAssetTypeResolver::GetKind( "level.scene" ) );
        ASSERT_TRUE( ASSETKIND_SKELETON == CAssetTypeResolver::GetKind( "rig.skeleton" ) );
        ASSERT_TRUE( ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "readme.unknownext" ) && ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "noextension" ) && ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "folder.fbx/inside" ) );
        ASSERT_TRUE( ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "dot." ) && ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "" ) );

        CAssetTypeResolver resolver;
        ASSERT_TRUE( "Model" == resolver.GetTypeName( "a.fbx", false ) && "Image" == resolver.GetTypeName( "a.png", false ) && "Texture" == resolver.GetTypeName( "a.dds", false ) );
        ASSERT_TRUE( "Material" == resolver.GetTypeName( "a.material", false ) && "Scene" == resolver.GetTypeName( "a.scene", false ) );
        ASSERT_TRUE( "Skeleton" == resolver.GetTypeName( "a.skeleton", false ) && "Other" == resolver.GetTypeName( "a.bin", false ) );
        ASSERT_TRUE( "Audio" == resolver.GetTypeName( "a.wav", false ) && "Package" == resolver.GetTypeName( "a.zip", false ) && "Metadata" == resolver.GetTypeName( "a.png.meta", false ) && "Document" == resolver.GetTypeName( "a.md", false ) );
        // only what the views read is opened: a .umap or .mtl has a kind of its own but no view
        ASSERT_TRUE( CAssetTypeResolver::HasView( "a.fbx" ) && CAssetTypeResolver::HasView( "a.png" ) && CAssetTypeResolver::HasView( "a.dds" ) && CAssetTypeResolver::HasView( "a.material" ) && CAssetTypeResolver::HasView( "a.scene" ) );
        ASSERT_TRUE( !CAssetTypeResolver::HasView( "a.umap" ) && !CAssetTypeResolver::HasView( "a.mtl" ) && !CAssetTypeResolver::HasView( "a.wav" ) && !CAssetTypeResolver::HasView( "a.meta" ) );
        ASSERT_TRUE( resolver.IsSidecar( "a.png.meta", false ) && !resolver.IsSidecar( "a.png", false ) && !resolver.IsSidecar( "meta", true ) );
        CORE::CString::StringVector imageExtensions, textureExtensions;
        CAssetTypeResolver::GetExtensions( ASSETKIND_IMAGE, imageExtensions );
        CAssetTypeResolver::GetExtensions( ASSETKIND_TEXTURE, textureExtensions );
        ASSERT_TRUE( imageExtensions.size() >= 12 && 7 == textureExtensions.size() );
        ASSERT_TRUE( "Folder" == resolver.GetTypeName( "models", true ) );
        ASSERT_TRUE( "png" == CAssetTypeResolver::GetExtension( "dir.x/a.PNG" ) && CAssetTypeResolver::GetExtension( "dir.x/a" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestOrbitController( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Orbit controller: framing the bounds puts the whole sphere in view for every aspect ratio, and the distance is the one of the field of view" )
    try
    {
        const Float32 aspects[ 5 ] = { 0.4f, 0.75f, 1.0f, 1.78f, 3.0f };
        for ( UInt32 a=0; a<5; ++a )
        {
            COrbitController controller;
            controller.SetFieldOfView( 45.0f );
            controller.SetAspectRatio( aspects[ a ] );
            const COrbitController::SVector3 center( 3.0f, 2.0f, -1.0f );
            controller.SetBounds( center, 5.0f );
            ASSERT_TRUE( controller.IsSphereInView( center, 5.0f ) );
            // the view looks at the center, from the distance of the narrower field of view
            ASSERT_TRUE( Near( Distance( controller.GetEye(), controller.GetTarget() ), controller.GetDistance(), 1e-2f ) );
            ASSERT_TRUE( Near( controller.GetDistance(), controller.GetFramingDistance( 5.0f ), 1e-3f ) );
            // closer in, the sphere does not fit any more
            controller.SetDistance( controller.GetDistance() * 0.5f );
            ASSERT_FALSE( controller.IsSphereInView( center, 5.0f ) );
            // the same for the other end: framing again after an orbit
            controller.SetAngles( 1.0f, -0.3f );
            controller.Frame();
            ASSERT_TRUE( controller.IsSphereInView( center, 5.0f ) );
        }

        COrbitController controller;
        controller.SetFieldOfView( 45.0f );
        controller.SetAspectRatio( 1.0f );
        controller.SetBounds( COrbitController::SVector3(), 1.0f );
        const Float32 half = 0.5f * 45.0f * 3.14159265f / 180.0f;
        ASSERT_TRUE( Near( controller.GetFramingDistance( 1.0f ), 1.12f / sinf( half ), 1e-3f ) );
        // the clip planes enclose the sphere
        ASSERT_TRUE( controller.GetNearClipDistance() < controller.GetDistance() - 1.0f && controller.GetFarClipDistance() > controller.GetDistance() + 1.0f );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Orbit controller: the left button orbits (the pitch stops short of the poles), shift and the right button pan, the wheel zooms, the keys frame and pick the views" )
    try
    {
        COrbitController controller;
        controller.OnSurfaceResized( 400, 300 );
        controller.SetBounds( COrbitController::SVector3( 0.0f, 0.0f, 0.0f ), 2.0f );
        ASSERT_TRUE( controller.ConsumeChanged() );
        ASSERT_FALSE( controller.ConsumeChanged() );
        const Float32 yaw = controller.GetYaw();
        const Float32 pitch = controller.GetPitch();
        const Float32 distance = controller.GetDistance();

        // a drag with the left button turns the view and keeps the distance
        const UInt32 left = 1U << GUI::MOUSEBUTTON_LEFT;
        controller.OnPointerButton( 100, 100, GUI::MOUSEBUTTON_LEFT, true, 1, 0 );
        controller.OnPointerMove( 140, 100, left, 0 );
        ASSERT_TRUE( controller.ConsumeChanged() );
        ASSERT_TRUE( controller.GetYaw() < yaw && Near( controller.GetPitch(), pitch ) );
        controller.OnPointerMove( 140, 130, left, 0 );
        ASSERT_TRUE( controller.GetPitch() > pitch );
        ASSERT_TRUE( Near( controller.GetDistance(), distance ) );
        // far past the pole is the pole
        controller.OnPointerMove( 140, 100000, left, 0 );
        ASSERT_TRUE( controller.GetPitch() < 1.5708f && controller.GetPitch() > 1.5f );
        controller.OnPointerMove( 140, -100000, left, 0 );
        ASSERT_TRUE( controller.GetPitch() > -1.5708f && controller.GetPitch() < -1.5f );
        // the move of a pointer that is not dragging does nothing
        controller.OnPointerButton( 140, 100, GUI::MOUSEBUTTON_LEFT, false, 1, 0 );
        controller.ConsumeChanged();
        controller.OnPointerMove( 10, 10, 0, 0 );
        ASSERT_FALSE( controller.ConsumeChanged() );

        // panning moves the target in the plane of the view and leaves the angles and the distance
        controller.Frame();
        controller.SetAngles( 0.0f, 0.0f );
        COrbitController::SVector3 forward, right, up;
        controller.GetBasis( forward, right, up );
        const COrbitController::SVector3 before = controller.GetTarget();
        controller.OnPointerButton( 100, 100, GUI::MOUSEBUTTON_LEFT, true, 1, 0 );
        controller.OnPointerMove( 150, 100, left, INPUT::KEYMOD_SHIFT );
        const COrbitController::SVector3 after = controller.GetTarget();
        const Float32 moved = Distance( before, after );
        ASSERT_TRUE( moved > 0.01f );
        ASSERT_TRUE( Near( ( after.x - before.x ) * forward.x + ( after.y - before.y ) * forward.y + ( after.z - before.z ) * forward.z, 0.0f ) );
        ASSERT_TRUE( ( after.x - before.x ) * right.x < 0.0f );    // dragging right moves the scene with the pointer: the target goes left
        ASSERT_TRUE( Near( controller.GetYaw(), 0.0f ) );

        // the wheel moves in and out, never through the target
        controller.Frame();
        const Float32 framed = controller.GetDistance();
        controller.OnWheel( 0, 0, 1, 0 );
        ASSERT_TRUE( Near( controller.GetDistance(), framed * 0.88f, 1e-3f ) );
        controller.OnWheel( 0, 0, -2, 0 );
        ASSERT_TRUE( controller.GetDistance() > framed );
        controller.OnWheel( 0, 0, 500, 0 );
        ASSERT_TRUE( controller.GetDistance() >= controller.GetBoundsRadius() * 0.02f * 0.999f );

        // keys: 3 looks down, F frames again
        controller.OnKey( INPUT::KEYCODE_3, true, 0 );
        ASSERT_TRUE( controller.GetPitch() > 1.5f );
        controller.OnKey( INPUT::KEYCODE_F, true, 0 );
        ASSERT_TRUE( Near( controller.GetDistance(), controller.GetFramingDistance( controller.GetBoundsRadius() ), 1e-3f ) );
        controller.OnKey( INPUT::KEYCODE_1, true, 0 );
        ASSERT_TRUE( Near( controller.GetPitch(), 0.0f ) && Near( controller.GetYaw(), 0.0f ) );
        controller.OnKey( INPUT::KEYCODE_2, true, 0 );
        ASSERT_TRUE( Near( controller.GetYaw(), 1.5707963f, 1e-3f ) );
        COrbitController::SVector3 eye = controller.GetEye();
        ASSERT_TRUE( eye.x > 1.0f && Near( eye.z, 0.0f, 1e-3f ) );    // the side view is from +x
        controller.OnKey( INPUT::KEYCODE_1, true, 0 );
        eye = controller.GetEye();
        ASSERT_TRUE( eye.z > 1.0f && Near( eye.x, 0.0f, 1e-3f ) );    // the front view is from +z

        // the size of the surface gives the aspect ratio
        controller.OnSurfaceResized( 800, 200 );
        ASSERT_TRUE( Near( controller.GetAspectRatio(), 4.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestMounts( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Mounts: a spec is a mount point and a source, paths have no first or last slash, archives are told by their extension" )
    try
    {
        CORE::CString mountPoint, source;
        ASSERT_TRUE( CAssetMounter::ParseSpec( "town=/mnt/data/pack.unitypackage", mountPoint, source ) && "town" == mountPoint && "/mnt/data/pack.unitypackage" == source );
        ASSERT_TRUE( CAssetMounter::ParseSpec( "/a/b/=c=d", mountPoint, source ) && "a/b" == mountPoint && "c=d" == source );
        ASSERT_FALSE( CAssetMounter::ParseSpec( "nosource=", mountPoint, source ) );
        ASSERT_FALSE( CAssetMounter::ParseSpec( "=nomount", mountPoint, source ) );
        ASSERT_FALSE( CAssetMounter::ParseSpec( "nothing", mountPoint, source ) );
        ASSERT_TRUE( "a/b/c" == CAssetMounter::NormalizePath( "//a\\b/c//" ) && CAssetMounter::NormalizePath( "/" ).IsNULLOrEmpty() );
        ASSERT_TRUE( "zip" == CAssetMounter::GetArchiveType( "/x/y/pack.ZIP" ) );
        ASSERT_TRUE( CAssetMounter::GetArchiveType( "/x/y/pack.txt" ).IsNULLOrEmpty() && CAssetMounter::GetArchiveType( "/x/y/pack" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mounts: a folder, a zip and a Unity package inside the zip are mounted and found by the longest mount point; bad sources and used mount points are refused" )
    try
    {
        const CORE::CString dir = MakeCleanTestDir( "mounts" );
        ASSERT_TRUE( WriteTestFile( dir + "/folder/sub/a.txt", "folder file" ) );

        // a Unity package with one file (the only archive type that the VFS reads from the entry of another archive), a zip with the package
        // in it and a file
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        {
            CORE::CFileAccess output( dir + "/inner.unitypackage", "wb" );
            VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "tgz" );
            ASSERT_FALSE( writer.IsNULL() );
            ASSERT_TRUE( writer->Open( output ) );
            ASSERT_TRUE( AddEntry( *writer, "0123456789abcdef0123456789abcdef/asset", "inner file!\n" ) );
            ASSERT_TRUE( AddEntry( *writer, "0123456789abcdef0123456789abcdef/pathname", "Assets/Models/b.txt\n" ) );
            ASSERT_TRUE( writer->Close() );
        }
        {
            CORE::CFileAccess output( dir + "/outer.zip", "wb" );
            VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
            ASSERT_FALSE( writer.IsNULL() );
            ASSERT_TRUE( writer->Open( output ) );
            VFS::CArchiveWriterEntrySettings settings;
            ASSERT_TRUE( writer->AddFileFromPath( "packs/inner.unitypackage", dir + "/inner.unitypackage", settings ) );
            ASSERT_TRUE( AddEntry( *writer, "readme.txt", "outer\n" ) );
            ASSERT_TRUE( writer->Close() );
        }

        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avfolder", dir + "/folder" ) );
        ASSERT_TRUE( vfs.FileExists( "avfolder/sub/a.txt" ) );
        ASSERT_TRUE( mounter.Mount( "avouter", dir + "/outer.zip" ) );
        ASSERT_TRUE( vfs.FileExists( "avouter/readme.txt" ) && vfs.FileExists( "avouter/packs/inner.unitypackage" ) );
        // the package in the zip, given by its VFS path
        ASSERT_TRUE( mounter.Mount( "avinner", "avouter/packs/inner.unitypackage" ) );
        ASSERT_TRUE( vfs.FileExists( "avinner/Assets/Models/b.txt" ) );
        ASSERT_TRUE( 3 == mounter.GetMountCount() );
        ASSERT_TRUE( "unitypackage" == mounter.GetMount( 2 ).archiveType && "FileSystem" == mounter.GetMount( 0 ).archiveType );

        // the longest mount point wins, and a path of no mount is none
        CAssetMounter::SMount mount;
        CORE::CString relative;
        ASSERT_TRUE( mounter.FindMount( "/avinner/Assets/Models/b.txt", mount, relative ) && "avinner" == mount.mountPoint && "Assets/Models/b.txt" == relative );
        ASSERT_TRUE( mounter.FindMount( "avouter/packs/inner.unitypackage", mount, relative ) && "avouter" == mount.mountPoint && "packs/inner.unitypackage" == relative );
        ASSERT_TRUE( mounter.FindMount( "avfolder", mount, relative ) && relative.IsNULLOrEmpty() );
        ASSERT_FALSE( mounter.FindMount( "avfolderx/a.txt", mount, relative ) );
        ASSERT_FALSE( mounter.FindMount( "elsewhere/a.txt", mount, relative ) );

        // refused: the mount point is in use, the source is nothing, the file is no archive, the archive is not in a mount
        ASSERT_FALSE( mounter.Mount( "avfolder", dir + "/folder" ) );
        ASSERT_FALSE( mounter.Mount( "avnothing", dir + "/does/not/exist" ) );
        ASSERT_FALSE( mounter.Mount( "avtext", dir + "/folder/sub/a.txt" ) );
        ASSERT_FALSE( mounter.Mount( "avnoarchive", "avouter/readme.txt" ) );
        ASSERT_TRUE( 3 == mounter.GetMountCount() );

        // the specs of a command line
        CORE::CString::StringVector specs;
        specs.push_back( "avsecond=" + dir + "/folder" );
        specs.push_back( "broken" );
        ASSERT_TRUE( 1 == mounter.MountAll( specs ) && 4 == mounter.GetMountCount() );

        mounter.UnmountAll();
        ASSERT_TRUE( 0 == mounter.GetMountCount() );
        ASSERT_FALSE( vfs.FileExists( "avfolder/sub/a.txt" ) || vfs.FileExists( "avinner/Assets/Models/b.txt" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestBlockDecoder( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Block decoder: BC1 and BC3 blocks give their colours and alpha, BC4 and BC5 their channels, pictures that are no multiple of 4 are cut" )
    try
    {
        ASSERT_TRUE( CBlockDecoder::CanDecode( Ogre::PF_DXT1 ) && CBlockDecoder::CanDecode( Ogre::PF_DXT5 ) && CBlockDecoder::CanDecode( Ogre::PF_BC4_UNORM ) && CBlockDecoder::CanDecode( Ogre::PF_BC5_UNORM ) );
        ASSERT_FALSE( CBlockDecoder::CanDecode( Ogre::PF_BC7_UNORM ) || CBlockDecoder::CanDecode( Ogre::PF_ETC2_RGB8 ) || CBlockDecoder::CanDecode( Ogre::PF_BYTE_RGBA ) );
        ASSERT_TRUE( 8 == CBlockDecoder::GetBlocksSize( Ogre::PF_DXT1, 4, 4 ) && 4 * 16 == CBlockDecoder::GetBlocksSize( Ogre::PF_DXT5, 5, 5 ) );

        // white and black with the four ways to mix them
        const UInt8 bc1[ 8 ] = { 0xFF, 0xFF, 0x00, 0x00, 0xE4, 0x00, 0x00, 0x00 };
        UInt8 pixels[ 4 * 4 * 4 ];
        ASSERT_TRUE( CBlockDecoder::Decode( Ogre::PF_DXT1, bc1, 4, 4, pixels ) );
        ASSERT_TRUE( 255 == pixels[ 0 ] && 255 == pixels[ 1 ] && 255 == pixels[ 2 ] && 255 == pixels[ 3 ] );          // index 0: the first end point
        ASSERT_TRUE( 0 == pixels[ 4 ] && 0 == pixels[ 5 ] && 0 == pixels[ 6 ] && 255 == pixels[ 7 ] );                // index 1: the second
        ASSERT_TRUE( 170 == pixels[ 8 ] && 170 == pixels[ 9 ] && 170 == pixels[ 10 ] );                                // index 2: two thirds of the first
        ASSERT_TRUE( 85 == pixels[ 12 ] && 85 == pixels[ 13 ] && 85 == pixels[ 14 ] );                                 // index 3: one third

        // a block whose first end point is below the second has a transparent fourth colour
        const UInt8 transparent[ 8 ] = { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        ASSERT_TRUE( CBlockDecoder::Decode( Ogre::PF_DXT1, transparent, 4, 4, pixels ) );
        ASSERT_TRUE( 0 == pixels[ 3 ] && 0 == pixels[ 63 ] );

        // BC3: an opaque alpha block and a red colour block
        const UInt8 bc3[ 16 ] = { 0xFF, 0x00, 0, 0, 0, 0, 0, 0,   0x00, 0xF8, 0x00, 0xF8, 0, 0, 0, 0 };
        ASSERT_TRUE( CBlockDecoder::Decode( Ogre::PF_DXT5, bc3, 4, 4, pixels ) );
        ASSERT_TRUE( 255 == pixels[ 0 ] && 0 == pixels[ 1 ] && 0 == pixels[ 2 ] && 255 == pixels[ 3 ] );
        // alpha end points 40 and 200 with the index 1 for the first pixel: the second end point
        const UInt8 bc3Alpha[ 16 ] = { 40, 200, 0x01, 0, 0, 0, 0, 0,   0x00, 0xF8, 0x00, 0xF8, 0, 0, 0, 0 };
        ASSERT_TRUE( CBlockDecoder::Decode( Ogre::PF_DXT5, bc3Alpha, 4, 4, pixels ) );
        ASSERT_TRUE( 200 == pixels[ 3 ] && 40 == pixels[ 7 ] );

        // BC4: one channel as grey; BC5: red and green
        const UInt8 bc4[ 8 ] = { 128, 128, 0, 0, 0, 0, 0, 0 };
        ASSERT_TRUE( CBlockDecoder::Decode( Ogre::PF_BC4_UNORM, bc4, 4, 4, pixels ) );
        ASSERT_TRUE( 128 == pixels[ 0 ] && 128 == pixels[ 1 ] && 128 == pixels[ 2 ] && 255 == pixels[ 3 ] );
        const UInt8 bc5[ 16 ] = { 200, 200, 0, 0, 0, 0, 0, 0,   50, 50, 0, 0, 0, 0, 0, 0 };
        ASSERT_TRUE( CBlockDecoder::Decode( Ogre::PF_BC5_UNORM, bc5, 4, 4, pixels ) );
        ASSERT_TRUE( 200 == pixels[ 0 ] && 50 == pixels[ 1 ] && 0 == pixels[ 2 ] );

        // 5 x 5 pixels are 2 x 2 blocks; the pixels beyond the picture are not written
        UInt8 blocks[ 32 ];
        for ( UInt32 b=0; b<4; ++b )
            memcpy( blocks + b * 8, bc1, 8 );
        UInt8 big[ 5 * 5 * 4 + 4 ];
        memset( big, 0x7A, sizeof( big ) );
        ASSERT_TRUE( CBlockDecoder::Decode( Ogre::PF_DXT1, blocks, 5, 5, big ) );
        ASSERT_TRUE( 255 == big[ ( 4 * 5 + 4 ) * 4 ] );
        ASSERT_TRUE( 0x7A == big[ 5 * 5 * 4 ] );
        ASSERT_FALSE( CBlockDecoder::Decode( Ogre::PF_BC7_UNORM, blocks, 4, 4, big ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSceneTreeFileSystem( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Scene tree: the nodes are folders and the objects files of a file system that cannot be written" )
    try
    {
        CSceneTreeFileSystemAccess* tree = GUCEF_NEW CSceneTreeFileSystemAccess( "Scene" );
        CORE::TFileSystemAccessPtr access( tree );

        CSceneTreeFileSystemAccess::TNodeVector nodes;
        CSceneTreeFileSystemAccess::SNode root;
        root.path = "/";
        nodes.push_back( root );
        CSceneTreeFileSystemAccess::SNode parent;
        parent.path = "/Parent";
        nodes.push_back( parent );
        CSceneTreeFileSystemAccess::SNode child;
        child.path = "/Parent/Child";
        CSceneTreeFileSystemAccess::SObject cube;
        cube.name = "TheCube";
        cube.kind = "entity";
        child.objects.push_back( cube );
        nodes.push_back( child );
        CSceneTreeFileSystemAccess::SNode lamp;
        lamp.path = "/Lamp";
        CSceneTreeFileSystemAccess::SObject light;
        light.name = "LampLight";
        light.kind = "light";
        lamp.objects.push_back( light );
        nodes.push_back( lamp );
        tree->SetTree( nodes );

        ASSERT_TRUE( "/" == access->GetRootPath() && "Scene" == access->GetDisplayName() && !access->IsWriteable( "/Parent" ) );

        CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( "/Parent/Child", type ) && CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( "/Parent/Child/TheCube.entity", type ) && CORE::FILESYSTEMENTRYTYPE_FILE == type );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( "/Lamp/LampLight.light", type ) && CORE::FILESYSTEMENTRYTYPE_FILE == type );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( "/Parent/Nothing", type ) && CORE::FILESYSTEMENTRYTYPE_NONE == type );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( "/", type ) && CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type );

        CORE::TFileSystemIteratorPtr iterator;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIterator( "/", iterator ) );
        CORE::CFileSystemEntryListIterator::TEntryVector entries;
        ASSERT_TRUE( CORE::CFileSystemEntryListIterator::TryReadAll( *iterator, "/", entries ) );
        ASSERT_TRUE( 2 == entries.size() && HasName( entries, "Parent", true ) && HasName( entries, "Lamp", true ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIterator( "/Parent/Child", iterator ) );
        ASSERT_TRUE( CORE::CFileSystemEntryListIterator::TryReadAll( *iterator, "/Parent/Child", entries ) );
        ASSERT_TRUE( 1 == entries.size() && HasName( entries, "TheCube.entity", false ) && "/Parent/Child/TheCube.entity" == entries[ 0 ].path );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryGetFileSystemIterator( "/Nothing", iterator ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY == access->TryGetFileSystemIterator( "/Lamp/LampLight.light", iterator ) );

        // nothing is written
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_READ_ONLY == access->TryCreateDirectory( "/New", false ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_READ_ONLY == access->TryDeleteDirectory( "/Parent", true ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_READ_ONLY == access->TryRenameFile( "/Lamp/LampLight.light", "x" ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_READ_ONLY == access->TryDeleteFile( "/Lamp/LampLight.light" ) );

        CORE::CResourceMetaData metaData;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetMetaData( "/Parent", metaData ) && "Parent" == metaData.name );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryGetMetaData( "/Missing", metaData ) );

        // the helpers
        ASSERT_TRUE( "/A" == CSceneTreeFileSystemAccess::JoinPath( "/", "A" ) && "/A/B" == CSceneTreeFileSystemAccess::JoinPath( "/A", "B" ) );
        ASSERT_TRUE( "/" == CSceneTreeFileSystemAccess::ParentOf( "/A" ) && "/A" == CSceneTreeFileSystemAccess::ParentOf( "/A/B" ) );
        ASSERT_TRUE( "entity" == CSceneTreeFileSystemAccess::GetObjectKind( "x.entity" ) && "light" == CSceneTreeFileSystemAccess::GetObjectKind( "x.light" ) && CSceneTreeFileSystemAccess::GetObjectKind( "x.txt" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Where a point is in the view of the camera, as a part of half the view: x and y between -1 and 1 where it is inside */
static void
Project( const COrbitController& controller, const COrbitController::SVector3& point, Float32 aspect, Float32& x, Float32& y )
{
    const COrbitController::SVector3 eye = controller.GetEye();
    COrbitController::SVector3 forward, right, up;
    controller.GetBasis( forward, right, up );
    const Float32 dx = point.x - eye.x, dy = point.y - eye.y, dz = point.z - eye.z;
    const Float32 depth = dx * forward.x + dy * forward.y + dz * forward.z;
    const Float32 tanVertical = tanf( 0.5f * controller.GetFieldOfView() * 3.14159265f / 180.0f );
    x = ( dx * right.x + dy * right.y + dz * right.z ) / ( depth * tanVertical * aspect );
    y = ( dx * up.x + dy * up.y + dz * up.z ) / ( depth * tanVertical );
}

static void
TestAutoFrame( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Auto frame: a box is framed from the angles of the view so that its corners fill the asked part of the narrower side, for every preset and aspect ratio" )
    try
    {
        const Float32 aspects[ 4 ] = { 0.5f, 1.0f, 1.78f, 2.4f };
        const COrbitController::EViewPreset presets[ 4 ] = { COrbitController::VIEWPRESET_FRONT, COrbitController::VIEWPRESET_SIDE, COrbitController::VIEWPRESET_TOP, COrbitController::VIEWPRESET_THREE_QUARTER };
        // a small, thin and tall flame (a tenth wide, a third high) and a wide flat grid
        const COrbitController::SVector3 boxes[ 2 ][ 2 ] = { { COrbitController::SVector3( -0.05f, 0.0f, -0.05f ), COrbitController::SVector3( 0.05f, 0.33f, 0.05f ) },
                                                              { COrbitController::SVector3( -8.0f, 0.0f, -5.0f ), COrbitController::SVector3( 8.0f, 1.5f, 5.0f ) } };
        for ( UInt32 b=0; b<2; ++b )
        {
            for ( UInt32 a=0; a<4; ++a )
            {
                for ( UInt32 p=0; p<4; ++p )
                {
                    COrbitController controller;
                    controller.SetFieldOfView( 45.0f );
                    controller.SetAspectRatio( aspects[ a ] );
                    controller.SetBoundsBox( boxes[ b ][ 0 ], boxes[ b ][ 1 ], 0.4f, true );
                    ASSERT_TRUE( controller.HasBoundsBox() && controller.IsBoxCentredOnCovered() && Near( controller.GetFillFraction(), 0.4f, 1e-6f ) );
                    controller.ApplyPreset( presets[ p ] );
                    // what the corners cover is in the middle of the view, and reaches the asked part of the view and not past it
                    Float32 lowX = 1e9f, highX = -1e9f, lowY = 1e9f, highY = -1e9f;
                    Float32 x = 0.0f, y = 0.0f;
                    for ( UInt32 c=0; c<8; ++c )
                    {
                        const COrbitController::SVector3 corner( c & 1 ? boxes[ b ][ 1 ].x : boxes[ b ][ 0 ].x, c & 2 ? boxes[ b ][ 1 ].y : boxes[ b ][ 0 ].y, c & 4 ? boxes[ b ][ 1 ].z : boxes[ b ][ 0 ].z );
                        Project( controller, corner, aspects[ a ], x, y );
                        lowX = x < lowX ? x : lowX; highX = x > highX ? x : highX;
                        lowY = y < lowY ? y : lowY; highY = y > highY ? y : highY;
                    }
                    ASSERT_TRUE( Near( lowX + highX, 0.0f, 4e-3f ) && Near( lowY + highY, 0.0f, 4e-3f ) );
                    const Float32 largest = 0.5f * ( highX - lowX > highY - lowY ? highX - lowX : highY - lowY );
                    ASSERT_TRUE( Near( largest, 0.4f, 2e-3f ) );
                }
            }
        }

        // without centring what the box covers the camera looks at the middle of the box, and the corners reach the part asked for
        for ( UInt32 p=0; p<4; ++p )
        {
            COrbitController controller;
            controller.SetFieldOfView( 45.0f );
            controller.SetAspectRatio( 1.78f );
            controller.SetBoundsBox( boxes[ 1 ][ 0 ], boxes[ 1 ][ 1 ], 0.5f );
            ASSERT_TRUE( controller.HasBoundsBox() && !controller.IsBoxCentredOnCovered() );
            controller.ApplyPreset( presets[ p ] );
            Float32 x = 0.0f, y = 0.0f;
            Project( controller, controller.GetBoundsCenter(), 1.78f, x, y );
            ASSERT_TRUE( Near( x, 0.0f, 1e-3f ) && Near( y, 0.0f, 1e-3f ) );
            Float32 largest = 0.0f;
            for ( UInt32 c=0; c<8; ++c )
            {
                const COrbitController::SVector3 corner( c & 1 ? boxes[ 1 ][ 1 ].x : boxes[ 1 ][ 0 ].x, c & 2 ? boxes[ 1 ][ 1 ].y : boxes[ 1 ][ 0 ].y, c & 4 ? boxes[ 1 ][ 1 ].z : boxes[ 1 ][ 0 ].z );
                Project( controller, corner, 1.78f, x, y );
                largest = fabsf( x ) > largest ? fabsf( x ) : largest;
                largest = fabsf( y ) > largest ? fabsf( y ) : largest;
            }
            ASSERT_TRUE( Near( largest, 0.5f, 2e-3f ) );
        }

        // the default is a part of 0.9, a sphere is still a sphere
        COrbitController plain;
        plain.SetBoundsBox( COrbitController::SVector3( -1.0f, -1.0f, -1.0f ), COrbitController::SVector3( 1.0f, 1.0f, 1.0f ) );
        ASSERT_TRUE( Near( plain.GetFillFraction(), 0.9f, 1e-6f ) && Near( plain.GetBoundsRadius(), sqrtf( 3.0f ), 1e-4f ) );
        plain.SetBounds( COrbitController::SVector3(), 2.0f );
        ASSERT_FALSE( plain.HasBoundsBox() );
        ASSERT_TRUE( Near( plain.GetDistance(), plain.GetFramingDistance( 2.0f ), 1e-3f ) );

        // the part of an orthographic view that a box takes: the bigger of the parts of the two sides
        ASSERT_TRUE( Near( COrbitController::GetFilledFraction( 1.0f, 4.0f, 10.0f, 2.0f ), 0.4f, 1e-6f ) );
        ASSERT_TRUE( Near( COrbitController::GetFilledFraction( 10.0f, 1.0f, 10.0f, 2.0f ), 0.5f, 1e-6f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Showcase labels: as high as their cell allows, smaller for a long name and cut where the letters would be too small, standing up, shown only while their letters can be read" )
    try
    {
        // names that fit: a fifth of the cell
        UInt32 kept = 0;
        Float32 height = CShowcaseView::GetLabelHeight( 10.0f, 5, kept );
        ASSERT_TRUE( Near( height, 2.0f, 1e-5f ) && 5 == kept );
        // a name that is too wide gets smaller letters, down to 8 percent of the cell, and then loses its end
        height = CShowcaseView::GetLabelHeight( 10.0f, 10, kept );
        ASSERT_TRUE( 10 == kept && Near( 6.0f * 10.0f * height / 7.0f, 9.5f, 1e-3f ) );
        height = CShowcaseView::GetLabelHeight( 10.0f, 30, kept );
        ASSERT_TRUE( Near( height, 0.8f, 1e-5f ) && kept < 30 && kept >= 13 && 6.0f * (Float32) kept * height / 7.0f <= 9.5f + 1e-3f );
        // the letters in pixels: a view of 900 pixels, a field of view of 45 degrees: letters of 1 unit at 10 units away are 108 pixels high
        const Float32 pixels = CShowcaseView::GetLabelPixelHeight( 1.0f, 10.0f, 45.0f, 900 );
        ASSERT_TRUE( Near( pixels, 1.0f / ( 2.0f * 10.0f * tanf( 22.5f * 3.14159265f / 180.0f ) ) * 900.0f, 1e-2f ) && pixels > 100.0f && pixels < 115.0f );
        ASSERT_TRUE( CShowcaseView::IsLabelReadable( 10.0f ) && !CShowcaseView::IsLabelReadable( 9.9f ) && !CShowcaseView::IsLabelReadable( 0.0f ) );

        // the text standing up has its letters in the plane z, the first row at the top
        CWorkbenchStage::TLineVertexVector ground, upright;
        CShowcaseView::AddTextLines( ground, "I", 0.0f, 0.0f, 7.0f, 1.0f, 1.0f, 1.0f );
        CShowcaseView::AddUprightTextLines( upright, "I", 0.0f, 2.0f, 5.0f, 7.0f, 1.0f, 1.0f, 1.0f );
        ASSERT_TRUE( !ground.empty() && ground.size() == upright.size() );
        bool inPlane = true;
        Float32 lowest = 1e9f, highest = -1e9f;
        for ( size_t i=0; i<upright.size(); ++i )
        {
            inPlane = inPlane && upright[ i ].z == 5.0f;
            if ( upright[ i ].y < lowest ) lowest = upright[ i ].y;
            if ( upright[ i ].y > highest ) highest = upright[ i ].y;
        }
        ASSERT_TRUE( inPlane && lowest >= 2.0f && highest <= 9.0f && highest > 8.0f );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRobustFraming( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Robust framing: a sky dome, a cloud ring and an object far bigger than the rest are in the scene but not in the box that the camera frames; a scene of only such objects is framed as it is" )
    try
    {
        GUCEF::vector< Ogre::AxisAlignedBox > boxes;
        CORE::CString::StringVector names;
        // 40 props of about 12 m on a lot of 100 m, a dome of 3.3 km, a ring of 2.6 km and a ground of 400 m
        for ( UInt32 i=0; i<40; ++i )
        {
            const Float32 x = 10.0f * (Float32) ( i % 8 ), z = 10.0f * (Float32) ( i / 8 );
            boxes.push_back( Ogre::AxisAlignedBox( Ogre::Vector3( x, 0.0f, z ), Ogre::Vector3( x + 12.0f, 6.0f, z + 9.0f ) ) );
            names.push_back( "SM_Prop_" + CORE::UInt32ToString( i ) );
        }
        boxes.push_back( Ogre::AxisAlignedBox( Ogre::Vector3( -1650.0f, -10.0f, -1650.0f ), Ogre::Vector3( 1650.0f, 800.0f, 1650.0f ) ) );
        names.push_back( "SM_Generic_SkyDome_01" );
        boxes.push_back( Ogre::AxisAlignedBox( Ogre::Vector3( -1300.0f, 100.0f, -1300.0f ), Ogre::Vector3( 1300.0f, 200.0f, 1300.0f ) ) );
        names.push_back( "SM_Generic_CloudRing_01" );
        boxes.push_back( Ogre::AxisAlignedBox( Ogre::Vector3( -200.0f, -1.0f, -200.0f ), Ogre::Vector3( 200.0f, 0.0f, 200.0f ) ) );
        names.push_back( "SM_Env_Ground_01" );
        Ogre::AxisAlignedBox box;
        UInt32 left = 0;
        ASSERT_TRUE( CSceneView::GetFramingBox( boxes, names, 20.0f, box, left ) );
        // the sky and the ring by name, the ground (400 m against a median of 12) by size: the props are the frame
        ASSERT_TRUE( 3 == left );
        ASSERT_TRUE( box.getMinimum().x == 0.0f && box.getMaximum().x == 82.0f && box.getMaximum().y == 6.0f && box.getMaximum().z == 49.0f );

        // an object that is only named like a sky but is the scene is the scene
        GUCEF::vector< Ogre::AxisAlignedBox > lone( 1, boxes[ 40 ] );
        CORE::CString::StringVector loneNames( 1, names[ 40 ] );
        ASSERT_TRUE( CSceneView::GetFramingBox( lone, loneNames, 20.0f, box, left ) && 0 == left && box.getMaximum().x == 1650.0f );
        ASSERT_FALSE( CSceneView::GetFramingBox( GUCEF::vector< Ogre::AxisAlignedBox >(), CORE::CString::StringVector(), 20.0f, box, left ) );

        // what is left out stays in view: the far plane reaches it
        COrbitController controller;
        controller.SetBoundsBox( COrbitController::SVector3( 0.0f, 0.0f, 0.0f ), COrbitController::SVector3( 82.0f, 6.0f, 49.0f ) );
        const Float32 near = controller.GetFarClipDistance();
        controller.SetMinimumFarClip( 5000.0f );
        ASSERT_TRUE( controller.GetFarClipDistance() >= 5000.0f && near < 5000.0f );
        controller.SetBounds( COrbitController::SVector3(), 1.0f );
        ASSERT_TRUE( controller.GetFarClipDistance() < 5000.0f );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestReportLoggerFolds( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Report logger: a file that is not in the pack and a name in more than one folder are one line each with the number and the first name, real errors stay errors" )
    try
    {
        CORE::CString name;
        ASSERT_TRUE( CReportLogger::FOLD_FILE_NOT_IN_PACK == CReportLogger::ClassifyMessage( "Vfs: Unable to locate a mount which can provide the file: /pack/Textures/T_A.uasset", name ) && "/pack/Textures/T_A.uasset" == name );
        ASSERT_TRUE( CReportLogger::FOLD_FILE_NOT_IN_PACK == CReportLogger::ClassifyMessage( "Ogre: Error: preparing texture 'Signs_01.psd'. Texture layer will be blank: Cannot locate resource Signs_01.psd in resource group g", name ) && "Signs_01.psd" == name );
        ASSERT_TRUE( CReportLogger::FOLD_FILE_NOT_IN_PACK == CReportLogger::ClassifyMessage( "CModelView: The picture Signs_01.psd of material Signs is not in the mount: the material is shown without it", name ) && "Signs_01.psd" == name );
        ASSERT_TRUE( CReportLogger::FOLD_NAME_IN_MORE_FOLDERS == CReportLogger::ClassifyMessage( "Ogre: Warning: Skipping 'FX.meta' because it already exists in resource group 'pack'", name ) && "FX.meta" == name );
        ASSERT_TRUE( CReportLogger::FOLD_WEIGHTS_REDUCED == CReportLogger::ClassifyMessage( "Ogre: Warning: the mesh 'intermediate/Hero.gltf' includes vertices with more than 4 bone assignments. The lowest weighted assignments", name ) && "intermediate/Hero.gltf" == name );
        ASSERT_TRUE( CReportLogger::FOLD_NONE == CReportLogger::ClassifyMessage( "CModelView: The model x cannot be loaded: bad", name ) );
        ASSERT_TRUE( CReportLogger::IsImporterDiagnostic( "Ogre: Assimp: Error, T0: Material property$tex.mapping is a string" ) && !CReportLogger::IsImporterDiagnostic( "CModelView: The model x cannot be loaded: bad" ) );

        CReportLogger logger;
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );
        for ( UInt32 i=0; i<3; ++i )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Vfs: Unable to locate a mount which can provide the file: /pack/Textures/T_" + CORE::UInt32ToString( i ) + ".uasset" );
        for ( UInt32 i=0; i<2; ++i )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Ogre: Warning: Skipping 'FX" + CORE::UInt32ToString( i ) + ".meta' because it already exists in resource group 'pack'" );
        for ( UInt32 i=0; i<4; ++i )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Ogre: Warning: the mesh 'intermediate/Hero.gltf' includes vertices with more than 4 bone assignments. The lowest weighted assignments beyond this limit have been removed" );
        // the importer calls this an error, the model went on: a warning
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Ogre: Assimp: Error, T0: Material property$tex.mapping is a string; failed to parse an integer array out of it." );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "report logger test: a real failure" );
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &logger );

        // only the real failure is an error and is there at once, with the warning of the importer; the folded ones come with the Pump after the messages stopped
        ASSERT_TRUE( 1 == logger.GetErrorCount() && 1 == logger.GetWarningCount() && 2 == logger.GetPendingCount() );
        ASSERT_TRUE( 2 == logger.Pump( GUCEF_NULL ) );
        ASSERT_TRUE( 3 == logger.Pump( GUCEF_NULL ) );
        ASSERT_TRUE( 0 == logger.Pump( GUCEF_NULL ) );
        ASSERT_TRUE( 1 == logger.GetErrorCount() && 2 == logger.GetWarningCount() );

        const CORE::CString notInPack = CReportLogger::MakeFoldedMessage( CReportLogger::FOLD_FILE_NOT_IN_PACK, 3, "/pack/Textures/T_0.uasset" );
        ASSERT_TRUE( notInPack.HasSubstr( "3 files that the pack refers to are not in it (the first: /pack/Textures/T_0.uasset)" ) >= 0 && notInPack.HasSubstr( "default" ) >= 0 );
        const CORE::CString duplicates = CReportLogger::MakeFoldedMessage( CReportLogger::FOLD_NAME_IN_MORE_FOLDERS, 2, "FX0.meta" );
        ASSERT_TRUE( duplicates.HasSubstr( "2 file names are in more than one folder" ) >= 0 && duplicates.HasSubstr( "FX0.meta" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestReportLogger( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Report logger: warnings and errors are kept for the report view, nothing below that, a long text keeps its first line as the message" )
    try
    {
        CReportLogger logger;
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "report logger test: just a message" );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "report logger test: a warning" );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "report logger test: an error\nwith a second line" );
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &logger );

        ASSERT_TRUE( 1 == logger.GetWarningCount() && 1 == logger.GetErrorCount() );
        ASSERT_TRUE( 2 == logger.GetPendingCount() );
        ASSERT_TRUE( 2 == logger.Pump( GUCEF_NULL ) );
        ASSERT_TRUE( 0 == logger.GetPendingCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchBasicTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.Basic" );

    TestResolver();
    TestOrbitController();
    TestMounts();
    TestBlockDecoder();
    TestSceneTreeFileSystem();
    TestReportLogger();
    TestReportLoggerFolds();
    TestAutoFrame();
    TestRobustFraming();
}

/*-------------------------------------------------------------------------*/
