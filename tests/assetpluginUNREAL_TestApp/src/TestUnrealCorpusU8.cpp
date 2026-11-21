/*
 *  assetpluginUNREAL_TestApp: Tests of textures and static meshes on the local corpus of licensed packages (GUCEF_TEST_ASSET_CORPUS=1)
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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <algorithm>

#include "gucef_test_framework.h"

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H
#include "assetpluginUNREAL_CUnrealTextureConverter.h"

#include "assetpluginUNREAL_CUnrealTextureReader.h"

#include "assetpluginUNREAL_CUnrealCompressedBuffer.h"

#include "assetpluginUNREAL_CUnrealDeltaTextureDecoder.h"

#include "assetpluginUNREAL_CUnrealPngWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H
#include "assetpluginUNREAL_CUnrealStaticMeshReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#include "assetpluginUNREAL_CUnrealImporter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#include "TestUnrealOodle.h"
#include "TestUnrealCorpusU8.h"

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

namespace {

/** Unmounts an archive when the test case is done */
class CMountGuard
{
    public:
    explicit CMountGuard( const CORE::CString& archiveName ) : m_name( archiveName ) {}
    ~CMountGuard() { VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( m_name ); }
    private:
    CORE::CString m_name;
};

/** Points the oodle tool variable at the tool for the tests and puts the old value back */
class CToolScope
{
    public:
    explicit CToolScope( const CORE::CString& path )
    {
        const char* old = getenv( CUnrealCapabilities::OodleToolPathVariable );
        m_had = GUCEF_NULL != old;
        if ( m_had )
            m_old = old;
        setenv( CUnrealCapabilities::OodleToolPathVariable, path.C_String(), 1 );
    }
    ~CToolScope()
    {
        if ( m_had )
            setenv( CUnrealCapabilities::OodleToolPathVariable, m_old.C_String(), 1 );
        else
            unsetenv( CUnrealCapabilities::OodleToolPathVariable );
    }
    private:
    bool m_had;
    CORE::CString m_old;
};

}

/*-------------------------------------------------------------------------*/

static bool
MountProject( const CORE::CString& name )
{
    const CORE::CString zip = CorpusSetting( "projectzip" );
    if ( zip.IsNULLOrEmpty() || !CORE::FileExists( zip ) )
        return false;
    return VFS::CVfsGlobal::Instance()->GetVfs().MountArchive( name, zip, "zip", name, false, false, zip );
}

/*-------------------------------------------------------------------------*/

/** The packages of a mount whose path holds the text, sorted */
static void
PackagesWith( const CORE::CString& mount, const CORE::CString& text, CORE::CString::StringVector& packages )
{
    VFS::CVFS::TStringVector files;
    VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( files, mount, true, true, CORE::CString::Empty, 1000000 );
    for ( size_t i=0; i<files.size(); ++i )
    {
        const CORE::CString lower = files[ i ].Lowercase();
        if ( lower.HasSubstr( ".uasset" ) == (Int32) lower.Length() - 7 && files[ i ].HasSubstr( text ) >= 0 )
            packages.push_back( files[ i ] );
    }
    std::sort( packages.begin(), packages.end() );
}

/*-------------------------------------------------------------------------*/

