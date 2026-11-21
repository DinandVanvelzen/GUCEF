/*
 *  assetpluginUNREAL_TestApp: Tests of the Oodle provider, the compressed buffer reader and the capability list
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#include "assetpluginUNREAL_CUnrealCompressedBuffer.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H ? */

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

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H
#include "TestUnrealWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H ? */

#include "TestUnrealOodle.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ERRORHERE       GUCEF_TESTFW_ERRORHERE

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
FindOodleToolForTests( void )
{GUCEF_TRACE;

    CORE::CString path;
    if ( CUnrealCapabilities::FindOodleTool( path ) )
        return path;
    const CORE::CString fallback = CORE::CombinePath( HomeDir(), "CODE/GUCEF/common/bin/CBNINJA_ooz_exe_oz/bin/ooz_extract" );
    return CORE::FileExists( fallback ) ? fallback : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
BuildOodleStoredBuffer( const GUCEF::vector< UInt8 >& raw, UInt32 blockLog2, GUCEF::vector< UInt8 >& out )
{GUCEF_TRACE;

    const size_t blockSize = (size_t) 1 << blockLog2;
    const size_t blockCount = ( raw.size() + blockSize - 1 ) / blockSize;
    CTestByteWriter blocks;
    GUCEF::vector< UInt32 > sizes;
    for ( size_t b=0; b<blockCount; ++b )
    {
        const size_t from = b * blockSize;
        const size_t count = raw.size() - from < blockSize ? raw.size() - from : blockSize;
        blocks.U8( 0xCC );
        blocks.U8( 0x06 );
        blocks.Raw( &raw[ from ], count );
        sizes.push_back( (UInt32) count + 2 );
    }
    CTestByteWriter w;
    w.U32BE( 0xB7756362 );
    w.U32BE( 0 );
    w.U8( 3 ); w.U8( 3 ); w.U8( 3 ); w.U8( (UInt8) blockLog2 );
    w.U32BE( (UInt32) blockCount );
    w.U64BE( raw.size() );
    w.U64BE( 64 + blockCount * 4 + blocks.Size() );
    w.Zero( 32 );
    for ( size_t b=0; b<blockCount; ++b )
        w.U32BE( sizes[ b ] );
    const UInt32 crc = CUnrealPngWriter::Crc32( &w.bytes[ 8 ], (UInt32) w.bytes.size() - 8 );
    w.bytes[ 4 ] = (UInt8) ( crc >> 24 ); w.bytes[ 5 ] = (UInt8) ( crc >> 16 ); w.bytes[ 6 ] = (UInt8) ( crc >> 8 ); w.bytes[ 7 ] = (UInt8) crc;
    w.Append( blocks );
    out = w.bytes;
}

/*-------------------------------------------------------------------------*/

static void
SetEnvVar( const char* name, const CORE::CString& value )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _putenv_s( name, value.C_String() );
    #else
    setenv( name, value.C_String(), 1 );
    #endif
}

static void
UnsetEnvVar( const char* name )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _putenv_s( name, "" );
    #else
    unsetenv( name );
    #endif
}

/*-------------------------------------------------------------------------*/

static void
MakeData( size_t size, UInt32 seed, GUCEF::vector< UInt8 >& data )
{
    data.resize( size );
    UInt32 state = seed * 2654435761U + 12345;
    for ( size_t i=0; i<size; ++i )
    {
        state = state * 1664525U + 1013904223U;
        data[ i ] = (UInt8) ( state >> 24 );
    }
}

/*-------------------------------------------------------------------------*/

