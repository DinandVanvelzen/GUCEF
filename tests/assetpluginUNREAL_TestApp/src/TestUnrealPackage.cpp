/*
 *  assetpluginUNREAL_TestApp: Tests of the package layer of the Unreal reader
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

#include <string.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#include "assetpluginUNREAL_CUnrealEditorBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#include "assetpluginUNREAL_CUnrealLegacyBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#include "assetpluginUNREAL_CUnrealCompressedBufferHeader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#include "assetpluginUNREAL_CUnrealCustomVersionIds.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#ifndef GUCEF_CORE_TESTUNREALWRITER_H
#include "TestUnrealWriter.h"
#define GUCEF_CORE_TESTUNREALWRITER_H
#endif /* GUCEF_CORE_TESTUNREALWRITER_H ? */

#include "TestUnrealPackage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------*/

/** A package with names, imports, exports, custom versions and a soft path list; the layout follows the versions */
static void
BuildBasicPackage( CTestPackageWriter& writer, GUCEF::vector< UInt8 >& bytes )
{
    writer.customVersions.push_back( CUnrealCustomVersion( CUnrealCustomVersionIds::EditorObjectVersion(), 40 ) );
    writer.customVersions.push_back( CUnrealCustomVersion( CUnrealGuid( 0xDEAD, 0xBEEF, 0xCAFE, 0xF00D ), 7 ) );
    const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/Engine" );
    const Int32 staticMeshClass = writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "StaticMesh" );
    const Int32 materialClass = writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "Material" );

    CTestByteWriter bodyA;
    writer.BeginBody( bodyA );
    CTestPropertyWriter propsA( bodyA, writer.names, writer.ue4, writer.ue5 );
    CTestPropertyWriter::SOpen open = propsA.Begin( "Count", CTestType( "IntProperty" ) );
    bodyA.I32( 77 );
    propsA.End( open );
    propsA.None();
    CTestByteWriter bodyB;
    writer.BeginBody( bodyB );
    CTestPropertyWriter propsB( bodyB, writer.names, writer.ue4, writer.ue5 );
    propsB.None();
    bodyB.Zero( 12 );
    const Int32 meshExport = writer.AddExport( staticMeshClass, 0, "SM_Test", bodyA );
    writer.AddExport( materialClass, meshExport, "Inner_3", bodyB, (Int64) bodyB.Size() - 12 );
    if ( writer.ue5 >= UE5_ADD_SOFTOBJECTPATH_LIST )
    {
        CUnrealSoftObjectPath path;
        path.packageName = "/Game/Other/Package";
        path.assetName = "Asset";
        path.subPath = "Sub";
        writer.softPaths.push_back( path );
    }
    writer.Build( bytes );
}

/*-------------------------------------------------------------------------*/

static bool
CheckBasicPackage( const CUnrealPackage& package, const CTestPackageWriter& writer )
{
    const CUnrealPackageSummary& summary = package.GetSummary();
    bool ok = package.IsOpen();
    ok = ok && summary.versions.GetUe4Version() == writer.ue4 && summary.versions.GetUe5Version() == writer.ue5 && summary.versions.GetLegacyFileVersion() == writer.legacy;
    ok = ok && summary.packageName == writer.packageName && 2 == package.GetExports().size() && 3 == package.GetImports().size();
    ok = ok && package.GetNames().GetCount() == writer.names.names.size();
    ok = ok && 2 == summary.versions.GetCustomVersions().size() && 40 == summary.versions.GetCustomVersion( CUnrealCustomVersionIds::EditorObjectVersion() ) && 7 == summary.versions.GetCustomVersion( CUnrealGuid( 0xDEAD, 0xBEEF, 0xCAFE, 0xF00D ) );
    ok = ok && -1 == summary.versions.GetCustomVersion( CUnrealCustomVersionIds::CoreObjectVersion() );
    ok = ok && "StaticMesh" == package.GetClassName( package.GetExports()[ 0 ] ) && "Material" == package.GetClassName( package.GetExports()[ 1 ] );
    ok = ok && "SM_Test" == package.GetExports()[ 0 ].objectName && "Inner_3" == package.GetExports()[ 1 ].objectName;
    ok = ok && "/Script/Engine.StaticMesh" == package.GetImportPath( 1 );
    ok = ok && package.GetExportPath( 1 ) == writer.packageName + ".SM_Test:Inner_3";
    ok = ok && 4 == summary.savedByEngineVersion.major - ( writer.ue5 > 0 ? 1 : 0 ) && 3 == summary.savedByEngineVersion.minor && 12345 == summary.savedByEngineVersion.changelist;
    ok = ok && 1 == summary.generations.size() && 2 == summary.generations[ 0 ].exportCount;
    ok = ok && package.IsExportRangeValid( 0 ) && package.IsExportRangeValid( 1 ) && package.GetProblems().empty();
    if ( writer.ue5 >= UE5_ADD_SOFTOBJECTPATH_LIST )
        ok = ok && 1 == package.GetSoftObjectPaths().size() && "/Game/Other/Package.Asset:Sub" == package.GetSoftObjectPaths()[ 0 ].ToString();
    if ( writer.ue5 >= UE5_PACKAGE_SAVED_HASH )
        ok = ok && summary.hasSavedHash && 0xA0 == summary.savedHash[ 0 ];
    else
        ok = ok && !summary.hasSavedHash && 0x11 == summary.guid.a;
    if ( writer.ue4 >= UE4_ADDED_PACKAGE_SUMMARY_LOCALIZATION_ID )
        ok = ok && "locid" == summary.localizationId;
    if ( writer.ue4 >= UE4_ADDED_PACKAGE_OWNER )
        ok = ok && 0x55 == summary.persistentGuid.a;
    // the first export holds one property: its range gives a reader at the tagged data
    CUnrealArchiveReader reader;
    ok = ok && const_cast< CUnrealPackage& >( package ).OpenPropertyRange( 0, reader );
    ok = ok && reader.GetRemaining() == (UInt64) writer.exports[ 0 ].body.Size();
    return ok;
}

