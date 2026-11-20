/*
 *  assetpluginUNREAL_TestApp: Tests of the forms of the UE 4.25 to 4.27 and UE 5.0 to 5.2 packages and of the classification of packages
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

#include <string.h>

#ifndef GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_CORE_ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define GUCEF_CORE_ASSETCONVERTERLIB_H
#endif /* GUCEF_CORE_ASSETCONVERTERLIB_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#include "assetpluginUNREAL_CUnrealCustomVersionIds.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#include "assetpluginUNREAL_CUnrealEditorBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#include "assetpluginUNREAL_CUnrealLegacyBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYPAYLOADREADER_H
#include "assetpluginUNREAL_CUnrealLegacyPayloadReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYPAYLOADREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYPAYLOADREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H
#include "assetpluginUNREAL_CUnrealMeshDescription.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H
#include "assetpluginUNREAL_CUnrealStaticMeshReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H
#include "assetpluginUNREAL_CUnrealTextureConverter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#include "assetpluginUNREAL_CUnrealObjectPathResolver.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#include "assetpluginUNREAL_CUnrealPngWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H
#include "TestUnrealWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H ? */

#include "TestUnrealTexture.h"
#include "TestUnrealLegacyForms.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define ERRORHERE       GUCEF_TESTFW_ERRORHERE

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CTestType T( const char* name ) { return CTestType( name ); }
static CTestType T( const char* name, const CTestType& a ) { return CTestType( name, a ); }

/*-------------------------------------------------------------------------*/

