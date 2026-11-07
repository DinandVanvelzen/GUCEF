/*
 *  tablecodecpluginPARQUET_TestApp: tests of the parquet table codec plugin
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
#include <stdlib.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestParquetSupport.h"
#include "TestParquetFixtures.h"
#include "TestParquetInterop.h"

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

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace PARQUETTEST;

namespace {

/** The supported columns of the file, in file order: what an empty projection reads */
void
SupportedColumns( const CITableFileMetaDataPtr& metaData, CDataTableSchema& supported, CDataTableSchema::TColumnIndexVector& projection )
{
    supported.Clear();
    projection.clear();
    const CDataTableSchemaPtr& schema = metaData->GetSchema();
    for ( UInt32 c=0; c<schema->GetColumnCount(); ++c )
    {
        if ( DATATABLE_TYPE_UNSUPPORTED == schema->GetColumn( c ).type )
            continue;
        supported.AddColumn( schema->GetColumn( c ) );
        projection.push_back( c );
    }
}

/** Reads a file of memory and dumps its supported columns; the status of the read */
TDataTableStatus
ReadAndDump( const CDynamicBuffer& file, CString& dump, CString& error, UInt32& unsupportedColumns )
{
    unsupportedColumns = 0;
    IOAccessPtr input = MakeReadAccess( file );
    CITableFileMetaDataPtr metaData;
    TDataTableStatus status = GetParquetCodec()->ReadMetaData( *input, CValueList(), metaData, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    CDataTableSchema supported;
    CDataTableSchema::TColumnIndexVector projection;
    SupportedColumns( metaData, supported, projection );
    unsupportedColumns = metaData->GetSchema()->GetColumnCount() - supported.GetColumnCount();
    TBatchVector batches;
    status = ReadParquet( file, batches, error, &projection, 5 );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    dump = DumpRows( batches, supported );
    return DATATABLE_STATUS_OK;
}

/** The first line that differs, for the message of a failed comparison */
CString
FirstDifference( const CString& expected, const CString& actual )
{
    const char* e = expected.C_String();
    const char* a = actual.C_String();
    UInt32 line = 1;
    const char* lineStartE = e;
    const char* lineStartA = a;
    while ( '\0' != *e && *e == *a )
    {
        if ( '\n' == *e ) { ++line; lineStartE = e + 1; lineStartA = a + 1; }
        ++e; ++a;
    }
    const char* endE = strchr( lineStartE, '\n' );
    const char* endA = strchr( lineStartA, '\n' );
    return "line " + ToString( line ) + ": expected \"" + CString( lineStartE, GUCEF_NULL != endE ? (UInt32) ( endE - lineStartE ) : (UInt32) strlen( lineStartE ) ) +
           "\", got \"" + CString( lineStartA, GUCEF_NULL != endA ? (UInt32) ( endA - lineStartA ) : (UInt32) strlen( lineStartA ) ) + "\"";
}

bool
WriteTextFile( const CString& path, const CString& text )
{
    CFileAccess file( path, "wb" );
    if ( !file.IsValid() )
        return false;
    const UInt32 length = (UInt32) strlen( text.C_String() );
    const bool written = length == file.Write( text.C_String(), 1, length );
    file.Close();
    return written;
}

bool
ReadTextFile( const CString& path, CString& text )
{
    CDynamicBuffer data;
    if ( !data.LoadContentFromFile( path ) )
        return false;
    text = CString( static_cast< const char* >( data.GetConstBufferPtr() ), data.GetDataSize() );
    return true;
}

/** The files this side writes for other readers */
struct SWrittenVariant
{
    const char* name;
    bool nullable;
    UInt32 nullPercent;
    UInt32 rows;
    TDataTableDictionaryMode dictionary;
    const char* compression;
    UInt32 rowGroupRows;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformParquetInteropTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "tablecodecpluginPARQUET.Interop" );

    GUCEF_TESTFW_TESTCASE( "Interop: files built byte by byte from the specification read with the values they hold; SNAPPY and DELTA files are refused with UNSUPPORTED naming the feature" )
    try
    {
        for ( UInt32 f=0; f<ParquetFixtureCount; ++f )
        {
            const SParquetFixture& fixture = ParquetFixtures[ f ];
            if ( 0 == strcmp( fixture.note, "needs gzip" ) && !IsGzipAvailable() )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "Parquet interop: fixture " << fixture.name << " skipped, no gzip codec" );
                continue;
            }
            CDynamicBuffer file;
            file.CopyFrom( fixture.size, fixture.bytes );
            CString dump, error;
            UInt32 unsupported = 0;
            const TDataTableStatus status = ReadAndDump( file, dump, error, unsupported );
            if ( 0 == strcmp( fixture.expectedStatus, "OK" ) )
            {
                if ( DATATABLE_STATUS_OK != status )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet interop: fixture " << fixture.name << ": " << DataTableStatusToString( status ) << " " << error );
                ASSERT_TRUE( DATATABLE_STATUS_OK == status );
                const bool same = dump == fixture.dump;
                if ( !same )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet interop: fixture " << fixture.name << " differs at " << FirstDifference( fixture.dump, dump ) );
                ASSERT_TRUE( same );
                ASSERT_TRUE( ( 0 == strcmp( fixture.note, "one UNSUPPORTED column" ) ? 1u : 0u ) == unsupported );
            }
            else
            {
                ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == status );
                const bool named = GUCEF_NULL != strstr( error.C_String(), fixture.note );
                if ( !named )
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet interop: fixture " << fixture.name << " refused with \"" << error << "\"" );
                ASSERT_TRUE( named );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Interop: in the directory of GUCEF_TEST_PARQUET_INTEROP_DIR, files for other readers are written with their dumps, and the files of other writers are read" )
    GUCEF_TESTFW_REQUIRE_ENV( interopDirText, "GUCEF_TEST_PARQUET_INTEROP_DIR" );
    try
    {
        const CString interopDir( interopDirText.c_str() );
        CreateDirs( interopDir );

        // what this side writes, for other readers
        static const SWrittenVariant variants[] =
        {
            { "gucef_required", false, 0, 5000, DATATABLE_DICTIONARY_AUTO, "none", 1048576 },
            { "gucef_nullable", true, 25, 5000, DATATABLE_DICTIONARY_AUTO, "none", 1048576 },
            { "gucef_dictionary_always", true, 10, 3000, DATATABLE_DICTIONARY_ALWAYS, "none", 1048576 },
            { "gucef_plain_never", true, 10, 3000, DATATABLE_DICTIONARY_NEVER, "none", 1048576 },
            { "gucef_gzip", true, 10, 3000, DATATABLE_DICTIONARY_AUTO, "gzip", 1048576 },
            { "gucef_rowgroups", true, 10, 3500, DATATABLE_DICTIONARY_AUTO, "none", 1000 },
            { "gucef_empty", true, 0, 0, DATATABLE_DICTIONARY_AUTO, "none", 1048576 },
            { "gucef_onerow", false, 0, 1, DATATABLE_DICTIONARY_AUTO, "none", 1048576 },
            { "gucef_allnull", true, 100, 100, DATATABLE_DICTIONARY_AUTO, "none", 1048576 }
        };
        CString manifest;
        for ( UInt32 v=0; v<sizeof( variants ) / sizeof( variants[ 0 ] ); ++v )
        {
            const SWrittenVariant& variant = variants[ v ];
            if ( 0 == strcmp( variant.compression, "gzip" ) && !IsGzipAvailable() )
                continue;
            CDataTableSchema schema = MakeAllTypesSchema( variant.nullable );
            schema.SetSortKey( CString::StringVector() );
            const CDataTableSchemaPtr schemaPtr = CreateDataTableSchemaPtr( schema );
            CTestRandom random( 1000 + v );
            TBatchVector batches;
            if ( variant.rows > 0 )
                batches.push_back( MakeRandomBatch( schemaPtr, variant.rows, variant.nullPercent, random, 0 == v % 2 ? 50 : 0 ) );
            CDataTableWriterOptions options;
            options.dictionary = variant.dictionary;
            options.compression = variant.compression;
            options.rowGroupRowCount = variant.rowGroupRows;
            options.pageRowCount = 700;
            CDynamicBuffer file;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteParquet( schemaPtr, batches, options, file, error ) );
            ASSERT_TRUE( file.WriteContentToFile( CombinePath( interopDir, CString( variant.name ) + ".parquet" ), 0, true ) );
            ASSERT_TRUE( WriteTextFile( CombinePath( interopDir, CString( variant.name ) + ".dump" ), DumpRows( batches, schema ) ) );
            manifest += CString( variant.name ) + "\n";
        }
        ASSERT_TRUE( WriteTextFile( CombinePath( interopDir, "gucef_manifest.txt" ), manifest ) );

        // what other writers wrote: a manifest of "name status" lines next to the files and their dumps
        CString foreign;
        if ( ReadTextFile( CombinePath( interopDir, "foreign_manifest.txt" ), foreign ) )
        {
            CString results;
            CString::StringVector lines = foreign.ParseElements( '\n', false );
            for ( size_t i=0; i<lines.size(); ++i )
            {
                CString::StringVector parts = lines[ i ].ParseElements( ' ', false );
                if ( parts.size() < 2 )
                    continue;
                const CString& name = parts[ 0 ];
                const CString& expected = parts[ 1 ];
                if ( expected == "GZIP" && !IsGzipAvailable() )
                    continue;
                CDynamicBuffer file;
                ASSERT_TRUE( file.LoadContentFromFile( CombinePath( interopDir, name + ".parquet" ) ) );
                CString expectedDump;
                ReadTextFile( CombinePath( interopDir, name + ".dump" ), expectedDump );
                CString dump, error;
                UInt32 unsupported = 0;
                const TDataTableStatus status = ReadAndDump( file, dump, error, unsupported );
                CString outcome;
                if ( expected == "OK" || expected == "GZIP" )
                {
                    const bool good = DATATABLE_STATUS_OK == status && dump == expectedDump;
                    ASSERT_TRUE( good );
                    outcome = good ? CString( "OK" ) : CString( DataTableStatusToString( status ) ) + " " + error + ( DATATABLE_STATUS_OK == status ? " " + FirstDifference( expectedDump, dump ) : CString::Empty );
                }
                else
                {
                    const bool refused = DATATABLE_STATUS_UNSUPPORTED == status;
                    ASSERT_TRUE( refused );
                    outcome = CString( DataTableStatusToString( status ) ) + " " + error;
                }
                results += name + "\t" + outcome + "\n";
                GUCEF_LOG( LOGLEVEL_NORMAL, "Parquet interop: " << name << ": " << outcome );
            }
            WriteTextFile( CombinePath( interopDir, "gucef_read_results.txt" ), results );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
