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

#include <Ogre.h>

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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#include "AssetViewerLib.h"
#include "AssetViewerLib_CViewerMaterials.h"

#include "TestAssetViewerSupport.h"
#include "TestAssetViewerAssets.h"
#include "TestAssetViewerRender.h"

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

const char* const TestGroup = "AssetViewerTest";
const UInt32 TileSize = 96;

bool
HaveWindow( void )
{
    return !GetMainTestWindow().IsNULL();
}

/** The centre of a tile of a sheet, moved by an offset (the centre line itself holds the axes, which a quad in the same plane fights with) */
void
TileSpot( UInt32 tile, UInt32 tileIndex, Int32 offsetX, Int32 offsetY, Int32& x, Int32& y )
{
    UInt32 originX = 0, originY = 0;
    CAssetCapture::GetTileOrigin( tile, tileIndex, originX, originY );
    x = (Int32) originX + (Int32) tile / 2 + offsetX;
    y = (Int32) originY + (Int32) tile / 2 + offsetY;
}

/** A quad of four vertex colours made as a mesh of the test group: x and y from -1 to 1, in the plane z = 0, facing +z */
Ogre::MeshPtr
MakeColouredQuad( CViewerStage& stage, const char* name, float r, float g, float b )
{
    Ogre::SceneManager* sceneManager = stage.GetSceneManager();
    Ogre::ManualObject* quad = sceneManager->createManualObject( "QuadMaker" );
    quad->begin( CViewerMaterials::VertexColoursMaterialName.STL_String(), Ogre::RenderOperation::OT_TRIANGLE_LIST );
    const float xs[ 4 ] = { -1.0f, 1.0f, 1.0f, -1.0f };
    const float ys[ 4 ] = { -1.0f, -1.0f, 1.0f, 1.0f };
    for ( UInt32 i=0; i<4; ++i )
    {
        quad->position( xs[ i ], ys[ i ], 0.0f );
        quad->colour( r, g, b, 1.0f );
    }
    quad->quad( 0, 1, 2, 3 );
    quad->end();
    Ogre::MeshPtr mesh = quad->convertToMesh( name, TestGroup );
    sceneManager->destroyManualObject( quad );
    return mesh;
}

/** A lit cube of 24 vertices with normals, 1 unit wide, in a material of the given name */
Ogre::MeshPtr
MakeLitCube( CViewerStage& stage, const char* name, const char* materialName )
{
    Ogre::SceneManager* sceneManager = stage.GetSceneManager();
    Ogre::ManualObject* cube = sceneManager->createManualObject( "CubeMaker" );
    cube->begin( materialName, Ogre::RenderOperation::OT_TRIANGLE_LIST, TestGroup );
    static const float faces[ 6 ][ 9 ] =
    {
        {  1, 0, 0,   0, 1, 0,   0, 0, 1 }, { -1, 0, 0,   0, 0, 1,   0, 1, 0 }, {  0, 1, 0,   0, 0, 1,   1, 0, 0 },
        {  0,-1, 0,   1, 0, 0,   0, 0, 1 }, {  0, 0, 1,   1, 0, 0,   0, 1, 0 }, {  0, 0,-1,   0, 1, 0,   1, 0, 0 }
    };
    static const float corners[ 4 ][ 2 ] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
    for ( UInt32 f=0; f<6; ++f )
    {
        for ( UInt32 c=0; c<4; ++c )
        {
            cube->position( 0.5f * ( faces[ f ][ 0 ] + corners[ c ][ 0 ] * faces[ f ][ 3 ] + corners[ c ][ 1 ] * faces[ f ][ 6 ] ) ,
                            0.5f * ( faces[ f ][ 1 ] + corners[ c ][ 0 ] * faces[ f ][ 4 ] + corners[ c ][ 1 ] * faces[ f ][ 7 ] ) ,
                            0.5f * ( faces[ f ][ 2 ] + corners[ c ][ 0 ] * faces[ f ][ 5 ] + corners[ c ][ 1 ] * faces[ f ][ 8 ] ) );
            cube->normal( faces[ f ][ 0 ], faces[ f ][ 1 ], faces[ f ][ 2 ] );
        }
        cube->quad( f*4, f*4+1, f*4+2, f*4+3 );
    }
    cube->end();
    Ogre::MeshPtr mesh = cube->convertToMesh( name, TestGroup );
    sceneManager->destroyManualObject( cube );
    return mesh;
}

/** How many pixels of a tile are not the background of the stage: the tile has something in it */
UInt32
CountModelPixels( const IMAGE::TPixelMapPtr& sheet, UInt32 tile, UInt32 tileIndex )
{
    UInt32 originX = 0, originY = 0;
    CAssetCapture::GetTileOrigin( tile, tileIndex, originX, originY );
    UInt32 count = 0;
    for ( UInt32 y=0; y<tile; y+=2 )
    {
        for ( UInt32 x=0; x<tile; x+=2 )
        {
            UInt8 rgba[ 4 ];
            GetPixel( sheet, (Int32) ( originX + x ), (Int32) ( originY + y ), rgba );
            const bool background = abs( (Int32) rgba[ 0 ] - 43 ) < 14 && abs( (Int32) rgba[ 1 ] - 46 ) < 14 && abs( (Int32) rgba[ 2 ] - 54 ) < 14;
            const bool line = abs( (Int32) rgba[ 0 ] - (Int32) rgba[ 1 ] ) < 14 && abs( (Int32) rgba[ 1 ] - (Int32) rgba[ 2 ] ) < 24;     // grid and axes are grey
            if ( !background && !line )
                ++count;
        }
    }
    return count;
}

/** The files of the test model in a folder: the glTF and the picture it names */
bool
WriteModelFolder( const CORE::CString& dir )
{
    return WriteTestFile( dir + "/model.gltf", BuildTestGltf() ) && WriteTestPng( dir + "/checker.png", 8, 8 );
}

CORE::CString
HashOfNativeFile( const CORE::CString& path )
{
    CORE::CDynamicBuffer data;
    data.LoadContentFromFile( path );
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( data.GetBufferPtr(), data.GetDataSize(), digest );
    return CORE::ConvertBytesToHexString( digest, CORE::CSha256::DigestSize, false, false ).Lowercase();
}