/** The bytes compressed as a zlib stream by the codec of the registry; false when there is no zlib codec */
static bool
ZlibCompress( const UInt8* bytes, size_t size, GUCEF::vector< UInt8 >& out )
{
    CORE::CCodecRegistry::TICodecPtr codec = CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, CUnrealLegacyPayloadReader::ZlibCodecName );
    if ( codec.IsNULL() )
        return false;
    CORE::CDynamicBuffer source;
    source.CopyFrom( (UInt32) size, bytes );
    CORE::CDynamicBuffer dest;
    CORE::CDynamicBufferAccess sourceAccess( &source, false );
    CORE::CDynamicBufferAccess destAccess( &dest, false );
    if ( !codec->Encode( sourceAccess, destAccess ) )
        return false;
    out.assign( static_cast< const UInt8* >( dest.GetConstBufferPtr() ), static_cast< const UInt8* >( dest.GetConstBufferPtr() ) + dest.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The chunked zlib form of legacy bulk data: the package tag, the chunk size, the totals, the sizes of the chunks, the chunks */
static bool
BuildZlibChunks( const GUCEF::vector< UInt8 >& raw, UInt32 chunkSize, GUCEF::vector< UInt8 >& out )
{
    GUCEF::vector< GUCEF::vector< UInt8 > > chunks;
    for ( size_t at=0; at<raw.size(); at+=chunkSize )
    {
        const size_t size = raw.size() - at < chunkSize ? raw.size() - at : chunkSize;
        GUCEF::vector< UInt8 > chunk;
        if ( !ZlibCompress( &raw[ at ], size, chunk ) )
            return false;
        chunks.push_back( chunk );
    }
    CTestByteWriter w;
    UInt64 compressed = 0;
    for ( size_t c=0; c<chunks.size(); ++c )
        compressed += chunks[ c ].size();
    w.I64( (Int64) UE_PACKAGE_FILE_TAG );
    w.I64( chunkSize );
    w.I64( (Int64) compressed );
    w.I64( (Int64) raw.size() );
    for ( size_t c=0; c<chunks.size(); ++c )
    {
        w.I64( (Int64) chunks[ c ].size() );
        const size_t rawSize = raw.size() - c * chunkSize < chunkSize ? raw.size() - c * chunkSize : chunkSize;
        w.I64( (Int64) rawSize );
    }
    for ( size_t c=0; c<chunks.size(); ++c )
        w.Raw( &chunks[ c ][ 0 ], chunks[ c ].size() );
    out = w.bytes;
    return true;
}

/*-------------------------------------------------------------------------*/

/** A bit array as the UE 4 mesh description stores it: the number of bits, then the words, all bits set */
static void
AllBits( CTestByteWriter& w, UInt32 count )
{
    w.U32( count );
    for ( UInt32 word=0; word<( count + 31 ) / 32; ++word )
    {
        const UInt32 left = count - word * 32;
        w.U32( left >= 32 ? 0xFFFFFFFF : ( ( 1U << left ) - 1 ) );
    }
}

/*-------------------------------------------------------------------------*/

static void
Ue4Str( CTestByteWriter& w, const char* text )
{
    w.U32( (UInt32) strlen( text ) + 1 );
    w.Raw( text, strlen( text ) + 1 );
}

/*-------------------------------------------------------------------------*/

/** An attribute of the UE 4 layout with one channel of 'count' values of 'valueBytes' each, the value bytes given by 'fill' */
static void
Ue4Attribute( CTestByteWriter& w, const char* name, UInt32 type, UInt32 count, UInt32 valueBytes, const GUCEF::vector< UInt8 >& values, UInt32 defaultBytes, UInt32 channels = 1 )
{
    Ue4Str( w, name );
    w.U32( type ); w.U32( count ); w.U32( channels );
    for ( UInt32 c=0; c<channels; ++c )
    {
        w.U32( valueBytes ); w.U32( count );
        w.Raw( values.empty() ? GUCEF_NULL : &values[ 0 ], values.size() );
    }
    w.Zero( defaultBytes );
    w.U32( 1 );
}

/*-------------------------------------------------------------------------*/

static void
PushFloats( GUCEF::vector< UInt8 >& out, const Float32* values, UInt32 count )
{
    const UInt8* p = reinterpret_cast< const UInt8* >( values );
    out.insert( out.end(), p, p + count * 4 );
}

/*-------------------------------------------------------------------------*/

/**
 *  A quad in the mesh description layout of UE 4.25 to 4.27: four vertices, six vertex instances (0 1 2 and 0 2 3), two polygons
 *  with a triangle each; with two groups the second triangle is in polygon group 1 ("Mat_B"), else both are in group 0 ("Mat_A")
 */
static void
BuildUe4QuadDescription( bool twoGroups, GUCEF::vector< UInt8 >& bytes )
{
    CTestByteWriter w;
    const UInt32 groups = twoGroups ? 2 : 1;
    AllBits( w, 4 );                                                    // vertices
    AllBits( w, 6 );                                                    // vertex instances and their vertices
    const Int32 instanceVertex[ 6 ] = { 0, 1, 2, 0, 2, 3 };
    for ( UInt32 i=0; i<6; ++i ) w.I32( instanceVertex[ i ] );
    AllBits( w, 5 );                                                    // edges and their vertices
    const Int32 edges[ 10 ] = { 0, 1, 1, 2, 2, 0, 2, 3, 3, 0 };
    for ( UInt32 i=0; i<10; ++i ) w.I32( edges[ i ] );
    AllBits( w, 2 );                                                    // polygons: an empty perimeter and the group
    w.U32( 0 ); w.U32( 0 );
    w.U32( 0 ); w.U32( twoGroups ? 1 : 0 );
    AllBits( w, groups );                                               // polygon groups

    // the attributes of the vertices
    w.U32( 4 ); w.U32( 2 );
    { GUCEF::vector< UInt8 > v; const Float32 p[ 12 ] = { 0,0,0, 100,0,0, 100,100,0, 0,100,0 }; PushFloats( v, p, 12 ); Ue4Attribute( w, "Position ", 1, 4, 12, v, 12 ); }
    { GUCEF::vector< UInt8 > v( 16, 0 ); Ue4Attribute( w, "CornerSharpness", 3, 4, 4, v, 4 ); }
    // the attributes of the vertex instances: two UV sets in two channels
    w.U32( 6 ); w.U32( 5 );
    { GUCEF::vector< UInt8 > v; const Float32 uv[ 12 ] = { 0,0, 1,0, 1,1, 0,0, 1,1, 0,1 }; PushFloats( v, uv, 12 ); Ue4Attribute( w, "TextureCoordinate", 2, 6, 8, v, 8, 2 ); }
    { GUCEF::vector< UInt8 > v; for ( UInt32 i=0; i<6; ++i ) { const Float32 n[ 3 ] = { 0, 0, 1 }; PushFloats( v, n, 3 ); } Ue4Attribute( w, "Normal", 1, 6, 12, v, 12 ); }
    { GUCEF::vector< UInt8 > v; for ( UInt32 i=0; i<6; ++i ) { const Float32 n[ 3 ] = { 1, 0, 0 }; PushFloats( v, n, 3 ); } Ue4Attribute( w, "Tangent", 1, 6, 12, v, 12 ); }
    { GUCEF::vector< UInt8 > v; for ( UInt32 i=0; i<6; ++i ) { const Float32 s = -1.0f; PushFloats( v, &s, 1 ); } Ue4Attribute( w, "BinormalSign", 3, 6, 4, v, 4 ); }
    { GUCEF::vector< UInt8 > v; for ( UInt32 i=0; i<6; ++i ) { const Float32 c[ 4 ] = { 1, 0.5f, 0.25f, 1 }; PushFloats( v, c, 4 ); } Ue4Attribute( w, "Color", 0, 6, 16, v, 16 ); }
    // the attributes of the edges: a boolean is one byte per value, its default 32 bits
    w.U32( 5 ); w.U32( 1 );
    { GUCEF::vector< UInt8 > v( 5, 0 ); v[ 1 ] = 1; Ue4Attribute( w, "IsHard", 5, 5, 1, v, 4 ); }
    // the polygons: no attributes
    w.U32( 2 ); w.U32( 0 );
    // the polygon groups: the names of their material slots
    w.U32( groups ); w.U32( 1 );
    Ue4Str( w, "ImportedMaterialSlotName" ); w.U32( 6 ); w.U32( groups ); w.U32( 1 );
    w.U32( groups ); Ue4Str( w, "Mat_A" ); if ( twoGroups ) Ue4Str( w, "Mat_B" );
    Ue4Str( w, "None" ); w.U32( 0 );
    // the triangles: three vertex instances and the polygon of each, then their attributes (none)
    AllBits( w, 2 );
    w.I32( 0 ); w.I32( 1 ); w.I32( 2 ); w.I32( 0 );
    w.I32( 3 ); w.I32( 4 ); w.I32( 5 ); w.I32( 1 );
    w.U32( 2 ); w.U32( 0 );
    bytes = w.bytes;
}

/*-------------------------------------------------------------------------*/

/**
 *  A package of UE 4.25 to 4.27 with one StaticMesh: two material slots, one source model whose mesh description is legacy bulk
 *  data at the end of the file (flags as given: 0x2 compresses it with zlib in chunks, 0x40 puts it inline); 'modelValid' false
 *  writes a source model without data; 'stripped' sets the strip flag of the editor data
 */
static bool
BuildUe4StaticMeshPackage( Int32 ue4, Int32 editorVersion, const GUCEF::vector< UInt8 >& description, UInt32 flags, bool modelValid, bool stripped, GUCEF::vector< UInt8 >& bytes )
{
    GUCEF::vector< UInt8 > stored = description;
    if ( 0 != ( flags & CUnrealLegacyBulkData::FLAG_COMPRESSED_ZLIB ) && !BuildZlibChunks( description, 64, stored ) )
        return false;

    CTestPackageWriter writer( -7, ue4, 0 );
    writer.packageName = "/Game/Meshes/SM_Quad";
    writer.customVersions.push_back( CUnrealCustomVersion( CUnrealCustomVersionIds::EditorObjectVersion(), editorVersion ) );
    const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/Engine" );
    writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "StaticMesh" );
    CTestByteWriter body;
    writer.BeginBody( body );
    CTestPropertyWriter p( body, writer.names, writer.ue4, writer.ue5 );
    CTestPropertyWriter::SOpen o = p.Begin( "StaticMaterials", T( "ArrayProperty", StructType( "StaticMaterial" ) ) );
    body.I32( 2 );
    CTestPropertyWriter::SOpen inner = p.BeginInnerStructTag( "StaticMaterials", "StaticMaterial" );
    for ( UInt32 s=0; s<2; ++s )
    {
        CTestPropertyWriter::SOpen a = p.Begin( "MaterialSlotName", T( "NameProperty" ) ); writer.names.WriteName( body, 0 == s ? "Slot_A" : "Slot_B" ); p.End( a );
        a = p.Begin( "ImportedMaterialSlotName", T( "NameProperty" ) ); writer.names.WriteName( body, 0 == s ? "Mat_A" : "Mat_B" ); p.End( a );
        p.None();
    }
    p.End( inner );
    p.End( o );
    o = p.Begin( "SourceModels", T( "ArrayProperty", StructType( "StaticMeshSourceModel" ) ) );
    body.I32( 1 );
    inner = p.BeginInnerStructTag( "SourceModels", "StaticMeshSourceModel" );
    p.None();
    p.End( inner );
    p.End( o );
    p.None();
    const Int64 scriptEnd = (Int64) body.Size();

    // the native data: guid flag, strip flags, cooked flag, body setup, navigation collision, two values, lighting guid, sockets
    body.Bool32( false );
    body.U8( stripped ? 1 : 0 ); body.U8( 0 );
    body.U32( 0 );
    body.I32( 0 ); body.I32( 0 );
    body.U32( 0xF1A240E5 ); body.U32( 0 );
    body.Guid( CUnrealGuid( 0x11, 0x22, 0x33, 0x44 ) );
    body.I32( 0 );
    // the source model: its flag, the legacy bulk data of its mesh description, the guid and whether the guid is a hash
    size_t offsetField = 0;
    body.U32( modelValid ? 1 : 0 );
    if ( modelValid )
    {
        body.U32( flags );
        body.I32( (Int32) description.size() );
        body.I32( (Int32) stored.size() );
        offsetField = body.Size();
        body.I64( 0 );
        if ( 0 != ( flags & CUnrealLegacyBulkData::FLAG_FORCE_INLINE_PAYLOAD ) )
            body.Raw( &stored[ 0 ], stored.size() );
        body.Guid( CUnrealGuid( 5, 6, 7, 8 ) );
        body.U32( 1 );
    }
    body.I32( 0 );
    writer.AddExport( -2, 0, "SM_Quad", body, scriptEnd );
    writer.Build( bytes );
    if ( modelValid && 0 == ( flags & CUnrealLegacyBulkData::FLAG_FORCE_INLINE_PAYLOAD ) )
    {
        // the payload follows the package: its offset is known once the package is built, the layout does not change with it
        writer.exports[ 0 ].body.Patch64( offsetField, (Int64) bytes.size() );
        const size_t size = bytes.size();
        writer.Build( bytes );
        if ( size != bytes.size() )
            return false;
        bytes.insert( bytes.end(), stored.begin(), stored.end() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** A package of UE 4.25 to 4.27 with one Texture2D whose PNG source is legacy bulk data compressed with zlib in chunks at the end of the file */
static bool
BuildUe4TexturePackage( const GUCEF::vector< UInt8 >& png, UInt32 width, UInt32 height, GUCEF::vector< UInt8 >& bytes )
{
    GUCEF::vector< UInt8 > stored;
    if ( !BuildZlibChunks( png, 256, stored ) )
        return false;
    CTestPackageWriter writer( -7, 522, 0 );
    writer.packageName = "/Game/Textures/T_Old";
    const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/Engine" );
    writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "Texture2D" );
    CTestByteWriter body;
    CTestPropertyWriter p( body, writer.names, writer.ue4, writer.ue5 );
    CTestPropertyWriter::SOpen o = p.Begin( "Source", StructType( "TextureSource" ) );
    {
        CTestPropertyWriter::SOpen a = p.Begin( "SizeX", T( "IntProperty" ) ); body.I32( (Int32) width ); p.End( a );
        a = p.Begin( "SizeY", T( "IntProperty" ) ); body.I32( (Int32) height ); p.End( a );
        a = p.Begin( "NumSlices", T( "IntProperty" ) ); body.I32( 1 ); p.End( a );
        a = p.Begin( "NumMips", T( "IntProperty" ) ); body.I32( 1 ); p.End( a );
        a = p.Begin( "bPNGCompressed", T( "BoolProperty" ), true ); p.End( a );
        a = p.Begin( "Format", T( "ByteProperty", T( "ETextureSourceFormat" ) ) ); writer.names.WriteName( body, "TSF_BGRA8" ); p.End( a );
        p.None();
    }
    p.End( o );
    p.None();
    const Int64 scriptEnd = (Int64) body.Size();
    body.Bool32( false );
    body.U8( 0 ); body.U8( 0 );
    body.U32( CUnrealLegacyBulkData::FLAG_PAYLOAD_AT_END_OF_FILE | CUnrealLegacyBulkData::FLAG_COMPRESSED_ZLIB );
    body.I32( (Int32) png.size() );
    body.I32( (Int32) stored.size() );
    const size_t offsetField = body.Size();
    body.I64( 0 );
    body.Bool32( false );
    writer.AddExport( -2, 0, "T_Old", body, scriptEnd );
    writer.Build( bytes );
    writer.exports[ 0 ].body.Patch64( offsetField, (Int64) bytes.size() );
    const size_t size = bytes.size();
    writer.Build( bytes );
    if ( size != bytes.size() )
        return false;
    bytes.insert( bytes.end(), stored.begin(), stored.end() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** A package of UE5 file version 'ue5' with a StaticMesh whose source model refers to a StaticMeshDescriptionBulkData object holding an editor bulk data record */
static void
BuildUe5StaticMeshPackage( Int32 legacy, Int32 ue5, const GUCEF::vector< UInt8 >& description, GUCEF::vector< UInt8 >& bytes )
{
    CTestPackageWriter writer( legacy, 522, ue5 );
    writer.packageName = "/Game/Meshes/SM_Quad5";
    const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/Engine" );
    writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "StaticMesh" );
    writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "StaticMeshDescriptionBulkData" );
    const UInt32 payload = writer.AddPayload( description, true );

    CTestByteWriter mesh;
    writer.BeginBody( mesh );
    CTestPropertyWriter p( mesh, writer.names, writer.ue4, writer.ue5 );
    CTestPropertyWriter::SOpen o = p.Begin( "SourceModels", T( "ArrayProperty", StructType( "StaticMeshSourceModel" ) ) );
    mesh.I32( 1 );
    CTestPropertyWriter::SOpen inner;
    if ( !p.IsNewLayout() ) inner = p.BeginInnerStructTag( "SourceModels", "StaticMeshSourceModel" );
    {
        CTestPropertyWriter::SOpen a = p.Begin( "StaticMeshDescriptionBulkData", T( "ObjectProperty" ) ); mesh.I32( 2 ); p.End( a );
        p.None();
    }
    if ( !p.IsNewLayout() ) p.End( inner );
    p.End( o );
    p.None();
    writer.AddExport( -2, 0, "SM_Quad5", mesh );

    CTestByteWriter holder;
    writer.BeginBody( holder );
    CTestPropertyWriter h( holder, writer.names, writer.ue4, writer.ue5 );
    h.None();
    const Int64 scriptEnd = (Int64) holder.Size();
    holder.Bool32( false );
    writer.WriteEditorBulkData( holder, payload );
    holder.Guid( CUnrealGuid( 1, 2, 3, 4 ) );
    holder.U32( 1 );
    writer.AddExport( -3, 1, "StaticMeshDescriptionBulkData_0", holder, scriptEnd );
    writer.Build( bytes );
}

/*-------------------------------------------------------------------------*/

/** A package with one export of the class; 'outerImport' makes the export belong to an object of another package */
static void
BuildClassPackage( const char* className, bool outerImport, GUCEF::vector< UInt8 >& bytes )
{
    CTestPackageWriter writer( -8, 522, 1008 );
    writer.packageName = "/Game/Misc/Holder";
    const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/Engine" );
    const Int32 classIndex = writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, className );
    const Int32 levelPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Game/Maps/Level" );
    const Int32 level = writer.AddImport( "/Script/Engine", "Level", levelPackage, "PersistentLevel" );
    CTestByteWriter body;
    writer.BeginBody( body );
    CTestPropertyWriter p( body, writer.names, writer.ue4, writer.ue5 );
    p.None();
    writer.AddExport( classIndex, outerImport ? level : 0, "Thing", body );
    writer.Build( bytes );
}

/*-------------------------------------------------------------------------*/

/** Imports the package with the asset selected in the report, as the pipeline does; the status the pipeline gives it */
static TAssetStatus
ImportAsPipeline( const CORE::CString& relativePath, CConversionReport& report, CORE::CString& firstReason )
{
    TAssetImporterPtr importer;
    if ( !CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) )
        return ASSETSTATUS_FAILED;
    CAssetPackManifest manifest;
    manifest.id = "forms";
    manifest.source = WorkVfsPath( "forms" );
    manifest.importer = "unreal";
    manifest.intermediate = WorkVfsPath( "forms_intermediate" );
    CAssetPackContext context;
    if ( !context.Open( manifest ) )
        return ASSETSTATUS_FAILED;
    importer->IndexPack( context );
    CConversionReportEntry& entry = report.SelectAsset( relativePath );
    CAssetScene scene;
    const bool imported = importer->Import( context.ToVfsPath( relativePath ), context, scene, report );
    firstReason.Clear();
    for ( size_t i=0; i<entry.items.size() && firstReason.IsNULLOrEmpty(); ++i )
        if ( REPORTITEM_ERROR == entry.items[ i ].kind || REPORTITEM_SKIPPED == entry.items[ i ].kind )
            firstReason = entry.items[ i ].message;
    if ( !imported && !entry.skipped && !entry.HasErrors() )
        report.AddError( "unreal", relativePath, "the import failed" );
    return entry.GetStatus();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnrealLegacyFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.LegacyForms" );

    GUCEF_TESTFW_TESTCASE( "UE 4 mesh description: the layout of 4.25 to 4.27 reads into the elements of the UE 5 layout, two polygon groups make two sections, damage never crashes" )
    try
    {
        GUCEF::vector< UInt8 > bytes;
        BuildUe4QuadDescription( true, bytes );
        CUnrealMeshDescription description;
        ASSERT_TRUE( description.ReadUe4Layout( &bytes[ 0 ], bytes.size() ) );
        const CUnrealMeshDescription::SElement* vertices = description.FindElement( "Vertices" );
        const CUnrealMeshDescription::SElement* instances = description.FindElement( "VertexInstances" );
        const CUnrealMeshDescription::SElement* triangles = description.FindElement( "Triangles" );
        ASSERT_TRUE( GUCEF_NULL != vertices && GUCEF_NULL != instances && GUCEF_NULL != triangles && GUCEF_NULL != description.FindElement( "PolygonGroups" ) );
        if ( GUCEF_NULL != vertices && GUCEF_NULL != instances && GUCEF_NULL != triangles )
        {
            ASSERT_TRUE( 4 == vertices->channels[ 0 ].GetAllocatedCount() && 100.0f == vertices->channels[ 0 ].Find( "Position" )->GetFloat( 0, 2, 0, 1 ) );
            ASSERT_TRUE( 3 == instances->channels[ 0 ].Find( "VertexIndex" )->GetInt32( 0, 5, 0 ) && 2 == instances->channels[ 0 ].Find( "TextureCoordinate" )->blocks.size() );
            ASSERT_TRUE( 4 == triangles->channels[ 0 ].Find( "VertexInstanceIndex" )->GetInt32( 0, 1, 1 ) && 1 == triangles->channels[ 0 ].Find( "PolygonGroupIndex" )->GetInt32( 0, 1, 0 ) );
        }
        CUnrealSkeletalMesh::TMaterialSlotVector slots( 2 );
        slots[ 0 ].importedSlotName = "Mat_B";
        slots[ 1 ].importedSlotName = "Mat_A";
        CUnrealStaticMesh::SLod lod;
        CORE::CString error;
        ASSERT_TRUE( CUnrealStaticMeshReader::BuildLod( description, slots, lod, error ) );
        ASSERT_TRUE( 2 == lod.sections.size() && 2 == lod.triangleCount && 6 == lod.vertexCount && 2 == lod.uvSetCount );
        if ( 2 == lod.sections.size() )
            ASSERT_TRUE( 1 == lod.sections[ 0 ].slot && "Mat_A" == lod.sections[ 0 ].groupName && 0 == lod.sections[ 1 ].slot && "Mat_B" == lod.sections[ 1 ].groupName );
        ASSERT_TRUE( 1.0f == lod.windingAgreement && !lod.sections[ 0 ].colours.empty() && -1.0f == lod.sections[ 0 ].tangents[ 3 ] );

        // one group: one section
        GUCEF::vector< UInt8 > single;
        BuildUe4QuadDescription( false, single );
        CUnrealMeshDescription one;
        ASSERT_TRUE( one.ReadUe4Layout( &single[ 0 ], single.size() ) && CUnrealStaticMeshReader::BuildLod( one, slots, lod, error ) && 1 == lod.sections.size() );

        // bytes behind the layout, every cut, flipped bytes: a reason or a result, never a crash
        GUCEF::vector< UInt8 > longer = bytes;
        longer.push_back( 0 );
        CUnrealMeshDescription tail;
        ASSERT_TRUE( !tail.ReadUe4Layout( &longer[ 0 ], longer.size() ) && tail.GetError().HasSubstr( "behind the end" ) >= 0 );
        UInt32 refused = 0;
        for ( size_t cut=0; cut<bytes.size(); ++cut )
        {
            CUnrealMeshDescription part;
            if ( !part.ReadUe4Layout( &bytes[ 0 ], cut ) )
                ++refused;
        }
        ASSERT_TRUE( bytes.size() == refused );
        UInt32 state = 11;
        for ( UInt32 round=0; round<3000; ++round )
        {
            GUCEF::vector< UInt8 > bad = bytes;
            for ( UInt32 flips=0; flips<1 + round % 3; ++flips )
            {
                state = state * 1664525U + 1013904223U;
                bad[ ( state >> 8 ) % bad.size() ] = (UInt8) ( state >> 3 );
            }
            CUnrealMeshDescription part;
            if ( part.ReadUe4Layout( &bad[ 0 ], bad.size() ) )
            {
                CUnrealStaticMesh::SLod broken;
                CUnrealStaticMeshReader::BuildLod( part, slots, broken, error );
            }
        }
        // the UE 5 reader refuses the UE 4 layout and the other way round
        CUnrealMeshDescription crossed;
        ASSERT_FALSE( crossed.Read( &bytes[ 0 ], bytes.size() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Legacy payload: a chunked zlib stream inflates to its bytes; damaged headers are refused with a reason before anything is inflated" )
    try
    {
        ASSERT_TRUE( CUnrealLegacyPayloadReader::IsZlibCodecAvailable() );
        GUCEF::vector< UInt8 > raw( 1000 );
        for ( size_t i=0; i<raw.size(); ++i )
            raw[ i ] = (UInt8) ( ( i * 31 ) ^ ( i >> 3 ) );
        GUCEF::vector< UInt8 > chunks;
        ASSERT_TRUE( BuildZlibChunks( raw, 300, chunks ) );
        CORE::CDynamicBuffer out;
        bool codecMissing = true;
        CORE::CString error;
        ASSERT_TRUE( CUnrealLegacyPayloadReader::InflateChunks( &chunks[ 0 ], chunks.size(), raw.size(), 1 << 20, 1 << 20, out, codecMissing, error ) );
        ASSERT_TRUE( !codecMissing && out.GetDataSize() == raw.size() && 0 == memcmp( out.GetConstBufferPtr(), &raw[ 0 ], raw.size() ) );
        // the raw size of the bulk data is not known: the header says it
        ASSERT_TRUE( CUnrealLegacyPayloadReader::InflateChunks( &chunks[ 0 ], chunks.size(), 0, 1 << 20, 1 << 20, out, codecMissing, error ) && out.GetDataSize() == raw.size() );

        struct SDamage { size_t offset; Int64 value; const char* reason; };
        const SDamage damages[] =
        {
            { 0, 0x12345678, "package tag" },                   // the tag
            { 8, 0, "chunk size" },                             // a chunk size of 0
            { 8, 1 << 30, "chunk size" },                       // a chunk size over the budget
            { 24, 999, "differs from the size" },               // the raw total against the bulk data
            { 16, 5, "do not add up" },                         // the compressed total
            { 32, 1 << 25, "not possible" },                    // a chunk larger than the budget of a chunk
            { 40, 301, "not possible" }                         // a chunk with more raw bytes than the chunk size
        };
        for ( UInt32 d=0; d<sizeof( damages ) / sizeof( damages[ 0 ] ); ++d )
        {
            GUCEF::vector< UInt8 > bad = chunks;
            for ( UInt32 b=0; b<8; ++b )
                bad[ damages[ d ].offset + b ] = (UInt8) ( (UInt64) damages[ d ].value >> ( 8 * b ) );
            const bool read = CUnrealLegacyPayloadReader::InflateChunks( &bad[ 0 ], bad.size(), raw.size(), 1 << 20, 1 << 20, out, codecMissing, error );
            if ( read || error.HasSubstr( damages[ d ].reason ) < 0 )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Damage " << d << ": " << error );
            ASSERT_TRUE( !read && error.HasSubstr( damages[ d ].reason ) >= 0 && !codecMissing );
        }
        // a damaged chunk, a stream cut short, a budget too small
        GUCEF::vector< UInt8 > corrupt = chunks;
        corrupt[ 32 + 4 * 16 + 4 ] ^= 0xFF;
        ASSERT_FALSE( CUnrealLegacyPayloadReader::InflateChunks( &corrupt[ 0 ], corrupt.size(), raw.size(), 1 << 20, 1 << 20, out, codecMissing, error ) );
        ASSERT_FALSE( CUnrealLegacyPayloadReader::InflateChunks( &chunks[ 0 ], chunks.size() - 10, raw.size(), 1 << 20, 1 << 20, out, codecMissing, error ) );
        ASSERT_FALSE( CUnrealLegacyPayloadReader::InflateChunks( &chunks[ 0 ], chunks.size(), raw.size(), 999, 1 << 20, out, codecMissing, error ) );
        ASSERT_FALSE( CUnrealLegacyPayloadReader::InflateChunks( &chunks[ 0 ], 31, raw.size(), 1 << 20, 1 << 20, out, codecMissing, error ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UE 4 static mesh (4.25 to 4.27): the mesh description of the source model in legacy bulk data (zlib in chunks, stored, inline) converts; empty and stripped meshes have no geometry; older forms are refused" )
    try
    {
        GUCEF::vector< UInt8 > description;
        BuildUe4QuadDescription( true, description );
        struct SCase { Int32 ue4; Int32 editorVersion; UInt32 flags; };
        const SCase cases[] =
        {
            { 522, 40, CUnrealLegacyBulkData::FLAG_PAYLOAD_AT_END_OF_FILE | CUnrealLegacyBulkData::FLAG_COMPRESSED_ZLIB },
            { 518, 38, CUnrealLegacyBulkData::FLAG_PAYLOAD_AT_END_OF_FILE | CUnrealLegacyBulkData::FLAG_COMPRESSED_ZLIB },
            { 522, 40, CUnrealLegacyBulkData::FLAG_PAYLOAD_AT_END_OF_FILE },
            { 522, 40, CUnrealLegacyBulkData::FLAG_FORCE_INLINE_PAYLOAD | CUnrealLegacyBulkData::FLAG_COMPRESSED_ZLIB }
        };
        for ( UInt32 c=0; c<4; ++c )
        {
            GUCEF::vector< UInt8 > bytes;
            ASSERT_TRUE( BuildUe4StaticMeshPackage( cases[ c ].ue4, cases[ c ].editorVersion, description, cases[ c ].flags, true, false, bytes ) );
            CUnrealPackage package;
            ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "SM_Quad.uasset" ) );
            CUnrealStaticMeshReader reader;
            CUnrealStaticMesh mesh;
            const bool read = reader.Read( package, 0, GUCEF_NULL, GUCEF_NULL, mesh );
            if ( !read )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UE 4 static mesh case " << c << ": " << reader.GetError() );
            ASSERT_TRUE( read && 1 == mesh.lods.size() && 2 == mesh.materialSlots.size() );
            if ( 1 == mesh.lods.size() && 2 == mesh.lods[ 0 ].sections.size() )
                ASSERT_TRUE( 2 == mesh.lods[ 0 ].triangleCount && 0 == mesh.lods[ 0 ].sections[ 0 ].slot && 1 == mesh.lods[ 0 ].sections[ 1 ].slot );
        }

        // through the importer: one mesh in the scene, converted
        GUCEF::vector< UInt8 > bytes;
        ASSERT_TRUE( BuildUe4StaticMeshPackage( 522, 40, description, cases[ 0 ].flags, true, false, bytes ) );
        ASSERT_TRUE( WriteWorkFile( "forms/Content/SM_Quad.uasset", bytes ) );
        CConversionReport report;
        CORE::CString reason;
        const TAssetStatus meshStatus = ImportAsPipeline( "Content/SM_Quad.uasset", report, reason );
        if ( ASSETSTATUS_CONVERTED != meshStatus && ASSETSTATUS_APPROXIMATED != meshStatus )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UE 4 static mesh through the importer: " << ToString( meshStatus ) << ", " << reason );
        ASSERT_TRUE( ASSETSTATUS_CONVERTED == meshStatus || ASSETSTATUS_APPROXIMATED == meshStatus );

        // a source model without data and a mesh whose editor data is stripped: no geometry in the package
        ASSERT_TRUE( BuildUe4StaticMeshPackage( 522, 40, description, cases[ 0 ].flags, false, false, bytes ) );
        {
            CUnrealPackage package;
            ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "SM_Quad.uasset" ) );
            CUnrealStaticMeshReader reader;
            CUnrealStaticMesh mesh;
            ASSERT_TRUE( !reader.Read( package, 0, GUCEF_NULL, GUCEF_NULL, mesh ) && reader.IsWithoutGeometry() && reader.GetError().HasSubstr( "no geometry data" ) >= 0 );
        }
        ASSERT_TRUE( WriteWorkFile( "forms/Content/SM_Empty.uasset", bytes ) );
        CConversionReport emptyReport;
        ASSERT_TRUE( ASSETSTATUS_SKIPPED == ImportAsPipeline( "Content/SM_Empty.uasset", emptyReport, reason ) && reason.HasSubstr( "no geometry data" ) >= 0 );
        ASSERT_TRUE( BuildUe4StaticMeshPackage( 522, 40, description, cases[ 0 ].flags, true, true, bytes ) );
        {
            CUnrealPackage package;
            ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "SM_Quad.uasset" ) );
            CUnrealStaticMeshReader reader;
            CUnrealStaticMesh mesh;
            ASSERT_TRUE( !reader.Read( package, 0, GUCEF_NULL, GUCEF_NULL, mesh ) && reader.IsWithoutGeometry() && reader.GetError().HasSubstr( "stripped" ) >= 0 );
        }

        // the raw mesh form before 4.25: refused with the reason, not misread
        ASSERT_TRUE( BuildUe4StaticMeshPackage( 517, 30, description, cases[ 0 ].flags, true, false, bytes ) );
        {
            CUnrealPackage package;
            ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "SM_Quad.uasset" ) );
            CUnrealStaticMeshReader reader;
            CUnrealStaticMesh mesh;
            ASSERT_TRUE( !reader.Read( package, 0, GUCEF_NULL, GUCEF_NULL, mesh ) && !reader.IsWithoutGeometry() && reader.GetError().HasSubstr( "raw mesh form" ) >= 0 );
        }

        // damaged native data: a reason, never a crash
        ASSERT_TRUE( BuildUe4StaticMeshPackage( 522, 40, description, cases[ 0 ].flags, true, false, bytes ) );
        UInt32 state = 3;
        for ( UInt32 round=0; round<400; ++round )
        {
            GUCEF::vector< UInt8 > bad = bytes;
            state = state * 1664525U + 1013904223U;
            bad[ bad.size() - 1 - ( state >> 8 ) % 600 ] = (UInt8) ( state >> 3 );
            CUnrealPackage package;
            if ( !package.OpenMemory( &bad[ 0 ], (UInt32) bad.size(), "SM_Bad.uasset" ) )
                continue;
            CUnrealStaticMeshReader reader;
            CUnrealStaticMesh mesh;
            if ( !reader.Read( package, 0, GUCEF_NULL, GUCEF_NULL, mesh ) )
                ASSERT_TRUE( !reader.GetError().IsNULLOrEmpty() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UE 4 texture: a PNG source in legacy bulk data compressed with zlib in chunks is kept as the PNG file" )
    try
    {
        GUCEF::vector< UInt8 > pixels( 4 * 9 * 4 );
        for ( size_t i=0; i<pixels.size(); ++i )
            pixels[ i ] = (UInt8) ( i * 13 );
        CORE::CDynamicBuffer png;
        ASSERT_TRUE( CUnrealPngWriter::Write( &pixels[ 0 ], 9, 4, 4, 8, png ) );
        const GUCEF::vector< UInt8 > pngBytes( static_cast< const UInt8* >( png.GetConstBufferPtr() ), static_cast< const UInt8* >( png.GetConstBufferPtr() ) + png.GetDataSize() );
        GUCEF::vector< UInt8 > bytes;
        ASSERT_TRUE( BuildUe4TexturePackage( pngBytes, 9, 4, bytes ) );
        CUnrealPackage package;
        ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Old.uasset" ) );
        CUnrealTextureConverter::SResult result;
        ASSERT_TRUE( CUnrealTextureConverter::Convert( package, 0, GUCEF_NULL, GUCEF_NULL, WorkVfsPath( "forms_textures" ), result ) );
        ASSERT_TRUE( 9 == result.width && 4 == result.height && "TSCF_PNG/TSF_BGRA8" == result.sourceFormat && result.fileSize == pngBytes.size() );
        ASSERT_TRUE( result.fileChecksum == CUnrealPngWriter::Crc32( &pngBytes[ 0 ], (UInt32) pngBytes.size() ) );

        // through the importer the texture package is converted
        ASSERT_TRUE( WriteWorkFile( "forms/Content/T_Old.uasset", bytes ) );
        CConversionReport report;
        CORE::CString reason;
        const TAssetStatus status = ImportAsPipeline( "Content/T_Old.uasset", report, reason );
        ASSERT_TRUE( ASSETSTATUS_CONVERTED == status || ASSETSTATUS_APPROXIMATED == status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UE 5.0 to 5.2 (file versions 1002 to 1008): the editor bulk data record of a mesh description is read as from 1009; earlier versions are refused" )
    try
    {
        GUCEF::vector< UInt8 > description;
        BuildQuadDescription( "Mat_A", description );
        struct SCase { Int32 legacy; Int32 ue5; bool readable; };
        const SCase cases[] = { { -8, 1008, true }, { -8, 1004, true }, { -8, 1002, true }, { -8, 1009, true }, { -8, 1001, false } };
        for ( UInt32 c=0; c<5; ++c )
        {
            GUCEF::vector< UInt8 > bytes;
            BuildUe5StaticMeshPackage( cases[ c ].legacy, cases[ c ].ue5, description, bytes );
            CUnrealPackage package;
            ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "SM_Quad5.uasset" ) );
            CUnrealStaticMeshReader reader;
            CUnrealStaticMesh mesh;
            const bool read = reader.Read( package, 0, GUCEF_NULL, GUCEF_NULL, mesh );
            if ( read != cases[ c ].readable )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UE5 version " << cases[ c ].ue5 << ": " << reader.GetError() );
            ASSERT_TRUE( read == cases[ c ].readable );
            if ( read )
                ASSERT_TRUE( 1 == mesh.lods.size() && 2 == mesh.lods[ 0 ].triangleCount );
            else
                ASSERT_TRUE( reader.GetError().HasSubstr( "before 1002" ) >= 0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Classification: packages that hold nothing to convert are skipped with the reason (materials, unsupported classes, actors of a level), a texture that cannot be read fails" )
    try
    {
        struct SCase { const char* file; const char* className; bool outerImport; TAssetStatus status; const char* reason; };
        const SCase cases[] =
        {
            { "Content/MI_Only.uasset", "MaterialInstanceConstant", false, ASSETSTATUS_SKIPPED, "a material is converted with the meshes that use it" },
            { "Content/BP_Only.uasset", "Blueprint", false, ASSETSTATUS_SKIPPED, "Class Blueprint: the asset kind is not yet supported" },
            { "Content/__ExternalActors__/A/B/XYZ.uasset", "StaticMeshActor", true, ASSETSTATUS_SKIPPED, "holds no asset of its own" }
        };
        for ( UInt32 c=0; c<3; ++c )
        {
            GUCEF::vector< UInt8 > bytes;
            BuildClassPackage( cases[ c ].className, cases[ c ].outerImport, bytes );
            ASSERT_TRUE( WriteWorkFile( CORE::CString( "forms/" ) + cases[ c ].file, bytes ) );
            CConversionReport report;
            CORE::CString reason;
            const TAssetStatus status = ImportAsPipeline( cases[ c ].file, report, reason );
            if ( status != cases[ c ].status || reason.HasSubstr( cases[ c ].reason ) < 0 )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Classification of " << cases[ c ].file << ": status " << ToString( status ) << ", " << reason );
            ASSERT_TRUE( status == cases[ c ].status && reason.HasSubstr( cases[ c ].reason ) >= 0 );
        }

        // the package name of a UE 4 package comes from its place: its summary stores "None"
        ASSERT_TRUE( "/Game/Props/MI_Box" == CUnrealObjectPathResolver::PackageNameOfFile( "None", "Content/Props/MI_Box.uasset" ) );
        ASSERT_TRUE( "/Game/Props/MI_Box" == CUnrealObjectPathResolver::PackageNameOfFile( "", "/mount/Pack/Content/Props/MI_Box.uasset" ) );
        ASSERT_TRUE( "/Effects/Fire/NS_Fire" == CUnrealObjectPathResolver::PackageNameOfFile( "None", "Plugins/Effects/Content/Fire/NS_Fire.uasset" ) );
        ASSERT_TRUE( "/Game/Stored/Name" == CUnrealObjectPathResolver::PackageNameOfFile( "/Game/Stored/Name", "Content/Elsewhere/Other.uasset" ) );
        ASSERT_TRUE( CUnrealObjectPathResolver::PackageNameOfFile( "None", "Loose/Thing.uasset" ).IsNULLOrEmpty() );

        // a texture whose source lies outside the package: an error of the asset, not a skip
        GUCEF::vector< UInt8 > png( 64, 1 );
        GUCEF::vector< UInt8 > bytes;
        ASSERT_TRUE( BuildUe4TexturePackage( png, 2, 2, bytes ) );
        bytes.resize( bytes.size() - 8 );
        ASSERT_TRUE( WriteWorkFile( "forms/Content/T_Cut.uasset", bytes ) );
        CConversionReport report;
        CORE::CString reason;
        ASSERT_TRUE( ASSETSTATUS_FAILED == ImportAsPipeline( "Content/T_Cut.uasset", report, reason ) && reason.HasSubstr( "The texture cannot be read" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    CORE::DeleteDir( WorkDiskPath( "forms" ), true );
    CORE::DeleteDir( WorkDiskPath( "forms_intermediate" ), true );
    CORE::DeleteDir( WorkDiskPath( "forms_textures" ), true );
}

/*-------------------------------------------------------------------------*/
