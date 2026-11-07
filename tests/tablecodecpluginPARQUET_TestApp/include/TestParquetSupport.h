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

#ifndef TABLECODECPLUGIN_PARQUET_TESTAPP_TESTPARQUETSUPPORT_H
#define TABLECODECPLUGIN_PARQUET_TESTAPP_TESTPARQUETSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_CORE_CDATATABLESCANSTATS_H
#include "gucefCORE_CDataTableScanStats.h"
#define GUCEF_CORE_CDATATABLESCANSTATS_H
#endif /* GUCEF_CORE_CDATATABLESCANSTATS_H ? */

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#include "gucefCORE_CDataTableWriterOptions.h"
#define GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#endif /* GUCEF_CORE_CDATATABLEWRITEROPTIONS_H ? */

#ifndef GUCEF_CORE_CFILEDATATABLE_H
#include "gucefCORE_CFileDataTable.h"
#define GUCEF_CORE_CFILEDATATABLE_H
#endif /* GUCEF_CORE_CFILEDATATABLE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace PARQUETTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef GUCEF::vector< GUCEF::CORE::CDataTableBatchPtr > TBatchVector;

/** A small, fast, seeded generator, so random tests repeat exactly */
class CTestRandom
{
    public:

    explicit CTestRandom( GUCEF::UInt64 seed ) : m_state( 0 != seed ? seed : 0x9E3779B97F4A7C15ULL ) {}

    inline GUCEF::UInt64 Next( void )
    {
        m_state ^= m_state << 13;
        m_state ^= m_state >> 7;
        m_state ^= m_state << 17;
        return m_state;
    }

    inline GUCEF::UInt32 Below( GUCEF::UInt32 bound ) { return 0 == bound ? 0 : (GUCEF::UInt32) ( Next() % bound ); }

    private:

    GUCEF::UInt64 m_state;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Loads codecspluginZLIB and dstorepluginJSONPARSER once, from the directory of the module */
void LoadRuntimePlugins( void );
bool IsGzipAvailable( void );
bool IsJsonAvailable( void );

/** One parquet codec object for the tests */
GUCEF::CORE::CFileDataTable::TTableCodecPtr GetParquetCodec( void );

/** Read access to the bytes of a file held in memory; the buffer must outlive the access */
GUCEF::CORE::IOAccessPtr MakeReadAccess( const GUCEF::CORE::CDynamicBuffer& data );

/** Writes the batches with the codec into the buffer; each batch is followed by a row group flush when flushEach is set */
GUCEF::CORE::TDataTableStatus WriteParquet( const GUCEF::CORE::CDataTableSchemaPtr& schema                ,
                                            const TBatchVector& batches                                    ,
                                            const GUCEF::CORE::CDataTableWriterOptions& options            ,
                                            GUCEF::CORE::CDynamicBuffer& file                              ,
                                            GUCEF::CORE::CString& error                                    ,
                                            bool flushEach = false                                         ,
                                            GUCEF::CORE::CDataTableFileSummary* summary = GUCEF_NULL       );

/**
 *  Reads every row group of the file with the projection (empty: all columns) into batches, one per read batch.
 *  stats collects the reader counters when given.
 */
GUCEF::CORE::TDataTableStatus ReadParquet( const GUCEF::CORE::CDynamicBuffer& file                       ,
                                           TBatchVector& batches                                          ,
                                           GUCEF::CORE::CString& error                                    ,
                                           const GUCEF::CORE::CDataTableSchema::TColumnIndexVector* projection = GUCEF_NULL ,
                                           GUCEF::UInt32 batchRowCount = 0                                ,
                                           GUCEF::CORE::CDataTableScanStats* stats = GUCEF_NULL           ,
                                           bool materializeDictionaries = false                           ,
                                           const GUCEF::CORE::CValueList* readOptions = GUCEF_NULL        );

/** A schema with a column of every type the codec writes: nullable or not */
GUCEF::CORE::CDataTableSchema MakeAllTypesSchema( bool nullable );

/** Random rows for the schema; nullPercent of the cells of nullable columns are null; text uses distinctTexts values (0: all different) */
GUCEF::CORE::CDataTableBatchPtr MakeRandomBatch( const GUCEF::CORE::CDataTableSchemaPtr& schema, GUCEF::UInt32 rows, GUCEF::UInt32 nullPercent,
                                                 CTestRandom& random, GUCEF::UInt32 distinctTexts = 0 );

/** Total rows of batches */
GUCEF::UInt64 CountRows( const TBatchVector& batches );

/**
 *  Whether the rows of two batch sequences are the same, cell by cell (types, nulls, values; floats by their bits).
 *  Column c of the actual batches is compared with column columnMap[ c ] of the expected ones (all when empty).
 */
bool SameRows( const TBatchVector& expected, const TBatchVector& actual, GUCEF::CORE::CString& difference,
               const GUCEF::CORE::CDataTableSchema::TColumnIndexVector* columnMap = GUCEF_NULL );

/**
 *  The canonical text of rows, as the interop scripts outside the repository write it: a line of column names, then
 *  one line per row, cells separated by tabs. N is null; booleans 0 / 1; integers, dates (days), timestamps (ticks of
 *  the unit) and decimal mantissas in decimal; floats as their IEEE bits (f and 8 hex digits, d and 16) or nan; text
 *  and binary as x and the hex of their bytes.
 */
GUCEF::CORE::CString DumpRows( const TBatchVector& batches, const GUCEF::CORE::CDataTableSchema& schema );

/** A file of the test directory: the temporary directory of the platform plus the name */
GUCEF::CORE::CString TestFilePath( const GUCEF::CORE::CString& name );

}; /* namespace PARQUETTEST */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_TESTAPP_TESTPARQUETSUPPORT_H ? */
