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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CCsvRecordParser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 InitialRecordCapacity = 4096;

/*-------------------------------------------------------------------------*/

CCsvRecordParser::CCsvRecordParser( CIOAccess& source              ,
                                    const IOAccessPtr& sourceHolder ,
                                    CDataTableBufferPool& pool     ,
                                    const SCsvReadOptions& options ,
                                    MT::CMutex* sharedFileLock     )
    : m_source( &source )
    , m_sourceHolder( sourceHolder )
    , m_pool( pool )
    , m_options( options )
    , m_sharedFileLock( sharedFileLock )
    , m_block()
    , m_blockPos( 0 )
    , m_blockEnd( 0 )
    , m_eof( false )
    , m_started( false )
    , m_filePosition( 0 )
    , m_consumedBytes( 0 )
    , m_hadBom( false )
    , m_data( GUCEF_NULL )
    , m_dataSize( 0 )
    , m_dataCapacity( 0 )
    , m_fields()
    , m_state( STATE_FIELD_START )
    , m_fieldStart( 0 )
    , m_fieldQuoted( false )
    , m_hasContent( false )
    , m_blank( false )
    , m_recordNumber( 0 )
    , m_recordOffset( 0 )
    , m_crlfCount( 0 )
    , m_lfCount( 0 )
    , m_crCount( 0 )
{GUCEF_TRACE;

    m_pool.Acquire( BlockSize, m_block );
    m_data = GUCEF_NEW char[ InitialRecordCapacity ];
    m_dataCapacity = InitialRecordCapacity;
    ResetStream();
}

/*-------------------------------------------------------------------------*/

