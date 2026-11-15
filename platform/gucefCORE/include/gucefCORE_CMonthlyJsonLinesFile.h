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

#ifndef GUCEF_CORE_CMONTHLYJSONLINESFILE_H
#define GUCEF_CORE_CMONTHLYJSONLINESFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CIUTCCLOCK_H
#include "gucefCORE_CIUtcClock.h"
#define GUCEF_CORE_CIUTCCLOCK_H
#endif /* GUCEF_CORE_CIUTCCLOCK_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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
 *  A directory of JSON Lines files, one per month (<yyyy-mm>.jsonl, owner only), that a line is appended to whole or
 *  not at all: the storage of audit trails and command journals of services that run unattended.
 *  - A line that cannot be written whole (a full or read-only disk) is taken back: the file is cut to the size it had
 *    before the append (TruncateFile()), so the next line never follows a fragment, and Append() returns false.
 *  - A fragment that a process left at the end of the newest file (killed, or out of space while it wrote) is cut off
 *    by SetDirectory(): a line that was never acknowledged; the lines before it stay as they are.
 *  - The month of a line never goes back: after the wall clock was set back across the start of a month, lines go on
 *    into the newest file, so the order of the files is the order of the lines.
 *  - Retention: SetKeepMonths( n ) keeps the newest n month files by their names (not by the wall clock, so a clock
 *    set years ahead never makes the trail look expired); older files are deleted when a new month begins and by
 *    SetDirectory(); 0 keeps every file.
 *  - Every append is synced to storage before Append() returns.
 *  The wall clock is the system UTC clock unless SetUtcClock() gives another (tests).
 *  Not thread safe: the owner serialises the calls.
 */
class GUCEF_CORE_PUBLIC_CPP CMonthlyJsonLinesFile
{
    public:

    static const char* const FileExtension;     /**< "jsonl" */

    CMonthlyJsonLinesFile( void );
    ~CMonthlyJsonLinesFile();

    /** The clock that decides the month of a new line; not owned, GUCEF_NULL for the system UTC clock */
    void SetUtcClock( CIUtcClock* clock );

    /**
     *  Uses the directory (made, owner only, when it does not exist): finds the newest month file, cuts off a fragment
     *  at its end and applies the retention. False when the directory cannot be made
     */
    bool SetDirectory( const CString& directory );
    const CString& GetDirectory( void ) const;

    /** 0 keeps every month (the default) */
    void SetKeepMonths( UInt32 keepMonths );
    UInt32 GetKeepMonths( void ) const;

    /** The file the next line goes into: the month of now, or a later one written before */
    CString GetCurrentFilePath( void ) const;

    /** The last complete line of the newest file, without its line end, as SetDirectory() found it; empty for none */
    const CString& GetLastLineAtOpen( void ) const;

    /** Bytes of a trailing fragment that SetDirectory() cut off; 0 when the newest file ended with a whole line */
    UInt32 GetRepairedBytesAtOpen( void ) const;

    /** Appends one line (the line end is added; a line holding a line break is refused) whole and synced, or nothing */
    bool Append( const CString& line, CString& error );

    /**
     *  Reads the complete lines of the newest 'newestMonths' month files (0: all), oldest file first and in file
     *  order; a fragment at the end of a file is skipped. False when a file cannot be read
     */
    bool ReadLines( UInt32 newestMonths, CString::StringVector& lines ) const;

    /** The month files of the directory, by name ("<yyyy-mm>.jsonl"), oldest first */
    void GetMonthFiles( CString::StringVector& fileNames ) const;

    /** Deletes the month files beyond the retention, the oldest first; how many were deleted */
    UInt32 ApplyRetention( void );

    /** Failed appends since the directory was set */
    UInt32 GetFailedAppendCount( void ) const;

    /** The "yyyy-mm" of a file name "<yyyy-mm>.jsonl" of a real month (01 to 12, a year from 1970), empty otherwise */
    static CString MonthOfFileName( const CString& fileName );

    private:

    CMonthlyJsonLinesFile( const CMonthlyJsonLinesFile& src );              /**< not implemented */
    CMonthlyJsonLinesFile& operator=( const CMonthlyJsonLinesFile& src );   /**< not implemented */

    CString MonthOfNow( void ) const;
    void RepairNewestFile( void );

    CIUtcClock* m_clock;
    CString m_directory;
    CString m_newestMonth;
    CString m_lastLineAtOpen;
    UInt32 m_repairedBytesAtOpen;
    UInt32 m_keepMonths;
    UInt32 m_failedAppends;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CMONTHLYJSONLINESFILE_H ? */
