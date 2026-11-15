/*
 *  AssetViewerLib_TestApp: Test application for the asset viewer library (AssetViewerLib)
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

#include "AssetViewerLib.h"
#include "AssetViewerLib_CBlockDecoder.h"
#include "AssetViewerLib_CSceneTreeFileSystemAccess.h"
#include "AssetViewerLib_CReportLogger.h"

#include "TestAssetViewerSupport.h"
#include "TestAssetViewerAssets.h"
#include "TestAssetViewerBasic.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETVIEWER;

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
        ASSERT_TRUE( ASSETKIND_TEXTURE == CAssetTypeResolver::GetKind( "tex.PNG" ) && ASSETKIND_TEXTURE == CAssetTypeResolver::GetKind( "t.dds" ) && ASSETKIND_TEXTURE == CAssetTypeResolver::GetKind( "t.ktx" ) );
        ASSERT_TRUE( ASSETKIND_MATERIAL == CAssetTypeResolver::GetKind( "m.material" ) );
        ASSERT_TRUE( ASSETKIND_SCENE == CAssetTypeResolver::GetKind( "level.scene" ) );
        ASSERT_TRUE( ASSETKIND_SKELETON == CAssetTypeResolver::GetKind( "rig.skeleton" ) );
        ASSERT_TRUE( ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "readme.txt" ) && ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "noextension" ) && ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "folder.fbx/inside" ) );
        ASSERT_TRUE( ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "dot." ) && ASSETKIND_OTHER == CAssetTypeResolver::GetKind( "" ) );

        CAssetTypeResolver resolver;
        ASSERT_TRUE( "Model" == resolver.GetTypeName( "a.fbx", false ) && "Texture" == resolver.GetTypeName( "a.png", false ) );
        ASSERT_TRUE( "Material" == resolver.GetTypeName( "a.material", false ) && "Scene" == resolver.GetTypeName( "a.scene", false ) );
        ASSERT_TRUE( "Skeleton" == resolver.GetTypeName( "a.skeleton", false ) && "Other" == resolver.GetTypeName( "a.bin", false ) );
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
PerformAssetViewerBasicTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetViewerLib.Basic" );

    TestResolver();
    TestOrbitController();
    TestMounts();
    TestBlockDecoder();
    TestSceneTreeFileSystem();
    TestReportLogger();
}

/*-------------------------------------------------------------------------*/
