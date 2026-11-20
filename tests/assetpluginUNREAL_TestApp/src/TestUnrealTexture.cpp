/*
 *  assetpluginUNREAL_TestApp: Tests of the texture sources, the PNG writer and the mesh descriptions of static meshes
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

#include <stdlib.h>
#include <string.h>

#include "gucef_test_framework.h"

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define ASSETCONVERTERLIB_H
#endif /* ASSETCONVERTERLIB_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_COODLETOOLPROVIDER_H
#include "assetpluginUNREAL_COodleToolProvider.h"
#define GUCEF_ASSETPLUGIN_UNREAL_COODLETOOLPROVIDER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_COODLETOOLPROVIDER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#include "assetpluginUNREAL_CUnrealCapabilities.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#include "assetpluginUNREAL_CUnrealPngWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H
#include "assetpluginUNREAL_CUnrealTextureConverter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H
#include "assetpluginUNREAL_CUnrealStaticMeshReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHCONVERTER_H
#include "assetpluginUNREAL_CUnrealStaticMeshConverter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHCONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHCONVERTER_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H
#include "TestUnrealWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H ? */

#include "TestUnrealOodle.h"
#include "TestUnrealTexture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ERRORHERE       GUCEF_TESTFW_ERRORHERE

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CTestType T( const char* name ) { return CTestType( name ); }
static CTestType T( const char* name, const CTestType& a ) { return CTestType( name, a ); }

/*-------------------------------------------------------------------------*/

/** A package with one Texture2D whose source is the given payload; oodle selects a buffer of method 3 (blocks of 1 KiB), else method 0 */
static void
BuildTexturePackage( const char* compressionFormat, const char* format, UInt32 width, UInt32 height, const GUCEF::vector< UInt8 >& source, bool oodle, GUCEF::vector< UInt8 >& bytes )
{
    CTestPackageWriter writer( -9, 522, 1018 );
    writer.packageName = "/Game/Textures/T_Test";
    const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/Engine" );
    writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "Texture2D" );
    UInt32 payload = 0;
    if ( oodle )
    {
        GUCEF::vector< UInt8 > buffer;
        BuildOodleStoredBuffer( source, 10, buffer );
        payload = writer.AddPayload( buffer, false );
    }
    else
        payload = writer.AddPayload( source, true );
    CTestByteWriter body;
    writer.BeginBody( body );
    CTestPropertyWriter p( body, writer.names, writer.ue4, writer.ue5 );
    CTestPropertyWriter::SOpen o = p.Begin( "Source", StructType( "TextureSource" ) );
    {
        CTestPropertyWriter::SOpen a = p.Begin( "SizeX", T( "IntProperty" ) ); body.I32( (Int32) width ); p.End( a );
        a = p.Begin( "SizeY", T( "IntProperty" ) ); body.I32( (Int32) height ); p.End( a );
        a = p.Begin( "NumSlices", T( "IntProperty" ) ); body.I32( 1 ); p.End( a );
        a = p.Begin( "NumMips", T( "IntProperty" ) ); body.I32( 1 ); p.End( a );
        a = p.Begin( "CompressionFormat", T( "ByteProperty", T( "ETextureSourceCompressionFormat" ) ) ); writer.names.WriteName( body, compressionFormat ); p.End( a );
        a = p.Begin( "Format", T( "ByteProperty", T( "ETextureSourceFormat" ) ) ); writer.names.WriteName( body, format ); p.End( a );
        p.None();
    }
    p.End( o );
    o = p.Begin( "CompressionSettings", T( "ByteProperty", T( "TextureCompressionSettings" ) ) ); writer.names.WriteName( body, "TC_Normalmap" ); p.End( o );
    o = p.Begin( "SRGB", T( "BoolProperty" ), false ); p.End( o );
    p.None();
    const Int64 scriptEnd = (Int64) body.Size();
    body.Bool32( false );
    body.U8( 0 ); body.U8( 0 );
    writer.WriteEditorBulkData( body, payload );
    body.Zero( 8 );
    writer.AddExport( -2, 0, "T_Test", body, scriptEnd );
    writer.Build( bytes );
}

/*-------------------------------------------------------------------------*/