static Int32
FirstExportOfClass( CUnrealPackage& package, const char* className )
{
    for ( size_t i=0; i<package.GetExports().size(); ++i )
        if ( package.GetExports()[ i ].outerIndex.IsNull() && className == package.GetClassName( package.GetExports()[ i ] ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Float( Float32 value )
{
    char text[ 32 ];
    sprintf( text, "%.3f", value );
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

/** The statistics of the decoded pixels of a TSCF_UEDELTA texture, in the channel order of the source format */
struct SDeltaStatistics
{
    CORE::CString text;         /**< one line for the golden file */
    bool equalColour;           /**< 4 channels of 8 bits: B, G and R equal in every pixel and A opaque */
    UInt64 zeros;               /**< samples of the first channel at 0 */
    UInt64 full;                /**< samples of the first channel at the largest value */
    Float64 unitNormals;        /**< share of the pixels whose RGB, as a normal, has length 1 (within the precision of the format) */
    Float64 seamRatio;          /**< mean step between rows where a strip starts against the mean step elsewhere (8 bit units, plus 1 each): near 1 when decoded right */
};

static bool
DeltaStatistics( CUnrealPackage& package, UInt32 exportIndex, CIUnrealOodleProvider* provider, SDeltaStatistics& out, CORE::CString& error )
{
    CUnrealTextureReader reader;
    CUnrealTextureReader::STextureSource source;
    if ( !reader.Read( package, exportIndex, source ) )
    {
        error = reader.GetError();
        return false;
    }
    CORE::CDynamicBuffer data;
    CUnrealCompressedBuffer buffer( package, provider );
    if ( !buffer.Open( source.payload ) || !buffer.ReadAll( data ) )
    {
        error = buffer.GetError();
        return false;
    }
    UInt8* bytes = static_cast< UInt8* >( data.GetBufferPtr() );
    if ( !CUnrealDeltaTextureDecoder::Decode( source.format, source.width, source.height, bytes, data.GetDataSize(), error ) )
        return false;
    const UInt32 bytesPerPixel = CUnrealDeltaTextureDecoder::BytesPerPixel( source.format );
    const bool words = 8 == bytesPerPixel;
    const UInt32 channels = words ? 4 : bytesPerPixel;
    const UInt64 pixels = (UInt64) source.width * source.height;
    const UInt32 largest = words ? 65535 : 255;
    GUCEF::vector< UInt64 > sums( channels, 0 );
    out.equalColour = 4 == channels && !words;
    out.zeros = 0;
    out.full = 0;
    UInt64 unit = 0;
    UInt32 rows = 0;
    if ( !CUnrealDeltaTextureDecoder::StripRows( source.format, source.width, source.height, rows, error ) )
        return false;
    for ( UInt64 p=0; p<pixels; ++p )
    {
        UInt32 value[ 4 ] = { 0, 0, 0, 0 };
        for ( UInt32 c=0; c<channels; ++c )
        {
            value[ c ] = words ? ( (UInt32) bytes[ p * 8 + c * 2 ] | ( (UInt32) bytes[ p * 8 + c * 2 + 1 ] << 8 ) ) : bytes[ p * bytesPerPixel + c ];
            sums[ c ] += value[ c ];
        }
        if ( 0 == value[ 0 ] )
            ++out.zeros;
        if ( largest == value[ 0 ] )
            ++out.full;
        if ( out.equalColour && ( value[ 0 ] != value[ 1 ] || value[ 1 ] != value[ 2 ] || 255 != value[ 3 ] ) )
            out.equalColour = false;
        if ( 4 == channels )
        {
            // BGRA8 holds the normal in bytes 2, 1, 0; RGBA16 in words 0, 1, 2
            const Float64 x = ( words ? value[ 0 ] : value[ 2 ] ) * 2.0 / largest - 1.0;
            const Float64 y = value[ 1 ] * 2.0 / largest - 1.0;
            const Float64 z = ( words ? value[ 2 ] : value[ 0 ] ) * 2.0 / largest - 1.0;
            if ( fabs( sqrt( x * x + y * y + z * z ) - 1.0 ) < ( words ? 0.01 : 0.04 ) )
                ++unit;
        }
    }
    out.unitNormals = 0 == pixels ? 0.0 : (Float64) unit / (Float64) pixels;
    // a strip decoded with the wrong height shows as a step at the rows where the strips start
    const UInt64 samples = (UInt64) source.width * channels;
    Float64 seamSum = 0.0, otherSum = 0.0;
    UInt64 seamCount = 0, otherCount = 0;
    for ( UInt32 y=1; y<source.height; ++y )
    {
        Float64 step = 0.0;
        for ( UInt64 i=0; i<samples; ++i )
        {
            const UInt64 at = (UInt64) y * samples + i;
            const Int32 a = words ? (Int32) ( (UInt32) bytes[ at * 2 ] | ( (UInt32) bytes[ at * 2 + 1 ] << 8 ) ) : (Int32) bytes[ at ];
            const Int32 b = words ? (Int32) ( (UInt32) bytes[ ( at - samples ) * 2 ] | ( (UInt32) bytes[ ( at - samples ) * 2 + 1 ] << 8 ) ) : (Int32) bytes[ at - samples ];
            step += a > b ? a - b : b - a;
        }
        if ( words )
            step /= 257.0;
        if ( 0 == y % rows )
        {
            seamSum += step;
            seamCount += samples;
        }
        else
        {
            otherSum += step;
            otherCount += samples;
        }
    }
    out.seamRatio = ( ( 0 == seamCount ? 0.0 : seamSum / seamCount ) + 1.0 ) / ( ( 0 == otherCount ? 0.0 : otherSum / otherCount ) + 1.0 );
    out.text = source.name + " " + CORE::UInt32ToString( source.width ) + "x" + CORE::UInt32ToString( source.height ) + " " + source.format + " pixel crc " +
               CORE::UInt32ToString( CUnrealPngWriter::Crc32( bytes, (UInt32) ( pixels * bytesPerPixel ) ) ) + " means";
    for ( UInt32 c=0; c<channels; ++c )
        out.text += " " + Float( (Float32) ( (Float64) sums[ c ] / (Float64) pixels ) );
    out.text += " first channel at 0 " + CORE::UInt64ToString( out.zeros ) + " at max " + CORE::UInt64ToString( out.full );
    if ( source.IsNormalMap() )
        out.text += " unit normals " + Float( (Float32) out.unitNormals );
    out.text += " strip seam ratio " + Float( (Float32) out.seamRatio );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnrealCorpusU8Tests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.CorpusU8" );

    GUCEF_TESTFW_TESTCASE( "Corpus U8c: the textures of the UE 5.3 project (PNG sources, stored and inside Oodle) are read through the tool, the numbers are golden" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString tool = FindOodleToolForTests();
    if ( tool.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "The oodle tool is not available" );
    try
    {
        CMountGuard guard( "u8corpus_tex" );
        if ( !MountProject( "u8corpus_tex" ) )
            GUCEF_TESTFW_SKIP( "The corpus manifest names no project archive" );
        CORE::CString::StringVector packages;
        PackagesWith( "u8corpus_tex", "/T_", packages );
        COodleToolProvider provider( tool );
        const CORE::CString folder = WorkVfsPath( "u8c/textures" );
        UInt32 decoded = 0, unsupported = 0, failed = 0, texturePackages = 0, reduced = 0;
        UInt64 pixelsTotal = 0;
        CORE::CString text;
        GUCEF::map< CORE::CString, UInt32 > forms;
        const UInt64 start = MT::PrecisionTickCount();
        for ( size_t p=0; p<packages.size(); ++p )
        {
            CUnrealPackage package;
            if ( !package.Open( packages[ p ] ) )
                continue;
            const Int32 index = FirstExportOfClass( package, "Texture2D" );
            if ( index < 0 )
                continue;
            ++texturePackages;
            CUnrealTextureConverter::SResult result;
            if ( CUnrealTextureConverter::Convert( package, (UInt32) index, &provider, GUCEF_NULL, folder, result ) )
            {
                ++decoded;
                pixelsTotal += (UInt64) result.width * result.height;
                if ( result.reducedTo8Bit ) ++reduced;
                ++forms[ result.sourceFormat + " " + CORE::UInt32ToString( result.channels ) + "x" + CORE::UInt32ToString( result.bitsPerChannel ) ];
                CORE::CString line = result.textureName + " | " + CORE::UInt32ToString( result.width ) + "x" + CORE::UInt32ToString( result.height ) + " | " + CORE::UInt32ToString( result.channels ) + " ch " + CORE::UInt32ToString( result.bitsPerChannel ) + " bit | " + CORE::UInt64ToString( result.fileSize ) + " bytes | crc " + CORE::UInt32ToString( result.fileChecksum ) + "\n";
                text += line;
            }
            else if ( result.unsupported )
            {
                ++unsupported;
                text += result.textureName + " | unsupported: " + result.error + "\n";
            }
            else
            {
                ++failed;
                text += result.textureName + " | FAILED: " + result.error + "\n";
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Texture " << packages[ p ] << ": " << result.error );
            }
            // the big extracts are not kept
            if ( !result.file.IsNULLOrEmpty() )
                VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( result.file, true );
        }
        const Float64 seconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) / 1000.0;
        CORE::CString summary = "textures " + CORE::UInt32ToString( texturePackages ) + ": decoded " + CORE::UInt32ToString( decoded ) + ", unsupported " + CORE::UInt32ToString( unsupported ) + ", failed " + CORE::UInt32ToString( failed ) + ", reduced to 8 bit " + CORE::UInt32ToString( reduced ) + ", children started " + CORE::UInt32ToString( provider.GetStartCount() ) + "\n";
        for ( GUCEF::map< CORE::CString, UInt32 >::const_iterator f = forms.begin(); f != forms.end(); ++f )
            summary += "  form " + (*f).first + ": " + CORE::UInt32ToString( (*f).second ) + "\n";
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus textures of the project: " << summary << " " << pixelsTotal << " pixels in " << seconds << " s" );
        CORE::CString problem;
        ASSERT_TRUE( texturePackages >= 70 && 0 == failed && 0 == unsupported && 1 == provider.GetStartCount() );
        ASSERT_TRUE( CheckGolden( "u8c_project_textures", summary + text, problem ) );
        if ( !problem.IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Golden: " << problem );
        CORE::DeleteDir( WorkDiskPath( "u8c" ), true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Corpus U8c: the textures of the MetaHuman packages: the characters get their body and garment textures through the importer, every texture package decodes, the delta sources included" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString tool2 = FindOodleToolForTests();
    if ( tool2.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "The oodle tool is not available" );
    try
    {
        const CORE::CString archiveDir = CorpusSetting( "archivedir" );
        if ( archiveDir.IsNULLOrEmpty() || !CORE::DirExists( archiveDir ) )
            GUCEF_TESTFW_SKIP( "The corpus manifest names no archive folder" );
        CToolScope scope( tool2 );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CMountGuard guard( "u8corpus_archives" );
        ASSERT_TRUE( vfs.MountArchive( "u8corpus_archives", archiveDir, VFS::CVFS::FileSystemArchiveTypeName, "u8archives", false, false, archiveDir ) );
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        ASSERT_TRUE( WriteWorkFile( "u8mh/readme.txt", GUCEF::vector< UInt8 >( 3, 'x' ) ) );
        const char* characters[] = { "scarletjohansson", "margaretqualley", "emiliaclarke" };
        CORE::CString summary;
        for ( UInt32 c=0; c<3; ++c )
        {
            CAssetPackManifest manifest;
            manifest.id = "u8mh";
            manifest.source = WorkVfsPath( "u8mh" );
            manifest.importer = "unreal";
            manifest.intermediate = WorkVfsPath( CORE::CString( "u8mh_intermediate_" ) + characters[ c ] );
            CAssetPackContext context;
            ASSERT_TRUE( context.Open( manifest ) );
            CAssetScene scene;
            CConversionReport report;
            const UInt64 start = MT::PrecisionTickCount();
            ASSERT_TRUE( importer->Import( CORE::CString( "u8archives/" ) + characters[ c ] + ".mhpkg", context, scene, report ) );
            const Float64 seconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) / 1000.0;
            UInt32 filled = 0;
            CORE::CString bodyMaterial;
            for ( size_t m=0; m<scene.materials.size(); ++m )
                if ( scene.materials[ m ].name.Lowercase().HasSubstr( "body_shader" ) >= 0 )
                {
                    bodyMaterial += ( bodyMaterial.IsNULLOrEmpty() ? "" : "; " ) + scene.materials[ m ].name + " base " + ( scene.materials[ m ].baseColorTexture.IsUsed() ? scene.textures[ (size_t) scene.materials[ m ].baseColorTexture.textureIndex ].name : CORE::CString( "-" ) ) +
                                   " normal " + ( scene.materials[ m ].normalTexture.IsUsed() ? scene.textures[ (size_t) scene.materials[ m ].normalTexture.textureIndex ].name : CORE::CString( "-" ) );
                    if ( scene.materials[ m ].baseColorTexture.IsUsed() && scene.materials[ m ].normalTexture.IsUsed() )
                        filled = 1;
                }
            UInt32 files = 0;
            for ( size_t t=0; t<scene.textures.size(); ++t )
                if ( !scene.textures[ t ].sources.empty() && vfs.FileExists( scene.textures[ t ].sources[ 0 ] ) )
                    ++files;
            summary += CORE::CString( characters[ c ] ) + ": textures " + CORE::UInt64ToString( scene.textures.size() ) + ", files " + CORE::UInt32ToString( files ) + ", body material " + bodyMaterial + "\n";
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus character " << characters[ c ] << ": " << scene.textures.size() << " textures in " << seconds << " s; " << bodyMaterial );
            ASSERT_TRUE( scene.textures.size() >= 20 && files == scene.textures.size() && 1 == filled );
            // the head of the combined body (UV tile 1001) has its own material with the face textures
            ASSERT_TRUE( bodyMaterial.HasSubstr( "body_shader_shader_UDIM1001 base " ) >= 0 && bodyMaterial.HasSubstr( "_Face_Basecolor" ) >= 0 );
            // the texture packages of the garments are in the archive: they are read and decoded, not reported as missing or unsupported
            UInt32 missingGarmentTextures = 0, unsupportedTextures = 0;
            const CConversionReport::TEntryVector& entries = report.GetEntries();
            for ( size_t e=0; e<entries.size(); ++e )
                for ( size_t i=0; i<entries[ e ].items.size(); ++i )
                {
                    if ( entries[ e ].items[ i ].message.HasSubstr( "not in the pack, those materials have no picture" ) >= 0 && entries[ e ].items[ i ].message.HasSubstr( "T_DenimOutfit" ) >= 0 )
                        ++missingGarmentTextures;
                    if ( entries[ e ].items[ i ].message.HasSubstr( "Unsupported texture source" ) >= 0 )
                        ++unsupportedTextures;
                }
            ASSERT_TRUE( 0 == missingGarmentTextures && 0 == unsupportedTextures );
            // the outfit of each character has a hidden face map (UE 5.8 BodyHiddenFaceMapTexture, UE 5.6 BodyHiddenFaceMap): one worn set
            ASSERT_TRUE( 1 == scene.hiddenFaceSets.size() );
            if ( 1 == scene.hiddenFaceSets.size() )
                ASSERT_TRUE( scene.hiddenFaceSets[ 0 ].active && scene.hiddenFaceSets[ 0 ].hiddenTriangles > 0 && 0 == scene.hiddenFaceSets[ 0 ].name.HasSubstr( "WI_" ) );
        }
        // the extracted images are not kept
        for ( UInt32 c=0; c<3; ++c )
            CORE::DeleteDir( WorkDiskPath( CORE::CString( "u8mh_intermediate_" ) + characters[ c ] ), true );
        CORE::CString problem;
        ASSERT_TRUE( CheckGolden( "u8c_character_textures", summary, problem ) );

        // the textures of the packages themselves are TSCF_UEDELTA in UE 5.6 and 5.8: every one decodes; the checksums of the files are golden
        VFS::CVFS::TStringVector archives;
        vfs.GetFileList( archives, "u8archives", false, true, CORE::CString::Empty, 100000 );
        std::sort( archives.begin(), archives.end() );
        UInt32 deltaSources = 0, decodable = 0, refused = 0, other = 0, texturePackages = 0;
        CORE::CString files;
        for ( size_t a=0; a<archives.size(); ++a )
        {
            if ( archives[ a ].Lowercase().HasSubstr( ".mhpkg" ) != (Int32) archives[ a ].Length() - 6 )
                continue;
            CORE::CString mountName = "u8corpus_mh_" + CORE::UInt64ToString( a );
            CORE::CString type;
            if ( !vfs.TryGetArchiveTypeForFile( archives[ a ], type ) || !vfs.MountArchive( mountName, archives[ a ], type, mountName, false, false ) )
                continue;
            CORE::CString::StringVector packages;
            PackagesWith( mountName, "/Textures/", packages );
            COodleToolProvider provider( tool2 );
            for ( size_t p=0; p<packages.size(); ++p )
            {
                CUnrealPackage package;
                if ( !package.Open( packages[ p ] ) )
                    continue;
                const Int32 index = FirstExportOfClass( package, "Texture2D" );
                if ( index < 0 )
                    continue;
                ++texturePackages;
                CUnrealTextureConverter::SResult result;
                if ( CUnrealTextureConverter::Convert( package, (UInt32) index, &provider, GUCEF_NULL, WorkVfsPath( "u8c/mh" ), result ) )
                {
                    ++decodable;
                    if ( result.sourceFormat.HasSubstr( "TSCF_UEDELTA" ) == 0 )
                        ++deltaSources;
                    files += CORE::ExtractFilename( archives[ a ] ) + " " + result.textureName + " " + result.sourceFormat + " " + CORE::UInt32ToString( result.width ) + "x" + CORE::UInt32ToString( result.height ) +
                             " channels " + CORE::UInt32ToString( result.channels ) + " file crc " + CORE::UInt32ToString( result.fileChecksum ) + "\n";
                    if ( !result.file.IsNULLOrEmpty() )
                        vfs.DeleteFile( result.file, true );
                }
                else if ( result.unsupported )
                {
                    ++refused;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Corpus texture refused: " << packages[ p ] << ": " << result.error );
                }
                else
                {
                    ++other;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Corpus texture failed: " << packages[ p ] << ": " << result.error );
                }
            }
            vfs.UnmountArchiveByName( mountName );
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus MetaHuman texture packages: " << texturePackages << ", decoded " << decodable << " of which delta sources " << deltaSources << ", refused " << refused << ", other failures " << other );
        ASSERT_TRUE( texturePackages >= 100 && deltaSources >= 100 && 0 == other && 0 == refused && decodable == texturePackages );
        ASSERT_TRUE( CheckGolden( "u8c_metahuman_texture_files", files, problem ) );
        if ( !problem.IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Golden: " << problem );
        CORE::DeleteDir( WorkDiskPath( "u8c" ), true );
        CORE::DeleteDir( WorkDiskPath( "u8mh" ), true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Corpus U8c: the hidden face map and the garment textures of emiliaclarke decode to the pictures they hold: a two-level mask, unit normals, golden statistics and checksums" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString toolDelta = FindOodleToolForTests();
    if ( toolDelta.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "The oodle tool is not available" );
    try
    {
        const CORE::CString archiveDir = CorpusSetting( "archivedir" );
        const CORE::CString archive = CORE::CombinePath( archiveDir, "emiliaclarke.mhpkg" );
        if ( archiveDir.IsNULLOrEmpty() || !CORE::FileExists( archive ) )
            GUCEF_TESTFW_SKIP( "The corpus manifest names no archive folder with emiliaclarke.mhpkg" );
        // where the decoded images are copied to for a look, when the corpus manifest names a folder (deltaexportdir=...)
        const CORE::CString exportDir = CorpusSetting( "deltaexportdir" );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CMountGuard folderGuard( "u8delta_archives" );
        ASSERT_TRUE( vfs.MountArchive( "u8delta_archives", archiveDir, VFS::CVFS::FileSystemArchiveTypeName, "u8deltaarchives", false, false, archiveDir ) );
        CORE::CString type;
        ASSERT_TRUE( vfs.TryGetArchiveTypeForFile( "u8deltaarchives/emiliaclarke.mhpkg", type ) );
        CMountGuard guard( "u8delta_emilia" );
        ASSERT_TRUE( vfs.MountArchive( "u8delta_emilia", "u8deltaarchives/emiliaclarke.mhpkg", type, "u8delta_emilia", false, false ) );
        CORE::CString::StringVector packages;
        PackagesWith( "u8delta_emilia", "/Textures/", packages );
        COodleToolProvider provider( toolDelta );
        CORE::CString summary;
        UInt32 garments = 0, masks = 0, normalMaps = 0;
        for ( size_t p=0; p<packages.size(); ++p )
        {
            CUnrealPackage package;
            ASSERT_TRUE( package.Open( packages[ p ] ) );
            const Int32 index = FirstExportOfClass( package, "Texture2D" );
            ASSERT_TRUE( index >= 0 );
            CUnrealTextureConverter::SResult result;
            ASSERT_TRUE( CUnrealTextureConverter::Convert( package, (UInt32) index, &provider, GUCEF_NULL, WorkVfsPath( "u8delta" ), result ) );
            ASSERT_TRUE( result.sourceFormat.HasSubstr( "TSCF_UEDELTA/" ) == 0 );
            SDeltaStatistics statistics;
            CORE::CString error;
            ASSERT_TRUE( DeltaStatistics( package, (UInt32) index, &provider, statistics, error ) );
            summary += statistics.text + " png channels " + CORE::UInt32ToString( result.channels ) + " file crc " + CORE::UInt32ToString( result.fileChecksum ) + "\n";
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Delta texture: " << statistics.text );
            CUnrealTextureReader reader;
            CUnrealTextureReader::STextureSource source;
            ASSERT_TRUE( reader.Read( package, (UInt32) index, source ) );
            // no step where the strips start (decoding with twice the strip height gives 2.3 to 23 on these textures)
            ASSERT_TRUE( statistics.seamRatio < 1.5 );
            if ( result.textureName.HasSubstr( "OpacityMask" ) >= 0 )
            {
                // the hidden face map of the outfit: grey with opaque alpha, written as one channel; black where the body is hidden
                ++masks;
                ASSERT_TRUE( 4096 == result.width && 4096 == result.height && statistics.equalColour && 1 == result.channels && result.greyAsOneChannel );
                ASSERT_TRUE( 13990707 == statistics.zeros && 2482057 == statistics.full );
            }
            else
            {
                ++garments;
                if ( source.IsNormalMap() )
                {
                    // the normal maps of the outfit hold unit vectors; the detail weave map holds shorter ones (median length 0.93) by authoring
                    ++normalMaps;
                    if ( result.textureName.HasSubstr( "Detail" ) < 0 )
                        ASSERT_TRUE( statistics.unitNormals > 0.99 );
                }
            }
            if ( !exportDir.IsNULLOrEmpty() )
            {
                CORE::CreateDirs( exportDir );
                CORE::CopyFile( WorkDiskPath( "u8delta/" + result.textureName + ".png" ), CORE::CombinePath( exportDir, result.textureName + ".png" ), true );
            }
            vfs.DeleteFile( result.file, true );
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Delta textures of emiliaclarke: " << masks << " hidden face map, " << garments << " garment textures, " << normalMaps << " normal maps" );
        ASSERT_TRUE( 1 == masks && garments >= 11 && normalMaps >= 6 );
        CORE::CString problem;
        ASSERT_TRUE( CheckGolden( "u8c_emiliaclarke_delta_textures", summary, problem ) );
        if ( !problem.IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Golden: " << problem );
        CORE::DeleteDir( WorkDiskPath( "u8delta" ), true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Corpus U8c: the hidden face map of the outfit of emiliaclarke hides the body under the garments: the faces stay in the body, the visible body hardly shows through, golden numbers" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString toolHidden = FindOodleToolForTests();
    if ( toolHidden.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "The oodle tool is not available" );
    try
    {
        const CORE::CString archiveDir = CorpusSetting( "archivedir" );
        if ( archiveDir.IsNULLOrEmpty() || !CORE::FileExists( CORE::CombinePath( archiveDir, "emiliaclarke.mhpkg" ) ) )
            GUCEF_TESTFW_SKIP( "The corpus manifest names no archive folder with emiliaclarke.mhpkg" );
        CToolScope scope( toolHidden );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CMountGuard guard( "u8hidden_archives" );
        ASSERT_TRUE( vfs.MountArchive( "u8hidden_archives", archiveDir, VFS::CVFS::FileSystemArchiveTypeName, "u8hiddenarchives", false, false, archiveDir ) );
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        ASSERT_TRUE( WriteWorkFile( "u8hidden/readme.txt", GUCEF::vector< UInt8 >( 3, 'x' ) ) );
        CAssetPackManifest manifest;
        manifest.id = "u8hidden";
        manifest.source = WorkVfsPath( "u8hidden" );
        manifest.importer = "unreal";
        manifest.intermediate = WorkVfsPath( "u8hidden_intermediate" );
        CAssetPackContext context;
        ASSERT_TRUE( context.Open( manifest ) );
        CAssetScene scene;
        CConversionReport report;
        report.SelectAsset( "emiliaclarke.mhpkg" );
        ASSERT_TRUE( importer->Import( "u8hiddenarchives/emiliaclarke.mhpkg", context, scene, report ) );
        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );

        // one set: the denim outfit with its three garments, worn
        ASSERT_TRUE( 1 == scene.hiddenFaceSets.size() );
        const CAssetHiddenFaceSet& set = scene.hiddenFaceSets[ 0 ];
        ASSERT_TRUE( "WI_DenimOutfit" == set.name && set.active && 3 == set.garments.size() && set.hiddenTriangles > 0 );
        // the body keeps every face: the submeshes of the set hold the hidden ones, per level
        UInt32 hiddenSubMeshes = 0, hiddenTriangles = 0, bodyTriangles = 0;
        CORE::CString levels;
        for ( size_t m=0; m<scene.meshes.size(); ++m )
        {
            UInt32 meshHidden = 0;
            for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
                if ( 0 != scene.meshes[ m ].subMeshes[ s ].hiddenBy )
                {
                    ++hiddenSubMeshes;
                    meshHidden += scene.meshes[ m ].subMeshes[ s ].GetTriangleCount();
                }
            if ( meshHidden > 0 )
            {
                bodyTriangles += scene.meshes[ m ].GetTriangleCount();
                levels += " " + scene.meshes[ m ].name + " " + CORE::UInt32ToString( meshHidden ) + " of " + CORE::UInt32ToString( scene.meshes[ m ].GetTriangleCount() );
            }
            hiddenTriangles += meshHidden;
        }
        ASSERT_TRUE( hiddenSubMeshes >= 4 && hiddenTriangles == set.hiddenTriangles );

        // the layer metric on the visible body, and with the outfit taken off
        CLayerPenetrationValidator::SResult worn;
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, worn ) );
        scene.hiddenFaceSets[ 0 ].active = false;
        CLayerPenetrationValidator::SResult bare;
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, bare ) );
        scene.hiddenFaceSets[ 0 ].active = true;
        char depth[ 64 ];
        sprintf( depth, "%.4f / %.4f", worn.deepestPoke, bare.deepestPoke );
        const CORE::CString summary = "set " + set.name + " | garments " + CORE::UInt64ToString( set.garments.size() ) + " | hidden triangles " + CORE::UInt32ToString( set.hiddenTriangles ) + " | body triangles " + CORE::UInt32ToString( bodyTriangles ) + "\n" +
            "levels" + levels + "\n" +
            "outfit taken off: body vertices " + CORE::UInt32ToString( bare.bodyVertices ) + " | covered " + CORE::UInt32ToString( bare.coveredVertices ) + " | showing through " + CORE::UInt32ToString( bare.pokingVertices ) + "\n" +
            "outfit worn: visible body vertices " + CORE::UInt32ToString( worn.bodyVertices ) + " | hidden " + CORE::UInt32ToString( worn.hiddenBodyVertices ) + " | covered " + CORE::UInt32ToString( worn.coveredVertices ) + " | showing through " + CORE::UInt32ToString( worn.pokingVertices ) + "\n" +
            "deepest worn / taken off " + depth + "\n";
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Hidden faces of emiliaclarke: " << summary );
        // the outfit taken off is the body as it was measured before the hidden face maps (thousands of vertices show through); worn,
        // what is left is the edges of the map (the ends of the sleeves, the wrists) and the head tile, which the map does not cover
        ASSERT_TRUE( bare.pokingVertices > 1000 && worn.pokingVertices * 10 < bare.pokingVertices );
        ASSERT_TRUE( worn.hiddenBodyVertices > 0 && bare.bodyVertices > worn.bodyVertices );
        CORE::CString problem;
        ASSERT_TRUE( CheckGolden( "u8c_emiliaclarke_hidden_faces", summary, problem ) );
        if ( !problem.IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Golden: " << problem );
        CORE::DeleteDir( WorkDiskPath( "u8hidden_intermediate" ), true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Corpus U8d: the static meshes of the UE 5.3 project (mesh descriptions inside Oodle): counts, bounds and the winding of every mesh" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString tool3 = FindOodleToolForTests();
    if ( tool3.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "The oodle tool is not available" );
    try
    {
        CMountGuard guard( "u8corpus_mesh" );
        if ( !MountProject( "u8corpus_mesh" ) )
            GUCEF_TESTFW_SKIP( "The corpus manifest names no project archive" );
        CORE::CString::StringVector packages;
        PackagesWith( "u8corpus_mesh", "/SM_", packages );
        COodleToolProvider provider( tool3 );
        UInt32 meshes = 0, failed = 0, noData = 0, lowAgreement = 0, noUv = 0, multiLod = 0, moreUv = 0;
        UInt64 vertices = 0, triangles = 0;
        Float64 agreementSum = 0;
        Float32 agreementMin = 1.0f;
        CORE::CString text;
        const UInt64 start = MT::PrecisionTickCount();
        for ( size_t p=0; p<packages.size(); ++p )
        {
            CUnrealPackage package;
            if ( !package.Open( packages[ p ] ) )
                continue;
            const Int32 index = FirstExportOfClass( package, "StaticMesh" );
            if ( index < 0 )
                continue;
            CUnrealStaticMeshReader reader;
            CUnrealStaticMesh mesh;
            if ( !reader.Read( package, (UInt32) index, &provider, GUCEF_NULL, mesh ) )
            {
                // the presets of the vehicles hold no source data at all
                if ( reader.IsWithoutGeometry() )
                {
                    ++noData;
                    text += "no source data: " + mesh.name + "\n";
                    continue;
                }
                ++failed;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Static mesh " << packages[ p ] << ": " << reader.GetError() );
                continue;
            }
            ++meshes;
            vertices += mesh.GetVertexCount( 0 );
            triangles += mesh.GetTriangleCount( 0 );
            agreementSum += mesh.lods[ 0 ].windingAgreement;
            if ( mesh.lods[ 0 ].windingAgreement < agreementMin ) agreementMin = mesh.lods[ 0 ].windingAgreement;
            if ( mesh.lods[ 0 ].windingAgreement < 0.5f ) ++lowAgreement;
            if ( 0 == mesh.lods[ 0 ].uvSetCount ) ++noUv;
            if ( mesh.lods.size() > 1 ) ++multiLod;
            if ( mesh.lods[ 0 ].sourceUvSetCount > 4 ) ++moreUv;
            const CORE::CString name = mesh.name;
            if ( "SM_Bld_Wall_Glass_Door_Double_01" == name || "SM_Prop_Kitchen_Drinks_Machine_01" == name || "SM_Sign_Large_Hunting_01" == name || "SM_Bld_Base_Floor_01" == name )
            {
                Float32 lo[ 3 ] = { 0, 0, 0 }, hi[ 3 ] = { 0, 0, 0 };
                mesh.GetPositionBounds( 0, lo, hi );
                text += name + " | lods " + CORE::UInt64ToString( mesh.lods.size() ) + " | vertices " + CORE::UInt32ToString( mesh.GetVertexCount( 0 ) ) + " triangles " + CORE::UInt32ToString( mesh.GetTriangleCount( 0 ) ) + " sections " + CORE::UInt64ToString( mesh.lods[ 0 ].sections.size() ) +
                    " | min " + Float( lo[ 0 ] ) + " " + Float( lo[ 1 ] ) + " " + Float( lo[ 2 ] ) + " max " + Float( hi[ 0 ] ) + " " + Float( hi[ 1 ] ) + " " + Float( hi[ 2 ] ) + " | winding " + Float( mesh.lods[ 0 ].windingAgreement ) + "\n";
            }
        }
        const Float64 seconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) / 1000.0;
        CORE::CString summary = "static meshes " + CORE::UInt32ToString( meshes ) + ", without source data " + CORE::UInt32ToString( noData ) + ", failed " + CORE::UInt32ToString( failed ) + ", first LOD vertices " + CORE::UInt64ToString( vertices ) + " triangles " + CORE::UInt64ToString( triangles ) +
            ", more than one LOD " + CORE::UInt32ToString( multiLod ) + ", without UV " + CORE::UInt32ToString( noUv ) + ", over four UV sets " + CORE::UInt32ToString( moreUv ) + ", winding below one half " + CORE::UInt32ToString( lowAgreement ) + ", children started " + CORE::UInt32ToString( provider.GetStartCount() ) + "\n";
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus static meshes of the project: " << summary << " mean winding agreement " << ( meshes > 0 ? agreementSum / meshes : 0.0 ) << ", lowest " << agreementMin << ", " << seconds << " s" );
        CORE::CString problem;
        ASSERT_TRUE( meshes >= 2200 && 0 == failed && meshes + noData >= 2230 && 1 == provider.GetStartCount() );
        ASSERT_TRUE( CheckGolden( "u8d_project_static_meshes", summary + text, problem ) );
        if ( !problem.IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Golden: " << problem );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Corpus U8d: the conformal head and body meshes of the MetaHuman packages (UE 5.6 and 5.8)" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString tool4 = FindOodleToolForTests();
    if ( tool4.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "The oodle tool is not available" );
    try
    {
        const CORE::CString archiveDir = CorpusSetting( "archivedir" );
        if ( archiveDir.IsNULLOrEmpty() || !CORE::DirExists( archiveDir ) )
            GUCEF_TESTFW_SKIP( "The corpus manifest names no archive folder" );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CMountGuard guard( "u8corpus_archives2" );
        ASSERT_TRUE( vfs.MountArchive( "u8corpus_archives2", archiveDir, VFS::CVFS::FileSystemArchiveTypeName, "u8archives2", false, false, archiveDir ) );
        VFS::CVFS::TStringVector archives;
        vfs.GetFileList( archives, "u8archives2", false, true, CORE::CString::Empty, 100000 );
        std::sort( archives.begin(), archives.end() );
        CORE::CString text;
        UInt32 meshes = 0, failed = 0;
        for ( size_t a=0; a<archives.size(); ++a )
        {
            if ( archives[ a ].Lowercase().HasSubstr( ".mhpkg" ) != (Int32) archives[ a ].Length() - 6 )
                continue;
            CORE::CString mountName = "u8corpus_mh2_" + CORE::UInt64ToString( a );
            CORE::CString type;
            if ( !vfs.TryGetArchiveTypeForFile( archives[ a ], type ) || !vfs.MountArchive( mountName, archives[ a ], type, mountName, false, false ) )
                continue;
            CORE::CString::StringVector packages;
            PackagesWith( mountName, "/SM_", packages );
            COodleToolProvider provider( tool4 );
            for ( size_t p=0; p<packages.size(); ++p )
            {
                CUnrealPackage package;
                if ( !package.Open( packages[ p ] ) )
                    continue;
                const Int32 index = FirstExportOfClass( package, "StaticMesh" );
                if ( index < 0 )
                    continue;
                CUnrealStaticMeshReader reader;
                CUnrealStaticMesh mesh;
                if ( !reader.Read( package, (UInt32) index, &provider, GUCEF_NULL, mesh ) )
                {
                    ++failed;
                    text += CORE::CString( "FAILED " ) + packages[ p ] + ": " + reader.GetError() + "\n";
                    continue;
                }
                ++meshes;
                Float32 lo[ 3 ] = { 0, 0, 0 }, hi[ 3 ] = { 0, 0, 0 };
                mesh.GetPositionBounds( 0, lo, hi );
                const CORE::CString archiveName = CORE::ExtractFilename( archives[ a ] );
                text += archiveName + " " + mesh.name + " | lods " + CORE::UInt64ToString( mesh.lods.size() ) + " | vertices " + CORE::UInt32ToString( mesh.GetVertexCount( 0 ) ) + " triangles " + CORE::UInt32ToString( mesh.GetTriangleCount( 0 ) ) + " sections " + CORE::UInt64ToString( mesh.lods[ 0 ].sections.size() ) + " uv " + CORE::UInt32ToString( mesh.lods[ 0 ].uvSetCount ) +
                    " | min " + Float( lo[ 0 ] ) + " " + Float( lo[ 1 ] ) + " " + Float( lo[ 2 ] ) + " max " + Float( hi[ 0 ] ) + " " + Float( hi[ 1 ] ) + " " + Float( hi[ 2 ] ) + " | winding " + Float( mesh.lods[ 0 ].windingAgreement ) + "\n";
            }
            vfs.UnmountArchiveByName( mountName );
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus MetaHuman static meshes: " << meshes << " read, " << failed << " failed\n" << text );
        CORE::CString problem;
        ASSERT_TRUE( meshes >= 3 && 0 == failed );
        ASSERT_TRUE( CheckGolden( "u8d_metahuman_static_meshes", text, problem ) );
        if ( !problem.IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Golden: " << problem );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "Corpus U8d: cross-engine check: the same models of the Shops pack from the Unreal project come out as from the Godot and FBX forms (bounds, facing, winding)" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString tool5 = FindOodleToolForTests();
    if ( tool5.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "The oodle tool is not available" );
    try
    {
        CToolScope scope( tool5 );
        const CORE::CString projectZip = CorpusSetting( "projectzip" );
        if ( projectZip.IsNULLOrEmpty() || !CORE::FileExists( projectZip ) )
            GUCEF_TESTFW_SKIP( "The corpus manifest names no project archive" );
        // the project archive is the source of the pack, as the converter has it: a file in the VFS that the context mounts
        const CORE::CString zipFolder = CORE::StripFilename( projectZip );
        CMountGuard guard( "u8corpus_xfs" );
        ASSERT_TRUE( VFS::CVfsGlobal::Instance()->GetVfs().MountArchive( "u8corpus_xfs", zipFolder, VFS::CVFS::FileSystemArchiveTypeName, "u8proj", false, false, zipFolder ) );
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        ASSERT_TRUE( WriteWorkFile( "u8x/readme.txt", GUCEF::vector< UInt8 >( 3, 'x' ) ) );
        // the bounds of the Godot and the FBX conversion of the same models (metres, Y up, right handed), from the comparison of the routes;
        // the Unity route has the same sizes and turns the models half a circle about the vertical axis: for the drinks machine its z range is -0.53 to 0.37
        struct SExpected { const char* path; Float32 min[ 3 ]; Float32 max[ 3 ]; UInt32 triangles; };
        const SExpected expected[] = {
            { "Content/PolygonShops/Meshes/Props/SM_Prop_Kitchen_Drinks_Machine_01.uasset", { -0.52f, 0.00f, -0.37f }, { 0.52f, 1.15f, 0.53f }, 1284 },
            { "Content/PolygonShops/Meshes/Signs/SM_Sign_Large_Hunting_01.uasset", { -3.87f, -0.40f, -0.12f }, { 3.87f, 0.96f, 0.12f }, 1288 }
        };
        CORE::CString text;
        CAssetPackManifest manifest;
        manifest.id = "u8x";
        manifest.source = CORE::CString( "u8proj/" ) + CORE::ExtractFilename( projectZip );
        manifest.importer = "unreal";
        manifest.intermediate = WorkVfsPath( "u8x_intermediate" );
        manifest.include.push_back( "**" );
        CAssetPackContext context;
        ASSERT_TRUE( context.Open( manifest ) );
        ASSERT_TRUE( importer->IndexPack( context ) );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Cross-engine pack: " << context.GetFiles().size() << " files, " << context.GetIndexSize( CUnrealImporter::PackageIndexName ) << " indexed packages" );
        for ( size_t e=0; e<sizeof( expected ) / sizeof( expected[ 0 ] ); ++e )
        {
            ASSERT_TRUE( context.HasFile( expected[ e ].path ) );
            const CORE::CString vfsPath = context.ToVfsPath( expected[ e ].path );
            CAssetScene scene;
            CConversionReport report;
            ASSERT_TRUE( importer->Import( vfsPath, context, scene, report ) );
            // the materials of the models name textures: they are read through the tool and sit in the slots of the materials
            UInt32 texturedMaterials = 0;
            for ( size_t m=0; m<scene.materials.size(); ++m )
                if ( scene.materials[ m ].baseColorTexture.IsUsed() )
                {
                    ++texturedMaterials;
                    ASSERT_TRUE( (size_t) scene.materials[ m ].baseColorTexture.textureIndex < scene.textures.size() && !scene.textures[ (size_t) scene.materials[ m ].baseColorTexture.textureIndex ].sources.empty() );
                }
            text += CORE::CString( "materials " ) + CORE::UInt64ToString( scene.materials.size() ) + " textured " + CORE::UInt32ToString( texturedMaterials ) + " textures " + CORE::UInt64ToString( scene.textures.size() ) + "\n";
            ASSERT_TRUE( texturedMaterials >= 1 );
            ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
            Float32 lo[ 3 ] = { 1e9f, 1e9f, 1e9f }, hi[ 3 ] = { -1e9f, -1e9f, -1e9f };
            UInt32 triangles = 0, agree = 0;
            for ( size_t n=0; n<scene.nodes.size(); ++n )
            {
                if ( scene.nodes[ n ].mesh < 0 )
                    continue;
                const MATH::TFloat32Matrix4D world = scene.GetWorldMatrix( (UInt32) n );
                const CAssetMesh& mesh = scene.meshes[ (size_t) scene.nodes[ n ].mesh ];
                for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
                {
                    const CAssetSubMesh& sub = mesh.subMeshes[ s ];
                    GUCEF::vector< Float32 > p( sub.positions.size() ), q( sub.normals.size() );
                    for ( size_t v=0; v+2<sub.positions.size(); v+=3 )
                    {
                        for ( UInt32 r=0; r<3; ++r )
                        {
                            p[ v + r ] = world( r, 0 ) * sub.positions[ v ] + world( r, 1 ) * sub.positions[ v + 1 ] + world( r, 2 ) * sub.positions[ v + 2 ] + world( r, 3 );
                            q[ v + r ] = world( r, 0 ) * sub.normals[ v ] + world( r, 1 ) * sub.normals[ v + 1 ] + world( r, 2 ) * sub.normals[ v + 2 ];
                            if ( p[ v + r ] < lo[ r ] ) lo[ r ] = p[ v + r ];
                            if ( p[ v + r ] > hi[ r ] ) hi[ r ] = p[ v + r ];
                        }
                    }
                    for ( size_t t=0; t+2<sub.indices.size(); t+=3 )
                    {
                        const Float32* a = &p[ sub.indices[ t ] * 3 ]; const Float32* b = &p[ sub.indices[ t + 1 ] * 3 ]; const Float32* c = &p[ sub.indices[ t + 2 ] * 3 ];
                        const Float32* na = &q[ sub.indices[ t ] * 3 ];
                        const Float32 e1[ 3 ] = { b[ 0 ] - a[ 0 ], b[ 1 ] - a[ 1 ], b[ 2 ] - a[ 2 ] };
                        const Float32 e2[ 3 ] = { c[ 0 ] - a[ 0 ], c[ 1 ] - a[ 1 ], c[ 2 ] - a[ 2 ] };
                        const Float32 g[ 3 ] = { e1[ 1 ] * e2[ 2 ] - e1[ 2 ] * e2[ 1 ], e1[ 2 ] * e2[ 0 ] - e1[ 0 ] * e2[ 2 ], e1[ 0 ] * e2[ 1 ] - e1[ 1 ] * e2[ 0 ] };
                        if ( g[ 0 ] * na[ 0 ] + g[ 1 ] * na[ 1 ] + g[ 2 ] * na[ 2 ] > 0.0f )
                            ++agree;
                        ++triangles;
                    }
                }
            }
            text += CORE::CString( expected[ e ].path ) + " | metres min " + Float( lo[ 0 ] ) + " " + Float( lo[ 1 ] ) + " " + Float( lo[ 2 ] ) + " max " + Float( hi[ 0 ] ) + " " + Float( hi[ 1 ] ) + " " + Float( hi[ 2 ] ) + " | triangles " + CORE::UInt32ToString( triangles ) + " | outward winding " + Float( triangles > 0 ? (Float32) agree / triangles : 0.0f ) + "\n";
            ASSERT_TRUE( expected[ e ].triangles == triangles );
            for ( UInt32 c=0; c<3; ++c )
                ASSERT_TRUE( fabsf( lo[ c ] - expected[ e ].min[ c ] ) < 0.012f && fabsf( hi[ c ] - expected[ e ].max[ c ] ) < 0.012f );
            ASSERT_TRUE( agree >= triangles * 99 / 100 );
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Cross-engine check of the Shops models:\n" << text );
        CORE::CString problem;
        ASSERT_TRUE( CheckGolden( "u8d_cross_engine", text, problem ) );
        CORE::DeleteDir( WorkDiskPath( "u8x" ), true );
        CORE::DeleteDir( WorkDiskPath( "u8x_intermediate" ), true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "Corpus U8c: the image files of the textures are decoded by gucefIMAGE (the codec plugin of the corpus manifest): sizes and channels as the reader states them" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString tool6 = FindOodleToolForTests();
    const CORE::CString codecPlugin = CorpusSetting( "imageplugin" );
    if ( tool6.IsNULLOrEmpty() || codecPlugin.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "The oodle tool or the image codec plugin of the corpus manifest is not available" );
    try
    {
        if ( !CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginFromDir( codecPlugin, "assetpluginUNREAL_TestApp", true ) )
            GUCEF_TESTFW_SKIP( "The image codec plugin cannot be loaded" );
        CMountGuard guard( "u8corpus_img" );
        if ( !MountProject( "u8corpus_img" ) )
            GUCEF_TESTFW_SKIP( "The corpus manifest names no project archive" );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CORE::CString::StringVector packages;
        PackagesWith( "u8corpus_img", "/T_", packages );
        COodleToolProvider provider( tool6 );
        UInt32 checked = 0, decodedByCodec = 0;
        for ( size_t p=0; p<packages.size() && checked < 25; p+=3 )
        {
            CUnrealPackage package;
            if ( !package.Open( packages[ p ] ) )
                continue;
            const Int32 index = FirstExportOfClass( package, "Texture2D" );
            if ( index < 0 )
                continue;
            CUnrealTextureConverter::SResult result;
            ASSERT_TRUE( CUnrealTextureConverter::Convert( package, (UInt32) index, &provider, GUCEF_NULL, WorkVfsPath( "u8c/img" ), result ) );
            ++checked;
            VFS::TBasicVfsResourcePtr resource = vfs.GetFile( result.file, "rb", false );
            ASSERT_TRUE( !resource.IsNULL() && !resource->GetAccess().IsNULL() );
            IMAGE::CImage image;
            if ( image.Load( *resource->GetAccess(), "png", true, false ) && image.HasFrames() )
            {
                IMAGE::TPixelMapPtr pixels = image.GetPixelMap( 0, 0 );
                ASSERT_TRUE( !pixels.IsNULL() && pixels->GetWidthInPixels() == result.width && pixels->GetHeightInPixels() == result.height && pixels->GetNumberOfChannelsPerPixel() == result.channels );
                ++decodedByCodec;
            }
            vfs.DeleteFile( result.file, true );
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus: " << decodedByCodec << " of " << checked << " extracted textures decoded by gucefIMAGE" );
        ASSERT_TRUE( checked >= 20 && decodedByCodec == checked );
        CORE::DeleteDir( WorkDiskPath( "u8c" ), true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "Corpus U8c: the textures of the UE 4.14 project (legacy bulk data compressed with zlib in chunks) are inflated through the zlib codec, without the tool" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString ue4Zip = CorpusSetting( "ue4zip" );
    if ( ue4Zip.IsNULLOrEmpty() || !CORE::FileExists( ue4Zip ) )
        GUCEF_TESTFW_SKIP( "The corpus manifest names no UE 4.14 project archive" );
    try
    {
        CMountGuard guard( "u8corpus_ue4" );
        ASSERT_TRUE( VFS::CVfsGlobal::Instance()->GetVfs().MountArchive( "u8corpus_ue4", ue4Zip, "zip", "u8corpus_ue4", false, false, ue4Zip ) );
        CORE::CString::StringVector packages;
        PackagesWith( "u8corpus_ue4", "/Textures/T_", packages );
        COodleToolProvider noTool( "/no/such/tool" );
        UInt32 decoded = 0, failed = 0, unsupported = 0, texturePackages = 0;
        CORE::CString text;
        for ( size_t p=0; p<packages.size(); ++p )
        {
            CUnrealPackage package;
            if ( !package.Open( packages[ p ] ) )
                continue;
            const Int32 index = FirstExportOfClass( package, "Texture2D" );
            if ( index < 0 )
                continue;
            ++texturePackages;
            CUnrealTextureConverter::SResult result;
            if ( CUnrealTextureConverter::Convert( package, (UInt32) index, &noTool, GUCEF_NULL, WorkVfsPath( "u8c/ue4" ), result ) )
            {
                ++decoded;
                text += result.textureName + " | " + result.sourceFormat + " | " + CORE::UInt32ToString( result.width ) + "x" + CORE::UInt32ToString( result.height ) + " | " + CORE::UInt32ToString( result.channels ) + " ch | crc " + CORE::UInt32ToString( result.fileChecksum ) + "\n";
                VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( result.file, true );
            }
            else if ( result.unsupported )
            {
                ++unsupported;
                text += result.textureName + " | unsupported: " + result.error + "\n";
            }
            else
            {
                ++failed;
                text += result.textureName + " | FAILED: " + result.error + "\n";
            }
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus UE 4.14 textures: " << texturePackages << " packages, " << decoded << " decoded, " << unsupported << " unsupported, " << failed << " failed\n" << text );
        CORE::CString problem;
        // the sources of this project are zlib compressed legacy bulk data: inflated by the zlib codec, no oodle tool involved
        ASSERT_TRUE( texturePackages >= 1 && 0 == failed && decoded + unsupported == texturePackages && decoded > 0 && 0 == noTool.GetStartCount() );
        ASSERT_TRUE( CheckGolden( "u8c_ue414_textures_zlib", text, problem ) );
        CORE::DeleteDir( WorkDiskPath( "u8c" ), true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
