/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H
#define GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_DRN_CDRNPUBLISHEDCOLUMN_H
#include "gucefDRN_CDRNPublishedColumn.h"
#define GUCEF_DRN_CDRNPUBLISHEDCOLUMN_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDCOLUMN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The schema of a published table, the value of the reserved grid key "#schema": the columns in order, which one is
 *  the grid key, and the default order. Encoded as one CORE::CVariantStream segment starting with a version byte.
 */
class GUCEF_DRN_EXPORT_CPP CDRNPublishedTableSchema
{
    public:

    static const UInt8 EncodingVersion = 1;

    /**
     *  The most columns a published table has. A #schema with more is refused when it is read (and by IsValid()), so
     *  that a hostile or broken schema cannot make every row cost that many values on a replica.
     */
    static const UInt32 MaxColumnCount = 1024;

    typedef GUCEF::vector< CDRNPublishedColumn > TColumnVector;

    TColumnVector columns;
    UInt32 keyColumn;                       /**< index of the column that is the grid key */
    Int32 defaultSortColumn;                /**< -1 = key index order */
    bool defaultSortAscending;

    CDRNPublishedTableSchema( void );

    /** At most MaxColumnCount valid columns, unique names and ids, a key column that exists and is not nullable */
    bool IsValid( CORE::CString& error ) const;

    /** The index of the column with this id, -1 when there is none */
    Int32 FindColumnById( const CORE::CString& columnId ) const;

    void Encode( CORE::CVariantStream& stream ) const;

    /**
     *  Reads a #schema segment; a schema that is not valid (IsValid()) is refused
     *  @param unsupported set when the segment is of a newer version than this reader knows
     */
    bool Decode( CORE::CVariantStream& stream, bool& unsupported );

    /** The data table schema: the key column first, then the others in order; the key column is the primary key */
    bool ToDataTableSchema( CORE::CDataTableSchema& schema ) const;

    /** The schema column of the published column at an index (the key column is 0, the others follow) */
    UInt32 ToDataTableColumnIndex( UInt32 publishedIndex ) const;

    bool operator==( const CDRNPublishedTableSchema& other ) const;
    bool operator!=( const CDRNPublishedTableSchema& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H ? */
