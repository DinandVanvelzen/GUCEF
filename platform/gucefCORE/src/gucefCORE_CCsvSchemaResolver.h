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

#ifndef GUCEF_CORE_CCSVSCHEMARESOLVER_H
#define GUCEF_CORE_CCSVSCHEMARESOLVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCSVTABLEREADER_H
#include "gucefCORE_CCsvTableReader.h"
#define GUCEF_CORE_CCSVTABLEREADER_H
#endif /* GUCEF_CORE_CCSVTABLEREADER_H ? */

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
 *  What is known of a column while its values are looked at: which types it can still be. A value can only turn
 *  possibilities off, so the type that results can only get wider with more rows (section 11.1 of the design):
 *  bool, int64, decimal(p,s), float64, date, timestamp(us,utc), utf8.
 */
class CCsvColumnInference
{
    public:

    explicit CCsvColumnInference( const SCsvReadOptions& options );

    /** Looks at one field */
    void Feed( const char* data, UInt32 length, bool quoted );

    /** A record that has no field for the column */
    void FeedMissing( void );

    /** The column that the values seen allow. allNullable makes it nullable whatever was seen */
    CDataTableColumnDef GetColumn( const CString& name, bool allNullable, CString& dateFormat ) const;

    /**
     *  The values written in digits (no exponent) with more than 15 of them, which a float64 does not keep exactly
     *  (identifiers of 20 digits, decimals beyond 18 digits). Only meaningful for a column that GetColumn() makes float64:
     *  a column that stays int64 or decimal reads those values exactly.
     */
    UInt64 GetLossyFloatValueCount( void ) const;

    private:

    const SCsvReadOptions& m_options;
    bool m_sawValue;
    bool m_sawNull;
    bool m_sawQuotedNullToken;
    bool m_possibleBool;
    bool m_possibleInt;
    bool m_possibleDecimal;
    bool m_possibleFloat;
    bool m_possibleTimestamp;
    UInt32 m_maxIntDigits;
    UInt32 m_maxFracDigits;
    UInt32 m_possibleDateFormats;       /**< bit per format of the options */
    UInt64 m_lossyFloatValues;          /**< values of more than 15 digits that a float64 column would round */
};

/*-------------------------------------------------------------------------*/

/**
 *  Finds the schema of a CSV file: the header row and the types, from the schema or the type overrides of the options
 *  and else from the values (inference over inferRowCount rows, all rows when it is 0, which needs a source that can be
 *  repositioned afterwards).
 */
class CCsvSchemaResolver
{
    public:

    static TDataTableStatus Resolve( CIOAccess& source               ,
                                     const IOAccessPtr& sourceHolder ,
                                     CDataTableBufferPool& pool      ,
                                     const SCsvReadOptions& options  ,
                                     MT::CMutex* sharedFileLock      ,
                                     CITableFileMetaDataPtr& metaData,
                                     CString& error                  );

    private:

    CCsvSchemaResolver( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCSVSCHEMARESOLVER_H ? */
