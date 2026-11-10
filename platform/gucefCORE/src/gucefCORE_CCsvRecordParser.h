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

#ifndef GUCEF_CORE_CCSVRECORDPARSER_H
#define GUCEF_CORE_CCSVRECORDPARSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

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
 *  Cuts a CSV byte stream into records of fields (RFC 4180 with the options of SCsvReadOptions).
 *
 *  The stream is read in blocks of 64 KiB taken from the buffer pool; the fields of a record are unquoted into one
 *  record buffer and handed out as byte ranges, so nothing is allocated per field. Memory is bounded by the block, the
 *  longest record (maxRecordBytes) and the number of fields (maxColumns), whatever the size of the file.
 *
 *  Records end at CRLF, LF or a lone CR. Quoted fields may hold the delimiter, doubled quotes and line breaks, which are
 *  kept as they are. A quote in an unquoted field is literal. A leading UTF-8 byte order mark is removed; UTF-16 is
 *  refused. A NUL byte, a quoted field that is not closed, and text between a closing quote and the next delimiter are
 *  CORRUPT_DATA. One thread at a time.
 */
class CCsvRecordParser
{
    public:

    static const UInt32 BlockSize = 65536;

    /** The holder, when given, keeps the source alive for the life of the parser */
    CCsvRecordParser( CIOAccess& source                  ,
                      const IOAccessPtr& sourceHolder    ,
                      CDataTableBufferPool& pool         ,
                      const SCsvReadOptions& options     ,
                      MT::CMutex* sharedFileLock = GUCEF_NULL );

    ~CCsvRecordParser();

    /** Back to the start of the file: needs Setpos() of the source, UNSUPPORTED when it cannot */
    TDataTableStatus Rewind( CString& error );

    /** Skips whole physical lines, quotes not considered, as the title lines in front of a header */
    TDataTableStatus SkipLines( UInt32 lineCount, CString& error );

    /**
     *  Reads the next record. gotRecord is false at the end of the file. A record without any byte (an empty line) is a
     *  record with IsBlank() set and no fields; the caller decides what it is.
     */
    TDataTableStatus NextRecord( bool& gotRecord, CString& error );

    bool IsBlank( void ) const { return m_blank; }
    UInt32 GetFieldCount( void ) const { return (UInt32) m_fields.size(); }
    const char* GetFieldData( UInt32 index ) const { return m_data + m_fields[ index ].offset; }
    UInt32 GetFieldLength( UInt32 index ) const { return m_fields[ index ].length; }
    bool IsFieldQuoted( UInt32 index ) const { return m_fields[ index ].quoted; }

    /** Records returned so far, blank ones included */
    UInt64 GetRecordNumber( void ) const { return m_recordNumber; }

    /** The byte offset in the file where the last record started */
    UInt64 GetRecordOffset( void ) const { return m_recordOffset; }

    bool HadByteOrderMark( void ) const { return m_hadBom; }

    UInt64 GetCrLfCount( void ) const { return m_crlfCount; }
    UInt64 GetLfCount( void ) const { return m_lfCount; }
    UInt64 GetCrCount( void ) const { return m_crCount; }

    /** "crlf", "lf", "cr", "mixed" when more than one kind was seen, "none" when no record ended with a line break */
    CString GetLineEnding( void ) const;

    /** The record that is being read, or failed: "record 12 at byte 345" */
    CString Describe( void ) const;

    /** The record that was returned last: "record 12 at byte 345" */
    CString DescribeLast( void ) const;

    /** The bytes of the file that were parsed so far, the byte order mark included */
    UInt64 GetBytesConsumed( void ) const { return m_consumedBytes; }

    private:

    CCsvRecordParser( const CCsvRecordParser& src );                /**< not implemented */
    CCsvRecordParser& operator=( const CCsvRecordParser& src );     /**< not implemented */

    struct SField
    {
        UInt32 offset;
        UInt32 length;
        bool quoted;
    };
    typedef GUCEF::vector< SField > TFieldVector;

    enum EState
    {
        STATE_FIELD_START ,
        STATE_UNQUOTED    ,
        STATE_QUOTED      ,
        STATE_QUOTE_IN_QUOTED ,
        STATE_AFTER_QUOTED
    };

    TDataTableStatus Refill( CString& error );
    TDataTableStatus AppendToRecord( const char* bytes, UInt32 count, CString& error );
    TDataTableStatus EndField( CString& error );
    TDataTableStatus ConsumeLineBreak( char first, CString& error );
    void ResetRecord( void );
    void ResetStream( void );
    TDataTableStatus Fail( TDataTableStatus status, const CString& message, CString& error ) const;

    private:

    CIOAccess* m_source;
    IOAccessPtr m_sourceHolder;
    CDataTableBufferPool& m_pool;
    const SCsvReadOptions& m_options;
    MT::CMutex* m_sharedFileLock;

    CDataTableBufferPtr m_block;
    UInt32 m_blockPos;
    UInt32 m_blockEnd;
    bool m_eof;
    bool m_started;
    UInt64 m_filePosition;              /**< where the next block is read from */
    UInt64 m_consumedBytes;             /**< file offset of the next byte to parse */
    bool m_hadBom;

    char* m_data;
    UInt32 m_dataSize;
    UInt32 m_dataCapacity;
    TFieldVector m_fields;
    EState m_state;
    UInt32 m_fieldStart;
    bool m_fieldQuoted;
    bool m_hasContent;
    bool m_blank;
    UInt64 m_recordNumber;
    UInt64 m_recordOffset;

    UInt64 m_crlfCount;
    UInt64 m_lfCount;
    UInt64 m_crCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCSVRECORDPARSER_H ? */