/*-------------------------------------------------------------------------*/

void
PerformUnrealPackageTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Package" );

    GUCEF_TESTFW_TESTCASE( "Summary: every UE5 file version from 1000 to 1018 is read back with its tables" )
    try
    {
        UInt32 combinations = 0;
        for ( Int32 ue5=1000; ue5<=1018; ++ue5 )
        {
            CTestPackageWriter writer( ue5 < 1016 ? -8 : -9, 522, ue5 );
            GUCEF::vector< UInt8 > bytes;
            BuildBasicPackage( writer, bytes );
            CUnrealPackage package;
            const bool opened = package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "test", CUnrealReaderLimits() );
            ASSERT_TRUE( opened );
            if ( !opened || !CheckBasicPackage( package, writer ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UE5 version " << ue5 << " failed: " << package.GetError() );
                ASSERT_TRUE( false );
            }
            ++combinations;
        }
        ASSERT_TRUE( 19 == combinations );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Summary: every UE4 file version from 508 to 522 (no UE5 version), editor only flag variants" )
    try
    {
        for ( Int32 ue4=508; ue4<=522; ++ue4 )
        {
            for ( UInt32 variant=0; variant<2; ++variant )
            {
                CTestPackageWriter writer( -7, ue4, 0 );
                if ( 1 == variant )
                    writer.packageFlags = UE_PKG_FILTER_EDITOR_ONLY;
                GUCEF::vector< UInt8 > bytes;
                BuildBasicPackage( writer, bytes );
                CUnrealPackage package;
                const bool opened = package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "test", CUnrealReaderLimits() );
                ASSERT_TRUE( opened );
                if ( opened )
                {
                    const CUnrealPackageSummary& summary = package.GetSummary();
                    ASSERT_TRUE( ue4 == summary.versions.GetUe4Version() && 0 == summary.versions.GetUe5Version() );
                    ASSERT_TRUE( 3 == package.GetImports().size() && 2 == package.GetExports().size() && package.GetNames().GetCount() == writer.names.names.size() );
                    ASSERT_TRUE( "SM_Test" == package.GetExports()[ 0 ].objectName && "Inner_3" == package.GetExports()[ 1 ].objectName );
                    ASSERT_TRUE( (UInt64) writer.exports[ 1 ].body.Size() == (UInt64) package.GetExports()[ 1 ].serialSize );
                    // the 4.14 layout has no searchable names; the persistent GUID and the owner GUID have their own boundaries
                    ASSERT_TRUE( ( 0 != ( variant & 1 ) ) == ( 0 != ( summary.packageFlags & UE_PKG_FILTER_EDITOR_ONLY ) ) );
                    if ( 0 == variant && ue4 >= UE4_ADDED_PACKAGE_OWNER )
                        ASSERT_TRUE( 0x55 == summary.persistentGuid.a );
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Summary: packages of the three engine generations are opened through the VFS" )
    try
    {
        struct SCase { Int32 legacy; Int32 ue4; Int32 ue5; const char* file; const char* release; };
        const SCase cases[] = { { -7, 508, 0, "p414.uasset", "4.14 to 4.21" }, { -8, 522, 1009, "p53.uasset", "5.3" }, { -9, 522, 1017, "p56.uasset", "5.6" }, { -9, 522, 1018, "p58.uasset", "5.7 to 5.8" } };
        for ( UInt32 i=0; i<4; ++i )
        {
            CTestPackageWriter writer( cases[ i ].legacy, cases[ i ].ue4, cases[ i ].ue5 );
            GUCEF::vector< UInt8 > bytes;
            BuildBasicPackage( writer, bytes );
            ASSERT_TRUE( WriteWorkFile( cases[ i ].file, bytes ) );
            CUnrealPackage package;
            ASSERT_TRUE( package.Open( WorkVfsPath( cases[ i ].file ) ) );
            ASSERT_TRUE( CheckBasicPackage( package, writer ) );
            ASSERT_TRUE( package.GetSummary().IsEditorPackage() );
            ASSERT_TRUE( CORE::CString( cases[ i ].release ) == CUnrealVersionContext::GuessEngineRelease( cases[ i ].ue4, cases[ i ].ue5 ) );
            ASSERT_TRUE( bytes.size() == package.GetSource().GetSize() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Summary: unknown and unsupported versions are refused with the value found and the limit" )
    try
    {
        struct SRefusal { Int32 legacy; Int32 ue4; Int32 ue5; Int32 licensee; UInt32 tag; Int32 chunks; const char* expected; };
        const SRefusal refusals[] =
        {
            { -9, 522, 1019, 0, UE_PACKAGE_FILE_TAG, 0, "UE5 file version 1019 is newer than the newest supported (1018)" },
            { -10, 522, 1018, 0, UE_PACKAGE_FILE_TAG, 0, "Legacy file version -10 is newer than the newest supported (-9)" },
            { -6, 508, 0, 0, UE_PACKAGE_FILE_TAG, 0, "Legacy file version -6 is older than the oldest supported (-7)" },
            { -7, 507, 0, 0, UE_PACKAGE_FILE_TAG, 0, "UE4 file version 507 is older than the oldest supported (508, engine 4.14)" },
            { -9, 522, 1018, 5, UE_PACKAGE_FILE_TAG, 0, "Licensee version 5 is not supported" },
            { -9, 522, 1018, 0, 0x12345678, 0, "Not an Unreal package: the tag is 0x12345678" },
            { -9, 522, 1018, 0, 0xC1832A9E, 0, "byte swapped" },
            { -9, 522, 1018, 0, UE_PACKAGE_FILE_TAG, 2, "Package compression (2 compressed chunks) is not supported" },
            { -9, 522, 999, 0, UE_PACKAGE_FILE_TAG, 0, "UE5 file version 999 is not a version of the format" },
            { -8, 400, 1009, 0, UE_PACKAGE_FILE_TAG, 0, "UE4 file version 400 is older than the oldest supported (508, engine 4.14)" }
        };
        for ( UInt32 i=0; i<10; ++i )
        {
            CTestPackageWriter writer( refusals[ i ].legacy, refusals[ i ].ue4, refusals[ i ].ue5 );
            writer.licensee = refusals[ i ].licensee;
            writer.tag = refusals[ i ].tag;
            writer.compressedChunks = refusals[ i ].chunks;
            GUCEF::vector< UInt8 > bytes;
            BuildBasicPackage( writer, bytes );
            CUnrealPackage package;
            const bool opened = package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "test", CUnrealReaderLimits() );
            ASSERT_FALSE( opened );
            if ( package.GetError().HasSubstr( refusals[ i ].expected ) < 0 )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Refusal " << i << ": expected \"" << refusals[ i ].expected << "\" found \"" << package.GetError() << "\"" );
            ASSERT_TRUE( package.GetError().HasSubstr( refusals[ i ].expected ) >= 0 );
        }
        // a file that is far too small, and an empty one
        CUnrealPackage tiny;
        const UInt8 few[ 8 ] = { 0xC1, 0x83, 0x2A, 0x9E, 0xF7, 0xFF, 0xFF, 0xFF };
        ASSERT_FALSE( tiny.OpenMemory( few, 8, "tiny", CUnrealReaderLimits() ) );
        ASSERT_TRUE( !tiny.GetError().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Version context: custom versions are named when known, unknown GUIDs are kept" )
    try
    {
        CUnrealVersionContext context;
        context.SetFileVersions( -9, 864, 522, 1018, 0 );
        context.SetCustomVersion( CUnrealCustomVersionIds::Ue5MainStreamObjectVersion(), 123 );
        context.SetCustomVersion( CUnrealCustomVersionIds::Ue5MainStreamObjectVersion(), 124 );
        context.SetCustomVersion( CUnrealGuid( 1, 2, 3, 4 ), 5 );
        ASSERT_TRUE( 2 == context.GetCustomVersions().size() && 124 == context.GetCustomVersion( CUnrealCustomVersionIds::Ue5MainStreamObjectVersion() ) );
        ASSERT_TRUE( context.HasCustomVersion( CUnrealGuid( 1, 2, 3, 4 ) ) && !context.HasCustomVersion( CUnrealGuid( 4, 3, 2, 1 ) ) );
        ASSERT_TRUE( CORE::CString( "Ue5MainStreamObjectVersion" ) == CUnrealCustomVersionIds::GetName( CUnrealCustomVersionIds::Ue5MainStreamObjectVersion() ) );
        ASSERT_TRUE( GUCEF_NULL == CUnrealCustomVersionIds::GetName( CUnrealGuid( 1, 2, 3, 4 ) ) );
        ASSERT_TRUE( CORE::CString( "697DD581-E64F-41AB-AA4A-51ECBEB7B628" ) == CUnrealCustomVersionIds::Ue5MainStreamObjectVersion().ToString() );
        ASSERT_TRUE( context.IsAtLeast( UE5_IMPORT_TYPE_HIERARCHIES ) && context.IsAtLeast( UE4_NON_OUTER_PACKAGE_IMPORT ) );
        context.SetFileVersions( -8, 864, 522, 1009, 0 );
        ASSERT_TRUE( !context.IsAtLeast( UE5_PROPERTY_TAG_COMPLETE_TYPE_NAME ) && context.IsAtLeast( UE5_DATA_RESOURCES ) );
        // all the GUIDs of the survey are named
        const CUnrealGuid ids[] = { CUnrealCustomVersionIds::EditorObjectVersion(), CUnrealCustomVersionIds::RenderingObjectVersion(), CUnrealCustomVersionIds::ReleaseObjectVersion(), CUnrealCustomVersionIds::FrameworkObjectVersion(),
                                    CUnrealCustomVersionIds::Ue5ReleaseStreamObjectVersion(), CUnrealCustomVersionIds::FortniteMainBranchObjectVersion(), CUnrealCustomVersionIds::FortniteReleaseBranchObjectVersion(), CUnrealCustomVersionIds::SkeletalMeshCustomVersion(),
                                    CUnrealCustomVersionIds::AnimationObjectVersion(), CUnrealCustomVersionIds::AnimationPhysicsObjectVersion(), CUnrealCustomVersionIds::CoreObjectVersion(), CUnrealCustomVersionIds::NiagaraObjectVersion(), CUnrealCustomVersionIds::InstancedStructCustomVersion() };
        for ( UInt32 i=0; i<13; ++i )
            ASSERT_TRUE( GUCEF_NULL != CUnrealCustomVersionIds::GetName( ids[ i ] ) );
        ASSERT_TRUE( CORE::CString( "E4B068ED-F494-42E9-A231-DA0B2E46BB41" ) == ids[ 0 ].ToString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Reader: primitives, strings in ANSI and UTF-16, sticky errors, bounded ranges and window refills" )
    try
    {
        CTestByteWriter w;
        w.U8( 200 ); w.U16( 60000 ); w.I32( -5 ); w.U64( 0x1122334455667788ULL ); w.F32( 1.5f ); w.F64( -2.25 ); w.Bool32( true ); w.U32BE( 0x01020304 ); w.U64BE( 0x0102030405060708ULL );
        w.Str( "plain" ); w.Str( "caf\xC3\xA9 \xE2\x82\xAC \xF0\x9F\x98\x80" ); w.Str( "" ); w.Str( "wide", true );
        for ( UInt32 i=0; i<3000; ++i ) w.U32( i );
        w.U32( 2 );
        CUnrealPackageSource source;
        ASSERT_TRUE( source.OpenMemory( &w.bytes[ 0 ], (UInt32) w.bytes.size(), "reader" ) );
        CUnrealArchiveReader reader;
        ASSERT_TRUE( source.OpenRange( 0, w.bytes.size(), reader, 4096 ) );
        ASSERT_TRUE( 200 == reader.ReadUInt8() && 60000 == reader.ReadUInt16() && -5 == reader.ReadInt32() && 0x1122334455667788ULL == reader.ReadUInt64() );
        ASSERT_TRUE( 1.5f == reader.ReadFloat32() && -2.25 == reader.ReadFloat64() && reader.ReadBool32() && 0x01020304 == reader.ReadUInt32BE() && 0x0102030405060708ULL == reader.ReadUInt64BE() );
        CORE::CString text;
        ASSERT_TRUE( reader.ReadString( text ) && "plain" == text );
        ASSERT_TRUE( reader.ReadString( text ) && "caf\xC3\xA9 \xE2\x82\xAC \xF0\x9F\x98\x80" == text );
        ASSERT_TRUE( reader.ReadString( text ) && text.IsNULLOrEmpty() );
        ASSERT_TRUE( reader.ReadString( text ) && "wide" == text );
        // 12000 bytes through a window of 4096 bytes: refilled
        for ( UInt32 i=0; i<3000; ++i )
            ASSERT_TRUE( i == reader.ReadUInt32() );
        ASSERT_TRUE( reader.GetBytesFetched() >= w.bytes.size() - 8 && !reader.HasError() );
        // a range that is narrowed cannot be read past its end; the error is sticky and keeps the first reason
        const UInt64 here = reader.GetPosition();
        reader.Seek( here - 8 );
        const UInt64 previous = reader.NarrowEnd( here - 4 );
        ASSERT_TRUE( 4 == reader.GetRemaining() );
        reader.ReadUInt32();
        ASSERT_TRUE( !reader.HasError() );
        reader.ReadUInt32();
        ASSERT_TRUE( reader.HasError() && reader.GetErrorOffset() == here - 4 );
        const CORE::CString firstReason = reader.GetError();
        reader.ReadUInt32();
        reader.SetError( "second" );
        ASSERT_TRUE( reader.GetError() == firstReason && 0 == reader.ReadUInt32() );
        reader.ClearError();
        reader.RestoreEnd( previous );
        ASSERT_TRUE( reader.Seek( here ) && 2 == reader.ReadUInt32() && !reader.HasError() );
        // a count that cannot fit in the bytes that remain is an error before anything is reserved
        ASSERT_FALSE( reader.CheckCount( 1000000, 4, "things" ) );
        ASSERT_TRUE( reader.HasError() );
        // a string that claims more than the limit or than remains
        CTestByteWriter bad;
        bad.I32( 0x7FFFFFF0 ); bad.U32( 1 );
        CUnrealPackageSource badSource;
        badSource.OpenMemory( &bad.bytes[ 0 ], (UInt32) bad.bytes.size(), "bad" );
        CUnrealArchiveReader badReader;
        badSource.OpenRange( 0, bad.bytes.size(), badReader, 4096 );
        ASSERT_FALSE( badReader.ReadString( text ) );
        ASSERT_TRUE( badReader.HasError() );
        // Int32 minimum as the string length
        CTestByteWriter min;
        min.U32( 0x80000000 );
        CUnrealPackageSource minSource;
        minSource.OpenMemory( &min.bytes[ 0 ], 4, "min" );
        CUnrealArchiveReader minReader;
        minSource.OpenRange( 0, 4, minReader, 4096 );
        ASSERT_FALSE( minReader.ReadString( text ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Soft object paths: the name form of UE4, the top level form of UE5 1007 and the list form of 1008" )
    try
    {
        // UE4: one name with the whole path and a sub path string
        {
            CTestNames names;
            CTestByteWriter w;
            names.WriteName( w, "/Game/Dir/Pkg.Asset" ); w.Str( "Sub" );
            CUnrealNameTable table;
            for ( size_t i=0; i<names.names.size(); ++i ) table.Add( names.names[ i ] );
            CUnrealVersionContext versions;
            versions.SetFileVersions( -7, 864, 522, 0, 0 );
            CUnrealPackageSource source;
            source.OpenMemory( &w.bytes[ 0 ], (UInt32) w.bytes.size(), "s" );
            CUnrealArchiveReader reader;
            source.OpenRange( 0, w.bytes.size(), reader, 4096 );
            CUnrealSoftObjectPath path;
            ASSERT_TRUE( path.Read( reader, table, versions, GUCEF_NULL ) );
            ASSERT_TRUE( "/Game/Dir/Pkg" == path.packageName && "Asset" == path.assetName && "Sub" == path.subPath && "/Game/Dir/Pkg.Asset:Sub" == path.ToString() );
        }
        // UE5 1007: two names; 1008 with a list: an index
        for ( Int32 ue5=1007; ue5<=1008; ++ue5 )
        {
            CTestNames names;
            CTestByteWriter w;
            if ( 1007 == ue5 ) { names.WriteName( w, "/Game/A" ); names.WriteName( w, "B" ); w.Str( "" ); }
            else w.I32( 1 );
            CUnrealNameTable table;
            for ( size_t i=0; i<names.names.size(); ++i ) table.Add( names.names[ i ] );
            CUnrealVersionContext versions;
            versions.SetFileVersions( -8, 864, 522, ue5, 0 );
            CUnrealSoftObjectPath::TSoftObjectPathVector list;
            CUnrealSoftObjectPath first; first.packageName = "/Game/First"; first.assetName = "One";
            CUnrealSoftObjectPath second; second.packageName = "/Game/A"; second.assetName = "B";
            list.push_back( first ); list.push_back( second );
            CUnrealPackageSource source;
            source.OpenMemory( &w.bytes[ 0 ], (UInt32) w.bytes.size(), "s" );
            CUnrealArchiveReader reader;
            source.OpenRange( 0, w.bytes.size(), reader, 4096 );
            CUnrealSoftObjectPath path;
            ASSERT_TRUE( path.Read( reader, table, versions, 1008 == ue5 ? &list : GUCEF_NULL ) );
            ASSERT_TRUE( "/Game/A.B" == path.ToString() );
            // an index outside the list is an error
            CTestByteWriter wrong;
            wrong.I32( 9 );
            CUnrealPackageSource wrongSource;
            wrongSource.OpenMemory( &wrong.bytes[ 0 ], 4, "w" );
            CUnrealArchiveReader wrongReader;
            wrongSource.OpenRange( 0, 4, wrongReader, 4096 );
            if ( 1008 == ue5 )
            {
                CUnrealSoftObjectPath bad;
                ASSERT_FALSE( bad.Read( wrongReader, table, versions, &list ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Trailer: versions 0 to 2, payload lookup by hash, editor bulk data records, stored compressed buffers read by range" )
    try
    {
        for ( Int32 version=0; version<=2; ++version )
        {
            CTestPackageWriter writer( -9, 522, 1018 );
            writer.trailerVersion = version;
            GUCEF::vector< UInt8 > pngLike( 1000 );
            for ( size_t i=0; i<pngLike.size(); ++i ) pngLike[ i ] = (UInt8) ( i * 7 );
            GUCEF::vector< UInt8 > big( 600000 );
            for ( size_t i=0; i<big.size(); ++i ) big[ i ] = (UInt8) ( i * 13 + 1 );
            const UInt32 first = writer.AddPayload( pngLike );
            const UInt32 second = writer.AddPayload( big );
            const UInt32 third = writer.AddPayload( GUCEF::vector< UInt8 >(), true );
            CTestByteWriter body;
            writer.BeginBody( body );
            writer.WriteEditorBulkData( body, second );
            writer.WriteEditorBulkData( body, first );
            writer.WriteEditorBulkData( body, third );
            writer.AddExport( 0, 0, "Texture", body );
            GUCEF::vector< UInt8 > bytes;
            writer.Build( bytes );
            CUnrealPackage package;
            ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "trailer", CUnrealReaderLimits() ) );
            ASSERT_TRUE( package.HasTrailer() && package.GetProblems().empty() );
            const CUnrealPackageTrailer& trailer = package.GetTrailer();
            ASSERT_TRUE( (UInt32) version == trailer.GetVersion() && 3 == trailer.GetEntries().size() );
            ASSERT_TRUE( trailer.GetTrailerOffset() == (UInt64) package.GetSummary().payloadTocOffset );

            CUnrealArchiveReader records;
            ASSERT_TRUE( package.OpenExportRange( 0, records ) );
            records.Skip( 1 ); // the control byte of the class serialisation
            CUnrealEditorBulkData recordA, recordB, recordC;
            ASSERT_TRUE( recordA.Read( records, package.GetVersions() ) && recordB.Read( records, package.GetVersions() ) && recordC.Read( records, package.GetVersions() ) );
            ASSERT_TRUE( 0x200 == recordA.flags && 600000 == recordA.payloadSize && 1000 == recordB.payloadSize );
            CUnrealPayloadReference referenceA, referenceB, referenceC;
            recordA.ToReference( &trailer, referenceA );
            recordB.ToReference( &trailer, referenceB );
            recordC.ToReference( &trailer, referenceC );
            ASSERT_TRUE( CUnrealPayloadReference::KIND_TRAILER == referenceA.kind && CUnrealPayloadReference::KIND_TRAILER == referenceB.kind && CUnrealPayloadReference::KIND_EMPTY == referenceC.kind );
            ASSERT_TRUE( 600000 == referenceA.rawSize && referenceA.storedSize == 600000 + 64 );
            ASSERT_TRUE( referenceA.IsReadable() && !referenceC.IsReadable() );

            // the payload is a compressed buffer of method 0: the header, then the bytes
            CUnrealArchiveReader payload;
            ASSERT_TRUE( package.OpenPayload( referenceA, payload ) );
            CUnrealCompressedBufferHeader header;
            ASSERT_TRUE( header.Read( payload, referenceA.storedSize ) );
            ASSERT_TRUE( header.IsStored() && 600000 == header.rawSize && 1 == header.blockCount && 600000 == header.blockSizes[ 0 ] && 64 == header.GetHeaderBytes() );
            ASSERT_TRUE( referenceA.storedSize == header.GetHeaderBytes() + header.rawSize );
            GUCEF::vector< UInt8 > read( 600000 );
            ASSERT_TRUE( payload.Read( &read[ 0 ], read.size() ) && read == big );
            CUnrealArchiveReader small;
            ASSERT_TRUE( package.OpenPayload( referenceB, small ) );
            CUnrealCompressedBufferHeader smallHeader;
            ASSERT_TRUE( smallHeader.Read( small, referenceB.storedSize ) && 1 == smallHeader.blockCount && 1000 == smallHeader.rawSize );
            // a compressed header (Oodle, three blocks of 256 KiB) with its table of block sizes
            CTestByteWriter compressed;
            compressed.U32BE( 0xB7756362 ); compressed.U32BE( 0 ); compressed.U8( 3 ); compressed.U8( 3 ); compressed.U8( 3 ); compressed.U8( 18 ); compressed.U32BE( 3 ); compressed.U64BE( 600000 ); compressed.U64BE( 64 + 12 + 300 ); compressed.Zero( 32 );
            compressed.U32BE( 100 ); compressed.U32BE( 100 ); compressed.U32BE( 100 ); compressed.Zero( 300 );
            CUnrealPackageSource compressedSource;
            compressedSource.OpenMemory( &compressed.bytes[ 0 ], (UInt32) compressed.bytes.size(), "c" );
            CUnrealArchiveReader compressedReader;
            compressedSource.OpenRange( 0, compressed.bytes.size(), compressedReader, 4096 );
            CUnrealCompressedBufferHeader oodle;
            ASSERT_TRUE( oodle.Read( compressedReader, compressed.bytes.size() ) && !oodle.IsStored() && 3 == oodle.blockCount && 18 == oodle.blockSizeLog2 && 100 == oodle.blockSizes[ 2 ] && 76 == oodle.GetHeaderBytes() - 0 && CUnrealCompressedBufferHeader::METHOD_OODLE == oodle.method );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Trailer: a record that refers to nothing, a payload outside the table and virtualized payloads are unavailable with a reason" )
    try
    {
        CTestPackageWriter writer( -9, 522, 1018 );
        GUCEF::vector< UInt8 > data( 100, 7 );
        writer.AddPayload( data );
        CTestByteWriter body;
        writer.BeginBody( body );
        writer.AddExport( 0, 0, "Thing", body );
        GUCEF::vector< UInt8 > bytes;
        writer.Build( bytes );
        CUnrealPackage package;
        ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "t", CUnrealReaderLimits() ) );
        CUnrealEditorBulkData record;
        record.flags = 0x200;
        record.payloadSize = 50;
        memset( record.payloadId, 0x42, 20 );
        CUnrealPayloadReference reference;
        record.ToReference( &package.GetTrailer(), reference );
        ASSERT_TRUE( CUnrealPayloadReference::KIND_UNAVAILABLE == reference.kind && reference.reason.HasSubstr( "not in the table" ) >= 0 );
        record.flags = 0x1;
        record.ToReference( &package.GetTrailer(), reference );
        ASSERT_TRUE( CUnrealPayloadReference::KIND_UNAVAILABLE == reference.kind && reference.reason.HasSubstr( "virtualized" ) >= 0 );
        record.flags = 0x200;
        record.ToReference( GUCEF_NULL, reference );
        ASSERT_TRUE( CUnrealPayloadReference::KIND_UNAVAILABLE == reference.kind && reference.reason.HasSubstr( "has none" ) >= 0 );
        // a package of an older UE5 version: the record layout is not known
        CTestByteWriter rec;
        rec.Zero( 48 );
        CUnrealPackageSource source;
        source.OpenMemory( &rec.bytes[ 0 ], 48, "r" );
        CUnrealArchiveReader reader;
        source.OpenRange( 0, 48, reader, 4096 );
        CUnrealVersionContext old;
        old.SetFileVersions( -8, 864, 522, 1005, 0 );
        CUnrealEditorBulkData oldRecord;
        ASSERT_FALSE( oldRecord.Read( reader, old ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Trailer: damaged footers, lengths and entries are reported as problems and the package stays usable" )
    try
    {
        CTestPackageWriter writer( -9, 522, 1018 );
        writer.AddPayload( GUCEF::vector< UInt8 >( 300, 3 ) );
        CTestByteWriter body;
        writer.BeginBody( body );
        CTestPropertyWriter props( body, writer.names, 522, 1018 );
        props.None();
        writer.AddExport( 0, 0, "Thing", body );
        GUCEF::vector< UInt8 > good;
        writer.Build( good );
        for ( UInt32 damage=0; damage<4; ++damage )
        {
            GUCEF::vector< UInt8 > bytes = good;
            if ( 0 == damage ) bytes[ bytes.size() - 1 ] ^= 0xFF;                          // the package tag of the footer
            if ( 1 == damage ) bytes[ bytes.size() - 12 ] ^= 0x01;                         // the length of the trailer
            if ( 2 == damage ) bytes.resize( bytes.size() - 40 );                          // truncated: the footer is gone
            if ( 3 == damage ) { const size_t at = bytes.size() - 20 - 300 - 64 - 4 - 49 + 20; bytes[ at + 7 ] = 0x7F; } // an offset in the table
            CUnrealPackage package;
            ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "d", CUnrealReaderLimits() ) );
            ASSERT_TRUE( !package.HasTrailer() && !package.GetProblems().empty() );
            ASSERT_TRUE( package.GetProblems()[ 0 ].HasSubstr( "trailer" ) >= 0 || package.GetProblems()[ 0 ].HasSubstr( "Trailer" ) >= 0 );
            ASSERT_TRUE( 1 == package.GetExports().size() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Legacy bulk data: inline payload, payload at the bulk data offset, separate file, unused and 64 bit sizes" )
    try
    {
        CUnrealVersionContext versions;
        versions.SetFileVersions( -7, 864, 508, 0, 0 );
        struct SCase { UInt32 flags; bool wide; Int64 elements; Int64 size; Int64 offset; bool badVersion; };
        const SCase cases[] =
        {
            { CUnrealLegacyBulkData::FLAG_FORCE_INLINE_PAYLOAD, false, 8, 8, 0, false },
            { CUnrealLegacyBulkData::FLAG_PAYLOAD_AT_END_OF_FILE, false, 16, 16, 100, false },
            { CUnrealLegacyBulkData::FLAG_PAYLOAD_AT_END_OF_FILE | CUnrealLegacyBulkData::FLAG_NO_OFFSET_FIXUP, false, 16, 16, 300, false },
            { CUnrealLegacyBulkData::FLAG_PAYLOAD_IN_SEPERATE_FILE, false, 16, 16, 0, false },
            { CUnrealLegacyBulkData::FLAG_UNUSED, false, 0, 0, 0, false },
            { CUnrealLegacyBulkData::FLAG_PAYLOAD_AT_END_OF_FILE | CUnrealLegacyBulkData::FLAG_SIZE_64BIT, true, 40, 40, 7, false },
            { CUnrealLegacyBulkData::FLAG_PAYLOAD_AT_END_OF_FILE | CUnrealLegacyBulkData::FLAG_BAD_DATA_VERSION, false, 4, 4, 0, true },
            { CUnrealLegacyBulkData::FLAG_PAYLOAD_AT_END_OF_FILE, false, 16, 16, 4000, false }
        };
        for ( UInt32 i=0; i<8; ++i )
        {
            CTestByteWriter w;
            w.U32( cases[ i ].flags );
            if ( cases[ i ].wide ) { w.I64( cases[ i ].elements ); w.I64( cases[ i ].size ); } else { w.I32( (Int32) cases[ i ].elements ); w.I32( (Int32) cases[ i ].size ); }
            w.I64( cases[ i ].offset );
            if ( cases[ i ].badVersion ) w.U16( 1 );
            const UInt64 headerEnd = 1000 + w.Size();
            w.U32( 0xABABABAB );
            CUnrealPackageSource source;
            source.OpenMemory( &w.bytes[ 0 ], (UInt32) w.bytes.size(), "b" );
            CUnrealArchiveReader reader;
            source.OpenRange( 0, w.bytes.size(), reader, 4096 );
            CUnrealLegacyBulkData data;
            ASSERT_TRUE( data.Read( reader, versions ) );
            ASSERT_TRUE( reader.GetPosition() == w.Size() - 4 );
            CUnrealPayloadReference reference;
            data.ToReference( 1000, headerEnd, 2000, reference );
            if ( 0 == i ) ASSERT_TRUE( CUnrealPayloadReference::KIND_INLINE == reference.kind && headerEnd == reference.offset && 8 == reference.storedSize );
            if ( 1 == i ) ASSERT_TRUE( CUnrealPayloadReference::KIND_INLINE == reference.kind && 1100 == reference.offset );
            if ( 2 == i ) ASSERT_TRUE( CUnrealPayloadReference::KIND_INLINE == reference.kind && 300 == reference.offset );
            if ( 3 == i ) ASSERT_TRUE( CUnrealPayloadReference::KIND_UNAVAILABLE == reference.kind && reference.reason.HasSubstr( "separate file" ) >= 0 );
            if ( 4 == i ) ASSERT_TRUE( CUnrealPayloadReference::KIND_EMPTY == reference.kind );
            if ( 5 == i ) ASSERT_TRUE( CUnrealPayloadReference::KIND_INLINE == reference.kind && 1007 == reference.offset && 40 == reference.rawSize );
            if ( 6 == i ) ASSERT_TRUE( CUnrealPayloadReference::KIND_INLINE == reference.kind );
            if ( 7 == i ) ASSERT_TRUE( CUnrealPayloadReference::KIND_UNAVAILABLE == reference.kind && reference.reason.HasSubstr( "outside" ) >= 0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Exports: a range outside the package is a problem of that export, the others stay usable; outer chains are bounded" )
    try
    {
        CTestPackageWriter writer( -9, 522, 1018 );
        CTestByteWriter body;
        writer.BeginBody( body );
        CTestPropertyWriter props( body, writer.names, 522, 1018 );
        props.None();
        writer.AddExport( 0, 0, "Fine", body );
        writer.AddExport( 0, 0, "Broken", body );
        writer.AddExport( 0, 0, "Backwards", body );
        writer.AddExport( 0, 0, "CycleA", body );
        writer.AddExport( 0, 0, "CycleB", body );
        writer.exports[ 1 ].forcedSize = 5000000;
        writer.exports[ 2 ].forcedOffset = 90000000;
        writer.exports[ 3 ].outer = 5;
        writer.exports[ 4 ].outer = 4;
        GUCEF::vector< UInt8 > bytes;
        writer.Build( bytes );
        CUnrealPackage package;
        ASSERT_TRUE( package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "e", CUnrealReaderLimits() ) );
        ASSERT_TRUE( package.IsExportRangeValid( 0 ) && !package.IsExportRangeValid( 1 ) && !package.IsExportRangeValid( 2 ) && package.IsExportRangeValid( 3 ) );
        ASSERT_TRUE( 2 == package.GetProblems().size() );
        ASSERT_TRUE( package.GetProblems()[ 0 ].HasSubstr( "Export 1" ) >= 0 && package.GetProblems()[ 0 ].HasSubstr( "beyond the package size" ) >= 0 );
        CUnrealArchiveReader reader;
        ASSERT_TRUE( package.OpenExportRange( 0, reader ) && !package.OpenExportRange( 1, reader ) && !package.OpenExportRange( 99, reader ) );
        // a cycle of outers ends (the path is the part that was followed)
        const CORE::CString path = package.GetExportPath( 3 );
        ASSERT_TRUE( !path.IsNULLOrEmpty() && path.Length() < 2000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Embedded: a package inside a larger file (a stored entry of an archive) is read with the offsets of the package" )
    try
    {
        CTestPackageWriter writer( -9, 522, 1018 );
        writer.AddPayload( GUCEF::vector< UInt8 >( 700, 5 ) );
        GUCEF::vector< UInt8 > bytes;
        BuildBasicPackage( writer, bytes );
        GUCEF::vector< UInt8 > file( 5000, 0xEE );
        file.insert( file.end(), bytes.begin(), bytes.end() );
        file.insert( file.end(), 3000, 0xDD );
        ASSERT_TRUE( WriteWorkFile( "embedded.bin", file ) );
        CUnrealPackage package;
        ASSERT_TRUE( package.OpenEmbedded( WorkVfsPath( "embedded.bin" ), 5000, bytes.size() ) );
        ASSERT_TRUE( CheckBasicPackage( package, writer ) );
        ASSERT_TRUE( package.HasTrailer() && 1 == package.GetTrailer().GetEntries().size() && bytes.size() == package.GetSource().GetSize() );
        CUnrealPackage outside;
        ASSERT_FALSE( outside.OpenEmbedded( WorkVfsPath( "embedded.bin" ), 5000, file.size() ) );
        ASSERT_FALSE( outside.OpenEmbedded( WorkVfsPath( "embedded.bin" ), 2000, bytes.size() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Memory: a package of 48 MiB is read by range: the header only, payloads and exports on demand, budgets enforced" )
    try
    {
        CTestPackageWriter writer( -9, 522, 1018 );
        GUCEF::vector< UInt8 > blob( 24 * 1024 * 1024 );
        for ( size_t i=0; i<blob.size(); i+=4093 ) blob[ i ] = (UInt8) i;
        const UInt32 payload = writer.AddPayload( blob );
        CTestByteWriter body;
        writer.BeginBody( body );
        CTestPropertyWriter props( body, writer.names, 522, 1018 );
        props.None();
        writer.WriteEditorBulkData( body, payload );
        CTestByteWriter bulk;
        bulk.Zero( 24 * 1024 * 1024 );
        body.Append( bulk );
        writer.AddExport( 0, 0, "Big", body );
        GUCEF::vector< UInt8 > bytes;
        writer.Build( bytes );
        ASSERT_TRUE( bytes.size() > 48 * 1024 * 1024 );
        ASSERT_TRUE( WriteWorkFile( "big.uasset", bytes ) );
        const UInt64 fileSize = bytes.size();
        GUCEF::vector< UInt8 >().swap( bytes );

        // the header budget is one MiB and the window 64 KiB: opening must not read the 48 MiB
        CUnrealReaderLimits limits;
        limits.maxHeaderBytes = 1024 * 1024;
        limits.windowBytes = 64 * 1024;
        limits.maxPropertyBytes = 1024 * 1024;
        CUnrealPackage package;
        ASSERT_TRUE( package.Open( WorkVfsPath( "big.uasset" ), limits ) );
        ASSERT_TRUE( fileSize == package.GetSource().GetSize() );
        ASSERT_TRUE( package.GetBytesFetched() < 512 * 1024 );
        ASSERT_TRUE( package.HasTrailer() && 1 == package.GetTrailer().GetEntries().size() );
        // the export is larger than the budget of the properties: its property range is refused, its whole range is a reader
        CUnrealArchiveReader refused;
        ASSERT_FALSE( package.OpenPropertyRange( 0, refused ) );
        CUnrealArchiveReader whole;
        ASSERT_TRUE( package.OpenExportRange( 0, whole ) && whole.GetRemaining() > 24 * 1024 * 1024 );
        // reading a few bytes in the middle takes one window, not the file
        whole.Seek( whole.GetStart() + 12 * 1024 * 1024 );
        whole.ReadUInt32();
        ASSERT_TRUE( whole.GetBytesFetched() <= 64 * 1024 );
        // the payload is read in ranges: a thousand bytes of its end
        CUnrealArchiveReader tail;
        CUnrealPayloadReference reference;
        CUnrealEditorBulkData record;
        CUnrealArchiveReader records;
        ASSERT_TRUE( package.OpenExportRange( 0, records ) );
        records.Skip( 1 );
        // after the 'None' terminator of the tags (8 bytes) the record follows
        records.Skip( 8 );
        ASSERT_TRUE( record.Read( records, package.GetVersions() ) );
        record.ToReference( &package.GetTrailer(), reference );
        ASSERT_TRUE( reference.IsReadable() && 24u * 1024 * 1024 == reference.rawSize );
        CUnrealArchiveReader payloadReader;
        ASSERT_TRUE( package.OpenPayload( reference, payloadReader ) );
        payloadReader.Skip( reference.storedSize - 1000 );
        GUCEF::vector< UInt8 > last( 1000 );
        ASSERT_TRUE( payloadReader.Read( &last[ 0 ], 1000 ) );
        ASSERT_TRUE( payloadReader.GetBytesFetched() <= 64 * 1024 );
        // the same package with a header budget below its header is refused with the budget in the message
        CUnrealReaderLimits tight;
        tight.maxHeaderBytes = 200;
        CUnrealPackage refusedPackage;
        ASSERT_FALSE( refusedPackage.Open( WorkVfsPath( "big.uasset" ), tight ) );
        ASSERT_TRUE( !refusedPackage.GetError().IsNULLOrEmpty() );
        DeleteWorkFile( "big.uasset" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

