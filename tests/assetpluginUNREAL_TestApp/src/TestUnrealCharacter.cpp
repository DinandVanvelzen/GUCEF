/*
 *  assetpluginUNREAL_TestApp: Tests of the MetaHuman character: root asset, textures, assembly and presentation hints
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
#include <stdio.h>
#include <string.h>

#include "gucef_test_framework.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define ASSETCONVERTERLIB_H
#endif /* ASSETCONVERTERLIB_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#include "assetpluginUNREAL_CUnrealMetaHumanCharacter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H
#include "assetpluginUNREAL_CUnrealMetaHumanTextures.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H
#include "assetpluginUNREAL_CUnrealHiddenFaceMap.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H
#include "assetpluginUNREAL_CUnrealPresentationHints.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#include "assetpluginUNREAL_CUnrealImporter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H ? */

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

#include "TestUnrealCharacter.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static CTestType T( const char* name ) { return CTestType( name ); }
static CTestType T( const char* name, const CTestType& a ) { return CTestType( name, a ); }
static CTestType T( const char* name, const CTestType& a, const CTestType& b ) { return CTestType( name, a, b ); }

/*-------------------------------------------------------------------------*/

static bool
IsNear( Float32 a, Float32 b, Float32 tolerance )
{
    return fabsf( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Num( Float32 value )
{
    char text[ 32 ];
    sprintf( text, "%.3f", (double) value );
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Vec( const CUnrealPresentationHints::SVector& v )
{
    return Num( v.v[ 0 ] ) + " " + Num( v.v[ 1 ] ) + " " + Num( v.v[ 2 ] );
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

/** Bytes that begin like a PNG file of the size: the signature and the header chunk, then filler */
static void
MakePngLike( UInt32 width, UInt32 height, UInt8 fill, GUCEF::vector< UInt8 >& bytes )
{
    CTestByteWriter w;
    const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    w.Raw( signature, 8 );
    w.U32BE( 13 );
    w.U8( 'I' ); w.U8( 'H' ); w.U8( 'D' ); w.U8( 'R' );
    w.U32BE( width );
    w.U32BE( height );
    for ( UInt32 i=0; i<40; ++i )
        w.U8( (UInt8) ( fill + i ) );
    bytes = w.bytes;
}

/*-------------------------------------------------------------------------*/

/** A compressed buffer of Oodle (method 3, one block) as the package stores it: its content is not decodable here */
static void
MakeOodleLike( GUCEF::vector< UInt8 >& bytes )
{
    CTestByteWriter w;
    w.U32BE( 0xB7756362 ); w.U32BE( 0 ); w.U8( 3 ); w.U8( 3 ); w.U8( 3 ); w.U8( 18 ); w.U32BE( 1 ); w.U64BE( 1000 ); w.U64BE( 64 + 4 + 100 ); w.Zero( 32 );
    w.U32BE( 100 ); w.Zero( 100 );
    bytes = w.bytes;
}

/*-------------------------------------------------------------------------*/

static const char* const FaceKeys[] = { "Basecolor", "Normal", "Cavity", "Basecolor_Animated_CM1", "Basecolor_Animated_CM2", "Basecolor_Animated_CM3", "Normal_Animated_WM1", "Normal_Animated_WM2", "Normal_Animated_WM3" };
static const char* const BodyKeys[] = { "Body_Basecolor", "Body_Normal", "Body_Cavity", "Body_Underwear_Basecolor", "Body_Underwear_Normal", "Body_Underwear_Mask", "Chest_Basecolor", "Chest_Normal", "Chest_Cavity", "Chest_Underwear_Basecolor", "Chest_Underwear_Normal" };

/** One map of texture information: an enumerator key and a struct of sizes per texture, in the order of the property map */
static void
WriteTextureMap( CTestPackageWriter& package, CTestPropertyWriter& p, CTestByteWriter& b, const char* property, const char* enumName, const char* const* keys, UInt32 count, UInt32 size )
{
    CTestPropertyWriter::SOpen o = p.Begin( property, T( "MapProperty", T( "EnumProperty", T( enumName ), T( "ByteProperty" ) ), StructType( "MetaHumanCharacterTextureInfo" ) ) );
    b.I32( 0 );
    b.I32( (Int32) count );
    for ( UInt32 i=0; i<count; ++i )
    {
        package.names.WriteName( b, CORE::CString( enumName ) + "::" + keys[ i ] );
        CTestPropertyWriter::SOpen a = p.Begin( "SizeX", T( "IntProperty" ) ); b.I32( (Int32) size ); p.End( a );
        a = p.Begin( "SizeY", T( "IntProperty" ) ); b.I32( (Int32) size ); p.End( a );
        a = p.Begin( "NumSlices", T( "IntProperty" ) ); b.I32( 1 ); p.End( a );
        a = p.Begin( "Format", T( "ByteProperty" ) ); b.U8( 1 ); p.End( a );
        p.None();
    }
    p.End( o );
}

/*-------------------------------------------------------------------------*/

/** A package with a MetaHumanCharacter export: viewport, name, skin and the two texture maps; behind the properties 24 records (4 of state, 9 face, 11 body) */
static void
BuildCharacterPackage( GUCEF::vector< UInt8 >& bytes, GUCEF::vector< GUCEF::vector< UInt8 > >& payloadBytes )
{
    CTestPackageWriter writer( -9, 522, 1018 );
    writer.packageName = "/Game/EditableCharacters/Testy/Testy";
    const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/MetaHumanCharacter" );
    writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "MetaHumanCharacter" );

    // the payloads in the order of the records: state data, the face textures in the order of the enumeration, the body textures
    GUCEF::vector< UInt32 > payloads;
    payloadBytes.clear();
    for ( UInt32 r=0; r<24; ++r )
    {
        GUCEF::vector< UInt8 > raw;
        bool stored = true;
        if ( 4 == r || 8 == r || 13 == r || 14 == r )
            MakePngLike( 4, 4, (UInt8) ( r * 10 ), raw );                 // face base colour, face normal, body base colour, body normal
        else if ( 9 == r )
        {
            MakeOodleLike( raw );
            stored = false;
        }
        else
            raw.assign( 200 + r, (UInt8) r );
        payloadBytes.push_back( raw );
        payloads.push_back( writer.AddPayload( raw, stored ) );
    }

    CTestByteWriter body;
    writer.BeginBody( body );
    CTestPropertyWriter p( body, writer.names, writer.ue4, writer.ue5 );
    CTestPropertyWriter::SOpen o = p.Begin( "SkinSettings", StructType( "MetaHumanCharacterSkinSettings" ) );
    {
        CTestPropertyWriter::SOpen skin = p.Begin( "Skin", StructType( "MetaHumanCharacterSkinProperties" ) );
        CTestPropertyWriter::SOpen a = p.Begin( "U", T( "FloatProperty" ) ); body.F32( 0.25f ); p.End( a );
        a = p.Begin( "FaceTextureIndex", T( "IntProperty" ) ); body.I32( 7 ); p.End( a );
        p.None();
        p.End( skin );
        p.None();
    }
    p.End( o );
    o = p.Begin( "bHasHighResolutionTextures", T( "BoolProperty" ), true );
    WriteTextureMap( writer, p, body, "SynthesizedFaceTexturesInfo", "EFaceTextureType", FaceKeys, 9, 4 );
    WriteTextureMap( writer, p, body, "HighResBodyTexturesInfo", "EBodyTextureType", BodyKeys, 11, 4 );
    o = p.Begin( "ViewportSettings", StructType( "MetaHumanCharacterViewportSettings" ) );
    {
        CTestPropertyWriter::SOpen a = p.Begin( "LightRotation", T( "FloatProperty" ) ); body.F32( -33.5f ); p.End( a );
        a = p.Begin( "CameraFrame", T( "EnumProperty", T( "EMetaHumanCharacterCameraFrame" ), T( "ByteProperty" ) ) ); writer.names.WriteName( body, "EMetaHumanCharacterCameraFrame::Body" ); p.End( a );
        a = p.Begin( "CharacterEnvironment", T( "EnumProperty", T( "EMetaHumanCharacterEnvironment" ), T( "ByteProperty" ) ) ); writer.names.WriteName( body, "EMetaHumanCharacterEnvironment::Studio" ); p.End( a );
        p.None();
    }
    p.End( o );
    o = p.Begin( "AssemblySettings", StructType( "MetaHumanCharacterAssemblySettings" ) );
    {
        CTestPropertyWriter::SOpen a = p.Begin( "NameOverride", T( "StrProperty" ) ); body.Str( "Testy Override" ); p.End( a );
        p.None();
    }
    p.End( o );
    p.None();
    const Int64 scriptEnd = (Int64) body.Size();
    body.Bool32( false );                       // no object guid
    for ( UInt32 r=0; r<24; ++r )
        writer.WriteEditorBulkData( body, payloads[ r ] );
    body.Zero( 8 );
    writer.AddExport( -2, 0, "Testy", body, scriptEnd );
    writer.Build( bytes );
}

/*-------------------------------------------------------------------------*/

/** A body mesh by hand: root, pelvis at 95 cm and the head 65 cm above it; 'headVertices' vertices weighted to the head, 100 to the pelvis */
static void
MakeBodyMesh( UInt32 headVertices, CUnrealSkeletalMesh& mesh )
{
    mesh.Clear();
    mesh.name = "SKM_Testy_CombinedBody";
    CUnrealReferenceSkeleton::TBoneVector bones( 3 );
    const char* names[] = { "pelvis_root", "pelvis", "head" };
    const Int32 parents[] = { -1, 0, 1 };
    const Float64 heights[] = { 0.0, 95.0, 65.0 };
    for ( UInt32 b=0; b<3; ++b )
    {
        memset( &bones[ b ].rotation, 0, sizeof( bones[ b ].rotation ) );
        memset( &bones[ b ].translation, 0, sizeof( bones[ b ].translation ) );
        bones[ b ].name = names[ b ];
        bones[ b ].parent = parents[ b ];
        bones[ b ].rotation[ 3 ] = 1.0;
        bones[ b ].translation[ 2 ] = heights[ b ];
        bones[ b ].scale[ 0 ] = bones[ b ].scale[ 1 ] = bones[ b ].scale[ 2 ] = 1.0;
    }
    mesh.skeleton.SetBones( bones );
    CUnrealSkeletalMesh::SLod lod;
    lod.uvSetCount = 1;
    lod.screenSize = 1.0f;
    CUnrealSkeletalMesh::SSection section;
    section.materialIndex = 0;
    section.baseIndex = 0;
    section.baseVertexIndex = 0;
    section.maxInfluences = 1;
    section.castShadow = true;
    section.disabled = false;
    section.boneMap.push_back( 0 );
    section.boneMap.push_back( 1 );
    section.boneMap.push_back( 2 );
    const UInt32 total = headVertices + 100;
    for ( UInt32 v=0; v<total; ++v )
    {
        CUnrealSkeletalMesh::SVertex vertex;
        memset( &vertex, 0, sizeof( vertex ) );
        const bool head = v < headVertices;
        const Float32 t = (Float32) ( head ? v : v - headVertices ) / (Float32) ( head ? headVertices : 100 );
        vertex.position[ 0 ] = head ? -10.0f + 20.0f * t : -25.0f + 50.0f * t;
        vertex.position[ 1 ] = head ? 10.0f - 20.0f * t : -10.0f + 20.0f * t;
        vertex.position[ 2 ] = head ? 150.0f + 25.0f * t : 150.0f * t;
        vertex.tangentX[ 0 ] = 1.0f;
        vertex.tangentZ[ 2 ] = 1.0f;
        vertex.tangentZ[ 3 ] = 1.0f;
        memset( vertex.colour, 255, sizeof( vertex.colour ) );
        vertex.bones[ 0 ] = head ? 2 : 1;
        vertex.weights[ 0 ] = 65535;
        section.vertices.push_back( vertex );
    }
    section.triangleCount = total / 3;
    for ( UInt32 i=0; i<section.triangleCount * 3; ++i )
        lod.indices.push_back( i );
    lod.sections.push_back( section );
    mesh.lods.push_back( lod );
    mesh.materialSlots.resize( 1 );
}

/*-------------------------------------------------------------------------*/

void
PerformUnrealCharacterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Character" );

    GUCEF_TESTFW_TESTCASE( "Character: the root asset reads its viewport, name and settings, the texture maps and the payload records; stored PNG payloads are told from Oodle ones" )
    try
    {
        GUCEF::vector< UInt8 > bytes;
        GUCEF::vector< GUCEF::vector< UInt8 > > payloadBytes;
        BuildCharacterPackage( bytes, payloadBytes );
        CUnrealPackage package;
        ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "Testy.uasset" ) );
        ASSERT_TRUE( 1 == package.GetExports().size() && "MetaHumanCharacter" == package.GetClassName( package.GetExports()[ 0 ] ) );
        CUnrealMetaHumanCharacter character;
        ASSERT_TRUE( character.Read( package, 0 ) );
        ASSERT_TRUE( "Testy" == character.name && "Testy Override" == character.nameOverride && "/Game/EditableCharacters/Testy/Testy" == character.packageName );
        ASSERT_TRUE( "Body" == character.cameraFrame && "Studio" == character.characterEnvironment && character.hasLightRotation && IsNear( character.lightRotation, -33.5f, 1e-4f ) );
        ASSERT_TRUE( character.hasHighResolutionTextures );
        ASSERT_TRUE( 20 == character.textures.size() && 24 == character.records.size() && 20 == character.payloadsAssigned );
        // the settings are a compact tree: scalars as attributes
        const CORE::CDataNode* skin = character.settings.FindChild( "SkinSettings" ) ? character.settings.FindChild( "SkinSettings" )->FindChild( "Skin" ) : GUCEF_NULL;
        ASSERT_TRUE( GUCEF_NULL != skin && 7 == skin->GetAttributeValue( "FaceTextureIndex" ).AsInt32() && IsNear( skin->GetAttributeValue( "U" ).AsFloat32(), 0.25f, 1e-5f ) );
        // the face textures are stored in the order of the enumeration, not of the property map: the normal map is the fifth
        const CUnrealMetaHumanCharacter::STexture* faceBase = GUCEF_NULL;
        const CUnrealMetaHumanCharacter::STexture* faceNormal = GUCEF_NULL;
        const CUnrealMetaHumanCharacter::STexture* wrinkle = GUCEF_NULL;
        const CUnrealMetaHumanCharacter::STexture* bodyBase = GUCEF_NULL;
        for ( size_t t=0; t<character.textures.size(); ++t )
        {
            const CUnrealMetaHumanCharacter::STexture& texture = character.textures[ t ];
            if ( "face" == texture.group && "Basecolor" == texture.type ) faceBase = &texture;
            if ( "face" == texture.group && "Normal" == texture.type ) faceNormal = &texture;
            if ( "face" == texture.group && "Normal_Animated_WM1" == texture.type ) wrinkle = &texture;
            if ( "body" == texture.group && "Body_Basecolor" == texture.type ) bodyBase = &texture;
        }
        ASSERT_TRUE( GUCEF_NULL != faceBase && GUCEF_NULL != faceNormal && GUCEF_NULL != wrinkle && GUCEF_NULL != bodyBase );
        ASSERT_TRUE( 4 == faceBase->record && 8 == faceNormal->record && 9 == wrinkle->record && 13 == bodyBase->record );
        ASSERT_TRUE( faceBase->IsExtractable() && 4 == faceBase->pngWidth && 4 == faceBase->pngHeight && faceBase->pngSize == payloadBytes[ 4 ].size() );
        ASSERT_TRUE( !wrinkle->IsExtractable() && 3 == wrinkle->method );
        UInt32 extractable = 0;
        for ( size_t t=0; t<character.textures.size(); ++t )
            if ( character.textures[ t ].IsExtractable() )
                ++extractable;
        ASSERT_TRUE( 4 == extractable );
        // the PNG file comes out as it went in
        CORE::CDynamicBuffer png;
        ASSERT_TRUE( character.ReadPng( package, *faceNormal, png ) && png.GetDataSize() == payloadBytes[ 8 ].size() && 0 == memcmp( png.GetConstBufferPtr(), &payloadBytes[ 8 ][ 0 ], payloadBytes[ 8 ].size() ) );
        ASSERT_TRUE( !character.ReadPng( package, *wrinkle, png ) );

        // a character that states no viewport settings has no light rotation and no frame
        CUnrealMetaHumanCharacter empty;
        ASSERT_TRUE( !empty.hasLightRotation && empty.cameraFrame.IsNULLOrEmpty() && 0 == empty.textures.size() );

        // the textures become files and images of the scene
        CAssetScene scene;
        CConversionReport report;
        GUCEF::vector< Int32 > indices;
        ASSERT_TRUE( 0 == CUnrealMetaHumanTextures::AddToScene( package, character, CORE::CString::Empty, "Testy", "Testy", scene, report, indices ) );
        ASSERT_TRUE( 1 == CountReportItems( report, REPORTITEM_WARNING, "no intermediate folder" ) && 0 == scene.textures.size() );
        ASSERT_TRUE( 4 == CUnrealMetaHumanTextures::AddToScene( package, character, WorkVfsPath( "character/intermediate" ), "Testy", "Testy", scene, report, indices ) );
        ASSERT_TRUE( 4 == scene.textures.size() && 1 == CountReportItems( report, REPORTITEM_SKIPPED, "need the oodle provider" ) );
        const CORE::CString diskPath = WorkDiskPath( "character/intermediate/embedded/Testy/Testy_Face_Normal.png" );
        ASSERT_TRUE( CORE::FileExists( diskPath ) );
        CORE::CDynamicBuffer written;
        ASSERT_TRUE( written.LoadContentFromFile( diskPath ) && written.GetDataSize() == payloadBytes[ 8 ].size() && 0 == memcmp( written.GetConstBufferPtr(), &payloadBytes[ 8 ][ 0 ], payloadBytes[ 8 ].size() ) );
        // the usage follows the type: the normal map is a normal map, the colour maps are colour
        UInt32 normals = 0;
        UInt32 colours = 0;
        for ( size_t t=0; t<scene.textures.size(); ++t )
        {
            if ( TEXTUREUSAGE_NORMAL == scene.textures[ t ].usage ) ++normals;
            if ( TEXTUREUSAGE_COLOUR == scene.textures[ t ].usage ) ++colours;
        }
        ASSERT_TRUE( 2 == normals && 2 == colours );
        for ( size_t t=0; t<scene.textures.size(); ++t )
            VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( scene.textures[ t ].sources[ 0 ], true );

        // the textures on the materials: the body maps on the body shader, the face maps stay images that no material uses
        CAssetMaterial bodyMaterial;
        bodyMaterial.name = "body_shader_shader";
        CAssetMaterial dress;
        dress.name = "MI_Dress";
        scene.materials.push_back( bodyMaterial );
        scene.materials.push_back( dress );
        CORE::CString::StringVector unassigned;
        ASSERT_TRUE( 2 == CUnrealMetaHumanTextures::AttachToMaterials( character, indices, scene, report, unassigned ) );
        ASSERT_TRUE( scene.materials[ 0 ].baseColorTexture.IsUsed() && scene.materials[ 0 ].normalTexture.IsUsed() && !scene.materials[ 1 ].baseColorTexture.IsUsed() );
        ASSERT_TRUE( 2 == unassigned.size() && "Testy_Face_Basecolor" == unassigned[ 0 ] );
        CORE::CDataNode record( "gucefCharacter" );
        CUnrealMetaHumanTextures::DescribeCharacter( character, "Testy Override", indices, scene, record );
        ASSERT_TRUE( 20 == record.FindChild( "textures" )->GetNrOfDirectChildNodes() && "Testy Override" == record.GetAttributeValue( "name" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL != record.FindChild( "settings" ) && GUCEF_NULL != record.FindChild( "settings" )->FindChild( "SkinSettings" ) && GUCEF_NULL == record.FindChild( "" ) );
        ASSERT_TRUE( "needs the oodle provider" == record.FindChild( "textures" )->FindChild( "Testy Override_Face_Normal_Animated_WM1" )->GetAttributeValue( "state" ).AsString() );
        ASSERT_TRUE( "image" == record.FindChild( "textures" )->FindChild( "Testy Override_Face_Basecolor" )->GetAttributeValue( "state" ).AsString() );

        // a map that does not fit the known set of texture types gets no payloads assigned
        CUnrealMetaHumanCharacter odd;
        ASSERT_TRUE( odd.Read( package, 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Character: the portrait framing from bounds and head bounds, with the numbers a viewer uses" )
    try
    {
        typedef CUnrealPresentationHints::SVector TVector;
        CUnrealPresentationHints::SHints hints;
        CUnrealPresentationHints::Compute( TVector( -0.3f, 0.0f, -0.1f ), TVector( 0.3f, 1.75f, 0.1f ), TVector( -0.1f, 1.5f, -0.1f ), TVector( 0.1f, 1.75f, 0.1f ), TVector( 0.0f, 0.0f, -1.0f ), false, 0.0f, hints );
        const CUnrealPresentationHints::SView& portrait = hints.portrait;
        // two head heights from above the crown to the shoulders, centred 0.8 head heights below the crown
        ASSERT_TRUE( IsNear( portrait.frameHeight, 0.5f, 1e-5f ) && IsNear( portrait.target.v[ 1 ], 1.55f, 1e-5f ) && IsNear( portrait.target.v[ 0 ], 0.0f, 1e-5f ) );
        ASSERT_TRUE( IsNear( portrait.fovDegrees, 30.0f, 1e-5f ) && IsNear( portrait.yawDegrees, 30.0f, 1e-5f ) && IsNear( portrait.pitchDegrees, 3.0f, 1e-5f ) );
        ASSERT_TRUE( IsNear( portrait.distance, 0.25f / tanf( 15.0f * 3.14159265f / 180.0f ), 1e-4f ) );
        // the camera is in front of the face and a little to the side, slightly above the target, at the distance
        ASSERT_TRUE( IsNear( portrait.position.v[ 0 ], -0.4659f, 2e-3f ) && IsNear( portrait.position.v[ 1 ], 1.5988f, 2e-3f ) && IsNear( portrait.position.v[ 2 ], -0.8069f, 2e-3f ) );
        Float32 length = 0.0f;
        for ( UInt32 c=0; c<3; ++c )
            length += ( portrait.position.v[ c ] - portrait.target.v[ c ] ) * ( portrait.position.v[ c ] - portrait.target.v[ c ] );
        ASSERT_TRUE( IsNear( sqrtf( length ), portrait.distance, 1e-4f ) );
        // the side vector is up x facing
        ASSERT_TRUE( IsNear( hints.side.v[ 0 ], -1.0f, 1e-5f ) && IsNear( hints.side.v[ 2 ], 0.0f, 1e-5f ) );
        // the full body frames the whole height with a margin, centred on the bounds
        ASSERT_TRUE( IsNear( hints.fullBody.target.v[ 1 ], 0.875f, 1e-5f ) && IsNear( hints.fullBody.frameHeight, 1.925f, 1e-4f ) && hints.fullBody.distance > hints.fullBody.frameHeight );
        ASSERT_TRUE( IsNear( hints.turntableCentre.v[ 1 ], 0.0f, 1e-5f ) && IsNear( hints.turntableRadius, hints.fullBody.distance, 1e-5f ) );
        // the lights: a default key at 45 on the side of the camera, the negated light rotation of the asset when it states one
        ASSERT_TRUE( IsNear( hints.key.yawDegrees, 45.0f, 1e-5f ) && hints.key.intensity > hints.fill.intensity && IsNear( hints.fill.yawDegrees, -45.0f, 1e-5f ) );
        CUnrealPresentationHints::Compute( TVector( -0.3f, 0.0f, -0.1f ), TVector( 0.3f, 1.75f, 0.1f ), TVector( -0.1f, 1.5f, -0.1f ), TVector( 0.1f, 1.75f, 0.1f ), TVector( 0.0f, 0.0f, -1.0f ), true, -47.5f, hints );
        ASSERT_TRUE( IsNear( hints.key.yawDegrees, 47.5f, 1e-5f ) && IsNear( hints.fill.yawDegrees, -47.5f, 1e-5f ) );
        // a facing direction that is not horizontal is projected, a degenerate one falls back
        CUnrealPresentationHints::Compute( TVector( -0.3f, 0.0f, -0.1f ), TVector( 0.3f, 1.75f, 0.1f ), TVector( -0.1f, 1.5f, -0.1f ), TVector( 0.1f, 1.75f, 0.1f ), TVector( 0.0f, 1.0f, 0.0f ), false, 0.0f, hints );
        ASSERT_TRUE( IsNear( hints.facing.v[ 2 ], 1.0f, 1e-5f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Character: the head in UV tile 1001 gets a material with the face textures, the body in tile 1002 keeps the body textures with the UVs of its texture" )
    try
    {
        CAssetScene scene;
        CAssetMaterial body;
        body.name = "body_shader_shader";
        body.baseColorTexture = CAssetTextureReference( 2 );
        body.normalTexture = CAssetTextureReference( 3 );
        scene.materials.push_back( body );
        CAssetMaterial cloth;
        cloth.name = "MI_Cloth";
        scene.materials.push_back( cloth );
        for ( UInt32 t=0; t<4; ++t )
            scene.textures.push_back( CAssetTexture() );

        // six triangles: two in tile 1001 (u 0 to 1), three in tile 1002 (u 1 to 2), one that crosses; each its own vertices
        CAssetMesh mesh;
        mesh.name = "SKM_Body_LOD0";
        CAssetSubMesh sub;
        sub.materialIndex = 0;
        const Float32 us[ 6 ][ 3 ] = { { 0.1f, 0.2f, 0.3f }, { 0.5f, 0.6f, 0.9f }, { 1.1f, 1.2f, 1.3f }, { 1.5f, 1.6f, 1.9f }, { 1.0f, 1.4f, 1.7f }, { 0.8f, 1.2f, 0.9f } };
        for ( UInt32 t=0; t<6; ++t )
        {
            for ( UInt32 c=0; c<3; ++c )
            {
                sub.positions.push_back( (Float32) t ); sub.positions.push_back( (Float32) c ); sub.positions.push_back( 0.0f );
                sub.normals.push_back( 0.0f ); sub.normals.push_back( 0.0f ); sub.normals.push_back( 1.0f );
                sub.uvSets[ 0 ].push_back( us[ t ][ c ] ); sub.uvSets[ 0 ].push_back( 0.5f );
                sub.indices.push_back( t * 3 + c );
            }
        }
        mesh.subMeshes.push_back( sub );
        // a second level whose only section lies in tile 1001: it changes material as a whole
        CAssetMesh level1;
        level1.name = "SKM_Body_LOD1";
        CAssetSubMesh eyes;
        eyes.materialIndex = 0;
        for ( UInt32 c=0; c<3; ++c )
        {
            eyes.positions.push_back( 0.0f ); eyes.positions.push_back( (Float32) c ); eyes.positions.push_back( 1.0f );
            eyes.uvSets[ 0 ].push_back( 0.25f ); eyes.uvSets[ 0 ].push_back( 0.25f );
            eyes.indices.push_back( c );
        }
        level1.subMeshes.push_back( eyes );
        scene.meshes.push_back( mesh );
        scene.meshes.push_back( level1 );

        CConversionReport report;
        ASSERT_TRUE( 0 == CUnrealMetaHumanTextures::SplitHeadTile( 1, 0, 1, scene, report ) );     // the cloth material has no tiles
        ASSERT_TRUE( 0 == CUnrealMetaHumanTextures::SplitHeadTile( 0, -1, -1, scene, report ) );   // no face textures
        ASSERT_TRUE( 3 == CUnrealMetaHumanTextures::SplitHeadTile( 0, 0, 1, scene, report ) );
        ASSERT_TRUE( 3 == scene.materials.size() && "body_shader_shader_UDIM1001" == scene.materials[ 2 ].name );
        ASSERT_TRUE( 0 == scene.materials[ 2 ].baseColorTexture.textureIndex && 1 == scene.materials[ 2 ].normalTexture.textureIndex );
        ASSERT_TRUE( 2 == scene.materials[ 0 ].baseColorTexture.textureIndex && 3 == scene.materials[ 0 ].normalTexture.textureIndex );

        const CAssetMesh& split = scene.meshes[ 0 ];
        ASSERT_TRUE( 2 == split.subMeshes.size() && 0 == split.subMeshes[ 0 ].materialIndex && 2 == split.subMeshes[ 1 ].materialIndex );
        ASSERT_TRUE( 4 == split.subMeshes[ 0 ].GetTriangleCount() && 12 == split.subMeshes[ 0 ].GetVertexCount() && 2 == split.subMeshes[ 1 ].GetTriangleCount() && 6 == split.subMeshes[ 1 ].GetVertexCount() );
        // the head keeps its UVs; the body's tile 1002 is now 0 to 1; the triangle that crosses keeps its UVs and the body material
        ASSERT_TRUE( 0.1f == split.subMeshes[ 1 ].uvSets[ 0 ][ 0 ] && 0.9f == split.subMeshes[ 1 ].uvSets[ 0 ][ 10 ] );
        ASSERT_TRUE( fabs( split.subMeshes[ 0 ].uvSets[ 0 ][ 0 ] - 0.1f ) < 1e-6f && fabs( split.subMeshes[ 0 ].uvSets[ 0 ][ 2 * 5 ] - 0.9f ) < 1e-6f );
        ASSERT_TRUE( 0.0f == split.subMeshes[ 0 ].uvSets[ 0 ][ 2 * 6 ] && 0.8f == split.subMeshes[ 0 ].uvSets[ 0 ][ 2 * 9 ] && 1.2f == split.subMeshes[ 0 ].uvSets[ 0 ][ 2 * 10 ] );
        // the head triangles kept their place
        ASSERT_TRUE( 0.0f == split.subMeshes[ 1 ].positions[ 0 ] && 1.0f == split.subMeshes[ 1 ].positions[ 9 ] );
        ASSERT_TRUE( 2 == scene.meshes[ 1 ].subMeshes[ 0 ].materialIndex && 0.25f == scene.meshes[ 1 ].subMeshes[ 0 ].uvSets[ 0 ][ 0 ] );
        ASSERT_TRUE( 1 == CountReportItems( report, REPORTITEM_INFO, "tile 1001 (u 0 to 1) is the head" ) && 1 == CountReportItems( report, REPORTITEM_WARNING, "cross the UV tiles" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Character: the head position and the head bounds come from the skeleton and the weights of the assembled body; without them the top of the bounds stands in" )
    try
    {
        CUnrealSkeletalMesh body;
        MakeBodyMesh( 300, body );
        GUCEF::vector< Int32 > slots( 1, 0 );
        CAssetScene scene;
        scene.coordinateSystem = CUnrealSkeletalMeshConverter::GetCoordinateSystem();
        CConversionReport report;
        CUnrealCharacterAssembler::SResult assembly;
        ASSERT_TRUE( CUnrealCharacterAssembler::Assemble( "Testy", CUnrealCharacterAssembler::SPart( &body, &slots ), CUnrealCharacterAssembler::TPartVector(), scene, report, assembly ) );
        CUnrealMetaHumanCharacter character;
        character.hasLightRotation = true;
        character.lightRotation = -20.0f;
        CUnrealPresentationHints::SHints hints;
        ASSERT_TRUE( CUnrealPresentationHints::Build( scene, assembly, &character, "Testy", report, hints ) );
        // the head joint stands 1.6 m high (95 cm plus 65 cm), the source is centimetres with Z up
        ASSERT_TRUE( hints.hasHeadJoint && !hints.headEstimated );
        ASSERT_TRUE( IsNear( hints.headJoint.v[ 1 ], 1.6f, 1e-4f ) && IsNear( hints.headJoint.v[ 0 ], 0.0f, 1e-4f ) && IsNear( hints.headJoint.v[ 2 ], 0.0f, 1e-4f ) );
        // the vertices of the head run from 1.5 to 1.75 m
        ASSERT_TRUE( IsNear( hints.headMin.v[ 1 ], 1.5f, 2e-3f ) && IsNear( hints.headMax.v[ 1 ], 1.75f, 2e-3f ) );
        ASSERT_TRUE( IsNear( hints.boundsMin.v[ 1 ], 0.0f, 2e-3f ) && IsNear( hints.boundsMax.v[ 1 ], 1.75f, 2e-3f ) );
        // a mesh faces +Y in the source, which is -Z in the intermediate
        ASSERT_TRUE( IsNear( hints.facing.v[ 0 ], 0.0f, 1e-5f ) && IsNear( hints.facing.v[ 2 ], -1.0f, 1e-5f ) && IsNear( hints.up.v[ 1 ], 1.0f, 1e-5f ) );
        ASSERT_TRUE( IsNear( hints.portrait.target.v[ 1 ], 1.55f, 3e-3f ) && IsNear( hints.portrait.frameHeight, 0.5f, 4e-3f ) );
        ASSERT_TRUE( IsNear( hints.key.yawDegrees, 20.0f, 1e-5f ) );
        // the tree for the extras
        CORE::CDataNode node( CUnrealPresentationHints::ExtrasKey );
        CUnrealPresentationHints::ToDataNode( hints, &character, "Testy", node );
        ASSERT_TRUE( "character" == node.GetAttributeValue( "kind" ).AsString() && "portrait" == node.GetAttributeValue( "defaultView" ).AsString() && "gucefPresentation/1" == node.GetAttributeValue( "schema" ).AsString() );
        const CORE::CDataNode* view = node.FindChild( "views" ) ? node.FindChild( "views" )->FindChild( "portrait" ) : GUCEF_NULL;
        ASSERT_TRUE( GUCEF_NULL != view && IsNear( view->GetAttributeValue( "fovDegrees" ).AsFloat32(), 30.0f, 1e-5f ) && GUCEF_NULL != view->FindChild( "position" ) && GUCEF_NULL != view->FindChild( "target" ) );
        ASSERT_TRUE( node.FindChild( "head" ) && node.FindChild( "head" )->GetAttributeValue( "hasJoint" ).AsBool() && !node.FindChild( "head" )->GetAttributeValue( "estimated" ).AsBool() );
        ASSERT_TRUE( node.FindChild( "lights" ) && node.FindChild( "lights" )->FindChild( "key" ) && node.FindChild( "source" ) && IsNear( node.FindChild( "source" )->GetAttributeValue( "lightRotation" ).AsFloat32(), -20.0f, 1e-5f ) );

        // too few vertices on the head: the top of the bounds stands in and the report says so
        CUnrealSkeletalMesh sparse;
        MakeBodyMesh( 20, sparse );
        CAssetScene sparseScene;
        sparseScene.coordinateSystem = CUnrealSkeletalMeshConverter::GetCoordinateSystem();
        CConversionReport sparseReport;
        CUnrealCharacterAssembler::SResult sparseAssembly;
        ASSERT_TRUE( CUnrealCharacterAssembler::Assemble( "Testy", CUnrealCharacterAssembler::SPart( &sparse, &slots ), CUnrealCharacterAssembler::TPartVector(), sparseScene, sparseReport, sparseAssembly ) );
        CUnrealPresentationHints::SHints sparseHints;
        ASSERT_TRUE( CUnrealPresentationHints::Build( sparseScene, sparseAssembly, GUCEF_NULL, "Testy", sparseReport, sparseHints ) );
        ASSERT_TRUE( sparseHints.headEstimated && sparseHints.hasHeadJoint && 1 == CountReportItems( sparseReport, REPORTITEM_APPROXIMATION, "top 15 percent" ) );
        ASSERT_TRUE( IsNear( sparseHints.headMax.v[ 1 ] - sparseHints.headMin.v[ 1 ], 0.15f * 1.75f, 4e-3f ) );
        ASSERT_TRUE( IsNear( sparseHints.key.yawDegrees, 45.0f, 1e-5f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Hidden face map: a triangle is hidden when every texel it covers is below the cull value; the corners count for small triangles" )
    try
    {
        // 8 x 8 texels: the left half white, the right half black, column 3 grey (128)
        CUnrealHiddenFaceMap::TByteVector pixels( 8 * 8 * 4, 255 );
        for ( UInt32 y=0; y<8; ++y )
            for ( UInt32 x=0; x<8; ++x )
                for ( UInt32 c=0; c<3; ++c )
                    pixels[ ( y * 8 + x ) * 4 + c ] = x >= 4 ? 0 : ( 3 == x ? 128 : 255 );
        CUnrealHiddenFaceMap map;
        ASSERT_FALSE( map.SetImage( pixels, 8, 7, 4 ) );
        ASSERT_TRUE( map.SetImage( pixels, 8, 8, 4 ) && 8 == map.GetWidth() && 8 == map.GetHeight() );
        ASSERT_TRUE( fabs( map.GetMaxCullValue() - 0.1f ) < 1e-6f && fabs( map.GetMinKeepValue() - 0.9f ) < 1e-6f );
        ASSERT_TRUE( 0.0f == map.Sample( 0.9f, 0.5f ) && 1.0f == map.Sample( 0.1f, 0.5f ) && fabs( map.Sample( 0.4f, 0.1f ) - 128.0f / 255.0f ) < 1e-6f );
        ASSERT_TRUE( 1.0f == map.Sample( -1.0f, 2.0f ) );     // clamped to the edge
        const Float32 inBlack[ 3 ][ 2 ] = { { 0.55f, 0.1f }, { 0.95f, 0.1f }, { 0.75f, 0.9f } };
        ASSERT_TRUE( map.IsHidden( inBlack[ 0 ], inBlack[ 1 ], inBlack[ 2 ] ) );
        // a triangle that reaches one texel of the grey column is kept; a tiny one inside a black texel is hidden
        const Float32 touching[ 3 ][ 2 ] = { { 0.45f, 0.1f }, { 0.95f, 0.1f }, { 0.75f, 0.9f } };
        ASSERT_FALSE( map.IsHidden( touching[ 0 ], touching[ 1 ], touching[ 2 ] ) );
        const Float32 tiny[ 3 ][ 2 ] = { { 0.80f, 0.80f }, { 0.801f, 0.80f }, { 0.80f, 0.801f } };
        ASSERT_TRUE( map.IsHidden( tiny[ 0 ], tiny[ 1 ], tiny[ 2 ] ) );
        // with a cull value above the grey, the grey column hides too
        map.SetMaxCullValue( 0.6f );
        ASSERT_TRUE( map.IsHidden( touching[ 0 ], touching[ 1 ], touching[ 2 ] ) );
        Float32 hiddenShare = 0.0f, bandShare = 0.0f;
        map.GetCoverage( hiddenShare, bandShare );
        ASSERT_TRUE( fabs( hiddenShare - 0.625f ) < 1e-6f && 0.0f == bandShare );
        map.SetMaxCullValue( 0.1f );
        map.GetCoverage( hiddenShare, bandShare );
        ASSERT_TRUE( fabs( hiddenShare - 0.5f ) < 1e-6f && fabs( bandShare - 0.125f ) < 1e-6f );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Hidden face map: on a small body, a garment shell and a mask the body under the garment is hidden, kept as a submesh of the set, and the visible body no longer shows through" )
    try
    {
        // the body: a patch of 11 x 11 vertices, 0.2 m square, facing +z, in UV tile 1002 (u from 1 to 2) as a combined body stores
        // it; a second submesh in another material (the head tile) lies in the same place and is left alone. The garment shell lies
        // 1 cm in front of the part with x below 0.099 m and 5 mm behind the rest: there the body shows through
        CAssetScene scene;
        scene.name = "shell";
        CAssetMesh bodyMesh;
        bodyMesh.name = "BodyLod0";
        CAssetSubMesh patch;
        patch.materialIndex = 0;
        for ( UInt32 y=0; y<=10; ++y )
        {
            for ( UInt32 x=0; x<=10; ++x )
            {
                patch.positions.push_back( 0.02f * x ); patch.positions.push_back( 0.02f * y ); patch.positions.push_back( 0.0f );
                patch.normals.push_back( 0.0f ); patch.normals.push_back( 0.0f ); patch.normals.push_back( 1.0f );
                patch.uvSets[ 0 ].push_back( 1.0f + 0.1f * x ); patch.uvSets[ 0 ].push_back( 0.1f * y );
                if ( x < 10 && y < 10 )
                {
                    const UInt32 a = y * 11 + x;
                    patch.indices.push_back( a ); patch.indices.push_back( a + 1 ); patch.indices.push_back( a + 12 );
                    patch.indices.push_back( a ); patch.indices.push_back( a + 12 ); patch.indices.push_back( a + 11 );
                }
            }
        }
        bodyMesh.subMeshes.push_back( patch );
        CAssetSubMesh head( patch );
        head.materialIndex = 1;
        for ( size_t v=0; v<head.positions.size(); v+=3 )
            head.positions[ v + 1 ] += 1.0f;        // well away from the garment
        bodyMesh.subMeshes.push_back( head );
        scene.meshes.push_back( bodyMesh );
        CAssetMesh shell;
        shell.name = "Shell";
        const Float32 quads[ 2 ][ 3 ] = { { -0.01f, 0.099f, 0.01f }, { 0.099f, 0.21f, -0.005f } };
        for ( UInt32 q=0; q<2; ++q )
        {
            CAssetSubMesh quad;
            const Float32 x0 = quads[ q ][ 0 ], x1 = quads[ q ][ 1 ], z = quads[ q ][ 2 ];
            const Float32 corners[ 4 ][ 2 ] = { { x0, -0.01f }, { x1, -0.01f }, { x1, 0.21f }, { x0, 0.21f } };
            for ( UInt32 c=0; c<4; ++c )
            {
                quad.positions.push_back( corners[ c ][ 0 ] ); quad.positions.push_back( corners[ c ][ 1 ] ); quad.positions.push_back( z );
                quad.normals.push_back( 0.0f ); quad.normals.push_back( 0.0f ); quad.normals.push_back( 1.0f );
            }
            const UInt32 indices[] = { 0, 1, 2,  0, 2, 3 };
            quad.indices.assign( indices, indices + 6 );
            shell.subMeshes.push_back( quad );
        }
        scene.meshes.push_back( shell );
        const UInt32 root = scene.AddNode( "Character" );
        const UInt32 bodyNode = scene.AddNode( "Body", (Int32) root );
        scene.nodes[ bodyNode ].mesh = 0;
        scene.nodes[ bodyNode ].extras.SetAttribute( CLayerPenetrationValidator::PartAttribute, CLayerPenetrationValidator::BodyPart );
        const UInt32 shellNode = scene.AddNode( "Shell", (Int32) root );
        scene.nodes[ shellNode ].mesh = 1;
        scene.nodes[ shellNode ].extras.SetAttribute( CLayerPenetrationValidator::PartAttribute, CLayerPenetrationValidator::GarmentPart );

        CLayerPenetrationValidator::SResult before;
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, before ) );
        ASSERT_TRUE( 242 == before.bodyVertices && 66 == before.pokingVertices && 55 == before.coveredVertices );

        // the mask: 20 x 20 grey texels, black from u 0.35 on (the shell does not cover the body in front from u 0.5 on); the texel
        // edge lies between the vertex columns, so the rounding of the UVs does not decide
        CUnrealHiddenFaceMap::TByteVector pixels( 20 * 20, 255 );
        for ( UInt32 y=0; y<20; ++y )
            for ( UInt32 x=7; x<20; ++x )
                pixels[ y * 20 + x ] = 0;
        CUnrealHiddenFaceMap map;
        ASSERT_TRUE( map.SetImage( pixels, 20, 20, 1 ) );
        CAssetHiddenFaceSet set;
        set.name = "Shell";
        set.garments.push_back( "Shell" );
        set.active = true;
        CUnrealHiddenFaceMap::TInt32Set leaveAlone;
        leaveAlone.insert( 1 );
        CUnrealHiddenFaceMap::SResult result;
        map.HideFaces( scene.meshes[ 0 ], CAssetHiddenFaceSet::BitOf( 0 ), 1, leaveAlone, result );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Hidden face map test: " << result.triangles << " triangles in the tile, " << result.hiddenTriangles << " hidden, " << result.otherTriangles << " other, " << result.newSubMeshes << " new submeshes" );
        set.hiddenTriangles = result.hiddenTriangles;
        scene.hiddenFaceSets.push_back( set );
        // the triangles of the quads from x 0.08 m on are hidden: 6 of 10 columns of quads (the corners at u 0.4 lie in black texels)
        ASSERT_TRUE( 200 == result.triangles && 120 == result.hiddenTriangles && 200 == result.otherTriangles && 1 == result.newSubMeshes );
        ASSERT_TRUE( 3 == scene.meshes[ 0 ].subMeshes.size() );
        const CAssetSubMesh& shown = scene.meshes[ 0 ].subMeshes[ 0 ];
        const CAssetSubMesh& kept = scene.meshes[ 0 ].subMeshes[ 1 ];
        const CAssetSubMesh& hidden = scene.meshes[ 0 ].subMeshes[ 2 ];
        ASSERT_TRUE( 0 == shown.hiddenBy && 80 == shown.GetTriangleCount() && 0 == shown.materialIndex );
        ASSERT_TRUE( 0 == kept.hiddenBy && 1 == kept.materialIndex && 200 == kept.GetTriangleCount() );
        ASSERT_TRUE( 1 == hidden.hiddenBy && 120 == hidden.GetTriangleCount() && 0 == hidden.materialIndex && 77 == hidden.GetVertexCount() );
        // nothing is lost: the body has all its triangles
        ASSERT_TRUE( 400 == scene.meshes[ 0 ].GetTriangleCount() );

        CLayerPenetrationValidator::SResult after;
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, after ) );
        ASSERT_TRUE( 0 == after.pokingVertices && 77 == after.hiddenBodyVertices && 55 + 121 == after.bodyVertices && 55 == after.coveredVertices );
        // taking the outfit off brings the body back as it was
        scene.hiddenFaceSets[ 0 ].active = false;
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, after ) && 66 == after.pokingVertices && 0 == after.hiddenBodyVertices );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Corpus: the importer assembles each MetaHuman package into one character with a skeleton, garments, textures and presentation hints: golden numbers" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString archiveDir = CorpusSetting( "archivedir" );
    if ( archiveDir.IsNULLOrEmpty() || !CORE::DirExists( archiveDir ) )
        GUCEF_TESTFW_SKIP( "The corpus manifest names no archive folder" );
    try
    {
        CNoOodleToolScope noTool;
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.MountArchive( "unreal_character_corpus", archiveDir, VFS::CVFS::FileSystemArchiveTypeName, "charcorpus", false, false, archiveDir ) );
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        ASSERT_TRUE( WriteWorkFile( "mhpack/readme.txt", GUCEF::vector< UInt8 >( 3, 'x' ) ) );
        const char* packages[] = { "scarletjohansson", "margaretqualley", "emiliaclarke" };
        CORE::CString summary;
        for ( UInt32 p=0; p<3; ++p )
        {
            CAssetPackManifest manifest;
            manifest.id = "mhtest";
            manifest.source = WorkVfsPath( "mhpack" );
            manifest.importer = "unreal";
            manifest.intermediate = WorkVfsPath( CORE::CString( "mhintermediate_" ) + packages[ p ] );
            CAssetPackContext context;
            ASSERT_TRUE( context.Open( manifest ) );
            CAssetScene scene;
            CConversionReport report;
            ASSERT_TRUE( importer->Import( CORE::CString( "charcorpus/" ) + packages[ p ] + ".mhpkg", context, scene, report ) );
            // one scene, one root node named after the character, one skin and the hints in the extras
            ASSERT_TRUE( 1 == scene.skins.size() && scene.rootNodes.size() >= 1 && scene.nodes[ 0 ].name == scene.name && !scene.name.IsNULLOrEmpty() );
            const CORE::CDataNode* presentation = scene.extras.FindChild( CUnrealPresentationHints::ExtrasKey );
            const CORE::CDataNode* record = scene.extras.FindChild( CUnrealPresentationHints::CharacterKey );
            ASSERT_TRUE( GUCEF_NULL != presentation && GUCEF_NULL != record );
            ASSERT_TRUE( "portrait" == presentation->GetAttributeValue( "defaultView" ).AsString() );
            UInt32 skinned = 0;
            UInt32 lodGroups = (UInt32) scene.lodGroups.size();
            for ( size_t n=0; n<scene.nodes.size(); ++n )
                if ( scene.nodes[ n ].mesh >= 0 )
                {
                    ASSERT_TRUE( 0 == scene.nodes[ n ].skin );
                    ++skinned;
                }
            ASSERT_TRUE( skinned == scene.meshes.size() && skinned >= 4 );
            // the skeleton of the combined body: 342 joints, nothing added by the garments
            ASSERT_TRUE( 342 == scene.skins[ 0 ].joints.size() );

            // the hints in the extras are read back from the tree
            const CORE::CDataNode* head = presentation->FindChild( "head" );
            const CORE::CDataNode* bounds = presentation->FindChild( "bounds" );
            const CORE::CDataNode* views = presentation->FindChild( "views" );
            ASSERT_TRUE( GUCEF_NULL != head && GUCEF_NULL != bounds && GUCEF_NULL != views && GUCEF_NULL != views->FindChild( "portrait" ) );
            ASSERT_TRUE( !head->GetAttributeValue( "estimated" ).AsBool() && head->GetAttributeValue( "hasJoint" ).AsBool() );
            const Float32 height = bounds->GetAttributeValue( "height" ).AsFloat32();
            const Float32 headHeight = head->GetAttributeValue( "height" ).AsFloat32();
            ASSERT_TRUE( height > 1.4f && height < 2.1f );
            ASSERT_TRUE( headHeight > 0.15f && headHeight < 0.35f );
            const CORE::CDataNode* portrait = views->FindChild( "portrait" );
            const CORE::CDataNode* target = portrait->FindChild( "target" );
            ASSERT_TRUE( target->GetAttributeValue( "y" ).AsFloat32() > 0.75f * height );

            UInt32 images = 0;
            UInt32 imageFiles = 0;
            for ( size_t t=0; t<scene.textures.size(); ++t )
            {
                ++images;
                if ( !scene.textures[ t ].sources.empty() && vfs.FileExists( scene.textures[ t ].sources[ 0 ] ) )
                    ++imageFiles;
            }
            ASSERT_TRUE( images == imageFiles );
            UInt32 mapped = 0;
            for ( size_t m=0; m<scene.materials.size(); ++m )
                if ( !scene.materials[ m ].source.shaderName.IsNULLOrEmpty() && scene.materials[ m ].source.shaderName != "Unreal/unknown" )
                    ++mapped;
            const CORE::CDataNode* source = presentation->FindChild( "source" );
            const CORE::CDataNode* light = presentation->FindChild( "lights" )->FindChild( "key" );
            summary += CORE::CString( packages[ p ] ) + " | scene " + scene.name + " | nodes " + CORE::UInt64ToString( scene.nodes.size() ) + " | meshes " + CORE::UInt64ToString( scene.meshes.size() ) + " | skins " + CORE::UInt64ToString( scene.skins.size() ) +
                " | joints " + CORE::UInt64ToString( scene.skins[ 0 ].joints.size() ) + " | lod groups " + CORE::UInt32ToString( lodGroups ) + " | materials " + CORE::UInt64ToString( scene.materials.size() ) + " (" + CORE::UInt32ToString( mapped ) + " mapped) | textures " + CORE::UInt32ToString( images ) +
                " | vertices " + CORE::UInt32ToString( scene.GetVertexCount() ) + " | triangles " + CORE::UInt32ToString( scene.GetTriangleCount() ) + "\n";
            summary += "  extras | camera frame " + source->GetAttributeValue( "cameraFrame" ).AsString() + " | environment " + source->GetAttributeValue( "characterEnvironment" ).AsString() + " | light rotation " +
                ( source->GetAttributeValue( "hasLightRotation" ).AsBool() ? Num( source->GetAttributeValue( "lightRotation" ).AsFloat32() ) : CORE::CString( "none" ) ) + " | key yaw " + Num( light->GetAttributeValue( "yawDegrees" ).AsFloat32() ) + "\n";
            summary += "  height " + Num( height ) + " | head height " + Num( headHeight ) + " | portrait target y " + Num( target->GetAttributeValue( "y" ).AsFloat32() ) + " | distance " + Num( portrait->GetAttributeValue( "distance" ).AsFloat32() ) + "\n";
            const CORE::CDataNode* textures = record->FindChild( "textures" );
            UInt32 states[ 3 ] = { 0, 0, 0 };
            for ( CORE::CDataNode::const_iterator t = textures->ConstBegin(); t != textures->ConstEnd(); ++t )
            {
                const CORE::CString state = (*t)->GetAttributeValue( "state" ).AsString();
                ++states[ "image" == state ? 0 : ( "needs the oodle provider" == state ? 1 : 2 ) ];
            }
            summary += "  character textures " + CORE::UInt64ToString( textures->GetNrOfDirectChildNodes() ) + " | images " + CORE::UInt32ToString( states[ 0 ] ) + " | need the oodle provider " + CORE::UInt32ToString( states[ 1 ] ) + " | other " + CORE::UInt32ToString( states[ 2 ] ) +
                " | unassigned " + CORE::UInt32ToString( record->FindChild( "unassignedTextures" ) ? record->FindChild( "unassignedTextures" )->GetAttCount() : 0 ) + " | wardrobe items " + CORE::UInt32ToString( record->FindChild( "wardrobe" ) ? record->FindChild( "wardrobe" )->GetAttCount() : 0 ) + "\n";
            // what the assembly reports about each garment: matched bones, extra joints, bones with another parent or bind pose
            for ( size_t e=0; e<report.GetEntries().size(); ++e )
                for ( size_t i=0; i<report.GetEntries()[ e ].items.size(); ++i )
                {
                    const CConversionReportItem& item = report.GetEntries()[ e ].items[ i ];
                    if ( REPORTITEM_INFO == item.kind && 0 == item.message.HasSubstr( "Garment skinned" ) )
                        summary += "  " + item.subject + ": " + item.message.SubstrFromRange( item.message.HasSubstr( "by joint name: " ) + 15, item.message.Length() ) + "\n";
                }
            summary += "  skipped";
            for ( size_t e=0; e<report.GetEntries().size(); ++e )
                for ( size_t i=0; i<report.GetEntries()[ e ].items.size(); ++i )
                    if ( REPORTITEM_SKIPPED == report.GetEntries()[ e ].items[ i ].kind && report.GetEntries()[ e ].items[ i ].message.HasSubstr( "Not " ) == 0 )
                        summary += " " + report.GetEntries()[ e ].items[ i ].subject;
            summary += "\n  meshes";
            for ( size_t n=0; n<scene.nodes.size(); ++n )
                if ( scene.nodes[ n ].lodGroup >= 0 || ( scene.nodes[ n ].mesh >= 0 && scene.nodes[ n ].parent >= 0 && scene.nodes[ (size_t) scene.nodes[ n ].parent ].lodGroup < 0 ) )
                    summary += " " + scene.nodes[ n ].name;
            summary += "\n";
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Character " << packages[ p ] << ": " << scene.nodes.size() << " nodes, " << scene.meshes.size() << " meshes, " << images << " images, head " << headHeight << " m, height " << height << " m" );
            for ( size_t i=0; i<scene.textures.size(); ++i )
                if ( !scene.textures[ i ].sources.empty() )
                    vfs.DeleteFile( scene.textures[ i ].sources[ 0 ], true );
        }
        vfs.UnmountArchiveByName( "unreal_character_corpus" );
        CORE::CString problem;
        const bool golden = CheckGolden( "character_metahuman", summary, problem );
        if ( !golden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
        ASSERT_TRUE( golden );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