CCsvRecordParser::~CCsvRecordParser()
{GUCEF_TRACE;

    delete[] m_data;
    m_data = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CCsvRecordParser::ResetRecord( void )
{GUCEF_TRACE;

    m_fields.clear();
    m_dataSize = 0;
    m_state = STATE_FIELD_START;
    m_fieldStart = 0;
    m_fieldQuoted = false;
    m_hasContent = false;
    m_blank = false;
}

/*-------------------------------------------------------------------------*/

void
CCsvRecordParser::ResetStream( void )
{GUCEF_TRACE;

    m_blockPos = 0;
    m_blockEnd = 0;
    m_eof = false;
    m_started = false;
    m_filePosition = 0;
    m_consumedBytes = 0;
    m_hadBom = false;
    m_recordNumber = 0;
    m_recordOffset = 0;
    m_crlfCount = 0;
    m_lfCount = 0;
    m_crCount = 0;
    ResetRecord();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordParser::Fail( TDataTableStatus status, const CString& message, CString& error ) const
{GUCEF_TRACE;

    error = message + " (" + Describe() + ")";
    return status;
}

/*-------------------------------------------------------------------------*/

CString
CCsvRecordParser::Describe( void ) const
{GUCEF_TRACE;

    return "record " + UInt64ToString( m_recordNumber + 1 ) + " at byte " + UInt64ToString( m_recordOffset );
}

/*-------------------------------------------------------------------------*/

CString
CCsvRecordParser::DescribeLast( void ) const
{GUCEF_TRACE;

    return "record " + UInt64ToString( m_recordNumber ) + " at byte " + UInt64ToString( m_recordOffset );
}

/*-------------------------------------------------------------------------*/

CString
CCsvRecordParser::GetLineEnding( void ) const
{GUCEF_TRACE;

    const int kinds = ( m_crlfCount > 0 ? 1 : 0 ) + ( m_lfCount > 0 ? 1 : 0 ) + ( m_crCount > 0 ? 1 : 0 );
    if ( 0 == kinds ) return "none";
    if ( kinds > 1 ) return "mixed";
    if ( m_crlfCount > 0 ) return "crlf";
    return m_lfCount > 0 ? "lf" : "cr";
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordParser::Rewind( CString& error )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_source || 0 != m_source->Setpos( 0 ) )
    {
        error = "The source cannot be repositioned to its start";
        return DATATABLE_STATUS_UNSUPPORTED;
    }
    ResetStream();
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordParser::Refill( CString& error )
{GUCEF_TRACE;

    if ( m_eof )
        return DATATABLE_STATUS_OK;
    if ( m_block.IsNULL() || GUCEF_NULL == m_source )
    {
        error = "No buffer or no source to read from";
        return DATATABLE_STATUS_OUT_OF_MEMORY;
    }

    char* block = static_cast< char* >( m_block->GetData() );
    UInt32 filled = 0;
    {
        // The shared input is positioned for every read: other readers move it
        MT::CObjectScopeLock lock( m_sharedFileLock );
        if ( GUCEF_NULL != m_sharedFileLock && 0 != m_source->Setpos( m_filePosition ) )
        {
            error = "The source could not be positioned";
            return DATATABLE_STATUS_IO_ERROR;
        }

        // The first block is filled until the byte order mark can be told
        const UInt32 wanted = m_started ? 1 : 4;
        while ( filled < wanted )
        {
            const UInt32 got = m_source->Read( block + filled, 1, BlockSize - filled );
            if ( 0 == got )
            {
                m_eof = true;
                break;
            }
            filled += got;
        }
    }
    m_blockPos = 0;
    m_blockEnd = filled;
    m_filePosition += filled;

    if ( !m_started )
    {
        m_started = true;
        const unsigned char* b = reinterpret_cast< const unsigned char* >( block );
        if ( filled >= 3 && 0xEF == b[ 0 ] && 0xBB == b[ 1 ] && 0xBF == b[ 2 ] )
        {
            m_blockPos = 3;
            m_consumedBytes = 3;
            m_hadBom = true;
        }
        else
        if ( filled >= 2 && ( ( 0xFF == b[ 0 ] && 0xFE == b[ 1 ] ) || ( 0xFE == b[ 0 ] && 0xFF == b[ 1 ] ) ) )
        {
            return Fail( DATATABLE_STATUS_UNSUPPORTED, "The file is UTF-16, only UTF-8 is supported", error );
        }
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordParser::AppendToRecord( const char* bytes, UInt32 count, CString& error )
{GUCEF_TRACE;

    if ( 0 == count )
        return DATATABLE_STATUS_OK;
    if ( count > m_options.maxRecordBytes || m_dataSize > m_options.maxRecordBytes - count )
        return Fail( DATATABLE_STATUS_LIMIT_EXCEEDED, "A record is larger than " + UInt32ToString( m_options.maxRecordBytes ) + " bytes", error );

    const UInt32 needed = m_dataSize + count;
    if ( needed > m_dataCapacity )
    {
        UInt64 newCapacity = (UInt64) m_dataCapacity * 2;
        if ( newCapacity < needed ) newCapacity = needed;
        if ( newCapacity > m_options.maxRecordBytes ) newCapacity = m_options.maxRecordBytes;
        if ( newCapacity < needed ) newCapacity = needed;
        char* grown = GUCEF_NEW char[ (size_t) newCapacity ];
        if ( GUCEF_NULL == grown )
            return Fail( DATATABLE_STATUS_OUT_OF_MEMORY, "A record could not be allocated", error );
        memcpy( grown, m_data, m_dataSize );
        delete[] m_data;
        m_data = grown;
        m_dataCapacity = (UInt32) newCapacity;
    }
    memcpy( m_data + m_dataSize, bytes, count );
    m_dataSize += count;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordParser::EndField( CString& error )
{GUCEF_TRACE;

    UInt32 length = m_dataSize - m_fieldStart;
    if ( !m_fieldQuoted && m_options.trimWhitespace )
    {
        while ( length > 0 && ( ' ' == m_data[ m_fieldStart + length - 1 ] || '\t' == m_data[ m_fieldStart + length - 1 ] ) )
            --length;
    }
    if ( length > m_options.maxFieldBytes )
        return Fail( DATATABLE_STATUS_LIMIT_EXCEEDED, "A field is larger than " + UInt32ToString( m_options.maxFieldBytes ) + " bytes", error );
    if ( m_fields.size() >= m_options.maxColumns )
        return Fail( DATATABLE_STATUS_LIMIT_EXCEEDED, "A record has more than " + UInt32ToString( m_options.maxColumns ) + " fields", error );

    SField field;
    field.offset = m_fieldStart;
    field.length = length;
    field.quoted = m_fieldQuoted;
    m_fields.push_back( field );
    m_fieldStart = m_dataSize;
    m_fieldQuoted = false;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordParser::ConsumeLineBreak( char first, CString& error )
{GUCEF_TRACE;

    // the line break is at m_blockPos - 1 and was consumed already: a CR is a CR LF when an LF follows, which may be in the next block
    if ( '\n' == first )
    {
        ++m_lfCount;
        return DATATABLE_STATUS_OK;
    }

    if ( m_blockPos >= m_blockEnd )
    {
        TDataTableStatus status = Refill( error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
    }
    if ( m_blockPos < m_blockEnd && '\n' == static_cast< const char* >( m_block->GetData() )[ m_blockPos ] )
    {
        ++m_blockPos;
        ++m_consumedBytes;
        ++m_crlfCount;
    }
    else
    {
        ++m_crCount;
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordParser::SkipLines( UInt32 lineCount, CString& error )
{GUCEF_TRACE;

    while ( lineCount > 0 )
    {
        if ( m_blockPos >= m_blockEnd )
        {
            TDataTableStatus status = Refill( error );
            if ( DATATABLE_STATUS_OK != status )
                return status;
            if ( m_blockPos >= m_blockEnd )
                return DATATABLE_STATUS_OK;
        }

        const char* b = static_cast< const char* >( m_block->GetData() );
        const char c = b[ m_blockPos ];
        ++m_blockPos;
        ++m_consumedBytes;
        if ( '\n' == c || '\r' == c )
        {
            TDataTableStatus status = ConsumeLineBreak( c, error );
            if ( DATATABLE_STATUS_OK != status )
                return status;
            --lineCount;
        }
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordParser::NextRecord( bool& gotRecord, CString& error )
{GUCEF_TRACE;

    gotRecord = false;
    ResetRecord();

    const char delimiter = m_options.delimiter;
    const char quote = m_options.quote;
    const bool trim = m_options.trimWhitespace;
    bool recordStarted = false;
    TDataTableStatus status = DATATABLE_STATUS_OK;

    for ( ;; )
    {
        if ( m_blockPos >= m_blockEnd )
        {
            status = Refill( error );
            if ( DATATABLE_STATUS_OK != status )
                return status;

            if ( m_blockPos >= m_blockEnd )
            {
                // the end of the file
                if ( STATE_QUOTED == m_state )
                    return Fail( DATATABLE_STATUS_CORRUPT_DATA, "The file ends inside a quoted field: the quote is not closed", error );
                if ( STATE_FIELD_START == m_state && !m_hasContent && m_fields.empty() )
                    return DATATABLE_STATUS_OK;

                status = EndField( error );
                if ( DATATABLE_STATUS_OK != status )
                    return status;
                ++m_recordNumber;
                gotRecord = true;
                return DATATABLE_STATUS_OK;
            }
        }

        const char* b = static_cast< const char* >( m_block->GetData() );

        if ( !recordStarted )
        {
            recordStarted = true;
            m_recordOffset = m_consumedBytes;
        }

        char c = b[ m_blockPos ];
        bool endOfRecord = false;

        switch ( m_state )
        {
            case STATE_FIELD_START:
            {
                if ( '\0' == c )
                    return Fail( DATATABLE_STATUS_CORRUPT_DATA, "A NUL byte in the data", error );
                if ( trim && c != delimiter && ( ' ' == c || '\t' == c ) )
                {
                    m_hasContent = true;
                    ++m_blockPos;
                    ++m_consumedBytes;
                }
                else
                if ( '\0' != quote && c == quote )
                {
                    m_state = STATE_QUOTED;
                    m_fieldQuoted = true;
                    m_fieldStart = m_dataSize;
                    m_hasContent = true;
                    ++m_blockPos;
                    ++m_consumedBytes;
                }
                else
                if ( c == delimiter )
                {
                    m_hasContent = true;
                    status = EndField( error );
                    if ( DATATABLE_STATUS_OK != status )
                        return status;
                    ++m_blockPos;
                    ++m_consumedBytes;
                }
                else
                if ( '\r' == c || '\n' == c )
                {
                    endOfRecord = true;
                }
                else
                {
                    m_state = STATE_UNQUOTED;
                    m_fieldStart = m_dataSize;
                    m_hasContent = true;
                }
                break;
            }
            case STATE_UNQUOTED:
            {
                const UInt32 start = m_blockPos;
                UInt32 pos = start;
                while ( pos < m_blockEnd )
                {
                    c = b[ pos ];
                    if ( c == delimiter || '\r' == c || '\n' == c || '\0' == c )
                        break;
                    ++pos;
                }
                status = AppendToRecord( b + start, pos - start, error );
                if ( DATATABLE_STATUS_OK != status )
                    return status;
                m_consumedBytes += ( pos - start );
                m_blockPos = pos;
                if ( pos < m_blockEnd )
                {
                    if ( '\0' == c )
                        return Fail( DATATABLE_STATUS_CORRUPT_DATA, "A NUL byte in the data", error );
                    if ( c == delimiter )
                    {
                        status = EndField( error );
                        if ( DATATABLE_STATUS_OK != status )
                            return status;
                        m_state = STATE_FIELD_START;
                        ++m_blockPos;
                        ++m_consumedBytes;
                    }
                    else
                    {
                        endOfRecord = true;
                    }
                }
                break;
            }
            case STATE_QUOTED:
            {
                const UInt32 start = m_blockPos;
                UInt32 pos = start;
                while ( pos < m_blockEnd )
                {
                    c = b[ pos ];
                    if ( c == quote || '\0' == c )
                        break;
                    ++pos;
                }
                status = AppendToRecord( b + start, pos - start, error );
                if ( DATATABLE_STATUS_OK != status )
                    return status;
                m_consumedBytes += ( pos - start );
                m_blockPos = pos;
                if ( pos < m_blockEnd )
                {
                    if ( '\0' == c )
                        return Fail( DATATABLE_STATUS_CORRUPT_DATA, "A NUL byte in the data", error );
                    m_state = STATE_QUOTE_IN_QUOTED;
                    ++m_blockPos;
                    ++m_consumedBytes;
                }
                break;
            }
            case STATE_QUOTE_IN_QUOTED:
            {
                if ( c == quote )
                {
                    status = AppendToRecord( &c, 1, error );
                    if ( DATATABLE_STATUS_OK != status )
                        return status;
                    m_state = STATE_QUOTED;
                    ++m_blockPos;
                    ++m_consumedBytes;
                }
                else
                if ( c == delimiter )
                {
                    status = EndField( error );
                    if ( DATATABLE_STATUS_OK != status )
                        return status;
                    m_state = STATE_FIELD_START;
                    ++m_blockPos;
                    ++m_consumedBytes;
                }
                else
                if ( '\r' == c || '\n' == c )
                {
                    endOfRecord = true;
                }
                else
                if ( trim && c != delimiter && ( ' ' == c || '\t' == c ) )
                {
                    m_state = STATE_AFTER_QUOTED;
                    ++m_blockPos;
                    ++m_consumedBytes;
                }
                else
                if ( '\0' == c )
                {
                    return Fail( DATATABLE_STATUS_CORRUPT_DATA, "A NUL byte in the data", error );
                }
                else
                {
                    return Fail( DATATABLE_STATUS_CORRUPT_DATA, "A quote closes a field and is followed by text instead of a delimiter or the end of the record (a stray quote)", error );
                }
                break;
            }
            case STATE_AFTER_QUOTED:
            {
                if ( c != delimiter && ( ' ' == c || '\t' == c ) )
                {
                    ++m_blockPos;
                    ++m_consumedBytes;
                }
                else
                if ( c == delimiter )
                {
                    status = EndField( error );
                    if ( DATATABLE_STATUS_OK != status )
                        return status;
                    m_state = STATE_FIELD_START;
                    ++m_blockPos;
                    ++m_consumedBytes;
                }
                else
                if ( '\r' == c || '\n' == c )
                {
                    endOfRecord = true;
                }
                else
                if ( '\0' == c )
                {
                    return Fail( DATATABLE_STATUS_CORRUPT_DATA, "A NUL byte in the data", error );
                }
                else
                {
                    return Fail( DATATABLE_STATUS_CORRUPT_DATA, "A quote closes a field and is followed by text instead of a delimiter or the end of the record (a stray quote)", error );
                }
                break;
            }
        }

        if ( endOfRecord )
        {
            if ( STATE_FIELD_START == m_state && !m_hasContent && m_fields.empty() )
            {
                m_blank = true;
            }
            else
            {
                status = EndField( error );
                if ( DATATABLE_STATUS_OK != status )
                    return status;
            }

            c = b[ m_blockPos ];
            ++m_blockPos;
            ++m_consumedBytes;
            status = ConsumeLineBreak( c, error );
            if ( DATATABLE_STATUS_OK != status )
                return status;
            ++m_recordNumber;
            gotRecord = true;
            return DATATABLE_STATUS_OK;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
