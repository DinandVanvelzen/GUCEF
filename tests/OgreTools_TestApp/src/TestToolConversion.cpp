/*
 *  OgreTools_TestApp: Test application for the Ogre asset tools and plugins (OgreAssimpConverter, OgreXMLConverter,
 *  OgreMeshUpgrader, OgreCodec_Assimp, OgrePlugin_DotScene)
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

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "Ogre.h"
#include "OgreMeshSerializer.h"

#include "TestOgreToolsSupport.h"
#include "TestGltfAsset.h"
#include "TestOgreEnv.h"

#include "TestToolConversion.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

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

static CORE::CString g_gltfDir;
static CORE::CString g_convDir;
static CORE::CString g_xmlDir;
static CORE::CString g_roundTripDir;

static const Ogre::String ConvGroup = "OgreToolsConverted";
static const Ogre::String RoundTripGroup = "OgreToolsRoundTrip";
static const CORE::CString ConvMountPath = "/ogretools_converted";

/*-------------------------------------------------------------------------*/

CORE::CString
GetConvertedDir( void )
{
    return g_convDir;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetGltfDir( void )
{
    return g_gltfDir;
}

/*-------------------------------------------------------------------------*/

static CORE::CProcess::TStringVector
Args( const char* a = GUCEF_NULL, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL, const char* d = GUCEF_NULL,
      const char* e = GUCEF_NULL, const char* f = GUCEF_NULL, const char* g = GUCEF_NULL, const char* h = GUCEF_NULL,
      const char* i = GUCEF_NULL, const char* j = GUCEF_NULL, const char* k = GUCEF_NULL, const char* l = GUCEF_NULL )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    const char* all[ 12 ] = { a, b, c, d, e, f, g, h, i, j, k, l };
    for ( int n=0; n<12; ++n )
        if ( GUCEF_NULL != all[ n ] )
            args.push_back( all[ n ] );
    return args;
}

/*-------------------------------------------------------------------------*/

