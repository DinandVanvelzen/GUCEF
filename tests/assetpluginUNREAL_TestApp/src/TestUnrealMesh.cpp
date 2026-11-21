/*
 *  assetpluginUNREAL_TestApp: Tests of the skeletal mesh reader and the mesh and material importer
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
#include <algorithm>
#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define GUCEF_CORE_ASSETCONVERTERLIB_H
#endif /* GUCEF_CORE_ASSETCONVERTERLIB_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H
#include "assetpluginUNREAL_CUnrealMaterialMapper.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H
#include "assetpluginUNREAL_CUnrealMaterialInstanceReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#include "assetpluginUNREAL_CUnrealObjectPathResolver.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#include "assetpluginUNREAL_CUnrealImporter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H
#include "assetpluginUNREAL_CUnrealSkeletalMeshReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H
#include "assetpluginUNREAL_CUnrealSkeletalMeshConverter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#ifndef GUCEF_CORE_TESTUNREALWRITER_H
#include "TestUnrealWriter.h"
#define GUCEF_CORE_TESTUNREALWRITER_H
#endif /* GUCEF_CORE_TESTUNREALWRITER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#include "assetpluginUNREAL_CUnrealMetaHumanCharacter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H
#include "assetpluginUNREAL_CUnrealCharacterAssembler.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H ? */

#include "TestUnrealMesh.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;
using namespace GUCEF::ASSETCONVERTER;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The combination of fields that differ between engine releases */
struct SLayout
{
    bool doubles;
    UInt32 materialSlotWords;
    bool rayTracingFlag;
    bool weights16;
    bool modelStripFlags;
};

/*-------------------------------------------------------------------------*/

static void
WriteVertex( CTestByteWriter& w, UInt32 section, UInt32 i, bool weights16 )
{
    const Float32 base = (Float32) ( section * 10 + i );
    w.F32( base ); w.F32( base * 2.0f ); w.F32( base * 3.0f );                  // position
    w.F32( 1.0f ); w.F32( 0.0f ); w.F32( 0.0f );                                // tangent x
    w.F32( 0.0f ); w.F32( 1.0f ); w.F32( 0.0f );                                // tangent y
    w.F32( 0.0f ); w.F32( 0.0f ); w.F32( 1.0f ); w.F32( 0 == i % 2 ? 1.0f : -1.0f );     // tangent z and the sign
    for ( UInt32 u=0; u<4; ++u )
    {
        w.F32( 0 == u ? (Float32) i * 0.25f : 0.0f );
        w.F32( 0 == u ? 0.5f : 0.0f );
    }
    w.U8( 30 ); w.U8( 20 ); w.U8( (UInt8) ( 10 * i ) ); w.U8( 255 );          // B, G, R, A
    for ( UInt32 b=0; b<12; ++b )
        w.U16( b < 2 ? (UInt16) b : 0 );
    for ( UInt32 b=0; b<12; ++b )
    {
        const UInt16 weight = 0 == b ? 40000 : ( 1 == b ? 25535 : 0 );
        if ( weights16 ) w.U16( weight ); else w.U8( (UInt8) ( weight / 257 ) );
    }
}

/*-------------------------------------------------------------------------*/

/** One section: 'vertices' vertices from 'baseVertex', 'triangles' triangles from index 'baseIndex' */
static void
WriteSection( CTestByteWriter& w, const SLayout& layout, UInt32 sectionId, UInt32 material, UInt32 baseIndex, UInt32 triangles, UInt32 baseVertex, UInt32 vertices, UInt32 influences, UInt32 boneMapCount )
{
    w.U8( 0 ); w.U8( 0 );                           // strip flags
    w.U16( (UInt16) material );
    w.U32( baseIndex );
    w.U32( triangles );
    w.Bool32( false );                              // recompute tangents
    w.U8( 3 );                                      // colour channel
    w.Bool32( true );                               // cast shadow
    if ( layout.rayTracingFlag ) w.Bool32( true );
    w.U32( baseVertex );
    w.I32( (Int32) vertices );
    for ( UInt32 i=0; i<vertices; ++i )
        WriteVertex( w, sectionId, i, layout.weights16 );
    w.Bool32( false );                              // 16 bit bone indices
    w.I32( (Int32) boneMapCount );
    for ( UInt32 b=0; b<boneMapCount; ++b )
        w.U16( (UInt16) ( 1 == boneMapCount ? 0 : ( b + 1 ) % 3 ) );
    w.I32( (Int32) vertices );
    w.I32( (Int32) influences );
    w.I32( 0 );                                     // cloth mapping data
    w.U16( 0xFFFF );
    w.Zero( 16 );
    w.I32( -1 );
    w.I32( 1 );                                     // overlapping vertices: one entry
    w.I32( 0 ); w.I32( 2 ); w.I32( 0 ); w.I32( 1 );
    w.Bool32( false );                              // disabled
    w.I32( -1 ); w.I32( 0 ); w.I32( -1 );
}

/*-------------------------------------------------------------------------*/

static void
WriteLod( CTestByteWriter& w, const SLayout& layout, bool second )
{
    w.U8( 0 ); w.U8( 0 );                           // strip flags
    if ( !second )
    {
        w.I32( 2 );
        WriteSection( w, layout, 0, 0, 0, 2, 0, 4, 2, 2 );
        WriteSection( w, layout, 1, 1, 6, 1, 4, 3, 1, 1 );
    }
    else
    {
        w.I32( 1 );
        WriteSection( w, layout, 0, 0, 0, 1, 0, 3, 2, 3 );
    }
    // the user data of the sections
    const UInt32 sections = second ? 1 : 2;
    w.I32( (Int32) sections );
    for ( UInt32 s=0; s<sections; ++s )
    {
        w.I32( (Int32) s );
        w.U8( 0 ); w.U8( 0 ); w.Bool32( false ); w.U8( 3 ); w.Bool32( true );
        if ( layout.rayTracingFlag ) w.Bool32( true );
        w.Bool32( false ); w.I32( -1 ); w.U16( 0xFFFF ); w.Zero( 16 ); w.I32( -1 );
    }
    // the indices
    if ( !second )
    {
        const UInt32 indices[] = { 0, 1, 2, 2, 1, 3, 4, 5, 6 };
        w.I32( 9 );
        for ( UInt32 i=0; i<9; ++i ) w.U32( indices[ i ] );
    }
    else
    {
        w.I32( 3 ); w.U32( 0 ); w.U32( 1 ); w.U32( 2 );
    }
    w.I32( 2 ); w.U16( 0 ); w.U16( 1 );             // active bones
    w.I32( 0 );                                     // imported mesh infos
    w.I32( 0 );                                     // size
    w.I32( second ? 3 : 7 );                        // vertex count
    w.I32( 3 ); w.U16( 0 ); w.U16( 1 ); w.U16( 2 ); // required bones
    w.I32( 1 ); w.I32( 0 );                         // raw point indices
    w.Str( "A1F7003559E431EB83122131C8550394" );
    w.Bool32( true ); w.Bool32( false );
    w.I32( 1 ); w.I32( 0 ); w.I32( 0 );             // vertex to imported vertex map and the highest imported vertex
    w.I32( 1 );                                     // UV sets
    w.I32( 0 );                                     // skin weight profiles
}

/*-------------------------------------------------------------------------*/

static void
WriteSkeletalMeshNative( CTestByteWriter& w, CTestNames& names, const SLayout& layout, bool exportNames )
{
    w.Bool32( false );                              // no object guid
    w.U8( 0 ); w.U8( 0 );                           // strip flags
    const Float64 bounds[] = { 1.0, 2.0, 3.0, 40.0, 50.0, 60.0, 90.0 };
    for ( UInt32 i=0; i<7; ++i )
    {
        if ( layout.doubles ) w.F64( bounds[ i ] ); else w.F32( (Float32) bounds[ i ] );
    }
    w.I32( 2 );                                     // material slots
    const char* slotNames[] = { "body_mat", "head_mat" };
    for ( UInt32 m=0; m<2; ++m )
    {
        w.I32( -3 );
        names.WriteName( w, slotNames[ m ] );
        w.Bool32( true );
        names.WriteName( w, CORE::CString( slotNames[ m ] ) + "_imported" );
        // the texture streaming data: initialised, density override (older), four UV densities, and a word that newer releases add
        w.Bool32( true );
        if ( layout.materialSlotWords >= 6 ) w.Bool32( false );
        w.F32( 1.5f ); w.F32( 0.0f ); w.F32( 0.0f ); w.F32( 0.0f );
        if ( layout.materialSlotWords >= 7 ) w.I32( 0 );
    }
    // the reference skeleton: root, spine below it, head below the spine
    const char* boneNames[] = { "root", "spine_01", "head" };
    w.I32( 3 );
    for ( UInt32 b=0; b<3; ++b )
    {
        names.WriteName( w, boneNames[ b ] );
        w.I32( (Int32) b - 1 );
        if ( exportNames ) w.Str( boneNames[ b ] );
    }
    w.I32( 3 );
    for ( UInt32 b=0; b<3; ++b )
    {
        const Float64 pose[] = { 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, (Float64) b * 10.0, 1.0, 1.0, 1.0 };
        for ( UInt32 i=0; i<10; ++i )
        {
            if ( layout.doubles ) w.F64( pose[ i ] ); else w.F32( (Float32) pose[ i ] );
        }
    }
    w.I32( 3 );
    for ( UInt32 b=0; b<3; ++b ) { names.WriteName( w, boneNames[ b ] ); w.I32( (Int32) b ); }

    // the model
    if ( layout.modelStripFlags ) { w.U8( 0 ); w.U8( 0 ); }
    w.I32( 2 );
    WriteLod( w, layout, false );
    WriteLod( w, layout, true );
    w.Zero( 40 );                                   // the model guid and what follows: not read
}