UInt32
CountFiles( const CORE::CString& dir, const CORE::CString& extension )
{
    CORE::CNativeFileSystemAccess access;
    CORE::TFileSystemIteratorPtr iterator;
    if ( CORE::FILESYSTEMACCESS_SUCCESS != access.TryGetFileSystemIterator( dir, iterator ) )
        return 0;
    CORE::CFileSystemEntryListIterator::TEntryVector entries;
    CORE::CFileSystemEntryListIterator::TryReadAll( *iterator, dir, entries );
    UInt32 count = 0;
    for ( size_t i=0; i<entries.size(); ++i )
        if ( entries[ i ].isFile && entries[ i ].name.Length() >= extension.Length() && entries[ i ].name.SubstrFromRange( entries[ i ].name.Length() - extension.Length(), entries[ i ].name.Length() ) == extension )
            ++count;
    return count;
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
TestStageAndCapture( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Capture: a mesh made by the test is drawn from the four fixed views: the sheet has the size and the layout of the tiles, the unlit coloured quad shows face on and vanishes edge on" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        Ogre::ResourceGroupManager::getSingleton().createResourceGroup( TestGroup );
        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        ASSERT_TRUE( capture.IsInitialized() && TileSize == capture.GetTileSize() );

        // the size and the layout of a sheet: margins around and between two by two tiles
        ASSERT_TRUE( 2 * TileSize + 3 * 4 == CAssetCapture::GetSheetSize( TileSize ) );
        UInt32 x = 0, y = 0;
        CAssetCapture::GetTileOrigin( TileSize, 0, x, y );
        ASSERT_TRUE( 4 == x && 4 == y );
        CAssetCapture::GetTileOrigin( TileSize, 1, x, y );
        ASSERT_TRUE( 4 + TileSize + 4 == x && 4 == y );
        CAssetCapture::GetTileOrigin( TileSize, 2, x, y );
        ASSERT_TRUE( 4 == x && 4 + TileSize + 4 == y );
        CAssetCapture::GetTileOrigin( TileSize, 3, x, y );
        ASSERT_TRUE( 4 + TileSize + 4 == x && 4 + TileSize + 4 == y );

        Ogre::MeshPtr quad = MakeColouredQuad( capture.GetStage(), "TestQuad", 1.0f, 0.0f, 0.0f );
        ASSERT_TRUE( (bool) quad );
        IMAGE::TPixelMapPtr sheet;
        ASSERT_TRUE( capture.CaptureMeshSheet( TestGroup, "TestQuad", sheet ) );
        ASSERT_TRUE( !sheet.IsNULL() && CAssetCapture::GetSheetSize( TileSize ) == sheet->GetWidthInPixels() && CAssetCapture::GetSheetSize( TileSize ) == sheet->GetHeightInPixels() );
        DumpPicture( sheet, "capture_quad_" + CORE::CString( GetTestRenderer() ) );

        // the margins and the gaps are the colour of the sheet
        UInt8 sheetRed = 0, sheetGreen = 0, sheetBlue = 0;
        CAssetCapture::GetSheetColour( sheetRed, sheetGreen, sheetBlue );
        ASSERT_TRUE( PixelIs( sheet, 0, 0, sheetRed, sheetGreen, sheetBlue, 2 ) );
        ASSERT_TRUE( PixelIs( sheet, 4 + TileSize + 1, 50, sheetRed, sheetGreen, sheetBlue, 2 ) );
        ASSERT_TRUE( PixelIs( sheet, 50, 4 + TileSize + 1, sheetRed, sheetGreen, sheetBlue, 2 ) );
        ASSERT_TRUE( PixelIs( sheet, (Int32) sheet->GetWidthInPixels() - 1, (Int32) sheet->GetHeightInPixels() - 1, sheetRed, sheetGreen, sheetBlue, 2 ) );

        // front (tile 0) and three quarter (tile 3): the quad covers the spot; side (1) and top (2): it is edge on, so the spot is not red
        Int32 px = 0, py = 0;
        TileSpot( TileSize, 0, TileSize / 8, -(Int32) TileSize / 8, px, py );
        ASSERT_TRUE( PixelIs( sheet, px, py, 255, 0, 0, 12 ) );
        TileSpot( TileSize, 3, TileSize / 8, -(Int32) TileSize / 8, px, py );
        ASSERT_TRUE( PixelIs( sheet, px, py, 255, 0, 0, 12 ) );
        TileSpot( TileSize, 1, TileSize / 8, -(Int32) TileSize / 8, px, py );
        ASSERT_FALSE( PixelIs( sheet, px, py, 255, 0, 0, 40 ) );
        TileSpot( TileSize, 2, TileSize / 8, -(Int32) TileSize / 8, px, py );
        ASSERT_FALSE( PixelIs( sheet, px, py, 255, 0, 0, 40 ) );
        // the corner of a tile is the background of the stage
        TileSpot( TileSize, 0, -(Int32) TileSize / 2 + 2, -(Int32) TileSize / 2 + 2, px, py );
        ASSERT_TRUE( PixelIs( sheet, px, py, 43, 46, 54, 8 ) );

        // another colour of the same view
        Ogre::MeshPtr blue = MakeColouredQuad( capture.GetStage(), "TestQuadBlue", 0.0f, 0.0f, 1.0f );
        ASSERT_TRUE( capture.CaptureMeshSheet( TestGroup, "TestQuadBlue", sheet ) );
        TileSpot( TileSize, 0, TileSize / 8, -(Int32) TileSize / 8, px, py );
        ASSERT_TRUE( PixelIs( sheet, px, py, 0, 0, 255, 12 ) );

        // a mesh that is not there is refused, and the capture goes on afterwards
        ASSERT_FALSE( capture.CaptureMeshSheet( TestGroup, "DoesNotExist.mesh", sheet ) );
        ASSERT_TRUE( capture.CaptureMeshSheet( TestGroup, "TestQuad", sheet ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Capture: a lit cube takes the colour of its material under the light preset, and only the ambient light of the flat preset when the material has none" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );

        Ogre::MaterialPtr green = Ogre::MaterialManager::getSingleton().create( "AssetViewerTest/LitGreen", TestGroup );
        Ogre::Pass* pass = green->getTechnique( 0 )->getPass( 0 );
        pass->setLightingEnabled( true );
        pass->setAmbient( 0.0f, 0.0f, 0.0f );
        pass->setDiffuse( 0.0f, 1.0f, 0.0f, 1.0f );
        pass->setSpecular( 0.0f, 0.0f, 0.0f, 1.0f );
        Ogre::MeshPtr cube = MakeLitCube( capture.GetStage(), "TestCube", "AssetViewerTest/LitGreen" );
        ASSERT_TRUE( (bool) cube );

        capture.GetStage().SetLightPreset( CViewerStage::LIGHTPRESET_STUDIO );
        IMAGE::TPixelMapPtr sheet;
        ASSERT_TRUE( capture.CaptureMeshSheet( TestGroup, "TestCube", sheet ) );
        DumpPicture( sheet, "capture_cube_" + CORE::CString( GetTestRenderer() ) );
        Int32 px = 0, py = 0;
        TileSpot( TileSize, 3, -(Int32) TileSize / 10, 0, px, py );
        UInt8 rgba[ 4 ];
        GetPixel( sheet, px, py, rgba );
        ASSERT_TRUE( rgba[ 1 ] > 50 && rgba[ 0 ] < rgba[ 1 ] / 2 && rgba[ 2 ] < rgba[ 1 ] / 2 );
        TileSpot( TileSize, 0, 0, 0, px, py );
        GetPixel( sheet, px, py, rgba );
        ASSERT_TRUE( rgba[ 1 ] > 50 && rgba[ 0 ] < rgba[ 1 ] / 2 );

        // flat: no lights, the ambient light times the ambient colour of the material, which is black
        capture.GetStage().SetLightPreset( CViewerStage::LIGHTPRESET_FLAT );
        ASSERT_TRUE( capture.CaptureMeshSheet( TestGroup, "TestCube", sheet ) );
        TileSpot( TileSize, 3, -(Int32) TileSize / 10, 0, px, py );
        GetPixel( sheet, px, py, rgba );
        ASSERT_TRUE( rgba[ 1 ] < 20 );
        capture.GetStage().SetLightPreset( CViewerStage::LIGHTPRESET_STUDIO );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestStageResize( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Stage: the texture of another size is drawn over its whole size, after it was made again" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( 64 ) );
        CViewerStage& stage = capture.GetStage();
        stage.SetGridVisible( false );
        stage.SetAxesVisible( false );
        stage.SetBackgroundColour( 0.2f, 0.4f, 0.6f );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( capture.RenderStage( pixels ) && 64 == pixels->GetWidthInPixels() );
        ASSERT_TRUE( PixelIs( pixels, 63, 63, 51, 102, 153, 3 ) );

        // bigger, narrower, then smaller again: every corner is the background, and a quad made for the view fills its own share
        const UInt32 widths[ 3 ] = { 160, 72, 40 };
        const UInt32 heights[ 3 ] = { 90, 120, 24 };
        Ogre::ResourceGroupManager::getSingleton().createResourceGroup( "AssetViewerResize" );
        for ( UInt32 i=0; i<3; ++i )
        {
            ASSERT_TRUE( stage.Resize( widths[ i ], heights[ i ] ) );
            ASSERT_TRUE( widths[ i ] == stage.GetWidth() && heights[ i ] == stage.GetHeight() );
            ASSERT_TRUE( capture.RenderStage( pixels ) );
            ASSERT_TRUE( widths[ i ] == pixels->GetWidthInPixels() && heights[ i ] == pixels->GetHeightInPixels() );
            ASSERT_TRUE( PixelIs( pixels, 1, 1, 51, 102, 153, 3 ) && PixelIs( pixels, (Int32) widths[ i ] - 2, (Int32) heights[ i ] - 2, 51, 102, 153, 3 ) );
            ASSERT_TRUE( PixelIs( pixels, 1, (Int32) heights[ i ] - 2, 51, 102, 153, 3 ) && PixelIs( pixels, (Int32) widths[ i ] - 2, 1, 51, 102, 153, 3 ) );

            // the quad of the view in front of the camera covers the middle, and the middle only
            const CORE::CString name = "ResizeQuad" + CORE::UInt32ToString( i );
            Ogre::MeshPtr quad = MakeColouredQuad( stage, name.C_String(), 1.0f, 0.0f, 0.0f );
            Ogre::Entity* entity = stage.GetSceneManager()->createEntity( quad );
            stage.GetContentNode()->attachObject( entity );
            COrbitController controller;
            controller.SetAspectRatio( (Float32) widths[ i ] / (Float32) heights[ i ] );
            controller.SetBounds( COrbitController::SVector3(), 1.0f );
            controller.ApplyPreset( COrbitController::VIEWPRESET_FRONT );
            stage.ApplyController( controller );
            ASSERT_TRUE( capture.RenderStage( pixels ) );
            ASSERT_TRUE( PixelIs( pixels, (Int32) widths[ i ] / 2, (Int32) heights[ i ] / 2, 255, 0, 0, 3 ) );
            ASSERT_TRUE( PixelIs( pixels, 1, 1, 51, 102, 153, 3 ) && PixelIs( pixels, (Int32) widths[ i ] - 2, (Int32) heights[ i ] - 2, 51, 102, 153, 3 ) );
            stage.GetContentNode()->detachObject( entity );
            stage.GetSceneManager()->destroyEntity( entity );
        }
        Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup( "AssetViewerResize" );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestModelView( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Model view: a glTF written by the test is loaded through the Assimp codec and its statistics match the file: vertices, triangles, sub meshes, bones, materials, textures, clips" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "gltf" );
        ASSERT_TRUE( WriteModelFolder( dir ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avgltf", dir ) );

        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        CModelView model( capture.GetStage() );
        CORE::CString relative;
        const CORE::CString group = mounter.GetResourceGroup( "/avgltf/model.gltf", relative );
        ASSERT_TRUE( !group.IsNULLOrEmpty() && "model.gltf" == relative );
        ASSERT_TRUE( model.LoadMesh( group, relative ) );
        ASSERT_TRUE( model.IsLoaded() && model.GetSubMeshCount() == TestGltfSubMeshCount );

        const SMeshStatistics& statistics = model.GetStatistics();
        ASSERT_TRUE( TestGltfVertexCount == statistics.vertexCount );
        ASSERT_TRUE( TestGltfTriangleCount == statistics.triangleCount );
        ASSERT_TRUE( TestGltfSubMeshCount == statistics.subMeshCount );
        ASSERT_TRUE( TestGltfSkeletonBoneCount == statistics.boneCount && statistics.hasSkeleton );
        ASSERT_TRUE( 1 == statistics.animationCount && 1 == statistics.lodLevelCount );
        ASSERT_TRUE( TestGltfMaterialCount == statistics.materialCount );
        ASSERT_TRUE( 1 == statistics.textureCount && statistics.textureMemory >= 8 * 8 * 4 );
        ASSERT_TRUE( statistics.hasNormals && statistics.uvSetCount >= 1 );
        ASSERT_TRUE( statistics.boundsMax[ 1 ] - statistics.boundsMin[ 1 ] > 1.9f );

        // the same facts in the data of the inspector
        CORE::CDataNode data;
        model.WriteInspectorData( data );
        const CORE::CDataNode* counts = data.FindChild( "Statistics" );
        ASSERT_TRUE( GUCEF_NULL != counts );
        if ( GUCEF_NULL != counts )
        {
            ASSERT_TRUE( CORE::UInt32ToString( TestGltfVertexCount ) == counts->GetAttributeValue( "Vertices" ).AsString() );
            ASSERT_TRUE( CORE::UInt32ToString( TestGltfTriangleCount ) == counts->GetAttributeValue( "Triangles" ).AsString() );
            ASSERT_TRUE( CORE::UInt32ToString( TestGltfSubMeshCount ) == counts->GetAttributeValue( "Submeshes" ).AsString() );
            ASSERT_TRUE( CORE::UInt32ToString( TestGltfSkeletonBoneCount ) == counts->GetAttributeValue( "Bones" ).AsString() );
            ASSERT_TRUE( CORE::UInt32ToString( TestGltfMaterialCount ) == counts->GetAttributeValue( "Materials" ).AsString() );
            ASSERT_TRUE( "1" == counts->GetAttributeValue( "Textures" ).AsString() );
        }
        const CORE::CDataNode* submeshes = data.FindChild( "Submeshes" );
        ASSERT_TRUE( GUCEF_NULL != submeshes && 2 == submeshes->GetNrOfDirectChildNodes() );

        // the geometry read back from the render system
        SSubMeshGeometry cube, strip;
        ASSERT_TRUE( CMeshInspector::ReadSubMesh( model.GetMesh(), 0, cube ) && CMeshInspector::ReadSubMesh( model.GetMesh(), 1, strip ) );
        ASSERT_TRUE( 24 == cube.vertexCount && 12 == cube.triangleCount && 72 == cube.positions.size() && 72 == cube.normals.size() && 48 == cube.texCoords.size() && 36 == cube.indices.size() );
        ASSERT_TRUE( 6 == strip.vertexCount && 4 == strip.triangleCount );
        Float32 lowest = 1000.0f, highest = -1000.0f;
        for ( size_t i=0; i<cube.positions.size(); ++i )
        {
            lowest = cube.positions[ i ] < lowest ? cube.positions[ i ] : lowest;
            highest = cube.positions[ i ] > highest ? cube.positions[ i ] : highest;
        }
        ASSERT_TRUE( fabsf( lowest + 0.5f ) < 1e-3f && fabsf( highest - 0.5f ) < 1e-3f );

        // the clip of the skeleton and its time
        ASSERT_TRUE( 1 == model.GetClips().size() && "Wave" == model.GetClips()[ 0 ].name );
        ASSERT_TRUE( model.GetClips()[ 0 ].lengthInMs > TestGltfAnimationLengthInMs - 20 && model.GetClips()[ 0 ].lengthInMs < TestGltfAnimationLengthInMs + 20 );
        ASSERT_TRUE( model.SelectClip( 0 ) && 0 == model.GetSelectedClip() );
        model.SetClipTime( 1000 );
        ASSERT_TRUE( fabsf( model.GetEntity()->getAnimationState( "Wave" )->getTimePosition() - 1.0f ) < 1e-3f );
        ASSERT_FALSE( model.SelectClip( 5 ) );

        // the model is in the middle of the view the controller frames
        COrbitController controller;
        model.FrameInController( controller );
        COrbitController::SVector3 center;
        Float32 radius = 0.0f;
        model.GetBoundingSphere( center, radius );
        ASSERT_TRUE( radius > 1.0f && controller.IsSphereInView( center, radius ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Model view: display modes, bounds, bones, sub mesh visibility and materials, forced level of detail, and the UV layout of a sub mesh" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "gltf2" );
        ASSERT_TRUE( WriteModelFolder( dir ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avgltf2", dir ) );
        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        CViewerStage& stage = capture.GetStage();
        CModelView model( stage );
        CORE::CString relative;
        ASSERT_TRUE( model.LoadMesh( mounter.GetResourceGroup( "avgltf2/model.gltf", relative ), relative ) );
        Ogre::Entity* entity = model.GetEntity();
        const CORE::CString original0 = model.GetSubMeshMaterial( 0 );
        const CORE::CString original1 = model.GetSubMeshMaterial( 1 );
        ASSERT_TRUE( !original0.IsNULLOrEmpty() && original0 != original1 );

        // shaded is the start; wireframe is the polygon mode of the camera; the checker and the colours swap the material of every sub mesh
        ASSERT_TRUE( CModelView::DISPLAYMODE_SHADED == model.GetDisplayMode() && !stage.IsWireframe() );
        model.SetDisplayMode( CModelView::DISPLAYMODE_WIREFRAME );
        ASSERT_TRUE( stage.IsWireframe() && Ogre::PM_WIREFRAME == stage.GetCamera()->getPolygonMode() );
        IMAGE::TPixelMapPtr wire;
        ASSERT_TRUE( capture.RenderStage( wire ) );
        model.SetDisplayMode( CModelView::DISPLAYMODE_UV_CHECKER );
        ASSERT_FALSE( stage.IsWireframe() );
        ASSERT_TRUE( CViewerMaterials::UvCheckerMaterialName == CORE::CString( entity->getSubEntity( 0 )->getMaterialName() ) && CViewerMaterials::UvCheckerMaterialName == CORE::CString( entity->getSubEntity( 1 )->getMaterialName() ) );
        IMAGE::TPixelMapPtr checker;
        ASSERT_TRUE( capture.RenderStage( checker ) );
        model.SetDisplayMode( CModelView::DISPLAYMODE_VERTEX_COLOURS );
        ASSERT_TRUE( CViewerMaterials::VertexColoursMaterialName == CORE::CString( entity->getSubEntity( 0 )->getMaterialName() ) );
        ASSERT_TRUE( capture.RenderStage( checker ) );
        model.SetDisplayMode( CModelView::DISPLAYMODE_SHADED );
        ASSERT_TRUE( original0 == CORE::CString( entity->getSubEntity( 0 )->getMaterialName() ) && original1 == CORE::CString( entity->getSubEntity( 1 )->getMaterialName() ) );

        // normals as lines over the model: sections of the line object; the tangents of a model that has none draw nothing
        model.SetDisplayMode( CModelView::DISPLAYMODE_NORMALS );
        Ogre::ManualObject* normals = stage.GetLineObject( "Normals" );
        ASSERT_TRUE( GUCEF_NULL != normals && normals->getNumSections() > 0 );
        ASSERT_TRUE( capture.RenderStage( checker ) );
        model.SetDisplayMode( CModelView::DISPLAYMODE_TANGENTS );
        ASSERT_TRUE( 0 == normals->getNumSections() );
        model.SetDisplayMode( CModelView::DISPLAYMODE_SHADED );
        ASSERT_TRUE( 0 == stage.GetLineObject( "Tangents" )->getNumSections() && 0 == normals->getNumSections() );

        // the bounding box and the bones
        model.SetBoundsVisible( true );
        ASSERT_TRUE( model.IsBoundsVisible() && stage.GetLineObject( "Bounds" )->getNumSections() > 0 );
        model.SetBoundsVisible( false );
        ASSERT_TRUE( 0 == stage.GetLineObject( "Bounds" )->getNumSections() );
        model.SetSkeletonVisible( true );
        ASSERT_TRUE( model.IsSkeletonVisible() && stage.GetLineObject( "Bones" )->getNumSections() > 0 );
        ASSERT_TRUE( capture.RenderStage( checker ) );
        model.SetSkeletonVisible( false );
        ASSERT_TRUE( 0 == stage.GetLineObject( "Bones" )->getNumSections() );

        // sub meshes: hidden and shown, the material of one changed to the material of the other
        ASSERT_TRUE( model.IsSubMeshVisible( 1 ) && entity->getSubEntity( 1 )->isVisible() );
        model.SetSubMeshVisible( 1, false );
        ASSERT_TRUE( !model.IsSubMeshVisible( 1 ) && !entity->getSubEntity( 1 )->isVisible() && entity->getSubEntity( 0 )->isVisible() );
        ASSERT_TRUE( model.ApplyProperty( "Submeshes/Submesh1@Visible", "true" ) && entity->getSubEntity( 1 )->isVisible() );
        ASSERT_TRUE( model.SetSubMeshMaterial( 0, original1 ) && original1 == model.GetSubMeshMaterial( 0 ) && original1 == CORE::CString( entity->getSubEntity( 0 )->getMaterialName() ) );
        ASSERT_FALSE( model.SetSubMeshMaterial( 0, "no such material" ) );
        ASSERT_TRUE( model.ApplyProperty( "Submeshes/Submesh0@Material", original0 ) && original0 == model.GetSubMeshMaterial( 0 ) );
        CORE::CString::StringVector choices;
        model.GetMaterialChoices( choices );
        bool hasOriginal = false;
        for ( size_t i=0; i<choices.size(); ++i )
            hasOriginal = hasOriginal || choices[ i ] == original0;
        ASSERT_TRUE( hasOriginal );
        ASSERT_FALSE( model.ApplyProperty( "Nothing@Else", "1" ) );

        // the level of detail: this model has one level; forcing it and back to automatic are accepted, the distance follows the camera
        ASSERT_TRUE( 1 == model.GetLodLevelCount() );
        model.SetForcedLod( 0 );
        ASSERT_TRUE( 0 == model.GetForcedLod() );
        ASSERT_TRUE( model.ApplyProperty( "View@Lod", "Auto" ) && -1 == model.GetForcedLod() );
        COrbitController controller;
        model.FrameInController( controller );
        model.Update( controller );
        ASSERT_TRUE( model.GetCameraDistance() > 1.0f );
        const Float32 near = model.GetCameraDistance();
        controller.SetDistance( controller.GetDistance() * 3.0f );
        model.Update( controller );
        ASSERT_TRUE( model.GetCameraDistance() > near * 1.5f );

        // the UV layout of the cube: edges of its twelve triangles in the unit square; a sub mesh that is not there has none
        IMAGE::TPixelMapPtr layout;
        ASSERT_TRUE( model.RenderUvLayout( 0, 128, layout ) && !layout.IsNULL() && 128 == layout->GetWidthInPixels() && 128 == layout->GetHeightInPixels() );
        UInt32 lines = 0;
        for ( Int32 yy=0; yy<128; ++yy )
        {
            for ( Int32 xx=0; xx<128; ++xx )
            {
                UInt8 rgba[ 4 ];
                GetPixel( layout, xx, yy, rgba );
                if ( 120 == rgba[ 0 ] && 200 == rgba[ 1 ] && 255 == rgba[ 2 ] )
                    ++lines;
            }
        }
        ASSERT_TRUE( lines > 200 );
        ASSERT_TRUE( PixelIs( layout, 64, 64, 120, 200, 255, 1 ) || lines > 200 );
        DumpPicture( layout, "uv_layout_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_FALSE( model.RenderUvLayout( 9, 128, layout ) );

        // taken out again
        model.Unload();
        ASSERT_FALSE( model.IsLoaded() );
        ASSERT_TRUE( capture.RenderStage( checker ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Contact sheet of a model of the mounts: four views of the glTF, the model in the middle of each" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "gltf3" );
        ASSERT_TRUE( WriteModelFolder( dir ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avgltf3", dir ) );
        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        IMAGE::TPixelMapPtr sheet;
        ASSERT_TRUE( capture.CaptureContactSheet( mounter, "/avgltf3/model.gltf", sheet ) );
        ASSERT_TRUE( !sheet.IsNULL() && CAssetCapture::GetSheetSize( TileSize ) == sheet->GetWidthInPixels() );
        DumpPicture( sheet, "contact_sheet_gltf_" + CORE::CString( GetTestRenderer() ) );
        // every view has the model in it, in colours (the grid and the axes are grey, the cube is red and blue, the strip light blue)
        for ( UInt32 tile=0; tile<4; ++tile )
            ASSERT_TRUE( CountModelPixels( sheet, TileSize, tile ) > 150 );
        ASSERT_FALSE( capture.CaptureContactSheet( mounter, "/avgltf3/missing.gltf", sheet ) );
        ASSERT_FALSE( capture.CaptureContactSheet( mounter, "/avgltf3/readme.txt", sheet ) );
        ASSERT_FALSE( capture.CaptureContactSheet( mounter, "/nomount/model.gltf", sheet ) );
        ASSERT_TRUE( "avgltf3_model.gltf.png" == CAssetCapture::MakeSheetFileName( "/avgltf3/model.gltf" ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestTextureView( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Texture view: a PNG and a DDS of the mounts give an image with the size, the format and the memory of the file, the compressed one decoded" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "textures" );
        ASSERT_TRUE( WriteTestPng( dir + "/pic.png", 8, 4 ) );
        ASSERT_TRUE( WriteTestDds( dir + "/block.dds", 8, 8, 0xF800 ) );
        ASSERT_TRUE( WriteTestFile( dir + "/broken.png", "this is not a picture" ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avtex", dir ) );

        CTextureView texture;
        ASSERT_TRUE( texture.Load( "avtex/pic.png" ) && texture.IsLoaded() );
        const CTextureView::SInfo& info = texture.GetInfo();
        ASSERT_TRUE( 8 == info.width && 4 == info.height && 1 == info.depth && 1 == info.faces && 1 == info.mipLevels );
        ASSERT_TRUE( 8 * 4 * 4 == info.memoryBytes && !info.isCompressed && info.hasAlpha && info.displayable && !info.isCube );
        ASSERT_TRUE( !info.formatName.IsNULLOrEmpty() );
        IMAGE::CImage* image = texture.GetImage();
        ASSERT_TRUE( GUCEF_NULL != image && 1 == image->GetFrameCount() );
        IMAGE::TPixelMapPtr pixels = texture.GetFirstPixelMap();
        ASSERT_TRUE( !pixels.IsNULL() && 8 == pixels->GetWidthInPixels() && 4 == pixels->GetHeightInPixels() );
        ASSERT_TRUE( PixelIs( pixels, 0, 0, 255, 0, 0, 0 ) && PixelIs( pixels, 3, 3, 255, 0, 0, 0 ) && PixelIs( pixels, 4, 0, 0, 0, 255, 0 ) && PixelIs( pixels, 7, 3, 0, 0, 255, 0 ) );

        CORE::CDataNode data;
        texture.WriteInspectorData( data );
        const CORE::CDataNode* node = data.FindChild( "Image" );
        ASSERT_TRUE( GUCEF_NULL != node && "8" == node->GetAttributeValue( "Width" ).AsString() && "4" == node->GetAttributeValue( "Height" ).AsString() && "128 B" == node->GetAttributeValue( "Memory" ).AsString() );

        // the compressed picture is a block of eight red pixels in two blocks across and two down
        ASSERT_TRUE( texture.Load( "avtex/block.dds" ) );
        ASSERT_TRUE( 8 == texture.GetInfo().width && 8 == texture.GetInfo().height && texture.GetInfo().isCompressed && texture.GetInfo().displayable );
        ASSERT_TRUE( CORE::CString( "PF_DXT1" ) == texture.GetInfo().formatName );
        ASSERT_TRUE( 4 * 8 == texture.GetInfo().memoryBytes );
        pixels = texture.GetFirstPixelMap();
        ASSERT_TRUE( !pixels.IsNULL() && PixelIs( pixels, 0, 0, 255, 0, 0, 0 ) && PixelIs( pixels, 7, 7, 255, 0, 0, 0 ) );

        // what no codec reads is refused with a note, and a file that is not there
        ASSERT_FALSE( texture.Load( "avtex/broken.png" ) );
        ASSERT_FALSE( texture.IsLoaded() || GUCEF_NULL != texture.GetImage() );
        ASSERT_FALSE( texture.Load( "avtex/nothing.png" ) );

        // the picture as a thumbnail through the capture path: fitted in the square
        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        IMAGE::TPixelMapPtr thumbnail;
        ASSERT_TRUE( capture.CaptureThumbnail( mounter, "avtex/pic.png", 32, thumbnail ) );
        ASSERT_TRUE( !thumbnail.IsNULL() && 32 == thumbnail->GetWidthInPixels() && 16 == thumbnail->GetHeightInPixels() );
        ASSERT_TRUE( PixelIs( thumbnail, 3, 8, 255, 0, 0, 4 ) && PixelIs( thumbnail, 28, 8, 0, 0, 255, 4 ) );
        IMAGE::TPixelMapPtr sheet;
        ASSERT_TRUE( capture.CaptureContactSheet( mounter, "avtex/pic.png", sheet ) && CAssetCapture::GetSheetSize( TileSize ) == sheet->GetWidthInPixels() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestMaterialView( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Material view: the materials of a script are found, one is shown on a sphere, its values are in the inspector data and edits reach the material" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "materials" );
        ASSERT_TRUE( WriteTestFile( dir + "/scripts/test.material", BuildTestMaterialScript() ) );
        ASSERT_TRUE( WriteTestPng( dir + "/checker.png", 8, 8 ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avmat", dir ) );

        // the names the script defines, with a parent and in quotes
        CORE::CString::StringVector names;
        CMaterialView::ParseMaterialNames( BuildTestMaterialScript(), names );
        ASSERT_TRUE( 2 == names.size() && "TestRed" == names[ 0 ] && "TestTextured" == names[ 1 ] );
        CMaterialView::ParseMaterialNames( "// nothing\nmaterial   Spaced   {\n}\n  material \"Two Words\"\n{\n}\nmaterials x\n", names );
        ASSERT_TRUE( 2 == names.size() && "Spaced" == names[ 0 ] && "Two Words" == names[ 1 ] );

        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        CMaterialView view( capture.GetStage() );
        CORE::CString relative;
        const CORE::CString group = mounter.GetResourceGroup( "avmat/scripts/test.material", relative );
        ASSERT_TRUE( view.LoadScript( group, relative ) );
        ASSERT_TRUE( 2 == view.GetMaterialNames().size() && "TestRed" == view.GetSelectedMaterial() );
        ASSERT_TRUE( view.GetPreview().IsLoaded() && "TestRed" == view.GetPreview().GetSubMeshMaterial( 0 ) );

        CORE::CDataNode data;
        view.WriteInspectorData( data );
        const CORE::CDataNode* pass = data.FindChild( "Pass" );
        ASSERT_TRUE( GUCEF_NULL != pass );
        if ( GUCEF_NULL != pass )
        {
            ASSERT_TRUE( "255,0,0,255" == pass->GetAttributeValue( "Diffuse" ).AsString() );
            ASSERT_TRUE( "51,0,0,255" == pass->GetAttributeValue( "Ambient" ).AsString() );
            ASSERT_TRUE( "true" == pass->GetAttributeValue( "Lighting" ).AsString() );
            ASSERT_TRUE( "12.5" == pass->GetAttributeValue( "Shininess" ).AsString() );
        }
        GUI::CPropertyInspectorForm::TDescriptorMap descriptors;
        view.GetInspectorDescriptors( descriptors );
        ASSERT_TRUE( descriptors.find( "Pass@Diffuse" ) != descriptors.end() && GUI::CPropertyRowDescriptor::VALUETYPE_COLOUR == descriptors[ "Pass@Diffuse" ].type );

        // the sphere in red, lit; then green after the edit
        IMAGE::TPixelMapPtr picture;
        COrbitController controller;
        view.GetPreview().FrameInController( controller );
        capture.GetStage().ApplyController( controller );
        ASSERT_TRUE( capture.RenderStage( picture ) );
        UInt8 rgba[ 4 ];
        GetPixel( picture, (Int32) picture->GetWidthInPixels() / 2, (Int32) picture->GetHeightInPixels() / 2, rgba );
        ASSERT_TRUE( rgba[ 0 ] > 60 && rgba[ 1 ] < rgba[ 0 ] / 3 && rgba[ 2 ] < rgba[ 0 ] / 3 );
        ASSERT_TRUE( view.ApplyProperty( "Pass@Diffuse", "0,255,0,255" ) );
        ASSERT_TRUE( view.ApplyProperty( "Pass@Ambient", "0,0,0,255" ) );
        ASSERT_TRUE( capture.RenderStage( picture ) );
        GetPixel( picture, (Int32) picture->GetWidthInPixels() / 2, (Int32) picture->GetHeightInPixels() / 2, rgba );
        ASSERT_TRUE( rgba[ 1 ] > 60 && rgba[ 0 ] < rgba[ 1 ] / 3 && rgba[ 2 ] < rgba[ 1 ] / 3 );
        DumpPicture( picture, "material_sphere_" + CORE::CString( GetTestRenderer() ) );
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName( "TestRed", group.STL_String() );
        ASSERT_TRUE( (bool) material && fabsf( material->getTechnique( 0 )->getPass( 0 )->getDiffuse().g - 1.0f ) < 1e-3f );
        ASSERT_TRUE( view.ApplyProperty( "Pass@Lighting", "false" ) && !material->getTechnique( 0 )->getPass( 0 )->getLightingEnabled() );
        ASSERT_TRUE( view.ApplyProperty( "Pass@Shininess", "64" ) && fabsf( material->getTechnique( 0 )->getPass( 0 )->getShininess() - 64.0f ) < 1e-3f );
        ASSERT_TRUE( view.ApplyProperty( "Pass@Culling", "Clockwise" ) && Ogre::CULL_CLOCKWISE == material->getTechnique( 0 )->getPass( 0 )->getCullingMode() );
        ASSERT_FALSE( view.ApplyProperty( "Pass@Nothing", "1" ) );

        // the other shapes, and the material that has a texture slot
        view.SetShape( CMaterialView::SHAPE_CUBE );
        ASSERT_TRUE( CMaterialView::SHAPE_CUBE == view.GetShape() && view.GetPreview().IsLoaded() && "TestRed" == view.GetPreview().GetSubMeshMaterial( 0 ) );
        view.SetShape( CMaterialView::SHAPE_PLANE );
        ASSERT_TRUE( view.GetPreview().IsLoaded() );
        ASSERT_TRUE( view.ShowMaterial( "TestTextured", group ) );
        view.WriteInspectorData( data = CORE::CDataNode() );
        const CORE::CDataNode* textures = data.FindChild( "Textures" );
        ASSERT_TRUE( GUCEF_NULL != textures && "checker.png" == textures->FindChild( "Slot0" )->GetAttributeValue( "Texture" ).AsString() );
        ASSERT_FALSE( view.ShowMaterial( "NoSuchMaterial", group ) );
        ASSERT_FALSE( view.LoadScript( group, "scripts/nothing.material" ) );

        // the capture path shows the material too
        IMAGE::TPixelMapPtr thumbnail;
        ASSERT_TRUE( capture.CaptureThumbnail( mounter, "avmat/scripts/test.material", 64, thumbnail ) && 64 == thumbnail->GetWidthInPixels() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSceneView( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Scene view: a DotScene file loads, its node tree is listed as folders and files, a selection is drawn and described" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "scene" );
        ASSERT_TRUE( WriteModelFolder( dir ) );
        ASSERT_TRUE( WriteTestFile( dir + "/level/test.scene", BuildTestScene( "model.gltf" ) ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avscene", dir ) );

        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        CSceneView scene( capture.GetStage() );
        CORE::CString relative;
        const CORE::CString group = mounter.GetResourceGroup( "avscene/level/test.scene", relative );
        ASSERT_TRUE( scene.Load( group, relative ) && scene.IsLoaded() );
        ASSERT_TRUE( 4 == scene.GetNodeCount() && 2 == scene.GetObjectCount() );

        // the tree
        CORE::TFileSystemAccessPtr tree = scene.GetFileSystem();
        CORE::TFileSystemIteratorPtr iterator;
        CORE::CFileSystemEntryListIterator::TEntryVector entries;
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == tree->TryGetFileSystemIterator( "/", iterator ) && CORE::CFileSystemEntryListIterator::TryReadAll( *iterator, "/", entries ) );
        ASSERT_TRUE( 2 == entries.size() && HasName( entries, "Parent", true ) && HasName( entries, "Lamp", true ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == tree->TryGetFileSystemIterator( "/Parent", iterator ) && CORE::CFileSystemEntryListIterator::TryReadAll( *iterator, "/Parent", entries ) );
        ASSERT_TRUE( 2 == entries.size() && HasName( entries, "Child", true ) && HasName( entries, "Sibling", true ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == tree->TryGetFileSystemIterator( "/Parent/Child", iterator ) && CORE::CFileSystemEntryListIterator::TryReadAll( *iterator, "/Parent/Child", entries ) );
        ASSERT_TRUE( 1 == entries.size() && HasName( entries, "TheCube.entity", false ) );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == tree->TryGetFileSystemIterator( "/Lamp", iterator ) && CORE::CFileSystemEntryListIterator::TryReadAll( *iterator, "/Lamp", entries ) );
        ASSERT_TRUE( 1 == entries.size() && HasName( entries, "LampLight.light", false ) );

        // the selection: a node, an object, and what is not there
        CViewerStage& stage = capture.GetStage();
        ASSERT_TRUE( scene.Select( "/Parent/Child" ) && "/Parent/Child" == scene.GetSelectedPath() );
        ASSERT_TRUE( stage.GetLineObject( "Selection" )->getNumSections() > 0 );
        CORE::CDataNode data;
        scene.WriteInspectorData( data );
        const CORE::CDataNode* node = data.FindChild( "Node" );
        ASSERT_TRUE( GUCEF_NULL != node && "Child" == node->GetAttributeValue( "Name" ).AsString() && "0, 5, 0" == node->GetAttributeValue( "Position" ).AsString() );
        // the world position has the scale and the position of the parent in it
        ASSERT_TRUE( "1, 12, 3" == node->GetAttributeValue( "WorldPosition" ).AsString() );
        ASSERT_TRUE( scene.Select( "/Parent/Child/TheCube.entity" ) );
        scene.WriteInspectorData( data = CORE::CDataNode() );
        const CORE::CDataNode* object = data.FindChild( "Object" );
        ASSERT_TRUE( GUCEF_NULL != object && "entity" == object->GetAttributeValue( "Kind" ).AsString() && "model.gltf" == object->GetAttributeValue( "Mesh" ).AsString() && "2" == object->GetAttributeValue( "Submeshes" ).AsString() );
        ASSERT_TRUE( scene.Select( "/Lamp/LampLight.light" ) );
        scene.WriteInspectorData( data = CORE::CDataNode() );
        ASSERT_TRUE( "point" == data.FindChild( "Object" )->GetAttributeValue( "LightType" ).AsString() );
        ASSERT_TRUE( scene.Select( "/" ) );
        ASSERT_FALSE( scene.Select( "/Parent/Nothing" ) );
        ASSERT_FALSE( scene.Select( "/Parent/Child/Nothing.entity" ) );

        // the scene frames in the controller and is drawn
        COrbitController::SVector3 center;
        Float32 radius = 0.0f;
        scene.GetBoundingSphere( center, radius );
        ASSERT_TRUE( radius > 0.5f );
        COrbitController controller;
        controller.SetBounds( center, radius );
        capture.GetStage().ApplyController( controller );
        IMAGE::TPixelMapPtr picture;
        ASSERT_TRUE( capture.RenderStage( picture ) );
        DumpPicture( picture, "scene_" + CORE::CString( GetTestRenderer() ) );

        // the whole scene is one asset of the capture
        IMAGE::TPixelMapPtr sheet;
        scene.Unload();
        ASSERT_FALSE( scene.IsLoaded() );
        ASSERT_TRUE( capture.CaptureContactSheet( mounter, "avscene/level/test.scene", sheet ) );

        // a scene file that is not one
        ASSERT_TRUE( WriteTestFile( dir + "/level/bad.scene", "<scene><nodes><node" ) );
        ASSERT_FALSE( scene.Load( group, "level/bad.scene" ) );
        ASSERT_FALSE( scene.Load( group, "level/missing.scene" ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestThumbnails( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Thumbnails: answered later, from the cache when the content is the same, rendered again when it changed, none for what is no asset" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "thumbs" );
        // a cache folder that is not there yet, two levels deep: the provider makes it
        const CORE::CString cacheDir = MakeCleanTestDir( "thumbs_cache" ) + "/nested/cache";
        ASSERT_FALSE( CORE::DirExists( cacheDir ) );
        ASSERT_TRUE( WriteModelFolder( dir ) );
        ASSERT_TRUE( WriteTestFile( dir + "/notes.txt", "not an asset" ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avthumb", dir ) );
        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( 64 ) );

        CThumbnailProvider provider( mounter, capture );
        provider.SetCacheFolder( cacheDir );
        ASSERT_TRUE( CORE::DirExists( cacheDir ) );
        CThumbnailRecorder recorder( &provider, GUI::CIThumbnailProvider::ThumbnailReadyEvent );

        // the name of a cache file has the content, the version of the viewer and the size
        ASSERT_TRUE( CORE::CString( "abc_v" ) + CORE::UInt32ToString( CThumbnailProvider::GetViewerVersion() ) + "_64.png" == CThumbnailProvider::MakeCacheFileName( "abc", 64 ) );
        // the hash is the SHA-256 of the file
        const CORE::CString hash = CThumbnailProvider::HashContent( "/avthumb/model.gltf" );
        ASSERT_TRUE( 64 == hash.Length() && hash == HashOfNativeFile( dir + "/model.gltf" ) );
        ASSERT_TRUE( CThumbnailProvider::HashContent( "/avthumb/missing.gltf" ).IsNULLOrEmpty() );

        // a request does not answer: the answer comes when the application gives the provider its turn
        provider.RequestThumbnail( "/avthumb/model.gltf", 64 );
        provider.RequestThumbnail( "/avthumb/model.gltf", 64 );
        ASSERT_TRUE( 1 == provider.GetQueuedCount() && 0 == recorder.count );
        ASSERT_TRUE( 1 == provider.Update( 100 ) );
        ASSERT_TRUE( 1 == recorder.count && "/avthumb/model.gltf" == recorder.lastPath && 64 == recorder.lastSize && recorder.lastHadPixels );
        ASSERT_TRUE( 1 == provider.GetRenderCount() && 0 == provider.GetCacheHitCount() && 0 == provider.GetQueuedCount() );
        const CORE::CString cacheFile = CORE::CombinePath( cacheDir, CThumbnailProvider::MakeCacheFileName( hash, 64 ) );
        ASSERT_TRUE( CORE::FileExists( cacheFile ) );

        // the same content again: the cache serves it, nothing is rendered
        provider.RequestThumbnail( "/avthumb/model.gltf", 64 );
        ASSERT_TRUE( 1 == provider.Update( 100 ) );
        ASSERT_TRUE( 2 == recorder.count && recorder.lastHadPixels );
        ASSERT_TRUE( 1 == provider.GetRenderCount() && 1 == provider.GetCacheHitCount() );

        // a cache file that is no picture is not trusted: the thumbnail is made again
        ASSERT_TRUE( WriteTestFile( cacheFile, "garbage" ) );
        provider.RequestThumbnail( "/avthumb/model.gltf", 64 );
        ASSERT_TRUE( 1 == provider.Update( 100 ) && recorder.lastHadPixels );
        ASSERT_TRUE( 2 == provider.GetRenderCount() && 1 == provider.GetCacheHitCount() );

        // another size is another thumbnail
        provider.RequestThumbnail( "/avthumb/model.gltf", 32 );
        ASSERT_TRUE( 1 == provider.Update( 100 ) && 32 == recorder.lastSize );
        ASSERT_TRUE( 3 == provider.GetRenderCount() );

        // changed content has another hash and is rendered again
        const UInt32 filesBefore = CountFiles( cacheDir, ".png" );
        ASSERT_TRUE( WriteTestFile( dir + "/model.gltf", BuildTestGltf() + "\n\n" ) );
        ASSERT_TRUE( CThumbnailProvider::HashContent( "/avthumb/model.gltf" ) != hash );
        provider.RequestThumbnail( "/avthumb/model.gltf", 64 );
        ASSERT_TRUE( 1 == provider.Update( 100 ) && recorder.lastHadPixels );
        ASSERT_TRUE( 4 == provider.GetRenderCount() && 1 == provider.GetCacheHitCount() );
        ASSERT_TRUE( CountFiles( cacheDir, ".png" ) == filesBefore + 1 );

        // a picture is a thumbnail too; a text file and a file that is not there are answered with no picture, at once
        provider.RequestThumbnail( "/avthumb/checker.png", 32 );
        ASSERT_TRUE( 1 == provider.Update( 100 ) && recorder.lastHadPixels );
        const UInt32 renders = provider.GetRenderCount();
        provider.RequestThumbnail( "/avthumb/notes.txt", 64 );
        provider.RequestThumbnail( "/avthumb/missing.fbx", 64 );
        ASSERT_TRUE( 2 == provider.GetQueuedCount() );
        const UInt32 failures = provider.GetFailureCount();
        // both are answered in one turn: the text file at once, the file that is not there after a try at a render
        ASSERT_TRUE( 2 == provider.Update( 100 ) && !recorder.lastHadPixels && "/avthumb/missing.fbx" == recorder.lastPath );
        ASSERT_TRUE( renders == provider.GetRenderCount() && failures + 1 == provider.GetFailureCount() );

        // a cancelled request is not answered
        const UInt32 answered = recorder.count;
        provider.RequestThumbnail( "/avthumb/model.gltf", 48 );
        provider.CancelThumbnail( "/avthumb/model.gltf", 48 );
        ASSERT_TRUE( 0 == provider.GetQueuedCount() && 0 == provider.Update( 100 ) && answered == recorder.count );

        // without a cache folder everything is rendered
        provider.SetCacheFolder( CORE::CString::Empty );
        provider.RequestThumbnail( "/avthumb/model.gltf", 64 );
        ASSERT_TRUE( 1 == provider.Update( 100 ) && recorder.lastHadPixels );
        ASSERT_TRUE( 1 == provider.GetCacheHitCount() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

namespace {

/** Appends a 32 bit value, little endian as the binary STL format has it */
void
AppendUInt32( CORE::CDynamicBuffer& buffer, UInt32 value )
{
    const UInt8 bytes[ 4 ] = { (UInt8) ( value & 0xFF ), (UInt8) ( ( value >> 8 ) & 0xFF ), (UInt8) ( ( value >> 16 ) & 0xFF ), (UInt8) ( ( value >> 24 ) & 0xFF ) };
    buffer.Append( bytes, 4 );
}

void
AppendFloat32( CORE::CDynamicBuffer& buffer, Float32 value )
{
    UInt32 bits = 0;
    memcpy( &bits, &value, 4 );
    AppendUInt32( buffer, bits );
}

/** A binary STL of a box from 'low' to 'high', two triangles a side: 12 triangles, 3 vertices each */
bool
WriteBinaryStlBox( const CORE::CString& path, const Float32 low[ 3 ], const Float32 high[ 3 ] )
{
    // the corners by bits: bit 0 picks x, bit 1 y, bit 2 z; each side as two triangles of corner indices
    static const UInt32 sides[ 12 ][ 3 ] =
    {
        { 0, 2, 1 }, { 1, 2, 3 }, { 4, 5, 6 }, { 5, 7, 6 },     // z low, z high
        { 0, 1, 4 }, { 1, 5, 4 }, { 2, 6, 3 }, { 3, 6, 7 },     // y low, y high
        { 0, 4, 2 }, { 2, 4, 6 }, { 1, 3, 5 }, { 3, 7, 5 }      // x low, x high
    };
    CORE::CDynamicBuffer buffer;
    const UInt8 header[ 80 ] = { 0 };
    buffer.Append( header, 80 );
    AppendUInt32( buffer, 12 );
    for ( UInt32 t=0; t<12; ++t )
    {
        for ( UInt32 n=0; n<3; ++n )
            AppendFloat32( buffer, 0.0f );      // the reader makes the normals
        for ( UInt32 v=0; v<3; ++v )
        {
            const UInt32 corner = sides[ t ][ v ];
            AppendFloat32( buffer, ( corner & 1 ) ? high[ 0 ] : low[ 0 ] );
            AppendFloat32( buffer, ( corner & 2 ) ? high[ 1 ] : low[ 1 ] );
            AppendFloat32( buffer, ( corner & 4 ) ? high[ 2 ] : low[ 2 ] );
        }
        const UInt8 attributes[ 2 ] = { 0, 0 };
        buffer.Append( attributes, 2 );
    }
    CORE::CFileAccess file( path, "wb" );
    if ( !file.IsValid() )
        return false;
    return buffer.GetDataSize() == file.Write( buffer.GetBufferPtr(), 1, buffer.GetDataSize() );
}

bool
AddTextEntry( VFS::CIArchiveWriter& writer, const CORE::CString& name, const CORE::CString& text )
{
    CORE::CDynamicBuffer content;
    content.CopyFrom( text.Length(), text.C_String() );
    CORE::CDynamicBufferAccess access( content );
    VFS::CArchiveWriterEntrySettings settings;
    return writer.AddFile( name, access, settings );
}

/** A 3MF package with one object: a tetrahedron 10 wide and 20 high along Z, standing at the origin */
bool
Write3mfTetrahedron( const CORE::CString& path )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CFileAccess output( path, "wb" );
    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
    if ( writer.IsNULL() || !writer->Open( output ) )
        return false;
    const bool written =
        AddTextEntry( *writer, "[Content_Types].xml",
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
            "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
            "<Default Extension=\"model\" ContentType=\"application/vnd.ms-package.3dmanufacturing-3dmodel+xml\"/></Types>\n" ) &&
        AddTextEntry( *writer, "_rels/.rels",
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
            "<Relationship Target=\"/3D/3dmodel.model\" Id=\"rel0\" Type=\"http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel\"/>"
            "</Relationships>\n" ) &&
        AddTextEntry( *writer, "3D/3dmodel.model",
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<model unit=\"millimeter\" xml:lang=\"en-US\" xmlns=\"http://schemas.microsoft.com/3dmanufacturing/core/2015/02\">"
            "<resources><object id=\"1\" type=\"model\"><mesh>"
            "<vertices><vertex x=\"0\" y=\"0\" z=\"0\"/><vertex x=\"10\" y=\"0\" z=\"0\"/><vertex x=\"0\" y=\"10\" z=\"0\"/><vertex x=\"0\" y=\"0\" z=\"20\"/></vertices>"
            "<triangles><triangle v1=\"0\" v2=\"2\" v3=\"1\"/><triangle v1=\"0\" v2=\"1\" v3=\"3\"/><triangle v1=\"1\" v2=\"2\" v3=\"3\"/><triangle v1=\"0\" v2=\"3\" v3=\"2\"/></triangles>"
            "</mesh></object></resources><build><item objectid=\"1\"/></build></model>\n" );
    return writer->Close() && written;
}

bool
Near( Float32 a, Float32 b )
{
    return fabsf( a - b ) < 1e-3f;
}

bool
CenterIs( const CModelView& model, Float32 x, Float32 y, Float32 z )
{
    COrbitController::SVector3 center;
    Float32 radius = 0.0f;
    model.GetBoundingSphere( center, radius );
    return Near( center.x, x ) && Near( center.y, y ) && Near( center.z, z );
}

}

/*-------------------------------------------------------------------------*/

void
TestPrintFormats( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Print formats: the extensions of 3D printing files in any case are known, the others are not" )
    try
    {
        ASSERT_TRUE( CModelView::IsPrintFormat( "stl" ) && CModelView::IsPrintFormat( "STL" ) && CModelView::IsPrintFormat( "3mf" ) && CModelView::IsPrintFormat( "Amf" ) );
        ASSERT_FALSE( CModelView::IsPrintFormat( "fbx" ) || CModelView::IsPrintFormat( "obj" ) || CModelView::IsPrintFormat( "" ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Print formats: a binary STL, an ASCII STL with an upper case extension and a 3MF stand Z up on the grid; the settings turn and move them back; a broken file does not load" )
    try
    {
        if ( !HaveWindow() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        const CORE::CString dir = MakeCleanTestDir( "print" );
        // a box 20 wide (x), 10 deep (y) and 30 high (z), on a build plate at 100, 50
        const Float32 low[ 3 ] = { 100.0f, 50.0f, 0.0f };
        const Float32 high[ 3 ] = { 120.0f, 60.0f, 30.0f };
        ASSERT_TRUE( WriteBinaryStlBox( dir + "/box.stl", low, high ) );
        ASSERT_TRUE( WriteTestFile( dir + "/TRI.STL", "solid tri\nfacet normal 0 0 1\n outer loop\n  vertex 0 0 0\n  vertex 10 0 0\n  vertex 0 10 0\n endloop\nendfacet\nendsolid tri\n" ) );
        ASSERT_TRUE( Write3mfTetrahedron( dir + "/tetra.3mf" ) );
        ASSERT_TRUE( WriteTestFile( dir + "/broken.stl", "not a model" ) );
        ASSERT_TRUE( WriteModelFolder( dir ) );
        CAssetMounter mounter;
        ASSERT_TRUE( mounter.Mount( "avprint", dir ) );

        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        ASSERT_TRUE( capture.Initialize( TileSize ) );
        CViewerStage& stage = capture.GetStage();
        CModelView model( stage );
        CORE::CString relative;

        // the binary STL: three vertices a triangle, Z up and on the grid: its middle over the origin, its foot on the ground
        CORE::CString group = mounter.GetResourceGroup( "/avprint/box.stl", relative );
        ASSERT_TRUE( model.LoadMesh( group, relative ) );
        ASSERT_TRUE( 12 == model.GetStatistics().triangleCount && 36 == model.GetStatistics().vertexCount );
        ASSERT_TRUE( CModelView::UPAXIS_AUTOMATIC == model.GetUpAxis() && CModelView::UPAXIS_Z == model.GetEffectiveUpAxis() );
        ASSERT_TRUE( CModelView::PLACEMENT_ON_GRID == model.GetEffectivePlacement() );
        ASSERT_TRUE( CenterIs( model, 0.0f, 15.0f, 0.0f ) );
        // the content node turns the file's Z into Y: the top of the box is 30 up
        const Ogre::Vector3 top = stage.GetContentNode()->_getFullTransform() * Ogre::Vector3( 110.0f, 55.0f, 30.0f );
        ASSERT_TRUE( Near( top.x, 0.0f ) && Near( top.y, 30.0f ) && Near( top.z, 0.0f ) );

        // the size in the inspector is that of the file: 20 x 10 x 30, and the view says how the model stands
        CORE::CDataNode data;
        model.WriteInspectorData( data );
        const CORE::CDataNode* bounds = data.FindChild( "Bounds" );
        const CORE::CDataNode* view = data.FindChild( "View" );
        ASSERT_TRUE( GUCEF_NULL != bounds && GUCEF_NULL != view );
        if ( GUCEF_NULL != bounds && GUCEF_NULL != view )
        {
            const CORE::CString expected = GUI::CPropertyRowDescriptor::NumberToText( 20.0, 3 ) + " x " + GUI::CPropertyRowDescriptor::NumberToText( 10.0, 3 )
                                           + " x " + GUI::CPropertyRowDescriptor::NumberToText( 30.0, 3 );
            ASSERT_TRUE( expected == bounds->GetAttributeValue( "Size" ).AsString() );
            ASSERT_TRUE( CORE::CString( CModelView::GetUpAxisName( CModelView::UPAXIS_Z ) ) == view->GetAttributeValue( "UpAxis" ).AsString() );
            ASSERT_TRUE( CORE::CString( CModelView::GetPlacementName( CModelView::PLACEMENT_ON_GRID ) ) == view->GetAttributeValue( "Placement" ).AsString() );
        }

        // the settings: Y up as authored is the file as it is, Z up as authored turns it about X only
        model.SetUpAxis( CModelView::UPAXIS_Y );
        model.SetPlacement( CModelView::PLACEMENT_AS_AUTHORED );
        ASSERT_TRUE( CenterIs( model, 110.0f, 55.0f, 15.0f ) );
        ASSERT_TRUE( stage.GetContentNode()->getOrientation() == Ogre::Quaternion::IDENTITY && stage.GetContentNode()->getPosition() == Ogre::Vector3::ZERO );
        model.SetUpAxis( CModelView::UPAXIS_Z );
        ASSERT_TRUE( CenterIs( model, 110.0f, 15.0f, -55.0f ) );
        // Y up on the grid: the file's lowest Y on the ground
        model.SetUpAxis( CModelView::UPAXIS_Y );
        model.SetPlacement( CModelView::PLACEMENT_ON_GRID );
        ASSERT_TRUE( CenterIs( model, 0.0f, 5.0f, 0.0f ) );
        model.SetUpAxis( CModelView::UPAXIS_AUTOMATIC );
        model.SetPlacement( CModelView::PLACEMENT_AUTOMATIC );

        // the ASCII STL of the upper case name: a flat triangle that lies on the ground
        group = mounter.GetResourceGroup( "/avprint/TRI.STL", relative );
        ASSERT_TRUE( model.LoadMesh( group, relative ) );
        ASSERT_TRUE( 1 == model.GetStatistics().triangleCount && CModelView::UPAXIS_Z == model.GetEffectiveUpAxis() );
        ASSERT_TRUE( CenterIs( model, 0.0f, 0.0f, 0.0f ) );

        // the 3MF, a zip that the reader seeks in from its end: four triangles, 20 high
        group = mounter.GetResourceGroup( "/avprint/tetra.3mf", relative );
        ASSERT_TRUE( model.LoadMesh( group, relative ) );
        ASSERT_TRUE( 4 == model.GetStatistics().triangleCount && CModelView::UPAXIS_Z == model.GetEffectiveUpAxis() );
        ASSERT_TRUE( CenterIs( model, 0.0f, 10.0f, 0.0f ) );

        // a glTF is shown as authored and Y up, from the coordinates of the stage
        group = mounter.GetResourceGroup( "/avprint/model.gltf", relative );
        ASSERT_TRUE( model.LoadMesh( group, relative ) );
        ASSERT_TRUE( CModelView::UPAXIS_Y == model.GetEffectiveUpAxis() && CModelView::PLACEMENT_AS_AUTHORED == model.GetEffectivePlacement() );
        ASSERT_TRUE( stage.GetContentNode()->getOrientation() == Ogre::Quaternion::IDENTITY && stage.GetContentNode()->getPosition() == Ogre::Vector3::ZERO );

        // a file the importer cannot read is a failure, not an empty model, and leaves the stage as it was
        group = mounter.GetResourceGroup( "/avprint/broken.stl", relative );
        ASSERT_FALSE( model.LoadMesh( group, relative ) );
        ASSERT_FALSE( model.IsLoaded() );
        ASSERT_TRUE( stage.GetContentNode()->getOrientation() == Ogre::Quaternion::IDENTITY && stage.GetContentNode()->getPosition() == Ogre::Vector3::ZERO );

        // a Z up model that is let go of leaves the stage in its own coordinates
        group = mounter.GetResourceGroup( "/avprint/box.stl", relative );
        ASSERT_TRUE( model.LoadMesh( group, relative ) );
        ASSERT_FALSE( stage.GetContentNode()->getOrientation() == Ogre::Quaternion::IDENTITY );
        model.Unload();
        ASSERT_TRUE( stage.GetContentNode()->getOrientation() == Ogre::Quaternion::IDENTITY && stage.GetContentNode()->getPosition() == Ogre::Vector3::ZERO );

        // the contact sheet of the box: the middle of the front view is the box, lit (its file has zero normals, which the reader
        // makes from the facets: with zero normals it would be black)
        IMAGE::TPixelMapPtr sheet;
        ASSERT_TRUE( capture.CaptureContactSheet( mounter, "/avprint/box.stl", sheet ) );
        DumpPicture( sheet, "print_box_" + CORE::CString( GetTestRenderer() ) );
        UInt32 originX = 0, originY = 0;
        CAssetCapture::GetTileOrigin( TileSize, 0, originX, originY );
        UInt8 middle[ 4 ];
        GetPixel( sheet, (Int32) ( originX + TileSize / 2 ), (Int32) ( originY + TileSize / 2 ), middle );
        ASSERT_TRUE( (UInt32) middle[ 0 ] + middle[ 1 ] + middle[ 2 ] > 3 * 90 );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

void
PerformAssetViewerRenderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetViewerLib.Render" );

    TestStageAndCapture();
    TestStageResize();
    TestModelView();
    TestTextureView();
    TestMaterialView();
    TestSceneView();
    TestThumbnails();
    TestPrintFormats();

    if ( Ogre::ResourceGroupManager::getSingletonPtr() && Ogre::ResourceGroupManager::getSingleton().resourceGroupExists( TestGroup ) )
        Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup( TestGroup );
}

/*-------------------------------------------------------------------------*/
