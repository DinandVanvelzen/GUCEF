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

#ifndef GUCEF_CORE_CCSVTABLEWRITER_H
#define GUCEF_CORE_CCSVTABLEWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CCSVTABLEOPTIONS_H
#include "gucefCORE_CCsvTableOptions.h"
#define GUCEF_CORE_CCSVTABLEOPTIONS_H
#endif /* GUCEF_CORE_CCSVTABLEOPTIONS_H ? */

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

/**
 *  Writes batches as CSV (section 11.2 of the design): a header row, fields quoted when they hold the delimiter, a quote,
 *  CR or LF or start or end with a blank, decimals with exactly their scale, dates as yyyy-mm-dd, timestamps as ISO 8601
 *  with Z when UTC adjusted, booleans as true / false, binary as base64, null as the null token (empty by default).
 *  A text value that is empty or equals the null token is written quoted, so it reads back as the text and not as null.
 *  The output is buffered in blocks of 64 KiB. One thread at a time.
 */
class CCsvTableWriter : public CITableWriter
{
    public:

    CCsvTableWriter( const IOAccessPtr& destination        ,
                     const CDataTableSchemaPtr& schema     ,
                     const SCsvWriteOptions& options       );

    virtual ~CCsvTableWriter() GUCEF_VIRTUAL_OVERRIDE;

    /** Writes the byte order mark and the header row */
    TDataTableStatus Start( void );

    virtual TDataTableStatus WriteBatch( const CDataTableBatch& batch ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus FlushRowGroup( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus Finish( CDataTableFileSummary& summary ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Abort( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    static const UInt32 FlushThreshold = 65536;

    TDataTableStatus Fail( TDataTableStatus status, const CString& message );
    TDataTableStatus Flush( void );
    void AppendBytes( const char* bytes, UInt32 count );
    void AppendField( const char* data, UInt32 length, bool quoteIfEmpty );
    bool CheckBatch( const CDataTableBatch& batch );

    private:

    IOAccessPtr m_destination;
    CDataTableSchemaPtr m_schema;
    SCsvWriteOptions m_options;
    GUCEF::vector< char > m_out;
    GUCEF::vector< UInt64 > m_nullCounts;
    UInt64 m_rowCount;
    UInt64 m_byteCount;
    bool m_started;
    bool m_finished;
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

#endif /* GUCEF_CORE_CCSVTABLEWRITER_H ? */
