/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCSVTABLEREADER_H
#define GUCEF_CORE_CCSVTABLEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEREADER_H
#include "gucefCORE_CITableReader.h"
#define GUCEF_CORE_CITABLEREADER_H
#endif /* GUCEF_CORE_CITABLEREADER_H ? */

#ifndef GUCEF_CORE_CCSVRECORDPARSER_H
#include "gucefCORE_CCsvRecordParser.h"
#define GUCEF_CORE_CCSVRECORDPARSER_H
#endif /* GUCEF_CORE_CCSVRECORDPARSER_H ? */

#ifndef GUCEF_CORE_CCSVCOLUMNCONVERTER_H
#include "gucefCORE_CCsvColumnConverter.h"
#define GUCEF_CORE_CCSVCOLUMNCONVERTER_H
#endif /* GUCEF_CORE_CCSVCOLUMNCONVERTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The file metadata of a CSV file: the schema (given or inferred) and key values, no row count and no statistics */
class CCsvTableMetaData : public CITableFileMetaData
{
    public:

    CCsvTableMetaData( const CDataTableSchemaPtr& schema, const CValueList& keyValues );

    virtual ~CCsvTableMetaData() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CDataTableSchemaPtr& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** GUCEF_UINT64MAX: unknown */
    virtual UInt64 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetRowGroupCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetRowGroupInfo( UInt32 rowGroup, CDataTableRowGroupInfo& info ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetMemoryFootprint( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  lineEnding (crlf, lf, cr, mixed, none), bom (true, false), delimiter, and for the date columns that were inferred
     *  dateFormat.<column name> with the format text
     */
    virtual void GetKeyValueMetaData( CValueList& keyValues ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDataTableSchemaPtr m_schema;
    CValueList m_keyValues;
};

/*-------------------------------------------------------------------------*/

/**
 *  Records of a file as the table sees them: the title lines are skipped, blank lines are skipped (except in a table of
 *  one column, where an empty line is a row with an empty field) and lines that match skipLinesMatching are dropped.
 */
class CCsvRecordReader
{
    public:

    CCsvRecordReader( CIOAccess& source                  ,
                      const IOAccessPtr& sourceHolder    ,
                      CDataTableBufferPool& pool         ,
                      const SCsvReadOptions& options     ,
                      MT::CMutex* sharedFileLock         );

    /** Positions at the start of the file and skips the title lines */
    TDataTableStatus Start( CString& error );

    /** columnCountHint is the number of columns when it is known, else 0 */
    TDataTableStatus Next( UInt32 columnCountHint, bool& gotRecord, CString& error );

    UInt32 GetFieldCount( void ) const { return m_synthetic ? 1 : m_parser.GetFieldCount(); }
    const char* GetFieldData( UInt32 index ) const { return m_synthetic ? "" : m_parser.GetFieldData( index ); }
    UInt32 GetFieldLength( UInt32 index ) const { return m_synthetic ? 0 : m_parser.GetFieldLength( index ); }
    bool IsFieldQuoted( UInt32 index ) const { return m_synthetic ? false : m_parser.IsFieldQuoted( index ); }

    CCsvRecordParser& GetParser( void ) { return m_parser; }
    const CCsvRecordParser& GetParser( void ) const { return m_parser; }

    private:

    bool MatchesSkipPattern( void ) const;

    private:

    CIOAccess& m_source;
    bool m_shared;
    const SCsvReadOptions& m_options;
    CCsvRecordParser m_parser;
    bool m_synthetic;
};

/*-------------------------------------------------------------------------*/

/**
 *  Reads one CSV file as batches. The file is one row group without statistics. The pool is the one of the read request;
 *  memory is bounded by the read block, the longest record and one batch, whatever the size of the file.
 */
class CCsvTableReader : public CITableReader
{
    public:

    /** The metadata holds the schema the reader converts to; the reader keeps no reference to the source beyond the pointer */
    CCsvTableReader( const IOAccessPtr& source               ,
                     const CITableFileMetaDataPtr& metaData  ,
                     const SCsvReadOptions& options          );

    virtual ~CCsvTableReader() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus OpenRowGroup( UInt32 rowGroup, const CDataTableReadRequest& request ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus ReadNextBatch( CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::vector< SCsvColumnConversion > TConversionVector;

    TDataTableStatus Fail( TDataTableStatus status, const CString& message );
    TDataTableStatus ReadHeader( void );
    TDataTableStatus AppendRecord( void );

    private:

    IOAccessPtr m_source;
    CITableFileMetaDataPtr m_metaData;
    SCsvReadOptions m_options;
    TConversionVector m_conversions;            /**< per file schema column */
    CDataTableSchemaPtr m_fileSchema;

    // set by OpenRowGroup
    CDataTableReadRequest m_request;
    CDataTableBufferPool* m_pool;
    CCsvRecordReader* m_records;
    CCsvColumnConverter* m_converter;
    CDataTableSchemaPtr m_projectedSchema;
    CDataTableBatchBuilder* m_builder;
    CDataTableSchema::TColumnIndexVector m_projection;
    UInt32 m_batchRowCount;
    UInt64 m_rowsRead;
    UInt64 m_valueErrors;
    bool m_open;
    bool m_ended;
    bool m_failed;
    TDataTableStatus m_failedStatus;
    CString m_lastError;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCSVTABLEREADER_H ? */
