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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXDATES_H
#define TABLECODECPLUGIN_XLSX_CXLSXDATES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef TABLECODECPLUGIN_XLSX_MACROS_H
#include "tablecodecpluginXLSX_macros.h"
#define TABLECODECPLUGIN_XLSX_MACROS_H
#endif /* TABLECODECPLUGIN_XLSX_MACROS_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the number format of a cell makes of its number */
enum EXlsxNumberKind
{
    XLSX_NUMBERKIND_PLAIN = 0   ,   /**< an ordinary number */
    XLSX_NUMBERKIND_DATE        ,   /**< a date: the number is a serial date */
    XLSX_NUMBERKIND_DATETIME    ,   /**< a date and a time of day */
    XLSX_NUMBERKIND_TIME            /**< a time of day only: the number is a fraction of a day, not a date */
};
typedef enum EXlsxNumberKind TXlsxNumberKind;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Date helpers of the xlsx codec: serial dates of the 1900 and the 1904 date system, ISO 8601 text, and the
 *  classification of number formats (xl/styles.xml). All of them are exact: the time of day is computed from the digits
 *  of the number text and never through a floating point number.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxDates
{
    public:

    static const Int32 MaxDays;                         /**< 9999-12-31 as days since 1970-01-01 */
    static const Int64 MicrosPerDay;

    /**
     *  The days since 1970-01-01 of a whole serial number. The 1900 system counts 1 as 1900-01-01 and, as Excel
     *  does, holds a 29th of February 1900 that never existed (serial 60): that serial and 0 are refused, so a serial
     *  from 61 is one day further than the calendar says it is. The 1904 system counts 0 as 1904-01-01.
     *  False for a serial that is no date (below the first day, 1900 serial 60, above 9999-12-31).
     */
    static bool SerialToDays( Int64 serial, bool date1904, Int32& days );

    /**
     *  The microseconds of the fraction of a day given as the digits behind the decimal point of the number text
     *  (15 digits are used), rounded half up. 86400000000 for a fraction that rounds up to the next midnight.
     */
    static Int64 FractionToMicros( const char* digits, UInt32 digitCount );

    /** Days since 1970-01-01 of a proleptic Gregorian date; false for a month or day that does not exist */
    static bool CivilToDays( Int32 year, UInt32 month, UInt32 day, Int32& days );

    /** The year, month and day of days since 1970-01-01 */
    static void DaysToCivil( Int32 days, Int32& year, UInt32& month, UInt32& day );

    /**
     *  ISO 8601 text of the value cell type d: yyyy-mm-dd, or yyyy-mm-ddThh:mm[:ss[.f..]][Z]. The result is the days and
     *  the microseconds of the day; hasTime tells whether the text has a time of day. A time zone offset is refused.
     */
    static bool ParseIso( const char* text, UInt32 length, Int32& days, Int64& microsOfDay, bool& hasTime );

    /** yyyy-mm-dd, or yyyy-mm-ddThh:mm:ss[.ffffff] when the microseconds of the day are not 0 */
    static CORE::CString ToIsoText( Int32 days, Int64 microsOfDay );

    /** The kind a built in number format id (0..163) has; PLAIN for the ones that are not date formats */
    static TXlsxNumberKind KindOfBuiltInFormat( UInt32 numFmtId );

    /** The kind a custom format code has, by the date and time tokens outside quotes and brackets */
    static TXlsxNumberKind KindOfFormatCode( const CORE::CString& formatCode );

    private:

    CXlsxDates( void );
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

#endif /* TABLECODECPLUGIN_XLSX_CXLSXDATES_H ? */