/** The pixels of a PNG file that CUnrealPngWriter made: the data of the stored deflate blocks without the filter bytes */
static bool
DecodeStoredPng( const GUCEF::vector< UInt8 >& png, UInt32 width, UInt32 height, UInt32 bytesPerPixel, GUCEF::vector< UInt8 >& pixels )
{
    size_t at = 8;
    GUCEF::vector< UInt8 > zlib;
    while ( at + 12 <= png.size() )
    {
        const UInt32 length = ( (UInt32) png[ at ] << 24 ) | ( (UInt32) png[ at + 1 ] << 16 ) | ( (UInt32) png[ at + 2 ] << 8 ) | png[ at + 3 ];
        if ( 0 == memcmp( &png[ at + 4 ], "IDAT", 4 ) )
            zlib.insert( zlib.end(), png.begin() + at + 8, png.begin() + at + 8 + length );
        at += 12 + length;
    }
    GUCEF::vector< UInt8 > scan;
    size_t z = 2;
    for ( ;; )
    {
        if ( z + 5 > zlib.size() )
            return false;
        const bool last = 0 != ( zlib[ z ] & 1 );
        const UInt32 length = zlib[ z + 1 ] | ( zlib[ z + 2 ] << 8 );
        z += 5;
        if ( z + length > zlib.size() )
            return false;
        scan.insert( scan.end(), zlib.begin() + z, zlib.begin() + z + length );
        z += length;
        if ( last )
            break;
    }
    const size_t rowBytes = (size_t) width * bytesPerPixel;
    if ( scan.size() != ( rowBytes + 1 ) * height )
        return false;
    pixels.clear();
    for ( UInt32 y=0; y<height; ++y )
        pixels.insert( pixels.end(), scan.begin() + y * ( rowBytes + 1 ) + 1, scan.begin() + ( y + 1 ) * ( rowBytes + 1 ) );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ReadVfsFile( const CORE::CString& vfsPath, GUCEF::vector< UInt8 >& bytes )
{
    CORE::CDynamicBuffer buffer;
    if ( !buffer.LoadContentFromFile( vfsPath ) )
    {
        // the work folder is mounted in the VFS: the file is on disk below the work folder
        const CORE::CString prefix = WorkVfsPath( "" );
        if ( 0 != vfsPath.HasSubstr( prefix ) )
            return false;
        if ( !buffer.LoadContentFromFile( WorkDiskPath( vfsPath.SubstrFromRange( prefix.Length(), vfsPath.Length() ) ) ) )
            return false;
    }
    bytes.assign( static_cast< const UInt8* >( buffer.GetConstBufferPtr() ), static_cast< const UInt8* >( buffer.GetConstBufferPtr() ) + buffer.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Builds a mesh description in the layout CUnrealMeshDescription documents */
class CMeshBytes
{
    public:

    CTestByteWriter w;

    void Str( const char* text ) { w.U32( (UInt32) strlen( text ) + 1 ); w.Raw( text, strlen( text ) + 1 ); }

    void Channel( UInt32 slots, UInt32 attributes )
    {
        w.U32( slots );
        for ( UInt32 word=0; word<( slots + 31 ) / 32; ++word )
        {
            const UInt32 left = slots - word * 32;
            w.U32( left >= 32 ? 0xFFFFFFFF : ( ( 1U << left ) - 1 ) );
        }
        w.U32( 0 ); w.U32( slots ); w.U32( attributes );
    }

    /** One attribute with one block of the given values */
    void Attr( const char* name, UInt32 type, UInt32 perId, UInt32 slots, UInt32 bytesPerValue, const GUCEF::vector< UInt8 >& values, UInt32 defaultBytes, UInt32 blocks = 1 )
    {
        Str( name ); w.U32( type ); w.U32( perId ); w.U32( slots ); w.U32( blocks );
        for ( UInt32 b=0; b<blocks; ++b )
        {
            w.U32( perId ); w.U32( bytesPerValue ); w.U32( slots * perId );
            w.Raw( values.empty() ? GUCEF_NULL : &values[ 0 ], values.size() );
        }
        w.Zero( defaultBytes );
        w.U32( 0x21 );
    }
};

static void
AddFloats( GUCEF::vector< UInt8 >& out, const Float32* values, UInt32 count )
{
    const UInt8* p = reinterpret_cast< const UInt8* >( values );
    out.insert( out.end(), p, p + count * 4 );
}

static void
AddInts( GUCEF::vector< UInt8 >& out, const Int32* values, UInt32 count )
{
    const UInt8* p = reinterpret_cast< const UInt8* >( values );
    out.insert( out.end(), p, p + count * 4 );
}

/** A quad of two triangles in the plane z = 0 facing +Z (counter-clockwise seen from +Z), 6 vertex instances, one polygon group named group */
void
BuildQuadDescription( const char* group, GUCEF::vector< UInt8 >& bytes )
{
    CMeshBytes m;
    m.w.U32( 7 );
    // vertices
    m.Str( "Vertices" ); m.w.U32( 1 ); m.Channel( 4, 1 );
    { GUCEF::vector< UInt8 > v; const Float32 p[ 12 ] = { 0,0,0, 100,0,0, 100,100,0, 0,100,0 }; AddFloats( v, p, 12 ); m.Attr( "Position", 1, 1, 4, 12, v, 12 ); }
    // vertex instances: 0 1 2 and 0 2 3
    m.Str( "VertexInstances" ); m.w.U32( 1 ); m.Channel( 6, 4 );
    { GUCEF::vector< UInt8 > v; const Int32 p[ 6 ] = { 0, 1, 2, 0, 2, 3 }; AddInts( v, p, 6 ); m.Attr( "VertexIndex", 4, 1, 6, 4, v, 4 ); }
    { GUCEF::vector< UInt8 > v; const Float32 uv[ 12 ] = { 0,0, 1,0, 1,1, 0,0, 1,1, 0,1 }; AddFloats( v, uv, 12 ); m.Attr( "TextureCoordinate", 2, 1, 6, 8, v, 8 ); }
    { GUCEF::vector< UInt8 > v; for ( UInt32 i=0; i<6; ++i ) { const Float32 n[ 3 ] = { 0, 0, 1 }; AddFloats( v, n, 3 ); } m.Attr( "Normal", 1, 1, 6, 12, v, 12 ); }
    { GUCEF::vector< UInt8 > v; for ( UInt32 i=0; i<6; ++i ) { const Float32 n[ 3 ] = { 1, 0, 0 }; AddFloats( v, n, 3 ); } m.Attr( "Tangent", 1, 1, 6, 12, v, 12 ); }
    // UVs
    m.Str( "UVs" ); m.w.U32( 1 ); m.Channel( 0, 0 );
    // edges
    m.Str( "Edges" ); m.w.U32( 1 ); m.Channel( 0, 0 );
    // triangles
    m.Str( "Triangles" ); m.w.U32( 1 ); m.Channel( 2, 2 );
    { GUCEF::vector< UInt8 > v; const Int32 p[ 6 ] = { 0, 1, 2, 3, 4, 5 }; AddInts( v, p, 6 ); m.Attr( "VertexInstanceIndex", 4, 3, 2, 4, v, 4 ); }
    { GUCEF::vector< UInt8 > v; const Int32 p[ 2 ] = { 0, 0 }; AddInts( v, p, 2 ); m.Attr( "PolygonGroupIndex", 4, 1, 2, 4, v, 4 ); }
    // polygons, polygon groups
    m.Str( "Polygons" ); m.w.U32( 1 ); m.Channel( 0, 0 );
    m.Str( "PolygonGroups" ); m.w.U32( 1 ); m.Channel( 1, 1 );
    {
        m.Str( "ImportedMaterialSlotName" ); m.w.U32( 6 ); m.w.U32( 1 ); m.w.U32( 1 ); m.w.U32( 1 );
        m.w.U32( 1 ); m.w.U32( 1 ); m.Str( group ); m.Str( "None" ); m.w.U32( 0x20 );
    }
    bytes = m.w.bytes;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnrealTextureTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Texture" );

    GUCEF_TESTFW_TESTCASE( "PNG writer: the checksum, the scan lines of raw pixels, the facts of PNG and JPEG headers" )
    try
    {
        ASSERT_TRUE( 0xCBF43926 == CUnrealPngWriter::Crc32( reinterpret_cast< const UInt8* >( "123456789" ), 9 ) );
        const UInt8 pixels[ 3 * 2 * 4 ] = { 1,2,3,4, 5,6,7,8, 9,10,11,12,  13,14,15,16, 17,18,19,20, 21,22,23,24 };
        CORE::CDynamicBuffer png;
        ASSERT_TRUE( CUnrealPngWriter::Write( pixels, 3, 2, 4, 8, png ) );
        UInt32 w = 0, h = 0, c = 0, b = 0;
        ASSERT_TRUE( CUnrealPngWriter::ReadPngInfo( static_cast< const UInt8* >( png.GetConstBufferPtr() ), png.GetDataSize(), w, h, c, b ) && 3 == w && 2 == h && 4 == c && 8 == b );
        GUCEF::vector< UInt8 > bytes( static_cast< const UInt8* >( png.GetConstBufferPtr() ), static_cast< const UInt8* >( png.GetConstBufferPtr() ) + png.GetDataSize() ), back;
        ASSERT_TRUE( DecodeStoredPng( bytes, 3, 2, 4, back ) && 24 == back.size() && 0 == memcmp( &back[ 0 ], pixels, 24 ) );
        // a large image needs several stored blocks
        GUCEF::vector< UInt8 > big( 300 * 300 * 3 );
        for ( size_t i=0; i<big.size(); ++i ) big[ i ] = (UInt8) ( i * 7 );
        ASSERT_TRUE( CUnrealPngWriter::Write( &big[ 0 ], 300, 300, 3, 8, png ) );
        bytes.assign( static_cast< const UInt8* >( png.GetConstBufferPtr() ), static_cast< const UInt8* >( png.GetConstBufferPtr() ) + png.GetDataSize() );
        ASSERT_TRUE( DecodeStoredPng( bytes, 300, 300, 3, back ) && back == big );
        // 16 bit words are written big endian
        const UInt8 words[ 2 ] = { 0x34, 0x12 };
        ASSERT_TRUE( CUnrealPngWriter::Write( words, 1, 1, 1, 16, png ) );
        bytes.assign( static_cast< const UInt8* >( png.GetConstBufferPtr() ), static_cast< const UInt8* >( png.GetConstBufferPtr() ) + png.GetDataSize() );
        ASSERT_TRUE( DecodeStoredPng( bytes, 1, 1, 2, back ) && 0x12 == back[ 0 ] && 0x34 == back[ 1 ] );
        ASSERT_TRUE( !CUnrealPngWriter::Write( pixels, 0, 2, 4, 8, png ) && !CUnrealPngWriter::Write( pixels, 3, 2, 2, 8, png ) && !CUnrealPngWriter::Write( pixels, 3, 2, 4, 12, png ) );
        // a JPEG start of frame
        const UInt8 jpeg[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x04, 0x4A, 0x46, 0xFF, 0xC4, 0x00, 0x03, 0x00, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x40, 0x00, 0x80, 0x03, 0x01, 0x22, 0x00 };
        ASSERT_TRUE( CUnrealPngWriter::ReadJpegInfo( jpeg, sizeof( jpeg ), w, h, c ) && 128 == w && 64 == h && 3 == c );
        ASSERT_TRUE( !CUnrealPngWriter::ReadJpegInfo( jpeg, 12, w, h, c ) && !CUnrealPngWriter::ReadPngInfo( jpeg, sizeof( jpeg ), w, h, c, b ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Texture sources: raw formats and the delta form become PNG files, PNG and JPEG are kept as they are, unknown forms and unestablished layouts are refused with a reason" )
    try
    {
        const CORE::CString folder = WorkVfsPath( "texture/out" );
        struct SCase { const char* compression; const char* format; UInt32 w; UInt32 h; UInt32 bytesPerPixel; UInt32 channels; UInt32 bits; bool reduced; };
        const SCase cases[] = {
            { "TSCF_None", "TSF_BGRA8", 4, 3, 4, 4, 8, false },
            { "TSCF_None", "TSF_G8", 5, 2, 1, 1, 8, false },
            { "TSCF_None", "TSF_RGBA8", 2, 2, 4, 4, 8, false },
            { "TSCF_None", "TSF_RGBA16", 3, 3, 8, 4, 8, true },
            { "TSCF_None", "TSF_G16", 3, 2, 2, 1, 8, true }
        };
        for ( size_t k=0; k<sizeof( cases ) / sizeof( cases[ 0 ] ); ++k )
        {
            const SCase& test = cases[ k ];
            GUCEF::vector< UInt8 > raw( (size_t) test.w * test.h * test.bytesPerPixel );
            for ( size_t i=0; i<raw.size(); ++i ) raw[ i ] = (UInt8) ( 17 + i * 5 );
            for ( UInt32 oodle=0; oodle<2; ++oodle )
            {
                COodleToolProvider provider( FindOodleToolForTests().IsNULLOrEmpty() ? CORE::CString( "/nonexistent/ooz" ) : FindOodleToolForTests() );
                if ( oodle && !provider.IsAvailable() )
                    continue;
                GUCEF::vector< UInt8 > bytes;
                BuildTexturePackage( test.compression, test.format, test.w, test.h, raw, 0 != oodle, bytes );
                CUnrealPackage package;
                ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                CUnrealTextureConverter::SResult result;
                ASSERT_TRUE( CUnrealTextureConverter::Convert( package, 0, &provider, GUCEF_NULL, folder, result ) );
                ASSERT_TRUE( test.w == result.width && test.h == result.height && test.channels == result.channels && test.bits == result.bitsPerChannel && test.reduced == result.reducedTo8Bit );
                GUCEF::vector< UInt8 > file, pixels;
                ASSERT_TRUE( ReadVfsFile( result.file, file ) && file.size() == result.fileSize && CUnrealPngWriter::Crc32( &file[ 0 ], (UInt32) file.size() ) == result.fileChecksum );
                ASSERT_TRUE( DecodeStoredPng( file, test.w, test.h, test.channels, pixels ) );
                const size_t count = (size_t) test.w * test.h;
                for ( size_t i=0; i<count; ++i )
                {
                    if ( 0 == strcmp( test.format, "TSF_BGRA8" ) )
                        ASSERT_TRUE( pixels[ i * 4 ] == raw[ i * 4 + 2 ] && pixels[ i * 4 + 1 ] == raw[ i * 4 + 1 ] && pixels[ i * 4 + 2 ] == raw[ i * 4 ] && pixels[ i * 4 + 3 ] == raw[ i * 4 + 3 ] );
                    if ( 0 == strcmp( test.format, "TSF_G8" ) || 0 == strcmp( test.format, "TSF_RGBA8" ) )
                        ASSERT_TRUE( 0 == memcmp( &pixels[ 0 ], &raw[ 0 ], pixels.size() ) );
                    if ( 0 == strcmp( test.format, "TSF_RGBA16" ) && i < 2 )
                    {
                        const UInt32 first = (UInt32) raw[ i * 8 ] | ( (UInt32) raw[ i * 8 + 1 ] << 8 );
                        ASSERT_TRUE( pixels[ i * 4 ] == (UInt8) ( ( first * 255 + 32767 ) / 65535 ) );
                    }
                }
            }
        }

        // PNG and JPEG sources are kept as the files they are
        {
            CORE::CDynamicBuffer png;
            const UInt8 one[ 8 ] = { 9, 8, 7, 6, 5, 4, 3, 2 };
            ASSERT_TRUE( CUnrealPngWriter::Write( one, 2, 1, 4, 8, png ) );
            GUCEF::vector< UInt8 > pngBytes( static_cast< const UInt8* >( png.GetConstBufferPtr() ), static_cast< const UInt8* >( png.GetConstBufferPtr() ) + png.GetDataSize() );
            GUCEF::vector< UInt8 > bytes;
            BuildTexturePackage( "TSCF_PNG", "TSF_BGRA8", 2, 1, pngBytes, false, bytes );
            CUnrealPackage package;
            ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
            COodleToolProvider noTool( "/nonexistent/ooz" );
            CUnrealTextureConverter::SResult result;
            ASSERT_TRUE( CUnrealTextureConverter::Convert( package, 0, &noTool, GUCEF_NULL, folder, result ) && 0 == noTool.GetStartCount() );
            GUCEF::vector< UInt8 > file;
            ASSERT_TRUE( ReadVfsFile( result.file, file ) && file == pngBytes && 2 == result.width && 1 == result.height && result.file.HasSubstr( ".png" ) > 0 );
            // a PNG source that is not a PNG file
            GUCEF::vector< UInt8 > notPng( 100, 'x' );
            BuildTexturePackage( "TSCF_PNG", "TSF_BGRA8", 2, 1, notPng, false, bytes );
            CUnrealPackage package2;
            ASSERT_TRUE( package2.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
            ASSERT_TRUE( !CUnrealTextureConverter::Convert( package2, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && result.error.HasSubstr( "not a PNG" ) >= 0 );
            const UInt8 jpeg[] = { 0xFF, 0xD8, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x40, 0x00, 0x80, 0x03, 0x01, 0x22, 0x00, 0xFF, 0xD9 };
            GUCEF::vector< UInt8 > jpegBytes( jpeg, jpeg + sizeof( jpeg ) );
            BuildTexturePackage( "TSCF_JPEG", "TSF_BGRA8", 128, 64, jpegBytes, false, bytes );
            CUnrealPackage package3;
            ASSERT_TRUE( package3.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
            ASSERT_TRUE( CUnrealTextureConverter::Convert( package3, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && result.file.HasSubstr( ".jpg" ) > 0 && 128 == result.width && 64 == result.height && 3 == result.channels );
        }

        // forms that are not known, formats that are not handled
        {
            GUCEF::vector< UInt8 > raw( 64, 1 ), bytes;
            CUnrealTextureConverter::SResult result;
            BuildTexturePackage( "TSCF_Strange", "TSF_BGRA8", 2, 4, raw, false, bytes );
            CUnrealPackage package2;
            ASSERT_TRUE( package2.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
            ASSERT_TRUE( !CUnrealTextureConverter::Convert( package2, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && result.unsupported );
            BuildTexturePackage( "TSCF_None", "TSF_RGBA16F", 2, 4, raw, false, bytes );
            CUnrealPackage package3;
            ASSERT_TRUE( package3.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
            ASSERT_TRUE( !CUnrealTextureConverter::Convert( package3, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && result.unsupported && result.error.HasSubstr( "TSF_RGBA16F" ) >= 0 );
            // a raw source that is smaller than the sizes say
            BuildTexturePackage( "TSCF_None", "TSF_BGRA8", 100, 100, raw, false, bytes );
            CUnrealPackage package4;
            ASSERT_TRUE( package4.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
            ASSERT_TRUE( !CUnrealTextureConverter::Convert( package4, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && !result.unsupported && !result.error.IsNULLOrEmpty() );
        }

        // the delta form of the newer engines: the row prediction is reversed and the pixels are written as the raw formats are
        {
            // TSF_BGRA8, 64 x 40: one strip (512 rows at width 64), every row below the first is the difference to the row above
            const UInt32 w = 64, h = 40;
            GUCEF::vector< UInt8 > pixels( (size_t) w * h * 4 ), stored;
            for ( size_t i=0; i<pixels.size(); ++i )
                pixels[ i ] = (UInt8) ( i * 7 + i / 256 );
            stored = pixels;
            for ( size_t i=(size_t) w * 4; i<pixels.size(); ++i )
                stored[ i ] = (UInt8) ( pixels[ i ] - pixels[ i - w * 4 ] );
            for ( UInt32 oodle=0; oodle<2; ++oodle )
            {
                COodleToolProvider provider( FindOodleToolForTests().IsNULLOrEmpty() ? CORE::CString( "/nonexistent/ooz" ) : FindOodleToolForTests() );
                if ( oodle && !provider.IsAvailable() )
                    continue;
                GUCEF::vector< UInt8 > bytes, file, back;
                BuildTexturePackage( "TSCF_UEDELTA", "TSF_BGRA8", w, h, stored, 0 != oodle, bytes );
                CUnrealPackage package;
                ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                CUnrealTextureConverter::SResult result;
                ASSERT_TRUE( CUnrealTextureConverter::Convert( package, 0, &provider, GUCEF_NULL, folder, result ) );
                ASSERT_TRUE( w == result.width && h == result.height && 4 == result.channels && 8 == result.bitsPerChannel && !result.reducedTo8Bit && !result.greyAsOneChannel && "TSCF_UEDELTA/TSF_BGRA8" == result.sourceFormat );
                ASSERT_TRUE( ReadVfsFile( result.file, file ) && DecodeStoredPng( file, w, h, 4, back ) );
                for ( size_t i=0; i<(size_t) w * h; ++i )
                    ASSERT_TRUE( back[ i * 4 ] == pixels[ i * 4 + 2 ] && back[ i * 4 + 1 ] == pixels[ i * 4 + 1 ] && back[ i * 4 + 2 ] == pixels[ i * 4 ] && back[ i * 4 + 3 ] == pixels[ i * 4 + 3 ] );
            }

            // a grey mask with opaque alpha (equal colour channels): one grey channel, losslessly
            {
                GUCEF::vector< UInt8 > mask( (size_t) w * h * 4 ), bytes, file, back;
                for ( size_t p=0; p<(size_t) w * h; ++p )
                {
                    const UInt8 value = (UInt8) ( p < 1000 ? 0 : 255 - p % 3 );
                    const UInt8 above = (UInt8) ( p < w ? 0 : ( p - w < 1000 ? 0 : 255 - ( p - w ) % 3 ) );
                    mask[ p * 4 ] = mask[ p * 4 + 1 ] = mask[ p * 4 + 2 ] = p < w ? value : (UInt8) ( value - above );
                    mask[ p * 4 + 3 ] = p < w ? 255 : 0;
                }
                BuildTexturePackage( "TSCF_UEDELTA", "TSF_BGRA8", w, h, mask, false, bytes );
                CUnrealPackage package;
                ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                CUnrealTextureConverter::SResult result;
                ASSERT_TRUE( CUnrealTextureConverter::Convert( package, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && 1 == result.channels && result.greyAsOneChannel );
                ASSERT_TRUE( ReadVfsFile( result.file, file ) && DecodeStoredPng( file, w, h, 1, back ) );
                ASSERT_TRUE( 0 == back[ 0 ] && 0 == back[ 999 ] && 255 - 1000 % 3 == back[ 1000 ] && 255 - ( w * h - 1 ) % 3 == back[ w * h - 1 ] );
            }

            // TSF_RGBA16: 16 bit words, the difference plus 0x8080; the file has 8 bits per channel and says so
            {
                const UInt8 words[ 2 * 8 * 2 ] = { 0x00, 0x10, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF,   0x00, 0x10, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF,
                                                   0x80, 0x90, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80,   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };
                GUCEF::vector< UInt8 > source( words, words + sizeof( words ) ), bytes, file, back;
                BuildTexturePackage( "TSCF_UEDELTA", "TSF_RGBA16", 2, 2, source, false, bytes );
                CUnrealPackage package;
                ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                CUnrealTextureConverter::SResult result;
                ASSERT_TRUE( CUnrealTextureConverter::Convert( package, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && 4 == result.channels && 8 == result.bitsPerChannel && result.reducedTo8Bit );
                ASSERT_TRUE( ReadVfsFile( result.file, file ) && DecodeStoredPng( file, 2, 2, 4, back ) );
                // row 1, pixel 0: R 0x1000 + 0x1000 = 0x2000, G 0x00FF + 1 = 0x0100, B 0, A 0xFFFF
                ASSERT_TRUE( (UInt8) ( ( 0x2000 * 255 + 32767 ) / 65535 ) == back[ 8 ] && (UInt8) ( ( 0x0100 * 255 + 32767 ) / 65535 ) == back[ 9 ] && 0 == back[ 10 ] && 255 == back[ 11 ] );
            }

            // TSF_G8: one channel as it is
            {
                const UInt8 grey[ 2 * 3 ] = { 10, 200,  5, 0xFB,  0, 0 };
                GUCEF::vector< UInt8 > source( grey, grey + sizeof( grey ) ), bytes, file, back;
                BuildTexturePackage( "TSCF_UEDELTA", "TSF_G8", 2, 3, source, false, bytes );
                CUnrealPackage package;
                ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                CUnrealTextureConverter::SResult result;
                ASSERT_TRUE( CUnrealTextureConverter::Convert( package, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && 1 == result.channels && !result.greyAsOneChannel );
                ASSERT_TRUE( ReadVfsFile( result.file, file ) && DecodeStoredPng( file, 2, 3, 1, back ) );
                ASSERT_TRUE( 10 == back[ 0 ] && 200 == back[ 1 ] && 15 == back[ 2 ] && 195 == back[ 3 ] && 15 == back[ 4 ] && 195 == back[ 5 ] );
            }

            // refused before the payload is read: a format and a size whose layout is not established (no tool is started)
            {
                GUCEF::vector< UInt8 > raw( 64, 1 ), bytes;
                COodleToolProvider noTool( "/nonexistent/ooz" );
                CUnrealTextureConverter::SResult result;
                BuildTexturePackage( "TSCF_UEDELTA", "TSF_RGBA16F", 2, 4, raw, true, bytes );
                CUnrealPackage package;
                ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                ASSERT_TRUE( !CUnrealTextureConverter::Convert( package, 0, &noTool, GUCEF_NULL, folder, result ) && result.unsupported && !result.providerMissing && result.error.HasSubstr( "TSF_RGBA16F" ) >= 0 && result.error.HasSubstr( "not established" ) >= 0 );
                BuildTexturePackage( "TSCF_UEDELTA", "TSF_BGRA8", 300, 200, raw, true, bytes );
                CUnrealPackage package2;
                ASSERT_TRUE( package2.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                ASSERT_TRUE( !CUnrealTextureConverter::Convert( package2, 0, &noTool, GUCEF_NULL, folder, result ) && result.unsupported && result.error.HasSubstr( "not established" ) >= 0 && 0 == noTool.GetStartCount() );
                // sizes that are not valid are an error of the source, not a form this reader lacks
                BuildTexturePackage( "TSCF_UEDELTA", "TSF_BGRA8", 0, 4, raw, false, bytes );
                CUnrealPackage package3;
                ASSERT_TRUE( package3.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                ASSERT_TRUE( !CUnrealTextureConverter::Convert( package3, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && !result.unsupported && !result.error.IsNULLOrEmpty() );
                BuildTexturePackage( "TSCF_UEDELTA", "TSF_BGRA8", 100000, 100000, raw, false, bytes );
                CUnrealPackage package4;
                ASSERT_TRUE( package4.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                ASSERT_TRUE( !CUnrealTextureConverter::Convert( package4, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && !result.unsupported && !result.error.IsNULLOrEmpty() );
                // a payload that is shorter than the sizes say
                BuildTexturePackage( "TSCF_UEDELTA", "TSF_BGRA8", 100, 100, raw, false, bytes );
                CUnrealPackage package5;
                ASSERT_TRUE( package5.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
                ASSERT_TRUE( !CUnrealTextureConverter::Convert( package5, 0, GUCEF_NULL, GUCEF_NULL, folder, result ) && !result.unsupported && result.error.HasSubstr( "bytes where" ) >= 0 );
            }
        }

        // a compressed source without the tool: the asset says what it needs, nothing fails otherwise
        {
            GUCEF::vector< UInt8 > raw( 2 * 2 * 4, 3 ), bytes;
            BuildTexturePackage( "TSCF_None", "TSF_BGRA8", 2, 2, raw, true, bytes );
            CUnrealPackage package;
            ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "T_Test.uasset" ) );
            COodleToolProvider noTool( WorkDiskPath( "texture/no_such_tool" ) );
            CUnrealTextureConverter::SResult result;
            ASSERT_TRUE( !CUnrealTextureConverter::Convert( package, 0, &noTool, GUCEF_NULL, folder, result ) && result.providerMissing && result.error.HasSubstr( "needs the oodle provider: tool not found" ) >= 0 );
            CAssetScene scene;
            CConversionReport report;
            Int32 index = 0;
            ASSERT_TRUE( !CUnrealTextureConverter::AddToScene( package, -1, &noTool, folder, scene, report, index ) && -1 == index && scene.textures.empty() );
            UInt32 skipped = 0;
            for ( size_t i=0; i<report.GetEntries().size(); ++i )
                for ( size_t m=0; m<report.GetEntries()[ i ].items.size(); ++m )
                    if ( REPORTITEM_SKIPPED == report.GetEntries()[ i ].items[ m ].kind && report.GetEntries()[ i ].items[ m ].message.HasSubstr( "needs the oodle provider: tool not found" ) >= 0 )
                        ++skipped;
            ASSERT_TRUE( 1 == skipped );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mesh description: a quad is read, its triangles keep their order, damaged descriptions are refused and never crash" )
    try
    {
        GUCEF::vector< UInt8 > bytes;
        BuildQuadDescription( "Mat_A", bytes );
        CUnrealMeshDescription description;
        ASSERT_TRUE( description.Read( &bytes[ 0 ], bytes.size() ) && 7 == description.GetElements().size() );
        const CUnrealMeshDescription::SElement* vertices = description.FindElement( "Vertices" );
        ASSERT_TRUE( GUCEF_NULL != vertices && 1 == vertices->channels.size() && 4 == vertices->channels[ 0 ].GetAllocatedCount() );
        ASSERT_TRUE( 100.0f == vertices->channels[ 0 ].Find( "Position" )->GetFloat( 0, 2, 0, 1 ) );
        CUnrealSkeletalMesh::TMaterialSlotVector slots( 2 );
        slots[ 0 ].importedSlotName = "Other";
        slots[ 1 ].importedSlotName = "Mat_A";
        CUnrealStaticMesh::SLod lod;
        CORE::CString error;
        ASSERT_TRUE( CUnrealStaticMeshReader::BuildLod( description, slots, lod, error ) );
        ASSERT_TRUE( 1 == lod.sections.size() && 1 == lod.sections[ 0 ].slot && "Mat_A" == lod.sections[ 0 ].groupName && 6 == lod.vertexCount && 2 == lod.triangleCount && 1 == lod.uvSetCount );
        ASSERT_TRUE( 6 == lod.sections[ 0 ].indices.size() && 0 == lod.sections[ 0 ].indices[ 0 ] && 1 == lod.sections[ 0 ].indices[ 1 ] && 2 == lod.sections[ 0 ].indices[ 2 ] );
        ASSERT_TRUE( 1.0f == lod.windingAgreement );
        ASSERT_TRUE( 1.0f == lod.sections[ 0 ].uvSets[ 0 ][ 2 ] && 100.0f == lod.sections[ 0 ].positions[ 3 ] && lod.sections[ 0 ].colours.empty() );
        // a group that no slot names takes the slot of its own number
        BuildQuadDescription( "Nothing", bytes );
        CUnrealMeshDescription second;
        ASSERT_TRUE( second.Read( &bytes[ 0 ], bytes.size() ) && CUnrealStaticMeshReader::BuildLod( second, slots, lod, error ) && 0 == lod.sections[ 0 ].slot );

        // every cut and a flipped byte anywhere: a reason or a result, no crash
        BuildQuadDescription( "Mat_A", bytes );
        UInt32 refused = 0;
        for ( size_t cut=0; cut<bytes.size(); ++cut )
        {
            CUnrealMeshDescription part;
            if ( !part.Read( &bytes[ 0 ], cut ) )
                ++refused;
        }
        ASSERT_TRUE( bytes.size() == refused + 1 || bytes.size() - refused <= 3 );
        UInt32 state = 5;
        for ( UInt32 round=0; round<3000; ++round )
        {
            GUCEF::vector< UInt8 > bad = bytes;
            for ( UInt32 flips=0; flips<1 + round % 3; ++flips )
            {
                state = state * 1664525U + 1013904223U;
                bad[ ( state >> 8 ) % bad.size() ] = (UInt8) ( state >> 3 );
            }
            CUnrealMeshDescription part;
            if ( part.Read( &bad[ 0 ], bad.size() ) )
            {
                CUnrealStaticMesh::SLod broken;
                CUnrealStaticMeshReader::BuildLod( part, slots, broken, error );
            }
        }
        // counts that cannot fit are refused before memory is reserved
        GUCEF::vector< UInt8 > huge = bytes;
        huge[ 4 + 4 + 9 + 4 ] = 0xFF; huge[ 4 + 4 + 9 + 5 ] = 0xFF; huge[ 4 + 4 + 9 + 6 ] = 0xFF; huge[ 4 + 4 + 9 + 7 ] = 0x7F;
        CUnrealMeshDescription absurd;
        ASSERT_TRUE( !absurd.Read( &huge[ 0 ], huge.size() ) && !absurd.GetError().IsNULLOrEmpty() );
        ASSERT_TRUE( !absurd.Read( &bytes[ 0 ], 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
