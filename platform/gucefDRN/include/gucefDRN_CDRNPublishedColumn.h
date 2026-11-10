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

#ifndef GUCEF_DRN_CDRNPUBLISHEDCOLUMN_H
#define GUCEF_DRN_CDRNPUBLISHEDCOLUMN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATATABLECOLUMNDEF_H
#include "gucefCORE_CDataTableColumnDef.h"
#define GUCEF_CORE_CDATATABLECOLUMNDEF_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNDEF_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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
 *  One column of a published table (#schema of a grid that carries a table, CDRNTablePublisher): the data column
 *  and how to present it. Hint names are those of GUI::CTableViewColumn members (alignment, decimals,
 *  thousandsSeparator, prefix, suffix, dateFormat, booleanDisplay, nullText); DRN stores them as text and does not
 *  interpret them.
 */
class GUCEF_DRN_EXPORT_CPP CDRNPublishedColumn
{
    public:

    CORE::CString id;                       /**< stable identifier, never reused for another meaning: the contract with client code */
    CORE::CDataTableColumnDef def;          /**< name, type, nullability, description */
    CORE::CString title;                    /**< display title, empty = the name */
    Int32 width;                            /**< pixels, 0 = choose */
    bool hidden;                            /**< data for the client, not shown by default */
    CORE::CValueList hints;                 /**< presentation hints, name to text */

    CDRNPublishedColumn( void );
    CDRNPublishedColumn( const CORE::CString& columnId, const CORE::CDataTableColumnDef& columnDef );

    /** Appends the column to a #schema segment */
    void Encode( CORE::CVariantStream& stream ) const;

    /** Reads the column from a #schema segment; false when it is malformed or the type is unknown */
    bool Decode( CORE::CVariantStream& stream );

    bool operator==( const CDRNPublishedColumn& other ) const;
    bool operator!=( const CDRNPublishedColumn& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNPUBLISHEDCOLUMN_H ? */
