/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CGRIDCSVEXPORTER_H
#define GUCEF_GUI_CGRIDCSVEXPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

#ifndef GUCEF_GUI_CGRIDEXPORTSOURCE_H
#include "gucefGUI_CGridExportSource.h"
#define GUCEF_GUI_CGRIDEXPORTSOURCE_H
#endif /* GUCEF_GUI_CGRIDEXPORTSOURCE_H ? */

#ifndef GUCEF_GUI_CGRIDCSVOPTIONS_H
#include "gucefGUI_CGridCsvOptions.h"
#define GUCEF_GUI_CGRIDCSVOPTIONS_H
#endif /* GUCEF_GUI_CGRIDCSVOPTIONS_H ? */

#ifndef GUCEF_GUI_CGRIDCSVEXPORTREPORT_H
#include "gucefGUI_CGridCsvExportReport.h"
#define GUCEF_GUI_CGRIDCSVEXPORTREPORT_H
#endif /* GUCEF_GUI_CGRIDCSVEXPORTREPORT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes what a CGridExportSource reads as CSV, streaming: rows are taken a batch at a time (batchRowCount of the
 *  options), turned into a batch of text columns and written, so the memory an export takes is one batch whatever
 *  the number of rows; the table is never copied. The writing is the CSV table codec of gucefCORE (RFC 4180 quoting,
 *  delimiter, line ending, byte order mark), with the titles as the first row and an empty cell as an empty field.
 *
 *  Begin(), Step() until STEP_DONE and Finish() run an export piece by piece on the calling thread, as the export
 *  flow of a grid does between frames. Export() does all of it at once on the calling thread. ExportAsync() runs the
 *  same steps in a task on a thread of a pool and gives a future; the source must then be safe to read from that thread
 *  (a detached source of a model that allows it), and both the source and the output are held by the task until it
 *  ended. Abort() stops; the output then holds a part and is the caller's to remove.
 */
class GUCEF_GUI_PUBLIC_CPP CGridCsvExporter
{
    public:

    enum EStepResult
    {
        STEP_MORE = 0   ,   /**< rows are left */
        STEP_DONE       ,   /**< every row is written; call Finish() */
        STEP_FAILED         /**< writing failed (GetLastError()); the export has ended */
    };

    CGridCsvExporter( void );

    ~CGridCsvExporter();

    /** Starts an export of the source to the output. False, with the reason, for options that are not valid, a source without columns or an output that cannot be written */
    bool Begin( CGridExportSource& source, const CORE::IOAccessPtr& output, const CGridCsvOptions& options, CString& error );

    /** Writes up to maxRows rows (at most a batch); the header goes with the first step */
    EStepResult Step( UInt32 maxRows );

    /** Writes what is left of the file; the report says what was written. False when the export failed */
    bool Finish( CGridCsvExportReport& report );

    /** Stops the export; the output keeps what was written */
    void Abort( void );

    /** Begin() was done and neither Finish() nor Abort() yet */
    bool IsActive( void ) const;

    /** The rows the export writes: the rows of the source when it began */
    UInt64 GetRowCount( void ) const;

    UInt64 GetRowsWritten( void ) const;

    UInt32 GetColumnCount( void ) const;

    const CString& GetLastError( void ) const;

    /** The whole export on the calling thread */
    static bool Export( CGridExportSource& source, const CORE::IOAccessPtr& output, const CGridCsvOptions& options, CGridCsvExportReport& report );

    /** The whole export in a task on a thread of the pool of the options (threadPoolName; the default pool when empty) */
    static CORE::CFutureResult ExportAsync( const TGridExportSourcePtr& source, const CORE::IOAccessPtr& output, const CGridCsvOptions& options );

    /** The report of an asynchronous export once its future is ready; false while it is not */
    static bool GetAsyncResult( const CORE::CFutureResult& future, CGridCsvExportReport& report );

    private:

    bool WriteBatch( UInt32 rows );
    void Fail( const CString& error );
    void Release( void );

    CGridCsvExporter( const CGridCsvExporter& src );
    CGridCsvExporter& operator=( const CGridCsvExporter& src );

    private:

    CGridExportSource* m_source;
    CORE::IOAccessPtr m_output;
    CGridCsvOptions m_options;
    CORE::CDataTableSchemaPtr m_schema;
    CORE::CITableWriterPtr m_writer;
    UInt64 m_rowCount;
    UInt64 m_nextRow;
    UInt32 m_columnCount;
    bool m_headerPending;
    bool m_active;
    bool m_failed;
    CString m_error;
    GUCEF::vector< CString > m_cells;      /**< the texts of one row, kept between rows */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCSVEXPORTER_H ? */
