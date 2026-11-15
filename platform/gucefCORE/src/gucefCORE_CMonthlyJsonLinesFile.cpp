/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CMonthlyJsonLinesFile.h"

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSYSTEMUTCCLOCK_H
#include "gucefCORE_CSystemUtcClock.h"
#define GUCEF_CORE_CSYSTEMUTCCLOCK_H
#endif /* GUCEF_CORE_CSYSTEMUTCCLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

const char* const CMonthlyJsonLinesFile::FileExtension = "jsonl";

/*-------------------------------------------------------------------------*/

namespace {

/** The "yyyy-mm" of UTC milliseconds since the Unix epoch, from the civil calendar of the day number */
CString
YearMonthOfUtcMs( Int64 utcMs )
{GUCEF_TRACE;

    Int64 days = utcMs / 86400000;
    if ( utcMs < 0 && 0 != ( utcMs % 86400000 ) )
        --days;

    // days since 1970-01-01 to a civil date (proleptic Gregorian), in eras of 400 years
    days += 719468;
    const Int64 era = ( days >= 0 ? days : days - 146096 ) / 146097;
    const Int64 dayOfEra = days - era * 146097;
    const Int64 yearOfEra = ( dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096 ) / 365;
    const Int64 dayOfYear = dayOfEra - ( 365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100 );
    const Int64 monthIndex = ( 5 * dayOfYear + 2 ) / 153;
    const Int64 month = monthIndex < 10 ? monthIndex + 3 : monthIndex - 9;
    const Int64 year = yearOfEra + era * 400 + ( month <= 2 ? 1 : 0 );

    char text[ 16 ];
    text[ 0 ] = (char) ( '0' + ( year / 1000 ) % 10 );
    text[ 1 ] = (char) ( '0' + ( year / 100 ) % 10 );
    text[ 2 ] = (char) ( '0' + ( year / 10 ) % 10 );
    text[ 3 ] = (char) ( '0' + year % 10 );
    text[ 4 ] = '-';
    text[ 5 ] = (char) ( '0' + month / 10 );
    text[ 6 ] = (char) ( '0' + month % 10 );
    text[ 7 ] = '\0';
    return CString( text );
}

/*-------------------------------------------------------------------------*/

CString
MonthFileName( const CString& month )
{GUCEF_TRACE;

    return month + "." + CMonthlyJsonLinesFile::FileExtension;
}

}

/*-------------------------------------------------------------------------*/

