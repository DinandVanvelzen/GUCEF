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

#ifndef GUCEF_CORE_CDATATABLECOLUMNSTATS_H
#define GUCEF_CORE_CDATATABLECOLUMNSTATS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

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
 *  Statistics of one column over a unit (partition, row group or page), used for pruning (section 5.3).
 *
 *  minValue / maxValue are owned variants in the native form of the column: Int64 for signed integers, decimal
 *  mantissas, timestamps; UInt64 for unsigned integers; Float64 for floats; the date variant for dates; UTF8 text and
 *  binary blobs for text and binary. Inexact bounds (truncated text statistics of other writers) are still valid
 *  bounds. A unit without statistics is never pruned.
 *
 *  Float statistics leave NaN out of the bounds. A NaN cell is not equal to any value, so it matches every NOT_EQUALS and
 *  a unit whose bounds are equal to the literal can still hold matching rows: those units are only pruned for NOT_EQUALS
 *  when hasNanCount says that no cell is NaN (nanCount is 0). Statistics of other writers that carry no NaN count
 *  therefore never prune NOT_EQUALS on a float column.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableColumnStats
{
    public:

    bool hasMinMax;
    CVariant minValue;
    CVariant maxValue;
    bool minExact;
    bool maxExact;
    bool hasNullCount;
    UInt64 nullCount;
    bool hasNanCount;           /**< float columns: the number of NaN cells is known. NaN cells are left out of minValue / maxValue */
    UInt64 nanCount;
    UInt64 valueCount;          /**< rows, nulls included */

    CDataTableColumnStats( void );

    /** No statistics at all */
    void Clear( void );

    /** True when no row of the unit holds a value */
    bool IsAllNull( void ) const;

    bool operator==( const CDataTableColumnStats& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLECOLUMNSTATS_H ? */
