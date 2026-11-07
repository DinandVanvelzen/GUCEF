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

#ifndef GUCEF_CORE_CDATATABLEROWGROUPINFO_H
#define GUCEF_CORE_CDATATABLEROWGROUPINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLECOLUMNSTATS_H
#include "gucefCORE_CDataTableColumnStats.h"
#define GUCEF_CORE_CDATATABLECOLUMNSTATS_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNSTATS_H ? */

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
 *  What a file says about one of its row groups: the row count, statistics per column (indexed like the file schema)
 *  and the columns the rows are sorted on.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableRowGroupInfo
{
    public:

    typedef GUCEF::vector< CDataTableColumnStats > TColumnStatsVector;
    typedef GUCEF::vector< UInt32 > TColumnIndexVector;

    UInt64 rowCount;
    TColumnStatsVector columnStats;         /**< one per file schema column; entries without statistics have hasMinMax and hasNullCount false */
    TColumnIndexVector sortedOn;            /**< file schema columns the rows are sorted on, ascending, nulls last; empty when not known */

    CDataTableRowGroupInfo( void );

    void Clear( void );

    /** The statistics of the column, GUCEF_NULL when the file has none for it */
    const CDataTableColumnStats* GetColumnStats( UInt32 fileColumnIndex ) const;

    bool operator==( const CDataTableRowGroupInfo& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEROWGROUPINFO_H ? */