/*-------------------------------------------------------------------------*/

static void
BuildMeshPackage( Int32 legacy, Int32 ue4, Int32 ue5, const SLayout& layout, GUCEF::vector< UInt8 >& bytes )
{
    CTestPackageWriter writer( legacy, ue4, ue5 );
    const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/Engine" );
    writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "SkeletalMesh" );
    CTestByteWriter body;
    writer.BeginBody( body );
    CTestPropertyWriter props( body, writer.names, writer.ue4, writer.ue5 );
    props.None();
    const Int64 scriptEnd = (Int64) body.Size();
    WriteSkeletalMeshNative( body, writer.names, layout, true );
    writer.AddExport( -2, 0, "SK_Test", body, scriptEnd );
    writer.Build( bytes );
}

/*-------------------------------------------------------------------------*/

static bool
CheckMesh( const CUnrealSkeletalMesh& mesh, const SLayout& layout )
{
    if ( mesh.name != "SK_Test" || mesh.materialSlots.size() != 2 || mesh.skeleton.GetBones().size() != 3 || mesh.lods.size() != 2 )
        return false;
    if ( mesh.materialSlots[ 1 ].slotName != "head_mat" || mesh.materialSlots[ 0 ].importedSlotName != "body_mat_imported" )
        return false;
    if ( mesh.boundsOrigin[ 0 ] != 1.0 || mesh.boundsExtent[ 2 ] != 60.0 || mesh.boundsRadius != 90.0 )
        return false;
    const CUnrealReferenceSkeleton::TBoneVector& bones = mesh.skeleton.GetBones();
    if ( bones[ 0 ].name != "root" || bones[ 2 ].parent != 1 || bones[ 2 ].translation[ 2 ] != 20.0 || bones[ 1 ].rotation[ 3 ] != 1.0 || mesh.skeleton.FindBone( "head" ) != 2 )
        return false;
    const CUnrealSkeletalMesh::SLod& lod0 = mesh.lods[ 0 ];
    if ( lod0.sections.size() != 2 || lod0.indices.size() != 9 || lod0.indices[ 8 ] != 6 || lod0.requiredBones.size() != 3 || lod0.activeBones.size() != 2 )
        return false;
    const CUnrealSkeletalMesh::SSection& s1 = lod0.sections[ 1 ];
    if ( s1.materialIndex != 1 || s1.baseIndex != 6 || s1.baseVertexIndex != 4 || s1.vertices.size() != 3 || s1.maxInfluences != 1 || s1.boneMap.size() != 1 )
        return false;
    const CUnrealSkeletalMesh::SVertex& v = lod0.sections[ 0 ].vertices[ 3 ];
    if ( v.position[ 0 ] != 3.0f || v.position[ 2 ] != 9.0f || v.tangentZ[ 3 ] != -1.0f || v.uv[ 0 ][ 0 ] != 0.75f || v.colour[ 0 ] != 30 || v.colour[ 2 ] != 30 || v.colour[ 3 ] != 255 )
        return false;
    // 8 bit weights are widened: 155 * 257 is not 40000 but within one step of it
    const Int32 w0 = v.weights[ 0 ];
    if ( layout.weights16 ? 40000 != w0 : abs( w0 - 40000 ) > 257 )
        return false;
    if ( v.bones[ 1 ] != 1 || mesh.lods[ 1 ].sections.size() != 1 || mesh.lods[ 1 ].sections[ 0 ].boneMap.size() != 3 || mesh.lods[ 1 ].uvSetCount != 1 )
        return false;
    return mesh.GetVertexCount( 0 ) == 7 && mesh.GetTriangleCount( 0 ) == 3 && mesh.GetVertexCount( 1 ) == 3;
}

/*-------------------------------------------------------------------------*/

static bool
ReadMeshFromBytes( const GUCEF::vector< UInt8 >& bytes, CUnrealSkeletalMesh& mesh, CORE::CString& problem )
{
    CUnrealPackage package;
    if ( !package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "mesh.uasset" ) )
    {
        problem = package.GetError();
        return false;
    }
    CUnrealSkeletalMeshReader reader;
    const bool ok = package.GetExports().size() > 0 && reader.Read( package, 0, mesh );
    if ( !ok )
        problem = reader.GetError();
    return ok;
}

/*-------------------------------------------------------------------------*/

