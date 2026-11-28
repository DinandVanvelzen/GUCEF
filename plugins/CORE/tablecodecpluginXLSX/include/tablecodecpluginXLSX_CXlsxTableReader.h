/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXTABLEREADER_H
#define TABLECODECPLUGIN_XLSX_CXLSXTABLEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEREADER_H
#include "gucefCORE_CITableReader.h"
#define GUCEF_CORE_CITABLEREADER_H
#endif /* GUCEF_CORE_CITABLEREADER_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H
#include "tablecodecpluginXLSX_CXlsxFileMetaData.h"
#define TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H
#include "tablecodecpluginXLSX_CXlsxReadOptions.h"
#define TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads the worksheet of an .xlsx file as batches. The file is one row group without statistics.
 *
 *  The XML reader of the data store codecs is a push parser: it reads a part from its start to its end and reports what it
 *  meets, so a reader cannot be paused between two batches. OpenRowGroup() therefore reads the sheet once and builds the
 *  batches as the rows come, straight from the cells into the columns of the pooled buffers (a row is never held as an object,
 *  and the text of a cell is copied once, into the buffer of its column); ReadNextBatch() hands them out one by one. What the
 *  sheet can hold in memory is bounded by the limits of the options (maxCells, maxRows, maxCellBytes).
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxTableReader : public CORE::CITableReader
{
    public:

    /** The metadata is the one of the codec, made by ReadMetaData() with the same options */
    CXlsxTableReader( const CORE::IOAccessPtr& source, const CORE::CITableFileMetaDataPtr& metaData, const SXlsxReadOptions& options );

    virtual ~CXlsxTableReader() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus OpenRowGroup( UInt32 rowGroup, const CORE::CDataTableReadRequest& request ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus ReadNextBatch( CORE::CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message );

    CORE::IOAccessPtr m_source;
    CORE::CITableFileMetaDataPtr m_metaData;
    SXlsxReadOptions m_options;
    GUCEF::vector< CORE::CDataTableBatchPtr > m_batches;
    size_t m_nextBatch;
    bool m_open;
    bool m_failed;
    CORE::TDataTableStatus m_failedStatus;
    CORE::CString m_lastError;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_XLSX_CXLSXTABLEREADER_H ? */