/** Runs a tool and logs what it said when it failed, so that a failing test says why */
static bool
RunToolOk( const CORE::CString& tool, const CORE::CProcess::TStringVector& args, const CORE::CString& workDir )
{GUCEF_TRACE;

    Int32 exitCode = -1;
    CORE::CString output;
    const bool ran = RunTool( tool, args, workDir, exitCode, output );
    if ( !ran || 0 != exitCode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RunToolOk: " << tool << " ran=" << ran << " exitCode=" << exitCode << " output: " << output );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
TestWriteAsset( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "glTF asset: the test writes a textured cube with a skinned strip, and the cube alone" )
        try
        {
            g_gltfDir = MakeCleanTestDir( "gltf" );
            ASSERT_TRUE( WriteTestFile( g_gltfDir + "/ogretest.gltf", BuildTestGltf( true ) ) );
            ASSERT_TRUE( WriteTestFile( g_gltfDir + "/cubeonly.gltf", BuildTestGltf( false ) ) );
            ASSERT_TRUE( GetTestFileSize( g_gltfDir + "/ogretest.gltf" ) > GetTestFileSize( g_gltfDir + "/cubeonly.gltf" ) );
            ASSERT_TRUE( GetTestFileSize( g_gltfDir + "/cubeonly.gltf" ) > 1000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestAssimpConverter( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OgreAssimpConverter: converts the glTF to .mesh, .skeleton and .material" )
        try
        {
            g_convDir = MakeCleanTestDir( "converted" );
            ASSERT_TRUE( !FindToolExecutable( "OgreAssimpConverter" ).IsNULLOrEmpty() );

            const CORE::CString log = g_convDir + "/OgreAssimp.log";
            ASSERT_TRUE( RunToolOk( "OgreAssimpConverter", Args( "-q", "-log", log.C_String(), ( g_gltfDir + "/ogretest.gltf" ).C_String(), g_convDir.C_String() ), g_convDir ) );
            ASSERT_TRUE( GetTestFileSize( g_convDir + "/ogretest.mesh" ) > 0 );
            ASSERT_TRUE( GetTestFileSize( g_convDir + "/ogretest.skeleton" ) > 0 );
            ASSERT_TRUE( GetTestFileSize( g_convDir + "/ogretest.material" ) > 0 );

            // the cube alone has no skeleton
            ASSERT_TRUE( RunToolOk( "OgreAssimpConverter", Args( "-q", "-log", log.C_String(), ( g_gltfDir + "/cubeonly.gltf" ).C_String(), g_convDir.C_String() ), g_convDir ) );
            ASSERT_TRUE( GetTestFileSize( g_convDir + "/cubeonly.mesh" ) > 0 );
            ASSERT_FALSE( GetTestFileSize( g_convDir + "/cubeonly.skeleton" ) > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestConvertedMesh( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OgreAssimpConverter result: Ogre loads the mesh with its skeleton, with the sub meshes, vertices, bones and animation of the asset" )
        try
        {
            ASSERT_TRUE( AddVfsResourceLocation( ConvGroup, g_convDir, ConvMountPath ) );
            SMeshFacts facts;
            ASSERT_TRUE( LoadMeshFacts( "ogretest.mesh", ConvGroup, facts ) );
            CheckTestAssetFacts( facts );
            // no levels of detail yet (the base level only); the converter has Assimp make the tangents
            ASSERT_TRUE( 1 == facts.lodLevelCount );
            ASSERT_TRUE( facts.allSubMeshesHaveTangents );

            SMeshFacts cube;
            ASSERT_TRUE( LoadMeshFacts( "cubeonly.mesh", ConvGroup, cube ) );
            ASSERT_TRUE( 1 == cube.subMeshCount );
            ASSERT_TRUE( TestGltfCubeVertexCount == cube.vertexCounts[ 0 ] );
            ASSERT_FALSE( cube.hasSkeleton );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestXmlRoundTrip( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OgreXMLConverter: mesh and skeleton to XML and back, the result has the facts of the original" )
        try
        {
            g_xmlDir = MakeCleanTestDir( "xml" );
            g_roundTripDir = MakeCleanTestDir( "roundtrip" );
            ASSERT_TRUE( !FindToolExecutable( "OgreXMLConverter" ).IsNULLOrEmpty() );
            const CORE::CString log = g_xmlDir + "/OgreXMLConverter.log";

            ASSERT_TRUE( RunToolOk( "OgreXMLConverter", Args( "-q", "-log", log.C_String(), ( g_convDir + "/ogretest.mesh" ).C_String(), ( g_xmlDir + "/ogretest.mesh.xml" ).C_String() ), g_xmlDir ) );
            ASSERT_TRUE( RunToolOk( "OgreXMLConverter", Args( "-q", "-log", log.C_String(), ( g_convDir + "/ogretest.skeleton" ).C_String(), ( g_xmlDir + "/ogretest.skeleton.xml" ).C_String() ), g_xmlDir ) );
            ASSERT_TRUE( GetTestFileSize( g_xmlDir + "/ogretest.mesh.xml" ) > 0 );
            ASSERT_TRUE( GetTestFileSize( g_xmlDir + "/ogretest.skeleton.xml" ) > 0 );

            ASSERT_TRUE( RunToolOk( "OgreXMLConverter", Args( "-q", "-log", log.C_String(), ( g_xmlDir + "/ogretest.mesh.xml" ).C_String(), ( g_roundTripDir + "/ogretest.mesh" ).C_String() ), g_xmlDir ) );
            ASSERT_TRUE( RunToolOk( "OgreXMLConverter", Args( "-q", "-log", log.C_String(), ( g_xmlDir + "/ogretest.skeleton.xml" ).C_String(), ( g_roundTripDir + "/ogretest.skeleton" ).C_String() ), g_xmlDir ) );
            ASSERT_TRUE( GetTestFileSize( g_roundTripDir + "/ogretest.mesh" ) > 0 );
            ASSERT_TRUE( GetTestFileSize( g_roundTripDir + "/ogretest.skeleton" ) > 0 );

            ASSERT_TRUE( AddVfsResourceLocation( RoundTripGroup, g_roundTripDir, "/ogretools_roundtrip" ) );
            SMeshFacts facts;
            ASSERT_TRUE( LoadMeshFacts( "ogretest.mesh", RoundTripGroup, facts ) );
            CheckTestAssetFacts( facts );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestMeshUpgrader( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OgreMeshUpgrader: makes levels of detail by number of levels and by percentage, keeps the skeleton, and makes tangents" )
        try
        {
            ASSERT_TRUE( !FindToolExecutable( "OgreMeshUpgrader" ).IsNULLOrEmpty() );
            const CORE::CString log = g_convDir + "/OgreMeshUpgrader.log";

            // The skinned mesh with 2 levels of detail, each 30 percent less triangles than the one before
            ASSERT_TRUE( RunToolOk( "OgreMeshUpgrader", Args( "-log", log.C_String(), "-l", "2", "-p", "30", "-d", "50",
                                                                ( g_convDir + "/ogretest.mesh" ).C_String(), ( g_convDir + "/ogretest_lod.mesh" ).C_String() ), g_convDir ) );
            ASSERT_TRUE( GetTestFileSize( g_convDir + "/ogretest_lod.mesh" ) > GetTestFileSize( g_convDir + "/ogretest.mesh" ) );

            // A mesh without tangents: the cube as the codec of OgreCodec_Assimp loads it, written by Ogre. The codec does
            // not have Assimp make tangents, the converter does
            ASSERT_TRUE( AddVfsResourceLocation( "OgreToolsCubeSource", GetGltfDir(), "/ogretools_cubesource" ) );
            {
                Ogre::MeshPtr cube = Ogre::MeshManager::getSingleton().load( "cubeonly.gltf", "OgreToolsCubeSource" );
                ASSERT_TRUE( (bool) cube );
                if ( cube )
                {
                    Ogre::MeshSerializer serializer;
                    serializer.exportMesh( cube.get(), ( g_convDir + "/cubeplain.mesh" ).STL_String() );
                }
            }

            // Levels of detail by percentage and tangents, the options of the tool for it
            ASSERT_TRUE( RunToolOk( "OgreMeshUpgrader", Args( "-log", log.C_String(), "-l", "2", "-p", "30", "-d", "50", "-t",
                                                                ( g_convDir + "/cubeplain.mesh" ).C_String(), ( g_convDir + "/cubeplain_up.mesh" ).C_String() ), g_convDir ) );
            // and the same without any option: the mesh stays as it is
            ASSERT_TRUE( RunToolOk( "OgreMeshUpgrader", Args( "-log", log.C_String(), ( g_convDir + "/cubeplain.mesh" ).C_String(), ( g_convDir + "/cubeplain_same.mesh" ).C_String() ), g_convDir ) );

            // the files are new to the VFS location of the converted files: it lists them again
            ASSERT_TRUE( RefreshVfsResourceLocation( ConvGroup, ConvMountPath ) );

            SMeshFacts lod;
            ASSERT_TRUE( LoadMeshFacts( "ogretest_lod.mesh", ConvGroup, lod ) );
            ASSERT_TRUE( 2 == lod.subMeshCount );
            ASSERT_TRUE( 3 == lod.lodLevelCount );    // the base level and the 2 generated ones
            ASSERT_TRUE( lod.hasSkeleton );
            ASSERT_TRUE( TestGltfSkeletonBoneCount == lod.boneCount );
            ASSERT_TRUE( "Wave" == lod.firstAnimationName );

            SMeshFacts plain;
            ASSERT_TRUE( LoadMeshFacts( "cubeplain.mesh", ConvGroup, plain ) );
            ASSERT_FALSE( plain.allSubMeshesHaveTangents );
            ASSERT_TRUE( 1 == plain.lodLevelCount );

            SMeshFacts same;
            ASSERT_TRUE( LoadMeshFacts( "cubeplain_same.mesh", ConvGroup, same ) );
            ASSERT_FALSE( same.allSubMeshesHaveTangents );
            ASSERT_TRUE( 1 == same.lodLevelCount );
            ASSERT_TRUE( plain.vertexCounts[ 0 ] == same.vertexCounts[ 0 ] && plain.triangleCounts[ 0 ] == same.triangleCounts[ 0 ] );

            SMeshFacts upgraded;
            ASSERT_TRUE( LoadMeshFacts( "cubeplain_up.mesh", ConvGroup, upgraded ) );
            ASSERT_TRUE( 1 == upgraded.subMeshCount );
            ASSERT_TRUE( 3 == upgraded.lodLevelCount );
            ASSERT_TRUE( upgraded.allSubMeshesHaveTangents );
            ASSERT_TRUE( upgraded.allSubMeshesHaveUVs );
            // tangents that are split at mirrored UVs or rotated bases can add vertices, never take them away
            ASSERT_TRUE( upgraded.vertexCounts[ 0 ] >= TestGltfCubeVertexCount );
            ASSERT_TRUE( TestGltfCubeTriangleCount == upgraded.triangleCounts[ 0 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformToolConversionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "OgreTools.ToolConversion" );

    TestWriteAsset();
    TestAssimpConverter();
    TestConvertedMesh();
    TestXmlRoundTrip();
    TestMeshUpgrader();
}

/*-------------------------------------------------------------------------*/