CMonthlyJsonLinesFile::CMonthlyJsonLinesFile( void )
    : m_clock( GUCEF_NULL )
    , m_directory()
    , m_newestMonth()
    , m_lastLineAtOpen()
    , m_repairedBytesAtOpen( 0 )
    , m_keepMonths( 0 )
    , m_failedAppends( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMonthlyJsonLinesFile::~CMonthlyJsonLinesFile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMonthlyJsonLinesFile::SetUtcClock( CIUtcClock* clock )
{GUCEF_TRACE;

    m_clock = clock;
}

/*-------------------------------------------------------------------------*/

CString
CMonthlyJsonLinesFile::MonthOfFileName( const CString& fileName )
{GUCEF_TRACE;

    // "yyyy-mm.jsonl": 13 characters with digits where the digits are
    const CString suffix = CString( "." ) + FileExtension;
    if ( 13 != fileName.Length() || fileName.SubstrFromRange( 7, 13 ) != suffix || '-' != fileName[ 4 ] )
        return CString::Empty;
    for ( UInt32 i=0; i<7; ++i )
    {
        if ( 4 != i && ( fileName[ i ] < '0' || fileName[ i ] > '9' ) )
            return CString::Empty;
    }

    // only a real month: a name such as "9999-99" would otherwise be the newest month for good
    const Int32 year = ( fileName[ 0 ] - '0' ) * 1000 + ( fileName[ 1 ] - '0' ) * 100 + ( fileName[ 2 ] - '0' ) * 10 + ( fileName[ 3 ] - '0' );
    const Int32 month = ( fileName[ 5 ] - '0' ) * 10 + ( fileName[ 6 ] - '0' );
    if ( year < 1970 || month < 1 || month > 12 )
        return CString::Empty;
    return fileName.SubstrToIndex( 7, true );
}

/*-------------------------------------------------------------------------*/

CString
CMonthlyJsonLinesFile::MonthOfNow( void ) const
{GUCEF_TRACE;

    Int64 nowMs = 0;
    if ( GUCEF_NULL != m_clock )
    {
        nowMs = m_clock->GetUtcTimeInMs();
    }
    else
    {
        CSystemUtcClock systemClock;
        nowMs = systemClock.GetUtcTimeInMs();
    }

    // the month never goes back: after the clock was set back lines go on into the newest file
    const CString month = YearMonthOfUtcMs( nowMs );
    return month < m_newestMonth ? m_newestMonth : month;
}

/*-------------------------------------------------------------------------*/

void
CMonthlyJsonLinesFile::GetMonthFiles( CString::StringVector& fileNames ) const
{GUCEF_TRACE;

    fileNames.clear();
    if ( m_directory.IsNULLOrEmpty() )
        return;

    CString::StringSet names;
    CFileSystemIterator files;
    if ( files.FindFirst( m_directory ) )
    {
        do
        {
            if ( files.IsAFile() )
            {
                const CString name = files.GetResourceName();
                if ( !MonthOfFileName( name ).IsNULLOrEmpty() )
                    names.insert( name );
            }
        }
        while ( files.FindNext() );
        files.FindClose();
    }

    // the names sort as their months do
    CString::StringSet::const_iterator i = names.begin();
    while ( i != names.end() )
    {
        fileNames.push_back( (*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMonthlyJsonLinesFile::SetDirectory( const CString& directory )
{GUCEF_TRACE;

    if ( directory.IsNULLOrEmpty() )
        return false;

    const bool existed = DirExists( directory );
    if ( !existed && !CreateDirs( directory ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "MonthlyJsonLinesFile: the directory " << directory << " cannot be made" );
        return false;
    }
    if ( !existed && !SetOwnerOnlyAccess( directory ) )
        GUCEF_WARNING_LOG( LOGLEVEL_IMPORTANT, "MonthlyJsonLinesFile: the directory " << directory << " cannot be restricted to its owner" );

    m_directory = directory;
    m_newestMonth.Clear();
    m_lastLineAtOpen.Clear();
    m_repairedBytesAtOpen = 0;
    m_failedAppends = 0;

    CString::StringVector fileNames;
    GetMonthFiles( fileNames );
    if ( !fileNames.empty() )
        m_newestMonth = MonthOfFileName( fileNames.back() );

    RepairNewestFile();
    ApplyRetention();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMonthlyJsonLinesFile::RepairNewestFile( void )
{GUCEF_TRACE;

    if ( m_newestMonth.IsNULLOrEmpty() )
        return;

    const CString path = CombinePath( m_directory, MonthFileName( m_newestMonth ) );
    CDynamicBuffer content;
    if ( !content.LoadContentFromFile( path ) || 0 == content.GetDataSize() )
        return;

    // whole lines end at the last line end; bytes after it belong to a line that was never acknowledged
    const char* bytes = static_cast< const char* >( content.GetConstBufferPtr() );
    const UInt32 size = content.GetDataSize();
    UInt32 wholeSize = size;
    while ( wholeSize > 0 && '\n' != bytes[ wholeSize - 1 ] )
        --wholeSize;

    if ( wholeSize < size )
    {
        if ( TruncateFile( path, wholeSize ) )
        {
            m_repairedBytesAtOpen = size - wholeSize;
            GUCEF_WARNING_LOG( LOGLEVEL_IMPORTANT, "MonthlyJsonLinesFile: " << path << " ended in " << m_repairedBytesAtOpen
                               << " bytes of a line that was not written whole; they were cut off" );
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "MonthlyJsonLinesFile: " << path << " ends in a line that was not written whole and cannot be cut back" );
        }
    }

    if ( wholeSize > 1 )
    {
        const UInt32 lineEnd = wholeSize - 1;
        UInt32 lineStart = lineEnd;
        while ( lineStart > 0 && '\n' != bytes[ lineStart - 1 ] )
            --lineStart;
        m_lastLineAtOpen.Set( bytes + lineStart, lineEnd - lineStart );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CMonthlyJsonLinesFile::GetDirectory( void ) const
{GUCEF_TRACE;

    return m_directory;
}

/*-------------------------------------------------------------------------*/

void
CMonthlyJsonLinesFile::SetKeepMonths( UInt32 keepMonths )
{GUCEF_TRACE;

    m_keepMonths = keepMonths;
}

/*-------------------------------------------------------------------------*/

UInt32
CMonthlyJsonLinesFile::GetKeepMonths( void ) const
{GUCEF_TRACE;

    return m_keepMonths;
}

/*-------------------------------------------------------------------------*/

CString
CMonthlyJsonLinesFile::GetCurrentFilePath( void ) const
{GUCEF_TRACE;

    if ( m_directory.IsNULLOrEmpty() )
        return CString::Empty;
    return CombinePath( m_directory, MonthFileName( MonthOfNow() ) );
}

/*-------------------------------------------------------------------------*/

const CString&
CMonthlyJsonLinesFile::GetLastLineAtOpen( void ) const
{GUCEF_TRACE;

    return m_lastLineAtOpen;
}

/*-------------------------------------------------------------------------*/

UInt32
CMonthlyJsonLinesFile::GetRepairedBytesAtOpen( void ) const
{GUCEF_TRACE;

    return m_repairedBytesAtOpen;
}

/*-------------------------------------------------------------------------*/

UInt32
CMonthlyJsonLinesFile::GetFailedAppendCount( void ) const
{GUCEF_TRACE;

    return m_failedAppends;
}

/*-------------------------------------------------------------------------*/

bool
CMonthlyJsonLinesFile::Append( const CString& line, CString& error )
{GUCEF_TRACE;

    if ( m_directory.IsNULLOrEmpty() )
    {
        error = "no directory was set";
        return false;
    }
    if ( 0 <= line.HasChar( '\n' ) || 0 <= line.HasChar( '\r' ) )
    {
        error = "a line may not hold a line break";
        return false;
    }

    const CString month = MonthOfNow();
    const CString path = CombinePath( m_directory, MonthFileName( month ) );
    const bool newMonth = month != m_newestMonth;

    if ( !FileExists( path ) )
    {
        // owner only from its creation on
        CString problem;
        if ( !CreateOwnerOnlyFile( path, "", 0, problem ) )
        {
            ++m_failedAppends;
            error = "the file " + path + " cannot be made: " + problem;
            return false;
        }
    }

    const CString text = line + "\n";
    const UInt32 size = text.ByteSize() - 1;
    UInt64 sizeBefore = 0;
    bool written = false;
    {
        // the sizes are those of the open file
        CFileAccess file( path, "ab" );
        if ( !file.IsValid() )
        {
            ++m_failedAppends;
            error = "the file " + path + " cannot be opened to append to";
            return false;
        }
        sizeBefore = file.GetSize();

        // the write may only reach the buffer of the stream: whether the bytes are in the file shows at the sync
        written = size == file.Write( text.C_String(), 1, size );
        written = file.Sync() && written;
        written = written && file.GetSize() == sizeBefore + size;
        file.Close();
    }

    if ( !written )
    {
        ++m_failedAppends;

        // back to the size before the append: the next line never follows a fragment
        const bool takenBack = TruncateFile( path, sizeBefore );
        error = "the line could not be written whole to " + path + " (is the disk full or read only?)";
        if ( !takenBack )
            error += "; the fragment could not be taken back, the next SetDirectory() cuts it off";
        return false;
    }

    if ( newMonth )
    {
        m_newestMonth = month;
        ApplyRetention();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMonthlyJsonLinesFile::ReadLines( UInt32 newestMonths, CString::StringVector& lines ) const
{GUCEF_TRACE;

    lines.clear();
    CString::StringVector fileNames;
    GetMonthFiles( fileNames );

    UInt32 first = 0;
    if ( 0 != newestMonths && fileNames.size() > (size_t) newestMonths )
        first = (UInt32) fileNames.size() - newestMonths;

    bool ok = true;
    for ( UInt32 f=first; f<(UInt32)fileNames.size(); ++f )
    {
        const CString path = CombinePath( m_directory, fileNames[ f ] );
        CDynamicBuffer content;
        if ( !content.LoadContentFromFile( path ) )
        {
            if ( 0 != FileSize( path ) )
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "MonthlyJsonLinesFile: the file " << path << " cannot be read" );
                ok = false;
            }
            continue;
        }

        const char* bytes = static_cast< const char* >( content.GetConstBufferPtr() );
        const UInt32 size = content.GetDataSize();
        UInt32 lineStart = 0;
        for ( UInt32 i=0; i<size; ++i )
        {
            if ( '\n' == bytes[ i ] )
            {
                // a line ends here; empty lines are skipped. What follows the last line end is a fragment
                if ( i > lineStart )
                {
                    CString lineText;
                    lineText.Set( bytes + lineStart, i - lineStart );
                    lines.push_back( lineText );
                }
                lineStart = i + 1;
            }
        }
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

UInt32
CMonthlyJsonLinesFile::ApplyRetention( void )
{GUCEF_TRACE;

    if ( 0 == m_keepMonths || m_directory.IsNULLOrEmpty() )
        return 0;

    // the newest keepMonths files stay, by their names: a wall clock set years ahead never makes the trail look expired
    CString::StringVector fileNames;
    GetMonthFiles( fileNames );
    if ( fileNames.size() <= (size_t) m_keepMonths )
        return 0;

    const UInt32 toDelete = (UInt32) fileNames.size() - m_keepMonths;
    UInt32 deleted = 0;
    for ( UInt32 i=0; i<toDelete; ++i )
    {
        const CString path = CombinePath( m_directory, fileNames[ i ] );
        if ( DeleteFile( path ) )
        {
            ++deleted;
            GUCEF_LOG( LOGLEVEL_NORMAL, "MonthlyJsonLinesFile: deleted " << path << ", beyond the retention of " << m_keepMonths << " months" );
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "MonthlyJsonLinesFile: " << path << " is beyond the retention of " << m_keepMonths << " months and cannot be deleted" );
        }
    }
    return deleted;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
