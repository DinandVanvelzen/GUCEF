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

#ifndef GUCEF_CORE_CCSVCOLUMNCONVERTER_H
#define GUCEF_CORE_CCSVCOLUMNCONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

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

/** What the converter needs to know of one file column */
struct SCsvColumnConversion
{
    CDataTableColumnDef column;
    SCsvReadOptions::TDateFormatVector dateFormats;     /**< date columns: the formats that are tried, in order */
};

/**
 *  Turns the text of a field into a value of a column, straight into a column builder, without a string per cell.
 *
 *  Null rule: an unquoted field that equals a null token is null in every column. A quoted field that equals a null
 *  token is null in every column that is not a text column (a number written as "" is missing), and in a text column it
 *  is the text itself, so an empty string and a null stay apart.
 *
 *  A value that does not fit its column is TYPE_MISMATCH, or null when the onValueError option says so, in which case it
 *  is counted. Text that is not valid UTF-8 is CORRUPT_DATA, whatever onValueError says. One thread at a time.
 */
class CCsvColumnConverter
{
    public:

    explicit CCsvColumnConverter( const SCsvReadOptions& options );

    TDataTableStatus Convert( const SCsvColumnConversion& column  ,
                              const char* data                    ,
                              UInt32 length                       ,
                              bool quoted                         ,
                              CDataTableColumnBuilder& builder    ,
                              UInt64& valueErrors                 ,
                              CString& error                      );

    /** Appends null: an absent field */
    TDataTableStatus ConvertMissing( const SCsvColumnConversion& column, CDataTableColumnBuilder& builder, CString& error );

    /** The value with the bytes that cannot be shown replaced, at most 40 of them, for messages */
    static CString Excerpt( const char* data, UInt32 length );

    private:

    CCsvColumnConverter( const CCsvColumnConverter& src );              /**< not implemented */
    CCsvColumnConverter& operator=( const CCsvColumnConverter& src );   /**< not implemented */

    /** Returns true when the value was taken; false with reason set when it does not fit the column */
    bool ConvertValue( const SCsvColumnConversion& column, const char* data, UInt32 length, CDataTableColumnBuilder& builder, CString& reason );

    TDataTableStatus ValueError( const SCsvColumnConversion& column, const char* data, UInt32 length, const CString& reason,
                                 CDataTableColumnBuilder& builder, UInt64& valueErrors, CString& error );

    private:

    const SCsvReadOptions& m_options;
    GUCEF::vector< char > m_scratch;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCSVCOLUMNCONVERTER_H ? */