static UInt32
CountReportItems( const CConversionReport& report, TReportItemKind kind, const CORE::CString& text )
{
    UInt32 count = 0;
    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
        for ( size_t i=0; i<entries[ e ].items.size(); ++i )
            if ( entries[ e ].items[ i ].kind == kind && entries[ e ].items[ i ].message.HasSubstr( text ) >= 0 )
                ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

/** The bounding box of the first LOD of a normalised scene, in metres */
static void
SceneBounds( const CAssetScene& scene, Float32* low, Float32* high )
{
    for ( UInt32 i=0; i<3; ++i ) { low[ i ] = 1e30f; high[ i ] = -1e30f; }
    if ( scene.meshes.empty() )
        return;
    for ( size_t s=0; s<scene.meshes[ 0 ].subMeshes.size(); ++s )
    {
        const CAssetSubMesh& subMesh = scene.meshes[ 0 ].subMeshes[ s ];
        for ( size_t v=0; v<subMesh.positions.size() / 3; ++v )
            for ( UInt32 c=0; c<3; ++c )
            {
                if ( subMesh.positions[ v * 3 + c ] < low[ c ] ) low[ c ] = subMesh.positions[ v * 3 + c ];
                if ( subMesh.positions[ v * 3 + c ] > high[ c ] ) high[ c ] = subMesh.positions[ v * 3 + c ];
            }
    }
}

/*-------------------------------------------------------------------------*/

/** The share of the triangles of the first LOD whose winding agrees with the vertex normals: close to 1 when the winding is right */
static Float32
WindingAgreement( const CAssetScene& scene )
{
    UInt64 agree = 0;
    UInt64 total = 0;
    if ( scene.meshes.empty() )
        return 0.0f;
    for ( size_t s=0; s<scene.meshes[ 0 ].subMeshes.size(); ++s )
    {
        const CAssetSubMesh& m = scene.meshes[ 0 ].subMeshes[ s ];
        for ( size_t t=0; t+2<m.indices.size(); t+=3 )
        {
            const Float32* a = &m.positions[ m.indices[ t ] * 3 ];
            const Float32* b = &m.positions[ m.indices[ t + 1 ] * 3 ];
            const Float32* c = &m.positions[ m.indices[ t + 2 ] * 3 ];
            const Float32 e1[ 3 ] = { b[ 0 ] - a[ 0 ], b[ 1 ] - a[ 1 ], b[ 2 ] - a[ 2 ] };
            const Float32 e2[ 3 ] = { c[ 0 ] - a[ 0 ], c[ 1 ] - a[ 1 ], c[ 2 ] - a[ 2 ] };
            const Float32 n[ 3 ] = { e1[ 1 ] * e2[ 2 ] - e1[ 2 ] * e2[ 1 ], e1[ 2 ] * e2[ 0 ] - e1[ 0 ] * e2[ 2 ], e1[ 0 ] * e2[ 1 ] - e1[ 1 ] * e2[ 0 ] };
            Float32 vertexNormal[ 3 ] = { 0, 0, 0 };
            for ( UInt32 k=0; k<3; ++k )
                for ( UInt32 c3=0; c3<3; ++c3 )
                    vertexNormal[ c3 ] += m.normals[ m.indices[ t + k ] * 3 + c3 ];
            const Float32 dot = n[ 0 ] * vertexNormal[ 0 ] + n[ 1 ] * vertexNormal[ 1 ] + n[ 2 ] * vertexNormal[ 2 ];
            if ( 0.0f == dot )
                continue;
            ++total;
            if ( dot > 0.0f )
                ++agree;
        }
    }
    return total > 0 ? (Float32) agree / (Float32) total : 0.0f;
}

/*-------------------------------------------------------------------------*/

/** Reads the skeletal meshes of a package, converts the first LOD of each and describes it in one line */
static bool
DescribeMeshesOfPackage( const CORE::CString& vfsPath, const CORE::CString& label, CORE::CString& text, UInt32& meshCount, bool checkWinding )
{
    CUnrealPackage package;
    if ( !package.Open( vfsPath ) )
        return false;
    bool ok = true;
    for ( size_t e=0; e<package.GetExports().size(); ++e )
    {
        if ( !package.GetExports()[ e ].outerIndex.IsNull() || "SkeletalMesh" != package.GetClassName( package.GetExports()[ e ] ) )
            continue;
        CUnrealSkeletalMeshReader reader;
        CUnrealSkeletalMesh mesh;
        if ( !reader.Read( package, (UInt32) e, mesh ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Skeletal mesh " << label << " failed: " << reader.GetError() );
            text += label + " | FAILED " + reader.GetError() + "\n";
            ok = false;
            continue;
        }
        ++meshCount;
        CAssetScene scene;
        scene.coordinateSystem = CUnrealSkeletalMeshConverter::GetCoordinateSystem();
        CConversionReport report;
        GUCEF::vector< Int32 > slots( mesh.materialSlots.size(), -1 );
        UInt32 root = 0;
        ok = CUnrealSkeletalMeshConverter::AddToScene( mesh, slots, scene, report, root ) && ok;
        ok = CAssetNormaliser::Normalise( scene, report ) && ok;
        Float32 low[ 3 ];
        Float32 high[ 3 ];
        SceneBounds( scene, low, high );
        const Float32 agreement = WindingAgreement( scene );
        if ( checkWinding && agreement < 0.9f )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Skeletal mesh " << label << ": the winding agrees with the normals for " << agreement << " of the triangles" );
            ok = false;
        }
        CORE::CString screenSizes;
        for ( size_t l=0; l<mesh.lods.size(); ++l )
            screenSizes += ( 0 == l ? "" : "/" ) + CORE::Int32ToString( (Int32) ( mesh.lods[ l ].screenSize * 1000.0f + 0.5f ) );
        UInt32 facial = 0;
        for ( size_t b=0; b<mesh.skeleton.GetBones().size(); ++b )
            if ( mesh.skeleton.GetBones()[ b ].name.HasSubstr( "FACIAL_" ) == 0 )
                ++facial;
        text += label + " | bones " + CORE::UInt64ToString( mesh.skeleton.GetBones().size() ) + " facial " + CORE::UInt32ToString( facial ) + " | slots " + CORE::UInt64ToString( mesh.materialSlots.size() ) + " | lods " + CORE::UInt64ToString( mesh.lods.size() ) +
                " | lod0 vertices " + CORE::UInt32ToString( mesh.GetVertexCount( 0 ) ) + " triangles " + CORE::UInt32ToString( mesh.GetTriangleCount( 0 ) ) + " sections " + CORE::UInt64ToString( mesh.lods[ 0 ].sections.size() ) +
                " uv " + CORE::UInt32ToString( mesh.lods[ 0 ].uvSetCount ) + " | screen sizes " + screenSizes + " | size cm " + CORE::Int32ToString( (Int32) ( ( high[ 0 ] - low[ 0 ] ) * 100.0f + 0.5f ) ) + " x " + CORE::Int32ToString( (Int32) ( ( high[ 1 ] - low[ 1 ] ) * 100.0f + 0.5f ) ) +
                " x " + CORE::Int32ToString( (Int32) ( ( high[ 2 ] - low[ 2 ] ) * 100.0f + 0.5f ) ) + " | " + CORE::CString( agreement > 0.9f ? "winding ok" : "winding check" ) + "\n";
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** One line per material of a mount: the chain and what was resolved */
static void
DescribeMaterial( const CORE::CString& vfsPath, const CUnrealObjectPathResolver& resolver, CORE::CString& text, UInt32& resolvedCount, UInt32& problemCount )
{
    CUnrealResolvedMaterial resolved;
    const bool ok = CUnrealMaterialInstanceReader::Resolve( vfsPath, resolver, resolved );
    text += CORE::ExtractFilename( vfsPath ) + " | " + ( ok ? "ok" : "FAILED" ) + " | chain " + CORE::UInt64ToString( resolved.chain.size() ) + " | master " + ( resolved.masterPath.IsNULLOrEmpty() ? CORE::CString( "none" ) : CORE::ExtractFilename( resolved.masterPath ) ) +
            " | scalars " + CORE::UInt64ToString( resolved.scalars.size() ) + " colours " + CORE::UInt64ToString( resolved.colours.size() ) + " textures " + CORE::UInt64ToString( resolved.textures.size() ) + " switches " + CORE::UInt64ToString( resolved.switches.size() ) +
            " | blend " + resolved.blendMode + " shading " + resolved.shadingModel + " two sided " + ( resolved.twoSided ? "yes" : "no" ) + " | problems " + CORE::UInt64ToString( resolved.problems.size() ) + "\n";
    for ( size_t p=0; p<resolved.problems.size(); ++p )
        text += "    problem: " + resolved.problems[ p ] + "\n";
    if ( ok ) ++resolvedCount;
    problemCount += (UInt32) resolved.problems.size();
}

/*-------------------------------------------------------------------------*/

/** Writes a property tree as indented text: the way to look at what a package holds */
static void
DumpNode( const CORE::CDataNode& node, UInt32 depth, CORE::CString& text )
{
    if ( depth > 10 )
        return;
    CORE::CString value = node.GetValue().AsString();
    if ( value.Length() > 100 )
        value = value.SubstrFromRange( 0, 100 );
    for ( UInt32 pad=0; pad<depth * 2; ++pad )
        text += " ";
    text += node.GetName() + " [" + node.GetAttributeValue( "type" ).AsString() + "]";
    if ( !value.IsNULLOrEmpty() )
        text += " = " + value;
    const CORE::CString path = node.GetAttributeValue( "path" ).AsString();
    if ( !path.IsNULLOrEmpty() )
        text += " path " + path;
    // the values of native structs are attributes
    const char* const components[] = { "default", "R", "G", "B", "A", "X", "Y", "Z", "W" };
    for ( UInt32 c=0; c<sizeof( components ) / sizeof( components[ 0 ] ); ++c )
        if ( GUCEF_NULL != node.GetAttribute( components[ c ] ) )
            text += CORE::CString( " " ) + components[ c ] + "=" + node.GetAttributeValue( components[ c ] ).AsString();
    text += "\n";
    UInt32 shown = 0;
    CORE::CDataNode::const_iterator c = node.ConstBegin();
    while ( c != node.ConstEnd() && shown < 60 )
    {
        DumpNode( **c, depth + 1, text );
        ++c;
        ++shown;
    }
}

/*-------------------------------------------------------------------------*/

/** Dumps the exports of one package of an archive (GUCEF_TEST_UNREAL_DUMP="archive.mhpkg:entry.uasset") to the scratch area; for a MetaHuman character also its payload records, texture list and stored PNG files */
static void
DumpRequestedPackage( void )
{
    const char* request = getenv( "GUCEF_TEST_UNREAL_DUMP" );
    if ( GUCEF_NULL == request || !CorpusEnabled() )
        return;
    const CORE::CString text( request );
    const Int32 colon = text.HasChar( ':' );
    if ( colon <= 0 )
        return;
    const CORE::CString archiveName = text.SubstrFromRange( 0, (UInt32) colon );
    const CORE::CString entry = text.SubstrFromRange( (UInt32) colon + 1, text.Length() );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const bool projectArchive = "project" == archiveName;
    const CORE::CString nativePath = projectArchive ? CorpusSetting( "projectzip" ) : CORE::CombinePath( CorpusSetting( "archivedir" ), archiveName );
    if ( !vfs.MountArchive( "dumpmount", nativePath, projectArchive ? "zip" : "mhpkg", "dumpmount", false, false, nativePath ) )
        return;
    CUnrealPackage package;
    if ( package.Open( "dumpmount/" + entry ) )
    {
        CORE::CString dump;
        for ( size_t i=0; i<package.GetExports().size(); ++i )
        {
            const CUnrealObjectExport& object = package.GetExports()[ i ];
            CUnrealArchiveReader reader;
            CORE::CDataNode root( "root" );
            CUnrealPropertyReader properties( package );
            if ( !package.OpenExportRange( (UInt32) i, reader ) )
                continue;
            properties.ReadExport( reader, root, package.GetClassName( object ) );
            dump += "== export " + CORE::UInt64ToString( i + 1 ) + " " + object.objectName + " (" + package.GetClassName( object ) + ") outer " + CORE::Int32ToString( object.outerIndex.index ) + "\n";
            DumpNode( root, 0, dump );
            if ( "MetaHumanCharacter" == package.GetClassName( object ) )
            {
                CUnrealMetaHumanCharacter character;
                character.Read( package, (UInt32) i );
                for ( size_t r=0; r<character.records.size(); ++r )
                    dump += "record " + CORE::UInt64ToString( r ) + " method " + CORE::Int32ToString( character.records[ r ].method ) + " raw " + CORE::UInt64ToString( character.records[ r ].payload.rawSize ) + " stored " + CORE::UInt64ToString( character.records[ r ].payload.storedSize ) + " png " + CORE::UInt32ToString( character.records[ r ].pngWidth ) + "x" + CORE::UInt32ToString( character.records[ r ].pngHeight ) + "\n";
                for ( size_t r=0; r<character.records.size(); ++r )
                    if ( character.records[ r ].isPng )
                    {
                        CUnrealMetaHumanCharacter::STexture probe;
                        probe.hasPayload = true; probe.isPng = true; probe.payload = character.records[ r ].payload;
                        CORE::CDynamicBuffer png;
                        if ( character.ReadPng( package, probe, png ) )
                        {
                            CORE::CreateDirs( CORE::CombinePath( CORE::CombinePath( HomeDir(), "AssetTests/unreal" ), "out/png" ) );
                            png.WriteContentToFile( CORE::CombinePath( CORE::CombinePath( HomeDir(), "AssetTests/unreal/out/png" ), character.name + "_rec" + CORE::UInt64ToString( r ) + ".png" ), 0, true );
                        }
                    }
                for ( size_t t=0; t<character.textures.size(); ++t )
                    dump += "texture " + character.textures[ t ].group + " " + character.textures[ t ].type + " " + CORE::UInt32ToString( character.textures[ t ].width ) + "x" + CORE::UInt32ToString( character.textures[ t ].height ) + "\n";
            }
            if ( "SkeletalMesh" == package.GetClassName( object ) || "Skeleton" == package.GetClassName( object ) || "MetaHumanCharacter" == package.GetClassName( object ) )
            {
                // the first bytes of the native data, behind the properties
                dump += "native data starts at " + CORE::UInt64ToString( reader.GetPosition() ) + " of " + CORE::Int64ToString( object.serialSize ) + "\n";
                CORE::CDynamicBuffer native;
                const UInt64 left = reader.GetRemaining() < 4 * 1024 * 1024 ? reader.GetRemaining() : 4 * 1024 * 1024;
                native.SetDataSize( (UInt32) left );
                reader.Read( native.GetBufferPtr(), left );
                native.WriteContentToFile( CORE::CombinePath( CORE::CombinePath( CORE::CombinePath( HomeDir(), "AssetTests/unreal" ), "out" ), "native_" + CORE::ExtractFilename( entry ) + ".bin" ), 0, true );
            }
        }
        CORE::CreateDirs( CORE::CombinePath( CORE::CombinePath( HomeDir(), "AssetTests/unreal" ), "out" ) );
        CORE::WriteStringAsTextFile( CORE::CombinePath( CORE::CombinePath( CORE::CombinePath( HomeDir(), "AssetTests/unreal" ), "out" ), "dump_" + CORE::ExtractFilename( entry ) + ".txt" ), dump );
    }
    vfs.UnmountArchiveByName( "dumpmount" );
}

/*-------------------------------------------------------------------------*/

void
PerformUnrealMeshTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Mesh" );

    DumpRequestedPackage();

    GUCEF_TESTFW_TESTCASE( "Mesh: a skeletal mesh is read in every combination of the layout fields that differ between engine releases" )
    try
    {
        struct SVersion { Int32 legacy; Int32 ue4; Int32 ue5; };
        const SVersion versions[] = { { -9, 522, 1018 }, { -8, 522, 1009 }, { -7, 522, 0 } };
        for ( UInt32 v=0; v<3; ++v )
        {
            for ( UInt32 combination=0; combination<24; ++combination )
            {
                SLayout layout;
                layout.doubles = versions[ v ].ue5 >= UE5_LARGE_WORLD_COORDINATES;
                layout.materialSlotWords = 5 + combination % 3;
                layout.rayTracingFlag = 0 != ( ( combination / 3 ) & 1 );
                layout.weights16 = 0 != ( ( combination / 3 ) & 2 );
                layout.modelStripFlags = 0 != ( ( combination / 3 ) & 4 );
                GUCEF::vector< UInt8 > bytes;
                BuildMeshPackage( versions[ v ].legacy, versions[ v ].ue4, versions[ v ].ue5, layout, bytes );
                CUnrealSkeletalMesh mesh;
                CORE::CString problem;
                const bool ok = ReadMeshFromBytes( bytes, mesh, problem );
                if ( !ok )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Mesh of version " << v << ", combination " << combination << ": " << problem );
                ASSERT_TRUE( ok );
                if ( !CheckMesh( mesh, layout ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Mesh of version " << v << ", combination " << combination << " has the wrong content" );
                ASSERT_TRUE( CheckMesh( mesh, layout ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mesh: the conversion makes bone nodes, a skin, a mesh per LOD with counter-clockwise triangles and a LOD group" )
    try
    {
        SLayout layout = { true, 7, true, true, true };
        GUCEF::vector< UInt8 > bytes;
        BuildMeshPackage( -9, 522, 1018, layout, bytes );
        CUnrealSkeletalMesh mesh;
        CORE::CString problem;
        ASSERT_TRUE( ReadMeshFromBytes( bytes, mesh, problem ) );
        mesh.lods[ 0 ].screenSize = 1.0f;
        mesh.lods[ 1 ].screenSize = 0.25f;

        CAssetScene scene;
        CConversionReport report;
        GUCEF::vector< Int32 > slots;
        slots.push_back( 5 );
        slots.push_back( -1 );
        UInt32 rootNode = 0;
        ASSERT_TRUE( CUnrealSkeletalMeshConverter::AddToScene( mesh, slots, scene, report, rootNode ) );
        // the node of the mesh, three bones, and a node per LOD
        ASSERT_TRUE( 6 == scene.nodes.size() && 0 == rootNode );
        ASSERT_TRUE( 1 == scene.skins.size() && 3 == scene.skins[ 0 ].joints.size() && 3 == scene.skins[ 0 ].inverseBindMatrices.size() );
        ASSERT_TRUE( 2 == scene.meshes.size() && 2 == scene.meshes[ 0 ].subMeshes.size() && 1 == scene.meshes[ 1 ].subMeshes.size() );
        ASSERT_TRUE( 1 == scene.lodGroups.size() && 2 == scene.lodGroups[ 0 ].levels.size() && 0.25f == scene.lodGroups[ 0 ].levels[ 1 ].screenRelativeHeight );
        ASSERT_TRUE( scene.nodes[ rootNode ].lodGroup == 0 );
        const CAssetSubMesh& first = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( 5 == first.materialIndex && -1 == scene.meshes[ 0 ].subMeshes[ 1 ].materialIndex );
        ASSERT_TRUE( 4 == first.GetVertexCount() && 2 == first.GetTriangleCount() && 2 == first.influencesPerVertex );
        // the triangle (0,1,2) of the engine is (0,2,1) now
        ASSERT_TRUE( 0 == first.indices[ 0 ] && 2 == first.indices[ 1 ] && 1 == first.indices[ 2 ] );
        // the second section counts its vertices from its own start
        ASSERT_TRUE( 0 == scene.meshes[ 0 ].subMeshes[ 1 ].indices[ 0 ] && 2 == scene.meshes[ 0 ].subMeshes[ 1 ].indices[ 1 ] );
        // joints are bones through the bone map, weights add up to one
        ASSERT_TRUE( 1 == first.joints[ 0 ] && 2 == first.joints[ 1 ] );
        ASSERT_TRUE( fabs( first.weights[ 0 ] + first.weights[ 1 ] - 1.0f ) < 1e-5f && fabs( first.weights[ 0 ] - 40000.0f / 65535.0f ) < 1e-4f );
        ASSERT_TRUE( 4 == first.tangents.size() / first.GetVertexCount() && -1.0f == first.tangents[ 3 * 4 + 3 ] );
        // the vertex colours of a skeletal mesh are masks of the material, not colours: they are not written as COLOR_0 (a glTF consumer
        // would multiply the base colour with them), and the report says so
        ASSERT_TRUE( first.colours.empty() && scene.meshes[ 0 ].subMeshes[ 1 ].colours.empty() );
        ASSERT_TRUE( 1 == CountReportItems( report, REPORTITEM_INFO, "have vertex colours: they are masks for the material of the engine" ) );
        // they are kept as the masks of the vertices: R, G, B, A divided by 255 (vertex 1 of the section is R 10, G 20, B 30, A 255)
        ASSERT_TRUE( 16 == first.masks.size() && 0.0f == first.masks[ 0 ] && 1.0f == first.masks[ 3 ] );
        ASSERT_TRUE( fabs( first.masks[ 4 ] - 10.0f / 255.0f ) < 1e-6f && fabs( first.masks[ 5 ] - 20.0f / 255.0f ) < 1e-6f && fabs( first.masks[ 6 ] - 30.0f / 255.0f ) < 1e-6f && 1.0f == first.masks[ 7 ] );
        // the inverse bind matrix of the head undoes its world position (0,0,0+10+20)
        const MATH::TFloat32Matrix4D world = scene.GetWorldMatrix( scene.skins[ 0 ].joints[ 2 ] );
        const MATH::TFloat32Matrix4D product = world * scene.skins[ 0 ].inverseBindMatrices[ 2 ];
        for ( UInt32 r=0; r<4; ++r )
            for ( UInt32 c=0; c<4; ++c )
                ASSERT_TRUE( fabs( product( r, c ) - ( r == c ? 1.0f : 0.0f ) ) < 1e-4f );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Character: a body and two garments with copies of its skeleton become one skeleton and one skin, the garments skinned by joint name" )
    try
    {
        SLayout layout = { true, 7, true, true, true };
        GUCEF::vector< UInt8 > bytes;
        BuildMeshPackage( -9, 522, 1018, layout, bytes );
        CUnrealSkeletalMesh body, shirt, boots;
        CORE::CString problem;
        ASSERT_TRUE( ReadMeshFromBytes( bytes, body, problem ) && ReadMeshFromBytes( bytes, shirt, problem ) && ReadMeshFromBytes( bytes, boots, problem ) );
        body.name = "SKM_Body";
        shirt.name = "SKM_Shirt";
        boots.name = "SKM_Boots";
        GUCEF::vector< Int32 > slots( 2, 0 );
        CAssetScene scene;
        CConversionReport report;
        CUnrealCharacterAssembler::TPartVector garments;
        garments.push_back( CUnrealCharacterAssembler::SPart( &shirt, &slots ) );
        garments.push_back( CUnrealCharacterAssembler::SPart( &boots, &slots ) );
        CUnrealCharacterAssembler::SResult result;
        ASSERT_TRUE( CUnrealCharacterAssembler::Assemble( "Testy", CUnrealCharacterAssembler::SPart( &body, &slots ), garments, scene, report, result ) );
        // the root node, three bones, and for each of the three meshes a holder with two LOD nodes
        ASSERT_TRUE( 13 == scene.nodes.size() && "Testy" == scene.nodes[ result.rootNode ].name );
        ASSERT_TRUE( 1 == scene.skins.size() && 3 == scene.skins[ 0 ].joints.size() && 3 == result.jointNodes.size() );
        ASSERT_TRUE( 6 == scene.meshes.size() && 3 == scene.lodGroups.size() );
        ASSERT_TRUE( 0 == result.extraJoints && 0 == result.mismatches && 2 == result.garmentHolderNodes.size() );
        UInt32 skinned = 0;
        for ( size_t n=0; n<scene.nodes.size(); ++n )
            if ( scene.nodes[ n ].mesh >= 0 )
            {
                ASSERT_TRUE( 0 == scene.nodes[ n ].skin );
                ++skinned;
            }
        ASSERT_TRUE( 6 == skinned );
        ASSERT_TRUE( 4 == result.garmentMeshFirst[ 0 ] - result.bodyMeshFirst + 2 && 4 == result.garmentMeshFirst[ 1 ] );
        // the garments hang below the root node next to the bones
        ASSERT_TRUE( scene.nodes[ result.garmentHolderNodes[ 0 ] ].parent == (Int32) result.rootNode && scene.nodes[ result.bodyHolderNode ].parent == (Int32) result.rootNode );
        ASSERT_TRUE( 0 == CountReportItems( report, REPORTITEM_WARNING, "bone" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Character: a garment whose bind pose differs from the body is moved into the bind pose of the body, by the weights of its vertices" )
    try
    {
        SLayout layout = { true, 7, true, true, true };
        GUCEF::vector< UInt8 > bytes;
        BuildMeshPackage( -9, 522, 1018, layout, bytes );
        CUnrealSkeletalMesh body, same, raised;
        CORE::CString problem;
        ASSERT_TRUE( ReadMeshFromBytes( bytes, body, problem ) && ReadMeshFromBytes( bytes, same, problem ) && ReadMeshFromBytes( bytes, raised, problem ) );
        body.name = "SKM_Body";
        same.name = "SKM_Shirt";
        raised.name = "SKM_Shirt";
        // the garment skeleton has its spine (and so the head below it) 2 cm higher than the body
        CUnrealReferenceSkeleton::TBoneVector bones = raised.skeleton.GetBones();
        bones[ 1 ].translation[ 2 ] += 2.0;
        raised.skeleton.SetBones( bones );
        GUCEF::vector< Int32 > slots( 2, 0 );

        CAssetScene sameScene;
        CConversionReport sameReport;
        CUnrealCharacterAssembler::TPartVector sameGarments;
        sameGarments.push_back( CUnrealCharacterAssembler::SPart( &same, &slots ) );
        CUnrealCharacterAssembler::SResult sameResult;
        ASSERT_TRUE( CUnrealCharacterAssembler::Assemble( "Testy", CUnrealCharacterAssembler::SPart( &body, &slots ), sameGarments, sameScene, sameReport, sameResult ) );
        ASSERT_TRUE( 0 == CountReportItems( sameReport, REPORTITEM_INFO, "vertices moved from the bind pose of the garment" ) );

        CAssetScene scene;
        CConversionReport report;
        CUnrealCharacterAssembler::TPartVector garments;
        garments.push_back( CUnrealCharacterAssembler::SPart( &raised, &slots ) );
        CUnrealCharacterAssembler::SResult result;
        ASSERT_TRUE( CUnrealCharacterAssembler::Assemble( "Testy", CUnrealCharacterAssembler::SPart( &body, &slots ), garments, scene, report, result ) );
        ASSERT_TRUE( 1 == result.mismatches && 1 == CountReportItems( report, REPORTITEM_INFO, "vertices moved from the bind pose of the garment into that of the body" ) );

        // every vertex moves by the weighted offsets of its joints: -2 cm in z for the spine and the head, nothing for the root
        UInt32 checked = 0;
        UInt32 moved = 0;
        for ( size_t m=result.garmentMeshFirst[ 0 ]; m<scene.meshes.size(); ++m )
        {
            for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
            {
                const CAssetSubMesh& after = scene.meshes[ m ].subMeshes[ s ];
                const CAssetSubMesh& before = sameScene.meshes[ m ].subMeshes[ s ];
                ASSERT_TRUE( after.GetVertexCount() == before.GetVertexCount() );
                for ( UInt32 v=0; v<after.GetVertexCount(); ++v )
                {
                    Float32 offset = 0.0f;
                    Float32 total = 0.0f;
                    for ( UInt32 k=0; k<after.influencesPerVertex; ++k )
                    {
                        const Float32 w = after.weights[ v * after.influencesPerVertex + k ];
                        const CORE::CString& joint = scene.nodes[ scene.skins[ 0 ].joints[ after.joints[ v * after.influencesPerVertex + k ] ] ].name;
                        if ( w > 0.0f && "root" != joint )
                            offset -= 2.0f * w;
                        total += w;
                    }
                    offset /= total > 0.0f ? total : 1.0f;
                    ASSERT_TRUE( fabs( after.positions[ v * 3 ] - before.positions[ v * 3 ] ) < 1e-4f && fabs( after.positions[ v * 3 + 1 ] - before.positions[ v * 3 + 1 ] ) < 1e-4f );
                    ASSERT_TRUE( fabs( after.positions[ v * 3 + 2 ] - ( before.positions[ v * 3 + 2 ] + offset ) ) < 1e-4f );
                    // a translation does not turn the normals
                    ASSERT_TRUE( fabs( after.normals[ v * 3 + 2 ] - before.normals[ v * 3 + 2 ] ) < 1e-5f );
                    ++checked;
                    if ( offset < -1e-3f )
                        ++moved;
                }
            }
        }
        ASSERT_TRUE( checked >= 6 && moved >= 3 );
        // the body itself does not move
        for ( size_t v=0; v<scene.meshes[ 0 ].subMeshes[ 0 ].positions.size(); ++v )
            ASSERT_TRUE( scene.meshes[ 0 ].subMeshes[ 0 ].positions[ v ] == sameScene.meshes[ 0 ].subMeshes[ 0 ].positions[ v ] );
        // the holders say which part is the body and which a garment
        ASSERT_TRUE( "body" == scene.nodes[ result.bodyHolderNode ].extras.GetAttributeValue( "gucefCharacterPart" ).AsString() );
        ASSERT_TRUE( "garment" == scene.nodes[ result.garmentHolderNodes[ 0 ] ].extras.GetAttributeValue( "gucefCharacterPart" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Character: garments whose skeleton differs are skinned by name, extra bones become extra joints and every mismatch is reported" )
    try
    {
        SLayout layout = { true, 7, true, true, true };
        GUCEF::vector< UInt8 > bytes;
        BuildMeshPackage( -9, 522, 1018, layout, bytes );
        CUnrealSkeletalMesh body, cape;
        CORE::CString problem;
        ASSERT_TRUE( ReadMeshFromBytes( bytes, body, problem ) && ReadMeshFromBytes( bytes, cape, problem ) );
        body.name = "SKM_Body";
        cape.name = "SKM_Cape";

        // the cape has the bones in another order (an extra bone in between), the spine at another place, the head below the root and a bone of its own
        CUnrealReferenceSkeleton::TBoneVector bones = body.skeleton.GetBones();
        CUnrealReferenceSkeleton::SBone extra = bones[ 0 ];
        extra.name = "cape_root";
        extra.parent = 0;
        extra.translation[ 2 ] = 7.0;
        CUnrealReferenceSkeleton::TBoneVector capeBones;
        capeBones.push_back( bones[ 0 ] );
        capeBones.push_back( extra );
        bones[ 1 ].parent = 0;
        bones[ 1 ].translation[ 2 ] = 15.0;
        bones[ 2 ].parent = 2;                       // below the spine in the body; here below the extra bone, which stands at 2 after the shift
        capeBones.push_back( bones[ 1 ] );
        capeBones.push_back( bones[ 2 ] );
        capeBones[ 2 ].parent = 0;
        capeBones[ 3 ].parent = 1;                   // the head is below the cape root, not below the spine
        cape.skeleton.SetBones( capeBones );
        ASSERT_TRUE( CUnrealCharacterAssembler::IsCompatible( body.skeleton, cape.skeleton ) );

        UInt32 missing = 0;
        CORE::CString::StringVector texts;
        ASSERT_TRUE( 2 == CUnrealCharacterAssembler::CompareSkeletons( body.skeleton, cape.skeleton, missing, texts ) && 1 == missing && 2 == texts.size() );
        ASSERT_TRUE( texts[ 0 ].HasSubstr( "spine_1" ) >= 0 && texts[ 0 ].HasSubstr( "bind pose" ) >= 0 && texts[ 1 ].HasSubstr( "head" ) >= 0 && texts[ 1 ].HasSubstr( "parent" ) >= 0 );

        GUCEF::vector< Int32 > slots( 2, 0 );
        CAssetScene scene;
        CConversionReport report;
        CUnrealCharacterAssembler::TPartVector garments;
        garments.push_back( CUnrealCharacterAssembler::SPart( &cape, &slots ) );
        CUnrealCharacterAssembler::SResult result;
        ASSERT_TRUE( CUnrealCharacterAssembler::Assemble( "Testy", CUnrealCharacterAssembler::SPart( &body, &slots ), garments, scene, report, result ) );
        ASSERT_TRUE( 1 == result.extraJoints && 2 == result.mismatches && 4 == scene.skins[ 0 ].joints.size() && 4 == scene.skins[ 0 ].inverseBindMatrices.size() );
        ASSERT_TRUE( 2 == CountReportItems( report, REPORTITEM_WARNING, "bone" ) );
        // the extra joint hangs below the root joint of the body, which is its parent in the cape
        ASSERT_TRUE( "cape_root" == scene.nodes[ result.jointNodes[ 3 ] ].name && scene.nodes[ result.jointNodes[ 3 ] ].parent == (Int32) result.jointNodes[ 0 ] );
        ASSERT_TRUE( 7.0f == scene.nodes[ result.jointNodes[ 3 ] ].transform.position.z );
        // every influence of the cape points to the joint of the body bone with the name of the bone of the cape
        const ASSETCONVERTER::CAssetMesh& capeMesh = scene.meshes[ result.garmentMeshFirst[ 0 ] ];
        UInt32 checked = 0;
        for ( size_t sec=0; sec<cape.lods[ 0 ].sections.size(); ++sec )
        {
            const CUnrealSkeletalMesh::SSection& section = cape.lods[ 0 ].sections[ sec ];
            const ASSETCONVERTER::CAssetSubMesh& sub = capeMesh.subMeshes[ sec ];
            for ( size_t v=0; v<section.vertices.size(); ++v )
                for ( UInt32 i=0; i<sub.influencesPerVertex; ++i )
                {
                    if ( 0.0f == sub.weights[ v * sub.influencesPerVertex + i ] )
                        continue;
                    const CORE::CString& name = capeBones[ section.boneMap[ section.vertices[ v ].bones[ i ] ] ].name;
                    ASSERT_TRUE( scene.nodes[ scene.skins[ 0 ].joints[ sub.joints[ v * sub.influencesPerVertex + i ] ] ].name == name );
                    ++checked;
                }
        }
        ASSERT_TRUE( checked >= 6 );

        // a skeleton with another root does not fit
        CUnrealSkeletalMesh other;
        ASSERT_TRUE( ReadMeshFromBytes( bytes, other, problem ) );
        CUnrealReferenceSkeleton::TBoneVector otherBones = other.skeleton.GetBones();
        otherBones[ 0 ].name = "Armature";
        other.skeleton.SetBones( otherBones );
        ASSERT_TRUE( !CUnrealCharacterAssembler::IsCompatible( body.skeleton, other.skeleton ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Material: the rule table maps the parameters of a master material and its instance to a metal-roughness material and reports the rest" )
    try
    {
        CUnrealResolvedMaterial resolved;
        resolved.name = "MI_Test";
        resolved.chain.push_back( "/Game/MI_Test" );
        resolved.masterPath = "/Game/M_Master.M_Master";
        resolved.blendMode = "BLEND_Masked";
        resolved.shadingModel = "MSM_DefaultLit";
        resolved.twoSided = true;
        resolved.opacityMaskClipValue = 0.4;
        CUnrealResolvedMaterial::SColour tint = { 0.5, 0.25, 1.0, 1.0 };
        CUnrealResolvedMaterial::SColour glow = { 1.0, 0.5, 0.0, 1.0 };
        CUnrealResolvedMaterial::SColour odd = { 0.1, 0.2, 0.3, 1.0 };
        resolved.colours[ "Base Color Tint" ] = tint;
        resolved.colours[ "Emissive_Color" ] = glow;
        resolved.colours[ "Glitter_Color" ] = odd;
        resolved.scalars[ "BaseColor_Brightness" ] = 2.0;
        resolved.scalars[ "BaseColor_Saturation" ] = 1.0;
        resolved.scalars[ "Roughness_Min" ] = 0.2;
        resolved.scalars[ "Roughness_Max" ] = 0.8;
        resolved.scalars[ "metallic" ] = 0.9;
        resolved.scalars[ "NormalStrength" ] = 0.5;
        resolved.scalars[ "AO_Strength" ] = 0.25;
        resolved.scalars[ "Specular" ] = 0.3;
        resolved.scalars[ "EmissiveIntensity" ] = 4.0;
        resolved.scalars[ "Glitter_Amount" ] = 1.0;
        resolved.switches[ "Use_ORM_Metallic" ] = false;
        resolved.switches[ "Use_Glitter" ] = true;
        resolved.textures[ "Normal_Tex" ] = "/Game/T_Normal.T_Normal";
        resolved.textures[ "ORM_Tex" ] = "/Game/T_Orm.T_Orm";

        CAssetMaterial material;
        CConversionReport report;
        CUnrealMaterialMapper::Map( resolved, "MI_Test", material, report );
        // no base colour texture: brightness is baked into the tint (0.5 * 2 = 1)
        ASSERT_TRUE( fabs( material.baseColorFactor[ 0 ] - 1.0f ) < 1e-5f && fabs( material.baseColorFactor[ 1 ] - 0.5f ) < 1e-5f && fabs( material.baseColorFactor[ 2 ] - 2.0f ) < 1e-5f );
        ASSERT_TRUE( fabs( material.roughnessFactor - 0.5f ) < 1e-5f );
        // the switch turns the metallic channel off
        ASSERT_TRUE( 0.0f == material.metallicFactor );
        ASSERT_TRUE( 0.5f == material.normalTexture.scale && 0.25f == material.occlusionTexture.scale );
        ASSERT_TRUE( ALPHAMODE_MASK == material.alphaMode && fabs( material.alphaCutoff - 0.4f ) < 1e-5f && material.doubleSided );
        ASSERT_TRUE( 1.0f == material.emissiveFactor[ 0 ] && 0.5f == material.emissiveFactor[ 1 ] && 4.0f == material.emissiveStrength );
        ASSERT_TRUE( GUCEF_NULL != material.extensions.FindChild( "KHR_materials_specular" ) );
        // the textures are recorded with their slots, the rest is in the source material and in one report entry
        const CORE::CDataNode* normal = material.source.textures.FindChild( "Normal_Tex" );
        ASSERT_TRUE( GUCEF_NULL != normal && "normalTexture" == normal->GetAttributeValue( "slot" ).AsString() );
        const CORE::CDataNode* orm = material.source.textures.FindChild( "ORM_Tex" );
        ASSERT_TRUE( GUCEF_NULL != orm && orm->GetAttributeValue( "slot" ).AsString().HasSubstr( "metallicRoughnessTexture" ) >= 0 );
        ASSERT_TRUE( material.source.properties.GetAttribute( "Glitter_Amount" ) != GUCEF_NULL && material.source.properties.GetAttribute( "Use_Glitter" ) != GUCEF_NULL );
        ASSERT_TRUE( material.source.properties.GetAttribute( "Roughness_Min" ) == GUCEF_NULL && material.source.properties.GetAttribute( "Use_ORM_Metallic" ) == GUCEF_NULL );
        UInt32 unmapped = 0;
        for ( size_t e=0; e<report.GetEntries().size(); ++e )
            for ( size_t i=0; i<report.GetEntries()[ e ].items.size(); ++i )
                if ( report.GetEntries()[ e ].items[ i ].message.HasSubstr( "3 parameters have no counterpart" ) >= 0 )
                    ++unmapped;
        ASSERT_TRUE( 1 == unmapped );

        // with a texture for the base colour the brightness stays in the source material
        resolved.textures[ "BaseColor_Tex" ] = "/Game/T_Base.T_Base";
        resolved.blendMode = "BLEND_Translucent";
        resolved.scalars[ "Opacity" ] = 0.6;
        CUnrealMaterialMapper::Map( resolved, "MI_Test", material, report );
        ASSERT_TRUE( fabs( material.baseColorFactor[ 0 ] - 0.5f ) < 1e-5f && ALPHAMODE_BLEND == material.alphaMode && fabs( material.baseColorFactor[ 3 ] - 0.6f ) < 1e-5f );
        ASSERT_TRUE( material.source.properties.GetAttribute( "BaseColor_Brightness" ) != GUCEF_NULL );
        ASSERT_TRUE( "basecolortint" == CUnrealMaterialMapper::Normalise( "Base Color_Tint" ) );

        // a black tint next to the texture is the default of the parameter: not the factor (the texture would show black)
        const CUnrealResolvedMaterial::SColour black = { 0.0, 0.0, 0.0, 1.0 };
        resolved.colours[ "Base Color Tint" ] = black;
        CConversionReport blackReport;
        CUnrealMaterialMapper::Map( resolved, "MI_Black", material, blackReport );
        ASSERT_TRUE( 1.0f == material.baseColorFactor[ 0 ] && 1.0f == material.baseColorFactor[ 1 ] && 1.0f == material.baseColorFactor[ 2 ] );
        UInt32 blackTint = 0;
        for ( size_t e=0; e<blackReport.GetEntries().size(); ++e )
            for ( size_t i=0; i<blackReport.GetEntries()[ e ].items.size(); ++i )
                if ( REPORTITEM_APPROXIMATION == blackReport.GetEntries()[ e ].items[ i ].kind && blackReport.GetEntries()[ e ].items[ i ].message.HasSubstr( "is black (the default of the parameter)" ) >= 0 )
                    ++blackTint;
        ASSERT_TRUE( 1 == blackTint );
        // without a texture a black tint is the colour
        resolved.textures.erase( "BaseColor_Tex" );
        CUnrealMaterialMapper::Map( resolved, "MI_Black", material, blackReport );
        ASSERT_TRUE( 0.0f == material.baseColorFactor[ 0 ] && 0.0f == material.baseColorFactor[ 2 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mesh: damaged skeletal mesh data fails cleanly, never reads wrongly and never crashes" )
    try
    {
        SLayout layout = { true, 7, true, true, true };
        GUCEF::vector< UInt8 > bytes;
        BuildMeshPackage( -9, 522, 1018, layout, bytes );
        // find the native data to damage it: the export starts at its serial offset
        CUnrealPackage original;
        ASSERT_TRUE( original.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "mesh.uasset" ) );
        const UInt64 nativeStart = (UInt64) original.GetExports()[ 0 ].serialOffset;
        UInt32 survived = 0;
        for ( UInt32 variant=0; variant<400; ++variant )
        {
            GUCEF::vector< UInt8 > damaged( bytes );
            if ( variant < 120 )
            {
                // truncation: the package says the export is bigger than the file, or the file ends inside it
                const size_t keep = nativeStart + ( ( bytes.size() - nativeStart ) * variant ) / 120;
                damaged.resize( keep );
            }
            else
            {
                const size_t at = nativeStart + ( ( (UInt64) variant * 2654435761ULL ) % ( bytes.size() - nativeStart ) );
                damaged[ at ] ^= (UInt8) ( 0x01 << ( variant % 8 ) );
                // counts blown up
                if ( 0 == variant % 7 && at + 3 < damaged.size() ) { damaged[ at + 3 ] = 0x7F; }
            }
            CUnrealSkeletalMesh mesh;
            CORE::CString problem;
            if ( ReadMeshFromBytes( damaged, mesh, problem ) )
            {
                // when it reads, it is consistent: indices inside the vertices of their sections
                ++survived;
                for ( size_t l=0; l<mesh.lods.size(); ++l )
                    for ( size_t s=0; s<mesh.lods[ l ].sections.size(); ++s )
                        ASSERT_TRUE( mesh.lods[ l ].sections[ s ].materialIndex < mesh.materialSlots.size() || mesh.materialSlots.empty() );
            }
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Damaged skeletal meshes: " << survived << " of 400 variants still read as consistent meshes" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "Corpus: the skeletal meshes of the MetaHuman packages and of the project of 5.3 read and convert: golden summaries, sizes, winding" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString archiveDir = CorpusSetting( "archivedir" );
    if ( archiveDir.IsNULLOrEmpty() || !CORE::DirExists( archiveDir ) )
        GUCEF_TESTFW_SKIP( "The corpus manifest names no archive folder" );
    try
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CORE::CString::StringVector archives;
        archives.push_back( "scarletjohansson" );
        archives.push_back( "margaretqualley" );
        archives.push_back( "emiliaclarke" );
        UInt32 total = 0;
        for ( size_t a=0; a<archives.size(); ++a )
        {
            const CORE::CString nativePath = CORE::CombinePath( archiveDir, archives[ a ] + ".mhpkg" );
            if ( !CORE::FileExists( nativePath ) )
                continue;
            const CORE::CString mountName = "unreal_meshes_" + archives[ a ];
            ASSERT_TRUE( vfs.MountArchive( mountName, nativePath, "mhpkg", mountName, false, false, nativePath ) );
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, mountName, true, true, CORE::CString::EmptyStringSet, 100000 );
            std::sort( files.begin(), files.end() );
            CORE::CString text;
            UInt32 meshes = 0;
            bool ok = true;
            const UInt64 start = MT::PrecisionTickCount();
            for ( size_t f=0; f<files.size(); ++f )
            {
                if ( files[ f ].Lowercase().HasSubstr( ".uasset" ) != (Int32) files[ f ].Length() - 7 )
                    continue;
                // packages that hold a skeletal mesh: SKM_ and SK_ by name, the combined body is one of them
                const CORE::CString file = CORE::ExtractFilename( files[ f ] );
                if ( file.HasSubstr( "SKM_" ) != 0 && file.HasSubstr( "SK_" ) != 0 )
                    continue;
                ok = DescribeMeshesOfPackage( files[ f ], file, text, meshes, false ) && ok;
            }
            const double ms = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus meshes " << archives[ a ] << ": " << meshes << " skeletal meshes read and converted in " << ms << " ms" );
            vfs.UnmountArchiveByName( mountName );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( meshes >= 1 );
            total += meshes;
            CORE::CString problem;
            const bool golden = CheckGolden( "meshes_" + archives[ a ], text, problem );
            if ( !golden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
            ASSERT_TRUE( golden );
        }
        ASSERT_TRUE( total >= 3 );

        // the project of 5.3: the skeletal meshes of the characters and the mannequins
        const CORE::CString projectZip = CorpusSetting( "projectzip" );
        if ( !projectZip.IsNULLOrEmpty() && CORE::FileExists( projectZip ) )
        {
            ASSERT_TRUE( vfs.MountArchive( "unreal_meshes_project", projectZip, "zip", "unreal_meshes_project", false, false, projectZip ) );
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, "unreal_meshes_project", true, true, CORE::CString::EmptyStringSet, 1000000 );
            std::sort( files.begin(), files.end() );
            CORE::CString text;
            UInt32 meshes = 0;
            bool ok = true;
            for ( size_t f=0; f<files.size(); ++f )
            {
                const CORE::CString file = CORE::ExtractFilename( files[ f ] );
                if ( file.Lowercase().HasSubstr( ".uasset" ) != (Int32) file.Length() - 7 || ( file.HasSubstr( "SK_" ) != 0 && file.HasSubstr( "SKM_" ) != 0 ) || file.HasSubstr( "Skeleton" ) >= 0 )
                    continue;
                ok = DescribeMeshesOfPackage( files[ f ], files[ f ].SubstrFromRange( files[ f ].HasSubstr( "Content/" ), files[ f ].Length() ), text, meshes, false ) && ok;
            }
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus meshes project of 5.3: " << meshes << " skeletal meshes read and converted" );
            VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( "unreal_meshes_project" );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( meshes >= 20 );
            CORE::CString problem;
            const bool golden = CheckGolden( "meshes_project", text, problem );
            if ( !golden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
            ASSERT_TRUE( golden );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "Corpus: the importer turns a MetaHuman package into one scene with the skeletal meshes, their skeletons and mapped materials" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString archiveDir = CorpusSetting( "archivedir" );
    if ( archiveDir.IsNULLOrEmpty() || !CORE::DirExists( archiveDir ) )
        GUCEF_TESTFW_SKIP( "The corpus manifest names no archive folder" );
    try
    {
        CNoOodleToolScope noTool;
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.MountArchive( "unreal_import_corpus", archiveDir, VFS::CVFS::FileSystemArchiveTypeName, "importcorpus", false, false, archiveDir ) );
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        const char* packages[] = { "scarletjohansson", "margaretqualley" };
        CORE::CString summary;
        for ( UInt32 p=0; p<2; ++p )
        {
            CAssetPackContext context;
            CAssetScene scene;
            CConversionReport report;
            const UInt64 start = MT::PrecisionTickCount();
            ASSERT_TRUE( importer->Import( CORE::CString( "importcorpus/" ) + packages[ p ] + ".mhpkg", context, scene, report ) );
            const double ms = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
            ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
            Float32 low[ 3 ];
            Float32 high[ 3 ];
            SceneBounds( scene, low, high );
            UInt32 lodGroups = (UInt32) scene.lodGroups.size();
            UInt32 mapped = 0;
            for ( size_t m=0; m<scene.materials.size(); ++m )
                if ( !scene.materials[ m ].source.shaderName.IsNULLOrEmpty() && scene.materials[ m ].source.shaderName != "Unreal/unknown" )
                    ++mapped;
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Import of " << packages[ p ] << ": " << scene.meshes.size() << " meshes, " << scene.skins.size() << " skins, " << scene.nodes.size() << " nodes, " << scene.materials.size() << " materials (" << mapped << " from a master material), " << scene.GetVertexCount() << " vertices, " << scene.GetTriangleCount() << " triangles in " << ms << " ms" );
            summary += CORE::CString( packages[ p ] ) + " | meshes " + CORE::UInt64ToString( scene.meshes.size() ) + " | skins " + CORE::UInt64ToString( scene.skins.size() ) + " | nodes " + CORE::UInt64ToString( scene.nodes.size() ) + " | lod groups " + CORE::UInt32ToString( lodGroups ) +
                " | materials " + CORE::UInt64ToString( scene.materials.size() ) + " (" + CORE::UInt32ToString( mapped ) + " mapped) | vertices " + CORE::UInt32ToString( scene.GetVertexCount() ) + " | triangles " + CORE::UInt32ToString( scene.GetTriangleCount() ) + "\n";
            for ( size_t m=0; m<scene.materials.size(); ++m )
                summary += "  material " + scene.materials[ m ].name + " | " + scene.materials[ m ].source.shaderName + " | alpha " + ToString( scene.materials[ m ].alphaMode ) + " | double sided " + CORE::CString( scene.materials[ m ].doubleSided ? "yes" : "no" ) + "\n";
            // the textures and static meshes of the package are Oodle payloads: reported with that reason, the skeleton with its facial joints
            const UInt32 oodle = CountReportItems( report, REPORTITEM_SKIPPED, "needs the oodle provider" );
            const UInt32 skeletons = CountReportItems( report, REPORTITEM_INFO, "Skeleton with " );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Import of " << packages[ p ] << ": " << oodle << " assets need the oodle provider, " << skeletons << " skeleton assets read" );
            ASSERT_TRUE( oodle >= 2 && ( skeletons >= 1 || 1 == p ) );
            // body and garments share one skeleton and one skin
            ASSERT_TRUE( 1 == scene.skins.size() && scene.meshes.size() >= 3 && lodGroups >= 1 );
            ASSERT_TRUE( mapped >= 1 );
            // the combined body is the first mesh of the first package: about 1.69 m tall, Y up
            if ( 0 == p )
            {
                const Float32 height = high[ 1 ] - low[ 1 ];
                ASSERT_TRUE( height > 1.66f && height < 1.72f );
            }
        }
        vfs.UnmountArchiveByName( "unreal_import_corpus" );
        CORE::CString problem;
        const bool golden = CheckGolden( "importer_metahuman", summary, problem );
        if ( !golden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
        ASSERT_TRUE( golden );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Corpus: the parent chains of the material instances resolve to their master materials in the packages of 5.3, 5.6 and 5.8" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString archiveDir = CorpusSetting( "archivedir" );
    if ( archiveDir.IsNULLOrEmpty() || !CORE::DirExists( archiveDir ) )
        GUCEF_TESTFW_SKIP( "The corpus manifest names no archive folder" );
    try
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CORE::CString::StringVector archives;
        archives.push_back( "scarletjohansson" );
        archives.push_back( "margaretqualley" );
        for ( size_t a=0; a<archives.size(); ++a )
        {
            const CORE::CString nativePath = CORE::CombinePath( archiveDir, archives[ a ] + ".mhpkg" );
            const CORE::CString mountName = "unreal_materials_" + archives[ a ];
            ASSERT_TRUE( vfs.MountArchive( mountName, nativePath, "mhpkg", mountName, false, false, nativePath ) );
            CUnrealObjectPathResolver resolver;
            ASSERT_TRUE( resolver.AddPackagesOf( mountName ) > 10 );
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, mountName, true, true, CORE::CString::EmptyStringSet, 100000 );
            std::sort( files.begin(), files.end() );
            CORE::CString text;
            UInt32 resolvedCount = 0;
            UInt32 problems = 0;
            UInt32 instances = 0;
            for ( size_t f=0; f<files.size(); ++f )
            {
                const CORE::CString file = CORE::ExtractFilename( files[ f ] );
                if ( file.HasSubstr( "MI_" ) == 0 || file.HasSubstr( "M_" ) == 0 )
                {
                    ++instances;
                    DescribeMaterial( files[ f ], resolver, text, resolvedCount, problems );
                }
            }
            vfs.UnmountArchiveByName( mountName );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus materials " << archives[ a ] << ": " << resolvedCount << " of " << instances << " materials resolved, " << problems << " problems" );
            ASSERT_TRUE( instances > 0 && resolvedCount == instances );
            CORE::CString problem;
            const bool golden = CheckGolden( "materials_" + archives[ a ], text, problem );
            if ( !golden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
            ASSERT_TRUE( golden );
        }

        // the instances of the project of 5.3, whose parents are in the project or in the engine
        const CORE::CString projectZip = CorpusSetting( "projectzip" );
        if ( !projectZip.IsNULLOrEmpty() && CORE::FileExists( projectZip ) )
        {
            ASSERT_TRUE( vfs.MountArchive( "unreal_materials_project", projectZip, "zip", "unreal_materials_project", false, false, projectZip ) );
            CUnrealObjectPathResolver resolver;
            resolver.AddPackagesOf( "unreal_materials_project" );
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, "unreal_materials_project", true, true, CORE::CString::EmptyStringSet, 1000000 );
            std::sort( files.begin(), files.end() );
            CORE::CString text;
            UInt32 resolvedCount = 0;
            UInt32 problems = 0;
            UInt32 instances = 0;
            for ( size_t f=0; f<files.size(); ++f )
            {
                const CORE::CString file = CORE::ExtractFilename( files[ f ] );
                if ( file.Lowercase().HasSubstr( ".uasset" ) == (Int32) file.Length() - 7 && ( file.HasSubstr( "MI_" ) == 0 || file.HasSubstr( "M_" ) == 0 ) )
                {
                    ++instances;
                    DescribeMaterial( files[ f ], resolver, text, resolvedCount, problems );
                }
            }
            vfs.UnmountArchiveByName( "unreal_materials_project" );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus materials project of 5.3: " << resolvedCount << " of " << instances << " materials resolved, " << problems << " problems" );
            ASSERT_TRUE( instances >= 20 && resolvedCount == instances );
            CORE::CString problem;
            const bool golden = CheckGolden( "materials_project", text, problem );
            if ( !golden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
            ASSERT_TRUE( golden );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

}
/*-------------------------------------------------------------------------*/