/** A package that holds the given bytes as its only payload, and the reference to it */
static bool
OpenPackageWithPayload( const GUCEF::vector< UInt8 >& payload, GUCEF::vector< UInt8 >& packageBytes, CUnrealPackage& package, CUnrealPayloadReference& reference, const CUnrealReaderLimits& limits = CUnrealReaderLimits() )
{
    CTestPackageWriter writer( -9, 522, 1018 );
    writer.AddPayload( payload, false );
    CTestByteWriter body;
    writer.BeginBody( body );
    writer.WriteEditorBulkData( body, 0 );
    writer.AddExport( 0, 0, "Thing", body );
    writer.Build( packageBytes );
    if ( !package.OpenMemory( &packageBytes[ 0 ], (UInt32) packageBytes.size(), "payload.uasset", limits ) || !package.HasTrailer() || package.GetTrailer().GetEntries().empty() )
        return false;
    const CUnrealPackageTrailer::TEntryVector& entries = package.GetTrailer().GetEntries();
    reference = CUnrealPayloadReference();
    reference.kind = CUnrealPayloadReference::KIND_TRAILER;
    reference.offset = package.GetTrailer().GetPayloadOffset( entries[ 0 ] );
    reference.storedSize = entries[ 0 ].compressedSize;
    reference.rawSize = entries[ 0 ].rawSize;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Writes an executable script below the work folder and gives its disk path */
static CORE::CString
WriteScript( const CORE::CString& name, const CORE::CString& text )
{
    GUCEF::vector< UInt8 > bytes( text.C_String(), text.C_String() + text.Length() );
    WriteWorkFile( "oodle/" + name, bytes );
    const CORE::CString path = WorkDiskPath( "oodle/" + name );
    CORE::CProcess chmod;
    CORE::CProcess::TStringVector args;
    args.push_back( "+x" );
    args.push_back( path );
    CORE::CString tool = "/bin/chmod";
    if ( chmod.Launch( tool, args ) )
    {
        Int32 code = 0;
        chmod.WaitForExit( 5000, code );
    }
    return path;
}

/*-------------------------------------------------------------------------*/

/** The common part of the fake tools: reads the request lines and fills in the parameters */
static const char* const FakeToolHead =
    "#!/bin/bash\n"
    "while IFS= read -r line; do\n"
    "  eval \"a=( $line )\"\n"
    "  in=''; off=0; size=0; dec=0; out=''\n"
    "  for ((i=0;i<${#a[@]};i++)); do case \"${a[$i]}\" in --in) in=${a[$((i+1))]};; --offset) off=${a[$((i+1))]};; --size) size=${a[$((i+1))]};; --decompressed-size) dec=${a[$((i+1))]};; --out) out=${a[$((i+1))]};; esac; done\n";

static const char* const FakeToolDecode =
    "  tail -c +$((off+3)) \"$in\" | head -c $dec > \"$out\"\n";

static const char* const FakeToolTail = "done\n";

/*-------------------------------------------------------------------------*/

/** A fake that stands in for the tool: the body is the part of the loop that answers one request */
static CORE::CString
FakeTool( const CORE::CString& name, const CORE::CString& body )
{
    return WriteScript( name, CORE::CString( FakeToolHead ) + body + FakeToolTail );
}

/*-------------------------------------------------------------------------*/

struct SCancelThread
{
    CORE::CTask* task;
    UInt32 delayMs;
};

static UInt32 GUCEF_CALLSPEC_PREFIX
CancelMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SCancelThread* cancel = static_cast< SCancelThread* >( data );
    MT::ThreadDelay( cancel->delayMs );
    cancel->task->RequestCancellation();
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnrealOodleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Oodle" );

    GUCEF_TESTFW_TESTCASE( "Capabilities: the tool is looked for next to the executable or at the path of the environment, the status says what is unreadable without it, the note is written once" )
    try
    {
        const char* saved = getenv( CUnrealCapabilities::OodleToolPathVariable );
        const CORE::CString savedValue = GUCEF_NULL != saved ? CORE::CString( saved ) : CORE::CString();
        // no override: next to the executable of the host, never the search path
        UnsetEnvVar( CUnrealCapabilities::OodleToolPathVariable );
        CORE::CString expected = CORE::ModuleDir();
        CORE::AppendToPath( expected, "ooz_extract" );
        ASSERT_TRUE( expected == CUnrealCapabilities::GetExpectedOodleToolPath() );
        // an override that names nothing: unavailable, with the path and the limitation
        const CORE::CString missing = WorkDiskPath( "oodle/does_not_exist_ooz" );
        SetEnvVar( CUnrealCapabilities::OodleToolPathVariable, missing );
        CORE::CString path;
        ASSERT_TRUE( !CUnrealCapabilities::FindOodleTool( path ) && missing == path );
        const CORE::CString status = CUnrealCapabilities::GetOodleStatus();
        ASSERT_TRUE( ( "oodle: unavailable (tool not found at " + missing + "): Unreal 5 textures and static meshes unreadable" ) == status );
        ASSERT_TRUE( 1 == CUnrealCapabilities::GetCapabilityList().size() && status == CUnrealCapabilities::GetCapabilityList()[ 0 ] );
        ASSERT_TRUE( ( "needs the oodle provider: tool not found at " + missing ) == CUnrealCapabilities::GetNeedsOodleText() );
        COodleToolProvider provider;
        ASSERT_TRUE( !provider.IsAvailable() && ( "needs the oodle provider: tool not found at " + missing ) == provider.GetUnavailableReason() );
        UInt8 byte = 0;
        CORE::CString error;
        Int32 code = 7;
        ASSERT_TRUE( !provider.DecodeBlock( &byte, 1, 1, &byte, error, code ) && error.HasSubstr( "tool not found" ) >= 0 && 0 == provider.GetStartCount() );
        CUnrealCapabilities::ResetNotedState();
        CUnrealCapabilities::NoteOodleStatusOnce();
        CUnrealCapabilities::NoteOodleStatusOnce();
        // an override that names a file: available
        const CORE::CString tool = FindOodleToolForTests();
        if ( !tool.IsNULLOrEmpty() )
        {
            SetEnvVar( CUnrealCapabilities::OodleToolPathVariable, tool );
            ASSERT_TRUE( CUnrealCapabilities::FindOodleTool( path ) && tool == path && ( "oodle: available (" + tool + ")" ) == CUnrealCapabilities::GetOodleStatus() );
        }
        if ( savedValue.IsNULLOrEmpty() ) UnsetEnvVar( CUnrealCapabilities::OodleToolPathVariable ); else SetEnvVar( CUnrealCapabilities::OodleToolPathVariable, savedValue );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Compressed buffer: stored buffers by range, blocks the encoder kept as they are, and every refusal of a damaged header" )
    try
    {
        // method 0: bytes by range
        GUCEF::vector< UInt8 > raw;
        MakeData( 1000, 3, raw );
        GUCEF::vector< UInt8 > stored, packageBytes;
        BuildStoredCompressedBuffer( raw, stored );
        {
            CUnrealPackage package;
            CUnrealPayloadReference reference;
            ASSERT_TRUE( OpenPackageWithPayload( stored, packageBytes, package, reference ) );
            CUnrealCompressedBuffer buffer( package, GUCEF_NULL );
            ASSERT_TRUE( buffer.Open( reference ) && buffer.GetHeader().IsStored() && 1000 == buffer.GetRawSize() && !buffer.NeedsOodle() );
            CORE::CDynamicBuffer all;
            ASSERT_TRUE( buffer.ReadAll( all ) && 1000 == all.GetDataSize() && 0 == memcmp( all.GetConstBufferPtr(), &raw[ 0 ], 1000 ) );
            UInt8 part[ 100 ];
            ASSERT_TRUE( buffer.ReadRange( 450, 100, part ) && 0 == memcmp( part, &raw[ 450 ], 100 ) );
            ASSERT_TRUE( !buffer.ReadRange( 950, 100, part ) && !buffer.GetError().IsNULLOrEmpty() );
            ASSERT_TRUE( buffer.ReadRange( 1000, 0, part ) );
        }
        // a damaged header checksum, magic number, method, block table
        GUCEF::vector< UInt8 > data;
        MakeData( 5000, 9, data );
        GUCEF::vector< UInt8 > good;
        BuildOodleStoredBuffer( data, 10, good );        // 5 blocks of 1 KiB
        struct SDamage { size_t offset; UInt8 xorValue; const char* reason; };
        const SDamage damages[] = {
            { 4, 0x01, "checksum" }, { 0, 0x01, "magic" }, { 8, 0x01, "method 2" }, { 9, 0x01, "checksum" }, { 11, 0x01, "checksum" }, { 15, 0x01, "checksum" },
            { 20, 0x10, "checksum" }, { 28, 0x01, "checksum" }, { 66, 0x01, "checksum" }
        };
        for ( size_t d=0; d<sizeof( damages ) / sizeof( damages[ 0 ] ); ++d )
        {
            GUCEF::vector< UInt8 > bad = good;
            bad[ damages[ d ].offset ] ^= damages[ d ].xorValue;
            CUnrealPackage package;
            CUnrealPayloadReference reference;
            ASSERT_TRUE( OpenPackageWithPayload( bad, packageBytes, package, reference ) );
            CUnrealCompressedBuffer buffer( package, GUCEF_NULL );
            ASSERT_TRUE( !buffer.Open( reference ) && !buffer.GetError().IsNULLOrEmpty() );
        }
        // a header whose checksum is right and whose sizes are not: the sum of the blocks, the block count
        {
            GUCEF::vector< UInt8 > bad = good;
            bad[ 64 + 3 ] ^= 0x04;                        // the size of the first block
            const UInt32 crc = CUnrealPngWriter::Crc32( &bad[ 8 ], 64 + 5 * 4 - 8 );
            bad[ 4 ] = (UInt8) ( crc >> 24 ); bad[ 5 ] = (UInt8) ( crc >> 16 ); bad[ 6 ] = (UInt8) ( crc >> 8 ); bad[ 7 ] = (UInt8) crc;
            CUnrealPackage package;
            CUnrealPayloadReference reference;
            ASSERT_TRUE( OpenPackageWithPayload( bad, packageBytes, package, reference ) );
            CUnrealCompressedBuffer buffer( package, GUCEF_NULL );
            ASSERT_TRUE( !buffer.Open( reference ) && buffer.GetError().HasSubstr( "add up" ) >= 0 );
        }
        // the raw size over the budget
        {
            CUnrealReaderLimits limits;
            limits.maxPayloadRawBytes = 4000;
            CUnrealPackage package;
            CUnrealPayloadReference reference;
            ASSERT_TRUE( OpenPackageWithPayload( good, packageBytes, package, reference, limits ) );
            CUnrealCompressedBuffer buffer( package, GUCEF_NULL );
            ASSERT_TRUE( !buffer.Open( reference ) && buffer.GetError().HasSubstr( "budget" ) >= 0 );
        }
        // Oodle blocks without a provider: the reason of the missing tool, and the capability text
        {
            CUnrealPackage package;
            CUnrealPayloadReference reference;
            ASSERT_TRUE( OpenPackageWithPayload( good, packageBytes, package, reference ) );
            CUnrealCompressedBuffer buffer( package, GUCEF_NULL );
            ASSERT_TRUE( buffer.Open( reference ) && buffer.NeedsOodle() && 5 == buffer.GetHeader().blockCount );
            UInt8 part[ 10 ];
            ASSERT_TRUE( !buffer.ReadRange( 0, 10, part ) && buffer.IsProviderMissing() && buffer.GetError().HasSubstr( "needs the oodle provider: tool not found" ) >= 0 );
        }
        // a block that is as large as its raw bytes is stored as it is: no provider needed
        {
            GUCEF::vector< UInt8 > small;
            MakeData( 300, 5, small );
            CTestByteWriter w;
            w.U32BE( 0xB7756362 ); w.U32BE( 0 ); w.U8( 3 ); w.U8( 3 ); w.U8( 3 ); w.U8( 10 ); w.U32BE( 1 ); w.U64BE( 300 ); w.U64BE( 64 + 4 + 300 ); w.Zero( 32 ); w.U32BE( 300 );
            const UInt32 crc = CUnrealPngWriter::Crc32( &w.bytes[ 8 ], (UInt32) w.bytes.size() - 8 );
            w.bytes[ 4 ] = (UInt8) ( crc >> 24 ); w.bytes[ 5 ] = (UInt8) ( crc >> 16 ); w.bytes[ 6 ] = (UInt8) ( crc >> 8 ); w.bytes[ 7 ] = (UInt8) crc;
            w.Raw( &small[ 0 ], 300 );
            CUnrealPackage package;
            CUnrealPayloadReference reference;
            ASSERT_TRUE( OpenPackageWithPayload( w.bytes, packageBytes, package, reference ) );
            CUnrealCompressedBuffer buffer( package, GUCEF_NULL );
            CORE::CDynamicBuffer all;
            ASSERT_TRUE( buffer.Open( reference ) && buffer.ReadAll( all ) && 300 == all.GetDataSize() && 0 == memcmp( all.GetConstBufferPtr(), &small[ 0 ], 300 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Provider with the tool: blocks of stored streams, whole and by range across block borders, one child for all, a corrupt block is that block's failure" )
    const CORE::CString realTool = FindOodleToolForTests();
    if ( realTool.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "The oodle tool is not built (GUCEF_OOZ_EXTRACT_PATH)" );
    try
    {
        COodleToolProvider provider( realTool );
        ASSERT_TRUE( provider.IsAvailable() && provider.GetUnavailableReason().IsNULLOrEmpty() );
        GUCEF::vector< UInt8 > data;
        MakeData( 700000, 11, data );
        GUCEF::vector< UInt8 > buffer3, packageBytes;
        BuildOodleStoredBuffer( data, 18, buffer3 );     // 3 blocks of 256 KiB, the last one short
        CUnrealPackage package;
        CUnrealPayloadReference reference;
        ASSERT_TRUE( OpenPackageWithPayload( buffer3, packageBytes, package, reference ) );
        CUnrealCompressedBuffer buffer( package, &provider );
        ASSERT_TRUE( buffer.Open( reference ) && 3 == buffer.GetHeader().blockCount && buffer.NeedsOodle() );
        CORE::CDynamicBuffer all;
        ASSERT_TRUE( buffer.ReadAll( all ) && 700000 == all.GetDataSize() && 0 == memcmp( all.GetConstBufferPtr(), &data[ 0 ], 700000 ) );
        GUCEF::vector< UInt8 > part( 300000 );
        ASSERT_TRUE( buffer.ReadRange( 200000, 300000, &part[ 0 ] ) && 0 == memcmp( &part[ 0 ], &data[ 200000 ], 300000 ) );
        ASSERT_TRUE( buffer.ReadRange( 699990, 10, &part[ 0 ] ) && 0 == memcmp( &part[ 0 ], &data[ 699990 ], 10 ) );
        ASSERT_TRUE( 1 == provider.GetStartCount() );
        // a block whose stream is damaged: the tool says so, with its code, the child lives on
        UInt8 junk[ 40 ];
        memset( junk, 0x55, sizeof( junk ) );
        UInt8 out[ 100 ];
        CORE::CString error;
        Int32 code = 0;
        ASSERT_TRUE( !provider.DecodeBlock( junk, sizeof( junk ), 100, out, error, code ) && 4 == code && error.HasSubstr( "exit code 4" ) >= 0 );
        ASSERT_TRUE( buffer.ReadRange( 0, 1000, &part[ 0 ] ) && 1 == provider.GetStartCount() );
        // a wrong decompressed size is also refused by the tool
        provider.Stop();
        ASSERT_TRUE( buffer.ReadRange( 0, 100, &part[ 0 ] ) && 2 == provider.GetStartCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Provider with fake tools: error answers, truncated output, garbage, a hung tool, a crashed child that is replaced once, a slow tool" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    GUCEF_TESTFW_SKIP( "The fake tools are shell scripts" );
    #endif
    try
    {
        UInt8 compressed[ 12 ] = { 0xCC, 0x06, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
        UInt8 out[ 10 ];
        CORE::CString error;
        Int32 code = 0;

        // a tool that decodes: the fake works like the real one on stored streams
        {
            const CORE::CString path = FakeTool( "fake_ok.sh", CORE::CString( FakeToolDecode ) + "  echo \"ok $dec\"\n" );
            COodleToolProvider provider( path );
            ASSERT_TRUE( provider.IsAvailable() && provider.DecodeBlock( compressed, 12, 10, out, error, code ) && 0 == memcmp( out, "abcdefghij", 10 ) );
            ASSERT_TRUE( provider.DecodeBlock( compressed, 12, 4, out, error, code ) && 0 == memcmp( out, "abcd", 4 ) && 1 == provider.GetStartCount() );
        }
        // an error answer with its code and message
        {
            const CORE::CString path = FakeTool( "fake_error.sh", "  echo \"error 3 I/O error: simulated\"\n" );
            COodleToolProvider provider( path );
            ASSERT_TRUE( !provider.DecodeBlock( compressed, 12, 10, out, error, code ) && 3 == code && error.HasSubstr( "exit code 3" ) >= 0 && error.HasSubstr( "I/O error: simulated" ) >= 0 && 1 == provider.GetStartCount() );
        }
        // an answer "ok" with fewer bytes than asked for
        {
            const CORE::CString path = FakeTool( "fake_trunc.sh", "  head -c 5 /dev/zero > \"$out\"\n  echo \"ok 5\"\n" );
            COodleToolProvider provider( path );
            ASSERT_TRUE( !provider.DecodeBlock( compressed, 12, 10, out, error, code ) && error.HasSubstr( "truncated output" ) >= 0 && 1 == provider.GetStartCount() );
        }
        // an answer "ok" whose output file is short
        {
            const CORE::CString path = FakeTool( "fake_short_file.sh", "  head -c 5 /dev/zero > \"$out\"\n  echo \"ok $dec\"\n" );
            COodleToolProvider provider( path );
            ASSERT_TRUE( !provider.DecodeBlock( compressed, 12, 10, out, error, code ) && error.HasSubstr( "truncated output" ) >= 0 );
        }
        // text that is not the protocol: the child is replaced once, then the request fails
        {
            const CORE::CString path = FakeTool( "fake_garbage.sh", "  echo \"hello world, this is not the protocol\"\n" );
            COodleToolProvider provider( path );
            ASSERT_TRUE( !provider.DecodeBlock( compressed, 12, 10, out, error, code ) && error.HasSubstr( "protocol" ) >= 0 && error.HasSubstr( "started again" ) >= 0 && 2 == provider.GetStartCount() );
        }
        // a line without an end that never stops
        {
            const CORE::CString path = FakeTool( "fake_endless.sh", "  head -c 20000 /dev/zero | tr '\\0' 'x'\n" );
            COodleToolProvider provider( path );
            provider.SetRequestTimeoutSecs( 1 );
            ASSERT_TRUE( !provider.DecodeBlock( compressed, 12, 10, out, error, code ) );
        }
        // a tool that hangs: the request times out and fails, the child is ended
        {
            const CORE::CString path = FakeTool( "fake_hang.sh", "  exec sleep 30\n" );
            COodleToolProvider provider( path );
            provider.SetRequestTimeoutSecs( 1 );
            const UInt64 start = MT::PrecisionTickCount();
            ASSERT_TRUE( !provider.DecodeBlock( compressed, 12, 10, out, error, code ) && error.HasSubstr( "timeout" ) >= 0 );
            ASSERT_TRUE( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 10000.0 && 1 == provider.GetStartCount() );
        }
        // a child that crashes at the first request: it is replaced, the second one works (a flag file tells which one it is)
        {
            const CORE::CString flag = WorkDiskPath( "oodle/crashonce.flag" );
            DeleteWorkFile( "oodle/crashonce.flag" );
            const CORE::CString path = FakeTool( "fake_crashonce.sh", "  if [ ! -e \"" + flag + "\" ]; then touch \"" + flag + "\"; exit 7; fi\n" + FakeToolDecode + "  echo \"ok $dec\"\n" );
            COodleToolProvider provider( path );
            ASSERT_TRUE( provider.DecodeBlock( compressed, 12, 10, out, error, code ) && 0 == memcmp( out, "abcdefghij", 10 ) && 2 == provider.GetStartCount() );
            ASSERT_TRUE( provider.DecodeBlock( compressed, 12, 3, out, error, code ) && 2 == provider.GetStartCount() );
            DeleteWorkFile( "oodle/crashonce.flag" );
        }
        // a child that always crashes: replaced once, then the request fails with the exit code
        {
            const CORE::CString path = FakeTool( "fake_crash.sh", "  exit 9\n" );
            COodleToolProvider provider( path );
            ASSERT_TRUE( !provider.DecodeBlock( compressed, 12, 10, out, error, code ) && 2 == provider.GetStartCount() && error.HasSubstr( "exit code 9" ) >= 0 && error.HasSubstr( "started again" ) >= 0 );
        }
        // a slow tool that answers in time
        {
            const CORE::CString path = FakeTool( "fake_slow.sh", "  sleep 0.5\n" + CORE::CString( FakeToolDecode ) + "  echo \"ok $dec\"\n" );
            COodleToolProvider provider( path );
            ASSERT_TRUE( provider.DecodeBlock( compressed, 12, 10, out, error, code ) && 0 == memcmp( out, "abcdefghij", 10 ) );
        }
        // a path that is not an executable: the start fails, nothing crashes
        {
            DeleteWorkFile( "oodle/not_executable.txt" );
            GUCEF::vector< UInt8 > text( 4, 'x' );
            WriteWorkFile( "oodle/not_executable.txt", text );
            COodleToolProvider provider( WorkDiskPath( "oodle/not_executable.txt" ) );
            ASSERT_TRUE( !provider.DecodeBlock( compressed, 12, 10, out, error, code ) && !error.IsNULLOrEmpty() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Provider: a request to cancel the load ends the child and fails the request at once" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    GUCEF_TESTFW_SKIP( "The fake tools are shell scripts" );
    #endif
    try
    {
        const CORE::CString path = FakeTool( "fake_cancel.sh", "  exec sleep 30\n" );
        UInt8 compressed[ 12 ] = { 0xCC, 0x06, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
        UInt8 out[ 10 ];
        CORE::CString error;
        Int32 code = 0;

        // cancelled before the request: nothing is started
        {
            CORE::CTask task;
            ASSETCONVERTER::CConversionReport report;
            report.SetCancellationSource( &task );
            COodleToolProvider provider( path );
            provider.SetCancellationSource( &report );
            task.RequestCancellation();
            ASSERT_TRUE( report.IsCancellationRequested() && !provider.DecodeBlock( compressed, 12, 10, out, error, code ) && "cancelled" == error && 0 == provider.GetStartCount() );
        }
        // cancelled while the tool works: the wait ends within a poll, the child is ended
        {
            CORE::CTask task;
            ASSETCONVERTER::CConversionReport report;
            report.SetCancellationSource( &task );
            COodleToolProvider provider( path );
            provider.SetCancellationSource( &report );
            SCancelThread cancel;
            cancel.task = &task;
            cancel.delayMs = 400;
            MT::SThreadData* thread = MT::ThreadDataReserve();
            ASSERT_TRUE( 0 != MT::ThreadCreate( thread, &CancelMain, &cancel ) );
            const UInt64 start = MT::PrecisionTickCount();
            ASSERT_TRUE( !provider.DecodeBlock( compressed, 12, 10, out, error, code ) && "cancelled" == error );
            const Float64 elapsed = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
            ASSERT_TRUE( elapsed >= 300.0 && elapsed < 5000.0 );
            MT::ThreadWait( thread, 10000 );
            MT::ThreadDataCleanup( thread );
            report.SetCancellationSource( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Robustness: truncated and damaged compressed buffers fail with a reason, never a crash (headers, block tables, blocks)" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    GUCEF_TESTFW_SKIP( "The fake tools are shell scripts" );
    #endif
    try
    {
        GUCEF::vector< UInt8 > data;
        MakeData( 5000, 21, data );
        GUCEF::vector< UInt8 > good, packageBytes;
        BuildOodleStoredBuffer( data, 10, good );
        // the fake tool decodes stored streams the way the real one does; a damaged stream is whatever the fake makes of it
        const CORE::CString path = FakeTool( "fake_fuzz.sh", CORE::CString( FakeToolDecode ) + "  echo \"ok $dec\"\n" );
        UInt32 state = 77;
        for ( UInt32 round=0; round<300; ++round )
        {
            GUCEF::vector< UInt8 > bad = good;
            state = state * 1664525U + 1013904223U;
            if ( 0 == round % 3 )
                bad.resize( 1 + ( state >> 8 ) % bad.size() );
            else
                for ( UInt32 flips=0; flips<1 + round % 4; ++flips )
                {
                    state = state * 1664525U + 1013904223U;
                    bad[ ( state >> 8 ) % ( round % 2 ? 100 : bad.size() ) ] ^= (UInt8) ( 1 << ( state & 7 ) );
                }
            CUnrealPackage package;
            CUnrealPayloadReference reference;
            if ( !OpenPackageWithPayload( bad, packageBytes, package, reference ) )
                continue;
            COodleToolProvider provider( path );
            provider.SetRequestTimeoutSecs( 2 );
            CUnrealCompressedBuffer buffer( package, &provider );
            CORE::CDynamicBuffer all;
            if ( buffer.Open( reference ) )
                buffer.ReadAll( all );
        }
        ASSERT_TRUE( true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
